#include "Nexiora/NCP/Logging/NxLog.h"
#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/Research/NxAutonomousExecution.h"
#include "Nexiora/Research/NxResearchDashboard.h"
#include "Nexiora/Research/NxPersistentMemory.h"
#include "Nexiora/Conversation/NxConversation.h"
#include "Nexiora/Research/NxTopicInvestigation.h"
#include "Nexiora/Research/NxKnowledgeStore.h"
#include "Nexiora/Research/NxLearningCore.h"

#include <stdio.h>
#include <string.h>

static void nx_print_research_run_result(const NxAutonomousExecutionResult* result)
{
    printf("------------------------------------------------\n");
    printf(" Nexiora Autonomous Research Session\n");
    printf("------------------------------------------------\n\n");
    printf("Session Path          : %s\n", result->session_path);
    printf("Experiments Discovered: %zu\n", result->experiments_discovered);
    printf("Experiments Executed  : %zu\n", result->experiments_executed);
    printf("Evidence Generated    : %zu\n", result->evidence_generated);
    printf("Promotion Candidates  : %zu\n", result->promotion_candidates);
    printf("Knowledge Graph Nodes : %zu\n", result->graph_nodes);
    printf("Knowledge Graph Edges : %zu\n\n", result->graph_edges);
    printf("Recommendation Gate   : Waiting for human approval\n");
    printf("Runtime Promotion     : Not automatic\n\n");
    printf("Artifacts:\n");
    printf("  summary.txt\n");
    printf("  report.md\n");
    printf("  metrics.json\n");
    printf("  knowledge.json\n");
    printf("  graph.json\n");
    printf("  graph.dot\n");
    printf("  graph.svg\n");
    printf("  dashboard.html\n\n");
    printf("%s\n", result->message);
}

static void nx_print_memory_summary(const NxPersistentMemorySummary* summary, const char* path)
{
    printf("------------------------------------------------\n");
    printf(" Nexiora Persistent Knowledge Memory\n");
    printf("------------------------------------------------\n\n");
    printf("Memory Path        : %s\n", path);
    printf("Facts              : %zu\n", summary->facts);
    printf("Decisions          : %zu\n", summary->decisions);
    printf("Hypotheses         : %zu\n", summary->hypotheses);
    printf("Concepts           : %zu\n", summary->concepts);
    printf("Average Confidence : %d%%\n\n", summary->average_confidence);
    printf("Nexiora now preserves persistent research memory.\n");
}

static void nx_print_dashboard_result(const NxResearchDashboardResult* result)
{
    printf("------------------------------------------------\n");
    printf(" Nexiora Research Dashboard\n");
    printf("------------------------------------------------\n\n");
    printf("Dashboard Path : %s\n\n", result->dashboard_path);
    printf("Artifacts detected:\n");
    printf("  summary.txt             : %s\n", result->has_summary ? "yes" : "no");
    printf("  report.md               : %s\n", result->has_report ? "yes" : "no");
    printf("  metrics.json            : %s\n", result->has_metrics ? "yes" : "no");
    printf("  knowledge.json          : %s\n", result->has_knowledge ? "yes" : "no");
    printf("  graph.json              : %s\n", result->has_graph_json ? "yes" : "no");
    printf("  graph.dot               : %s\n", result->has_graph_dot ? "yes" : "no");
    printf("  graph.svg               : %s\n\n", result->has_graph_svg ? "yes" : "no");
    printf("%s\n", result->message);
}

static int nx_run_conversation(NxRuntime* runtime)
{
    NxConversationStatus conversation_status;

    nx_log_write(NX_LOG_INFO, "Conversation", "Starting Nexiora laboratory conversation.");
    conversation_status = NxConversation_RunInteractive(stdin, stdout);
    if (conversation_status != NX_CONVERSATION_OK)
    {
        fprintf(stderr,
            "Conversation failed: %s\n",
            NxConversation_StatusToString(conversation_status));
        nx_runtime_shutdown(runtime);
        return 7;
    }

    nx_runtime_shutdown(runtime);
    return 0;
}

