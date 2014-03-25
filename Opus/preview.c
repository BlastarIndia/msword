/* Print Preview	 Ray Gram 8/85 */
/* Converted to Windows  Dave Bourne 2/86 */
/* Resurrected           Dave Bourne 5/88 */

#define NOTBITMAP
#define NOCLIPBOARD
#define NOCREATESTRUCT
#define NOMEMMGR
#define NOOPENFILE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOWH
#define NOGDICAPMASKS
#define NOICON
#define NODRAWTEXT
/*
#define NOFONT
*/
#define NOMB
#define NOWNDCLASS

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "doslib.h"
#include "props.h"
#include "format.h"
#include "layout.h"
#include "border.h"
#include "file.h"
#include "heap.h"
#include "ch.h"
#include "ibdefs.h"
#include "inter.h"
#include "prm.h"
#include "sel.h"
#include "preview.h"
#include "print.h"
#include "screen.h"
#include "splitter.h"
#define USEBCM
#include "opuscmd.h"
#define PREVIEWC
#include "keys.h"
#include "message.h"
#include "debug.h"
#define REVMARKING
#include "compare.h"
#include "resource.h"

#include "help.h"
#include "rerr.h"
#include "error.h"
#include "cmdtbl.h"
#include "recorder.h"

/* Preview globals */
PVS vpvs;               /* global PreViewState */
HWND vhwndPgPrvw;       /* Page preview window handle */
HWND vhwndPgPrvwScrl;   /* scroll bar */
HWND vhwndPrvwIconBar;  /* preview icon bar */

/* Externals */
extern char szClsPgPrvw[];
extern char szClsRSB[];
extern struct ITR       vitr;
extern BOOL		vfElFunc;
extern int              fElActive;
extern int		vcElParams;
extern int              vlm;
extern int              vflm;
extern int              wwCur;
extern int              vfDrawPics;
extern struct FTI       vfti;
extern HBRUSH           vhbrGray, vhbrLtGray, vhbrDkGray, vhbrWhite;
extern int              vfPrvwDisp;
extern HBRUSH           vhbrGray;
extern struct CA        caPara;
extern struct CA        caTap;
extern struct SEL       selCur;
extern struct PREF      vpref;
extern struct TCC       vtcc;
extern struct TCXS      vtcxs;
extern struct FLI       vfli;
extern struct SCI       vsci;
extern struct PRI       vpri;
extern struct MERR      vmerr;
extern struct PAP       vpapFetch;
extern struct TAP       vtapFetch;
extern struct SEP       vsepFetch;
extern struct WWD       **hwwdCur;
extern struct SELS      vselsMom;
extern struct FMTSS     vfmtss;
extern struct CBTSAV    **vhcbtsav;
extern HCURSOR          vhcArrow, vhcOtlCross, vhcPrvwCross;
extern HANDLE           vhInstance;
extern HWND             vhwndStatLine;
extern HWND             vhwndRibbon;
extern HWND             vhwndDeskTop;
extern HWND             vhwndCBT;
extern HWND             vhwndApp;
extern HWND             vhwndAppIconBar;
extern int              vfIconBarMode;
extern BOOL             vfHelp;
extern long             vcmsecHelp;
extern int		vfRecording;

LONG FAR PASCAL PgPrvwWndProc(HWND, unsigned, WORD, LONG);
DisplayLrTable();

/* MUST match declaration in prvw2.c */
csconst CHAR szCsPrvwOnePg[] = SzKey("One P&age",PrvwOnePg);
csconst CHAR szCsPrvwTwoPg[] = SzKey("Two P&ages",PrvwTwoPg);

csconst CHAR szCsPage[] = SzKey("Page ",PageSpace);
csconst CHAR szCsPages[] = SzKey("Pages ",PagesSpace);
csconst CHAR szCsPrvwFont[] = SzKey("Prev",Prev);

/* %%Function:CmdTurnOnPrvw %%Owner:davidbo */
CMD CmdTurnOnPrvw()
{
	CMD cmd;
	int fPrintableArea;
	struct RC rcPrvw, rcIcon;

	/* already on...ignore */
	if (vlm == lmPreview)
		return cmdOK;

	if (!FPrinterOK())
		{
		ErrorEid(eidNoPrinter, "");
		return cmdError;
		}

	/* make the cursors available */
	if (!vhcPrvwCross && !FAssureIrcds(ircdsPrvwCross))
		{
		SetErrorMat(matMem);
		return cmdError;
		}

	Assert(selCur.doc != docNil);
	vpvs.docPrvw = DocMother(selCur.doc);
	/* use local because you can't pass the address of a bitfield! */
	fPrintableArea = fTrue;
	if (!FCheckPageAndMargins(vpvs.docPrvw, fFalse, fFalse, &fPrintableArea))
		return cmdCancelled;
	vpvs.fPrintableArea = fPrintableArea;

	if (vpref.fDraftView && CmdDraftView(NULL) != cmdOK)
		return cmdError;

	/* save docHdrDisp back to docHdr */
	Assert(wwCur != wwNil);
	if (PwwdWw(wwCur)->fHdr)
		FSaveHeader(selCur.doc, fFalse);

	if ((*hwwdCur)->wk & wkSDoc)
		{
		int fRecSav, wwNew = WwOther(wwCur);
		Assert(wwNew != wwNil);
		PwwdWw(wwNew)->fDirty = fFalse; /* prevent update for now */
		InhibitRecorder(&fRecSav, fTrue);
		NewCurWw(wwNew, fFalse);
		InhibitRecorder(&fRecSav, fFalse);
		Assert(DocBaseWw(wwCur) == vpvs.docPrvw);
		}

	/* update the fields in the document (as if printing) */
	AssertDo(FPrepareDocFieldsForPrint (vpvs.docPrvw, pfpNormal));

	vpvs.lmSave = vlm;
	vpvs.flmSave = vflm;
	if (!FInitWwLbsForRepag(wwCur, vpvs.docPrvw, lmPreview, &vpvs.lbsText,&vpvs.lbsFtn))
		{
		EndFliLayout(vpvs.lmSave, vpvs.flmSave);
		return cmdError;
		}

	ShowOldWindows(HIDE_WINDOW);

	vpvs.fCreating = fTrue; /* to disable response to size msg before it is ready */
	/* Create the Page Preview Window */
	GetClientRect(vhwndApp,(LPRECT) &rcPrvw);
	rcIcon = rcPrvw;
	rcPrvw.ypTop += (dypIconPrvw - vsci.dypBorder);
	vpvs.dxpCli = rcPrvw.xpRight - rcPrvw.xpLeft;
	vpvs.dypCli = rcPrvw.ypBottom - rcPrvw.ypTop;
	Assert(vpvs.dxpCli > 0 && vpvs.dypCli > 0);
	if (!FInitPrvw(fTrue))
		{
		cmd = cmdNoMemory;
		UnInitPrvw(fTrue);
		goto LErrRet;
		}
	vhwndPrvwIconBar = NULL;
	if ((vhwndPgPrvw = CreateWindow((LPSTR)szClsPgPrvw,
			(LPSTR)NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, dypIconPrvw - vsci.dypBorder, vpvs.dxpCli, vpvs.dypCli, vhwndApp,
			NULL, vhInstance, (LPSTR)NULL)) == NULL)
		{
		cmd = cmdError;
		UnInitPrvw(fTrue);
		goto LErrRet;
		}

	if (vhwndPgPrvwScrl == NULL || !FCreatePrvwIconBar(vhwndApp, &rcIcon))
		{
		EndPreviewMode();
		/* don't go to LErrRet...same stuff done in EndPreviewMode */
		return cmdError;
		}
	RSBSetSpsLim( vhwndPgPrvwScrl, 2 );
	vpvs.fCreating = fFalse; /* ok to response to size msg from here on */

		{
		char sz[maxww(sizeof(szCsPrvwOnePg), sizeof(szCsPrvwTwoPg))];
		if (vpref.fPrvwTwo)
			CopyCsSz(szCsPrvwOnePg, sz);
		else
			CopyCsSz(szCsPrvwTwoPg, sz);
		SetIibbTexthwndIb(vhwndPrvwIconBar, iibbPrvwPages, sz);
		}

	ShowWindow(vhwndPgPrvw,SHOW_OPENWINDOW);

	/* Display preview window on top of all other child windows */
	BringWindowToTop(vhwndPgPrvw);
	SizePrvw(vhwndPgPrvw, vpvs.dxpCli, vpvs.dypCli);
	ShowWindow(vhwndPgPrvwScrl,SHOW_OPENWINDOW);
	UpdateWindow(vhwndPgPrvwScrl);

	/* blow font cache so we can cache preview fonts */
	FreeFontsPfti(vsci.pfti);
	UpdateWindow(vhwndPgPrvw); /* this call does repag and display */
	return cmdOK;

LErrRet:
	TrashWw(wwCur);
	ShowOldWindows(SHOW_OPENWINDOW);

	return cmd;
}


/* E N D  P R E V I E W  M O D E */
/* %%Function:EndPreviewMode %%Owner:davidbo */
EndPreviewMode()
{
	struct PLCPGD **hplcpgd;
	CP cpFirst;

	/* close up shop and go home. */
	UnInitPrvw(fTrue);
	if (vfIconBarMode)
		DestroyKmpIB(vhwndPrvwIconBar);
	if (vhwndPgPrvw)
		{
		DestroyWindow(vhwndPgPrvw);
		vhwndPgPrvw = NULL;
		}
	vhwndPgPrvwScrl = NULL;

	if (vhwndPrvwIconBar)
		{
		DestroyWindow(vhwndPrvwIconBar);
		vhwndPrvwIconBar = NULL;
		}

	/* blow cached preview fonts */
	FreeFontsPfti(vsci.pfti);
	SetAgain(bcmNil);

	/* skip the rest of this junk if in CBT */
	if (vhcbtsav != hNil)
		goto LRet;

	hplcpgd = PdodDoc(vpvs.docPrvw)->hplcpgd;

	/* one of these will be true if prvw killed while being started. */
	if (hplcpgd == hNil || vpvs.ipgdPrvw == ipgdLost)
		goto LRet;

	Assert(!((*hwwdCur)->wk & wkSDoc));

	if (vpvs.ipgdPrvw < IMacPlc(hplcpgd))
		cpFirst = CpFromIpgd(vpvs.docPrvw, wwCur, vpvs.ipgdPrvw);
	else
		cpFirst = CpPlc(hplcpgd, vpvs.ipgdPrvw);

		{
		CP cp;
		struct WWD *pwwd = *hwwdCur;
		if (pwwd->fPageView)
			{
			pwwd->ipgd = -1;  /* force new drs */
			SetPageDisp(wwCur, vpvs.ipgdPrvw, YeTopPage(wwCur), fFalse, fFalse);
			}
		else  if ((cp = PdrGalley(pwwd)->cpFirst) < cpFirst || cp >= CpPlc(hplcpgd, vpvs.ipgdPrvw + 1))
			{
			SetWwCpFirst(wwCur, cpFirst);
			}
		}

LRet:
	TrashAllWws();
	ShowOldWindows(SHOW_OPENWINDOW);
}


/* C M D  P R I N T  P R E V I E W */
/* Turn page preview mode on or off */
/* %%Function:CmdPrintPreview %%Owner:davidbo */
CMD CmdPrintPreview(pcmb)
CMB *pcmb;
{
	CMD cmd;
	StartLongOp();
	if (vlm == lmPreview)
		{
		EndPreviewMode();
		cmd = cmdOK;
		}
	else
		cmd = CmdTurnOnPrvw();
	EndLongOp(fFalse);
	return cmd;
}


/* %%Function:ElPrintPreview %%Owner:davidbo */
EL ElPrintPrvw(fOn)
{
	if (vcElParams != 0 && vfElFunc)
		RtError(rerrIllegalFunctionCall);

	if (!vfElFunc && (vcElParams == 0 || (!fOn) != (vlm != lmPreview)))
		CmdPrintPreview(NULL);

	return (vlm == lmPreview) ? -1 : 0;
}


/*
	PgPrvwWndProc handles all Windows message to the page preview
	window and it's children (two scroll bars and a sizebox)
*/

/* messages to this window are filtered in wprocn.asm */

