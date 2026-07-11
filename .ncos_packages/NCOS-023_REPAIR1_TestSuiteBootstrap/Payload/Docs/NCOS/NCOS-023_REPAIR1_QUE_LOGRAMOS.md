# NCOS-023 Repair1 — Test Suite Bootstrap and Staged Self-Update

## Qué logramos

- Se restaura el objetivo CMake `nexiora_test_suite` requerido por el `apply` nativo vigente.
- Se materializan todos los ejecutables de prueba antes de CTest.
- Se compila el Package Manager nuevo como `nexiora_package.next.exe` para evitar sobrescribir un binario en ejecución.
- Se programa un reemplazo atómico y diferido al terminar el proceso `apply` actual.
- Se conserva rollback transaccional si build, warnings, pruebas o documentación fallan.
- Se mantiene el descubrimiento automático de paquetes pendientes de NCOS-023.
