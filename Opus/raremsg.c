/* R A R E M S G . C */


#ifdef DEBUG
#ifdef PCJ
/* #define SHOWYIELDWAITMSG */
#endif /* PCJ */
#endif /* DEBUG */

#define NOKEYSTATE
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMETAFILE
#define NOMINMAX
#define NOPEN
#define NOREGION
#define NOSOUND
#define NOTEXTMETRIC
#define NOWNDCLASS
#define NOCOMM
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#define NOIFK
#define USEBCM
#include "opuscmd.h"
#include "keys.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "screen.h"
#include "print.h"
#include "ch.h"
#include "format.h"
#include "status.h"
#include "wininfo.h"
#include "menu2.h"
#include "dde.h"
#include "helpapi.h"
#include "help.h"
#include "debug.h"
#include "field.h"
#include "cmdtbl.h"
#include "sdmver.h"
#include "sdm.h"
#include "core.h"
#include "idle.h"
#include "message.h"
#include "rareflag.h"
#include "file.h"
#include "doslib.h"
#include "layout.h"
#include "preview.h"
#include "error.h"
#include "prompt.h"

#define MPILCDBCM
#include "cmdlook.h"

extern BOOL             vfDdeIdle;
extern BOOL             vfRecording;
extern FARPROC          vlppFWHChain;
extern int              vfInitializing;
extern struct MWD       **hmwdCur;
extern int              vlm;
extern HANDLE  vhInstance;
extern HWND  vhwndApp;
extern HWND  vhwndDeskTop;
extern HMENU vhMenu;
extern BOOL  vfInitializing;
extern struct WWD       **hwwdCur;
extern int              wwCur;
extern struct WWD       **mpwwhwwd[];
extern struct MWD       **hmwdCur;
extern int              mwCur;
extern struct MWD       **mpmwhmwd[];
extern struct STTB      **vhsttbWnd;
extern struct SCI  vsci;
extern struct SEL  selCur;
extern struct PREF vpref;
extern struct FTI  vfti;
extern struct LCB  vlcb;
extern struct CA caPage;
extern struct CA caPara;
extern struct FLI vfli;
extern int wwCur;
extern BOOL vfOvertypeMode;
extern BOOL vfExtendSel;
extern BOOL vfBlockSel;
extern struct MERR vmerr;
extern char szClsStatLine[];
extern struct RC        vrcUnZoom;
extern int              mwCur;
extern int              mwMac; /* mac of child window */
extern int              wwMac;
extern int              wwCreating;
extern struct MWD       **mpmwhmwd[];
extern int              docGlobalDot;
extern struct DOD       **mpdochdod[];
extern struct SCI       vsci;
extern struct PRI       vpri;
extern struct FLI       vfli;
extern BOOL             vfWaitForDraw;
extern HWND             vhwndStatLine;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern HWND             vhwndAppIconBar;
extern HWND             vhwndDeskTop;
extern HWND             vhwndPrompt;
extern HWND             vhwndPgPrvw;
extern struct RC        vrcUnZoom;
extern struct PREF      vpref;
extern HANDLE           vhInstance;
extern HWND             vhwndApp;          /* handle to parent's window */
extern HWND             vhwndDeskTop;
extern HWND             vhwndCBT;
extern HWND             vhwndStatLine;
extern BOOL             vfExtendSel;
extern MSG              vmsgLast;
extern struct WWD       **mpwwhwwd[];
extern struct WWD       **hwwdCur;
extern BOOL             vfInLongOperation;
extern int              wwCur;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vfDeactByOtherApp;
extern HWND             vhwndMsgBoxParent;
extern IDF		vidf;
extern int              vfSysMenu;
extern int              vssc;
extern struct MERR      vmerr;
extern int              flashID;
extern int              vfFocus;
extern int              vfDoubleClick;
extern HCURSOR          vhcStyWnd;
extern CP               vcpFetch;
extern int              vfSeeSel;
extern char             szApp[];
extern int              docCur;
extern HWND             vhwndAppModalFocus;
extern BOOL		fRecordMove;
extern BOOL		fRecordSize;
extern int              vfLargeFrameEMS;
extern int              vcInMessageBox;
extern int              vfSmallFrameEMS;
extern int              vfUrgentAlloc;
extern PVS		vpvs;
extern int FAR pascal LbcCmpLbox();
extern WORD				wmWinHelp;
#ifdef WIN23
extern int              vdbmgDevice;
#endif /* WIN23 */

#ifdef BATCH
/* Batch Mode stuff */
BOOL vfBatchMode = fFalse;
CHAR szBatchString[128];
BOOL vfBatchIdle = fFalse;	/* if fTrue, do not exit upon idle in batch mode */
csconst char szOpusRip[] = "opus.rip";


/* %%Function:RipSz %%Owner:TONYKR */
/* Writes a string out to opus.rip */
RipSz(sz)
CHAR *sz;
{
	OFSTRUCT OutReOpenBuff;     /* output file reopen buffer */
	int hFile;

	if (OpenFile((LPSTR) szOpusRip, (OFSTRUCT far *)(&OutReOpenBuff),
			OF_EXIST) == -1)
		{
		hFile = (OpenFile((LPSTR) szOpusRip, (OFSTRUCT far *)(&OutReOpenBuff),
				OF_WRITE | OF_CREATE));
		}
	else
		{
		hFile = (OpenFile((LPSTR) szOpusRip, (OFSTRUCT far *)(&OutReOpenBuff),
				OF_WRITE));
		}
	if (hFile != -1)
		{
		DwSeekDw(hFile, 0, 2);	/* Go to end of file */
		CchWriteDoshnd(hFile, (LPSTR) sz, CchSz(sz) - 1);
		FCloseDoshnd(hFile);
		}
}


