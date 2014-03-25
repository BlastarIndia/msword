/* ****************************************************************************
**
**      COPYRIGHT (C) 1987, 1988 MICROSOFT
**
** ****************************************************************************
*
*  Module: dlghyph.c --- contains dialog functions for hyphenation dialog.
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** ************************************************************************* */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "debug.h"
#include "doslib.h"
#include "file.h"
#include "ch.h"
#include "wininfo.h"
#include "sedit.h"
#include "prm.h"
#include "screen.h"
#include "prompt.h"
#include "message.h"
#include "hyph.h"
#include "cmd.h"
#include "field.h"
#include "error.h"
#include "help.h"
#include "rareflag.h"
#include "core.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "hyphen.hs"
#include "hyphen.sdm"


extern BOOL		vfRecording;
extern char szClsStaticEdit[];
extern struct MERR      vmerr;
extern struct SEL       selCur;
extern int		wwCur;
extern struct WWD	**hwwdCur;
extern CHAR             szEmpty[];
extern struct CHP	vchpFetch;
extern int		vccpFetch;
extern CHAR HUGE	*vhpchFetch;
extern CP		vcpFetch;
extern struct UAB       vuab;
extern struct PREF      vpref;
extern struct SCI       vsci;
extern struct BPTB	vbptbExt;
extern struct HYPFD	vhypfd;
extern struct HYPB    **vhhypb;
extern HCURSOR		vhcArrow;
extern HCURSOR		vhcIBeam;
extern HWND             vhwndApp;
extern HWND             vhwndCBT;

FARPROC			lpprocBlinkCaretSEdit;
FARPROC			lpprocStaticEditWndProc;

HWND			hwndStaticEdit = NULL;


EXPORT PASCAL BlinkCaretSEdit(); /* DECLARATION ONLY */
#define flashIDHyph 0x1059
#define XpFromHypIch(dxp, ich)  (DxpLeftSE(dxp) + ((dxp) * (ich)))

csconst CHAR szOK[] = SzSharedKey("OK",OK);
csconst CHAR szYes[] = SzSharedKey("&Yes",Yes);
#define cchPBTextMax ((sizeof(szOK) > sizeof(szYes)) ? \
							sizeof(szOK) : sizeof(szYes))

#define cchUTextMax cchMaxWord*2

struct UTXD
	{
	int         ichSelFirst;
	int         ichSelLim;
	int         ichLimEffective;
	int         flashID;
	int         fSelOn;
	char        stText[cchUTextMax+1];
};

#define cbUTXD (sizeof(struct UTXD))
#define cwUTXD ((cbUTXD + 1) / 2)

extern CP CpBestHypd();
#ifdef WIN23
extern CHAR *PchLastFit3();
#endif /* WIN23 */

/***************************
*
*  Hyphenation functions:
*      CmdHyphenate(), DlgfHyphenate() and their supporting
*      functions.   Main hyphenation functions are located
*      in cshare\hyphenate.c
*
***************************/

/* ****
*
	Function: CmdHyphenate
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Menu level command function for "Proof Hyphenate" dialog
*
** ***/

/* %%Function:CmdHyphenate %%Owner:bryanl */
CMD CmdHyphenate(pcmb)
CMB * pcmb;
{
	CABHYPHEN      *pcab;
	CHAR           *szT;
	int             dxaT;
	struct DOP     *pdop;
	CMD             cmdReturn=cmdOK;
	TMC		tmc;
	BOOL		fOverflow;
	CHAR            szNum[ichMaxNum + 1];
	CPR		cpr;

	cmdReturn = cmdOK; /* we haven't failed yet */

	if (pcmb->fDefaults)
		{
		CHAR	*pch;

		/* Get the hot zone value from the current dop. */
		pdop = &PdodMother(selCur.doc)->dop;

		if (pdop->dxaHotZ <= 0 || vsci.xaRightMax <= pdop->dxaHotZ)
			{
			pdop->dxaHotZ = dxaInch / 4;
			}
		pch = szNum;
		CchExpZa(&pch, pdop->dxaHotZ, vpref.ut, ichMaxNum, fTrue);
		*pch = '\0';
		if (!FSetCabSz(pcmb->hcab, szNum, Iag(CABHYPHEN, hszHypHotZ)))
			return cmdNoMemory;

		pcab = (CABHYPHEN *) *pcmb->hcab;
		pcab->fHypCaps = vrf.fHyphCapitals;
		pcab->fHypConfirm = fTrue;
		vrf.fHotZValid = fTrue;
		}

	if (pcmb->fDialog || pcmb->fAction)
		{
		char dlt [sizeof (dltHyphenate)];

		if (vhhypb == NULL)
			{	/* do only if not already set up */
			cmdReturn = cmdError;
			if (!FLoadHyph()
					|| (cmdReturn = CmdHyphInit()) != cmdOK)
				goto LReturnUnload;
			}
#ifdef DEBUG
		else
			{
			Assert( vhypfd.fn != fnNil );
			Assert( vhypfd.hrgoutp != hNil );
			}
#endif

		BltDlt(dltHyphenate, dlt);
		if ((tmc = TmcOurDoDlg(dlt, pcmb)) == tmcError)
			{
			cmdReturn = cmdNoMemory;
			goto LReturnCancel;
			}

		if (tmc == tmcCancel)
			{
			cmdReturn = cmdCancelled;
			goto LReturnCancel;
			}

		if (vfRecording)
			{
			FRecordCab(pcmb->hcab, IDDHyphenate, pcmb->tmc, 
					imiHyphenate);
			}

		/* Save the global hyphenation setting for the next time. */
		/* First, protect ourselves from a heap movement.         */
		pcab = (CABHYPHEN *) *pcmb->hcab;
		vrf.fHyphCapitals = pcab->fHypCaps;

		GetCabSz(pcmb->hcab, szNum, ichMaxNum,
				Iag(CABHYPHEN, hszHypHotZ));
		if (FZaFromSs(&dxaT, szNum, CchSz(szNum) - 1,
				vpref.ut, &fOverflow) &&
				dxaT > 0)
			{
			PdodMother(selCur.doc)->dop.dxaHotZ = dxaT;
			}
		else  if (vrf.fHotZValid)
			{
			/* Haven't warned the user yet about this invalid
				hot zone measurement. */
			ErrorEid(eidInvalidHotZ, " CmdHyphenate");
			cmdReturn = cmdError;
			goto LReturnCancel;
			}

		if (tmc == tmcHypChange)
			{
			HyphChangeSelection(dxaT, vrf.fHyphCapitals, &cpr);
			/* Because of unmatching StartLongOp() in
				HyphChangeSelection(). */
			EndLongOp(fFalse /* fAll */);
			}
		cmdReturn = cmdOK;
LReturnCancel:  
		HyphCancel();
LReturnUnload:	
		UnloadHyph();
		}

	return cmdReturn;
}


/******
*
*  Function: FDlgHyphenate
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Dialog box function for "Proof Hyphenate" dialog
*
******/

