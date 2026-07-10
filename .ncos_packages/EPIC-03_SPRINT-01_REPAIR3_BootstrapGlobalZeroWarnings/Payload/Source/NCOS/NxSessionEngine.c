#include "Nexiora/NCOS/NxSessionEngine.h"

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static void nx_zero_info(NxSessionInfo* info)
{
    if (info) memset(info, 0, sizeof(*info));
}

static int nx_is_empty(const char* s)
{
    return s == NULL || *s == '\0';
}

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t la, lb, ls, total;
    char tmp[1024];
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) return;
    la = strlen(a); lb = strlen(b);
    if (la > 0U && a[la - 1U] != '/' && a[la - 1U] != '\\') sep = "/";
    ls = strlen(sep);
    if (la > SIZE_MAX - ls || la + ls > SIZE_MAX - lb) return;
    total = la + ls + lb;
    if (total + 1U > (size_t)dst_size || total + 1U > sizeof(tmp)) return;
    if (la > 0U) memcpy(tmp, a, la);
    if (ls > 0U) memcpy(tmp + la, sep, ls);
    if (lb > 0U) memcpy(tmp + la + ls, b, lb);
    tmp[total] = '\0';
    memmove(dst, tmp, total + 1U);
    return;
}

static void nx_safe_name(char* dst, size_t dst_size, const char* src)
{
    size_t w = 0;
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "session";
    for (size_t i = 0; src[i] != '\0' && w + 1 < dst_size; ++i)
    {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c)) dst[w++] = (char)tolower(c);
        else if (c == '-' || c == '_' || c == '.') dst[w++] = (char)c;
        else if (c == ' ' && w > 0 && dst[w - 1] != '_') dst[w++] = '_';
    }
    if (w == 0) { nx_copy(dst, dst_size, "session"); return; }
    dst[w] = '\0';
}

static void nx_timestamp(char* dst, size_t dst_size)
{
    time_t now = time(NULL);
    struct tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    (void)strftime(dst, dst_size, "%Y-%m-%dT%H:%M:%S", &tmv);
}

static void nx_ensure_dir(const char* path)
{
    if (path != NULL && *path != '\0') (void)NX_MKDIR(path);
}

static void nx_session_base(char* dst, size_t dst_size, const char* root)
{
    char knowledge[512] = {0};
    char ncos[512] = {0};
    /* Ensure the root directory exists before creating nested Knowledge/NCOS paths.
       Without this, tests using a fresh root such as .nexiora_test_session_engine
       fail on Windows/Linux because mkdir("root/Knowledge") cannot create the
       missing parent directory. */
    nx_ensure_dir(root);
    nx_join(knowledge, sizeof(knowledge), root, "Knowledge");
    nx_join(ncos, sizeof(ncos), knowledge, "NCOS");
    nx_join(dst, dst_size, ncos, "Sessions");
    nx_ensure_dir(knowledge);
    nx_ensure_dir(ncos);
    nx_ensure_dir(dst);
}

static void nx_active_path(char* dst, size_t dst_size, const char* root)
{
    char base[512];
    nx_session_base(base, sizeof(base), root);
    nx_join(dst, dst_size, base, "active_session.txt");
}

static int nx_write_active(const char* root, const char* name)
{
    char path[512];
    nx_active_path(path, sizeof(path), root);
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "%s\n", name ? name : "");
    fclose(f);
    return 1;
}

static int nx_read_active(const char* root, char* name, size_t name_size)
{
    char path[512];
    nx_active_path(path, sizeof(path), root);
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    if (!fgets(name, (int)name_size, f)) { fclose(f); return 0; }
    fclose(f);
    name[strcspn(name, "\r\n")] = '\0';
    return name[0] != '\0';
}

static void nx_session_path(char* dst, size_t dst_size, const char* root, const char* name)
{
    char base[512];
    char safe[128];
    char filename[160];
    nx_session_base(base, sizeof(base), root);
    nx_safe_name(safe, sizeof(safe), name);
    (void)snprintf(filename, sizeof(filename), "%s.jsonl", safe);
    filename[sizeof(filename)-1] = '\0';
    nx_join(dst, dst_size, base, filename);
}

static int nx_append_event(const char* path, const char* kind, const char* text)
{
    char ts[64];
    nx_timestamp(ts, sizeof(ts));
    FILE* f = fopen(path, "ab");
    if (!f) return 0;
    fprintf(f, "{\"time\":\"%s\",\"kind\":\"%s\",\"text\":\"", ts, kind ? kind : "event");
    if (text)
    {
        for (const char* p = text; *p; ++p)
        {
            if (*p == '"' || *p == '\\') fputc('\\', f);
            if (*p == '\n' || *p == '\r') fputc(' ', f);
            else fputc(*p, f);
        }
    }
    fprintf(f, "\"}\n");
    fclose(f);
    return 1;
}

