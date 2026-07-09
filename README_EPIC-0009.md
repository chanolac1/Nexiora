# EPIC-0009 — Knowledge Acquisition Engine

## Objetivo

Agregar la primera base para que Nexiora pueda preparar investigaciones sobre temas externos o internos, generar un plan, clasificar fuentes candidatas y persistir artefactos locales.

Esta EPIC no descarga Internet todavía. Construye la capa de planeación y el contrato de providers para que la adquisición real sea controlada, trazable y verificable.

## Comandos

Verificación completa:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0009.ps1
```

Uso directo:

```powershell
.\Build\windows-msvc-release\bin\nexiora_acquire.exe SQLite
.\Build\windows-msvc-release\bin\nexiora_acquire.exe libro Clean Architecture
.\Build\windows-msvc-release\bin\nexiora_acquire.exe RFC QUIC
```

Uso integrado si `NxBootstrap.c` pudo ser actualizado:

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe acquire plan SQLite
```

## Artefactos generados

```text
Research/Acquisition/<tema>/plan.md
Research/Acquisition/<tema>/plan.json
```

## Commit

```powershell
git add .
git commit -m "EPIC-0009: Add Knowledge Acquisition Engine"
git push
```
