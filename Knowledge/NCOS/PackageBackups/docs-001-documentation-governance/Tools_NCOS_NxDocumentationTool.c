#include "Nexiora/NCOS/NxDocumentationManager.h"
#include <stdio.h>
#include <string.h>

static void nx_usage(void)
{
    puts("Uso:");
    puts("  nexiora_docs validate <repo_root>");
    puts("  nexiora_docs finalize-sprint <repo_root> <release.nxdoc>");
}

int main(int argc, char** argv)
{
    if (argc < 3) { nx_usage(); return 2; }
    if (strcmp(argv[1], "validate") == 0) {
        NxDocumentationValidationResult result;
        int ok = NxDocumentationManager_Validate(argv[2], &result);
        printf("%s\n", result.message);
        return ok ? 0 : 1;
    }
    if (strcmp(argv[1], "finalize-sprint") == 0 && argc == 4) {
        NxDocumentationRelease release;
        char message[512];
        if (!NxDocumentationManager_LoadRelease(argv[3], &release, message, sizeof(message))) {
            fprintf(stderr, "%s\n", message);
            return 1;
        }
        if (!NxDocumentationManager_FinalizeSprint(argv[2], &release, message, sizeof(message))) {
            fprintf(stderr, "%s\n", message);
            return 1;
        }
        puts(message);
        return 0;
    }
    nx_usage();
    return 2;
}
