#include "Nexiora/NCP/Logging/NxLog.h"
#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/Research/NxAutonomousExecution.h"
#include "Nexiora/Research/NxResearchDashboard.h"
#include "Nexiora/Research/NxPersistentMemory.h"
#include "Nexiora/Research/NxLearningCore.h"
#include "Nexiora/Research/NxTopicQuestion.h"
#include "Nexiora/Conversation/NxConversation.h"

#include <stdio.h>
#include <string.h>

static void nx_join_args(char* dst, unsigned long dst_size, int argc, char** argv, int start)
{
    int i;
    unsigned long used = 0;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    for (i = start; i < argc; ++i)
    {
        int written;
        if (argv[i] == 0) continue;
        written = snprintf(dst + used, dst_size - used, "%s%s", used > 0 ? " " : "", argv[i]);
        if (written < 0) break;
        used += (unsigned long)written;
        if (used + 1 >= dst_size) break;
    }
}

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
        fprintf(stderr, "Conversation failed: %s\n", NxConversation_StatusToString(conversation_status));
        nx_runtime_shutdown(runtime);
        return 7;
    }
    nx_runtime_shutdown(runtime);
    return 0;
}

static int nx_handle_learn(NxRuntime* runtime, const char* topic)
{
    NxLearningCoreResult learning_result;
    NxLearningCoreStatus learning_status;
    learning_status = NxLearningCore_Learn(topic, &learning_result);
    if (learning_status != NX_LEARNING_CORE_OK && learning_status != NX_LEARNING_CORE_SCRIPT_FAILED)
    {
        fprintf(stderr, "Aprendizaje fallido: %s\n", NxLearningCore_StatusToString(learning_status));
        nx_runtime_shutdown(runtime);
        return 8;
    }
    nx_runtime_shutdown(runtime);
    return 0;
}

static int nx_handle_query(NxRuntime* runtime, const char* topic)
{
    char answer[8192];
    NxLearningCoreStatus query_status;
    query_status = NxLearningCore_Query(topic, answer, sizeof(answer));
    (void)query_status;
    printf("%s\n", answer);
    nx_runtime_shutdown(runtime);
    return query_status == NX_LEARNING_CORE_OK ? 0 : 9;
}

static int nx_answer_needs_learning(NxTopicQuestionStatus status, const char* answer)
{
    if (status != NX_TOPIC_QUESTION_OK) return 1;
    if (answer == 0) return 1;
    if (strstr(answer, "No tengo conocimiento suficiente") != 0) return 1;
    if (strstr(answer, "Ejecuta primero") != 0) return 1;
    return 0;
}

static int nx_handle_auto_question(NxRuntime* runtime, const char* topic, const char* question)
{
    char answer[16384];
    NxTopicQuestionResult question_result;
    NxTopicQuestionStatus question_status;

    printf("================================================\n");
    printf(" NEXIORA - Pregunta automatica con investigacion\n");
    printf("================================================\n\n");
    printf("Tema: %s\n", topic);
    printf("Pregunta: %s\n\n", question);

    printf("[1/4] Consultando memoria local...\n");
    question_status = NxTopicQuestion_Ask(topic, question, answer, sizeof(answer), &question_result);

    if (!nx_answer_needs_learning(question_status, answer))
    {
        printf("[2/4] Memoria suficiente. No fue necesario investigar.\n\n");
        printf("%s\n", answer);
        printf("Respuesta guardada: %s\n", question_result.answer_path);
        nx_runtime_shutdown(runtime);
        return 0;
    }

    printf("[2/4] Memoria insuficiente. Iniciando investigacion automatica...\n\n");

    {
        NxLearningCoreResult learning_result;
        NxLearningCoreStatus learning_status;
        learning_status = NxLearningCore_Learn(topic, &learning_result);
        if (learning_status != NX_LEARNING_CORE_OK && learning_status != NX_LEARNING_CORE_SCRIPT_FAILED)
        {
            fprintf(stderr, "Investigacion automatica fallida: %s\n", NxLearningCore_StatusToString(learning_status));
            nx_runtime_shutdown(runtime);
            return 11;
        }
        printf("\n[3/4] Investigacion completada. Reintentando pregunta...\n");
    }

    question_status = NxTopicQuestion_Ask(topic, question, answer, sizeof(answer), &question_result);
    printf("\n[4/4] Respuesta generada desde memoria actualizada.\n\n");
    printf("%s\n", answer);

    if (question_status != NX_TOPIC_QUESTION_OK)
    {
        printf("Nota: despues de investigar, Nexiora aun no tiene evidencia suficiente para responder con confianza.\n");
        nx_runtime_shutdown(runtime);
        return 12;
    }

    printf("Respuesta guardada: %s\n", question_result.answer_path);
    nx_runtime_shutdown(runtime);
    return 0;
}

