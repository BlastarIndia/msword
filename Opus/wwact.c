/* W W A C T . C */
/*  code relating to activation of windows */

#define RSHDEFS

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "screen.h"
#include "heap.h"
#include "ibdefs.h"
#include "idle.h"
#include "outline.h"
#include "scc.h"
#include "doc.h"
#include "prm.h"
#include "ruler.h"
#include "format.h"
#include "layout.h"
#include "menu2.h"
#include "keys.h"
#include "cmd.h"
#include "cmdtbl.h"
#include "rsb.h"
#include "version.h"
#include "wininfo.h"
#include "debug.h"
#include "rareflag.h"
#include "ch.h"

/* G L O B A L S */

HFONT vhfntStyWnd; /* font for style name window */
int vdypStyWndFont; /* height of style name window's font */
struct RC       vrcUnZoom;  /* rect info of the window before it is zoomed */
struct RULSS    vrulss;     /* ruler sprm state */
HWND vhwndRibbon;
BOOL vcInNewCurWw = 0;  /* to avoid badness if nested in WM_MOUSEACTIVATE */
KME vkme;

	char            rgchEop [] = { 
	chReturn, chEop 	};


	char            rgchTable[] = {
	chReturn, chTable	}; /* act like a fake CRLF */


	char            rgchSect[1] = {
	chSect	};


/* E X T E R N A L S */

KME * PkmeFromKc();
KME * PkmeOfKcInChain();

extern int viMenu;
extern HWND vhwndApp;
extern HANDLE vhMenuLongFull;
extern HMENU vhMenu;
extern struct MWD ** hmwdCur;
extern int              vistg;
extern struct STTB      **hsttbMenu;
extern int              fnFetch;
extern int		vdocFetch;
extern int              vdocPapLast;
extern int              wwCur;
extern char             szApp[];
extern int              vfInitializing;
extern int              vfFocus;
extern int              vdocTemp;
extern int              vfSeeSel;
extern int		wwMac;
extern struct MERR      vmerr;
extern struct WWD       **mpwwhwwd[];
extern struct MWD       **mpmwhmwd[];
extern struct WWD       **hwwdCur;
extern int		docMac;
extern HWND             vhwndDeskTop;
extern int              flashID;
extern struct DOD       **mpdochdod[];
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern struct SEL       selMacro;
extern struct CHP       vchpFetch;
extern struct SCC       vsccAbove;
extern struct SCC       *vpsccBelow;
extern struct CA	caPage;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA	caHdt;
extern struct FLI       vfli;
extern struct FLS       vfls;
extern struct SELS      rgselsInsert[];
extern struct UAB       vuab;
extern struct SAB       vsab;
extern struct RULDS	vrulds;
extern int              vfWordActive;
extern int              vssc;
extern struct LLC       vllc;
extern struct RULSS	vrulss;
extern struct SPOP vspop;
extern int vgrfMenuCmdsAreDirty;
extern int vgrfMenuKeysAreDirty;
extern int docDotMudCur;
extern int vkcPrev;
extern HBITMAP hbmpSystem;
extern int vfSysMenu;
extern BOOL vfFileCacheDirty;
extern BOOL vfWndCacheDirty;
extern HWND             vhwndAppModalFocus;
extern BOOL             vfDeactByOtherApp;
extern HWND		vhwndStartup;
extern HWND		vhwndStatLine;
extern HWND		vhwndRibbon;
extern KMP **		hkmpCur;
extern HWND		vhwndMsgBoxParent;
extern int		dclMac;
extern BOOL		vfExtendSel;
extern BOOL		vfBlockSel;
extern IDF		vidf;
extern int		vcInMessageBox;
extern int		vfRecording;
extern int		mwCur;
extern struct PREF	vpref;
extern struct SCI	vsci;
extern int              vdocScratch;
extern int              vfTableKeymap;



