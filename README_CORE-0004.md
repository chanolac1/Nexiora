# CORE-0004 — Concept-aware Question Answering

Mejora `nexiora pregunta <tema> "<pregunta>"` para responder de forma específica cuando la pregunta se refiere a conceptos conocidos del tema GeneXus.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0004.ps1
```

## Pruebas manuales

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
.\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Para que sirve una Knowledge Base?"
.\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Que es una Transaction?"
.\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Que es un Data Provider?"
```

## Commit

```powershell
git add .
git commit -m "CORE-0004: Add concept-aware question answering"
git push
```
