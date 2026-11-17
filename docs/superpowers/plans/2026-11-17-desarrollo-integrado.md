# Pet IoT — Implementation Plan por entregas integradas

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **Portabilidad para el equipo:** la indicación anterior aplica si esas habilidades están instaladas. No son dependencias del repositorio. Sin ellas, seguir los pasos, verificaciones y revisión humana aquí definidos. El encargo del integrante determina la tarea y el método; no ejecutar el plan completo sin que lo solicite.

**Goal:** entregar el 16 de diciembre un personaje físico con autonomía local, apps Android/Windows coordinadas, asistente opcional y evidencia de funcionamiento.

**Architecture:** ESP32-S3 como autoridad del estado físico y del permiso de control. Aplicaciones KMP/Compose comparten lógica y datos; adaptadores de plataforma resuelven voz, credenciales y ciclo de vida. Simulador JVM y firmware consumen el mismo contrato, sin sustituir pruebas físicas.

**Tech Stack:** ESP-IDF/C/C++, Kotlin Multiplatform, Compose, Gradle, Room al incorporar datos, HTTPS/JSON y proveedores opcionales. Versiones fijadas tras compilación mínima, no asumidas como verificadas.

**Spec:** [diseño aprobado, revisión 1.0](../specs/2026-11-17-diseno-software.md). Leerlo junto con este plan.

**Estado del plan:** preparado el 17 de noviembre de 2026. Ninguna tarea de producto está ejecutada. Las firmas y rutas siguientes son decisiones para implementar, no APIs existentes.

## Global Constraints

- «Funcionamiento básico sin internet, sin saldo de IA y sin computador.»
- «Entrega máxima el 16 de diciembre de 2026.»
- Android y Windows son destinos iniciales; no se compromete app Linux completa.
- «Las claves privadas de cada dispositivo se generan localmente, no se distribuyen en Git.»
- «La validación TLS no se desactiva para hacer funcionar una demostración.»
- «Todo relevo abrupto sin cierre contable confirmado deja pausadas las nuevas llamadas pagadas».
- Permiso inicial de 6.000 ms, renovación cada 2.000 ms, reloj monotónico de placa.
- «La prueba de cinco minutos sin red requiere retener al menos 300 muestras periódicas, además de los eventos funcionales.» Buffer propuesto: 600 muestras, separado de eventos relevantes.
- Captura bajo demanda, límite de 15 segundos; INMP441 solo para intensidad local.
- No claves/API en placa, audio persistente, llamadas pagadas automáticas en tests ni material académico íntegro en Git.
- OLED SH1106 y pines por validar; sonidos con MAX98357A, sin zumbador ni vibración.

## Review Focus

1. Un reintento cuya respuesta salió de caché no vuelve a ejecutar una acción: `evictedRetryIsStale` en T01 y prueba contra placa en T03.
2. Una app con reloj civil cambiado no prolonga el control ni el estudio: `wallClockDoesNotChangeLeaseOrSession` en T01/T04.
3. Una captura delegada pierde su centro mientras Android sigue conectado a la placa: `handoffCancelsDelegatedCapture` en T08.
4. Una solicitud pagada no visible en la copia del nuevo centro obliga a pausar IA: `uncleanHandoffPausesEvenWithEmptyReplica` en T07.
5. Borrar un dato antes de un corte no debe hacerlo reaparecer al sincronizar: `deletedNoteDoesNotResurrect` en T05.

---

## Entregas y dependencias

| Entrega | Objetivo de calendario | Tareas | Resultado observable |
|---|---|---|---|
| E0 | 17–23 nov | T00, T00-V | Entornos mínimos reproducibles; riesgos de voz medidos |
| E1 | 17–23 nov, completar al disponer de placa | T01 → T02 → T03 | Una expresión solicitada desde Android/Windows, aplicada y confirmada por HTTPS |
| E2 | 24–30 nov | T04, T05, T06; iniciar T07 | Sesión, sensores, sonidos, historial y relevo integrado |
| E3 | 1–7 dic | T07, T08, T09 | Texto/voz, herramientas, gasto y avisos de agente |
| E4 | 8–16 dic | T10 | Validación, correcciones, ensayo y entrega |

Camino crítico: T00 → T01 → T02 → T03 → T04/T05 → T06 → T07/T08 → T10. T00-V empieza tras el arranque mínimo y avanza en paralelo al protocolo. T09 puede avanzar tras T03/T06 usando eventos simulados. A/B/C son los roles de la guía; asignar nombres en la primera reunión.

Las fechas son objetivos de integración, no una garantía de capacidad. Si el 23 de noviembre falla viabilidad o el 30 no existe recorrido físico, revisar el alcance con el docente; no ocultar el retraso ni abrir ampliaciones. El 13 de diciembre se congela funcionalidad.

## Mapa de archivos y módulos

Una raíz Gradle: `:shared` en `apps/shared`, `:androidApp` en `apps/androidApp`, `:desktopApp` en `apps/desktopApp`, `:simulator` en `simulator`. Shared contiene lógica e interfaz común; las dos apps son entradas pequeñas con servicios nativos. El simulador usa lógica/contrato de shared, sin abrir interfaz.

