import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { chmodSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, rmSync, statSync, symlinkSync, writeFileSync } from 'node:fs';
import { join, resolve } from 'node:path';

class CommandVerification {
    static run(directory) {
        const invoke = (...args) => {
            const result = spawnSync(resolve(directory, 'tool'), ['command', ...args], {
                cwd: directory, encoding: 'utf8', timeout: 5000,
            });
            if (result.error) throw result.error;
            return result;
        };
        assert.match(invoke('status').stdout, /^Command Control ready/);
        assert.equal(invoke('check', 'echo').stdout.trim(), 'Registered');
        assert.equal(invoke('check', 'absent-command').stdout.trim(), 'Unknown command');
        assert.deepEqual(invoke('list', 'ech').stdout.trim().split('\n'), ['echo']);
        assert.equal(invoke('inspect', 'echo').stdout.trim(), 'echo [core]');
        const echo = invoke('run', 'echo', 'ää🌱', 'two words', 'tail');
        assert.equal(echo.status, 0, echo.stderr);
        assert.equal(echo.stdout, 'ää🌱 two words tail\n');
        assert.match(invoke('run', 'code', 'self-test', '--language', 'wasm').stdout, /^CODE self-test passed/);
        for (const [action, name, message] of [
            ['run', 'absent-command', /Unknown command/],
            ['inspect', 'absent-command', /Unknown command/],
            ['run', 'command', /cannot dispatch itself/],
        ]) {
            const failure = invoke(action, name);
            assert.equal(failure.status, 1);
            assert.match(failure.stderr, message);
        }
        if (process.platform === 'linux') this.verifyInstallation(directory, invoke);
    }

    static verifyInstallation(directory, command) {
        const source = resolve(directory, 'tool');
        const workspace = mkdtempSync(join(directory, 'installation-'));
        const invoke = (binary, ...args) => {
            const result = spawnSync(binary, args, { cwd: workspace, encoding: 'utf8', timeout: 5000 });
            if (result.error) throw result.error;
            return result;
        };
        try {
            const destination = join(workspace, 'local bin');
            mkdirSync(destination);
            const target = join(destination, 'tool');
            const operation = ['installation', 'self-install', destination];
            const blocked = invoke(source, 'api', ...operation);
            assert.match(blocked.stdout + blocked.stderr, /requires an explicit local CLI invocation/);
            assert.deepEqual(readdirSync(destination), []);

            const install = command('run', ...operation);
            assert.equal(install.status, 0, install.stderr);
            assert.match(install.stdout, /^Installed Tool at /);
            assert.equal(statSync(target).mode & 0o7777, 0o700);
            assert.deepEqual(readFileSync(target), readFileSync(source));
            assert.match(invoke(target, 'code', 'self-test').stdout, /^CODE self-test passed/);
            assert.match(invoke(source, ...operation).stdout, /^Tool is already installed at /);
            assert.deepEqual(readdirSync(destination), ['tool']);

            rmSync(target);
            writeFileSync(target, 'existing file');
            assert.equal(invoke(source, ...operation).status, 1);
            assert.equal(readFileSync(target, 'utf8'), 'existing file');
            rmSync(target);
            symlinkSync(source, target);
            assert.equal(invoke(source, ...operation).status, 1);
            rmSync(target);
            chmodSync(destination, 0o777);
            assert.match(invoke(source, ...operation).stderr, /writable only by that user/);
            assert.deepEqual(readdirSync(destination), []);
        } finally {
            rmSync(workspace, { recursive: true, force: true });
        }
    }
}

CommandVerification.run(resolve(process.argv[2]));
console.log('Native Command Control checks passed');
