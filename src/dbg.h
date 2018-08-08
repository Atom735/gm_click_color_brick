#ifndef _H_DBG_H_
#define _H_DBG_H_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define LOGDBG(...) rLogDbg(__VA_ARGS__)

#ifndef NDEBUG
    VOID rLogDbg( LPCWSTR fmt, ... );
    VOID rLogDbgInit( LPCWSTR fName );
    VOID rLogDbgRelease();
    VOID LOGDBG_WM( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#else
    #define rLogDbg(...) ((void)0)
    #define rLogDbgInit(a) ((void)0)
    #define rLogDbgRelease() ((void)0)
    #define LOGDBG_WM(a,b,c,d) ((void)0)
#endif

#ifndef DBG_LOG_STRINGS
    extern LPCWSTR g_asLogStrings[];
#else
LPCWSTR         g_asLogStrings[] = {
    L"Код ошибки: %08X (%d)",
    L"### LOG START ###",
    L"### LOG END ###",
    L"Невозможно зарегестрировать класс окна",
    L"Класс окна успешно зарегестрирован",
    L"Класс окна успешно освобожден",
    L"Невозможно создать окно",
    L"Окно создано",
};
#endif

enum
{
    LOGS_ERROR_CODE = 0,
    LOGS___START,
    LOGS___END,
    LOGS_ERR_WNDCL_CANT_REG,
    LOGS_OK_WNDCL_REGED,
    LOGS_OK_WNDCL_UNREGED,
    LOGS_ERR_WND_CANT_CREATE,
    LOGS_OK_WND_CREATED,
};

#endif