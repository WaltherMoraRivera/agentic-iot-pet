# Contrato de la primera integración — versión 1

Estado: especificación de implementación para T01–T03; todavía no hay servidor ni cliente. Alcance inicial: identidad, estado, permiso temporal y una expresión. Los comandos de estudio, voz y sincronización se incorporarán junto con sus tareas y pruebas; no se anuncian como implementados.

## Transporte e identidad

HTTPS con identidad verificada por el emparejamiento. En desarrollo se generan credenciales/certificados exclusivos de pruebas, fuera de Git; no usar credenciales de producción ni un cliente que acepte todo certificado.

`Content-Type: application/json`, UTF-8 y `Authorization: Bearer <credencial-del-cliente>`. El servidor vincula la credencial a un `client_id` registrado y a su categoría Android/Windows. El cliente no elige su autoridad declarando un rol. No registrar el encabezado Authorization.

IDs de cliente, solicitud y arranque: UUID con representación canónica en minúsculas. Un arranque nuevo de placa genera `boot_id` nuevo. No usar la hora del equipo como única fuente de IDs.

Órdenes/control: máximo 1.024 bytes por petición. Respuestas de esta primera integración: máximo 4.096 bytes. Rechazar JSON con claves repetidas, tipos incorrectos, propiedades desconocidas y valores fuera de rango. No convertir cadenas a números ni aceptar booleanos como enteros. Las futuras páginas de eventos y capturas de audio tendrán límites separados.

Los campos enteros de la petición se escriben con dígitos decimales, sin signo, fracción ni exponente. Por ejemplo, una duración de `100` es válida, y `100.0` y `1e2` se rechazan. Esta regla de representación se comprueba sobre el cuerpo recibido, junto con los tipos y rangos del esquema.

## Estado: `GET /v1/state`

Solo clientes autenticados. Respuesta 200 con `version`, `boot_id`, `expression`, `owner_client_id`, `lease_epoch`, `lease_remaining_ms` y `uptime_ms`. `owner_client_id` es null cuando no hay centro; en ese caso `lease_remaining_ms=0`. `lease_epoch` es un contador creciente dentro del arranque, inicialmente 0. `uptime_ms` es monotónico, no una fecha civil.

La expresión es una de `idle`, `happy`, `thinking`, `alert`. Estas etiquetas nombran una intención visual; cada dibujo pertenece al firmware. Las apps no envían píxeles.

Ejemplo: [state.json](examples/state.json). Los valores son datos ficticios de prueba.

## Permiso temporal de control

Rutas `POST /v1/control/acquire`, `/renew` y `/release`.

Todas reciben `version:1`, `request_id`, `client_id`, `boot_id` y `sequence` (entero 1–2.147.483.647). Renovar/liberar añaden `lease_epoch` (entero positivo). Adquirir no incluye época porque solicita una nueva. Sin campos adicionales. La secuencia es creciente por cliente y arranque para operaciones nuevas, incluyendo órdenes; un reintento repite la solicitud exacta.

Las respuestas aplicadas contienen `version:1`, `request_id`, `status:"applied"`, `state` (estructura de `/state`) y `code:null`. Los rechazos contienen la misma versión/solicitud, `status:"rejected"`, `state:null` y `code` estable.

- **Adquirir:** con centro ausente, asignar el cliente, incrementar época y dar 6.000 ms desde el reloj del receptor. Con ese mismo cliente ya activo, devolver su permiso vigente sin prolongarlo. Con otro cliente activo, rechazar `BUSY`. El relevo preferente Windows se añadirá en T06, no se simula en T01 como una carrera entre clientes.
- **Renovar:** solo propietario de época vigente y no vencida; nueva caducidad = ahora + 6.000 ms. La app propone renovación cada 2.000 ms.
- **Liberar:** solo propietario vigente; dejar centro null. La época se conserva hasta una adquisición nueva. Una repetición válida de la misma liberación devuelve el resultado anterior y no vuelve a producir efectos.
- **Vencer:** cuando `now_ms >= expires_at_ms`, ningún comando nuevo de ese permiso puede aplicarse. Vencer no detiene temporizador ni reacciones locales.

La adquisición y cualquier relevo son operaciones atómicas en la placa. El simulador debe reproducir esa exclusión, aunque reciba peticiones concurrentes.

## Orden: `POST /v1/commands`

Forma inicial especificada en [command.schema.json](v1/command.schema.json). Ejemplo: [command-expression.json](examples/command-expression.json).

| Campo | Regla |
|---|---|
| `version` | Entero, exactamente 1 |
| `request_id` | UUID; se conserva en un reintento |
| `client_id` | UUID asociado a la credencial |
| `boot_id` | UUID del arranque actual |
| `lease_epoch` | Entero positivo igual a la época activa |
| `sequence` | Entero 1–2.147.483.647, nuevo y creciente |
| `action` | Inicialmente solo `expression.set` |
| `params.expression` | `idle`, `happy`, `thinking` o `alert` |
| `params.duration_ms` | Entero de 100 a 5.000, inclusive |

