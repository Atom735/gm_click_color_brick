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

    VOID ShadowRectLV( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
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
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
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

    VOID ShadowRectL_R3( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
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
                if (_r>1.0f) continue;
                const FLOAT f = 1.0f-sqrtf(_r);
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }


    VOID ShadowRectL_R2( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
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
                if (_r>1.0f) continue;
                const FLOAT f = 1.0f-sqrtf(_r);
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_R1( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
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
                if (_r>1.0f) continue;
                const FLOAT f = 1.0f-sqrtf(_r);
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_R0( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
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
                if (_r>1.0f) continue;
                const FLOAT f = 1.0f-sqrtf(_r);
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_RR0( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b, FLOAT w2, FLOAT h2 )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w2), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h2), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y)/h;
            const FLOAT fy2 = ((FLOAT)iy+0.5f-y)/h2;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x)/w;
                const FLOAT _r = fx*fx+fy*fy;
                if (_r<1.0f) continue;
                const FLOAT fx2 = ((FLOAT)ix+0.5f-x)/w2;
                const FLOAT _r2 = fx2*fx2+fy2*fy2;
                if (_r2>1.0f) continue;
                const FLOAT f = (1.0f-sqrtf(_r2))/(sqrtf(_r)-sqrtf(_r2));
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_RR1( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b, FLOAT w2, FLOAT h2 )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w2), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h2), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y)/h;
            const FLOAT fy2 = ((FLOAT)iy+0.5f-y)/h2;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x-w2)/w;
                const FLOAT _r = fx*fx+fy*fy;
                if (_r<1.0f) continue;
                const FLOAT fx2 = ((FLOAT)ix+0.5f-x-w2)/w2;
                const FLOAT _r2 = fx2*fx2+fy2*fy2;
                if (_r2>1.0f) continue;
                const FLOAT f = (1.0f-sqrtf(_r2))/(sqrtf(_r)-sqrtf(_r2));
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }


    VOID ShadowRectL_RR2( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b, FLOAT w2, FLOAT h2 )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w2), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h2), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y-h2)/h;
            const FLOAT fy2 = ((FLOAT)iy+0.5f-y-h2)/h2;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x)/w;
                const FLOAT _r = fx*fx+fy*fy;
                if (_r<1.0f) continue;
                const FLOAT fx2 = ((FLOAT)ix+0.5f-x)/w2;
                const FLOAT _r2 = fx2*fx2+fy2*fy2;
                if (_r2>1.0f) continue;
                const FLOAT f = (1.0f-_r2)/(_r-_r2);
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRectL_RR3( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b, FLOAT w2, FLOAT h2 )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w2), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h2), _h );
        FOR_(iy,uy0,uy1)
        {
            const FLOAT fy = ((FLOAT)iy+0.5f-y-h2)/h;
            const FLOAT fy2 = ((FLOAT)iy+0.5f-y-h2)/h2;
            FOR_(ix,ux0,ux1)
            {
                const FLOAT fx = ((FLOAT)ix+0.5f-x-w2)/w;
                const FLOAT _r = fx*fx+fy*fy;
                if (_r<1.0f) continue;
                const FLOAT fx2 = ((FLOAT)ix+0.5f-x-w2)/w2;
                const FLOAT _r2 = fx2*fx2+fy2*fy2;
                if (_r2>1.0f) continue;
                const FLOAT f = (1.0f-_r2)/(_r-_r2);
                GET_LIGHT(ix,iy) *= 1.0f-a+f*(a-b);
            }
        }
    }

    VOID ShadowRect( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a )
    {
        if (x+w<0.0f) return;
        if (y+h<0.0f) return;
        const UINT ux0 = (x<0.0f) ? 0 : (UINT)roundf(x);
        const UINT ux1 = __min( (UINT)roundf(x+w), _w );
        const UINT uy0 = (y<0.0f) ? 0 : (UINT)roundf(y);
        const UINT uy1 = __min( (UINT)roundf(y+h), _h );
        FOR_(iy,uy0,uy1) FOR_(ix,ux0,ux1) GET_LIGHT(ix,iy) *= 1.0f-a;
    }
    VOID ShadowRectR( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT rx, FLOAT ry )
    {
        if (rx <= 0.5f)
        {
            ShadowRect(x,y,w,h,a);
            return;
        }
        if (ry <= 0.5f)
        {
            ShadowRect(x,y,w,h,a);
            return;
        }
        if( rx+rx>w ) rx = w*0.5f;
        if( ry+ry>h ) ry = h*0.5f;
        ShadowRectL_R3(x     ,y     ,  rx   ,  ry   ,a,a);
        ShadowRect    (x+  rx,y     ,w-rx-rx,  ry   ,a);
        ShadowRectL_R2(x+w-rx,y     ,  rx   ,  ry   ,a,a);
        ShadowRect    (x     ,y+  ry,w      ,h-ry-ry,a);
        ShadowRectL_R1(x     ,y+h-ry,  rx   ,  ry   ,a,a);
        ShadowRect    (x+  rx,y+h-ry,w-rx-rx,  ry   ,a);
        ShadowRectL_R0(x+w-rx,y+h-ry,  rx   ,  ry   ,a,a);
    }


    VOID ShadowRectAA( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT r )
    {
        if(r>w) r=w;
        if(r>h) r=h;
        const FLOAT r2 = r*0.5f;
        ShadowRect    (x+r2  ,y+r2  ,w-r,h-r,a);
        ShadowRectLV  (x+r2  ,y-r2  ,w-r,  r,0.0f,a);
        ShadowRectL_R2(x-r2+w,y-r2  ,  r,  r,0.0f,a);
        ShadowRectLH  (x-r2+w,y+r2  ,  r,h-r,a,0.0f);
        ShadowRectL_R0(x-r2+w,y-r2+h,  r,  r,0.0f,a);
        ShadowRectLV  (x+r2  ,y-r2+h,w-r,  r,a,0.0f);
        ShadowRectL_R1(x-r2  ,y-r2+h,  r,  r,0.0f,a);
        ShadowRectLH  (x-r2  ,y+r2  ,  r,h-r,0.0f,a);
        ShadowRectL_R3(x-r2  ,y-r2  ,  r,  r,0.0f,a);
    }

    VOID ShadowRectAAR( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT rx, FLOAT ry, FLOAT rr )
    {
        if (rx <= 0.5f)
        {
            ShadowRectAA(x,y,w,h,a,rr);
            return;
        }
        if (ry <= 0.5f)
        {
            ShadowRectAA(x,y,w,h,a,rr);
            return;
        }
        if( rx+rx<rr ) rr = rx*2.0f;
        if( ry+ry<rr ) rr = ry*2.0f;
        if( rx+rx>w-rr ) rx = (w-rr)*0.5f;
        if( ry+ry>h-rr ) ry = (h-rr)*0.5f;
        const FLOAT r2 = rr*0.5f;

        const FLOAT fx0 = x-r2;
        const FLOAT fx1 = x+r2;
        const FLOAT fx2 = x+rx;
        const FLOAT fx3 = x-rx+w;
        const FLOAT fx4 = x-r2+w;
        // const FLOAT fx5 = x+r2+w;

        const FLOAT fy0 = y-r2;
        const FLOAT fy1 = y+r2;
        const FLOAT fy2 = y+ry;
        const FLOAT fy3 = y-ry+h;
        const FLOAT fy4 = y-r2+h;
        // const FLOAT fy5 = y+r2+h;

        ShadowRectL_RR3(fx0,fy0,rx-r2,ry-r2,0.0f,a,rx+r2,ry+r2);
        ShadowRectL_RR2(fx3,fy0,rx-r2,ry-r2,0.0f,a,rx+r2,ry+r2);
        ShadowRectL_RR1(fx0,fy3,rx-r2,ry-r2,0.0f,a,rx+r2,ry+r2);
        ShadowRectL_RR0(fx3,fy3,rx-r2,ry-r2,0.0f,a,rx+r2,ry+r2);

        ShadowRectLV(fx2,fy0,fx3-fx2,rr,0.0f,a);
        ShadowRectLV(fx2,fy4,fx3-fx2,rr,a,0.0f);
        ShadowRectLH(fx0,fy2,rr,fy3-fy2,0.0f,a);
        ShadowRectLH(fx4,fy2,rr,fy3-fy2,a,0.0f);

        ShadowRectL_R3 (fx1,fy1,rx-r2,ry-r2,a,a);
        ShadowRectL_R2 (fx3,fy1,rx-r2,ry-r2,a,a);
        ShadowRectL_R1 (fx1,fy3,rx-r2,ry-r2,a,a);
        ShadowRectL_R0 (fx3,fy3,rx-r2,ry-r2,a,a);

        ShadowRect(fx2,fy1,fx3-fx2,ry-r2,a);
        ShadowRect(fx1,fy2,w-rr,fy3-fy2,a);
        ShadowRect(fx2,fy3,fx3-fx2,ry-r2,a);

    }

    // ShadowRectR( 10.0f, 10.0f, 1000.0f/13.0f, 1000.0f/13.0f, 0.5f, f_x, f_y );
    // ShadowRectAAR( 11.0f, 11.0f, 1000.0f/13.0f-2.0f, 700.0f/13.0f-2.0f, 0.5f, f_x, f_y, 1.0f );

    ShadowRectL_RR0( 1.0f, 1.0f, 8.0f, 8.0f, 0.9f, 0.0f, 100.0f, 766.0f );
    ShadowRectL_RR0( 1.0f, 1.0f, 100.0f, 766.0f, 0.5f, 1.0f, 1024.0f, 768.0f );
    ShadowRectL_RR0( 1.0f, 1.0f,1024.0f, 768.0f, 1.0f, 0.4f, 1300.0f, 1300.0f );

    FOR_ALL_PIXELS_VERTICAL(iy)
    FOR_ALL_PIXELS_HORIZONTAL(ix)
    {
        memset(_pc+(ix+iy*_w),(BYTE)GET_LIGHT(ix,iy),4);
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
        GET_COLOR(x,iy)^=(iy>>2)&1?0x7f<<16:0x7f<<8;
    FOR_ALL_PIXELS_HORIZONTAL(ix)
        GET_COLOR(ix,y)^=(ix>>2)&1?0x7f:0x7f<<8;


}
