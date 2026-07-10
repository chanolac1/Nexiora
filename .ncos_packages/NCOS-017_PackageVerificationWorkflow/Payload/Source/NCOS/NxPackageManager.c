#include "Nexiora/NCOS/NxPackageManager.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <errno.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

#define NX_PM_PATH_MAX 512
#define NX_PM_LINE_MAX 1024

static int nx_pm_is_sep(char c)
{
    return c == '/' || c == '\\';
}

static int nx_pm_exists(const char* path)
{
    struct stat st;
    if (path == NULL) {
        return 0;
    }
    return stat(path, &st) == 0;
}

static int nx_pm_copy_string(char* dst, size_t dst_size, const char* src)
{
    size_t len;
    if (dst == NULL || dst_size == 0 || src == NULL) {
        return 0;
    }
    len = strlen(src);
    if (len >= dst_size) {
        dst[0] = '\0';
        return 0;
    }
    memcpy(dst, src, len + 1);
    return 1;
}

static int nx_pm_join(char* dst, size_t dst_size, const char* left, const char* right)
{
    size_t left_len;
    size_t right_len;
    int need_sep;

    if (dst == NULL || dst_size == 0 || left == NULL || right == NULL) {
        return 0;
    }

    left_len = strlen(left);
    right_len = strlen(right);
    need_sep = left_len > 0 && !nx_pm_is_sep(left[left_len - 1]);

    if (left_len + (need_sep ? 1u : 0u) + right_len >= dst_size) {
        dst[0] = '\0';
        return 0;
    }

    memcpy(dst, left, left_len);
    if (need_sep) {
        dst[left_len] = '/';
        memcpy(dst + left_len + 1u, right, right_len + 1u);
    } else {
        memcpy(dst + left_len, right, right_len + 1u);
    }

    return 1;
}

static void nx_pm_normalize_id(char* dst, size_t dst_size, const char* src)
{
    size_t out = 0;
    size_t i;
    int last_dash = 0;

    if (dst == NULL || dst_size == 0) {
        return;
    }

    dst[0] = '\0';
    if (src == NULL) {
        return;
    }

    for (i = 0; src[i] != '\0' && out + 1u < dst_size; ++i) {
        unsigned char ch = (unsigned char)src[i];
        if (isalnum(ch)) {
            dst[out++] = (char)tolower(ch);
            last_dash = 0;
        } else if (!last_dash && out > 0) {
            dst[out++] = '-';
            last_dash = 1;
        }
    }

    if (out > 0 && dst[out - 1u] == '-') {
        --out;
    }
    dst[out] = '\0';
}

static int nx_pm_mkdir_chain(const char* path)
{
    char tmp[NX_PM_PATH_MAX];
    size_t len;
    size_t i;

    if (!nx_pm_copy_string(tmp, sizeof(tmp), path)) {
        return 0;
    }

    len = strlen(tmp);
    if (len == 0) {
        return 0;
    }

    for (i = 1; i < len; ++i) {
        if (nx_pm_is_sep(tmp[i])) {
            char saved = tmp[i];
            tmp[i] = '\0';
            if (strlen(tmp) > 0 && !nx_pm_exists(tmp)) {
                (void)NX_MKDIR(tmp);
            }
            tmp[i] = saved;
        }
    }

    if (!nx_pm_exists(tmp)) {
        (void)NX_MKDIR(tmp);
    }

    return nx_pm_exists(tmp);
}

static int nx_pm_parent_dir(const char* path, char* out, size_t out_size)
{
    size_t len;
    size_t i;

    if (!nx_pm_copy_string(out, out_size, path)) {
        return 0;
    }

    len = strlen(out);
    for (i = len; i > 0; --i) {
        if (nx_pm_is_sep(out[i - 1u])) {
            out[i - 1u] = '\0';
            return 1;
        }
    }

    out[0] = '.';
    out[1] = '\0';
    return 1;
}

static unsigned long long nx_pm_hash_file(const char* path)
{
    FILE* f;
    unsigned long long hash = 1469598103934665603ull;
    int ch;

    f = fopen(path, "rb");
    if (f == NULL) {
        return 0ull;
    }

    while ((ch = fgetc(f)) != EOF) {
        hash ^= (unsigned char)ch;
        hash *= 1099511628211ull;
    }

    fclose(f);
    return hash;
}

static int nx_pm_copy_file(const char* src, const char* dst)
{
    FILE* in;
    FILE* out;
    unsigned char buffer[4096];
    size_t n;

    in = fopen(src, "rb");
    if (in == NULL) {
        return 0;
    }

    out = fopen(dst, "wb");
    if (out == NULL) {
        fclose(in);
        return 0;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return 0;
        }
    }

    fclose(in);
    fclose(out);
    return 1;
}

