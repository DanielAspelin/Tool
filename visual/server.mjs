// SPDX-License-Identifier: GPL-3.0-only
import { timingSafeEqual, randomBytes } from 'node:crypto';
import { createReadStream } from 'node:fs';
import { realpath, stat } from 'node:fs/promises';
import { createServer } from 'node:http';
import { basename, extname, join, resolve, sep } from 'node:path';
import { pipeline } from 'node:stream/promises';
import { fileURLToPath } from 'node:url';
import { VisualPool } from './pool.mjs';

export class VisualServer {
    #pools = new Map();
    #token = randomBytes(32).toString('hex');
    #server;
    #origin;
    #webr;
    #closing;
    operations = new Map([['state', { method: 'GET' }], ['pool', { method: 'POST' }],
        ['regulate', { method: 'POST' }]]);

    async start({ port = 0, webrDist, sockets = 4 } = {}) {
        if (webrDist) this.#webr = await realpath(webrDist);
        await this.add('primary', sockets);
        this.#server = createServer((request, response) => {
            this.route(request, response).catch(() => {
                if (!response.headersSent) this.reply(response, 400, { error: 'Request could not be completed' });
                else response.destroy();
            });
        });
        try {
            await new Promise((ok, fail) => {
                this.#server.once('error', fail);
                this.#server.listen(port, '127.0.0.1', ok);
            });
        } catch (error) {
            await this.close();
            throw error;
        }
        this.#origin = `http://127.0.0.1:${this.#server.address().port}`;
        return this.#origin;
    }

    async add(id, sockets) {
        if (this.#closing || this.#pools.has(id) || this.#pools.size >= 4) {
            throw new Error('Visual system cannot be added');
        }
        const pool = new VisualPool(id, sockets);
        const ready = pool.start();
        this.#pools.set(id, { pool, ready });
        try {
            await ready;
        } catch (error) {
            this.#pools.delete(id);
            throw error;
        }
    }

    authorized(request) {
        const candidate = request.headers['x-tool-token'];
        return request.headers.origin === this.#origin && typeof candidate === 'string' &&
            candidate.length === this.#token.length &&
            timingSafeEqual(Buffer.from(candidate), Buffer.from(this.#token));
    }

    async route(request, response) {
        if (request.headers.host !== new URL(this.#origin).host) return this.reply(response, 403, {});
        const path = request.url;
        if (request.method === 'GET' && path === '/api/state') {
            return this.reply(response, 200, { token: this.#token, runtime: Boolean(this.#webr),
                systems: Array.from(this.#pools.values(), ({ pool }) => pool.snapshot()) });
        }
        if (request.method === 'GET' || request.method === 'HEAD') {
            return this.asset(path.split('?')[0], request, response);
        }
        if (request.method !== 'POST' || !this.authorized(request)) return this.reply(response, 403, {});
        if (!['/api/pool', '/api/regulate'].includes(path) ||
            request.headers['content-type'] !== 'application/json') return this.reply(response, 404, {});
        let body = '';
        for await (const chunk of request) {
            body += chunk;
            if (body.length > 4096) return this.reply(response, 413, {});
        }
        const input = JSON.parse(body);
        if (path === '/api/pool') await this.add(input.system, input.sockets);
        else {
            const entry = this.#pools.get(input.system);
            if (!entry) return this.reply(response, 404, {});
            await entry.ready;
            entry.pool.regulate(input.service, input);
        }
        return this.reply(response, 200, { ok: true });
    }

    async asset(path, request, response) {
        const names = new Set(['index.html', 'index.js', 'index.R']);
        const type = new Map([['.html', 'text/html; charset=utf-8'], ['.js', 'text/javascript'],
            ['.mjs', 'text/javascript'], ['.R', 'text/plain'], ['.wasm', 'application/wasm'], ['.json', 'application/json']]);
        let file;
        if (path === '/' || names.has(path.slice(1))) {
            file = fileURLToPath(new URL(path === '/' ? './index.html' : './' + basename(path), import.meta.url));
        } else if (this.#webr && /^\/webr\/[-A-Za-z0-9_.]+(?:\/[-A-Za-z0-9_.]+)*$/.test(path)) {
            try {
                file = await realpath(join(this.#webr, path.slice(6)));
                if (!file.startsWith(this.#webr + sep)) return this.reply(response, 404, {});
            } catch {
                return this.reply(response, 404, {});
            }
        } else return this.reply(response, 404, {});
        const info = await stat(file);
        if (!info.isFile()) return this.reply(response, 404, {});
        const size = info.size;
        const range = request.headers.range;
        const headers = { 'Content-Type': type.get(extname(file)) ?? 'application/octet-stream',
            'Content-Length': size, 'Accept-Ranges': 'bytes',
            'Cross-Origin-Opener-Policy': 'same-origin', 'Cross-Origin-Embedder-Policy': 'require-corp',
            'X-Content-Type-Options': 'nosniff', 'Content-Security-Policy':
                "default-src 'self'; script-src 'self' 'wasm-unsafe-eval' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; worker-src 'self' blob:; frame-ancestors 'none'" };
        let options;
        if (range !== undefined) {
            const match = /^bytes=(\d+)-(\d*)$/.exec(range);
            const start = match ? Number(match[1]) : size;
            const end = Math.min(match?.[2] ? Number(match[2]) : size - 1, size - 1);
            if (start >= size || end < start) {
                response.writeHead(416, { 'Content-Range': `bytes */${size}` });
                response.end();
                return;
            }
            options = { start, end };
            headers['Content-Range'] = `bytes ${start}-${end}/${size}`;
            headers['Content-Length'] = end - start + 1;
        }
        response.writeHead(options ? 206 : 200, headers);
        if (request.method === 'HEAD') response.end();
        else await pipeline(createReadStream(file, options), response);
    }

    reply(response, code, data) {
        response.writeHead(code, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' });
        response.end(JSON.stringify(data));
    }

    async close() {
        if (this.#closing) return this.#closing;
        this.#closing = Promise.all([
            this.#server?.listening ? new Promise(ok => this.#server.close(ok)) : undefined,
            ...Array.from(this.#pools.values(), async ({ pool, ready }) => {
                await ready.catch(() => {});
                await pool.close();
            }),
        ]);
        await this.#closing;
    }
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
    const server = new VisualServer();
    const url = await server.start({ port: Number(process.env.TOOL_VISUAL_PORT || 4780),
        webrDist: process.env.TOOL_WEBR_DIST });
    console.log(`Tool Visual Control · ${url}`);
    for (const signal of ['SIGTERM', 'SIGINT']) process.once(signal, () => server.close());
}
