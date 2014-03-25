/* O P E N . C */

#define NOMINMAX
#define NOREGION
#define NOWH
#define NORASTEROPS
#define NOGDI
#define NOMETAFILE
#define NOBITMAP
#define NOWNDCLASS
#define NOBRUSH
#define NONCMESSAGES
#define NOKEYSTATE
#define NOCLIPBOARD
#define NOHDC
#define NOCREATESTRUCT
#define NOTEXTMETRIC
#define NOGDICAPMASKS
#define NODRAWTEXT
#define NOSYSMETRICS
#define NOCOLOR
#define NOPEN
#define NOFONT
#define NOOPENFILE
#define NOMEMMGR
#define NORESOURCE
#define NOICON
#define NOKANJI
#define NOSOUND
#define NOCOMM

#ifdef DEBUG
#ifdef PCJ
/* #define SHOWFLDS */
/* #define DBGDOCOPEN */
#endif /* PCJ */
#endif /* DEBUG */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "ch.h"
#include "props.h"
#include "format.h"
#include "print.h"
#include "doc.h"
#include "disp.h"
#include "sel.h"
#include "prompt.h"
#include "message.h"
#include "screen.h" /* for vsci */
#include "doslib.h"
#include "file.h"
#include "wininfo.h"
#ifdef INTL /* csl chokes on long bcm names inside quotes */
#define USEBCM
#endif
#include "opuscmd.h"
#include "field.h"
#include "debug.h"
#include "rsb.h"
#define SZCLS
#include "error.h"
#include "rareflag.h"
#include "automcr.h"


/* E X T E R N A L S */

extern struct PREF      vpref;
extern struct SCI       vsci;
extern struct FLI       vfli;
extern struct PRI       vpri;
extern int              docMac;
extern struct DOD       **mpdochdod[];
extern struct MWD       **hmwdCur;
extern struct MWD       **mpmwhmwd[];
extern struct WWD       **mpwwhwwd[];
extern struct FCB       **mpfnhfcb[];
extern struct SEL       selCur;
extern struct MERR      vmerr;
extern struct UAB       vuab;
extern HWND             vhwndDeskTop;
extern struct PRI           vpri;
extern HWND             vhwndStartup;
extern HWND             vhInstance;
extern CHAR             stEmpty[];
extern int              vwwClipboard;
extern int              vmwClipboard;
extern KMP              **vhkmpUser;
extern struct CHP	vchpFetch;
extern int  	    	docGlobalDot;
extern char szClsMwd[];
extern char szClsSplitBar[];
extern char szClsWwPane[];
extern char szClsRSB[];
extern int              vwWinVersion;


/* Set if only font widths are needed, not actually selecting them in */

int wwCreating;         /* truly local to this module, as long as
	WwCreate and WwPaneCreate are in the same module */
int mwCreate;   	/* communication during window creation */


PN PnAlloc();
HWND HwndCreateScrollBar();


/* D O C  O P E N  S T  D O F */
/* NOTE: It is OK to pass a heap pointer for stName */
/* dof values may be found in wordwin.h.  prcMw is the rect for the new
	mwd, if any.  Normally, you should just use NULL, and FCreateMw will
	compute the appropriate rect when necessary. */
/* %%Function:DocOpenStDof %%Owner:peterj */
DocOpenStDof(stName, dof, prcMw)
char * stName;
int dof;
struct RC * prcMw;          /* normally is NULL */
{
	extern struct FCB ** mpfnhfcb [];

	BOOL fErrors = !(dof & dofNoErrors);
	int eidErr = eidNil;
	int fn, doc = docNil;
	char st[ichMaxFile + 2];
	struct SELS sels;
	BOOL fSearched=fFalse;
	int fose = foseNoCreate;
	int pdc;
	CP cpMac;
	unsigned rgw [2 + (sizeof (long)/sizeof (unsigned))];
	CHAR szAuthor [cchMaxSz];

	StartLongOp();
	sels.ca.doc = docNil;

#ifdef DBGDOCOPEN
	CommSzSt (SzShared("DocOpenStDof: "), stName);
#endif /* DBGDOCOPEN */

	if (vmerr.fMemFail)
		{
		/* Don't even bother trying in a low memory situation! */
		eidErr = eidNoMemory;
		goto LReturn;
		}

	if (dof & dofNormalized)
		{
		Assert(FStFileIsNormal(stName));
		CopySt(stName, st);
		}
	else  if (!FNormalizeStFile(stName, st, nfoNormal))
		{
		if (dof & (dofSearchDot|dofCmdNewOpen))
			{ /* try to find file by other means */
			CopySt (stName, st);
			goto LFileNotFound;
			}
		/* doc = docNil; */
		ReportSz("This shouldn't happen!");
		eidErr = eidBadFile;
		goto LReturn;
		}

LHaveSt:
	if (!(dof & dofNoMemory) && (doc = DocFromSt(st)) != docNil)
		{
		fn = PdodDoc(doc)->fn;
		if (dof & dofReadOnly && fn != fnNil)
			PfcbFn (fn)->fReadOnly = fTrue;
		if ((dof & dofNativeOnly) && fn != fnNil &&
				!PfcbFn(fn)->fHasFib)
			{
			doc = docNil;
			eidErr = eidCantOpen;
			goto LReturn;
			}
		}
	else
		{
		/* for dofCmdNewOpen inhibit cant open errors
			since we will get them below */
		if ((fn = FnOpenSt(st, (fErrors && !(dof & dofCmdNewOpen) ?
				fOstReportErr:0)
				| (dof&dofReadOnly?fOstReadOnly:0),
				ofcDoc, &fose)) == fnNil)
			{
LFileNotFound:
#ifdef DBGDOCOPEN
			CommSz (SzShared("DocOpen..: File not found\n\r"));
#endif /* DBGDOCOPEN */
			/* remove not found name from mru list if on it */
			DelStFileFromMru(st);

			if (fose <= foseBadFile)
				/* Bad file or too many files open, 
					don't look further */
				{
				}

			else  if (dof & dofCmdNewOpen)
				{
				doc = DocDoCmdNewOpen(st);
				goto LReturn;
				}

			else  if (dof & dofSearchDot && !fSearched)
				{ /* search other directories for dot */
				CHAR stFile [ichMaxFile];

				fSearched = fTrue;
				Assert (!fErrors && !(dof&dofReadOnly));

				if (FFindFileSpec(st, stFile, grpfpiDot, nfoDot))
					{
					CopySt(stFile, st);
					goto LHaveSt;
					}

				/* cannot find file */
				}

			/* doc = docNil; */
			eidErr = eidCantOpen;
			goto LReturn;
			}

		if ((dof & dofNativeOnly) && !PfcbFn (fn)->fHasFib)
			/* file is not opus native format as requested */
			{
			eidErr = eidCantOpen; /* never used */
			goto LReturn;
			}

		if ((doc = DocCreateFn(fn, fErrors, NULL, NULL, &sels))
				== docNil)
			{
			/* doc = docNil; */
			eidErr = eidCantOpen;
			goto LReturn;
			}
		else  if (doc == docCancel)
			/*  the user was given the option of translating
				a foreign file and opted to cancel, not an error
				but no return doc! */
			{
			/* eidErr = eidNil; */
			doc = docNil;
			goto LReturn;
			}

		CopySt((*mpfnhfcb[fn])->stFile, st);
		}

