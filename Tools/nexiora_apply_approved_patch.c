#include "Nexiora/NCOS/NxApprovedPatchApplyEngine.h"

#include <stdio.h>
#include <string.h>

static void print_result(const char* title, const NxApprovedPatchApplyResult* r)
{
    printf("================================================\n");
    printf(" NEXIORA - Approved Patch Apply Engine\n");
    printf("================================================\n\n");
    printf("Accion       : %s\n", title);
    printf("Run ID       : %s\n", r->run_id);
    printf("Estado       : %s\n", r->status);
    printf("Aprobado     : %s\n", r->approved ? "si" : "no");
    printf("Approval     : %s\n", r->approval_path);
    printf("Proposal     : %s\n", r->proposal_path);
    printf("Aplicacion   : %s\n", r->application_path);
    printf("Resumen      : %s\n", r->summary);
}

int main(int argc, char** argv)
{
    NxApprovedPatchApplyResult r;
    if (argc < 3) {
        printf("Uso:\n");
        printf("  nexiora_apply_approved_patch apply <run_id>\n");
        printf("  nexiora_apply_approved_patch status <run_id>\n");
        return 1;
    }
    if (strcmp(argv[1], "apply") == 0) {
        int ok = NxApprovedPatchApply_Run(".", argv[2], &r);
        print_result("apply", &r);
        return ok ? 0 : 2;
    }
    if (strcmp(argv[1], "status") == 0) {
        int ok = NxApprovedPatchApply_Status(".", argv[2], &r);
        print_result("status", &r);
        return ok ? 0 : 2;
    }
    printf("Comando no reconocido: %s\n", argv[1]);
    return 1;
}
