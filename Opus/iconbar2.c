/* I C O N B A R 2 . C */
/* Creation/destruction routines for iconbars */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "resource.h"
#include "props.h"
#include "heap.h"
#include "disp.h"
#include "error.h"
#include "format.h"
#include "style.h"
#include "prm.h"
#include "doc.h"
#include "sel.h"
#include "screen.h"
#include "wininfo.h"
#include "resource.h"
#include "prompt.h"
#include "keys.h"
#include "ch.h"
#define USEBCM
#include "cmd.h"
#include "cmdlook.h"
#define SZCLS
#define RULER
#define REPEAT
#include "ruler.h"
#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "help.h"
#include "debug.h"
#include "core.h"
#include "disp.h"
#include "rareflag.h"

#define MPIBITGRPF
#include "iconbar.h"
#ifdef WIN23
#include "dac.h"	/* so we can hack sdm's font sizes */
#endif /* WIN23 */

#define RULERIB
#define RIBBON
#include "ibdefs.h"

extern char szClsRulerMark[];
extern char szClsIconBar[];
extern struct PREF  vpref;
extern struct SCI   vsci;
extern HANDLE       vhInstance;
extern CHAR         szEmpty[];
extern HWND         vhwndApp;
extern struct MERR  vmerr;
#ifdef WIN23
extern struct BMI *mpidrbbmi;
#else
extern struct BMI mpidrbbmi[];
#endif /* WIN23 */
extern struct WWD  **mpwwhwwd[];
extern HWND             vhwndRibbon;
extern HWND   vhwndApp;
extern BOOL   vfRecording;
extern HANDLE vhInstance;
extern CHAR   szEmpty[];
extern int    wwCur;
extern int    wwMac;
extern struct WWD ** hwwdCur;
extern struct MWD ** hmwdCur;
extern struct MWD ** mpmwhmwd[];
extern struct SEL    selCur;
extern struct CA     caPara;
extern struct SCI    vsci;
extern struct FTI    vfti;
extern struct FLI    vfli;
extern struct UAB    vuab;
extern struct PREF   vpref;
extern struct RULSS  vrulss;
extern struct WWD    ** mpwwhwwd[];
extern int           wwCreating;
extern RRF	     	 rrf;
extern int           vdocStsh;
extern HWND			 vhwndCBT;
#ifdef WIN23
extern int          vdbmgDevice;
#endif /* WIN23 */

#ifdef WIN23
extern FARPROC		lpfnSdmWndProc;
extern FARPROC		lpfnIconDlgWndProc;
#endif /* WIN23 */

extern struct REB * PrebInitPrebHrsd ();
extern int ParseStyle();
InvalStyle();
HDLG HdlgHibsDlgCur(HIBS);

/* ribbon/ruler globals */

int     ypScaleBottom = 0;
int     ypMarkBottom;
int     dypMark;
int     dypMajorTick;
int     dypMinorTick;
int     dypTinyTick;

struct REB * vpreb = NULL;

#ifdef DEBUG
BOOL fCreatingIB = fFalse;
#endif /* DEBUG */


/* Global rgtmw's for ruler and ribbon */
CHAR *rgtmwRibbon[CbRuntimeCtm(ctmDltRibbon)];
/* this one is big - 5 * 88 bytes! */
#define iRulerMax  (5)   /* by peter's declaration 9/7/89 */
CHAR *rgtmwRuler[iRulerMax][CbRuntimeCtm(ctmDltRuler)];
int vgrpfiTmw = 0;  /* bits tell which ruler tmw's are used */