Para abreviar las listas, se definen estas rutas exactas:

- **S** = `apps/shared/src/commonMain/kotlin/pet/`.
- **ST** = `apps/shared/src/commonTest/kotlin/pet/`.
- **A** = `apps/androidApp/src/main/kotlin/pet/`.
- **D** = `apps/desktopApp/src/main/kotlin/pet/`.
- **F** = `firmware/main/`.

En cada tarea, `S/protocol/Protocol.kt` significa la ruta completa resultante; no es una carpeta llamada S. Los tests `ST` se ejecutarán mediante el destino compartido `desktopTest`. Pruebas de permisos/servicios se ejecutarán además en Android real.

Los comandos Gradle indicados son para PowerShell desde raíz. En Linux sustituir `.\gradlew.bat` por `./gradlew` para componentes compatibles. Los comandos ESP-IDF se ejecutan desde `firmware` con su entorno activado.

## T00 — Entorno mínimo y compilación común

**Responsable:** B, con A para firmware y C para revisión. **Depende:** diseño y revisión de este plan. **Produce:** base compilable para las siguientes tareas.

**Files:** crear `settings.gradle.kts`, `build.gradle.kts`, `gradle/libs.versions.toml`, Wrapper y archivos de módulos; `S/ui/PetApp.kt`, `A/MainActivity.kt`, `D/Main.kt`, `simulator/src/main/kotlin/pet/simulator/Main.kt`; `firmware/CMakeLists.txt`, `F/CMakeLists.txt`, `F/main.c`, `firmware/sdkconfig.defaults`; actualizar `docs/equipo/ENTORNOS.md`. Crear `.github/workflows/build.yml` cuando los comandos funcionen.

**Interfaces:** `PetApp()` muestra un estado local de bienvenida; entradas Android/Windows la abren. El simulador solo arranca e informa que aún no atiende el protocolo. Firmware mínimo identifica arranque y memoria por USB, sin pines de periféricos supuestos.

- [ ] **Paso 1:** inventariar versiones y dispositivos con la guía de entornos; registrar qué hay instalado y quién tiene acceso a hardware. Fijar minSdk/targetSdk y conjunto compatible de versiones con las herramientas oficiales; no usar versiones dinámicas.
- [ ] **Paso 2:** crear los proyectos mínimos y Wrapper, fijar versiones y target `esp32s3`. No añadir todavía Room, proveedores, cámara o audio. Reutilizar la plantilla oficial compatible, adaptando los nombres de módulos acordados.
- [ ] **Paso 3:** ejecutar `.\gradlew.bat :androidApp:assembleDebug :desktopApp:compileKotlin :simulator:test` e `idf.py build`. Esperado: todas las tareas terminan correctamente; los tests inexistentes no se reportan como cobertura funcional.
- [ ] **Paso 4:** abrir `:desktopApp:run` en Windows e instalar APK en el INFINIX. Registrar versión Android real y foto/captura de ambas pantallas. Con placa disponible, flashear el ejemplo mínimo y registrar arranque; sin placa, declarar esa comprobación pendiente.
- [ ] **Paso 5:** fijar en CI los mismos comandos de compilación ya probados, sin secretos de ejecución, más un trabajo de compilación firmware con la versión fijada. Registrar el workflow como no ejecutado hasta observarlo en GitHub; no inventar un resultado local equivalente.
- [ ] **Paso 6:** revisar diferencias y hacer commit `build: iniciar proyectos Android Windows y ESP32`. No incluir SDKs, binarios, credenciales o directorios de build.

**Aceptación:** aplicaciones abren en Android/Windows, firmware compila para S3 y otra persona reproduce el build. Una instalación no comprobada no se marca lista.

## T00-V — Comprobar riesgos de voz temprano

**Responsable:** B/C. **Depende:** apps mínimas de T00; no espera el protocolo completo.

**Files:** crear `A/diagnostics/MicrophoneProbe.kt`, `D/diagnostics/MicrophoneProbe.kt` y `docs/pruebas/viabilidad-voz.md`. Acceso solo en compilación de depuración; no convertirlo en pantalla del producto final.

**Interfaces:** `runProbe(): ProbeResult` registra capacidad de iniciar/cancelar captura, motivo y latencia, nunca audio o texto dictado. La salida distingue Android/Windows y estado visible/segundo plano.

- [ ] **Paso 1:** preparar tabla de pruebas: micrófono libre, permiso denegado, videollamada ya activa, otra grabación que empieza durante la captura, cambio/desconexión de dispositivo y pantalla bloqueada Android.
- [ ] **Paso 2:** implementar captura mínima bajo acción explícita, solo en memoria, liberación y observación de conflictos con APIs documentadas. Sin proveedores ni modo exclusivo de Windows. No sustituir detección por la sola lectura del registro.
- [ ] **Paso 3:** ejecutar cada escenario en Windows y el INFINIX, registrar lo observado y la API usada. La ausencia de conflicto en una aplicación no demuestra compatibilidad universal. Probar especialmente inicio de captura solicitado desde segundo plano.
- [ ] **Paso 4:** presentar limitaciones antes del 23 de noviembre; conservar texto como alternativa. Si la política de la v3 no puede cumplirse, registrar ajuste con equipo/docente antes de T08. No declarar P09/P10 aprobadas por una prueba de apertura básica.
- [ ] **Paso 5:** commit `test: registrar viabilidad de captura en equipos reales`. Reutilizar solo código útil en T08; retirar el diagnóstico del artefacto de entrega.

