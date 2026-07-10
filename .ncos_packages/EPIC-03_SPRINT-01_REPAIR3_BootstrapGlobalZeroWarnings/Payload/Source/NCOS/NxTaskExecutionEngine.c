#include "Nexiora/NCOS/NxTaskExecutionEngine.h"

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    size_t n;
    if (dst == NULL || dst_size == 0U) return;
    if (src == NULL) src = "";
    n = strlen(src);
    if (n >= dst_size) n = dst_size - 1U;
    if (n > 0U) memmove(dst, src, n);
    dst[n] = '\0';
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t la, lb, ls, total;
    char tmp[1024];
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) return 0;
    la = strlen(a); lb = strlen(b);
    if (la > 0U && a[la - 1U] != '/' && a[la - 1U] != '\\') sep = "/";
    ls = strlen(sep);
    if (la > SIZE_MAX - ls || la + ls > SIZE_MAX - lb) return 0;
    total = la + ls + lb;
    if (total + 1U > (size_t)dst_size || total + 1U > sizeof(tmp)) return 0;
    if (la > 0U) memcpy(tmp, a, la);
    if (ls > 0U) memcpy(tmp + la, sep, ls);
    if (lb > 0U) memcpy(tmp + la + ls, b, lb);
    tmp[total] = '\0';
    memmove(dst, tmp, total + 1U);
    return 1;
}

static int nx_mkdir_if_needed(const char* path)
{
    if (path == NULL || path[0] == '\0') return 0;
    if (NX_MKDIR(path) == 0) return 1;
    return 1;
}

static int nx_ensure_task_dirs(const char* root, char* tasks_dir, size_t tasks_dir_size)
{
    char knowledge[512];
    char ncos[512];
    if (root == NULL || root[0] == '\0') root = ".";
    nx_mkdir_if_needed(root);
    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    nx_mkdir_if_needed(knowledge);
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    nx_mkdir_if_needed(ncos);
    if (!nx_join(tasks_dir, tasks_dir_size, ncos, "Tasks")) return 0;
    nx_mkdir_if_needed(tasks_dir);
    return 1;
}

static void nx_normalize_id(const char* input, char* out, size_t out_size)
{
    size_t j = 0;
    size_t i;
    if (out == NULL || out_size == 0) return;
    if (input == NULL || input[0] == '\0') input = "plan";
    for (i = 0; input[i] != '\0' && j + 1 < out_size; ++i)
    {
        unsigned char ch = (unsigned char)input[i];
        if (isalnum(ch)) out[j++] = (char)tolower(ch);
        else if ((ch == '-' || ch == '_' || ch == ' ') && j > 0 && out[j - 1] != '_') out[j++] = '_';
    }
    if (j > 0 && out[j - 1] == '_') --j;
    if (j == 0) out[j++] = 'p';
    out[j] = '\0';
}

static int nx_task_file_path(const char* root, const char* plan_id, char* dst, size_t dst_size)
{
    char tasks_dir[512];
    char normalized[128];
    char filename[192];
    if (!nx_ensure_task_dirs(root, tasks_dir, sizeof(tasks_dir))) return 0;
    nx_normalize_id(plan_id, normalized, sizeof(normalized));
    (void)snprintf(filename, sizeof(filename), "%s.tasks", normalized);
    filename[sizeof(filename) - 1] = '\0';
    return nx_join(dst, dst_size, tasks_dir, filename);
}

int NxTaskEngine_LogPath(const char* root, const char* plan_id, char* dst, size_t dst_size)
{
    char tasks_dir[512];
    char normalized[128];
    char filename[192];
    if (!nx_ensure_task_dirs(root, tasks_dir, sizeof(tasks_dir))) return 0;
    nx_normalize_id(plan_id, normalized, sizeof(normalized));
    (void)snprintf(filename, sizeof(filename), "%s.action.log", normalized);
    filename[sizeof(filename) - 1] = '\0';
    return nx_join(dst, dst_size, tasks_dir, filename);
}

const char* NxTaskStatus_ToString(NxTaskStatus status)
{
    switch (status)
    {
        case NX_TASK_PENDING: return "PENDING";
        case NX_TASK_READY: return "READY";
        case NX_TASK_RUNNING: return "RUNNING";
        case NX_TASK_BLOCKED: return "BLOCKED";
        case NX_TASK_FAILED: return "FAILED";
        case NX_TASK_COMPLETED: return "COMPLETED";
        case NX_TASK_SKIPPED: return "SKIPPED";
        default: return "UNKNOWN";
    }
}

