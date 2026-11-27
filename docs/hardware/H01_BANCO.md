# H01 · Banco de pruebas por componente

Procedimiento para [#7 H01](https://github.com/wmrDev/agentic-iot-pet/issues/7). El firmware de banco está en `firmware/diagnostics/h01/` y prueba **un módulo por carga**, como pide H01. No es firmware de producto.

**Ningún GPIO viene configurado.** Cada pin empieza en `-1` y la prueba no arranca hasta que se configura el pin verificado en la placa real. El firmware rechaza:

- GPIO19 y GPIO20, que usa la consola USB;
- GPIO26 a GPIO32, conectados a la flash y la PSRAM internas del encapsulado.

Además avisa, sin bloquear la prueba, en estos casos:

- GPIO33 a GPIO37: quedan reservados si el módulo tiene PSRAM octal. El [inventario](INVENTARIO.md) informa 2 MB de PSRAM, que es cuádruple, así que deberían estar libres; confirmar la variante antes de usarlos.
- Pines de arranque (0, 3, 45 o 46). Evitar GPIO45 y GPIO46 salvo que no quede otra opción: un módulo que los fuerce al arrancar cambia la tensión de la flash (GPIO45) o el modo de arranque (GPIO46).

## Antes de energizar

Por cada módulo, y siempre con la placa desconectada:

1. Fotografiar el módulo con su serigrafía legible.
2. Medir continuidad de cada jumper entre el pin de la placa y el pin del módulo.
3. Comprobar que no hay continuidad entre 3V3 y GND, ni entre 5V y GND.
4. Conectar solo ese módulo. Desconectar los jumpers del resto.

### Alimentación por módulo

Los GPIO del ESP32-S3 no toleran 5 V. Ningún módulo cuya salida llegue a un GPIO se alimenta a 5 V.

| Módulo | Alimentación | Conexiones obligatorias |
|---|---|---|
| OLED 128×64 (I2C) | 3V3 de la placa, nunca 5 V | GND común. Con 5 V las resistencias de pull-up del módulo llevarían SDA y SCL a 5 V |
| TTP223 | 3V3 de la placa, nunca 5 V | GND común. Su salida I/O va directa al GPIO |
| INMP441 | 3V3 de la placa, nunca 5 V (máximo absoluto 3,6 V) | GND común. **L/R a GND** para usar el canal izquierdo que lee el firmware |
| MAX98357A | Según su módulo: 5 V del USB o 3V3 | GND común con la placa. Parlante solo entre las salidas del módulo |
| LED | GPIO a través de resistencia | **Resistencia de 330 Ω en serie**; cátodo a GND |

## Compilar y cargar

Desde la terminal ESP-IDF (en Windows, el perfil «ESP-IDF 5.5.6» que crea el instalador):

```powershell
cd firmware\diagnostics\h01
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p COM12 flash monitor
```

En `menuconfig` → **H01 bench**: elegir la prueba en *Test to run* y escribir solo los pines del módulo que se va a probar. El puerto `COM12` es el que vio Winderson y puede cambiar; consultar `Get-CimInstance Win32_SerialPort`.

La velocidad del bus I2C se ajusta en *OLED (I2C)* → `I2C SCL speed (Hz)`. Por defecto es 100 kHz, que tolera jumpers largos y pull-ups débiles; subir a 400 kHz solo después de que la OLED funcione a 100 kHz, y anotar la velocidad en el registro.

Si el puerto se desconecta solo, entrar en modo programación (mantener BOOT, pulsar y soltar RESET, soltar BOOT). Esa incidencia está abierta en el [registro de T00](../pruebas/resultados/2026-11-25-T00-prueba-usb.md).

Antes de la primera carga en una placa con firmware original, seguir el respaldo de `docs/equipo/TRASPASO.md`.

## Pruebas

Las cinco imprimen al arrancar la identificación de la placa y todos los pines configurados. Guardar ese encabezado en el registro.

| Prueba (*Test to run*) | Pines a configurar | Qué observar | Resultado esperado |
|---|---|---|---|
| TTP223 touch and LED | `TTP223 I/O GPIO`, `LED GPIO` | Una línea `press` y una `release` por cada contacto, con su duración en ms. El LED sigue al contacto | Un solo par por toque; sin pulsos sin tocar durante 60 s. Anotar duraciones de toque breve, doble toque y toque sostenido: T04 las necesita para calibrar los gestos |
| I2C bus scan | `SDA GPIO`, `SCL GPIO` | `ACK at 0x..` por cada dispositivo | Una dirección por OLED conectada. Anotar la dirección real |
| OLED border and two eyes | SDA, SCL, `OLED address` y *Controller to try* | Borde de 1 px en los cuatro lados y dos ojos | Los cuatro bordes visibles, ninguno cortado ni doble. Si falta o sobra una franja de dos columnas, repetir con el otro controlador. Si la pantalla queda negra aunque el escaneo dé ACK, repetir con la opción SSD1306; el log indica qué secuencia de inicio se envió |
| INMP441 relative level | `SCK`, `WS`, `SD` | Cinco líneas por segundo con `rms`, `peak`, `dc` y `zeros` | En silencio, `rms` bajo y estable; con una palmada a distancia fija, `peak` sube. `zeros` igual al total indica cableado o alimentación incorrectos |
| MAX98357A test tone | `BCLK`, `LRC`, `DIN`, volumen | Tono de 400 Hz, 1 s sí y 1 s no | Tono limpio y sin reinicios. Anotar `reset_reason` si la placa se reinicia al sonar |

El micrófono informa niveles relativos a la escala digital (0 a 1), no dB SPL. Las muestras se usan solo para calcular el nivel de cada ventana: no se guardan ni se transmiten.

El amplificador va en I2S1 y el micrófono en I2S0, separados como indica T04. Empezar con el volumen por defecto (10 %). El parlante va entre las salidas del módulo; ninguna salida a GND.

## Registro

Un registro por módulo en `docs/pruebas/resultados/`, con la [plantilla](../pruebas/PLANTILLA_RESULTADO.md), nombrado `AAAA-MM-DD-H01-<modulo>.md`. Incluir:

- commit del firmware de banco y opciones de `menuconfig` usadas;
- pines usados y continuidad medida;
- foto del montaje y extracto del log;
- resultado observado y qué no demuestra.

Cuando los cinco módulos tengan registro, pasar los pines verificados a `docs/hardware/PINOUT.md` (lo crea T03) y actualizar `openspec/changes/02-secure-device/tasks.md`. Hasta entonces, el pinout de las fichas EVA_02 sigue «por registrar».

## Qué no demuestra

Funcionamiento simultáneo de los módulos, consumo (ficha E10 de EVA_02), calibración de gestos ni integración con Android.