/* %%Function:PgPrvwWndProc %%Owner:davidbo */
EXPORT LONG FAR PASCAL PgPrvwWndProc(hwnd, message, wParam, lParam)
HWND hwnd;
unsigned message;
WORD wParam;
LONG lParam;
{
	static int ipgdLast; /* Last page no. displayed while tracking thumb */
	struct PT pt;
	struct RC *prc;
	int fLeft, fShift, fResetToPrint, fFail;
	HDC hdc;
	CP cpMac, cp;
	PAINTSTRUCT ps;

#ifdef DAVIDBO
	ShowMsg("Pr", hwnd, message, wParam, lParam);
#endif

#ifdef DEBUG
	if (vhwndPgPrvw != NULL)
		Assert(hwnd == vhwndPgPrvw);
#endif

	fFail = fFalse;
	switch (message)
		{
	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);

		vhwndPgPrvwScrl = CreateWindow(szClsRSB,
				(LPSTR) NULL, WS_CHILD | SBS_VERT, 0, 0, 0, 0, hwnd, NULL,
				vhInstance, 0L);

		ipgdLast = 0;
		vpvs.fFirstPaint = fTrue;
		break;

	case WM_SIZE:
		if (!vpvs.fCreating)
			SizePrvw(hwnd, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
			{
		/*  Deal with erasing the background so some hosers
			can use color screens.
		*/
			HANDLE hOld;

			hOld = SelectObject(wParam, vsci.hbrDesktop);
			PatBlt(wParam, 0, 0, vpvs.dxpCli, vpvs.dypCli, PATCOPY);
			if (hOld != NULL)
				SelectObject(wParam, hOld);
			return(fTrue);
			}

	case WM_PAINT:
		/* Painting: very straight-forward.  Oh yeah, there
			is one kluge.  The first time through, all of this
			is handled by GotoPagePrvw.  The real place to do
			this call is after all the initialization crap.
			Unfortunately, some of the initialization crap is
			in the WM_SIZE message.  Calling GotoPagePrvw at
			every WM_SIZE won't work.
		*/

		BeginPaint(vhwndPgPrvw,(LPPAINTSTRUCT) &ps);
		if (!ps.hdc)
			{
			ErrorNoMemory(eidNoMemDisplay);
			goto LRet;
			}

		/* blank pages */
		BlankPageRect(ps.hdc, &vpvs.rcPage0);
		if (vpref.fPrvwTwo)
			BlankPageRect(ps.hdc, &vpvs.rcPage1);

		if (vpvs.fFirstPaint)
			{
			if ((*hwwdCur)->fPageView)
				{
				cp = CpMin(CpMacDocEdit(vpvs.docPrvw),
					CpPlc(PdodDoc(vpvs.docPrvw)->hplcpgd, (*hwwdCur)->ipgd));
				}
			else
				DocCpWithinPage(&cp);
			fFail = !FGotoPagePrvw(ps.hdc, YpFromCp(cp), dipgdRandom);
			vpvs.fFirstPaint = fFalse;
			}
		else if (!vpref.fPrvwTwo)
			{
			/*  On single page display, LRs are still valid! */
			if (fFail = !FDisplayLrsInPage(ps.hdc, fFalse, fFalse))
				goto LEndPaint;
			ShowAreas(ps.hdc, fTrue);
			goto LPaintPgn;
			}
		else
			{
			if (fFail = !FChangePrvwDoc(ps.hdc, fFalse, fFalse))
				goto LEndPaint;
LPaintPgn:
			ShowPgnP(vpvs.ipgdPrvw, fFalse);
			}

LEndPaint:
		EndPaint(hwnd,(LPPAINTSTRUCT) &ps);
		if (fFail)
			goto LErrRet;
		break;


	case WM_VSCROLL:
			{
		/* Deals with mouse activity in vertical scroll bar.
			Jumps by 1 or 2 pages (depending on vpvs.fFacing flag)
			unless thumb is manually moved, in which case the
			jump is random.
		*/

			cpMac = CpMacDoc(vpvs.docPrvw);

			if ((hdc = GetDC(hwnd)) == NULL)
				{
				ErrorNoMemory(eidNoMemDisplay);
				goto LRet;
				}

			switch (wParam)
				{
			case SB_THUMBPOSITION:
				fFail = !FGotoPagePrvw(hdc,LOWORD(lParam),dipgdRandom);
				break;

			case SB_THUMBTRACK:
				/* Converts scroll position to a cp and guesses
					a page number to show while manually moving
					the thumb.
			*/
					{
					int ipgd;
					struct PLCPGD **hplcpgd = PdodDoc(vpvs.docPrvw)->hplcpgd;
					CP cpThumb;

					if (LOWORD(lParam) == vpvs.ypLastScrlBar)
						cpThumb= cpMac-1;
					else
						cpThumb = (cpMac*LOWORD(lParam))/(vpvs.ypLastScrlBar);
					if (hplcpgd == hNil || (ipgd=IInPlcCheck(hplcpgd,cpThumb)) < 0)
						ipgd = 0;
					if (ipgd != ipgdLast)
						{
						ipgdLast = ipgd;
						ShowPgnP(ipgd, fTrue);
						}
					}
				break;

			case SB_LINEUP:
			case SB_PAGEUP:
				fFail = !FPrvwPageScroll(hdc, fTrue /* fUp */);
				break;

			case SB_LINEDOWN:
			case SB_PAGEDOWN:
				fFail = !FPrvwPageScroll(hdc, fFalse /* fUp */);
				break;

			default:
				break;
				}

			ReleaseDC(hwnd, hdc);
			if (fFail)
				goto LErrRet;
			break;
			}

	case WM_MOUSEMOVE:
		/* if the first paint hasn't been done, ignore */
		if (vpvs.fFirstPaint)
			goto LDefault;

		pt = MAKEPOINT(lParam);
		if (FPtInRc(&vpvs.rcPage0, pt))
			prc = &vpvs.rcPage0;
		else  if (FPtInRc(&vpvs.rcPage1, pt))
			prc = &vpvs.rcPage1;
		else
			goto LRestoreCursor;
		if (vpvs.tShowAreas != tAreasOff &&
				!PdodDoc(vpvs.docPrvw)->fLockForEdit &&
				FTrackAndChangeObject(NULL, pt, prc, fFalse, fFalse, NULL))
			{
			OurSetCursor(vhcPrvwCross);
			}
		else
			{
LRestoreCursor:
			OurSetCursor(vhcArrow);
			}
		break;

	case WM_LBUTTONDBLCLK:
		if (GetMessageTime() < vcmsecHelp)
			return(fTrue);       /* ignore DblClk if user requesting help */
		if (vhwndCBT)
			SendMessage(vhwndCBT, WM_CBTSEMEV, smvPrvwPageView, 0L);
		pt = MAKEPOINT(lParam);
		fLeft = !(FPtInRc(&vpvs.rcPage1, pt));
		if (!fLeft && !vpvs.fRightEmpty && !vpvs.fFirstBlank)
			vpvs.ipgdPrvw++;
		FExecCmd(bcmPageView);
		break;

	case WM_LBUTTONDOWN:
			{
		/* Deals with mouse activity in the preview page.
			If mouse clicked in a page, determines if cursor
			is near an important line, like a margin or a page
			break.  If so, the line is dragged until the mouse
			button is released.
		*/
		/* if the first paint hasn't been done, ignore */
			if (vpvs.fFirstPaint)
				goto LDefault;

			if (vfHelp)
				{
				GetHelp(cxtPrintPreview);
				return(fTrue);
				}

			if ((hdc = GetDC(hwnd)) == NULL)
				{
				ErrorNoMemory(eidNoMemDisplay);
				goto LRet;
				}

			SetCapture(hwnd);
			pt = MAKEPOINT(lParam);
			fShift = wParam & MK_SHIFT;
			if (vpvs.tShowAreas != tAreasOff)
				{
				if (FPtInRc(&vpvs.rcPage0, pt))
					{
					fLeft = fTrue;
					prc = &vpvs.rcPage0;
					}
				else  if (FPtInRc(&vpvs.rcPage1, pt))
					{
					fLeft = fFalse;
					prc = &vpvs.rcPage1;
					}
				else
					{
					if (vhwndCBT)          /* Advise CBT of the update */
						SendMessage(vhwndCBT, WM_CBTSEMEV, smvPrvwUpdate, 0L);
					fFail = !FSetModeNorm(hdc, fFalse);
					goto LReleaseButton;
					}

			/* click on opposite page switches margin page */
				if (vpref.fPrvwTwo && ((vpvs.tShowAreas == tAreasLeft) != fLeft))
					{
					if (vhwndCBT)
						SendMessage(vhwndCBT, WM_CBTSEMEV, smvPrvwOtherPage, 0L);
					if (vpvs.fModeMargin)
						{
						fFail = !FSetModeNorm(hdc, fFalse);
						goto LReleaseButton;
						}
					if (fLeft ? vpvs.fLeftEmpty : vpvs.fRightEmpty)
						goto LTrackMouse;
					ShowAreas(hdc, fTrue); /* erase */
					vpvs.fAreasKnown = fFalse;
					vpvs.tShowAreas = TAreasOpposite(vpvs.tShowAreas);
					ShowAreas(hdc, fFalse); /* draw on other page */
					goto LReleaseButton;
					}

				if (PdodDoc(vpvs.docPrvw)->fLockForEdit)
					goto LTrackMouse;

				if (FTrackAndChangeObject(hdc, pt, prc, fShift, fTrue, &fFail))
					goto LReleaseButton;
				}
			else  if /* !vpvs.tShowAreas */
			(vhwndCBT)    /* Advise CBT that user clicked */
				SendMessage(vhwndCBT, WM_CBTSEMEV, smvPrvwClick, 0L);

LTrackMouse:
			TrackMouseXaYa(hdc, bitDragXa + bitDragYa, &pt);
LReleaseButton:
			ReleaseCapture();
			ReleaseDC(hwnd, hdc);
			if (fFail)
				goto LErrRet;
			break;
			}

	case WM_CLOSE:
		EndPreviewMode();
		break;

	case WM_SYSCOMMAND:
		/* Pass the buck!  When the "user" clicks on the
			size box, pass responsibility back to the window
			that can handle it.
		*/
		if ((wParam & 0xfff0) == SC_SIZE)
			hwnd = vhwndApp;
		/* FALL THROUGH!!! */
LDefault:
		return(DefWindowProc(hwnd, message, wParam, lParam));
#ifdef DEBUG
	default:
		Assert(fFalse);
		goto LDefault;
#endif
		}
	goto LRet;

LErrRet:
	Assert(fFail);
	/*
		*  Can't call EndPreviewMode directly because it does a DestroyWindow,
		*  which is a very bad thing to do inside the WndProc outside the
		*  WM_CLOSE processing.  Did you get that? */
	PostMessage(vhwndPgPrvw,WM_CLOSE,0,0L);
LRet:
	return (0L);
}


/* %%Function:SizePrvw %%Owner:davidbo */
SizePrvw(hwnd, dxp, dyp)
HWND hwnd;
int dxp, dyp;
{
	int zpMaxOld, zpPosOld, zpPosNew;
	struct RC rcPage0, rcPage1;

	/* save new size of client area of preview window */
	vpvs.dxpCli = dxp;
	vpvs.dypCli = dyp;
	/* refigure maximum xp and yp available when window size changes. */
	vpvs.xpMacPrvw = xpMinPrvw + dxp - (dxpScrl - vsci.dxpBorder);
	vpvs.ypMacPrvw = ypMinPrvw + dyp;

	/*
	*  The range and position of the scroll bar depends on
	*  the screen height (vertical) of the scroll bar in
	*  pixels.  Therefore, when the window size changes,
	*  the scroll bar sizes change, and thus the range
	*  and thumb position of each scroll bar changes.
	*/
	vpvs.ypLastScrlBar = max(1, dyp - 2*GetSystemMetrics(SM_CYVSCROLL)
			- GetSystemMetrics(SM_CYVTHUMB) + 2*vsci.dypBorder);

	/*
	*  Change the range of the scrollbar to [0..vpvs.ypLastScrlBar+1] and map
	*  the old thumb position to a new position based on the new range.
	*/
	Assert(vhwndPgPrvwScrl != NULL);
	zpPosOld = SpsFromHwndRSB(vhwndPgPrvwScrl);
	zpMaxOld = SpsLimFromHwndRSB(vhwndPgPrvwScrl);
	RSBSetSpsLim(vhwndPgPrvwScrl, vpvs.ypLastScrlBar+1);
	zpPosNew = NMultDiv(zpPosOld, vpvs.ypLastScrlBar+1, zpMaxOld);
	RSBSetSps(vhwndPgPrvwScrl, zpPosNew);

	/* Move the preview scroll bar... */
	MoveWindow(vhwndPgPrvwScrl, dxp - dxpScrl, ypMinPrvw - vsci.dypBorder,
			dxpScrl, dyp + 2*vsci.dypBorder, fTrue);

	/* ...and the icon bar */
	Assert(vhwndPrvwIconBar != NULL);
	MoveWindow(vhwndPrvwIconBar, -vsci.dxpBorder, -vsci.dypBorder,
			dxp + 2*vsci.dxpBorder, dypIconPrvw, fTrue);

	/* Deal with status area...which is on the icon bar */
	if (vpvs.ipgdPrvw != ipgdLost)
		ShowPgnP(vpvs.ipgdPrvw, fFalse);

	rcPage0 = vpvs.rcPage0;
	rcPage1 = vpvs.rcPage1;
	vpvs.fAreasKnown = fFalse;
	SetPageRcs();
	if (FNeRgch(&rcPage0, &vpvs.rcPage0, sizeof(struct RC)) ||
			FNeRgch(&rcPage1, &vpvs.rcPage1, sizeof(struct RC)))
		InvalidateRect(vhwndPgPrvw, (LPRECT)NULL, fTrue);
}


