#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/NCP/Logging/NxLog.h"
#include <stdio.h>

int main(void) {
    NxRuntime runtime;
    NxResult result = nx_runtime_initialize(&runtime, "nexiora.log");
    if (result != NX_OK) {
        fprintf(stderr, "Failed to initialize Nexiora: %s\n", nx_result_to_string(result));
        return 1;
    }

    nx_log_write(NX_LOG_INFO, "Bootstrap", "Welcome to Nexiora Genesis.");
    nx_runtime_print_status(&runtime);
    nx_runtime_shutdown(&runtime);
    return 0;
}
