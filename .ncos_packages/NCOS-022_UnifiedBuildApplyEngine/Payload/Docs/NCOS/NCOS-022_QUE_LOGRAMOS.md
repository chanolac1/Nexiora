# NCOS-022 — Unified Build & Apply Engine

## Qué logramos

`nexiora_package apply <package_dir>` ejecuta de forma nativa la certificación completa del paquete: verify, dependencias, instalación transaccional, configuración CMake, materialización de pruebas, compuerta de cero warnings, CTest, validación documental e historial.

Si una fase posterior a la instalación falla, el motor usa el `transaction_id` recién creado y revierte exactamente esa transacción.

## Contrato

- Sin scripts externos como instalador oficial.
- C23, CMake, Ninja y MSYS2 UCRT64.
- Cero warnings.
- Suite completa obligatoria.
- Documentación sincronizada.
- Rollback automático.
- Log persistente en `Knowledge/NCOS/PackageApply/apply.log`.
