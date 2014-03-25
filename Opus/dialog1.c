/* D I A L O G 1 . C */
/* Code required to create a dialog */

#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "error.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "resource.h"
#include "wininfo.h"
#include "opuscmd.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "version.h"
#include "screen.h"
/*#include "el.h"*/
#include "idd.h"
#include "prompt.h"
#include "message.h"
#include "doslib.h"
#include "keys.h"
#include "rareflag.h"
#include "props.h"
#include "prm.h"
#include "core.h"
#include "dlbenum.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

extern BOOL fElActive;
extern struct PREF vpref;
extern HWND vhwndStartup;
extern HWND vhwndStatLine;
extern struct MERR  vmerr;
extern HWND vhwndApp;
extern int vcxtHelp;  /* help context */
extern BOOL vfDeactByOtherApp;
extern int vcInMessageBox;

char szOk[] = SzGlobalKey("OK", SdmOk);
char szCancel[] = SzGlobalKey("Cancel", SdmCancel);

/*  %%Function:  FSdmDoIdle  %%Owner:  bobz       */

BOOL FSdmDoIdle(fFirst)
BOOL fFirst;
{
#ifdef BATCH
	/* If vfBatchIdle is true, allow idle-ing in batch mode */
	if (vfBatchMode && !vfBatchIdle)
		BatchModeError(SzShared("Idle entered! (SDM)"), NULL, 0, 0);
#endif /* BATCH */


	/* first time returns true so sdm will keep sending idle
		messages; next time is the last time it will be called;
		returning false causes sdm to stop idle processing bz
	*/
	if (fFirst)
		{
		extern char vstPrompt [];

#ifdef RSH
		StopSubTimer(uasDialogOvrhd);
		StartSubTimer(uasDialogSession);
#endif /* RSH */

		EndLongOp(fTrue);

		if (!PcmbDlgCur()->fNoHelp && vhwndStatLine != NULL)
			DisplayHelpPrompt(fTrue);
		FAbortNewestCmg(cmgDlgNew, fTrue, fTrue);
		return fTrue;
		}

	return fFalse;
}



/* T M C  O U R  D O  D L G */
/* This is the one normaly used.  It accepts a pointer to a local dlt. */

/*  %%Function:  TmcOurDoDlg  %%Owner:  bobz       */

TMC TmcOurDoDlg(pdlt, pcmb)
DLT * pdlt;
CMB * pcmb;
{
	extern HWND vhwndCBT;
	extern TMC TmcOurDoDlg1();
	extern far PASCAL dialog1_q();

#ifdef RSH
	StartSubTimer(uasDialogOvrhd);
#endif /* RSH */

	if (!vrf.fNotFirstDlg)
		{
		vrf.fNotFirstDlg = fTrue;
		if (vrf.fExtendedMemory)
			CacheCodeSegment( GetCodeHandle( dialog1_q ), 1);
		}


	if (vhwndCBT == NULL)
		{
		/* Auto-position the dialog... */
		pdlt->bdr |= bdrAutoPosX;
		pdlt->bdr |= bdrAutoPosY;
		pdlt->rec.x = 0;
		pdlt->rec.y = 0;
		}
	else
		{
		/* CBT wants all dialogs in the upper left corner... */
		pdlt->rec.x = 8;
		pdlt->rec.y = 16;
		}

	return TmcOurDoDlg1(&pdlt, pcmb);
}


/* T M C  O U R  D O  D L G  1 */
/* This accepts a handle to a dlt which is possibly on the heap. */
/*  %%Function:  TmcOurDoDlg1  %%Owner:  bobz       */

TMC TmcOurDoDlg1(hdlt, pcmb)
DLT ** hdlt;
CMB * pcmb;
{
	extern KMP ** hkmpCur;
	extern char szEmpty [];
	extern char vstPrompt [];
	extern HWND vhwndCBT;
	extern int vrerr;

	DLI dli;
	int cxtT = vcxtHelp;
	int wLongOp;
	int cb = CbRuntimeCtm((*hdlt)->ctmBase);

#ifdef RSH
	StartSubTimer(uasDialogOvrhd);
#endif /* RSH */

	SetBytes(&dli, 0, sizeof (DLI));
	dli.fdlg = fdlgModal | fdlgFedt;
	dli.wRef = (WORD) pcmb;

	/* this returns from this func if there is not enough stack for AllocA below */
	ReturnOnNoStack(cb, tmcError, fTrue);

	/* we now alloc one of these for each dialog */
	dli.rgb = OurAllocA(cb);

	FAbortNewestCmg(cmgCoreSdmAll, fFalse, fFalse);

	if (vfDeactByOtherApp)
		{
		WaitActivation();
		}

	if ((!vmerr.fSdmInit && !FAssureSdmInit()) ||
			HkmpNew(0, kmfStopHere + kmfAppModal) == hNil)
		{
LRetErr:
		pcmb->tmc = tmcError;
#ifdef RSH
		StopSubTimer(uasDialogOvrhd);
#endif /* RSH */
		return tmcError;
		}

	if (vhwndStartup != NULL)
		EndStartup();

	/* close any files on floppies so we are not hosed if the user
	   changes/removes a disk */
	CloseEveryFn(fFalse);

	vcxtHelp = CxtFromHid((*hdlt)->hid);

	GetLongOpState(&wLongOp);
	StartLongOp();

	/* Set the SAB to 0 so no sub-dialogs will appear... */
	*((WORD *) (*pcmb->hcab) + cwCabMin - 1) = 0;

		/* TmcDoDlgDli not safe if dialog coming down */
	if (FIsDlgDying())
		{
		RemoveKmp(hkmpCur);
		goto LRetErr;  /* not safe to call TmcDoDlgDli at oom */
		}

	pcmb->tmc = TmcDoDlgDli(hdlt, pcmb->hcab, &dli);

#ifdef RSH
	StopSubTimer(uasDialogSession);
	StartSubTimer(uasDialogOvrhd);
#endif /* RSH */

	  /* returning from a dialog when a message box is up will reenable Opus
		 we don't want this. (a lot of these only happen at
		 WM_QUERYABORTSESSION when a dialog or mb is up and we click on the
		 dos exec. So if we are still in an mb, redisable Opus  bz  9/19/89
	  */
	if (vcInMessageBox)
		EnableWindow(vhwndApp, fFalse);

	/* Assert that SetErrorMat has already been done */
	Assert(pcmb->tmc != tmcError || vmerr.fMemFail || vmerr.fDiskFail ||
			(vrerr != 0 && fElActive));

	ResetLongOpState(wLongOp);

	vcxtHelp = cxtT; /* restore nested context */


	RemoveKmp(hkmpCur);

	if (!pcmb->fNoHelp && vhwndStatLine != NULL)
		DisplayHelpPrompt(fFalse);

	/* clear the key state, in case we are in abortable process */
	GetAsyncKeyState(VK_ESCAPE);
	GetAsyncKeyState(VK_CANCEL);

	OldestCmg(cmgDlgOld);

#ifdef RSH
		StopSubTimer(uasDialogOvrhd);
#endif /* RSH */

	return pcmb->tmc;
}


/* F  F I L T E R  S D M  M S G */
/* General Windows message filter for modal dialogs.  Returning fTrue
causes SDM to ignore the message. */
/*  %%Function:  FFilterSdmMsg  %%Owner:  bobz       */

BOOL FFilterSdmMsg(lpmsg)
LPMSG lpmsg;

