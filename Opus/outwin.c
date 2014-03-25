/* O U T W I N . C */
/* dispite its name, this module should contain routines required only when
	actually in outline mode.
*/

#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "format.h"
#include "sel.h"
#include "ch.h"
#include "cmd.h"
#include "keys.h"
#include "outline.h"
#include "wininfo.h"
#include "heap.h"
#include "resource.h"
#include "debug.h"
#include "ibdefs.h"
#include "error.h"
#include "field.h"
#include "help.h"
#ifdef WIN23
#include "iconbar.h"
#endif /* WIN23 */

#define ropMonoBm	0x00990066

extern int		wwCur;
extern int		mwCur;
extern struct CAP       vcap;
extern int		vcConseqScroll;
extern HCURSOR		vhcOtlVert;
extern HCURSOR		vhcOtlHorz;
extern CP               vcpFetch;
extern struct WWD	**hwwdCur;
extern struct MWD	**hmwdCur;
extern struct WWD	**mpwwhwwd[];
extern struct MWD	**mpmwhmwd[];
extern struct SEL       selCur;
extern struct SCI	vsci;
extern struct FTI	vfti;
extern struct FLI	vfli;
extern struct PREF	vpref;
extern int              fcmFetch;
extern BOOL             vfDoubleClick;
extern HWND		vhwndCBT;
extern int              vlm;
extern struct CA        caPara;
extern struct PAP	vpapFetch;
extern HBRUSH		vhbrGray;
extern HCURSOR          vhcOtlCross;

BOOL FHookOutlineKmpMwWw();

int		icnOtl;
FARPROC		lpFBltOutlineSplat;

EXPORT BOOL FAR PASCAL FBltOutlineSplat(HDC, LPRECT, int); /* DECLARATION ONLY */


/* C M D  O U T L I N E */
/* turns on or off the pane's outline mode.
*/
/* %%Function:CmdOutline %%Owner:davidlu */
CMD CmdOutline(pcmb)
CMB *pcmb;
{

	CMD		   cmd;

	Assert( wwCur != wwNil );
	StartLongOp();

	/* if preview is up, blow it away... */
	if (vlm == lmPreview)
		FExecCmd(bcmPrintPreview);

	if (!vhcOtlCross)
		{
		if (!FAssureIrcds(ircdsOtlCross) || !FAssureIrcds(ircdsOtlVert)
				|| !FAssureIrcds(ircdsOtlHorz))
			{
			EndLongOp(fFalse);
			return cmdError;
			}
		}

	Assert( !PdodDoc(selCur.doc)->fShort );

	if (selCur.fBlock)
		TurnOffBlock(&selCur);

	cmd = (FToggleOutline(wwCur)) ? cmdOK : cmdError; 
	EndLongOp(fFalse);
	return cmd;
}


/* %%Function:FToggleOutline %%Owner:davidlu */
BOOL FToggleOutline(ww)
int ww;
{
	struct WWD	**hwwd;
	struct WWD	**hwwdIcon;
	int 		fWwOtherOK = fTrue; 
	int 		wwOther;
	int 		lvlNew;
	int 		fAdjustPane = fFalse;
	int		fAdjustPaneOther = fFalse;  
	BOOL		fIconBar;
	int		cdlT;
	int 		ipad, ipadMac;
	struct WWD 	**hwwdOther;
	struct PLC	**hplcpad; 
	struct PAD 	pad; 

	hwwd = mpwwhwwd[ww];
	fIconBar = fFalse;
	hwwdIcon = hNil;
	wwOther = WwOther(ww);

	if ((*hwwdCur)->fPageView && CmdExecUcm(bcmPageView, hNil) != cmdOK)
		return(fFalse);

	if (!(*hwwd)->fOutline)
		{ /* enable */
		PdrGalley((*hwwd))->dypAbove = 0;

		if (ww == wwCur &&
				((*hwwd)->fHadRuler = 
				(PmwdMw(mwCur)->wwRuler == ww)))
			{
			ShowRulerWwCur(fFalse, fFalse);
			}

/* put up outline iconbar if other pane doesn't have one */
		if (wwOther == wwNil ||	!PwwdWw(wwOther)->fOutline)
			{
			if (fIconBar = FCreateOutlineIcnBr(hwwd))
				{
				hwwdIcon = hwwd;
				AdjustPane(hwwd, vsci.dypIconBar - vsci.dypBorder);
				}
			else
				{
LErrRet:
				/* Restore ruler. */
				if (ww == wwCur && (*hwwd)->fHadRuler)
					ShowRulerWwCur(fTrue, fFalse);
				TrashWw(ww);
				return fFalse;
				}
			}

		if (!FHookOutlineKmpHmwdWw(mpmwhmwd[(*hwwd)->mw], ww))
			goto LErrRetPrep;

		if (!FUpdateHplcpad(selCur.doc))
			{
			ErrorNoMemory(eidNoMemOutline);
			UnhookOutlineKmpHmwdWw(mpmwhmwd[(*hwwd)->mw], ww);
LErrRetPrep:
			DestroyIconBar(hwwd);
			AdjustPane(hwwd, -vsci.dypIconBar+vsci.dypBorder);
			goto LErrRet;
			}
		(*hwwd)->wk = wkOutline;
		(*hwwd)->grpfvisi.flm = FlmDisplayFromFlags(
				(*hwwd)->fDisplayAsPrint, fFalse, vpref.fDraftView);
		InvalVisiCache();
		if (ww == wwCur)
			{
/* expand to outline selection if more than one paragraph is selected */
			CachePara(selCur.doc, selCur.cpFirst);
			if (selCur.cpLim > caPara.cpLim)
				{
				struct CA caT;
				caT = selCur.ca;
				ExpandOutlineCa(&caT, fFalse);
				if (!selCur.fTable && (selCur.cpFirst != caT.cpFirst ||
						selCur.cpLim != caT.cpLim))
					Select(&selCur, caT.cpFirst, caT.cpLim);
				}
			}
		}
	else
		{
/* take down icon bar */
		if ((*hwwd)->hwndIconBar)
			{
			/* pane has an iconbar, so destroy it */
			DestroyIconBar(hwwd);
			fAdjustPane = fTrue; 
	/* 		AdjustPane(hwwd, -vsci.dypIconBar+vsci.dypBorder); */

			if (wwOther != wwNil && PwwdWw(wwOther)->fOutline)
				{
				/* other pane has outline, put up iconbar
					in other pane
				*/
			
				TrashWw(wwOther);
				hwwdOther = mpwwhwwd[wwOther];
				if ((fIconBar = fWwOtherOK = FCreateOutlineIcnBr(hwwdOther)))
					{
					hwwdIcon = hwwdOther;
					PdrGalley((*hwwdOther))->dypAbove = 0;
					fAdjustPaneOther = fTrue; 
				/*	AdjustPane(hwwdOther, vsci.dypIconBar - vsci.dypBorder);*/
					}
				}
			}

		/* Dispose outline keymap. */
		UnhookOutlineKmpHmwdWw(mpmwhmwd[(*hwwd)->mw], ww);

		/* Show Ruler if there was one. */
		if (ww == wwCur && (*hwwd)->fHadRuler)
			{
			ShowRulerWwCur(fTrue, fFalse);
			InvalYpMin(hwwd, vsci.dypRuler);
			}

		(*hwwd)->wk = wkDoc;
		if (fAdjustPane) 
			AdjustPane(hwwd, -vsci.dypIconBar+vsci.dypBorder);
		if (fAdjustPaneOther) 
			AdjustPane(hwwdOther, vsci.dypIconBar - vsci.dypBorder);
				
	
		(*hwwd)->grpfvisi.flm = FlmDisplayFromFlags(
				(*hwwd)->fDisplayAsPrint, (*hwwd)->fPageView, vpref.fDraftView);
		InvalVisiCache();
#ifdef BOGUS
/* Now that ww of different wk does not scroll together, 
this screws up if the other window has not been sync yet */
		NormCp(ww, selCur.doc, selCur.cpFirst, ncpHoriz, 0,
				selCur.fInsEnd);
#endif
		if (!fWwOtherOK)
			{
			Assert(wwOther != wwNil); 
			FToggleOutline(wwOther); 
			}
		}
	
		
/* scroll to normal state, or synch with other pane if there is a wk match */
	SetScrollHoriz(ww, (*hwwd)->xhScroll -
			(WwScrollWkTest(ww) == wwNil ? 0 :
			PwwdOther(ww)->xhScroll));

	if (fIconBar)
		{
		Assert(hwwdIcon != hNil);

		ShowWindow((*hwwdIcon)->hwndIconBar, SHOW_OPENWINDOW);
		InvalYpMin(hwwdIcon, vsci.dypIconBar);
		UpdateWindow((*hwwdIcon)->hwndIconBar);
		}

	if ((*hwwd)->fOutline)
		{
		/* We got into outline mode. */
		if (fIconBar)
			{
			/* We had to put up a new icon bar. */
			PmwdMw((*hwwd)->mw)->lvlOutline =
					PdodDoc(selCur.doc)->lvl;
			Assert((*hwwdIcon)->hwndIconBar != hNil);
			ToggleOutlineLvl((*hwwdIcon)->hwndIconBar, lvlNone,
					lvlNew = PdodDoc(selCur.doc)->lvl);
/* we need to issue the command corresponding to the selected button, because 
   the user may have applied styles and changed the level of the text */
			switch (lvlNew)
				{
			case 0: 
				CmdShowToLvl1(0); 
				break; 
			case 1:
				CmdShowToLvl2(0); 
				break;
			case 2:
				CmdShowToLvl3(0); 
				break;
			case 3:
				CmdShowToLvl4(0); 
				break;
			case 4:
				CmdShowToLvl5(0); 
				break;
			case 5:
				CmdShowToLvl6(0); 
				break;
			case 6:
				CmdShowToLvl7(0); 
				break;
			case 7:
				CmdShowToLvl8(0); 
				break; 
			case 8:
				CmdShowToLvl9(0);
				break; 
			case 9:
				if ((hplcpad = PdodDoc(selCur.doc)->hplcpad) == 0)
					break; 
				ipadMac = IpadMac(hplcpad);
				for (ipad = 0; ipad < ipadMac; ipad++)
					{
					GetPlc(hplcpad, ipad, &pad);
					if (!pad.fShow)
						{
						CmdExpandAll(0);
						break; 
						}
					}
				break; 
				}
			}
		MakeSelCurVisi(fFalse/*fForceBlockToIp*/);
#ifdef ESFICON
		InitESF();
#endif /* ESFICON */
		}
	if (CpMacDoc(selCur.doc) == ccpEop)
		{
		/* for empty docs, change to outline or body if not already */
		CachePara(selCur.doc, cp0);
		if ((*hwwdCur)->fOutline)
			{
			if (vpapFetch.stc < stcLevMin ||
					vpapFetch.stc > stcLevLast)
				{
				CmdOutlnUcm(ucmOtlLeft);
				}
			}
		else  if (vpapFetch.stc >= stcLevMin &&
				vpapFetch.stc <= stcLevLast)
			{
			int ww = wwNil;
			while ((ww = WwDisp(selCur.doc, ww, fFalse)) != wwNil)
				if (PwwdWw(ww)->fOutline)
					break;
			if (ww == wwNil)
				/* no one displaying outline view */
				CmdOutlnUcm(ucmOtlBody);
			}
		}
	SetUndoNil();

	TrashWw(ww);

	/* This will re-normalize pwwd->cpFirst given the new situation. */
	DypScanAbove(ww, 0, &cdlT);
	SyncSbHor(ww);
	return fTrue;
}