/* %%Function:HwndCreateIconBar %%Owner:peterj */
HWND HwndCreateIconBar(pibd, pdlt, hwndParent, prc, ptmw)
struct IBD *pibd;
DLT *pdlt;
HWND hwndParent;
struct RC *prc;
CHAR *ptmw;
{
	HIBS hibs;
	int iibbSys;
	int dypIconbar;
	HWND hwnd;
#ifdef WIN23
	/*****
	When running under Win3, we can't use the win 3 system font height for
	either the new visuals or the old. If we are running new visuals, then
	dxpFakeTmWidth and dypFakeTmheight are set up to be the PM system font
	metrics. If we are running old visuals, they are set up to be the Win 2
	system font metrics.
	*****/
	int dySysFont;
	int dxSysFont;
	int dxpTmWidth = vsci.fWin3 ? vsci.dxpFakeTmWidth : vsci.dxpTmWidth;
	int dypTmHeight = vsci.fWin3 ? vsci.dypFakeTmHeight : vsci.dypTmHeight;
#endif /* WIN23 */

	/* walk rgibid and determine size of ibs needed, create ibs on heap */
		{
		int cbIbs;
		int ibit;
		int cibbUsr = 0, cibbSys = 0;
		struct IBID *pibid;
		struct IBS *pibs;

		for (pibid = pibd->rgibid; (ibit = pibid->ibit) != ibitEnd; pibid++)
			if (mpibitgrpf[ibit].fHasIbb)
				if (pibid->iibb != iibbNone)
					cibbUsr++;
				else
					cibbSys++;

		cbIbs = cbIBSBase + ((cibbUsr+cibbSys) * cbIBB);
		if ((hibs = HAllocateCb(cbIbs)) == hNil)
			return hNil;
		pibs = *hibs;
		SetBytes(pibs, 0, cbIbs);
		pibs->iibbDlg = iibbNil;
		pibs->iibbMac = cibbUsr+cibbSys;
		pibs->iibbMinSys = iibbSys = cibbUsr;
		pibs->pfn = pibd->pfn;
		pibs->ibc = pibd->ibc;
		}

	/* create iconbar of correct size */

	Debug(fCreatingIB = fTrue); /* for the sake of SDM mem failure asserts */

	prc->xpLeft -= vsci.dxpBorder;
	prc->xpRight += vsci.dxpBorder;
	prc->ypTop -= vsci.dypBorder;

#ifdef WIN23
	dypIconbar = UMultDiv(pibd->dy, dypTmHeight, 8);
#else
	dypIconbar = UMultDiv(pibd->dy, vsci.dypTmHeight, 8);
#endif /* WIN23 */
	/* HACK!!!!! */
	if (pibd->fAddOnePixel)
		dypIconbar++;

	(*hibs)->hwnd = hwnd = CreateWindow((LPSTR)szClsIconBar, 
			(LPSTR)szEmpty,
			WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			prc->xpLeft,
			prc->ypTop,
			prc->xpRight - prc->xpLeft,
			dypIconbar,
			hwndParent,
			(HMENU)NULL, vhInstance, (LPSTR)NULL);

	if (hwnd == NULL)
		goto LFail;

	SetWindowWord(hwnd, IDWIBHIBS, hibs);

	/* Tell CBT about this window; also give it a unique id to distinguish
		the different iconbars.  You may ask the obvious question:  Why
		not send the id # in the (otherwise unused) lParam of the
		CBTNEWWND message?  Well, I asked that too, and was told that it
		has to be this way because the CBTWNDID message was invented as an
		after-thought and it was harder for Whimper to go back and change
		their CBTNEWWND calls (apparently they didn't know the id # yet at
		that point) than to add a second message later on.  So we have to
		do it their way  (rp - 9/1/89) */

	if (vhwndCBT)
		{
		SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		SendMessage(vhwndCBT, WM_CBTWNDID, hwnd, (LONG) pibd->ibc);
		}

	/* create children */
		{
		int dxp, dyp;
		int idrb = 0, idcb;
		int iibb, ibit;
		struct IBID *pibid;
		struct IBB *pibb;

		for (pibid = pibd->rgibid; (ibit = pibid->ibit) != ibitEnd; pibid++)
			{
			if (mpibitgrpf[ibit].fHasIbb)
				{
				if ((iibb = pibid->iibb) == iibbNone)
					iibb = iibbSys++;

				Assert(iibb < (*hibs)->iibbMac);
				pibb = PibbFromHibsIibb(hibs, iibb);
				pibb->ibit = ibit;
				pibb->bcm = pibid->bcm;

				if (mpibitgrpf[ibit].fHasCoord)
					{
#ifdef WIN23
					pibb->xpLeft = UMultDiv(pibid->x, dxpTmWidth, 4);
					pibb->ypTop = UMultDiv(pibid->y, dypTmHeight, 8);
					dxp = UMultDiv(pibid->dx, dxpTmWidth, 4);
					dyp = UMultDiv(pibid->dy, dypTmHeight, 8);
#else
					pibb->xpLeft = UMultDiv(pibid->x, vsci.dxpTmWidth, 4);
					pibb->ypTop = UMultDiv(pibid->y, vsci.dypTmHeight, 8);
					dxp = UMultDiv(pibid->dx, vsci.dxpTmWidth, 4);
					dyp = UMultDiv(pibid->dy, vsci.dypTmHeight, 8);
#endif /* WIN23 */
					pibb->xpRight = pibb->xpLeft + dxp;
					pibb->ypBottom = pibb->ypTop + dyp;
					}
				}

			switch (ibit)
				{
			case ibitBitmap:
				idrb = pibid->x;
				idcb = pibid->y;
				break;

			case ibitCustomWnd:
				{
				HWND hwndCus = CreateWindow((LPSTR)szClsRulerMark,
						(LPSTR)szEmpty, 
						WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
						pibb->xpLeft, pibb->ypTop, dxp, dyp, 
						hwnd, (HMENU)NULL, vhInstance, 0L);
				Assert(pibd->ibc == ibcRuler);
				if (hwndCus == NULL)
					goto LFail;
				PibbFromHibsIibb(hibs, iibb)->hwndCus = hwndCus;
				break;
				}

			case ibitDialog:
				/* create a modeless, child dialog */
				{
				HDLT hdlt = HAllocateCb(pibid->dx/*cbDlt*/);
				HCAB hcab = HcabAlloc(pibid->dy/*cabi*/);
				HDLG hdlg;
#ifdef WIN23
				int xpDlg = UMultDiv(pibid->x, dxpTmWidth, 4)
						+ vsci.dxpBorder;
				int ypDlg = UMultDiv(pibid->y, dypTmHeight, 8)
						+ vsci.dypBorder;
#else
				int xpDlg = UMultDiv(pibid->x, vsci.dxpTmWidth, 4)
					+ vsci.dxpBorder;
				int ypDlg = UMultDiv(pibid->y, vsci.dypTmHeight, 8)
					+ vsci.dypBorder;
#endif /* WIN23 */
				DLI dli;

				if (hdlt == hNil || hcab == hNil)
					{
LDialogFail:
					FreeH(hdlt);
					if (hcab != hNil)
						FreeCab(hcab);
					goto LFail;
					}

				bltb(pdlt, *hdlt, pibid->dx);
				SetBytes(&dli, 0, sizeof (DLI));
				dli.hwnd = hwndParent;
				dli.dx = prc->xpLeft + xpDlg;
				dli.dy = prc->ypTop + ypDlg;
				dli.fdlg = fdlgInvisible | fdlgFedt;
				dli.wRef = hibs;
				Assert (ptmw != NULL);
				dli.rgb = ptmw;

				if (FIsDlgDying() || (!vmerr.fSdmInit && !FAssureSdmInit()))
					goto LDialogFail;
#ifdef WIN23
				/****
				Ugly Kludge. If we are under Win 3, but not running Win 3
				visuals, we have a problem. We have ribbon and ruler dlgs
				sized for the Win 3 font and the Win 2 font, but the Win 3
				dlgs wont fit into the Win 2 visuals. The Win 2 dlgs scaled
				with the Win 3 fonts wont fit either. So I fake out SDM
				and give it the Win 2 font sizes just while creating these
				dialogs, and then fix it up.
				*****/
				if (vsci.fWin3 && !vsci.fWin3Visuals)
					{
					dySysFont = dac.dySysFontChar;
					dac.dySysFontChar = vsci.dypFakeTmHeight;
					dxSysFont = dac.dxSysFontChar;
					dac.dxSysFontChar = vsci.dxpFakeTmWidth;
					}
#endif /* WIN23 */
				if ((hdlg = HdlgStartDlg(hdlt,hcab,&dli)) == hNil)
					goto LDialogFail;

#ifdef WIN23
				if (vsci.fWin3 && !vsci.fWin3Visuals)
					{
					dac.dySysFontChar = dySysFont;
					dac.dxSysFontChar = dxSysFont;
					}
#endif /* WIN23 */
				(*hibs)->iibbDlg = iibb;
				pibb = PibbFromHibsIibb(hibs, iibb);
#ifdef WIN23
				/* can't assume pdlt->rec is (0,0) */
				pibb->xpDlg = xpDlg + pdlt->rec.x;
				pibb->ypDlg = ypDlg + pdlt->rec.y;
#else
				pibb->xpDlg = xpDlg;
				pibb->ypDlg = ypDlg;
#endif /* WIN23 */
				pibb->hdlg = hdlg;
				pibb->hdlt = hdlt;
				pibb->hcab = hcab;

				SetWindowPos(hwnd, HwndFromDlg(hdlg), 0, 0, 0, 0, 
						SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
#ifdef WIN23
				/****
				Subclass the sdm dialog window so we can trap
				erase backround messages
				****/
				if (vsci.fWin3Visuals)
					{
					lpfnSdmWndProc = (FARPROC) GetWindowLong(HwndFromDlg(hdlg), GWL_WNDPROC);
					SetWindowLong(HwndFromDlg(hdlg), GWL_WNDPROC, (LONG) lpfnIconDlgWndProc);
					}
#endif /* WIN23 */

				break;
				}

			case ibitDlgItem:
				pibb->tmc = pibid->y;
				break;

			case ibitToggleText:
			case ibitText:
				{
				CHAR **hsz = HszCreate(pibid->sz);
				pibb = &(*hibs)->rgibb[iibb];
				/* ok to store hNil */
				pibb->hsz = hsz;
				break;
				}

			case ibitToggleBmp:
				{
				struct BMI *pbmi = &mpidrbbmi[idrb];
				Assert(idrb > 0);
				Assert(pbmi->idcbMax != 0);

				/* setup bitmap size */
#ifdef WIN23
				if (vsci.fWin3Visuals )
					{
					if (pibb->fAddOnePixel = pibid->fAddOnePixel)
						{	/***
							ugly kludges to get EGA +/= buttons right
							only they should have fAddOnePixel set
							***/
						if (vdbmgDevice == dbmgEGA3)
							{
							if (iibb == IDLKSSUPERSCRIPT)
								{
								dyp++;
								}
							else
								{
								dyp++;
								pibb->ypTop++;
								pibb->ypBottom++;
								}

							}
						}
					pibb->fDontEraseTopLine = pibid->fDontEraseTopLine;
					pibb->fLatch = pibid->fLatch;
					pibb->fComposite = pibid->fComposite;
					if ( pbmi->dyp == 0)
						{	/* else we've done it already */
						pbmi->dyp = dyp - 2 * cPixBorder + pibb->fAddOnePixel; /* Review */
						pbmi->dxpEach = dxp - 2 * cPixBorder;
						pbmi->dxp = pbmi->dxpEach * pbmi->idcbMax;
						if (pibd->fGrayable)
							{	/*set up the gray version as well */
							struct BMI *pbmi2 = &mpidrbbmi[idrb + 1];
							pbmi2->dxp = pbmi->dxp;
							pbmi2->dyp = pbmi->dyp;
							pbmi2->dxpEach = pbmi->dxpEach;
							}
						}
					}
				else
					{
					pbmi->dxpEach = dxp - (vsci.dxpBorder << 2);
					pbmi->dxp = pbmi->dxpEach * pbmi->idcbMax;
					pbmi->dyp = dyp - (vsci.dypBorder << 2);
					}
#else
				pbmi->dxpEach = dxp - (vsci.dxpBorder << 2);
				pbmi->dxp = pbmi->dxpEach * pbmi->idcbMax;
				pbmi->dyp = dyp - (vsci.dypBorder << 2);
#endif /* WIN23 */

				pibb->idrb = idrb;
				pibb->idcb = idcb++;
				break;
				}

#ifdef DEBUG
			default: 
				Assert(fFalse); 
				break;
#endif /* DEBUG */
				}
			}
		}

	CallIbProc(hibs, ibmCreate, iibbNil, 0, 0L);
	if (!pibd->fInvisible)
		{
		ShowWindow(hwnd, SHOW_OPENWINDOW);
		UpdateWindow(hwnd);
		}

	Debug(fCreatingIB = fFalse);

	return hwnd;

LFail:
	Debug(fCreatingIB = fFalse);

	DestroyHibs(hibs);
	return NULL;
}



/* D E S T R O Y  H I B S */

/* %%Function:DestroyHibs %%Owner:peterj */
DestroyHibs(hibs)
HIBS hibs;
{
	HWND hwnd;
	struct IBB *pibb;
	int iibb, iibbMac;

	if (hibs == hNil)
		return;

	if ((hwnd = (*hibs)->hwnd) != NULL)
		{
		DestroyWindow(hwnd); /* recursively calls DestroyHibs() */
		return;
		}

	pibb = &(*hibs)->rgibb[0];
	iibbMac = (*hibs)->iibbMac;

	for (iibb = 0; iibb < iibbMac; iibb++, pibb++)
		{
		switch (pibb->ibit)
			{
		case ibitToggleText:
		case ibitText:
			FreeH(pibb->hsz);
			break;

		case ibitDialog:
				{
				HDLG hdlg = pibb->hdlg;
				HCAB hcab = pibb->hcab;
				HDLT hdlt = pibb->hdlt;

				if (hdlg != hNil)
					{
					HdlgSetCurDlg(hdlg);
					EndDlg(tmcCancel);    /* ok to call EndDlg here */

					AssertDo(FFreeDlg());
					}

				if (hcab != hNil)
					FreeCab(hcab);
				FreeH(hdlt);
				break;
				}
			}
		}

	FreeH(hibs);
}


/* H I D E  I I B B  H W N D  I B */
/* %%Function:HideIibbHwndIb %%Owner:peterj */
HideIibbHwndIb(hwnd, iibb, fHidden)
HWND hwnd;
int iibb;
BOOL fHidden;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);

	switch (pibb->ibit)
		{
	case ibitToggleText:
	case ibitToggleBmp:
	case ibitText:
		pibb->fHidden = fHidden;
		pibb->fDisabled = fHidden;
		UpdateIibb(hibs, iibb, fTrue);
		break;

#ifdef DEBUG
	default: 
		Assert(fFalse); 
		break;
		/* LATER: hide dialog item */
#endif /* DEBUG */
		}
}


