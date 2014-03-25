/* R S B . C */
#define OEMRESOURCE
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "disp.h"
#include "help.h"
#include "screen.h"
#include "rsb.h"
#include "resource.h"
#include "debug.h"



extern struct SCI	vsci;
extern int		mwCur;
extern HCURSOR		vhcSplit;
extern HCURSOR		vhcArrow;
extern HWND 		vhwndCBT;
extern int 		vfHelp;
extern long 		vcmsecHelp;
extern struct BMI   	mpidrbbmi[];
#ifdef WIN23
extern int              wwCur;
#endif /* WIN23 */
extern HWND		vhwndApp;
struct RSBI		vrsbi;

struct BMS *PbmsFromFlags();

/*************************************************************************/
/*********** CALLS PROVIDED TO CALLERS OUTSIDE THIS MODULE ***************/
/*************************************************************************/

/*************************/
/* R S B  W n d  P r o c */
/* Window procedure for RSB controls */

/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:RSBWndProc %%Owner:bryanl */
long EXPORT FAR PASCAL RSBWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	static BYTE fInzed = fFalse;

	switch (message)
		{
#ifdef DEBUG
	default:
		Assert( fFalse );
		break;
#endif	/* DEBUG */

    case WM_NCCREATE:
		if (!fInzed)
			{
			if (!FInitRSB())
				return (long)fFalse;
			fInzed = fTrue;
			}
		break;

	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		SetWindowWord( hwnd, offset(RSBS,sps), 0 );
		SetWindowWord( hwnd, offset(RSBS,fVert),
				((WORD)((LPCREATESTRUCT)lParam)->style) & SBS_VERT );
		SetWindowWord( hwnd, offset(RSBS,grpfWnd),
				((WORD)((LPCREATESTRUCT)lParam)->style) & maskWnd );
		SetWindowWord( hwnd, offset(RSBS,spsLim), dqMax );
		SetWindowWord( hwnd, offset(RSBS,izppInvert), iNil);
		break;
	case WM_ERASEBKGND:
		goto LRet;
	case WM_PAINT:
		Assert( GetWindowWord( hwnd, offset(RSBS,izppInvert)) == iNil );
		RSBPaint( hwnd );
		break;
	case WM_LBUTTONDOWN:
		if (vfHelp)
			{
			long LRSBHelp();

			return LRSBHelp(hwnd);
			}
		goto LClick;
	case WM_LBUTTONDBLCLK:
		/* do not respond if doubleclick right after help request */
		if (GetMessageTime() < vcmsecHelp)
			return(0L);
LClick:
	/* do not respond to mouse hits on whited-out control */
		if (GetWindowWord( hwnd, offset(RSBS,fBlank)))
			break;

		EnsureFocusInPane();

		switch ( GetWindowWord( hwnd, offset(RSBS,grpfWnd)) )
			{
		case maskWndSplitBox:
			return (long) FSplitBoxMouse( hwnd, message, lParam );
		case maskWndSizeBox:
				{
				int sc = (message == WM_LBUTTONDOWN ? SC_SIZE : SC_ZOOM);

				ClientToScreen(hwnd, (POINT FAR *)&lParam);

/* FOLLOWING COMMENT COMES FROM WINDOWS CODE WHENCE THIS WAS STOLEN */
			/* convert HT value into a move value. This is shitty,
			* but this is purely temporary.
			*/
				SendMessage(GetParent(hwnd), WM_SYSCOMMAND,
						(sc | (HTBOTTOMRIGHT - HTSIZEFIRST + 1)), lParam);
				break;
				}
		case maskWndPgVw:
			TrackPgVwMouse( hwnd, MAKEPOINT(lParam), 
					GetWindowWord(hwnd, offset(RSBS,bcm)) );
			break;
		default:	/* scroll bar */
			RSBHit( hwnd, MAKEPOINT(lParam) );
			break;
			}
		break;
	case WM_SETCURSOR:
		OurSetCursor( 
				((GetWindowWord(hwnd, offset(RSBS,grpfWnd)) & maskWndSplitBox)
				&& (PmwdMw(mwCur)->hwndSplitBox == hwnd))
				? vhcSplit : vhcArrow );
		break;
		}
LRet:
	return ((long)fTrue);
}