/* This is needed because switching in and out of outline mode leaves
	garbage in the ypMin area, especially when a ruler is involved */

/* %%Function:InvalYpMin %%Owner:davidlu */
InvalYpMin(hwwd, dyp)
struct WWD **hwwd;
int dyp;
{
	HWND hwnd = (*hwwd)->hwnd;
	struct RC rc;

	GetClientRect(hwnd, (LPRECT) &rc);
	rc.ypTop = dyp;
	rc.ypBottom = (*hwwdCur)->ywMin;
	InvalidateRect(hwnd, (LPRECT) &rc, fTrue /* fErase */);
}



/* D X P  O U T L I N E  S P L A T S */
/* draws ellipses after prematurely terminated body text and
draws a gray line representing invisible sub-text.
This routine is Windows specific.
Return value is effective width added to dl.
*/

/* %%Function:DxpOutlineSplats %%Owner:davidlu */
EXPORT DxpOutlineSplats(hdc, xp, ptText, prcwOpaque)
HDC hdc;
int xp;
struct PT ptText;
struct RC *prcwOpaque;
{
	char rgch[3];
	struct RC rc;
	int dxp = 0;

	if (!(vfli.chBreak == chEop || vfli.chBreak == chSect))
		{
		/* not last line. */
		if ((*hwwdCur)->fEllip && vfli.fSplatDots)
			{
			rgch[0] = rgch[1] = rgch[2] = '.';
/* FUTURE using vfti here may not be correct, but is used to determine
   height and width of ellipses. */
			ExtTextOut(hdc, ptText.xp, ptText.yp - vfti.dypAscent,
					ETO_CLIPPED,         /* action bits */
			(LPRECT)prcwOpaque,  /* opaque rect */
					(LPCH) rgch,         /* string */
					3,                   /* length */
			(int far *)(DWORD) NULL );		/* lpdx */
			dxp = (int)GetTextExtent( hdc, (LPSTR) rgch, 3 );
			}
		}
	else
		{
		/* last line. */
		if (vfli.fSplatSub)
			{
			struct RC rcT;
			int	  idc;

		/* Because GrayString munges this dc. */
			if ((idc = SaveDC(hdc)) == 0)
				goto LRet;

			SetRect((LPRECT) &rc, xp + vfli.xpSub,
					ptText.yp,
					xp + vfli.xpRight - vfli.xpLeft,
					ptText.yp + vsci.dypBorder * 2);
			if (rc.xpRight - rc.xpLeft < vfti.dxpInch)
				{
				rc.xpRight = rc.xpLeft + vfti.dxpInch;
				}
			bltbyte(&rc, &rcT, sizeof(struct RC));
		/* Bring the rectangle to (0, 0) origin, so that the
		GrayString can deal with it. */
			OffsetRect((LPRECT) &rcT, -rcT.xpLeft, -rcT.ypTop);

			dxp = max(0,rc.xpRight - ptText.xp);
			GrayString(hdc, vsci.hbrText, lpFBltOutlineSplat,
					(LPSTR)((LPRECT) &rcT), -1,
					rc.xpLeft, rc.ypTop,
					rc.xpRight - rc.xpLeft, rc.ypBottom - rc.ypTop);
			RestoreDC(hdc, idc);
			}
		}
LRet:
	return dxp;
}


