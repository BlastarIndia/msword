/* cmd2.c -- Miscellaneous commands */

#ifdef PCJ
/* #define DEDITPIC */
#endif /* PCJ */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "cmdtbl.h"
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "ch.h"
#include "keys.h"
#include "prm.h"
#include "doc.h"
#define USEBCM
#include "cmd.h"
#include "profile.h"
#include "format.h"
#include "screen.h"
#include "heap.h"
#include "border.h" /* to get TCC */
#include "rerr.h"
#define AUTOSAVE
#include "autosave.h"
#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "iconbar.h"
#include "resource.h"
#include "error.h"
#include "pic.h"
#include "menu2.h"
#define RSBSONLY
#include "rsb.h"
#include "version.h"
#include "message.h"
#include "field.h"
#define RULER
#define REPEAT
#include "ruler.h"

#include "viewpref.hs"
#include "viewpref.sdm"

#include "autosave.hs"
#include "autosave.sdm"

extern ASD      asd;
extern CHAR szApp[];
extern BOOL fElActive;
extern struct SCI vsci;
extern struct WWD ** hwwdCur;
extern HMENU            vhMenu;
extern int              viMenu;
extern int		wwMac;
extern struct WWD **    mpwwhwwd[];
extern BOOL         vfFileCacheDirty;
extern BOOL         vfWndCacheDirty;
extern struct MWD ** hmwdCur;
extern HWND vhwndStatLine;
extern struct SAB vsab;
extern HWND       vhwndApp;
extern HANDLE vhInstance;
extern struct AAB       vaab;
extern BOOL vfExtendSel;
extern BOOL vfOvertypeMode;
extern CHAR HUGE        *vhpchFetch;
extern struct RULSS  vrulss;
extern RRF		rrf;
extern HWND		vhwndRibbon;
extern BOOL vfBlockSel;
extern struct SEL selCur;
extern KMP ** hkmpCur;
extern struct CHP vchpFetch;
extern int wwCur;
extern int mwCur;
extern struct PREF vpref;
extern struct STTB ** vhsttbWnd;
extern struct UAB vuab;
extern struct MWD **mpmwhmwd[];
extern struct RC vrcUnZoom;
extern CP  vcpFetch;
extern int mwMac;
extern int vmwClipboard;
extern struct TCC vtcc;
extern int vlm;
extern struct PIC   vpicFetch;
extern CP   vcpFetch;
extern int  vccpFetch;
extern struct BMI mpidrbbmi [];
extern struct MERR vmerr;
extern HWND vhwndPgvCtl;
extern BOOL vfRecording;
extern struct PAP vpapFetch;

CP CpFirstBlock();

/*  %%Function: Overtype  %%Owner: bradch  */

/* REVIEW bradch (rp): Is this even used? */

#ifdef OPEL
FUNCTION STATEMENT Overtype(fOn)
OPTIONAL BOOL fOn;
{
	extern BOOL vfOvertypeMode;

	if (fOn == fNil)
		return vfOvertypeMode ? -1 : 0;

	if (fOn == 0 && vfOvertypeMode == 0)
		return; /* nothing to do */

	/* Cannot go into Overtype mode if already in Revision Marking mode */
	if (fOn != 0 && PdodMother(selCur.doc)->dop.fRevMarking)
		{
		Beep();
		return;
		}
	vfOvertypeMode = fOn;

	if (vhwndStatLine != NULL)
		UpdateStatusLine(usoToggles);
}


#endif /* OPEL */

/*  %%Function: BlockModeEnd  %%Owner: chic  */

BlockModeEnd()
{
	if (vfBlockSel)
		{
		Assert(hmwdCur != hNil);
		ToggleBlockSel();
		}
}


/*  %%Function: ToggleBlockSel  %%Owner: chic  */

ToggleBlockSel()
{
	vfBlockSel = !vfBlockSel;
	if (vhwndStatLine != NULL)
		UpdateStatusLine(usoToggles);
}


/*  %%Function: CmdBlkExtend  %%Owner: chic  */

CMD CmdBlkExtend(pcmb)
CMB *pcmb;
{
	CP ccpFetch;

	if ((*hwwdCur)->fOutline)
		{
LBeep:
		Beep();
		return cmdError;
		}
	FetchCpPccpVisible(selCur.doc, selCur.cpFirst, &ccpFetch, wwCur, fFalse);
	if (selCur.cpFirst != vcpFetch)
		goto LBeep;
	if (FInTableDocCp(selCur.doc, selCur.cpFirst))
		/* make it the same as extending the selection, not a block sel */
		return CmdExtend(pcmb);

	if (vfBlockSel) /* it is a toggle key */
		{
		BlockModeEnd();
		SelectIns(&selCur, CpFirstBlock());
		}
	else 		
		{
		ToggleBlockSel();
/* get ready for block selection, collapsed into an insertion point */
		if (!selCur.fBlock)
			{
			BlockSelBegin(&selCur);
			selCur.cpFirst = selCur.cpLast = selCur.cpAnchor;
			/* make something that is visible */
			selCur.xpLim++;
			MarkSelsBlock(wwCur, &selCur);
			}
		}

	return cmdOK;
}


/* K E Y  C M D  D E L  W O R D */
/* delete word, forward or backward from beginning of current selection.
	i.e.  we collapse the selection first.  AutoDelete does NOT cause
	the selection to be deleted first in this case.

	fScrap indicates whether deleted item goes to the scrap/clipboard.
*/

