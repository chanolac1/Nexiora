# UI-001 Repair1 — Win32 Entry Point and Output Directory

## Qué logramos

- Se corrigió el enlace de la aplicación Unicode Win32 con MinGW mediante `-municode`.
- Se preservó `wWinMain` como punto de entrada nativo Unicode.
- Se corrigió la transferencia de `NEXIORA_OUTPUT_DIR` desde PowerShell a CMake.
- CMake rechaza ahora rutas de salida vacías o relativas.
- La instalación verifica que `nexiora_control_center.exe` se genere exactamente en `Build/windows-msvc-release/bin`.
- Se mantienen C23, cero warnings, pruebas, historial y rollback transaccional.

## Causa raíz

La aplicación se enlazaba con `-mwindows`, pero MinGW buscaba `WinMain` porque no se había habilitado el startup Unicode. Además, PowerShell enviaba el texto literal `$outputDir` en lugar de la ruta resuelta.
