# NCOS-021 — Native Package Apply Workflow

## Qué logramos

Nexiora incorpora `nexiora_package apply <package_dir>` como flujo nativo de certificación.

El comando ejecuta verificación, dependencias, instalación transaccional, configuración CMake, build, materialización de pruebas, compuerta de cero warnings, CTest, validación documental y rollback exacto de la transacción si una fase falla.

No usa `verify_install.ps1` ni scripts externos.

## Evidencia

El resultado muestra cada fase, el ID de transacción, la fase fallida, el estado del rollback y la ruta de `apply.log`.
