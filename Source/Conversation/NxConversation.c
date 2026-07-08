#include "Nexiora/Conversation/NxConversation.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void nx_conversation_lower_ascii(char* dst, size_t dst_size, const char* src)
{
    size_t i;

    if (dst == 0 || dst_size == 0)
    {
        return;
    }

    if (src == 0)
    {
        dst[0] = '\0';
        return;
    }

    for (i = 0; i + 1 < dst_size && src[i] != '\0'; ++i)
    {
        unsigned char ch = (unsigned char)src[i];
        dst[i] = (char)tolower(ch);
    }

    dst[i] = '\0';
}

static int nx_conversation_contains(const char* haystack, const char* needle)
{
    return strstr(haystack, needle) != 0;
}

static NxConversationIntent nx_conversation_detect_intent(const char* input)
{
    char text[512];

    nx_conversation_lower_ascii(text, sizeof(text), input);

    if (nx_conversation_contains(text, "salir") ||
        nx_conversation_contains(text, "exit") ||
        nx_conversation_contains(text, "adios") ||
        nx_conversation_contains(text, "adiós"))
    {
        return NX_CONVERSATION_INTENT_EXIT;
    }

    if (nx_conversation_contains(text, "ayuda") ||
        nx_conversation_contains(text, "help") ||
        nx_conversation_contains(text, "comandos"))
    {
        return NX_CONVERSATION_INTENT_HELP;
    }

    if (nx_conversation_contains(text, "sorprendeme") ||
        nx_conversation_contains(text, "sorpréndeme"))
    {
        return NX_CONVERSATION_INTENT_SURPRISE;
    }

    if (nx_conversation_contains(text, "aprend") ||
        nx_conversation_contains(text, "que sabes") ||
        nx_conversation_contains(text, "qué sabes"))
    {
        return NX_CONVERSATION_INTENT_LEARNED;
    }

    if (nx_conversation_contains(text, "recomienda") ||
        nx_conversation_contains(text, "recomendaciones") ||
        nx_conversation_contains(text, "que sigue") ||
        nx_conversation_contains(text, "qué sigue"))
    {
        return NX_CONVERSATION_INTENT_RECOMMENDATIONS;
    }

    if (nx_conversation_contains(text, "ultima") ||
        nx_conversation_contains(text, "última") ||
        nx_conversation_contains(text, "ultimo") ||
        nx_conversation_contains(text, "último"))
    {
        return NX_CONVERSATION_INTENT_LAST_SESSION;
    }

    if (nx_conversation_contains(text, "sesiones") ||
        nx_conversation_contains(text, "historial"))
    {
        return NX_CONVERSATION_INTENT_SESSIONS;
    }

    if (nx_conversation_contains(text, "investiga") ||
        nx_conversation_contains(text, "investigar") ||
        nx_conversation_contains(text, "analiza") ||
        nx_conversation_contains(text, "revisa"))
    {
        return NX_CONVERSATION_INTENT_RESEARCH;
    }

    if (nx_conversation_contains(text, "estado") ||
        nx_conversation_contains(text, "como estas") ||
        nx_conversation_contains(text, "cómo estás") ||
        nx_conversation_contains(text, "como vas") ||
        nx_conversation_contains(text, "cómo vas"))
    {
        return NX_CONVERSATION_INTENT_STATUS;
    }

    return NX_CONVERSATION_INTENT_UNKNOWN;
}

static NxConversationStatus nx_conversation_write(
    NxConversationResponse* response,
    NxConversationIntent intent,
    unsigned confidence,
    int should_exit,
    const char* text)
{
    int written;

    if (response == 0 || text == 0)
    {
        return NX_CONVERSATION_INVALID_ARGUMENT;
    }

    response->intent = intent;
    response->confidence_percent = confidence;
    response->should_exit = should_exit;

    written = snprintf(response->text, sizeof(response->text), "%s", text);
    if (written < 0 || (size_t)written >= sizeof(response->text))
    {
        response->text[sizeof(response->text) - 1] = '\0';
        return NX_CONVERSATION_OUTPUT_TOO_SMALL;
    }

    return NX_CONVERSATION_OK;
}

