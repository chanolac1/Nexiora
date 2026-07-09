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

$slug = Slugify $Topic
$outDir = Join-Path (Join-Path (Join-Path '.' 'Knowledge') 'Topics') $slug
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$memoryPath = Join-Path $outDir 'memory.jsonl'
$reportPath = Join-Path $outDir 'report.md'
$answerPath = Join-Path $outDir 'answer.txt'
$sourcesPath = Join-Path $outDir 'sources.json'

Write-Step 35 'Buscando fuentes publicas'

$sources = New-Object System.Collections.Generic.List[object]
$facts = New-Object System.Collections.Generic.List[string]
$concepts = New-Object System.Collections.Generic.List[string]
$summary = ''
$title = $Topic

# Provider 1: Wikipedia OpenSearch + REST summary
try {
    $encoded = [uri]::EscapeDataString($Topic)
    $searchUrl = "https://en.wikipedia.org/w/api.php?action=opensearch&search=$encoded&limit=1&namespace=0&format=json"
    $search = Invoke-RestMethod -Uri $searchUrl -TimeoutSec 20 -Headers @{ 'User-Agent' = 'NexioraResearchBot/0.1' }
    if ($search.Count -ge 2 -and $search[1].Count -gt 0) {
        $title = [string]$search[1][0]
        $titleEncoded = [uri]::EscapeDataString($title.Replace(' ', '_'))
        $summaryUrl = "https://en.wikipedia.org/api/rest_v1/page/summary/$titleEncoded"
        Write-Step 50 "Descargando resumen de Wikipedia: $title"
        $wiki = Invoke-RestMethod -Uri $summaryUrl -TimeoutSec 20 -Headers @{ 'User-Agent' = 'NexioraResearchBot/0.1' }
        if ($wiki.extract) {
            $summary = [string]$wiki.extract
            $facts.Add($summary) | Out-Null
            $sources.Add([pscustomobject]@{ type='wikipedia'; title=$title; url=[string]$wiki.content_urls.desktop.page }) | Out-Null
        }
    }
} catch {
    $facts.Add("Wikipedia no estuvo disponible para '$Topic': $($_.Exception.Message)") | Out-Null
}

# Provider 2: DuckDuckGo instant answer
try {
    $encoded = [uri]::EscapeDataString($Topic)
    $ddgUrl = "https://api.duckduckgo.com/?q=$encoded&format=json&no_redirect=1&no_html=1"
    Write-Step 62 'Consultando DuckDuckGo Instant Answer'
    $ddg = Invoke-RestMethod -Uri $ddgUrl -TimeoutSec 20 -Headers @{ 'User-Agent' = 'NexioraResearchBot/0.1' }
    if ($ddg.AbstractText) {
        $facts.Add([string]$ddg.AbstractText) | Out-Null
        if ($ddg.AbstractURL) {
            $sources.Add([pscustomobject]@{ type='duckduckgo'; title=[string]$ddg.Heading; url=[string]$ddg.AbstractURL }) | Out-Null
        }
    }
    if ($ddg.RelatedTopics) {
        foreach ($rt in $ddg.RelatedTopics | Select-Object -First 8) {
            if ($rt.Text) {
                $facts.Add([string]$rt.Text) | Out-Null
            }
        }
    }
} catch {
    $facts.Add("DuckDuckGo no estuvo disponible para '$Topic': $($_.Exception.Message)") | Out-Null
}

# Provider 3: topic-specific hints for common engineering subjects.
if ($Topic -match '(?i)^genexus$') {
    $sources.Add([pscustomobject]@{ type='official'; title='GeneXus Documentation'; url='https://docs.genexus.com/' }) | Out-Null
    $facts.Add('GeneXus es una plataforma low-code orientada al desarrollo y mantenimiento de aplicaciones empresariales a partir de una base de conocimiento.') | Out-Null
    $facts.Add('En GeneXus, la base de conocimiento modela entidades, transacciones, reglas, objetos y generadores para producir aplicaciones en distintas plataformas.') | Out-Null
    $concepts.Add('Knowledge Base') | Out-Null
    $concepts.Add('Transaction') | Out-Null
    $concepts.Add('Generator') | Out-Null
    $concepts.Add('WorkWithPlus') | Out-Null
    $concepts.Add('Rules') | Out-Null
}

Write-Step 75 'Extrayendo conceptos'

$tokens = @()
foreach ($fact in $facts) {
    $matches = [regex]::Matches($fact, '\b[A-Z][A-Za-z0-9\+\.#-]{2,}\b')
    foreach ($m in $matches) {
        if ($tokens.Count -lt 40) { $tokens += $m.Value }
    }
}
foreach ($token in ($tokens | Select-Object -Unique | Select-Object -First 15)) {
    if (-not $concepts.Contains($token)) { $concepts.Add($token) | Out-Null }
}
if (-not $concepts.Contains($title)) { $concepts.Insert(0, $title) }
if ($concepts.Count -eq 0) { $concepts.Add($Topic) | Out-Null }

Write-Step 86 'Actualizando memoria persistente'

Remove-Item -Force -ErrorAction SilentlyContinue $memoryPath
Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"topic","name":"' + (JsonEscape $Topic) + '","title":"' + (JsonEscape $title) + '","confidence":72}')
foreach ($src in $sources) {
    Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"source","topic":"' + (JsonEscape $Topic) + '","source_type":"' + (JsonEscape $src.type) + '","title":"' + (JsonEscape $src.title) + '","url":"' + (JsonEscape $src.url) + '"}')
}
foreach ($c in ($concepts | Select-Object -Unique | Select-Object -First 25)) {
    Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"concept","topic":"' + (JsonEscape $Topic) + '","name":"' + (JsonEscape $c) + '","confidence":70}')
}
foreach ($fact in ($facts | Select-Object -First 12)) {
    Add-Content -Path $memoryPath -Encoding UTF8 -Value ('{"type":"fact","topic":"' + (JsonEscape $Topic) + '","text":"' + (JsonEscape $fact) + '","confidence":68}')
}

$sources | ConvertTo-Json -Depth 4 | Set-Content -Path $sourcesPath -Encoding UTF8

$mainFact = if ($facts.Count -gt 0) { $facts[0] } else { "No se obtuvo una descripcion externa suficiente para $Topic." }
$conceptText = ($concepts | Select-Object -Unique | Select-Object -First 12) -join ', '
$sourceText = ($sources | ForEach-Object { "- $($_.title): $($_.url)" }) -join "`n"
if ([string]::IsNullOrWhiteSpace($sourceText)) { $sourceText = '- Sin fuentes externas confiables registradas.' }

$answer = @"
Tema: $Topic
Titulo detectado: $title
Confianza: 72 %

Resumen:
$mainFact

Conceptos relacionados:
$conceptText

Fuentes:
$sourceText

Nota: Esta respuesta proviene de la memoria local generada por Nexiora.
"@
$answer | Set-Content -Path $answerPath -Encoding UTF8

$report = @"
# Investigacion: $Topic

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

Write-Step 100 'Aprendizaje completado'
Write-Host "Reporte: $reportPath"
Write-Host "Memoria: $memoryPath"
