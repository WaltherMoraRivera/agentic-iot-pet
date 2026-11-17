# Pet agéntico IoT

Asistente físico de escritorio con expresiones, contacto, detección de ruido, sonidos y acompañamiento de estudio. Android es su centro por defecto; Windows puede tomar el control. La IA por API es opcional y tiene presupuesto configurable.

**Equipo:** Walther, Maximiliano y Winderson. **Entrega objetivo:** 16 de diciembre de 2026.

## Estado real

El diseño inicial fue aprobado el **17 de noviembre de 2026**. Esta base contiene documentación, plan, contratos iniciales y plantillas de colaboración. **Todavía no hay firmware, apps, simulador ni compilación automática implementados.** Los comandos de compilación del plan corresponden a archivos que se crearán en T00.

Repositorio compartido: https://github.com/WaltherMoraRivera/agentic-iot-pet. La preparación de OpenSpec y GitHub no implementa funciones de producto ni implica llamadas de IA pagadas.

## Empieza aquí

1. Lee la [guía del equipo](docs/equipo/INICIO_EQUIPO.md) y asignen los roles A/B/C.
2. Consulta el [diseño aprobado](docs/superpowers/specs/2026-11-17-diseno-software.md).
3. Revisa el [plan por entregas](docs/superpowers/plans/2026-11-17-desarrollo-integrado.md). La primera tarea es T00; T00-V comprueba pronto los riesgos de voz.
4. Registra el entorno siguiendo [ENTORNOS.md](docs/equipo/ENTORNOS.md).
5. Abre Claude o Codex en esta carpeta y asigna **una tarea identificada**, con sus límites y verificación.

## Documentación canónica

| Documento | Contenido |
|---|---|
| [Plan ejecutable](docs/superpowers/plans/2026-11-17-desarrollo-integrado.md) | Dependencias, tareas, interfaces, archivos y pruebas |
| [Contrato inicial](contracts/PROTOCOLO_INICIAL.md) | HTTPS, identidad, control y primera orden de expresión |
| [Guía del equipo](docs/equipo/INICIO_EQUIPO.md) | Roles, ramas, revisión y uso de agentes |
| [Entornos](docs/equipo/ENTORNOS.md) | Inventario, instalaciones y verificaciones |
| [Decisiones](docs/equipo/DECISIONES.md) | Acuerdos y asuntos aún por resolver |
| [Inventario de hardware](docs/hardware/INVENTARIO.md) | Componentes y validación física |
| [Matriz de pruebas](docs/pruebas/MATRIZ.md) | P01–P19, pruebas adicionales y trazabilidad |
| [Guion de entrega](docs/entrega/GUION.md) | Demostración y preparación de evidencias |

## Organización

Un repositorio, una rama principal integrable y una rama por tarea. Cada integrante trabaja en su clon, con revisión por otra persona. Placa, aplicación y prueba forman parte de la misma entrega.

`AGENTS.md` contiene las reglas comunes; `CLAUDE.md` las importa. No necesitan instalar las habilidades particulares del autor del plan: las tareas incluyen sus pasos y verificaciones.

El firmware y los proyectos Gradle se crearán al ejecutar T00 y las siguientes tareas. No se añaden carpetas vacías para aparentar implementación.

## Condiciones de funcionamiento

- Android y Windows son los destinos iniciales. Linux permite desarrollar componentes compatibles; la app Linux completa no está comprometida.
- La placa funciona localmente sin internet. El chat de lenguaje libre necesita un proveedor habilitado.
- La voz usa el micrófono del teléfono o computador; el INMP441 solo mide intensidad local.
- El parlante reproducirá sonidos breves mediante MAX98357A. La voz sintetizada es alcance ampliado.
- Un relevo abrupto mantiene el control local, pero pausa nuevas llamadas pagadas hasta conciliar el gasto.
- Pantalla, memoria, pines y coexistencia de micrófonos requieren pruebas reales.

Las limitaciones se registran; un resultado simulado nunca se presenta como prueba en hardware.

## Planificación compartida

[OpenSpec, responsabilidades, etiquetas y uso de GitHub](docs/equipo/OPENSPEC_GITHUB.md). Los issues y milestones se derivan de los cambios OpenSpec; el alcance opcional queda separado.

[Backlog publicado con responsables](docs/equipo/BACKLOG.md) · [Issues](https://github.com/WaltherMoraRivera/agentic-iot-pet/issues) · [Milestones](https://github.com/WaltherMoraRivera/agentic-iot-pet/milestones).
