// Prueba del mockup V01 en Chromium headless, sin dependencias (Node >= 22, DevTools Protocol).
// Uso: node docs/diseno/probar-mockup.mjs   (variable CHROMIUM para otro ejecutable)
// Los controles se buscan por rol y nombre accesible y se accionan con ratón o teclado reales.
import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { once } from 'node:events';
import { mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, test } from 'node:test';
import { setTimeout as sleep } from 'node:timers/promises';

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
  await waitFor(`document.readyState === 'complete' && !!document.querySelector('canvas[aria-label]')`, 'cargar mockup.html con la cara', 3000);
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
  (await send('Runtime.callFunctionOn', { objectId, functionDeclaration: fn, returnByValue: true })).result.value;

const click = async (name, role = 'button') => {
  const box = await on(await find(name, role), `function () {
    this.scrollIntoView({ block: 'center' });
    const r = this.getBoundingClientRect(), x = r.x + r.width / 2, y = r.y + r.height / 2;
    return { x, y, hit: this.contains(document.elementFromPoint(x, y)) };
  }`);
  assert.ok(box.hit, `«${name}» está tapado y no se puede pulsar`);
  for (const type of ['mousePressed', 'mouseReleased'])
    await send('Input.dispatchMouseEvent', { type, x: box.x, y: box.y, button: 'left', clickCount: 1 });
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

const face = () => evaluate(`document.querySelector('canvas').getAttribute('aria-label')`);
const faceIs = (start, timeout) =>
  waitFor(`document.querySelector('canvas').getAttribute('aria-label').startsWith('Cara: ${start}')`, `cara «${start}»`, timeout);
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
  await keyboard('Pedir cara contenta');
  await says('order-status', 'Esperando confirmación');
  assert.equal((await status('order-status')).kind, '#i-wait');
  assert.match(await face(), /^Cara: Concentrada/, 'sin respuesta no se muestra éxito');
  await keyboard('Confirmar orden (placa)');
  await says('order-status', 'Confirmada');
  assert.deepEqual([(await status('order-status')).icon, (await status('order-status')).kind], [true, '#i-ok']);
  await faceIs('Confirmado');
  await faceIs('Concentrada');
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
  assert.match(await face(), /^Cara: Sin conexión/);
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
    assert.match(await face(), /^Cara: Capturando/);
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

test('sin dependencias externas ni errores de script', () => {
  assert.ok(requests.some(url => url.endsWith('/mockup.html')));
  assert.deepEqual(requests.filter(url => !/^(file|about|data):/.test(url)), []);
  assert.deepEqual(errors, []);
});
