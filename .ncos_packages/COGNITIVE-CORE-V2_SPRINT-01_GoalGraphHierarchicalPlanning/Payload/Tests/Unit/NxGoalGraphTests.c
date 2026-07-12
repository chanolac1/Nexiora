#include "Nexiora/CognitiveV2/NxGoalGraph.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <process.h>
#define NX_PID() ((unsigned int)_getpid())
#else
#include <unistd.h>
#define NX_PID() ((unsigned int)getpid())
#endif
static int failures = 0;
#define CHECK(e,m) do { if (!(e)) { fprintf(stderr,"FAIL: %s\n",m); ++failures; } } while (0)
int main(void) {
    char path[256]; NxGoalGraph graph; NxGoalReadySet ready; unsigned int architecture, engine, gameplay, deps[2];
    (void)snprintf(path, sizeof(path), "Build/goal_graph_%u/demo.nxgoal", NX_PID());
    CHECK(NxGoalGraph_Create(path, "Desarrollar un RPG de accion") == NX_GOAL_GRAPH_OK, "create graph");
    CHECK(NxGoalGraph_Add(path, 0U, "Arquitectura del juego", 100U, NULL, 0U, &architecture) == NX_GOAL_GRAPH_OK, "add architecture");
    deps[0] = architecture;
    CHECK(NxGoalGraph_Add(path, architecture, "Motor de combate", 90U, deps, 1U, &engine) == NX_GOAL_GRAPH_OK, "add engine");
    deps[0] = engine;
    CHECK(NxGoalGraph_Add(path, architecture, "Bucle de gameplay", 80U, deps, 1U, &gameplay) == NX_GOAL_GRAPH_OK, "add gameplay");
    CHECK(NxGoalGraph_Load(path, &graph) == NX_GOAL_GRAPH_OK, "load graph");
    CHECK(graph.node_count == 3U, "three nodes");
    CHECK(NxGoalGraph_ListReady(&graph, &ready) == NX_GOAL_GRAPH_OK && ready.count == 1U && ready.node_ids[0] == architecture, "only architecture ready");
    CHECK(NxGoalGraph_SetStatus(path, engine, NX_GOAL_DONE) == NX_GOAL_GRAPH_INVALID_TRANSITION, "cannot skip dependency");
    CHECK(NxGoalGraph_SetStatus(path, architecture, NX_GOAL_DONE) == NX_GOAL_GRAPH_OK, "complete architecture");
    CHECK(NxGoalGraph_Load(path, &graph) == NX_GOAL_GRAPH_OK, "reload graph");
    CHECK(NxGoalGraph_ListReady(&graph, &ready) == NX_GOAL_GRAPH_OK && ready.count == 1U && ready.node_ids[0] == engine, "engine ready next");
    CHECK(NxGoalGraph_SetStatus(path, engine, NX_GOAL_DONE) == NX_GOAL_GRAPH_OK, "complete engine");
    CHECK(NxGoalGraph_Load(path, &graph) == NX_GOAL_GRAPH_OK, "reload second");
    CHECK(NxGoalGraph_ListReady(&graph, &ready) == NX_GOAL_GRAPH_OK && ready.count == 1U && ready.node_ids[0] == gameplay, "gameplay ready last");
    deps[0] = gameplay;
    CHECK(NxGoalGraph_Add(path, 0U, "Ciclo invalido", 10U, deps, 1U, NULL) == NX_GOAL_GRAPH_OK, "forward dependency valid after target exists");
    CHECK(NxGoalGraph_Load(path, &graph) == NX_GOAL_GRAPH_OK, "final graph valid");
    (void)remove(path);
    if (failures != 0) return 1;
    puts("NxGoalGraphTests: PASS"); return 0;
}
