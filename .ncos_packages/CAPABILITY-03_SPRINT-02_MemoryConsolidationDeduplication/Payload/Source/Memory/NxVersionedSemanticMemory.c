#include "Nexiora/Memory/NxVersionedSemanticMemory.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define nx_vsm_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define nx_vsm_mkdir(path) mkdir(path, 0777)
#endif

static void nx_vsm_copy(char* dst, size_t cap, const char* src) {
    size_t n;
    if (dst == NULL || cap == 0U) return;
    dst[0] = '\0';
    if (src == NULL) return;
    n = strlen(src);
    if (n >= cap) n = cap - 1U;
    if (n > 0U) memcpy(dst, src, n);
    dst[n] = '\0';
}

static int nx_vsm_text_equal_ci(const char* a, const char* b) {
    size_t i = 0U;
    if (a == NULL || b == NULL) return 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return 0;
        ++i;
    }
    return a[i] == '\0' && b[i] == '\0';
}

static int nx_vsm_make_parent_dirs(const char* path) {
    char temp[NX_VSM_MAX_PATH];
    size_t i;
    if (path == NULL || path[0] == '\0') return 0;
    nx_vsm_copy(temp, sizeof(temp), path);
    for (i = 0U; temp[i] != '\0'; ++i) {
        if ((temp[i] == '/' || temp[i] == '\\') && i > 0U) {
            char saved = temp[i];
            temp[i] = '\0';
            if (!(i == 2U && temp[1] == ':')) (void)nx_vsm_mkdir(temp);
            temp[i] = saved;
        }
    }
    return 1;
}

static void nx_vsm_sanitize_line(char* text) {
    size_t i;
    if (text == NULL) return;
    for (i = 0U; text[i] != '\0'; ++i) {
        if (text[i] == '\r' || text[i] == '\n') text[i] = ' ';
    }
}

static NxVsmStatus nx_vsm_save(const char* path, const NxVsmStore* store) {
    FILE* file;
    size_t i;
    if (path == NULL || store == NULL) return NX_VSM_INVALID_ARGUMENT;
    if (!nx_vsm_make_parent_dirs(path)) return NX_VSM_IO_ERROR;
    file = fopen(path, "wb");
    if (file == NULL) return NX_VSM_IO_ERROR;
    (void)fprintf(file, "nxsemantic/1\nentry_count=%zu\nnext_id=%u\n", store->entry_count, store->next_id);
    for (i = 0U; i < store->entry_count; ++i) {
        NxVsmEntry copy = store->entries[i];
        nx_vsm_sanitize_line(copy.concept);
        nx_vsm_sanitize_line(copy.belief);
        nx_vsm_sanitize_line(copy.source);
        nx_vsm_sanitize_line(copy.timestamp);
        nx_vsm_sanitize_line(copy.change_reason);
        (void)fprintf(file,
            "entry_begin\nid=%u\nversion=%u\nsupersedes=%u\nconfidence=%u\nactive=%d\nconcept=%s\nbelief=%s\nsource=%s\ntimestamp=%s\nreason=%s\nentry_end\n",
            copy.id, copy.version, copy.supersedes_id, copy.confidence, copy.active,
            copy.concept, copy.belief, copy.source, copy.timestamp, copy.change_reason);
    }
    if (fclose(file) != 0) return NX_VSM_IO_ERROR;
    return NX_VSM_OK;
}

NxVsmStatus NxVersionedSemanticMemory_Init(const char* path) {
    NxVsmStore store;
    if (path == NULL || path[0] == '\0') return NX_VSM_INVALID_ARGUMENT;
    memset(&store, 0, sizeof(store));
    store.next_id = 1U;
    return nx_vsm_save(path, &store);
}

