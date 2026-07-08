#include "Nexiora/NCP/Research/NxExperimentRunner.h"

#include <stdio.h>
#include <stdlib.h>

static int test_runner_lifecycle(void)
{
    const char* journal_path = "nx_runner_test_journal.log";
    remove(journal_path);
    remove("Research/Reports/latest_experiment_report.md");

    NxJournal journal;
    if (nx_journal_open(&journal, journal_path) != NX_OK)
        return 1;

    NxExperimentRunner runner;
    if (nx_experiment_runner_initialize(&runner, "Research", &journal) != NX_OK)
        return 2;

    NxExperimentRunResult result;
    if (nx_experiment_runner_execute(&runner, "LAB-0004", &result) != NX_OK)
        return 3;

    if (result.status != NX_EXPERIMENT_RUN_COMPLETED)
        return 4;

    if (runner.runs_completed != 1)
        return 5;

    if (nx_experiment_runner_write_report(&runner, &result) != NX_OK)
        return 6;

    size_t journal_count = 0;
    if (nx_journal_read_count(journal_path, &journal_count) != NX_OK)
        return 7;

    if (journal_count < 2)
        return 8;

    nx_experiment_runner_shutdown(&runner);
    nx_journal_close(&journal);
    remove(journal_path);

    return 0;
}

int main(void)
{
    int rc = test_runner_lifecycle();
    if (rc != 0)
    {
        printf("NxExperimentRunnerTests failed: %d\n", rc);
        return rc;
    }

    printf("NxExperimentRunnerTests passed\n");
    return 0;
}
