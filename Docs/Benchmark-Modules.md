# Nexiora Benchmark Modules

Starting in Nexiora 0.0.6, benchmarks can be run by module.

This prevents unrelated benchmark noise from blocking promotion of a module that did not change.

Examples:

```powershell
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module containers
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module memory
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module string
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module hardware
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module all
```

Promotion rule:

- For a module change, first run tests.
- Then run the benchmark for the affected module.
- Run `--module all` as a periodic full regression check, not as the only gate for a small module.
