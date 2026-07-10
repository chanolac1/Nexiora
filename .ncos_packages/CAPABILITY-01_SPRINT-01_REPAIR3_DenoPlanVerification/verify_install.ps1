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
$log=Join-Path $rootPath 'Build\capability01-repair3.log'
& cmake --build --preset release --target nexiora_web_cognitive NxWebCognitivePipelineTests nexiora_package 2>&1 | Tee-Object -FilePath $log
if($LASTEXITCODE -ne 0){throw 'Focused build failed'}
$warnings=Select-String -Path $log -Pattern '(?i)warning:'
if($warnings){$warnings|ForEach-Object{$_.Line};throw "Focused build emitted $($warnings.Count) warning(s)"}
Run-Step { ctest --test-dir (Join-Path $rootPath 'Build\windows-msvc-release') -R NxWebCognitivePipelineTests --output-on-failure } 'Focused tests failed'
$web=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_web_cognitive.exe'
if(-not(Test-Path $web)){throw "Web cognitive executable missing: $web"}
$deno=Join-Path $rootPath 'Tools\Managed\deno\deno.exe'
if(-not(Test-Path $deno)){throw "Managed Deno executable missing: $deno"}
$planLines=@(& $web plan $rootPath 'https://www.youtube.com/watch?v=GSK55XYhhac' es)
if($LASTEXITCODE -ne 0){throw 'YouTube plan failed'}
$planText=$planLines -join "`n"
$planLines | ForEach-Object { Write-Host $_ }
if($planText -notmatch '(?m)^managed_js_runtime=CONFIGURED$'){throw 'Behavioral plan verification did not configure managed Deno'}
if($planText -notmatch [regex]::Escape('--js-runtimes')){throw 'Behavioral plan verification did not emit --js-runtimes'}
if($planText -notmatch [regex]::Escape($deno)){throw "Behavioral plan verification did not use managed Deno path: $deno"}
Run-Step { & $pm history 'CAPABILITY-01 Sprint-01 Repair3 Deno Plan Verification' } 'history failed'
Write-Host 'CAPABILITY-01 Sprint-01 Repair3 verified: managed Deno behavior confirmed, flexible transcript discovery retained, 0 warnings and tests passed.'
