# NCOS Package Deployment Workflow

Este documento define el flujo estándar para instalar entregas nuevas de Nexiora sin aplicar cambios manuales.

## Flujo recomendado

1. Descomprimir el ZIP de entrega sobre la raíz del repositorio.
2. Verificar el paquete:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe verify .\.ncos_packages\NCOS-017_PackageVerificationWorkflow
```

3. Instalar el paquete:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe install .\.ncos_packages\NCOS-017_PackageVerificationWorkflow
```

4. Reconfigurar, compilar y ejecutar pruebas:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```

5. Confirmar estado instalado:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe status "NCOS-017 Package Verification Workflow"
```

## Regla de oro

Los scripts ya no deben copiar archivos manualmente. El paquete declara archivos en `manifest.npkg` y el Package Manager decide:

- instalar,
- omitir si ya es idéntico,
- respaldar si reemplaza,
- registrar evidencia.

## Qué habilita

A partir de esta entrega, cada nuevo paquete puede validarse antes de instalarse. Esto reduce errores de rutas, payload incompleto y aplicaciones parciales.
