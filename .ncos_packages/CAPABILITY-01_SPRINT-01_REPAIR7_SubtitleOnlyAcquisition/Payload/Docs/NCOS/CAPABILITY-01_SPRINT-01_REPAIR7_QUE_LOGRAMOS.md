# CAPABILITY-01 Sprint-01 Repair7 — Subtitle-Only Acquisition

## Qué logramos

- Eliminamos los warnings operativos de yt-dlp en el flujo de solo subtítulos.
- El pipeline usa `youtube:player_client=default`, una política explícita compatible con adquisición de subtítulos sin runtime JavaScript obligatorio.
- El pipeline selecciona un único formato (`best`) aunque `--skip-download` impide descargar video; esto evita la selección automática de combinaciones que requieren FFmpeg.
- No se usa `--no-warnings` ni se filtra la salida: se corrige la causa de ambos avisos.
- Se agregaron pruebas de regresión sobre el plan generado.
