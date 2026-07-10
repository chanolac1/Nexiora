# EPIC-03 Sprint-02 Repair1 — Decoder Plan Diagnostics

## Qué logramos

- Los planes fallidos siempre inicializan completamente su estructura de salida.
- `NOT_FOUND` conserva la ruta solicitada y devuelve un diagnóstico explícito.
- La CLI deja de imprimir memoria residual como `output=C`.
- `plan` declara que es una simulación y que no genera archivos.
- `execute` continúa siendo la operación que produce evidencia textual.
- Se agregó regresión para planes fallidos con memoria previamente contaminada.