/* Note:  If this is compiled with AI_DELETE defined, word deletion
			will work as follows (. denotes whitespace, | denotes
			starting insertion point or beginning of sel, ^ indicates
			the cp where we delete to (non-inclusive in fwd case,
			inclusive in backward case)):

					Forward:          |           Backward:
									|
			word....|word....word    |    word....word|....word
							^       |        ^
			word..|..word....word    |    word....word..|..word
							^       |        ^
			word....wo|rd....word    |    word....wo|rd....word
						^           |            ^
			word|....word....word    |    word....word....|word
						^           |            ^

			If AI_DELETE is not defined, word deletion will work as
			in MacWord.
*/

#define AI_DELETE

/*  %%Function: KeyCmdDelWord  %%Owner: rosiep  */

KeyCmdDelWord( fForward, fScrap )
BOOL fForward;
BOOL fScrap;
{
	extern int vfSeeSel;
	CP   cpFirst, cpLim, cpT;
	struct CA caT, caRM;
#ifdef AI_DELETE
	BOOL fInWord = fFalse;
	CP   cpLimNonWhite;

	fInWord = (selCur.cpFirst != CpFirstSty(wwCur, selCur.doc,selCur.cpFirst, styWord, fFalse));

	cpLimNonWhite = CpLimSty( wwCur, selCur.doc,CpFirstSty( wwCur, selCur.doc,selCur.cpFirst, styWord, fTrue ),
			styWordNoSpace, fFalse );
#endif /* AI_DELETE */

	if (selCur.cpFirst == cp0 && !fForward)
		{
LBeep:
		Beep();
		return;
		}

	/* Block Sel mode will have already been terminated, but there
		may be a block selection, in which case we need to get the
		appropriate cpFirst of the sel, and collapse the selection
		to there */

	if (selCur.fBlock)  /* includes table sel */
		{
		struct BKS bks;
		CP cpFirst, dcp;

		InitBlockStat(&bks);
		if (FGetBlockLine(&cpFirst, &dcp, &bks))
			SelectIns(&selCur,cpFirst);
		else
			goto LBeep;
		}

	/* Don't allow deleting a table cell mark */
	if (FInTableDocCp(selCur.doc, selCur.cpFirst - (fForward ? 0 : 1)))
		{
		CacheTc(wwNil, selCur.doc, selCur.cpFirst - (fForward ? 0 : 1), fFalse, fFalse);
		if (selCur.cpFirst == vtcc.cpLim - (fForward ? ccpEop : 0))
			goto LBeep;
		}

	/* Don't allow deleting fSpec or vanished characters */
	FetchCpAndPara(selCur.doc, selCur.cpFirst - (fForward ? 0 : 1), fcmProps);
	if (vchpFetch.fSpec || ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
			(!((*hwwdCur)->grpfvisi.fSeeHidden || (*hwwdCur)->grpfvisi.fvisiShowAll)
			|| vchpFetch.fSysVanish)))
		goto LBeep;

	if (fForward)
		{
		cpFirst = selCur.cpFirst;
		cpLim = CpLimSty( wwCur, selCur.doc,selCur.cpFirst, styWord, fFalse );

#ifdef AI_DELETE
		if (fInWord)
			{
			if (cpFirst == cpLimNonWhite)
				cpLim = CpLimSty( wwCur, selCur.doc,cpLim, styWordNoSpace, fFalse );
			else  if (cpFirst > cpLimNonWhite)
				cpLim = CpLimSty( wwCur, selCur.doc,cpLim, styWord, fFalse );
			else
				cpLim = CpLimSty( wwCur, selCur.doc,cpFirst, styWordNoSpace, fFalse );
			}
#endif /* AI_DELETE */

		/* limit deletion to not delete fSpec or vanished text */
		for (cpT = cpFirst; cpT < cpLim; cpT += vccpFetch)
			{
			FetchCpAndPara(selCur.doc, cpT, fcmProps);
			if (vchpFetch.fSpec || ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
					(!((*hwwdCur)->grpfvisi.fSeeHidden || (*hwwdCur)->grpfvisi.fvisiShowAll)
					|| vchpFetch.fSysVanish)))
				{
				cpLim = vcpFetch;
				break;
				}
			}
		}
	else
		{
		cpLim = selCur.cpFirst;  /* delete from beginning of selection */
		cpFirst = CpFirstSty( wwCur, selCur.doc,selCur.cpFirst, styWord, fTrue );
#ifdef AI_DELETE
		if (fInWord && selCur.cpFirst >= cpLimNonWhite && cpFirst != cp0)
			cpFirst = CpLimSty( wwCur, selCur.doc,CpFirstSty( wwCur, selCur.doc,cpFirst, styWord, fTrue ),
					styWordNoSpace, fFalse );
#endif /* AI_DELETE */
		/* limit deletion to not delete fSpec or vanished text */
		for (cpT = cpFirst; cpT < cpLim; cpT += vccpFetch)
			{
			FetchCpAndPara(selCur.doc, cpT, fcmProps);
			if (vchpFetch.fSpec || ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
					(!((*hwwdCur)->grpfvisi.fSeeHidden || (*hwwdCur)->grpfvisi.fvisiShowAll)
					|| vchpFetch.fSysVanish)))
				cpFirst = vcpFetch + vccpFetch;
			}
		}

	AssureLegalSel( PcaSet( &caT, selCur.doc, cpFirst, cpLim ));
	if (cpFirst == caT.cpFirst && cpLim == caT.cpLim)
		{
		caT = selCur.ca;
		if (!FSetUndoBefore(bcmDeleteWord, uccDeleteSel))
			return cmdCancelled;
		TurnOffSel(&selCur);
		Select( &selCur, cpFirst, cpLim );
		/* we did a select, so no block will be needed here */
		/* note that FDelSel leaves caRm as nil in case of block sel;
		block sel handles its own sel and undo; if we could have
		a block here, we would have to skip the setundo and select. bz
		*/
		Assert (!selCur.fBlock);
		FDelSel(fScrap, fFalse  /* fBlockOk */, &caRM);
		if (caRM.doc != docNil)
			{
			SelectPostDelete( fForward ? caRM.cpLim : caRM.cpFirst );
			SetUndoAfter(&caRM);
			}
		else
			{
			Select( &selCur, caT.cpFirst, caT.cpLim);
			SetUndoNil();
			}
		UpdateWw( wwCur, fFalse /* fAbortOk*/);
		vfSeeSel = fTrue;
		}
	else
		Beep();
}