/* %%Function:FBltOutlineSplat %%Owner:davidlu */
EXPORT BOOL FAR PASCAL FBltOutlineSplat(hdc, lprc, c)
HDC	hdc;
LPRECT	lprc;
int	c;
{
	FillRect(hdc, lprc, GetStockObject(BLACK_BRUSH));
	return fTrue;
}


/* D R A W  O T L  M O V E  M A R K E R */
/* draws the outline move marker at the xp yp positon in xor mode */
/* %%Function:DrawOtlMoveMarker %%Owner:davidlu */
DrawOtlMoveMarker(xp, yp, dyp, mkt)
{
	int xw, yw;
	int xwT, ywT;
	int idcb;
	HDC hdc = PwwdWw(wwCur)->hdc;
	int iLevel;
	struct RC rc, rcDraw;

	if (mkt == mktOff || (iLevel = SaveDC(hdc)) == 0)
		return;

	xw = XwFromXp(hwwdCur, 0, xp);
	yw = YwFromYp(hwwdCur, 0, yp);

	switch (mkt)
		{
	case mktBody:
	case mktLeftRight:
		OtlMarkPos(xw, yw, dyp, &xwT, &ywT);
		DrawOtlPat(wwCur, hdc, idcbOtlMarkLeftRight, xwT, ywT);

		SetRect(&rcDraw, xw - vsci.dxpBorder,
				(*hwwdCur)->ywMin - vsci.dypMinWwInit,
				xw, (*hwwdCur)->ywMac);
		SelectObject(hdc, vhbrGray);
		PatBltRc(hdc, &rcDraw, PATINVERT);

		ywT += vfti.dypInch / rzOtlPat / 2;
		SetRect(&rcDraw, xwT + vfti.dxpInch / rzOtlPat + 1,
				ywT - vsci.dypBorder,
				xw - vsci.dxpBorder, ywT);
		PatBltRc(hdc, &rcDraw, PATINVERT);
		break;
	case mktUpDown:
		DrawOtlPat(wwCur, hdc, idcbOtlMarkUpDown,
				xw - (vfti.dxpInch / rzOtlPat),
				yw - (vfti.dypInch / rzOtlPat / 2));

		SetRect(&rcDraw, (*hwwdCur)->xwMin, yw,
				(*hwwdCur)->xwMac, yw + vsci.dypBorder);
		SelectObject(hdc, vhbrGray);
		PatBltRc(hdc, &rcDraw, 	PATINVERT);
		break;
	case mktHighlight + omkPlus:
		idcb = idcbOtlMarkHiPlus;
		goto LHi;
	case mktHighlight + omkMinus:
		idcb = idcbOtlMarkHiMinus;
		goto LHi;
	case mktHighlight + omkBody:
		idcb = idcbOtlMarkHiBody;
LHi:
		OtlMarkPos(xw, yw, dyp, &xwT, &ywT);
		SetRect(&rc, xwT, ywT,
				xwT + (vfti.dxpInch / rzOtlPat),
				ywT + (vfti.dypInch / rzOtlPat));
		EraseRc(wwCur, &rc);
		DrawOtlPat(wwCur, hdc, idcb, xwT, ywT);
		break;
		}
	RestoreDC(hdc, iLevel);
}


/* O T L  M A R K  P O S */
/* defines position of outline mark relative to the llc of edl */
/* %%Function:OtlMarkPos %%Owner:davidlu */
EXPORT OtlMarkPos(xw, yw, dyp, pxw, pyw)
int xw, yw, dyp, *pxw, *pyw;
{
	*pxw = xw - NMultDiv(dxaOtlMark, vfli.dxsInch, dxaInch);
	*pyw = yw + (dyp / 2) - (vfti.dypInch / rzOtlPat / 2);
}


/* A N I M A T E  O T L */
/* animate outline expansion or collapse. Rectangles are returned for
actual animation by AnimateLocalRect */
/* %%Function:AnimateOtl %%Owner:davidlu */
AnimateOtl(fExpand, doc, ipad, prc1, prc2)
struct RC *prc1, *prc2;
{
	int xp, yp;
	int cPad;
	int ipadT, ipadMac;
	int lvl;
	int dl;
	struct PLC **hplcpad = PdodDoc(selCur.doc)->hplcpad;
	struct PLDR **hpldr;
	int idr;
	struct EDL edl;
	struct PAD pad;
	struct RC rcSmall, rcLarge;

/* ipad points to cpFirst. We are collapsing/expanding starting at ipad + 1.
*/
	GetPlc(hplcpad, ipad + 1, &pad);
	ClearCap();
	xp = XpFromLvl(pad.fBody ? LvlFromIpad(doc, ipad + 1, fTrue) :
			pad.lvl, pad.fBody);
/* calculate estimate cPad of pad's expanded or collapsed */
	ipadMac = IpadMac(hplcpad);
	GetPlc(hplcpad, ipad, &pad);
	lvl = pad.lvl;
	for (ipadT = ipad + 1, cPad = 0;
			ipadT < ipadMac && cPad <= 20; ipadT++, cPad++)
		{
		GetPlc(hplcpad, ipadT, &pad);
		if (fExpand)
			{
			if (pad.fShow) break;
			}
		else
			{
			if (pad.lvl <= lvl) break;
			}
		}
/* calculate yp */
	if ((dl = DlWhereDocCp(wwCur, doc, CpPlc(hplcpad, ipad), fFalse,
			&hpldr, &idr, NULL, NULL,fTrue)) == dlNil)
		return;
	GetPlc(PdrGalley(*hwwdCur)->hplcedl, dl, &edl);
	yp = edl.ypTop + edl.dyp;

/* now we have xp, yp, cPad */
	SetRect(&rcSmall, xp, yp, xp + 40, yp + 1);
	SetRect(&rcLarge, xp, yp,
			DxpFromDxa(*hwwdCur, 6 * dxaInch),
			yp + cPad * 15);
	RcpToRcw(hwwdCur, 0, &rcSmall, &rcSmall);
	RcpToRcw(hwwdCur, 0, &rcLarge, &rcLarge);

	if (fExpand)
		{
		struct RC *prcT = prc1;
		prc1 = prc2;
		prc2 = prcT;
		}
	*prc2 = rcSmall;
	*prc1 = rcLarge;
}




/*-----------------------------------------------------------------------*/
/* Beginning of outline code not so shared with MacWord.                 */
/*-----------------------------------------------------------------------*/

/* T O G G L E  O U T L I N E  L V L */
/* Toggle the level icons from the old level to the new one. */
/* %%Function:ToggleOutlineLvl %%Owner:davidlu */
ToggleOutlineLvl(hwndIconBar, lvlOld, lvlNew)
HWND	hwndIconBar;
int	lvlOld, lvlNew;
{
#ifdef WIN23
	if (vsci.fWin3Visuals)
		SelectRadioIibb(hwndIconBar, IDOIBLVL1, IDOIBALL,
			lvlNew == lvlNone ? iibbNil : IDOIBLVL1 + lvlNew, fFalse);
	else
		{
#endif /* WIN23 */
	if (lvlOld != lvlNone)
		{
		SetValIibbHwndIb(hwndIconBar, IDOIBLVL1 + lvlOld, 0);
		}
	if (lvlNew != lvlNone)
		{
		SetValIibbHwndIb(hwndIconBar, IDOIBLVL1 + lvlNew, 1);
		}
#ifdef WIN23
		}	
#endif /* WIN23 */
}




