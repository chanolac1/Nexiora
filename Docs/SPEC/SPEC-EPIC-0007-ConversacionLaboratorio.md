# SPEC — EPIC-0007 Conversación con el Laboratorio

## Objetivo

Permitir ejecutar `nexiora.exe` sin argumentos para abrir una consola conversacional en español.

## Comandos soportados

- ayuda
- estado
- investigar
- sesiones
- ultima sesion
- que aprendiste
- recomendaciones
- sorprendeme
- salir

También se aceptan frases simples como:

- ¿Cómo vas?
- Investiga Scheduler
- ¿Qué aprendiste?
- ¿Qué recomiendas?

## Componentes

- `NxConversation.h`
- `NxConversation.c`
- `NxConversationTests.c`
- Integración en `NxBootstrap.c`

## Criterios de aceptación

- El proyecto compila.
- `NxConversationTests` pasa.
- `nexiora.exe` sin argumentos abre la consola.
- `ayuda` muestra comandos disponibles.
- `salir` cierra la conversación sin modificar el Runtime.
