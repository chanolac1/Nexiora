#include "Nexiora/Tools/NxToolManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_record(NxToolStatus s, const NxToolRecord* r) {
    printf("status=%s\n", NxToolManager_StatusName(s));
    if (s == NX_TOOL_OK && r != NULL) {
        printf("id=%s\nversion=%s\nexecutable=%s\nsha256=%s\nsource_url=%s\n", r->id, r->version, r->executable, r->sha256, r->source_url);
    }
}

int main(int argc, char** argv) {
    NxToolRecord r; NxToolStatus s;
    if (argc < 4) {
        fprintf(stderr, "usage: nexiora_tool <status|verify|install|remove> <root> <tool-id>\n");
        return 2;
    }
    if (strcmp(argv[1], "status") == 0 || strcmp(argv[1], "verify") == 0) {
        s = NxToolManager_Verify(argv[2], argv[3], &r); print_record(s, &r); return s == NX_TOOL_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "remove") == 0) {
        s = NxToolManager_Remove(argv[2], argv[3]); print_record(s, NULL); return s == NX_TOOL_OK ? 0 : 1;
    }
    if (strcmp(argv[1], "install") == 0) {
        char command[2048]; int n;
        if (strpbrk(argv[2], "\"\r\n") != NULL || strpbrk(argv[3], "\"\r\n") != NULL) return 2;
        n = snprintf(command, sizeof(command), "powershell -ExecutionPolicy Bypass -File \"%s/Tools/ToolManager/install_tool.ps1\" -Root \"%s\" -ToolId \"%s\"", argv[2], argv[2], argv[3]);
        if (n < 0 || (size_t)n >= sizeof(command)) return 2;
        return system(command) == 0 ? 0 : 1;
    }
    fprintf(stderr, "unknown operation\n"); return 2;
}
