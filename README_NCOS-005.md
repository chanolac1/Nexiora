# NCOS-005 — Graph Reasoning

## Qué logramos

Con esta entrega Nexiora deja de solo **guardar relaciones** y empieza a **explicarlas**.

Ahora puede responder por qué dos conceptos están relacionados usando el grafo de conocimiento:

- relación directa;
- relación indirecta de hasta 2 saltos;
- evidencia usada;
- confianza de la explicación.

Esto es un primer paso hacia el `Reasoning Engine` real.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-005.ps1
```

## Prueba manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_graph.exe link Genexus DataSelector related_to DataProvider
.\Build\windows-msvc-release\bin\nexiora_graph.exe link Genexus DataProvider belongs_to KnowledgeBase
.\Build\windows-msvc-release\bin\nexiora_reason.exe why Genexus DataSelector KnowledgeBase
```

## Resultado esperado

Nexiora debe explicar que `DataSelector` se relaciona indirectamente con `KnowledgeBase` a través de `DataProvider`, mostrando evidencia y confianza.

## Commit

```powershell
git add .
git commit -m "NCOS-005: Add graph reasoning"
git push
```