static int nx_pm_write_text(const char* path, const char* text)
{
    FILE* f;
    char parent[NX_PM_PATH_MAX];

    if (!nx_pm_parent_dir(path, parent, sizeof(parent))) {
        return 0;
    }
    if (!nx_pm_mkdir_chain(parent)) {
        return 0;
    }

    f = fopen(path, "wb");
    if (f == NULL) {
        return 0;
    }
    fputs(text != NULL ? text : "", f);
    fclose(f);
    return 1;
}

static int nx_pm_append_text(const char* path, const char* text)
{
    FILE* f;
    char parent[NX_PM_PATH_MAX];

    if (!nx_pm_parent_dir(path, parent, sizeof(parent))) {
        return 0;
    }
    if (!nx_pm_mkdir_chain(parent)) {
        return 0;
    }

    f = fopen(path, "ab");
    if (f == NULL) {
        return 0;
    }
    fputs(text != NULL ? text : "", f);
    fclose(f);
    return 1;
}

static char* nx_pm_trim(char* s)
{
    char* end;
    if (s == NULL) {
        return NULL;
    }
    while (*s != '\0' && isspace((unsigned char)*s)) {
        ++s;
    }
    end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) {
        --end;
    }
    *end = '\0';
    return s;
}

static int nx_pm_starts_with(const char* s, const char* prefix)
{
    size_t prefix_len;
    if (s == NULL || prefix == NULL) {
        return 0;
    }
    prefix_len = strlen(prefix);
    return strncmp(s, prefix, prefix_len) == 0;
}

static int nx_pm_parse_file_mapping(const char* line,
                                    char* src_rel,
                                    size_t src_size,
                                    char* dst_rel,
                                    size_t dst_size)
{
    const char* value;
    const char* arrow;
    size_t src_len;
    char left[NX_PM_LINE_MAX];
    char right[NX_PM_LINE_MAX];
    char* ltrim;
    char* rtrim;

    if (!nx_pm_starts_with(line, "file=")) {
        return 0;
    }

    value = line + 5;
    arrow = strstr(value, "=>");
    if (arrow == NULL) {
        return 0;
    }

    src_len = (size_t)(arrow - value);
    if (src_len >= sizeof(left)) {
        return 0;
    }

    memcpy(left, value, src_len);
    left[src_len] = '\0';
    if (!nx_pm_copy_string(right, sizeof(right), arrow + 2)) {
        return 0;
    }

    ltrim = nx_pm_trim(left);
    rtrim = nx_pm_trim(right);

    return nx_pm_copy_string(src_rel, src_size, ltrim) && nx_pm_copy_string(dst_rel, dst_size, rtrim);
}

static int nx_pm_read_manifest_identity(const char* manifest_path,
                                        char* package_id,
                                        size_t package_id_size,
                                        char* version,
                                        size_t version_size)
{
    FILE* f;
    char line[NX_PM_LINE_MAX];

    f = fopen(manifest_path, "rb");
    if (f == NULL) {
        return 0;
    }

    package_id[0] = '\0';
    version[0] = '\0';

    while (fgets(line, sizeof(line), f) != NULL) {
        char* trimmed = nx_pm_trim(line);
        if (nx_pm_starts_with(trimmed, "id=")) {
            nx_pm_normalize_id(package_id, package_id_size, trimmed + 3);
        } else if (nx_pm_starts_with(trimmed, "version=")) {
            (void)nx_pm_copy_string(version, version_size, nx_pm_trim(trimmed + 8));
        }
    }

    fclose(f);

    if (package_id[0] == '\0') {
        return 0;
    }
    if (version[0] == '\0') {
        (void)nx_pm_copy_string(version, version_size, "0.0.0");
    }
    return 1;
}

static int nx_pm_registry_paths(const char* repo_root,
                                const char* package_id,
                                char* registry_file,
                                size_t registry_size,
                                char* log_file,
                                size_t log_size)
{
    char knowledge[NX_PM_PATH_MAX];
    char ncos[NX_PM_PATH_MAX];
    char packages[NX_PM_PATH_MAX];
    char pkg_dir[NX_PM_PATH_MAX];

    if (!nx_pm_join(knowledge, sizeof(knowledge), repo_root != NULL ? repo_root : ".", "Knowledge")) {
        return 0;
    }
    if (!nx_pm_join(ncos, sizeof(ncos), knowledge, "NCOS")) {
        return 0;
    }
    if (!nx_pm_join(packages, sizeof(packages), ncos, "Packages")) {
        return 0;
    }
    if (!nx_pm_join(pkg_dir, sizeof(pkg_dir), packages, package_id)) {
        return 0;
    }
    if (!nx_pm_mkdir_chain(pkg_dir)) {
        return 0;
    }
    if (!nx_pm_join(registry_file, registry_size, pkg_dir, "registry.txt")) {
        return 0;
    }
    if (!nx_pm_join(log_file, log_size, pkg_dir, "install.log")) {
        return 0;
    }
    return 1;
}

