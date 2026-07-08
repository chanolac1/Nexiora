#ifndef NEXIORA_RESEARCH_NX_RESEARCH_GRAPH_QUERY_H
#define NEXIORA_RESEARCH_NX_RESEARCH_GRAPH_QUERY_H

#include "Nexiora/Research/NxResearchGraph.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchGraphQueryStatus
{
    NX_RESEARCH_GRAPH_QUERY_OK = 0,
    NX_RESEARCH_GRAPH_QUERY_INVALID_ARGUMENT = 1,
    NX_RESEARCH_GRAPH_QUERY_NOT_FOUND = 2,
    NX_RESEARCH_GRAPH_QUERY_BUFFER_TOO_SMALL = 3
} NxResearchGraphQueryStatus;

typedef struct NxResearchGraphQueryResult
{
    size_t total_matches;
    size_t written_count;
} NxResearchGraphQueryResult;

NxResearchGraphQueryStatus NxResearchGraphQuery_CountNodesByType(
    const NxResearchGraph* graph,
    NxResearchNodeType type,
    size_t* count_out);

NxResearchGraphQueryStatus NxResearchGraphQuery_FindNodesByType(
    const NxResearchGraph* graph,
    NxResearchNodeType type,
    const NxResearchGraphNode** nodes_out,
    size_t nodes_capacity,
    NxResearchGraphQueryResult* result_out);

NxResearchGraphQueryStatus NxResearchGraphQuery_FindOutgoingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type,
    const NxResearchGraphEdge** edges_out,
    size_t edges_capacity,
    NxResearchGraphQueryResult* result_out);

NxResearchGraphQueryStatus NxResearchGraphQuery_FindIncomingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type,
    const NxResearchGraphEdge** edges_out,
    size_t edges_capacity,
    NxResearchGraphQueryResult* result_out);

NxResearchGraphQueryStatus NxResearchGraphQuery_FindConnectedNodes(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType edge_type,
    int outgoing,
    const NxResearchGraphNode** nodes_out,
    size_t nodes_capacity,
    NxResearchGraphQueryResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
