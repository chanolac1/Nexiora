# TOOL-001 Repair1 — Direct Process Execution

Corrige la integración entre `NxWebCognitivePipeline` y el Tool Manager. Las herramientas administradas y el motor cognitivo ya no se ejecutan mediante `system()` ni a través de `cmd.exe`. Nexiora resuelve la raíz a una ruta absoluta y crea procesos con argumentos separados, evitando que `root="."` sea interpretado como comando, así como errores de comillas y espacios en rutas.
