# NCOS-023 — Automatic Package Discovery and Pure Apply

## Qué logramos

Nexiora Package Manager puede descubrir de forma determinista el paquete pendiente más reciente dentro de `.ncos_packages` y certificarlo con un único comando sin ruta de paquete.

El flujo nativo ejecuta exactamente una vez:

1. descubrimiento;
2. verificación;
3. dependencias;
4. instalación transaccional;
5. configuración CMake;
6. build completo;
7. compuerta de cero warnings;
8. suite CTest completa;
9. validación documental;
10. historial o rollback exacto.

## Selección determinista

Un candidato debe:

- contener un manifiesto válido;
- no estar instalado;
- tener todas sus dependencias satisfechas.

Entre los candidatos elegibles se selecciona el manifiesto más reciente. En empate se usa el nombre de ruta como criterio estable.

## Uso

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe
```

o:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe apply
```

La forma explícita continúa disponible para diagnóstico:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe apply .\.ncos_packages\PAQUETE
```

## Calidad

No se deshabilitaron warnings. La compilación aislada de motor, CLI y pruebas se validó con C23, `-Wall`, `-Wextra`, `-Wpedantic` y `-Werror`.
