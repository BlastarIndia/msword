#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "debug.h"
#include "error.h"
#include "screen.h"
#include "wininfo.h"
#include "splitter.h"
#include "resource.h"
#include "opuscmd.h"
#include "keys.h"
#include "doc.h"
#include "help.h"
#define RULER
#include "ruler.h"
#include "scc.h"


extern char 		szClsSplitBar[];
extern HANDLE  		vhInstance;
extern HCURSOR 		vhcArrow;
extern HCURSOR 		vhcSplit;
extern BOOL    		vfInLongOperation;
extern int     		wwCur;
extern struct WWD     	**mpwwhwwd[];
extern struct WWD     	**hwwdCur;
extern struct MWD     	**hmwdCur;
extern int		mwCur;
extern struct MWD     	**mpmwhmwd[];
extern struct SCI     	vsci;
extern struct SEL     	selCur;
extern int            	vgrpfKeyBoardState;
extern int              vlm;
extern HBRUSH           vhbrGray;
extern HWND             vhwndPgPrvw;
extern int            	mwMac;
extern BOOL           	vfRecording;
extern int              vdocHdrEdit;
extern int              vfIconBarMode;
extern struct SCC	vsccAbove;
extern int     		vssc;
extern struct MERR      vmerr;
extern BOOL             vfSeeSel;


/* %%Function:CmdSplit %%Owner:rosiep */
CMD CmdSplit(pcmb)
CMB *pcmb;
{
	BOOL fRecordingSav;
	struct PT  ptCursor;

	InhibitRecorder(&fRecordingSav, fTrue);

	Assert(hmwdCur);
	/* Move cursor to split bar or center of doc, depending on
		whether there's a split or not. */

	ptCursor.xp = (*hwwdCur)->xwMac / 2;
	if ((*hmwdCur)->fSplit)
		{
		ptCursor.yp = (*hmwdCur)->ypSplit;
		ClientToScreen( (*hmwdCur)->hwnd, (LPPOINT) &ptCursor );
		}
	else
		{
		ptCursor.yp = (*hwwdCur)->ywMac / 2;
		ClientToScreen( (*hwwdCur)->hwnd, (LPPOINT) &ptCursor );
		}
	SetCursorPos( ptCursor.xp, ptCursor.yp );

	TrackSplitBar(ptCursor, (*hwwdCur)->wk);

	InhibitRecorder(&fRecordingSav, fFalse);

	if (vfRecording)
		RecordSplit((*hmwdCur)->ypSplit);

	return cmdOK;
}





/* %%Function:TrackSplitBar %%Owner:rosiep */
TrackSplitBar(pt, wk)
struct PT pt;    /* screen coords. */
int wk; /* window split type */
{
	struct RC rcLim;

	EnsureCursorVisible();

	/* confine the tracer bar to the document area */
	GetDocAreaPrc(*hmwdCur, &rcLim);

	if (FTrackDragLine((*hmwdCur)->hwnd, &pt, vhcSplit,
			rcLim, fTrue /*fHorz*/))
		{
		FSplitMwd(hmwdCur, pt.yp, wk);
		} /* end of FTrackDragLine */

	EnsureCursorInvisibleMouseless();
}




