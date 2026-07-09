#include "Nexiora/NCOS/NxSessionEngine.h"

#include <stdio.h>
#include <string.h>

static void print_info(const char* label, const NxSessionInfo* info)
{
    printf("%s\n", label);
    printf("Sesion : %s\n", info->name);
    printf("Estado : %s\n", info->status);
    printf("Eventos: %d\n", info->event_count);
    printf("Archivo: %s\n", info->path);
    if (info->goal[0]) printf("Objetivo: %s\n", info->goal);
}

static void usage(void)
{
    printf("Nexiora NCOS Session Engine\n\n");
    printf("Uso:\n");
    printf("  nexiora_session start <nombre> [objetivo]\n");
    printf("  nexiora_session note <texto>\n");
    printf("  nexiora_session status\n");
    printf("  nexiora_session close\n");
}

int main(int argc, char** argv)
{
    const char* root = ".";
    NxSessionInfo info;
    NxSessionResult r;

    if (argc < 2) { usage(); return 0; }

    if (strcmp(argv[1], "start") == 0)
    {
        if (argc < 3) { usage(); return 2; }
        r = NxSession_Start(root, argv[2], argc >= 4 ? argv[3] : "", &info);
        if (r != NX_SESSION_OK) { printf("Error: %s\n", NxSession_ResultName(r)); return 1; }
        print_info("Sesion iniciada.", &info);
        return 0;
    }

    if (strcmp(argv[1], "note") == 0)
    {
        if (argc < 3) { usage(); return 2; }
        r = NxSession_AddNote(root, argv[2], &info);
        if (r != NX_SESSION_OK) { printf("Error: %s\n", NxSession_ResultName(r)); return 1; }
        print_info("Nota registrada.", &info);
        return 0;
    }

    if (strcmp(argv[1], "status") == 0 || strcmp(argv[1], "continue") == 0)
    {
        r = NxSession_Status(root, &info);
        if (r != NX_SESSION_OK) { printf("No hay sesion activa.\n"); return 1; }
        print_info("Sesion activa.", &info);
        return 0;
    }

    if (strcmp(argv[1], "close") == 0)
    {
        r = NxSession_Close(root, &info);
        if (r != NX_SESSION_OK) { printf("Error: %s\n", NxSession_ResultName(r)); return 1; }
        print_info("Sesion cerrada.", &info);
        return 0;
    }

    usage();
    return 2;
}