NxConversationStatus NxConversation_Respond(
    const char* input,
    NxConversationResponse* response_out)
{
    NxConversationIntent intent;

    if (input == 0 || response_out == 0)
    {
        return NX_CONVERSATION_INVALID_ARGUMENT;
    }

    intent = nx_conversation_detect_intent(input);

    switch (intent)
    {
        case NX_CONVERSATION_INTENT_HELP:
            return nx_conversation_write(response_out, intent, 100, 0,
                "Comandos disponibles:\n\n"
                "  ayuda\n"
                "  estado\n"
                "  investigar\n"
                "  sesiones\n"
                "  ultima sesion\n"
                "  que aprendiste\n"
                "  recomendaciones\n"
                "  sorprendeme\n"
                "  salir\n\n"
                "Puedes escribirlos como frases naturales. Ejemplos:\n"
                "  ¿Cómo vas?\n"
                "  ¿Qué aprendiste?\n"
                "  Investiga el Scheduler\n");

        case NX_CONVERSATION_INTENT_STATUS:
            return nx_conversation_write(response_out, intent, 90, 0,
                "Estado del laboratorio:\n\n"
                "  Estado general       : disponible\n"
                "  Investigacion activa : no\n"
                "  Runtime              : sin promocion automatica\n"
                "  Decision humana      : requerida para promover cambios\n\n"
                "Mi lectura actual: puedo ejecutar investigaciones, generar evidencia,\n"
                "crear reportes y mostrar el dashboard. Todavia estoy aprendiendo a\n"
                "mantener memoria persistente mas profunda.\n\n"
                "Confianza: 90 %\n");

        case NX_CONVERSATION_INTENT_RESEARCH:
            return nx_conversation_write(response_out, intent, 86, 0,
                "Entendido. Puedo ayudarte a preparar una investigacion.\n\n"
                "En este punto puedo iniciar la corrida autonoma existente con:\n\n"
                "  nexiora research run\n\n"
                "Para una investigacion especifica, mi siguiente paso recomendado es:\n"
                "1. registrar el objetivo,\n"
                "2. buscar evidencia previa,\n"
                "3. ejecutar experimentos relacionados,\n"
                "4. generar reporte,\n"
                "5. esperar aprobacion humana.\n\n"
                "Confianza: 86 %\n");

        case NX_CONVERSATION_INTENT_SESSIONS:
            return nx_conversation_write(response_out, intent, 82, 0,
                "Sesiones de investigacion:\n\n"
                "La sesion principal conocida por esta version es:\n\n"
                "  Research/Sessions/first_autonomous_execution\n\n"
                "Puedes revisarla con:\n"
                "  nexiora research dashboard\n\n"
                "Confianza: 82 %\n");

        case NX_CONVERSATION_INTENT_LAST_SESSION:
            return nx_conversation_write(response_out, intent, 88, 0,
                "Ultima sesion conocida:\n\n"
                "  first_autonomous_execution\n\n"
                "Artefactos esperados:\n"
                "  summary.txt\n"
                "  report.md\n"
                "  metrics.json\n"
                "  knowledge.json\n"
                "  graph.json\n"
                "  graph.svg\n"
                "  dashboard.html\n\n"
                "Si quieres verla, ejecuta:\n"
                "  nexiora research dashboard\n\n"
                "Confianza: 88 %\n");

        case NX_CONVERSATION_INTENT_LEARNED:
            return nx_conversation_write(response_out, intent, 80, 0,
                "Lo que he aprendido hasta ahora:\n\n"
                "  1. Puedo ejecutar un ciclo autonomo de investigacion.\n"
                "  2. Toda promocion al Runtime debe esperar aprobacion humana.\n"
                "  3. La evidencia debe existir antes de cualquier recomendacion fuerte.\n"
                "  4. El dashboard ayuda a observar los artefactos generados.\n\n"
                "Aun no debo fingir conocimiento que no exista en la evidencia.\n\n"
                "Confianza: 80 %\n");

        case NX_CONVERSATION_INTENT_RECOMMENDATIONS:
            return nx_conversation_write(response_out, intent, 84, 0,
                "Recomendacion actual:\n\n"
                "  Prioridad 1: consolidar memoria persistente.\n"
                "  Prioridad 2: mejorar preguntas sobre sesiones y evidencia.\n"
                "  Prioridad 3: permitir investigaciones con objetivo especifico.\n\n"
                "Motivo:\n"
                "  Ya existe una primera ejecucion autonoma. Ahora conviene que\n"
                "  Nexiora recuerde, consulte y explique mejor lo que produjo.\n\n"
                "Confianza: 84 %\n");

        case NX_CONVERSATION_INTENT_SURPRISE:
            return nx_conversation_write(response_out, intent, 87, 0,
                "Hay algo importante que observo:\n\n"
                "  Nexiora ya puede ejecutar una investigacion completa, pero todavia\n"
                "  necesita convertir esos resultados en memoria consultable.\n\n"
                "Mi sugerencia es no agregar mas modulos aislados hasta que puedas\n"
                "preguntar con naturalidad:\n\n"
                "  ¿Que evidencia respalda esta recomendacion?\n\n"
                "Ese sera un salto grande hacia un companero de ingenieria real.\n\n"
                "Confianza: 87 %\n");

        case NX_CONVERSATION_INTENT_EXIT:
            return nx_conversation_write(response_out, intent, 100, 1,
                "Cerrando conversacion con el laboratorio.\n"
                "No se realizaron cambios en el Runtime.\n");

        case NX_CONVERSATION_INTENT_UNKNOWN:
        default:
            return nx_conversation_write(response_out, intent, 55, 0,
                "Todavia no entiendo completamente esa solicitud.\n\n"
                "Puedo ayudarte con:\n"
                "  ayuda\n"
                "  estado\n"
                "  investigar\n"
                "  sesiones\n"
                "  que aprendiste\n"
                "  recomendaciones\n\n"
                "Mi respuesta es conservadora porque no debo inventar una conclusion\n"
                "sin evidencia suficiente.\n\n"
                "Confianza: 55 %\n");
    }
}

