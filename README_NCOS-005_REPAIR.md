# NCOS-005 Repair

Corrige la verificación de NCOS-005 sin tocar el motor principal.

## Qué corrige

1. Limpia estado persistido de `Knowledge/NCOS/Concepts/genexus` y `Knowledge/NCOS/ConceptGraphs/genexus` antes de correr tests.
2. Evita que `NxConceptRegistryTests` falle porque un concepto previo ya tenía versión mayor a 1.
3. Cambia la validación del smoke test a comportamiento real:
   - relación indirecta detectada;
   - origen `dataselector`;
   - destino `knowledgebase`;
   - intermedio `dataprovider`;
   - evidencia y confianza presentes.

## Uso

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-005-repair.ps1
```