/* %%Function:BatchModeError %%Owner:BRADV */
BatchModeError(sz1, sz2, n1, n2)
CHAR *sz1, *sz2;
int n1, n2;
{
	CHAR rgch[256];
	CHAR *pch = rgch;

	DebugBreak(10);

	CchPchToPpch(szBatchString, &pch);
	if (sz1)
		CchPchToPpch(sz1, &pch);
	if (sz2)
		CchPchToPpch(sz2, &pch);
	CchPchToPpch(SzShared(" "), &pch);
	CchIntToPpch(n1, &pch);
	CchPchToPpch(SzShared(", "), &pch);
	CchIntToPpch(n2, &pch);
	*pch = 0;
#ifdef DEBUG
	DoStackTrace(rgch);
#else
	/* If we're not in debug version just write the message to opus.rip */
	*(pch++) = '\r';
	*(pch++) = '\n';
	*pch = '\0';
	RipSz(rgch);
#endif /* DEBUG */
	OurExitWindows();
}


#endif /* BATCH */


/* %%Function:AppWndProcRare %%Owner:chic */
long AppWndProcRare(hwnd, message, wParam, lParam)
HWND hwnd;
int message;
WORD wParam;
LONG lParam;
{
	HDLG hdlg;
	LPPOINT rgpt;

	switch (message)
		{
	default:
		Assert( fFalse );
		break;
	case WM_CREATE:
		/* tell CBT about new window */
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		vhwndApp = hwnd;
		break;

	case WM_CBTINIT:
		/* CBT sends us this message when it is ready to go */
		vrf.fCbtInit = fTrue;
		return ((LONG) fTrue);

	case WM_SYSTEMERROR:
		if (vrf.fCbtInit)
			break;
		if (wParam == 8)  /* wParam == 8 indicates out of memory */
			SetErrorMat(matMem);
		/* Fall through */

	case WM_CBTTERM:  /* CBT is quitting */
		/* Note: we also get WM_CBTTERM from DOT when it quits; but
				vfr.fRanCBT won't be true in that case */
		if (vhwndCBT != NULL && vrf.fRanCBT)
			{
			/* don't do these in StopCBT because we'll never get there
				(we'll be in the idle loops of dialog or menu */
			EndMenu();  /* end menu mode if we're in it */
			if ((hdlg = HdlgGetCur()) != NULL && FModalDlg(hdlg))
				/* close any open dialog(s) */
				EndDlg(tmcCancel);   /* ok to call EndDlg here */
			/* check for modeless dialog drop-downs open */
			if (HdlgGetFocus() != NULL)
				TermCurIBDlg(fFalse);

			/* postpone rest of CBT cleanup until CbtLib is no longer on
				the stack, since StopCBT does a FreeLibrary of hCbtLib */
			vrf.fMustStopCBT = fTrue;
			}
		else
			{
			vhwndCBT = NULL;
			vrf.fCbtInit = 2;
			}
		return(0L);

	case WM_CBTSEMEV:
		/* if CBT is not installed, we will get this message when
			we send it to ourselves (see below under WM_COMMAND). We
			must return TRUE so normal WM_COMMAND handling occurs */
		return ((LONG) fTrue);

	case WM_SIZE:
			/* Window's size is changing.  lParam contains the height
			** and width, in the high and low words, respectively. */

			{
			static dypApp;

			if (vidf.fIconic = IsIconic(hwnd))
				{
				if (vfInitializing) 
					dypApp = 0; /* make sure we init this */
				break;
				}
	
			if (vfRecording && fRecordSize)
				RecordAppSize(LOWORD(lParam), HIWORD(lParam));

			fRecordMove = fFalse;
			fRecordSize = fFalse;

			if (!vfInitializing || vhwndDeskTop != NULL)
				/* NOTE:  We are guaranteed that dypApp will be set by
					the time it is used here, because the first time
					through this code (at initialization time), the call
					to AppSize won't be made. */
				AppSize(HIWORD(lParam) < dypApp);

			dypApp = HIWORD(lParam);
			break;
			}

	case WM_MOVE:
		EnsureFocusInPane();

		if (vfRecording && fRecordMove)
			RecordAppMove(LOWORD(lParam), HIWORD(lParam));
		fRecordMove = fFalse;

defproc:    
		return(DefWindowProc(hwnd, message, wParam, lParam));

	case WM_TIMER:
		/* flashID timer event is used to flash the title bar when not
			active.  Other timer events may be used from time to time so
			we must check.
		*/
		if (wParam == flashID)
			{
			Assert (vfDeactByOtherApp);
			FlashWindow(vhwndApp, fTrue);
			}
		break;

	case WM_CLOSE:
		/* The user has selected "Close" on the system menu */
        /* in win3, the tak manager can send this via end task */
		/* Failure to process this message means that DefWindowProc */
		/* Will send us a Destroy message */
		/* A return value of TRUE says " don't close" */
		/* Calling DestroyWindow means "Go ahead and close" */

		/* When Windows sends this message, wParam is 0 (acQuerySave).
			This message might also be posted by the FileExit statement,
				in which case wParam will be the parameter used there. */

		Assert(hwnd == vhwndApp); /* Please tell Brad if this fails... */
		Assert((uns) wParam != acAutoSave);

        if (FCancelClose())
			return (fTrue);

		return !FAppClose(wParam /* ac */ );

	case WM_QUERYENDSESSION:
		{
		extern int vpisPrompt;

		BOOL f;

		/* User has selected "End Session" from the MS-DOS window */
		/* Return TRUE if willing to quit, else return FALSE */

        if (FCancelClose())
			return (fFalse);

		if (vidf.fDead)
			/* we are already shutting down, no need to go through this again */
			return (fTrue);

		vrf.fInQueryEndSession = fTrue;	 /* save dlg needs this */
		f = FQueryEndSession(acQuerySave);
		vrf.fInQueryEndSession = fFalse;   /* in case we don't end */
		if (f)
			{
			WriteUserState();
			return fTrue;
			}
		break;
		}
	case WM_ENDSESSION:
		/* if wParam is TRUE, Windows is shutting down.  This is the
			last message we ever get if the user closes down the session
			from the MSDOS executive. We only have to perform those
			termination actions that could have an effect after Windows
			goes away. */
		/* if wParam is FALSE, then an "End session" has been aborted */
		if (wParam)
			CleanUpForExit();
		break;

	case WM_DESTROY:
		/* Window is being destroyed. */
		AppDestroy();
		return (LONG) TRUE;
	case WM_SYSCOLORCHANGE:
	case WM_WININICHANGE:
	case WM_DEVMODECHANGE:
	case WM_FONTCHANGE:
		AppWinSysChange( message, lParam );
		break;


		/*---------------CLIPBOARD INTERACTION-------------*/

	case WM_RENDERFORMAT:
		/* A request to render the contents of the clipboard
			in the data format specified.  Reception of this message
			implies that the receiver is the current owner of the
			clipboard. See clipbord.c */
		FRenderFormat( wParam );
		break;

	case WM_DESTROYCLIPBOARD:
		/*  A notification that we are about to lose the ownership
			of the clipboard.  We should free any resources that are
			holding the contents of the clipboard */
		DestroyClip();
		break;

		/*-------CLIPBOARD DISPLAY---------------------*/

	case WM_PAINTCLIPBOARD:
		/* A request to paint the clipboard contents.
			wParam is a handle to the clipboard window
			LOWORD( lParam ) is a handle to a PAINTSTRUCT giving
				a DC and RECT for the area to repaint */

           /* could get here while playing a metafile, which would be
              trouble. Invalidate the whole rect so it will be
              painted sometime bz
           */
	    if (vrf.fInDisplayFli)
		    {
            InvalidateRect(wParam, (LPRECT) NULL, fTrue);
            break;
            }
		PaintClipboard( wParam, LOWORD(lParam) );
		break;

	case WM_VSCROLLCLIPBOARD:
		/* A request to vertically scroll the clipboard contents.
			wParam is a handle to the clipboard window
			LOWORD( lParam ) is the scroll type (SB_)
			HIWORD( lParam ) is the new thumb position (if needed) */

	    if (vrf.fInDisplayFli)
            break;

		VscrollClipboard( wParam, LOWORD(lParam), HIWORD(lParam) );
		break;

	case WM_HSCROLLCLIPBOARD:
		/* A request to horizontally scroll the clipboard contents.
			wParam is a handle to the clipboard window
			LOWORD( lParam ) is the scroll type (SB_)
			HIWORD( lParam ) is the new thumb position (if needed) */

	    if (vrf.fInDisplayFli)
            break;

		HscrollClipboard( wParam, LOWORD(lParam), HIWORD(lParam) );
		break;

	case WM_SIZECLIPBOARD:
		/* A notification that the clipboard window is being re-sized.
			wParam is a handle to the clipboard window
			LOWORD(lParam) is a handle to a RECT giving the new size */

	    if (vrf.fInDisplayFli)
		    break;

		SizeClipboard( wParam, LOWORD(lParam) );
		break;

	case WM_ASKCBFORMATNAME:
		/* A request for the name of the CF_OWNERDISPLAY clip format.
			wParam is the max. # of chars to store (including terminator)
			lParam is a long pointer to a buffer in which to store the
			name */

		AskCBFormatName( (LPCH) lParam, wParam );
		break;

	case WM_DDE_INITIATE:
		InitiateDde (wParam, LOWORD (lParam), HIWORD (lParam));
		break;

	case WM_NCLBUTTONDBLCLK:
		if (wParam == HTMENU && vfSysMenu)
			{
			POINT pt;
			RECT rc;

			pt = MAKEPOINT(lParam);
			ScreenToClient(hwnd, (LPPOINT) &pt);
			GetWindowRect(hwnd, (LPRECT) &rc);
			ScreenToClient(hwnd, (LPPOINT) &rc);
			rc.top += vsci.dypBdrCapt;
			if (pt.x < vsci.dxpBmpSystem + 2 * vsci.dxpTmWidth && 
					pt.y >= rc.top && pt.y < rc.top + vsci.dypMenuBar)
				{
				CmdExecBcmKc(bcmCloseWnd, kcNil);
				break;
				}
			}
		goto defproc;
		}

	return 0L;
}


