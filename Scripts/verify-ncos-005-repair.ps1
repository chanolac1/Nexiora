$ErrorActionPreference = "Stop"

Write-Host "== NCOS-005 repair: clean test state =="

# Clean persisted NCOS concept state that can make regression tests start from version > 1.
$pathsToClean = @(
    ".\Knowledge\NCOS\Concepts\genexus",
    ".\Knowledge\NCOS\ConceptGraphs\genexus",
    ".\Build\windows-msvc-release\Testing\Temporary\ncos_concept_registry_test",
    ".\Build\windows-msvc-release\ncos_concept_registry_test",
    ".\Build\windows-msvc-release\Knowledge\NCOS\Concepts\genexus",
    ".\Build\windows-msvc-release\Knowledge\NCOS\ConceptGraphs\genexus"
)

foreach ($p in $pathsToClean) {
    if (Test-Path $p) {
        Remove-Item -Path $p -Recurse -Force
    }
}

Write-Host "== NCOS-005 repair: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-005 repair: build =="
cmake --build --preset release

Write-Host "== NCOS-005 repair: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-005 repair: smoke graph reasoning =="
$graph = ".\Build\windows-msvc-release\bin\nexiora_graph.exe"
$reason = ".\Build\windows-msvc-release\bin\nexiora_reason.exe"

if (!(Test-Path $graph)) { throw "No se encontro nexiora_graph.exe" }
if (!(Test-Path $reason)) { throw "No se encontro nexiora_reason.exe" }

# Recreate deterministic graph evidence after cleaning.
& $graph link Genexus DataSelector related_to DataProvider | Tee-Object -FilePath .\ncos-005-link1.log
& $graph link Genexus DataProvider belongs_to KnowledgeBase | Tee-Object -FilePath .\ncos-005-link2.log
& $reason why Genexus DataSelector KnowledgeBase | Tee-Object -FilePath .\ncos-005-reason.log

$reasonText = Get-Content .\ncos-005-reason.log -Raw

# Behavior-based validation: do not depend on one exact sentence.
if ($reasonText -notmatch "relacion indirecta") { throw "No se explico una relacion indirecta." }
if ($reasonText -notmatch "dataselector") { throw "No se uso el concepto origen dataselector." }
if ($reasonText -notmatch "knowledgebase") { throw "No se uso el concepto destino knowledgebase." }
if ($reasonText -notmatch "dataprovider") { throw "No se uso el concepto intermedio dataprovider." }
if ($reasonText -notmatch "Evidencia usada") { throw "No se mostro evidencia usada." }
if ($reasonText -notmatch "Confianza") { throw "No se mostro confianza." }

Write-Host "== NCOS-005 repair complete =="
