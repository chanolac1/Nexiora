#include "Nexiora/CognitiveV2/NxGoalGraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_deps(const char* text, unsigned int* deps, unsigned int* count) {
    char buffer[256]; char* cursor;
    *count = 0U; if (text == NULL || text[0] == '\0' || strcmp(text, "-") == 0) return 1;
    (void)snprintf(buffer, sizeof(buffer), "%s", text); buffer[sizeof(buffer)-1U] = '\0'; cursor = buffer;
    while (*cursor != '\0') { char* end; unsigned long v; if (*count >= NX_GOAL_GRAPH_MAX_DEPS) return 0; v = strtoul(cursor, &end, 10); if (end == cursor || v == 0UL) return 0; deps[(*count)++] = (unsigned int)v; if (*end == '\0') break; if (*end != ',') return 0; cursor = end + 1; }
    return 1;
}

static void print_graph(const NxGoalGraph* graph) {
    NxGoalReadySet ready;
    printf("status=OK\nobjective=%s\nnode_count=%u\n", graph->objective, graph->node_count);
    for (unsigned int i = 0U; i < graph->node_count; ++i) {
        const NxGoalNode* n = &graph->nodes[i];
        printf("node_%u=%s|parent=%u|priority=%u|status=%s\n", n->id, n->title, n->parent_id, n->priority, NxGoal_StatusName(n->status));
    }
    if (NxGoalGraph_ListReady(graph, &ready) == NX_GOAL_GRAPH_OK) {
        printf("ready_count=%u\n", ready.count);
        for (unsigned int i = 0U; i < ready.count; ++i) printf("ready_%u=%u\n", i + 1U, ready.node_ids[i]);
    }
}

int main(int argc, char** argv) {
    NxGoalGraphStatus status;
    if (argc < 2) {
        puts("Uso: nexiora_goal_graph create <archivo> <objetivo> | add <archivo> <parent> <priority> <deps|- > <titulo> | status <archivo> | done <archivo> <id>");
        return 0;
    }
    if (strcmp(argv[1], "create") == 0 && argc == 4) {
        status = NxGoalGraph_Create(argv[2], argv[3]); printf("status=%s\npath=%s\n", NxGoalGraph_StatusName(status), argv[2]); return status == NX_GOAL_GRAPH_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "add") == 0 && argc == 7) {
        unsigned int deps[NX_GOAL_GRAPH_MAX_DEPS]; unsigned int count, id = 0U;
        if (!parse_deps(argv[5], deps, &count)) { puts("status=INVALID_DEPENDENCY"); return 1; }
        status = NxGoalGraph_Add(argv[2], (unsigned int)strtoul(argv[3], NULL, 10), argv[6], (unsigned int)strtoul(argv[4], NULL, 10), deps, count, &id);
        printf("status=%s\nnode_id=%u\n", NxGoalGraph_StatusName(status), id); return status == NX_GOAL_GRAPH_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "status") == 0 && argc == 3) {
        NxGoalGraph graph; status = NxGoalGraph_Load(argv[2], &graph); if (status == NX_GOAL_GRAPH_OK) print_graph(&graph); else printf("status=%s\n", NxGoalGraph_StatusName(status)); return status == NX_GOAL_GRAPH_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "done") == 0 && argc == 4) {
        status = NxGoalGraph_SetStatus(argv[2], (unsigned int)strtoul(argv[3], NULL, 10), NX_GOAL_DONE); printf("status=%s\n", NxGoalGraph_StatusName(status)); return status == NX_GOAL_GRAPH_OK ? 0 : 1;
    }
    puts("status=INVALID_ARGUMENT"); return 1;
}
