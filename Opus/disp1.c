/* D I S P 1 . C */
#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "ch.h"
#include "format.h"
#include "scc.h"
#include "field.h"
#include "resource.h"
#define REVMARKING
#include "compare.h"
#include "debug.h"
#include "print.h"
#include "idle.h"
#include "border.h"
#include "rareflag.h"

#ifdef DEBUG
/* #define DFONT */
/* #define SHOWFLD */
#ifdef PCJ
/* #define DFLDSELECT */
#endif /* PCJ */
#endif


#ifdef DFONT
int vfDbgRTFlag = fFalse;
#endif

/* G L O B A L S */
struct FLI      vfli;
struct FMTSS    vfmtss;
int             vflm=flmIdle;   /* FormatLine mode */
struct FPC	vfpc;		/* fast plc cache */
struct FFS *    vpffs;          /* display field formatting info */

/* pointer to heap block containing chr's, byte based Max, and Mac of the block */
int             vbchrMax;
int             vbchrMac;
struct SEL      selCur;
struct CHP	vchpGraySelCur;
struct PAP	vpapSelCur;
struct PAP	vpapGraySelCur;
struct SEL      selDotted;
struct TCC      vtcc;
struct TCXS     vtcxs;
int             vitcMic;
CP              vmpitccp[itcMax+1];
struct CA	caTap;
struct TAP      vtapFetch;
int		vitcMicAux;
CP		vmpitccpAux[itcMax+1];
struct CA	caTapAux;
struct TAP	vtapFetchAux;
int             vxpFirstMark;
int             vxpLimMark;
int             vylDrCursor;
BOOL		vfEndCursor;	/* set false iff sel has been "extended"
to cpFirst of next line (like fInsEnd) */
struct PLDR	**vhpldrRetSel;	/* return values from CpFirstSty */
int		vidrRetSel;

struct SCC      vsccAbove;
int             vfLastScrollDown;
int		vcConseqScroll = 0;
int		dypCS = -1;
int		sbScroll = SB_NIL;
int		sbScrollLast = SB_NIL;
BOOL            vfLastScroll;



/* E X T E R N A L S */
extern struct MERR      vmerr;
extern struct SCI       vsci;
extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern char             (**vhgrpchr)[];
extern int              vbchrMax;
extern int              vbchrMac;
extern int              vflm;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vxpFirstMark;
extern int              vxpLimMark;
extern int              wwCur;
extern struct WWD       **hwwdCur;
extern struct WWD       **mpwwhwwd[];

extern struct DOD       **mpdochdod[];
extern BOOL             vfFliMunged;
extern struct STTB      **vhsttbFont;
extern int              vwWinVersion;

extern struct CA        caPage;
extern struct CA        caSect;
extern struct CA        caPara;

extern struct FMTSS     vfmtss;

extern struct PAP       vpapFetch;

extern struct BMI       vbmiEmpty;

extern BOOL             vfInsertMode;
extern CP               cpInsert;
extern struct DCIB      *vpdcibDisplayFli;

#ifdef WIN
extern struct DRDL      vdrdlDisplayFli;
extern IDF		vidf;
extern HRGN		vhrgnGrey;
extern HRGN		vhrgnPage;
struct RC *PrcSelBar();
extern struct RF	vrf;
#endif /* WIN */

#ifdef DEBUG
extern struct DBS       vdbs;
#endif /* DEBUG */

extern struct PREF      vpref;
extern struct PRI       vpri;

extern int		vlm;
extern int              vfPrvwDisp;
extern struct WWD       **vhwwdOrigin;
extern HBRUSH		vhbrGray;
extern HBRUSH		vhbrBlack;

struct MDCD *PmdcdCacheIdrb();

/* action codes */
#define dacNop 0
#define dacChr 1
#define dacSpacing 2

HRGN		vhrgnGrey = NULL;
HRGN		vhrgnPage = NULL;

/* DisplayFli
Main procedure for displaying a formatted line on the screen.

New features:
		external bitmap to reduce flashing
		more underline modes
		letterspacing supported
		pictures within the lines
		changes in division of labor with UpdateWindow
		invisible character format supported
		super/subscript positioning

Here is a map of the procedure:

DisplayFli()
		fill EDL from FLI (could be done outside)
		determine line position (could be done outside)
		if line not too tall for the bitmap buffer to be used
				save port's old bitmap
				set port to buffer bitmap vqwBmb/cbwBmb long
		clip to line boundaries, incl style bar, erase bitmap.
		mark style bar
		clip out style bar
		if splat
				draw splat and goto LEndLine;

		main loop drawing the line follows:     (moved to DisplayFliCore)
		for (;;)
				switch(dac) the display actions are:
						next chr, including end of line
						change in space width
						special char
						letterspacing
LCalcDac:
				calculate ichNext and next dac
				draw text if any from ich to ichNext
				ich = ichNext
				end of main loop
LEndLine:
		if visible mode draw visible spaces and tabs
		draw selection
		if off screen bitmap was used
				restore bitmap in screen port
				copy bits from buffer to screen
end of DisplayFli....

Display actions (dac's) are:

dacChr:
		next chr is at pchr (note: no heap allocations allowed)
		switch (pchr->chrm)
				chrmVanish:
						incr pchr;
						goto LCalcDac;
				chrmEol:
						treat end underline.
						goto LEndLine;
				chrmTab:
						fill space with filler (decoded from tlc)
						width is rgdxp[ich]...
						incr pchr
						ich++ to skip over the tab character
						calculate new xp
						position to new xp
						goto LCalcDac;
				chrmChp:
						treat end underline.
						incr pchr
						if not picture      ---not needed bz 8/24/87---
								loadfont
						postion according to super/subscript
						treat begin underline, but not over ichSpace3 because
								trailing white space is not to be underlined
						goto LCalcDac;
dacSpacing:
				width of spaces changes. also change the pointer
				to the ichSpace* where the next change is to take place.
				the intervals are: 0:Normal, 1:Extra, 2:Extra+1, 3:Normal
				(initialize iichSpace=0, pichSpace= &ichSpace1, extra space 0)

				switch (++iichSpace)
				1:      set extra space width dxpExtra; pichSpace++;
				2:      set extra space width dxpExtra+1; pichSpace++;
				3:      set extra space width 0; pichSpace = &ichMac;
						stop underlining, if any.
				goto LCalcDac;
				if there is no justification
						iichSpace and pichSpace are initialized so as to
						wait for ichSpace3 from the beginning.
dacNop:
				goto LCalcDac;


The ichNext and dac are calculated as follows:
		1. assume dacChr first
				dac =  dacChr;
				ichNext = pchr->ich;
		2. if change in spacing occurs earlier, assume dacSpacing
				if (*pichSpace < ichNext)
						ichNext = *pichSpace; dac = dacSpacing;
		3. if ichNext > ich (i.e. above actions are not immediate)
				&& letterspacing
				Move(dxp, 0), ichNext = ich+1
				dac = dacNop i.e. new dac to be calc'd.
		4. if ichNext > ich && special
				ichNext = ich+1, dacNop;
				treat special character at ich.
				continue;

to treat special character:
				ich++;
				if picture
						draw picture starting at current point in line
						reposition to end of slug, as with a tab.
				else
						get characters and DrawText the extension of
						the special ch.
				continue;

begin underline:
		when an underline begins, we remember the ich and position (GetPen)
				and the underline bits stored in the same byte of a chp.
				These will be used to control drawing the line when the
				underline ends.

end underline:
		draw underline from starting to ending positions.
		do not lose graphport position...
		New style underline (w.o. underlining spaces) will have to scan
		rgch to draw the line except for spaces.

*/



#ifdef DEBUG
/* D I S P L A Y  F L I */
/* Display formatted line in window pwwd at line dl. ypLine is bottom of
the line. */
/* %%Function:C_DisplayFli %%Owner:bryanl */
HANDNATIVE C_DisplayFli(ww, hpldr/* or hscc or hwwd */, idr, dl, ypLine)
struct PLDR	**hpldr;
int		ww, idr, dl, ypLine;
{
	struct PLCEDL	**hplcedl;
	int		dypToYw;
	int		dxpToXw, xw;
	struct DR	*pdr;
	struct PT	ptOrigin;
	struct WWD	*pwwd, **hwwd;
	BOOL		fInTable;
	BOOL		fFat;
	HDC		hdc;
	BOOL		fRMark;
	int  		fSoftPageBreak = fFalse;
	GRPFVISI	grpfvisi;
	struct EDL	edl;
	struct RC	rcw, rcwErase, rcwClip;
	struct DRC	drcp;
	int 		irmBar;
	struct DRF	drfFetch;
	BOOL            fFrameLines;

	vrf.fInDisplayFli = !vrf.fInDisplayFli;
	Assert (vrf.fInDisplayFli);
	if (!vrf.fInDisplayFli)
		{
		vrf.fInDisplayFli = !vrf.fInDisplayFli;
		return;
		}

	Assert(hpldr != NULL );
	FreezeHp();

	pdr = PdrFetch(hpldr, idr, &drfFetch);
	hplcedl = pdr->hplcedl;

	Assert((!vfli.fPrint || ww == wwNil) && (*hplcedl)->dlMax > dl);

/* Fill up EDL */
	PutCpPlc(hplcedl, dl, vfli.cpMin);
/* Include margin area, to support xwLimScroll optimization */
	edl.dxp = ((vfli.fRight || vfli.fTop || vfli.fBottom)
			?
			max( vfli.xpMarginRight + dxpLeftRightSpace +  
			DxFromBrc(vfli.brcRight,fFalse/*fFrameLines*/), 
			vfli.xpRight)
			:
			vfli.xpRight) - (edl.xpLeft = vfli.xpLeft);

	edl.dcp = vfli.cpMac - vfli.cpMin;
	edl.ypTop = ypLine - (edl.dyp = vfli.dypLine);
	/* set flags below; wipes out dcpDepend and other flags that are set later */
	Assert (!dlkNil);
	edl.grpfEdl = 0;
#ifdef INEFFICIENT
	/* Native Note: zero this whole byte, give or take fHasBitmap */
	edl.fDirty = edl.fTableDirty = fFalse;
	edl.dlk = dlkNil;
	edl.fNeedsEnhance = fFalse;
#endif /* INEFFICIENT */

	fRMark = vfli.fRMark;
	if ((fInTable = pdr->fInTable) != 0)
		edl.fRMark = fRMark;
	edl.dcpDepend = vfli.dcpDepend;

	/* to keep color metafiles out of scc cache */
	edl.fColorMFP = vfli.fMetafile && !vsci.fMonochrome;

	edl.hpldr = hNil;
	if (ww == wwNil)
		{
		edl.fDirty = fTrue;
		PutPlc(hplcedl, dl, &edl);
		MeltHp();
		goto LRet0;
		}
	PutPlc(hplcedl, dl, &edl);

#ifdef WIN
	if (vfli.fPicture)
		{
		vdrdlDisplayFli.hpldr = hpldr;
		vdrdlDisplayFli.idr = idr;
		vdrdlDisplayFli.dl = dl;
		}
#endif /* WIN */

	Debug(grpfvisi = PwwdWw(ww)->grpfvisi);
	Assert((int) vfli.grpfvisi == (int) grpfvisi);

	/* fake x coordinates in edl for erase and clip rectangle */
	/* then transform into w's.  Intersect with rcwDisp */
	ptOrigin = PtOrigin(hpldr, idr);
	hwwd = vhwwdOrigin;
	pwwd = *hwwd;

	dxpToXw = ptOrigin.xl + pwwd->rcePage.xeLeft + pwwd->xwMin;
	rcwErase.xwLeft = (drcp.xp = -pdr->dxpOutLeft) + dxpToXw;
	if (pwwd->fPageView)
		rcwErase.xwRight = rcwErase.xwLeft +
				(drcp.dxp = pdr->dxl + pdr->dxpOutRight + pdr->dxpOutLeft);
	else
		{
		rcwErase.xpRight = pwwd->xwMac;
		drcp.dxp = rcwErase.xwRight - rcwErase.xwLeft;
		}

	dypToYw = ptOrigin.yl + pwwd->rcePage.yeTop + pwwd->ywMin;
	rcwErase.ywTop = (drcp.yp = edl.drcp.yp) + dypToYw;
	rcwErase.ywBottom = rcwErase.ywTop + (drcp.dyp = edl.drcp.dyp);

/* if displaying to screen cache bitmap:
		use scratchpad memory DC
		do not validate dl if bitmap cannot fit the line 
*/
	hdc = pwwd->hdc;
	ClipRectFromDr(hwwd, hpldr, idr, &drcp, &rcw);
	rcwClip = rcw;
	Assert( hdc != NULL );
	/* Check for quick return if the line is not visible at all. */
	if (rcw.ywTop >= rcw.ywBottom || rcw.xwLeft >= rcw.xwRight)
		{
		MeltHp();
		goto LRet;
		}
	/* This tells if there is a dr overflow. */
	fFat = pdr->doc > 0
		&& (fInTable ? pdr->fCantGrow : vfli.fPageView)
		&& ((ypLine == pdr->dyl && pdr->idrFlow == idrNil
				&& (fInTable ? !vfli.fSplatBreak || vfli.chBreak != chTable
					: vfli.cpMac < pdr->cpLim))
			|| ypLine > pdr->dyl);

	Scribble(ispDisplayFli,'D');

LBeginLine:

	MeltHp();
	if (!vfli.fPageView)
		{
		if (vfli.fSplatBreak && !fInTable)
			{
			int ywMid;
			int ipat;

			xw = rcw.xwLeft;
			edl.dxp = rcw.xwRight - dxpToXw - edl.xpLeft;
LSplat:
/* Must erase the line and sel bar background, since we won't be calling
	DisplayFliCore. */
			Assert(vfli.doc != docNil);
			PatBltRc(hdc, &rcw, vsci.ropErase);

LSplat2:
			ipat = vfli.fSplatColumn ? ipatHorzLtGray : ipatHorzGray;
			CacheSect(vfli.doc, vfli.cpMin);
			/* Draw the break/section splat. */
			ywMid = rcwErase.ywBottom - vfli.dypBase;
			if (ywMid < rcw.ywBottom && ywMid >= rcw.ywTop)
				DrawPatternLine(hdc, xw, ywMid,	rcw.xwRight - xw, ipat, pltHorz);
			PutPlc(hplcedl, dl, &edl);
			if (vfli.cpMac == caSect.cpLim)
				{
				ywMid = rcwErase.ywBottom - vfli.dypBase -
						min((edl.dyp - vfli.dypBase), 
						(vsci.dypBorder << 1));
				Assert(!vfli.fSplatColumn);
				if (ywMid < rcw.ywBottom && ywMid >= rcw.ywTop)
					DrawPatternLine(hdc, xw, ywMid, rcw.xwRight - xw, ipat,pltHorz );
				}
			/* Paint the outline bullets because LHighlight
				is located after we call AddVisiSpaces(). */
			if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
				{
				AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip);
				}
			goto LHighlight;
			}
		else
/* check for soft page breaks */
			{
			struct DOD *pdod;

			pdod = PdodDoc(vfli.doc);
			if (!pdod->fShort && pdod->hplcpgd && !(*hwwd)->fOutline)
				{
				if (!FInCa(vfli.doc, vfli.cpMac - 1, &caPage))
					{
					CP cpFli;
					int wwFli, docFli, dxaFli;
					wwFli = vfli.ww;
					docFli = vfli.doc;
					cpFli = vfli.cpMin;
					dxaFli = vfli.dxa;
					CachePage(vfli.doc, vfli.cpMac - 1);
					if (vfFliMunged && wwFli != wwNil && docFli != docNil)
						FormatLineDxa(wwFli, docFli, cpFli, dxaFli);
					}
/* we are now guaranteed that cpMac-1 is within the cached page */
				if (!fInTable && vfli.cpMac == caPage.cpLim
						&& vfli.cpMac < CpMacDocEdit(vfli.doc))
					{
/* last line of page. Show page break */
/* (BL) yes this flag is ugly; it is for speed of modifying the hand
	native code */
					fSoftPageBreak = fTrue;
					}
				}
			}
		}

	fFrameLines = ((vfli.fPageView && FDrawPageDrsWw(ww) && !fInTable) ||
			 (fInTable && FDrawTableDrsWw(ww)));

	/* don't blow away the frame lines if they're just going to get
			drawn back again; otherwise they'll flicker */
	if (fFrameLines)
		{
		if (dl == 0 && pdr->yl == 0)
			rcw.ywTop = min(rcw.ywBottom, rcw.ywTop + dyFrameLine);

		if (rcwErase.ywBottom >= YwFromYl(hpldr, (*hpldr)->dyl) &&
				pdr->fBottomTableFrame && !fFat)
			rcw.ywBottom = max(rcw.ywTop, rcw.ywBottom - dyFrameLine);
		}

	DisplayFliCore(ww, rcw,
			dxpToXw + /* aesthetic fudge */ (fWin ? 0 : vsci.dxpBorder),
			rcwErase.ywBottom);

     /* edl may have changed in drawing picture in displayflicore. Update. */ 
	GetPlc(hplcedl, dl, &edl);

	if (fSoftPageBreak)
		{
		edl.dxp = rcw.xwRight - dxpToXw - edl.xpLeft;
		DrawPatternLine(hdc, rcw.xwLeft,
				rcwErase.ywBottom - vsci.dypBorder,
				rcw.xwRight - rcw.xwLeft,
				ipatHorzLtGray, pltHorz);
		PutPlc(hplcedl, dl, &edl);
		}

	if (!vfli.fPrint && vlm != lmPreview)
		{
		if (fFrameLines)
			FrameDrLine(ww, hpldr, idr, rcwErase.ywTop, rcwErase.ywBottom,
					fFat, fInTable/*fForceLine*/, fInTable/*fInnerDr*/);
		}

	if (vfli.fOutline)
		{
		struct PT	ptT;

		/* Know: what x position is after trailing space, so ellipses
			can be drawn ... */
		ptT.xp = vfli.xpRight + dxpToXw;
		ptT.yp = ypLine;
		edl.dxp += DxpOutlineSplats(hdc, dxpToXw, ptT, &rcwErase);
		PutPlc(hplcedl, dl, &edl);
		}

