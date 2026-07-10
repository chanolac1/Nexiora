# DECISIONS

Registro de decisiones arquitectónicas de Nexiora.

## ADR-0001 — Documentación como parte del estado del sistema

**Estado:** Aceptada  
**Fecha:** 2026-07-10

### Contexto

La evolución autónoma requiere que el código y su explicación técnica permanezcan sincronizados.

### Decisión

Los documentos rectores de `Docs/` son artefactos versionados, probados e instalados exclusivamente mediante el Package Manager. Cada Sprint debe actualizar `PROJECT_STATE.md` y `CHANGELOG.md`; las decisiones arquitectónicas importantes deben añadirse aquí.

### Consecuencias

Una entrega con documentación ausente o estructuralmente inválida debe rechazarse.

## ADR-0002 — Documentación como parte transaccional de cada Sprint

**Estado:** Aceptada  
**Fecha:** 2026-07-10

Los documentos rectores se versionan y distribuyen dentro del mismo paquete instalable que el código del Sprint. Nexiora los actualiza durante la preparación de la entrega y el Package Manager los instala o revierte junto con los demás artefactos. No se requieren ediciones documentales manuales del operador.
