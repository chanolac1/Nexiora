#include "Nexiora/Research/NxHypothesisEngine.h"

#include <stdlib.h>
#include <string.h>

static void NxHypothesis_Copy(char* destination, size_t destination_size, const char* source)
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
        length = destination_size - 1;
    }

    (void)memcpy(destination, source, length);
    destination[length] = '\0';
}

static int NxHypothesis_ClampConfidence(int confidence)
{
    if (confidence < 0)
    {
        return 0;
    }

    if (confidence > 100)
    {
        return 100;
    }

    return confidence;
}

static NxHypothesisState NxHypothesis_DeriveState(const NxHypothesis* hypothesis)
{
    if (hypothesis->confidence >= 80 && hypothesis->support_count > hypothesis->contradiction_count)
    {
        return NX_HYPOTHESIS_STATE_SUPPORTED;
    }

    if (hypothesis->confidence <= 25 && hypothesis->contradiction_count > hypothesis->support_count)
    {
        return NX_HYPOTHESIS_STATE_REJECTED;
    }

    if (hypothesis->contradiction_count > 0)
    {
        return NX_HYPOTHESIS_STATE_CONTESTED;
    }

    return NX_HYPOTHESIS_STATE_PROPOSED;
}

static NxHypothesis* NxHypothesisEngine_FindMutableById(NxHypothesisEngine* engine, uint32_t hypothesis_id)
{
    size_t index;

    if (engine == 0)
    {
        return 0;
    }

    for (index = 0; index < engine->hypothesis_count; ++index)
    {
        if (engine->hypotheses[index].id == hypothesis_id)
        {
            return &engine->hypotheses[index];
        }
    }

    return 0;
}

static NxHypothesisStatus NxHypothesisEngine_EnsureCapacity(NxHypothesisEngine* engine, size_t wanted_capacity)
{
    size_t new_capacity;
    NxHypothesis* new_items;

    if (engine == 0)
    {
        return NX_HYPOTHESIS_INVALID_ARGUMENT;
    }

    if (engine->hypothesis_capacity >= wanted_capacity)
    {
        return NX_HYPOTHESIS_OK;
    }

    new_capacity = engine->hypothesis_capacity == 0 ? 8u : engine->hypothesis_capacity * 2u;
    while (new_capacity < wanted_capacity)
    {
        new_capacity *= 2u;
    }

    new_items = (NxHypothesis*)realloc(engine->hypotheses, new_capacity * sizeof(NxHypothesis));
    if (new_items == 0)
    {
        return NX_HYPOTHESIS_OUT_OF_MEMORY;
    }

    engine->hypotheses = new_items;
    engine->hypothesis_capacity = new_capacity;
    return NX_HYPOTHESIS_OK;
}

NxHypothesisStatus NxHypothesisEngine_Init(NxHypothesisEngine* engine)
{
    if (engine == 0)
    {
        return NX_HYPOTHESIS_INVALID_ARGUMENT;
    }

    engine->hypotheses = 0;
    engine->hypothesis_count = 0;
    engine->hypothesis_capacity = 0;
    engine->next_hypothesis_id = 1u;

    return NX_HYPOTHESIS_OK;
}

void NxHypothesisEngine_Shutdown(NxHypothesisEngine* engine)
{
    if (engine == 0)
    {
        return;
    }

    free(engine->hypotheses);
    engine->hypotheses = 0;
    engine->hypothesis_count = 0;
    engine->hypothesis_capacity = 0;
    engine->next_hypothesis_id = 1u;
}

void NxHypothesisEngine_Clear(NxHypothesisEngine* engine)
{
    if (engine == 0)
    {
        return;
    }

    engine->hypothesis_count = 0;
    engine->next_hypothesis_id = 1u;
}

