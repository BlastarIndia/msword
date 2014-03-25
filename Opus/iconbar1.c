/* I C O N B A R 1 . C */
/*  Core iconbar routines */

#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "keys.h"
#include "help.h"
#include "screen.h"
#include "wininfo.h"
#include "resource.h"
#include "iconbar.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "debug.h"
#include "idle.h"
#ifdef WIN23
#include "dac.h"	/* so we can hack sdm's internal colors */
#endif /* WIN23 */
extern HWND         vhwndCBT;
extern int          wwCur;
extern struct WWD **hwwdCur;
extern HCURSOR      vhcArrow;
extern struct SCI   vsci;
#ifdef WIN23
extern struct BMI   *mpidrbbmi;
#else
extern struct BMI   mpidrbbmi[];
#endif /* WIN23 */
extern struct PREF  vpref;
extern BOOL         vfHelp;       /* whether we're in Help Mode */
extern long         vcmsecHelp;
extern IDF	    vidf;
extern HWND         vhwndStatLine;
extern HWND         vhwndRibbon;
extern struct MERR  vmerr;
extern int          vdbmgDevice;
#ifdef WIN23
FARPROC		lpfnSdmWndProc;
FARPROC		lpfnIconDlgWndProc;
extern HBRUSH vhbrLtGray;
extern HBRUSH vhbrGray;
extern HBRUSH vhbrWhite;
#endif /* WIN23 */
HDLG HdlgHibsDlgCur(HIBS);

/* W R E F  F R O M  H W N D  I B */
/* %%Function:WRefFromHwndIb %%Owner:peterj */
NATIVE WORD WRefFromHwndIb(hwnd)
HWND hwnd;
{
	AssertH(HibsFromHwndIb(hwnd));
	return (*HibsFromHwndIb(hwnd))->wRef;
}


/* P I B B  F R O M  H I B S  I I B B */
/* %%Function:PibbFromHibsIibb %%Owner:peterj */
NATIVE struct IBB *PibbFromHibsIibb(hibs, iibb)
HIBS hibs;
int iibb;
{
	AssertH(hibs);
	Assert(iibb < (*hibs)->iibbMac);
	return (*hibs)->rgibb + iibb;
}


/* F  M O U S E  H I T  I I B B */
/* %%Function:FMouseHitIibb %%Owner:peterj */
FMouseHitIibb(hibs, iibb, ppibb)
HIBS hibs;
int iibb;
struct IBB **ppibb;
{
	struct IBB *pibb;
	if (iibb == iibbNil)
		return fFalse;
	*ppibb = pibb = PibbFromHibsIibb(hibs, iibb);
	return mpibitgrpf[pibb->ibit].fMouseHit 
			&& !pibb->fHidden && !pibb->fDisabled;
}


/*  I C O N  B A R  W N D  P R O C */