/* after the line has been painted, retrofit visible spaces, draw selection
and return bitmaps to their original state */

	if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
		{
		AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip);
		}

/* splats in page view */
	if (vfli.fPageView && vfli.fSplatBreak && !fInTable && vfli.grpfvisi.fvisiShowAll)
		{
		struct RC rcwT;

		xw = dxpToXw + vfli.xpRight - vfli.rgdxp[vfli.ichMac - 1];
		rcw.xwLeft = xw;
		DrclToRcw(hpldr, &pdr->drcl, &rcwT);
		rcw.xwRight = rcwT.xwRight;
		if (FDrawPageDrsWw(ww))
			{
/* adjust if FDrawPageDrsWw because PatBltRc would erase the boundary line */
			rcw.xwLeft += vsci.dxpBorder;
			rcw.xwRight -= vsci.dxpBorder;
			if (rcw.ywTop <= rcwT.ywTop)
				rcw.ywTop += vsci.dypBorder;
			}
		if (vfli.ichMac > 1)
			goto LSplat2; /* not the only thing on the line, background already erased */
		goto LSplat;
		}



/* show highlight */
LHighlight:
	if (PwwdWw(ww)->xwSelBar > 0)
		{
		Assert(!PwwdWw(ww)->fPageView);
		if (!fInTable)
			DrawStyNameFromWwDl(ww, hpldr, idr, dl);
/* for table, draw the style name in FUpdateTable for each table row */
		}

	MarkSel(ww, hpldr, idr, &selCur, &edl, vfli.cpMin, vfli.cpMac );
	MarkSel(ww, hpldr, idr, &selDotted, &edl, vfli.cpMin, vfli.cpMac );

/* iff fRMark text was on line, draw revision bar in selection bar */
	if (fRMark && !fInTable && ((irmBar = PdodMother(vfli.doc)->dop.irmBar) != irmBarNone))
		{
		int xwRevBar;

		if (!(*hwwd)->fPageView)
			xwRevBar = (*hwwd)->xwSelBar + dxwSelBarSci / 2;
		else
			{
			DrclToRcw(hpldr, &pdr->drcl, &rcw);
			switch (irmBar)
				{
			default:
				Assert(fFalse);
				/* fall through */
			case irmBarOutside:
				if (vfmtss.pgn & 1 && FFacingPages(DocMother(vfli.doc)))
					goto LRight;
				/* fall through */
			case irmBarLeft:
				xwRevBar = rcw.xwLeft - (dxwSelBarSci / 2);
				break;
			case irmBarRight:
LRight:
				xwRevBar = rcw.xwRight + (dxwSelBarSci / 2);
				break;
				}
			}
		DrawRevBar( (*hwwd)->hdc, xwRevBar,
				ypLine - edl.dyp + dypToYw, edl.dyp, &rcwClip);
		}

LRet:
	pdr->xwLimScroll = max( pdr->xwLimScroll, 
			edl.xpLeft + edl.dxp + dxpToXw);
LRet0:
	Scribble(ispDisplayFli,' ');
	FreePdrf(&drfFetch);
	vrf.fInDisplayFli = !vrf.fInDisplayFli;
}


#endif /* DEBUG */


#ifdef NOASM
/*
*  DrawRevBar draws a revision bar at (xw,yw).  The height of the bar is
*  determined by the size of vfli.dypLine or vfli.dypFont. If not NULL,
*  drawing is clipped to *prcwClip.
*/

/* %%Function:DrawRevBar %%Owner:bryanl */
DrawRevBar(hdc, xw, yw, dyw, prcwClip)
HDC hdc;
int xw, yw, dyw;
struct RC *prcwClip;
{
	HBRUSH hbrOld;

	if (prcwClip != NULL)
		{
		if (xw < prcwClip->xwLeft || xw + vfti.dxpBorder > prcwClip->xwRight)
			return;
		if (yw < prcwClip->ywTop)
			{
			dyw -= yw - prcwClip->ywTop;
			yw = prcwClip->ywTop;
			}
		if ((dyw = min(dyw, prcwClip->ywBottom - yw)) <= 0)
			return;
		Assert(yw >= prcwClip->ywTop && yw + dyw <= prcwClip->ywBottom && dyw > 0);
		}
	if (vlm = lmPreview)
		{
		DrawPrvwLine(hdc, xw, yw, vfti.dxpBorder, dyw, colAuto);
		return;
		}
	hbrOld = SelectObject(hdc, vfli.fPrint ? vpri.hbrText:vsci.hbrText);
	PatBlt(hdc, xw, yw, vfti.dxpBorder, dyw, PATCOPY);
	if (hbrOld != NULL)
		SelectObject(hdc, hbrOld);
}


#endif /* NOASM */


