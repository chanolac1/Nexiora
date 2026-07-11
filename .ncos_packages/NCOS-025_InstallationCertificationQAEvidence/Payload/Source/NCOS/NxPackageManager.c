#include "Nexiora/NCOS/NxPackageManager.h"
#include "Nexiora/NCOS/NxPackageCertification.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <errno.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif


#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
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


static int nx_pm_dependency_installed(const char* repo_root, const char* dependency_id)
{
    char normalized[128];
    char registry_path[NX_PM_PATH_MAX];
    char log_path[NX_PM_PATH_MAX];

    nx_pm_normalize_id(normalized, sizeof(normalized), dependency_id);
    if (normalized[0] == '\0') {
        return 0;
    }
    if (!nx_pm_registry_paths(repo_root, normalized, registry_path, sizeof(registry_path), log_path, sizeof(log_path))) {
        return 0;
    }
    return nx_pm_exists(registry_path);
}

int NxPackageManager_VerifyDependencies(const char* repo_root,
                                        const char* package_dir,
                                        NxPackageVerifyResult* out_result)
{
    NxPackageVerifyResult result;
    char manifest_path[NX_PM_PATH_MAX];
    FILE* manifest;
    char line[NX_PM_LINE_MAX];

    memset(&result, 0, sizeof(result));
    nx_pm_copy_string(result.message, sizeof(result.message), "Dependency verification failed.");

    if (repo_root == NULL || package_dir == NULL ||
        !nx_pm_join(manifest_path, sizeof(manifest_path), package_dir, "manifest.npkg") ||
        !nx_pm_exists(manifest_path)) {
        if (out_result != NULL) {
            *out_result = result;
        }
        return 0;
    }

    result.manifest_found = 1;
    if (!nx_pm_read_manifest_identity(manifest_path, result.package_id, sizeof(result.package_id),
                                      result.package_version, sizeof(result.package_version))) {
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
        if (nx_pm_starts_with(trimmed, "requires=")) {
            char* dependency = nx_pm_trim(trimmed + 9);
            result.dependencies_declared++;
            if (nx_pm_dependency_installed(repo_root, dependency)) {
                result.dependencies_satisfied++;
            } else {
                result.dependencies_missing++;
            }
        }
    }
    fclose(manifest);

    result.success = result.dependencies_missing == 0;
    if (result.success) {
        nx_pm_copy_string(result.message, sizeof(result.message),
                          result.dependencies_declared == 0 ? "Package has no dependencies." : "All package dependencies are installed.");
    } else {
        nx_pm_copy_string(result.message, sizeof(result.message), "Package has missing dependencies.");
    }

    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}


static int nx_pm_remove_file(const char* path)
{
    return path != NULL && remove(path) == 0;
}

static int nx_pm_transaction_dir(const char* repo_root,
                                 const char* package_id,
                                 char* out,
                                 size_t out_size)
{
    char knowledge[NX_PM_PATH_MAX];
    char ncos[NX_PM_PATH_MAX];
    char transactions[NX_PM_PATH_MAX];

    if (!nx_pm_join(knowledge, sizeof(knowledge), repo_root, "Knowledge") ||
        !nx_pm_join(ncos, sizeof(ncos), knowledge, "NCOS") ||
        !nx_pm_join(transactions, sizeof(transactions), ncos, "PackageTransactions") ||
        !nx_pm_join(out, out_size, transactions, package_id) ||
        !nx_pm_mkdir_chain(out)) {
        return 0;
    }
    return 1;
}

static int nx_pm_transaction_path(const char* repo_root,
                                  const char* package_id,
                                  char* out,
                                  size_t out_size)
{
    char dir[NX_PM_PATH_MAX];
    if (!nx_pm_transaction_dir(repo_root, package_id, dir, sizeof(dir))) {
        return 0;
    }
    return nx_pm_join(out, out_size, dir, "transaction.log");
}

static int nx_pm_pending_transaction_path(const char* repo_root,
                                          const char* package_id,
                                          char* out,
                                          size_t out_size)
{
    char dir[NX_PM_PATH_MAX];
    if (!nx_pm_transaction_dir(repo_root, package_id, dir, sizeof(dir))) {
        return 0;
    }
    return nx_pm_join(out, out_size, dir, "transaction.pending.log");
}

