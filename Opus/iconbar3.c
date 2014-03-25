/* I C O N B A R 3 . C */
/* code for specific icon bar creation and iconbar mode */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "iconbar.h"
#include "disp.h"
#include "wininfo.h"
#include "resource.h"
#include "props.h"
#include "cmdtbl.h"
#include "prompt.h"
#define ICONBAR3C
#include "keys.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "debug.h"
#include "idle.h"
#include "screen.h"

#define USEBCM
#include "cmd.h"

#define OUTLINEIB
#define MCRIB
#define HDRIB
#define PREVIEWIB
#include "ibdefs.h"
#include "help.h"

HDLG HdlgHibsDlgCur(HIBS);

extern HWND		vhwndCBT;
extern HWND             vhwndRibbon;
extern HWND             vhwndAppIconBar;
extern HWND             vhwndApp;
extern HWND             vhwndDeskTop;
extern HWND             vhwndAppModalFocus;
extern HWND             vhwndPrvwIconBar;
extern HWND             vhwndStatLine;
extern int	mwCur;
extern struct WWD **hwwdCur;
extern struct MWD **hmwdCur;
extern KMP **          hkmpCur;
extern MSG              vmsgLast;
extern int	mpibccxt[];
extern IDF		vidf;
extern struct MERR vmerr;
extern KME *PkmeOfKcInChain();
#ifdef WIN23
extern struct SCI vsci;
#endif /* WIN23 */

BOOL                    vfRestoreRibbon = fFalse;
int	vfIconBarMode = fFalse;

csconst char stLinkPrev[] = StSharedKey("&Link to Previous", LinkPrev);
csconst char stSetToDefault[] = StSharedKey("&Reset", SetToDefault);


/* F  C R E A T E  H D R  I C O N  B A R */
/* %%Function:FCreateHdrIconBar %%Owner:PETERJ */
FCreateHdrIconBar(hwwd)
struct WWD **hwwd;
{

#ifdef WIN23
	CHAR ibd [sizeof(ibdHdr2) > sizeof(ibdHdr3) ? sizeof(ibdHdr2) : sizeof(ibdHdr3)];
#else
	CHAR ibd [sizeof(ibdHdr)];
#endif /* WIN23 */
	Assert(hwwd != hNil && (*hwwd)->wk == wkHdr);
	if ((*hwwd)->hwndIconBar != NULL)
		{/* IconBar is already up */
		Assert (fFalse);
		return fFalse;
		}
	Assert ((*hwwd)->hwnd != NULL);
#ifdef WIN23
	if (vsci.fWin3Visuals)
		BltIbd(ibdHdr3, ibd);
	else
		BltIbd(ibdHdr2, ibd);
#else
	BltIbd(ibdHdr, ibd);
#endif /* WIN23 */
	return FCreateWwIconBar(hwwd, ibd);
}


/* S E T U P  I C O N  B A R  B U T T O N */
/* %%Function:SetupIconBarButton %%Owner:PETERJ */
SetupIconBarButton(wwHdr, doc, cp, ihdt)
int	wwHdr;
int	doc;
CP cp;
int	ihdt;
{
	extern struct SEP vsepFetch;

	/* Set up the text to be "Copy From Previous" or "Set To Default".
	Disable "Copy From Previous" if in first section.
	*/

	HWND hwndIb = PwwdWw(wwHdr)->hwndIconBar;
	int	fEditHdr = (ihdt < ihdtMaxSep);
	int	f;
	CHAR stBuf [256];

	Assert(hwndIb);

	/* hide the bitmaps if not in header */
	HideIibbHwndIb(hwndIb, iibbHdrPage, !fEditHdr);
	HideIibbHwndIb(hwndIb, iibbHdrDate, !fEditHdr);
	HideIibbHwndIb(hwndIb, iibbHdrTime, !fEditHdr);

	f = FCanLinkPrev(wwHdr);

#ifdef DISABLE_PJ
	if (fEditHdr)
		{
		int	grpfIhdt = 1 << ihdt;

		CacheSect(doc, cp);
		f = FInFirstSection(doc, cp) ? fFalse : 
				(vsepFetch.grpfIhdt & grpfIhdt);
		}
	else
		f = IhddFromIhdtFtn(doc, ihdt) >= 0;
#endif /* DISABLE_PJ */

	DisableIibbHwndIb(hwndIb, iibbHdrLinkPrev, !f);
	GrayIibbHwndIb(hwndIb, iibbHdrLinkPrev, !f);

	/* "Link to Previous" or "Reset" button */
	if (fEditHdr)
		CopyCsSt((char far * )stLinkPrev, stBuf);
	else
		CopyCsSt((char far * )stSetToDefault, stBuf);
	stBuf[stBuf[0]+1] = 0;
	SetIibbTexthwndIb(hwndIb, iibbHdrLinkPrev, &stBuf[1]);
}


