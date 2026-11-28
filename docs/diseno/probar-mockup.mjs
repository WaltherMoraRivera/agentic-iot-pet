// Prueba del mockup V01 en Chromium headless, sin dependencias (Node >= 22, DevTools Protocol).
// Uso: node docs/diseno/probar-mockup.mjs   (variable CHROMIUM para otro ejecutable)
// Los controles se buscan por rol y nombre accesible y se accionan con ratón o teclado reales.
import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { once } from 'node:events';
import { copyFile, mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, test } from 'node:test';
import { setTimeout as sleep } from 'node:timers/promises';
import { pathToFileURL } from 'node:url';

const PAGE = new URL('./mockup.html', import.meta.url).href;
const requests = [];
const errors = [];
let browser, profile, socket, sessionId;
let lastId = 0;
const waiting = new Map();

const send = (method, params = {}, session = sessionId) => new Promise((resolve, reject) => {
  const id = ++lastId;
  waiting.set(id, { resolve, reject, method });
  socket.send(JSON.stringify({ id, method, params, sessionId: session }));
});

before(async () => {
  profile = await mkdtemp(join(tmpdir(), 'pet-mockup-'));
  browser = spawn(process.env.CHROMIUM || 'chromium', [
    '--headless=new', '--remote-debugging-port=0', `--user-data-dir=${profile}`,
    // Corta toda salida a la red: el mockup debe funcionar sin ella.
    '--no-first-run', '--disable-gpu', '--host-resolver-rules=MAP * ~NOTFOUND', 'about:blank',
  ], { stdio: ['ignore', 'ignore', 'pipe'] });
  const url = await new Promise((resolve, reject) => {
    let log = '';
    browser.on('error', reject);
    browser.on('exit', () => reject(new Error(`Chromium terminó antes de tiempo:\n${log}`)));
    browser.stderr.on('data', chunk => {
      log += chunk;
      const found = log.match(/DevTools listening on (ws:\S+)/);
      if (found) resolve(found[1]);
    });
  });
  socket = new WebSocket(url);
  await once(socket, 'open');
  socket.addEventListener('message', ({ data }) => {
    const message = JSON.parse(data);
    const pending = waiting.get(message.id);
    if (pending) {
      waiting.delete(message.id);
      if (message.error) pending.reject(new Error(`${pending.method}: ${message.error.message}`));
      else pending.resolve(message.result);
    } else if (message.method === 'Network.requestWillBeSent') requests.push(message.params.request.url);
    else if (message.method === 'Runtime.exceptionThrown') errors.push(message.params.exceptionDetails.exception?.description);
  });
  const { targetId } = await send('Target.createTarget', { url: 'about:blank' });
  ({ sessionId } = await send('Target.attachToTarget', { targetId, flatten: true }));
  for (const domain of ['Page', 'Runtime', 'Network', 'DOM', 'Accessibility']) await send(`${domain}.enable`);
});

after(async () => {
  socket?.close();
  if (browser && browser.exitCode === null) {
    const closed = once(browser, 'exit');
    browser.kill();
    await closed;
  }
  await rm(profile, { recursive: true, force: true, maxRetries: 5, retryDelay: 100 });
});

const evaluate = async expression => {
  const { result, exceptionDetails } = await send('Runtime.evaluate', { expression, returnByValue: true });
  if (exceptionDetails) throw new Error(exceptionDetails.exception.description);
  return result.value;
};

const open = async (width = 1280, height = 800) => {
  await send('Emulation.setDeviceMetricsOverride', { width, height, deviceScaleFactor: 1, mobile: width < 500 });
  // about:blank entre medio para que cada caso parta con el estado inicial.
  await send('Page.navigate', { url: 'about:blank' });
  await send('Page.navigate', { url: PAGE });
  await waitFor(`document.readyState === 'complete' && !!document.getElementById('face')`, 'cargar mockup.html con la cara', 3000);
  await faceIs('Sin conexión', 3000);
};

