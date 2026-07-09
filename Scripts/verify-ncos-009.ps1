$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir) { throw 'No se encontro la raiz del repositorio con CMakePresets.json.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
Set-Location $repo

Write-Host '== NCOS-009: applying =='
powershell -ExecutionPolicy Bypass -File (Join-Path $repo '.nexiora_sprints\NCOS-009_CompilerEngine\Scripts\apply-ncos-009.ps1')

Write-Host '== NCOS-009: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-009: build =='
cmake --build --preset release

Write-Host '== NCOS-009: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-009: smoke compiler engine =='
$exe = '.\Build\windows-msvc-release\bin\nexiora_compile.exe'
if (!(Test-Path $exe)) { throw 'No se encontro nexiora_compile.exe' }

$logDir = '.\Knowledge\NCOS\Compiler'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$logFile = Join-Path $logDir 'smoke_echo.log'
if (Test-Path $logFile) { Remove-Item $logFile -Force }

& $exe run smoke_echo "cmd /c echo NCOS_COMPILER_OK" | Tee-Object -FilePath 'ncos-009-smoke.log'

if (!(Test-Path $logFile)) { throw "No se genero el log esperado: $logFile" }
$log = Get-Content $logFile -Raw
if ($log -notmatch 'NCOS_COMPILER_OK') { throw 'El log no capturo la salida del comando.' }

Write-Host '== NCOS-009 complete =='
