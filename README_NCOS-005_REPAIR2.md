# NCOS-005 REPAIR2

## Qué corregimos

`NxConceptRegistryTests` fallaba cuando ya existía una tarjeta previa de `DataSelector`, porque el versionado persistido hacía que el primer `upsert` ya no fuera versión 1.

Este repair hace la prueba repetible:

- limpia el estado temporal antes de ejecutar;
- limpia el estado temporal al finalizar;
- usa una verificación enfocada y luego la suite completa.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-005-repair2.ps1
```
