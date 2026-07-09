$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

Write-Host '== CORE-0001 REPAIR: applying real learning core =='

$payload = Join-Path $root '_core0001_payload'
if (!(Test-Path $payload)) {
    throw 'No se encontro _core0001_payload. Descomprime el ZIP completo sobre D:\Nexiora.'
}

$files = @(
    @{src='Include\Nexiora\Research\NxLearningCore.h'; dst='Include\Nexiora\Research\NxLearningCore.h'},
    @{src='Source\Research\NxLearningCore.c'; dst='Source\Research\NxLearningCore.c'},
    @{src='Tests\Unit\NxLearningCoreTests.c'; dst='Tests\Unit\NxLearningCoreTests.c'},
    @{src='Scripts\nexiora-learn.ps1'; dst='Scripts\nexiora-learn.ps1'}
)
foreach ($f in $files) {
    $src = Join-Path $payload $f.src
    $dst = Join-Path $root $f.dst
    if (!(Test-Path $src)) { throw "Payload faltante: $src" }
    $dstDir = Split-Path -Parent $dst
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }
    if ((Resolve-Path $src).Path -ne (Resolve-Path $dst -ErrorAction SilentlyContinue).Path) {
        Copy-Item -Path $src -Destination $dst -Force
    }
}

$cmakePath = Join-Path $root 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw

if ($cmake -notmatch 'Source/Research/NxLearningCore\.c') {
    $anchors = @('Source/Research/NxKnowledgeStore.c','Source/Research/NxTopicInvestigation.c','Source/Research/NxKnowledgeAcquisition.c','Source/Research/NxProgressEngine.c')
    $done = $false
    foreach ($anchor in $anchors) {
        if (!$done -and $cmake.Contains($anchor)) {
            $cmake = $cmake.Replace($anchor, $anchor + "`r`n    Source/Research/NxLearningCore.c")
            $done = $true
        }
    }
    if (!$done) { throw 'No se encontro punto de insercion para Source/Research/NxLearningCore.c en CMakeLists.txt' }
}

if ($cmake -notmatch 'add_executable\(NxLearningCoreTests') {
    $block = @'

# ============================================================
# CORE-0001 — Real Learning Core
# ============================================================

if(NEXIORA_BUILD_TESTS)
    add_executable(NxLearningCoreTests
        Tests/Unit/NxLearningCoreTests.c
    )
    target_link_libraries(NxLearningCoreTests PRIVATE NexioraNCP)
    add_test(NAME NxLearningCoreTests COMMAND NxLearningCoreTests)
endif()
'@
    $cmake = $cmake.TrimEnd() + $block + "`r`n"
}
Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8

$bootstrapPath = Join-Path $root 'Source\NxBootstrap.c'
$bootstrap = Get-Content $bootstrapPath -Raw

if ($bootstrap -notmatch 'NxLearningCore\.h') {
    $bootstrap = $bootstrap -replace '(#include\s+"Nexiora/[^\r\n]+"\s*\r?\n)(?!#include)', ('$1#include "Nexiora/Research/NxLearningCore.h"' + "`r`n")
    if ($bootstrap -notmatch 'NxLearningCore\.h') { $bootstrap = '#include "Nexiora/Research/NxLearningCore.h"' + "`r`n" + $bootstrap }
}

if ($bootstrap -notmatch 'CORE-0001 real learning command') {
    $commandBlock = @'

    /* CORE-0001 real learning command */
    if (argc >= 3 && (strcmp(argv[1], "aprende") == 0 || strcmp(argv[1], "investiga") == 0))
    {
        char topic[256];
        NxLearningCoreResult learning_result;
        NxLearningCoreStatus learning_status;
        int i;

        topic[0] = '\0';
        for (i = 2; i < argc; ++i)
        {
            if (i > 2)
            {
                (void)strncat(topic, " ", sizeof(topic) - strlen(topic) - 1);
            }
            (void)strncat(topic, argv[i], sizeof(topic) - strlen(topic) - 1);
        }

        learning_status = NxLearningCore_Learn(topic, &learning_result);
        if (learning_status != NX_LEARNING_CORE_OK && learning_status != NX_LEARNING_CORE_SCRIPT_FAILED)
        {
            fprintf(stderr, "Learning failed: %s\n", NxLearningCore_StatusToString(learning_status));
            nx_runtime_shutdown(&runtime);
            return 8;
        }

        nx_runtime_shutdown(&runtime);
        return 0;
    }

    /* CORE-0001 memory query command */
    if (argc >= 4 && strcmp(argv[1], "que") == 0 && strcmp(argv[2], "sabes") == 0)
    {
        char topic[256];
        char answer[8192];
        NxLearningCoreStatus query_status;
        int i;

        topic[0] = '\0';
        for (i = 3; i < argc; ++i)
        {
            if (i > 3)
            {
                (void)strncat(topic, " ", sizeof(topic) - strlen(topic) - 1);
            }
            (void)strncat(topic, argv[i], sizeof(topic) - strlen(topic) - 1);
        }

        query_status = NxLearningCore_Query(topic, answer, sizeof(answer));
        printf("%s\n", answer);
        nx_runtime_shutdown(&runtime);
        return query_status == NX_LEARNING_CORE_OK ? 0 : 10;
    }
'@
    $inserted = $false
    $patterns = @(
        '    if \(argc >= 3 && strcmp\(argv\[1\], "research"\) == 0 && strcmp\(argv\[2\], "run"\) == 0\)',
        '    nx_log_write\(NX_LOG_INFO, "Bootstrap", "Welcome to Nexiora Genesis\."\);'
    )
    foreach ($pat in $patterns) {
        if (!$inserted -and $bootstrap -match $pat) {
            $bootstrap = [regex]::Replace($bootstrap, $pat, ($commandBlock + "`r`n" + '$0'), 1)
            $inserted = $true
        }
    }
    if (!$inserted) { throw 'No se encontro punto de insercion para comandos aprende/que sabes en Source/NxBootstrap.c' }
}

if ($bootstrap -notmatch 'nexiora aprende <tema>') {
    $help = @'
    printf("  nexiora aprende <tema>        Aprende un tema usando conectores reales\n");
    printf("  nexiora que sabes <tema>      Consulta lo aprendido sobre un tema\n");
'@
    $anchorHelp = '    printf("  nexiora memory summary        Show persistent memory summary\n");'
    if ($bootstrap.Contains($anchorHelp)) {
        $bootstrap = $bootstrap.Replace($anchorHelp, $anchorHelp + "`r`n" + $help.TrimEnd())
    }
}

Set-Content -Path $bootstrapPath -Value $bootstrap -Encoding UTF8
Write-Host 'CORE-0001 repair applied.'
