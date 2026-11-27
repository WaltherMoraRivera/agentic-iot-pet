# Cómo retomar (cierre del 2026-11-25)

Punto de partida: rama `feat/T00-firmware`; PRUEBA_USB compiló en Linux sobre el árbol previo a `424032f`; los ajustes de `468fca1` no se han compilado y **nada se ha cargado** en la placa. Detalle en [el registro de T00](../pruebas/resultados/2026-11-25-T00-prueba-usb.md) y [las mediciones de H01](../pruebas/resultados/2026-11-25-H01-preliminar.md).

Los pasos 5 (restauración), 6, 10 a 12 y 13 escriben en la placa, la cablean o amplían alcance: requieren autorización expresa.

## 1. Activar ESP-IDF

Linux:

```bash
source ~/esp/esp-idf-v5.5.6/export.sh
cd <repositorio>
git switch feat/T00-firmware && git pull
idf.py --version   # ESP-IDF v5.5.6
```

Windows: abrir la terminal «ESP-IDF PowerShell» de la instalación v5.5.6 (preparación aún no verificada) y ejecutar los mismos comandos de Git dentro del repositorio.

## 2. Conectar la placa aislada

Sin jumpers a TTP223, LED ni otros módulos. Cable USB de datos.

## 3. Permisos y puerto

Linux:

```bash
id -nG                      # debe incluir uucp; si no: newgrp uucp, o cerrar sesión y entrar
ls -l /dev/ttyACM*
```

Windows (los puertos pueden cambiar):

```powershell
Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Description
```

En los comandos siguientes, sustituir `/dev/ttyACM0` o `COM12` por el puerto detectado.

## 4. Modo programación (si el puerto se desconecta solo)

Mantener BOOT, pulsar y soltar RESET, soltar BOOT.

## 5. Respaldar el firmware original (antes de escribir)

Guardar fuera del repositorio; no subirlo a Git.

Leer la memoria dos veces y comparar los SHA-256: una sola lectura no prueba que el respaldo sea fiel.

```bash
# Linux
mkdir -p ~/Respaldos
esptool.py --chip esp32s3 -p /dev/ttyACM0 read_flash 0 0x400000 ~/Respaldos/esp32s3-original.bin
esptool.py --chip esp32s3 -p /dev/ttyACM0 read_flash 0 0x400000 ~/Respaldos/esp32s3-original-2.bin
ls -l ~/Respaldos/esp32s3-original*.bin   # 4194304 bytes cada uno
sha256sum ~/Respaldos/esp32s3-original.bin ~/Respaldos/esp32s3-original-2.bin
```

```powershell
# Windows
New-Item -ItemType Directory -Force "$HOME\Respaldos"
python -m esptool --chip esp32s3 -p COM12 read_flash 0 0x400000 "$HOME\Respaldos\esp32s3-original.bin"
python -m esptool --chip esp32s3 -p COM12 read_flash 0 0x400000 "$HOME\Respaldos\esp32s3-original-2.bin"
certutil -hashfile "$HOME\Respaldos\esp32s3-original.bin" SHA256
certutil -hashfile "$HOME\Respaldos\esp32s3-original-2.bin" SHA256
```

Si una lectura falla, el tamaño no es 4194304 bytes o los dos SHA-256 difieren, no continuar. Anotar el SHA-256 en el registro de la prueba (no el archivo).

Restaurar el original (solo si hace falta y con autorización; sobrescribe toda la flash):

```bash
esptool.py --chip esp32s3 -p <puerto> write_flash 0x0 <archivo>
# Windows: python -m esptool --chip esp32s3 -p <puerto> write_flash 0x0 <archivo>
```

## 6. Compilar y flashear PRUEBA_USB

```bash
cd firmware
idf.py build
idf.py -p /dev/ttyACM0 flash      # Windows: idf.py -p COM12 flash
```

No usar `erase-flash`. Flashear sobrescribe bootloader, tabla de particiones y aplicación.

## 7. Monitor serie

Si se entró en modo programación a mano, pulsar RESET sin mantener BOOT. Después:

```bash
idf.py -p /dev/ttyACM0 monitor --no-reset    # Windows: idf.py -p COM12 monitor --no-reset
```

Usar `--no-reset`: sin esa opción, abrir el monitor reinicia el chip, y el `seq=0` y el `reset_reason` que se ven serían provocados por el propio monitor, no por la placa. Si se abre sin ella, anotarlo y no contar ese reinicio como falla. Salir con `Ctrl+]`.

Los registros de la aplicación salen por USB Serial/JTAG como consola secundaria (la primaria es UART0, valor por defecto del ESP32-S3). Ante un pánico, el firmware imprime los registros y se detiene (`CONFIG_ESP_SYSTEM_PANIC_PRINT_HALT`): copiar el texto y pulsar RESET para continuar.

## 8. Validar

Durante varios minutos (anotar cuántos):

- `seq` sube de uno en uno cada 2 s y `uptime` crece.
- Si `seq` vuelve a 0, hubo reinicio: anotar `reset_reason`, `reset_name` y `boot_count` (`boot_count` sube en cada reinicio sin corte de energía y vuelve a 1 tras un encendido). Valores de `esp_reset_reason_t` en ESP-IDF v5.5:

  | Número | `reset_name` | Significado |
  |---|---|---|
  | 0 | UNKNOWN | No se pudo determinar |
  | 1 | POWERON | Encendido |
  | 2 | EXT | Pin externo (RESET) |
  | 3 | SW | Software (`esp_restart`) |
  | 4 | PANIC | Excepción o pánico |
  | 5 | INT_WDT | Watchdog de interrupciones |
  | 6 | TASK_WDT | Watchdog de tareas |
  | 7 | WDT | Otros watchdogs |
  | 8 | DEEPSLEEP | Salida de sueño profundo |
  | 9 | BROWNOUT | Caída de tensión |
  | 10 | SDIO | SDIO |
  | 11 | USB | Periférico USB |
  | 12 | JTAG | JTAG |
  | 13 | EFUSE | Error de eFuse |
  | 14 | PWR_GLITCH | Perturbación de alimentación |
  | 15 | CPU_LOCKUP | Bloqueo de CPU (doble excepción) |

- Comprobar si el puerto sigue desapareciendo (incidencia abierta).

## 9. Registrar

Nuevo archivo en `docs/pruebas/resultados/` con [la plantilla](../pruebas/PLANTILLA_RESULTADO.md): commit, entorno, resultado observado y revisor.

## 10 a 12. Cableado (sin alimentación)

Requiere autorización expresa y coordinación con Maximiliano (H01). TOUCH en GPIO4 y LED en GPIO5 son una **propuesta pendiente de aprobación en H01, no pines confirmados**: si H01 aprueba otros, usar esos.

10. Confirmar en la serigrafía qué pines físicos son GPIO4 y GPIO5.
11. Con la placa sin energía, comprobar con el multímetro continuidad de cada conexión, ausencia de corto entre 3V3 y GND, GND común, resistencia de 330 Ω en serie con el LED y su polaridad.
12. Reconectar: TTP223 VCC → 3V3, GND → GND, I/O → GPIO4; GPIO5 → 330 Ω → ánodo; cátodo → GND.

## 13. PRUEBA_TOQUE

Previa coordinación con Maximiliano (H01) y Walther, y autorización. Esperado: LED apagado en reposo, encendido al tocar, apagado al soltar.

## 14. Después

OLED (verificar controlador), INMP441, MAX98357A y parlante, uno a uno antes de integrar.
