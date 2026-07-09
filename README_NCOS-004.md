# NCOS-004 — Concept Graph

Agrega el primer grafo de conceptos estructurados para NCOS.

## Qué agrega

- `NxConceptGraph`: relaciones persistentes entre conceptos.
- `nexiora_graph`: CLI para crear y consultar relaciones.
- `NxConceptGraphTests`: prueba de comportamiento aislada.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-004.ps1
```

## Uso manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_graph.exe link Genexus DataSelector related_to DataProvider
.\Build\windows-msvc-release\bin\nexiora_graph.exe link Genexus DataSelector used_by Procedure
.\Build\windows-msvc-release\bin\nexiora_graph.exe show Genexus DataSelector
.\Build\windows-msvc-release\bin\nexiora_graph.exe stats Genexus DataSelector
```

## Artefacto generado

```text
Knowledge/NCOS/ConceptGraphs/genexus/edges.jsonl
```

## Commit

```powershell
git add .
git commit -m "NCOS-004: Add concept graph"
git push
```