/* F  C R E A T E  O U T L I N E  I C B  B R */
/* %%Function:FCreateOutlineIcnBr %%Owner:PETERJ */
FCreateOutlineIcnBr(hwwd)
struct WWD **hwwd;
{
#ifdef WIN23
	CHAR ibd [sizeof(ibdOutline3) > sizeof(ibdOutline2) ? sizeof(ibdOutline3) : sizeof(ibdOutline2)];
#else
	CHAR ibd [sizeof(ibdOutline)];
#endif /* WIN23 */
	Assert(hwwd != hNil);
	if ((*hwwd)->hwndIconBar != NULL)
		{/* IconBar is already up */
		Assert (fFalse);
		return fFalse;
		}
	Assert ((*hwwd)->hwnd != NULL);
#ifdef WIN23
	if (vsci.fWin3Visuals)
		BltIbd(ibdOutline3, ibd);
	else
		BltIbd(ibdOutline2, ibd);
#else
	BltIbd(ibdOutline, ibd);
#endif /* WIN23 */
	return FCreateWwIconBar(hwwd, ibd);
}




/* F  C R E A T E  M C R  I C O N  B A R */
/* %%Function:FCreateMcrIconBar %%Owner:PETERJ */
FCreateMcrIconBar()
{
#ifdef WIN23
	CHAR ibd [sizeof(ibdMcr3) > sizeof(ibdMcr2) ? sizeof(ibdMcr3) : sizeof(ibdMcr2)];
	if (vsci.fWin3Visuals)
		BltIbd(ibdMcr3, ibd);
	else
		BltIbd(ibdMcr2, ibd);
#else
	CHAR ibd [sizeof(ibdMcr)];
	BltIbd(ibdMcr, ibd);
#endif /* WIN23 */
	return FCreateAppIconBar(ibd);
}


/* F  C R E A T E  W W  I C O N  B A R */
/* %%Function:FCreateWwIconBar %%Owner:PETERJ */
FCreateWwIconBar(hwwd, pibd)
struct WWD **hwwd;
CHAR *pibd;
{
	struct RC rc;

	Assert((*hwwd)->hwndIconBar == NULL);

	/*  get wwd rectangle in mwd coordinates */
	GetHwndParentRc((*hwwd)->hwnd, &rc);

	if (((*hwwd)->hwndIconBar = 
			HwndCreateIconBar(pibd, NULL, PmwdMw((*hwwd)->mw)->hwnd, &rc, NULL)) == NULL)
		{
		ReportSz("HwndCreateIconBar failed!");
		return fFalse;
		}

	return fTrue;
}


/* %%Function:FCreateAppIconBar %%Owner:PETERJ */
FCreateAppIconBar(pibd)
CHAR *pibd;
{
	struct RC rc;
	Assert(vfRestoreRibbon == fFalse);

	if (vfRestoreRibbon = (vhwndRibbon != NULL))
		FTurnRibbonFOn(fFalse, fFalse, fFalse);
	Assert(vhwndAppIconBar == NULL);
	GetClientRect(vhwndApp, (LPRECT) & rc);
	if ((vhwndAppIconBar = HwndCreateIconBar(pibd, NULL, vhwndApp, &rc, NULL)) == NULL)
		{
		ReportSz("HwndCreateIconBar failed!");
		return fFalse;
		}
	if (!vfRestoreRibbon)
		{
		GetDeskTopPrc(&rc);
		MoveWindowRc(vhwndDeskTop, &rc, fTrue);
		}
	ShowWindow(vhwndAppIconBar, SHOW_OPENWINDOW);
	if (!vfRestoreRibbon)
		ResizeMwds();

	return fTrue;
}


/* Preview icon bar */

/* %%Function:FCreatePrvwIconBar %%Owner:PETERJ */
FCreatePrvwIconBar(hwnd, prc)
HWND hwnd;
struct RC *prc;
{
#ifdef WIN23
	CHAR ibd[sizeof(ibdPrvw3) > sizeof(ibdPrvw2) ? sizeof(ibdPrvw3) : sizeof(ibdPrvw2)];
#else
	CHAR ibd[sizeof(ibdPrvw)];
#endif /* WIN23 */

#ifdef WIN23
	if (vsci.fWin3Visuals)
		BltIbd(ibdPrvw3, ibd);
	else
		BltIbd(ibdPrvw2, ibd);
#else
	BltIbd(ibdPrvw, ibd);
#endif /* WIN23 */
	Assert(vhwndPrvwIconBar == NULL);
	if ((vhwndPrvwIconBar = HwndCreateIconBar(&ibd, NULL, hwnd, prc, NULL)) == NULL)
		{
		ReportSz("HwndCreateIconBar failed!");
		return fFalse;
		}
	return fTrue;
}


