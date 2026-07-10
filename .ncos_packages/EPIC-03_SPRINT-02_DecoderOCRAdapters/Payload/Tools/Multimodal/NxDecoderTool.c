#include "Nexiora/Multimodal/NxDecoderAdapters.h"
#include <stdio.h>
#include <string.h>

static void print_tools(const NxDecoderTools* tools) {
    printf("tesseract=%s\nffmpeg=%s\nwhisper=%s\n", tools->tesseract_available ? tools->tesseract_path : "missing", tools->ffmpeg_available ? tools->ffmpeg_path : "missing", tools->whisper_available ? tools->whisper_path : "missing");
}

int main(int argc, char** argv) {
    NxDecoderTools tools;
    NxDecoderPlan plan;
    NxDecoderResult result;
    NxDecoderStatus status;
    if (argc < 2) {
        fprintf(stderr, "Uso: nexiora_decode status | plan <archivo> [idioma] [modelo] | execute <archivo> [idioma] [modelo]\n");
        return 2;
    }
    status = NxDecoder_DiscoverTools(&tools);
    if (status != NX_DA_OK) return 1;
    if (strcmp(argv[1], "status") == 0) { print_tools(&tools); return 0; }
    if ((strcmp(argv[1], "plan") == 0 || strcmp(argv[1], "execute") == 0) && argc >= 3) {
        const char* language = argc >= 4 ? argv[3] : "spa";
        const char* model = argc >= 5 ? argv[4] : "models/ggml-base.bin";
        status = NxDecoder_CreatePlan(&tools, argv[2], language, model, &plan);
        printf("status=%s\noperation=%s\noutput=%s\nprimary=%s\nsecondary=%s\nmessage=%s\n", NxDecoder_StatusToString(status), NxDecoder_OperationToString(plan.operation), plan.output_text_path, plan.command_primary, plan.command_secondary, plan.message);
        if (status != NX_DA_OK) return 1;
        if (strcmp(argv[1], "plan") == 0) return 0;
        status = NxDecoder_ExecutePlan(&plan, &result);
        printf("execution=%s\ncommands=%d\nevidence=%s\nmessage=%s\n", NxDecoder_StatusToString(status), result.commands_executed, result.output_text_path, result.message);
        return status == NX_DA_OK ? 0 : 1;
    }
    fprintf(stderr, "Comando invalido.\n");
    return 2;
}
