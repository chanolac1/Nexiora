#include "Nexiora/NCOS/NxSessionEngine.h"

#include <stdio.h>
#include <string.h>

static int assert_true(int condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

int main(void)
{
    const char* root = ".nexiora_test_session_engine_isolated";
    NxSessionInfo info;
    int failures = 0;

    failures += assert_true(NxSession_Start(root, "Proyecto Videojuego", "Crear un videojuego simple", &info) == NX_SESSION_OK, "start should succeed");
    failures += assert_true(strcmp(info.name, "proyecto_videojuego") == 0, "session name should be normalized");
    failures += assert_true(info.event_count >= 1, "start should create event");

    failures += assert_true(NxSession_AddNote(root, "Definir loop de juego", &info) == NX_SESSION_OK, "note should succeed");
    failures += assert_true(info.event_count >= 2, "note should append event");

    failures += assert_true(NxSession_Status(root, &info) == NX_SESSION_OK, "status should find active session");
    failures += assert_true(strcmp(info.status, "active") == 0, "status should be active");

    failures += assert_true(NxSession_Close(root, &info) == NX_SESSION_OK, "close should succeed");
    failures += assert_true(strcmp(info.status, "closed") == 0, "status should be closed");

    failures += assert_true(NxSession_Status(root, &info) == NX_SESSION_NOT_FOUND, "status should not find active session after close");

    if (failures == 0)
    {
        printf("NxSessionEngineTests passed\n");
        return 0;
    }
    return 1;
}