/* U P D A T E  O U T L I N E  I C O N  B A R */
/* Hilight the toggle corresponding to the last "Show to level" command. */
/* %%Function:UpdateOutlineIconBar %%Owner:davidlu */
UpdateOutlineIconBar()
{
	int	lvlOld, lvlNew;
	int	ww;

	lvlOld = (*hmwdCur)->lvlOutline;
	lvlNew = PdodDoc((*hmwdCur)->doc)->lvl;

	if (lvlOld != lvlNew)
		{
		(*hmwdCur)->lvlOutline = lvlNew;
		ww = (*hmwdCur)->wwUpper;
		if (PwwdWw(ww)->hwndIconBar == NULL)
			{
			ww = (*hmwdCur)->wwLower;
			}
		Assert(PwwdWw(ww)->hwndIconBar != NULL);
		ToggleOutlineLvl(PwwdWw(ww)->hwndIconBar, lvlOld, lvlNew);
		}
}


#ifdef SHOWLEVEL
/* Updates show-level toggles in the outline iconbar to indicate
	the level of heading at the current selection. */
/* %%Function:UpdateOutlineIconBar %%Owner:NOTUSED */
UpdateOutlineIconBar()
{
	int ipadFirst, ipadLim, ipadCur;
	struct PLC **hplcpad;
	struct CA   ca;
	int         lvl2;
	BOOL        fAllSameLvl, fLvlValid;
	struct PAD  pad;
	HWND        hwndIconBar;
	int		mw;
	struct MWD	**hmwd;
	int		wwOtherCur = WwOther(wwCur);

	Assert((*hwwdCur)->fOutline);

	ca = selCur.ca;
	ExpandOutlineCa(&ca, fFalse);
	hplcpad = PdodDoc(ca.doc)->hplcpad;
	ipadFirst = IInPlc(hplcpad, ca.cpFirst);
	ipadLim = IInPlc2(hplcpad, ca.cpLim, ipadFirst);

	fLvlValid = fFalse;
	fAllSameLvl = fTrue;

	for (ipadCur = ipadFirst; ipadCur < ipadLim && fAllSameLvl; ipadCur++)
		{
		GetPlc(hplcpad, ipadCur, &pad);
		if (pad.fShow)
			{
			if (fLvlValid)
				{
				fAllSameLvl = Lvl2FromIpad(ca.doc, ipadCur, fFalse) == lvl2;
				}
			else
				{
				lvl2 = Lvl2FromIpad(ca.doc, ipadCur, fFalse);
				fLvlValid = fTrue;
				}
			}
		}

	hwndIconBar = ((*hwwdCur)->hwndIconBar ? (*hwwdCur)->hwndIconBar :
			PwwdWw(wwOtherCur)->hwndIconBar);
	mw = (*hwwdCur)->mw;
	hmwd = mpmwhmwd[mw];

	Assert(mw == 
			((*hwwdCur)->hwndIconBar ? (*hwwdCur)->mw :
			PwwdWw(wwOtherCur)->mw));

	if (hwndIconBar == NULL)
		{
		/* Wimp out. */
		return;
		}
	if (fLvlValid && fAllSameLvl)
		{
		int lvl;
		BOOL fBody;

		/* The following line is machine dependent. */
		fBody = (lvl2 & 0x1) != 0;
		lvl = lvl2 / 2;
		/* Hilight the lvl-th toggle. */
		if (fBody || lvl != (*hmwd)->lvlOutline)
			{
			if ((*hmwd)->lvlOutline != lvlNone)
				{
				SetValIibbHwndIb(hwndIconBar, IDOIBLVL1 + (*hmwd)->lvlOutline, 0);
				}
			if (!fBody)
				{
				SetValIibbHwndIb(hwndIconBar, IDOIBLVL1 + lvl, 0);
				(*hmwd)->lvlOutline = lvl;
				}
			else
				{
				(*hmwd)->lvlOutline = lvlNone;
				}
			}

		}
	else
		{
		/* No toggles are to be hilighted. */
		if ((*hmwd)->lvlOutline != lvlNone)
			{
			SendMessage((*hrghwndKids)[IDOIBLVL1 + (*hmwd)->lvlOutline],
					BM_SETCHECK, TSCHK_OFF, 0L);
			}
		(*hmwd)->lvlOutline = lvlNone;
		}
}


#endif


#ifdef ESFICON  /* NOTE: unused! Out of date WRT iconbars. */
/* %%Function:InitESF %%Owner:NOTUSED */
InitESF()
{
	BOOL      fOn;
	HWND    hwndIconBar;
	int	wwOtherCur = WwOther(wwCur);

	hwndIconBar = ((*hwwdCur)->hwndIconBar ? (*hwwdCur)->hwndIconBar :
			PwwdWw(wwOtherCur)->hwndIconBar);
	hrghwndKids = (HWND **) GetWindowWord(hwndIconBar, IDWICONBARHKIDS);

	fOn = (*hwwdCur)->fEllip = vpref.fEllip;
	SendMessage((*hrghwndKids)[IDOIBELLIP], BM_SETCHECK,
			(fOn ? TSCHK_ON : TSCHK_OFF), 0L);


	fOn = (*hwwdCur)->fShowF = vpref.fShowF;
	SendMessage((*hrghwndKids)[IDOIBSHOWF], BM_SETCHECK,
			(fOn ? TSCHK_ON : TSCHK_OFF), 0L);
}


#endif /* ESFICON */

#ifdef ESFICON
/* %%Function:ToggleESF %%Owner:NOTUSED */
ToggleESF(ww, fTgEllip, fTgShowF)
int ww;
BOOL fTgEllip, fTgShowF;
{
	struct WWD **hwwd = mpwwhwwd[ww];
	HWND **hrghwndKids;
	HWND    hwndIconBar;
	int	wwOtherCur = WwOther(wwCur);

	Assert(hwwd != hNil);

	hwndIconBar = ((*hwwd)->hwndIconBar ? (*hwwd)->hwndIconBar :
			PwwdWw(wwOtherCur)->hwndIconBar);
	hrghwndKids = (HWND **) GetWindowWord(hwndIconBar, IDWICONBARHKIDS);

	if (fTgEllip)
		{
		SendMessage((*hrghwndKids)[IDOIBELLIP], BM_SETCHECK,
				((*hwwd)->fEllip ? TSCHK_OFF : TSCHK_ON), 0L);
		vpref.fEllip = ((*hwwd)->fEllip = !(*hwwd)->fEllip);
		}

	if (fTgShowF)
		{
		SendMessage((*hrghwndKids)[IDOIBSHOWF], BM_SETCHECK,
				((*hwwd)->fShowF ? TSCHK_OFF : TSCHK_ON), 0L);
		vpref.fShowF = ((*hwwd)->fShowF = !(*hwwd)->fShowF);
		}

	TrashWw(ww);
}


#else
/* %%Function:ToggleESF %%Owner:davidlu */
ToggleESF(ww)
int ww;
{
	struct WWD **hwwd = mpwwhwwd[ww];
	vpref.fEllip = ((*hwwd)->fEllip = !(*hwwd)->fEllip);
	TrashWw(ww);
}


#endif


