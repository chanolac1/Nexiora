#include "Nexiora/NCOS/NxWorkspaceEngine.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#define NX_RMDIR_COMMAND_PREFIX "cmd /c rmdir /s /q \""
#define NX_RMDIR_COMMAND_SUFFIX "\""
#else
#include <unistd.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#define NX_RMDIR_COMMAND_PREFIX "rm -rf \""
#define NX_RMDIR_COMMAND_SUFFIX "\""
#endif

#define NX_PATH_CAPACITY 1024U

static int nx_copy_text(char* destination, size_t capacity, const char* source)
{
    size_t length;
    if (destination == NULL || capacity == 0U || source == NULL) {
        return 0;
    }
    length = strlen(source);
    if (length >= capacity) {
        destination[0] = '\0';
        return 0;
    }
    memcpy(destination, source, length + 1U);
    return 1;
}

static int nx_append_text(char* destination, size_t capacity, const char* suffix)
{
    size_t used;
    size_t extra;
    if (destination == NULL || suffix == NULL || capacity == 0U) {
        return 0;
    }
    used = strlen(destination);
    extra = strlen(suffix);
    if (used + extra >= capacity) {
        return 0;
    }
    memcpy(destination + used, suffix, extra + 1U);
    return 1;
}

static int nx_join(char* destination, size_t capacity, const char* left, const char* right)
{
    size_t length;
    if (!nx_copy_text(destination, capacity, left)) {
        return 0;
    }
    length = strlen(destination);
    if (length > 0U && destination[length - 1U] != '/' && destination[length - 1U] != '\\') {
        if (!nx_append_text(destination, capacity, "/")) {
            return 0;
        }
    }
    return nx_append_text(destination, capacity, right);
}

static int nx_normalize_id(char* destination, size_t capacity, const char* source)
{
    size_t input_index;
    size_t output_index = 0U;
    int previous_dash = 0;
    if (destination == NULL || capacity < 2U || source == NULL || source[0] == '\0') {
        return 0;
    }
    for (input_index = 0U; source[input_index] != '\0'; ++input_index) {
        unsigned char value = (unsigned char)source[input_index];
        if (isalnum(value) != 0) {
            if (output_index + 1U >= capacity) {
                return 0;
            }
            destination[output_index++] = (char)tolower(value);
            previous_dash = 0;
        } else if (!previous_dash && output_index > 0U) {
            if (output_index + 1U >= capacity) {
                return 0;
            }
            destination[output_index++] = '-';
            previous_dash = 1;
        }
    }
    while (output_index > 0U && destination[output_index - 1U] == '-') {
        --output_index;
    }
    destination[output_index] = '\0';
    return output_index > 0U;
}

static int nx_path_is_safe_relative(const char* path)
{
    if (path == NULL || path[0] == '\0') {
        return 0;
    }
    if (path[0] == '/' || path[0] == '\\') {
        return 0;
    }
    if (strlen(path) > 1U && path[1] == ':') {
        return 0;
    }
    if (strstr(path, "../") != NULL || strstr(path, "..\\") != NULL || strcmp(path, "..") == 0) {
        return 0;
    }
    return 1;
}

