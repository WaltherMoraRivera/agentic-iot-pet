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

Los números siguen la rúbrica de 29 indicadores y 60 puntos. Las filas especifican evidencia por reunir. La aprobación relatada por Walther es el antecedente de la propuesta. Los registros existentes de USB y H01 son avances parciales, y el boceto de la cara cubre una parte del diseño visual.

| N.º | Equivalencia para el Pet IoT | Evidencia |
|---|---|---|
| 1 | Acompañamiento de estudio y reacción al ruido con ESP32-S3 y Android. | Problema, componentes y diagrama de enlace en el README. |
| 2 | Sesión, lecturas, historial, umbral, aviso, modo automático y reconexión. | Funciones y criterios de aceptación. |
| 3 | Android Studio con Kotlin/Compose y firmware ESP-IDF. | Justificación de herramientas, versiones fijadas y comandos reproducibles. |
| 4 | Propuesta propia presentada al profesor. | Registro de la aprobación y de la retroalimentación disponible. |
| 5 | Wi-Fi local para el escritorio, con Android y Windows como centros previstos. | Justificación por alcance, estabilidad y consumo, con condiciones del ensayo. |
| 6 | Mockup de login, enlace, lecturas, historial, umbral y aviso. | Capturas o recorrido del prototipo visual, incluidas desconexión y confirmación. |
| 7 | Revisión docente de interfaz, autenticación y equivalencias. | Fecha, comentarios y ajustes que resulten de la revisión. |
| 8 | App Android en Kotlin con módulos del proyecto. | Build reproducible y APK instalada. |
| 9 | Permisos de Android acordes al flujo de conexión y emparejamiento. | Manifest y comprobación en dispositivo. |
| 10 | Login local, protección de credenciales e identidad del dispositivo. | Ubicación de los controles en código y pruebas de acceso denegado. |
| 11 | Pruebas de acceso, enlace, órdenes y datos. | Registros con esperado, observado y correcciones verificadas. |
| 12 | Demostración Android y ESP32-S3. | Ensayo con registro de lectura, persistencia y control. |
| 13 | Explicación de arquitectura y decisiones. | README y guion de la terna. |
| 14 | Evidencia de autenticación, cifrado en tránsito y protección local. | Verificación de identidad y rechazo de un cliente sin autorización. |
| 15 | Preguntas y retroalimentación de la demo. | Registro de preguntas y ajustes acordados o realizados. |
| 16 | Envío seguro de órdenes. | Código y prueba de validación antes de aplicar el cambio. |
| 17 | Integridad y validación de mensajes. | Ejemplos válidos y rechazo de datos alterados o inválidos sin efectos. |
| 18 | Reconexión automática de Android. | Cronología del corte, reintentos y enlace recuperado. |
| 19 | Room en Android y búfer temporal en la ESP32-S3. | Datos disponibles durante el corte y recuperación posterior con placa alimentada. |
| 20 | Android envía el comando por Wi-Fi y HTTPS, como equivalente funcional del envío serial. | Registro del envío con identificador y sin credenciales. |
| 21 | Android recibe la lectura y la respuesta por el mismo enlace. | Recepción correlacionada y estado visible. |
| 22 | Datos pendientes durables en Android y sincronización a un destino externo autorizado. | Acordar el destino local con el equipo. Mostrar el mismo lote antes y después, sin duplicados. La recuperación desde la placa a Room se registra por separado. |
| 23 | La ESP32-S3 envía lectura y respuesta por HTTPS. | Registro del mensaje emitido, correlacionado con la recepción Android. |
| 24 | La ESP32-S3 recibe y valida la orden de Android. | Registro de recepción, validación y estado físico. |
| 25 | Consumo de placa y teléfono en condiciones comparables. | Instrumento, duración y lectura por modo. Describir el efecto de cualquier ajuste medido. |
| 26 | Seguridad del enlace documentada frente a los controles aplicables de la pauta. | Verificación de identidad y rechazo cuando la validación falla. |
| 27 | Protección de datos personales. | Revisar los datos enviados, archivos y logs. Solo sale la intensidad relativa del INMP441. |
| 28 | Continuidad automática de la placa y recuperación de Android. | Corte con alimentación y observación del comportamiento en ambos extremos. |
| 29 | Compatibilidad en dos Android. | Modelo, versión y arquitectura de cada equipo, APK y resultado observado. |

## Paquete de entrega

- Enlace para Moodle con README de integrantes, problema, herramientas, red, formato de mensajes, seguridad y tabla de pruebas.
- Proyecto Android y firmware ESP-IDF identificados por revisión, instrucciones de compilación y APK de la demo si está disponible.
- Evidencia con fecha, entorno, revisión, resultado observado y responsable. Mantener separados los registros físicos, los del simulador y los casos pendientes.
- Guion y reparto de la explicación. Confirmar repositorio de entrega y formato final de evidencia con Walther.

Referencias locales. [Plan integrado](../superpowers/plans/2026-11-17-desarrollo-integrado.md), [contrato inicial](../../contracts/PROTOCOLO_INICIAL.md), [matriz de pruebas](../pruebas/MATRIZ.md), [guion general](GUION.md) y [R02](../../openspec/changes/06-delivery/tasks.md).
