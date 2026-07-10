# NCOS-016 — Nexiora Package Manager

## Qué logramos

Nexiora ahora tiene un instalador propio para paquetes NCOS.

El objetivo es eliminar los errores repetidos de instalación manual:

- `Copy-Item` sobre sí mismo.
- Rutas calculadas incorrectamente.
- Aplicaciones parciales.
- Verificadores no idempotentes.
- Reinstalaciones que rompen estado previo.

## Capacidades incluidas

- Manifest `manifest.npkg`.
- Smart copy desde `Payload`.
- Comparación de contenido por hash.
- Omitir archivos idénticos.
- Backup de archivos modificados.
- Registro local por paquete.
- Log de instalación.
- CLI `nexiora_package`.
- Test unitario aislado.

## Comandos

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-016.ps1
```

Prueba manual:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe install .\.ncos_packages\NCOS-016_NexioraPackageManager
.\Build\windows-msvc-release\bin\nexiora_package.exe status "NCOS-016 Nexiora Package Manager"
```

## Commit sugerido

```powershell
git add .
git commit -m "NCOS-016: Add Nexiora Package Manager"
git push
```