#ifdef DEBUG
/* D I S P L A Y  F L I  C O R E */
/* paint line described by vfli onto DC hdc, at coordinates described
	by *pptPen. There is no need to paint beyond rcClip.
*/
/* %%Function:C_DisplayFliCore %%Owner:bryanl */
HANDNATIVE C_DisplayFliCore(ww, rcwClip, dxpToXw, ywLine)
int		ww;
struct RC	rcwClip;
int		dxpToXw, ywLine;
{
	int		dypPen;
	int		ich;
	int		ichNext;
	CP		dcpVanish;
	struct CHR	*pchr;
	int		hpsPosPrev;
	int		tlc;
	HDC		hdc = PwwdWw(ww)->hdc;
	int		eto;
	BOOL		fErased = fFalse;
	int		fPrvwPrint = vfli.fPrint || vfPrvwDisp;
	int		bchrCur;
	int		bchpCur;
	struct ULS	uls;
	struct PT	ptPen;
	struct RC	rcOpaque, rcErase, rcAdjusted;
	BOOL	    	fRcAdjusted;
	BOOL		fNoSkip = fFalse;

	int dxp;
	int ilevel;
	struct CHP *pchp;
	struct PT  ptPenT;  /* A copy of ptPen to be used by DrawBrcl */

	ptPenT.xw = ptPen.xw = vfli.xpLeft + dxpToXw;
	ptPenT.yw = ptPen.yw = ywLine - vfli.dypBase;

	Assert( ww != wwNil );
	Assert( hdc !=NULL );
	uls.ww = ww;
	uls.hdc = hdc;
	uls.grpfUL = 0;
	ich = 0;
	pchr = &(**vhgrpchr)[0];
	dcpVanish = cp0;
	hpsPosPrev = 0;
	dypPen = 0;

	blt(&rcwClip,&rcOpaque,cwRC);
	blt(&rcwClip,&rcErase,cwRC);

	eto = ETO_CLIPPED;
	if (!fPrvwPrint)
		eto |= ETO_OPAQUE;

/* main loop, goes around for every chr */
/* pen position must be at the baseline in front of the next character. */

	for (;;)
		{
		int chrm;
/* if is used instead of switch for better space/speed */
		if ((chrm = pchr->chrm) == chrmTab)
			{
			char ch;

/* special chrmTab for visible characters: special draw for pub chars, ignore all others. */
			bchrCur = (char *) pchr - (char *)*vhgrpchr;
			bchpCur = (char *) pchp - (char *)*vhgrpchr;
			if ((ch = ((struct CHRT *)pchr)->ch) == 0)
				{
				dxp = vfli.rgdxp[ich];

				if ((tlc = ((struct CHRT *)pchr)->tlc) || pchp->kul)
					{
/* draw tab leader or underlining.  Blank first. */
					int xpNew; 
					struct RC rcT;
					int fOver = fFalse;

					xpNew = ptPen.xp + dxp;
					if (!fErased && !fPrvwPrint)
						{
						blt(&rcErase, &rcT, cwRC);
						rcT.xpRight = xpNew;
						PatBltRc(hdc, &rcT, vsci.ropErase);
						rcOpaque.xpLeft = rcErase.xpLeft = xpNew;
						}
					if (!tlc)
						{
/* 4th param means round up.  Do not round up iff next char is not ul'd. */
						struct CHR *pchrT =	((struct CHRT *) pchr) + 1;
						fOver = !(pchrT->chrm == chrmChp &&
								pchrT->ich == ich + 1 &&
								pchrT->chp.kul != kulSingle);
						}
/* note: underlined tabs must be drawn as underlined space "leaders" */
					DrawTlc(hdc, ptPen, tlc, dxp, pchp, fOver, &rcwClip);
					}
				ptPen.xp += dxp;
				ich++;
				}
			else  if ((uns)(ch - chPubMin) < (chPubMax-chPubMin))
				{
				DrawChPubs( hdc, &ptPen, ch, &rcOpaque,
						&rcErase, fErased, eto);
				ich++;
				}
			(char *) pchr = (char *)*vhgrpchr + bchrCur;
			(char *) pchp = (char *)*vhgrpchr + bchpCur;
			((struct CHRT *)pchr)++;
			}
		else  if (chrm == chrmChp)
			{
			bchrCur = (char *) pchr - (char *)*vhgrpchr;
			pchp = &pchr->chp;
/* these show if any underline at level 1 or level 2 are set in current looks */
			if (uls.grpfUL)
				{
				EndULPast(&uls,ptPen.xp,ich,&rcwClip);
				/* reset pointers into heap */
				pchr = (char *)*vhgrpchr + bchrCur;
				pchp = &pchr->chp;
				}

			if (ich < vfli.ichSpace3)
				uls.kul = pchr->fcid.kul;

			if (hpsPosPrev != pchp->hpsPos)
				{
				int dypOffset, dypPenMotion;
				hpsPosPrev = pchp->hpsPos;
				/* 2 * 72 converts half points to inches */
				dypOffset = NMultDiv(
						(hpsPosPrev >= 128) ?
						(hpsPosPrev - 256) : hpsPosPrev,
						vfti.dypInch, 2 * 72);

				dypPenMotion = dypOffset - dypPen;
				ptPen.yp -= dypPenMotion;
				dypPen += dypPenMotion;
				}
			LoadFcidFull( pchr->fcid );
/* treat begin underline */
			/* reset pointers into heap */
			pchr = (char *)*vhgrpchr + bchrCur;
			pchp = &pchr->chp;

			if (uls.grpfUL)
				{
				uls.pt = ptPen;
				uls.dypPen = dypPen;
				}
			ForeColor(hdc, pchp->ico);

			/* advance pchr to next CHR */
			pchr = (char *)pchr + sizeof(struct CHR);
			}
		else  if (chrm == chrmVanish)
			{
/* real cp can be calculated as vfli.cpMin + ich + dcpVanish */
			dcpVanish += ((struct CHRV *)pchr)->dcp;
			(char *)pchr += cbCHRV;
			}
		else  if (chrm == chrmDisplayField)
			{
			int xpNew = ptPen.xp + ((struct CHRDF *)pchr)->dxp;
			struct PT ptT;
			struct RC rcT;

			bchrCur = (char *)pchr - (char *)*vhgrpchr;
			bchpCur = (char *)pchp - (char *)*vhgrpchr;
			if (uls.grpfUL)
				{
				uls.xwLim = ptPen.xp;
				EndUL(&uls, &rcwClip);
				pchr = (char *)*vhgrpchr + bchrCur;
				pchp = (char *)*vhgrpchr + bchpCur;
				}
			ptT.xp = ptPen.xp;
			ptT.yp = ptPen.yp + vfli.dypBase - 
					vfli.dypLine + dypPen;
			/*  chDisplayField is a placeholder
				character in rgch for the CHRDF.  Skip over it.
			*/
			Assert (vfli.rgch [ich] == chDisplayField);
			Assert (vfli.rgdxp [ich] == ((struct CHRDF *)pchr)->dxp);
			Assert (ich == pchr->ich);
			ich++;

#ifdef SHOWFLD
			CommSzLong ("Displaying CHRDF: ", 
					vfli.cpMin + dcpVanish + pchr->ich);
#endif /* SHOWFLD */

			blt( &rcOpaque, &rcT, cwRC );
			rcT.xpRight = xpNew;

			if (!fErased && !fPrvwPrint)
				{
				PatBltRc( hdc, &rcT, vsci.ropErase );
				rcErase.xpLeft = rcOpaque.xpLeft = xpNew;
				}

			DisplayField (vfli.doc,
					vfli.cpMin + dcpVanish + pchr->ich,
					bchrCur, ((struct CHRDF *)pchr)->flt,
					hdc, ptT, ptPen.yp+dypPen, 
					ptPen.yp, (ich < vfli.ichSpace3 ? &uls : NULL), &rcT);

			pchr = (char *)*vhgrpchr + bchrCur;
			pchp = (char *)*vhgrpchr + bchpCur;
			dcpVanish += ((struct CHRDF *)pchr)->dcp -1;
			ptPen.xp = xpNew;
			(char *) pchr += cbCHRDF;
			}
		else  if (chrm == chrmFormula)
			{
			int	xpNew, ypNew;

			if (uls.grpfUL)
				{
				uls.xwLim = ptPen.xp;
				bchrCur = (char *)pchr - (char *)*vhgrpchr;
				bchpCur = (char *)pchp - (char *)*vhgrpchr;
				EndUL(&uls, &rcwClip);
				pchr = (char *)*vhgrpchr + bchrCur;
				pchp = (char *)*vhgrpchr + bchpCur;
				}
			xpNew = ptPen.xp + ((struct CHRF *) pchr)->dxp;
			ypNew = ptPen.yp + ((struct CHRF *) pchr)->dyp;
			dxp = ((struct CHRF *) pchr)->dxp;
			if (((struct CHRF *) pchr)->fLine)
				{
				ilevel = SaveDC(hdc);

				if (vfPrvwDisp)
					{
					InflateRect((LPRECT)&rcOpaque,10,10);
					IntersectClipRect(hdc, 
							rcOpaque.xpLeft,
							rcOpaque.ypTop, 
							rcOpaque.xpRight,
							rcOpaque.ypBottom);
					InflateRect((LPRECT)&rcOpaque,-10,-10);
					DrawPrvwLine(hdc, ptPen.xp, 
							ptPen.yp, xpNew - ptPen.xp, 
							ypNew - ptPen.yp, colText);
					}
				else
					{
					IntersectClipRect(hdc, rcOpaque.xpLeft,
							rcOpaque.ypTop, rcOpaque.xpRight,
							rcOpaque.ypBottom);

					Assert(!(vfli.fPrint & 0xFE));
					DrawFormulaLine(hdc, xpNew, ypNew,
							ptPen.xp, ptPen.yp,
							vfli.fPrint /* fPrint */
							+ 8 /* fCreate */
							+ 4 /* fMove */
					+ 2 /* fDestroy */);
					}

				RestoreDC (hdc, ilevel);
				}
			ptPen.xp = xpNew;
			ptPen.yp = ypNew;
			fNoSkip = vfli.fPrint;
			(char *) pchr += chrmFormula;
			}
		else  if (chrm == chrmFormatGroup)
			{
			fErased = fTrue;
			rcOpaque.xpRight = rcwClip.xpRight;
			if (eto & ETO_OPAQUE)
				/*  erase the rest of the line */
				{
				PatBltRc (hdc, &rcErase, vsci.ropErase);
				SetBkMode (hdc, TRANSPARENT);
				eto &= ~ETO_OPAQUE;
				}
			/* note that rcOpaque and rcErase will no
				longer change (rcErase not used) */
			(char *) pchr += chrmFormatGroup;
			}
		else  /*if (chrm == chrmEnd)*/
			
			{
			Assert(chrm == chrmEnd);
			goto LEnd;
			}

LDacNop:
/* calculate the next action and the ich it should take place at or perform
	the action immediately */
/* see comments at the beginning of the module */
		ichNext = pchr->ich;
		if (ichNext > ich)
			{
			int *pdxp;
			int cch;
			int cchChop;
			int yp = ptPen.yp - vfti.dypAscent;

			/* if we got here via LDacNop, pchp is unitialized */
/* now show the text & update the pen position */
/* min and max avoid printing chars beyond ichSpace3, like underlined spaces
	however, for display, may need to display visi spaces, para marks, etc */

			if (fPrvwPrint && ichNext > vfli.ichSpace3)
				{
				cch = vfli.ichSpace3;
				cchChop = ichNext - vfli.ichSpace3;
				}
			else
				{
				cch = ichNext;
				cchChop = 0;
				}
			cch = max( cch - ich, 0 );
			pdxp = &vfli.rgdxp [ich];

/* Optimizations: Skip stuff that is clipped off anyway */

			/* Yes, but italic characters are a real bummer! */
			while ((ptPen.xp + *pdxp + vfti.dxpOverhang <= rcwClip.xpLeft) && cch)
				{
				cch--;
				ptPen.xp += *(pdxp++);
				ich++;
				}
			if (!cch)
				continue;
			if (!fNoSkip && ptPen.xp >= rcwClip.xpRight)
				/* Already past clip region; save some work */
				goto LEnd;

			/* Deal with special chars */
			if (pchp->fSpec)
				{
				int xpNew = ptPen.xp + *pdxp;
				struct RC rcT;
				int nSaveDC = 0;
				CHAR chSpec = vfli.rgch [ich];

				Assert(vfli.rgch [ich] != chDisplayField);

				if (chSpec == chPicture)
					if (uls.grpfUL)
						{
						uls.xwLim = ptPen.xp;
						bchrCur = (char *) pchr - (char *)*vhgrpchr;
						bchpCur = (char *) pchp - (char *)*vhgrpchr;
						EndUL(&uls, &rcwClip);
						(char *) pchr = (char *)*vhgrpchr + bchrCur;
						(char *) pchp = (char *)*vhgrpchr + bchpCur;
						}

				if (!fErased && !fPrvwPrint)
					{ /* because ShowSpec is using TextOut,
							we have to erase the background */
					blt( &rcErase, &rcT, cwRC );
					rcT.xpRight = min(xpNew, rcwClip.xpRight);
					PatBltRc( hdc, &rcT, vsci.ropErase );
					rcErase.xpLeft = rcOpaque.xpLeft = xpNew;
					}
				vfmtss.cpRef = vfli.cpMin + dcpVanish + ich;
				if (ww != wwNil)
					{
					nSaveDC = SaveDC(hdc);
					if (vfPrvwDisp)
						InflateRect((LPRECT) &rcwClip, 10,10);
					IntersectClipRect(hdc, rcwClip.xpLeft,
							rcwClip.ypTop, rcwClip.xpRight,
							rcwClip.ypBottom);
					if (vfPrvwDisp)
						InflateRect((LPRECT) &rcwClip, -10,-10);
					}
				bchrCur = (char *) pchr - (char *)*vhgrpchr;
				bchpCur = (char *) pchp - (char *)*vhgrpchr;
				ShowSpec(ww, ich, &ptPen, yp, pchp, &uls);
				(char *) pchr = (char *)*vhgrpchr + bchrCur;
				(char *) pchp = (char *)*vhgrpchr + bchpCur;

				if (vfli.grpfvisi.fvisiShowAll && 
						(chSpec == chFootnote 
						|| chSpec == chTFtn || chSpec == chTFtnCont))
					{
					struct RC rcBox;
					rcBox.xpLeft = ptPen.xp;
					rcBox.xpRight = xpNew - 1;
					rcBox.ypTop = yp + 1;
					rcBox.ypBottom = ptPenT.yp;
					/*yp = rcBox.ypTop - vfti.dypAscent */
					DrawDottedLineBox(hdc, rcBox);
					}
				if (nSaveDC != 0)
					{
					RestoreDC(hdc, nSaveDC);
					nSaveDC = 0;
					}
				ich++;
				ptPen.xp = xpNew;
				continue;
				}

			if (cch > 0)
				{
				int xp = ptPen.xp;
				int cchT = cch + cchChop;

				while (cchT--)
					ptPen.xp += *pdxp++;
#ifdef DFONT
				if (vfDbgRTFlag)
					{
					HFONT hfontT;
					SelectObject (hdc, (hfontT =
							SelectObject (hdc, GetStockObject(SYSTEM_FONT))));
					CommSzNum ("Font = ", hfontT);
					}
#endif /* DFONT */

/* following test prevents inefficient extra call to put out trailing
	space at end of line */
				if (ich < vfli.ichMac-1 || vfli.rgch [ich] != chSpace)
					{
					int far *lpdxp;
/* use array-of-widths if:
		(1) user-defined character spacing is in effect (pchp->qpsSpace) 
		OR (2) justification is on
		OR (3) visi spaces and space width != visi space char width
		OR (4) we're displaying for preview */

#ifdef BRYANL
					if (vfli.fAdjustForVisi)
						CommSz(SzShared("adjust for visi...\r\n"));
#endif
					if (pchp->qpsSpace 
							|| vfli.ichSpace2 != ichNil 
							|| vfli.fAdjustForVisi
							|| vfPrvwDisp)
						lpdxp = (int far *)&vfli.rgdxp [ich];
					else
						lpdxp = (int far *)(DWORD) NULL;

					if (!fErased && !fPrvwPrint)
						{
/* blank to the end of rcwClip if this is the last run of visible chars, for
	efficiency, to avoid an extra PatBlt */
						rcErase.xpLeft = 
								rcOpaque.xpRight = 
								(ptPen.xp >= vfli.xpRight ||
								ptPen.xp >= rcwClip.xpRight) ? 
								rcwClip.xpRight : ptPen.xp;
						}

					/* if there are any borders, don't opaque them,
						to avoid flashing; instead we need to wipe
						out any space outside the borders that's
						in the EDL */
					fRcAdjusted = fFalse;
					if (!fPrvwPrint &&
							(vfli.fTop || vfli.fLeft || vfli.fBottom || vfli.fRight))
						{
						fRcAdjusted = fTrue;
						AdjustOpaqueRect(&rcOpaque, &rcAdjusted, hdc, dxpToXw, ywLine);
						}
#ifdef BRYANL
					if (vfli.fPrint)
						{
						TEXTMETRIC tm;
						int rgw [5];

						CommSzNum( SzShared( "Text out at xp: "), xp );
						CommSzRgNum( SzShared( " TextOut, rgdxp is: "), &vfli.rgdxp [ich], cch );
						rgw [0] = eto;
						blt(fRcAdjusted ? (LPRECT)&rcAdjusted :	(LPRECT)&rcOpaque,  	
								&rgw [1], cwRC );
						CommSzRgNum( SzShared( " eto, rc is: "), rgw, 5 );
							{
							char szFace[256];
							int rgw [6];

							GetTextFace(hdc, LF_FACESIZE, (LPSTR)szFace);
							GetTextMetrics( hdc, (LPTEXTMETRIC) &tm );
							CommSzSz(SzShared(" Actual face name: "), szFace);
							rgw [0] = tm.tmHeight - tm.tmInternalLeading;
							rgw [1] = tm.tmAveCharWidth;
							rgw [2] = tm.tmInternalLeading + tm.tmExternalLeading;
							rgw [3] = tm.tmAscent;
							rgw [4] = tm.tmDescent;
							rgw [5] = tm.tmOverhang;
							CommSzRgNum( SzShared(" Actual ht, wid, leading, asc, desc, overhang: "),
									rgw, 6 );

							CommFontAttr( SzShared( " Actual Attributes: "),
									tm.tmItalic, tm.tmStruckOut, tm.tmUnderlined, tm.tmWeight > FW_NORMAL);

							rgw [0] = tm.tmPitchAndFamily >> 4;
							rgw [1] = tm.tmPitchAndFamily & 1;
							rgw [2] = tm.tmCharSet;
							CommSzRgNum(SzShared(" Actual font family, pitch, charset: "),
									rgw, 3 );
							}
						}
#endif

	/* FUTURE - The only reason protecting pchr and pchp from heap
		movement here is that the "Cannot print" dialog may be brought
		up under ExtTextOut.  If that dialog becomes system modal
		and does not move the heap, we can remove this junk.
		Brad Verheiden (4-13-89).
	*/
					bchrCur = (char *) pchr - (char *)*vhgrpchr;
					bchpCur = (char *) pchp - (char *)*vhgrpchr;
                      /* protection around the ExtTextOut calls */
                    vrf.fInExternalCall = fTrue;

				/* Now we have underlining fun for printing */
					if (vfli.fPrint && lpdxp &&
							(vfti.fcid.kul || vfti.fcid.fStrike))
						{
						PrintUlLine(hdc, xp, yp,
								eto, 			/* action bits */
						fRcAdjusted ? (LPRECT) &rcAdjusted :
								(LPRECT) &rcOpaque,  	/* opaque rect */
								lpdxp,
								cch);
						}

	 				if (vwWinVersion < 0x300 && !vfli.fPrint && lpdxp && pchp->fItalic)
	 					{
	 					struct FFN *pffn;
	 
	 					pffn = PstFromSttb(vhsttbFont, vfti.fcid.ibstFont);
	 					if (pffn->fRaster)
	 						goto LRealExtTextOut;
	 					ExtTextOutKludge(hdc, xp, yp,
	 						eto, 			/* action bits */
	 						fRcAdjusted ? (LPRECT)&rcAdjusted :
	 						(LPRECT)&rcOpaque,  	/* opaque rect */
	 						(LPCH)&vfli.rgch [ich],	/* string */
	 						cch,			/* length */
	 						lpdxp );		/* lpdx */
						}
	 				else
						{
LRealExtTextOut:
	 					ExtTextOut(hdc, xp, yp,
	 						eto, 			/* action bits */
	 						fRcAdjusted ? (LPRECT)&rcAdjusted :
	 						(LPRECT)&rcOpaque,  	/* opaque rect */
	 						(LPCH)&vfli.rgch [ich],	/* string */
	 						cch,			/* length */
	 						lpdxp );		/* lpdx */
	 					}
                    vrf.fInExternalCall = fFalse;
					(char *) pchr = (char *)*vhgrpchr + bchrCur;
					(char *) pchp = (char *)*vhgrpchr + bchpCur;

					if (!fErased && !fPrvwPrint)
						rcOpaque.xpLeft = ptPen.xp;
					}

				}

			ich = ichNext;
			}	/* end if (ichNext > ich) */
		}	/* end for ( ;; ) */
LEnd:
	if (uls.grpfUL)
		EndULPast(&uls, ptPen.xp, ich, &rcwClip);
/* erase portion of the line not erased within ExtTextOut blanking box */
	if (!fPrvwPrint)
		{
		SetBkMode( hdc, OPAQUE );   /* restore output mode */
		if (!fErased && rcErase.xpLeft < rcErase.xpRight)
			PatBltRc( hdc, &rcErase, vsci.ropErase );
		}

	ForeColor(hdc, icoAuto);

	/* return if no para border OR 
	printing and not the pass for drawing borders... 
	*/
	if (vfli.grpfBrc == 0)
		return;
	if (vfli.fPrint)
		{
		if (vpri.fDPR)	/* can only use DRAWPATTERNRECT in fText pass */
			{
			if (!vpri.fText)
				return;
			}
		else
			{
			if (!vpri.fGraphics)
				return;
			}
		}

/* handle drawing of borders */
	if (
			/* Borders */
	(vfli.fTop || vfli.fLeft || vfli.fBottom || vfli.fRight)
			&&
			(ilevel = SaveDC( hdc )))
		{
		if (vfPrvwDisp)
			InflateRect((LPRECT) &rcwClip, 10,10);
		IntersectClipRect( hdc, rcwClip.xpLeft, rcwClip.ypTop,
				rcwClip.xpRight, rcwClip.ypBottom );
		DrawBorders(dxpToXw, ywLine);
		RestoreDC( hdc, ilevel );
		}
	return;
}