	if (!(dof & dofNoWindow))
		{
		BOOL fNoPrevWw = WwDisp(doc,wwNil, fFalse) == wwNil;

		if (!FCreateMw(doc, wkDoc, &sels, prcMw))
			{
			DisposeDoc(doc);
			doc = docNil;
			eidErr = eidSessionComplex;
			goto LReturn;
			}

		/* close any non-dirty Untitled document that might be open */
		CheckCloseUntitled();

		AddStFileToMru(st);

		/* check if there are DDEHOT links that should be started */
		if (fNoPrevWw)
			CheckQueryAndStartHotLinks (doc);

		/* BLOCK */
			{
			cpMac = CpMacDoc (doc);
			/*  display prompt:  shortfile.ext: 123456 Ch. */
			rgw [0] = doc;
			bltb (&cpMac, &rgw[1], sizeof (CP));
			Assert (sizeof (CP) == sizeof (long));
			if ((fn = PdodDoc(doc)->fn) != fnNil &&
					PfcbFn(fn)->fReadOnly)
				{
				/* mst = mstNumCharsRO; */
				pdc = pdcmPmt | pdcmCreate | pdcmSL | pdcmImmed |
						pdcmTimeout | pdcmRestOnInput;
				}
			else  if (PdodDoc(doc)->fLockForEdit)
				{
				/* mst = mstNumCharsLFA; */
				pdc = pdcmPmt | pdcmCreate | pdcmSL | pdcmImmed |
						pdcmTimeout | pdcmRestOnInput;
				rgw [3] = szAuthor;
				Assert(PdodDoc(doc)->hsttbAssoc);
				GetSzFromSttb(PdodDoc(doc)->hsttbAssoc, 
						ibstAssocAuthor, szAuthor);
				}
			else
				{
				/* mst = mstNumChars; */
				pdc = pdcAdvise;
				}

			/* SetPromptRgwMst (mst, rgw, pdc) */
			SetPromptRgwMst ( ((fn = PdodDoc(doc)->fn) != fnNil && 
					PfcbFn(fn)->fReadOnly) ? mstNumCharsRO :
					((PdodDoc(doc)->fLockForEdit) ? 
					mstNumCharsLFA : mstNumChars ),  rgw, pdc);
			}
		}

LReturn:
	if (fErrors && eidErr != eidNil)
		{
		if (vmerr.fMemFail)
			ErrorNoMemory(eidNoMemOperation);
		else
			ErrorEid (eidErr, " DocOpenStDof");
		}
	EndLongOp(fFalse /* fAll */);
	return doc;
}


/*
*  Create a new Mac window for the document, doc.
*  selCur.doc contains the new document.  The window
*  title is st. *pcaPrevSel (if its doc != docNil) is loaded into the
*  "Go back" cache.
*  WARNING:  If this routine fails the passed in doc may have been Disposed!
*/
/* %%Function:FCreateMw %%Owner:chic */
FCreateMw(doc, wk, psels, prcMw)
int doc;
int wk;
struct SELS *psels;
struct RC * prcMw;    /* normally NULL; contains RC for new mw; used by CBT */
{
	extern struct RC vrcUnZoom;

	struct RC rcTopPane,rcTopScrlBar,rcSplitBox,
	rcSplitBar, rcBottomPane, rcBottomScrlBar;
	HWND hwndMw = NULL;
	int mw = mwNil;
	struct RC rc;
	struct MWD ** hmwd, * pmwd;
	LONG ws;  /* window style */
	HWND hwndMwOld = hmwdCur ? (*hmwdCur)->hwnd : NULL;
	BOOL fVert, fHorz;
	BOOL fEndStartup = fFalse;
	int wwActive = wwNil;
	HWND hwndT;
	HDC hdcMw;

	Scribble(14, 'A'); /* for startup timing */
	StartLongOp ();
	Assert(vhwndDeskTop != NULL);
	Assert(PdodDoc(doc)->fMother);
	if (vmerr.fMwFull)
		goto FailExit;

	Assert(!(wk & wkSDoc));
	mw = IAllocCls(clsMWD, cwMWD);
	if (vmerr.fMemFail)
		goto FailExit;

	hmwd = mpmwhmwd[mwCreate = mw]; /* funny communication with MwdWndProc*/
	Assert(hmwd != hNil);
	pmwd = *hmwd;
	SetWords(pmwd, 0, cwMWD);

	/* this is wwNil for now so too-early callers to WwFromHmwd don't barf */
	pmwd->wwActive = wwNil;
	pmwd->doc = doc;

