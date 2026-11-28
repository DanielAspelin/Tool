// SPDX-License-Identifier: GPL-3.0-only

export class DuplexChannel {
    operations = new Map([['send', { framing: 'NDJSON' }], ['close', { effect: 'destroy' }]]);
    #buffer = '';
    #socket;

    constructor(socket, receive, fail) {
        this.#socket = socket;
        socket.setEncoding('utf8');
        socket.on('data', chunk => {
            try {
                this.#buffer += chunk;
                const lines = this.#buffer.split('\n');
                this.#buffer = lines.pop();
                if (this.#buffer.length > 32768) throw new Error('Invalid socket frame');
                for (const line of lines) {
                    if (line.length > 32768) throw new Error('Invalid socket frame');
                    receive(JSON.parse(line));
                }
            } catch {
                this.close();
            }
        });
        socket.on('error', fail);
        socket.once('close', fail);
    }

    send(value) {
        if (this.#socket.destroyed) throw new Error('Worker socket is closed');
        const encoded = JSON.stringify(value);
        if (encoded.length > 32768 || this.#socket.writableLength > 32768) {
            throw new Error('Worker socket limit exceeded');
        }
        this.#socket.write(encoded + '\n');
    }

    close() {
        this.#socket.destroy();
    }
}
