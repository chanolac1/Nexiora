#ifndef NEXIORA_CONTROL_CENTER_MODEL_H
#define NEXIORA_CONTROL_CENTER_MODEL_H

#include <stddef.h>

typedef enum NxCcAction {
    NX_CC_ACTION_DASHBOARD = 0,
    NX_CC_ACTION_INGEST_FILE,
    NX_CC_ACTION_WEB_LEARN,
    NX_CC_ACTION_TOOL_STATUS,
    NX_CC_ACTION_RUN_TESTS,
    NX_CC_ACTION_PACKAGE_HISTORY,
    NX_CC_ACTION_DOCS_VALIDATE
} NxCcAction;

int NxCc_BuildCommand(NxCcAction action,
                      const char* root,
                      const char* input,
                      const char* option,
                      char* output,
                      size_t output_size);
const char* NxCc_ActionTitle(NxCcAction action);
const char* NxCc_ActionDescription(NxCcAction action);

#endif