/* C M D  D E L  W O R D  C L E A R */
/* Delete one word forwards, not putting result in scrap */
/*  %%Function: CmdDelWordClear  %%Owner: rosiep  */

CMD CmdDelWordClear(pcmb)
CMB *pcmb;
{
	KeyCmdDelWord(fTrue /* fForward */, fFalse /* fScrap */);
	return cmdOK;
}



/* C M D  D E L  B A C K  W O R D  C L E A R */
/* Delete one word bacwkards, not putting result in scrap */
/*  %%Function: CmdDelBackWordClear  %%Owner: rosiep  */

CMD CmdDelBackWordClear(pcmb)
CMB *pcmb;
{
	KeyCmdDelWord(fFalse /* fForward */, fFalse /* fScrap */);
	return cmdOK;
}




/*  %%Function: CmdViewPreferences  %%Owner: rosiep  */

CMD CmdViewPreferences(pcmb)
CMB *pcmb;
{
	extern struct PREF vpref;
	extern struct MWD ** hmwdCur;
	extern struct WWD ** hwwdCur;
	extern struct SEL selCur;
	extern int wwCur;
	extern int mwCur;
	extern struct FLI vfli;
	extern int vflm;
	GRPFVISI grpfvisi;
	GRPFVISI grpfvisiSave;
	int dxwStyWnd;
	BOOL fForceBlockToIp;

	CABVIEWPREF * pcabviewpref;
	Assert(hwwdCur && hmwdCur);

	grpfvisi = (*hwwdCur)->grpfvisi;
	if (FCmdFillCab())
		{
		/* initialize Command Argument Block */
		FreezeHp();
		pcabviewpref = (CABVIEWPREF *) *pcmb->hcab;
		pcabviewpref->fvisiTabs      = grpfvisi.fvisiTabs;
		pcabviewpref->fvisiSpaces    = grpfvisi.fvisiSpaces;
		pcabviewpref->fvisiParaMarks = grpfvisi.fvisiParaMarks;
		pcabviewpref->fvisiCondHyp   = grpfvisi.fvisiCondHyphens;
		pcabviewpref->fSeeHidden     = grpfvisi.fSeeHidden;
		pcabviewpref->fvisiShowAll   = grpfvisi.fvisiShowAll;

		pcabviewpref->fTextBound     = grpfvisi.fDrawPageDrs;
		pcabviewpref->fTblGridlines  = grpfvisi.fDrawTableDrs;

		pcabviewpref->fDispAsPrint   = (*hwwdCur)->fDisplayAsPrint;

		pcabviewpref->fShowPictures = !grpfvisi.fNoShowPictures;
		pcabviewpref->fHorzScroll    = (*hmwdCur)->fHorzScrollBar;
		pcabviewpref->fVertScroll    = (*hmwdCur)->fVertScrollBar;
		pcabviewpref->uNmAreaSize    = DxaFromDxs(0/*dummy*/, ((*hwwdCur)->xwSelBar));
		MeltHp();
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltViewPref)];

		BltDlt(dltViewPref, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}

		}


	if (FCmdAction())
		{
		BOOL fTrashAll;
		int wk;

		pcabviewpref = (CABVIEWPREF *) *pcmb->hcab;

		if (pcabviewpref->fvisiShowAll != grpfvisi.fvisiShowAll)
			{
			TrashFltg(fltgOther);
			selCur.fUpdateRibbon = fTrue;
			}

/* This call assures that selCur.chp is up to date.  It is necessary
	in case fSeeHidden changes -- we need to keep the old transient
	insertion point properties. */

		GetSelCurChp(fFalse);
		FreezeHp();
		/* Set preferences for this window and future windows */
		/* Set global preferences for all windows. */
		grpfvisi.fvisiTabs = pcabviewpref->fvisiTabs;
		grpfvisi.fvisiSpaces = pcabviewpref->fvisiSpaces;
		grpfvisi.fvisiParaMarks = pcabviewpref->fvisiParaMarks;
		grpfvisi.fvisiCondHyphens = pcabviewpref->fvisiCondHyp;
		grpfvisi.fvisiShowAll     = pcabviewpref->fvisiShowAll;
		grpfvisi.fSeeHidden = pcabviewpref->fSeeHidden;
		grpfvisi.fNoShowPictures = !pcabviewpref->fShowPictures;

		grpfvisi.fDrawPageDrs = pcabviewpref->fTextBound;
		grpfvisi.fDrawTableDrs = pcabviewpref->fTblGridlines;
		grpfvisi.flm = FlmDisplayFromFlags( pcabviewpref->fDispAsPrint,
				(*hwwdCur)->fPageView, vpref.fDraftView);

		(*hwwdCur)->fDisplayAsPrint = vpref.fDisplayAsPrint = 
				pcabviewpref->fDispAsPrint;

		vpref.grpfvisi = grpfvisi;
		grpfvisiSave = (*hwwdCur)->grpfvisi;
		(*hwwdCur)->grpfvisi = grpfvisi;
		fForceBlockToIp = grpfvisiSave.fvisiShowAll != grpfvisi.fvisiShowAll ||
				(grpfvisiSave.fSeeHidden != grpfvisi.fSeeHidden &&
				!grpfvisi.fSeeHidden && !grpfvisi.fvisiShowAll);
		/* if changing a pageview pane,
			change all of the doc's pageview panes */
		if ((*hwwdCur)->fPageView)
			SyncPageViewGrpfvisi(DocMother(selCur.doc), grpfvisi,
					pcabviewpref->fDispAsPrint);

		/* Set global preferences for future windows */
		vpref.fHorzScrollBar = pcabviewpref->fHorzScroll;
		vpref.fVertScrollBar = pcabviewpref->fVertScroll;
		MeltHp();

		TrashWw(wwCur);
		/* Sync current mw with scroll bar settings in vpref */
		SetScrollBarsMw( mwCur );

		pcabviewpref = (CABVIEWPREF *) *pcmb->hcab;
		dxwStyWnd = DxsFromDxa(0/*dummy*/, (pcabviewpref->uNmAreaSize));
		vpref.dxaStyWnd = pcabviewpref->uNmAreaSize;

		wk = PwwdWw(wwCur)->wk;
		if (wk != wkMacro && wk != wkClipboard && wk != wkPage &&
				dxwStyWnd != PwwdWw(wwCur)->xwSelBar)
			{
			ShowStyWnd(mwCur, dxwStyWnd, fTrue/*fSetDxw*/);
			}
		MakeSelCurVisi(fForceBlockToIp);
		SyncSbHor(wwCur);
		}

	return cmdOK;
}


