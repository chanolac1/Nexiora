#include "Nexiora/NCOS/NxPlanningEngine.h"

#include <ctype.h>
#include <stdio.h>
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
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    if (dst == NULL || dst_size == 0) return;
    const char* left = a == NULL ? "" : a;
    const char* right = b == NULL ? "" : b;
    const char sep = '/';
    size_t len = strlen(left);
    if (len == 0) {
        (void)snprintf(dst, dst_size, "%s", right);
    } else if (left[len - 1] == '/' || left[len - 1] == '\\') {
        (void)snprintf(dst, dst_size, "%s%s", left, right);
    } else {
        (void)snprintf(dst, dst_size, "%s%c%s", left, sep, right);
    }
    dst[dst_size - 1] = '\0';
}

static void nx_normalize_id(const char* text, char* out, size_t out_size)
{
    if (out == NULL || out_size == 0) return;
    size_t j = 0;
    int last_sep = 0;
    const char* src = text == NULL ? "plan" : text;
    for (size_t i = 0; src[i] != '\0' && j + 1 < out_size; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c)) {
            out[j++] = (char)tolower(c);
            last_sep = 0;
        } else if (!last_sep && j > 0) {
            out[j++] = '_';
            last_sep = 1;
        }
    }
    while (j > 0 && out[j - 1] == '_') --j;
    if (j == 0) {
        nx_copy(out, out_size, "plan");
        return;
    }
    out[j] = '\0';
}

static void nx_ensure_base_dirs(const char* root, char* plans_dir, size_t plans_dir_size)
{
    char knowledge[NX_PLAN_MAX_PATH];
    char ncos[NX_PLAN_MAX_PATH];
    nx_join(knowledge, sizeof(knowledge), root == NULL ? "." : root, "Knowledge");
    (void)NX_MKDIR(knowledge);
    nx_join(ncos, sizeof(ncos), knowledge, "NCOS");
    (void)NX_MKDIR(ncos);
    nx_join(plans_dir, plans_dir_size, ncos, "Plans");
    (void)NX_MKDIR(plans_dir);
}

static void nx_active_path(const char* root, char* out, size_t out_size)
{
    char plans[NX_PLAN_MAX_PATH];
    nx_ensure_base_dirs(root, plans, sizeof(plans));
    nx_join(out, out_size, plans, "active_plan.txt");
}

static void nx_plan_path(const char* root, const char* id, char* out, size_t out_size)
{
    char plans[NX_PLAN_MAX_PATH];
    char file[NX_PLAN_MAX_PATH];
    nx_ensure_base_dirs(root, plans, sizeof(plans));
    (void)snprintf(file, sizeof(file), "%s.jsonl", id == NULL ? "plan" : id);
    file[sizeof(file) - 1] = '\0';
    nx_join(out, out_size, plans, file);
}

static int nx_write_active(const char* root, const char* id)
{
    char path[NX_PLAN_MAX_PATH];
    nx_active_path(root, path, sizeof(path));
    FILE* f = fopen(path, "wb");
    if (f == NULL) return 0;
    fprintf(f, "%s\n", id == NULL ? "" : id);
    fclose(f);
    return 1;
}

static int nx_read_active(const char* root, char* id, size_t id_size)
{
    char path[NX_PLAN_MAX_PATH];
    nx_active_path(root, path, sizeof(path));
    FILE* f = fopen(path, "rb");
    if (f == NULL) return 0;
    if (fgets(id, (int)id_size, f) == NULL) {
        fclose(f);
        return 0;
    }
    fclose(f);
    id[strcspn(id, "\r\n")] = '\0';
    return id[0] != '\0';
}

static const char* nx_default_steps[] = {
    "Entender el objetivo y restricciones",
    "Revisar conocimiento y evidencia disponible",
    "Dividir el trabajo en tareas verificables",
    "Ejecutar la primera tarea funcional",
    "Validar resultados y registrar aprendizaje"
};

