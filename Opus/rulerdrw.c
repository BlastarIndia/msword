/*  rulerdrw.c  --  ruler drawing module
	This module contains the ruler draw code & windows procedure.
*/


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "ch.h"
#include "cmd.h"
#include "disp.h"
#include "prm.h"
#include "doc.h"
#include "sel.h"
#include "screen.h"
#include "wininfo.h"
#include "debug.h"
#include "error.h"
#include "format.h"
#include "style.h"
#include "resource.h"
#include "help.h"
#include "cmdlook.h"
#include "keys.h"
#include "iconbar.h"
#include "ibdefs.h"
#include "layout.h"
#define RULER
#define REPEAT
#include "ruler.h"


extern int    mwCur;
extern HWND   vhwndApp;
extern BOOL   vfRecording;
extern HANDLE vhInstance;
extern HMENU  vhMenu;
extern CHAR   szEmpty[];
extern char   (**vhmpiststcp)[];
extern int    vdocStsh;
extern int    wwCur;
extern int    wwMac;
extern struct MWD ** mpmwhmwd[];
extern struct CA     caPara;
extern struct UAB    vuab;
extern struct WWD    ** mpwwhwwd[];
extern int           wwCreating;
extern RRF	     rrf;
extern struct WWD ** hwwdCur;
extern struct MWD ** hmwdCur;
extern struct SEL    selCur;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
extern struct SCI    vsci;
extern struct RULSS  vrulss;
extern struct PREF   vpref;
extern struct SEP    vsepFetch;
extern struct FTI    vfti;
extern struct FLI    vfli;
#ifdef WIN23
extern struct BMI    *mpidrbbmi;
#else
extern struct BMI    mpidrbbmi[];
#endif /* WIN23 */
extern struct TAP    vtapFetch;
extern BOOL    vfInitializingRuler;
extern CHAR    szEmpty[];
extern HCURSOR vhcArrow;
extern struct CA    caTap;
extern BOOL         vfSeeSel;

extern struct REB * PrebInitPrebHrsd ();
extern int ParseStyle();
InvalStyle();

extern BOOL         vfHelp;
extern HWND         vhwndCBT;

extern int     ypScaleBottom;
extern int     ypMarkBottom;
extern int     dypMark;
extern int     dypMajorTick;
extern int     dypMinorTick;
extern int     dypTinyTick;
extern struct MERR vmerr;
#ifdef WIN23
extern HBRUSH vhbrLtGray;
extern HBRUSH vhbrGray;
extern HBRUSH vhbrWhite;
extern HBRUSH vhbrBlack;
extern HBRUSH vhbrDkGray;
extern HFONT hfontStatLine;
extern int              vdbmgDevice;
#endif /* WIN23 */

extern struct REB * vpreb;


/* scale display infromation */
/*   domain:  utInch, utPt, utCm, utPica */

/* distance between major ticks */
csconst int mputdxa [] = {  
	czaInch, czaPoint * 50, czaCm, czaInch  };


/* major tick label increments */
csconst int mputdn [] = {  
	1, 50, 1, 1  };


/* minor ticks per major increment (plus one) */
csconst int mputn1 [] = {  
	2, 1, 2, 1  };


/* tiny ticks per major increment (plus one) */
csconst int mputn2 [] = {  
	8, 10, 10, 6  };
#ifdef WIN23
csconst int mputn23 [] = {  
	8, 10, 4, 6  };
#endif /* WIN23 */

/* distance apart tabs are when they are merged */
csconst int mputdxaQuantum [] = 
{ 
	czaInch/16, czaPoint*5, 142/*czaCm/4*/, czaPicas/2 };


#define ddxaCmErrorQuantum 567

/*  1/2 width of cursor in RulerEditMode, distance considered "close"  */
csconst int mputdxaCursorDiv2 [] =
{ 
	czaInch/16, czaPoint*5, 142/*czaCm/4*/, czaPicas/2 };



/* I B  P R O C  R U L E R */
/* %%Function:IbProcRuler  %%Owner:peterj */
IbProcRuler(hwndRuler, ibm, idRul, wParam, lParam)
HWND hwndRuler;
WORD ibm;
WORD idRul;
WORD wParam;
LONG lParam;
{
	struct RSD **hrsd = HrsdFromHwndRuler(hwndRuler);

	switch (ibm)
		{
	case ibmInit:
	case ibmTerm:
	case ibmCancel:
	case ibmDblClk:
		IbProcRulerCmd(hwndRuler, hrsd, ibm, idRul, wParam, lParam);
		break;

	case ibmMoveSize:
		/* position the "mark" window based on the parent width */
		PositionMarkWindow(hwndRuler);
		break;

	case ibmCommand:
		DoRulerCommand(hrsd, idRul);
		break;

	case ibmDestroy:
		if (hrsd)
			{
			AssertH(hrsd);
			FreeH(hrsd);
			}
		break;
		}
}


/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:RulerMarkWndProc   %%Owner:peterj */
EXPORT LONG FAR PASCAL RulerMarkWndProc (hwndMark, wMessage, wParam, lParam)
HWND hwndMark;
unsigned wMessage;
WORD wParam;
LONG lParam;

{
	switch (wMessage)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		return (LONG) DefWindowProc (hwndMark, wMessage, wParam, lParam);
#endif /* DEBUG */

	case WM_LBUTTONDBLCLK:
		DoRulerDblClick(MAKEPOINT(lParam), fTrue);
		break;

	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwndMark, 0L);
		break;

	case WM_PAINT:
			{
			struct RSD ** hrsd = HrsdFromHwndRuler(GetParent(hwndMark));
			AssertH(hrsd);
			PaintRuler (hrsd, rpcPaintMsg);
			break;
			}

	case WM_LBUTTONDOWN:
			{
			struct PT pt;
			struct RSD ** hrsd = HrsdFromHwndRuler(GetParent(hwndMark));
			AssertH(hrsd);
			if (vfHelp)
				{
				GetHelp(cxtRulerIconBar);
				break;
				}
			if (vmerr.hrgwEmerg1 == hNil)
				{
				SetErrorMat(matMem);
				break;
				}
			/* terminate Block or Extend or dyadic move mode if on */
			SpecialSelModeEnd();
			Assert (vpreb == NULL);
			pt = MAKEPOINT (lParam);
			if (pt.yp >= ypMarkTop && pt.yp <= ypMarkBottom)
				/* point is in the mark portion of the ruler */
				DragMark (hrsd, pt);
			break;
			}
		}

	return fTrue;
}


/* P O S I T I O N  M A R K  W I N D O W */
/* %%Function:PositionMarkWindow  %%Owner:peterj */
PositionMarkWindow(hwndRuler)
HWND hwndRuler;
{
	struct RC rcParent, rcMark;
	HWND hwndMark = HwndCusFromHwndIibb(hwndRuler, idRulMark);
	int ww, mw;
	int xpLeft, xpRight;

	if (ypScaleBottom == 0)
		return;

	GetClientRect(hwndRuler, &rcParent);
	GetHwndParentRc(hwndMark, &rcMark);

	mw = MwFromHwndMw(GetParent(hwndRuler));
	ww = mw == mwCur ? wwCur : PmwdMw(mw)->wwActive;
	if (ww != wwNil)
		{
		GetXpsForHwwd(mpwwhwwd[ww], HrsdFromHwndRuler(hwndRuler),
				&xpLeft, NULL, &xpRight);
		xpRight = min(xpRight, rcParent.xpRight);
		}
	else
		{
		xpLeft = rcMark.xpLeft;
		xpRight = rcMark.xpRight;
		}

	MoveWindow(hwndMark, xpLeft, rcMark.ypTop,
			xpRight - xpLeft,
			ypMarkBottom, fTrue);
}



/* RULER UPDATE CODE */


/*  UpdateRuler
	insures that the ruler display correctly reflects the current selection.
*/

/* %%Function:UpdateRuler   %%Owner:peterj */
UpdateRuler (hmwd, fInit, rkNew, fAbortOK)
struct MWD ** hmwd;
BOOL fInit;
int rkNew;
BOOL fAbortOK;

