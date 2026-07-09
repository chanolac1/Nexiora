# EPIC-0010 — Research Progress Framework

Objetivo: hacer observables las tareas largas de Nexiora.

## Incluye

- `NxProgressEngine`
- `NxProgressTask`
- Estados: pendiente, en proceso, OK, advertencia, error, cancelado
- Barra de progreso textual
- Eventos de progreso por callback
- Demo: `nexiora_progress_demo.exe`
- Test unitario: `NxProgressEngineTests`

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0010.ps1
```

## Uso manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_progress_demo.exe SQLite
```

## Commit

```powershell
git add .
git commit -m "EPIC-0010: Add Research Progress Framework"
git push
```
