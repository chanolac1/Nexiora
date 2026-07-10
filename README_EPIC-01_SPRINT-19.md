# EPIC-01 Sprint-19 — Autonomous Workspace Engine

Paquete administrado por el Nexiora Package Manager.

## Instalación

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe verify .\.ncos_packages\EPIC-01_SPRINT-19_AutonomousWorkspaceEngine
.\Build\windows-msvc-release\bin\nexiora_package.exe deps .\.ncos_packages\EPIC-01_SPRINT-19_AutonomousWorkspaceEngine
.\Build\windows-msvc-release\bin\nexiora_package.exe install .\.ncos_packages\EPIC-01_SPRINT-19_AutonomousWorkspaceEngine
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-01-sprint-19.ps1
```
