#include "Nexiora/Multimodal/NxDecoderAdapters.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define NX_DA_NULL_DEVICE "NUL"
#define NX_DA_WHERE_COMMAND "where"
#else
#define NX_DA_NULL_DEVICE "/dev/null"
#define NX_DA_WHERE_COMMAND "command -v"
#endif

static void nx_da_copy(char* dst, size_t size, const char* src) {
    size_t count;
    if (dst == NULL || size == 0U) return;
    if (src == NULL) { dst[0] = '\0'; return; }
    count = strlen(src);
    if (count >= size) count = size - 1U;
    if (count > 0U) memcpy(dst, src, count);
    dst[count] = '\0';
}

static int nx_da_safe_argument(const char* value) {
    const unsigned char* p = (const unsigned char*)value;
    if (value == NULL || value[0] == '\0') return 0;
    while (*p != 0U) {
        if (*p == '"' || *p == '\r' || *p == '\n' || *p == '&' || *p == '|' || *p == '<' || *p == '>') return 0;
        ++p;
    }
    return 1;
}

static const char* nx_da_extension(const char* path) {
    const char* dot = strrchr(path, '.');
    const char* slash = strrchr(path, '/');
    const char* back = strrchr(path, '\\');
    const char* separator = slash;
    if (separator == NULL || (back != NULL && back > separator)) separator = back;
    return (dot != NULL && (separator == NULL || dot > separator)) ? dot : "";
}

static int nx_da_equal_ci(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) return 0;
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

static int nx_da_replace_extension(const char* source, const char* suffix, char* out, size_t size) {
    const char* extension = nx_da_extension(source);
    size_t prefix = extension[0] == '\0' ? strlen(source) : (size_t)(extension - source);
    size_t suffix_size = strlen(suffix);
    if (prefix + suffix_size + 1U > size) return 0;
    if (prefix > 0U) memcpy(out, source, prefix);
    memcpy(out + prefix, suffix, suffix_size + 1U);
    return 1;
}

static int nx_da_command_exists(const char* name, char* path, size_t path_size) {
    char command[256];
    int written;
    int rc;
    written = snprintf(command, sizeof(command), "%s %s > %s 2>&1", NX_DA_WHERE_COMMAND, name, NX_DA_NULL_DEVICE);
    if (written < 0 || (size_t)written >= sizeof(command)) return 0;
    rc = system(command);
    if (rc != 0) return 0;
    nx_da_copy(path, path_size, name);
    return 1;
}

static int nx_da_file_exists(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) return 0;
    fclose(file);
    return 1;
}

const char* NxDecoder_StatusToString(NxDecoderStatus status) {
    switch (status) {
        case NX_DA_OK: return "OK";
        case NX_DA_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_DA_NOT_FOUND: return "NOT_FOUND";
        case NX_DA_TOOL_UNAVAILABLE: return "TOOL_UNAVAILABLE";
        case NX_DA_UNSUPPORTED: return "UNSUPPORTED";
        case NX_DA_COMMAND_FAILED: return "COMMAND_FAILED";
        case NX_DA_OUTPUT_MISSING: return "OUTPUT_MISSING";
        case NX_DA_OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
        default: return "UNKNOWN";
    }
}

const char* NxDecoder_OperationToString(NxDecoderOperation operation) {
    switch (operation) {
        case NX_DA_OPERATION_OCR_IMAGE: return "ocr-image";
        case NX_DA_OPERATION_TRANSCRIBE_AUDIO: return "transcribe-audio";
        case NX_DA_OPERATION_TRANSCRIBE_VIDEO: return "transcribe-video";
        case NX_DA_OPERATION_OCR_PDF: return "ocr-pdf";
        default: return "none";
    }
}

NxDecoderStatus NxDecoder_DiscoverTools(NxDecoderTools* tools_out) {
    if (tools_out == NULL) return NX_DA_INVALID_ARGUMENT;
    memset(tools_out, 0, sizeof(*tools_out));
    tools_out->tesseract_available = nx_da_command_exists("tesseract", tools_out->tesseract_path, sizeof(tools_out->tesseract_path));
    tools_out->ffmpeg_available = nx_da_command_exists("ffmpeg", tools_out->ffmpeg_path, sizeof(tools_out->ffmpeg_path));
    tools_out->whisper_available = nx_da_command_exists("whisper-cli", tools_out->whisper_path, sizeof(tools_out->whisper_path));
    if (!tools_out->whisper_available) {
        tools_out->whisper_available = nx_da_command_exists("whisper", tools_out->whisper_path, sizeof(tools_out->whisper_path));
    }
    return NX_DA_OK;
}

