# NCOS-022 Repair 1 — Apply Contract Unification

## Qué logramos

- Se eliminó la definición duplicada de `NxPackageApplyResult`.
- `NxPackageManager.h` es la única fuente de verdad del contrato público de `apply`.
- `NxPackageApply.h` contiene únicamente las fases y utilidades específicas del flujo.
- La implementación y la CLI usan los campos canónicos `*_passed` y `rolled_back`.
- Se conserva compatibilidad con NCOS-021 y el comportamiento transaccional de NCOS-022.
- Se añadió una regresión de compilación que incluye ambos encabezados en la misma unidad.