/* %%Function:PrvwPageUp %%Owner:davidbo */
void PrvwPageUp()
{
	PrvwPageScroll(fTrue /* fUp */);
}


/* %%Function:PrvwPageDown %%Owner:davidbo */
void PrvwPageDown()
{
	PrvwPageScroll(fFalse /* fDown */);
}


/* %%Function:PrvwPageScroll %%Owner:davidbo */
PrvwPageScroll(fUp)
int fUp;
{
	int fFail;
	HDC hdc = GetDC(vhwndPgPrvw);

	if (hdc == NULL)
		{
		ErrorNoMemory(eidNoMemDisplay);
		return;
		}
	fFail = !FPrvwPageScroll(hdc, fUp);
	ReleaseDC(vhwndPgPrvw, hdc);
	if (fFail)
		EndPreviewMode();
}


/* %%Function:FPrvwPage %%Owner:davidbo */
FPrvwPageScroll(hdc, fUp)
{
	int dipgd;

	if (fUp)
		{
		if (vpvs.ypScrlBar == 0)
			goto LBeep;
		else  if (vpvs.ipgdPrvw == 0)
			{
			RSBSetSps(vhwndPgPrvwScrl, vpvs.ypScrlBar = 0);
			return fTrue;
			}
		else  if (!vpref.fPrvwTwo || !vpvs.fFacing)
			dipgd = -1;
		else
			{
			struct PGD pgd;
			AssertH(PdodDoc(vpvs.docPrvw)->hplcpgd);
			Assert((vpvs.ipgdPrvw-1) >= 0);
			GetPlc(PdodDoc(vpvs.docPrvw)->hplcpgd, vpvs.ipgdPrvw-1, &pgd);
			dipgd = (pgd.pgn &1 ) ? -2 : -1;
			}
		}
	else
		{
		if (vpvs.ypScrlBar < vpvs.ypLastScrlBar || vpvs.lbsFtn.fContinue)
			dipgd = (vpref.fPrvwTwo &&vpvs.fFacing &&!vpvs.fFirstBlank) ? 2 : 1;
		else
			{
LBeep:      
			Beep();
			return fTrue;
			}
		}
	return FGotoPagePrvw(hdc, 0, dipgd);
}


void PrvwF1();
void PrvwShiftF1();
void PrvwTab();
void PrvwReturn();

/* Definition of preview icon bar keymap moved to keys.h for localization. */

csconst KME rgkmePrvw[] =
{
rgkmePrvwDef
	};

#define ckmePreview     (sizeof(rgkmePrvw) / sizeof(KME))

/* %%Function:FInitPrvw %%Owner:davidbo */
FInitPrvw(fGenKmp)
int fGenKmp;
{
	int ipgd, ipgdMac, ipgdMacFtn, docFtn;
	struct PLC **hplcpgd, **hplcpgdFtn;
	struct PGD pgd;

	SetUndoNil();

	if (fGenKmp)
		{
		KMP *pkmp;

		/* create keymap for preview mode */
		if ((vpvs.hkmpPrvw = HkmpNew(ckmePreview, kmfPlain)) == hNil)
			return fFalse;

		pkmp = *vpvs.hkmpPrvw;
		Assert(pkmp->ikmeMax >= ckmePreview);
		bltx((int FAR *)&rgkmePrvw, (int FAR *) pkmp->rgkme, ckmePreview*cwKME);
		pkmp->ikmeMac = ckmePreview;
		Assert(pkmp->hkmpNext);
		AddAllKeysOfBcm(pkmp->hkmpNext, vpvs.hkmpPrvw,bcmIconBarMode,bcmIconBarMode);
#ifdef DEBUG
		CheckHkmp(vpvs.hkmpPrvw);
#endif
		}

	vpvs.hplaor = hNil;
	vpvs.iobj = iobjNil;
	vpvs.fLeftEmpty = vpvs.fRightEmpty = fTrue;
	SetWords(&vpvs.lbsLeft, 0, cwLBS);
	SetWords(&vpvs.lbsLeftFtn, 0, cwLBS);
	vpvs.fFacing = FFacingPages(vpvs.docPrvw);
	vpvs.fPrvwTab = vpvs.fModeMargin = vpvs.fAreasKnown = vpvs.fFirstBlank = vpvs.f1To2= fFalse;
	vpvs.tShowAreas = tAreasOff;
	vpvs.ipgdLbs = vpvs.ipgdPrvw = ipgdLost;
	vpvs.cchIbLast = 0;

	if (!FInitLayout(wwLayout, vpvs.docPrvw, &vpvs.lbsText, &vpvs.lbsFtn))
		return fFalse;

	SetPageRcs();

		/* determine the maximum reliable cp in plcpgd */
		{
		struct DOD *pdod;
		pdod = PdodDoc(vpvs.docPrvw);
		docFtn = pdod->docFtn;
		hplcpgd = pdod->hplcpgd;
		AssertH(hplcpgd);
		}
	if (docFtn != docNil)
		{
		hplcpgdFtn = PdodDoc(docFtn)->hplcpgd;
		AssertH(hplcpgdFtn);
		ipgdMacFtn = IMacPlc(hplcpgdFtn);
		}
	for (ipgdMac = IMacPlc(hplcpgd), ipgd = 0; ipgd < ipgdMac; ipgd++)
		{
		GetPlc(hplcpgd, ipgd, &pgd);
		if (pgd.fUnk)
			break;
		if (docFtn != docNil)
			{
			if (ipgd >= ipgdMacFtn)
				break;
			GetPlc(hplcpgdFtn, ipgd, &pgd);
			if (pgd.fUnk)
				break;
			}
		}
	vpvs.cpMacPrvw = CpPlc(hplcpgd, ipgd);

	/* Add preview font to master font table */
	if (vpvs.fLoadPrvwFon)
		LoadPreviewFont();

	return fTrue;
}


/* %%Function:UnInitPrvw %%Owner:davidbo */
UnInitPrvw(fNukeKmp)
int fNukeKmp;
{
	FreePhpl(&vpvs.lbsLeft.hpllr);
	FreePh(&vpvs.lbsLeft.hpllnbc);
	FreePhpl(&vpvs.lbsLeftFtn.hpllr);
	FreePh(&vpvs.hplaor);
	Assert(vpvs.lbsLeftFtn.hpllnbc == 0);
	Assert(vpvs.lmSave != lmPreview);
	EndFliLayout(vpvs.lmSave, vpvs.flmSave);
	EndLayout(&vpvs.lbsText, &vpvs.lbsFtn);
	if (fNukeKmp)
		RemoveKmp(vpvs.hkmpPrvw);
}


/* %%Function:FGotoPagePrvw %%Owner:davidbo */
FGotoPagePrvw(hdc, yp, dipgd)
HDC hdc;
int yp;
int dipgd;  /* -1 or +1 for sequential scrolling, -2 or +2 for sequential
with facing pages, dipgdRandom otherwise */
{
/* randomly accesses document in order to display pages in the 2 page preview
	pages.  The picture for the page containing the CP corresponding to scroll
	position yp or (vpvs.ipgdPrvw + dipgd) is drawn in the left page; the next
	page is drawn in the right page.  If either of these pages is already on
	the screen, the page is not recalculated.
	If vpvs.fFacing, then only even page numbers are allowed on the left page.
	Thus, if the pgn is odd AND vpvs.fFacing, then ipgd is decremented.
*/
	int ipgd;
	int fLbsUnknown = fFalse;
	struct PLCPGD **hplcpgd = PdodDoc(vpvs.docPrvw)->hplcpgd;
	struct PGD pgd;
	CP cpMac = CpMacDoc(vpvs.docPrvw);
	struct RPL rpl;
	struct LBS lbsText, lbsFtn;
	int flmSave = vflm;
	int lmSave = vlm;

	Assert(hdc);
	AssertH(hplcpgd);
	SetWords(&rpl, pgnMax, cwRPL);
	rpl.cp = cpMax;

	/* we could come into here when need to repaint preview window while
	printing from preview, need to restore to print mode after we are 
	done if necessary */
	Assert(vlm == lmPrint || vlm == lmPreview || vlm == lmNil);
	if ((vflm != flmRepaginate || PwwdWw(wwLayout)->grpfvisi.flm != flmRepaginate)
		&& !FResetWwLayout(vpvs.docPrvw, flmRepaginate, lmPreview))
		return fFalse;

	StartLongOp();
/* for random access, determine what page contains the cp - this is based on
	fact, not just on current state of hplcpgd */
	if (dipgd == dipgdRandom)
		{
		Assert(vpvs.ypLastScrlBar > 0);
		rpl.cp = CpMin(cpMac - ccpEop, (cpMac * yp) / vpvs.ypLastScrlBar);
		rpl.ipgd = IInPlc(hplcpgd, rpl.cp);
		if (rpl.cp >= vpvs.cpMacPrvw && vpvs.cpMacPrvw != cpMac)
			goto LUpdate;
		}
	else
		{
		rpl.ipgd = max(0, min(IMacPlc(hplcpgd) - 1, vpvs.ipgdPrvw + dipgd));
		if (dipgd > 0 && CpPlc(hplcpgd, rpl.ipgd) >= vpvs.cpMacPrvw)
			{
			rpl.ipgd = vpvs.ipgdPrvw + dipgd;
LUpdate:
			/* first try a cheap scan - if plcpgd is clean */
			for (ipgd = 0; ipgd < rpl.ipgd; ipgd++)
				{
				GetPlc(hplcpgd, ipgd, &pgd);
				if (pgd.fUnk)
					break;
				}
			if (ipgd < rpl.ipgd || rpl.ipgd >= IMacPlc(hplcpgd) - 1)
				{
				/* dirty entry found, or last entry */
				if (dipgd == dipgdRandom)
					rpl.ipgd = pgnMax;
				else
					rpl.cp = cpMax;
				LinkDocToWw(vpvs.docPrvw, wwLayout, wwNil);
				if (!FUpdateHplcpgd(wwLayout, vpvs.docPrvw, &vpvs.lbsText, &vpvs.lbsFtn, &rpl, patSilent))
					goto LErrRet;

				vpvs.cpMacPrvw = CpMax(vpvs.cpMacPrvw, vpvs.lbsText.cp);
				fLbsUnknown = fTrue;
				}
			}
		}
	if (dipgd == dipgdRandom)
		dipgd = rpl.ipgd - vpvs.ipgdPrvw;
	else
		rpl.cp = CpPlc(hplcpgd, rpl.ipgd);

/* determine if any movement is necessary. */
	GetPlc(hplcpgd, rpl.ipgd, &pgd);
	if (vpref.fPrvwTwo && vpvs.fFacing && !pgd.fUnk && !pgd.fEmptyPage && (pgd.pgn & 1))
		{
		rpl.ipgd = max(0, rpl.ipgd - 1);
		dipgd = rpl.ipgd - vpvs.ipgdPrvw;  /* even page available, use it */
		}
	/* reset scroll bar value */
	vpvs.ypScrlBar = YpFromCp(CpPlc(hplcpgd, rpl.ipgd));
	RSBSetSps(vhwndPgPrvwScrl, vpvs.ypScrlBar);
	if (rpl.ipgd == vpvs.ipgdPrvw)
		{
		/* lbs is in an unknown state, clear it */
		if (fLbsUnknown)
			{
			vpvs.ipgdLbs = rpl.ipgd = vpvs.ipgdPrvw;
			rpl.cp = cpMax;
			LinkDocToWw(vpvs.docPrvw, wwLayout, wwNil);
			if (!FUpdateHplcpgd(wwLayout, vpvs.docPrvw, &vpvs.lbsText, &vpvs.lbsFtn, &rpl, patSilent))
				goto LErrRet;
			}
		ShowPgnP(vpvs.ipgdPrvw, fFalse);
		Beep();
		goto LRet; /* no change */
		}

/* now ipgd is required page entry */
	ShowAreas(hdc, fFalse);    /* erase */
	vpvs.fAreasKnown = fFalse;        /* force area recalc */
	vpvs.ipgdPrvw = rpl.ipgd;
	if (dipgd == 1 && !vpvs.fFacing && vpref.fPrvwTwo)
		rpl.ipgd++; /* we're going to blt right to left, don't reformat */
	ShowPgnP(vpvs.ipgdPrvw, fFalse);

/* reposition lbs to page containing the desired cp */
	if (fLbsUnknown || rpl.ipgd != vpvs.ipgdLbs)
		{
		rpl.cp = cpMax;
		LinkDocToWw(vpvs.docPrvw, wwLayout, wwNil);
		if (!FUpdateHplcpgd(wwLayout, vpvs.docPrvw, &vpvs.lbsText, &vpvs.lbsFtn, &rpl, patReport))
			goto LErrRet;
		vpvs.cpMacPrvw = CpMax(vpvs.cpMacPrvw, vpvs.lbsText.cp);
		vpvs.ipgdLbs = rpl.ipgd;
		}

/* display the new pages */
	if (vpref.fPrvwTwo && vpvs.fFacing)
		{
		vpvs.fLeftEmpty = vpvs.fRightEmpty = fFalse;
		if (abs(dipgd) < 3)
			AnimateFlipPage(hdc, dipgd);
LDisplayPage:
		if (!FDisplayPage(hdc, fFalse) || !FDisplayPage(hdc, fTrue))
			goto LErrRet;
		}
	else  if (!vpref.fPrvwTwo || abs(dipgd) > 1)
		{
		vpvs.fLeftEmpty = vpvs.fRightEmpty = fTrue;
		if (!FDisplayPage(hdc, fFalse))        /* can't reuse either page */
			goto LErrRet;
		if (vpref.fPrvwTwo && !FDisplayPage(hdc, fTrue))
			goto LErrRet;
		}
	else  if (dipgd == 1)
		{
		/* copy right to left (forward in doc) */
		if (!FSyncLbs())
			goto LErrRet;
		if (!FCopyPageBitmap(hdc, fFalse) && !FDisplayLrsInPage(hdc, fFalse, fFalse))
			goto LErrRet;
		vpvs.fLeftEmpty = vpvs.fRightEmpty;
		vpvs.fRightEmpty = fTrue;
		if (!FDisplayPage(hdc, fTrue))
			goto LErrRet;
		}
	else
		{
		/* copy left to right (backward) */
		goto LDisplayPage;
#ifdef BOGUS
		int fRedisplay = !FCopyPageBitmap(hdc, fTrue);
		vpvs.fRightEmpty = vpvs.fLeftEmpty;
		vpvs.fLeftEmpty = fTrue;
		lbsText = vpvs.lbsLeft;
		lbsFtn = vpvs.lbsLeftFtn;
		SetWords(&vpvs.lbsLeft, 0, cwLBS);
		SetWords(&vpvs.lbsLeftFtn, 0, cwLBS);
		if (!FDisplayPage(hdc, fFalse))
			goto LErrRet;
		vpvs.lbsText = lbsText;
		vpvs.lbsFtn = lbsFtn;
		if (fRedisplay && !FDisplayLrsInPage(hdc, fTrue, fFalse))
			goto LErrRet;
		vpvs.ipgdLbs++;
#endif
		}
	ShowPgnP(vpvs.ipgdPrvw, fFalse);
	ShowAreas(hdc, fTrue);
LRet:
	EndLongOp(fFalse /* fAll */);
	if (vfRecording && !vpvs.fFirstPaint && dipgd != 0)
		{
		AddToVrac(racopVScroll, dipgd < 0 ? SB_PAGEUP : SB_PAGEDOWN, 
				abs(dipgd) + (vpvs.fFacing ? 1 : 0));
		}
	/* restore to print mode if necessary, see assert at top of routine */
	if (lmSave == lmPrint)
		FResetWwLayout(vpvs.docPrvw, flmSave, lmPrint);
	return fTrue;

LErrRet:
	/* restore to print mode if necessary, see assert at top of routine */
	if (lmSave == lmPrint)
		FResetWwLayout(vpvs.docPrvw, flmSave, lmPrint);
	EndLongOp(fFalse /* fAll */);
	return fFalse;
}


