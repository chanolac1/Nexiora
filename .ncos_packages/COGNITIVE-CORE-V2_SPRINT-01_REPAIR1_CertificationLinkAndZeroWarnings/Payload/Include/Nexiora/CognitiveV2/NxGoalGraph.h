#ifndef NEXIORA_COGNITIVE_V2_NX_GOAL_GRAPH_H
#define NEXIORA_COGNITIVE_V2_NX_GOAL_GRAPH_H

#include <stddef.h>

#define NX_GOAL_GRAPH_MAX_PATH 1024U
#define NX_GOAL_GRAPH_MAX_TEXT 512U
#define NX_GOAL_GRAPH_MAX_NODES 128U
#define NX_GOAL_GRAPH_MAX_DEPS 16U

typedef enum NxGoalStatus {
    NX_GOAL_PENDING = 0,
    NX_GOAL_READY = 1,
    NX_GOAL_ACTIVE = 2,
    NX_GOAL_DONE = 3,
    NX_GOAL_BLOCKED = 4
} NxGoalStatus;

typedef enum NxGoalGraphStatus {
    NX_GOAL_GRAPH_OK = 0,
    NX_GOAL_GRAPH_INVALID_ARGUMENT = 1,
    NX_GOAL_GRAPH_IO_ERROR = 2,
    NX_GOAL_GRAPH_NOT_FOUND = 3,
    NX_GOAL_GRAPH_CAPACITY_EXCEEDED = 4,
    NX_GOAL_GRAPH_INVALID_PARENT = 5,
    NX_GOAL_GRAPH_INVALID_DEPENDENCY = 6,
    NX_GOAL_GRAPH_CYCLE_DETECTED = 7,
    NX_GOAL_GRAPH_INVALID_TRANSITION = 8
} NxGoalGraphStatus;

typedef struct NxGoalNode {
    unsigned int id;
    unsigned int parent_id;
    unsigned int priority;
    NxGoalStatus status;
    unsigned int dependency_count;
    unsigned int dependencies[NX_GOAL_GRAPH_MAX_DEPS];
    char title[NX_GOAL_GRAPH_MAX_TEXT];
} NxGoalNode;

typedef struct NxGoalGraph {
    char path[NX_GOAL_GRAPH_MAX_PATH];
    char objective[NX_GOAL_GRAPH_MAX_TEXT];
    unsigned int node_count;
    NxGoalNode nodes[NX_GOAL_GRAPH_MAX_NODES];
} NxGoalGraph;

typedef struct NxGoalReadySet {
    unsigned int count;
    unsigned int node_ids[NX_GOAL_GRAPH_MAX_NODES];
} NxGoalReadySet;

NxGoalGraphStatus NxGoalGraph_Create(const char* path, const char* objective);
NxGoalGraphStatus NxGoalGraph_Load(const char* path, NxGoalGraph* out_graph);
NxGoalGraphStatus NxGoalGraph_Add(
    const char* path,
    unsigned int parent_id,
    const char* title,
    unsigned int priority,
    const unsigned int* dependencies,
    unsigned int dependency_count,
    unsigned int* out_node_id);
NxGoalGraphStatus NxGoalGraph_Validate(const NxGoalGraph* graph);
NxGoalGraphStatus NxGoalGraph_ListReady(const NxGoalGraph* graph, NxGoalReadySet* out_ready);
NxGoalGraphStatus NxGoalGraph_SetStatus(const char* path, unsigned int node_id, NxGoalStatus status);
const NxGoalNode* NxGoalGraph_Find(const NxGoalGraph* graph, unsigned int node_id);
const char* NxGoalGraph_StatusName(NxGoalGraphStatus status);
const char* NxGoal_StatusName(NxGoalStatus status);

#endif
