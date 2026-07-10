param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$exe = Join-Path $Root "Build/windows-msvc-release/bin/nexiora_package.exe"
$pkg = Join-Path $Root ".ncos_packages/DOCS-002_DocumentationManager"
& $exe verify $pkg
& $exe deps $pkg
& $exe install $pkg
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --test-dir (Join-Path $Root "Build/windows-msvc-release") -R NxDocumentationManagerTests --output-on-failure
ctest --test-dir (Join-Path $Root "Build/windows-msvc-release") --output-on-failure
& (Join-Path $Root "Build/windows-msvc-release/bin/nexiora_docs.exe") validate $Root
& $exe history "DOCS-002 Documentation Manager"