{
	int xa;
	int rpc;
	int idr;
	BOOL fChangeAll = fFalse;
	HWND hwndRuler;
	struct RSD ** hrsd, *prsd;
	BOOL fDiffProp, fDiffGray;
	int itc;
	int xpLeft, xpZero, xpRight;
	int itbd, itbdMac;
	struct SEL *psel = &selCur;  /* intentionally not PselActive bz */
	struct PAP papDiffProp;
	struct PAP papDiffGray;
	extern int vssc;


	if (hmwd == hNil)
		{  /* no child => no ruler !! shouldn't have gotten here */
		Assert (fFalse);
		return;
		}

	/*  what to do if hmwd != hmwdCur ?? */
	/*  there is no current selection for any but the current mac window.
		it would be pretty easy to have all ruler up to date.  first you
		would need to be able to call update on all rulers. this can be done by
		going through the table of mw's and following hwndrulers.  then
		we need to have some way of getting a selection & properties for
		that mwd. */

	Assert (hmwd == hmwdCur);

	/* since we base whether we update on selCur and reset its
		flags, we don't want to be in the position of having dyadic
		move across windows and having selCur.ww not be wwCur.
		We could try to keep track of selDotted.fUpdateRuler, etc, but
		it would be messy. This way, while in dyadic move mode, we do
		not update the ruler and when it ends the update flags will
		still be on.
	*/
	if (vssc != sscNil)
		return;

	if ((hwndRuler = (*hmwd)->hwndRuler) == NULL)
		{  /* no ruler to update -- shouldn't have gotten here */
		Assert (fFalse);
		return;
		}

	hrsd = HrsdFromHwndRuler(hwndRuler);
	FreezeHp();
	prsd = *hrsd;
	Assert (prsd->hwndRuler == hwndRuler);

	/* check validity of rk */
	if ((prsd->rk == rkTable || rkNew == rkTable)
			&& !(psel->fTable || psel->fWithinCell))
		rkNew = rkNormal;

	if (fInit || (rkNew != -1 && rkNew != prsd->rk))
		{
		Assert(rkNew != -1);
		fChangeAll = fTrue;
		prsd->rk = rkNew;
		rpc = rpcAllPaint;
		fAbortOK = fFalse;
		}
	else
		/* the rpc field of the hrsd will have been set if someone wants to
			force something to be drawn, irregardless of the apparent need for
			update */
		rpc = prsd->rpc;

	prsd->rpc = rpcNoPaint;

	MeltHp();
	/* assure that vpapSelCur & papGray are up to date */
	if (selCur.fUpdatePap)
		if ( ! FGetParaState(fFalse /*!fAll*/, fAbortOK ))
			/* was interrupted */
			{
			/* we don't allow interuptions during initialization */
			Assert (!fInit);
			return;
			}
	FreezeHp();
	prsd = *hrsd;

	selCur.fUpdateRuler = fFalse;

	/* set up the difference pap's and the difference flags */
	if (fChangeAll)
		{  /* first time around, everything is "different" */
		SetWords (&papDiffProp, 0xffff, cwPAP);
		fDiffProp = fTrue;
		fDiffGray = fTrue;

		Assert (!fInit || prsd->jcTabCur == -1); /* assure ordering correct */
		}

	else
		{  /* see whats different */
		SetWords (&papDiffProp, 0, cwPAP);
		SetWords (&papDiffGray, 0, cwPAP);
		fDiffProp = FSetRgwDiff (&vpapSelCur, &prsd->pap, &papDiffProp,
				cwPAP);
		fDiffGray = FSetRgwDiff (&vpapGraySelCur, &prsd->papGray,
				&papDiffGray, cwPAP);

		Assert (prsd->jcTabCur != -1); /* assure ordering correct */
		}

	/* reflect updates */

	if (fDiffProp)
		blt (&vpapSelCur, &prsd->pap, cwPAP);

	if (fDiffGray)
		blt (&vpapGraySelCur, &prsd->papGray, cwPAP);

	/* check for changing section, document & display properties */

	MeltHp();
	if ((idr = IdrFromHpldrDocCp(hwwdCur, psel->ww, psel->doc, psel->cpFirst, 
			fFalse, fFalse)) != (*hrsd)->idr || fChangeAll)
		{
		(*hrsd)->idr = idr == -1 ? 0 : idr;
		rpc |= (rpcMarkArea | rpcScaleArea);
		}

	/* get the current props */
	CacheSect (psel->doc, psel->cpFirst);

	FreezeHp();
	prsd = *hrsd;

	if (prsd->rk == rkNormal)
		{
		int dxaColumn = prsd->pap.dxaWidth ? prsd->pap.dxaWidth :
		vsepFetch.dxaColumnWidth;
		BOOL fTabGray;
		BOOL fInTable = psel->fTable || psel->fWithinCell;
		int dxaTab = PdodMother (psel->doc)->dop.dxaTab;
		if (dxaTab == 0) dxaTab = dxaInch/2;  /* will divide by dxaTab */
		itbdMac = prsd->pap.itbdMac;

		Assert(PdodMother(psel->doc) == PdodDoc(DocMother(psel->doc)));

		if ((*hwwdCur)->fPageView && idr >= 0 && idr < (*hwwdCur)->idrMac)
			{
			struct DR *pdr;

			pdr = PInPl(hwwdCur, idr);
			if (pdr->lrk == lrkAbsHit)
				dxaColumn = DxaFromDxp(*hwwdCur, pdr->dxl);
			}

		/* update the fGrayTab flag */
		if (prsd->papGray.itbdMac)
			/* if the number of tabs differs, some tab must differ */
			fTabGray = fTrue;
		else
			fTabGray = 
					!(FRgbZero(prsd->papGray.rgdxaTab, prsd->pap.itbdMac*2)
					&& FRgbZero(prsd->papGray.rgtbd, prsd->pap.itbdMac));

		if (fChangeAll || dxaTab != prsd->dxaTab
				|| papDiffProp.dxaRight || papDiffGray.dxaRight
				|| papDiffProp.dxaLeft1 || papDiffGray.dxaLeft1
				|| papDiffProp.dxaLeft || papDiffGray.dxaLeft
				|| papDiffProp.itbdMac || papDiffGray.itbdMac
				|| !FRgbZero (papDiffGray.rgdxaTab, itbdMac * 2)
				|| !FRgbZero (papDiffProp.rgdxaTab, itbdMac * 2)
				|| !FRgbZero (papDiffProp.rgtbd, itbdMac)
				|| !FRgbZero (papDiffGray.rgtbd, itbdMac)
				|| fTabGray != prsd->fTabGray
				|| fInTable != prsd->fInTable)
			{
			prsd->dxaTab = dxaTab;
			prsd->fTabGray = fTabGray;
			prsd->fInTable = fInTable;
			rpc |= rpcMarkArea;
			}

		if (fInTable)
			{
			int itc;
			int dxaCellLeft;
			MeltHp();
			if (psel->fColumn)
				{
				itc = psel->itcFirst;
				if (psel->itcFirst != psel->itcLim - 1)
					goto LGrayTc;
				}
			else
				{
				itc = ItcFromDocCp(psel->doc, psel->cpFirst);
				if (!psel->fIns && 
						itc != ItcFromDocCp(psel->doc, psel->cpLim-1))
					{
LGrayTc:
					if (!prsd->papGray.dxaLeft)
						rpc |= rpcMarkArea | rpcScaleArea;
					prsd->papGray.dxaLeft = prsd->papGray.dxaLeft1 =
							prsd->papGray.dxaRight = 0xffff;
					}
				}

			CpFirstTap(psel->doc, psel->cpFirst);
			dxaCellLeft = vtapFetch.rgdxaCenter[itc] + vtapFetch.dxaGapHalf
					+ vtapFetch.dxaAdjust;
			if (itc >= vtapFetch.itcMac)
				dxaColumn = vtapFetch.dxaGapHalf * 2;
			else
				{
				int itcEnd = itc+1;
				if (vtapFetch.rgtc[itc].fFirstMerged)
					while (itcEnd < vtapFetch.itcMac)
						{
						if (!vtapFetch.rgtc[itcEnd].fMerged)
							break;
						itcEnd++;
						}
				dxaColumn = vtapFetch.rgdxaCenter[itcEnd] - dxaCellLeft
						- vtapFetch.dxaGapHalf + vtapFetch.dxaAdjust;
				}
			FreezeHp();
			prsd = *hrsd;
			if (fChangeAll || dxaCellLeft != prsd->dxaCellLeft || 
					itc != prsd->itcCur)
				{
				prsd->dxaCellLeft = dxaCellLeft;
				prsd->itcCur = itc;
				rpc |= rpcMarkArea | rpcScaleArea;
				}
			}
		else
			{
			prsd->itcCur = 0;
			prsd->dxaCellLeft = 0;
			}
		if (fChangeAll || prsd->dxaColumnNorm != dxaColumn)
			{
			prsd->dxaColumnNorm = dxaColumn;
			rpc |= rpcColumnMark;
			}
		}
	else  if (prsd->rk == rkTable)
		/* get table information */
		{
		int rgdxaCenter[itcMax+1];
		BOOL fTcGray = fFalse;
		int itc;
		MeltHp();
		if (!FGetTapState(&psel->ca, fFalse))  /* Ninch */
			{
			fTcGray = fTrue;
			CpFirstTap(psel->doc, psel->cpFirst);
			}

		for (itc = 0 ; itc <= vtapFetch.itcMac ; itc++ )
			rgdxaCenter[itc] = vtapFetch.dxaAdjust + vtapFetch.rgdxaCenter[itc];
		for (itc = 0; itc < vtapFetch.itcMac; itc++)
			if (vtapFetch.rgtc[itc].fMerged)
				rgdxaCenter[itc+1] = xaNil;

		FreezeHp();
		prsd = *hrsd;
		if (fChangeAll || prsd->fTcGray != fTcGray
				|| prsd->itcMac != vtapFetch.itcMac
				|| vtapFetch.dxaGapHalf != prsd->dxaGapHalf
				|| FNeRgch(rgdxaCenter, &prsd->rgdxaCenter, 
				(vtapFetch.itcMac + 1) * sizeof(int)))
			{
			prsd->fTcGray = fTcGray;
			prsd->dxaGapHalf = vtapFetch.dxaGapHalf;
			prsd->itcMac = vtapFetch.itcMac;
			blt(rgdxaCenter, &prsd->rgdxaCenter, vtapFetch.itcMac+1);
			rpc |= rpcMarkArea;
			}
		}

	else  /* prsd->rk == rkPage */
		
		{
		BOOL fPageView = (*hwwdCur)->fPageView;
		struct DOP *pdop = &PdodMother(psel->doc)->dop;
		int dxaExtraRight = pdop->dxaGutter;
		int dxaExtraLeft = 0;
		int dxaRightMar;
		int dxaLeftMar;
		BOOL fRight = fFalse;
		BOOL fMirrorMargins = fFalse;

		if (fPageView)
			{
			int ipgd = (*hwwdCur)->ipgd;
			struct PGD pgd;
			if (FFacingPages(DocMother(psel->doc)))
				{
				GetPlc(PdodMother(psel->doc)->hplcpgd, ipgd, &pgd);
				fRight = pgd.fRight;
				}
			fMirrorMargins = pdop->fMirrorMargins;
			if (fRight)
				{
				dxaExtraLeft = dxaExtraRight;
				dxaExtraRight = 0;
				}
			}
		if (!fMirrorMargins || fRight)
			{
			dxaLeftMar = pdop->dxaLeft;
			dxaRightMar = pdop->dxaRight;
			}
		else
			{
			dxaLeftMar = pdop->dxaRight;
			dxaRightMar = pdop->dxaLeft;
			}

		if (fChangeAll || prsd->dxaLeftMar != dxaLeftMar
				|| prsd->dxaRightMar != dxaRightMar
				|| prsd->ddxaColumns != vsepFetch.dxaColumns
				|| prsd->dxaExtraRight != dxaExtraRight
				|| prsd->dxaExtraLeft != dxaExtraLeft
				|| prsd->ccolM1 != vsepFetch.ccolM1
				|| prsd->dxaColumnPage != vsepFetch.dxaColumnWidth)
			{
			prsd->dxaLeftMar = dxaLeftMar;
			prsd->dxaRightMar = dxaRightMar;
			prsd->ddxaColumns = vsepFetch.dxaColumns;
			prsd->dxaExtraRight = dxaExtraRight;
			prsd->dxaExtraLeft = dxaExtraLeft;
			prsd->ccolM1 = vsepFetch.ccolM1;
			prsd->dxaColumnPage = vsepFetch.dxaColumnWidth;
			rpc |= rpcMarkArea;
			}
		if (fChangeAll || prsd->xaPage != pdop->xaPage)
			{
			prsd->xaPage = pdop->xaPage;
			rpc |= rpcMarkArea | rpcScaleArea;
			}
		}

	Assert (vpref.ut >= 0 && vpref.ut < utMaxUser);

	/* dxaCellLeft must already be set if rk == rkNormal! */
	GetXpsForHwwd(hwwdCur, hrsd, &xpLeft, &xpZero, &xpRight);

	if (fChangeAll || vpref.ut != prsd->ut
			|| prsd->xpZero != xpZero)
		{
		prsd->ut = vpref.ut;
		prsd->xpZero = xpZero;
		rpc |= (rpcScaleArea | rpcMarkArea);
		}
	if (fChangeAll || xpLeft != prsd->xpLeft
			|| xpRight != prsd->xpRight)
		{
		prsd->xpLeft = xpLeft;
		prsd->xpRight = xpRight;
		rpc |= (rpcScaleArea | rpcMarkArea);
		PositionMarkWindow(prsd->hwndRuler);
		}

	MeltHp();

	/* are there any changes ?? */
	if (! fDiffProp && ! fDiffGray && ! (rpc & rpcAllPaint))
		/* nope */
		{
		goto LCheckEnable;
		}

	/* enable it for update */
	if (!IsWindowEnabled(hwndRuler))
		EnableIconAndCombo(hwndRuler, idRulStyle, fTrue/*fEnable*/);

	/* if initializing set & show the selected tab style */
	if (fInit)
		{
		(*hrsd)->jcTabCur = jcLeft;
		Assert (jcLeft != -1); /* used as a test value! */
		UpdateRulerToggle  (hrsd, (*hrsd)->jcTabCur, fFalse, idRulTabLeft);
		}

	/* see what has changed */
	if (fDiffProp | fDiffGray)
		{
		/* Make Toggle Updates where needed */

	/* **********************  NOTE!!!!!  *************************
		Failure can occur in the dialog portion of the ruler which will
		cause the ruler to be destroyed by sdm. We therefore check
		the ruler window after the style update, and bag out
		if the window has been destroyed.
		************************************************************ */


		/* style */
		if (  papDiffProp.stc | papDiffGray.stc )
			{
			UpdateRulerStyle (hrsd, (*hrsd)->pap.stc,
					(*hrsd)->papGray.stc);
			if ((*hmwd)->hwndRuler == NULL)
				return;
			}


		FreezeHp();
		prsd = *hrsd;

		/* spacing */
		if (  papDiffProp.dyaLine | papDiffGray.dyaLine )
			UpdateRulerToggle (hrsd, prsd->pap.dyaLine,
					prsd->papGray.dyaLine, idRulSpace1);

		/* paragraph spacing */
		if (  papDiffProp.dyaBefore | papDiffGray.dyaBefore )
			UpdateRulerToggle (hrsd, prsd->pap.dyaBefore,
					prsd->papGray.dyaBefore, idRulParaClose);

		/* paragraph justification */
		if (  papDiffProp.jc | papDiffGray.jc )
			UpdateRulerToggle (hrsd, prsd->pap.jc,
					prsd->papGray.jc, idRulParaLeft);
		MeltHp();
		}


	/* paint changes */
	/*    Note:  if initializing (fInit) then the window is
			not yet visible and there is no
			point in painting the ruler yet (there will be a paint message later
			to ensure the window gets painted) */
	if (!fInit)
		if (rpc != rpcNoPaint)
			PaintRuler (hrsd, rpc);

LCheckEnable:
	/* disable after update if current doc is locked */
	if ((PdodDoc(selCur.doc)->fLockForEdit || 
			(PdodDoc(selCur.doc)->fShort && PdodMother(selCur.doc)->fLockForEdit)) &&
			IsWindowEnabled(hwndRuler))
		EnableIconAndCombo(hwndRuler, idRulStyle, fFalse /*fEnable*/);
}



