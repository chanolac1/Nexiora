#include "NxControlCenterModel.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int nx_cc_valid(const char* value)
{
    return value != NULL && value[0] != '\0' && strchr(value, '"') == NULL && strchr(value, '\r') == NULL && strchr(value, '\n') == NULL;
}

static int nx_cc_format(char* output, size_t output_size, const char* format, ...)
{
    int written;
    va_list args;
    if (output == NULL || output_size == 0U || format == NULL) {
        return 0;
    }
    va_start(args, format);
    written = vsnprintf(output, output_size, format, args);
    va_end(args);
    return written >= 0 && (size_t)written < output_size;
}

const char* NxCc_ActionTitle(NxCcAction action)
{
    switch (action) {
        case NX_CC_ACTION_INGEST_FILE: return "Aprender de archivo";
        case NX_CC_ACTION_WEB_LEARN: return "Aprender de YouTube";
        case NX_CC_ACTION_TOOL_STATUS: return "Herramientas administradas";
        case NX_CC_ACTION_RUN_TESTS: return "Pruebas del sistema";
        case NX_CC_ACTION_PACKAGE_HISTORY: return "Historial de paquetes";
        case NX_CC_ACTION_DOCS_VALIDATE: return "Documentación";
        case NX_CC_ACTION_DASHBOARD:
        default: return "Panel principal";
    }
}

const char* NxCc_ActionDescription(NxCcAction action)
{
    switch (action) {
        case NX_CC_ACTION_INGEST_FILE: return "Ingiere un archivo y lo convierte en conocimiento verificable.";
        case NX_CC_ACTION_WEB_LEARN: return "Obtiene subtítulos, normaliza evidencia y activa el análisis cognitivo.";
        case NX_CC_ACTION_TOOL_STATUS: return "Verifica el estado de las herramientas administradas por Nexiora.";
        case NX_CC_ACTION_RUN_TESTS: return "Ejecuta la suite completa y muestra el progreso en tiempo real.";
        case NX_CC_ACTION_PACKAGE_HISTORY: return "Consulta las instalaciones y reparaciones registradas.";
        case NX_CC_ACTION_DOCS_VALIDATE: return "Comprueba que la documentación técnica esté sincronizada.";
        case NX_CC_ACTION_DASHBOARD:
        default: return "Centro unificado para operar y observar las capacidades de Nexiora.";
    }
}

int NxCc_BuildCommand(NxCcAction action,
                      const char* root,
                      const char* input,
                      const char* option,
                      char* output,
                      size_t output_size)
{
    const char* safe_option = (option != NULL && option[0] != '\0') ? option : "es";
    if (!nx_cc_valid(root)) {
        return 0;
    }
    output[0] = '\0';
    switch (action) {
        case NX_CC_ACTION_INGEST_FILE:
            if (!nx_cc_valid(input)) return 0;
            return nx_cc_format(output, output_size,
                "\"%s\\Build\\windows-msvc-release\\bin\\nexiora_ingest.exe\" ingest \"%s\" \"%s\" \"Control Center Import\"",
                root, root, input);
        case NX_CC_ACTION_WEB_LEARN:
            if (!nx_cc_valid(input) || !nx_cc_valid(safe_option)) return 0;
            return nx_cc_format(output, output_size,
                "\"%s\\Build\\windows-msvc-release\\bin\\nexiora_web_cognitive.exe\" learn \"%s\" \"%s\" %s control_center_web",
                root, root, input, safe_option);
        case NX_CC_ACTION_TOOL_STATUS:
            return nx_cc_format(output, output_size,
                "\"%s\\Build\\windows-msvc-release\\bin\\nexiora_tool.exe\" status \"%s\" yt-dlp",
                root, root, "");
        case NX_CC_ACTION_RUN_TESTS:
            return nx_cc_format(output, output_size,
                "ctest --test-dir \"%s\\Build\\windows-msvc-release\" --output-on-failure",
                root, "", "");
        case NX_CC_ACTION_PACKAGE_HISTORY:
            return nx_cc_format(output, output_size,
                "\"%s\\Build\\windows-msvc-release\\bin\\nexiora_package.exe\" history \"%s\"",
                root, (input != NULL) ? input : "", "");
        case NX_CC_ACTION_DOCS_VALIDATE:
            return nx_cc_format(output, output_size,
                "\"%s\\Build\\windows-msvc-release\\bin\\nexiora_docs.exe\" validate \"%s\"",
                root, root, "");
        case NX_CC_ACTION_DASHBOARD:
        default:
            return 0;
    }
}
