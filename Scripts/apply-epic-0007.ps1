$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$payload = Join-Path $root "Payload"

function Copy-PayloadDirectory($name) {
    $src = Join-Path $payload $name
    $dst = Join-Path $root $name
    if (Test-Path $src) {
        if (-not (Test-Path $dst)) {
            New-Item -ItemType Directory -Force -Path $dst | Out-Null
        }
        Copy-Item -Path (Join-Path $src "*") -Destination $dst -Recurse -Force
    }
}

Write-Host "== EPIC-0007 REPAIR2: copying payload =="
Copy-PayloadDirectory "Include"
Copy-PayloadDirectory "Source"
Copy-PayloadDirectory "Tests"
Copy-PayloadDirectory "Docs"
Copy-PayloadDirectory ".nexiora_sprints"

$readme = Join-Path $payload "README_EPIC-0007.md"
if (Test-Path $readme) {
    Copy-Item -Path $readme -Destination (Join-Path $root "README_EPIC-0007.md") -Force
}

Write-Host "== EPIC-0007 REPAIR2: replacing Source/NxBootstrap.c =="
$bootstrapPayload = Join-Path $payload "Source\NxBootstrap.c"
$bootstrapPath = Join-Path $root "Source\NxBootstrap.c"
if (-not (Test-Path $bootstrapPayload)) { throw "Payload no contiene Source\NxBootstrap.c" }
if (Test-Path $bootstrapPath) {
    Copy-Item -Path $bootstrapPath -Destination ($bootstrapPath + ".epic0007.bak") -Force
}
Copy-Item -Path $bootstrapPayload -Destination $bootstrapPath -Force

Write-Host "== EPIC-0007 REPAIR2: patching CMakeLists.txt =="
$cmakePath = Join-Path $root "CMakeLists.txt"
if (-not (Test-Path $cmakePath)) { throw "No existe CMakeLists.txt" }
$cmake = Get-Content $cmakePath -Raw

if (-not $cmake.Contains('Source/Conversation/NxConversation.c')) {
    $anchors = @(
        '    Source/Research/NxPersistentMemory.c',
        '    Source/Research/NxConflictDetector.c',
        '    Source/Research/NxResearchDashboard.c'
    )
    $inserted = $false
    foreach ($anchor in $anchors) {
        if ($cmake.Contains($anchor)) {
            $cmake = $cmake.Replace($anchor, $anchor + "`r`n" + '    Source/Conversation/NxConversation.c')
            $inserted = $true
            break
        }
    }
    if (-not $inserted) { throw "No se encontro punto para agregar Source/Conversation/NxConversation.c" }
}

if (-not $cmake.Contains('add_executable(NxConversationTests')) {
    $testBlock = @'

if(NEXIORA_BUILD_TESTS)
    add_executable(NxConversationTests
        Tests/Unit/NxConversationTests.c
    )
    target_link_libraries(NxConversationTests PRIVATE NexioraNCP)
    add_test(NAME NxConversationTests COMMAND NxConversationTests)
endif()
'@
    $cmake = $cmake.TrimEnd() + "`r`n" + $testBlock + "`r`n"
}

Copy-Item -Path $cmakePath -Destination ($cmakePath + ".epic0007.bak") -Force
Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8

if (Test-Path $payload) { Remove-Item -Path $payload -Recurse -Force }
Write-Host "EPIC-0007 repair2 aplicado correctamente."