/*  UpdateRulerStyle
	Sets the text of the style combo box.
*/

/* %%Function:UpdateRulerStyle   %%Owner:peterj */
UpdateRulerStyle (hrsd, stc, fGray)
struct RSD **hrsd;
int stc;
BOOL fGray;
{
	struct STSH stsh;
	char sz[cchMaxStyle];

	RecordStshForDocNoExcp(DocMother(selCur.doc), &stsh);
	sz[0] = 0;
	if (!fGray)
		{
		GenStyleNameForStcp(sz, stsh.hsttbName, stsh.cstcStd, 
				StcpFromStc(stc, stsh.cstcStd));
		StToSzInPlace(sz);
		}
	SetIibbTextHwndIb((*hrsd)->hwndRuler, idRulStyle, sz);
}



/*  UpdateRulerToggle
	Sets the toggles for idRul to value val.
*/

/* %%Function:UpdateRulerToggle   %%Owner:peterj */
UpdateRulerToggle (hrsd, val, fGray, idRul)
struct RSD ** hrsd;
int val, idRul;
BOOL fGray;
{

	int idRulSet = -1;
	int idRulFirst = 0;
	int idRulLast = 0;

	switch (idRul)
		{
	case idRulTabLeft:
	case idRulTabCenter:
	case idRulTabRight:
	case idRulTabDecimal:
		idRulFirst = idRulTabLeft;
		idRulLast = idRulTabDecimal;
		idRulSet = val - jcLeft + idRulTabLeft;
		break;

	case idRulParaLeft:
	case idRulParaCenter:
	case idRulParaRight:
	case idRulParaBoth:
		idRulFirst = idRulParaLeft;
		idRulLast = idRulParaBoth;
		idRulSet = val - jcLeft + idRulParaLeft;
		break;

	case idRulSpace1:
	case idRulSpace15:
	case idRulSpace2:
		idRulFirst = idRulSpace1;
		idRulLast = idRulSpace2;
		val = abs(val);

		if (val <= dyaSpace1)
			idRulSet = idRulSpace1;
		else  if (val >= dyaSpace2)
			idRulSet = idRulSpace2;
		else
			idRulSet = idRulSpace15;
		break;

	case idRulParaClose:
	case idRulParaOpen:
		idRulFirst = idRulParaClose;
		idRulLast = idRulParaOpen;

		if (val <= dyaParaClose)
			idRulSet = idRulParaClose;
		else
			idRulSet = idRulParaOpen;
		break;

#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif  /* DEBUG */
		}

	SelectRadioIibb((*hrsd)->hwndRuler, idRulFirst, idRulLast,
			(fGray ? idNil : idRulSet), fGray);
}



/* %%Function:UpdateRulerTab  %%Owner:peterj */
UpdateRulerTab(jc, hwndRuler)
int jc;
{
	/* called out of tabs dialog to update ruler */
	struct RSD **hrsd;

	Assert (hwndRuler != hNil);
	hrsd = HrsdFromHwndRuler(hwndRuler);
	if ((*hrsd)->jcTabCur != jc)
		{
		(*hrsd)->jcTabCur = jc;
		UpdateRulerToggle (hrsd, jc, fFalse /*!gray*/,
				idRulTabLeft - jcLeft + jc);
		}
}


/* RULER SCROLLING */


/*  ScrollRuler
	called when the document window is scrolled either left or right, this
	function performs the indicated scroll and updates xhScroll.  Repainting
	and cliping will be handled by windows.
*/
/* %%Function:ScrollRuler   %%Owner:peterj */
ScrollRuler ()
{
	HWND hwndRuler;
	HWND hwndMark;
	struct RSD ** hrsd;
	int dxpScroll, xpZero, xpLeft, xpRight;

	Assert (hmwdCur != hNil);

	if ((hwndRuler = (*hmwdCur)->hwndRuler) == NULL)
		/* no ruler to scroll */
		return;
	if (selCur.fUpdateRuler)
		UpdateRuler(hmwdCur, fFalse /*fInit*/, -1, fFalse /*fAbortOK*/);

	hrsd = HrsdFromHwndRuler(hwndRuler);
	Assert ((*hrsd)->hwndRuler == hwndRuler);
	hwndMark = (*hrsd)->hwndMark;

	/* figure amount to scroll */
	GetXpsForHwwd(hwwdCur, hrsd, &xpLeft, &xpZero, &xpRight);

	if (xpLeft != (*hrsd)->xpLeft || xpRight != (*hrsd)->xpRight)
		{
		(*hrsd)->xpLeft = xpLeft;
		(*hrsd)->xpRight = xpRight;
		PositionMarkWindow(hwndRuler);
		}

	dxpScroll = xpZero - (*hrsd)->xpZero;
	(*hrsd)->xpZero = xpZero;

	if (dxpScroll)
		{
		int xpRightOld = vpreb == NULL ? 0 : vpreb->xpRight;

		/* scroll the ruler mark window */
		ScrollWindow(hwndMark, dxpScroll, 0, NULL, NULL);

		/* this causes ruler to be redrawn */
		UpdateWindow (hwndMark);

		if (xpRight)
			{
			vpreb->xpRight = xpRightOld;
			vpreb->xpLeft = 0;
			}
		}
}


