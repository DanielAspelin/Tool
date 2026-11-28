// SPDX-License-Identifier: GPL-3.0-only
import { Buffer } from 'node:buffer';
import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { TelecomData } from './telecom.mjs';

export class TelecomAdapters {
    operations = new Map([
        ['imei', ['validate']], ['gsm', ['modem-status']], ['sms', ['plan', 'send', 'status']],
        ['number-verification', ['verify']],
    ]);

    constructor(connections, config, exec = promisify(execFile)) {
        this.connections = connections;
        this.config = config;
        this.exec = exec;
    }

    register(communications) {
        communications.register('imei', { validate: ({ imei }) => ({ valid: TelecomData.validateIMEI(imei) }) });
        if (this.config.modem) {
            communications.register('gsm', { 'modem-status': () => this.modem() });
        }
        const sms = { plan: ({ text }) => TelecomData.planSMS(text) };
        if (this.config.twilio) {
            const request = this.connections.open('twilio');
            sms.send = input => this.send(request, input);
            sms.status = input => this.status(request, input);
        }
        communications.register('sms', sms);
        if (this.config.camara) {
            const request = this.connections.open('camara');
            communications.register('number-verification', {
                verify: input => this.verifyNumber(request, input),
            });
        }
    }

    async modem() {
        if (process.platform !== 'linux') throw new Error('ModemManager adapter requires Linux');
        const { modemId } = this.config.modem;
        if (!Number.isSafeInteger(modemId) || modemId < 0) throw new Error('Invalid configured modem ID');
        const { stdout } = await this.exec('mmcli', ['--modem', String(modemId), '--output-json'], {
            timeout: 5000, maxBuffer: 256 * 1024,
        });
        const { modem } = JSON.parse(stdout);
        return {
            state: modem.generic.state,
            accessTechnologies: modem.generic['access-technologies'],
            registration: modem['3gpp']?.['registration-state'],
        };
    }

    twilioConnection() {
        const { accountSid, apiKeySid, apiKeySecret, messagingServiceSid } = this.config.twilio;
        if (!/^AC[a-f0-9]{32}$/i.test(accountSid) || !/^SK[a-f0-9]{32}$/i.test(apiKeySid) ||
            !/^MG[a-f0-9]{32}$/i.test(messagingServiceSid) || !apiKeySecret) {
            throw new Error('Twilio messaging credentials or service are unavailable');
        }
        const authorization = Buffer.from(`${apiKeySid}:${apiKeySecret}`).toString('base64');
        return {
            path: `/2010-04-01/Accounts/${accountSid}/Messages`,
            headers: { Authorization: `Basic ${authorization}` },
            messagingServiceSid,
        };
    }

    async send(request, { to, text }) {
        const { segments } = TelecomData.planSMS(text);
        if (segments > this.config.twilio.maxSegments) throw new RangeError('SMS exceeds route segment limit');
        const { path, headers, messagingServiceSid } = this.twilioConnection();
        const message = await request(`${path}.json`, {
            method: 'POST', headers,
            body: new URLSearchParams({
                To: TelecomData.phone(to), Body: text, MessagingServiceSid: messagingServiceSid,
            }),
        });
        return { reference: message.sid, state: message.status };
    }

    async status(request, { reference }) {
        if (typeof reference !== 'string' || !/^(SM|MM)[a-f0-9]{32}$/i.test(reference)) {
            throw new TypeError('Invalid Twilio message reference');
        }
        const { path, headers } = this.twilioConnection();
        const message = await request(`${path}/${reference}.json`, { headers });
        return { reference: message.sid, state: message.status };
    }

    async verifyNumber(request, { number, subscriberToken }) {
        if (typeof subscriberToken !== 'string' || !/^[A-Za-z0-9._~+\/=-]+$/.test(subscriberToken)) {
            throw new Error('A subscriber-bound access token is required');
        }
        const result = await request(this.config.camara.verifyPath, {
            method: 'POST',
            headers: { Authorization: `Bearer ${subscriberToken}`, 'Content-Type': 'application/json' },
            body: JSON.stringify({ phoneNumber: TelecomData.phone(number) }),
        });
        if (typeof result.devicePhoneNumberVerified !== 'boolean') {
            throw new Error('Invalid Number Verification response');
        }
        return { verified: result.devicePhoneNumberVerified };
    }
}
