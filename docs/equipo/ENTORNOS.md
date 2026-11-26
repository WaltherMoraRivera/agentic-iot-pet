# Entornos y compilación reproducible

## Inventario pendiente de completar por cada integrante

| Dato | Walther | Maximiliano | Winderson |
|---|---|---|---|
| Sistema y versión exacta | Windows 25H2, build 26200.9974 | Windows; versión por registrar | Linux y Windows; versiones por registrar |
| Git | Detectado en PATH | Por verificar | Por verificar |
| JDK | Microsoft OpenJDK 17.0.20.1 | Por verificar | Por verificar |
| Android Studio y SDK | Build 262.9437.185.2621.16467767, plataforma 36 rev. 2, Build Tools 35.0.0 y ADB 37.0.1 | Por verificar | Por verificar |
| ESP-IDF y herramientas | ESP-IDF 5.5.6 instalado. El entorno de shell inicial requiere activación y herramientas de build. | Por verificar | ESP-IDF 5.5.6 en CachyOS, usado en el PR #27 |
| Teléfono | INFINIX ZERO 30 5G, XOS 14.5.0 | iPhone 14 | Galaxy S24 Ultra |
| Android real / API level | Por registrar | Por registrar | Por registrar |

Inventario revisado el 26 de noviembre. En Windows el SDK está en `%LOCALAPPDATA%\Android\Sdk`. Android Studio permite instalar la plataforma API 36 desde el SDK Manager. Para compilar desde PowerShell, usar el SDK configurado en Android Studio o definir `ANDROID_HOME` en la sesión. Para firmware, activar el entorno de ESP-IDF 5.5.6.

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
| JDK y Android Studio | Microsoft OpenJDK 17.0.20.1 y Android Studio instalado en Windows |
| Gradle Wrapper, plugins Android y Kotlin | Gradle 8.13, AGP 8.11.1, Kotlin 2.2.21 |
| Compose Multiplatform | Compose 1.9.3 y Material 3 1.9.0 |
| Android minSdk / compileSdk / targetSdk | 24 / 36 / 36. La instalación en teléfonos reales se verifica por separado. |
| ESP-IDF y toolchain | A registrar en T00 |
| Room, SQLite y KSP | A fijar en T05 con prueba Android/Windows |
| Sistema Windows de referencia | A registrar en T00 |

La compilación local produjo la APK de depuración y el código de escritorio. El workflow de Gradle está preparado y su ejecución en GitHub queda pendiente.

## Comandos de T00

Se ejecutan desde la raíz, salvo firmware. El build de ESP32-S3 requiere el checkout que contiene el diagnóstico del PR #27.

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
