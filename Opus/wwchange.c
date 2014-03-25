/* WWCHANGE.C -- an eclectic collection of stuff, grouped for swap tuning
					purposes.  This module will be generally brought in when the 
					state of the world changes (windows global environment
					changes, window size changes, windows closed or opened,..*/


#define NOMINMAX
#define NOSCROLL
#define NOREGION
#define NOWH
#define NOMETAFILE
#define NOBITMAP
#define NOWNDCLASS
#define NOBRUSH
#define NONCMESSAGES
#define NOKEYSTATE
#define NOCLIPBOARD
#define NOHDC
#define NOCREATESTRUCT
#define NOGDICAPMASKS
#define NODRAWTEXT
#define NOCTLMGR
#define NOMENUS
#define NOFONT
#define NOMEMMGR
#define NORESOURCE
#define NOICON
#define NOKANJI
#define NOSOUND
#define NOCOMM

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "screen.h"
#include "help.h"
#include "doc.h"
#include "ch.h"
#include "props.h"
#include "opuscmd.h"
#include "format.h"
#include "version.h"
#include "splitter.h"
#include "doslib.h"
#include "sel.h"
#include "layout.h"
#include "idle.h"
#include "debug.h"
#include "inter.h"
#include "file.h"
#include "core.h"
#include "wininfo.h"
#include "message.h"
#include "automcr.h"
#include "el.h"
#include "macrocmd.h"
#include "scc.h"
#include "rareflag.h"
#include "print.h"

/* G L O A B L S */
int vdxpStagger; /* dxp offset for a child window initial position */
int vdypStagger; /* dyp offset for a child window initial position */


/* E X T E R N A L S */
extern MES **		vhmes;
extern struct STTB **   vhsttbWnd;
extern BOOL             vfWndCacheDirty;
extern IDF		vidf;
extern struct STTB **   vhsttbOpen;
extern int vgrfMenuCmdsAreDirty;
extern struct MERR		vmerr;
extern int vgrfMenuKeysAreDirty;
extern BOOL             vfFileCacheDirty;
extern struct FTI       vfti;
extern struct SCI       vsci;
extern HMENU            vhMenu;
extern struct PREF      vpref;
extern struct DOD       **mpdochdod[];
extern struct WWD       **mpwwhwwd[];
extern int              wwCur;
extern struct SEL       selCur;
extern struct MWD       **mpmwhmwd[];
extern int              wwMac;
extern BOOL             vfRecording;
extern BOOL		fRecordMove;
extern BOOL		fRecordSize;
extern HWND             vhwndCBT;
extern int              fnMac;
extern struct FCB       **mpfnhfcb[];
extern struct MWD       **hmwdCur;
extern int		mwCreate;
extern int              mwCur;
extern struct RC        vrcUnZoom;
extern int              docGlobalDot;
extern int              vmwClipboard;
extern HWND             vhwndStatLine;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern HWND             vhwndAppIconBar;
extern HWND             vhwndDeskTop;
extern HWND             vhwndPrompt;
extern HWND             vhwndPgPrvw;
extern int              vlm;
extern BOOL             vfInLongOperation;
extern int              vfInitializing;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern struct SAB       vsab;
extern int              vfTableKeymap;
extern HWND		vhwndMsgBoxParent;
extern struct UAB       vuab;
extern int              dclMac;
extern CHAR             szApp[];
extern HBRUSH           vhbrGray;
extern int              mwMac;
extern int              vfSysMenu;
extern struct SCC	vsccAbove;
extern struct WWD	**mpwwhwwd[];
extern struct PRI       vpri;
extern int              vcInMessageBox;

#ifdef	WIN
extern	BOOL	vfAs400;
#endif /* WIN */

HWND			vhwndZoom = hNil;



/* %%Function:MwdWndProcRare %%Owner:chic */
long MwdWndProcRare(hwnd, message, wParam, lParam)
HWND hwnd;
int message;
WORD wParam;
LONG lParam;
{

	extern int mwCreate;
	int mw;
	LPPOINT rgpt;

	switch (message)
		{
#ifdef DEBUG
	default:
		Assert (fFalse);
#endif /* DEBUG */

	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		Assert (mwCreate != mwNil);
		PmwdMw(mwCreate)->hwnd = hwnd;
		SetWindowWord(hwnd, IDWMW, mwCreate);
		break;

	case WM_MOVE:
		EnsureFocusInPane();

		/* Record the new position, LOWORD is x coordinate and
		HIWORD is y coordinate (all in client coordinate of vhwndDeskTop */

		mw = (mwCreate == mwNil ? MwFromHwndMw(hwnd) : mwCreate);
		if (mw != mwNil)
			{
			struct MWD *pmwd = PmwdMw(mw);

			pmwd->xp = LOWORD(lParam);
			pmwd->yp = HIWORD(lParam);
			/* the dimensions are recorded in the size message */

			if (vfRecording && fRecordMove)
				RecordDocMove(LOWORD(lParam), HIWORD(lParam));
			fRecordMove = fFalse;
			}
		break;

	case WM_SIZE:
		/* Window's size is changing.  lParam contains the height
		** and width, in the low and high words, respectively.
		** wParam contains SIZENORMAL for "normal" size changes,
		** SIZEICONIC when the window is being made iconic, and
		** SIZEFULLSCREEN when the window is being made full screen. */

		EnsureFocusInPane();

		if (wParam != SIZEICONIC)
			MwdSize(hwnd, LOWORD(lParam), HIWORD(lParam));

		if (vfRecording && fRecordSize)
			RecordDocSize(LOWORD(lParam), HIWORD(lParam));
		fRecordMove = fFalse;
		fRecordSize = fFalse;

		break;

	case WM_GETMINMAXINFO:
		rgpt = (LPPOINT) lParam;
		rgpt[3].y = vsci.dypScrlBar + vsci.dypBorder + 2 * vsci.dypWwMin +
				vsci.dypScrlBar + 1;
		break;
	case WM_CLOSE:
		CloseDocWin(hwnd, acQuerySave);
		break;

		}

	return(0L);
}


/* %%Function:RareWwPaneWndProc %%Owner:chic */
long RareWwPaneWndProc(hwnd, message, wParam, lParam)
HWND hwnd;
int message;
WORD wParam;
LONG lParam;
{
	int ww;
	struct WWD *pwwd;

	switch (message)
		{
	default:
		Assert( fFalse );
		break;
	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		WwPaneCreate( hwnd );
		break;

	case WM_SIZE:
			{
			int xwMac = LOWORD(lParam);
			int ywMac = HIWORD(lParam);

			AssertDo((ww = WwFromHwnd( hwnd )) != wwNil);
			pwwd = PwwdWw(ww);
			pwwd->fDirty = fTrue;

			if (!pwwd->fPageView)
		/* adjust height of DR */
				PdrGalley(pwwd)->dyl = ywMac - pwwd->ywMin;
		/* Record the new size of the pane window. */
			pwwd->xwMac = xwMac;
			pwwd->ywMac = ywMac;
			break;
			}
		}
	return 0L;
}



/* H w n d  C r e a t e  W i n d o w  R c */
/* %%Function:HwndCreateWindowRc %%Owner:chic */
HwndCreateWindowRc( szCls, ws, rc, hwndParent )
CHAR szCls[];
long ws;
struct RC rc;
HWND hwndParent;
{
	extern HANDLE vhInstance;

	return CreateWindow( (LPSTR)szCls, (LPSTR) NULL, ws, rc.xpLeft, rc.ypTop,
			rc.xpRight - rc.xpLeft,
			rc.ypBottom - rc.ypTop,
			hwndParent,
			(HMENU) NULL, vhInstance, (LPSTR) NULL );
}




/* M W D  S I Z E */
/* handles WM_SIZE message for MWD windows */

/* %%Function:MwdSize %%Owner:chic */
MwdSize(hwnd, dxpNew, dypNew)
HWND hwnd;
int dxpNew;
int dypNew;
{ /* This is for the child window, not the pane window */
	int mw;
	struct MWD *pmwd;
	int ddxp, ddyp;

	if (mwCreate == mwNil)
		{
		pmwd = PmwdMw( mw = MwFromHwndMw(hwnd) );
		ddxp = dxpNew - pmwd->dxp;
		ddyp = dypNew - pmwd->dyp;
		pmwd->dxp = dxpNew;
		pmwd->dyp = dypNew;
		AdjustMwdArea( mpmwhmwd[mw], ddxp, ddyp, fTrue );
		}
	else
		{
		pmwd = PmwdMw(mwCreate);
		pmwd->dxp = dxpNew;
		pmwd->dyp = dypNew;
		}
}


/* G E T  D O C  A R E A  P R C */

/* Return rect for documents + split + vert scroll bar through parm
	Results depend on:
		(1) size of mac-in-box child window
		(2) presence of scroll bars */

/* %%Function:GetDocAreaPrc %%Owner:chic */
GetDocAreaPrc( pmwd, prc )
struct MWD *pmwd;
struct RC *prc;
{
	FreezeHp();
	Assert( pmwd->hwnd != NULL );
	GetClientRect( pmwd->hwnd, (LPRECT)prc );

	if (pmwd->fHorzScrollBar)
		{
/* - vsci.dypBorder because border of scroll bar overlaps border of mwd */
		prc->ypBottom -= vsci.dypScrlBar - vsci.dypBorder;
		prc->ypBottom = max(0, prc->ypBottom);
		}
	MeltHp();
}



/* G E T   D O C  A R E A    C H I L D   R C S */
/* Supplies rectangles for all children of passed MWD based on
	parameters and state of mwd.
	prc's relating to split will not be valid if ypSplit == 0
	prc's for scroll bars will be valid if scroll bars are present
	NOTE: prc's for panes INCLUDE the rectangles of any iconbar or
		ruler that may be present.  MovePane should be used instead
		of MoveWindowRc whenever adjusting a pane to this rectangle,
		so that ruler/iconbar rectangles are also adjusted accordingly
*/

/* %%Function:GetDocAreaChildRcs %%Owner:chic */
GetDocAreaChildRcs( hmwd, rcMwd, wk, ypSplit, prcTopPane, prcTopScrlBar,
prcSplitBox, prcSplitBar, prcBottomPane, prcBottomScrlBar )

struct MWD **hmwd;
struct RC rcMwd;
int wk, ypSplit;
struct RC *prcTopPane;
struct RC *prcTopScrlBar;
struct RC *prcSplitBox;
struct RC *prcSplitBar;
struct RC *prcBottomPane;
struct RC *prcBottomScrlBar;
{
	struct MWD *pmwd = *hmwd;
	int dxpVertScroll =
	pmwd->fVertScrollBar ? vsci.dxpScrlBar - vsci.dxpBorder : 0;
	struct RC rcT;

	prcTopPane->ypTop = rcMwd.ypTop;
	prcTopPane->xpLeft = prcBottomPane->xpLeft = rcMwd.xpLeft;
	prcSplitBar->xpLeft = rcMwd.xpLeft - vsci.dxpBorder;

	prcTopPane->xpRight = prcBottomPane->xpRight = prcTopScrlBar->xpLeft =
			prcSplitBox->xpLeft = prcBottomScrlBar->xpLeft =
			rcMwd.xpRight - dxpVertScroll;
	prcSplitBar->xpRight = rcMwd.xpRight - dxpVertScroll + vsci.dxpBorder;

