#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include "dbg.h"
#include "gm_ccb.h"

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


} GM_CCB;

PGM_CCB gmCCB_Create( UINT GUI_height, UINT GUI_offset )
{
    PGM_CCB p = (PGM_CCB)malloc(sizeof(GM_CCB));
    memset(p,0,sizeof(GM_CCB));
    p->VP.go = GUI_offset;
    p->VP.gh = GUI_height;
    p->Grid.w = 10;
    p->Grid.h = 7;
    p->Grid.p = (PUINT8)malloc(p->Grid.w*p->Grid.h*sizeof(UINT8));
    p->Colors.m = 7;
    p->Colors.p = (PUINT32)malloc((p->Colors.m*2+2)*sizeof(UINT32));
    p->Colors.c = (PUINT)malloc((p->Colors.m+1)*sizeof(UINT));
    p->Colors.p[0] = 0x37474F;
    p->Colors.p[1] = 0x263238;
    gmCCB_ColorAdd( p, 0xC62828, 0xE53935 ); /* Red */
    // gmCCB_ColorAdd( p, 0x6A1B9A, 0x8E24AA ); /* Purple */
    gmCCB_ColorAdd( p, 0x283593, 0x3949AB ); /* Indigo */
    // gmCCB_ColorAdd( p, 0x00838F, 0x00ACC1 ); /* Cyan */
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
        ++p->Colors.c[(p->Grid.p[i]=(((k=__rand(k,137,574))%p->Colors.n)+1)<<1)>>1];
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
                if(p->Colors.c[0]==w*h)
                {
                    p->n_scores = (p->n_scores*110)/100;
                }
                else
                if(p->Colors.c[0]==(w*h-1))
                {
                    p->n_scores = (p->n_scores*103)/100;
                }
                else
                if(p->Colors.c[0]<=(w*h-6))
                {
                    p->n_scores = (p->n_scores*90)/100;
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
            n = snprintf( str, 64, "%d:%d", i, p->Colors.c[i] );
            TextOutA( hDC, tuiL+((tuiw*((i<<1)+1)/(p->Colors.n+1))>>1), p->VP.go+(p->VP.gh>>1), str, n );
        }

        SelectFont( hDC, bmp_hFont_Old );
    }
}
