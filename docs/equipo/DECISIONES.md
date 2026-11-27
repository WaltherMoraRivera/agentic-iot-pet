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

La explicación de la terna sigue siendo necesaria para la rúbrica (indicador 13 y R02, tarea 2.3); se prepara en el ensayo, no como condición para cerrar cada issue.
