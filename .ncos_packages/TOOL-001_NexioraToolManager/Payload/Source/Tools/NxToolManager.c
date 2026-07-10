#include "Nexiora/Tools/NxToolManager.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

static int nx_copy(char* dst, size_t cap, const char* src) {
    const size_t n = src == NULL ? 0U : strlen(src);
    if (dst == NULL || cap == 0U || src == NULL || n >= cap) return 0;
    memcpy(dst, src, n + 1U);
    return 1;
}

static int nx_join(char* dst, size_t cap, const char* a, const char* b) {
    size_t na;
    size_t nb;
    int sep;
    if (dst == NULL || cap == 0U || a == NULL || b == NULL) return 0;
    na = strlen(a); nb = strlen(b);
    sep = na > 0U && a[na - 1U] != '/' && a[na - 1U] != '\\';
    if (na + (sep ? 1U : 0U) + nb >= cap) return 0;
    memcpy(dst, a, na);
    if (sep) dst[na++] = '/';
    memcpy(dst + na, b, nb + 1U);
    return 1;
}

static int nx_exists(const char* path) {
    struct stat st;
    return path != NULL && stat(path, &st) == 0;
}

static int nx_mkdir_if_needed(const char* path) {
    if (nx_exists(path)) return 1;
    return NX_MKDIR(path) == 0;
}

static int nx_registry_path(char* out, size_t cap, const char* root, const char* id) {
    char tools[512]; char registry[512]; char filename[128];
    const int n = snprintf(filename, sizeof(filename), "%s.registry", id);
    if (n < 0 || (size_t)n >= sizeof(filename)) return 0;
    return nx_join(tools, sizeof(tools), root, "Tools") &&
           nx_join(registry, sizeof(registry), tools, "Registry") &&
           nx_join(out, cap, registry, filename);
}

static int nx_read_value(char* dst, size_t cap, const char* line, const char* key) {
    const size_t nk = strlen(key);
    size_t n;
    if (strncmp(line, key, nk) != 0 || line[nk] != '=') return 0;
    line += nk + 1U;
    n = strcspn(line, "\r\n");
    if (n >= cap) return -1;
    memcpy(dst, line, n); dst[n] = '\0';
    return 1;
}

NxToolStatus NxToolManager_GetRecord(const char* root, const char* tool_id, NxToolRecord* out_record) {
    char path[512]; char line[1200]; FILE* f;
    if (root == NULL || tool_id == NULL || out_record == NULL || tool_id[0] == '\0') return NX_TOOL_INVALID_ARGUMENT;
    memset(out_record, 0, sizeof(*out_record));
    if (!nx_registry_path(path, sizeof(path), root, tool_id)) return NX_TOOL_INVALID_ARGUMENT;
    f = fopen(path, "rb"); if (f == NULL) return NX_TOOL_NOT_FOUND;
    while (fgets(line, sizeof(line), f) != NULL) {
        (void)nx_read_value(out_record->id, sizeof(out_record->id), line, "id");
        (void)nx_read_value(out_record->version, sizeof(out_record->version), line, "version");
        (void)nx_read_value(out_record->executable, sizeof(out_record->executable), line, "executable");
        (void)nx_read_value(out_record->sha256, sizeof(out_record->sha256), line, "sha256");
        (void)nx_read_value(out_record->source_url, sizeof(out_record->source_url), line, "source_url");
    }
    if (fclose(f) != 0) return NX_TOOL_IO_ERROR;
    if (out_record->id[0] == '\0' || out_record->executable[0] == '\0' || out_record->sha256[0] == '\0') return NX_TOOL_IO_ERROR;
    return NX_TOOL_OK;
}

NxToolStatus NxToolManager_Verify(const char* root, const char* tool_id, NxToolRecord* out_record) {
    NxToolRecord local;
    NxToolStatus s = NxToolManager_GetRecord(root, tool_id, &local);
    if (s != NX_TOOL_OK) return s;
    if (!nx_exists(local.executable)) return NX_TOOL_NOT_FOUND;
    if (strlen(local.sha256) != 64U) return NX_TOOL_INTEGRITY_ERROR;
    if (out_record != NULL) *out_record = local;
    return NX_TOOL_OK;
}

NxToolStatus NxToolManager_Register(const char* root, const NxToolRecord* record) {
    char tools[512]; char registry[512]; char path[512]; char temp[520]; FILE* f;
    if (root == NULL || record == NULL || record->id[0] == '\0' || record->executable[0] == '\0' || strlen(record->sha256) != 64U) return NX_TOOL_INVALID_ARGUMENT;
    if (!nx_join(tools, sizeof(tools), root, "Tools") || !nx_join(registry, sizeof(registry), tools, "Registry")) return NX_TOOL_INVALID_ARGUMENT;
    if (!nx_mkdir_if_needed(tools) || !nx_mkdir_if_needed(registry) || !nx_registry_path(path, sizeof(path), root, record->id)) return NX_TOOL_IO_ERROR;
    if (!nx_copy(temp, sizeof(temp), path) || strlen(temp) + 4U >= sizeof(temp)) return NX_TOOL_INVALID_ARGUMENT;
    strcat(temp, ".tmp");
    f = fopen(temp, "wb"); if (f == NULL) return NX_TOOL_IO_ERROR;
    if (fprintf(f, "id=%s\nversion=%s\nexecutable=%s\nsha256=%s\nsource_url=%s\n", record->id, record->version, record->executable, record->sha256, record->source_url) < 0 || fclose(f) != 0) { (void)remove(temp); return NX_TOOL_IO_ERROR; }
    (void)remove(path);
    if (rename(temp, path) != 0) { (void)remove(temp); return NX_TOOL_IO_ERROR; }
    return NX_TOOL_OK;
}

NxToolStatus NxToolManager_Remove(const char* root, const char* tool_id) {
    NxToolRecord r; char path[512]; NxToolStatus s = NxToolManager_GetRecord(root, tool_id, &r);
    if (s != NX_TOOL_OK) return s;
    if (nx_exists(r.executable) && remove(r.executable) != 0) return NX_TOOL_IO_ERROR;
    if (!nx_registry_path(path, sizeof(path), root, tool_id) || remove(path) != 0) return NX_TOOL_IO_ERROR;
    return NX_TOOL_OK;
}

const char* NxToolManager_StatusName(NxToolStatus status) {
    switch (status) {
        case NX_TOOL_OK: return "OK";
        case NX_TOOL_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_TOOL_NOT_FOUND: return "NOT_FOUND";
        case NX_TOOL_IO_ERROR: return "IO_ERROR";
        case NX_TOOL_INTEGRITY_ERROR: return "INTEGRITY_ERROR";
        case NX_TOOL_UNSUPPORTED: return "UNSUPPORTED";
        default: return "UNKNOWN";
    }
}
