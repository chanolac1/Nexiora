# RFC-0002 — Nexiora Evidence Engine

## Estado
Aceptado para Nexiora 0.0.2.

## Propósito
El Evidence Engine registra resultados de benchmark, compara mediciones contra la línea base previa y detecta mejoras o regresiones.

## Reglas
1. Todo benchmark debe registrar historial reproducible.
2. Toda mejora se marca como candidata.
3. Ninguna mejora se promueve automáticamente.
4. Toda promoción requiere aprobación humana explícita.
5. Toda regresión debe bloquearse hasta revisión.

## Archivos principales
- `Include/Nexiora/NCP/Evidence/NxEvidence.h`
- `Source/NCP/Evidence/NxEvidence.c`
- `Benchmarks/History/nexiora_bench_history.csv`
- `Benchmarks/Reports/latest_evidence_report.txt`
- `Benchmarks/Approvals/`

## Criterio inicial
Una diferencia mayor a 1.0% se considera significativa para esta versión preliminar. En versiones futuras se sustituirá por análisis estadístico con múltiples corridas.
