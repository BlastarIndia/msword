/* C M D W N D . C */
/* windowing commands  */


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
#include "version.h"
#include "heap.h"
#define NOIFK
#define USEBCM
#include "opuscmd.h"
#include "keys.h"
#include "doc.h"
#include "disp.h"
#include "splitter.h"
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
#include "help.h"
#include "debug.h"
#include "field.h"
#include "cmdtbl.h"
#include "core.h"
#include "idle.h"
#include "layout.h"
#include "preview.h"
#include "message.h"
#include "rsb.h"
#include "resource.h"
#include "error.h"
#include "rerr.h"
#include "scc.h"
#include "el.h"
#include "macrocmd.h"
#include "automcr.h"
#include "doslib.h"

extern MES ** vhmes;
extern struct STTB **   vhsttbOpen;
extern BOOL             vfFileCacheDirty;
extern BOOL             vfDdeIdle;
extern HBRUSH           vhbrGray;
extern HWND             vhwndPgPrvw;
extern BOOL             vfRecording;
extern struct SCC       vsccAbove;
extern struct WWD **vhwwdOrigin;
extern FARPROC          vlppFWHChain;
extern int              vfInitializing;
extern struct MWD       **hmwdCur;
extern int              vlm;
extern PVS              vpvs;
extern int              mwCur;
extern int              mwMac; /* mac of child window */
extern int              wwMac;
#ifdef WIN23
extern struct BMI *mpidrbbmi;
#else
extern struct BMI mpidrbbmi[];
#endif /* WIN23 */
extern int              wwCreating;
extern struct MWD       **mpmwhmwd[];
extern int              docGlobalDot;
extern struct DOD       **mpdochdod[];
extern struct RC        vrcUnZoom;
extern struct SCI       vsci;
extern struct PRI       vpri;
extern struct FLI       vfli;
extern BOOL             vfWaitForDraw;
extern struct PREF      vpref;
extern HANDLE           vhInstance;
extern HWND             vhwndApp;          /* handle to parent's window */
extern HWND             vhwndDeskTop;
extern HWND             vhwndPgvCtl;
extern int              vmwClipboard;
extern BOOL             fElActive;
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
extern int              vfSmallFrameEMS;
extern int              vfUrgentAlloc;
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
extern int              vcInMessageBox;



/* %%Function:CmdArrangeWnd %%Owner:chic */
CMD CmdArrangeWnd(pcmb)
CMB *pcmb;
{
	HWND hwnd;
	int cmw;
	int crow, ccol;
	int irow, icol;
	int x, y;
	int dx, dy;
	int dxDesk, dyDesk;
	struct RC rc;
	HWND rghwndMw[mwMax];
	int ihwnd;
	BOOL fRecSav;
	extern BOOL vfRecording;

	/* turn off recorder for now */
	InhibitRecorder(&fRecSav, fTrue);

	Assert(hmwdCur);

	/* get top to bottom list of window, so current window is always on top
		row and left most column after the arrangment */
	rghwndMw[ihwnd = 0] = (*hmwdCur)->hwnd;
	while ((hwnd = rghwndMw[ihwnd]) != hNil)
		{
		rghwndMw[++ihwnd] = GetNextWindow(hwnd, GW_HWNDNEXT );
		}
	cmw = ihwnd; /* window count */

	crow = min(cmw, 3); /* max 3 rows */

	GetClientRect(vhwndDeskTop, (LPRECT)&rc);
	dxDesk = rc.xpRight - rc.xpLeft;
	dyDesk = rc.ypBottom - rc.ypTop;
	dy = dyDesk/crow;
	y = -dy;
	irow = icol = -1;
	ccol = 0;

	for (ihwnd = 0; ihwnd < mwMax && (hwnd = rghwndMw[ihwnd]) != hNil; ihwnd++)
		{
		if (++icol >= ccol)
			{
			irow++;
			icol = 0;
			y += dy;
			x = 0;
			ccol = cmw/crow;
			/* assign leftovers to bottommost rows */
			if (cmw-ccol*crow >= crow-irow)
				ccol++;
			dx = dxDesk/ccol;
			}
		else
			x += dx;

		Assert(ccol);

		if (vpref.fZoomMwd && hwnd == (*hmwdCur)->hwnd)
			{
			vpref.fZoomMwd = fFalse;
			SetRect((LPRECT)&vrcUnZoom, x, y, x+dx, y+dy);
			UnZoomWnd(hwnd, &vrcUnZoom);
			SetWindowText(vhwndApp, (LPSTR) szAppTitle);
/* make sure child system menu is in the right place */
			CorrectSystemMenu();
			}
		else
			SetWindowPos(hwnd, NULL, x, y, dx, dy,
					SWP_NOACTIVATE|SWP_NOZORDER);
		}

	InhibitRecorder(&fRecSav, fFalse);

	return cmdOK;
}





/* %%Function:CmdSizeWnd %%Owner:chic */
CMD CmdSizeWnd(pcmb)
CMB *pcmb;
{
	SendMessage((*hmwdCur)->hwnd, WM_SYSCOMMAND, SC_SIZE, 0L);
	return cmdOK;
}


/* %%Function:CmdMoveWnd %%Owner:chic */
CMD CmdMoveWnd(pcmb)
CMB *pcmb;
{
	SendMessage((*hmwdCur)->hwnd, WM_SYSCOMMAND, SC_MOVE, 0L);
	return cmdOK;
}