**Aceptación:** matriz real que permite acotar T08. Un resultado negativo documentado es útil; no se oculta como un test omitido.

## T01 — Contrato y autoridad simulada comprobables

**Responsable:** C con A/B como consumidores. **Depende:** T00. **Entrega:** estado, permiso y expresión en lógica pura.

**Files:** `S/protocol/Protocol.kt`, `S/protocol/ProtocolCodec.kt`, `S/control/DeviceAuthority.kt`, `ST/protocol/ProtocolTest.kt`, `ST/control/DeviceAuthorityTest.kt`; ampliar `contracts/` solo de forma coherente.

**Interfaces:** `ProtocolCodec.decodeCommand(json: String): Command`; tipos `Command`, `ControlRequest`, `Principal`, `DeviceState`, `Reply` según contrato. `DeviceAuthority(clock: MonotonicClock)` ofrece `suspend fun handle(request: Request, principal: Principal): Reply` y `suspend fun snapshot(): DeviceState`. `MonotonicClock.nowMs(): Long` se inyecta para tests. Serializar aplicación de efectos y secuencias; no usar hora civil.

- [ ] **Paso 1:** escribir tests de todos los casos de aceptación del [contrato inicial](../../../contracts/PROTOCOLO_INICIAL.md), incluidos `concurrentAcquireHasOneOwner`, `evictedRetryIsStale` y `wallClockDoesNotChangeLeaseOrSession` (en esta tarea, parte del permiso).
- [ ] **Paso 2:** ejecutar `.\gradlew.bat :shared:desktopTest`. Esperado inicial: fallos por lógica no implementada; si falla la configuración, corregir T00 antes de interpretar el resultado como test funcional.
- [ ] **Paso 3:** implementar parsing estricto, respuestas, reloj inyectado y autoridad. Usar esquema como referencia; autenticación real se incorpora en T02/T03. Un `Principal` de test nunca equivale a una credencial aceptable en red.
- [ ] **Paso 4:** repetir tests. Afirmaciones mínimas: a `now=6000` un permiso inicial venció; renovar a 2000 vence a 8000; repetir no produce un segundo efecto; duración 99/5001/string/bool se rechaza; tras 65 respuestas la operación expulsada no se aplica otra vez.
- [ ] **Paso 5:** revisar ejemplos con A y B; commit `feat: definir autoridad y contrato inicial`.

**Aceptación:** casos negativos sin efectos y exactamente un centro; contrato entendido por firmware y apps.

## T02 — Identidad, primer emparejamiento y almacenamiento seguro

**Responsable:** A/B; C revisa límites de confianza. **Depende:** T01.

**Files:** `F/provisioning.c/.h`, `F/credentials.c/.h`, `F/api.c/.h` (servidor mínimo de emparejamiento, ampliado por T03); `S/pairing/PairingModels.kt`, `S/pairing/PairingClient.kt`, `S/security/SecretStore.kt`, `D/provisioning/UsbProvisioner.kt`, `D/security/WindowsSecretStore.kt`, `A/security/AndroidSecretStore.kt`, `A/pairing/QrScanner.kt`; `ST/pairing/PairingTest.kt`; `contracts/EMPAREJAMIENTO.md`.

**Interfaces:** `PairingOffer(deviceId, certificateFingerprint, oneTimeSecret, expiresAt)` se obtiene por USB/canal autorizado; no por anuncio mDNS. `SecretStore.put(alias: String, value: ByteArray)`, `get(alias): ByteArray?`, `delete(alias)`. `suspend pair(offer: PairingOffer): PairedDevice` valida identidad y confirmación antes de registrar credencial.

- [ ] **Paso 1:** fijar en el contrato de emparejamiento el flujo inicial y la ruta `POST /v1/pair`, caducidad de 60 segundos del secreto, consumo de un solo uso, revocación y recuperación física. El secreto de alta autentica exclusivamente esa ruta; no da acceso a comandos. La fecha civil mostrada no sustituye la caducidad monotónica que valida quien emite el secreto.
- [ ] **Paso 2:** escribir `expiredOfferIsRejected`, `offerCannotBeReused`, `wrongFingerprintNeverSendsCredentials`, `revokedClientCannotReadState` y `noTouchDoesNotPair`; ejecutar y observar fallos por implementación faltante.
- [ ] **Paso 3:** implementar aprovisionamiento USB, generación local de claves y emparejamiento TLS, utilizando bibliotecas de criptografía/QR revisadas. El modo físico inicial se activa deliberadamente durante arranque con el sensor, y no por una orden remota sin autenticar. Permiso Android de cámara solo al escanear.
- [ ] **Paso 4:** implementar stores de plataforma. Android protege una clave de cifrado mediante Keystore; Windows usa almacenamiento protegido del sistema. Ningún secreto viaja al log, esquema JSON de ejemplo o NVS como clave de proveedor.
- [ ] **Paso 5:** ejecutar tests compartidos y repetir emparejamiento, revocación y huella incorrecta en plataformas. Falta de sensor puede cubrirse en simulación, pero deja la confirmación física pendiente. Registrar `P04/P19` parciales.
- [ ] **Paso 6:** commit `feat: establecer confianza y credenciales por dispositivo`.

