// SPDX-License-Identifier: GPL-3.0-only
import { spawn } from 'node:child_process';
import { once } from 'node:events';
import { randomBytes } from 'node:crypto';
import { createServer } from 'node:net';
import { fileURLToPath } from 'node:url';
import { DuplexChannel } from './channel.mjs';

class WorkerSlot {
    operations = new Map([['start', { transport: 'authenticated loopback socket' }], ['run', { position: 'service pool' }]]);
    state = 'starting';
    completed = 0;
    lastPulse = 0;
    #id = 0;
    #listener;
    #workerKey = randomBytes(32).toString('hex');
    #child;
    #channel;
    #job;
    #resolveReady;
    #rejectReady;
    #exit;
    #closing;

    constructor(id, service) {
        this.id = id;
        this.service = service;
    }

    async start() {
        const ready = new Promise((resolve, reject) => {
            this.#resolveReady = resolve;
            this.#rejectReady = reject;
        });
        // Transport failures can occur before the startup await is reached.
        ready.catch(() => {});
        this.#listener = createServer(socket => {
            if (this.#channel || this.#closing) {
                socket.destroy();
                return;
            }
            this.#channel = new DuplexChannel(socket, frame => this.receive(frame),
                () => this.fail('Worker socket closed'));
            this.#listener.close();
        });
        this.#listener.on('error', () => this.fail('Worker listener failed'));
        this.#listener.listen(0, '127.0.0.1');
        await once(this.#listener, 'listening');
        this.#child = spawn(process.execPath, [
            fileURLToPath(new URL('./worker.mjs', import.meta.url)),
            String(this.#listener.address().port), this.service,
        ], { detached: true, stdio: 'ignore', env: { TOOL_VISUAL_WORKER_KEY: this.#workerKey } });
        this.#child.on('error', () => this.fail('Worker could not start'));
        this.#child.once('exit', () => this.fail('Worker process exited'));
        this.#exit = once(this.#child, 'close').catch(() => {});
        const timeout = setTimeout(() => this.fail('Worker startup timed out'), 5000);
        try {
            await ready;
        } finally {
            clearTimeout(timeout);
        }
    }

    receive(frame) {
        if (this.state === 'starting') {
            if (frame.type !== 'ready' || frame.key !== this.#workerKey) {
                this.fail('Worker authentication failed');
                return;
            }
            this.state = 'idle';
            this.lastPulse = Date.now();
            this.#resolveReady();
            return;
        }
        if (frame.type === 'pulse') {
            this.lastPulse = Date.now();
            return;
        }
        if (frame.type !== 'result' || this.#job?.id !== frame.id) {
            this.fail('Unexpected worker response');
            return;
        }
        const { resolve, timeout } = this.#job;
        clearTimeout(timeout);
        this.#job = undefined;
        this.state = 'idle';
        this.completed++;
        resolve(frame.result);
    }

    run(tick) {
        if (this.state !== 'idle') return Promise.reject(new Error('Worker is unavailable'));
        this.state = 'active';
        return new Promise((resolve, reject) => {
            const id = ++this.#id;
            const timeout = setTimeout(() => this.fail('Worker operation timed out'), 2000);
            this.#job = { id, timeout, resolve, reject };
            try {
                this.#channel.send({ type: 'job', id, tick });
            } catch {
                this.fail('Worker dispatch failed');
            }
        });
    }

    fail(reason) {
        if (this.state === 'failed' || this.state === 'stopped') return;
        this.state = this.#closing ? 'stopped' : 'failed';
        this.#rejectReady(new Error(reason));
        if (this.#job) {
            clearTimeout(this.#job.timeout);
            this.#job.reject(new Error(reason));
            this.#job = undefined;
        }
        this.#channel?.close();
        if (this.#listener?.listening) this.#listener.close();
        this.#child?.kill();
    }

    async close() {
        this.#closing = true;
        this.fail('Worker stopped');
        if (this.#exit) await this.#exit;
        this.state = 'stopped';
    }

    snapshot() {
        return { id: this.id, service: this.service, state: this.state,
            completed: this.completed, lastPulse: this.lastPulse };
    }
}

class RegulatedService {
    #slots;
    enabled = true;
    limit;
    completed = 0;
    failed = 0;
    result;
    lastTick = -1;
    regulations = new Map([['enabled', { phase: 'dispatch' }], ['limit', { phase: 'concurrency' }]]);

    constructor(id, slots) {
        this.id = id;
        this.#slots = slots;
        this.limit = slots.length;
    }

    regulate({ enabled, limit }) {
        if (typeof enabled !== 'boolean' || !Number.isInteger(limit) || limit < 1 ||
            limit > this.#slots.length) throw new RangeError('Invalid service regulation');
        this.enabled = enabled;
        this.limit = limit;
        return this.snapshot();
    }

    dispatch(tick) {
        if (!this.enabled) return;
        const active = this.#slots.filter(slot => slot.state === 'active').length;
        const idle = this.#slots.filter(slot => slot.state === 'idle');
        for (const slot of idle.slice(0, Math.max(0, this.limit - active))) {
            slot.run(tick).then(result => {
                this.completed++;
                if (tick >= this.lastTick) {
                    this.lastTick = tick;
                    this.result = result;
                }
            }).catch(() => this.failed++);
        }
    }

    snapshot() {
        return { id: this.id, enabled: this.enabled, limit: this.limit, capacity: this.#slots.length,
            completed: this.completed, failed: this.failed, result: this.result };
    }
}

export class VisualPool {
    #started = false;
    #slots = [];
    #services = new Map();
    #timer;
    #tick = 0;
    #closed = false;
    operations = new Map([['start', { min: 2, max: 4 }], ['regulate', { controlsPerService: 2 }]]);

    constructor(id, width) {
        if (typeof id !== 'string' || !/^[a-z][a-z0-9-]{0,23}$/.test(id) ||
            !Number.isInteger(width) || width < 2 || width > 4) throw new RangeError('Invalid system or socket count');
        this.id = id;
        this.width = width;
    }

    async start() {
        if (this.#started || this.#closed) throw new Error('Visual pool cannot be started');
        this.#started = true;
        this.#slots = Array.from({ length: this.width }, (_, index) =>
            new WorkerSlot(index + 1, index % 2 ? 'telemetry' : 'geometry'));
        try {
            const outcomes = await Promise.allSettled(this.#slots.map(slot => slot.start()));
            const failed = outcomes.find(outcome => outcome.status === 'rejected');
            if (failed) throw failed.reason;
            for (const service of ['geometry', 'telemetry']) {
                this.#services.set(service, new RegulatedService(service,
                    this.#slots.filter(slot => slot.service === service)));
            }
            this.#timer = setInterval(() => this.dispatch(), 250);
            this.dispatch();
        } catch (error) {
            await this.close();
            throw error;
        }
        return this;
    }

    dispatch() {
        this.#tick++;
        for (const slot of this.#slots) {
            if (slot.state === 'idle' && Date.now() - slot.lastPulse > 2500) {
                slot.fail('Worker heartbeat expired');
            }
        }
        for (const service of this.#services.values()) service.dispatch(this.#tick);
    }

    regulate(service, values) {
        const controller = this.#services.get(service);
        if (!controller) throw new Error('Unknown visual service');
        return controller.regulate(values);
    }

    snapshot() {
        return { id: this.id, sockets: this.width,
            slots: this.#slots.map(slot => slot.snapshot()),
            services: Array.from(this.#services.values(), service => service.snapshot()) };
    }

    async close() {
        this.#closed = true;
        clearInterval(this.#timer);
        await Promise.all(this.#slots.map(slot => slot.close()));
    }
}