/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:IconBarWndProc %%Owner:peterj */
EXPORT LONG FAR PASCAL IconBarWndProc(hwnd, wMessage, wParam, lParam)
HWND hwnd;
unsigned wMessage;
WORD wParam;
LONG lParam;
{
	HIBS hibs;
	WORD hdlg;

#ifdef DBGIB
	ShowMsg(SzShared("ib"), hwnd, wMessage, wParam, lParam);
#endif  /* DBGIB */

	switch (wMessage)
		{
#ifdef DEBUG
	default: 
		Assert(fFalse); 
		goto LDefault;
#endif /* DEBUG */

#ifdef COMMENT
	case WM_CREATE:
			/* Notification to CBT of window creation has been moved
				to HwndCreateIconBar */
#endif

	case WM_NCDESTROY:
			{
			hibs = HibsFromHwndIb(hwnd);
			Assert((*hibs)->hwnd == hwnd);
			CallIbProc(hibs, ibmDestroy, iibbNil, 0, 0L);
			(*hibs)->hwnd = NULL;
			DestroyHibs(hibs);
			break;
			}

	case WM_LBUTTONDOWN:
			{
			struct PT pt;
			int iibb;
			struct IBB *pibb;

			long LparamCBTFromIbcIibb();

			hibs = HibsFromHwndIb(hwnd);

			if (vfHelp)
				GetHelp(CxtFromIbcHwnd((*hibs)->ibc, hwnd));
			else
				{
				EnsureFocusInPane();
#ifdef BRADCH
				SpecialSelModeEnd();
#endif
				if (vmerr.hrgwEmerg1 == hNil)
					{
					SetErrorMat(matMem);
					break;
					}
				pt = MAKEPOINT(lParam);
				iibb = IibbFromHibsPt(hibs, pt);

				if (!FMouseHitIibb(hibs, iibb, &pibb))
					CallIbProc(hibs, ibmClick, iibb, wParam, lParam);

				else  /* toggle */					
					{
					struct RC rc;
					BOOL fHilite;
					BCM bcm = pibb->bcm;

					Assert(mpibitgrpf[pibb->ibit].fToggle);
					SetCapture(hwnd);
#ifdef WIN23
					ToggleButton(hwnd, iibb, fHilite = fTrue);

#else				
					HiliteBorder(hwnd, iibb, fHilite = fTrue);
#endif /* WIN23 */
					rc = pibb->rc;
					while (FStillDownReplay(&pt,fFalse))
						if (PtInRect((LPRECT)&rc, pt) != fHilite)
#ifdef WIN23
							ToggleButton(hwnd, iibb, fHilite = !fHilite);
#else
							HiliteBorder(hwnd, iibb, fHilite = !fHilite);
#endif /* WIN23 */
					ReleaseCapture();
					if (fHilite)
						{
#ifdef WIN23
						if (vsci.fWin3Visuals)
							{	/* Heap may have moved */
							pibb = PibbFromHibsIibb(hibs, iibb);
							if (!pibb->fLatch)
								ToggleButton(hwnd, iibb, fFalse);
							else
								{
								/* if the button is already on, it won't get redraw
									by the UpdateRuler code (since it thinks nothing
									has changed), so we have to force it to redraw here.
									Also, in some cases we won't redraw from the hilited
									state back to the off state.
									*/
								pibb->fHilite = fFalse;
								InvalidateRect(hwnd, &pibb->rc, fFalse);
								}
							}
						else
							ToggleButton(hwnd, iibb, fFalse);
#else
						HiliteBorder(hwnd, iibb, fFalse);
#endif /* WIN23 */
						NewCurWw(wwCur, fFalse /*fDoNotSelect*/);
						/* Give CBT veto power over this action */
						if (!vhwndCBT || 
								SendMessage(vhwndCBT, WM_CBTSEMEV, SmvFromIbc((*hibs)->ibc), 
								LparamCBTFromIbcIibb((*hibs)->ibc, iibb)))
							{
							if (bcm != bcmNil)
								ExecIconBarCmd(bcm, kcNil);
							else
								CallIbProc(hibs, ibmCommand, iibb, 0, 0L);
							}
						}
					}
				}
			break;
			}

	case WM_LBUTTONDBLCLK:
			{
			int iibb;
			struct IBB *pibb;

			if (GetMessageTime() < vcmsecHelp)  /* Abort if help mode */
				return(fTrue);
			EnsureFocusInPane();
			if (vmerr.hrgwEmerg1 == hNil)
				{
				SetErrorMat(matMem);
				break;
				}
			hibs = HibsFromHwndIb(hwnd);
			if (!FMouseHitIibb(hibs, 
					iibb = IibbFromHibsPt(hibs, MAKEPOINT(lParam)), &pibb))
				CallIbProc(hibs, ibmDblClk, iibb, wParam, lParam);
			break;
			}

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) != SC_KEYMENU)
			goto LDefault;
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		ExecIconBarCmd(bcmNil, wParam);
		break;

	case WM_PAINT:
		/* paint each item */
		PaintHwndIb(hwnd);
		break;

	case WM_SETVISIBLE:
		hibs = HibsFromHwndIb(hwnd);
		if (hibs != hNil && (*hibs)->iibbDlg != iibbNil)
			{
			hdlg = HdlgHibsDlgCur(hibs);
		 		/*  not trustworthy if dlg to come down */
			if (!FIsDlgDying())
				ShowDlg(wParam);
			/* restore previous current dialog */
			HdlgSetCurDlg(hdlg);
			}
		goto LDefault;

	case WM_MOVE:
		hibs = HibsFromHwndIb(hwnd);
		hdlg = hNil;
		if (hibs != hNil && (*hibs)->iibbDlg != iibbNil)
			{
			struct RC rc;
			struct IBB *pibb;
			hdlg = HdlgHibsDlgCur(hibs);
		     /* routine not trustworthy if dlg to come down */
			if (!FIsDlgDying())
				{
				GetHwndParentRc(hwnd, &rc);
				pibb = PibbFromHibsIibb(hibs, (*hibs)->iibbDlg);
				MoveDlg(rc.xpLeft + pibb->xpDlg, rc.ypTop + pibb->ypDlg);
				}
			}
		if (hwnd != vhwndRibbon)
			CallIbProc(hibs, ibmMoveSize, iibbNil, 0, 0L);
		/* restore previous current dialog */
		if (hdlg != hNil)
			HdlgSetCurDlg(hdlg);
		goto LDefault;

	case WM_SIZE:
		if (hwnd != vhwndRibbon)
			CallIbProc(HibsFromHwndIb(hwnd), ibmMoveSize, iibbNil, 0, 0L);
		goto LDefault;

	case WM_ENABLE:
		hibs = HibsFromHwndIb(hwnd);
		if (hibs != hNil && (*hibs)->iibbDlg != iibbNil)
			{
			struct RC rc;
			struct IBB *pibb;
			int iibb, iibbMac = (*hibs)->iibbMac;
			hdlg = HdlgHibsDlgCur(hibs);
			for (iibb = 0, pibb = (*hibs)->rgibb; iibb < iibbMac;
					iibb++, pibb++)
				if (pibb->ibit == ibitDlgItem)
					{
					EnableTmc(pibb->tmc, wParam);
					pibb = PibbFromHibsIibb(hibs, iibb);
					}
			/* restore previous current dialog */
			HdlgSetCurDlg(hdlg);
			}
		/* fall through */
LDefault:
		return DefWindowProc(hwnd, wMessage, wParam, lParam);

		}

	return fTrue;
}

#ifdef WIN23
long EXPORT FAR PASCAL IconDlgWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	DWORD clrWindow;
	LONG rval;
	if (message == WM_ERASEBKGND || message == WM_PAINT)
		{
		clrWindow = dac.clrWindow;
		dac.clrWindow = vdbmgDevice != dbmgEGA3 ? rgbLtGray : rgbGray;
		}
	rval = CallWindowProc(lpfnSdmWndProc, hwnd, message, wParam, lParam);
	if (message == WM_ERASEBKGND || message == WM_PAINT)
		{
		dac.clrWindow = clrWindow;
		}
	return (rval);
}

#endif /* WIN23 */
/* F  D L G  I B */
/* %%Function:FdlgIb %%Owner:peterj */
BOOL FDlgIb(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	HIBS hibs;
	BOOL fRet = fTrue;


	hibs = (HIBS)WRefDlgCur();

#ifdef DBGIB
	CommSzNum(SzShared("FDlgIb: dlm = "), dlm);
	CommSzNum(SzShared("\thibs = "), hibs);
#endif

	switch (dlm)
		{
	case dlmDlgDblClick:
		if (GetMessageTime() < vcmsecHelp)
			{
			fRet = fFalse;
			goto LRet;
			}
		EnsureFocusInPane();
		CallIbProc(hibs, ibmDblClk, iibbNil, 0, 0L);
		fRet = fFalse;
		goto LRet;

	case dlmDlgClick:
		/* SDM gets the focus, send it back to the pane */
#ifdef DBGIB
		CommSz(SzShared("Iconbar refusing focus.\r\n"));
#endif /* DBGIB */
		if (!vidf.fIBDlgMode)
			SetFocus(hwwdCur == hNil ? NULL : (*hwwdCur)->hwnd);
		else
			TermCurIBDlg(fFalse); /* "escape" without applying */
		fRet = fFalse;
		goto LRet;

	case dlmSetDlgFocus:
		if (!vidf.fIBDlgMode)
			{
#ifdef DBGIB
			CommSz(SzShared("starting dialog\r\n"));
#endif /* DBGIB */
#ifdef RSH
			LogUa(uaIBDlgGetFocus);
			SetUatMode(uamCommand);
#endif /* RSH */
			vidf.fIBDlgMode = fTrue;
			if (vhwndStatLine != NULL)
				DisplayHelpPrompt(fTrue);
			CallIbProc(hibs, ibmInit, iibbNil, 0, 0L);
			}
		break;

	case dlmKillDlgFocus:
		if (vidf.fIBDlgMode)
			/* verify dialog and apply to document */
			{
#ifdef DBGIB
			CommSz(SzShared("ending dialog\r\n"));
#endif /* DBGIB */
#ifdef RSH
			LogUa(uaIBDlgLoseFocus);
#endif /* RSH */

			vidf.fIBDlgMode = fFalse;
			if (vhwndStatLine != NULL)
				DisplayHelpPrompt(fFalse);

			CallIbProc(hibs, ibmTerm, iibbNil, fTrue, 0L);
			}
		break;

	case dlmChange:
		CallIbProc(hibs, ibmChange, IibbFromHibsTmc(hibs, tmc), 0, 0L);
		break;

	case dlmSetItmFocus:
		/* note we pass whether the old tmc was tmc or tmc + 1. This is
			for the ribbon, who cares it it is from the list
			box of a combo or from itself
		*/
		CallIbProc(hibs, ibmSetItmFocus, IibbFromHibsTmc(hibs, tmc),
				wOld == tmc + 1 || wOld == tmc, 0L);
		break;

	case dlmDblClk:
		/* note: the edit control is parsed, so it returns tmvWord,
			not tmvString. The list should be NoType and that is the
			only time we should do this
		*/
		if (TmvGetTmc(tmc) == tmvNoType)  /* don't do it on edit control */
			TermCurIBDlg(fTrue);
		break;

	case dlmKey:
		if (wNew == kcReturn)
			TermCurIBDlg(fTrue);
		else  if (wNew == kcEscape)
			TermCurIBDlg(fFalse);
		else
			Beep();
		break;

		}

LRet:
	return fRet;
}


