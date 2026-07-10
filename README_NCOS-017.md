# NCOS-017 — Package Verification Workflow

## Qué logramos

Nexiora ahora puede verificar un paquete antes de instalarlo.

Esto agrega al Package Manager:

- comando `verify <package_dir>`;
- validación de `manifest.npkg`;
- conteo de archivos declarados;
- detección de payload faltante;
- prueba de regresión para manifest/payload;
- documentación del nuevo flujo de despliegue.

## Instalación con el nuevo flujo

Después de descomprimir este ZIP sobre `D:\Nexiora`, usa el Package Manager existente:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe install .\.ncos_packages\NCOS-017_PackageVerificationWorkflow
```

Luego recompila:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```

A partir de esa recompilación ya estará disponible:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe verify .\.ncos_packages\NCOS-017_PackageVerificationWorkflow
```

## Verificación guiada

También puedes ejecutar:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-017.ps1
```

Este script usa el Package Manager, no copia archivos manualmente.
