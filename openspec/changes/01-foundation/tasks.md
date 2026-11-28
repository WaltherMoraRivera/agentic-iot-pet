# Tasks

## 1. T00 Compilación mínima Android, Windows y ESP32
- [ ] 1.1 Revisar contrato, dependencias y alcance del issue [#1](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/1).
- [ ] 1.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 1.3 Verificar: Las apps abren en Android/Windows y firmware compila para S3; versiones y comandos quedan registrados.
- [ ] 1.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

## 2. T00-V Viabilidad de micrófonos y Android en segundo plano
- [ ] 2.1 Revisar contrato, dependencias y alcance del issue [#2](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/2).
- [ ] 2.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 2.3 Verificar: Registrar permisos, llamada previa/durante, bloqueo Android y latencias reales; limitaciones se comunican antes de T08.
- [ ] 2.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

## 3. V01 Lenguaje visual, expresiones y flujos de la interfaz
- [ ] 3.1 Revisar contrato, dependencias y alcance del issue [#3](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/3).
- [x] 3.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
  - Avance 2026-11-25 (parcial): primer boceto de la cara (estilo B) en `docs/diseno/`.
  - Avance 2026-11-27: mockup navegable `docs/diseno/mockup.html` (conexión, estudio, chat, estado y flujo EVA02 con datos simulados), ceño de «Concentrada» suavizado y prueba `node docs/diseno/probar-mockup.mjs`.
  - Revisión de Walther incorporada: gestos T04, pausa y sesión interrumpida, espera visible de 15 s y captura exclusiva de voz bajo demanda.
  - Escenario: WHEN se envía una orden durante el estudio THEN se ve la espera, la confirmación explícita y el regreso al estado vigente.
  - Escenario: WHEN se corta el enlace durante la espera THEN se ve «Sin conexión» con texto e icono y la orden queda no confirmada, aunque llegue una respuesta tardía.
  - Escenario: WHEN coinciden captura y aviso THEN tiene prioridad el micrófono.
  - Escenario: WHEN una lectura simulada supera el umbral THEN hay un aviso con texto e icono.
- [ ] 3.3 Verificar: Diseñar cara 128×64 y pantallas de conexión, estudio, chat y estado; distinguir desconexión, captura y confirmación sin depender solo del color.
  - 2026-11-27: pasó en simulador ([registro](../../../docs/pruebas/resultados/2026-11-27-V01-mockup.md)). Pendiente: prueba en OLED físico.
- [ ] 3.4 Adjuntar evidencia y revisión cruzada antes de cerrar.
  - 2026-11-27: evidencia simulada adjunta. Pendiente: revisión de Walther y evidencia física.

