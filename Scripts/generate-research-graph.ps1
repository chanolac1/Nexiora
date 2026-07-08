$ErrorActionPreference = "Stop"

$outputDir = Join-Path (Get-Location) "Artifacts\ResearchGraph"
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$candidates = @(
    "Build\windows-msvc-release\bin\NxResearchGraphExportSample.exe",
    "Build\windows-msvc-release\NxResearchGraphExportSample.exe",
    "Build\windows-msvc-release\Tools\ResearchGraph\NxResearchGraphExportSample.exe"
)

$exe = $null
foreach ($candidate in $candidates) {
    $path = Join-Path (Get-Location) $candidate
    if (Test-Path $path) {
        $exe = $path
        break
    }
}

if ($null -eq $exe) {
    throw "NxResearchGraphExportSample.exe not found. Build first with cmake --build --preset release."
}

& $exe $outputDir

$svg = Join-Path $outputDir "research_graph.svg"
$dot = Join-Path $outputDir "research_graph.dot"
$json = Join-Path $outputDir "research_graph.json"

if (!(Test-Path $svg)) { throw "SVG was not generated: $svg" }
if (!(Test-Path $dot)) { throw "DOT was not generated: $dot" }
if (!(Test-Path $json)) { throw "JSON was not generated: $json" }

Write-Host "Generated:" -ForegroundColor Green
Write-Host "  $svg"
Write-Host "  $dot"
Write-Host "  $json"

Start-Process $svg
