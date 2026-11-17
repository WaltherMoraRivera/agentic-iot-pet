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