/* D E S T R O Y  I C O N  B A R */
/* hwwd may be hNil to destroy the app icon bar */
/* %%Function:DestroyIconBar %%Owner:PETERJ */
DestroyIconBar(hwwd)
struct WWD **hwwd;
{
	HWND hwnd, *phwnd;

	if (hwwd == hNil)
		/* destroy app icon bar */
		phwnd = &vhwndAppIconBar;
	else
		phwnd = &(*hwwd)->hwndIconBar;

	hwnd = *phwnd;
	*phwnd = NULL;
	Assert(hwnd != NULL);
	DestroyWindow(hwnd);

	if (hwwd == hNil)
		if (vfRestoreRibbon)
			{
			FTurnRibbonFOn(fTrue, fFalse, fTrue);
			vfRestoreRibbon = fFalse;
			}
		else
			{
			/* close desktop back up */
			struct RC rc;

			GetDeskTopPrc(&rc);
			MoveWindowRc(vhwndDeskTop, &rc, fTrue);
			}
}

csconst KME   rgKmeOutline[] = 
{
/* Defined in keys.h for localization */

rgKmeOutlineDef

	/* These must be added outside of the table because the key codes
		are variables... */
	/*                {kcExpand2,     ktMacro, bcmExpand},
				{kcCollapse2,   ktMacro, bcmCollapse}*/
};


#define cKmeOutline  (sizeof(rgKmeOutline) / sizeof(KME))



/* F  H O O K  O U T L I N E  K M P  H M W D  W W */
/* %%Function:FHookOutlineKmpHmwdWw %%Owner:PETERJ */
BOOL FHookOutlineKmpHmwdWw(hmwd, ww)
struct MWD **hmwd;
int	ww;
{
	struct WWD *pwwd;
	KMP        * *hkmp;
	int	wwUpper, wwOther;

	Assert(hmwd != hNil);
	wwUpper = (*hmwd)->wwUpper;
	wwOther = (wwUpper == ww) ? (*hmwd)->wwLower : wwUpper;

	if (wwOther != wwNil && (PwwdWw(wwOther))->fOutline)
		{
		hkmp = (PwwdWw(wwOther))->hkmpCur;
		PwwdWw(ww)->hkmpCur = hkmp;
#ifdef YXYDEBUG
		CommSzNum(SzShared("Sharing a keymap "), hkmp);
#endif
		}
	else
		{
		if ((hkmp = HkmpNew(cKmeOutline, kmfPlain)) == hNil)
			{
			return fFalse;
			}

#ifdef YXYDEBUG
		CommSzNum(SzShared("cKmeOutline * sizeof(KME)"), cKmeOutline * sizeof(KME));
#endif
		bltbx((KME FAR * ) rgKmeOutline, (KME FAR * ) ((*hkmp)->rgkme),
				cKmeOutline * sizeof(KME));
		(*hkmp)->ikmeMac = cKmeOutline;
		/* These must be added outside of the table because the key codes
		are variables... */
		AddKeyToKmp(hkmp, kcExpand2, bcmExpand);
		AddKeyToKmp(hkmp, kcCollapse2, bcmCollapse);
#ifdef DEBUG
		CheckHkmp(hkmp);
#endif
		}

	return fTrue;
}


/* U N H O O K  O U T L I N E  K M P  H M W D  W W */
/* %%Function:UnhookOutlineKmpHmwdWw %%Owner:PETERJ */
UnhookOutlineKmpHmwdWw(hmwd, ww)
struct MWD **hmwd;
int	ww;
{
	struct WWD *pwwd;
	KMP * *hkmpOutline;
	int	wwUpper, wwOther;

	Assert(hmwd != hNil);
	wwUpper = (*hmwd)->wwUpper;
	wwOther = (wwUpper == ww) ? (*hmwd)->wwLower : wwUpper;
	pwwd = PwwdWw(ww);
	hkmpOutline = pwwd->hkmpCur;
	hkmpCur = pwwd->hkmpCur = (*hkmpOutline)->hkmpNext;

	if (wwOther == wwNil || !PwwdWw(wwOther)->fOutline)
		RemoveKmp(hkmpOutline);
}



/* %%Function:IBReturn  %%Owner:PETERJ */
void IBReturn ()
{
	HIBS hibs = HibsFromHwndIb(vhwndAppModalFocus);
	struct IBB *pibb = PibbFromHibsIibb(hibs, IibbHiliteHibs(hibs));
	Assert(mpibitgrpf[pibb->ibit].fToggle);
	Assert(pibb->bcm != bcmNil);
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, pibb->bcm, 0L);
}


