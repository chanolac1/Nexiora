# NCOS-015 Warning Repair

## Qué logramos

Se limpia `NxPatchVerificationGate.c` para eliminar warnings de compilación:

- Se eliminan `if(out)*out=r; return 1;` ambiguos.
- Se reemplaza `snprintf` en `nx_join` por construcción manual con validación de tamaño.
- Se separan funciones largas de una sola línea.
- El verificador falla si vuelven warnings de `NxPatchVerificationGate.c`.

## Ejecutar

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-015-warning-repair.ps1
```
