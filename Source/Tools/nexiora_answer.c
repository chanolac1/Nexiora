#include "Nexiora/NCOS/NxAnswerComposer.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    NxAnswerComposerResult result;
    if (argc < 5)
    {
        printf("Uso: nexiora_answer explain <dominio> <origen> <destino>\n");
        return 1;
    }
    if (NxAnswerComposer_ComposeRelationAnswer(".", argv[2], argv[3], argv[4], &result) == 0)
    {
        printf("No fue posible componer la respuesta.\n");
        return 2;
    }
    printf("%s\n\n", result.answer);
    printf("Evidencia usada:\n%s\n", result.evidence);
    printf("Confianza: %d %%\n", result.confidence);
    (void)NxAnswerComposer_WriteAnswer("ncos-006-last-answer.txt", &result);
    return 0;
}
