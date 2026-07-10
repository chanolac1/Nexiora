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
$log=Join-Path $rootPath 'Build\capability01-repair2.log'
& cmake --build --preset release --target nexiora_web_cognitive NxWebCognitivePipelineTests nexiora_package --clean-first 2>&1 | Tee-Object -FilePath $log
if($LASTEXITCODE -ne 0){throw 'Clean focused build failed'}
$warnings=Select-String -Path $log -Pattern '(?i)warning:'
if($warnings){$warnings|ForEach-Object{$_.Line};throw "Clean focused build emitted $($warnings.Count) warning(s)"}
Run-Step { ctest --test-dir (Join-Path $rootPath 'Build\windows-msvc-release') -R NxWebCognitivePipelineTests --output-on-failure } 'Focused tests failed'
$web=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_web_cognitive.exe'
if(-not(Test-Path $web)){throw "Web cognitive executable missing: $web"}
$plan=& $web plan $rootPath 'https://www.youtube.com/watch?v=GSK55XYhhac' es
if($LASTEXITCODE -ne 0){throw 'YouTube plan failed'}
if($plan -notmatch '--js-runtimes'){throw 'Rebuilt executable does not contain managed Deno integration'}
Run-Step { & $pm history 'CAPABILITY-01 Sprint-01 Repair2 Transcript Discovery' } 'history failed'
Write-Host 'CAPABILITY-01 Sprint-01 Repair2 verified: clean rebuild, flexible transcript discovery, 0 warnings and tests passed.'