int NxPlanning_Create(const char* root, const char* goal, NxPlanCreateResult* out)
{
    if (out != NULL) memset(out, 0, sizeof(*out));
    if (goal == NULL || goal[0] == '\0') return 0;

    char id[NX_PLAN_MAX_ID];
    nx_normalize_id(goal, id, sizeof(id));

    char path[NX_PLAN_MAX_PATH];
    nx_plan_path(root, id, path, sizeof(path));

    FILE* f = fopen(path, "wb");
    if (f == NULL) return 0;

    time_t now = time(NULL);
    fprintf(f, "{\"type\":\"plan\",\"id\":\"%s\",\"goal\":\"%s\",\"status\":\"active\",\"created\":%lld}\n", id, goal, (long long)now);
    for (int i = 0; i < 5; ++i) {
        fprintf(f, "{\"type\":\"step\",\"index\":%d,\"status\":\"pending\",\"title\":\"%s\"}\n", i + 1, nx_default_steps[i]);
    }
    fclose(f);

    if (!nx_write_active(root, id)) return 0;

    if (out != NULL) {
        out->ok = 1;
        nx_copy(out->id, sizeof(out->id), id);
        nx_copy(out->goal, sizeof(out->goal), goal);
        nx_copy(out->path, sizeof(out->path), path);
        out->step_count = 5;
    }
    return 1;
}

static int nx_parse_json_string(const char* line, const char* key, char* out, size_t out_size)
{
    char pattern[64];
    (void)snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    const char* p = strstr(line, pattern);
    if (p == NULL) return 0;
    p += strlen(pattern);
    size_t j = 0;
    while (*p != '\0' && *p != '"' && j + 1 < out_size) out[j++] = *p++;
    out[j] = '\0';
    return j > 0;
}

static int nx_parse_json_int(const char* line, const char* key)
{
    char pattern[64];
    (void)snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    const char* p = strstr(line, pattern);
    if (p == NULL) return 0;
    p += strlen(pattern);
    return atoi(p);
}

int NxPlanning_Status(const char* root, NxPlanStatus* out)
{
    if (out == NULL) return 0;
    memset(out, 0, sizeof(*out));

    char id[NX_PLAN_MAX_ID];
    if (!nx_read_active(root, id, sizeof(id))) return 0;

    char path[NX_PLAN_MAX_PATH];
    nx_plan_path(root, id, path, sizeof(path));
    FILE* f = fopen(path, "rb");
    if (f == NULL) return 0;

    out->exists = 1;
    nx_copy(out->id, sizeof(out->id), id);
    nx_copy(out->path, sizeof(out->path), path);

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strstr(line, "\"type\":\"plan\"") != NULL) {
            (void)nx_parse_json_string(line, "goal", out->goal, sizeof(out->goal));
        } else if (strstr(line, "\"type\":\"step\"") != NULL) {
            if (out->step_count < NX_PLAN_MAX_STEPS) {
                NxPlanStep* s = &out->steps[out->step_count++];
                s->index = nx_parse_json_int(line, "index");
                (void)nx_parse_json_string(line, "title", s->title, sizeof(s->title));
                (void)nx_parse_json_string(line, "status", s->status, sizeof(s->status));
                if (strcmp(s->status, "done") == 0) out->completed_count++;
            }
        }
    }
    fclose(f);
    return 1;
}

int NxPlanning_CompleteStep(const char* root, int step_index, NxPlanStatus* out)
{
    char id[NX_PLAN_MAX_ID];
    if (!nx_read_active(root, id, sizeof(id))) return 0;
    char path[NX_PLAN_MAX_PATH];
    nx_plan_path(root, id, path, sizeof(path));
    FILE* f = fopen(path, "ab");
    if (f == NULL) return 0;
    fprintf(f, "{\"type\":\"step_update\",\"index\":%d,\"status\":\"done\"}\n", step_index);
    fclose(f);
    return NxPlanning_Status(root, out);
}

int NxPlanning_AddNote(const char* root, const char* note, NxPlanStatus* out)
{
    char id[NX_PLAN_MAX_ID];
    if (!nx_read_active(root, id, sizeof(id))) return 0;
    char path[NX_PLAN_MAX_PATH];
    nx_plan_path(root, id, path, sizeof(path));
    FILE* f = fopen(path, "ab");
    if (f == NULL) return 0;
    fprintf(f, "{\"type\":\"note\",\"text\":\"%s\"}\n", note == NULL ? "" : note);
    fclose(f);
    return NxPlanning_Status(root, out);
}
