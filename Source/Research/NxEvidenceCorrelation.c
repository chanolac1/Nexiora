#include "Nexiora/Research/NxEvidenceCorrelation.h"

#include <stdlib.h>
#include <string.h>

static void NxEvidenceCorrelation_Copy(char* destination, size_t destination_size, const char* source)
{
    size_t length;

    if (destination == 0 || destination_size == 0)
    {
        return;
    }

    destination[0] = '\0';

    if (source == 0)
    {
        return;
    }

    length = strlen(source);
    if (length >= destination_size)
    {
        length = destination_size - 1u;
    }

    (void)memcpy(destination, source, length);
    destination[length] = '\0';
}

static NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_EnsureCapacity(
    NxEvidenceCorrelationEngine* engine,
    size_t wanted_capacity)
{
    size_t new_capacity;
    NxEvidenceObservation* new_observations;

    if (engine == 0)
    {
        return NX_EVIDENCE_CORRELATION_INVALID_ARGUMENT;
    }

    if (engine->observation_capacity >= wanted_capacity)
    {
        return NX_EVIDENCE_CORRELATION_OK;
    }

    new_capacity = engine->observation_capacity == 0u ? 8u : engine->observation_capacity * 2u;
    while (new_capacity < wanted_capacity)
    {
        new_capacity *= 2u;
    }

    new_observations = (NxEvidenceObservation*)realloc(
        engine->observations,
        new_capacity * sizeof(NxEvidenceObservation));

    if (new_observations == 0)
    {
        return NX_EVIDENCE_CORRELATION_OUT_OF_MEMORY;
    }

    engine->observations = new_observations;
    engine->observation_capacity = new_capacity;
    return NX_EVIDENCE_CORRELATION_OK;
}

static NxEvidenceCorrelationVerdict NxEvidenceCorrelation_DeriveVerdict(
    size_t support_count,
    size_t contradiction_count,
    size_t neutral_count,
    int support_weight,
    int contradiction_weight)
{
    int total_directional_weight;
    int smaller_weight;
    int larger_weight;

    if (support_count == 0u && contradiction_count == 0u)
    {
        return neutral_count > 0u ? NX_EVIDENCE_CORRELATION_VERDICT_NEUTRAL : NX_EVIDENCE_CORRELATION_VERDICT_INSUFFICIENT;
    }

    if ((support_count + contradiction_count) < 2u)
    {
        return NX_EVIDENCE_CORRELATION_VERDICT_INSUFFICIENT;
    }

    if (support_weight > 0 && contradiction_weight > 0)
    {
        smaller_weight = support_weight < contradiction_weight ? support_weight : contradiction_weight;
        larger_weight = support_weight > contradiction_weight ? support_weight : contradiction_weight;
        if (larger_weight > 0 && (smaller_weight * 100) >= (larger_weight * 45))
        {
            return NX_EVIDENCE_CORRELATION_VERDICT_CONFLICTING;
        }
    }

    total_directional_weight = support_weight + contradiction_weight;
    if (total_directional_weight >= 50 && support_weight > contradiction_weight)
    {
        return NX_EVIDENCE_CORRELATION_VERDICT_CONVERGENT;
    }

    if (total_directional_weight >= 50 && contradiction_weight > support_weight)
    {
        return NX_EVIDENCE_CORRELATION_VERDICT_CONFLICTING;
    }

    return NX_EVIDENCE_CORRELATION_VERDICT_INSUFFICIENT;
}

NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_Init(NxEvidenceCorrelationEngine* engine)
{
    if (engine == 0)
    {
        return NX_EVIDENCE_CORRELATION_INVALID_ARGUMENT;
    }

    engine->observations = 0;
    engine->observation_count = 0u;
    engine->observation_capacity = 0u;
    engine->next_observation_id = 1u;

    return NX_EVIDENCE_CORRELATION_OK;
}

void NxEvidenceCorrelationEngine_Shutdown(NxEvidenceCorrelationEngine* engine)
{
    if (engine == 0)
    {
        return;
    }

    free(engine->observations);
    engine->observations = 0;
    engine->observation_count = 0u;
    engine->observation_capacity = 0u;
    engine->next_observation_id = 1u;
}