NxConversationStatus NxConversation_RunInteractive(FILE* input, FILE* output)
{
    char line[512];

    if (input == 0 || output == 0)
    {
        return NX_CONVERSATION_INVALID_ARGUMENT;
    }

    fprintf(output, "═══════════════════════════════════════════════════════\n");
    fprintf(output, "                 N E X I O R A\n");
    fprintf(output, "      Laboratorio Autonomo de Investigacion\n");
    fprintf(output, "═══════════════════════════════════════════════════════\n\n");
    fprintf(output, "Estado: listo.\n");
    fprintf(output, "Escribe \"ayuda\" para comenzar.\n\n");

    for (;;)
    {
        NxConversationResponse response;
        NxConversationStatus status;

        fprintf(output, "> ");
        fflush(output);

        if (fgets(line, sizeof(line), input) == 0)
        {
            fprintf(output, "\n");
            break;
        }

        status = NxConversation_Respond(line, &response);
        if (status != NX_CONVERSATION_OK)
        {
            fprintf(output, "Error conversacional: %s\n", NxConversation_StatusToString(status));
            return status;
        }

        fprintf(output, "\n%s\n", response.text);

        if (response.should_exit)
        {
            break;
        }
    }

    return NX_CONVERSATION_OK;
}

const char* NxConversation_StatusToString(NxConversationStatus status)
{
    switch (status)
    {
        case NX_CONVERSATION_OK: return "OK";
        case NX_CONVERSATION_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_CONVERSATION_OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
        default: return "UNKNOWN";
    }
}

const char* NxConversation_IntentToString(NxConversationIntent intent)
{
    switch (intent)
    {
        case NX_CONVERSATION_INTENT_UNKNOWN: return "UNKNOWN";
        case NX_CONVERSATION_INTENT_HELP: return "HELP";
        case NX_CONVERSATION_INTENT_STATUS: return "STATUS";
        case NX_CONVERSATION_INTENT_RESEARCH: return "RESEARCH";
        case NX_CONVERSATION_INTENT_SESSIONS: return "SESSIONS";
        case NX_CONVERSATION_INTENT_LAST_SESSION: return "LAST_SESSION";
        case NX_CONVERSATION_INTENT_LEARNED: return "LEARNED";
        case NX_CONVERSATION_INTENT_RECOMMENDATIONS: return "RECOMMENDATIONS";
        case NX_CONVERSATION_INTENT_SURPRISE: return "SURPRISE";
        case NX_CONVERSATION_INTENT_EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}
