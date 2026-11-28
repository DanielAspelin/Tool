// SPDX-License-Identifier: GPL-3.0-only
import { Buffer } from 'node:buffer';

export class PaymentContract {
    static currencies = new Map([
        ['USD', 2], ['EUR', 2], ['GBP', 2], ['NOK', 2], ['DKK', 2],
        ['SEK', 2], ['INR', 2], ['BTC', 8], ['JPY', 0],
    ]);

    static identifier(value) {
        if (typeof value !== 'string' || !/^[A-Za-z0-9_-]{1,40}$/.test(value)) {
            throw new TypeError('Expected a 1 to 40 character order key');
        }
        return value;
    }

    static money({ amountMinor, currency }) {
        const places = this.currencies.get(currency);
        if (!Number.isSafeInteger(amountMinor) || amountMinor <= 0 || places === undefined) {
            throw new TypeError('Expected a positive integer amount and supported currency');
        }
        const digits = String(amountMinor).padStart(places + 1, '0');
        return { amountMinor, currency, decimal: places
            ? `${digits.slice(0, -places)}.${digits.slice(-places)}` : digits };
    }

    static decimalMinor(amount, currency) {
        const places = this.currencies.get(currency);
        const parts = typeof amount === 'string' && /^(\d+)(?:\.(\d+))?$/.exec(amount);
        if (!parts || places === undefined || (parts[2]?.length ?? 0) > places) return undefined;
        return Number(parts[1] + (parts[2] ?? '').padEnd(places, '0'));
    }

    static https(value, origins) {
        const url = new URL(value);
        if (url.protocol !== 'https:' || url.username || url.password ||
            !origins.includes(url.origin)) {
            throw new Error('Provider checkout URL has an unregistered origin');
        }
        return url.href;
    }

    static basic(user, secret) {
        return `Basic ${Buffer.from(`${user}:${secret}`).toString('base64')}`;
    }

    static json(body, headers = {}) {
        return {
            method: 'POST', headers: { ...headers, 'Content-Type': 'application/json' },
            body: JSON.stringify(body),
        };
    }

    static form(fields, headers = {}) {
        return { method: 'POST', headers, body: new URLSearchParams(fields) };
    }

    static result(id, redirect, config) {
        if (typeof id !== 'string' || !id || id.length > 2048) throw new Error('Missing provider reference');
        return {
            state: 'checkout-required',
            reference: id,
            checkoutUrl: this.https(redirect, config.checkoutOrigins),
        };
    }

    static path(id) {
        if (typeof id !== 'string' || !id || id.length > 2048) throw new TypeError('Missing provider reference');
        return encodeURIComponent(id);
    }
}

