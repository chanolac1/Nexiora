# HIST-0002 — Fixed Console Progress UI

## Objetivo

Actualizar la investigación observable para que el progreso no llene la pantalla. Ahora `nexiora.exe investiga SQLite` usa una UI de consola que redibuja el mismo panel durante la operación.

## Nuevos componentes

- `NxConsoleUI`
- `NxTopicInvestigation` con modo live-console
- Tests:
  - `NxConsoleUITests`
  - `NxTopicInvestigationLiveTests`

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-hist-0002.ps1
```

Demo manual:

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe investiga SQLite
```

## Commit

```powershell
git add .
git commit -m "HIST-0002: Add fixed-screen console progress UI"
git push
```
