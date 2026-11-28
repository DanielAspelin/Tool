// SPDX-License-Identifier: GPL-3.0-only

export class TransmissionControl {
    operations = new Map([['request', { protocol: 'https', redirects: 'reject' }]]);

    constructor(fetch = globalThis.fetch) {
        this.fetch = fetch;
    }

    async request(origin, path, { method = 'GET', headers, body } = {}) {
        const url = new URL(path, origin);
        if (url.origin !== origin || url.username || url.password || url.protocol !== 'https:') {
            throw new TypeError('Transmission requires the registered HTTPS origin');
        }
        const response = await this.fetch(url, {
            method, headers, body, redirect: 'error',
            signal: AbortSignal.timeout(15_000),
        });
        if (!response.ok) {
            await response.body?.cancel();
            throw new Error(`Provider request failed (HTTP ${response.status})`);
        }
        return response.json();
    }
}

export class ConnectionControl {
    #routes = new Map();

    register(name, origin, transport) {
        if (this.#routes.has(name)) throw new Error('Industry connection already registered');
        const endpoint = new URL(origin);
        if (endpoint.origin !== origin || endpoint.protocol !== 'https:' ||
            endpoint.username || endpoint.password) {
            throw new TypeError('Register an exact HTTPS origin without credentials or path');
        }
        this.#routes.set(name, { origin, transport });
    }

    open(name) {
        const route = this.#routes.get(name);
        if (!route) throw new Error('Industry connection is unavailable');
        return (path, options) => route.transport.request(route.origin, path, options);
    }
}

export class CommunicationControl {
    #profiles = new Map();

    register(profile, operations) {
        if (this.#profiles.has(profile)) throw new Error('Industry profile already registered');
        this.#profiles.set(profile, new Map(Object.entries(operations)));
    }

    capabilities() {
        return {
            schema: 'tool.industry.v1',
            profiles: Array.from(this.#profiles, ([id, operations]) => ({
                id, operations: Array.from(operations.keys()),
            })),
        };
    }

    invoke({ profile, operation, input }) {
        const handler = this.#profiles.get(profile)?.get(operation);
        if (!handler) throw new Error('Industry operation is unavailable');
        return handler(input);
    }
}