	if (wk == wkClipboard)  /* special MWD for CLIPBRD.EXE display */
		{
		int ww;

		pmwd->fHorzScrollBar = pmwd->fVertScrollBar = fTrue;
		/* just give a dummy rect to WwCreate. Size messages will
			make it the right size */
		SetWords(&rcTopPane, 0, sizeof (struct RC) / sizeof (int));
		if ((ww = WwCreate(doc, wwNil, mw, rcTopPane, wk)) == wwNil)
			/* Ran out of memory trying to create window */
			goto FailExit;
		vwwClipboard = ww;
		vmwClipboard = mw;
		mwCreate = mwNil;
		goto LExit;
		}
	else
		{
		vrf.fMwCreate = fTrue; /* SetActiveMw looks at this flag */
		pmwd->fHorzScrollBar = vpref.fHorzScrollBar;
		pmwd->fVertScrollBar = vpref.fVertScrollBar;
		}

	SetUndoNil();

	if (prcMw == NULL)            /* use default window positioning? */
		GetMwPrc(mw, &rc);
	else
		rc = *prcMw;

	ws = vpref.fZoomMwd ? CHILDWW_STYLE & ~WS_SIZEBOX : CHILDWW_STYLE;

	if (vpref.fZoomMwd && prcMw == NULL)
		AdjustWindowRect( (LPRECT)&rc, ws, fFalse );

	if ((hwndMw = HwndCreateWindowRc( szClsMwd, ws, rc,
			vhwndDeskTop )) == NULL)
		goto FailExit;

/* place window on top of Z order */
	SetWindowPos(hwndMw, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);

/* set up MW system menu */
/*    Warning!!!  This makes major assumptions about the child
		system menu that Windows is providing us.  They give us no way
		to find out what is already on a menu, so we have to guess.
		If they ever change it from what it is today when I wrote this
		(3/23/87) this code probably won't work -- rosiep
*/

		{
		extern int vgrfMenuKeysAreDirty;
		HMENU hMwMenu = GetSystemMenu(hwndMw, fFalse); /*HMenuFromMw(mw);*/
		if (hMwMenu == NULL)
			goto FailExit;

		if (!ChangeMenu(hMwMenu, SC_RESTORE, 
				(LPSTR)SzSharedKey("&Restore",Restore), bcmRestoreWnd, MF_CHANGE) ||
				!ChangeMenu(hMwMenu, SC_MOVE, 
				(LPSTR)SzSharedKey("&Move",Move), bcmMoveWnd, MF_CHANGE) ||
				!ChangeMenu(hMwMenu, SC_SIZE, 
				(LPSTR)SzSharedKey("&Size",Size), bcmSizeWnd, MF_CHANGE) ||
				!ChangeMenu(hMwMenu, SC_MINIMIZE, 
				(LPSTR)NULL, -1, MF_DELETE) ||
				!ChangeMenu(hMwMenu, SC_MAXIMIZE, 
				(LPSTR)SzSharedKey("Ma&ximize",Maximize), bcmZoomWnd, MF_CHANGE) ||
				!ChangeMenu(hMwMenu, SC_CLOSE, 
				(LPSTR)SzSharedKey("&Close",CloseMenu), bcmCloseWnd, MF_CHANGE) ||
				!ChangeMenu(hMwMenu, 0, (LPSTR)NULL, -1, MF_APPEND | MF_SEPARATOR) ||
				!ChangeMenu(hMwMenu, 0, 
				(LPSTR)SzSharedKey("Spli&t",Split), bcmSplit, MF_APPEND | MF_STRING))
			{
			goto FailExit;
			}

		/* change Window's separators' bcms to negative values */
		{
		int w;
		while (ChangeMenu(hMwMenu, 0, (LPSTR)NULL, -1, MF_CHANGE | MF_SEPARATOR));
		if ((w=GetMenuState(hMwMenu, 0, MF_BYCOMMAND)) != 0 && w != -1)
				goto FailExit;
		}

		/* REVIEW win3 (bz) should be in our qwindows.h */
#define SC_TASKLIST 0xF130

		/* replace doc sys menu switchto wit next window, as win3 does
		   for its mdi  bz
		*/
		if (vwWinVersion >= 0x0300)
			if (!ChangeMenu(hMwMenu, SC_TASKLIST, 
					(LPSTR)SzSharedKey("&Next Window",NextWindow), bcmNextMwd, MF_CHANGE))
				{
				goto FailExit;
				}

		vgrfMenuKeysAreDirty = 0xffff;
		}


/* Compute rectangles where windows should go */

	GetDocAreaPrc( *hmwd, &rc );
	GetDocAreaChildRcs( hmwd, rc, wk,
			0 /*ypSplit*/,
			&rcTopPane, &rcTopScrlBar, &rcSplitBox,
			&rcSplitBar, &rcBottomPane, &rcBottomScrlBar );

	mwCreate = mwNil;  /* Tells MwdSize that we're not creating */

/* Create pane */
	Scribble(14, 'B'); /* for startup timing */
	if ((wwActive = WwCreate(doc, wwNil, mw, rcTopPane, wk)) == wwNil)
		/* Ran out of memory trying to create window */
		goto FailExit;

/* Create vertical scroll bar, split box and size box (if no horz scrl bar)*/

	/* Don't want to actually change vpref if scrl bar creation fails */
	fVert = vpref.fVertScrollBar;
	fHorz = vpref.fHorzScrollBar;

	Scribble(14, 'C'); /* for startup timing */
	if (!FCreateScrollBars( &fVert, &fHorz, wwActive, *hmwd, rc, rcTopScrlBar,
			rcSplitBox, rcBottomScrlBar, fFalse /* fSplit */))
		{
		goto FailExit;
		}

