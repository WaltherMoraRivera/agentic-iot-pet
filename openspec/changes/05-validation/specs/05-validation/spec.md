## ADDED Requirements

### Requirement: Q01 Pruebas físicas P01–P19 y A01–A03
El sistema SHALL cumplir el siguiente criterio: Completar matriz con dos Android y Windows, mediciones y fallos registrados; una prueba no ejecutada no cuenta como aprobada.

#### Scenario: Verificación de Q01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de Q01
- **THEN** Completar matriz con dos Android y Windows, mediciones y fallos registrados; una prueba no ejecutada no cuenta como aprobada.

#### Scenario: Evidencia insuficiente de Q01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: Q02 Revisión de seguridad y fallos de integración
El sistema SHALL cumplir el siguiente criterio: Probar credenciales ajenas, certificados incorrectos, mensajes antiguos, llamadas inciertas y pérdida de centro; sin secretos/audio persistentes.

#### Scenario: Verificación de Q02
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de Q02
- **THEN** Probar credenciales ajenas, certificados incorrectos, mensajes antiguos, llamadas inciertas y pérdida de centro; sin secretos/audio persistentes.

#### Scenario: Evidencia insuficiente de Q02
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: V03 Usabilidad, legibilidad y accesibilidad en equipos reales
El sistema SHALL cumplir el siguiente criterio: Verificar navegación, tamaños, contraste y estados de OLED en Android/Windows reales; sin indicadores de éxito antes de confirmación.

#### Scenario: Verificación de V03
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de V03
- **THEN** Verificar navegación, tamaños, contraste y estados de OLED en Android/Windows reales; sin indicadores de éxito antes de confirmación.

#### Scenario: Evidencia insuficiente de V03
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

