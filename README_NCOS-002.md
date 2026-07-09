# NCOS-002 — Planning Engine

Convierte una intención de Jorge en un plan persistente, verificable y consultable.

## Comandos

```powershell
.\Build\windows-msvc-release\bin\nexiora_plan.exe create "Crear un videojuego simple"
.\Build\windows-msvc-release\bin\nexiora_plan.exe status
.\Build\windows-msvc-release\bin\nexiora_plan.exe note "Definir loop de juego"
.\Build\windows-msvc-release\bin\nexiora_plan.exe done 1
```

## Artefactos

```txt
Knowledge/NCOS/Plans/<plan>.jsonl
Knowledge/NCOS/Plans/active_plan.txt
```
