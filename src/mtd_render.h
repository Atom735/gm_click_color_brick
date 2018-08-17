#ifndef _H_MTD_RENDER_H_
#define _H_MTD_RENDER_H_

#include <windows.h>

typedef struct render RENDER;

RENDER* RENDER_Alloc();
VOID    RENDER_Free( RENDER * p );
VOID    RENDER_Create( RENDER * p, HDC hDC, UINT w, UINT h );
VOID    RENDER_Flush( RENDER * p, HDC hDC );
VOID    RENDER_DrawTest( RENDER * p, UINT w, UINT h, UINT x, UINT y, UINT i );

#endif