/* RULER PAINTING FUNCTIONS */


/*  PaintRuler
*/

/* %%Function:PaintRuler   %%Owner:peterj */
PaintRuler (hrsd, rpc)
struct RSD ** hrsd;
int rpc;

{
	HDC hdc;
	int xp;
	HWND hwndMark = (*hrsd)->hwndMark;
	struct REB *preb, reb;
	BOOL fErased = fFalse;
	PAINTSTRUCT ps;
#ifdef WIN23
	HBRUSH hbrOld;
#endif /* WIN23 */


	Assert (vpreb == NULL || vpreb->hrsd != hNil);

	/*  I am asserting that UpdateRuler w/fInit is called before we ever
		get around to painting the ruler.  Otherwise the rsd will contain
		uninitialized values.  */
	Assert ((*hrsd)->jcTabCur != -1); /* assure ordering correct */

	/* get a preb to paint with.  this uses any applicable global preb
		that is out there otherwise the local one.  if using the local preb, it
		is not made available to others */
	preb = PrebInitPrebHrsd (&reb, hrsd, fFalse/*fGlobal*/);

	if ( rpc & rpcmPaintStr )
		{
		BeginPaint(hwndMark, (LPPAINTSTRUCT)&ps);
		hdc = ps.hdc;
		fErased = ps.fErase;
		preb->xpLeft = ps.rcPaint.left;
		preb->xpRight = ps.rcPaint.right;/* restrict paint to this rectangle */
		}
	else
		hdc = GetDC(hwndMark);

	if (!FSetDcAttribs (hdc, dccRuler))
		{
		ErrorNoMemory(eidNoMemDisplay);
		goto LRetCleanup;
		}
#ifdef WIN23
	if (vsci.fWin3Visuals)
		hbrOld = SelectObject(hdc, vdbmgDevice != dbmgEGA3 ? vhbrLtGray : vhbrGray);
#endif /* WIN23 */

	/* were all set up -- do the painting */

	if ((rpc & (rpcmScale | rpcmColumn)) && 
			SelectObject (hdc, vsci.hpen) == NULL)
		{
		ErrorNoMemory(eidNoMemDisplay);
		goto LRetCleanup;
		}

	if (rpc & rpcmScale)
		DrawRulerScale (hdc, preb, fErased);

	if (rpc & rpcmMark)
		DrawRulerMarks (hdc, preb, fErased);

	if ((*hrsd)->rk == rkNormal)
		{
		if ((rpc & rpcmColumn) && 
				(xp = XpMarkFromXaMark ((*hrsd)->dxaColumnNorm, hrsd)) < 
				preb->xpRight && xp >= preb->xpLeft)
			PatBltVertLine(hdc, xp, 0, ypMarkBottom);
		}
	else
		{
		if ((*hrsd)->xpZero == 0)
			PatBltVertLine(hdc, 0, ypMark, dypMark);
		if ((*hrsd)->xpRight != 0x7fff)
			PatBltVertLine(hdc, (*hrsd)->xpRight-vsci.dxpBorder, ypMark, 
					dypMark);
		}


LRetCleanup:
	/* clean up */
	FreePreb (preb);
#ifdef WIN23
	if (vsci.fWin3Visuals && hbrOld != NULL)
		SelectObject(hdc, hbrOld);
#endif /* WIN23 */

	if (rpc & rpcmPaintStr)
		EndPaint(hwndMark, (LPPAINTSTRUCT)&ps);
	else
		ReleaseDC (hwndMark, hdc);
}



/*  DrawRulerScale
	draws the scale line, ticks & labels
*/
/* %%Function:DrawRulerScale   %%Owner:peterj */
#ifdef WIN23
DrawRulerScl2 (hdc, preb, fErased)
#else
DrawRulerScale (hdc, preb, fErased)
#endif /* WIN23 */
HDC hdc;
struct REB * preb;
BOOL fErased;
{
	struct RSD ** hrsd = preb->hrsd;
	int xp;
	int iTick;
	int ut = (*hrsd)->ut;
	int dxa = mputdxa [ut];
	int ypText = ypScale-dypMajorTick+(vsci.dypTmInternalLeading<2);
	int xa;
	CHAR *pch, sz [10];
	HBRUSH hbrOld;
	/* hpen must have already been selected in */

	if (!fErased)
		PatBlt(hdc, preb->xpLeft, 0, preb->xpRight-preb->xpLeft, ypScale, 
				vsci.dcibScreen.ropErase);

	hbrOld = SelectObject(hdc, vsci.hbrText);

	/* ticks & labels */
	for (iTick = DxaFromDxs (0/*dummy*/, -(*hrsd)->xpZero) / dxa - 1, 
			xa = iTick * dxa;
			(xp = XpMarkFromXaMark (xa, hrsd)) <= preb->xpRight
			&& xa >= xaMin;
			iTick++, xa += dxa)
		{
		/* draw major tick */
		PatBlt(hdc,  xp, ypText, vsci.dxpBorder, dypMajorTick,
				PATCOPY);

		/* label */
		pch = sz;
		CchIntToPpch (iTick * mputdn [ut], &pch);
		pch [1] = '\0';
		Assert (*pch <= sz + 8);
#ifdef WIN23
		TextOut (hdc, xp + vsci.dxpBorder, ypText - vsci.fWin3, (LPSTR) sz, pch - sz);
#else
		TextOut (hdc, xp + vsci.dxpBorder, ypText, (LPSTR) sz, pch - sz);
#endif /* WIN23 */

		/* minor & tiny ticks */
		DrawMinorTicks(hdc, xp, ypScale, dypMinorTick,dxa,mputn1[ut]);
		DrawMinorTicks(hdc, xp, ypScale, dypTinyTick, dxa,mputn2[ut]);
		}

	/* scale line */
	PatBlt(hdc, preb->xpLeft, ypScale, preb->xpRight-preb->xpLeft,
			vsci.dypBorder, PATCOPY);

	if (hbrOld != NULL)
		SelectObject(hdc, hbrOld);
}






/*  DrawRulerMarks
	draws tabs, default tabs and indents in the mark area.  grays those
	items which should be gray.  imkNoDraw & itbdNoDraw are set if this draw is
	called as a result of a scroll during a drag--they identify the item being
	dragged (which should not be drawn)
*/

/* %%Function:DrawRulerMarks   %%Owner:peterj */
DrawRulerMarks (hdc, preb, fErased)
HDC hdc;
struct REB * preb;
BOOL fErased;
{
	struct RSD **hrsd = preb->hrsd;
	struct RSD *prsd = *hrsd;
	int rk = prsd->rk;
	int itbd, itbdMac, imk;
	int xa, xaRight;
	int imkNoDraw;             /* do draw this mark (it is being dragged) */
	int iNoDraw;               /* do not draw this tab/cell/col (same reason) */
	int ms;                    /* mark state:  normal or gray */
	int dxaTab;                /* default tab spacing */

	FreezeHp();

	/* get the no-draw flags */
	imkNoDraw = preb->imkNoDraw;
	iNoDraw = preb->iNoDraw;

	/* erase the old marks */
#ifdef WIN23
	if (!fErased)
		PatBlt(hdc, preb->xpLeft, ypMark, preb->xpRight-preb->xpLeft, dypMark,
				vsci.fWin3Visuals ? PATCOPY : vsci.dcibScreen.ropErase);
#else
	if (!fErased)
		PatBlt(hdc, preb->xpLeft, ypMark, preb->xpRight-preb->xpLeft, 
				dypMark, vsci.dcibScreen.ropErase);
#endif /* WIN23 */


	if (rk == rkNormal)
		{
				/* DEFAULT TABS */
		/* first default tab */
		/* default tabs appear at the right of
				max (right-most tab, min (left indent, left1 indent) )
			and to the left of the right indent */
		/* computationally intense */
	{{  /* NATIVE - DrawRulerMarks */
			dxaTab = prsd->dxaTab;

			if ((itbdMac = prsd->pap.itbdMac) > 0)
				xa = prsd->pap.rgdxaTab [itbdMac -1] + 1;
			else
				xa = xaMin;
			xa = max (xa, min (prsd->pap.dxaLeft1 + prsd->pap.dxaLeft,
					prsd->pap.dxaLeft) + 1);
			xa = ((xa + dxaTab - 1) / dxaTab) * dxaTab;

			xaRight = min (prsd->dxaColumnNorm - prsd->pap.dxaRight,
					XaMarkFromXpMark (preb->xpRight, hrsd));
			}}

		for (; xa < xaRight; xa += dxaTab)
#ifdef WIN23
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkDefTab, msNormal, fFalse);
#else
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkDefTab, msNormal);
#endif /* WIN23 */

		/* INDENTS */
		/* left */
		xa = prsd->pap.dxaLeft;
		if (imkNoDraw != imkIndLeft && imkNoDraw != imkIndCombo)
			{
			ms = (prsd->papGray.dxaLeft) ? msGray : msNormal;
#ifdef WIN23
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkIndLeft, ms, fFalse);
#else
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkIndLeft, ms);
#endif /* WIN23 */
			}

		/* left1 */
		/* note: left1 xa dependent on left xa */
		if (imkNoDraw != imkIndLeft1 && imkNoDraw != imkIndCombo)
			{
			xa += prsd->pap.dxaLeft1;
			ms = (prsd->papGray.dxaLeft1) ? msGray : msNormal;
#ifdef WIN23
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkIndLeft1, ms, fFalse);
#else
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkIndLeft1, ms);
#endif /* WIN23 */
			}

		/* right */
		if (imkNoDraw != imkIndRight)
			{
			xa = prsd->dxaColumnNorm - prsd->pap.dxaRight;
			ms = (prsd->papGray.dxaRight) ? msGray : msNormal;
#ifdef WIN23
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkIndRight, ms, fFalse);
#else
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
					imkIndRight, ms);