int NxTaskStatus_FromString(const char* text, NxTaskStatus* out_status)
{
    if (text == NULL || out_status == NULL) return 0;
#define NX_MATCH_STATUS(name, value) if (strcmp(text, name) == 0) { *out_status = value; return 1; }
    NX_MATCH_STATUS("PENDING", NX_TASK_PENDING)
    NX_MATCH_STATUS("READY", NX_TASK_READY)
    NX_MATCH_STATUS("RUNNING", NX_TASK_RUNNING)
    NX_MATCH_STATUS("BLOCKED", NX_TASK_BLOCKED)
    NX_MATCH_STATUS("FAILED", NX_TASK_FAILED)
    NX_MATCH_STATUS("COMPLETED", NX_TASK_COMPLETED)
    NX_MATCH_STATUS("SKIPPED", NX_TASK_SKIPPED)
#undef NX_MATCH_STATUS
    return 0;
}

static void nx_apply_count(NxTaskPlanSummary* s, NxTaskStatus status)
{
    if (s == NULL) return;
    s->total++;
    if (status == NX_TASK_PENDING || status == NX_TASK_READY) s->pending++;
    else if (status == NX_TASK_RUNNING) s->running++;
    else if (status == NX_TASK_COMPLETED) s->completed++;
    else if (status == NX_TASK_FAILED) s->failed++;
    else if (status == NX_TASK_BLOCKED) s->blocked++;
}

static int nx_parse_task_line(const char* line, NxTaskRecord* rec)
{
    char buffer[768];
    char* p;
    char* id;
    char* status;
    char* title;
    char* result;
    NxTaskStatus st;
    if (line == NULL || rec == NULL) return 0;
    nx_copy(buffer, sizeof(buffer), line);
    p = strchr(buffer, '\n'); if (p) *p = '\0';
    id = buffer;
    status = strchr(id, '|'); if (status == NULL) return 0; *status++ = '\0';
    title = strchr(status, '|'); if (title == NULL) return 0; *title++ = '\0';
    result = strchr(title, '|'); if (result == NULL) return 0; *result++ = '\0';
    if (!NxTaskStatus_FromString(status, &st)) return 0;
    rec->id = atoi(id);
    rec->status = st;
    nx_copy(rec->title, sizeof(rec->title), title);
    nx_copy(rec->result, sizeof(rec->result), result);
    return 1;
}

static void nx_write_task(FILE* f, const NxTaskRecord* rec)
{
    fprintf(f, "%d|%s|%s|%s\n", rec->id, NxTaskStatus_ToString(rec->status), rec->title, rec->result);
}

int NxTaskEngine_AppendLog(const char* root, const char* plan_id, const char* event_type, const char* detail)
{
    char path[512];
    FILE* f;
    time_t now;
    if (!NxTaskEngine_LogPath(root, plan_id, path, sizeof(path))) return 0;
    f = fopen(path, "a");
    if (f == NULL) return 0;
    now = time(NULL);
    fprintf(f, "%lld|%s|%s\n", (long long)now, event_type ? event_type : "event", detail ? detail : "");
    fclose(f);
    return 1;
}

static void nx_default_summary(const char* root, const char* plan_id, NxTaskPlanSummary* out)
{
    if (out == NULL) return;
    memset(out, 0, sizeof(*out));
    nx_normalize_id(plan_id, out->plan_id, sizeof(out->plan_id));
    nx_task_file_path(root, plan_id, out->path, sizeof(out->path));
}

int NxTaskEngine_CreatePlan(const char* root,
                            const char* plan_id,
                            const char* goal,
                            const char* const* steps,
                            size_t step_count,
                            NxTaskPlanSummary* out_summary)
{
    char path[512];
    FILE* f;
    size_t i;
    static const char* defaults[] = {
        "Analizar objetivo",
        "Preparar estructura de trabajo",
        "Ejecutar primera accion",
        "Validar resultado",
        "Registrar aprendizaje"
    };
    if (step_count == 0 || steps == NULL)
    {
        steps = defaults;
        step_count = sizeof(defaults) / sizeof(defaults[0]);
    }
    if (!nx_task_file_path(root, plan_id, path, sizeof(path))) return 0;
    f = fopen(path, "w");
    if (f == NULL) return 0;
    fprintf(f, "#PLAN|%s\n", goal ? goal : "");
    for (i = 0; i < step_count; ++i)
    {
        NxTaskRecord rec;
        memset(&rec, 0, sizeof(rec));
        rec.id = (int)i + 1;
        rec.status = (i == 0) ? NX_TASK_READY : NX_TASK_PENDING;
        nx_copy(rec.title, sizeof(rec.title), steps[i] ? steps[i] : "Tarea");
        nx_copy(rec.result, sizeof(rec.result), "");
        nx_write_task(f, &rec);
    }
    fclose(f);
    NxTaskEngine_AppendLog(root, plan_id, "PLAN_CREATED", goal ? goal : "");
    return NxTaskEngine_Status(root, plan_id, out_summary);
}