/* %%Function:FDlgHyphenate %%Owner:bryanl */
BOOL FDlgHyphenate(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{
	int          dxaT;
	struct HYPD *phypd;
	CHAR         szT[cchPBTextMax];
	struct UTXD  utxd;
	HWND hwnd;
	struct RC rcDlg, rcApp;
	int dyp;

	
	switch (dlm)
		{
	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (wNew /* cIdle */ == 3)
			FAbortNewestCmg (cmgUtilEdit, fTrue, fTrue);
		else  if (wNew /* cIdle */ == 5)
			FAbortNewestCmg (cmgPromptUtilAC, fTrue, fTrue);
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */

	case dlmInit:
		if (FIsDlgDying())
			return fTrue;   /* just get out. Dlg will come down  bz */
	/* position the dialog out of the way */
		GetClientRect(vhwndApp, (LPRECT) &rcApp);
		hwnd = HwndFromDlg(HdlgGetCur());
		GetWindowRect(hwnd, (LPRECT) &rcDlg);
		dyp = rcDlg.ypBottom - rcDlg.ypTop;
		ScreenToClient(vhwndApp, (LPPOINT) &rcDlg.ptTopLeft);
		MoveDlg(rcDlg.xpLeft, rcApp.ypBottom - dyp - vsci.dypBdrCapt);
		Assert( vhhypb != hNil );
		SetHyphStaticEdit(NULL);

		/* if selection exactly one word, do hyphenation and display it */
		if ((*vhhypb)->fHyphWord)
			{
			FreezeHp();
			FetchToHypd(selCur.doc, selCur.cpFirst, selCur.cpLim,
					&(*vhhypb)->hypd);
			HyphenateWord(&(*vhhypb)->hypd, vrf.fHyphCapitals);
			(*vhhypb)->hypd.ichLimEffective = -1;
			if ((*vhhypb)->hypd.iichMac > 0)
				{
				(*vhhypb)->hypd.iichBest = (*vhhypb)->hypd.iichMac-1;
				if (FHypdWidow(&(*vhhypb)->hypd))
					goto LNoBest;
				}
			else
				{
LNoBest:		
				(*vhhypb)->hypd.iichBest = -1;
				}
			MeltHp();
			EnableTmc(tmcHypNoChange, fFalse);
			EnableTmc(tmcHypConfirm, fFalse);
			vrf.fHyphHaveWord = fTrue;
			goto LDispWord;
			}

	/* We want to hyphenate the entire document/selection. */

		/* Set up push buttons to initial state. */
		EnableTmc(tmcHypNoChange, fFalse);
		CchCopyLpszCchMax(szOK, (CHAR FAR *) szT, cchPBTextMax);
		SetTmcText(tmcHypChange, szT);
		EnableTmc(tmcHypAt, fFalse);
		vrf.fHyphHaveWord = fFalse;
		break;

	case dlmClick:
		switch (tmc)
			{
			CP 	cp;
			int	cHyphen;
			int	ichLast;
			CHAR	*pchCur;
			HCAB hcab;

		case tmcHypChange:

			if (!vrf.fHyphHaveWord)
				goto LNextWord;	/* don't have a word yet, go scan for one */

			DelHyphHhypb(vhhypb);
			SendMessage(hwndStaticEdit, WM_COMMAND,	SEM_GETDSCRP, (LPSTR) &utxd);
			Assert( utxd.ichSelFirst > 0 );
			for (pchCur = &(utxd.stText[1]), cHyphen = ichLast = 0;
					ichLast < utxd.ichSelFirst;  ichLast++, pchCur++)
				{
				if (*pchCur == '-')
					cHyphen++;
				}
			phypd = &((*vhhypb)->hypd);
#ifdef BRYANL
			CommSzNum( SzShared( "Insert Hyph at cp = " ), 
					(int) (phypd->rgdcp[utxd.ichSelFirst - cHyphen] + phypd->cpFirst) );
#endif
			HyphInsertCp( phypd->rgdcp[utxd.ichSelFirst - cHyphen]
					+ phypd->cpFirst);
			Debug(phypd = 0); /* can't assume no HM */

			if (vmerr.fDiskFail || vmerr.fMemFail)
			/* if failure, dialog will be taken down without EndDlg */
				return (fTrue);

LMaybeNextWord:	
			if ((*vhhypb)->fHyphWord)
				goto LEndTmc;

LNextWord:	
			if (!FGetHotZoneWidth(tmcHypHotZ, &dxaT))
				{
				SetTmcTxs(tmcHypHotZ, TxsAll());
				break;
				}
			if (!ValGetTmc(tmcHypConfirm))
				{
LEndTmc:
				if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
					return fFalse;
				if (hcab != hcabNull)
					{
					Assert(PcmbDlgCur()->hcab == hcab);
					EndDlg(tmc);  /* ok to call EndDlg here */

					}
			/* if hcabNull, dialog will be taken down without EndDlg */
				return (fTrue);
				break;
				}
			if (!vrf.fHyphHaveWord)
				{
				vrf.fHyphHaveWord = fTrue;
				EnableTmc(tmcHypNoChange, fTrue);
				CchCopyLpszCchMax(szYes, (CHAR FAR *) szT, cchPBTextMax);
				SetTmcText(tmcHypChange, szT);
				}
			vrf.fHyphCapitals = ValGetTmc(tmcHypCaps);
LRetry:
			if (!FHyphFind(dxaT, fFalse, vrf.fHyphCapitals, NULL))
				{
				if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
					return fFalse;
				if (hcab != hcabNull)
					{
					Assert(PcmbDlgCur()->hcab == hcab);
					EndDlg(tmcHypNoChange);   /* ok to call EndDlg here */

					}
			/* if hcabNull, dialog will be taken down without EndDlg */
				return (fTrue);
				}

			cp = CpBestHypd(&(*vhhypb)->hypd);
/* calculate effective hyphenation limit */
			phypd = &(*vhhypb)->hypd;
			if (phypd->ichLimEffective <= 0)
				{
				IchLimEffectiveHypd();
				}
			Debug(phypd = 0); /* can't assume no HM */
LDispWord:	
			ProposeHyph(vhhypb);
			EnableTmc(tmcHypAt, fTrue);
			SetHyphStaticEdit(vhhypb);

			if ((*vhhypb)->hypd.iichBest < 0)
				{
/* hyphenation's opinion is that we should not hyphenate this word.  However,
	if we just selected a word and asked hyphenate to break it down, we want
	to offer the option anyway.  Hence this code. */
				SendMessage(hwndStaticEdit, WM_COMMAND,
						SEM_GETDSCRP, (LPSTR) &utxd);
				SetDefaultTmc( (*vhhypb)->fHyphWord ?
						tmcCancel :
						tmcHypNoChange );
				if ((*vhhypb)->fHyphWord && utxd.ichSelFirst > 0)
					{
					EnableTmc( tmcHypChange, fTrue );
					if (FEnabledTmc(tmcHypText))
						SetFocusTmc(tmcHypText);
					}
				else
					EnableTmc( tmcHypChange, fFalse );
				break;
				}
/* have both suggested and recommended hyphenation points for this word */
			EnableTmc( tmcHypChange, fTrue );
			SetDefaultTmc( tmcHypChange );
			if (FEnabledTmc(tmcHypText))
				SetFocusTmc(tmcHypText);
			break;

		case tmcHypNoChange:
			Assert( vrf.fHyphHaveWord );
			DelHyphHhypb(vhhypb);
			goto LMaybeNextWord;
			}	/* end switch (tmc) */
		}	/* end switch (dlm) */
	return fTrue;
}



/******
*
*  Function: FGetHotZoneWidth
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Get a dxa value from the hot zone edit control.
*               Returns fTrue iff the hot zone value is correct.
*
******/

/* %%Function:FGetHotZoneWidth %%Owner:bryanl */
FGetHotZoneWidth(tmc, pdxa)
int     tmc;
int    *pdxa;
{
	int         cch;
	BOOL	fOverflow;
	CHAR        szNum[ichMaxNum];

	cch = CchGetTmcText(tmc, szNum, ichMaxNum - 1);
	if (cch != 0)
		{
		if (FZaFromSs(pdxa, szNum, cch, vpref.ut, &fOverflow)
				&& !fOverflow
				&& *pdxa > dxaHotZMin && *pdxa < vsci.xaRightMax)
			{
			vrf.fHotZValid = fTrue;
			return fTrue;
			}
		}
LInvalid:
	ErrorEid(eidInvalidHotZ, "FGetHotZoneWidth");
	vrf.fHotZValid = fFalse;
	return (fFalse);
}


/* %%Function:CmdHyphInit %%Owner:bryanl */
CMD CmdHyphInit()
{
	struct HYPB	hypb;
	BOOL            fUndo = fTrue;

	SetBytes(&hypb, 0, cbHYPB);

/* back up to beginning of paragraph */
	hypb.cpStart = CpFirstSty(selCur.ww, selCur.doc,
			selCur.cpFirst, styPara, fFalse);
	hypb.fHyphWord = fFalse;
	hypb.fDirty = (DiDirtyDoc(selCur.doc) != diNotDirty);

	if (hypb.fWholeDocScan = selCur.fIns)
		{
		hypb.cpLim = CpMacDoc(selCur.doc);
		PcaSetWholeDoc(&hypb.caUndo, selCur.doc);
		}
	else
		{
		CP	cpWordLim;

		/* if selection exactly one word, set flag */
		if (selCur.cpFirst != selCur.cpLim &&
				selCur.cpFirst == CpFirstSty(wwCur, selCur.doc,
				selCur.cpLim - 1, styWord,
				fFalse) &&
				(selCur.cpLim == (cpWordLim = CpLimSty(wwCur,
				selCur.doc, selCur.cpFirst,
				styWord, fFalse)) ||
				selCur.cpLim + 1 == cpWordLim))
			{
			hypb.fHyphWord = fTrue;
			hypb.cpStart = selCur.cpFirst;
			hypb.cpLim = selCur.cpLim;
			}
		else
			{
			/* cpStart is already set. */
			Assert(hypb.cpStart ==
					CpFirstSty(selCur.ww, selCur.doc,
					selCur.cpFirst, styPara,
					fFalse));
			hypb.cpLim = CpLimSty(selCur.ww, selCur.doc,
					selCur.cpLim - 1, styPara,
					fFalse);
			}
		PcaSet(&hypb.caUndo, selCur.doc, hypb.cpStart, hypb.cpLim);
		}
	if ((PdodDoc(selCur.doc)->fFtn || PdodDoc(selCur.doc)->fAtn) &&
			!FCheckLargeEditFtn(&hypb.caUndo, &fUndo))
		return cmdCancelled;
	if (fUndo)
		{
		if (!FSetUndoB1(imiHyphenate, uccPaste, &hypb.caUndo))
			{
			return cmdCancelled;
			}
		}
	else
		SetUndoNil();

	hypb.cpLine = hypb.cpStart = CpFirstSty(selCur.ww, selCur.doc, selCur.cpFirst, styPara, fFalse);

	Assert( vhhypb == hNil );
	if ((vhhypb = HAllocateCw(CwFromCch(cbHYPB))) == hNil)
		return(cmdNoMemory);

		/* We only hyphenate in the show result mode, so that
			we won't end up hyphenating field keywords, etc. */
		{
		GRPFVISI	grpfvisi;
		struct CA	ca;

		grpfvisi = PwwdWw(wwCur)->grpfvisi;
		hypb.grpfvisiSave = grpfvisi.w;
		grpfvisi.grpfShowResults = ~0;
		grpfvisi.fForceField = fTrue;
		PwwdWw(wwCur)->grpfvisi = grpfvisi;
		CheckInvalCpFirst(PcaSetWholeDoc(&ca, selCur.doc));
		InvalCp(&ca);
		}

/* if the selection is an insertion point (meaning we will be searching
the whole doc) or the selection covers more than 500 chars then
preload the external cache with the hyphenation pattern file. */
	if (selCur.fIns || selCur.cpFirst + 500 < selCur.cpLim)
		{
		PN cpnRead, pn;
		struct FCB *pfcb = PfcbFn(vhypfd.fn);
		StartLongOp();

		/* external cache better exist */
		Assert(vbptbExt.ibpMac);

		cpnRead = min((PN) ((pfcb->cbMac + cbSector - 1) >> shftSector), 5);
		for (pn = 0; pn < cpnRead; pn++)
			IbpCacheFilePage(vhypfd.fn, pn);
		EndLongOp(fTrue);
		}

/* initialize hypd and hypb */
	hypb.hypd.ichMac = 0;
	hypb.hypd.ichLimEffective = -1;
	hypb.hypd.ichSel = -1;

	bltb(&hypb, *vhhypb, cbHYPB);

	return(cmdOK);
}


/* %%Function:HyphCancel %%Owner:bryanl */
HyphCancel()
{
	struct CA	ca;
	struct HYPB hypb;

	bltb( *vhhypb, &hypb, cbHYPB);

	DelHyphHhypb(vhhypb);

	hypb.caUndo.cpLim += hypb.cHyph;
	SetUndoAfter(&hypb.caUndo);

	Select(&selCur, hypb.caUndo.cpFirst, hypb.caUndo.cpLim);

	PwwdWw(wwCur)->grpfvisi.w = hypb.grpfvisiSave;
	PcaSetWholeDoc(&ca, selCur.doc);
	CheckInvalCpFirst(&ca);
	InvalCp(&ca);

	if (hypb.cHyph == 0)
		{
		PdodDoc(DocMother(selCur.doc))->fDirty = hypb.fDirty;  /* restore the fDirty flag */
		}
	FreePh(&vhhypb);
}


/* U n l o a d   H y p h */
/* free hyphenation data file fn and all heap variables used to store
	stuff read from it */
/* %%Function:UnloadHyph %%Owner:bryanl */

UnloadHyph()
{
	if (vhypfd.fn != fnNil)
		{
		DeleteFn(vhypfd.fn, fFalse);
		vhypfd.fn = fnNil;
		}

	FreePh(&vhypfd.hrgoutp);
	FreePh(&vhypfd.hrgbctt);
}


/* D E L   H Y P H   H H Y P B */
/* Delete the suggested hyphenation */
/* %%Function:DelHyphHhypb %%Owner:bryanl */
DelHyphHhypb(hhypb)
struct HYPB     **hhypb;
{
	struct CA ca;

	/* Turn off the selection anyway even if there is no
		hyphen to clear. */
	TurnOffSel(&selCur);
	if ((*hhypb)->fClearHyph)
		{
		struct HYPD *phypd;

		phypd = &(*hhypb)->hypd;
		FDelete( PcaSetDcp( &ca, selCur.doc, CpBestHypd(phypd), (CP) 1 ));
#ifdef BRYANL
		CommSzNum( SzShared( "Delete Hyph at cp = " ), (int) CpBestHypd(phypd) );
#endif
		(*hhypb)->fClearHyph = fFalse;
		}
}



/* ------ Routines used in wordtech\hyph.c but sufficiently ------ */
/* -------------- different from the Mac counterpart. ------------ */

/* P r o p o s e  H y p h */
/* Scrolls the suggested hyphenation point into a view complete
	with a hyphen. */
/* %%Function:ProposeHyph %%Owner:bryanl */
ProposeHyph(hhypb)
struct HYPB **hhypb;
{
	/* Put in a hyphen to show where the line would break. */
	CP                  cpSuggested;
	struct HYPD		*phypd;
	struct CA		ca;

	TurnOffSel(&selCur);
	phypd = &(*hhypb)->hypd;
	if (phypd->iichBest >= 0)
		{
		CHAR    rgchHyphen[1];

		FreezeHp();
		cpSuggested = CpBestHypd(phypd);
#ifdef BRYANL
		CommSzNum( SzShared( "Propose Hyph at cp = " ), (int) cpSuggested );
#endif
		MeltHp();
		rgchHyphen[0] = chHyphen;
/* if this fails, so what, you don't get a hyphen */
		FInsertRgch(selCur.doc, cpSuggested, rgchHyphen, 1, NULL, NULL);
		InvalCp(PcaSetDcp( &ca,selCur.doc, cpSuggested, (CP) 1));
		AssureCpAboveDyp(cpSuggested,
				((*hwwdCur)->ywMac - (*hwwdCur)->ywMin) / 3, fTrue);
		Select(&selCur,cpSuggested, cpSuggested + 1);
		(*hhypb)->fClearHyph = fTrue;
		}
	else
		{
		(*hhypb)->fClearHyph = fFalse;
		}
	TurnOnSelCur();
}




/* %%Function:FLoadHyph %%Owner:bryanl */
BOOL FLoadHyph()
{
	/* read in hyphenation outputs and translation table */

	int cb;
	int cch;
	struct HYPFH	 hypfh;
	CHAR stFile[ichMaxFile + 2]; /* Used as stz with GetProgramDir */

	if (vhypfd.fn != fnNil)
		{
		return fTrue;
		}
	if (!FFindFileSpec(StShared("HYPH.DAT"), stFile, grpfpiUtil, nfoNormal)
			|| (vhypfd.fn = FnOpenSt(stFile, 0, ofcDoc, NULL)) == fnNil)
		{
		ErrorEid(eidHypNoDataFile, "FLoadHyph");
		return(fFalse);
		}

	/* read in base pointers for rest of hyphenation data */
	SetFnPos(vhypfd.fn, (FC) cbSectorPre35);
	ReadRgchFromFn(vhypfd.fn, (CHAR *) &hypfh, cchHYPFH);

	/* allocate memory for and read in hyphenation outputs */
	cb = hypfh.cbCttBase - hypfh.cbOutpBase;
	if ((vhypfd.hrgoutp = HAllocateCw(CwFromCch(cb))) == hNil)
		{
		return fFalse;
		}
	SetFnPos(vhypfd.fn, (FC) hypfh.cbOutpBase);
	ReadRgchFromFn(vhypfd.fn, (CHAR *) &(**vhypfd.hrgoutp)[0], cb);

	/* read in hyphenation translation table, if any */
	vhypfd.cbTrieBase = hypfh.cbTrieBase;
	if (hypfh.cbMac > hypfh.cbCttBase)
		{
		FC fcPrd = (FC) hypfh.cbCttBase + 2;
		/* 2 is WFromStm(VHYPFD.FN), count of translation runs (not used) */
		vhypfd.chHttFirst = *(char HUGE *) HpchFromFc(vhypfd.fn, fcPrd++);
		vhypfd.chHttLast = *(char HUGE *) HpchFromFc(vhypfd.fn, fcPrd++);
		cb = vhypfd.chHttLast - vhypfd.chHttFirst + 1;
		if ((vhypfd.hrgbctt = HAllocateCw(CwFromCch(cb))) == hNil)
			goto ErrRet;
		SetFnPos(vhypfd.fn, (FC) hypfh.cbCttBase + 4);
		ReadRgchFromFn(vhypfd.fn,
				(CHAR *) &(**vhypfd.hrgbctt)[0], cb);
		}
	else
		{ /* no translation */
		vhypfd.chHttLast = vhypfd.chHttFirst = -1;
		vhypfd.hrgbctt = 0;
		}

	return fTrue;

ErrRet:
	FreePh(&vhypfd.hrgoutp);
	return fFalse;
}


/* F E T C H  T O  H Y P D */
/* fetches the contents of a ca to hypd.
Vanished text and chNonReqHypen's will be skipped and their places indicated
in rgdcp.
No heap movement allowed.
*/
/* %%Function:FetchToHypd %%Owner:bryanl */
FetchToHypd(doc, cpFirst, cpLim, phypd)
int doc; 
CP cpFirst, cpLim; 
struct HYPD *phypd;
{
	int ichMac = 0, ch;
	CHAR *pchTo = &phypd->rgch[0], HUGE *hpch;
	CP *pdcpTo = &phypd->rgdcp[0];
	CP dcp = cp0, ccp;

	phypd->cpFirst = cpFirst;

	CachePara(doc, cpFirst);
	while (cpFirst < cpLim)
		{
		FetchCp(doc, cpFirst, fcmChars + fcmProps);
		doc = docNil;
		if (vchpFetch.fVanish)
			dcp += vccpFetch;
		else
			{
			for (ccp = (int)CpMin((CP)vccpFetch, cpLim - vcpFetch),
					hpch = vhpchFetch; ccp > 0;
					ccp--, hpch++, dcp++)
				if ((ch = *hpch) != chNonReqHyphen && ichMac < cchMaxWord - 1)
					{
					if (!(FUpper(ch) || FLower(ch)))
						goto LEnd;
					*pchTo++ = ch;
					*pdcpTo++ = dcp;
					ichMac++;
					}
			}
		cpFirst += vccpFetch;
		}
LEnd:
	Assert( ichMac <= cchMaxWord - 1 );
	phypd->ichMac = ichMac;
	phypd->iichMac = 0;
	phypd->iichBest = -1;
}


/* %%Function:SetHyphStaticEdit %%Owner:bryanl */
SetHyphStaticEdit(hhypb)
struct HYPB **hhypb;
{
	int         ichTo;
	struct UTXD utxd;

	ichTo = 0;
	
	SetBytes(&utxd, 0, sizeof(struct UTXD)); 

	if (hhypb != NULL)
		{
		int ichFrom, iich;
		struct HYPD *phypd = &(*hhypb)->hypd;
		char *rgch;
		BOOL fAdjust;

		rgch = &(utxd.stText[1]);
		utxd.ichLimEffective = phypd->ichLimEffective;
#ifdef DBGYXY
		CommSzNum(SzShared("ichLimEffective(before): "), utxd.ichLimEffective);
#endif
		for (ichFrom = iich = 0; ichFrom < phypd->ichMac; ichFrom++)
			{
			Assert( ichTo < cchUTextMax - 1 );
			rgch[ichTo++] = phypd->rgch[ichFrom];
			if (ichFrom == phypd->ichSel && iich > 0 &&
					ichFrom - 1 == phypd->rgich[iich - 1])
				{
				Assert(ichTo >= 2);
				utxd.ichSelFirst = ichTo - 2;
				utxd.ichSelLim = ichTo - 1;
				}
			if (iich < phypd->iichMac && ichFrom == phypd->rgich[iich])
				{
				if (ichTo <= utxd.ichLimEffective)
					{
					utxd.ichLimEffective++;
					}
				rgch[ichTo++] = '-';
				iich++;
				}
			}

#ifdef DBGYXY
		CommSzNum(SzShared("ichLimEffective(after): "), utxd.ichLimEffective);
#endif
		
		}

	if (hhypb == hNil || (*hhypb)->fHyphWord)
		{
		utxd.ichLimEffective = -1;
		}
	utxd.fSelOn = fFalse;
	utxd.flashID = NULL;
	utxd.stText[0] = ichTo;
	Assert(hwndStaticEdit != NULL);
#ifdef DBGYXY
	CommSzSt(SzShared("utxd.stText: "), utxd.stText);
#endif
	SendMessage(hwndStaticEdit, WM_COMMAND, SEM_SETDSCRP, (LPSTR) &utxd);
}


/* Routines to make the hyphenation progress reporting mechanism
	sharable. */

/* %%Function:InitHyphCancelPoll %%Owner:bryanl */
BOOL InitHyphCancelPoll(pcpr)
CPR	*pcpr;
{
	struct HYPB	*phypb;

	if ((*vhhypb)->fHyphWord)
		return;
	Assert (pcpr != NULL);
	if (pcpr == NULL) /* just to avoid low memory trashing */
		return;
	pcpr->hppr = HpprStartProgressReport(mstHyphenate, NULL,
			nIncrPercent, fTrue);
	phypb = *vhhypb;
	ProgressReportPercent(pcpr->hppr, phypb->cpStart, phypb->cpLim,
			phypb->cpLine, &pcpr->cpRptNext);
}


/* %%Function:FHyphCancelPoll %%Owner:bryanl */
BOOL FHyphCancelPoll(pcpr, fBatch)
CPR	*pcpr;
BOOL	fBatch;
{
	if (fBatch && (*vhhypb)->hypd.cpFirst >= pcpr->cpRptNext)
		{
#ifdef DBGYXY
		CommSzLong(SzShared("cpStart: "), (*vhhypb)->cpStart);
		CommSzLong(SzShared("cpLim: "), (*vhhypb)->cpLim);
		CommSzLong(SzShared("cpFirst: "), (*vhhypb)->hypd.cpFirst);
#endif
		if (!(*vhhypb)->fHyphWord)
			{
			ProgressReportPercent(pcpr->hppr, (*vhhypb)->cpStart,
					(*vhhypb)->cpLim, (*vhhypb)->hypd.cpFirst,
					&(pcpr->cpRptNext));
			if (FQueryAbortCheck())
				{
				TurnOffSel(&selCur);
				Select(&selCur, (*vhhypb)->hypd.cpFirst,
						(*vhhypb)->hypd.cpFirst);
				NormCp(wwCur, selCur.doc, (*vhhypb)->hypd.cpFirst, 3,
						((*hwwdCur)->ywMac - (*hwwdCur)->ywMin) / 3,
						fTrue);
				TurnOnSelCur();
				return fTrue;
				}
			}
		}
	return fFalse;
}


/* To accomodate difference in the prompt scheme between WIN and MAC. */
/* %%Function:IAlertHyphAn %%Owner:bryanl */
int IAlertHyphAn(an, fResponse)
CHAR *an;
BOOL fResponse;
{
	int	wRet;

	Assert( vhhypb != hNil );
	if (an == anHyphContinue)
		{
		wRet = IdMessageBoxMstRgwMb(mstHypRepeat, NULL, MB_DEFYESQUESTION)
				== IDYES;
		}
	else  if (!(*vhhypb)->fHyphWord)
		{
		SetPromptMst(mstHyphComplete, pdcAdvise2);
		wRet = NULL;
		}
	return wRet;
}



/* ---------------- Hyphenation Dialog Static Edit Stuff ----------------- */

/* %%Function:WPictureStaticEdit %%Owner:bryanl */
EXPORT WORD WPictureStaticEdit(tmm, psdmp, hwndDlg, reserved, tmc, wParam)
TMM	tmm;
SDMP	*psdmp;
HWND	hwndDlg;
WORD	reserved;
TMC	tmc;
WORD	wParam;
{
	HANDLE		hInst;
	HDC		hdc;
	struct UTXD	**hutxd;
	HDLG		hdlgSave;
	TEXTMETRIC	tm;
	CHAR	sz[CchMaxWndText];

#ifdef DBGYXY
	CommSzNum(SzShared("WPictureStaticEdit tmm: "), tmm);
#endif
	switch (tmm)
		{
	case tmmCreate:
		Assert(hwndStaticEdit == NULL);
		hInst = (HANDLE) GetWindowWord(hwndDlg, GWW_HINSTANCE);
		hwndStaticEdit = (WORD)CreateWindow(
				(LPSTR)szClsStaticEdit,
				(LPSTR)NULL,
				WS_CHILDWINDOW | WS_VISIBLE |
				WS_BORDER | SES_HYPHENATE,
				psdmp->prec->x, psdmp->prec->y,
				psdmp->prec->dx, psdmp->prec->dy,
				hwndDlg, (HMENU) tmc, hInst, 0L);
		if (hwndStaticEdit == NULL)
			{
			return fFalse;
			}
		if (FNoHeap(GetWindowWord(hwndStaticEdit, IDSEDITDSCRP)))
			{
			DestroyWindow( hwndStaticEdit );
			return fFalse;
			}

		SetWindowLong(hwndStaticEdit, GWL_WNDPROC,
				lpprocStaticEditWndProc);
		break;
	case tmmPaint:
		Assert(hwndStaticEdit != NULL);
		SendMessage(hwndStaticEdit, WM_PAINT, 0, 0L);
		break;
	case tmmSetFocus:
		SetFocus(hwndStaticEdit);
		SetFocusTmc(tmc);  /* so SDM will know it has the focus */
		break;
	case tmmClear:
	case tmmCopy:
	case tmmCut:
	case tmmPaste:
		/* We ignore these. */
		/* Tell SDM we didn't deal with these */
		return fFalse;
	case tmmInput:
		switch (psdmp->lpmsg->message)
			{
		default:
		case WM_KEYDOWN:
			/* Return fTrue so that WndProc gets this message. */
			/* Hack!                                           */
		case WM_KEYUP:
			return fTrue;
		case WM_ENABLE:
			EnableWindow(hwndStaticEdit, psdmp->lpmsg->wParam);
			return fTrue;
			}
		}
	return fTrue;
}


/* %%Function:StaticEditWndProc %%Owner:bryanl */
NATIVE LONG FAR PASCAL StaticEditWndProc(hwnd, msg, wParam, lParam)
HWND		hwnd;
unsigned	msg;
WORD		wParam;
LONG		lParam;
{
	struct UTXD	**hutxd;
	int		ichNew;
	int		cch;
	HDC		hdc;
	STL		stl;
	HDLG		hdlgSave;
	HWND		hwndT;
#ifdef WIN23
	int		dxpCh;
	int		dxpText;
#endif

	stl = GetWindowLong(hwnd, GWL_STYLE);
	hutxd = GetWindowWord(hwnd, IDSEDITDSCRP);


#ifdef DEBUG
	if (msg != WM_NCCREATE && msg != WM_CREATE && msg != WM_DESTROY &&
		msg != WM_NCDESTROY && msg != WM_NCCALCSIZE)
		Assert( !FNoHeap(hutxd) );
#endif

#ifdef DBGYXY
	ShowMsg("se" , hwnd, msg, wParam, lParam);
#endif  /* DBGSEDIT */

	switch (msg)
		{
	case WM_CREATE:
			{
			struct UTXD *putxd;
			TEXTMETRIC    tm;

			if (vhwndCBT)
				/* this must be the first thing we do under WM_CREATE */
				SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);

			SetWindowWord(hwnd, IDSEDITDSCRP, NULL);
			if ((hdc = GetDC(hwnd)) == NULL)
				{
				return 0L;	/* return val ignored but null hutxd cues caller */
				}

			GetTextMetrics(hdc, (LPTEXTMETRIC) &tm);
			SetWindowWord(hwnd, IDSEDITDXPCH, tm.tmAveCharWidth);
#ifdef BIGSYSFONT
			SetWindowWord(hwnd, IDSEDITDYPEXTLEAD, tm.tmExternalLeading);
#else
			SetWindowWord(hwnd, IDSEDITDYPEXTLEAD,
					tm.tmInternalLeading / 2);
#endif
			SetWindowWord(hwnd, IDSEDITDYPHEIGHT, tm.tmHeight);
			ReleaseDC(hwnd, hdc);

				/* not in native due to native bug! */
				{{  /* !NATIVE - StaticEditWndProc */
				hutxd = HAllocateCw(cwUTXD);
				}}

			SetWindowWord(hwnd, IDSEDITDSCRP, hutxd);
			if (FNoHeap(hutxd))
				{
				return 0L; 	/* return value ignored, but creator catches it */
				}

			putxd = *hutxd;
			putxd->flashID = NULL;
			putxd->ichSelFirst = putxd->ichSelLim =
					putxd->ichLimEffective = (putxd->stText)[0] = 0;
			goto LDefSEdit;
			}
	case WM_DESTROY:
		hwndStaticEdit = NULL;
		KillTimerSEdit(hwnd, hutxd);
		FreeH(hutxd);
		break;
	case WM_PAINT:
			{
			PAINTSTRUCT   ps;
			LONG          lReturn;

			BeginPaint(hwnd, (LPPAINTSTRUCT) &ps);
			UpdateSEditHyph(hwnd, ps.hdc, hutxd);
			EndPaint(hwnd, (LPPAINTSTRUCT) &ps);
			break;
			}
	case WM_SETFOCUS:
		if ((*hutxd)->stText[0] == 0 && wParam != NULL)
			{
			/* This should never happen under new SDM. */
			Assert(fFalse);
			/* If we don't have anything in it, set
				the focus back. */
			SetFocus(NULL);
			return (0L);
			}
LRestartT:
		RestartTimerSEdit(hwnd, hutxd);
		break;
	case WM_KILLFOCUS:
LKillT:
		KillTimerSEdit(hwnd, hutxd);
		break;
	case WM_COMMAND: /* To ask about hyphenation point, and other stuff */
		switch (wParam)
			{
		case SEM_KILLTIMER:
			goto LKillT;
		case SEM_RESTARTTIMER:
			goto LRestartT;
		case SEM_SETDSCRP:
			/* Save the length of previously displayed text. */
			cch = (*hutxd)->stText[0];
			KillTimerSEdit(hwnd, hutxd);

#ifdef WIN23
			if ((hdc = GetDC(hwnd)) != NULL && cch != 0)
				{
				dxpCh = GetWindowWord(hwnd, IDSEDITDXPCH);
				if (vsci.fWin3)
					dxpText = LOWORD(GetTextExtent(hdc, (LPSTR)&(*hutxd)->stText[1], cch));
				else
					dxpText = cch * dxpCh;
				}

			bltbx((LPSTR) lParam, (LPSTR) *hutxd, cbUTXD);
			if ((*hutxd)->stText[0] != 0)
				{
				SetWindowLong(hwnd, GWL_STYLE,
						(WS_TABSTOP | GetWindowLong(hwnd, GWL_STYLE)));
				}
			else
				{
				SetWindowLong(hwnd, GWL_STYLE,
						((~WS_TABSTOP) & GetWindowLong(hwnd, GWL_STYLE)));
				}
			/* Force the repaint */
			if (hdc != NULL)
				{
				if (cch != 0)
					{
					FSetDcAttribs(hdc, dccSEdit);
					PatBlt(hdc,
							XpFromHypIch(dxpCh, 0),
							GetWindowWord(hwnd,
							IDSEDITDYPEXTLEAD),
							dxpText,
							GetWindowWord(hwnd,
							IDSEDITDYPHEIGHT),
							vsci.ropErase);
					}

				UpdateSEditHyph(hwnd, hdc, hutxd);
				if (!FIsDlgDying())
					{
					if ((hwndT = GetParent(hwnd)) != NULL)
						{
						hdlgSave = HdlgFromHwnd(hwndT);
					    if (hdlgSave != hdlgNull)
							{
							hdlgSave = HdlgSetCurDlg(hdlgSave);
							EnableTmc((TMC) GetWindowWord(hwnd, GWW_ID),
								(*hutxd)->stText[0]);
							}
						HdlgSetCurDlg(hdlgSave);
						}
					}
				ReleaseDC(hwnd, hdc);
				}

#else
			bltbx((LPSTR) lParam, (LPSTR) *hutxd, cbUTXD);
			if ((*hutxd)->stText[0] != 0)
				{
				SetWindowLong(hwnd, GWL_STYLE,
						(WS_TABSTOP | GetWindowLong(hwnd, GWL_STYLE)));
				}
			else
				{
				SetWindowLong(hwnd, GWL_STYLE,
						((~WS_TABSTOP) & GetWindowLong(hwnd, GWL_STYLE)));
				}
			/* Force the repaint */
			if ((hdc = GetDC(hwnd)) != NULL)
				{
				if (cch != 0)
					{
					int dxpCh;

					dxpCh = GetWindowWord(hwnd,
							IDSEDITDXPCH);
					FSetDcAttribs(hdc, dccSEdit);
					PatBlt(hdc,
							XpFromHypIch(dxpCh, 0),
							GetWindowWord(hwnd,
							IDSEDITDYPEXTLEAD),
							cch * dxpCh,
							GetWindowWord(hwnd,
							IDSEDITDYPHEIGHT),
							vsci.ropErase);
					}

				UpdateSEditHyph(hwnd, hdc, hutxd);
				if (!FIsDlgDying())
					{
					if ((hwndT = GetParent(hwnd)) != NULL)
						{
						hdlgSave = HdlgFromHwnd(hwndT);
					    if (hdlgSave != hdlgNull)
							{
							hdlgSave = HdlgSetCurDlg(hdlgSave);
							EnableTmc((TMC) GetWindowWord(hwnd, GWW_ID),
								(*hutxd)->stText[0]);
							}
						HdlgSetCurDlg(hdlgSave);
						}
					}
				ReleaseDC(hwnd, hdc);
				}
#endif /* ~WIN23 */
			break;
		case SEM_GETDSCRP:
			bltbx((LPSTR) *hutxd, (LPSTR) lParam, cbUTXD);
			break;
		default:
			goto LDefSEdit;
			}

	case WM_GETDLGCODE:
		return ((LONG) DLGC_WANTARROWS);

	case WM_SETCURSOR:
		OurSetCursor(vhcIBeam);
		return fTrue;

	case WM_KEYDOWN:
			{
			int     dich;

			if (wParam == VK_RIGHT)
				{
				dich = 1;
				}
			else  if (wParam == VK_LEFT)
				{
				dich = -1;
				}
			else
				{
				return (LONG) fFalse;
				}
			cch = (*hutxd)->stText[0];
			if (cch != 0) /* If empty string, nothing to do. */
				{
				if (dich < 0)
					{
					ichNew = (*hutxd)->ichSelFirst + dich;
					}
				else
					{
					ichNew = (*hutxd)->ichSelLim + dich;
					}
				MoveHyphPointIch(ichNew, cch, hutxd, hwnd);
				EnableTmc(tmcHypChange,
						(*hutxd)->ichSelFirst > 0);
				return (LONG) fTrue;
				}
			return (LONG) fFalse;
			}

	case WM_MOUSEMOVE:
		if ((wParam & MK_LBUTTON) == NULL)
			{
			break;
			}
		goto LUpdateCursor;

	case WM_LBUTTONDOWN:
		  /* hdlgFromHwnd not stable if dialog coming down */
		if (!FIsDlgDying())
			{
			if ((hwndT = GetParent(hwnd)) != NULL)
				{
				hdlgSave = HdlgFromHwnd(hwndT);
			    if (hdlgSave != hdlgNull)
					{
					hdlgSave = HdlgSetCurDlg(hdlgSave);
					SetFocusTmc((TMC) GetWindowWord(hwnd, GWW_ID));
					}
				HdlgSetCurDlg(hdlgSave);
				}
			}

		SetCapture(hwnd);
		KillTimerSEdit(hwnd, hutxd);
LUpdateCursor:
		/* LOWORD(lParam) == xpMouse */
		MouseMoveSEdit(hwnd, LOWORD(lParam), hutxd);
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		MouseMoveSEdit(hwnd, LOWORD(lParam), hutxd);
		RestartTimerSEdit(hwnd, hutxd);
		if ((*hutxd)->stText[0] > 0)
			{
			EnableTmc(tmcHypChange,
					(*hutxd)->ichSelFirst > 0);
			}
		break;

	case WM_ENABLE:
		InvalidateRect(hwnd, (LPRECT) NULL, TRUE);
		break;

	default:
LDefSEdit:
		return ((LONG) DefWindowProc(hwnd, msg, wParam, lParam));
		}
	return 0L;
} /* StaticEditWndProc */