/* %%Function:CmdRestoreWnd %%Owner:chic */
CMD CmdRestoreWnd(pcmb)
CMB *pcmb;
{
	HWND hwnd;

	if (!vpref.fZoomMwd)
		return cmdOK;

	vpref.fZoomMwd = fFalse;

	Assert(mwCur != mwNil && mpmwhmwd[mwCur] != hNil);
	hwnd = (*mpmwhmwd[mwCur])->hwnd;
	UnZoomWnd(hwnd, &vrcUnZoom);
	SetWindowText(vhwndApp, (LPSTR) szAppTitle);
/* make sure child system menu is in the right place */
	CorrectSystemMenu();
	return cmdOK;
}


/* %%Function:CmdZoomWnd %%Owner:chic */
CMD CmdZoomWnd(pcmb)
CMB *pcmb;
{
	HWND hwndMw;

	vpref.fZoomMwd = !vpref.fZoomMwd;

	Assert(mwCur != mwNil && mpmwhmwd[mwCur] != hNil);
	hwndMw = (*mpmwhmwd[mwCur])->hwnd;
	if (vpref.fZoomMwd)
		{
		/* There is no rc information to ZoomWnd because there is a
		standard way to figure it out and is redudent to spend codes
		in every callers.
		The app's caption is also set up in ZoomWnd for the same
		reason above. */
		ZoomWnd( hwndMw );
		}
	else
		{
		/* There is a need to pass in a rc to UnZoomWnd because although
		most of the time we use vrcUnZoom, there are times we want to
		use a different rc, e.g. when we are creating a window when
		we already have a zoomed window, we want to create the new
		window in zoomed size first, then unzoom the old window, there-
		fore we have to save away the old unzoom size in another rc to
		be passed to UnZoomWnd. */
		UnZoomWnd( hwndMw, &vrcUnZoom );

		/* Reset the app's caption to contain only the app name.
		This is only necessary if going to a nonzoom state and not
		necessary whenever we want to unzoom a window because a ZoomWnd
		would have followed in other places where UnZoomWnd is called
		and would have set up the app's caption appropriately. */
		SetWindowText( vhwndApp, (LPSTR)szAppTitle );
		}

/* make sure child system menu is in the right place */
	CorrectSystemMenu();

	return cmdOK;
}


/* C M D  F I L E  C L O S E */
/* %%Function:CmdFileClose %%Owner:chic */
CMD CmdFileClose(pcmb)
CMB * pcmb;
{
	BOOL fRecordingSav;
	CMD cmd = cmdOK;

	Assert(hwwdCur != hNil);

	/* don't close window if there is msg box up, else focus of msg box
	is returned to destroyed window */
	if (vcInMessageBox > 0) 
		return cmdCancelled;

	/* Stop any recording while we are closing file */
	InhibitRecorder(&fRecordingSav, fTrue);


	if ((*hwwdCur)->wk == wkMacro)
		{
		int mwCurSav;
		int docDot;

		docDot = PmeiImei(ImeiFromMw(mwCur))->elg;

		mwCurSav = mwCur;
		CloseDocWin((*hmwdCur)->hwnd, acQuerySave);
		if (mwCurSav == mwCur)
			{
			cmd = cmdCancelled;
			goto LRet;
			}

		/* Close the macros template (iff not global) */
		if (docDot != docGlobalDot && mpdochdod[docDot] != hNil &&
				!FCloseAllWinsOfDoc(docDot, acQuerySave))
			{
			cmd = cmdCancelled;
			}
		}
	else
		{
		if (!FCloseAllWinsOfDoc(selCur.doc, acQuerySave))
			cmd = cmdCancelled;
		}

LRet:
	InhibitRecorder(&fRecordingSav, fFalse);
	return cmd;
}


/* %%Function:ElFileClose %%Owner:chic */
EL ElFileClose(ac)
uns ac;
{
	int rerr = rerrNil;
	BOOL fRecordingSav;

	if (ac > 2)
		RtError(rerrIllegalFunctionCall);

	Assert(hwwdCur != hNil);

	/* don't close window if there is msg box up, else focus of msg box
	is returned to destroyed window */
	if (vcInMessageBox > 0) 
		{
		RtError(rerrCommandFailed); /* to stop the macro from continue */
		return;
		}

	/* Stop any recording while we are closing file */
	InhibitRecorder(&fRecordingSav, fTrue);
	

	if ((*hwwdCur)->wk == wkMacro)
		{
		extern RERR vrerr;

		int docDot;

		docDot = PmeiImei(ImeiFromMw(mwCur))->elg;

		CloseDocWin((*hmwdCur)->hwnd, ac);
		if (vrerr != rerrNil)
			{
			rerr = rerrHalt;
			goto LRet;
			}

		/* Close the macros template (iff not global) */
		if (docDot != docGlobalDot && mpdochdod[docDot] != hNil &&
				!FCloseAllWinsOfDoc(docDot, ac))
			{
			rerr = rerrCommandFailed;
			}

		goto LRet;
		}

	if (!FCloseAllWinsOfDoc(selCur.doc, ac))
		{
		rerr = rerrCommandFailed;
		}

LRet:
	InhibitRecorder(&fRecordingSav, fFalse);
	if (rerr != rerrNil)
		RtError(rerr);
}


