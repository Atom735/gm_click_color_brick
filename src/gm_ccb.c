#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dbg.h"
#include "gm_ccb.h"


#define DGM_CCS_SETTINGS_FILE_NAME "gm_ccs.settings.bin"
#define DGM_CCS_SETTINGS_COLORS_N (2+(15*2))


typedef struct gm_ccs_settings {
    UINT gw;    /* grid width */
    UINT gh;    /* grid height */
    UINT gn;    /* grid number */
    UINT rg;    /* rules for building groups */
    UINT sr;    /* scoring rule */
    UINT rd;    /* rules for the destruction of groups and
        penalties for the remaining single groups */
    UINT nc;    /* number of colors */
    UINT32 pc[DGM_CCS_SETTINGS_COLORS_N]; /* pointer to colors */
} GM_CCS_SETTINGS;
/* rules for building groups */
enum {
    GM_CCS_SETTINGS_RG_CROSS = 0,
    GM_CCS_SETTINGS_RG_NEIGHBORS,
    GM_CCS_SETTINGS_RG___LAST,
};
/* scoring rule */
enum {
    GM_CCS_SETTINGS_SR_CLASSIC = 0,
    GM_CCS_SETTINGS_SR_ARI_PRO,
    GM_CCS_SETTINGS_SR___LAST,
};
/* rules for the destruction of groups and
        penalties for the remaining single groups */
enum {
    GM_CCS_SETTINGS_RD_PEN_2N = 0,
    GM_CCS_SETTINGS_RD_PEN_5N,
    GM_CCS_SETTINGS_RD_PEN_N_NP1,
    GM_CCS_SETTINGS_RD_YCDSG,
    GM_CCS_SETTINGS_RD___LAST,
};

#define GM_CCS_SETTINGS_NC___LAST 16
#define GM_CCS_SETTINGS_GW___LAST (1<<8)
#define GM_CCS_SETTINGS_GH___LAST (1<<8)
#define GM_CCS_SETTINGS_GN___LAST (1<<24)


static BOOL _gm_CCS_Settings8BytesGet( GM_CCS_SETTINGS * p, BYTE const * h )
{
    assert( p != NULL );
    assert( h != NULL );
    if( ( h[6] != (h[0]+h[1]+h[2]+h[3]+h[4]+h[5]) ) || ( h[7] != (h[0]^h[1]^h[2]^h[3]^h[4]^h[5]^h[6]) ) )
    { /* data struct uncorect */
        return FALSE;
    }
    p->rg   = (h[0]>>7)&0x1;
    p->sr   = (h[0]>>6)&0x1;
    p->rd   = (h[0]>>4)&0x3;
    p->nc   = (h[0]>>0)&0x7;
    p->gn   = (h[1]<<16)|(h[2]<<8)|(h[3]<<0);
    p->gw   = h[4];
    p->gh   = h[5];
    return TRUE;
}
static void _gm_CCS_Settings8BytesSet( GM_CCS_SETTINGS const * p, BYTE * h )
{
    assert( p != NULL );
    assert( h != NULL );
    assert( p->rg < GM_CCS_SETTINGS_RG___LAST );
    assert( p->sr < GM_CCS_SETTINGS_SR___LAST );
    assert( p->rd < GM_CCS_SETTINGS_RD___LAST );
    assert( p->nc > 1 );
    assert( p->nc < GM_CCS_SETTINGS_NC___LAST );
    assert( p->gw > 1 );
    assert( p->gw < GM_CCS_SETTINGS_GW___LAST );
    assert( p->gh > 1 );
    assert( p->gh < GM_CCS_SETTINGS_GH___LAST );
    assert( p->gn < GM_CCS_SETTINGS_GN___LAST );
    memset( h, 0, 8 );
    h[0] = ( ((p->rg&0x1)<<7) | ((p->sr&0x1)<<6) | ((p->rd&0x3)<<4) | ((p->nc&0x7)<<0) );
    h[1] = p->gn>>16;
    h[2] = p->gn>>8;
    h[3] = p->gn>>0;
    h[4] = p->gw;
    h[5] = p->gh;
    h[6] = h[0]+h[1]+h[2]+h[3]+h[4]+h[5];
    h[7] = h[0]^h[1]^h[2]^h[3]^h[4]^h[5]^h[6];
}
static void _gm_CCS_SettingsDefaultColors( UINT32 * p )
{
    assert( p != NULL );
    p[0] = 0xFF37474F;
    p[1] = 0xFF263238;
    p[2] = 0xFFCFD8DC;
    p[4] = 0xFFff0000;
    p[5] = 0xFF0000ff;
    p[6] = 0xFF00ff00;
    p[7] = 0xFF7f00ff;
}
static void _gm_CCS_SettingsDefault( GM_CCS_SETTINGS * p )
{
    assert( p != NULL );
    p->gw = 16;
    p->gh = 10;
    p->gn = 1;
    p->rg = GM_CCS_SETTINGS_RG_CROSS;
    p->sr = GM_CCS_SETTINGS_SR_CLASSIC;
    p->rd = GM_CCS_SETTINGS_RD_PEN_2N;
    p->nc = 4;
    _gm_CCS_SettingsDefaultColors( p->pc );


    p->gw = 4;
    p->gh = 3;
}
static void _gm_CCS_SettingsSave( GM_CCS_SETTINGS const * p )
{
    assert( p != NULL );
    FILE *pF = fopen( DGM_CCS_SETTINGS_FILE_NAME, "wb" );
    if ( pF == NULL )
    { /* open fail */
        return;
    }
    {
        BYTE h[8];
        _gm_CCS_Settings8BytesSet( p, h );
        fwrite( h , 1, 8, pF);
        fwrite( p->pc, 4, DGM_CCS_SETTINGS_COLORS_N, pF );
    }
    fclose( pF );
}
static void _gm_CCS_SettingsLoad( GM_CCS_SETTINGS * p )
{
    assert( p != NULL );
    // FILE *pF = fopen( DGM_CCS_SETTINGS_FILE_NAME, "rb" );
    FILE *pF = NULL;
    if ( pF == NULL )
    { /* open fail */
        _gm_CCS_SettingsDefault( p );
        _gm_CCS_SettingsSave( p );
        return;
    }
    UINT bsz;
    {
        BYTE h[8];
        if( ( bsz = fread( &h, 1, 8, pF ) ) != 8 )
        { /* read fail */
            _gm_CCS_SettingsDefault( p );
            _gm_CCS_SettingsSave( p );
            fclose( pF );
            return;
        }
        if( ! _gm_CCS_Settings8BytesGet( p, h ) )
        { /* data struct uncorect */
            _gm_CCS_SettingsDefault( p );
            _gm_CCS_SettingsSave( p );
            fclose( pF );
            return;
        }
    }
    if( ( bsz = fread( p->pc, 4, DGM_CCS_SETTINGS_COLORS_N, pF ) ) != (4*DGM_CCS_SETTINGS_COLORS_N) )
    { /* read color fail */
        _gm_CCS_SettingsDefaultColors( p->pc );
        _gm_CCS_SettingsSave( p );
    }
    fclose( pF );
    return;
}

