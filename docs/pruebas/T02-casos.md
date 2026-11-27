# Casos de prueba de T02 preparados sin placa

Estos casos preparan la verificación del [issue #5](https://github.com/wmrDev/agentic-iot-pet/issues/5) (T02: identidad, emparejamiento USB y credenciales) a partir del [diseño aprobado](../superpowers/specs/2026-11-17-diseno-software.md) (sección 8), la tarea T02 del [plan](../superpowers/plans/2026-11-17-desarrollo-integrado.md) y el [contrato inicial](../../contracts/PROTOCOLO_INICIAL.md). Son casos planificados: no hay código de emparejamiento ni resultados. Ningún caso se da por aprobado hasta registrarlo con la [plantilla de pruebas](PLANTILLA_RESULTADO.md).

Los estados HTTP, códigos y valores que el contrato inicial no fija se toman de [`contracts/EMPAREJAMIENTO.md`](../../contracts/EMPAREJAMIENTO.md), que es una **propuesta pendiente de aprobación del equipo**. Cada uno se marca como *(propuesta)*; si el equipo cambia el contrato, se actualiza aquí. Las preguntas que bloqueaban la implementación están al final, con su respuesta propuesta.

En el contrato propuesto la huella viaja como 64 hexadecimales en minúsculas. Los datos sintéticos `AA:BB:CC:...:01` de abajo nombran la huella de cada dispositivo de prueba, no su formato en la red.

## Entornos

| Entorno | Qué es | Necesita placa |
|---|---|---|
| **Compartido** | `ST/pairing/PairingTest.kt` en `:shared:desktopTest`, con un dispositivo simulado en Kotlin, reloj monotónico inyectado, sensor de toque simulado y transporte TLS simulado que informa la huella del par | No |
| **Simulador** | Servidor HTTPS de `simulator/` con certificados exclusivos de prueba, fuera de Git. Hoy el simulador es un esqueleto sin protocolo: estos casos quedan bloqueados hasta que sirva `POST /v1/pair` | No |
| **Placa** | ESP32-S3 con firmware, USB, TTP223 y registro serie | Sí |

Un caso del entorno Compartido prueba las reglas del cliente y del modelo del dispositivo, no el firmware. Para evidencia `P04`/`P19` en hardware, los casos marcados "Repetir en placa" se repiten después en la placa sin presentar el resultado simulado como físico.

## Datos sintéticos comunes

- Dispositivo D1: `deviceId` `00000000-0000-4000-8000-0000000000d1`, huella de certificado `AA:BB:CC:...:01`. Dispositivo D2: `deviceId` `00000000-0000-4000-8000-0000000000d2`, huella `AA:BB:CC:...:02`.
- Secreto de un solo uso: `TEST-ONLY-SECRET-0001` y siguientes. Credenciales de cliente: `TEST-ONLY-CREDENTIAL-A`, `-B`, `-X`. Ningún valor real, ni siquiera de desarrollo, entra en el repositorio.
- Clientes A (Android) y B (Windows). X es un cliente ajeno que nunca se emparejó.
- Reloj monotónico del emisor controlado, con origen en 0 ms. El reloj civil del cliente es independiente y puede moverse.
- "Sin efecto" se observa con: registro de credenciales del dispositivo simulado (lista de `client_id`), `SecretStore` simulado (llamadas a `put`), grabador del transporte (bytes de aplicación enviados tras el handshake), contador de usos de la oferta y sumidero de registros capturado.

## Oferta y caducidad

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-01 `expiredOfferIsRejected` (límite) | D1 emite una oferta en t=0 ms, con validez de 60 000 ms en su reloj monotónico. En instancias separadas, A envía `POST /v1/pair` válido y toca en t=59 999 ms y en t=60 000 ms. | En 59 999 ms el alta procede tras el toque, porque la caducidad se comprueba al recibir la petición *(propuesta, pregunta 2)*. En 60 000 ms se rechaza porque `now_ms >= expires_at_ms`, la misma regla de límite de T01: 409 `OFFER_EXPIRED` *(propuesta)*. | En 60 000 ms: el dispositivo no registra `client_id`, `SecretStore.put` no se llama y no se espera toque. | Compartido. Repetir en placa |
| T02-02 `expiredOfferIsRejected` (reloj civil) | Oferta emitida en t=0. En t=30 000 ms se adelanta 1 h el reloj civil de A (la fecha mostrada indica que venció) y se envía el alta. En otra instancia, en t=60 000 ms se atrasa 1 h el reloj civil (la fecha mostrada parece vigente). | Solo decide el reloj monotónico del emisor: se acepta la primera y se rechaza la segunda, con el mismo código que T02-01. El cliente puede advertir con la fecha mostrada, pero no puede convertirla en aceptación. | En la segunda: ninguna credencial registrada ni guardada. | Compartido |
| T02-03 `offerCannotBeReused` | A se empareja con la oferta O1. Después, A y luego B envían O1 otra vez, dentro de los 60 s. | Ambos reintentos se rechazan con 409 `OFFER_CONSUMED` *(propuesta)*. Si A repite exactamente su misma petición dentro de los 30 000 ms de retención, recibe su resultado original; después, también `OFFER_CONSUMED`. La credencial de A sigue siendo válida. | El registro sigue con un solo `client_id`. Ningún toque nuevo se solicita y B no guarda nada. | Compartido. Repetir en placa |
| T02-04 `concurrentPairWithSameOfferHasOneWinner` | A y B envían O1 en paralelo, ambos con toque disponible. | Exactamente una alta aplicada. La otra se rechaza como oferta consumida. Se acepta cualquiera como ganadora. | Un solo `client_id` nuevo y una sola llamada a `put` en total. | Compartido |
| T02-05 `offerForAnotherDeviceIsRejected` | La oferta indica D1 (`AA:BB:CC:...:01`), pero la dirección responde como D2 con la huella `AA:BB:CC:...:02`. En otra instancia, D1 recibe un alta cuyo `deviceId` es D2. | El cliente aborta en el primer caso, igual que T02-07. En el segundo, D1 rechaza el alta con 409 `DEVICE_MISMATCH` *(propuesta)*. | El secreto no sale del cliente en el primero. D1 no registra credencial ni consume su oferta en el segundo *(propuesta, pregunta 4)*. | Compartido |
| T02-06 `malformedOfferIsRejectedLocally` | El contenido del QR tiene una huella truncada (`AA:BB`), un `deviceId` que no es un UUID canónico en minúsculas, un secreto vacío o un campo desconocido. | El cliente rechaza la oferta antes de conectarse y muestra un error sin el secreto. | El transporte no abre ninguna conexión y no se llama a `put`. | Compartido |

## Identidad y TLS

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-07 `wrongFingerprintNeverSendsCredentials` | Oferta válida con la huella `AA:BB:CC:...:01`. El transporte simulado presenta `AA:BB:CC:...:99` en el handshake. | El cliente aborta el handshake, el alta falla con un error local de identidad y no se reintenta sin verificar. | El grabador registra 0 bytes de aplicación: ni petición, ni `Authorization`, ni secreto. La oferta sigue sin consumirse en el dispositivo. | Compartido |
| T02-08 `SimulatorPairingTest.wrongFingerprintNeverSendsCredentials` | El simulador sirve un certificado de prueba distinto del de la oferta. El cliente real de `PairingClient` usa TLS de verdad. | El handshake falla en el cliente. | El registro de peticiones del simulador no contiene ningún `POST /v1/pair` ni el secreto. | Simulador. Repetir en placa |
| T02-09 `tlsValidationNeverDisabled` | El simulador presenta un certificado autofirmado desconocido, sin oferta asociada, a un cliente autenticado de `GET /v1/state`. Además se revisa el código: no hay `TrustManager` que acepte todo, ni verificación de host desactivada, ni opción de depuración que omita la validación. | La conexión falla en todas las variantes de compilación, incluida la de depuración. La revisión no encuentra un camino que la desactive. | No se envía la credencial ni se muestra el estado. | Simulador, con revisión estática |

## Confirmación física

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-10 `noTouchDoesNotPair` | Oferta vigente y alta válida de A, sin pulsación simulada hasta que vence la ventana de confirmación. En otra instancia, una pulsación llega justo después de esa ventana. | La primera petición recibe 202 `pending_confirmation`. La ventana dura 15 000 ms desde la reserva; al vencer, la repetición exacta recibe 409 `CONFIRMATION_EXPIRED` *(propuesta)*. Una pulsación soltada en `reserved_at_ms + 15 000` o después no la completa. | Ningún `client_id` se registra, `put` no se llama y A no recibe credencial. La oferta queda consumida: un nuevo intento con ella recibe `OFFER_CONSUMED` *(propuesta, pregunta 4)*. | Compartido |
| T02-11 `noTouchDoesNotPair` (TTP223) | Igual que T02-10, con el TTP223 real en modo momentáneo y sin tocarlo. Después, repetir tocándolo dentro de la ventana. | Sin toque, no hay alta. Con toque, el alta procede. | Igual que T02-10. Se registra el lote y el modo del sensor según H01. | Placa |

## Clientes, revocación y autorización

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-12 `firstClientPairsAfterTouch` | Dispositivo sin clientes y oferta O1 obtenida por el canal USB simulado. A verifica la huella, envía el alta y toca. | Alta aplicada. A guarda su credencial con `SecretStore.put` y `GET /v1/state` responde 200 con la estructura del contrato. El alta responde 202 `pending_confirmation` hasta el toque y luego 200 `applied` con `client_id` y credencial *(propuesta)*. | Una sola credencial. El secreto O1 queda consumido. | Compartido. Repetir en placa |
| T02-13 `secondClientNeedsAuthorizedOfferAndTouch` | A está emparejado. A obtiene una oferta O2 por el flujo autorizado. B la escanea, envía el alta y toca. | B queda emparejado con un `client_id` distinto. La credencial de A sigue respondiendo 200 en `GET /v1/state`. | Ninguna credencial de A cambia. | Compartido |
| T02-14 `pairingWithoutAuthorizedOfferIsRejected` | A está emparejado. B envía un alta con un secreto inventado (`TEST-ONLY-SECRET-9999`), sin pasar por A ni por USB, y toca. | 401 `UNAUTHENTICATED`, igual que si no hubiera oferta *(propuesta)*. A no se ve afectado. | El registro mantiene solo a A. El toque de B no completa nada. | Compartido |
| T02-15 `revokedClientCannotReadState` | A y B están emparejados. Se revoca A y luego A envía `GET /v1/state` con `TEST-ONLY-CREDENTIAL-A`. | 401 `UNAUTHENTICATED`. B sigue recibiendo 200. B revoca a A con `POST /v1/clients/revoke`; cualquier cliente emparejado puede revocar a cualquiera, sin toque *(propuesta)*. | La respuesta no contiene `boot_id`, `expression`, `owner_client_id` ni otro campo de estado. | Compartido. Repetir en placa |
| T02-16 `revokedClientCannotReuseCachedReplies` | A tenía el permiso y una orden aplicada con respuesta retenida. Se revoca A, que repite exactamente esa orden y una adquisición nueva. | 401 `UNAUTHENTICATED` en ambas. Las credenciales revocadas no recuperan respuestas anteriores, según el contrato. | No se devuelve la respuesta retenida ni hay un segundo efecto. El permiso vigente de A termina al revocarlo, como una liberación: `owner_client_id` null en el `GET /v1/state` de B *(propuesta, pregunta 8)*. | Compartido |
| T02-17 `foreignClientIsRejected` | X nunca se emparejó y envía `GET /v1/state` y `POST /v1/control/acquire` con `TEST-ONLY-CREDENTIAL-X`. | 401 `UNAUTHENTICATED` en ambas. | Sin estado en la respuesta ni secuencia consumida. | Compartido |
| T02-18 `pairingSecretOnlyAuthenticatesPairRoute` | Oferta vigente. Su secreto se usa como `Authorization: Bearer TEST-ONLY-SECRET-0001` en `GET /v1/state`, `POST /v1/control/acquire` y `POST /v1/commands`. | 401 `UNAUTHENTICATED` en las tres: el secreto no es una credencial de cliente. | Sin estado, sin permiso y sin efectos. Estos intentos no consumen ni invalidan la oferta: después, un `POST /v1/pair` válido con ese secreto todavía la reserva *(propuesta, pregunta 4)*. | Compartido |

## Transporte

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-19 `transportRejectsNonJsonContentType` | Un cliente autenticado envía una orden válida con `Content-Type: text/plain` y, en otra petición, con bytes UTF-8 inválidos. | 415 `UNSUPPORTED_MEDIA_TYPE`, en la capa de transporte y antes de `ProtocolCodec`. En `POST /v1/pair` se aplica igual, antes de analizar el cuerpo y sin tocar la oferta *(propuesta, pregunta 3)*. | Sin efectos ni secuencia consumida. | Simulador |

## Secretos y almacenamiento

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-20 `secretsAbsentFromLogs` | Se ejecuta en un mismo test un alta exitosa, una con huella incorrecta, una vencida y una revocación, con todos los registros capturados. Además se formatean como texto `PairingOffer`, `PairedDevice` y los errores. | La salida identifica los eventos sin contener los valores. | Ningún registro ni texto contiene `TEST-ONLY-SECRET-*`, `TEST-ONLY-CREDENTIAL-*` ni el encabezado `Authorization`. | Compartido |
| T02-21 `secretsAbsentFromLogs` (firmware) | Captura completa del registro serie durante el aprovisionamiento USB, el alta y la revocación. | Igual que T02-20. Las claves de proveedor no llegan a la placa ni a NVS. | La búsqueda de los valores sintéticos y de "Bearer" no encuentra coincidencias. | Placa |
| T02-22 `secretStoreRoundTripAndDelete` | Contrato de `SecretStore`: `put("pairing/D1", bytes)`, `get`, `delete` y `get` de nuevo. Se ejecuta con el almacén simulado y con `WindowsSecretStore` en Windows. Un alta fallida, como en T02-07 o T02-10, nunca llama a `put`. | `get` devuelve los mismos bytes y, tras `delete`, `null`. | Tras `delete` no queda el valor en el almacén. El archivo de Windows no contiene el valor en claro. | Compartido, y Windows sin placa. Android Keystore se prueba en un Android real |

## Aprovisionamiento y recuperación física

| ID / test planificado | Precondición y entrada | Resultado esperado | Sin efecto observable | Entorno |
|---|---|---|---|---|
| T02-23 `provisioningModeRequiresPhysicalEntry` | Arranque sin sostener el sensor y petición remota por red o USB para entrar en modo de aprovisionamiento. Luego, arranque sosteniéndolo. | Solo el arranque con el sensor activa el modo temporal. La orden remota se rechaza. | Sin el gesto físico no se emite oferta ni identidad por USB. | Placa |
| T02-24 `physicalResetClearsTrust` | A y B están emparejados y se ejecuta el restablecimiento físico. | Las credenciales de A y B devuelven 401 `UNAUTHENTICATED` y el dispositivo vuelve a exigir aprovisionamiento USB. | No queda ninguna credencial ni configuración de red. | Placa |
| T02-25 `cameraPermissionRequestedOnlyForScan` | La app Android se instala de nuevo y se usa sin escanear. Luego se abre el escaneo de QR. | El permiso de cámara se pide solo al abrir el escaneo. Si se deniega, el alta no avanza y queda la entrada de dirección manual. | Ningún uso de la cámara fuera del escaneo. | Android real, sin placa (manual) |

## Resumen

| Entorno | Casos | Cantidad |
|---|---|---|
| Compartido (sin placa) | T02-01 a T02-07, T02-10, T02-12 a T02-18, T02-20 y T02-22 | 17 |
| Android real, sin placa (manual) | T02-25 | 1 |
| Simulador (sin placa) | T02-08, T02-09 y T02-19 | 3 |
| Placa | T02-11, T02-21, T02-23 y T02-24 | 4 |

Total: 25 casos. Además, T02-01, T02-03, T02-08, T02-12 y T02-15 deben repetirse en placa para registrar `P04` y `P19` parciales. Los casos del simulador requieren antes que el simulador sirva HTTPS. Por el paso 2 del plan, los tests compartidos se escriben primero y se observa que fallan por falta de implementación.

## Preguntas abiertas del contrato que bloquean la implementación

Cada pregunta tiene una **decisión propuesta** en la sección final de [`contracts/EMPAREJAMIENTO.md`](../../contracts/EMPAREJAMIENTO.md), pendiente de aprobación del equipo. Siguen abiertas hasta que se aprueben.

1. **Formato de la oferta.** Faltan la codificación del QR, la versión, el algoritmo de huella (por ejemplo, SHA-256 del certificado o de la clave pública), el formato y la normalización de mayúsculas de la huella, y la longitud y codificación del secreto. *Respuesta propuesta en el contrato, pregunta 1.*
2. **Semántica de `expiresAt` y emisor.** ¿Es una fecha civil informativa o un plazo relativo? ¿Quién emite y valida el secreto de los clientes siguientes: la placa a petición de la app autorizada, o la app? ¿La caducidad se comprueba al recibir `POST /v1/pair` o al confirmar con el toque? *Respuesta propuesta en el contrato, pregunta 2.*
3. **Respuestas de `POST /v1/pair`.** Faltan el estado y el código para el alta exitosa, la oferta vencida, la ya consumida y el secreto desconocido, así como para `deviceId` ajeno, falta de toque, modo de alta inactivo y alta ya en curso. Tampoco se sabe si se aplican los límites de tamaño, el análisis estricto y el 415 del contrato inicial. *Respuesta propuesta en el contrato, pregunta 3.*
4. **Consumo ante fallo.** ¿Un intento fallido (vencido, `deviceId` ajeno, sin toque o el secreto presentado en otra ruta) consume o invalida la oferta? ¿Hay un límite de intentos? *Respuesta propuesta en el contrato, pregunta 4.*
5. **Confirmación.** Falta la duración de la ventana de toque. ¿La petición espera el toque o devuelve un estado pendiente? La versión 1 no tiene `pending_confirmation`. *Respuesta propuesta en el contrato, pregunta 5.*
6. **Concurrencia.** ¿Qué ocurre con una segunda alta mientras otra espera confirmación? ¿Una sola pulsación confirma solo la alta en curso? *Respuesta propuesta en el contrato, pregunta 6.*
7. **Credencial e identidad del cliente.** Falta el formato de la credencial (token portador, TLS mutuo o ambos). ¿Quién genera el `client_id` y cómo se fija la categoría Android o Windows sin que el cliente declare su rol? ¿Hay un número máximo de clientes? *Respuesta propuesta en el contrato, pregunta 7.*
8. **Revocación.** Faltan la ruta y quién puede revocar (el propio cliente, otro cliente autorizado o el gesto físico). ¿Qué pasa con el permiso de control vigente de un cliente revocado? ¿Cómo se entera el otro cliente? *Respuesta propuesta en el contrato, pregunta 8.*
9. **Presentación del secreto.** ¿Va en el cuerpo o en un encabezado de `POST /v1/pair`? De eso depende que T02-18 compruebe 401 o un rechazo del análisis. *Respuesta propuesta en el contrato, pregunta 9.*
10. **Reaprovisionamiento USB.** ¿Un nuevo aprovisionamiento con un cliente ya emparejado añade un cliente, reemplaza al anterior o exige primero el restablecimiento físico? *Respuesta propuesta en el contrato, pregunta 10.*
