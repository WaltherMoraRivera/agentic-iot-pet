# T04 · Pruebas en la placa

Procedimiento del paso 6 de T04 ([#8](https://github.com/wmrDev/agentic-iot-pet/issues/8)). **Estado: NO EJECUTADO.** Requiere la placa y los pines verificados en el [banco H01](../hardware/H01_BANCO.md).

Cada ejecución genera un registro con la [plantilla](PLANTILLA_RESULTADO.md), nombrado `AAAA-MM-DD-T04-<prueba>.md`.

## 1. Tests Unity en la placa

Desde la terminal ESP-IDF:

```powershell
cd firmware
idf.py -C test set-target esp32s3
idf.py -C test -p COM12 flash monitor
```

- **Esperado:** el resumen final de Unity con `0 Failures` sobre 49 tests. Copiar al registro las líneas finales del monitor.
- **Ojo:** `test_settings.c` borra la NVS de la placa. Después de los tests, volver a cargar el firmware de producto (paso 2).
- `test_wall_clock_does_not_change_session` cambia la hora civil de la placa una hora y la restaura al terminar.

Si algún test falla, registrar cuál, el mensaje de Unity y la revisión. No corregir el test para que pase sin revisar antes la lógica.

## 2. Configurar y cargar el firmware de producto

```powershell
cd firmware
idf.py menuconfig
idf.py -p COM12 flash monitor
```

En `menuconfig` → **Pet hardware**, escribir solo los pines verificados en H01. Dejar **Sound capture calibrated** desactivado hasta hacer el paso 4.

Al arrancar, el monitor muestra `PET_SENSORS: touch=… led=… mic=…`, la configuración cargada y, si hubo un reinicio con sesión activa, `Previous session was interrupted by a reboot`.

## 3. Gestos y sesión (P02, P12 parcial, P15 parcial)

| Caso | Procedimiento | Esperado en el monitor |
|---|---|---|
| Toque breve | Tocar y soltar rápido | `study … phase=1` (en curso) y un sonido corto |
| Doble toque | Dos toques seguidos | `phase=2` (pausa); repetir: `phase=1` |
| Mantener | Mantener más de 0,8 s y soltar | `voice requested`, LED encendido; al soltar `voice released`, LED apagado. La sesión no cambia |
| Toque intermedio | Mantener unos 0,5 s | Nada |
| Fin de sesión | Esperar la duración por defecto (25 min), o bajarla temporalmente a 60 000 ms en `settings_defaults()` mientras no exista `config.set` | `phase=3` (terminada) y el sonido de fin |
| Reinicio con sesión | Iniciar una sesión y pulsar RESET | Al arrancar, `interrupted` y `phase=4`; sin duración inventada |
| Sin red | Todo lo anterior sin Wi-Fi configurado | Funciona igual: la sesión no depende de la red |

Anotar las duraciones reales de cada gesto (el banco H01 las mide) y comparar con las ventanas provisionales 300/350/800 ms de `gestures.h`. Si no coinciden, ajustar `GESTURE_WINDOWS_PROVISIONAL`, registrar los valores y repetir los tests.

## 4. Calibrar la captura de sonido

1. Con **Sound capture calibrated** desactivado, la placa guarda el nivel pero no reacciona.
2. Dejar la placa 60 s en silencio y luego producir el ruido de referencia (palmada a distancia fija, anotada). Leer los niveles en el banco H01 (prueba del INMP441), que imprime cinco valores por segundo.
3. Elegir un umbral entre el ruido de fondo y la palmada, con margen mayor que la histéresis (0,05). Anotarlo en el registro.
4. Mientras no exista `config.set` (T03), cambiar el valor por defecto en `settings_defaults()` o cargarlo por NVS, y activar **Sound capture calibrated**.

## 5. Sonido sin bucle (A01)

| Caso | Procedimiento | Esperado |
|---|---|---|
| Reacción | Palmada por encima del umbral | Un evento `sound_reaction` y un solo sonido de aviso |
| Ruido persistente | Ruido continuo por encima del umbral durante 10 s | Un solo aviso; no se repite mientras el ruido no baje |
| Silencio | `muted = true` en la configuración | Sin sonido; la sesión y los gestos siguen |
| Estabilidad | 20 avisos seguidos | Sin reinicios (`reset_reason` igual tras la prueba) |

Medir el tiempo entre el fin del sonido y la siguiente reacción posible. Si 200 ms no basta para evitar que la mascota reaccione a su propio sonido, ajustar `SOUND_DEFAULT_POST_PLAYBACK_MS` y registrar el valor medido.

## Qué no demuestran estas pruebas

Órdenes desde Android o Windows (P03 completo, T03), confirmación de una operación real de la app (P12 completo), recuperación de telemetría por la red (P16, T05) ni consumo (E10 de EVA_02).
