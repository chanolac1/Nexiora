#ifndef NEXIORA_RESEARCH_NX_KNOWLEDGE_QUERY_H
#define NEXIORA_RESEARCH_NX_KNOWLEDGE_QUERY_H

#include "Nexiora/Research/NxKnowledgeBase.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxKnowledgeQueryResult
{
    const NxKnowledgeFact** facts;
    size_t count;
    size_t capacity;
} NxKnowledgeQueryResult;

NxKnowledgeStatus NxKnowledgeQueryResult_Init(
    NxKnowledgeQueryResult* result,
    const NxKnowledgeFact** buffer,
    size_t capacity);

void NxKnowledgeQueryResult_Clear(NxKnowledgeQueryResult* result);

NxKnowledgeStatus NxKnowledgeQuery_BySubject(
    const NxKnowledgeBase* knowledge_base,
    const char* subject,
    NxKnowledgeQueryResult* result);

NxKnowledgeStatus NxKnowledgeQuery_ByPredicate(
    const NxKnowledgeBase* knowledge_base,
    const char* predicate,
    NxKnowledgeQueryResult* result);

NxKnowledgeStatus NxKnowledgeQuery_ByMinimumConfidence(
    const NxKnowledgeBase* knowledge_base,
    int minimum_confidence,
    NxKnowledgeQueryResult* result);

#ifdef __cplusplus
}
#endif

#endif
