# Preparación del equipo para la junta del sábado 28-11

**Para:** Walther, Winderson y Maximiliano, y el agente (Claude, Codex u otro) de cada uno.
**Estado al viernes 27-11-2026:** `main` incluye los PR #27 a #44. Para cargar en la placa se usa la revisión que fija la [hoja de banco](2026-11-28-hoja-banco.md) (`1623ada`) o una posterior, siempre anotando el commit que se usó.

## Cómo usar este documento

Pasa este archivo completo a tu agente y dile: *«Soy <tu nombre>. Haz mi parte de este documento antes de la junta.»* El agente hace la sección de esa persona y las tareas comunes que le correspondan.

## Instrucciones para el agente

1. Trabaja en el repositorio [wmrDev/agentic-iot-pet](https://github.com/wmrDev/agentic-iot-pet). Antes de cambiar nada, lee [`AGENTS.md`](../../AGENTS.md), el [README](../../README.md) y [`docs/equipo/DECISIONES.md`](../equipo/DECISIONES.md), en especial D10, D11 y D12.
2. Actualiza `main` (`git fetch` y `git pull`) y crea **una rama por tarea**. Puedes subir esa rama y abrir el PR. No hagas commit ni push directo a `main` ni push forzado, y no reescribas el trabajo de otro integrante.
3. **Los PR los une solo la persona, nunca el agente.** Tampoco cierres issues ni publiques comentarios que no sean los del PR que abriste, salvo que tu integrante te lo pida expresamente.
4. **Primera línea de cada PR:** `Closes #N` solo si el PR cumple todos los criterios de cierre del issue; si es un avance parcial, `Parte de #N` (D11). La [plantilla de PR](../../.github/pull_request_template.md) ya trae ese formato. Ninguna tarea de esta guía cierra un issue: todas usan `Parte de`.
5. **Revisión (D10):** otro agente, distinto del que implementó, revisa el código. El resultado queda como comentario en el PR y sus hallazgos se corrigen o se derivan a un issue.
6. **Evidencia:** cada prueba registra commit, entorno y resultado observado con la [plantilla](../pruebas/PLANTILLA_RESULTADO.md), en `docs/pruebas/resultados/`. No inventes mediciones ni GPIO, y no presentes una compilación o una simulación como prueba física. Lo que no se ejecutó queda como `NO EJECUTADA` con su motivo.
7. Documentación en español; símbolos y mensajes de código en inglés. No subas credenciales, contraseñas Wi-Fi, fotos con datos personales ni documentos de la universidad.
8. Antes de terminar, informa a tu integrante qué hiciste, qué quedó sin hacer y el enlace del PR.

## Contexto común (leerlo todos)

- **Cerrados:** #1 T00 (base Android, Windows y firmware) y #4 T01 (contrato y autoridad). El CI compila Android, Windows, el simulador y los tres proyectos de firmware en cada PR.
- **En `main`:**
  - el firmware de diagnóstico USB y la lógica autónoma T04, con 61 tests de Unity;
  - el firmware de banco H01;
  - las fichas EVA_02;
  - la [matriz de evidencia de los 29 indicadores](EVA2.md#matriz-de-evidencia);
  - los [casos de T02](../pruebas/T02-casos.md);
  - el [contrato de emparejamiento](../../contracts/EMPAREJAMIENTO.md), aprobado (D12).
  Nada se ha ejecutado todavía en la placa.
- **Cadena crítica para la demo:** T02 #5 (emparejamiento) → T03 #6 (orden por HTTPS) → T05 #9 (historial) → #28 (reconexión). En paralelo, la placa tiene que quedar estable en el banco (#7).
- **Pendientes y decisiones abiertas:** [#37](https://github.com/wmrDev/agentic-iot-pet/issues/37).
- **Junta del sábado:** sigue la [hoja de banco](2026-11-28-hoja-banco.md), en este orden: 1) ESP32-S3 sola, 2) componentes uno a uno, 3) autonomía T04, 4) acuerdo de demo.

## Tareas comunes (los tres)