int main(int argc, char** argv)
{
    NxRuntime runtime;
    NxResult result;

    result = nx_runtime_initialize(&runtime, "nexiora.log");
    if (result != NX_OK)
    {
        fprintf(stderr, "Failed to initialize Nexiora: %s\n", nx_result_to_string(result));
        return 1;
    }

    if (argc == 1)
    {
        return nx_run_conversation(&runtime);
    }


    /* CORE-0001 real learning command */
    if (argc >= 3 && (strcmp(argv[1], "aprende") == 0 || strcmp(argv[1], "investiga") == 0))
    {
        char topic[256];
        NxLearningCoreResult learning_result;
        NxLearningCoreStatus learning_status;
        int i;

        topic[0] = '\0';
        for (i = 2; i < argc; ++i)
        {
            if (i > 2)
            {
                (void)strncat(topic, " ", sizeof(topic) - strlen(topic) - 1);
            }
            (void)strncat(topic, argv[i], sizeof(topic) - strlen(topic) - 1);
        }

        learning_status = NxLearningCore_Learn(topic, &learning_result);
        if (learning_status != NX_LEARNING_CORE_OK && learning_status != NX_LEARNING_CORE_SCRIPT_FAILED)
        {
            fprintf(stderr, "Learning failed: %s\n", NxLearningCore_StatusToString(learning_status));
            nx_runtime_shutdown(&runtime);
            return 8;
        }

        nx_runtime_shutdown(&runtime);
        return 0;
    }

    /* CORE-0001 memory query command */
    if (argc >= 4 && strcmp(argv[1], "que") == 0 && strcmp(argv[2], "sabes") == 0)
    {
        char topic[256];
        char answer[8192];
        NxLearningCoreStatus query_status;
        int i;

        topic[0] = '\0';
        for (i = 3; i < argc; ++i)
        {
            if (i > 3)
            {
                (void)strncat(topic, " ", sizeof(topic) - strlen(topic) - 1);
            }
            (void)strncat(topic, argv[i], sizeof(topic) - strlen(topic) - 1);
        }

        query_status = NxLearningCore_Query(topic, answer, sizeof(answer));
        printf("%s\n", answer);
        nx_runtime_shutdown(&runtime);
        return query_status == NX_LEARNING_CORE_OK ? 0 : 10;
    }
    if (argc >= 3 && strcmp(argv[1], "research") == 0 && strcmp(argv[2], "run") == 0)
    {
        NxAutonomousExecutionResult run_result;
        NxAutonomousExecutionStatus run_status;
        NxResearchDashboardResult dashboard_result;

        nx_log_write(NX_LOG_INFO, "Research", "Starting first autonomous research execution.");
        run_status = NxAutonomousExecution_RunDefault(".", &run_result);
        if (run_status != NX_AUTONOMOUS_EXECUTION_OK)
        {
            fprintf(stderr,
                "Autonomous research execution failed: %s\n",
                NxAutonomousExecution_StatusToString(run_status));
            nx_runtime_shutdown(&runtime);
            return 2;
        }

        (void)NxResearchDashboard_Generate(run_result.session_path, &dashboard_result);
        nx_print_research_run_result(&run_result);
        printf("Dashboard generated : %s\n", dashboard_result.dashboard_path);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "research") == 0 && strcmp(argv[2], "dashboard") == 0)
    {
        NxResearchDashboardResult dashboard_result;
        NxResearchDashboardStatus dashboard_status;

        nx_log_write(NX_LOG_INFO, "Research", "Generating autonomous research dashboard.");
        dashboard_status = NxResearchDashboard_GenerateDefault(".", &dashboard_result);
        if (dashboard_status != NX_RESEARCH_DASHBOARD_OK)
        {
            fprintf(stderr,
                "Research dashboard generation failed: %s\n",
                NxResearchDashboard_StatusToString(dashboard_status));
            nx_runtime_shutdown(&runtime);
            return 3;
        }

        nx_print_dashboard_result(&dashboard_result);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "memory") == 0 && strcmp(argv[2], "seed") == 0)
    {
        char memory_path[260];
        NxPersistentMemoryStatus memory_status;

        memory_status = NxPersistentMemory_SeedFromFirstAutonomousExecution(".", memory_path, sizeof(memory_path));
        if (memory_status != NX_PERSISTENT_MEMORY_OK)
        {
            fprintf(stderr, "Persistent memory seed failed: %s\n", NxPersistentMemory_StatusToString(memory_status));
            nx_runtime_shutdown(&runtime);
            return 4;
        }

        printf("Persistent memory created: %s\n", memory_path);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "memory") == 0 && strcmp(argv[2], "summary") == 0)
    {
        NxPersistentMemory memory;
        NxPersistentMemorySummary summary;
        NxPersistentMemoryStatus memory_status;
        const char* memory_path = ".\\Knowledge\\Memory\\memory.jsonl";

        memory_status = NxPersistentMemory_Init(&memory);
        if (memory_status != NX_PERSISTENT_MEMORY_OK)
        {
            fprintf(stderr, "Persistent memory init failed.\n");
            nx_runtime_shutdown(&runtime);
            return 5;
        }

        memory_status = NxPersistentMemory_LoadJsonl(&memory, memory_path);
        if (memory_status != NX_PERSISTENT_MEMORY_OK)
        {
            fprintf(stderr, "Persistent memory load failed: %s\n", NxPersistentMemory_StatusToString(memory_status));
            NxPersistentMemory_Shutdown(&memory);
            nx_runtime_shutdown(&runtime);
            return 6;
        }

        (void)NxPersistentMemory_GetSummary(&memory, &summary);
        nx_print_memory_summary(&summary, memory_path);
        NxPersistentMemory_Shutdown(&memory);
        nx_runtime_shutdown(&runtime);
        return 0;
    }


    if (argc >= 3 && strcmp(argv[1], "investiga") == 0)
    {
        NxTopicInvestigationResult investigation_result;
        NxTopicInvestigationStatus investigation_status;

        investigation_status = NxTopicInvestigation_Run(".", argv[2], stdout, &investigation_result);
        if (investigation_status != NX_TOPIC_INVESTIGATION_OK)
        {
            fprintf(stderr,
                "Investigation failed: %s\n",
                NxTopicInvestigation_StatusToString(investigation_status));
            nx_runtime_shutdown(&runtime);
            return 8;
        }

        nx_runtime_shutdown(&runtime);
        return 0;
    }

    if (argc >= 4 && strcmp(argv[1], "que") == 0 && strcmp(argv[2], "sabes") == 0)
    {
        NxKnowledgeAnswer answer;
        NxKnowledgeStoreStatus knowledge_status;
        char answer_text[2048];

        knowledge_status = NxKnowledgeStore_Query(".", argv[3], &answer);
        if (knowledge_status != NX_KS_OK)
        {
            fprintf(stderr,
                "No tengo conocimiento suficiente sobre '%s'. Ejecuta primero: nexiora investiga %s\n",
                argv[3], argv[3]);
            nx_runtime_shutdown(&runtime);
            return 9;
        }

        (void)NxKnowledgeStore_FormatAnswerSpanish(&answer, answer_text, sizeof(answer_text));
        printf("%s\n", answer_text);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    nx_log_write(NX_LOG_INFO, "Bootstrap", "Welcome to Nexiora Genesis.");
    nx_runtime_print_status(&runtime);
    printf("\nCommands:\n");
    printf("  nexiora                       Abrir conversacion con el laboratorio\n");
    printf("  nexiora research run          Execute the first autonomous research session\n");
    printf("  nexiora research dashboard    Generate dashboard.html for the latest session\n");
    printf("  nexiora memory seed           Create persistent memory from current research state\n");
    printf("  nexiora memory summary        Show persistent memory summary\n");
    printf("  nexiora aprende <tema>        Aprende un tema usando conectores reales\n");
    printf("  nexiora que sabes <tema>      Consulta lo aprendido sobre un tema\n");
    printf("  nexiora investiga <tema>      Ejecutar aprendizaje observable sobre un tema\n");
    printf("  nexiora que sabes <tema>      Consultar memoria aprendida\n");

    nx_runtime_shutdown(&runtime);
    return 0;
}

