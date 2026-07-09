# NCOS-007 — Intent-to-Plan Bridge

## Qué logramos

Nexiora ahora puede convertir una petición natural en un plan persistente:

```txt
"crear un videojuego simple"
        ↓
intención: build_project
        ↓
objetivo normalizado: Construir proyecto: un videojuego simple
        ↓
Planning Engine
        ↓
plan persistente con pasos verificables
```

Esto conecta conversación/intención con planificación. Es un paso necesario para que, más adelante, Nexiora pueda construir proyectos completos.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-007.ps1
```

## Prueba manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_intent_plan.exe "crear un videojuego simple"
.\Build\windows-msvc-release\bin\nexiora_plan.exe status
```

## Commit

```powershell
git add .
git commit -m "NCOS-007: Add intent-to-plan bridge"
git push
```