/* %%Function:IBCancel  %%Owner:PETERJ */
void IBCancel ()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, tmcCancel, 0L);
}


/* %%Function:IBHdrPage  %%Owner:PETERJ */
void IBHdrPage ()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, bcmHdrPage, 0L);
}


/* %%Function:IBHdrDate  %%Owner:PETERJ */
void IBHdrDate ()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, bcmHdrDate, 0L);
}


/* %%Function:IBHdrTime %%Owner:PETERJ */
void IBHdrTime ()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, bcmHdrTime, 0L);
}


/* %%Function:IBHdrLinkPrev %%Owner:PETERJ */
void IBHdrLinkPrev ()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, bcmHdrLinkPrev, 0L);
}


/* %%Function:IBTab %%Owner:PETERJ */
void IBTab ()
{
	SetHiliteNext(vhwndAppModalFocus, !vfShiftKey);
}


/* %%Function:IBMoveLeft %%Owner:PETERJ */
void IBMoveLeft ()
{
	SetHiliteNext(vhwndAppModalFocus, fFalse);
}


void IBMoveRight ()
{
	SetHiliteNext(vhwndAppModalFocus, fTrue);
}


/* %%Function:IBHdrRetToDoc %%Owner:PETERJ */
void IBHdrRetToDoc ()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, bcmHdrRetToDoc, 0L);
}


/* %%Function:IBExpand %%Owner:PETERJ */
void IBExpand ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmExpand, 0L);
}


/* %%Function:IBCollapse %%Owner:PETERJ */
void IBCollapse ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmCollapse, 0L);
}


/* %%Function:IBLevel1 %%Owner:PETERJ */
void IBLevel1 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel1, 0L);
}


/* %%Function:IBLevel2 %%Owner:PETERJ */
void IBLevel2 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel2, 0L);
}


/* %%Function:IBLevel3 %%Owner:PETERJ */
void IBLevel3 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel3, 0L);
}


/* %%Function:IBLevel4 %%Owner:PETERJ */
void IBLevel4 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel4, 0L);
}


/* %%Function:IBLevel5 %%Owner:PETERJ */
void IBLevel5 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel5, 0L);
}


/* %%Function:IBLevel6 %%Owner:PETERJ */
void IBLevel6 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel6, 0L);
}


/* %%Function:IBLevel7 %%Owner:PETERJ */
void IBLevel7 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel7, 0L);
}


/* %%Function:IBLevel8 %%Owner:PETERJ */
void IBLevel8 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel8, 0L);
}


/* %%Function:IBLevel9 %%Owner:PETERJ */
void IBLevel9 ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowToLevel9, 0L);
}


/* %%Function:IBExpandAll %%Owner:PETERJ */
void IBExpandAll ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmExpandAll, 0L);
}


/* %%Function:IBMoveUp %%Owner:PETERJ */
void IBMoveUp ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmMoveUp, 0L);
}


/* %%Function:IBMoveDown %%Owner:PETERJ */
void IBMoveDown ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmMoveDown, 0L);
}


/* %%Function:IBConvertToBody %%Owner:PETERJ */
void IBConvertToBody ()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmConvertToBody, 0L);
}


#ifdef ESFICON
/* %%Function:IBToggleEllip %%Owner:NOTUSED */
void IBToggleEllip()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmToggleEllip, 0L);
}


/* %%Function:IBShowFormat %%Owner:NOTUSED */
void IBShowFormat()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowFormat, 0L);
}


#endif /* ESFICON */

/* %%Function:IBTraceMacro %%Owner:PETERJ */
void IBTraceMacro()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmTraceMacro, 0L);
}


/* %%Function:IBAnimateMacro %%Owner:PETERJ */
void IBAnimateMacro()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmAnimateMacro, 0L);
}


/* %%Function:IBContinueMacro %%Owner:PETERJ */
void IBContinueMacro()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmContinueMacro, 0L);
}


/* %%Function:IBStepMacro %%Owner:PETERJ */
void IBStepMacro()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmStepMacro, 0L);
}


/* %%Function:IBShowVars %%Owner:PETERJ */
void IBShowVars()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmShowVars, 0L);
}


/* %%Function:IBPrvwBound %%Owner:PETERJ */
void IBPrvwBound()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmPrvwBound, 0L);
}


/* %%Function:IBPrvwPages %%Owner:PETERJ */
void IBPrvwPages()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmPrvwPages, 0L);
}


/* %%Function:IBPrvwPrint %%Owner:PETERJ */
void IBPrvwPrint()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmPrint, 0L);
}