#endif /* WIN23 */
			}

		/* TABS */
		ms = (prsd->fTabGray) ? msGray : msNormal;
		for (itbd = 0, itbdMac = prsd->pap.itbdMac; itbd < itbdMac; itbd++)
			if (itbd != iNoDraw)
#ifdef WIN23
				BltRulerMark (hdc, preb,
						XpMarkFromXaMark (prsd->pap.rgdxaTab [itbd], hrsd),
						((struct TBD)prsd->pap.rgtbd [itbd]).jc, ms, fFalse);
#else
				BltRulerMark (hdc, preb,
						XpMarkFromXaMark (prsd->pap.rgdxaTab [itbd], hrsd),
						((struct TBD)prsd->pap.rgtbd [itbd]).jc, ms);
#endif /* WIN23 */

		/* CURSOR */
		if ( (xa = preb->xaCursor) != xaNil)
			DrawRulerCursor (hdc, preb, xa);
		}

	else  if (rk == rkTable)
		{
		int itc;
		int itcMac = prsd->itcMac;
		int ms = prsd->fTcGray ? msGray : msNormal;

		/*  left mark */
		if (imkNoDraw != imkTLeft)
#ifdef WIN23
			BltRulerMark(hdc, preb, XpMarkFromXaMark(prsd->rgdxaCenter[0]
					+ prsd->dxaGapHalf, hrsd), imkTLeft, ms, fFalse);
#else
			BltRulerMark(hdc, preb, XpMarkFromXaMark(prsd->rgdxaCenter[0]
					+ prsd->dxaGapHalf, hrsd), imkTLeft, ms);
#endif /* WIN23 */

		for (itc = 0; itc < itcMac; itc++)
			if (iNoDraw != itc && prsd->rgdxaCenter[itc+1] != xaNil)
#ifdef WIN23
				BltRulerMark(hdc, preb, 
						XpMarkFromXaMark(prsd->rgdxaCenter[itc+1], hrsd),
						imkTableCol, ms, fFalse);
#else
				BltRulerMark(hdc, preb, 
						XpMarkFromXaMark(prsd->rgdxaCenter[itc+1], hrsd),
						imkTableCol, ms);
#endif /* WIN23 */
		}

	else  /* rk == rkPage */
		
		{
		int dxaCol, ddxaCol, xa;
		int icol, icolMac = prsd->ccolM1;
		int i = 0;

		xa = min(prsd->dxaLeftMar+prsd->dxaExtraLeft, xaRightMaxSci);

		/* left margin */
		if (iNoDraw != i++)
#ifdef WIN23
			BltRulerMark(hdc, preb, XpMarkFromXaMark(xa, hrsd), imkLeftMargin,
					msNormal, fFalse);
#else
			BltRulerMark(hdc, preb, XpMarkFromXaMark(xa, hrsd), imkLeftMargin,
					msNormal);
#endif /* WIN23 */

		/* right margin */
		if (iNoDraw != i++)
#ifdef WIN23
			BltRulerMark(hdc, preb, XpMarkFromXaMark(min(prsd->xaPage - 
					prsd->dxaRightMar-prsd->dxaExtraRight, xaRightMaxSci), hrsd),
					imkRightMargin, msNormal, fFalse);
#else
			BltRulerMark(hdc, preb, XpMarkFromXaMark(min(prsd->xaPage - 
					prsd->dxaRightMar-prsd->dxaExtraRight, xaRightMaxSci), hrsd),
					imkRightMargin, msNormal);
#endif /* WIN23 */

		dxaCol = prsd->dxaColumnPage;
		ddxaCol = prsd->ddxaColumns;
		for (icol = 0; icol < icolMac; icol++)
			{
			xa += dxaCol;
			if (iNoDraw != i++)
#ifdef WIN23
				BltRulerMark(hdc, preb, XpMarkFromXaMark(xa, hrsd),
						imkRightMargin, msNormal, fFalse);
#else
				BltRulerMark(hdc, preb, XpMarkFromXaMark(xa, hrsd),
						imkRightMargin, msNormal);
#endif /* WIN23 */
			xa += ddxaCol;
			if (iNoDraw != i++)
#ifdef WIN23
				BltRulerMark(hdc, preb, XpMarkFromXaMark(xa, hrsd),
						imkLeftMargin, msNormal, fFalse);
#else
				BltRulerMark(hdc, preb, XpMarkFromXaMark(xa, hrsd),
						imkLeftMargin, msNormal);
#endif /* WIN23 */
			}
		}

	MeltHp();
}



/*  DrawMinorTicks
	draw n -1 minor ticks over an area dxa wide starting at xp with height
	dyp.
*/

/* %%Function:DrawMinorTicks   %%Owner:peterj */
DrawMinorTicks (hdc, xp, yp, dyp, dxa, n)
HDC hdc;
int xp, yp, dyp, dxa, n;
{
	int i;
	int xpCur;

	yp -= dyp;

	for (i = 1; i < n; i++)
		{
		xpCur = xp + DxsFromDxa (0/*dummy*/, NMultDiv (i, dxa, n));
		PatBlt(hdc, xpCur, yp, vsci.dxpBorder, dyp, PATCOPY);
		}
}




/* RULER DRAWING UTILITIES */

/* %%Function:PatBltVertLine  %%Owner:peterj */
PatBltVertLine(hdc, xp, yp, dyp)
HDC hdc;
int xp, yp, dyp;
{
	HBRUSH hbrOld;

	hbrOld = SelectObject(hdc, vsci.hbrText);
	PatBlt(hdc, xp, yp, vsci.dxpBorder, dyp, PATCOPY);
	if (hbrOld != NULL)
		SelectObject(hdc, hbrOld);
}


/*  BltRulerMark
	place a ruler mark on hdc in state ms.
*/

/* %%Function:BltRulerMark   %%Owner:peterj */
#ifdef WIN23
BltRulerMark2 (hdc, preb, xp, imk, ms)
#else
BltRulerMark (hdc, preb, xp, imk, ms)
#endif /* WIN23 */
HDC hdc;
struct REB * preb;
int xp, imk, ms;
{
#ifdef WIN23
	struct BMI *pbmi = &mpidrbbmi [idrbRulerMarks2];
#else
	struct BMI *pbmi = &mpidrbbmi [idrbRulerMarks];
#endif /* WIN23 */
	int dxp = pbmi->dxpEach;
	int ddxp = dxp / 2;
	struct MDCD *pmdcd;

	long rop = ((ms>>1) ^ (vsci.fInvertMonochrome))
		?  ROP_PDSxxn  :  ROP_DPSxx;

	/* assumptions made in calculating rop above */
	Assert (vsci.fInvertMonochrome == 1 || vsci.fInvertMonochrome == 0);
	Assert (msInvert == 2 && msNormal == 0 && msGray == 1);

	Assert (preb->hrsd != hNil);
	Assert( dxp != 0 );

	if (xp + ddxp < preb->xpLeft || xp - ddxp > preb->xpRight)
		/* none of the mark is visible, don't bother */
		return;

#ifdef WIN23
	if ((pmdcd = PmdcdCacheIdrb( idrbRulerMarks2, hdc )) != NULL)
#else
	if ((pmdcd = PmdcdCacheIdrb( idrbRulerMarks, hdc )) != NULL)
#endif /* WIN23 */
		{
		int xpSrc;

		xpSrc = imk * pbmi->dxpEach;
		if (ms == msGray)
			xpSrc += (pbmi->dxp>>1);

		BitBlt (hdc, (xp - ddxp), ypMark, dxp, pbmi->dyp, pmdcd->hdc,
				xpSrc, 0, rop);
		}
}


/*  PrebInitPrebHrsd
	this obtains a ruler edit block for subsequent edits or paints of the
	ruler.  if there is already a globally available reb for this hrsd, we use
	it.  otherwise we use the one passed to us.
*/

/* %%Function:PrebInitPrebHrsd   %%Owner:peterj */
struct REB * PrebInitPrebHrsd (preb, hrsd, fGlobal)
struct REB * preb;
struct RSD ** hrsd;
BOOL fGlobal;

{
	struct REB * prebUse;
	struct RC rc;

	Assert (!fGlobal || vpreb == NULL);

	if ((prebUse = vpreb) == NULL || prebUse->hrsd != hrsd)
		{
		prebUse = preb;
		prebUse->hrsd = hrsd;
		prebUse->cInit = 0;
		prebUse->rpcSetup = rpcNil;
		prebUse->imkNoDraw = iNil;
		prebUse->iNoDraw = iNil;
		prebUse->xaCursor = xaNil;
		prebUse->fUndoSet = fFalse;
		}

	if (fGlobal)
		vpreb = prebUse;

	prebUse->cInit++;
	GetClientRect((*hrsd)->hwndMark, &rc);
	prebUse->xpLeft = 0;
	prebUse->xpRight = rc.xpRight;

	return prebUse;
}



/*  FreePreb
	the passed preb is not going to be used further.  free any resources it has
	obtained.
*/

/* %%Function:FreePreb   %%Owner:peterj */
FreePreb (preb)
struct REB * preb;
{
	/* for every initpreb there is a matching freepreb.  if there are still
		unmatched init's then this preb is still in use, don't free anything */

