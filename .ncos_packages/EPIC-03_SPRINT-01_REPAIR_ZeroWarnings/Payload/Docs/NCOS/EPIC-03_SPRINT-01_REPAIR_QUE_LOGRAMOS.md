# EPIC-03 Sprint-01 Repair — Zero Warnings

## Qué logramos

- Eliminamos los warnings de compilación de `NxMultimodalIngestionTests` en configuración Release.
- Sustituimos el uso de `assert()` como mecanismo de validación por comprobaciones activas que no desaparecen con `NDEBUG`.
- Conservamos la cobertura de los casos de texto, video sin transcripción y video con transcripción.
- Añadimos una compuerta automática que rechaza la entrega si CMake, Ninja, GCC o Clang emiten warnings durante la configuración o compilación.
- Mantenemos instalación transaccional, historial y rollback mediante el Package Manager.

## Causa raíz

En Release, CMake define `NDEBUG`. Las expresiones dentro de `assert()` se eliminan y las variables usadas únicamente por esas expresiones quedan sin uso, generando warnings. La reparación usa validaciones explícitas y permanentes.