/* C A L L  I B  P R O C */

/* %%Function:CallIbProc %%Owner:peterj */
CallIbProc(hibs, ibm, iibb, wParam, lParam)
HIBS hibs;
WORD ibm;
WORD iibb;
WORD wParam;
LONG lParam;
{
	PFN     pfn;
#ifdef DBGIB
	int rgw[6];
	rgw[0]=hibs;
	rgw[1]=ibm;
	rgw[2]=iibb;
	rgw[3]=wParam;
	rgw[4]=LOWORD(lParam);
	rgw[5]=HIWORD(lParam);
	CommSzRgNum(SzShared("CallIbProc: "),rgw,6);
#endif /* DBGIB */

	Debug( if (vdbs.fShakeHeap) ShakeHeap(); )
		Debug( if (vdbs.fCkHeap) CkHeap(); )

			if (hibs)
				{
				AssertH(hibs);
				if ((pfn = (*hibs)->pfn) != NULL)
					(*pfn)((*hibs)->hwnd, ibm, iibb, wParam, lParam);
				}
}


/* I I B B  F R O M  H I B S  P T */

/* %%Function:IibbFromHibsPt %%Owner:peterj */
IibbFromHibsPt(hibs, pt)
HIBS hibs;
struct PT pt;
{
	struct IBB *pibb = (*hibs)->rgibb;
	int iibb, iibbMac = (*hibs)->iibbMinSys;

	for (iibb = 0; iibb < iibbMac; iibb++, pibb++)
		if (mpibitgrpf[pibb->ibit].fHasCoord)
			if (PtInRect((LPRECT)&pibb->rc, pt))
				return iibb;

	return iibbNil;
}


/* I I B B  F R O M  H I B S  T M C */
/* %%Function:IibbFromHibsTmc %%Owner:peterj */
IibbFromHibsTmc(hibs, tmc)
HIBS hibs;
TMC tmc;
{
	int iibb, iibbMac = (*hibs)->iibbMac;
	struct IBB *pibb = (*hibs)->rgibb;

	for (iibb = 0; iibb < iibbMac; iibb++, pibb++)
		if (pibb->ibit == ibitDlgItem && 
				(pibb->tmc|ftmcGrouped) == (tmc|ftmcGrouped))
			return iibb;

	return iibbNil;
}


/* P A I N T  H W N D  I B */
/* %%Function:PaintHwndIb %%Owner:peterj */
PaintHwndIb(hwnd)
HWND hwnd;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	int iibb, iibbMac = (*hibs)->iibbMac;
	int ibit;
	struct IBB *pibb;
	PAINTSTRUCT ps;

	BeginPaint(hwnd, &ps);
#ifdef WIN23
	FSetDcAttribs(ps.hdc, vsci.dccIconBar);
	if (vsci.fWin3Visuals)
		SetBkColor(ps.hdc, vdbmgDevice != dbmgEGA3 ? rgbLtGray : rgbGray);
#else
	FSetDcAttribs(ps.hdc, dccIconBar);
#endif /* WIN23 */

#ifdef WIN23
	if (vsci.fWin3Visuals)
		DrawShadowLines(hwnd, ps.hdc, fTrue, fTrue);
#endif /* WIN23 */

	FreezeHp();

	for (iibb = 0, pibb = &(*hibs)->rgibb; iibb < iibbMac; iibb++, pibb++)
		if (!pibb->fHidden && mpibitgrpf[ibit = pibb->ibit].fPaint)
			{
			int nSaveDc = 0;
			struct RC rcInter;

			if (mpibitgrpf[ibit].fHasCoord &&
					!IntersectRect((LPRECT)&rcInter, (LPRECT)&pibb->rc, 
					(LPRECT)&ps.rcPaint))
				/* nothing to draw for this item */
				continue;

			switch (ibit)
				{
			case ibitText:
#ifdef WIN23
				/****
				IbTextOut no longer blots out the background, so it
				is now the caller's responsibility to erase it.
				****/
				ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &pibb->rc, NULL, 0, (LPINT)NULL);
#endif /* WIN23 */
				if (pibb->hsz != hNil)
					IbTextOut(ps.hdc, &pibb->rc, *pibb->hsz, fFalse,
							pibb->fGray);
				break;

			case ibitToggleBmp:
			case ibitToggleText:
				PaintToggle(hwnd, ps.hdc, pibb);
				break;

			case ibitCustomWnd:
				MeltHp();
				UpdateWindow(pibb->hwndCus);
				FreezeHp();
				pibb = PibbFromHibsIibb(hibs, iibb);
				break;

			case ibitDialog:
				MeltHp();
				UpdateWindow(HwndFromDlg(pibb->hdlg));
				FreezeHp();
				pibb = PibbFromHibsIibb(hibs, iibb);
				break;

#ifdef DEBUG
			default: 
				Assert(fFalse); 
				break;
#endif /* DEBUG */
				}
			}
	MeltHp();

	EndPaint(hwnd, &ps);
}


