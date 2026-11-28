# Registro de decisiones

## Aprobadas el 17 de noviembre de 2026

| ID | Decisión | Motivo |
|---|---|---|
| D01 | Entregas integradas, repositorio único | Detectar problemas de integración pronto |
| D02 | Android y Windows iniciales; Linux como entorno compatible | Equipos disponibles y plazo |
| D03 | KMP/Compose; ESP-IDF; Room al incorporar historial | Arquitectura docente y control de periféricos/HTTPS |
| D04 | Placa autónoma y centro único con permiso temporal | Continuidad y exclusión de órdenes incompatibles |
| D05 | Primer aprovisionamiento USB; siguientes altas con confianza y toque | Resolver el primer emparejamiento sin identidad previa |
| D06 | Claves API ingresadas por separado, fuera del repositorio | Evitar sincronización de secretos en la primera versión |
| D07 | API opcional y pausa pagada tras relevo abrupto sin conciliación | Funcionamiento local y control conservador del gasto |
| D08 | Sonidos breves con MAX98357A y parlante; OLED familia SH1106 por verificar | Cambios de componentes acordados tras la v3 |
| D09 | Voz móvil delegada cuando Windows sea centro | Conservar la alternativa prevista en la v3 |

La aprobación corresponde al diseño inicial. Las restricciones de micrófono/segundo plano siguen sujetas a evidencia y posible ajuste con el docente.

## Decisiones de implementación del plan

Se proponen módulos Gradle `shared`, `androidApp`, `desktopApp` y `simulator`; contrato inicial limitado a estado, control temporal y expresión; archivos y pruebas según el plan. No representan código ya escrito. Los cambios menores durante T00 se registran y actualizan coherentemente; los cambios de alcance se discuten con el equipo.

## Por resolver en el arranque

- Integrantes responsables de A/B/C y acceso físico al prototipo.
- Segundo Android, versiones de sistemas y condiciones de red.
- Cuenta propietaria y visibilidad de GitHub; nombres de usuario para invitar colaboradores.
- Llegada del MAX98357A, controlador OLED real y mapa de pines.
- Versiones compatibles de herramientas después de la compilación mínima.
- Importe del presupuesto de ejecución y perfiles de proveedor antes de llamadas reales.

## Cómo registrar un cambio

Añadir fecha, ID, problema, decisión, alternativas relevantes, tareas afectadas y aprobación cuando cambie el alcance. Vincular evidencia si responde a una prueba. No reemplazar un fallo observado por una promesa ni reescribir acuerdos históricos para ocultar un recorte.

## Organización y plan aprobados posteriormente

Walther aprobó inicialmente el plan ejecutable y autorizó conectar y publicar en `WaltherMoraRivera/agentic-iot-pet`. Walther lidera con flexibilidad; Maximiliano colabora con foco en pruebas físicas; Winderson se enfoca en gráficos e interfaz. Los responsables nominales vigentes están en BACKLOG.md y los issues. OpenSpec organiza requisitos y escenarios; MoSCoW organiza prioridades. Las etiquetas Modelo/Esfuerzo describen capacidad y razonamiento, no horas.

## Decisiones del 27 de noviembre de 2026