Una expresión temporal vuelve a la expresión base al vencer; en la integración inicial la base es `idle`. No puede bloquear un indicador de captura o error relevante cuando esas funciones se incorporen. Una respuesta exitosa confirma el estado aplicado, no solo recepción de bytes.

Respuesta: [reply-expression.json](examples/reply-expression.json), misma forma general del control. En esta primera integración no hay `pending_confirmation`; T04 lo añadirá junto con un flujo completo de confirmación, sin que el cliente inicial lo trate como aplicado.

## Orden de validación e idempotencia

1. Limitar tamaño y parsear estrictamente; autenticar identidad antes de revelar estado o resultados privados.
2. Verificar versión, identidad asociada y arranque actual.
3. Buscar duplicado por cliente/arranque/solicitud; comparar todos los campos. Mismo ID con otro contenido: `ID_CONFLICT`.
4. Un duplicado solo devuelve su resultado si pertenece a la época que sigue vigente y su permiso no venció; como excepción, la misma liberación puede devolver su resultado hasta una nueva adquisición. Un duplicado de adquisición conserva la época otorgada mientras no cambie ni venza. Nunca usar caché de otra época para autorizar una acción.
5. Para una solicitud nueva, comprobar secuencia y autoridad/época/caducidad, aplicar atómicamente, guardar respuesta y avanzar secuencia.

Se retienen las últimas 64 respuestas por cliente del arranque actual. Se mantiene además la secuencia máxima consumida, incluso si se expulsa una respuesta: una secuencia antigua sin resultado retenido devuelve `STALE_SEQUENCE`, nunca se reejecuta. Solicitudes autenticadas, bien formadas y con secuencia nueva consumen esa secuencia también si son rechazadas por estado; un nuevo intento después de cambiar el estado usa ID y secuencia nuevos. Una desconexión no habilita a inventar otro ID para repetir una acción de resultado incierto.

La secuencia y el cache se serializan con la aplicación de efectos. El cache puede guardar rechazos; una solicitud malformada o no autenticada no crea estado de deduplicación. Las credenciales revocadas no recuperan respuestas anteriores. Al reiniciar, las apps descartan órdenes pendientes y vuelven a consultar estado. Una respuesta repetida describe el resultado original; consultar `/state` para mostrar el estado actual después de un reintento.

## Errores y HTTP

| HTTP | Código | Significado |
|---|---|---|
| 400 | `INVALID_MESSAGE` | JSON, claves, tipo, rango, versión o acción no soportados |
| 401 | `UNAUTHENTICATED` | Credencial ausente/no válida; no revelar datos de estado |
| 403 | `CLIENT_MISMATCH` / `NOT_OWNER` | Identidad distinta o cliente sin control |
| 409 | `STALE_BOOT` / `STALE_EPOCH` / `STALE_SEQUENCE` | Mensaje de contexto anterior |
| 409 | `BUSY` / `ID_CONFLICT` | Otro centro activo o ID reutilizado con contenido distinto |
| 409 | `LEASE_EXPIRED` | El permiso ya no es válido |
| 413 | `MESSAGE_TOO_LARGE` | Petición supera el límite |
| 415 | `UNSUPPORTED_MEDIA_TYPE` | No es JSON UTF-8 admitido |

En error sin un `request_id` válido, la respuesta puede llevar `request_id:null`. Nunca incluir trazas, tokens o contraseñas. El cliente debe manejar una respuesta desconocida como fallo, sin mostrarla como éxito. Si falla el transporte o el servidor, la app conserva la incertidumbre de la operación y consulta/reintenta con el mismo ID cuando sea válido.

## Casos de aceptación de T01

- Orden válida con permiso vigente: `applied`, una sola modificación y caducidad temporal de expresión.
- Duración 100 y 5.000: aceptadas; 99, 5.001, `"100"` y `true`: rechazadas sin efectos.
- Credencial de cliente B con cuerpo de A: rechazo aunque el cuerpo reclame una época válida.
- Dos adquisiciones concurrentes: exactamente un propietario.
- Repetición exacta: mismo resultado, contador de aplicaciones igual a 1.
- Mismo ID con otros parámetros: `ID_CONFLICT`, sin segundo efecto.
- A los 6.000 ms exactos el permiso sin renovar ya venció; una renovación válida a los 2.000 ms lo extiende a 8.000 ms. Repetirla no lo extiende otra vez.
- Arranque o época antiguos: rechazo; un reinicio nunca aplica una orden guardada del arranque anterior.
- Más de 64 respuestas: una repetición expulsada no produce otro efecto.
- JSON con clave repetida, campos extra o tamaño excesivo: rechazo.

## Evolución

T04 define comandos/configuración, confirmaciones, gestos y eventos; T05 define cursores y sincronización; T06 define transferencia de centro; T07 define herramientas/presupuesto; T08 define permisos de captura y formato de audio. Cada ampliación añade ejemplos y pruebas antes de repartir sus consumidores. Un cambio incompatible aumenta versión; una versión desconocida se rechaza explícitamente.
