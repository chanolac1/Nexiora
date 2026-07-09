param(
    [Parameter(Mandatory=$true)][string]$Topic
)

$ErrorActionPreference = 'Stop'

function Slugify([string]$Text) {
    $s = $Text.ToLowerInvariant() -replace '[^a-z0-9]+','-'
    $s = $s.Trim('-')
    if ([string]::IsNullOrWhiteSpace($s)) { return 'topic' }
    return $s
}

function Write-Step([int]$Pct, [string]$Text) {
    $filled = [int][Math]::Round($Pct / 5)
    if ($filled -lt 0) { $filled = 0 }
    if ($filled -gt 20) { $filled = 20 }
    $bar = ('#' * $filled) + ('.' * (20 - $filled))
    Write-Host ("[{0}] {1,3}% {2}" -f $bar, $Pct, $Text)
}

function JsonEscape([string]$Text) {
    if ($null -eq $Text) { return '' }
    return ($Text -replace '\\','\\' -replace '"','\"' -replace "`r",' ' -replace "`n",' ')
}

function Add-Fact([System.Collections.Generic.List[string]]$List, [string]$Text) {
    if (-not [string]::IsNullOrWhiteSpace($Text)) {
        $clean = ($Text -replace '\s+',' ').Trim()
        if (-not $List.Contains($clean)) { $List.Add($clean) | Out-Null }
    }
}

function Add-Concept([System.Collections.Generic.List[string]]$List, [string]$Text) {
    if (-not [string]::IsNullOrWhiteSpace($Text)) {
        $clean = ($Text -replace '\s+',' ').Trim()
        if (-not $List.Contains($clean)) { $List.Add($clean) | Out-Null }
    }
}

$language = 'es-MX'
$slug = Slugify $Topic
$outDir = Join-Path (Join-Path (Join-Path '.' 'Knowledge') 'Topics') $slug
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$memoryPath = Join-Path $outDir 'memory.jsonl'
$reportPath = Join-Path $outDir 'report.md'
$answerPath = Join-Path $outDir 'answer.txt'
$sourcesPath = Join-Path $outDir 'sources.json'

Write-Step 20 'Preparando aprendizaje en espanol'

$sources = New-Object System.Collections.Generic.List[object]
$facts = New-Object System.Collections.Generic.List[string]
$concepts = New-Object System.Collections.Generic.List[string]
$title = $Topic
$summary = ''

# Conocimiento curado minimo para temas frecuentes del usuario/proyecto.
if ($Topic -match '(?i)^genexus$') {
    $title = 'GeneXus'
    $sources.Add([pscustomobject]@{ type='official'; title='Documentacion oficial de GeneXus'; url='https://docs.genexus.com/' }) | Out-Null
    Add-Fact $facts 'GeneXus es una plataforma low-code orientada a crear, mantener y evolucionar aplicaciones empresariales a partir de una base de conocimiento.'
    Add-Fact $facts 'En GeneXus, la base de conocimiento describe objetos como transacciones, reglas, procedimientos, paneles, APIs y generadores para producir aplicaciones en distintas plataformas.'
    Add-Fact $facts 'El enfoque principal de GeneXus es modelar conocimiento de negocio y generar codigo para multiples tecnologias, reduciendo trabajo manual repetitivo.'
    Add-Fact $facts 'Para Nexiora, GeneXus es relevante como tema de investigacion por su enfoque de modelado, generacion automatica, bases de conocimiento y productividad empresarial.'
    foreach ($c in @('Base de conocimiento','Transaccion','Reglas','Generadores','Low-code','WorkWithPlus','Procedimientos','APIs','Modelado de negocio')) { Add-Concept $concepts $c }
}

# Provider 1: Wikipedia en espanol, si existe.
try {
    $encoded = [uri]::EscapeDataString($Topic)
    $searchUrl = "https://es.wikipedia.org/w/api.php?action=opensearch&search=$encoded&limit=1&namespace=0&format=json"
    Write-Step 35 'Buscando fuente en espanol: Wikipedia'
    $search = Invoke-RestMethod -Uri $searchUrl -TimeoutSec 20 -Headers @{ 'User-Agent' = 'NexioraResearchBot/0.1 (es-MX)' }
    if ($search.Count -ge 2 -and $search[1].Count -gt 0) {
        $title = [string]$search[1][0]
        $titleEncoded = [uri]::EscapeDataString($title.Replace(' ', '_'))
        $summaryUrl = "https://es.wikipedia.org/api/rest_v1/page/summary/$titleEncoded"
        Write-Step 48 "Descargando resumen en espanol: $title"
        $wiki = Invoke-RestMethod -Uri $summaryUrl -TimeoutSec 20 -Headers @{ 'User-Agent' = 'NexioraResearchBot/0.1 (es-MX)' }
        if ($wiki.extract) {
            $summary = [string]$wiki.extract
            Add-Fact $facts $summary
            $pageUrl = ''
            try { $pageUrl = [string]$wiki.content_urls.desktop.page } catch { $pageUrl = "https://es.wikipedia.org/wiki/$titleEncoded" }
            $sources.Add([pscustomobject]@{ type='wikipedia-es'; title=$title; url=$pageUrl }) | Out-Null
        }
    }
} catch {
    Add-Fact $facts "No fue posible consultar Wikipedia en espanol para '$Topic': $($_.Exception.Message)"
}

