# NCOS-001 Repair2

Corrige la causa real del fallo de `NxSessionEngineTests`: el motor intentaba crear `root/Knowledge` sin asegurar primero que `root` existiera.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-001-repair2.ps1
```
