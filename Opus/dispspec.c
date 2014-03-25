/* D I S P S P E C . C */

#define WINDOWS

#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "file.h"
#include "ch.h"
#include "props.h"
#include "border.h"
#include "format.h"
#include "formula.h"
#include "disp.h"
#include "scc.h"
#include "sel.h"
#include "prm.h"
#include "print.h"
#include "style.h"
#include "doc.h"
#include "ruler.h"
#include "heap.h"
#include "debug.h"


extern struct WWD       **mpwwhwwd[];
extern struct DOD       **mpdochdod[];
extern int              wwCur;
extern struct FLI       vfli;
extern struct WWD       **hwwdCur;
extern int              catrLast;
extern struct SCC       vsccAbove;
extern struct SCI	vsci;
extern struct FTI       vfti;
extern struct PRSU      vprsu;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern struct PAP       vpapFetch;
extern struct PRI       vpri;
extern struct TAP       vtapFetch;
extern int		vlm;
extern int		vfPrvwDisp;
extern struct SCI vsci;
extern struct MWD **hmwdCur;
extern int        mwMac;
extern int        wwMac;
extern int        wwCur;
extern int        mwCur;
extern struct MERR      vmerr;
extern HCURSOR    vhcArrow;
extern HCURSOR    vhcStyWnd;
extern struct MWD **mpmwhmwd[];
extern struct CA  caPara;
extern struct WWD **mpwwhwwd[];
extern struct DOD **mpdochdod[];
extern struct PAP  vpapFetch;
extern int         vdypStyWndFont;
extern struct FLI  vfli;
extern struct PREF vpref;
extern HFONT       vhfntStyWnd;
extern struct SEL selCur;
extern struct WWD **hwwdCur;
extern struct SCC       vsccAbove;
extern struct WWD **vhwwdOrigin;
extern struct PREF      vpref;
extern HBRUSH		vhbrGray;
extern HBRUSH		vhbrBlack;
extern int		vwwClipboard;
struct RC *PrcSelBar();

static BOOL fRecurse = fFalse;

/* decodes tlc (tab leader code) */
csconst char            mptlcch[] = " .-_";


/* S C R O L L  H O R I Z */
/* Scroll the pane ww horizontally by dxpScroll (>0 scroll text to left)
In case of split windows, scroll both panes if wk's are the same, otherwise
just scroll the single pane ww.
*/
/* %%Function:ScrollHoriz %%Owner:chic %%reviewed 7/10/89 */
ScrollHoriz(ww, dxpScroll)
int ww, dxpScroll;
{
	int wwSplit;

	ScrollHoriz1(ww, dxpScroll);
	if ((wwSplit = WwScrollWkTest(ww)) != wwNil)
		ScrollHoriz1(wwSplit, dxpScroll);

/* "one per window" actions */
	SyncSbHor(ww);

	/* if we have a ruler, redraw invalidated scale and tabs */
	if (PmwdWw(ww)->hwndRuler != NULL)
		{
		/* Scroll ruler's scale and tabs. */
		ScrollRuler();
		}
}


/* S C R O L L  H O R I Z  1 */
/* perform scrolling for a single pane by dxpScroll (>0 scroll text to left) */
/* %%Function:ScrollHoriz1 %%Owner:chic %%reviewed 7/10/89 */
ScrollHoriz1(ww, dxpScroll)
{
	struct WWD *pwwd = PwwdWw(ww);
	BOOL fPageView = pwwd->fPageView;
	HDC hdc = pwwd->hdc;
	struct RC rcUpdate;

	ClearInsertLine(&selCur);
	ClearInsertLine(&selDotted);

/* make sure WINDOW base invalidation is clean up first, e.g. dialog box taken down */
	if (ww != vwwClipboard)
		UpdateWindow(PwwdWw(ww)->hwnd);

/* round dxpScroll so that gray lines will be shown seamlessly */
	dxpScroll = (dxpScroll + 4) & (~07);

/* adjust pwwd's rcePage, rcwDisp, xhScroll */
	SetScrollHoriz(ww, dxpScroll);

/* accumulate internal record of inval area */
	pwwd = PwwdWw(ww);
	if (!FEmptyRc(&pwwd->rcwInval))
		OffsetRect((LPRECT) &pwwd->rcwInval, -dxpScroll, 0);

/* do the scroll - do not use ScrollWindow, it is too slow */
	ScrollDC( hdc, -dxpScroll, 0, (LPSTR) &pwwd->rcwDisp,
			(LPSTR) &pwwd->rcwDisp, (HRGN) NULL, (LPRECT) &rcUpdate );

	InvalWwRc( ww, &rcUpdate );

	if (!fPageView)
		{
		/* erase exposed area */
		PatBltRc( hdc, &rcUpdate, vsci.ropErase );
		/* erase selection bar */
		PatBltRc(hdc, PrcSelBar( ww, &rcUpdate ), vsci.ropErase);
		/* always invalidate the last dl (so end mark gets redrawn) */
		InvalLastDl(ww);
		/* update scc's */
		SynchSccWw(&vsccAbove, wwCur);
		}
	else
		{ /* finish up the page boundary/background stuff */
		DrawBlankPage(ww, &rcUpdate);
		}
}


