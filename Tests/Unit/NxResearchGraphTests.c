#include "Nexiora/Research/NxResearchGraph.h"

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

static int TestGraphCreatesKnowledgeChain(void)
{
    NxResearchGraph graph;
    NxResearchGraphStatus status;
    uint32_t experiment_id = 0;
    uint32_t evidence_id = 0;
    uint32_t promotion_id = 0;
    uint32_t edge_id = 0;
    const NxResearchGraphNode* node = 0;
    const NxResearchGraphEdge* edge = 0;

    status = NxResearchGraph_Init(&graph);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "graph init should succeed")) return 1;

    status = NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, "SIMD_Vector_Experiment", &experiment_id);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "experiment node should be added")) return 1;

    status = NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EVIDENCE, "SIMD_Vector_Evidence", &evidence_id);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "evidence node should be added")) return 1;

    status = NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_PROMOTION_RECOMMENDATION, "SIMD_Vector_Promotion", &promotion_id);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "promotion node should be added")) return 1;

    status = NxResearchGraph_AddEdge(&graph, experiment_id, evidence_id, NX_RESEARCH_EDGE_PRODUCES, "produces", &edge_id);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "experiment should produce evidence")) return 1;

    status = NxResearchGraph_AddEdge(&graph, evidence_id, promotion_id, NX_RESEARCH_EDGE_SUPPORTS, "supports", &edge_id);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "evidence should support promotion")) return 1;

    if (Expect(NxResearchGraph_GetNodeCount(&graph) == 3, "graph should contain 3 nodes")) return 1;
    if (Expect(NxResearchGraph_GetEdgeCount(&graph) == 2, "graph should contain 2 edges")) return 1;

    node = NxResearchGraph_FindNodeByName(&graph, "SIMD_Vector_Evidence");
    if (Expect(node != 0, "find node by name should succeed")) return 1;
    if (Expect(node->id == evidence_id, "found evidence id should match")) return 1;
    if (Expect(node->type == NX_RESEARCH_NODE_EVIDENCE, "found node should be evidence")) return 1;

    edge = NxResearchGraph_FindEdgeById(&graph, 1);
    if (Expect(edge != 0, "find edge by id should succeed")) return 1;
    if (Expect(edge->from_node_id == experiment_id, "first edge source should be experiment")) return 1;
    if (Expect(edge->to_node_id == evidence_id, "first edge target should be evidence")) return 1;

    if (Expect(NxResearchGraph_CountOutgoingEdges(&graph, experiment_id, NX_RESEARCH_EDGE_PRODUCES) == 1,
        "experiment should have one produces edge")) return 1;
    if (Expect(NxResearchGraph_CountIncomingEdges(&graph, promotion_id, NX_RESEARCH_EDGE_SUPPORTS) == 1,
        "promotion should have one supporting evidence edge")) return 1;

    NxResearchGraph_Shutdown(&graph);
    return 0;
}

static int TestGraphRejectsInvalidEdges(void)
{
    NxResearchGraph graph;
    NxResearchGraphStatus status;
    uint32_t experiment_id = 0;
    uint32_t edge_id = 0;

    status = NxResearchGraph_Init(&graph);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "graph init should succeed")) return 1;

    status = NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, "Experiment", &experiment_id);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "experiment node should be added")) return 1;

    status = NxResearchGraph_AddEdge(&graph, experiment_id, 9999, NX_RESEARCH_EDGE_PRODUCES, "invalid", &edge_id);
    if (Expect(status == NX_RESEARCH_GRAPH_EDGE_ENDPOINT_NOT_FOUND, "invalid edge endpoint should fail")) return 1;
    if (Expect(NxResearchGraph_GetEdgeCount(&graph) == 0, "invalid edge should not be inserted")) return 1;

    NxResearchGraph_Shutdown(&graph);
    return 0;
}

static int TestGraphGrowsBeyondInitialCapacity(void)
{
    NxResearchGraph graph;
    NxResearchGraphStatus status;
    uint32_t ids[64];
    size_t index = 0;

    status = NxResearchGraph_Init(&graph);
    if (Expect(status == NX_RESEARCH_GRAPH_OK, "graph init should succeed")) return 1;

    for (index = 0; index < 64; index++)
    {
        char name[64];
        sprintf(name, "Experiment_%u", (unsigned int)index);
        status = NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, name, &ids[index]);
        if (Expect(status == NX_RESEARCH_GRAPH_OK, "node insertion should grow capacity")) return 1;
    }

    for (index = 1; index < 64; index++)
    {
        uint32_t edge_id = 0;
        status = NxResearchGraph_AddEdge(&graph, ids[index - 1], ids[index], NX_RESEARCH_EDGE_RELATES_TO, "next", &edge_id);
        if (Expect(status == NX_RESEARCH_GRAPH_OK, "edge insertion should grow capacity")) return 1;
    }

    if (Expect(NxResearchGraph_GetNodeCount(&graph) == 64, "graph should contain 64 nodes")) return 1;
    if (Expect(NxResearchGraph_GetEdgeCount(&graph) == 63, "graph should contain 63 edges")) return 1;

    NxResearchGraph_Shutdown(&graph);
    return 0;
}

int main(void)
{
    if (TestGraphCreatesKnowledgeChain() != 0) return 1;
    if (TestGraphRejectsInvalidEdges() != 0) return 1;
    if (TestGraphGrowsBeyondInitialCapacity() != 0) return 1;

    printf("NxResearchGraphTests passed\n");
    return 0;
}
