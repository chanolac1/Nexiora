#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>

static int nx_quote(char* out, size_t cap, const char* value)
{
    int written;
    if (out == NULL || cap == 0U || value == NULL) return 0;
    written = snprintf(out, cap, "\"%s\"", value);
    return written >= 0 && (size_t)written < cap;
}

static int nx_log_has_completion(const char* log_path)
{
    FILE* file;
    char line[1024];
    if (log_path == NULL) return 0;
    file = fopen(log_path, "rb");
    if (file == NULL) return 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, "phase=apply state=completed") != NULL ||
            strstr(line, "Package applied and certified successfully") != NULL) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

static int nx_replace_after_success(const char* current_path,
                                    const char* next_path,
                                    const char* log_path)
{
    unsigned int attempt;
    for (attempt = 0U; attempt < 3000U; ++attempt) {
        if (!nx_log_has_completion(log_path)) {
            Sleep(200U);
            continue;
        }
        if (MoveFileExA(next_path, current_path,
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            return 0;
        }
        Sleep(200U);
    }
    return 3;
}


static int nx_replace_now(const char* current_path, const char* next_path)
{
    if (current_path == NULL || next_path == NULL) return 8;
    if (!MoveFileExA(next_path, current_path,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return 9;
    return 0;
}
static int nx_schedule(const char* current_path,
                       const char* next_path,
                       const char* log_path)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char self_path[MAX_PATH];
    char quoted_self[MAX_PATH * 2U];
    char quoted_current[MAX_PATH * 2U];
    char quoted_next[MAX_PATH * 2U];
    char quoted_log[MAX_PATH * 2U];
    char command[MAX_PATH * 9U];
    DWORD length;
    int written;

    length = GetModuleFileNameA(NULL, self_path, (DWORD)sizeof(self_path));
    if (length == 0U || length >= sizeof(self_path)) return 4;
    if (!nx_quote(quoted_self, sizeof(quoted_self), self_path) ||
        !nx_quote(quoted_current, sizeof(quoted_current), current_path) ||
        !nx_quote(quoted_next, sizeof(quoted_next), next_path) ||
        !nx_quote(quoted_log, sizeof(quoted_log), log_path)) return 5;

    written = snprintf(command, sizeof(command), "%s --replace-after-success %s %s %s",
                       quoted_self, quoted_current, quoted_next, quoted_log);
    if (written < 0 || (size_t)written >= sizeof(command)) return 6;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));
    if (!CreateProcessA(NULL, command, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW | DETACHED_PROCESS,
                        NULL, NULL, &si, &pi)) return 7;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
#endif

int main(int argc, char** argv)
{
#if defined(_WIN32)
    if (argc == 5 && strcmp(argv[1], "--schedule") == 0) {
        return nx_schedule(argv[2], argv[3], argv[4]);
    }
    if (argc == 5 && strcmp(argv[1], "--replace-after-success") == 0) {
        return nx_replace_after_success(argv[2], argv[3], argv[4]);
    }
    if (argc == 4 && strcmp(argv[1], "--replace-now") == 0) {
        return nx_replace_now(argv[2], argv[3]);
    }
    fputs("Usage: nexiora_self_update --schedule <current.exe> <next.exe> <apply.log>\n"
          "       nexiora_self_update --replace-now <current.exe> <next.exe>\n", stderr);
    return 2;
#else
    (void)argc;
    (void)argv;
    return 0;
#endif
}