/* Set scroll bars for passed MWD according to current view prefs */
/*  %%Function: SetScrollBarsMw  %%Owner: rosiep  */

SetScrollBarsMw( mw )
int mw;
{
	int wwUpper, wwLower, wwActive;

	struct RC rc, rcTopPane, rcTopScrlBar, rcSplitBox, rcSplitBar,
	rcBottomPane, rcBottomScrlBar;
	int fVertCreate, fHorzCreate;
	int dfVert, dfHorz;
	struct MWD **hmwd = mpmwhmwd [mw];

	Assert( hmwd != hNil );

	wwActive = (*hmwd)->wwActive;
	wwUpper = (*hmwd)->wwUpper;
	wwLower = WwOther(wwUpper);

/* df: 0=no change, 1=add bar, -1=delete bar */
	dfVert = vpref.fVertScrollBar - (*hmwd)->fVertScrollBar;
	dfHorz = vpref.fHorzScrollBar - (*hmwd)->fHorzScrollBar;
	if (dfVert == 0 && dfHorz == 0)
		return;

/* destroy scroll bars if necessary */
	if (dfVert == -1)
		{	/* Destroy vertical scroll bar(s) */
		DestroyPhwnd( &(*hmwd)->hwndSplitBox );
		DestroyVScrlWw(wwUpper);
		if ((*hmwd)->fSplit)
			{
			DestroyVScrlWw(wwLower);
			}
		(*hmwd)->fVertScrollBar = fFalse;
		}
	if (dfHorz == -1)
		{	/* Destroy horizontal scroll bar */
		DestroyPhwnd( &(*hmwd)->hwndHScroll );
		(*hmwd)->fHorzScrollBar = fFalse;
		}
	if ((*hmwd)->hwndSizeBox && (dfVert == -1 || dfHorz == -1))
		DestroyPhwnd( &(*hmwd)->hwndSizeBox );

/* Get subwindow dimensions */
	GetDocAreaPrc( *hmwd, &rc );
	GetDocAreaChildRcs( hmwd, rc, 
			PwwdWw(wwUpper)->wk,
			(*hmwd)->fSplit ? (*hmwd)->ypSplit : 0,
			&rcTopPane, &rcTopScrlBar, &rcSplitBox, &rcSplitBar,
			&rcBottomPane, &rcBottomScrlBar );

/* Create scroll bars if necessary */
	fVertCreate = (dfVert == 1);
	fHorzCreate = (dfHorz == 1);

/* Hack to keep top scroll bar from flickering */
	if (fVertCreate)
		MoveWindowRc( PwwdWw(wwUpper)->hwnd, &rcTopPane, fTrue );

	if (fVertCreate || fHorzCreate)
		{
		if (!FCreateScrollBars( &fVertCreate, &fHorzCreate, 
				wwActive,
				*hmwd, rc, rcTopScrlBar,
				rcSplitBox, rcBottomScrlBar,
				(*hmwd)->fSplit ))
			{
			dfVert = min( dfVert, fVertCreate );
			dfHorz = min( dfVert, fHorzCreate );
			SetErrorMat(matMem);
			}
		UpdateControlsMw(mw);
		SetScrollHoriz(wwActive, DxpScrollHorizWw(wwActive));
		}
/* resize windows to accomodate scroll bar changes */
	AdjustMwdArea( hmwd,
			dfVert * -vsci.dxpScrlBar, dfHorz * -vsci.dypScrlBar, fFalse );
}


