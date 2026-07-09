# CORE-0002 - Salida y memoria en español

Reemplaza `Scripts/nexiora-learn.ps1` para que el aprendizaje guarde respuestas, reportes y memoria en español (`es-MX`) siempre que sea posible.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0002-spanish.ps1
```

## Uso

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
.\Build\windows-msvc-release\bin\nexiora.exe que sabes Genexus
```

## Commit

```powershell
git add .
git commit -m "CORE-0002: Normalize learning output to Spanish"
git push
```
