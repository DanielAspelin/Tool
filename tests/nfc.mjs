// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { readFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

class Reader extends EventTarget {
    static current;
    static denial;

    constructor() {
        super();
        Reader.current = this;
    }

    async scan({ signal }) {
        this.signal = signal;
        if (Reader.denial) throw new DOMException('Permission denied', 'NotAllowedError');
    }

    read(records) {
        this.dispatchEvent(Object.assign(new Event('reading'), { message: { records } }));
    }
}

class NFCVerification {
    static mockR = {
        async evalRRaw(code, type) {
            assert.equal(type, 'number[]');
            const args = code.match(/\)\(c\(([0-9,]+)\), (\d+), (\d+), (\d+)\)$/);
            assert.ok(args);
            const totals = args[1].split(',').map(Number);
            totals[Number(args[2]) - 1]++;
            totals[6] += Number(args[3]);
            totals[7] += Number(args[4]);
            return totals;
        },
    };

    static platform(secure = true) {
        const window = {
            NDEFReader: Reader, isSecureContext: secure, document: {},
            async fetch(url) { return new Response(await readFile(fileURLToPath(url))); },
        };
        window.top = window;
        return window;
    }

    static native(directory) {
        const tool = resolve(directory, 'tool');
        const invoke = (...args) => execFileSync(tool, args, { encoding: 'utf8' }).trim();
        const capabilities = JSON.parse(invoke('nfc', 'capabilities'));
        assert.deepEqual(capabilities, JSON.parse(invoke('nfc.capabilities')));
        assert.equal(capabilities.schema, 'tool.nfc.v1');
        assert.equal(capabilities.adapter, 'native');
        assert.equal(capabilities.state, 'metadata-only');
        assert.deepEqual(capabilities.operations, ['status', 'capabilities']);
        assert.match(capabilities.reader, /^(present|not-detected|unknown)$/);
        assert.equal(invoke('nfc'), invoke('nfc.status'));
        assert.match(invoke('nfc', 'scan'), /^Usage: nfc/);
    }

    static async browser(directory, webR) {
        const { NFCSystem } = await import(pathToFileURL(resolve(directory, 'nfc.mjs')));
        const nfc = await NFCSystem.create(webR, this.platform());
        assert.equal((await nfc.status()).state, 'permission-required');
        assert.equal((await nfc.capabilities()).schema, 'tool.nfc.v1');

        const pending = nfc.scan();
        const reader = Reader.current;
        await assert.rejects(nfc.scan(), { name: 'InvalidStateError' });
        const raw = Uint8Array.of(0, 97, 98, 0);
        reader.read([{ recordType: 'text', encoding: 'utf-8', data: new DataView(raw.buffer, 1, 2) }]);
        const result = await pending;
        raw[1] = 99;
        assert.deepEqual(result.records[0].payload, Uint8Array.of(97, 98));
        assert.equal(reader.signal.aborted, true);

        const stopping = nfc.scan();
        const stopped = assert.rejects(stopping, { name: 'AbortError' });
        assert.equal(await nfc.stop(), true);
        await stopped;
        assert.equal(await nfc.stop(), false);

        const oversized = nfc.scan();
        Reader.current.read([{ data: new DataView(new ArrayBuffer(4097)) }]);
        await assert.rejects(oversized, RangeError);

        Reader.denial = true;
        await assert.rejects(nfc.scan(), { name: 'NotAllowedError' });
        Reader.denial = false;

        assert.deepEqual(await nfc.statistics(), {
            status: 1, capabilities: 1, scan: 4, read: 1, error: 4, stop: 2, records: 1, bytes: 2,
        });

        const prior = Reader.current;
        const unavailable = await NFCSystem.create(webR, this.platform(false));
        assert.equal((await unavailable.capabilities()).state, 'unavailable');
        await assert.rejects(unavailable.scan(), { name: 'NotSupportedError' });
        assert.equal(Reader.current, prior);
    }

    static async run(directory, webRModule) {
        this.native(directory);
        let webR;
        try {
            if (webRModule) {
                const { WebR } = await import(pathToFileURL(resolve(webRModule)));
                webR = new WebR();
                await webR.init();
            }
            await this.browser(directory, webR ?? this.mockR);
        } finally {
            await webR?.close();
        }
        console.log(`NFC contract checks passed (${webR ? 'webR' : 'simulated R transport'})`);
    }
}

await NFCVerification.run(process.argv[2], process.env.TOOL_TEST_WEBR);