/*  %%Function: FDlgViewPref  %%Owner: rosiep  */

BOOL FDlgViewPref(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int wwOther;
	int wk;

	switch (dlm)
		{
	case dlmInit:
		if (hwwdCur != hNil)
			{
			wk = (*hwwdCur)->wk;
			if (wk == wkMacro || wk == wkPage ||
					((wwOther = WwOther(wwCur)) != wwNil && PwwdWw(wwOther)->fPageView))
				EnableTmc(tmcVPNmAreaSize, fFalse);
			}
		break;

	case dlmTerm:
		if (tmc == tmcCancel)
			break;

		return ValGetTmc(tmcVPNmAreaSize) != wError;
		}

	return fTrue;
}


/*  %%Function: WParseStyleNameSize  %%Owner: davidbo  */

WORD WParseStyleNameSize(tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	extern struct PREF vpref;
	extern struct FLI vfli;	/* For DxaFromDxs macro */

	WORD wVal;
	int dxsMin;
	struct RC rc;

	switch (tmm)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		return 0;
#endif

#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif

#ifdef tmmFree	
	case tmmFree:
		break;
#endif

	case tmmFormat:
#define WFromPpvB(ppv, b) *((int *) PvParseArg(ppv, bArg))
#define SetPpvBToW(ppv, b, w) *((int *) PvParseArg(ppv, bArg)) = w

		CchExpZa(&sz, WFromPpvB(ppv, bArg), vpref.ut, ichMaxNum, fTrue);
		*sz = '\0';
		break;

	case tmmParse:
		Assert(hmwdCur); /* catch in case we allow View/Pref w/o a mwd */
		GetClientRect((*hmwdCur)->hwnd, (LPRECT)&rc);
		dxsMin = rc.xpRight >> 1;
		if (DpvParseFdxa(&wVal, tmc, sz, 0, DxaFromDxs(0/*dummy*/, dxsMin), 
				dpvNormal, eidOutOfRange, 
				fTrue /* fDxa */, vpref.ut) == dpvNormal)
			{
			SetPpvBToW(ppv, bArg, wVal);
			return fTrue;
			}

		return fFalse;
		}

	return 0;
}


#ifdef UNUSED

/*  %%Function: IFromBrcp  %%Owner: notused */

uns IFromBrcp(brcp)
int brcp;
{
	/* Sorry for this switch statement, but there is no convenient 
			mapping because of the way brcp is coded. */

	switch (brcp)
		{
	default:
		/* At this point it doesn't matter what you return. */
		return uNinch;

	case brcpNone:
		return 0;

	case brcpBox:
		return 1;

	case brcpBar:
		return 2;

	case brcpAbove:
		return 3;

	case brcpBelow:
		return 4;
		}
}


#endif /* UNUSED */





/* Repeat: Repeat the last command. */
/*  %%Function: CmdRepeat  %%Owner: bradch  */