	prcTopScrlBar->xpRight = prcSplitBox->xpRight =
			prcBottomScrlBar->xpRight = rcMwd.xpRight + vsci.dxpBorder;

	if (ypSplit == 0)
		{
		if (!FCanSplit(hmwd, wkDoc))
			/* no split box */
			prcTopScrlBar->ypTop = rcMwd.ypTop - vsci.dypBorder;
		else
			prcTopScrlBar->ypTop = prcSplitBox->ypBottom = vsci.dypSplitBox +
					(prcSplitBox->ypTop = (rcMwd.ypTop - vsci.dypBorder));
		prcTopScrlBar->ypBottom = rcMwd.ypBottom + vsci.dypBorder;
		prcTopPane->ypBottom = rcMwd.ypBottom;
		}
	else
		{
		prcTopScrlBar->ypTop = rcMwd.ypTop - vsci.dypBorder;
		prcTopPane->ypBottom = prcSplitBar->ypTop = ypSplit;
		prcTopScrlBar->ypBottom = prcSplitBox->ypTop =
				ypSplit - ((vsci.dypSplitBox - vsci.dypSplitBar)>>1);

		prcBottomPane->ypTop = (prcSplitBar->ypBottom = 
				ypSplit + vsci.dypSplitBar);
		prcSplitBox->ypBottom = prcBottomScrlBar->ypTop =
				prcSplitBox->ypTop + vsci.dypSplitBox;
		prcBottomScrlBar->ypBottom = rcMwd.ypBottom + vsci.dypBorder;
		prcBottomPane->ypBottom = rcMwd.ypBottom;
		}
}




/* F I L L  R G W S I */
/* %%Function:FillRgwsi %%Owner:chic */
FillRgwsi(ww, pwsi, piwsi, prcScrlBar, prcPgvUp, prcPgvDown, prcPane)
int ww;
struct WSI *pwsi;
int *piwsi;
struct RC *prcScrlBar, *prcPgvUp, *prcPgvDown, *prcPane;
{
	int iwsi = *piwsi;

	if (PwwdWw(ww)->fPageView)
		{
		*prcPgvUp = *prcPgvDown = *prcScrlBar;

		pwsi->hwnd = PwwdWw(ww)->hwndPgvUp;
		prcPgvUp->ypBottom = prcPgvUp->ypTop + vsci.dypScrollArrow;
		pwsi->prc = prcPgvUp;
		pwsi->fPane = fFalse;
		pwsi++;

		pwsi->hwnd = PwwdWw(ww)->hwndPgvDown;
		prcPgvDown->ypTop = prcPgvDown->ypBottom - vsci.dypScrollArrow;
		pwsi->prc = prcPgvDown;
		pwsi->fPane = fFalse;
		pwsi++;
		iwsi += 2;

		/* adjust vert scroll bar for the two pageview controls */
		prcScrlBar->ypTop = prcPgvUp->ypBottom - vsci.dypBorder;
		prcScrlBar->ypBottom = prcPgvDown->ypTop + vsci.dypBorder;
		}
	pwsi->hwnd = PwwdWw(ww)->hwndVScroll;
	pwsi->prc = prcScrlBar;
	pwsi->fPane = fFalse;
	pwsi++;

	pwsi->ww = ww;
	pwsi->prc = prcPane;
	pwsi->fPane = fTrue;
	iwsi += 2;

	*piwsi = iwsi;
}


int dypDocAreaOld = -1;

/* A D J U S T   D O C   A R E A   P R C */
/* Adjust position of doc window(s), vert scroll bar(s), and split
	bar to fill passed RC.  Locate split at ypSplit if ypSplit is nonzero */
/* Intended to handle adjustments to the doc area size:
		(1) Movement of split
		(2) Addition or deletion of ruler
		(3) Addition or deletion of horizontal scroll bar
		(4) Resize of mwd window */

/* %%Function:AdjustDocAreaPrc %%Owner:chic */
AdjustDocAreaPrc( hmwd, ddxp, ddyp )
struct MWD **hmwd;
int ddxp; /* change in x direction. > 0 means grow, so far not being used yet. */
int ddyp; /* change in y direction. < 0 means shrink */
{
	struct MWD *pmwd = *hmwd;
	int wwUpper = (*hmwd)->wwUpper;
	int wwLower = (*hmwd)->wwLower;
	int ypSplit;
	int ypSplitOld = pmwd->fSplit ? pmwd->ypSplit : 0;
	int wk = PwwdWw(wwUpper)->wk;

	struct RC rcTopPane, rcTopScrlBar, rcSplitBar, rcSplitBox, rcBottomPane,
	rcBottomScrlBar;
	struct RC rcDocArea;

	Assert( hmwd != hNil && pmwd->wwActive != wwNil && wwUpper != wwNil );

	FreezeHp();
#ifdef DEBUG
	if (pmwd->fSplit)
		Assert(dypDocAreaOld != -1);
#endif /* DEBUG */

	GetDocAreaPrc( pmwd, &rcDocArea );
	if (ddyp != 0 && pmwd->fSplit)
		pmwd->ypSplit = NMultDiv(pmwd->ypSplit,
				rcDocArea.ypBottom - rcDocArea.ypTop, dypDocAreaOld);
	dypDocAreaOld = rcDocArea.ypBottom - rcDocArea.ypTop;

	ypSplit = (pmwd->fSplit ? pmwd->ypSplit : 0);

/* Compute Rectangles */

	GetDocAreaChildRcs( hmwd, rcDocArea, wk, ypSplit, 
			&rcTopPane, &rcTopScrlBar,
			&rcSplitBox, &rcSplitBar, &rcBottomPane, &rcBottomScrlBar);

/* possibility exists that proposed split will shrink one window below the
	minimum size -- redistribute space */

	MeltHp();

	if (pmwd->fSplit)
		{
		int dypT;
		int rgdyp[2];

		GetNonPaneRgdyp(hmwd, rgdyp);

		if (!FCanSplit(hmwd, wkDoc))
			{
			KillSplit(hmwd, pmwd->wwActive != pmwd->wwLower /*fLower*/,
					ddxp != 0 /*fAdjustWidth*/);
			return;
			}

		if ((dypT = rcBottomPane.ypBottom - rcBottomPane.ypTop - rgdyp[1]) <
				vsci.dypWwMin)
			{
			ypSplit -= vsci.dypWwMin - dypT;
LGet:
			GetDocAreaChildRcs( hmwd, rcDocArea, wk, ypSplit, &rcTopPane, &rcTopScrlBar,
					&rcSplitBox, &rcSplitBar, &rcBottomPane, &rcBottomScrlBar);

			}
		else  if ((dypT = rcTopPane.ypBottom - rcTopPane.ypTop - rgdyp[0]) <
				vsci.dypWwMin)
			{
			ypSplit += vsci.dypWwMin - dypT;
			goto LGet;
			}
		/* reestablish split position since it could have been adjusted 
			because of min size requirement */
		pmwd->ypSplit = ypSplit; 

		Assert( rcBottomPane.ypBottom - rcBottomPane.ypTop >= vsci.dypWwMin &&
				rcTopPane.ypBottom - rcTopPane.ypTop >= vsci.dypWwMin);
		}

/* perform necessary moves */

	if ((*hmwd)->hwndSplitBox)
		if (FCanSplit(hmwd, wkDoc))
			{
			MoveWindowRc( (*hmwd)->hwndSplitBox, &rcSplitBox, fTrue );
			if (!IsWindowVisible((*hmwd)->hwndSplitBox))
				{
				ShowWindow((*hmwd)->hwndSplitBox, SHOW_OPENWINDOW);
				}
			}
		else  if (IsWindowVisible((*hmwd)->hwndSplitBox))
			{
			ShowWindow((*hmwd)->hwndSplitBox, HIDE_WINDOW);
			}

#define cMoveWindows 9
		{
		struct RC rcT;
		struct PT ptT;
		struct WSI rgwsi[cMoveWindows];
		int iwsi, i, iT;
		BOOL fTopDown = fFalse;
		struct RC rcBottomPgvUp, rcBottomPgvDown, rcTopPgvUp, rcTopPgvDown;

		if ((*hmwd)->fSplit)
			{
			GetWindowRect(PwwdWw(wwUpper)->hwnd, (LPRECT) &rcT);
			ptT = rcTopPane.ptBottomRight;
			ClientToScreen((*hmwd)->hwnd, (LPPOINT) &ptT);
			if (ptT.yp < rcT.ypBottom)
				fTopDown = fTrue;
			}

		iwsi = 0;
		if ((*hmwd)->fSplit)
			{
			FillRgwsi(wwLower, &rgwsi[iwsi], &iwsi, &rcBottomScrlBar, 
					&rcBottomPgvUp, &rcBottomPgvDown, &rcBottomPane);
			rgwsi[iwsi].hwnd = (*hmwd)->hwndSplitBar;
			rgwsi[iwsi].prc = &rcSplitBar;
			rgwsi[iwsi++].fPane = fFalse;
			}
		FillRgwsi(wwUpper, &rgwsi[iwsi], &iwsi, &rcTopScrlBar, 
				&rcTopPgvUp, &rcTopPgvDown, &rcTopPane);
		Assert(iwsi <= cMoveWindows);

		for (i = 0; i < iwsi; i++)
			{
			iT = (fTopDown ? iwsi - i - 1 : i);
			if (rgwsi[iT].hwnd)
				{
				if (rgwsi[iT].fPane)
					MovePane(rgwsi[iT].ww, rgwsi[iT].prc,
							fTopDown ? mpdTopDown : mpdBottomUp);
				else
					MoveWindowRc(rgwsi[iT].hwnd, rgwsi[iT].prc, fTrue);
				}
			}
		}

}


/* F  C A N  S P L I T */
/* Returns true iff there's enough room in the mwd to have a split. */

/* %%Function:FCanSplit %%Owner:chic */
FCanSplit(hmwd, wk)
struct MWD **hmwd;
int wk;
{
	struct MWD *pmwd = *hmwd;
	struct RC rc;
	int dyp;
	BOOL fCanSplit;

	FreezeHp();
	Assert(pmwd->hwnd != NULL);
	GetClientRect(pmwd->hwnd, (LPRECT)&rc);
	if (pmwd->fHorzScrollBar)
		{
/* - vsci.dypBorder because border of scroll bar overlaps border of mwd */
		rc.ypBottom -= vsci.dypScrlBar - vsci.dypBorder;
		}

/* Calculate minimum height for child window to support a split */

	/* if we split we'd have at least two panes and a split bar */
	dyp = 2 * vsci.dypWwMin + vsci.dypSplitBar;

	if (pmwd->wwActive == wwNil)
		{
		if (vpref.fRuler)
			dyp += vsci.dypRuler;
		goto LRet;
		}

	/* there will never be more than one ruler and its existence
			is not affected by splitting */
	if (pmwd->hwndRuler)
		dyp += vsci.dypRuler;

	/* all split types will keep the top (or only) pane's iconbar if up */
	if (pmwd->wwUpper && PwwdWw(pmwd->wwUpper)->hwndIconBar)
		dyp += vsci.dypIconBar;

	switch (wk)
		{
	default:
		Assert (fFalse);
	case wkDoc:  /* normal window splitting or moving a split */
	case wkPage:
	case wkOutline:
	case wkMacro:
		if (pmwd->fSplit)  /* this will not be true yet unless split
				already existed */
			{
			/* bottom pane's iconbar */
			if (pmwd->wwLower && PwwdWw(pmwd->wwLower)->hwndIconBar)
				dyp += vsci.dypIconBar;
			}
		break;

	case wkFtn:  /* fall thru */
	case wkAtn:
		/* creating new split; new pane will have no children */
		Assert(!pmwd->fSplit);
		break;

	case wkHdr:
		/* creating new split; new pane will have an iconbar */
		dyp += vsci.dypIconBar;
		break;
		}

LRet:
	fCanSplit = (rc.ypBottom - rc.ypTop >= dyp);
	if (wk == wkDoc || wk == wkPage)
		pmwd->fCanSplit = fCanSplit;
	MeltHp();
	return fCanSplit;
}




