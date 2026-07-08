#include "Nexiora/Research/NxResearchKernel.h"
#include "Nexiora/NCP/String/NxString.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static NxResult nx_research_copy_path(char* destination, size_t destination_size, const char* value, const char* fallback) {
    const char* selected = value != NULL ? value : fallback;
    return nx_string_copy(destination, destination_size, selected);
}

NxResult nx_research_kernel_initialize(NxResearchKernel* kernel, const NxResearchKernelConfig* config) {
    if (kernel == NULL) {
        return NX_ERROR_ARGUMENT;
    }

    memset(kernel, 0, sizeof(*kernel));

    const char* root = config != NULL ? config->root_path : NULL;
    const char* registry = config != NULL ? config->registry_path : NULL;
    const char* journal = config != NULL ? config->journal_path : NULL;

    if (nx_research_copy_path(kernel->root_path, sizeof(kernel->root_path), root, "Research") != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_research_copy_path(kernel->registry_path, sizeof(kernel->registry_path), registry, "Research/Registry/registry.nxr") != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_research_copy_path(kernel->journal_path, sizeof(kernel->journal_path), journal, "Research/Journals/journal.log") != NX_OK) return NX_ERROR_ARGUMENT;

    kernel->initialized = 1;
    kernel->stats.initialized_count = 1;
    return NX_OK;
}

void nx_research_kernel_shutdown(NxResearchKernel* kernel) {
    if (kernel == NULL) {
        return;
    }
    kernel->initialized = 0;
}

NxResult nx_research_kernel_create_experiment(NxResearchKernel* kernel, const NxExperiment* experiment) {
    if (kernel == NULL || experiment == NULL || !kernel->initialized) {
        return NX_ERROR_ARGUMENT;
    }
    if (kernel->experiment_count >= NX_RESEARCH_MAX_EXPERIMENTS) {
        return NX_ERROR_MEMORY;
    }

    kernel->experiments[kernel->experiment_count] = *experiment;
    kernel->experiment_count += 1;
    kernel->stats.created_experiments += 1;
    kernel->stats.registered_experiment_count = kernel->experiment_count;
    return nx_research_kernel_append_journal(kernel, "ExperimentCreated", experiment->id);
}

NxResult nx_research_kernel_write_registry(const NxResearchKernel* kernel) {
    if (kernel == NULL || !kernel->initialized) {
        return NX_ERROR_ARGUMENT;
    }

    FILE* file = fopen(kernel->registry_path, "wb");
    if (file == NULL) {
        return NX_ERROR_IO;
    }

    fprintf(file, "NXR-REGISTRY 1\n");
    fprintf(file, "count=%zu\n", kernel->experiment_count);
    for (size_t i = 0; i < kernel->experiment_count; ++i) {
        const NxExperiment* experiment = &kernel->experiments[i];
        fprintf(file, "%s|%s|%s|%u\n",
                experiment->id,
                nx_experiment_status_to_string(experiment->status),
                experiment->module,
                nx_experiment_hash(experiment));
    }

    fclose(file);
    return NX_OK;
}

NxResult nx_research_kernel_append_journal(const NxResearchKernel* kernel, const char* event_name, const char* experiment_id) {
    if (kernel == NULL || !kernel->initialized || event_name == NULL || experiment_id == NULL) {
        return NX_ERROR_ARGUMENT;
    }

    FILE* file = fopen(kernel->journal_path, "ab");
    if (file == NULL) {
        return NX_ERROR_IO;
    }

    fprintf(file, "%llu|%s|%s\n", (unsigned long long)time(NULL), event_name, experiment_id);
    fclose(file);

    /* Intentional cast: function is logically const for users, but stats are runtime telemetry. */
    ((NxResearchKernel*)kernel)->stats.journal_events += 1;
    return NX_OK;
}

NxResearchKernelStats nx_research_kernel_get_stats(const NxResearchKernel* kernel) {
    NxResearchKernelStats empty;
    memset(&empty, 0, sizeof(empty));
    if (kernel == NULL) {
        return empty;
    }
    return kernel->stats;
}