export class PaymentAdapters {
    #operations = new Map([
        ['stripe', { create: 'stripeCreate', status: 'stripeStatus' }],
        ['paypal', { create: 'paypalCreate', capture: 'paypalCapture', status: 'paypalStatus' }],
        ['vipps', { create: 'vippsCreate', capture: 'vippsCapture', status: 'vippsStatus' }],
        ['upi', { create: 'upiCreate', status: 'upiStatus' }],
        ['bitcoin', { create: 'bitcoinCreate', status: 'bitcoinStatus' }],
        ['shopify', { create: 'shopifyCreate' }],
    ]);

    constructor(connections, config) {
        this.connections = connections;
        this.config = config;
    }

    register(communications) {
        const profiles = new Map([
            ['visa', 'stripe'], ['mastercard', 'stripe'], ['google-pay', 'stripe'],
            ['apple-pay', 'stripe'], ['paypal', 'paypal'], ['vipps', 'vipps'],
            ['upi', 'upi'], ['bitcoin', 'bitcoin'], ['shopify', 'shopify'],
        ]);
        for (const [profile, provider] of profiles) {
            const config = this.config[provider];
            if (!config) continue;
            if (provider === 'stripe' && profile.endsWith('-pay') && !config.wallets?.includes(profile)) {
                continue;
            }
            const request = this.connections.open(provider);
            const operations = Object.entries(this.#operations.get(provider)).map(([op, method]) => [
                op, input => this[method](request, config, input),
            ]);
            communications.register(profile, Object.fromEntries(operations));
        }
    }

    async stripeCreate(request, config, { orderKey, ...amount }) {
        const key = PaymentContract.identifier(orderKey);
        const { amountMinor, currency } = PaymentContract.money(amount);
        if (currency === 'BTC') throw new TypeError('Stripe Checkout requires a fiat currency');
        const session = await request('/v1/checkout/sessions', PaymentContract.form({
            mode: 'payment', client_reference_id: key,
            success_url: config.returnUrl, cancel_url: config.cancelUrl,
            'payment_method_types[0]': 'card',
            'line_items[0][quantity]': '1',
            'line_items[0][price_data][currency]': currency.toLowerCase(),
            'line_items[0][price_data][unit_amount]': String(amountMinor),
            'line_items[0][price_data][product_data][name]': config.itemName,
        }, { Authorization: `Bearer ${config.secret}`, 'Idempotency-Key': key,
            'Stripe-Version': config.apiVersion }));
        return PaymentContract.result(session.id, session.url, config);
    }

    async stripeStatus(request, config, { reference }) {
        const session = await request(`/v1/checkout/sessions/${PaymentContract.path(reference)}`, {
            headers: { Authorization: `Bearer ${config.secret}`, 'Stripe-Version': config.apiVersion },
        });
        return { state: session.payment_status === 'paid' ? 'paid'
            : session.status === 'expired' ? 'expired' : 'pending', reference,
            amountMinor: session.amount_total, currency: session.currency?.toUpperCase() };
    }

    async paypalToken(request, config) {
        const { access_token: token } = await request('/v1/oauth2/token', PaymentContract.form(
            { grant_type: 'client_credentials' },
            { Authorization: PaymentContract.basic(config.clientId, config.secret) }
        ));
        if (!token) throw new Error('PayPal did not return an access token');
        return { Authorization: `Bearer ${token}` };
    }

    async paypalCreate(request, config, { orderKey, ...amount }) {
        const key = PaymentContract.identifier(orderKey);
        const { decimal, currency } = PaymentContract.money(amount);
        if (currency === 'BTC' || key.length > 38) throw new TypeError('Unsupported PayPal payment input');
        const headers = await this.paypalToken(request, config);
        const order = await request('/v2/checkout/orders', PaymentContract.json({
            intent: 'CAPTURE', purchase_units: [{ reference_id: key,
                amount: { currency_code: currency, value: decimal } }],
            payment_source: { paypal: { experience_context: {
                return_url: config.returnUrl, cancel_url: config.cancelUrl,
            } } },
        }, { ...headers, 'PayPal-Request-Id': key }));
        const approve = order.links?.find(link => ['approve', 'payer-action'].includes(link.rel));
        return PaymentContract.result(order.id, approve?.href, config);
    }

    async paypalCapture(request, config, { reference, requestKey }) {
        const key = PaymentContract.identifier(requestKey);
        if (key.length > 38) throw new TypeError('PayPal request key exceeds 38 characters');
        const headers = await this.paypalToken(request, config);
        const order = await request(`/v2/checkout/orders/${PaymentContract.path(reference)}/capture`,
            PaymentContract.json({}, { ...headers, 'PayPal-Request-Id': key }));
        return this.paypalResult(order, reference);
    }

    async paypalStatus(request, config, { reference }) {
        const order = await request(`/v2/checkout/orders/${PaymentContract.path(reference)}`,
            { headers: await this.paypalToken(request, config) });
        return this.paypalResult(order, reference);
    }

    paypalResult(order, reference) {
        const captures = order.purchase_units?.flatMap(unit => unit.payments?.captures ?? []) ?? [];
        const capture = captures.length === 1 ? captures[0] : undefined;
        const state = capture?.status === 'COMPLETED' ? 'paid'
            : order.status === 'APPROVED' ? 'approved' : order.status === 'VOIDED' ? 'canceled' : 'pending';
        return { state, reference, amount: capture?.amount?.value, currency: capture?.amount?.currency_code };
    }

    async vippsToken(request, config) {
        const common = { 'Ocp-Apim-Subscription-Key': config.subscriptionKey,
            'Merchant-Serial-Number': config.merchantSerialNumber,
            'Vipps-System-Name': 'Tool', 'Vipps-System-Version': '1.0.0' };
        const { access_token: token } = await request('/accesstoken/get', { method: 'POST',
            headers: { ...common, client_id: config.clientId, client_secret: config.secret } });
        if (!token) throw new Error('Vipps did not return an access token');
        return { ...common, Authorization: `Bearer ${token}` };
    }

    async vippsCreate(request, config, { orderKey, ...amount }) {
        const key = PaymentContract.identifier(orderKey);
        const { amountMinor, currency } = PaymentContract.money(amount);
        if (key.length < 8 || !['NOK', 'DKK', 'EUR', 'SEK'].includes(currency)) {
            throw new TypeError('Unsupported Vipps ePayment input');
        }
        const headers = await this.vippsToken(request, config);
        const payment = await request('/epayment/v1/payments', PaymentContract.json({
            reference: key, amount: { value: amountMinor, currency },
            paymentMethod: { type: 'WALLET' }, userFlow: 'WEB_REDIRECT',
            returnUrl: config.returnUrl, paymentDescription: config.itemName,
        }, { ...headers, 'Idempotency-Key': key }));
        return PaymentContract.result(payment.reference, payment.redirectUrl, config);
    }

    async vippsCapture(request, config, { reference, requestKey, ...amount }) {
        const { amountMinor: value, currency } = PaymentContract.money(amount);
        const headers = await this.vippsToken(request, config);
        await request(`/epayment/v1/payments/${PaymentContract.path(reference)}/capture`,
            PaymentContract.json({ modificationAmount: { currency, value } },
                { ...headers, 'Idempotency-Key': PaymentContract.identifier(requestKey) }));
        return this.vippsStatus(request, config, { reference });
    }

    async vippsStatus(request, config, { reference }) {
        const payment = await request(`/epayment/v1/payments/${PaymentContract.path(reference)}`,
            { headers: await this.vippsToken(request, config) });
        const { capturedAmount, refundedAmount } = payment.aggregate ?? {};
        const paid = capturedAmount?.value > 0 && capturedAmount.value === payment.amount?.value &&
            capturedAmount.currency === payment.amount?.currency && refundedAmount?.value === 0;
        const state = paid ? 'paid' : ({
            AUTHORIZED: 'approved', ABORTED: 'canceled', EXPIRED: 'expired', TERMINATED: 'canceled',
        })[payment.state] ?? 'pending';
        return { state, reference, amountMinor: capturedAmount?.value, currency: capturedAmount?.currency };
    }

    async upiCreate(request, config, { orderKey, ...amount }) {
        const key = PaymentContract.identifier(orderKey);
        const { amountMinor, currency } = PaymentContract.money(amount);
        if (currency !== 'INR') throw new TypeError('This UPI adapter requires INR');
        const payment = await request('/v1/payment_links', PaymentContract.json({
            amount: amountMinor, currency, upi_link: true, reference_id: key,
            description: config.itemName, notify: { sms: false, email: false }, reminder_enable: false,
        }, { Authorization: PaymentContract.basic(config.keyId, config.secret) }));
        return PaymentContract.result(payment.id, payment.short_url, config);
    }

    async upiStatus(request, config, { reference }) {
        const payment = await request(`/v1/payment_links/${PaymentContract.path(reference)}`, {
            headers: { Authorization: PaymentContract.basic(config.keyId, config.secret) },
        });
        const state = ({ paid: 'paid', expired: 'expired', cancelled: 'canceled' })[payment.status] ?? 'pending';
        return { state, reference, amountMinor: payment.amount_paid, currency: payment.currency };
    }

    async bitcoinCreate(request, config, { orderKey, ...amount }) {
        const key = PaymentContract.identifier(orderKey);
        const { decimal, currency } = PaymentContract.money(amount);
        const payment = await request(`/api/v1/stores/${PaymentContract.path(config.storeId)}/invoices`,
            PaymentContract.json({
                amount: decimal, currency, metadata: { orderId: key },
                checkout: { paymentMethods: ['BTC-CHAIN'] },
            },
                { Authorization: `token ${config.apiKey}` }));
        return PaymentContract.result(payment.id, payment.checkoutLink, config);
    }

    async bitcoinStatus(request, config, { reference }) {
        const payment = await request(`/api/v1/stores/${PaymentContract.path(config.storeId)}/invoices/${PaymentContract.path(reference)}`,
            { headers: { Authorization: `token ${config.apiKey}` } });
        const state = payment.status === 'Settled' && payment.additionalStatus === 'None' ? 'paid'
            : payment.status === 'Expired' ? 'expired' : 'pending';
        return { state, reference, amount: payment.amount, currency: payment.currency };
    }

    async shopifyCreate(request, config, { lines }) {
        if (!Array.isArray(lines) || !lines.length || lines.length > 100) {
            throw new TypeError('Expected 1 to 100 Shopify cart lines');
        }
        const cartLines = lines.map(({ merchandiseId, quantity }) => {
            if (typeof merchandiseId !== 'string' || !/^gid:\/\/shopify\/ProductVariant\/\d+$/.test(merchandiseId) ||
                !Number.isInteger(quantity) || quantity < 1 || quantity > 100) {
                throw new TypeError('Invalid Shopify cart line');
            }
            return { merchandiseId, quantity };
        });
        const result = await request(`/api/${config.apiVersion}/graphql.json`, PaymentContract.json({
            query: 'mutation Checkout($input: CartInput!) { cartCreate(input: $input) { cart { id checkoutUrl } userErrors { code } } }',
            variables: { input: { lines: cartLines } },
        }, { 'Shopify-Storefront-Private-Token': config.secret }));
        const checkout = result.data?.cartCreate;
        if (result.errors?.length || checkout?.userErrors?.length || !checkout?.cart) {
            throw new Error('Shopify could not create the cart');
        }
        return PaymentContract.result(checkout.cart.id, checkout.cart.checkoutUrl, config);
    }
}
