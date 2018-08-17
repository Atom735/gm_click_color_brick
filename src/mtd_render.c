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

    const FLOAT R = x*0.5f;
    const FLOAT r = y*0.5f;
    const FLOAT sigma = R/3.0f;
    const FLOAT s2 = 1.0f/(sigma*sqrtf(2.0f));
    const FLOAT fcx = (FLOAT)(_w)*0.5f;

    FOR_(ix,0,_w)
    {
        FLOAT fx = ((FLOAT)ix+4.5f)-fcx;
        FLOAT fy = (erff(fx*s2)*0.5f+0.5f);
        FOR_(iy,0,fy*_h)
        {
            GET_LIGHT(ix,iy) *= 0.5f;
        }
    }

    FOR_(iy,0,_h)
    {
        GET_LIGHT((UINT)(fcx),iy) *= 0.8f;
        GET_LIGHT((UINT)(fcx+R),iy) *= 0.8f;
        GET_LIGHT((UINT)(fcx-R),iy) *= 0.8f;
    }

    // FOR_(iy,0,__min(128,_h))
    // {
    //     const FLOAT fy = (erff((63.5f-(FLOAT)iy)*s2)*0.5f+0.5f);
    //     FOR_(ix,0,__min(128,_w))
    //     {
    //         const FLOAT fx = (erff((63.5f-(FLOAT)ix)*s2)*0.5f+0.5f);
    //         GET_LIGHT(ix,iy) *= 1.0f-(fx*fy);
    //     }
    // }

    // FOR_(iy,128,__min(256,_h))
    // {
    //     const FLOAT fy = (iy-128)/127.0f;
    //     FOR_(ix,0,__min(128,_w))
    //     {
    //         const FLOAT fx = (ix)/127.0f;
    //         GET_LIGHT(ix,iy) *= 1.0f-(fx*fy);
    //     }
    // }

    void ShadowRectangle( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a )
    {
        const UINT ux1 = (UINT)round(x>0.0f?x:0.0f);
        const UINT ux2 = __min((UINT)round(x+w)+1,_w);
        const UINT uy1 = (UINT)round(y>0.0f?y:0.0f);
        const UINT uy2 = __min((UINT)round(y+h)+1,_h);
        FOR_(iy,uy1,uy2)
        FOR_(ix,ux1,ux2)
        {
            GET_LIGHT(ix,iy) *= 1.0f-a;
        }
    }
    void ShadowRectangleLinerHorizontal( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux1 = (UINT)round(x>0.0f?x:0.0f);
        const UINT ux2 = __min((UINT)round(x+w)+1,_w);
        const UINT uy1 = (UINT)round(y>0.0f?y:0.0f);
        const UINT uy2 = __min((UINT)round(y+h)+1,_h);
        FOR_(ix,ux1,ux2)
        {
            const FLOAT fx = (FLOAT)ix+0.5f;
            const FLOAT f = (fx-x)/w;
            FOR_(iy,uy1,uy2)
            {
                GET_LIGHT(ix,iy) *= 1.0f-f*(b-a)-a;
            }
        }
    }
    void ShadowRectangleLinerVertical( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a, FLOAT b )
    {
        const UINT ux1 = (UINT)round(x>0.0f?x:0.0f);
        const UINT ux2 = __min((UINT)round(x+w)+1,_w);
        const UINT uy1 = (UINT)round(y>0.0f?y:0.0f);
        const UINT uy2 = __min((UINT)round(y+h)+1,_h);
        FOR_(iy,uy1,uy2)
        {
            const FLOAT fy = (FLOAT)iy+0.5f;
            const FLOAT f = (fy-y)/h;
            FOR_(ix,ux1,ux2)
            {
                GET_LIGHT(ix,iy) *= 1.0f-f*(b-a)-a;
            }
        }
    }

    void ShadowRectangleA( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT a )
    {
        ShadowRectangle( x+0.5f, y+0.5f, w-1.0f, h-1.0f, a);

        ShadowRectangleLinerHorizontal( x-0.5f  , y+0.5f, 1.0f, h-1.0f, 0.0f, a );
        ShadowRectangleLinerHorizontal( x-0.5f+w, y+0.5f, 1.0f, h-1.0f, a, 0.0f );

        ShadowRectangleLinerVertical  ( x+0.5f, y-0.5f  , w-1.0f, 1.0f, 0.0f, a );
        ShadowRectangleLinerVertical  ( x+0.5f, y-0.5f+h, w-1.0f, 1.0f, a, 0.0f );

        // const FLOAT sa = a*a;

        // ShadowRectangleLinerVertical  ( x-0.5f, y-0.5f, 1.0f, 1.0f, 0.0f, sa );
        // ShadowRectangleLinerHorizontal( x-0.5f, y-0.5f, 1.0f, 1.0f, 0.0f, sa );

        // ShadowRectangleLinerVertical  ( x-0.5f+w, y-0.5f, 1.0f, 1.0f, 0.0f, sa );
        // ShadowRectangleLinerHorizontal( x-0.5f+w, y-0.5f, 1.0f, 1.0f, sa, 0.0f );

        // ShadowRectangleLinerVertical  ( x-0.5f, y-0.5f+h, 1.0f, 1.0f, sa, 0.0f );
        // ShadowRectangleLinerHorizontal( x-0.5f, y-0.5f+h, 1.0f, 1.0f, 0.0f, sa );

        // ShadowRectangleLinerVertical  ( x-0.5f+w, y-0.5f+h, 1.0f, 1.0f, sa, 0.0f );
        // ShadowRectangleLinerHorizontal( x-0.5f+w, y-0.5f+h, 1.0f, 1.0f, sa, 0.0f );

    }

    // ShadowRectangleA( 260.0f+R*0.1f, 260.0f+r*0.1f, 5.0f, 5.0f, 0.9f );

    ShadowRectangle( 500.0f+R*0.1f, 500.0f+r*0.1f, 2.0f, 2.0f, 0.9f );

    ShadowRectangleA( 261.0f+R*0.1f, 261.0f+r*0.1f, 2.0f, 2.0f, 0.9f );


    FOR_(iy,0,__min(128,_h))
    {
        const FLOAT fy = ((FLOAT)iy-63.5f);
        const FLOAT gy = (fy<=0.0f)?1.0f:0.0f;
        FOR_(ix,0,__min(128,_w))
        {
            const FLOAT fx = ((FLOAT)ix-63.5f);
            const FLOAT gx = (fx<=0.0f)?1.0f:0.0f;
            GET_LIGHT(ix,iy) *= 1.0f-(gy*gx);
        }
    }

    FOR_(iy,0,__min(128,_h))
    {
        const FLOAT fy = ((FLOAT)iy-63.5f);
        const FLOAT gy = (fy<=0.0f)?1.0f:0.0f;
        FOR_(ix,128,__min(128*2,_w))
        {
            const FLOAT fx = ((FLOAT)ix-63.5f-128.0f);
            const FLOAT gx = 0.5f-erff(fx*s2)*0.5f;
            GET_LIGHT(ix,iy) *= 1.0f-(gy*gx);
        }
    }

    FOR_(iy,128,__min(128*2,_h))
    {
        const FLOAT fy = ((FLOAT)iy-63.5f-128.0f);
        const FLOAT gy = 0.5f-erff(fy*s2)*0.5f;
        FOR_(ix,0,__min(128,_w))
        {
            const FLOAT fx = ((FLOAT)ix-63.5f);
            const FLOAT gx = (fx<=0.0f)?1.0f:0.0f;
            GET_LIGHT(ix,iy) *= 1.0f-(gy*gx);
        }
    }

    FOR_(iy,128,__min(128*2,_h))
    {
        const FLOAT fy = ((FLOAT)iy-63.5f-128.0f);
        const FLOAT gy = 0.5f-erff(fy*s2)*0.5f;
        FOR_(ix,128,__min(128*2,_w))
        {
            const FLOAT fx = ((FLOAT)ix-63.5f-128.0f);
            const FLOAT gx = 0.5f-erff(fx*s2)*0.5f;
            GET_LIGHT(ix,iy) *= 1.0f-(gy*gx);
        }
    }

    FOR_(iy,0,_h)
    {
        GET_LIGHT((UINT)(fcx),iy) *= 0.8f;
        GET_LIGHT((UINT)(fcx+R),iy) *= 0.8f;
        GET_LIGHT((UINT)(fcx-R),iy) *= 0.8f;
    }

    FOR_ALL_PIXELS(i) memset(_pc+i,(BYTE)GET_LIGHT_(i),4);


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

        TEXTOUT(0,"R = %f", R);
        TEXTOUT(1,"r = %f", r);
        TEXTOUT(2,"Sigma = %f", sigma);
    }
    SelectFont( p->bmp_hDC, p->bmp_hFont_Old );


}
#if 0
{
    assert( p != NULL );
    if( p->w == 0 ) return;
    if( p->h == 0 ) return;

    const UINT      _w  = p->w;
    const UINT      _h  = p->h;
    const UINT      _wh = _w*_h;
    UINT32 * const  _pc = p->bmp_pvBits;
    FLOAT * const   _pl = p->bmp_pvLights;

    memset(_pc,0,_wh*4);

    const UINT32 _COLOR_BACKGROUND = 0xB0BEC5;
    const UINT32 _COLOR_CARD_BACKGROUND = 0xECEFF1;

    #define FOR_(_I,_A,_B) for( UINT _I = _A; _I < _B; ++_I )
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


    // FOR_ALL_PIXELS(i)
    // {
    //     GET_COLOR_(i)=_COLOR_BACKGROUND;
    //     // GET_LIGHT_(i)=1.0f;
    // }

    BitBlt( p->bmp_hDC, 0, 0, p->w, p->h, p->_bmp_hDC, 0, 0, SRCCOPY );

    FLOAT sigma = (FLOAT)x*0.05f;
    FLOAT s2 = -0.5f/(sigma*sigma);
    FLOAT s0 = (FLOAT)0.3989422804f/(sigma);
    const UINT R = (UINT)ceilf(3.0f*sigma);

    FLOAT _Gauss( FLOAT f )
    {
        return expf(s2*f*f)*s0;
    }

    if(R > 1)
    {
        FLOAT gKernel[R];
        FOR_(i,1,R) gKernel[i]=expf(s2*(FLOAT)(i*i))*s0;
        FOR_ALL_PIXELS_VERTICAL(iy)
        {
            FLOAT _cR[_w];
            FLOAT _cG[_w];
            FLOAT _cB[_w];
            FOR_ALL_PIXELS_HORIZONTAL(ix)
            {
                _cR[ix] = (FLOAT)(BYTE)(GET_COLOR(ix,iy)>>16);
                _cG[ix] = (FLOAT)(BYTE)(GET_COLOR(ix,iy)>> 8);
                _cB[ix] = (FLOAT)(BYTE)(GET_COLOR(ix,iy)    );
            }
            FOR_ALL_PIXELS_HORIZONTAL(ix)
            {
                FLOAT _R = s0*_cR[ix];
                FLOAT _G = s0*_cG[ix];
                FLOAT _B = s0*_cB[ix];
                FOR_(i,1,R)
                {
                    const FLOAT _ = gKernel[i];
                    if(_<0.001f) break;
                    if(ix+i<_w)
                    {
                        _R += _*_cR[(ix+i)];
                        _G += _*_cG[(ix+i)];
                        _B += _*_cB[(ix+i)];
                    }
                    else
                    {
                        _R += _*_cR[(_w-1)];
                        _G += _*_cG[(_w-1)];
                        _B += _*_cB[(_w-1)];
                    }
                    if(ix-i<_w)
                    {
                        _R += _*_cR[(ix-i)];
                        _G += _*_cG[(ix-i)];
                        _B += _*_cB[(ix-i)];
                    }
                    else
                    {
                        _R += _*_cR[0];
                        _G += _*_cG[0];
                        _B += _*_cB[0];
                    }
                }
                if(_R>255.2f) _R=255.2f;
                if(_G>255.2f) _G=255.2f;
                if(_B>255.2f) _B=255.2f;
                GET_COLOR(ix,iy) = (((UINT32)(BYTE)_R)<<16)|(((UINT32)(BYTE)_G)<<8)|(((UINT32)(BYTE)_B));
            }
        }

        FOR_ALL_PIXELS_HORIZONTAL(ix)
        {
            FLOAT _cR[_h];
            FLOAT _cG[_h];
            FLOAT _cB[_h];
            FOR_ALL_PIXELS_VERTICAL(iy)
            {
                _cR[iy] = (FLOAT)(BYTE)(GET_COLOR(ix,iy)>>16);
                _cG[iy] = (FLOAT)(BYTE)(GET_COLOR(ix,iy)>> 8);
                _cB[iy] = (FLOAT)(BYTE)(GET_COLOR(ix,iy)    );
            }
            FOR_ALL_PIXELS_VERTICAL(iy)
            {
                FLOAT _R = s0*_cR[iy];
                FLOAT _G = s0*_cG[iy];
                FLOAT _B = s0*_cB[iy];
                FOR_(i,1,R)
                {
                    const FLOAT _ = gKernel[i];
                    if(_<0.001f) break;
                    if(iy+i<_h)
                    {
                        _R += _*_cR[(iy+i)];
                        _G += _*_cG[(iy+i)];
                        _B += _*_cB[(iy+i)];
                    }
                    else
                    {
                        _R += _*_cR[(_h-1)];
                        _G += _*_cG[(_h-1)];
                        _B += _*_cB[(_h-1)];
                    }
                    if(iy-i<_h)
                    {
                        _R += _*_cR[(iy-i)];
                        _G += _*_cG[(iy-i)];
                        _B += _*_cB[(iy-i)];
                    }
                    else
                    {
                        _R += _*_cR[0];
                        _G += _*_cG[0];
                        _B += _*_cB[0];
                    }
                }
                if(_R>255.2f) _R=255.2f;
                if(_G>255.2f) _G=255.2f;
                if(_B>255.2f) _B=255.2f;
                GET_COLOR(ix,iy) = (((UINT32)(BYTE)_R)<<16)|(((UINT32)(BYTE)_G)<<8)|(((UINT32)(BYTE)_B));
            }
        }
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

        TEXTOUT(0,"Sigma = %f", sigma);
        TEXTOUT(1,"S2 = %f", s2);
        TEXTOUT(2,"S0 = %f", s0);
        TEXTOUT(3,"R = %i", R);
    }
    SelectFont( p->bmp_hDC, p->bmp_hFont_Old );

}

