#include "Nexiora/CognitiveV2/NxGoalGraph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

static void nx_copy(char* dst, size_t size, const char* src) {
    if (dst == NULL || size == 0U) return;
    if (src == NULL) src = "";
    (void)snprintf(dst, size, "%s", src);
    dst[size - 1U] = '\0';
}

static int nx_ensure_parent(const char* path) {
    char buffer[NX_GOAL_GRAPH_MAX_PATH];
    size_t length;
    if (path == NULL) return 0;
    nx_copy(buffer, sizeof(buffer), path);
    length = strlen(buffer);
    for (size_t i = 1U; i < length; ++i) {
        if (buffer[i] == '/' || buffer[i] == '\\') {
            char saved = buffer[i];
            buffer[i] = '\0';
            if (!(i == 2U && buffer[1] == ':')) (void)NX_MKDIR(buffer);
            buffer[i] = saved;
        }
    }
    return 1;
}

static NxGoalStatus nx_parse_node_status(const char* text) {
    if (strcmp(text, "READY") == 0) return NX_GOAL_READY;
    if (strcmp(text, "ACTIVE") == 0) return NX_GOAL_ACTIVE;
    if (strcmp(text, "DONE") == 0) return NX_GOAL_DONE;
    if (strcmp(text, "BLOCKED") == 0) return NX_GOAL_BLOCKED;
    return NX_GOAL_PENDING;
}

static int nx_write_graph(const NxGoalGraph* graph) {
    FILE* file;
    if (graph == NULL || graph->path[0] == '\0') return 0;
    (void)nx_ensure_parent(graph->path);
    file = fopen(graph->path, "wb");
    if (file == NULL) return 0;
    fprintf(file, "nxgoalgraph/1\nobjective=%s\nnode_count=%u\n", graph->objective, graph->node_count);
    for (unsigned int i = 0U; i < graph->node_count; ++i) {
        const NxGoalNode* node = &graph->nodes[i];
        fprintf(file, "node=%u|parent=%u|priority=%u|status=%s|title=%s|deps=",
            node->id, node->parent_id, node->priority, NxGoal_StatusName(node->status), node->title);
        for (unsigned int d = 0U; d < node->dependency_count; ++d) {
            fprintf(file, "%s%u", d == 0U ? "" : ",", node->dependencies[d]);
        }
        fputc('\n', file);
    }
    return fclose(file) == 0;
}

NxGoalGraphStatus NxGoalGraph_Create(const char* path, const char* objective) {
    NxGoalGraph graph;
    if (path == NULL || path[0] == '\0' || objective == NULL || objective[0] == '\0') return NX_GOAL_GRAPH_INVALID_ARGUMENT;
    memset(&graph, 0, sizeof(graph));
    nx_copy(graph.path, sizeof(graph.path), path);
    nx_copy(graph.objective, sizeof(graph.objective), objective);
    return nx_write_graph(&graph) ? NX_GOAL_GRAPH_OK : NX_GOAL_GRAPH_IO_ERROR;
}

static int nx_parse_dependencies(char* text, NxGoalNode* node) {
    char* cursor = text;
    if (text == NULL || node == NULL || text[0] == '\0') return 1;
    while (*cursor != '\0') {
        char* end;
        unsigned long value;
        if (node->dependency_count >= NX_GOAL_GRAPH_MAX_DEPS) return 0;
        value = strtoul(cursor, &end, 10);
        if (end == cursor || value == 0UL || value > 0xffffffffUL) return 0;
        node->dependencies[node->dependency_count++] = (unsigned int)value;
        if (*end == '\0') break;
        if (*end != ',') return 0;
        cursor = end + 1;
    }
    return 1;
}

