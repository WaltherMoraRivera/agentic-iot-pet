## ADDED Requirements

### Requirement: T00 Compilación mínima Android, Windows y ESP32
El sistema SHALL cumplir el siguiente criterio: Las apps abren en Android/Windows y firmware compila para S3; versiones y comandos quedan registrados.

#### Scenario: Verificación de T00
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T00
- **THEN** Las apps abren en Android/Windows y firmware compila para S3; versiones y comandos quedan registrados.

#### Scenario: Evidencia insuficiente de T00
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: T00-V Viabilidad de micrófonos y Android en segundo plano
El sistema SHALL cumplir el siguiente criterio: Registrar permisos, llamada previa/durante, bloqueo Android y latencias reales; limitaciones se comunican antes de T08.

#### Scenario: Verificación de T00-V
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de T00-V
- **THEN** Registrar permisos, llamada previa/durante, bloqueo Android y latencias reales; limitaciones se comunican antes de T08.

#### Scenario: Evidencia insuficiente de T00-V
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: V01 Lenguaje visual, expresiones y flujos de la interfaz
El sistema SHALL cumplir el siguiente criterio: Diseñar cara 128×64 y pantallas de conexión, estudio, chat y estado; distinguir desconexión, captura y confirmación sin depender solo del color.

#### Scenario: Verificación de V01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de V01
- **THEN** Diseñar cara 128×64 y pantallas de conexión, estudio, chat y estado; distinguir desconexión, captura y confirmación sin depender solo del color.

#### Scenario: Evidencia insuficiente de V01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