**Aceptación:** primera app confiable, segundo cliente incorporable, cliente ajeno rechazado y credencial revocable sin desactivar TLS.

## T03 — Primera orden completa: app → HTTPS → expresión → confirmación

**Responsable:** A/B/C. **Depende:** T01/T02.

**Files:** `S/device/DeviceClient.kt`, `S/ui/ConnectionScreen.kt`, `S/ui/DeviceScreen.kt`; `simulator/src/main/kotlin/pet/simulator/HttpsDeviceServer.kt`, `simulator/src/test/kotlin/pet/simulator/ConformanceTest.kt`; `F/api.c/.h`, `F/authority.c/.h`, `F/display.c/.h`; `docs/hardware/PINOUT.md` y registro de E1.

**Interfaces:** `DeviceClient.state(): DeviceState`, `acquire(): Reply`, `renew(): Reply`, `release(): Reply`, `setExpression(expression: Expression, durationMs: Int): Reply`, todas `suspend`. Servidor/simulador implementan rutas del contrato. Configuración de cliente consume identidad/credencial de T02.

- [ ] **Paso 1:** escribir pruebas de cliente contra servidor HTTPS de test: orden válida, certificado equivocado, respuesta perdida con reintento, corte, JSON inválido y error no reconocido. Esperado: fallo controlado, nunca éxito supuesto.
- [ ] **Paso 2:** implementar servidor simulado y cliente con TLS validado. Generar certificados temporales en test; no guardarlos en Git. Ejecutar `.\gradlew.bat :shared:desktopTest :simulator:test`.
- [ ] **Paso 3:** A verifica pantalla/controlador y pines, registra el mapa y adapta API/autoridad al ESP32. Usar biblioteca gráfica compatible y atribuida; mantener dibujo no bloqueante. No deducir SH1106 únicamente del nombre comercial.
- [ ] **Paso 4:** conectar Android, adquirir control, pedir `happy` durante 1000 ms y verificar confirmación y retorno a `idle`. Liberar, repetir desde Windows; todavía no hay traspaso automático. Guardar evidencia simulada y física por separado.
- [ ] **Paso 5:** añadir tarea `:simulator:conformance` que ejecute casos de contrato contra un destino indicado explícitamente por configuración local (URL, identidad y credencial de test). Sin destino, debe fallar explicando la falta de configuración, no aprobar silenciosamente. Probar contra placa `evictedRetryIsStale`, cliente ajeno, reinicio y época antigua.
- [ ] **Paso 6:** compilar firmware, repetir pruebas y medir memoria/TLS con animación. Medir conexión inicial aparte de confirmación sobre enlace ya abierto. Commit `feat: integrar expresiones con control autenticado`.

**Aceptación:** un recorrido real de extremo a extremo, apps muestran estado confirmado y ninguna orden inválida aplica. La demo simulada no cierra la parte física de E1.

## T04 — Mascota autónoma, estudio, gestos y sonidos

**Responsable:** A; B integra controles; C revisa estados. **Depende:** T03.

**Files:** `F/pet_state.c/.h`, `F/study_session.c/.h`, `F/sensors.c/.h`, `F/audio.c/.h`; aplicación de prueba Unity en `firmware/test/CMakeLists.txt`, `firmware/test/main/CMakeLists.txt` y `firmware/test/main/test_pet_state.c`; `S/study/StudyController.kt`, `S/ui/StudyScreen.kt`, `ST/study/StudyTest.kt`; `contracts/ESTUDIO_Y_EVENTOS.md`. Modificar `F/api.c`, `S/device/DeviceClient.kt` y el simulador para las nuevas operaciones.

**Interfaces:** comandos `study.start(duration_ms)`, `study.pause`, `study.resume`, `study.finish`, `config.set`, `alert.set`; `StudyState(sessionId, phase, elapsedMs, targetMs)`. Fases `idle/running/paused/finished/interrupted`. `Confirmation(id, operationId, leaseEpoch, expiresInMs)` de 15.000 ms; estados `pending/applied/cancelled`.

