## ADDED Requirements

### Requirement: T01 Contrato y autoridad simulada de control
El sistema SHALL cumplir el siguiente criterio: Un solo propietario; expiración a 6000 ms; reintentos sin duplicar efectos; ejemplos válidos e inválidos comprobados.

#### Scenario: Verificación de T01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T01
- **THEN** Un solo propietario; expiración a 6000 ms; reintentos sin duplicar efectos; ejemplos válidos e inválidos comprobados.

#### Scenario: Evidencia insuficiente de T01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T02 Identidad, emparejamiento USB y credenciales
El sistema SHALL cumplir el siguiente criterio: Primera app autorizada con identidad verificada; QR vencido/reutilizado y cliente revocado rechazados; no se desactiva TLS.

#### Scenario: Verificación de T02
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T02
- **THEN** Primera app autorizada con identidad verificada; QR vencido/reutilizado y cliente revocado rechazados; no se desactiva TLS.

#### Scenario: Evidencia insuficiente de T02
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T03 Expresión confirmada de extremo a extremo por HTTPS
El sistema SHALL cumplir el siguiente criterio: Android y Windows solicitan expresión y ven el estado confirmado; repetir en placa real, sin presentar simulación como hardware.

#### Scenario: Verificación de T03
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T03
- **THEN** Android y Windows solicitan expresión y ven el estado confirmado; repetir en placa real, sin presentar simulación como hardware.

#### Scenario: Evidencia insuficiente de T03
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: H01 Recepción, pinout y pruebas individuales de componentes
El sistema SHALL cumplir el siguiente criterio: Registrar lote, memoria, OLED/controlador, pines, TTP223, INMP441, LED y amplificador; medir continuidad y alimentación antes del montaje completo.

#### Scenario: Verificación de H01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de H01
- **THEN** Registrar lote, memoria, OLED/controlador, pines, TTP223, INMP441, LED y amplificador; medir continuidad y alimentación antes del montaje completo.

#### Scenario: Evidencia insuficiente de H01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

