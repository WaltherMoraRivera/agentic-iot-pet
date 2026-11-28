# Tasks

## 1. T01 Contrato y autoridad simulada de control
- [x] 1.1 Revisar contrato, dependencias y alcance del issue [#4](https://github.com/wmrDev/agentic-iot-pet/issues/4).
- [x] 1.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [x] 1.3 Verificar un solo propietario, expiración a 6000 ms, reintentos sin duplicar efectos y ejemplos válidos e inválidos.
- [x] 1.4 Adjuntar evidencia y revisión cruzada antes de cerrar.
  - [Casos de prueba](../../../docs/pruebas/T01-contrato.md) y [resultado local](../../../docs/pruebas/resultados/2026-11-26-T01-contrato.md). Revisión de código de agente en el PR #33 (D10); 21 casos con 0 fallos en `8267a0b` y CI verde. La semántica de duplicados de un rechazo queda en #37.

## 2. T02 Identidad, emparejamiento USB y credenciales
- [ ] 2.1 Revisar contrato, dependencias y alcance del issue [#5](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/5).
- [ ] 2.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
  - [Contrato de emparejamiento](../../../contracts/EMPAREJAMIENTO.md) aprobado el 27-11-2026 (D12): flujo USB y oferta, `POST /v1/pair`, revocación y errores; responde las 10 preguntas de los casos. Paso 1 de T02 hecho; sin implementación.
- [ ] 2.3 Verificar: Primera app autorizada con identidad verificada; QR vencido/reutilizado y cliente revocado rechazados; no se desactiva TLS.
- [ ] 2.4 Adjuntar evidencia y revisión cruzada antes de cerrar.
  - [Casos preparados sin placa](../../../docs/pruebas/T02-casos.md): 25 casos planificados (17 compartidos, 1 manual en Android, 3 de simulador y 4 de placa) y preguntas abiertas del contrato. Sin implementación ni resultados.

## 3. T03 Expresión confirmada de extremo a extremo por HTTPS
- [ ] 3.1 Revisar contrato, dependencias y alcance del issue [#6](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/6).
- [ ] 3.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 3.3 Verificar: Android y Windows solicitan expresión y ven el estado confirmado; repetir en placa real, sin presentar simulación como hardware.
- [ ] 3.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

## 4. H01 Recepción, pinout y pruebas individuales de componentes
- [ ] 4.1 Revisar contrato, dependencias y alcance del issue [#7](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/7).
- [ ] 4.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 4.3 Verificar: Registrar lote, memoria, OLED/controlador, pines, TTP223, INMP441, LED y amplificador; medir continuidad y alimentación antes del montaje completo.
- [ ] 4.4 Adjuntar evidencia y revisión cruzada antes de cerrar.
  - Avance 2026-11-25 (parcial): mediciones eléctricas preliminares de TTP223 y LED; placa identificada. Pendiente: confirmar pines, continuidad, prueba por GPIO, OLED, INMP441, amplificador y revisión. [Evidencia](../../../docs/pruebas/resultados/2026-11-25-H01-preliminar.md).

