# PACKAGE STANDARD

## Transitional bootstrap

NCOS-023 Repair3 is installed once with explicit `verify`, `deps` and `install`, then compiled and atomically activated.

## Normal operation after bootstrap

Running `nexiora_package.exe` with no package path discovers the newest valid pending package and applies it automatically.

## Native apply contract

- verify
- dependencies
- transactional install
- CMake configure
- full staged build
- warning gate
- complete CTest suite
- documentation validation
- history
- exact-transaction rollback on failure

## Artefactos cognitivos obligatorios
Los paquetes de capacidad con CLI deben añadir tanto la prueba como el ejecutable al target `nexiora_test_suite_staged`; una biblioteca sin su CLI no certifica la entrega.

## Artefactos cognitivos obligatorios
Los paquetes de memoria deben materializar la CLI y su prueba como dependencias de `nexiora_test_suite_staged`.
