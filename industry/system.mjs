// SPDX-License-Identifier: GPL-3.0-only
import { CommunicationControl, ConnectionControl, TransmissionControl } from './controls.mjs';
import { PaymentAdapters, PaymentContract } from './payment.mjs';
import { TelecomAdapters } from './telecom-adapter.mjs';
import { PlatformSystem } from '../platform/system.mjs';

export class IndustrySystem {
    #communications = new CommunicationControl();
    #authorize;
    #profiles = new Map([
        ['stripe', { origins: ['https://api.stripe.com'],
            fields: ['secret', 'apiVersion', 'returnUrl', 'cancelUrl', 'itemName'] }],
        ['paypal', { origins: ['https://api-m.sandbox.paypal.com', 'https://api-m.paypal.com'],
            fields: ['clientId', 'secret', 'returnUrl', 'cancelUrl'] }],
        ['vipps', { origins: ['https://apitest.vipps.no', 'https://api.vipps.no'],
            fields: ['clientId', 'secret', 'subscriptionKey', 'merchantSerialNumber', 'returnUrl', 'itemName'] }],
        ['upi', { origins: ['https://api.razorpay.com'], fields: ['keyId', 'secret', 'itemName'] }],
        ['bitcoin', { fields: ['apiKey', 'storeId'] }],
        ['shopify', { fields: ['secret', 'apiVersion'] }],
        ['twilio', { origins: ['https://api.twilio.com'],
            fields: ['accountSid', 'apiKeySid', 'apiKeySecret', 'messagingServiceSid'] }],
        ['camara', { fields: ['verifyPath'] }],
        ['internet', { fields: ['statusPath'] }],
    ]);
    #payments = new Set([
        'visa', 'mastercard', 'vipps', 'upi', 'google-pay', 'apple-pay', 'paypal', 'bitcoin', 'shopify',
    ]);

    constructor({ providers = {}, authorize, fetch, exec }) {
        if (typeof authorize !== 'function') throw new TypeError('An application authorization handler is required');
        this.#authorize = authorize;
        providers = structuredClone(providers);
        const connections = new ConnectionControl();
        const transport = new TransmissionControl(fetch);
        for (const [name, { origins, fields }] of this.#profiles) {
            if (!providers[name]) continue;
            const { origin, checkoutOrigins } = providers[name];
            if (origins && !origins.includes(origin)) throw new TypeError(`Invalid provider origin for ${name}`);
            if (fields.some(field => typeof providers[name][field] !== 'string' || !providers[name][field].trim())) {
                throw new TypeError(`Required configuration is missing for ${name}`);
            }
            for (const field of ['returnUrl', 'cancelUrl']) {
                if (!fields.includes(field)) continue;
                const url = new URL(providers[name][field]);
                if (url.protocol !== 'https:' || url.username || url.password) {
                    throw new TypeError(`Invalid checkout return URL for ${name}`);
                }
            }
            if (['stripe', 'paypal', 'vipps', 'upi', 'bitcoin', 'shopify'].includes(name)) {
                if (!Array.isArray(checkoutOrigins) || !checkoutOrigins.length ||
                    checkoutOrigins.some(origin => new URL(origin).origin !== origin || !origin.startsWith('https://'))) {
                    throw new TypeError(`Invalid checkout origin list for ${name}`);
                }
            }
            connections.register(name, origin, transport);
        }
        if (providers.twilio && (!Number.isInteger(providers.twilio.maxSegments) || providers.twilio.maxSegments < 1)) {
            throw new TypeError('The SMS route requires a positive maximum segment count');
        }
        if (providers.stripe?.wallets !== undefined && (!Array.isArray(providers.stripe.wallets) ||
            providers.stripe.wallets.some(wallet => !['apple-pay', 'google-pay'].includes(wallet)))) {
            throw new TypeError('Stripe wallet routes must be explicitly configured');
        }
        if (providers.shopify && !/^\d{4}-(01|04|07|10)$/.test(providers.shopify.apiVersion)) {
            throw new TypeError('Shopify requires a pinned Storefront API version');
        }
        if (providers.stripe && !/^\d{4}-\d{2}-\d{2}(\.[a-z]+)?$/.test(providers.stripe.apiVersion)) {
            throw new TypeError('Stripe requires a pinned API version');
        }
        if (providers.camara && !/^\/(?!\/)[A-Za-z0-9_/-]+$/.test(providers.camara.verifyPath)) {
            throw new TypeError('Register an absolute CAMARA verification path');
        }
        new PaymentAdapters(connections, providers).register(this.#communications);
        new TelecomAdapters(connections, providers, exec).register(this.#communications);
        if (providers.platform) {
            new PlatformSystem({ root: providers.platform.imageRoot, exec }).register(this.#communications);
        }
        if (providers.internet) {
            const request = connections.open('internet');
            const { statusPath } = providers.internet;
            if (!/^\/(?!\/)[A-Za-z0-9_/-]+$/.test(statusPath)) {
                throw new TypeError('Register an absolute HTTPS status resource path');
            }
            this.#communications.register('internet', {
                status: async () => {
                    const report = await request(statusPath);
                    if (typeof report.state !== 'string' ||
                        !['ready', 'degraded', 'unavailable'].includes(report.state)) {
                        throw new Error('Registered HTTPS health source returned an invalid state');
                    }
                    return { state: report.state };
                },
            });
        }
    }

    capabilities() {
        return this.#communications.capabilities();
    }

    async invoke({ profile, operation, input }, identity) {
        // The host returns trusted order data and bound references after checking identity and permission.
        const trusted = await this.#authorize({ profile, operation, input, identity });
        if (!trusted || typeof trusted !== 'object') {
            const error = new Error('Industry operation denied');
            error.code = 'INDUSTRY_DENIED';
            throw error;
        }
        const result = await this.#communications.invoke({ profile, operation, input: trusted });
        return this.#payments.has(profile) && result.state === 'paid'
            ? this.reconcilePayment(result, trusted) : result;
    }

    reconcilePayment(result, expected) {
        const actual = result.amountMinor ??
            PaymentContract.decimalMinor(result.amount, result.currency);
        const matched = Number.isSafeInteger(expected.amountMinor) &&
            expected.amountMinor > 0 && expected.amountMinor === actual &&
            expected.currency === result.currency;
        return matched ? result : { ...result, state: 'reconciliation-required' };
    }

}
