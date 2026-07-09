# NCOS-010 REPAIR3

Corrige definitivamente el problema de rutas del repair anterior:

- Detecta la raíz real del repositorio buscando `CMakePresets.json`.
- No asume `D:\` ni `.nexiora_sprints` como raíz.
- Aplica el fix de C2x agregando `<stdlib.h>` si el test usa `system()`.
- Ejecuta prueba enfocada, suite completa y smoke test del analizador.

Uso:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-010-repair3.ps1
```
