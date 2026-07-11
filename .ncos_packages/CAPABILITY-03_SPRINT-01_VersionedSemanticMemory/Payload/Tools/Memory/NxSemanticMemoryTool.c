#include "Nexiora/Memory/NxVersionedSemanticMemory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void nx_print_usage(void) {
    (void)printf("Uso:\n");
    (void)printf("  nexiora_semantic_memory init <archivo>\n");
    (void)printf("  nexiora_semantic_memory remember <archivo> <concepto> <creencia> <fuente> <timestamp> <confianza>\n");
    (void)printf("  nexiora_semantic_memory recall <archivo> <concepto>\n");
    (void)printf("  nexiora_semantic_memory history <archivo> <concepto>\n");
}

int main(int argc, char** argv) {
    NxVsmStatus status;
    if (argc < 3) {
        nx_print_usage();
        return 2;
    }
    if (strcmp(argv[1], "init") == 0 && argc == 3) {
        status = NxVersionedSemanticMemory_Init(argv[2]);
        (void)printf("status=%s\npath=%s\n", NxVersionedSemanticMemory_StatusName(status), argv[2]);
        return status == NX_VSM_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "remember") == 0 && argc == 8) {
        NxVsmRememberResult result;
        unsigned long confidence = strtoul(argv[7], NULL, 10);
        status = NxVersionedSemanticMemory_Remember(argv[2], argv[3], argv[4], argv[5], argv[6], (unsigned int)confidence, &result);
        (void)printf("status=%s\nconcept=%s\nversion=%u\nentry_id=%u\nprevious_entry_id=%u\nconfidence=%u\nbelief_changed=%s\nprevious_belief=%s\ncurrent_belief=%s\nexplanation=%s\n",
            NxVersionedSemanticMemory_StatusName(status), result.concept, result.version, result.entry_id,
            result.previous_entry_id, result.confidence, result.belief_changed ? "true" : "false",
            result.previous_belief, result.current_belief, result.explanation);
        return (status == NX_VSM_OK || status == NX_VSM_UNCHANGED) ? 0 : 1;
    }
    if (strcmp(argv[1], "recall") == 0 && argc == 4) {
        NxVsmEntry entry;
        status = NxVersionedSemanticMemory_Recall(argv[2], argv[3], &entry);
        (void)printf("status=%s\n", NxVersionedSemanticMemory_StatusName(status));
        if (status == NX_VSM_OK) {
            (void)printf("concept=%s\nversion=%u\nconfidence=%u\nbelief=%s\nsource=%s\ntimestamp=%s\nreason=%s\n",
                entry.concept, entry.version, entry.confidence, entry.belief, entry.source, entry.timestamp, entry.change_reason);
        }
        return status == NX_VSM_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "history") == 0 && argc == 4) {
        NxVsmEntry entries[NX_VSM_MAX_ENTRIES];
        size_t count = NxVersionedSemanticMemory_History(argv[2], argv[3], entries, NX_VSM_MAX_ENTRIES);
        size_t i;
        (void)printf("status=%s\ncount=%zu\n", count > 0U ? "OK" : "NOT_FOUND", count);
        for (i = 0U; i < count; ++i) {
            (void)printf("entry_%zu_version=%u\nentry_%zu_active=%s\nentry_%zu_confidence=%u\nentry_%zu_belief=%s\nentry_%zu_source=%s\n",
                i + 1U, entries[i].version, i + 1U, entries[i].active ? "true" : "false",
                i + 1U, entries[i].confidence, i + 1U, entries[i].belief, i + 1U, entries[i].source);
        }
        return count > 0U ? 0 : 1;
    }
    nx_print_usage();
    return 2;
}
