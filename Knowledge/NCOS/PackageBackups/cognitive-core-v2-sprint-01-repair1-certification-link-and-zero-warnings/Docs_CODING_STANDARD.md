# CODING STANDARD

- C23 for GCC/UCRT64 builds.
- CMake and Ninja only.
- `-Wall -Wextra -Wpedantic` with zero warnings.
- No warning suppression used as a repair.
- Bounds-checked path and string composition.
- One canonical declaration and implementation per public API.
- Deterministic behavior and explicit error states.

## Regla de orquestadores cognitivos
Los orquestadores deben reutilizar contratos existentes, no duplicar algoritmos de razonamiento, y deben propagar estados explícitos como `OK`, `CONTRADICTORY_EVIDENCE` y `GAP_OPENED`.
## Certificación
Todo nuevo artefacto público debe declararse con `artifact=` en el manifiesto y verificarse durante Apply.

