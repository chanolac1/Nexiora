#ifndef NEXIORA_RESEARCH_NX_RESEARCH_DASHBOARD_H
#define NEXIORA_RESEARCH_NX_RESEARCH_DASHBOARD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_RESEARCH_DASHBOARD_PATH_MAX 260
#define NX_RESEARCH_DASHBOARD_MESSAGE_MAX 256

typedef enum NxResearchDashboardStatus
{
    NX_RESEARCH_DASHBOARD_OK = 0,
    NX_RESEARCH_DASHBOARD_INVALID_ARGUMENT = 1,
    NX_RESEARCH_DASHBOARD_IO_ERROR = 2
} NxResearchDashboardStatus;

typedef struct NxResearchDashboardResult
{
    char dashboard_path[NX_RESEARCH_DASHBOARD_PATH_MAX];
    int has_summary;
    int has_report;
    int has_metrics;
    int has_knowledge;
    int has_graph_json;
    int has_graph_dot;
    int has_graph_svg;
    char message[NX_RESEARCH_DASHBOARD_MESSAGE_MAX];
} NxResearchDashboardResult;

const char* NxResearchDashboard_StatusToString(
    NxResearchDashboardStatus status);

NxResearchDashboardStatus NxResearchDashboard_Generate(
    const char* session_path,
    NxResearchDashboardResult* result_out);

NxResearchDashboardStatus NxResearchDashboard_GenerateDefault(
    const char* workspace_root,
    NxResearchDashboardResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
