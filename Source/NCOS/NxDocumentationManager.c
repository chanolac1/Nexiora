#include "Nexiora/NCOS/NxDocumentationManager.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define NX_DOC_PATH_MAX 1024
#define NX_DOC_TEXT_MAX 131072

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

static int nx_read_all(const char* path, char* out, size_t capacity)
{
    FILE* file;
    size_t count;
    if (path == NULL || out == NULL || capacity < 2U) return 0;
    file = fopen(path, "rb");
    if (file == NULL) return 0;
    count = fread(out, 1U, capacity - 1U, file);
    if (ferror(file) != 0 || !feof(file)) { (void)fclose(file); return 0; }
    out[count] = '\0';
    return fclose(file) == 0;
}

static int nx_write_all(const char* path, const char* text)
{
    FILE* file;
    size_t length;
    if (path == NULL || text == NULL) return 0;
    file = fopen(path, "wb");
    if (file == NULL) return 0;
    length = strlen(text);
    if (fwrite(text, 1U, length, file) != length) { (void)fclose(file); return 0; }
    return fclose(file) == 0;
}

static void nx_copy_text(char* destination, size_t capacity, const char* source)
{
    if (destination == NULL || capacity == 0U) return;
    if (source == NULL) { destination[0] = '\0'; return; }
    (void)snprintf(destination, capacity, "%s", source);
}

static char* nx_trim(char* text)
{
    char* end;
    while (*text != '\0' && isspace((unsigned char)*text) != 0) ++text;
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1]) != 0) --end;
    *end = '\0';
    return text;
}

static int nx_set_release_value(NxDocumentationRelease* release, const char* key, const char* value)
{
    if (strcmp(key, "sprint_id") == 0) nx_copy_text(release->sprint_id, sizeof(release->sprint_id), value);
    else if (strcmp(key, "sprint_name") == 0) nx_copy_text(release->sprint_name, sizeof(release->sprint_name), value);
    else if (strcmp(key, "completion_date") == 0) nx_copy_text(release->completion_date, sizeof(release->completion_date), value);
    else if (strcmp(key, "next_sprint_id") == 0) nx_copy_text(release->next_sprint_id, sizeof(release->next_sprint_id), value);
    else if (strcmp(key, "next_sprint_name") == 0) nx_copy_text(release->next_sprint_name, sizeof(release->next_sprint_name), value);
    else if (strcmp(key, "capabilities") == 0) nx_copy_text(release->capabilities, sizeof(release->capabilities), value);
    else if (strcmp(key, "decision_id") == 0) nx_copy_text(release->decision_id, sizeof(release->decision_id), value);
    else if (strcmp(key, "decision_title") == 0) nx_copy_text(release->decision_title, sizeof(release->decision_title), value);
    else if (strcmp(key, "decision_body") == 0) nx_copy_text(release->decision_body, sizeof(release->decision_body), value);
    else return 0;
    return 1;
}

int NxDocumentationManager_LoadRelease(const char* release_path,
                                       NxDocumentationRelease* out_release,
                                       char* error_message,
                                       size_t error_message_size)
{
    FILE* file;
    char line[2048];
    NxDocumentationRelease release = {0};
    if (release_path == NULL || out_release == NULL) return 0;
    file = fopen(release_path, "rb");
    if (file == NULL) { nx_copy_text(error_message, error_message_size, "Unable to open release descriptor."); return 0; }
    while (fgets(line, (int)sizeof(line), file) != NULL) {
        char* separator;
        char* key;
        char* value;
        key = nx_trim(line);
        if (key[0] == '\0' || key[0] == '#') continue;
        separator = strchr(key, '=');
        if (separator == NULL) { (void)fclose(file); nx_copy_text(error_message, error_message_size, "Malformed release descriptor line."); return 0; }
        *separator = '\0';
        value = nx_trim(separator + 1);
        key = nx_trim(key);
        if (!nx_set_release_value(&release, key, value)) { (void)fclose(file); nx_copy_text(error_message, error_message_size, "Unknown release descriptor key."); return 0; }
    }
    if (fclose(file) != 0) { nx_copy_text(error_message, error_message_size, "Unable to close release descriptor."); return 0; }
    if (release.sprint_id[0] == '\0' || release.sprint_name[0] == '\0' ||
        release.completion_date[0] == '\0' || release.next_sprint_id[0] == '\0' ||
        release.next_sprint_name[0] == '\0' || release.capabilities[0] == '\0') {
        nx_copy_text(error_message, error_message_size, "Release descriptor misses required fields.");
        return 0;
    }
    *out_release = release;
    nx_copy_text(error_message, error_message_size, "Release descriptor loaded.");
    return 1;
}