/* D I S A B L E  I I B B  H W N D  I B */
/* %%Function:DisableIibbHwndIb %%Owner:peterj */
DisableIibbHwndIb(hwnd, iibb, fDisable)
HWND hwnd;
int iibb;
BOOL fDisable;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	HDLG hdlg;

	switch (pibb->ibit)
		{
	case ibitDlgItem:
			{
			TMC tmc = pibb->tmc;
			pibb->fDisabled = fDisable;
			hdlg = HdlgHibsDlgCur(hibs);
			EnableTmc(tmc, !fDisable);
			HdlgSetCurDlg(hdlg);
			break;
			}

	case ibitToggleText:
	case ibitToggleBmp:
	case ibitText:
		pibb->fDisabled = fDisable;
		break;

#ifdef DEBUG
	default: 
		Assert(fFalse); 
		break;
#endif /* DEBUG */
		}
}


/* D I S A B L E  IBDLG H W N D  I B */
/* enable/disable the dialog that iibb is a child of */
/* hack for ribbon, ruler, and context sensitive help */

/* %%Function:DisableIbdlgHwndIb %%Owner:bobz */
DisableIbdlgHwndIb(hwnd, iibb, fDisable)
HWND hwnd;
int iibb;
BOOL fDisable;
{
	HWND hwndDlg;
	HDLG hdlg;
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);

	TMC tmc = pibb->tmc;
	hdlg = HdlgHibsDlgCur(hibs);
	if ((hwndDlg = HwndOfTmc(tmc)) != hNil)
        if ((hwndDlg = GetParent(hwndDlg)) != hNil)  /* window of the dialog itself */
	        EnableWindow(hwndDlg, !fDisable);
	HdlgSetCurDlg(hdlg);

}