/* %%Function:IBPageView %%Owner:PETERJ */
void IBPageView()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmPageView, 0L);
}


/* %%Function:IBPrvwClose %%Owner:PETERJ */
void IBPrvwClose()
{
	PostMessage(vhwndAppModalFocus, AMM_TERMINATE, bcmPrintPreview, 0L);
}


/* %%Function:IBGetHelpPreview %%Owner:PETERJ */
void IBGetHelpPreview()
{
	GetHelp(cxtPrintPreviewIconBarMode);
}


/* %%Function:IBGetHelpOutline %%Owner:PETERJ */
void IBGetHelpOutline()
{
	GetHelp(cxtOutlineIconBarMode);
}


/* %%Function:IBGetHelpHdr %%Owner:PETERJ */
void IBGetHelpHdr()
{
	GetHelp(CxtFromIbcHwnd(ibcHdr, vhwndAppModalFocus));
}


/* %%Function:IBGetHelpMacro %%Owner:PETERJ */
void IBGetHelpMacro()
{
	GetHelp(cxtMacroEditingIconBarMode);
}


/* NOTE: this table must be sorted by kc's */
csconst KME rgKmeHdrIconBar[] = 
{
/* Defined in keys.h for localization */

rgKmeHdrIBDef

};


#define ckmeHdrIconBar (sizeof(rgKmeHdrIconBar) / sizeof(KME))

/* NOTE: this table must be sorted by kc's */
csconst KME rgKmeOutlineIconBar[] = 
{
/* Defined in keys.h for localization */

rgKmeOutlineIBDef

};


#define ckmeOutlineIconBar (sizeof(rgKmeOutlineIconBar) / sizeof(KME))

/* NOTE: this table must be sorted by kc's */
csconst KME rgKmeDbgIconBar[] = 
{
/* Defined in keys.h for localization */

rgKmeDbgIBDef

};


#define ckmeDbgIconBar (sizeof(rgKmeDbgIconBar) / sizeof(KME))

/* NOTE: this table must be sorted by kc's */
csconst KME rgKmePreviewIconBar[] = 
{
/* Defined in keys.h for localization */

rgKmePreviewIBDef

};


#define ckmePreviewIconBar (sizeof(rgKmePreviewIconBar) / sizeof(KME))


/* %%Function:CmdIconBarMode %%Owner:PETERJ */
CMD CmdIconBarMode(pcmb)
CMB *pcmb;
{
	/*
	this routine has to be in the same module where the keymaps for
	iconbars are defined
	*/
	KMP * *hkmp;
	KME FAR * lpkme;
	int	ckme;
	int	tmc;
	int	cmdRet;
	HWND hwndIb;
	HIBS hibs;

	if (mwCur == mwNil || hwwdCur == NULL)
		return cmdError;

	if ((hwndIb = vhwndPrvwIconBar) == NULL && 
			(hwndIb = (*hwwdCur)->hwndIconBar) == NULL)
		hwndIb = vhwndAppIconBar;

	if (hwndIb == NULL)
		{
		Beep();
		return cmdError;
		}

	hibs = HibsFromHwndIb(hwndIb);

	switch ((*hibs)->ibc)
		{
	case ibcHdr:
		ckme = ckmeHdrIconBar;
		break;
	case ibcMcr:
		ckme = ckmeDbgIconBar;
		break;
	case ibcOutline:
		ckme = ckmeOutlineIconBar;
		break;
	case ibcPreview:
		ckme = ckmePreviewIconBar;
		break;
#ifdef DEBUG
	default:
		Assert(fFalse);
		return cmdError;
#endif /* DEBUG */
		}

	if ((hkmp = HkmpNew(ckme, kmfStopHere)) == hNil)
		return cmdNoMemory;

	switch ((*hibs)->ibc)
		{
	case ibcHdr:
		lpkme = (KME FAR * ) & rgKmeHdrIconBar;
		break;
	case ibcMcr:
		lpkme = (KME FAR * ) & rgKmeDbgIconBar;
		break;
	case ibcOutline:
		lpkme = (KME FAR * ) & rgKmeOutlineIconBar;
		break;
	case ibcPreview:
		lpkme = (KME FAR * ) & rgKmePreviewIconBar;
		break;
#ifdef DEBUG
	default:
		Assert(fFalse);
		return cmdError;
#endif /* DEBUG */
		}

	FillKmp(hkmp, lpkme, ckme);

	/* hilight the first bitmap */
	Assert(hmwdCur && hwndIb);
	SetHiliteNext(hwndIb, fTrue);

	vfIconBarMode = fTrue;
	cmdRet = cmdOK;

	switch (tmc = WAppModalHwnd (hwndIb, mmoFalseOnMouse))
		{
	case fFalse:
	case tmcCancel:
		cmdRet =  cmdCancelled;
		break;
		}
	if (vfIconBarMode)
		DestroyKmpIB(hwndIb);

	if (cmdRet != cmdCancelled)
		ExecIconBarCmd(tmc, kcNil);

	return cmdRet;
}