const waitFor = async (expression, what, timeout = 5000) => {
  const end = Date.now() + timeout;
  while (Date.now() < end) {
    if (await evaluate(expression)) return;
    await sleep(50);
  }
  const body = await evaluate('document.body?.innerText.slice(0, 600)');
  assert.fail(`No ocurrió: ${what}. Pantalla:\n${body}`);
};

const find = async (name, role) => {
  const { result } = await send('Runtime.evaluate', { expression: 'document' });
  const { nodes } = await send('Accessibility.queryAXTree', { objectId: result.objectId, accessibleName: name, role });
  const shown = nodes.filter(node => !node.ignored);
  assert.equal(shown.length, 1, `control «${name}» (${role}): ${shown.length} visibles`);
  const { object } = await send('DOM.resolveNode', { backendNodeId: shown[0].backendDOMNodeId });
  return object.objectId;
};

const on = async (objectId, fn) =>
  (await send('Runtime.callFunctionOn', { objectId, functionDeclaration: fn, returnByValue: true, awaitPromise: true })).result.value;

const click = async (name, role = 'button') => {
  const target = await find(name, role);
  // El OLED va en otro proceso: justo tras cargar o desplazar, Chromium puede perder un evento sintético
  // (visto 1 de cada 80 cargas). Se comprueba que el clic llegó al control y solo si no llegó se repite.
  for (let attempt = 0; attempt < 3; attempt++) {
    const box = await on(target, `async function () {
      this.scrollIntoView({ block: 'center' });
      await new Promise(done => requestAnimationFrame(() => requestAnimationFrame(done)));
      this.received = false;
      this.addEventListener('click', () => { this.received = true; }, { once: true });
      const r = this.getBoundingClientRect(), x = r.x + r.width / 2, y = r.y + r.height / 2;
      return { x, y, hit: this.contains(document.elementFromPoint(x, y)) };
    }`);
    assert.ok(box.hit, `«${name}» está tapado y no se puede pulsar`);
    for (const type of ['mousePressed', 'mouseReleased'])
      await send('Input.dispatchMouseEvent', { type, x: box.x, y: box.y, button: 'left', clickCount: 1 });
    if (await on(target, 'function () { return this.received; }')) {
      if (role === 'link') {
        const panel = await on(target, 'function () { return this.hash.slice(1); }');
        // El clic llega antes de que hashchange muestre el panel nuevo.
        await waitFor(`document.getElementById(${JSON.stringify(panel)})?.hidden === false`, `mostrar ${name}`);
      }
      return;
    }
    await sleep(100);
  }
  assert.fail(`el clic en «${name}» no llega al control`);
};

const fill = async (name, value, role = 'textbox') => {
  await click(name, role);
  await on(await find(name, role), 'function () { this.select(); }');
  await send('Input.insertText', { text: value });
};

const press = async (key, windowsVirtualKeyCode, text) => {
  await send('Input.dispatchKeyEvent', { type: 'keyDown', key, code: key, windowsVirtualKeyCode, text });
  await send('Input.dispatchKeyEvent', { type: 'keyUp', key, code: key, windowsVirtualKeyCode });
};

// Recorre con Tab hasta el control y lo activa con Enter; exige además un foco visible.
const keyboard = async name => {
  for (let i = 0; i < 80; i++) {
    await press('Tab', 9);
    const focus = await evaluate(`(() => { const e = document.activeElement;
      return { name: e.textContent.trim(), ring: getComputedStyle(e).outlineStyle }; })()`);
    if (focus.name === name) {
      assert.notEqual(focus.ring, 'none', `«${name}» sin foco visible`);
      return press('Enter', 13, '\r');
    }
  }
  assert.fail(`«${name}» no se alcanza con Tab`);
};

