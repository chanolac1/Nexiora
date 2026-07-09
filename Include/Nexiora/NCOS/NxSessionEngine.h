#ifndef NEXIORA_NCOS_NX_SESSION_ENGINE_H
#define NEXIORA_NCOS_NX_SESSION_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxSessionInfo
{
    char name[128];
    char path[512];
    char goal[512];
    char status[64];
    int event_count;
} NxSessionInfo;

typedef enum NxSessionResult
{
    NX_SESSION_OK = 0,
    NX_SESSION_INVALID_ARGUMENT = 1,
    NX_SESSION_IO_ERROR = 2,
    NX_SESSION_NOT_FOUND = 3
} NxSessionResult;

NxSessionResult NxSession_Start(const char* root, const char* name, const char* goal, NxSessionInfo* out_info);
NxSessionResult NxSession_AddNote(const char* root, const char* note, NxSessionInfo* out_info);
NxSessionResult NxSession_Status(const char* root, NxSessionInfo* out_info);
NxSessionResult NxSession_Close(const char* root, NxSessionInfo* out_info);
const char* NxSession_ResultName(NxSessionResult result);

#ifdef __cplusplus
}
#endif

#endif
