# NRL-0010 — Research Graph Exporter / Sprint completo

Este paquete reemplaza la entrega por partes. Aplica, compila, prueba y genera el SVG visible con un solo comando.

## Uso recomendado

Descomprime este ZIP sobre:

```txt
D:\Nexiora
```

Luego ejecuta desde la raíz del repo:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-nrl-0010.ps1
```

Ese comando hace todo:

1. Aplica el Sprint.
2. Repara cualquier `CMakeLists.txt` parcialmente modificado por la entrega anterior.
3. Regenera CMake.
4. Compila Release.
5. Ejecuta `ctest`.
6. Genera:
   - `Artifacts\ResearchGraph\research_graph.dot`
   - `Artifacts\ResearchGraph\research_graph.json`
   - `Artifacts\ResearchGraph\research_graph.svg`
7. Abre automáticamente el SVG.

## Commit

Cuando todo pase:

```powershell
git add .
git commit -m "NRL-0010: Add Research Graph Exporter"
git push
```

## Nota

Este paquete crea respaldo de CMake:

```txt
CMakeLists.txt.nrl0010.full.bak
```