{
	if (vmerr.fMemFail)
		{

#ifdef BZ
		if (vmerr.fMemFail)
			ReportSz("Warning - dialog taken down in FFilterSdmMsg (oom)");
		else
			ReportSz("Warning - dialog taken down in FFilterSdmMsg (disk)");
#endif /* BZ */

		EndDlg(tmcError);  /* ok to call EndDlg here */

		return fTrue;
		}

	  /* note that SetKeyState is called below, so even if in win3 the call
		 in FCheckT... does nothing if the msg is not removed, it will
		 be set below when needed. bz 10/18/89
	  */
	if (FCheckToggleKeyMessage(lpmsg))
		return fFalse;

	if (lpmsg->message == WM_KEYDOWN || lpmsg->message == WM_SYSKEYDOWN)
		{
		int kc;

		kc = lpmsg->wParam;

		SetOurKeyState();
		if (vfShiftKey)
			kc = KcShift(kc);
		if (vfControlKey)
			kc = KcCtrl(kc);
		if (vfOptionKey)
			kc = KcAlt(kc);

#ifdef DEBUG
		if (FProcessDbgKey(kc))
			return fTrue;
#endif /* DEBUG */

		if (kc == KcShift(kcF1))
			{
			GetHelp(vcxtHelp);
			return fTrue;
			}
		else  if (kc == kcF1)
			{
			GetHelp(vcxtHelp);
			return fTrue;
			}

		/* This is how the change keys dialog does it's thing. */
		if (vrf.fChangeKeys && FChangeKeysHook(kc))
			return fTrue;

		return FExecKc(kc);
		}

	return fFalse;
}


/*  %%Function:  GrayButtonOnBlank  %%Owner:  bobz       */

VOID GrayButtonOnBlank(tmcEdit, tmcButton)
TMC tmcEdit, tmcButton;
{
	char sz [ichMaxBufDlg];

	GetTmcText(tmcEdit, sz, ichMaxBufDlg);
	GrayBtnOnSzBlank(sz, tmcButton);
}


/*  %%Function:  GrayBtnOnSzBlank  %%Owner:  bobz       */

GrayBtnOnSzBlank(sz, tmcButton)
char *sz;
TMC  tmcButton;
{
	BOOL fEnable = !(CchStripString(sz, CchSz(sz) - 1) == 0);
	EnableTmc(tmcButton, fEnable);
	if (tmcButton == tmcOK && fEnable)
		SetDefaultTmc(tmcOK);
}

#ifdef NOTUSED
/*  %%Function:  PcmbInitCmb  %%Owner:  bobz       */

CMB * PcmbInitCmb(pcmb, kc, bcm, cmm)
CMB * pcmb;
int kc;
BCM bcm;
int cmm;
{
	Assert(pcmb != NULL);
	pcmb->kc = kc;
	pcmb->bcm = bcm;
	pcmb->cmm = cmm;
	pcmb->pv = 0;
	pcmb->hcab = hNil;

	return pcmb;
}


#endif /* NOTUSED */


/*  %%Function:  WListBoxNull  %%Owner:  bobz       */

WORD WListBoxNull(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	return 0;
}


/* General purpose list box fill function. */
/*  %%Function:  WListEntbl  %%Owner:  bobz       */

EXPORT WORD WListEntbl(tmm, sz, isz, filler, tmc, iEntbl)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD iEntbl; /* wParam */
{
	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		return FEnumIEntbl(iEntbl, isz, sz);
		}

	return 0;
}



/*  P R O P  T O  P R B  G R A Y */
/* NEW SDM  */
/* writes properties specified by mpiagsprm into prb to be put into cab later */
/*  %%Function:  PropToPrbGray  %%Owner:  bobz       */

PropToPrbGray(prb, prgbProp, prgbGray, mpiagspnt)
WORD * prb;
char * prgbProp;
char * prgbGray;
SPNT mpiagspnt [];
{
	int sprm;
	int iag = 0;
	int iagMac = *prb;    /* 1st entry in prb is number of entries */

	for ( ; iag < iagMac; ++iag)
		{
		if ((sprm = mpiagspnt[iag].sprm) != sprmNoop)
			{
			if (prgbGray != NULL &&
					ValFromPropSprm(prgbGray, sprm) != 0)
				{
				*(prb + iag + 1) =
						(mpiagspnt[iag].wNinchVal == 0) ? wNinch :
						mpiagspnt[iag].wNinchVal;

				}
			else
				{
				WORD val;

				val = ValFromPropSprm(prgbProp, sprm);

				/* Translate val if necessary... */
				switch (sprm)
					{
				case sprmPStc:
				case sprmCKul:
					continue;
					}

				/* +1 because 1st entry is count */
				*(prb + iag + 1) =  val;
#ifdef BZ
				CommSzNum(SzShared("Proptocabgray iag: "), iag);
				CommSzNum(SzShared("val: "), 
						*(prb + iag + 1));
#endif
				}
			}
		}
}


/*  %%Function:  PrbToCab  %%Owner:  bobz       */

PrbToCab(prb, hcab, iagStart, ciag)
WORD *prb;
HCAB hcab;
int iagStart;  /* starting iag for blt */
int ciag;      /* iags to copy */
{
	/* copy entries from prb into cab.  Assumes in sync. Assert in caller */

	blt ( (prb + 1 + iagStart),	((WORD *) (*hcab)) + cwCabMin + iagStart,
			ciag );
}




/* From FEDT.C */

#include "fedt.h"

extern struct SCI   vsci;
extern struct MERR  vmerr;
extern int vgrpfKeyBoardState;

/* Macros to keep the rest of this module as Excel-like as reasonable */

#define fAlt        (wKbsOptionMask)
#define fShift      (wKbsShiftMask)
#define fControl    (wKbsControlMask)
#define FOREVER     for ( ;; )
#define chLF        (chEol)
#define chBackSpace (chBS)
#define chNull      '\0'
#define SzLen(sz)   (CchSz(sz)-1)
#ifdef DEBUG
int wErrFedt;
#endif

extern HCURSOR      vhcIBeam;
extern int ichMinRef;

long FedtNotifyParent();
char *PchLastFit();
#ifdef WIN23
char *PchLastFit3();
#endif /* WIN23 */
char *PchPrevTextBrk();

#define pfedt (*hfedt)

/*----------------------------------------------------------------------------
|    FedtWndProc
|
|    Formula edit window procedure.  Used for formula bar and modeless
|    dialog edit controls.
|
|    Arguments:
|        hwnd        window receiving the message
|        wm        window message
|        wParam, lParam    additional info
|
|    Returns:
|        depends on the message.
----------------------------------------------------------------------------*/

/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */
/*  %%Function:  FedtWndProc  %%Owner:  bryanl       */

