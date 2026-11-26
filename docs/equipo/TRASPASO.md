# Cómo retomar (cierre del 2026-11-25)

Punto de partida: rama `feat/T00-firmware`; PRUEBA_USB compila y **no se ha cargado** en la placa. Detalle en [el registro de T00](../pruebas/resultados/2026-11-25-T00-prueba-usb.md) y [las mediciones de H01](../pruebas/resultados/2026-11-25-H01-preliminar.md).

Los pasos 5, 6 y 13 escriben o amplían alcance: requieren autorización expresa.

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

```bash
# Linux
mkdir -p ~/Respaldos
esptool.py --chip esp32s3 -p /dev/ttyACM0 read_flash 0 0x400000 ~/Respaldos/esp32s3-original.bin
ls -l ~/Respaldos/esp32s3-original.bin   # 4194304 bytes
```

```powershell
# Windows
New-Item -ItemType Directory -Force "$HOME\Respaldos"
python -m esptool --chip esp32s3 -p COM12 read_flash 0 0x400000 "$HOME\Respaldos\esp32s3-original.bin"
```

Si falla o el tamaño no coincide, no continuar.

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
idf.py -p /dev/ttyACM0 monitor    # Windows: idf.py -p COM12 monitor
```

Salir con `Ctrl+]`.

## 8. Validar

Durante varios minutos (anotar cuántos):

- `seq` sube de uno en uno cada 2 s y `uptime` crece.
- Si `seq` vuelve a 0, hubo reinicio: anotar `reset_reason` (1 encendido, 3 software, 4 pánico, 6 watchdog de tarea, 9 caída de tensión, 11 USB).
- Comprobar si el puerto sigue desapareciendo (incidencia abierta).

## 9. Registrar

Nuevo archivo en `docs/pruebas/resultados/` con [la plantilla](../pruebas/PLANTILLA_RESULTADO.md): commit, entorno, resultado observado y revisor.

## 10 a 12. Cableado (sin alimentación)

10. Confirmar en la serigrafía qué pines físicos son GPIO4 y GPIO5.
11. Con la placa sin energía, comprobar con el multímetro continuidad de cada conexión, ausencia de corto entre 3V3 y GND, GND común, resistencia de 330 Ω en serie con el LED y su polaridad.
12. Reconectar: TTP223 VCC → 3V3, GND → GND, I/O → GPIO4; GPIO5 → 330 Ω → ánodo; cátodo → GND.

## 13. PRUEBA_TOQUE

Previa coordinación con Maximiliano (H01) y Walther, y autorización. Esperado: LED apagado en reposo, encendido al tocar, apagado al soltar.

## 14. Después

OLED (verificar controlador), INMP441, MAX98357A y parlante, uno a uno antes de integrar.
