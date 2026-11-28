// Render monocromo de la cara (128×64, estilo B), compartido por cara-mascota.html y mockup.html.
// Script clásico y no módulo: así abre también desde file://.
const PetFace = (() => {
  // Búfer de 1 bit: se pinta por píxel para que no haya suavizado, igual que en el OLED.
  let buf;
  const paint = (x0, y0, x1, y1, v, inside) => {
    for (let y = Math.max(0, Math.trunc(y0)); y <= Math.min(63, y1); y++)
      for (let x = Math.max(0, Math.trunc(x0)); x <= Math.min(127, x1); x++)
        if (inside(x, y)) buf[y * 128 + x] = v;
  };
  const box = (cx, cy, w, h, r, v = 1) => paint(cx - w / 2 - 1, cy - h / 2 - 1, cx + w / 2 + 1, cy + h / 2 + 1, v, (x, y) => {
    if (Math.abs(x - cx) > w / 2 || Math.abs(y - cy) > h / 2) return false;
    const rr = Math.min(r, w / 2, h / 2);
    const dx = Math.max(Math.abs(x - cx) - (w / 2 - rr), 0), dy = Math.max(Math.abs(y - cy) - (h / 2 - rr), 0);
    return dx * dx + dy * dy <= rr * rr;
  });
  const ell = (cx, cy, rx, ry, v = 1) => paint(cx - rx, cy - ry, cx + rx, cy + ry, v, (x, y) => ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1);
  const line = (ax, ay, bx, by, t, v = 1) => paint(Math.min(ax, bx) - t, Math.min(ay, by) - t, Math.max(ax, bx) + t, Math.max(ay, by) + t, v, (x, y) => {
    const dx = bx - ax, dy = by - ay;
    const k = Math.max(0, Math.min(1, ((x - ax) * dx + (y - ay) * dy) / (dx * dx + dy * dy || 1)));
    return Math.hypot(x - ax - k * dx, y - ay - k * dy) <= t / 2;
  });

  // Ojo liso: bloque redondeado.
  const eye = (cx, cy, w, h) => {
    box(cx, cy, w, h, 10);
  };
  const pair = (cx, cy, w, h) => { eye(40 + cx, cy, w, h); eye(88 + cx, cy, w, h); };
  // Parpadeo: 1 = abierto, cae a 0.1 durante un instante cada ciclo.
  const blink = (t, every = 3200) => { const p = t % every; return p < 180 ? Math.max(0.1, Math.abs(p - 90) / 90) : 1; };
  const joy = (t) => {
    const b = Math.sin(t / 140) * 2;
    for (const x of [40, 88]) { box(x, 30 + b, 30, 28, 12); ell(x, 46 + b, 23, 17, 0); }
    for (const x of [14, 114]) for (let i = 0; i < 3; i++) line(x - 6 + i * 5, 50, x - 9 + i * 5, 55, 1);
  };
  const star = (x, y, s) => { line(x - s, y, x + s, y, 1); line(x, y - s, x, y + s, 1); };
  const heart = (x, y) => { ell(x - 3, y, 3.5, 3.5); ell(x + 3, y, 3.5, 3.5); for (let i = 0; i <= 7; i++) line(x - 6 + i, y + 1 + i, x + 6 - i, y + 1 + i, 1); };

  // @spec:01-foundation-v01-lenguaje-visual-expresiones-y-flujos-de-la-interfaz
  const faces = {
    normal: { label: 'Normal: mira a los lados y parpadea', draw: t => {
      const look = [0, 0, -8, -8, 0, 8, 8, 0][Math.floor(t / 900) % 8];
      pair(look, 32, 28, 36 * blink(t));
    } },
    happy: { label: 'Contenta: rebota y se sonroja', draw: t => joy(t) },
    love: { label: 'Enamorada: caricia larga', draw: t => {
      const s = 1 + Math.sin(t / 160) * 0.12;
      for (const x of [40, 88]) {
        paint(x - 18, 14, x + 18, 50, 1, (px, py) => {
          const a = (px - x) / (14 * s), b = -(py - 30) / (14 * s);
          return (a * a + b * b - 1) ** 3 - a * a * b * b * b <= 0;
        });
      }
      heart(64, 52 - (t / 60) % 40);
    } },
    surprised: { label: 'Sorprendida: salto', draw: t => {
      const k = t % 2000 < 200 ? 1.15 : 1;
      pair(0, 33, 30 * k, 44 * k);
      line(64, 4, 64, 14, 3); ell(64, 21, 2, 2);
    } },
    sleepy: { label: 'Con sueño: cabecea', draw: t => {
      const y = 38 + Math.sin(t / 700) * 3;
      pair(0, y, 30, 18); box(40, y - 9, 34, 14, 0, 0); box(88, y - 9, 34, 14, 0, 0);
      const p = (t / 90) % 24;
      for (const [x0, y0, s] of [[104, 22, 4], [114, 12, 6]]) {
        const yy = y0 - p / 4;
        line(x0, yy, x0 + s, yy, 1); line(x0 + s, yy, x0, yy + s, 1); line(x0, yy + s, x0 + s, yy + s, 1);
      }
    } },
    focus: { label: 'Concentrada: ceño y avance', draw: t => {
      const h = 30 * blink(t, 4500);
      pair(0, 30, 28, h);
      // Párpados y cejas casi horizontales (3 px de caída): concentración sin parecer enojo.
      for (let i = 0; i < 12; i++) { line(24, 11 + i, 58, 14 + i, 1, 0); line(70, 14 + i, 104, 11 + i, 1, 0); }
      line(24, 13, 56, 16, 3); line(72, 16, 104, 13, 3);
      box(64, 58, 120, 1, 0); box(4 + ((t / 80) % 120) / 2, 58, (t / 80) % 120, 5, 1);
    } },
    confirmed: { label: 'Confirmado: celebra', draw: t => {
      joy(t);
      line(55, 9, 61, 15, 3); line(61, 15, 73, 3, 3);
      if (t % 600 < 300) { star(12, 12, 4); star(116, 16, 3); } else { star(22, 22, 2); star(108, 8, 5); }
    } },
    offline: { label: 'Sin conexión: carita triste', draw: t => {
      pair(0, 36, 26, 28 * blink(t));
      for (let i = 0; i < 12; i++) { line(24, 24 - i, 56, 14 - i, 1, 0); line(72, 14 - i, 104, 24 - i, 1, 0); }
      ell(117, 9, 8, 8); ell(117, 9, 6, 6, 0); line(111, 15, 123, 3, 2);
    } },
    capture: { label: 'Capturando: atenta, con ondas', draw: t => {
      pair(-8, 34, 24, 34 * blink(t));
      box(116, 8, 8, 13, 4); line(110, 11, 114, 17, 1); line(114, 17, 118, 17, 1); line(118, 17, 122, 11, 1); line(116, 17, 116, 22, 1); line(112, 22, 120, 22, 1);
      for (let i = 0; i < 4; i++) { const h = 4 + Math.abs(Math.sin(t / 110 + i * 1.7)) * 22; box(108 + i * 5, 42, 2, h, 1); }
    } },
  };

  // Con movimiento reducido se dibuja siempre el mismo instante.
  const still = matchMedia('(prefers-reduced-motion: reduce)').matches;

  // Dibuja la cara `key` en un canvas de 128×64.
  const draw = (canvas, key, t) => {
    buf = new Uint8Array(128 * 64);
    faces[key].draw(still ? 1000 : t);
    const g = canvas.getContext('2d');
    const img = g.createImageData(128, 64);
    for (let i = 0; i < buf.length; i++) img.data.set(buf[i] ? [255, 255, 255, 255] : [0, 0, 0, 255], i * 4);
    g.putImageData(img, 0, 0);
  };

  return { faces, draw, still };
})();