	if (--preb->cInit)
		{
		/* restore the clipping area */
		struct RC rc;
		GetClientRect((*preb->hrsd)->hwndMark, &rc);
		preb->xpLeft = 0;
		preb->xpRight = rc.xpRight;
		return;
		}

	/* just in case */
	preb->hrsd = hNil;
	preb->rpcSetup = rpcNil;

	if (vpreb == preb)
		vpreb = NULL;
}


/* %%Function:GetXpsForHwwd  %%Owner:peterj */
GetXpsForHwwd(hwwd, hrsd, pxpLeft, pxpZero, pxpRight)
struct WWD **hwwd;
struct RSD **hrsd;
int *pxpLeft, *pxpZero, *pxpRight;
{
	int rk = (*hrsd)->rk;
	int xpLeft, xpZero;
	int dxaExtra = rk == rkNormal ? (*hrsd)->dxaCellLeft : 0;
	BOOL fPageView = (*hwwd)->fPageView;
	struct DOP *pdop = &PdodMother((*hwwd)->sels.doc)->dop;

	/* xpLeft: screen unit distance from left edge of ruler/ww to the
		beginning of active text (right of selbar in galley view) 
		positive or zero. */
	/* xpZero: screen unit distance from beginning of active text (also
		left edge of mark window) to xa0 (zero point on ruler).  May be
		positive, negative or zero. */
	/* xpRight: distance from xpLeft to where the window should end.  Only
		used in page mode. */

	if (rk == rkPage)
		{
		if (fPageView)
			/* xpZero = xpLeft = XwFromXe(hwwd, (*hwwd)->rcePage.xeLeft); IE */
			/* code XwFromXe in line now */
			xpZero = xpLeft = (*hwwd)->xwMin + (*hwwd)->rcePage.xeLeft;
		else
			xpZero = xpLeft = XwFromXp(hwwd, 0, -DxsFromDxa(0, pdop->dxaLeft));
		}
	else 
		
		{
		xpZero = XwFromXp(hwwd, (*hrsd)->idr, DxsFromDxa(0, dxaExtra));

		if (fPageView)
			xpLeft = 0;
		else
			xpLeft = (*hwwd)->xwSelBar + dxwSelBarSci;
		}

	xpLeft = max(xpLeft, 0);
	xpZero -= xpLeft;

	if (pxpLeft != NULL)
		*pxpLeft = xpLeft;

	if (pxpZero != NULL)
		*pxpZero = xpZero;

	if (pxpRight != NULL)
		{
		int xpRight;
		if (rk != rkPage)
			xpRight = 0x7fff;
		else  if (fPageView)
		/* xpRight = XwFromXe(hwwd, DxsFromDxa(0, pdop->xaPage)
			+ (*hwwd)->rcePage.xeLeft); */
		/* code XwFromXe in line now */
			xpRight = (*hwwd)->xwMin + DxsFromDxa(0, pdop->xaPage)
					+ (*hwwd)->rcePage.xeLeft;
		else
			xpRight = XwFromXp(hwwd, 0, DxsFromDxa(0, pdop->xaPage-pdop->dxaLeft));

		*pxpRight = max(xpRight, xpLeft);
		}
}








/*  XaMarkFromXpMark
	determine the document xa from a screen xp for a ruler mark.  quantize to
	nearest dxaQuantum.
*/

/* %%Function:XaMarkFromXpMark   %%Owner:peterj */
XaMarkFromXpMark (xp, hrsd)
int xp;
struct RSD **hrsd;
{
	/* convert to twips & quantize */
	return XaQuantize (DxaFromDxs (0/*dummy*/, xp-(*hrsd)->xpZero), 
			mputdxaQuantum[vpref.ut], vpref.ut==utCm?ddxaCmErrorQuantum:0);
}



/*  XpMarkFromXaMark
	get the screen xp for a document xa
*/

/* %%Function:XpMarkFromXaMark   %%Owner:peterj */
XpMarkFromXaMark (xa, hrsd)
int xa;
struct RSD **hrsd;
{
	return (xa < xaMin) ? -1 :
			DxsFromDxa (0/*dummy*/, xa) + (*hrsd)->xpZero;
}



/*  XaQuantize
	limit xa to within [xaMin, xaMax] and make it a multiple of dxa
*/
/* %%Function:XaQuantize   %%Owner:peterj */
NATIVE XaQuantize (xa, dxa, ddxaAdj)
int xa;
int dxa;
int ddxaAdj; /* for cm adjustment */

{
	int dxaAdj = ddxaAdj > 0 ? -(xa/ddxaAdj) : 0;
	xa += dxaAdj;
	return min ( max (
			((((xa>=0)?(xa + (dxa>>1)):(xa - (dxa>>1))) / dxa) * dxa) + dxaAdj, 
			xaMin), xaMax );
}





/* GENERAL RULER COMMAND UTILITIES */


/*  UpdateRulerProp
	this function modifies the currently cached versions of the pap and papGray
	in **hrsd.
*/
/* %%Function:UpdateRulerProp   %%Owner:peterj */
UpdateRulerProp (hrsd, prlVal, cbPrlVal, prlGray, cbPrlGray)
struct RSD ** hrsd;
CHAR * prlVal, * prlGray;
int cbPrlVal, cbPrlGray;
{
	/* I'm passing heap pointers around! */
	FreezeHp ();

	if (cbPrlVal > 0)
		ApplyPrlSgc ((char HUGE *) prlVal, cbPrlVal, & (*hrsd)->pap, sgcPap);

	if (cbPrlGray > 0)
		ApplyPrlSgc ((char HUGE *) prlGray, cbPrlGray, & (*hrsd)->papGray, sgcPap);

	MeltHp ();
}


/* F  R G B  Z E R O

	Return fTrue iff rgb is all zeros
*/

/* %%Function:FRgbZero   %%Owner:peterj */
FRgbZero (rgb, ibMax)
char * rgb;
int ibMax;

{
	char * pb;
	char * pbMax = &rgb [ibMax];

	for (pb = rgb; pb < pbMax && !*pb; pb++);

	return pb == pbMax;
}


/* Ribbon drawing code */


/* ----- External variables */

extern BOOL             vfRecording;
extern CHAR             szEmpty[];
extern HWND             vhwndMsgBoxParent;
extern struct SEL       selCur;
extern HMENU            vhMenu;
extern HMENU            vhMenuWithMwd;
extern struct CA        caPara;
extern CP               vcpFetch;
extern int              vccpFetch;
extern struct PREF      vpref;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct DOD       **mpdochdod [];
extern struct STTB      **vhsttbFont;
extern HANDLE           vhInstance;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern struct WWD       **hwwdCur;
extern HWND             vhwndStatLine;
extern HWND             vhwndDeskTop;
extern HWND             vhwndCBT;
extern HCURSOR          vhcArrow;

extern struct SCI           vsci;
extern struct PRI           vpri;

extern RRF		rrf;
extern struct CHP  chpRibbon;   /* these hold current state of ribbon */
extern struct CHP  chpRibbonGray;
extern CHAR grpfShowAllRibbon;


/* UpdateRibbon

	updates the state of the ribbon.  It gets the current values for
	the properties that can be shown and compares them to the state of what
	is being displayed.

	If fInit is true, all controls are updated no matter what their
		previous state. fInit is set true during initial creation.

*/

