// Simulación local del mockup V01: la "placa" y la "red" son los controles del panel lateral.
// Nada sale del navegador ni se guarda.
(() => {
  const $ = id => document.getElementById(id);
  const OFFLINE = 'Sin conexión';
  const SCREENS = ['conexion', 'estudio', 'chat', 'estado', 'eva02'];
  const ORDER_TIMEOUT_MS = 15000; // vencimiento de una confirmación según el plan (T04)
  const CAPTURE_LIMIT_MS = 15000; // máximo de captura según el diseño aprobado
  const FACE_NAMES = { normal: 'Normal', focus: 'Concentrada', paused: 'En pausa', interrupted: 'Sesión interrumpida', waiting: 'Esperando confirmación', confirmed: 'Confirmado', offline: OFFLINE, capture: 'Capturando', surprised: 'Sorprendida' };

  const state = {
    link: 'none', // none | connecting | on | lost | failed
    lostAt: 0,
    pending: null, // orden que la app espera
    board: null, // orden que la placa aún no responde; sobrevive al corte para poder llegar tarde
    sequence: 0,
    order: null, // último resultado: { icon, text, private }
    study: { phase: 'idle', ms: 0, since: 0 },
    capture: null, // temporizador de la captura en curso
    talkNote: '',
    flash: null, // expresión temporal: { face, until }
    user: '',
    threshold: 0.6,
    readings: [],
    readingCount: 0,
    alert: '',
    replying: false,
    blocked: new Set(), // errores de formulario causados por el estado
  };

  const now = () => performance.now();
  const format = value => value.toFixed(2).replace('.', ',');
  const setText = (element, text) => { if (element.textContent !== text) element.textContent = text; };
  const setDisabled = (element, disabled) => element.setAttribute('aria-disabled', String(disabled));
  const setStatus = (id, icon, text) => {
    const element = $(id);
    element.hidden = !text;
    element.querySelector('use').setAttribute('href', `#i-${icon}`);
    setText(element.querySelector('span'), text);
  };
  const flash = (face, ms) => { state.flash = { face, until: now() + ms }; };
  const staleSeconds = () => Math.round((now() - state.lostAt) / 1000);
  const screen = () => SCREENS.find(id => id === location.hash.slice(1)) ?? 'conexion';
  // Si el título quedó fuera o muy abajo, la vista vuelve a la navegación.
  const focusScreen = () => {
    const heading = $(`h-${screen()}`);
    const { top } = heading.getBoundingClientRect();
    if (top < $('notices').offsetHeight || top > innerHeight / 2) {
      scrollTo(0, document.querySelector('nav').getBoundingClientRect().top + scrollY);
    }
    heading.focus({ preventScroll: true });
  };
  // Si un aviso con el foco desaparece (clic, vencimiento o corte), el foco queda en la zona de avisos.
  const showNotice = (id, shown) => {
    const element = $(id);
    if (!shown && element.contains(document.activeElement)) $('notices').focus({ preventScroll: true });
    element.hidden = !shown;
  };

  // Errores que dependen del estado y no de lo escrito: se recalculan y se borran solos.
  const BLOCKERS = {
    'reading-error': () => state.link === 'on' ? '' : `${OFFLINE}: no llegan lecturas. Conecta la placa.`,
    'threshold-error': () => {
      if (state.link !== 'on') return `${OFFLINE}: no se puede cambiar el umbral.`;
      return state.pending ? 'Hay otra orden en espera.' : '';
    },
  };
  const isBlocked = id => {
    if (!BLOCKERS[id]()) return false;
    state.blocked.add(id);
    return true;
  };
  const showError = (id, text) => {
    state.blocked.delete(id);
    setText($(id), text);
  };
  const renderBlocked = () => {
    for (const id of state.blocked) {
      const text = BLOCKERS[id]();
      setText($(id), text);
      if (!text) state.blocked.delete(id);
    }
  };

  // El micrófono manda sobre todo; después el enlace, lo temporal y el estado de fondo.
  // @spec:01-foundation-v01-lenguaje-visual-expresiones-y-flujos-de-la-interfaz
  const faceKey = () => {
    if (state.capture) return 'capture';
    if (state.link !== 'on') return 'offline';
    if (state.flash && now() < state.flash.until) return state.flash.face;
    if (state.pending?.physical) return 'waiting';
    return { running: 'focus', paused: 'paused', interrupted: 'interrupted' }[state.study.phase] ?? 'normal';
  };

  // --- Órdenes ---
  const abandonOrder = reason => {
    if (!state.pending) return;
    const { timer, label, private: isPrivate } = state.pending;
    clearTimeout(timer);
    state.order = { icon: 'off', text: `${reason} La orden «${label}» quedó no confirmada.`, private: isPrivate };
    state.pending = null;
  };

  // `private`: la orden nombra datos que solo se ven con sesión.
  // @spec:01-foundation-v01-lenguaje-visual-expresiones-y-flujos-de-la-interfaz
  const sendOrder = ({ label, apply, physical = false, private: isPrivate = false }) => {
    if (state.link !== 'on' || state.pending) return;
    const id = ++state.sequence;
    const timer = setTimeout(() => { abandonOrder('La placa no respondió a tiempo.'); render(); }, ORDER_TIMEOUT_MS);
    state.pending = { id, label, apply, timer, physical, expiresAt: now() + ORDER_TIMEOUT_MS, private: isPrivate };
    state.board = { id, label, private: isPrivate };
    state.order = { icon: 'wait', text: `Esperando confirmación de la placa: «${label}».`, private: isPrivate };
  };

  // Solo la orden que sigue en espera puede confirmarse: una respuesta tardía se descarta.
  // @spec:01-foundation-v01-lenguaje-visual-expresiones-y-flujos-de-la-interfaz
  const receiveReply = ({ id, label, private: isPrivate }, applied) => {
    const report = (icon, text) => { state.order = { icon, text, private: isPrivate }; };
    if (state.pending?.id !== id) {
      report('off', `Llegó una respuesta tardía y se descartó: «${label}» sigue no confirmada.`);
      return;
    }
    if (now() >= state.pending.expiresAt) {
      abandonOrder('La confirmación venció.');
      return;
    }
    const { apply, timer } = state.pending;
    clearTimeout(timer);
    state.pending = null;
    if (!applied) {
      report('warn', `La placa rechazó «${label}»: no se aplicó. Puedes pedirla otra vez.`);
      return;
    }
    apply();
    flash('confirmed', 2000);
    report('ok', `Confirmada por la placa: «${label}».`);
  };

  const boardReply = applied => {
    const order = state.board;
    if (!order) return;
    state.board = null;
    receiveReply(order, applied);
  };

  // Lo pedido con sesión no sobrevive a ella: ni se aplica ni deja su valor a la vista.
  const dropPrivateOrders = () => {
    if (state.board?.private) state.board = null;
    if (state.pending?.private) {
      clearTimeout(state.pending.timer);
      state.pending = null;
      state.order = { icon: 'off', text: 'Cerraste sesión: se canceló el cambio de umbral sin aplicarlo.' };
    } else if (state.order?.private) {
      state.order = null;
    }
  };

  // --- Estudio ---
  const studyElapsed = () => {
    const { phase, ms, since } = state.study;
    // Sin enlace se congela lo último que supo la app.
    const until = state.link === 'on' ? now() : state.lostAt;
    return phase === 'running' ? ms + Math.max(0, until - since) : ms;
  };
  const running = ms => { state.study = { phase: 'running', ms, since: now() }; };
  const stopped = phase => { state.study = { phase, ms: studyElapsed(), since: 0 }; };
  const ORDERS = {
    start: { from: ['idle', 'finished', 'interrupted'], apply: () => running(0) },
    pause: { from: ['running'], apply: () => stopped('paused') },
    resume: { from: ['paused'], apply: () => running(state.study.ms) },
    finish: { from: ['running', 'paused'], apply: () => stopped('finished') },
    happy: { from: ['idle', 'running', 'paused', 'finished', 'interrupted'], apply: () => {} }, // solo cambia la cara
  };
  const canOrder = key => state.link === 'on' && !state.pending && ORDERS[key].from.includes(state.study.phase);

  // --- Enlace y captura ---
  const linkUp = () => {
    state.link = 'on';
    state.talkNote = '';
  };
  const stopCapture = note => {
    clearTimeout(state.capture);
    state.capture = null;
    state.talkNote = note;
  };

  const connect = () => {
    if (state.link === 'on' || state.link === 'connecting') return;
    state.link = 'connecting';
    setTimeout(() => {
      if ($('fail-link').checked) state.link = 'failed';
      else linkUp();
      $('fail-link').checked = false;
      render();
    }, 400);
  };

  // @spec:01-foundation-v01-lenguaje-visual-expresiones-y-flujos-de-la-interfaz
  const cutLink = () => {
    if (state.link !== 'on') return;
    state.link = 'lost';
    state.lostAt = now();
    state.flash = null;
    abandonOrder('Se cortó el enlace.');
    if (state.capture) stopCapture('Captura cancelada: se perdió el enlace.');
  };

  const toggleTalk = () => {
    if (state.capture) {
      stopCapture('Captura detenida.');
    } else if (state.link === 'on') {
      state.talkNote = '';
      state.capture = setTimeout(() => { stopCapture('Captura detenida al llegar a 15 s.'); render(); }, CAPTURE_LIMIT_MS);
    }
  };

  // Las confirmaciones físicas tienen prioridad frente a los gestos de estudio o voz.
  const boardGesture = gesture => {
    if (state.link !== 'on') return;
    if (state.pending?.physical) {
      if (gesture === 'short') boardReply(true);
      return;
    }
    if (gesture === 'short' && ORDERS.start.from.includes(state.study.phase)) running(0);
    if (gesture === 'double' && state.study.phase === 'running') stopped('paused');
    else if (gesture === 'double' && state.study.phase === 'paused') running(state.study.ms);
    if (gesture === 'long' && !state.capture) toggleTalk();
  };

  const restartBoard = () => {
    if (state.link !== 'on') return;
    if (ORDERS.finish.from.includes(state.study.phase)) stopped('interrupted');
    abandonOrder('Se reinició la placa simulada.');
    state.board = null;
    state.flash = null;
    if (state.capture) stopCapture('Captura cancelada: se reinició la placa.');
  };

  // --- Chat ---
  const addMessage = text => {
    const item = document.createElement('li');
    item.textContent = text;
    $('chat-log').append(item);
    $('chat-empty').hidden = true;
  };

  const sendMessage = () => {
    const text = $('message').value.trim();
    if (state.replying) return;
    setText($('chat-error'), text ? '' : 'Escribe un mensaje antes de enviar.');
    if (!text) return;
    addMessage(`Tú: ${text}`);
    $('message').value = '';
    state.replying = true;
    setTimeout(() => {
      state.replying = false;
      addMessage('Respuesta simulada: recibí tu mensaje. No se envió a ningún proveedor.');
      render();
    }, 600);
  };

  // --- EVA02 ---
  const renderHistory = () => {
    const rows = state.readings.map(({ number, value, threshold }) => {
      const row = document.createElement('tr');
      for (const text of [String(number), format(value), format(threshold), value > threshold ? 'Aviso' : 'Normal']) {
        const cell = document.createElement('td');
        cell.textContent = text;
        row.append(cell);
      }
      return row;
    });
    $('history').replaceChildren(...rows);
    let note = 'Todavía no hay lecturas. Envía la primera con el formulario de arriba.';
    if (state.readingCount > state.readings.length) {
      note = `Se muestran las ${state.readings.length} más recientes de ${state.readingCount}.`;
    } else if (state.readingCount) {
      note = `Lecturas en esta sesión: ${state.readingCount}.`;
    }
    setText($('history-note'), note);
  };

  const login = () => {
    const user = $('user').value.trim();
    let error = '';
    if (!user || !$('password').value) {
      error = 'Escribe usuario y contraseña.';
    } else if ($('fail-login').checked) {
      error = 'Usuario o contraseña incorrectos (rechazo simulado). Vuelve a intentarlo.';
      $('fail-login').checked = false;
    }
    setText($('login-error'), error);
    if (error) return;
    // La contraseña no se guarda ni se compara: solo se borra del campo.
    $('password').value = '';
    state.user = user;
    render();
    $('h-readings').focus();
  };

  const logout = () => {
    Object.assign(state, { user: '', alert: '', readings: [], readingCount: 0, flash: null });
    dropPrivateOrders();
    for (const id of ['login-form', 'reading-form', 'threshold-form']) $(id).reset();
    for (const id of Object.keys(BLOCKERS)) showError(id, '');
    renderHistory();
    render();
    $('user').focus();
  };

  // El paso 0,01 lo valida la plataforma (el formulario es novalidate): se compara lo mismo que se muestra.
  const numberIn = id => {
    const input = $(id);
    return input.validity.valid ? input.valueAsNumber : Number.NaN;
  };

  // @spec:01-foundation-v01-lenguaje-visual-expresiones-y-flujos-de-la-interfaz
  const addReading = () => {
    if (isBlocked('reading-error')) return;
    const value = numberIn('reading');
    const invalid = Number.isNaN(value);
    showError('reading-error', invalid ? 'Escribe un valor entre 0 y 1, con dos decimales como máximo.' : '');
    if (invalid) return;
    const { threshold } = state;
    state.readingCount += 1;
    // shortcut: solo 20 filas en memoria; paginar y persistir cuando exista el historial real (T05).
    state.readings = [{ number: state.readingCount, value, threshold }, ...state.readings].slice(0, 20);
    if (value > threshold) {
      state.alert = `Aviso: lectura simulada ${format(value)} supera el umbral ${format(threshold)}.`;
      flash('surprised', 3000);
    }
    renderHistory();
  };

  const requestThreshold = () => {
    const value = numberIn('threshold');
    if (Number.isNaN(value)) {
      showError('threshold-error', 'El umbral debe estar entre 0,01 y 1, con dos decimales como máximo.');
      return;
    }
    if (isBlocked('threshold-error')) return;
    showError('threshold-error', '');
    sendOrder({ label: `Cambiar umbral a ${format(value)}`, apply: () => { state.threshold = value; }, physical: true, private: true });
  };

  // --- Pintado ---
  const LINK = {
    none: ['off', `${OFFLINE}: no hay placa enlazada.`, 'Todavía no hay ninguna placa enlazada. Conéctala para ver su cara y darle órdenes.'],
    connecting: ['wait', 'Conectando…', 'Buscando la placa simulada…'],
    on: ['ok', 'Conectada a la placa simulada.', 'Enlazada con «Mascota de prueba». Una orden solo cuenta cuando la placa la confirma.'],
    lost: ['off', `${OFFLINE}: se cortó el enlace.`, 'La placa sigue funcionando sola. Restablece el enlace o vuelve a conectar.'],
    failed: ['off', `${OFFLINE}: No se encontró la placa.`, 'Revisa que esté encendida y en la misma red, y reintenta.'],
  };
  const STUDY = { idle: 'No hay sesión en curso.', running: 'Sesión en curso', paused: 'Sesión en pausa', finished: 'Sesión terminada', interrupted: 'Sesión interrumpida' };

  const staleText = () => {
    if (state.link === 'on') return '';
    // lostAt y no el estado: un reintento fallido tras el corte sigue mostrando datos antiguos.
    if (state.lostAt) return `${OFFLINE}: datos antiguos, de hace ${staleSeconds()} s.`;
    return `${OFFLINE}: conecta la placa en la pantalla Conexión.`;
  };

  const studyText = () => {
    const { phase } = state.study;
    if (phase === 'idle') return STUDY.idle;
    const seconds = Math.floor(studyElapsed() / 1000);
    const clock = [Math.floor(seconds / 60), seconds % 60].map(n => String(n).padStart(2, '0')).join(':');
    return `${STUDY[phase]}: ${clock}.`;
  };

  const renderFace = () => {
    const key = faceKey();
    const oled = $('face');
    if (oled.dataset.face !== key) {
      oled.dataset.face = key;
      // replace y no src: cambiar de cara no debe llenar el historial del botón Atrás.
      oled.contentWindow.location.replace(new URL(`cara-mascota.html#${key}`, location.href).href);
    }
    setText($('face-name'), `OLED simulado 128×64 · ${FACE_NAMES[key]}`);
    setText($('sum-face'), FACE_NAMES[key]);
  };

  const renderStatus = () => {
    const [icon, text, help] = LINK[state.link];
    setStatus('link-status', icon, text);
    setText($('link-help'), help);
    setStatus('order-status', state.order?.icon ?? 'wait', state.order?.text ?? '');
    const clock = $('confirmation-clock');
    clock.hidden = !state.pending?.physical;
    const seconds = clock.hidden ? 0 : Math.max(0, Math.ceil((state.pending.expiresAt - now()) / 1000));
    setText(clock, clock.hidden ? '' : `Tiempo restante para tocar la placa: ${seconds} s.`);
    showNotice('mic-status', Boolean(state.capture));
    showNotice('alert-status', Boolean(state.alert));
    setText($('alert-status').querySelector('span'), state.alert);
  };

  const renderControls = () => {
    setDisabled($('connect'), state.link === 'on' || state.link === 'connecting');
    for (const button of document.querySelectorAll('[data-order]')) setDisabled(button, !canOrder(button.dataset.order));
    setDisabled($('send'), state.replying);
    setDisabled($('talk'), !state.capture && state.link !== 'on');
    $('talk').setAttribute('aria-pressed', String(Boolean(state.capture)));
    setDisabled($('reading-send'), state.link !== 'on');
    setDisabled($('threshold-send'), state.link !== 'on' || Boolean(state.pending));
    setDisabled($('reply-ok'), !state.board);
    setDisabled($('reply-no'), !state.board);
    setDisabled($('cut'), state.link !== 'on');
    setDisabled($('restore'), state.link !== 'lost');
    setText($('sim-pending'), state.board ? `La placa aún no responde: «${state.board.label}».` : 'La placa no tiene órdenes por responder.');
  };

  const renderTexts = () => {
    const stale = staleText();
    setText($('study-text'), studyText());
    const phase = state.study.phase;
    $('study-icon').hidden = phase !== 'paused' && phase !== 'interrupted';
    $('study-icon').querySelector('use').setAttribute('href', phase === 'paused' ? '#i-pause' : '#i-warn');
    setText($('study-link'), stale);
    setText($('talk-help'), state.link === 'on' || state.capture
      ? `${state.talkNote} No se usa tu micrófono; la captura simulada dura 15 s como máximo.`.trim()
      : `${state.talkNote} Para hablar hace falta la placa conectada; puedes escribir.`.trim());
    setText($('sum-link'), stale || LINK.on[1]);
    setText($('sum-study'), studyText());
    setText($('sum-order'), state.order?.text ?? 'No hay ninguna orden que mostrar.');
  };

  const renderEva = () => {
    $('login-form').hidden = Boolean(state.user);
    $('private').hidden = !state.user;
    const last = state.readings[0];
    const lastText = last ? `${format(last.value)} (simulada)` : 'Todavía no hay lecturas.';
    setText($('session'), state.user && `Sesión simulada de ${state.user}.`);
    setText($('reading-now'), `${staleText()} Última lectura: ${lastText}`.trim());
    setText($('threshold-now'), `Umbral vigente: ${format(state.threshold)}`);
    setText($('sum-threshold'), state.user ? format(state.threshold) : 'Inicia sesión en EVA02 para verlo.');
    setText($('sum-reading'), state.user ? lastText : 'Inicia sesión en EVA02 para verla.');
  };

  const render = () => {
    renderFace();
    renderStatus();
    renderControls();
    renderTexts();
    renderEva();
    renderBlocked();
  };

  const route = () => {
    for (const id of SCREENS) $(id).hidden = id !== screen();
    for (const link of document.querySelectorAll('nav a')) {
      if (link.hash === `#${screen()}`) link.setAttribute('aria-current', 'page');
      else link.removeAttribute('aria-current');
    }
  };

  // --- Eventos ---
  const handle = (id, type, action) => $(id).addEventListener(type, event => {
    event.preventDefault();
    action();
    render();
  });
  handle('connect', 'click', connect);
  handle('cut', 'click', cutLink);
  handle('restore', 'click', () => { if (state.link === 'lost') linkUp(); });
  handle('reply-ok', 'click', () => boardReply(true));
  handle('reply-no', 'click', () => boardReply(false));
  handle('board-short', 'click', () => boardGesture('short'));
  handle('board-double', 'click', () => boardGesture('double'));
  handle('board-long', 'click', () => boardGesture('long'));
  handle('board-restart', 'click', restartBoard);
  handle('talk', 'click', toggleTalk);
  handle('chat-form', 'submit', sendMessage);
  handle('login-form', 'submit', login);
  handle('logout', 'click', logout);
  handle('reading-form', 'submit', addReading);
  handle('threshold-form', 'submit', requestThreshold);
  handle('mic-stop', 'click', () => stopCapture('Captura detenida.'));
  handle('alert-ok', 'click', () => { state.alert = ''; });
  for (const button of document.querySelectorAll('[data-order]')) {
    button.addEventListener('click', () => {
      const key = button.dataset.order;
      if (canOrder(key)) sendOrder({ label: button.textContent, apply: ORDERS[key].apply });
      render();
    });
  }
  addEventListener('hashchange', () => { route(); render(); focusScreen(); });

  // Los avisos fijos no deben tapar el foco: ni al desplazarse ni cuando crecen.
  const FOCUS_RING_PX = 8; // el anillo de foco sobresale del control y también debe verse
  new ResizeObserver(() => {
    const notices = $('notices');
    document.documentElement.style.scrollPaddingTop = `${notices.offsetHeight + FOCUS_RING_PX}px`;
    const focused = document.activeElement;
    if (!focused || focused === document.body || notices.contains(focused)) return;
    const { top, bottom } = focused.getBoundingClientRect();
    if (top < notices.getBoundingClientRect().bottom + FOCUS_RING_PX && bottom > 0) focused.scrollIntoView({ block: 'nearest' });
  }).observe($('notices'));

  route();
  renderHistory();
  render();
  // shortcut: repinta todo cada 100 ms; basta para un prototipo (cara, reloj y datos antiguos).
  setInterval(render, 100);
})();
