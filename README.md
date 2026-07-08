# NRL-0009 — Research Graph Foundation

Este Sprint agrega el primer grafo de conocimiento del laboratorio de investigación de Nexiora.

## Qué incluye

- `Include/Nexiora/Research/NxResearchGraph.h`
- `Source/Research/NxResearchGraph.c`
- `Tests/Unit/NxResearchGraphTests.c`
- `Tests/Benchmark/NxResearchGraphBenchmark.c`
- `RFC/RFC-0024-Research-Graph-Foundation.md`
- `SPEC/SPEC-NRL-0009-ResearchGraph.md`
- Actualización parcial del BOOK
- Script de integración CMake

## Aplicación

Descomprime este ZIP sobre la raíz del proyecto:

```txt
D:\Nexiora
```

Luego ejecuta:

```powershell
.\Scripts\apply-nrl-0009.ps1
```

Si PowerShell bloquea scripts, usa:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-nrl-0009.ps1
```

## Compilar

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
```

## Comprobar

```powershell
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```

Debe aparecer un nuevo test:

```txt
NxResearchGraphTests
```

## Benchmark opcional

```powershell
.\Build\windows-msvc-release\bin\NxResearchGraphBenchmark.exe
```

## Commit

```powershell
git add .
git commit -m "NRL-0009: Add Research Graph Foundation"
git push
```

## Filosofía

Este Sprint mantiene la regla del BOOK:

```txt
Nexiora may recommend. Only a human may promote.
```