/* %%Function:UpdateRibbon  %%Owner:bobz */
UpdateRibbon(fInit)
BOOL fInit;
{
	HWND hwnd = vhwndRibbon;
	struct CHP chpDiffProp;
	struct CHP chpDiffGray;
	BOOL fDiffProp, fDiffGray, fDiffShowAll;
	BOOL fEnabled;
	CHAR grpfShowAllNew;

	Assert (vhwndRibbon != hNil);

	/* General scheme:
	
	        0. Be sure that selCur is the active selection. Bag out
	           in dyadic mode, as wwCUr may not be selCur.ww
	
		1. Be sure selCur.chp reflects current state of selection .
	
		2. selCur.fUpdateRibbon is set in FGetCharState so that if other people
		call FGetCharState the LH will get updated anytime it is required -
		Otherwise, some weird case may prevent an update.
	
		3. Get differences between selCur chp's and those used by the ribbon.
		If there are any differences in the fields of either the
		property or gray chps between the selection and the ribbon,
		change the corresponding ribbon fields to reflect the selection.
	
		4. We assert that by the time  UpdateRibbon is done, the Ribbon
		chps will match those of the selCur, so we don't change the
		chp entries as each property changes, but merely update the state
		of the LH controls. At the end, we blt the selCur chps onto the LH
		chps.
	
		5. If no child window is up, we gray out everything (FGetCharState will
		prepare this) and then disable the LH window to prevent the
		combo boxes from getting the focus
	*/

	if (PselActive() != &selCur)   /* only valid for selCur */
		return;

	if (hwwdCur != hNil)
		grpfShowAllNew = (*hwwdCur)->grpfvisi.fvisiShowAll;
	else
		grpfShowAllNew = 2; /* gray, not 0 or 1 */

	if (selCur.fUpdateChpGray || fInit || selCur.fUpdateChp)
		if (FGetCharState(fFalse/*fAll*/, fTrue /*fAbortOk*/ ) == fFalse)
			{
			/* was interrupted */
			if (fInit)
				/* at init, interrupt causes all gray in LH */
				SetWords(&vchpGraySelCur, 0xFFFF, cwCHP);
			else
				return;
			}

	selCur.fUpdateRibbon = fFalse;  /* prevent unneeded updates later */

	/* get difference chps. if fInit, set one array to all different to
		force full update */

	if (fInit)
		{
		/* force all character attributes to be updated */
		SetWords(&chpDiffProp, 0xFFFF, cwCHP);  /* acts like all compares failed */
		fDiffProp = fTrue;  /* need >= 1 of these to be true */
		fDiffShowAll = fTrue;
		}
	else
		{
		SetWords(&chpDiffProp, 0, cwCHP);  /* note WORD blt's */
		SetWords(&chpDiffGray, 0, cwCHP);
		fDiffProp = FSetRgwDiff (&selCur.chp, &chpRibbon, &chpDiffProp, cwCHP);
		fDiffGray = FSetRgwDiff (&vchpGraySelCur, &chpRibbonGray,
				&chpDiffGray, cwCHP);
		fDiffShowAll = grpfShowAllRibbon != grpfShowAllNew;
		}


	/* enable a disabled ribbon if there is a doc and it is not a locked
		doc with no changes
	*/
	/* if LH had been disabled before when we lost the last child window,
		and we haven't opened a document yet, then butt out.
		Otherwise, reenable it now so that we can update the content
		of toggles and combo's. */

	fEnabled = IsWindowEnabled(vhwndRibbon);

	if (!fEnabled)
		if (hwwdCur != hNil)
			if (!(PdodDoc(selCur.doc)->fLockForEdit
					&& !fDiffProp && !fDiffGray && !fDiffShowAll))
				EnableIconAndCombo(vhwndRibbon, IDLKSFONT, fTrue /*fEnable*/);
	/* any non-zero fields in either of the Diff chps mean that an lh property
		needs updating */

	if (!fDiffProp && !fDiffGray && !fDiffShowAll)
		goto CheckNoChild;   /* no changes detected */


	/*  Special case for the ASTERISK button - no CHP property for it. */
	/*   and it can never be gray unless hwwdCur==hNil. If fEnabled, then
		the ribbon has just lost the last doc, it is to be grayed, so
		update. If there is no doc, we will keep being called in idle.
		checking fEnabled lets us not flash the ASTERISK by updating it
		all the time
	*/

	if (fDiffShowAll)
		{
		if (hwwdCur==hNil)
			{
			if (fEnabled)
				UpdateLHProp(1, fTrue, IDLKSSHOWALL);
			}
		else
			UpdateLHProp ((*hwwdCur)->grpfvisi.fvisiShowAll,
					fFalse, IDLKSSHOWALL);

		grpfShowAllRibbon = grpfShowAllNew;
		}

	/* **********************  NOTE!!!!!  *************************
		At this point we are changing the display, and assume that the final
		blt of the chps will be done, so we do  not allow this portion
		if the routine to be interrupted
		************************************************************ */


	/* **********************  NOTE!!!!!  *************************
		Failure can occur in the dialog portion of the ribbon which will
		cause the ribbon to be destroyed by sdm. We therefore check
		vhwndRibbon after the font and font size updates, and bag out
		if the window has been destroyed.
		************************************************************ */

	/* font name */

	if (chpDiffProp.ftc || chpDiffGray.ftc)
		UpdateLHProp (selCur.chp.ftc, vchpGraySelCur.ftc,
				IDLKSFONT);

	if (vmerr.fKillRibbon)
		return;

	/* font point size */

	if (chpDiffProp.hps || chpDiffGray.hps)
		UpdateLHProp (selCur.chp.hps, vchpGraySelCur.hps,
				IDLKSPOINT);

	if (vmerr.fKillRibbon)
		return;

	/* toggle style character properties */

	if (chpDiffProp.fBold || chpDiffGray.fBold)
		UpdateLHProp (selCur.chp.fBold, vchpGraySelCur.fBold,
				IDLKSBOLD);

	if (chpDiffProp.fItalic || chpDiffGray.fItalic)
		UpdateLHProp (selCur.chp.fItalic, vchpGraySelCur.fItalic,
				IDLKSITALIC);

	if (chpDiffProp.fSmallCaps || chpDiffGray.fSmallCaps)
		UpdateLHProp (selCur.chp.fSmallCaps,
				vchpGraySelCur.fSmallCaps, IDLKSSMALLCAPS);

	/* Underlining codes:
	
		kul requires both the sprm and the val to determine. If kulNone,
		we send the lh id for single underline, and UpdateLHProp handles
		clearing out the buttons based on the value being kulNone
	*/

	Assert (selCur.chp.kul >= kulNone && selCur.chp.kul <= kulDotted);
	if (chpDiffProp.kul || chpDiffGray.kul)
		{
		int idDisplay;

		switch (selCur.chp.kul)
			{
		default:   /* handles kulNone, kulDotted. use Single id */
		case kulSingle:
			idDisplay = IDLKSUNDERLINE;
			break;
		case kulWord:
			idDisplay = IDLKSWORDULINE;
			break;
		case kulDouble:
			idDisplay = IDLKSDOUBLEULINE;

			break;
			}
		UpdateLHProp (selCur.chp.kul, vchpGraySelCur.kul, idDisplay);
		}

	/* Sub/super scripts
		If the hpsPos value is 0, then position is normal, so no button
		should be on. As with underline, pass an arbitrary id in the correct
		range (in this case IDLKSSUPERSCRIPT), then have UpdateLHProp
		figure out special handling for the 0 case. Test with 0x80 because
		is is an 8 bit signed field.
	*/

	if (chpDiffProp.hpsPos || chpDiffGray.hpsPos)
		UpdateLHProp (selCur.chp.hpsPos, vchpGraySelCur.hpsPos,
				(selCur.chp.hpsPos & 0x80) ? IDLKSSUBSCRIPT : IDLKSSUPERSCRIPT);

	/* copy in the selCur chps to ensure that LH chps are up to date with
	respect to the display */

	blt(&selCur.chp, &chpRibbon, cwCHP);
	blt(&vchpGraySelCur, &chpRibbonGray, cwCHP);
	/* grpfShowAllRibbon already updated above */

CheckNoChild:
	/* abandon focus if needed and disable window if there
		is no child window up */

	if (!vmerr.fKillRibbon && (hwwdCur == hNil || PdodDoc(selCur.doc)->fLockForEdit))
		EnableIconAndCombo(vhwndRibbon, IDLKSFONT, fFalse /*fEnable*/);

}  /* UpdateRibbon */



/* ****
*  Description: Updates a single ribbon property. Updates the
*     display only. Someone else will update the chps, usually by blt'ing
		in the chps in selCur.
** **** */

/* %%Function:UpdateLHProp   %%Owner:peterj */
UpdateLHProp (valProp, fGray, idDisplay)
int valProp;
BOOL fGray;
int idDisplay;
{
	int id;
	switch (idDisplay)
		{
	default:
		Assert (fFalse);
		break;

	case idNil:
		return;

	case IDLKSBOLD:
	case IDLKSITALIC:
	case IDLKSSMALLCAPS:
	case IDLKSSHOWALL:
		SetValIibbHwndIb(vhwndRibbon, idDisplay, fGray ? -1 : valProp);
		break;


	case IDLKSUNDERLINE:
	case IDLKSDOUBLEULINE:
	case IDLKSWORDULINE:
		/* if none, turn off all the underline codes */
		/* trick of setting id to idNil will set all buttons off or gray.
		
			Also note trick in this group that if gray is true, all items
			in the group are grayed. That is because we do not keep track
			of separate (e.g.) underline codes, but only see that the
			selection contained move than 1 underline state.
		*/

		id = ((valProp == kulNone) || fGray) ? idNil : idDisplay;

		SelectRadioIibb(vhwndRibbon, IDLKSUNDERLINE, IDLKSDOUBLEULINE, id,
				fGray);
		break;

	case IDLKSSUPERSCRIPT:
	case IDLKSSUBSCRIPT:
		/* if none, turn off all the position codes. If gray, gray them
			all
		*/

		id = ((valProp == 0) || fGray) ? idNil : idDisplay;

		SelectRadioIibb(vhwndRibbon, IDLKSSUPERSCRIPT, IDLKSSUBSCRIPT, id,
				fGray);
		break;

	case IDLKSFONT:
			{
			CHAR *pch;
			CHAR szFfn [LF_FACESIZE];

			if (fGray)
				pch = szEmpty;
			else
				{
				int ibst = IbstFontFromFtcDoc (valProp, selCur.doc);

				if (ibst == iNil)
					pch = szEmpty;
				else
					{
					CchCopySz( ((struct FFN *)
							PstFromSttb(vhsttbFont, ibst ))->szFfn,
							szFfn );
					pch = szFfn;
					}
				}

			SetIibbTextHwndIb(vhwndRibbon, IDLKSFONT, pch);
			break;
			}

	case IDLKSPOINT:
			{
			CHAR *pch;
			CHAR rgb[ichMaxNum];

			if (!fGray)
				{
				pch = rgb;
				CchHpsToPpch(valProp, &pch);
				*pch = '\0';
				}
			SetIibbTextHwndIb(vhwndRibbon, IDLKSPOINT, fGray ? szEmpty : rgb);
			break;
			}
		}

}


/* I B  P R O C  R I B B O N */
/* %%Function:IbProcRibbon  %%Owner:peterj */
IbProcRibbon(hwnd, ibm, iibb, wParam, lParam)
HWND hwnd;
WORD ibm;
WORD iibb;
WORD wParam;
LONG lParam;
{
	switch (ibm)
		{
	case ibmTerm:
	case ibmCancel:
	case ibmDblClk:
	case ibmInit:
		IbProcRibbonCmd(hwnd, ibm, iibb, wParam, lParam);
		break;

	case ibmSetItmFocus:
		/* don't redo if focus coming from list box or itself. wParam
			true if focus was at tmc + 1, which would be the list box
			of a combo or  from tmc, which was itself*/
		if (iibb == IDLKSPOINT && !wParam)
			InvalidateIibb(hwnd, IDLKSPOINT);
		break;

	case ibmCommand:
		/* convert id to a looks ilcd  and use the looks code
			to apply the property and update the ribbon.
			Note that the ilcd table in keys.h/cmd.c must be in the
			same order as the LH id's defined in ribbon.h */

		if (iibb >= IDLKSBOLD && iibb <= IDLKSSUBSCRIPT)
			{
			int ilcd;
			if (!FSetUndoBefore(bcmFormatting, uccFormat))
				{
				break;
				}
			/* terminate Block or Extend or Dyadic Move Mode
				if on. */

			SpecialSelModeEnd();
			DoLooks ((ilcd = IlcdFromIdLH (iibb)), fTrue /*fUpdate*/,
					hNil);

			if (vfRecording)
				RecordLooks(ilcd);
			}
#ifdef DEBUG
		else
			Assert (fFalse);
#endif   /* DEBUG */
		break;
		}
}

