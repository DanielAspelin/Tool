// SPDX-License-Identifier: GPL-3.0-only

class VisualControl {
    #operations = new Map([['pool', { action: 'create' }], ['regulate', { action: 'service regulation' }]]);
    #systems = new Map();
    #canvas;
    #context;
    #token;
    #selected = 'primary';
    #runtime;
    #analysis;
    #analysisAt = 0;
    #analyzing = false;

    mount() {
        const style = document.createElement('style');
        style.textContent = `
            :root{color-scheme:dark;font:15px system-ui,sans-serif;color:#e9f4ee;background:#101713}
            *{box-sizing:border-box}body{margin:0}main{max-width:1280px;margin:auto;padding:36px clamp(16px,4vw,58px)}
            header{display:flex;align-items:end;justify-content:space-between;gap:18px;margin-bottom:29px}
            h1{font-size:clamp(32px,6vw,55px);letter-spacing:-.07em;line-height:1;margin:8px 0 0}
            h2{font-size:20px;margin:0 0 14px;letter-spacing:-.03em}p{margin:0}small,.secondary{color:#9fb7a9}
            .eyebrow{font-size:11px;text-transform:uppercase;letter-spacing:.15em;color:#8ef0ae}
            .grid{display:grid;grid-template-columns:minmax(0,1fr) minmax(245px,325px);gap:20px}
            section,aside{border:1px solid #324437;border-radius:15px;background:#17231b;overflow:hidden}
            section{padding:20px}aside{padding:20px}.toolbar,form,.line{display:flex;align-items:center;gap:9px;flex-wrap:wrap}
            .toolbar,.line{justify-content:space-between}.toolbar{margin-bottom:12px}
            canvas{width:100%;height:400px;display:block;border-radius:10px;background:#111b15}
            button,select,input{color:#ebfff0;background:#243a2b;border:1px solid #496550;border-radius:7px;padding:10px 12px;font:inherit}
            button{cursor:pointer}button:hover{background:#36583f}button:focus-visible,select:focus-visible,input:focus-visible{outline:2px solid #9cf5bb}
            form{margin-top:18px}input{min-width:0;width:125px}
            .service{margin-top:12px;padding:12px 0 0;border-top:1px solid #324437}.service .line{margin-bottom:8px}
            .audit{margin-top:16px;line-height:1.45}#status{display:block;margin-top:14px;min-height:20px}
            @media(max-width:790px){.grid{grid-template-columns:1fr}header{align-items:start;flex-direction:column}canvas{height:335px}}
        `;
        document.head.append(style);
        const main = document.createElement('main');
        main.innerHTML = `
            <header><div><p class="eyebrow">Tool / Visual Control</p><h1>Vector systems</h1></div>
            <span class="secondary" id="connection" role="status">Connecting to the pool</span></header>
            <div class="grid"><section><div class="toolbar"><h2>Live topology</h2>
            <label>System <select id="system"></select></label></div>
            <canvas aria-label="Pool topology showing background services and full-duplex worker sockets"></canvas>
            <p id="audit" class="audit secondary" aria-live="polite">Reading background state…</p>
            <p id="telemetry" class="secondary"></p></section>
            <aside><h2>Service regulation</h2><div id="services"></div>
            <form><input id="name" required pattern="[a-z][a-z0-9\\-]{0,23}" placeholder="system-id" aria-label="New system ID">
            <select id="sockets" aria-label="Socket count"><option>2</option><option>3</option><option selected>4</option></select>
            <button>Add system</button></form><output id="status" role="status"></output></aside></div>`;
        document.body.append(main);
        this.#canvas = main.querySelector('canvas');
        this.#context = this.#canvas.getContext('2d');
        main.querySelector('#system').addEventListener('change', event => {
            this.#selected = event.target.value;
            this.render();
        });
        main.querySelector('form').addEventListener('submit', event => {
            event.preventDefault();
            const system = this.element('name').value;
            this.invoke('pool', { system, sockets: Number(this.element('sockets').value) })
                .then(ok => {
                    if (!ok) return;
                    this.#selected = system;
                    this.element('name').value = '';
                });
        });
        main.querySelector('#services').addEventListener('click', event => {
            if (event.target.tagName !== 'BUTTON') return;
            const service = this.#systems.get(this.#selected)?.services.find(s => s.id === event.target.dataset.service);
            if (service) this.regulate(service, !service.enabled, service.limit);
        });
        main.querySelector('#services').addEventListener('change', event => {
            const service = this.#systems.get(this.#selected)?.services.find(s => s.id === event.target.dataset.service);
            if (service) this.regulate(service, service.enabled, Number(event.target.value));
        });
        new ResizeObserver(() => this.paint()).observe(this.#canvas);
        this.poll();
    }

    element(id) {
        return document.getElementById(id);
    }

    regulate(service, enabled, limit) {
        return this.invoke('regulate', { system: this.#selected, service: service.id, enabled, limit });
    }

    async invoke(action, input) {
        if (!this.#operations.has(action)) return;
        try {
            const response = await fetch('/api/' + action, {
                method: 'POST', headers: { 'Content-Type': 'application/json', 'X-Tool-Token': this.#token },
                body: JSON.stringify(input),
            });
            if (!response.ok) throw new Error('The pool rejected this operation');
            this.element('status').textContent = action === 'pool' ? 'System added' : 'Regulation updated';
        } catch (error) {
            this.element('status').textContent = error.message;
            return false;
        }
        return true;
    }

    async poll() {
        try {
            const response = await fetch('/api/state', { cache: 'no-store', signal: AbortSignal.timeout(3000) });
            if (!response.ok) throw new Error();
            const state = await response.json();
            this.#token = state.token;
            this.#systems = new Map(state.systems.map(system => [system.id, system]));
            if (!this.#systems.has(this.#selected)) this.#selected = this.#systems.keys().next().value;
            this.element('connection').textContent = `${state.systems.length} local system${state.systems.length === 1 ? '' : 's'}`;
            this.#runtime ??= state.runtime ? this.initR() : Promise.resolve(null);
            this.render();
            this.analyze();
        } catch {
            this.element('connection').textContent = 'Pool connection unavailable';
        } finally {
            setTimeout(() => this.poll(), 500);
        }
    }

    render() {
        const selector = this.element('system');
        const keys = Array.from(this.#systems.keys());
        if (keys.join() !== Array.from(selector.options, option => option.value).join()) {
            selector.replaceChildren(...keys.map(key => new Option(key, key)));
        }
        selector.value = this.#selected;
        const system = this.#systems.get(this.#selected);
        const container = this.element('services');
        const signature = JSON.stringify([system?.id, system?.services.map(s => [s.id, s.enabled, s.limit, s.capacity])]);
        if (signature !== container.dataset.signature) {
            container.dataset.signature = signature;
            container.replaceChildren(...(system?.services ?? []).map(service => this.serviceForm(service)));
        }
        const telemetry = system?.services.find(service => service.id === 'telemetry')?.result;
        this.element('telemetry').textContent = telemetry ? `Telemetry worker RSS: ${telemetry.rssKiB} KiB` : '';
        this.paint();
    }

    serviceForm(service) {
        const panel = document.createElement('div');
        panel.className = 'service';
        const title = document.createElement('strong');
        title.textContent = service.id === 'geometry' ? 'Vector geometry' : 'Worker telemetry';
        const button = document.createElement('button');
        button.dataset.service = service.id;
        button.textContent = service.enabled ? 'Pause' : 'Resume';
        const line = document.createElement('div');
        line.className = 'line';
        line.append(title, button);
        const select = document.createElement('select');
        select.dataset.service = service.id;
        select.replaceChildren(...Array.from({ length: service.capacity }, (_, i) => new Option(i + 1, i + 1)));
        select.value = service.limit;
        const label = document.createElement('label');
        label.append('Parallel workers ', select);
        panel.append(line, label);
        return panel;
    }

    paint() {
        const pool = this.#systems.get(this.#selected);
        if (!pool) return;
        const canvas = this.#canvas, ctx = this.#context, ratio = window.devicePixelRatio || 1;
        const w = canvas.clientWidth, h = canvas.clientHeight;
        if (canvas.width !== Math.round(w * ratio) || canvas.height !== Math.round(h * ratio)) {
            canvas.width = Math.round(w * ratio);
            canvas.height = Math.round(h * ratio);
        }
        ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
        ctx.clearRect(0, 0, w, h);
        ctx.font = '12px system-ui';
        ctx.textAlign = 'center';
        const center = w / 2, radio = Math.max(70, w * .25);
        this.node(ctx, center, 48, 'Foreground', '#acf7ba');
        const columns = new Map([['geometry', center - radio], ['telemetry', center + radio]]);
        for (const service of pool.services) {
            const x = columns.get(service.id);
            this.line(ctx, center, 66, x, 124, '#4c6857');
            this.node(ctx, x, 142, service.id === 'geometry' ? 'Geometry' : 'Telemetry',
                service.enabled ? '#75c59a' : '#c0ac7c');
            const workers = pool.slots.filter(slot => slot.service === service.id);
            for (const [i, slot] of workers.entries()) {
                const sx = x + (i - (workers.length - 1) / 2) * Math.min(82, w / 5);
                const ok = !['failed', 'stopped', 'starting'].includes(slot.state);
                this.line(ctx, x, 160, sx, 225, ok ? '#4fa786' : '#b07a6d');
                this.node(ctx, sx, 243, `⇄ ${slot.id}`, ok ? '#82ddad' : '#e49b8b');
            }
        }
        const vectors = pool.services.find(service => service.id === 'geometry')?.result?.points;
        if (vectors?.length) {
            ctx.strokeStyle = '#84f4a6';
            ctx.lineWidth = 2;
            ctx.beginPath();
            for (const [i, [x, y]] of vectors.entries()) {
                const px = 14 + x * (w - 28), py = 270 + y * Math.max(0, h - 282);
                if (i === 0) ctx.moveTo(px, py);
                else ctx.lineTo(px, py);
            }
            ctx.stroke();
        }
    }

    node(ctx, x, y, label, color) {
        const width = Math.max(44, ctx.measureText(label).width + 23);
        ctx.beginPath();
        ctx.roundRect(x - width / 2, y - 17, width, 34, 9);
        ctx.fillStyle = '#21372a';
        ctx.fill();
        ctx.strokeStyle = color;
        ctx.lineWidth = 1;
        ctx.stroke();
        ctx.fillStyle = color;
        ctx.fillText(label, x, y + 4);
    }

    line(ctx, x, y, toX, toY, color) {
        ctx.beginPath();
        ctx.moveTo(x, y);
        ctx.bezierCurveTo(x, (y + toY) / 2, toX, (y + toY) / 2, toX, toY);
        ctx.strokeStyle = color;
        ctx.lineWidth = 1.5;
        ctx.stroke();
    }

    async initR() {
        try {
            const [{ WebR }, response] = await Promise.all([
                import('/webr/webr.js'), fetch('./index.R'),
            ]);
            if (!response.ok) return null;
            const runtime = new WebR({ baseUrl: new URL('/webr/', location.href).href });
            await runtime.init();
            return { runtime, source: await response.text() };
        } catch {
            return null;
        }
    }

    async analyze() {
        const pool = this.#systems.get(this.#selected);
        if (!pool || this.#analyzing || Date.now() - this.#analysisAt < 1500) return;
        this.#analyzing = true;
        try {
            const live = pool.slots.filter(slot => ['active', 'idle'].includes(slot.state)).length;
            const completed = pool.services.reduce((total, s) => total + s.completed, 0);
            const failed = pool.services.reduce((total, s) => total + s.failed, 0);
            const r = await this.#runtime;
            const summary = r ? await r.runtime.evalRRaw(
                `(${r.source})(${pool.sockets},${live},${completed},${failed})`, 'number[]'
            ) : null;
            this.#analysis = summary ? `R analysis: ${summary[0].toFixed(0)}% live · ${summary[1].toFixed(0)}% completed`
                : 'R runtime unavailable';
            if (pool.id === this.#selected) this.element('audit').textContent =
                `${live}/${pool.sockets} full-duplex worker sockets · ${completed} jobs · ${this.#analysis}`;
        } catch {
            this.element('audit').textContent = 'R analysis unavailable';
        } finally {
            this.#analysisAt = Date.now();
            this.#analyzing = false;
        }
    }
}

new VisualControl().mount();