long EXPORT FAR PASCAL FedtWndProc(hwnd, wm, wParam, lParam)
HWND hwnd;
int wm;
int wParam;
long lParam;
{
	FEDT **hfedt;
	int ich, cch;
	BOOL fShowSav;
	char *pch;
	struct RC rc;
	PAINTSTRUCT paint;
	int ch;
	HDC hdcSav;

	hfedt = HfedtFromHwnd(hwnd);

	switch (wm)
		{

	case WM_NCCREATE:
		return((long)FFedtInit(hwnd));

	case WM_DESTROY:
		FedtFree(hfedt);
		goto LRetDefProc0;

	case WM_SIZE:
		GetClientRect(hwnd, (LPRECT)&rc);
#ifdef WIN23
		if (pfedt->fSingleLine)
			{
			int dyBorder = 0;
			int dyRect =  rc.ypBottom - rc.ypTop;
			if (dyRect > vsci.dypTmHeight)
				dyBorder = (dyRect - vsci.dypTmHeight) / 2;
			InflateRect((LPRECT)&rc, -(vsci.dxpTmWidth/2-1), -dyBorder);
			}
		else
			InflateRect((LPRECT)&rc, -(vsci.dxpTmWidth/2-1), -(vsci.dypTmHeight/4-1));
#else
		InflateRect((LPRECT)&rc, -(vsci.dxpTmWidth/2-1), -(vsci.dypTmHeight/4-1));
#endif /* WIN23 */
		rc.xpRight = max(rc.xpRight, rc.xpLeft+vsci.dxpTmWidth);
		rc.ypBottom = max(rc.ypBottom, rc.ypTop+vsci.dypTmHeight);
		pfedt->rcView = rc;
		pfedt->rcFmt.ptBottomRight = rc.ptBottomRight;
		break;

	case WM_SETFOCUS:
		pfedt->fOverType = FALSE;
		FedtNotifyParent(&hfedt, FN_SETFOCUS);
		Assert( hfedt != NULL && IsWindow( (*hfedt)->hwnd ) );
		/* make sure focus is still in this window */
		if (GetFocus() != hwnd)
			break;
		if (!pfedt->fCaretOn)
			{
			CreateCaret(hwnd, NULL, 1, vsci.dypTmHeight);
			pfedt->fCaretOn = TRUE;
			}
/* Bug fix (#3430): changed from "2" to "true" so we show caret when we have 
	the focus and show entire text when we don't */
		FedtShowSel(hfedt, fTrue);
		goto LRet0;

	case WM_KILLFOCUS:
		FedtNotifyParent(&hfedt, FN_KILLFOCUS);
		Assert( hfedt != NULL && IsWindow( (*hfedt)->hwnd ) );
		FedtShowSel(hfedt, FALSE);
		if (pfedt->fCaretOn)
			{
			pfedt->fCaretOn = FALSE;
			DestroyCaret();
			}
/* Bug fix (BL, bug 3741): if leaving control, restore scroll to (0,0) */
		if (FNeRgch( &pfedt->rcFmt.ptTopLeft, &pfedt->rcView.ptTopLeft, sizeof (struct PT)))
			{
			pfedt->rcFmt.ptTopLeft = pfedt->rcView.ptTopLeft;
			FedtRedraw(hfedt, 0, pfedt->cchText);
			}
		goto LRet0;

	case WM_KEYDOWN:
	case WM_CHAR:
		FFedtKeybd(hfedt, wm, wParam);
		goto LRet0;

	case WM_KEYUP:
		break;

	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
		FedtTrackSel(hfedt, lParam, wParam, (int)(wm-WM_LBUTTONDOWN) % 3 == 2);
LRet0:	
		FedtReleaseDC(hfedt);
		break;

	case WM_SETREDRAW:
		if (pfedt->fCaretOn && pfedt->fShowSel)
			{
			if (wParam)
				ShowCaret(hwnd);
			else
				HideCaret(hwnd);
			}
		goto LRetDefProc;

	case WM_ERASEBKGND:
		HideCaret(hwnd);
		hdcSav = pfedt->hdc;
		FedtInitDC(hfedt, wParam);
		GetClipBox((HDC)wParam, (LPRECT)&rc);
		PatBltRc( (HDC)wParam, &rc, PATCOPY );
		ShowCaret(hwnd);
		pfedt->hdc = hdcSav;
		FedtReleaseDC(hfedt);
		return(1L);

	case WM_PAINT:
		HideCaret(hwnd);
		BeginPaint(hwnd, (LPPAINTSTRUCT)&paint);
		if (IsWindowVisible(hwnd))
			{
			hdcSav = pfedt->hdc;
			FedtInitDC( hfedt, paint.hdc );
			GetClientRect(hwnd, (LPRECT)&rc);
			/* white out area outside the view rectangle */
			if (!paint.fErase)
				{
				PatBlt(paint.hdc, 0, 0,
						rc.xpRight, pfedt->rcView.ypTop,
						PATCOPY);
				PatBlt(paint.hdc, 0, 0,
						pfedt->rcView.xpLeft, rc.ypBottom,
						PATCOPY);
				PatBlt(paint.hdc, 0, pfedt->rcView.ypBottom,
						rc.xpRight, rc.ypBottom - pfedt->rcView.ypBottom,
						PATCOPY);
				PatBlt(paint.hdc, pfedt->rcView.xpRight, 0,
						rc.xpRight - pfedt->rcView.xpRight, rc.ypBottom,
						PATCOPY);
				}
			/* redraw the text in the format area */
			FedtRedraw(hfedt, 0, pfedt->cchText);
			if (pfedt->fShowSel && pfedt->ichMicSel != pfedt->ichMacSel)
				FedtInvertRgn(hfedt, pfedt->hrgnSel);
			}
		EndPaint(hwnd, (LPPAINTSTRUCT)&paint);
		if (IsWindowVisible(hwnd))
			{
			ShowCaret(hwnd);
			pfedt->hdc = hdcSav;
			}
		goto LRet0;

	case WM_GETTEXTLENGTH:
		return(pfedt->cchText);

	case WM_GETTEXT:
		ich = 0;
		wParam -= 1;
		if (wParam > 0)
			{
			ich = min(pfedt->cchText, wParam);
			pch = PchFromHsz(pfedt->hszText);
			bltbx((LPSTR)pch, (LPSTR)lParam, ich);
			}
		((LPSTR)lParam)[ich] = 0;
		return((long)ich);

	case WM_SETTEXT:
			{
			CHAR szBuf[cchMaxSz+1];

		/* allocate new buffer with new text */
			cch = min(cchMaxSz, CchLpszLen(lParam));
			/* cch does not include null terminator */
			/* temp buffer because lParam is a heap pointer */
			bltbx((LPSTR)lParam, szBuf, cch);
			szBuf [cch++] = '\0';
			/* cch now includes null terminator */
			if ((wParam = HAllocateCw(CwFromCch(cch))) == hNil)
				{
				FedtNotifyParent(&hfedt, EN_ERRSPACE);
				
				/* REVIEW bryanl (sah):Win 3.0 ref manual mentioned */
				/* LB_ERRSPACE or CB_ERRSPACE as appropriate return */
				/* value if fail to set string due to memory fail.  */
				
				return 0L;
				}

			bltbyte(szBuf, *(char **)wParam, cch );
			}
		/* fall through */
	case EM_SETHANDLE:
		fShowSav = pfedt->fShowSel;
		FedtShowSel(hfedt, FALSE);
		FedtSetSel(hfedt, 0, 0);
		FreeH( pfedt->hszText );
		pfedt->hszText = (HANDLE)wParam;
		pfedt->cchText = SzLen(PchFromHsz((char **)wParam));
		pfedt->rcFmt.ptTopLeft = pfedt->rcView.ptTopLeft;
		FFedtReformat(&hfedt, 0, 0x7fff);
		if (hfedt != NULL)
			{
			FedtSetSel(hfedt, 0, 0x7fff);
			FedtShowSel(hfedt, fShowSav);
			FedtNotifyChange(hfedt);
			FedtReleaseDC(hfedt);
			}
		return(1L);

	case WM_GETDLGCODE:
		return((long)(DLGC_WANTARROWS|DLGC_HASSETSEL|DLGC_WANTCHARS));

	case WM_COPY:
		if (!FFedtCopy(&hfedt))
			break;
		goto SuccessCCP;

	case WM_CUT:
		if (!FFedtCopy(&hfedt) || !FFedtClear(&hfedt))
			break;
		goto SuccessCCP;

	case WM_CLEAR:
		if (!FFedtClear(&hfedt))
			break;
		goto SuccessCCP;

	case WM_PASTE:
		if (!FFedtPaste(&hfedt))
			break;
SuccessCCP:
		FedtReleaseDC(hfedt);
		return(1L);

	case EM_SETSEL:
		FedtSetSel(hfedt, LOWORD(lParam), HIWORD(lParam));
		FedtReleaseDC(hfedt);
		break;

	case EM_GETSEL:
		return(MakeLong(pfedt->ichMicSel, pfedt->ichMacSel));

	case EM_GETLINECOUNT:
		return(pfedt->liMac);

	case EM_REPLACESEL:
			{
			long l;
			l = ((long)FFedtReplSel(&hfedt, lParam, CchLpszLen(lParam)));
			FedtReleaseDC(hfedt);
			return l;
			}

	case EM_GETHANDLE:
		return((long)pfedt->hszText);

#ifdef DEBUG
	default:
		Assert(fFalse);
#endif /* DEBUG */
		}

	Assert( !pfedt->hdc );
	return 0L;

LRetDefProc:
	Assert( !pfedt->hdc );
LRetDefProc0:
	return(DefWindowProc(hwnd, wm, wParam, lParam));
}


