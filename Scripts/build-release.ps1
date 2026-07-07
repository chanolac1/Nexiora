Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --preset release-tests --output-on-failure
