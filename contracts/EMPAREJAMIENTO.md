# Contrato de emparejamiento — versión 1

**Estado: propuesta, pendiente de aprobación del equipo.** Lo propone T02 ([issue #5](https://github.com/wmrDev/agentic-iot-pet/issues/5)) para responder las diez preguntas abiertas de [T02-casos.md](../docs/pruebas/T02-casos.md). Ninguna ruta, trama USB ni almacenamiento descrito aquí existe todavía en la placa, el simulador ni las apps. Nada de este documento se considera acordado hasta que el equipo lo apruebe y se registre en [DECISIONES.md](../docs/equipo/DECISIONES.md).

Extiende el [contrato v1](PROTOCOLO_INICIAL.md) sin cambiar lo ya fijado: HTTPS, JSON UTF-8, UUID canónicos en minúsculas, análisis estricto, representación de enteros, límites de tamaño, orden de validación, idempotencia y códigos de error. Solo se describe lo nuevo. Deriva de la sección 8 del [diseño](../docs/superpowers/specs/2026-11-17-diseno-software.md), de D05 y del paso 1 de T02 en el [plan](../docs/superpowers/plans/2026-11-17-desarrollo-integrado.md).

## Resumen del flujo

1. **Modo de aprovisionamiento.** Se activa solo al arrancar la placa sosteniendo el TTP223 (paso 3 de T02). No existe orden remota, por red ni por USB, para entrar en él. Dura hasta 300 000 ms de reloj monotónico o hasta el siguiente reinicio.
2. **Identidad.** La primera vez, la placa genera localmente su par de claves TLS, un certificado autofirmado y su `device_id`. La clave privada nunca sale de la placa.
3. **Primera app (USB).** El asistente de escritorio (`UsbProvisioner`) lee por USB la identidad, configura la red y pide una oferta. Muestra la oferta como QR para Android, o la usa directamente si la primera app es la de Windows.
4. **Alta.** La app verifica la huella durante el handshake TLS, antes de enviar un solo byte de aplicación. Luego envía `POST /v1/pair` con el secreto. La placa reserva la oferta y espera un toque físico. La app repite la misma petición hasta recibir la credencial o un rechazo.
5. **Clientes siguientes.** Un cliente ya emparejado pide una oferta con `POST /v1/pair/offers` y la muestra como QR. El cliente nuevo sigue el paso 4. Siempre se exige el toque.
6. **Revocación y recuperación.** Cualquier cliente emparejado puede revocar a cualquier cliente con `POST /v1/clients/revoke`. El restablecimiento físico borra toda la confianza.

La placa es la única que emite y valida secretos de alta y credenciales. Las apps nunca generan secretos aceptables por la placa.

## Identidad del dispositivo y TLS

| Elemento | Regla |
|---|---|
| Clave del servidor | ECDSA P-256, generada en la placa con el generador aleatorio criptográfico de ESP-IDF. Su costo de generación y de handshake se mide en T02/T03; no se ha medido todavía |
| Certificado | Autofirmado, generado en la placa. Las apps no lo validan contra una autoridad ni contra el nombre de host: lo validan por la huella |
| `fingerprint` | SHA-256 de la estructura `SubjectPublicKeyInfo` (DER) del certificado del servidor. Se transmite como 64 caracteres hexadecimales en minúsculas, sin separadores. La app puede mostrarla en mayúsculas agrupada con `:` para compararla a ojo, pero el valor que analiza y guarda es el de 64 minúsculas |
| `device_id` | UUID versión 4 canónico en minúsculas, generado en la placa junto a la clave |
| TLS | Versión 1.2 o superior. La comprobación de huella ocurre durante el handshake. Si no coincide, la conexión se cierra sin enviar petición, encabezado `Authorization` ni secreto, y no se reintenta sin verificar |

Reglas obligatorias para las apps y el simulador:

- No existe un `TrustManager`, callback o variante de depuración que acepte cualquier certificado. La única confianza aceptada es la huella guardada para ese `device_id`, o la de la oferta durante el alta.
- No hay respaldo a los certificados del sistema: un certificado firmado por una autoridad pública con otra clave también se rechaza.
- La dirección (`host`) puede cambiar con DHCP. La identidad la prueba la huella, no la dirección ni mDNS.
- Los certificados del simulador son exclusivos de pruebas, se generan fuera de Git y no se reutilizan en hardware real.

## Oferta de emparejamiento

La oferta es el contenido del QR y la respuesta de las dos formas de emitirla (USB y `POST /v1/pair/offers`). Es un objeto JSON en UTF-8 que se analiza con las mismas reglas estrictas del contrato v1. En el QR se codifica tal cual, en modo byte. Máximo 512 bytes.

| Campo | Tipo y regla |
|---|---|
| `version` | Entero, exactamente 1 |
| `device_id` | UUID canónico en minúsculas |
| `fingerprint` | Exactamente 64 caracteres `[0-9a-f]` |
| `host` | Cadena de 1 a 63 bytes: IPv4 en notación decimal con puntos o nombre `[a-z0-9-]{1,57}.local`. Es una pista de dirección, no una prueba de identidad |
| `port` | Entero de 1 a 65 535 |
| `client_kind` | `android` o `windows`. Lo fija quien emite la oferta, no el cliente nuevo |
| `secret` | Exactamente 43 caracteres base64url sin relleno (`[A-Za-z0-9_-]`), que codifican 32 bytes aleatorios generados por la placa |
| `expires_in_ms` | Entero de 1 a 60 000: tiempo restante en el reloj monotónico de la placa en el instante en que la placa compuso esta oferta |

Ejemplo (valores ficticios; el secreto real nunca se escribe en el repositorio):

```json
{
  "version": 1,
  "device_id": "00000000-0000-4000-8000-0000000000d1",
  "fingerprint": "0000000000000000000000000000000000000000000000000000000000000001",
  "host": "192.168.1.50",
  "port": 443,
  "client_kind": "android",
  "secret": "TEST-ONLY-SECRET-0001xxxxxxxxxxxxxxxxxxxxxx",
  "expires_in_ms": 60000
}
```

La app rechaza localmente, antes de abrir ninguna conexión, una oferta con campos faltantes, extra o repetidos, tipos o longitudes incorrectos, huella en mayúsculas o con separadores, o un `client_kind` distinto del suyo. El error mostrado no incluye el secreto.

## Caducidad

- La placa registra `expires_at_ms = issued_at_ms + 60 000` en su reloj monotónico. La oferta venció cuando `now_ms >= expires_at_ms`, la misma regla de límite que T01.
- La caducidad se comprueba **al recibir** `POST /v1/pair`. Una petición recibida en 59 999 ms reserva la oferta; una recibida en 60 000 ms se rechaza con `OFFER_EXPIRED`.
- Una vez reservada, rige solo la ventana de toque. Un toque válido dentro de la ventana completa el alta aunque la oferta haya cumplido ya sus 60 000 ms.
- El campo `expiresAt` de `PairingOffer` en el plan es un plazo local de la app: `scan_monotonic_ms + expires_in_ms`, medido con el reloj monotónico del equipo que lee la oferta. Como la app lee la oferta después de emitida, su plazo nunca es anterior al real. Solo sirve para mostrar el tiempo restante o dejar de intentar; nunca para aceptar una oferta. La fecha civil del equipo no interviene.
- Las ofertas viven solo en RAM. Un reinicio de la placa las invalida.

## Emisión de ofertas

La placa guarda como máximo **una** oferta a la vez, con estado `active`, `reserved` o `consumed`. Emitir una oferta nueva invalida la anterior si estaba `active`. Si hay una `reserved` esperando toque, la emisión se rechaza con `CONFIRMATION_BUSY`. Si ya hay 4 clientes registrados, se rechaza con `CLIENT_LIMIT_REACHED`.

La placa conserva el secreto en claro solo en RAM y solo mientras la oferta está `active`, para poder repetir una respuesta de emisión. Para validar guarda su SHA-256 y compara en tiempo constante.

### Primera oferta por USB

Solo durante el modo de aprovisionamiento. El canal es el puerto USB de la placa que el firmware ya usa para su diagnóstico; el detalle del periférico USB se verifica en la implementación de T02, sin presuponerlo aquí.

Cada trama es una línea que empieza por `@pp1 ` seguida de un objeto JSON y un salto de línea, con máximo 1 024 bytes. El firmware las distingue de sus líneas de registro y nunca reproduce una trama en el registro.

| `op` de la petición | Campos adicionales | Respuesta aplicada |
|---|---|---|
| `hello` | Ninguno | `device_id`, `fingerprint`, `port` y `host` (null mientras no haya red) |
| `wifi.set` | `ssid` (1–32 bytes), `password` (8–63 bytes, WPA2-PSK) | `host` obtenido, o rechazo `WIFI_FAILED` |
| `offer.create` | `client_kind` | `offer`, con la forma de la sección anterior |

Toda trama de petición lleva `version: 1`, `op` y `request_id`. Las respuestas llevan `version`, `op`, `request_id`, `status` (`applied` o `rejected`) y `code`. Fuera del modo de aprovisionamiento, cualquier trama se rechaza con `PROVISIONING_INACTIVE`, sin efectos. Las redes abiertas sin contraseña quedan fuera de la versión 1.

### Ofertas siguientes: `POST /v1/pair/offers`

Requiere `Authorization: Bearer` de un cliente emparejado. No requiere permiso de control.

Petición: `version`, `request_id`, `client_id`, `boot_id`, `sequence` (mismas reglas que el control en v1) y `client_kind`. Sin campos adicionales.

Respuesta aplicada: `version`, `request_id`, `status: "applied"`, `code: null` y `offer`. Rechazo: `status: "rejected"`, `code` y `offer: null`.

Idempotencia: consume secuencia como cualquier operación nueva. La caché de respuestas guarda la respuesta **sin** el secreto. Una repetición exacta devuelve la misma oferta, con el `expires_in_ms` restante, solo si sigue `active`; si no, devuelve `OFFER_EXPIRED` u `OFFER_CONSUMED` sin emitir otra. Es una excepción acotada a la regla de T01, como la de liberar.

## Alta: `POST /v1/pair`

No lleva encabezado `Authorization`: el secreto de la oferta, en el cuerpo, es la única autenticación de esta ruta. Una petición con `Authorization` se rechaza con `INVALID_MESSAGE`. El secreto no autentica ninguna otra ruta.

Petición, máximo 1 024 bytes, sin campos adicionales:

| Campo | Regla |
|---|---|
| `version` | Entero, exactamente 1 |
| `request_id` | UUID; se conserva en cada repetición de esta alta |
| `device_id` | UUID de la oferta |
| `secret` | 43 caracteres base64url, como en la oferta |
| `client_name` | 1 a 32 caracteres `[A-Za-z0-9 ._-]`, sin espacio inicial ni final. Es una etiqueta para listas; no otorga autoridad |

El cliente no envía `client_id` ni `client_kind`. La placa genera `client_id` (UUID versión 4) y toma `client_kind` de la oferta.

### Respuestas

Todas tienen `version`, `request_id`, `status`, `code`, `confirmation` y `pairing`. Máximo 4 096 bytes.

| HTTP | `status` | Contenido |
|---|---|---|
| 202 | `pending_confirmation` | `code: null`, `confirmation: {"expires_in_ms": <restante>}`, `pairing: null` |
| 200 | `applied` | `code: null`, `confirmation: null`, `pairing: {"client_id", "client_kind", "credential"}` |
| Según tabla de errores | `rejected` | `code` estable, `confirmation: null`, `pairing: null` |

La app no trata `pending_confirmation` como alta aplicada. Repite la petición **exacta** cada 1 000 ms mientras reciba 202. Solo al recibir 200 guarda la credencial con `SecretStore.put("pairing/<device_id>", ...)`. Cualquier otra respuesta es un fallo y no llama a `put`.

### Orden de validación

1. Transporte: `Content-Type: application/json` y UTF-8 válido, si no 415 `UNSUPPORTED_MEDIA_TYPE`; más de 1 024 bytes, 413 `MESSAGE_TOO_LARGE`; encabezado `Authorization` presente, 400 `INVALID_MESSAGE`.
2. Análisis estricto y versión: 400 `INVALID_MESSAGE`.
3. `device_id` distinto del propio: 409 `DEVICE_MISMATCH`. No se consulta ni modifica la oferta.
4. Si existe una reserva con el mismo `request_id`: comparar el cuerpo completo. Igual: devolver el estado actual de esa alta (202, 200 o el rechazo final). Distinto: 409 `ID_CONFLICT`.
5. Comparar en tiempo constante el SHA-256 del secreto con el de la oferta guardada. Sin oferta o sin coincidencia: 401 `UNAUTHENTICATED`. No se distingue "no hay oferta" de "secreto incorrecto".
6. Oferta vencida (`now_ms >= expires_at_ms`): 409 `OFFER_EXPIRED`, y se descarta. Oferta `reserved` o `consumed`: 409 `OFFER_CONSUMED`.
7. 4 clientes registrados: 409 `CLIENT_LIMIT_REACHED`. Otra confirmación física pendiente, por ejemplo de una orden de T04: 409 `CONFIRMATION_BUSY`. En ambos casos la oferta sigue `active`.
8. Reservar de forma atómica: oferta `reserved`, vinculada a este `request_id` y a este cuerpo; ventana de toque de 15 000 ms; señal de confirmación en la placa. Responder 202.

### Confirmación física

- Ventana de 15 000 ms desde la reserva, con las reglas de confirmación de [ESTUDIO_Y_EVENTOS.md](ESTUDIO_Y_EVENTOS.md): solo cuenta una pulsación que empezó después de la reserva, se toma el instante en que se suelta y vale si `release_ms < reserved_at_ms + 15 000`.
- Comparte el único espacio de confirmación de la placa: mientras un alta espera toque, una orden que requiera toque recibe `CONFIRMATION_BUSY`, y al revés. Un toque confirma una sola cosa y, mientras hay confirmación pendiente, no inicia estudio ni voz.
- Toque válido: de forma atómica la placa genera `client_id` y credencial, guarda el registro en NVS y marca la oferta `consumed`. Si la escritura en NVS falla, el alta termina en 500 `STORAGE_FAILED` sin registrar al cliente.
- Ventana vencida sin toque: el alta termina en 409 `CONFIRMATION_EXPIRED` y la oferta queda `consumed`. Una pulsación tardía no la completa.
- El resultado final (200 con credencial, o el rechazo) se retiene en RAM 30 000 ms para las repeticiones exactas. Después, una repetición recibe `OFFER_CONSUMED`. Si la app nunca recibió el 200, el cliente queda registrado sin uso; aparece en `GET /v1/clients` y se revoca como cualquier otro.

## Credenciales de cliente

| Elemento | Regla |
|---|---|
| Forma | Token portador opaco: 32 bytes aleatorios generados por la placa, en 43 caracteres base64url sin relleno |
| Uso | `Authorization: Bearer <credential>` en toda ruta salvo `POST /v1/pair`, como fija el contrato v1 |
| En la placa | NVS guarda por cliente `client_id`, `client_kind`, `client_name` y el SHA-256 de la credencial, nunca el valor. La comparación es en tiempo constante |
| En la app | El valor solo existe en `SecretStore` (Keystore en Android, almacenamiento protegido en Windows) |
| Autenticación | Se comprueba en cada petición, no una vez por conexión TLS |
| Máximo | 4 clientes, incluidos los registrados sin uso. Se revisará con la medición de memoria de T03 |

## Clientes y revocación

### `GET /v1/clients`

Requiere un cliente autenticado. Respuesta 200:

```json
{
  "version": 1,
  "clients": [
    {"client_id": "00000000-0000-4000-8000-00000000000a", "client_kind": "android", "client_name": "Android A", "self": true}
  ]
}
```

### `POST /v1/clients/revoke`

Requiere un cliente autenticado. Cualquier cliente emparejado puede revocarse a sí mismo o a otro. No requiere permiso de control ni toque.

Petición: `version`, `request_id`, `client_id`, `boot_id`, `sequence` y `target_client_id` (UUID). Respuesta con la forma de control del contrato v1: `status`, `state` y `code`. Un `target_client_id` no registrado se rechaza con 409 `UNKNOWN_CLIENT`.

Efectos atómicos al aplicar:

1. Se borra el registro del cliente en NVS. Su credencial devuelve 401 `UNAUTHENTICATED` desde la siguiente petición, también en conexiones TLS ya abiertas.
2. Si el revocado tenía el permiso de control, el permiso termina como en una liberación: `owner_client_id` null, `lease_remaining_ms` 0 y la época se conserva hasta la siguiente adquisición. Se cancela cualquier confirmación pendiente ligada a esa época.
3. Se descartan su caché de respuestas y su secuencia; las credenciales revocadas no recuperan respuestas anteriores.
4. Los demás clientes lo ven en `GET /v1/clients` y, si tenía el control, en `GET /v1/state`. La versión 1 no añade aviso automático.

Si un cliente se revoca a sí mismo y pierde la respuesta, la repetición recibe 401. La app interpreta ese 401, tras una autorrevocación enviada, como revocación completada y borra su credencial local.

## Aprovisionamiento con clientes ya emparejados y restablecimiento

- Entrar de nuevo en modo de aprovisionamiento no borra clientes. Permite cambiar la red y emitir una oferta que **añade** un cliente, con el mismo toque y el mismo límite de 4.
- Para quitar clientes se usa la revocación o el restablecimiento físico.
- El restablecimiento físico borra credenciales, red, clave TLS, certificado y `device_id`, y la placa vuelve a exigir aprovisionamiento USB. Las apps que guardaban la identidad anterior fallan en el handshake por huella distinta y deben emparejarse de nuevo. El gesto exacto se fija con las mediciones del TTP223 de H01; debe ser distinto del de arranque en aprovisionamiento y nunca puede pedirse por red ni por USB.

## Errores

Se reutilizan del contrato v1 `INVALID_MESSAGE` (400), `UNAUTHENTICATED` (401), `CLIENT_MISMATCH` (403), `STALE_BOOT` y `STALE_SEQUENCE` (409), `ID_CONFLICT` (409), `MESSAGE_TOO_LARGE` (413) y `UNSUPPORTED_MEDIA_TYPE` (415), y de T04 `CONFIRMATION_BUSY` (409). Los 415 y 413 se aplican también a `POST /v1/pair`, antes de analizar el cuerpo.

Códigos nuevos:

| HTTP | Código | Significado |
|---|---|---|
| 409 | `OFFER_EXPIRED` | La oferta venció en el reloj monotónico de la placa |
| 409 | `OFFER_CONSUMED` | La oferta ya fue reservada, usada o reemplazada |
| 409 | `DEVICE_MISMATCH` | El `device_id` de la petición no es el de esta placa |
| 409 | `CONFIRMATION_EXPIRED` | Terminó la ventana de toque sin confirmación |
| 409 | `CLIENT_LIMIT_REACHED` | Ya hay 4 clientes registrados |
| 409 | `UNKNOWN_CLIENT` | El cliente a revocar no está registrado |
| 500 | `STORAGE_FAILED` | No se pudo guardar el registro del cliente; no hubo alta |
| USB | `PROVISIONING_INACTIVE` | Trama USB fuera del modo de aprovisionamiento |
| USB | `WIFI_FAILED` | La placa no pudo unirse a la red indicada |

## Nunca se registra

En el firmware, las apps, el simulador y los tests no se escriben en registros, mensajes de error, `toString` ni ejemplos:

- El secreto de alta, la credencial, el encabezado `Authorization` ni sus hashes.
- La contraseña WiFi.
- Cuerpos de `POST /v1/pair`, respuestas de `POST /v1/pair/offers` y tramas USB `offer.create` o `wifi.set`.
- La clave privada TLS.

Se pueden registrar `device_id`, `fingerprint`, `client_id`, `client_kind`, `request_id`, códigos de error y transiciones de estado de la oferta. Las claves de proveedores de IA nunca viajan a la placa por ninguna de estas rutas.

## Preguntas abiertas y decisiones propuestas

**1. Formato de la oferta.** Decisión propuesta: JSON estricto en el QR (modo byte, ≤ 512 bytes), huella SHA-256 del `SubjectPublicKeyInfo` en 64 hexadecimales minúsculos y secreto de 32 bytes en base64url. Reutiliza el analizador estricto que ya existe y fija la huella a la clave, no al certificado, para que reemitir el certificado con la misma clave no rompa el emparejamiento. Alternativa descartada: una URI `scheme://?k=v` con huella del certificado completo y separadores `:`, que exige otro analizador y normalización de mayúsculas.

**2. `expiresAt` y emisor.** Decisión propuesta: la oferta lleva `expires_in_ms` relativo; la placa emite y valida todos los secretos y comprueba la caducidad al recibir `POST /v1/pair`. La placa es la única verificadora y no tiene reloj civil fiable. Alternativa descartada: que la app autorizada genere el secreto y se lo comunique a la placa, lo que duplica la autoridad y hace viajar el secreto una vez más; y comprobar la caducidad al tocar, que haría fallar sin aviso a quien escanea cerca del límite.

**3. Respuestas de `POST /v1/pair`.** Decisión propuesta: 202 `pending_confirmation`, 200 `applied` con credencial, y los rechazos de la tabla, con 401 único para secreto desconocido o modo inactivo. Se aplican los mismos límites, el análisis estricto y el 415 del contrato v1. Alternativa descartada: códigos separados para "secreto desconocido" y "sin oferta", que darían a cualquier equipo de la red un oráculo sobre el estado de alta.

**4. Consumo ante fallo.** Decisión propuesta: consumen la oferta solo la reserva y lo que sigue a ella (alta completada o ventana vencida). Vencimiento la descarta. No la consumen un `device_id` ajeno, un secreto desconocido, una petición malformada, `CONFIRMATION_BUSY`, el límite de clientes ni el uso del secreto como `Bearer` en otras rutas. No hay contador de intentos: el secreto tiene 256 bits y 60 s de vida, y un contador permitiría a cualquier equipo de la red anular un alta legítima. Alternativa descartada: invalidar la oferta tras 5 fallos.

**5. Confirmación.** Decisión propuesta: ventana de 15 000 ms, la misma de T04; la petición devuelve 202 y la app repite la petición exacta cada 1 000 ms. El servidor HTTP de ESP-IDF atiende los manejadores desde su tarea de servidor, así que esperar el toque dentro de la petición bloquearía las demás durante la ventana; debe confirmarse con la configuración que se use. Alternativa descartada: petición bloqueante de larga espera.

**6. Concurrencia.** Decisión propuesta: una sola oferta y una sola reserva a la vez. Otra alta con el mismo secreto recibe `OFFER_CONSUMED`; mientras hay una confirmación pendiente, nuevas ofertas o altas reciben `CONFIRMATION_BUSY`. Un toque confirma solo la reserva en curso. Alternativa descartada: varias ofertas simultáneas con una cola de confirmaciones, que exige mostrar en la placa a quién confirma cada toque.

**7. Credencial e identidad del cliente.** Decisión propuesta: token portador por cliente sobre HTTPS con huella fijada; `client_id` lo genera la placa; `client_kind` lo fija quien emite la oferta y lo confirma el toque; máximo 4 clientes. El contrato v1 ya usa `Authorization: Bearer`, la revocación es borrar un hash y no requiere que la placa verifique certificados de cliente ni que las apps generen claves de cliente. Alternativa descartada: TLS mutuo, con más memoria y trabajo por handshake en el ESP32-S3, generación de claves de cliente en Keystore y Windows, y un simulador más complejo, sin una mejora proporcional para una red local con huella fijada.

**8. Revocación.** Decisión propuesta: `POST /v1/clients/revoke`, cualquier cliente emparejado sobre cualquiera, sin toque; el permiso de control del revocado termina como una liberación; los demás lo ven en `GET /v1/clients` y `GET /v1/state`. Revocar solo quita confianza, así que el peor abuso es negar servicio, recuperable por USB o restablecimiento físico. Alternativa descartada: exigir toque para revocar a otro, que impediría quitar a tiempo un teléfono perdido.

**9. Presentación del secreto.** Decisión propuesta: en el cuerpo de `POST /v1/pair`, con `Authorization` prohibido en esa ruta. El encabezado queda reservado a credenciales de cliente; por eso T02-18 comprueba 401 en las otras rutas y la capa de autenticación nunca ve el secreto. Alternativa descartada: un esquema `Authorization: Pairing <secreto>`, que añade otro camino de autenticación y otro encabezado sensible que filtrar en registros.

**10. Reaprovisionamiento USB.** Decisión propuesta: con clientes ya emparejados, el modo de aprovisionamiento cambia la red y puede añadir un cliente con toque, sin reemplazar ni borrar; quitar exige revocación o restablecimiento físico. Quien puede arrancar la placa sosteniendo el sensor ya podría restablecerla, así que no se pierde seguridad. Alternativa descartada: exigir restablecimiento previo, que obligaría a reemparejar todo solo para cambiar de red.

## Decisiones que el equipo debe confirmar expresamente

- La identidad del servidor se valida **solo** por huella de clave, sin autoridad ni nombre de host. No es desactivar TLS, pero el revisor debe comprobar que la implementación no se convierta en un "aceptar todo".
- Cualquier cliente emparejado puede revocar a otro sin toque.
- El modo de aprovisionamiento puede añadir clientes sin que los existentes lo autoricen, con presencia física.
- Máximo de 4 clientes y retención de 30 000 ms de la credencial en RAM para repeticiones.
- Las tramas USB transportan el secreto y la contraseña WiFi por el mismo puerto que el registro serie; la prueba T02-21 debe buscar valores sintéticos fuera de las tramas `@pp1`.
- El gesto de restablecimiento físico y la duración del sostenido de arranque quedan pendientes de H01.
