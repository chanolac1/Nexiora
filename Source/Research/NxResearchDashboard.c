#include "Nexiora/Research/NxResearchDashboard.h"

#include <stdio.h>
#include <string.h>

static void nx_dash_copy(char* dst, size_t dst_size, const char* src)
{
    size_t i;
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    for (i = 0; i + 1 < dst_size && src[i] != '\0'; ++i)
    {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

static void nx_dash_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    size_t i;
    size_t pos = 0;
    if (!dst || dst_size == 0) return;
    if (!a) a = ".";
    if (!b) b = "";

    for (i = 0; pos + 1 < dst_size && a[i] != '\0'; ++i)
    {
        dst[pos++] = a[i];
    }

    if (pos > 0 && dst[pos - 1] != '/' && dst[pos - 1] != '\\' && pos + 1 < dst_size)
    {
#ifdef _WIN32
        dst[pos++] = '\\';
#else
        dst[pos++] = '/';
#endif
    }

    for (i = 0; pos + 1 < dst_size && b[i] != '\0'; ++i)
    {
        dst[pos++] = b[i];
    }

    dst[pos] = '\0';
}

static int nx_dash_file_exists(const char* path)
{
    FILE* file;
    if (!path || path[0] == '\0') return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

static int nx_dash_write_artifact_row(FILE* file, const char* name, int exists)
{
    if (!file || !name) return 0;
    fprintf(file,
        "<tr><td>%s</td><td class=\"%s\">%s</td><td>",
        name,
        exists ? "ok" : "missing",
        exists ? "available" : "missing");
    if (exists)
    {
        fprintf(file, "<a href=\"%s\">open</a>", name);
    }
    else
    {
        fprintf(file, "- ");
    }
    fprintf(file, "</td></tr>\n");
    return 1;
}

const char* NxResearchDashboard_StatusToString(
    NxResearchDashboardStatus status)
{
    switch (status)
    {
        case NX_RESEARCH_DASHBOARD_OK:
            return "ok";
        case NX_RESEARCH_DASHBOARD_INVALID_ARGUMENT:
            return "invalid argument";
        case NX_RESEARCH_DASHBOARD_IO_ERROR:
            return "io error";
        default:
            return "unknown";
    }
}

NxResearchDashboardStatus NxResearchDashboard_Generate(
    const char* session_path,
    NxResearchDashboardResult* result_out)
{
    char path[NX_RESEARCH_DASHBOARD_PATH_MAX];
    FILE* file;

    if (!session_path || session_path[0] == '\0' || !result_out)
    {
        return NX_RESEARCH_DASHBOARD_INVALID_ARGUMENT;
    }

    nx_dash_copy(result_out->dashboard_path,
        sizeof(result_out->dashboard_path),
        "");
    result_out->has_summary = 0;
    result_out->has_report = 0;
    result_out->has_metrics = 0;
    result_out->has_knowledge = 0;
    result_out->has_graph_json = 0;
    result_out->has_graph_dot = 0;
    result_out->has_graph_svg = 0;
    nx_dash_copy(result_out->message, sizeof(result_out->message), "");

    nx_dash_join(path, sizeof(path), session_path, "summary.txt");
    result_out->has_summary = nx_dash_file_exists(path);
    nx_dash_join(path, sizeof(path), session_path, "report.md");
    result_out->has_report = nx_dash_file_exists(path);
    nx_dash_join(path, sizeof(path), session_path, "metrics.json");
    result_out->has_metrics = nx_dash_file_exists(path);
    nx_dash_join(path, sizeof(path), session_path, "knowledge.json");
    result_out->has_knowledge = nx_dash_file_exists(path);
    nx_dash_join(path, sizeof(path), session_path, "graph.json");
    result_out->has_graph_json = nx_dash_file_exists(path);
    nx_dash_join(path, sizeof(path), session_path, "graph.dot");
    result_out->has_graph_dot = nx_dash_file_exists(path);
    nx_dash_join(path, sizeof(path), session_path, "graph.svg");
    result_out->has_graph_svg = nx_dash_file_exists(path);

    nx_dash_join(result_out->dashboard_path,
        sizeof(result_out->dashboard_path),
        session_path,
        "dashboard.html");

    file = fopen(result_out->dashboard_path, "wb");
    if (!file)
    {
        nx_dash_copy(result_out->message,
            sizeof(result_out->message),
            "could not create dashboard.html");
        return NX_RESEARCH_DASHBOARD_IO_ERROR;
    }

    fprintf(file, "<!doctype html>\n<html lang=\"en\">\n<head>\n");
    fprintf(file, "<meta charset=\"utf-8\">\n");
    fprintf(file, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
    fprintf(file, "<title>Nexiora Research Dashboard</title>\n");
    fprintf(file, "<style>\n");
    fprintf(file, ":root{font-family:Segoe UI,Arial,sans-serif;color:#e8eefc;background:#09111f;}\n");
    fprintf(file, "body{margin:0;padding:32px;background:linear-gradient(135deg,#09111f,#17233c);}\n");
    fprintf(file, ".wrap{max-width:1100px;margin:0 auto;}\n");
    fprintf(file, ".hero{padding:28px;border:1px solid #2d3d62;border-radius:18px;background:#101a2e;box-shadow:0 20px 60px rgba(0,0,0,.28);}\n");
    fprintf(file, "h1{margin:0 0 8px;font-size:32px;}p{color:#aebbd7;}\n");
    fprintf(file, ".grid{display:grid;grid-template-columns:repeat(4,1fr);gap:14px;margin:20px 0;}\n");
    fprintf(file, ".card{padding:18px;border-radius:14px;background:#14213a;border:1px solid #2b3a60;}\n");
    fprintf(file, ".num{font-size:34px;font-weight:700;} .label{color:#aebbd7;font-size:13px;}\n");
    fprintf(file, "table{width:100%%;border-collapse:collapse;background:#101a2e;border-radius:14px;overflow:hidden;}\n");
    fprintf(file, "td,th{padding:12px 14px;border-bottom:1px solid #263657;text-align:left;}\n");
    fprintf(file, ".ok{color:#69e6a3;font-weight:700}.missing{color:#ffcc66;font-weight:700}a{color:#8db7ff;}\n");
    fprintf(file, ".graph{margin-top:22px;padding:18px;background:#fff;border-radius:14px;}\n");
    fprintf(file, "iframe{width:100%%;min-height:520px;border:0;background:#fff;border-radius:10px;}\n");
    fprintf(file, "@media(max-width:850px){.grid{grid-template-columns:repeat(2,1fr)}}\n");
    fprintf(file, "</style>\n</head>\n<body><div class=\"wrap\">\n");
    fprintf(file, "<section class=\"hero\">\n");
    fprintf(file, "<h1>Nexiora Autonomous Research Dashboard</h1>\n");
    fprintf(file, "<p>Session: <code>%s</code></p>\n", session_path);
    fprintf(file, "<p><strong>Recommendation Gate:</strong> waiting for human approval. <strong>Runtime Promotion:</strong> not automatic.</p>\n");
    fprintf(file, "</section>\n");
    fprintf(file, "<section class=\"grid\">\n");
    fprintf(file, "<div class=\"card\"><div class=\"num\">%d</div><div class=\"label\">Core reports</div></div>\n", result_out->has_summary + result_out->has_report + result_out->has_metrics);
    fprintf(file, "<div class=\"card\"><div class=\"num\">%d</div><div class=\"label\">Knowledge artifacts</div></div>\n", result_out->has_knowledge + result_out->has_graph_json);
    fprintf(file, "<div class=\"card\"><div class=\"num\">%d</div><div class=\"label\">Graph exports</div></div>\n", result_out->has_graph_dot + result_out->has_graph_svg);
    fprintf(file, "<div class=\"card\"><div class=\"num\">7</div><div class=\"label\">Expected artifacts</div></div>\n");
    fprintf(file, "</section>\n");
    fprintf(file, "<h2>Session artifacts</h2>\n<table><thead><tr><th>Artifact</th><th>Status</th><th>Action</th></tr></thead><tbody>\n");
    nx_dash_write_artifact_row(file, "summary.txt", result_out->has_summary);
    nx_dash_write_artifact_row(file, "report.md", result_out->has_report);
    nx_dash_write_artifact_row(file, "metrics.json", result_out->has_metrics);
    nx_dash_write_artifact_row(file, "knowledge.json", result_out->has_knowledge);
    nx_dash_write_artifact_row(file, "graph.json", result_out->has_graph_json);
    nx_dash_write_artifact_row(file, "graph.dot", result_out->has_graph_dot);
    nx_dash_write_artifact_row(file, "graph.svg", result_out->has_graph_svg);
    fprintf(file, "</tbody></table>\n");

    if (result_out->has_graph_svg)
    {
        fprintf(file, "<section class=\"graph\"><iframe src=\"graph.svg\" title=\"Research graph\"></iframe></section>\n");
    }

    fprintf(file, "</div></body></html>\n");
    fclose(file);

    nx_dash_copy(result_out->message,
        sizeof(result_out->message),
        "dashboard generated; open dashboard.html in a browser");
    return NX_RESEARCH_DASHBOARD_OK;
}

NxResearchDashboardStatus NxResearchDashboard_GenerateDefault(
    const char* workspace_root,
    NxResearchDashboardResult* result_out)
{
    char research_path[NX_RESEARCH_DASHBOARD_PATH_MAX];
    char sessions_path[NX_RESEARCH_DASHBOARD_PATH_MAX];
    char session_path[NX_RESEARCH_DASHBOARD_PATH_MAX];

    if (!workspace_root || workspace_root[0] == '\0' || !result_out)
    {
        return NX_RESEARCH_DASHBOARD_INVALID_ARGUMENT;
    }

    nx_dash_join(research_path, sizeof(research_path), workspace_root, "Research");
    nx_dash_join(sessions_path, sizeof(sessions_path), research_path, "Sessions");
    nx_dash_join(session_path, sizeof(session_path), sessions_path, "first_autonomous_execution");

    return NxResearchDashboard_Generate(session_path, result_out);
}