	Scribble(14, 'D'); /* for startup timing */
		/* BLOCK */
		/* This MUST be done before window is made visible!! */
		{
	/* Don't want to record changing to the new window! */
		extern BOOL vfRecording;
		BOOL fRecordingSav ;
		InhibitRecorder(&fRecordingSav, fTrue);
		NewCurWw(wwActive, fFalse /*fDoNotSelect*/);
		vrf.fMwCreate = fFalse;
		InhibitRecorder(&fRecordingSav, fFalse);
		}

	Scribble(14, 'E'); /* for startup timing */
	if (vpref.fPageView && wk == wkDoc)
		{
		CmdPageViewOn(wwActive, doc);
		/* if creating a pageview pane, force all of this doc's pageview
			panes to match this one. */
		SyncPageViewGrpfvisi(doc, PwwdWw(wwActive)->grpfvisi, PwwdWw(wwActive)->fDisplayAsPrint);
		}

	Assert (selCur.doc == doc); /* should have happened somewhere in there */
	Assert (hmwdCur == hmwd);

#ifdef DISABLE_PJ /* disabled to avoid extra swapping */
/* following line forces time-consuming actions to occur
	before we have shown the user anything, to improve apparent painting speed */
	FormatLine(wwActive, doc, cp0);
#endif /* DISABLE_PJ */

/* BEGIN VISUAL DISPLAY OF WINDOW */
/* desired effect is top to bottom and fast */

/* remove startup window and display ribbon (if initializing) */
	if (vhwndStartup != NULL)
		{
		fEndStartup = fTrue;
		EndStartup1();
		}

/* set app title */
	UpdateTitlesForDoc(doc);

	Scribble(14, 'F'); /* for startup timing */

/* show mwd window */
	ShowWindow(hwndMw, SHOW_OPENWINDOW);

	if (vpref.fZoomMwd && hwndMwOld != NULL)
		{
		/* done behind the top window - unzoom the old window */
		UnZoomWnd( hwndMwOld, &vrcUnZoom );
		SetWords(&vrcUnZoom, 0, CwFromCch(sizeof(struct RC)));
		/* so that CmdRestoreWnd will calculate the unzoom rc */
		}

/* paint the new window background */
	GetClientRect(hwndMw, (LPRECT)&rc);
	hdcMw = GetDC(hwndMw);
	if (hdcMw != NULL)
		{
		SelectObject(hdcMw, vsci.hbrBkgrnd);
		PatBltRc(hdcMw, &rc, vsci.ropErase);
		ReleaseDC(hwndMw, hdcMw);
		}

/* display the ruler */
	if ((*hmwd)->hwndRuler != NULL)
		DisplayRuler ();

/* display scrollbars/etc */
	UpdateControlsMw(mw);

/* display the text */
		{
		extern int vywEndmarkLim;

		vywEndmarkLim = 0;	  /* inform endmark draw that entire ww is blanked */
		ShowWindow(PwwdWw(wwActive)->hwnd, SHOW_OPENWINDOW);
		UpdateWindow( PwwdWw(wwActive)->hwnd );
		vywEndmarkLim = 0x7FFF;
		}

/* display the status bar */
	if (fEndStartup)
		EndStartup2();

	Scribble(14, ' '); /* for startup timing */
/* END OF VISUAL ACTIVITY */

/* load up "go back" cache with previous selection, e.g. from FIB */
	if (psels != NULL && psels->doc != docNil)
		PushInsSels(psels);

	vmerr.fHadDispAlert = fFalse; /* reset every time a new window is opened */

LExit:
	vrf.fMwCreate = fFalse;
	EndLongOp (fFalse);
	return fTrue;

FailExit:
	SetErrorMat(matMem);
	mwCreate = mwNil;
	vrf.fMwCreate = fFalse;
	Assert(mpdochdod[doc] != hNil);
	/* keep CloseWw from Disposing doc! */
	PdodDoc(doc)->crefLock++;
	if (wwActive != wwNil)
		CloseWw(wwActive); /* calls DisposeMw */
	else
		{
		if (hwndMw)
			DestroyWindow(hwndMw);
		FreeCls(clsMWD, mw); /* FreeCls simply return if mw == mwNil */
		}
	Assert(mpdochdod[doc] != hNil);
	PdodDoc(doc)->crefLock--;
	EndLongOp (fFalse);
	return fFalse;
}


/* %%Function:FCreateScrollBars %%Owner:chic */
FCreateScrollBars( pfVert, pfHorz, wwTop, pmwd, rcDocArea,
rcTopScrlBar, rcSplitBox, rcBottomScrlBar, fSplit)
BOOL *pfHorz, *pfVert;
int wwTop;
struct MWD *pmwd;
struct RC rcDocArea, rcTopScrlBar, rcSplitBox, rcBottomScrlBar;
BOOL fSplit;
{
	struct RC rcHorzScrollBar, rcSizeBox;
	int wwBottom;


	FreezeHp();
/* Create vertical scroll bar, split box and size box (if no horz scrl bar) */

	if (*pfVert)
		{
		if ((pmwd->hwndSplitBox = HwndCreateWindowRc(szClsRSB,
				WS_CHILD | WS_CLIPSIBLINGS | maskWndSplitBox,
				rcSplitBox, pmwd->hwnd )) == NULL)
			goto LFailVert3;

		if (!FCreateVScrlWw(wwTop, rcTopScrlBar))
			goto LFailVert2;

		if (fSplit && !FCreateVScrlWw(WwOther(wwTop), rcBottomScrlBar))
			goto LFailVert1;

		pmwd->fVertScrollBar = fTrue;
		}

/* Create horizontal scroll bar and size box (if in horz scroll bar row) */

	if (*pfHorz)
		{
		GetHorzBarRcs( pmwd, rcDocArea, &rcHorzScrollBar, &rcSizeBox );

		/* Create the Horizontal scroll bar */
		if ((pmwd->hwndHScroll = HwndCreateScrollBar( fFalse /* fVert */,
				fFalse /* fVisible */, rcHorzScrollBar, pmwd->hwnd )) == NULL)
			{
LFailHorz:
			*pfHorz = fFalse;
			MeltHp();
			return (fFalse);
			}
		RSBSetSpsLim( pmwd->hwndHScroll, vsci.xpRightMax );
		pmwd->fHorzScrollBar = fTrue;
		}

	if (pmwd->fHorzScrollBar && pmwd->fVertScrollBar)
		{
		if ((pmwd->hwndSizeBox = HwndCreateWindowRc( szClsRSB,
				WS_CHILD | SBS_SIZEBOX | WS_CLIPSIBLINGS,
				rcSizeBox, pmwd->hwnd )) == NULL)
			{ /* failed; destroy horz scroll so AdjustMwdArea doesn't
					do weird things */
			DestroyPhwnd(&(pmwd->hwndHScroll));
			goto LFailHorz;
			}
		}
	MeltHp();
	return (fTrue);


LFailVert1:
	DestroyVScrlWw(wwTop);
LFailVert2:
	DestroyPhwnd(&(pmwd->hwndSplitBox));
LFailVert3:
	*pfVert = fFalse;
	MeltHp();
	return (fFalse);
}