#endif /* DEBUG */


#ifdef NOASM
/* M A R K  S E L */
/* Mark the selection (i.e. do appropriate inversion) psel in dc hdc
	for line pedl whose location is given by (xpLine, ypLine).  */
/* We do not mark the selection if its hidden bit is on; if the selection */
/* is in a TLE, we do not mark it and we set the hidden bit to TRUE */

/* %%Function:MarkSel %%Owner:bryanl */
HANDNATIVE MarkSel(ww, hpldr, idr, psel, pedl, cpMinDl, cpMacDl)
int		ww;
struct PLDR	**hpldr;
int		idr;
struct EDL	*pedl;
struct SEL	*psel;
CP		cpMinDl, cpMacDl;
{
	struct DRF drfFetch;

	if (psel->ww == ww && !psel->fHidden && !psel->fNil &&
			psel->doc == PdrFetchAndFree(hpldr, idr, &drfFetch)->doc &&
			psel->cpLim >= cpMinDl && psel->cpFirst <= cpMacDl)
		{
		if (FMarkLine(psel, hpldr, idr, pedl,
				psel->cpFirst, psel->cpLim, cpMinDl))
			{
			psel->fOn = fTrue;
			}
		}
}


#endif /* NOASM */



#ifdef DEBUG
/* F  M A R K  L I N E */
/* Highlight the visible portion of the selection psel in the line pedl.
Returns:
if fIns:
		true    if insert line is drawn
		false   if nothing is drawn
if !fIns
		true    if selection is complete (cpLimSel < cpFirstLine)
		false   if selection is not yet completely drawn
Highlight is done effectively in xor mode.
FormatLine may be called to ensure vfli validity.

implicit: wwCur, psel->fInsEnd if fIns.
pa is the highlighting mode: invert or dotted.
xpOut, ypOut give starting coordinates of the display area showing the line
*/

/* %%Function:C_FMarkLine %%Owner:bryanl */
HANDNATIVE int C_FMarkLine(psel, hpldr, idr, pedl,
cpFirstSel, cpLimSel, cpFirstLine)
struct SEL	*psel;
struct PLDR	**hpldr;
int		idr;
struct EDL	*pedl;	/* read-only */
CP		cpFirstSel, cpLimSel, cpFirstLine;
{
	int xpFirst, xpLim, dxp, dyp, ypLine, xwFirst, xwLim, ywLine;
	int sk = psel->sk, ichT;
	struct DR *pdr;
	CP cpLimLine = cpFirstLine + pedl->dcp;
	struct WWD *pwwd;
	struct RC rc;
	struct DRC drcp;
	struct RC rcwClip;
	BOOL fTableSel;
	struct DRF drfFetch;

/* since FormatLine can move the heap, and we don't always call it... */
	Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);

/* check if sel entirely below or entirely above the line */
	if (cpLimSel < cpFirstLine) return (sk != skIns);
/* if the sel is a block selection, we want to continue for one line more
	since its cpLimSel is really the cpFirst of the last line of the
	selection. */
	if (cpLimSel == cpFirstLine && (!psel->fBlock || psel->fTable))
		if (sk != skIns || psel->fInsEnd) return fFalse;
	if (cpFirstSel > cpLimLine) return fFalse;
	if (cpFirstSel == cpLimLine)
/* return if fIns sel is at the end of line but fInsEnd is not set;
or if 0 chars left in a !fIns selection */
		if (sk != skIns || !psel->fInsEnd)
			return fFalse;

	dyp = pedl->dyp;
	ypLine = pedl->ypTop + dyp;

	pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);

	if (fTableSel = (pdr->fInTable && !(psel->fWithinCell || psel->fIns)))
		{
		if (psel->fColumn &&
				(idr < psel->itcFirst || idr >= psel->itcLim))
			return fFalse;

		drcp = pdr->drcl;
		drcp.yp = 0;
		drcp.xp = xpFirst = idr == 0 ? 0 : -pdr->dxpOutLeft;
		xpLim = pdr->dxl + pdr->dxpOutRight;
		drcp.dxp += (-xpFirst + pdr->dxpOutRight);
		}
	else
		{
		drcp = pedl->drcp;
		if (psel->fBlock)
			{
			drcp.xp = psel->xpFirst;
			drcp.dxp = psel->xpLim - drcp.xp;
			}
		}
	ClipRectFromDr(HwwdWw(psel->ww), hpldr, idr, &drcp, &rcwClip);

/* Pad rcwClip to allow the insertion point to the right of the character */
	if (psel->fIns && !psel->fBlock)
		rcwClip.xwRight += (vsci.dxpBorder << 1);
	psel->rcwClip = rcwClip;

/* check if column or row selection */
	if (!fTableSel)
		{
/* check if block selection */
		if (psel->fBlock)
			{
			xpFirst = psel->xpFirst;
			xpLim = psel->xpLim;
			}
		else
/* check if entire line is selected; for pics, we have to do a FormatLine anyway */
			{
			BOOL fNeedFirst = fFalse, fNeedLim = fFalse;
			cpFirstSel = CpMax(cpFirstLine, cpFirstSel);
			cpLimSel = CpMin(cpLimLine, cpLimSel);
/* following series of checks is simple in native code. */
			if (psel->fGraphics)
				{
				fNeedFirst = fTrue; 
				fNeedLim = fTrue;
				goto LFormat;
				}

			if (cpFirstSel == cpFirstLine)
				xpFirst = pedl->xpLeft;
			else  if (cpFirstSel == psel->cpFirst &&
					psel->xpFirst != xpNil && sk != skIns)
				xpFirst = psel->xpFirst;
			else  if (cpFirstSel == psel->cpLim &&
					psel->xpLim != xpNil && sk != skIns)
				xpFirst = psel->xpLim;
			else  
				fNeedFirst = fTrue;

			if (cpLimSel == cpLimLine)
				xpLim = pedl->xpLeft + pedl->dxp;
			else  if (cpLimSel == psel->cpFirst &&
					psel->xpFirst != xpNil && sk != skIns)
				xpLim = psel->xpFirst;
			else  if (cpLimSel == psel->cpLim &&
					psel->xpLim != xpNil && sk != skIns)
				xpLim = psel->xpLim;
			else  
				fNeedLim = fTrue;

			if (fNeedFirst || fNeedLim)
				{
				int xpLimT, xpFirstT;
/* ensure vfli is correct */
LFormat:
				FormatLineDr(psel->ww, cpFirstLine, pdr);

/* decode cpFirstSel => xpFirst */

				xpLimT = XpFromDcp(
						(cpFirstSel - cpFirstLine),
						(cpLimSel - cpFirstLine),
						&xpFirstT, &ichT);

#ifdef DFLDSELECT
					{
					uns rgw [5];
					rgw[0]=(uns)(cpFirstSel-cpFirstLine);
					rgw[1]=(uns)(cpLimSel-cpFirstLine);
					rgw[2]=(uns)xpFirstT;
					rgw[3]=(uns)xpLimT;
					rgw[4]=ichT;
					CommSzRgNum(
							SzShared("XpFromDcp (dcp1,dcp2,xpF,xpL,ich): "),
							rgw, 5);
					}
#endif /* DFLDSELECT */

				if (fNeedFirst) xpFirst = xpFirstT;
				if (fNeedLim) xpLim = xpLimT;
				}
/* return globals */
			if (cpFirstSel > cpFirstLine) vxpFirstMark = xpFirst;
			if (cpLimSel < cpLimLine) vxpLimMark = xpLim;
			}
		}

/* now we have xpFirst, xpLim in unscrolled line coordinates. Transform: */

/* native code note: call PtOrigin only once for these two lines. */
	ywLine = YwFromYp(hpldr, idr, ypLine);
	xwFirst = XwFromXp(hpldr, idr, xpFirst);
	xwLim = xwFirst - xpFirst + xpLim;

	if (sk != skIns)
		{
		HDC hdc;
		int xwFirstNoClip;

		pwwd = PwwdWw(psel->ww);
		hdc = pwwd->hdc;

		xwFirstNoClip = xwFirst;
		/* clip out the selbar */
		xwFirst = max(xwFirst, pwwd->xwMin);

		PrcSet(&rc, xwFirst, ywLine - dyp, xwLim, ywLine);
		if (!FSectRc(&rc, &psel->rcwClip, &rc))
			return fFalse;
		if (psel->pa != paDotted || sk == skBlock)
			{
/* invert solid highlighting */
			/* note: hilitepicsel1 can handle it if the sel is
				really an import field graphic selection */
			if (psel->fGraphics)
				{
				/* pic wants the original rect from formatline */
				rc.xwLeft = xwFirstNoClip;
				rc.ywBottom = ywLine;
				rc.ywTop = ywLine-dyp; /*necessary only to */
				/*make valid rect   */
				HilitePicSel1(psel, &rc, &psel->rcwClip);
				}
			else
				{
				if (rc.ywTop < pwwd->rcwDisp.ywTop)
					{
					rc.ywTop = pwwd->rcwDisp.ywTop;
					}
				PatBltRc( hdc, &rc, DSTINVERT );
				}
			}
		else
			{
			/* note: rc has been clipped by the clip rect */
			DrawPatternLine( hdc, rc.xpLeft, ywLine,
					rc.xpRight - rc.xpLeft, ipatHorzGray,
					pltHorz | pltInvert );
			}
		return fFalse;
		}
	else
		{
/* Set up cursor draw dimensions. */
		Assert( vfli.ww == psel->ww );
		psel->dyp = min(vfli.dypFont, vfli.dypLine - vfli.dypAfter);
		psel->yw = ywLine - vfli.dypAfter;
		psel->xw = xwFirst;
		psel->tickOld = 0;
		DrawInsertLine(psel);
		return fTrue;
		}
}


#endif /* DEBUG */



#ifdef DEBUG
/* P A T  B L T  R C */
/* %%Function:C_PatBltRc %%Owner:bryanl */
HANDNATIVE C_PatBltRc( hdc, prc, rop )
HDC hdc;
struct RC *prc;
long rop;
	{  /* Pat Blt for our RC type (test for Nil is necessary because it uses
		our own criteria, not Windows') */

	if (!FEmptyRc( prc ))
		PatBlt( hdc, prc->xpLeft, prc->ypTop, prc->xpRight - prc->xpLeft,
				prc->ypBottom - prc->ypTop, rop );
}


#endif /* DEBUG */


