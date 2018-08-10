#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include "dbg.h"
#include "gm_ccb.h"

typedef struct gm_CCB
{
    UINT random_seed;
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
    } Colors;

} GM_CCB;

PGM_CCB gmCCB_Create( UINT GUI_height, UINT GUI_offset )
{
    PGM_CCB p = (PGM_CCB)malloc(sizeof(GM_CCB));
    memset(p,0,sizeof(GM_CCB));
    p->VP.go = GUI_offset;
    p->VP.gh = GUI_height;
    p->Grid.w = 40;
    p->Grid.h = 30;
    p->Grid.p = (PUINT8)malloc(p->Grid.w*p->Grid.h*sizeof(UINT8));
    p->Colors.m = 7;
    p->Colors.p = (PUINT32)malloc((p->Colors.m*2+2)*sizeof(UINT32));
    p->Colors.p[0] = 0x37474F;
    p->Colors.p[1] = 0x263238;
    gmCCB_ColorAdd( p, 0xC62828, 0xE53935 ); /* Red */
    gmCCB_ColorAdd( p, 0x6A1B9A, 0x8E24AA ); /* Purple */
    gmCCB_ColorAdd( p, 0x283593, 0x3949AB ); /* Indigo */
    gmCCB_ColorAdd( p, 0x00838F, 0x00ACC1 ); /* Cyan */
    gmCCB_ColorAdd( p, 0x558B2F, 0x7CB342 ); /* Light Green */
    gmCCB_ColorAdd( p, 0xF9A825, 0xFDD835 ); /* Yellow */
    gmCCB_ColorAdd( p, 0x4E342E, 0x6D4C41 ); /* Brown */
    return p;
}
VOID    gmCCB_Delete( PGM_CCB p )
{
    free(p->Colors.p);
    free(p->Grid.p);
    free(p);
}
VOID    gmCCB_NewGame( PGM_CCB p, UINT random_seed )
{
    p->random_seed = random_seed;
    UINT k = p->random_seed;
    for( UINT i=0; i<(p->Grid.w*p->Grid.h); ++i )
        p->Grid.p[i]=(((k=__rand(k,137,574))%p->Colors.n)+1)<<1;
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
    }
    ++p->Colors.n;
    p->Colors.p[p->Colors.n*2+0]=cNormal;
    p->Colors.p[p->Colors.n*2+1]=cHover;
}
VOID    gmCCB_ReCalcVP( PGM_CCB p, UINT w, UINT h )
{
    CONST UINT vw = w-(p->VP.go*2);
    CONST UINT vh = h-(p->VP.go*2)-(p->VP.gh);
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
        p->VP.y = (h-p->VP.h-p->VP.gh)/2;
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
            if((((c=pg[uy+ux*h])&1)==0)&&(c>0))
                c=pg[oy+ox*h],_r(ox,oy),c=pg[uy+ux*h],k=0,_r(ux,uy),b=TRUE;
        }
        else
        {
            c=pg[oy+ox*h],_r(ox,oy),k=0,b=TRUE;
        }
    }
    else
    if((ux<w)&&(uy<h))
    {
        if(((c=pg[uy+ux*h])>0))
            k=0,_r(ux,uy),b=TRUE;
    }
    p->Grid.n=k;
    return b;
}

HDC bmp_hDC;
PUINT32 bmp_pvBits;
HBITMAP bmp_hBMP;
HBITMAP bmp_hBMP_Old;

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
}
VOID    gmCCB_Render_GDI_Delete( PGM_CCB p, HDC hDC, UINT w, UINT h )
{
    SelectBitmap( bmp_hDC, bmp_hBMP_Old );
    DeleteBitmap( bmp_hBMP );
    DeleteDC( bmp_hDC );
}

VOID    gmCCB_Render_GDI( PGM_CCB p, HDC hDC, UINT w, UINT h )
{
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
        for( UINT y=0; y<h; ++y )
        for( UINT x=0; x<w; ++x )
        {
            CONST FLOAT fgx = (FLOAT)(x-vpx)*faw;
            CONST FLOAT fgy = (FLOAT)(y-vpy)*fah;
            if((x>=p->VP.go)&&(x<w-p->VP.go)&&(y>=h-p->VP.gh)) *pb=pc[0];
            else
            if((fgx<0.0f)||(fgy<0.0f)) *pb=pc[1];
            else
            {
                CONST UINT ux = (UINT)floorf(fgx);
                CONST UINT uy = (UINT)floorf(fgy);
                if((ux>=gw)||(uy>=gh)) *pb=pc[1];
                else *pb=pc[pg[uy+ux*gh]];
            }
            ++pb;
        }
    }
    {
        BitBlt( hDC, 0, 0, w, h,bmp_hDC, 0, 0, SRCCOPY );

    }
    {
        CHAR str[64];
        UINT n = snprintf( str, 64, "Bricks: [%d]", p->Grid.n );
        SetTextColor( hDC,
            ((((p->Colors.p[1]>>16)&0xff))|
            (((p->Colors.p[1]>>8)&0xff)<<8)|
            (((p->Colors.p[1])&0xff)<<16)) );
        SetBkMode( hDC, TRANSPARENT );
        TextOutA( hDC, p->VP.x, 0, str, n );
    }
}