/* M O V E  W I N D O W  R C */

/* %%Function:MoveWindowRc %%Owner:chic */
MoveWindowRc( hwnd, prc, fPaint )
HWND hwnd;
struct RC *prc;
BOOL fPaint;
{
	if (hwnd != NULL)
		{
		MoveWindow( hwnd, prc->xpLeft, prc->ypTop,
				prc->xpRight - prc->xpLeft,
				prc->ypBottom - prc->ypTop, fPaint );
/* if we want immediate update */
		if (fPaint)
			UpdateWindow( hwnd );
		}
}


/* min window size are arbitary */
#define DXPMWMIN   50
#define DYPMWMIN   60


/* %%Function:GetMwPrc %%Owner:chic */
GetMwPrc(mw, prc)
struct RC *prc;
int mw;
	{ /* Fill in prc, cStagger is used to determine the starting xp
			and yp position if not vpref.fZoomMwd.
			Note : this routine is used only for creating a mac window
	*/

	extern int vdxpStagger;
	extern int vdypStagger;
	extern int mwMac;
	static int cStagger = 0;

	int mwT;
	struct MWD *pmwd;
	int xpLeft;
	int ypTop;


	if (vpref.fZoomMwd)
		{
		GetWindowRect( vhwndDeskTop, (LPRECT)prc );
		ScreenToClient( vhwndDeskTop, (LPPOINT)&(prc->ptTopLeft) );
		ScreenToClient( vhwndDeskTop, (LPPOINT)&(prc->ptBottomRight) );
		return;
		}

	if (mwMac == mwDocMin+1)
	/* no other open docs */
		cStagger = 0;

	if (vidf.fIconic)
	/* pretend we are full-sized */
		SetRect(prc, 0, 0, GetSystemMetrics(SM_CXFULLSCREEN),
				GetSystemMetrics(SM_CYFULLSCREEN) - GetSystemMetrics(SM_CYMENU));
	else
		GetClientRect(vhwndDeskTop, (LPRECT)prc);

	xpLeft = xpStartFromcStagger(cStagger);
	ypTop = ypStartFromcStagger(cStagger);

/* Check overlap starting position */
	for (mwT = mwDocMin; mwT < mwMac; mwT++)
		{
		if (mpmwhmwd[mwT] != hNil)
			{
			pmwd = *mpmwhmwd[mwT];
			if (pmwd->xp == xpLeft && pmwd->yp == ypTop)
				{
			/* offset the starting position */
				xpLeft += vdxpStagger;
				ypTop += vdypStagger;
				mwT = 0; /* try again */
				}
			}
		}

	if ((prc->xpRight - xpLeft) < DXPMWMIN ||
			(prc->ypBottom - ypTop) < DYPMWMIN)
		xpLeft = ypTop = 0; /* let it be as large as the desktop */

	prc->xpLeft = xpLeft;
	prc->xpRight -= vdxpStagger;
	prc->ypTop = ypTop;
	prc->ypBottom -= vdypStagger;
	cStagger++;
}





/*------------------------ FORMERLY FILEUTIL.C----------------------------*/

CHAR  stDOSPath[ichMaxFile+1]; /* +1 for terminator added in FNormalize..*/

CHAR *PchSkipSpacesPch();


/* %%Function:CopyChUpper %%Owner:chic */
NATIVE CopyChUpper( szSource, szDest, cch )
CHAR *szSource;
CHAR *szDest;
int cch;
{
	while (cch--)
		*(szDest++) = ChUpper(*(szSource++));
}


/* U p d a t e  S t D O S P a t h */
/* store current DOS path in stDOSPath, because asking for it can hit the disk */
/* %%Function:UpdateStDOSPath %%Owner:chic */
UpdateStDOSPath()
{
	CHAR szT [ichMaxFile];
	CHAR stDOSSave[ichMaxFile];

	CopySt(stDOSPath, stDOSSave); /* save path in case we fail (drive door open) */

	stDOSPath[0] = '\0';  /* force FNormalize... to fetch the path */
	if (FNormalizeSzFile( szEmpty, szT, nfoPathOK ))
		SzToSt( szT, stDOSPath );
	else
		CopySt(stDOSSave, stDOSPath);
}



/*          FNormalizeStFile( stFile, stFileNorm, nfo )
*
*      Converts a MSDOS filename into an unambiguous representation
*
*
*      See FNormalizeSzFile
*
*      This routine will also append an extension if the passed filename
*      does not have one:
*        nfoNormal:           ".DOC" (szDoc)
*        nfoPic:              ".TIF" (szTif)
*        nfoDot or nfoDotExt: ".DOT" (szDot)
*      If nfoDot and there is no path specified, this routine will provide
*      the proper template path (not current dir).
*/


/* %%Function:FNormalizeStFile %%Owner:chic */
FNormalizeStFile( stFile, stFileNorm, nfo )
CHAR *stFile;
CHAR *stFileNorm;
int  nfo;
{
	extern CHAR szDoc[], szDot[], szTif[];
	BOOL f;
	BOOL fDot = (nfo & nfoDot);
	struct FNS fns;
	CHAR szFile [ichMaxFile];

	/* prepend dot path if name does not start with backslash or drive
		so that dot filenames are relative to the dot directory
	*/
	if (fDot && *stFile >= 2 && *(stFile+2) != ':' && *(stFile+1) != '\\')
		/*  no path specified for DOT, prepend the correct DOT path */
		{
		if (!FGetStFpi(fpiDotPath, szFile))
			AssertDo(FGetStFpi(fpiProgram, szFile));
		  /* leave room for ichMaxFile - 1 chars and size byte */
		if (*szFile + *stFile >= sizeof(szFile))
			return fFalse;
		StStAppend(szFile, stFile);
		StToSzInPlace(szFile);
		}
	else
		{
		/* in place of a StToSz call which can destroy the stack */
		int cch = min((int)(stFile[0]), sizeof(szFile)-1);
		Assert(cch >= 0);
		*(bltbyte(&stFile[1], szFile, cch)) = '\0';
		}

	f = FNormalizeSzFile( szFile, stFileNorm, (nfo & ~nfoDot));

	if (f)
		{
	/* We do all this work only if we were successful normalizing. */
		SzToStInPlace( stFileNorm );

	/* Add an extension for a document file, if no extension was supplied */
	/* Do not add if stFileNorm contains just a path name! */

		if (stFileNorm[stFileNorm[0]] != '\\')
			{
			StFileToPfns( stFileNorm, &fns );
			if (fns.stExtension [0] == 0)
				{
				SzToSt(fDot || (nfo & nfoDotExt) ? szDot :
						(nfo & nfoPic) ? szTif :
						szDoc, fns.stExtension);
				PfnsToStFile( &fns, stFileNorm );
				}
			}
		}

	return f;
}


/***        FNormalizeSzFile - Normalize MSDOS filename
*
*      Converts a MSDOS filename into an unambiguous representation
*
*      ENTRY:  szFile - a filename; drive, path, and extension
*                       are optional
*      EXIT:   szNormal - A normalized filename or pathname
*                         (pathnames always include a terminating '\')                    
*      RETURNS: FALSE - Errors found in filename     (szNormal left undefined)
*               TRUE  - No errors found in filename  ( but there may be some
*                                                      that we didn't find )
*
*      The form of the filename on entry is:
*
*      { <drive-letter>: }{ <amb-path> }<filename>{.<extension>}
*
*      The form of the normalized filename is:
*
*      <drive-letter>:<unamb-path><filename>.<extension>
*
*      Where all alphabetics in the normalized name are in upper case
*      and <unamb-path> contains no "." or ".." uses nor any forward
*      slashes.
*
*      All attributes required in the normalized filename and not
*      provided in the szFile are taken from the defaults:
*
*          drive - current (DOS)
*          path - current (DOS)
*
*      It is permissible to call this routine with szFile containing a path
*      name instead of a filename.  The resulting szNormal will be backslash
*      terminated if szFile was, not if szFile was not.
*      "" is converted into the current path
*
*      WARNING:  The paths "." and ".." will produce errors
*                (but ".\" and "..\" are OK)
*
*
*/


/* %%Function:FNormalizeSzFile %%Owner:chic */
FNormalizeSzFile( szFile, szNormal, nfo )
CHAR *szFile;
CHAR *szNormal;
int nfo;
{
/* Treat separators like terminators */

#define FIsTermCh( ch )     ((ch) == '\0' || (ch) == ',' || (ch == ' ') || \
								(ch) == '+' || (ch) == '\011')
	CHAR szPath [ichMaxFile];

	int junk;
	int  cchPath;
	CHAR *pchFileEye;   /* We read szFile with the Eye */
	CHAR *pchNormPen;   /* and write szNormal with the Pen */
	CHAR *pchNormPath;
	CHAR *pchPath;
	int fHasDrive;

/* open code PchSkipSpacesPch */
	while (*szFile == ' ' || *szFile == 0x09)
		szFile++;
	pchFileEye = szFile;

	fHasDrive = (szFile [1] == ':');

/* Assert( CchSz( szFile ) <= ichMaxFile );*/
	if (CchSz(szFile) > ichMaxFile)
		return(FALSE);

/* Get current (DOS) path: "X:\...\...\" */

	if (stDOSPath [0] != 0 && (!fHasDrive || szFile [0] == stDOSPath [1]))
		{       /* already have path for current drive stored in a global */
		pchPath = &stDOSPath [1];
		cchPath = stDOSPath [0];
		stDOSPath [++cchPath] = '\0';   /* pretend sz */
		}
	else
/* fetch current path for drive specified in szFile, or for current drive
	if szFile has no explicit drive letter. Advance the eye past the drive
	letter and colon. */
		{
		StartUMeas(umNormPath);
		if ((cchPath = CchCurSzPath(
				pchPath = &szPath [0],
				fHasDrive ? (ChUpper(szFile [0]) - ('A'-1)) : 0)) < 3)
			return fFalse;
		OemToAnsi( szPath, szPath );
		StopUMeas(umNormPath);
		}

/* Write Drive Letter and colon */
	CopyChUpper( pchPath, &szNormal [0], 2 );

	pchNormPen = pchNormPath = &szNormal [2];
	pchPath += 2;
	cchPath -= 2;
	if (fHasDrive)
		pchFileEye += 2;

