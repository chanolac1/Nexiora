#include "Nexiora/Research/NxRegistry.h"
#include "Nexiora/NCP/String/NxString.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static NxExperimentStatus nx_registry_status_from_string(const char* status) {
    if (strcmp(status, "Prepared") == 0) return NX_EXPERIMENT_STATUS_PREPARED;
    if (strcmp(status, "Running") == 0) return NX_EXPERIMENT_STATUS_RUNNING;
    if (strcmp(status, "Measured") == 0) return NX_EXPERIMENT_STATUS_MEASURED;
    if (strcmp(status, "Validated") == 0) return NX_EXPERIMENT_STATUS_VALIDATED;
    if (strcmp(status, "Approved") == 0) return NX_EXPERIMENT_STATUS_APPROVED;
    if (strcmp(status, "Integrated") == 0) return NX_EXPERIMENT_STATUS_INTEGRATED;
    if (strcmp(status, "Archived") == 0) return NX_EXPERIMENT_STATUS_ARCHIVED;
    if (strcmp(status, "Rejected") == 0) return NX_EXPERIMENT_STATUS_REJECTED;
    return NX_EXPERIMENT_STATUS_DRAFT;
}

NxResult nx_registry_initialize(NxRegistry* registry) {
    if (registry == NULL) return NX_ERROR_ARGUMENT;
    memset(registry, 0, sizeof(*registry));
    return NX_OK;
}

NxResult nx_registry_add(NxRegistry* registry, const NxExperiment* experiment) {
    if (registry == NULL || experiment == NULL) return NX_ERROR_ARGUMENT;
    if (registry->count >= NX_REGISTRY_MAX_ENTRIES) return NX_ERROR_MEMORY;

    NxRegistryEntry* entry = &registry->entries[registry->count];
    if (nx_string_copy(entry->id, sizeof(entry->id), experiment->id) != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_string_copy(entry->module, sizeof(entry->module), experiment->module) != NX_OK) return NX_ERROR_ARGUMENT;
    entry->status = experiment->status;
    entry->hash = nx_experiment_hash(experiment);
    registry->count += 1;
    return NX_OK;
}

NxResult nx_registry_write(const NxRegistry* registry, const char* path) {
    if (registry == NULL || path == NULL) return NX_ERROR_ARGUMENT;
    FILE* file = fopen(path, "wb");
    if (file == NULL) return NX_ERROR_IO;
    fprintf(file, "NXR-REGISTRY 1\n");
    fprintf(file, "count=%zu\n", registry->count);
    for (size_t i = 0; i < registry->count; ++i) {
        const NxRegistryEntry* entry = &registry->entries[i];
        fprintf(file, "%s|%s|%s|%u\n", entry->id, nx_experiment_status_to_string(entry->status), entry->module, entry->hash);
    }
    fclose(file);
    return NX_OK;
}

NxResult nx_registry_read(NxRegistry* registry, const char* path) {
    if (registry == NULL || path == NULL) return NX_ERROR_ARGUMENT;
    FILE* file = fopen(path, "rb");
    if (file == NULL) return NX_ERROR_IO;

    nx_registry_initialize(registry);
    char line[512];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strchr(line, '|') == NULL) continue;
        char* id = strtok(line, "|");
        char* status = strtok(NULL, "|");
        char* module = strtok(NULL, "|");
        char* hash_text = strtok(NULL, "|\r\n");
        if (id == NULL || status == NULL || module == NULL || hash_text == NULL) continue;
        if (registry->count >= NX_REGISTRY_MAX_ENTRIES) {
            fclose(file);
            return NX_ERROR_MEMORY;
        }
        NxRegistryEntry* entry = &registry->entries[registry->count];
        nx_string_copy(entry->id, sizeof(entry->id), id);
        nx_string_copy(entry->module, sizeof(entry->module), module);
        entry->status = nx_registry_status_from_string(status);
        entry->hash = (unsigned int)strtoul(hash_text, NULL, 10);
        registry->count += 1;
    }

    fclose(file);
    return NX_OK;
}

const NxRegistryEntry* nx_registry_find(const NxRegistry* registry, const char* id) {
    if (registry == NULL || id == NULL) return NULL;
    for (size_t i = 0; i < registry->count; ++i) {
        if (strcmp(registry->entries[i].id, id) == 0) {
            return &registry->entries[i];
        }
    }
    return NULL;
}
