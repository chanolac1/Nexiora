#include "Nexiora/NCOS/NxAutoFixProposal.h"
#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("NEXIORA - Auto-Fix Proposal Engine\n\n");
    printf("Uso:\n");
    printf("  nexiora_fix_proposal propose <proposal_id> <analysis_report.md>\n");
}

int main(int argc, char** argv)
{
    NxAutoFixProposal p;
    int i;
    if (argc < 4 || strcmp(argv[1], "propose") != 0) {
        usage();
        return 1;
    }
    if (!NxAutoFixProposal_CreateFromReport(".", argv[2], argv[3], &p)) {
        printf("No fue posible crear propuesta de correccion.\n");
        printf("Resumen: %s\n", p.summary);
        return 2;
    }
    printf("================================================\n");
    printf(" NEXIORA - Auto-Fix Proposal Engine\n");
    printf("================================================\n\n");
    printf("Proposal ID : %s\n", argv[2]);
    printf("Reporte base: %s\n", argv[3]);
    printf("Sugerencias : %d\n", p.suggestion_count);
    printf("Archivo     : %s\n\n", p.proposal_path);
    printf("Resumen     : %s\n", p.summary);
    if (p.suggestion_count > 0) {
        printf("\nAcciones propuestas:\n");
        for (i = 0; i < p.suggestion_count && i < 8; ++i) {
            printf("  %d. [%s] %s (confianza %d%%)\n", i + 1,
                   p.suggestions[i].category,
                   p.suggestions[i].action,
                   p.suggestions[i].confidence);
        }
    }
    return 0;
}
