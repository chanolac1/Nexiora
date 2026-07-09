# NCOS-011 — Auto-Fix Proposal Engine

## Qué logramos

Nexiora ahora puede tomar un reporte de análisis de build y generar una propuesta de corrección estructurada.

Esta entrega **no modifica código automáticamente**. Solo propone acciones seguras para revisión humana.

## Qué habilita

- Autocorrección controlada en NCOS-012.
- Experiencia de build: compilar → analizar error → proponer fix → aprobar → aplicar.
- Memoria de errores y estrategias futuras.

## Comandos

```powershell
nexiora_fix_proposal propose sample_fix .\Knowledge\NCOS\BuildAnalysis\repair3.analysis.md
```
