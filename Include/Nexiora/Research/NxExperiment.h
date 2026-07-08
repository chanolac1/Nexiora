#ifndef NEXIORA_RESEARCH_NXEXPERIMENT_H
#define NEXIORA_RESEARCH_NXEXPERIMENT_H

#include "Nexiora/NCP/Common/NxResult.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_EXPERIMENT_ID_MAX 32
#define NX_EXPERIMENT_TITLE_MAX 128
#define NX_EXPERIMENT_AUTHOR_MAX 64
#define NX_EXPERIMENT_MODULE_MAX 64
#define NX_EXPERIMENT_TEXT_MAX 512
#define NX_EXPERIMENT_PATH_MAX 260

typedef enum NxExperimentStatus {
    NX_EXPERIMENT_STATUS_DRAFT = 0,
    NX_EXPERIMENT_STATUS_PREPARED,
    NX_EXPERIMENT_STATUS_RUNNING,
    NX_EXPERIMENT_STATUS_MEASURED,
    NX_EXPERIMENT_STATUS_VALIDATED,
    NX_EXPERIMENT_STATUS_APPROVED,
    NX_EXPERIMENT_STATUS_INTEGRATED,
    NX_EXPERIMENT_STATUS_ARCHIVED,
    NX_EXPERIMENT_STATUS_REJECTED
} NxExperimentStatus;

typedef struct NxExperiment {
    char id[NX_EXPERIMENT_ID_MAX];
    char title[NX_EXPERIMENT_TITLE_MAX];
    char author[NX_EXPERIMENT_AUTHOR_MAX];
    char module[NX_EXPERIMENT_MODULE_MAX];
    char hypothesis[NX_EXPERIMENT_TEXT_MAX];
    char target[NX_EXPERIMENT_MODULE_MAX];
    NxExperimentStatus status;
    uint64_t created_unix_seconds;
    uint64_t last_execution_unix_seconds;
} NxExperiment;

const char* nx_experiment_status_to_string(NxExperimentStatus status);
NxResult nx_experiment_initialize(NxExperiment* experiment,
                                  const char* id,
                                  const char* title,
                                  const char* author,
                                  const char* module,
                                  const char* hypothesis,
                                  const char* target);
NxResult nx_experiment_transition(NxExperiment* experiment, NxExperimentStatus next_status);
int nx_experiment_transition_is_valid(NxExperimentStatus current_status, NxExperimentStatus next_status);
uint32_t nx_experiment_hash(const NxExperiment* experiment);
NxResult nx_experiment_write_manifest(const NxExperiment* experiment, const char* path);

#ifdef __cplusplus
}
#endif

#endif
