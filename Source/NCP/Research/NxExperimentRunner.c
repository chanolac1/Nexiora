#include "Nexiora/NCP/Research/NxExperimentRunner.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static void nx_runner_copy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    size_t i = 0;
    for (; i + 1 < dst_size && src[i] != '\0'; ++i)
        dst[i] = src[i];
    dst[i] = '\0';
}

static uint64_t nx_runner_now_unix(void)
{
    return (uint64_t)time(NULL);
}

const char* nx_experiment_run_status_to_string(NxExperimentRunStatus status)
{
    switch (status)
    {
        case NX_EXPERIMENT_RUN_PREPARED: return "Prepared";
        case NX_EXPERIMENT_RUN_EXECUTING: return "Executing";
        case NX_EXPERIMENT_RUN_COMPLETED: return "Completed";
        case NX_EXPERIMENT_RUN_FAILED: return "Failed";
        default: return "Unknown";
    }
}

NxResult nx_experiment_runner_initialize(
    NxExperimentRunner* runner,
    const char* workspace_path,
    NxJournal* journal
)
{
    if (!runner || !workspace_path || workspace_path[0] == '\0')
        return NX_ERROR_ARGUMENT;

    nx_runner_copy(runner->workspace_path, sizeof(runner->workspace_path), workspace_path);
    nx_runner_copy(runner->report_path, sizeof(runner->report_path), "Research/Reports/latest_experiment_report.md");
    runner->journal = journal;
    runner->runs_completed = 0;
    runner->runs_failed = 0;

    return NX_OK;
}

NxResult nx_experiment_runner_execute(
    NxExperimentRunner* runner,
    const char* experiment_id,
    NxExperimentRunResult* out_result
)
{
    if (!runner || !experiment_id || experiment_id[0] == '\0' || !out_result)
        return NX_ERROR_ARGUMENT;

    memset(out_result, 0, sizeof(*out_result));
    nx_runner_copy(out_result->experiment_id, sizeof(out_result->experiment_id), experiment_id);

    out_result->status = NX_EXPERIMENT_RUN_EXECUTING;
    out_result->started_unix = nx_runner_now_unix();

    if (runner->journal)
    {
        NxJournalEvent started = nx_journal_make_event(
            NX_JOURNAL_EVENT_EXPERIMENT_STARTED,
            experiment_id,
            "ExperimentRunner",
            "Experiment execution started"
        );
        nx_journal_write_event(runner->journal, &started);
    }

    /*
        NRL-0004 intentionally implements the runner skeleton only.
        Real experiment plugins will be attached in NRL-0005/NRL-0006.
        For now, the runner validates lifecycle, journal integration and report generation.
    */

    out_result->finished_unix = nx_runner_now_unix();
    out_result->elapsed_seconds = (double)(out_result->finished_unix - out_result->started_unix);
    out_result->status = NX_EXPERIMENT_RUN_COMPLETED;
    nx_runner_copy(out_result->message, sizeof(out_result->message), "Runner lifecycle completed");

    runner->runs_completed++;

    if (runner->journal)
    {
        NxJournalEvent completed = nx_journal_make_event(
            NX_JOURNAL_EVENT_BENCHMARK_COMPLETED,
            experiment_id,
            "ExperimentRunner",
            "Experiment execution completed"
        );
        nx_journal_write_event(runner->journal, &completed);
    }

    return NX_OK;
}

NxResult nx_experiment_runner_write_report(
    const NxExperimentRunner* runner,
    const NxExperimentRunResult* result
)
{
    if (!runner || !result || runner->report_path[0] == '\0')
        return NX_ERROR_ARGUMENT;

    FILE* file = fopen(runner->report_path, "wb");
    if (!file)
        return NX_ERROR_IO;

    fprintf(file, "# Experiment Report\n\n");
    fprintf(file, "## Experiment\n%s\n\n", result->experiment_id);
    fprintf(file, "## Status\n%s\n\n", nx_experiment_run_status_to_string(result->status));
    fprintf(file, "## Elapsed Seconds\n%.6f\n\n", result->elapsed_seconds);
    fprintf(file, "## Message\n%s\n", result->message);

    fclose(file);
    return NX_OK;
}

NxResult nx_experiment_runner_shutdown(NxExperimentRunner* runner)
{
    if (!runner)
        return NX_ERROR_ARGUMENT;

    runner->workspace_path[0] = '\0';
    runner->report_path[0] = '\0';
    runner->journal = NULL;
    runner->runs_completed = 0;
    runner->runs_failed = 0;

    return NX_OK;
}

