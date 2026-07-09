# NCOS-001 — Session Engine

Entrega funcional del primer componente del NCOS Kernel: sesiones persistentes.

## Verificar

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-001.ps1
```

## Probar manualmente

```powershell
.\Build\windows-msvc-release\bin\nexiora_session.exe start videojuego "Crear un videojuego simple"
.\Build\windows-msvc-release\bin\nexiora_session.exe note "Definir loop de juego"
.\Build\windows-msvc-release\bin\nexiora_session.exe status
.\Build\windows-msvc-release\bin\nexiora_session.exe close
```

## Commit

```powershell
git add .
git commit -m "NCOS-001: Add persistent session engine"
git push
```
