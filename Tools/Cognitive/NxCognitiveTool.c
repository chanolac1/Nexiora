#include "Nexiora/Cognitive/NxCognitiveCore.h"

#include <stdio.h>
#include <string.h>

static void print_usage(void)
{
    printf("Nexiora Cognitive Core\n\n");
    printf("Uso:\n");
    printf("  nexiora_cognitive ingest <tema> <archivo>\n");
    printf("  nexiora_cognitive ask <tema> \"<pregunta>\"\n");
    printf("  nexiora_cognitive inspect <tema>\n\n");
    printf("Ejemplos:\n");
    printf("  nexiora_cognitive ingest Genexus Docs\\genexus.txt\n");
    printf("  nexiora_cognitive ask Genexus \"Para que sirve una Knowledge Base?\"\n");
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "ingest") == 0)
    {
        NxCognitiveIngestResult result;
        NxCognitiveStatus status;
        if (argc < 4)
        {
            print_usage();
            return 1;
        }
        printf("================================================\n");
        printf(" NEXIORA - Cognitive ingest\n");
        printf("================================================\n\n");
        printf("Tema   : %s\n", argv[2]);
        printf("Archivo: %s\n\n", argv[3]);
        status = NxCognitive_IngestFile(".", argv[2], argv[3], &result);
        if (status != NX_COGNITIVE_OK && status != NX_COGNITIVE_UNSUPPORTED_INPUT)
        {
            fprintf(stderr, "Ingesta fallida: %s\n", NxCognitive_StatusToString(status));
            return 2;
        }
        if (status == NX_COGNITIVE_UNSUPPORTED_INPUT)
        {
            printf("El archivo fue registrado como metadata-only.\n");
            printf("Para imagen/video/audio agrega un sidecar .txt/.srt/.vtt con transcripcion o descripcion.\n\n");
        }
        printf("Resultado:\n");
        printf("  Directorio : %s\n", result.topic_dir);
        printf("  Memoria    : %s\n", result.memory_path);
        printf("  Conceptos  : %lu\n", (unsigned long)result.concepts_written);
        printf("  Fragmentos : %lu\n", (unsigned long)result.chunks_written);
        printf("  Bytes      : %lu\n", (unsigned long)result.bytes_read);
        return status == NX_COGNITIVE_UNSUPPORTED_INPUT ? 3 : 0;
    }

    if (strcmp(argv[1], "ask") == 0)
    {
        NxCognitiveAnswer answer;
        NxCognitiveStatus status;
        if (argc < 4)
        {
            print_usage();
            return 1;
        }
        status = NxCognitive_Ask(".", argv[2], argv[3], &answer);
        if (status != NX_COGNITIVE_OK)
        {
            fprintf(stderr, "Pregunta fallida: %s\n", NxCognitive_StatusToString(status));
            return 4;
        }
        printf("================================================\n");
        printf(" NEXIORA - Respuesta cognitiva\n");
        printf("================================================\n\n");
        printf("Tema: %s\n", argv[2]);
        printf("Pregunta: %s\n\n", argv[3]);
        printf("%s\n\n", answer.answer);
        printf("Respuesta guardada: %s\n", answer.answer_path);
        return 0;
    }

    if (strcmp(argv[1], "inspect") == 0)
    {
        char output[4096];
        NxCognitiveStatus status;
        if (argc < 3)
        {
            print_usage();
            return 1;
        }
        status = NxCognitive_Inspect(".", argv[2], output, sizeof(output));
        if (status != NX_COGNITIVE_OK)
        {
            fprintf(stderr, "Inspeccion fallida: %s\n", NxCognitive_StatusToString(status));
            return 5;
        }
        printf("%s\n", output);
        return 0;
    }

    print_usage();
    return 1;
}