// El OLED es el boceto embebido en un marco aislado (otro proceso): se lee desde su propio destino.
const frames = new Map();
const face = async () => {
  const { targetInfos } = await send('Target.getTargets');
  const frame = targetInfos.find(t => t.type === 'iframe' && t.url.includes('cara-mascota.html'));
  if (!frame) return '';
  if (!frames.has(frame.targetId)) {
    const attached = await send('Target.attachToTarget', { targetId: frame.targetId, flatten: true });
    frames.set(frame.targetId, attached.sessionId);
    await send('Runtime.enable', {}, attached.sessionId);
  }
  // Solo vale si hay una cara visible y con píxeles encendidos.
  const { result } = await send('Runtime.evaluate', { returnByValue: true, expression: `(() => {
    const shown = [...document.querySelectorAll('canvas')].filter(c => c.checkVisibility());
    const lit = shown.length === 1 && shown[0].getContext('2d').getImageData(0, 0, 128, 64).data.some((v, i) => i % 4 === 0 && v);
    return lit ? shown[0].getAttribute('aria-label') : ''; })()` }, frames.get(frame.targetId));
  return result.value ?? '';
};
const faceIs = async (start, timeout = 5000) => {
  const end = Date.now() + timeout;
  let shown = await face();
  while (!shown.startsWith(start) && Date.now() < end) {
    await sleep(50);
    shown = await face();
  }
  assert.ok(shown.startsWith(start), `se esperaba la cara «${start}» y se ve «${shown}»`);
};
const status = id => evaluate(`(() => { const e = document.getElementById('${id}');
  const icon = e.querySelector('svg')?.getBoundingClientRect();
  return { text: e.innerText.trim(), shown: e.checkVisibility(), icon: !!icon && icon.width > 0,
    kind: e.querySelector('use')?.getAttribute('href') }; })()`);
const says = (id, part) =>
  waitFor(`document.getElementById('${id}').innerText.includes(${JSON.stringify(part)})`, `#${id} dice «${part}»`);
const bodyText = () => evaluate('document.body.innerText');

const connect = async () => {
  await click('Conectar placa simulada');
  await says('link-status', 'Conectada');
};
const startStudy = async () => {
  await click('Estudio', 'link');
  await click('Iniciar estudio');
  await click('Confirmar orden (placa)');
  await faceIs('Concentrada');
};
const login = async () => {
  await click('EVA02', 'link');
  await fill('Usuario', 'persona-de-prueba');
  await fill('Contraseña', 'cualquiera');
  await click('Ingresar');
  await says('session', 'persona-de-prueba');
};
const reading = async value => {
  await fill('Lectura simulada (0 a 1)', value, 'spinbutton');
  await click('Enviar lectura simulada');
};

test('S1: orden durante estudio espera, se confirma y vuelve al estado vigente (solo teclado)', async () => {
  await open();
  await keyboard('Conectar placa simulada');
  await says('link-status', 'Conectada');
  await keyboard('Estudio');
  await keyboard('Iniciar estudio');
  await keyboard('Confirmar orden (placa)');
  await faceIs('Concentrada');
  await open();
  await connect();
  await startStudy();
  const entries = await evaluate('history.length');
  await keyboard('Pedir cara contenta');
  await says('order-status', 'Esperando confirmación');
  assert.equal((await status('order-status')).kind, '#i-wait');
  assert.match(await face(), /^Concentrada/, 'sin respuesta no se muestra éxito');
  await keyboard('Confirmar orden (placa)');
  await says('order-status', 'Confirmada');
  assert.deepEqual([(await status('order-status')).icon, (await status('order-status')).kind], [true, '#i-ok']);
  await faceIs('Confirmado');
  await faceIs('Concentrada');
  assert.equal(await evaluate('history.length'), entries, 'cambiar de cara no llena el historial de Atrás');
});

