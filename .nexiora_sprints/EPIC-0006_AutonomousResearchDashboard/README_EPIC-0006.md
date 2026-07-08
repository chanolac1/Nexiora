# EPIC-0006 — Autonomous Research Dashboard

This epic adds the first visible dashboard for Nexiora autonomous research sessions.

## Commands

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0006.ps1
```

Manual commands:

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe research run
.\Build\windows-msvc-release\bin\nexiora.exe research dashboard
```

Open:

```text
Research\Sessions\first_autonomous_execution\dashboard.html
```

## Commit

```powershell
git add .
git commit -m "EPIC-0006: Add Autonomous Research Dashboard"
git push
```
