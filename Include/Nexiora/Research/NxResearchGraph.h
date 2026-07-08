#ifndef NEXIORA_RESEARCH_NX_RESEARCH_GRAPH_H
#define NEXIORA_RESEARCH_NX_RESEARCH_GRAPH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchGraphStatus
{
    NX_RESEARCH_GRAPH_OK = 0,
    NX_RESEARCH_GRAPH_INVALID_ARGUMENT = 1,
    NX_RESEARCH_GRAPH_OUT_OF_MEMORY = 2,
    NX_RESEARCH_GRAPH_NOT_FOUND = 3,
    NX_RESEARCH_GRAPH_DUPLICATE_ID = 4,
    NX_RESEARCH_GRAPH_EDGE_ENDPOINT_NOT_FOUND = 5
} NxResearchGraphStatus;

typedef enum NxResearchNodeType
{
    NX_RESEARCH_NODE_UNKNOWN = 0,
    NX_RESEARCH_NODE_EXPERIMENT = 1,
    NX_RESEARCH_NODE_EVIDENCE = 2,
    NX_RESEARCH_NODE_JOURNAL_ENTRY = 3,
    NX_RESEARCH_NODE_MANIFEST = 4,
    NX_RESEARCH_NODE_PROMOTION_RECOMMENDATION = 5,
    NX_RESEARCH_NODE_HYPOTHESIS = 6
} NxResearchNodeType;

typedef enum NxResearchEdgeType
{
    NX_RESEARCH_EDGE_UNKNOWN = 0,
    NX_RESEARCH_EDGE_PRODUCES = 1,
    NX_RESEARCH_EDGE_SUPPORTS = 2,
    NX_RESEARCH_EDGE_RECORDS = 3,
    NX_RESEARCH_EDGE_DECLARES = 4,
    NX_RESEARCH_EDGE_RECOMMENDS = 5,
    NX_RESEARCH_EDGE_RELATES_TO = 6
} NxResearchEdgeType;

typedef struct NxResearchGraphNode
{
    uint32_t id;
    NxResearchNodeType type;
    char name[64];
} NxResearchGraphNode;

typedef struct NxResearchGraphEdge
{
    uint32_t id;
    uint32_t from_node_id;
    uint32_t to_node_id;
    NxResearchEdgeType type;
    char label[64];
} NxResearchGraphEdge;

typedef struct NxResearchGraph
{
    NxResearchGraphNode* nodes;
    size_t node_count;
    size_t node_capacity;

    NxResearchGraphEdge* edges;
    size_t edge_count;
    size_t edge_capacity;

    uint32_t next_node_id;
    uint32_t next_edge_id;
} NxResearchGraph;

NxResearchGraphStatus NxResearchGraph_Init(NxResearchGraph* graph);
void NxResearchGraph_Shutdown(NxResearchGraph* graph);
void NxResearchGraph_Clear(NxResearchGraph* graph);

NxResearchGraphStatus NxResearchGraph_AddNode(
    NxResearchGraph* graph,
    NxResearchNodeType type,
    const char* name,
    uint32_t* node_id_out);

NxResearchGraphStatus NxResearchGraph_AddEdge(
    NxResearchGraph* graph,
    uint32_t from_node_id,
    uint32_t to_node_id,
    NxResearchEdgeType type,
    const char* label,
    uint32_t* edge_id_out);

size_t NxResearchGraph_GetNodeCount(const NxResearchGraph* graph);
size_t NxResearchGraph_GetEdgeCount(const NxResearchGraph* graph);

const NxResearchGraphNode* NxResearchGraph_FindNodeById(
    const NxResearchGraph* graph,
    uint32_t node_id);

const NxResearchGraphNode* NxResearchGraph_FindNodeByName(
    const NxResearchGraph* graph,
    const char* name);

const NxResearchGraphEdge* NxResearchGraph_FindEdgeById(
    const NxResearchGraph* graph,
    uint32_t edge_id);

size_t NxResearchGraph_CountOutgoingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type);

size_t NxResearchGraph_CountIncomingEdges(
    const NxResearchGraph* graph,
    uint32_t node_id,
    NxResearchEdgeType type);

#ifdef __cplusplus
}
#endif

#endif
