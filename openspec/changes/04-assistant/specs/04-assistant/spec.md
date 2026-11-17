## ADDED Requirements

### Requirement: T07 Chat, herramientas seguras y presupuesto común
El sistema SHALL cumplir el siguiente criterio: Validar herramientas y reservas antes de gastar; pausa de IA ante relevo incierto; controles locales continúan; dos perfiles prueban respaldo.

#### Scenario: Verificación de T07
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T07
- **THEN** Validar herramientas y reservas antes de gastar; pausa de IA ante relevo incierto; controles locales continúan; dos perfiles prueban respaldo.

#### Scenario: Evidencia insuficiente de T07
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T08 Voz bajo demanda y captura móvil delegada
El sistema SHALL cumplir el siguiente criterio: Indicador confirmado antes de capturar; límite 15 segundos; cancelar ante conflicto, relevo o corte; texto siempre disponible.

#### Scenario: Verificación de T08
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T08
- **THEN** Indicador confirmado antes de capturar; límite 15 segundos; cancelar ante conflicto, relevo o corte; texto siempre disponible.

#### Scenario: Evidencia insuficiente de T08
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T09 Avisos de Claude Code sin aprobar permisos
El sistema SHALL cumplir el siguiente criterio: Stop y PermissionRequest producen avisos temporales; receptor local autenticado; no transmitir prompts ni bloquear el agente.

#### Scenario: Verificación de T09
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T09
- **THEN** Stop y PermissionRequest producen avisos temporales; receptor local autenticado; no transmitir prompts ni bloquear el agente.

#### Scenario: Evidencia insuficiente de T09
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

