#include "Nexiora/Cognitive/NxCognitiveCore.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE* f;
    NxCognitiveIngestResult ingest;
    NxCognitiveAnswer answer;
    NxCognitiveStatus status;

    f = fopen("cognitive_test_genexus.txt", "wb");
    if (f == 0)
    {
        return 1;
    }
    fprintf(f,
        "GeneXus es una plataforma low-code. "
        "Una Knowledge Base sirve como repositorio central del conocimiento de una aplicacion. "
        "En una Knowledge Base se almacenan Transactions, Procedures, Data Providers, reglas y objetos. "
        "Una Transaction representa una entidad de negocio y ayuda a inferir tablas y pantallas.\n");
    fclose(f);

    status = NxCognitive_IngestFile(".", "Genexus", "cognitive_test_genexus.txt", &ingest);
    if (status != NX_COGNITIVE_OK)
    {
        printf("ingest failed: %s\n", NxCognitive_StatusToString(status));
        return 2;
    }
    if (ingest.concepts_written == 0 || ingest.chunks_written == 0)
    {
        printf("expected concepts and chunks\n");
        return 3;
    }

    status = NxCognitive_Ask(".", "Genexus", "Para que sirve una Knowledge Base?", &answer);
    if (status != NX_COGNITIVE_OK)
    {
        printf("ask failed: %s\n", NxCognitive_StatusToString(status));
        return 4;
    }
    if (strstr(answer.answer, "Knowledge Base") == 0 || strstr(answer.answer, "repositorio central") == 0)
    {
        printf("answer did not include expected evidence:\n%s\n", answer.answer);
        return 5;
    }

    return 0;
}
