#ifndef NEXIORA_RESEARCH_NX_RESEARCH_GRAPH_EXPORTER_H
#define NEXIORA_RESEARCH_NX_RESEARCH_GRAPH_EXPORTER_H

#include "Nexiora/Research/NxResearchGraph.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchGraphExportStatus
{
    NX_RESEARCH_GRAPH_EXPORT_OK = 0,
    NX_RESEARCH_GRAPH_EXPORT_INVALID_ARGUMENT = 1,
    NX_RESEARCH_GRAPH_EXPORT_IO_ERROR = 2
} NxResearchGraphExportStatus;

NxResearchGraphExportStatus NxResearchGraph_ExportDot(
    const NxResearchGraph* graph,
    const char* file_path);

NxResearchGraphExportStatus NxResearchGraph_ExportJson(
    const NxResearchGraph* graph,
    const char* file_path);

NxResearchGraphExportStatus NxResearchGraph_ExportSvg(
    const NxResearchGraph* graph,
    const char* file_path);

#ifdef __cplusplus
}
#endif

#endif