/******
*
*  Function: BlinkCaretSEdit
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Timer call-back function to blink caret/selection
*               in the hyphenation static edit.
*
******/

/* %%Function:BlinkCaretSEdit %%Owner:bryanl */
EXPORT BOOL FAR PASCAL BlinkCaretSEdit(hwnd, msg, wParam, lParam)
HWND    hwnd;
int     msg;
int     wParam;
LONG    lParam;
{
	HDC     hdc;
	STL     stl = (STL) GetWindowLong(hwnd, GWL_STYLE);

#ifdef DBG
	ShowMsg("bc", hwnd, msg, wParam, lParam);
#endif
	if (FCHECKSTL(stl, SES_HYPHENATE))
		{
		struct UTXD **hutxd;

		hutxd = GetWindowWord(hwnd, IDSEDITDSCRP);
		return (FToggleCaretHyp(hwnd, hutxd));
		}
	else  /* SES_DDL */		
		{
		Assert(fFalse);
		}
}


/******
*
*  Function: TurnOffCaretHyp, TurnOnCaretHyp, FToggleCaretHyp
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Caret/Selection inversion functions.
*
******/

/* %%Function:TurnOffCaretHyp %%Owner:bryanl */
TurnOffCaretHyp(hwnd, hutxd)
HWND          hwnd;
struct UTXD **hutxd;
{
	if ((*hutxd)->fSelOn)
		{
		FToggleCaretHyp(hwnd, hutxd);
		}
}


