# COGNITIVE-0001 — Cognitive Core inicial

## Verificar

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-cognitive-0001.ps1
```

## Ejercicios funcionales

### 1. Ingerir un archivo TXT o Markdown

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ingest Genexus .\Samples\Cognitive\genexus_intro.txt
```

### 2. Preguntar sobre lo ingerido

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ask Genexus "Para que sirve una Knowledge Base?"
```

### 3. Inspeccionar memoria

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe inspect Genexus
```

### 4. Probar con un archivo propio

Crea `Samples\Cognitive\mi_tema.md` con información técnica y ejecuta:

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ingest MiTema .\Samples\Cognitive\mi_tema.md
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ask MiTema "Que problema resuelve?"
```

### 5. Imagen o video con sidecar

Para una imagen/video/audio todavía se requiere un sidecar textual:

```powershell
# Ejemplo
# video.mp4
# video.mp4.txt  <- descripcion o transcripcion

.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ingest CursoGenexus .\Samples\Cognitive\video.mp4
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ask CursoGenexus "Que explica el video?"
```

Si no existe sidecar, Nexiora registra el archivo como metadata-only y avisa que falta extractor.

## Commit

```powershell
git add .
git commit -m "COGNITIVE-0001: Add initial cognitive core"
git push
```
