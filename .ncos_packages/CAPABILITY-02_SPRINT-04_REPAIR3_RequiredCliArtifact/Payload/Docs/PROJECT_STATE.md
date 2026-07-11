# PROJECT STATE
# Nexiora

Última actualización:
2026-07-11

## Estado General

Estado del proyecto:
Activo

Rama:
main

Lenguaje:
C23

Compilador:
MSYS2 UCRT64 GCC

Build:
CMake + Ninja

## Estado del Desarrollo

Sprint actual:
CAPABILITY-02 Sprint-04 Repair2

Nombre:
Autonomous Knowledge Gap Research — Documentation Contract Alignment

Estado:
Implementado y preparado para certificación Pure Apply.

## Capacidad incorporada

- Detecta cuándo la evidencia es insuficiente sin inventar respuestas.
- Persiste vacíos de conocimiento mediante el contrato `nxgap/1`.
- Formula consultas de investigación auditables.
- Registra fuentes permitidas y criterios verificables de cierre.
- Evita abrir una investigación cuando la evidencia existente ya es suficiente.
- Mantiene sincronizados los nueve documentos rectores con el código instalado.

## Referencias de código verificables

<!-- NX-CODE: Include/Nexiora/Research/NxKnowledgeGapResearch.h -->
<!-- NX-CODE: Source/Research/NxKnowledgeGapResearch.c -->
<!-- NX-CODE: Tools/Research/NxKnowledgeGapResearchTool.c -->
<!-- NX-CODE: Tests/Unit/NxKnowledgeGapResearchTests.c -->
<!-- NX-CODE: Samples/Reasoning/gap_demo.nxevidence -->

## CAPABILITY-02 Sprint-04 Repair3 — Required CLI Artifact
Estado: listo para Pure Apply.
La certificación exige materializar `nexiora_gap.exe` además de su prueba.
NX-CODE: Tools/Research/NxKnowledgeGapResearchTool.c
NX-CODE: CMakeLists.txt