CMD CmdRepeat(pcmb)
CMB * pcmb;
{
	CP CpBackspaceDocCpDcp();
	extern struct SEL selCur;
	extern struct CA  caPara;

	BOOL fMove = fFalse;
	CMD cmd = cmdOK;
	int docTmp;
	CP cpStart;
	struct CA caT, caT1, caT2;
	CMB cmb;
	BCM bcm;
	int fBackspace=fFalse;

	bcm = vaab.bcm;
#ifdef DISABLE
	CommSzNum(SzShared("CmdRepeat: bcm="), bcm);
#endif

	if (bcm == bcmNil || !FRepeatableBcm(bcm))
		{
LBeep:
		Beep();
		return cmdError;
		}

	/* Deal with weird cases before setting up cmb... */
	switch (bcm)
		{
	case imiSearch:
	case imiGoto:
		/* These two are collapsed to FindNext. */
		bcm = bcmFindNext;
		break;

	case bcmSplitTable:
		/* SplitTable is what InsColumnBreak does when in a table */
		bcm = bcmInsColumnBreak;
		break;

	case bcmTyping:
		/* Document is locked (annotations feature) */
		if (PdodDoc(selCur.doc)->fLockForEdit)
			goto LBeep;

		if (selCur.fTable)
			{
			/* Select the first cell */
			if (!FDeleteCheck(fFalse, rpkText, NULL))
				return cmdError;
			}

		CheckCollapseBlock();

		/* so that uab will not be combined. */
		if (!FSetUndoBefore(bcmTypingAgain, uccInsert))
			{
			return cmdCancelled;
			}

		/* paste over new selection */
		Assert(vaab.doc != docNil);
		docTmp = DocCreate(fnNil);
		if (docTmp == docNil)
			goto LBeep;

		/* Unformatted until proven otherwise... */
		PdodDoc(docTmp)->fFormatted = fFalse;

		SetWholeDoc(docTmp, 
				PcaSet(&caT, vaab.doc, vaab.cpLow, vaab.cpLim));
		if (vmerr.fMemFail)
			goto LError;

		if (vaab.cpLow != vaab.cpFirst)	    /* They backspaced */
			{
			CP dcp;

			Select(&selCur, CpBackspaceDocCpDcp(selCur.doc, 
					selCur.cpFirst, 
					dcp = (vaab.cpFirst - vaab.cpLow)),
					selCur.cpLim);
			fBackspace = fTrue;

			if (vfRecording)
				{
				while (dcp-- > 0)
					RecordBksp();
				}
			}

		/* deselect trailing delimiter, as insert does. */
		if (selCur.fIns)
			ClearInsertLine(&selCur);
		else  if (!selCur.fTable)
			{
			CachePara(selCur.doc, selCur.cpLim - 1);
			if (caPara.cpLim == selCur.cpLim && !vpapFetch.fInTable)
				{
				int ch;

				/* check that the first character inserted 
					is not chCRJ, Sect, or Eop */
				CachePara(docTmp, cp0);
				FetchCp(docTmp, cp0, fcmChars);
				ch = *vhpchFetch;
				CacheSect(selCur.doc, selCur.cpLim - 1);
				if (ch != chCRJ && 
						ch != chSect && 
						ch != chReturn && 
		/* section break is treated as paragraph break, don't backup ccpEop in that case */
				caSect.cpLim != selCur.cpLim)
					{
					Select(&selCur, selCur.cpFirst, 
							selCur.cpLim - ccpEop);
					}
				}

			if (selCur.sty == styWord)
				{
				CP cpT = CpLimNoSpaces(&selCur.ca);
				if (cpT != selCur.cpLim)
					Select(&selCur, selCur.cpFirst, cpT);
				}
			}

		cpStart = selCur.cpFirst;
		caT1 = selCur.ca;
		PcaSetWholeDoc(&caT2, docTmp);
		if (!vpref.fAutoDelete && !fBackspace)
			{
			caT1.cpLim = caT1.cpFirst;
			}
/* Note: FDeleteCheck assumes what we propose to delete is selCur */
		else  if (!FDeleteCheck(fFalse, rpkCa, &caT2))
			{
			Beep();
			goto LError;
			}
		else  if ((caT1 = selCur.ca, DcpCa(&caT1) != cp0) && 
				!FSetUndoDelete(&caT1, fFalse /* fScrap */))
			{
LError:
			DisposeDoc(docTmp);
			return cmdError;
			}

		if (vfRecording)
			RecordRepeatTyping(&caT2);

		if (!FReplaceCpsRM(&caT1, &caT2))
			goto LError;
		caT1.cpLim = caT1.cpFirst + DcpCa(&caT2) + DcpCa(&caT1);
		AssureNestedPropsCorrect(&caT1, fTrue);
		SelectPostDelete(caT1.cpLim); /* ensure back up to CpMacDocEdit */
		SetUndoAfter(&caT1);
		if (vaab.doc != docNil)
			{
/* vaab.doc could be destroyed due to auto delete on, e.g. source is in header
display doc and destination includes a section break, when the section break is
deleted, any header window showing headers for that section will be destroyed, 
so the header display doc can also be destroyed. */
			SetAgain(bcmTyping);
			if (vaab.cpLim > cpStart && vaab.doc == selCur.doc)
				{      /* Need to update cp's */
				/* Set the again block to the new(!!) text
				this allows agains in the middle of
				previous agains */
				SetAgainCp(vaab.doc,
						cpStart + (vaab.cpFirst - vaab.cpLow),
						cpStart,
						cpStart + (vaab.cpLim - vaab.cpLow));
				}
			}

		DisposeDoc(docTmp);
		NormCp(wwCur, selCur.doc, selCur.cpFirst,
				ncpHoriz+ncpVisifyPartialDl, 0, selCur.fInsEnd);

		return cmdOK;

	case bcmRRFormat:
		if (PdodDoc(selCur.doc)->fLockForEdit)
			goto LBeep;
		if (vrulss.caRulerSprm.doc != docNil)
			{
			rrf.fNoClear = fTrue;
			FlushRulerSprms();
			rrf.fNoClear = fFalse;
			}

		if (!(rrf.fSelMoved && rrf.fDirty &&
				((rrf.fRibbon && vhwndRibbon != NULL) ||
				(!rrf.fRibbon && (*hmwdCur)->hwndRuler != NULL))))
			{
			goto LBeep;
			}

		return FRepeatRulerRibbon() ? cmdOK : cmdError;

	case bcmFormatting:
			{
			CHAR grpprl [cbMaxGrpprl];

			if (PdodDoc(selCur.doc)->fLockForEdit ||
					vaab.hgrpprl == hNil || vaab.cbGrpprl == 0)
				goto LBeep;

			bltbyte(*(vaab.hgrpprl), grpprl, vaab.cbGrpprl);
			ApplyGrpprlSelCur(grpprl, vaab.cbGrpprl, fTrue);

			if (vfRecording)
				RecordGrpprl(grpprl, vaab.cbGrpprl);

			return (vmerr.fMemFail ? cmdError : cmdOK);
			}

	case bcmCopySp:
	case bcmMoveSp:
		/* we know the dest doc is selCur.doc for repeat, so
			we can bag out now if locked, while we can't for 1st time copy
		*/
		if (PdodMother(selCur.doc)->fLockForEdit)
			goto LBeep;
		if ((vuab.uac == uacUndoNil) || (vuab.ca.doc == docNil))
			goto LBeep;
		}  /* switch */

	AssertDo(FInitCmb(&cmb, bcm, vaab.hcab, 
		cmmNormal | cmmBuiltIn)); /* Can't fail! */
	cmb.tmc = tmcOK;

	switch (bcm)
		{
/* Repeatable command group, no dialog box. */
	default:
		cmb.fDefaults = fFalse;
		cmb.fDialog = fFalse;
		cmb.fCheck = fTrue;
		/* FALL THROUGH */

/* Repeatable command group, with a dialog box (or computation). */
	case imiPrintMerge:
	case imiNew:
	case imiOpen:
	case imiReplace:
	case imiInsFile:
	case imiDefineStyle:
	case imiSpelling:
	case imiThesaurus:
	case imiHyphenate:
	case imiCompare:
	case imiSort:
	case imiChangeKeys:
	case imiAssignToMenu:
	case bcmChangeCase:
	case imiInsPicture:
		cmb.fRepeated = fTrue;
		cmd = CmdExecCmb(&cmb);

		if (vfRecording && !cmb.fDialog)
			{
			FetchCm(cmb.bcm);
			if (vpsyFetch->mct == mctSdm)
				{
				FRecordCab(cmb.hcab, HidFromPsy(vpsyFetch), 
						cmb.tmc, cmb.bcm);
				}
			else  if (vpsyFetch->mct == mctCmd)
				GenStatement(cmb.bcm);
			}
		/* break; */
		}

	if (vaab.bcm == bcmNil)
		/* command may have invalidated Again, assuming command
			dispatcher will set it. */
		SetAgain(bcm);

	return cmd;
}


