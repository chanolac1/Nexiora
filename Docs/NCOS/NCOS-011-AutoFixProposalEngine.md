# NCOS-011 — Auto-Fix Proposal Engine

## Qué logramos

Nexiora ahora puede convertir hallazgos de build en propuestas de corrección.

Entrada:

```txt
Build analysis report
```

Salida:

```txt
Knowledge/NCOS/FixProposals/<id>.proposal.md
```

## Política de seguridad

El motor no modifica código por sí solo. Genera propuestas revisables por humano.
