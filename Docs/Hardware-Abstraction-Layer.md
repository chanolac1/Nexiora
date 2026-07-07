# Nexiora Hardware Abstraction Layer

NXM-0002 adds a first formal HAL implementation.

## Public Header

```text
Include/Nexiora/NCP/Hardware/NxHardware.h
```

## Implementation

```text
Source/NCP/Hardware/NxHardware.c
```

## Main API

```c
NxResult nx_hardware_query(NxHardwareInfo* info);
void nx_hardware_print(const NxHardwareInfo* info);
const char* nx_hardware_vendor_to_string(NxHardwareVendor vendor);
int nx_hardware_supports_any_simd(const NxHardwareInfo* info);
```

## Windows Build

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
.\Build\windows-msvc-debug\bin\nexiora.exe
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 100000
```

## Expected Output

`nexiora.exe` now prints CPU vendor, CPU brand, memory, cache data, and instruction-set capabilities.

## Future Work

- Better physical core detection on Windows.
- NUMA topology.
- GPU detection.
- Runtime dispatch table for optimized implementations.
