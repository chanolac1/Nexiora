# Nexiora 0.0.6 — Containers

This package adds NXM-0004: `NxVector`.

## Files

- `Include/NCP/Containers/NxVector.h`
- `Source/NCP/Containers/NxVector.c`
- `Tests/Unit/NxVectorTests.c`
- `RFC/RFC-0011-Containers-Architecture.md`
- `Docs/Containers.md`

## Integration notes

Add `Source/NCP/Containers/NxVector.c` to the `NexioraNCP` library in `CMakeLists.txt`.

Add a test executable:

```cmake
add_executable(NxVectorTests Tests/Unit/NxVectorTests.c)
target_link_libraries(NxVectorTests PRIVATE NexioraNCP)
add_test(NAME NxVectorTests COMMAND NxVectorTests)
```

Then run:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
```
