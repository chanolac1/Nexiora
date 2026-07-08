#ifndef NEXIORA_NCP_RESEARCH_NXEXPERIMENTRUNNER_H
#define NEXIORA_NCP_RESEARCH_NXEXPERIMENTRUNNER_H

#include <stddef.h>
#include <stdint.h>

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/NCP/Research/NxJournal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_RUNNER_EXPERIMENT_ID_MAX 32
#define NX_RUNNER_PATH_MAX 260
#define NX_RUNNER_MESSAGE_MAX 256

typedef enum NxExperimentRunStatus
{
    NX_EXPERIMENT_RUN_UNKNOWN = 0,
    NX_EXPERIMENT_RUN_PREPARED = 1,
    NX_EXPERIMENT_RUN_EXECUTING = 2,
    NX_EXPERIMENT_RUN_COMPLETED = 3,
    NX_EXPERIMENT_RUN_FAILED = 4
} NxExperimentRunStatus;

typedef struct NxExperimentRunResult
{
    char experiment_id[NX_RUNNER_EXPERIMENT_ID_MAX];
    NxExperimentRunStatus status;
    uint64_t started_unix;
    uint64_t finished_unix;
    double elapsed_seconds;
    char message[NX_RUNNER_MESSAGE_MAX];
} NxExperimentRunResult;

typedef struct NxExperimentRunner
{
    char workspace_path[NX_RUNNER_PATH_MAX];
    char report_path[NX_RUNNER_PATH_MAX];
    NxJournal* journal;
    size_t runs_completed;
    size_t runs_failed;
} NxExperimentRunner;

const char* nx_experiment_run_status_to_string(NxExperimentRunStatus status);

NxResult nx_experiment_runner_initialize(
    NxExperimentRunner* runner,
    const char* workspace_path,
    NxJournal* journal
);

NxResult nx_experiment_runner_execute(
    NxExperimentRunner* runner,
    const char* experiment_id,
    NxExperimentRunResult* out_result
);

NxResult nx_experiment_runner_write_report(
    const NxExperimentRunner* runner,
    const NxExperimentRunResult* result
);

NxResult nx_experiment_runner_shutdown(NxExperimentRunner* runner);

#ifdef __cplusplus
}
#endif

#endif
