param([string]$Root='.')
$ErrorActionPreference='Stop'
$rootPath=(Resolve-Path $Root).Path
$configurePreset='windows-msvc-release'
$buildPreset='release'
$pm=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_package.exe'
$packageDir=Split-Path -Parent $MyInvocation.MyCommand.Path
function Run-Step([scriptblock]$Action,[string]$Message){ & $Action; if($LASTEXITCODE -ne 0){throw $Message} }
if(-not(Test-Path $pm)){
  Run-Step { cmake --preset $configurePreset } 'Bootstrap configure failed'
  Run-Step { cmake --build --preset $buildPreset --target nexiora_package } 'Package Manager bootstrap failed'
}
Run-Step { & $pm verify $packageDir } 'verify failed'
Run-Step { & $pm deps $packageDir } 'deps failed'
Run-Step { & $pm install $packageDir } 'install failed'
Run-Step { cmake --preset $configurePreset } 'configure failed'
$focused=Join-Path $rootPath 'Build\tool001-focused.log'
& cmake --build --preset $buildPreset --target nexiora_tool NxToolManagerTests 2>&1 | Tee-Object -FilePath $focused
if($LASTEXITCODE -ne 0){throw 'Focused build failed'}
$warnings=Select-String -Path $focused -Pattern '(?i)warning:'
if($warnings){$warnings|ForEach-Object{$_.Line};throw "Focused build emitted $($warnings.Count) warning(s)"}
Run-Step { ctest --test-dir (Join-Path $rootPath 'Build\windows-msvc-release') -R NxToolManagerTests --output-on-failure } 'Focused tests failed'
$full=Join-Path $rootPath 'Build\tool001-full.log'
& cmake --build --preset $buildPreset 2>&1 | Tee-Object -FilePath $full
if($LASTEXITCODE -ne 0){throw 'Full build failed'}
$warnings=Select-String -Path $full -Pattern '(?i)warning:'
if($warnings){$warnings|ForEach-Object{$_.Line};throw "Full build emitted $($warnings.Count) warning(s)"}
Run-Step { ctest --preset release-tests --output-on-failure } 'Full test suite failed'
$tool=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_tool.exe'
if(-not(Test-Path $tool)){throw "Tool Manager executable missing: $tool"}
Run-Step { & $tool install $rootPath yt-dlp } 'Managed yt-dlp installation failed'
Run-Step { & $tool verify $rootPath yt-dlp } 'Managed yt-dlp verification failed'
Run-Step { & $pm history 'TOOL-001 Nexiora Tool Manager' } 'history failed'
Write-Host 'TOOL-001 verified: Tool Manager installed, yt-dlp managed locally, 0 warnings and all tests passed.'
