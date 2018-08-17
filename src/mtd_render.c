#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dbg.h"
#include "mtd_render.h"

struct render
{
    UINT w;
    UINT h;

    HDC         bmp_hDC;
    UINT32     *bmp_pvBits;
    FLOAT      *bmp_pvLights;
    HBITMAP     bmp_hBMP;
    HBITMAP     bmp_hBMP_Old;
    HFONT       bmp_hFont;
    HFONT       bmp_hFont_Old;

    HDC         _bmp_hDC;
    HBITMAP     _bmp_hBMP;
    HBITMAP     _bmp_hBMP_Old;
};

RENDER* RENDER_Alloc()
{
    RENDER * p = (RENDER*)malloc( sizeof(RENDER) );
    assert( p != NULL );
    memset( p, 0, sizeof(RENDER) );
    return p;
}

static void _RENDER_Delete( RENDER * p )
{
    assert( p != NULL );
    if( p->bmp_hDC != NULL )
    {
        SelectBitmap( p->_bmp_hDC, p->_bmp_hBMP_Old );
        DeleteBitmap( p->_bmp_hBMP );
        DeleteDC( p->_bmp_hDC );


        DeleteFont( p->bmp_hFont );
        SelectBitmap( p->bmp_hDC, p->bmp_hBMP_Old );
        DeleteBitmap( p->bmp_hBMP );
        DeleteDC( p->bmp_hDC );
        p->bmp_hDC = NULL;
        free(p->bmp_pvLights);
    }
}
VOID    RENDER_Free( RENDER * p )
{
    assert( p != NULL );
    _RENDER_Delete( p );
    free( p );
}
VOID    RENDER_Create( RENDER * p, HDC hDC, UINT w, UINT h )
{
    assert( p != NULL );
    assert( hDC != NULL );
    _RENDER_Delete( p );
    p->w = w;
    p->h = h;
    if( w == 0 ) return;
    if( h == 0 ) return;
    p->bmp_hDC = CreateCompatibleDC( hDC );
    assert( p->bmp_hDC != NULL );
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;
    p->bmp_hBMP = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
        (PVOID*)(&p->bmp_pvBits), NULL, 0x0 );
    assert( p->bmp_hBMP != NULL );
    assert( p->bmp_pvBits != NULL );
    p->bmp_hBMP_Old = SelectBitmap( p->bmp_hDC, p->bmp_hBMP );

    p->bmp_pvLights = (FLOAT*)malloc( w * h * sizeof(FLOAT) );
    assert( p->bmp_pvLights != NULL );

    LOGFONTW lf;
    lf.lfHeight         = -24;
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_DONTCARE;//FW_REGULAR;
    lf.lfItalic         = FALSE;
    lf.lfUnderline      = FALSE;
    lf.lfStrikeOut      = FALSE;
    lf.lfCharSet        = ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    memcpy( lf.lfFaceName, L"Roboto", 6*2 );
    p->bmp_hFont = CreateFontIndirectW( &lf );


    p->_bmp_hDC = CreateCompatibleDC( hDC );
    assert( p->_bmp_hDC != NULL );
    p->_bmp_hBMP = (HBITMAP)LoadImageA( NULL, "+test.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
    assert( p->_bmp_hBMP != NULL );
    p->_bmp_hBMP_Old = SelectBitmap( p->_bmp_hDC, p->_bmp_hBMP );
}
VOID    RENDER_Flush( RENDER * p, HDC hDC )
{
    assert( p != NULL );
    assert( hDC != NULL );
    if( p->w == 0 ) return;
    if( p->h == 0 ) return;
    BitBlt( hDC, 0, 0, p->w, p->h, p->bmp_hDC, 0, 0, SRCCOPY );
}






