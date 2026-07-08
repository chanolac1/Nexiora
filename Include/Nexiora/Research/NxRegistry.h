#ifndef NEXIORA_RESEARCH_NXREGISTRY_H
#define NEXIORA_RESEARCH_NXREGISTRY_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/Research/NxExperiment.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_REGISTRY_MAX_ENTRIES 256

typedef struct NxRegistryEntry {
    char id[NX_EXPERIMENT_ID_MAX];
    char module[NX_EXPERIMENT_MODULE_MAX];
    NxExperimentStatus status;
    unsigned int hash;
} NxRegistryEntry;

typedef struct NxRegistry {
    NxRegistryEntry entries[NX_REGISTRY_MAX_ENTRIES];
    size_t count;
} NxRegistry;

NxResult nx_registry_initialize(NxRegistry* registry);
NxResult nx_registry_add(NxRegistry* registry, const NxExperiment* experiment);
NxResult nx_registry_write(const NxRegistry* registry, const char* path);
NxResult nx_registry_read(NxRegistry* registry, const char* path);
const NxRegistryEntry* nx_registry_find(const NxRegistry* registry, const char* id);

#ifdef __cplusplus
}
#endif

#endif
