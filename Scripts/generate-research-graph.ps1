$ErrorActionPreference = "Stop"

$outputDir = Join-Path (Get-Location) "Artifacts\ResearchGraph"
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$exe = Join-Path (Get-Location) "Build\windows-msvc-release\bin\NxResearchGraphExportSample.exe"
if (!(Test-Path $exe)) {
    $exe = Join-Path (Get-Location) "Build\windows-msvc-release\NxResearchGraphExportSample.exe"
}

if (!(Test-Path $exe)) {
    throw "NxResearchGraphExportSample.exe not found. Build the project first with cmake --build --preset release."
}

& $exe $outputDir

$svg = Join-Path $outputDir "research_graph.svg"
if (Test-Path $svg) {
    Write-Host "Opening SVG: $svg"
    Start-Process $svg
}
