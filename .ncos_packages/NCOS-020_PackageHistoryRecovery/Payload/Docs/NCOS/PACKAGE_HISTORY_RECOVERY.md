# NCOS-020 Package History & Recovery

Adds a persistent transaction history index and targeted rollback by transaction ID.

Commands:

```powershell
nexiora_package history <package_id>
nexiora_package rollback-tx <package_id> <transaction_id>
```

A no-op reinstall does not erase history. Existing `rollback` still targets the latest committed transaction.
