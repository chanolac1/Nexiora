# NCOS-003 — Concept Registry

Primera pieza del Knowledge Representation Engine.

Agrega tarjetas de concepto persistentes en:

```text
Knowledge/NCOS/Concepts/<dominio>/<concepto>.card
```

Cada tarjeta guarda:

- dominio
- nombre
- identidad normalizada
- definición
- propósito
- relaciones
- confianza
- versión

## Pruebas

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-003.ps1
```

## Uso manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_concept.exe upsert Genexus DataSelector "Un DataSelector define una consulta reutilizable sobre datos." "Sirve para evitar repetir filtros." "Data Provider,Transaction,Procedure"
.\Build\windows-msvc-release\bin\nexiora_concept.exe show Genexus DataSelector
```
