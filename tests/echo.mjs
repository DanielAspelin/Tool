import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

class EchoVerification {
    static host = '0123456789abcdef0123456789abcdef.sessions.mainframes.digital';

    static native(directory, args) {
        const result = spawnSync(resolve(directory, 'tool'), args, { cwd: directory, encoding: 'utf8' });
        if (result.error) throw result.error;
        return result;
    }

    static async verify(directory, openRoute) {
        const encoder = new TextEncoder();
        for (const [index, input] of ['', 'hello Tool', 'ää🌱', 'é'.repeat(2048)].entries()) {
            const native = this.native(directory, ['platform', 'preview', this.host, 'echo', input]);
            assert.equal(native.status, 0, native.stderr);
            assert.equal(native.stdout, input + '\n');
            const route = await openRoute(index + 1, encoder.encode(input).length);
            assert.equal(route.state(), 2);
            route.passThrough();
            route.output();
            route.exit();
            route.close();
            assert.equal(route.state(), 6);
            assert.throws(() => route.passThrough(), /rejected/);
        }

        const oversized = 'é'.repeat(2048) + 'x';
        const native = this.native(directory, ['platform', 'preview', this.host, 'echo', oversized]);
        assert.equal(native.status, 1);
        assert.match(native.stderr, /exceeds 4096 bytes/);
        await assert.rejects(openRoute(1, encoder.encode(oversized).length), RangeError);

        const joined = this.native(directory, [
            'platform', 'preview', this.host, 'echo', '', 'a'.repeat(4094), 'b',
        ]);
        assert.equal(joined.status, 0, joined.stderr);
        assert.equal(joined.stdout, 'a'.repeat(4094) + ' b\n');

        const first = await openRoute(1, 0);
        const second = await openRoute(1, 0);
        first.passThrough();
        assert.equal(second.state(), 2);
    }

    static async run(directory) {
        const wasm = await readFile(resolve(directory, 'tool-route.wasm'));
        const { instance } = await WebAssembly.instantiate(wasm);
        const { tool_javascript_size: size, tool_javascript_byte: byte } = instance.exports;
        const temporary = await mkdtemp(join(tmpdir(), 'tool-echo-'));
        const originalFetch = globalThis.fetch;
        try {
            const loader = join(temporary, 'index.mjs');
            await writeFile(loader, Uint8Array.from({ length: size() }, (_, index) => byte(index)));
            globalThis.fetch = async (url) => {
                assert.equal(url.href, pathToFileURL(join(temporary, 'tool-route.wasm')).href);
                return new Response(wasm, { headers: { 'Content-Type': 'application/wasm' } });
            };
            const { openRoute } = await import(pathToFileURL(loader).href);
            await this.verify(directory, openRoute);
        } finally {
            globalThis.fetch = originalFetch;
            await rm(temporary, { recursive: true, force: true });
        }
    }
}

await EchoVerification.run(resolve(process.argv[2]));
console.log('Native and WASM echo checks passed');