/***********************/
/* R S B  S e t  S p s */
/* Set elevator of RSB control */

/* %%Function:RSBSetSps %%Owner:bryanl */
RSBSetSps( hwnd, spsNew )
HWND hwnd;
int spsNew;
{
	Assert( !GetWindowWord( hwnd, offset(RSBS,grpfWnd) ));
#ifdef WIN23
	Assert( (uns)spsNew <= GetWindowWord( hwnd, offset(RSBS,spsLim)) );
#else
	Assert( (uns)spsNew < GetWindowWord( hwnd, offset(RSBS,spsLim)) );
#endif /* WIN23 */

	RSBSmartUpdate(hwnd, spsNew);
}


/**********************************/
/* S p s  F r o m  H w n d  R S B */
/* Get current elevator setting of RSB control */

/* %%Function:SpsFromHwndRSB %%Owner:bryanl */
SpsFromHwndRSB( hwnd )
HWND hwnd;
{
	Assert( !GetWindowWord( hwnd, offset(RSBS,grpfWnd) ));
	return GetWindowWord( hwnd, offset(RSBS, sps) );
}


/**********************************/
/* S p s  L i m  F r o m  H w n d  R S B */
/* Get current elevator setting of RSB control */

/* %%Function:SpsLimFromHwndRSB %%Owner:bryanl */
SpsLimFromHwndRSB( hwnd )
HWND hwnd;
{
	Assert( !GetWindowWord( hwnd, offset(RSBS,grpfWnd) ));
	return GetWindowWord( hwnd, offset(RSBS, spsLim) );
}


/******************************/
/* R S B  S e t  S p s  L i m */
/* Set Lim allowable setting for elevator. */
/* WARNING: Does NOT update control, caller's responsibility */

/* %%Function:RSBSetSpsLim %%Owner:bryanl */
RSBSetSpsLim( hwnd, spsLim )
HWND hwnd;
int spsLim;
{
	Assert( !GetWindowWord( hwnd, offset(RSBS,grpfWnd) ));
	Assert( spsLim > 0 );

	SetWindowWord( hwnd, offset(RSBS,spsLim), spsLim );
}


/*************************************************************************/
/**********************  D R A W I N G  C O D E **************************/
/*************************************************************************/

/********************/
/* R S B  P a i n t */
/* Process paint message */

/* %%Function:RSBPaint %%Owner:bryanl */
RSBPaint(hwnd)
{
	PAINTSTRUCT ps;
	struct RC rc;
	HDC hdc;
	int sps = GetWindowWord( hwnd, offset(RSBS,sps) );

	BeginPaint( hwnd, (LPPAINTSTRUCT) &ps );
	RSBPaintCore( hwnd, ps.hdc, sps, &ps.rcPaint );
	EndPaint( hwnd, (LPPAINTSTRUCT)&ps );
}


/***************************/
/* R S B  P a i n t  A l l */
/* Paint entire control */

/* %%Function:RSBPaintAll %%Owner:bryanl */
RSBPaintAll( hwnd )
HWND hwnd;
{
	struct RC rc;
	HDC hdc;

	if ((hdc = GetDC( hwnd )) != NULL)
		{
		GetClientRect( hwnd, (LPRECT)&rc );
		RSBPaintCore( hwnd, hdc,
				GetWindowWord( hwnd, offset(RSBS,sps)), &rc );
		ReleaseDC( hwnd, hdc );
		}
}


/*****************************/
/* R S B  P a i n t  C o r e */
/* Given a DC and an area to paint, paint appropriate parts of the RSB */

/* %%Function:RSBPaintCore %%Owner:bryanl */
RSBPaintCore( hwnd, hdc, sps, prcPaint )
HWND hwnd;
HDC hdc;
int sps;
struct RC *prcPaint;
{
	int izpp;
	struct ZPP *pzpp;
	int grpfWnd = GetWindowWord( hwnd, offset(RSBS,grpfWnd) );
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	int bzpPt = (fVert ? offset(PT,yp) : offset(PT,xp) );
#define ZpFromPt(pt)	 *((int *)((char *)&(pt) + bzpPt))
	union GRPZPP grpzpp;
	struct RC rc;

	GetClientRect( hwnd, &rc );

/* SPECIAL WINDOW STATE: BLANKED OUT, SHOW BORDER ONLY */

