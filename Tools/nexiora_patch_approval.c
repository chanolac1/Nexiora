#include "Nexiora/NCOS/NxPatchApprovalEngine.h"

#include <stdio.h>
#include <string.h>

static void print_result(const NxPatchApprovalResult* r)
{
    printf("================================================\n");
    printf(" NEXIORA - Patch Approval Engine\n");
    printf("================================================\n\n");
    printf("Run ID   : %s\n", r->run_id);
    printf("Estado   : %s\n", r->status);
    printf("Reviewer : %s\n", r->reviewer);
    printf("Archivo  : %s\n", r->approval_path);
    printf("Proposal : %s\n", r->proposal_path);
    printf("Resumen  : %s\n", r->summary);
}

static int usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_patch_approval request <run_id> <proposal.md>\n");
    printf("  nexiora_patch_approval approve <run_id> <reviewer>\n");
    printf("  nexiora_patch_approval reject <run_id> <reviewer> <reason>\n");
    printf("  nexiora_patch_approval status <run_id>\n");
    return 2;
}

int main(int argc, char** argv)
{
    NxPatchApprovalResult r;
    if (argc < 3) return usage();

    if (strcmp(argv[1], "request") == 0) {
        if (argc < 4) return usage();
        if (!NxPatchApproval_Request(".", argv[2], argv[3], &r)) return 1;
        print_result(&r);
        return 0;
    }
    if (strcmp(argv[1], "approve") == 0) {
        if (argc < 4) return usage();
        if (!NxPatchApproval_Approve(".", argv[2], argv[3], &r)) return 1;
        print_result(&r);
        return 0;
    }
    if (strcmp(argv[1], "reject") == 0) {
        if (argc < 5) return usage();
        if (!NxPatchApproval_Reject(".", argv[2], argv[3], argv[4], &r)) return 1;
        print_result(&r);
        return 0;
    }
    if (strcmp(argv[1], "status") == 0) {
        if (!NxPatchApproval_Status(".", argv[2], &r)) return 1;
        print_result(&r);
        return 0;
    }

    return usage();
}