- [ ] **Paso 1:** concretar rangos: estudio 60.000–7.200.000 ms; intensidad relativa normalizada 0–1, con umbral válido `(0,1]`, histéresis inicial 0,05 y configuración rechazada si produce límites inválidos. Calibrar captura antes de habilitar reacción; no etiquetar valores como dB.
- [ ] **Paso 2:** tests de inicio/pausa/doble toque, reloj civil cambiado, expiración de confirmación, relevo durante confirmación y toque sostenido que no inicia a la vez estudio y voz. Comprobar `wallClockDoesNotChangeLeaseOrSession` en el temporizador. Ejecutar tests y observar los fallos relevantes.
- [ ] **Paso 3:** implementar temporizador monotónico, estados y prioridades. Toque breve contextual inicia la duración configurada; doble toque pausa/reanuda; mantener solicita voz solo cuando no hay confirmación pendiente. Fijar ventanas de gestos tras calibración y registrarlas. Un gesto que abre un modo no puede confirmarlo hasta soltar y tocar de nuevo.
- [ ] **Paso 4:** implementar INMP441, LED, TTP223 y sonidos PCM breves desde flash con dos canales I2S separados. Reproducción inhibe reacciones acústicas propias y después espera inicialmente 200 ms; medir y ajustar sin detener temporizador. Volumen inicial moderado y silencio configurables.
- [ ] **Paso 5:** buffer de 600 muestras a 1 Hz más eventos funcionales separados; overflow visible. Persistir configuración al cambiar y una marca de sesión activa al iniciar/cerrar, no telemetría cada segundo. Reinicio con marca activa produce estado de sesión interrumpida, sin inventar duración tras apagado.
- [ ] **Paso 6:** ejecutar `.\gradlew.bat :shared:desktopTest`. Desde `firmware`, preparar target S3 del proyecto `test`, ejecutar `idf.py -C test build` y flashear/monitorizar esa app Unity usando el puerto confirmado de la placa. Ejecutar sus casos y exigir cero fallos; después volver a flashear el firmware de producto y probar corte de red con sesión activa. Documentar P01–P03/P12/P15 parciales y A01 de sonido. Si falta amplificador, registrar la parte pendiente; no sustituirla por voz del PC como si fuera el parlante.
- [ ] **Paso 7:** commit `feat: incorporar sesiones y comportamiento autonomo`.

**Aceptación:** sesión continúa sin red ni IA, audio no provoca bucle de alarmas y gestos/confirmaciones producen un solo efecto.

## T05 — Datos locales, acceso, historial y sincronización incremental

**Responsable:** B con C. **Depende:** T04.

**Files:** `S/data/PetDatabase.kt`, `S/data/Entities.kt`, `S/sync/SyncService.kt`, `S/auth/LocalLogin.kt`, `S/ui/HistoryScreen.kt`, `S/ui/SettingsScreen.kt`, `ST/sync/SyncTest.kt`, `ST/auth/LocalLoginTest.kt`; `contracts/SINCRONIZACION.md`; adaptadores Room de plataforma en `apps/shared/src/androidMain/` y `apps/shared/src/desktopMain/`. Modificar API de firmware, cliente y simulador para `GET /v1/events` y sus cursores.

**Interfaces:** `EventId(bootId, sequence)`, `SyncCursor(bootId, lastSequence)`, `EventPage(events, nextCursor, gap)`, `suspend syncAfter(cursor: SyncCursor?): EventPage`. Tablas: eventos, sesiones, configuración/versiones, notas, recordatorios y marcas de borrado. `LocalLogin.verify(password: CharArray): Boolean` usa verificador PBKDF2 con sal; no guarda contraseña recuperable.

- [ ] **Paso 1:** fijar paginación máxima de 32 eventos, cursor por arranque, versión de configuración y resolución por centro. Borrado deja marca replicable; acompañante offline solo crea borradores. Contrato devuelve `gap` si ya no conserva el cursor solicitado.
- [ ] **Paso 2:** tests `replayedPageDoesNotDuplicate`, `deletedNoteDoesNotResurrect`, `bufferGapIsVisible`, `newBootDoesNotReuseCursor` y `wrongPasswordIsRejected`. Inicialmente deben fallar por implementación ausente.
- [ ] **Paso 3:** añadir Room/SQLite/KSP con versiones compatibles verificadas en ambas plataformas, migraciones y esquema versionado. Aplicar páginas y cursor en una misma transacción. No copiar archivos de base SQLite entre dispositivos.
- [ ] **Paso 4:** implementar login, alta local de contraseña, almacenamiento de verificador y controles de historial/configuración/borrado. Fijar parámetros de derivación tras medir coste en los dispositivos; documentar valores y evitar implementaciones criptográficas propias.
- [ ] **Paso 5:** ejecutar tests compartidos, pruebas de persistencia en Android y Windows, cerrar/reabrir apps y simular caída entre escritura y confirmación del cursor. Cortar cinco minutos la red: recuperar al menos 300 muestras sin duplicados, o mostrar brecha real si hubo overflow/reinicio.
- [ ] **Paso 6:** commit `feat: persistir y sincronizar historial local`.

**Aceptación:** datos sobreviven reinicio de app, reintentos no duplican y borrados no reaparecen. No se promete retener telemetría RAM tras pérdida de alimentación de placa.

