#ifndef NEXIORA_RESEARCH_NX_EVIDENCE_CORRELATION_H
#define NEXIORA_RESEARCH_NX_EVIDENCE_CORRELATION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxEvidenceCorrelationStatus
{
    NX_EVIDENCE_CORRELATION_OK = 0,
    NX_EVIDENCE_CORRELATION_INVALID_ARGUMENT = 1,
    NX_EVIDENCE_CORRELATION_OUT_OF_MEMORY = 2,
    NX_EVIDENCE_CORRELATION_NOT_FOUND = 3,
    NX_EVIDENCE_CORRELATION_INVALID_WEIGHT = 4
} NxEvidenceCorrelationStatus;

typedef enum NxEvidencePolarity
{
    NX_EVIDENCE_POLARITY_NEUTRAL = 0,
    NX_EVIDENCE_POLARITY_SUPPORTS = 1,
    NX_EVIDENCE_POLARITY_CONTRADICTS = 2
} NxEvidencePolarity;

typedef enum NxEvidenceCorrelationVerdict
{
    NX_EVIDENCE_CORRELATION_VERDICT_INSUFFICIENT = 0,
    NX_EVIDENCE_CORRELATION_VERDICT_CONVERGENT = 1,
    NX_EVIDENCE_CORRELATION_VERDICT_CONFLICTING = 2,
    NX_EVIDENCE_CORRELATION_VERDICT_NEUTRAL = 3
} NxEvidenceCorrelationVerdict;

typedef struct NxEvidenceObservation
{
    uint32_t id;
    uint32_t hypothesis_id;
    char evidence_key[96];
    NxEvidencePolarity polarity;
    int weight;
} NxEvidenceObservation;

typedef struct NxEvidenceCorrelationSummary
{
    uint32_t hypothesis_id;
    size_t support_count;
    size_t contradiction_count;
    size_t neutral_count;
    int support_weight;
    int contradiction_weight;
    int neutral_weight;
    int confidence_delta;
    NxEvidenceCorrelationVerdict verdict;
} NxEvidenceCorrelationSummary;

typedef struct NxEvidenceCorrelationEngine
{
    NxEvidenceObservation* observations;
    size_t observation_count;
    size_t observation_capacity;
    uint32_t next_observation_id;
} NxEvidenceCorrelationEngine;

NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_Init(NxEvidenceCorrelationEngine* engine);
void NxEvidenceCorrelationEngine_Shutdown(NxEvidenceCorrelationEngine* engine);
void NxEvidenceCorrelationEngine_Clear(NxEvidenceCorrelationEngine* engine);

NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_AddObservation(
    NxEvidenceCorrelationEngine* engine,
    uint32_t hypothesis_id,
    const char* evidence_key,
    NxEvidencePolarity polarity,
    int weight,
    uint32_t* observation_id_out);

size_t NxEvidenceCorrelationEngine_GetObservationCount(const NxEvidenceCorrelationEngine* engine);

const NxEvidenceObservation* NxEvidenceCorrelationEngine_FindObservationById(
    const NxEvidenceCorrelationEngine* engine,
    uint32_t observation_id);

NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_Summarize(
    const NxEvidenceCorrelationEngine* engine,
    uint32_t hypothesis_id,
    NxEvidenceCorrelationSummary* summary_out);

#ifdef __cplusplus
}
#endif

#endif