NxDecoderStatus NxDecoder_CreatePlan(const NxDecoderTools* tools, const char* source_path,
    const char* language, const char* whisper_model_path, NxDecoderPlan* plan_out) {
    const char* extension;
    int written;
    char output_base[NX_DA_MAX_PATH];
    if (tools == NULL || plan_out == NULL || !nx_da_safe_argument(source_path)) return NX_DA_INVALID_ARGUMENT;
    if (!nx_da_file_exists(source_path)) return NX_DA_NOT_FOUND;
    if (language == NULL || language[0] == '\0') language = "spa";
    if (!nx_da_safe_argument(language)) return NX_DA_INVALID_ARGUMENT;
    memset(plan_out, 0, sizeof(*plan_out));
    nx_da_copy(plan_out->source_path, sizeof(plan_out->source_path), source_path);
    if (!nx_da_replace_extension(source_path, ".txt", plan_out->output_text_path, sizeof(plan_out->output_text_path))) return NX_DA_OUTPUT_TOO_SMALL;
    extension = nx_da_extension(source_path);

    if (nx_da_equal_ci(extension, ".png") || nx_da_equal_ci(extension, ".jpg") || nx_da_equal_ci(extension, ".jpeg") || nx_da_equal_ci(extension, ".bmp") || nx_da_equal_ci(extension, ".tif") || nx_da_equal_ci(extension, ".tiff")) {
        if (!tools->tesseract_available) return NX_DA_TOOL_UNAVAILABLE;
        plan_out->operation = NX_DA_OPERATION_OCR_IMAGE;
        if (!nx_da_replace_extension(source_path, "", output_base, sizeof(output_base))) return NX_DA_OUTPUT_TOO_SMALL;
        written = snprintf(plan_out->command_primary, sizeof(plan_out->command_primary), "\"%s\" \"%s\" \"%s\" -l %s", tools->tesseract_path, source_path, output_base, language);
    } else if (nx_da_equal_ci(extension, ".wav") || nx_da_equal_ci(extension, ".mp3") || nx_da_equal_ci(extension, ".flac") || nx_da_equal_ci(extension, ".ogg") || nx_da_equal_ci(extension, ".m4a")) {
        if (!tools->whisper_available || !nx_da_safe_argument(whisper_model_path)) return NX_DA_TOOL_UNAVAILABLE;
        plan_out->operation = NX_DA_OPERATION_TRANSCRIBE_AUDIO;
        if (!nx_da_replace_extension(source_path, "", output_base, sizeof(output_base))) return NX_DA_OUTPUT_TOO_SMALL;
        written = snprintf(plan_out->command_primary, sizeof(plan_out->command_primary), "\"%s\" -m \"%s\" -f \"%s\" -otxt -of \"%s\"", tools->whisper_path, whisper_model_path, source_path, output_base);
    } else if (nx_da_equal_ci(extension, ".mp4") || nx_da_equal_ci(extension, ".mkv") || nx_da_equal_ci(extension, ".avi") || nx_da_equal_ci(extension, ".mov") || nx_da_equal_ci(extension, ".webm")) {
        if (!tools->ffmpeg_available || !tools->whisper_available || !nx_da_safe_argument(whisper_model_path)) return NX_DA_TOOL_UNAVAILABLE;
        plan_out->operation = NX_DA_OPERATION_TRANSCRIBE_VIDEO;
        if (!nx_da_replace_extension(source_path, ".nxdecode.wav", plan_out->temporary_audio_path, sizeof(plan_out->temporary_audio_path))) return NX_DA_OUTPUT_TOO_SMALL;
        written = snprintf(plan_out->command_primary, sizeof(plan_out->command_primary), "\"%s\" -y -i \"%s\" -vn -ar 16000 -ac 1 \"%s\"", tools->ffmpeg_path, source_path, plan_out->temporary_audio_path);
        if (written >= 0 && (size_t)written < sizeof(plan_out->command_primary)) {
            if (!nx_da_replace_extension(source_path, "", output_base, sizeof(output_base))) return NX_DA_OUTPUT_TOO_SMALL;
            written = snprintf(plan_out->command_secondary, sizeof(plan_out->command_secondary), "\"%s\" -m \"%s\" -f \"%s\" -otxt -of \"%s\"", tools->whisper_path, whisper_model_path, plan_out->temporary_audio_path, output_base);
        }
    } else if (nx_da_equal_ci(extension, ".pdf")) {
        plan_out->operation = NX_DA_OPERATION_OCR_PDF;
        nx_da_copy(plan_out->message, sizeof(plan_out->message), "PDF OCR requires a rasterizer adapter planned for Sprint-03.");
        return NX_DA_UNSUPPORTED;
    } else {
        return NX_DA_UNSUPPORTED;
    }
    if (written < 0 || (size_t)written >= sizeof(plan_out->command_primary)) return NX_DA_OUTPUT_TOO_SMALL;
    nx_da_copy(plan_out->message, sizeof(plan_out->message), "Decoder plan created; execution remains evidence-gated.");
    return NX_DA_OK;
}

NxDecoderStatus NxDecoder_ExecutePlan(const NxDecoderPlan* plan, NxDecoderResult* result_out) {
    int rc;
    if (plan == NULL || result_out == NULL || plan->command_primary[0] == '\0') return NX_DA_INVALID_ARGUMENT;
    memset(result_out, 0, sizeof(*result_out));
    result_out->operation = plan->operation;
    rc = system(plan->command_primary);
    result_out->commands_executed = 1;
    if (rc != 0) { result_out->status = NX_DA_COMMAND_FAILED; nx_da_copy(result_out->message, sizeof(result_out->message), "Primary decoder command failed."); return result_out->status; }
    if (plan->command_secondary[0] != '\0') {
        rc = system(plan->command_secondary);
        result_out->commands_executed = 2;
        (void)remove(plan->temporary_audio_path);
        if (rc != 0) { result_out->status = NX_DA_COMMAND_FAILED; nx_da_copy(result_out->message, sizeof(result_out->message), "Secondary decoder command failed."); return result_out->status; }
    }
    if (!nx_da_file_exists(plan->output_text_path)) { result_out->status = NX_DA_OUTPUT_MISSING; nx_da_copy(result_out->message, sizeof(result_out->message), "Decoder completed without producing textual evidence."); return result_out->status; }
    result_out->status = NX_DA_OK;
    nx_da_copy(result_out->output_text_path, sizeof(result_out->output_text_path), plan->output_text_path);
    nx_da_copy(result_out->message, sizeof(result_out->message), "Textual evidence generated successfully.");
    return result_out->status;
}
