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
