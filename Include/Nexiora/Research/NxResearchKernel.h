#ifndef NEXIORA_RESEARCH_NXRESEARCHKERNEL_H
#define NEXIORA_RESEARCH_NXRESEARCHKERNEL_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/Research/NxExperiment.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_RESEARCH_KERNEL_VERSION 1
#define NX_RESEARCH_MAX_EXPERIMENTS 128

typedef struct NxResearchKernelConfig {
    const char* root_path;
    const char* registry_path;
    const char* journal_path;
} NxResearchKernelConfig;

typedef struct NxResearchKernelStats {
    uint64_t initialized_count;
    uint64_t created_experiments;
    uint64_t journal_events;
    size_t registered_experiment_count;
} NxResearchKernelStats;

typedef struct NxResearchKernel {
    int initialized;
    char root_path[NX_EXPERIMENT_PATH_MAX];
    char registry_path[NX_EXPERIMENT_PATH_MAX];
    char journal_path[NX_EXPERIMENT_PATH_MAX];
    NxExperiment experiments[NX_RESEARCH_MAX_EXPERIMENTS];
    size_t experiment_count;
    NxResearchKernelStats stats;
} NxResearchKernel;

NxResult nx_research_kernel_initialize(NxResearchKernel* kernel, const NxResearchKernelConfig* config);
void nx_research_kernel_shutdown(NxResearchKernel* kernel);
NxResult nx_research_kernel_create_experiment(NxResearchKernel* kernel, const NxExperiment* experiment);
NxResult nx_research_kernel_write_registry(const NxResearchKernel* kernel);
NxResult nx_research_kernel_append_journal(const NxResearchKernel* kernel, const char* event_name, const char* experiment_id);
NxResearchKernelStats nx_research_kernel_get_stats(const NxResearchKernel* kernel);

#ifdef __cplusplus
}
#endif

#endif
