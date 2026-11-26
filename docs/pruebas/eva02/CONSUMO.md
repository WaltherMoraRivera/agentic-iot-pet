# Plan de medición de consumo EVA_02

Estado: **NO EJECUTADA**. Este plan fija método, condiciones y registro. Un valor estimado por software nunca se presenta como lectura de un instrumento.

## Placa

### Instrumento

Elegir uno y anotar marca y modelo en [DISPOSITIVOS.md](DISPOSITIVOS.md#instrumentos):

1. **Medidor USB en línea** entre la fuente de 5 V y la placa. No figura en el inventario: conseguirlo es una decisión del equipo, no de este plan.
2. **Multímetro en serie** en la línea de 5 V mediante un adaptador USB con borneras o un cable preparado para ello. Usar la escala de mA con su fusible; empezar por la escala mayor. No abrir el circuito en la línea de 3,3 V con la placa conectada al PC.

Si no hay forma segura de medir corriente, registrar `LIMITACIÓN` con el motivo y medir solo la tensión de alimentación bajo carga.

### Condiciones

Medir cada estado al menos 60 s, con el mismo firmware y la misma fuente. Anotar mínimo, máximo y valor típico observado, en mA a la tensión medida.

| ID | Estado | Cómo provocarlo |
|---|---|---|
| C1 | Reposo tras arranque, sin Wi-Fi | Firmware sin credenciales de red o con radio desactivada, si la versión lo permite |
| C2 | Wi-Fi conectado, sin cliente | Placa en la red, app cerrada |
| C3 | App conectada, telemetría 1/s | Sesión iniciada en Android |
| C4 | OLED con expresión activa | `expression.set` repetido o expresión base visible |
| C5 | Sonido en reproducción | Aviso con el MAX98357A, volumen anotado |
| C6 | Todo simultáneo | Wi-Fi, app, OLED, sonido y micrófono activos |

Si una condición no se puede provocar con el firmware disponible, dejarla `NO EJECUTADA` con el motivo.

### Registro

| ID | Tensión (V) | Corriente mín. (mA) | Corriente máx. (mA) | Típica (mA) | Duración (s) | Observaciones |
|---|---|---|---|---|---|---|
| C1 | | | | | | |
| C2 | | | | | | |
| C3 | | | | | | |
| C4 | | | | | | |
| C5 | | | | | | |
| C6 | | | | | | |

Anotar además cualquier reinicio (`reset_reason` en el log serie) o caída de tensión durante el sonido: es el síntoma de alimentación insuficiente que debe revisar H01.

<a id="telefono"></a>

## Teléfono

Android no ofrece una lectura de instrumento del consumo de una app. Se registran dos datos y se etiquetan por separado:

1. **Observación directa:** porcentaje de batería al inicio y al final de E11, hora exacta y temperatura ambiente aproximada. Pantalla apagada, sin cargar, mismas apps abiertas en ambos teléfonos.
2. **Estimación por software:** si ADB funciona, al terminar E11:

```powershell
adb shell dumpsys batterystats --charged io.github.wmrdev.petiot
```

Antes de empezar, `adb shell dumpsys batterystats --reset` deja limpio el contador. El resultado se anota como **estimación del sistema operativo**, nunca como medición física.

### Registro

| Teléfono | Batería inicial (%) | Batería final (%) | Duración (min) | Ahorro de batería | Estimación batterystats | Observaciones |
|---|---|---|---|---|---|---|
| INFINIX ZERO 30 5G | | | | | | |
| Galaxy S24 Ultra | | | | | | |

Una diferencia de 1 % en 30 min está dentro de la resolución del indicador: no permite comparar teléfonos ni concluir que la app consume poco.
