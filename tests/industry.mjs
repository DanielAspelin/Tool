// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { readFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';
import { IndustrySystem } from '../industry/system.mjs';
import { IndustryEndpoint } from '../industry/endpoint.mjs';
import { ConnectionControl, TransmissionControl } from '../industry/controls.mjs';
import { TelecomData } from '../industry/telecom.mjs';

class IndustryVerification {
    static amountMinor = 1050;
    static sid = 'SM' + '1'.repeat(32);
    static approval = 'https://checkout.example.test';
    static app = 'https://app.example.test';

    static providers() {
        const common = { secret: 'fixture', checkoutOrigins: [this.approval], itemName: 'Fixture',
            returnUrl: this.app + '/return', cancelUrl: this.app + '/cancel' };
        return {
            stripe: { ...common, origin: 'https://api.stripe.com', apiVersion: '2026-10-01.fixture',
                wallets: ['apple-pay', 'google-pay'] },
            paypal: { ...common, origin: 'https://api-m.sandbox.paypal.com', clientId: 'fixture' },
            vipps: { ...common, origin: 'https://apitest.vipps.no',
                clientId: 'fixture', subscriptionKey: 'fixture', merchantSerialNumber: 'fixture' },
            upi: { ...common, origin: 'https://api.razorpay.com', keyId: 'fixture' },
            bitcoin: { ...common, origin: 'https://btcpay.example.test', apiKey: 'fixture', storeId: 'store' },
            shopify: { ...common, origin: 'https://shop.example.test', apiVersion: '2026-10' },
            modem: { modemId: 0 },
            twilio: { origin: 'https://api.twilio.com', accountSid: 'AC' + '1'.repeat(32),
                apiKeySid: 'SK' + '1'.repeat(32), apiKeySecret: 'fixture',
                messagingServiceSid: 'MG' + '1'.repeat(32), maxSegments: 3 },
            camara: { origin: 'https://carrier.example.test', verifyPath: '/number-verification/v2/verify' },
            internet: { origin: 'https://service.example.test', statusPath: '/health' },
        };
    }

    static fixture(url, options) {
        const { method, headers, redirect, signal, body } = options;
        assert.equal(redirect, 'error');
        assert.ok(signal);
        const json = () => JSON.parse(body);
        const checkout = this.approval + '/start';
        const amount = this.amountMinor;
        const p = url.pathname;
        if (p === '/v1/checkout/sessions') {
            assert.match(headers['Idempotency-Key'], /^order-/);
            assert.equal(body.get('line_items[0][price_data][unit_amount]'), String(amount));
            return { id: 'stripe', url: checkout };
        }
        if (p === '/v1/checkout/sessions/stripe') {
            return { payment_status: 'paid', amount_total: amount, currency: 'eur' };
        }
        if (p === '/v1/oauth2/token' || p === '/accesstoken/get') {
            assert.equal(method, 'POST');
            return { access_token: 'fixture' };
        }
        if (p === '/v2/checkout/orders') {
            assert.equal(json().purchase_units[0].amount.value, '10.50');
            return { id: 'paypal', links: [{ rel: 'payer-action', href: checkout }] };
        }
        if (p === '/v2/checkout/orders/paypal') return { status: 'APPROVED' };
        if (p === '/v2/checkout/orders/paypal/capture') {
            assert.equal(headers['PayPal-Request-Id'], 'capture-paypal');
            return { status: 'COMPLETED', purchase_units: [{ payments: { captures: [
                { status: 'COMPLETED', amount: { value: '10.50', currency_code: 'EUR' } },
            ] } }] };
        }
        if (p === '/epayment/v1/payments') return { reference: json().reference, redirectUrl: checkout };
        if (p.endsWith('/capture') && p.startsWith('/epayment/')) {
            assert.equal(json().modificationAmount.value, amount);
            return {};
        }
        if (p.startsWith('/epayment/v1/payments/')) {
            return { state: 'AUTHORIZED', amount: { value: amount, currency: 'EUR' }, aggregate: {
                capturedAmount: { value: amount, currency: 'EUR' }, refundedAmount: { value: 0 },
            } };
        }
        if (p === '/v1/payment_links') {
            assert.equal(json().upi_link, true);
            assert.equal(json().currency, 'INR');
            return { id: 'upi', short_url: checkout };
        }
        if (p === '/v1/payment_links/upi') return { status: 'paid', amount_paid: amount, currency: 'INR' };
        if (p.endsWith('/invoices')) {
            assert.equal(json().amount, '0.00001050');
            assert.deepEqual(json().checkout.paymentMethods, ['BTC-CHAIN']);
            return { id: 'btc', checkoutLink: checkout };
        }
        if (p.endsWith('/invoices/btc')) {
            return { status: 'Settled', additionalStatus: 'None', amount: '0.00001050', currency: 'BTC' };
        }
        if (p === '/api/2026-10/graphql.json') {
            assert.equal(json().variables.input.lines.length, 1);
            return { data: { cartCreate: { cart: { id: 'cart', checkoutUrl: checkout }, userErrors: [] } } };
        }
        if (p.includes('/Messages')) return { sid: this.sid, status: method === 'POST' ? 'queued' : 'delivered' };
        if (p.endsWith('/verify')) return { devicePhoneNumberVerified: true };
        if (p === '/health') return { state: 'ready' };
        assert.fail(`Unexpected fixture request: ${p}`);
    }

