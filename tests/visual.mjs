// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { mkdtemp, rm, symlink, writeFile } from 'node:fs/promises';
import { join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { setTimeout } from 'node:timers/promises';
import { PlatformSystem } from '../platform/system.mjs';
import { VisualServer } from '../visual/server.mjs';

class VisualPlatformVerification {
    static async wait(read, predicate) {
        for (let i = 0; i < 30; i++) {
            const value = await read();
            if (predicate(value)) return value;
            await setTimeout(100);
        }
        assert.fail('Background state did not arrive within the test window');
    }

    static async visual() {
        const server = new VisualServer();
        try {
            const origin = await server.start();
            const state = async () => (await fetch(origin + '/api/state')).json();
            const start = await this.wait(state, report => report.systems[0].services.every(s => s.completed > 0));
            assert.equal(start.systems[0].slots.length, 4);
            const pulse = start.systems[0].slots[0].lastPulse;
            const headers = { origin, 'content-type': 'application/json', 'x-tool-token': start.token };
            const post = (path, payload, custom = headers) => fetch(origin + '/api/' + path, {
                method: 'POST', headers: custom, body: JSON.stringify(payload),
            });
            assert.equal((await post('pool', { system: 'two', sockets: 2 }, { ...headers, origin: 'https://other.test' })).status, 403);
            assert.equal((await post('pool', { system: 'bad', sockets: 5 })).status, 400);
            assert.equal((await post('pool', { system: 'two', sockets: 2 })).status, 200);
            assert.equal((await post('pool', { system: 'three', sockets: 3 })).status, 200);
            assert.deepEqual((await state()).systems.map(system => system.slots.length), [4, 2, 3]);
            assert.equal((await post('regulate', {
                system: 'primary', service: 'geometry', enabled: false, limit: 1,
            })).status, 200);
            const paused = await this.wait(state, report => {
                const system = report.systems[0];
                return !system.services[0].enabled && system.slots[0].state === 'idle' &&
                    system.slots[0].lastPulse > pulse;
            });
            await setTimeout(350);
            const later = (await state()).systems[0];
            assert.equal(later.services[0].completed, paused.systems[0].services[0].completed);
            assert.ok(later.services[1].completed > paused.systems[0].services[1].completed);
            assert.match(await (await fetch(origin + '/')).text(), /index\.js/);
            assert.match(await (await fetch(origin + '/index.R')).text(), /function\(slots/);
            const head = await fetch(origin + '/index.R', { method: 'HEAD' });
            assert.equal(head.status, 200);
            assert.ok(Number(head.headers.get('content-length')) > 40);
            const partial = await fetch(origin + '/index.R', { headers: { range: 'bytes=0-2' } });
            assert.equal(partial.status, 206);
            assert.equal(await partial.text(), '# S');
        } finally {
            await server.close();
        }
    }

    static async platform(directory) {
        const native = execFileSync(resolve(directory, 'tool'), ['isoiec.reference'], { encoding: 'utf8' });
        assert.match(native, /^ISOIEC:/);
        const root = await mkdtemp(join(tmpdir(), 'tool-media-'));
        try {
            const system = new PlatformSystem({ root, platform: 'linux', deviceAccess: async () => {},
                exec: async (file, args) => {
                    assert.equal(file, 'virsh');
                    assert.ok(args.includes('--readonly'));
                    return { stdout: 'guest-1\n' };
                } });
            assert.deepEqual((await system.kvm()).names, ['guest-1']);
            assert.equal((await system.hyperv()).state, 'unsupported');
            const windows = new PlatformSystem({ platform: 'win32', exec: async (file, args) => {
                assert.equal(file, 'powershell.exe');
                assert.match(args.at(-1), /Get-VM/);
                return { stdout: '{"Name":"guest-2","State":"Off"}' };
            } });
            assert.deepEqual((await windows.hyperv()).machines, [{ Name: 'guest-2', State: 'Off' }]);
            await writeFile(join(root, 'fixture.bin'), Buffer.from('7f454c46', 'hex'));
            assert.equal((await system.bin('fixture.bin')).format, 'ELF');
            await writeFile(join(root, 'empty.bin'), '');
            assert.match((await system.bin('empty.bin')).sha256, /^e3b0c442/);
            const iso = Buffer.alloc(18 * 2048);
            const start = 16 * 2048;
            iso[start] = 1;
            iso.write('CD001', start + 1);
            iso[start + 6] = 1;
            iso.write('TOOL_MEDIA', start + 40);
            iso.writeUInt32LE(18, start + 80);
            iso.writeUInt32BE(18, start + 84);
            iso.writeUInt16LE(2048, start + 128);
            iso.writeUInt16BE(2048, start + 130);
            iso[start + 2048] = 255;
            iso.write('CD001', start + 2049);
            iso[start + 2054] = 1;
            await writeFile(join(root, 'fixture.iso'), iso);
            assert.equal((await system.iso('fixture.iso')).format, 'ISO 9660');
            await writeFile(join(root, 'truncated.iso'), iso.subarray(0, 17 * 2048));
            assert.equal((await system.iso('truncated.iso')).format, 'unrecognized');
            await symlink('/etc/passwd', join(root, 'escape.bin'));
            await assert.rejects(system.bin('escape.bin'), /outside/);
            assert.equal(system.isoiec().identifier, 'ISOIEC');
        } finally {
            await rm(root, { recursive: true, force: true });
        }
    }

    static async run(directory) {
        await this.visual();
        await this.platform(directory);
        console.log('Vector worker pools and virtualization/image contracts passed');
    }
}

await VisualPlatformVerification.run(process.argv[2]);
