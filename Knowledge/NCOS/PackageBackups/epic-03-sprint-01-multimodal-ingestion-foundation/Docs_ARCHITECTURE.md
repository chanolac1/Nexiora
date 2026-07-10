# ARCHITECTURE

## Documentation Manager

El motor recibe un descriptor de entrega generado por Nexiora, valida el conjunto documental, prepara las actualizaciones y confirma los tres documentos mutables mediante una transacción atómica con rollback local.
