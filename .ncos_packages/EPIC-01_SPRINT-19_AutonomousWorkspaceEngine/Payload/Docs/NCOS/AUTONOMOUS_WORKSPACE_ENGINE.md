# EPIC-01 Sprint-19 — Autonomous Workspace Engine

## Objetivo

Crear áreas de trabajo aisladas donde Nexiora pueda experimentar con archivos seleccionados sin modificar el repositorio principal.

## Capacidades

- Normalización segura del identificador del workspace.
- Rechazo de rutas absolutas o con navegación `..`.
- Copia de archivos conservando la estructura relativa.
- Estado persistente `READY` / `CLOSED`.
- Registro de acciones.
- Limpieza explícita del workspace.
- Pruebas aisladas y repetibles.

## Ubicación

`Knowledge/NCOS/Workspaces/<workspace-id>/`

## Flujo

```text
repositorio
    ↓
selección de archivos
    ↓
workspace aislado
    ↓
experimentos / compilación / validación
    ↓
cierre o limpieza
```

Este sprint no promueve cambios al repositorio principal. Esa decisión permanece bajo los gates humanos existentes.
