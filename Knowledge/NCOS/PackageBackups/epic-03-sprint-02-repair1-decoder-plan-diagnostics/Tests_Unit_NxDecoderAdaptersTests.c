#include "Nexiora/Multimodal/NxDecoderAdapters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check(int condition, const char* message) {
    if (condition) return 1;
    fprintf(stderr, "NxDecoderAdaptersTests: FAIL: %s\n", message);
    return 0;
}

int main(void) {
    const char* source = "nx_decoder_test.png";
    FILE* file = fopen(source, "wb");
    NxDecoderTools tools;
    NxDecoderPlan plan;
    NxDecoderStatus status;
    if (!check(file != NULL, "cannot create fixture")) return 1;
    if (fputs("fixture", file) == EOF || fclose(file) != 0) return 1;
    memset(&tools, 0, sizeof(tools));
    tools.tesseract_available = 1;
    (void)snprintf(tools.tesseract_path, sizeof(tools.tesseract_path), "%s", "tesseract");
    status = NxDecoder_CreatePlan(&tools, source, "spa", "model.bin", &plan);
    if (!check(status == NX_DA_OK, "image OCR plan was not created")) return 1;
    if (!check(plan.operation == NX_DA_OPERATION_OCR_IMAGE, "wrong image operation")) return 1;
    if (!check(strcmp(plan.output_text_path, "nx_decoder_test.txt") == 0, "wrong text output path")) return 1;
    if (!check(strstr(plan.command_primary, "tesseract") != NULL, "OCR command missing tool")) return 1;
    tools.tesseract_available = 0;
    status = NxDecoder_CreatePlan(&tools, source, "spa", "model.bin", &plan);
    if (!check(status == NX_DA_TOOL_UNAVAILABLE, "missing OCR tool was not rejected")) return 1;
    status = NxDecoder_CreatePlan(&tools, "bad&path.png", "spa", "model.bin", &plan);
    if (!check(status == NX_DA_INVALID_ARGUMENT, "unsafe path was not rejected")) return 1;
    (void)remove(source);
    puts("NxDecoderAdaptersTests: PASS");
    return 0;
}
