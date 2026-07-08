# NRL-0008 — Experiment Discovery Engine

Copy these files into the Nexiora repository root.

## Files

- Include/Nexiora/Research/NxDiscoveryEngine.h
- Source/Research/NxDiscoveryEngine.c
- Tests/Unit/NxDiscoveryEngineTests.c
- Tests/Benchmarks/NxDiscoveryEngineBenchmark.c
- Patches/CMakeLists.NRL-0008.patch
- RFC/RFC-NRL-0008-ExperimentDiscoveryEngine.md
- SPEC/SPEC-NRL-0008-ExperimentDiscoveryEngine.md
- BOOK updates

## CMake integration

Apply the CMake patch manually or add:

```cmake
Source/Research/NxDiscoveryEngine.c
```

to `add_library(NexioraNCP STATIC ...)`.

Then add this inside `if(NEXIORA_BUILD_TESTS)`:

```cmake
add_executable(NxDiscoveryEngineTests
    Tests/Unit/NxDiscoveryEngineTests.c
)
target_link_libraries(NxDiscoveryEngineTests PRIVATE NexioraNCP)
add_test(NAME NxDiscoveryEngineTests COMMAND NxDiscoveryEngineTests)
```

If benchmarks are enabled, add this inside `if(NEXIORA_ENABLE_BENCHMARKS)`:

```cmake
add_executable(NxDiscoveryEngineBenchmark
    Tests/Benchmarks/NxDiscoveryEngineBenchmark.c
)
target_link_libraries(NxDiscoveryEngineBenchmark PRIVATE NexioraNCP)
```

## Verify

```powershell
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
.\Build\windows-msvc-release\bin\NxDiscoveryEngineBenchmark.exe
```

## Commit

```powershell
git add .
git commit -m "NRL-0008: Add Experiment Discovery Engine"
git push
```