/* U P D A T E  I I B B */
/* %%Function:UpdateIibb %%Owner:peterj */
UpdateIibb(hibs, iibb, fErase)
HIBS hibs;
int iibb;
BOOL fErase;
{
	InvalidateRect((*hibs)->hwnd, 
			(LPRECT)&PibbFromHibsIibb(hibs, iibb)->rc, fErase);
	UpdateWindow((*hibs)->hwnd);
}


/* H D L G   H I B S  */
/* Returns hdlg assocuiated with hibs.  */
/* %%Function:HdlgHibs %%Owner:bobz */
HDLG HdlgHibs(hibs)
HIBS hibs;
{
			Assert(PibbFromHibsIibb(hibs, (*hibs)->iibbDlg)->ibit == ibitDialog);
	return (PibbFromHibsIibb(hibs, (*hibs)->iibbDlg)->hdlg);
}


/* H D L G   H I B S  D L G  C U R */
/* Set current dialog to be that of the hibs. Returns previous current hdlg */
/* %%Function:HdlgHibsDlgCur %%Owner:bobz */
HDLG HdlgHibsDlgCur(hibs)
HIBS hibs;
{
	return (HdlgSetCurDlg(HdlgHibs(hibs)));
}


/* S E T  I I B B  T E X T  H W N D  I B */
/* %%Function:SetIibbTextHwndIb %%Owner:peterj */
SetIibbTextHwndIb(hwnd, iibb, sz)
HWND hwnd;
int iibb;
CHAR *sz;
{
	HDLG hdlg;
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);

	switch (pibb->ibit)
		{
	case ibitToggleText:
	case ibitText:
		if (pibb->hsz != hNil)
			{
			AssertH(pibb->hsz);
			FreeH(pibb->hsz);
			}
		/* ok to store hNil into hsz */
		PibbFromHibsIibb(hibs, iibb)->hsz = HszCreate(sz); /* HM */
		UpdateIibb(hibs, iibb, fFalse);
		break;

	case ibitDlgItem:
			{
			TMC tmc = pibb->tmc;
			hdlg = HdlgHibsDlgCur(hibs);
			SetTmcText(tmc, sz);
			HdlgSetCurDlg(hdlg);
			break;
			}

#ifdef DEBUG
	default: 
		Assert(fFalse); 
		break;
#endif /* DEBUG */
		}
}


/* G R A Y  I I B B  H W N D  I B */
/* %%Function:GrayIibbHwndIb %%Owner:peterj */
GrayIibbHwndIb(hwnd, iibb, fGray)
HWND hwnd;
int iibb;
BOOL fGray;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);

	switch (pibb->ibit)
		{
	case ibitToggleText:
	case ibitToggleBmp:
	case ibitText:
		if (pibb->fGray != fGray)
			{
			pibb->fGray = fGray;
			UpdateIibb(hibs, iibb, fFalse);
			}
		break;

#ifdef DEBUG
	default: 
		Assert(fFalse); 
		break;
#endif /* DEBUG */
		}
}


/* S E T  V A L  I I B B  H W N D  I B */
/* %%Function:SetValIibbHwndIb %%Owner:peterj */
SetValIibbHwndIb(hwnd, iibb, val)
HWND hwnd;
int iibb;
int val; /* many things */
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	HDLG hdlg;

	switch (pibb->ibit)
		{
	case ibitDlgItem:
			{
			TMC tmc = pibb->tmc;
			hdlg = HdlgHibsDlgCur(hibs);
			SetTmcVal(tmc,val);
			HdlgSetCurDlg(hdlg);
			break;
			}

	case ibitToggleText:
	case ibitToggleBmp:
			/* val: -1 - gray
						0 - off
					+1 - on  */
			{
			int fOn = (val > 0);
			int fGray = (val < 0);
			if (fGray != pibb->fGray)
				{
				pibb->fGray = fGray;
				pibb->fOn = fOn;
#ifdef WIN23
				/* also done in TurnOnToggle. If fOn and fHilite,
					the button won't get lit, so be sure to turn
					off fHilite when not needed.
				*/
				if (vsci.fWin3Visuals)
					pibb->fHilite = fFalse;
#endif /* WIN23 */
				UpdateIibb(hibs, iibb, fFalse);
				}
			else
				TurnOnToggle(hwnd, pibb, fOn);
			break;
			}

	case ibitText:
		/* val is pch to text */
		SetIibbTextHwndIb(hwnd, iibb, val);
		break;

#ifdef DEBUG
	default: 
		Assert(fFalse); 
		break;
#endif /* DEBUG */
		}
}


/* S E L E C T  R A D I O  I I B B */
/* %%Function:SelectRadioIibb %%Owner:peterj */
SelectRadioIibb(hwnd, iibbFirst, iibbLast, iibbSelect, fGray)
HWND hwnd;
int iibbFirst, iibbLast, iibbSelect;
BOOL fGray;
{
	int val;

	for (; iibbFirst <= iibbLast; iibbFirst++)
		{
		val = 0;        /* default is off */
		if (iibbSelect == iibbNil) /* all off or gray */
			{
			if (fGray)
				val = -1;
			}
		else  if (iibbSelect == iibbFirst)
			{
			if (fGray)
				val = -1;
			else
				val = 1;
			}
		SetValIibbHwndIb(hwnd, iibbFirst, val);
		}
}



/* Rounded toggle routines */


/* %%Function:GetToggleRc %%Owner:peterj */
NATIVE GetToggleRc(prc, prcInside)
struct RC *prc;
struct RC *prcInside;
{
	int dxp = vsci.dxpBorder << 1;
	int dyp = vsci.dypBorder << 1;
#ifdef WIN23
	if (vsci.fWin3Visuals)
		{
		*prcInside = *prc;
		return;
		}
#endif /* WIN23 */
	prcInside->xpLeft = prc->xpLeft + dxp;
	prcInside->xpRight = prc->xpRight - dxp;
	prcInside->ypTop = prc->ypTop + dyp;
	prcInside->ypBottom = prc->ypBottom - dyp;
}


/* %%Function:SetRgRcs %%Owner:peterj */
NATIVE SetRgRcs(i, fVert, xp, yp, dzp, rgrc1, rgrc2)
int i;
BOOL fVert;
int xp, yp, dzp;
struct RC rgrc1[], rgrc2[];
{
	struct DRC drc;