NxHypothesisStatus NxHypothesisEngine_Propose(
    NxHypothesisEngine* engine,
    const char* title,
    const char* rationale,
    int initial_confidence,
    uint32_t* hypothesis_id_out)
{
    NxHypothesisStatus status;
    NxHypothesis* hypothesis;

    if (engine == 0 || title == 0 || rationale == 0 || hypothesis_id_out == 0)
    {
        return NX_HYPOTHESIS_INVALID_ARGUMENT;
    }

    if (initial_confidence < 0 || initial_confidence > 100)
    {
        return NX_HYPOTHESIS_INVALID_CONFIDENCE;
    }

    status = NxHypothesisEngine_EnsureCapacity(engine, engine->hypothesis_count + 1u);
    if (status != NX_HYPOTHESIS_OK)
    {
        return status;
    }

    hypothesis = &engine->hypotheses[engine->hypothesis_count++];
    hypothesis->id = engine->next_hypothesis_id++;
    NxHypothesis_Copy(hypothesis->title, sizeof(hypothesis->title), title);
    NxHypothesis_Copy(hypothesis->rationale, sizeof(hypothesis->rationale), rationale);
    hypothesis->confidence = initial_confidence;
    hypothesis->support_count = 0;
    hypothesis->contradiction_count = 0;
    hypothesis->state = NX_HYPOTHESIS_STATE_PROPOSED;

    *hypothesis_id_out = hypothesis->id;
    return NX_HYPOTHESIS_OK;
}

NxHypothesisStatus NxHypothesisEngine_AddSupport(
    NxHypothesisEngine* engine,
    uint32_t hypothesis_id,
    int evidence_weight)
{
    NxHypothesis* hypothesis;

    if (engine == 0)
    {
        return NX_HYPOTHESIS_INVALID_ARGUMENT;
    }

    hypothesis = NxHypothesisEngine_FindMutableById(engine, hypothesis_id);
    if (hypothesis == 0)
    {
        return NX_HYPOTHESIS_NOT_FOUND;
    }

    hypothesis->support_count += 1u;
    hypothesis->confidence = NxHypothesis_ClampConfidence(hypothesis->confidence + evidence_weight);
    hypothesis->state = NxHypothesis_DeriveState(hypothesis);

    return NX_HYPOTHESIS_OK;
}

NxHypothesisStatus NxHypothesisEngine_AddContradiction(
    NxHypothesisEngine* engine,
    uint32_t hypothesis_id,
    int evidence_weight)
{
    NxHypothesis* hypothesis;

    if (engine == 0)
    {
        return NX_HYPOTHESIS_INVALID_ARGUMENT;
    }

    hypothesis = NxHypothesisEngine_FindMutableById(engine, hypothesis_id);
    if (hypothesis == 0)
    {
        return NX_HYPOTHESIS_NOT_FOUND;
    }

    hypothesis->contradiction_count += 1u;
    hypothesis->confidence = NxHypothesis_ClampConfidence(hypothesis->confidence - evidence_weight);
    hypothesis->state = NxHypothesis_DeriveState(hypothesis);

    return NX_HYPOTHESIS_OK;
}

size_t NxHypothesisEngine_GetCount(const NxHypothesisEngine* engine)
{
    if (engine == 0)
    {
        return 0;
    }

    return engine->hypothesis_count;
}

const NxHypothesis* NxHypothesisEngine_FindById(
    const NxHypothesisEngine* engine,
    uint32_t hypothesis_id)
{
    size_t index;

    if (engine == 0)
    {
        return 0;
    }

    for (index = 0; index < engine->hypothesis_count; ++index)
    {
        if (engine->hypotheses[index].id == hypothesis_id)
        {
            return &engine->hypotheses[index];
        }
    }

    return 0;
}

const NxHypothesis* NxHypothesisEngine_FindByTitle(
    const NxHypothesisEngine* engine,
    const char* title)
{
    size_t index;

    if (engine == 0 || title == 0)
    {
        return 0;
    }

    for (index = 0; index < engine->hypothesis_count; ++index)
    {
        if (strcmp(engine->hypotheses[index].title, title) == 0)
        {
            return &engine->hypotheses[index];
        }
    }

    return 0;
}
