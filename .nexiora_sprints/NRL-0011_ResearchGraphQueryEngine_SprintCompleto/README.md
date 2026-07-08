# NRL-0011 — Research Graph Query Engine

Este Sprint agrega el motor de consulta para el grafo de investigación de Nexiora.

## Qué agrega

- `NxResearchGraphQuery.h`
- `NxResearchGraphQuery.c`
- `NxResearchGraphQueryTests.c`
- RFC-0026
- SPEC NRL-0011
- Entrada de HISTORY del BOOK
- Script de aplicación y verificación completa

## Uso

Descomprime el paquete sobre la raíz del proyecto `D:\Nexiora` y ejecuta:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-nrl-0011.ps1
```

Ese único comando aplica el Sprint, configura CMake, compila y corre todos los tests.

## Commit

```powershell
git add .
git commit -m "NRL-0011: Add Research Graph Query Engine"
git push
```

## Nota arquitectónica

Este Sprint no reemplaza al exportador visual. Lo complementa: el grafo ahora puede verse con NRL-0010 y consultarse programáticamente con NRL-0011.