NxVsmStatus NxVersionedSemanticMemory_Load(const char* path, NxVsmStore* out_store) {
    FILE* file;
    char line[2048];
    NxVsmStore store;
    NxVsmEntry current;
    int in_entry = 0;
    if (path == NULL || out_store == NULL) return NX_VSM_INVALID_ARGUMENT;
    file = fopen(path, "rb");
    if (file == NULL) return NX_VSM_NOT_FOUND;
    memset(&store, 0, sizeof(store));
    store.next_id = 1U;
    memset(&current, 0, sizeof(current));
    if (fgets(line, sizeof(line), file) == NULL || strncmp(line, "nxsemantic/1", 12U) != 0) {
        (void)fclose(file);
        return NX_VSM_INVALID_FORMAT;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        while (len > 0U && (line[len - 1U] == '\n' || line[len - 1U] == '\r')) line[--len] = '\0';
        if (strcmp(line, "entry_begin") == 0) {
            memset(&current, 0, sizeof(current));
            in_entry = 1;
        } else if (strcmp(line, "entry_end") == 0 && in_entry != 0) {
            if (store.entry_count >= NX_VSM_MAX_ENTRIES) {
                (void)fclose(file);
                return NX_VSM_CAPACITY_EXCEEDED;
            }
            store.entries[store.entry_count++] = current;
            if (store.next_id <= current.id) store.next_id = current.id + 1U;
            in_entry = 0;
        } else if (in_entry != 0) {
            if (strncmp(line, "id=", 3U) == 0) current.id = (unsigned int)strtoul(line + 3U, NULL, 10);
            else if (strncmp(line, "version=", 8U) == 0) current.version = (unsigned int)strtoul(line + 8U, NULL, 10);
            else if (strncmp(line, "supersedes=", 11U) == 0) current.supersedes_id = (unsigned int)strtoul(line + 11U, NULL, 10);
            else if (strncmp(line, "confidence=", 11U) == 0) current.confidence = (unsigned int)strtoul(line + 11U, NULL, 10);
            else if (strncmp(line, "active=", 7U) == 0) current.active = atoi(line + 7U);
            else if (strncmp(line, "concept=", 8U) == 0) nx_vsm_copy(current.concept, sizeof(current.concept), line + 8U);
            else if (strncmp(line, "belief=", 7U) == 0) nx_vsm_copy(current.belief, sizeof(current.belief), line + 7U);
            else if (strncmp(line, "source=", 7U) == 0) nx_vsm_copy(current.source, sizeof(current.source), line + 7U);
            else if (strncmp(line, "timestamp=", 10U) == 0) nx_vsm_copy(current.timestamp, sizeof(current.timestamp), line + 10U);
            else if (strncmp(line, "reason=", 7U) == 0) nx_vsm_copy(current.change_reason, sizeof(current.change_reason), line + 7U);
        }
    }
    (void)fclose(file);
    *out_store = store;
    return NX_VSM_OK;
}

static NxVsmEntry* nx_vsm_find_active(NxVsmStore* store, const char* concept) {
    size_t i;
    if (store == NULL || concept == NULL) return NULL;
    for (i = store->entry_count; i > 0U; --i) {
        NxVsmEntry* entry = &store->entries[i - 1U];
        if (entry->active != 0 && nx_vsm_text_equal_ci(entry->concept, concept)) return entry;
    }
    return NULL;
}

