// SPDX-License-Identifier: GPL-3.0-only

export class TelecomData {
    static basic = new Set('@£$¥èéùìòÇ\nØø\rÅåΔ_ΦΓΛΩΠΨΣΘΞÆæßÉ !"#¤%&\'()*+,-./0123456789:;<=>?¡ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÑÜ§¿abcdefghijklmnopqrstuvwxyzäöñüà');
    static extended = new Set('\f^{}\\[~]|€');

    static validateIMEI(value) {
        if (typeof value !== 'string' || !/^[0-9]{15}$/.test(value) || /^0{15}$/.test(value)) return false;
        const sum = Array.from(value, (digit, i) => {
            const number = Number(digit) * (i % 2 ? 2 : 1);
            return number > 9 ? number - 9 : number;
        }).reduce((a, b) => a + b, 0);
        return sum % 10 === 0;
    }

    static planSMS(text) {
        if (typeof text !== 'string' || !text.length || text.length > 2048 || !text.isWellFormed()) {
            throw new TypeError('Expected a non-empty, well-formed SMS body up to 2048 UTF-16 units');
        }
        const characters = Array.from(text);
        const gsm = characters.every(character => this.basic.has(character) || this.extended.has(character));
        const sizes = characters.map(character => gsm
            ? this.extended.has(character) ? 2 : 1
            : character.length);
        const units = sizes.reduce((a, b) => a + b, 0);
        const limit = gsm ? 160 : 70;
        const concatenatedLimit = gsm ? 153 : 67;
        let segments = 1;
        if (units > limit) {
            let used = 0;
            for (const size of sizes) {
                if (used + size > concatenatedLimit) {
                    segments++;
                    used = 0;
                }
                used += size;
            }
        }
        return { encoding: gsm ? 'gsm-7' : 'utf-16', units, segments };
    }

    static phone(value) {
        if (typeof value !== 'string' || !/^\+[1-9][0-9]{1,14}$/.test(value)) {
            throw new TypeError('Expected an E.164 phone number');
        }
        return value;
    }
}
