#ifndef NEXIORA_RESEARCH_NX_REASONING_ENGINE_H
#define NEXIORA_RESEARCH_NX_REASONING_ENGINE_H

#include "Nexiora/Research/NxHypothesisEngine.h"
#include "Nexiora/Research/NxKnowledgeBase.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxReasoningStatus
{
    NX_REASONING_OK = 0,
    NX_REASONING_INVALID_ARGUMENT = 1,
    NX_REASONING_OUT_OF_CAPACITY = 2
} NxReasoningStatus;

typedef enum NxReasoningConclusionType
{
    NX_REASONING_CONCLUSION_UNKNOWN = 0,
    NX_REASONING_CONCLUSION_FACT_SUPPORTS_HYPOTHESIS = 1,
    NX_REASONING_CONCLUSION_FACT_CONTRADICTS_HYPOTHESIS = 2,
    NX_REASONING_CONCLUSION_CONFLICT_DETECTED = 3
} NxReasoningConclusionType;

typedef struct NxReasoningConclusion
{
    NxReasoningConclusionType type;
    uint32_t hypothesis_id;
    uint32_t fact_id;
    int confidence;
    char message[160];
} NxReasoningConclusion;

typedef struct NxReasoningReport
{
    NxReasoningConclusion* conclusions;
    size_t conclusion_count;
    size_t conclusion_capacity;
} NxReasoningReport;

NxReasoningStatus NxReasoningReport_Init(
    NxReasoningReport* report,
    NxReasoningConclusion* buffer,
    size_t capacity);

void NxReasoningReport_Clear(NxReasoningReport* report);

NxReasoningStatus NxReasoningEngine_EvaluateHypotheses(
    const NxKnowledgeBase* knowledge_base,
    const NxHypothesisEngine* hypothesis_engine,
    NxReasoningReport* report);

size_t NxReasoningEngine_CountConflicts(
    const NxReasoningReport* report);

#ifdef __cplusplus
}
#endif

#endif
