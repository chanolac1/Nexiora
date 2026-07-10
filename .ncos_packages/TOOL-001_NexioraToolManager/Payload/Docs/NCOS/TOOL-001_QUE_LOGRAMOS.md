# TOOL-001 — Qué logramos

Nexiora incorpora un Tool Manager local y auditable. Las herramientas administradas viven dentro de `Tools/Managed`, no alteran el PATH global, se descargan solo desde proveedores permitidos, se instalan de forma atómica y registran versión, origen y SHA-256. El primer proveedor automatizado es yt-dlp.