#ifdef DEBUG
/* A D D  V I S I  S P A C E S */
/* Put a centered dot in each space character, and show all tabs.
Graphport and clipping are all set up. Parameters are in vfli
and from locals in DisplayFli */
/* %%Function:C_AddVisiSpaces %%Owner:bryanl */
HANDNATIVE C_AddVisiSpaces(hdc, dxpToXw, ywLine, prcwClip)
HDC hdc;
int dxpToXw;
int ywLine;
struct RC *prcwClip;
{
	int ich, ichNext;
	int chrm, ch;
	struct CHR *pchr;
	int bchrCur;
	int *pdxp;
	int xwPos, ywPos;
	int xwLast;
	int hps, hpsLast = -1; /* illegal value */
	int dypDot, dypDotPrev;
	HBRUSH hbrOld;
	int dxp, dyp;
	GRPFVISI grpfvisi;

	dypDot = dypDotPrev = 0;
	grpfvisi = vfli.grpfvisi;
/* position to start of the line. c.f. DisplayFli */
	xwPos = vfli.xpLeft + dxpToXw;
	ywPos = ywLine - vfli.dypBase;

	if (vfli.omk != omkNil && vfli.fOutline)
		{
		int	xwT, ywT;
		OtlMarkPos(xwPos, ywLine - vfli.dypLine, vfli.dypLine,
				&xwT, &ywT);
		DrawOtlPat(vfli.ww, hdc, vfli.omk == omkPlus ? idcbOtlMarkPlus :
				(vfli.omk == omkMinus) ?
				idcbOtlMarkMinus : idcbOtlMarkBody,
				xwT, ywT);
		if (!vfli.grpfvisi.w) return; /* for speed */
		}

	pchr = &(**vhgrpchr)[0];
	bchrCur = 0;
	pdxp = &vfli.rgdxp[0];

	Assert(pchr->ich == 0);

	for (ich = 0;;)
		{
		ichNext = pchr->ich;
		while (ich < ichNext)
			{
/* unlike Mac: spaces are handled in FormatLine, non-breaking-spaces
	are handled like non-breaking-hyphen */
			if (vfli.rgch [ich] == chTab &&
					(grpfvisi.fvisiTabs || grpfvisi.fvisiShowAll) )
				DrawChVis( hdc, idcbChVisTab, xwPos, ich,
						ywPos, prcwClip );
			xwLast = xwPos;
			xwPos += *pdxp++;
			ich++;
			}

		if ((chrm = pchr->chrm) == chrmTab &&
				(ch = ((struct CHRT *)pchr)->ch) != 0)
			{
/* weird chrmTab for visi chars */
/* NOTE: chNonBreakSpace and chNonReqHyphen generate one of these but
	are currently handled by character substitution */
			if (ch == chNonBreakHyphen && 
					(grpfvisi.fvisiCondHyphens || grpfvisi.fvisiShowAll) &&
					(xwPos >= prcwClip->xwLeft && xwPos < prcwClip->xwRight))
				{
				int xwLim = min(xwPos + vfli.rgdxp [ich], prcwClip->xwRight);

				DrawPatternLine( hdc,
						xwPos, ywLine - vfli.dypAfter - 
						((vfli.dypLine - vfli.dypAfter - vfli.dypBefore)>>1),
						xwLim - xwPos, ipatHorzBlack, pltHorz );
				}
			xwPos += *pdxp++;
			ich++;
			}

		else  if (chrm == chrmChp)
			{
			LoadFcidFull(pchr->fcid);
			/* reset the pointer into the heap. */
			pchr = (char *)*vhgrpchr + bchrCur;

			if ((hps = pchr->chp.hps) != hpsLast)
/* set ywPos according to new font size */
				{
#define DypFromHps(hps) (NMultDiv(hps * (dyaPoint / 2), vfli.dysInch, dxaInch))
				dypDot = DypFromHps(hps)/4;
				ywPos = ywPos - dypDot + dypDotPrev;
				hpsLast = hps;
				dypDotPrev = dypDot;
				}
			}

		else  if (chrm == chrmFormula)
			{
			xwPos += ((struct CHRF *)pchr)->dxp;
			ywPos += ((struct CHRF *)pchr)->dyp;
			}

		else  if (chrm == chrmEnd)
			break;
		bchrCur += chrm;
		(char *)pchr += chrm;
		}

/* draw CRJ symbol if needed */
	if (vfli.chBreak == chCRJ &&
			(grpfvisi.fvisiParaMarks || grpfvisi.fvisiShowAll) )
		DrawChVis( hdc, idcbChVisCRJ, xwLast, vfli.ichMac-1, ywPos, prcwClip );
}


#endif /* DEBUG */


#ifdef DEBUG	    /* LN_DrawChVis in disp1n.asm used if not DEBUG */
/* D R A W  C H  V I S */
/* %%Function:DrawChVis %%Owner:bryanl */
DrawChVis( hdc, idcb, xp, ich, ywPos, prcwClip )
HDC hdc;
int idcb;
int xp;
int ich;
int ywPos;
struct RC *prcwClip;
{
	struct MDCD *pmdcd;
	int dxp, dyp;
	int xpSrc, ypSrc, dxpSrc, dypSrc;
	struct RC rcwDest, rcT;


	if ((pmdcd = PmdcdCacheIdrb( idrbChVis, hdc )) == NULL)
		return;

	if (idcb == idcbChVisTab)
		{
		/* center it. */
		if (dxpChVisEach > vfli.rgdxp[ich])
			{
			dxp = vfli.rgdxp[ich];
			}
		else
			{
			dxp = dxpChVisEach;
			xp += (vfli.rgdxp[ich] - dxpChVisEach) / 2;
			}
		}
	else  /* CRJ */
		
		{
		dxp = vfli.rgdxp[ich];
		}

	dyp = min(dypChVis,vfli.dypLine - vfli.dypBase - vfli.dypBefore);

/* clipping part */
	PrcSet(&rcT, xp, ywPos - dyp, dxp, dyp);
	DrcToRc(&rcT, &rcT);
	if (!FSectRc(&rcT, prcwClip, &rcwDest /*rcResult*/))
		return;

	dxpSrc = NMultDiv(dxpChVisEach, rcwDest.xwRight-rcwDest.xwLeft, dxp);
	dypSrc = NMultDiv(dypChVis, rcwDest.ywBottom-rcwDest.ywTop, dyp);
	xpSrc = idcb * dxpChVisEach;
	if (rcwDest.xpLeft != rcT.xpLeft)
		/* clipped on the left edge, show right edge of bitmap */
		xpSrc += dxpChVisEach - dxpSrc;

	StretchBlt( hdc, rcwDest.xwLeft, rcwDest.ywTop,
			rcwDest.xwRight - rcwDest.xwLeft, rcwDest.ywBottom - rcwDest.ywTop,
			pmdcd->hdc,                 /* hdcSrc */
			xpSrc, 0,  /* xpSrc,ypSrc */
	dxpSrc, dypSrc,
			vsci.dcibMemDC.ropBitBlt );
}


#endif /* DEBUG */


#ifdef NOASM
/* F  S C R O L L  O  K  */
/* DypScroll may not be called if this proc returns false meaning not
all of the window is visible either by its position on the screen or
because it is not on the top. */
/* %%Function:FScrollOK %%Owner:bryanl */
HANDNATIVE BOOL FScrollOK(ww)
int ww;
{
	extern int	vwwClipboard;
	extern HWND	vhwndApp;
	extern int	vfFocus;
	struct RC	rcwDisp, rcScreen, rcInter;
	struct WWD	*pwwd = PwwdWw(ww);

/* Check 1: is any of the window off the screen edge? */

	FreezeHp();
	Assert( pwwd->hdc != NULL );
	rcwDisp = pwwd->rcwDisp;
	if (GetClipBox(pwwd->hdc, (LPRECT)&rcScreen) != SIMPLEREGION
			|| !FSectRc( &rcwDisp, &rcScreen, &rcInter )
			|| FNeRgw(&rcInter, &rcwDisp, cwRC))
		{
		MeltHp();
		return fFalse;
		}

/* Check 2: is any window covering this ww? */
	MeltHp();
	if (ww != wwCur)
		{
		return fFalse;
		}
	else
		{
		HWND	hwnd;

		hwnd = GetWindow(vhwndApp, GW_HWNDFIRST);
		while (hwnd != NULL && !IsWindowVisible(hwnd))
			{
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
		return (vhwndApp == hwnd);
		}
}


#endif /* NOASM */


/* T O G G L E  S E L */
/* Flip selection highlighting on/off
psel provides pa, ww...
*/
/* %%Function:ToggleSel %%Owner:bryanl */
ToggleSel(psel, cpFirst, cpLim)
struct SEL *psel;
CP cpFirst, cpLim; /* bounds */
{

#ifdef MAC
	InvalidateDbcWw(psel->ww);  /* invalidate dialog bit-caches */
#endif

	vxpFirstMark = vxpLimMark = xpNil;
	ToggleSel1(psel, HwwdWw(psel->ww), cpFirst, cpLim);
}


/* T O G G L E  S E L 1 */
/* used to implement the recursive call for nested frames */
/* %%Function:ToggleSel1 %%Owner:bryanl */
ToggleSel1(psel, hpldr/*hwwd*/, cpFirst, cpLim)
struct SEL *psel;
struct PLDR **hpldr;
CP cpFirst, cpLim; /* bounds */
{
	struct PLCEDL **hplcedl;
	int idr, idrMac;
	struct DR *pdr;
	int dl, dlMac;
	CP cpT;
	struct RC rc;
	struct DRF drfFetch;

	idrMac = (*hpldr)->idrMac;
	for (idr = 0; idr < idrMac; FreePdrf(&drfFetch), idr++)
		{
		pdr = PdrFetch(hpldr, idr, &drfFetch);
		if (pdr->doc != psel->doc || pdr->hplcedl == hNil)
			continue;
		dlMac = IMacPlc(hplcedl = pdr->hplcedl);
		if (idrMac != 1)
			{
			if (pdr->cpFirst > cpLim ||
					CpPlc(hplcedl, dlMac) < cpFirst) continue;
			}
		if ((cpT = CpPlc(hplcedl, 0)) > cpFirst)
			dl = (cpT > cpLim) ? dlMac : 0;
		else if ((dl = IInPlcCheck(hplcedl, cpFirst)) == -1)
			dl = dlMac;
		if (dl != 0 && psel->fInsEnd && psel->fIns)
			dl--;
		for ( ; dl < dlMac; dl++)
			{
			struct EDL edl;

			if ((cpT = CpPlc(hplcedl, dl)) > cpLim)
				break;
			GetPlc(hplcedl, dl, &edl);
			if (edl.hpldr != hNil)
				ToggleSel1(psel, edl.hpldr, cpFirst, cpLim);
			else  if (edl.fEnd ||
#ifdef MAC
					FMarkLine(psel, hpldr, idr, &edl, cpFirst, cpLim, cpT, fFalse))
#else
				FMarkLine(psel, hpldr, idr, &edl, cpFirst, cpLim, cpT))
#endif
						break;
			}
		}
}


/* C L E A R  I N S E R T  L I N E */
/* %%Function:ClearInsertLine %%Owner:bryanl */
ClearInsertLine(psel) /* moved to disp1.c in WIN */
struct SEL *psel;
{
	if (psel->fIns && psel->fOn && !psel->fNil)
		DrawInsertLine(psel);
}


#ifdef DEBUG
/* %%Function:C_DrawInsertLine %%Owner:bryanl */
HANDNATIVE C_DrawInsertLine(psel)
struct SEL *psel;
{
	struct WWD *pwwd = PwwdWw(psel->ww);
	struct RC rcIns, rcT;
	HBRUSH       hbrOld;
	int xwLeftT;
	int plt;

	Assert( pwwd->hdc != NULL );
	SetRect( (LPRECT) &rcIns, psel->xw, psel->yw - psel->dyp + 1,
			psel->xw + (vsci.dxpBorder << 1),
			psel->yw - vsci.dypBorder + 1 );
	if (!FSectRc( &rcIns, &psel->rcwClip, &rcIns ))
		{
		selCur.fOn = fFalse;
		return;
		}

/* following line assumes we do not need to take responsibility for
	clipping the insert line on the right */


	hbrOld = SelectObject(pwwd->hdc, vsci.hbrText);
	xwLeftT = rcIns.xpLeft - 1;
	plt = pltVert | pltInvert | pltDouble;
	if (pwwd->fPageView && xwLeftT < psel->rcwClip.xwLeft)
		{
		struct RC rcwPage;
		RceToRcw(pwwd, &pwwd->rcePage, &rcwPage);
		if (xwLeftT < rcwPage.xwLeft)
			{
/* make ip thin only if on paper edge */
			plt = pltVert | pltInvert;
			xwLeftT = rcwPage.xwLeft;
			}
		}

	DrawPatternLine( pwwd->hdc, xwLeftT, rcIns.ypTop,
			rcIns.ypBottom - rcIns.ypTop,
			psel->pa == paDotted ? ipatVertGray : ipatVertBlack,
			plt );


	if (hbrOld != NULL)
		SelectObject(pwwd->hdc, hbrOld);

	psel->fOn = 1 - psel->fOn;
	Scribble(ispInsertLine, psel->fOn ? '|' : ' ');
}


#endif /* DEBUG */


/* D r a w  P a t t e r n  L i n e */
/* Draw horizontal or vertical line in specified pattern by
	blting pattern from pattern bitmap.  Caller is
	responsible for setting up the foreground and background brushes */