/*----------------------------------------------------------------------------
|    FedtNotifyParent
|
|    Sends a notification code to a fedt control's parent.
|
|    Arguments:
|        phf/edt        pointer to fedt pointer - MAY CHANGE ON
|                RETURN!!
|        fn        notification code
----------------------------------------------------------------------------*/
/*  %%Function:  FedtNotifyParent  %%Owner:  bryanl       */

long FedtNotifyParent(phfedt, fn)
FEDT ***phfedt;
int fn;
{
	HWND hwnd, hwndParent;
	long lRet = 0L;

	if ((hwnd = (**phfedt)->hwnd) == NULL)
		goto LRetErr;
	if ((hwndParent = GetParent(hwnd)) == NULL)
		goto LRetErr;

	lRet = SendMessage(hwndParent, WM_COMMAND,
			GetWindowWord(hwnd, GWW_ID), MakeLong(hwnd, fn));
/* Set handle to NULL if notification caused window to be destroyed */
	if (IsWindow( hwnd ))
		*phfedt = HfedtFromHwnd(hwnd);
	else
LRetErr:
		*phfedt = NULL;

	return(lRet);
}


/*----------------------------------------------------------------------------
|    FFedtInit
|
|    Creates a new fedt control.
|
|    Arguments:
|        hwnd        window handle of the control
|
|    Returns:
|        TRUE if successful
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtInit  %%Owner:  bryanl       */

BOOL FFedtInit(hwnd)
HWND hwnd;
{
	long ws;
	FEDT **hfedt;
	int dxT, dyT;
	FEDT *pfedtT;

	ws = GetWindowLong(hwnd, GWL_STYLE);
	if ((hfedt = HAllocateCw(CwFromCch(CbFedt(1)))) == hNil)
		return FALSE;
	pfedtT = *hfedt;
	SetBytes( pfedtT, 0, sizeof(FEDT));
	SetWindowWord(hwnd, 0, hfedt);
	pfedtT->liMax = 1;
	pfedtT->hwnd = hwnd;
	pfedtT->hwndParent = GetWindowWord(hwnd,GWW_HWNDPARENT);
	Assert( pfedtT->hwndParent != NULL );
	pfedtT->liMac = 1;
	pfedtT->fSingleLine = (ws & FS_MULTILINE) == 0;
	/* initialize format rectangle to something, anything */
	dxT = vsci.dxpTmWidth/2-1;
	dyT = vsci.dypTmHeight/4-1;
	PrcSet( &pfedtT->rcFmt, dxT,
			dyT,
			dxT + vsci.dxpTmWidth,
			dyT + vsci.dypTmHeight);
	if ((pfedt->hszText = HAllocateCw(8)) == hNil ||
	    (pfedt->hrgnSel = CreateRectRgn(0, 0, 0, 0)) == NULL)
		{
		FedtFree(hfedt);
		return(FALSE);
		}

	LogGdiHandle(pfedt->hrgnSel, 15000);
	(*pfedt->hszText) [0] = '\0';

	return(TRUE);
}


/*----------------------------------------------------------------------------
|    FedtInitDC
|
|    Sets up the fedt control's DC based on system metrics.
|
|    Arguments:
|        hfedt
----------------------------------------------------------------------------*/
/*  %%Function:  FedtInitDC  %%Owner:  bryanl       */

FedtInitDC(hfedt, hdc)
FEDT **hfedt;
HDC hdc;
{
	HBRUSH hbr;
	HWND hwndParent;

	pfedt->hdc = hdc;
	if ((hwndParent = GetParent(pfedt->hwnd)) != NULL &&
			((hbr = (HBRUSH)SendMessage( hwndParent, WM_CTLCOLOR,
			(WORD)hdc, MAKELONG(pfedt->hwnd, CTLCOLOR_EDIT))) != NULL))
		SelectObject(hdc, hbr);
	SetBkColor(hdc, vsci.rgbBkgrnd );
	SetTextColor(hdc, vsci.rgbText );
}



/*----------------------------------------------------------------------------
|    FedtFree
|
|    Frees up a fedt control.
|
|    Arguments:
|        hfedt        fedt to free up
----------------------------------------------------------------------------*/
/*  %%Function:  FedtFree  %%Owner:  bobz       */

FedtFree(hfedt)
FEDT **hfedt;
{
	if (pfedt->hrgnSel != NULL)
		{
		UnlogGdiHandle(pfedt->hrgnSel, -1);
		DeleteObject(pfedt->hrgnSel);
		}
	if (pfedt->hszText != NULL)
		FreeH(pfedt->hszText);
	FreeH(hfedt);
}

/*----------------------------------------------------------------------------
|    FedtShowSel
|
|    Displays/hides the selection in the given fedt control.
|
|    Arguments:
|        hfedt        fedt control to change the selection in
|        fShow        TRUE to show, FALSE to hide selection, 2
|                to show selection but not scroll the caret
|                into view.
----------------------------------------------------------------------------*/
/*  %%Function:  FedtShowSel  %%Owner:  bryanl       */

FedtShowSel(hfedt, fShow)
FEDT **hfedt;
BOOL fShow;
{
	char *pchLeft, *pchText;
	int xCaret, xpLeft;
	struct RC rc;
	BOOL fShowSel;
	HWND hwnd;

	Debug(CheckFedt(hfedt));
	fShowSel = (fShow != 0);
	if (fShowSel == pfedt->fShowSel)
		return;
	pfedt->fShowSel = fShowSel;
	hwnd = pfedt->hwnd;
	if (fShowSel)
		{
		FedtComputeRgnSel(hfedt);
		if (fShow != 2)
			FFedtMakeVis(hfedt, FALSE);
		FedtInvertRgn(hfedt, pfedt->hrgnSel);
		if (pfedt->fCaretOn)
			{
			SetCaretPos(pfedt->ptCaret.xp, pfedt->ptCaret.yp);
			ShowCaret(hwnd);
			}
		}
	else
		{
		if (pfedt->fCaretOn)
			HideCaret(hwnd);
		FedtInvertRgn(hfedt, pfedt->hrgnSel);
		}
	Debug(CheckFedt(hfedt));
}


/*----------------------------------------------------------------------------
|    FedtSetSel
|
|    Sets the selection in an fedt control.
|
|    Arguments:
|        hfedt        fedt control to set selection in
|        ichMic        new beginning of selection
|        ichMac        new end of selection
----------------------------------------------------------------------------*/
/*  %%Function:  FedtSetSel  %%Owner:  bryanl       */

FedtSetSel(hfedt, ichMic, ichMac)
FEDT **hfedt;
int ichMic, ichMac;
{
	FedtSetSel2(hfedt, ichMic, ichMac, 0x7fff, -2, FALSE);
}


/*----------------------------------------------------------------------------
|    FedtSetSel2
|
|    Sets the selection for the given fedt control.  Helps take care
|    of the beginning of line/end of line selection ambiguity by taking
|    optional line numbers.
|
|    Arguments:
|        hfedt        fedt control to set selection in
|        ichMic, ichMac    new selection range
|        ichCaret    position of new caret
|                0x7fff for don't care
|        liCaret        line number of caret,
|                -2 for don't care
|        fMinScroll    do minimum scrolling to display caret after
|                selection change
----------------------------------------------------------------------------*/
/*  %%Function:  FedtSetSel2  %%Owner:  bryanl       */