test('S2: corte durante la espera deja la orden sin confirmar, aun con respuesta tardía', async () => {
  await open();
  await connect();
  await startStudy();
  await click('Pedir cara contenta');
  await says('order-status', 'Esperando confirmación');
  await click('Cortar enlace');
  const link = await status('link-status');
  assert.match(link.text, /Sin conexión/);
  assert.deepEqual([link.icon, link.kind], [true, '#i-off']);
  await says('order-status', 'no confirmada');
  await faceIs('Sin conexión');
  await click('Confirmar orden (placa)'); // la respuesta llega tarde
  await says('order-status', 'respuesta tardía');
  assert.match((await status('order-status')).text, /no confirmada/);
  assert.match(await face(), /^Sin conexión/);
  await click('Restablecer enlace');
  await says('link-status', 'Conectada');
  assert.match((await status('order-status')).text, /no confirmada/);
  await faceIs('Concentrada'); // la placa siguió contando la sesión
});

test('S3: si captura y aviso coinciden, manda el micrófono', async () => {
  for (const captureFirst of [true, false]) {
    await open();
    await connect();
    await login();
    const talk = async () => { await click('Chat', 'link'); await click('Hablar (simulado)'); };
    const loud = async () => { await click('EVA02', 'link'); await reading('0.9'); };
    for (const step of captureFirst ? [talk, loud] : [loud, talk]) await step();
    await faceIs('Capturando');
    const mic = await status('mic-status');
    assert.match(mic.text, /Micrófono en uso/);
    assert.deepEqual([mic.icon, mic.kind], [true, '#i-mic']);
    assert.match((await status('alert-status')).text, /supera el umbral/, 'el aviso no se pierde');
    assert.ok(await evaluate(`!!(document.getElementById('mic-status').compareDocumentPosition(
      document.getElementById('alert-status')) & Node.DOCUMENT_POSITION_FOLLOWING)`), 'el micrófono va antes');
    await sleep(3500); // vence la expresión del aviso; el micrófono sigue
    assert.match(await face(), /^Capturando/);
    await click('Detener captura');
    await faceIs('Normal');
  }
});

test('S4: lectura simulada sobre el umbral avisa con texto e icono', async () => {
  await open();
  await connect();
  assert.doesNotMatch(await bodyText(), /Historial/, 'sin ingreso no se ven datos');
  await login();
  assert.match(await bodyText(), /Todavía no hay lecturas/);
  await reading('0.6'); // igual al umbral: no lo supera
  assert.equal((await status('alert-status')).shown, false);
  await reading('0.82');
  await says('alert-status', 'Aviso: lectura simulada 0,82 supera el umbral 0,60');
  const alert = await status('alert-status');
  assert.deepEqual([alert.shown, alert.icon, alert.kind], [true, true, '#i-warn']);
  await faceIs('Sorprendida');
  await faceIs('Normal');
  assert.match(await evaluate(`document.querySelector('tbody').innerText`), /0,82[\s\S]*Aviso[\s\S]*0,60[\s\S]*Normal/);
  await click('Entendido');
  assert.equal((await status('alert-status')).shown, false);
  // El umbral cambia solo cuando la placa lo confirma.
  await fill('Umbral (0,01 a 1)', '0.9', 'spinbutton');
  await click('Pedir cambio de umbral');
  await says('order-status', 'Esperando confirmación');
  assert.match(await bodyText(), /Umbral vigente: 0,60/);
  await click('Confirmar orden (placa)');
  await says('threshold-now', '0,90');
  await reading('0.82');
  assert.equal((await status('alert-status')).shown, false);
});

