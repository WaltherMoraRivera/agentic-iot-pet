## ADDED Requirements

### Requirement: S01 Pulir animaciones y transiciones del personaje
El sistema SHALL cumplir el siguiente criterio: Mejorar transiciones sin bloquear sensores ni reducir prioridad del indicador de captura; conservar fluidez medida.

#### Scenario: Verificación de S01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de S01
- **THEN** Mejorar transiciones sin bloquear sensores ni reducir prioridad del indicador de captura; conservar fluidez medida.

#### Scenario: Evidencia insuficiente de S01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: C01 Respuesta hablada mediante sintetizador del sistema
El sistema SHALL cumplir el siguiente criterio: Reproducir voz opcional en el dispositivo activo con silencio/cancelación; no prometer streaming al parlante sin nueva especificación.

#### Scenario: Verificación de C01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de C01
- **THEN** Reproducir voz opcional en el dispositivo activo con silencio/cancelación; no prometer streaming al parlante sin nueva especificación.

#### Scenario: Evidencia insuficiente de C01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: C02 Pausa por ausencia con cámara opcional
El sistema SHALL cumplir el siguiente criterio: Cámara desactivada por defecto, indicador visible y sin guardar imágenes; validar falsas pausas y uso de CPU.

#### Scenario: Verificación de C02
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de C02
- **THEN** Cámara desactivada por defecto, indicador visible y sin guardar imágenes; validar falsas pausas y uso de CPU.

#### Scenario: Evidencia insuficiente de C02
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: C03 Imagen bajo demanda y visión por API
El sistema SHALL cumplir el siguiente criterio: Enviar imagen solo con solicitud y consentimiento explícitos, con reserva de coste; no almacenar capturas privadas.

#### Scenario: Verificación de C03
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de C03
- **THEN** Enviar imagen solo con solicitud y consentimiento explícitos, con reserva de coste; no almacenar capturas privadas.

#### Scenario: Evidencia insuficiente de C03
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: C04 Acciones de escritorio en lista autorizada
El sistema SHALL cumplir el siguiente criterio: Solo acciones registradas, parámetros validados y toque físico vigente; sin shell ni acceso general a archivos.

#### Scenario: Verificación de C04
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de C04
- **THEN** Solo acciones registradas, parámetros validados y toque físico vigente; sin shell ni acceso general a archivos.

#### Scenario: Evidencia insuficiente de C04
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: C05 MCP para expresiones explícitas de agentes
El sistema SHALL cumplir el siguiente criterio: Herramientas limitadas a expresiones y avisos autenticados, con caducidad; no sustituir hooks ni aprobar permisos.

#### Scenario: Verificación de C05
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de C05
- **THEN** Herramientas limitadas a expresiones y avisos autenticados, con caducidad; no sustituir hooks ni aprobar permisos.

#### Scenario: Evidencia insuficiente de C05
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