/*  %%Function: HidFromPsy  %%Owner: bradch  */

HidFromPsy(psy)
SY * psy;
{
	return HidFromIeldi(IeldiFromPsy(psy));
}


/*  %%Function: FRepeatRulerRibbon  %%Owner: bobz  */

FRepeatRulerRibbon()
{
	int		cb;
	struct CA	ca, caInval;
	struct SEBL	sebl;
	CHAR		grpprl[cbMaxGrpprl];

	if (!rrf.fSelMoved || !rrf.fDirty)
		{
		return fFalse;
		}

	cb = 0;
	grpprl[0] = sprmNoop;
	if (rrf.fRibbon)
		{
		/* Repeat ribbon stuff. */
		if (rrf.fBold)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCFBold, 1);
			}
		if (rrf.fItalic)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCFItalic, 1);
			}
		if (rrf.fSCaps)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCFSmallCaps, 1);
			}
		if (rrf.kul != kulNone)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCKul, rrf.kul);
			}
		if (rrf.iSuperSub != iSSNormal)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCHpsPos,
					(rrf.iSuperSub == iSSSuper) ? 6 : -6);
			}
		if (rrf.ftc != ftcNil)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCFtc, rrf.ftc);
			}
		if (rrf.hps != 0)
			{
			cb = CbMergeInOnePrl(grpprl, cb, sprmCHps, rrf.hps);
			}

		ApplyGrpprlSelCur(grpprl, cb, fTrue);

		if (vmerr.fMemFail)
			return fFalse;
		}
	else  /* ruler */		
		{
		ca = selCur.ca;
		ExpandCaSprm(&ca, &caInval, grpprl);
		if (!FSetUndoB1(bcmFormatting, uccFormat, &ca))
			{
			return fFalse;
			}
		if (rrf.cbGrpprl != 0)
			{
			Assert(rrf.hgrpprl != hNil);
			bltbyte(*(rrf.hgrpprl), grpprl, cb = rrf.cbGrpprl);
			ApplyGrpprlSelCur(grpprl, cb, fFalse);
			}
		SetUndoAfter(&ca);
		}

	SetAgain(bcmRRFormat);


	if (vfRecording && cb != 0)
		RecordGrpprl(grpprl, cb);

	return fTrue;
}


/*  %%Function: CbMergeInOnePrl  %%Owner: davidlu  */

int CbMergeInOnePrl(grpprl, cb, sprm, val)
CHAR	*grpprl;
int	cb;
int	sprm, val;
{
	int		cbNew;
	int             cbOld = cb;
	CHAR		grpprlNew[6]; /* long enough for use by RepeatRulerRibbon. */

/* This function assumes grpprl has cbMaxGrpprl bytes allocated for it. */
	cbNew = CchPrlFromSprmVal(grpprlNew, sprm, val);
	Assert(cbNew <= 6);

	MergeGrpprls(grpprl, &cb, grpprlNew, cbNew);

	if (cb > cbMaxGrpprl)
		{
		/* We overflowed. */
		return cbOld;
		}
	return cb;
}



/* View: Short Menus */
/*  %%Function: CmdShortMenus  %%Owner: krishnam  */

CMD CmdShortMenus(pcmb)
CMB * pcmb;
{
	extern struct PREF vpref;

	if (!vpref.fShortMenus)
		{
		vpref.fShortMenus = fTrue;
		return FSetMenu(selCur.doc == docNil ? iMenuShortMin : iMenuShortFull)
				? cmdOK : cmdError;
		}

	return cmdOK;
}


/* View: Long Menus */
/*  %%Function: CmdLongMenus  %%Owner: krishnam  */

CMD CmdLongMenus(pcmb)
CMB * pcmb;
{
	extern struct PREF vpref;

	if (vpref.fShortMenus)
		{
		vpref.fShortMenus = fFalse;
		return FSetMenu(selCur.doc == docNil ? iMenuLongMin : iMenuLongFull)
				? cmdOK : cmdError;
		}

	return cmdOK;
}


