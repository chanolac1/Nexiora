#include "Nexiora/NCOS/NxPackageApply.h"
#include "Nexiora/NCOS/NxPackageManager.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/wait.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

static int nx_path_exists(const char* path)
{
    struct stat st;
    return path != NULL && stat(path, &st) == 0;
}

static int nx_make_dirs(const char* path)
{
    char tmp[512];
    size_t i;
    size_t n;
    if (path == NULL) return 0;
    n = strlen(path);
    if (n == 0U || n >= sizeof(tmp)) return 0;
    memcpy(tmp, path, n + 1U);
    for (i = 1U; i < n; ++i) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char saved = tmp[i];
            tmp[i] = '\0';
            if (tmp[0] != '\0' && !(strlen(tmp) == 2U && tmp[1] == ':')) {
                if (NX_MKDIR(tmp) != 0 && errno != EEXIST) return 0;
            }
            tmp[i] = saved;
        }
    }
    if (NX_MKDIR(tmp) != 0 && errno != EEXIST) return 0;
    return 1;
}

static int nx_join(char* out, size_t cap, const char* a, const char* b)
{
    int written;
    if (out == NULL || cap == 0U || a == NULL || b == NULL) return 0;
    written = snprintf(out, cap, "%s/%s", a, b);
    return written >= 0 && (size_t)written < cap;
}

static int nx_append(FILE* out, const char* text)
{
    if (out == NULL || text == NULL) return 0;
    return fputs(text, out) >= 0;
}

#if defined(_WIN32)
static int nx_run_process(const char* working_dir,
                          const char* executable,
                          const char* arguments,
                          const char* log_path)
{
    SECURITY_ATTRIBUTES sa;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    HANDLE log_handle;
    DWORD exit_code = 1U;
    char command_line[4096];
    int written;

    written = snprintf(command_line, sizeof(command_line), "\"%s\" %s", executable, arguments != NULL ? arguments : "");
    if (written < 0 || (size_t)written >= sizeof(command_line)) return 0;

    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    log_handle = CreateFileA(log_path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (log_handle == INVALID_HANDLE_VALUE) return 0;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = log_handle;
    si.hStdError = log_handle;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcessA(NULL, command_line, NULL, NULL, TRUE, 0U, NULL, working_dir, &si, &pi)) {
        CloseHandle(log_handle);
        return 0;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    (void)GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(log_handle);
    return exit_code == 0U;
}
#else
static int nx_run_process(const char* working_dir,
                          const char* executable,
                          const char* arguments,
                          const char* log_path)
{
    char command[4096];
    int written;
    int rc;
    written = snprintf(command, sizeof(command), "cd \"%s\" && \"%s\" %s >> \"%s\" 2>&1",
                       working_dir, executable, arguments != NULL ? arguments : "", log_path);
    if (written < 0 || (size_t)written >= sizeof(command)) return 0;
    rc = system(command);
    return rc != -1 && WIFEXITED(rc) && WEXITSTATUS(rc) == 0;
}
#endif

static int nx_contains_warning(const char* line)
{
    char lower[4096];
    size_t i;
    size_t n;
    if (line == NULL) return 0;
    n = strlen(line);
    if (n >= sizeof(lower)) n = sizeof(lower) - 1U;
    for (i = 0U; i < n; ++i) lower[i] = (char)tolower((unsigned char)line[i]);
    lower[n] = '\0';
    if (strstr(lower, "warning:") != NULL) return 1;
    if (strstr(lower, ": warning ") != NULL) return 1;
    if (strstr(lower, "cmake warning") != NULL) return 1;
    return 0;
}

int NxPackageManager_LogHasWarnings(const char* log_path, int* out_warning_count)
{
    FILE* f;
    char line[4096];
    int count = 0;
    if (out_warning_count != NULL) *out_warning_count = 0;
    if (log_path == NULL) return 0;
    f = fopen(log_path, "rb");
    if (f == NULL) return 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (nx_contains_warning(line)) ++count;
    }
    fclose(f);
    if (out_warning_count != NULL) *out_warning_count = count;
    return count > 0;
}

const char* NxPackageManager_ApplyPhaseName(NxPackageApplyPhase phase)
{
    switch (phase) {
        case NX_PACKAGE_APPLY_VERIFY: return "verify";
        case NX_PACKAGE_APPLY_DEPENDENCIES: return "dependencies";
        case NX_PACKAGE_APPLY_INSTALL: return "install";
        case NX_PACKAGE_APPLY_CONFIGURE: return "configure";
        case NX_PACKAGE_APPLY_BUILD: return "build";
        case NX_PACKAGE_APPLY_WARNING_GATE: return "warning-gate";
        case NX_PACKAGE_APPLY_TESTS: return "tests";
        case NX_PACKAGE_APPLY_DOCUMENTATION: return "documentation";
        case NX_PACKAGE_APPLY_COMPLETE: return "complete";
        default: return "none";
    }
}

static void nx_fail(NxPackageApplyResult* r, NxPackageApplyPhase phase, const char* message)
{
    if (r == NULL) return;
    (void)snprintf(r->failed_phase, sizeof(r->failed_phase), "%s", NxPackageManager_ApplyPhaseName(phase));
    (void)snprintf(r->message, sizeof(r->message), "%s", message != NULL ? message : "Apply failed.");
}

static void nx_log_phase(FILE* log, NxPackageApplyPhase phase)
{
    char line[128];
    int written = snprintf(line, sizeof(line), "\n=== phase: %s ===\n", NxPackageManager_ApplyPhaseName(phase));
    if (written > 0 && (size_t)written < sizeof(line)) (void)nx_append(log, line);
}