static int nx_pm_commit_transaction(const char* pending_path, const char* committed_path)
{
    if (pending_path == NULL || committed_path == NULL) {
        return 0;
    }
    if (nx_pm_exists(committed_path)) {
        (void)remove(committed_path);
    }
    return rename(pending_path, committed_path) == 0;
}

static int nx_pm_history_dir(const char* repo_root,
                             const char* package_id,
                             char* out,
                             size_t out_size)
{
    char knowledge[NX_PM_PATH_MAX];
    char ncos[NX_PM_PATH_MAX];
    char histories[NX_PM_PATH_MAX];

    if (!nx_pm_join(knowledge, sizeof(knowledge), repo_root, "Knowledge") ||
        !nx_pm_join(ncos, sizeof(ncos), knowledge, "NCOS") ||
        !nx_pm_join(histories, sizeof(histories), ncos, "PackageHistory") ||
        !nx_pm_join(out, out_size, histories, package_id) ||
        !nx_pm_mkdir_chain(out)) {
        return 0;
    }
    return 1;
}

static int nx_pm_history_index_path(const char* repo_root,
                                    const char* package_id,
                                    char* out,
                                    size_t out_size)
{
    char dir[NX_PM_PATH_MAX];
    if (!nx_pm_history_dir(repo_root, package_id, dir, sizeof(dir))) {
        return 0;
    }
    return nx_pm_join(out, out_size, dir, "history.index");
}

static int nx_pm_read_next_transaction(const char* counter_path)
{
    FILE* f;
    int value = 1;
    f = fopen(counter_path, "rb");
    if (f != NULL) {
        if (fscanf(f, "%d", &value) != 1 || value < 1) {
            value = 1;
        }
        fclose(f);
    }
    return value;
}

static int nx_pm_archive_transaction(const char* repo_root,
                                     const char* package_id,
                                     const char* version,
                                     const char* tx_path,
                                     int files_changed,
                                     NxPackageInstallResult* result)
{
    char dir[NX_PM_PATH_MAX];
    char counter[NX_PM_PATH_MAX];
    char history[NX_PM_PATH_MAX];
    char archive[NX_PM_PATH_MAX];
    char name[64];
    char text[256];
    int id;
    int written;

    if (!nx_pm_history_dir(repo_root, package_id, dir, sizeof(dir)) ||
        !nx_pm_join(counter, sizeof(counter), dir, "next.txt") ||
        !nx_pm_join(history, sizeof(history), dir, "history.index")) {
        return 0;
    }

    id = nx_pm_read_next_transaction(counter);
    written = snprintf(name, sizeof(name), "tx-%06d.log", id);
    if (written <= 0 || (size_t)written >= sizeof(name) ||
        !nx_pm_join(archive, sizeof(archive), dir, name) ||
        !nx_pm_copy_file(tx_path, archive)) {
        return 0;
    }

    written = snprintf(text, sizeof(text), "%06d|%s|files=%d|%s\n",
                       id, version != NULL ? version : "0.0.0", files_changed, archive);
    if (written <= 0 || (size_t)written >= sizeof(text) ||
        !nx_pm_append_text(history, text)) {
        return 0;
    }

    written = snprintf(text, sizeof(text), "%d\n", id + 1);
    if (written <= 0 || (size_t)written >= sizeof(text) ||
        !nx_pm_write_text(counter, text)) {
        return 0;
    }

    if (result != NULL) {
        (void)snprintf(result->transaction_id, sizeof(result->transaction_id), "%06d", id);
        (void)nx_pm_copy_string(result->history_path, sizeof(result->history_path), history);
    }
    return 1;
}

