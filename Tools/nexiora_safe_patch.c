#include "Nexiora/NCOS/NxSafePatchEngine.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    NxSafePatchResult result;
    if (argc < 4) {
        printf("Uso: nexiora_safe_patch propose <run_id> <analysis.md>\n");
        return 2;
    }
    if (strcmp(argv[1], "propose") != 0) {
        printf("Comando no reconocido: %s\n", argv[1]);
        return 2;
    }
    if (!NxSafePatch_CreateProposal(".", argv[2], argv[3], &result)) {
        printf("No se pudo generar propuesta de parche seguro.\n");
        return 1;
    }
    printf("================================================\n");
    printf(" NEXIORA - Safe Patch Engine\n");
    printf("================================================\n\n");
    printf("Run ID              : %s\n", result.run_id);
    printf("Estado              : PROPOSED\n");
    printf("Cambios propuestos  : %d\n", result.proposed_changes);
    printf("Aprobacion humana   : requerida\n");
    printf("Propuesta           : %s\n", result.proposal_path);
    printf("Resumen             : %s\n", result.summary);
    return 0;
}