FedtSetSel2(hfedt, ichMic, ichMac, ichCaret, liCaret, fMinScroll)
FEDT **hfedt;
int ichMic, ichMac;
int ichCaret;
int liCaret;
BOOL fMinScroll;
{
	char *pchText;
	BOOL fMacCaret;
	int li;

	Debug(CheckFedt(hfedt));

/* find position of selection */

	if (ichMic > ichMac)
		SwapPw2(&ichMic, &ichMac);
	ichMic = PegRange(ichMic, 0, pfedt->cchText);
	ichMac = PegRange(ichMac, 0, pfedt->cchText);
	/* ichCaret == 0x7fff maps to ichMacSel!! */
	ichCaret = PegRange(ichCaret, ichMic, ichMac);
	fMacCaret = (ichCaret == ichMac);

/* get line of caret - if caller supplied a reasonable line hint, use 
	it to decide which line to use if it's at a line break */

	li = LiFedtFromIch(hfedt, ichCaret);
	pchText = PchFromHsz(pfedt->hszText);
	if (!(liCaret == li-1 && ichCaret == pfedt->mpliichBrk[liCaret] && 
			(ichCaret == 0 || pchText[ichCaret-1] != chLF )))
		liCaret = li;

/* set selection in the fedt structure */

	pfedt->ichMicSel = ichMic;
	pfedt->ichMacSel = ichMac;
	pfedt->liCaret = liCaret;
	pfedt->fMacCaret = fMacCaret;

	FedtUpdateSel(hfedt, fMinScroll);

	Debug(CheckFedt(hfedt));

}


/*----------------------------------------------------------------------------
|    FedtUpdateSel
|
|    Redisplays the new selection.  Takes advantage of the existing
|    inverted area to minimize the screen area to modify.
|
|    Arguments:
|        hfedt        fedt control whose selection changed.
|        fMinScroll    do minimum scrolling to display caret
|                after the selection change
|
----------------------------------------------------------------------------*/
/*  %%Function:  FedtUpdateSel  %%Owner:  bryanl       */

FedtUpdateSel(hfedt, fMinScroll)
FEDT **hfedt;
BOOL fMinScroll;
{
	int rt;
	HRGN hrgnSwap;
	struct PT pt;
	struct RC far *lprc;
	HRGN hrgn;

	Debug(CheckFedt(hfedt));
	if (!pfedt->fShowSel)
		return;
	if ((hrgn = CreateRectRgn(0,0,0,0)) == NULL)
		return;
	LogGdiHandle(hrgn, 25000);
	/* save old selection in hrgn/pt */
	hrgnSwap = hrgn;
	hrgn = pfedt->hrgnSel;
	pfedt->hrgnSel = hrgnSwap;
	pt = pfedt->ptCaret;
	FedtComputeRgnSel(hfedt);
	if (FFedtMakeVis(hfedt, fMinScroll))
		SetRectRgn(hrgn, 0, 0, 0, 0);
	rt = CombineRgn(hrgn, pfedt->hrgnSel, hrgn, RGN_XOR);
	if (rt >= SIMPLEREGION)
		{
		HideCaret(pfedt->hwnd);
		FedtInvertRgn(hfedt, hrgn);
		}
	if (*(long **)&pfedt->ptCaret != *(long *)&pt)
		SetCaretPos(pfedt->ptCaret.xp, pfedt->ptCaret.yp);
	if (rt >= SIMPLEREGION)
		ShowCaret(pfedt->hwnd);
	UnlogGdiHandle(hrgn, -1);
	DeleteObject( hrgn );
}


/*----------------------------------------------------------------------------
|    FedtComputeRgnSel
|
|    Recalculates the selection for the given fedt control.  This routin
|    must be called before redisplaying the selection after a changing
|    the selection range.
|
|    Arguments:
|        hfedt        fedt control to compute selection for
|
----------------------------------------------------------------------------*/
/*  %%Function:  FedtComputeRgnSel  %%Owner:  bryanl       */

FedtComputeRgnSel(hfedt)
FEDT **hfedt;
{
	int li, liFirst, liLast;
	int ichMicSel, ichMacSel;
	HRGN hrgnSel;
	HRGN hrgnT;
	struct RC rc;

	Debug(CheckFedt(hfedt));

/* calculate Mic end-point of the selection */

	if ((hrgnT = CreateRectRgn(0,0,0,0)) == NULL)
		return;
	LogGdiHandle(hrgnT, 1004);
	liFirst = liLast = pfedt->liCaret;
	ichMicSel = pfedt->ichMicSel;
	ichMacSel = pfedt->ichMacSel;
	hrgnSel = pfedt->hrgnSel;
	if (ichMicSel != ichMacSel)
		{
		if (pfedt->fMacCaret)
			liFirst = LiFedtFromIch(hfedt, ichMicSel);
		else
			liLast = LiFedtFromIch(hfedt, ichMacSel);
		}
	rc.ypTop = pfedt->rcFmt.ypTop + liFirst * vsci.dypTmHeight;
	rc.xpLeft = DxFedtPos(hfedt, liFirst, ichMicSel);
	/* always set caret position to Mic end-point - if it's supposed to be
		at the Mac end-point, we'll patch it later */
	pfedt->ptCaret = rc.ptTopLeft;
	rc.xpLeft = max(pfedt->rcFmt.xpLeft, rc.xpLeft);

/* compute inverted selection region if it exists */

	if (ichMicSel == ichMacSel)
		SetRectRgn(hrgnSel, 0, 0, 0, 0);
	else
		{
		for (li = liFirst; ; )
			{
			rc.ypBottom = rc.ypTop + vsci.dypTmHeight;
			if (ichMacSel < pfedt->mpliichBrk[li])
				{
				rc.xpRight = DxFedtPos(hfedt, li, ichMacSel);
				if (!pfedt->fSingleLine)
					rc.xpRight = min(rc.xpRight, pfedt->rcFmt.xpRight);
				}
			else
				{
				rc.xpRight = DxFedtPos(hfedt, li, pfedt->mpliichBrk[li]);
				rc.xpRight = max(rc.xpRight, pfedt->rcView.xpRight);
				}
			if (li == liFirst)
				SetRectRgn(hrgnSel, rc.xpLeft, rc.ypTop, rc.xpRight, rc.ypBottom);
			else
				{
				SetRectRgn(hrgnT, rc.xpLeft, rc.ypTop, rc.xpRight, rc.ypBottom);
				CombineRgn(hrgnSel, hrgnSel, hrgnT, RGN_OR);
				}
			if (li++ >= liLast)
				{
				if (pfedt->fMacCaret)
					{
					/* set caret to Mac end-point */
					pfedt->ptCaret.yp = rc.ypTop;
					pfedt->ptCaret.xp = rc.xpRight;
					}
				break;
				}
			rc.xpLeft = pfedt->rcFmt.xpLeft;
			rc.ypTop = rc.ypBottom;
			}
		}
	UnlogGdiHandle(hrgnT, 1004);
	DeleteObject( hrgnT );
}


/*----------------------------------------------------------------------------
|    FFedtMakeVis
|
|    Makes the caret in the given fedt control visible in the view
|    rectangle.
|
|    Arguments:
|        hfedt        fedt control whose caret needs showing
|        fMinScroll    use minimal scrolling to make caret visible
|
|    Returns:
|        TRUE if the fedt control text was redrawn - the selection
|        will be recomputed but will NOT be redrawn by this routine.
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtMakeVis  %%Owner:  bryanl       */

