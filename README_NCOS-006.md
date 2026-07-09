# NCOS-006 — Answer Composer

## Qué logramos

Nexiora ahora puede convertir tarjetas de concepto y relaciones del grafo en una respuesta estructurada con:

- definición del concepto origen;
- propósito;
- relación directa o inferida;
- evidencia usada;
- confianza.

Esto acerca a Nexiora a responder como un sistema cognitivo, no como un listado de fragmentos.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-006.ps1
```

## Prueba manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_answer.exe explain Genexus DataSelector KnowledgeBase
```

## Commit

```powershell
git add .
git commit -m "NCOS-006: Add structured answer composer"
git push
```