struct gm_ccs
{
    UINT8  *pGrid;
    UINT    nScores;
    UINT    nBlocks;
    UINT    nCurX;
    UINT    nCurY;
    UINT    nCurN;
    GM_CCS_SETTINGS s;
};


GM_CCS* gm_ccs_Alloc()
{
    GM_CCS * p = (GM_CCS*)malloc( sizeof(GM_CCS) );
    assert( p != NULL );
    memset( p, 0, sizeof(GM_CCS) );
    _gm_CCS_SettingsLoad( &(p->s) );
    return p;
}
VOID    gm_ccs_Free( GM_CCS * p )
{
    assert( p != NULL );
    _gm_CCS_SettingsSave( &(p->s) );
    if( p->pGrid != NULL )
        free( p->pGrid );
    free( p );
}
VOID    gm_ccs_NewGame( GM_CCS * p )
{
    assert( p != NULL );
    assert( p->s.rg < GM_CCS_SETTINGS_RG___LAST );
    assert( p->s.sr < GM_CCS_SETTINGS_SR___LAST );
    assert( p->s.rd < GM_CCS_SETTINGS_RD___LAST );
    assert( p->s.nc > 1 );
    assert( p->s.nc < GM_CCS_SETTINGS_NC___LAST );
    assert( p->s.gw > 1 );
    assert( p->s.gw < GM_CCS_SETTINGS_GW___LAST );
    assert( p->s.gh > 1 );
    assert( p->s.gh < GM_CCS_SETTINGS_GH___LAST );
    assert( p->s.gn < GM_CCS_SETTINGS_GN___LAST );
    // ++p->s.gn;
    if( p->pGrid != NULL )
        free( p->pGrid );
    const UINT _sz = p->s.gw*p->s.gh;
    p->pGrid = (UINT8*)malloc( _sz );
    assert( p->pGrid != NULL );
    UINT k = p->s.gn;
    for( UINT i=0; i<_sz; ++i )
        p->pGrid[i]=(((k=__rand(k,9719,1709))%(15*7))%p->s.nc);
}

struct render
{
    UINT w;
    UINT h;

    HDC bmp_hDC;
    PUINT32 bmp_pvBits;
    HBITMAP bmp_hBMP;
    HBITMAP bmp_hBMP_Old;
    HFONT bmp_hFont;
    HFONT bmp_hFont_Old;
};

static void _render_gdi_Free( RENDER * p )
{
    assert( p != NULL );
    if( p->bmp_hDC != NULL )
    {
        DeleteFont( p->bmp_hFont );
        SelectBitmap( p->bmp_hDC, p->bmp_hBMP_Old );
        DeleteBitmap( p->bmp_hBMP );
        DeleteDC( p->bmp_hDC );
    }
}