/* %%Function:DrawPatternLine %%Owner:bryanl */
EXPORT DrawPatternLine( hdc, xpDest, ypDest, dzp, ipat, plt )
HDC hdc;
int xpDest, ypDest, dzp, ipat;
WORD plt;
{
	extern HBITMAP vhbmPatVert, vhbmPatHorz;
	HBITMAP hbmOld, hbmPat;
	long rop = (vsci.fInvertMonochrome ? ROP_DPSxx : ROP_PDSxxn);
	int dzpSkinny = (plt & pltVert ? vsci.dxpBorder : vsci.dypBorder);
	int dxpBlt, dypBlt, xpSrc, ypSrc, dzpSrcMac;
	int *pdzpBlt, *pzpDest, *pzpSrc;
	int fFirstTime = fTrue;
	int cBitAlign;
	struct MDCD *pmdcd;

	if (!(plt & pltInvert))
		{
		if (hdc == vsci.hdcScratch)
			rop = SRCCOPY;
		else
			rop = vsci.dcibScreen.ropBitBlt;
		}
	if (plt & pltOnePixel)
		dzpSkinny = 1;
	else  if (plt & pltDouble)
		{
		dzpSkinny <<= 1;
/* currently we do not support double-height, only double-width.
	This is purely to save space in the pattern bitmaps */
		Assert( plt & pltVert );
		}

/* initialize  for common loop that works either horizontally or vertically */

	if (plt & pltVert)
		{
		dxpBlt = dzpSkinny;
		pdzpBlt = &dypBlt;
		pzpDest = &ypDest;
		pzpSrc = &ypSrc;
		dzpSrcMac = dypHbmVert;
		hbmPat = vhbmPatVert;
		ypSrc = 0;
		xpSrc = ipat * (8);  /* 2 8-bit patterns in vert bitmap */

#ifdef BZTEST   /* for testing bitmap */
		xpSrc = 0;
		rop = SRCCOPY;
		dxpBlt = 16;
		dypBlt = dypHbmVert;
#endif


		}

	else
		{
		dypBlt = dzpSkinny;
		pdzpBlt = &dxpBlt;
		pzpDest = &xpDest;
		pzpSrc = &xpSrc;
		dzpSrcMac = dxpHbmHorz;
		hbmPat = vhbmPatHorz;
		xpSrc = 0;
		ypSrc = ipat * vsci.dypBorder;
		}

/* Bitmaps shrink out of existence in preview...do this instead */
	if (vlm == lmPreview)
		{
		*pdzpBlt = dzp;
		DrawPrvwLine(hdc, xpDest, ypDest, dxpBlt, dypBlt, colAuto);
		return;
		}

/* select pattern bitmap into an appropriate memory DC */
	Assert( hbmPat );
	pmdcd = (hdc == vsci.mdcdScratch.hdc ? &vsci.mdcdBmp :&vsci.mdcdScratch);
	Assert( pmdcd->hdc != NULL );

/* following line "cheats", bypassing the cached pbmi for the mdcd.
	All will be well since the old hbm gets restored at the end of this routine.
	Could avoid this by building global BMI's for the pattern bitmaps */

	if ((hbmOld = SelectObject( pmdcd->hdc, hbmPat )) == NULL)
		return NULL;

/* draw the line in sections that are as large as our source bitmap */

	while (dzp > 0)
		{
		*pdzpBlt = min( dzp, dzpSrcMac );

		if (fFirstTime)
			{
			fFirstTime = fFalse;
/* Align start point so it matches the pattern of a line drawn from 0 */
			if ((cBitAlign = (*pzpDest % (lcmPat * 8))) != 0)
				{
				*pdzpBlt = min( *pdzpBlt, dzpSrcMac - cBitAlign );
				*pzpSrc = cBitAlign;
				}
			}
		else
			*pzpSrc = 0;

		BitBlt( hdc, xpDest, ypDest, dxpBlt, dypBlt,
				pmdcd->hdc, xpSrc,  ypSrc, rop );

		*pzpDest += *pdzpBlt;
		dzp -= *pdzpBlt;
		}

	Assert(hbmOld != NULL);
	SelectObject( pmdcd->hdc, hbmOld );   /* so cache stays accurate */
}


int	vywEndmarkLim = 0x7FFF;

/* D R A W  E N D M A R K */
/* draws one of:
emkBlank: blanks rest of dr (up to ypLim actually)
emkSplat: draws splat in PageView fashion (above ypTop, except for =0 case)
emkEndmark: draws galley endmark
*/
/* %%Function:DrawEndmark %%Owner:bryanl */
EXPORT DrawEndmark(ww, hpldr, idr, ypTop, ypLim, emk)
int		ww;
struct PLDR	**hpldr;
int		idr;
int		ypTop, ypLim;
int		emk;
{
	struct WWD	*pwwd = PwwdWw(ww);
	struct DR	*pdr;
	HDC		hdc = pwwd->hdc;
	int		xwLeft0;
	struct RC	rcw;
	struct RC	rcp;
	struct RC	rcT;
	struct DRF	drfFetch;
	struct RC	rcwClip, rcwErase, rcwDraw;
	int		ywBottomDr;


	pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
	RcpToRcw(hpldr, idr, 
			PrcSet( &rcp, 0, 0, pdr->drcl.dxl, pdr->drcl.dyl),
			&rcw);
	if (!pwwd->fPageView && (*hpldr)->hpldrBack == hNil /* not in table */)
		rcw.xwRight = pwwd->xwMac;

	rcw.ywTop += ypTop;
/* rcw now describes the rectangle in the dr starting at ypTop. */
	switch (emk)
		{
	case emkBlank:
/* set up clip rect */
		if (pwwd->fPageView)
			RceToRcw(pwwd, &pwwd->rcePage, &rcwClip);
		else
			rcwClip = pwwd->rcwDisp;

		ywBottomDr = rcw.ywBottom;
		rcw.ywBottom = rcw.ywTop + ypLim - ypTop;
		rcw.xwLeft -= pdr->dxpOutLeft;
		rcw.xwRight += pdr->dxpOutRight;
		if (pwwd->fPageView && FDrawPageDrsWw(ww))
			{
/* erase in pieces to avoid border flicker; rcw spans the
	dr exactly from left to right */
			rcw.ywBottom = min(ywBottomDr, rcw.ywBottom);
			rcwErase = rcw;
			if (ywBottomDr == rcw.ywBottom)
				rcwErase.ywBottom -= vsci.dypBorder;
			rcwErase.xwRight = rcwErase.xwLeft + pdr->dxpOutLeft;
			if (FSectRc(&rcwErase, &rcwClip, &rcwDraw))
				PatBltRc(hdc, &rcwDraw, vsci.ropErase);
			rcwErase.xwLeft = rcwErase.xwRight + vsci.dxpBorder;
			rcwErase.xwRight = rcw.xwRight - pdr->dxpOutRight - vsci.dxpBorder;
			if (FSectRc(&rcwErase, &rcwClip, &rcwDraw))
				PatBltRc(hdc, &rcwDraw, vsci.ropErase);
			rcwErase.xwLeft = rcwErase.xwRight + vsci.dxpBorder;
			rcwErase.xwRight = rcw.xwRight;
			if (FSectRc(&rcwErase, &rcwClip, &rcwDraw))
				PatBltRc(hdc, &rcwDraw, vsci.ropErase);
			}
		else
			{
			if (FSectRc(&rcw, &rcwClip,&rcw))
				PatBltRc(hdc, &rcw, vsci.ropErase);
			}
		return;
	case emkSplat:
		if (ypTop > 0)
			rcw.ywTop--;
		DrawPatternLine( hdc, rcw.xwLeft, rcw.ypTop, DxOfRc(&rcw),
				ipatHorzGray, pltHorz );
		return;
	case emkEndmark:
		rcw.xwRight = max( rcw.xwLeft + vsci.dxpScrlBar,
				pwwd->xwLimScroll );
		rcw.ywBottom = max( rcw.ywTop + 10 * vsci.dypBorder,
				min( rcw.ywBottom, vywEndmarkLim ) );
		if (!FEmptyRc( &pwwd->rcwInval ))
			{
			rcw.xwRight = max( rcw.xwRight, pwwd->rcwInval.xwRight );
			rcw.ywBottom = max( rcw.ywBottom, pwwd->rcwInval.ywBottom );
			}
#ifdef BRYANL
		CommSzNum( SzShared( "DRAW ENDMARK, height: "), rcw.ywBottom - rcw.ywTop );
		CommSzNum( SzShared( "         blank width: "), rcw.xwRight - rcw.xwLeft );
#endif
		break;
		}

	xwLeft0 = rcw.xwLeft;
	rcw.xwLeft -= pdr->dxpOutLeft;
	PatBltRc( hdc, &rcw, vsci.ropErase );
	DrawPatternLine( hdc, xwLeft0, rcw.ypTop + 4 * vsci.dypBorder,
			vsci.dxpScrlBar, ipatHorzBlack, pltHorz );
	DrawPatternLine( hdc, xwLeft0, rcw.ypTop + 5 * vsci.dypBorder,
			vsci.dxpScrlBar, ipatHorzBlack, pltHorz );
	/* erase left of rcwDisp */
	PatBltRc( hdc, 
			PrcSet( &rcT, 0, rcw.ywTop, pwwd->xwSelBar, rcw.ywBottom ),
			vsci.ropErase );
	/* draw styname border */
	if (pwwd->xwSelBar > 0)
		DrawStyNameBorder(hdc, 
				PrcSet( &rcw, 
				pwwd->xwSelBar - vsci.dxpBorder,
				ypTop + pwwd->ywMin,	/* to screen coordinate */
		pwwd->xwSelBar,
				pwwd->ywMac ));
}


/* S C R O L L  W W */
/* primitive to move dyp scanlines in dr up or down */
/* %%Function:ScrollWw %%Owner:bryanl */
EXPORT ScrollWw(ww, hpldr, idr, ypFrom, ypTo, dyp)
struct PLDR **hpldr;
{
	struct WWD *pwwd;
	struct DR *pdr;
	struct SEL *psel;
	BOOL fInnerDr, fFrameLines, fFatLine, fFatLineAfter;
	HDC hdc;
	int ywFrom, ywTo;
	struct RC rcDisp, rcScroll;
	struct DRF drf;

	if (dyp >= 0)
		{
		FreezeHp();
		pwwd = PwwdWw(ww);
		hdc = pwwd->hdc;

		pdr = PdrFetch(hpldr, idr, &drf);
		rcDisp = pwwd->rcwDisp;
		DrclToRcw(hpldr, &pdr->drcl, &rcScroll);
		ywFrom = rcScroll.ywTop + ypFrom;
		ywTo = rcScroll.ywTop + ypTo;

		if (pwwd->fPageView || (*hpldr)->hpldrBack != hNil)
			{
			rcScroll.xwLeft -= pdr->dxpOutLeft;
			rcScroll.xwRight += pdr->dxpOutRight;
			}
		else
			{
/* include the style name area to vertical scrolling */
			if (pwwd->xwSelBar != 0)
				{
				rcDisp.xwLeft = 0;
				rcScroll.xwLeft = 0;
				}
			else
				rcScroll.xwLeft = pwwd->xwMin;
			rcScroll.xwRight = pwwd->xwMac;
			}
		fInnerDr = (*hpldr)->hpldrBack != NULL;
		fFrameLines = fInnerDr ? FDrawTableDrsWw(ww) : pwwd->fPageView && FDrawPageDrsWw(ww);
		fFatLine = pdr->fFatLine && ywFrom + dyp >= rcScroll.ywBottom;
		/* FUTURE: this is an approximate rule, best we can do with the
		/* info in hand. The callers of this routine could pass in info to
		/* tell us whether or not the DR will overflow after the scroll.
		/**/
		fFatLineAfter = pdr->fFatLine && ywTo + dyp >= rcScroll.ywBottom;
		rcScroll.ywTop = ywFrom;
		rcScroll.ywBottom = rcScroll.ywTop + dyp;
		if (!fFrameLines)
			rcScroll.xwRight = min( rcScroll.xwRight, pwwd->xwLimScroll );
		MeltHp();
		if ((psel = PselActive())->ww == ww && psel->sk == skBlock)
			{
			rcScroll.xwRight = max( rcScroll.xpRight,XwFromXp(hpldr, idr, psel->xpLim)  );
			pwwd->xwLimScroll = max(pwwd->xwLimScroll, rcScroll.xwRight);
			}

#ifdef BRYANL
		CommSzNum( SzShared( "Scrolling, xw Scroll Limit is: "), rcScroll.xwRight );
#endif
		if (fFrameLines)
			FrameDrLine(ww, hpldr, idr, ywFrom, ywFrom+dyp,
					fFatLine, fFalse/*fForceLine*/, fInnerDr);
		ScrollDC( hdc, 0, ywTo - ywFrom, (LPRECT)&rcScroll,
				(LPRECT)&rcDisp, (HRGN) NULL, (LPRECT) NULL );
		if (fFrameLines)
			FrameDrLine(ww, hpldr, idr, ywTo, ywTo+dyp,
					fFatLineAfter, fFalse/*fForceLine*/, fInnerDr);
		pdr->fFatLine = fFatLineAfter;
		FreePdrf(&drf);
		}
}




/* S C R O L L  D E L T A */
/* Based on the consequtive scroll record, determine the
	amount to be scrolled.  */
/* Moved from curskeys.c for swap tuning purpose. */
/* %%Function:ScrollDelta %%Owner:bryanl */
ScrollDelta(dSty, cScroll, dlMax, dypWw, pdysMin, pdysMax)
int	dSty;
int	cScroll;
int	dlMax;
int	dypWw;
int	*pdysMin;
int	*pdysMax;
{
	/* Current rule:
		less than 1/3 of dlMax         --> scroll by line.
		between 1/3 and 3/4 of dlMax   --> scroll by 1/6 screen.
		more than 3/4 of dlMax         --> scroll by 1/4 screen.
	*/
	if (cScroll < dlMax / 3)
		{
		dSty = min(dlMax / 4, dSty);
		*pdysMin = vsci.dysMinAveLine * dSty;
		*pdysMax = vsci.dysMacAveLine * dSty;
		return;
		}
	else  if (cScroll < (dlMax * 3) / 4)
		{
		/* So that we won't look like we are not scrolling
			on the benchmark doc. */
		*pdysMin = *pdysMax = (dypWw / 6) + vsci.dysMinAveLine;
		}
	else
		{
		/* So that we won't look like we are not scrolling
			on the benchmark doc. */
		*pdysMin = *pdysMax = (dypWw / 4) + vsci.dysMinAveLine;
		}
	*pdysMax += vsci.dysMinAveLine;
}


