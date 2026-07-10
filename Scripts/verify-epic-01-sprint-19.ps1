$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
while ($root -and !(Test-Path (Join-Path $root 'CMakePresets.json'))) {
    $parent = Split-Path -Parent $root
    if ($parent -eq $root) { break }
    $root = $parent
}
if (!(Test-Path (Join-Path $root 'CMakePresets.json'))) {
    throw 'No se encontró la raíz de Nexiora.'
}

Push-Location $root
try {
    Write-Host '== EPIC-01 Sprint-19: configure =='
    cmake --preset windows-msvc-release
    if ($LASTEXITCODE -ne 0) { throw 'Falló configure.' }

    Write-Host '== EPIC-01 Sprint-19: build =='
    $buildOutput = cmake --build --preset release 2>&1
    $buildOutput | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw 'Falló build.' }
    if (($buildOutput | Out-String) -match '(?im)\bwarning:') {
        throw 'Se detectaron warnings durante el build.'
    }

    Write-Host '== EPIC-01 Sprint-19: focused test =='
    ctest --test-dir .\Build\windows-msvc-release -R NxWorkspaceEngineTests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Falló NxWorkspaceEngineTests.' }

    Write-Host '== EPIC-01 Sprint-19: full tests =='
    ctest --test-dir .\Build\windows-msvc-release --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Falló la suite completa.' }

    Write-Host '== EPIC-01 Sprint-19: smoke =='
    $exe = '.\Build\windows-msvc-release\bin\nexiora_workspace.exe'
    if (!(Test-Path $exe)) { throw 'No existe nexiora_workspace.exe.' }
    & $exe clean sprint19-demo *> $null
    & $exe create sprint19-demo CMakeLists.txt README.md
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo crear el workspace demo.' }
    $status = & $exe status sprint19-demo | Out-String
    if ($status -notmatch 'READY' -or $status -notmatch 'Copiados\s+: 2') {
        throw 'El workspace demo no reportó el comportamiento esperado.'
    }
    & $exe close sprint19-demo
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo cerrar el workspace demo.' }
    & $exe clean sprint19-demo
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo limpiar el workspace demo.' }

    Write-Host '== EPIC-01 Sprint-19 complete =='
}
finally {
    Pop-Location
}
