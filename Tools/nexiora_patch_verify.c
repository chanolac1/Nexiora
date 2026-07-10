#include "Nexiora/NCOS/NxPatchVerificationGate.h"
#include <stdio.h>
#include <string.h>

static void usage(void){
    printf("Uso:\n");
    printf("  nexiora_patch_verify verify <id>\n");
    printf("  nexiora_patch_verify status <id>\n");
}
static void print_result(const NxPatchVerificationResult* r){
    printf("================================================\n");
    printf(" NEXIORA - Patch Verification Gate\n");
    printf("================================================\n\n");
    printf("Run ID        : %s\n", r->id);
    printf("Estado        : %s\n", r->status);
    printf("Aprobacion    : %s\n", r->approval_found?"encontrada":"no encontrada");
    printf("Aplicacion    : %s\n", r->application_found?"encontrada":"no encontrada");
    printf("Tests         : %s\n", r->tests_passed?"PASSED":"sin evidencia suficiente");
    printf("Promovible    : %s\n", r->promotable?"SI":"NO");
    printf("Reporte       : %s\n", r->verification_report);
}
int main(int argc,char**argv){
    if(argc<3){usage(); return 2;}
    NxPatchVerificationResult r;
    if(strcmp(argv[1],"verify")==0){ if(!NxPatchVerificationGate_Verify(".",argv[2],&r)){printf("No se pudo verificar.\n"); return 1;} print_result(&r); return r.promotable?0:3; }
    if(strcmp(argv[1],"status")==0){ if(!NxPatchVerificationGate_Status(".",argv[2],&r)){printf("No existe verificacion para %s\n",argv[2]); return 1;} print_result(&r); return 0; }
    usage(); return 2;
}
