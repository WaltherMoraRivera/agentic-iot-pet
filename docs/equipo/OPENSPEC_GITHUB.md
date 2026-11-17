# OpenSpec, GitHub y prioridades

Plan aprobado inicialmente por Walther. Walther lidera e integra de forma flexible; Maximiliano colabora con foco en montaje y pruebas físicas; Winderson lidera lo gráfico y visual. No son silos exclusivos. Se requiere revisor distinto del implementador.

## Fuente de verdad

OpenSpec contiene cambios propuestos en `openspec/changes/`, con proposal, design, delta specs y tareas pendientes. El diseño y plan originales siguen como referencia detallada. `openspec/specs/` se consolida al implementar y archivar, no se llena con funciones ficticiamente terminadas. Los issues enlazan cada requisito y el hito correspondiente.

OpenSpec 2.1.0 se usa sin instalación global: `npx --yes @fission-ai/openspec@2.1.0 validate --all --strict --no-interactive`. En PowerShell usar `npx.cmd`. Se inicializó con `--tools none`: Claude y Codex pueden seguir estos archivos y la CLI; no se instalaron slash commands ni se modificaron preferencias globales.

## MoSCoW

- Must: base comprometida y verificaciones necesarias; no recortar sin acuerdo docente.
- Should: mejora importante si queda margen; no bloquea entrega base.
- Could: ampliación opcional sin fecha comprometida.
- Won't: fuera de esta entrega; no ejecutar por estar en el backlog.

## Modelos y esfuerzo

`Modelo: low/medium/high` indica capacidad recomendada, no un ID comercial. `Esfuerzo: medium/high/xhigh` indica razonamiento, no horas de trabajo. Las recomendaciones son juicio de asignación, no garantías. Low: Claude Haiku 5.5 / GPT-6 Luna; medium: Claude Sonnet 5.5 / GPT-6.1 Sol; high: Claude Opus 5.5 / GPT-6 Astra. Para xhigh de Codex, usar high en Claude si el cliente no ofrece equivalente. Comprobar disponibilidad de cada cuenta antes de ejecutar, sin contratar APIs por este documento.

Fuentes consultadas: [Claude](https://platform.claude.com/docs/en/models/overview), [Codex](https://developers.openai.com/codex/models), [OpenSpec](https://github.com/Fission-AI/OpenSpec). La selección concreta puede actualizarse sin cambiar requisitos.

## Cierre de un issue

Vincular PR, evidencia y escenario verificado; actualizar tasks.md y revisar con otro integrante. Archivar un cambio OpenSpec solo cuando esté completo. Un validador estructural aprobado no demuestra funcionamiento del software. Los hitos opcionales no tienen fecha y no amplían la entrega obligatoria.

## Cuentas del equipo

Solo WaltherMoraRivera figura actualmente como colaborador. Maximiliano y Winderson quedan identificados nominalmente en los issues; se asignarán sus cuentas cuando se conozcan y tengan acceso. No se inventan usuarios ni se envían invitaciones sin identificarlos.
