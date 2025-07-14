# Plan para migración a la clase Platform

1. **Revisar el código original**: Analizar `main_tanyaTMC.cpp` para extraer toda la configuración de pines, inicialización de sensores y driver. Ejemplo en las líneas 81‑98 se configuran los pines de motor, sensor y el `attachInterrupt` para el final de carrera.
2. **Completar la clase `Platform`**: Añadir a `Platform.cpp` un método de inicialización que incluya:
   - `pinMode` para `driver_pul`, `driver_dir`, `driver_en`, `trigPin`, `echoPin` y `limitSwitchPin`.
   - `attachInterrupt(digitalPinToInterrupt(limitSwitchPin), stopMotor, RISING)`.
   - Arranque del driver y parámetros de `AccelStepper`.
   - Definición de `Platform::stopMotor()` para actualizar la bandera `limitReached`.
3. **Ajustar `attachPlatform`**: Utilizar este método para almacenar los parámetros de configuración recibidos y llamar a la nueva función de inicialización.
4. **Actualizar `PlatformStateMachine`**: En `setup()` llamar a `platform.attachPlatform(...)` seguido de `platform.begin()` (la nueva función de inicialización). Asegurarse de que las llamadas a `configureDriver` y a la configuración de velocidad se realizan dentro de `begin()` o inmediatamente después.
5. **Migrar variables globales**: Todas las variables relacionadas con la plataforma (p. ej. `cups_on_platform`, `limitDistance`, etc.) deben convertirse en miembros de la clase. Revisar que en el `state machine` se accede mediante `platform.miembro` y no con variables globales antiguas.
6. **Revisar la lógica de lectura del sensor**: El método `updateCupPresence()` ya encapsula la lectura ultrasónica (líneas 90‑109 de `Platform.cpp`). Sustituir en el `loop` las lecturas manuales por llamadas a este método.
7. **Verificar estados**: La máquina de estados actual para `id=1` se basa en la previa. Confirmar que los cambios en `state`, `cups_on_platform` y el control del motor utilizan las funciones de `Platform` (por ejemplo `moveToNextCup()` y `rotateToLimit()`).
8. **Probar el código**: Compilar con `platformio` usando la configuración de `platformio.ini` y cargar en el dispositivo. Observar por consola que el sensor muestra valores distintos de `0.0` y que el motor gira en la etapa de inicialización.
9. **Depurar**: Si el motor no gira, verificar conexiones físicas, dirección de giro (`turn_direction`) y valores de `max_speed` y `max_acc`. Si el sensor sigue mostrando `0.0`, revisar el cableado y el cálculo de la distancia en `readUltrasonicSensor()`.
10. **Limpiar**: Una vez todo funcione, eliminar el código antiguo (`main_tanyaTMC.*`) o mantenerlo como referencia, y documentar la inicialización y los estados dentro de la clase para futuras ampliaciones.
