#include "Nexiora/Research/NxReasoningEngine.h"

#include <stdio.h>
#include <string.h>

static void NxReasoning_Copy(char* destination, size_t destination_size, const char* source)
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

static int NxReasoning_StringContains(const char* text, const char* fragment)
{
    if (text == 0 || fragment == 0)
    {
        return 0;
    }

    return strstr(text, fragment) != 0;
}

static NxReasoningStatus NxReasoningReport_Add(
    NxReasoningReport* report,
    NxReasoningConclusionType type,
    uint32_t hypothesis_id,
    uint32_t fact_id,
    int confidence,
    const char* message)
{
    NxReasoningConclusion* conclusion;

    if (report == 0)
    {
        return NX_REASONING_INVALID_ARGUMENT;
    }

    if (report->conclusion_count >= report->conclusion_capacity)
    {
        return NX_REASONING_OUT_OF_CAPACITY;
    }

    conclusion = &report->conclusions[report->conclusion_count++];
    conclusion->type = type;
    conclusion->hypothesis_id = hypothesis_id;
    conclusion->fact_id = fact_id;
    conclusion->confidence = confidence;
    NxReasoning_Copy(conclusion->message, sizeof(conclusion->message), message);

    return NX_REASONING_OK;
}

NxReasoningStatus NxReasoningReport_Init(
    NxReasoningReport* report,
    NxReasoningConclusion* buffer,
    size_t capacity)
{
    if (report == 0 || buffer == 0 || capacity == 0)
    {
        return NX_REASONING_INVALID_ARGUMENT;
    }

    report->conclusions = buffer;
    report->conclusion_count = 0;
    report->conclusion_capacity = capacity;
    return NX_REASONING_OK;
}

void NxReasoningReport_Clear(NxReasoningReport* report)
{
    if (report == 0)
    {
        return;
    }

    report->conclusion_count = 0;
}

NxReasoningStatus NxReasoningEngine_EvaluateHypotheses(
    const NxKnowledgeBase* knowledge_base,
    const NxHypothesisEngine* hypothesis_engine,
    NxReasoningReport* report)
{
    size_t hypothesis_index;
    size_t fact_index;

    if (knowledge_base == 0 || hypothesis_engine == 0 || report == 0)
    {
        return NX_REASONING_INVALID_ARGUMENT;
    }

    for (hypothesis_index = 0; hypothesis_index < hypothesis_engine->hypothesis_count; ++hypothesis_index)
    {
        const NxHypothesis* hypothesis = &hypothesis_engine->hypotheses[hypothesis_index];
        int saw_support = 0;
        int saw_contradiction = 0;

        for (fact_index = 0; fact_index < knowledge_base->fact_count; ++fact_index)
        {
            const NxKnowledgeFact* fact = &knowledge_base->facts[fact_index];

            if (NxReasoning_StringContains(hypothesis->title, fact->subject) != 0 ||
                NxReasoning_StringContains(hypothesis->rationale, fact->subject) != 0)
            {
                if (strcmp(fact->predicate, "supports") == 0 || strcmp(fact->predicate, "improves") == 0)
                {
                    saw_support = 1;
                    if (NxReasoningReport_Add(report,
                            NX_REASONING_CONCLUSION_FACT_SUPPORTS_HYPOTHESIS,
                            hypothesis->id,
                            fact->id,
                            fact->confidence,
                            "Knowledge fact supports hypothesis") != NX_REASONING_OK)
                    {
                        return NX_REASONING_OUT_OF_CAPACITY;
                    }
                }
                else if (strcmp(fact->predicate, "contradicts") == 0 || strcmp(fact->predicate, "regresses") == 0)
                {
                    saw_contradiction = 1;
                    if (NxReasoningReport_Add(report,
                            NX_REASONING_CONCLUSION_FACT_CONTRADICTS_HYPOTHESIS,
                            hypothesis->id,
                            fact->id,
                            fact->confidence,
                            "Knowledge fact contradicts hypothesis") != NX_REASONING_OK)
                    {
                        return NX_REASONING_OUT_OF_CAPACITY;
                    }
                }
            }
        }

        if (saw_support != 0 && saw_contradiction != 0)
        {
            if (NxReasoningReport_Add(report,
                    NX_REASONING_CONCLUSION_CONFLICT_DETECTED,
                    hypothesis->id,
                    0u,
                    hypothesis->confidence,
                    "Hypothesis has both support and contradiction evidence") != NX_REASONING_OK)
            {
                return NX_REASONING_OUT_OF_CAPACITY;
            }
        }
    }

    return NX_REASONING_OK;
}

size_t NxReasoningEngine_CountConflicts(
    const NxReasoningReport* report)
{
    size_t index;
    size_t count = 0;

    if (report == 0)
    {
        return 0;
    }

    for (index = 0; index < report->conclusion_count; ++index)
    {
        if (report->conclusions[index].type == NX_REASONING_CONCLUSION_CONFLICT_DETECTED)
        {
            ++count;
        }
    }

    return count;
}