NxGoalGraphStatus NxGoalGraph_Load(const char* path, NxGoalGraph* out_graph) {
    FILE* file;
    char line[2048];
    if (path == NULL || out_graph == NULL) return NX_GOAL_GRAPH_INVALID_ARGUMENT;
    memset(out_graph, 0, sizeof(*out_graph));
    nx_copy(out_graph->path, sizeof(out_graph->path), path);
    file = fopen(path, "rb");
    if (file == NULL) return NX_GOAL_GRAPH_NOT_FOUND;
    if (fgets(line, sizeof(line), file) == NULL || strncmp(line, "nxgoalgraph/1", 13U) != 0) {
        fclose(file); return NX_GOAL_GRAPH_IO_ERROR;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "objective=", 10U) == 0) {
            nx_copy(out_graph->objective, sizeof(out_graph->objective), line + 10U);
        } else if (strncmp(line, "node=", 5U) == 0) {
            NxGoalNode* node;
            char title[NX_GOAL_GRAPH_MAX_TEXT] = {0};
            char status[32] = {0};
            char deps[256] = {0};
            unsigned int id = 0U, parent = 0U, priority = 0U;
            int matched;
            if (out_graph->node_count >= NX_GOAL_GRAPH_MAX_NODES) { fclose(file); return NX_GOAL_GRAPH_CAPACITY_EXCEEDED; }
            matched = sscanf(line, "node=%u|parent=%u|priority=%u|status=%31[^|]|title=%511[^|]|deps=%255[^\n]",
                &id, &parent, &priority, status, title, deps);
            if (matched < 5) { fclose(file); return NX_GOAL_GRAPH_IO_ERROR; }
            node = &out_graph->nodes[out_graph->node_count++];
            memset(node, 0, sizeof(*node));
            node->id = id; node->parent_id = parent; node->priority = priority;
            node->status = nx_parse_node_status(status); nx_copy(node->title, sizeof(node->title), title);
            if (matched == 6 && !nx_parse_dependencies(deps, node)) { fclose(file); return NX_GOAL_GRAPH_IO_ERROR; }
        }
    }
    fclose(file);
    return NxGoalGraph_Validate(out_graph);
}

const NxGoalNode* NxGoalGraph_Find(const NxGoalGraph* graph, unsigned int node_id) {
    if (graph == NULL || node_id == 0U) return NULL;
    for (unsigned int i = 0U; i < graph->node_count; ++i) if (graph->nodes[i].id == node_id) return &graph->nodes[i];
    return NULL;
}

static int nx_visit(const NxGoalGraph* graph, unsigned int index, unsigned char* marks) {
    const NxGoalNode* node = &graph->nodes[index];
    if (marks[index] == 1U) return 0;
    if (marks[index] == 2U) return 1;
    marks[index] = 1U;
    for (unsigned int d = 0U; d < node->dependency_count; ++d) {
        unsigned int dep_index = NX_GOAL_GRAPH_MAX_NODES;
        for (unsigned int i = 0U; i < graph->node_count; ++i) if (graph->nodes[i].id == node->dependencies[d]) { dep_index = i; break; }
        if (dep_index == NX_GOAL_GRAPH_MAX_NODES || !nx_visit(graph, dep_index, marks)) return 0;
    }
    marks[index] = 2U;
    return 1;
}

NxGoalGraphStatus NxGoalGraph_Validate(const NxGoalGraph* graph) {
    unsigned char marks[NX_GOAL_GRAPH_MAX_NODES] = {0};
    if (graph == NULL) return NX_GOAL_GRAPH_INVALID_ARGUMENT;
    for (unsigned int i = 0U; i < graph->node_count; ++i) {
        const NxGoalNode* node = &graph->nodes[i];
        if (node->id == 0U || node->title[0] == '\0') return NX_GOAL_GRAPH_IO_ERROR;
        if (node->parent_id != 0U && NxGoalGraph_Find(graph, node->parent_id) == NULL) return NX_GOAL_GRAPH_INVALID_PARENT;
        for (unsigned int d = 0U; d < node->dependency_count; ++d) {
            if (node->dependencies[d] == node->id || NxGoalGraph_Find(graph, node->dependencies[d]) == NULL) return NX_GOAL_GRAPH_INVALID_DEPENDENCY;
        }
    }
    for (unsigned int i = 0U; i < graph->node_count; ++i) if (!nx_visit(graph, i, marks)) return NX_GOAL_GRAPH_CYCLE_DETECTED;
    return NX_GOAL_GRAPH_OK;
}

NxGoalGraphStatus NxGoalGraph_Add(const char* path, unsigned int parent_id, const char* title, unsigned int priority,
    const unsigned int* dependencies, unsigned int dependency_count, unsigned int* out_node_id) {
    NxGoalGraph graph;
    NxGoalNode* node;
    NxGoalGraphStatus status = NxGoalGraph_Load(path, &graph);
    if (status != NX_GOAL_GRAPH_OK || title == NULL || title[0] == '\0') return status == NX_GOAL_GRAPH_OK ? NX_GOAL_GRAPH_INVALID_ARGUMENT : status;
    if (graph.node_count >= NX_GOAL_GRAPH_MAX_NODES || dependency_count > NX_GOAL_GRAPH_MAX_DEPS) return NX_GOAL_GRAPH_CAPACITY_EXCEEDED;
    if (parent_id != 0U && NxGoalGraph_Find(&graph, parent_id) == NULL) return NX_GOAL_GRAPH_INVALID_PARENT;
    node = &graph.nodes[graph.node_count]; memset(node, 0, sizeof(*node));
    node->id = graph.node_count == 0U ? 1U : graph.nodes[graph.node_count - 1U].id + 1U;
    node->parent_id = parent_id; node->priority = priority; node->status = NX_GOAL_PENDING; nx_copy(node->title, sizeof(node->title), title);
    for (unsigned int i = 0U; i < dependency_count; ++i) node->dependencies[node->dependency_count++] = dependencies[i];
    graph.node_count++;
    status = NxGoalGraph_Validate(&graph);
    if (status != NX_GOAL_GRAPH_OK) return status;
    if (!nx_write_graph(&graph)) return NX_GOAL_GRAPH_IO_ERROR;
    if (out_node_id != NULL) *out_node_id = node->id;
    return NX_GOAL_GRAPH_OK;
}