static int nx_directory_exists(const char* path)
{
    struct stat info;
    return path != NULL && stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

static int nx_file_exists(const char* path)
{
    struct stat info;
    return path != NULL && stat(path, &info) == 0 && S_ISREG(info.st_mode);
}

static int nx_make_directory(const char* path)
{
    if (nx_directory_exists(path)) {
        return 1;
    }
    return NX_MKDIR(path) == 0 || errno == EEXIST;
}

static int nx_make_directory_chain(const char* path)
{
    char buffer[NX_PATH_CAPACITY];
    size_t index;
    if (!nx_copy_text(buffer, sizeof(buffer), path)) {
        return 0;
    }
    for (index = 1U; buffer[index] != '\0'; ++index) {
        if (buffer[index] == '/' || buffer[index] == '\\') {
            char saved = buffer[index];
            buffer[index] = '\0';
            if (!(index == 2U && buffer[1] == ':') && !nx_make_directory(buffer)) {
                return 0;
            }
            buffer[index] = saved;
        }
    }
    return nx_make_directory(buffer);
}

static int nx_parent_directory(char* destination, size_t capacity, const char* path)
{
    char* slash;
    char* backslash;
    char* separator;
    if (!nx_copy_text(destination, capacity, path)) {
        return 0;
    }
    slash = strrchr(destination, '/');
    backslash = strrchr(destination, '\\');
    separator = slash;
    if (backslash != NULL && (separator == NULL || backslash > separator)) {
        separator = backslash;
    }
    if (separator == NULL) {
        destination[0] = '.';
        destination[1] = '\0';
        return 1;
    }
    *separator = '\0';
    return destination[0] != '\0';
}

static int nx_copy_file(const char* source, const char* destination)
{
    FILE* input;
    FILE* output;
    unsigned char buffer[8192];
    size_t amount;
    char parent[NX_PATH_CAPACITY];
    if (!nx_parent_directory(parent, sizeof(parent), destination) || !nx_make_directory_chain(parent)) {
        return 0;
    }
    input = fopen(source, "rb");
    if (input == NULL) {
        return 0;
    }
    output = fopen(destination, "wb");
    if (output == NULL) {
        fclose(input);
        return 0;
    }
    while ((amount = fread(buffer, 1U, sizeof(buffer), input)) > 0U) {
        if (fwrite(buffer, 1U, amount, output) != amount) {
            fclose(input);
            fclose(output);
            return 0;
        }
    }
    if (ferror(input) != 0) {
        fclose(input);
        fclose(output);
        return 0;
    }
    fclose(input);
    return fclose(output) == 0;
}

static int nx_write_metadata(const NxWorkspaceInfo* info)
{
    FILE* file;
    time_t now;
    if (info == NULL) {
        return 0;
    }
    file = fopen(info->manifest_path, "wb");
    if (file == NULL) {
        return 0;
    }
    now = time(NULL);
    fprintf(file, "id=%s\n", info->id);
    fprintf(file, "state=%s\n", NxWorkspace_StateName(info->state));
    fprintf(file, "files_requested=%zu\n", info->files_requested);
    fprintf(file, "files_copied=%zu\n", info->files_copied);
    fprintf(file, "files_skipped=%zu\n", info->files_skipped);
    fprintf(file, "updated_at=%lld\n", (long long)now);
    return fclose(file) == 0;
}

static int nx_append_log(const NxWorkspaceInfo* info, const char* action, const char* detail)
{
    FILE* file;
    time_t now;
    if (info == NULL || action == NULL || detail == NULL) {
        return 0;
    }
    file = fopen(info->log_path, "ab");
    if (file == NULL) {
        return 0;
    }
    now = time(NULL);
    fprintf(file, "%lld|%s|%s\n", (long long)now, action, detail);
    return fclose(file) == 0;
}

static int nx_build_paths(const char* root, const char* workspace_id, NxWorkspaceInfo* info)
{
    char safe_id[128];
    char knowledge[NX_PATH_CAPACITY];
    char ncos[NX_PATH_CAPACITY];
    char workspaces[NX_PATH_CAPACITY];
    if (root == NULL || info == NULL || !nx_normalize_id(safe_id, sizeof(safe_id), workspace_id)) {
        return 0;
    }
    memset(info, 0, sizeof(*info));
    if (!nx_copy_text(info->id, sizeof(info->id), safe_id) ||
        !nx_join(knowledge, sizeof(knowledge), root, "Knowledge") ||
        !nx_join(ncos, sizeof(ncos), knowledge, "NCOS") ||
        !nx_join(workspaces, sizeof(workspaces), ncos, "Workspaces") ||
        !nx_join(info->path, sizeof(info->path), workspaces, safe_id) ||
        !nx_join(info->source_path, sizeof(info->source_path), info->path, "Source") ||
        !nx_join(info->manifest_path, sizeof(info->manifest_path), info->path, "workspace.state") ||
        !nx_join(info->log_path, sizeof(info->log_path), info->path, "actions.log")) {
        return 0;
    }
    return 1;
}

static int nx_parse_size_value(const char* line, const char* key, size_t* value)
{
    size_t key_length;
    char* end;
    unsigned long long parsed;
    if (line == NULL || key == NULL || value == NULL) {
        return 0;
    }
    key_length = strlen(key);
    if (strncmp(line, key, key_length) != 0 || line[key_length] != '=') {
        return 0;
    }
    parsed = strtoull(line + key_length + 1U, &end, 10);
    if (end == line + key_length + 1U) {
        return 0;
    }
    *value = (size_t)parsed;
    return 1;
}

const char* NxWorkspace_StateName(NxWorkspaceState state)
{
    switch (state) {
        case NX_WORKSPACE_STATE_READY: return "READY";
        case NX_WORKSPACE_STATE_CLOSED: return "CLOSED";
        default: return "UNKNOWN";
    }
}

int NxWorkspace_Create(
    const char* repository_root,
    const char* workspace_id,
    const char* const* relative_files,
    size_t file_count,
    NxWorkspaceInfo* out_info)
{
    NxWorkspaceInfo info;
    size_t index;
    if (repository_root == NULL || relative_files == NULL || file_count == 0U ||
        !nx_build_paths(repository_root, workspace_id, &info)) {
        return 0;
    }
    if (nx_file_exists(info.manifest_path)) {
        return 0;
    }
    if (!nx_make_directory_chain(info.source_path)) {
        return 0;
    }
    info.files_requested = file_count;
    info.state = NX_WORKSPACE_STATE_READY;
    for (index = 0U; index < file_count; ++index) {
        char source[NX_PATH_CAPACITY];
        char destination[NX_PATH_CAPACITY];
        if (!nx_path_is_safe_relative(relative_files[index]) ||
            !nx_join(source, sizeof(source), repository_root, relative_files[index]) ||
            !nx_join(destination, sizeof(destination), info.source_path, relative_files[index])) {
            ++info.files_skipped;
            continue;
        }
        if (!nx_file_exists(source)) {
            ++info.files_skipped;
            continue;
        }
        if (!nx_copy_file(source, destination)) {
            return 0;
        }
        ++info.files_copied;
        if (!nx_append_log(&info, "COPY", relative_files[index])) {
            return 0;
        }
    }
    if (info.files_copied == 0U || !nx_write_metadata(&info) || !nx_append_log(&info, "CREATE", "workspace ready")) {
        return 0;
    }
    if (out_info != NULL) {
        *out_info = info;
    }
    return 1;
}

int NxWorkspace_Status(const char* repository_root, const char* workspace_id, NxWorkspaceInfo* out_info)
{
    NxWorkspaceInfo info;
    FILE* file;
    char line[256];
    if (!nx_build_paths(repository_root, workspace_id, &info)) {
        return 0;
    }
    file = fopen(info.manifest_path, "rb");
    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "state=READY") == 0) {
            info.state = NX_WORKSPACE_STATE_READY;
        } else if (strcmp(line, "state=CLOSED") == 0) {
            info.state = NX_WORKSPACE_STATE_CLOSED;
        } else if (!nx_parse_size_value(line, "files_requested", &info.files_requested) &&
                   !nx_parse_size_value(line, "files_copied", &info.files_copied)) {
            (void)nx_parse_size_value(line, "files_skipped", &info.files_skipped);
        }
    }
    fclose(file);
    if (info.state == NX_WORKSPACE_STATE_UNKNOWN) {
        return 0;
    }
    if (out_info != NULL) {
        *out_info = info;
    }
    return 1;
}

