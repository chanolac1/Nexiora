#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if !defined(_WIN32)
extern FILE* popen(const char* command, const char* mode);
extern int pclose(FILE* stream);
#endif

#if defined(_WIN32)
#define NX_POPEN _popen
#define NX_PCLOSE _pclose
#else
#define NX_POPEN popen
#define NX_PCLOSE pclose
#endif

#define NX_LINE_CAPACITY 4096U
#define NX_HASH_OFFSET 1469598103934665603ULL
#define NX_HASH_PRIME 1099511628211ULL

static const char* nxwm_required_docs[] = {
    "Docs/MASTER_CONTEXT.md", "Docs/PROJECT_STATE.md", "Docs/ROADMAP.md",
    "Docs/CHANGELOG.md", "Docs/DECISIONS.md", "Docs/ARCHITECTURE.md",
    "Docs/CODING_STANDARD.md", "Docs/TESTING_STANDARD.md", "Docs/PACKAGE_STANDARD.md"
};

static void nxwm_result_init(NxWorkspaceManifestResult* result)
{
    if (result != NULL) {
        memset(result, 0, sizeof(*result));
        result->status = NX_WORKSPACE_MANIFEST_INVALID_ARGUMENT;
    }
}

static int nxwm_copy(char* destination, size_t capacity, const char* source)
{
    size_t length;
    if (destination == NULL || capacity == 0U || source == NULL) return 0;
    length = strlen(source);
    if (length >= capacity) return 0;
    memcpy(destination, source, length + 1U);
    return 1;
}

static int nxwm_join(char* destination, size_t capacity, const char* left, const char* right)
{
    size_t left_length;
    size_t right_length;
    int needs_separator;
    if (destination == NULL || left == NULL || right == NULL) return 0;
    left_length = strlen(left);
    right_length = strlen(right);
    needs_separator = left_length > 0U && left[left_length - 1U] != '/' && left[left_length - 1U] != '\\';
    if (left_length + (size_t)needs_separator + right_length >= capacity) return 0;
    memcpy(destination, left, left_length);
    if (needs_separator) destination[left_length++] = '/';
    memcpy(destination + left_length, right, right_length + 1U);
    return 1;
}

static int nxwm_file_exists(const char* path)
{
    struct stat info;
    return path != NULL && stat(path, &info) == 0 && S_ISREG(info.st_mode);
}

static int nxwm_directory_exists(const char* path)
{
    struct stat info;
    return path != NULL && stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

static unsigned long long nxwm_hash_bytes(unsigned long long hash, const unsigned char* data, size_t size)
{
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= (unsigned long long)data[index];
        hash *= NX_HASH_PRIME;
    }
    return hash;
}

static unsigned long long nxwm_hash_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    unsigned char buffer[8192];
    size_t amount;
    unsigned long long hash = NX_HASH_OFFSET;
    if (file == NULL) return 0ULL;
    while ((amount = fread(buffer, 1U, sizeof(buffer), file)) > 0U) hash = nxwm_hash_bytes(hash, buffer, amount);
    fclose(file);
    return hash;
}

static void nxwm_trim(char* value)
{
    size_t length;
    if (value == NULL) return;
    length = strlen(value);
    while (length > 0U && (value[length - 1U] == '\n' || value[length - 1U] == '\r' || isspace((unsigned char)value[length - 1U]) != 0)) value[--length] = '\0';
}

static int nxwm_command(const char* root, const char* command, char* output, size_t capacity)
{
    char full[NX_LINE_CAPACITY];
    FILE* pipe;
    size_t used = 0U;
    int status;
    if (snprintf(full, sizeof(full), "cd \"%s\" && %s 2>nul", root, command) < 0) return 0;
#if !defined(_WIN32)
    if (snprintf(full, sizeof(full), "cd \"%s\" && %s 2>/dev/null", root, command) < 0) return 0;
#endif
    pipe = NX_POPEN(full, "r");
    if (pipe == NULL) return 0;
    output[0] = '\0';
    while (used + 1U < capacity && fgets(output + used, (int)(capacity - used), pipe) != NULL) used = strlen(output);
    status = NX_PCLOSE(pipe);
    nxwm_trim(output);
    return status == 0;
}

static size_t nxwm_count_token_in_file(const char* path, const char* token)
{
    FILE* file = fopen(path, "rb");
    char line[NX_LINE_CAPACITY];
    size_t count = 0U;
    if (file == NULL) return 0U;
    while (fgets(line, sizeof(line), file) != NULL) if (strstr(line, token) != NULL) ++count;
    fclose(file);
    return count;
}

static size_t nxwm_count_executables(const char* root)
{
    char path[NX_PATH_CAPACITY];
    DIR* directory;
    struct dirent* entry;
    size_t count = 0U;
    if (!nxwm_join(path, sizeof(path), root, "Build/windows-msvc-release/bin")) return 0U;
    directory = opendir(path);
    if (directory == NULL) return 0U;
    while ((entry = readdir(directory)) != NULL) {
        size_t length = strlen(entry->d_name);
        if (length > 4U && strcmp(entry->d_name + length - 4U, ".exe") == 0) ++count;
    }
    closedir(directory);
    return count;
}