| ID | Problema | Decisión | Alternativas descartadas | Afecta | Aprobación |
|---|---|---|---|---|---|
| D10 | La regla de cierre pedía revisión de otro integrante, mientras el diseño aprobado ya permitía que revisara un agente. Eso bloqueaba cerrar issues terminados, como #1 y #4. | La revisión de código puede hacerla un agente, registrada en el PR con sus hallazgos corregidos o derivados a un issue. Con evidencia y tasks.md al día, el líder cierra el issue. Las pruebas físicas siguen siendo necesarias donde el issue las pide. | Mantener la revisión humana obligatoria; confirmación humana breve en la junta. | Todos los issues; plantillas de PR e issue; `OPENSPEC_GITHUB.md`, `INICIO_EQUIPO.md`, README. Los «Criterios de cierre» de los issues creados antes de esta fecha se leen según D10. | Walther, 27-11-2026 |
| D11 | Los PR no nombraban su issue y no cerraban nada al unirse. | Primera línea del PR: `Closes #N` si completa el issue, o `Parte de #N` si es un avance parcial. | `Closes` en todo PR, que cerraría issues con pruebas físicas pendientes. | Plantilla de PR, `OPENSPEC_GITHUB.md`. | Walther, 27-11-2026 |
| D12 | T02 no podía implementarse sin fijar el emparejamiento (10 preguntas abiertas en `T02-casos.md`). | Se aprueba [`contracts/EMPAREJAMIENTO.md`](../../contracts/EMPAREJAMIENTO.md) v1: huella SHA-256 de la clave TLS fijada en la app; token portador por cliente; oferta de 60 s y toque de 15 s; máximo 4 clientes. El USB empareja solo al primer cliente o tras un restablecimiento (respeta D05). Autorrevocación sin toque; revocar a otro cliente exige el toque. | TLS mutuo; revocación de otros sin toque; USB que agrega clientes con clientes ya emparejados. | T02 (#5), T03 (#6), Q02 (#16); `T02-casos.md`. Pendiente de medición: ECDSA P-256, memoria para 4 clientes y gesto de restablecimiento (H01). | Walther, 27-11-2026 |

## Decisiones del 28 de noviembre de 2026

| ID | Problema | Decisión | Alternativas descartadas | Afecta | Aprobación |
|---|---|---|---|---|---|
| D13 | #28 e indicador 22 piden que Android sincronice un lote pendiente con un «destino externo local autorizado», y no había destino acordado. | El destino es la **app de Windows emparejada**, que actúa como réplica local. Android envía el lote por HTTPS en la red local a la app de Windows y esta lo guarda en su Room. La sincronización es idempotente: usa identificadores de evento y cursor confirmado, y un reintento no duplica. Se autentica con las identidades que las apps intercambian al emparejarse (diseño, sección 8, punto 4); la huella va fijada y el TLS no se desactiva. Sin nube, broker ni servicios de terceros. Antes de implementarla, la operación nueva se especifica en OpenSpec (`03-study-sync`). | Exportar el lote a un archivo e importarlo en Windows: es manual y no muestra reintento automático. Usar como destino el servidor de `simulator/`: es una herramienta de prueba y no un destino autorizado. Usar un NAS o un servicio externo: agrega infraestructura y está fuera del alcance de `AGENTS.md`. | #28, T05 (#9), T06 (#11), T02 (#5) para las identidades; OpenSpec `03-study-sync`; fila 22 de `EVA2.md`. Depende de T02 y del historial de T05. | Walther, 28-11-2026 |
| D14 | En #37 quedaron tres comportamientos de T04 sin definir: cómo terminar la sesión desde la placa, el `nvs_commit` dentro del ciclo de 10 ms y el toque soltado después de que vence una confirmación. [Propuesta de Maximiliano](https://github.com/wmrDev/agentic-iot-pet/issues/37#issuecomment-7230856354). | 1) Con la sesión **en pausa**, un toque breve termina la sesión (`study_finish`); en curso, el toque breve sigue sin efecto. 2) La persistencia sale del ciclo: una tarea de baja prioridad guarda el último estado pedido (cola de un valor con `xQueueOverwrite`) y la misma tarea servirá para las escrituras de D12. Se mide en la placa cuánto dura hoy `settings_set_session_marker`. Riesgo aceptado: un corte de energía entre pedir y escribir la marca deja la sesión como `idle` en lugar de `interrupted`. 3) Un toque que **empezó** con una confirmación pendiente le pertenece a esa confirmación: si se suelta tarde se ignora y nunca inicia estudio. Vale también para las confirmaciones de emparejamiento de D12. Ninguna confirmación pendiente termina la sesión. | Mantener pulsado para terminar (ya se usa para pedir la voz); triple toque; dejar `nvs_commit` en el ciclo sin medirlo; conservar el comportamiento actual del toque tardío. | T04 (#8), con tests Unity nuevos en `firmware/test`; `contracts/ESTUDIO_Y_EVENTOS.md`; T02 (#5) por el espacio de confirmación compartido. | Walther, 28-11-2026 |

La explicación de la terna sigue siendo necesaria para la rúbrica (indicador 13 y R02, tarea 2.3); se prepara en el ensayo, no como condición para cerrar cada issue.
