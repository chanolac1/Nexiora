# ARCHITECTURE

## Package lifecycle

Package discovery -> verification -> dependency resolution -> transactional install -> configure -> staged build -> warning gate -> complete tests -> documentation validation -> history.

## Self-update boundary

`nexiora_package.exe` remains active during installation. A separate candidate `nexiora_package.next.exe` is produced. Replacement occurs only after certification and process termination.