/* E N A B L E  I C O N  A N D  C O M B O */
/* %%Function:EnableIconAndCombo %%Owner:chic */
/* Enable or disable the iconbar and its combo boxes, the combo boxes
have to be handled separately because it is not a child of the iconbar,
the combo boxes have it's own parent, all we need is one id of the combo
box and from there DisableIbdlgHwndIb can fetch the parent window to
enable or disable.
So far, only ruler and ribbon have combo boxes 
*/
EnableIconAndCombo(hwnd, iibb, fEnable)
HWND hwnd; /* usually vhwndRibbon, or hwndRuler */
int iibb; /* id of combo box from which we can fetch it's real parent */
BOOL fEnable;
{

	/* handle the dialog portion separately */
	DisableIbdlgHwndIb(hwnd, iibb, !fEnable /*fDisable*/);
	EnableWindow(hwnd, fEnable);
}


/* S e t E n d  I b  d l g   */
/* set flags to take down ribbon or ruler based on who owns hdlg.
   Called from FRetrySdmError.
*/

/* %%FunctionFSetEndIbdlg %%Owner:bobz */
SetEndIbdlg(hdlg)
HDLG hdlg;
{
	HIBS hibs;

	Assert(!FModalDlg(hdlg));

	/* is it the ribbon? */
	if (vhwndRibbon != NULL)
		{
		hibs = HibsFromHwndIb(vhwndRibbon);
		if (hdlg == HdlgHibs(hibs))
			{
			vmerr.fKillRibbon = fTrue;
			return;
			}
		}

	/* ruler? */
	/* I claim, without asserting, that only the current ruler will fail
		at the sdm level. The failures will occur in allocating; non-current
		rulers may repaint, but should not update.
	
		If I am wrong, we will assert at the end of this routine. bz
	*/

	if (hmwdCur != hNil && (*hmwdCur)->hwndRuler != NULL)
		{
		hibs = HibsFromHwndIb((*hmwdCur)->hwndRuler);
		if (hdlg == HdlgHibs(hibs))
			{
			vmerr.fKillRuler = fTrue;
			return;
			}
		}

	Assert (fCreatingIB);   /* or else we are confused */
}



