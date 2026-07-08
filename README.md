# NRL-0010 CMake Repair

Run from the Nexiora repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\repair-nrl-0010-cmake.ps1
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```

This repair creates a backup named `CMakeLists.txt.nrl0010.bak`.