test('estados de error: enlace fallido, ingreso rechazado, valores fuera de rango y rechazo', async () => {
  await open();
  await click('Hacer fallar el próximo enlace', 'checkbox');
  await click('Conectar placa simulada');
  await says('link-status', 'No se encontró la placa');
  await connect();
  await click('EVA02', 'link');
  await click('Ingresar');
  await says('login-error', 'Escribe usuario y contraseña');
  await click('Rechazar el próximo ingreso', 'checkbox');
  await fill('Usuario', 'x');
  await fill('Contraseña', 'x');
  await click('Ingresar');
  await says('login-error', 'incorrectos');
  await login();
  await reading('7');
  await says('reading-error', 'entre 0 y 1');
  await fill('Umbral (0,01 a 1)', '0', 'spinbutton');
  await click('Pedir cambio de umbral');
  await says('threshold-error', 'entre 0,01 y 1');
  await click('Estudio', 'link');
  await click('Iniciar estudio');
  await click('Rechazar orden (placa)');
  await says('order-status', 'rechazó');
  await click('Cortar enlace');
  await click('EVA02', 'link');
  assert.match(await bodyText(), /datos antiguos/);
  await click('Cerrar sesión');
  assert.doesNotMatch(await bodyText(), /Historial/);
});

test('móvil 375 px: ninguna pantalla desborda a lo ancho', async () => {
  await open(375, 667);
  await connect();
  await login();
  for (let i = 0; i < 25; i++) await reading('0.7');
  assert.match(await bodyText(), /20 más recientes de 25/);
  await click('Chat', 'link');
  await fill('Mensaje', 'a'.repeat(200));
  await click('Enviar');
  await says('chat-log', 'Respuesta simulada');
  for (const screen of ['Conexión', 'Estudio', 'Chat', 'Estado', 'EVA02']) {
    await click(screen, 'link');
    const [content, view] = await evaluate('[document.documentElement.scrollWidth, innerWidth]');
    assert.ok(content <= 375 && view === 375, `${screen}: ${content} px de contenido en ${view} px`);
  }
});

// A la vista de verdad: dentro de la ventana y sin nada pintado encima.
const seen = id => evaluate(`(() => { const e = document.getElementById('${id}'), r = e.getBoundingClientRect();
  return e.checkVisibility() && r.top >= 0 && r.bottom <= innerHeight
    && e.contains(document.elementFromPoint(r.x + r.width / 2, r.y + r.height / 2)); })()`);
const assertSeen = async (id, when) => assert.ok(await seen(id), `#${id} fuera de la vista ${when}`);
const text = id => evaluate(`document.getElementById('${id}').textContent`);

for (const [width, height] of [[375, 667], [320, 568], [667, 375]]) {
  test(`móvil ${width}×${height}: cada aviso queda a la vista junto al control usado`, async () => {
    await open(width, height);
    await connect();
    await click('Chat', 'link');
    await click('Hablar (simulado)');
    await assertSeen('mic-status', 'al pulsar Hablar');
    await login();
    await reading('0.9');
    await assertSeen('mic-status', 'al coincidir captura y aviso');
    await assertSeen('alert-status', 'al superar el umbral');
    await click('Estudio', 'link');
    await click('Iniciar estudio');
    await assertSeen('order-status', 'al pedir una orden');
    await click('Confirmar orden (placa)');
    await assertSeen('order-status', 'al confirmar desde el final de la página');
    await click('Cortar enlace');
    await assertSeen('link-status', 'al cortar desde el final de la página');
  });
}

