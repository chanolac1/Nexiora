# CHANGELOG

Todos los cambios relevantes de Nexiora se registran aquí.

## [Unreleased]

### Added

- Documentation Governance Engine.
- Conjunto documental rector en `Docs/`.
- Validación automática de presencia y estructura documental.
- Actualización transaccional de estado, capacidades y decisiones.

## [DOCS-001] — 2026-07-10

### Added

- `MASTER_CONTEXT.md`, `PROJECT_STATE.md`, `ROADMAP.md`, `CHANGELOG.md`, `DECISIONS.md`, `ARCHITECTURE.md`, `CODING_STANDARD.md`, `TESTING_STANDARD.md` y `PACKAGE_STANDARD.md`.

## [DOCS-001-REPAIR] — 2026-07-10

### Fixed

- Se corrigió el flujo de instalación para respetar la interfaz real de NCOS-020: `verify`, `deps` e `install` reciben únicamente `<package_dir>`.
- Se eliminó la actualización documental manual del flujo operativo del usuario.
- Se formalizó que cada paquete de Sprint transporta, valida, instala y revierte su documentación sincronizada.