/* %%Function:FCopyPageBitmap %%Owner:davidbo */
FCopyPageBitmap(hdc, fToRight)
HDC hdc;
int fToRight;
{
	/* Copies the left page to the right page or vice versa.
		Tries to use BitBlt instead of redisplaying lrs.  Returns
		fTrue it blt, else fFalse;
	*/

	struct RC rcSmall, rcSrc, rcDst, rcClient;
	int dxpSrc;

	Assert(hdc);
	if (fToRight)
		{
		rcSrc = vpvs.rcPage0;
		rcDst = vpvs.rcPage1;
		}
	else
		{
		rcSrc = vpvs.rcPage1;
		rcDst = vpvs.rcPage0;
		}

	/* do some visual sex...Mac-like animation for movement */
	rcSmall.xpLeft = ((rcDst.xpLeft + rcDst.xpRight) >> 1)  - 10;
	rcSmall.ypTop = ((rcDst.ypTop + rcDst.ypBottom) >> 1) - 10;
	rcSmall.xpRight = rcSmall.xpLeft + 20;
	rcSmall.ypBottom = rcSmall.ypTop + 20;
	AnimateRect(hdc, &rcSrc, &rcSmall);
	AnimateRect(hdc, &rcSmall, &rcDst);

	/* Determine how much of rcSrc and rcDst actually show in the window */
	GetWindowRect(vhwndPgPrvw, (LPRECT) &rcClient);
	ScreenToClient(vhwndPgPrvw, (LPPOINT) &rcClient.ptTopLeft);
	ScreenToClient(vhwndPgPrvw, (LPPOINT) &rcClient.ptBottomRight);
	rcClient.xpLeft = max(0, rcClient.xpLeft);
	rcClient.xpRight = min(vpvs.xpMacPrvw, rcClient.xpRight);
	rcClient.ypTop = max(0, rcClient.ypTop);
	rcClient.ypBottom = min(vpvs.ypMacPrvw, rcClient.ypBottom);
	FSectRc(&rcClient, &rcSrc, &rcSrc);
	FSectRc(&rcClient, &rcDst, &rcDst);

	/* If a popup covers any part of rcSrc, can't do a BitBlt */
	/* rcSrc must be at least as wide as rcDst in order to do BitBlt */
	dxpSrc = rcSrc.xpRight - rcSrc.xpLeft;
	if (FPopupOverlap(vhwndPgPrvw, &rcSrc) || dxpSrc < (rcDst.xpRight - rcDst.xpLeft))
		{
		BlankPageRect(hdc, fToRight ? &vpvs.rcPage1 : &vpvs.rcPage0);
		return fFalse;
		}
	else
		{
		BitBlt(hdc, rcDst.xpLeft, rcDst.ypTop, dxpSrc,
				rcSrc.ypBottom - rcSrc.ypTop, hdc,
				rcSrc.xpLeft, rcSrc.ypTop, SRCCOPY);
		return fTrue;
		}
}


/* %%Function:DisplayLrs %%Owner:davidbo */
DisplayLrs(plbsText, fCanAbort)
struct LBS *plbsText;
int fCanAbort;
	/* display layout rectangles */
{
	HDC hdc;
	int ilr, ilrMac, fOutlineWw, fOutlineLr;
	int dxaLnn, dxa, xlLnn, xlRight, dyt, dytTable, yl, ylLnn;
	int fTop, fBottom, dytAbove, dytBelow, fTable;
	int fCalcXpRevBar = fTrue;
	int irmBar = PdopDoc(DocMother(plbsText->doc))->irmBar;
	int ilnbc, ilnbcMac, xpRevBar, flmSave;
	struct LNBC *plnbc, **hpllnbc;
	CP cpFirstLr;
	struct PLC **hplcpad = hNil;
	struct RC rc, rcLnn;
	struct LR lr;
	struct PAD pad;

	Scribble(2, 'D');

	Assert(vlm == lmPreview);
	if (plbsText->hpllr == hNil)
		return;
	hdc = PwwdWw(wwLayout)->hdc;
	if (plbsText->fOutline)
		hplcpad = PdodDoc(plbsText->doc)->hplcpad;


/* display all of the layout rectangles */
	Assert(wwLayout == plbsText->ww);
	fOutlineWw = PwwdWw(plbsText->ww)->fOutline;
	vfDrawPics = fTrue;
	vfli.fLayout = fTrue;
	ilrMac = (*plbsText->hpllr)->ilrMac;
	for (ilr = 0; ilr < ilrMac; ilr++)
		{
		bltLrp(LrpInPl(plbsText->hpllr, ilr), &lr, sizeof(struct LR));
		dxa = NMultDiv(lr.dxl, dxaInch, vfli.dxuInch);
		lr.xl = NMultDiv(lr.xl, vfli.dxsInch, vfli.dxuInch);
		lr.dxl = NMultDiv(lr.dxl, vfli.dxsInch, vfli.dxuInch);
		/* save position in printer units */
		dyt = lr.yl;
		lr.yl = NMultDiv(lr.yl, vfli.dysInch, vfli.dyuInch);
		lr.dyl = NMultDiv(lr.dyl, vfli.dysInch, vfli.dyuInch);

		rc.xpLeft = lr.xl;
		rc.xpRight = lr.xl + lr.dxl;

		fOutlineLr = lr.ihdt == ihdtNil ? fOutlineWw : fFalse;
		PwwdWw(wwLayout)->fOutline = fOutlineLr;
		if (lr.doc < 0)
			{
			Assert(vfli.fLayout);
			if (FFormatLineFspec(plbsText->ww, plbsText->doc, lr.dxl, -lr.doc))
				{
				rc.ypTop = lr.yl;
				rc.ypBottom = lr.yl + NMultDiv(vfli.dytLine, vfli.dysInch, vfli.dyuInch);
				DisplayPrvwFli(wwLayout, rc, lr.xl - vfli.xpLeft, rc.ypBottom);
				}
			continue;
			}

/* normal LR */
		if (lr.cp == cpNil)
			continue;

		CachePara(lr.doc, lr.cp);
		if (vpapFetch.dxaRight < 0)
			rc.xpRight -= NMultDiv(vpapFetch.dxaRight, vfli.dxsInch, czaInch);
		if (vpapFetch.dxaLeft < 0)
			rc.xpLeft += NMultDiv(vpapFetch.dxaLeft, vfli.dxsInch, czaInch);
		if (vpapFetch.dxaLeft1 < 0)
			rc.xpLeft += NMultDiv(vpapFetch.dxaLeft1, vfli.dxsInch, czaInch);
		if (rc.xpLeft < 0)
			rc.xpLeft = 0;
		RawLrCps(plbsText, &lr, wwLayout);
		Assert(vfli.fLayout);

		for (cpFirstLr = lr.cp; lr.cp < lr.cpLim; )
			{
			CachePara(lr.doc, lr.cp);
			fTable = FInTableVPapFetch(lr.doc, lr.cp);
/* outline mode and para not being shown */
			if (hplcpad && fOutlineLr)
				{
				GetPlc(hplcpad, IInPlc(hplcpad, lr.cp), &pad);
				if (!pad.fShow)
					{
					if (!fTable)
						lr.cp = caPara.cpLim;
					else
						{
						CpFirstTap1(lr.doc, lr.cp, fOutlineLr);
						lr.cp = caTap.cpLim;
						}
					continue;
					}
				}

/* table */
			if (fTable)
				{
				CachePara(lr.doc, lr.cp);
/* absolute text encountered during a non-abs table lr */
				if (!fOutlineLr && FAbsPap(lr.doc, &vpapFetch) && lr.lrk != lrkAbs)
					{
					CpFirstTap1(lr.doc, lr.cp, fOutlineLr);
					Assert(caTap.cpFirst == lr.cp);
					lr.cp = caTap.cpLim;
					continue;
					}
				CacheTc(wwLayout, lr.doc, lr.cp, fFalse, fFalse);
				CpFirstTap1(lr.doc, lr.cp, fOutlineLr);
				Assert(caTap.cpFirst == lr.cp);
				lr.cp = caTap.cpLim;
				fTop = vtcc.fFirstRow || caTap.cpFirst == cpFirstLr && lr.fForceFirstRow;
				CachePara(lr.doc, caTap.cpLim);
				/* may fail if row is followed by abs object */
				if (vtcc.fLastRow || (lr.lrk == lrkAbs && caTap.cpLim >= lr.cpLim))
					fBottom = fTrue;
				else  if (lr.cp != lr.cpLim)
					fBottom = fFalse;
				else  if (ilr == ilrMac - 1)
					fBottom = fTrue;
				else 					
					{
					LRP lrp;
					lrp = LrpInPl(plbsText->hpllr, ilr + 1);
					/* lr.xl got munged, so we have to consult the original */
					fBottom = LrpInPl(plbsText->hpllr, ilr)->xl != lrp->xl || lr.doc != lrp->doc;
					}
				/* Set flmPrint so table height comes back in printer units */
				flmSave = vflm;
				FResetWwLayout(vpvs.docPrvw, flmRepaginate, lmPreview);
				PwwdWw(wwLayout)->fOutline = fOutlineLr;

				CacheTc(wwLayout, lr.doc, caTap.cpFirst, fTop, fBottom);
				CpFirstTap1(lr.doc, vtcc.cpFirst, fOutlineLr);
				dytTable = DypHeightTable(wwLayout, lr.doc, caTap.cpFirst, fTop, fBottom, &dytAbove, &dytBelow);
				dyt += dytTable + dytAbove + dytBelow;
				/* convert now while dysInch and dyuInch are correct */
				yl = NMultDiv(dyt, vfli.dysInch, vfli.dyuInch);
				FResetWwLayout(vpvs.docPrvw, flmSave,lmPreview);
				PwwdWw(wwLayout)->fOutline = fOutlineLr;

				CacheTc(wwLayout, lr.doc, caTap.cpFirst, fFalse, fFalse);
				CpFirstTap1(lr.doc, vtcc.cpFirst, fOutlineLr);
				Assert(vfli.fLayout);
				ScanTableRow(wwLayout, lr.doc, caTap.cpFirst, DisplayLrTable, &lr, dytAbove);
				PrintTableBorders(wwLayout, lr.xl, lr.yl,
						NMultDiv(dytTable, vfli.dysInch, vfli.dyuInch),
						NMultDiv(dytAbove, vfli.dysInch, vfli.dyuInch),
						NMultDiv(dytBelow, vfli.dysInch, vfli.dyuInch),
						fTop, fBottom);
				lr.yl = yl;
				continue;
				}

/* absolute text encountered during a non-abs lr */
			if (hplcpad == hNil && FAbsPap(lr.doc, &vpapFetch) && lr.lrk != lrkAbs)
				{
				lr.cp = caPara.cpLim;
				continue;
				}

/* normal text - format a line */
			Assert(vfli.fLayout);
			FormatLineDxa(wwLayout, lr.doc, lr.cp, dxa);
			lr.cp = vfli.cpMac;     /* advance for next line */
			if (vfli.ichMac == 0)
				continue;
			if (lr.fSpaceBefore)
				{
				dyt -= NMultDiv(vfli.dypBefore, vfli.dyuInch, vfli.dysInch);
				lr.yl -= vfli.dypBefore;
				lr.fSpaceBefore = fFalse;
				}
			if (vfli.fSplatBreak && vfli.ichMac == 1)
				{
				if (PdodDoc(lr.doc)->fShort || lr.lrk == lrkAbs)
					{
					dyt += vfli.dytLine;
					lr.yl = NMultDiv(dyt, vfli.dysInch, vfli.dyuInch);
					continue;
					}
				Assert(lr.cp == lr.cpLim);
				break;
				}

			rc.ypTop = lr.yl;
			dyt += vfli.dytLine;
			rc.ypBottom = (lr.yl = NMultDiv(dyt, vfli.dysInch, vfli.dyuInch));

			DisplayPrvwFli(wwLayout, rc, lr.xl, lr.yl);

			/* draw revision bar if needed */
			if (vfli.fRMark && irmBar != irmBarNone)
				{
				if (fCalcXpRevBar)
					{
					/* plbs->pgn refers to Next page...we need
						fOdd for this page. */
					int fOdd = !(plbsText->pgn & 1);
					/* if true, bar goes in left margin on this page */
					int fLeft;
					int xpRight, xpLeft;
					struct DOP *pdop;

					if (irmBar == irmBarOutside)
						fLeft = !(FFacingPages(plbsText->doc) && fOdd);
					else
						fLeft = (irmBar == irmBarLeft);
					pdop = PdopDoc(DocMother(plbsText->doc));
					GetXlMargins(pdop, fOdd, vfli.dxsInch,&xpLeft,&xpRight);
					if (fLeft)
						xpRevBar = xpLeft -
								NMultDiv(dxaInch/8, vfli.dxsInch, dxaInch);
					else
						xpRevBar = xpRight +
								NMultDiv(dxaInch/8, vfli.dxsInch, dxaInch);
					fCalcXpRevBar = fFalse;
					}
				DrawRevBar(hdc, xpRevBar, lr.yl - vfli.dypLine,
						vfli.dypLine, NULL /* prcwClip */);
				}

/* check for line number */
			if (lr.lnn == lnnNil || vpapFetch.fNoLnn)
				continue;
			CacheSect(lr.doc, vfli.cpMin);
			if ((vfmtss.lnn = lr.lnn++) % vsepFetch.nLnnMod)
				continue;

/* line number */
			dxaLnn = (vsepFetch.dxaLnn) ? vsepFetch.dxaLnn :
					((vsepFetch.ccolM1) ? dxaInch/8 : dxaInch/4);
			ylLnn = lr.yl - vfli.dypBase;
			Assert(vfli.fLayout);
			if (FFormatLineFspec(plbsText->ww, plbsText->doc, 0, chLnn))
				{
				/* we go left requested amount from text, plus
					enough more to make line number right flush to
					that mark; have to ignore chEop at end
				*/
				xlRight = vfli.xpRight - vfli.rgdxp[vfli.ichMac-1];
				xlLnn = lr.xl - xlRight - NMultDiv(dxaLnn,vfli.dxsInch,dxaInch);
				rcLnn = rc;
				rcLnn.xpLeft = xlLnn;
				rcLnn.xpRight = lr.xl;
				DisplayPrvwFli(wwLayout, rcLnn, xlLnn, ylLnn + vfli.dypBase);
				}
			}
		}
	vfPrvwDisp = fFalse;
	PwwdWw(wwLayout)->fOutline = fOutlineWw;

	/* draw vertical lines between columns */
	hpllnbc = plbsText->hpllnbc;
	ilnbcMac = (hpllnbc == hNil) ? 0 : IMacPl(hpllnbc);
	if (ilnbcMac != 0)
		{
		int xp, yp, dyp;
		int dxp = min(1,vfli.dxsInch/72);
		int dxpO2 = dxp >> 1;

		Assert(plbsText->hpllnbc);
		FreezeHp();
		plnbc = PInPl(hpllnbc, 0);
		for (ilnbc = 0; ilnbc < ilnbcMac; ilnbc++, plnbc++)
			{
			xp = NMultDiv(plnbc->xl, vfli.dxsInch, vfli.dxuInch);
			yp = NMultDiv(plnbc->yl, vfli.dysInch, vfli.dyuInch);
			dyp = NMultDiv(plnbc->dyl, vfli.dysInch, vfli.dyuInch);
			DrawPrvwLine(hdc, xp - dxpO2, yp, dxp, dyp, colAuto);
			}
		MeltHp();
		}

	vfDrawPics = fFalse;
	vfli.fLayout = fFalse;
	Scribble(2, ' ');
}