	drc.xp = xp;
	drc.yp = yp;
	drc.dxp = fVert ? vsci.dxpBorder : dzp;
	drc.dyp = fVert ? dzp : vsci.dypBorder;
	if (rgrc1)
		DrcToRc(&drc, rgrc1 + i);
	if (fVert)
		drc.xp += vsci.dxpBorder;
	else
		drc.yp += vsci.dypBorder;
	if (rgrc2)
		DrcToRc(&drc, rgrc2 + i);
}


#define crcNor 8
#define crcInv 4

/* if non-null, rgrcs must have crcNor/Inv elements */
/* %%Function:GetToggleBorderRcs %%Owner:peterj */
NATIVE GetToggleBorderRcs(prc, rgrcNor, rgrcInv)
struct RC *prc;
struct RC rgrcNor[];
struct RC rgrcInv[];
{
	int dxp1 = vsci.dxpBorder;
	int dxp2 = dxp1 << 1;
	int dxp4 = dxp2 << 1;
	int dyp1 = vsci.dypBorder;
	int dyp2 = dyp1 << 1;
	int dyp4 = dyp2 << 1;
	int dxp, dyp;
	struct RC rc;

	if (rgrcNor != NULL)
		SetBytes(rgrcNor, 0, sizeof(struct RC)*crcNor);
	if (rgrcInv != NULL)
		SetBytes(rgrcInv, 0, sizeof(struct RC)*crcInv);

	rc = *prc;
	dxp = rc.xpRight - rc.xpLeft;
	dyp = rc.ypBottom - rc.ypTop;

	if (dxp - dxp4 < dxp2 || dyp - dyp4 < dyp2)
		/* if toggle too small, resort to rectangle */
		{
#ifdef DEBUG
		static BOOL fReported = fFalse;
		if (!fReported)
			{
			fReported = fTrue;
			ReportSz("res to low for round rect");
			}
#endif /* DEBUG */
		SetRgRcs(0, fTrue, rc.xpLeft, rc.ypTop, dyp, rgrcNor, NULL);
		SetRgRcs(1, fTrue, rc.xpRight-dxp2, rc.ypTop, dyp, NULL, rgrcNor);
		SetRgRcs(2, fFalse, rc.xpLeft, rc.ypTop, dxp, rgrcNor, NULL);
		SetRgRcs(3, fFalse, rc.xpLeft, rc.ypBottom-dyp2, dxp, NULL, rgrcNor);
		return;
		}

/* left */
	SetRgRcs(0, fTrue, rc.xpLeft, rc.ypTop+dyp2, dyp-dyp4, 
			rgrcNor, rgrcInv);
/* Right */
	SetRgRcs(1, fTrue, rc.xpRight-dxp2, rc.ypTop+dyp2, dyp-dyp4, 
			rgrcInv, rgrcNor);
/* Top */
	SetRgRcs(2, fFalse, rc.xpLeft+dxp2, rc.ypTop, dxp-dxp4, 
			rgrcNor, rgrcInv);
/* Bottom */
	SetRgRcs(3, fFalse, rc.xpLeft+dxp2, rc.ypBottom-dyp2, dxp-dxp4, 
			rgrcInv, rgrcNor);

/* top left */
	SetRgRcs(4, fFalse, rc.xpLeft+dxp1, rc.ypTop+dyp1, dxp1, rgrcNor, NULL);
/* top right */
	SetRgRcs(5, fFalse, rc.xpRight-dxp2, rc.ypTop+dyp1, dxp1, rgrcNor, NULL);
/* bottom left */
	SetRgRcs(6, fFalse, rc.xpLeft+dxp1, rc.ypBottom-dyp2, dxp1, rgrcNor, NULL);
/* bottom right */
	SetRgRcs(7, fFalse, rc.xpRight-dxp2, rc.ypBottom-dyp2, dxp1, rgrcNor, NULL);
}




/* P A I N T  B O R D E R S */

/* %%Function:PaintBorders %%Owner:peterj */
PaintBorders(hdc, pibb)
HDC hdc;
struct IBB *pibb;
{
	int irc;
	HBRUSH hbrSave = SelectObject(hdc, vsci.hbrBorder);
	struct RC *prc, *prcMax;
	struct RC rgrcNor[crcNor], rgrcInv[crcInv];

	GetToggleBorderRcs(&pibb->rc, rgrcNor, rgrcInv);

	for (prc = rgrcNor, prcMax = prc+crcNor; prc < prcMax; prc++)
		PatBltRc(hdc, prc, PATCOPY);

	if (pibb->fOn || !pibb->fHilite)
		SelectObject(hdc, vsci.hbrBkgrnd);
	for (prc = rgrcInv, prcMax = prc+crcInv; prc < prcMax; prc++)
		{
		PatBltRc(hdc, prc, PATCOPY);
		if (pibb->fOn && !pibb->fHilite)
			InvertRect(hdc, (LPRECT)prc);
		}

	if (hbrSave != NULL)
		SelectObject(hdc, hbrSave);
}


/* H I L I T E  B O R D E R */
/* %%Function:HiliteBorder %%Owner:peterj */
#ifdef WIN23
ToggleButton2(hwnd, iibb, fHilite)
#else
HiliteBorder(hwnd, iibb, fHilite)
#endif /* WIN23 */
HWND hwnd;
int iibb;
BOOL fHilite;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	HDC hdc;

	Assert(pibb->ibit == ibitToggleText || pibb->ibit == ibitToggleBmp);

	if (fHilite == pibb->fHilite || (hdc = GetDC(hwnd)) == NULL)
		return;

	pibb->fHilite = fHilite;
	PaintBorders(hdc, pibb);
	ReleaseDC (hwnd, hdc);
}