/* I N V A L  L A S T  D L */

/* %%Function:InvalLastDl %%Owner:tomsax */
InvalLastDl(ww)
int	ww;
{
	struct PLCEDL	**hplcedl = HplcedlWw(ww, 0);
	struct EDL	edl;
	struct RC	rcw;

	AssertH(hplcedl);
	Assert(IMacPlc(hplcedl) > 0);
	GetPlc(hplcedl, IMacPlc(hplcedl) - 1, &edl);
	if (edl.fEnd)
		{
		DrcpToRcw(HwwdWw(ww), 0, &(edl.drcp), &rcw);
		InvalWwRc(ww, &rcw);
		}
}


/* S C R O L L  L E F T */
/* Scroll current window left dxp pixels */
/* scrolls the text to the left (allowing us to look at text more
to the right).  xh is decremented.
Stop scrolling at home if the home point were to be crossed by text moving
from the right to the left. */
/* %%Function:ScrollLeft %%Owner:tomsax */
ScrollLeft(ww, dxp)
int ww;
int dxp;
{
	struct WWD	*pwwd = PwwdWw(ww);
	int		dxpHome;
	int		dxpDisp = DxOfRc(&pwwd->rcwDisp);

	if ((dxp = min(dxp,
			(pwwd->fPageView ? 
			max(xeLeftPage(ww), DxOfRc(&pwwd->rcePage) - dxpDisp + dxpGrayOutsideSci) :
			DxsFromDxa(pwwd, xaRightMaxSci) - dxpDisp)
			+ pwwd->xhScroll)) <= 0)
		/* would scroll too far */
		return;
	else
		{
		/* how far are we from home point? */
		if ((dxpHome = DxpScrollHorizWw(ww)) > 3) /* 3 because dxp will be rounded in ScrollHoriz1 */
			dxp = min(dxp, dxpHome); /* if cross => go to home */
		ScrollHoriz(ww, dxp);
		}
}


/* S C R O L L  R I G H T */
/* scrolls the text to the right (allowing us to look at text more
to the left). xh is incremented.
Stop scrolling at home if the home point were to be crossed by text moving
from the left to the right. */
/* %%Function:ScrollRight %%Owner:tomsax */
ScrollRight(ww, dxp)
int ww;
int dxp;
{
	struct WWD *pwwd = PwwdWw(ww);
	int dxpHome;

	if ((dxp = min(dxp, (pwwd->fPageView ? dxpGrayOutsideSci :
			DxsFromDxa(pwwd, -xaLeftMinSci))
			- pwwd->xhScroll)) <= 0)
		/* would scroll too far */
		return;
	else
		{
		/* how far are we from home point ? */
		if ((dxpHome = DxpScrollHorizWw(ww)) < -3) /* 3 because dxp will be rounded in ScrollHoriz1 */
			dxp = min(dxp, -dxpHome); /* if cross => go to home */
		ScrollHoriz(ww, -dxp);
		}
}


/* D X P  S C R O L L  H O R I Z  W W */
/* Returns a quantity dxp representing the horiz scroll state of ww such that:
dxp<0: text is to the left of its home position
dxp=0: text is at its home position
dxp>0: text is to the right of the home position,
with abs(dxp) being the distance from home.
*/
/* %%Function:DxpScroll %%Owner:tomsax */
DxpScrollHorizWw(ww)
int ww;
{
	struct WWD *pwwd = PwwdWw(ww);

	if (pwwd->fPageView)
		return(pwwd->xeScroll - XeLeftPage(ww));
	else
		return(pwwd->xhScroll);
}


