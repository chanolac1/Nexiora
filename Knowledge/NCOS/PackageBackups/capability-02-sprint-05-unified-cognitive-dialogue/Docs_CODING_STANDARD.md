# CODING STANDARD

- C23 for GCC/UCRT64 builds.
- CMake and Ninja only.
- `-Wall -Wextra -Wpedantic` with zero warnings.
- No warning suppression used as a repair.
- Bounds-checked path and string composition.
- One canonical declaration and implementation per public API.
- Deterministic behavior and explicit error states.
