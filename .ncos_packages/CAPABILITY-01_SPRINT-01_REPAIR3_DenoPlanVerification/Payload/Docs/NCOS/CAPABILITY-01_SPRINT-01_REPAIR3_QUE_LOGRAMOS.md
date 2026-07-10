# Qué logramos — CAPABILITY-01 Sprint-01 Repair3

- Se eliminó la validación defectuosa que buscaba texto dentro del ejecutable.
- La CLI `plan` expone `managed_js_runtime=CONFIGURED` cuando el plan contiene Deno administrado.
- La verificación comprueba comportamiento real del comando `plan` y la ruta absoluta de Deno.
- Se conserva el descubrimiento flexible de archivos WebVTT.
- Se mantiene C23, cero warnings, pruebas, historial y rollback.