/* E N D  U  L */
/* draw underlines from a point described in uls to the current pen position
(which is saved and restored.) Pen is at the baseline.
Graphport and clipping is set up by DisplayFli.
ichLim is the first ich not to be underlined (by this call at least).

		kulSingle: for printer, no need to do anything, was handled by LoadFont
					for screen, draw a line
		kulDouble: for printer, first underline was handled by Loadfont;
								next one is done by putting out underlined spaces
					for screen, draw two lines
Line positions relative to the baseline can be adjusted herein.
*/

/* %%Function:EndUL %%Owner:tomsax */
EXPORT EndUL(puls, prcClip)
struct ULS *puls;
struct RC *prcClip;
{
	int kul = puls->kul;
	HDC hdc = puls->hdc;
	int xpFirst = puls->pt.xp;
	int xpLim = puls->xwLim;
	int ypBase = puls->pt.yp;
	int ypSingle;
	int ypDouble;
	int xwMin;
	int dxp;

	if (vfli.fPrint)
		{
		EndULPrint(puls);
		goto LRet;
		}

	if (xpFirst < (xwMin = PwwdWw(puls->ww)->xwMin))
		xpFirst = xwMin;

/* following checks are in lieu of clipping */
	if ((dxp = min( xpLim, prcClip->xpRight)  - xpFirst) <= 0)
		goto LRet;

	ypSingle = ypBase;
	ypDouble = min( ypBase + 1 + vsci.dypBorder,
			ypBase + puls->dypPen + vfli.dypBase - 1 );

/* clipping checks continued... */
	if (ypBase < vsci.dypMinWwInit)
		goto LRet;  /* baseline is above clip area -- don't show underline */
	ypBase = puls->kul == kulDouble ? ypDouble : ypSingle;
	if (ypBase >= prcClip->ypBottom)
		goto LRet;  /* baseline is below clip area -- don't show underline */

/* if necessary, move up single underline to accomodate double line */
	if (kul == kulDouble && ypSingle == ypDouble)
		ypSingle--;

	switch (puls->kul)
		{
#ifdef DEBUG
	default:            
		Assert( fFalse );
	case kulNone:       
		break;
#endif  /* DEBUG */
	case kulDotted:
		Assert(!vfPrvwDisp);
		DrawPatternLine( hdc, xpFirst, ypSingle, dxp, ipatHorzLtGray,
				pltHorz + pltOnePixel );
		break;
	case kulDouble:
		if (vfPrvwDisp)
			DrawPrvwLine(hdc, xpFirst, ypDouble, dxp, 1, colFetch);
		else
			DrawPatternLine( hdc, xpFirst, ypDouble, dxp, ipatHorzBlack, pltHorz );
		/* FALL THROUGH */
	case kulSingle:
	case kulWord:
		if (vfPrvwDisp)
			DrawPrvwLine(hdc, xpFirst, ypSingle, dxp, 1, colFetch);
		else
			DrawPatternLine( hdc, xpFirst, ypSingle, dxp, ipatHorzBlack, pltHorz );
		break;
		}
LRet:
	puls->grpfUL = 0;
}


#ifdef DEBUG
/* E n d  U L  P a s t */
/* Terminate underline; limit underline to not past vfli.ichSpace3
	given current ich and corresponding xp */

/* %%Function:EndULPast %%Owner:tomsax */
HANDNATIVE EndULPast(puls, xpLim, ichLim, prcClip)
struct ULS *puls;
int xpLim, ichLim;
struct RC *prcClip;
{
	while (--ichLim >= (int)vfli.ichSpace3)
		xpLim -= vfli.rgdxp [ichLim];

	if ((puls->xwLim = xpLim) > puls->pt.xp)
		EndUL(puls, prcClip);
}


#endif /* DEBUG */


#define	cdywFatLine	2

