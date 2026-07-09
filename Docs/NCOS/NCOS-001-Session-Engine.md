# NCOS-001 — Session Engine

Primera pieza del NCOS Kernel.

## Capacidad

Permite crear una sesión persistente de trabajo para que Nexiora recuerde el objetivo activo, notas y eventos de colaboración.

## Comandos

```powershell
nexiora_session start videojuego "Crear un videojuego simple"
nexiora_session note "Definir loop de juego"
nexiora_session status
nexiora_session close
```

## Artefactos

```text
Knowledge/NCOS/Sessions/active_session.txt
Knowledge/NCOS/Sessions/<session>.jsonl
```

## Propósito

Esta capacidad prepara a Nexiora para trabajo continuo: construir proyectos, retomar contexto, registrar decisiones y evolucionar con experiencia acumulada.
