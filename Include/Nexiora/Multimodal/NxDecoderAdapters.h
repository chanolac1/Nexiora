#ifndef NEXIORA_MULTIMODAL_NX_DECODER_ADAPTERS_H
#define NEXIORA_MULTIMODAL_NX_DECODER_ADAPTERS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_DA_MAX_PATH 512
#define NX_DA_MAX_COMMAND 2048
#define NX_DA_MAX_MESSAGE 256

typedef enum NxDecoderStatus {
    NX_DA_OK = 0,
    NX_DA_INVALID_ARGUMENT = 1,
    NX_DA_NOT_FOUND = 2,
    NX_DA_TOOL_UNAVAILABLE = 3,
    NX_DA_UNSUPPORTED = 4,
    NX_DA_COMMAND_FAILED = 5,
    NX_DA_OUTPUT_MISSING = 6,
    NX_DA_OUTPUT_TOO_SMALL = 7
} NxDecoderStatus;

typedef enum NxDecoderOperation {
    NX_DA_OPERATION_NONE = 0,
    NX_DA_OPERATION_OCR_IMAGE = 1,
    NX_DA_OPERATION_TRANSCRIBE_AUDIO = 2,
    NX_DA_OPERATION_TRANSCRIBE_VIDEO = 3,
    NX_DA_OPERATION_OCR_PDF = 4
} NxDecoderOperation;

typedef struct NxDecoderTools {
    int tesseract_available;
    int ffmpeg_available;
    int whisper_available;
    char tesseract_path[NX_DA_MAX_PATH];
    char ffmpeg_path[NX_DA_MAX_PATH];
    char whisper_path[NX_DA_MAX_PATH];
} NxDecoderTools;

typedef struct NxDecoderPlan {
    NxDecoderOperation operation;
    char source_path[NX_DA_MAX_PATH];
    char output_text_path[NX_DA_MAX_PATH];
    char temporary_audio_path[NX_DA_MAX_PATH];
    char command_primary[NX_DA_MAX_COMMAND];
    char command_secondary[NX_DA_MAX_COMMAND];
    char message[NX_DA_MAX_MESSAGE];
} NxDecoderPlan;

typedef struct NxDecoderResult {
    NxDecoderStatus status;
    NxDecoderOperation operation;
    int commands_executed;
    char output_text_path[NX_DA_MAX_PATH];
    char message[NX_DA_MAX_MESSAGE];
} NxDecoderResult;

const char* NxDecoder_StatusToString(NxDecoderStatus status);
const char* NxDecoder_OperationToString(NxDecoderOperation operation);
NxDecoderStatus NxDecoder_DiscoverTools(NxDecoderTools* tools_out);
NxDecoderStatus NxDecoder_CreatePlan(const NxDecoderTools* tools, const char* source_path,
    const char* language, const char* whisper_model_path, NxDecoderPlan* plan_out);
NxDecoderStatus NxDecoder_ExecutePlan(const NxDecoderPlan* plan, NxDecoderResult* result_out);

#ifdef __cplusplus
}
#endif
#endif