/* Now we have pchNormPen, pchPath, pchFileEye pointing at their path names */

	/* Write path name */
	if ( (*pchFileEye == '\\') || (*pchFileEye =='/') )
		{   /* "\....." -- basis is root */
		*pchFileEye++;
		*(pchNormPen++) = '\\';
		}
	else
		{   /* ".\" OR "..\" OR <text> -- basis is current path */
		CopyChUpper( pchPath, pchNormPen, cchPath );
		pchNormPen += cchPath - 1;
		}

	for ( ;; )
		{           /* Loop until we have built the whole szNormal */
		register CHAR ch=*(pchFileEye++);
		register int  cch;

		Assert( *(pchNormPen - 1) == '\\' );
		Assert( (pchNormPen > pchNormPath) &&
				(pchNormPen <= &szNormal [ichMaxFile]));

		if ( FIsTermCh( ch ) )
			/* We get here if there is no filename portion  */
			/* This means we have produced a path name */
			{
			Assert(pchNormPen < &szNormal [ichMaxFile]);
			*pchNormPen = '\0';
			break;
			}

		if ( ch == '.' )
			if ( ((ch = *(pchFileEye++)) == '\\') || (ch == '/') )
				/* .\ and ./ do nothing */
				continue;
			else  if ( ch == '.' )
				if ( ((ch = *(pchFileEye++)) == '\\') || (ch == '/') )
					{   /* ..\ and ../ back up by one directory */
					for ( pchNormPen-- ; *(pchNormPen-1) != '\\' ; pchNormPen-- )
						if ( pchNormPen <= pchNormPath )
							/* Can't back up, already at root */
							return FALSE;
					continue;
					}
				else
					/* ERROR: .. not followed by slash */
					return FALSE;
			else
				/* Legal file and path names do not begin with periods */
				return FALSE;

	/* Filename or Path -- copy ONE directory or file name */

		for ( cch = 1; !FIsTermCh(ch) && ( ch != '\\') && ( ch != '/' ) ; cch++ )
			ch = *(pchFileEye++);

		if ( cch > ichMaxLeaf || (pchNormPen - szNormal + cch > ichMaxFile))
			/* Directory or file name too long or entire name too long */
			return FALSE;

		CopyChUpper( pchFileEye - cch, pchNormPen, cch );
		pchNormPen += cch;
		if ( ch == '/' )
			*(pchNormPen-1) = '\\';
		else  if ( FIsTermCh( ch ) )
			{    /* Filename looks good, add extension & exit */
			*(pchNormPen-1) = '\0';
			break;
			}
		}   /* Endfor (loop to build szNormal) */

/* If there is anything but whitespace after the filename, then it is illegal */

	pchFileEye--;  /* Point at the terminator */
	Assert( FIsTermCh( *pchFileEye ));

	for ( ;; )
		{
		CHAR ch = *(pchFileEye++);

		if (ch == '\0')
			break;
		else  if ((ch != ' ') && (ch != '\011'))
			/* Non-whitespace after filename; return failure */
			return FALSE;
		}

	Assert( CchSz(szNormal) <= ichMaxFile );

	return FntSz( szNormal, CchSz( szNormal ) - 1, &junk, nfo);
}


#ifdef DEBUG
/* %%Function:FStFileIsNormal %%Owner:chic */
FStFileIsNormal(st)
CHAR *st;
{
	CHAR stNorm[ichMaxFile];
	return FNormalizeStFile(st, stNorm, nfoNormal) &&
			FEqualSt(st, stNorm);
}


#endif /* DEBUG */


#ifdef DEBUG
/* %%Function:FValidStFile %%Owner:chic */
FValidStFile( stFile )
CHAR stFile [];
{   /* return TRUE if the passed stFile is valid, FALSE otherwise */
	int ichErr;
	int ichMac = stFile [0];
	int fValid = fFalse;

	StToSzInPlace( stFile );
	if ((uns)ichMac > 0)
		fValid = FValidFile( stFile, ichMac, &ichErr, nfoNormal );
	SzToStInPlace( stFile );
	return fValid;
}
#endif

/* Parses the cch chars stored in rgch. If fSpecAllowed is true, wild
	card characters are interpreted per MSDOS.  Returns fntInvalid if
	it is an invalid file name.  fntValid if valid and fSpecAllowed
	is fFalse.  fntValid if valid, fSpecAllowed fTrue but no wild card
	chars.  fntValidWild otherwise.  If fntInvalid is returned, *pichError
	is updated to have ich of first illegal char in the name. */
/* NOTE: this routine is tuned for ASCII on MS-DOS */

/* %%Function:FntSz %%Owner:chic */
int FntSz(rgch, ichMax, pichError, nfo)
register char rgch[];
int ichMax;
int *pichError;
int nfo;
{
	int ich;
	register int ichStart;
	CHAR ch;
	int cchBase;
	int dchWE, dchWEName;
	int fWasWild;
	int ichDot = iNil;
	int ichPathSepLast = -1;

	ichStart = 0;
	if (ichMax >= 2 &&
			FAlpha(rgch[0]) && rgch[1] == ':')
		{
		ichStart = 2;
		ichPathSepLast = 2;
		}

	for (; ichStart < ichMax;)
		{
		/* Does the file name begin with ".\" or "..\"? */
		if (rgch[ichStart] == '.' &&
				(rgch[ichStart + 1] == '\\' || rgch[ichStart + 1] == '/'))
			{
			ichStart += 2;
			}
		else  if (rgch[ichStart] == '.' && rgch[ichStart + 1] == '.' &&
				(rgch[ichStart + 2] == '\\' || rgch[ichStart + 2] == '/'))
			{
			ichStart += 3;
			}
		else
			{
			break;
			}
		}

	cchBase = ichStart;

	if (ichStart >= ichMax && !(nfo & nfoPathOK))
		{
		ich = ichStart;
		goto badchar;
		}

	dchWEName = dchWE = 0;
	fWasWild = fFalse;
	/* Are all characters legal? */
	for (ich = ichStart; ich < ichMax; ich++)
		{
		ch = rgch[ich];
		/* range check */

		if ((unsigned char)ch >= 0x80)
		/* To allow international filenames, pass everything above 128 */
			continue;
		if (ch < '!' || ch > '~')
			goto badchar;
		switch (ch)
			{
		default:
			continue;
		case '.':
			if (ichDot != iNil || ich == cchBase)
				/* More than  one dot in the name */
				/* Or null filename */
				goto badchar;
			ichDot = ich;
			dchWEName = dchWE;
			dchWE = 0;
			continue;
		case '\\':
		case '/':
			/* We can not have wild character in the path portion. */
			if (ichDot == iNil)
				{
				if (dchWE != 0)
					{
					goto badchar;
					}
				}
			else
				{
				if (dchWE != 0 || dchWEName != 0)
					{
					goto badchar;
					}
				}

			/* Are there no more than eight chars before the '.'? */
			if (((ichDot == iNil) ? ich : ichDot) - cchBase > 8)
				{
				ich = 8+cchBase;
				goto badchar;
				}
			/* If there is no '.' we are fine */
			/* Are there no more than three chars after the '.'? */
			if (ichDot != iNil && ich - ichDot - 1 > 3)
				{
				ich = ichDot + 3 + 1;
				goto badchar;
				}

			/* note end of the path */
			if (ich + 1 == ichMax && !(nfo & nfoPathOK))
				goto badchar;
			cchBase = ich+1;
			ichDot = iNil;
			dchWEName = dchWE = 0;
			ichPathSepLast = ich;
			continue;
		case chDOSWildAll:
			dchWE--;
			/* Fall through is intentional */
		case chDOSWildSingle:
			fWasWild = fTrue;
			if (!(nfo & nfoWildOK))
				{
				goto badchar;
				}
			continue;
		case ':':
		case '"':
/*          case '#':     DOS allows this char */
		case '+':
		case ',':
		case ';':
		case '<':
		case '=':
		case '>':
		case '[':
		case ']':
		case '|':
			goto badchar;
			}
		}

	/* Are there no more than eight chars before the '.'? */
	if (((ichDot == iNil) ? dchWE : dchWEName) + 
			((ichDot == iNil) ? ichMax : ichDot) - cchBase > 8)
		{
		ich = 8 + cchBase;
		goto badchar;
		}
	/* If there is no '.' we are fine */
	/* Are there no more than three chars after the '.'? */
	if (ichDot != iNil && dchWE + ichMax - ichDot - 1 > 3)
		{
		ich = ichDot + 3 + 1;
		goto badchar;
		}

	if (nfo & nfoPathCk && ichPathSepLast > 0)
		{
		CHAR szT[ichMaxFile];

		ichPathSepLast = min(ichMaxFile - 1, ichPathSepLast);
		bltbyte(rgch, szT, ichPathSepLast);
		szT[ichPathSepLast] ='\0';
		if (!FValidSubdir(szT))
			{
			ich = ichPathSepLast;
			goto badchar;
			}
		}

	if (!(nfo & nfoWildOK))
		return fntValid;
	else
		return (fWasWild ? fntValidWild : fntValid);

badchar:
	*pichError = ich;
	return fntInvalid;
}


/* %%Function:FValidSubdir %%Owner:chic */
FValidSubdir(sz)
char *sz;
{
	int da;
	int ichLast = CchSz(sz)-2;
	CHAR szOem[ichMaxFile];

	AnsiToOem((LPSTR) sz, (LPSTR) szOem);

	if (ichLast >= 0 && szOem[ichLast] == '\\')
		/* remove trailing \ */
		szOem[ichLast--] = 0;

	if (!*szOem || (ichLast == 1 && szOem[1] == ':'))
	/* cases: ""(current dir), "a:"(root) valid*/
		return fTrue;
	else
		{
		Assert(vfNovellNetInited);
		if (vmerr.fNovellNet)
			/* insane world.  must cd to dir to see if it is a valid subdir */
			{
			CHAR stzPrevPath[ichMaxFile+1];
			CHAR stzNewPath[ichMaxFile+1];
			BOOL fRet;

			stzPrevPath[0] = CchCurSzPath(&stzPrevPath[1], 0)-1;
			if (stzPrevPath[stzPrevPath[0]] == '\\')
				stzPrevPath[stzPrevPath[0]--] = 0;
			stzNewPath[0] = CchCopyLpszCchMax(szOem, &stzNewPath[1], ichMaxFile);

#ifdef PCJ
			CommSzSt(SzShared("stzPrev: "), stzPrevPath);
			CommSzSz(SzShared("stzPrev: "), stzPrevPath+1);
			CommSzSt(SzShared("stzNew: "), stzNewPath);
			CommSzSz(SzShared("stzNew: "), stzNewPath+1);
#endif /* PCJ */

			/* try to cd to new directory */
			fRet = FSetCurStzPath(stzNewPath);
			/* go back to where we were */
			AssertDo(FSetCurStzPath(stzPrevPath));
			return fRet;
			}
		else
			{
			da = DaGetFileModeSz(szOem);
			return da != DA_NIL && (da & DA_SUBDIR);
			}
		}
}