/* %%Function:FCreateVScrlWw %%Owner:chic */
FCreateVScrlWw(ww, rcVScrlBar)
int ww;
struct RC rcVScrlBar;
{
	struct WWD *pwwd = PwwdWw(ww);

	FreezeHp();
	if ((pwwd->hwndVScroll = HwndCreateScrollBar(fTrue /* fVert */, 
			fFalse /* fVisible */, rcVScrlBar, PmwdWw(ww)->hwnd)) != NULL)
		{
		if (pwwd->fPageView)
			{
			if (!FCreatePgvCtls(ww))
				goto LFail;
			}
		MeltHp();
		return fTrue;
		}

LFail:
	DestroyPhwnd(&(pwwd->hwndVScroll));
	MeltHp();
	return fFalse;
}



/* %%Function:HwndCreateScrollBar %%Owner:chic */
HWND HwndCreateScrollBar(fVert, fVisible, rc, hwndParent)
BOOL fVert;
BOOL fVisible;
struct RC rc;
HWND hwndParent;
{
	return HwndCreateWindowRc( szClsRSB,
			WS_CHILD|WS_CLIPSIBLINGS| (fVert ? SBS_VERT : SBS_HORZ) | (fVisible ? WS_VISIBLE : 0),
			rc, hwndParent);
}