/*  E n d  I b  d l g   */
/* take down ribbon or ruler based on vmerr flags. 
   Called from TopofMainLoop.
*/

/* %%Function:EndIbdlg %%Owner:bobz */
EndIbdlg()
{
	/* give focus back to app */
	TermCurIBDlg(fFalse);  /* escapes without doing action */

	if (vmerr.fKillRibbon)
		{
		vmerr.fKillRibbon = fFalse;
		AssertDo(FTurnRibbonFOn(fFalse /*fOn*/,
				fTrue /*fAdjust*/, fTrue /*fUpdate*/));
		}

	/* ruler */

	/* I claim, without asserting, that only the current ruler will fail
		at the sdm level. The failures will occur in allocating; non-current
		rulers may repains, but should not update.
	
		If I am wrong, we have asserted in FSetEndIbdlg. bz
	*/

	if (vmerr.fKillRuler)
		{
		vmerr.fKillRuler = fFalse;
		ShowRulerWwCur (fFalse /* fCreate */, fFalse /* fCmd */);
		}
}





/* S E T  W R E F  H W N D  I B */
/* %%Function:SetWRefHwndIb %%Owner:peterj */
SetWRefHwndIb(hwnd, wRef)
HWND hwnd;
WORD wRef;
{
	AssertH(HibsFromHwndIb(hwnd));
	(*HibsFromHwndIb(hwnd))->wRef = wRef;
}


/* H W N D  C U S  F R O M  H W N D  I I B B */
/* %%Function:HwndCusFromHwndIibb %%Owner:peterj */
HWND HwndCusFromHwndIibb(hwnd, iibb)
HWND hwnd;
int iibb;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	Assert(pibb->ibit = ibitCustomWnd);
	return pibb->hwndCus;
}


/* G E T  H W N D  P A R E N T  R C */
/* %%Function:GetHwndParentRc %%Owner:peterj */
GetHwndParentRc(hwnd, prc)
HWND hwnd;
struct RC *prc;
{
	HWND hwndParent = GetParent(hwnd);
	GetWindowRect(hwnd, (LPRECT)prc);
	ScreenToClient(hwndParent,(LPPOINT)&prc->ptTopLeft);
	ScreenToClient(hwndParent,(LPPOINT)&prc->ptBottomRight);
}


/* I N V A L I D A T E  I I B B */
/* %%Function:InvalidateIibb %%Owner:peterj */
InvalidateIibb(hwnd, iibb)
HWND hwnd;
int iibb;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	TMC tmc = pibb->tmc;
	HDLG hdlg;

	Assert(pibb->ibit == ibitDlgItem);
	hdlg = HdlgHibsDlgCur(hibs);
	RedisplayTmc(tmc);
	HdlgSetCurDlg(hdlg);
}




/* RULER and RIBBON creation code */



/* RULER CREATION & DESTRUCTION */


/*  CmdRuler
	this function is called in response to the selection of the
	RULER menu item.  it toggles the display state of the ruler.
*/
/* %%Function:CmdRuler %%Owner:peterj */
CMD CmdRuler (pcmb)
CMB *pcmb;
{
	Assert(hmwdCur != hNil && hwwdCur != hNil);
	StartLongOp();
	FAbortNewestCmg(cmgRulerInit, fTrue/*fLoad*/, fFalse);
	ShowRulerWwCur ((*hmwdCur)->hwndRuler == NULL, fTrue/*fCmd*/);

	/*  default is now what ruler is (remember ShowRuler may have failed) */
	vpref.fRuler = ((*hmwdCur)->hwndRuler != NULL);
	EndLongOp(fFalse/*fAll*/);

	return cmdOK;
}



/*  ShowRulerWwCur
	creates or destroys the ruler for the current child ruler.
	Adjusts the windows.
*/
/* %%Function:ShowRulerWwCur %%Owner:peterj */
ShowRulerWwCur (fCreate, fCmd)
int fCreate, fCmd;
{
	int wwRuler = (*hmwdCur)->wwRuler; /* get it before it is cleared if !fCreate */
	int dyp;
	struct RC rcPane;

	Assert (hmwdCur != hNil);

	if (fCreate && (*hmwdCur)->hwndRuler == NULL)
		{
		if (!FCreateRuler (wwCur, fCmd))
			return;
		wwRuler = (*hmwdCur)->wwRuler; /* get it after it is set */
		}
	else  if (!fCreate && (*hmwdCur)->hwndRuler != NULL)
	/* try to destroy the ruler */
		AssertDo(FDestroyRuler(hmwdCur));
	else
		return;

	Assert(wwRuler >= wwDocMin && wwRuler < wwMac);
	dyp = vsci.dypRuler - vsci.dypBorder;
	AdjustPane(mpwwhwwd[wwRuler], fCreate ? dyp : -dyp);

	if (fCreate)
		{
		FAbortNewestCmg(cmgRulerDisp, fTrue/*fLoad*/, fFalse);
		/* Initializes the ruler and does a ShowWindow */
		DisplayRuler ();
		}
}