void NxEvidenceCorrelationEngine_Clear(NxEvidenceCorrelationEngine* engine)
{
    if (engine == 0)
    {
        return;
    }

    engine->observation_count = 0u;
    engine->next_observation_id = 1u;
}

NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_AddObservation(
    NxEvidenceCorrelationEngine* engine,
    uint32_t hypothesis_id,
    const char* evidence_key,
    NxEvidencePolarity polarity,
    int weight,
    uint32_t* observation_id_out)
{
    NxEvidenceCorrelationStatus status;
    NxEvidenceObservation* observation;

    if (engine == 0 || hypothesis_id == 0u || evidence_key == 0 || observation_id_out == 0)
    {
        return NX_EVIDENCE_CORRELATION_INVALID_ARGUMENT;
    }

    if (weight < 0 || weight > 100)
    {
        return NX_EVIDENCE_CORRELATION_INVALID_WEIGHT;
    }

    status = NxEvidenceCorrelationEngine_EnsureCapacity(engine, engine->observation_count + 1u);
    if (status != NX_EVIDENCE_CORRELATION_OK)
    {
        return status;
    }

    observation = &engine->observations[engine->observation_count++];
    observation->id = engine->next_observation_id++;
    observation->hypothesis_id = hypothesis_id;
    NxEvidenceCorrelation_Copy(observation->evidence_key, sizeof(observation->evidence_key), evidence_key);
    observation->polarity = polarity;
    observation->weight = weight;

    *observation_id_out = observation->id;
    return NX_EVIDENCE_CORRELATION_OK;
}

size_t NxEvidenceCorrelationEngine_GetObservationCount(const NxEvidenceCorrelationEngine* engine)
{
    if (engine == 0)
    {
        return 0u;
    }

    return engine->observation_count;
}

const NxEvidenceObservation* NxEvidenceCorrelationEngine_FindObservationById(
    const NxEvidenceCorrelationEngine* engine,
    uint32_t observation_id)
{
    size_t index;

    if (engine == 0)
    {
        return 0;
    }

    for (index = 0u; index < engine->observation_count; ++index)
    {
        if (engine->observations[index].id == observation_id)
        {
            return &engine->observations[index];
        }
    }

    return 0;
}

NxEvidenceCorrelationStatus NxEvidenceCorrelationEngine_Summarize(
    const NxEvidenceCorrelationEngine* engine,
    uint32_t hypothesis_id,
    NxEvidenceCorrelationSummary* summary_out)
{
    size_t index;

    if (engine == 0 || hypothesis_id == 0u || summary_out == 0)
    {
        return NX_EVIDENCE_CORRELATION_INVALID_ARGUMENT;
    }

    summary_out->hypothesis_id = hypothesis_id;
    summary_out->support_count = 0u;
    summary_out->contradiction_count = 0u;
    summary_out->neutral_count = 0u;
    summary_out->support_weight = 0;
    summary_out->contradiction_weight = 0;
    summary_out->neutral_weight = 0;
    summary_out->confidence_delta = 0;
    summary_out->verdict = NX_EVIDENCE_CORRELATION_VERDICT_INSUFFICIENT;

    for (index = 0u; index < engine->observation_count; ++index)
    {
        const NxEvidenceObservation* observation = &engine->observations[index];
        if (observation->hypothesis_id != hypothesis_id)
        {
            continue;
        }

        if (observation->polarity == NX_EVIDENCE_POLARITY_SUPPORTS)
        {
            summary_out->support_count += 1u;
            summary_out->support_weight += observation->weight;
        }
        else if (observation->polarity == NX_EVIDENCE_POLARITY_CONTRADICTS)
        {
            summary_out->contradiction_count += 1u;
            summary_out->contradiction_weight += observation->weight;
        }
        else
        {
            summary_out->neutral_count += 1u;
            summary_out->neutral_weight += observation->weight;
        }
    }

    summary_out->confidence_delta = summary_out->support_weight - summary_out->contradiction_weight;
    summary_out->verdict = NxEvidenceCorrelation_DeriveVerdict(
        summary_out->support_count,
        summary_out->contradiction_count,
        summary_out->neutral_count,
        summary_out->support_weight,
        summary_out->contradiction_weight);

    return NX_EVIDENCE_CORRELATION_OK;
}
