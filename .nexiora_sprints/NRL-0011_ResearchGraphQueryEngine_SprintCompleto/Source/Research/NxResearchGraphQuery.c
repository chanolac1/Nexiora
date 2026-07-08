#include "Nexiora/Research/NxResearchGraphQuery.h"

static int NxResearchGraphQuery_EdgeTypeMatches(NxResearchEdgeType actual, NxResearchEdgeType expected)
{
    return expected == NX_RESEARCH_EDGE_UNKNOWN || actual == expected;
}

static int NxResearchGraphQuery_NodeTypeMatches(NxResearchNodeType actual, NxResearchNodeType expected)
{
    return expected == NX_RESEARCH_NODE_UNKNOWN || actual == expected;
}

static void NxResearchGraphQuery_ResetResult(NxResearchGraphQueryResult* result)
{
    if (result != 0)
    {
        result->total_matches = 0;
        result->written_count = 0;
    }
}

NxResearchGraphQueryStatus NxResearchGraphQuery_CountNodesByType(
    const NxResearchGraph* graph,
    NxResearchNodeType type,
    size_t* count_out)
{
    size_t index = 0;
    size_t count = 0;

    if (graph == 0 || count_out == 0)
    {
        return NX_RESEARCH_GRAPH_QUERY_INVALID_ARGUMENT;
    }

    for (index = 0; index < graph->node_count; index++)
    {
        if (NxResearchGraphQuery_NodeTypeMatches(graph->nodes[index].type, type))
        {
            count++;
        }
    }

    *count_out = count;
    return NX_RESEARCH_GRAPH_QUERY_OK;
}

NxResearchGraphQueryStatus NxResearchGraphQuery_FindNodesByType(
    const NxResearchGraph* graph,
    NxResearchNodeType type,
    const NxResearchGraphNode** nodes_out,
    size_t nodes_capacity,
    NxResearchGraphQueryResult* result_out)
{
    size_t index = 0;
    NxResearchGraphQueryStatus status = NX_RESEARCH_GRAPH_QUERY_OK;

    if (graph == 0 || result_out == 0 || (nodes_capacity > 0 && nodes_out == 0))
    {
        return NX_RESEARCH_GRAPH_QUERY_INVALID_ARGUMENT;
    }

    NxResearchGraphQuery_ResetResult(result_out);

    for (index = 0; index < graph->node_count; index++)
    {
        if (NxResearchGraphQuery_NodeTypeMatches(graph->nodes[index].type, type))
        {
            if (result_out->written_count < nodes_capacity)
            {
                nodes_out[result_out->written_count] = &graph->nodes[index];
                result_out->written_count++;
            }
            else
            {
                status = NX_RESEARCH_GRAPH_QUERY_BUFFER_TOO_SMALL;
            }

            result_out->total_matches++;
        }
    }

    return status;
}

NxResearchGraphQueryStatus NxResearchGraphQuery_FindOutgoingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type,
    const NxResearchGraphEdge** edges_out,
    size_t edges_capacity,
    NxResearchGraphQueryResult* result_out)
{
    size_t index = 0;
    NxResearchGraphQueryStatus status = NX_RESEARCH_GRAPH_QUERY_OK;

    if (graph == 0 || result_out == 0 || (edges_capacity > 0 && edges_out == 0))
    {
        return NX_RESEARCH_GRAPH_QUERY_INVALID_ARGUMENT;
    }

    if (NxResearchGraph_FindNodeById(graph, node_id) == 0)
    {
        NxResearchGraphQuery_ResetResult(result_out);
        return NX_RESEARCH_GRAPH_QUERY_NOT_FOUND;
    }

    NxResearchGraphQuery_ResetResult(result_out);

    for (index = 0; index < graph->edge_count; index++)
    {
        if (graph->edges[index].from_node_id == node_id &&
            NxResearchGraphQuery_EdgeTypeMatches(graph->edges[index].type, type))
        {
            if (result_out->written_count < edges_capacity)
            {
                edges_out[result_out->written_count] = &graph->edges[index];
                result_out->written_count++;
            }
            else
            {
                status = NX_RESEARCH_GRAPH_QUERY_BUFFER_TOO_SMALL;
            }

            result_out->total_matches++;
        }
    }

    return status;
}

NxResearchGraphQueryStatus NxResearchGraphQuery_FindIncomingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type,
    const NxResearchGraphEdge** edges_out,
    size_t edges_capacity,
    NxResearchGraphQueryResult* result_out)
{
    size_t index = 0;
    NxResearchGraphQueryStatus status = NX_RESEARCH_GRAPH_QUERY_OK;

    if (graph == 0 || result_out == 0 || (edges_capacity > 0 && edges_out == 0))
    {
        return NX_RESEARCH_GRAPH_QUERY_INVALID_ARGUMENT;
    }

    if (NxResearchGraph_FindNodeById(graph, node_id) == 0)
    {
        NxResearchGraphQuery_ResetResult(result_out);
        return NX_RESEARCH_GRAPH_QUERY_NOT_FOUND;
    }

    NxResearchGraphQuery_ResetResult(result_out);

    for (index = 0; index < graph->edge_count; index++)
    {
        if (graph->edges[index].to_node_id == node_id &&
            NxResearchGraphQuery_EdgeTypeMatches(graph->edges[index].type, type))
        {
            if (result_out->written_count < edges_capacity)
            {
                edges_out[result_out->written_count] = &graph->edges[index];
                result_out->written_count++;
            }
            else
            {
                status = NX_RESEARCH_GRAPH_QUERY_BUFFER_TOO_SMALL;
            }

            result_out->total_matches++;
        }
    }

    return status;
}

NxResearchGraphQueryStatus NxResearchGraphQuery_FindConnectedNodes(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType edge_type,
    int outgoing,
    const NxResearchGraphNode** nodes_out,
    size_t nodes_capacity,
    NxResearchGraphQueryResult* result_out)
{
    size_t index = 0;
    NxResearchGraphQueryStatus status = NX_RESEARCH_GRAPH_QUERY_OK;

    if (graph == 0 || result_out == 0 || (nodes_capacity > 0 && nodes_out == 0))
    {
        return NX_RESEARCH_GRAPH_QUERY_INVALID_ARGUMENT;
    }

    if (NxResearchGraph_FindNodeById(graph, node_id) == 0)
    {
        NxResearchGraphQuery_ResetResult(result_out);
        return NX_RESEARCH_GRAPH_QUERY_NOT_FOUND;
    }

    NxResearchGraphQuery_ResetResult(result_out);

    for (index = 0; index < graph->edge_count; index++)
    {
        uint32_t connected_node_id = 0;
        const NxResearchGraphNode* connected_node = 0;

        if (!NxResearchGraphQuery_EdgeTypeMatches(graph->edges[index].type, edge_type))
        {
            continue;
        }

        if (outgoing != 0)
        {
            if (graph->edges[index].from_node_id != node_id)
            {
                continue;
            }

            connected_node_id = graph->edges[index].to_node_id;
        }
        else
        {
            if (graph->edges[index].to_node_id != node_id)
            {
                continue;
            }

            connected_node_id = graph->edges[index].from_node_id;
        }

        connected_node = NxResearchGraph_FindNodeById(graph, connected_node_id);
        if (connected_node == 0)
        {
            continue;
        }

        if (result_out->written_count < nodes_capacity)
        {
            nodes_out[result_out->written_count] = connected_node;
            result_out->written_count++;
        }
        else
        {
            status = NX_RESEARCH_GRAPH_QUERY_BUFFER_TOO_SMALL;
        }

        result_out->total_matches++;
    }

    return status;
}
