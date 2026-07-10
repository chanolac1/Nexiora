param([string]$Root='.')
$ErrorActionPreference='Stop'
$rootPath=(Resolve-Path $Root).Path
$packageDir=Split-Path -Parent $MyInvocation.MyCommand.Path
$pm=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_package.exe'
function Run-Step([scriptblock]$Action,[string]$Message){ & $Action; if($LASTEXITCODE -ne 0){throw $Message} }
if(-not(Test-Path $pm)){ Run-Step { cmake --preset windows-msvc-release } 'Bootstrap configure failed'; Run-Step { cmake --build --preset release --target nexiora_package } 'Package Manager bootstrap failed' }
Run-Step { & $pm verify $packageDir } 'verify failed'
Run-Step { & $pm deps $packageDir } 'deps failed'
Run-Step { & $pm install $packageDir } 'install failed'
Run-Step { cmake --preset windows-msvc-release } 'configure failed'
$log=Join-Path $rootPath 'Build\tool001-repair1.log'
& cmake --build --preset release --target nexiora_web_cognitive NxWebCognitivePipelineTests 2>&1 | Tee-Object -FilePath $log
if($LASTEXITCODE -ne 0){throw 'Focused build failed'}
$warnings=Select-String -Path $log -Pattern '(?i)warning:'
if($warnings){$warnings|ForEach-Object{$_.Line};throw "Focused build emitted $($warnings.Count) warning(s)"}
Run-Step { ctest --test-dir (Join-Path $rootPath 'Build\windows-msvc-release') -R NxWebCognitivePipelineTests --output-on-failure } 'Focused tests failed'
$web=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_web_cognitive.exe'
$yt=Join-Path $rootPath 'Tools\Managed\yt-dlp\yt-dlp.exe'
if(-not(Test-Path $web)){throw "Web cognitive executable missing: $web"}
if(-not(Test-Path $yt)){throw "Managed yt-dlp missing: $yt"}
$plan=& $web plan $rootPath 'https://www.youtube.com/watch?v=gSj0QDJawX0' es
if($LASTEXITCODE -ne 0){throw 'Absolute-path YouTube plan failed'}
if($plan -match '(?m)^command="\."'){throw 'Regression detected: root is still being executed as a command'}
Run-Step { & $pm history 'TOOL-001 Repair1 Direct Process Execution' } 'history failed'
Write-Host 'TOOL-001 Repair1 verified: managed tools execute directly, absolute paths resolved, 0 warnings and tests passed.'