## T06 — Descubrimiento, relevo de centro y Android en segundo plano

**Responsable:** B/A; C prueba fallas. **Depende:** T05.

**Files:** `S/control/Coordinator.kt`, `S/sync/TransferBundle.kt`, `A/service/DeviceService.kt`, `A/network/Discovery.kt`, `D/network/Discovery.kt`, `ST/control/HandoffTest.kt`; extender `F/authority.c` y `contracts/CONTROL_Y_TRASPASO.md`.

**Interfaces:** `Coordinator.mode` = `autonomous/primary/companion`; `suspend requestHandoff(targetClientId: String): HandoffResult`. `TransferBundle` incluye revisión de configuración, cursores/eventos confirmados, revisión contable y cierre contable confirmado. Hasta T07 el registro contable solo puede estar vacío porque aún no se habilitan llamadas reales.

- [ ] **Paso 1:** especificar solicitud/aceptación/confirmación del relevo: el centro deja de originar cambios, sincroniza, la placa cambia propietario/época atómicamente y ambas apps consultan el resultado. El modo manual que fija Android impide toma preferente de Windows.
- [ ] **Paso 2:** escribir `handoffHasOneOwner`, `stalePrimaryCannotApply`, `androidPinnedRejectsDesktopTakeover`, `suspendDesktopRecoversMobile` y `companionNeverExecutesDirectly`. Probar paquetes duplicados y pérdida de cada mensaje del traspaso.
- [ ] **Paso 3:** implementar mDNS/NSD con identidad verificada y dirección manual alternativa. La presencia de un nombre en red no concede confianza. Implementar servicio Android con permisos/tipos justificados y notificación de estado.
- [ ] **Paso 4:** tras salida abrupta, recuperar control por vencimiento y marcar `accountingUncertain=true` aun con copia vacía. Si la app vieja regresa, funciona como acompañante hasta adquirir otra autorización; respuestas tardías no producen herramientas.
- [ ] **Paso 5:** tests y demostración de apertura/cierre/suspensión de Windows, partición entre apps con placa alcanzable y teléfono bloqueado 30 min. Probar hotspot del INFINIX y router. Registrar P06/P07/P18; respetar restricciones del sistema sin desactivar protecciones globales.
- [ ] **Paso 6:** commit `feat: coordinar centros y continuidad Android`.

**Aceptación:** siempre un centro, recuperación móvil observable y limitaciones de segundo plano registradas. La pausa contable no bloquea control ni temporizador local.

## T07 — Asistente por texto, herramientas y presupuesto

**Responsable:** C; B interfaz/datos y A confirmaciones. **Depende:** T05/T06.

**Files:** `S/assistant/AssistantService.kt`, `S/assistant/ToolRouter.kt`, `S/providers/ProviderClient.kt`, `S/budget/BudgetLedger.kt`, `S/ui/ChatScreen.kt`, `S/ui/ProviderScreen.kt`, `ST/assistant/AssistantTest.kt`, `ST/budget/BudgetTest.kt`; ampliar Room y `contracts/HERRAMIENTAS_Y_GASTO.md`.

**Interfaces:** `suspend ProviderClient.complete(request: ModelRequest): ModelReply`; `ToolRouter.validate(call: ToolCall): ValidatedTool`; `suspend AssistantService.send(text: String): AssistantResult`. Herramientas: sesiones, recordatorios, notas, historial, estado de agentes, umbral y avisos. `MoneyMicros(amount: Long, currency: String)` evita flotantes para registro; `BudgetLedger.reserve(id, upperBound)`, `settle(id, actual)`, `markUncertain(id)` y `pauseForUncleanHandoff()` son transacciones persistentes.

- [ ] **Paso 1:** especificar tipos/rangos de herramientas y confirmaciones siguiendo la tabla de la v3. Estudio/notas/recordatorios/consultas no requieren toque; umbral y avisos sí. Entradas al modelo nunca habilitan un intérprete del sistema.
- [ ] **Paso 2:** tests con proveedor simulado: JSON inválido, herramienta desconocida, parámetros inválidos, caída/timeout, respuesta tardía tras relevo y reintento de herramienta. Afirmar cero efectos ante inválidos y uno ante duplicados válidos.
- [ ] **Paso 3:** implementar un adaptador compatible con formato de chat, perfiles con URL/modelo/capacidades y prueba de conexión. Dos perfiles simulados prueban respaldo. El usuario habilita proveedor real con clave local; no incorporarla a fixtures.
- [ ] **Paso 4:** tests de presupuesto: límite 10.000 unidades, reserva 7.000 y segunda 4.000 → segunda rechazada; timeout conserva reserva; gasto real liquida sin duplicar; `uncleanHandoffPausesEvenWithEmptyReplica` bloquea nuevas llamadas con historial aparentemente vacío. `settleAboveReservePauses` bloquea gasto nuevo si el real excede lo reservado. Tarifas ausentes/estimación no acotable bloquean consumo. Usar una moneda de cuenta USD y rechazar perfiles con otra moneda, sin conversión implícita.
- [ ] **Paso 5:** implementar reserva previa, serialización de solicitudes y sincronización de registro. Incluir transcripción futura en la misma política: su estimación debe usar duración máxima y la unidad tarifaria del proveedor, no solo tokens. Un presupuesto desconocido nunca equivale a ilimitado.
- [ ] **Paso 6:** crear chat/configuración y fallback local. Sin API, los controles manuales siguen funcionando y el chat explica su limitación. Al cerrar estudio, el comentario del personaje es opcional y no bloquea guardar la sesión. La política de proveedor y límites por hora se configuran explícitamente antes de activarse; no hay llamadas automáticas por defecto. Los recordatorios conservan instante/zona y un ID único de ejecución: probar relevo y cambio horario sin duplicarlos; una hora ambigua solicita aclaración. Borrar detalles de historial no reinicia el total consumido ni las reservas del periodo; informar la conservación del agregado mínimo necesario para el límite.
- [ ] **Paso 7:** pruebas compartidas y prueba manual pagada solo con proveedor/presupuesto habilitados por el equipo. Verificar cambio de perfil sin recompilar, fallo del preferido y recuperación local. Registrar P11/P13/P14/P19 con alcance real/simulado diferenciado.
- [ ] **Paso 8:** commit `feat: integrar asistente y control de consumo`.

