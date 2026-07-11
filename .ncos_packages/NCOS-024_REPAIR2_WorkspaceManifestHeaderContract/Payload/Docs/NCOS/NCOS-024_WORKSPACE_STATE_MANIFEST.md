# NCOS-024 — Workspace State Manifest

## Qué logramos

Nexiora puede generar `workspace.nxws` como fotografía técnica verificable del repositorio.

Comandos:

- `nexiora_workspace snapshot <root> [salida]`
- `nexiora_workspace validate <root> [manifest]`
- `nexiora_workspace diff <root> <manifest>`

El manifiesto conserva Git, C23, inventario CMake, pruebas, ejecutables, paquetes y estado documental.

NX-CODE: Include/Nexiora/NCOS/NxWorkspaceManifest.h
NX-CODE: Source/NCOS/NxWorkspaceManifest.c
NX-CODE: Tools/NCOS/NxWorkspaceTool.c
NX-CODE: Tests/Unit/NxWorkspaceManifestTests.c
