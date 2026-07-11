#include "Nexiora/Memory/NxMemoryConsolidation.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define NX_MC_MAX_TOKENS 128U
#define NX_MC_MAX_TOKEN 48U

typedef struct NxMcTokenSet {
    char tokens[NX_MC_MAX_TOKENS][NX_MC_MAX_TOKEN];
    size_t count;
    int has_negation;
} NxMcTokenSet;

static void nx_mc_copy(char* dst, size_t capacity, const char* src) {
    size_t length;
    if (dst == NULL || capacity == 0U) return;
    dst[0] = '\0';
    if (src == NULL) return;
    length = strlen(src);
    if (length >= capacity) length = capacity - 1U;
    if (length > 0U) memcpy(dst, src, length);
    dst[length] = '\0';
}

static int nx_mc_equal_ci(const char* left, const char* right) {
    size_t index = 0U;
    if (left == NULL || right == NULL) return 0;
    while (left[index] != '\0' && right[index] != '\0') {
        if (tolower((unsigned char)left[index]) != tolower((unsigned char)right[index])) return 0;
        ++index;
    }
    return left[index] == '\0' && right[index] == '\0';
}

static int nx_mc_is_negation(const char* token) {
    return nx_mc_equal_ci(token, "no") || nx_mc_equal_ci(token, "nunca") ||
           nx_mc_equal_ci(token, "jamas") || nx_mc_equal_ci(token, "sin") ||
           nx_mc_equal_ci(token, "not") || nx_mc_equal_ci(token, "never") ||
           nx_mc_equal_ci(token, "without");
}

static int nx_mc_has_token(const NxMcTokenSet* set, const char* token) {
    size_t index;
    if (set == NULL || token == NULL) return 0;
    for (index = 0U; index < set->count; ++index) {
        if (nx_mc_equal_ci(set->tokens[index], token)) return 1;
    }
    return 0;
}

static void nx_mc_tokenize(const char* text, NxMcTokenSet* out_set) {
    size_t index = 0U;
    char token[NX_MC_MAX_TOKEN];
    size_t token_length = 0U;
    if (out_set == NULL) return;
    memset(out_set, 0, sizeof(*out_set));
    if (text == NULL) return;
    while (1) {
        unsigned char ch = (unsigned char)text[index];
        if (isalnum(ch) != 0 || ch >= 128U) {
            if (token_length + 1U < sizeof(token)) token[token_length++] = (char)tolower(ch);
        } else if (token_length > 0U) {
            token[token_length] = '\0';
            if (nx_mc_is_negation(token)) out_set->has_negation = 1;
            if (!nx_mc_has_token(out_set, token) && out_set->count < NX_MC_MAX_TOKENS) {
                nx_mc_copy(out_set->tokens[out_set->count], NX_MC_MAX_TOKEN, token);
                out_set->count++;
            }
            token_length = 0U;
        }
        if (ch == '\0') break;
        ++index;
    }
}

unsigned int NxMemoryConsolidation_Similarity(const char* left, const char* right) {
    NxMcTokenSet left_set;
    NxMcTokenSet right_set;
    size_t index;
    size_t intersection = 0U;
    size_t union_count;
    nx_mc_tokenize(left, &left_set);
    nx_mc_tokenize(right, &right_set);
    if (left_set.count == 0U && right_set.count == 0U) return 100U;
    for (index = 0U; index < left_set.count; ++index) {
        if (nx_mc_has_token(&right_set, left_set.tokens[index])) intersection++;
    }
    union_count = left_set.count + right_set.count - intersection;
    if (union_count == 0U) return 100U;
    return (unsigned int)((intersection * 100U) / union_count);
}

static int nx_mc_is_opposition(const char* left, const char* right, unsigned int similarity) {
    NxMcTokenSet left_set;
    NxMcTokenSet right_set;
    nx_mc_tokenize(left, &left_set);
    nx_mc_tokenize(right, &right_set);
    return similarity >= 35U && left_set.has_negation != right_set.has_negation;
}

static int nx_mc_source_seen(char sources[NX_MC_MAX_SOURCES][NX_VSM_MAX_SOURCE], size_t count, const char* source) {
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (nx_mc_equal_ci(sources[index], source)) return 1;
    }
    return 0;
}