#if 0

$mdc-elevation-baseline-color: black;
$mdc-elevation-umbra-opacity: .2;
$mdc-elevation-penumbra-opacity: .14;
$mdc-elevation-ambient-opacity: .12;

$mdc-elevation-umbra-map: (
  0: "0px 0px 0px 0px",
  1: "0px 2px 1px -1px",
  2: "0px 3px 1px -2px",
  3: "0px 3px 3px -2px",
  4: "0px 2px 4px -1px",
  5: "0px 3px 5px -1px",
  6: "0px 3px 5px -1px",
  7: "0px 4px 5px -2px",
  8: "0px 5px 5px -3px",
  9: "0px 5px 6px -3px",
  10: "0px 6px 6px -3px",
  11: "0px 6px 7px -4px",
  12: "0px 7px 8px -4px",
  13: "0px 7px 8px -4px",
  14: "0px 7px 9px -4px",
  15: "0px 8px 9px -5px",
  16: "0px 8px 10px -5px",
  17: "0px 8px 11px -5px",
  18: "0px 9px 11px -5px",
  19: "0px 9px 12px -6px",
  20: "0px 10px 13px -6px",
  21: "0px 10px 13px -6px",
  22: "0px 10px 14px -6px",
  23: "0px 11px 14px -7px",
  24: "0px 11px 15px -7px"
);

