# Nexiora 0.0.1 "Genesis"

Nexiora is a modular, evidence-driven AI platform foundation.

This first release does **not** include an LLM yet. It establishes the Nexiora Core Platform (NCP):

- Runtime lifecycle
- Result/error system
- String manager with allocation statistics
- Logging system
- Hardware detector
- Benchmark engine
- Core tests
- Windows-first CMake/Ninja build

## Promise

> Nexiora never claims to be better. Nexiora demonstrates improvement through reproducible evidence.

## Windows build

Requirements:

- Windows 10/11 x64
- Visual Studio 2022 with Desktop development with C++
- CMake 3.25+
- Ninja

Open **Developer PowerShell for VS 2022**:

```powershell
cd C:\Development\Nexiora
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
.\Build\windows-msvc-debug\bin\nexiora.exe
.\Build\windows-msvc-debug\bin\nexiora_bench.exe
```

Release:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --preset release-tests --output-on-failure
.\Build\windows-msvc-release\bin\nexiora.exe
.\Build\windows-msvc-release\bin\nexiora_bench.exe
```

Important: Genesis creates `nexiora_bench.exe`, not `NxBenchmark.exe`.

## Repository layout

```text
Nexiora/
├── RFC/                  Architecture decision documents
├── Docs/                 Build and engineering documentation
├── Include/Nexiora/NCP/  Public C API headers
├── Source/NCP/           NCP implementation
├── Tests/                Unit tests and benchmark entry points
├── Scripts/              Build helper scripts
├── Config/               Future configuration files
├── Models/               Future model resources
├── Datasets/             Future datasets
├── Tools/                Future developer tools
└── Build/                Generated build output
```

## Current executables

- `nexiora.exe`: runtime bootstrap and status.
- `nexiora_tests.exe`: core tests.
- `nexiora_bench.exe`: benchmark executable.


## Nexiora 0.0.5 String

Esta versión incorpora el primer String Engine:

- historial de benchmarks;
- comparación contra la medición anterior;
- detección básica de mejora/regresión;
- reporte de evidencia;
- aprobación humana explícita para candidatos mejorados.

Flujo recomendado:

```powershell
.\Scripts
xbuild.ps1 all
```

Benchmark únicamente:

```powershell
.\Scripts
xbuild.ps1 benchmark -Iterations 100000
```

Aprobar candidato:

```powershell
.\Scripts
xbuild.ps1 approve
```


## 0.0.5 String

This release adds NXM-0002, the Nexiora Hardware Abstraction Layer. It detects CPU vendor, CPU brand, memory, cache line information, cache metadata, logical processors, and SIMD/CPU capabilities.

Run:

```powershell
.\Build\windows-msvc-debug\bin\nexiora.exe
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 100000
```

## Nexiora 0.0.5 — String

Esta entrega agrega NXM-0003 String Primitives:

- `nx_string_length`
- `nx_string_compare`
- `nx_string_copy`
- `nx_string_find_char`
- `NxStringView`
- pruebas unitarias
- benchmarks con Evidence Engine