/* %%Function:StFileToPfns %%Owner:chic */
StFileToPfns( stFile, pfns )
CHAR stFile[];
struct FNS *pfns;
	{   /* Decompose filename into a FNS structure at passed pointer.
		A NULL extension will leave stExtension [0] == 0; a "." extension
		will yield stExtension [0] == 1, stExtension [1] == '.'
		Mapping a normalized stFile to a FNS and back will not disturb
		normalization.
		Process works OK for path names w/o filenames too, as long as
		they have a trailing "\"
		(e.g. C:\WINDOWS\BIN\)
		WARNING: This routine may not work correctly for some filenames
		that have not been normalized.
*/
	int ichLast = stFile [0];
	int cch;
	CHAR *pch;
	CHAR ch;

      /* FUTURE: (bz) instead of using the ichMaxes more or less blindly, we
         should get the sizes for blting directly from the structure 
      */
	Assert( sizeof (pfns->stPath) >= ichMaxPath );
	Assert( sizeof (pfns->stShortName) >= ichMaxShortName );
	Assert( sizeof (pfns->stExtension) >= ichMaxExtension );

	pfns->stPath [0] = pfns->stShortName [0] = pfns->stExtension [0] = 0;

	if (ichLast == 0)
		return;

/* Map extension --> pfns->stExtension; adjust ichLast  */

	for ( cch = 0, pch = &stFile [ichLast]; ++cch < ichMaxExtension; pch-- )
		if ((ch = *(pch)) == '.')
			{
			Assert( (unsigned) cch < ichMaxExtension );
			bltbyte( pch, &pfns->stExtension [1],
					pfns->stExtension [0] = cch );
			ichLast = pch - stFile - 1;
			break;
			}
		else  if (ch == '\\' || ch == ':')
			/* Either there is no extension, or we have been passed
				a path name that has an extension, like
				"C:\PUBLIC\WINDOWS.NV\" */
			break;

/* Map short file name --> pfns->stShortName; adjust ichLast  */

	for ( cch = 0, pch = &stFile [ichLast];
			++cch < ichMaxShortName + 1 && pch >= stFile; pch-- )
		if (pch == stFile || (ch = *(pch)) == '\\' || ch == ':')
			{
			if (--cch > 0)
				{
				Assert( (unsigned) cch < ichMaxShortName );

				bltbyte( ++pch, &pfns->stShortName [1],
						pfns->stShortName [0] = cch );
				ichLast = pch - stFile - 1;
				Assert( ichLast >= 0 );
				}
			break;
			}

/* We may not have gotten a short name if the passed string was just a path */
/* Copy stFile up through ichLast into pfns->stPath */

	Assert( (unsigned) ichLast < ichMaxPath );
       
        /* umin test is bulletproofing so we don't overwrite.
             Still bogus, so we still assert, but cut our losses in
             fast version bz
          */
	bltbyte( &stFile [1], &pfns->stPath [1],
        (pfns->stPath [0] = umin (ichLast, ichMaxPath - 1)) );
}



/* %%Function:PfnsToStFile %%Owner:chic */
PfnsToStFile( pfns, stFile )
struct FNS *pfns;
CHAR stFile[];
{
	CopySt(pfns->stPath, stFile);
	StStAppend(stFile, pfns->stShortName);
	StStAppend(stFile, pfns->stExtension);
}


/* S t F i l e A b s  T o  S t F i l e R e l */
/* convert from full, normalized pathname of a file to one that is
	relative to the current drive and directory, according to the
	following rules:

	if filename drive != current drive
		==> return full pathname with drive
	else if filename path matches or is below current directory
		==> return path relative to current directory
	else
		==> return full pathname without drive

This routine is written so that the two passed parameters may point to 
the same buffer.
*/

/* %%Function:StFileAbsToStFileRel %%Owner:chic */
StFileAbsToStFileRel( stFileAbs, stFileRel )
CHAR stFileAbs[], stFileRel[];
{
	Assert( stDOSPath [0] >= 3 );
	if (stFileAbs [1] != stDOSPath [1] || stFileAbs [2] != ':')
		{ /* not a full path name or drive name does not match */
		if (stFileAbs != stFileRel)
			CopySt( stFileAbs, stFileRel );
		}
	else  if ( stDOSPath [0] < stFileAbs [0] &&
			!FNeRgch( &stFileAbs [1], &stDOSPath [1], stDOSPath [0]))
		{
		int cchPath = stDOSPath [0];

		bltbyte( &stFileAbs [cchPath+1], &stFileRel [1],
				stFileRel [0] = stFileAbs [0] - cchPath );
		}
	else
		{
		bltbyte( &stFileAbs [3], &stFileRel [1],
				stFileRel [0] = stFileAbs [0] - 2 );
		}
}




/* S T  S H O R T  F R O M  S T  N O R M  F I L E */
/*  Returns a filename of the form  XXXXXXXX.YYY from a normalized
	filename.
*/

/* %%Function:SzShortFromStNormFile  %%Owner:chic */
SzShortFromStNormFile (szShort, stNorm)
CHAR *szShort, *stNorm;

{
	struct FNS fns;

	/*  get short file name */
	StFileToPfns (stNorm, &fns);
	CopySt (fns.stShortName, szShort);
	StStAppend (szShort, fns.stExtension);
	StToSzInPlace (szShort);
}




/* F  V A L I D  F I L E N A M E */
/*  Assure that stFile 1) can be normalized, 2) that it is a valid
	filename and 3) that it is not a path.  If pfExists is not NULL then
	it is set true if the file exist, else false.  If fDot then the default
	extension is .DOT, else is .DOC.  If returns true and stName is not
	NULL then *stName will contain the short form of the filename (FOO.DOC).
*/

/* %%Function:FValidFilename  %%Owner:chic */
FValidFilename (stFile, stNorm, szName, pfOnDisk, pfOnLine, nfo)
CHAR *stFile, *stNorm, *szName;
BOOL *pfOnDisk, *pfOnLine;
int nfo;

{
	nfo |= nfoPathCk;

	if (*stFile == 0 || !FNormalizeStFile (stFile, stNorm, nfo))
		return fFalse;

	Assert(FValidStFile(stNorm) && stNorm[*stNorm] != '\\');

	if (pfOnDisk != NULL)
		*pfOnDisk = FExistsStFile (stNorm, fFalse);
	if (pfOnLine != NULL)
		*pfOnLine = (DocFromSt (stNorm) != docNil);

	if (szName != NULL)
		SzShortFromStNormFile (szName, stNorm);

	return fTrue;
}



/* F   E X I S T S   S T   F I L E */
/* %%Function:FExistsStFile %%Owner:chic */
FExistsStFile(stFile, fAnywhere)
CHAR stFile[];
int fAnywhere;
	{ /*
	DESCRIPTION:
	Determine if file stFile exists.  If fAnywhere is TRUE, check first in the
	array of fcbs to see if it might exist on some other floppy not currently
	on-line.  If fAnywhere is FALSE, only check for existence on the diskette that
	is currently on-line.
	RETURNS:
	fTrue iff file exists, fFalse otherwise.
*/
	OFSTRUCT ofs;
	CHAR szFile [ichMaxFile];

	Assert( FValidStFile( stFile ) );

	if (fAnywhere)
		{
		if (FHasFcbFromSt( stFile ))
			return fTrue;
		}
	else
		{
		if (FnFromOnlineSt( stFile ) != fnNil)
			return fTrue;
		}

	Assert( stFile [0] < ichMaxFile );
	StToSz( stFile, szFile );

	return (OpenFile( (LPSTR) szFile, (LPOFSTRUCT)&ofs, 
			(OF_EXIST | OF_READ | bSHARE_DENYWR) ) != -1) ||
			(ofs.nErrCode == nErrNoAcc /*&& DosxError() == dosxSharing*/ );

}       /* end FExistsStFile */



/*  %%Function:FnFromOnlineSt %%Owner:peterj  */
/* F N  F R O M  O N  L I N E  S T */
int FnFromOnlineSt( st )
CHAR st[];
	{       /* Check currently-online disk for the presence of file st.
		If the file exists and there is already an fn for it,
		return the fn.  If not, return fnNil.
		Complexity is because under Windows, we may have more than one
		fn that matches the name st.
		If there is no fn for any file with the name st, we do not hit
	the disk */

	extern int vfScratchFile;
	int fn;
	struct FCB *pfcb;
	struct FCB **hfcb;
	struct OFS ofs;

	for ( fn = 1 + !(vmerr.fScratchFileInit && !vmerr.fPostponeScratch); 
			fn < fnMac; fn++ )
		{
		if ((hfcb = mpfnhfcb [fn]) != NULL)
			{
			pfcb = *hfcb;

			if (FEqualSt( st, pfcb->stFile ))
				{
				int osfn;

				if (pfcb->ofh.fFixedDisk)
					return fn;

				/* removable media, must close and re-open
					file to make sure the right disk is in
					the drive */

				FCloseFn( fn );

				/* Flags:
						OF_REOPEN   Use contents of structure
						OF_EXIST    Leave file closed  */

				bltbyte( &pfcb->ofh, &ofs, sizeof (struct OFH) );
				StToSz( pfcb->stFile, ofs.szFile );
				AnsiToOem( ofs.szFile, ofs.szFile );
				osfn = OpenFile( (LPSTR) szEmpty,
						(LPOFSTRUCT)&ofs, OF_REOPEN|OF_EXIST);

				if (osfn != osfnNil)
					/* Found matching fcb */
					return fn;
				}
			}

		}

	return fnNil;
}


/* A D D  S T  F I L E  T O  M R U */
/* Place passed st at front of Work on Cache */
/* st must already be normalized! */
/* %%Function:AddStFileToMru %%Owner:chic */
AddStFileToMru(st)
char * st;
{
	int ibstDel;

	Assert(vhsttbOpen != hNil);

	/* look for existing entry */
	if ((ibstDel = IbstFindSt(vhsttbOpen, st)) == 0)
		return; /* already at head of list */

	/* don't let the cache exceed max entries */
	if (ibstDel == iNil && (*vhsttbOpen)->ibstMac == ibstMaxFileCache)
		ibstDel = ibstMaxFileCache-1;

	/* delete old entry */
	if (ibstDel != iNil)
		DeleteIFromSttb(vhsttbOpen, ibstDel);

	/* add new entry */
	if (!FInsStInSttb(vhsttbOpen, 0, st))
		return;

	vfFileCacheDirty = fTrue; /* so menu gets updated */
	Assert ((*vhsttbOpen)->ibstMac <= ibstMaxFileCache);
}


csconst char stDocument[] = StSharedKey("Document",Document);
csconst char stFormLetter[] = StSharedKey("Form Letters",FormLetters);
csconst char stTemplate[] = StSharedKey("Template",Template);

/* G E T  D O C  S T */
/*  Get name of a doc into st.  St must have sufficient room (up to
	ichMaxFile) for name.  If doc is the Untitled document, returns
	"Untitled" unless gdsoNoUntitled is specified (in which case you get
	empty st).  Otherwise what you get depends on gdso:
		gdsoFullPath	C:\DOC\FOO.DOC
	gdsoRelative	FOO.DOC (if c:\doc is cur directory) or
					C:\DOC\FOO.DOC (if c:\doc not current directory)
		gdsoNotRelDot   turns off gdsoRelative if doc is a dot
	gdsoShortName	FOO.DOC
	Returns an empty st if doc does not have a name (true for sub docs!).
*/