/* %%Function:FCreateSplit %%Owner:rosiep */
FCreateSplit(doc, yp, wk)
int doc;
int yp;
int wk;
{
	extern CHAR   stEmpty[];
	extern int dypDocAreaOld;
	struct RC rcTopPane,rcTopScrlBar,rcSplitBox,
	rcSplitBar, rcBottomPane, rcBottomScrlBar, rc;
	int wwNew, wwUpper = wwCur;
	struct WWD **hwwdUpper = hwwdCur;
	struct WWD **hwwdNew;
	int hwndVScroll = NULL;
	int fPageView = (*hwwdUpper)->fPageView;
	struct PT ptParentClientOrg;   /* origin of client rect of parent */
	int ypSplit, dlSplit;
	struct PLDR **hpldr;
	struct SPT spt;
	struct PT pt;
	int idr;
	CP cpFirstNew;
	CP cpT;
	int edcDrp;
	struct CA caMom;
	int docMom = DocMother(DocBaseWw(wwUpper));
	struct DRF drfFetch;


/* save the view hidden text bit if creating atn pane */
	if (wk == wkAtn)
		{
		struct WWD *pwwd = *hwwdUpper;
		(*hmwdCur)->fAtnVisiSaved = pwwd->grpfvisi.fSeeHidden;
		if (!(*hmwdCur)->fAtnVisiSaved)
			{ /* avoid trashing wwUpper if not necessary */
			pwwd->grpfvisi.fSeeHidden = fTrue;
			CopyGrpfvisiToScc(wwUpper);
			if (pwwd->fPageView)
				SyncPageViewGrpfvisi(docMom, pwwd->grpfvisi, pwwd->fDisplayAsPrint);
			}
		}

	/* avoid problems with multiple selections */
	if (vssc != sscNil)
		CancelDyadic();

	(*hmwdCur)->ypSplit = yp;

	TurnOffSel(&selCur);

/* compute rectangles of all child windows for new, split configuration */

	GetDocAreaPrc( *hmwdCur, &rc );
	dypDocAreaOld = rc.ypBottom - rc.ypTop;

	GetDocAreaChildRcs( hmwdCur, rc,
			wk, (*hmwdCur)->ypSplit,
			&rcTopPane, &rcTopScrlBar, &rcSplitBox,
			&rcSplitBar, &rcBottomPane, &rcBottomScrlBar );

/* create new windows: bottom scroll bar, split bar, bottom pane */

	if ((*hmwdCur)->fVertScrollBar &&
			((hwndVScroll = HwndCreateScrollBar( fTrue /*fVert*/, fTrue /*fVisible*/, 
			rcBottomScrlBar, (*hmwdCur)->hwnd )) == NULL))
		goto LFail;
	if (((*hmwdCur)->hwndSplitBar = HwndCreateWindowRc( szClsSplitBar,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			rcSplitBar, (*hmwdCur)->hwnd )) == NULL)
		goto LFail;

/* figure out the doc to be in the new window */
	switch (wk)
		{
	case wkDoc:
	case wkMacro:
	case wkPage:
	case wkOutline:
		doc = docMom;
		break;
	case wkFtn:
		doc = PdodDoc(docMom)->docFtn;
		Assert(doc != docNil && PdodDoc(doc)->fFtn);
		break;
	case wkAtn:
		doc = PdodDoc(docMom)->docAtn;
		Assert(doc != docNil && PdodDoc(doc)->fAtn);
		break;
	case wkHdr:
		Assert(doc != docNil);
		break;
#ifdef DEBUG
	default:
		Assert(fFalse);
		return fFalse;
#endif /* DEBUG */
		}
	Assert(doc != docNil);


/* create the new pane */
	if ((wwNew = WwCreate(doc, wwUpper, mwCur, rcBottomPane, wk)) == wwNil)
		{
		DestroyPhwnd(&(*hmwdCur)->hwndSplitBar);

LFail:           /* destroy any windows that have been created */
		(*hmwdCur)->ypSplit = 0;
		if (hwndVScroll)
			DestroyWindow(hwndVScroll);
		return (fFalse);
		}


/* move existing windows up out of the way: split box, top scroll bar, top pane */
	MoveWindowRc( (*hmwdCur)->hwndSplitBox, &rcSplitBox, fTrue );
	Assert (wwUpper == (*hmwdCur)->wwActive);
	if (fPageView)
		ShowPgvCtls(wwUpper, fTrue /*fMove*/); /* takes care of pgv icons and vscrl bar */
	else
		MoveWindowRc( (*hwwdUpper)->hwndVScroll, &rcTopScrlBar, fTrue );
	MovePane( wwUpper, &rcTopPane, mpdPaneOnly );


	(*hmwdCur)->fSplit = fTrue;
	hwwdNew = HwwdWw(wwNew);
	(*hwwdNew)->xhScroll = (*hwwdUpper)->xhScroll;
	if (wk != wkPage)
		{
		if (wk == wkHdr)
			{
			cpFirstNew = cp0;
			Assert((*hwwdNew)->hwndIconBar);
			GetCaHdrMom(&caMom, wwCur);
			SetupIconBarButton(wwNew, docMom, caMom.cpFirst, PdodDoc(doc)->ihdt);
			ShowWindow((*hwwdNew)->hwndIconBar, SHOW_OPENWINDOW);
			UpdateWindow((*hwwdNew)->hwndIconBar);
			}
		else  if (wk == wkFtn || wk == wkAtn)
			{
			Assert(PdrGalley(*hwwdNew)->doc == (wk == wkFtn ? 
					PdodDoc(docMom)->docFtn :
					PdodDoc(docMom)->docAtn));
			edcDrp = (wk == wkAtn ? edcDrpAtn : edcDrpFtn);
			cpFirstNew = fPageView ? cp0 : CpFirstRef(selCur.doc, 
					PdrGalley(*hwwdUpper)->cpFirst,&cpT, edcDrp);
			}
		else  /* wkDoc, wkMacro, wkOutline */			
			{
/*  Figure out ypSplit in coordinates relative to top pane. */
			GetClientRect( (*hmwdCur)->hwnd, (LPRECT) &rc );
			ptParentClientOrg.yp = rc.ypTop;
			ClientToScreen( (*hmwdCur)->hwnd, (LPPOINT) &ptParentClientOrg );
			GetWindowRect( (*hwwdUpper)->hwnd, (LPRECT) &rc);
			ypSplit = (*hmwdCur)->ypSplit - (rc.ypTop - ptParentClientOrg.yp);
			pt.xp = (*hwwdUpper)->xwMin;
			pt.yp = ypSplit - 1;
			if (FOutlineEmpty(wwUpper, fFalse))
				cpFirstNew = cp0;
			else
				{
				dlSplit = DlWherePt(wwUpper, &pt, &hpldr, &idr, &spt, 
						fFalse /*fInner*/, fFalse);
				if (dlSplit != dlNil)
					{
					cpFirstNew = CpPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl, dlSplit);
					FreePdrf(&drfFetch);
					}
				else
					cpFirstNew = cp0;
				}
			}
		Assert(cpFirstNew <= CpMacDoc((*hwwdNew)->sels.doc));
		PdrGalley((*hwwdNew))->cpFirst = cpFirstNew;
		}