/* %%Function:DisplayPrvwFli %%Owner:davidbo */
DisplayPrvwFli(ww, rc, xl, yl)
int ww, xl, yl;
struct RC rc;
{
	/*
	*  vfPrvwDisp affects ResetFont: if fTrue, deselects font in prvw window
	*  DC; if fFalse, deselects font in all the other screen DCs.  Call first
	*  with vfPrvwDisp fFalse to kill font left selected into the screen DCs
	*  by FormatLine.  After DisplayFliCore, call ResetFont with vfPrvwDisp
	*  fTrue to kill font selected into prvw window DC.
	*/
	ResetFont(fFalse);
	vfPrvwDisp = fTrue;
	DisplayFliCore(ww, rc, xl, yl);
	ResetFont(fFalse);
	vfPrvwDisp = fFalse;
}


/* %%Function:SetPageRcs %%Owner:davidbo */
SetPageRcs()
{
	int xpMin, ypMin, shftXPage;
	int dxp, dyp, dxpPageSc, dypPageSc, dxpPrvw, dypPrvw;
	int dxpRc, dypRc, dxpCliLocal;
	long l;
	struct DOD *pdod;

	/* document's page size in ut's */
	FreezeHp(); /* pdod used at bottom of routine! */
	pdod = PdodDoc(vpvs.docPrvw);
	vpvs.dxaPage = max(1, pdod->dop.xaPage);
	vpvs.dyaPage = max(1, pdod->dop.yaPage);

	/* document's page size in screen pixels */
	vpvs.rcPagePrint.xpLeft = vpvs.rcPagePrint.ypTop = 0;
	vpvs.rcPagePrint.xpRight = max(1,NMultDiv(vpvs.dxaPage, vfli.dxsInch, czaInch));
	vpvs.rcPagePrint.ypBottom = max(1,NMultDiv(vpvs.dyaPage, vfli.dysInch,czaInch));

	vpvs.xpMaxPrvw = vsci.dxpScreen - dxpScrl + vsci.dxpBorder;
	vpvs.ypMaxPrvw = vsci.dypScreen - dypCaption - dypMenu - dypIconPrvw;
	xpMin = xpMinPrvw + 6*vsci.dxpBorder;
	ypMin = ypMinPrvw + 6*vsci.dypBorder;
	dxpPrvw = vpvs.xpMaxPrvw - xpMinPrvw - 12 * vsci.dxpBorder;
	dypPrvw = vpvs.ypMaxPrvw - ypMinPrvw - 12 * vsci.dypBorder;
	shftXPage = 0;

	if (!vpref.fPrvwTwo)
		/* single page image */
		dxp = dxpPrvw;
	else  if (vpvs.fFacing)
		{
		/* Facing papers are flush against each other.
			Together, they form a single rect in the
			middle of the window.
		*/
		dxp = dxpPrvw;
		shftXPage = 1;
		}
	else
		{
		/* non-facing papers are seperated a bit and are
			drawn separately, centered in the two halves
			of the window.
		*/
		dxp = ((vpvs.xpMaxPrvw - xpMinPrvw) >> 1) - 9*vsci.dxpBorder;
		}
	dyp = min(dypPrvw, vpvs.rcPagePrint.ypBottom);

	/* Compare the aspect ratios of paper and the window to
		determine scaling and translation of the paper in
		the window.  If (vpvs.dxaPage/vpvs.dyaPage) < (dxp/dyp) then
		dypPageSc = dypWindow, and we compute the horizontal
		dimension proportionally.  Otherwise, dxpPageSc =
		dxpWindow, and we compute the vert. dimension
		proportionally.  Note the factors for warping by the
		aspect ratio of the current display device...done separately
		to avoid overflow.
	*/
	l = ((long)dxp * vpvs.dyaPage) / dyp;
	l = (l * vfli.dysInch) / vfli.dxsInch;
	if (((long)vpvs.dxaPage << shftXPage) < l)
		{
		dypPageSc = dyp;
		l = (((long)vpvs.dxaPage * dyp) / vpvs.dyaPage) << shftXPage;
		dxpPageSc = (l * vfli.dxsInch) / vfli.dysInch;
		Assert(dxpPageSc <= dxp);
		}
	else
		{
		dxpPageSc = dxp;
		l = (((long)vpvs.dyaPage * dxp) / vpvs.dxaPage) >> shftXPage;
		dypPageSc = (l * vfli.dysInch) / vfli.dxsInch;
		Assert(dypPageSc <= dyp);
		}

	/* center paper in window */
	vpvs.rcPage0.ypTop = ypMin + ((dypPrvw - dypPageSc) >> 1);
	vpvs.rcPage0.ypBottom = vpvs.rcPage0.ypTop + dypPageSc;
	vpvs.rcPage0.xpLeft = xpMin + ((dxp - dxpPageSc) >> 1);
	vpvs.rcPage1.ypTop = vpvs.rcPage0.ypTop;
	vpvs.rcPage1.ypBottom = vpvs.rcPage0.ypBottom;
	if (!vpref.fPrvwTwo)
		{
		vpvs.rcPage0.xpRight = vpvs.rcPage0.xpLeft + dxpPageSc;
		vpvs.rcPage1.xpLeft = vpvs.rcPage1.xpRight = zpUnknown;
		}
	else  if (vpvs.fFacing)
		{
		vpvs.rcPage0.xpRight = vpvs.rcPage0.xpLeft + (dxpPageSc>>1);
		vpvs.rcPage1.xpLeft = vpvs.rcPage0.xpRight + 1;
		vpvs.rcPage1.xpRight = vpvs.rcPage1.xpLeft + (dxpPageSc>>1);
		}
	else
		{
		vpvs.rcPage0.xpRight = vpvs.rcPage0.xpLeft + dxpPageSc;
		vpvs.rcPage1.xpLeft = ((vpvs.xpMaxPrvw + xpMinPrvw) >> 1) +
				3*vsci.dxpBorder + ((dxp - dxpPageSc) >> 1);
		vpvs.rcPage1.xpRight = vpvs.rcPage1.xpLeft + dxpPageSc;
		}

	/* Adjust RCs if they didn't fit in the client area of the window */
	/* Either attempt to center RCs in client area, or offset by the
		width of the system font */

	dxpCliLocal = vpvs.dxpCli - dxpScrl + vsci.dxpBorder;
	dxpRc = (vpref.fPrvwTwo ? vpvs.rcPage1.xpRight : vpvs.rcPage0.xpRight) - vpvs.rcPage0.xpLeft;
	dxp = vpvs.rcPage0.xpLeft - (dxpRc <= dxpCliLocal ? ((dxpCliLocal-dxpRc)>>1) : vsci.dxpTmWidth);
	vpvs.rcPage0.xpLeft -= dxp;
	vpvs.rcPage0.xpRight -= dxp;
	if (vpref.fPrvwTwo)
		{
		vpvs.rcPage1.xpLeft -= dxp;
		vpvs.rcPage1.xpRight -= dxp;
		}

	dypRc = vpvs.rcPage0.ypBottom - vpvs.rcPage0.ypTop;
	dyp = vpvs.rcPage0.ypTop - (dypRc <= vpvs.dypCli ? ((vpvs.dypCli - dypRc)>>1) : vsci.dypTmHeight);
	vpvs.rcPage0.ypTop -= dyp;
	vpvs.rcPage0.ypBottom -= dyp;
	if (vpref.fPrvwTwo)
		{
		vpvs.rcPage1.ypTop -= dyp;
		vpvs.rcPage1.ypBottom -= dyp;
		}

	SetVdxpGutter(pdod->dop.dxaGutter);
	MeltHp();
}


