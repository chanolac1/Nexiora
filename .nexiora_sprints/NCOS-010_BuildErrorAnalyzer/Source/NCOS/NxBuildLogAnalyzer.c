#include "Nexiora/NCOS/NxBuildLogAnalyzer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#define NX_SEP "\\"
#else
#include <unistd.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#define NX_SEP "/"
#endif

static void nx_zero(void* p, size_t n) { memset(p, 0, n); }

static void nx_lower_copy(char* dst, size_t dst_size, const char* src)
{
    size_t i;
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    for (i = 0; i + 1 < dst_size && src[i]; ++i) dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = 0;
}

static int nx_contains_ci(const char* text, const char* needle)
{
    char a[2048];
    char b[256];
    nx_lower_copy(a, sizeof(a), text);
    nx_lower_copy(b, sizeof(b), needle);
    return strstr(a, b) != NULL;
}

static void nx_safe_id(char* dst, size_t dst_size, const char* src)
{
    size_t i, j = 0;
    if (!dst || dst_size == 0) return;
    if (!src || !*src) src = "run";
    for (i = 0; src[i] && j + 1 < dst_size; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c)) dst[j++] = (char)tolower(c);
        else if (c == '_' || c == '-') dst[j++] = (char)c;
        else if (j > 0 && dst[j - 1] != '_') dst[j++] = '_';
    }
    if (j == 0) dst[j++] = 'r';
    dst[j] = 0;
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    size_t la;
    const char* sep = NX_SEP;
    if (!dst || dst_size == 0 || !a || !b) return 0;
    la = strlen(a);
    if (la > 0 && (a[la - 1] == '/' || a[la - 1] == '\\')) sep = "";
    return snprintf(dst, dst_size, "%s%s%s", a, sep, b) > 0;
}

static void nx_mkdir_if_missing(const char* path)
{
    if (!path || !*path) return;
    NX_MKDIR(path);
}

static void nx_ensure_dirs(const char* root, char* compiler_dir, size_t compiler_dir_size)
{
    char knowledge[512];
    char ncos[512];
    const char* base = (root && *root) ? root : ".";
    nx_join(knowledge, sizeof(knowledge), base, "Knowledge");
    nx_mkdir_if_missing(base);
    nx_mkdir_if_missing(knowledge);
    nx_join(ncos, sizeof(ncos), knowledge, "NCOS");
    nx_mkdir_if_missing(ncos);
    nx_join(compiler_dir, compiler_dir_size, ncos, "BuildAnalysis");
    nx_mkdir_if_missing(compiler_dir);
}

static void nx_trim_newline(char* s)
{
    size_t n;
    if (!s) return;
    n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = 0;
}

static void nx_add_finding(NxBuildLogAnalysis* out, const char* type, const char* line)
{
    NxBuildLogFinding* f;
    char tmp[1024];
    char* p;
    if (!out || out->finding_count >= 32 || !line) return;
    f = &out->findings[out->finding_count++];
    nx_zero(f, sizeof(*f));
    snprintf(f->type, sizeof(f->type), "%s", type ? type : "info");
    snprintf(f->message, sizeof(f->message), "%s", line);
    snprintf(tmp, sizeof(tmp), "%s", line);
    p = strchr(tmp, ':');
    if (p) {
        *p = 0;
        if (strchr(tmp, '/') || strchr(tmp, '\\') || strchr(tmp, '.')) {
            snprintf(f->file, sizeof(f->file), "%s", tmp);
            if (isdigit((unsigned char)p[1])) f->line = atoi(p + 1);
        }
    }
}

static void nx_classify_line(NxBuildLogAnalysis* out, const char* line)
{
    if (!out || !line || !*line) return;

    if (nx_contains_ci(line, "error:") || nx_contains_ci(line, "fatal error") ||
        nx_contains_ci(line, "undefined reference") || nx_contains_ci(line, "unresolved external") ||
        nx_contains_ci(line, "no se reconoce") || nx_contains_ci(line, "failed")) {
        out->error_count++;
        nx_add_finding(out, "error", line);
        return;
    }

    if (nx_contains_ci(line, "warning:") || nx_contains_ci(line, "advertencia")) {
        out->warning_count++;
        nx_add_finding(out, "warning", line);
        return;
    }

    if (nx_contains_ci(line, "tests failed") || nx_contains_ci(line, "failed out of") ||
        nx_contains_ci(line, "***failed")) {
        out->failure_count++;
        nx_add_finding(out, "test_failure", line);
        return;
    }
}

static int nx_write_report(const char* report_path, const char* run_id, const char* log_path, const NxBuildLogAnalysis* out)
{
    FILE* fp;
    int i;
    fp = fopen(report_path, "wb");
    if (!fp) return 0;
    fprintf(fp, "# NCOS Build Log Analysis\n\n");
    fprintf(fp, "Run ID: %s\n\n", run_id ? run_id : "run");
    fprintf(fp, "Log: %s\n\n", log_path ? log_path : "");
    fprintf(fp, "Status: %s\n\n", out->success ? "OK" : "FAILED");
    fprintf(fp, "Errors: %d\n", out->error_count);
    fprintf(fp, "Warnings: %d\n", out->warning_count);
    fprintf(fp, "Test failures: %d\n", out->failure_count);
    fprintf(fp, "Findings: %d\n\n", out->finding_count);
    fprintf(fp, "## Findings\n\n");
    for (i = 0; i < out->finding_count; ++i) {
        fprintf(fp, "- [%s] %s\n", out->findings[i].type, out->findings[i].message);
    }
    fclose(fp);
    return 1;
}

int NxBuildLogAnalyzer_AnalyzeFile(const char* knowledge_root,
                                   const char* run_id,
                                   const char* log_path,
                                   NxBuildLogAnalysis* out)
{
    FILE* fp;
    char line[2048];
    char safe[128];
    char dir[512];
    char report_name[192];

    if (!out || !log_path || !*log_path) return 0;
    nx_zero(out, sizeof(*out));

    fp = fopen(log_path, "rb");
    if (!fp) {
        out->success = 0;
        out->error_count = 1;
        snprintf(out->summary, sizeof(out->summary), "No pude abrir el log: %s", log_path);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        nx_trim_newline(line);
        nx_classify_line(out, line);
    }
    fclose(fp);

    out->success = (out->error_count == 0 && out->failure_count == 0) ? 1 : 0;
    nx_safe_id(safe, sizeof(safe), run_id);
    nx_ensure_dirs(knowledge_root, dir, sizeof(dir));
    snprintf(report_name, sizeof(report_name), "%s.analysis.md", safe);
    nx_join(out->report_path, sizeof(out->report_path), dir, report_name);

    snprintf(out->summary, sizeof(out->summary),
             "%s. Errores=%d, warnings=%d, fallas de prueba=%d, hallazgos=%d.",
             out->success ? "El log no muestra fallas criticas" : "El log contiene fallas que requieren accion",
             out->error_count, out->warning_count, out->failure_count, out->finding_count);

    nx_write_report(out->report_path, run_id, log_path, out);
    return 1;
}