/* %%Function:Lvl2FromIpad %%Owner:davidlu */
int Lvl2FromIpad(doc, ipad, fInsert)
int doc;
int ipad;
BOOL fInsert;
{
	int		lvl2;
	struct PLC **hplcpad;
	struct PAD	pad;

	lvl2 = 2 * LvlFromIpad(doc, ipad, fInsert);

	hplcpad = PdodDoc(doc)->hplcpad;
	GetPlc(hplcpad, ipad, &pad);
	if (pad.fBody)
		{
		lvl2--;
		}
	return lvl2;
}


/* %%Function:Lvl2FromDocCp %%Owner:davidlu */
int Lvl2FromDocCp(doc, cp)
int	doc;
CP	cp;
{
	return (Lvl2FromIpad(doc, IInPlc(PdodDoc(doc)->hplcpad, cp), fFalse));
}



/* %%Function:DrawOtlPat %%Owner:davidlu */
EXPORT DrawOtlPat(ww, hdc, idcbOtlMark, xwT, ywT)
int ww;
HDC hdc;
int idcbOtlMark;
int xwT, ywT;
{
	struct MDCD	*pmdcd;
	long		rgbTextSave, rgbBkgrndSave;
#ifdef WIN23
	int sbmOld;
#endif /* WIN23 */

/* don't draw the mark if the left edge would not be visible; this prevents
   drawing it in the sel bar when scrolled */
#ifdef WIN23
	if ((pmdcd = PmdcdCacheIdrb(vsci.fWin3Visuals ? idrbOtlPat3 : idrbOtlPat2, hdc)) == NULL
		|| xwT < PwwdWw(ww)->xwMin)
#else
	if ((pmdcd = PmdcdCacheIdrb(idrbOtlPat, hdc)) == NULL || xwT < PwwdWw(ww)->xwMin)
#endif /* WIN23 */
		return;

	rgbTextSave = SetTextColor(hdc, 0x00ffffffL);
	rgbBkgrndSave = SetBkColor(hdc, 0x00000000L);

#ifdef WIN23
	/*****
	The otlpat bmp is actually a two color dib. Unfortunately, due
	to differences between the default PM palette and the default WIN3
	palette, 0 is white and 1 is black! In order to preserve the black (1)
	pixels in the bitmap, we need to set WHITEONBLACK (1 on 0)
	*****/
	if (vsci.fWin3Visuals)
		sbmOld = SetStretchBltMode(hdc, WHITEONBLACK);
	StretchBlt(hdc, xwT, ywT,
			vfti.dxpInch / rzOtlPat, vfti.dypInch / rzOtlPat,
			pmdcd->hdc,
			idcbOtlMark * dxpOtlPat, 0, dxpOtlPat, vsci.dypOtlPat,
			vsci.fMonochrome ? ropMonoBm : SRCINVERT);
	if (vsci.fWin3Visuals)
		SetStretchBltMode(hdc, sbmOld);
#else
	StretchBlt(hdc, xwT, ywT,
			vfti.dxpInch / rzOtlPat, vfti.dypInch / rzOtlPat,
			pmdcd->hdc,
			idcbOtlMark * dxpOtlPat, 0, dxpOtlPat, dypOtlPat,
			vsci.fMonochrome ? ropMonoBm : SRCINVERT);
#endif /* WIN23 */

	SetBkColor(hdc, rgbBkgrndSave);
	SetTextColor(hdc, rgbTextSave);
}



/* O U T L I N E  P R O P S */
/* modifies effective indents and space before in FormatLine according to the
outlining rules:
	6" right margin
	lvl * standard tab width as left margin
	2 points of extra lead before "headings" (non-body text)
caPara is set up with paragraph in question.
*/
/* %%Function:OutlineProps %%Owner:davidlu */
EXPORT OutlineProps(pxaLeft, pxaRight, pdypBefore)
int *pxaLeft, *pxaRight, *pdypBefore;
{
	struct PLCPAD **hplcpad = PdodDoc(caPara.doc)->hplcpad;
	uns dxaTab = DxaTabOutline(caPara.doc);
	int ipad;
	int dxaRight;
	struct PAD pad;
	CP cpFirstPara = caPara.cpFirst;
	BOOL fFirstLine;

	if (vpapFetch.fInTable)
		{
		extern CP vcpFirstTablePara;
		FInTableDocCp(caPara.doc, caPara.cpFirst);
		cpFirstPara = vcpFirstTablePara;
		}

	fFirstLine = (vfli.cpMin == cpFirstPara);

	ipad = IInPlcCheck(hplcpad, caPara.cpFirst);
	if (ipad < 0)
		/* special case: doc starts with a non-table portion of a table 
			(treat it as body text) */
		{
		*pxaLeft = dxaTab + dxaOtlMark;
		pad.lvl = lvlMax;
		pad.fBody = fTrue;
		Assert(!fFirstLine);
		}
	else
		{
		GetPlc(hplcpad, ipad, &pad);
		*pxaLeft = LvlFromIpad(caPara.doc, ipad, fFalse) * dxaTab + dxaOtlMark;
		}
	*pdypBefore = 0;
	dxaRight = 0;
	/* vfli.omk = omkNil;  IE*/
	if (pad.fBody)
		{
/* in body text, leave room for ellipses */
#ifdef MAC
		if (!vpref.fOtlShowBody)
			{
			dxaRight = dyaPoint/*sic.*/ * 9;
			vfli.fSplatDots = fTrue;
			}
#else /* WIN */
		if ((*mpwwhwwd[vfli.ww])->fEllip)
			{
			dxaRight = NMultDiv(3 * DxpFromCh('.', &vfti),
					dxaInch, vfli.dxsInch);
			vfli.fSplatDots = fTrue;
			}
#endif
		*pxaLeft -= dxaTab / 2;
		if (fFirstLine)
			vfli.omk = omkBody;
		}
	else  if (fFirstLine)
		vfli.omk = omkMinus;
/* determine if there follows invisible text, and sub-text */
	if (++ipad < IpadMac(hplcpad))
		{
		int lvl = pad.lvl;
		GetPlc(hplcpad, ipad, &pad);
		if (vfli.omk == omkMinus && pad.lvl > lvl)
			vfli.omk = omkPlus;
		if (!pad.fShow)
			{
			vfli.fSplatSub = fTrue;
			vfli.xpSub = NMultDiv(
					LvlFromIpad(caPara.doc, ipad, fFalse) * dxaTab + dxaOtlMark
					- (pad.fBody ? dxaTab >> 1 : 0),
					vfli.dxsInch, dxaInch);
			}
		}
/* room for 10 columns plus + 1 inch or 6" */
	*pxaRight = umax((lvlMax * dxaTab) + dxaInch, 6 * dxaInch) - dxaRight;
/* don't let it overflow the right margin unless it has to */
	if (*pxaRight > vfli.dxa && *pxaLeft < vfli.dxa - dxaInch)
		*pxaRight = vfli.dxa - dxaRight;
}


/* O U T L I N E  C H P */
/* changes vchpFetch to what is required by outline */
/* %%Function:OutlineChp %%Owner:davidlu */
NATIVE OutlineChp(doc, pchp)
int doc;
struct CHP *pchp;
{
	int fSpec;
	FC fcPic;
	int fFldVanish; 
	struct PAP papT;

	if (!WinMac((PwwdWw(vfli.ww))->fShowF, vpref.fOtlShowFormat))
		{
		fSpec = pchp->fSpec;
		fFldVanish = pchp->fFldVanish; 
		fcPic = pchp->fcPic;
		MapStc(PdodDoc(doc), stcNormal, pchp, &papT);
		pchp->fSpec = fSpec;
		pchp->fFldVanish = fFldVanish; 
		pchp->fcPic = fcPic;
		fcmFetch = -1; /* invalidate fetch cache */
		}
}