/* T U R N  O N  T O G G L E */
/* %%Function:TurnOnToggle %%Owner:peterj */
TurnOnToggle(hwnd, pibb, fOn)
HWND hwnd;
struct IBB *pibb;
BOOL fOn;
{
	HDC hdc;
	struct RC rcInside;

#ifdef WIN23
	if (fOn == pibb->fOn && !(vsci.fWin3Visuals && pibb->fHilite))
#else
	if (fOn == pibb->fOn)
#endif /* WIN23 */
		return;

	pibb->fOn = fOn;
#ifdef WIN23
	pibb->fHilite = fFalse;
#endif /* WIN23 */
	if (!IsWindowVisible(hwnd))
		return;
	if ((hdc = GetDC(hwnd)) == NULL)
		return;

#ifdef WIN23
	if (vsci.fWin3Visuals)
		{
		FSetDcAttribs(hdc, vsci.dccIconBar);
		SetBkColor( hdc, vdbmgDevice != dbmgEGA3 ? rgbLtGray : rgbGray);
		PaintToggle3(hwnd, hdc, pibb);
		}
	else
		{
		GetToggleRc(&pibb->rc, &rcInside);
		InvertRect(hdc, (LPRECT)&rcInside);
		PaintBorders(hdc, pibb);
		}
#else
	GetToggleRc(&pibb->rc, &rcInside);
	InvertRect(hdc, (LPRECT)&rcInside);
	PaintBorders(hdc, pibb);
#endif /* WIN23 */

	ReleaseDC (hwnd, hdc);
}


/* P A I N T  T O G G L E */
/* %%Function:PaintToggle %%Owner:peterj */
#ifdef WIN23
PaintToggle2(hwnd, hdc, pibb)
#else
PaintToggle(hwnd, hdc, pibb)
#endif /* WIN23 */
HWND hwnd;
HDC hdc;
struct IBB *pibb;
{
	struct RC rc;

	if (!IsWindowVisible(hwnd))
		return;
	FreezeHp();
	GetToggleRc(&pibb->rc, &rc);

	if (pibb->ibit == ibitToggleBmp)
		{
		int idrb = pibb->idrb;
		int idcb = pibb->idcb;
		struct BMI *pbmi;
		struct MDCD *pmdcd;
		int xpSrc;

		pmdcd = PmdcdCacheIdrb(idrb, hdc);
		if (pmdcd != NULL)
			{
			pbmi = &mpidrbbmi[idrb];
			xpSrc = idcb * pbmi->dxpEach;
			if (pibb->fGray)
				xpSrc += (pbmi->dxp>>1);

			BitBlt(hdc, rc.xpLeft, rc.ypTop,
					pbmi->dxpEach, pbmi->dyp,
					pmdcd->hdc,
					xpSrc, 0,
					vsci.dcibScreen.ropBitBlt);
			}
		else
			PatBltRc(hdc, &rc, vsci.ropErase);
		}

	else if /* ibitToggleText */
	(pibb->hsz != hNil)
		IbTextOut(hdc, &rc, *pibb->hsz, fTrue, pibb->fGray);

	PaintBorders(hdc, pibb);
	if (pibb->fOn)
		InvertRect(hdc, (LPRECT)&rc);

	MeltHp();
}


/* I B  T E X T  O U T */
/* %%Function:IbTextOut %%Owner:peterj */
#ifdef WIN23
IbTextOut2(hdc, prc, pch, fCenter, fGray)
#else
IbTextOut(hdc, prc, pch, fCenter, fGray)
#endif /* WIN23 */
HDC hdc;
struct RC *prc;
CHAR *pch;
BOOL fCenter, fGray;
{
	int cch, ichAcc = -1;
	int xp, yp, dxp, dyp;
	CHAR *pchDest;
	CHAR rgch[256];

	/*  copy string looking for accelerators */
	for (pchDest = rgch; *pch; pch++)
		if (*pch != '&')
			*pchDest++ = *pch;
		else
			ichAcc = pchDest - rgch;

	cch = pchDest - rgch;
	Assert(cch < 256);
	*pchDest = 0;

#ifdef WIN23
	if (vsci.fWin3)
		dxp = LOWORD(GetTextExtent(hdc, (LPSTR)rgch, cch));
	else
#endif /* WIN23 */
	dxp = vsci.dxpTmWidth * cch;
	dyp = vsci.dypTmHeight;

	if (fCenter)
		{
		xp = prc->xpLeft + (prc->xpRight - prc->xpLeft - dxp)/2;
		yp = prc->ypTop + (prc->ypBottom - prc->ypTop - dyp)/2 
				- vsci.dypBorder + (vsci.dypTmInternalLeading < 2);
		}
	else
		{
		xp = prc->xpLeft;
		yp = prc->ypTop;
		}

	if (fGray)
		{
		PatBltRc(hdc, prc, vsci.dcibScreen.ropErase);

/* Note: this is a work-around for a Windows bug.  GrayString calls
	BltColor, which sets the text and background colors to black and white
	respectively.  When that gets fixed, this code can be taken out. (LATER)
	-mattb, 8/16/88
*/
			{
			DWORD dwBkColorSav, dwTextColorSav;
			dwTextColorSav = GetTextColor(hdc);
			dwBkColorSav = GetBkColor(hdc);
			GrayString(hdc, NULL, (FARPROC)NULL, (LPSTR)rgch, cch,
					xp, yp, dxp, dyp);
			SetTextColor(hdc, dwTextColorSav);
			SetBkColor(hdc, dwBkColorSav);
			}
		}
	else
		{
		ExtTextOut(hdc, xp, yp, ETO_OPAQUE|ETO_CLIPPED, (LPRECT)prc,
				(LPSTR)rgch, cch, NULL);

		/* draw accelerators */
		if (ichAcc >= 0 && ichAcc < cch)
			{
			HBRUSH hbrOld = SelectObject(hdc, vsci.hbrText);
			PatBlt(hdc, xp+(ichAcc * vsci.dxpTmWidth), 
					yp+vsci.dypTmAscent+vsci.dypBorder, vsci.dxpTmWidth,
					vsci.dypBorder, PATCOPY);
			if (hbrOld != NULL)
				SelectObject(hdc, hbrOld);
			}
		}
}


#ifdef WIN23
PaintToggle(hwnd, hdc, pibb)
HWND hwnd;
HDC hdc;
struct IBB *pibb;
{
	if (vsci.fWin3Visuals)
		PaintToggle3(hwnd, hdc, pibb);
	else
		PaintToggle2(hwnd, hdc, pibb);
}

