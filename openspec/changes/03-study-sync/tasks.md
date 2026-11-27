# Tasks

## 1. T04 Mascota autónoma, temporizador, gestos y sonidos
- [ ] 1.1 Revisar contrato, dependencias y alcance del issue [#8](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/8).
- [ ] 1.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
  - Parcial: firmware local completo en `firmware/main/` (sesión, gestos, confirmación, modo de aviso, reacción acústica, telemetría, configuración en NVS, TTP223, INMP441, LED, MAX98357A con sonidos propios y ciclo principal) y 61 tests Unity en `firmware/test/`. Compila tras las correcciones de revisión del PR #36 (ver 1.4); no se ha ejecutado en la placa. Contrato propuesto en `contracts/ESTUDIO_Y_EVENTOS.md`. Faltan la API (T03), la parte Kotlin y las pruebas de `docs/pruebas/T04_PLACA.md`.
- [ ] 1.3 Verificar: Sesión continúa sin red; audio no genera bucle acústico; gestos y confirmaciones producen un solo efecto; montaje y consumo documentados.
- [ ] 1.4 Adjuntar evidencia y revisión cruzada antes de cerrar.
  - 2026-11-27: PR #36 unido con revisión de código de agente (D10). Producto y tests compilan en Windows (`b503c73` y `8f0c83a`); los 61 tests y `T04_PLACA.md` quedan pendientes de ejecutar en la placa.

## 2. T05 Historial, acceso local y sincronización incremental
- [ ] 2.1 Revisar contrato, dependencias y alcance del issue [#9](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/9).
- [ ] 2.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 2.3 Verificar: Room persiste en ambas plataformas; páginas repetidas no duplican; borrados no reaparecen; al menos 300 muestras tras corte de cinco minutos.
- [ ] 2.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

## 3. V02 Implementar interfaz compartida y estados de error
- [ ] 3.1 Revisar contrato, dependencias y alcance del issue [#10](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/10).
- [ ] 3.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 3.3 Verificar: Implementar componentes Compose sobre contratos reales de T04/T05/T07, entregándolos por función; mostrar loading, error, sin IA y confirmación sin simular éxito.
- [ ] 3.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

## 4. T06 Descubrimiento y relevo Android/Windows
- [ ] 4.1 Revisar contrato, dependencias y alcance del issue [#11](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues/11).
- [ ] 4.2 Implementar o documentar el resultado solicitado y sus pruebas pertinentes.
- [ ] 4.3 Verificar: Windows toma control autorizado y Android lo recupera tras suspensión/corte; nunca dos centros; salida abrupta marca contabilidad incierta.
- [ ] 4.4 Adjuntar evidencia y revisión cruzada antes de cerrar.