/* %%Function:GetDocSt %%Owner:chic */
GetDocSt(doc, st, gdso)
int doc;
char *st;
int gdso;
{
	extern struct DOD **mpdochdod[];
	extern int docMac;
	extern char stNormalDot[];
	struct DOD **hdod;
	int fn;

	st[0] = 0;

	Assert (doc < docMac && doc >= docNil);

	/* Only dkDoc's and dkDot's have names */
	if ((hdod = mpdochdod [doc]) == hNil || !((*hdod)->dk & (dkDoc | dkDot)))
		return;

	Assert(doc >= docMinNormal);

	if ((*hdod)->fDot && (gdso & gdsoNotRelDot))
		gdso &= ~gdsoRelative;

	if ((fn = (*hdod)->fn) != fnNil)
		/* existing file */
		{
		CopySt(PfcbFn(fn)->stFile, st);
		}

	else  
		switch ((*hdod)->udt)
			{
		case udtMacroEdit:
			AssertDo(FGetStMacro(doc, st));
			return;

		case udtGlobalDot:
			AssertDo(FNormalizeStFile(stNormalDot, st, nfoDot));
			break;

		case udtDocument:
			CopyCsSt(stDocument, st);
			goto LAddInstance;

		case udtFormLetter:
			CopyCsSt(stFormLetter, st);
			goto LAddInstance;

		case udtTemplate:
			CopyCsSt(stTemplate, st);

LAddInstance:
			if (gdso & gdsoNoUntitled)
				{
				*st = 0;
				return;
				}
			else
				{
				int i = max((*hdod)->iInstance, 1);
				Assert(*st);
				/* punt >= 100 case */
				if (i >= 10)
					st[++(*st)] = '0'+ ((i/10)%10);
				st[++(*st)] = '0' + (i%10);
				return;
				}

		default:
			/* doc has no name */
			return;
			}

	if (gdso & gdsoRelative)
		StFileAbsToStFileRel(st, st);

	else  if (gdso & gdsoShortName)
		{
		struct FNS fns;
		StFileToPfns(st, &fns);
		CopySt(fns.stShortName, st);
		StStAppend(st, fns.stExtension);
		}
}


/* %%Function:SetAppCaptionFromHwnd %%Owner:chic */
SetAppCaptionFromHwnd(hwnd)
HWND hwnd;
{
	extern CHAR szApp[];

	int cch;
	CHAR rgch[ichMaxFile+32];

	Assert(CchSz(szAppTitle) < 32-3);     /* Ensure less than rgch buffer above */
	cch = CchCopySz( szAppTitle, rgch );
	cch += CchCopySz(SzSharedKey(" - ",OneDash), &rgch[cch] );
	GetWindowText( hwnd, (LPSTR)&rgch[cch], ichMaxFile+32-cch );
	SetWindowText( vhwndApp, (LPSTR)rgch );
}


/*  Format of vhsttbWnd:
		Each entry corresponds to one mw.  Every mw has an entry.  They
	are in alpha order by filename (relative) then in instance order
	(as determined by wwDisp chain order).  Entry contains text as
	displayed in caption (relative name[:inst]) plus a single char
	which is the mw.

	FUTURE: could use cbExtra of sttbWnd instead of having mw as part
		of st itself.
*/


/* M W  F R O M  I B S T  W N D */
/* %%Function:MwFromIbstWnd  %%Owner:chic */
MwFromIbstWnd (ibst)
int ibst;
{
	CHAR *pst;

	Assert( ibst < (*vhsttbWnd)->ibstMac );
	pst = PstFromSttb( vhsttbWnd, ibst );
	return((int)*(pst + *pst));
}


/* I B S T  W N D  F I N D  M W */
/* %%Function:IbstWndFindMw  %%Owner:chic */
IbstWndFindMw (mw)
int mw;
{
	int ibstMac = (*vhsttbWnd)->ibstMac;
	int ibst;

	for (ibst = 0; ibst < ibstMac; ibst++)
		if (MwFromIbstWnd(ibst) == mw) /* found it */
			return ibst;

	return ibstNil;
}


/* U P D A T E  T I T L E S */
/*  Updates all mw captions, vhsttbWnd, and window menu.
*/

/* %%Function:UpdateTitles %%Owner:peterj */
UpdateTitles()
{
	int doc;
	extern int docMac;

	for (doc = docMinNormal; doc < docMac; doc++)
		if (mpdochdod[doc])
			UpdateTitlesForDoc (doc);

	/* Update the macro icon bar too! */
	if (vhmes != hNil)
		{
		int imeiSav;

		imeiSav = (*vhmes)->imeiCur;
		(*vhmes)->imeiCur = -1;
		SetImeiCur(imeiSav);
		}
}


/* U P D A T E  T I T L E S  F O R  D O C */
/*  Updates mw captions and vhsttbWnd for doc.
*/

/* %%Function:UpdateTitlesForDoc %%Owner:peterj */
UpdateTitlesForDoc(doc)
int doc;
{
	int WCompSt();
	HWND hwnd;
	int imwMac, imw, ibst, ibstDel, mw, rgmw[mwMax];
	BOOL fInsertOK = fTrue;
	CHAR st [ichMaxFile+5];

	if (doc < docMinNormal || PdodDoc(doc)->fShort)
		return;

	AssertH(vhsttbWnd);
	BuildRgmwForDoc (doc, rgmw, &imwMac);

	/* first delete the old titles...*/
	for (imw = 0; imw < imwMac; imw++)
	/* delete old window caption (if any) */
		if ((ibstDel = IbstWndFindMw (rgmw[imw])) != ibstNil)
			DeleteIFromSttb (vhsttbWnd, ibstDel);

	/* then add the new ones... */
	for (imw = 0; imw < imwMac && fInsertOK; imw++)
		{

		mw = rgmw [imw];

	/* generate window caption */
		GetStWndForMw (doc, mw, st, rgmw, imwMac);

	/* add new caption to STTB */
		if (!imw)
		/* first is inserted in alpha order, subsequent ones all follow */
			FSearchSttb (vhsttbWnd, st, &ibst, WCompSt);
		/* nothing to do if this fails--window just does not appear on list! */
		if (fInsertOK = FInsStInSttb (vhsttbWnd, ibst, st))
			ibst++;

	/* change windows caption */
		st[0]--; /* get rid of mw at tail of st */
		StToSzInPlace (st);
		SetWindowText((hwnd = (*mpmwhmwd[mw])->hwnd), (LPSTR)st);

	/* Change app's caption */
		if (vpref.fZoomMwd && mw == mwCur)
			SetAppCaptionFromHwnd (hwnd);

		}

	vfWndCacheDirty = fTrue;
}


/* B U I L D  R G M W  F O R  D O C */
/* %%Function:BuildRgmwForDoc  %%Owner:chic */
BuildRgmwForDoc (doc, rgmw, pimwMac)
int doc, rgmw[], *pimwMac;
{
	int imw = 0;
	int ww = wwNil;
	int imwT, mw;

	while ((ww = WwDisp(doc, ww, fFalse)) != wwNil)
		{
		mw = PwwdWw(ww)->mw;
		for (imwT = 0; imwT < imw; imwT++)
			if (mw == rgmw[imwT])
				break;
		if (imwT >= imw)
			rgmw[imw++] = mw;
		}
	*pimwMac = imw;
}


/* G E T  S T  W N D  F O R  M W */
/* %%Function:GetStWndForMw  %%Owner:chic */
GetStWndForMw (doc, mw, st, rgmw, imwMac)
int doc, mw;
CHAR *st;
int rgmw[], imwMac;

{

	if (doc == docNil)
		doc = PmwdMw(mw)->doc;

	Assert(doc != docNil);
	GetDocSt(doc, st, gdsoRelative);

	if (imwMac > 1)
		/* compute imw of mw and append :<imw+1> */
		{
		int imw, imwP1;

		st[++(*st)] = chColon;
		for (imw = 0; rgmw[imw] != mw; imw++)
			Assert(imw < imwMac);
		imwP1 = imw+1;
		Assert(imwP1 < 100);
		if (imwP1 >= 10)
			{
			st[++(*st)] = '0' + (imwP1/10);
			imwP1 = imwP1 % 10;
			}
		st[++(*st)] = '0' + imwP1;
		}

	Assert(mw < 255);
	st[++(*st)] = (CHAR)mw;
}


/*
*  Given the name of a file, DocFromSt returns the doc number for
*  that file if it already exists as an opus document, docNil otherwise.
*/
/* %%Function:DocFromSt %%Owner:chic */
DocFromSt(st)
char *st;
{
	int doc;
	CHAR rgch[ichMaxFile];
	extern int docMac;

	if (st[0] == 0)
		return docNil;
	for (doc = docMinNormal; doc < docMac; doc++)
		{
		GetDocSt(doc, rgch, gdsoFullPath);
		if (rgch[0] && FEqNcSt(st, rgch))
			return doc;
		}
	return docNil;
}




/* %%Function:CloseDocWin %%Owner:chic */
CloseDocWin(hwnd, ac)
HWND hwnd;
int ac;
{
	HWND hwndNew;

	/* If in Preview, Close really means nuke preview! */
	if (vlm == lmPreview)
		{
		FExecCmd(bcmPrintPreview);
		return;
		}

	/* don't close window if there is msg box up, else focus of msg box
	is returned to destroyed window */
	if (vcInMessageBox > 0) 
		return;

	EnsureFocusInPane();

	/* terminate Block or Extend or dyadic move mode if on */
	SpecialSelModeEnd();

	if (vpref.fZoomMwd && 
			(hwndNew = GetNextWindow(hwnd, GW_HWNDNEXT)) != NULL)
		{
		ZoomWnd(hwndNew);
		}

	if (FCloseMw(MwFromHwndMw(hwnd), ac) && vfRecording)
		RecordDocClose();

	/*  updates window list and all MWD titles */
	UpdateTitles();
}


