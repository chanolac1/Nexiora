#include "NxControlCenterModel.h"

#include <stdio.h>
#include <string.h>

static int require_true(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "NxControlCenterTests: FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    char command[2048];
    if (!require_true(NxCc_BuildCommand(NX_CC_ACTION_WEB_LEARN, "D:\\Nexiora", "https://www.youtube.com/watch?v=abc123", "es", command, sizeof(command)), "web command")) return 1;
    if (!require_true(strstr(command, "nexiora_web_cognitive.exe") != NULL, "web executable")) return 1;
    if (!require_true(strstr(command, "D:\\Nexiora") != NULL, "absolute root")) return 1;
    if (!require_true(NxCc_BuildCommand(NX_CC_ACTION_RUN_TESTS, "D:\\Nexiora", "", "", command, sizeof(command)), "test command")) return 1;
    if (!require_true(strstr(command, "ctest --test-dir") != NULL, "ctest command")) return 1;
    if (!require_true(!NxCc_BuildCommand(NX_CC_ACTION_WEB_LEARN, "D:\\Nexiora", "bad\"url", "es", command, sizeof(command)), "reject unsafe input")) return 1;
    puts("NxControlCenterTests: PASS");
    return 0;
}