/* %%Function:AppWndProcAct %%Owner:chic */
long AppWndProcAct(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{

	switch (message)
		{
	case WM_ACTIVATE:
		/* Don't do anything here if the desktop is not created yet
			or if we are shutting down.
		*/
		ClearAbortKeyState();
		SetOurKeyState();
		if (vhwndDeskTop == NULL)
			break;
		if (wParam /*fActive*/&& !vidf.fDead)
			{
			if (mwCur != mwNil && !PmwdMw(mwCur)->fActive)
				{
				ActivateWn(hmwdCur, fTrue);
				if (vfDeactByOtherApp)
					BlankScrollBarsMwd( hmwdCur, fFalse );
				}
			if (vhwndAppModalFocus)
				SetFocus(vhwndAppModalFocus);
			else  if (wwCur != wwNil)
				SetFocus((*hwwdCur)->hwnd);
			else
				SetFocus(vhwndApp);
			}
		else
			SetActiveMw(mwNone);
		break;

	case WM_ACTIVATEAPP:
		/* We are activated or deactivated by another application */
#ifdef RSH
		SetUatMode(uamIdle);	/* Set current mode to Idle */
#endif /* RSH */
		vfDeactByOtherApp = !wParam;
		if (vfDeactByOtherApp)
			{
			Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
			/* hide selection if needed */
			if (wwCur != wwNil && !selCur.fHidden && !vidf.fDead)
				{
				UpdateWindow(vhwndApp);
				TurnOffSel(&selCur);
				TurnOffSel(&selDotted);
				}
			/* make sure all of our files are open so other apps can't
				get at them. */
			if (!vidf.fDead)
				OpenEveryHardFn();
			}
		else
			/* we are active */
			{
			ClearAbortKeyState();
			SetOurKeyState();
			if (flashID)
				{
				KillTimer (vhwndApp, flashID);
				flashID = 0;
				FlashWindow (vhwndApp, fFalse);
				}
			}
		/* adjust our swap area (grow if we are active, else shrink) */
		OurSetSas(vfDeactByOtherApp ? sasMin : sasFull);
		if (!vfDeactByOtherApp && vrf.fWaitForDraw)
			DoEditPic2();
		break;
		}
	/* A window proc should always return something */
	return(0L);
}


/* W W  A L L O C  C O M M O N  */
/* performs allocation and initialization of WWD and the PLCEDL for rgdr[0].*/
/* %%Function:WwAllocCommon %%Owner:chic */
WwAllocCommon(doc, mw, psels, prcwDisp, prcePage, dlMax, wk)
int doc;
int mw;
struct SELS *psels;
struct RC *prcwDisp;
struct RC *prcePage;
int dlMax;
int wk;
{
	struct PLCEDL **hplcedl = NULL;
	int ww;
	struct WWD *pwwd;

	hplcedl = HplcInit(cchEDL, dlMax, cpMax, fTrue /* ext rgFoo */);

	ww = IAllocCls(clsWWD, cwWWD);
	if (ww == wwNil || hplcedl == hNil || vmerr.fMemFail)
		{
LFail:
		FreeHplc(hplcedl);
		FreeCls(clsWWD, ww);
		SetErrorMat( matLow );
		return(wwNil);
		}
	FreezeHp();
	pwwd = PwwdWw(ww);
	pwwd->mw = mw;
	pwwd->wk = wk;
	pwwd->sels = *psels;
	pwwd->rcwDisp = *prcwDisp;
	pwwd->rcePage = *prcePage;

/* Initialize outline display modes. */
	pwwd->fEllip = vpref.fEllip;
	pwwd->fShowF = vpref.fShowF;

/* fill in PL variables. */
	pwwd->idrMac = 1;
	pwwd->idrMax = 1;
	pwwd->cbDr = sizeof(struct DR);
	pwwd->brgdr = offset(WWD, rgdr);
	pwwd->xhScroll = 0;
	PdrGalley(pwwd)->hplcedl = hplcedl;
	MeltHp();
	InitGalleyDr(ww);

/* NOTE: this has been changed to insert ww in to the list at the END, 
	not the beginning. This is so multiple windows to same doc have increasing
	instance numbers.
*/
	AppendWw(doc, ww);
	return (ww);
}


/* I N I T  G A L L E Y  D R */
/* initializes dr0 in galley mode (except for hplcedl) */
/* %%Function:InitGalleyDr %%Owner:chic %%reviewed: 7/10/89 */
InitGalleyDr(ww)
{
	struct WWD *pwwd = PwwdWw(ww);
	struct PLCEDL **hplcedlT;
	struct DR *pdr, dr;

	if (pwwd->idrMac == 0)
		{
		SetWords(&dr, 0, cwDR);
		if (!FInsertInPl(HwwdWw(ww), 0, &dr))
			{
			Assert(fFalse);
			return;	/* we're gonna die */
			}
		pwwd = PwwdWw(ww);
		pdr = PdrGalley(pwwd);
		}
	else
		{
		Assert(pwwd->idrMac == 1);
		pdr = PdrGalley(pwwd);
		hplcedlT = pdr->hplcedl;
		SetWords(pdr, 0, cwDR);
		pdr->hplcedl = hplcedlT;
		}
	pdr->doc = pwwd->sels.doc;
	/*pdr->drcl.xl = pdr->drcl.yl = 0;*/
/* xa is conservatively turned into an xp. DxsFromDxa is not used
because necessary quantities are not yet set up and precision is not nec. */

	pdr->drcl.dxl = xpRightMaxSci;
	pdr->dxa = NMultDiv(pdr->dxl, dxaInch, vfli.dxsInch);
	pdr->dxpOutLeft = -xpLeftMinSci;
	pdr->drcl.dyl = DyOfRc(&pwwd->rcwDisp);
	pdr->idrFlow = idrNil;
	pdr->fNoPrev = fTrue;
}


/* N E W  C U R  W W */
/* Procedure does the new window specific part of changing windows:
	0) sets Cur globals
	1) moves the SELS (and ww) stored in the window to the selCur if
		vssc == sscNil. selDotted always starts nil.
	2) initializes caPage
	3) sets the fActive bit of the proper pane of the window
	5) fills the screen caches

	Even newer and very simple rules for window management:
	wwCur and hwwdCur represent the doc window which is on the top
	and which is used for selection of selCur (if vssc == sscNil)
	or selDotted otherwise.
	Note that this means that selCur.ww is not always wwCur if
	vssc != sscNil.
	The window will start with: If vssc == sscNil
		the same selCur as it was the last time in that window,
	else
		nil selDotted.
	sel.doc always follows sel.ww!

fDoNotSelect is used to prevent the display of selection in the window
when the activation is in preparation of an imminent mouse selection.
*/
/* %%Function:NewCurWw %%Owner:chic */
NewCurWw(ww, fDoNotSelect)
int ww;
{
	struct MWD *pmwd;
	struct PLC **hplcpgd;
	int mwOld = mwNil;
	struct PLCEDL **hplcedl;
	struct DR *pdr;
	int idr, cdr;
	int wwRuler;
	long hctl;
/* for header */
	int wwOld = wwCur;
	struct WWD **hwwdOld;
#ifdef DEBUG
	int fPageViewOld;
#endif
	/* this inhibits PeekMessages out of FMsgPresent when in NewCurww
	for reasons mentioned below. We want to be sure a mouseactivate
	message is not in the queue when PeekMessage is called, and
	since NewCurWw seems to be the guy that would do that, I am setting
	this counter here rather than at every caller. Be sure you
	do not exit without decrementing it (goto Lret)
	*/

/* Kludge Alert!  Windows has a re-entrancy problem with MA_ACTIVATEANDEAT
	such that we cannot call PeekMessage while nested in a WM_MOUSEACTIVATE
	message to which we plan to return MA_ACTIVATEANDEAT.  If we do, the
	message will stay on the queue.  NewCurWw can cause FMsgPresent to be
	called indirectly.  rp - 1/4/89

	Dyadic copy formatting, which does some extra NewCurWw calls had a similar
	problem with the MOUSEACTIVATE message not getting cleared. Setting the
	flag so FMsgPresent will return solved the problem, though I am not sure
	why, since the details are a little different that Rosie describes in Opus
	bug 5810. bz 4-27-89

	Problems with double click messages in activating panes with rulers
	turned out the same as above: the LButtonDown message was not being
	removed from the stack, and it turned into a double click. bz 5/10/89
*/

	vcInNewCurWw++;

	/* check for IB dialog in effect */
	if (vidf.fIBDlgMode)
		{
		TermCurIBDlg(fTrue);
		}

	Assert(wwCur == wwNil || wwCur >= wwDocMin);    /* always true */
	if (ww == wwCur)
		{
		EnsureFocusInPane();
		goto LRet;
		}

/* clean up from old window. handle test avoids actions from
	CloseWw-->PickNewWw-->NewCurwWw for WIN version */

/* bug fix, don't just test for wwCur != wwNil because wwCur's structure may have been destroyed in CloseWw */
	if (mpwwhwwd[wwCur] != hNil)
		{
		/* this update is necessary for selection; sets ww, draws etc. */
		UpdateWw(wwCur, fFalse);
		if (vssc == sscNil)
			TurnOffSel(&selCur);
		TurnOffSel(&selDotted);
		mwOld = (*hwwdCur)->mw;
		Debug(fPageViewOld = (*hwwdCur)->fPageView);
		/* we really do want selCur, not selDotted here. We are saving the
			sel to restore another time, or to set up selDotted with
			"reasonable" values. When we leave a window that had a selDotted,
			we lose that selDotted; coming back does not restore it. In all
			cases I can think of, it is right to save and restore from
			selCur.  bz 9-15-88
		*/
		if (selCur.ww == wwCur)
			{
			Assert(fPageViewOld || selCur.doc == (*hwwdCur)->sels.doc);
			blt(&selCur, &(*hwwdCur)->sels, cwSELS);
			}

		if ((*hwwdCur)->fHdr)
			{
			Assert(PselActive()->doc == (*hwwdCur)->sels.doc);
			FSaveHeader(PselActive()->doc, fFalse);
			}

/* delete all edls if the window is not visible; leaves all drs, no edls */
		if (FWindowHidden(wwCur))
			{
			for (pdr = PInPl(hwwdCur, 0), cdr = (*hwwdCur)->idrMac; cdr-- > 0; pdr++)
				{
				if ((hplcedl = pdr->hplcedl) != hNil)
					{
					pdr->fIncomplete = pdr->fDirty = fTrue;
					FreeEdls(hplcedl, 0, IMacPlc(hplcedl));
					FreePh(&pdr->hplcedl);
					}
				}
			}
		}

	hwwdCur = mpwwhwwd[wwCur = ww];

/* from here on, wwCur is ww, changed all use of ww to wwCur */
	/* ensure a valid selection in selDotted */
	if (hwwdCur)
		{
		blt(&(*hwwdCur)->sels, &selDotted, cwSELS);
		if ((*hwwdCur)->fPageView && (*hwwdCur)->ipgd == ipgdNil)
/* get out of pageview when we are in trouble */
			FExecCmd(bcmPageView);
		}

	selDotted.sk = skNil;
	selDotted.pa = paDotted;
	SetSelWw(&selDotted, wwCur);

	/* If we are changing to a macro window with an animation selection, turn
	 * off the selection (selMacro) here.  This is because we have stored
	 * selMacro as the current selection for that window, but we only copied
	 * in a short sel (SELS), which does not include the fHidden bit.
	 * When TurnOnSelCur then turns on the new selCur in the macro window,
	 * it assumes the selection is off, turns it on, and we get a double-
	 * selection.  To fix, we turn off the selection here, so it can be
	 * turned back on by TurnOnSelCur. */
	if (selMacro.doc != docNil && selMacro.ww == wwCur && !selMacro.fHidden)
		TurnOffSel(&selMacro);

	InvalSelCurProps(fTrue);
	if (vssc == sscNil)
		{
		struct SELS selsT;

		selCur.sk = skNil;
		SetSelWw(&selCur, wwCur);
		if (hwwdCur)
			{
			selsT = (*hwwdCur)->sels;
			/* protects cp beyond CpMacDoc when AdjustSels does not 
			catch cases where we delete beyond cpMacDocEdit */
			selsT.cpFirst = CpMin(selsT.cpFirst, CpMacDocEdit(selsT.doc));
			selsT.cpLim = CpMin(selsT.cpLim, selsT.fIns ? 
				CpMacDocEdit(selsT.doc) : CpMacDoc(selsT.doc));
			}

		if (!fDoNotSelect && hwwdCur)
			{
			SetSelCurSels(&selsT);   /* HM */
			(*hwwdCur)->sels = selsT;
			MakeSelCurVisi(fFalse/*fForceBlockToCp*/);
			}
		else  if (hwwdCur)
			{
			blt(&selsT, &selCur, cwSELS);
			selCur.sk = skNil;
			SetSelWw(&selCur, wwCur);
			}
		}

	if (wwCur != wwNil)
		{
		/* set fields in MWD */
		pmwd = PmwdWw(wwCur);
		pmwd->wwActive = wwCur;
		if (mwOld == (*hwwdCur)->mw && WwScrollWkTest(wwCur) == wwNil)
			SyncSbHor(wwCur);
		}

/* Turn off the special selection modes when changing windows */
	if (vfBlockSel)
		BlockModeEnd();
/* prevent turning off extend mode if coming back from a search, goto
	dialog box or message box.  Usually the mode would have been turned
	off by most commands. */
	if (vfExtendSel && !vcInMessageBox)
		ExtendSelEnd();

	InvalFli();
/* establish identities of screen/line break caches */
	if (wwCur != wwNil && !(*hwwdCur)->fPageView)
		SetSccCurWw(&vsccAbove, wwCur);
	else  if (mpwwhwwd[vsccAbove.ww] == 0)
		vsccAbove.ww = wwNil;

	/* set table keymap, if necessary */
	if (vfTableKeymap)
		UnhookTableKeymap();
	if (wwCur != wwNil && (selCur.fWithinCell || selCur.fTable))
		{
		if (!vfTableKeymap)
			{
			/* don't care if this fails, at this point */
			FHookTableKeymap();
			}
		}

/* set current keymap and menu */
	if (wwCur != wwNil)
		{
		int mw = PwwdWw(wwCur)->mw;

		if (!(*hkmpCur)->fAppModal)
			{
			hkmpCur = (*hwwdCur)->hkmpCur;
			AssertH(hkmpCur);
			}
		if (vfRecording)
			{
			/* If the mw hasn't changed, we must be changing from one pane
			 * to the opposite pane, so we record a PaneChange.  Otherwise,
			 * the mw has changed, and we record a WindowChange.  We include
			 * a pane number if the new mw is split.  (Pane number 1 or 2
			 * indicates the upper pane, 3 or 4 indicates the lower pane.) */
			if (mw == mwCur)
				{
				Assert(PmwdMw(mw)->fSplit);
				RecordPaneChange();
				}
			else if (PmwdMw(mw)->fSplit)
				{
				RecordWindowChange(PmwdMw(mw)->hwnd,
					(wwCur == PmwdMw(mw)->wwUpper) ? 1 : 3 /* Pane "number" */);
				}
			else
				RecordWindowChange(PmwdMw(mw)->hwnd, 0 /* Don't record pane # */);
			}

		SetActiveMw(mw);
		if (!vfDeactByOtherApp)
			SetFocus(PwwdWw(wwCur)->hwnd);
		}
	else
		SetActiveMw(mwNil);

	/* these will happen in idle but we want immediate update at activation time */
	if (!fDoNotSelect && vhwndStatLine && vhwndStartup == NULL)
		UpdateStatusLine(usoNormal);

	if (!fDoNotSelect && vhwndRibbon && vhwndStartup == NULL)
		/* this will disable the ribbon if appropriate */
		UpdateRibbon(fFalse);

	if (wwCur != wwNil && PmwdWw(wwCur)->hwndRuler)
		{
		int doc = PselActive()->doc;
		BOOL fEnableRuler = !PdodDoc(doc)->fLockForEdit;
		if (PdodDoc(doc)->fShort)
			fEnableRuler &= !PdodMother(doc)->fLockForEdit;
		EnableWindow(PmwdWw(wwCur)->hwndRuler, fEnableRuler);
		}

	ChangeCursor(fFalse/*fExact*/);
LRet:
	vcInNewCurWw--;  /* this is a counter not a flag due to recursive calls */

}


#ifndef JR
/* D i r t y  O u t l i n e */
/* %%Function:DirtyOutline %%Owner:chic */
DirtyOutline(doc)
int doc;
{
/* if any ww for the doc is in outline mode, update the pad */
	struct WWD *pwwd;
	int ww, ipad;
	struct DOD *pdod = PdodDoc(doc);
	struct PAD pad;
	struct PLC **hplcpad;

	if (pdod->fShort || (hplcpad = pdod->hplcpad) == hNil)
		return;

	AssertH(hplcpad);
	for (ww = WwDisp(doc, wwNil, fFalse); ww != wwNil; 
			ww = WwDisp(doc, ww, fFalse))
		if ((pwwd = PwwdWw(ww))->fOutline)
			{
			pdod->fOutlineDirty = fTrue;
			for (ipad = IMacPlc(hplcpad); ipad-- > 0; )
				{
				GetPlc(hplcpad, ipad, &pad);
				pad.lvl = lvlUpdate;
				PutPlcLast(hplcpad, ipad, &pad);
				}
			FUpdateHplcpad(doc);
			vuab.fOutlineDirty = fTrue;
			break;
			}
}


#endif /* JR */



/*******************************/
/* R e m o v e   W w   D i s p */
/* %%Function:RemoveWwDisp %%Owner:chic */
RemoveWwDisp(doc, ww)
int doc, ww;
{
/* remove ww from the wwDisp chain for its mother doc */
/* note that there's no pretty way to do this because dod.wwDisp and 
	wwd.wwDisp are different sizes */

	struct DOD *pdod = PdodMother(doc);
	int wwDisp = pdod->wwDisp;
	struct WWD *pwwd;

	if (wwDisp == ww)
		pdod->wwDisp = PwwdWw(ww)->wwDisp;
	else
		{
		while (wwDisp != wwNil)
			if ((wwDisp = (pwwd = PwwdWw(wwDisp))->wwDisp) == ww)
				break;
		pwwd->wwDisp = PwwdWw(ww)->wwDisp;
		}
}


/* F  S E T  I D R  M A C */
/* %%Function:FSetIdrMac %%Owner:chic */
FSetIdrMac(hpldr, idrMac)
struct PLDR **hpldr;
{
	(*hpldr)->idrMac = idrMac;
	(*hpldr)->idrMax = idrMac;
	return (FChngSizeHCw(hpldr,
			((*hpldr)->brgdr + (*hpldr)->cbDr * idrMac) / sizeof(int),
			fTrue));
}


/* F   D O C   B U S Y */
/* %%Function:FDocBusy %%Owner:chic */
FDocBusy(doc)
int doc;
{
/* returns true if the doc cannot be deleted */
	int docRef;
	struct DOD **hdod;
	struct CA caT;


/* referenced by scrap or locked */
	if (doc < docMinNormal || 
			PdodDoc(doc)->crefLock != 0)
		{
		return(fTrue);
		}

	if (vsab.docStsh == doc)
		{
		CopyFonts(doc, PcaSetDcp(&caT,docScrap, cp0, CpMacDoc(docScrap)));
		CopyStyles1(doc, docScrap, cp0, CpMacDoc(docScrap));
		vsab.docStsh = docNil;
		}

/* break allegiance of vdocTemp */
	if (vdocTemp != docNil && PdodDoc(vdocTemp)->doc == doc)
		{
		FDelete(PcaSetWholeDoc(&caT, vdocTemp));
		PdodDoc(vdocTemp)->doc = docScrap;
		}
	if (vdocScratch != docNil && PdodDoc(vdocScratch)->doc == doc)
		{
		struct DOD *pdod;

		Assert (!fDocScratchInUse);
		FDelete(PcaSetWholeDoc(&caT, vdocScratch));
		(pdod = PdodDoc(vdocScratch))->doc = docNil;
		pdod->fMotherStsh = fFalse;
		}

/* referenced by a subdoc */
	for (docRef = docMinNormal; docRef < docMax; docRef++)
		{
		hdod = mpdochdod[doc];
		if (hdod != hNil && (*hdod)->fShort && (*hdod)->doc == doc)
			return(fTrue);
		}
/* displayed in a wwd */
	return(WwDisp(doc, wwNil, fFalse) != wwNil);
}


/* S Y N C H  S C C  W W */
/* if not valid for the window, empties the cache and
sets its width and height.
*/
/* %%Function:SynchSccWw %%Owner:chic */
SynchSccWw(pscc, ww)
struct SCC *pscc;
int ww;
{
	SetSccCurWw(pscc, ww);
}


/* moved from wwact.c */
/* S E T  S C C  C U R  W W */
/* %%Function:SetSccCurWw %%Owner:chic */
SetSccCurWw(pscc, ww)
struct SCC *pscc;
int ww;
{
	pscc->ww = ww;
	if (ww == wwNil)
		pscc->doc = docNil;
	else
		pscc->doc = DocBaseWw(ww);
	PutIMacPlc( pscc->hplcedl, 0 );
	pscc->fFull = fFalse;
}



/* F  S E T  M E N U */
/* %%Function:FSetMenu %%Owner:chic */
BOOL FSetMenu(iMenu)
int iMenu;
{
	HMENU HMenuLoadCodeMenu();
	HMENU hMenu, hMenuOld;
	extern int vgrfMenuKeysAreDirty;
	extern int vgrfMenuCmdsAreDirty;
	int iMenuOld;

	if (viMenu == iMenu)
		/* nothing to do */
		return fTrue;

	hMenuOld = vhMenu;
	iMenuOld = viMenu;

	if (iMenu == iMenuLongFull)
		hMenu = vhMenuLongFull;
	else  if ((hMenu = HMenuLoadCodeMenu(iMenu)) == NULL)
		return fFalse;

	if (vhMenu != NULL)
		FRemoveSystemMenu();

	vhMenu = hMenu;
	viMenu = iMenu;

	if ((iMenu==iMenuShortFull || iMenu==iMenuLongFull) && vpref.fZoomMwd)
		FInsertSystemMenu(hmwdCur == hNil ? NULL : (*hmwdCur)->hwnd);

	if (vhwndApp != NULL && !SetMenu(vhwndApp, vhMenu))
			goto LFail;

	if (iMenuOld != iMenuLongFull)
		DestroyMenu(hMenuOld);
	vfFileCacheDirty = vfWndCacheDirty = fTrue;
	vgrfMenuKeysAreDirty = 0xffff;
	vgrfMenuCmdsAreDirty = 0xffff;

	return fTrue;

LFail:
	viMenu = iMenuOld;
	vhMenu = hMenuOld;
	if (iMenu != iMenuLongFull)
		DestroyMenu(hMenu);

	return fFalse;
}


/* C O R R E C T  S Y S T E M  M E N U */
/* Add or remove the system menu of hmwdCur to the app menu. */
/* %%Function:CorrectSystemMenu %%Owner:chic */
CorrectSystemMenu()
{
	BOOL fRedraw;

	if (vpref.fZoomMwd)
		fRedraw = FInsertSystemMenu(hmwdCur == hNil ? NULL : (*hmwdCur)->hwnd);
	else
		fRedraw = FRemoveSystemMenu();

	if (fRedraw && vhwndApp != NULL)
		DrawMenuBar(vhwndApp);
}


/* F  I N S E R T  S Y S T E M  M E N U */
/*  Place the system menu of hwnd onto the app menu.  return fTrue if anything
	changed. */
/* %%Function:FInsertSystemMenu %%Owner:chic */
FInsertSystemMenu(hwnd)
HWND hwnd;
{
	extern HBITMAP hbmpSystem;
	HMENU hmenuSys;
	int mf;
	CHAR rgch[10];

	Assert(hbmpSystem != NULL);

	mf = MF_INSERT;
	hmenuSys = (hwnd == NULL) ? NULL : GetSystemMenu(hwnd, FALSE);

	Assert(viMenu == iMenuLongFull || viMenu == iMenuShortFull);
	Assert(viMenu != iMenuLongFull || vhMenu == vhMenuLongFull);

	rgch[0] = GetMenuString(vhMenu, 0, (LPSTR) &rgch[1], sizeof (rgch)-1,
			MF_BYPOSITION);

	if (rgch[0] == 0)
		{
		HMENU hmenuT;

		/* if the fake system menu is on the menu bar, GetSubMenu()
			will return NULL */
		hmenuT = GetSubMenu(vhMenu, 0);
		if (hmenuT == hmenuSys)
			return fFalse;
		if (hmenuT != NULL)
			{
			/* remove--don't care too much if it fails! 
			(which seems unlikely) */
			ChangeMenu(vhMenu, 0, NULL, NULL, 
					MF_REMOVE | MF_BYPOSITION);
			}
		else
			{
			mf = MF_CHANGE;
			}
		}

	if (ChangeMenu(vhMenu, 0, MakeLong(hbmpSystem,0), hmenuSys, 
			mf | MF_BYPOSITION | MF_BITMAP |
			(hmenuSys == NULL ? 0 : MF_POPUP)) == NULL)
		{
		if (mf == MF_CHANGE)
			return FRemoveSystemMenu();
		else
			{
			vfSysMenu = fFalse;
			return fFalse;
			}
		}
	vfSysMenu = fTrue;
	vgrfMenuKeysAreDirty |= (1 << imnuDocControl);

	return fTrue;
}


/* F  R E M O V E  S Y S T E M  M E N U */
/*  Remove any MWD system menu from the app menu.  return fTrue if anything
	changed */
/* %%Function:FRemoveSystemMenu %%Owner:chic */
FRemoveSystemMenu()
{
	CHAR rgch[10];

	vfSysMenu = fFalse;

	StartUMeas(umGetMenuStr);
	rgch[0] = GetMenuString(vhMenu, 0, (LPSTR)&rgch[1], sizeof (rgch)-1,
			MF_BYPOSITION);
	StopUMeas(umGetMenuStr);
	if (rgch[0] != 0)
		return fFalse;

	StartUMeas(umChangeMenu);
	/* remove--don't care too much if it fails! (which seems unlikely) */
	ChangeMenu(vhMenu, 0, NULL, NULL, MF_REMOVE | MF_BYPOSITION);
	StopUMeas(umChangeMenu);

	return fTrue;
}




/* S E T U P  M E N U */
/*  Set the app menu as appropriate.  does not change anything if same dot
	is in effect.*/
/* %%Function:SetupMenu %%Owner:chic */
SetupMenu(docDot)
int docDot;  /* for when we're just changing dot */
{
	extern struct PREF vpref;
	extern struct SEL selCur;
	extern struct WWD ** hwwdCur;

	int iMenu = vpref.fShortMenus ? iMenuShortFull : iMenuLongFull;

	/* Apply customizations to menus */
	if (docDot == docNil)
		docDot = wwCur==wwNil ? docNil : DocDotMother(PmwdWw(wwCur)->doc);

	/* There is nothing to change, menu wise, if we are switching 
			to a document of the same type as the one we had or if we
			are in short menus. */
	if (docDot != docDotMudCur && iMenu == iMenuLongFull)
		{
		/* Menus need recalculating... */
		vfFileCacheDirty = vfWndCacheDirty = fTrue;
		docDotMudCur = docDot;
		vgrfMenuCmdsAreDirty = 0xffff;
		vgrfMenuKeysAreDirty = 0xffff;
		}
	else
		vgrfMenuKeysAreDirty |= (1 << imnuDocControl);

	if (viMenu != iMenu)
		FSetMenu(iMenu);
	else
		CorrectSystemMenu();
}


extern HANDLE           vhInstance;




/* %%Function:WwPaneLoseFocus %%Owner:chic */
WwPaneLoseFocus(hwnd, hwndNewFocus)
HWND hwnd;
HWND hwndNewFocus;
{
	Assert(hwwdCur != hNil);

	/* if we are in dyadic move/copy, in 2 different docs, selCur may
		refer to a window other than wwCur, so blting would screw up
		everything. In that case, move in selDotted instead  (PselActive
		handles this).
	*/

	blt(PselActive(), &(*hwwdCur)->sels, cwSELS);

	if (vfFocus)
		{
#ifdef UNUSED /* don't care since MAC does not do this either and it is causing a lot of ghost cursor bug */
		/* any insert point should be left visible as we leave */
		if (!selCur.fOn)
			ShowInsertLine(&selCur);
#endif
		vfFocus = fFalse;
		}
}


/* %%Function:WwPaneGetFocus %%Owner:chic */
WwPaneGetFocus(hwnd, hwndPrevFocus)
HWND hwnd;
HWND hwndPrevFocus;
{
	extern long dtickCaret;

/* by the time we get here, everything is supposed to be set up! */
	Assert( hwnd == PwwdWw(wwCur)->hwnd );

	if (vhwndAppModalFocus != hNil)
		{
		SetFocus (vhwndAppModalFocus);
		return;
		}

	vhwndMsgBoxParent = hwnd;

	if (wwCur != wwNil && hwnd != PwwdWw(wwCur)->hwnd)
		{
		Assert( WwFromHwnd(hwnd) != wwNil );
		NewCurWw(WwFromHwnd(hwnd), fFalse /*fDoNotSelect*/);
		}

	if (!vfFocus)
		{
		vfFocus = fTrue;
		selCur.tickOld = 0;
		if (GetCaretBlinkTime() != dtickCaret)
		/* blink time changed */
			dtickCaret = GetCaretBlinkTime();

	/* Update globals that tell us the state of the lock/shift keys */
		SetOurKeyState ();
		}
}




/* S e t  A c t i v e  M w */
/* Make MWD window mw be active (meaning: fActive is set and the
	window is on top.
	Special values:  mwNone - deactivate current MWD, do not change mwCur etc
			mwNil - make no MWD current (only when no MWDs exist)
*/

/* %%Function:SetActiveMw %%Owner:chic */
SetActiveMw(mw)
int mw;
{
	extern BOOL vfWndCacheDirty;
	extern struct RC vrcUnZoom;

	static int cRecurse = 0;
	int mwPrev;
	int docPrev;
	HWND hwndMwOld = hNil;
	struct RC rcUnZoomSav;

	Assert( cRecurse == 0 );
	if (cRecurse > 0 || vidf.fDead || mw == mwCur)
		return;

	Assert( mw == mwNil || mw == mwNone || mpmwhmwd[mw] != hNil );
	cRecurse++;

	mwPrev = mwCur;

	docPrev = (hwwdCur == hNil) ? docNil : DocBaseWw(wwCur);

	if (mw == mwNil)
		{
/* deactivating the last MWD */
		hmwdCur = hNil;
		mwCur = mwNil;
		if (!vfDeactByOtherApp)
			SetFocus(vhwndApp);

		if (vpref.fZoomMwd)
			SetWindowText(vhwndApp, (LPSTR) szAppTitle);

		if (!vfInitializing) /* to avoid flash from FileNew dialog */
			FSetMinMenu();
		}
	else
		{
/* Deactivate the current child window */
/* explicit check for handle catches case when MWD has been disposed
	already (CloseWw->NewCurWw->SetActiveMw for last pane) */

		if (mpmwhmwd[mwCur] != mwNil)
			{
			Debug(vdbs.fCkMwd ? CkMwds() : 0);
			if ((*hmwdCur)->fActive)
				{
				ActivateWn(hmwdCur, fFalse);
				if (mw != mwNone)
					BlankScrollBarsMwd( hmwdCur, fTrue );
				}
			}

		if (mw != mwNone)
			{
			struct MWD * pmwd;
			HWND hwnd;

/* Activate a new child window */
			mwCur = mw;
			hmwdCur = mpmwhmwd[mwCur];

			/* make sure mwd set up properly */
			Assert((*hmwdCur)->wwActive != wwNil);
			Assert(wwCur == wwNil || hwwdCur != hNil);

			pmwd = *hmwdCur;
			if (mwCur != mwPrev && PwwdWw(pmwd->wwActive)->wk == wkMacro)
				{
				SelectEdMacro(mwCur);
				pmwd = *hmwdCur;
				}

			/* Make sure we have the right menu and keymap set up */
			if (mwCur != mwPrev)
				{
				SetupMenu(docNil);
				if (vpref.fZoomMwd && !vrf.fMwCreate /* not creating mw */)
					{
					rcUnZoomSav = vrcUnZoom;
					ZoomWnd((*hmwdCur)->hwnd);
					}
				}

			if (!(*hmwdCur)->fActive)
				{
				ActivateWn(hmwdCur, fTrue);
				BlankScrollBarsMwd( hmwdCur, fFalse );
				}

			/* if mw not already on top, make it be */
			if (GetNextWindow(hwnd = (*hmwdCur)->hwnd, GW_HWNDPREV) != NULL
					&& !vfDeactByOtherApp)
				BringWindowToTop(hwnd);
			if (vpref.fZoomMwd && !vrf.fMwCreate && 
					mwCur != mwPrev && mpmwhmwd[mwPrev] != hNil)
				{
				hwndMwOld = PmwdMw(mwPrev)->hwnd;
				UnZoomWnd(hwndMwOld, &rcUnZoomSav);
				}
			}
		}

	vfWndCacheDirty = fTrue;

	cRecurse--;
}


/* %%Function:ActivateWn %%Owner:chic */
ActivateWn(hmwd, fActive)
struct MWD **hmwd;
BOOL fActive;
{
	HWND hwnd = (*hmwd)->hwnd;
	long ws = GetWindowLong(hwnd, GWL_STYLE);

	if (fActive)
		{
		ws |= WS_SYSMENU | WS_MAXIMIZEBOX;
		}
	else
		{
		ws &= ~(WS_SYSMENU | WS_MAXIMIZEBOX);
		}
	SetWindowLong(hwnd, GWL_STYLE, ws);

	SendMessage(hwnd, WM_NCACTIVATE, fActive, 0L); /* hilight/dehilight caption */
	(*hmwd)->fActive = fActive;
}


/******************************************/
/* B l a n k  S c r o l l  B a r s  M w d */
/* make scroll bars of MWD window blank or non-blank */

/* %%Function:BlankScrollBarsMwd %%Owner:chic */
BlankScrollBarsMwd( hmwd, fBlank )
struct MWD **hmwd;
int fBlank;
{
	struct MWD *pmwd = *hmwd;
	HWND hwnd;

	Assert( hmwd != hNil );
	RSBSetFBlank( pmwd->hwndHScroll, fBlank );
	RSBSetFBlank( pmwd->hwndSizeBox, fBlank );
	RSBSetFBlank( pmwd->hwndSplitBox, fBlank );
	BSBPwwd( PwwdWw(pmwd->wwUpper), fBlank );
	if (pmwd->wwLower != wwNil)
		BSBPwwd( PwwdWw( pmwd->wwLower ), fBlank );
}


/* %%Function:BSBPwwd %%Owner:chic */
static BSBPwwd( pwwd, fBlank )
struct WWD *pwwd;
int fBlank;
{
	HWND hwnd;

	RSBSetFBlank( pwwd->hwndVScroll, fBlank );
	RSBSetFBlank( pwwd->hwndPgvUp, fBlank );
	RSBSetFBlank( pwwd->hwndPgvDown, fBlank );
}


/******************************/
/* R S B  S e t  F B l a n k  */
/* Blank or unblank RSB control */

/* %%Function:RSBSetFBlank %%Owner:chic */
RSBSetFBlank( hwnd, fBlank )
HWND hwnd;
int fBlank;
{
	if (hwnd == NULL)
		return;
	if ( fBlank != GetWindowWord( hwnd, offset(RSBS,fBlank)) )
		{
		SetWindowWord( hwnd, offset(RSBS,fBlank), fBlank );
		RSBPaintAll( hwnd );
		}
}



extern int wwCur;
extern KMP ** hkmpCur;
extern KMP ** hkmpBase;
extern MSG vmsgLast;
extern struct WWD ** hwwdCur;
extern vgrfMenukeysAreDirty;



#define ckmeGrow        10

KME * PkmeFromKc();



/* H K M P  C R E A T E */
/* %%Function:HkmpCreate %%Owner:chic */
KMP ** HkmpCreate(ckmeInit, kmf)
int ckmeInit;
int kmf;
{
	KMP ** hkmp, * pkmp;

	hkmp = (KMP **) HAllocateCw(cwKMP + ckmeInit * cwKME);
	if (hkmp != hNil)
		{
		pkmp = *hkmp;
		pkmp->hkmpNext = hNil;
		pkmp->ikmeMax = ckmeInit;
		pkmp->ikmeMac = 0;
		pkmp->grpf = kmf;
		}

	vgrfMenuKeysAreDirty = 0xffff;
	return hkmp;
}



/* H K M P  N E W */
/* Create a new key-map and link it into the chain up front */
/* %%Function:HkmpNew %%Owner:chic */
KMP ** HkmpNew(ckmeInit, kmf)
int ckmeInit;
int kmf;
{
	KMP ** hkmp;

	hkmp = HkmpCreate(ckmeInit, kmf);
	LinkHkmp(hkmp, kmf);
	return hkmp;
}


/* L I N K  H K M P */
/* %%Function:LinkHkmp %%Owner:chic */
LinkHkmp(hkmp, kmf)
KMP ** hkmp;
{
	if (hkmp == hNil)
		return;

	(*hkmp)->hkmpNext = hkmpCur;
	hkmpCur = hkmp;

	if (!(kmf & kmfAppModal) && hwwdCur != hNil)
		(*hwwdCur)->hkmpCur = hkmp;
}


/* R E M O V E  K M P */
/* Remove the given key-map from the chain and free up its storage */
/* %%Function:RemoveKmp %%Owner:chic */
RemoveKmp(hkmpRemove)
KMP ** hkmpRemove;
{
	Assert(hkmpRemove != hkmpBase); /* Don't remove base! */

	if (FDislinkHkmp(hkmpRemove))
		FreeH(hkmpRemove);
}


/* returns fTrue if hkmpRemove was found in the list */
/* %%Function:FDislinkHkmp %%Owner:chic */
BOOL FDislinkHkmp(hkmpRemove)
KMP ** hkmpRemove;
{
	KMP ** hkmp;
	KMP ** hkmpPrev = NULL;

	if (hkmpRemove == hNil)
		return fFalse;

	vgrfMenukeysAreDirty = 0xffff;

	if (hwwdCur != hNil && hkmpRemove == (*hwwdCur)->hkmpCur)
		(*hwwdCur)->hkmpCur = (*hkmpRemove)->hkmpNext;

	/* Remove the keymap from the chain */
	for (hkmp = hkmpCur; hkmp != hNil; hkmp = (*hkmp)->hkmpNext)
		if (hkmp == hkmpRemove)
			{
			if (hkmpPrev != NULL)
				(*hkmpPrev)->hkmpNext = (*hkmp)->hkmpNext;
			else
				hkmpCur = (*hkmp)->hkmpNext;
			return fTrue;  /* found and removed */
			}
		else
			{
			hkmpPrev = hkmp;
			}
	return fFalse;  /* hkmpRemove not found */
}


/* P K M E  A D D  K C */
/* Returns a pointer to the entry for kc in hkmp.  Entry is created if
it does not already exist.  Assumes caller will set wFoo and kt. */
/* %%Function:PkmeAddKc %%Owner:chic */
KME * PkmeAddKc(hkmp, kc)
KMP ** hkmp;
int kc;
{
	KMP * pkmp;
	KME * pkme;
	int ikme;
	BOOL fFound;

	fFound = FSearchKmp(hkmp, kc, &ikme);
	pkmp = *hkmp;
	pkme = &pkmp->rgkme[ikme];
	if (!fFound)
		{
		if (pkmp->ikmeMac == pkmp->ikmeMax)
			{
			if (!FChngSizeHCw(hkmp,
					cwKMP + (pkmp->ikmeMax + ckmeGrow) * cwKME,
					fFalse))
				{
				return 0;
				}
			pkmp = *hkmp;
			pkmp->ikmeMax += ckmeGrow;
			}
		pkmp = *hkmp;
		pkme = &pkmp->rgkme[ikme];
		bltb(pkme, pkme + 1,
				sizeof (KME) * (pkmp->ikmeMac - ikme));
		++pkmp->ikmeMac;
		pkme->kc = kc;
		}

	return pkme;
}


/* A D D  K E Y  P F N */
/* Bind a key directly to a function */
/* %%Function:AddKeyPfn %%Owner:chic */
AddKeyPfn(hkmp, kc, pfn)
KMP ** hkmp;
int kc;
void (* pfn)();
{
	KME * pkme;

	pkme = PkmeAddKc(hkmp, kc);
	if (pkme != 0)
		{
		pkme->pfn = pfn;
		pkme->kt = ktFunc;
		}
}



/* P I C K  N E W  W W */
/* Some ww is being closed.  Identify new ww to get the focus and
	call NewCurWw on it. */
/* %%Function:PickNewWw %%Owner:chic */
PickNewWw()
{
	extern HWND vhwndDeskTop;
	int ww, mw;
	struct MWD *pmwd;

	Assert( vhwndDeskTop != NULL );
	mw = MwFromHwndMw( GetTopWindow( vhwndDeskTop ) );
	if (mw == mwNil)
		ww = wwNil;
	else
		{
		FreezeHp();
		pmwd = PmwdMw(mw);
		ww = pmwd->wwActive;
		Assert(ww != wwNil && ww < wwMax);
		if (mpwwhwwd[ww] == hNil) /* ww already destroyed, find the other ww */
			ww = (pmwd->wwUpper == ww ? pmwd->wwLower : pmwd->wwUpper);
		Assert(ww >= wwDocMin && ww < wwMac);
		AssertH( mpwwhwwd [ww] );
		MeltHp();
		}
	NewCurWw( ww, fFalse /*fDoNotSelect*/);
}


extern KMP ** hkmpSearch;
extern int ikmeSearch;


/* K C  N E X T  O F  B C M */
/* FUTURE: this could be done much better! */
/* %%Function:KcNextOfBcm %%Owner:chic */
KcNextOfBcm(bcm) /* This is here because of the statics used */
int bcm;
{
	KMP * pkmp;
	KME * pkme;

	for ( ; hkmpSearch != hNil; hkmpSearch = pkmp->hkmpNext)
		{{ /* NATIVE - KcNextOfBcm */
		pkmp = *hkmpSearch;
		pkme = &pkmp->rgkme[ikmeSearch];
		for ( ; ikmeSearch++ < pkmp->ikmeMac; ++pkme)
			{
			if (pkme->kt == ktMacro && pkme->bcm == bcm)
				{{ /* !NATIVE - KcNextOfBcm */
				KME * pkmeT;

				/* Make sure key is not being overridden
					by a previous keymap. */
				pkmeT = PkmeOfKcInChain(pkme->kc);

				if (pkmeT != 0 &&
						pkmeT->kt == ktMacro && pkmeT->bcm == bcm)
					{
					return pkme->kc;
					}
				}}
			}
		ikmeSearch = 0;
		}}

	return kcNil;
}


/* A D D  K E Y  T O  K M P */
/* Bind a key to something in the command table */
/* %%Function:AddKeyToKmp %%Owner:chic */
AddKeyToKmp(hkmp, kc, bcm)
KMP ** hkmp;
int kc;
int bcm;
{
	KME * pkme;

	pkme = PkmeAddKc(hkmp, kc);
	if (pkme != 0)
		{
		pkme->bcm = bcm;
		pkme->kt = ktMacro;
		}
}


/* A D D  A L L  K E Y S  O F  B C M */
/* add all keys currently mapped to bcmSrch to hkmp mapped to bcmUse */
/* %%Function:AddAllKeysOfBcm %%Owner:chic */
AddAllKeysOfBcm(hkmpSearch, hkmpAdd, bcmSrch, bcmAdd)
KMP ** hkmpSearch;
KMP ** hkmpAdd;
int bcmSrch, bcmAdd;
{
	int kc;

	Assert(bcmSrch != bcmNil);
	Assert(bcmAdd != bcmNil);
	Assert(hkmpSearch != hNil);
	Assert(hkmpAdd != hNil);

	kc = KcFirstOfBcm(hkmpSearch, bcmSrch);
	for ( ; kc != kcNil; kc = KcNextOfBcm(bcmSrch))
		{
		AddKeyToKmp(hkmpAdd, kc, bcmAdd);
		}
}


