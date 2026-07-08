#include "Nexiora/NCP/Research/NxResearchEvidence.h"

#include <stdio.h>
#include <stdlib.h>

static int test_evidence_from_completed_run(void)
{
    NxExperimentRunResult run;
    run.status = NX_EXPERIMENT_RUN_COMPLETED;
    run.elapsed_seconds = 9.0;
    snprintf(run.experiment_id, sizeof(run.experiment_id), "LAB-0005");

    NxResearchEvidence evidence;
    if (nx_research_evidence_from_run(&run, 10.0, &evidence) != NX_OK)
        return 1;

    if (evidence.status != NX_RESEARCH_EVIDENCE_VALID)
        return 2;

    if (evidence.change_percent < 9.9 || evidence.change_percent > 10.1)
        return 3;

    if (nx_research_evidence_write_report("Research/Reports/evidence_test.md", &evidence) != NX_OK)
        return 4;

    remove("Research/Reports/evidence_test.md");
    return 0;
}

int main(void)
{
    int rc = test_evidence_from_completed_run();

    if (rc != 0)
    {
        printf("NxResearchEvidenceTests failed: %d\n", rc);
        return rc;
    }

    printf("NxResearchEvidenceTests passed\n");
    return 0;
}