static int nx_load_tasks(const char* root, const char* plan_id, NxTaskRecord* records, size_t capacity, size_t* out_count, NxTaskPlanSummary* out_summary)
{
    char path[512];
    FILE* f;
    char line[1024];
    size_t count = 0;
    nx_default_summary(root, plan_id, out_summary);
    if (!nx_task_file_path(root, plan_id, path, sizeof(path))) return 0;
    f = fopen(path, "r");
    if (f == NULL) return 0;
    if (out_summary != NULL) nx_copy(out_summary->path, sizeof(out_summary->path), path);
    while (fgets(line, sizeof(line), f) != NULL)
    {
        NxTaskRecord rec;
        if (strncmp(line, "#PLAN|", 6) == 0)
        {
            if (out_summary != NULL)
            {
                char* nl = strchr(line + 6, '\n');
                if (nl) *nl = '\0';
                nx_copy(out_summary->goal, sizeof(out_summary->goal), line + 6);
            }
            continue;
        }
        if (nx_parse_task_line(line, &rec))
        {
            if (records != NULL && count < capacity) records[count] = rec;
            count++;
            nx_apply_count(out_summary, rec.status);
        }
    }
    fclose(f);
    if (out_count != NULL) *out_count = count;
    return 1;
}

static int nx_save_tasks(const char* root, const char* plan_id, const char* goal, const NxTaskRecord* records, size_t count)
{
    char path[512];
    FILE* f;
    size_t i;
    if (!nx_task_file_path(root, plan_id, path, sizeof(path))) return 0;
    f = fopen(path, "w");
    if (f == NULL) return 0;
    fprintf(f, "#PLAN|%s\n", goal ? goal : "");
    for (i = 0; i < count; ++i) nx_write_task(f, &records[i]);
    fclose(f);
    return 1;
}

int NxTaskEngine_Status(const char* root, const char* plan_id, NxTaskPlanSummary* out_summary)
{
    return nx_load_tasks(root, plan_id, NULL, 0, NULL, out_summary);
}

int NxTaskEngine_RunNext(const char* root,
                         const char* plan_id,
                         NxTaskRecord* out_task,
                         NxTaskPlanSummary* out_summary)
{
    NxTaskRecord records[256];
    NxTaskPlanSummary before;
    size_t count = 0;
    size_t i;
    int selected = -1;
    if (!nx_load_tasks(root, plan_id, records, 256, &count, &before)) return 0;
    for (i = 0; i < count; ++i)
    {
        if (records[i].status == NX_TASK_READY || records[i].status == NX_TASK_PENDING)
        {
            selected = (int)i;
            break;
        }
    }
    if (selected < 0) return NxTaskEngine_Status(root, plan_id, out_summary);
    records[selected].status = NX_TASK_RUNNING;
    nx_copy(records[selected].result, sizeof(records[selected].result), "Ejecutando");
    NxTaskEngine_AppendLog(root, plan_id, "TASK_RUNNING", records[selected].title);
    records[selected].status = NX_TASK_COMPLETED;
    nx_copy(records[selected].result, sizeof(records[selected].result), "Completada por executor deterministico");
    if ((size_t)(selected + 1) < count && records[selected + 1].status == NX_TASK_PENDING)
        records[selected + 1].status = NX_TASK_READY;
    if (!nx_save_tasks(root, plan_id, before.goal, records, count)) return 0;
    NxTaskEngine_AppendLog(root, plan_id, "TASK_COMPLETED", records[selected].title);
    if (out_task != NULL) *out_task = records[selected];
    return NxTaskEngine_Status(root, plan_id, out_summary);
}

int NxTaskEngine_RunAll(const char* root, const char* plan_id, NxTaskPlanSummary* out_summary)
{
    NxTaskPlanSummary summary;
    int guard = 0;
    while (guard++ < 512)
    {
        NxTaskRecord task;
        if (!NxTaskEngine_Status(root, plan_id, &summary)) return 0;
        if (summary.completed + summary.failed + summary.blocked >= summary.total) break;
        if (!NxTaskEngine_RunNext(root, plan_id, &task, &summary)) return 0;
    }
    if (out_summary != NULL) *out_summary = summary;
    return 1;
}
