#include "Nexiora/Memory/NxMemoryConsolidation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(void) {
    puts("Uso:");
    puts("  nexiora_memory_consolidation analyze <memoria> <concepto> [umbral]");
    puts("  nexiora_memory_consolidation consolidate <memoria> <concepto> <timestamp> [umbral]");
}

static void print_report(const NxMcReport* report) {
    printf("status=%s\n", NxMemoryConsolidation_StatusName(report->status));
    printf("concept=%s\n", report->concept);
    printf("active_entry_id=%u\n", report->active_entry_id);
    printf("active_version=%u\n", report->active_version);
    printf("old_confidence=%u\n", report->old_confidence);
    printf("proposed_confidence=%u\n", report->proposed_confidence);
    printf("supporting_entries=%u\n", report->supporting_entries);
    printf("independent_sources=%u\n", report->independent_sources);
    printf("opposing_entries=%u\n", report->opposing_entries);
    printf("duplicate_entries=%u\n", report->duplicate_entries);
    printf("similarity_threshold=%u\n", report->similarity_threshold);
    printf("belief=%s\n", report->active_belief);
    printf("summary=%s\n", report->summary);
}

static unsigned int parse_threshold(const char* text) {
    unsigned long value;
    if (text == NULL) return 65U;
    value = strtoul(text, NULL, 10);
    if (value < 1UL || value > 100UL) return 0U;
    return (unsigned int)value;
}

int main(int argc, char** argv) {
    NxMcReport report;
    NxMcStatus status;
    unsigned int threshold;
    if (argc < 4) {
        print_usage();
        return 2;
    }
    if (strcmp(argv[1], "analyze") == 0) {
        threshold = parse_threshold(argc >= 5 ? argv[4] : NULL);
        if (threshold == 0U) return 2;
        status = NxMemoryConsolidation_Analyze(argv[2], argv[3], threshold, &report);
        print_report(&report);
        return status == NX_MC_OK || status == NX_MC_UNCHANGED || status == NX_MC_CONFLICT_DETECTED ? 0 : 1;
    }
    if (strcmp(argv[1], "consolidate") == 0 && argc >= 5) {
        threshold = parse_threshold(argc >= 6 ? argv[5] : NULL);
        if (threshold == 0U) return 2;
        status = NxMemoryConsolidation_Consolidate(argv[2], argv[3], argv[4], threshold, &report);
        print_report(&report);
        return status == NX_MC_OK || status == NX_MC_UNCHANGED || status == NX_MC_CONFLICT_DETECTED ? 0 : 1;
    }
    print_usage();
    return 2;
}
