#ifndef NEXIORA_RESEARCH_NX_HYPOTHESIS_ENGINE_H
#define NEXIORA_RESEARCH_NX_HYPOTHESIS_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxHypothesisStatus
{
    NX_HYPOTHESIS_OK = 0,
    NX_HYPOTHESIS_INVALID_ARGUMENT = 1,
    NX_HYPOTHESIS_OUT_OF_MEMORY = 2,
    NX_HYPOTHESIS_NOT_FOUND = 3,
    NX_HYPOTHESIS_INVALID_CONFIDENCE = 4
} NxHypothesisStatus;

typedef enum NxHypothesisState
{
    NX_HYPOTHESIS_STATE_PROPOSED = 0,
    NX_HYPOTHESIS_STATE_SUPPORTED = 1,
    NX_HYPOTHESIS_STATE_CONTESTED = 2,
    NX_HYPOTHESIS_STATE_REJECTED = 3
} NxHypothesisState;

typedef struct NxHypothesis
{
    uint32_t id;
    char title[96];
    char rationale[192];
    int confidence;
    size_t support_count;
    size_t contradiction_count;
    NxHypothesisState state;
} NxHypothesis;

typedef struct NxHypothesisEngine
{
    NxHypothesis* hypotheses;
    size_t hypothesis_count;
    size_t hypothesis_capacity;
    uint32_t next_hypothesis_id;
} NxHypothesisEngine;

NxHypothesisStatus NxHypothesisEngine_Init(NxHypothesisEngine* engine);
void NxHypothesisEngine_Shutdown(NxHypothesisEngine* engine);
void NxHypothesisEngine_Clear(NxHypothesisEngine* engine);

NxHypothesisStatus NxHypothesisEngine_Propose(
    NxHypothesisEngine* engine,
    const char* title,
    const char* rationale,
    int initial_confidence,
    uint32_t* hypothesis_id_out);

NxHypothesisStatus NxHypothesisEngine_AddSupport(
    NxHypothesisEngine* engine,
    uint32_t hypothesis_id,
    int evidence_weight);

NxHypothesisStatus NxHypothesisEngine_AddContradiction(
    NxHypothesisEngine* engine,
    uint32_t hypothesis_id,
    int evidence_weight);

size_t NxHypothesisEngine_GetCount(const NxHypothesisEngine* engine);

const NxHypothesis* NxHypothesisEngine_FindById(
    const NxHypothesisEngine* engine,
    uint32_t hypothesis_id);

const NxHypothesis* NxHypothesisEngine_FindByTitle(
    const NxHypothesisEngine* engine,
    const char* title);

#ifdef __cplusplus
}
#endif

#endif