NxVsmStatus NxVersionedSemanticMemory_Remember(
    const char* path, const char* concept, const char* belief, const char* source,
    const char* timestamp, unsigned int confidence, NxVsmRememberResult* out_result) {
    NxVsmStore store;
    NxVsmStatus status;
    NxVsmEntry* previous;
    NxVsmEntry* next;
    NxVsmRememberResult result;
    if (path == NULL || concept == NULL || belief == NULL || source == NULL || timestamp == NULL || out_result == NULL ||
        concept[0] == '\0' || belief[0] == '\0' || source[0] == '\0' || timestamp[0] == '\0' || confidence > 100U) {
        return NX_VSM_INVALID_ARGUMENT;
    }
    memset(&result, 0, sizeof(result));
    status = NxVersionedSemanticMemory_Load(path, &store);
    if (status == NX_VSM_NOT_FOUND) {
        status = NxVersionedSemanticMemory_Init(path);
        if (status != NX_VSM_OK) return status;
        status = NxVersionedSemanticMemory_Load(path, &store);
    }
    if (status != NX_VSM_OK) return status;
    previous = nx_vsm_find_active(&store, concept);
    if (previous != NULL && strcmp(previous->belief, belief) == 0 && confidence <= previous->confidence) {
        result.status = NX_VSM_UNCHANGED;
        result.entry_id = previous->id;
        result.version = previous->version;
        result.confidence = previous->confidence;
        nx_vsm_copy(result.concept, sizeof(result.concept), previous->concept);
        nx_vsm_copy(result.current_belief, sizeof(result.current_belief), previous->belief);
        nx_vsm_copy(result.explanation, sizeof(result.explanation), "La creencia existente ya tiene igual o mayor confianza; no se creó una versión redundante.");
        *out_result = result;
        return NX_VSM_UNCHANGED;
    }
    if (store.entry_count >= NX_VSM_MAX_ENTRIES) return NX_VSM_CAPACITY_EXCEEDED;
    next = &store.entries[store.entry_count];
    memset(next, 0, sizeof(*next));
    next->id = store.next_id++;
    next->version = previous == NULL ? 1U : previous->version + 1U;
    next->supersedes_id = previous == NULL ? 0U : previous->id;
    next->confidence = confidence;
    next->active = 1;
    nx_vsm_copy(next->concept, sizeof(next->concept), concept);
    nx_vsm_copy(next->belief, sizeof(next->belief), belief);
    nx_vsm_copy(next->source, sizeof(next->source), source);
    nx_vsm_copy(next->timestamp, sizeof(next->timestamp), timestamp);
    if (previous == NULL) {
        nx_vsm_copy(next->change_reason, sizeof(next->change_reason), "Creencia inicial incorporada desde evidencia con procedencia.");
    } else {
        previous->active = 0;
        nx_vsm_copy(next->change_reason, sizeof(next->change_reason), "La nueva evidencia modificó la creencia activa; la versión anterior fue preservada como historial.");
        result.previous_entry_id = previous->id;
        result.belief_changed = strcmp(previous->belief, belief) != 0;
        nx_vsm_copy(result.previous_belief, sizeof(result.previous_belief), previous->belief);
    }
    store.entry_count++;
    status = nx_vsm_save(path, &store);
    if (status != NX_VSM_OK) return status;
    result.status = NX_VSM_OK;
    result.entry_id = next->id;
    result.version = next->version;
    result.confidence = next->confidence;
    nx_vsm_copy(result.concept, sizeof(result.concept), next->concept);
    nx_vsm_copy(result.current_belief, sizeof(result.current_belief), next->belief);
    nx_vsm_copy(result.explanation, sizeof(result.explanation), next->change_reason);
    *out_result = result;
    return NX_VSM_OK;
}

NxVsmStatus NxVersionedSemanticMemory_Recall(const char* path, const char* concept, NxVsmEntry* out_entry) {
    NxVsmStore store;
    NxVsmStatus status;
    NxVsmEntry* entry;
    if (path == NULL || concept == NULL || out_entry == NULL) return NX_VSM_INVALID_ARGUMENT;
    status = NxVersionedSemanticMemory_Load(path, &store);
    if (status != NX_VSM_OK) return status;
    entry = nx_vsm_find_active(&store, concept);
    if (entry == NULL) return NX_VSM_NOT_FOUND;
    *out_entry = *entry;
    return NX_VSM_OK;
}

size_t NxVersionedSemanticMemory_History(const char* path, const char* concept, NxVsmEntry* out_entries, size_t capacity) {
    NxVsmStore store;
    size_t i;
    size_t count = 0U;
    if (path == NULL || concept == NULL || out_entries == NULL || capacity == 0U) return 0U;
    if (NxVersionedSemanticMemory_Load(path, &store) != NX_VSM_OK) return 0U;
    for (i = 0U; i < store.entry_count && count < capacity; ++i) {
        if (nx_vsm_text_equal_ci(store.entries[i].concept, concept)) out_entries[count++] = store.entries[i];
    }
    return count;
}

const char* NxVersionedSemanticMemory_StatusName(NxVsmStatus status) {
    switch (status) {
        case NX_VSM_OK: return "OK";
        case NX_VSM_UNCHANGED: return "UNCHANGED";
        case NX_VSM_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_VSM_IO_ERROR: return "IO_ERROR";
        case NX_VSM_NOT_FOUND: return "NOT_FOUND";
        case NX_VSM_CAPACITY_EXCEEDED: return "CAPACITY_EXCEEDED";
        case NX_VSM_INVALID_FORMAT: return "INVALID_FORMAT";
        default: return "UNKNOWN";
    }
}