static int nx_pm_rollback_transaction(const char* tx_path, NxPackageInstallResult* result)
{
    FILE* f; char line[NX_PM_LINE_MAX]; int ok = 1;
    f = fopen(tx_path, "rb"); if (f == NULL) return 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        char* t = nx_pm_trim(line);
        if (nx_pm_starts_with(t, "RESTORE|")) {
            char* sep = strchr(t + 8, '|');
            if (sep != NULL) { *sep='\0'; if (!nx_pm_copy_file(t+8, sep+1)) ok=0; else result->files_rolled_back++; }
        } else if (nx_pm_starts_with(t, "DELETE|")) {
            if (nx_pm_exists(t+7) && !nx_pm_remove_file(t+7)) ok=0; else result->files_rolled_back++;
        }
    }
    fclose(f); return ok;
}
int NxPackageManager_Install(const char* repo_root,
                             const char* package_dir,
                             NxPackageInstallResult* out_result)
{
    NxPackageInstallResult result;
    NxPackageVerifyResult verify_result;
    char manifest_path[NX_PM_PATH_MAX];
    char package_id[128];
    char package_version[64];
    char registry_path[NX_PM_PATH_MAX];
    char log_path[NX_PM_PATH_MAX];
    char tx_path[NX_PM_PATH_MAX];
    char pending_tx_path[NX_PM_PATH_MAX];
    FILE* manifest = NULL;
    char line[NX_PM_LINE_MAX];
    int ok = 1;

    memset(&result, 0, sizeof(result));
    memset(&verify_result, 0, sizeof(verify_result));

    if (repo_root == NULL || package_dir == NULL ||
        !NxPackageManager_VerifyPackage(package_dir, &verify_result) ||
        !NxPackageManager_VerifyDependencies(repo_root, package_dir, &verify_result)) {
        goto done;
    }

    if (!nx_pm_join(manifest_path, sizeof(manifest_path), package_dir, "manifest.npkg") ||
        !nx_pm_read_manifest_identity(manifest_path, package_id, sizeof(package_id),
                                      package_version, sizeof(package_version)) ||
        !nx_pm_registry_paths(repo_root, package_id, registry_path, sizeof(registry_path),
                              log_path, sizeof(log_path)) ||
        !nx_pm_transaction_path(repo_root, package_id, tx_path, sizeof(tx_path)) ||
        !nx_pm_pending_transaction_path(repo_root, package_id, pending_tx_path,
                                        sizeof(pending_tx_path))) {
        goto done;
    }

    nx_pm_copy_string(result.package_id, sizeof(result.package_id), package_id);
    nx_pm_copy_string(result.package_version, sizeof(result.package_version), package_version);
    nx_pm_copy_string(result.registry_path, sizeof(result.registry_path), registry_path);
    nx_pm_copy_string(result.install_log_path, sizeof(result.install_log_path), log_path);
    nx_pm_copy_string(result.transaction_path, sizeof(result.transaction_path), tx_path);

    nx_pm_write_text(log_path, "Nexiora Package Manager transactional install log\n");
    nx_pm_write_text(pending_tx_path, "BEGIN\n");

    manifest = fopen(manifest_path, "rb");
    if (manifest == NULL) {
        ok = 0;
        goto finalize;
    }

    while (fgets(line, sizeof(line), manifest) != NULL) {
        char* trimmed = nx_pm_trim(line);
        char src_rel[NX_PM_PATH_MAX];
        char dst_rel[NX_PM_PATH_MAX];
        char src_path[NX_PM_PATH_MAX];
        char dst_path[NX_PM_PATH_MAX];
        char parent[NX_PM_PATH_MAX];
        char journal[NX_PM_LINE_MAX];
        unsigned long long source_hash;
        unsigned long long destination_hash;

        if (!nx_pm_parse_file_mapping(trimmed, src_rel, sizeof(src_rel),
                                      dst_rel, sizeof(dst_rel))) {
            continue;
        }

        result.files_declared++;
        if (!nx_pm_join(src_path, sizeof(src_path), package_dir, src_rel) ||
            !nx_pm_join(dst_path, sizeof(dst_path), repo_root, dst_rel) ||
            !nx_pm_parent_dir(dst_path, parent, sizeof(parent)) ||
            !nx_pm_mkdir_chain(parent)) {
            ok = 0;
            break;
        }

        source_hash = nx_pm_hash_file(src_path);
        destination_hash = nx_pm_hash_file(dst_path);
        if (source_hash != 0ull && source_hash == destination_hash) {
            result.files_skipped++;
            continue;
        }

        if (nx_pm_exists(dst_path)) {
            char backup[NX_PM_PATH_MAX];
            int written;
            if (!nx_pm_make_backup_path(repo_root, package_id, dst_rel,
                                        backup, sizeof(backup)) ||
                !nx_pm_copy_file(dst_path, backup)) {
                ok = 0;
                break;
            }
            result.files_backed_up++;
            written = snprintf(journal, sizeof(journal), "RESTORE|%s|%s\n", backup, dst_path);
            if (written <= 0 || (size_t)written >= sizeof(journal) ||
                !nx_pm_append_text(pending_tx_path, journal)) {
                ok = 0;
                break;
            }
        } else {
            int written = snprintf(journal, sizeof(journal), "DELETE|%s\n", dst_path);
            if (written <= 0 || (size_t)written >= sizeof(journal) ||
                !nx_pm_append_text(pending_tx_path, journal)) {
                ok = 0;
                break;
            }
        }

        if (!nx_pm_copy_file(src_path, dst_path)) {
            ok = 0;
            break;
        }
        result.files_installed++;
    }

finalize:
    if (manifest != NULL) {
        fclose(manifest);
    }

    if (!ok) {
        (void)nx_pm_rollback_transaction(pending_tx_path, &result);
        (void)nx_pm_append_text(pending_tx_path, "ROLLED_BACK\n");
    } else if (result.files_installed > 0) {
        if (!nx_pm_append_text(pending_tx_path, "COMMIT\n") ||
            !nx_pm_commit_transaction(pending_tx_path, tx_path)) {
            ok = 0;
        } else if (!nx_pm_archive_transaction(repo_root, package_id, package_version,
                                              tx_path, result.files_installed, &result)) {
            ok = 0;
        } else {
            result.transaction_committed = 1;
        }
    } else {
        /* A no-op installation must not destroy the last rollbackable transaction. */
        (void)remove(pending_tx_path);
        result.transaction_committed = nx_pm_exists(tx_path) ? 1 : 0;
    }

    result.success = ok;
    {
        char text[1200];
        int written = snprintf(text, sizeof(text),
                               "id=%s\nversion=%s\nstatus=%s\ntransaction=%s\n"
                               "files_installed=%d\nfiles_skipped=%d\nfiles_rolled_back=%d\n",
                               result.package_id,
                               result.package_version,
                               ok ? "installed" : "rolled_back",
                               tx_path,
                               result.files_installed,
                               result.files_skipped,
                               result.files_rolled_back);
        if (written > 0 && (size_t)written < sizeof(text)) {
            (void)nx_pm_write_text(registry_path, text);
        }
    }

done:
    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}

