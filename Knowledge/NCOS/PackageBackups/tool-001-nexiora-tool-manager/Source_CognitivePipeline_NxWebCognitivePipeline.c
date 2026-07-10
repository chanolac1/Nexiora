#include "Nexiora/CognitivePipeline/NxWebCognitivePipeline.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_WCP_MKDIR(path) _mkdir(path)
#define NX_WCP_ACCESS _access
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#define NX_WCP_MKDIR(path) mkdir((path), 0777)
#define NX_WCP_ACCESS access
#endif

static int nx_copy(char* dst, size_t dst_size, const char* src)
{
    size_t length;
    if (dst == NULL || dst_size == 0U || src == NULL) { return 0; }
    length = strlen(src);
    if (length >= dst_size) { dst[0] = '\0'; return 0; }
    memcpy(dst, src, length + 1U);
    return 1;
}

static int nx_append(char* dst, size_t dst_size, const char* src)
{
    size_t left;
    size_t right;
    if (dst == NULL || src == NULL || dst_size == 0U) { return 0; }
    left = strlen(dst);
    right = strlen(src);
    if (left >= dst_size || right >= dst_size - left) { return 0; }
    memcpy(dst + left, src, right + 1U);
    return 1;
}

static int nx_join(char* dst, size_t dst_size, const char* left, const char* right)
{
    char separator[2] = {'/', '\0'};
    size_t length;
    if (dst == NULL || left == NULL || right == NULL || dst_size == 0U) { return 0; }
    dst[0] = '\0';
    if (!nx_append(dst, dst_size, left)) { return 0; }
    length = strlen(dst);
    if (length > 0U && dst[length - 1U] != '/' && dst[length - 1U] != '\\') {
        if (!nx_append(dst, dst_size, separator)) { return 0; }
    }
    return nx_append(dst, dst_size, right);
}

static int nx_exists(const char* path)
{
    return path != NULL && NX_WCP_ACCESS(path, 0) == 0;
}

static int nx_mkdir_if_missing(const char* path)
{
    if (nx_exists(path)) { return 1; }
    return NX_WCP_MKDIR(path) == 0;
}

static int nx_safe_token(const char* text)
{
    size_t index;
    if (text == NULL || text[0] == '\0') { return 0; }
    for (index = 0U; text[index] != '\0'; ++index) {
        const unsigned char ch = (unsigned char)text[index];
        if (!(isalnum(ch) || ch == '-' || ch == '_')) { return 0; }
    }
    return 1;
}

static int nx_extract_youtube_id(const char* url, char* output, size_t output_size)
{
    const char* start = NULL;
    const char* marker;
    size_t length = 0U;
    if (url == NULL || output == NULL || output_size == 0U) { return 0; }
    marker = strstr(url, "youtu.be/");
    if (marker != NULL) { start = marker + 9; }
    if (start == NULL) {
        marker = strstr(url, "youtube.com/watch?");
        if (marker != NULL) {
            marker = strstr(marker, "v=");
            if (marker != NULL) { start = marker + 2; }
        }
    }
    if (start == NULL) {
        marker = strstr(url, "youtube.com/shorts/");
        if (marker != NULL) { start = marker + 19; }
    }
    if (start == NULL) { return 0; }
    while (start[length] != '\0' && start[length] != '&' && start[length] != '?' && start[length] != '#' && start[length] != '/') {
        if (!(isalnum((unsigned char)start[length]) || start[length] == '-' || start[length] == '_')) { return 0; }
        ++length;
    }
    if (length == 0U || length >= output_size) { return 0; }
    memcpy(output, start, length);
    output[length] = '\0';
    return 1;
}

int NxWcp_IsYouTubeUrl(const char* url)
{
    char id[128];
    return nx_extract_youtube_id(url, id, sizeof(id));
}

static int nx_prepare_dirs(const char* root, const char* source_id, char* workspace, size_t workspace_size)
{
    char knowledge[NX_WCP_MAX_PATH];
    char web[NX_WCP_MAX_PATH];
    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) { return 0; }
    if (!nx_mkdir_if_missing(knowledge)) { return 0; }
    if (!nx_join(web, sizeof(web), knowledge, "WebCognitive")) { return 0; }
    if (!nx_mkdir_if_missing(web)) { return 0; }
    if (!nx_join(workspace, workspace_size, web, source_id)) { return 0; }
    return nx_mkdir_if_missing(workspace);
}