/* Returns fFalse iff cancelled */
/* %%Function:FCloseAllWinsOfDoc %%Owner:chic */
FCloseAllWinsOfDoc(doc, ac)
int doc, ac;
{
	extern BOOL vfDisableAutoMacros;
	BOOL fDisableAutoMacrosSav;

	Assert(doc != docNil);

	doc = DocMother (doc);

	Assert (doc != docNil && 
			!PdodDoc(doc)->fShort);

	if (PdodDoc(doc)->docHdr != docNil)
		FCleanUpHdr(doc, fFalse, fFalse);

	if (PdodDoc(doc)->fDot && !FCloseDotMacros(doc, ac))
		return fFalse;

	if (CmdRunAtmOnDoc(atmClose, doc) == cmdCancelled)
		return fFalse;

	if (!FConfirmSaveDoc(doc, fTrue /* fForce */, ac))
		return fFalse;

	/* if preview is up, kill it, too! */
	if (vlm == lmPreview)
		FExecCmd(bcmPrintPreview);

	if (mpdochdod[doc] != docNil)
		{
		/* Disable the AutoClose macro while we close the 
			windows since we already did it! */
		fDisableAutoMacrosSav = vfDisableAutoMacros;
		vfDisableAutoMacros = fTrue;

		CloseMwsOfDoc(doc);

		vfDisableAutoMacros = fDisableAutoMacrosSav;
		}

	return fTrue;
}


/* C L O S E  D O C */
/*  Close all windows on doc */
/* %%Function:CloseMwsOfDoc %%Owner:chic */
CloseMwsOfDoc(doc)
int doc;
{
	int ww, wwPrev;

	Assert (doc != docNil && !PdodDoc(doc)->fShort);

	UndirtyDoc(doc);  /* so we don't get subsequent prompts */
	wwPrev = wwNil;
	while ((ww = WwDisp(doc, wwNil,fFalse)) != wwNil && ww != wwPrev)
		{
		SendMessage( PmwdMw(PwwdWw(wwPrev = ww)->mw)->hwnd, WM_CLOSE, 0, 0L );
		if (mpdochdod [doc] == hNil)
			/* doc was disposed of by above SendMessage */
			break;
		}
}



/* C m d  S t a t u s  A r e a */
/* %%Function:CmdStatusArea %%Owner:chic */
CMD CmdStatusArea(pcmb)
CMB *pcmb;
{
	extern int vfInitializing;
	extern HWND vhwndDeskTop;
	extern HWND vhwndStatLine;
	struct RC   rc;
	BOOL        fShow;
	int fHaveStatLine = (vhwndStatLine != NULL);
	int df;

	StartLongOp ();

	if (!fHaveStatLine)
		{
		/* There is no status line, bring one up. */
		if (!FCreateStatLine())
			{
			SetErrorMat(matMem);
			return cmdError;
			}
		}
	else
		/* Take down the existing window. */
		DestroyStatLine();

	/* If we're initializing, the status line window will get
		shown once the initialization process is done, so we
		don't need to do it here.  Also, at initialization, the
		desktop is sized with the status line in mind, so we
		don't need to adjust the MWDs here. */

	if (!vfInitializing)
		{
		GetDeskTopPrc( &rc );
		MoveWindowRc( vhwndDeskTop, &rc, fTrue );
		if (vhwndStatLine)
			{
			/* Get MWDs out of the way if necessary. */
			ResizeMwds();
			MakeStatLineVisible();
			}
		}

LRet:
	EndLongOp (fFalse /* fAll */);
	return cmdOK;
}


/* F  C r e a t  e  S t a t  L i n e */
/* %%Function:FCreateStatLine %%Owner:chic */
BOOL NEAR FCreateStatLine()
{
	struct RC rc;

	GetClientRect( vhwndApp, (LPRECT) &rc );

	if ((vhwndStatLine = CreateWindow((LPSTR)szClsStatLine, (LPSTR)NULL,
			WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER,
			rc.xpLeft - vsci.dxpBorder,
			rc.ypBottom - vsci.dypStatLine + vsci.dypBorder,
			rc.xpRight - rc.xpLeft + 2 * vsci.dxpBorder, vsci.dypStatLine,
			vhwndApp, NULL, vhInstance, (LPSTR)NULL)) == NULL)
		return (fFalse);

	InitStatLineState();

	return (fTrue);
}


/* M a k e  S t a t  L i n e  V i s i b l e */
/* The status line is created invisble initially. This routine is called
	to make it visible. */

/* %%Function:MakeStatLineVisible %%Owner:chic */
MakeStatLineVisible()
{
	ShowWindow( vhwndStatLine, SHOW_OPENWINDOW );
	UpdateWindow( vhwndStatLine );
}


/* I n i t  S t a t  L i n e  S t a t e */
/* %%Function:InitStatLineState %%Owner:chic */
InitStatLineState()
{
	extern struct SLS vsls;
	struct SLS sls;

	SetWords(&sls, 0, cwSLS);
	FGetStatLineState(&sls,usoNormal);
	blt( &sls, &vsls, cwSLS );
}


/* D e s t r o y  S t a t  L i n e */
/* %%Function:DestroyStatLine %%Owner:chic */
DestroyStatLine()
{
	Assert(vhwndStatLine != NULL);

	AssertDo(DestroyWindow(vhwndStatLine));
	vhwndStatLine = NULL;
}