int NxPackageManager_Rollback(const char* repo_root,
                              const char* package_id,
                              NxPackageInstallResult* out_result)
{
    NxPackageInstallResult result; char safe[128], tx[NX_PM_PATH_MAX];
    memset(&result,0,sizeof(result));
    if(repo_root==NULL || package_id==NULL) goto done;
    nx_pm_normalize_id(safe,sizeof(safe),package_id);
    nx_pm_copy_string(result.package_id,sizeof(result.package_id),safe);
    if(!nx_pm_transaction_path(repo_root,safe,tx,sizeof(tx))) goto done;
    nx_pm_copy_string(result.transaction_path,sizeof(result.transaction_path),tx);
    result.success=nx_pm_rollback_transaction(tx,&result);
    if(result.success) nx_pm_append_text(tx,"MANUAL_ROLLBACK\n");
done:
    if(out_result!=NULL)*out_result=result;
    return result.success;
}

int NxPackageManager_History(const char* repo_root,
                             const char* package_id,
                             NxPackageHistoryResult* out_result)
{
    NxPackageHistoryResult result;
    char safe[128];
    char index_path[NX_PM_PATH_MAX];
    FILE* f;
    char line[NX_PM_LINE_MAX];

    memset(&result, 0, sizeof(result));
    if (repo_root == NULL || package_id == NULL) {
        goto done;
    }

    nx_pm_normalize_id(safe, sizeof(safe), package_id);
    if (safe[0] == '\0' ||
        !nx_pm_history_index_path(repo_root, safe, index_path, sizeof(index_path))) {
        goto done;
    }

    (void)nx_pm_copy_string(result.package_id, sizeof(result.package_id), safe);
    (void)nx_pm_copy_string(result.history_path, sizeof(result.history_path), index_path);

    f = fopen(index_path, "rb");
    if (f == NULL) {
        goto done;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        if (nx_pm_trim(line)[0] != '\0') {
            result.entries++;
        }
    }
    fclose(f);
    result.success = result.entries > 0;

done:
    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}