// 320×256 equivale a un zoom del 400 %.
for (const [width, height] of [[375, 667], [320, 256]]) {
  test(`${width}×${height} con todos los avisos: dejan sitio y no tapan el foco (Tab y Mayús+Tab)`, async () => {
    await open(width, height);
    await connect();
    await login();
    await click('Chat', 'link');
    await click('Hablar (simulado)');
    await click('EVA02', 'link');
    await reading('0.9');
    await fill('Umbral (0,01 a 1)', '0.5', 'spinbutton');
    await click('Pedir cambio de umbral');
    const [tall, view] = await evaluate("[document.getElementById('notices').offsetHeight, innerHeight]");
    assert.ok(tall <= view * 0.6 + 1, `los avisos ocupan ${tall} de ${view} px`);
    await assertSeen('mic-status', 'con todos los avisos');
    for (const modifiers of [0, 8]) {
      for (let i = 0; i < 30; i++) {
        for (const type of ['keyDown', 'keyUp'])
          await send('Input.dispatchKeyEvent', { type, key: 'Tab', code: 'Tab', windowsVirtualKeyCode: 9, modifiers });
        const covered = await evaluate(`(() => { const e = document.activeElement;
          if (!e || e === document.body) return '';
          const r = e.getBoundingClientRect(), hit = document.elementFromPoint(r.x + r.width / 2, r.y + r.height / 2);
          return e.contains(hit) || hit?.contains(e) ? '' : e.outerHTML.slice(0, 80); })()`);
        assert.equal(covered, '', `foco tapado (${modifiers ? 'Mayús+Tab' : 'Tab'})`);
      }
    }
  });
}

test('un aviso que crece no tapa el control recién usado', async () => {
  await open(375, 667);
  await connect();
  await click('Chat', 'link');
  // Deja «Hablar» pegado bajo los avisos: al aparecer el del micrófono lo cubriría.
  await on(await find('Hablar (simulado)', 'button'), `function () {
    scrollBy(0, this.getBoundingClientRect().top - document.getElementById('notices').offsetHeight - 2);
    this.focus({ preventScroll: true });
  }`);
  await press('Enter', 13, '\r');
  await faceIs('Capturando');
  // 5 px: lo que sobresale el anillo de foco.
  await waitFor(`document.activeElement.getBoundingClientRect().top - 5
    >= document.getElementById('notices').getBoundingClientRect().bottom`, '«Hablar» y su anillo quedan libres', 2000);
});

test('más de dos decimales se rechaza: el aviso nunca compara valores que muestra iguales', async () => {
  await open();
  await connect();
  await login();
  await reading('0.604');
  await says('reading-error', 'dos decimales');
  assert.equal((await status('alert-status')).shown, false);
  assert.match(await bodyText(), /Todavía no hay lecturas/);
  await fill('Umbral (0,01 a 1)', '0.014', 'spinbutton');
  await click('Pedir cambio de umbral');
  await says('threshold-error', 'dos decimales');
  assert.match(await text('sim-pending'), /no tiene órdenes/);
  // Ningún valor legítimo de dos decimales debe caer por redondeo binario.
  assert.deepEqual(await evaluate(`['reading', 'threshold'].flatMap(id => {
    const input = document.getElementById(id), rejected = [];
    for (let i = id === 'reading' ? 0 : 1; i <= 100; i++) {
      input.value = (i / 100).toFixed(2);
      if (!input.validity.valid) rejected.push(id + ' ' + input.value);
    }
    return rejected; })`), []);
});

test('cerrar sesión cancela el cambio de umbral y no deja su valor a la vista', async () => {
  await open();
  await connect();
  await login();
  const request = async () => {
    await fill('Umbral (0,01 a 1)', '0.9', 'spinbutton');
    await click('Pedir cambio de umbral');
  };
  const nothingPrivate = async when => {
    for (const id of ['order-status', 'sim-pending', 'sum-order', 'sum-threshold'])
      assert.doesNotMatch(await text(id), /0,90/, `#${id} muestra el umbral ${when}`);
  };
  await request();
  await click('Cerrar sesión');
  await nothingPrivate('tras cerrar con la orden en espera');
  await says('order-status', 'canceló');
  await click('Confirmar orden (placa)');
  await nothingPrivate('tras el toque tardío en la placa');
  await login();
  assert.match(await bodyText(), /Umbral vigente: 0,60/);
  await request();
  await click('Confirmar orden (placa)');
  await says('threshold-now', '0,90');
  await click('Cerrar sesión');
  await nothingPrivate('tras cerrar con la orden ya confirmada');
});

