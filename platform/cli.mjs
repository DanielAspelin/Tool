// SPDX-License-Identifier: GPL-3.0-only
import { PlatformSystem } from './system.mjs';

class PlatformCLI {
    registry = new Map([['kvm', { operands: 0 }], ['hyperv', { operands: 0 }],
        ['bin', { operands: 1 }], ['iso', { operands: 1 }], ['isoiec', { operands: 0 }]]);

    async run([profile, ...args]) {
        const command = this.registry.get(profile);
        if (!command || args.length !== command.operands) {
            console.error('Usage: node platform/cli.mjs kvm|hyperv|isoiec|bin FILE|iso FILE');
            process.exitCode = 2;
            return;
        }
        const system = new PlatformSystem({ root: process.env.TOOL_IMAGE_ROOT });
        try {
            console.log(JSON.stringify(await system[profile](...args), null, 2));
        } catch (error) {
            console.error(error.message);
            process.exitCode = 1;
        }
    }
}

await new PlatformCLI().run(process.argv.slice(2));