	if (GetWindowWord( hwnd, offset(RSBS,fBlank)))
		{
LBlank:
		if (FSetDcAttribs( hdc, dccmBkgrnd+dccmHpenBorder))
			PatBltRc( hdc, prcPaint, vsci.ropErase );

		if (SelectObject( hdc, vsci.hbrBorder ) &&
				(grpfWnd & maskWnd) != maskWndPgVw)
			{
			PatBltBorder( hdc, &rc, fTrue /*fVert*/, fTrue /*fTop*/ );
			PatBltBorder( hdc, &rc, fFalse /*fVert*/, fTrue /*fTop*/ );
			}
		return;
		}

/* SPECIAL WINDOW TYPES: SIZE BOX & SPLIT BOX & PG VW ICON */

	switch ( grpfWnd & maskWnd )
		{
	case maskWndPgVw:
			{
			struct MDCD *pmdcd;
			struct BMI *pbmi;
#ifdef WIN23
			int idrb = (vsci.fWin3Visuals ? idrbPageview3 : idrbPageview2);
#endif /* WIN23 */

			RSBSetDCColors(hdc);
#ifdef WIN23
			if ((pmdcd = PmdcdCacheIdrb( idrb, hdc )) == NULL)
#else
			if ((pmdcd = PmdcdCacheIdrb( idrbPageview, hdc )) == NULL)
#endif /* WIN23 */
				goto LBlank;

#ifdef WIN23
			pbmi = &mpidrbbmi [idrb];
#else
			pbmi = &mpidrbbmi [idrbPageview];
#endif /* WIN23 */
/* paint by bltting appropriate bitmap to screen */

			BitBlt( hdc, 
					rc.xpLeft,
					rc.ypTop,                       /* destination */
					pbmi->dxpEach, pbmi->dyp,       /* size */
					pmdcd->hdc,                     /* source */
			0, 0,
					vsci.dcibScreen.ropBitBlt );    /* raster op */
			}
		return;
	case maskWndSizeBox:
		RSBPaintBms( hdc, &vrsbi.bmsSizeBox, &rc );
		return;
	case maskWndSplitBox:
		if (SelectObject( hdc, vsci.hbrBorder ))
			PatBltRc( hdc, &rc, PATCOPY );
		return;
		}

/* SCROLL BAR WINDOW: Paint dirty areas */

	RSBComputeGrpzpp( hwnd, sps, &grpzpp );
	for ( izpp = 0, pzpp = grpzpp.rgzpp; izpp < izppMax ; izpp++, pzpp++ )
		{
		if (ZpFromPt(prcPaint->ptTopLeft) < pzpp->zpMac &&
				ZpFromPt(prcPaint->ptBottomRight) > pzpp->zpMin)
			{
			RSBPaintIzpp( hwnd, hdc, pzpp, izpp );
			}
		}

#undef ZpFromPt
}


/*****************************/
/* R S B  P a i n t  I z p p */
/* Paint 1 region of the scroll bar */

/* %%Function:RSBPaintIzpp %%Owner:bryanl */
RSBPaintIzpp( hwnd, hdc, pzpp, izpp )
HWND hwnd;
HDC hdc;
struct ZPP *pzpp;
int izpp;
{
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	struct RC rc, rcT;

	Assert( !GetWindowWord( hwnd, offset(RSBS,fBlank) ) );
	if (pzpp->zpMin >= pzpp->zpMac)
		return;

	RSBZppToRc( hwnd, pzpp, &rc );

	switch (izpp)
		{
#ifdef DEBUG
	default:
		Assert( fFalse );
		break;
#endif	/* DEBUG */
	case izppBelowThumb:
	case izppAboveThumb:
		SelectObject( hdc, vsci.hbrScrollBar );
		PatBltRc( hdc, &rc, PATCOPY );
		break;
	case izppThumb:
#ifdef WIN23
		if (vsci.fWin3)
			{
			LONG clrSav = SetBkColor(hdc, vsci.rgbButtonFace);
			/* fill inside of thumb rect */
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);
			SetBkColor(hdc, clrSav);
			/* draw inner shadow and highlight */
			InflateRect(&rc, -1, -1);
			DrawButtonShadow(hdc, &rc, fTrue, fFalse);
			InflateRect(&rc, 1, 1);  /* restore the rect */
			}
		else
			{
			SelectObject( hdc, vsci.hbrBkgrnd );
			PatBltRc( hdc, &rc, PATCOPY );
			}
		SelectObject( hdc, vsci.hbrBorder);
		PatBltBorder( hdc, &rc, fVert, fFalse );
#else
		SelectObject( hdc, vsci.hbrBkgrnd );
		PatBltRc( hdc, &rc, PATCOPY );
		SelectObject( hdc, vsci.hbrBorder );
		PatBltBorder( hdc, &rc, fVert, fFalse );
#endif /* WIN23 */
		break;
	case izppUArrow:
	case izppDArrow:
		RSBPaintBms( hdc, PbmsFromFlags( fVert, izpp==izppUArrow ), &rc );
		break;
		}

	SelectObject( hdc, vsci.hbrBorder );
	PatBltBorder( hdc, &rc, !fVert, fTrue /*fTop*/ );
	PatBltBorder( hdc, &rc, !fVert, fFalse /*fTop*/);
	PatBltBorder( hdc, &rc, fVert, izpp <= izppThumb /*fTop*/ );

	Assert( GetWindowWord( hwnd, offset(RSBS,izppInvert)) != izpp );
}