/************************************************************************
				Win2/3 visual layer routines
************************************************************************/
ToggleButton(hwnd, iibb, fDown)
HWND hwnd;
int iibb;
BOOL fDown;
{
	if (vsci.fWin3Visuals)
		ToggleButton3(hwnd, iibb, fDown);
	else
		ToggleButton2(hwnd, iibb, fDown);
}
IbTextOut(hdc, prc, pch, fCenter, fGray)
HDC hdc;
struct RC *prc;
CHAR *pch;
BOOL fCenter, fGray;
{
	if (vsci.fWin3Visuals)
		IbTextOut3(hdc, prc, pch, fCenter, fGray);
	else
		IbTextOut2(hdc, prc, pch, fCenter, fGray);

}
/*************************************************************************
	Routine used only for the Win 3 Visuals
*************************************************************************/
DrawShadowLines(hwnd, hdc, fAbove, fBelow)
HWND hwnd;
HDC hdc;
BOOL fAbove, fBelow;
		{	/* draw shadow lines */
		struct RC rc;
		HBRUSH hbrOld;
		GetClientRect( hwnd, &rc );
		hbrOld = SelectObject(hdc, vhbrWhite);
		if (fAbove)
			PatBlt(hdc, rc.xpLeft, rc.ypTop, rc.xpRight - rc.xpLeft,
				1, PATCOPY);
		SelectObject(hdc, vhbrGray);
		if (fBelow)
			PatBlt(hdc, rc.xpLeft, rc.ypBottom-1, rc.xpRight - rc.xpLeft,
				1, PATCOPY);
		if (hbrOld)
			SelectObject(hdc, hbrOld);
		}
ToggleButton3(hwnd, iibb, fDown)
HWND hwnd;
int iibb;
BOOL fDown;
{
	HIBS hibs = HibsFromHwndIb(hwnd);
	struct IBB *pibb = PibbFromHibsIibb(hibs, iibb);
	HDC hdc;

	Assert(pibb->ibit == ibitToggleText || pibb->ibit == ibitToggleBmp);

	if (fDown == pibb->fHilite || (hdc = GetDC(hwnd)) == NULL)
		return;

	pibb->fHilite = fDown;
	FSetDcAttribs(hdc, vsci.dccIconBar);
	SetBkColor( hdc, vdbmgDevice != dbmgEGA3 ? rgbLtGray : rgbGray);
	PaintToggle3(hwnd, hdc, pibb);
	ReleaseDC (hwnd, hdc);

}
/* P A I N T  T O G G L E */
/* %%Function:PaintToggle %%Owner:ricks */
PaintToggle3(hwnd, hdc, pibb)
HWND hwnd;
HDC hdc;
struct IBB *pibb;
{
	struct RC rc;
	struct RC rcInvert;
	int fThickShadow = pibb->fHilite;
		/* only use thick shadow when hilited */
	int fLit = (pibb->fOn && !pibb->fGray) && !fThickShadow;
		/* lit face if on and not gray and not hilited */
	int fDown = fThickShadow || fLit;	/* hilited, or on and not gray */
		/* down if hilited or lit */
	int dxp = vsci.dxpBorder;
	int dyp = vsci.dypBorder;

	if (!IsWindowVisible(hwnd))
		return;
	FreezeHp();
	GetToggleRc(&pibb->rc, &rc);

	/* do outer border */
	DrawButtonBorder(hdc, vsci.rgbBorder, &rc, fDown, pibb->fDontEraseTopLine);

	/* deflate by size of border */
	rc.xpLeft += dxp;
	rc.ypBottom -= dyp;
	rc.xpRight -= dxp;
	rc.ypTop += dyp;
	/* fill inside  with lt. grey or dithered white/grey */
	if (fLit)
		{
		LONG clrSav = SetTextColor(hdc, rgbWhite);
		HBRUSH hbrOld = SelectObject(hdc, vsci.hbrLitButton);
		PatBltRc(hdc, &rc, ROP_DPo);
		if (hbrOld)
			SelectObject(hdc, hbrOld);
		SetTextColor(hdc, clrSav);
		}
	else
		{
		LONG clrSav = SetBkColor(hdc, vsci.rgbButtonFace);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);
		SetBkColor(hdc, clrSav);
		}
	/* use DrawButtonShadow to draw shadow */
	DrawButtonShadow(hdc, &rc, fThickShadow, fDown);

	if (pibb->ibit == ibitToggleBmp)
		{
		int idrb = pibb->idrb ;
		int idcb = pibb->idcb;
		struct BMI *pbmi;
		struct MDCD *pmdcd;
		int xpSrc;

		/* do the inside */
		idrb += pibb->fGray;	/* gray set is 1 after regular set */
		pmdcd = PmdcdCacheIdrb(idrb, hdc);
		if (pmdcd != NULL)
			{
			pbmi = &mpidrbbmi[idrb];
			xpSrc = idcb * pbmi->dxpEach;
			/* review */
			BitBlt(hdc, rc.xpLeft + (3 * dxp) + fDown,
					rc.ypTop + (3 * dyp) + fDown - fLit - pibb->fAddOnePixel,
					pbmi->dxpEach, pbmi->dyp,
					pmdcd->hdc,
					xpSrc, 0,
					pibb->fGray ? SRCCOPY : SRCAND);
			}
		else
			PatBltRc(hdc, &rc, vsci.ropErase);
		}

	else if (pibb->hsz != hNil)
		{
		REC rec;
		/* ibitToggleText */

		/* deflate by size of shadow and shift for down */
		if (fDown)
			{
			rc.xpLeft += 2 * dxp;
			/* rc.ypBottom -= dyp - fDown; */
			/* rc.xpRight -= dxp - fDown; */
			rc.ypTop += 2 * dyp;
			}
		else
			{
			rc.xpLeft += dxp;
			rc.ypBottom -= 2 * dyp;
			rc.xpRight -= 2 * dxp;
			rc.ypTop += dyp + 1;
			}

		IbTextOut(hdc, &rc, *pibb->hsz, fTrue, pibb->fGray);
		}

	MeltHp();
}

DrawButtonBorder(hdc, clr, prc, fDown, fDontEraseTopLine)
HDC		hdc;
DWORD		clr;
struct 	RC *prc;
WORD fDontEraseTopLine;
	{
	struct RC	rc;
	DWORD	clrSav;
	if (fDown)
		{	/* erase old top border */
		if (fDontEraseTopLine)
			prc->ypTop += 1;	/* drop top line by one pixel */
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, (LPINT)NULL);
		if (!fDontEraseTopLine)
			prc->ypTop += 1;	/* drop top line by one pixel */
		}
	clrSav = SetBkColor(hdc, clr);

	// Bottom line.
	rc.xpLeft = prc->xpLeft + vsci.dxpBorder;
	rc.xpRight = prc->xpRight - vsci.dxpBorder;
	rc.ypTop = prc->ypBottom - vsci.dypBorder;
	rc.ypBottom = prc->ypBottom;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

	// Top line.
	rc.ypTop = prc->ypTop;
	rc.ypBottom = prc->ypTop + vsci.dypBorder;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

	// Left line.
	rc.xpLeft = prc->xpLeft;
	rc.xpRight = prc->xpLeft + vsci.dxpBorder;
	rc.ypTop = prc->ypTop + vsci.dypBorder;
	rc.ypBottom = prc->ypBottom - vsci.dypBorder;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

	// Right line.
	rc.xpLeft = prc->xpRight - vsci.dxpBorder;
	rc.xpRight = prc->xpRight;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);
	SetBkColor(hdc, clrSav);
	}