$mdc-elevation-penumbra-map: (
  0: "0px 0px 0px 0px",
  1: "0px 1px 1px 0px",
  2: "0px 2px 2px 0px",
  3: "0px 3px 4px 0px",
  4: "0px 4px 5px 0px",
  5: "0px 5px 8px 0px",
  6: "0px 6px 10px 0px",
  7: "0px 7px 10px 1px",
  8: "0px 8px 10px 1px",
  9: "0px 9px 12px 1px",
  10: "0px 10px 14px 1px",
  11: "0px 11px 15px 1px",
  12: "0px 12px 17px 2px",
  13: "0px 13px 19px 2px",
  14: "0px 14px 21px 2px",
  15: "0px 15px 22px 2px",
  16: "0px 16px 24px 2px",
  17: "0px 17px 26px 2px",
  18: "0px 18px 28px 2px",
  19: "0px 19px 29px 2px",
  20: "0px 20px 31px 3px",
  21: "0px 21px 33px 3px",
  22: "0px 22px 35px 3px",
  23: "0px 23px 36px 3px",
  24: "0px 24px 38px 3px"
);

$mdc-elevation-ambient-map: (
  0: "0px 0px 0px 0px",
  1: "0px 1px 3px 0px",
  2: "0px 1px 5px 0px",
  3: "0px 1px 8px 0px",
  4: "0px 1px 10px 0px",
  5: "0px 1px 14px 0px",
  6: "0px 1px 18px 0px",
  7: "0px 2px 16px 1px",
  8: "0px 3px 14px 2px",
  9: "0px 3px 16px 2px",
  10: "0px 4px 18px 3px",
  11: "0px 4px 20px 3px",
  12: "0px 5px 22px 4px",
  13: "0px 5px 24px 4px",
  14: "0px 5px 26px 4px",
  15: "0px 6px 28px 5px",
  16: "0px 6px 30px 5px",
  17: "0px 6px 32px 5px",
  18: "0px 7px 34px 6px",
  19: "0px 7px 36px 6px",
  20: "0px 8px 38px 7px",
  21: "0px 8px 40px 7px",
  22: "0px 8px 42px 7px",
  23: "0px 9px 44px 8px",
  24: "0px 9px 46px 8px"
);
#endif
#endif