/* F R A M E  D R */
/* Given a window and a dr will draw borders around the dr. */
/* Pass fFat==fTrue to draw a thick line at the bottom of the DR */
/* F R A M E  D R */
/* Given a window and a dr will draw borders around the dr. */
/* Pass fFat==fTrue to draw a thick line at the bottom of the DR */
/* %%Function:FrameDr %%Owner:tomsax */
EXPORT FrameDr( ww, hpldr, idr, fFat )
struct PLDR **hpldr;
BOOL fFat;
{
	struct PLC **hplcedl;
	struct DR *pdr;
	int iMac;
	struct WWD *pwwd;
	struct RC rcw;
	struct EDL edl;
	struct DRF drfFetch;

	/* get info about our DR */
	DrclToRcw( hpldr, &(pdr=PdrFetch(hpldr,idr,&drfFetch))->drcl, &rcw );
	hplcedl = pdr->hplcedl;
	Assert ( !FEmptyRc(&rcw) );
	Assert (FDrawPageDrsWw(ww) || fFat);

	if (hplcedl != hNil && (iMac = IMacPlc(hplcedl)) > 0)
		{
		GetPlc ( hplcedl, iMac - 1, &edl );
		rcw.ywTop = YwFromYp(hpldr,idr,edl.ypTop+edl.dyp);
		}
	FreePdrf(&drfFetch);

	if (rcw.ywTop < rcw.ywBottom)
		FrameDrLine(ww, hpldr, idr, rcw.ywTop, rcw.ywBottom, fFat, fTrue/*fForceLine*/, fFalse/*fInnerDr*/);
}


/* F R A M E  M A R G I N S  W W */
/* %%Function:FrameMarginsWw %%Owner:chic %%reviewed 7/10/89 */
FrameMarginsWw(ww)
int ww;
{
#define dxpFrameLine (vsci.dxpBorder)
#define dypFrameLine (vsci.dypBorder)

	struct WWD *pwwd = PwwdSetWw(ww, cptDoc);
	HDC hdc = pwwd->hdc;
	int dx, dy;
	int grpf;
	struct RC rcw, rcwDraw, rcwPage, rcwDrawDest;

	GetMarginsRcw(&rcw, ww);
	RceToRcw(PwwdWw(ww), &pwwd->rcePage, &rcwPage);

	dx = DxsFromDxa(pwwd,dxaInch>>2) - dxpFrameLine;
	dy = DysFromDya((czaInch>>2)) - dypFrameLine;


#undef  fTop
#undef  fLeft
#undef  fUseDx
#define fTop   (grpf & 1)
#define fLeft  (grpf & 2)
#define fUseDx (grpf & 4)

	for (grpf = 0; grpf < 8; grpf++)
		{
/* In the order of Bottom Right, Bottom Left, Upper Right, Upper Left.
	grpf from 0 to 3: draw vertical rect
	grpf from 4 to 7: draw horizontal rect
*/
		/* determine origin point: Left, Right, Top or Bottom */
		rcwDraw.xwLeft = rcwDraw.xwRight = fLeft ?
				rcw.xwLeft : rcw.xwRight;
		rcwDraw.ywTop = rcwDraw.ywBottom = fTop ?
				rcw.ywTop : rcw.ywBottom;

		/* fill a vertical or horizontal rect */
		if (fLeft)
			rcwDraw.xwLeft -= dxpFrameLine + (fUseDx ? dx : 0);
		else
			rcwDraw.xwRight += dxpFrameLine + (fUseDx ? dx : 0);
		if (fTop)
			rcwDraw.ywTop -= dypFrameLine + (fUseDx ? 0 : dy);
		else
			rcwDraw.ywBottom += dypFrameLine + (fUseDx ? 0 : dy);

		SectRc(&rcwDraw, &rcwPage, &rcwDrawDest);
		FillRect(hdc, (LPRECT)&rcwDrawDest, vhbrGray);
		}
#undef  fTop
#undef  fLeft
#undef  fUseDx
}


/* G E T  M A R G I N S  R C W */
/* %%Function:GetMarginsRcw %%Owner:chic %%reviewed 7/10/89 */
GetMarginsRcw(prcw, ww)
struct RC *prcw;
{
	int doc = DocMother(DocBaseWw(ww));
	int ipgd = PwwdWw(ww)->ipgd;
	struct DOD *pdod = PdodDoc(doc);
	struct DOP *pdop = &pdod->dop;
	struct PGD pgd;

