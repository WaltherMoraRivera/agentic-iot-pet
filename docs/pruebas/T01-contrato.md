# Casos de prueba de T01

Estos casos preparan el [issue #4](https://github.com/wmrDev/agentic-iot-pet/issues/4) según el [contrato inicial](../../contracts/PROTOCOLO_INICIAL.md). Se ejecutarán en `:shared:desktopTest` cuando estén disponibles `ProtocolCodec` y `DeviceAuthority`. El estado actual de los casos es **NO EJECUTADA**.

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
| `wallClockDoesNotChangeLeaseOrSession` | Alterar la hora civil de prueba mientras el reloj inyectado conserva el valor en 2.000 ms. Avanzar el reloj monotónico a 6.000 ms. | En T01 el permiso conserva sus 4.000 ms hasta que avance el reloj monotónico y vence a los 6.000 ms. La sesión de estudio se verifica en su propia tarea. |
| `expressionRetryDoesNotApplyTwice` | A adquiere y pide `happy` por 1.000 ms en t=0. En t=500 ms reenvía el cuerpo exacto. Consultar estado a los 999 ms y a los 1.000 ms. | La primera orden aplica `happy`. Se devuelve la respuesta original en el reintento. A los 999 ms sigue `happy` y a los 1.000 ms vuelve a `idle`, con el plazo original. |
| `requestWithChangedContentIsRejected` | Aplicar la orden de expresión y repetir su ID con otros parámetros, conservando lo demás. | `ID_CONFLICT`. Permanecen el efecto inicial y su plazo. |
| `releaseRetryOnlyAppliesUntilNextAcquire` | A adquiere y libera. Repetir la liberación exacta y consultar estado. Después B adquiere y se repite otra vez la liberación antigua de A. | Antes de la adquisición de B, la repetición devuelve el resultado original y el estado sigue sin dueño. Después, el reintento anterior se rechaza y B mantiene la época nueva. |
| `evictedRetryIsStale` | A adquiere con secuencia 1 y pide `happy` con secuencia 2. En el mismo permiso, emitir 64 adquisiciones nuevas del mismo cliente, secuencias 3 a 66. Avanzar a los 1.000 ms y repetir la orden de secuencia 2. | El estado está en `idle` cuando se reintenta. La respuesta expulsada se rechaza con `STALE_SEQUENCE` y el estado sigue en `idle`. |
| `busyRequestConsumesSequence` | A tiene el permiso y B intenta adquirir con secuencia 1, recibiendo `BUSY`. A libera. B intenta adquirir con otro ID y secuencia 1, seguido de otro intento con ID nuevo y secuencia 2. | El intento que reutiliza la secuencia se rechaza con `STALE_SEQUENCE`. El de secuencia 2 obtiene el permiso. |
| `rejectsMismatchedIdentityAndStaleContext` | En instancias separadas, enviar un cuerpo de A como B, usar un `boot_id` de un arranque anterior, y usar una época anterior tras una nueva adquisición. | `CLIENT_MISMATCH`, `STALE_BOOT` y `STALE_EPOCH`, respectivamente. Ninguno produce efectos ni devuelve estado privado en su respuesta de rechazo. |

## Validación del mensaje

| Caso | Entrada | Resultado esperado |
|---|---|---|
| `durationIsAnIntegerInRange` | Sobre órdenes nuevas y válidas, probar `100`, `5000`, `99`, `5001`, `"100"` y `true` como `duration_ms`. | Se aceptan los dos límites. Los demás valores generan `INVALID_MESSAGE` y dejan el estado previo. |
| `malformedRequestsDoNotConsumeSequence` | Desde el ejemplo, probar una clave repetida, un campo extra, una versión distinta y una acción desconocida. Después enviar una orden válida con la misma secuencia. | Los mensajes inválidos se rechazan con `INVALID_MESSAGE`. La secuencia sigue disponible para la solicitud válida. |
| `requestSizeLimitUsesUtf8Bytes` | Usar el JSON válido del ejemplo y rellenar con espacio JSON hasta 1.024 bytes. En otra instancia, probar 1.025 bytes. | El primer cuerpo llega a la validación. El segundo se rechaza con `MESSAGE_TOO_LARGE` antes de aplicar efectos. |

Al implementar, ejecutar `gradlew.bat :shared:desktopTest` primero con la lógica pendiente para comprobar el fallo pertinente. Repetir después de implementar y guardar el resultado observado según la [plantilla de pruebas](PLANTILLA_RESULTADO.md).