/* %%Function:FCancelClose %%Owner:bobz */
/* returns true and puts up a message if we are not prepared to close */
FCancelClose()
{
	    HDLG hdlg;

		/* what we are doing here: we can get into trouble if message boxes
		   or dialogs are up at exit windows time, or we are in an external
           library that can bring up mb or dialogs. If we go ahead and try
		   to close,we get more dialogs and message boxes brought up and in
		   certain cases we will lose track of who is activated. So, to
		   prevent that (and to avoid saving and quitting inthe middle of
		   a replace or spell, for example, we don't allow end session
		   while message boxes or modal dialogs are up.

           Note that we can get into similar problems at wm_close in win3
           with the task manager end task call. In that case we should not
           get the message if we are deactivated, (when a dialog or
           message box is up), but doing all the checks
           should cover us when something changes.
		   
		   bz 9/20/89
		*/

		if (vcInMessageBox || vrf.fInExternalCall ||
			vpisPrompt >= pisAbortCheckMin ||
			vhwndAppModalFocus != NULL ||
			((hdlg = HdlgGetCur()) != hdlgNull && FModalDlg(hdlg)))
			{
            /* must be a system modal message box */
			ErrorEid (eidCantQuitWord, "AppWndProcRare");
			return (fTrue);
			}
        return fFalse;   /* ok to close */
}