	FreezeHp();
	if (ipgd != ipgdNil && ipgd < IMacPlc(pdod->hplcpgd))
		GetPlc(pdod->hplcpgd, ipgd, &pgd);
	else
		pgd.pgn = 0;

	GetXlMargins(pdop, pgd.pgn & 1, WinMac(vfli.dxsInch, DxsSetDxsInch(doc)),
			&prcw->xlLeft, &prcw->xlRight);
	prcw->ylTop = DysFromDya(abs(pdop->dyaTop));
	prcw->ylBottom = DysFromDya(pdop->yaPage - abs(pdop->dyaBottom));
	MeltHp();

	RclToRcw(HwwdWw(ww), prcw, prcw);
}



/* D R A W  T L C */
/* draws leader dots or underlined tab starting at current pos, returns
to same pos */
/* %%Function:DrawTlc %%Owner:tomsax */
EXPORT DrawTlc(hdc, pt, tlc, dxp, pchp, fOver, prcwClip)
HDC hdc;
struct PT pt;
int tlc, dxp;
struct CHP *pchp;
BOOL fOver;
struct RC *prcwClip;
{
	int c, cchLast;
	int ch;
	int dxpCh, dxtCh, dxpAdj;
	int yp;
	int bkm;
	int fClippedLeft = fFalse;
	char rgch[32];

	if (tlc == tlcHeavy)
		tlc = tlcSingle;

/* take care of clipping */
	if (pt.xp < prcwClip->xpLeft)
		{
		dxp -= prcwClip->xpLeft - pt.xp;
		pt.xp = prcwClip->xpLeft;
		fClippedLeft = fTrue;
		}
	if (pt.xp + dxp > prcwClip->xpRight)
		{
		dxp = prcwClip->xpRight - pt.xp;
		fOver = fTrue;
		}		
	if (dxp < 0 || pt.yp < prcwClip->ypTop || pt.yp >= prcwClip->ypBottom)
		return;

	ch = mptlcch[tlc];
	SetBytes(&rgch[0], ch, 32);
/* to gain speed, we output 32 leader characters at a time */
	LoadFont(pchp,fFalse/*fWidthsOnly*/);
#ifdef BRYANL
	{
	CommSzNum( SzShared("DrawTlc: DxpFromCh result is "), DxpFromCh( ch, &vfti ));
	CommSzNum( SzShared("  GetTextExtent X 1 result is "), (int)GetTextExtent( hdc, (LPSTR)&ch, 1 ));
	CommSzNum( SzShared("  GetTextExtent X 32 result is "), (int)GetTextExtent( hdc, (LPSTR)rgch, 32 ));
	}
#endif

	yp = pt.yp - vfti.dypAscent;
	/* we always use vfti because its units always match rgdxp */
	dxtCh = dxpCh = DxpFromCh(ch, &vfti);
	if (dxpCh == 0)
		dxpCh = dxtCh = 1;

/* advance to next multiple of dxpCh */
	dxpAdj = tlc == 0 ? 0 : (pt.xp + dxpCh - 1) / dxpCh * dxpCh - pt.xp;
	if (fClippedLeft && dxpAdj > 0)
		dxpAdj -= dxpCh;
	pt.xp += dxpAdj;

	c = (dxp - dxpAdj + (fOver ? dxtCh - 1 : 0)) / dxtCh;
	cchLast = c & 31;
	c >>= 5;

/* set transparent mode for printers like Epson which will,
	like screens, white out over existing stuff */
	bkm = SetBkMode( hdc, TRANSPARENT );

	while (c-- > 0)
		{
		TextOut( hdc, pt.xp, yp, (LPSTR) rgch, 32 );
		pt.xp += dxpCh << 5;
		}
	TextOut( hdc, pt.xp, yp, (LPSTR) rgch, cchLast );
	SetBkMode( hdc, bkm );
}



/* %%Function:FrameRectRop %%Owner:tomsax */
FrameRectRop(hdc,prc,rop)
HDC hdc;
struct RC *prc;
unsigned long rop;
{
	/*  Uses PatBlt to draw a rectangle using raster-op rop. */
	int dxp = prc->xpRight - prc->xpLeft;
	int dyp = prc->ypBottom - prc->ypTop - (vsci.dypBorder << 1);

	/* top */
	PatBlt( hdc, prc->xpLeft, prc->ypTop, dxp, vsci.dypBorder, rop);

	/* right */
	PatBlt( hdc, prc->xpRight - vsci.dxpBorder, prc->ypTop + vsci.dypBorder,
			vsci.dxpBorder, dyp, rop);

	/* left */
	PatBlt( hdc, prc->xpLeft, prc->ypTop + vsci.dypBorder,
			vsci.dxpBorder, dyp, rop);

	/* bottom */
	PatBlt( hdc, prc->xpLeft, prc->ypBottom - vsci.dypBorder,
			dxp, vsci.dypBorder, rop);
}


