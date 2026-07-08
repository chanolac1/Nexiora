#include "Nexiora/Research/NxResearchGraph.h"

#include <stdlib.h>
#include <string.h>

#define NX_RESEARCH_GRAPH_INITIAL_NODE_CAPACITY ((size_t)16)
#define NX_RESEARCH_GRAPH_INITIAL_EDGE_CAPACITY ((size_t)32)

static void NxResearchGraph_CopyText(char* destination, size_t destination_size, const char* source)
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

static NxResearchGraphStatus NxResearchGraph_EnsureNodeCapacity(NxResearchGraph* graph, size_t required_capacity)
{
    size_t new_capacity = 0;
    NxResearchGraphNode* new_nodes = 0;

    if (graph == 0)
    {
        return NX_RESEARCH_GRAPH_INVALID_ARGUMENT;
    }

    if (graph->node_capacity >= required_capacity)
    {
        return NX_RESEARCH_GRAPH_OK;
    }

    new_capacity = graph->node_capacity == 0 ? NX_RESEARCH_GRAPH_INITIAL_NODE_CAPACITY : graph->node_capacity;
    while (new_capacity < required_capacity)
    {
        new_capacity *= 2;
    }

    new_nodes = (NxResearchGraphNode*)realloc(graph->nodes, new_capacity * sizeof(NxResearchGraphNode));
    if (new_nodes == 0)
    {
        return NX_RESEARCH_GRAPH_OUT_OF_MEMORY;
    }

    graph->nodes = new_nodes;
    graph->node_capacity = new_capacity;
    return NX_RESEARCH_GRAPH_OK;
}

static NxResearchGraphStatus NxResearchGraph_EnsureEdgeCapacity(NxResearchGraph* graph, size_t required_capacity)
{
    size_t new_capacity = 0;
    NxResearchGraphEdge* new_edges = 0;

    if (graph == 0)
    {
        return NX_RESEARCH_GRAPH_INVALID_ARGUMENT;
    }

    if (graph->edge_capacity >= required_capacity)
    {
        return NX_RESEARCH_GRAPH_OK;
    }

    new_capacity = graph->edge_capacity == 0 ? NX_RESEARCH_GRAPH_INITIAL_EDGE_CAPACITY : graph->edge_capacity;
    while (new_capacity < required_capacity)
    {
        new_capacity *= 2;
    }

    new_edges = (NxResearchGraphEdge*)realloc(graph->edges, new_capacity * sizeof(NxResearchGraphEdge));
    if (new_edges == 0)
    {
        return NX_RESEARCH_GRAPH_OUT_OF_MEMORY;
    }

    graph->edges = new_edges;
    graph->edge_capacity = new_capacity;
    return NX_RESEARCH_GRAPH_OK;
}

NxResearchGraphStatus NxResearchGraph_Init(NxResearchGraph* graph)
{
    if (graph == 0)
    {
        return NX_RESEARCH_GRAPH_INVALID_ARGUMENT;
    }

    graph->nodes = 0;
    graph->node_count = 0;
    graph->node_capacity = 0;
    graph->edges = 0;
    graph->edge_count = 0;
    graph->edge_capacity = 0;
    graph->next_node_id = 1;
    graph->next_edge_id = 1;

    return NX_RESEARCH_GRAPH_OK;
}

void NxResearchGraph_Shutdown(NxResearchGraph* graph)
{
    if (graph == 0)
    {
        return;
    }

    free(graph->nodes);
    free(graph->edges);

    graph->nodes = 0;
    graph->node_count = 0;
    graph->node_capacity = 0;
    graph->edges = 0;
    graph->edge_count = 0;
    graph->edge_capacity = 0;
    graph->next_node_id = 1;
    graph->next_edge_id = 1;
}

void NxResearchGraph_Clear(NxResearchGraph* graph)
{
    if (graph == 0)
    {
        return;
    }

    graph->node_count = 0;
    graph->edge_count = 0;
    graph->next_node_id = 1;
    graph->next_edge_id = 1;
}