#ifdef REFERENCE /* In DrawEndmark */
/* E R A S E   I N   D R */
/* %%Function:EraseInDr %%Owner:NOTUSED */
EraseInDr(ww, hdc, prcw, hpldr, idr, fPartialErase)
int ww;
HDC hdc;
struct RC *prcw;        /* the portion of the dr you want to erase */
struct PLDR **hpldr;
int idr;
int fPartialErase;      /* ENABLES partial erasing; vpref overrides */
{
	struct DR *pdr;
	struct RC rcwErase;
	struct DRF drfFetch;
	struct RC rcwClip;
	struct WWD *pwwd = PwwdWw(ww);

	if (pwwd->fPageView)
		{
		RceToRcw(pwwd, &pwwd->rcePage, &rcwClip);
		}
	else
		rcwClip = pwwd->rcwDisp;

	if (fPartialErase && FDrawPageDrsWw(ww))
		{
		/* erase in pieces to avoid border flicker; rcw spans the
			dr exactly from left to right */
		rcwErase = *prcw;
		pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
/* shouldn't have reduce the bottom if it is not at the bottom edge of dr */
		rcwErase.ywBottom -= vsci.dypBorder;
		rcwErase.xpRight = rcwErase.xpLeft + pdr->dxpOutLeft;
		SectRc(&rcwErase, &rcwClip, &rcwErase);
		PatBltRc(hdc, &rcwErase, vsci.ropErase);
		rcwErase.xpLeft = rcwErase.xpRight + vsci.dxpBorder;
		rcwErase.xpRight = prcw->xpRight - pdr->dxpOutRight - vsci.dxpBorder;
		SectRc(&rcwErase, &rcwClip, &rcwErase);
		PatBltRc(hdc, &rcwErase, vsci.ropErase);
		rcwErase.xpLeft = rcwErase.xpRight + vsci.dxpBorder;
		rcwErase.xpRight = prcw->xpRight;
		SectRc(&rcwErase, &rcwClip, &rcwErase);
		PatBltRc(hdc, &rcwErase, vsci.ropErase);
		}
	else
		{
		SectRc(prcw, &rcwClip, prcw);
		PatBltRc(hdc, prcw, vsci.ropErase);
		}
}


#endif /* REFERENCE */


/* C L I P  R E C T  F R O M  D R  */
/* Creates the correct clipping rectangle given a dr and the
	line we want to draw in that dr */
/* %%Function:ClipRectFromDr %%Owner:bryanl */
EXPORT ClipRectFromDr(hwwd, hpldr, idr, pdrcp, prcwClip)
struct WWD **hwwd;
struct PLDR **hpldr;
int idr;
struct RC *pdrcp, *prcwClip;    /* Return clip rectangle in prcwClip */
{
	struct RC rcw, rcwDr, rcwPage;
	int ywPageTop, ywPageBottom, idrCur;
	struct PLDR **hpldrCur;
	struct DOP *pdop;
	struct DR *pdr;
	struct WWD *pwwd;
	struct DRF drfFetch;

	/* Convert the line into window coordinates */
	DrcpToRcw(hpldr, idr, pdrcp, &rcw);

	hpldrCur = hpldr;
	idrCur = idr;

	/* Intersect the bounds of each dr with our line */
	while (hpldrCur != hNil)
		{
		/* Quit early for Galley View */
		if (!(*hwwd)->fPageView && (*hpldrCur)->hpldrBack == hNil)
			break;
		pdr = PdrFetchAndFree(hpldrCur, idrCur, &drfFetch);
		DrclToRcw(hpldrCur, &pdr->drcl, &rcwDr);
		rcwDr.xwRight += pdr->dxpOutRight;
		rcwDr.xwLeft -= pdr->dxpOutLeft;
		if ((*hpldrCur)->hpldrBack != hNil)
			{
			/* Only do this for inner drs (otherwise dyl is bogus) */
			if (pdr->drcl.yl + pdr->drcl.dyl > (*hpldrCur)->dyl)
				rcwDr.ywBottom = rcwDr.ywTop + (*hpldrCur)->dyl - pdr->drcl.yl;
			}
		SectRc(&rcw, &rcwDr, &rcw);
		idrCur = (*hpldrCur)->idrBack;
		hpldrCur = (*hpldrCur)->hpldrBack;
		}

	/* Now clip to the window */
	pwwd = *hwwd;
	SectRc(&rcw, &pwwd->rcwDisp, &rcw);

	/* If in pageview, clip to the top and bottom page bounds */
	if (pwwd->fPageView)
		{
		RceToRcw(pwwd, &pwwd->rcePage, &rcwPage);
		SectRc(&rcw, &rcwPage, &rcw);
		}

	*prcwClip = rcw;
}



/* %%Function:UpdateWindowWw %%Owner:bryanl */
UpdateWindowWw(ww)
int ww;
{
	extern HWND vhwndApp;
	struct RC rcInval;
	int fSelectOn;
	struct SEL *psel;

/* not sure why we want to do this since the app window is 
totally covered by ribbon, status line, and desktop anyway. (cc) */

/* we needed to do this in Write to be sure the app window
	borders, title, etc got painted.  I don't see why we should take it 
	out -- the borders and title are not covered by children. (bl)*/

/* First, get the update rect for the parent.  This will do necessary background
	erasure and border validation, the returned rectangle is ignored.
	Test for wwCur assures we only do this once per UpdateWindowWw */
	if (ww == wwCur)
		GetUpdateRect( vhwndApp, (LPRECT) &rcInval, fTrue /* fErase */ );

	if (vidf.fDead)
		{
		/* If we are in the process of shutting down, we DONT want to repaint,
		but we DO want to erase the bkgrnd and validate the border */
		PatBltRc( PwwdWw(ww)->hdc, &rcInval, vsci.ropErase );
		return;
		}

	/*  WINDOWS BUG: ValidateRect does not work for Iconic apps in Win 2.xx */
	if (vidf.fIconic)
		{
		PAINTSTRUCT ps;
		BeginPaint(PwwdWw(ww)->hwnd, (LPPAINTSTRUCT) &ps);
		EndPaint(PwwdWw(ww)->hwnd, (LPPAINTSTRUCT) &ps);
		return;
		}

	if (FWindowHidden(ww))
		{
		ValidateRect( PwwdWw(ww)->hwnd, (LPRECT) NULL );
		/* cheaper than calling BeginPaint and EndPaint */
		return;
		}

	psel = PselActive();
	fSelectOn = psel->fOn;
	UpdateWw( ww, fFalse );
	if (fSelectOn != psel->fOn)
		{
		/*DrawInsertLine(psel); doing this will leave ghost cursor */
		if (fSelectOn)
			TurnOnSel(psel);
		else
			TurnOffSel(psel);
		}
}


/* P R C  S E L  B A R */

/* %%Function:PrcSelBar %%Owner:bryanl */
struct RC *PrcSelBar( ww, prc )
int ww;
struct RC *prc;
{
	struct WWD *pwwd = PwwdWw(ww);

	return PrcSet( prc, pwwd->xwSelBar, 
			pwwd->ywMin, 
			pwwd->xwSelBar + dxwSelBarSci,
			pwwd->ywMac );
}


/* %%Function:GetSelBarRect %%Owner:bryanl */
GetSelBarRect( ww, prc )
int ww;
struct RC *prc;
{
	struct WWD *pwwd = PwwdWw(ww);

	PrcSet( prc, pwwd->xwSelBar, 
			pwwd->ywMin, 
			pwwd->xwSelBar + dxwSelBarSci,
			pwwd->ywMac );
}


/* The following two lines of comments is copied straight from
	Mac code.  It is beyond me what they meant by this....... */
/* D R A W  V I S I  B I T S  H I  M I N U S */
/* draws the constant for tables */
/* %%Function:DrawVisiBitsOtlMarkBody %%Owner:bryanl */
DrawVisiBitsOtlMarkBody(ww, xwT, ywT)
{
	DrawOtlPat(ww, PwwdWw(ww)->hdc, idcbOtlMarkBody, xwT, ywT);
}


/*  Following code was SCREEN.C */

extern struct PREF          vpref;
extern struct WWD           **hwwdCur;
extern int		    wwCreating;
extern struct SCI           vsci;
extern struct PRI           vpri;
extern struct FLI           vfli;
extern int                  vflm;
extern int                  vlm;
extern int                  vfPrvwDisp;
extern struct FTI           vfti;
extern struct FTI           vftiDxt;
extern struct MERR          vmerr;
extern struct BMI           vbmiEmpty;
extern int                  vfInitializing;
#ifdef WIN23
extern struct BMI           *mpidrbbmi;
#else
extern struct BMI           mpidrbbmi[];
#endif /* WIN23 */
extern HANDLE               vhInstance;
extern int                  wwCur;
extern struct WWD           **mpwwhwwd[];
extern IDF		    vidf;

/* F T I  C o n n e c t i o n  M o d e  */
#define fticmScreen   0
#define fticmPrinter  1
#define fticmIC       2
#define fticmForce    4   /* force creation */


DWORD GetRgbIco(int);

/* C O L O R  note that 0 corresponds to icoBlack, not icoAuto */
csconst long rgco[8] =
{
	rgbBlack,   
			rgbBlue,    
			rgbCyan,    
			rgbGreen,   
			rgbMagenta, 
			rgbRed,     
			rgbYellow,  
			rgbWhite   
};



#ifdef DEBUG	/* special SetFlm debugging aid, temporary (BL) */

#define iflmMax	6
struct DSF {
	int fDcFail;
	int rgflm [iflmMax];
};

struct DSF	vdsf = { 0, -1, -1, -1, -1, -1, -1 };
#endif


/* S E T   F L M */
/* This must be declared NATIVE or EXPORT because it is called by
	N_FormatLineDxa. */
/* %%Function:SetFlm %%Owner:bryanl %%reviewed 7/10/89 */
NATIVE SetFlm( flm ) /* Only minimal preface code is native. */
int flm;
	{   /* Set FormatLine mode.  Choices are:  displaying, displaying as printed,
		printing, repaginating, idle, tossing printer DC.
	
		Sets up:
	
			vfti        vftiDxt         vsci                 vpri
			vfli.fPrint                 vfli.fFormatAsPrint
	
		Note the following anomaly: vftiDxt.dxpInch and vftiDxt.dypInch
		do not always match the resolution of the device connected to 
		vftiDxt.  This is because FormatLine requires that 
		vfti.dxpInch == vftiDxt.dxpInch etc. for the one-device cases
		(printing, display not as printed), while we sometimes save
		a connection in vftiDxt as an optimization.
	
		Does nothing if flm matches current mode
*/


	struct FCE *pfce;

#ifdef DEBUG 
/* REVIEW BryanL (tk): Are these fFoo's a remnant of some debugging?
 * They do not appear to be used, and redefining a name like that hoses
 * a new CS compiler (so says BobD). */
/*	int fFoo = -1; */
	struct DSF dsf;
/*	int fFoo = -2; */
#endif
	/* call ShakeHeap() here because someone might assume we will
		not move when we can, at least for flms involving the
		printer, which may need to load fonts
	*/
	Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);

	if (flm == vflm)
		return;

		{{ /* !NATIVE - SetFlm */
		Assert( !(flm != flmIdle && vidf.fDead) ); /* For efficiency, shouldn't call SetFlm while exiting */

#ifdef DEBUG	   /* Put FLM history on stack to enhance RIP file */
		blt( &vdsf.rgflm [0], &vdsf.rgflm [1], iflmMax - 1 );
		vdsf.rgflm [0] = flm;
		dsf = vdsf;
#endif

		Debug( vdbs.fCkFont ? CkFont() : 0 );
		switch (flm) 
			{

#ifdef DEBUG
		default:   
			Assert( fFalse );
			break;
#endif

		case flmTossPrinter:
/* don't change flm, just toss printer DC */
/* for swap tuning reasons, caller should not call during following conditions */
			Assert( vlm != lmPrint );
			Assert( vpri.pfti != NULL );
			Assert( vpri.hdc );

#ifdef BRYANL
			CommSzSz(SzFrame("Tossing printer DC"),SzFrame(""));
#endif
			FreeHdcPrinter();
			Assert( vpri.pfti );
			vpri.pfti->fTossedPrinterDC = fTrue;
			flm = vflm;
			break;

		case flmIdle:
LFlmIdle:
			DisconnectFromFti( &vfti );
			DisconnectFromFti( &vftiDxt );
			vfli.fPrint = vfli.fFormatAsPrint = fFalse;
			break;

		case flmDisplayAsPrint:

			if (vpref.fDraftView)
				goto LFlmDisplay;

/* printer is not accessible; don't waste time asking for a DC */
			if (vmerr.fPrintEmerg)
				{
				Assert( vpri.hdc == NULL );
				goto LFlmDisplay;
				}

/* take advantage of existing connections */

			if (vpri.pfti == &vfti || vsci.pfti == &vftiDxt)
				SwapFtiConnections();
			ConnectToFti( &vftiDxt, (fticmPrinter | fticmIC) );
			if (vpri.hdc != NULL || vftiDxt.fTossedPrinterDC)
				{
				Assert( vpri.pfti == &vftiDxt );
				vfli.fFormatAsPrint = fTrue;
				vftiDxt.dxpInch = vfli.dxuInch;
				vftiDxt.dypInch = vfli.dyuInch;
				goto LDisplayCommon; /* go connect screen to vfti */
				}

/* No accessible printer DC -- revert to plain display */
LFlmDisplay:
			flm = flmDisplay;
		/* FALL THROUGH */
		case flmDisplay:
			if (vpri.pfti == &vfti)
				SwapFtiConnections();
			vfli.fFormatAsPrint = fFalse;
			vftiDxt.dxpInch = vfli.dxsInch;
			vftiDxt.dypInch = vfli.dysInch;

LDisplayCommon:
			vfli.fPrint = fFalse;
			ConnectToFti( &vfti, fticmScreen );
			vfti.dxpInch = vfli.dxsInch;
			vfti.dypInch = vfli.dysInch;
			vfti.dxpBorder = vsci.dxpBorder;
			vfti.dypBorder = vsci.dypBorder;
			break;

		case flmRepaginate:
		case flmPrint:

/* printer is not accessible; just return */
			if (vmerr.fPrintEmerg)
				{
				Assert( vpri.hdc == NULL );
				return;
				}

/* preserve screen connection, if any, by saving it in vftiDxt; make use of
	existing printer connection in vftiDxt, if any, by moving it to vfti.
	If we already have vfti<->printer, vftiDxt<->screen, do not disturb */
			if (vsci.pfti != NULL && vpri.pfti != &vfti)
				SwapFtiConnections();

/* printer DC was tossed away, but no more: we need it for this flm */
/* must free fonts to remove ibstFontNil entries that might have
	gotten in there while the DC was tossed */
			if (vpri.pfti && vpri.pfti->fTossedPrinterDC && flm == flmPrint)
				{
				FreeFontsPfti( vpri.pfti );
				vpri.pfti->fTossedPrinterDC = fFalse;
				}

			ConnectToFti( &vfti, flm == flmRepaginate ? 
					fticmPrinter | fticmIC :
					fticmPrinter | fticmForce );
/* no printer available -- can't go into this flm */
			if (vpri.hdc == NULL && !(vpri.pfti != NULL && vpri.pfti->fTossedPrinterDC))
				{
				flm = flmIdle;
				goto LFlmIdle;
				}
			vfli.fPrint = fTrue;
			vfli.fFormatAsPrint = fFalse;
			vftiDxt.dxpInch = vfti.dxpInch = vfli.dxuInch;
			vftiDxt.dypInch = vfti.dypInch = vfli.dyuInch;
			vfti.dxpBorder = max(1,NMultDiv(vsci.dxpBorder, vfli.dxuInch, vfli.dxsInch<<vpri.ptScaleFactor.xp))<<vpri.ptScaleFactor.xp;
			vfti.dypBorder = max(1,NMultDiv(vsci.dypBorder, vfli.dyuInch, vfli.dysInch<<vpri.ptScaleFactor.yp))<<vpri.ptScaleFactor.yp;
			break;
			}

		vflm = flm;
/* blow away nasty cached data in case we changed devices (printer -> screen
	or screen -> printer) */
		vtcc.ca.doc = docNil;
		vtcc.caTap.doc = docNil;
		vtcxs.ca.doc = docNil;
		InvalFli();
		Debug( vdbs.fCkFont ? CkFont() : 0 );
		}}
}


