# Tasks

## 1. T00 Compilación mínima Android, Windows y ESP32
- [x] 1.1 Revisar contrato, dependencias y alcance del issue [#1](https://github.com/wmrDev/agentic-iot-pet/issues/1).
- [x] 1.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [x] 1.3 Verificar: Las apps abren en Android/Windows y firmware compila para S3; versiones y comandos quedan registrados.
- [x] 1.4 Adjuntar evidencia y revisión cruzada antes de cerrar.
  - 2026-11-27: PR #27 y #31 unidos a `main` con revisión de código de agente (D10). Build reproducido desde un clon limpio por Winderson (#32). Job `firmware` en CI (PR #39) con `espressif/idf:v5.5.6`: producto, tests de Unity y banco H01 compilan para `esp32s3` ([ejecución](https://github.com/wmrDev/agentic-iot-pet/actions/runs/46013730537)). La carga en la placa se registra en H01 (#7).
  - Avance 2026-11-25 (parcial): firmware PRUEBA_USB implementado y compilado para S3 (`424032f`; en Windows, `b951b8f`). Pendiente: prueba en placa. [Evidencia](../../../docs/pruebas/resultados/2026-11-25-T00-prueba-usb.md).
  - Avance parcial en `codex/t00-apps`: APK instalada y abierta en el INFINIX ZERO 30 5G, ventana Windows observada y simulador arrancado. [Registro T00](../../../docs/pruebas/resultados/2026-11-26-T00-base-aplicaciones.md).

## 2. T00-V Viabilidad de micrófonos y Android en segundo plano
- [ ] 2.1 Revisar contrato, dependencias y alcance del issue [#2](https://github.com/wmrDev/agentic-iot-pet/issues/2).
- [ ] 2.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 2.3 Verificar: Registrar permisos, llamada previa/durante, bloqueo Android y latencias reales; limitaciones se comunican antes de T08.
- [ ] 2.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

## 3. V01 Lenguaje visual, expresiones y flujos de la interfaz
- [ ] 3.1 Revisar contrato, dependencias y alcance del issue [#3](https://github.com/wmrDev/agentic-iot-pet/issues/3).
- [ ] 3.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 3.3 Verificar: Diseñar cara 128×64 y pantallas de conexión, estudio, chat y estado; distinguir desconexión, captura y confirmación sin depender solo del color.
- [ ] 3.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

