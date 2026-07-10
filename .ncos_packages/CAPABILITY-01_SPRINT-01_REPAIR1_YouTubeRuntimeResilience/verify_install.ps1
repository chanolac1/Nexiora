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
$installer=Join-Path $rootPath 'Tools\ToolManager\install_tool.ps1'
& powershell -ExecutionPolicy Bypass -File $installer -Root $rootPath -ToolId deno
if($LASTEXITCODE -ne 0){throw 'Managed Deno installation failed'}
$deno=Join-Path $rootPath 'Tools\Managed\deno\deno.exe'
if(-not(Test-Path $deno)){throw "Managed Deno missing: $deno"}
Run-Step { cmake --preset windows-msvc-release } 'configure failed'
$log=Join-Path $rootPath 'Build\capability01-repair1.log'
& cmake --build --preset release --target nexiora_web_cognitive NxWebCognitivePipelineTests 2>&1 | Tee-Object -FilePath $log
if($LASTEXITCODE -ne 0){throw 'Focused build failed'}
$warnings=Select-String -Path $log -Pattern '(?i)warning:'
if($warnings){$warnings|ForEach-Object{$_.Line};throw "Focused build emitted $($warnings.Count) warning(s)"}
Run-Step { ctest --test-dir (Join-Path $rootPath 'Build\windows-msvc-release') -R NxWebCognitivePipelineTests --output-on-failure } 'Focused tests failed'
$web=Join-Path $rootPath 'Build\windows-msvc-release\bin\nexiora_web_cognitive.exe'
$plan=& $web plan $rootPath 'https://www.youtube.com/watch?v=gSj0QDJawX0' es
if($LASTEXITCODE -ne 0){throw 'YouTube plan failed'}
if($plan -notmatch '--js-runtimes'){throw 'Managed Deno was not included in the acquisition plan'}
Run-Step { & $pm history 'CAPABILITY-01 Sprint-01 Repair1 YouTube Runtime Resilience' } 'history failed'
Write-Host 'CAPABILITY-01 Sprint-01 Repair1 verified: managed Deno, resilient subtitle acquisition, flexible VTT discovery, 0 warnings and tests passed.'
