# Qué logramos — CAPABILITY-01 Sprint-01 Repair4

- Se eliminó el bloqueo duro por ausencia de Deno.
- yt-dlp puede adquirir subtítulos con su modo compatible actual.
- Deno queda como acelerador opcional y se usa automáticamente cuando está disponible.
- Se conserva el descubrimiento flexible de archivos VTT.
- El paquete no incluye ni depende de scripts PowerShell.
- La instalación se realiza mediante `nexiora_package.exe`.

## Decisión

El runtime JavaScript no debe impedir el aprendizaje cuando la fuente ya entrega subtítulos sin él. La ausencia del runtime se reporta, pero no se trata como dependencia obligatoria.
