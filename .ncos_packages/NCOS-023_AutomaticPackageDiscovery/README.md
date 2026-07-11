# NCOS-023 — Automatic Package Discovery

Evoluciona `nexiora_package.exe` para descubrir y aplicar automáticamente el paquete pendiente más reciente.

Después del bootstrap de esta entrega, los paquetes futuros se instalan ejecutando únicamente:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe
```
