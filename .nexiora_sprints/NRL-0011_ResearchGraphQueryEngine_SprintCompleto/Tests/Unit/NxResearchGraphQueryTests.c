#include "Nexiora/Research/NxResearchGraph.h"
#include "Nexiora/Research/NxResearchGraphQuery.h"

#include <stdio.h>
#include <string.h>

static int Expect(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAILED: %s\n", message);
        return 1;
    }

    return 0;
}

int main(void)
{
    NxResearchGraph graph;
    uint32_t experiment = 0;
    uint32_t evidence_a = 0;
    uint32_t evidence_b = 0;
    uint32_t promotion = 0;
    uint32_t journal = 0;
    uint32_t edge_id = 0;
    size_t count = 0;
    NxResearchGraphQueryResult result;
    const NxResearchGraphNode* nodes[4];
    const NxResearchGraphEdge* edges[4];
    NxResearchGraphQueryStatus query_status;

    if (Expect(NxResearchGraph_Init(&graph) == NX_RESEARCH_GRAPH_OK, "graph init") != 0)
    {
        return 1;
    }

    if (Expect(NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, "vector-growth", &experiment) == NX_RESEARCH_GRAPH_OK, "add experiment") != 0 ||
        Expect(NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EVIDENCE, "evidence-speed", &evidence_a) == NX_RESEARCH_GRAPH_OK, "add evidence a") != 0 ||
        Expect(NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EVIDENCE, "evidence-memory", &evidence_b) == NX_RESEARCH_GRAPH_OK, "add evidence b") != 0 ||
        Expect(NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_PROMOTION_RECOMMENDATION, "promotion-candidate", &promotion) == NX_RESEARCH_GRAPH_OK, "add promotion") != 0 ||
        Expect(NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_JOURNAL_ENTRY, "journal-entry", &journal) == NX_RESEARCH_GRAPH_OK, "add journal") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (Expect(NxResearchGraph_AddEdge(&graph, experiment, evidence_a, NX_RESEARCH_EDGE_PRODUCES, "produces", &edge_id) == NX_RESEARCH_GRAPH_OK, "experiment produces evidence a") != 0 ||
        Expect(NxResearchGraph_AddEdge(&graph, experiment, evidence_b, NX_RESEARCH_EDGE_PRODUCES, "produces", &edge_id) == NX_RESEARCH_GRAPH_OK, "experiment produces evidence b") != 0 ||
        Expect(NxResearchGraph_AddEdge(&graph, evidence_a, promotion, NX_RESEARCH_EDGE_SUPPORTS, "supports", &edge_id) == NX_RESEARCH_GRAPH_OK, "evidence supports promotion") != 0 ||
        Expect(NxResearchGraph_AddEdge(&graph, journal, experiment, NX_RESEARCH_EDGE_RECORDS, "records", &edge_id) == NX_RESEARCH_GRAPH_OK, "journal records experiment") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    if (Expect(NxResearchGraphQuery_CountNodesByType(&graph, NX_RESEARCH_NODE_EVIDENCE, &count) == NX_RESEARCH_GRAPH_QUERY_OK, "count evidence status") != 0 ||
        Expect(count == 2, "count evidence nodes") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindNodesByType(&graph, NX_RESEARCH_NODE_EVIDENCE, nodes, 4, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_OK, "find evidence nodes status") != 0 ||
        Expect(result.total_matches == 2, "find evidence total") != 0 ||
        Expect(result.written_count == 2, "find evidence written") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindOutgoingEdges(&graph, experiment, NX_RESEARCH_EDGE_PRODUCES, edges, 4, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_OK, "find outgoing status") != 0 ||
        Expect(result.total_matches == 2, "find outgoing total") != 0 ||
        Expect(edges[0]->from_node_id == experiment, "outgoing edge from experiment") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindIncomingEdges(&graph, promotion, NX_RESEARCH_EDGE_SUPPORTS, edges, 4, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_OK, "find incoming status") != 0 ||
        Expect(result.total_matches == 1, "find incoming total") != 0 ||
        Expect(edges[0]->to_node_id == promotion, "incoming edge to promotion") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindConnectedNodes(&graph, experiment, NX_RESEARCH_EDGE_PRODUCES, 1, nodes, 4, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_OK, "connected outgoing status") != 0 ||
        Expect(result.total_matches == 2, "connected outgoing total") != 0 ||
        Expect(nodes[0]->type == NX_RESEARCH_NODE_EVIDENCE, "connected node type") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindConnectedNodes(&graph, promotion, NX_RESEARCH_EDGE_SUPPORTS, 0, nodes, 4, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_OK, "connected incoming status") != 0 ||
        Expect(result.total_matches == 1, "connected incoming total") != 0 ||
        Expect(strcmp(nodes[0]->name, "evidence-speed") == 0, "connected incoming name") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindOutgoingEdges(&graph, experiment, NX_RESEARCH_EDGE_PRODUCES, edges, 1, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_BUFFER_TOO_SMALL, "buffer too small status") != 0 ||
        Expect(result.total_matches == 2, "buffer too small total") != 0 ||
        Expect(result.written_count == 1, "buffer too small written") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    query_status = NxResearchGraphQuery_FindOutgoingEdges(&graph, 9999u, NX_RESEARCH_EDGE_UNKNOWN, edges, 4, &result);
    if (Expect(query_status == NX_RESEARCH_GRAPH_QUERY_NOT_FOUND, "missing node status") != 0)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    NxResearchGraph_Shutdown(&graph);
    return 0;
}