RENDER* render_Alloc()
{
    RENDER * p = (RENDER*)malloc( sizeof(RENDER) );
    assert( p != NULL );
    memset( p, 0, sizeof(RENDER) );
    return p;
}
VOID    render_Free( RENDER * p )
{
    assert( p != NULL );
    _render_gdi_Free( p );
    free( p );
}
VOID    render_GDI_Create( RENDER * p, HDC hDC, UINT w, UINT h )
{
    assert( p != NULL );
    assert( hDC != NULL );
    assert( w >= 1 );
    assert( h >= 1 );
    _render_gdi_Free( p );
    p->bmp_hDC = CreateCompatibleDC( hDC );
    assert( p->bmp_hDC != NULL );
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = p->w = w;
    bmi.bmiHeader.biHeight      = p->h = h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;
    p->bmp_hBMP = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
        (PVOID*)(&p->bmp_pvBits), NULL, 0x0 );
    assert( p->bmp_hBMP != NULL );
    assert( p->bmp_pvBits != NULL );
    p->bmp_hBMP_Old = SelectBitmap( p->bmp_hDC, p->bmp_hBMP );

    LOGFONTA lf;
    lf.lfHeight         = -18;
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_REGULAR;
    lf.lfItalic         = FALSE;
    lf.lfUnderline      = FALSE;
    lf.lfStrikeOut      = FALSE;
    lf.lfCharSet        = ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    memcpy( lf.lfFaceName, "Roboto Slab", 12 );
    p->bmp_hFont = CreateFontIndirectA( &lf );
}

VOID    render_Flush( RENDER * p, HDC hDC )
{
    assert( p != NULL );
    assert( p->w >= 1 );
    assert( p->h >= 1 );
    assert( hDC != NULL );
    BitBlt( hDC, 0, 0, p->w, p->h, p->bmp_hDC, 0, 0, SRCCOPY );
}

VOID render_draw_Point( RENDER * p, UINT x, UINT y, UINT32 c )
{
    assert( p != NULL );
    assert( p->w >= 1 );
    assert( p->h >= 1 );
    if(x >= p->w) return;
    if(y >= p->h) return;
    if((c&0xff000000)==0xff000000)
        p->bmp_pvBits[y*p->w+x] = c;
    else
    {
        const UINT a = c>>24;
        const UINT b = 0xff-a;
        const UINT32 o = p->bmp_pvBits[y*p->w+x];
        p->bmp_pvBits[y*p->w+x] =
            ((((((c>>16)&0xff)*a)+(((o>>16)&0xff)*b))/0xff)<<16)|
            ((((((c>> 8)&0xff)*a)+(((o>> 8)&0xff)*b))/0xff)<< 8)|
            ((((((c>> 0)&0xff)*a)+(((o>> 0)&0xff)*b))/0xff)<< 0);
    }
}