/* %%Function:DrawStyNameBorder %%Owner:tomsax */
DrawStyNameBorder(hdc, prc)
HDC hdc;
struct RC *prc;
{
	/* Save the brush */
	HBRUSH hbrOld = SelectObject(hdc, vsci.hbrText);

	PatBltRc(hdc, prc, PATCOPY);

	/* Restore the brush */
	if (hbrOld != NULL)
		SelectObject(hdc, hbrOld);
}


#define xpTextLeft (3)

/* %%Function:DrawStyNameFromWwDl %%Owner:tomsax */
EXPORT DrawStyNameFromWwDl(ww, hpldr, idr, dl)
int ww;
struct PLDR **hpldr;
int idr;
int dl;
{
	struct WWD **hwwd = mpwwhwwd[ww];
	struct WWD *pwwd = *hwwd;
	int xpStyWnd = pwwd->xwSelBar;
	HDC hdc = pwwd->hdc;
	int eto = ETO_OPAQUE | ETO_CLIPPED;
	CP cpStart;
	HFONT hFontOld;
	HBRUSH hbrOld;
	struct RC rc;
	int yp;
	struct EDL edl;
	struct PT pt;
	int dyPToW;
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	int doc;
	struct DRF drfFetch;
	CHAR stStyName[cchMaxStyle];

	pdr = PdrFetch(hpldr, idr, &drfFetch);
	hplcedl = pdr->hplcedl;
	doc = pdr->doc;

	if (hdc == NULL)
		/* screen cache */
		{
		hdc = vsci.hdcScratch;
		}

	Assert(hdc);
	FreezeHp();

	/* Get a font for the styname area */
	if (vhfntStyWnd == NULL)
		{ /* fill in a fcid and create vhfntStyWnd */
		union FCID fcid;
		LOGFONT lf;

		SetBytes(&fcid, 0, sizeof(union FCID));
		fcid.hps = hpsDefault; /* desired height in half point */
		fcid.ibstFont = 0; /* default font index into vhsttbFont */
		FGraphicsFcidToPlf( fcid, &lf, fFalse /* fPrinterFont */);
		if ((vhfntStyWnd = CreateFontIndirect((LPLOGFONT)&lf)) == NULL)
			{
			MeltHp();
			goto LRet;
			}
		else
			{ /* get font height */
			TEXTMETRIC tm;

			LogGdiHandle(vhfntStyWnd, 1013);
			GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);
			vdypStyWndFont = tm.tmAscent;
			}
		}
	hFontOld = SelectObject(hdc, vhfntStyWnd);

	GetPlc( hplcedl, dl, &edl );
	/* calculate clip rect for ExtTextOut */
	SetRect( &rc, 0, edl.ypTop, xpStyWnd, edl.ypTop + edl.dyp );
	pt = PtOrigin(hpldr, idr);
	pwwd = *vhwwdOrigin;
	dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;
	rc.ywTop += dyPToW;
	rc.ywBottom += dyPToW;

	cpStart = CpPlc( hplcedl, dl );
	MeltHp();
	if (cpStart < CpMacDoc(doc))
		{
		CachePara(doc, cpStart);
		if (caPara.cpFirst == cpStart && !FInTableDocCp(doc, cpStart))
			{ /* we are at the first line of a paragraph, show style name */
			int cstcStd = PdodMother(doc)->stsh.cstcStd;
			int stcp = StcpFromStc(vpapFetch.stc, cstcStd);

			GenStyleNameForStcp(stStyName, PdodMother(doc)->stsh.hsttbName, cstcStd, stcp);
			if (vfli.doc != doc || vfli.cpMin != cpStart)
				FormatLine(ww, doc, cpStart);

			yp = rc.ypBottom - vfli.dypBase - vdypStyWndFont;
			if (yp + vdypStyWndFont >= rc.ypBottom)
				yp = rc.ypTop;
			else  if (yp < rc.ypTop)
				yp = rc.ypTop;
			/* don't write into tiny strip above, clip to pwwd->ywMin */
			rc.ypTop = max(rc.ypTop, (*vhwwdOrigin)->ywMin);
			Assert(rc.ypTop < rc.ypBottom);
			ExtTextOut(hdc, xpTextLeft, yp, 
					eto,                /* action bits */
					&rc,                /* opaque rect */
					&stStyName[1],      /* string */
					stStyName[0],       /* length */
			(int far *)0);      /* lpdx */
			}
		else
			PatBltRc(hdc, &rc, vsci.ropErase); /* erase background ourself */
		}

	/* Restore the origianl font to hdc */
	if (hFontOld != NULL)
		SelectObject(hdc, hFontOld);


	/* Draw border */
	rc.xpLeft = xpStyWnd - vsci.dxpBorder;
	rc.xpRight = xpStyWnd;
	DrawStyNameBorder(hdc, &rc);
