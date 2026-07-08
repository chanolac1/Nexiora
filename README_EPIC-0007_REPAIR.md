# EPIC-0007 Repair

Corrige el error de PowerShell:

```text
El operador -ireplace solo permite que lo sigan dos elementos, no 4.
```

Además fuerza una configuración limpia para integrar realmente la consola conversacional.

## Uso

Descomprimir sobre `D:\Nexiora` y ejecutar:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0007.ps1
```

Después probar:

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe
```

Comandos dentro de Nexiora:

```text
ayuda
estado
que aprendiste
recomendaciones
sorprendeme
salir
```
