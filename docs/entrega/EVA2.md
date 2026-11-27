# Evaluación 2 del Pet agéntico IoT

**Evaluación:** Evaluación 13, Sumativa, Aplicación Android + Hardware Microcontrolador. La terna gestiona el calendario.

**Equipo:** Walther, Maximiliano y Winderson. Walther confirmó el 26 de noviembre que el profesor aprobó la terna, la sustitución del prototipo simulado de EVA1 por el Pet agéntico IoT y la adaptación de la pauta mediante equivalencias funcionales.

## Recorrido propuesto

El Pet acompaña el estudio y reacciona al ruido cerca del escritorio. Android permite consultar la intensidad relativa del sonido y el historial, ajustar el umbral y controlar un aviso. La ESP32-S3 ejecuta el modo automático aunque se corte el enlace. El INMP441 aporta una medida relativa de intensidad y el TTP223 sirve para confirmar físicamente los cambios protegidos.

1. Iniciar sesión y conectar el Android a la ESP32-S3 por Wi-Fi local con HTTPS e identidad verificada.
2. Mostrar una lectura actual y una anterior guardada en Room. Registrar el mismo flujo en un segundo Android, con modelo y versión.
3. Cambiar el umbral y encender o apagar el aviso. Mostrar la solicitud, el toque de confirmación, la respuesta de la placa y el estado aplicado.
4. Cortar Wi-Fi con la placa encendida. Observar el modo automático, la señalización de la desconexión y la recuperación de lecturas tras la reconexión.
5. Mostrar un intento rechazado sin efecto y explicar cómo se protegen las credenciales y los datos.

## Qué podemos empezar hoy

### Walther · Android, contrato e informe

1. **#1 T00.** Resolver el SDK configurado en Android Studio, verificar `adb` y preparar el proyecto mínimo con Gradle Wrapper. Primer resultado verificable: generar la APK, abrirla en el INFINIX y registrar revisión, versiones y comando de compilación. La comprobación de Windows y la del firmware pueden correr por separado.
2. **#4 T01.** Usar los [ejemplos del contrato](../../contracts/PROTOCOLO_INICIAL.md) para preparar entradas y resultados esperados de consulta de estado, autoridad exclusiva, vencimiento y reintento. Ejecutar los casos de lógica compartida en cuanto T00 habilite Gradle. Los casos de confianza y rechazo de #5, #6 y #16 se preparan con datos sintéticos.
3. **#19 R02.** Completar el problema, arquitectura, herramientas y equivalencias de la rúbrica en el borrador. Incorporar en orden los resultados de cada frente. Con T00 disponible, preparar también pruebas locales de login, historial y reconexión para #9, #11 y #28.

### Winderson · Mockup, interfaz y banco disponible

1. **#3 V01.** Preparar el flujo de login, enlace, lecturas, historial, umbral y aviso. Incluir tres estados distinguibles: sin conexión, esperando confirmación y cambio aplicado. Guardar capturas del mockup en tamaño móvil e identificar los valores de muestra.
2. **#7 H01.** Con el prototipo disponible, retomar el diagnóstico del [PR #27](https://github.com/wmrDev/agentic-iot-pet/pull/27) y el [traspaso](../equipo/TRASPASO.md). Registrar primero arranque, estabilidad del puerto y latido. La identificación física de pines, continuidad y pruebas individuales siguen en ese orden. Registrar el resultado observado por paso.
3. **#10 V02, #17 V03 y #18 R01.** Adelantar estados de pantalla, legibilidad y guía de instalación. Crear vistas previas de Compose cuando esté la base de T00. Documentar modelo, versión, API y arquitectura del Galaxy para las pruebas de la APK.

### Pruebas físicas · Frente sin asignar

Los issues #7, #8, #15 y #29 quedan sin asignación en GitHub. Las primeras tareas del frente son preparar casos de umbral, autenticación y pérdida de enlace, y definir qué se medirá, con qué instrumento y bajo qué condiciones. Walther y Winderson pueden registrar por separado los datos de sus Android. Usar la [plantilla de resultados](../pruebas/PLANTILLA_RESULTADO.md) para incorporar cada prueba cuando pueda ejecutarse.

## Puntos de integración

