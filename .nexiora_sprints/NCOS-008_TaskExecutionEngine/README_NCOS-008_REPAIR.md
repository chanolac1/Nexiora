# NCOS-008 Repair

Corrige rutas del verificador y aplicador para que siempre detecten la raiz real del repositorio mediante `CMakePresets.json`.

Tambien evita `Copy-Item` sobre si mismo y hace repetible el smoke test limpiando el caso `videojuego_demo` antes de ejecutarlo.
