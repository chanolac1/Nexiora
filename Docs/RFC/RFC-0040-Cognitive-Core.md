# RFC-0040 — Cognitive Core

## Decisión

Nexiora tendrá un núcleo cognitivo unificado para convertir distintas entradas en memoria consultable.

## Motivación

El usuario necesita que Nexiora analice archivos, imágenes, videos y otros medios. El primer incremento debe ser funcional y no un cascarón: ingesta real de archivos textuales y consulta sobre memoria generada.

## Política

Todo conector debe producir los mismos artefactos base:

- `sources.txt`
- `chunks.txt`
- `concepts.txt`
- `memory.jsonl`
- `answer.txt`

## Evolución

Los siguientes conectores agregarán OCR, transcripción, PDF y web. Cada uno debe alimentar el mismo Cognitive Core.
