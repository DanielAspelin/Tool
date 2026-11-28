import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { resolve } from 'node:path';

class CodeVerification {
    static run(directory) {
        const invoke = (...args) => {
            const result = spawnSync(resolve(directory, 'tool'), ['code', ...args], {
                cwd: directory, encoding: 'utf8', timeout: 5000,
            });
            if (result.error) throw result.error;
            return result;
        };
        assert.equal(invoke('self-test').stdout.trim(), 'CODE self-test passed (6 checks)');
        for (const [language, bytes, expected] of [
            ['cpp', '0', 'rejected'], ['wasm', '1048576', 'accepted'],
            ['both', '1048577', 'rejected'],
        ]) {
            const result = invoke('self-test', '--language', language, '--bytes', bytes);
            assert.equal(result.status, 0, result.stderr);
            assert.ok(result.stdout.includes('Languages: ' + language + '\n'));
            assert.ok(result.stdout.includes('Sample bytes: ' + bytes + ' (' + expected + ' as expected)'));
        }
        for (const args of [
            ['--language', 'python'], ['--bytes', '1048578'], ['--bytes', '-1'],
            ['--bytes', '0', '--bytes', '1'], ['--language'], ['--unknown', '1'],
        ]) {
            const result = invoke('self-test', ...args);
            assert.equal(result.status, 1, args.join(' '));
            assert.equal(result.stdout, '');
            assert.notEqual(result.stderr.trim(), '');
        }
        assert.equal(invoke('cpp', 'int main() {}').stdout.trim(), 'Valid source record');
        assert.equal(invoke('wasm', '').stdout.trim(), 'Invalid source record');
    }
}

CodeVerification.run(resolve(process.argv[2]));
console.log('Native CODE parameter checks passed');