else /* fix up for page view */
		{
	/* copy dr */
		int idrMac = (*hwwdUpper)->idrMac;
		int idr;
		struct DR dr;

		(*hwwdNew)->wk = wkPage;
		(*hwwdNew)->ipgd = (*hwwdUpper)->ipgd;
		Assert((*hwwdUpper)->fPageView);
		FreeDrs(hwwdNew, 0);
		for (idr = 0; idr < idrMac; idr++)
			{
			blt(PInPl(hwwdUpper, idr), &dr, cwDR);
			dr.fDirty = fTrue;
			dr.hplcedl = hNil; /* let FUpdateDr allocate */
			if (!FInsertInPl(hwwdNew, idr, &dr))
				{
				FreeDrs(hwwdNew, 0);
				(*hwwdNew)->ipgd = ipgdNil;
				vmerr.fNoDrs = fTrue;
				break;
				}
			}

		/* this is how we scroll in page view */
		OffsetRect(&((*hwwdNew)->rcePage), 0, 
				(*hwwdUpper)->rcwDisp.ywTop - (*hwwdUpper)->rcwDisp.ywBottom);
		/* if creating a pageview pane, force all of this doc's pageview
			panes to match this one. */
		SyncPageViewGrpfvisi(DocMother(doc), (*hwwdNew)->grpfvisi, (*hwwdNew)->fDisplayAsPrint);
		}

	TrashWw(wwNew);
	if ((*hmwdCur)->fVertScrollBar)
		{
		(*hwwdNew)->hwndVScroll = hwndVScroll;
		if ((*hwwdNew)->fPageView)
			{
			FCreatePgvCtls(wwNew);
			ShowPgvCtls(wwNew, fFalse /*fMove*/);
			}
		SetElevWw(wwNew);
		}

	ShowWindow((*hwwdNew)->hwnd, SHOW_OPENWINDOW);
	UpdateWindowWw(wwNew);
	NewCurWw(wwNew, !fPageView);
	if (!fPageView)
		SelectIns(&selCur, cpFirstNew);
	return (fTrue);
}




/*  Remove the split bar and kill the given window.  Readjust the
	rest of the windows afterwards. */

