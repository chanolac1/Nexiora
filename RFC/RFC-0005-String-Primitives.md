# RFC-0005 — NXM-0003 String Primitives

## Estado
Aceptado para Nexiora 0.0.5.

## Objetivo
Crear las primeras primitivas de cadenas de Nexiora Core Platform para reducir dependencia directa del CRT y preparar implementaciones futuras especializadas por hardware.

## Alcance inicial
- `nx_string_length`
- `nx_string_length_bounded`
- `nx_string_compare`
- `nx_string_compare_n`
- `nx_string_copy`
- `nx_string_copy_n`
- `nx_string_find_char`
- `NxStringView`

## Principios
1. API C estable.
2. Costo mínimo por llamada.
3. Comportamiento definido para argumentos nulos cuando sea seguro.
4. Benchmarks obligatorios antes de optimización SIMD.
5. Implementación portable como referencia de corrección.

## Decisión
La versión 0.0.5 implementa una ruta portable conservadora. La ruta SIMD se pospone hasta que el HAL exponga dispatch estable por capacidades.

## Criterios de aceptación
- Compila en Windows.
- Pasa pruebas unitarias.
- Integra benchmarks con Evidence Engine.
- No rompe NXM-0001 Memory Manager ni NXM-0002 HAL.
