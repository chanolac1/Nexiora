#include "Nexiora/Research/NxResearchGraphExporter.h"

#include <stdio.h>
#include <string.h>

static const char* NxResearchNodeTypeName(NxResearchNodeType type)
{
    switch (type)
    {
        case NX_RESEARCH_NODE_EXPERIMENT: return "experiment";
        case NX_RESEARCH_NODE_EVIDENCE: return "evidence";
        case NX_RESEARCH_NODE_JOURNAL_ENTRY: return "journal_entry";
        case NX_RESEARCH_NODE_MANIFEST: return "manifest";
        case NX_RESEARCH_NODE_PROMOTION_RECOMMENDATION: return "promotion_recommendation";
        case NX_RESEARCH_NODE_HYPOTHESIS: return "hypothesis";
        default: return "unknown";
    }
}

static const char* NxResearchEdgeTypeName(NxResearchEdgeType type)
{
    switch (type)
    {
        case NX_RESEARCH_EDGE_PRODUCES: return "produces";
        case NX_RESEARCH_EDGE_SUPPORTS: return "supports";
        case NX_RESEARCH_EDGE_RECORDS: return "records";
        case NX_RESEARCH_EDGE_DECLARES: return "declares";
        case NX_RESEARCH_EDGE_RECOMMENDS: return "recommends";
        case NX_RESEARCH_EDGE_RELATES_TO: return "relates_to";
        default: return "unknown";
    }
}

static void NxResearchGraph_WriteEscaped(FILE* file, const char* text)
{
    const unsigned char* cursor = (const unsigned char*)text;

    if (text == 0)
    {
        return;
    }

    while (*cursor != 0)
    {
        switch (*cursor)
        {
            case '\\': fputs("\\\\", file); break;
            case '"': fputs("\\\"", file); break;
            case '\n': fputs("\\n", file); break;
            case '\r': fputs("\\r", file); break;
            case '\t': fputs("\\t", file); break;
            case '&': fputs("&amp;", file); break;
            case '<': fputs("&lt;", file); break;
            case '>': fputs("&gt;", file); break;
            default: fputc((int)*cursor, file); break;
        }

        cursor++;
    }
}

NxResearchGraphExportStatus NxResearchGraph_ExportDot(
    const NxResearchGraph* graph,
    const char* file_path)
{
    FILE* file = 0;
    size_t index = 0;

    if (graph == 0 || file_path == 0 || file_path[0] == '\0')
    {
        return NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT;
    }

    file = fopen(file_path, "w");
    if (file == 0)
    {
        return NX_RESEARCH_GRAPH_EXPORT_IO_ERROR;
    }

    fputs("digraph NexioraResearchGraph {\n", file);
    fputs("  graph [rankdir=LR, label=\"Nexiora Research Graph\", labelloc=t];\n", file);
    fputs("  node [shape=box, style=rounded];\n", file);

    for (index = 0; index < graph->node_count; index++)
    {
        const NxResearchGraphNode* node = &graph->nodes[index];
        fprintf(file, "  n%u [label=\"", (unsigned int)node->id);
        NxResearchGraph_WriteEscaped(file, node->name);
        fprintf(file, "\\n%s\"];\n", NxResearchNodeTypeName(node->type));
    }

    for (index = 0; index < graph->edge_count; index++)
    {
        const NxResearchGraphEdge* edge = &graph->edges[index];
        fprintf(file, "  n%u -> n%u [label=\"",
            (unsigned int)edge->from_node_id,
            (unsigned int)edge->to_node_id);

        if (edge->label[0] != '\0')
        {
            NxResearchGraph_WriteEscaped(file, edge->label);
        }
        else
        {
            fputs(NxResearchEdgeTypeName(edge->type), file);
        }

        fputs("\"];\n", file);
    }

    fputs("}\n", file);
    fclose(file);
    return NX_RESEARCH_GRAPH_EXPORT_OK;
}

