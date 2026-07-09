#include "Nexiora/NCOS/NxConceptRegistry.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_concept upsert <dominio> <concepto> <definicion> <proposito> [relaciones]\n");
    printf("  nexiora_concept show <dominio> <concepto>\n");
}

int main(int argc, char** argv)
{
    const char* root = ".";
    if (argc < 2)
    {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "upsert") == 0)
    {
        if (argc < 6)
        {
            usage();
            return 2;
        }
        const char* relations = argc >= 7 ? argv[6] : "";
        NxConceptCard card;
        if (!NxConcept_Upsert(root, argv[2], argv[3], argv[4], argv[5], relations, 0.85, &card))
        {
            printf("No fue posible guardar el concepto.\n");
            return 3;
        }
        printf("Concepto guardado.\n");
        printf("Dominio : %s\n", card.domain);
        printf("Concepto: %s\n", card.name);
        printf("Version : %d\n", card.version);
        printf("Archivo : %s\n", card.path);
        return 0;
    }

    if (strcmp(argv[1], "show") == 0)
    {
        if (argc < 4)
        {
            usage();
            return 2;
        }
        NxConceptCard card;
        if (!NxConcept_Load(root, argv[2], argv[3], &card))
        {
            printf("No encontre el concepto '%s' en el dominio '%s'.\n", argv[3], argv[2]);
            return 4;
        }
        char answer[4096];
        if (!NxConcept_FormatAnswer(&card, answer, sizeof(answer)))
        {
            printf("No fue posible formatear la respuesta.\n");
            return 5;
        }
        printf("%s", answer);
        return 0;
    }

    usage();
    return 1;
}