NxResearchGraphStatus NxResearchGraph_AddNode(
    NxResearchGraph* graph,
    NxResearchNodeType type,
    const char* name,
    uint32_t* node_id_out)
{
    NxResearchGraphStatus status = NX_RESEARCH_GRAPH_OK;
    NxResearchGraphNode* node = 0;

    if (graph == 0 || node_id_out == 0)
    {
        return NX_RESEARCH_GRAPH_INVALID_ARGUMENT;
    }

    status = NxResearchGraph_EnsureNodeCapacity(graph, graph->node_count + 1);
    if (status != NX_RESEARCH_GRAPH_OK)
    {
        return status;
    }

    node = &graph->nodes[graph->node_count];
    node->id = graph->next_node_id++;
    node->type = type;
    NxResearchGraph_CopyText(node->name, sizeof(node->name), name);

    graph->node_count++;
    *node_id_out = node->id;

    return NX_RESEARCH_GRAPH_OK;
}

NxResearchGraphStatus NxResearchGraph_AddEdge(
    NxResearchGraph* graph,
    uint32_t from_node_id,
    uint32_t to_node_id,
    NxResearchEdgeType type,
    const char* label,
    uint32_t* edge_id_out)
{
    NxResearchGraphStatus status = NX_RESEARCH_GRAPH_OK;
    NxResearchGraphEdge* edge = 0;

    if (graph == 0 || edge_id_out == 0)
    {
        return NX_RESEARCH_GRAPH_INVALID_ARGUMENT;
    }

    if (NxResearchGraph_FindNodeById(graph, from_node_id) == 0 ||
        NxResearchGraph_FindNodeById(graph, to_node_id) == 0)
    {
        return NX_RESEARCH_GRAPH_EDGE_ENDPOINT_NOT_FOUND;
    }

    status = NxResearchGraph_EnsureEdgeCapacity(graph, graph->edge_count + 1);
    if (status != NX_RESEARCH_GRAPH_OK)
    {
        return status;
    }

    edge = &graph->edges[graph->edge_count];
    edge->id = graph->next_edge_id++;
    edge->from_node_id = from_node_id;
    edge->to_node_id = to_node_id;
    edge->type = type;
    NxResearchGraph_CopyText(edge->label, sizeof(edge->label), label);

    graph->edge_count++;
    *edge_id_out = edge->id;

    return NX_RESEARCH_GRAPH_OK;
}

size_t NxResearchGraph_GetNodeCount(const NxResearchGraph* graph)
{
    return graph == 0 ? 0 : graph->node_count;
}

size_t NxResearchGraph_GetEdgeCount(const NxResearchGraph* graph)
{
    return graph == 0 ? 0 : graph->edge_count;
}

const NxResearchGraphNode* NxResearchGraph_FindNodeById(
    const NxResearchGraph* graph,
    uint32_t node_id)
{
    size_t index = 0;

    if (graph == 0)
    {
        return 0;
    }

    for (index = 0; index < graph->node_count; index++)
    {
        if (graph->nodes[index].id == node_id)
        {
            return &graph->nodes[index];
        }
    }

    return 0;
}

const NxResearchGraphNode* NxResearchGraph_FindNodeByName(
    const NxResearchGraph* graph,
    const char* name)
{
    size_t index = 0;

    if (graph == 0 || name == 0)
    {
        return 0;
    }

    for (index = 0; index < graph->node_count; index++)
    {
        if (strcmp(graph->nodes[index].name, name) == 0)
        {
            return &graph->nodes[index];
        }
    }

    return 0;
}

const NxResearchGraphEdge* NxResearchGraph_FindEdgeById(
    const NxResearchGraph* graph,
    uint32_t edge_id)
{
    size_t index = 0;

    if (graph == 0)
    {
        return 0;
    }

    for (index = 0; index < graph->edge_count; index++)
    {
        if (graph->edges[index].id == edge_id)
        {
            return &graph->edges[index];
        }
    }

    return 0;
}

size_t NxResearchGraph_CountOutgoingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type)
{
    size_t index = 0;
    size_t count = 0;

    if (graph == 0)
    {
        return 0;
    }

    for (index = 0; index < graph->edge_count; index++)
    {
        if (graph->edges[index].from_node_id == node_id &&
            (type == NX_RESEARCH_EDGE_UNKNOWN || graph->edges[index].type == type))
        {
            count++;
        }
    }

    return count;
}

size_t NxResearchGraph_CountIncomingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type)
{
    size_t index = 0;
    size_t count = 0;

    if (graph == 0)
    {
        return 0;
    }

    for (index = 0; index < graph->edge_count; index++)
    {
        if (graph->edges[index].to_node_id == node_id &&
            (type == NX_RESEARCH_EDGE_UNKNOWN || graph->edges[index].type == type))
        {
            count++;
        }
    }

    return count;
}
