# Nexiora 0.0.13 — Promotion Pipeline Patch

## Copy files

- `Include/Nexiora/NCP/Research/NxPromotion.h`
- `Source/NCP/Research/NxPromotion.c`
- `Tests/Unit/NxPromotionTests.c`
- `SPEC/SPEC-0006-Promotion-Pipeline.md`
- `RFC/RFC-0027-Promotion-Pipeline.md`
- `LABBOOK/LAB-0006-Promotion-Pipeline.md`
- `BOOK/01_CURRENT_STATE.md`
- `BOOK/18_AI_CONTEXT.md`

## CMake additions

Add inside `add_library(NexioraNCP STATIC ...)`:

```cmake
Source/NCP/Research/NxPromotion.c
```

Add inside `if(NEXIORA_BUILD_TESTS)`:

```cmake
add_executable(NxPromotionTests
    Tests/Unit/NxPromotionTests.c
)
target_link_libraries(NxPromotionTests PRIVATE NexioraNCP)
add_test(NAME NxPromotionTests COMMAND NxPromotionTests)
```

## Build

```powershell
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```
