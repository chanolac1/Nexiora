# NCOS-003 Repair

Corrige dos problemas:

1. `NxConceptRegistryTests` fallaba porque el motor no creaba el directorio raíz del entorno de prueba antes de `Knowledge/NCOS/Concepts`.
2. `verify-ncos-003.ps1` validaba sobre un arreglo de líneas de PowerShell, lo cual hacía fallar la comprobación aunque el texto existiera.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-003.ps1
```
