// SPDX-License-Identifier: GPL-3.0-only

class NFCAudit {
    #operations = new Map([
        ['status', { index: 1 }],
        ['capabilities', { index: 2 }],
        ['scan', { index: 3 }],
        ['read', { index: 4 }],
        ['error', { index: 5 }],
        ['stop', { index: 6 }],
    ]);
    #totals = Array(8).fill(0);
    #pending = Promise.resolve();
    #failure;

    constructor(webR, source) {
        this.webR = webR;
        this.source = source;
    }

    record(operation, records = 0, bytes = 0) {
        const { index } = this.#operations.get(operation);
        const update = this.#pending.then(async () => {
            const code = `(${this.source})(c(${this.#totals.join(',')}), ${index}, ${records}, ${bytes})`;
            this.#totals = await this.webR.evalRRaw(code, 'number[]');
        });
        this.#pending = update.catch(error => { this.#failure ??= error; });
        return update;
    }

    async summary() {
        await this.#pending;
        if (this.#failure) throw new Error("NFC audit is incomplete", { cause: this.#failure });
        const counts = Array.from(this.#operations, ([operation, { index }]) => [
            operation, this.#totals[index - 1],
        ]);
        return { ...Object.fromEntries(counts), records: this.#totals[6], bytes: this.#totals[7] };
    }
}

class WebNFCAdapter {
    operations = new Map([
        ['capabilities', { format: 'tool.nfc.v1' }],
        ['scan', { permission: 'user gesture' }],
        ['read', { maxRecords: 32, maxBytes: 4096 }],
    ]);

    constructor(platform) {
        this.platform = platform;
    }

    capabilities() {
        const { isSecureContext, top, document, NDEFReader } = this.platform;
        const available = isSecureContext === true && top === this.platform &&
            Boolean(document) && typeof NDEFReader === 'function';
        return {
            schema: 'tool.nfc.v1', adapter: 'web-nfc',
            state: available ? 'permission-required' : 'unavailable', reader: 'unknown',
            operations: available ? ['status', 'capabilities', 'scan', 'stop', 'statistics']
                : ['status', 'capabilities', 'statistics'],
        };
    }

    scan(signal, receive, fail) {
        const reader = new this.platform.NDEFReader();
        reader.addEventListener('reading', event => receive(event.message.records), { signal });
        reader.addEventListener('readingerror', () => fail(
            new DOMException('The tag could not be read as NDEF', 'NotReadableError')
        ), { signal });
        return reader.scan({ signal });
    }

    read(records) {
        const { maxRecords, maxBytes } = this.operations.get('read');
        if (records.length > maxRecords) throw new RangeError('Too many NDEF records');

        let bytes = 0;
        const message = records.map(({ recordType, mediaType, encoding, lang, data }) => {
            bytes += data?.byteLength ?? 0;
            if (bytes > maxBytes) throw new RangeError('NDEF payload exceeds 4096 bytes');
            const payload = data
                ? new Uint8Array(data.buffer, data.byteOffset, data.byteLength).slice()
                : new Uint8Array();
            return { recordType, mediaType, encoding, lang, payload };
        });
        return { records: message, bytes };
    }
}

class NFCController {
    operations = new Map([
        ['scan', { timeoutMs: 30_000 }],
        ['stop', { effect: 'abort active scan' }],
    ]);
    #active;

    constructor(adapter, audit) {
        this.adapter = adapter;
        this.audit = audit;
    }

    async scan() {
        try {
            if (this.#active) throw new DOMException('An NFC scan is active', 'InvalidStateError');
            if (this.adapter.capabilities().state === 'unavailable') {
                throw new DOMException('Web NFC is unavailable in this context', 'NotSupportedError');
            }
            const message = await this.start();
            await this.audit.record('read', message.records.length, message.bytes);
            return { records: message.records };
        } catch (error) {
            await this.audit.record('error').catch(() => {});
            throw error;
        }
    }

    start() {
        const session = new AbortController();
        return new Promise((resolve, reject) => {
            const finish = (settle, value) => {
                if (this.#active?.session !== session) return;
                clearTimeout(timer);
                this.#active = undefined;
                session.abort();
                settle(value);
            };
            const fail = error => finish(reject, error);
            const receive = records => {
                try {
                    finish(resolve, this.adapter.read(records));
                } catch (error) {
                    fail(error);
                }
            };
            const timer = setTimeout(() => fail(new DOMException(
                'NFC scan timed out', 'TimeoutError'
            )), this.operations.get('scan').timeoutMs);
            this.#active = { session, fail };

            // Invoke NDEFReader before awaiting R to preserve the browser user gesture.
            this.audit.record('scan').catch(fail);
            try {
                this.adapter.scan(session.signal, receive, fail).catch(fail);
            } catch (error) {
                fail(error);
            }
        });
    }

    async stop() {
        const active = this.#active;
        active?.fail(new DOMException('NFC scan stopped', 'AbortError'));
        await this.audit.record('stop');
        return Boolean(active);
    }
}

export class NFCSystem {
    operations = new Map([
        ['status', { result: 'capability envelope' }],
        ['capabilities', { result: 'capability envelope' }],
        ['scan', { controller: 'NFCController' }],
        ['stop', { controller: 'NFCController' }],
        ['statistics', { runtime: 'R' }],
    ]);

    static async create(webR, platform = globalThis) {
        const source = await platform.fetch(new URL('./nfc.R', import.meta.url));
        if (!source.ok) throw new Error(`Could not load Tool NFC audit (${source.status})`);
        return new this(new WebNFCAdapter(platform), new NFCAudit(webR, await source.text()));
    }

    constructor(adapter, audit) {
        this.adapter = adapter;
        this.audit = audit;
        this.controller = new NFCController(adapter, audit);
    }

    async status() {
        await this.audit.record('status');
        return this.adapter.capabilities();
    }

    async capabilities() {
        await this.audit.record('capabilities');
        return this.adapter.capabilities();
    }

    scan() {
        return this.controller.scan();
    }

    stop() {
        return this.controller.stop();
    }

    statistics() {
        return this.audit.summary();
    }
}
