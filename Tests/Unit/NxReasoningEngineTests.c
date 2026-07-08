#include "Nexiora/Research/NxHypothesisEngine.h"
#include "Nexiora/Research/NxKnowledgeBase.h"
#include "Nexiora/Research/NxReasoningEngine.h"
#include "Nexiora/Research/NxResearchGraph.h"

#include <stdio.h>

int main(void)
{
    NxResearchGraph graph;
    NxKnowledgeBase knowledge_base;
    NxHypothesisEngine hypotheses;
    NxReasoningConclusion conclusions[8];
    NxReasoningReport report;
    uint32_t node_id;
    uint32_t fact_id;
    uint32_t hypothesis_id;

    if (NxResearchGraph_Init(&graph) != NX_RESEARCH_GRAPH_OK)
    {
        printf("Graph init failed\n");
        return 1;
    }

    if (NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EVIDENCE, "NxVectorEvidence", &node_id) != NX_RESEARCH_GRAPH_OK)
    {
        printf("Node add failed\n");
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (NxKnowledgeBase_Init(&knowledge_base, &graph) != NX_KNOWLEDGE_OK)
    {
        printf("Knowledge init failed\n");
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (NxHypothesisEngine_Init(&hypotheses) != NX_HYPOTHESIS_OK)
    {
        printf("Hypothesis init failed\n");
        NxKnowledgeBase_Shutdown(&knowledge_base);
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    (void)NxHypothesisEngine_Propose(
        &hypotheses,
        "NxVector append optimization",
        "NxVector should improve append throughput",
        70,
        &hypothesis_id);

    (void)NxKnowledgeBase_AddFact(&knowledge_base, "NxVector", "supports", "append optimization", 92, node_id, &fact_id);
    (void)NxKnowledgeBase_AddFact(&knowledge_base, "NxVector", "regresses", "memory pressure", 81, node_id, &fact_id);

    if (NxReasoningReport_Init(&report, conclusions, 8u) != NX_REASONING_OK)
    {
        printf("Report init failed\n");
        NxHypothesisEngine_Shutdown(&hypotheses);
        NxKnowledgeBase_Shutdown(&knowledge_base);
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (NxReasoningEngine_EvaluateHypotheses(&knowledge_base, &hypotheses, &report) != NX_REASONING_OK)
    {
        printf("Reasoning failed\n");
        NxHypothesisEngine_Shutdown(&hypotheses);
        NxKnowledgeBase_Shutdown(&knowledge_base);
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (report.conclusion_count != 3u)
    {
        printf("Expected 3 conclusions, got %u\n", (unsigned)report.conclusion_count);
        NxHypothesisEngine_Shutdown(&hypotheses);
        NxKnowledgeBase_Shutdown(&knowledge_base);
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (NxReasoningEngine_CountConflicts(&report) != 1u)
    {
        printf("Expected conflict\n");
        NxHypothesisEngine_Shutdown(&hypotheses);
        NxKnowledgeBase_Shutdown(&knowledge_base);
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    NxHypothesisEngine_Shutdown(&hypotheses);
    NxKnowledgeBase_Shutdown(&knowledge_base);
    NxResearchGraph_Shutdown(&graph);
    return 0;
}
