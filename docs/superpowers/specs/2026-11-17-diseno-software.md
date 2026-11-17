# Asistente IoT de escritorio: diseño del software y organización del desarrollo

**Equipo:** Walther, Maximiliano y Winderson.  
**Fecha:** 17 de noviembre de 2026. **Revisión:** 1.0 aprobada.  
**Entrega objetivo:** 16 de diciembre de 2026.  
**Estado:** diseño inicial aprobado expresamente por Walther el 17 de noviembre de 2026, incluida la pausa de llamadas pagadas tras un relevo abrupto. Las expresiones «propuesta» y «para revisión» del cuerpo conservan el contexto de redacción; sus decisiones iniciales están aprobadas. Todavía no constituye software implementado ni validación del hardware.  
**Carpeta de trabajo acordada:** `Proyecto_pet_agentico_IOT`.  
**Documento de referencia:** `propuesta_asistente_iot_v3.pdf`, versión 3.0, 10 de noviembre de 2026, entregado por el docente.

## 1. Qué vamos a construir y para qué

Un personaje físico de escritorio que permita ver estados y recibir avisos sin depender de mirar continuamente una aplicación. Tendrá expresiones animadas, reaccionará al contacto y al sonido, acompañará sesiones de estudio, representará eventos de agentes de programación y permitirá solicitar tareas acotadas por texto o voz.

El ESP32 constituye el cuerpo y conserva las funciones locales. Las aplicaciones Android y Windows coordinan el dispositivo y proporcionan interfaz, historial y acceso opcional a servicios de IA. Android es el centro por defecto; Windows puede asumir el control cuando está disponible y emparejado. La utilidad del objeto se evaluará con demostraciones y pruebas; no se presupone una mejora de la concentración.

El objetivo de esta preparación es que los tres integrantes puedan trabajar con Claude y Codex desde un mismo repositorio, sin depender del historial de un chat. La primera meta es una función completa y pequeña que conecte aplicación y placa; después se amplía manteniendo esa integración funcionando.

## 2. Acuerdos, propuestas y evidencias disponibles

### Acuerdos expresos del equipo

- Tres integrantes, experiencia superficial en aplicaciones móviles y electrónica; mayor experiencia en pruebas y documentación.
- Disponibilidad indicada: cuatro horas semanales de clases y cinco horas adicionales durante el fin de semana. Se planificará con aproximadamente nueve horas compartidas por semana, sin asumir trabajo adicional garantizado.
- Entrega máxima el 16 de diciembre de 2026. No se dará por completada ninguna actividad del calendario anterior sin evidencia.
- Uso opcional de APIs de pago aceptado por recomendación docente. Las claves y el consumo de ejecución se administran aparte de las herramientas utilizadas para desarrollar.
- Funcionamiento básico sin internet, sin saldo de IA y sin computador.
- Desarrollo mediante entregas integradas desde el inicio, aceptado por el equipo el 17 de noviembre.
- Futuro repositorio compartido en GitHub; todavía no se ha elegido aquí su cuenta propietaria ni su visibilidad.
- Reproducción de sonidos breves en el prototipo base usando el parlante adquirido y un MAX98357A.

### Entornos conocidos

| Integrante | Computadores | Teléfono confirmado |
|---|---|---|
| Walther | Windows | INFINIX ZERO 30 5G, XOS 14.5.0 |
| Maximiliano | Windows | Pendiente de identificar |
| Winderson | Linux en laptop y Windows en computador | Pendiente de identificar |

XOS no determina por sí solo la versión de Android registrada para pruebas. Se comprobarán versión de Android, memoria disponible, permisos y restricciones de batería en el teléfono real. Hace falta identificar un segundo Android para la validación prevista por la v3.

### Decisiones propuestas en este documento, sujetas a su revisión

1. Un único repositorio con aplicaciones, firmware, contratos y documentación.
2. Windows como plataforma de escritorio prioritaria; Linux como entorno de desarrollo para las partes compatibles. La aplicación completa en Linux no es compromiso inicial.
3. Kotlin Multiplatform y Compose Multiplatform para las aplicaciones, conservando la arquitectura de la v3; C/C++ con ESP-IDF para el firmware.
4. Un simulador de la placa, construido junto con la primera integración y basado en el mismo contrato. No sustituye las pruebas físicas.
5. Aprovisionamiento inicial por USB como forma de establecer confianza con la primera aplicación; emparejamientos posteriores mediante la aplicación ya autorizada y confirmación física.
6. Ingreso separado de claves de proveedor en cada dispositivo en la primera entrega; transferencia automática de claves pospuesta.
7. Rotación de revisores y pruebas compartidas, sin tres proyectos aislados ni tres implementaciones del mismo protocolo.
8. Tras un relevo abrupto sin cierre contable confirmado, pausar nuevas llamadas pagadas hasta reconciliar el gasto; las funciones locales y el control móvil continúan. Esta limitación conservadora se somete expresamente a revisión.

Estas propuestas no se atribuyen al docente ni al equipo como acuerdos previos. Aprobar este documento fija estas decisiones, manteniendo explícitos los puntos de viabilidad pendientes.

