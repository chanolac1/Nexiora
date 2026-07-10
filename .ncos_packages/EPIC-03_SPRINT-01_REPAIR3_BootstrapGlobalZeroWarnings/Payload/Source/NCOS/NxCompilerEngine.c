#include "Nexiora/NCOS/NxCompilerEngine.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define nx_popen _popen
#define nx_pclose _pclose
#define NX_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <sys/types.h>
#define nx_popen popen
#define nx_pclose pclose
#define NX_PATH_SEP "/"
#endif

static int nx_mkdir_one(const char* path)
{
    if (path == NULL || path[0] == '\0') return 0;
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(path, 0777) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t la, lb, ls, total;
    char tmp[1024];
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) return 0;
    la = strlen(a); lb = strlen(b);
    if (la > 0U && a[la - 1U] != '/' && a[la - 1U] != '\\') sep = NX_PATH_SEP;
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

static void nx_make_safe_id(char* dst, size_t dst_size, const char* src)
{
    size_t i;
    size_t j = 0;
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL || src[0] == '\0') src = "compiler_run";
    for (i = 0; src[i] != '\0' && j + 1 < dst_size; ++i)
    {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c)) dst[j++] = (char)tolower(c);
        else if (c == '-' || c == '_' || c == '.') dst[j++] = (char)c;
        else if (j > 0 && dst[j - 1] != '_') dst[j++] = '_';
    }
    if (j == 0) dst[j++] = 'r';
    dst[j] = '\0';
}

static int nx_ensure_compiler_dirs(const char* root, char* out_dir, size_t out_dir_size)
{
    char knowledge[512];
    char ncos[512];

    if (root == NULL || root[0] == '\0') root = ".";

    /* Important: tests and callers may provide a fresh sandbox root.
       Create the root first before creating nested Knowledge/NCOS/Compiler. */
    if (!nx_mkdir_one(root)) return 0;

    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    if (!nx_mkdir_one(knowledge)) return 0;
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    if (!nx_mkdir_one(ncos)) return 0;
    if (!nx_join(out_dir, out_dir_size, ncos, "Compiler")) return 0;
    if (!nx_mkdir_one(out_dir)) return 0;
    return 1;
}

static int nx_contains_word_ci(const char* text, const char* needle)
{
    size_t n;
    if (text == NULL || needle == NULL) return 0;
    n = strlen(needle);
    if (n == 0) return 0;
    for (; *text; ++text)
    {
        size_t i = 0;
        while (i < n && text[i] && tolower((unsigned char)text[i]) == tolower((unsigned char)needle[i])) ++i;
        if (i == n) return 1;
    }
    return 0;
}

const char* NxCompiler_StatusText(NxCompilerRunStatus status)
{
    switch (status)
    {
        case NX_COMPILER_RUN_OK: return "OK";
        case NX_COMPILER_RUN_FAILED: return "FAILED";
        case NX_COMPILER_RUN_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_COMPILER_RUN_IO_ERROR: return "IO_ERROR";
        default: return "UNKNOWN";
    }
}

NxCompilerRunStatus NxCompiler_Run(const char* root,
                                    const char* command,
                                    const char* run_id,
                                    NxCompilerRunResult* out_result)
{
    char safe_id[128];
    char out_dir[512];
    char log_name[192];
    FILE* pipe;
    FILE* log;
    char buffer[512];
    int rc;

    if (out_result == NULL || command == NULL || command[0] == '\0') return NX_COMPILER_RUN_INVALID_ARGUMENT;
    memset(out_result, 0, sizeof(*out_result));
    out_result->status = NX_COMPILER_RUN_FAILED;
    out_result->exit_code = -1;
    nx_copy(out_result->command, sizeof(out_result->command), command);

    if (!nx_ensure_compiler_dirs(root, out_dir, sizeof(out_dir)))
    {
        nx_copy(out_result->summary, sizeof(out_result->summary), "No se pudo preparar Knowledge/NCOS/Compiler.");
        out_result->status = NX_COMPILER_RUN_IO_ERROR;
        return out_result->status;
    }

    nx_make_safe_id(safe_id, sizeof(safe_id), run_id);
    snprintf(log_name, sizeof(log_name), "%s.log", safe_id);
    if (!nx_join(out_result->log_path, sizeof(out_result->log_path), out_dir, log_name))
    {
        nx_copy(out_result->summary, sizeof(out_result->summary), "Ruta de log demasiado larga.");
        out_result->status = NX_COMPILER_RUN_IO_ERROR;
        return out_result->status;
    }

    log = fopen(out_result->log_path, "wb");
    if (log == NULL)
    {
        nx_copy(out_result->summary, sizeof(out_result->summary), "No se pudo crear el log de compilacion.");
        out_result->status = NX_COMPILER_RUN_IO_ERROR;
        return out_result->status;
    }

    fprintf(log, "NCOS Compiler Run\nCommand: %s\n\n", command);

    pipe = nx_popen(command, "r");
    if (pipe == NULL)
    {
        fprintf(log, "ERROR: no se pudo ejecutar el comando.\n");
        fclose(log);
        nx_copy(out_result->summary, sizeof(out_result->summary), "No se pudo ejecutar el comando.");
        out_result->status = NX_COMPILER_RUN_IO_ERROR;
        return out_result->status;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        size_t len = strlen(buffer);
        fwrite(buffer, 1, len, log);
        out_result->bytes_captured += len;
        if (nx_contains_word_ci(buffer, "error") || nx_contains_word_ci(buffer, "failed")) out_result->saw_error = 1;
        if (nx_contains_word_ci(buffer, "warning")) out_result->saw_warning = 1;
    }

    rc = nx_pclose(pipe);
    out_result->exit_code = rc;
    fprintf(log, "\nExitCode: %d\nWarnings: %d\nErrors: %d\n", out_result->exit_code, out_result->saw_warning, out_result->saw_error);
    fclose(log);

    if (rc == 0)
    {
        out_result->status = NX_COMPILER_RUN_OK;
        snprintf(out_result->summary, sizeof(out_result->summary), "Comando ejecutado correctamente. Log: %.217s", out_result->log_path);
    }
    else
    {
        out_result->status = NX_COMPILER_RUN_FAILED;
        snprintf(out_result->summary, sizeof(out_result->summary), "El comando fallo. Revisa el log: %.222s", out_result->log_path);
    }

    return out_result->status;
}
