#include "Nexiora/NCOS/NxPackageApply.h"
#include "Nexiora/NCOS/NxPackageManager.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif





static int nx_join(char* out, size_t cap, const char* a, const char* b)
{
    int written;
    if (out == NULL || cap == 0U || a == NULL || b == NULL) return 0;
    written = snprintf(out, cap, "%s/%s", a, b);
    return written >= 0 && (size_t)written < cap;
}





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
        case NX_PACKAGE_APPLY_DISCOVERY: return "discovery";
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







static int nx_manifest_has_pure_apply_contract(const char* package_dir)
{
    char manifest[512];
    FILE* f;
    char line[1024];
    int has_configure = 0;
    int has_build = 0;
    int has_target = 0;
    int has_test = 0;
    if (!nx_join(manifest, sizeof(manifest), package_dir, "manifest.npkg")) return 0;
    f = fopen(manifest, "rb");
    if (f == NULL) return 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strncmp(line, "apply.configure_preset=", 23U) == 0 && line[23] != '\0' && line[23] != '\r' && line[23] != '\n') has_configure = 1;
        else if (strncmp(line, "apply.build_preset=", 19U) == 0 && line[19] != '\0' && line[19] != '\r' && line[19] != '\n') has_build = 1;
        else if (strncmp(line, "apply.test_target=", 18U) == 0 && line[18] != '\0' && line[18] != '\r' && line[18] != '\n') has_target = 1;
        else if (strncmp(line, "apply.test_preset=", 18U) == 0 && line[18] != '\0' && line[18] != '\r' && line[18] != '\n') has_test = 1;
    }
    fclose(f);
    return has_configure && has_build && has_target && has_test;
}

static int nx_manifest_mtime(const char* package_dir, time_t* out_value)
{
    char manifest[512];
    struct stat st;
    if (out_value != NULL) *out_value = (time_t)0;
    if (!nx_join(manifest, sizeof(manifest), package_dir, "manifest.npkg")) return 0;
    if (stat(manifest, &st) != 0) return 0;
    if (out_value != NULL) *out_value = st.st_mtime;
    return 1;
}

static int nx_candidate_is_better(time_t candidate_time,
                                  const char* candidate_path,
                                  time_t selected_time,
                                  const char* selected_path)
{
    if (selected_path == NULL || selected_path[0] == '\0') return 1;
    if (candidate_time > selected_time) return 1;
    if (candidate_time < selected_time) return 0;
    return strcmp(candidate_path, selected_path) > 0;
}

static void nx_consider_package(const char* repo_root,
                                const char* package_dir,
                                NxPackageDiscoveryResult* result,
                                time_t* selected_time)
{
    NxPackageVerifyResult verify;
    NxPackageVerifyResult deps;
    NxPackageInstallResult status;
    time_t manifest_time = (time_t)0;

    memset(&verify, 0, sizeof(verify));
    memset(&deps, 0, sizeof(deps));
    memset(&status, 0, sizeof(status));
    ++result->directories_scanned;

    if (!NxPackageManager_VerifyPackage(package_dir, &verify)) return;
    if (!nx_manifest_has_pure_apply_contract(package_dir)) {
        ++result->blocked_candidates;
        return;
    }
    if (NxPackageManager_Status(repo_root, verify.package_id, &status)) {
        ++result->installed_candidates;
        return;
    }
    if (!NxPackageManager_VerifyDependencies(repo_root, package_dir, &deps)) {
        ++result->blocked_candidates;
        return;
    }
    ++result->valid_candidates;
    (void)nx_manifest_mtime(package_dir, &manifest_time);
    if (!nx_candidate_is_better(manifest_time, package_dir, *selected_time, result->package_dir)) return;

    (void)snprintf(result->package_dir, sizeof(result->package_dir), "%s", package_dir);
    (void)snprintf(result->package_id, sizeof(result->package_id), "%s", verify.package_id);
    (void)snprintf(result->package_version, sizeof(result->package_version), "%s", verify.package_version);
    *selected_time = manifest_time;
    result->found = 1;
}

int NxPackageManager_DiscoverPendingPackage(const char* repo_root,
                                            NxPackageDiscoveryResult* out_result)
{
    NxPackageDiscoveryResult result;
    char packages_dir[512];
    time_t selected_time = (time_t)0;

    memset(&result, 0, sizeof(result));
    if (out_result != NULL) memset(out_result, 0, sizeof(*out_result));
    if (repo_root == NULL || out_result == NULL) return 0;
    if (!nx_join(packages_dir, sizeof(packages_dir), repo_root, ".ncos_packages")) return 0;

#if defined(_WIN32)
    {
        WIN32_FIND_DATAA entry;
        HANDLE handle;
        char pattern[512];
        if (!nx_join(pattern, sizeof(pattern), packages_dir, "*")) return 0;
        handle = FindFirstFileA(pattern, &entry);
        if (handle == INVALID_HANDLE_VALUE) {
            (void)snprintf(result.message, sizeof(result.message), "%s", "Package directory does not exist or is not readable.");
            *out_result = result;
            return 0;
        }
        do {
            char candidate[512];
            if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U) continue;
            if (strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0) continue;
            if (!nx_join(candidate, sizeof(candidate), packages_dir, entry.cFileName)) continue;
            nx_consider_package(repo_root, candidate, &result, &selected_time);
        } while (FindNextFileA(handle, &entry));
        FindClose(handle);
    }
#else
    {
        DIR* directory = opendir(packages_dir);
        struct dirent* entry;
        if (directory == NULL) {
            (void)snprintf(result.message, sizeof(result.message), "%s", "Package directory does not exist or is not readable.");
            *out_result = result;
            return 0;
        }
        while ((entry = readdir(directory)) != NULL) {
            char candidate[512];
            struct stat st;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            if (!nx_join(candidate, sizeof(candidate), packages_dir, entry->d_name)) continue;
            if (stat(candidate, &st) != 0 || !S_ISDIR(st.st_mode)) continue;
            nx_consider_package(repo_root, candidate, &result, &selected_time);
        }
        closedir(directory);
    }
#endif

    if (result.found) {
        (void)snprintf(result.message, sizeof(result.message), "%s", "Newest pending package discovered and ready to apply.");
    } else {
        (void)snprintf(result.message, sizeof(result.message), "%s", "No valid dependency-satisfied pending package was found.");
    }
    *out_result = result;
    return result.found;
}

int NxPackageManager_ApplyNext(const char* repo_root,
                               NxPackageDiscoveryResult* out_discovery,
                               NxPackageApplyResult* out_result)
{
    NxPackageDiscoveryResult discovery;
    memset(&discovery, 0, sizeof(discovery));
    if (out_discovery != NULL) memset(out_discovery, 0, sizeof(*out_discovery));
    if (out_result != NULL) memset(out_result, 0, sizeof(*out_result));
    if (repo_root == NULL || out_discovery == NULL || out_result == NULL) return 0;
    if (!NxPackageManager_DiscoverPendingPackage(repo_root, &discovery)) {
        *out_discovery = discovery;
        (void)snprintf(out_result->failed_phase, sizeof(out_result->failed_phase), "%s", "discovery");
        (void)snprintf(out_result->message, sizeof(out_result->message), "%s", discovery.message);
        return 0;
    }
    *out_discovery = discovery;
    return NxPackageManager_Apply(repo_root, discovery.package_dir, out_result);
}

