# EPIC-0007 Repair 2

Este reparador reemplaza de forma directa `Source/NxBootstrap.c` para integrar la consola conversacional cuando se ejecuta `nexiora.exe` sin argumentos.

También integra `NxConversation` en CMake y ejecuta una prueba de humo con comandos reales:

- ayuda
- estado
- que aprendiste
- recomendaciones
- sorprendeme
- salir

Uso:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0007.ps1
```
