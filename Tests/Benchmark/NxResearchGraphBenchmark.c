#include "Nexiora/Research/NxResearchGraph.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
    NxResearchGraph graph;
    uint32_t ids[1000];
    size_t index = 0;
    clock_t start;
    clock_t end;
    double elapsed_ms;

    if (NxResearchGraph_Init(&graph) != NX_RESEARCH_GRAPH_OK)
    {
        printf("Failed to initialize research graph\n");
        return 1;
    }

    start = clock();

    for (index = 0; index < 1000; index++)
    {
        char name[64];
        sprintf(name, "ResearchNode_%u", (unsigned int)index);
        if (NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, name, &ids[index]) != NX_RESEARCH_GRAPH_OK)
        {
            printf("Failed to add node\n");
            NxResearchGraph_Shutdown(&graph);
            return 1;
        }
    }

    for (index = 1; index < 1000; index++)
    {
        uint32_t edge_id = 0;
        if (NxResearchGraph_AddEdge(&graph, ids[index - 1], ids[index], NX_RESEARCH_EDGE_RELATES_TO, "sequence", &edge_id) != NX_RESEARCH_GRAPH_OK)
        {
            printf("Failed to add edge\n");
            NxResearchGraph_Shutdown(&graph);
            return 1;
        }
    }

    end = clock();
    elapsed_ms = ((double)(end - start) * 1000.0) / (double)CLOCKS_PER_SEC;

    printf("NxResearchGraphBenchmark\n");
    printf("nodes: %u\n", (unsigned int)NxResearchGraph_GetNodeCount(&graph));
    printf("edges: %u\n", (unsigned int)NxResearchGraph_GetEdgeCount(&graph));
    printf("elapsed_ms: %.3f\n", elapsed_ms);

    NxResearchGraph_Shutdown(&graph);
    return 0;
}