LRet:
	FreePdrf(&drfFetch);
}


/* P a i n t  S t y  S t u b */
/* if passed rect intersects portion of sty border above ww's ypMin,
	paint the stub that lives up there */

/* %%Function:PaintStyStub %%Owner:tomsax */
PaintStyStub(ww, prc)
int ww;
struct RC *prc;
{
	struct WWD *pwwd = PwwdWw(ww);
	struct RC rcBorder;

	if (!pwwd->xwSelBar)
		return;

/* draw portion of style name window border that extends above ypMin */
/* This is the only drawing/invalidation that we have to do in this area */

	if (!FEmptyRc(prc) && prc->ypTop < pwwd->ywMin)
		{
		SetRect( &rcBorder, pwwd->xwSelBar - vsci.dxpBorder, 0,
				pwwd->xwSelBar, pwwd->ywMin );
		DrawStyNameBorder( pwwd->hdc, &rcBorder );
		}
}

export
ExtTextOutKludge(hdc, xw, yw, eto, lprcw, pch, cch, lpdx)
HDC hdc;
int xw, yw;
int eto;
LPRECT lprcw;
char far * pch;
int cch;
int far * lpdx;
{
	int ich, cLoop;
	int xwCur, dxwSpace, dxwChar, dxwFill;
	int bkmodeSav;
	TEXTMETRIC tm;
	char chSpaceLocal = chSpace;

	Assert(lpdx != 0L);

	GetTextMetrics(hdc, (TEXTMETRIC far *) &tm);
          
	dxwSpace = GetTextExtent(hdc, (LPSTR)&chSpaceLocal, 1) - tm.tmOverhang;
	if (eto & ETO_OPAQUE)
		PatBltRc(hdc, lprcw, vsci.ropErase);
	eto &= ETO_CLIPPED;

	if (tm.tmUnderlined)
		{
		for (xwCur = dxwFill = ich = 0; ich < cch; ich++)
			{
			dxwChar = GetTextExtent(hdc, (LPSTR)&pch+ich, 1) - tm.tmOverhang;

			/* kludgy loop to reduce code size */
			for(dxwFill += dxwChar, cLoop = 2;	cLoop--;
					dxwFill += (cLoop ? lpdx[ich] - dxwChar : 0))
				while (dxwFill >= dxwSpace)
					{
					ExtTextOut(hdc, xw+xwCur, yw, eto, lprcw,
						(LPSTR)&chSpaceLocal, 1, (LPSTR) 0L);
					dxwFill -= dxwSpace;
					xwCur += dxwSpace;
					}
	      }
	   /* line up last filler to go to end of extent */
	   if (dxwFill)
			{
			ExtTextOut(hdc, xw + xwCur - dxwSpace + dxwFill, 
				yw, eto, lprcw, (LPSTR)&chSpaceLocal, 1, (LPSTR) 0L);
			}
		}

	/* output the actual characters */
	bkmodeSav = SetBkMode(hdc, TRANSPARENT);
	while (--cch >= 0)
		{
		ExtTextOut(hdc, xw, yw, eto, lprcw, pch++, 1, (LPSTR) 0L);
		xw +=  *lpdx++;
		}
   SetBkMode(hdc, bkmodeSav);
}
