# ARCHITECTURE

## Visión general

Nexiora es un Sistema Operativo Cognitivo Autónomo en C23.

## Capas principales

- **CORE:** runtime, memoria, contenedores y primitivas.
- **COGNITIVE:** investigación, conocimiento, razonamiento y aprendizaje.
- **NCOS:** sesiones, planificación, ejecución, compilación, parches, validación y evolución.
- **TOOLS:** interfaces CLI verificables.
- **KNOWLEDGE:** estado y conocimiento persistente.
- **PACKAGE MANAGER:** instalación transaccional, dependencias, historial y rollback.

## Documentation Governance Engine

Valida el conjunto documental rector y proporciona operaciones estructuradas para registrar finalizaciones de Sprint, capacidades y decisiones. No modifica código ni declara un Sprint completado sin que el orquestador haya validado compilación y pruebas.
