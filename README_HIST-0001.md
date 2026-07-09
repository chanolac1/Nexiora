# HIST-0001 — Investiga SQLite

Entrega vertical funcional: Nexiora ejecuta una investigación observable de un tema, muestra progreso, extrae conceptos base y genera artefactos locales.

## Comandos

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-hist-0001.ps1
```

Demo manual:

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe investiga SQLite
.\Build\windows-msvc-release\bin\nexiora_investigate.exe SQLite
```

Artefactos esperados:

```text
Knowledge/Investigations/sqlite/report.md
Knowledge/Investigations/sqlite/memory.jsonl
```

Nota: esta historia usa un catálogo local controlado para SQLite. No descarga Internet todavía; deja listo el flujo visible y persistente para conectar providers reales en la siguiente historia.
