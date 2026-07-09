# CORE-0001 Repair

Corrige la integración del núcleo real de aprendizaje:

- evita copiar archivos sobre sí mismos;
- integra `NxLearningCore` en CMake;
- agrega los comandos `nexiora aprende <tema>` y `nexiora que sabes <tema>`;
- valida aprendizaje real de Genexus y consulta posterior.

Ejecutar:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0001.ps1
```
