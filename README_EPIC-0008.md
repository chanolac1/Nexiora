# EPIC-0008 — Persistent Knowledge Memory

This epic adds Nexiora persistent knowledge memory.

## New commands

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe memory seed
.\Build\windows-msvc-release\bin\nexiora.exe memory summary
```

The memory is stored at:

```text
Knowledge/Memory/memory.jsonl
```

It preserves facts, decisions, hypotheses and concepts with confidence and provenance.