/* %%Function:FCreatePgvCtls %%Owner:chic %%reviewed: 7/10/89 */
FCreatePgvCtls(ww)
int ww;
{
	extern CHAR szClsRSB [];
	struct WWD **hwwd = HwwdWw(ww);
	struct BMI *pbmi;
	struct RC rcVertBar, rc;

	if ((*hwwd)->hwndVScroll)
		{
		GetVertBarRc(ww, &rcVertBar);
		rc = rcVertBar;
		rc.ypBottom = rcVertBar.ypTop + vsci.dypScrollArrow;
		if (((*hwwd)->hwndPgvUp = HwndCreateWindowRc( szClsRSB,
				WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | maskWndPgVw,
				rc,
				PmwdWw(ww)->hwnd )) == NULL)
			return fFalse;

		SetWindowWord( (*hwwd)->hwndPgvUp, 
				offset( RSBS, bcm ), bcmPgvPrevPage );
		SetWindowWord( (*hwwd)->hwndPgvUp, 
				offset( RSBS, ww ), ww );

		rc = rcVertBar;
		rc.ypTop = rcVertBar.ypBottom - vsci.dypScrollArrow;
		if (((*hwwd)->hwndPgvDown = HwndCreateWindowRc( szClsRSB,
				WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | maskWndPgVw,
				rc,
				PmwdWw(ww)->hwnd )) == NULL)
			{
			DestroyPhwnd(&(*hwwd)->hwndPgvUp);
			return fFalse;
			}
		SetWindowWord( (*hwwd)->hwndPgvDown, 
				offset( RSBS, bcm ), bcmPgvNextPage );
		SetWindowWord( (*hwwd)->hwndPgvDown, 
				offset( RSBS, ww ), ww );
		GetClientRect((*hwwd)->hwndPgvDown, &rc);
#ifdef WIN23
		pbmi = &mpidrbbmi[vsci.fWin3Visuals ? idrbPageview3 : idrbPageview2];
#else
		pbmi = &mpidrbbmi[idrbPageview];
#endif /* WIN23 */
		pbmi->dxp = pbmi->dxpEach = rc.xpRight - rc.xpLeft;
		pbmi->dyp = rc.ypBottom - rc.ypTop;
		}
	return fTrue;
}


/* %%Function:DestroyPgvCtls %%Owner:chic %%reviewed: 7/10/89 */
DestroyPgvCtls(ww)
int ww;
{
	struct WWD **hwwd = HwwdWw(ww);
	struct RC rcVertBar;

	if ((*hwwd)->hwndPgvUp)
		{
		DestroyPhwnd(&(*hwwd)->hwndPgvUp);
		}
	if ((*hwwd)->hwndPgvDown)
		{
		DestroyPhwnd(&(*hwwd)->hwndPgvDown);
		}
	if ((*hwwd)->hwndVScroll)
		{
		GetVertBarRc(ww, &rcVertBar);
		MoveWindowRc((*hwwd)->hwndVScroll, &rcVertBar, fTrue);
		}
}


/* %%Function:ShowPgvCtls %%Owner:chic %%reviewed: 7/10/89 */
ShowPgvCtls(ww, fMove)
int ww;
int fMove;
{
	struct WWD **hwwd = HwwdWw(ww);
	HWND hwnd, hwndScroll;
	struct RC rcVertBar;
	struct RC rcT;
	int dyp = vsci.dypScrollArrow - vsci.dypBorder;

	if (hwndScroll = (*hwwd)->hwndVScroll)
		{
		GetVertBarRc(ww, &rcVertBar);
		rcT = rcVertBar;
		rcT.ypTop += dyp;
		rcT.ypBottom -= dyp;
		MoveWindowRc((*hwwd)->hwndVScroll, &rcT, fTrue);
		hwnd = (*hwwd)->hwndPgvUp;
		if (fMove)
			{
			rcT = rcVertBar;
			rcT.ypBottom = rcT.ypTop + vsci.dypScrollArrow;
			MoveWindowRc(hwnd, &rcT, fTrue);
			}
		CWDisplayAndUpdateControl(hwnd);
		CWDisplayAndUpdateControl(hwndScroll);
		hwnd = (*hwwd)->hwndPgvDown;
		if (fMove)
			{
			rcT = rcVertBar;
			rcT.ypTop = rcT.ypBottom - vsci.dypScrollArrow;
			MoveWindowRc(hwnd, &rcT, fTrue);
			}
		CWDisplayAndUpdateControl(hwnd);
		}
}


/* %%Function:DestroyVScrlWw %%Owner:chic */
DestroyVScrlWw(ww)
int ww;
{
	DestroyPhwnd( &PwwdWw(ww)->hwndVScroll );
	if (PwwdWw(ww)->fPageView)
		DestroyPgvCtls(ww);
}


/* Force all pageview panes to have passed grpfvisi */
/* %%Function:SyncPageViewGrpfvisi %%Owner:chic  %%reviewed: 7/10/89*/
SyncPageViewGrpfvisi(docMother, grpfvisi, fDisplayAsPrint)
int docMother;
GRPFVISI grpfvisi;
{
	int ww = wwNil;
	struct WWD *pwwd;

	Assert(docMother == DocMother(docMother));
	while ((ww = WwDisp(docMother, ww,fFalse)) != wwNil)
		{
		pwwd = PwwdWw(ww);
		pwwd->fDisplayAsPrint = fDisplayAsPrint;
		if (pwwd->fPageView && pwwd->grpfvisi.w != grpfvisi.w)
			{
			pwwd->grpfvisi = grpfvisi;
			TrashWw(ww);
			}
		}
}



/* C M D  N E W  W N D */
/* %%Function:CmdNewWnd %%Owner:chic */
CMD CmdNewWnd(pcmb)
CMB *pcmb;
{ /* duplicate another instance of the current doc window */

	int doc = DocMother(selCur.doc);
	struct SELS sels;

	blt(&selCur, &sels, cwSELS);  /* for the goback cache */

	if (!FCreateMw(doc, wkDoc, &sels, NULL))
		{
		ErrorEid(eidSessionComplex, " CmdNewWnd");
		return cmdError;
		}
	return cmdOK;
}


/* C M D  C L O S E  W N D */
/* %%Function:CmdCloseWnd %%Owner:chic */
CMD CmdCloseWnd(pcmb)
CMB *pcmb;
{
	if (vlm == lmPreview)
		CmdExecBcmKc(bcmPrintPreview, kcNil);
	else
		SendMessage((*hmwdCur)->hwnd, WM_CLOSE, 0, 0L);

	return cmdError; /* so we don't get recorded! */
}



