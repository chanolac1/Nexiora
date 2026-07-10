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
