# Plan de medición de consumo EVA_02

Estado: **NO EJECUTADA**. Este plan fija método, condiciones y registro. Un valor estimado por software nunca se presenta como lectura de un instrumento.

## Placa

### Instrumento

Elegir uno y anotar marca y modelo en [DISPOSITIVOS.md](DISPOSITIVOS.md#instrumentos):

1. **Medidor USB en línea que deje pasar datos** entre el puerto del PC (o un hub USB alimentado) y la placa. No figura en el inventario: conseguirlo es una decisión del equipo, no de este plan.
2. **Multímetro en serie** en la línea de 5 V mediante un adaptador USB con borneras o un cable preparado para ello. Empezar siempre por la escala mayor. En C5 y C6 (Wi-Fi y amplificador activos) usar la **escala de 10 A**, no la de mA, para no fundir el fusible de mA. No abrir el circuito en la línea de 3,3 V con la placa conectada al PC.

La SuperMini tiene un solo USB-C para alimentación y consola USB Serial/JTAG (ver la preparación común de [FICHAS.md](FICHAS.md)). Con log serie, la placa se alimenta desde el PC o un hub alimentado; con fuente externa (la fuente USB-C 5 V / 3 A está **por adquirir** según el [inventario](../../hardware/INVENTARIO.md)), la sesión va sin log y se registra aparte. Anotar en cada fila la fuente real.

Si no hay forma segura de medir corriente, la condición queda `NO EJECUTADA` con el motivo y se mide solo la tensión de alimentación bajo carga. Solo se registra `LIMITACIÓN ACORDADA` si existen fecha y aprobación documentadas.

### Condiciones

Medir cada estado al menos 60 s, con el mismo firmware y la misma fuente. Anotar mínimo, máximo y valor típico observado, en mA a la tensión medida. El mínimo y el máximo son lecturas del instrumento a su frecuencia de refresco (anotarla si el manual la indica); no son picos transitorios y no se presentan como tales.

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

| ID | Fuente real | Escala | Tensión (V) | Corriente mín. (mA) | Corriente máx. (mA) | Típica (mA) | Duración (s) | Observaciones |
|---|---|---|---|---|---|---|---|---|
| C1 | | | | | | | | |
| C2 | | | | | | | | |
| C3 | | | | | | | | |
| C4 | | | | | | | | |
| C5 | | 10 A | | | | | | |
| C6 | | 10 A | | | | | | |

Anotar además cualquier reinicio (`reset_reason` en el log serie, en las sesiones con log) o caída de tensión durante el sonido: es el síntoma de alimentación insuficiente que debe revisar H01. Si el instrumento no permite observar la caída de tensión durante el sonido, anotarla como «no medida»; no se deduce de la ausencia de reinicios.

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
