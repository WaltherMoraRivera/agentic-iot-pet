## ADDED Requirements

### Requirement: T04 Mascota autónoma, temporizador, gestos y sonidos
El sistema SHALL cumplir el siguiente criterio: Sesión continúa sin red; audio no genera bucle acústico; gestos y confirmaciones producen un solo efecto; montaje y consumo documentados.

#### Scenario: Verificación de T04
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T04
- **THEN** Sesión continúa sin red; audio no genera bucle acústico; gestos y confirmaciones producen un solo efecto; montaje y consumo documentados.

#### Scenario: Evidencia insuficiente de T04
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T05 Historial, acceso local y sincronización incremental
El sistema SHALL cumplir el siguiente criterio: Room persiste en ambas plataformas; páginas repetidas no duplican; borrados no reaparecen; al menos 300 muestras tras corte de cinco minutos.

#### Scenario: Verificación de T05
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T05
- **THEN** Room persiste en ambas plataformas; páginas repetidas no duplican; borrados no reaparecen; al menos 300 muestras tras corte de cinco minutos.

#### Scenario: Evidencia insuficiente de T05
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: V02 Implementar interfaz compartida y estados de error
El sistema SHALL cumplir el siguiente criterio: Implementar componentes Compose sobre contratos reales de T04/T05/T07, entregándolos por función; mostrar loading, error, sin IA y confirmación sin simular éxito.

#### Scenario: Verificación de V02
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de V02
- **THEN** Implementar componentes Compose sobre contratos reales de T04/T05/T07, entregándolos por función; mostrar loading, error, sin IA y confirmación sin simular éxito.

#### Scenario: Evidencia insuficiente de V02
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T06 Descubrimiento y relevo Android/Windows
El sistema SHALL cumplir el siguiente criterio: Windows toma control autorizado y Android lo recupera tras suspensión/corte; nunca dos centros; salida abrupta marca contabilidad incierta.

#### Scenario: Verificación de T06
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T06
- **THEN** Windows toma control autorizado y Android lo recupera tras suspensión/corte; nunca dos centros; salida abrupta marca contabilidad incierta.

#### Scenario: Evidencia insuficiente de T06
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