/* %%Function:ElDocClose %%Owner:chic */
EL ElDocClose(ac)
unsigned ac;
{
	if (ac > 2)
		RtError(rerrIllegalFunctionCall);

	CloseDocWin((*hmwdCur)->hwnd, ac);
}


/* %%Function:CmdMwd1 %%Owner:chic */
CMD CmdMwd1(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(0);
}


/* %%Function:CmdMwd2 %%Owner:chic */
CMD CmdMwd2(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(1);
}


/* %%Function:CmdMwd3 %%Owner:chic */
CMD CmdMwd3(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(2);
}


/* %%Function:CmdMwd4 %%Owner:chic */
CMD CmdMwd4(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(3);
}


/* %%Function:CmdMwd5 %%Owner:chic */
CMD CmdMwd5(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(4);
}


/* %%Function:CmdMwd6 %%Owner:chic */
CMD CmdMwd6(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(5);
}


/* %%Function:CmdMwd7 %%Owner:chic */
CMD CmdMwd7(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(6);
}


/* %%Function:CmdMwd8 %%Owner:chic */
CMD CmdMwd8(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(7);
}


/* %%Function:CmdMwd9 %%Owner:chic */
CMD CmdMwd9(pcmb)
CMB *pcmb;
{
	return CmdSetMwd(8);
}


/* %%Function:CmdSetMwd %%Owner:chic */
CMD CmdSetMwd(imw)
int imw;
{
	CHAR HUGE *hpst;
	if (imw >= (*vhsttbWnd)->ibstMac)
		{
		if (fElActive)
			RtError(rerrOutOfRange);
		return cmdError;
		}
	hpst = HpstFromSttb(vhsttbWnd, imw);
	SetMw((int)*(hpst + *hpst));
	return cmdOK;
}


/* %%Function:SetMw %%Owner:chic */
SetMw(mw)
int mw;
{
	extern BOOL vfRecording;
	HWND hwndMw = PmwdMw(mw)->hwnd;
	HWND hwndMwOld = PmwdMw(mwCur)->hwnd;
	struct RC rcUnZoomSave;
	BOOL fRecordingSav;

	InhibitRecorder(&fRecordingSav, fTrue);

	if (mw != mwCur)
		NewCurWw( PmwdMw(mw)->wwActive, fFalse);

	InhibitRecorder(&fRecordingSav, fFalse);
}


/* C M D  O T H E R  P A N E */
/* %%Function:CmdOtherPane %%Owner:chic */
CMD CmdOtherPane(pcmb)
CMB *pcmb;
{
	int ww;
	extern struct SEL selMacro;
	BOOL fRecordingSav;

	if ((ww = WwOther(wwCur)) != wwNil)
		{
		InhibitRecorder(&fRecordingSav, fTrue);
		NewCurWw(ww, fFalse);
		InhibitRecorder(&fRecordingSav, fFalse);
		}

	if (PwwdWw(wwCur)->wk == wkMacro && (*vhmes)->fAnimate)
		{
		TurnOffSel(&selMacro);
		blt(&selCur, &selMacro, cwSEL);
		TurnOnSel(&selMacro);
		}

	return cmdOK;
}


/* C M D  N E X T  M W D */

/* %%Function:CmdNextMwd %%Owner:chic */
CMD CmdNextMwd(pcmb)
CMB *pcmb;
{
	NextPrevMwd( 1 /* dmw */ );
	return cmdOK;
}



/* C M D  P R E V  M W D */

/* %%Function:CmdPrevMwd %%Owner:chic */
CMD CmdPrevMwd(pcmb)
CMB *pcmb;
{
	NextPrevMwd(- 1 /* dmw */);
	return cmdOK;
}


/* N E X T  P R E V  M W D */
/* go through mpmwhmwd looking for the next entry referring to an
existing child window; cycle back to the beginning of array if
necessary */

/* %%Function:NextPrevMwd %%Owner:chic */
NextPrevMwd(dmw)
int dmw;
{
	int mw, imw;
	int cmw = mwMac - mwDocMin;

	Assert( mwMac > 1 );

	/* get next (prev) window handle; skip NULL handles */
	/* cycle through the mw's mod mwMac */
	Assert( (*hmwdCur)->hwnd != NULL );
	mw = mwCur;
	do
		{
		mw = mwDocMin + ((mw - mwDocMin + dmw + cmw) % cmw);
		}
	while (mw != vmwClipboard && mpmwhmwd[mw] == NULL);
	/* clipboard window has no entry in our window menu */

	if (mw != mwCur && (imw = IbstMw(mw)) >= 0)
		/* imw is index in list of open windows (on Window menu) */
		CmdSetMwd(imw);
}


/* I B S T  M W */
/* %%Function:IbstMw %%Owner:chic */
IbstMw(mw)
int mw;
{
	extern struct STTB **vhsttbWnd;
	int ibstMac = (*vhsttbWnd)->ibstMac;
	int ibst;
	CHAR HUGE *hpst;

	for (ibst = 0; ibst < ibstMac; ibst++)
		{
		hpst = HpstFromSttb(vhsttbWnd, ibst);
		if ((int)*(hpst + *hpst) == mw) /* found it */
			return ibst;
		}

	return ibstNil;
}