1. **Primer intercambio.** Unir la APK, el diagnóstico estable y el contrato. Demostrar primero identidad, consulta de estado y respuesta a una orden en #5 y #6. Usar OLED y pines validados al comprobar la expresión física.
2. **Recorrido Android.** Conectar por función el control y aviso de #8, login e historial de #9, pantallas de #10 y reconexión de #11. La [sincronización #28](https://github.com/wmrDev/agentic-iot-pet/issues/28) verifica recuperación de datos y entrega al destino externo local acordado.
3. **Validación y cierre.** Registrar los casos aplicables de #15, #16 y #17 en la [validación en dos Android #29](https://github.com/wmrDev/agentic-iot-pet/issues/29). Usar la misma revisión de firmware y APK, registrar cada caso por separado y cerrar paquete, informe y ensayo con #18, #19 y #30.

El [issue coordinador #30](https://github.com/wmrDev/agentic-iot-pet/issues/30) reúne el orden. El [hito EVA_02](https://github.com/wmrDev/agentic-iot-pet/milestone/9) contiene las tareas específicas y la [etiqueta EVA_02](https://github.com/wmrDev/agentic-iot-pet/issues?q=label%3AEVA_02) reúne también los issues compartidos con el producto.

**Equipos disponibles:** Winderson tiene el hardware. Los Android de prueba son el INFINIX ZERO 30 5G de Walther y el Galaxy S24 Ultra de Winderson. Registrar versión de Android, nivel de API y arquitectura al ejecutar las pruebas. El iPhone 14 de Maximiliano puede servir para documentar el ensayo.

## Referencia técnica de StackChan

StackChan sirve de referencia para el controlador OLED candidato, las expresiones temporales, las pruebas de lógica en el computador y el diagnóstico de memoria y red. El issue #30 reúne las aplicaciones a cada tarea. La compatibilidad con el lote del Pet se comprueba en banco. [Referencia en el issue #30](https://github.com/wmrDev/agentic-iot-pet/issues/30) · [Revisión de StackChan](https://github.com/m5stack/StackChan/commit/28b9f0c8bd6acd81cb3c70f1a93c3e373b39d400).

## Matriz de evidencia

Los números siguen la rúbrica de 29 indicadores y 60 puntos. Revisión de referencia: `main` en `1623ada` (PR #39), 27 de noviembre de 2026. «Disponible» incluye solo lo que existe hoy en `main`, en un PR fusionado o en un issue. Tipos: **compilación**, **simulación**, **banco** (componente físico aislado), **integrada** (Android real con la placa real) y **documento** (diseño, contrato o código sin ejecutar). La instalación de la APK en el INFINIX se marca **física, solo Android**, porque no usa la placa.

**Resumen.** 24 indicadores tienen alguna evidencia y 5 no tienen ninguna (6, 7, 15, 18 y 22). De los 24, 11 tienen una verificación ejecutada y parcial (compilación, simulación, banco preliminar o la instalación en el INFINIX) y 13 solo documento o código sin ejecutar. **Ningún indicador tiene todavía una prueba integrada**, y ninguna de las fichas [E01–E12](../pruebas/eva02/FICHAS.md) se ha ejecutado.

Vacíos más críticos para la demo:

1. **No existe enlace Android–ESP32-S3.** Faltan el servidor HTTPS en la placa, la identidad y el emparejamiento ([T02 #5](https://github.com/wmrDev/agentic-iot-pet/issues/5)) y la orden de extremo a extremo ([T03 #6](https://github.com/wmrDev/agentic-iot-pet/issues/6)). Bloquea los indicadores 10, 12, 14, 16, 17 y 20 a 26.
2. **La placa no ejecuta todavía ningún firmware observado.** La incidencia USB sigue abierta, los pines no están verificados en [H01 #7](https://github.com/wmrDev/agentic-iot-pet/issues/7) y el [procedimiento de T04](../pruebas/T04_PLACA.md) ([#8](https://github.com/wmrDev/agentic-iot-pet/issues/8)) no se ha ejecutado. Bloquea los indicadores 12, 16, 19, 24 y 28.
3. **La app Android solo muestra la pantalla de bienvenida.** No tiene login, Room, reconexión ni sincronización ([T05 #9](https://github.com/wmrDev/agentic-iot-pet/issues/9), [#28](https://github.com/wmrDev/agentic-iot-pet/issues/28)), y falta el mockup del flujo ([V01 #3](https://github.com/wmrDev/agentic-iot-pet/issues/3)). Bloquea los indicadores 6, 10, 18, 19 y 22.

| N.º | Equivalencia para el Pet IoT | Evidencia disponible | Prueba o evidencia pendiente |
|---|---|---|---|
| 1 | Acompañamiento de estudio y reacción al ruido con ESP32-S3 y Android. | Problema y componentes en el [README](../../README.md) y el [diseño aprobado](../superpowers/specs/2026-11-17-diseno-software.md) (documento). | Diagrama del enlace y redacción del problema en el informe ([R02 #19](https://github.com/wmrDev/agentic-iot-pet/issues/19)). |
| 2 | Sesión, lecturas, historial, umbral, aviso, modo automático y reconexión. | Funciones en el [diseño](../superpowers/specs/2026-11-17-diseno-software.md) y resultados esperados en las [fichas E02–E09](../pruebas/eva02/FICHAS.md) (documento). | Tabla de funciones con su criterio de aceptación en el informe (#19). |
| 3 | Android Studio con Kotlin/Compose y firmware ESP-IDF. | Versiones fijadas en [ENTORNOS.md](../equipo/ENTORNOS.md), el [catálogo](../../gradle/libs.versions.toml) y Gradle Wrapper. [CI](../../.github/workflows/build.yml) en verde en el [PR #39](https://github.com/wmrDev/agentic-iot-pet/pull/39) y reproducción desde clon limpio en [#32](https://github.com/wmrDev/agentic-iot-pet/issues/32) (compilación). | Justificación de cada herramienta en el informe (#19). |
| 4 | Propuesta propia presentada al profesor. | Aprobación del 26-11 relatada por Walther, al inicio de este documento (documento). | Respaldo escrito de la aprobación (correo o acta) y retroalimentación recibida (#19). |
| 5 | Wi-Fi local para el escritorio, con Android y Windows como centros previstos. | Transporte HTTPS local en el [contrato](../../contracts/PROTOCOLO_INICIAL.md) y el [diseño](../superpowers/specs/2026-11-17-diseno-software.md) (documento). | Justificación por estabilidad y consumo con datos de [E09–E11](../pruebas/eva02/FICHAS.md) y condiciones del ensayo (#19, [#29](https://github.com/wmrDev/agentic-iot-pet/issues/29)). |
| 6 | Mockup de login, enlace, lecturas, historial, umbral y aviso. | Ninguna todavía. El [boceto de la cara](https://github.com/wmrDev/agentic-iot-pet/issues/3#issuecomment-7180532036) está en una rama sin fusionar y no cubre el flujo Android. | Capturas en tamaño móvil del flujo, con desconexión, espera de confirmación y cambio aplicado ([V01 #3](https://github.com/wmrDev/agentic-iot-pet/issues/3)). |
| 7 | Revisión docente de interfaz, autenticación y equivalencias. | Ninguna todavía. | Fecha, comentarios del profesor y ajustes derivados ([#30](https://github.com/wmrDev/agentic-iot-pet/issues/30)). |
| 8 | App Android en Kotlin con módulos del proyecto. | APK de `93529a3` generada e instalada en el INFINIX ([registro T00](../pruebas/resultados/2026-11-26-T00-base-aplicaciones.md), [PR #31](https://github.com/wmrDev/agentic-iot-pet/pull/31)); módulos `apps/shared`, `apps/androidApp` y `apps/desktopApp` (compilación; física, solo Android). | APK con las funciones de T03 y T05, identificada por commit y SHA-256 ([E01](../pruebas/eva02/FICHAS.md), #29). |
| 9 | Permisos de Android acordes al flujo de conexión y emparejamiento. | [Manifest](../../apps/androidApp/src/main/AndroidManifest.xml) sin permisos declarados y con respaldo desactivado (documento). Aún no hay función de red que los requiera. | Permisos que añada T03 y su comprobación en ambos teléfonos (paso 4 de [E01](../pruebas/eva02/FICHAS.md), #6, #29). |
| 10 | Login local, protección de credenciales e identidad del dispositivo. | Diseño de credenciales e identidad en el [contrato](../../contracts/PROTOCOLO_INICIAL.md) (documento). Sin código de login ni de identidad. | Implementación en [T02 #5](https://github.com/wmrDev/agentic-iot-pet/issues/5) y [T05 #9](https://github.com/wmrDev/agentic-iot-pet/issues/9); acceso denegado en [E02](../pruebas/eva02/FICHAS.md) y E08a–E08d. |
| 11 | Pruebas de acceso, enlace, órdenes y datos. | 21 casos de autoridad, JSON y ejemplos en `8267a0b`, con fallo previo y mutaciones verificadas ([registro T01](../pruebas/resultados/2026-11-26-T01-contrato.md), [casos](../pruebas/T01-contrato.md), [PR #33](https://github.com/wmrDev/agentic-iot-pet/pull/33)) (simulación). | Registros con esperado y observado de [E02, E07 y E08](../pruebas/eva02/FICHAS.md) con la placa (#6, #29). |
| 12 | Demostración Android y ESP32-S3. | Solo avances aislados: mediciones preliminares de alimentación, LED y TTP223, sin fotos ([H01 preliminar](../pruebas/resultados/2026-11-25-H01-preliminar.md)), y placa identificada con esptool ([registro USB](../pruebas/resultados/2026-11-25-T00-prueba-usb.md)) (banco). No demuestran la demo. | Ensayo con lectura, persistencia y control: [E03, E04 y E06](../pruebas/eva02/FICHAS.md) tras #6 y #8; ensayo final en #30. |
| 13 | Explicación de arquitectura y decisiones. | [README](../../README.md), [diseño](../superpowers/specs/2026-11-17-diseno-software.md), [decisiones](../equipo/DECISIONES.md) y [guion general](GUION.md) (documento). | Guion de la terna adaptado al recorrido EVA_02, con reparto de la explicación (#19). |
| 14 | Evidencia de autenticación, cifrado en tránsito y protección local. | TLS, identidad y credenciales descritos en el [contrato](../../contracts/PROTOCOLO_INICIAL.md); respaldo de datos desactivado en el [manifest](../../apps/androidApp/src/main/AndroidManifest.xml) (documento). | Verificación de identidad y rechazo de un cliente no autorizado en E08a–E08d (#5, #29) y revisión en [Q02 #16](https://github.com/wmrDev/agentic-iot-pet/issues/16). |
| 15 | Preguntas y retroalimentación de la demo. | Ninguna todavía. | Registro de preguntas y ajustes tras la demo (#30). |
| 16 | Envío seguro de órdenes. | Autoridad con permiso temporal, época, secuencia e idempotencia probada en Kotlin ([registro T01](../pruebas/resultados/2026-11-26-T01-contrato.md)) (simulación). Confirmación física y validación de umbral en el firmware, con tests Unity que solo se compilan en CI ([PR #36](https://github.com/wmrDev/agentic-iot-pet/pull/36)) (compilación). | Tests Unity en la placa ([T04_PLACA](../pruebas/T04_PLACA.md), paso 1, #8); [E04, E05 y E08](../pruebas/eva02/FICHAS.md) integradas (#6). |
| 17 | Integridad y validación de mensajes. | `ProtocolCodec` rechaza claves repetidas, propiedades desconocidas y tamaños excedidos; los [ejemplos](../../contracts/examples/) pasan en el flujo simulado ([registro T01](../pruebas/resultados/2026-11-26-T01-contrato.md)) (simulación). [Esquema de la orden](../../contracts/v1/command.schema.json) (documento). | La misma batería contra la placa en E08e, E08f y siguientes (#6, #29). |
| 18 | Reconexión automática de Android. | Ninguna todavía. | Cronología del corte, reintentos y enlace recuperado en [E09](../pruebas/eva02/FICHAS.md) ([#28](https://github.com/wmrDev/agentic-iot-pet/issues/28), #9). |
| 19 | Room en Android y búfer temporal en la ESP32-S3. | Búfer de 600 muestras y 64 eventos en [telemetry.h](../../firmware/main/telemetry.h), con tests Unity compilados en CI y sin ejecutar ([PR #36](https://github.com/wmrDev/agentic-iot-pet/pull/36)) (compilación). Room no está implementado. | Room en T05 (#9); datos durante el corte y recuperación con la placa alimentada en E09 (#28). |
| 20 | Android envía el comando por Wi-Fi y HTTPS, como equivalente funcional del envío serial. | Formato de la orden en el [contrato](../../contracts/PROTOCOLO_INICIAL.md) y su [ejemplo](../../contracts/examples/command-expression.json), verificado en el flujo simulado de T01 (simulación). Sin cliente HTTPS. | Registro del envío con `request_id` y sin credenciales en [E07](../pruebas/eva02/FICHAS.md) (#6). |
| 21 | Android recibe la lectura y la respuesta por el mismo enlace. | Ejemplos de [estado](../../contracts/examples/state.json) y [respuesta](../../contracts/examples/reply-expression.json) decodificados en el flujo simulado de T01 (simulación). | Recepción correlacionada y estado visible en E03 y E07 (#6). |
| 22 | Datos pendientes durables en Android y sincronización a un destino externo autorizado. | Ninguna todavía. El destino local no está acordado. | Acordar el destino y mostrar el mismo lote antes y después, sin duplicados ([#28](https://github.com/wmrDev/agentic-iot-pet/issues/28), #9). |
| 23 | La ESP32-S3 envía lectura y respuesta por HTTPS. | Solo el formato en el [contrato](../../contracts/PROTOCOLO_INICIAL.md) y el borrador de [estudio y eventos](../../contracts/ESTUDIO_Y_EVENTOS.md) (documento). El firmware no tiene servidor HTTPS. | Servidor HTTPS en T03 (#6); mensaje emitido y correlacionado con Android en E07. |
| 24 | La ESP32-S3 recibe y valida la orden de Android. | Validación de rangos de umbral y configuración en [settings.c](../../firmware/main/settings.c) y [sound_reaction.c](../../firmware/main/sound_reaction.c), sin recepción por red (documento; tests solo compilados). | Recepción, validación y estado físico en E04 y E08 (#6, #8). |
| 25 | Consumo de placa y teléfono en condiciones comparables. | Método e instrumentos en el [plan de consumo](../pruebas/eva02/CONSUMO.md), en estado NO EJECUTADA (documento). | Lecturas por modo en [E10 y E11](../pruebas/eva02/FICHAS.md) (#29); falta elegir el instrumento. |
| 26 | Seguridad del enlace documentada frente a los controles aplicables de la pauta. | Controles descritos en el [contrato](../../contracts/PROTOCOLO_INICIAL.md) y en [AGENTS.md](../../AGENTS.md) (documento). | Rechazo cuando falla la verificación de identidad en E08d (#5) y revisión de seguridad (#16). |
| 27 | Protección de datos personales. | El nivel del INMP441 se calcula sin guardar ni transmitir muestras ([sensors.c](../../firmware/main/sensors.c), [H01_BANCO](../hardware/H01_BANCO.md)); respaldo desactivado en el manifest (documento). | Revisión de datos enviados, archivos y logs durante E07 y E08 (#16, #29). |
| 28 | Continuidad automática de la placa y recuperación de Android. | Lógica autónoma sin red del firmware de T04, compilada en CI y sin ejecutar en la placa ([PR #36](https://github.com/wmrDev/agentic-iot-pet/pull/36), [ejecución de CI](https://github.com/wmrDev/agentic-iot-pet/actions/runs/46013730537)) (compilación). | Caso «Sin red» de [T04_PLACA](../pruebas/T04_PLACA.md) (#8) y corte con alimentación en E09 (#28). |
| 29 | Compatibilidad en dos Android. | APK de T00 instalada y abierta en el INFINIX ZERO 30 5G con Android 14 ([captura](../pruebas/resultados/2026-11-26-T00-infinix.png)) (física, solo Android). Galaxy S24 Ultra sin registrar. | [E01](../pruebas/eva02/FICHAS.md) en ambos teléfonos con la misma APK, y API y arquitectura en [DISPOSITIVOS](../pruebas/eva02/DISPOSITIVOS.md) (#29). |

## Paquete de entrega

- Enlace para Moodle con README de integrantes, problema, herramientas, red, formato de mensajes, seguridad y tabla de pruebas.
- Proyecto Android y firmware ESP-IDF identificados por revisión, instrucciones de compilación y APK de la demo si está disponible.
- Evidencia con fecha, entorno, revisión, resultado observado y responsable. Mantener separados los registros físicos, los del simulador y los casos pendientes.
- Guion y reparto de la explicación. Confirmar repositorio de entrega y formato final de evidencia con Walther.

Referencias locales. [Plan integrado](../superpowers/plans/2026-11-17-desarrollo-integrado.md), [contrato inicial](../../contracts/PROTOCOLO_INICIAL.md), [matriz de pruebas](../pruebas/MATRIZ.md), [guion general](GUION.md) y [R02](../../openspec/changes/06-delivery/tasks.md).
