# Qué logramos — EPIC-03 Sprint-02

Nexiora incorporó adaptadores reales para transformar medios binarios en evidencia textual:

- descubrimiento de Tesseract, FFmpeg y whisper-cli;
- OCR de imágenes;
- transcripción local de audio;
- extracción y transcripción de audio de video;
- planes reproducibles antes de ejecutar herramientas;
- rechazo de rutas inseguras, herramientas ausentes y salidas inexistentes;
- CLI `nexiora_decode`;
- pruebas activas en Release y Debug.

Los adaptadores no descargan modelos ni ocultan dependencias. La disponibilidad y el resultado quedan explícitamente verificados.
