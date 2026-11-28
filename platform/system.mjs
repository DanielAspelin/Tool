// SPDX-License-Identifier: GPL-3.0-only
import { execFile } from 'node:child_process';
import { constants } from 'node:fs';
import { access, open, realpath } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { basename, isAbsolute, relative, resolve, sep } from 'node:path';
import { promisify } from 'node:util';

export class PlatformSystem {
    operations = new Map([
        ['kvm', { host: 'linux' }], ['hyperv', { host: 'win32' }],
        ['bin', { effect: 'read file' }], ['iso', { effect: 'read image descriptors' }],
        ['isoiec', { identifier: 'ISOIEC' }],
    ]);

    constructor({ root, exec = promisify(execFile), platform = process.platform, deviceAccess = access } = {}) {
        this.root = root;
        this.exec = exec;
        this.platform = platform;
        this.deviceAccess = deviceAccess;
    }

    register(communications) {
        communications.register('isoiec', { reference: () => this.isoiec() });
        communications.register('kvm', { status: () => this.kvm() });
        communications.register('hyperv', { status: () => this.hyperv() });
        if (this.root) {
            communications.register('bin', { inspect: ({ name }) => this.bin(name) });
            communications.register('iso', { inspect: ({ name }) => this.iso(name) });
        }
    }

    isoiec() {
        return { identifier: 'ISOIEC', formalName: 'ISO/IEC',
            isoImageSpecification: 'ISO/IEC 9660:2023 / ECMA-119' };
    }

    async kvm() {
        if (this.platform !== 'linux') return { adapter: 'kvm', state: 'unsupported' };
        let device;
        try {
            await this.deviceAccess('/dev/kvm', constants.R_OK | constants.W_OK);
            device = 'accessible';
        } catch (error) {
            device = error.code === 'ENOENT' ? 'absent' : 'unavailable';
        }
        if (device === 'absent') return { adapter: 'kvm', state: 'unavailable', device };
        try {
            const { stdout } = await this.exec('virsh',
                ['--readonly', '--connect', 'qemu:///system', 'list', '--all', '--name'],
                { timeout: 5000, maxBuffer: 256 * 1024 });
            const names = stdout.split(/\r?\n/).map(line => line.trim()).filter(Boolean);
            return { adapter: 'kvm', state: 'inventory-readable', device, names };
        } catch {
            return { adapter: 'kvm', state: 'inventory-unavailable', device };
        }
    }

    async hyperv() {
        if (this.platform !== 'win32') return { adapter: 'hyperv', state: 'unsupported' };
        const script = '$ErrorActionPreference="Stop"; Get-VM | Select-Object Name, @{n="State";e={$_.State.ToString()}} | ConvertTo-Json -Compress';
        try {
            const { stdout } = await this.exec('powershell.exe',
                ['-NoProfile', '-NonInteractive', '-Command', script],
                { timeout: 6000, maxBuffer: 256 * 1024, windowsHide: true });
            const parsed = stdout.trim() ? JSON.parse(stdout) : [];
            return { adapter: 'hyperv', state: 'inventory-readable', machines: [].concat(parsed) };
        } catch {
            return { adapter: 'hyperv', state: 'inventory-unavailable' };
        }
    }

    async file(name, inspect) {
        if (!this.root || typeof name !== 'string' || !name || isAbsolute(name) ||
            name.includes('\\') || name.includes('\0') || name.split('/').includes('..')) {
            throw new TypeError('Expected an approved root-relative file name');
        }
        const root = await realpath(this.root);
        const path = await realpath(resolve(root, name));
        const rel = relative(root, path);
        if (!rel || isAbsolute(rel) || rel === '..' || rel.startsWith('..' + sep)) {
            throw new Error('File is outside the configured image root');
        }
        const handle = await open(path, constants.O_RDONLY | (constants.O_NOFOLLOW ?? 0) |
            (constants.O_NONBLOCK ?? 0));
        try {
            const stat = await handle.stat();
            if (!stat.isFile()) throw new TypeError('Expected a regular file');
            return { name: basename(path), bytes: stat.size, ...await inspect(handle, stat.size) };
        } finally {
            await handle.close();
        }
    }

    bin(name) {
        return this.file(name, async (handle, size) => {
            const header = Buffer.alloc(4);
            await handle.read(header, 0, 4, 0);
            const signature = header.toString('hex');
            const format = signature === '7f454c46' ? 'ELF' : signature === '0061736d'
                ? 'WebAssembly' : signature.startsWith('4d5a') ? 'DOS MZ header' : 'opaque';
            const sha256 = createHash('sha256');
            if (size) {
                for await (const chunk of handle.createReadStream({ start: 0, end: size - 1, autoClose: false })) {
                    sha256.update(chunk);
                }
            }
            return { format, sha256: sha256.digest('hex') };
        });
    }

    iso(name) {
        return this.file(name, async (handle, size) => {
            const descriptor = Buffer.alloc(2048);
            let volume;
            let terminated = false;
            for (let sector = 16; sector < 144 && (sector + 1) * 2048 <= size; sector++) {
                const { bytesRead } = await handle.read(descriptor, 0, 2048, sector * 2048);
                if (bytesRead !== 2048 || descriptor.toString('ascii', 1, 6) !== 'CD001' ||
                    descriptor[6] !== 1) break;
                if (descriptor[0] === 255) {
                    terminated = true;
                    break;
                }
                if (descriptor[0] === 1 && !volume) {
                    const blocks = descriptor.readUInt32LE(80);
                    const blockSize = descriptor.readUInt16LE(128);
                    if (blocks !== descriptor.readUInt32BE(84) || blockSize !== descriptor.readUInt16BE(130) ||
                        !blocks || ![512, 1024, 2048].includes(blockSize) || blocks * blockSize > size) break;
                    volume = { id: descriptor.toString('ascii', 40, 72).trim(), blocks, blockSize };
                }
            }
            return volume && terminated ? { format: 'ISO 9660', volume } : { format: 'unrecognized' };
        });
    }
}
