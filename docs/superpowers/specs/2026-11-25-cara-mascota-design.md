# Cara de la mascota (V01, issue #3, parte 1)

**Estado:** primer boceto aprobado por Winderson el 25-11-2026, ajustable más adelante. Falta la revisión de Walther.

**Boceto animado:** [`docs/diseno/cara-mascota.html`](../../diseno/cara-mascota.html) (descargar y abrir en el navegador). Imagen fija: [`cara-mascota.png`](../../diseno/cara-mascota.png).

## Intención

Mascota tierna y simpática, con ojos expresivos y simples, que se vea profesional ante el equipo.

## Alcance

- Incluye: la cara en el OLED de 128×64, blanco y negro, y sus estados.
- No incluye todavía: pantallas de la app (conexión, estudio, chat, estado), que son el resto de V01; animaciones de transición (S01); firmware.

## Alternativas

| Estilo | A favor | En contra |
|---|---|---|
| A: ojos grandes con brillos y boca | La más tierna | Más trabajo de animación |
| **B: ojos de bloque redondeado, sin boca (elegida)** | Limpia, legible y fácil de programar | Menos recursos para expresar |
| C: carita con mejillas y boca | Muy simpática | Los detalles finos se pierden en 1,3" |

Se eligió B para que se vea profesional.

## Estados

| Estado | Cuándo | Cómo se distingue |
|---|---|---|
| Normal | Reposo | Bloques altos y lisos; mira a los lados y parpadea |
| Contenta | Toque | Ojos en arco, mejillas y rebote |
| Enamorada | Caricia larga | Ojos de corazón |
| Sorprendida | Ruido fuerte | Ojos grandes y signo de exclamación |
| Con sueño | Inactividad prolongada | Ojos entrecerrados, cabeceo y «z» |
| Concentrada | Sesión de estudio | Ceño y barra de avance |
| Confirmado | La placa aceptó una orden | Contenta, marca de verificación y destellos |
| Sin conexión | No hay app enlazada | Cara triste y círculo tachado |
| Capturando | Micrófono en uso | Micrófono y ondas |

El indicador de captura tiene prioridad sobre los demás iconos y adornos. Las expresiones temporales caducan y vuelven al estado de fondo.

## Escenarios

- **WHEN** la placa pierde la app **THEN** se ve el círculo tachado y la cara sigue funcionando.
- **WHEN** hay captura y otro aviso a la vez **THEN** se muestra el micrófono.
- **WHEN** termina una expresión temporal **THEN** vuelve al estado anterior vigente.

## A cargo de quien lo implemente

Dibujar con figuras en el firmware (no animar con imágenes completas), elegir la biblioteca según el controlador verificado y ajustar tiempos y tamaños al ver el OLED real.
