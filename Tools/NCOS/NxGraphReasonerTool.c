#include "Nexiora/NCOS/NxGraphReasoner.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_reason.exe why <dominio> <conceptoA> <conceptoB>\n");
}

int main(int argc, char** argv)
{
    char output[4096];
    if (argc < 5)
    {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "why") == 0)
    {
        int ok = NxGraphReasoner_WhyRelated(".", argv[2], argv[3], argv[4], output, sizeof(output));
        printf("%s", output);
        return ok ? 0 : 2;
    }

    usage();
    return 1;
}
