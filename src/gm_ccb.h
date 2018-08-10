#ifndef _H_GM_CCB_H_
#define _H_GM_CCB_H_

#include <windows.h>

typedef struct gm_CCB *PGM_CCB;

PGM_CCB gmCCB_Create( UINT GUI_height, UINT GUI_offset );
VOID    gmCCB_Delete( PGM_CCB p );
VOID    gmCCB_NewGame( PGM_CCB p, UINT random_seed );
VOID    gmCCB_ColorAdd( PGM_CCB p, UINT32 cNormal, UINT32 cHover );
VOID    gmCCB_ReCalcVP( PGM_CCB p, UINT w, UINT h );
BOOL    gmCCB_ReCalcCursor( PGM_CCB p, UINT x, UINT y );
BOOL    gmCCB_ReCalcClick( PGM_CCB p, UINT x, UINT y );
VOID    gmCCB_Render_GDI_Create( PGM_CCB p, HDC hDC, UINT w, UINT h );
VOID    gmCCB_Render_GDI_Delete( PGM_CCB p, HDC hDC, UINT w, UINT h );
VOID    gmCCB_Render_GDI( PGM_CCB p, HDC hDC, UINT w, UINT h );

#endif

