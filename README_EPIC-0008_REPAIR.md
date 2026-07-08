# EPIC-0008 Repair

Corrige el problema de Ninja/CMake:

`manifest 'build.ninja' still dirty after 100 tries`

Causa probable: timestamps inconsistentes en archivos fuente/CMake después de descomprimir el paquete.

El reparador:

1. Normaliza timestamps de `CMakeLists.txt`, `CMakePresets.json`, `Include`, `Source`, `Tests` y `Scripts`.
2. Elimina `Build/windows-msvc-release` para forzar una regeneración limpia.
3. Reejecuta configure, build, tests y comandos de memoria.

Uso:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0008.ps1
```
