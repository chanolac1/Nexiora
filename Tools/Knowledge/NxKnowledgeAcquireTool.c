#include "Nexiora/Research/NxKnowledgeAcquisition.h"

#include <stdio.h>
#include <string.h>

static void print_plan(const NxKnowledgeAcquisitionPlan* plan)
{
    size_t i;

    printf("================================================\n");
    printf(" NEXIORA - Adquisicion de conocimiento\n");
    printf("================================================\n\n");
    printf("Objetivo: %s\n", plan->topic);
    printf("Tipo: %s\n", plan->kind);
    printf("Tiempo estimado: %u minutos\n", plan->estimated_minutes);
    printf("Confianza esperada: %u %%\n\n", plan->expected_confidence);

    printf("Fuentes candidatas:\n");
    for (i = 0; i < plan->source_count; ++i)
    {
        printf("  %zu. %s [%s]\n", i + 1U, plan->sources[i].name,
            NxKnowledgeAcquisition_SourceTypeToString(plan->sources[i].type));
        printf("     Prioridad: %u | Confianza: %u %%\n", plan->sources[i].priority, plan->sources[i].trust_score);
        printf("     Motivo: %s\n", plan->sources[i].reason);
    }

    printf("\nPlan:\n");
    for (i = 0; i < plan->step_count; ++i)
    {
        printf("  %zu. %s\n", i + 1U, plan->steps[i]);
    }

    printf("\nNota: esta epica genera el plan y artefactos locales.\n");
    printf("La adquisicion desde Internet se habilitara mediante providers controlados.\n");
}

int main(int argc, char** argv)
{
    char topic[256];
    size_t i;
    NxKnowledgeAcquisitionPlan plan;
    NxKnowledgeAcquisitionStatus status;
    char md_path[260];
    char json_path[260];

    if (argc < 2)
    {
        printf("Uso:\n");
        printf("  nexiora_acquire SQLite\n");
        printf("  nexiora_acquire libro Clean Architecture\n");
        printf("  nexiora_acquire RFC QUIC\n");
        return 1;
    }

    topic[0] = '\0';
    for (i = 1; i < (size_t)argc; ++i)
    {
        if (i > 1)
        {
            (void)strncat(topic, " ", sizeof(topic) - strlen(topic) - 1U);
        }
        (void)strncat(topic, argv[i], sizeof(topic) - strlen(topic) - 1U);
    }

    status = NxKnowledgeAcquisition_BuildPlan(topic, &plan);
    if (status != NX_KA_STATUS_OK)
    {
        fprintf(stderr, "No se pudo crear el plan: %s\n", NxKnowledgeAcquisition_StatusToString(status));
        return 2;
    }

    print_plan(&plan);

    status = NxKnowledgeAcquisition_WritePlanMarkdown(&plan, ".", md_path, sizeof(md_path));
    if (status != NX_KA_STATUS_OK)
    {
        fprintf(stderr, "No se pudo escribir plan.md: %s\n", NxKnowledgeAcquisition_StatusToString(status));
        return 3;
    }

    status = NxKnowledgeAcquisition_WritePlanJson(&plan, ".", json_path, sizeof(json_path));
    if (status != NX_KA_STATUS_OK)
    {
        fprintf(stderr, "No se pudo escribir plan.json: %s\n", NxKnowledgeAcquisition_StatusToString(status));
        return 4;
    }

    printf("\nArtefactos generados:\n");
    printf("  %s\n", md_path);
    printf("  %s\n", json_path);

    return 0;
}
