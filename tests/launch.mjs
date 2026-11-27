import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { once } from 'node:events';
import { hostname } from 'node:os';
import { resolve } from 'node:path';
import { createInterface } from 'node:readline';

class LaunchVerification {
    static async run(directory) {
        const tool = resolve(directory, 'tool');
        const env = { ...process.env };
        delete env.TOOL_LAUNCH_HOST;
        const host = hostname();
        const command = (...args) => spawnSync(tool, args, { cwd: directory, env, encoding: 'utf8', timeout: 5000 });
        assert.match(command('launch', 'plan').stdout, /Prelaunch:[\s\S]*Liftoff:[\s\S]*Postlaunch:/);
        const preflight = command('launch', 'preflight', 'mobile', '0');
        assert.equal(preflight.status, 0, preflight.stderr);
        assert.ok(preflight.stdout.split('\n').includes('Launch host: ' + host));
        assert.match(preflight.stdout, /Mode: mobile\nBind: 127\.0\.0\.1:0 \(assigned on start\)\nHandoff: disabled/);
        assert.equal(command('workspace', 'preflight', 'mobile', '0').stdout, preflight.stdout);
        assert.equal(command('server', 'preflight', 'mobile', '0').stdout, preflight.stdout);
        assert.match(command('launch', 'preflight', 'auto').stdout, /Bind: 127\.0\.0\.1:9966\nHandoff: 127\.0\.0\.1:6699/);
        for (const action of ['preflight', 'start']) {
            assert.equal(command('launch', action, 'invalid', '0').status, 1);
            assert.equal(command('launch', action, 'auto', '65536').status, 1);
            const mismatch = spawnSync(tool, ['launch', action, 'auto', '0'], {
                cwd: directory, encoding: 'utf8', timeout: 5000,
                env: { ...env, TOOL_LAUNCH_HOST: host + '-mismatch' },
            });
            assert.equal(mismatch.status, 1);
            assert.match(mismatch.stderr, /Launch host does not match TOOL_LAUNCH_HOST/);
            assert.doesNotMatch(mismatch.stdout, /http:\/\/127\.0\.0\.1:/);
        }

        const child = spawn(tool, ['launch', 'start', 'mobile', '0'], {
            cwd: directory, stdio: ['ignore', 'pipe', 'inherit'],
            env: { ...env, TOOL_LAUNCH_HOST: host },
        });
        const exit = once(child, 'exit');
        const lines = createInterface({ input: child.stdout });
        const deadline = setTimeout(() => child.kill('SIGKILL'), 8000);
        try {
            let baseUrl;
            let identifiedHost = false;
            for await (const line of lines) {
                if (line === 'Launch host: ' + host) identifiedHost = true;
                if (line.startsWith('http://127.0.0.1:')) {
                    baseUrl = line;
                    break;
                }
            }
            assert.ok(baseUrl, 'Expected a loopback readiness URL');
            assert.ok(identifiedHost, 'Expected the detected launch host in startup output');
            const response = await fetch(new URL('/api/runtime', baseUrl), { signal: AbortSignal.timeout(4000) });
            assert.equal(response.status, 200);
            assert.equal(await response.text(), 'runtime\trunning\tmobile');
            child.kill('SIGTERM');
            assert.deepEqual(await exit, [0, null]);
        } finally {
            clearTimeout(deadline);
            lines.close();
            child.kill('SIGKILL');
            await exit;
        }
    }
}

await LaunchVerification.run(resolve(process.argv[2]));
console.log('Native Launch Control checks passed');