/* C P  O U T L I N E  A D V A N C E */
/* advances to least cp' >= cp that points to the start of a fShow
paragraph or accepts cp pointing in the middle of a !fBody para.
*/
/* %%Function:CpOutlineAdvance %%Owner:davidlu */
EXPORT CP CpOutlineAdvance(doc, cp)
int doc; 
CP cp;
{
	struct PLCPAD **hplcpad = PdodDoc(doc)->hplcpad;
	int ipad;
	int ipadMac;
	struct PAD pad;

	if (hplcpad == hNil)
		return cp; 
	ipad = IInPlc(hplcpad, cp);
	ipadMac = IpadMac(hplcpad);

	if (ipad >= ipadMac)
		 return cp;
	GetPlc(hplcpad, ipad, &pad);
	if (pad.fShow && (!pad.fBody || 
			WinMac(!(*hwwdCur)->fEllip, vpref.fOtlShowBody) ||
			cp == CpPlc(hplcpad, ipad) || pad.fInTable))
		return cp;
/* entry at ipad is not good for some reason. find start of next fShow entry */
	while (++ipad < ipadMac)
		{
		GetPlc(hplcpad, ipad, &pad);
		if (pad.fShow)
			break;
		}
	return CpPlc(hplcpad, ipad);
}


/* marker state structure */
struct MKS
	{
	int     mkt;
	int     xp;
	int     yp;
	int	dyp;
};

/* F  O U T L I N E  S E L E C T */
/* outline mode portion of DoContentHit. Returns true iff click was
treated here, otherwise normal click - should be treated by caller.
*/
/* %%Function:FOutlineSelect %%Owner:davidlu */
FOutlineSelect(pt, fShift, fOption, fCommand)
struct PT pt;
{

	int xp;
	int dl;
	int lvl;
	int spt;
	int ypTo;
	int ipad, ipadFirst, ipadLim;
	int lvlLead, lvlTo;
	struct PLCEDL **hplcedl;
	struct PLC **hplcpad;
	struct PLDR **hpldr;
	int idr;
	int stcLead;
	BOOL fMoved, fUpDown;
	int dxw, dyw, dxpOffset;
	int omk;
	int lvlLast, lvlSel;
	int fTableSel;
	BOOL fInTable;
	struct RC rc1, rc2;

	CP cpSel, cpTo, cpLead;
	struct PT ptT, ptFirst;
	struct EDL edl;
	struct CA caOp;
	struct PAD pad;
	struct MKS mks, mksHighlight;
	int dysMinScroll, dysMaxScroll;
#ifdef 	WIN
	struct WWD *pwwd;
	int dywPane;
	BOOL fForward = fFalse;
#endif

	ptT = pt;
	if ((dl = DlWherePt(wwCur, &ptT, &hpldr, &idr, &spt, fFalse, fFalse)) == dlNil)
		return fTrue;
	hplcedl = PdrGalley(*hwwdCur)->hplcedl;
	if (!FUpdateHplcpad(selCur.doc))
		return fTrue;
	hplcpad = PdodDoc(selCur.doc)->hplcpad;

	if (!FXwInBullet(wwCur, dl, pt.xw))
		return(fFalse);


	cpSel = CpPlc(hplcedl, dl);

	PcaPoint(&caOp, selCur.doc, cpSel);
	ipad = IInPlc(hplcpad, cpSel);
/* clicked down on the outline mark */
	if (vfDoubleClick)
		{
		if (ipad >= IpadMac(hplcpad) - 1)
			{
LBeep:
			Beep();
			return fTrue;
			}
		
/* there is text following */
#ifdef WIN
		/* Tell CBT that user is expanding/collapsing text*/
		if (vhwndCBT)
			if (!SendMessage(vhwndCBT, WM_CBTSEMEV, 
					smvOutlineExpand, MAKELONG((int)cpSel, 0)))
				{
				goto LInval;
				}
#endif /* WIN */
		GetPlc(hplcpad, ipad, &pad);
		lvl = pad.lvl;
		GetPlc(hplcpad, ipad + 1, &pad);
		if (lvl >= pad.lvl && pad.fShow)
			goto LBeep; 
		
				
/* there is sub-text following */
/* do animation in two steps, so that drawing immediately precedes UpdateWw */
		AnimateOtl(!pad.fShow, caOp.doc, ipad, &rc1, &rc2);
		if (pad.fShow)
/* there is visible sub-text: collapse it */
			{
			cpLead = CpPlc(hplcpad, ipad + 1);
			CmdOutlnUcm1(ucmOtlMinus,
					&caOp, styPara, 0);
			}
		else
/* there is invisible sub-text: expand it */
			{
			CmdOutlnUcm1(ucmOtlPlus,
					&caOp, styPara, 0);
			cpLead = caOp.cpLim;
			}
		AnimateLocalRect(wwCur,	&rc1, &rc2);
			
		if (!FStillDownReplay(&pt, fFalse))
			{
			UpdateWw(wwCur, fFalse);
			return fTrue;
			}
		UpdateWw(wwCur, fFalse);
		}
/* detemine operand for the operation (caOp is set up to point to cpSel) */
	if (fShift || fOption)
		{
		caOp = selCur.ca;
		if (fShift)
			{
			if (cpSel < selCur.cpFirst)
				caOp.cpFirst = cpSel;
			else
				{
				caOp.cpLim = CpPlc(hplcpad, ipad + 1);
				goto LLim;
				}
			}
		else  /*if (fOption)*/			
			{
			if (cpSel < selCur.cpFirst || cpSel >= selCur.cpLim)
				{
				caOp.cpFirst = CpPlc(hplcpad, ipad);
				caOp.cpLim = CpPlc(hplcpad, ipad + 1);
				}
			}
		caOp.cpLim = CpLimSubText(&caOp, fFalse, fFalse);
		}
	else
/* select all sub-text for cp */
		{
LLim:           
		caOp.cpLim = CpLimSubText(&caOp, fTrue, fFalse);
		}
/* no we have: caOp, cpSel */

	fMoved = fFalse;
	ptFirst = pt;

/* calculate lvlLast in selection - for later checking for acceptable level
shift.
Also obtain, lvlLead and stcLead for the leftmost (leader) level.
lvlSel - the level at the line of the selection.
*/
	ipadFirst = IInPlc(hplcpad, caOp.cpFirst);
	GetPlc(hplcpad, ipadFirst, &pad);
	fInTable = pad.fInTable;
	lvlSel = lvlLead = pad.lvl;
	stcLead = pad.stc;
	ipadLim = IInPlc2(hplcpad, caOp.cpLim, ipadFirst);
	lvlLast = 0;
	cpLead = caOp.cpFirst;
	for (ipad = ipadFirst; ipad < ipadLim; ipad++)
		{
		GetPlc(hplcpad, ipad, &pad);
		if (pad.lvl < lvlLead)
			{
			lvlLead = pad.lvl;
			stcLead = pad.stc;
			cpLead = CpPlc(hplcpad, ipad);
			}
		if (CpPlc(hplcpad, ipad) == cpSel)
			lvlSel = pad.lvl;
		if (pad.lvl != lvlMax)
			lvlLast = max(lvlLast, pad.lvl);
		}
/* remember omk for highlight refresh */
	FormatLine(wwCur, caOp.doc, cpLead);
	omk = vfli.omk;
/* calculate offset for horizontal movement */
	dxpOffset = 0;
	if (lvlLead != lvlSel)
/* know: Lead < Sel, hence Lead != lvlMax */
		dxpOffset = XpFromLvl(lvlLead, fFalse) -
				(lvlSel == lvlMax ? XpFromXw(hwwdCur, 0, pt.xw) :
				XpFromLvl(lvlSel, fFalse));

/* initialize marker state */
	mks.mkt = mktOff;
	mksHighlight.mkt = mktOff;

