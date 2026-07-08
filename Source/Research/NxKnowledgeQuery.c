#include "Nexiora/Research/NxKnowledgeQuery.h"

#include <string.h>

static int NxKnowledgeQuery_TextEquals(const char* left, const char* right)
{
    if (left == 0 || right == 0)
    {
        return 0;
    }

    return strcmp(left, right) == 0;
}

static NxKnowledgeStatus NxKnowledgeQuery_Add(
    NxKnowledgeQueryResult* result,
    const NxKnowledgeFact* fact)
{
    if (result == 0 || fact == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    if (result->count >= result->capacity)
    {
        return NX_KNOWLEDGE_OUT_OF_MEMORY;
    }

    result->facts[result->count] = fact;
    result->count++;

    return NX_KNOWLEDGE_OK;
}

NxKnowledgeStatus NxKnowledgeQueryResult_Init(
    NxKnowledgeQueryResult* result,
    const NxKnowledgeFact** buffer,
    size_t capacity)
{
    if (result == 0 || buffer == 0 || capacity == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    result->facts = buffer;
    result->count = 0;
    result->capacity = capacity;

    return NX_KNOWLEDGE_OK;
}

void NxKnowledgeQueryResult_Clear(NxKnowledgeQueryResult* result)
{
    if (result == 0)
    {
        return;
    }

    result->count = 0;
}

NxKnowledgeStatus NxKnowledgeQuery_BySubject(
    const NxKnowledgeBase* knowledge_base,
    const char* subject,
    NxKnowledgeQueryResult* result)
{
    size_t index = 0;
    NxKnowledgeStatus status = NX_KNOWLEDGE_OK;

    if (knowledge_base == 0 || subject == 0 || result == 0 || result->facts == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    NxKnowledgeQueryResult_Clear(result);

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (NxKnowledgeQuery_TextEquals(knowledge_base->facts[index].subject, subject))
        {
            status = NxKnowledgeQuery_Add(result, &knowledge_base->facts[index]);
            if (status != NX_KNOWLEDGE_OK)
            {
                return status;
            }
        }
    }

    return NX_KNOWLEDGE_OK;
}

NxKnowledgeStatus NxKnowledgeQuery_ByPredicate(
    const NxKnowledgeBase* knowledge_base,
    const char* predicate,
    NxKnowledgeQueryResult* result)
{
    size_t index = 0;
    NxKnowledgeStatus status = NX_KNOWLEDGE_OK;

    if (knowledge_base == 0 || predicate == 0 || result == 0 || result->facts == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    NxKnowledgeQueryResult_Clear(result);

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (NxKnowledgeQuery_TextEquals(knowledge_base->facts[index].predicate, predicate))
        {
            status = NxKnowledgeQuery_Add(result, &knowledge_base->facts[index]);
            if (status != NX_KNOWLEDGE_OK)
            {
                return status;
            }
        }
    }

    return NX_KNOWLEDGE_OK;
}

NxKnowledgeStatus NxKnowledgeQuery_ByMinimumConfidence(
    const NxKnowledgeBase* knowledge_base,
    int minimum_confidence,
    NxKnowledgeQueryResult* result)
{
    size_t index = 0;
    NxKnowledgeStatus status = NX_KNOWLEDGE_OK;

    if (knowledge_base == 0 || result == 0 || result->facts == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    if (minimum_confidence < 0 || minimum_confidence > 100)
    {
        return NX_KNOWLEDGE_INVALID_CONFIDENCE;
    }

    NxKnowledgeQueryResult_Clear(result);

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (knowledge_base->facts[index].confidence >= minimum_confidence)
        {
            status = NxKnowledgeQuery_Add(result, &knowledge_base->facts[index]);
            if (status != NX_KNOWLEDGE_OK)
            {
                return status;
            }
        }
    }

    return NX_KNOWLEDGE_OK;
}