/*********************************/
/* R S B  S m a r t  U p d a t e */
/* screen display is OK but sps has changed; move the thumb */

/* %%Function:RSBSmartUpdate %%Owner:bryanl */
RSBSmartUpdate(hwnd, spsNew)
HWND hwnd;
int spsNew;
{
	HDC hdc;
	int sps = GetWindowWord( hwnd, offset(RSBS,sps) );
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	int bzpPt = (fVert ? offset(PT,yp) : offset(PT,xp) );
	int izpp, izppInvert = GetWindowWord( hwnd, offset(RSBS,izppInvert));
	struct RC rcInvert, rcThumbFrom, rcThumbTo, rcScroll, rcUpdate, rcT;
#define ZpFromPt(pt)	 *((int *)((char *)&(pt) + bzpPt))
	struct GRPZPP grpzpp, grpzppNew;

	if (sps == spsNew)
		return;		/* current setting is already correct */
	if (GetWindowWord( hwnd, offset(RSBS,fBlank) ))
		goto LDone;

	if ((hdc = GetDC( hwnd)) == NULL)
		return;

	RSBComputeGrpzpp( hwnd, sps, &grpzpp );

/* PgUp/PgDn area mouse hit: uninvert before bltting */

	Assert( izppUArrow < izppAboveThumb && izppDArrow > izppBelowThumb );
	if (izppInvert != iNil && 
			(uns)(izppInvert - izppAboveThumb) <= izppBelowThumb - izppAboveThumb)
		{
		RSBZppToRc( hwnd, &grpzpp.rgzpp [izppInvert], &rcInvert );
		RSBInvertForTrack( hwnd, hdc, &rcInvert, izppInvert );
		}

/* figure out where the thumb is coming from & going to, then BitBlt */

	RSBComputeGrpzpp( hwnd, spsNew, &grpzppNew );

	RSBZppToRc( hwnd, &grpzpp.zppThumb, &rcThumbFrom );
	RSBZppToRc( hwnd, &grpzppNew.zppThumb, &rcThumbTo );

	UnionRect( (LPRECT)&rcScroll, (LPRECT) &rcThumbTo, (LPRECT) &rcThumbFrom );
	ScrollDC( hdc, rcThumbTo.xpLeft - rcThumbFrom.xpLeft, 
			rcThumbTo.ypTop - rcThumbFrom.ypTop,
			(LPSTR) &rcScroll,(LPRECT)&rcScroll,
			(HRGN) NULL, (LPRECT)&rcUpdate );

/* Thumb is in new location -- now paint over the area it vacated
	CASE 1: we scrolled invalid bits from off the screen,
		a covering window, etc; repaint everything that
		ScrollDC's returned rcUpdate says is invalid
	CASE 2: simple case, only need to paint above (below) thumb where
		the thumb used to be.	
*/

	izpp = (grpzppNew.zppThumb.zpMin > grpzpp.zppThumb.zpMin ?
			izppAboveThumb : izppBelowThumb);
	RSBZppToRc( hwnd, &grpzppNew.rgzpp [izpp], &rcT );
	if (!FSectRc( &rcUpdate, &rcT, &rcT ) || FNeRgw( &rcUpdate, &rcT, sizeof(struct RC)/sizeof(WORD)))
		/* CASE 1 */
		RSBPaintCore( hwnd, hdc, spsNew, &rcUpdate );
	else
		/* CASE 2 */
		RSBPaintIzpp( hwnd, hdc, &grpzppNew.rgzpp [izpp], izpp );

	ReleaseDC( hwnd, hdc );
LDone:
	SetWindowWord( hwnd, offset(RSBS,sps), spsNew );
#undef ZpFromPt
}


