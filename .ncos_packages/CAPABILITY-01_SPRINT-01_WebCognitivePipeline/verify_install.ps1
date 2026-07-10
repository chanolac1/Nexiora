param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$packageDir = Join-Path $rootPath ".ncos_packages\CAPABILITY-01_SPRINT-01_WebCognitivePipeline"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"

function Invoke-Checked([scriptblock]$Command, [string]$Message) {
    & $Command
    if ($LASTEXITCODE -ne 0) { throw $Message }
}

Push-Location $rootPath
try {
    if (-not (Test-Path $pm)) {
        Invoke-Checked { cmake --preset windows-msvc-release } "Configuración para bootstrap falló"
        Invoke-Checked { cmake --build --preset release --target nexiora_package } "Bootstrap del Package Manager falló"
    }
    if (-not (Test-Path $pm)) { throw "Package Manager no disponible: $pm" }

    Invoke-Checked { & $pm verify $packageDir } "verify falló"
    Invoke-Checked { & $pm deps $packageDir } "deps falló"
    Invoke-Checked { & $pm install $packageDir } "install falló"
    Invoke-Checked { cmake --preset windows-msvc-release } "Configuración CMake falló"

    $focusedLog = Join-Path $rootPath "Build\capability01-sprint01-focused.log"
    & cmake --build --preset release --target nexiora_web_cognitive NxWebCognitivePipelineTests 2>&1 | Tee-Object -FilePath $focusedLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación enfocada falló" }
    $focusedWarnings = @(Select-String -Path $focusedLog -Pattern "warning:" -SimpleMatch)
    if ($focusedWarnings.Count -ne 0) { throw "Compilación enfocada produjo $($focusedWarnings.Count) warning(s)" }

    Invoke-Checked { ctest --test-dir .\Build\windows-msvc-release -R NxWebCognitivePipelineTests --output-on-failure } "Prueba enfocada falló"

    $fullLog = Join-Path $rootPath "Build\capability01-sprint01-full.log"
    & cmake --build --preset release 2>&1 | Tee-Object -FilePath $fullLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación completa falló" }
    $fullWarnings = @(Select-String -Path $fullLog -Pattern "warning:" -SimpleMatch)
    if ($fullWarnings.Count -ne 0) { throw "Compilación completa produjo $($fullWarnings.Count) warning(s)" }
    Invoke-Checked { ctest --test-dir .\Build\windows-msvc-release --output-on-failure } "Suite completa falló"

    $webTool = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_web_cognitive.exe"
    $cognitiveTool = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_multimodal_cognitive.exe"
    if (-not (Test-Path $webTool)) { throw "Ejecutable no generado: $webTool" }
    if (-not (Test-Path $cognitiveTool)) { throw "Motor cognitivo no disponible: $cognitiveTool" }

    $demoDir = Join-Path $rootPath "Knowledge\WebCognitive\demo_web_source"
    New-Item -ItemType Directory -Force -Path $demoDir | Out-Null
    $demoKnowledge = Join-Path $demoDir "demo_web_source.nxknowledge"
    Invoke-Checked { & $webTool convert ".\Samples\Web\youtube_demo.es.vtt" "https://www.youtube.com/watch?v=demo_web_source" "Nexiora Web Cognitive Demo" $demoKnowledge } "Conversión demostrativa falló"
    $analysisOutput = & $cognitiveTool analyze $rootPath $demoKnowledge "web_learning"
    if ($LASTEXITCODE -ne 0) { throw "Análisis cognitivo demostrativo falló" }
    $analysisOutput | ForEach-Object { Write-Host $_ }
    $analysisLine = $analysisOutput | Where-Object { $_ -like "analysis_dir=*" } | Select-Object -First 1
    if (-not $analysisLine) { throw "El motor cognitivo no reportó analysis_dir" }
    $analysisId = Split-Path ($analysisLine.Substring("analysis_dir=".Length)) -Leaf
    Invoke-Checked { & $cognitiveTool query $rootPath $analysisId "Que permite la evidencia" } "Consulta demostrativa falló"

    Invoke-Checked { & $pm history "CAPABILITY-01 Sprint-01 Web Cognitive Pipeline Foundation" } "history falló"
    Write-Host "CAPABILITY-01 Sprint-01 verified: URL plan, WebVTT evidence, cognitive analysis, grounded query, 0 warnings and all tests passed."
} finally { Pop-Location }
