# Registro de prueba: mediciones preliminares de H01

- ID de prueba / tarea: H01 (issue #7), avance parcial. No corresponde a una aprobación de P01 ni P02.
- Fecha, hora y responsable: 2026-11-25, hora no registrada; informado por Winderson. Responsable nominal de H01: Maximiliano.
- Revisor: pendiente.
- Commit o revisión exacta: no aplica (prueba eléctrica, sin firmware).
- Entorno: físico.
- Hardware, sistema operativo y versiones: ESP32-S3 SuperMini, TTP223, LED externo, resistencia de 330 Ω, protoboard, multímetro (marcas y modelos no registrados).
- Condiciones iniciales, red, alimentación y permisos: ESP32 y TTP223 soldados; montaje inicial en protoboard.
- Comando o procedimiento reproducible: medir la alimentación; alimentar el LED directamente; medir la salida I/O del TTP223 en reposo, al tocar y al soltar. Puntos de medición exactos no registrados.
- Resultado esperado: alimentación cercana a 3,3 V; LED enciende; salida táctil cambia solo durante el contacto.
- Resultado observado, con mediciones y unidades cuando corresponda:
  - Alimentación: 3,30 V y 3,32 V.
  - LED externo: encendió con alimentación directa.
  - TTP223: 0 V en reposo, 2,94 V al tocar, 0 V al soltar (activo en alto, momentáneo).
- Estado: NO EJECUTADA como H01 completo. Las mediciones anteriores son observaciones preliminares.
- Evidencia: solo el relato del responsable; sin fotografías.
- Defecto asociado o ajuste acordado: ninguno.
- Qué no demuestra esta prueba: control por GPIO ni firmware; continuidad; correspondencia física de GPIO4 y GPIO5; OLED, INMP441, MAX98357A, parlante ni integración.

## Pinout previsto, sin confirmar en la placa

| Señal | Conexión prevista |
|---|---|
| TTP223 VCC / GND | 3V3 / GND |
| TTP223 I/O | GPIO4 |
| LED | GPIO5 → 330 Ω → ánodo; cátodo → GND |

Los jumpers quedaron desconectados para programar la placa.
