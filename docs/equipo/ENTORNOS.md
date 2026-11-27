# Entornos y compilación reproducible

## Inventario pendiente de completar por cada integrante

| Dato | Walther | Maximiliano | Winderson |
|---|---|---|---|
| Sistema y versión exacta | Windows; versión por registrar | Windows 11 Home Single Language 26H2, compilación 26300.10075 (26-11) | Linux y Windows; versiones por registrar |
| Git | Detectado en PATH | Git 2.54.0.windows.1 (26-11) | Por verificar |
| JDK | Detectado JDK 17 en PATH; no validado con el proyecto | Por verificar | Por verificar |
| Android Studio y SDK | Por verificar | Por verificar | Por verificar |
| ESP-IDF y herramientas | No encontrados en PATH; instalación no descartada | Windows: v5.5.6 instalado con EIM y compilación verificada (26-11); ver abajo | Linux: v5.5.6 verificado (25-11); Windows: por verificar |
| Teléfono | INFINIX ZERO 30 5G, XOS 14.5.0 | Por identificar | Por identificar |
| Android real / API level | Por registrar | Por registrar | Por registrar |

El inventario de Walther es una inspección parcial del 17 de noviembre. `adb`, `idf.py`, CMake, Ninja y Gradle no se encontraron en PATH; pueden existir dentro de instalaciones del IDE. Python está disponible, pero no se usará su versión global para sustituir el entorno compatible que instala ESP-IDF.

## Preparación por rol

- Todos: Git, editor/IDE, acceso al futuro remoto y lectura de las reglas comunes.
- B/C: Android Studio con soporte KMP, Android SDK y JDK compatible con el conjunto de versiones escogido; ejecución en Android real y Windows.
- A: ESP-IDF con herramientas para `esp32s3`, terminal de su entorno y acceso USB a la placa.
- C: mismo conjunto Gradle que B para simulador y pruebas; no instalar otra pila de servidor.

La [guía de Kotlin Multiplatform](https://kotlinlang.org/docs/multiplatform/quickstart.html) documenta el arranque con Gradle e IDE. La [guía ESP-IDF para ESP32-S3](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html) documenta instalación, compilación y flasheo. Seguir fuentes oficiales; fijar las versiones después del ejemplo mínimo, no instalar indiscriminadamente «lo más nuevo» en cada equipo.

## Registro de versiones de T00

T00 completará esta tabla con versiones exactas probadas y comandos observados, y versionará Gradle Wrapper, catálogo de dependencias y configuración reproducible del firmware.

| Componente | Versión validada / evidencia |
|---|---|
| JDK y Android Studio | A registrar en T00 |
| Gradle Wrapper, plugins Android y Kotlin | A registrar en T00 |
| Compose Multiplatform | A registrar en T00 |
| Android minSdk / compileSdk / targetSdk | A fijar con dispositivos reales y dependencias |
| ESP-IDF y toolchain | Parcial (25-11, CachyOS): ESP-IDF v5.5.6, Xtensa GCC 14.2.0, Python 3.14.10, CMake 4.5.1, Ninja 1.13.2, esptool 4.14.0; compila `firmware/`. [Evidencia](../pruebas/resultados/2026-11-25-T00-prueba-usb.md)<br>Parcial (26-11, Windows 11 de Maximiliano): ESP-IDF v5.5.6, Xtensa GCC 14.2.0 (`esp-14.2.0_20260121`), Python 3.11.9 del entorno virtual de ESP-IDF, CMake 3.30.2, Ninja 1.12.1, esptool 4.14.0; compilan `firmware/`, `firmware/test/` y `firmware/diagnostics/h01/` sin avisos. Sin flashear ni ejecutar en la placa. [Evidencia en #35](https://github.com/wmrDev/agentic-iot-pet/pull/35) y [#36](https://github.com/wmrDev/agentic-iot-pet/pull/36) |
| Room, SQLite y KSP | A fijar en T05 con prueba Android/Windows |
| Sistema Windows de referencia | A registrar en T00 |

### Instalación de ESP-IDF en Windows (verificada el 26-11)

Con el instalador oficial [ESP-IDF Installation Manager](https://docs.espressif.com/projects/idf-im-ui/en/latest/) (EIM CLI 0.23.0, paquete de winget `Espressif.EIM-CLI`). Requiere Git. La instalación solo para `esp32s3` ocupó 8,2 GB en `C:\Espressif`.

```powershell
winget install --id Espressif.EIM-CLI --exact
eim install -i v5.5.6 -t esp32s3 -p C:\Espressif -n true --do-not-track true --cleanup true
```

En cada terminal nueva, activar el entorno antes de `idf.py`:

```powershell
. C:\Espressif\tools\Microsoft.v5.5.6.PowerShell_profile.ps1
```

El instalador también deja en el escritorio un acceso directo a PowerShell con el entorno ya activado. `eim` no queda en PATH hasta abrir una terminal nueva.

No hay versiones de producto validadas en hardware en este momento. El JDK detectado no obliga a elegirlo si la combinación seleccionada requiere otro compatible.

## Comandos previstos después de T00

Se ejecutan desde la raíz, salvo firmware. **Todavía no existen los wrappers ni los proyectos Gradle; solo `idf.py build` funciona hoy.**

| Objetivo | Windows / PowerShell | Linux |
|---|---|---|
| Pruebas compartidas JVM | `.\gradlew.bat :shared:desktopTest` | `./gradlew :shared:desktopTest` |
| APK de depuración | `.\gradlew.bat :androidApp:assembleDebug` | `./gradlew :androidApp:assembleDebug` |
| App Windows | `.\gradlew.bat :desktopApp:run` | No valida soporte Windows |
| Simulador | `.\gradlew.bat :simulator:run` | `./gradlew :simulator:run` |
| Tests simulador | `.\gradlew.bat :simulator:test` | `./gradlew :simulator:test` |
| Tests escritorio | `.\gradlew.bat :desktopApp:test` | Solo partes JVM compatibles |
| Compilar firmware | `idf.py build`, dentro de `firmware` y terminal ESP-IDF | Igual, tras activar ESP-IDF |

T00 comprobará o ajustará estos nombres de tareas junto con todos los consumidores del plan. Compartir una sola raíz Gradle: `:shared` → `apps/shared`, `:androidApp` → `apps/androidApp`, `:desktopApp` → `apps/desktopApp`, `:simulator` → `simulator`. Shared tendrá destino JVM llamado `desktop` para la prueba común y un destino Android compatible con los plugins elegidos.

No se instala Gradle global si se usa el wrapper versionado. Si Linux informa que `gradlew` no es ejecutable, T00 debe registrar su bit ejecutable en Git. Mantener finales de línea según `.gitattributes`.

## Comprobación de hardware y red

Antes del primer flasheo, A verifica modelo real, cable USB de datos, puerto y alimentación. Seleccionar `esp32s3`; no asumir que un ejemplo de ESP32 clásico es equivalente. Fotografiar la serigrafía antes de asignar pines.

Android, Windows y placa deben alcanzar la misma red local de 2,4 GHz para el prototipo. Probar hotspot del INFINIX y un router alternativo; documentar aislamiento de clientes o bloqueo de descubrimiento. No resolver un fallo deshabilitando toda la protección del sistema: limitar cualquier regla a la aplicación y red de pruebas.

La carpeta indicada tiene espacios. Si una herramienta demuestra una incompatibilidad con esa ruta, usar un clon en una ruta corta, por ejemplo `C:\dev\pet-iot`, y trabajar por Git. No mover ni borrar el original automáticamente.