/* %%Function:TurnOnCaretHyp %%Owner:bryanl */
TurnOnCaretHyp(hwnd, hutxd)
HWND          hwnd;
struct UTXD **hutxd;
{
	if (!(*hutxd)->fSelOn)
		{
		FToggleCaretHyp(hwnd, hutxd);
		}
}


/* %%Function:FToggleCaretHyp %%Owner:bryanl */
BOOL FToggleCaretHyp(hwnd, hutxd)
HWND          hwnd;
struct UTXD **hutxd;
{
	struct RC     rc;
	int           xp, dxp;
	HDC           hdc;
	int           dxpCh;
	int           dypExtLead;
	int           dypHeight;
#ifdef WIN23
	int xpLim;
#endif

	if ((*hutxd)->ichSelFirst <= 0)
		{
		/* Nothing to do. */
		return (fTrue);
		}

	dxpCh = GetWindowWord(hwnd, IDSEDITDXPCH);
	Assert(dxpCh != 0);
	dypExtLead = GetWindowWord(hwnd, IDSEDITDYPEXTLEAD);
	dypHeight = GetWindowWord(hwnd, IDSEDITDYPHEIGHT);

	if ((hdc = GetDC(hwnd)) != NULL)
		{
#ifdef WIN23
		if (vsci.fWin3)
			{
			xp = (dxpCh >> 1) + /* see XpFromHypIch */
				(LOWORD(GetTextExtent(hdc, (LPSTR)&(*hutxd)->stText[1], 
				(*hutxd)->ichSelFirst)));
			xpLim = (dxpCh >> 1) + 
				(LOWORD(GetTextExtent(hdc, (LPSTR)&(*hutxd)->stText[1], 
				(*hutxd)->ichSelLim))) + 1;
			}
		else
			{
			xp  = XpFromHypIch(dxpCh, (*hutxd)->ichSelFirst);
			xpLim = XpFromHypIch(dxpCh, (*hutxd)->ichSelLim) + 1;
			}
		SetRect(&rc, xp, dypExtLead, xpLim, dypExtLead+dypHeight);
#else 
		xp  = XpFromHypIch(dxpCh, (*hutxd)->ichSelFirst);
		dxp = XpFromHypIch(dxpCh, (*hutxd)->ichSelLim) -
				XpFromHypIch(dxpCh, (*hutxd)->ichSelFirst) + 1;
		SetRect(&rc, xp, dypExtLead, xp + dxp, dypExtLead + dypHeight);
#endif /* WIN23 */
		PatBltRc(hdc, &rc, DSTINVERT);
		ReleaseDC(hwnd, hdc);
		(*hutxd)->fSelOn = !(*hutxd)->fSelOn;

		return (fTrue);
		}
	return (fFalse);
}


