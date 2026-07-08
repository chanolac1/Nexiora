# EPIC-0007 — Conversación con el Laboratorio

Esta Épica agrega la primera consola conversacional de Nexiora en español.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0007.ps1
```

## Uso

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe
```

Comandos iniciales:

```text
ayuda
estado
investigar
sesiones
ultima sesion
que aprendiste
recomendaciones
sorprendeme
salir
```

## Commit

```powershell
git add .
git commit -m "EPIC-0007: Add Laboratory Conversation"
git push
```
