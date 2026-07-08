#include "Nexiora/Research/NxKnowledgeBase.h"

#include <stdlib.h>
#include <string.h>

#define NX_KNOWLEDGE_INITIAL_FACT_CAPACITY ((size_t)16)

static void NxKnowledge_CopyText(char* destination, size_t destination_size, const char* source)
{
    size_t index = 0;

    if (destination == 0 || destination_size == 0)
    {
        return;
    }

    if (source == 0)
    {
        destination[0] = '\0';
        return;
    }

    while (index + 1 < destination_size && source[index] != '\0')
    {
        destination[index] = source[index];
        index++;
    }

    destination[index] = '\0';
}

static int NxKnowledge_TextEquals(const char* left, const char* right)
{
    if (left == 0 || right == 0)
    {
        return 0;
    }

    return strcmp(left, right) == 0;
}

static NxKnowledgeStatus NxKnowledgeBase_EnsureFactCapacity(NxKnowledgeBase* knowledge_base, size_t required_capacity)
{
    size_t new_capacity = 0;
    NxKnowledgeFact* new_facts = 0;

    if (knowledge_base == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    if (knowledge_base->fact_capacity >= required_capacity)
    {
        return NX_KNOWLEDGE_OK;
    }

    new_capacity = knowledge_base->fact_capacity == 0 ? NX_KNOWLEDGE_INITIAL_FACT_CAPACITY : knowledge_base->fact_capacity;
    while (new_capacity < required_capacity)
    {
        new_capacity *= 2;
    }

    new_facts = (NxKnowledgeFact*)realloc(knowledge_base->facts, new_capacity * sizeof(NxKnowledgeFact));
    if (new_facts == 0)
    {
        return NX_KNOWLEDGE_OUT_OF_MEMORY;
    }

    knowledge_base->facts = new_facts;
    knowledge_base->fact_capacity = new_capacity;
    return NX_KNOWLEDGE_OK;
}

NxKnowledgeStatus NxKnowledgeBase_Init(NxKnowledgeBase* knowledge_base, const NxResearchGraph* graph)
{
    if (knowledge_base == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    knowledge_base->facts = 0;
    knowledge_base->fact_count = 0;
    knowledge_base->fact_capacity = 0;
    knowledge_base->next_fact_id = 1;
    knowledge_base->graph = graph;

    return NX_KNOWLEDGE_OK;
}

void NxKnowledgeBase_Shutdown(NxKnowledgeBase* knowledge_base)
{
    if (knowledge_base == 0)
    {
        return;
    }

    free(knowledge_base->facts);
    knowledge_base->facts = 0;
    knowledge_base->fact_count = 0;
    knowledge_base->fact_capacity = 0;
    knowledge_base->next_fact_id = 1;
    knowledge_base->graph = 0;
}

void NxKnowledgeBase_Clear(NxKnowledgeBase* knowledge_base)
{
    if (knowledge_base == 0)
    {
        return;
    }

    knowledge_base->fact_count = 0;
    knowledge_base->next_fact_id = 1;
}

NxKnowledgeStatus NxKnowledgeBase_AddFact(
    NxKnowledgeBase* knowledge_base,
    const char* subject,
    const char* predicate,
    const char* object,
    int confidence,
    uint32_t source_node_id,
    uint32_t* fact_id_out)
{
    NxKnowledgeStatus status = NX_KNOWLEDGE_OK;
    NxKnowledgeFact* fact = 0;

    if (knowledge_base == 0 || subject == 0 || predicate == 0 || object == 0 || fact_id_out == 0)
    {
        return NX_KNOWLEDGE_INVALID_ARGUMENT;
    }

    if (confidence < 0 || confidence > 100)
    {
        return NX_KNOWLEDGE_INVALID_CONFIDENCE;
    }

    if (knowledge_base->graph != 0 && source_node_id != 0 &&
        NxResearchGraph_FindNodeById(knowledge_base->graph, source_node_id) == 0)
    {
        return NX_KNOWLEDGE_NOT_FOUND;
    }

    if (NxKnowledgeBase_FindFact(knowledge_base, subject, predicate, object) != 0)
    {
        return NX_KNOWLEDGE_DUPLICATE_FACT;
    }

    status = NxKnowledgeBase_EnsureFactCapacity(knowledge_base, knowledge_base->fact_count + 1);
    if (status != NX_KNOWLEDGE_OK)
    {
        return status;
    }

    fact = &knowledge_base->facts[knowledge_base->fact_count];
    fact->id = knowledge_base->next_fact_id++;
    NxKnowledge_CopyText(fact->subject, sizeof(fact->subject), subject);
    NxKnowledge_CopyText(fact->predicate, sizeof(fact->predicate), predicate);
    NxKnowledge_CopyText(fact->object, sizeof(fact->object), object);
    fact->confidence = confidence;
    fact->source_node_id = source_node_id;

    knowledge_base->fact_count++;
    *fact_id_out = fact->id;

    return NX_KNOWLEDGE_OK;
}

size_t NxKnowledgeBase_GetFactCount(const NxKnowledgeBase* knowledge_base)
{
    if (knowledge_base == 0)
    {
        return 0;
    }

    return knowledge_base->fact_count;
}

const NxKnowledgeFact* NxKnowledgeBase_FindFactById(
    const NxKnowledgeBase* knowledge_base,
    uint32_t fact_id)
{
    size_t index = 0;

    if (knowledge_base == 0)
    {
        return 0;
    }

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (knowledge_base->facts[index].id == fact_id)
        {
            return &knowledge_base->facts[index];
        }
    }

    return 0;
}

const NxKnowledgeFact* NxKnowledgeBase_FindFact(
    const NxKnowledgeBase* knowledge_base,
    const char* subject,
    const char* predicate,
    const char* object)
{
    size_t index = 0;

    if (knowledge_base == 0 || subject == 0 || predicate == 0 || object == 0)
    {
        return 0;
    }

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        const NxKnowledgeFact* fact = &knowledge_base->facts[index];
        if (NxKnowledge_TextEquals(fact->subject, subject) &&
            NxKnowledge_TextEquals(fact->predicate, predicate) &&
            NxKnowledge_TextEquals(fact->object, object))
        {
            return fact;
        }
    }

    return 0;
}

size_t NxKnowledgeBase_CountFactsBySubject(
    const NxKnowledgeBase* knowledge_base,
    const char* subject)
{
    size_t index = 0;
    size_t count = 0;

    if (knowledge_base == 0 || subject == 0)
    {
        return 0;
    }

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (NxKnowledge_TextEquals(knowledge_base->facts[index].subject, subject))
        {
            count++;
        }
    }

    return count;
}

size_t NxKnowledgeBase_CountFactsByPredicate(
    const NxKnowledgeBase* knowledge_base,
    const char* predicate)
{
    size_t index = 0;
    size_t count = 0;

    if (knowledge_base == 0 || predicate == 0)
    {
        return 0;
    }

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (NxKnowledge_TextEquals(knowledge_base->facts[index].predicate, predicate))
        {
            count++;
        }
    }

    return count;
}

size_t NxKnowledgeBase_CountFactsBySourceNode(
    const NxKnowledgeBase* knowledge_base,
    uint32_t source_node_id)
{
    size_t index = 0;
    size_t count = 0;

    if (knowledge_base == 0)
    {
        return 0;
    }

    for (index = 0; index < knowledge_base->fact_count; index++)
    {
        if (knowledge_base->facts[index].source_node_id == source_node_id)
        {
            count++;
        }
    }

    return count;
}