- [ ] Leer la [hoja de banco](2026-11-28-hoja-banco.md) y el [plan de la junta](2026-11-27-plan-junta-eva-02.md). *Hecho cuando:* puedes nombrar los cuatro pasos de la junta y qué te toca en cada uno.
- [ ] Preparar una explicación de unos 2 minutos sobre una parte del proyecto **ajena a tu foco**, para ensayar la rúbrica (indicador 13 y R02 #19). *Hecho cuando:* el tema elegido está anotado en un comentario de [#30](https://github.com/wmrDev/agentic-iot-pet/issues/30). Lo publica la persona, no el agente.
- [ ] Revisar en la [matriz](EVA2.md#matriz-de-evidencia) las filas que te tocan. *Hecho cuando:* en ese mismo comentario de #30 anotaste qué evidencia puedes aportar y para cuándo.

## Winderson · interfaz, Galaxy y hardware

Tienes la placa. **Llévala el sábado con su cable USB-C de datos.** Si tienes un adaptador USB-UART, llévalo también; el inventario no lo registra, así que queda por confirmar.

- [ ] **APK en el Galaxy S24 Ultra** (ficha E01, [#29](https://github.com/wmrDev/agentic-iot-pet/issues/29)).
  - Compilar desde `main` con `.\gradlew.bat :androidApp:assembleDebug` y anotar el commit.
  - Anotar el SHA-256 de `apps/androidApp/build/outputs/apk/debug/androidApp-debug.apk`.
  - Instalarla y abrirla.
  - Registrar el modelo, la versión de Android, el nivel de API, la arquitectura y la captura de la pantalla de bienvenida en `docs/pruebas/resultados/2026-11-28-E01-galaxy.md` (usar la fecha real de la prueba).
  - PR con `Parte de #29`.
  - *Hecho cuando:* el registro muestra la app abierta en el Galaxy, con el commit y el hash.
- [ ] **Boceto de la cara V01** ([#3](https://github.com/wmrDev/agentic-iot-pet/issues/3), con la [revisión de Walther](https://github.com/wmrDev/agentic-iot-pet/issues/3#issuecomment-7224755637)). Ajustar en la rama `feat/V01-cara`:
  1. «Enamorada» ya no se activa con caricia larga, porque en T04 mantener el toque pide la voz.
  2. El toque breve inicia el estudio, así que su reacción visible pasa a ser «Concentrada».
  3. Agregar los estados *esperando confirmación* (15 s con cuenta atrás), *estudio en pausa* y *sesión interrumpida*.
  4. «Capturando» solo con la voz bajo demanda, nunca con la medición de ruido.

  Agrega también el mockup de las pantallas de la app de **conexión, desconexión y confirmación**. Abre el PR desde `feat/V01-cara` con `Parte de #3`.
  *Hecho cuando:* el PR tiene la imagen o el HTML actualizado y el documento de estados.
- [ ] *(Opcional)* **Guía de instalación** ([R01 #18](https://github.com/wmrDev/agentic-iot-pet/issues/18)): borrador de los pasos para instalar la APK, abrir la app de Windows y cargar el firmware, tomando como base [`ENTORNOS.md`](../equipo/ENTORNOS.md) y el CI. PR con `Parte de #18`. *Hecho cuando:* otra persona podría seguir el borrador sin preguntar.

## Maximiliano · banco, pruebas físicas y consumo

- [ ] **Confirmar la hoja de banco** ([#7](https://github.com/wmrDev/agentic-iot-pet/issues/7)). Revisa el orden, los comandos y las fichas que se pueden ejecutar el sábado (E01, E12 y parte de E10). *Hecho cuando:* la hoja no tiene cambios pendientes, o hay un PR con `Parte de #7` que los corrige.
- [ ] **Inventario real.** Confirma qué llegó: OLED (¿SH1106 o SSD1306?), MAX98357A y parlante, INMP441, TTP223, LED con resistencia, protoboard y cables. Confirma también qué instrumentos hay: multímetro, medidor USB en línea y adaptador USB-UART.
  - *Hecho cuando:* [`docs/hardware/INVENTARIO.md`](../hardware/INVENTARIO.md) dice qué hay y qué falta, en un PR con `Parte de #29`.
  - Si no hay instrumento de medición, el consumo queda como `NO EJECUTADA` con el motivo.
- [ ] **Entorno listo en tu PC.** Activar ESP-IDF v5.5.6 y compilar en `main` los tres proyectos: `firmware`, `firmware/test` y `firmware/diagnostics/h01`.
  - En Windows, el Python 3.11 de `C:\Espressif\tools\idf-python` tiene que ir primero en el PATH antes de `export.ps1`, o se puede usar el perfil de EIM; ver `ENTORNOS.md`.
  - Si GCC falla de forma intermitente con un segfault, compilar con `CMAKE_BUILD_PARALLEL_LEVEL=1`.
  - *Hecho cuando:* las tres compilaciones están registradas, con commit y resultado, en `docs/pruebas/resultados/`, en un PR con `Parte de #7`.
- [ ] **Decisiones de T04** ([#37](https://github.com/wmrDev/agentic-iot-pet/issues/37)). Prepara una propuesta para tres puntos:
  1. un gesto para terminar la sesión desde la placa;
  2. si se acepta el `nvs_commit` dentro del ciclo de 10 ms o se mueve;
  3. qué pasa con un toque soltado después de que vence la confirmación.
  Ten en cuenta que, según D12, el emparejamiento comparte con T04 el único espacio de confirmación física.
  *Hecho cuando:* hay una opción recomendada para cada punto, escrita en un comentario de #37 que publica la persona.

## Walther · T02, coordinación e informe

- [ ] **T02, paso 2** ([#5](https://github.com/wmrDev/agentic-iot-pet/issues/5)): escribir en `PairingTest.kt` las pruebas de los [casos](../pruebas/T02-casos.md) marcados como compartidos, en el destino `:shared:desktopTest` y con la ruta que indica ese documento. Ejecutar `.\gradlew.bat :shared:desktopTest` y ver que **fallan** antes de implementar. *Hecho cuando:* el PR con `Parte de #5` incluye la salida de los tests en rojo.
- [ ] **#28:** decidir el «destino externo local autorizado» para la sincronización. *Hecho cuando:* queda una entrada D13 en `DECISIONES.md`, en un PR con `Parte de #28`.
- [ ] **Junta (acción humana):** llevar la hoja de banco impresa o abierta, moderar el acuerdo de demo (punto 4) y registrar en #30 quién explica cada parte. *Hecho cuando:* el reparto de la explicación está en #30.

## Qué traer el sábado

| Quién | Qué |
|---|---|
| Winderson | Placa ESP32-S3, cable USB-C de datos, módulos disponibles y el Galaxy S24 Ultra |
| Maximiliano | PC con ESP-IDF listo, multímetro u otro instrumento de medición, y el iPhone 14 para documentar |
| Walther | PC con Android Studio y ESP-IDF, el INFINIX y la hoja de banco |
