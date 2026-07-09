#include "Nexiora/NCOS/NxIntentPlanner.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_trim_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    while (*src && isspace((unsigned char)*src)) src++;
    size_t n = strlen(src);
    while (n > 0 && isspace((unsigned char)src[n - 1])) n--;
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void nx_lower_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    size_t j = 0;
    if (src == NULL) src = "";
    for (size_t i = 0; src[i] != '\0' && j + 1 < dst_size; ++i) {
        dst[j++] = (char)tolower((unsigned char)src[i]);
    }
    dst[j] = '\0';
}

static int nx_contains(const char* text, const char* needle)
{
    char a[512];
    char b[128];
    nx_lower_copy(a, sizeof(a), text);
    nx_lower_copy(b, sizeof(b), needle);
    return strstr(a, b) != NULL;
}

static const char* nx_after_keyword(const char* request, const char* keyword)
{
    if (request == NULL || keyword == NULL) return NULL;
    char lower_req[512];
    char lower_key[128];
    nx_lower_copy(lower_req, sizeof(lower_req), request);
    nx_lower_copy(lower_key, sizeof(lower_key), keyword);
    const char* p = strstr(lower_req, lower_key);
    if (p == NULL) return NULL;
    size_t offset = (size_t)(p - lower_req) + strlen(lower_key);
    return request + offset;
}

int NxIntentPlanner_Classify(const char* request, char* intent, int intent_size, char* target, int target_size)
{
    if (request == NULL || request[0] == '\0') return 0;
    const char* t = NULL;

    if (nx_contains(request, "videojuego") || nx_contains(request, "crear") || nx_contains(request, "construye") || nx_contains(request, "build") || nx_contains(request, "proyecto")) {
        nx_copy(intent, (size_t)intent_size, "build_project");
        t = nx_after_keyword(request, "crear");
        if (t == NULL) t = nx_after_keyword(request, "construye");
        if (t == NULL) t = nx_after_keyword(request, "build");
        if (t == NULL) t = request;
        nx_trim_copy(target, (size_t)target_size, t);
        return 1;
    }

    if (nx_contains(request, "aprende") || nx_contains(request, "investiga") || nx_contains(request, "estudia")) {
        nx_copy(intent, (size_t)intent_size, "learn_topic");
        t = nx_after_keyword(request, "aprende");
        if (t == NULL) t = nx_after_keyword(request, "investiga");
        if (t == NULL) t = nx_after_keyword(request, "estudia");
        if (t == NULL) t = request;
        nx_trim_copy(target, (size_t)target_size, t);
        return 1;
    }

    if (nx_contains(request, "analiza") || nx_contains(request, "revisa") || nx_contains(request, "repositorio")) {
        nx_copy(intent, (size_t)intent_size, "analyze_project");
        t = nx_after_keyword(request, "analiza");
        if (t == NULL) t = nx_after_keyword(request, "revisa");
        if (t == NULL) t = request;
        nx_trim_copy(target, (size_t)target_size, t);
        return 1;
    }

    nx_copy(intent, (size_t)intent_size, "general_goal");
    nx_trim_copy(target, (size_t)target_size, request);
    return 1;
}

static void nx_make_goal(const char* intent, const char* target, char* out, size_t out_size)
{
    if (strcmp(intent, "build_project") == 0) {
        (void)snprintf(out, out_size, "Construir proyecto: %s", target == NULL ? "" : target);
    } else if (strcmp(intent, "learn_topic") == 0) {
        (void)snprintf(out, out_size, "Aprender tema: %s", target == NULL ? "" : target);
    } else if (strcmp(intent, "analyze_project") == 0) {
        (void)snprintf(out, out_size, "Analizar proyecto: %s", target == NULL ? "" : target);
    } else {
        (void)snprintf(out, out_size, "Objetivo general: %s", target == NULL ? "" : target);
    }
    out[out_size - 1] = '\0';
}

int NxIntentPlanner_CreatePlan(const char* root, const char* request, NxIntentPlanResult* out)
{
    if (out != NULL) memset(out, 0, sizeof(*out));
    if (request == NULL || request[0] == '\0') return 0;

    NxIntentPlanResult local;
    memset(&local, 0, sizeof(local));

    if (!NxIntentPlanner_Classify(request, local.intent, sizeof(local.intent), local.target, sizeof(local.target))) return 0;
    nx_make_goal(local.intent, local.target, local.normalized_goal, sizeof(local.normalized_goal));

    if (!NxPlanning_Create(root, local.normalized_goal, &local.plan)) return 0;

    NxPlanStatus status;
    char note[512];
    (void)snprintf(note, sizeof(note), "Intento detectado: %s | Objetivo original: %s", local.intent, request);
    (void)NxPlanning_AddNote(root, note, &status);

    local.ok = 1;
    if (out != NULL) *out = local;
    return 1;
}