/******
*
*  Function: UpdateSEditHyph
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Draws a text and dotted line in the hyphenation
*               static edit.
*
******/

/* %%Function:UpdateSEditHyph %%Owner:bryanl */
UpdateSEditHyph(hwnd, hdc, hutxd)
HWND          hwnd;
HDC           hdc;
struct UTXD **hutxd;
{
	int    dxpCh, xpLimEffective, dypExtLead, dypHeight;

	extern struct SCI   vsci;
	extern HBRUSH       vhbrGray;

	FSetDcAttribs(hdc, dccSEdit);

	/* Take care of the rectangle in this function. */
	dxpCh = GetWindowWord(hwnd, IDSEDITDXPCH);
	Assert(dxpCh != 0);
	dypExtLead = GetWindowWord(hwnd, IDSEDITDYPEXTLEAD);
	dypHeight = GetWindowWord(hwnd, IDSEDITDYPHEIGHT);

	TurnOffCaretHyp(hwnd, hutxd);

	DrawHyphenationText(hutxd, hdc, dxpCh, dypExtLead, dypHeight);

	if ((*hutxd)->ichLimEffective > 0)
		{
		/* Draw a dotted line to show where the break limit is. */
		SelectObject(hdc, vhbrGray);
#ifdef WIN23
		if (vsci.fWin3)
			{
			xpLimEffective = (dxpCh >> 1) +
				(LOWORD(GetTextExtent(hdc, (LPSTR)&(*hutxd)->stText[1], 
				(*hutxd)->ichLimEffective)));
			PatBlt(hdc, xpLimEffective,	dypExtLead, 1, dypHeight, PATINVERT);
			}
		else
#endif /* WIN23 */
		PatBlt(hdc, xpLimEffective = XpFromHypIch(dxpCh,
				(*hutxd)->ichLimEffective),
				dypExtLead, 1, dypHeight, PATINVERT);
		}

