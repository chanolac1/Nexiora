#include "Nexiora/NCOS/NxDocumentationGovernance.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define NX_DOC_PATH_MAX 1024

static const char* const nx_required_docs[] = {
    "MASTER_CONTEXT.md", "PROJECT_STATE.md", "ROADMAP.md", "CHANGELOG.md",
    "DECISIONS.md", "ARCHITECTURE.md", "CODING_STANDARD.md",
    "TESTING_STANDARD.md", "PACKAGE_STANDARD.md"
};

static int nx_join(char* out, size_t size, const char* root, const char* suffix)
{
    int written;
    if (out == NULL || size == 0U || root == NULL || suffix == NULL) return 0;
    written = snprintf(out, size, "%s/%s", root, suffix);
    return written >= 0 && (size_t)written < size;
}

static int nx_exists(const char* path)
{
    struct stat info;
    return path != NULL && stat(path, &info) == 0;
}

static int nx_contains(const char* path, const char* token)
{
    FILE* file;
    char line[1024];
    if (path == NULL || token == NULL) return 0;
    file = fopen(path, "rb");
    if (file == NULL) return 0;
    while (fgets(line, (int)sizeof(line), file) != NULL) {
        if (strstr(line, token) != NULL) { (void)fclose(file); return 1; }
    }
    (void)fclose(file);
    return 0;
}

static int nx_append(const char* path, const char* text)
{
    FILE* file;
    if (path == NULL || text == NULL) return 0;
    file = fopen(path, "ab");
    if (file == NULL) return 0;
    if (fputs(text, file) == EOF) { (void)fclose(file); return 0; }
    return fclose(file) == 0;
}

int NxDocumentation_Validate(const char* repo_root, NxDocumentationValidationResult* out_result)
{
    NxDocumentationValidationResult result = {0};
    char path[NX_DOC_PATH_MAX];
    size_t index;
    if (repo_root == NULL || out_result == NULL) return 0;
    for (index = 0U; index < sizeof(nx_required_docs) / sizeof(nx_required_docs[0]); ++index) {
        char relative[NX_DOC_PATH_MAX];
        int written = snprintf(relative, sizeof(relative), "Docs/%s", nx_required_docs[index]);
        result.files_checked++;
        if (written < 0 || (size_t)written >= sizeof(relative) || !nx_join(path, sizeof(path), repo_root, relative) || !nx_exists(path)) result.files_missing++;
    }
    if (nx_join(path, sizeof(path), repo_root, "Docs/PROJECT_STATE.md") && nx_exists(path) && !nx_contains(path, "# PROJECT STATE")) result.structural_errors++;
    if (nx_join(path, sizeof(path), repo_root, "Docs/CHANGELOG.md") && nx_exists(path) && !nx_contains(path, "# CHANGELOG")) result.structural_errors++;
    if (nx_join(path, sizeof(path), repo_root, "Docs/DECISIONS.md") && nx_exists(path) && !nx_contains(path, "# DECISIONS")) result.structural_errors++;
    result.success = result.files_missing == 0 && result.structural_errors == 0;
    (void)snprintf(result.message, sizeof(result.message), "checked=%d missing=%d structural_errors=%d", result.files_checked, result.files_missing, result.structural_errors);
    *out_result = result;
    return result.success;
}

int NxDocumentation_CompleteSprint(const char* repo_root, const char* sprint, const char* capability, const char* date)
{
    char state_path[NX_DOC_PATH_MAX];
    char changelog_path[NX_DOC_PATH_MAX];
    char entry[1024];
    int written;
    if (repo_root == NULL || sprint == NULL || capability == NULL || date == NULL || sprint[0] == '\0' || capability[0] == '\0' || date[0] == '\0') return 0;
    if (!nx_join(state_path, sizeof(state_path), repo_root, "Docs/PROJECT_STATE.md") || !nx_join(changelog_path, sizeof(changelog_path), repo_root, "Docs/CHANGELOG.md")) return 0;
    written = snprintf(entry, sizeof(entry), "\n## Finalización automática — %s\n\n- Fecha: %s\n- Capacidad: %s\n", sprint, date, capability);
    if (written < 0 || (size_t)written >= sizeof(entry) || !nx_append(state_path, entry)) return 0;
    written = snprintf(entry, sizeof(entry), "\n## [%s] — %s\n\n### Added\n\n- %s\n", sprint, date, capability);
    return written >= 0 && (size_t)written < sizeof(entry) && nx_append(changelog_path, entry);
}

int NxDocumentation_RecordDecision(const char* repo_root, const char* decision_id, const char* title, const char* body, const char* date)
{
    char path[NX_DOC_PATH_MAX];
    char entry[2048];
    int written;
    if (repo_root == NULL || decision_id == NULL || title == NULL || body == NULL || date == NULL) return 0;
    if (!nx_join(path, sizeof(path), repo_root, "Docs/DECISIONS.md")) return 0;
    written = snprintf(entry, sizeof(entry), "\n## %s — %s\n\n**Estado:** Aceptada  \n**Fecha:** %s\n\n%s\n", decision_id, title, date, body);
    return written >= 0 && (size_t)written < sizeof(entry) && nx_append(path, entry);
}