/* Put hwnd as first child of Z ordering */
/* %%Function:DisplayAndUpdateControl %%Owner:chic */
DisplayAndUpdateControl(hwnd)
HWND hwnd;
{
	SetWindowPos(hwnd, NULL, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	ShowWindow(hwnd, SHOW_OPENWINDOW);
	UpdateWindow(hwnd);
}


/* U p d a t e  C o n t r o l s  M w */
/* paint invalid portions of scroll bars, split bars/boxes, size boxes
	associated with mw and its ww children */

/* %%Function:UpdateControlsMw %%Owner:chic */
UpdateControlsMw(mw)
int mw;
{
	struct MWD *pmwd = PmwdMw(mw);
	int wwUpper = pmwd->wwUpper;
	int wwLower = pmwd->wwLower;
	HWND hwndT;

	FreezeHp();
	if (hwndT = pmwd->hwndHScroll)
		{
		DisplayAndUpdateControl(hwndT);
		Assert(pmwd->wwActive != wwNil);
		Assert(pmwd->wwActive == wwUpper || pmwd->wwActive == wwLower);
		SyncSbHor(pmwd->wwActive);
		}
	if (hwndT = pmwd->hwndSplitBox)
		DisplayAndUpdateControl(hwndT);
	if (hwndT = pmwd->hwndSizeBox)
		DisplayAndUpdateControl(hwndT);
	if (hwndT = pmwd->hwndSplitBar)
		DisplayAndUpdateControl(hwndT);
	MeltHp();

	if (hwndT = PwwdWw(wwUpper)->hwndVScroll)
		{
		PwwdWw(wwUpper)->dqElevator = -1;
		SetElevWw(wwUpper);
		UpdateVScrlWw(wwUpper);
		}
	if (wwLower != wwNil && (hwndT = PwwdWw(wwLower)->hwndVScroll))
		{
		PwwdWw(wwLower)->dqElevator = -1;
		SetElevWw(wwLower);
		UpdateVScrlWw(wwLower);
		}
}


/* %%Function:UpdateVScrlWw %%Owner:chic */
UpdateVScrlWw(ww)
int ww;
{
	struct WWD *pwwd = PwwdWw(ww);

	if (pwwd->fPageView)
		ShowPgvCtls(ww, fFalse /*fMove*/); /* will also update hwndVScroll */
	else 
		DisplayAndUpdateControl(pwwd->hwndVScroll);
}


/* W W  C R E A T E  */
/* WwCreate creates a pane window to display doc.
	The title of the window
	is set to be stName. If space is not available to build the requested
	window, WwCreate returns wwNil else it returns a valid ww. */
/* %%Function:WwCreate %%Owner:chic */
WwCreate(doc, wwOther, mw, rcWw, wk)
int doc;
int wwOther;           /* Other ww (pane), or wwNil if first pane
		in Mac-in-the-Box child window */
int mw;
struct RC rcWw;
int wk;
{
	int ww = wwNil;
	struct MWD **hmwd = mpmwhmwd[mw];
	struct WWD *pwwd;
	struct DOD *pdod;
	struct RC rcePage, rcwDisp;
	struct SELS sels;
	int xwMin = 0;
	int dxwStyWnd = 0;
	int dxsGalleyPage;
	HWND hwndWw = NULL;
	KMP ** hkmpT;
	struct WWD **hwwdOther = wwOther == wwNil ? hNil : HwwdWw(wwOther);

	Assert( doc != docNil );    /* Case not supported for OPUS */

	if (wwOther != wwNil && !(wk & wkSDoc))
		{
		sels = *((struct SELS *)&selCur);
		}
	else
		{
		SetWords(&sels, 0, cwSELS);
		sels.doc = doc;
		sels.fForward = fTrue;
		sels.fIns = fTrue;
		}

	if (PdodDoc(doc)->fShort)
		{
		if ((hkmpT = HkmpCreate(0, kmfPlain)) == hNil)
			return wwNil;
		}
	else
		hkmpT = hNil;

	if (wk != wkClipboard && wk != wkMacro)
		{
		if (wwOther != wwNil)
			dxwStyWnd = (*hwwdOther)->xwSelBar;
		else
			{
			dxwStyWnd = vpref.dxaStyWnd != 0 ?
					NMultDiv( vpref.dxaStyWnd, vfli.dxsInch, czaInch ):
					0;
			if (dxwStyWnd)
				AdjustStyNameDxw(hmwd, &dxwStyWnd);
			}

		if (wwOther != wwNil && (wk & ~(wkSDoc+wkOutline)) == ((*hwwdOther)->wk & ~(wkSDoc+wkOutline)))
			xwMin = (*hwwdOther)->xwMin;
		else  if (wk != wkPage)
			xwMin += dxwStyWnd;
		}
	SetRect(&rcwDisp,
			xwMin,
			(wk == wkPage ? 0 : vsci.dypMinWwInit),
			rcWw.xpRight,
			rcWw.ypBottom);


	FreezeHp();
	pdod = PdodMother(doc);
	dxsGalleyPage = NMultDiv(pdod->dop.xaPage - pdod->dop.dxaLeft,
			vfli.dxsInch, dxaInch);
	if (wwOther != wwNil)
		{
		rcePage =(*hwwdOther)->rcePage;
		if ((*hwwdOther)->fPageView && wk != wkPage)
			{
			/* upper pane in page view but not lower pane, make
			lower pane in galley coordinate */
			/* at this point, ww is not allocate yet, but we can use wwOther for these calculation */
			rcePage.xeLeft -= XeLeftPage(wwOther) - dxwSelBarSci;
			rcePage.yeTop = 0;
			rcePage.xeRight = rcePage.xeLeft + dxsGalleyPage;
			rcePage.yeBottom = ypMaxAll;
			}
		}
	else  if (wk != wkPage)
		SetRect(&rcePage,
				dxwSelBarSci,
				0,
				dxwSelBarSci + dxsGalleyPage,
				ypMaxAll);
	else
		{
		SetRect(&rcePage,
				0,
				0,
				NMultDiv(pdod->dop.xaPage, vfli.dxsInch, dxaInch),
				DysFromDya(pdod->dop.yaPage));
				MoveRc(&rcePage, /* XeLeftPage, YeTopPage */
		-NMultDiv(pdod->dop.dxaLeft, vfli.dxsInch, dxaInch) + dxwSelBarSci,
				-DysFromDya(abs(pdod->dop.dyaTop)) + dypMinWwInitSci);
		}
	MeltHp();

/* fix up wk for page view :
	The trick we play here is that let WwCreate set up the rcwDisp and rcePage
	for page view but pass wkDoc to WwAllocCommon to create the pane in galley
	mode.  Then we correct wk, xhScroll, ipgd to really make it page view.
	This is so that we can still share WwAllocCommon with Mac. 
*/
	if ((ww = WwAllocCommon(doc, mw, &sels, &rcwDisp, &rcePage,
			wk == wkPage ? 0 : 
			max(0, ((rcWw.ypBottom - rcWw.ypTop) / vsci.dysMinAveLine)),
			wk == wkPage ? wkDoc : wk))  == wwNil)
		{
		if (hkmpT != hNil)
			FreeH(hkmpT);
		return (wwNil);
		}
	pwwd = PwwdWw(ww);
	pwwd->fDirty = fTrue;
	pwwd->grpfvisi = vpref.grpfvisi;
	pwwd->fDisplayAsPrint = vpref.fDisplayAsPrint;
	pwwd->grpfvisi.flm = FlmDisplayFromFlags(vpref.fDisplayAsPrint, vpref.fPageView, vpref.fDraftView);
	pwwd->xwSelBar = wk == wkPage ? 0 : dxwStyWnd;
	pwwd->dxwStyWnd = dxwStyWnd;
	pwwd->fLower = wwOther == wwNil ? fFalse : fTrue;

/* set up mwd */
	if (wwOther != wwNil)
		{ /* we are splitting and create a lower pane */
		PwwdWw(ww)->fHadRuler = (*hwwdOther)->fHadRuler;
		(*hmwd)->fSplit = fTrue;
		(*hmwd)->wwLower = ww;
		}
	else
		{
		(*hmwd)->wwActive = (*hmwd)->wwUpper = ww;
		}

/* Set up the window's keymap. */
		{
		int docDot;
		KMP ** hkmp;

		if (wwOther != wwNil && (*hwwdOther)->fOutline && !PdodDoc(doc)->fShort)
			{
			/* inherit outline mode */
			hkmp = (*hwwdOther)->hkmpCur;
			}
		else  if ((docDot = DocDotMother(doc)) != docNil)
			hkmp = PdodDoc(docDot)->hkmpUser;
		else
			hkmp = vhkmpUser;

		/* Now, if we're a short dod, link in an empty keymap because
			short dod's in windows usually have special commands... */
		if (PdodDoc(doc)->fShort)
			{
			(*hkmpT)->hkmpNext = hkmp;
			hkmp = hkmpT;
			}

		AssertH(hkmp);
		PwwdWw(ww)->hkmpCur = hkmp;
		}

	if (wk == wkClipboard)
		goto LExit;	/* Window belongs to CLIPBRD.EXE; don't create one */

/* pass hwwd via global to WwPaneCreate, so subsequent messages from within
	CreateWindow find their way to the correct wwd */

	wwCreating = ww;
	if ((hwndWw = HwndCreateWindowRc(szClsWwPane,  /* HEAP MVMT */
			WWPANE_STYLE,
			rcWw,
			(*hmwd)->hwnd )) == NULL)
		{
LFail:		
		SetErrorMat(matMem);
		RemoveWwDisp(doc, ww);
		if (hwndWw != NULL)
			DestroyWindow(hwndWw);
		wwCreating = wwNil;
		FreeDrs(HwwdWw(ww), 0);
		FreeCls(clsWWD,ww);
		if (hkmpT != hNil)
			FreeH(hkmpT);
		if (wwOther != wwNil)
			{
			(*hmwd)->fSplit = fFalse;
			(*hmwd)->wwLower = wwNil;
			}
		return wwNil;
		}

	FreezeHp();
	pwwd = PwwdWw(ww);
	if ((pwwd->hdc = GetDC( pwwd->hwnd )) == NULL ||
			!FSetDcAttribs( pwwd->hdc, dccDoc ))
		{
		MeltHp();
		goto LFail;
		}
	MeltHp();

/* Create icon bar if appropriate */
/* Note: we'll never have to create outline iconbar here because a new
	pane can never come up already in outline mode AND needing an icon
	bar.  If this is the second of two panes, the other pane will already
	have the icon bar. */
	if (wk == wkHdr)
		{
		if (!FCreateHdrIconBar(mpwwhwwd[ww]))
			goto LFail;
		rcWw.ypTop += vsci.dypIconBar - vsci.dypBorder;
		}

/* Create the ruler, OK if fails (may fail due to size) */
	if (wk != wkMacro && wk != wkDebug && wk != wkOutline)
		{
		if (vpref.fRuler && wwOther == wwNil)
			if (FCreateRuler (ww, fFalse))
				rcWw.ypTop += vsci.dypRuler - vsci.dypBorder;
		}

	MoveWindowRc(PwwdWw(ww)->hwnd, &rcWw, fTrue);
/* control by caller 
		ShowWindow(PwwdWw(ww)->hwnd, SHOW_OPENWINDOW);
*/
	wwCreating = wwNil; /* reset */
LExit:
	return (ww);
}



/* %%Function:WwPaneCreate %%Owner:chic */
WwPaneCreate( hwnd )
HWND hwnd;
	{       /* Must store ww so that WwFromHwnd call in following WM_SIZE
	message succeeds. */

	Assert( wwCreating != wwNil );
/* set ww into extra word in handle */
	SetWindowWord(hwnd, IDWWW, wwCreating);
	PwwdWw(wwCreating)->hwnd = hwnd; /* still setting this because a lot of
		debug checks on WwFromHwnd(PwwdWw(wwCreating)->hwnd)
		are used. */
}


/* A P P E N D  W W  - appends ww onto chain of ww's for this doc.
*  The ww is appended to the end of the chain, so that newer ww's have
*  increasing instance numbers in chain.
*/
/* %%Function:AppendWw %%Owner:chic */
AppendWw(doc, ww)
int doc;
int ww;
{
	struct DOD *pdod = PdodMother(doc);
	struct WWD *pwwdLast;

	if (pdod->wwDisp != wwNil)
		{
		WwLastInDoc(doc, &pwwdLast);
		Assert(pwwdLast->wwDisp == wwNil);
		pwwdLast->wwDisp = ww;
		}
	else
		pdod->wwDisp = ww;

	PwwdWw(ww)->wwDisp = wwNil;
}


/* W W  L A S T  I N  D O C */
/* returns last ww in ww chain for doc.  *ppwwd is pwwd to last wwd.
*/
/* %%Function:WwLastInDoc %%Owner:chic */
WwLastInDoc(doc, ppwwd)
int doc;
struct WWD **ppwwd;
{
	int ww = PdodMother(doc)->wwDisp;
	struct WWD *pwwd = PwwdWw(ww);

	Assert( ww != wwNil ); /* assert that there is at least 1 ww in doc */
	for (; pwwd->wwDisp != wwNil; pwwd = PwwdWw(ww = pwwd->wwDisp))
		;
	*ppwwd = pwwd;
	return ww;
}



/* Q U E R Y  A N D  S T A R T  H O T  L I N K S */
/*  Check doc and see if it has any ddeHot links.  If it does ask the
	user if they should be started and if they should, refresh all ddehot
	links in the doc.
*/

/* %%Function:CheckQueryAndStartHotLinks  %%Owner:peterj */
CheckQueryAndStartHotLinks (doc)
int doc;

{
	extern struct CA caAdjust;

	int ifld, ifldMac;
	struct DOD *pdod = PdodDoc(doc);
	struct PLCFLD **hplcfld = pdod->hplcfld;
	struct FLD fld;

	if (hplcfld == hNil || !pdod->fMother || pdod->fLockForEdit)
		return;

	for ( ifld = 0, ifldMac = IMacPlc( hplcfld ); ifld < ifldMac; ifld++ )
		{
		GetPlc( hplcfld, ifld, &fld );

		if (fld.ch == chFieldBegin && fld.flt == fltDdeHot)
			{
			QueryAndStartHotLinks(doc);
			break;
			}
		}

	return;
}


int viDocument = 0;
int viTemplate = 0;

/* E L  N E W  F I L E */
/* %%Function:ElNewFile %%Owner:peterj */
EL ElNewFile(stType, fNewDot)
char * stType;
BOOL fNewDot;
{
	int doc = docNil;
	int docDot = docNil;
	int dk = fNewDot ? dkDot : dkDoc;
	int eid;
	BOOL f;
	struct DOD * pdod;
	char ** hst;

/* WARNING: Need to use stType before heap moves!!!! */

	StartLongOp ();

	if ((stType)[0] == 0)
		{          /* New document is not based on a type */
		if ((doc = DocCloneDoc (fNewDot?docNew:docGlobalDot, dk))
				== docNil)
			{
			eid = eidCantOpen;
			goto LNewErrRet;
			}
		PdodDoc(doc)->fFormatted = fNewDot;
		}
	else
		{          /* New document is based on a type */
		CHAR stDot[ichMaxFile];

		/*  find and open the dot the new document is based on */
		if (!FFindFileSpec(stType, stDot, grpfpiDot, nfoDot) &&
				!FNormalizeStFile(stType, stDot, nfoDot))
			{
			eid = eidBadFile;
			goto LNewErrRet;
			}

		if ((docDot = DocOpenStDof(stDot,
				dofNoWindow|dofSearchDot|dofNoErrors, NULL)) == docNil)
			{
			eid = eidCantOpen;
			goto LNewErrRet;
			}

		/*  Clone document type if:
			*    1) creating a new document
			*    2) creating a new document type AND the source type
			*       had been opened before the above DocOpen.  After
			*       the clone, the source type is no longer needed.
			*    3) creating a new document type AND the source is
			*       not a document type.
			*/
		if (!fNewDot || PdodDoc(docDot)->crefLock >= 1 ||
				!PdodDoc (docDot)->fDot || FDocBusy(docDot))
			{
			if ((doc = DocCloneDoc (docDot, dk)) == docNil)
				{
				DisposeDoc (docDot);
				eid = eidNoMemory;
				goto LNewErrRet;
				}
			if (!fNewDot)
				/* link to docDot */
				{
				if (!PdodDoc (docDot)->fDot)
					/* docDot is not really a dot, use its dot */
					/*  This is the case where the user specified
						a doc which is not a dot.  Clone the doc
						and give new doc the dot of the old doc.
					*/
					{
					int docDotDot = PdodDoc (docDot)->docDot;
					Assert (PdodDoc (docDot)->fDoc);
					if (docDotDot != docNil)
						{
						Assert (PdodDoc (docDotDot)->fDot);
						GetDocSt (docDotDot, stDot, gdsoFullPath);
						PdodDoc(docDotDot)->crefLock++;
						}
					else
						stDot [0] = 0;
					DisposeDoc (docDot);
					docDot = docDotDot;
					}
				else  if (docDot != docNil)
					PdodDoc (docDot)->crefLock++;

				PdodDoc (doc)->docDot = docDot;
				}
			}
		else  /* fNewDot && crefLock == 0 */				
			{ /* docDot is not otherwise used, make it the new
					doc Type. */
			doc = docDot;
			PdodDoc (doc)->fn = fnNil; /* disassociate from file */
			Debug (PdodDoc(doc)->dop.nRevision = 0);
			/* get rid of unwanted summary info */
			FreePhsttb (&PdodDoc (doc)->hsttbAssoc);
			}

		/*  associate doc with its dot */
		if (!fNewDot)
			{   /* assumes hsttb not copied w/dot */
			struct STTB **hsttbAssoc = HsttbAssocEnsure (doc);
			if (hsttbAssoc == hNil ||
					!FChangeStInSttb (hsttbAssoc, ibstAssocDot, stDot))
				{
				eid = eidNoMemory;
				goto LNewErrRet;
				}
			}
		}  /* end of based on a type */

	/* set up document management properties */
	ApplyDocMgmtNew (doc, fnNil);

		/*  name the new document */
		{
		struct DOD *pdod = PdodDoc(doc);
		pdod->udt = fNewDot ? udtTemplate : udtDocument;
		pdod->iInstance = fNewDot ? ++viTemplate : ++viDocument;
		}

	/* place document in window */
	if (!FCreateMw(doc, wkDoc, NULL, NULL))
		{
		eid = eidSessionComplex;
		goto LNewErrRet;
		}

	/* close the original untitled */
	CheckCloseUntitled();

	/* check if there are DDEHOT links that should be started */
	CheckQueryAndStartHotLinks (doc);

	/* refresh some fields (only interesting if from template) */
	if (docDot != docNil)
		{
		int docT;
		int cT;
		extern struct CA caAdjust;

		AcquireCaAdjust ();

		/*  header/footer */
		if (FHasFields(docT = PdodDoc (doc)->docHdr))
			if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
				FCalcFields (&caAdjust, frmPrint, fFalse, fTrue);

		/*  footnotes */
		if (FHasFields(docT = PdodDoc (doc)->docFtn))
			if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
				FCalcFields (&caAdjust, frmPrint, fFalse, fTrue);

		/*  annotations */
		if (FHasFields(docT = PdodDoc (doc)->docAtn))
			if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
				FCalcFields (&caAdjust, frmPrint, fFalse, fTrue);

		/*  main doc */
		if (FHasFields(doc))
			if (FSetPcaForCalc (&caAdjust, doc, cp0, CpMacDoc (doc), &cT))
				FCalcFields (&caAdjust, frmPrint, fFalse, fTrue);

		ReleaseCaAdjust ();
		}

	EndLongOp (fFalse /* fAll */);
	return cmdOK;

LNewErrRet:
	EndLongOp (fFalse /* fAll */);
	ErrorEid(eid," New Document");
	DisposeDoc(doc);

	return cmdError;
}


/* C H E C K  C L O S E  U N T I T L E D */
/*  If there is an non-dirty Untitled document, close it. if there is more
	then one window open onto it, leave it alone! */
/* %%Function:CheckCloseUntitled %%Owner:peterj */
CheckCloseUntitled()
{
	int ww;
	int doc;
	BOOL fRecordingSav;

	for (doc = docMinNormal; doc < docMac; doc++)
		if (mpdochdod[doc] != hNil 
				&& !PdodDoc(doc)->fShort
				&& PdodDoc(doc)->udt == udtDocument 
				&& PdodDoc(doc)->iInstance == 0) /* special "untitled" */
			{
			if (!DiDirtyDoc(doc) && (ww = WwDisp(doc, wwNil, fFalse)) != wwNil &&
					WwDisp(doc, ww, fFalse) == wwNil)
				{
			/* Don't record closing these type of windows */
				InhibitRecorder(&fRecordingSav, fTrue);
				SendMessage(PmwdMw(PwwdWw(ww)->mw)->hwnd, WM_CLOSE, 0, 0L);
				InhibitRecorder(&fRecordingSav, fFalse);
				}
			return;
			}
}