NxGoalGraphStatus NxGoalGraph_ListReady(const NxGoalGraph* graph, NxGoalReadySet* out_ready) {
    if (graph == NULL || out_ready == NULL) return NX_GOAL_GRAPH_INVALID_ARGUMENT;
    memset(out_ready, 0, sizeof(*out_ready));
    for (unsigned int i = 0U; i < graph->node_count; ++i) {
        const NxGoalNode* node = &graph->nodes[i];
        int ready = node->status == NX_GOAL_PENDING || node->status == NX_GOAL_READY;
        for (unsigned int d = 0U; ready && d < node->dependency_count; ++d) {
            const NxGoalNode* dep = NxGoalGraph_Find(graph, node->dependencies[d]);
            if (dep == NULL || dep->status != NX_GOAL_DONE) ready = 0;
        }
        if (ready) out_ready->node_ids[out_ready->count++] = node->id;
    }
    for (unsigned int i = 0U; i < out_ready->count; ++i) {
        for (unsigned int j = i + 1U; j < out_ready->count; ++j) {
            const NxGoalNode* a = NxGoalGraph_Find(graph, out_ready->node_ids[i]);
            const NxGoalNode* b = NxGoalGraph_Find(graph, out_ready->node_ids[j]);
            if (a != NULL && b != NULL && (b->priority > a->priority || (b->priority == a->priority && b->id < a->id))) {
                unsigned int tmp = out_ready->node_ids[i]; out_ready->node_ids[i] = out_ready->node_ids[j]; out_ready->node_ids[j] = tmp;
            }
        }
    }
    return NX_GOAL_GRAPH_OK;
}

NxGoalGraphStatus NxGoalGraph_SetStatus(const char* path, unsigned int node_id, NxGoalStatus status) {
    NxGoalGraph graph;
    NxGoalNode* target = NULL;
    NxGoalGraphStatus load = NxGoalGraph_Load(path, &graph);
    if (load != NX_GOAL_GRAPH_OK) return load;
    for (unsigned int i = 0U; i < graph.node_count; ++i) if (graph.nodes[i].id == node_id) { target = &graph.nodes[i]; break; }
    if (target == NULL) return NX_GOAL_GRAPH_NOT_FOUND;
    if (status == NX_GOAL_DONE) {
        for (unsigned int d = 0U; d < target->dependency_count; ++d) {
            const NxGoalNode* dep = NxGoalGraph_Find(&graph, target->dependencies[d]);
            if (dep == NULL || dep->status != NX_GOAL_DONE) return NX_GOAL_GRAPH_INVALID_TRANSITION;
        }
    }
    target->status = status;
    return nx_write_graph(&graph) ? NX_GOAL_GRAPH_OK : NX_GOAL_GRAPH_IO_ERROR;
}

const char* NxGoalGraph_StatusName(NxGoalGraphStatus status) {
    switch (status) {
        case NX_GOAL_GRAPH_OK: return "OK"; case NX_GOAL_GRAPH_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_GOAL_GRAPH_IO_ERROR: return "IO_ERROR"; case NX_GOAL_GRAPH_NOT_FOUND: return "NOT_FOUND";
        case NX_GOAL_GRAPH_CAPACITY_EXCEEDED: return "CAPACITY_EXCEEDED"; case NX_GOAL_GRAPH_INVALID_PARENT: return "INVALID_PARENT";
        case NX_GOAL_GRAPH_INVALID_DEPENDENCY: return "INVALID_DEPENDENCY"; case NX_GOAL_GRAPH_CYCLE_DETECTED: return "CYCLE_DETECTED";
        case NX_GOAL_GRAPH_INVALID_TRANSITION: return "INVALID_TRANSITION"; default: return "UNKNOWN";
    }
}

const char* NxGoal_StatusName(NxGoalStatus status) {
    switch (status) { case NX_GOAL_READY: return "READY"; case NX_GOAL_ACTIVE: return "ACTIVE"; case NX_GOAL_DONE: return "DONE"; case NX_GOAL_BLOCKED: return "BLOCKED"; default: return "PENDING"; }
}
