import { spawn } from 'node:child_process';
import { createHash } from 'node:crypto';
import { createReadStream } from 'node:fs';
import { copyFile, mkdir, mkdtemp, readFile, realpath, rm, stat, statfs, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { parseArgs } from 'node:util';

class Packages {
    static #tmpfs = 0x01021994;
    static #chunkSize = 3 * 512 * 1024;
    static #prefix = 'export default "';
    static #suffix = '";\n';

    static async catalog(path) {
        const rows = JSON.parse(await readFile(path ?? new URL('./packages.json', import.meta.url), 'utf8'));
        if (!Array.isArray(rows)) throw new Error('Package catalog must be an array');
        const packages = new Map();
        for (const pkg of rows) {
            if (typeof pkg?.id !== 'string' || !/^[a-z0-9][a-z0-9.-]*$/.test(pkg.id) ||
                packages.has(pkg.id) || !['runtime', 'source', 'toolchain'].includes(pkg.kind) ||
                typeof pkg.version !== 'string' || typeof pkg.filename !== 'string' ||
                !/^[A-Za-z0-9][A-Za-z0-9._+-]*$/.test(pkg.filename) ||
                !Number.isSafeInteger(pkg.bytes) || pkg.bytes <= 0 ||
                typeof pkg.sha256 !== 'string' || !/^[a-f0-9]{64}$/.test(pkg.sha256) ||
                !Array.isArray(pkg.urls) || pkg.urls.length < 1) {
                throw new Error('Invalid or duplicate package record');
            }
            for (const value of pkg.urls) {
                if (typeof value !== 'string') throw new Error(`Invalid URL in package ${pkg.id}`);
                const url = new URL(value);
                if (url.protocol !== 'https:' || url.username || url.password) {
                    throw new Error(`Package ${pkg.id} requires HTTPS URLs without credentials`);
                }
            }
            packages.set(pkg.id, pkg);
        }
        return packages;
    }

    static encodedSize(bytes) {
        return 4 * Math.ceil(bytes / 3);
    }

    static async directory(runtime, bytes) {
        if (process.platform !== 'linux') throw new Error('Package staging requires Linux tmpfs');
        const base = await realpath(runtime ?? process.env.TOOL_BASH_RUNTIME ?? '/dev/shm');
        const filesystem = await statfs(base);
        if (filesystem.type !== this.#tmpfs) throw new Error('Package staging requires a tmpfs path');
        if (filesystem.bavail * filesystem.bsize < bytes) throw new Error('Insufficient free tmpfs space');
        return mkdtemp(join(base, 'tool-packages-'));
    }

    static process(program, args, options = {}) {
        return new Promise((resolve, reject) => {
            const child = spawn(program, args, { stdio: ['ignore', 'inherit', 'inherit'], ...options });
            child.once('error', reject);
            child.once('close', (code, signal) => {
                if (code === 0) resolve();
                else reject(new Error(`${program} ended with ${signal ?? code}`));
            });
        });
    }

    static fetch(url, target, timeout, bytes) {
        return this.process('curl', [
            '-q', '--fail', '--location', '--silent', '--show-error',
            '--proto', '=https', '--proto-redir', '=https', '--max-redirs', '5',
            '--connect-timeout', String(Math.min(timeout, 10)), '--max-time', String(timeout),
            '--max-filesize', String(bytes), '--header', 'Accept: application/octet-stream',
            '--output', target, '--url', url,
        ], { stdio: ['ignore', 'ignore', 'inherit'] });
    }

    static async verify(pkg, path) {
        if ((await stat(path)).size !== pkg.bytes) throw new Error('Archive byte count differs from pin');
        const sha = createHash('sha256');
        for await (const block of createReadStream(path)) sha.update(block);
        if (sha.digest('hex') !== pkg.sha256) throw new Error('Archive SHA-256 differs from pin');
    }

    static async archive(pkg, target, { timeout, local }) {
        const candidates = local ? [{ local }, ...pkg.urls.map(url => ({ url }))] : pkg.urls.map(url => ({ url }));
        const failures = [];
        for (const candidate of candidates) {
            try {
                if (candidate.local) {
                    const info = await stat(candidate.local);
                    if (!info.isFile() || info.size !== pkg.bytes) throw new Error('Local archive does not match the expected size');
                    await copyFile(candidate.local, target);
                } else {
                    await this.fetch(candidate.url, target, timeout, pkg.bytes);
                }
                await this.verify(pkg, target);
                return candidate.local ? 'local archive' : candidate.url;
            } catch (error) {
                await rm(target, { force: true });
                failures.push(`${candidate.url ?? 'local archive'}: ${error.message}`);
            }
        }
        throw new Error(`All download candidates failed for ${pkg.id}\n${failures.join('\n')}`);
    }

    static async embed(pkg, archive, directory) {
        const partsDirectory = join(directory, 'parts');
        await mkdir(partsDirectory, { mode: 0o700 });
        let carry = Buffer.alloc(0);
        let parts = 0;
        for await (const block of createReadStream(archive, { highWaterMark: this.#chunkSize })) {
            const bytes = carry.length ? Buffer.concat([carry, block]) : block;
            const end = bytes.length - bytes.length % 3;
            if (end > 0) await this.part(partsDirectory, parts++, bytes.subarray(0, end));
            carry = bytes.subarray(end);
        }
        if (carry.length) await this.part(partsDirectory, parts++, carry);
        const manifest = join(directory, 'payload.mjs');
        await writeFile(manifest, `import { createHash } from 'node:crypto';
import { readFile } from 'node:fs/promises';

export const packageInfo = Object.freeze(${JSON.stringify({
    id: pkg.id, filename: pkg.filename, version: pkg.version, bytes: pkg.bytes,
    base64Bytes: this.encodedSize(pkg.bytes), sha256: pkg.sha256, parts,
})});

export async function* archiveBytes() {
    const sha = createHash('sha256');
    let bytes = 0;
    for (let index = 0; index < packageInfo.parts; ++index) {
        const file = new URL(\`./parts/\${index}.mjs\`, import.meta.url);
        const part = await readFile(file, 'utf8');
        const prefix = ${JSON.stringify(this.#prefix)};
        const suffix = ${JSON.stringify(this.#suffix)};
        if (!part.startsWith(prefix) || !part.endsWith(suffix)) throw new Error('Invalid package part');
        const chunk = Buffer.from(part.slice(prefix.length, -suffix.length), 'base64');
        sha.update(chunk);
        bytes += chunk.length;
        yield chunk;
    }
    if (bytes !== packageInfo.bytes || sha.digest('hex') !== packageInfo.sha256) {
        throw new Error('Embedded archive does not match the package pin');
    }
}
`, { mode: 0o600, flag: 'wx' });
        return manifest;
    }

    static async part(directory, index, bytes) {
        await writeFile(join(directory, `${index}.mjs`),
            this.#prefix + bytes.toString('base64') + this.#suffix, { mode: 0o600, flag: 'wx' });
    }

    static async stage(pkg, options) {
        const base64Bytes = this.encodedSize(pkg.bytes);
        const partsEstimate = Math.ceil(pkg.bytes / this.#chunkSize) + 1;
        const required = pkg.bytes + (options.base64 ? base64Bytes + 8192 * partsEstimate : 0);
        const directory = await this.directory(options.runtime, required);
        const path = join(directory, pkg.filename);
        try {
            const source = await this.archive(pkg, path, options);
            const manifest = options.base64 ? await this.embed(pkg, path, directory) : undefined;
            return { id: pkg.id, version: pkg.version, bytes: pkg.bytes, base64Bytes, sha256: pkg.sha256,
                directory, archive: path, source, manifest };
        } catch (error) {
            await rm(directory, { recursive: true, force: true });
            throw error;
        }
    }

    static async build(input, options) {
        const source = await realpath(input);
        const info = await stat(source);
        if (!info.isFile()) throw new Error('Build input must be a C++ source file');
        const target = options.target ?? 'native';
        const profiles = {
            native: { compiler: 'c++', file: 'application', flags: ['-pthread'] },
            wasi: { compiler: 'clang++', file: 'application.wasm', flags: ['--target=wasm32-wasip1'] },
            browser: { compiler: 'em++', file: 'application.mjs',
                flags: ['-sMODULARIZE=1', '-sEXPORT_ES6=1', '-sENVIRONMENT=web,node'] },
        };
        if (!Object.hasOwn(profiles, target)) throw new Error('Build target must be native, wasi, or browser');
        const profile = profiles[target];
        const selected = options.compiler ?? profile.compiler;
        const compiler = selected.includes('/') ? await realpath(selected) : selected;
        const directory = await this.directory(options.runtime, info.size);
        const output = join(directory, profile.file);
        try {
            await this.process(compiler, ['-std=c++17', '-O2', ...profile.flags, source, '-o', output], {
                cwd: directory,
                env: { ...process.env, TMPDIR: directory, EM_CACHE: join(directory, 'emscripten-cache') },
            });
            const wasm = target === 'browser' ? join(directory, 'application.wasm') :
                (target === 'wasi' ? output : undefined);
            return { target, directory, output, wasm };
        } catch (error) {
            await rm(directory, { recursive: true, force: true });
            throw error;
        }
    }

    static async run() {
        const { values, positionals } = parseArgs({ options: {
            catalog: { type: 'string' }, runtime: { type: 'string' }, timeout: { type: 'string' },
            local: { type: 'string' }, target: { type: 'string' }, compiler: { type: 'string' }, base64: { type: 'boolean' }, help: { type: 'boolean', short: 'h' },
        }, allowPositionals: true });
        const [action, id] = positionals;
        if (values.help || !action) {
            console.log('node packages.mjs list [--catalog FILE]\n' +
                'node packages.mjs stage PACKAGE [--catalog FILE] [--runtime TMPFS] [--timeout SECONDS] [--base64] [--local ARCHIVE]\n' +
                'node packages.mjs build INPUT.cpp [--target native|wasi|browser] [--compiler PATH] [--runtime TMPFS]');
            return;
        }
        if (action === 'build' && positionals.length === 2) {
            console.log(JSON.stringify(await this.build(id, values), null, 2));
            return;
        }
        const catalog = await this.catalog(values.catalog);
        if (action === 'list' && positionals.length === 1) {
            const packages = Array.from(catalog.values(), pkg => ({ ...pkg, base64Bytes: this.encodedSize(pkg.bytes) }));
            const counts = { runtime: 0, source: 0, toolchain: 0 };
            for (const pkg of packages) counts[pkg.kind]++;
            console.log(JSON.stringify({ counts, packages }, null, 2));
            return;
        }
        if (action !== 'stage' || positionals.length !== 2 || !catalog.has(id)) {
            throw new Error('Expected a package ID from node packages.mjs list');
        }
        const timeout = Number(values.timeout ?? 120);
        if (!Number.isInteger(timeout) || timeout < 1 || timeout > 3600) {
            throw new Error('Download timeout must be an integer from 1 to 3600 seconds');
        }
        console.log(JSON.stringify(await this.stage(catalog.get(id), { ...values, timeout }), null, 2));
    }
}

await Packages.run().catch(error => {
    console.error(error.message);
    process.exitCode = 1;
});
