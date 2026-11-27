# T00 · Base de aplicaciones y reproducción del diagnóstico USB

- Tarea: [#1 T00](https://github.com/wmrDev/agentic-iot-pet/issues/1).
- Fecha y responsable: 26 de noviembre de 2026, entorno de Walther. Revisión cruzada pendiente.
- Revisiones: `93529a3` para Android, Windows y simulador. `558ae67` para el diagnóstico ESP32-S3 del [PR #27](https://github.com/wmrDev/agentic-iot-pet/pull/27).
- Entorno: Windows 25H2, build 26200.9974. [Versiones y preparación](../../equipo/ENTORNOS.md).
- Resultado esperado: APK generada, aplicación Windows abre su ventana, simulador inicia y el diagnóstico genera una imagen para `esp32s3`.

| Verificación | Comando o procedimiento | Resultado observado |
|---|---|---|
| Compilación de apps | `gradlew.bat :androidApp:assembleDebug :desktopApp:compileKotlin :simulator:test :shared:desktopTest :simulator:run --console=plain` | Gradle 8.13 terminó con código 0. Los dos destinos de pruebas indicaron `NO-SOURCE`. |
| Inspección de la APK | `aapt dump badging apps/androidApp/build/outputs/apk/debug/androidApp-debug.apk` | Paquete `io.github.wmrdev.petiot`, actividad de inicio `pet.MainActivity`, SDK mínimo 24 y destino 36. La APK local ocupa 10 296 304 bytes. |
| Instalación y arranque en Android físico | Walther instaló `Pet-IoT-T00-93529a3.apk` desde Descargas y abrió la app en el INFINIX ZERO 30 5G con Android 14, según indicó desde Ajustes. | **PASÓ EN HARDWARE.** Se observan «Pet IoT» y «Device connection is not available yet.». [Captura aportada por Walther](2026-11-26-T00-infinix.png). |
| Arranque de escritorio | `gradlew.bat :desktopApp:run` | Se observó la ventana «Pet IoT». Tras cerrar la ventana, Gradle terminó con código 0. |
| Arranque del simulador | `gradlew.bat :simulator:run` | Mostró el aviso de arranque e indisponibilidad del protocolo. Terminó con código 0. |
| Compilación de firmware | `idf.py build` en el proyecto del PR #27, con directorio de salida y `SDKCONFIG` aislados | ESP-IDF 5.5.6, Xtensa GCC 14.2.0, Python 3.11.2, CMake 3.30.2 y Ninja 1.12.1. Se generó `prueba_usb.bin` de `0x2bef0` bytes con 83 % libre en la partición de aplicación. |
| Estructura de OpenSpec | `npx --yes @fission-ai/openspec@2.1.0 validate 01-foundation --strict --no-interactive` | Cambio válido. |

La compilación Android instaló la plataforma 36 y Build Tools 35.0.0 con la licencia del SDK ya registrada. El aviso de empaquetado de una biblioteca nativa sin quitar símbolos no interrumpió la generación de la APK.

**Preparación de la prueba física Android:** Windows reconoció el INFINIX ZERO 30 5G por USB. ADB 37.0.1 no lo listó después de la autorización en el teléfono. Se verificó el SHA-256 de la APK de `93529a3` en el PC y se transfirió por MTP a Descargas como `Pet-IoT-T00-93529a3.apk`. Windows confirmó el nombre y tamaño de 10 296 304 bytes en el teléfono.

**Pendientes:** cargar y observar el diagnóstico en la placa, revisión cruzada y reproducción desde otro clon. Queda por integrar el job de firmware con el PR #27. El job `gradle` del workflow de GitHub Actions ya se observó en verde en el PR #31 (`windows-2025`).
