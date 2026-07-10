#include "Nexiora/NCOS/NxDocumentationGovernance.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char** argv)
{
    NxDocumentationValidationResult result;
    if (argc < 3) { fprintf(stderr, "Usage: nexiora_docs <validate|complete-sprint|record-decision> <repo> [args]\n"); return 2; }
    if (strcmp(argv[1], "validate") == 0) {
        int ok = NxDocumentation_Validate(argv[2], &result);
        printf("%s\n", result.message);
        return ok ? 0 : 1;
    }
    if (strcmp(argv[1], "complete-sprint") == 0 && argc == 6) return NxDocumentation_CompleteSprint(argv[2], argv[3], argv[4], argv[5]) ? 0 : 1;
    if (strcmp(argv[1], "record-decision") == 0 && argc == 7) return NxDocumentation_RecordDecision(argv[2], argv[3], argv[4], argv[5], argv[6]) ? 0 : 1;
    fprintf(stderr, "Invalid command or arguments.\n");
    return 2;
}