/* F I L L  K M P */
/* Given an array of KME's in csconst space, fill a keymap.  It is assumed that
	the keymap was initialized to be the size of the array.  The array must
	be sorted by kc's.
*/
/* %%Function:FillKmp %%Owner:PETERJ */
FillKmp(hkmp, lpkme, ikmeMac)
KMP **hkmp;
KME FAR *lpkme;
int	ikmeMac;
{
	KMP * pkmp;

	pkmp = *hkmp;
	Assert(pkmp->ikmeMax >= ikmeMac);
	bltx(lpkme, (int FAR * ) pkmp->rgkme, ikmeMac * cwKME);
	pkmp->ikmeMac = ikmeMac;

#ifdef DEBUG
	CheckHkmp(hkmp);
#endif
}


/* %%Function:DestroyKmpIB %%Owner:PETERJ */
DestroyKmpIB(hwndIconBar)
HWND hwndIconBar;
{
	Assert(hwwdCur != hNil);
	Assert(vfIconBarMode);

	if (hwwdCur == hNil || !vfIconBarMode)
		return;

	Assert(hwndIconBar);
	RemoveKmp(hkmpCur); /* hkmpCur is set to the next keymap */
	Assert(hkmpCur != hNil);
	(*hwwdCur)->hkmpCur = hkmpCur;
	SetHiliteHibs(HibsFromHwndIb(hwndIconBar), iibbNil);
	vfIconBarMode = fFalse;
}



/* S E T  H I L I T E  I I B B */
/* %%Function:SetHiliteIibb %%Owner:PETERJ */
SetHiliteIibb(hibs, iibb, fOn)
HIBS hibs;
int	iibb;
BOOL fOn;
{
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);

	if (pibb->fHilite == fOn)
		return;

	/* no others supported at the moment */
	Assert(mpibitgrpf[pibb->ibit].fToggle);
#ifdef WIN23
	ToggleButton((*hibs)->hwnd, iibb, fOn);
#else
	HiliteBorder((*hibs)->hwnd, iibb, fOn);
#endif /* WIN23 */
}


/* S E T  H I L I T E  H I B S */
/* %%Function:SetHiliteHibs %%Owner:PETERJ */
SetHiliteHibs(hibs, iibb)
HIBS hibs;
int	iibb;
{
	int	iibbPrev = IibbHiliteHibs(hibs);
	if (iibb != iibbNil)
		SetHiliteIibb(hibs, iibb, fTrue);
	if (iibbPrev != iibb && iibbPrev != iibbNil)
		SetHiliteIibb(hibs, iibbPrev, fFalse);
}


/* I I B B  A F T E R  H I B S */
/* %%Function:IibbAfterHibs %%Owner:PETERJ */
IibbAfterHibs(hibs, iibbFrom, fNext)
HIBS hibs;
int	iibbFrom;
BOOL fNext;
{
	struct IBB *pibb;
	int	iibbMac, iibb;
	int	d = fNext ? 1 : -1;
	Debug( int cLoop = 0 );

	iibbMac = (*hibs)->iibbMac;
	if (iibbFrom == iibbNil)
		iibb = fNext ? 0 : iibbMac - 1;
	else
		iibb = iibbFrom + d;

	pibb = PibbFromHibsIibb(hibs, iibb);

	for (; ; iibb += d, pibb += d)
		{
		Assert(cLoop++ < iibbMac);
		if (iibb >= iibbMac || iibb < 0)
			{
			iibb = fNext ? 0 : iibbMac - 1;
			pibb = PibbFromHibsIibb(hibs, iibb);
			}
		if (mpibitgrpf[pibb->ibit].fToggle && 
				!pibb->fDisabled && !pibb->fHidden)
			return iibb;
		}
}


/* I I B B  H I L I T E  H I B S */
/* %%Function:IibbHiliteHibs %%Owner:PETERJ */
IibbHiliteHibs(hibs)
HIBS hibs;
{
	struct IBB *pibb = (*hibs)->rgibb;
	int	iibb, iibbMac = (*hibs)->iibbMac;

	for (iibb = 0; iibb < iibbMac; iibb++, pibb++)
		{
		if (pibb->fHilite)
			return iibb;
		}
	return iibbNil;
}