/****************************************/
/* R S B  I n v e r t  F o r  T r a c k */

/* %%Function:RSBInvertForTrack %%Owner:bryanl */
RSBInvertForTrack( hwnd, hdc, prc, izpp )
HWND hwnd;
HDC hdc;
struct RC *prc;
int izpp;
{
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	int izppInvert = GetWindowWord( hwnd, offset( RSBS,izppInvert) );

	Assert( izppInvert == izpp || izppInvert == iNil );

	SetWindowWord( hwnd, offset(RSBS,izppInvert), (izppInvert == iNil ? izpp : iNil) );
	if (FEmptyRc(prc))
		{
		Assert( GetWindowWord( hwnd, offset( RSBS,izppInvert)) == iNil);
		return;
		}

	if (izpp == izppThumb)
		{
		extern HBRUSH vhbrGray;
		SelectObject( hdc, vhbrGray );
		FrameRectRop( hdc, prc, PATINVERT );
		}
#ifdef WIN23
	else if (vsci.fWin3 && (izpp == izppUArrow || izpp == izppDArrow))
		{
		/* exchange arrow bitmaps with depressed 3d bitmap */
		struct BMS *pbms = PbmsFromFlags( fVert, izpp == izppUArrow);
		
		if (izppInvert == iNil)  /* if we should invert */
			pbms += 5;  /* point to depressed bitmap */
		RSBPaintBms( hdc, pbms, prc );
		}
#endif /* WIN23 */
	else
		{
		struct RC rcT;

		rcT = *prc;
		InflateRect( &rcT, -vsci.dxpBorder, -vsci.dypBorder );
		if (izpp > izppThumb)
			if (fVert)
				rcT.ypTop -= vsci.dypBorder;
			else
				rcT.xpLeft -= vsci.dxpBorder;
		else if (fVert)
			rcT.ypBottom += vsci.dypBorder;
		else
			rcT.xpRight += vsci.dxpBorder;

		PatBltRc( hdc, &rcT, DSTINVERT );
		}
}


/*******************************/
/* P a t  B l t  B o r d e r */

/* %%Function:PatBltBorder %%Owner:bryanl */
PatBltBorder( hdc, prc, fVert, fTop )
HDC hdc;
struct RC *prc;
int fVert, fTop;
{
	struct RC rc;

	rc = *prc;
	if (fVert)
		if (fTop)
			rc.ypBottom = rc.ypTop + vsci.dypBorder;
		else
			rc.ypTop = rc.ypBottom - vsci.dypBorder;
	else if (fTop)
		rc.xpRight = rc.xpLeft + vsci.dxpBorder;
	else
		rc.xpLeft = rc.xpRight - vsci.dxpBorder;

	PatBltRc( hdc, &rc, PATCOPY );
}


/***************************/
/* R S B  P a i n t  B m s */

/* %%Function:RSBPaintBms %%Owner:bryanl */
RSBPaintBms( hdc, pbms, prc )
HDC hdc;
struct BMS *pbms;
struct RC *prc;
{
	HBITMAP hbmSave;

	RSBSetDCColors(hdc);
	if ((hbmSave = SelectObject( vsci.hdcScratch, pbms->hbm )) != NULL)
		{
		StretchBlt( hdc, prc->xpLeft, prc->ypTop,
				prc->xpRight - prc->xpLeft,
				prc->ypBottom - prc->ypTop,
				vsci.hdcScratch,
				0, 0,
				pbms->dxp, pbms->dyp,
				vsci.fInvertMonochrome ? NOTSRCCOPY : SRCCOPY );
		SelectObject( vsci.hdcScratch, hbmSave );
		}
}

