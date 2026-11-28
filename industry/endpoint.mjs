// SPDX-License-Identifier: GPL-3.0-only

export class IndustryEndpoint {
    operations = new Map([['GET', { effect: 'capabilities' }], ['POST', { effect: 'invoke' }]]);
    #origin;
    #system;
    #authenticate;
    #maxBytes = 16_384;

    constructor({ origin, system, authenticate }) {
        if (new URL(origin).origin !== origin || !origin.startsWith('https://') ||
            typeof authenticate !== 'function') {
            throw new TypeError('An HTTPS application origin and authentication handler are required');
        }
        this.#origin = origin;
        this.#system = system;
        this.#authenticate = authenticate;
    }

    reply(value, status = 200) {
        return Response.json(value, {
            status, headers: { 'Cache-Control': 'no-store', 'X-Content-Type-Options': 'nosniff' },
        });
    }

    async handle(request) {
        if (!this.operations.has(request.method)) return this.reply({ error: 'method-not-allowed' }, 405);
        if (request.method === 'POST' && request.headers.get('origin') !== this.#origin) {
            return this.reply({ error: 'origin-denied' }, 403);
        }
        let identity;
        try {
            identity = await this.#authenticate(request);
        } catch {
            return this.reply({ error: 'authentication-failed' }, 401);
        }
        if (!identity) return this.reply({ error: 'authentication-required' }, 401);
        if (request.method === 'GET') return this.reply(this.#system.capabilities());
        const type = request.headers.get('content-type')?.split(';')[0]?.trim().toLowerCase();
        if (type !== 'application/json') return this.reply({ error: 'json-required' }, 415);
        let input;
        try {
            input = await this.read(request);
        } catch (error) {
            return error instanceof RangeError
                ? this.reply({ error: 'request-too-large' }, 413)
                : this.reply({ error: 'invalid-request' }, 400);
        }
        try {
            return this.reply(await this.#system.invoke(input, identity));
        } catch (error) {
            if (error.code === 'INDUSTRY_DENIED') return this.reply({ error: 'operation-denied' }, 403);
            return this.reply({ error: 'operation-failed' }, 422);
        }
    }

    async read(request) {
        if (!request.body) throw new TypeError('Request body is required');
        const reader = request.body.getReader();
        const decoder = new TextDecoder('utf-8', { fatal: true });
        let text = '';
        let size = 0;
        for (;;) {
            const { done, value } = await reader.read();
            if (done) break;
            size += value.byteLength;
            if (size > this.#maxBytes) {
                await reader.cancel();
                throw new RangeError('Request body exceeds limit');
            }
            text += decoder.decode(value, { stream: true });
        }
        const input = JSON.parse(text + decoder.decode());
        if (!input || typeof input.profile !== 'string' || typeof input.operation !== 'string' ||
            !input.input || typeof input.input !== 'object' || Array.isArray(input.input)) {
            throw new TypeError('Invalid industry invocation');
        }
        return input;
    }
}