	ClearCap(); 

	fTableSel = fFalse;
	if (FInTableDocCp(selCur.doc, caOp.cpFirst))
		fTableSel = FCaInTable(&caOp);

	if (fTableSel)
		SelectRow(&selCur, caOp.cpFirst, caOp.cpLim);
	else
		Select(&selCur, caOp.cpFirst, caOp.cpLim);
#ifdef WIN
	SetCapture((*hwwdCur)->hwnd);
	vcConseqScroll = 0;
	pwwd = *hwwdCur;
	hplcedl = pwwd->rgdr[idr].hplcedl;
	dywPane = pwwd->ywMac - pwwd->ywMin;
#else
	dysMinScroll = dysMinAveLineSci;
	dysMaxScroll = dysMacAveLineSci;
#endif
	while (FStillDownReplay(&pt, fFalse))
		{
/* states:
	for a short delay, nothing will happen unless mouse moves
	after delay, operand will be selected
	if mouse moves, direction is established and operand is selected
*/
		dxw = abs(pt.xw - ptFirst.xw);
		dyw = abs(pt.yw - ptFirst.yw);
		if (!fMoved && (dxw > 4 || dyw > 4))
			{
			fMoved = fTrue;
			SetCrs((fUpDown = dyw >= dxw) ? crsOtlVert : crsOtlHorz);
			}
		if (fMoved && fUpDown)
			{
			if (pt.yw >= (*hwwdCur)->ywMac-1)
				{
#ifdef 	WIN
				if (!fForward)
					{
					vcConseqScroll = 0;
					}
				fForward = fTrue;
				ScrollDelta(1, vcConseqScroll + 1,
						(*hplcedl)->dlMax,
						dywPane, &dysMinScroll, &dysMaxScroll);
#endif
				if (!FEndVisible(wwCur))
					{
					mksHighlight.mkt = mktOff;
					DrawMks(&mks, mktOff, 0, 0, 0);
					ScrollUp(wwCur, dysMinScroll,
							dysMaxScroll);
					}
				goto DoCont1;
				}
			else  if (pt.yw < (*hwwdCur)->ywMin)
				{
#ifdef WIN
				if (fForward)
					{
					vcConseqScroll = 0;
					}
				fForward = fFalse;
				ScrollDelta(1, vcConseqScroll + 1,
						(*hplcedl)->dlMax,
						dywPane, &dysMinScroll, &dysMaxScroll);
#endif
				mksHighlight.mkt = mktOff;
				DrawMks(&mks, mktOff, 0, 0, 0);
				ScrollDown(wwCur, dysMinScroll, dysMaxScroll);
DoCont1:
#ifdef WIN
				if (vcConseqScroll < 0x7FFF)
					{
					vcConseqScroll++;
					}
#endif
				UpdateWw(wwCur, fFalse);
				}
			}
/* turn on highlight at line cpLead */
		if (mksHighlight.mkt == mktOff &&
				(dl = DlWhereDocCp(wwCur, caOp.doc, cpLead, fFalse,
				&hpldr, &idr, NULL, NULL, fFalse)) != dlNil)
			{
			GetPlc(hplcedl, dl, &edl);
			DrawMks(&mksHighlight, mktHighlight + omk,
					edl.xpLeft, edl.ypTop, edl.dyp);
			}
		ptT.xw = pt.xw;
		if (fMoved)
			{
			if (pt.xw < (*hwwdCur)->xwSelBar + dxwSelBarSci
					|| pt.xw >= (*hwwdCur)->xwMac) goto LNoMarker;
			if (fUpDown)
				{
				ptT.yw = min(pt.yw, (*hwwdCur)->ywMac);
				dl = DlWherePt(wwCur, &ptT, &hpldr, &idr, &spt, fFalse, fFalse);
				if (dl != dlNil)
					{
					CP cpT, cpFirstLine;
					cpT = CpPlc(hplcedl, dl);
					cpFirstLine = cpT;
					GetPlc(hplcedl, dl, &edl);
					ypTo = edl.ypTop;
					if (cpT > caOp.cpFirst)
						{
						struct CA caT;
						cpFirstLine = CpPlc(hplcedl, dl + 1);
						cpT = CpLimSubText(
								PcaPoint(&caT, caOp.doc, cpT),
								fFalse, fFalse);
						ypTo += edl.dyp;
						}
/* check to avoid middle of multi-line heading, or overlapping move */
					ipad = IInPlc(hplcpad, cpFirstLine);
					if (CpPlc(hplcpad, ipad) != cpFirstLine) continue;
					if (cpT < caOp.cpFirst || cpT > caOp.cpLim)
						{
						cpTo = cpT;
/* rule for indent: if previous pad is body text, use its lvl, otherwise
its level + 1 */
						xp = lvlLead == lvlMax ?
								XpFromLvl(
								LvlFromIpad(caOp.doc,
								IInPlc(hplcpad, cpTo),
								fTrue), fTrue) :
								XpFromLvl(lvlLead, fFalse);
						DrawMks(&mks, mktUpDown, xp, ypTo, edl.dyp);
						}
					else
						goto LNoMarker;
					}
				}
			else  /* if fLeftRight */				
				{
				if (fInTable) goto LNoMarker;
				lvlTo = min(LvlFromXp(XpFromXw(hwwdCur, 0, ptT.xw + dxpOffset)),
						lvlMax);
				if (lvlTo != lvlLead && pt.yw >= (*hwwdCur)->ywMin &&
						pt.yw < (*hwwdCur)->ywMac &&
						(lvlTo == lvlMax || lvlTo + lvlLast - lvlLead < lvlMax))
					{
					xp = lvlTo == lvlMax ?
							XpFromLvl(
							LvlFromIpad(caOp.doc,
							ipadFirst,
							fTrue), fTrue) :
							XpFromLvl(lvlTo, fFalse);
/* draw box at leader line, if it exists */
					ypTo = mksHighlight.mkt == mktOff ?
							-100 : mksHighlight.yp;
					DrawMks(&mks, lvlTo != lvlMax ? mktLeftRight : mktBody,
							xp, ypTo, mksHighlight.dyp);
					DrawLlcStyle1(lvlTo == lvlMax ? stcNormal : stcLevLast - lvlTo);
					}
				else
					goto LNoMarker;
				}
			}
		else
			{
LNoMarker:
			DrawMks(&mks, mktOff, 0, 0, 0);
			DrawLlcStyle1(stcLead);
			}
/* at this point marker is up iff cpTo, lvlTo are legal */
		}
/* upclick of the mouse */
	Win( ReleaseCapture() );
	DrawMks(&mksHighlight, mktOff, 0, 0, 0);
	if (mks.mkt != mktOff)
		{
		DrawMks(&mks, mktOff, 0, 0, 0);

		if (!fUpDown)
			{
#ifdef WIN
			/* Tell CBT that we're promoting/demoting */
			if (vhwndCBT)
				if (!SendMessage (vhwndCBT, WM_CBTSEMEV, smvOutlinePromote, 
						MAKELONG((int)caOp.cpFirst,lvlTo)))
					{
					goto LInval;
					}
#endif
			if (lvlLead == lvlMax)
				{
				/*if (lvlTo != lvlMax)*/
				CmdOutlnUcm1(ucmOtlLvl, &caOp, 0,
						lvlTo);
				}
			else
				{
				if (lvlTo == lvlMax)
					CmdOutlnUcm1(ucmOtlBody, &caOp, 0, 0);
				else  /*if (lvlTo != lvlLead)*/
					CmdOutlnUcm1(ucmOtlVal, &caOp, 0,
							lvlTo - lvlLead);
				}
			}
		else 			
			{
#ifdef WIN
			/* Tell CBT that we're moving up/down */
			if (vhwndCBT)
				if (!SendMessage (vhwndCBT, WM_CBTSEMEV, smvOutlineMove, 
						MAKELONG((int)caOp.cpFirst,(int)cpTo)))
					{
					goto LInval;
					}
#endif /* WIN */
			if (CmdMoveUpDn1(&caOp, cpTo,
					WinMac(
					(caOp.cpFirst < cpTo) ? ucmOtlDown : ucmOtlUp,
					ucmNil)) == cmdError) goto LInval;
			}
		}
	else
		{
#ifdef WIN
		/* Advise CBT that we're selecting */
		if (vhwndCBT)
			SendMessage (vhwndCBT, WM_CBTSEMEV, smvOutlineSelect, 
					MakeLong((int)caOp.cpFirst, 0));
#endif /* WIN */
LInval:
/* this is to refresh the highlighted button in front of the first line */
		InvalCp(PcaSetDcp(&caOp, caOp.doc, cpLead, (CP)1));
		}
	return fTrue;
}


