# Nexiora 0.0.2 — Evidence Engine

Esta versión agrega el primer sistema de evidencia de Nexiora.

## Ejecutar flujo completo

```powershell
.\Scripts\nxbuild.ps1 all
```

## Ejecutar solo benchmark

```powershell
.\Scripts\nxbuild.ps1 benchmark -Iterations 100000
```

## Archivos generados

```text
Benchmarks\History\nexiora_bench_history.csv
Benchmarks\Reports\latest_evidence_report.txt
Benchmarks\Approvals\
```

## Aprobar candidato

Si el benchmark muestra `improved candidate`, se debe aprobar manualmente:

```powershell
.\Scripts\nxbuild.ps1 approve
```

Esto crea un archivo de aprobación en:

```text
Benchmarks\Approvals\approval-YYYYMMDD-HHMMSS.txt
```

## Notas

En 0.0.2 la comparación usa la última medición como línea base. En próximas versiones habrá baseline estable, promedio de múltiples corridas y percentiles.
