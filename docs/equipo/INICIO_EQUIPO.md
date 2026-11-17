> Actualización: plan aprobado inicialmente. Walther lidera e integra, Maximiliano prioriza pruebas físicas y Winderson lo visual. La tabla A/B/C original queda como referencia funcional, no como asignación vigente. Ver OPENSPEC_GITHUB.md para el reparto actual.

# Inicio y colaboración del equipo

## Primera reunión: 45 minutos

1. **10 min:** leer el README y comprobar que todos entienden qué funciona sin IA y qué necesita proveedor.
2. **10 min:** asignar A/B/C, revisor y responsable de integrar la primera entrega; registrar quién tendrá la placa.
3. **10 min:** registrar sistemas operativos, versión de Android, segundo teléfono y disponibilidad de componentes.
4. **10 min:** revisar T00, T00-V y T01 del [plan](../superpowers/plans/2026-11-17-desarrollo-integrado.md); repartirlas respetando dependencias.
5. **5 min:** acordar la siguiente integración y la cuenta/visibilidad del repositorio GitHub.

## Responsabilidades

| Rol | Trabajo principal | Integrante | Revisor |
|---|---|---|---|
| A | Dispositivo, firmware, montaje, recursos y audio | Por asignar | B |
| B | Aplicaciones, interfaz, datos, emparejamiento y sincronización | Por asignar | C |
| C | Proveedores, herramientas, hooks y automatización de pruebas | Por asignar | A |

Walther y Maximiliano cuentan con Windows; Winderson dispone de Linux y Windows. La experiencia indicada es superficial en móvil/electrónica y mayor en documentación/pruebas. Nadie queda dedicado solo a documentación. B y C comparten el trabajo de voz; los tres participan en la prueba física semanal.

Las tareas nuevas se asignan por responsable humano, no por modelo de IA. Si falta experiencia, el agente debe explicar el cambio y proponer una prueba pequeña que el integrante pueda ejecutar.

## Ritmo semanal

De las nueve horas compartidas, reservar aproximadamente dos para integración/revisión y una para evidencias y explicación. El resto se distribuye en trabajo de construcción y diagnóstico. Es una pauta, no horas adicionales a las disponibles.

Cada integración empieza desde la versión común y termina con una demostración observable. No se integran tres áreas completas el último día. El responsable de integración rota por entrega.

## GitHub y ramas

Antes de publicar, el equipo decide propietario y visibilidad; se recomienda privado durante el desarrollo si hay material académico. No publicar PDF docentes ni secretos. Remoto autorizado: https://github.com/WaltherMoraRivera/agentic-iot-pet.

Para iniciar Git localmente, desde la raíz y solo si todavía no existe `.git`:

```text
git init -b main
git status --short
git add README.md LEEME.md AGENTS.md CLAUDE.md .gitignore .gitattributes .github docs contracts
git diff --cached --stat
git diff --cached
git commit -m "docs: preparar bases del proyecto IoT"
```

Configurar nombre y correo propios si Git los solicita; no adoptar la identidad de un compañero. Crear después el repositorio vacío en la cuenta elegida y seguir sus instrucciones para conectar `origin`. No copiar una URL de ejemplo como si fuera el destino real.

Cada compañero clona ese remoto en su equipo. Una tarea usa una rama como `feat/T01-contract` o `docs/T00-entorno`. Antes de iniciar, actualizar `main` y crear la rama. Antes de incorporar cambios ajenos, conservar el trabajo propio mediante commit o una operación revisada; no borrar cambios para salir de un conflicto.

Cada pull request indica problema, resultado, tarea, verificaciones y riesgos. Otra persona revisa y solicita correcciones si hace falta. Se propone integrar con squash para dejar un commit por tarea terminada. Proteger `main` contra cambios directos cuando las opciones de la cuenta lo permitan.

Las plantillas locales están en `.github/ISSUE_TEMPLATE/` y `.github/pull_request_template.md`; estarán disponibles en GitHub al publicar el repositorio. No hay aún automatización de compilación: T00 añadirá las comprobaciones sobre comandos reales.

## Trabajo con agentes

Abrir la raíz del repositorio. Codex usa la guía `AGENTS.md`; `CLAUDE.md` importa esa misma guía para Claude Code. Las interfaces de chat que no leen el disco requieren adjuntar o pegar los documentos relevantes y devolver los cambios al repositorio. La memoria de un chat no es el registro de decisiones del equipo. [Guía oficial de Codex](https://developers.openai.com/codex/guides/agents-md), [memoria e importaciones de Claude Code](https://code.claude.com/docs/en/memory).

Encargo inicial sugerido para un integrante:

```text
Trabajamos en el proyecto Pet agéntico IoT. Lee AGENTS.md, README.md,
el diseño aprobado y la tarea T00 del plan. Mi rol es B.
Comprueba el estado local y realiza únicamente T00 en mi rama.
Conserva trabajo existente, registra las versiones realmente probadas
y no hagas llamadas pagadas ni publiques cambios.
Entrega archivos modificados, comandos ejecutados y resultados observados.
Si una dependencia de otro integrante bloquea algo, explica qué falta.
```

Para revisar trabajo de otro integrante:

```text
Revisa los cambios de la tarea indicada frente al contrato y al diseño.
Comprueba consumidores, pruebas y límites, especialmente autoridad,
reintentos, datos sensibles y fallos de conexión.
No modifiques archivos ni des por aprobada una prueba que no se ejecutó.
Informa problemas concretos y su impacto; omite preferencias de estilo.
```

Para cerrar una sesión:

```text
Resume qué quedó implementado y qué se comprobó. Indica archivos y comandos,
riesgos, pendientes y siguiente tarea elegible. Actualiza la evidencia
correspondiente sin cambiar resultados no observados ni el diseño aprobado.
```

No se necesitan las habilidades `superpowers` de este entorno en los otros equipos: el plan contiene la secuencia y las comprobaciones. Si están disponibles, pueden ayudar a ejecutar por tareas; no son dependencias del producto.

## Definición de tarea terminada

- Cumple el comportamiento y criterios de la tarea y sus consumidores.
- Pasa sus comprobaciones automáticas; la evidencia manual pendiente está diferenciada.
- Conserva seguridad, datos y compatibilidad acordadas.
- Otra persona revisó el cambio y el responsable humano lo puede explicar.
- La documentación refleja el estado real y el commit queda identificado.

Un agente que termina de escribir no convierte por sí solo una tarea en terminada.