int NxPackageManager_RollbackTransaction(const char* repo_root,
                                         const char* package_id,
                                         const char* transaction_id,
                                         NxPackageInstallResult* out_result)
{
    NxPackageInstallResult result;
    char safe[128];
    char dir[NX_PM_PATH_MAX];
    char tx_path[NX_PM_PATH_MAX];
    char tx_name[64];
    long id;
    char* end = NULL;
    int written;

    memset(&result, 0, sizeof(result));
    if (repo_root == NULL || package_id == NULL || transaction_id == NULL) {
        goto done;
    }

    id = strtol(transaction_id, &end, 10);
    if (end == transaction_id || *end != '\0' || id < 1 || id > 999999) {
        goto done;
    }

    nx_pm_normalize_id(safe, sizeof(safe), package_id);
    if (safe[0] == '\0' || !nx_pm_history_dir(repo_root, safe, dir, sizeof(dir))) {
        goto done;
    }

    written = snprintf(tx_name, sizeof(tx_name), "tx-%06ld.log", id);
    if (written <= 0 || (size_t)written >= sizeof(tx_name) ||
        !nx_pm_join(tx_path, sizeof(tx_path), dir, tx_name)) {
        goto done;
    }

    (void)nx_pm_copy_string(result.package_id, sizeof(result.package_id), safe);
    (void)snprintf(result.transaction_id, sizeof(result.transaction_id), "%06ld", id);
    (void)nx_pm_copy_string(result.transaction_path, sizeof(result.transaction_path), tx_path);
    result.success = nx_pm_rollback_transaction(tx_path, &result);
    if (result.success) {
        (void)nx_pm_append_text(tx_path, "HISTORICAL_ROLLBACK\n");
    }

done:
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


/* NCOS-021 - Native Package Apply Workflow */
static int nx_pm_apply_join3(char* out, size_t cap,
                             const char* a, const char* b, const char* d)
{
    char temp[NX_PM_PATH_MAX];
    return nx_pm_join(temp, sizeof(temp), a, b) &&
           nx_pm_join(out, cap, temp, d);
}

static int nx_pm_apply_read_value(const char* package_dir,
                                  const char* key,
                                  char* out,
                                  size_t out_cap,
                                  const char* fallback)
{
    char manifest_path[NX_PM_PATH_MAX];
    FILE* f;
    char line[NX_PM_LINE_MAX];
    size_t key_len;

    if (out == NULL || out_cap == 0u || key == NULL) {
        return 0;
    }
    out[0] = '\0';
    if (package_dir == NULL ||
        !nx_pm_join(manifest_path, sizeof(manifest_path), package_dir, "manifest.npkg")) {
        return nx_pm_copy_string(out, out_cap, fallback != NULL ? fallback : "");
    }
    f = fopen(manifest_path, "rb");
    if (f == NULL) {
        return nx_pm_copy_string(out, out_cap, fallback != NULL ? fallback : "");
    }
    key_len = strlen(key);
    while (fgets(line, sizeof(line), f) != NULL) {
        char* trimmed = nx_pm_trim(line);
        if (strncmp(trimmed, key, key_len) == 0 && trimmed[key_len] == '=') {
            char* value = nx_pm_trim(trimmed + key_len + 1u);
            int ok = nx_pm_copy_string(out, out_cap, value);
            fclose(f);
            return ok;
        }
    }
    fclose(f);
    return nx_pm_copy_string(out, out_cap, fallback != NULL ? fallback : "");
}

static int nx_pm_apply_append_phase(const char* log_path,
                                    const char* phase,
                                    const char* state)
{
    char line[256];
    int written = snprintf(line, sizeof(line), "[NCOS-021] phase=%s state=%s\n",
                           phase != NULL ? phase : "unknown",
                           state != NULL ? state : "unknown");
    return written > 0 && (size_t)written < sizeof(line) &&
           nx_pm_append_text(log_path, line);
}

#if defined(_WIN32)
static int nx_pm_apply_run(const char* repo_root,
                           const char* executable,
                           const char* const* arguments,
                           size_t argument_count,
                           const char* log_path)
{
    HANDLE log_handle;
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    SECURITY_ATTRIBUTES security;
    char command[4096];
    size_t used = 0u;
    size_t i;
    DWORD exit_code = 1u;

    if (repo_root == NULL || executable == NULL || log_path == NULL) {
        return 0;
    }
    command[0] = '\0';
    if (snprintf(command, sizeof(command), "\"%s\"", executable) <= 0) {
        return 0;
    }
    used = strlen(command);
    for (i = 0u; i < argument_count; ++i) {
        int written;
        if (arguments[i] == NULL || strchr(arguments[i], '"') != NULL) {
            return 0;
        }
        written = snprintf(command + used, sizeof(command) - used, " \"%s\"", arguments[i]);
        if (written <= 0 || (size_t)written >= sizeof(command) - used) {
            return 0;
        }
        used += (size_t)written;
    }

    memset(&security, 0, sizeof(security));
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;
    log_handle = CreateFileA(log_path, FILE_APPEND_DATA,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (log_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    memset(&startup, 0, sizeof(startup));
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log_handle;
    startup.hStdError = log_handle;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    memset(&process, 0, sizeof(process));

    if (!CreateProcessA(NULL, command, NULL, NULL, TRUE, 0u, NULL, repo_root,
                        &startup, &process)) {
        CloseHandle(log_handle);
        return 0;
    }
    CloseHandle(process.hThread);
    WaitForSingleObject(process.hProcess, INFINITE);
    if (!GetExitCodeProcess(process.hProcess, &exit_code)) {
        exit_code = 1u;
    }
    CloseHandle(process.hProcess);
    CloseHandle(log_handle);
    return exit_code == 0u;
}
#else
static int nx_pm_apply_run(const char* repo_root,
                           const char* executable,
                           const char* const* arguments,
                           size_t argument_count,
                           const char* log_path)
{
    pid_t child;
    int status = 0;
    char* argv[16];
    size_t i;
    int fd;

    if (repo_root == NULL || executable == NULL || log_path == NULL || argument_count > 13u) {
        return 0;
    }
    argv[0] = (char*)executable;
    for (i = 0u; i < argument_count; ++i) {
        argv[i + 1u] = (char*)arguments[i];
    }
    argv[argument_count + 1u] = NULL;
    fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        return 0;
    }
    child = fork();
    if (child == 0) {
        if (chdir(repo_root) != 0 || dup2(fd, STDOUT_FILENO) < 0 ||
            dup2(fd, STDERR_FILENO) < 0) {
            _exit(126);
        }
        close(fd);
        execvp(executable, argv);
        _exit(127);
    }
    close(fd);
    if (child < 0 || waitpid(child, &status, 0) < 0) {
        return 0;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
#endif

int NxPackageManager_ApplyLogHasWarnings(const char* log_path)
{
    FILE* f;
    char line[NX_PM_LINE_MAX];
    if (log_path == NULL) {
        return 1;
    }
    f = fopen(log_path, "rb");
    if (f == NULL) {
        return 1;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char lowered[NX_PM_LINE_MAX];
        size_t i;
        for (i = 0u; i + 1u < sizeof(lowered) && line[i] != '\0'; ++i) {
            lowered[i] = (char)tolower((unsigned char)line[i]);
        }
        lowered[i] = '\0';
        if (strstr(lowered, "warning:") != NULL ||
            strstr(lowered, "cmake warning") != NULL ||
            strstr(lowered, " warning c") != NULL) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

static int nx_pm_apply_docs(const char* repo_root, const char* log_path)
{
    char docs_executable[NX_PM_PATH_MAX];
    const char* args[2];
#if defined(_WIN32)
    const char* name = "nexiora_docs.exe";
#else
    const char* name = "nexiora_docs";
#endif
    if (!nx_pm_apply_join3(docs_executable, sizeof(docs_executable), repo_root,
                           "Build/windows-msvc-release/bin", name)) {
        return 0;
    }
    if (!nx_pm_exists(docs_executable)) {
        return 1;
    }
    args[0] = "validate";
    args[1] = repo_root;
    return nx_pm_apply_run(repo_root, docs_executable, args, 2u, log_path);
}

int NxPackageManager_Apply(const char* repo_root,
                           const char* package_dir,
                           NxPackageApplyResult* out_result)
{
    NxPackageApplyResult result;
    NxPackageVerifyResult verify;
    NxPackageInstallResult install;
    NxPackageInstallResult rollback;
    char configure_preset[64];
    char build_preset[64];
    char test_target[128];
    char test_preset[64];
    char apply_dir[NX_PM_PATH_MAX];
    char safe_id[128];
    const char* configure_args[2];
    const char* build_args[5];
    const char* test_args[3];
    size_t build_arg_count = 3u;
    int ok = 0;
    int qa_repetitions = 2;
    int qa_passed = 0;
    time_t certification_started = time(NULL);
    NxPackageCertificationSummary certification;

    memset(&result, 0, sizeof(result));
    memset(&verify, 0, sizeof(verify));
    memset(&install, 0, sizeof(install));
    memset(&rollback, 0, sizeof(rollback));
    memset(&certification, 0, sizeof(certification));
    nx_pm_copy_string(result.message, sizeof(result.message), "Apply failed before verification.");

    if (repo_root == NULL || package_dir == NULL ||
        !NxPackageManager_VerifyPackage(package_dir, &verify)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "verify");
        goto done;
    }
    result.verify_passed = 1;
    nx_pm_copy_string(result.package_id, sizeof(result.package_id), verify.package_id);
    nx_pm_copy_string(result.package_version, sizeof(result.package_version), verify.package_version);
    nx_pm_normalize_id(safe_id, sizeof(safe_id), verify.package_id);
    if (!nx_pm_apply_join3(apply_dir, sizeof(apply_dir), repo_root,
                           "Knowledge/NCOS/PackageApply", safe_id) ||
        !nx_pm_mkdir_chain(apply_dir) ||
        !nx_pm_join(result.apply_log_path, sizeof(result.apply_log_path), apply_dir, "apply.log") ||
        !nx_pm_write_text(result.apply_log_path, "NCOS-021 native apply\n")) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "log");
        goto done;
    }
    nx_pm_apply_append_phase(result.apply_log_path, "verify", "passed");

    if (!NxPackageManager_VerifyDependencies(repo_root, package_dir, &verify)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "dependencies");
        nx_pm_copy_string(result.message, sizeof(result.message), verify.message);
        goto done;
    }
    result.dependencies_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "dependencies", "passed");

    if (!NxPackageManager_Install(repo_root, package_dir, &install)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "install");
        nx_pm_copy_string(result.message, sizeof(result.message), "Transactional installation failed.");
        goto done;
    }
    result.install_passed = 1;
    nx_pm_copy_string(result.transaction_id, sizeof(result.transaction_id), install.transaction_id);
    nx_pm_apply_append_phase(result.apply_log_path, "install", "passed");

    nx_pm_apply_read_value(package_dir, "apply.configure_preset", configure_preset,
                           sizeof(configure_preset), "windows-msvc-release");
    nx_pm_apply_read_value(package_dir, "apply.build_preset", build_preset,
                           sizeof(build_preset), "release");
    nx_pm_apply_read_value(package_dir, "apply.test_target", test_target,
                           sizeof(test_target), "nexiora_test_suite");
    nx_pm_apply_read_value(package_dir, "apply.test_preset", test_preset,
                           sizeof(test_preset), "release-tests");

    configure_args[0] = "--preset";
    configure_args[1] = configure_preset;
    nx_pm_apply_append_phase(result.apply_log_path, "configure", "started");
    if (!nx_pm_apply_run(repo_root, "cmake", configure_args, 2u, result.apply_log_path)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "configure");
        nx_pm_copy_string(result.message, sizeof(result.message), "CMake configuration failed.");
        goto rollback_install;
    }
    result.configure_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "configure", "passed");

    build_args[0] = "--build";
    build_args[1] = "--preset";
    build_args[2] = build_preset;
    if (test_target[0] != '\0' && strcmp(test_target, "none") != 0) {
        build_args[3] = "--target";
        build_args[4] = test_target;
        build_arg_count = 5u;
    }
    nx_pm_apply_append_phase(result.apply_log_path, "build", "started");
    if (!nx_pm_apply_run(repo_root, "cmake", build_args, build_arg_count, result.apply_log_path)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "build");
        nx_pm_copy_string(result.message, sizeof(result.message), "Full build or test materialization failed.");
        goto rollback_install;
    }
    result.build_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "build", "passed");

    if (NxPackageManager_ApplyLogHasWarnings(result.apply_log_path)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "warning-gate");
        nx_pm_copy_string(result.message, sizeof(result.message), "Warning gate rejected the build log.");
        goto rollback_install;
    }
    result.warning_gate_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "warning-gate", "passed");

    test_args[0] = "--preset";
    test_args[1] = test_preset;
    test_args[2] = "--output-on-failure";
    nx_pm_apply_append_phase(result.apply_log_path, "tests", "started");
    if (!nx_pm_apply_run(repo_root, "ctest", test_args, 3u, result.apply_log_path)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "tests");
        nx_pm_copy_string(result.message, sizeof(result.message), "CTest suite failed.");
        goto rollback_install;
    }
    result.tests_passed = 1;
    qa_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "tests", "passed");

    {
        char repetitions_text[32];
        int repetition;
        nx_pm_apply_read_value(package_dir, "apply.qa_repetitions", repetitions_text,
                               sizeof(repetitions_text), "2");
        qa_repetitions = atoi(repetitions_text);
        if (qa_repetitions < 1) {
            qa_repetitions = 1;
        }
        if (qa_repetitions > 5) {
            qa_repetitions = 5;
        }
        for (repetition = 2; repetition <= qa_repetitions; ++repetition) {
            nx_pm_apply_append_phase(result.apply_log_path, "qa-repeatability", "started");
            if (!nx_pm_apply_run(repo_root, "ctest", test_args, 3u, result.apply_log_path)) {
                nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "qa-repeatability");
                nx_pm_copy_string(result.message, sizeof(result.message),
                                  "Repeated CTest execution failed; tests are not hermetic.");
                goto rollback_install;
            }
            qa_passed = repetition;
            nx_pm_apply_append_phase(result.apply_log_path, "qa-repeatability", "passed");
        }
    }
    result.qa_repeatability_passed = (qa_passed == qa_repetitions);

    if (!NxPackageCertification_ValidateArtifacts(repo_root, package_dir, &certification)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "artifacts");
        nx_pm_copy_string(result.message, sizeof(result.message),
                          "Required artifact validation failed.");
        goto rollback_install;
    }
    result.artifacts_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "artifacts", "passed");

    nx_pm_apply_append_phase(result.apply_log_path, "documentation", "started");
    if (!nx_pm_apply_docs(repo_root, result.apply_log_path)) {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "documentation");
        nx_pm_copy_string(result.message, sizeof(result.message), "Documentation synchronization validation failed.");
        goto rollback_install;
    }
    result.documentation_passed = 1;
    nx_pm_apply_append_phase(result.apply_log_path, "documentation", "passed");

    result.success = 1;
    nx_pm_copy_string(result.message, sizeof(result.message),
                      "Package applied and QA certification completed.");
    nx_pm_apply_append_phase(result.apply_log_path, "apply", "completed");
    if (!NxPackageCertification_Generate(repo_root, package_dir, &result, certification_started,
                                         time(NULL), qa_repetitions, qa_passed, &certification)) {
        result.success = 0;
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "certification");
        nx_pm_copy_string(result.message, sizeof(result.message),
                          "Certification report generation failed.");
        goto rollback_install;
    }
    result.certification_generated = 1;
    nx_pm_copy_string(result.certification_report_path, sizeof(result.certification_report_path),
                      certification.nxcert_path);
    nx_pm_copy_string(result.certification_text_path, sizeof(result.certification_text_path),
                      certification.text_path);
    nx_pm_copy_string(result.release_recommendation, sizeof(result.release_recommendation),
                      certification.release_recommendation);
    ok = 1;
    goto done;

