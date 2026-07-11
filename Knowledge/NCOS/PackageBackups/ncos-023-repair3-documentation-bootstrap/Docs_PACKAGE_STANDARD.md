# PACKAGE_STANDARD

## Flujo oficial

Toda entrega de Nexiora se instala y certifica mediante:

```powershell
.\Build\windows-msvc-release\bin\nexiora_package.exe apply .\.ncos_packages\<PAQUETE>
```

`apply` debe ejecutar verificación, dependencias, instalación transaccional, configuración, build completo, cero warnings, pruebas, validación documental e historial. Cualquier fallo posterior a la instalación debe revertir la transacción exacta.

No se permiten scripts externos como instalador oficial.
