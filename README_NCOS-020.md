# NCOS-020 — Package History & Recovery

## Qué logramos

- Historial persistente de transacciones por paquete.
- ID secuencial para cada instalación con cambios reales.
- Consulta del historial desde CLI.
- Rollback dirigido a una transacción específica.
- Las reinstalaciones sin cambios conservan el historial.

## Instalación

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe verify .\.ncos_packages\NCOS-020_PackageHistoryRecovery
.\Build\windows-msvc-release\bin\nexiora_package.exe deps .\.ncos_packages\NCOS-020_PackageHistoryRecovery
.\Build\windows-msvc-release\bin\nexiora_package.exe install .\.ncos_packages\NCOS-020_PackageHistoryRecovery
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```