static int nx_pm_make_backup_path(const char* repo_root,
                                  const char* package_id,
                                  const char* dst_rel,
                                  char* out,
                                  size_t out_size)
{
    char knowledge[NX_PM_PATH_MAX];
    char ncos[NX_PM_PATH_MAX];
    char backups[NX_PM_PATH_MAX];
    char pkg_backups[NX_PM_PATH_MAX];
    char safe_rel[NX_PM_PATH_MAX];
    size_t i;

    if (!nx_pm_copy_string(safe_rel, sizeof(safe_rel), dst_rel)) {
        return 0;
    }
    for (i = 0; safe_rel[i] != '\0'; ++i) {
        if (nx_pm_is_sep(safe_rel[i]) || safe_rel[i] == ':') {
            safe_rel[i] = '_';
        }
    }

    if (!nx_pm_join(knowledge, sizeof(knowledge), repo_root != NULL ? repo_root : ".", "Knowledge")) {
        return 0;
    }
    if (!nx_pm_join(ncos, sizeof(ncos), knowledge, "NCOS")) {
        return 0;
    }
    if (!nx_pm_join(backups, sizeof(backups), ncos, "PackageBackups")) {
        return 0;
    }
    if (!nx_pm_join(pkg_backups, sizeof(pkg_backups), backups, package_id)) {
        return 0;
    }
    if (!nx_pm_mkdir_chain(pkg_backups)) {
        return 0;
    }
    return nx_pm_join(out, out_size, pkg_backups, safe_rel);
}

int NxPackageManager_Install(const char* repo_root,
                             const char* package_dir,
                             NxPackageInstallResult* out_result)
{
    NxPackageInstallResult result;
    char manifest_path[NX_PM_PATH_MAX];
    char package_id[128];
    char package_version[64];
    char registry_path[NX_PM_PATH_MAX];
    char log_path[NX_PM_PATH_MAX];
    FILE* manifest;
    char line[NX_PM_LINE_MAX];
    int ok = 1;

    memset(&result, 0, sizeof(result));

    if (repo_root == NULL || package_dir == NULL) {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    if (!nx_pm_join(manifest_path, sizeof(manifest_path), package_dir, "manifest.npkg")) {
        return 0;
    }

    if (!nx_pm_read_manifest_identity(manifest_path, package_id, sizeof(package_id), package_version, sizeof(package_version))) {
        return 0;
    }

    if (!nx_pm_registry_paths(repo_root, package_id, registry_path, sizeof(registry_path), log_path, sizeof(log_path))) {
        return 0;
    }

    (void)nx_pm_copy_string(result.package_id, sizeof(result.package_id), package_id);
    (void)nx_pm_copy_string(result.package_version, sizeof(result.package_version), package_version);
    (void)nx_pm_copy_string(result.registry_path, sizeof(result.registry_path), registry_path);
    (void)nx_pm_copy_string(result.install_log_path, sizeof(result.install_log_path), log_path);

    (void)nx_pm_write_text(log_path, "Nexiora Package Manager install log\n");

    manifest = fopen(manifest_path, "rb");
    if (manifest == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), manifest) != NULL) {
        char* trimmed = nx_pm_trim(line);
        char src_rel[NX_PM_PATH_MAX];
        char dst_rel[NX_PM_PATH_MAX];
        char src_path[NX_PM_PATH_MAX];
        char dst_path[NX_PM_PATH_MAX];
        char parent[NX_PM_PATH_MAX];
        unsigned long long src_hash;
        unsigned long long dst_hash;

        if (!nx_pm_parse_file_mapping(trimmed, src_rel, sizeof(src_rel), dst_rel, sizeof(dst_rel))) {
            continue;
        }

        result.files_declared++;

        if (!nx_pm_join(src_path, sizeof(src_path), package_dir, src_rel)) {
            ok = 0;
            continue;
        }
        if (!nx_pm_join(dst_path, sizeof(dst_path), repo_root, dst_rel)) {
            ok = 0;
            continue;
        }
        if (!nx_pm_parent_dir(dst_path, parent, sizeof(parent)) || !nx_pm_mkdir_chain(parent)) {
            ok = 0;
            continue;
        }

        src_hash = nx_pm_hash_file(src_path);
        dst_hash = nx_pm_hash_file(dst_path);

        if (src_hash != 0ull && src_hash == dst_hash) {
            result.files_skipped++;
            (void)nx_pm_append_text(log_path, "SKIP identical file\n");
            continue;
        }

        if (nx_pm_exists(dst_path)) {
            char backup_path[NX_PM_PATH_MAX];
            if (nx_pm_make_backup_path(repo_root, package_id, dst_rel, backup_path, sizeof(backup_path))) {
                if (nx_pm_copy_file(dst_path, backup_path)) {
                    result.files_backed_up++;
                }
            }
        }

        if (!nx_pm_copy_file(src_path, dst_path)) {
            ok = 0;
            (void)nx_pm_append_text(log_path, "ERROR copy failed\n");
            continue;
        }

        result.files_installed++;
        (void)nx_pm_append_text(log_path, "INSTALL file\n");
    }

    fclose(manifest);

    result.success = ok;

    {
        char registry_text[1024];
        int written = snprintf(registry_text,
                               sizeof(registry_text),
                               "id=%s\nversion=%s\nstatus=%s\nfiles_declared=%d\nfiles_installed=%d\nfiles_skipped=%d\nfiles_backed_up=%d\n",
                               result.package_id,
                               result.package_version,
                               result.success ? "installed" : "failed",
                               result.files_declared,
                               result.files_installed,
                               result.files_skipped,
                               result.files_backed_up);
        if (written > 0 && (size_t)written < sizeof(registry_text)) {
            (void)nx_pm_write_text(registry_path, registry_text);
        }
    }

    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}