    static async server() {
        const transport = new TransmissionControl();
        await assert.rejects(transport.request('https://api.stripe.com', '//wrong.example/test'), TypeError);
        assert.throws(() => new ConnectionControl().register('x', 'http://example.test', transport), TypeError);
        let calls = 0;
        const system = new IndustrySystem({
            providers: this.providers(),
            authorize: ({ input, identity }) => identity === 'merchant' ? input : null,
            fetch: async (url, options) => {
                calls++;
                return Response.json(this.fixture(url, options));
            },
            exec: async (file, args) => {
                assert.deepEqual([file, ...args], ['mmcli', '--modem', '0', '--output-json']);
                return { stdout: JSON.stringify({ modem: { generic: { state: 'registered',
                    'access-technologies': ['lte'] }, '3gpp': { 'registration-state': 'home' } } }) };
            },
        });
        const invoke = (profile, operation, input) => system.invoke({ profile, operation, input }, 'merchant');
        assert.equal(system.capabilities().schema, 'tool.industry.v1');
        await assert.rejects(system.invoke({ profile: 'visa', operation: 'create', input: {} }, 'guest'), /denied/);
        assert.equal(calls, 0);
        const disabled = new IndustrySystem({ authorize: () => ({}) });
        assert.deepEqual(disabled.capabilities().profiles.map(profile => profile.id), ['imei', 'sms']);
        const badCheckout = new IndustrySystem({
            providers: { stripe: this.providers().stripe },
            authorize: ({ input }) => input,
            fetch: async () => Response.json({ id: 'wrong', url: 'https://wrong.example.test/start' }),
        });
        await assert.rejects(badCheckout.invoke({
            profile: 'visa', operation: 'create',
            input: { orderKey: 'one', amountMinor: 100, currency: 'EUR' },
        }, 'merchant'), /unregistered origin/);
        const base = { amountMinor: this.amountMinor, currency: 'EUR' };
        for (const profile of ['visa', 'mastercard', 'apple-pay', 'google-pay', 'paypal', 'vipps', 'upi', 'bitcoin', 'shopify']) {
            const created = await invoke(profile, 'create', {
                ...base, orderKey: 'order-' + profile, currency: profile === 'upi' ? 'INR'
                    : profile === 'bitcoin' ? 'BTC' : 'EUR',
                lines: [{ merchandiseId: 'gid://shopify/ProductVariant/123', quantity: 1 }],
            });
            assert.equal(created.state, 'checkout-required');
            assert.equal(created.checkoutUrl, this.approval + '/start');
        }
        assert.equal((await invoke('visa', 'status', { ...base, reference: 'stripe' })).state, 'paid');
        assert.equal((await invoke('visa', 'status', { ...base, amountMinor: 1, reference: 'stripe' })).state,
            'reconciliation-required');
        assert.equal((await invoke('paypal', 'status', { ...base, reference: 'paypal' })).state, 'approved');
        assert.equal((await invoke('paypal', 'capture', { ...base, reference: 'paypal',
            requestKey: 'capture-paypal' })).state, 'paid');
        assert.equal((await invoke('vipps', 'capture', { ...base, reference: 'order-vipps',
            requestKey: 'capture-vipps' })).state, 'paid');
        assert.equal((await invoke('upi', 'status', { ...base, currency: 'INR', reference: 'upi' })).state, 'paid');
        assert.equal((await invoke('bitcoin', 'status', { ...base, currency: 'BTC', reference: 'btc' })).state, 'paid');
        await assert.rejects(invoke('shopify', 'status', { reference: 'cart' }), /unavailable/);

        assert.deepEqual(await invoke('imei', 'validate', { imei: '490154203237518' }), { valid: true });
        assert.equal((await invoke('gsm', 'modem-status', {})).state, 'registered');
        assert.equal((await invoke('sms', 'send', { text: 'Fixture', to: '+15550100000' })).state, 'queued');
        assert.equal((await invoke('sms', 'status', { reference: this.sid })).state, 'delivered');
        assert.equal((await invoke('number-verification', 'verify', {
            number: '+15550100000', subscriberToken: 'fixture',
        })).verified, true);
        assert.equal((await invoke('internet', 'status', { path: '//unused.test' })).state, 'ready');

        const endpoint = new IndustryEndpoint({
            origin: this.app, system, authenticate: () => 'merchant',
        });
        const input = { profile: 'imei', operation: 'validate', input: { imei: '490154203237518' } };
        const request = (origin, body) => new Request(this.app + '/api/industry', { method: 'POST',
            headers: { origin, 'content-type': 'application/json' }, body: JSON.stringify(body) });
        assert.equal((await endpoint.handle(request('https://other.example', input))).status, 403);
        assert.equal((await endpoint.handle(request(this.app, { huge: 'a'.repeat(17_000) }))).status, 413);
        assert.deepEqual(await (await endpoint.handle(request(this.app, input))).json(), { valid: true });
        return endpoint;
    }