/***************************/
/* R S B  S e t  D C  C o l o r s */
/* this function needed since Win3's default colors cause our scroll bars to
come out black on black! */

/* %%Function:RSBPaintBms %%Owner:bryanl */
RSBSetDCColors(hdc)
HDC hdc;
{
	long rgbBk = GetSysColor(COLOR_CAPTIONTEXT);
	long rgbText = GetSysColor(COLOR_WINDOWFRAME);
	if (rgbBk == rgbText)
		{
		rgbBk = rgbWhite;
		rgbText = rgbBlack;
		}
	SetBkColor( hdc, rgbBk );
	SetTextColor( hdc, rgbText );
}

/*************************************************************************/
/***************  M O U S E  T R A C K I N G  C O D E  *******************/
/*************************************************************************/


csconst int mpizppsbm[] = { 
	SB_LINEUP, SB_PAGEUP, 0, SB_PAGEDOWN, SB_LINEDOWN };


/****************/
/* R S B  H i t */
/* Respond to mouse hits on RSB control */

/* %%Function:RSBHit %%Owner:bryanl */
RSBHit( hwnd, ptHit )
HWND hwnd;
struct PT ptHit;
{
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	int bzpPt = (fVert ? offset(PT,yp) : offset(PT,xp) );
#define ZpFromPt(pt)	 *((int *)((char *)&(pt) + bzpPt))
	int zpHit = ZpFromPt(ptHit);
	int izpp;
	int dzpAway;
	struct ZPP *pzpp;
	struct RC rc;
	int sps = GetWindowWord( hwnd, offset(RSBS,sps));
	int spsLim = GetWindowWord( hwnd, offset(RSBS,spsLim));
	struct PT ptMouse;
	HDC hdc;
	int hwndParent = GetParent(hwnd);
	int msg = fVert ? WM_VSCROLL : WM_HSCROLL;
	union GRPZPP grpzpp;

/* determine which area was hit */
	RSBComputeGrpzpp( hwnd, sps, &grpzpp );

	for ( izpp = 0, pzpp = grpzpp.rgzpp ; izpp < izppMax ; izpp++, pzpp++ )
		if (zpHit >= pzpp->zpMin && zpHit < pzpp->zpMac)
			break;
	if (izpp >= izppMax)
		return;

/* if there is no thumb (not enough display space), only the arrows work */

	if (grpzpp.zppThumb.zpMac <= grpzpp.zppThumb.zpMin &&
			izpp != izppUArrow && izpp != izppDArrow)
		return;

/* UpdateWindow avoids confusing the inversion code by insuring that the
	window is up to date */
	UpdateWindow(hwnd);

	if ((hdc = GetDC(hwnd)) == NULL)
		return;