## 3. Alternativas consideradas

| Alternativa | Ventaja | Coste o riesgo | Decisión |
|---|---|---|---|
| Entregas integradas, repositorio único | Cada avance puede probarse de extremo a extremo; detecta desacuerdos pronto | Exige acordar interfaces y revisar cambios compartidos | Elegida por el equipo; se propone un repositorio único para materializarla |
| Subsistemas completos e integración tardía | Mayor independencia al comenzar | Riesgo de descubrir incompatibilidades cerca del plazo | Descartada en la conversación |

La división por responsabilidades se mantiene, pero las entregas son del producto completo. Por ejemplo, «iniciar una sesión desde Android y verla en la placa» requiere colaboración de firmware, app y pruebas, aunque cada uno modifique archivos distintos.

## 4. Alcance y comportamiento sin servicios externos

### Base comprometida en la v3, más el sonido acordado

| Capacidad | Entrega esperada | Dependencia |
|---|---|---|
| Personaje autónomo | Ojos/expresiones, contacto, intensidad de sonido, LED, avisos y temporizador | Placa alimentada y periféricos montados |
| Sonidos breves | Inicio, aviso y fin de sesión con volumen y silencio configurables | MAX98357A y parlante; archivos breves en flash |
| Control desde apps | Emparejar, consultar estado, cambiar configuración y verificar su aplicación | Red local |
| Estudio | Iniciar, pausar, reanudar y cerrar; historial de eventos | Placa para sesión autónoma; app para historial |
| Dos centros posibles | Android por defecto, Windows preferente y recuperación al perderlo | Red local y credenciales válidas |
| Asistente por texto | Chat, herramientas acotadas y respuestas del personaje | IA opcional para interpretar lenguaje libre |
| Voz bajo demanda | Pulsación sostenida, botón y atajo de Windows; transcripción | Micrófono de teléfono/PC, permisos y servicio disponible |
| Proveedores | Configuración, capacidades, respaldo y control de gasto | API habilitada por el usuario |
| Agentes | Aviso de fin de turno y solicitud de permiso de Claude Code | Aplicación Windows e integración local |
| Robustez | Validación, reconexión, idempotencia, seguridad y evidencia | Pruebas automáticas y físicas |

Sin IA, las funciones se operan con controles explícitos de la interfaz y del sensor; el chat explica la indisponibilidad. No se promete comprender lenguaje libre sin un proveedor. Los recordatorios se programan en el centro activo; la placa conserva el temporizador de la sesión, no sustituye una agenda con reloj respaldado por batería.

Sin red local, la placa sigue reaccionando y contando una sesión mientras conserve alimentación. Sin alimentación no se garantiza continuidad del reloj: al reiniciar se registra una interrupción y no se inventa tiempo estudiado.

### Ampliado, solo después de aprobar la base

Cámara para pausa por ausencia, imágenes bajo demanda, respuesta hablada, acciones de computador en lista autorizada y servidor MCP para expresiones. Reproducir sonidos breves en la placa no incluye automáticamente sintetizar o transmitir voz al parlante.

La integración automática de notificaciones de Codex queda por evaluar aparte. Utilizar Codex para desarrollar no implica disponer ya de un adaptador de eventos para la mascota. La base académica identificada pide los hooks de Claude Code.

### Fuera de esta entrega

Escucha permanente, palabra de activación, modelos locales pesados, reconocimiento de identidad, grabación persistente de audio/video, motores de desplazamiento, vibración, batería interna, actualizaciones OTA y publicación en tiendas de aplicaciones. Tampoco se añade un servidor público, una cuenta en la nube propia o un broker público obligatorio.

Un cambio que quite una función base de la v3 se documentará y se someterá al equipo y al docente. Reordenar su implementación no equivale a eliminarla.

## 5. Hardware que condiciona el software

| Elemento | Estado conocido | Consecuencia para el desarrollo |
|---|---|---|
| ESP32-S3 SuperMini | Adquirida | Verificar memoria, GPIO y arranque del lote real antes de fijar pines |
| INMP441 | Adquirido | Entrada I2S para intensidad relativa; no captura de voz destinada a la IA |
| Parlante 3 W / 4 Ω | Adquirido | Requiere amplificador; no conectar directamente a GPIO |
| MAX98357A | Compra separada pendiente de confirmación en este registro | Salida I2S; desarrollar sonidos sin bloquear el resto si se retrasa |
| OLED blanca 1,3 pulgadas, 128 × 64 | Sustitución elegida para carrito Afel | Familia SH1106 según la ficha, pendiente de verificar al recibir; no asumir SSD1306 |
| TTP223, LED y componentes de montaje | Pedido MechatronicStore confirmado | Gestos, indicador de escucha y pruebas de montaje |
| Resto del carrito Afel | Preparado; compra no confirmada en este registro | No confundir carrito con material recibido |
| SX1308 | Adquirido, sin uso en la alimentación USB prevista | No necesita módulo de software |

