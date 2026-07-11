# NCOS-024 Repair1 — Test Preset and CLI Materialization

Corrige dos defectos de NCOS-024:

1. `apply.test_preset` apuntaba a `windows-msvc-release`, que es un configure preset, no un test preset. Ahora usa `release-tests`.
2. `nexiora_test_suite` no exigía construir `nexiora_workspace.exe`. Ahora la CLI y `NxWorkspaceEngineTests.exe` son artefactos obligatorios.

Instalación: descomprimir sobre la raíz de Nexiora y ejecutar `nexiora_package.exe` sin argumentos.