VOID render_draw_Rectangle( RENDER * p, FLOAT x, FLOAT y, UINT32 c, FLOAT w, FLOAT h, FLOAT r, FLOAT r2 )
{
    assert( p != NULL );
    assert( p->w >= 1 );
    assert( p->h >= 1 );

    UINT32 * const _pvb = p->bmp_pvBits;
    const UINT _w = p->w;
    const UINT _h = p->h;
    const UINT _mul = 8;
    const FLOAT _Nmul = 1.0f/(FLOAT)_mul;
    x *= _Nmul;
    y *= _Nmul;
    w *= _Nmul;
    h *= _Nmul;
    r *= _Nmul;
    r2 *= _Nmul;

    const FLOAT _src_a = (FLOAT)((c>>24)&0xff);
    const FLOAT _src_r = (FLOAT)((c>>16)&0xff);
    const FLOAT _src_g = (FLOAT)((c>> 8)&0xff);
    const FLOAT _src_b = (FLOAT)((c>> 0)&0xff);

    void point( UINT x, UINT y, FLOAT a )
    {
        const FLOAT _a = 1.0f-a;

        for( UINT ky=y*_mul; ky<(y+1)*_mul; ++ky )
        for( UINT kx=x*_mul; kx<(x+1)*_mul; ++kx )
        {
            const FLOAT _dst_a = (FLOAT)((_pvb[ky*_w+kx]>>24)&0xff)*_a;
            const FLOAT _dst_r = (FLOAT)((_pvb[ky*_w+kx]>>16)&0xff)*_a;
            const FLOAT _dst_g = (FLOAT)((_pvb[ky*_w+kx]>> 8)&0xff)*_a;
            const FLOAT _dst_b = (FLOAT)((_pvb[ky*_w+kx]>> 0)&0xff)*_a;
            const UINT __a = (((UINT)(_dst_a+_src_a*a))&0xff)<<24;
            const UINT __r = (((UINT)(_dst_r+_src_r*a))&0xff)<<16;
            const UINT __g = (((UINT)(_dst_g+_src_g*a))&0xff)<< 8;
            const UINT __b = (((UINT)(_dst_b+_src_b*a))&0xff)<< 0;
            _pvb[ky*_w+kx] = __a|__r|__g|__b;
        }
    }

    const FLOAT x0 = x;
    const FLOAT x1 = x+r;
    const FLOAT x2 = x+w-r;
    const FLOAT x3 = x+w;

    const FLOAT y0 = y;
    const FLOAT y1 = y+r;
    const FLOAT y2 = y+h-r;
    const FLOAT y3 = y+h;

    const UINT ux0 = (UINT)__min(ceilf(x0),(FLOAT)_w*_Nmul);
    const UINT ux1 = (UINT)__min(ceilf(x1),(FLOAT)_w*_Nmul);
    const UINT ux2 = (UINT)__min(ceilf(x2),(FLOAT)_w*_Nmul);
    const UINT ux3 = (UINT)__min(ceilf(x3),(FLOAT)_w*_Nmul);

    const UINT uy0 = (UINT)__min(ceilf(y0),(FLOAT)_h*_Nmul);
    const UINT uy1 = (UINT)__min(ceilf(y1),(FLOAT)_h*_Nmul);
    const UINT uy2 = (UINT)__min(ceilf(y2),(FLOAT)_h*_Nmul);
    const UINT uy3 = (UINT)__min(ceilf(y3),(FLOAT)_h*_Nmul);

    const FLOAT _rs0 = (r-r2)*(r-r2);
    const FLOAT _rs1 = r*r;
    const FLOAT _Nr = 1.0f/r2;
    UINT uy=uy0;
    for(;uy<uy1;++uy)
    {
        const FLOAT _y = y1-((FLOAT)uy+0.5f);
        UINT ux=ux0;
        for(;ux<ux1;++ux)
        {
            const FLOAT _x = x1-((FLOAT)ux+0.5f);
            const FLOAT _rs = (_x*_x)+(_y*_y);
            if( _rs<=_rs0 )
            {
                point( ux, uy, 1.0f );
            }
            else
            if( _rs<_rs1 )
            {
                point( ux, uy, (sqrtf(_rs1)-sqrtf(_rs))*_Nr );
            }
        }
        if(_y < r-r2)
        {
            for(;ux<ux2;++ux)
            {
                point( ux, uy, 1.0f );
            }
        }
        else
        {
            for(;ux<ux2;++ux)
            {
                point( ux, uy, (r-_y)*_Nr );
            }
        }
        for(;ux<ux3;++ux)
        {
            const FLOAT _x = ((FLOAT)ux+0.5f)-x2;
            const FLOAT _rs = (_x*_x)+(_y*_y);
            if( _rs<=_rs0 )
            {
                point( ux, uy, 1.0f );
            }
            else
            if( _rs<_rs1 )
            {
                point( ux, uy, (sqrtf(_rs1)-sqrtf(_rs))*_Nr );
            }
        }
    }
    for(;uy<uy2;++uy)
    {
        UINT ux=ux0;

        const UINT ux1 = (UINT)__min(ceilf(x0+r2),(FLOAT)_w*_Nmul);
        const UINT ux2 = (UINT)__min(ceilf(x3-r2),(FLOAT)_w*_Nmul);
        for(;ux<ux1;++ux)
        {
            const FLOAT _x = fabsf(x1-((FLOAT)ux+0.5f));
            point( ux, uy, (r-_x)*_Nr );
        }
        for(;ux<ux2;++ux)
        {
            point( ux, uy, 1.0f );
        }
        for(;ux<ux3;++ux)
        {
            const FLOAT _x = fabsf(((FLOAT)ux+0.5f)-x2);
            point( ux, uy, (r-_x)*_Nr );
        }
    }
    for(;uy<uy3;++uy)
    {
        const FLOAT _y = ((FLOAT)uy+0.5f)-y2;
        UINT ux=ux0;
        for(;ux<ux1;++ux)
        {
            const FLOAT _x = x1-((FLOAT)ux+0.5f);
            const FLOAT _rs = (_x*_x)+(_y*_y);
            if( _rs<=_rs0 )
            {
                point( ux, uy, 1.0f );
            }
            else
            if( _rs<_rs1 )
            {
                point( ux, uy, (sqrtf(_rs1)-sqrtf(_rs))*_Nr );
            }
        }
        if(_y < r-r2)
        {
            for(;ux<ux2;++ux)
            {
                point( ux, uy, 1.0f );
            }
        }
        else
        {
            for(;ux<ux2;++ux)
            {
                point( ux, uy, (r-_y)*_Nr );
            }
        }
        for(;ux<ux3;++ux)
        {
            const FLOAT _x = ((FLOAT)ux+0.5f)-x2;
            const FLOAT _rs = (_x*_x)+(_y*_y);
            if( _rs<=_rs0 )
            {
                point( ux, uy, 1.0f );
            }
            else
            if( _rs<_rs1 )
            {
                point( ux, uy, (sqrtf(_rs1)-sqrtf(_rs))*_Nr );
            }
        }
    }
    #if 0
    for( UINT uy=uy0; uy<uy3; ++uy )
    {
        FLOAT ay = 0.0f;
        if( uy<uy1 ) ay = 1.0f-(y1-(FLOAT)uy)/r;
        else
        if( uy<uy2 ) ay = 1.0f;
        else
            ay = (y1-(FLOAT)uy)/r;
        UINT ux = ux0;
        for(; ux<ux1; ++ux )
        {
            const FLOAT ax = 1.0f-(x1-(FLOAT)ux)/r;
            point( ux, uy, sqrtf((ax*ax)+(ay*ay)) );
        }
        for(; ux<ux2; ++ux )
        {
            point( ux, uy, ay );
        }
        for(; ux<ux3; ++ux )
        {
            const FLOAT ax = (x1-(FLOAT)ux)/r;
            point( ux, uy, sqrtf((ax*ax)+(ay*ay)) );
        }
    }
    #endif


}