/* G E T  V E R T  B A R  R C S */
/* %%Function:GetVertBarRc %%Owner:chic %%reviewed: 7/10/89 */
GetVertBarRc(ww, prcVertScroll)
int ww;
struct RC *prcVertScroll;
{
	int mw = PwwdWw(ww)->mw;
	struct MWD **hmwd;
	struct RC rcMw;
	struct RC rcTopPane, rcBottomPane;
	struct RC rcTopScrlBar, rcBottomScrlBar;
	struct RC rcSplitBox, rcSplitBar;

	Assert(mw > mwNil && mw < mwMac);
	hmwd = mpmwhmwd[mw];
	GetDocAreaPrc(*hmwd, &rcMw);
	GetDocAreaChildRcs(hmwd, rcMw, wkDoc, (*hmwd)->ypSplit, &rcTopPane,
			&rcTopScrlBar, &rcSplitBox, &rcSplitBar, &rcBottomPane, &rcBottomScrlBar);
	Assert(ww == (*hmwd)->wwUpper || ((*hmwd)->fSplit && ww == (*hmwd)->wwLower));
	*prcVertScroll = (ww == (*hmwd)->wwUpper) ? rcTopScrlBar : rcBottomScrlBar;
}




/* %%Function:ShowStyWnd %%Owner:chic */
ShowStyWnd(mw, dxwNew, fSetDxw)
int mw;
int dxwNew; /* new width of the styname wnd */
{ /* Close, open or change the styname wnd size for a particular ww */
	/* We come to here only when there is something to do */
	struct MWD **hmwd;

	Assert( mw != mwNil && dxwNew >= 0 );

	hmwd = mpmwhmwd[mw];

	AdjustStyNameDxw(hmwd, &dxwNew);
	if (fSetDxw)
		vpref.dxaStyWnd = DxaFromDxs(0/*dummy*/, dxwNew); /* so new window will not new width */

	SetHmwdDxwStyWnd(hmwd, dxwNew, fSetDxw);
	selCur.fUpdateRuler = fTrue;

	SynchSccWw(&vsccAbove, wwCur); /* invalidate the screen cache also */
}


/* %%Function:SetHmwdDxwStyWnd %%Owner:chic */
SetHmwdDxwStyWnd(hmwd, dxwStyWnd, fSetDxw)
struct MWD **hmwd;
int dxwStyWnd;
int fSetDxw;
{
	int ww = (*hmwd)->wwUpper;

	SetDxwStyWndInWw(ww, dxwStyWnd, fSetDxw);
	if ((*hmwd)->ypSplit != 0)
		{
		Assert((*hmwd)->fSplit);
		SetDxwStyWndInWw((*hmwd)->wwLower, dxwStyWnd, fSetDxw);
		}
}




/* %%Function:SetDxwStyWndInWw %%Owner:chic */
SetDxwStyWndInWw(ww, dxwStyWnd, fSetDxw)
int ww;
int dxwStyWnd;
int fSetDxw;
{
	struct WWD *pwwd = PwwdWw(ww);
	struct RC rcInval;
	struct DR *pdr;
	int ddxw = dxwStyWnd - pwwd->xwSelBar;

	FreezeHp();
	pwwd->xwMin += ddxw; /* adjust rcwDisp */
	/* don't do this because the conversion routine already takes into account
	of rcePage and xwMin 
	pwwd->xeScroll += ddxw; adjust rcePage 
	*/
	pwwd->xwSelBar = dxwStyWnd;

#ifdef BOGUS
	if (!pwwd->fPageView)
		/* adjust DRCL */
		{
		pdr = PdrGalley(pwwd);
		RcToDrc(&pwwd->rcwDisp, &pdr->drcl);
		pdr->xl = pdr->yl = 0;
		}
#endif /* BOGUS */

	if (fSetDxw)
		{
		pwwd->dxwStyWnd = dxwStyWnd;
		}
	if (fSetDxw || ww != wwCur)
		{ /* the wwCur test is for wwOther of the one going into Page view */
		/* invalidate the area so it will be repainted */
		GetClientRect(pwwd->hwnd, (LPRECT)&rcInval);
		InvalidateRect(pwwd->hwnd, (LPRECT)&rcInval, fTrue/*fEraseBkGround*/);
		}
	MeltHp();
}




/* %%Function:TrackStyWndCursor %%Owner:chic */
TrackStyWndCursor(pt)
struct PT pt;
{
	struct RC rcLim;

	/* convert to screen coordinate first */
	ClientToScreen( (*hmwdCur)->hwnd, (LPPOINT) &pt );

	/* restrict the point within the client area of ww */
	GetClientRect((*hmwdCur)->hwnd, (LPRECT)&rcLim);
	if (PwwdWw((*hmwdCur)->wwUpper)->hwndIconBar != hNil) /* clip away the iconbar height */
		rcLim.ypTop += vsci.dypIconBar;
	rcLim.xpRight = rcLim.xpRight >> 1; /* don't let the stywnd takes up the whole window */

	if (vhcStyWnd == NULL && !FAssureIrcds(ircdsStyWnd))
		return;

	if (FTrackDragLine((*hmwdCur)->hwnd, &pt, vhcStyWnd, rcLim, fFalse /*fHorz*/))
		{
		/* dxpNew should actually be 
			xpRight (i.e. pt.xp) - xpLeft (i.e. -vsci.dxpBorder) 
		*/
		ShowStyWnd(mwCur, pt.xp + vsci.dxpBorder/*dxwNew*/, fTrue/*fSetDxw*/);
		}
}


/* Emulate Mac ROM call that does nothing if *ppt is in prc, but
	moves *ppt to closest border of prc if *ppt is outide prc.
*/
/* %%Function:PtIntoRc %%Owner:chic */
PtIntoRc(ppt, prc)
struct PT *ppt;
struct RC *prc;
{
	if (ppt->xp < prc->xpLeft)
		ppt->xp = prc->xpLeft;
	else  if (ppt->xp > prc->xpRight)
		ppt->xp = prc->xpRight;

