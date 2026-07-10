#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NxControlCenterModel.h"

#define IDC_NAV_BASE 1000
#define IDC_INPUT 1100
#define IDC_OPTION 1101
#define IDC_RUN 1102
#define IDC_BROWSE 1103
#define IDC_LOG 1104
#define IDC_PROGRESS 1105
#define IDC_TITLE 1106
#define IDC_DESC 1107
#define WM_NX_OUTPUT (WM_APP + 1)
#define WM_NX_DONE (WM_APP + 2)

static HWND g_main;
static HWND g_input;
static HWND g_option;
static HWND g_log;
static HWND g_progress;
static HWND g_title;
static HWND g_desc;
static HWND g_run;
static NxCcAction g_action = NX_CC_ACTION_DASHBOARD;
static WCHAR g_root[MAX_PATH];

static void nx_append_log(const WCHAR* text)
{
    LRESULT length = SendMessageW(g_log, WM_GETTEXTLENGTH, 0, 0);
    SendMessageW(g_log, EM_SETSEL, (WPARAM)length, (LPARAM)length);
    SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)text);
}

static void nx_set_action(NxCcAction action)
{
    WCHAR title[128];
    WCHAR desc[320];
    g_action = action;
    MultiByteToWideChar(CP_UTF8, 0, NxCc_ActionTitle(action), -1, title, 128);
    MultiByteToWideChar(CP_UTF8, 0, NxCc_ActionDescription(action), -1, desc, 320);
    SetWindowTextW(g_title, title);
    SetWindowTextW(g_desc, desc);
    EnableWindow(g_input, action == NX_CC_ACTION_INGEST_FILE || action == NX_CC_ACTION_WEB_LEARN || action == NX_CC_ACTION_PACKAGE_HISTORY);
    EnableWindow(g_option, action == NX_CC_ACTION_WEB_LEARN);
}

static char* nx_utf8_from_wide(const WCHAR* value)
{
    int required;
    char* result;
    if (value == NULL) return NULL;
    required = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
    if (required <= 0) return NULL;
    result = (char*)calloc((size_t)required, 1U);
    if (result == NULL) return NULL;
    if (WideCharToMultiByte(CP_UTF8, 0, value, -1, result, required, NULL, NULL) <= 0) {
        free(result);
        return NULL;
    }
    return result;
}

static WCHAR* nx_wide_from_utf8(const char* value)
{
    int required;
    WCHAR* result;
    if (value == NULL) return NULL;
    required = MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
    if (required <= 0) return NULL;
    result = (WCHAR*)calloc((size_t)required, sizeof(WCHAR));
    if (result == NULL) return NULL;
    if (MultiByteToWideChar(CP_UTF8, 0, value, -1, result, required) <= 0) {
        free(result);
        return NULL;
    }
    return result;
}

static DWORD WINAPI nx_worker(LPVOID parameter)
{
    char* command = (char*)parameter;
    WCHAR* wide_command = nx_wide_from_utf8(command);
    SECURITY_ATTRIBUTES security;
    HANDLE read_pipe = NULL;
    HANDLE write_pipe = NULL;
    STARTUPINFOW startup;
    PROCESS_INFORMATION process;
    char buffer[1024];
    DWORD read_count;
    DWORD exit_code = 1U;
    free(command);
    if (wide_command == NULL) {
        PostMessageW(g_main, WM_NX_DONE, 1U, 0);
        return 1U;
    }
    ZeroMemory(&security, sizeof(security));
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;
    if (!CreatePipe(&read_pipe, &write_pipe, &security, 0U)) {
        free(wide_command);
        PostMessageW(g_main, WM_NX_DONE, 1U, 0);
        return 1U;
    }
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0U);
    ZeroMemory(&startup, sizeof(startup));
    ZeroMemory(&process, sizeof(process));
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup.hStdOutput = write_pipe;
    startup.hStdError = write_pipe;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.wShowWindow = SW_HIDE;
    if (!CreateProcessW(NULL, wide_command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, g_root, &startup, &process)) {
        const WCHAR* failure = L"\r\n[NEXIORA] No se pudo iniciar el proceso.\r\n";
        WCHAR* copy = _wcsdup(failure);
        PostMessageW(g_main, WM_NX_OUTPUT, 0, (LPARAM)copy);
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        free(wide_command);
        PostMessageW(g_main, WM_NX_DONE, 1U, 0);
        return 1U;
    }
    CloseHandle(write_pipe);
    while (ReadFile(read_pipe, buffer, sizeof(buffer) - 1U, &read_count, NULL) && read_count > 0U) {
        WCHAR* message;
        buffer[read_count] = '\0';
        message = nx_wide_from_utf8(buffer);
        if (message != NULL) PostMessageW(g_main, WM_NX_OUTPUT, 0, (LPARAM)message);
    }
    WaitForSingleObject(process.hProcess, INFINITE);
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    CloseHandle(read_pipe);
    free(wide_command);
    PostMessageW(g_main, WM_NX_DONE, (WPARAM)exit_code, 0);
    return exit_code;
}

