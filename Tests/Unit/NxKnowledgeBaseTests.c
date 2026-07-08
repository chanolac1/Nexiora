#include "Nexiora/Research/NxKnowledgeBase.h"

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

static int TestKnowledgeStoresFactsWithEvidenceSource(void)
{
    NxResearchGraph graph;
    NxKnowledgeBase knowledge_base;
    NxResearchGraphStatus graph_status;
    NxKnowledgeStatus knowledge_status;
    uint32_t evidence_node_id = 0;
    uint32_t fact_id = 0;
    const NxKnowledgeFact* fact = 0;

    graph_status = NxResearchGraph_Init(&graph);
    if (Expect(graph_status == NX_RESEARCH_GRAPH_OK, "graph init should succeed")) return 1;

    graph_status = NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EVIDENCE, "Vector evidence", &evidence_node_id);
    if (Expect(graph_status == NX_RESEARCH_GRAPH_OK, "evidence node should be added")) return 1;

    knowledge_status = NxKnowledgeBase_Init(&knowledge_base, &graph);
    if (Expect(knowledge_status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;

    knowledge_status = NxKnowledgeBase_AddFact(
        &knowledge_base,
        "NxVector",
        "improves_through",
        "SIMD candidate",
        94,
        evidence_node_id,
        &fact_id);
    if (Expect(knowledge_status == NX_KNOWLEDGE_OK, "fact should be added")) return 1;

    if (Expect(NxKnowledgeBase_GetFactCount(&knowledge_base) == 1, "knowledge base should contain one fact")) return 1;

    fact = NxKnowledgeBase_FindFactById(&knowledge_base, fact_id);
    if (Expect(fact != 0, "fact should be found by id")) return 1;
    if (Expect(strcmp(fact->subject, "NxVector") == 0, "subject should match")) return 1;
    if (Expect(strcmp(fact->predicate, "improves_through") == 0, "predicate should match")) return 1;
    if (Expect(strcmp(fact->object, "SIMD candidate") == 0, "object should match")) return 1;
    if (Expect(fact->confidence == 94, "confidence should match")) return 1;
    if (Expect(fact->source_node_id == evidence_node_id, "source node should match")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    NxResearchGraph_Shutdown(&graph);
    return 0;
}

static int TestKnowledgeRejectsDuplicateFacts(void)
{
    NxKnowledgeBase knowledge_base;
    NxKnowledgeStatus status;
    uint32_t fact_id = 0;

    status = NxKnowledgeBase_Init(&knowledge_base, 0);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;

    status = NxKnowledgeBase_AddFact(&knowledge_base, "A", "relates_to", "B", 80, 0, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "first fact should be inserted")) return 1;

    status = NxKnowledgeBase_AddFact(&knowledge_base, "A", "relates_to", "B", 80, 0, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_DUPLICATE_FACT, "duplicate fact should be rejected")) return 1;

    if (Expect(NxKnowledgeBase_GetFactCount(&knowledge_base) == 1, "duplicate should not increase count")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    return 0;
}

static int TestKnowledgeCountsFacts(void)
{
    NxKnowledgeBase knowledge_base;
    NxKnowledgeStatus status;
    uint32_t fact_id = 0;

    status = NxKnowledgeBase_Init(&knowledge_base, 0);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;

    status = NxKnowledgeBase_AddFact(&knowledge_base, "ExperimentA", "produces", "EvidenceA", 90, 7, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "first fact should be inserted")) return 1;
    status = NxKnowledgeBase_AddFact(&knowledge_base, "ExperimentA", "supports", "PromotionA", 92, 7, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "second fact should be inserted")) return 1;
    status = NxKnowledgeBase_AddFact(&knowledge_base, "ExperimentB", "produces", "EvidenceB", 75, 8, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_OK, "third fact should be inserted")) return 1;

    if (Expect(NxKnowledgeBase_CountFactsBySubject(&knowledge_base, "ExperimentA") == 2, "subject count should match")) return 1;
    if (Expect(NxKnowledgeBase_CountFactsByPredicate(&knowledge_base, "produces") == 2, "predicate count should match")) return 1;
    if (Expect(NxKnowledgeBase_CountFactsBySourceNode(&knowledge_base, 7) == 2, "source node count should match")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    return 0;
}

static int TestKnowledgeValidatesConfidenceAndGraphSource(void)
{
    NxResearchGraph graph;
    NxKnowledgeBase knowledge_base;
    NxResearchGraphStatus graph_status;
    NxKnowledgeStatus status;
    uint32_t fact_id = 0;

    graph_status = NxResearchGraph_Init(&graph);
    if (Expect(graph_status == NX_RESEARCH_GRAPH_OK, "graph init should succeed")) return 1;

    status = NxKnowledgeBase_Init(&knowledge_base, &graph);
    if (Expect(status == NX_KNOWLEDGE_OK, "knowledge base init should succeed")) return 1;

    status = NxKnowledgeBase_AddFact(&knowledge_base, "A", "B", "C", 101, 0, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_INVALID_CONFIDENCE, "confidence above 100 should fail")) return 1;

    status = NxKnowledgeBase_AddFact(&knowledge_base, "A", "B", "C", 90, 999, &fact_id);
    if (Expect(status == NX_KNOWLEDGE_NOT_FOUND, "unknown graph source should fail")) return 1;

    NxKnowledgeBase_Shutdown(&knowledge_base);
    NxResearchGraph_Shutdown(&graph);
    return 0;
}

int main(void)
{
    if (TestKnowledgeStoresFactsWithEvidenceSource() != 0) return 1;
    if (TestKnowledgeRejectsDuplicateFacts() != 0) return 1;
    if (TestKnowledgeCountsFacts() != 0) return 1;
    if (TestKnowledgeValidatesConfidenceAndGraphSource() != 0) return 1;

    printf("NxKnowledgeBaseTests passed\n");
    return 0;
}