/* C o n n e c t  T o  F t i */
/* Connect the specified device to the specified FTI.  A requirement
	is that there is no other device connected to the FTI. 
*/

/* %%Function:ConnectToFti %%Owner:bryanl %%reviewed: 7/10/89 */
ConnectToFti( pfti, fticm )
struct FTI *pfti;
int fticm; /* Fti Connection Mode */
{
	struct FTI **ppfti;
	int fIC = ((fticm & fticmIC) != 0);
	int fPrinter = (fticm & fticmPrinter);
	Debug( int fICPrev = vpri.fIC );

	ppfti = (fPrinter ? &vpri.pfti : &vsci.pfti );
	if (*ppfti == pfti && !(fPrinter && (vpri.hdc == NULL ||
			(uns)fIC < (uns)vpri.fIC)))
		return;
/* following assert means: if the requested device was not already
	connected to pfti, then no device should be connected to pfti.
	The case should have been taken care of by the higher level procs.
*/
	Assert( *ppfti == pfti || *ppfti == NULL || (fPrinter && fIC != vpri.fIC));
	*ppfti = pfti;
	if (pfti->fPrinter = fPrinter)
		{
		if (fIC != vpri.fIC)
			{
			FreeHdcPrinter();
			fticm |= fticmForce;
			}
		if (!pfti->fTossedPrinterDC || (fticm & fticmForce))
			{
			AllocHdcPrinter( fIC );
			pfti->fTossedPrinterDC = fFalse;
			if (vpri.hdc == NULL)
				{
				struct FCE *pfce;
#ifdef DEBUG
				vdsf.fDcFail = fTrue;
#endif
				for ( pfce = pfti->pfce;  pfce != NULL;  pfce = pfce->pfceNext )
					FreeHandlesOfPfce( pfce );
				pfti->pfce = NULL;
				*ppfti = NULL;
				vmerr.fPrintEmerg = fTrue;

				ReportSz("Warning - Printer dc alloc failure");
#ifdef BRYANL
				CommSzSz( "Driver: ", **vpri.hszPrDriver );
				CommSzSz( "Printer: ", **vpri.hszPrinter );
				CommSzSz( "Port: ", **vpri.hszPrPort );
#endif
				}
			}
		}

	pfti->hfont = NULL;
}


/* S w a p  F t i  C o n n e c t i o n s */
/* swap device connections so vfti device is connected to vftiDxt and
	vice versa */

/* %%Function:SwapFtiConnections %%Owner:bryanl */
SwapFtiConnections()
{
	struct FTI ftiT, *pftiT;

/* swap FTI's */

	ftiT = vfti;
	vfti = vftiDxt;
	vftiDxt = ftiT;

/* Adjust device pointers to FTI's */

	if (vpri.pfti)
		vpri.pfti = (vpri.pfti == &vfti ? &vftiDxt : &vfti);
	if (vsci.pfti)
		vsci.pfti = (vsci.pfti == &vfti ? &vftiDxt : &vfti);
}



/* P m d c d  C a c h e  I d r b */
/* Perform setup for use of a resource bitmap.

	Inputs:     idrb            resource id of bitmap.


				vmpidrbbmi [idrb] should be filled out as follows:

					bmi.hbm       handle to resource bitmap, or NULL if it
									needs to be loaded
					bmi.dxp, dyp  Desired size of bitmap (will be stretched to
									fit if not an exact match)
					bmi.fGray     fTrue iff gray form of bitmap is needed

				hdcDest         if this is != vsci.hdcScratch,
								vsci.hdcScratch will be used

	Outputs:    bmi.hbm       bitmap that was loaded 


	Return Value: NULL if the operation failed
					else, a pointer to a MDCD structure for a memory DC
						with the specified bitmap selected into it
*/


/* %%Function:PmdcdCacheIdrb %%Owner:bryanl */
EXPORT struct MDCD *PmdcdCacheIdrb( idrb, hdcDest )
int idrb;
HDC hdcDest;
{
	struct MDCD *PmdcdCachePbmi();
	struct BMI *pbmi;

#ifdef WIN23
	Assert(idrb >= 0 && idrb < (vsci.fWin3Visuals ? idrbMax3 : idrbMax2));
#endif /* WIN23 */
	pbmi = &mpidrbbmi [idrb];

/* load bitmap resource if necessary */
	if (pbmi->hbm == NULL)
		{
		if (!FLoadResourceIdrb( idrb ))
			return NULL;
		}

/* select bitmap into DC */

	return PmdcdCachePbmi( pbmi, hdcDest );
}


/* %%Function:PmdcdCachePbmi %%Owner:bryanl */
struct MDCD *PmdcdCachePbmi( pbmi, hdcDest )
struct BMI *pbmi;
HDC hdcDest;
{
	struct MDCD *pmdcd;

/* maybe this is already cached in one of the memory DC's;
	if so, this avoids selecting it into the other */

	if (pbmi == vsci.mdcdScratch.pbmi)
		{
		Assert( vsci.mdcdScratch.hdc );
		return &vsci.mdcdScratch;
		}
	else  if (pbmi == vsci.mdcdBmp.pbmi)
		{
		Assert( vsci.mdcdBmp.hdc );
		return &vsci.mdcdBmp;
		}

/* choose which memory DC to use: use scratch unless that is the dest;
	in that case, use the Bmp one */

	pmdcd = (hdcDest == vsci.mdcdScratch.hdc ? &vsci.mdcdBmp : &vsci.mdcdScratch);

/* select bitmap into DC */

	if (!FSetPmdcdPbmi( pmdcd, pbmi ))
		return NULL;

	return pmdcd;
}


/* S e t  S c r a t c h  P b m i */

/* %%Function:SetScratchPbmi %%Owner:bryanl */
SetScratchPbmi( pbmi )
struct BMI *pbmi;
	{   /* Select bitmap at *pbmi into vsci.hdcScratch.
	Sets vsci.pbmiScratch. */

	FSetPmdcdPbmi( &vsci.mdcdScratch, pbmi );
}


/* F  S e t  P m d c d  P b m i */

/* %%Function:FSetPmdcdPbmi %%Owner:bryanl */
FSetPmdcdPbmi( pmdcd, pbmi )
struct MDCD *pmdcd;
struct BMI *pbmi;
	{   /* Select bitmap at *pbmi into pmdcd->hdc
	Sets pmdcd->pbmi */

/* If we are selecting a color bitmap into hdcScratch and what was
		there was not a color bitmap, we'll need to reset the text color
		because, due to some bogosity in Windows, it has been set to
		black without our permission. */

	BOOL fResetTextColor;

	if (pmdcd->pbmi == pbmi)
		return fTrue;

/* fResetTextColor = (pbmi != &vbmiEmpty && !(vsci.pbmiScratch)->fSameAsScreen); */
	fResetTextColor = pbmi != &vbmiEmpty;

	if (pmdcd->hdc != NULL)
		{
		HBITMAP hbm=pbmi->hbm;

		if (hbm == NULL)
			{
			hbm = vbmiEmpty.hbm;
			Assert( pbmi->dxp == 0 && pbmi->dyp == 0 );
			}

		pmdcd->pbmi = pbmi;
		Assert( hbm != NULL );
		if (SelectObject( pmdcd->hdc, hbm ) == NULL)
			{
			if (pbmi->fDiscardable)         /* bitmap was discarded */
				{
				UnlogGdiHandle(pbmi->hbm, -1);
				DeleteObject( pbmi->hbm );
				pbmi->dxp = pbmi->dyp = pbmi->hbm = 0;
				}
#ifdef BRYANL
			else
				{


				CommSzSz(SzFrame("Error selecting"),SzFrame(""));

				if (pbmi == &vbmiEmpty)
					CommSzSz(SzFrame(" empty bitmap"),SzFrame(""));
				else 
					CommSzSz(SzFrame(" random or resource bitmap"),SzFrame(""));

				if (pmdcd == &vsci.mdcdScratch)
					CommSzSz(SzFrame(" into scratch mem DC"),SzFrame(""));
				else
					{
					Assert( pmdcd == vsci.mdcdBmp.hdc );
					CommSzSz(SzFrame(" into Bmp mem DC"),SzFrame(""));
					}
				}
#endif  /* BRYANL */
			return fFalse;
			}

		else  if (fResetTextColor)
			SetTextColor( pmdcd->hdc, RGB( 0,0,0 ) );
		}
#ifdef DEBUG
	else
		Assert( pmdcd->pbmi == &vbmiEmpty );
#endif

	return fTrue;
}


/* %%Function:FSetDcAttribs %%Owner:bryanl */
EXPORT int FSetDcAttribs( hdc, dcc )
HDC hdc;
int dcc; /* dc attribute code */
{
	if (hdc == hNil)
		return fFalse;

	if  (dcc & dccmUnlockBrushPen)
		{   /* select in dummy brushes in preparation for freeing */
		SelectObject( hdc, GetStockObject( WHITE_BRUSH ) );
		SelectObject( hdc, GetStockObject( BLACK_PEN ) );
		return fTrue;
		}

#ifdef DEBUG
/*
	if (!(dcc & dccmTextColor))
		Assert( GetTextColor( hdc ) == RGB( 0, 0, 0 ));

	if (!(dcc & dccmBkgrnd))
		Assert( GetBkColor( hdc ) == RGB( 0xff, 0xff, 0xff ));
*/

/* better not use this func to set mem dc colors -- it's not smart enough */
	if (dcc & (dccmBkgrnd | dccmTextColor))
		Assert( hdc != vsci.hdcScratch && hdc != vsci.mdcdBmp.hdc );
#endif

	if (dcc & dccmBkgrnd)
		{
		if (SelectObject( hdc, vsci.hbrBkgrnd ) == NULL)
			return (fFalse);
		if (SetBkColor( hdc, vsci.rgbBkgrnd ) == 0x80000000)
			return (fFalse);
		}

	if (dcc & dccmTextColor)
		SetTextColor( hdc, vsci.rgbText );

	if (dcc & dccmTransparent)
		SetBkMode( hdc, TRANSPARENT );
	else  if (dcc & dccmOpaque)
		SetBkMode( hdc, OPAQUE );

	if (dcc & dccmHpen)
		if (SelectObject( hdc, vsci.hpen ) == NULL)
			return (fFalse);

	if (dcc & dccmHpenBorder)
		if (SelectObject( hdc, vsci.hpenBorder ) == NULL)
			return (fFalse);

	if (dcc & dccmCurFont)
		{
		if (vsci.pfti && vsci.pfti->hfont)
			if (SelectObject( hdc, vsci.pfti->hfont) == NULL)
				return fFalse;
		SetMapperFlags( hdc, ASPECT_FILTERING );
		}

	return (fTrue);
}


/* %%Function:GetRgbIco %%Owner:bryanl */
DWORD GetRgbIco(ico)
int ico;
{
	if (vfli.fPrint || vfPrvwDisp)
		{
		if (ico == icoAuto || (!vpri.fColor && ico != icoWhite))
			return (vpri.rgbText);
		else
			goto LMap;
		}
	else  if (ico == icoAuto)
		return (vsci.rgbText);
	else
LMap:
		return (rgco[ico - 1]);
}


/* %%Function:ForeColor %%Owner:bryanl */
EXPORT ForeColor(hdc, ico)
HDC hdc;
int ico;
{
	SetTextColor( hdc, GetRgbIco(ico) );
}



/*******************************/
/* G e t   X l   M a r g i n s */
/* %%Function:GetXlMargins %%Owner:chic %%reviewed 7/10/89 */
GetXlMargins(pdop, fRight, dxConvert, pxlLeft, pxlRight)
struct DOP *pdop;
int fRight, dxConvert, *pxlLeft, *pxlRight;
{
	int xaLeft, xaRight;

	if (fRight)
		{
		xaLeft = pdop->dxaLeft + pdop->dxaGutter;
		xaRight = pdop->xaPage - pdop->dxaRight;
		}
	else
		{
		xaLeft = (pdop->fMirrorMargins) ? pdop->dxaRight : pdop->dxaLeft;
		xaRight = pdop->xaPage - pdop->dxaGutter -
				((pdop->fMirrorMargins) ? pdop->dxaLeft : pdop->dxaRight);
		}
	*pxlLeft = NMultDiv(xaLeft, dxConvert, dxaInch);
	*pxlRight = NMultDiv(xaRight, dxConvert, dxaInch);
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Disp1_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Disp1_Last() */
