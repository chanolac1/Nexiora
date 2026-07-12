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

## Cognitive Core v2 / Goal Graph
`NxGoalGraph` representa objetivos complejos como nodos jerárquicos con dependencias explícitas. El formato `nxgoalgraph/1` es auditable, versionable y no depende de procesos externos. La CLI es un artefacto obligatorio de la suite materializada.