#ifdef WIN23
/************************************************************************
		Layer routines for Win 2/3 visuals
************************************************************************/

DrawRulerScale (hdc, preb, fErased)
HDC hdc;
struct REB * preb;
BOOL fErased;
{
	if (vsci.fWin3Visuals)
		DrawRulerScl3(hdc, preb, fErased);
	else
		DrawRulerScl2(hdc, preb, fErased);
}


BltRulerMark (hdc, preb, xp, imk, ms, fErase)
HDC hdc;
struct REB * preb;
int xp, imk, ms;
int fErase;	/* are we removing the mark? For color we have different ROPs */
{
	if (vsci.fWin3Visuals)
		BltRulerMark3 (hdc, preb, xp, imk, ms, fErase);
	else
		BltRulerMark2 (hdc, preb, xp, imk, ms);
}


/*************************************************************************
		Routines needed only for the Win2/3 version
*************************************************************************/


/* %%Function:DrawRulerScale   %%Owner:ricks */
DrawRulerScl3(hdc, preb, fErased)
HDC hdc;
struct REB * preb;
BOOL fErased;
{
	struct RSD ** hrsd = preb->hrsd;
	int ut = (*hrsd)->ut;
	int dxa = mputdxa [ut];
#ifdef REVIEW
	int xp;
	int iTick;
	int ypText = ypScale-dypMajorTick+(vsci.dypTmInternalLeading<2) - 1;
	CHAR *pch, sz [10];
	int xa;
#endif /* REVIEW */
	HBRUSH hbrOld;
 	HFONT hfontOld;

	if (hfontStatLine)
		hfontOld = SelectObject( hdc, hfontStatLine ); 

	/* hpen must have already been selected in */

	if (!fErased)
		{
		PatBlt(hdc, preb->xpLeft, 0, preb->xpRight-preb->xpLeft, ypScale, 
				PATCOPY);
		}

	hbrOld = SelectObject(hdc, vhbrBlack);

	/* scale line */
	/* we want to do the scale line first so we can overwrite the
		gray line above with the tick marks */
	PatBlt(hdc, preb->xpLeft, ypScale - 1, preb->xpRight-preb->xpLeft,
			vsci.dypBorder, PATCOPY);
	/* 3D effect */
	SelectObject(hdc, vhbrWhite);
	PatBlt(hdc, preb->xpLeft, ypScale, preb->xpRight-preb->xpLeft,
			vsci.dypBorder, PATCOPY);
	SelectObject(hdc, vhbrGray);
	PatBlt(hdc, preb->xpLeft, ypScale-2, preb->xpRight-preb->xpLeft,
			vsci.dypBorder, PATCOPY);
	SelectObject(hdc, vhbrBlack);

	/* ticks & labels */
#ifdef REVIEW
	for (iTick = DxaFromDxs (0/*dummy*/, -(*hrsd)->xpZero) / dxa - 1, 
			xa = iTick * dxa;
			(xp = XpMarkFromXaMark (xa, hrsd)) <= preb->xpRight
			&& xa >= xaMin;
			iTick++, xa += dxa)
		{
		/* draw major tick */
		PatBlt(hdc,  xp, ypText, vsci.dxpBorder, dypMajorTick,
				PATCOPY);
		/* label */
		pch = sz;
		CchIntToPpch (iTick * mputdn [ut], &pch);
		pch [1] = '\0';
		Assert (*pch <= sz + 8);
		TextOut (hdc, xp + vsci.dxpBorder + 1, ypText, (LPSTR) sz, pch - sz);

		/* minor & tiny ticks */
		DrawMinorTicks(hdc, xp, ypScale - 1, dypMinorTick,dxa,mputn1[ut]);
		DrawMinorTicks(hdc, xp, ypScale - 1, dypTinyTick, dxa,mputn23[ut]);
		}
#else
	DrawRulerTicks(hdc, hrsd, dxa, 0, ut, preb->xpRight, fTrue);
#endif /* REVIEW */
	/*****
		3D effect for ticks. I repeated the loop to avoid switching
		brushes constantly. 
	*****/
	SelectObject(hdc, vhbrWhite);
#ifdef REVIEW
	for (iTick = DxaFromDxs (0/*dummy*/, -(*hrsd)->xpZero) / dxa - 1, 
			xa = iTick * dxa;
			(xp = XpMarkFromXaMark (xa, hrsd)) <= preb->xpRight
			&& xa >= xaMin;
			iTick++, xa += dxa)
		{
		/* draw major tick */
		PatBlt(hdc,  xp + 1, ypText, vsci.dxpBorder, dypMajorTick,
				PATCOPY);
		/* minor & tiny ticks */
		DrawMinorTicks(hdc, xp + 1, ypScale - 1, dypMinorTick,dxa,mputn1[ut]);
		DrawMinorTicks(hdc, xp + 1, ypScale - 1, dypTinyTick, dxa,mputn23[ut]);
		}
#else
	DrawRulerTicks(hdc, hrsd, dxa, 1, ut, preb->xpRight, fFalse);
#endif /* REVIEW */
	SelectObject(hdc, vhbrGray);
#ifdef REVIEW
	for (iTick = DxaFromDxs (0/*dummy*/, -(*hrsd)->xpZero) / dxa - 1, 
			xa = iTick * dxa;
			(xp = XpMarkFromXaMark (xa, hrsd)) <= preb->xpRight
			&& xa >= xaMin;
			iTick++, xa += dxa)
		{
		/* draw major tick */
		PatBlt(hdc,  xp - 1, ypText, vsci.dxpBorder, dypMajorTick,
				PATCOPY);
		/* minor & tiny ticks */
		DrawMinorTicks(hdc, xp - 1, ypScale - 1, dypMinorTick,dxa,mputn1[ut]);
		DrawMinorTicks(hdc, xp - 1, ypScale - 1, dypTinyTick, dxa,mputn23[ut]);
		}
#else
	DrawRulerTicks(hdc, hrsd, dxa, -1, ut, preb->xpRight, fFalse);
#endif /* REVIEW */
	SelectObject(hdc, vhbrBlack);

	if (hfontStatLine && hfontOld)
		SelectObject( hdc, hfontOld ); 

	if (hbrOld != NULL)
		SelectObject(hdc, hbrOld);
}
/* %%Function:DrawRulerTicks   %%Owner:ricks */
DrawRulerTicks(hdc, hrsd, dxa, dxp, ut, xpRight, fDrawText)
HDC hdc;
struct RSD ** hrsd;
int dxa;
int dxp;
int ut;
int xpRight;
int fDrawText;
{
int iTick;
int xa;
int ypText = ypScale-dypMajorTick+(vsci.dypTmInternalLeading<2) - 1;
int xp;
CHAR *pch, sz [10];

	for (iTick = DxaFromDxs (0/*dummy*/, -(*hrsd)->xpZero) / dxa - 1, 
			xa = iTick * dxa;
			(xp = XpMarkFromXaMark (xa, hrsd)) <= xpRight
			&& xa >= xaMin;
			iTick++, xa += dxa)
		{
		/* draw major tick */
		PatBlt(hdc,  xp + dxp, ypText, vsci.dxpBorder, dypMajorTick,
				PATCOPY);
		if (fDrawText)
			{
			/* label */
			pch = sz;
			CchIntToPpch (iTick * mputdn [ut], &pch);
			pch [1] = '\0';
			Assert (*pch <= sz + 8);
			TextOut (hdc, xp + vsci.dxpBorder + 1, ypText, (LPSTR) sz, pch - sz);
			}
		/* minor & tiny ticks */
		DrawMinorTicks(hdc, xp + dxp, ypScale - 1, dypMinorTick,dxa,mputn1[ut]);
		DrawMinorTicks(hdc, xp + dxp, ypScale - 1, dypTinyTick, dxa,mputn23[ut]);
		}
}
/* %%Function:BltRulerMark   %%Owner:ricks */
BltRulerMark3 (hdc, preb, xp, imk, ms, fErase)
HDC hdc;
struct REB * preb;
int xp, imk, ms;
int fErase;	/* are we removing the mark? For color we have different ROPs */
{
	struct BMI *pbmi = &mpidrbbmi [idrbRulerMarks3];
	int dxp = pbmi->dxpEach;
	int ddxp = dxp / 2;
	struct MDCD *pmdcd;

	long rop;
	if (ms == msNormal)
		rop = (fErase) ? ROP_DPSnao : SRCAND;
	else if (ms == msInvert)
		rop = ROP_DSnx;
	else
		rop = ROP_DSnx;	/* REVIEW */

	Assert (preb->hrsd != hNil);
	Assert( dxp != 0 );

	if (xp + ddxp < preb->xpLeft || xp - ddxp > preb->xpRight)
		/* none of the mark is visible, don't bother */
		return;

	if ((pmdcd = PmdcdCacheIdrb( idrbRulerMarks3, hdc )) != NULL)
		{
		int xpSrc;

		xpSrc = imk * pbmi->dxpEach;

		BitBlt (hdc, (xp - ddxp), ypMark, dxp, pbmi->dyp, pmdcd->hdc,
				xpSrc, 0, rop);
		}
}

#endif /* WIN23 */
