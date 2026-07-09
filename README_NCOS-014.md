# NCOS-014 — Approved Patch Apply Engine

## Qué logramos

Nexiora ahora puede tomar una aprobación humana existente y convertirla en un registro de aplicación seguro y revisable.

Este motor no promueve cambios directamente al Runtime. Respeta la política de control humano:

1. Safe Patch Engine propone.
2. Patch Approval Engine aprueba o rechaza.
3. Approved Patch Apply Engine genera evidencia de aplicación revisable.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-014.ps1
```

## Prueba manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_apply_approved_patch.exe apply apply_demo
.\Build\windows-msvc-release\bin\nexiora_apply_approved_patch.exe status apply_demo
```