VOID    render_gm_ccs( RENDER * p, GM_CCS * gm )
{
    assert( p != NULL );
    assert( p->w >= 1 );
    assert( p->h >= 1 );
    assert( gm != NULL );

    UINT32 * const _pvb = p->bmp_pvBits;
    const UINT _w = p->w;
    const UINT _h = p->h;

    static UINT u=0; ++u;
    for( UINT y=0; y<_h; ++y )
    for( UINT x=0; x<_w; ++x ) _pvb[y*_w+x]=0;
        // _pvb[y*_w+x] = ((((y+u)&x)&0x3f)<<16)|(((y|(x+u))&0x3f)<<8)|((((y+u)^(x+u))&0x3f));

    // render_draw_Rectangle( p, 16.0f, 16.0f, 0xffff00ff, 32.0f, 32.0f, 0.0f );

    #if 0


    UINT32 * const _pvb = p->bmp_pvBits;
    const UINT _w = p->w;
    const UINT _h = p->h;
    for( UINT y=0; y<_h; ++y )
    for( UINT x=0; x<_w; ++x )
        _pvb[y*_w+x] = ((((y+u)&x)&0xff)<<16)|(((y|(x+u))&0xff)<<8)|((((y+u)^(x+u))&0xff));
    ++u;

    const UINT _gw = gm->s.gw;
    const UINT _gh = gm->s.gh;
    const UINT32 * _pc = gm->s.pc;
    const FLOAT _pW = (FLOAT)_w/(FLOAT)_gw;
    const FLOAT _pH = (FLOAT)_h/(FLOAT)_gh;
    const FLOAT _pA = __min(_pW,_pH);

    for( UINT ix=0; ix<_gw; ++ix )
    for( UINT iy=0; iy<_gh; ++iy )
    {
        const UINT _gd = gm->pGrid[ix*_gh+iy];
        const FLOAT _r = 0.1f*_pA;
        const FLOAT _fX0 = _pA*(FLOAT)(ix+0);
        const FLOAT _fY0 = _pA*(FLOAT)(iy+0);
        const FLOAT _fX1 = _pA*(FLOAT)(ix+1);
        const FLOAT _fY1 = _pA*(FLOAT)(iy+1);
        {
            const UINT _ux = __min((UINT)floorf(_fX1-_r), _w);
            const UINT _uy = __min((UINT)floorf(_fY1-_r), _h);
            for( UINT x=(UINT)ceilf(_fX0+_r); x<_ux; ++x )
            for( UINT y=(UINT)ceilf(_fY0+_r); y<_uy; ++y )
                _pvb[y*_w+x] = _pc[_gd];
        }

        {
            const UINT _ux = __min((UINT)ceilf(_fX0+_r), _w);
            const UINT _uy = __min((UINT)ceilf(_fY0+_r), _h);
            for( UINT x=(UINT)floorf(_fX0); x<_ux; ++x )
            for( UINT y=(UINT)floorf(_fY0); y<_uy; ++y )
            {
                const FLOAT _x = (FLOAT)x-_fX0-_r;
                const FLOAT _y = (FLOAT)y-_fY0-_r;
                const FLOAT _rq = (_x*_x)+(_y*_y);
                if( _rq <= _r*_r )
                {
                    _pvb[y*_w+x] = _pc[_gd];
                }
                else
                if( _rq < (_r+1.0f)*(_r+1.0f) )
                {
                    const FLOAT _a = _r+1.0f-sqrtf(_rq);
                    const FLOAT _dst_a = ((_pvb[y*_w+x]>>24)&0xff);
                    const FLOAT _dst_r = ((_pvb[y*_w+x]>>16)&0xff);
                    const FLOAT _dst_g = ((_pvb[y*_w+x]>> 8)&0xff);
                    const FLOAT _dst_b = ((_pvb[y*_w+x]>> 0)&0xff);
                    const FLOAT _src_a = ((_pc[_gd]>>24)&0xff);
                    const FLOAT _src_r = ((_pc[_gd]>>16)&0xff);
                    const FLOAT _src_g = ((_pc[_gd]>> 8)&0xff);
                    const FLOAT _src_b = ((_pc[_gd]>> 0)&0xff);

                    const UINT __a = (UINT)((_src_a*_a) + _dst_a - (_dst_a*_a));
                    const UINT __r = (UINT)((_src_r*_a) + _dst_r - (_dst_r*_a));
                    const UINT __g = (UINT)((_src_g*_a) + _dst_g - (_dst_g*_a));
                    const UINT __b = (UINT)((_src_b*_a) + _dst_b - (_dst_b*_a));

                    _pvb[y*_w+x] = (__a<<24)|(__r<<16)|(__g<<8)|(__b<<0);
                }
            }


            // render_DrawRect( p, _fX0, _fY0, _pA, _pA, 0x7f7f7f, _r );
        }


    }
    #endif
}