DrawButtonShadow(hdc, prc, fThickShadow, fPushed)
HDC		hdc;
struct RC *prc;
BOOL		fThickShadow;
BOOL		fPushed;
	{
	struct RC	rc;
	DWORD	clrSav;

	if (fPushed)
		{
		clrSav = SetBkColor(hdc, vsci.rgbButtonShadow);

		// Draw upper part of shadow.
		rc.xpLeft = prc->xpLeft;
		rc.xpRight = prc->xpRight;
		rc.ypTop = prc->ypTop;
		rc.ypBottom = prc->ypTop + vsci.dypBorder;
		if (fThickShadow)
			rc.ypBottom += vsci.dypBorder;

		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

		// Draw lower part of shadow.
		rc.xpRight = prc->xpLeft + vsci.dxpBorder;
		if (fThickShadow)
			rc.xpRight += vsci.dxpBorder;
		rc.ypTop += vsci.dypBorder;
		rc.ypBottom = prc->ypBottom;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);
		}
	else
		{
		/****
		in case we were down, we need to erase
		leftover dark gray on line below
		****/
		clrSav = SetBkColor(hdc, vsci.rgbButtonFace);
		rc.xpLeft = prc->xpLeft;
		rc.xpRight = prc->xpRight - vsci.dxpBorder;
		rc.ypTop = prc->ypTop + vsci.dypBorder;
		rc.ypBottom = prc->ypTop + 2 * vsci.dypBorder;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

		SetBkColor(hdc, rgbWhite);

		// Draw top highlight:
		rc.ypTop -= vsci.dypBorder;
		rc.ypBottom -= vsci.dypBorder;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);
		

		rc.ypBottom = prc->ypBottom - (vsci.dypBorder << 1);

		// Draw right highlight.
		rc.ypTop += vsci.dypBorder;
		rc.ypBottom += vsci.dypBorder;
		rc.xpRight = rc.xpLeft + vsci.dxpBorder;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

		// Draw bottom shadow.
		SetBkColor(hdc, vsci.rgbButtonShadow);

		rc.xpRight = prc->xpRight;
		rc.ypTop = prc->ypBottom - vsci.dypBorder;
		rc.ypBottom = rc.ypTop + vsci.dypBorder;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

		rc.xpLeft += vsci.dxpBorder;
		rc.ypTop -= vsci.dypBorder;
		rc.ypBottom -= vsci.dypBorder;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

		// Draw right shadow.
		rc.ypBottom = prc->ypBottom - (vsci.dypBorder << 1);
		rc.xpLeft = rc.xpRight - vsci.dxpBorder;
		rc.ypTop = prc->ypTop;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);

		rc.xpLeft -= vsci.dxpBorder;
		rc.xpRight -= vsci.dxpBorder;
		rc.ypTop += vsci.dypBorder;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, (LPINT)NULL);
		}

	SetBkColor(hdc, clrSav);
	}


/* I B  T E X T  O U T */
/* %%Function:IbTextOut3 %%Owner:ricks */
IbTextOut3(hdc, prc, pch, fCenter, fGray)
HDC hdc;
struct RC *prc;
CHAR *pch;
BOOL fCenter, fGray;
{
	int cch, ichAcc = -1;
	int xp, yp, dxp, dyp;
	CHAR *pchDest;
	CHAR rgch[256];

	/*  copy string looking for accelerators */
	for (pchDest = rgch; *pch; pch++)
		if (*pch != '&')
			*pchDest++ = *pch;
		else
			ichAcc = pchDest - rgch;

	cch = pchDest - rgch;
	Assert(cch < 256);
	*pchDest = 0;

	dxp = LOWORD(GetTextExtent(hdc, (LPSTR)rgch, cch));
	dyp = vsci.dypTmHeight;

	if (fCenter)
		{
		xp = prc->xpLeft + (prc->xpRight - prc->xpLeft - dxp)/2;
		yp = prc->ypTop + (prc->ypBottom - prc->ypTop - dyp)/2 
				- vsci.dypBorder + (vsci.dypTmInternalLeading < 2);
		}
	else
		{
		xp = prc->xpLeft;
		yp = prc->ypTop;
		}

	if (fGray)
		{

/* Note: this is a work-around for a Windows bug.  GrayString calls
	BltColor, which sets the text and background colors to black and white
	respectively.  When that gets fixed, this code can be taken out. (LATER)
	-mattb, 8/16/88
*/
			{
			DWORD dwBkColorSav, dwTextColorSav;
			dwTextColorSav = GetTextColor(hdc);
			dwBkColorSav = GetBkColor(hdc);
			GrayString(hdc, vsci.hbrButtonText, (FARPROC)NULL, (LPSTR)rgch, cch,
					xp, yp, dxp, dyp);
			SetTextColor(hdc, dwTextColorSav);
			SetBkColor(hdc, dwBkColorSav);
			}
		}
	else
		{
		short bkmode = SetBkMode(hdc, TRANSPARENT);
		DWORD  rgbSave = SetTextColor(hdc, vsci.rgbButtonText);
		ExtTextOut(hdc, xp, yp, ETO_CLIPPED, (LPRECT)prc,
				(LPSTR)rgch, cch, NULL);

		/* draw accelerators */
		if (ichAcc >= 0 && ichAcc < cch)
			{
			HBRUSH hbrOld = SelectObject(hdc, vsci.hbrButtonText);
			PatBlt(hdc, xp+LOWORD(GetTextExtent(hdc, (LPSTR)rgch, ichAcc)),
					yp+vsci.dypTmAscent+vsci.dypBorder,
					LOWORD(GetTextExtent(hdc, (LPSTR)&rgch[ichAcc], 1)),
					vsci.dypBorder, PATCOPY);
			if (hbrOld != NULL)
				SelectObject(hdc, hbrOld);
			}
		SetTextColor(hdc, rgbSave);
		SetBkMode(hdc, bkmode);
		}
}

#endif /* WIN23 */

