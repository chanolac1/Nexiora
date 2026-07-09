$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$payload = Join-Path $root '_payload'

function Copy-FromPayload($rel) {
    $src = Join-Path $payload $rel
    $dst = Join-Path $root $rel
    $dstDir = Split-Path -Parent $dst
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }
    $srcFull = (Resolve-Path $src).Path
    $dstFull = if (Test-Path $dst) { (Resolve-Path $dst).Path } else { $dst }
    if ($srcFull -ne $dstFull) { Copy-Item -Path $srcFull -Destination $dst -Force }
}

Copy-FromPayload 'Include/Nexiora/Research/NxLearningCore.h'
Copy-FromPayload 'Source/Research/NxLearningCore.c'
Copy-FromPayload 'Include/Nexiora/Research/NxTopicQuestion.h'
Copy-FromPayload 'Source/Research/NxTopicQuestion.c'
Copy-FromPayload 'Tests/Unit/NxTopicQuestionTests.c'
Copy-FromPayload 'Source/NxBootstrap.c'

# Copy learning script in Spanish.
$learnSrc = Join-Path $root 'Scripts/nexiora-learn.ps1'
# already extracted at Scripts/nexiora-learn.ps1 from package; leave it in place.

$cmakePath = Join-Path $root 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw

if ($cmake -notmatch [regex]::Escape('Source/Research/NxLearningCore.c')) {
    if ($cmake -match [regex]::Escape('Source/Research/NxKnowledgeAcquisition.c')) {
        $cmake = $cmake -replace [regex]::Escape('Source/Research/NxKnowledgeAcquisition.c'), "Source/Research/NxKnowledgeAcquisition.c`n    Source/Research/NxLearningCore.c"
    } elseif ($cmake -match [regex]::Escape('Source/Conversation/NxConversation.c')) {
        $cmake = $cmake -replace [regex]::Escape('Source/Conversation/NxConversation.c'), "Source/Conversation/NxConversation.c`n    Source/Research/NxLearningCore.c"
    }
}

if ($cmake -notmatch [regex]::Escape('Source/Research/NxTopicQuestion.c')) {
    if ($cmake -match [regex]::Escape('Source/Research/NxLearningCore.c')) {
        $cmake = $cmake -replace [regex]::Escape('Source/Research/NxLearningCore.c'), "Source/Research/NxLearningCore.c`n    Source/Research/NxTopicQuestion.c"
    } elseif ($cmake -match [regex]::Escape('Source/Research/NxKnowledgeAcquisition.c')) {
        $cmake = $cmake -replace [regex]::Escape('Source/Research/NxKnowledgeAcquisition.c'), "Source/Research/NxKnowledgeAcquisition.c`n    Source/Research/NxTopicQuestion.c"
    }
}

if ($cmake -notmatch [regex]::Escape('NxTopicQuestionTests')) {
    $cmake += @'

# ============================================================
# CORE-0003 — Topic Question Answering
# ============================================================
if(NEXIORA_BUILD_TESTS)
    add_executable(NxTopicQuestionTests
        Tests/Unit/NxTopicQuestionTests.c
    )
    target_link_libraries(NxTopicQuestionTests PRIVATE NexioraNCP)
    add_test(NAME NxTopicQuestionTests COMMAND NxTopicQuestionTests)
endif()
'@
}

Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8
Write-Host 'CORE-0003 applied.'
