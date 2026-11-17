# Entornos y compilación reproducible

## Inventario pendiente de completar por cada integrante

| Dato | Walther | Maximiliano | Winderson |
|---|---|---|---|
| Sistema y versión exacta | Windows; versión por registrar | Windows; versión por registrar | Linux y Windows; versiones por registrar |
| Git | Detectado en PATH | Por verificar | Por verificar |
| JDK | Detectado JDK 17 en PATH; no validado con el proyecto | Por verificar | Por verificar |
| Android Studio y SDK | Por verificar | Por verificar | Por verificar |
| ESP-IDF y herramientas | No encontrados en PATH; instalación no descartada | Por verificar | Por verificar |
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
| ESP-IDF y toolchain | A registrar en T00 |
| Room, SQLite y KSP | A fijar en T05 con prueba Android/Windows |
| Sistema Windows de referencia | A registrar en T00 |

No hay versiones de producto validadas en este momento. El JDK detectado no obliga a elegirlo si la combinación seleccionada requiere otro compatible.

## Comandos previstos después de T00

Se ejecutan desde la raíz, salvo firmware. **Todavía no existen los wrappers ni estos proyectos; no ejecutar como si esta preparación ya compilara.**

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
