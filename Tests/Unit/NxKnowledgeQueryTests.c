#include "Nexiora/Research/NxKnowledgeQuery.h"

#include <stdio.h>
#include <string.h>

static int Expect(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        return 1;
    }

    return 0;
}

static int SeedKnowledge(NxKnowledgeBase* knowledge_base)
{
    NxKnowledgeStatus status;
    uint32_t fact_id = 0;

    status = NxKnowledgeBase_AddFact(knowledge_base, "NxVector", "produces", "EvidenceA", 94, 0, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "seed fact 1 should be inserted")) return 1;
    status = NxKnowledgeBase_AddFact(knowledge_base, "NxVector", "supports", "PromotionA", 91, 0, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "seed fact 2 should be inserted")) return 1;
    status = NxKnowledgeBase_AddFact(knowledge_base, "NxString", "produces", "EvidenceB", 72, 0, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "seed fact 3 should be inserted")) return 1;

    return 0;
}

static int TestQueryBySubject(void)
{
    NxKnowledgeBase knowledge_base;
    const NxKnowledgeFact* buffer[8];
    NxKnowledgeQueryResult result;
    NxKnowledgeStatus status;

    status = NxKnowledgeBase_Init(&knowledge_base, 0);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;
    if (SeedKnowledge(&knowledge_base) != 0) return 1;

    status = NxKnowledgeQueryResult_Init(&result, buffer, 8);
    if (Expect(status == NX_KNOWLEDGE_OK, "query result init should succeed")) return 1;

    status = NxKnowledgeQuery_BySubject(&knowledge_base, "NxVector", &result);
    if (Expect(status == NX_KNOWLEDGE_OK, "query by subject should succeed")) return 1;
    if (Expect(result.count == 2, "query by subject should return two facts")) return 1;
    if (Expect(strcmp(result.facts[0]->subject, "NxVector") == 0, "first subject should match")) return 1;
    if (Expect(strcmp(result.facts[1]->subject, "NxVector") == 0, "second subject should match")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    return 0;
}

static int TestQueryByPredicate(void)
{
    NxKnowledgeBase knowledge_base;
    const NxKnowledgeFact* buffer[8];
    NxKnowledgeQueryResult result;
    NxKnowledgeStatus status;

    status = NxKnowledgeBase_Init(&knowledge_base, 0);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;
    if (SeedKnowledge(&knowledge_base) != 0) return 1;

    status = NxKnowledgeQueryResult_Init(&result, buffer, 8);
    if (Expect(status == NX_KNOWLEDGE_OK, "query result init should succeed")) return 1;

    status = NxKnowledgeQuery_ByPredicate(&knowledge_base, "produces", &result);
    if (Expect(status == NX_KNOWLEDGE_OK, "query by predicate should succeed")) return 1;
    if (Expect(result.count == 2, "query by predicate should return two facts")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    return 0;
}

static int TestQueryByMinimumConfidence(void)
{
    NxKnowledgeBase knowledge_base;
    const NxKnowledgeFact* buffer[8];
    NxKnowledgeQueryResult result;
    NxKnowledgeStatus status;

    status = NxKnowledgeBase_Init(&knowledge_base, 0);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;
    if (SeedKnowledge(&knowledge_base) != 0) return 1;

    status = NxKnowledgeQueryResult_Init(&result, buffer, 8);
    if (Expect(status == NX_KNOWLEDGE_OK, "query result init should succeed")) return 1;

    status = NxKnowledgeQuery_ByMinimumConfidence(&knowledge_base, 90, &result);
    if (Expect(status == NX_KNOWLEDGE_OK, "confidence query should succeed")) return 1;
    if (Expect(result.count == 2, "confidence query should return two facts")) return 1;

    status = NxKnowledgeQuery_ByMinimumConfidence(&knowledge_base, -1, &result);
    if (Expect(status == NX_KNOWLEDGE_INVALID_CONFIDENCE, "invalid confidence should fail")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    return 0;
}

static int TestQueryDetectsSmallBuffer(void)
{
    NxKnowledgeBase knowledge_base;
    const NxKnowledgeFact* buffer[1];
    NxKnowledgeQueryResult result;
    NxKnowledgeStatus status;

    status = NxKnowledgeBase_Init(&knowledge_base, 0);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;
    if (SeedKnowledge(&knowledge_base) != 0) return 1;

    status = NxKnowledgeQueryResult_Init(&result, buffer, 1);
    if (Expect(status == NX_KNOWLEDGE_OK, "query result init should succeed")) return 1;

    status = NxKnowledgeQuery_BySubject(&knowledge_base, "NxVector", &result);
    if (Expect(status == NX_KNOWLEDGE_OUT_OF_MEMORY, "small result buffer should fail predictably")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    return 0;
}

int main(void)
{
    if (TestQueryBySubject() != 0) return 1;
    if (TestQueryByPredicate() != 0) return 1;
    if (TestQueryByMinimumConfidence() != 0) return 1;
    if (TestQueryDetectsSmallBuffer() != 0) return 1;

    printf("NxKnowledgeQueryTests passed\n");
    return 0;
}
