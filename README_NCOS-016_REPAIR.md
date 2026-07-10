# NCOS-016 REPAIR - Package Manager Test Repeatability

Corrige `NxPackageManagerTests` para que sea repetible e independiente del estado previo en `Build/`.

## Qué corrige

- El primer install podía saltar el archivo si quedaba un archivo idéntico de una corrida previa.
- El test ahora limpia sus archivos específicos antes de ejecutar.
- Mantiene la validación clave: primer install copia, segundo install omite por hash idéntico.

## Ejecutar

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-016-repair.ps1
```