	TurnOnCaretHyp(hwnd, hutxd);

}


/******
*
*  Function: MoveHyphPointIch
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Moves the caret/selection in the hyphenation
*               static edit.
*
******/

/* %%Function:MoveHyphPointIch %%Owner:bryanl */
MoveHyphPointIch(ich, cch, hutxd, hwnd)
int         ich;
int         cch;
struct UTXD **hutxd;
HWND        hwnd;
{
	HDC         hdc;
	int         ichLim;
	CHAR        chT;

	/* First, put the new hyphenation point in the
		valid range. */
	if (ich <= 0)
		{
		ich = 1;
		}
	else  if (ich >= cch)
		{
		ich = cch - 1;
		}

	/* If the hyphen is at the new hyphenation point
		or if the preceeding character is a hyphen,
		then select the entire character. */
	ichLim = ich;
	if ((chT = (*hutxd)->stText[ich + 1]) == chHyphen ||
			chT == chNonReqHyphen)
		{
		ichLim++;
		}
	else  if ((chT = (*hutxd)->stText[ich]) == chHyphen ||
			chT == chNonReqHyphen)
		{
		ich--;
		}

	if (ich <= 0 || ichLim <= 0)
		{
		ich = ichLim = 1;
		}
	else  if (ichLim >= cch)
		{
		ichLim = cch;
		}

	/* Now, let's move the hyphenation point.   */
	/* If the hyphenation point hasn't changed, */
	/* then there's nothing to do.              */
	if ((*hutxd)->ichSelFirst != ich ||
			(*hutxd)->ichSelLim   != ichLim)
		{
		TurnOffCaretHyp(hwnd, hutxd);
		(*hutxd)->ichSelFirst = ich;
		(*hutxd)->ichSelLim   = ichLim;
		TurnOnCaretHyp(hwnd, hutxd);
		}
}