BOOL FFedtMakeVis(hfedt, fMinScroll)
FEDT **hfedt;
BOOL fMinScroll;
{
	int dx, dy;
	struct PT ptCaret;
	struct RC rcView;

	Debug(CheckFedt(hfedt));

	rcView = pfedt->rcView;
	ptCaret = pfedt->ptCaret;
	dx = (rcView.xpLeft + 3*rcView.xpRight) / 4;
	if (ptCaret.xp <= rcView.xpLeft)
		{
		if (fMinScroll)
			dx = rcView.xpLeft;
		dx = min(dx - ptCaret.xp, rcView.xpLeft - pfedt->rcFmt.xpLeft);
		}
	else  if (ptCaret.xp > rcView.xpRight)
		{
		if (fMinScroll)
			dx = rcView.xpRight;
		dx -= ptCaret.xp;
		}
	else
		dx = 0;
	if ((dy = rcView.ypBottom - (ptCaret.yp+vsci.dypTmHeight)) > 0)
		if ((dy = rcView.ypTop - ptCaret.yp) < 0)
			dy = 0;
	if (dx == 0 && dy == 0)
		return(FALSE);

	FedtScroll(hfedt, dx, dy);
	return(TRUE);
}


/*----------------------------------------------------------------------------
|    FedtRedraw
|
|    Redraws a sub-string of a fedt control's formatted text.
|
|    Arguments:
|        hfedt        fedt control to redraw
|        ichMic, ichMac    range of characters to redraw
|
|    WARNING!! - assumes the caret is off on entry.
----------------------------------------------------------------------------*/
/*  %%Function:  FedtRedraw  %%Owner:  bryanl       */

FedtRedraw(hfedt, ichMic, ichMac)
FEDT **hfedt;
int ichMic, ichMac;
{
	int li, liLast;
	int ichMacLine, cch;
	char *pch, *pchText;
	struct RC rc, rcT;

	Debug(CheckFedt(hfedt));
	if (!IsWindowVisible(pfedt->hwnd) || !FFedtValidateDC(hfedt))
		return;

	pchText = PchFromHsz(pfedt->hszText);
	li = LiFedtFromIch(hfedt, ichMic);
	if (li > 0 && pfedt->mpliichBrk[li-1] == ichMic)
		li--;
/* set blank/clip rect to include up to right of view rect; code below
	paints all the way to the end of lines anyway, and this will perform
	blanking of the area from rcFmt.xpRight to rcView.xpRight in the horz scroll case. */
	PrcSet( &rc, DxFedtPos(hfedt, li, ichMic), 
			pfedt->rcFmt.ypTop + li * vsci.dypTmHeight,
			pfedt->rcView.xpRight,
			0 );
	liLast = LiFedtFromIch(hfedt, ichMac);
	FOREVER
				{
		rc.ypBottom = rc.ypTop + vsci.dypTmHeight;
		ichMacLine = pfedt->mpliichBrk[li];
		cch = ichMacLine - ichMic;
		pch = pchText + ichMic;
		if (cch > 0 && pch[cch-1] == chLF)
			cch--;
		if (FSectRc( &pfedt->rcView, &rc, &rcT ))
			ExtTextOut(pfedt->hdc, rc.xpLeft, rc.ypTop,
					ETO_OPAQUE | ETO_CLIPPED,
					(LPRECT)&rcT, (LPSTR)pch, cch, 0L);
		if (++li > liLast)
			break;
		rc.ypTop = rc.ypBottom;
		rc.xpLeft = pfedt->rcFmt.xpLeft;
		ichMic = ichMacLine;
		}
	if (li >= pfedt->liMac && rc.ypBottom < pfedt->rcView.ypBottom)
		{
		rcT = pfedt->rcView;
		rcT.ypTop = rc.ypBottom;
		PatBltRc( pfedt->hdc, &rcT, PATCOPY );
		}
}


/*----------------------------------------------------------------------------
|    FedtNotifyChange
|
|    Called when a fedt control has changed.
|
|    Arguments:
|        hfedt
----------------------------------------------------------------------------*/
/*  %%Function:  FedtNotifyChange  %%Owner:  bryanl       */

FedtNotifyChange(hfedt)
FEDT **hfedt;
{
	FedtNotifyParent(&hfedt, FN_CHANGE);
	Assert( hfedt != NULL && IsWindow( (*hfedt)->hwnd ) );
}


/*----------------------------------------------------------------------------
|    FFedtReformat
|
|    Reformats the given fedt control.
|
|    Arguments:
|        phfedt
|        ichMicDraw
|        ichMacDraw
|
|    Returns:
|        FALSE on errors.
|
|    WARNING!! - assumes the caret is off on entry.
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtReformat  %%Owner:  bryanl       */

BOOL FFedtReformat(phfedt, ichMicDraw, ichMacDraw)
FEDT ***phfedt;
int ichMicDraw;
int ichMacDraw;
{
	int liOld;
	int ichMicBrk, ichMacBrk;
	struct RC rc;
	BOOL fSuccess;
	HWND hwnd;

/* reset line breaks */

	ichMacDraw = min(ichMacDraw, (**phfedt)->cchText);
	liOld = (**phfedt)->liMac;
	fSuccess = FFedtComputeLiBrks(*phfedt, &ichMicBrk, &ichMacBrk);

/* redraw changed text */

	if (ichMicBrk >= 0)
		ichMicDraw = min(ichMicBrk, ichMicDraw);
	if (ichMacBrk <= (**phfedt)->cchText)
		ichMacDraw = max(ichMacBrk, ichMacDraw);
	FedtRedraw(*phfedt, ichMicDraw, ichMacDraw);
	/* we're in stable enough state to display oom alert now */
	if (!fSuccess)
		{
		switch (LOWORD(FedtNotifyParent(phfedt, FN_ERRSPACE|FN_OOMLINEBRKS)))
			{
		case FN_OOMALERT:
			SetErrorMat(matMem);
		case FN_OOMABORT:
		case FN_OOMRETRY:
		case FN_OOMIGNORE:
			break;
			}
		}
	return(fSuccess);
}


/*----------------------------------------------------------------------------
|    FFedtComputeLiBrks
|
|    Calculates line breaks for the given fedt control.
|
|    Arguments:
|        hfedt        fedt control to compute line breaks for
|        pichMic        place to return position of first line
|                break change
|        pichMac        place to return position of last line
|                break change
|
|    Returns:
|        FALSE on out of memory
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtComputeLiBrks  %%Owner:  bryanl       */

