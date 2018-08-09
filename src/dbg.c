#define DBG_LOG_STRINGS
#include "dbg.h"

#ifndef NDEBUG
    HANDLE          g_hLogConsoleOut    = NULL;
    HANDLE          g_hLogFileOut       = NULL;
    VOID rLogDbg( LPCWSTR fmt, ... )
    {
        WCHAR pBuf[1024];
        DWORD dw;
        INT nLen = 0;
        nLen = snwprintf( pBuf, 1020, L"[% 10d]: ", (int)GetTickCount() );
        va_list args;
        va_start( args, fmt );
        nLen += vsnwprintf( pBuf+nLen, 1020-nLen, fmt, args );
        va_end( args );
        pBuf[nLen] = L'\n';
        WriteConsoleW( g_hLogConsoleOut, pBuf, nLen+1, &dw, NULL );
        WriteFile( g_hLogFileOut, pBuf, (nLen+1)*sizeof(WCHAR), &dw, NULL );
    }
    VOID rLogDbgInit( LPCWSTR fName )
    {
        AllocConsole();
        g_hLogConsoleOut = GetStdHandle( STD_OUTPUT_HANDLE );
        g_hLogFileOut = CreateFileW( fName, GENERIC_WRITE, FILE_SHARE_READ,
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        SetFilePointer( g_hLogFileOut, 0, NULL, FILE_END );
        rLogDbg( g_asLogStrings[LOGS___START] );
    }
    VOID rLogDbgRelease()
    {
        rLogDbg( g_asLogStrings[LOGS___END] );
        CloseHandle( g_hLogFileOut );
        system( "pause" );
    }
#endif