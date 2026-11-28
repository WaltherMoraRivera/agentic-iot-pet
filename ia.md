# Registro de uso de IA

Qué se pidió y qué se corrigió. No incluir claves, datos privados ni conversaciones completas.

## 27-11-2026 · V01, issue #3 (Claude Code)

**Pedido:** implementar la especificación aprobada de V01 como mockup HTML local navegable (conexión, estudio, chat, estado y flujo EVA02 con login, lecturas, historial, umbral y aviso, con datos simulados), reutilizando la cara monocroma de 128×64; primero reproducir en Chromium que faltaban los recorridos, luego implementar y verificar cuatro escenarios. Sin red, hardware ni instalaciones.

**Ajuste pedido:** suavizar el ceño de la cara «Concentrada».

**Cambios resultantes:**

- El ceño pasó de caer 9 px a 3 px, en párpados y cejas.
- El render de la cara se movió a `docs/diseno/cara.js` para que el boceto y el mockup usen el mismo. Ahora `cara-mascota.html` necesita ese archivo al lado.

**Correcciones durante el trabajo (detectadas por el agente):**

- A 320 px las líneas de estado con icono se medían solapadas; se alinearon con flex.
- Al cambiar de pantalla los avisos podían quedar fuera de vista; ahora la vista se sitúa en la navegación.
- Un reintento de enlace fallido tras un corte dejaba de indicar que los datos eran antiguos.

**Evidencia:** `docs/pruebas/resultados/2026-11-27-V01-mockup.md`.

**Pendiente de personas:** revisión de Walther, prueba en el OLED real y actualizar la nota «descargar y abrir» de `docs/superpowers/specs/2026-11-25-cara-mascota-design.md`.
