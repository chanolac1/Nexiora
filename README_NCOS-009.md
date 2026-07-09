# NCOS-009 — Compiler Engine

## Qué logramos

Nexiora ahora puede ejecutar comandos externos de compilación/verificación, capturar su salida, guardar logs persistentes y detectar si el proceso terminó correctamente.

Esto habilita el siguiente salto: leer errores de compilación y autocorregir proyectos.

## Comandos

```powershell
.\Build\windows-msvc-release\bin\nexiora_compile.exe run echo_test "cmd /c echo NCOS_COMPILER_OK"
.\Build\windows-msvc-release\bin\nexiora_compile.exe run configure "cmake --preset windows-msvc-release"
.\Build\windows-msvc-release\bin\nexiora_compile.exe run tests "ctest --test-dir .\Build\windows-msvc-release --output-on-failure"
```

Los logs quedan en:

```text
Knowledge/NCOS/Compiler/<run-id>.log
```