/* %%Function:KillSplit %%Owner:rosiep */
KillSplit(hmwd, fLower, fAdjustWidth)
struct MWD **hmwd;
BOOL fLower;
BOOL fAdjustWidth; /* true if we are adjusting the window width as well */
{
	struct RC rcTopPane,rcTopScrlBar,rcSplitBox,
	rcSplitBar, rcBottomPane, rcBottomScrlBar, rc;
	HWND hwndT;
	HWND hwndRuler = NULL;
	int wwKill, wwKeep;
	int ypSplitOld = (*hmwd)->ypSplit;
	struct WWD *pwwdKeep;
	extern int vssc;
	BOOL fRecordingSav;

	if (!(*hmwd)->fSplit)
		return;

	/* Don't record anything during this command */
	InhibitRecorder(&fRecordingSav, fTrue);

	if (vssc != sscNil)
		CancelDyadic();

	wwKill = (*hmwd)->wwUpper;
	if (fLower || (PwwdWw((*hmwd)->wwLower)->wk & wkSDoc))
		wwKill = WwOther(wwKill);
	wwKeep = WwOther(wwKill);

	if (PwwdWw(wwKill)->wk == wkAtn)
		{ /* restore the fSeeHidden bit */
		FreezeHp();
		pwwdKeep = PwwdWw(wwKeep);
		if (pwwdKeep->grpfvisi.fSeeHidden && 
				pwwdKeep->grpfvisi.fSeeHidden != (*hmwd)->fAtnVisiSaved)
			{ /* avoid trashing wwKeep if not necessary */
			pwwdKeep->grpfvisi.fSeeHidden = (*hmwd)->fAtnVisiSaved;
			CopyGrpfvisiToScc(wwKeep);
			if (pwwdKeep->fPageView)
				SyncPageViewGrpfvisi(DocBaseWw(wwKeep), pwwdKeep->grpfvisi, pwwdKeep->fDisplayAsPrint);
			}
		MeltHp();
		}
	else  if (PwwdWw(wwKill)->wk == wkHdr)
		{
		FSaveHeader(PdrGalley(PwwdWw(wwKill))->doc, fTrue);
		vdocHdrEdit = docNil;
		}

	if (hmwd == hmwdCur)
		TurnOffSel(&selCur); /* Heap Moves! */

	DestroyPhwnd( &(*hmwd)->hwndSplitBar );
	if (PwwdWw(wwKill)->hwndVScroll != NULL)
		DestroyVScrlWw(wwKill);

	/* If we will be destroying the only outline iconbar and other
		pane is also in outline mode, move iconbar to other pane */
	if (PwwdWw(wwKeep)->fOutline && PwwdWw(wwKeep)->hwndIconBar == NULL)
		{
		Assert( PwwdWw(wwKill)->hwndIconBar != NULL );
		PwwdWw(wwKeep)->hwndIconBar = PwwdWw(wwKill)->hwndIconBar;
		}
	else  if (PwwdWw(wwKill)->hwndIconBar)
		{
		if (vfIconBarMode)
			{
			Assert(wwKill == wwCur);
			DestroyKmpIB(PwwdWw(wwKill)->hwndIconBar);
			}
		DestroyIconBar(mpwwhwwd[wwKill]);
		}

	/* If ruler is visible in this pane and other pane can have a
		ruler, move it there; else destroy it */
	if ((*hmwd)->wwRuler == wwKill)
		{
		if (!PwwdWw(wwKeep)->fOutline)
			{
			hwndRuler = (*hmwd)->hwndRuler;
			ShowWindow(hwndRuler, SW_HIDE);
			(*hmwd)->wwRuler = wwKeep;
			}
		else
			FDestroyRuler(hmwd);
		}

/* all child windows will be automatically destroyed when parent
	is destroyed, the ruler and icon bar have to be explicitly
	destroyed so as to clean up objects used by ruler and icon bar */

	CloseWw( wwKill );
	(*hmwd)->wwLower = wwNil;
	(*hmwd)->wwUpper = wwKeep;
	(*hmwd)->ypSplit = 0;

	FreezeHp();
	(pwwdKeep = PwwdWw(wwKeep))->fLower = fFalse;

	GetDocAreaPrc( *hmwd, &rc );
	GetDocAreaChildRcs( hmwd, rc,
			pwwdKeep->wk, 0 /*ypSplit*/,
			&rcTopPane, &rcTopScrlBar, &rcSplitBox,
			&rcSplitBar, &rcBottomPane, &rcBottomScrlBar );

	if (!FCanSplit(hmwd, wkDoc) && (*hmwd)->hwndSplitBox &&
			IsWindowVisible((*hmwd)->hwndSplitBox))
		{
		ShowWindow((*hmwd)->hwndSplitBox, HIDE_WINDOW);
		}

	if (pwwdKeep->fPageView)
		ShowPgvCtls(wwKeep, fTrue /*fMove*/);
	else
		MoveWindowRc( pwwdKeep->hwndVScroll, &rcTopScrlBar, fTrue );
	MoveWindowRc( (*hmwd)->hwndSplitBox, &rcSplitBox, fTrue );
	MeltHp();
	MovePane( wwKeep, &rcTopPane, ((fLower && !hwndRuler && !fAdjustWidth) ?
			mpdPaneOnly : mpdTopDown) );

	if (hwndRuler != NULL)
		ShowWindow(hwndRuler, SW_SHOWNORMAL);

	/* Allow recording again, if applicable */
	InhibitRecorder(&fRecordingSav, fFalse);
}