static int nx_validate_code_references(const char* repo_root, const char* state_text,
                                       NxDocumentationValidationResult* result)
{
    const char* cursor = state_text;
    const char marker[] = "<!-- NX-CODE:";
    while ((cursor = strstr(cursor, marker)) != NULL) {
        const char* end = strstr(cursor, "-->");
        char relative[NX_DOC_PATH_MAX];
        char absolute[NX_DOC_PATH_MAX];
        size_t length;
        cursor += strlen(marker);
        if (end == NULL) { result->structural_errors++; return 0; }
        length = (size_t)(end - cursor);
        while (length > 0U && isspace((unsigned char)cursor[length - 1U]) != 0) --length;
        while (length > 0U && isspace((unsigned char)*cursor) != 0) { ++cursor; --length; }
        if (length == 0U || length >= sizeof(relative)) { result->structural_errors++; return 0; }
        (void)memcpy(relative, cursor, length);
        relative[length] = '\0';
        result->code_references_checked++;
        if (!nx_join(absolute, sizeof(absolute), repo_root, relative) || !nx_exists(absolute)) result->code_references_missing++;
        cursor = end + 3;
    }
    return 1;
}

int NxDocumentationManager_Validate(const char* repo_root,
                                    NxDocumentationValidationResult* out_result)
{
    NxDocumentationValidationResult result = {0};
    char path[NX_DOC_PATH_MAX];
    char text[NX_DOC_TEXT_MAX];
    size_t index;
    if (repo_root == NULL || out_result == NULL) return 0;
    for (index = 0U; index < sizeof(nx_required_docs) / sizeof(nx_required_docs[0]); ++index) {
        char relative[NX_DOC_PATH_MAX];
        int written = snprintf(relative, sizeof(relative), "Docs/%s", nx_required_docs[index]);
        result.files_checked++;
        if (written < 0 || (size_t)written >= sizeof(relative) ||
            !nx_join(path, sizeof(path), repo_root, relative) || !nx_exists(path)) result.files_missing++;
    }
    if (nx_join(path, sizeof(path), repo_root, "Docs/PROJECT_STATE.md") && nx_read_all(path, text, sizeof(text))) {
        if (strstr(text, "# PROJECT STATE") == NULL || strstr(text, "Sprint actual:") == NULL || strstr(text, "Estado:") == NULL) result.structural_errors++;
        (void)nx_validate_code_references(repo_root, text, &result);
    } else result.structural_errors++;
    if (!nx_join(path, sizeof(path), repo_root, "Docs/CHANGELOG.md") || !nx_read_all(path, text, sizeof(text)) || strstr(text, "# CHANGELOG") == NULL) result.structural_errors++;
    if (!nx_join(path, sizeof(path), repo_root, "Docs/DECISIONS.md") || !nx_read_all(path, text, sizeof(text)) || strstr(text, "# DECISIONS") == NULL) result.structural_errors++;
    result.success = result.files_missing == 0 && result.structural_errors == 0 && result.code_references_missing == 0;
    (void)snprintf(result.message, sizeof(result.message),
                   "checked=%d missing=%d structural_errors=%d code_refs=%d code_missing=%d",
                   result.files_checked, result.files_missing, result.structural_errors,
                   result.code_references_checked, result.code_references_missing);
    *out_result = result;
    return result.success;
}

static int nx_atomic_commit_three(const char* paths[3], const char* texts[3])
{
    char temporary[3][NX_DOC_PATH_MAX];
    char backup[3][NX_DOC_PATH_MAX];
    int index;
    int committed = 0;
    for (index = 0; index < 3; ++index) {
        if (snprintf(temporary[index], sizeof(temporary[index]), "%s.nxtmp", paths[index]) < 0 ||
            snprintf(backup[index], sizeof(backup[index]), "%s.nxbak", paths[index]) < 0 ||
            !nx_write_all(temporary[index], texts[index])) goto cleanup;
    }
    for (index = 0; index < 3; ++index) {
        (void)remove(backup[index]);
        if (rename(paths[index], backup[index]) != 0) goto rollback;
        if (rename(temporary[index], paths[index]) != 0) goto rollback;
        committed++;
    }
    for (index = 0; index < 3; ++index) (void)remove(backup[index]);
    return 1;
rollback:
    for (index = committed; index >= 0; --index) {
        (void)remove(paths[index]);
        (void)rename(backup[index], paths[index]);
    }
cleanup:
    for (index = 0; index < 3; ++index) { (void)remove(temporary[index]); }
    return 0;
}

