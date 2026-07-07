# Nexiora 0.0.1 Genesis - Windows Build Guide

This document is the authoritative build guide for Genesis.

## 1. Required tools

Install these tools first:

- Visual Studio 2022 Community or higher
- Visual Studio workload: **Desktop development with C++**
- MSVC v143 toolset
- Windows 10/11 SDK
- CMake 3.25 or newer
- Ninja

Open **Developer PowerShell for VS 2022**. Do not use normal PowerShell unless MSVC is already in your environment.

Verify:

```powershell
cl
cmake --version
ninja --version
```

## 2. Required folder layout

Recommended location:

```text
C:\Development\Nexiora
```

Source and resources remain in:

```text
C:\Development\Nexiora\Source
C:\Development\Nexiora\Include
C:\Development\Nexiora\Config
C:\Development\Nexiora\Models
C:\Development\Nexiora\Datasets
```

Generated files go under:

```text
C:\Development\Nexiora\Build
```

Do not put source files inside `Build`.

## 3. Debug build

```powershell
cd C:\Development\Nexiora
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
```

Expected binaries:

```text
Build\windows-msvc-debug\bin\nexiora.exe
Build\windows-msvc-debug\bin\nexiora_tests.exe
Build\windows-msvc-debug\bin\nexiora_bench.exe
```

Run:

```powershell
.\Build\windows-msvc-debug\bin\nexiora.exe
.\Build\windows-msvc-debug\bin\nexiora_tests.exe
.\Build\windows-msvc-debug\bin\nexiora_bench.exe
```

## 4. Release build

```powershell
cd C:\Development\Nexiora
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --preset release-tests --output-on-failure
```

Expected binaries:

```text
Build\windows-msvc-release\bin\nexiora.exe
Build\windows-msvc-release\bin\nexiora_tests.exe
Build\windows-msvc-release\bin\nexiora_bench.exe
```

Run:

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe
.\Build\windows-msvc-release\bin\nexiora_bench.exe
```

## 5. Shortcut scripts

From the repository root:

```powershell
.\Scripts\build-debug.ps1
.\Scripts\build-release.ps1
```

## 6. Important correction

The benchmark executable is named:

```text
nexiora_bench.exe
```

It is **not** named `NxBenchmark.exe` in Genesis.

## 7. Resource folders

No external resource is required for version 0.0.1.

- `Config/`: future configuration files.
- `Resources/`: future static resources.
- `Models/`: future model checkpoints and tokenizers.
- `Datasets/`: future datasets.