static size_t nxwm_count_subdirectories(const char* path)
{
    DIR* directory = opendir(path);
    struct dirent* entry;
    size_t count = 0U;
    if (directory == NULL) return 0U;
    while ((entry = readdir(directory)) != NULL) {
        char child[NX_PATH_CAPACITY];
        if (entry->d_name[0] == '.') continue;
        if (nxwm_join(child, sizeof(child), path, entry->d_name) && nxwm_directory_exists(child)) ++count;
    }
    closedir(directory);
    return count;
}

static void nxwm_collect_packages(const char* root, NxWorkspaceManifestResult* result)
{
    char packages[NX_PATH_CAPACITY];
    char registry[NX_PATH_CAPACITY];
    size_t scanned;
    if (!nxwm_join(packages, sizeof(packages), root, ".ncos_packages") || !nxwm_join(registry, sizeof(registry), root, "Knowledge/NCOS/Packages")) return;
    scanned = nxwm_count_subdirectories(packages);
    result->installed_package_count = nxwm_count_subdirectories(registry);
    result->pending_package_count = scanned > result->installed_package_count ? scanned - result->installed_package_count : 0U;
    result->blocked_package_count = 0U;
}

static void nxwm_collect_docs(const char* root, NxWorkspaceManifestResult* result)
{
    size_t index;
    result->documentation_checked = sizeof(nxwm_required_docs) / sizeof(nxwm_required_docs[0]);
    for (index = 0U; index < result->documentation_checked; ++index) {
        char path[NX_PATH_CAPACITY];
        if (!nxwm_join(path, sizeof(path), root, nxwm_required_docs[index]) || !nxwm_file_exists(path)) ++result->documentation_missing;
    }
}

static unsigned long long nxwm_source_hash(const char* root)
{
    const char* files[] = {"CMakeLists.txt", "CMakePresets.json", "Docs/PROJECT_STATE.md", "Docs/ARCHITECTURE.md"};
    unsigned long long hash = NX_HASH_OFFSET;
    size_t index;
    for (index = 0U; index < sizeof(files) / sizeof(files[0]); ++index) {
        char path[NX_PATH_CAPACITY];
        unsigned long long part;
        if (!nxwm_join(path, sizeof(path), root, files[index])) continue;
        part = nxwm_hash_file(path);
        hash = nxwm_hash_bytes(hash, (const unsigned char*)&part, sizeof(part));
    }
    return hash;
}

static int nxwm_collect(const char* root, NxWorkspaceManifestResult* result)
{
    char cmake[NX_PATH_CAPACITY];
    char status[256];
    nxwm_result_init(result);
    if (root == NULL || result == NULL || !nxwm_join(cmake, sizeof(cmake), root, "CMakeLists.txt") || !nxwm_file_exists(cmake)) return 0;
    (void)nxwm_command(root, "git branch --show-current", result->git_branch, sizeof(result->git_branch));
    (void)nxwm_command(root, "git rev-parse HEAD", result->git_commit, sizeof(result->git_commit));
    result->git_dirty = nxwm_command(root, "git status --porcelain", status, sizeof(status)) && status[0] != '\0';
    result->target_count = nxwm_count_token_in_file(cmake, "add_executable(") + nxwm_count_token_in_file(cmake, "add_library(") + nxwm_count_token_in_file(cmake, "add_custom_target(");
    result->test_count = nxwm_count_token_in_file(cmake, "add_test(");
    result->executable_count = nxwm_count_executables(root);
    nxwm_collect_packages(root, result);
    nxwm_collect_docs(root, result);
    result->source_inventory_hash = nxwm_source_hash(root);
    result->cmake_inventory_hash = nxwm_hash_file(cmake);
    {
        char registry[NX_PATH_CAPACITY];
        if (nxwm_join(registry, sizeof(registry), root, "Knowledge/NCOS/Packages")) {
            result->package_inventory_hash = NX_HASH_OFFSET ^ (unsigned long long)nxwm_count_subdirectories(registry);
        }
    }
    return 1;
}

