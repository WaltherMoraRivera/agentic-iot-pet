# Matriz de pruebas y evidencias

Estado inicial: **todas las pruebas de producto están NO EJECUTADAS**. Esta tabla organiza trabajo; no registra aprobaciones anticipadas. Crear un registro por ejecución con la [plantilla](PLANTILLA_RESULTADO.md).

El subconjunto de la Evaluación 2 tiene fichas propias en [eva02/](eva02/README.md); sus resultados se registran igual, con la plantilla.

Estados admitidos: `NO EJECUTADA`, `FALLÓ`, `PASÓ EN SIMULADOR`, `PASÓ EN HARDWARE`, `LIMITACIÓN ACORDADA`. Un mock de micrófono o proveedor nunca equivale a una llamada o captura real.

| ID | Escenario y procedimiento | Resultado exigido | Tarea / responsable |
|---|---|---|---|
| P01 | Encender/reiniciar placa y comprobar memoria, display y periféricos | Inicio estable; identificación y configuración real documentadas | T00/T03/T04; A |
| P02 | Contacto, doble toque, mantener y ruido controlado | Reacción correcta sin bloquear pantalla ni disparar varios gestos | T04; A |
| P03 | Cambiar umbral y aviso desde Android y Windows | Rango validado, confirmación física cuando corresponde y estado aplicado visible | T03/T04; A/B |
| P04 | Contraseña incorrecta, cliente ajeno, QR vencido y credencial revocada | Acceso/órdenes rechazados; no filtrar estado ni secretos | T02/T05; B/C |
| P05 | Mensajes alterados, repetidos, antiguos, con otro emisor y caché expulsada | Sin efectos inválidos ni duplicados; un solo centro | T01/T03; A/C |
| P06 | Abrir Windows con Android como centro | Relevo automático autenticado, configuración/eventos conservados | T06; B/C |
| P07 | Cerrar, suspender o aislar Windows; reabrirlo después | Android recupera control; órdenes viejas rechazadas; pausa contable si salida abrupta | T06/T07; B/C |
| P08 | Mantener sensor, hablar, soltar; repetir con botón/atajo y micrófono delegado | Captura indicada, finalizada al soltar y tarea aplicada una vez | T08; todos |
| P09 | Iniciar llamada/grabación antes de solicitar voz | No interferir con ella; ofrecer alternativa móvil/texto; registrar apps y SO probados | T00-V/T08; B/C |
| P10 | Iniciar otra captura durante dictado | Cancelar, liberar recurso y descartar audio; medir latencia y límites reales | T00-V/T08; B/C |
| P11 | Solicitar sesión/nota/recordatorio por texto y voz en ambas apps | Mismo validador y herramientas; sin IA, controles manuales y aviso claro | T07/T08; B/C |
| P12 | Solicitar cambio protegido; esperar sin tocar, tocar a tiempo, repetir toque y cambiar centro | Solo confirmación vigente de la operación actual produce un efecto | T04/T07; A/C |
| P13 | Cambiar perfil y provocar error/límite/timeout/respuesta inválida | Perfil seleccionable sin recompilar, respaldo conforme a política sin repetir herramientas; incertidumbre de coste preservada | T07; C |
| P14 | Presupuesto bajo, dos apps, llamada incierta y relevo abrupto con copia contable vacía | Tope estimado compartido, reserva previa y pausa conservadora; continúa lo local | T07; B/C |
| P15 | Cortar internet durante estudio y deshabilitar IA | Estudio, personaje y control local continúan | T04/T07; todos |
| P16 | Cortar enlace placa-app cinco minutos, reconectar y repetir páginas; probar overflow/reinicio aparte | Al menos 300 muestras recuperadas sin duplicados en corte con placa alimentada; brechas reales visibles | T04/T05; A/B |
| P17 | Fin de turno y solicitud de permiso reales en Claude Code; receptor caído/duplicados | Aviso temporal sin aprobar permisos ni bloquear el agente | T09; C |
| P18 | Android bloqueado 30 min con ahorro de batería, repetir en segundo teléfono; medir placa | Servicio activo o recuperación documentada, consumo real registrado y sin reinicios | T06/T10; todos |
| P19 | Revisar repo, datos, logs y contenido enviado a proveedor; borrar historial | Sin claves expuestas ni audio/video persistentes; envío mínimo y borrado sincronizado | T02/T05/T07/T08/T10; todos |

## Pruebas adicionales por decisiones posteriores

| ID | Procedimiento | Resultado exigido | Tarea |
|---|---|---|---|
| A01 | Reproducir avisos con MAX98357A, activar silencio y repetir con sensor acústico activo | Audio estable, silencio respetado y sin bucle de reacciones propias | T04 |
| A02 | Ejecutar contrato contra simulador y placa; probar certificado equivocado | Igual semántica de mensajes; TLS nunca omitido | T03 |
| A03 | Cambiar fecha/hora civil; borrar nota y provocar corte antes de sincronizar | Permiso/estudio no se alargan; nota no reaparece | T01/T04/T05 |
| A04 | Compilar e instalar desde clon limpio con versiones documentadas | Artefactos reproducibles y manual suficiente para otro integrante | T10 |

## Metas y condiciones

- Telemetría: una muestra por segundo; distinguir muestras periódicas y eventos.
- Confirmación de comando: meta inferior a un segundo **con enlace establecido**; medir conexión TLS inicial aparte.
- Permiso: seis segundos, renovado cada dos; medir el tiempo total de recuperación incluyendo descubrimiento y red.
- Voz: hasta 15 segundos por captura; no declarar «cancelación inmediata» sin medir condiciones y latencia.
- Desconexión P16 mantiene alimentación de la placa; pérdida de energía se prueba como escenario diferente.

## Trazabilidad académica

La referencia inicial es la sección 12 de la propuesta docente v3. Antes de entregar, T10 cotejará los 29 indicadores con la rúbrica original y completará una tabla de indicador, requisito, evidencia y resultado. No se han comprobado aquí esos 29 indicadores contra su fuente independiente.

Las limitaciones acordadas con el profesor deben tener fecha, descripción y aprobación documentada; no equivalen a «pasó».
