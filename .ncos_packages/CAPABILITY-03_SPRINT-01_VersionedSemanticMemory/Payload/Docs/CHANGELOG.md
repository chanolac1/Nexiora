# CHANGELOG

## CAPABILITY-02 Sprint-03 Repair2
- Corrected contextual evidence selection for conversational follow-up questions.
- Corrected the conversational demonstration evidence dataset.
- Added regression coverage against repeated answers across semantically different turns.

- Fixed conversational reasoning leakage by separating focal and comparative evidence selection.
- Added regression requiring the first answer to exclude semantic-memory evidence and use one top-ranked chunk.

## CAPABILITY-03 Sprint-01
- Añadido formato `nxsemantic/1`.
- Añadido historial inmutable de creencias y procedencia.
- Añadida CLI `nexiora_semantic_memory.exe`.
- Añadida regresión de cambio de creencias y rechazo de versiones redundantes.
