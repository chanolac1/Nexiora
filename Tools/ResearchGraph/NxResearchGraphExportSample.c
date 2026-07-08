#include "Nexiora/Research/NxResearchGraph.h"
#include "Nexiora/Research/NxResearchGraphExporter.h"

#include <stdio.h>
#include <string.h>

static void BuildPath(char* output, size_t output_size, const char* directory, const char* file_name)
{
    size_t length = strlen(directory);
    const char* separator = "";

    if (length > 0 && directory[length - 1] != '/' && directory[length - 1] != '\\')
    {
        separator = "/";
    }

    snprintf(output, output_size, "%s%s%s", directory, separator, file_name);
}

int main(int argc, char** argv)
{
    const char* output_dir = argc > 1 ? argv[1] : ".";
    char dot_path[512];
    char json_path[512];
    char svg_path[512];
    NxResearchGraph graph;
    uint32_t manifest_id = 0;
    uint32_t experiment_id = 0;
    uint32_t evidence_id = 0;
    uint32_t journal_id = 0;
    uint32_t promotion_id = 0;
    uint32_t edge_id = 0;

    BuildPath(dot_path, sizeof(dot_path), output_dir, "research_graph.dot");
    BuildPath(json_path, sizeof(json_path), output_dir, "research_graph.json");
    BuildPath(svg_path, sizeof(svg_path), output_dir, "research_graph.svg");

    if (NxResearchGraph_Init(&graph) != NX_RESEARCH_GRAPH_OK)
    {
        printf("Failed to initialize graph\n");
        return 1;
    }

    NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_MANIFEST, "Manifest", &manifest_id);
    NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EXPERIMENT, "SIMD_Vector_Experiment", &experiment_id);
    NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_EVIDENCE, "Evidence", &evidence_id);
    NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_JOURNAL_ENTRY, "Journal_Run", &journal_id);
    NxResearchGraph_AddNode(&graph, NX_RESEARCH_NODE_PROMOTION_RECOMMENDATION, "Human_Review_Recommendation", &promotion_id);

    NxResearchGraph_AddEdge(&graph, manifest_id, experiment_id, NX_RESEARCH_EDGE_DECLARES, "declares", &edge_id);
    NxResearchGraph_AddEdge(&graph, experiment_id, evidence_id, NX_RESEARCH_EDGE_PRODUCES, "produces", &edge_id);
    NxResearchGraph_AddEdge(&graph, journal_id, evidence_id, NX_RESEARCH_EDGE_RECORDS, "records", &edge_id);
    NxResearchGraph_AddEdge(&graph, evidence_id, promotion_id, NX_RESEARCH_EDGE_SUPPORTS, "supports", &edge_id);
    NxResearchGraph_AddEdge(&graph, promotion_id, experiment_id, NX_RESEARCH_EDGE_RECOMMENDS, "recommends review", &edge_id);

    if (NxResearchGraph_ExportDot(&graph, dot_path) != NX_RESEARCH_GRAPH_EXPORT_OK ||
        NxResearchGraph_ExportJson(&graph, json_path) != NX_RESEARCH_GRAPH_EXPORT_OK ||
        NxResearchGraph_ExportSvg(&graph, svg_path) != NX_RESEARCH_GRAPH_EXPORT_OK)
    {
        printf("Failed to export graph files\n");
        NxResearchGraph_Shutdown(&graph);
        return 1;
    }

    printf("Generated research graph files:\n");
    printf("  %s\n", dot_path);
    printf("  %s\n", json_path);
    printf("  %s\n", svg_path);

    NxResearchGraph_Shutdown(&graph);
    return 0;
}
