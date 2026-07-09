# NCOS-008 — Task Execution Engine

## Que logramos

Nexiora ahora puede ejecutar un plan como una lista persistente de tareas.

La capacidad nueva es:

1. Crear un plan ejecutable.
2. Persistir tareas con estado.
3. Ejecutar la siguiente tarea disponible.
4. Ejecutar todas las tareas pendientes.
5. Registrar un Action Log.
6. Consultar avance del plan.

Esto convierte al Planning Engine en algo accionable y prepara el Builder Core.

## Comandos

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-ncos-008.ps1
```

Prueba manual:

```powershell
.\Build\windows-msvc-release\bin\nexiora_task.exe create videojuego_demo "Crear un videojuego simple"
.\Build\windows-msvc-release\bin\nexiora_task.exe run-next videojuego_demo
.\Build\windows-msvc-release\bin\nexiora_task.exe run-all videojuego_demo
.\Build\windows-msvc-release\bin\nexiora_task.exe status videojuego_demo
.\Build\windows-msvc-release\bin\nexiora_task.exe log videojuego_demo
```

## Artefactos generados

```text
Knowledge/NCOS/Tasks/<plan>.tasks
Knowledge/NCOS/Tasks/<plan>.action.log
```

## Que habilita

NCOS-009 puede construir el Compiler Engine sobre esta base: una tarea podra compilar, capturar errores, registrar fallo y permitir autocorreccion.
