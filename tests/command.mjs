import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { resolve } from 'node:path';

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
    }
}

CommandVerification.run(resolve(process.argv[2]));
console.log('Native Command Control checks passed');
