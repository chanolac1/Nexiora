# RFC-0033 — Knowledge Acquisition Engine

## Estado

Aceptado para EPIC-0009.

## Decisión

Nexiora incorpora un motor inicial de adquisición de conocimiento que separa tres fases:

1. planear la investigación;
2. clasificar fuentes candidatas;
3. generar artefactos persistentes para revisión humana.

## Restricción

EPIC-0009 no ejecuta navegación web automática. La adquisición externa se implementará mediante providers controlados en futuras EPICs.

## Principios

- La fuente primaria tiene mayor prioridad.
- Las fuentes técnicas secundarias se usan para contraste, no como autoridad única.
- Toda conclusión futura debe diferenciar hechos, inferencias y opinión.
- La promoción al Runtime sigue requiriendo aprobación humana.