/* S E T  H I L I T E  N E X T */
/* %%Function:SetHiliteNext %%Owner:PETERJ */
SetHiliteNext(hwnd, fNext)
HWND hwnd;
BOOL fNext;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	SetHiliteHibs(hibs,
			IibbAfterHibs(hibs,
			IibbHiliteHibs(hibs), fNext));
}



/* S E T  F O C U S  I I B B */
/* %%Function:SetFocusIibb %%Owner:PETERJ */
SetFocusIibb(hwnd, iibb)
HWND hwnd;
int	iibb;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	TMC tmc = PibbFromHibsIibb(hibs, iibb)->tmc;
	HDLG hdlg = PibbFromHibsIibb(hibs, (*hibs)->iibbDlg)->hdlg;
	Assert(PibbFromHibsIibb(hibs, iibb)->ibit == ibitDlgItem);
	Assert(PibbFromHibsIibb(hibs, (*hibs)->iibbDlg)->ibit == ibitDialog);
	HdlgSetCurDlg(hdlg);

	SetTmcTxs(tmc & ~ftmcGrouped, TxsAll());
	if (!FIsDlgDying())
		HdlgSetFocusDlg(hdlg);
}



/* T E R M I N A T E  I B  D L G  M O D E */
/* %%Function:TerminateIBDlgMode %%Owner:PETERJ */
TerminateIBDlgMode(hibs, fOK)
HIBS hibs;
BOOL fOK;
{
	if (vidf.fIBDlgMode)
		{
		if (vhwndStatLine != NULL)
			DisplayHelpPrompt(fFalse);
		  /* none of the ibprocs can depend on this flag set at cancel or term.
			 We get completely out of the dialog state before performing the
			 action so none of those routines can get info from the dialog state.
		  */
		vidf.fIBDlgMode = fFalse;
		if (!FIsDlgDying())
			FKillDlgFocus();
		if (hwwdCur != hNil)
			SetFocus((*hwwdCur)->hwnd);
		CallIbProc(hibs, fOK ? ibmTerm : ibmCancel, iibbNil, fFalse, 0L);
		}
}


/* %%Function:TermCurIBDlg %%Owner:PETERJ */
TermCurIBDlg(fOK)
BOOL fOK;
{

	HDLG hdlgFocus, hdlgOld;

	if (!vidf.fIBDlgMode)
		return;

	/* want to use the dialog that has the focus, which should always
			be the current dialog. If not we will have to force it to
			be, then restore     */
	hdlgFocus = HdlgGetFocus();
	if (hdlgFocus == NULL)
		{
		ReportSz("Warning - no dialog has focus at term");
		Assert (fFalse);
		return;
		}
	hdlgOld = HdlgSetCurDlg(hdlgFocus);

#ifdef BZ
	if (hdlgFocus != hdlgOld && hdlgOld != hNil)
		{
		ReportSz("Warning - current dialog does not have focus");
		CommSzNum(SzShared("TermCurIBDlg hdlgOld "), hdlgOld);
		CommSzNum(SzShared("TermCurIBDlg hdlgFocus "), hdlgFocus);
		}
#endif /* BZ */


	TerminateIBDlgMode(WRefDlgCur(), fOK);

	if (hdlgFocus != hdlgOld)
		HdlgSetCurDlg(hdlgOld); /* restore current dialog */
}