NxWcpStatus NxWcp_BuildPlan(const char* root, const char* url, const char* language, NxWcpPlan* plan)
{
    char transcript_name[192];
    char knowledge_name[192];
    char output_template[NX_WCP_MAX_PATH];
    int written;
    if (plan == NULL) { return NX_WCP_INVALID_ARGUMENT; }
    memset(plan, 0, sizeof(*plan));
    plan->status = NX_WCP_INVALID_ARGUMENT;
    if (root == NULL || url == NULL || language == NULL || root[0] == '\0' || url[0] == '\0' || !nx_safe_token(language)) {
        nx_copy(plan->message, sizeof(plan->message), "Root, URL and safe language token are required.");
        return plan->status;
    }
    if (!nx_extract_youtube_id(url, plan->source_id, sizeof(plan->source_id))) {
        plan->status = NX_WCP_UNSUPPORTED_URL;
        nx_copy(plan->url, sizeof(plan->url), url);
        nx_copy(plan->message, sizeof(plan->message), "Only canonical YouTube watch, shorts and youtu.be URLs are supported in this sprint.");
        return plan->status;
    }
    if (!nx_copy(plan->url, sizeof(plan->url), url) || !nx_copy(plan->language, sizeof(plan->language), language)) {
        plan->status = NX_WCP_OUTPUT_TOO_SMALL;
        return plan->status;
    }
    if (!nx_prepare_dirs(root, plan->source_id, plan->workspace, sizeof(plan->workspace))) {
        plan->status = NX_WCP_IO_ERROR;
        nx_copy(plan->message, sizeof(plan->message), "Unable to create deterministic web cognitive workspace.");
        return plan->status;
    }
    written = snprintf(transcript_name, sizeof(transcript_name), "%s.%s.vtt", plan->source_id, language);
    if (written < 0 || (size_t)written >= sizeof(transcript_name) || !nx_join(plan->transcript_path, sizeof(plan->transcript_path), plan->workspace, transcript_name)) {
        plan->status = NX_WCP_OUTPUT_TOO_SMALL; return plan->status;
    }
    written = snprintf(knowledge_name, sizeof(knowledge_name), "%s.nxknowledge", plan->source_id);
    if (written < 0 || (size_t)written >= sizeof(knowledge_name) || !nx_join(plan->knowledge_path, sizeof(plan->knowledge_path), plan->workspace, knowledge_name)) {
        plan->status = NX_WCP_OUTPUT_TOO_SMALL; return plan->status;
    }
    if (!nx_join(output_template, sizeof(output_template), plan->workspace, "%(id)s.%(language)s.%(ext)s")) {
        plan->status = NX_WCP_OUTPUT_TOO_SMALL; return plan->status;
    }
    written = snprintf(plan->download_command, sizeof(plan->download_command),
        "yt-dlp --skip-download --write-auto-subs --write-subs --sub-langs \"%s\" --sub-format vtt --no-playlist -o \"%s\" \"%s\"",
        language, output_template, url);
    if (written < 0 || (size_t)written >= sizeof(plan->download_command)) {
        plan->status = NX_WCP_OUTPUT_TOO_SMALL; return plan->status;
    }
    plan->status = NX_WCP_OK;
    nx_copy(plan->message, sizeof(plan->message), "Auditable YouTube transcript acquisition plan created.");
    return plan->status;
}

static int nx_is_timing_line(const char* line)
{
    return line != NULL && strstr(line, "-->") != NULL;
}

static void nx_trim(char* line)
{
    size_t length;
    size_t start = 0U;
    if (line == NULL) { return; }
    length = strlen(line);
    while (start < length && isspace((unsigned char)line[start])) { ++start; }
    while (length > start && isspace((unsigned char)line[length - 1U])) { --length; }
    if (start > 0U) { memmove(line, line + start, length - start); }
    line[length - start] = '\0';
}

static void nx_remove_tags(char* line)
{
    size_t read_index = 0U;
    size_t write_index = 0U;
    int inside = 0;
    while (line[read_index] != '\0') {
        if (line[read_index] == '<') { inside = 1; ++read_index; continue; }
        if (line[read_index] == '>') { inside = 0; ++read_index; continue; }
        if (!inside) { line[write_index++] = line[read_index]; }
        ++read_index;
    }
    line[write_index] = '\0';
}

NxWcpStatus NxWcp_ConvertWebVtt(const char* transcript_path, const char* source_url, const char* title, const char* knowledge_path)
{
    FILE* input;
    FILE* output;
    char line[2048];
    char previous[2048] = {0};
    unsigned int evidence_lines = 0U;
    if (transcript_path == NULL || source_url == NULL || title == NULL || knowledge_path == NULL) { return NX_WCP_INVALID_ARGUMENT; }
    input = fopen(transcript_path, "rb");
    if (input == NULL) { return NX_WCP_SOURCE_NOT_FOUND; }
    output = fopen(knowledge_path, "wb");
    if (output == NULL) { fclose(input); return NX_WCP_IO_ERROR; }
    fprintf(output, "nxknowledge/1\nsource_type=web-video\nsource_url=%s\ntitle=%s\ncontent:\n", source_url, title);
    while (fgets(line, sizeof(line), input) != NULL) {
        nx_trim(line);
        if (line[0] == '\0' || strcmp(line, "WEBVTT") == 0 || nx_is_timing_line(line) || isdigit((unsigned char)line[0])) { continue; }
        nx_remove_tags(line);
        nx_trim(line);
        if (line[0] == '\0' || strcmp(line, previous) == 0) { continue; }
        fprintf(output, "%s\n", line);
        nx_copy(previous, sizeof(previous), line);
        ++evidence_lines;
    }
    if (fclose(input) != 0 || fclose(output) != 0) { return NX_WCP_IO_ERROR; }
    if (evidence_lines == 0U) { remove(knowledge_path); return NX_WCP_FORMAT_ERROR; }
    return NX_WCP_OK;
}