**Aceptación:** mismas herramientas seguras desde texto, ningún gasto sin autorización/configuración y ninguna repetición del efecto por respaldo entre proveedores.

## T08 — Voz bajo demanda, delegación y cancelación

**Responsable:** B/C y A para indicador/gestos. **Depende:** T00-V, T04, T06, T07.

**Files:** `S/voice/CaptureCoordinator.kt`, `S/voice/AudioClip.kt`, `S/voice/Transcriber.kt`, `A/voice/AndroidCapture.kt`, `D/voice/WindowsCapture.kt`, `D/voice/GlobalShortcut.kt`, `ST/voice/CaptureTest.kt`; `contracts/VOZ.md` y evidencia real de P08–P10.

**Interfaces:** `CaptureGrant(id, leaseEpoch, sourceClientId, expiresInMs)`; `start(grant)`, `stop(grantId)`, `cancel(grantId, reason)`. `AudioClip` usa PCM16 mono a 16 kHz, máximo 15 s (480.000 bytes PCM), memoria temporal; el adaptador lo empaqueta como WAV o formato admitido sin escribir disco. `suspend Transcriber.transcribe(clip: AudioClip): String` pasa por presupuesto antes de API.

- [ ] **Paso 1:** concretar transporte de audio apps↔apps HTTPS autenticado, límite 524.288 bytes para el contenedor, timeout y vigilancia de permiso. No enviar audio por la placa. Elegir un servicio de transcripción con formato/coste comprobables; el reconocedor Android es alternativa que también debe verificarse en el teléfono.
- [ ] **Paso 2:** tests `noIndicatorAckNoCapture`, `releaseEndsCapture`, `deniedPermissionOffersText`, `handoffCancelsDelegatedCapture`, `clipLimitCancels`, `lateTranscriptHasNoEffect` y conflicto que descarta audio. Probar simultaneidad de dos solicitudes: solo una captura autorizada.
- [ ] **Paso 3:** implementar botón, mantener sensor y atajo global configurable; no secuestrar un atajo usado por otra aplicación. Windows utiliza captura compartida; la disponibilidad incierta bloquea inicio. Android aplica el flujo permitido por su ciclo de vida, con indicador y sin escucha continua.
- [ ] **Paso 4:** Windows puede autorizar captura Android acompañante. Se vincula a época/solicitud, reconoce indicador físico antes de empezar y cancela ante relevo/pérdida de enlace. Las autorizaciones de captura necesitan vigilancia durante los 15 s, no un permiso estático que sobreviva a la pérdida del centro.
- [ ] **Paso 5:** reutilizar validación/herramientas de texto; no volver a ejecutar una transcripción al reconectar. Transcribir solo después de cerrar la captura y validar contexto; descartar el audio al cancelar o completar.
- [ ] **Paso 6:** tests y matriz real con llamadas antes/durante, Android bloqueado, segundo teléfono, webcam desactivada, red cortada y micrófono desconectado. Medir cancelación; si no cumple el objetivo docente, declarar la limitación y el ajuste acordado, sin rebajar silenciosamente P09/P10.
- [ ] **Paso 7:** commit `feat: incorporar voz bajo demanda y delegacion`.

**Aceptación:** dictado directo/delegado sin duplicar herramientas, captura visible y cancelable; comportamiento real de los dispositivos documentado. No se entrega voz sintetizada como parte de esta tarea.

## T09 — Eventos de Claude Code

**Responsable:** C; A/B integración visual. **Depende:** T03 y T06 para integración completa.

**Files:** `D/agents/AgentEventServer.kt`, `S/agents/AgentEvent.kt`, `ST/agents/AgentEventTest.kt`, `integrations/claude-code/notify.ps1`, `integrations/claude-code/README.md`; configuración de ejemplo sin tokens reales.