/*  DisplayRuler
	Initializes the ruler for hmwdCur and then performs the necessary
	convolutions to do a ShowWindow on the ruler.
*/
/* %%Function:DisplayRuler %%Owner:peterj */
DisplayRuler ()

{
	HWND hwndRuler;

	if (hmwdCur == hNil || (hwndRuler = (*hmwdCur)->hwndRuler) == NULL)
		return;

	/*  initialize the settings of the ruler based on selCur */
	UpdateRuler (hmwdCur, fTrue /*fInit*/, rkNormal, fFalse /*fAbortOK*/);

	ShowWindow(hwndRuler, SHOW_OPENWINDOW);
	UpdateWindow(hwndRuler);
}





/*  FCreateRuler
	this function creates the ruler window if it does not already
	exist. and allocates and initializes everything for it.

	returns true iff the ruler was added.
*/

/* %%Function:FCreateRuler %%Owner:peterj */
FCreateRuler(ww, fCmd)
int ww, fCmd;
{
	HWND hwndRuler = NULL;
	struct RSD **hrsd = hNil;
	struct RC rc;
	int mw = PwwdWw(ww)->mw;
	struct MWD **hmwd = mpmwhmwd[mw];
#ifdef WIN23
	CHAR ibd [sizeof(ibdRuler2) > sizeof(ibdRuler3) ? sizeof(ibdRuler2) : sizeof(ibdRuler3) ];
	CHAR dlt [sizeof(dltRuler3) > sizeof(dltRuler) ? sizeof(dltRuler3) : sizeof(dltRuler)];
#else
	CHAR ibd [sizeof(ibdRuler)];
	CHAR dlt [sizeof(dltRuler)];
#endif /* WIN23 */
	int i;

	Assert (hmwd != hNil);

	if ((*hmwd)->hwndRuler != NULL)
		{/* ruler is already up or not allowed */
		return fFalse;
		}

	Assert ((*hmwd)->hwnd != NULL);

	if (!FWwCanSpareDyp(ww, vsci.dypRuler))
		{  /* mwd is too small to create ruler */
		if (fCmd)
			ErrorEid(eidWindowTooSmall, "FCreateRuler" );
		return fFalse;
		}

	GetHwndParentRc(PwwdWw(ww)->hwnd, &rc);

	if (wwCreating != wwNil && PwwdWw(ww)->hwndIconBar)
		/* this can only be the header/footer iconbar */
		rc.ypTop += vsci.dypIconBar - (vsci.dypBorder << 1);


#ifdef WIN23
	/* REVIEW */
	if (vsci.fWin3Visuals)
		BltDlt(dltRuler3, dlt);	/* Dialog based on Win 3 font */
	else
		BltDlt(dltRuler, dlt);  /* Dialog based on Win 2 font */
	if (vsci.fWin3Visuals)
		BltIbd(ibdRuler3, ibd); /* Iconbar based on Win 3 font */
	else
		BltIbd(ibdRuler2, ibd); /* Iconbar based on Win 2 font */
#else
	BltDlt(dltRuler, dlt);
	BltIbd(ibdRuler, ibd);
#endif /* WIN23 */

	/* Create the window */
	/* find an available tmw (need 1 per ruler) */
	for (i = 0; i < iRulerMax; i++)
		if (!(vgrpfiTmw & 1 << i))
			break;
	if (i >= iRulerMax) /* will happen opening > 5 rulers */
		goto ErrorExit;

	hwndRuler = HwndCreateIconBar(ibd, dlt, (*hmwd)->hwnd, &rc, rgtmwRuler[i]);

	if (hwndRuler == NULL)
		/* creation failed */
		goto ErrorExit;

	/* allocate state struct */
	if ((hrsd = (struct RSD **)HAllocateCb(sizeof (struct RSD))) == hNil)
		/* allocation failed */
		goto ErrorExit;

	/* initialize */
	SetBytes (*hrsd, 0, sizeof (struct RSD));

	SetWRefHwndIb(hwndRuler, hrsd);

	/* set the ruler handle */
	(*hmwd)->hwndRuler = hwndRuler;
	(*hmwd)->wwRuler = ww;
	(*hrsd)->hwndRuler = hwndRuler;
	(*hrsd)->hwndMark = HwndCusFromHwndIibb(hwndRuler, idRulMark);

	if (ypScaleBottom == 0 && !FInitRulerData((*hrsd)->hwndMark, hwndRuler))
		goto ErrorExit;

	PositionMarkWindow(hwndRuler);

	/*  this is for an assertion that the ruler gets UpdateRuler'd
		before any attempt is made to paint it. */
	Debug (  (*hrsd)->jcTabCur = -1  );

	/* ruler is updated and ShowWindow'd by caller */

	(*hrsd)->irgTmw = i;
	vgrpfiTmw |= (1 << i);

	/* normal exit */
	return fTrue;


ErrorExit:
	/* error on window creation or initialization, destroy window */

	if (hwndRuler != NULL)
		DestroyWindow (hwndRuler);   /* frees hrsd! */
	(*hmwd)->hwndRuler = NULL;
	(*hmwd)->wwRuler = wwNil;

	if (fCmd)
		ErrorNoMemory(eidNoMemRuler);

	return fFalse;

}





/*  FDestroyRuler
	this function destroys the ruler for hmwd if there is one and then
	frees all allocated memory.

	returns true if the destruction did something.
*/

