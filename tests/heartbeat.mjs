import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { resolve } from 'node:path';

class HeartbeatVerification {
    static run(directory) {
        const result = spawnSync(resolve(directory, 'tool'), [], {
            cwd: directory,
            encoding: 'utf8',
            timeout: 10_000,
            input: [
                'heartbeat status hb-explicit',
                'heartbeat pulse hb-explicit',
                'heartbeat status hb-explicit',
                'heartbeat pulse hb-explicit',
                'tracerobin trace hb-explicit',
                'trigger hb-triggered',
                'heartbeat status hb-triggered',
                'reflex hb-reflex',
                'heartbeat status hb-reflex',
                'signal',
                'exit',
                '',
            ].join('\n'),
        });
        if (result.error) throw result.error;
        assert.equal(result.status, 0, result.stderr);
        const output = result.stdout;
        assert.match(output, /No heartbeat/);
        for (const target of ['hb-explicit', 'hb-triggered', 'hb-reflex']) {
            assert.match(output, new RegExp(target + ' last pulse \\d+ ms ago'));
        }
        const trace = [...output.matchAll(/\d+ \d+ hb-explicit (\w+)$/gm)];
        assert.deepEqual(trace.map(match => match[1]), ['heartbeat', 'reflex', 'heartbeat', 'reflex']);
        const signals = [...output.matchAll(/(hb-\w+\/\w+ \d+)$/gm)];
        assert.deepEqual(signals.map(match => match[1]), [
            'hb-explicit/heartbeat 2', 'hb-explicit/reflex 2',
            'hb-reflex/reflex 1', 'hb-triggered/trigger 1',
        ]);
    }
}

HeartbeatVerification.run(resolve(process.argv[2]));
console.log('Native heartbeat signaling checks passed');
