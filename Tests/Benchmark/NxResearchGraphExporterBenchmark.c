#include "Nexiora/Research/NxResearchGraph.h"
#include "Nexiora/Research/NxResearchGraphExporter.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
    NxResearchGraph graph;
    uint32_t previous_id = 0;
    uint32_t current_id = 0;
    uint32_t edge_id = 0;
    size_t index = 0;
    clock_t start;
    clock_t end;
    double elapsed_ms;

    if (NxResearchGraph_Init(&graph) != NX_RESEARCH_GRAPH_OK)
    {
        printf("Failed to initialize graph\n");
        return 1;
    }

    for (index = 0; index < 250; index++)
    {
        char name[64];
        sprintf(name, "Experiment_%u", (unsigned int)index);
        if (NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, name, &current_id) != NX_RESEARCH_GRAPH_OK)
        {
            NxResearchGraph_Shutdown(&graph);
            return 1;
        }

        if (index > 0)
        {
            if (NxResearchGraph_AddEdge(&graph, previous_id, current_id, NX_RESEARCH_EDGE_RELATES_TO, "next", &edge_id) != NX_RESEARCH_GRAPH_OK)
            {
                NxResearchGraph_Shutdown(&graph);
                return 1;
            }
        }

        previous_id = current_id;
    }

    start = clock();
    if (NxResearchGraph_ExportJson(&graph, "nrl_0010_export_benchmark.json") != NX_RESEARCH_GRAPH_EXPORT_OK)
    {
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }
    end = clock();

    elapsed_ms = ((double)(end - start) * 1000.0) / (double)CLOCKS_PER_SEC;

    printf("NxResearchGraphExporterBenchmark\n");
    printf("nodes: %u\n", (unsigned int)NxResearchGraph_GetNodeCount(&graph));
    printf("edges: %u\n", (unsigned int)NxResearchGraph_GetEdgeCount(&graph));
    printf("json_export_ms: %.3f\n", elapsed_ms);

    remove("nrl_0010_export_benchmark.json");
    NxResearchGraph_Shutdown(&graph);
    return 0;
}
