$ErrorActionPreference = "Stop"

Write-Host "== HIST-0003: configuring =="
cmake --preset windows-msvc-release

Write-Host "== HIST-0003: building =="
cmake --build --preset release

Write-Host "== HIST-0003: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== HIST-0003: learning SQLite =="
.\Build\windows-msvc-release\bin\nexiora.exe investiga SQLite

Write-Host "== HIST-0003: asking learned knowledge =="
.\Build\windows-msvc-release\bin\nexiora.exe que sabes SQLite

Write-Host "== HIST-0003 complete =="
