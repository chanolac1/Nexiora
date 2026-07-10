#include "Nexiora/CognitivePipeline/NxWebCognitivePipeline.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <process.h>
#include <stdlib.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#endif

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


static int nx_append(char* dst, size_t dst_size, const char* src);

static int nx_absolute_root(const char* root, char* output, size_t output_size)
{
    if (root == NULL || output == NULL || output_size == 0U) { return 0; }
#if defined(_WIN32)
    return _fullpath(output, root, output_size) != NULL;
#else
    if (root[0] == '/') { return nx_copy(output, output_size, root); }
    if (getcwd(output, output_size) == NULL) { return 0; }
    if (strcmp(root, ".") == 0) { return 1; }
    return nx_append(output, output_size, "/") && nx_append(output, output_size, root);
#endif
}

static void nx_progress(unsigned int percent, const char* stage, unsigned long elapsed_seconds)
{
    if (stage == NULL) { return; }
    fprintf(stderr, "[NEXIORA][%3u%%][%lus] %s\n", percent, elapsed_seconds, stage);
    fflush(stderr);
}

static void nx_sleep_seconds(unsigned int seconds)
{
#if defined(_WIN32)
    Sleep(seconds * 1000U);
#else
    (void)sleep(seconds);
#endif
}

static int nx_run_process_observable(const char* executable, const char* const arguments[],
                                     unsigned int percent, const char* stage)
{
    unsigned long elapsed = 0UL;
    if (executable == NULL || arguments == NULL || stage == NULL) { return 0; }
    nx_progress(percent, stage, elapsed);
#if defined(_WIN32)
    {
        const intptr_t process_value = _spawnv(_P_NOWAIT, executable, arguments);
        HANDLE process_handle;
        DWORD exit_code = 1U;
        if (process_value == (intptr_t)-1) { return 0; }
        process_handle = (HANDLE)process_value;
        for (;;) {
            const DWORD wait_result = WaitForSingleObject(process_handle, 1000U);
            if (wait_result == WAIT_OBJECT_0) { break; }
            if (wait_result != WAIT_TIMEOUT) { CloseHandle(process_handle); return 0; }
            ++elapsed;
            if ((elapsed % 5UL) == 0UL) { nx_progress(percent, stage, elapsed); }
        }
        if (!GetExitCodeProcess(process_handle, &exit_code)) { CloseHandle(process_handle); return 0; }
        CloseHandle(process_handle);
        return exit_code == 0U;
    }
#else
    {
        pid_t child = fork();
        int status = 0;
        if (child < 0) { return 0; }
        if (child == 0) {
            execv(executable, (char* const*)arguments);
            _exit(127);
        }
        for (;;) {
            const pid_t result = waitpid(child, &status, WNOHANG);
            if (result == child) { break; }
            if (result < 0) { return 0; }
            (void)sleep(1U);
            ++elapsed;
            if ((elapsed % 5UL) == 0UL) { nx_progress(percent, stage, elapsed); }
        }
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
#endif
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
    char absolute_root[NX_WCP_MAX_PATH];
    int written;
    if (plan == NULL) { return NX_WCP_INVALID_ARGUMENT; }
    memset(plan, 0, sizeof(*plan));
    plan->status = NX_WCP_INVALID_ARGUMENT;
    if (root == NULL || url == NULL || language == NULL || root[0] == '\0' || url[0] == '\0' || !nx_safe_token(language)) {
        nx_copy(plan->message, sizeof(plan->message), "Root, URL and safe language token are required.");
        return plan->status;
    }
    if (!nx_absolute_root(root, absolute_root, sizeof(absolute_root))) {
        plan->status = NX_WCP_IO_ERROR;
        nx_copy(plan->message, sizeof(plan->message), "Unable to resolve Nexiora root to an absolute path.");
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
    if (!nx_prepare_dirs(absolute_root, plan->source_id, plan->workspace, sizeof(plan->workspace))) {
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
    nx_progress(10U, "Acquisition plan ready", 0UL);
    {
        char managed_tool[NX_WCP_MAX_PATH];
        if (!nx_join(managed_tool, sizeof(managed_tool), absolute_root, "Tools/Managed/yt-dlp/yt-dlp.exe")) {
            plan->status = NX_WCP_OUTPUT_TOO_SMALL; return plan->status;
        }
        char managed_deno[NX_WCP_MAX_PATH];
        if (!nx_join(managed_deno, sizeof(managed_deno), absolute_root, "Tools/Managed/deno/deno.exe")) {
            plan->status = NX_WCP_OUTPUT_TOO_SMALL; return plan->status;
        }
        written = snprintf(plan->download_command, sizeof(plan->download_command),
            "\"%s\" --skip-download --ignore-no-formats-error --write-auto-subs --write-subs --sub-langs \"%s\" --sub-format vtt --no-playlist --js-runtimes \"deno:%s\" --remote-components ejs:github --sleep-requests 2 --retries 5 --extractor-retries 3 --retry-sleep 10 -o \"%s\" \"%s\"",
            managed_tool, language, managed_deno, output_template, url);
    }
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

static int nx_has_vtt_suffix(const char* name)
{
    size_t length;
    if (name == NULL) { return 0; }
    length = strlen(name);
    if (length < 4U) { return 0; }
    return tolower((unsigned char)name[length - 4U]) == '.' &&
           tolower((unsigned char)name[length - 3U]) == 'v' &&
           tolower((unsigned char)name[length - 2U]) == 't' &&
           tolower((unsigned char)name[length - 1U]) == 't';
}

static int nx_transcript_score(const NxWcpPlan* plan, const char* name)
{
    char language_marker[48];
    const size_t source_length = strlen(plan->source_id);
    int written;
    if (strncmp(name, plan->source_id, source_length) != 0 || !nx_has_vtt_suffix(name)) { return -1; }
    written = snprintf(language_marker, sizeof(language_marker), ".%s.", plan->language);
    if (written < 0 || (size_t)written >= sizeof(language_marker)) { return 0; }
    if (strstr(name, language_marker) != NULL) { return 3; }
    written = snprintf(language_marker, sizeof(language_marker), ".%s.vtt", plan->language);
    if (written >= 0 && (size_t)written < sizeof(language_marker) && strstr(name, language_marker) != NULL) { return 2; }
    return 1;
}

NxWcpStatus NxWcp_FindTranscript(const NxWcpPlan* plan, char* found, size_t found_size)
{
    int best_score = -1;
    char best_name[NX_WCP_MAX_PATH] = {0};
    if (plan == NULL || found == NULL || found_size == 0U) { return NX_WCP_INVALID_ARGUMENT; }
    found[0] = '\0';
    if (nx_exists(plan->transcript_path)) {
        return nx_copy(found, found_size, plan->transcript_path) ? NX_WCP_OK : NX_WCP_OUTPUT_TOO_SMALL;
    }
#if defined(_WIN32)
    {
        WIN32_FIND_DATAA data;
        HANDLE handle;
        char pattern[NX_WCP_MAX_PATH];
        if (!nx_join(pattern, sizeof(pattern), plan->workspace, "*.vtt")) { return NX_WCP_OUTPUT_TOO_SMALL; }
        handle = FindFirstFileA(pattern, &data);
        if (handle == INVALID_HANDLE_VALUE) { return NX_WCP_SOURCE_NOT_FOUND; }
        do {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U) {
                const int score = nx_transcript_score(plan, data.cFileName);
                if (score > best_score) {
                    best_score = score;
                    if (!nx_copy(best_name, sizeof(best_name), data.cFileName)) { FindClose(handle); return NX_WCP_OUTPUT_TOO_SMALL; }
                }
            }
        } while (FindNextFileA(handle, &data));
        FindClose(handle);
    }
#else
    {
        DIR* directory = opendir(plan->workspace);
        struct dirent* entry;
        if (directory == NULL) { return NX_WCP_SOURCE_NOT_FOUND; }
        while ((entry = readdir(directory)) != NULL) {
            const int score = nx_transcript_score(plan, entry->d_name);
            if (score > best_score) {
                best_score = score;
                if (!nx_copy(best_name, sizeof(best_name), entry->d_name)) { closedir(directory); return NX_WCP_OUTPUT_TOO_SMALL; }
            }
        }
        closedir(directory);
    }
#endif
    if (best_score < 0) { return NX_WCP_SOURCE_NOT_FOUND; }
    return nx_join(found, found_size, plan->workspace, best_name) ? NX_WCP_OK : NX_WCP_OUTPUT_TOO_SMALL;
}

NxWcpStatus NxWcp_Execute(const char* root, const char* url, const char* language, const char* domain, NxWcpResult* result)
{
    NxWcpPlan plan;
    NxWcpStatus status;
    char transcript[NX_WCP_MAX_PATH];
    char absolute_root[NX_WCP_MAX_PATH];
    if (result == NULL) { return NX_WCP_INVALID_ARGUMENT; }
    memset(result, 0, sizeof(*result));
    if (!nx_absolute_root(root, absolute_root, sizeof(absolute_root))) {
        result->status = NX_WCP_IO_ERROR;
        nx_copy(result->message, sizeof(result->message), "Unable to resolve Nexiora root to an absolute path.");
        return result->status;
    }
    nx_progress(5U, "Resolving root and validating request", 0UL);
    status = NxWcp_BuildPlan(absolute_root, url, language, &plan);
    if (status != NX_WCP_OK) { result->status = status; nx_copy(result->message, sizeof(result->message), plan.message); return status; }
    {
        char managed_tool[NX_WCP_MAX_PATH];
        char managed_deno[NX_WCP_MAX_PATH];
        if (!nx_join(managed_tool, sizeof(managed_tool), absolute_root, "Tools/Managed/yt-dlp/yt-dlp.exe") || !nx_exists(managed_tool)) {
            result->status = NX_WCP_TOOL_NOT_FOUND;
            nx_copy(result->message, sizeof(result->message), "Managed yt-dlp is not installed.");
            return result->status;
        }
        if (!nx_join(managed_deno, sizeof(managed_deno), absolute_root, "Tools/Managed/deno/deno.exe") || !nx_exists(managed_deno)) {
            result->status = NX_WCP_TOOL_NOT_FOUND;
            nx_copy(result->message, sizeof(result->message), "Managed Deno JavaScript runtime is not installed.");
            return result->status;
        }
    }
    {
        char managed_tool[NX_WCP_MAX_PATH];
        char managed_deno[NX_WCP_MAX_PATH];
        char js_runtime[NX_WCP_MAX_PATH + 16U];
        char output_template[NX_WCP_MAX_PATH];
        const char* cookie_browser = getenv("NEXIORA_YTDLP_COOKIES_BROWSER");
        const char* arguments[32];
        unsigned int attempt;
        int acquired = 0;
        size_t argument_count = 0U;
        if (!nx_join(managed_tool, sizeof(managed_tool), absolute_root, "Tools/Managed/yt-dlp/yt-dlp.exe") ||
            !nx_join(managed_deno, sizeof(managed_deno), absolute_root, "Tools/Managed/deno/deno.exe")) {
            result->status = NX_WCP_OUTPUT_TOO_SMALL;
            return result->status;
        }
        if (snprintf(js_runtime, sizeof(js_runtime), "deno:%s", managed_deno) < 0 ||
            !nx_join(output_template, sizeof(output_template), plan.workspace, "%(id)s.%(language)s.%(ext)s")) {
            result->status = NX_WCP_OUTPUT_TOO_SMALL;
            return result->status;
        }
        arguments[argument_count++] = managed_tool;
        arguments[argument_count++] = "--skip-download";
        arguments[argument_count++] = "--ignore-no-formats-error";
        arguments[argument_count++] = "--write-auto-subs";
        arguments[argument_count++] = "--write-subs";
        arguments[argument_count++] = "--sub-langs";
        arguments[argument_count++] = language;
        arguments[argument_count++] = "--sub-format";
        arguments[argument_count++] = "vtt";
        arguments[argument_count++] = "--no-playlist";
        arguments[argument_count++] = "--js-runtimes";
        arguments[argument_count++] = js_runtime;
        arguments[argument_count++] = "--remote-components";
        arguments[argument_count++] = "ejs:github";
        arguments[argument_count++] = "--sleep-requests";
        arguments[argument_count++] = "2";
        arguments[argument_count++] = "--retries";
        arguments[argument_count++] = "5";
        arguments[argument_count++] = "--extractor-retries";
        arguments[argument_count++] = "3";
        arguments[argument_count++] = "--retry-sleep";
        arguments[argument_count++] = "10";
        if (cookie_browser != NULL && nx_safe_token(cookie_browser)) {
            arguments[argument_count++] = "--cookies-from-browser";
            arguments[argument_count++] = cookie_browser;
        }
        arguments[argument_count++] = "-o";
        arguments[argument_count++] = output_template;
        arguments[argument_count++] = url;
        arguments[argument_count] = NULL;
        for (attempt = 1U; attempt <= 3U; ++attempt) {
            char stage[160];
            const int written = snprintf(stage, sizeof(stage), "Acquiring YouTube subtitles (attempt %u/3)", attempt);
            if (written < 0 || (size_t)written >= sizeof(stage)) { break; }
            if (nx_run_process_observable(managed_tool, arguments, 25U, stage)) { acquired = 1; break; }
            if (attempt < 3U) {
                const unsigned int delay = attempt * 20U;
                nx_progress(25U, "Acquisition failed; applying rate-limit backoff", delay);
                nx_sleep_seconds(delay);
            }
        }
        if (!acquired) {
            result->status = NX_WCP_COMMAND_FAILED;
            nx_copy(result->message, sizeof(result->message), "Subtitle acquisition failed after retries. YouTube may be rate-limiting this IP (HTTP 429). Open YouTube in a browser, complete any CAPTCHA, then optionally set NEXIORA_YTDLP_COOKIES_BROWSER=edge or chrome and retry.");
            return result->status;
        }
    }
    nx_progress(55U, "Subtitle acquisition process completed; locating evidence", 0UL);
    if (NxWcp_FindTranscript(&plan, transcript, sizeof(transcript)) != NX_WCP_OK) {
        result->status = NX_WCP_SOURCE_NOT_FOUND;
        nx_copy(result->message, sizeof(result->message), "No requested subtitle track was generated by yt-dlp.");
        return result->status;
    }
    nx_progress(65U, "Normalizing WebVTT transcript into nxknowledge/1", 0UL);
    status = NxWcp_ConvertWebVtt(transcript, url, plan.source_id, plan.knowledge_path);
    if (status != NX_WCP_OK) { result->status = status; nx_copy(result->message, sizeof(result->message), "Subtitle conversion did not produce usable evidence."); return status; }
    {
        char cognitive_tool[NX_WCP_MAX_PATH];
        const char* arguments[6];
        if (!nx_join(cognitive_tool, sizeof(cognitive_tool), absolute_root, "Build/windows-msvc-release/bin/nexiora_multimodal_cognitive.exe")) {
            result->status = NX_WCP_OUTPUT_TOO_SMALL;
            return result->status;
        }
        arguments[0] = cognitive_tool;
        arguments[1] = "analyze";
        arguments[2] = absolute_root;
        arguments[3] = plan.knowledge_path;
        arguments[4] = domain == NULL ? "web" : domain;
        arguments[5] = NULL;
        if (!nx_run_process_observable(cognitive_tool, arguments, 80U, "Integrating evidence into the cognitive engine")) {
            result->status = NX_WCP_COMMAND_FAILED;
            nx_copy(result->message, sizeof(result->message), "Cognitive analysis command failed after transcript acquisition.");
            return result->status;
        }
    }
    nx_progress(95U, "Persisting final result metadata", 0UL);
    result->status = NX_WCP_OK;
    nx_copy(result->source_id, sizeof(result->source_id), plan.source_id);
    nx_copy(result->transcript_path, sizeof(result->transcript_path), transcript);
    nx_copy(result->knowledge_path, sizeof(result->knowledge_path), plan.knowledge_path);
    nx_copy(result->analysis_id, sizeof(result->analysis_id), plan.source_id);
    nx_copy(result->message, sizeof(result->message), "YouTube transcript acquired, normalized and sent to cognitive integration.");
    nx_progress(100U, "Web cognitive learning completed", 0UL);
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