/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:DeskTopWndProc %%Owner:chic */
EXPORT long FAR PASCAL DeskTopWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	Assert (message == WM_SIZE);
	/* no effect to the doc windows inside except if vpref.fZoomMwd */
	if (vpref.fZoomMwd && hmwdCur != hNil)
		DoZoom((*hmwdCur)->hwnd);
	return(0L);
}






/*----------------------------------------------------------------------------
|	WaitActivation
|
|	This routine waits until we are active, while it waits it
|	flashes the title bar to show how impatient it is.
|
|	Arguments:
|		none.
----------------------------------------------------------------------------*/
/* %%Function:WaitActivation %%Owner:chic */
BOOL WaitActivation()
{
	MSG msg;

	Assert(vfDeactByOtherApp);

	if (vidf.fDead || InSendMessage ())
		return;

	Beep();

	SetTimer(vhwndApp, flashID = tidFlash, 500, (LPSTR)NULL);

	while (vfDeactByOtherApp)
		{
		GetMessage((MSG far *)&msg, NULL, 0, 0);
		/* Activated by clicking on our non-client area; don't allow
			menus to drop down */ /* Don't accept command messages unless
			wer're active either... */
		if ((msg.message != WM_COMMAND && msg.message != WM_NCLBUTTONDOWN) || 
				msg.hwnd != vhwndApp)
			{
#ifdef SHOWYIELDWAITMSG
			ShowMsg ("wa", msg.hwnd, msg.message, msg.wParam,
					msg.lParam);
#endif /* SHOWYIELDWAITMSG */
			TranslateMessage((MSG far *)&msg);
			DispatchMessage((MSG far *)&msg);
			}
		}
}



/* %%Function:LoadPreviewFont %%Owner:chic */
LoadPreviewFont()
{
	char *stPrev = StFrame("PREV.FON");
	char szBuffer[cchMaxSz];
	char stFull[ichMaxFile];

	vpvs.fLoadPrvwFon = fFalse;

	if (FFindFileSpec(stPrev, stFull, grpfpiUtil, nfoNormal))
		StToSzInPlace(stFull);
	else
		{
		StToSz(stPrev, stFull);
		/* Avoid "Cannot find PREV.FON" message box if they're not around */
		if (OpenFile((LPSTR)stFull, (LPOFSTRUCT)szBuffer, OF_EXIST) == -1)
			return;
		}
	AddFontResource((LPSTR)stFull);
}

#ifdef WIN23
/* We have two arrays because the vdbmg ordering is different under Win 3 */
/* REVIEW: CGA SIGMA, & 8514 widths */
csconst int mpdbmgTmWidth3[] = {10, 8, 8};
csconst int mpdbmgTmWidth2[] = {10, 8, 8, 8, 8};
#endif /* WIN23 */

/* F  A S S U R E  S D M  I N I T */
/* We postpone initialization of SDM until needed.  Now is the time
	to do it!
*/
/* %%Function:FAssureSdmInit %%Owner:chic */
FAssureSdmInit()
{
	/* Initialize SDM and then unload it... */
	extern FAR pascal sdminit_q();
	extern BOOL FFilterSdmMsg();
	extern HANDLE vhInstance;
	extern HWND vhwndApp, vhwndCBT;
	extern char szClsFedt[];
	extern HCURSOR     vhcArrow;

	BOOL fReturn;
	SDI sdi;

	Scribble(13, 'S');

	sdi.hinstCur = vhInstance;
	sdi.hinstPrev = NULL; /* no prev instances of opus! */
	sdi.strApp = szApp;
	sdi.hwndApp = vhwndApp;
	sdi.pfnFilterMsg = FFilterSdmMsg;
	sdi.strFedtClass = szClsFedt;
	sdi.hcursorArrow = vhcArrow;

	Assert (vsci.hdcScratch != hNil);
	sdi.hdcMem = vsci.hdcScratch;
#ifdef WIN23
	/*
	We want SDM to size based on the width of the old sys font so all our
	dialogs size as they used to. We want to use the height of the Win 3
	sys font if appropriate, since it is bigger than win 2
	*/
	if (vsci.fWin3Visuals)
		sdi.dxSysFontChar =  mpdbmgTmWidth3[vdbmgDevice];  /* Fake Width of system font */
	else if (vsci.fWin3)	/* under win 3, but not new visuals */
		sdi.dxSysFontChar =  mpdbmgTmWidth2[vdbmgDevice];  /* Fake Width of system font */
	else
		sdi.dxSysFontChar =  vsci.dxpTmWidth;  /* Real Width of system font */
	sdi.dySysFontChar =  vsci.dypTmHeight; /* Height of system font */
#else
	sdi.dxSysFontChar =  vsci.dxpTmWidth;  /* Width of system font */
	sdi.dySysFontChar =  vsci.dypTmHeight; /* Height of system font */
#endif /* WIN23 */
	sdi.dySysFontAscent =  vsci.dypTmAscent;
	sdi.szScrollClass = NULL;

	sdi.lpfncomp = LbcCmpLbox;   /* list box compare rtn */
	/* the following two values are new in the sdi structure for
		sdm 2.22 (added for the WIN23 version). */
	sdi.hfont = NULL;
	sdi.dyLeading = vsci.dypTmInternalLeading;

	if (!(fReturn = FInitSdm(&sdi)))
		SetErrorMat(matMem);
	else  if (vhwndCBT)
		CBTState(fTrue);  /* tell SDM that CBT is here */

	if (fReturn)
		ChangeColors(); /* set system colors */

	vmerr.fSdmInit = fReturn; /* we will try again if it failed */

	GlobalLruOldest( GetCodeHandle( (FARPROC)sdminit_q ) );

	Scribble(13, ' ');

	return fReturn;
}






