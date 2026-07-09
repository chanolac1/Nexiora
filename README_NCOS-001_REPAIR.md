# NCOS-001 Repair

Corrige la validación de NCOS-001 cuando `NxSessionEngineTests` falla por estado persistente previo.

## Problema

El motor de sesión funcionaba en el smoke test, pero la prueba unitaria fallaba porque encontraba datos previos en:

```text
Knowledge/NCOS/Sessions
```

## Solución

El repair limpia el estado persistente de sesiones antes de ejecutar la suite completa. Esto deja la prueba en un estado reproducible.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-001-repair.ps1
```

Después de pasar:

```powershell
git add .
git commit -m "NCOS-001: Fix session test isolation"
git push
```