static int nx_count_lines(const char* path)
{
    int count = 0;
    int c;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    while ((c = fgetc(f)) != EOF) if (c == '\n') ++count;
    fclose(f);
    return count;
}

NxSessionResult NxSession_Start(const char* root, const char* name, const char* goal, NxSessionInfo* out_info)
{
    char safe_name[128];
    char path[512];
    if (nx_is_empty(root) || nx_is_empty(name)) return NX_SESSION_INVALID_ARGUMENT;
    nx_zero_info(out_info);
    nx_safe_name(safe_name, sizeof(safe_name), name);
    nx_session_path(path, sizeof(path), root, safe_name);
    if (!nx_append_event(path, "start", goal && *goal ? goal : "Sesion iniciada")) return NX_SESSION_IO_ERROR;
    if (!nx_write_active(root, safe_name)) return NX_SESSION_IO_ERROR;
    if (out_info)
    {
        nx_copy(out_info->name, sizeof(out_info->name), safe_name);
        nx_copy(out_info->path, sizeof(out_info->path), path);
        nx_copy(out_info->goal, sizeof(out_info->goal), goal ? goal : "");
        nx_copy(out_info->status, sizeof(out_info->status), "active");
        out_info->event_count = nx_count_lines(path);
    }
    return NX_SESSION_OK;
}

NxSessionResult NxSession_AddNote(const char* root, const char* note, NxSessionInfo* out_info)
{
    char name[128];
    char path[512];
    if (nx_is_empty(root) || nx_is_empty(note)) return NX_SESSION_INVALID_ARGUMENT;
    nx_zero_info(out_info);
    if (!nx_read_active(root, name, sizeof(name))) return NX_SESSION_NOT_FOUND;
    nx_session_path(path, sizeof(path), root, name);
    if (!nx_append_event(path, "note", note)) return NX_SESSION_IO_ERROR;
    if (out_info)
    {
        nx_copy(out_info->name, sizeof(out_info->name), name);
        nx_copy(out_info->path, sizeof(out_info->path), path);
        nx_copy(out_info->status, sizeof(out_info->status), "active");
        out_info->event_count = nx_count_lines(path);
    }
    return NX_SESSION_OK;
}

NxSessionResult NxSession_Status(const char* root, NxSessionInfo* out_info)
{
    char name[128];
    char path[512];
    if (nx_is_empty(root)) return NX_SESSION_INVALID_ARGUMENT;
    nx_zero_info(out_info);
    if (!nx_read_active(root, name, sizeof(name))) return NX_SESSION_NOT_FOUND;
    nx_session_path(path, sizeof(path), root, name);
    if (out_info)
    {
        nx_copy(out_info->name, sizeof(out_info->name), name);
        nx_copy(out_info->path, sizeof(out_info->path), path);
        nx_copy(out_info->status, sizeof(out_info->status), "active");
        out_info->event_count = nx_count_lines(path);
    }
    return NX_SESSION_OK;
}

NxSessionResult NxSession_Close(const char* root, NxSessionInfo* out_info)
{
    char name[128];
    char path[512];
    if (nx_is_empty(root)) return NX_SESSION_INVALID_ARGUMENT;
    nx_zero_info(out_info);
    if (!nx_read_active(root, name, sizeof(name))) return NX_SESSION_NOT_FOUND;
    nx_session_path(path, sizeof(path), root, name);
    if (!nx_append_event(path, "close", "Sesion cerrada")) return NX_SESSION_IO_ERROR;
    if (!nx_write_active(root, "")) return NX_SESSION_IO_ERROR;
    if (out_info)
    {
        nx_copy(out_info->name, sizeof(out_info->name), name);
        nx_copy(out_info->path, sizeof(out_info->path), path);
        nx_copy(out_info->status, sizeof(out_info->status), "closed");
        out_info->event_count = nx_count_lines(path);
    }
    return NX_SESSION_OK;
}

const char* NxSession_ResultName(NxSessionResult result)
{
    switch (result)
    {
        case NX_SESSION_OK: return "OK";
        case NX_SESSION_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_SESSION_IO_ERROR: return "IO_ERROR";
        case NX_SESSION_NOT_FOUND: return "NOT_FOUND";
        default: return "UNKNOWN";
    }
}