NxMcStatus NxMemoryConsolidation_Analyze(
    const char* memory_path,
    const char* concept,
    unsigned int similarity_threshold,
    NxMcReport* out_report) {
    NxVsmStore store;
    NxVsmEntry active;
    NxVsmStatus load_status;
    NxMcReport report;
    char sources[NX_MC_MAX_SOURCES][NX_VSM_MAX_SOURCE];
    size_t source_count = 0U;
    size_t index;
    unsigned int support_confidence_sum = 0U;
    unsigned int support_confidence_count = 0U;
    unsigned int support_confidence_max = 0U;
    unsigned int proposal;
    if (memory_path == NULL || concept == NULL || out_report == NULL ||
        memory_path[0] == '\0' || concept[0] == '\0' ||
        similarity_threshold < 1U || similarity_threshold > 100U) {
        return NX_MC_INVALID_ARGUMENT;
    }
    memset(&report, 0, sizeof(report));
    memset(sources, 0, sizeof(sources));
    load_status = NxVersionedSemanticMemory_Load(memory_path, &store);
    if (load_status == NX_VSM_NOT_FOUND) return NX_MC_NOT_FOUND;
    if (load_status != NX_VSM_OK) return NX_MC_IO_ERROR;
    if (NxVersionedSemanticMemory_Recall(memory_path, concept, &active) != NX_VSM_OK) return NX_MC_NOT_FOUND;
    report.active_entry_id = active.id;
    report.active_version = active.version;
    report.old_confidence = active.confidence;
    report.similarity_threshold = similarity_threshold;
    nx_mc_copy(report.concept, sizeof(report.concept), active.concept);
    nx_mc_copy(report.active_belief, sizeof(report.active_belief), active.belief);
    for (index = 0U; index < store.entry_count; ++index) {
        const NxVsmEntry* entry = &store.entries[index];
        unsigned int similarity;
        if (!nx_mc_equal_ci(entry->concept, concept)) continue;
        similarity = NxMemoryConsolidation_Similarity(active.belief, entry->belief);
        if (nx_mc_is_opposition(active.belief, entry->belief, similarity)) {
            report.opposing_entries++;
            continue;
        }
        if (similarity >= similarity_threshold) {
            report.supporting_entries++;
            support_confidence_sum += entry->confidence;
            support_confidence_count++;
            if (entry->confidence > support_confidence_max) support_confidence_max = entry->confidence;
            if (strcmp(active.belief, entry->belief) == 0 && entry->id != active.id) report.duplicate_entries++;
            if (!nx_mc_source_seen(sources, source_count, entry->source) && source_count < NX_MC_MAX_SOURCES) {
                nx_mc_copy(sources[source_count], NX_VSM_MAX_SOURCE, entry->source);
                source_count++;
            }
        }
    }
    report.independent_sources = (unsigned int)source_count;
    proposal = active.confidence;
    if (support_confidence_count > 0U) {
        unsigned int average = support_confidence_sum / support_confidence_count;
        unsigned int source_bonus = report.independent_sources > 1U ? (report.independent_sources - 1U) * 5U : 0U;
        (void)average;
        proposal = support_confidence_max + source_bonus;
        if (proposal < active.confidence) proposal = active.confidence;
        if (proposal > 100U) proposal = 100U;
    }
    if (report.opposing_entries > 0U) {
        unsigned int penalty = report.opposing_entries * 10U;
        proposal = proposal > penalty ? proposal - penalty : 0U;
        report.status = NX_MC_CONFLICT_DETECTED;
        nx_mc_copy(report.summary, sizeof(report.summary),
            "Se detectaron versiones opuestas. La memoria conserva el conflicto y no consolida automáticamente una creencia definitiva.");
    } else if (proposal > active.confidence && report.independent_sources >= 2U) {
        report.status = NX_MC_OK;
        nx_mc_copy(report.summary, sizeof(report.summary),
            "Fuentes independientes respaldan una creencia equivalente; la confianza puede reforzarse sin eliminar el historial.");
    } else {
        report.status = NX_MC_UNCHANGED;
        nx_mc_copy(report.summary, sizeof(report.summary),
            "No existe soporte independiente suficiente para crear una nueva versión consolidada.");
    }
    report.proposed_confidence = proposal;
    *out_report = report;
    return report.status;
}

NxMcStatus NxMemoryConsolidation_Consolidate(
    const char* memory_path,
    const char* concept,
    const char* timestamp,
    unsigned int similarity_threshold,
    NxMcReport* out_report) {
    NxMcReport report;
    NxMcStatus status;
    NxVsmRememberResult remember_result;
    char source[NX_VSM_MAX_SOURCE];
    int written;
    if (timestamp == NULL || timestamp[0] == '\0' || out_report == NULL) return NX_MC_INVALID_ARGUMENT;
    status = NxMemoryConsolidation_Analyze(memory_path, concept, similarity_threshold, &report);
    if (status != NX_MC_OK) {
        *out_report = report;
        return status;
    }
    written = snprintf(source, sizeof(source), "consolidation:%u-sources", report.independent_sources);
    if (written < 0 || (size_t)written >= sizeof(source)) return NX_MC_IO_ERROR;
    if (NxVersionedSemanticMemory_Remember(memory_path, concept, report.active_belief, source,
            timestamp, report.proposed_confidence, &remember_result) != NX_VSM_OK) {
        return NX_MC_IO_ERROR;
    }
    report.active_entry_id = remember_result.entry_id;
    report.active_version = remember_result.version;
    report.old_confidence = remember_result.previous_entry_id == 0U ? report.old_confidence : report.old_confidence;
    nx_mc_copy(report.summary, sizeof(report.summary),
        "La memoria fue consolidada en una nueva versión trazable y la versión anterior permanece disponible en el historial.");
    report.status = NX_MC_OK;
    *out_report = report;
    return NX_MC_OK;
}

const char* NxMemoryConsolidation_StatusName(NxMcStatus status) {
    switch (status) {
        case NX_MC_OK: return "OK";
        case NX_MC_UNCHANGED: return "UNCHANGED";
        case NX_MC_CONFLICT_DETECTED: return "CONFLICT_DETECTED";
        case NX_MC_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_MC_NOT_FOUND: return "NOT_FOUND";
        case NX_MC_IO_ERROR: return "IO_ERROR";
        default: return "UNKNOWN";
    }
}
