# Cara de la mascota (V01, issue #3, parte 1)

**Estado:** estilo B elegido por Winderson y aprobado como base por Walther. Ajustes de la revisión incorporados al boceto y al mockup; falta validar en OLED físico.

**Boceto animado:** [`docs/diseno/cara-mascota.html`](../../diseno/cara-mascota.html) (descargar y abrir en el navegador). Imagen fija del boceto inicial: [`cara-mascota.png`](../../diseno/cara-mascota.png).

## Intención

Mascota tierna y simpática, con ojos expresivos y simples, que se vea profesional ante el equipo.

## Alcance

- Incluye: la cara en el OLED de 128×64, blanco y negro, y sus estados.
- [Mockup navegable de V01](../../diseno/mockup.html), con simulación de gestos, conexión y órdenes. Las transiciones avanzadas corresponden a S01; el dibujo en la placa requiere validación física.

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
| Contenta | Expresión temporal o final de sesión | Ojos en arco, mejillas y rebote |
| Enamorada | Expresión temporal, sin gesto propio | Ojos de corazón |
| Sorprendida | Ruido fuerte | Ojos grandes y signo de exclamación |
| Con sueño | Inactividad prolongada | Ojos entrecerrados, cabeceo y «z» |
| Concentrada | Estudio en curso, iniciado por toque breve en reposo o desde la app | Ceño suave y barra de avance |
| En pausa | Doble toque durante estudio o pausa desde la app; otro doble toque reanuda | Dos barras verticales |
| Sesión interrumpida | Reinicio con una sesión activa o en pausa | Triángulo de aviso |
| Esperando confirmación | Cambio protegido pendiente de toque físico (15 s) | Reloj en la cara y cuenta atrás visible en la app |
| Confirmado | La placa aceptó una orden | Contenta, marca de verificación y destellos |
| Sin conexión | No hay app enlazada | Cara triste y círculo tachado |
| Capturando | Captura de voz bajo demanda solicitada con toque largo o desde la app | Micrófono y ondas |

La medición local de ruido no activa la cara de captura. El toque breve confirma una orden protegida en espera en vez de iniciar estudio. El indicador de captura tiene prioridad sobre los demás iconos y adornos. Las expresiones temporales caducan y vuelven al estado de fondo.

## Escenarios

- **WHEN** la placa pierde la app **THEN** se ve el círculo tachado y la cara sigue funcionando.
- **WHEN** hay captura y otro aviso a la vez **THEN** se muestra el micrófono.
- **WHEN** termina una expresión temporal **THEN** vuelve al estado anterior vigente.

- **WHEN** pasan 15 s sin confirmar **THEN** desaparece la cuenta atrás y la orden queda sin aplicar.
- **WHEN** se reinicia la placa durante una sesión pausada o en curso **THEN** aparece «Sesión interrumpida».

## A cargo de quien lo implemente

Dibujar con figuras en el firmware (no animar con imágenes completas), elegir la biblioteca según el controlador verificado y ajustar tiempos y tamaños al ver el OLED real.
