# SPEC — EPIC-0010 Research Progress Framework

## Objetivo

Agregar un componente reutilizable para reportar progreso de investigaciones y tareas largas.

## API

- `NxProgressTask_Init`
- `NxProgressTask_Begin`
- `NxProgressTask_Update`
- `NxProgressTask_Warn`
- `NxProgressTask_Fail`
- `NxProgressTask_Cancel`
- `NxProgressTask_Finish`
- `NxProgressTask_FormatBar`
- `NxProgressTask_Emit`

## Criterio de aceptación

- Compila.
- `NxProgressEngineTests` pasa.
- `nexiora_progress_demo.exe SQLite` imprime progreso hasta 100%.
