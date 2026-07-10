# PACKAGE STANDARD

## Regla principal

El Package Manager es el único mecanismo oficial de integración. No se permiten copias manuales.

## Requisitos de cada paquete

- Identificador y versión.
- Dependencias declaradas.
- Payload completo.
- `verify` y `deps` exitosos.
- Instalación idempotente y transaccional.
- Historial persistente.
- Rollback funcional.
- Código, pruebas, documentación y “Qué logramos”.

## Flujo obligatorio

`verify → deps → install → configure → build → focused tests → full tests → history → rollback verification`

## Mantenimiento documental autónomo

Todo paquete de Sprint debe incluir las versiones actualizadas de los documentos rectores afectados. La instalación del paquete actualiza esos documentos de manera transaccional junto con el código.

Antes de publicar un paquete, Nexiora debe:

1. actualizar `PROJECT_STATE.md` con el Sprint finalizado y el siguiente objetivo;
2. registrar capacidades en `CHANGELOG.md`;
3. registrar decisiones arquitectónicas relevantes en `DECISIONS.md`;
4. ejecutar la validación de sincronización documental;
5. rechazar el paquete si código, pruebas y documentación no son coherentes.

El operador no debe editar ni ejecutar actualizaciones documentales manuales.