/* I B  D L G  L O O P */
/* %%Function:IBDlgLoop %%Owner:PETERJ */
IBDlgLoop()
{
	FTME ftme;
	while (vidf.fIBDlgMode && 
			GetMessage((LPMSG) & vmsgLast, (HWND)NULL, 0, 0))
		{
		if (vmerr.hrgwEmerg1 == hNil)
			{

			SetErrorMat(matMem);
			TermCurIBDlg(fFalse);
			continue;
			}
		if (vmsgLast.message == WM_KEYDOWN)
			{
			SetOurKeyState();
#ifdef DEBUG	/* special debug key processing */
			if (FProcessDbgKey(KcModified(vmsgLast.wParam)))
				/* message has been dealt with, throw away! */
				continue;
#endif /* DEBUG */
			if (vmsgLast.wParam == VK_F1)
				{
				/*REVIEW iftime(awe):if someone has free time (ha!), here's a good project:*/
				/* This is the "wimp" version.  If the user clicks F1 while in an Iconbar
				dialog, they will be returned to the normal editing view afterwards.  I
				believe that it's possible to return them to the dialog box, but I have
				tried to implement it (code below) and found it to be difficult.  If 
				there is time, I may return to try this again later.  The user ed team
				would prefer the "non-wimp" version, but doesn't feel that it's a 
				critical issue.      -awe
				*/
				int	hid;

				hid = HidOfDlg(HdlgGetFocus());
				TermCurIBDlg (fFalse);
				GetHelp(hid);

#ifdef DOESNT_WORK
				/* This is my attempt at the prefered solution (see note above).  This was
				supposed to return the input focus to the IB dialog box after the user
				finished with help.  It doesn't work.  -awe
				*/
				Assert (vhwndAppModalFocus == NULL);
				vhwndAppModalFocus = GetFocus();
				Assert (vhwndAppModalFocus != NULL);
				GetHelp(HidOfDlg(HdlgGetFocus()));
#endif /* DOESN'T_WORK */

				}
			else
				{
				KME * pkme;

				pkme = PkmeOfKcInChain(KcModified(vmsgLast.wParam & 0xff));
				if (pkme->kt == ktMacro)
					{
					HDLG hdlgFocus = HdlgGetFocus();
					HDLG hdlgOld = HdlgSetCurDlg(hdlgFocus);
					HIBS hibs = WRefDlgCur();
					if ((*hibs)->ibc == ibcRuler && pkme->bcm == bcmApplyStyleDlg)
						{
						TermCurIBDlg(fFalse); /* takes care of resetting current dialog */
						CmdExecBcmKc(bcmApplyStyleDlg, kcNil);
						continue;
						}
					if ((*hibs)->ibc == ibcRibbon && (pkme->bcm == bcmFont || 
							pkme->bcm == bcmFontSize))
						{
						TermCurIBDlg(fFalse);
						CmdExecBcmKc(bcmCharacter, kcNil);
						continue;
						}
					if (hdlgFocus != hdlgOld)
						HdlgSetCurDlg(hdlgOld); /* restore current dialog */
					}
				}
			}

		if (!(ftme = FtmeIsSdmMessage((LPMSG) & vmsgLast)))
			{
			TranslateMessage((LPMSG) & vmsgLast);
			DispatchMessage((LPMSG) & vmsgLast);
			}
		else  if (ftme == ftmeError)
			TermCurIBDlg(fFalse);
		}
}


/* V A L  G E T  I I B B  H W N D  I B */
/* %%Function:ValGetIibbHwndIb %%Owner:PETERJ */
WORD ValGetIibbHwndIb(hwnd, iibb)
HWND hwnd;
int	iibb;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	TMC tmc = pibb->tmc;
	HDLG hdlg;
	int	val;

	/* other cases not yet supported */
	Assert(pibb->ibit == ibitDlgItem);
	hdlg = HdlgHibsDlgCur(hibs);
	val = ValGetTmc(tmc);
	HdlgSetCurDlg(hdlg);
	return val;

}


/* G E T  T E X T  I I B B  H W N D  I B */
/* %%Function:GetTextIibbHwndIb %%Owner:PETERJ */
GetTextIibbHwndIb(hwnd, iibb, sz, cchMax)
HWND hwnd;
int	iibb;
CHAR *sz;
int	cchMax;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	TMC tmc = pibb->tmc;
	HDLG hdlg;

	/* other cases not yet supported */
	Assert(pibb->ibit == ibitDlgItem);
	hdlg = HdlgHibsDlgCur(hibs);
	GetTmcText(tmc, sz, cchMax);
	HdlgSetCurDlg(hdlg);  /* restore current */
}


/* %%Function:ExecIconBarCmd %%Owner:PETERJ */
ExecIconBarCmd(bcm, kc)
BCM bcm;
int	kc;
{
	if (kc != kcNil)
		{
		BOOL fTranslated;
		KME * pkme;

		if ((pkme = PkmeFromKc(kc, &fTranslated)) == 0)
			{
			kc = KcModified(kc);
			if ((pkme = PkmeFromKc(kc, &fTranslated)) == 0)
				return;
			}

		if (pkme->kt != ktMacro)
			{
			FExecKc(kc);
			return;
			}

		bcm = pkme->bcm;
		}


	/* Give CBT veto power over keys... */
	if (kc != kcNil && vhwndCBT && !SendMessage(vhwndCBT, WM_CBTSEMEV, 
			smvCommand, MAKELONG(CxtFromBcm(bcm), 0)))
		return;


	/* Due to major badness in el, the interpreter cannot be
		called directly from more than one place during the
		execution of a macro (it is called several times for
		one run when stepping or continuing).  In an attempt
		to keep Internal Error from hitting the user in the
		face, we post the command so the interpreter will only
		be called from the app window proc... */
	switch (bcm)
		{
		extern BOOL fElActive;

	case bcmTraceMacro:
	case bcmStepMacro:
	case bcmContinueMacro:
	case bcmAnimateMacro:
		if (fElActive)
			ModeError();
		else
			PostMessage(vhwndApp, WM_COMMAND, bcm, 0L);
		break;

	default:
		CmdExecBcmKc(bcm, kc);
		break;
		}
}


