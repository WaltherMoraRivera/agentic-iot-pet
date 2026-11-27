# Dispositivos del corte EVA_02

Se actualiza al inicio de cada sesión de pruebas. Un dato sin comprobar se deja como «por registrar»; no se completa con lo esperado. No anotar IMEI, número de serie, número de teléfono ni cuentas.

## Revisión bajo prueba

| Elemento | Valor | Cómo obtenerlo |
|---|---|---|
| Commit de la APK | por registrar | `git rev-parse --short HEAD` en el clon que generó la APK |
| SHA-256 de la APK | por registrar | `certutil -hashfile <apk> SHA256` (Windows) |
| Commit del firmware | por registrar | `git rev-parse --short HEAD` en el clon que compiló el firmware |
| Hash de la imagen de firmware | por registrar | `certutil -hashfile build\<imagen>.bin SHA256` |
| Versión de ESP-IDF | por registrar | `idf.py --version` |

## Android

| Dato | INFINIX ZERO 30 5G | Galaxy S24 Ultra |
|---|---|---|
| Aporta | Walther | Winderson |
| Versión de Android | 14, según Ajustes, preliminar: proviene del registro de T00 en el [PR #31](https://github.com/wmrDev/agentic-iot-pet/pull/31) (rama `codex/t00-apps`, sin fusionar); confirmar | por registrar |
| Nivel de API | por registrar | por registrar |
| Arquitectura (ABI) | por registrar | por registrar |
| Capa del fabricante y versión | por registrar | por registrar |
| Parche de seguridad | por registrar | por registrar |
| Modo de ahorro de batería usado | por registrar | por registrar |
| Método de instalación | por registrar | por registrar |
| Permisos concedidos a la app | por registrar | por registrar |

### Cómo obtener los datos

Con ADB autorizado (en el registro de T00, ADB no listó el INFINIX; si persiste, usar Ajustes):

```powershell
adb devices
adb shell getprop ro.build.version.release
adb shell getprop ro.build.version.sdk
adb shell getprop ro.product.cpu.abi
adb shell getprop ro.build.version.security_patch
adb shell dumpsys package io.github.wmrdev.petiot | Select-String "permission"
```

Sin ADB: Ajustes → Acerca del teléfono → Información de software. La arquitectura no siempre aparece ahí; en ese caso dejarla «por registrar» y anotar el motivo.

## Windows

| Dato | Valor |
|---|---|
| Equipo y responsable | por registrar |
| Edición y compilación | por registrar (`winver`) |
| Uso en EVA_02 | Monitor serie, cliente de pruebas de E08 y compilación; Windows como app no forma parte del recorrido #29 |

## Placa y periféricos

| Elemento | Valor | Fuente |
|---|---|---|
| Placa | ESP32-S3 SuperMini; chip revisión v0.2, 4 MB flash, 2 MB PSRAM (preliminar) | Registro `docs/pruebas/resultados/2026-11-25-T00-prueba-usb.md` (diagnóstico USB del commit `424032f`), en las ramas `feat/T00-firmware` y `feat/H01-banco`, sin fusionar; pendiente de revisión |
| OLED y controlador | por registrar | H01 (#7) |
| TTP223 | Activo en alto, momentáneo (preliminar) | Registro `docs/pruebas/resultados/2026-11-25-H01-preliminar.md`, en las ramas `feat/H01-banco` y `feat/T00-firmware`, sin fusionar; pin de salida sin verificar |
| INMP441 | por registrar | H01 |
| MAX98357A y parlante | por registrar; compra del amplificador pendiente de confirmar según inventario | H01, [INVENTARIO](../../hardware/INVENTARIO.md) |
| Fuente de alimentación | por registrar; la fuente USB-C 5 V / 3 A está por adquirir (carrito Afel preparado) | [INVENTARIO](../../hardware/INVENTARIO.md) |
| Pinout verificado | por registrar | H01; no se usa el pinout previsto como verificado |

## Instrumentos

| Instrumento | Marca y modelo | Uso |
|---|---|---|
| Multímetro | por registrar | Tensión y, si aplica, corriente en serie (E10) |
| Medidor USB en línea | por registrar si se consigue; no figura en el inventario | Corriente de la placa (E10) |
| iPhone 14 | — | Foto y vídeo del ensayo; no es dispositivo bajo prueba |
