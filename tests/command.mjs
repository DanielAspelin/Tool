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
        this.verifyTimeLock(directory);
        this.verifyBaseCoverage(invoke);
    }

    static verifyBaseCoverage(invoke) {
        const report = (...args) => {
            const result = invoke('run', 'coverage', 'base', ...args);
            assert.equal(result.status, 0, result.stderr);
            return result.stdout.trimEnd().split('\n').map(line => line.split('\t'));
        };
        const [header, ...requirements] = report();
        assert.deepEqual(header, ['Code', 'Owner', 'Control', 'Required evidence']);
        assert.deepEqual(requirements.map(row => row[0]),
            ['BCC-01', 'BCC-02', 'BCC-03', 'BCC-04', 'BCC-05', 'BCC-06', 'BCC-07', 'BCC-08']);
        assert.deepEqual(report('information').slice(1), requirements.filter(row => row[1] === 'Information Control'));
        assert.deepEqual(report('systems').slice(1), requirements.filter(row => row[1] === 'Systems Control'));
        assert.deepEqual(report('BCC-05'), [header, requirements[4]]);
        assert.deepEqual(report('classification'), [
            ['Perimeter', 'Base Designated Classification'],
            ['Native staging', 'Installation and verification perimeter'],
            ['Existing Tool Base', 'Reference installation'],
            ['Browser production', 'Browser deployment perimeter'],
            ['Project System Headquarters', 'Intended native deployment perimeter. Registered target pending identification'],
        ]);
        const invalid = invoke('run', 'coverage', 'base', 'BCC-99');
        assert.equal(invalid.status, 1);
        assert.match(invalid.stderr, /Usage: coverage base/);
        assert.match(invoke('run', 'coverage', 'classes').stdout, /\bCLI cli\b/);
    }

    static verifyTimeLock(directory) {
        const result = spawnSync(resolve(directory, 'tool'), [], {
            cwd: directory,
            encoding: 'utf8',
            timeout: 10_000,
            input: [
                'uptime start service',
                'uptime start other',
                'prepositioner prepare-lock service 60000',
                'uptime lock-status service',
                'prepositioner prepare-lock service 1',
                'uptime unlock service',
                'uptime lock-status service',
                'downtime begin service',
                'prepositioner.prepare-lock service 25',
                'sleep wait 250',
                'uptime lock-status service',
                'uptime status service',
                'downtime status service',
                'downtime end service',
                'sleep wait 75',
                'uptime status service',
                'downtime status service',
                'uptime lock-status other',
                'uptime unlock service',
                'uptime lock-status service',
                'downtime end service',
                'uptime lock service',
                'uptime lock-status service',
                'downtime begin service',
                'prepositioner prepare-lock other -1',
                'uptime lock-status other',
                'exit',
            ].join('\n'),
        });
        if (result.error) throw result.error;
        assert.equal(result.status, 0, result.stderr);
        assert.match(result.stderr, /Lock delay must be a whole number/);
        const lines = result.stdout.trim().split('\n');
        assert.deepEqual(lines.slice(0, 3), [
            'Availability tracking started', 'Availability tracking started',
            'Time lock armed',
        ]);
        assert.match(lines[3], /^phase=armed deadline_ns=\d+ remaining_ns=[1-9]\d*$/);
        assert.deepEqual(lines.slice(4, 9), [
            'Time lock already armed', 'Time lock canceled', 'phase=open',
            'Unavailable', 'Time lock armed',
        ]);
        assert.match(lines[10], /^phase=locked deadline_ns=\d+ remaining_ns=0$/);
        assert.match(lines[11], /^\d+ ms; state=down$/);
        assert.match(lines[12], /^\d+ ms; state=down$/);
        const frozenDowntimeMs = Number.parseInt(lines[12], 10);
        assert.ok(frozenDowntimeMs >= 25 && frozenDowntimeMs < 250, lines[12]);
        assert.equal(lines[13], 'Time counters locked');
        assert.deepEqual(lines.slice(15, 22), [
            lines[11], lines[12], 'phase=open', 'Time counters unlocked',
            'phase=open', 'Available', 'Time counters locked',
        ]);
        assert.match(lines[22], /^phase=locked deadline_ns=\d+ remaining_ns=0$/);
        assert.deepEqual(lines.slice(23, 25), ['Time counters locked', 'phase=open']);
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