/* F  X W  I N  B U L L E T */
/* returns fTrue if pt specified is in the outline bullet on line dl */
/* %%Function:FXwInBullet %%Owner:davidlu */
FXwInBullet(ww, dl, xw)
int ww, dl, xw;
{
	struct DR *pdr = PdrGalley(PwwdWw(ww));
	struct PLC **hplcedl = pdr->hplcedl;
	int xwLeft, xwMark, ywMark;
	int doc = pdr->doc;
	CP cp, ccp;
	struct EDL edl;
	extern CP vcpFirstTablePara;

	Assert(hplcedl != hNil);        /* otherwise how did we get a dl? */
	cp = CpPlc(hplcedl, dl);

	if (!FInTableDocCp(doc, cp) && vpapFetch.fInTable)
		/* piece of paragraph before table, does not have mark */
		return fFalse;
#ifdef WIN
/* do not allow outline action by mouse if document is locked for annotation */
	if (PdodDoc(doc)->fLockForEdit)
		return fFalse;

	/* find first visible character in paragraph */
	FetchCpPccpVisible(doc, vcpFirstTablePara, &ccp, ww, 0);

	if (cp < vcpFirstTablePara || cp > vcpFetch)
		/*  not at beginning of paragraph */
		return fFalse;
#else
	if (cp != vcpFirstTablePara)
		return fFalse;
#endif /* WIN */

	GetPlc(hplcedl, dl, &edl);
	if (edl.hpldr != hNil)
		xwLeft = XwFromXp(edl.hpldr, 0, 0);
	else
		xwLeft = XwFromXp(HwwdWw(ww), 0, edl.xpLeft);
	OtlMarkPos(xwLeft, 0, 0, &xwMark, &ywMark);
	return(xw >= xwMark && xw < xwLeft - (dxwSelBarSci / 2));
}


/* D R A W  M K S */
/* draws marker in xor mode. state is maintained so that marker can
be removed */
/* %%Function:DrawMks %%Owner:davidlu */
DrawMks(pmks, mkt, xp, yp, dyp)
struct MKS *pmks;
{
	if (pmks->mkt == mkt &&
			(pmks->xp == xp && pmks->yp == yp))
		return;
/* turn off current mkt */
	DrawOtlMoveMarker(pmks->xp, pmks->yp, pmks->dyp, pmks->mkt);
/* now the mark is off */
	pmks->mkt = mkt;
	pmks->xp = xp;
	pmks->yp = yp;
	pmks->dyp = dyp;
/* draw the mark in xor mode at xp, yp */
	DrawOtlMoveMarker(xp, yp, dyp, mkt);
}


/* L V L  F R O M  X P */
/* defines xp -> lvl mapping of the outline icon bar, in wwCur (outline mode) */
/* %%Function:LvlFromXp %%Owner:davidlu */
LvlFromXp(xp)
{
	return (DxaFromDxp(*hwwdCur, max(0, xp)) / DxaTabOutline(selCur.doc));
}


/* D X A   T A B   O U T L I N E */
/* returns space between outline levels; dop.dxaTab restricted to reasonable
	values */
/* %%Function:DxaTabOutline %%Owner:davidlu */
int DxaTabOutline(doc)
int doc;
{
	int dxaTab = PdodDoc(doc)->dop.dxaTab;

	return(max(min(dxaTab, 2 * dxaInch), dxaInch >> 2));
}


/* X P  F R O M  L V L */
/* defines position of tick mark for lvl on the outline icon bar
in wwCur (outline mode) */
/* %%Function:XpFromLvl %%Owner:davidlu */
XpFromLvl(lvl, fBody)
{
	int dxaTab = DxaTabOutline(selCur.doc);
	int dxpOtlMk = NMultDiv(dxaOtlMark, vfli.dxsInch, dxaInch);

	return (DxpFromDxa(*hwwdCur, dxaOtlMark +
			dxaTab * lvl - (fBody ? dxaTab / 2 : 0)));
}


/* S Y N C H  O U T L I N E */
/* synch other pane with ww (master). Know: pwwd->fSplit, fOutline's are not equal.
if master is main text:
		if cpFirst of master not visible in outline: scroll outline to cpFirst
else master is outline text
		scroll other window to cpFirst of outline.
*/
/* %%Function:SynchOutline %%Owner:davidlu */
SynchOutline(ww)
int ww;
{
	struct WWD *pwwd = PwwdWw(ww);
	struct WWD *pwwdOther;
	struct DR *pdrWw, *pdrOther; 

	Assert(!pwwd->fPageView);
	pdrWw = PdrGalley(pwwd); 
	if (pwwd->fOutline ||
			(!FCpVisible(WwOther(ww), pdrWw->doc,
			pdrWw->cpFirst, fFalse, fFalse, fFalse)))
		{
		pwwdOther = PwwdOther(ww);
		pdrOther = PdrGalley(pwwdOther); 
		Assert(!pwwdOther->fPageView);
		pdrOther->cpFirst = pdrWw->cpFirst;
		pdrOther->dypAbove = 0;
		pdrOther->fCpBad = fTrue;
		pwwdOther->fDirty = fTrue;
		pwwdOther->fSetElev = fTrue;
		}
}


/* F  S H O W  O U T L I N E */
/* returns true iff doc, cp entry in hplcpad has fShow set */
/* %%Function:FShowOutline %%Owner:davidlu */
EXPORT FShowOutline(doc, cp)
CP cp;
{
	struct PLCPAD **hplcpad = PdodDoc(doc)->hplcpad;
	struct PAD pad;

	GetPlc(hplcpad, IInPlc(hplcpad, cp), &pad);
	return pad.fShow;

}