static int nx_handle_question(NxRuntime* runtime, const char* topic, const char* question)
{
    char answer[16384];
    NxTopicQuestionResult result;
    NxTopicQuestionStatus status;
    status = NxTopicQuestion_Ask(topic, question, answer, sizeof(answer), &result);
    printf("%s\n", answer);
    if (status != NX_TOPIC_QUESTION_OK)
    {
        nx_runtime_shutdown(runtime);
        return 10;
    }
    printf("Respuesta guardada: %s\n", result.answer_path);
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

    if (argc >= 3 && strcmp(argv[1], "research") == 0 && strcmp(argv[2], "run") == 0)
    {
        NxAutonomousExecutionResult run_result;
        NxAutonomousExecutionStatus run_status;
        NxResearchDashboardResult dashboard_result;
        nx_log_write(NX_LOG_INFO, "Research", "Starting first autonomous research execution.");
        run_status = NxAutonomousExecution_RunDefault(".", &run_result);
        if (run_status != NX_AUTONOMOUS_EXECUTION_OK)
        {
            fprintf(stderr, "Autonomous research execution failed: %s\n", NxAutonomousExecution_StatusToString(run_status));
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
            fprintf(stderr, "Research dashboard generation failed: %s\n", NxResearchDashboard_StatusToString(dashboard_status));
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

    if (argc >= 3 && (strcmp(argv[1], "aprende") == 0 || strcmp(argv[1], "investiga") == 0))
    {
        char topic[512];
        nx_join_args(topic, sizeof(topic), argc, argv, 2);
        return nx_handle_learn(&runtime, topic);
    }

    if (argc >= 4 && strcmp(argv[1], "que") == 0 && strcmp(argv[2], "sabes") == 0)
    {
        char topic[512];
        nx_join_args(topic, sizeof(topic), argc, argv, 3);
        return nx_handle_query(&runtime, topic);
    }

    if (argc >= 4 && strcmp(argv[1], "pregunta") == 0)
    {
        char question[1024];
        nx_join_args(question, sizeof(question), argc, argv, 3);
        return nx_handle_question(&runtime, argv[2], question);
    }

    if (argc >= 4 && (strcmp(argv[1], "pregunta-auto") == 0 || strcmp(argv[1], "auto") == 0))
    {
        char question[1024];
        nx_join_args(question, sizeof(question), argc, argv, 3);
        return nx_handle_auto_question(&runtime, argv[2], question);
    }

    nx_log_write(NX_LOG_INFO, "Bootstrap", "Welcome to Nexiora Genesis.");
    nx_runtime_print_status(&runtime);
    printf("\nCommands:\n");
    printf("  nexiora                       Abrir conversacion con el laboratorio\n");
    printf("  nexiora research run          Execute the first autonomous research session\n");
    printf("  nexiora research dashboard    Generate dashboard.html for the latest session\n");
    printf("  nexiora memory seed           Create persistent memory from current research state\n");
    printf("  nexiora memory summary        Show persistent memory summary\n");
    printf("  nexiora aprende <tema>        Aprender un tema y guardarlo en memoria\n");
    printf("  nexiora que sabes <tema>      Consultar resumen aprendido\n");
    printf("  nexiora pregunta <tema> <q>   Preguntar algo especifico del tema\n");
    printf("  nexiora pregunta-auto <tema> <q>  Investigar automaticamente si no sabe\n");
    printf("  nexiora auto <tema> <q>       Alias de pregunta-auto\n");
    nx_runtime_shutdown(&runtime);
    return 0;
}