static int nxwm_write_manifest(const char* path, const NxWorkspaceManifestResult* result)
{
    FILE* file = fopen(path, "wb");
    if (file == NULL) return 0;
    fprintf(file, "nxworkspace/1\n");
    fprintf(file, "project=Nexiora\n");
    fprintf(file, "git_branch=%s\n", result->git_branch[0] != '\0' ? result->git_branch : "unknown");
    fprintf(file, "git_commit=%s\n", result->git_commit[0] != '\0' ? result->git_commit : "unknown");
    fprintf(file, "git_dirty=%s\n", result->git_dirty ? "true" : "false");
    fprintf(file, "compiler=MSYS2 UCRT64 GCC\n");
    fprintf(file, "c_standard=C23\n");
    fprintf(file, "warning_policy=zero\n");
    fprintf(file, "target_count=%zu\n", result->target_count);
    fprintf(file, "test_count=%zu\n", result->test_count);
    fprintf(file, "executable_count=%zu\n", result->executable_count);
    fprintf(file, "installed_packages=%zu\n", result->installed_package_count);
    fprintf(file, "pending_packages=%zu\n", result->pending_package_count);
    fprintf(file, "blocked_packages=%zu\n", result->blocked_package_count);
    fprintf(file, "documentation_checked=%zu\n", result->documentation_checked);
    fprintf(file, "documentation_missing=%zu\n", result->documentation_missing);
    fprintf(file, "source_inventory_hash=%016llx\n", result->source_inventory_hash);
    fprintf(file, "cmake_inventory_hash=%016llx\n", result->cmake_inventory_hash);
    fprintf(file, "package_inventory_hash=%016llx\n", result->package_inventory_hash);
    return fclose(file) == 0;
}

static int nxwm_parse_ull(const char* path, const char* key, unsigned long long* value)
{
    FILE* file = fopen(path, "rb");
    char line[NX_LINE_CAPACITY];
    size_t key_length = strlen(key);
    if (file == NULL) return 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, key, key_length) == 0 && line[key_length] == '=') {
            *value = strtoull(line + key_length + 1U, NULL, 16);
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

const char* NxWorkspaceManifest_StatusName(NxWorkspaceManifestStatus status)
{
    switch (status) {
        case NX_WORKSPACE_MANIFEST_OK: return "OK";
        case NX_WORKSPACE_MANIFEST_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_WORKSPACE_MANIFEST_IO_ERROR: return "IO_ERROR";
        case NX_WORKSPACE_MANIFEST_COMMAND_ERROR: return "COMMAND_ERROR";
        case NX_WORKSPACE_MANIFEST_OUT_OF_SYNC: return "OUT_OF_SYNC";
        default: return "UNKNOWN";
    }
}

int NxWorkspaceManifest_Snapshot(const char* repository_root, const char* output_path, NxWorkspaceManifestResult* out_result)
{
    NxWorkspaceManifestResult result;
    const char* target = output_path != NULL && output_path[0] != '\0' ? output_path : "workspace.nxws";
    if (!nxwm_collect(repository_root, &result)) return 0;
    if (!nxwm_copy(result.manifest_path, sizeof(result.manifest_path), target) || !nxwm_write_manifest(target, &result)) {
        result.status = NX_WORKSPACE_MANIFEST_IO_ERROR;
        nxwm_copy(result.message, sizeof(result.message), "No se pudo escribir el manifiesto del workspace.");
        if (out_result != NULL) *out_result = result;
        return 0;
    }
    result.status = NX_WORKSPACE_MANIFEST_OK;
    nxwm_copy(result.message, sizeof(result.message), "Workspace snapshot created.");
    if (out_result != NULL) *out_result = result;
    return 1;
}

int NxWorkspaceManifest_Diff(const char* repository_root, const char* manifest_path, NxWorkspaceManifestResult* out_result)
{
    NxWorkspaceManifestResult result;
    unsigned long long old_source = 0ULL, old_cmake = 0ULL, old_packages = 0ULL;
    if (!nxwm_collect(repository_root, &result) || manifest_path == NULL || !nxwm_file_exists(manifest_path)) return 0;
    if (!nxwm_parse_ull(manifest_path, "source_inventory_hash", &old_source) || !nxwm_parse_ull(manifest_path, "cmake_inventory_hash", &old_cmake) || !nxwm_parse_ull(manifest_path, "package_inventory_hash", &old_packages)) return 0;
    if (old_source != result.source_inventory_hash) ++result.differences;
    if (old_cmake != result.cmake_inventory_hash) ++result.differences;
    if (old_packages != result.package_inventory_hash) ++result.differences;
    result.status = result.differences == 0U ? NX_WORKSPACE_MANIFEST_OK : NX_WORKSPACE_MANIFEST_OUT_OF_SYNC;
    nxwm_copy(result.manifest_path, sizeof(result.manifest_path), manifest_path);
    nxwm_copy(result.message, sizeof(result.message), result.differences == 0U ? "Workspace matches manifest." : "Workspace differs from manifest.");
    if (out_result != NULL) *out_result = result;
    return result.differences == 0U;
}

int NxWorkspaceManifest_Validate(const char* repository_root, const char* manifest_path, NxWorkspaceManifestResult* out_result)
{
    NxWorkspaceManifestResult result;
    int matches = NxWorkspaceManifest_Diff(repository_root, manifest_path, &result);
    if (result.documentation_missing > 0U) {
        result.status = NX_WORKSPACE_MANIFEST_OUT_OF_SYNC;
        ++result.differences;
        matches = 0;
        nxwm_copy(result.message, sizeof(result.message), "Workspace documentation or inventories are out of sync.");
    }
    if (out_result != NULL) *out_result = result;
    return matches;
}
