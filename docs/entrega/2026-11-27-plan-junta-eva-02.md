# EVA_02 · Avances y plan para la junta

**Estado al 27 de noviembre de 2026. Junta del 28 de noviembre.**

La prioridad de la junta es estabilizar el USB del ESP32-S3 y verificar los componentes en banco. Las revisiones de software, el mockup y la preparación del informe pueden avanzar por separado antes de reunirse.

## Avances verificados

| Frente | Evidencia y pendientes |
| --- | --- |
| **T00 · Aplicaciones** | Winderson reprodujo la compilación desde un clon limpio en Windows y CachyOS, generó la APK y abrió la app de escritorio. El issue de reproducción se cerró. La revisión formal del PR sigue pendiente. [Resultado del clon limpio](https://github.com/wmrDev/agentic-iot-pet/issues/32#issuecomment-7206655317) · [PR #31](https://github.com/wmrDev/agentic-iot-pet/pull/31) |
| **T01 · Contrato** | Pasaron las 18 pruebas locales de autoridad, validación JSON y ejemplos del contrato. Winderson tiene la revisión solicitada y GitHub aún no registra su respuesta. El PR usa la rama de aplicaciones de T00 como base. [PR #33](https://github.com/wmrDev/agentic-iot-pet/pull/33) |
| **H01 · Banco** | Winderson identificó la placa y registró lecturas preliminares del TTP223 y el LED. Maximiliano compiló el diagnóstico de banco. Quedan pendientes la carga en placa y la verificación de pines, OLED, micrófono y amplificador. El último registro de USB indica desconexiones cada 2,5 s en arranque normal. [Reporte H01](https://github.com/wmrDev/agentic-iot-pet/issues/7#issuecomment-7180305182) · [Diagnóstico USB #27](https://github.com/wmrDev/agentic-iot-pet/pull/27) · [Compilación de banco #35](https://github.com/wmrDev/agentic-iot-pet/pull/35#issuecomment-7208079182) |
| **T04 · Autonomía** | Maximiliano compiló dos artefactos: el firmware con la lógica autónoma y la aplicación con 49 pruebas Unity. La ejecución en placa está pendiente. [Compilación de T04 #36](https://github.com/wmrDev/agentic-iot-pet/pull/36#issuecomment-7208524681) |
| **Diseño y EVA_02** | Winderson tiene un primer boceto animado de la cara. Maximiliano preparó 12 fichas, el inventario de dispositivos y el método de consumo. Las fichas están por ejecutar y falta el borrador de la matriz de 29 indicadores del informe. [Boceto V01](https://github.com/wmrDev/agentic-iot-pet/issues/3#issuecomment-7180532036) · [Fichas #34](https://github.com/wmrDev/agentic-iot-pet/pull/34) · [Informe #30](https://github.com/wmrDev/agentic-iot-pet/issues/30) |

## Tareas del 27 de noviembre

### Walther

- [ ] Revisar el PR de fichas y el boceto de la cara. [PR #34](https://github.com/wmrDev/agentic-iot-pet/pull/34) · [Boceto V01](https://github.com/wmrDev/agentic-iot-pet/issues/3#issuecomment-7180532036)
- [ ] Empezar la matriz del informe con una fila por indicador. Registrar la equivalencia en el Pet, la evidencia disponible y la prueba pendiente. [Informe #30](https://github.com/wmrDev/agentic-iot-pet/issues/30)
- [ ] Preparar los casos de T02 que pueden revisarse sin placa. Incluir oferta vencida, oferta reutilizada, huella incorrecta, credencial revocada y ausencia de confirmación física. [T02 #5](https://github.com/wmrDev/agentic-iot-pet/issues/5)

### Winderson

- [ ] Registrar la revisión de T00 y reproducir las 18 pruebas de T01. Anotar el commit, el entorno y el resultado en el PR correspondiente. [T00 #31](https://github.com/wmrDev/agentic-iot-pet/pull/31) · [T01 #33](https://github.com/wmrDev/agentic-iot-pet/pull/33)
- [ ] Comprobar el arranque en el Galaxy S24 Ultra con la misma APK de T00 usada en el INFINIX. Registrar la revisión, el hash de la APK y la versión de Android como evidencia parcial de compatibilidad. [Fichas de dispositivos #34](https://github.com/wmrDev/agentic-iot-pet/pull/34)
- [ ] Adelantar el mockup de conexión, desconexión y confirmación. [V01 #3](https://github.com/wmrDev/agentic-iot-pet/issues/3)

### Maximiliano

- [ ] Identificar las revisiones de H01 y T04 que se llevarán al banco y ordenar las fichas para la primera ronda. [H01 #35](https://github.com/wmrDev/agentic-iot-pet/pull/35) · [T04 #36](https://github.com/wmrDev/agentic-iot-pet/pull/36) · [Fichas #34](https://github.com/wmrDev/agentic-iot-pet/pull/34)
- [ ] Confirmar los módulos y los instrumentos disponibles. Si falta el instrumento de medición, dejar el consumo como pendiente. [Validación física #29](https://github.com/wmrDev/agentic-iot-pet/issues/29)

## Orden de la junta del 28 de noviembre

1. **ESP32-S3 solo.** Confirmar el puerto y respaldar el firmware original si sigue en la placa. Comprobar el arranque estable del diagnóstico. Si el USB sigue cayéndose, registrar el síntoma antes de añadir periféricos. [Diagnóstico USB #27](https://github.com/wmrDev/agentic-iot-pet/pull/27)
2. **Banco por componente.** Verificar primero TTP223 y LED, luego la dirección I²C y el controlador del OLED. Seguir con el INMP441 y el MAX98357A con parlante si están disponibles. Registrar los pines comprobados y la evidencia de cada módulo. [H01 #7](https://github.com/wmrDev/agentic-iot-pet/issues/7) · [Procedimiento de banco](../hardware/H01_BANCO.md)
3. **Autonomía si el banco está estable.** Ejecutar Unity y después comprobar inicio, pausa y reanudación de una sesión sin red. Las pruebas de configuración borran la NVS. Usar una configuración de banco respaldada y volver a cargar el firmware de producto al terminar. [Procedimiento T04](../pruebas/T04_PLACA.md)
4. **Acuerdo de demo.** Revisar la evidencia del INFINIX y el Galaxy, repartir la explicación y acordar qué puede demostrarse en físico y qué queda pendiente. El iPhone 14 puede usarse para documentar el ensayo. [Recorrido físico #29](https://github.com/wmrDev/agentic-iot-pet/issues/29) · [Coordinación #30](https://github.com/wmrDev/agentic-iot-pet/issues/30)

## Resultado esperado

Si el USB se estabiliza, el objetivo es salir con un banco confiable, un mapa de pines comprobado y una primera prueba de autonomía. Para cada prueba, registrar la revisión, el entorno y el resultado observado con la [plantilla de pruebas](../pruebas/PLANTILLA_RESULTADO.md).

El recorrido integrado Android–ESP32 sigue condicionado al emparejamiento de T02 y al primer enlace HTTPS de T03. En el informe, registrar por separado la compilación, la simulación, el banco y la prueba integrada. [T02 #5](https://github.com/wmrDev/agentic-iot-pet/issues/5) · [T03 #6](https://github.com/wmrDev/agentic-iot-pet/issues/6)