/* %%Function:FRetrySbError %%Owner:chic */
BOOL FRetrySbError(merr, sb, lcbNew, cretry)
int merr;
SB sb;
long lcbNew;
WORD cretry;
{
	return fFalse;
}


/* %%Function:FRetrySdmError %%Owner:chic */
BOOL FRetrySdmError(w, hdlg, sev)
int w;
HDLG hdlg;
SEV sev;
{
	extern int vsasCur;

	if (sev == sevMinor)
		return fFalse;

	if (sev == sevDirFill)
		{
		SetErrorMat(matNil); /* don't need to report another error */
		ErrorEid(eidLowMemLBox, "FRetrySdmError");
		return fFalse;
		}

	if (vsasCur != sasMin)
		{
		ShrinkSwapArea();
		Assert(vsasCur == sasMin);
		return fTrue;
		}

	/* this is necessary since retry may be for windows failure instead of
		a memory failure (memory failures are covered in FRetryLmemError)*/
	SetErrorMat(matLow);

	/* set flags to take down ruler or ribbon if hdlg associated */

	if (hdlg != hdlgNull && hdlg != hdlgCabError && !FModalDlg(hdlg))
		SetEndIbdlg(hdlg);

	/* for major problems, sdm will take down the dialog.
       We no longer call EndDlg ourselves */
	return fFalse;
}


extern int wwCur;
extern KMP ** hkmpCur;
extern KMP ** hkmpBase;
extern MSG vmsgLast;
extern struct WWD ** hwwdCur;
extern vgrfMenukeysAreDirty;

extern KME vkme;


#define ckmeGrow        10

KME * PkmeFromKc();


extern KMP ** hkmpSearch;
extern int ikmeSearch;


/* R E M O V E  K E Y  F R O M  K M P */
/* %%Function:RemoveKeyFromKmp %%Owner:chic */
RemoveKeyFromKmp(hkmp, kc, bcm)
KMP ** hkmp;
int kc, bcm;
{
	KMP * pkmp;
	KME * pkme;
	int ikme;

	if (FSearchKmp(hkmp, kc, &ikme))
		{
		pkmp = *hkmp;
		pkme = &pkmp->rgkme[ikme];
		bltb(pkme + 1, pkme,
				sizeof (KME) * (--pkmp->ikmeMac - ikme));
		}
}


/* E N D  K E Y  M O D E */
/* Finish mode by calling optional function specified in keymap and
make sure the keymap gets removed from the chain. */
/* %%Function:EndKeyMode %%Owner:chic */
EndKeyMode()
{
	int ikme;
	PFN pfn = NULL;

	/* The following can happen during initialization... */
	if (hkmpCur == hNil || !(*hkmpCur)->fModal)
		return;

	if (FSearchKmp(hkmpCur, kcModal, &ikme))
		{
		pfn = (*hkmpCur)->rgkme[ikme].pfn;
		}

	RemoveKmp(hkmpCur);

	if (pfn)
		(*pfn)();
}


/* F  C O P Y  K M P  T O  K M P */
/* %%Function:FCopyKmpToKmp %%Owner:chic */
FCopyKmpToKmp(hkmpSrc, hkmpDest)
KMP ** hkmpSrc, ** hkmpDest;
{
	int cw;
	/* expr split up due to compiler problems */
	cw = CwOfPfgr(hkmpSrc);
	if (!FChngSizeHCw(hkmpDest, cw, fTrue))
		return fFalse;

	bltb(*hkmpSrc, *hkmpDest, cw * 2);

	return fTrue;
}


#ifdef DEBUG
CMD CmdDumpKeymap(pcmb)
CMB *pcmb;
{
	KMP ** hkmp;
	KMP * pkmp;
	KME * pkme;
	int ikme, ikmeMac;
	char * pch;
	char sz[20];
	for (hkmp = hkmpCur; hkmp != hNil; hkmp = (*hkmp)->hkmpNext)
		{
		extern KMP ** vhkmpUser;
		if (hkmp == hkmpCur)
			CommSz(SzShared("Current!\r\n"));
		if (hkmp == hkmpBase)
			CommSz(SzShared("Base!\r\n"));
		if (hkmp == vhkmpUser)
			CommSz(SzShared("User!\r\n"));
		pkmp = *hkmp;
		CommSzNum(SzShared("ikmeMac:"), ikmeMac = pkmp->ikmeMac);
		CommSzNum(SzShared("grpf: "), pkmp->grpf);
		pch = sz;
		if (pkmp->fStopHere)
			*pch++ = 'S';
		if (pkmp->fModal)
			*pch++ = 'M';
		if (pkmp->fBeep)
			*pch++ = 'B';
		if (pkmp->fTranslate)
			*pch++ = 'T';
		if (pkmp->fAppModal)
			*pch++ = 'A';
		*pch++ = '\0';
		if (*sz != '\0')
			CommSzSz(SzShared("Bits: "), sz);
		CommSz(SzShared("--------------------\r\n"));
		}
#ifdef TOOVERBOSE
	for (pkme = &pkmp->rgkme[ikme = 0]; ikme < ikmeMac; ++ikme, ++pkme)
		{
		CommSzNum(SzShared("kc: "), pkme->kc);
		CommSzNum(SzShared("kt: "), pkme->kt);
		CommSzNum(SzShared("value: "), pkme->w);
		}
#endif
	return cmdOK;
}