rollback_install:
    nx_pm_apply_append_phase(result.apply_log_path, result.failed_phase, "failed");
    if (install.transaction_id[0] != '\0' &&
        NxPackageManager_RollbackTransaction(repo_root, install.package_id,
                                             install.transaction_id, &rollback)) {
        result.rolled_back = 1;
        nx_pm_apply_append_phase(result.apply_log_path, "rollback", "passed");
    } else if (install.files_installed == 0) {
        result.rolled_back = 1;
        nx_pm_apply_append_phase(result.apply_log_path, "rollback", "not-required");
    } else {
        nx_pm_apply_append_phase(result.apply_log_path, "rollback", "failed");
    }

done:
    if (!ok && result.failed_phase[0] == '\0') {
        nx_pm_copy_string(result.failed_phase, sizeof(result.failed_phase), "unknown");
    }
    if (!result.certification_generated && result.apply_log_path[0] != '\0') {
        if (NxPackageCertification_Generate(repo_root, package_dir, &result, certification_started,
                                            time(NULL), qa_repetitions, qa_passed, &certification)) {
            result.certification_generated = 1;
            nx_pm_copy_string(result.certification_report_path, sizeof(result.certification_report_path),
                              certification.nxcert_path);
            nx_pm_copy_string(result.certification_text_path, sizeof(result.certification_text_path),
                              certification.text_path);
            nx_pm_copy_string(result.release_recommendation, sizeof(result.release_recommendation),
                              certification.release_recommendation);
        }
    }
    if (out_result != NULL) {
        *out_result = result;
    }
    return result.success;
}
