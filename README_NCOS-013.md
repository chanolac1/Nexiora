# NCOS-013 — Patch Approval Engine

## Objetivo

Agregar un flujo formal de aprobación humana para propuestas de parche generadas por Nexiora.

## Qué logra

- Registra propuestas pendientes de aprobación.
- Permite aprobar o rechazar explícitamente una propuesta.
- Genera un archivo persistente de decisión.
- Mantiene la regla central del proyecto: Nexiora puede proponer cambios, pero no promoverlos sin aprobación humana.

## Comandos

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-013.ps1
```

Prueba manual:

```powershell
.\Build\windows-msvc-release\bin\nexiora_patch_approval.exe request demo .\Knowledge\NCOS\PatchProposals\demo.patch.md
.\Build\windows-msvc-release\bin\nexiora_patch_approval.exe approve demo Jorge
.\Build\windows-msvc-release\bin\nexiora_patch_approval.exe status demo
```

## Commit sugerido

```powershell
git add .
git commit -m "NCOS-013: Add patch approval engine"
git push
```
