## ADDED Requirements

### Requirement: R01 Clon limpio, artefactos y guías de instalación
El sistema SHALL cumplir el siguiente criterio: Reproducir builds e instalación de APK, Windows y firmware con versiones/commit; registrar A04 y límites.

#### Scenario: Verificación de R01
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de R01
- **THEN** Reproducir builds e instalación de APK, Windows y firmware con versiones/commit; registrar A04 y límites.

#### Scenario: Evidencia insuficiente de R01
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

### Requirement: R02 Informe, rúbrica y ensayo de los tres integrantes
El sistema SHALL cumplir el siguiente criterio: Contrastar 29 indicadores con rúbrica original, reunir evidencias y ensayar guion; los tres explican alguna parte ajena a su foco.

#### Scenario: Verificación de R02
- **GIVEN** las dependencias completadas y el entorno registrado
- **WHEN** se ejecuta la verificación de R02
- **THEN** Contrastar 29 indicadores con rúbrica original, reunir evidencias y ensayar guion; los tres explican alguna parte ajena a su foco.

#### Scenario: Evidencia insuficiente de R02
- **WHEN** falta evidencia o falla un criterio de aceptación
- **THEN** el issue permanece pendiente y se registra la limitación sin afirmar éxito

