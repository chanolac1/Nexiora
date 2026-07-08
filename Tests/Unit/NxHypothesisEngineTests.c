#include "Nexiora/Research/NxHypothesisEngine.h"

#include <stdio.h>

int main(void)
{
    NxHypothesisEngine engine;
    NxHypothesisStatus status;
    const NxHypothesis* hypothesis;
    uint32_t id;

    status = NxHypothesisEngine_Init(&engine);
    if (status != NX_HYPOTHESIS_OK)
    {
        printf("Init failed\n");
        return 1;
    }

    status = NxHypothesisEngine_Propose(
        &engine,
        "NxVector growth improves append performance",
        "NxVector capacity growth should reduce allocations",
        60,
        &id);

    if (status != NX_HYPOTHESIS_OK || id == 0u)
    {
        printf("Propose failed\n");
        NxHypothesisEngine_Shutdown(&engine);
        return 1;
    }

    if (NxHypothesisEngine_GetCount(&engine) != 1u)
    {
        printf("Unexpected hypothesis count\n");
        NxHypothesisEngine_Shutdown(&engine);
        return 1;
    }

    status = NxHypothesisEngine_AddSupport(&engine, id, 25);
    if (status != NX_HYPOTHESIS_OK)
    {
        printf("Support failed\n");
        NxHypothesisEngine_Shutdown(&engine);
        return 1;
    }

    hypothesis = NxHypothesisEngine_FindById(&engine, id);
    if (hypothesis == 0 || hypothesis->confidence != 85 || hypothesis->state != NX_HYPOTHESIS_STATE_SUPPORTED)
    {
        printf("Hypothesis was not supported\n");
        NxHypothesisEngine_Shutdown(&engine);
        return 1;
    }

    status = NxHypothesisEngine_AddContradiction(&engine, id, 70);
    if (status != NX_HYPOTHESIS_OK)
    {
        printf("Contradiction failed\n");
        NxHypothesisEngine_Shutdown(&engine);
        return 1;
    }

    hypothesis = NxHypothesisEngine_FindByTitle(&engine, "NxVector growth improves append performance");
    if (hypothesis == 0 || hypothesis->state != NX_HYPOTHESIS_STATE_CONTESTED)
    {
        printf("Hypothesis was not contested\n");
        NxHypothesisEngine_Shutdown(&engine);
        return 1;
    }

    NxHypothesisEngine_Shutdown(&engine);
    return 0;
}