#endif /* DEBUG */




#ifdef DEBUG
/* %%Function:CheckHkmp %%Owner:chic */
CheckHkmp(hkmp)
KMP ** hkmp;
{
	KMP * pkmp;
	KME * pkme;
	int kc;
	int ikme, ikmeMac;

	AssertH(hkmp);
	pkmp = *hkmp;

	if (pkmp->hkmpNext != hNil)
		AssertH(pkmp->hkmpNext);

	ikmeMac = pkmp->ikmeMac;
	Assert(ikmeMac >= 0);
	Assert(ikmeMac <= pkmp->ikmeMax);

	/* assert keymap is sorted */
	kc = kcNil;
	for (pkme = &pkmp->rgkme[ikme = 0]; ikme < ikmeMac; ++ikme, ++pkme)
		{
#ifndef INTL
		Assert(pkme->kc > kc);
#else  /* INTL */
		Assert(pkme->kc >= kc);
#endif /* INTL */
		kc = pkme->kc;
		}
}


#endif



#define ihwndMax 6

/* A P P  S I Z E */
/* handles WM_SIZE message for App window */

/* %%Function:AppSize %%Owner:chic */
AppSize(fShrink)
BOOL fShrink;
{
	struct RC rc;
	int ihwnd = 0;
	int i;
	HWND hwnd, rghwnd[ihwndMax]; /* WARNING: increase if add to rghwnd */

	Assert (!vfInitializing || vhwndDeskTop != NULL);


	if (vhwndStatLine)
		rghwnd[ihwnd++] = vhwndStatLine;

	if (vhwndPrompt)
		rghwnd[ihwnd++] = vhwndPrompt;

	Assert( vhwndDeskTop );
	rghwnd[ihwnd++] = vhwndDeskTop;

	if (vhwndRibbon)
		rghwnd[ihwnd++] = vhwndRibbon;

	if (vhwndAppIconBar != NULL)
		rghwnd[ihwnd++] = vhwndAppIconBar;

	if (vhwndPgPrvw != NULL)
		rghwnd[ihwnd++] = vhwndPgPrvw;

	i = ihwnd;
	Assert(i <= ihwndMax);
	ihwnd = (fShrink ? i - 1 : 0);

	/* This loop is set up to loop forwards through the hwnds if
		we are growing the Application and backwards if we are
		shrinking it. */

	while (i--)
		{
		hwnd = rghwnd[ihwnd];

		if (hwnd == vhwndStatLine || hwnd == vhwndPrompt)
			{
			Assert( vsci.dypStatLine );
			GetClientRect( vhwndApp, (LPRECT) &rc );
			rc.ypBottom += vsci.dypBorder;
			rc.ypTop = rc.ypBottom - vsci.dypStatLine;
			rc.xpLeft -= vsci.dxpBorder;
			rc.xpRight += vsci.dxpBorder;
			MoveWindowRc( hwnd, &rc, fTrue );
			}
		else  if (hwnd == vhwndDeskTop)
			{
			GetDeskTopPrc( &rc );
			MoveWindowRc( vhwndDeskTop, &rc, fTrue );
			}
		else  if (hwnd == vhwndRibbon || hwnd == vhwndAppIconBar)
			{
/* NOTE: height of ribbon and macro iconbar are the same - dypRibbon
but outline and header iconbar should be dypIconBar */
			Assert( vsci.dypRibbon );
			GetClientRect( vhwndApp, (LPRECT) &rc );
			rc.ypTop -= vsci.dxpBorder;
			rc.xpLeft -= vsci.dxpBorder;
			rc.xpRight += vsci.dxpBorder;
			rc.ypBottom = rc.ypTop + vsci.dypRibbon;
			MoveWindowRc( hwnd, &rc, fTrue );
			}
		else  if (hwnd == vhwndPgPrvw)
			{
			GetClientRect(vhwndApp, (LPRECT) &rc);
			/* leave room for preview icon bar */
			rc.ypTop += (vsci.dypRibbon - vsci.dypBorder);
			MoveWindowRc( hwnd, &rc, fTrue );
			}

		ihwnd += (fShrink ? -1 : 1);
		}
}



/* G E T  D E S K  T O P  P R C */

/* Return rect for desktop through parm.
	Results depend on:
			(1) size of app window
			(2) presence of ribbon
			(3) presence of status line */

/* %%Function:GetDeskTopPrc %%Owner:chic */
GetDeskTopPrc( prc )
struct RC *prc;
{
	GetClientRect( vhwndApp, (LPRECT)prc );

	if (vhwndStatLine)
		{
		Assert( vsci.dypStatLine );
		prc->ypBottom -= vsci.dypStatLine - vsci.dypBorder;
		}
	if (vhwndRibbon || vhwndAppIconBar != NULL)
		{
/* NOTE: height of ribbon and macro iconbar are the same - dypRibbon
but outline and header iconbar should be dypIconBar */
		Assert( vsci.dypRibbon );
		prc->ypTop += vsci.dypRibbon - vsci.dypBorder;
		}
}


/* G E T  N O N  P A N E  R G D Y P */
/* get height of non pane area in ww (currently that includes
	ruler/iconbar only) */