static int nx_find_transcript(const NxWcpPlan* plan, char* found, size_t found_size)
{
    char alternative[NX_WCP_MAX_PATH];
    char name[192];
    int written;
    if (nx_exists(plan->transcript_path)) { return nx_copy(found, found_size, plan->transcript_path); }
    written = snprintf(name, sizeof(name), "%s.%s-orig.vtt", plan->source_id, plan->language);
    if (written < 0 || (size_t)written >= sizeof(name) || !nx_join(alternative, sizeof(alternative), plan->workspace, name)) { return 0; }
    if (nx_exists(alternative)) { return nx_copy(found, found_size, alternative); }
    return 0;
}

NxWcpStatus NxWcp_Execute(const char* root, const char* url, const char* language, const char* domain, NxWcpResult* result)
{
    NxWcpPlan plan;
    NxWcpStatus status;
    char transcript[NX_WCP_MAX_PATH];
    char command[NX_WCP_MAX_COMMAND];
    int written;
    if (result == NULL) { return NX_WCP_INVALID_ARGUMENT; }
    memset(result, 0, sizeof(*result));
    status = NxWcp_BuildPlan(root, url, language, &plan);
    if (status != NX_WCP_OK) { result->status = status; nx_copy(result->message, sizeof(result->message), plan.message); return status; }
    if (system("yt-dlp --version >nul 2>&1") != 0) {
        result->status = NX_WCP_TOOL_NOT_FOUND;
        nx_copy(result->message, sizeof(result->message), "yt-dlp is required in PATH for live YouTube transcript acquisition.");
        return result->status;
    }
    if (system(plan.download_command) != 0) {
        result->status = NX_WCP_COMMAND_FAILED;
        nx_copy(result->message, sizeof(result->message), "yt-dlp failed to acquire subtitles or automatic captions.");
        return result->status;
    }
    if (!nx_find_transcript(&plan, transcript, sizeof(transcript))) {
        result->status = NX_WCP_SOURCE_NOT_FOUND;
        nx_copy(result->message, sizeof(result->message), "No requested subtitle track was generated by yt-dlp.");
        return result->status;
    }
    status = NxWcp_ConvertWebVtt(transcript, url, plan.source_id, plan.knowledge_path);
    if (status != NX_WCP_OK) { result->status = status; nx_copy(result->message, sizeof(result->message), "Subtitle conversion did not produce usable evidence."); return status; }
    written = snprintf(command, sizeof(command), "\"%s/Build/windows-msvc-release/bin/nexiora_multimodal_cognitive.exe\" analyze \"%s\" \"%s\" \"%s\"",
                       root, root, plan.knowledge_path, domain == NULL ? "web" : domain);
    if (written < 0 || (size_t)written >= sizeof(command)) { result->status = NX_WCP_OUTPUT_TOO_SMALL; return result->status; }
    if (system(command) != 0) { result->status = NX_WCP_COMMAND_FAILED; nx_copy(result->message, sizeof(result->message), "Cognitive analysis command failed after transcript acquisition."); return result->status; }
    result->status = NX_WCP_OK;
    nx_copy(result->source_id, sizeof(result->source_id), plan.source_id);
    nx_copy(result->transcript_path, sizeof(result->transcript_path), transcript);
    nx_copy(result->knowledge_path, sizeof(result->knowledge_path), plan.knowledge_path);
    nx_copy(result->analysis_id, sizeof(result->analysis_id), plan.source_id);
    nx_copy(result->message, sizeof(result->message), "YouTube transcript acquired, normalized and sent to cognitive integration.");
    return result->status;
}

const char* NxWcp_StatusToString(NxWcpStatus status)
{
    switch (status) {
        case NX_WCP_OK: return "OK";
        case NX_WCP_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_WCP_UNSUPPORTED_URL: return "UNSUPPORTED_URL";
        case NX_WCP_TOOL_NOT_FOUND: return "TOOL_NOT_FOUND";
        case NX_WCP_SOURCE_NOT_FOUND: return "SOURCE_NOT_FOUND";
        case NX_WCP_IO_ERROR: return "IO_ERROR";
        case NX_WCP_FORMAT_ERROR: return "FORMAT_ERROR";
        case NX_WCP_COMMAND_FAILED: return "COMMAND_FAILED";
        case NX_WCP_OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
        default: return "UNKNOWN";
    }
}