La placa dispone de dos periféricos I2S; se propone separar adquisición del INMP441 y reproducción del MAX98357A. Se medirán memoria, buffers y fluidez con ambas funciones y HTTPS activos. [Documentación I2S de Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/i2s.html).

No se fija aquí el cableado. El mapa definitivo requiere inspeccionar serigrafía y evitar pines reservados o de arranque. Se limitará el volumen y se evitará que los avisos del propio parlante disparen continuamente la detección de ruido: durante la reproducción y un breve periodo posterior se inhibirán las reacciones acústicas, sin detener el resto del dispositivo.

## 6. Arquitectura propuesta

### 6.1 Firmware: autoridad sobre el estado físico

Responsabilidades: sensores, expresiones, temporizador, reproducción de sonidos, credenciales de clientes, permiso de control y registro temporal de eventos. Una única ruta de aplicación de comandos serializa los efectos; las tareas de dibujo, audio y red no modifican el estado de forma independiente.

Se propone ESP-IDF porque proporciona las primitivas de red, TLS y periféricos que exige este diseño. Es una decisión de implementación, no un requisito del profesor. Su coste es una curva inicial mayor que un sketch sencillo; se amortiza con un único ejemplo mínimo de arranque reproducible y tareas pequeñas.

No se integra un robot comunitario completo de entrada. Las primeras expresiones serán simples y propias. Una biblioteca gráfica compatible con el controlador real se seleccionará revisando su licencia; la elección no cambia el contrato de expresiones con las apps.

### 6.2 Aplicaciones: lógica compartida, servicios de plataforma separados

Se conserva Kotlin Multiplatform con Compose Multiplatform. El código común contendrá modelos, validación, reglas del asistente, sincronización, acceso a proveedores e interfaz reutilizable. Android y Windows implementarán sus adaptadores de micrófono, credenciales, ciclo de vida y notificaciones; Windows añadirá atajo global y recepción de eventos de agente.

