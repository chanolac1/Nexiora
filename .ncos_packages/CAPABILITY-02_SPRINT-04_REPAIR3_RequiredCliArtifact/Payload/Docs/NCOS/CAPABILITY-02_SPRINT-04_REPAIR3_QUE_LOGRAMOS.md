# CAPABILITY-02 Sprint-04 Repair3 — Required CLI Artifact

## Qué logramos

Se corrigió el contrato de materialización de CAPABILITY-02 Sprint-04.
El objetivo de certificación Pure Apply ahora depende explícitamente de:

- `NxKnowledgeGapResearchTests`
- `nexiora_gap`

Esto impide que una instalación sea certificada si la biblioteca y sus pruebas compilan pero la herramienta de usuario no fue generada.

## Criterios de aceptación

- `Build/windows-msvc-release/bin/nexiora_gap.exe` existe al terminar `apply`.
- `NxKnowledgeGapResearchTests` pasa.
- La suite completa pasa.
- La documentación permanece sincronizada.
- Cualquier fallo produce rollback transaccional.

NX-CODE: Tools/Research/NxKnowledgeGapResearchTool.c
NX-CODE: Tests/Unit/NxKnowledgeGapResearchTests.c
NX-CODE: CMakeLists.txt