test('los avisos de bloqueo desaparecen cuando se levanta el bloqueo', async () => {
  await open();
  await connect();
  await login();
  await click('Cortar enlace');
  await reading('0.5');
  await says('reading-error', 'Sin conexión');
  await fill('Umbral (0,01 a 1)', '0.5', 'spinbutton');
  await click('Pedir cambio de umbral');
  await says('threshold-error', 'Sin conexión');
  await click('Restablecer enlace');
  await says('link-status', 'Conectada');
  assert.deepEqual([await text('reading-error'), await text('threshold-error')], ['', '']);
  assert.doesNotMatch(await text('order-status'), /Sin conexión/);
  await click('Pedir cambio de umbral');
  await click('Pedir cambio de umbral');
  await says('threshold-error', 'otra orden');
  await click('Confirmar orden (placa)');
  assert.equal(await text('threshold-error'), '');
  // La nota de una captura anterior no acompaña a la siguiente.
  await click('Chat', 'link');
  await click('Hablar (simulado)');
  await click('Cortar enlace');
  await says('talk-help', 'Captura cancelada');
  await click('Restablecer enlace');
  assert.doesNotMatch(await text('talk-help'), /Captura cancelada/);
});

test('al vencer los 15 s de captura el foco no se pierde', async () => {
  await open(375, 667);
  await connect();
  await click('Chat', 'link');
  await click('Hablar (simulado)');
  await on(await find('Detener captura', 'button'), 'function () { this.focus(); }');
  await sleep(15500);
  assert.equal((await status('mic-status')).shown, false);
  assert.equal(await evaluate('document.activeElement.id'), 'notices');
});

test('gestos físicos de estudio y reinicio simulado', async () => {
  await open();
  await connect();
  await click('Toque breve');
  await faceIs('Concentrada');
  await click('Doble toque');
  await faceIs('En pausa');
  await click('Doble toque');
  await faceIs('Concentrada');
  await click('Toque largo');
  await faceIs('Capturando');
  await click('Reiniciar placa simulada');
  await faceIs('Sesión interrumpida');
  await click('Estudio', 'link');
  await says('study-text', 'Sesión interrumpida');
});

test('la confirmación física cuenta 15 s y vence sin aplicar el umbral', async () => {
  await open();
  await connect();
  await login();
  await fill('Umbral (0,01 a 1)', '0.8', 'spinbutton');
  await click('Pedir cambio de umbral');
  await faceIs('Esperando confirmación');
  await says('confirmation-clock', '15 s');
  await says('confirmation-clock', '14 s');
  await waitFor("document.getElementById('order-status').innerText.includes('no confirmada')", 'vencer la orden', 17000);
  await says('threshold-now', '0,60');
  await click('Confirmar orden (placa)');
  await says('order-status', 'respuesta tardía');
  await says('threshold-now', '0,60');
});

test('el boceto de la cara funciona descargado solo', async () => {
  const alone = await mkdtemp(join(tmpdir(), 'pet-boceto-'));
  try {
    const copy = join(alone, 'cara-mascota.html');
    await copyFile(new URL('./cara-mascota.html', import.meta.url), copy);
    const earlier = errors.length;
    await send('Page.navigate', { url: pathToFileURL(copy).href });
    await waitFor(`document.readyState === 'complete' && [...document.querySelectorAll('canvas')]
      .filter(c => c.getContext('2d').getImageData(0, 0, 128, 64).data.some((v, i) => i % 4 === 0 && v)).length === 12`,
    'doce caras dibujadas sin archivos vecinos', 3000);
    assert.deepEqual(errors.slice(earlier), []);
  } finally {
    await rm(alone, { recursive: true, force: true });
  }
});

test('sin dependencias externas ni errores de script', () => {
  assert.ok(requests.some(url => url.endsWith('/mockup.html')));
  assert.deepEqual(requests.filter(url => !/^(file|about|data):/.test(url)), []);
  assert.deepEqual(errors, []);
});