NxResearchGraphExportStatus NxResearchGraph_ExportJson(
    const NxResearchGraph* graph,
    const char* file_path)
{
    FILE* file = 0;
    size_t index = 0;

    if (graph == 0 || file_path == 0 || file_path[0] == '\0')
    {
        return NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT;
    }

    file = fopen(file_path, "w");
    if (file == 0)
    {
        return NX_RESEARCH_GRAPH_EXPORT_IO_ERROR;
    }

    fputs("{\n", file);
    fprintf(file, "  \"node_count\": %u,\n", (unsigned int)graph->node_count);
    fprintf(file, "  \"edge_count\": %u,\n", (unsigned int)graph->edge_count);
    fputs("  \"nodes\": [\n", file);

    for (index = 0; index < graph->node_count; index++)
    {
        const NxResearchGraphNode* node = &graph->nodes[index];
        fprintf(file, "    {\"id\": %u, \"type\": \"%s\", \"name\": \"",
            (unsigned int)node->id,
            NxResearchNodeTypeName(node->type));
        NxResearchGraph_WriteEscaped(file, node->name);
        fprintf(file, "\"}%s\n", index + 1 == graph->node_count ? "" : ",");
    }

    fputs("  ],\n", file);
    fputs("  \"edges\": [\n", file);

    for (index = 0; index < graph->edge_count; index++)
    {
        const NxResearchGraphEdge* edge = &graph->edges[index];
        fprintf(file, "    {\"id\": %u, \"from\": %u, \"to\": %u, \"type\": \"%s\", \"label\": \"",
            (unsigned int)edge->id,
            (unsigned int)edge->from_node_id,
            (unsigned int)edge->to_node_id,
            NxResearchEdgeTypeName(edge->type));
        NxResearchGraph_WriteEscaped(file, edge->label);
        fprintf(file, "\"}%s\n", index + 1 == graph->edge_count ? "" : ",");
    }

    fputs("  ]\n", file);
    fputs("}\n", file);
    fclose(file);
    return NX_RESEARCH_GRAPH_EXPORT_OK;
}

NxResearchGraphExportStatus NxResearchGraph_ExportSvg(
    const NxResearchGraph* graph,
    const char* file_path)
{
    FILE* file = 0;
    size_t index = 0;
    unsigned int width = 900;
    unsigned int height = 220;

    if (graph == 0 || file_path == 0 || file_path[0] == '\0')
    {
        return NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT;
    }

    if (graph->node_count > 3)
    {
        width = 220U * (unsigned int)graph->node_count;
    }

    file = fopen(file_path, "w");
    if (file == 0)
    {
        return NX_RESEARCH_GRAPH_EXPORT_IO_ERROR;
    }

    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%u\" height=\"%u\" viewBox=\"0 0 %u %u\">\n", width, height, width, height);
    fputs("<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n", file);
    fputs("<text x=\"24\" y=\"32\" font-family=\"Arial\" font-size=\"20\">Nexiora Research Graph</text>\n", file);
    fputs("<defs><marker id=\"arrow\" markerWidth=\"10\" markerHeight=\"10\" refX=\"9\" refY=\"3\" orient=\"auto\" markerUnits=\"strokeWidth\"><path d=\"M0,0 L0,6 L9,3 z\" fill=\"#333\"/></marker></defs>\n", file);

    for (index = 0; index < graph->edge_count; index++)
    {
        const NxResearchGraphEdge* edge = &graph->edges[index];
        unsigned int from_index = 0;
        unsigned int to_index = 0;
        size_t node_index = 0;

        for (node_index = 0; node_index < graph->node_count; node_index++)
        {
            if (graph->nodes[node_index].id == edge->from_node_id) from_index = (unsigned int)node_index;
            if (graph->nodes[node_index].id == edge->to_node_id) to_index = (unsigned int)node_index;
        }

        fprintf(file, "<line x1=\"%u\" y1=\"110\" x2=\"%u\" y2=\"110\" stroke=\"#333\" stroke-width=\"2\" marker-end=\"url(#arrow)\"/>\n",
            170U + (from_index * 220U),
            55U + (to_index * 220U));
        fprintf(file, "<text x=\"%u\" y=\"96\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"middle\">",
            110U + (from_index * 220U) + ((to_index > from_index ? to_index - from_index : from_index - to_index) * 110U));
        NxResearchGraph_WriteEscaped(file, edge->label[0] == '\0' ? NxResearchEdgeTypeName(edge->type) : edge->label);
        fputs("</text>\n", file);
    }

    for (index = 0; index < graph->node_count; index++)
    {
        const NxResearchGraphNode* node = &graph->nodes[index];
        unsigned int x = 30U + ((unsigned int)index * 220U);

        fprintf(file, "<rect x=\"%u\" y=\"70\" width=\"150\" height=\"80\" rx=\"10\" fill=\"#f8f8f8\" stroke=\"#222\"/>\n", x);
        fprintf(file, "<text x=\"%u\" y=\"104\" font-family=\"Arial\" font-size=\"13\" text-anchor=\"middle\">", x + 75U);
        NxResearchGraph_WriteEscaped(file, node->name);
        fputs("</text>\n", file);
        fprintf(file, "<text x=\"%u\" y=\"128\" font-family=\"Arial\" font-size=\"11\" text-anchor=\"middle\">%s</text>\n", x + 75U, NxResearchNodeTypeName(node->type));
    }

    fputs("</svg>\n", file);
    fclose(file);
    return NX_RESEARCH_GRAPH_EXPORT_OK;
}