/* %%Function:SetVdxpGutter %%Owner:davidbo */
SetVdxpGutter(dxaGutter)
int dxaGutter;
{
	struct PT pt;

	/* calculate the gutter in small-page units */
	vpvs.dxpGutter = 0;
	if ((pt.xp = dxaGutter) != 0)
		{
		pt.yp = 0;
		pt.xp = NMultDiv(pt.xp, vfli.dxsInch, dxaInch);
		MapPt(&pt, &vpvs.rcPagePrint, &vpvs.rcPage0);
		vpvs.dxpGutter = pt.xp - vpvs.rcPage0.xpLeft;
		}
}


/* %%Function:FDisplayPage %%Owner:davidbo */
FDisplayPage(hdc, fRight)
HDC hdc;
int fRight;
{
	/*
	*  FDisplayPage - 'print' the current page
	*  If fRight is true, the right page is printed, else the left page is
	*/

	int fFail = fFalse;

	/*
	*  We can get a PAINT message while printing from preview.  We should
	*  never come here...should jump straight to DisplayLrsInPage.
	*/
	Assert(hdc);
	Assert(vlm == lmPreview);
	Assert(!fRight || fRight && vpref.fPrvwTwo);
	StartLongOp();
	/* create a new set of LRs - the cases where we don't create new LRs are:
		1. end of doc
		2. empty page at start of doc
		2. right page when only one page is being displayed
	*/
	if (!fRight)
		vpvs.fFirstBlank = fFalse;
	if ((vpref.fPrvwTwo || !fRight) &&
			(vpvs.lbsText.cp < CpMacDoc(vpvs.docPrvw) || vpvs.lbsFtn.fContinue))
		{
		if ((vflm != flmRepaginate || PwwdWw(wwLayout)->grpfvisi.flm != flmRepaginate) 
			&& !FResetWwLayout(vpvs.docPrvw, flmRepaginate, lmPreview))
			{
			fFail = fTrue;
			goto LEndDisplay;
			}
		ShowPgnP(vpvs.ipgdLbs, fTrue);
		vpvs.lbsText.fRight = vpvs.lbsText.pgn & 1;
		if (vpvs.lbsText.fRight != fRight && vpref.fPrvwTwo && vpvs.fFacing)
			{
			if (vpvs.ipgdLbs == 0)
				{
				vpvs.fLeftEmpty = vpvs.fFirstBlank = fTrue;
				goto LSkipFormat;
				}
			vpvs.lbsText.fRight = fRight;
			}
		SetPgdInvalid(vpvs.ipgdLbs, &vpvs.lbsText, &vpvs.lbsFtn);
		if (fFail = (LbcFormatPage(&vpvs.lbsText, &vpvs.lbsFtn) == lbcAbort))
			goto LEndDisplay;
		SetPgdValid(vpvs.ipgdLbs++, &vpvs.lbsText, &vpvs.lbsFtn, fTrue);
		vpvs.cpMacPrvw = CpMax(vpvs.cpMacPrvw, vpvs.lbsText.cp);
		if (fRight)
			vpvs.fRightEmpty = vpvs.lbsText.fEmptyPage;
		else
			vpvs.fLeftEmpty = vpvs.lbsText.fEmptyPage;
		}
	else if (fRight)
		vpvs.fRightEmpty = fTrue;
	else
		vpvs.fLeftEmpty = fTrue;
LSkipFormat:
	fFail = !FDisplayLrsInPage(hdc, fRight, fTrue);      /* draw the picture */

LEndDisplay:
	EndLongOp(fFalse /* fAll */);
	return (!fFail);
}


/* %%Function:FDisplayLrsInPage %%Owner:davidbo */
FDisplayLrsInPage(hdc, fRight, fSync)
HDC hdc;
int fRight, fSync;
{
	struct RC rcDraw, rcClip;
	int idcOld, fOK = fTrue;
	int flmSave, lmSave;

	/* We can get a paint message while printing from preview...this is
		Ok since our LRs are still around and all we have to do is redisplay
		them.
	*/
	Assert(hdc);
	Assert(vlm == lmPreview || vlm == lmPrint);

	vpvs.fLeft = !fRight;
	rcDraw = fRight ? vpvs.rcPage1 : vpvs.rcPage0;
	if ((fRight && vpvs.fRightEmpty) || (!fRight && vpvs.fLeftEmpty))
		{
		BlankPageRect(hdc, &rcDraw);
		return fTrue;
		}

	if ((idcOld = SaveDC(hdc)) == 0)
		{
		ErrorNoMemory(eidNoMemDisplay);
		return fFalse;
		}

	StartLongOp();
	/* Shrink drawing rectangle for the PatBlt...don't want to erase the
		black lines drawn by DrawPrvwContents!
	*/
	InflateRect((LPRECT) &rcDraw, -1, -1);
	PatBlt(hdc, rcDraw.xpLeft, rcDraw.ypTop, rcDraw.xpRight - rcDraw.xpLeft,
			rcDraw.ypBottom - rcDraw.ypTop, WHITENESS);
	if (vpvs.fPrintableArea)
		{
		rcClip.xpLeft = max(0, NMultDiv(vpri.ptUL.xp, vfli.dxsInch, vfli.dxuInch));
		rcClip.ypTop = max(0, NMultDiv(vpri.ptUL.yp, vfli.dysInch, vfli.dyuInch));
		rcClip.xpRight = rcClip.xpLeft + NMultDiv(vpri.dxpPrintable, vfli.dxsInch, vfli.dxuInch);
		rcClip.ypBottom = rcClip.ypTop + NMultDiv(vpri.dypPrintable, vfli.dysInch, vfli.dyuInch);
		}
	else
		rcClip = vpvs.rcPagePrint;
	MapRect(&rcClip, &vpvs.rcPagePrint, &rcDraw);
	rcClip.xpRight = min(rcClip.xpRight, rcDraw.xpRight);
	rcClip.ypBottom = min(rcClip.ypBottom, rcDraw.ypBottom);
	IntersectClipRect(hdc, rcClip.xpLeft, rcClip.ypTop, rcClip.xpRight, rcClip.ypBottom);

#ifdef FUTURE
	/* MacWord does something like this, should we? (db) */
	if (vpvs.dxpGutter != 0)
		{
		HANDLE hOld;
		struct RC rcGutter;

		rcGutter = rcDraw;
		if (FFacingEven(!fRight))
			rcGutter.xpLeft = rcDraw.xpRight - vpvs.dxpGutter;
		else
			rcGutter.xpRight = rcDraw.xpLeft + vpvs.dxpGutter;
		hOld = SelectObject(hdc, vhbrLtGray);
		PatBlt(hdc, rcGutter.xpLeft, rcGutter.ypTop, vpvs.dxpGutter, rcGutter.ypBottom - rcGutter.ypTop, PATCOPY);
		if (hOld != NULL)
			SelectObject(hdc, hOld);
		}
#endif

	/* scale to rcDraw */
	SetMapMode(hdc, MM_ANISOTROPIC);
	SetWindowOrg(hdc, 0, 0);
	SetWindowExt(hdc, dxpPage, dypPage);
	SetViewportOrg(hdc, rcDraw.xpLeft, rcDraw.ypTop);
	SetViewportExt(hdc, rcDraw.xpRight - rcDraw.xpLeft,
			rcDraw.ypBottom - rcDraw.ypTop);
	PwwdWw(wwLayout)->hdc = hdc;
	lmSave = vlm;
	flmSave = vflm;
	if (vflm != flmDisplayAsPrint && !FResetWwLayout(vpvs.docPrvw, flmDisplayAsPrint, lmPreview))
		{
		fOK = fFalse;
		goto LRet;
		}

	if (fSync && vpref.fPrvwTwo && !fRight && !(fOK = FSyncLbs()))
		{
		if (lmSave == lmPrint)
			/* no need to check return value of FResetWwLayout because */
			/* fOK returned would be false anyway */
			FResetWwLayout(vpvs.docPrvw, flmSave, lmPrint);
		goto LRet;
		}
	DisplayLrs(vpref.fPrvwTwo && !fRight ? &vpvs.lbsLeft : &vpvs.lbsText, fFalse);

#ifdef DAVIDBO
		{
	/* draw boxes for LRs */
		struct PLLR **hpllr = vpref.fPrvwTwo && !fRight ?
		vpvs.lbsLeft.hpllr : vpvs.lbsText.hpllr;
		int ilr, ilrMac = (*hpllr)->ilrMac;
		struct RC rc;
		struct LR lr;

		for (ilr = 0; ilr < ilrMac; ilr++)
			{
			bltLrp(LrpInPl(hpllr, ilr), &lr, sizeof(struct LR));
			lr.xl = NMultDiv(lr.xl, vfli.dxsInch, vfli.dxuInch);
			lr.dxl = NMultDiv(lr.dxl, vfli.dxsInch, vfli.dxuInch);
			lr.yl = NMultDiv(lr.yl, vfli.dysInch, vfli.dyuInch);
			lr.dyl = NMultDiv(lr.dyl, vfli.dysInch, vfli.dyuInch);
			rc.xpLeft = lr.xl;
			rc.xpRight = lr.xl + lr.dxl;
			rc.ypTop = lr.yl;
			rc.ypBottom = lr.yl + lr.dyl;
			FrameRectRop(hdc, &rc, PATINVERT);
			}
		}
#endif

	if (lmSave == lmPrint)
		fOK = FResetWwLayout(vpvs.docPrvw, flmSave, lmPrint);
LRet:
	RestoreDC(hdc, idcOld);
	EndLongOp(fFalse /* fAll */);
	return (fOK);
}