	if (ppt->yp < prc->ypTop)
		ppt->yp = prc->ypTop;
	else  if (ppt->yp > prc->ypBottom)
		ppt->yp = prc->ypBottom;
}



/* %%Function:FTrackDragLine %%Owner:chic */
FTrackDragLine(hwnd, ppt, hCursor, rcLim, fHorz)
HWND hwnd; /* where the tracking is going to be in */
struct PT *ppt; /* initial position in screen coordinates,
	final position in client coordinates when return fTrue */
HCURSOR hCursor;
struct RC rcLim;
int fHorz;
{
/* return fFalse if the user cancel out from the tracking.
	return fTrue if the tracking process is completed and return
	the final position of the point in ppt.
	This routine supports keyboard interface and handle both
	vertical and horizontal line tracking.
*/
	HDC hdc;
	int grpf;
	int fTrack;
	int wLongOp;

/* Note that the point passed in should be in screen coordinate */

	Assert(hwnd);

	if ((hdc = GetDC(NULL)) == NULL)
		return (fFalse);

	ClientToScreen(hwnd, (LPPOINT) &rcLim.ptTopLeft);
	ClientToScreen(hwnd, (LPPOINT) &rcLim.ptBottomRight);

	/* We want to change to the appropriate cursor even if in Long Op */
	GetLongOpState(&wLongOp);
	EndLongOp(fTrue /* fAll */);

	OurSetCursor(hCursor);
	SetCapture(hwnd);
	PtIntoRc(ppt, (struct RC *)&rcLim); /* confine trace line in rect */

	fTrack = FTrackLine(hwnd, hdc, ppt, rcLim, fHorz, 0);

	ReleaseDC(NULL, hdc);
	ReleaseCapture();

	ResetLongOpState(wLongOp);
	return(fTrack);
}


/* %%Function:FTrackLine %%Owner:chic */
FTrackLine(hwnd, hdc, ppt, rcLim, fHorz, grpfDrag)
HWND hwnd; /* where the tracking is going to be in */
HDC hdc;
struct PT *ppt; /* comes in like a lion and out like a lamb - oops, I
	mean in in screen coordinates, and out in client
	coordinates */
/* ...except for preview, which is done all in client
	coords. */
struct RC rcLim;
int fHorz;
int grpfDrag; /* grpf for preview ShowXaYa... */
{
	int grpf;
	struct PT ptMouse, ptT;

	SelectObject(hdc, vhbrGray);
	DrawDragLine(hdc, *ppt, &rcLim, fHorz); /* draw */
	bltbyte(ppt, &ptMouse, sizeof(struct PT));
	while ((grpf = GrpfStillTracking(&ptMouse, fHorz)) & FSTILLTRACKING)
		{
	/* handle keyboard interface */
		if (grpf & FTOP)
			if (fHorz)
				ptMouse.yp = rcLim.ypTop;
			else  /* FLEFT case for vertical line */
				ptMouse.xp = rcLim.xpLeft;
		else  if (grpf & FBOTTOM)
			if (fHorz)
				ptMouse.yp = rcLim.ypBottom;
			else  /* FRIGHT case for vertical line */
				ptMouse.xp = rcLim.xpRight;
		else
			PtIntoRc(&ptMouse, (struct RC *)&rcLim);

		if (grpf & FKEYBOARD)
			{
			ptT = ptMouse;
			SetCursorPos(ptT.xp, ptT.yp);
			}

	/* erase old line, draw new line */
		if ((fHorz && ppt->yp != ptMouse.yp) || (!fHorz && ppt->xp != ptMouse.xp))
			{
			DrawDragLine(hdc, *ppt, &rcLim, fHorz); /* erase */
			DrawDragLine(hdc, ptMouse, &rcLim, fHorz); /* draw */
			*ppt = ptMouse;
			}
		}

	PtIntoRc(&ptMouse, (struct RC *)&rcLim);
	DrawDragLine(hdc, *ppt, (struct RC *)&rcLim, fHorz); /* erase */

	ScreenToClient(hwnd, (LPPOINT)ppt);
	return(!(grpf & FESCAPE));
}