	SetCapture(hwnd);
	RSBZppToRc( hwnd, pzpp, &rc );
	RSBInvertForTrack( hwnd, hdc, &rc, izpp );	/* invert on */
	ptMouse = ptHit;
	if (izpp != izppThumb)
		{
		extern int vfMsg;
		DWORD usecHit = GetTickCount();

/* handle hits everywhere except the thumb */
		for ( ;; )
			{
			if (GetWindowWord(hwnd, offset(RSBS,izppInvert)) != iNil)
				{
				SendMessage( hwndParent, msg, mpizppsbm [izpp], MAKELONG(hwnd, hwnd));
				RSBComputeGrpzpp( hwnd, 
						GetWindowWord(hwnd,
						offset(RSBS,sps)),
						&grpzpp );
				RSBZppToRc( hwnd, pzpp, &rc );
				}
LPollMouse:	
			if (!FStillDownReplay(&ptMouse,fFalse))
				break;
			if (!PtInRect((LPRECT)&rc, ptMouse) ==
					(GetWindowWord(hwnd,offset(RSBS,izppInvert)) != iNil))
				RSBInvertForTrack( hwnd, hdc, &rc, izpp );
/* don't start repeated scrolls until we've waited a decent interval */
/* vfMsg assures that we don't act on a mouse move that was generated 
	before the buttonup; the "synced to queue" feature of GetKeyState
	would inform us that the button is still down when it's really not (bl) */
#define usecReptDelay	600
			if (vfMsg || GetTickCount() - usecHit < usecReptDelay)
				goto LPollMouse;
			}
		if (GetWindowWord( hwnd, offset(RSBS,izppInvert)) != iNil)
			RSBInvertForTrack( hwnd, hdc, &rc, izpp ); /* invert off */
		}
	else
		{
/* handle thumb dragging */
		int dzpAway = ZpFromPt(ptMouse) - grpzpp.zppThumb.zpMin;
		int dzpThumb = grpzpp.zppThumb.zpMac - grpzpp.zppThumb.zpMin;
		struct ZPP zppT;
		struct PT ptInvert;

		Assert( dzpThumb > 0 );
		ptInvert = ptMouse;
		while (FStillDownReplay(&ptMouse,fFalse))
			{
			if (ZpFromPt(ptMouse) != ZpFromPt(ptInvert))
				{
				RSBInvertForTrack( hwnd, hdc, &rc, izpp ); /* invert off */
				zppT.zpMin = max( grpzpp.zppAboveThumb.zpMin,
						min( ZpFromPt(ptMouse) - dzpAway,
						grpzpp.zppBelowThumb.zpMac - dzpThumb ));
				zppT.zpMac = zppT.zpMin + dzpThumb;
				RSBZppToRc( hwnd, &zppT, &rc );
				RSBInvertForTrack( hwnd, hdc, &rc, izpp ); /* invert on */
				ptInvert = ptMouse;
				}
			}
		RSBInvertForTrack( hwnd, hdc, &rc, izpp ); /* invert off */
		if (ZpFromPt(ptInvert) != ZpFromPt(ptHit))
			{
			int spsT = UMultDiv( ZpFromPt(rc.ptTopLeft) - grpzpp.zppAboveThumb.zpMin,
					spsLim,
					grpzpp.zppBelowThumb.zpMac - grpzpp.zppAboveThumb.zpMin - dzpThumb);

			SendMessage( hwndParent, msg, SB_THUMBPOSITION, 
					MAKELONG(spsT, hwnd ));
			};
		}
	ReleaseDC( hwnd, hdc );
	ReleaseCapture();

#undef ZpFromPt
}


/*************************************************************************/
/***********************  U T I L I T I E S ******************************/
/*************************************************************************/
#ifdef WIN23
csconst int mpibmsobm2 [] = { 
	OBM_OLD_UPARROW, OBM_OLD_DNARROW, OBM_OLD_LFARROW, OBM_OLD_RGARROW,
			OBM_BTSIZE
			};
csconst int mpibmsobm3 [] = { 
	OBM_UPARROW, OBM_DNARROW, OBM_LFARROW, OBM_RGARROW, OBM_BTSIZE,
	/* depressed arrow bitmaps */
	OBM_UPARROWD, OBM_DNARROWD, OBM_LFARROWD, OBM_RGARROWD
			};
#else
csconst int mpibmsobm [] = { 
	OBM_UPARROW, OBM_DNARROW, OBM_LFARROW, OBM_RGARROW,
			OBM_BTSIZE
			};
#endif /* WIN23 */


/*********************/
/* F  I n i t  R S B */

/* %%Function:FInitRSB %%Owner:bryanl */
FInitRSB()
{
	HANDLE hDisplay;
	int ibms;
	struct BMS *pbms;
	BITMAP bm;
#ifdef WIN23
	int ibmsMax = vsci.fWin3 ? ibmsMax3: ibmsMax2;
#endif /* WIN23 */

	if ((hDisplay = GetModuleHandle((LPSTR)SzFrame("DISPLAY"))) == NULL)
		return fFalse;


	for ( ibms = 0, pbms = vrsbi.rgbms; ibms < ibmsMax ; ibms++, pbms++ )
		{
#ifdef WIN23
		if ((pbms->hbm = LoadBitmap( hDisplay, 
				MAKEINTRESOURCE(vsci.fWin3 ? mpibmsobm3 [ibms] :
				mpibmsobm2 [ibms]))) == NULL)
#else
		if ((pbms->hbm = LoadBitmap( hDisplay, 
				MAKEINTRESOURCE(mpibmsobm [ibms]))) == NULL)
#endif /* WIN23 */
			return fFalse;
		LogGdiHandle(pbms->hbm, 1080 + ibms);
		if (GetObject( pbms->hbm, sizeof(BITMAP), (LPSTR)&bm) == 0)
			return fFalse;
		pbms->dxp = bm.bmWidth;
		pbms->dyp = bm.bmHeight;
		}

	return fTrue;
}


