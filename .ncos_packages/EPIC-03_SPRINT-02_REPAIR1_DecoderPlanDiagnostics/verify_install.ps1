param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$pkg = Join-Path $rootPath ".ncos_packages\EPIC-03_SPRINT-02_REPAIR1_DecoderPlanDiagnostics"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"
Push-Location $rootPath
try {
    if (-not (Test-Path $pm)) {
        cmake --preset windows-msvc-release
        if ($LASTEXITCODE -ne 0) { throw "Bootstrap configure failed" }
        cmake --build --preset release --target nexiora_package
        if ($LASTEXITCODE -ne 0) { throw "Package Manager bootstrap failed" }
    }
    & $pm verify $pkg; if ($LASTEXITCODE -ne 0) { throw "verify failed" }
    & $pm deps $pkg; if ($LASTEXITCODE -ne 0) { throw "deps failed" }
    & $pm install $pkg; if ($LASTEXITCODE -ne 0) { throw "install failed" }
    $logDir = Join-Path $rootPath "Build\verification"; New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $log = Join-Path $logDir "epic03_sprint02_repair1.log"
    cmake --preset windows-msvc-release 2>&1 | Tee-Object -FilePath $log
    if ($LASTEXITCODE -ne 0) { throw "configure failed" }
    cmake --build --preset release --target nexiora_decode NxDecoderAdaptersTests 2>&1 | Tee-Object -FilePath $log -Append
    if ($LASTEXITCODE -ne 0) { throw "focused build failed" }
    $warnings = @(Select-String -Path $log -Pattern '(?i)\bwarning\s*:' -ErrorAction SilentlyContinue)
    if ($warnings.Count -ne 0) { $warnings | ForEach-Object { Write-Host $_.Line }; throw "Build emitted $($warnings.Count) warning(s)" }
    ctest --test-dir .\Build\windows-msvc-release -R '^NxDecoderAdaptersTests$' --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "focused tests failed" }
    cmake --build --preset release 2>&1 | Tee-Object -FilePath $log -Append
    if ($LASTEXITCODE -ne 0) { throw "full build failed" }
    $warnings = @(Select-String -Path $log -Pattern '(?i)\bwarning\s*:' -ErrorAction SilentlyContinue)
    if ($warnings.Count -ne 0) { $warnings | ForEach-Object { Write-Host $_.Line }; throw "Build emitted $($warnings.Count) warning(s)" }
    ctest --preset release-tests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "full tests failed" }
    & $pm history "EPIC-03 Sprint-02 Repair1 Decoder Plan Diagnostics"
    Write-Host "EPIC-03 Sprint-02 Repair1 verified: deterministic diagnostics, 0 warnings, all tests passed."
} finally { Pop-Location }