static int nx_build_state(const char* original, const NxDocumentationRelease* release,
                          char* output, size_t capacity)
{
    int written = snprintf(output, capacity,
        "# PROJECT STATE\n# Nexiora\n\nÚltima actualización:\n%s\n\n---\n\n"
        "# Estado General\n\nEstado del proyecto:\n\n🟢 Activo\n\nRama:\n\nmain\n\n"
        "Lenguaje:\n\nC23\n\nCompilador:\n\nMSYS2 UCRT64 GCC\n\nBuild:\n\nCMake\n\nGenerador:\n\nNinja\n\n---\n\n"
        "# Estado del Desarrollo\n\nEPIC actual:\n\nEPIC-01\n\nSprint actual:\n\n%s\n\nNombre:\n\n%s\n\n"
        "Sprint anterior:\n\n%s — %s\n\nEstado:\n\nEn desarrollo\n\n---\n\n"
        "# Última capacidad incorporada\n\n%s — %s\n\n%s\n\n---\n\n"
        "# Calidad obligatoria\n\n0 errores\n\n0 warnings\n\nPruebas completas\n\nRollback funcional\n\nDocumentación sincronizada\n\n---\n\n"
        "# Referencias de código verificables\n\n<!-- NX-CODE: Include/Nexiora/NCOS/NxDocumentationManager.h -->\n"
        "<!-- NX-CODE: Source/NCOS/NxDocumentationManager.c -->\n"
        "<!-- NX-CODE: Tools/NCOS/NxDocumentationTool.c -->\n"
        "<!-- NX-CODE: Tests/Unit/NxDocumentationManagerTests.c -->\n\n"
        "# Regla Principal\n\nNunca disminuir la calidad del proyecto. Cada Sprint debe dejar el proyecto mejor de lo que estaba antes.\n",
        release->completion_date, release->next_sprint_id, release->next_sprint_name,
        release->sprint_id, release->sprint_name, release->sprint_id, release->sprint_name,
        release->capabilities);
    (void)original;
    return written >= 0 && (size_t)written < capacity;
}

int NxDocumentationManager_FinalizeSprint(const char* repo_root,
                                          const NxDocumentationRelease* release,
                                          char* error_message,
                                          size_t error_message_size)
{
    char paths_storage[3][NX_DOC_PATH_MAX];
    const char* paths[3] = { paths_storage[0], paths_storage[1], paths_storage[2] };
    char originals[3][NX_DOC_TEXT_MAX];
    char outputs[3][NX_DOC_TEXT_MAX];
    const char* texts[3] = { outputs[0], outputs[1], outputs[2] };
    NxDocumentationValidationResult validation;
    int written;
    if (repo_root == NULL || release == NULL) return 0;
    if (!NxDocumentationManager_Validate(repo_root, &validation)) { nx_copy_text(error_message, error_message_size, validation.message); return 0; }
    if (!nx_join(paths_storage[0], sizeof(paths_storage[0]), repo_root, "Docs/PROJECT_STATE.md") ||
        !nx_join(paths_storage[1], sizeof(paths_storage[1]), repo_root, "Docs/CHANGELOG.md") ||
        !nx_join(paths_storage[2], sizeof(paths_storage[2]), repo_root, "Docs/DECISIONS.md")) return 0;
    if (!nx_read_all(paths[0], originals[0], sizeof(originals[0])) ||
        !nx_read_all(paths[1], originals[1], sizeof(originals[1])) ||
        !nx_read_all(paths[2], originals[2], sizeof(originals[2]))) { nx_copy_text(error_message, error_message_size, "Unable to read governance documents."); return 0; }
    if (!nx_build_state(originals[0], release, outputs[0], sizeof(outputs[0]))) return 0;
    written = snprintf(outputs[1], sizeof(outputs[1]), "%s\n## [%s] — %s\n\n### Added\n\n- %s\n",
                       originals[1], release->sprint_id, release->completion_date, release->capabilities);
    if (written < 0 || (size_t)written >= sizeof(outputs[1])) return 0;
    if (release->decision_id[0] != '\0') {
        written = snprintf(outputs[2], sizeof(outputs[2]),
            "%s\n## %s — %s\n\n**Estado:** Aceptada  \n**Fecha:** %s\n\n%s\n",
            originals[2], release->decision_id, release->decision_title,
            release->completion_date, release->decision_body);
    } else written = snprintf(outputs[2], sizeof(outputs[2]), "%s", originals[2]);
    if (written < 0 || (size_t)written >= sizeof(outputs[2])) return 0;
    if (!nx_atomic_commit_three(paths, texts)) { nx_copy_text(error_message, error_message_size, "Atomic documentation transaction failed and was rolled back."); return 0; }
    if (!NxDocumentationManager_Validate(repo_root, &validation)) { nx_copy_text(error_message, error_message_size, "Post-update validation failed."); return 0; }
    nx_copy_text(error_message, error_message_size, "Sprint documentation finalized atomically.");
    return 1;
}
