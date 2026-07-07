Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
