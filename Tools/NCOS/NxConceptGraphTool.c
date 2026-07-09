#include "Nexiora/NCOS/NxConceptGraph.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_graph link <dominio> <origen> <relacion> <destino>\n");
    printf("  nexiora_graph show <dominio> <concepto>\n");
    printf("  nexiora_graph stats <dominio> <concepto>\n");
}

int main(int argc, char** argv)
{
    const char* root = ".";
    if (argc < 2)
    {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "link") == 0)
    {
        NxConceptGraphEdge edge;
        if (argc < 6)
        {
            usage();
            return 1;
        }
        if (!NxConceptGraph_Link(root, argv[2], argv[3], argv[4], argv[5], "cli", 85, &edge))
        {
            printf("No se pudo registrar la relacion.\n");
            return 2;
        }
        printf("Relacion registrada.\n");
        printf("Dominio : %s\n", edge.domain);
        printf("Origen  : %s\n", edge.from);
        printf("Relacion: %s\n", edge.relation);
        printf("Destino : %s\n", edge.to);
        printf("Archivo : %s\n", edge.path);
        return 0;
    }

    if (strcmp(argv[1], "show") == 0)
    {
        char out[4096];
        if (argc < 4)
        {
            usage();
            return 1;
        }
        if (!NxConceptGraph_Show(root, argv[2], argv[3], out, sizeof(out)))
        {
            printf("No se pudo mostrar el grafo.\n");
            return 2;
        }
        printf("%s", out);
        return 0;
    }

    if (strcmp(argv[1], "stats") == 0)
    {
        NxConceptGraphStats stats;
        if (argc < 4)
        {
            usage();
            return 1;
        }
        if (!NxConceptGraph_Stats(root, argv[2], argv[3], &stats))
        {
            printf("No se pudieron calcular estadisticas.\n");
            return 2;
        }
        printf("Estadisticas de grafo\n");
        printf("Aristas totales : %d\n", stats.edge_count);
        printf("Salientes       : %d\n", stats.outgoing_count);
        printf("Entrantes       : %d\n", stats.incoming_count);
        printf("Archivo         : %s\n", stats.path);
        return 0;
    }

    usage();
    return 1;
}
