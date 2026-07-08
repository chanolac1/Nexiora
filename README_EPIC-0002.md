# EPIC-0002: Knowledge Engine Foundation

This epic adds the first Knowledge Engine layer to Nexiora.

## What it adds

- `NxKnowledgeBase`: stores evidence-backed facts.
- `NxKnowledgeQuery`: queries knowledge using caller-owned buffers.
- Tests for storage, validation, duplicate detection, and queries.
- RFC and SPEC documentation.
- BOOK history entry.

## Verify

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0002.ps1
```

## Commit

```powershell
git add .
git commit -m "EPIC-0002: Knowledge Engine Foundation"
git push
```
