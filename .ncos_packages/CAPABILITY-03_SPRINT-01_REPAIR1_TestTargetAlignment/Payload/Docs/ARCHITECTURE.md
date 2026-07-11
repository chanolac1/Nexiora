# ARCHITECTURE

## Package lifecycle

Package discovery -> verification -> dependency resolution -> transactional install -> configure -> staged build -> warning gate -> complete tests -> documentation validation -> history.

## Cognitive knowledge-gap boundary

Grounded reasoning evaluates the existing evidence first. When evidence is insufficient, `NxKnowledgeGapResearch` creates an `nxgap/1` plan instead of synthesizing an unsupported answer. The plan is persistent and auditable, and can later be consumed by autonomous research execution.

## Code boundaries

<!-- NX-CODE: Include/Nexiora/Research/NxKnowledgeGapResearch.h -->
<!-- NX-CODE: Source/Research/NxKnowledgeGapResearch.c -->
<!-- NX-CODE: Tools/Research/NxKnowledgeGapResearchTool.c -->

## Unified Cognitive Dialogue
`NxUnifiedCognitiveDialogue` orquesta `NxConversationalContext`, `NxGroundedReasoning`, `NxContradictionHypothesis` y `NxKnowledgeGapResearch`. La fachada no duplica motores; coordina sus contratos y preserva trazabilidad.

## Versioned Semantic Memory
`NxVersionedSemanticMemory` persiste entradas `nxsemantic/1`. Una entrada activa puede reemplazar a otra mediante `supersedes_id`, sin eliminar el historial.
