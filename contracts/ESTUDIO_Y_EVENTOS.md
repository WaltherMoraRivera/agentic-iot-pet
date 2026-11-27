# Contrato de estudio, configuración y eventos — propuesta

**Estado: borrador para revisión.** Lo propone T04 (#8) para que T03 (#6), T05 (#9) y las apps lo revisen antes de implementarlo. Ninguna de estas rutas existe todavía en la placa ni en el simulador. La lógica local que respaldaría cada operación sí existe en `firmware/main/` (ver la última sección).

Extiende el [contrato v1](PROTOCOLO_INICIAL.md) sin cambiar lo ya acordado: mismo transporte HTTPS, autenticación, campos comunes, idempotencia, límites de tamaño y códigos de error. Solo se describe lo nuevo.

## Órdenes nuevas en `POST /v1/commands`

Mismos campos comunes que `expression.set`: `version`, `request_id`, `client_id`, `boot_id`, `lease_epoch`, `sequence`, `action` y `params`. Exigen permiso de control vigente.

| `action` | `params` | ¿Toque físico? | Efecto |
|---|---|---|---|
| `study.start` | `duration_ms`: entero de 60 000 a 7 200 000 | No | Inicia una sesión si no hay otra en curso o en pausa |
| `study.pause` | `{}` | No | Pausa la sesión en curso |
| `study.resume` | `{}` | No | Reanuda la sesión en pausa |
| `study.finish` | `{}` | No | Cierra la sesión en curso o en pausa |
| `config.set` | Al menos uno de: `sound_threshold` (número en `(0, 1]`), `sound_hysteresis` (número `>= 0`), `study_default_ms` (60 000–7 200 000), `volume_pct` (entero 0–100), `muted` (booleano) | Sí, si incluye `sound_threshold` o `sound_hysteresis` | Aplica todos los campos o ninguno |
| `alert.set` | `mode`: `"manual"` o `"auto"` | Sí | Activa el aviso manual o vuelve al modo automático |

La columna del toque sigue la tabla de la v3 adoptada en el diseño: sesiones no requieren toque; umbral y avisos sí. Si `config.set` mezcla campos con y sin toque, la orden completa espera el toque.

**Validación de `config.set`:** se valida la configuración resultante completa, no cada campo por separado. Se rechaza con `INVALID_MESSAGE` si `sound_threshold - sound_hysteresis <= 0`, si algún número no es finito o si `params` está vacío. Un rechazo no cambia nada de lo guardado.

## Respuestas

Mismo formato general que v1, con un tercer estado:

| `status` | Cuándo | Campos |
|---|---|---|
| `applied` | El efecto ya ocurrió | `state` actualizado, `code: null`, `confirmation: null` |
| `pending_confirmation` | La orden espera el toque físico | `state` actual (sin el cambio), `code: null`, `confirmation` |
| `rejected` | No se aplicó | `state: null`, `code` estable, `confirmation: null` |

`confirmation` en una respuesta pendiente:

```json
{
  "id": 12,
  "request_id": "<request_id de la orden>",
  "lease_epoch": 3,
  "expires_in_ms": 15000
}
```

La app **no** debe mostrar la orden como aplicada mientras esté pendiente. El resultado final se consulta en `GET /v1/state`.

Reglas de la confirmación (ya implementadas en `pet_state.c`):

- Dura 15 000 ms desde que la placa la crea.
- Solo una a la vez: otra orden que requiera toque se rechaza con `CONFIRMATION_BUSY` mientras haya una pendiente.
- Se cancela si cambia la época de control, si vence o si se pierde el permiso.
- Solo la confirma una pulsación que empezó **después** de crearla; un toque repetido no la aplica dos veces.
- Mientras está pendiente, el toque no inicia estudio ni pide voz.

Un reintento con el mismo `request_id` devuelve la respuesta original, como en v1. Si la confirmación ya se resolvió, `GET /v1/state` muestra su estado final.

## Códigos de error nuevos

| HTTP | Código | Significado |
|---|---|---|
| 409 | `WRONG_PHASE` | La orden de estudio no corresponde a la fase actual (por ejemplo, pausar sin sesión) |
| 409 | `CONFIRMATION_BUSY` | Ya hay una confirmación pendiente |

Los errores de rango o forma siguen usando `INVALID_MESSAGE`.

## Campos nuevos en `GET /v1/state`

```json
{
  "study": {
    "session_id": 4,
    "phase": "running",
    "elapsed_ms": 125000,
    "target_ms": 1500000
  },
  "alert_mode": "auto",
  "confirmation": null
}
```

- `phase`: `idle`, `running`, `paused`, `finished` o `interrupted`. `interrupted` aparece tras un reinicio con sesión activa; la placa no inventa la duración posterior al corte.
- `elapsed_ms` sale del reloj monotónico de la placa. Cambiar la hora del teléfono o del PC no lo modifica.
- `session_id` crece por sesión dentro de un arranque; junto con `boot_id` identifica la sesión.
- `confirmation`: `null` o `{id, request_id, lease_epoch, state, expires_in_ms}` con `state` = `pending`, `applied` o `cancelled`.

## Telemetría y eventos

Para recuperar datos tras un corte (P16, #28). Solo clientes autenticados; no requieren permiso de control.

- `GET /v1/telemetry/samples?after=<seq>&limit=<n>`
- `GET /v1/telemetry/events?after=<seq>&limit=<n>`

`after` es la última secuencia recibida (0 la primera vez); `limit` entre 1 y 50 para no superar los 4 096 bytes de respuesta.

```json
{
  "version": 1,
  "boot_id": "<uuid del arranque>",
  "first_seq": 6,
  "next_seq": 606,
  "dropped": 5,
  "gap": true,
  "items": [
    {"seq": 6, "uptime_ms": 5000, "sound_level": 0.021, "study_phase": "running"}
  ]
}
```

- Muestras: una por segundo, últimas 600 (10 minutos). Eventos: últimos 64, en un búfer separado.
- `gap: true` indica que se sobrescribieron entradas posteriores a `after`; la app debe mostrar la brecha, no ocultarla.
- Las secuencias empiezan en 1 en cada arranque. Si cambia `boot_id`, la app descarta el cursor y empieza desde 0.
- Repetir la misma página devuelve las mismas entradas: la app deduplica por `boot_id` y `seq`.
- `sound_level` es relativo a la escala digital (0 a 1), no dB.
- Evento: `{seq, uptime_ms, kind, value}` con `kind` = `touch`, `sound_reaction`, `study_changed` o `confirmation`.

## Pendiente de decidir con T03

- La placa identifica internamente la operación con un entero (`operation_id` en `pet_state.c`). La API debe asociarlo al `request_id` (UUID) de la orden y devolver este último.
- Si los comandos de estudio se envían en el mismo `POST /v1/commands` o en rutas propias.
- Qué hace la placa con una orden pendiente cuando la app se desconecta: este borrador propone cancelarla al vencer el permiso.

## Implementación local disponible

| Parte | Archivo |
|---|---|
| Fases y temporizador de estudio | `firmware/main/study_session.c` |
| Confirmación, prioridades y modo de aviso | `firmware/main/pet_state.c` |
| Umbral, histéresis e inhibición acústica | `firmware/main/sound_reaction.c` |
| Búfer de muestras y eventos | `firmware/main/telemetry.c` |
| Configuración y marca de sesión | `firmware/main/settings.c` |