BOOL FFedtComputeLiBrks(hfedt, pichMic, pichMac)
FEDT **hfedt;
int *pichMic;
int *pichMac;
{
	char *pchBrk, *pchMic, *pchMac, *pchMax;
	char *pchT;
	int dx, dxMax;
	int liMac;
	int ichCaret;
	int ichBrk, ichMic, ichMac, ichMax;
	char *PchNextHardBrk();
	FEDT **FedtAddBreak();
	FEDT **FedtWrapText();
#ifdef WIN23
	HDC hdc;
	int fReleaseHdc;
#endif /* WIN23 */

	Debug(CheckFedt2(hfedt));

	*pichMic = pfedt->cchText;
	*pichMac = 0;
	pfedt->fBadBrks = FALSE;

/* scan through the entire text building line break array */
#ifdef WIN23
	hdc = 0;
	/* make sure we have an hdc in hfedt */
	if (!(fReleaseHdc = pfedt->hdc == NULL) || FFedtValidateDC(hfedt))
		hdc = pfedt->hdc;
#endif /* WIN23 */

	pchBrk = pchMic = PchFromHsz(pfedt->hszText);
	liMac = pfedt->liMac = 0;
	dxMax = pfedt->rcFmt.xpRight - pfedt->rcFmt.xpLeft;
	pchMax = pchMic + pfedt->cchText;
	while (pchMic <= pchMax)
		{

/* find last char that fits on the line, up to the next hard break char */

		if (pchMic >= pchBrk)
			pchBrk = PchNextHardBrk(pchMic, pchMax);
		if (pfedt->fSingleLine)
			pchMac = pchBrk;
		else
			{
			dx = dxMax;
#ifdef WIN23
			if (vsci.fWin3 && hdc != NULL)
				pchMac = PchLastFit3(hdc, vsci.dxpTmWidth, pchMic, pchBrk, &dx);
			else
#endif /* WIN23 */
			pchMac = PchLastFit(vsci.dxpTmWidth, pchMic, pchBrk, &dx);
			if (pchMac < pchBrk)
				{
/* insert soft break - scan back for first break char and put soft break 
	after it (if we find one); eat trailing white space, too */

				if (*pchMac != chSpace)
					{
					pchT = PchPrevTextBrk(pchMac-1, pchMic, TRUE);
					if (pchT >= pchMic)
						pchMac = pchT + 1;
					}
				while (pchMac < pchBrk && *pchMac == chSpace)
					pchMac++;
				}
			}

/* don't let hard break immediately follow soft break, and ensure at least 
	one char/line (except last line) */

		if (pchMac == pchBrk)
			pchMac++;
		if (pchMac <= pchMax && pchMac <= pchMic)
			pchMac = pchMic + 1;

/* bump to next line and insert the new break */

		pchMic = pchMac;
		if (pchMac > pchMax)
			pchMac = pchMax;

		pchT = PchFromHsz(pfedt->hszText);
		ichBrk = pchBrk - pchT;
		ichMic = pchMic - pchT;
		ichMac = pchMac - pchT;
		ichMax = pchMax - pchT;
		if (!FFedtAddBrk(hfedt, pchMac, pichMic, pichMac)) /* HEAP MOVES */
			{
			/* out of memory */
			*pichMic = 0;
			*pichMac = pfedt->cchText;
			pfedt->mpliichBrk[liMac++] = pfedt->cchText;
			pfedt->liMac = liMac;
			break;
			}
		pchT = PchFromHsz(pfedt->hszText);
		pchBrk = pchT + ichBrk;
		pchMic = pchT + ichMic;
		pchMac = pchT + ichMac;
		pchMax = pchT + ichMax;
		liMac++;
		}

/* fill unused line break positions with 0 */

	Assert(liMac == pfedt->liMac);
	SetWords( &pfedt->mpliichBrk [liMac], 0, pfedt->liMax - liMac );

/* reset caret position */

	ichCaret = pfedt->sel.rgichSel[pfedt->fMacCaret];
	pfedt->liCaret = LiFedtFromIch(hfedt, ichCaret);
	if (FFedtIchAfterHardBrk(hfedt, ichCaret, pfedt->liCaret))
		pfedt->liCaret++;

/* and we're done! */
#ifdef WIN23
	if (hdc != 0 && fReleaseHdc)
		FedtReleaseDC(hfedt);
#endif /* WIN23 */

	Debug(CheckFedt(hfedt));
	return(!pfedt->fBadBrks);
}



/*----------------------------------------------------------------------------
|    PchNextHardBrk
|
|    Scans forward looking for next hard break character.
|
|    Arguments:
|        pch        pointer to first character to check
|        pchMac        last character position to check
|
|    Returns:
|        pointer to a hard break character (LF)
----------------------------------------------------------------------------*/
/*  %%Function:  PchNextHardBrk  %%Owner:  bryanl       */

char *PchNextHardBrk(pch, pchMac)
char *pch;
char *pchMac;
{
	int cch;

	while (pch < pchMac)
		{
		if (*pch == chLF)
			break;
		pch++;
		}
	return(pch);
}


/*----------------------------------------------------------------------------
|    FFedtIchAfterHardBrk
|
|    Checks if character position is to the right of a hard break
|    on the line.
|
|    Arguments:
|        hfedt
|        ich
|        li
|
|    Returns:
|        TRUE if the character to the left is a hard break -- note
|        that this is an illegal situation if we're talking about
|        a selection end-point.
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtIchAfterHardBrk  %%Owner:  bryanl       */

BOOL FFedtIchAfterHardBrk(hfedt, ich, li)
FEDT **hfedt;
int ich;
int li;
{
	char *pchText;

	pchText = PchFromHsz(pfedt->hszText);
	return(pfedt->mpliichBrk[li] == ich &&
			ich > pfedt->mpliichBrk[li-1] && 
			pchText[ich-1] == chLF);
}


/*----------------------------------------------------------------------------
|    LiFedtFromIch
|
|    Calculates the line the given character is contained in.
|
|    Arguments:
|        hfedt
|        ich
|
|    Returns:
|        line number that character ich lives in.
----------------------------------------------------------------------------*/
/*  %%Function:  LiFedtFromIch  %%Owner:  bryanl       */

int LiFedtFromIch(hfedt, ich)
FEDT **hfedt;
int ich;
{
	int li;
	FEDT *pfedtT = *hfedt;

	Assert(ich >= 0);
	for (li = 0; li < pfedtT->liMac-1; li++)
		if (ich < pfedtT->mpliichBrk[li])
			break;
	Assert(li < pfedtT->liMac);
	return(li);
}


/*----------------------------------------------------------------------------
|    DxFedtText
|
|    Nothin' fancy, just computes the width of the specified text in
|    the fedt control.  Does not handle tabs.   A hard break, if one
|    exists in the string, but be the last character in the string.
|
|    Arguments:
|        hfedt
|        ichMic
|        cch
|
|    Returns:
|        width of the text in pixels.
----------------------------------------------------------------------------*/
/*  %%Function:  DxFedtText  %%Owner:  bryanl       */

int DxFedtText(hfedt, ichMic, cch)
FEDT **hfedt;
int ichMic;
int cch;
{
	char *pch;
#ifdef WIN23
	int fReleaseHdc;
	uns dx;
#endif /* WIN23 */

	if (cch == 0)
		return(0);
	pch = PchFromHsz(pfedt->hszText) + ichMic;
	if (*(pch + cch - 1) == chLF)
		cch--;
#ifdef WIN23
	if ((fReleaseHdc = (*hfedt)->hdc == NULL) && !FFedtValidateDC(hfedt))
		return(vsci.dxpTmWidth * cch);
		
	dx = (LOWORD(GetTextExtent((*hfedt)->hdc, (LPSTR)pch, cch)));
	if (fReleaseHdc)
		FedtReleaseDC(hfedt);
	return dx;
#else
	return(vsci.dxpTmWidth * cch);
#endif
}


/*----------------------------------------------------------------------------
|    DxFedtPos
|
|    Calculates the pixel position at the left of the given character 
|    on the given line.
|
|    Arguments:
|        hfedt        fedt control to calculate position
|        li        line ich lives on
|        ich        index of character to find position of
|
|    Returns:
|        horizontal position of the character
----------------------------------------------------------------------------*/
/*  %%Function:  DxFedtPos  %%Owner:  bryanl       */

int DxFedtPos(hfedt, li, ich)
FEDT **hfedt;
int li;
int ich;
{
	int ichMic;

	ichMic = pfedt->mpliichBrk[li-1];
	return(pfedt->rcFmt.xpLeft + DxFedtText(hfedt, ichMic, ich - ichMic));
}


/*----------------------------------------------------------------------------
|    FedtInvertRgn
|
|    Inverts a region in the fedt controls view area.
|
|    Arguments:
|        hfedt
|        hrgnInv
|
----------------------------------------------------------------------------*/
/*  %%Function:  FedtInvertRgn  %%Owner:  bryanl       */

FedtInvertRgn(hfedt, hrgnInv)
FEDT **hfedt;
HRGN hrgnInv;
{
	HRGN hrgnT;

	if (!IsWindowVisible(pfedt->hwnd))
		return;
	if ((hrgnT = CreateRectRgn(0,0,0,0)) == NULL)
		return;
	LogGdiHandle(hrgnT, 1005);
	if (!FFedtValidateDC(hfedt))
		{
		UnlogGdiHandle(hrgnT, 1005);
		DeleteObject(hrgnT);
		return;
		}
	SetRectRgn(hrgnT, pfedt->rcView.xpLeft, pfedt->rcView.ypTop,
			pfedt->rcView.xpRight, pfedt->rcView.ypBottom);
	if (CombineRgn(hrgnT, hrgnInv, hrgnT, RGN_AND) >= SIMPLEREGION)
		InvertRgn(pfedt->hdc, hrgnT);
	UnlogGdiHandle(hrgnT, 1005);
	DeleteObject(hrgnT);
}



