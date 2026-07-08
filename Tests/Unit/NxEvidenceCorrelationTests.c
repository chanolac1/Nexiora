#include "Nexiora/Research/NxEvidenceCorrelation.h"

#include <stdio.h>

int main(void)
{
    NxEvidenceCorrelationEngine engine;
    NxEvidenceCorrelationSummary summary;
    NxEvidenceCorrelationStatus status;
    const NxEvidenceObservation* observation;
    uint32_t first_id;
    uint32_t ignored_id;

    status = NxEvidenceCorrelationEngine_Init(&engine);
    if (status != NX_EVIDENCE_CORRELATION_OK)
    {
        printf("Init failed\n");
        return 1;
    }

    status = NxEvidenceCorrelationEngine_AddObservation(
        &engine,
        7u,
        "bench/vector-growth/run-001",
        NX_EVIDENCE_POLARITY_SUPPORTS,
        45,
        &first_id);

    if (status != NX_EVIDENCE_CORRELATION_OK || first_id == 0u)
    {
        printf("Add support failed\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    (void)NxEvidenceCorrelationEngine_AddObservation(
        &engine,
        7u,
        "bench/vector-growth/run-002",
        NX_EVIDENCE_POLARITY_SUPPORTS,
        35,
        &ignored_id);

    (void)NxEvidenceCorrelationEngine_AddObservation(
        &engine,
        7u,
        "journal/vector-growth/note-001",
        NX_EVIDENCE_POLARITY_NEUTRAL,
        10,
        &ignored_id);

    (void)NxEvidenceCorrelationEngine_AddObservation(
        &engine,
        8u,
        "bench/string-cache/run-001",
        NX_EVIDENCE_POLARITY_CONTRADICTS,
        90,
        &ignored_id);

    if (NxEvidenceCorrelationEngine_GetObservationCount(&engine) != 4u)
    {
        printf("Unexpected observation count\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    observation = NxEvidenceCorrelationEngine_FindObservationById(&engine, first_id);
    if (observation == 0 || observation->hypothesis_id != 7u || observation->weight != 45)
    {
        printf("Observation lookup failed\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    status = NxEvidenceCorrelationEngine_Summarize(&engine, 7u, &summary);
    if (status != NX_EVIDENCE_CORRELATION_OK)
    {
        printf("Summarize failed\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    if (summary.support_count != 2u || summary.contradiction_count != 0u || summary.neutral_count != 1u)
    {
        printf("Unexpected summary counts\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    if (summary.support_weight != 80 || summary.confidence_delta != 80)
    {
        printf("Unexpected summary weights\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    if (summary.verdict != NX_EVIDENCE_CORRELATION_VERDICT_CONVERGENT)
    {
        printf("Expected convergent evidence\n");
        NxEvidenceCorrelationEngine_Shutdown(&engine);
        return 1;
    }

    NxEvidenceCorrelationEngine_Shutdown(&engine);
    return 0;
}
