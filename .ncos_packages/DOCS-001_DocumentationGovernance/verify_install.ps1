param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$exe = Join-Path $Root "Build/windows-msvc-release/bin/nexiora_package.exe"
$pkg = Join-Path $Root ".ncos_packages/DOCS-001_DocumentationGovernance"
& $exe verify $pkg
& $exe deps $Root $pkg
& $exe install $Root $pkg
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --test-dir (Join-Path $Root "Build/windows-msvc-release") -R NxDocumentationGovernanceTests --output-on-failure
ctest --test-dir (Join-Path $Root "Build/windows-msvc-release") --output-on-failure
& (Join-Path $Root "Build/windows-msvc-release/bin/nexiora_docs.exe") validate $Root
& $exe history $Root "DOCS-001 Documentation Governance"