/* %%Function:GetNonPaneRgdyp %%Owner:chic */
GetNonPaneRgdyp(hmwd, rgdyp)
struct MWD **hmwd;
int rgdyp[];
{
	int iww, ww;

	rgdyp[0] = rgdyp[1] = 0;

	for ( iww = 0 ; iww < 2 ; iww++ )
		if ((ww = (*hmwd)->rgww[iww]) != wwNil)
			{
			if (ww == (*hmwd)->wwRuler)
				rgdyp[iww] += vsci.dypRuler - vsci.dypBorder;
			if (PwwdWw(ww)->hwndIconBar)
				rgdyp[iww] += vsci.dypIconBar - vsci.dypBorder;
			}
}



/* %%Function:AdjustStyNameDxw %%Owner:chic */
AdjustStyNameDxw(hmwd, pdxw)
struct MWD **hmwd;
int *pdxw;
{
	int dxwNew = *pdxw;
	struct RC rc;

	if (dxwNew <= vsci.dxpBorder)
	/* not worth it */
		dxwNew = 0;
	else
		{
		GetClientRect((*hmwd)->hwnd, (LPRECT)&rc);
		dxwNew = min(rc.xpRight>>1, dxwNew); /* make sure dxpNew is not too big */
		}
	*pdxw = dxwNew;
}




/*  Resize MWDs as necessary so they won't be obscured if Status Line
	or Ribbon is brought up. */

/* %%Function:ResizeMwds %%Owner:chic */
ResizeMwds()
{
	int ihwnd;
	struct RC rc, rcT;
	HWND hwnd;
	HWND rghwnd[mwMax];
	int ypBottom;

	if (hmwdCur == hNil) /* fairly bogus test to see if any windows are open */
		return; /* nothing to do! */

	/* get top to bottom list of windows */

	rghwnd[ihwnd = 0] = (*hmwdCur)->hwnd;

	while ((hwnd = rghwnd[ihwnd]) != hNil)
		{
		rghwnd[++ihwnd] = GetNextWindow(hwnd, GW_HWNDNEXT);
		Assert (ihwnd < mwMax);
		}
	GetClientRect(vhwndDeskTop, (LPRECT) &rcT);

	/* move the windows in reverse order so that all windows are
	stacked in the right order and the current one is on top */

	while (ihwnd > 0)
		{
		GetWindowRect( rghwnd[--ihwnd], (LPRECT) &rc );
		ScreenToClient( vhwndDeskTop, (LPPOINT) &rc.ptTopLeft );
		ScreenToClient( vhwndDeskTop, (LPPOINT) &rc.ptBottomRight );
		ypBottom = rc.ypBottom;
		if (ypBottom > rcT.ypBottom + vsci.dypBorder)
			{
			rc.ypBottom = rcT.ypBottom;
			MoveWindowRc( rghwnd[ihwnd], &rc, fTrue );
			}
		}
}




/* A D D  L O O K S  K E Y S */
/* Add all of the character and paragraph formatting keyboard commands
	from hkmpSearch to hkmpAdd. */
/* %%Function:AddLooksKeys %%Owner:chic */
AddLooksKeys(hkmpSearch, hkmpAdd, ilcdMaxAdd)
KMP ** hkmpSearch, ** hkmpAdd;
int ilcdMaxAdd;
{
	KMP ** hkmpCurSav;
	int ilcd, bcm;

	Assert(ilcdMaxAdd <= sizeof(mpilcdbcm)/sizeof(int));

	hkmpCurSav = hkmpCur;
	hkmpCur = hkmpSearch;

	/*  warning - strong position dependance on bcm, ilcd  */
	for (ilcd = 0; ilcd < ilcdMaxAdd; ++ilcd)
		if ((bcm = mpilcdbcm [ilcd]) != bcmNil)
			AddAllKeysOfBcm(hkmpSearch, hkmpAdd, bcm, bcm);

	hkmpCur = hkmpCurSav;
}


/* R E C O R D  L O O K S */
/* %%Function:RecordLooks %%Owner:chic */
RecordLooks(ilcd)
int ilcd;
{
	int bcm = mpilcdbcm [ilcd];

	Assert(vfRecording);
	if (bcm != bcmNil)
		GenStatement(bcm);
}


/* A D D  K E Y  K T  W */
/* Bind a key of an arbitrary type */
/* %%Function:AddKeyKtW %%Owner:chic */
AddKeyKtW(hkmp, kc, kt, w)
KMP ** hkmp;
int kc, kt, w;
{
	KME * pkme;

	pkme = PkmeAddKc(hkmp, kc);
	if (pkme != 0)
		{
		pkme->kt = kt;
		pkme->w = w;
		}
}



/* F E D T  R A R E  R O U T I N E S */

#include "fedt.h"

/* Macros to keep the rest of this module as Excel-like as reasonable */

#define fAlt        (wKbsOptionMask)
#define fShift      (wKbsShiftMask)
#define fControl    (wKbsControlMask)
#define FOREVER     for ( ;; )
#define chLF        (chEol)
#define chBackSpace (chBS)
#define chNull      '\0'
#define SzLen(sz)   (CchSz(sz)-1)

#define pfedt (*hfedt)

