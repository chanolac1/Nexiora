# CORE-0005 — Auto Research Question Mode

Agrega comandos:

```powershell
nexiora pregunta-auto <tema> "<pregunta>"
nexiora auto <tema> "<pregunta>"
```

Flujo:

1. Consulta memoria local.
2. Si no sabe, ejecuta aprendizaje del tema.
3. Reintenta la pregunta.
4. Guarda la respuesta en `Knowledge/Topics/<tema>/last_question_answer.txt`.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0005.ps1
```

## Uso

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe pregunta-auto Genexus "Para que sirve una Knowledge Base?"
.\Build\windows-msvc-release\bin\nexiora.exe auto Kubernetes "Que es un Pod?"
```
