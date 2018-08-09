#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "dbg.h"

CONST LPCWSTR   g_lpWndClassName    = L"Atom735-WCN-gmClickColorBrick";
HINSTANCE       g_hInstance         = NULL;

struct gm_CCB
{




    struct {
        UINT x, y, w, h, o, oh;
    } VP;
    struct {
        UINT x, y, w, h;
        PUINT8 p;
    } Grid;
    struct {
        UINT n;
        PUINT32 p;
    } Colors;
};

LRESULT CALLBACK rMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LOGDBG_WM( hWnd, uMsg, wParam, lParam );

    static UINT     gm_nClientWidth     = 0;
    static UINT     gm_nClientHeight    = 0;

    static UINT     gm_nInterfaceOffset = 4;
    static UINT     gm_nInterfaceHeight = 32;
    static UINT     gm_nVP_x;
    static UINT     gm_nVP_y;
    static UINT     gm_nVP_w;
    static UINT     gm_nVP_h;

    static UINT     gm_nGridMouseX      = -1;
    static UINT     gm_nGridMouseY      = -1;

    static UINT     gm_nGridWidth       = 40;
    static UINT     gm_nGridHeight      = 30;
    static PUINT8   gm_pGrid            = NULL;

    static UINT     gm_nColorsCount     = 7;
    static UINT32   gm_pColors[16];

    INT _rand( INT x, INT a, INT c )
    {
        return (a*x+c);
    }

    switch( uMsg )
    {
        case WM_CREATE:
        {
            CONST UINT sz = gm_nGridHeight*gm_nGridWidth;
            gm_pGrid = (PUINT8)malloc( sz );
            // memset( gm_pGrid, 0, sz );
            int k = 534534;
            for( int i=0; i<sz; ++i )
                gm_pGrid[i] = (((k=_rand(k,137,574))%gm_nColorsCount)+1)<<1;

            inline VOID _setCol( INT i, UINT32 cN, UINT32 cG )
            {
                gm_pColors[i*2+0] = cN;
                gm_pColors[i*2+1] = cG;
            }

            _setCol( 0, 0x37474F, 0x263238 );
            /* 800 600 */
            _setCol( 1, 0xC62828, 0xE53935 ); /* Red */
            _setCol( 2, 0x6A1B9A, 0x8E24AA ); /* Purple */
            _setCol( 3, 0x283593, 0x3949AB ); /* Indigo */
            _setCol( 4, 0x00838F, 0x00ACC1 ); /* Cyan */
            _setCol( 5, 0x558B2F, 0x7CB342 ); /* Light Green */
            _setCol( 6, 0xF9A825, 0xFDD835 ); /* Yellow */
            _setCol( 7, 0x4E342E, 0x6D4C41 ); /* Brown */

            return 0;
        }
        case WM_SIZE:
        {
            gm_nClientWidth     = LOWORD(lParam);
            gm_nClientHeight    = HIWORD(lParam);

            gm_nVP_w    = gm_nClientWidth  - (gm_nInterfaceOffset*2);
            gm_nVP_h    = gm_nClientHeight - (gm_nInterfaceOffset*2) - gm_nInterfaceHeight;

            gm_nVP_x    = gm_nInterfaceOffset;
            gm_nVP_y    = gm_nInterfaceOffset;

            CONST UINT cw = gm_nVP_w*gm_nGridHeight;
            CONST UINT ch = gm_nVP_h*gm_nGridWidth;
            if(cw > ch )
            {
                gm_nVP_w = ch/gm_nGridHeight;
                gm_nVP_x = (gm_nClientWidth-gm_nVP_w)/2;
            }
            else
            {
                gm_nVP_h = cw/gm_nGridWidth;
                gm_nVP_y = (gm_nClientHeight-gm_nInterfaceHeight-gm_nVP_h)/2;
            }

            return 0;
        }
        case WM_DESTROY:
        {
            free( gm_pGrid );;
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
            HDC bmp_hDC;
            PUINT32 bmp_pvBits;
            HBITMAP bmp_hBMP;
            HBITMAP bmp_hBMP_Old;

            PAINTSTRUCT ps;
            HDC hDC = BeginPaint( hWnd, &ps);
            {
                bmp_hDC = CreateCompatibleDC( hDC );
                BITMAPINFO bmi;
                ZeroMemory(&bmi, sizeof(BITMAPINFO));
                bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth       = gm_nClientWidth;
                bmi.bmiHeader.biHeight      = gm_nClientHeight;
                bmi.bmiHeader.biPlanes      = 1;
                bmi.bmiHeader.biBitCount    = 32;
                bmi.bmiHeader.biCompression = BI_RGB;
                bmi.bmiHeader.biSizeImage   = gm_nClientWidth * gm_nClientHeight * 4;
                bmp_hBMP = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                    (PVOID*)(&bmp_pvBits), NULL, 0x0 );
                bmp_hBMP_Old = SelectBitmap( bmp_hDC, bmp_hBMP );
            }


            PUINT32 p=bmp_pvBits;
            for( int y=0; y<gm_nClientHeight; ++y )
            for( int x=0; x<gm_nClientWidth ; ++x )
            {
                FLOAT fgx = (FLOAT)(x-gm_nVP_x)*(FLOAT)(gm_nGridWidth )/(FLOAT)(gm_nVP_w);
                FLOAT fgy = (FLOAT)(y-gm_nVP_y)*(FLOAT)(gm_nGridHeight)/(FLOAT)(gm_nVP_h);

                if( ( fgx <= 0.0f ) || ( fgy <= 0.0f ) )
                    *p = gm_pColors[1];
                else
                {
                    CONST UINT ux = (UINT)floorf(fgx);
                    CONST UINT uy = (UINT)floorf(fgy);
                    if( ( ux >= gm_nGridWidth ) || ( uy >= gm_nGridHeight ) )
                        *p = gm_pColors[1];
                    else
                    {
                        INT i = (ux*gm_nGridHeight) + uy;
                        // *p = (((i)&0x7)<<5)|((((i>>6)&0x7)<<5)<<8)|((((i>>3)&0x7)<<5)<<16);
                        *p = gm_pColors[gm_pGrid[i]];
                    }
                }
                ++p;
            }



            {
                BitBlt( hDC, 0, 0, gm_nClientWidth, gm_nClientHeight,
                    bmp_hDC, 0, 0, SRCCOPY );
                SelectBitmap( bmp_hDC, bmp_hBMP_Old );
                DeleteBitmap( bmp_hBMP );
                DeleteDC( bmp_hDC );
            }
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
            UINT c = 0;
            static UINT _hover_count = 0;
            VOID _hover_unset( UINT x, UINT y )
            {
                if( ( x < gm_nGridWidth ) && ( y < gm_nGridWidth ) &&
                    ( ( gm_pGrid[y+x*gm_nGridHeight] & 0xfe ) == c ) &&
                    ( ( gm_pGrid[y+x*gm_nGridHeight] & 0x01 ) == 1 ) )
                {
                    gm_pGrid[y+x*gm_nGridHeight] &= 0xfe;
                    _hover_unset( x+1, y );
                    _hover_unset( x-1, y );
                    _hover_unset( x, y+1 );
                    _hover_unset( x, y-1 );
                }
            }
            VOID _hover_set( UINT x, UINT y )
            {
                if( ( x < gm_nGridWidth ) && ( y < gm_nGridWidth ) &&
                    ( ( gm_pGrid[y+x*gm_nGridHeight] & 0xfe ) == c ) &&
                    ( ( gm_pGrid[y+x*gm_nGridHeight] & 0x01 ) == 0 ) )
                {
                    gm_pGrid[y+x*gm_nGridHeight] |= 0x01;
                    ++_hover_count;
                    _hover_set( x+1, y );
                    _hover_set( x-1, y );
                    _hover_set( x, y+1 );
                    _hover_set( x, y-1 );
                }
            }
            CONST UINT ux = (UINT)((FLOAT)(GET_X_LPARAM(lParam)-gm_nVP_x)*(FLOAT)(gm_nGridWidth )/(FLOAT)(gm_nVP_w));
            CONST UINT uy = (UINT)((FLOAT)(gm_nClientHeight-GET_Y_LPARAM(lParam)-1-gm_nVP_y)*(FLOAT)(gm_nGridHeight)/(FLOAT)(gm_nVP_h));
            if( ( gm_nGridMouseX < gm_nGridWidth ) && ( gm_nGridMouseY < gm_nGridHeight ) )
            {
                c = gm_pGrid[gm_nGridMouseY+gm_nGridMouseX*gm_nGridHeight] & 0xfe;
                if( ( ux >= gm_nGridWidth ) || ( uy >= gm_nGridHeight ) )
                {
                    _hover_count = 0;
                    _hover_unset( gm_nGridMouseX, gm_nGridMouseY );
                }
                else
                if( ( ux < gm_nGridWidth ) && ( uy < gm_nGridHeight ) &&
                    ( ( gm_pGrid[ux+uy*gm_nGridHeight] & 0x01 ) == 0 ) )
                {
                    _hover_count = 0;
                    _hover_unset( gm_nGridMouseX, gm_nGridMouseY );
                    c = gm_pGrid[ux+uy*gm_nGridHeight] & 0xfe;
                    if( c > 0 )
                    {
                        _hover_set( ux, uy );
                    }
                }
            }
            else
            if( ( ux < gm_nGridWidth ) && ( uy < gm_nGridHeight ) )
            {
                _hover_count = 0;
                c = gm_pGrid[ux+uy*gm_nGridHeight] & 0xfe;
                if( c > 0 )
                {
                    _hover_set( ux, uy );
                }
            }
            gm_nGridMouseX = ux;
            gm_nGridMouseY = uy;
            InvalidateRect( hWnd, NULL, FALSE );
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