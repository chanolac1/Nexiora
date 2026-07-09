# SPEC — EPIC-0009 Knowledge Acquisition Engine

## API

- `NxKnowledgeAcquisition_BuildPlan(topic, plan_out)`
- `NxKnowledgeAcquisition_WritePlanMarkdown(plan, root, output_path, size)`
- `NxKnowledgeAcquisition_WritePlanJson(plan, root, output_path, size)`

## Clasificación inicial

- `book`
- `rfc`
- `software`
- `local-file`
- `general`

## Salidas

- `Research/Acquisition/<topic>/plan.md`
- `Research/Acquisition/<topic>/plan.json`

## Criterio de aceptación

- El proyecto compila.
- Todos los tests pasan.
- `NxKnowledgeAcquisitionTests` pasa.
- `nexiora_acquire.exe SQLite` genera plan y artefactos.