#define __rand__a   9719
#define __rand__c   1709


typedef struct gm_CCB
{
    UINT random_seed;
    UINT n_bricks;
    UINT n_scores;
    BOOL b_end;
    struct {
        UINT x, y, w, h, go, gh;
    } VP;
    struct {
        UINT x, y, w, h, n;
        PUINT8 p;
    } Grid;
    struct {
        UINT n, m;
        PUINT32 p;
        PUINT c;
    } Colors;

} GM_CCB, *PGM_CCB;



PGM_CCB gmCCB_Create( UINT GUI_height, UINT GUI_offset )
{
    PGM_CCB p = (PGM_CCB)malloc(sizeof(GM_CCB));
    memset(p,0,sizeof(GM_CCB));
    p->VP.go = GUI_offset;
    p->VP.gh = GUI_height;
    p->Grid.w = 30;
    p->Grid.h = 20;
    p->Grid.p = (PUINT8)malloc(p->Grid.w*p->Grid.h*sizeof(UINT8));
    p->Colors.m = 7;
    p->Colors.p = (PUINT32)malloc((p->Colors.m*2+2)*sizeof(UINT32));
    p->Colors.c = (PUINT)malloc((p->Colors.m+1)*sizeof(UINT));
    p->Colors.p[0] = 0x37474F;
    p->Colors.p[1] = 0x263238;
    gmCCB_ColorAdd( p, 0xC62828, 0xE53935 ); /* Red */
    // gmCCB_ColorAdd( p, 0x6A1B9A, 0x8E24AA ); /* Purple */
    // gmCCB_ColorAdd( p, 0x283593, 0x3949AB ); /* Indigo */
    gmCCB_ColorAdd( p, 0x00838F, 0x00ACC1 ); /* Cyan */
    gmCCB_ColorAdd( p, 0x558B2F, 0x7CB342 ); /* Light Green */
    // gmCCB_ColorAdd( p, 0xF9A825, 0xFDD835 ); /* Yellow */
    // gmCCB_ColorAdd( p, 0x4E342E, 0x6D4C41 ); /* Brown */
    return p;
}
VOID    gmCCB_Delete( PGM_CCB p )
{
    free(p->Colors.p);
    free(p->Colors.c);
    free(p->Grid.p);
    free(p);
}
VOID    gmCCB_NewGame( PGM_CCB p, UINT random_seed )
{
    p->random_seed = random_seed;
    p->n_bricks = 0;
    p->n_scores = 0;
    p->b_end = 0;
    p->Grid.x = p->Grid.w;
    UINT k = p->random_seed;
    memset( p->Colors.c, 0, (p->Colors.n+1)*sizeof(UINT) );
    for( UINT i=0; i<(p->Grid.w*p->Grid.h); ++i )
        ++p->Colors.c[(p->Grid.p[i]=(((k=__rand(k,__rand__a,__rand__c))%p->Colors.n)+1)<<1)>>1];
}
VOID    gmCCB_ColorAdd( PGM_CCB p, UINT32 cNormal, UINT32 cHover )
{
    if((p->Colors.n+1)>=p->Colors.m)
    {
        p->Colors.m+=8;
        PUINT32 buf=(PUINT32)malloc((p->Colors.m*2+2)*sizeof(UINT32));
        memcpy(buf,p->Colors.p,(p->Colors.n*2+2)*sizeof(UINT32));
        free(p->Colors.p);
        p->Colors.p=buf;
        buf=(PUINT)malloc((p->Colors.m+1)*sizeof(UINT));
        free(p->Colors.c);
        p->Colors.c=buf;
    }
    ++p->Colors.n;
    p->Colors.p[p->Colors.n*2+0]=cNormal;
    p->Colors.p[p->Colors.n*2+1]=cHover;
}
VOID    gmCCB_ReCalcVP( PGM_CCB p, UINT w, UINT h )
{
    CONST UINT vw = w-(p->VP.go*2);
    CONST UINT vh = h-(p->VP.go*3)-(p->VP.gh);
    CONST UINT cw = vw*p->Grid.h;
    CONST UINT ch = vh*p->Grid.w;
    if(cw>ch)
    {
        p->VP.w = ch/p->Grid.h;
        p->VP.h = vh;
        p->VP.x = (w-p->VP.w)/2;
        p->VP.y = p->VP.go;
    }
    else
    {

        p->VP.w = vw;
        p->VP.h = cw/p->Grid.w;
        p->VP.x = p->VP.go;
        p->VP.y = (h-p->VP.h-p->VP.gh-p->VP.go)/2;
    }
}
BOOL    gmCCB_ReCalcCursor( PGM_CCB p, UINT x, UINT y )
{
    CONST UINT ox=p->Grid.x;
    CONST UINT oy=p->Grid.y;
    CONST UINT w=p->Grid.w;
    CONST UINT h=p->Grid.h;
    CONST PUINT8 pg=p->Grid.p;
    BOOL b=FALSE;

    CONST UINT ux = (UINT)floorf(((FLOAT)(x-p->VP.x))*(FLOAT)(p->Grid.w)/(FLOAT)(p->VP.w));
    CONST UINT uy = (UINT)floorf(((FLOAT)(y-p->VP.y))*(FLOAT)(p->Grid.h)/(FLOAT)(p->VP.h));
    p->Grid.x=ux;
    p->Grid.y=uy;

    UINT32 c=0;
    UINT k=0;
    VOID _r(UINT x, UINT y) {
        if((x<w)&&(y<h)&&(pg[y+x*h]==c))
            pg[y+x*h]^=1,++k,_r(x,y+1),_r(x,y-1),_r(x+1,y),_r(x-1,y);
    }
    if((ox<w)&&(oy<h))
    {
        if((ux<w)&&(uy<h))
        {
            if(((c=pg[oy+ox*h])>0)&&((c&1)==1)) _r(ox,oy),b=TRUE;
            k=0;
            if(((c=pg[uy+ux*h])>0)&&((c&1)==0)) _r(ux,uy),b=TRUE;
        }
        else
        {
            if(((c=pg[oy+ox*h])>0)&&((c&1)==1)) _r(ox,oy),b=TRUE;
            k=0;
        }
    }
    else
    if((ux<w)&&(uy<h))
    {
        k=0;
        if(((c=pg[uy+ux*h])>0)&&((c&1)==0)) _r(ux,uy),b=TRUE;
    }
    p->Grid.n=k;
    return b;
}
BOOL    gmCCB_ReCalcClick( PGM_CCB p, UINT x, UINT y )
{
    if(p->b_end)
    {
        gmCCB_NewGame( p, p->random_seed+1 );
        return gmCCB_ReCalcCursor( p, x, y );
    }
    BOOL b=gmCCB_ReCalcCursor( p, x, y );
    CONST UINT ux=p->Grid.x;
    CONST UINT uy=p->Grid.y;
    CONST UINT w=p->Grid.w;
    CONST UINT h=p->Grid.h;
    CONST PUINT8 pg=p->Grid.p;
    UINT32 c=0;
    VOID _r(UINT x, UINT y) {
        if((x<w)&&(y<h)&&(pg[y+x*h]==c))
            pg[y+x*h]=0,_r(x,y+1),_r(x,y-1),_r(x+1,y),_r(x-1,y);
    }
    if((ux<w)&&(uy<h))
    {
        if(((c=pg[uy+ux*h])>0)&&((c&1)==1)&&(p->Grid.n>=2))
        {
            p->n_bricks += p->Grid.n;
            p->n_scores += (p->Grid.n*(p->Grid.n-1))>>1;
            _r(ux,uy),b=TRUE;

            memset( p->Colors.c, 0, (p->Colors.n+1)*sizeof(UINT) );

            for( UINT ix=0; ix<w; ++ix )
            {
                for( UINT iy=0; iy<h; ++iy )
                {
                    c=pg[iy+ix*h];
                    ++p->Colors.c[c>>1];
                    if(c>0)
                    {
                        for( UINT _y=iy-1; _y<h; --_y )
                        {
                            if(pg[_y+ix*h]==0)
                            {
                                pg[_y+ix*h]=pg[_y+1+ix*h];
                                pg[_y+1+ix*h]=0;
                            }
                            else break;
                        }
                    }
                }
            }
            for( UINT ix=w-2; ix<w; --ix )
            {
                BOOL bb=TRUE;
                for( UINT iy=0; (iy<h)&&bb; ++iy )
                    if(pg[iy+ix*h]>0) bb=FALSE;
                if(bb)
                {
                    memcpy( pg+ix*h, pg+ix*h+h, h*(w-ix-1) );
                    memset( pg+(w-1)*h, 0, h );
                }
            }

            p->Grid.x = p->Grid.w;
            gmCCB_ReCalcCursor( p, x, y );

            BOOL be=TRUE;

            for( UINT ix=0; (ix<w)&&be; ++ix )
            for( UINT iy=0; (iy<h)&&be; ++iy )
            {
                if(be&&(ix+1<w)&&((c=pg[iy+ix*h+h])>0))
                    be=(c!=pg[iy+ix*h]);
                if(be&&(ix-1<w)&&((c=pg[iy+ix*h-h])>0))
                    be=(c!=pg[iy+ix*h]);
                if(be&&(iy+1<h)&&((c=pg[iy+ix*h+1])>0))
                    be=(c!=pg[iy+ix*h]);
                if(be&&(iy-1<h)&&((c=pg[iy+ix*h-1])>0))
                    be=(c!=pg[iy+ix*h]);
            }
            p->b_end = be;
            if(be)
            {
                CONST UINT uu=w*h-p->Colors.c[0];
                if(uu==0)
                {
                    p->n_scores = (p->n_scores*110)/100;
                }
                else
                if(uu==1)
                {
                    p->n_scores = (p->n_scores*105)/100;
                }
                else
                {
                    p->n_scores -= uu*(uu-1);
                }
            }
        }
    }
    return b;
}

