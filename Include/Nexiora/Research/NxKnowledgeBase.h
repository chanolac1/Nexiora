#ifndef NEXIORA_RESEARCH_NX_KNOWLEDGE_BASE_H
#define NEXIORA_RESEARCH_NX_KNOWLEDGE_BASE_H

#include "Nexiora/Research/NxResearchGraph.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxKnowledgeStatus
{
    NX_KNOWLEDGE_OK = 0,
    NX_KNOWLEDGE_INVALID_ARGUMENT = 1,
    NX_KNOWLEDGE_OUT_OF_MEMORY = 2,
    NX_KNOWLEDGE_NOT_FOUND = 3,
    NX_KNOWLEDGE_DUPLICATE_FACT = 4,
    NX_KNOWLEDGE_INVALID_CONFIDENCE = 5
} NxKnowledgeStatus;

typedef struct NxKnowledgeFact
{
    uint32_t id;
    char subject[64];
    char predicate[64];
    char object[64];
    int confidence;
    uint32_t source_node_id;
} NxKnowledgeFact;

typedef struct NxKnowledgeBase
{
    NxKnowledgeFact* facts;
    size_t fact_count;
    size_t fact_capacity;
    uint32_t next_fact_id;
    const NxResearchGraph* graph;
} NxKnowledgeBase;

NxKnowledgeStatus NxKnowledgeBase_Init(NxKnowledgeBase* knowledge_base, const NxResearchGraph* graph);
void NxKnowledgeBase_Shutdown(NxKnowledgeBase* knowledge_base);
void NxKnowledgeBase_Clear(NxKnowledgeBase* knowledge_base);

NxKnowledgeStatus NxKnowledgeBase_AddFact(
    NxKnowledgeBase* knowledge_base,
    const char* subject,
    const char* predicate,
    const char* object,
    int confidence,
    uint32_t source_node_id,
    uint32_t* fact_id_out);

size_t NxKnowledgeBase_GetFactCount(const NxKnowledgeBase* knowledge_base);

const NxKnowledgeFact* NxKnowledgeBase_FindFactById(
    const NxKnowledgeBase* knowledge_base,
    uint32_t fact_id);

const NxKnowledgeFact* NxKnowledgeBase_FindFact(
    const NxKnowledgeBase* knowledge_base,
    const char* subject,
    const char* predicate,
    const char* object);

size_t NxKnowledgeBase_CountFactsBySubject(
    const NxKnowledgeBase* knowledge_base,
    const char* subject);

size_t NxKnowledgeBase_CountFactsByPredicate(
    const NxKnowledgeBase* knowledge_base,
    const char* predicate);

size_t NxKnowledgeBase_CountFactsBySourceNode(
    const NxKnowledgeBase* knowledge_base,
    uint32_t source_node_id);

#ifdef __cplusplus
}
#endif

#endif
