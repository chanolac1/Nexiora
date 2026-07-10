# NCOS-015 — Patch Verification Gate

## Qué logramos

Nexiora ahora puede verificar si una propuesta aplicada y aprobada tiene evidencia suficiente para ser promovible.

Flujo:

```text
Safe Patch Proposal
  -> Human Approval
  -> Approved Patch Apply Engine
  -> Patch Verification Gate
  -> PROMOTABLE / BLOCKED
```

El motor no promueve cambios automáticamente. Solo produce evidencia y una decisión verificable.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-015.ps1
```

## Prueba manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_patch_verify.exe verify verify_demo
.\Build\windows-msvc-release\bin\nexiora_patch_verify.exe status verify_demo
```

## Commit

```powershell
git add .
git commit -m "NCOS-015: Add patch verification gate"
git push
```
