# CORE-0003 — Topic Question Answering

Agrega preguntas específicas sobre temas aprendidos.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0003.ps1
```

## Uso

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
.\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Que es una Transaction?"
.\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Para que sirve una Knowledge Base?"
```

## Commit

```powershell
git add .
git commit -m "CORE-0003: Add topic question answering"
git push
```