HDC bmp_hDC;
PUINT32 bmp_pvBits;
HBITMAP bmp_hBMP;
HBITMAP bmp_hBMP_Old;
HFONT bmp_hFont;
HFONT bmp_hFont_Old;

VOID    gmCCB_Render_GDI_Create( PGM_CCB p, HDC hDC, UINT w, UINT h )
{
    bmp_hDC = CreateCompatibleDC( hDC );
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;
    bmp_hBMP = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
        (PVOID*)(&bmp_pvBits), NULL, 0x0 );
    bmp_hBMP_Old = SelectBitmap( bmp_hDC, bmp_hBMP );

    LOGFONTA lf;
    lf.lfHeight         = -(p->VP.gh*3/7);
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_REGULAR;
    lf.lfItalic         = FALSE;
    lf.lfUnderline      = FALSE;
    lf.lfStrikeOut      = FALSE;
    lf.lfCharSet        = ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    memcpy( lf.lfFaceName, "Roboto Slab", 12 );

    bmp_hFont = CreateFontIndirectA( &lf );
}
VOID    gmCCB_Render_GDI_Delete( PGM_CCB p, HDC hDC, UINT w, UINT h )
{
    DeleteFont( bmp_hFont );
    SelectBitmap( bmp_hDC, bmp_hBMP_Old );
    DeleteBitmap( bmp_hBMP );
    DeleteDC( bmp_hDC );
}

