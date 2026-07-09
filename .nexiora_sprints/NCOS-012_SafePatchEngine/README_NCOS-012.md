# NCOS-012 — Safe Patch Engine

## Qué logramos

Nexiora ahora puede convertir un análisis de build/error en una propuesta de parche segura y revisable.

El motor no modifica código automáticamente. Genera una propuesta estructurada y deja la decisión en una compuerta de aprobación humana.

## Comandos

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-012.ps1
```

Prueba manual:

```powershell
.\Build\windows-msvc-release\bin\nexiora_safe_patch.exe propose demo .\Knowledge\NCOS\BuildAnalysis\repair3.analysis.md
```

## Salida

```txt
Knowledge/NCOS/PatchProposals/<run_id>.patch.md
```

## Qué habilita

- NCOS-013: aplicar parches en un sandbox.
- NCOS-014: compilar y validar el parche.
- NCOS-015: promover cambios con aprobación humana.