/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:SplitBarWndProc %%Owner:rosiep */
EXPORT LONG FAR PASCAL SplitBarWndProc( hwnd, message, wParam, lParam )
HWND     hwnd;
unsigned message;
WORD     wParam;
LONG     lParam;
{
	PAINTSTRUCT ps;
	LONG ropErase;
	HBRUSH hbr;

	switch (message)
		{
	case WM_PAINT:
			{
			int mw;

			for (mw = mwDocMin; mw < mwMac; mw++)
				if ((*mpmwhmwd[mw])->hwndSplitBar == hwnd)
					break;

			if (PwwdWw(PmwdMw(mw)->wwLower)->wk & wkSDoc)
				{/* different look for special split */
				BeginPaint( hwnd, (LPPAINTSTRUCT) &ps );
				SelectObject( ps.hdc, vsci.hbrBorder );
				PatBltRc(ps.hdc, &ps.rcPaint, PATCOPY);
				EndPaint( hwnd, (LPPAINTSTRUCT) &ps );
				break;
				}
			}
LDefault:
		return (DefWindowProc( hwnd, message, wParam, lParam ));
#ifdef DEBUG
	default:
		Assert(fFalse);
		goto LDefault;
#endif
		}

	/* A window proc should always return something. */
	return ((LONG) 0);
}


/* F  S P L I T  M W D */
/* Splits, moves a split, or removes a split. */
/* %%Function:FSplitMwd %%Owner:rosiep */
FSplitMwd(hmwd, yp, wk)
struct MWD ** hmwd;
int yp;
int wk;
{
	BOOL fRecordingSav;
	int dyp;
	struct MWD * pmwd;
	BOOL fInLower, fPaneChange;
	struct RC rc;
	int rgdyp[2];  /* height of ruler/iconbar area of panes */

	InhibitRecorder(&fRecordingSav, fTrue);

	pmwd = *hmwd;

	fInLower = pmwd->wwLower == wwCur;
	fPaneChange = 0;

	GetDocAreaPrc(pmwd, &rc);
	GetNonPaneRgdyp(hmwd, rgdyp);

/* Reposition an exisiting split... */

	if (pmwd->fSplit)
		{
		if (pmwd->ypSplit == yp)
			{
			InhibitRecorder(&fRecordingSav, fFalse);
			return fTrue;
			}

		if (yp < rc.ypTop + rgdyp[0] + vsci.dypScrlBar)
			{
			/* kill top pane */
			KillSplit(hmwd, fFalse/*fLower*/, fFalse /*fAdjustWidth*/);
			fPaneChange = !fInLower;
			yp = 0;
			goto LRecordSplit;
			}
		else  if (yp >= rc.ypBottom - vsci.dypScrlBar)
			{
			/* kill bottom pane */
			KillSplit(hmwd, fTrue/*fLower*/, fFalse /*fAdjustWidth*/);
			fPaneChange = fInLower;
			yp = 0;
			goto LRecordSplit;
			}
		else
			{
			/* change split position */
			pmwd->ypSplit = yp;
			AdjustDocAreaPrc(hmwdCur, 0, 0);

			goto LRecordSplit;
			}
		/* NOT REACHED */
		Assert(fFalse);
		}

/* Create a new split... */

	if (!FCanSplit(hmwd, wk))
		{
		InhibitRecorder(&fRecordingSav, fFalse);
		return fFalse;
		}

	/* don't create the split if split position is within the
		"kill split" zone */
	if ((yp < rc.ypTop + rgdyp[0] + vsci.dypScrlBar) ||
			(yp >= rc.ypBottom - vsci.dypScrlBar))
		{
		InhibitRecorder(&fRecordingSav, fFalse);
		return fFalse;
		}

	/* adjust yp so neither pane is too small (this is guaranteed
	to work because FCanSplit returned true above) */

	if ((dyp = vsci.dypWwMin - (rc.ypBottom - (yp + vsci.dypSplitBar))) > 0)
		{
		/* bottom pane would be too small */
		yp -= dyp;
		}
	else  if ((dyp = vsci.dypWwMin - (yp - rgdyp[0] - rc.ypTop)) > 0)
		{
		/* top pane would be too small */
		yp += dyp;
		}

	Assert(yp - rc.ypTop >= vsci.dypWwMin &&
			rc.ypBottom - (yp + vsci.dypSplitBar) >= vsci.dypWwMin);

	/* do the split */
	if (!FCreateSplit(docNil, yp, wk))
		{
		SetErrorMat(matMem);
		InhibitRecorder(&fRecordingSav, fFalse);
		return fFalse;
		}

LRecordSplit:
	InhibitRecorder(&fRecordingSav, fFalse);
	if (vfRecording)
		{
#ifdef BRADCH
		if (fPaneChange)
			RecordPaneChange();
#endif
		RecordSplit(yp);
		}

	return fTrue;
}