/* %%Function:GrpfStillTracking %%Owner:chic */
GrpfStillTracking(ppt, fHorz)
struct PT   *ppt;     /* in and out in screen coords */
/* client coords for preview */
int fHorz;
{
	MSG msg;
	int wIncr;
	extern struct DBS vdbs;
	int grpfRet = 0;

	/* Since re-entrancy can move the heap sometimes, shake it */

	Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);

	if ( PeekMessage( (LPMSG)&msg, (HWND)NULL, NULL, NULL, fTrue ) )
		{
		switch (msg.message)
			{
		default:
			TranslateMessage( (LPMSG)&msg );
			DispatchMessage( (LPMSG)&msg );
			break;

		case WM_LBUTTONDOWN:
			break;

		case WM_LBUTTONUP:
			*ppt = MAKEPOINT(msg.lParam);
			/* lParam is given to us in client coords */
			if (vlm != lmPreview)
				ClientToScreen(msg.hwnd, (LPPOINT)ppt);
			return (fFalse);

		case WM_MOUSEMOVE:
			/* KLUGE ALERT:  if in preview and selecting/moving objects
				via the keyboard, WM_MOUSEMOVEs are generated by SetCursor
				Pos.  But, if the tab key is held down, the messages meant
				for one object might not be processed here until after a
				different object is selected.  There doesn't seem to be
				an easy way to couple object identifiers with their appro-
				priate messages, so we take the draconian step of filtering
				all mouse move messages while in preview keyboard mode. */
			if (vlm == lmPreview && vpvs.fPrvwTab)
				break;

			/* A Mouse Move, Mouse Down, or Mouse Up is waiting */
			*ppt = MAKEPOINT(msg.lParam);

			/* lParam is given to us in client coords */
			if (vlm != lmPreview)
				ClientToScreen(msg.hwnd, (LPPOINT)ppt);
			break;

		case WM_KEYUP:
			if (msg.wParam == VK_SHIFT || msg.wParam == VK_CONTROL)
				SetOurKeyState();
			goto LKeyboard;
			break;

		case WM_KEYDOWN:
			switch (msg.wParam)
				{
				extern int vfAwfulNoise;
			default:
LMakeNoise:
				Beep();
				vfAwfulNoise = fFalse;
				return (FSTILLTRACKING);
			case VK_SHIFT:
			case VK_CONTROL:
				SetOurKeyState();
				grpfRet = FSTILLTRACKING;
				break;
			case VK_UP:
			case VK_DOWN:
				if (fHorz && GetKeyState(VK_CONTROL) < 0)
					{
					grpfRet = msg.wParam == VK_UP ? FTOP : FBOTTOM;
					break;
					}
				wIncr = (vlm == lmPreview) ? 2 : 8;
				if (msg.wParam == VK_UP)
					wIncr = -wIncr;
				ppt->yp += wIncr * vsci.dypBorder;
				break;
			case VK_LEFT:
			case VK_RIGHT:
				if (!fHorz && GetKeyState(VK_CONTROL) < 0)
					{
					grpfRet = msg.wParam == VK_LEFT ? FTOP : FBOTTOM;
					break;
					}
				wIncr = (vlm == lmPreview) ? 2 : 8;
				if (msg.wParam == VK_LEFT)
					wIncr = -wIncr;
				ppt->xp += wIncr * vsci.dxpBorder;
				break;
			case VK_PRIOR:
			case VK_NEXT:
				if (vlm == lmPreview || fHorz)
					{
					grpfRet = msg.wParam == VK_PRIOR ? FTOP : FBOTTOM;
					break;
					}
				else  
					goto LMakeNoise;
			case VK_HOME:
			case VK_END:
				if (vlm != lmPreview && fHorz)
					goto LMakeNoise;
				grpfRet = msg.wParam == VK_HOME ? FTOP : FBOTTOM;
				break;
			case VK_TAB:
				if (vlm != lmPreview)
					goto LMakeNoise;
				/* if in preview, terminate tracking gunk but
					remember that tab was pressed. */
				PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
				/* Fall Through */
			case VK_RETURN:
				return (!FSTILLTRACKING);
				break;
			case VK_ESCAPE:
				return(FESCAPE);
				break;
				}
LKeyboard:
			grpfRet |= FKEYBOARD;
			break;
			}
		}

	grpfRet |= FSTILLTRACKING;
	return (grpfRet);
}


/* %%Function:DrawDragLine %%Owner:chic */
DrawDragLine(hdc, ptFrom, prc, fHorz)
HDC hdc;
struct PT ptFrom;
struct RC *prc;
int fHorz;
{
/* Draws a horizontal or vertical line (determined by fVert) through
	ptFrom and within prc.
	Uses the PATINVERT raster-op so that successive calls toggle the
	visibility of the line.
*/
	struct PT ptTo;

	ptTo = ptFrom;
	if (fHorz)
		{
		ptFrom.xp = prc->xpLeft;
		ptTo.xp = prc->xpRight;
		}
	else
		{
		ptFrom.yp = prc->ypTop;
		ptTo.yp = prc->ypBottom;
		}
	PatBlt(hdc,ptFrom.xp,ptFrom.yp,ptTo.xp - ptFrom.xp+1,ptTo.yp -
			ptFrom.yp+1,PATINVERT);
}




/* D e s t r o y  P h w n d */
/* Destroy passed window, zero out data structure.  Allows for the case
	in which phwnd is a heap pointer and the DestroyWindow produces
	heap movement.  OK to pass in NULL. */
/* %%Function:DestroyPhwnd %%Owner:chic %%reviewed: 7/10/89 */
DestroyPhwnd( phwnd )
HWND *phwnd;
{
	HWND hwnd = *phwnd;
	*phwnd = NULL;
	if (hwnd != NULL)
		{
		Assert(IsWindow(hwnd));
		DestroyWindow (hwnd);
		}
}



/* Put hwnd as first child of Z ordering */
/* %%Function:CWDisplayAndUpdateControl %%Owner:chic %%reviewed: 7/10/89 */
CWDisplayAndUpdateControl(hwnd)
HWND hwnd;
{
	SetWindowPos(hwnd, NULL, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	ShowWindow(hwnd, SHOW_OPENWINDOW);
	UpdateWindow(hwnd);
}



/* D E L  S T  F I L E  F R O M  M R U */
/* Remove passed st from Work on Cache */
/* %%Function:DelStFileFromMru %%Owner:chic */
DelStFileFromMru(st)
char * st;
{
	int ibstDel;
	char stNorm [ichMaxFile];

	Assert(vhsttbOpen != hNil);

	/* Store MRU list normalized so it's uambiguous */
	FNormalizeStFile(st, stNorm, nfoNormal);

	/* look for and delete existing entry */
	if ((ibstDel = IbstFindSt(vhsttbOpen, stNorm)) != iNil)
		DeleteIFromSttb(vhsttbOpen, ibstDel);

	vfFileCacheDirty = fTrue; /* so menu gets updated */
}


