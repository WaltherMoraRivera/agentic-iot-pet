# Corte EVA_02 de las pruebas físicas

Preparación del issue [#29 EVA02-02](https://github.com/wmrDev/agentic-iot-pet/issues/29), que es el corte EVA_02 de [#15 Q01](https://github.com/wmrDev/agentic-iot-pet/issues/15), [#16 Q02](https://github.com/wmrDev/agentic-iot-pet/issues/16) y [#17 V03](https://github.com/wmrDev/agentic-iot-pet/issues/17).

**Estado: todas las fichas están NO EJECUTADAS.** Estos documentos fijan procedimiento, datos y evidencia antes de probar; no registran resultados. Responsable nominal: Maximiliano. Revisor: Winderson o Walther, distinto de quien ejecute cada caso.

## Documentos

| Documento | Contenido |
|---|---|
| [FICHAS.md](FICHAS.md) | Casos E01–E12: objetivo, dependencia, pasos, esperado, casos negativos y evidencia |
| [DISPOSITIVOS.md](DISPOSITIVOS.md) | Inventario de los dos Android, Windows y placa, con cómo obtener cada dato |
| [CONSUMO.md](CONSUMO.md) | Plan de medición de consumo de la placa y del teléfono |

Cada ejecución genera un registro propio en `docs/pruebas/resultados/` con la [plantilla](../PLANTILLA_RESULTADO.md), nombrado `AAAA-MM-DD-EVA02-Exx-<dispositivo>.md`. La ficha no se edita para anotar resultados; solo se corrige su procedimiento si cambia, explicando el motivo.

## Trazabilidad con la aceptación de #29

| Criterio de #29 | Fichas | Matriz | Depende de |
|---|---|---|---|
| Misma revisión de APK y firmware en ambos Android; SO, API, arquitectura, permisos e instalación | E01 | A04 parcial | #1, #18 |
| Login, lectura actual y guardada, umbral, toque de confirmación, aviso manual y retorno automático | E02–E06 | P02 parcial, P03, P04, P12; la lectura guardada (E03) no tiene ID propio | #5, #6, #8, #9, #10 |
| Correlación de solicitudes y respuestas; rechazo de cliente no autorizado, TLS incorrecto o mensaje inválido sin aviso | E07, E08 | P04, P05, A02 | #5, #6, #16 |
| Corte con placa alimentada, autonomía, reconexión y recuperación sin duplicados | E09 | P16; P15 parcial solo si el corte deja sin internet | #8, #9, #28 |
| Consumo de placa y teléfono con condiciones e instrumentos | E10, E11 | P18 parcial | #7, #8, #11 |
| Banco de la placa e identificación del prototipo | E12 | P01 parcial | #7 |

## Reglas de evidencia

- **Tres registros distintos.** Una prueba de banco (placa sola), una simulada (simulador o proveedor simulado) y una integrada (Android con la placa real) nunca comparten registro ni estado. Un caso integrado solo puede quedar `PASÓ EN HARDWARE` si se ejecutó con la ESP32-S3 real.
- **Misma revisión.** Los casos E02–E11 en INFINIX y Galaxy usan la misma APK (SHA-256 registrado) y el mismo firmware (commit y hash de imagen). Si cambia uno, se repiten los casos afectados en ambos teléfonos.
- **Datos sintéticos.** Usuario, contraseña y nombre de dispositivo de prueba se generan para el ensayo y no se copian en las fichas ni en capturas. Recortar o tapar contraseñas, tokens, números de serie, IMEI y notificaciones personales.
- **Sin privacidad ajena.** Fotos del montaje sin rostros ni pantallas de otras personas. El iPhone 14 solo documenta el ensayo (foto o vídeo); no es un dispositivo bajo prueba.
- **No ejecutada no es aprobada.** Si falta la función de la que depende un caso, el registro dice `NO EJECUTADA` y el motivo. Una limitación solo cuenta como `LIMITACIÓN ACORDADA` con fecha y aprobación documentadas.
- **Consolidación.** Los resultados se resumen en #29 y se integran a la entrega en [#30](https://github.com/wmrDev/agentic-iot-pet/issues/30).