int NxWorkspace_Close(const char* repository_root, const char* workspace_id, NxWorkspaceInfo* out_info)
{
    NxWorkspaceInfo info;
    if (!NxWorkspace_Status(repository_root, workspace_id, &info)) {
        return 0;
    }
    if (info.state == NX_WORKSPACE_STATE_CLOSED) {
        if (out_info != NULL) {
            *out_info = info;
        }
        return 1;
    }
    info.state = NX_WORKSPACE_STATE_CLOSED;
    if (!nx_write_metadata(&info) || !nx_append_log(&info, "CLOSE", "workspace closed")) {
        return 0;
    }
    if (out_info != NULL) {
        *out_info = info;
    }
    return 1;
}

int NxWorkspace_Clean(const char* repository_root, const char* workspace_id)
{
    NxWorkspaceInfo info;
    char command[NX_PATH_CAPACITY + 64U];
    int written;
    if (!nx_build_paths(repository_root, workspace_id, &info)) {
        return 0;
    }
    if (!nx_directory_exists(info.path)) {
        return 1;
    }
    written = snprintf(command, sizeof(command), "%s%s%s", NX_RMDIR_COMMAND_PREFIX, info.path, NX_RMDIR_COMMAND_SUFFIX);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        return 0;
    }
    return system(command) == 0;
}