Se conserva Room multiplataforma para persistencia. Compartir el código de la base no sincroniza los datos: la sincronización se construye explícitamente con identificadores y revisiones. La documentación oficial contempla código común y configuración por destino. [Room para KMP](https://developer.android.com/kotlin/multiplatform/room).

Las versiones exactas de Kotlin, Compose, Gradle, Android SDK, Room y ESP-IDF se fijarán después de una compilación mínima compatible en Android y Windows. Se registrarán y bloquearán; no se copiarán versiones alfa solo porque aparezcan en un ejemplo actual ni se actualizarán herramientas durante la semana final sin una necesidad demostrada.

### 6.3 Simulador de protocolo

Proceso local de desarrollo que imita los mensajes de la placa: estado, comandos, eventos y vencimiento del control. Se propone Kotlin/JVM para reutilizar los tipos y validadores de las apps sin añadir otra plataforma.

Servirá para probar interfaces y errores de red mientras otra persona tiene la placa. Usará datos y certificados exclusivos de prueba, claramente marcados. No demuestra compatibilidad de GPIO, pantalla, consumo, memoria, TLS en ESP32 ni comportamiento real del micrófono.

### 6.4 Recorrido principal

1. El usuario empareja una app y ve el estado de la placa.
2. La app solicita el permiso de centro y envía una orden identificable.
3. El firmware valida identidad, permiso y parámetros, aplica el cambio y devuelve el estado real.
4. La app muestra el resultado confirmado, no solo el botón presionado.
5. Los eventos quedan registrados y se sincronizan al recuperar conexión.
6. Si Windows toma el control, Android conserva la interfaz y deriva las órdenes al nuevo centro.

## 7. Contratos que deberán fijarse antes de repartir implementación

### Transporte y formato

HTTPS local con mensajes JSON versionados. Primera opción: peticiones y consulta incremental de eventos; no añadir MQTT, WebSockets u otro transporte sin demostrar que son necesarios. mDNS ayuda a encontrar equipos, pero no demuestra su identidad. Se incluirá ingreso manual de dirección como recuperación ante redes que bloquean el descubrimiento.

El sobre de una orden identificará versión de protocolo, identificador de operación, cliente, sesión de arranque de la placa, época del permiso de control, secuencia, acción y parámetros. El rol se verifica en el receptor; declarar `centro` en un mensaje no concede permisos.

Las respuestas distinguirán aplicado, pendiente de confirmación y rechazado; un error contendrá un código estable y no secretos. Los eventos de placa llevarán identificador de arranque y secuencia, tipo y tiempo monotónico. La app incorporará fecha civil cuando la conozca, sin usar el reloj del PC como autoridad del temporizador.

Antes de programar en paralelo se publicarán ejemplos válidos e inválidos y límites de tamaño, rangos, plazos y códigos. El firmware y el simulador deberán superar las mismas pruebas del contrato.

### Control único y pérdida de enlace

La placa será la autoridad que otorga un permiso con vencimiento. Se propone inicialmente un permiso de seis segundos, renovado cada dos segundos, medido con el reloj monotónico de la placa. Son valores de diseño que deben medirse, no resultados obtenidos.

Android es elegible por defecto y puede fijarse manualmente como centro. Windows emparejado solicita prioridad cuando aparece; el relevo es atómico en la placa e incrementa la época del permiso. Dos clientes nunca obtienen autoridad simultánea. Órdenes de la época anterior se rechazan incluso si llegan tarde.

Si el centro desaparece, el permiso vence y otro cliente emparejado puede reclamarlo. Sin candidato, la placa queda autónoma. La app que pierde el permiso deja de originar operaciones y cancela cualquier captura anterior; no debe interpretar silencio de red como continuidad del control. Android podrá realizar una captura nueva como acompañante únicamente con una autorización temporal del centro vigente, según la sección 9.

El traspaso sincroniza configuración y eventos antes de ceder cuando es posible. Ante una salida abrupta, se recuperan los registros confirmados y se declara cualquier dato no sincronizado. Una respuesta de IA pendiente no autoriza ejecutar una herramienta después de perder el control.

### Reintentos, eventos y configuración

Reintentar una operación conserva su identificador. Una repetición dentro de su sesión devuelve el resultado ya obtenido; mismo identificador con otros parámetros se rechaza. Reiniciar la placa cambia su identificador de arranque y hace inválidos los mensajes viejos. No se reenviarán automáticamente acciones pendientes de una sesión anterior.

La prueba de cinco minutos sin red requiere retener al menos 300 muestras periódicas, además de los eventos funcionales. Se propone un buffer circular en RAM de 600 muestras y una cola separada para eventos relevantes. Se medirá su tamaño; los desbordes se informan como brechas de datos y nunca se presentan como historial completo. La garantía de desconexión no incluye pérdida de alimentación.

Se persiste la configuración al cambiar, no una vez por segundo. Las apps conservan eventos con identificadores únicos y confirmaciones por cursor. Notas y recordatorios solo se modifican a través del centro; el acompañante desconectado puede guardar borradores, pero no confirmar una modificación compartida. Eliminaciones se sincronizan como marcas para que no reaparezcan al reconectar.

## 8. Seguridad y primer emparejamiento

La v3 menciona un QR mostrado en la otra app, pero el primer uso todavía no tiene una app de confianza. Para resolver ese arranque se propone:

1. Un asistente de aprovisionamiento por USB configura la red y obtiene la identidad criptográfica de la placa local. Solo opera durante un modo físico explícito y temporal.
2. Ese asistente permite autorizar la primera app mediante un QR con identidad, huella y secreto de un solo uso. Se verifica el destino antes de enviar credenciales por red.
3. La primera app se conecta con TLS, valida la identidad obtenida por el canal físico y recibe su credencial individual. Una confirmación física completa el alta.
4. Los siguientes clientes se incorporan mediante la app autorizada, QR temporal y confirmación física. Las apps intercambian también sus identidades para autenticar la sincronización.
5. Credenciales revocables por cliente; restablecimiento físico explícito borra confianza y configuración. No se incluye una contraseña universal en el código.

Este flujo requiere un computador para la configuración inicial; después el sistema funciona solo con Android y la placa. La app Android contempla escanear el QR y pedir permiso de cámara exclusivamente para esa acción; no implica habilitar la función ampliada de visión.

La implementación elegirá bibliotecas mantenidas para generación de identidades y QR. Las claves privadas de cada dispositivo se generan localmente, no se distribuyen en Git. Los certificados de prueba no se reutilizan en hardware real. La validación TLS no se desactiva para hacer funcionar una demostración.

Se conservará el acceso local previsto en la v3: contraseña con verificador PBKDF2 y sal aleatoria, separado de las credenciales de red. No habrá registro en una nube. Android usará cifrado con una clave protegida por Keystore; Windows utilizará almacenamiento protegido del sistema. Las APIs de proveedor se configuran por separado en cada app y nunca viajan a la placa.

El servidor HTTPS del ESP32 y la concurrencia se medirán con la animación activa. La documentación de Espressif advierte que el establecimiento TLS tiene un coste apreciable y que la sincronización entre tareas corresponde a la aplicación; por ello se reutilizarán conexiones cuando sea viable y se medirán aparte conexión inicial y orden con enlace establecido. [HTTPS Server](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/protocols/esp_https_server.html).

## 9. Interacción, voz y seguridad de las acciones

### Gestos y prioridades

Una misma pulsación no puede simultáneamente confirmar una operación, iniciar estudio y abrir el micrófono. La máquina de estados dará precedencia a confirmación pendiente y emparejamiento explícito; después resolverá pulsación sostenida para hablar, doble toque para pausa/reanudación y toque breve contextual. Se probarán los umbrales con el TTP223 en modo momentáneo.

Toda confirmación mostrará la acción concreta en la app y una señal en la placa, tendrá vencimiento y quedará vinculada a una sola operación y a la época de control. Desconexión, relevo o vencimiento cancelan la solicitud. Un toque repetido no ejecuta dos veces la acción.

Las expresiones temporales tendrán caducidad y prioridad. El estado del temporizador y de conexión se conserva independientemente de la expresión. El indicador de captura tiene prioridad sobre avisos decorativos. Si falla la sincronización del indicador o se pierde el permiso, la aplicación cancela la captura.

### Voz: prueba de viabilidad temprana, no una garantía universal

El INMP441 mide ruido local. La voz se captura en Android o Windows solamente al solicitarlo. El audio temporal se mantiene en memoria y se descarta al terminar o cancelar; no se registra en archivos de diagnóstico. Se indica al usuario qué proveedor recibe la transcripción y se impide capturar por defecto.

Se conserva la alternativa de la v3: con Windows como centro, si su micrófono no está disponible, Android puede prestar el suyo. También puede solicitarse dictado explícitamente desde la app móvil acompañante. Autoridad de ejecución y autorización de captura son distintas: el centro concede una autorización breve vinculada a la época actual y a una solicitud, y el teléfono acepta solo si dispone de permiso y disponibilidad de micrófono.

Antes de iniciar la captura, el centro solicita y obtiene confirmación del indicador de escucha en la placa. El teléfono recibe esa autorización, captura únicamente mientras se mantiene la interacción y entrega el audio temporal al centro por HTTPS autenticado. El centro realiza la transcripción y la llamada al modelo; el acompañante no inicia consumo propio ni ejecuta herramientas. El formato y el límite acotado de audio se fijarán en el contrato de voz según el proveedor de transcripción seleccionado.

Una solicitud de captura tendrá caducidad, un máximo propuesto de 15 segundos y cancelación explícita. Conflicto con otra app, pérdida de enlace, expiración del permiso o cambio de centro cancelan la captura y descartan el audio. No se transfiere una grabación a medias al nuevo centro: se pide repetir. Si el teléfono tampoco está disponible, se ofrece el chat. El indicador físico y su cancelación se probarán también en esta ruta delegada.

La v3 pide que otra aplicación nunca pierda el micrófono y que una captura se cancele cuando otra lo necesite. Esa política se conserva como objetivo, pero las APIs de plataforma no deben suponerse un árbitro universal. Android aplica prioridades y permite detectar el silenciamiento de la propia captura. Windows distingue modos compartido y exclusivo; abrir el micrófono con éxito no prueba que nadie más lo use. Se descarta el modo exclusivo para este asistente. [Android: compartir entrada de audio](https://developer.android.com/media/platform/sharing-audio-input); [Windows: modos de audio](https://learn.microsoft.com/en-us/windows/win32/coreaudio/exclusive-mode-streams).

Android restringe el inicio de servicios de micrófono desde segundo plano. Un toque recibido por WiFi no se asumirá equivalente a una interacción en pantalla. Se probará una activación voluntaria de la sesión desde la app y, después, el toque con pantalla encendida, apagada y app en segundo plano. Ante imposibilidad, se ofrece abrir la app o usar texto y se documenta el ajuste para el docente. [Tipos de servicio en primer plano](https://developer.android.com/develop/background-work/services/fgs/service-types).

La prueba de viabilidad deberá cubrir INFINIX/XOS y Windows, permisos denegados, llamadas iniciadas antes y durante la captura, pérdida de red y desconexión del micrófono. Se registrará qué combinaciones funcionan y qué latencia de cancelación se logra. No se usarán solo entradas del registro de Windows como evidencia concluyente de disponibilidad.

La política conservadora será: ante ocupación conocida o estado incierto, no iniciar captura; ante conflicto detectado, cancelar y descartar. El acceso por texto permanece disponible. Esto no marca automáticamente como aprobadas P09/P10: las limitaciones encontradas se elevan al equipo/docente antes de congelar el alcance.

### Herramientas del asistente

El modelo propone acciones tipadas; la app valida y ejecuta un catálogo cerrado. Nunca recibe un intérprete de comandos ni acceso libre al sistema de archivos.

Se seguirá la tabla específica de la v3: sesiones, recordatorios, notas y consultas no requieren toque; cambios de umbral y avisos sí lo requieren. La frase general «acciones con efecto» de la v3 es ambigua frente a esa tabla: este documento propone adoptar la tabla como regla precisa. No se incorporan acciones del computador en la base.

Texto y voz utilizan el mismo validador y las mismas herramientas. El respaldo entre proveedores puede repetir una solicitud de interpretación, pero nunca una acción ya aplicada. Una herramienta no se ejecuta antes de validar completamente su resultado y comprobar que conserva autoridad.

## 10. IA opcional y control de consumo

Se propone un adaptador inicial para un formato compatible con Chat Completions y perfiles de proveedor con capacidades explícitas. Se validarán realmente las funciones utilizadas; una URL compatible no garantiza idénticos formatos de herramientas, audio o errores. Un segundo perfil probará el respaldo. No se intentará certificar todos los proveedores antes de la entrega.

La configuración inicia con IA desactivada y sin claves. El usuario debe seleccionar proveedor/modelo, registrar tarifas, establecer presupuesto y habilitar llamadas. Los textos locales y las funciones manuales siguen disponibles. Las pruebas automáticas usan respuestas simuladas y no generan consumo.

Solo el centro con permiso vigente puede iniciar llamadas. El gasto se registra con identificador de solicitud, proveedor, estimación reservada, consumo confirmado y estado incierto. Antes de una llamada se reserva su coste máximo estimado según entrada y límite de salida. Si faltan tarifas, límites o un historial confiable, se bloquea la llamada pagada.

Durante un relevo normal se dejan de admitir llamadas nuevas, se resuelven o conservan las reservas pendientes y se sincroniza el registro contable antes de habilitar llamadas en el nuevo centro. **Todo relevo abrupto sin cierre contable confirmado deja pausadas las nuevas llamadas pagadas**, aunque la última copia del registro parezca limpia: podría faltar una llamada posterior a esa copia. El relevo del control físico y las funciones locales siguen funcionando.

La propuesta inicial acepta que esa pausa dure hasta que el centro anterior vuelva y se reconcilien sus registros. Si no puede recuperarse, el equipo deberá conciliar el consumo con los datos del proveedor antes de reactivar el mismo presupuesto; no se ofrece un botón que borre reservas inciertas y finja que no hubo gasto. Recuperar automáticamente la IA pagada sin el centro anterior exigiría añadir un registro de reservas confiable y accesible a ambos antes de cada llamada. Esa complejidad no se incorpora en la primera versión propuesta; esta limitación debe aceptarse expresamente al revisar el diseño.

No se supone que cancelar una petición impide que el proveedor la facture. Un timeout incierto no dispara automáticamente otra llamada pagada sin resolver o conservar su reserva. Si no puede reconciliarse una reserva, se mantiene debitada de la disponibilidad del periodo.

El límite de la app controla sus solicitudes estimadas; no es una garantía sobre la factura completa de una cuenta usada también fuera del proyecto. Cuando el proveedor permita límites propios, se documentará cómo configurarlos. Los importes reales los define el equipo, no este informe.

## 11. Integración de agentes y colaboración de desarrollo

### La integración del producto

Claude Code enviará eventos acotados a un receptor local autenticado de la app de escritorio. No enviará código fuente ni transcripciones completas. Se probarán los eventos de fin de turno y solicitud de permiso con la versión instalada. La mascota comunica estado, no aprueba permisos ni certifica que una tarea esté correctamente resuelta. Los hooks son mecanismos de eventos documentados por Claude Code; su instalación se validará en Windows. [Referencia de hooks](https://code.claude.com/docs/en/hooks).

### El uso de Claude y Codex para construirlo

Cada tarea tendrá un responsable humano, un resultado comprobable, archivos asignados y un revisor distinto. Un agente puede implementar o revisar; el integrante sigue siendo responsable de probar el resultado y entenderlo.

Se prepararán `AGENTS.md` y `CLAUDE.md` breves que remitan a la misma documentación canónica, con instrucciones sobre alcance, comandos de validación, secretos, archivos compartidos y evidencia. Los agentes no deben copiar decisiones de otros chats ni inventar resultados de hardware.

Cada integrante trabajará en su clon y una rama por tarea. Se propone `main` como rama integrable y revisiones mediante pull request, sin rama `develop` adicional. Una persona será responsable de integrar cada entrega; no aprobará sola sus propios cambios funcionales. El repositorio remoto se creará cuando el equipo indique cuenta y visibilidad, sin publicar automáticamente archivos del curso.

Cuando dos tareas necesiten cambiar el protocolo o la misma configuración de compilación, se integrará primero ese cambio común. El segundo trabajo se actualizará desde él. No se resolverán conflictos aceptando ciegamente una versión generada por otro agente.

### Responsabilidades propuestas

| Rol de trabajo | Responsabilidad principal | Colaboración obligatoria |
|---|---|---|
| A: dispositivo | Firmware, montaje, recursos, audio y pruebas físicas | Define contrato con B; mide integración completa |
| B: aplicaciones | KMP, interfaz, datos, emparejamiento, sincronización | Implementa plataforma Android y revisa pruebas de A |
| C: asistente e integración | Proveedores, herramientas, hooks y automatización de pruebas | Comparte voz con B; no recibe toda la validación del proyecto |

No se asignan nombres sin conocer preferencias ni disponibilidad física del prototipo. La primera reunión repartirá A/B/C; nadie queda dedicado solo a documentación. La voz es una tarea conjunta de B y C y las pruebas físicas involucran a los tres. Cada integrante debe demostrar al menos una función fuera de su área principal.

## 12. Estructura prevista del repositorio

Esta es una propuesta de organización, no una afirmación de que estos archivos ya existan. Se crearán con el plan aprobado y se añadirán módulos solo cuando una entrega los necesite.

```text
Proyecto_pet_agentico_IOT/
  README.md                       Punto de entrada y estado real
  AGENTS.md                       Instrucciones compartidas para agentes
  CLAUDE.md                       Entrada para Claude; remite a la guía común
  .gitignore                      Secretos, compilaciones y datos locales
  .gitattributes                  Convenciones Windows/Linux
  .github/                        Plantillas de tareas/PR y validaciones
  firmware/                       Proyecto ESP-IDF
  apps/                           Proyecto Gradle con destinos Android/desktop
  simulator/                      Destino JVM que comparte el contrato
  contracts/                      Esquemas, ejemplos y compatibilidad
  integrations/claude-code/        Adaptador de eventos y guía de instalación
  docs/
    superpowers/specs/            Diseño aprobado y revisiones
    superpowers/plans/            Plan ejecutable por entregas
    equipo/                       Entornos, responsabilidades y colaboración
    hardware/                     Componentes, pinout confirmado y montaje
    pruebas/                      Casos, resultados y limitaciones
    entrega/                      Evidencias académicas y guion
```

El simulador y las apps compartirán un único conjunto de herramientas JVM y tipos de protocolo; la separación de carpetas no exige repositorios o sistemas de compilación independientes. Los nombres precisos de módulos Gradle se fijarán en la primera configuración compilable.

Documentación en español, nombres de símbolos y mensajes de protocolo consistentes en inglés. Los scripts necesarios tendrán instrucciones reproducibles para Windows y Linux; no se dependerá de rutas absolutas del computador de Walther. No se copiarán los PDF docentes al repositorio público sin autorización. Los recursos gráficos y sonoros incluirán procedencia y licencia.

## 13. Hoja de ruta revisada: entregas verificables

Este calendario organiza el alcance; el plan de implementación posterior lo convertirá en tareas con archivos, contratos y verificaciones concretas. No se considera que la semana 10–16 de noviembre haya producido software.

| Periodo | Entrega integrada | Evidencia para pasar a la siguiente |
|---|---|---|
| 17–23 nov | Entorno reproducible; apps mínimas Android/Windows; placa o simulador recibe una orden y confirma estado; prueba temprana de voz y TLS | Compilación en Windows, ejecución Android, contrato mínimo y registro de viabilidad. Una prueba física se exige al disponer del material |
| 24–30 nov | Personaje, sensores, sonidos si llega el módulo; sesión de estudio desde la app; historial, emparejamiento y relevo Android/Windows | Sesión continúa al cortar red; cliente no autorizado rechazado; relevo sin dos centros. Primera interacción por texto con proveedor |
| 1–7 dic | Chat con herramientas, voz bajo demanda según matriz validada, confirmación física, hooks y gasto compartido | Misma tarea por texto y voz; confirmación expirada no aplica; caída de proveedor y relevo no duplican efectos |
| 8–13 dic | Integración y pruebas de falla/privacidad/consumo en dos Android y Windows; corregir, sin abrir nuevas funciones ampliadas mientras falte base | Matriz P01–P19 con evidencia, defectos resueltos o limitaciones comunicadas al docente |
| 14–16 dic | Congelar versión, ensayo y entrega | Repositorio compilable desde clon limpio, artefactos identificados y demostración reproducible |

Cada semana reserva parte de las nueve horas para integrar y explicar lo realizado, no nueve horas exclusivamente para generar código. Se propone dedicar el bloque de clase a una integración con revisión y el bloque del fin de semana a construcción y prueba conjunta. El reparto fino depende de los horarios reales del equipo.

La primera entrega visible será: abrir Android, conectar con una identidad conocida, pedir una expresión, verla cambiar y recibir confirmación; repetir desde Windows de forma secuencial. Después se añade el relevo automático. El simulador permite avanzar en ese recorrido, pero su resultado se etiqueta como simulado hasta repetirlo con la placa.

### Puntos de decisión para proteger la fecha

- **23 nov:** confirmar compilación multiplataforma y resultados de micrófono/segundo plano. Las restricciones se presentan al equipo/docente esa semana, no el día de la entrega.
- **30 nov:** debe existir un recorrido físico integrado. Si falta, el equipo revisa dependencias y alcance con el docente; no se agregan ampliaciones.
- **7 dic:** las funciones base deben estar integradas para dejar pruebas y correcciones. Una función incompleta se declara como tal.
- **13 dic:** congelación funcional. Solo correcciones que protejan la demostración y la integridad de datos.

## 14. Validación y evidencias académicas

| Pruebas de la v3 | Qué se verificará | Evidencia mínima |
|---|---|---|
| P01–P03 | Arranque, entradas/salidas, configuración aplicada y sonido añadido | Identificación de placa, montaje, vídeo breve y estado confirmado |
| P04–P05 | Acceso, validación, identidad, permiso, mensajes antiguos e idempotencia | Casos negativos con resultado y sin efectos físicos |
| P06–P07 | Relevo al llegar y al desaparecer Windows | Cronología con época/centro y ausencia de doble control |
| P08–P10 | Voz, micrófono ocupado y conflicto durante captura | Matriz de apps/SO/permisos con resultados, límites y latencias |
| P11–P12 | Chat equivalente y confirmación física | Mismas herramientas, rechazo al expirar y un solo efecto |
| P13–P14 | Respaldo y presupuesto común | Fallas controladas, llamadas simuladas y prueba real acotada autorizada |
| P15–P16 | Sin internet y cinco minutos sin red | Continuidad local y recuperación de al menos 300 muestras sin duplicados |
| P17 | Avisos de agente | Fin de turno y solicitud de permiso con retorno de expresión |
| P18 | Segundo plano, dos teléfonos y consumo | Prueba de 30 minutos, condiciones de batería y medición física |
| P19 | Privacidad | Revisión de archivos, registros y contenido enviado sin secretos ni audio persistente |

Las pruebas automáticas cubrirán especialmente validadores, exclusividad del centro, expiración, reintentos, presupuesto, confirmaciones y sincronización. No se escriben pruebas que solo repitan constantes sin comprobar comportamiento. Hardware, permisos y segundo plano requieren pruebas reales, no solamente mocks.

Cada registro incluirá fecha, integrante, versión/commit, dispositivo, pasos, resultado esperado, resultado observado y evidencia. «No ejecutado», «falló», «pasó en simulador» y «pasó en hardware» son estados diferentes. Se conservan las metas de la v3 de una muestra por segundo y confirmación inferior a un segundo con conexión establecida como objetivos por medir, no garantías.

La relación con los 29 indicadores se hereda de la sección 12 de la v3 y deberá cotejarse con la rúbrica original antes de entregar. Este trabajo no ha verificado aún los 29 indicadores contra su archivo fuente independiente.

## 15. Incertidumbres, verificaciones y límites actuales

| Punto | Estado y siguiente acción |
|---|---|
| Android del INFINIX y segundo teléfono | Identificar en la primera reunión; no inferir Android a partir de XOS |
| Versiones de Windows y Linux | Registrar antes de fijar la matriz soportada |
| Roles A/B/C y acceso a la placa | Repartir con nombres y horario físico; no bloquea la revisión del diseño |
| Componentes recibidos, pantalla y MAX98357A | Confirmar llegada y lote; no generar un pinout definitivo a ciegas |
| Entornos locales | En este equipo se encontraron Git, Java/Javac y Python en PATH; adb, CMake, Ninja, idf.py y Gradle no aparecieron en PATH. Eso no demuestra que no estén instalados en otra ubicación |
| Micrófono y Android bloqueado | Prototipo de viabilidad prioritario; no prometer arbitraje universal |
| Hotspot y descubrimiento | Probar con el INFINIX; router de 2,4 GHz como red de prueba alternativa |
| Recursos ESP32 | Medir TLS, pantalla, I2S y buffers juntos; ajustar recursos sin retirar autenticación |
| Presupuesto de APIs | Definir importe y perfiles antes de habilitar llamadas reales; simulación primero |
| GitHub | Elegir propietario, colaboradores y visibilidad antes de crear/publicar el remoto |
| Paquetes/licencias | Fijar versiones después de compilación mínima; conservar atribuciones |

No se ha instalado software, programado la placa, ejecutado apps, creado un repositorio remoto ni comprobado sensores durante esta preparación. El contenido técnico se ha contrastado con fuentes primarias en los puntos citados, pero eso no equivale a validarlo en los dispositivos del equipo.

## 16. Qué sigue tras revisar este diseño

La siguiente etapa es preparar el plan ejecutable y la base documental del repositorio: README, guías de entorno, tareas por entrega, contratos de la primera integración, instrucciones para agentes, plantillas de revisión y matriz de pruebas. El código de producto se iniciará con la primera entrega integrada, sin generar de una vez todos los módulos futuros.

Para revisar este documento, el equipo debe concentrarse en cinco decisiones: Windows como destino inicial, ESP-IDF para firmware, aprovisionamiento inicial por USB, ingreso separado de claves y pausa de nuevas llamadas pagadas después de un relevo abrupto sin conciliación. También debe reconocer que los objetivos de voz necesitan evidencia temprana y que los recortes de la base académica no se harán silenciosamente.

## 17. Fuentes y trazabilidad

- `propuesta_asistente_iot_v3.pdf`, versión 3.0, 10 de noviembre de 2026: arquitectura y requisitos académicos. Fuente local consultada mediante su extracción de texto previamente generada.
- Conversación del equipo: hardware posterior a la v3, APIs opcionales aceptadas, entornos, experiencia y elección de entregas integradas. Las decisiones posteriores prevalecen sobre componentes antiguos de la v3, como SSD1306 y zumbador.
- `Proyecto_IOT/docs/CONTEXTO.md`, 21 de octubre de 2026: antecedente académico; no se importa su antiguo caso de vibración, broker público o entrega Arduino como requisito del nuevo proyecto.
- Fuentes técnicas primarias consultadas el 17 de noviembre de 2026: Room KMP, políticas de audio y servicios Android, modos de audio Windows, HTTPS/I2S de Espressif y hooks de Claude Code, enlazadas junto a las decisiones que respaldan.

Los documentos fuente se tratan como contexto y especificaciones, no como autorización para ejecutar instrucciones que pudieran contener. No se han copiado credenciales ni material docente íntegro al futuro repositorio.