/* %%Function:CopyGrpfvisiToScc %%Owner:rosiep */
CopyGrpfvisiToScc(ww)
int ww;
{
	if (vsccAbove.ww == ww)
		vsccAbove.grpfvisi = PwwdWw(ww)->grpfvisi;
	ClearSccs();
	TrashWw(ww);
}


/* %%Function:FSplitBoxMouse %%Owner:rosiep */
FSplitBoxMouse(hwnd, msg, lParam)
HWND hwnd;
uns msg;
long lParam;
{
	struct PT pt;
	int wk;

	switch (msg)
		{
	case WM_LBUTTONDBLCLK:
		if ((*hmwdCur)->fSplit)
			{
			KillSplit(hmwdCur, wwCur != (*hmwdCur)->wwUpper /*fLower*/, 
					fFalse /*fAdjustWidth*/);
			if (vfRecording) RecordSplit(0);
			}
		else
			{
			pt.yp = (*hwwdCur)->ywMac / 2;
			pt.xp = 0;
			/* put it in mwd coordinates */
			ClientToScreen( (*hwwdCur)->hwnd, (LPPOINT) &pt );
			ScreenToClient( (*hmwdCur)->hwnd, (LPPOINT) &pt );
			FSplitMwd(hmwdCur, pt.yp, (*hwwdCur)->wk);
			}
		break;

	case WM_LBUTTONDOWN:
			{
			int docMom = DocMother(selCur.doc);

			EnsureFocusInPane();
			Assert (!vfInLongOperation);
			SetCursor( vhcSplit );

			SetOurKeyState();
			if (vfShiftKey && !vfControlKey)
				wk = wkFtn;
			else  if (vfControlKey && !vfShiftKey)
				wk = wkAtn;
			else
				wk = (*hwwdCur)->wk;

			if ((wk == wkFtn || wk == wkAtn) && !FHasRefs(docMom, wk == wkFtn ? edcDrpFtn : edcDrpAtn))
				{
				ErrorEid(wk == wkFtn ? eidNoFNToShow : eidNoANNToShow,
						"FSplitBoxMouse");
				break;
				}
			pt.xp = LOWORD(lParam);
			pt.yp = HIWORD(lParam);
			ClientToScreen(hwnd, (LPPOINT) &pt );
			TrackSplitBar( pt, wk );
			if ((wk == wkFtn || wk == wkAtn) && PwwdWw(wwCur)->wk == wk)
				{ /* viewing a ftn/atn pane */
				vfSeeSel = fTrue; /* cause mother doc in sync */
				}
			break;
			}
		}

	return fFalse;
}


