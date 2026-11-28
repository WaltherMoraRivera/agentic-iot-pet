# E01 · Instalación y apertura en Galaxy S24 Ultra

- ID de prueba: E01, [EVA02-02 #29](https://github.com/wmrDev/agentic-iot-pet/issues/29).
- Responsable de la prueba: Winderson. Instalación que consta en el Galaxy: 26-11-2026 a las 21:11:53 (hora del dispositivo). Captura recibida y datos verificados por ADB el 27-11-2026.
- Revisor: pendiente.
- APK instalada: `io.github.wmrdev.petiot` 0.1.0 (código 1), 10 296 304 bytes. SHA-256 de `base.apk`: `1f4ca77c70ac3e6c64dce002fd03527c5f4c5ad1e01b3351fa9883dbea7ab781`.
- Commit de la APK: sin verificar. El tamaño coincide con la APK reportada en #32 para `840529fe145b01e4d5aa21897f53e1349d868f84`. Falta el hash de ese archivo original para comprobar que sea el mismo.
- Entorno: hardware físico, Samsung Galaxy S24 Ultra (SM-S928B), Android 16, API 36, ABI `arm64-v8a`.
- Condiciones: instalación y apertura reportadas por Winderson. La disponibilidad de la placa y las condiciones de red no se verificaron en este registro.
- Procedimiento observado: instalar la APK de depuración, abrir Pet IoT y capturar la pantalla inicial. Para verificar la evidencia se consultaron las propiedades del equipo con `adb shell getprop`, se localizó `base.apk` con `adb shell pm path io.github.wmrdev.petiot` y se calculó su SHA-256 con `sha256sum` en el dispositivo.
- Resultado esperado: la APK se instala y Pet IoT abre su pantalla de bienvenida.
- Resultado observado: la instalación fue exitosa según Winderson. La captura muestra «Pet IoT» y «Device connection is not available yet.».
- Estado: **PASÓ EN HARDWARE** para instalación y apertura. Queda pendiente confirmar el commit de la APK.
- Evidencia: [captura del Galaxy](E01-galaxy-pet-iot.png).
- Qué no demuestra esta prueba: conexión con la ESP32-S3, sincronización, ni correspondencia de la APK instalada con un commit específico. La compilación desde clon limpio y la apertura en Windows están registradas por separado en [T00-R #32](https://github.com/wmrDev/agentic-iot-pet/issues/32).
