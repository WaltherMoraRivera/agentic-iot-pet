# E01 · Instalación y apertura en Galaxy S24 Ultra

- ID de prueba: E01, [EVA02-02 #29](https://github.com/wmrDev/agentic-iot-pet/issues/29).
- Responsable de la prueba: Winderson. Fecha y hora exactas de instalación: sin registrar. Evidencia recibida el 27-11-2026.
- Revisor: pendiente.
- Commit y SHA-256 de la APK instalada: pendientes de identificar.
- Entorno: hardware físico, Samsung Galaxy S24 Ultra. Versión de Android, nivel de API y ABI del equipo: pendientes de registrar.
- Condiciones: instalación y apertura reportadas por Winderson. La disponibilidad de la placa y las condiciones de red no se verificaron en este registro.
- Procedimiento observado: instalar la APK de depuración, abrir Pet IoT y capturar la pantalla inicial.
- Resultado esperado: la APK se instala y Pet IoT abre su pantalla de bienvenida.
- Resultado observado: la instalación fue exitosa según Winderson. La captura muestra «Pet IoT» y «Device connection is not available yet.».
- Estado: **PASÓ EN HARDWARE** para instalación y apertura. Trazabilidad de la APK pendiente.
- Evidencia: [captura del Galaxy](E01-galaxy-pet-iot.png).
- Qué no demuestra esta prueba: conexión con la ESP32-S3, sincronización, ni correspondencia de la APK instalada con un commit específico. La compilación desde clon limpio y la apertura en Windows están registradas por separado en [T00-R #32](https://github.com/wmrDev/agentic-iot-pet/issues/32).