/* F  C L O S E  M W */
/* handles WM_CLOSE message send to MWD window */
/* %%Function:FCloseMw %%Owner:chic */
FCloseMw(mw, ac)
int mw, ac;
{
	extern BOOL vfRecording;

	struct DOD * pdod;
	struct MWD ** hmwd;
	int wwUpper;
	int wk;
	int doc;
	int imei;
	BOOL fRet;
	BOOL fRecordingSav;


	/* don't close window if there is msg box up, else focus of msg box
	is returned to destroyed window */
	if (vcInMessageBox > 0) 
		return fFalse;

	InhibitRecorder(&fRecordingSav, fTrue);


	fRet = fTrue;

	hmwd = mpmwhmwd[mw];
	Assert(hmwd != hNil);

	wwUpper = (*hmwd)->wwUpper;

	wk = PwwdWw(wwUpper)->wk;

	switch (wk)
		{
	case wkPage:
	case wkOutline:
	case wkDoc:
		doc = DocBaseWw(wwUpper);
		Assert(PdodDoc(doc)->fMother);

		/* Is this a great condition, or what! */
		if ((!(mpdochdod[doc] == hNil || /* no dod... */

				/* not a document or template... */
		(!(pdod = PdodDoc(doc))->fDoc && !pdod->fDot) ||

				/* not in a window... */
		pdod->wwDisp == wwNil ||

				/* no file and not a new document... */
		(pdod->fn == fnNil && !pdod->fUserDoc) ||

				/* new but empty document... */
		(pdod->udt == udtDocument && 
				CpMacDocEdit(doc) == 0 &&
				pdod->docHdr == docNil && 
				(DiDirtyDoc(doc) & dimStsh) == 0)) && 

				/* Run the auto macro... */
		CmdRunAtmOnDoc(atmClose, doc) == cmdCancelled)
				|| !FConfirmSaveOnCloseDoc(doc, ac))
			{
			fRet = fFalse;
			goto LReturn;
			}
/* need to check for non-zero mpmwhmwd slot since FConfirmSave... can
   cause it to be cleared if the document has to be closed because hplcbte
   couldn't be expanded. */
		if (mpmwhmwd[mw] != 0)
			CloseMwPanes(mw);
		break;

	case wkMacro:
		imei = ImeiFromMw(mw);
		Assert(imei != iNil);
		if (!FCloseEdMacro(imei, ac))
			{
			fRet = fFalse;
			goto LReturn;
			}
		break;

#ifdef DEBUG
	default:
		Assert(fFalse);
#endif
		}

LReturn:
	InhibitRecorder(&fRecordingSav, fFalse);

	return fRet;
}


/* %%Function:CloseMwPanes %%Owner:chic */
CloseMwPanes(mw)
int mw;
{
	struct MWD * pmwd;
	int wwLower, wwUpper;

	pmwd = PmwdMw(mw);
	wwUpper = pmwd->wwUpper;
	wwLower = pmwd->wwLower;

	if (wwLower != wwNil)
		CloseWw(wwLower);

	CloseWw(wwUpper);
}



/* %%Function:FConfirmSaveOnCloseDoc %%Owner:chic */
BOOL FConfirmSaveOnCloseDoc(doc, ac)
int doc;
int ac;
{
	int docDot; 
	struct DOD  *pdod;

	pdod = PdodMother(doc);
	docDot = (pdod->fDoc && pdod->docDot != docNil) ? pdod->docDot : docNil;
	if (!FConfirmSaveDoc( doc, fFalse/* fForce */, ac ))
		return fFalse;

	if (docDot != docNil && WwDisp(docDot, wwNil, fFalse) == wwNil)
		if (!FConfirmSaveDoc( docDot, fFalse, ac ))
			return fFalse;

	return fTrue;
}


/* D i s p o s e  M w d */
/* Called from CloseWw to throw away an MWD window */

/* %%Function:DisposeMwd %%Owner:chic */
DisposeMwd(mw)
int mw;
{
	struct MWD ** hmwd = mpmwhmwd[mw];
	struct MWD * pmwd;
	int wwUpper, wwLower;
	int ibstDel;
	HWND hwnd;
	BOOL fSysMenu;

	Assert(hmwd != hNil);
	pmwd = *hmwd;
	wwUpper = pmwd->wwUpper;
	wwLower = pmwd->wwLower;
	hwnd = pmwd->hwnd;
	Assert(wwUpper != wwNil);

/* all child windows will be automatically destroyed when parent
	is destroyed, the ruler and icon bar have to be explicitly
	destroyed so as to clean up objects used by ruler and icon bar */

	if (pmwd->hwndRuler)
		FDestroyRuler(hmwd);

	/* remove system menu from menubar before it is destroyed */
	if (vfSysMenu && 
			GetSubMenu(vhMenu, 0) == GetSystemMenu(hwnd, fFalse))
		FRemoveSystemMenu();

	DestroyWindow(hwnd);

	if ((ibstDel = IbstWndFindMw(mw)) != ibstNil)
		DeleteIFromSttb (vhsttbWnd, ibstDel);

	FreeCls(clsMWD, mw);
}


/* F   C O N F I R M   S A V E   D O C */
/* ask the user whether to save this doc; return fFalse if he hit Cancel
	or if an attempted save failed.
	if fForce then prompt even if the doc is referenced elsewhere.
*/
/* %%Function:FConfirmSaveDoc %%Owner:chic */
FConfirmSaveDoc( doc, fForce, ac )
int doc;
BOOL fForce;
uns ac;
{
	struct DOD * pdod = PdodDoc(doc);
	struct MWD * pmwd;
	int ww, di;
	BOOL fSave;

	/* if CBT is exiting, don't bother asking - we never save
		anything done in CBT */

	if (!vhwndCBT && vrf.fRanCBT)
		return fTrue;

	fSave = fFalse;

	Assert(doc != docNil);

	if (mpdochdod[doc] == hNil || !(di = DiDirtyDoc(doc)))
		return fTrue;

	Assert(!pdod->fSDoc && !pdod->fDispHdr);

	if (!fForce && (ww = WwDisp(doc, wwNil, fFalse)) != wwNil)
		{
		pmwd = PmwdWw(ww);

		while (ww != wwNil)
			{
			/* don't bother until closing the last one of 
				the multiple instances */
			if (PmwdWw(ww) != pmwd)
				return fTrue;
			ww = WwDisp(doc, ww, fFalse);
			}
		}

	Assert(pdod->fDoc || pdod->fDot);

	switch (ac)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		return fFalse;
#endif

	case acNoSave:
		break;

	case acSaveNoQuery:
		fSave = fTrue;
		break;

	case acAutoSave:
		if (doc == DocMother(selCur.doc))
			{
			/* already asked to Save Now? for the current doc */
			fSave = fTrue;
			break;
			}
		/* fall through */
	case acSaveAll:
	case acQuerySave:
		switch (IdMessageBoxMstRgwMb((doc == docGlobalDot) ? 
				mstSaveChangesGdt : mstSaveChangesDoc, &doc, mbQuerySave))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return fFalse;
#endif
		case IDCANCEL:
			return fFalse;

		case IDNO:
			break;

		case IDYES:
			fSave = fTrue;
			/* break; */
			}
		/* break; */
		}

	if (fSave)
		{
		DoSaveDoc(doc, ac==acAutoSave?fFalse:vpref.fPromptSI);
		return (mpdochdod[doc] == hNil || DiDirtyDoc(doc) == 0
					|| ac == acSaveAll);
		}

	return fTrue;
}


/* C L O S E   W W  */
/* ww must be a fDoc ww.
CloseWw performs the following actions:
	1) it releases the hplcedl of the WWD for the ww.
	2) it frees the hibList of the WWD for the ww.
	1) freeing the Mwwd if necessary (no more splits)
	3) it removes the ww from its document's list of displaying windows
	4) it calls DisposeDoc to kill the document for the window if no other
		windows refer to it.
	5) frees the window's WWD
	6) clobbers the UNDO doc and clears the screen caches.
NOTE: caller is responsible for:
	2) selecting a new ww.
*/
/* %%Function:CloseWw %%Owner:chic */
CloseWw(ww)
int ww;
{
	int wwDisp;
	int mw, idr;
	int doc = DocBaseWw(ww);
	struct CA caT;
	struct WWD *pwwdT;
	struct DOD *pdodMom;
	int wwOther = wwNil;
	unsigned char *pww, *pwwMac;
	int fSplit = fFalse;

/* Note: avoid holding onto pointers because heap often get moved */
	Assert(ww >= 0 && ww < wwMac);
	mw = PwwdWw(ww)->mw;

	if (ww == wwCur)
		TurnOffSel(&selCur);

	DestroyWindow(PwwdWw(ww)->hwnd);	  /* HEAP MOVES! */

/* Free up the space that was used by this window's EDLs */
	FreeDrs(HwwdWw(ww), 0);

/* must do this before MWD is blown away */
	if (PwwdWw(ww)->fOutline)
		UnhookOutlineKmpHmwdWw( mpmwhmwd[mw], ww );

/* Break off split wwd */
	if (fSplit = PmwdWw(ww)->fSplit)
		{
		wwOther = WwOther(ww);
		PmwdMw(mw)->fSplit = fFalse;
		}

	if (vfTableKeymap)
		UnhookTableKeymap();

	/* Short docs have a keymap we need to dispose of... */
	if (PdodDoc(doc)->fShort)
		{
		RemoveKmp(PwwdWw(ww)->hkmpCur);
		}

/* since this window is closing, we must remove its WWD from the document's
	list of displayed documents. */
	RemoveWwDisp(doc, ww);
	wwDisp = WwDisp(doc, wwNil, fFalse);    /* 1st other window for doc */

/* if doc is displayed in no more windows: 1) terminate any dde activity
	for this doc, 2) assure we won't try to offer it in a link */
	if (wwDisp == wwNil && dclMac > 1)
		{
		TerminateDdeDoc (doc);
		if (vsab.doc == doc)
			vsab.doc = docNil;
		}

	if (PdodDoc(doc)->dk == dkDispHdr)
		FCleanUpHdr(DocMother(doc), fFalse, fTrue);
	else  if (!fSplit)
		DisposeDoc(doc);

/* if we are really done with this document, this will try to free up its
	file. */
	MarkAllReferencedFn();
	DeleteNonReferencedFns(0);

	if (ww == wwCur)
		vhwndMsgBoxParent = NULL;
/* free this wwd */
	if (!fSplit)
		{
		DisposeMwd(mw);
		PwwdWw(ww)->mw = wwNil;	/* notify FCleanUpHdr */
		}
	FreeCls(clsWWD, ww);

	if (mpwwhwwd[vsccAbove.ww] == hNil)
		vsccAbove.ww = wwNil;
	if (mpwwhwwd[vpri.wwInit] == hNil)
		vpri.wwInit = wwNil;

/* bug fix - in case we are closing down all windows of a file, some of which
may not be the current window. */
	if (ww == wwCur)
		PickNewWw();
	else  if (wwOther != wwNil && PmwdMw(mw)->wwActive == ww)
		{
		PmwdMw(mw)->wwActive = wwOther;
		}

/* kill references to this ww in the mwd */
	if (wwOther != wwNil)
		for (pww = PmwdMw(mw)->rgww, pwwMac = pww+2; pww < pwwMac; pww++)
			if (*pww == ww)
				*pww = wwNil;

/* Following comment is preserved verbatim */
	/* Really stomp this sucker! */
	if (mpdochdod[doc] == hNil &&
			(vuab.ca.doc == doc || vuab.caMove.doc == doc))
		SetUndoNil();
/* if undo referred to this window, point it at another */
	else
		{
		if (vuab.wwBefore == ww)
			vuab.wwBefore = wwDisp;
		if (vuab.wwAfter == ww)
			vuab.wwAfter = wwDisp;
		if (vuab.wwBefore == wwNil /* OK to have wwAfter be wwNil */)
			SetUndoNil();/* redo cannot handle wwBefore being wwNil */
		}
	ClearSccs();
}



