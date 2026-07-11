#include "Nexiora/Reasoning/NxUnifiedCognitiveDialogue.h"

#include <stdio.h>
#include <string.h>

static void print_result(const NxUcdResult* result) {
    printf("status=%s\n", NxUnifiedCognitiveDialogue_StatusName(result->status));
    printf("mode=%s\n", result->mode == NX_UCD_MODE_CLAIM ? "CLAIM" : "ANSWER");
    printf("turn=%u\n", result->turn_index);
    printf("confidence=%u\n", result->confidence);
    printf("evidence_count=%u\n", result->evidence_count);
    printf("contradictions=%u\n", result->contradiction_count);
    printf("active_subject=%s\n", result->active_subject);
    printf("resolved_input=%s\n", result->resolved_input);
    printf("answer=%s\n", result->answer);
    printf("explanation=%s\n", result->explanation);
    printf("limitations=%s\n", result->limitations);
    printf("gap_plan=%s\n", result->gap_plan_path);
}

static void usage(void) {
    puts("Uso:");
    puts("  nexiora_cognitive_dialogue create <session> <evidence> <subject>");
    puts("  nexiora_cognitive_dialogue ask <session> <question>");
    puts("  nexiora_cognitive_dialogue claim <session> <claim>");
}

int main(int argc, char** argv) {
    NxUcdStatus status;
    NxUcdResult result;
    if (argc < 2) {
        usage();
        return 2;
    }
    if (strcmp(argv[1], "create") == 0) {
        if (argc != 5) {
            usage();
            return 2;
        }
        status = NxUnifiedCognitiveDialogue_Create(argv[2], argv[3], argv[4]);
        printf("status=%s\n", NxUnifiedCognitiveDialogue_StatusName(status));
        printf("session=%s\n", argv[2]);
        return status == NX_UCD_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "ask") == 0) {
        if (argc != 4) {
            usage();
            return 2;
        }
        status = NxUnifiedCognitiveDialogue_Ask(argv[2], argv[3], &result);
        print_result(&result);
        return (status == NX_UCD_OK || status == NX_UCD_GAP_OPENED) ? 0 : 1;
    }
    if (strcmp(argv[1], "claim") == 0) {
        if (argc != 4) {
            usage();
            return 2;
        }
        status = NxUnifiedCognitiveDialogue_EvaluateClaim(argv[2], argv[3], &result);
        print_result(&result);
        return (status == NX_UCD_OK || status == NX_UCD_GAP_OPENED || status == NX_UCD_CONTRADICTORY) ? 0 : 1;
    }
    usage();
    return 2;
}
