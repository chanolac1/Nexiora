#include "Nexiora/NCP/Common/NxResult.h"

const char* nx_result_to_string(NxResult result) {
    switch (result) {
        case NX_OK: return "NX_OK";
        case NX_ERROR_ARGUMENT: return "NX_ERROR_ARGUMENT";
        case NX_ERROR_MEMORY: return "NX_ERROR_MEMORY";
        case NX_ERROR_IO: return "NX_ERROR_IO";
        case NX_ERROR_UNSUPPORTED: return "NX_ERROR_UNSUPPORTED";
        case NX_ERROR_INTERNAL: return "NX_ERROR_INTERNAL";
        default: return "NX_ERROR_UNKNOWN";
    }
}