/* %%Function:FDestroyRuler %%Owner:peterj */
FDestroyRuler (hmwd)
struct MWD ** hmwd;
{
	HWND hwndRuler;
	struct RSD **hrsd;

	Assert (hmwd != hNil);

	if ((hwndRuler = (*hmwd)->hwndRuler) == NULL)
		{/* no ruler */
		Assert (fFalse);
		return fFalse;
		}

	/* free up slot in rgtmwRuler */
	hrsd = HrsdFromHwndRuler(hwndRuler);
	vgrpfiTmw &= ~(1 << (*hrsd)->irgTmw);

	/* destroy the ruler window -- hrsd destroyed by destroy window */
	DestroyWindow ((*hmwd)->hwndRuler);

	/* indicate no ruler */
	(*hmwd)->hwndRuler = NULL;
	(*hmwd)->wwRuler = wwNil;

	return fTrue;
}


/* F  I N I T  R U L E R  D A T A */
/* %%Function:FInitRulerData %%Owner:peterj */
FInitRulerData(hwndMark, hwndRuler)
HWND hwndMark, hwndRuler;
{
#ifdef WIN23
	struct BMI *pbmi = & mpidrbbmi [vsci.fWin3Visuals ?
		idrbRulerMarks3 : idrbRulerMarks2 ];
#else
	struct BMI *pbmi = & mpidrbbmi [idrbRulerMarks];
#endif /* WIN23 */
	struct RC rcRuler, rcMark;
#ifdef WIN23
	int dypTmHeight = (vsci.fWin3) ? vsci.dypFakeTmHeight : vsci.dypTmHeight;
#endif /* WIN23 */
	Assert(ypScaleBottom == 0);

	GetClientRect(hwndRuler, &rcRuler);
	GetHwndParentRc(hwndMark, &rcMark);

#ifdef WIN23
	ypScaleBottom = NMultDiv (dyScale, dypTmHeight, 8);
#else
	ypScaleBottom = NMultDiv (dyScale, vsci.dypTmHeight, 8);
#endif /* WIN23 */
	ypMarkBottom = rcRuler.ypBottom - rcMark.ypTop;
#ifdef WIN23
	if (vsci.fWin3Visuals)
		ypMarkBottom--;
#endif /* WIN23 */

	/* set up the marks bitmap */
#ifdef WIN23
	if (vsci.fWin3Visuals) 
		pbmi->dxpEach = NMultDiv (dxMark3, vsci.dxpFakeTmWidth, 4);
	else if (vsci.fWin3)
		pbmi->dxpEach = NMultDiv (dxMark, vsci.dxpFakeTmWidth, 4);
	else
		pbmi->dxpEach = NMultDiv (dxMark, vsci.dxpTmWidth, 4);
#else
	pbmi->dxpEach = NMultDiv (dxMark, vsci.dxpTmWidth, 4);
#endif /* WIN23 */
	dypMark = pbmi->dyp = ypMarkBottom - ypScaleBottom;

	/* NOTE:  it is very important that pbmi->dxsBmp be odd.  
				This allows marks to line up on the ticks of the ruler */

	if ( ! (pbmi->dxpEach & 1))
		/* even--force it odd */
		pbmi->dxpEach--;

	pbmi->dxp = pbmi->dxpEach * pbmi->idcbMax;

#ifdef WIN23
	if (!FLoadResourceIdrb( vsci.fWin3Visuals ? idrbRulerMarks3 :idrbRulerMarks2 ))
#else
	if (!FLoadResourceIdrb( idrbRulerMarks))
#endif /* WIN23 */
		{
		ErrorNoMemory(eidNoMemRuler);
		return fFalse;
		}

#ifdef WIN23
	dypMajorTick = NMultDiv (dyMajorTick, dypTmHeight, 8);
	dypMinorTick = NMultDiv (dyMinorTick, dypTmHeight, 8);
	dypTinyTick = NMultDiv (dyTinyTick, dypTmHeight, 8);
#else
	dypMajorTick = NMultDiv (dyMajorTick, vsci.dypTmHeight, 8);
	dypMinorTick = NMultDiv (dyMinorTick, vsci.dypTmHeight, 8);
	dypTinyTick = NMultDiv (dyTinyTick, vsci.dypTmHeight, 8);
#endif /* WIN23 */

	return fTrue;
}



/* F  M W D  C A N  S P A R E  D Y P */
/* Return fTrue iff it is OK to deduct dyp pixels of space from passed MWD */

/* %%Function:FWwCanSpareDyp %%Owner:peterj */
FWwCanSpareDyp(ww, dyp)
int ww;
int dyp;
{
	struct WWD *pwwd = *mpwwhwwd[ww];
	struct MWD **hmwd = mpmwhmwd[pwwd->mw];
	struct MWD *pmwd = *hmwd;
	struct RC rc;
	struct RC rcTopPane,rcTopScrlBar,rcSplitBox,
	rcSplitBar, rcBottomPane, rcBottomScrlBar;
	int dypSpare;

	FreezeHp();
	GetDocAreaPrc( pmwd, &rc );
	GetDocAreaChildRcs( hmwd, rc,
			PwwdWw(pmwd->wwUpper)->wk,
			pmwd->fSplit ? pmwd->ypSplit : 0,
			&rcTopPane, &rcTopScrlBar, &rcSplitBox,
			&rcSplitBar, &rcBottomPane, &rcBottomScrlBar );

	if (pwwd->fLower)
		dypSpare = rcBottomPane.ypBottom - rcBottomPane.ypTop - vsci.dypWwMin;
	else
		dypSpare = rcTopPane.ypBottom - rcTopPane.ypTop - vsci.dypWwMin;
	MeltHp();

	return (dypSpare >= dyp);
}