/* ************** items in View menu ********************* */
/*  %%Function: CmdDraftView  %%Owner: bryanl  */

CMD CmdDraftView(pcmb)
CMB * pcmb;
{
	int ww;
	struct WWD **hwwd;

	vpref.fDraftView = !vpref.fDraftView;

	/* if preview is up, blow it away... */
	if (vlm == lmPreview)
		FExecCmd(bcmPrintPreview);

	TrashAllWws();  /* state has changed, redraw windows */

/* following line forces a flush of the screen font cache. This is necessary because
	Draft View is implemented by always realizing the system/device default
	font in LoadFont.  The font handles in the cache will all be
	for the system font when in Draft View. Invalidating the font cache
	is a must so that the old mappings to font handles do not override
	the behavior of the current mode */

	FreeFontsPfti( vsci.pfti );

	for ( ww = wwDocMin ; ww < wwMac ; ww++ )
		if ((hwwd = mpwwhwwd[ww]) != hNil)
			{
/* draft view and page view are mutually exclusive, and draft view is global
	(due to its having been implemented in the font cache), so shut
	off page view in all windows */
			if (vpref.fDraftView && (*hwwd)->fPageView)
				{
			/* Force a redraw of the ruler */
				selCur.fUpdateRuler = fTrue;
				if (CmdPageViewOff(ww) != cmdOK)
					return cmdError;
				}
/* window flm changes wih draft view since draft view implies NOT display
	as printed */
			(*hwwd)->grpfvisi.flm = FlmDisplayFromFlags( 
					(*hwwd)->fDisplayAsPrint, fFalse, vpref.fDraftView);
			}

	return cmdOK;
}


/* C M D  A U T O S A V E */
/* Check a user response and take an appropriate action. */
/*  %%Function: CmdAutosave  %%Owner: peterj  */

CMD CmdAutosave()
{
	int  dMinute;

	Beep(); /* warn user */
	/* bring up dialog */
	dMinute = DMinuteUserPostpone();

	if (dMinute == dMinuteError)
		{
		return cmdError;
		}

	ResetASBase();

	if (dMinute != dMinuteCancel)
		{
		if (dMinute == 0)
			/* do save now */
			{
			FConfirmSaveAllDocs(acAutoSave);
			}
		else
			/* postpone by dMinute minutes */
			{
			asd.dmscLastPostpone = dMinute * dmscMinute;
			asd.mscPostpone = GetTickCount() + asd.dmscLastPostpone;
			}
		}

	return cmdOK;
}


/* D M I N U T E  U S E R  P O S T P O N E */
/* Prompt the user for autosave postponement. */
/*  %%Function: DMinuteUserPostpone  %%Owner: peterj  */

int DMinuteUserPostpone()
{
	int dMinute;
	CABAUTOSAVE * pcabautosave;
	CMB cmb;
	char dlt [sizeof (dltAutosave)];

	if ((cmb.hcab = HcabAlloc(cabiCABAUTOSAVE)) == hNil)
		return dMinuteError;

	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;
	pcabautosave = (CABAUTOSAVE *) *cmb.hcab;
	pcabautosave->fPostpone = fFalse;
	pcabautosave->dMinutePostpone = uNinch;

	BltDlt(dltAutosave, dlt);
	switch (TmcOurDoDlg(dlt, &cmb))
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		dMinute = dMinuteError;
		break;
#endif

	case tmcError:
		dMinute = dMinuteError;
		break;

	case tmcCancel:
		dMinute = dMinuteCancel;
		break;

	case tmcOK:
		dMinute = 0;
		break;
		}

	if (dMinute == 0)
		{
		pcabautosave = (CABAUTOSAVE *) *cmb.hcab;
		if (pcabautosave->fPostpone)
			{
			dMinute = min(120,max(0, pcabautosave->dMinutePostpone));
			}
		}

	FreeCab(cmb.hcab);

#ifdef DBGYXY
	CommSzNum(SzShared("dMinutePostpone: "), dMinute);
#endif

	return dMinute;
}



/* ****
*
*  Function: FDlgAutosave
*  Author:
*  Copyright: Microsoft 1987
*  Date: 9/5/87
*
*  Description: Dialog function for "Postpone Autosave?" dialog
*
*
** ***/
/*  %%Function: FDlgAutosave  %%Owner: peterj  */

BOOL FDlgAutosave(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	switch (dlm)
		{
	case dlmChange:
		if (tmc == tmcASDMinute && ValGetTmc(tmcASGroup) == 1)
			{
			GrayButtonOnBlank(tmcASDMinute, tmcOK);
			}
		break;

	case dlmSetItmFocus:
		if (tmc == tmcASDMinute && ValGetTmc(tmcASGroup) == 0)
			{
			SetTmcVal(tmcASGroup, 1);
			goto LDefaultVal;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcASYes:
			EnableTmc(tmcOK, fTrue);
			SetTmcVal(tmcASDMinute, uNinch);
			break;

		case tmcASPostpone:
			SetFocusTmc(tmcASDMinute);
LDefaultVal:
			if (ValGetTmc(tmcASDMinute) == uNinch)
				{
				SetTmcVal(tmcASDMinute,
						(int) ((long)asd.dmscLastPostpone/(long)dmscMinute));
				}
			break;
			}
		break;
		}

	return fTrue;
}


