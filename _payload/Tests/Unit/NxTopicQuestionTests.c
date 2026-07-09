#include "Nexiora/Research/NxTopicQuestion.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static void MakeDirs(void)
{
    (void)NX_MKDIR("Knowledge");
    (void)NX_MKDIR("Knowledge/Topics");
    (void)NX_MKDIR("Knowledge/Topics/genexus");
#if defined(_WIN32)
    (void)NX_MKDIR("Knowledge\\Topics\\genexus");
#endif
}

int main(void)
{
    FILE* f;
    char answer[8192];
    NxTopicQuestionResult result;
    NxTopicQuestionStatus status;

    MakeDirs();
    f = fopen("Knowledge/Topics/genexus/memory.jsonl", "wb");
    if (f == 0)
    {
        f = fopen("Knowledge\\Topics\\genexus\\memory.jsonl", "wb");
    }
    if (f == 0)
    {
        printf("Could not create test memory.\n");
        return 1;
    }

    fprintf(f, "{\"type\":\"topic\",\"name\":\"Genexus\",\"confidence\":\"72\"}\n");
    fprintf(f, "{\"type\":\"fact\",\"topic\":\"Genexus\",\"text\":\"GeneXus es una plataforma low-code orientada al desarrollo de aplicaciones empresariales mediante una base de conocimiento.\",\"confidence\":\"70\"}\n");
    fprintf(f, "{\"type\":\"fact\",\"topic\":\"Genexus\",\"text\":\"Una Transaction en GeneXus modela entidades de negocio, atributos, reglas y comportamiento persistente.\",\"confidence\":\"70\"}\n");
    fprintf(f, "{\"type\":\"concept\",\"topic\":\"Genexus\",\"name\":\"Transaction\",\"confidence\":\"70\"}\n");
    fprintf(f, "{\"type\":\"concept\",\"topic\":\"Genexus\",\"name\":\"Knowledge Base\",\"confidence\":\"70\"}\n");
    fprintf(f, "{\"type\":\"source\",\"topic\":\"Genexus\",\"title\":\"GeneXus Documentation\",\"url\":\"https://docs.genexus.com/\"}\n");
    fclose(f);

    status = NxTopicQuestion_Ask("Genexus", "Que es una Transaction?", answer, sizeof(answer), &result);
    if (status != NX_TOPIC_QUESTION_OK)
    {
        printf("Expected OK, got %s\n", NxTopicQuestion_StatusToString(status));
        return 1;
    }

    if (strstr(answer, "Transaction") == 0)
    {
        printf("Expected answer to mention Transaction.\n%s\n", answer);
        return 1;
    }

    if (strstr(answer, "Fuentes") == 0)
    {
        printf("Expected answer to include sources.\n%s\n", answer);
        return 1;
    }

    if (result.evidence_items < 1 || result.concepts_used < 1)
    {
        printf("Expected evidence and concepts.\n");
        return 1;
    }

    return 0;
}
