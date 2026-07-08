# Nexiora 0.0.12 — Research Evidence Generator Patch

## Copy these files into your repository

- `Include/Nexiora/NCP/Research/NxResearchEvidence.h`
- `Source/NCP/Research/NxResearchEvidence.c`
- `Tests/Unit/NxResearchEvidenceTests.c`
- `SPEC/SPEC-0005-Evidence-Generator.md`
- `RFC/RFC-0026-Research-Evidence-Generator.md`
- `LABBOOK/LAB-0005-Evidence-Generator.md`
- `BOOK/01_CURRENT_STATE.md`
- `BOOK/18_AI_CONTEXT.md`

## CMake additions

Add inside `add_library(NexioraNCP STATIC ...)`:

```cmake
Source/NCP/Research/NxResearchEvidence.c
```

Add inside `if(NEXIORA_BUILD_TESTS)`:

```cmake
add_executable(NxResearchEvidenceTests
    Tests/Unit/NxResearchEvidenceTests.c
)
target_link_libraries(NxResearchEvidenceTests PRIVATE NexioraNCP)
add_test(NAME NxResearchEvidenceTests COMMAND NxResearchEvidenceTests)
```

## Build

```powershell
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```
