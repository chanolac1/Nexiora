#ifndef NEXIORA_NCP_RESEARCH_NXRESEARCHEVIDENCE_H
#define NEXIORA_NCP_RESEARCH_NXRESEARCHEVIDENCE_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/NCP/Research/NxExperimentRunner.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_RESEARCH_EVIDENCE_MESSAGE_MAX 256

typedef enum NxResearchEvidenceStatus
{
    NX_RESEARCH_EVIDENCE_UNKNOWN = 0,
    NX_RESEARCH_EVIDENCE_VALID = 1,
    NX_RESEARCH_EVIDENCE_INCONCLUSIVE = 2,
    NX_RESEARCH_EVIDENCE_REJECTED = 3
} NxResearchEvidenceStatus;

typedef struct NxResearchEvidence
{
    char experiment_id[NX_RUNNER_EXPERIMENT_ID_MAX];
    NxResearchEvidenceStatus status;
    double elapsed_seconds;
    double baseline_seconds;
    double change_percent;
    char recommendation[NX_RESEARCH_EVIDENCE_MESSAGE_MAX];
} NxResearchEvidence;

const char* nx_research_evidence_status_to_string(NxResearchEvidenceStatus status);

NxResult nx_research_evidence_from_run(
    const NxExperimentRunResult* run,
    double baseline_seconds,
    NxResearchEvidence* out_evidence
);

NxResult nx_research_evidence_write_report(
    const char* path,
    const NxResearchEvidence* evidence
);

#ifdef __cplusplus
}
#endif

#endif