/*************************************/
/* R S B  C o m p u t e  G r p z p p */
/* compute location & size of various elements of scroll bar */

/* %%Function:RSBComputeGrpzpp %%Owner:bryanl */
RSBComputeGrpzpp( hwnd, sps, pgrpzpp )
HWND hwnd;
int sps;
union GRPZPP *pgrpzpp;
{
	int spsLim = GetWindowWord( hwnd, offset( RSBS, spsLim ) );
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	int bzpPt = (fVert ? offset(PT,yp) : offset(PT,xp) );
#define ZpFromPt(pt)	 *((int *)((char *)&(pt) + bzpPt))
	int zpThumb;
	int zpFirst, zpLim, dzpThumb;
	int dzpU, dzpD;
	struct BMS *pbmsU, *pbmsD;
	struct RC rc;

	GetClientRect( hwnd, (LPRECT)&rc );
	zpFirst = 0;
	zpLim = ZpFromPt(rc.ptBottomRight);
	pbmsU = PbmsFromFlags( fVert, fTrue /*fTopLeft*/ );
	pbmsD = PbmsFromFlags( fVert, fFalse /*fTopLeft*/ );
	Assert( offset(BMS, dyp) == offset(PT,yp) &&
			offset(BMS, dxp) == offset(PT,xp) );
	dzpU = ZpFromPt(*pbmsU);
	dzpD = ZpFromPt(*pbmsD);

	pgrpzpp->zppUArrow.zpMin = 0;
	pgrpzpp->zppDArrow.zpMac = zpLim;

	if (zpLim - zpFirst < dzpU + dzpD + 1)
/* CRUNCH CASE 1: too small to display complete arrows: shrink arrows equally */
		dzpU = dzpD = (zpLim - zpFirst - 1) / 2;

	pgrpzpp->zppAboveThumb.zpMin = pgrpzpp->zppUArrow.zpMac = (zpFirst += dzpU);
	pgrpzpp->zppBelowThumb.zpMac = pgrpzpp->zppDArrow.zpMin = (zpLim -= dzpD);

	if (zpLim - zpFirst < (dzpThumb = GetSystemMetrics(
			fVert ? SM_CYVTHUMB : SM_CXHTHUMB)) + 1)
/* CRUNCH CASE 2: too small to display the thumb */
		{
		zpThumb = zpLim;
		dzpThumb = 0;
		}
	else
		zpThumb = UMultDiv( sps, zpLim - zpFirst - dzpThumb, spsLim ) + zpFirst;

	pgrpzpp->zppAboveThumb.zpMac = pgrpzpp->zppThumb.zpMin = zpThumb;
	pgrpzpp->zppThumb.zpMac = pgrpzpp->zppBelowThumb.zpMin = zpThumb + dzpThumb;

#undef ZpFromPt
}


/**************************/
/* R S P  Z p p  T o  R c */

/* %%Function:RSBZppToRc %%Owner:bryanl */
RSBZppToRc( hwnd, pzpp, prc )
HWND hwnd;
struct ZPP *pzpp;
struct RC *prc;
{
	int fVert = GetWindowWord( hwnd, offset( RSBS, fVert ) );
	int bzpPt = (fVert ? offset(PT,yp) : offset(PT,xp) );
#define PzpFromPt(pt)	 ((int *)((char *)&(pt) + bzpPt))

	GetClientRect(hwnd, (LPRECT)prc );
	*PzpFromPt(prc->ptTopLeft) = pzpp->zpMin;
	*PzpFromPt(prc->ptBottomRight) = pzpp->zpMac;

#undef PzpFromPt
}


struct BMS *PbmsFromFlags( fVert, fTopLeft )
int fVert, fTopLeft;
{
	Assert( offset(RSBI,bmsUArrow) < offset(RSBI,bmsDArrow) &&
			offset(RSBI,bmsDArrow) < offset(RSBI,bmsLArrow) &&
			offset(RSBI,bmsLArrow) < offset(RSBI,bmsRArrow) );

	return &vrsbi.rgbms [((!fVert) << 1) + (!fTopLeft)];
}


