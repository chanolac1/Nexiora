#ifndef NEXIORA_TOOLS_NX_TOOL_MANAGER_H
#define NEXIORA_TOOLS_NX_TOOL_MANAGER_H

#include <stddef.h>

typedef enum NxToolStatus {
    NX_TOOL_OK = 0,
    NX_TOOL_INVALID_ARGUMENT = 1,
    NX_TOOL_NOT_FOUND = 2,
    NX_TOOL_IO_ERROR = 3,
    NX_TOOL_INTEGRITY_ERROR = 4,
    NX_TOOL_UNSUPPORTED = 5
} NxToolStatus;

typedef struct NxToolRecord {
    char id[64];
    char version[64];
    char executable[512];
    char sha256[65];
    char source_url[1024];
} NxToolRecord;

NxToolStatus NxToolManager_GetRecord(const char* root, const char* tool_id, NxToolRecord* out_record);
NxToolStatus NxToolManager_Verify(const char* root, const char* tool_id, NxToolRecord* out_record);
NxToolStatus NxToolManager_Register(const char* root, const NxToolRecord* record);
NxToolStatus NxToolManager_Remove(const char* root, const char* tool_id);
const char* NxToolManager_StatusName(NxToolStatus status);

#endif
