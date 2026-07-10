# Qué logramos — CAPABILITY-01 Sprint-01 Repair1

- Deno se administra localmente en `Tools/Managed/deno`.
- yt-dlp recibe `--js-runtimes deno:<ruta>` y el componente EJS oficial.
- La adquisición aplica tres intentos con espera progresiva y progreso observable.
- Se reconocen nombres VTT variables como `<id>.en.es.vtt`.
- Puede habilitarse cookies del navegador de forma optativa mediante `NEXIORA_YTDLP_COOKIES_BROWSER=edge` o `chrome`.
- Un HTTP 429 persistente se reporta como bloqueo externo; no se declara aprendizaje exitoso.
- FFmpeg no es requerido para la ruta de solo subtítulos y no se descarga video.