/* %%Function:ZoomWnd %%Owner:chic */
ZoomWnd(hwnd)
HWND hwnd;
{
	/* There is no rc information to ZoomWnd because there is a
	standard way to figure it out and redudent to spend codes
	in every callers.
	The app's caption is also set up in ZoomWnd for the same
	reason above. */
	if (hwnd == vhwndZoom)
		return;

	vhwndZoom = hwnd;
/* save away the rc info before it is zoomed */
	GetWindowRect( hwnd, (LPRECT)&vrcUnZoom );
	ScreenToClient( vhwndDeskTop, (LPPOINT)&vrcUnZoom.xpLeft );
	ScreenToClient( vhwndDeskTop, (LPPOINT)&vrcUnZoom.xpRight );

	DoZoom( hwnd );

/* set the title to the app's caption */
	SetAppCaptionFromHwnd( hwnd );
}


/* %%Function:DoZoom %%Owner:chic */
DoZoom(hwndMw)
HWND hwndMw;
{
	struct RC rcZoom;
	LONG ws;


/* use full size of desktop to be the new size */
	GetMwPrc( mwNil/* don't care if zooming */, &rcZoom );
/* if the desk top has shrunk too much to support a minimal sized doc window
	then call CmdArrangeWnd to put the doc in its own window. */
	if (vpref.fZoomMwd)
		{
		if (rcZoom.ypBottom - rcZoom.ypTop < vsci.dypScrlBar + vsci.dypBorder + 2 * vsci.dypWwMin +
				vsci.dypScrlBar + 1)
			{
			CmdRestoreWnd(0);
			return;
			}
		}
/* take away the size box style for zoomed window */
	ws = GetWindowLong( hwndMw, GWL_STYLE ) & ~WS_SIZEBOX;
	SetWindowLong( hwndMw, GWL_STYLE, ws );

/* adjust the rc to be it's client area size */
	AdjustWindowRect( (LPRECT)&rcZoom, ws, FALSE );
	MoveWindowRc( hwndMw, &rcZoom, TRUE /* fRepaint */ );
}


/* M O V E  P A N E */
/* Adjust the pane area to the rectangle allotted to it.  The pane
	area rectangle includes iconbar and ruler if they are present.
	This routine moves them to their appropriate places as children
	of the MWD and moves the pane window as well.

	mp:  mpdTopDown, mpdBottomUp, or mpdPaneOnly
*/

/* %%Function:MovePane %%Owner:chic */
MovePane( ww, prc, mpd )
int ww;
struct RC *prc;
int mpd;   /* Move Pane Direction */
{
	struct RC rcIconBar;
	struct RC rcRuler;
	int mw = PwwdWw(ww)->mw;
	HWND hwndRuler = NULL;
	HWND hwndIconBar;
	int dxpBorder = vsci.dxpBorder;
	int dypBorder = vsci.dypBorder;

	Assert(mw);
	if ((hwndIconBar = PwwdWw(ww)->hwndIconBar) != NULL)
		{
		rcIconBar.ypTop = prc->ypTop - dypBorder;
		rcIconBar.xpLeft = prc->xpLeft - dxpBorder;
		rcIconBar.xpRight = prc->xpRight + dxpBorder;
		rcIconBar.ypBottom = rcIconBar.ypTop + vsci.dypIconBar + dypBorder;
		prc->ypTop += vsci.dypIconBar;
		if (mpd == mpdTopDown)
			MoveWindowRc(hwndIconBar, &rcIconBar, fTrue);
		}

	if (PmwdMw(mw)->wwRuler == ww)
		{
		hwndRuler = PmwdMw(mw)->hwndRuler;
		rcRuler.ypTop = prc->ypTop - dypBorder;
		rcRuler.xpLeft = prc->xpLeft - dxpBorder;
		rcRuler.xpRight = prc->xpRight + dxpBorder;
		rcRuler.ypBottom = rcRuler.ypTop + vsci.dypRuler;
		prc->ypTop += vsci.dypRuler - dypBorder;
		if (mpd == mpdTopDown)
			MoveWindowRc(hwndRuler, &rcRuler, fTrue);
		}

	MoveWindowRc(PwwdWw(ww)->hwnd, prc, fTrue);
	if (mpd == mpdBottomUp)
		{
		if (hwndRuler)
			MoveWindowRc(hwndRuler, &rcRuler, fTrue);
		if (hwndIconBar)
			MoveWindowRc(hwndIconBar, &rcIconBar, fTrue);
		}
}


/* A D J U S T  P A N E */
/* Adjusts the pane window up or down to make room for or close up
	after removal of an iconbar or ruler.
*/

/* %%Function:AdjustPane %%Owner:chic */
AdjustPane( hwwd, dyp )
struct WWD **hwwd;
int dyp; /* > 0 means shrink pane */
{
	struct MWD **hmwd = mpmwhmwd[(*hwwd)->mw];

	FAbortNewestCmg(cmgAdjPane, fTrue/*fLoad*/, fFalse);

#ifdef REFERENCE
/* this does not handle show/hide split box according to window size */
	GetWindowRect((*hwwd)->hwnd, (LPRECT) &rcPane);
	ScreenToClient(PmwdMw(mw)->hwnd, (LPPOINT) &rcPane.ptTopLeft);
	ScreenToClient(PmwdMw(mw)->hwnd, (LPPOINT) &rcPane.ptBottomRight);
	rcPane.ypTop += dyp;
	MoveWindowRc((*hwwd)->hwnd, &rcPane, fTrue);
#endif
	AdjustDocAreaPrc(hmwd, 0 /*ddxp*/, -dyp);

}


/* A d j u s t  M w d  A r e a */

/* %%Function:AdjustMwdArea %%Owner:chic */
AdjustMwdArea(hmwd, ddxp, ddyp, fDocAreaFirst)
struct MWD **hmwd;
int ddxp, ddyp;
BOOL fDocAreaFirst;
{
	HWND hwndHorz;
	struct RC rcHorzScroll;
	struct RC rcSizeBox;
	struct RC rcDocArea;

	Assert( hmwd != hNil );

	GetDocAreaPrc( *hmwd, &rcDocArea);
	GetHorzBarRcs( *hmwd, rcDocArea, &rcHorzScroll, &rcSizeBox );

	/* Heap Movement */
	if (fDocAreaFirst && ddyp < 0)
		AdjustDocAreaPrc( hmwd, ddxp, ddyp );

	if ((*hmwd)->hwndSizeBox)
		MoveWindowRc( (*hmwd)->hwndSizeBox, &rcSizeBox, fTrue );

	if ((hwndHorz = (*hmwd)->hwndHScroll) != NULL)
		MoveWindowRc( hwndHorz, &rcHorzScroll, fTrue );

	/* Heap Movement */
	if (!fDocAreaFirst || ddyp >= 0)
		AdjustDocAreaPrc( hmwd, ddxp, ddyp );
}



/* G E T   H O R Z   B A R   R C S */
/* Get rectangles for: horizontal scroll bar, size box (if next to horz scrl bar) */
/* Size box rc will be invalid if no vertical scroll bar */

/* %%Function:GetHorzBarRcs %%Owner:chic */
GetHorzBarRcs( pmwd, rcDocArea, prcHorzScroll, prcSizeBox )
struct MWD *pmwd;
struct RC rcDocArea;
struct RC *prcHorzScroll, *prcSizeBox;
{
	prcHorzScroll->xpLeft = rcDocArea.xpLeft - vsci.dxpBorder;

	prcHorzScroll->xpRight =  rcDocArea.xpRight - (pmwd->fVertScrollBar ?
			vsci.dxpScrlBar : 0) + 2 * vsci.dxpBorder;
	prcSizeBox->xpLeft = prcHorzScroll->xpRight - vsci.dxpBorder;

	prcSizeBox->xpRight = rcDocArea.xpRight + vsci.dxpBorder;

	prcHorzScroll->ypTop = prcSizeBox->ypTop = rcDocArea.ypBottom;

	prcSizeBox->ypBottom = prcHorzScroll->ypBottom =
			rcDocArea.ypBottom + vsci.dypScrlBar;
}



/* %%Function:UnZoomWnd %%Owner:chic */
UnZoomWnd(hwnd , prc)
HWND hwnd;
struct RC *prc;
{
	extern HWND vhwndZoom;
	/* There is a need to pass in a rc to UnZoomWnd because although
	most of the time we use vrcUnZoom, there are times we want to
	use a different rc, e.g. when we are creating a window when
	we already have a zoomed window, we want to create the new
	window in zoomed size first, then unzoom the old window, there-
	fore we have to save away the old unzoom size in another rc to
	be passed to UnZoomWnd. */

	LONG ws = GetWindowLong( hwnd, GWL_STYLE ) | WS_SIZEBOX;
	int mw = MwFromHwndMw(hwnd);
	struct MWD **hmwd;

	if (prc->xpLeft == 0 && prc->xpRight == 0 && prc->ypTop == 0 && prc->ypBottom == 0)
		{ /* i.e. rc wasn't set up yet */
		GetMwPrc(mw, prc);
		}
/* Restore the sizebox style for nonzoomed window */
	SetWindowLong( hwnd, GWL_STYLE, ws );
	MoveWindowRc( hwnd, prc, TRUE /* fRepaint */ );
/* Don't always want to reset the app's caption because this routine
	is also called whenever we want to restore a zoomed window to its
	original size while another window will be brought up zoomed. */
	if (vhwndZoom == hwnd)
		vhwndZoom = hNil;

}


int CchCurSzPath(szPath, wDrive)
CHAR *szPath;
int	wDrive;
{
 	return (
#ifdef	WIN
				/* FUTURE : at present, the fix has been made for AS400 
								file servers drivers only.
								should be generalized in future. */
				vfAs400 ? CchCurSzPathAs400(szPath, wDrive) :
#endif /* WIN */
				CchCurSzPathNat (szPath, wDrive)
			);
}

#ifdef	WIN
/* Function:CchCurSzPathAs400 %%Owner:krishnam
   Windows call to get the current path name. 
	returns length of path including the trailing NULL. */
int	CchCurSzPathAs400(szPath, wDrive)
CHAR	*szPath;
int	wDrive;
{
	OFSTRUCT	ofstruct;
	CHAR	szTemp[8];

	Assert(wDrive >= 0);
	if (wDrive)
		{
		 szTemp[0]=wDrive-1+'A';
		 szTemp[1]=':';
		 szTemp[2]='\0';
		}
	else
		szTemp[0]='\0';

	SzSzAppend(szTemp, SzShared("foo"));
	if (OpenFile((LPSTR)szTemp, (LPOFSTRUCT)&ofstruct, OF_PARSE) == -1)
			{
			*szPath='\0';
			return 1;
			}

   /* take out the "foo" */
	CchCopySz(ofstruct.szPathName, szPath);
	StripSzFileName(szPath);

	return CchSz(szPath);
}


/* %%Function:StripSzFileName %%Owner:krishnam
   strips szPath of the trailing file name. */
StripSzFileName(szPath)
CHAR szPath[];
{
 	CHAR *pchFile, *pchMac, *pchLast;

	pchMac = szPath+CchSz(szPath)-1;
	pchFile = szPath;
	for (pchLast = pchFile; pchFile < pchMac; ++pchFile)
		{
		if (FPathCh(*pchFile))
			pchLast = pchFile+1;
		}

	*pchLast = '\0';
}


/* %%Function:FPathCh %%Owner:krishnam
   to recognize a special character in path */
BOOL	FPathCh(ch)
int ch;												 
{
	return(ch == ':' || ch == '\\' || ch == '/');
}

#endif /* WIN */

