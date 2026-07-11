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

static int nx_replace_when_available(const char* current_path, const char* next_path)
{
    unsigned int attempt;
    for (attempt = 0U; attempt < 300U; ++attempt) {
        if (MoveFileExA(next_path, current_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            return 0;
        }
        Sleep(200U);
    }
    return 3;
}

static int nx_schedule(const char* current_path, const char* next_path)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char self_path[MAX_PATH];
    char quoted_self[MAX_PATH * 2U];
    char quoted_current[MAX_PATH * 2U];
    char quoted_next[MAX_PATH * 2U];
    char command[MAX_PATH * 6U];
    DWORD length;
    int written;

    length = GetModuleFileNameA(NULL, self_path, (DWORD)sizeof(self_path));
    if (length == 0U || length >= sizeof(self_path)) return 4;
    if (!nx_quote(quoted_self, sizeof(quoted_self), self_path) ||
        !nx_quote(quoted_current, sizeof(quoted_current), current_path) ||
        !nx_quote(quoted_next, sizeof(quoted_next), next_path)) return 5;

    written = snprintf(command, sizeof(command), "%s --replace %s %s", quoted_self, quoted_current, quoted_next);
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
    if (argc == 4 && strcmp(argv[1], "--schedule") == 0) {
        return nx_schedule(argv[2], argv[3]);
    }
    if (argc == 4 && strcmp(argv[1], "--replace") == 0) {
        return nx_replace_when_available(argv[2], argv[3]);
    }
    fputs("Usage: nexiora_self_update --schedule <current.exe> <next.exe>\n", stderr);
    return 2;
#else
    (void)argc;
    (void)argv;
    return 0;
#endif
}