/*----------------------------------------------------------------------------
|    FFedtAddBrk
|
|    Appends a line break to the end of the line break array in the
|    fedt control.
|
|    Arguments:
|        hfedt        fedt control to append break to
|        pchBrk        position of the break
|        pichMic       first line break change saved here
|        pichMac
|
|    Returns:
|        FALSE if new line break could not be allocated 
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtAddBrk  %%Owner:  bryanl       */

BOOL FFedtAddBrk(hfedt, pchBrk, pichMic, pichMac)
FEDT **hfedt;
char *pchBrk;
int *pichMic, *pichMac;
{
#define dliBrkAlloc 2
	int li, liOldMax, liNew;
	int ichBrk, ichOld;

	ichBrk = pchBrk - PchFromHsz(pfedt->hszText);
	li = pfedt->liMac;
	liOldMax = pfedt->liMax;
	if (li >= liOldMax)
		{
		liNew = max(liOldMax, li + dliBrkAlloc);
		if (!FChngSizeHCw( hfedt, CwFromCch(CbFedt(liNew)), fTrue))
			{
			pfedt->fBadBrks = TRUE;
			return(FALSE);
			}
		SetWords(  &(*hfedt)->mpliichBrk [liOldMax], 0, liNew - liOldMax);
		pfedt->liMax = liNew;
		}
	ichOld = pfedt->mpliichBrk[li];
	pfedt->mpliichBrk[li] = ichBrk;
	if (ichOld != ichBrk)
		{
		*pichMic = min(ichBrk, min(*pichMic, ichOld));
		*pichMac = max(ichBrk, max(*pichMac, ichOld));
		}
	pfedt->liMac = li+1;
	return(TRUE);
}


/*---------------------------------------------------------------------*/
/*----------------- Excel-equivalent routines -------------------------*/
/*---------------------------------------------------------------------*/
/*  %%Function:  PegRange  %%Owner:  bryanl       */

int PegRange(w, wFirst, wLast)
int w;
int wFirst, wLast;
{
	Assert(wLast >= wFirst);
	return(min(max(w, wFirst), wLast));
}




/*  %%Function:  FFedtValidateDC  %%Owner:  bryanl       */

FFedtValidateDC(hfedt)
FEDT **hfedt;
{
	HDC hdc;

	if (pfedt->hdc == NULL)
		{
		if ((hdc = GetDC(pfedt->hwnd)) == NULL)
			return fFalse;
		FedtInitDC( hfedt, hdc );
		}
	return fTrue;
}


/*  %%Function:  FedtReleaseDC  %%Owner:  bryanl       */

FedtReleaseDC(hfedt)
FEDT **hfedt;
{
	HDC hdc;

	if (hfedt != NULL && (hdc = pfedt->hdc) != NULL)
		{
		ReleaseDC( pfedt->hwnd, hdc );
		pfedt->hdc = NULL;
		}
}


#ifdef DEBUG

/*  %%Function:  CheckFedt  %%Owner:  bryanl       */

CheckFedt(hfedt)
FEDT **hfedt;
{
	if (!FValidFedt(hfedt))
		{
#ifdef BRYANL
		CommSzNum(SzFrame("Fedt Validate Error: "),wErrFedt);
#endif
		Assert( fFalse );
		}
}


/*  %%Function:  CheckFedt2  %%Owner:  bryanl       */

CheckFedt2(hfedt)
FEDT **hfedt;
{
	if (!FValidFedt2(hfedt))
		{
#ifdef BRYANL
		CommSzNum(SzFrame("Fedt Validate Error: "),wErrFedt);
#endif
		Assert( fFalse );
		}
}


/*----------------------------------------------------------------------------
|    FValidFedt
|
|    Checks for a valid fedt contorl.  FValidFedt2 does not check the
|    line break array, since it is in an inconsistent state in several
|    places.
|
|    Arguments:
|        hfedt        fedt to test
|
|    Returns:
|        FALSE if something is wrong.
----------------------------------------------------------------------------*/
/*  %%Function:  FValidFedt  %%Owner:  bryanl       */

BOOL FValidFedt(hfedt)
FEDT **hfedt;
{
	int li;

	if (!FValidFedt2(hfedt))
		return(FALSE);

	/* check line break array */
	if (pfedt->ichBrk0 != 0)
		return(wErrFedt=80, FALSE);
	for (li = 0; ; li++)
		{
		if (pfedt->mpliichBrk[li] > pfedt->cchText)
			return(wErrFedt=81, FALSE);
		if (li == pfedt->liMac - 1)
			break;
		if (pfedt->mpliichBrk[li-1] >= pfedt->mpliichBrk[li])
			return(wErrFedt=82, FALSE);
		}
	if (pfedt->mpliichBrk[pfedt->liMac-1] != pfedt->cchText)
		return(wErrFedt=83, FALSE);
	for (li = pfedt->liMac; li < pfedt->liMax; li++)
		{
		if (pfedt->mpliichBrk[li] != 0)
			return(wErrFedt=84, FALSE);
		}

	/* check caret line positions */
	if (!FInRange(pfedt->liCaret, 0, pfedt->liMac))
		return(wErrFedt=90, FALSE);
	/* can't have a caret to the right of a hardbreak */
	if (FFedtIchAfterHardBrk(hfedt, pfedt->sel.rgichSel[pfedt->fMacCaret], pfedt->liCaret))
		return(wErrFedt=91, FALSE);

	return(TRUE);
}


/*  %%Function:  FValidFedt2  %%Owner:  bryanl       */

BOOL FValidFedt2(hfedt)
FEDT **hfedt;
{
	FEDT fedt;

	fedt = **hfedt;    /* to reduce pointer references */

	/* check window */
	if (!IsWindow(fedt.hwnd))
		return(wErrFedt=10, FALSE);
	if (hfedt != HfedtFromHwnd(fedt.hwnd))
		return(wErrFedt=11, FALSE);

	/* check selection */
	if (fedt.cchText < 0)
		return(wErrFedt=30, FALSE);
	if (fedt.ichMicSel < 0)
		return(wErrFedt=31, FALSE);
	if (fedt.ichMicSel > fedt.ichMacSel)
		return(wErrFedt=32, FALSE);
	if (fedt.ichMacSel > fedt.cchText)
		return(wErrFedt=33, FALSE);
	if (fedt.hrgnSel == NULL)
		return(wErrFedt=35, FALSE);

#ifdef WHA
	/* check format rectangle */
	if (!FValidRect(&fedt.rcView))
		return(wErrFedt=40, FALSE);
	if (!FValidRect(&fedt.rcFmt))
		return(wErrFedt=41, FALSE);
#endif  /* WHA */

	/* check text */
	if (fedt.hszText == hNil)
		return(wErrFedt=50, FALSE);

	if (SzLen(PchFromHsz(fedt.hszText)) != fedt.cchText)
		{

#ifdef BZ
		CommSzNum(SzShared("FValidFedt2 fail 51 cchText: "), fedt.cchText);
		CommSzNum(SzShared(" hszText: "), fedt.hszText);
		CommSzSz(SzShared(" *hszText"), PchFromHsz(fedt.hszText));
#endif /* BZ */

		return(wErrFedt=51, FALSE);
		}
	/* check line count */
	if (fedt.liMac < 1)
		return(wErrFedt=70, FALSE);
	if (fedt.liMax < 1)
		return(wErrFedt=71, FALSE);
	if (fedt.liMac > fedt.liMax)
		return(wErrFedt=72, FALSE);

	return(TRUE);
}


/*  %%Function:  FInRange  %%Owner:  bryanl       */

FInRange( w, wFirst, wLast )
int w, wFirst, wLast;
{
	return (w >= wFirst && w <= wLast);
}


#endif