/*----------------------------------------------------------------------------
|    FFedtCopy
|
|    Performs the Copy command on the given fedt control, i.e., puts
|    the text of the current selection in the clipboard.
|
|    Arguments:
|        phfedt        pointer to fedt control to copy from
|
|    Returns:
|        TRUE if successful
----------------------------------------------------------------------------*/
/* %%Function:FFedtCopy %%Owner:bobz */
BOOL FFedtCopy(phfedt)
FEDT ***phfedt;
{
	char *pch;
	int ich, cch;
	HANDLE hdata;
	FEDT **hfedt;
	char far *lpch;

	hfedt = *phfedt;
	Debug(CheckFedt(hfedt));

	if (!OpenClipboard(pfedt->hwnd))
		return(FALSE);
	EmptyClipboard();
	pch = PchFromHsz(pfedt->hszText) + pfedt->ichMicSel;
	cch = pfedt->ichMacSel - pfedt->ichMicSel;
	cch += CchCountCh(pch, pch + cch, chLF);
	while ((hdata = OurGlobalAlloc(GHND, (long)(uns)(cch+1))) == NULL)
		{
		switch (LOWORD(FedtNotifyParent(&hfedt, FN_ERRSPACE|FN_OOMCOPY)))
			{
		case FN_OOMALERT:
LError:
			SetErrorMat(matMem);
		case FN_OOMABORT:
		case FN_OOMIGNORE:
			CloseClipboard();
			*phfedt = hfedt;
			return(FALSE);
		case FN_OOMRETRY:
			break;
			}
		}
	if ((lpch = GlobalLockClip(hdata)) == NULL)
		goto LError;
	for (ich = pfedt->ichMicSel; ich < pfedt->ichMacSel; ich++)
		{
		/* map linefeeds into cr-lf */
		if (*pch == chLF)
			*lpch++ = chReturn;
		*lpch++ = *pch++;
		}
	*lpch = 0;
	GlobalUnlock(hdata);
	SetClipboardData(CF_TEXT, hdata);
	CloseClipboard();
	*phfedt = hfedt;
	return(TRUE);
}


/*----------------------------------------------------------------------------
|    FFedtClear
|
|    Clears the text of the current selection in the given fedt control.
|
|    Arguments:
|        phfedt        fedt control to clear the selection in
|
|    Returns:
|        TRUE if successful
----------------------------------------------------------------------------*/
/* %%Function:FFedtClear %%Owner:bobz */
BOOL FFedtClear(phfedt)
FEDT ***phfedt;
{
	char rgch[2];

	Debug(CheckFedt(*phfedt));

	if (!FFedtReplSel(phfedt, (char far *)rgch, 0))
		return(FALSE);
	FedtAdjustSel(*phfedt);
	return(TRUE);
}


/*----------------------------------------------------------------------------
|    FFedtPaste
|
|    Performs the Paste operation to the given fedt control, i.e.,
|    replaces the current selection with the text from the clipboard.
|
|    Arguments:
|        phfedt
|
|    Returns:
|        TRUE if successful
----------------------------------------------------------------------------*/
/* %%Function:FFedtPaste %%Owner:bobz */
BOOL FFedtPaste(phfedt)
FEDT ***phfedt;
{
	char far *lpch;
	HANDLE hdata, hdata2;
	FEDT **hfedt;
	HANDLE HFedtStripText();

	hfedt = *phfedt;
	Debug(CheckFedt(hfedt));

	if (!OpenClipboard(pfedt->hwnd))
		return(FALSE);
	hdata = GetClipboardData(CF_TEXT);
	/* BUG!! - it would be nice if we allowed LFs in chart alpha
		pasting */
	if ((hdata2 = HFedtStripText(hfedt, hdata)) == NULL
			|| (lpch = GlobalLockClip(hdata2)) == NULL)
		{
		CloseClipboard();
		return(FALSE);
		}
	if (FFedtReplSel(&hfedt, lpch, CchLpszLen(lpch)))
		FedtAdjustSel(hfedt);
	GlobalUnlock(hdata2);
	if (hdata != hdata2)
		GlobalFree(hdata2);
	CloseClipboard();
	*phfedt = hfedt;
	return(TRUE);
}


/*----------------------------------------------------------------------------
|    HFedtStripText
|
|    Strips text of all bizzaro characters.
|
|    Arguments:
|        hdata        handle to global object containing
|                text to check.
|
|    Returns:
|        handle to modified new object, hdata if data is ok, or
|        NULL if errors.
----------------------------------------------------------------------------*/
/* %%Function:HFedtStripText %%Owner:bobz */
HANDLE HFedtStripText(hfedt, hdata)
FEDT **hfedt;
HANDLE hdata;
{
	char far *lpch, far *lpchSav, far *lpch2;
	int ch;
	int fStopAtLF = pfedt->fSingleLine;
	HANDLE hdata2;

	if (hdata == NULL || (lpch = GlobalLock(hdata)) == NULL)
		return(NULL);

	lpchSav = lpch;
	/* look for ugly characters in the text */
	while  ((ch = *lpch++) != chNull)
		{
		if (ch < chSpace)
			{
			/* found an ugly character - allocate a work area and
				copy modified text into it */
/* NOTE: GHND 0-inits, no terminator necessary */
			hdata2 = OurGlobalAlloc(GHND, GlobalSize(hdata));
			if (hdata2 == NULL || (lpch2 = GlobalLock(hdata2)) == NULL)
				SetErrorMat(matMem);
			else
				{
				lpch = lpchSav;
				while ((ch = *lpch) != chNull)
					{
					if (ch < chSpace)
						{
						if (ch == chReturn)
							{
							ch = chLF;
							if (*(lpch+1) == chLF)
								lpch++;
							}
						if (ch == chLF)
							{
							if (fStopAtLF)
								break;
							}
						else
							ch = chSpace;
						}
					*lpch2++ = ch;
					lpch++;
					}
				GlobalUnlock(hdata);
				}
			hdata = hdata2;
			break;
			}
		}
	if (hdata != NULL)
		GlobalUnlock(hdata);
	return(hdata);
}