/*
	The ability to directly modify formatting properties such as boldness
	and font is displayed on screen in the form of the ribbon.

	The ribbon can be selectively displayed along with the ruler and
	status line.  There is a keyboard and a mouse interface for everything
	in the ribbon.

*/

/* ----- External variables */

extern BOOL             vfRecording;
extern CHAR             szEmpty[];
extern HWND             vhwndMsgBoxParent;
extern struct SEL       selCur;
extern struct CA        caPara;
extern CP               vcpFetch;
extern int              vccpFetch;
extern struct PREF      vpref;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct DOD       **mpdochdod [];
extern struct MERR      vmerr;
extern struct STTB      **vhsttbFont;
extern HANDLE           vhInstance;
extern HWND             vhwndApp;
extern struct WWD       **hwwdCur;
extern HWND             vhwndStatLine;
extern HWND             vhwndDeskTop;
extern HWND             vhwndCBT;
extern HCURSOR          vhcArrow;
extern BOOL             vfInitializing;

extern struct SCI           vsci;
extern struct PRI           vpri;

extern RRF		rrf;


/* ----- Globals  */

struct CHP  chpRibbon;   /* these hold current state of ribbon */
struct CHP  chpRibbonGray;
CHAR grpfShowAllRibbon;


/* ****
*
	Function:  CmdRibbon
*  Copyright: Microsoft 1986
*  Description: View Ribbon menu function
*
** ***/

/* %%Function:CmdRibbon %%Owner:peterj */
CMD CmdRibbon(pcmb)
CMB * pcmb;
{
	/* This routine toggles creation and destruction of the ribbon window */
	return FTurnRibbonFOn(vhwndRibbon == NULL, fTrue, fTrue) ? cmdOK : cmdNoMemory;
}


/* %%Function:FCreateRibbon %%Owner:peterj */
BOOL NEAR FCreateRibbon()
{
	struct RC rc;
#ifdef WIN23
	CHAR ibd[sizeof(ibdRibbon3) > sizeof(ibdRibbon2) ? sizeof(ibdRibbon3) : sizeof(ibdRibbon2) ];
	CHAR dlt[sizeof(dltRibbon3) > sizeof(dltRibbon) ? sizeof(dltRibbon3) : sizeof(dltRibbon)];
#else
	CHAR ibd[sizeof(ibdRibbon)];
	CHAR dlt[sizeof(dltRibbon)];
#endif /* WIN23 */


#ifdef WIN23
	if (vsci.fWin3Visuals)
		BltDlt(dltRibbon3, dlt);	/* dialog based on win 3 font */
	else
		BltDlt(dltRibbon, dlt);		/* dialog based on win 2 font */
	if (vsci.fWin3Visuals)
		BltIbd(ibdRibbon3, ibd);	/* ribbon layout based on Win 3 font */
	else
		BltIbd(ibdRibbon2, ibd);	/* ribbon layout based on Win 2 font */
#else
	BltIbd(ibdRibbon, ibd);
	BltDlt(dltRibbon, dlt);
#endif /* WIN23 */

	GetClientRect( vhwndApp, (LPRECT)&rc );

	if ((vhwndRibbon = HwndCreateIconBar(ibd, dlt, vhwndApp, &rc, rgtmwRibbon)) == NULL)
		return (fFalse);

	/* at startup, put off update until after doc is created  */
	/* ... same if we're restoring ribbon after returning from CBT */
	if (!vfInitializing && (vrf.fRibbonCBT != fTrue /* be explicit; tri-state flag */))
		UpdateRibbon(fTrue /* fInit */);

	Assert (vhwndRibbon != NULL);
	return fTrue;
}



/* F  T U R N  R I B B O N  F  O N */
/* This is the only function that should be used to turn the 
* ribbon on or off */
/* %%Function:FTurnRibbonFOn %%Owner:peterj */
FTurnRibbonFOn(fOn, fAdjust, fUpdate)
BOOL fOn, fAdjust, fUpdate;
{
	BOOL fOnCur;
	struct RC rc;

	fOnCur = (vhwndRibbon != NULL);

	if (!fOn == !fOnCur) /* please don't kill my !s (bac). */
		return fTrue;  /* no change needed */

	StartLongOp();

	if (fOn)
		{
		if (!FCreateRibbon())
			{
			EndLongOp(fFalse /* fAll */ );
			return fFalse;
			}
		if (!vfInitializing)
			{
			if (fUpdate)
				{
				ShowWindow(vhwndRibbon, SHOW_OPENWINDOW);
				UpdateWindow(vhwndRibbon);
				}
			if (fAdjust)
				{
				GetDeskTopPrc(&rc);
				MoveWindowRc(vhwndDeskTop, &rc, fTrue);
				ResizeMwds();
				}
			}
		}
	else  /* turn off ribbon */		
		{
		HWND hwndRibbon = vhwndRibbon;

		vhwndRibbon = NULL;
		if (!vfInitializing && fAdjust)
			{
			GetDeskTopPrc(&rc);
			MoveWindowRc(vhwndDeskTop, &rc, fTrue);
			}
		if (hwndRibbon)
		/* do this after desktop is adjusted to avoid seeing bkgrnd color */
			DestroyWindow(hwndRibbon);
		}

	EndLongOp(fFalse /* fAll */ );

	return fTrue;
}