    static async browser(directory, endpoint) {
        if (!process.env.TOOL_TEST_WEBR) return;
        const { WebR } = await import(pathToFileURL(resolve(process.env.TOOL_TEST_WEBR)));
        const { IndustryClient } = await import(pathToFileURL(resolve(directory, 'industry/client.mjs')));
        const webR = new WebR();
        await webR.init();
        const original = globalThis.fetch;
        globalThis.fetch = (url, options) => {
            const request = new Request(url, options);
            request.headers.set('Origin', this.app);
            return endpoint.handle(request);
        };
        try {
            const source = await readFile(resolve(directory, 'industry/industry.R'), 'utf8');
            const client = new IndustryClient(this.app + '/api/industry', webR, source);
            assert.equal((await client.capabilities()).schema, 'tool.industry.v1');
            assert.deepEqual(await client.invoke('imei', 'validate', { imei: '490154203237518' }), { valid: true });
            await assert.rejects(client.invoke('imei', 'wrong', {}), /HTTP 422/);
            assert.deepEqual(await client.statistics(), {
                capabilities: 1, operations: 2, succeeded: 2, failed: 1, auditComplete: true,
            });
        } finally {
            globalThis.fetch = original;
            await webR.close();
        }
    }

    static async run(directory) {
        assert.equal(TelecomData.validateIMEI('490154203237519'), false);
        assert.equal(TelecomData.validateIMEI('000000000000000'), false);
        assert.deepEqual(TelecomData.planSMS('^'.repeat(81)), { encoding: 'gsm-7', units: 162, segments: 2 });
        assert.deepEqual(TelecomData.planSMS('😀'.repeat(36)), { encoding: 'utf-16', units: 72, segments: 2 });
        const tool = resolve(directory, 'tool');
        assert.equal(execFileSync(tool, ['imei', 'validate', '490154203237518'], { encoding: 'utf8' }).trim(), 'valid');
        assert.equal(execFileSync(tool, ['imei.validate', '490154203237519'], { encoding: 'utf8' }).trim(), 'invalid');
        await this.browser(directory, await this.server());
        console.log('Industry provider, telecom and Internet contract checks passed');
    }
}

await IndustryVerification.run(process.argv[2]);