/******
*
*  Function: DrawHyphenationText
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Draws a text with a proper notation for already
*               existing non-required hyphen.
*
******/

/* %%Function:DrawHyphenationText %%Owner:bryanl */
DrawHyphenationText(hutxd, hdc, dxpCh, dypExtLead, dypHeight)
struct UTXD **hutxd;
HDC           hdc;
int           dxpCh;
int           dypExtLead;
int           dypHeight;
{
	CHAR          szT[cchUTextMax + 1];

	StToSz((*hutxd)->stText, szT);
	TextOut(hdc, XpFromHypIch(dxpCh, 0), dypExtLead,
			szT, (int) ((*hutxd)->stText[0]));

#ifdef OLD
	CHAR          *pchCur, *pchFirst;
	int           dxpFirst;
	for (pchCur = pchFirst = szT, dxpFirst = XpFromHypIch(dxpCh, 0);
			*pchCur != '\0';)
		{
		if (*pchCur == chNonReqHyphen)
			{
			int         cch;
			CHAR        szHyphen[2];

			/* First, TextOut the string before this optional hyphen. */
			*pchCur = '\0';
			TextOut(hdc, dxpFirst, dypExtLead, (LPSTR) pchFirst,
					cch = (int) pchCur - pchFirst);
			dxpFirst += dxpCh * cch;
			pchFirst = ++pchCur;

			/* Then, draw a hyphen in place of this non-required hyphen.  */
			szHyphen[0] = chHyphen;
			szHyphen[1] = '\0';
			TextOut(hdc, dxpFirst, dypExtLead, (LPSTR) szHyphen, 1);
			dxpFirst += dxpCh;
			}
		else
			{
			pchCur++;
			}
		}

	/* TextOut the remaining string, if there is any. */
	if (pchFirst != pchCur)
		{
		TextOut(hdc, dxpFirst, dypExtLead, (LPSTR) pchFirst,
				(int) pchCur - pchFirst);
		}
#endif
}