**Interfaces:** `AgentEvent(eventId, sessionId, kind)` donde `kind=turn_finished/permission_requested`; receptor de escritorio `POST /agent-events` solo en loopback, con token local y máximo 2.048 bytes. El adaptador conserva solo esos datos; no copia prompt, herramientas, código o rutas privadas.

- [ ] **Paso 1:** comprobar eventos `Stop` y `PermissionRequest` en la versión instalada de Claude Code; fijar su mapeo y límites en el README del adaptador. No asumir que fin de turno significa tarea correcta.
- [ ] **Paso 2:** tests de autenticación ausente, evento inválido, duplicado, app caída y ráfaga. Un evento duplicado no reinicia indefinidamente la expresión; el fallo del receptor no bloquea ni aprueba acciones del agente.
- [ ] **Paso 3:** implementar receptor loopback y adaptador PowerShell que lea entrada, reduzca campos y avise con timeout breve. No devolver decisiones de aprobación de permisos. Guardar el token fuera del repo y no registrarlo. No instalar hooks globales automáticamente.
- [ ] **Paso 4:** habilitarlo en un proyecto de prueba elegido por el integrante, según guía; probar una tarea y una solicitud de permiso reales en Windows. El aviso pasa por el centro y vuelve al estado anterior con caducidad.
- [ ] **Paso 5:** ejecutar tests compartidos/escritorio y registrar P17; commit `feat: mostrar eventos de Claude Code`.

**Aceptación:** eventos verificables, sin exposición de transcripciones ni modificación del mecanismo de permisos. Notificaciones de Codex y MCP siguen fuera de esta base.

## T10 — Validación completa y entrega reproducible

**Responsable:** todos; integrador rotativo. **Depende:** base integrada; empieza a recopilar evidencia desde E1.

**Files:** resultados bajo `docs/pruebas/resultados/`, actualizar [matriz](../../pruebas/MATRIZ.md), [guion](../../entrega/GUION.md), README y registro de versiones; generar artefactos de entrega fuera de Git cuando contengan binarios o datos sensibles.

**Interfaces:** cada resultado sigue [plantilla](../../pruebas/PLANTILLA_RESULTADO.md), identifica commit, dispositivos, estado y evidencia. No hay nueva interfaz de producto.

- [ ] **Paso 1:** ejecutar P01–P19 y A01–A04 de la matriz, incluyendo cortes, reinicios, dos Android, suspensión Windows, gasto incierto, borrado y privacidad. Marcar lo no probado con motivo.
- [ ] **Paso 2:** corregir fallos mediante tareas pequeñas con test de regresión pertinente. No ampliar cámara/MCP/voz sintetizada mientras falte base. Contrastar los 29 indicadores con la rúbrica original disponible para el equipo y registrar los ajustes académicos aprobados.
- [ ] **Paso 3:** desde un clon limpio, instalar las versiones documentadas y ejecutar compilaciones y tests. Preparar APK, aplicación Windows y firmware identificados con commit, instrucciones de instalación y licencias. Probar instalación; no confundir build con instalación validada.
- [ ] **Paso 4:** ensayo del guion con los tres explicando alguna parte ajena a su rol principal. Medir consumo con instrumentos adecuados y anotar configuración, no estimarlo desde software como si fuera una lectura física.
- [ ] **Paso 5:** congelar funcionalidad el 13 de diciembre; del 14 al 16, corregir solo lo necesario y entregar informe/evidencias/repositorio con limitaciones honestas. Publicación o envío externo se realiza según el encargo explícito del equipo.
- [ ] **Paso 6:** commit `docs: registrar validacion y entrega del prototipo` y etiqueta de versión cuando el equipo declare la entrega lista.

**Aceptación:** repositorio reproducible, artefactos identificados, evidencias accesibles sin secretos y ausencia de afirmaciones de pruebas no ejecutadas.

## Revisión de cobertura del plan

| Diseño | Tareas que lo cubren |
|---|---|
| Arquitectura y entornos | T00, T01, T03 |
| Identidad, acceso, seguridad | T02, T03, T05, T10 |
| Hardware, comportamiento autónomo, sonido | T03, T04 |
| Control, sincronización, continuidad | T01, T05, T06 |
| Texto, herramientas y gasto | T07 |
| Voz y alternativa móvil | T00-V, T08 |
| Eventos de agentes | T09 |
| Pruebas académicas, colaboración y entrega | Todas; consolidación T10 |

Las funciones ampliadas no tienen tareas de implementación por decisión de alcance. No se omiten requisitos base por falta de componentes: se dejan pendientes con responsable y se revisa su impacto en fecha.

## Inicio recomendado

Revisar este plan y empezar por T00 con un integrante responsable, conservando T00-V como comprobación temprana. El agente ejecutor trabaja tarea por tarea; otro integrante revisa el resultado. No es necesario iniciar tres agentes sobre la configuración común simultáneamente. Tras integrar T01, firmware y clientes pueden avanzar en archivos separados contra el contrato.