/* %%Function:ShowAreas %%Owner:davidbo */
ShowAreas(hdc, fTurningOn)
HDC hdc;
int fTurningOn;
{
	/* show the margins, page break, etc.
	*/
	int dxp, dyp, idcOld;
	int ilr, ilrMac, ircs, iaor, iaorMac;
	int lbc;
	struct LBS *plbs;
	struct PLLR **hpllr;
	struct RC *prc, *prcHdrFtr, rcHandle, rcLine, rcHdrFtr;
	struct PT pt;
	struct AOR aor;
	struct LR lr;

	Assert(hdc);
	if (vpvs.tShowAreas == tAreasOff)
		return;
	Assert(vpvs.tShowAreas == tAreasLeft || vpvs.tShowAreas == tAreasRight);

/* special case: when margins adjustment leaves an empty page where areas were
	being shown (end of doc or right/left adjustment) */
	if (fTurningOn && ((vpvs.tShowAreas == tAreasLeft) ? vpvs.fLeftEmpty :vpvs.fRightEmpty))
		{
		vpvs.fAreasKnown = fFalse;
		vpvs.tShowAreas = TAreasOpposite(vpvs.tShowAreas);
		}

/* set up the inside margin rectangle and determine page break type */
	prc = (vpvs.tShowAreas == tAreasLeft) ? &vpvs.rcPage0 : &vpvs.rcPage1;
	dyp = prc->ypBottom - prc->ypTop;
	plbs = FLeftLbs() ? &vpvs.lbsLeft : &vpvs.lbsText;

	if (!vpvs.fAreasKnown)
		{
		struct DOP *pdop;

		lbc = plbs->lbc;
		FreezeHp();
		pdop = PdopDoc(vpvs.docPrvw);
		vpvs.rcMargin.ypTop = prc->ypTop + NMultDiv(abs(pdop->dyaTop), dyp, vpvs.dyaPage);
		vpvs.rcMargin.ypBottom = prc->ypBottom - NMultDiv(abs(pdop->dyaBottom), dyp, vpvs.dyaPage);
		GetXlMargins(pdop, (plbs->pgn-1) & 1, dxaInch, &vpvs.rcMargin.xpLeft, &vpvs.rcMargin.xpRight);
		dxp = prc->xpRight - prc->xpLeft;
		vpvs.rcMargin.xpLeft = prc->xpLeft + NMultDiv(vpvs.rcMargin.xpLeft, dxp, vpvs.dxaPage);
		vpvs.rcMargin.xpRight = prc->xpLeft + NMultDiv(vpvs.rcMargin.xpRight, dxp, vpvs.dxaPage);

		vpvs.pbk = (lbc == lbcEndOfPage || lbc == lbcEndOfColumn) ? pbkForced :
				(lbc == lbcEndOfSection || lbc == lbcEndOfDoc) ?
				pbkSection : pbkNatural;
		MeltHp();
		}

	idcOld = SaveDC(hdc);
	IntersectClipRect(hdc,prc->xpLeft,prc->ypTop,prc->xpRight,prc->ypBottom);

/* draw margin lines */
	for (ircs = 0; ircs < 4; ++ircs)
		{
		GetMargin(ircs, &rcHandle, &rcLine);
		dxp = rcLine.xpRight - rcLine.xpLeft;
		dyp = rcLine.ypBottom - rcLine.ypTop;
		SelectObject(hdc, vhbrGray);
		PatBlt(hdc, rcLine.xpLeft, rcLine.ypTop, dxp, dyp, PATINVERT);
		dxp = rcHandle.xpRight - rcHandle.xpLeft;
		dyp = rcHandle.ypBottom - rcHandle.ypTop;
		SelectObject(hdc, vhbrWhite);
		PatBlt(hdc, rcHandle.xpLeft, rcHandle.ypTop, dxp, dyp, PATINVERT);
		}

	SelectObject(hdc,vhbrGray);
	if (vpvs.fAreasKnown)
		{
/* boxes are easy if we're just redrawing */
		if (vpvs.rcHdr.xpLeft > 0)
			FrameRectRop(hdc, &vpvs.rcHdr, PATINVERT);
		if (vpvs.rcFtr.xpLeft > 0)
			FrameRectRop(hdc, &vpvs.rcFtr, PATINVERT);
		if (vpvs.hplaor != hNil)
			{
			for (iaor = 0, iaorMac = (*vpvs.hplaor)->iaorMac; iaor < iaorMac; )
				{
				blt(PInPl(vpvs.hplaor, iaor++), &aor, cwAOR);
				FrameRectRop(hdc, &aor.rc, PATINVERT);
				}
			}
		}
	else
		{
/* find and draw outlines of headers, footers, abs objects */
		iaorMac = 0;
		if (vpvs.hplaor != hNil)
			(*vpvs.hplaor)->iaorMac = 0;
		hpllr = plbs->hpllr;
		Assert(hpllr);
		vpvs.rcHdr.xpLeft = vpvs.rcFtr.xpLeft = zpUnknown;    /* kill rc's */
		pt.yp = -1;     /* page break point */
		for (ilr = 0, ilrMac = (*hpllr)->ilrMac; ilr < ilrMac; ilr++)
			{
			bltLrp(LrpInPl(hpllr, ilr), &lr, sizeof(struct LR));
			lr.xl = NMultDiv(lr.xl, vfli.dxsInch, vfli.dxuInch);
			lr.dxl = NMultDiv(lr.dxl, vfli.dxsInch, vfli.dxuInch);
			lr.yl = NMultDiv(lr.yl, vfli.dysInch, vfli.dyuInch);
			lr.dyl = NMultDiv(lr.dyl, vfli.dysInch, vfli.dyuInch);
			if (lr.lrk == lrkAbs)
				{
				if (vmerr.fMemFail || vpvs.hplaor == hNil && (vpvs.hplaor = HplInit(sizeof(struct AOR), 1)) == hNil)
					continue;
				DrcToRc(&lr.drcl, &aor.rc);
				MapRect(&aor.rc, &vpvs.rcPagePrint, prc);
				aor.ilr = ilr;
				FInsertInPl(vpvs.hplaor, iaorMac++, &aor);
				if (!vmerr.fMemFail)
					FrameRectRop(hdc, &aor.rc, PATINVERT);
				continue;
				}

			if (lr.doc < 0 || lr.ihdt >= ihdtMaxSep)
				continue;
			if (lr.ihdt == ihdtNil)
				{
				pt.yp = max(lr.yl + lr.dyl, pt.yp); /* page break */
				continue;
				}

			prcHdrFtr = (FHeaderIhdt(lr.ihdt)) ? &vpvs.rcHdr : &vpvs.rcFtr;
			/* erase partial hdr box */
			if (prcHdrFtr->xpLeft >= 0)
				FrameRectRop(hdc, prcHdrFtr, PATINVERT);

			DrcToRc(&lr.drcl, &rcHdrFtr);
			MapRect(&rcHdrFtr, &vpvs.rcPagePrint, prc);
			/* force hdr/ftr just inside margins */
			rcHdrFtr.xpLeft = vpvs.rcMargin.xpLeft + xpClose;
			rcHdrFtr.xpRight = vpvs.rcMargin.xpRight - xpClose;
			if (prcHdrFtr->xpLeft >= 0)
				UnionRect((LPRECT) prcHdrFtr, (LPRECT) &rcHdrFtr, (LPRECT) prcHdrFtr);
			else
				*prcHdrFtr = rcHdrFtr;
			FrameRectRop(hdc, prcHdrFtr, PATINVERT);
			}

		if (!vpvs.lbsText.fCpOnPage || pt.yp < 0)
			vpvs.ypPageBreak = zpUnknown;     /* nothing to hook it to */
		else
			{
			/* scale the page break point to the display rc's */
			pt.xp = 0;
			MapPt(&pt, &vpvs.rcPagePrint, prc);
			vpvs.ypPageBreak = pt.yp;
			}
		}

/* draw page break - stays within margins */
	SelectObject(hdc, (vpvs.pbk == pbkNatural) ? vhbrDkGray : vhbrLtGray);
	PatBlt(hdc, vpvs.rcMargin.xpLeft + 1, vpvs.ypPageBreak,
			vpvs.rcMargin.xpRight - vpvs.rcMargin.xpLeft - 2, 1, PATINVERT);
	if (vpvs.pbk == pbkSection)
		{
		PatBlt(hdc, vpvs.rcMargin.xpLeft + 1, vpvs.ypPageBreak + 2,
				vpvs.rcMargin.xpRight - vpvs.rcMargin.xpLeft - 2, 1, PATINVERT);
		}
	vpvs.fAreasKnown = fTrue;
	RestoreDC(hdc, idcOld);
}


/* %%Function:YpFromCp %%Owner:davidbo */
YpFromCp(cp)
CP cp;
{
	/* Map a cp in the range [cp0..cpMacDoc] to a yp in the range
		[0..vpvs.ypLastScrlBar].
	*/
	CP cpMac = CpMacDoc(vpvs.docPrvw);
	return((int) ((cp*vpvs.ypLastScrlBar + cpMac - 1)/cpMac));
}


/* Display current page number(s) in the data box. */

/* %%Function:ShowPgnP %%Owner:davidbo */
ShowPgnP(ipgd, fOnePage)
int ipgd;
int fOnePage;
{
	struct PLC **hplcpgd = PdodDoc(vpvs.docPrvw)->hplcpgd;
	int ipgdMac, fGray, matOld;
	struct PGD pgd;
	char szPgn[ichMaxIDSTR];

	if (hplcpgd == hNil)
		return;

	ipgdMac = IMacPlc(hplcpgd);
	if (ipgd >= ipgdMac)
		{
		vpvs.cchIbLast = 0;
		szPgn[0] = 0;
		SetIibbTextHwndIb(vhwndPrvwIconBar, iibbPrvwStats, szPgn);
		return;
		}

	if (!vpref.fPrvwTwo || ipgd == ipgdMac - 1)
		fOnePage = fTrue;
	else  if (!fOnePage)
		{
		GetPlc(hplcpgd, ipgd, &pgd);
		if (pgd.fEmptyPage)
			{
			fOnePage = fTrue;
			ipgd = min(ipgd + 1, ipgdMac - 1);
			}
		else  if (vpvs.fLeftEmpty || vpvs.fRightEmpty)
			fOnePage = fTrue;
		else
			{
			GetPlc(hplcpgd, ipgd + 1, &pgd);
			if (pgd.fEmptyPage || pgd.fUnk)
				fOnePage = fTrue;
			}
		}

	vpvs.cchIbLast = CchPgnFromIpgd(szPgn,vpvs.docPrvw,ipgd,(fOnePage) ? -1 : ipgd+1);
	szPgn[vpvs.cchIbLast] = 0;
	fGray = !FValidPgn(vpvs.docPrvw,ipgd)||!(fOnePage||FValidPgn(vpvs.docPrvw,ipgd+1));
	matOld = vmerr.mat;
	SetIibbTextHwndIb(vhwndPrvwIconBar, iibbPrvwStats, szPgn);
	/* if above turned on matMem...map to matDisp */
	if (vmerr.mat == matMem && matOld != matMem)
		vmerr.mat = matDisp;
	GrayIibbHwndIb(vhwndPrvwIconBar, iibbPrvwStats, fGray);
}


/* %%Function:FValidPgn %%Owner:davidbo */
FValidPgn(doc, ipgd)
int doc, ipgd;
{
/* returns true if the page number can be trusted (this is not 100% accurate
	because it does not look at fPending and does not look ahead 1 page, but
	it's good enough for pgn display)
*/
	int ipgdT = 0;
	struct PLCPGD **hplcpgd = PdodDoc(doc)->hplcpgd;
	struct PGD pgd;

	AssertH(hplcpgd);
	while (ipgdT <= ipgd)
		{
		GetPlc(hplcpgd, ipgdT++, &pgd);
		if (pgd.fUnk)
			return(fFalse);
		}
	return(fTrue);
}


/* C C H  P G N  F R O M  I P G D */
/* %%Function:CchPgnFromIpgd %%Owner:davidbo */
CchPgnFromIpgd(pchFirst, doc, ipgd1, ipgd2)
char *pchFirst;
int doc, ipgd1, ipgd2;
{
/* produce a page number string from ipgd entries. if both ipgd1 and ipgd2
	are -1, "Page 1" is returned.  if ipgd2 is -1, "Page x" or "Px Sy" is
	returned.  if neither is -1, "Pages x-y" or "PwSx-PySz" or "Px-y Sz"
	is returned.  If a non-negative ipgd is given, hplcpgd is assumed to exist
*/
	char *pch = pchFirst;
	struct PLC **hplcsed;
	struct PLC **hplcpgd = PdodDoc(doc)->hplcpgd;
	int fSect = FMultipleSect(doc, &hplcsed);
	int ised1, ised2;
	CP cp1, cp2, cpMac;
	struct PGD pgd;

	Assert((ipgd1 < 0 && ipgd2 < 0) || hplcpgd != hNil);

	cpMac = CpMacDocEdit(doc);
	cp1 = (ipgd1 < 0) ? cp0 : CpMin(cpMac, CpPlc(hplcpgd, ipgd1));
	if (ipgd2 >= 0)
		cp2 = CpMin(cpMac, CpPlc(hplcpgd, ipgd2));
	if (fSect)
		{
		ised1 = IInPlc(hplcsed, cp1) + 1;
		ised2 = (ipgd2 < 0) ? ised1 : IInPlc(hplcsed, cp2) + 1;
		*pch++ = vitr.chPageNo;
		}
	else
		{
		if (ipgd2 < 0)
			CopyCsSz(szCsPage, pch);
		else
			CopyCsSz(szCsPages, pch);
		/* sizeof()-1 because sizeof counts 0 terminator */
		pch += ((ipgd2 < 0) ? sizeof(szCsPage)-1 : sizeof(szCsPages)-1);
		}
	CacheSect(doc, cp1);
	if (ipgd1 < 0)
		*pch++ = '1';
	else
		{
		GetPlc(hplcpgd, ipgd1, &pgd);
		pch += CchLongToRgchNfc((long)pgd.pgn,pch,vsepFetch.nfcPgn,cchMaxLong);
		}

	if (ipgd2 >= 0)
		{
		if (!fSect || ised1 == ised2)
			*pch++ = '-';   /* "Pages x-y" or "Px-y Sz" */
		else
			{
			*pch++ = vitr.chSectNo;   /* "PwSx-PySz" */
			CchIntToPpch(ised1, &pch);
			*pch++ = '-';
			*pch++ = vitr.chPageNo;
			}
		CacheSect(doc, cp2);
		GetPlc(hplcpgd, ipgd2, &pgd);
		pch += CchLongToRgchNfc((long)pgd.pgn,pch,vsepFetch.nfcPgn,cchMaxLong);
		}
	if (fSect)
		{
		if (ised2 < 0 || ised1 == ised2)
			*pch++ = ' ';
		*pch++ = vitr.chSectNo;
		CchIntToPpch(ised2, &pch);
		}
	return(pch - pchFirst);
}


/* %%Function:BlankPageRect %%Owner:davidbo */
BlankPageRect(hdc,prc)
HDC hdc;
struct RC *prc;
{
	/*  BlankPageRect - draws the border around rectangle *prc. */
	int dyp,dxp;
	struct RC rc;

	Assert(hdc);
	rc = *prc;
	dxp = rc.xpRight - rc.xpLeft;
	dyp = rc.ypBottom - rc.ypTop;
	PatBlt(hdc, rc.xpLeft, rc.ypTop, dxp, dyp, WHITENESS);
	PatBlt(hdc, rc.xpLeft, rc.ypTop, dxp, vsci.dypBorder, BLACKNESS);
	PatBlt(hdc, rc.xpLeft, rc.ypTop, vsci.dxpBorder, dyp, BLACKNESS);
	PatBlt(hdc, rc.xpLeft, rc.ypBottom, dxp, vsci.dypBorder, BLACKNESS);
	PatBlt(hdc, rc.xpRight, rc.ypTop, vsci.dxpBorder, dyp, BLACKNESS);
	PatBlt(hdc, rc.xpRight, rc.ypTop + 2*vsci.dypBorder,
			3*vsci.dxpBorder, dyp, BLACKNESS);
	PatBlt(hdc,rc.xpLeft + 2*vsci.dxpBorder, rc.ypBottom,
			dxp, 3*vsci.dypBorder, BLACKNESS);
}


