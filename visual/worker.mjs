// SPDX-License-Identifier: GPL-3.0-only
import { createConnection } from 'node:net';
import { DuplexChannel } from './channel.mjs';

class VisualWorker {
    operations = new Map([
        ['geometry', { operation: 'geometry' }],
        ['telemetry', { operation: 'telemetry' }],
    ]);
    #pulse;
    #channel;

    constructor(port, service) {
        const socket = createConnection({ host: '127.0.0.1', port });
        const started = Date.now();
        this.#channel = new DuplexChannel(socket, frame => this.receive(frame, service),
            () => clearInterval(this.#pulse));
        socket.once('connect', () => {
            this.#channel.send({ type: 'ready', key: process.env.TOOL_VISUAL_WORKER_KEY });
            delete process.env.TOOL_VISUAL_WORKER_KEY;
            this.#pulse = setInterval(() => {
                this.#channel.send({ type: 'pulse', uptime: Date.now() - started });
            }, 500);
            this.#pulse.unref();
        });
    }

    receive({ type, id, tick }, service) {
        if (type !== 'job' || !Number.isSafeInteger(id) || !Number.isSafeInteger(tick) ||
            !this.operations.has(service)) {
            this.#channel.close();
            return;
        }
        const result = this[service](tick);
        this.#channel.send({ type: 'result', id, result });
    }

    geometry(tick) {
        const points = Array.from({ length: 48 }, (_, i) => {
            const x = i / 47;
            return [x, 0.50 + 0.18 * Math.sin(x * 10 - tick / 8) + 0.08 * Math.cos(x * 22 + tick / 16)];
        });
        return { points };
    }

    telemetry() {
        return { rssKiB: Math.round(process.memoryUsage().rss / 1024) };
    }
}

new VisualWorker(Number(process.argv[2]), process.argv[3]);