# Provider 2: DuckDuckGo con preferencia regional en espanol.
try {
    $encoded = [uri]::EscapeDataString($Topic)
    $ddgUrl = "https://api.duckduckgo.com/?q=$encoded&format=json&no_redirect=1&no_html=1&kl=es-es"
    Write-Step 62 'Consultando fuente complementaria en espanol'
    $ddg = Invoke-RestMethod -Uri $ddgUrl -TimeoutSec 20 -Headers @{ 'User-Agent' = 'NexioraResearchBot/0.1 (es-MX)' }
    if ($ddg.AbstractText) {
        Add-Fact $facts ([string]$ddg.AbstractText)
        if ($ddg.AbstractURL) { $sources.Add([pscustomobject]@{ type='duckduckgo'; title=[string]$ddg.Heading; url=[string]$ddg.AbstractURL }) | Out-Null }
    }
    if ($ddg.RelatedTopics) {
        foreach ($rt in $ddg.RelatedTopics | Select-Object -First 6) {
            if ($rt.Text) { Add-Fact $facts ([string]$rt.Text) }
        }
    }
} catch {
    Add-Fact $facts "La fuente complementaria no estuvo disponible para '$Topic': $($_.Exception.Message)"
}

Write-Step 75 'Extrayendo conceptos en espanol'

# Conceptos derivados simples: nombres propios y frases tecnicas conocidas.
Add-Concept $concepts $title
Add-Concept $concepts $Topic
foreach ($fact in $facts) {
    $matches = [regex]::Matches($fact, '\b[A-ZÁÉÍÓÚÑ][A-Za-zÁÉÍÓÚÑáéíóúñ0-9\+\.#-]{2,}\b')
    foreach ($m in $matches) {
        if ($concepts.Count -lt 30) { Add-Concept $concepts $m.Value }
    }
}
if ($concepts.Count -eq 0) { Add-Concept $concepts $Topic }

Write-Step 86 'Guardando memoria en espanol'

Remove-Item -Force -ErrorAction SilentlyContinue $memoryPath
Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"topic","language":"' + $language + '","name":"' + (JsonEscape $Topic) + '","title":"' + (JsonEscape $title) + '","confidence":75}')
foreach ($src in $sources) {
    Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"source","language":"' + $language + '","topic":"' + (JsonEscape $Topic) + '","source_type":"' + (JsonEscape $src.type) + '","title":"' + (JsonEscape $src.title) + '","url":"' + (JsonEscape $src.url) + '"}')
}
foreach ($c in ($concepts | Select-Object -Unique | Select-Object -First 25)) {
    Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"concept","language":"' + $language + '","topic":"' + (JsonEscape $Topic) + '","name":"' + (JsonEscape $c) + '","confidence":72}')
}
foreach ($fact in ($facts | Select-Object -First 12)) {
    Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"fact","language":"' + $language + '","topic":"' + (JsonEscape $Topic) + '","text":"' + (JsonEscape $fact) + '","confidence":70}')
}

$sources | ConvertTo-Json -Depth 4 | Set-Content -Path $sourcesPath -Encoding UTF8

$mainFact = if ($facts.Count -gt 0) { $facts[0] } else { "No se obtuvo informacion externa suficiente sobre $Topic. La memoria conserva el tema para investigacion posterior." }
$conceptText = ($concepts | Select-Object -Unique | Select-Object -First 12) -join ', '
$sourceText = ($sources | ForEach-Object { "- $($_.title): $($_.url)" }) -join "`n"
if ([string]::IsNullOrWhiteSpace($sourceText)) { $sourceText = '- Sin fuentes externas confiables registradas.' }

$answer = @"
Tema: $Topic
Titulo detectado: $title
Idioma de respuesta: Espanol (es-MX)
Confianza: 75 %

Resumen:
$mainFact

Conceptos relacionados:
$conceptText

Fuentes:
$sourceText

Nota: Esta respuesta proviene de la memoria local de Nexiora. Si la fuente original estaba en otro idioma, Nexiora conserva la explicacion en espanol siempre que sea posible.
"@
$answer | Set-Content -Path $answerPath -Encoding UTF8

$report = @"
# Investigacion: $Topic

Idioma de trabajo: espanol (es-MX)

## Resumen

$mainFact

## Conceptos extraidos

$(($concepts | Select-Object -Unique | ForEach-Object { "- $_" }) -join "`n")

## Fuentes

$sourceText

## Hechos registrados

$(($facts | Select-Object -First 12 | ForEach-Object { "- $_" }) -join "`n")

## Estado

Memoria persistente actualizada en: `$memoryPath
"@
$report | Set-Content -Path $reportPath -Encoding UTF8

Write-Step 100 'Aprendizaje completado en espanol'
Write-Host "Reporte: $reportPath"
Write-Host "Memoria: $memoryPath"