/* %%Function:AnimateFlipPage %%Owner:davidbo */
AnimateFlipPage(hdc, direc)
HDC hdc;
int direc;
{
	/* Some nifty animation for turning pages when vpvs.fFacing is fTrue. */
	struct RC *prcFrom, *prcTo;
	struct RC rcSpine;

	Assert(hdc);
	Assert(vpvs.fFacing);
	if (direc < 0)
		{
		prcFrom = &vpvs.rcPage0;
		prcTo = &vpvs.rcPage1;
		}
	else
		{
		prcFrom = &vpvs.rcPage1;
		prcTo = &vpvs.rcPage0;
		}

	rcSpine = *prcFrom;
	rcSpine.xpLeft = ((vpvs.rcPage0.xpRight + vpvs.rcPage1.xpLeft)>>1) - 1;
	rcSpine.xpRight = rcSpine.xpLeft + 2;
	AnimateRect(hdc, prcFrom, &rcSpine);
	AnimateRect(hdc, &rcSpine, prcTo);
}


#define cRect 11
#define dRect 2
#define scaleOffset ((cRect+1)*cRect>>1)

/* Nifty animation routine to signify moving a rectangle from From to To. */
/* %%Function:AnimateRect %%Owner:davidbo */
AnimateRect(hdc,prcFrom, prcTo)
HDC hdc;
struct RC *prcFrom, *prcTo;
{
	int dxpLeft, dypTop, dxpRight, dypBottom;
	int iRect;
	int fBackwards;
	int scale;
	long lxpLeft1, lypTop1, lxpRight1, lypBottom1;
	long lxpLeft2, lypTop2, lxpRight2, lypBottom2;
	struct RC rc1;
	struct RC rc2;
	long LAreaFromPrc();

	Assert(hdc);
	fBackwards = (LAreaFromPrc(prcFrom) > LAreaFromPrc(prcTo));
	dxpLeft = prcTo->xpLeft - prcFrom->xpLeft;
	dypTop = prcTo->ypTop - prcFrom->ypTop;
	dxpRight = prcTo->xpRight - prcFrom->xpRight;
	dypBottom = prcTo->ypBottom - prcFrom->ypBottom;
	lxpLeft1 = lxpLeft2 = prcFrom->xpLeft * (long)scaleOffset;
	lypTop1 = lypTop2 = prcFrom->ypTop * (long)scaleOffset;
	lxpRight1 = lxpRight2 = prcFrom->xpRight * (long)scaleOffset;
	lypBottom1 = lypBottom2 = prcFrom->ypBottom * (long)scaleOffset;
	for (iRect = 0; iRect < cRect + dRect; iRect++)
		{
		if (iRect < cRect)
			{
			SetRect((LPRECT) &rc1,
					(int)(lxpLeft1 / scaleOffset), (int)(lypTop1 / scaleOffset),
					(int)(lxpRight1 / scaleOffset),(int)(lypBottom1 / scaleOffset));
			FrameRectRop(hdc,&rc1,PATINVERT);
			if (fBackwards)
				scale = cRect - iRect;
			else
				scale = iRect + 1;
			lxpLeft1 += dxpLeft * (long)scale;
			lypTop1 += dypTop * (long)scale;
			lxpRight1 += dxpRight * (long)scale;
			lypBottom1 += dypBottom * (long)scale;
			}
		if (iRect >= dRect)
			{
			SetRect((LPRECT) &rc2,
					(int)(lxpLeft2 / scaleOffset), (int)(lypTop2 / scaleOffset),
					(int)(lxpRight2 / scaleOffset),(int)(lypBottom2 / scaleOffset));
			FrameRectRop(hdc,&rc2,PATINVERT);
			if (fBackwards)
				scale = cRect + dRect - iRect;
			else
				scale = iRect - dRect + 1;
			lxpLeft2 += dxpLeft * (long)scale;
			lypTop2 += dypTop * (long)scale;
			lxpRight2 += dxpRight * (long)scale;
			lypBottom2 += dypBottom * (long)scale;
			}
		}
}


/* %%Function:Area %%Owner:davidbo */
long LAreaFromPrc(prc)
struct RC *prc;
{
	/* simply returns area of rectangle described by *prc */
	return(((long)(prc->xpRight-prc->xpLeft)*(long)(prc->ypBottom-prc->ypTop)));
}


/* Emulate Mac ROM call that moves *ppt from within rcFrom to a proportionally
	placed spot within rcTo.
*/
/* %%Function:MapPt %%Owner:davidbo */
MapPt(ppt, prcFrom, prcTo)
struct PT *ppt;
struct RC *prcFrom, *prcTo;
{
	int dxpIns, dypIns;
	int dxpFrom, dypFrom, dxpTo, dypTo;

	dxpFrom = prcFrom->xpRight - prcFrom->xpLeft;
	dypFrom = prcFrom->ypBottom - prcFrom->ypTop;
	dxpTo = prcTo->xpRight - prcTo->xpLeft;
	dypTo = prcTo->ypBottom - prcTo->ypTop;
	dxpIns = NMultDiv(ppt->xp - prcFrom->xpLeft, dxpTo, dxpFrom);
	dypIns = NMultDiv(ppt->yp - prcFrom->ypTop, dypTo, dypFrom);
	ppt->xp = prcTo->xpLeft + dxpIns;
	ppt->yp = prcTo->ypTop + dypIns;
}


/* Emulate Mac ROM call to map *prcRect from location within pprcFrom to
	proportional position within prcTo.
*/
/* %%Function:MapRect %%Owner:davidbo */
MapRect(prcRect, prcFrom, prcTo)
struct RC *prcRect, *prcFrom, *prcTo;
{
	MapPt(&(prcRect->ptTopLeft), prcFrom, prcTo);
	MapPt(&(prcRect->ptBottomRight), prcFrom, prcTo);
}


/* %%Function:FCopyPrvwFoo %%Owner:davidbo */
FCopyPrvwFoo(hplfooFrom, hplfooTo, pfoo)
struct PL **hplfooFrom, **hplfooTo;
char *pfoo;
{
	/* copy Foos from one Foo plex to another */
	int ifoo, ifooMacFrom, ifooMaxTo, cbFoo, fExternal;

	Assert(hplfooFrom);
	Assert(hplfooTo);
	ifooMacFrom = (*hplfooFrom)->iMac;
	ifooMaxTo = (*hplfooTo)->iMax;
	(*hplfooTo)->iMac = 0;
	cbFoo = (*hplfooFrom)->cb;
	fExternal = (*hplfooFrom)->fExternal;
	for (ifoo = 0; ifoo < ifooMacFrom; ifoo++)
		{
		/* have to copy: heap moves */
		if (fExternal)
			bltbh(HpInPl(hplfooFrom, ifoo), pfoo, cbFoo);
		else
			bltbyte(PInPl(hplfooFrom, ifoo), pfoo, cbFoo);

		if (ifoo < ifooMaxTo)
			{
			(*hplfooTo)->iMac++;
			if (fExternal)
				bltbh(pfoo, HpInPl(hplfooTo, ifoo), cbFoo);
			else
				bltbyte(pfoo, PInPl(hplfooTo, ifoo), cbFoo);
			}
		else  if (!FInsertInPl(hplfooTo, ifooMaxTo++, pfoo))
			return fFalse;
		}
	return fTrue;
}


/* %%Function:GenPrvwPlf %%Owner:davidbo */
EXPORT GenPrvwPlf(plf)
LOGFONT *plf;
{
	int lfHeight, lfWidth;

	Assert(vfPrvwDisp);
	lfHeight = abs(plf->lfHeight);
	lfWidth = lfHeight >> 1;
	lfHeight = NMultDiv(lfHeight, vpvs.rcPage0.ypBottom-vpvs.rcPage0.ypTop, dypPage);
	/* choose preview font if not OEM and request is less than 8 pts. */
	if (plf->lfCharSet != OEM_CHARSET  &&
			NMultDiv(lfHeight, 72, vfli.dysInch) < 8)
		{
		plf->lfWidth = lfWidth;
		plf->lfPitchAndFamily = FIXED_PITCH;
		CopyCsSz(szCsPrvwFont, plf->lfFaceName);
		}
#ifdef DAVIDBO
	/* test code for using vector font rather than preview font */
	plf->lfQuality = DRAFT_QUALITY;
	plf->lfCharSet = OEM_CHARSET;
	plf->lfPitchAndFamily = FF_ROMAN + VARIABLE_PITCH;
	bltbyte(SzSharedKey("Roman",Prev), plf->lfFaceName, 6);
#endif
}


#ifdef DEBUG /* near routine in formatn2.asm if !DEBUG */
/* %%Function:SelectPrvwFont %%Owner:davidbo */
EXPORT SelectPrvwFont(phfont, phdc)
HFONT *phfont;
HDC *phdc;
{
	*phdc = PwwdWw(wwLayout)->hdc;
	Assert(*phdc);

	if (OurSelectObject(*phdc, *phfont) == NULL)
		{
		*phfont = GetStockObject(SYSTEM_FONT);
		SelectObject(*phdc, *phfont);
		}
}


#endif /* DEBUG */


/* FPopupOverlap returns fTrue if a visible popup window overlaps *prc in
	hwnd, fFalse otherwise
*/
/* %%Function:FPopupOverlap %%Owner:davidbo */
FPopupOverlap(hwnd,prc)
HWND hwnd;
struct RC *prc;
{
	struct PUCHK puchk;
	extern FARPROC lpFCheckPopupRect;

	if (AnyPopup())
		{
		puchk.hwnd = hwnd;
		puchk.fPopupOverlap = fFalse;
		puchk.rc = *prc;
		EnumWindows(lpFCheckPopupRect,(LONG) (struct PUCHK FAR *) &puchk);
		if (puchk.fPopupOverlap)
			return fTrue;
		}
	return fFalse;
}


/* %%Function:FSyncLbs %%Owner:davidbo */
FSyncLbs()
{
	struct PLLR **hpllr;
	struct PLLNBC **hpllnbc;
	struct LBS *plbsFrom, *plbsTo;
	struct LNBC lnbc;
	struct LR lr;

	Assert(vpref.fPrvwTwo);

	/* copy everything but hpllr/hpllnbc */
	plbsFrom = &vpvs.lbsText;
	plbsTo = &vpvs.lbsLeft;
	hpllr = plbsTo->hpllr;
	hpllnbc = plbsTo->hpllnbc;
	*plbsTo = *plbsFrom;
	plbsTo->fRight = fFalse;
	plbsTo->hpllr = hpllr;
	plbsTo->hpllnbc = hpllnbc;

	/* Now copy the pllr */
	if (plbsTo->hpllr == hNil)
		{
		if ((plbsTo->hpllr = HCopyHeapBlock(plbsFrom->hpllr, strcPL)) == hNil)
			return fFalse;
		}
	else  if (!FCopyPrvwFoo(plbsFrom->hpllr, plbsTo->hpllr, &lr))
		return fFalse;

	/* Now copy the pllnbc */
	if (plbsTo->hpllnbc == hNil)
		{
		if ((plbsTo->hpllnbc=HCopyHeapBlock(plbsFrom->hpllnbc, strcPL)) == hNil)
			return fFalse;
		}
	else  if (!FCopyPrvwFoo(plbsFrom->hpllnbc, plbsTo->hpllnbc, &lnbc))
		return fFalse;

	plbsFrom = &vpvs.lbsFtn;
	plbsTo = &vpvs.lbsLeftFtn;
	hpllr = plbsTo->hpllr;
	*plbsTo = *plbsFrom;
	plbsTo->fRight = fFalse;
	plbsTo->hpllr = hpllr;

	/* Now copy the pllr */
	if (plbsTo->hpllr == hNil)
		{
		if ((plbsTo->hpllr = HCopyHeapBlock(plbsFrom->hpllr, strcPL)) == hNil)
			return fFalse;
		}
	else  if (!FCopyPrvwFoo(plbsFrom->hpllr, plbsTo->hpllr, &lr))
		return fFalse;

	return fTrue;
}


/* S H O W O L D W I N D O W S */
/* hide or show the desktop, ribbon, status line, macro iconbar */
/* %%Function:ShowOldWindows %%Owner:davidbo */
ShowOldWindows(wCmdShow)
int wCmdShow;
{
	Assert( vhwndDeskTop != NULL );
	ShowWindow( vhwndDeskTop, wCmdShow );
	if (vhwndRibbon)
		ShowWindow( vhwndRibbon, wCmdShow );
	if (vhwndStatLine)
		ShowWindow( vhwndStatLine, wCmdShow );
	if (vhwndAppIconBar)
		ShowWindow( vhwndAppIconBar, wCmdShow );
}