int NxPackageManager_Apply(const char* repo_root,
                           const char* package_dir,
                           NxPackageApplyResult* out_result)
{
    NxPackageVerifyResult verify;
    NxPackageVerifyResult deps;
    NxPackageInstallResult install;
    NxPackageInstallResult rollback;
    NxPackageApplyResult result;
    NxPackageApplyPhase phase = NX_PACKAGE_APPLY_NONE;
    char apply_dir[512];
    char log_path[512];
    char docs_exe[512];
    FILE* log = NULL;
    int warnings = 0;

    memset(&result, 0, sizeof(result));
    memset(&verify, 0, sizeof(verify));
    memset(&deps, 0, sizeof(deps));
    memset(&install, 0, sizeof(install));
    memset(&rollback, 0, sizeof(rollback));
    if (out_result != NULL) memset(out_result, 0, sizeof(*out_result));
    if (repo_root == NULL || package_dir == NULL || out_result == NULL) return 0;

    if (!nx_join(apply_dir, sizeof(apply_dir), repo_root, "Knowledge/NCOS/PackageApply")) return 0;
    if (!nx_make_dirs(apply_dir)) return 0;
    if (!nx_join(log_path, sizeof(log_path), apply_dir, "apply.log")) return 0;
    (void)snprintf(result.apply_log_path, sizeof(result.apply_log_path), "%s", log_path);
    log = fopen(log_path, "wb");
    if (log == NULL) return 0;

    phase = NX_PACKAGE_APPLY_VERIFY;
    nx_log_phase(log, phase);
    if (!NxPackageManager_VerifyPackage(package_dir, &verify)) {
        nx_fail(&result, phase, verify.message);
        goto done;
    }
    result.verify_ok = 1;
    (void)snprintf(result.package_id, sizeof(result.package_id), "%s", verify.package_id);
    (void)snprintf(result.package_version, sizeof(result.package_version), "%s", verify.package_version);

    phase = NX_PACKAGE_APPLY_DEPENDENCIES;
    nx_log_phase(log, phase);
    if (!NxPackageManager_VerifyDependencies(repo_root, package_dir, &deps)) {
        nx_fail(&result, phase, deps.message);
        goto done;
    }
    result.dependencies_ok = 1;

    phase = NX_PACKAGE_APPLY_INSTALL;
    nx_log_phase(log, phase);
    if (!NxPackageManager_Install(repo_root, package_dir, &install)) {
        nx_fail(&result, phase, "Transactional installation failed.");
        goto done;
    }
    result.install_ok = 1;
    (void)snprintf(result.transaction_id, sizeof(result.transaction_id), "%s", install.transaction_id);

    phase = NX_PACKAGE_APPLY_CONFIGURE;
    nx_log_phase(log, phase);
    fclose(log); log = NULL;
    if (!nx_run_process(repo_root, "cmake", "--preset windows-msvc-release", log_path)) {
        nx_fail(&result, phase, "CMake configuration failed.");
        goto rollback;
    }
    result.configure_ok = 1;

    phase = NX_PACKAGE_APPLY_BUILD;
    log = fopen(log_path, "ab"); if (log != NULL) { nx_log_phase(log, phase); fclose(log); log = NULL; }
    if (!nx_run_process(repo_root, "cmake", "--build --preset release --target nexiora_test_suite", log_path)) {
        nx_fail(&result, phase, "Full test-suite materialization failed.");
        goto rollback;
    }
    result.build_ok = 1;

    phase = NX_PACKAGE_APPLY_WARNING_GATE;
    if (!NxPackageManager_LogHasWarnings(log_path, &warnings)) {
        result.warning_gate_ok = 1;
    } else {
        char message[256];
        (void)snprintf(message, sizeof(message), "Build log contains %d warning(s).", warnings);
        nx_fail(&result, phase, message);
        goto rollback;
    }

    phase = NX_PACKAGE_APPLY_TESTS;
    log = fopen(log_path, "ab"); if (log != NULL) { nx_log_phase(log, phase); fclose(log); log = NULL; }
    if (!nx_run_process(repo_root, "ctest", "--test-dir Build/windows-msvc-release --output-on-failure", log_path)) {
        nx_fail(&result, phase, "CTest suite failed.");
        goto rollback;
    }
    result.tests_ok = 1;

    phase = NX_PACKAGE_APPLY_DOCUMENTATION;
    if (!nx_join(docs_exe, sizeof(docs_exe), repo_root, "Build/windows-msvc-release/bin/nexiora_docs.exe")) {
        nx_fail(&result, phase, "Documentation validator path is too long.");
        goto rollback;
    }
    if (!nx_path_exists(docs_exe)) {
        nx_fail(&result, phase, "Documentation validator executable is missing.");
        goto rollback;
    }
    if (!nx_run_process(repo_root, docs_exe, "validate .", log_path)) {
        nx_fail(&result, phase, "Documentation synchronization validation failed.");
        goto rollback;
    }
    result.documentation_ok = 1;

    result.success = 1;
    (void)snprintf(result.message, sizeof(result.message), "%s", "Package applied and certified successfully.");
    (void)snprintf(result.failed_phase, sizeof(result.failed_phase), "%s", "none");
    goto done;

rollback:
    if (install.transaction_id[0] != '\0' && install.package_id[0] != '\0') {
        if (NxPackageManager_RollbackTransaction(repo_root, install.package_id, install.transaction_id, &rollback)) {
            result.rollback_performed = 1;
        }
    }

done:
    if (log != NULL) fclose(log);
    *out_result = result;
    return result.success;
}