/******
*
*  Function: IchFromHypXp
*  Author:
*  Copyright: Microsoft 1986
*  Date: 12/12/86
*
*  Description: Return ich from a given mouse xp coordinate.
*
******/

/* %%Function:IchFromHypXp %%Owner:bryanl */
int IchFromHypXp(dxpCh, xp, cch)
int     dxpCh;
int     xp;
int     cch;
{
	int     ich;

	ich = xp /* - (dxpCh / 2) + (dxpCh / 2) */ / dxpCh;
	if (ich < 1)
		{
		ich = 1;
		}
	else  if (ich > cch)
		{
		ich = cch - 1;
		}
	return (ich);
}


/* %%Function:RestartTimerSEdit %%Owner:bryanl */
RestartTimerSEdit(hwnd, hutxd)
HWND		hwnd;
struct UTXD	**hutxd;
{
	if ((*hutxd)->flashID == NULL &&
			/* In case we came from SEM_RESTARTTIMER */
	(*hutxd)->stText[0] != 0)
		{
		SetTimer(hwnd, flashIDHyph,  GetCaretBlinkTime(),
				lpprocBlinkCaretSEdit);
		(*hutxd)->flashID = flashIDHyph;
		}
}


/* %%Function:KillTimerSEdit %%Owner:bryanl */
KillTimerSEdit(hwnd, hutxd)
HWND		hwnd;
struct UTXD	**hutxd;
{
	if (!FNoHeap(hutxd) && (*hutxd)->flashID != NULL)
		{
		/* Make sure the caret is on before we stop blinking. */
		TurnOnCaretHyp(hwnd, hutxd);
		KillTimer(hwnd, (*hutxd)->flashID);
		(*hutxd)->flashID = NULL;
		}
}



/* %%Function:MouseMoveSEdit %%Owner:bryanl */
MouseMoveSEdit(hwnd, xp, hutxd)
HWND		hwnd;
int		xp;
struct UTXD	**hutxd;
{
	int	ichNew, cch;
#ifdef WIN23
	HDC	hdc;
#endif

	cch = (*hutxd)->stText[0];
	if (cch != 0) /* If empty string, nohting to do. */
		{
#ifdef WIN23
		int dxpCh = GetWindowWord(hwnd, IDSEDITDXPCH);
		if (vsci.fWin3 && ((hdc = GetDC(hwnd)) != NULL))
			{
			ichNew = IchFromHypXp3(dxpCh, xp, hutxd, hdc);
			ReleaseDC(hwnd, hdc);
			}
		else
			ichNew =IchFromHypXp(dxpCh, xp, cch);
#else
		ichNew =IchFromHypXp(GetWindowWord(hwnd, IDSEDITDXPCH),
				xp, cch);
#endif /* WIN23 */
		MoveHyphPointIch(ichNew, cch, hutxd, hwnd);
		}
}


#ifdef WIN23
/* %%Function:IchFromHypXp3 %%Owner:chic */
int IchFromHypXp3(dxpAvgCh, xp, hutxd, hdc)
int dxpAvgCh; /* average character size */
int xp; /* space to fit into */
struct UTXD	**hutxd;
HDC hdc;
{
	int cch = (*hutxd)->stText[0];
	int dx = xp - (dxpAvgCh >> 1); /* adjust for little space to the left
									of static edit box */
	CHAR *pch;
	CHAR *pchMic;
	CHAR *pchMac;
	
	Assert(cch > 0);
	FreezeHp();
	pchMic = &(*hutxd)->stText[1];
	pchMac = pchMic + cch;
	pch = PchLastFit3(hdc, dxpAvgCh, pchMic, pchMac, &dx);
	MeltHp();
	return (pch - pchMic);
}
#endif /* WIN23 */
