# NCOS-001 REPAIR3

Corrige el problema de PowerShell donde `Copy-Item` intentaba sobrescribir `NxSessionEngine.c` consigo mismo.

Este repair no copia archivos de código; solo ejecuta configuración, build, tests y smoke test.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-001-repair3.ps1
```
