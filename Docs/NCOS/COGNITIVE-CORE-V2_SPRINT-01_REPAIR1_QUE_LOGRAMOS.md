# Qué logramos — COGNITIVE-CORE-V2 Sprint-01 Repair1

- Se restauró `Source/NCOS/NxPackageCertification.c` como fuente obligatoria de `NexioraNCP` mediante el CMake canónico de NCOS-025.
- Se eliminó el warning de truncamiento del Goal Graph con una copia acotada basada en `strlen` y `memcpy`.
- Se preservaron `nexiora_goal_graph.exe` y `NxGoalGraphTests.exe` como artefactos obligatorios.
- Pure Apply debe ejecutar build, warning gate, dos repeticiones de QA, documentación, artefactos y certificación.
- La publicación Git queda bloqueada hasta revisar el nuevo `certification-report.nxcert`.
