// SPDX-License-Identifier: GPL-3.0-only

export class IndustryClient {
    operations = new Map([
        ['capabilities', { method: 'GET', audit: 1 }],
        ['invoke', { method: 'POST', audit: 2 }],
    ]);
    #auditSource;
    #auditTail = Promise.resolve();
    #counts = [0, 0, 0, 0];
    #auditFailure;
    #endpoint;
    #webR;
    #csrf;

    constructor(endpoint, webR, auditSource, csrfToken) {
        this.#endpoint = endpoint;
        this.#webR = webR;
        this.#auditSource = auditSource;
        this.#csrf = csrfToken;
    }

    static async create({ webR, endpoint = '/api/industry', csrfToken }) {
        const url = new URL(endpoint, globalThis.location.href);
        if (url.origin !== globalThis.location.origin || url.protocol !== 'https:' ||
            url.username || url.password || url.hash) {
            throw new TypeError('Industry client requires a same-origin HTTPS endpoint');
        }
        const source = await fetch(new URL('./industry.R', import.meta.url), { redirect: 'error' });
        if (!source.ok) throw new Error('Industry audit could not be loaded');
        return new this(url, webR, await source.text(), csrfToken);
    }

    capabilities() {
        return this.request('capabilities');
    }

    invoke(profile, operation, input) {
        return this.request('invoke', { profile, operation, input });
    }

    async request(kind, input) {
        const { method, audit } = this.operations.get(kind);
        const headers = new Headers({ Accept: 'application/json' });
        if (method === 'POST') {
            headers.set('Content-Type', 'application/json');
            if (this.#csrf) headers.set('X-CSRF-Token', this.#csrf);
        }
        let success = 0;
        try {
            const response = await fetch(this.#endpoint, {
                method, headers, body: input ? JSON.stringify(input) : undefined,
                redirect: 'error', credentials: 'same-origin', cache: 'no-store',
            });
            if (!response.ok) throw new Error(`Industry request failed (HTTP ${response.status})`);
            const result = await response.json();
            success = 1;
            return result;
        } finally {
            this.#auditTail = this.#auditTail.then(async () => {
                const code = `(${this.#auditSource})(c(${this.#counts.join(',')}), ${audit}, ${success})`;
                this.#counts = await this.#webR.evalRRaw(code, 'number[]');
            }).catch(error => { this.#auditFailure ??= error; });
        }
    }

    async statistics() {
        await this.#auditTail;
        const [capabilities, operations, succeeded, failed] = this.#counts;
        return { capabilities, operations, succeeded, failed, auditComplete: !this.#auditFailure };
    }
}