int NxPackageManager_Status(const char* repo_root,
                            const char* package_id,
                            NxPackageInstallResult* out_result)
{
    NxPackageInstallResult result;
    char safe_id[128];
    char registry_path[NX_PM_PATH_MAX];
    char log_path[NX_PM_PATH_MAX];

    memset(&result, 0, sizeof(result));

    if (repo_root == NULL || package_id == NULL) {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    nx_pm_normalize_id(safe_id, sizeof(safe_id), package_id);
    if (safe_id[0] == '\0') {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    if (!nx_pm_registry_paths(repo_root, safe_id, registry_path, sizeof(registry_path), log_path, sizeof(log_path))) {
        return 0;
    }

    (void)nx_pm_copy_string(result.package_id, sizeof(result.package_id), safe_id);
    (void)nx_pm_copy_string(result.registry_path, sizeof(result.registry_path), registry_path);
    (void)nx_pm_copy_string(result.install_log_path, sizeof(result.install_log_path), log_path);
    result.success = nx_pm_exists(registry_path);

    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}


int NxPackageManager_VerifyPackage(const char* package_dir, NxPackageVerifyResult* out_result)
{
    NxPackageVerifyResult result;
    char manifest_path[NX_PM_PATH_MAX];
    FILE* manifest;
    char line[NX_PM_LINE_MAX];

    memset(&result, 0, sizeof(result));
    nx_pm_copy_string(result.message, sizeof(result.message), "Package verification failed.");

    if (package_dir == NULL) {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    if (!nx_pm_join(manifest_path, sizeof(manifest_path), package_dir, "manifest.npkg")) {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    if (!nx_pm_exists(manifest_path)) {
        nx_pm_copy_string(result.message, sizeof(result.message), "manifest.npkg not found.");
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    result.manifest_found = 1;
    if (!nx_pm_read_manifest_identity(manifest_path,
                                      result.package_id,
                                      sizeof(result.package_id),
                                      result.package_version,
                                      sizeof(result.package_version))) {
        nx_pm_copy_string(result.message, sizeof(result.message), "Invalid package identity.");
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    manifest = fopen(manifest_path, "rb");
    if (manifest == NULL) {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    while (fgets(line, sizeof(line), manifest) != NULL) {
        char* trimmed = nx_pm_trim(line);
        char src_rel[NX_PM_PATH_MAX];
        char dst_rel[NX_PM_PATH_MAX];
        char src_path[NX_PM_PATH_MAX];

        if (!nx_pm_parse_file_mapping(trimmed, src_rel, sizeof(src_rel), dst_rel, sizeof(dst_rel))) {
            continue;
        }

        result.files_declared++;
        if (!nx_pm_join(src_path, sizeof(src_path), package_dir, src_rel)) {
            result.payload_files_missing++;
            continue;
        }

        if (nx_pm_exists(src_path)) {
            result.payload_files_found++;
        } else {
            result.payload_files_missing++;
        }
    }

    fclose(manifest);

    result.success = result.manifest_found && result.files_declared > 0 && result.payload_files_missing == 0;
    if (result.success) {
        nx_pm_copy_string(result.message, sizeof(result.message), "Package is valid and installable.");
    } else if (result.files_declared == 0) {
        nx_pm_copy_string(result.message, sizeof(result.message), "Package has no file mappings.");
    } else {
        nx_pm_copy_string(result.message, sizeof(result.message), "Package has missing payload files.");
    }

    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}
