#include "Nexiora/Research/NxResearchGraph.h"
#include "Nexiora/Research/NxResearchGraphExporter.h"

#include <stdio.h>

static int Expect(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int FileExists(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file == 0)
    {
        return 0;
    }
    fclose(file);
    return 1;
}

static int BuildSampleGraph(NxResearchGraph* graph)
{
    uint32_t experiment_id = 0;
    uint32_t evidence_id = 0;
    uint32_t promotion_id = 0;
    uint32_t edge_id = 0;

    if (NxResearchGraph_Init(graph) != NX_RESEARCH_GRAPH_OK) return 1;
    if (NxResearchGraph_AddNode(graph, NX_RESEARCH_NODE_EXPERIMENT, "SIMD_Vector_Experiment", &experiment_id) != NX_RESEARCH_GRAPH_OK) return 1;
    if (NxResearchGraph_AddNode(graph, NX_RESEARCH_NODE_EVIDENCE, "SIMD_Vector_Evidence", &evidence_id) != NX_RESEARCH_GRAPH_OK) return 1;
    if (NxResearchGraph_AddNode(graph, NX_RESEARCH_NODE_PROMOTION_RECOMMENDATION, "SIMD_Vector_Promotion", &promotion_id) != NX_RESEARCH_GRAPH_OK) return 1;
    if (NxResearchGraph_AddEdge(graph, experiment_id, evidence_id, NX_RESEARCH_EDGE_PRODUCES, "produces", &edge_id) != NX_RESEARCH_GRAPH_OK) return 1;
    if (NxResearchGraph_AddEdge(graph, evidence_id, promotion_id, NX_RESEARCH_EDGE_SUPPORTS, "supports", &edge_id) != NX_RESEARCH_GRAPH_OK) return 1;

    return 0;
}

static int TestExportersCreateFiles(void)
{
    NxResearchGraph graph;

    remove("nrl_0010_test_graph.dot");
    remove("nrl_0010_test_graph.json");
    remove("nrl_0010_test_graph.svg");

    if (Expect(BuildSampleGraph(&graph) == 0, "sample graph should build")) return 1;

    if (Expect(NxResearchGraph_ExportDot(&graph, "nrl_0010_test_graph.dot") == NX_RESEARCH_GRAPH_EXPORT_OK, "DOT export should succeed")) return 1;
    if (Expect(NxResearchGraph_ExportJson(&graph, "nrl_0010_test_graph.json") == NX_RESEARCH_GRAPH_EXPORT_OK, "JSON export should succeed")) return 1;
    if (Expect(NxResearchGraph_ExportSvg(&graph, "nrl_0010_test_graph.svg") == NX_RESEARCH_GRAPH_EXPORT_OK, "SVG export should succeed")) return 1;

    if (Expect(FileExists("nrl_0010_test_graph.dot"), "DOT file should exist")) return 1;
    if (Expect(FileExists("nrl_0010_test_graph.json"), "JSON file should exist")) return 1;
    if (Expect(FileExists("nrl_0010_test_graph.svg"), "SVG file should exist")) return 1;

    NxResearchGraph_Shutdown(&graph);

    remove("nrl_0010_test_graph.dot");
    remove("nrl_0010_test_graph.json");
    remove("nrl_0010_test_graph.svg");

    return 0;
}

static int TestExportersRejectInvalidArguments(void)
{
    NxResearchGraph graph;

    if (Expect(BuildSampleGraph(&graph) == 0, "sample graph should build")) return 1;

    if (Expect(NxResearchGraph_ExportDot(0, "x.dot") == NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT, "DOT null graph should fail")) return 1;
    if (Expect(NxResearchGraph_ExportJson(&graph, 0) == NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT, "JSON null path should fail")) return 1;
    if (Expect(NxResearchGraph_ExportSvg(&graph, "") == NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT, "SVG empty path should fail")) return 1;

    NxResearchGraph_Shutdown(&graph);
    return 0;
}

int main(void)
{
    if (TestExportersCreateFiles() != 0) return 1;
    if (TestExportersRejectInvalidArguments() != 0) return 1;

    printf("NxResearchGraphExporterTests passed\n");
    return 0;
}
