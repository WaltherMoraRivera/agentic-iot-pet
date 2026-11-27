# Casos de prueba de T01

Estos casos verifican el [issue #4](https://github.com/wmrDev/agentic-iot-pet/issues/4) según el [contrato inicial](../../contracts/PROTOCOLO_INICIAL.md). Se ejecutan en `:shared:desktopTest` con `ProtocolCodec` y `DeviceAuthority`. El [registro de prueba](resultados/2026-11-26-T01-contrato.md) identifica el entorno, la revisión y el resultado observado.

## Preparación común

- Usar un reloj monotónico controlado que empiece en 0 ms. Cada caso parte de una autoridad nueva, con expresión `idle`, época 0 y sin propietario.
- Usar dos identidades ficticias autorizadas, A (Android) y B (Windows). Tomar el formato de UUID y JSON del [ejemplo de comando](../../contracts/examples/command-expression.json). Mantener la secuencia por cliente y arranque.
- Cada solicitud nueva usa un UUID nuevo y la siguiente secuencia. Un reintento exacto conserva el cuerpo completo. Observar estado y efectos a través de la respuesta y de una consulta independiente de estado.

## Autoridad y efectos

| Caso | Secuencia de prueba | Resultado esperado |
|---|---|---|
| `concurrentAcquireHasOneOwner` | Lanzar las adquisiciones de A y B concurrentemente contra la misma autoridad, con secuencia 1 para cada cliente. | Una responde `applied` y otra `BUSY`. El estado tiene un solo propietario y época 1. El caso acepta cualquiera de los dos ganadores. |
| `acquireAgainDoesNotExtendLease` | A adquiere en 0 ms y solicita otra adquisición, con ID y secuencia nuevos, en 2.000 ms. | Sigue como propietario en la misma época. Quedan 4.000 ms de permiso. |
| `leaseExpiresAtExactBoundary` | A adquiere en 0 ms. Consultar estado a los 5.999 ms y pedir `happy` a los 6.000 ms con la época anterior. | La consulta muestra 1 ms restante. La orden se rechaza con `LEASE_EXPIRED`, sin cambiar `idle`. |
| `renewRetryDoesNotExtendLeaseTwice` | A adquiere en 0 ms y renueva a los 2.000 ms. Repetir exactamente esa renovación a los 3.000 ms. | El reintento devuelve el resultado original. La consulta a los 3.000 ms muestra 5.000 ms restantes y la consulta a los 8.000 ms muestra que venció. |
| `wallClockDoesNotChangeLeaseOrSession` | Iniciar el reloj monotónico inyectado con un origen distinto de cero y avanzar 2.000 ms, luego 4.000 ms más. | El tiempo de actividad es relativo al arranque. Quedan 4.000 ms de permiso en la primera consulta y el permiso vence a los 6.000 ms. La sesión de estudio se verifica en su propia tarea. |
| `expressionRetryDoesNotApplyTwice` | A adquiere y pide `happy` por 1.000 ms en t=0. Repite en t=500 ms, al vencer el permiso y después de que B adquiera. Consultar estado a los 999 ms y a los 1.000 ms. | La primera orden aplica `happy` y el primer reintento devuelve la respuesta original. A los 999 ms sigue `happy` y a los 1.000 ms vuelve a `idle`. Los dos últimos reintentos se rechazan con `LEASE_EXPIRED` y `STALE_EPOCH`. |
| `requestWithChangedContentIsRejected` | Aplicar `happy` por 1.000 ms en t=0. A los 500 ms repetir su ID con `alert` por 5.000 ms, conservando lo demás. Consultar estado a los 999 ms y a los 1.000 ms. | `ID_CONFLICT` sin estado. A los 999 ms sigue `happy` y a los 1.000 ms vuelve a `idle`: permanecen el efecto inicial y su plazo. |
| `releaseRetryOnlyAppliesUntilNextAcquire` | A adquiere y libera. Repetir la liberación exacta y consultar estado. Después B adquiere y se repite otra vez la liberación antigua de A. | Antes de la adquisición de B, la repetición devuelve el resultado original y el estado sigue sin dueño. Después, el reintento anterior se rechaza y B mantiene la época nueva. |
| `evictedRetryIsStale` | A adquiere con secuencia 1 y pide `happy` con secuencia 2. En el mismo permiso, emitir 64 adquisiciones nuevas del mismo cliente, secuencias 3 a 66. Avanzar a los 1.000 ms y repetir la orden de secuencia 2. | El estado está en `idle` cuando se reintenta. La respuesta expulsada se rechaza con `STALE_SEQUENCE` y el estado sigue en `idle`. |
| `busyRequestConsumesSequence` | A tiene el permiso y B intenta adquirir con secuencia 1, recibiendo `BUSY`. A libera. B intenta adquirir con otro ID y secuencia 1, seguido de otro intento con ID nuevo y secuencia 2. | El intento que reutiliza la secuencia se rechaza con `STALE_SEQUENCE`. El de secuencia 2 obtiene el permiso. |
| `otherClientCannotUseCurrentEpoch` | A adquiere en 0 ms. A los 1.000 ms, B renueva, pide `alert` y libera con la época vigente de A, cada una con ID y secuencia nuevos de B. | Las tres se rechazan con `NOT_OWNER` sin estado. A sigue como propietario en la época 1 con 5.000 ms restantes y la expresión sigue en `idle`. |
| `ownerCannotRenewOrReleaseExpiredLease` | A adquiere en 0 ms. A los 6.000 ms, A renueva y luego libera con la época 1. Después B adquiere. | Ambas se rechazan con `LEASE_EXPIRED` sin estado. El estado no tiene propietario, conserva la época 1 con 0 ms restantes y B obtiene la época 2. |
| `rejectsMismatchedIdentityAndStaleContext` | En instancias separadas, enviar un cuerpo de A como B, usar un `boot_id` de un arranque anterior, y usar una época anterior tras una nueva adquisición. | `CLIENT_MISMATCH`, `STALE_BOOT` y `STALE_EPOCH`, respectivamente. Ninguno produce efectos ni devuelve estado privado en su respuesta de rechazo. |
| `authorizationAndValidityPrecedeDeduplication` | Enviar una orden fuera de rango, luego la orden válida con la misma secuencia. Repetir la orden válida sin identidad o con la identidad de otro cliente. | La orden válida conserva su secuencia. El reintento sin identidad o con otra identidad se rechaza sin revelar estado. |
| `authenticationPrecedesMessageValidity` | Con un permiso vigente de A, enviar sin identidad una orden con `duration_ms` 99. | `UNAUTHENTICATED` sin estado: la autenticación se comprueba antes que la validez del mensaje. |

## Validación del mensaje y formato de respuesta

| Caso | Entrada | Resultado esperado |
|---|---|---|
| `durationIsAnIntegerInRange` | Sobre órdenes nuevas y válidas, probar `100`, `5000`, `99`, `5001`, `"100"` y `true` como `duration_ms`. | Se aceptan los dos límites. Los demás valores generan `INVALID_MESSAGE` y dejan el estado previo. |
| `rejectsAmbiguousOrUnsupportedMessages` | Desde una orden válida, probar una clave repetida, un campo extra, una versión distinta y una acción desconocida. | Los mensajes inválidos se rechazan con `INVALID_MESSAGE`. |
| `controlFieldsDependOnTheRoute` | Decodificar adquisición, renovación y liberación con y sin `lease_epoch`. | La adquisición se acepta sin época y se rechaza con ella. Renovar y liberar exigen la época. |
| `requestSizeLimitUsesUtf8Bytes` | Usar el JSON válido del ejemplo y rellenar con espacio JSON hasta 1.024 bytes. En otra instancia, probar 1.025 bytes. | El primer cuerpo llega a la validación. El segundo se rechaza con `MESSAGE_TOO_LARGE` antes de aplicar efectos. |
| `wireReplyConfirmsAppliedStateAndKeepsRejectionPrivate` | Codificar una respuesta aplicada y un rechazo `UNAUTHENTICATED` sin `request_id`. | La aplicada lleva `status:"applied"`, el estado y `code:null`. El rechazo lleva `status:"rejected"`, `state:null` y `request_id:null`. |
| `repositoryExamplesMatchTheSimulatedFlow` | Cargar los ejemplos del repositorio, adquirir permiso a los 1.000 ms y aplicar la orden a los 1.100 ms. | El estado y la respuesta coinciden campo por campo con `state.json` y `reply-expression.json`. |

El conjunto tiene 21 casos: 15 de autoridad (`DeviceAuthorityTest`), 5 de JSON (`ProtocolTest`) y 1 con los ejemplos (`ContractExamplesTest`). El rechazo de UTF-8 inválido o de otro `Content-Type` (415) pertenece al transporte de T02/T03 y no se cubre aquí.

Para repetir el conjunto, ejecutar `.\gradlew.bat :shared:desktopTest --console=plain` y guardar el resultado observado según la [plantilla de pruebas](PLANTILLA_RESULTADO.md).
