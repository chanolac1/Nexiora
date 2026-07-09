# CORE-0004 Repair

Corrige la validación de respuestas específicas:

- `NxTopicQuestion` ahora imprime `Fuentes registradas` en respuestas conceptuales.
- La respuesta de Knowledge Base incluye explícitamente `Repositorio central`.
- El verificador acepta la salida real y valida contenido específico.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0004-repair.ps1
```
