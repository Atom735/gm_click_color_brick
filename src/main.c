#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "dbg.h"

CONST LPCWSTR   g_lpWndClassName    = L"Atom735-WCN-gmClickHexBlock";
HINSTANCE       g_hInstance         = NULL;
HANDLE          g_hLogFileOut       = NULL;

LRESULT CALLBACK rMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LOGDBG_WM( hWnd, uMsg, wParam, lParam );
    switch( uMsg )
    {
        case WM_CREATE:
        {
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect( hWnd, &rc );

            FillRect( (HDC)(wParam), &rc, GetStockBrush( BLACK_BRUSH ) );

            return 1;
        }
        case WM_PAINT:
        {
            HDC hDC;
            PAINTSTRUCT ps;
            hDC = BeginPaint( hWnd, &ps);
            EndPaint( hWnd, &ps );
            return 0;
        }
        case WM_KEYDOWN:
        {
            switch( wParam )
            {
                case VK_ESCAPE:
                {
                    DestroyWindow( hWnd );
                    return 0;
                }
                case VK_F5:
                {
                    InvalidateRect( hWnd, NULL, TRUE );
                    return 0;
                }
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            return 0;
        }
        case WM_CLOSE:
        {
            DestroyWindow( hWnd );
            return 0;
        }
    }
    return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

#ifdef UNICODE
    INT APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, INT nShowCmd )
#else
    INT APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, INT nShowCmd )
#endif
{
    rLogDbgInit( L"log.log" );
    g_hInstance = hInstance;
    LOGDBG( L"g_hInstance = [%p]", g_hInstance );

    {
        WNDCLASSEXW wc;
        ZeroMemory( &wc, sizeof( wc ) );

        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
        wc.lpfnWndProc   = rMsgProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = g_hInstance;
        wc.hIcon         = LoadIconW(NULL, IDI_WINLOGO);
        wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = g_lpWndClassName;
        wc.hIconSm       = LoadIconW(NULL, IDI_WINLOGO);

        if( RegisterClassExW( &wc ) == 0 )
        {
            LOGDBG( g_asLogStrings[LOGS_ERR_WNDCL_CANT_REG] );
            DWORD er = GetLastError();
            LOGDBG( g_asLogStrings[LOGS_ERROR_CODE], er, er );
            return er;
        }
        else
        {
            LOGDBG( g_asLogStrings[LOGS_OK_WNDCL_REGED] );
            LOGDBG( L"g_lpWndClassName = [%s]", g_lpWndClassName );
        }
    }

    {
        HWND hWnd = CreateWindowExW( 0, g_lpWndClassName, NULL, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
            NULL, g_hInstance, NULL );
        /*CW_USEDEFAULT*/ /*GetSystemMetrics( SM_CXSCREEN ) SM_CYSCREEN*/
        if( hWnd == NULL )
        {
            LOGDBG( g_asLogStrings[LOGS_ERR_WND_CANT_CREATE] );
            DWORD er = GetLastError();
            LOGDBG( g_asLogStrings[LOGS_ERROR_CODE], er, er );
            return er;
        }
        else
        {
            LOGDBG( g_asLogStrings[LOGS_OK_WND_CREATED] );
            LOGDBG( L"HWND = [%p]", hWnd );
            ShowWindow( hWnd, SW_SHOWMAXIMIZED );
            UpdateWindow( hWnd );
            SetFocus( hWnd );
        }
    }

    {
        MSG msg;
        ZeroMemory( &msg, sizeof( msg ) );
        while( msg.message != WM_QUIT )
        {
            if( PeekMessageW( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                TranslateMessage( &msg );
                DispatchMessageW( &msg );
            }
            else
            {
                /* TODO: Idle */
            }
        }
    }

    UnregisterClassW( g_lpWndClassName, g_hInstance );
    LOGDBG( g_asLogStrings[LOGS_OK_WNDCL_UNREGED] );

    rLogDbgRelease();
    return 0;
}