VOID    RENDER_DrawTest( RENDER * p, UINT w, UINT h, UINT x, UINT y, UINT i )
{
    assert( p != NULL );
    if( p->w == 0 ) return;
    if( p->h == 0 ) return;

    const UINT      _w  = p->w;
    const UINT      _h  = p->h;
    const UINT      _wh = _w*_h;
    UINT32 * const  _pc = p->bmp_pvBits;
    FLOAT * const   _pl = p->bmp_pvLights;

    memset(_pc,0xff,_wh*4);

    #define FOR_(_I,_A,_B) for( UINT _I = _A; _I < _B; ++_I )
    #define FOR_EQ(_I,_A,_B) for( UINT _I = _A; _I <= _B; ++_I )
    #define FOR_ALL_PIXELS(_I) FOR_(_I,0,_wh)
    #define FOR_ALL_PIXELS_HORIZONTAL(_I) FOR_(_I,0,_w)
    #define FOR_ALL_PIXELS_VERTICAL(_I) FOR_(_I,0,_h)
    #define GET_COLOR_(_I) _pc[_I]
    #define GET_COLOR(_X,_Y) _pc[(_X)+((_Y)*_w)]
    #define SET_COLOR_(_I,_C) GET_COLOR_(_I)=_C
    #define SET_COLOR(_X,_Y,_C) GET_COLOR_(_X,_Y)=_C
    #define GET_LIGHT_(_I) _pl[_I]
    #define GET_LIGHT(_X,_Y) _pl[(_X)+((_Y)*_w)]
    #define SET_LIGHT_(_I,_C) GET_LIGHT_(_I)=_C
    #define SET_LIGHT(_X,_Y,_C) GET_LIGHT(_X,_Y)=_C

    FOR_ALL_PIXELS(i) GET_LIGHT_(i) = 255.0f;

    const FLOAT f_x = (FLOAT)x/13.0f;
    const FLOAT f_y = (FLOAT)y/13.0f;

    VOID ShadowRect( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1) FOR_(ix,ux0,ux1) GET_LIGHT(ix,iy) *= 1.0f-a;
    }

    VOID ShadowRectLV( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT f = ((FLOAT)iy+0.5f-y)/h;
            FOR_(ix,ux0,ux1)
            {
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectLH( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(ix,ux0,ux1)
        {
            const FLOAT f = ((FLOAT)ix+0.5f-x)/w;
            FOR_(iy,uy0,uy1)
            {
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_R0( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y-h)/h;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x-w)/w;
                const FLOAT _r = fx*fx+fy*fy;
                const FLOAT f = (_r<1.0f) ? 1.0f-sqrtf(_r) : 0.0f;
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_R1( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y-h)/h;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x)/w;
                const FLOAT _r = fx*fx+fy*fy;
                const FLOAT f = (_r<1.0f) ? 1.0f-sqrtf(_r) : 0.0f;
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_R2( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y)/h;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x-w)/w;
                const FLOAT _r = fx*fx+fy*fy;
                const FLOAT f = (_r<1.0f) ? 1.0f-sqrtf(_r) : 0.0f;
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_R3( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y)/h;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x)/w;
                const FLOAT _r = fx*fx+fy*fy;
                const FLOAT f = (_r<1.0f) ? 1.0f-sqrtf(_r) : 0.0f;
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    ShadowRect    (f_x      ,f_y      ,9.0f,9.0f,0.5f);
    ShadowRect    (f_x+ 9.0f,f_y+ 9.0f,3.0f,3.0f,0.5f);
    ShadowRect    (f_x+12.0f,f_y+12.0f,2.0f,2.0f,0.5f);
    ShadowRect    (f_x+14.0f,f_y+14.0f,1.0f,1.0f,0.5f);

    ShadowRectLV  (f_x      ,f_y- 9.0f,9.0f,9.0f,0.0f,0.5f);
    ShadowRectLV  (f_x+ 9.0f,f_y+ 6.0f,3.0f,3.0f,0.0f,0.5f);
    ShadowRectLV  (f_x+12.0f,f_y+10.0f,2.0f,2.0f,0.0f,0.5f);
    ShadowRectLV  (f_x+14.0f,f_y+13.0f,1.0f,1.0f,0.0f,0.5f);

    ShadowRectLV  (f_x      ,f_y+ 9.0f,9.0f,9.0f,0.5f,0.0f);
    ShadowRectLV  (f_x+ 9.0f,f_y+12.0f,3.0f,3.0f,0.5f,0.0f);
    ShadowRectLV  (f_x+12.0f,f_y+14.0f,2.0f,2.0f,0.5f,0.0f);
    ShadowRectLV  (f_x+14.0f,f_y+15.0f,1.0f,1.0f,0.5f,0.0f);

    ShadowRectLH  (f_x- 9.0f,f_y      ,9.0f,9.0f,0.0f,0.5f);
    ShadowRectLH  (f_x+ 6.0f,f_y+ 9.0f,3.0f,3.0f,0.0f,0.5f);
    ShadowRectLH  (f_x+10.0f,f_y+12.0f,2.0f,2.0f,0.0f,0.5f);
    ShadowRectLH  (f_x+13.0f,f_y+14.0f,1.0f,1.0f,0.0f,0.5f);

    ShadowRectLH  (f_x+ 9.0f,f_y      ,9.0f,9.0f,0.5f,0.0f);
    ShadowRectLH  (f_x+12.0f,f_y+ 9.0f,3.0f,3.0f,0.5f,0.0f);
    ShadowRectLH  (f_x+14.0f,f_y+12.0f,2.0f,2.0f,0.5f,0.0f);
    ShadowRectLH  (f_x+15.0f,f_y+14.0f,1.0f,1.0f,0.5f,0.0f);

    ShadowRectL_R0(f_x- 9.0f,f_y- 9.0f,9.0f,9.0f,0.0f,0.5f);
    ShadowRectL_R0(f_x+ 6.0f,f_y+ 6.0f,3.0f,3.0f,0.0f,0.5f);
    ShadowRectL_R0(f_x+10.0f,f_y+10.0f,2.0f,2.0f,0.0f,0.5f);
    ShadowRectL_R0(f_x+13.0f,f_y+13.0f,1.0f,1.0f,0.0f,0.5f);

    ShadowRectL_R1(f_x+ 9.0f,f_y- 9.0f,9.0f,9.0f,0.0f,0.5f);
    ShadowRectL_R1(f_x+12.0f,f_y+ 6.0f,3.0f,3.0f,0.0f,0.5f);
    ShadowRectL_R1(f_x+14.0f,f_y+10.0f,2.0f,2.0f,0.0f,0.5f);
    ShadowRectL_R1(f_x+15.0f,f_y+13.0f,1.0f,1.0f,0.0f,0.5f);

    ShadowRectL_R2(f_x- 9.0f,f_y+ 9.0f,9.0f,9.0f,0.0f,0.5f);
    ShadowRectL_R2(f_x+ 6.0f,f_y+12.0f,3.0f,3.0f,0.0f,0.5f);
    ShadowRectL_R2(f_x+10.0f,f_y+14.0f,2.0f,2.0f,0.0f,0.5f);
    ShadowRectL_R2(f_x+13.0f,f_y+15.0f,1.0f,1.0f,0.0f,0.5f);

    ShadowRectL_R3(f_x+ 9.0f,f_y+ 9.0f,9.0f,9.0f,0.0f,0.5f);
    ShadowRectL_R3(f_x+12.0f,f_y+12.0f,3.0f,3.0f,0.0f,0.5f);
    ShadowRectL_R3(f_x+14.0f,f_y+14.0f,2.0f,2.0f,0.0f,0.5f);
    ShadowRectL_R3(f_x+15.0f,f_y+15.0f,1.0f,1.0f,0.0f,0.5f);


    FOR_ALL_PIXELS_VERTICAL(iy)
    FOR_ALL_PIXELS_HORIZONTAL(ix)
    {
        memset(_pc+(ix+iy*_w),(BYTE)GET_LIGHT(ix/13,iy/13),4);
    }

    p->bmp_hFont_Old = SelectFont( p->bmp_hDC, p->bmp_hFont );


    SetBkMode( p->bmp_hDC, TRANSPARENT );

    {
        SetTextAlign( p->bmp_hDC, TA_LEFT );
        CHAR str[256];
        UINT sz;
        #define TEXTOUT(_I, ...) ((sz=snprintf(str,256,__VA_ARGS__)),\
            SetTextColor( p->bmp_hDC, 0x000000 ),\
            TextOutA( p->bmp_hDC, 6,6+24*(_I), str, sz ),\
            SetTextColor( p->bmp_hDC, 0x7f7f7f ),\
            TextOutA( p->bmp_hDC, 9,9+24*(_I), str, sz ),\
            SetTextColor( p->bmp_hDC, 0xffffff ),\
            TextOutA( p->bmp_hDC, 8,8+24*(_I), str, sz ))

        TEXTOUT(0,"Cursor = %u : %u", x, y);
    }
    SelectFont( p->bmp_hDC, p->bmp_hFont_Old );

    FOR_ALL_PIXELS_VERTICAL(iy)
        GET_COLOR(x,iy)^=0xfffc00;
    FOR_ALL_PIXELS_HORIZONTAL(ix)
        GET_COLOR(ix,y)^=0x00fcff;


}