VOID    gmCCB_Render_GDI( PGM_CCB p, HDC hDC, UINT w, UINT h )
{
    CONST UINT tuiL=p->VP.go;
    CONST UINT tuiR=w-p->VP.go;
    {
        CONST UINT vpx=p->VP.x;
        CONST UINT vpy=p->VP.y;
        CONST UINT gw=p->Grid.w;
        CONST UINT gh=p->Grid.h;
        CONST FLOAT faw=(FLOAT)(gw)/(FLOAT)(p->VP.w);
        CONST FLOAT fah=(FLOAT)(gh)/(FLOAT)(p->VP.h);
        CONST PUINT8 pg=p->Grid.p;
        CONST PUINT32 pc=p->Colors.p;
        PUINT32 pb=bmp_pvBits;
        CONST UINT tuiB=h-p->VP.go-p->VP.gh;
        CONST UINT tuiT=tuiB+p->VP.gh;
        for( UINT y=0; y<h; ++y )
        for( UINT x=0; x<w; ++x )
        {
            CONST FLOAT fgx = (FLOAT)(x-vpx)*faw;
            CONST FLOAT fgy = (FLOAT)(y-vpy)*fah;
            if((x>=tuiL)&&(x<tuiR)&&(y>=tuiB)&&(y<tuiT)) *pb=pc[p->b_end];
            else
            if((fgx<0.0f)||(fgy<0.0f)) *pb=pc[!p->b_end];
            else
            {
                CONST UINT ux = (UINT)floorf(fgx);
                CONST UINT uy = (UINT)floorf(fgy);
                if((ux>=gw)||(uy>=gh)) *pb=pc[!p->b_end];
                else *pb=pc[pg[uy+ux*gh]];
            }
            ++pb;
        }
    }
    {
        BitBlt( hDC, 0, 0, w, h,bmp_hDC, 0, 0, SRCCOPY );
    }
    {
        bmp_hFont_Old = SelectFont( hDC, bmp_hFont );

        CHAR str[64];
        UINT n;
        SetTextColor( hDC,
            ((((p->Colors.p[!p->b_end]>>16)&0xff))|
            (((p->Colors.p[!p->b_end]>>8)&0xff)<<8)|
            (((p->Colors.p[!p->b_end])&0xff)<<16)) );
        SetBkMode( hDC, TRANSPARENT );
        SetTextAlign( hDC, TA_CENTER );

        CONST UINT tuiw = (tuiR-tuiL);

        n = snprintf( str, 64, "HIT_BRICKS: %d", p->Grid.n );
        TextOutA( hDC, tuiL+tuiw/8, p->VP.go, str, n );
        n = snprintf( str, 64, "HIT_SCORES: %d", (p->Grid.n*(p->Grid.n-1))>>1 );
        TextOutA( hDC, tuiL+tuiw*3/8, p->VP.go, str, n );
        n = snprintf( str, 64, "BRICKS: %d", p->n_bricks );
        TextOutA( hDC, tuiL+tuiw*5/8, p->VP.go, str, n );
        n = snprintf( str, 64, "SCORES: %d", p->n_scores );
        TextOutA( hDC, tuiL+tuiw*7/8, p->VP.go, str, n );

        for( UINT i=0; i<=p->Colors.n; ++i )
        {
            CONST UINT32 ccc=p->Colors.p[(i<<1)|(!p->b_end)];
            SetTextColor( hDC, ((((ccc>>16)&0xff))|(((ccc>>8)&0xff)<<8)|((ccc&0xff)<<16)));
            n = snprintf( str, 64, "%d", p->Colors.c[i] );
            TextOutA( hDC, tuiL+((tuiw*((i<<1)+1)/(p->Colors.n+1))>>1), p->VP.go+(p->VP.gh>>1), str, n );
        }

        SelectFont( hDC, bmp_hFont_Old );
    }
}