static void nx_start_action(void)
{
    WCHAR input_w[1024];
    WCHAR option_w[64];
    char* root;
    char* input;
    char* option;
    char* command;
    GetWindowTextW(g_input, input_w, 1024);
    GetWindowTextW(g_option, option_w, 64);
    root = nx_utf8_from_wide(g_root);
    input = nx_utf8_from_wide(input_w);
    option = nx_utf8_from_wide(option_w);
    command = (char*)calloc(4096U, 1U);
    if (root == NULL || input == NULL || option == NULL || command == NULL ||
        !NxCc_BuildCommand(g_action, root, input, option, command, 4096U)) {
        nx_append_log(L"\r\n[NEXIORA] Completa los datos requeridos para esta operación.\r\n");
        free(root); free(input); free(option); free(command);
        return;
    }
    free(root); free(input); free(option);
    SetWindowTextW(g_log, L"[NEXIORA] Iniciando operación...\r\n");
    SendMessageW(g_progress, PBM_SETMARQUEE, TRUE, 40);
    EnableWindow(g_run, FALSE);
    if (CreateThread(NULL, 0U, nx_worker, command, 0U, NULL) == NULL) {
        free(command);
        SendMessageW(g_progress, PBM_SETMARQUEE, FALSE, 0);
        EnableWindow(g_run, TRUE);
        nx_append_log(L"[NEXIORA] No se pudo crear el trabajador.\r\n");
    }
}

static void nx_create_controls(HWND window)
{
    static const WCHAR* labels[] = {L"Inicio", L"Aprender archivo", L"YouTube", L"Herramientas", L"Pruebas", L"Paquetes", L"Documentación"};
    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    int index;
    CreateWindowW(L"STATIC", L"NEXIORA", WS_CHILD | WS_VISIBLE, 24, 20, 180, 32, window, NULL, NULL, NULL);
    for (index = 0; index < 7; ++index) {
        HWND button = CreateWindowW(L"BUTTON", labels[index], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    20, 70 + (index * 44), 185, 34, window,
                                    (HMENU)(INT_PTR)(IDC_NAV_BASE + index), NULL, NULL);
        SendMessageW(button, WM_SETFONT, (WPARAM)font, TRUE);
    }
    g_title = CreateWindowW(L"STATIC", L"Panel principal", WS_CHILD | WS_VISIBLE, 240, 24, 620, 28, window, (HMENU)(INT_PTR)IDC_TITLE, NULL, NULL);
    g_desc = CreateWindowW(L"STATIC", L"Centro unificado para operar y observar Nexiora.", WS_CHILD | WS_VISIBLE, 240, 58, 700, 40, window, (HMENU)(INT_PTR)IDC_DESC, NULL, NULL);
    CreateWindowW(L"STATIC", L"Entrada (archivo, URL o Package ID)", WS_CHILD | WS_VISIBLE, 240, 112, 300, 20, window, NULL, NULL, NULL);
    g_input = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 240, 136, 620, 30, window, (HMENU)(INT_PTR)IDC_INPUT, NULL, NULL);
    CreateWindowW(L"STATIC", L"Idioma", WS_CHILD | WS_VISIBLE, 875, 112, 80, 20, window, NULL, NULL, NULL);
    g_option = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"es", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 875, 136, 80, 30, window, (HMENU)(INT_PTR)IDC_OPTION, NULL, NULL);
    g_run = CreateWindowW(L"BUTTON", L"Ejecutar", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 240, 180, 140, 36, window, (HMENU)(INT_PTR)IDC_RUN, NULL, NULL);
    g_progress = CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_MARQUEE, 395, 185, 560, 24, window, (HMENU)(INT_PTR)IDC_PROGRESS, NULL, NULL);
    g_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Bienvenido a Nexiora Control Center.\r\nSelecciona una capacidad para comenzar.",
                            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                            240, 235, 715, 380, window, (HMENU)(INT_PTR)IDC_LOG, NULL, NULL);
    SendMessageW(g_title, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(g_desc, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(g_input, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(g_option, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(g_run, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(g_log, WM_SETFONT, (WPARAM)font, TRUE);
    nx_set_action(NX_CC_ACTION_DASHBOARD);
}

static LRESULT CALLBACK nx_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
        case WM_CREATE:
            g_main = window;
            GetCurrentDirectoryW(MAX_PATH, g_root);
            nx_create_controls(window);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) >= IDC_NAV_BASE && LOWORD(wparam) < IDC_NAV_BASE + 7) {
                nx_set_action((NxCcAction)(LOWORD(wparam) - IDC_NAV_BASE));
                return 0;
            }
            if (LOWORD(wparam) == IDC_RUN) {
                nx_start_action();
                return 0;
            }
            break;
        case WM_NX_OUTPUT:
            if (lparam != 0) {
                nx_append_log((const WCHAR*)lparam);
                free((void*)lparam);
            }
            return 0;
        case WM_NX_DONE:
            SendMessageW(g_progress, PBM_SETMARQUEE, FALSE, 0);
            EnableWindow(g_run, TRUE);
            nx_append_log((wparam == 0U) ? L"\r\n[NEXIORA] Operación completada correctamente.\r\n" : L"\r\n[NEXIORA] La operación terminó con error. Revisa la salida anterior.\r\n");
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR command_line, int show)
{
    WNDCLASSEXW wc;
    HWND window;
    MSG message;
    INITCOMMONCONTROLSEX controls;
    (void)previous;
    (void)command_line;
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&controls);
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = nx_window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"NexioraControlCenterWindow";
    if (!RegisterClassExW(&wc)) return 1;
    window = CreateWindowExW(0, wc.lpszClassName, L"Nexiora Control Center", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700, NULL, NULL, instance, NULL);
    if (window == NULL) return 1;
    ShowWindow(window, show);
    UpdateWindow(window);
    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return (int)message.wParam;
}
