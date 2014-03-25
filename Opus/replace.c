/* replace.c    Replace code (split off from search.c 8/26/88 rp) */

#define NOGDICAPMASKS
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "doc.h"
#include "sel.h"
#include "ch.h"
#include "fkp.h"
#include "file.h"
#include "winddefs.h"
#include "prompt.h"
#include "message.h"
#include "cmdtbl.h"

#include "keys.h"
#include "wininfo.h"
#include "inter.h"
#include "field.h"
#include "debug.h"
#include "error.h"
#include "prm.h"
#include "cmd.h"
#include "cmdlook.h"
#define FINDNEXT
#define SEARCHTMC
#include "search.h"
#include "tmc.h"
#include "core.h"
#include "format.h"
#include "screen.h"
#include "rerr.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "replace.hs"
#include "replace.sdm"
#include "confirmr.hs"
#include "confirmr.sdm"

#ifdef PROTOTYPE
#include "replace.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */
extern BOOL          vfRecording;
extern BOOL          fElActive;
extern struct SEL    selCur;
extern struct FKPD   vfkpdChp;
extern struct FKPDP  vfkpdPap;
extern struct CHP    vchpStc;
extern int           vfSeeSel;
extern struct MERR   vmerr;
extern struct CA     caAdjust;
extern struct CA     caPara;
extern int           wwCur;
extern BOOL          vfExtendSel;
extern int           vdocScratch;
extern struct WWD    **hwwdCur;
extern FNI           fni;
extern struct PAP    vpapFetch;
extern CP            vcpMatchFirst, vcpMatchLim;
extern CP            vcpSearchStart, vcpSearchLim;
extern CP            vcpLimWrap;
extern CP            vcpLimWrapSave;
extern BOOL          vfSearchWord, vfSearchCase;
extern BOOL          vfNoWrap, vfFound, vfFwd;
extern BOOL          vfReplace, vfSearchRepl;
extern struct FB     vfbSearch, vfbReplace;
extern struct CHP    vchpSRGray;
extern CHAR          vszSearch[], vszReplace[];
extern HWND          vhwndApp;
extern struct SCI    vsci;
extern CP            vcpFetch;
extern int           vccpFetch;
extern char HUGE     *vhpchFetch;
extern HWND	     vhwndStatLine;

/* G L O B A L S */
BOOL    vfConfirm;
struct RCB *prcb;
CHAR    *stRealReplace;
struct RRI *vprri;


NATIVE SetParaReplace(); /* DECLARATION ONLY */
NATIVE AssignFnFcForStReplace(); /* DECLARATION ONLY */
CP CpDoReplace();


/* C M D  R E P L A C E */
/* %%Function:CmdReplace %%Owner:rosiep */
CMD CmdReplace(pcmb)
CMB * pcmb;
{
	CABREPLACE *pcab;
	TMC tmc;
	CMD cmdT = cmdOK;        /* return code */
	struct RRI rri;

	vprri = &rri;

	Assert (wwCur != wwNil);
	Assert( !vfExtendSel );

	if (selCur.fBlock)
		TurnOffBlock(&selCur);

	InitReplaceFormatting();

	cmdT = cmdOK;
	tmc  = tmcOK;

	vfSearchRepl = fTrue;
	vfReplace = fTrue;
	vcpLimWrapSave = cpNil;

	if (FCmdFillCab())
		{
		/* initialize Command Argument Block */
		pcab =  (CABREPLACE *) *pcmb->hcab;
		pcab->sab = 0;
		pcab->fFormatted = fFalse;
		pcab->fWholeWord = vfSearchWord;
		pcab->fMatchUpLow = vfSearchCase;
		pcab->fConfirm = fTrue; /* don't use global; too dangerous */
		FSetCabSz(pcmb->hcab, vszSearch, Iag (CABREPLACE, hszSearch));
		FSetCabSz(pcmb->hcab, vszReplace, Iag (CABREPLACE, hszReplace));
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		BOOL fRecordingSav;
		CHAR	dlt[sizeof(dltReplace)];
		struct FB fbSearchT, fbReplaceT;

		BltDlt(dltReplace, dlt);

		fbSearchT = vfbSearch;
		fbReplaceT = vfbReplace;

		InhibitRecorder(&fRecordingSav, fTrue);
		pcmb->pv = &fRecordingSav;

		tmc = TmcOurDoDlg(dlt, pcmb);
		switch (tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			cmdT = cmdError;
			break;
#endif
		case tmcError:
			cmdT = cmdNoMemory;
			break;
		case tmcCancel:
			vfbSearch = fbSearchT;
			vfbReplace = fbReplaceT;
			cmdT = cmdCancelled;
		case tmcOK:
			break;
			}
		InhibitRecorder(&fRecordingSav, fFalse);
		}

	if (pcmb->fCheck)
		{
		pcab = *pcmb->hcab;
		if (**pcab->hszSearch == '\0' && !pcab->fFormatted)
			RtError(rerrIllegalFunctionCall);
		}

	if (pcmb->fAction && tmc == tmcOK)
		{
		Profile(vpfi == pfiReplace ? StartProf(30) : 0);
		EnablePreload(PdodDoc(selCur.doc)->fn);
		pcab = (CABREPLACE *) *pcmb->hcab;
		if (WReplaceCore(pcmb->hcab, pcab->hszSearch, pcab->hszReplace,
				pcab->fMatchUpLow, pcab->fWholeWord,
				fFalse /* fUp */, !fElActive /* fWrap */, !pcab->fConfirm,
				fTrue /* fInteract */ ) == tmcSearchNil)
			{
			cmdT = cmdError;
			}
		DisablePreload();
		Profile(vpfi == pfiReplace ? StopProf() : 0);
		}

LRet:
	vfSearchRepl = fFalse;
	return (cmdT);
}


/* W  R E P L A C E  C O R E */

/* %%Function:WReplaceCore %%Owner:rosiep */
int WReplaceCore(hcab, pszSearch, pszReplace, fCase, fWord,
fUp, fWrap, fAll, fInteract)
HCABREPLACE hcab;
char ** pszSearch;
char ** pszReplace;
BOOL fCase, fWord, fUp, fWrap, fAll, fInteract;
{
	struct RCB rcb;
	char stSearch[cchSearchMax];
	/* have to keep the original around; otherwise I'd use the same space for both */
	char stReplace[cchReplaceMax];
	char stReplaceSubst[cchReplaceMax]; /* after ^p and case substitutions made */
	struct BMIB bmib;  /* Boyer-Moore Info Block */
	struct RPP rpp;   /* RePlace Prls */
	CP cpMac = CpMacDoc(selCur.doc);
	BOOL fMatchSel;
	TMC tmc;
	struct WWD *pwwd;

	Assert (PselActive() == &selCur);

	stRealReplace = stReplaceSubst;

	/* set up communication block */
	prcb = &rcb;
	rcb.iLastCase = -1;
	rcb.iChange = 0;
	rcb.pbmib = &bmib;
	rcb.prpp = &rpp;
	rcb.stReplace = stReplace;

	vfSearchRepl = fTrue;
	vfReplace = fTrue;
	fni.is = isSearch;

	CchCopyLpszCchMax(*pszSearch, vszSearch, cchSearchMax);
	CchCopyLpszCchMax(*pszReplace, vszReplace, cchReplaceMax);

	if (fElActive)
		{
		vfbSearch.fChp = vfbSearch.fPap = 
				vfbReplace.fChp = vfbReplace.fPap = 0;
		if (!FValidSRText(vszSearch, &vfbSearch) ||
				!FValidSRText(vszReplace, &vfbReplace))
			{
			if (vmerr.fMemFail)
				ErrorNoMemory(eidNoMemOperation);
			else
				ErrorEid(eidInvalidStyle, "CmdReplace");
			return tmcSearchNil;
			}
		}
	/* else FValidSRText done in dialog proc */

	pwwd = *hwwdCur;

	AcquireCaAdjust();
	caAdjust = selCur.ca;

	if (!fElActive || (*hcab)->fFormatted)
		CheckForFormatting();

	if (fElActive && !vfbSearch.fInfo)
		{
		extern int vrerr;

		/* Can't just RtError() because of preload stuff... */
		vrerr = rerrIllegalFunctionCall;
		return tmcSearchNil;
		}

	Assert(vfbSearch.fInfo);

	/* if empty replace text and no replace formatting, it means
		really replace text with null string */
	if (!vfbReplace.fInfo)
		vfbReplace.fText = fTrue;

	BuildReplaceGrpprls(&rpp);

	vfSearchWord = fWord;
	vfSearchCase = fCase;
	vfFwd = !fUp;
	vfConfirm = !fAll;

	SzToSt(vszReplace, stReplace);
	SetSpecialMatch(&bmib);
	vfbReplace.fReplMatch = fFalse;
	vfbReplace.fReplScrap = fFalse;
	if (vfbReplace.fText)
		SetParaReplace(stReplace, &rcb.fParaReplace, rcb.rgsrd);
	if (vfbReplace.fReplMatch)  /* set by SetParaReplace if ^m in replace string */
		{
		if (DocCreateScratch(DocMother(selCur.doc)) == docNil)
			{
			ErrorEid(eidReplaceFail, "WReplaceCore");
			return tmcSearchNil;
			}
		}

/* FUTURE rosiep (rp): fMatchSel can go - we can use vfFound;
    fIgnoreMatch parameter can go - we can use vfReplace;
	FSetUpForNextSearch can be greatly simplified, using vfFound
		(don't need to call CpSearchSz to see if selection matches
		exactly) - this will need some major code review to make
		sure it doesn't break anything - all of the Search/Replace
		code needs review badly anyway.
*/
	fMatchSel = FSetUpForNextSearch(&bmib, fTrue /* fIgnoreMatch */);

/* selCur can't have changed yet here */
	vcpSearchLim = (selCur.fIns || fMatchSel) ?
			((!fWrap || selCur.cpFirst == cp0) ? cpMac : vcpSearchStart) :
	selCur.cpLim;

	vfNoWrap = !fWrap;
	vfFound = fFalse;
/* selCur can't have changed yet here */
	if ((!selCur.fIns && !fMatchSel) || selCur.cpFirst == cp0)
		vfNoWrap = fTrue;

	if (!vfConfirm)
		goto LChangeAll;

	/* TmcConfirmReplace may change vfConfirm */
	if ((tmc = TmcConfirmReplace()) == tmcError || tmc == tmcCancel)
		goto LRet;

	/* dialog was dismissed to do change all */
	else  if (!vfConfirm && tmc == tmcReplYes)

LChangeAll:
		if (!bmib.fNotPlain || !bmib.fChTableInPattern)
			ChangeAll();

	/* else we got through the whole document with confirm on */

	NormCp(wwCur, selCur.doc, selCur.cpFirst, ncpHoriz, 0, selCur.fInsEnd);

LRet:
	vcpSearchLim = cpNil;
	ReleaseCaAdjust();
	if (vfbReplace.fReplMatch)
		ReleaseDocScratch();

	/* report number of changes made */
	SetPromptWMst(rcb.iChange == 1 ? mstOneChange : mstMultiChanges, 
			rcb.iChange, pdcNotify);

	vfSearchRepl = fFalse;
	return rcb.iChange;
}


/* F  N E X T  M A T C H */
/* Find the next match.  Return true if we found one, false if not */

/* %%Function:FNextMatch %%Owner:rosiep */
FNextMatch(fNoReplace)
BOOL fNoReplace;  /* user said No to confirm last replace */
{
	CP dcpAdjust = 0;
	CP cpMac = CpMacDocEdit(selCur.doc);
	CP cpLimT;

	/* if no replace, start search at next character instead of the usual 
	   skipping to the end of replaced text; we do the normal skip if we
	   have just formatting */
	if (fNoReplace && vfbSearch.fText)
		vcpSearchStart = CpLimSty(selCur.ww, selCur.doc, vcpMatchFirst, styChar, fFalse);

LFindNext:
	if (!FFindNext(prcb->pbmib))
		{
		if (vfFound)
			Select(&selCur, CpMax(cp0, caAdjust.cpFirst), CpMax(cp0, caAdjust.cpLim));
		TurnOnSelCur();
		return fFalse;
		}

	/* FDeleteCheck will remove final EOP from the selection.  This is OK only
		when the final EOP is the entire selection */
	if (selCur.cpLim > cpMac && selCur.cpFirst <= cpMac)
		dcpAdjust = selCur.cpLim - cpMac;

	cpLimT = selCur.cpLim;   /* because FDeleteCheck can adjust it at EOD */
	if (!FDeleteCheck(fFalse /* fPaste */, rpkText, NULL))
		{
		dcpAdjust = 0;
		vcpSearchStart = cpLimT; /* for next search */
		goto LFindNext;
		}

	selCur.cpLim += dcpAdjust;
	vcpSearchStart = selCur.cpLim; /* for next search */

	TurnOnSelCur();
	return fTrue;
}



/* F  C H A N G E  O N E */
/* Replace one occurrence and find the next one; return false if we
	found and replaced the last occurrence (and thus should bring
	down the dialog).
*/

/* %%Function:FChangeOne %%Owner:rosiep */
BOOL FChangeOne()
{
	CP ccpReplace;
	BOOL f;

	/* replace; then continue */

	TurnOffSel(&selCur);
	if ((ccpReplace = CpDoReplace(vcpMatchFirst, vcpMatchLim, prcb, &f)) == cpNil)
		return fFalse;


	vcpSearchStart = vcpMatchFirst + ccpReplace;

	prcb->iChange++;
	vfSeeSel = fTrue;
	UpdateWw( wwCur, fFalse);
	PushInsSelCur();

	return FNextMatch(fFalse);
}


/* C H A N G E  A L L */
/* Change all occurrences, don't ask the user for confirmation */

/* %%Function:ChangeAll %%Owner:rosiep */
BOOL ChangeAll()
{
	CP cpSearchStart, cpSearchLim, ccpReplace, cpSearchStartSav;
	BOOL fWholeDoc = !vfNoWrap;
	struct CA caT;
	CP cpMac = CpMacDoc(selCur.doc);
	CP cpNextRpt = 0;
	struct PPR **hppr;
	BOOL fRet = fTrue;
	BCM bcm;
	struct SEL sel;
	BOOL fDidIt;

	vprri->cpLimWrap = cpNil;
	prcb->iLastCase = -1;

	vfConfirm = fFalse;  /* needed later; could have been true here */

	cpSearchLim = vfNoWrap ? vcpSearchLim : cpMac;
	cpSearchStart = selCur.cpFirst;

/* Make cpLowRpt and cpHighRpt bracket the starting cp in such a way
	that cpHighRpt - cpLowRpt equals the total number of cp's that will
	be scanned.  If we are going to wrap, cpHighRpt will be past the end
	of doc.  If we have already wrapped, cpLowRpt will be negative.
*/
	vprri->cpLowRpt = caAdjust.cpFirst;
	vprri->cpHighRpt = (fWholeDoc || vprri->cpLowRpt == vcpSearchLim) ?
			vprri->cpLowRpt + cpMac : vcpSearchLim;
	if (vprri->cpLowRpt > cpSearchStart)
		{
		vprri->cpLowRpt -= cpMac;  /* intentionally negative */
		vprri->cpHighRpt -= cpMac;
		}
	bcm = imiReplace;
	if (!FSetUndoB1(bcm, uccPaste, 
		/* Note: NOT PcaSetWholeDoc - that only goes up to CpMacDocEdit! */
	PcaSet( &caT, selCur.doc, cp0, CpMacDoc(selCur.doc))))
		return fFalse;
	StartLongOp();

	hppr = HpprStartProgressReport(mstReplacing, NULL, nIncrPercent, fTrue);

LRestartChangeAll:
	cpSearchStartSav = cpSearchStart;
	InvalCp1Sub(PcaSet(&caT, selCur.doc, cpSearchStart, cpSearchLim));
	InvalCaFierce();
	DisableInval();
	while ((vcpMatchFirst = CpSearchSz(prcb->pbmib, cpSearchStart,
			cpSearchLim, &cpNextRpt, hppr)) != cpNil)
		{
		if (vprri->cpLimWrap == cpNil)
			vprri->cpLimWrap = vcpLimWrap;

		if (vfSearchWord &&
				!FWordCp(vcpMatchFirst, vcpMatchLim - vcpMatchFirst))
			{
			cpSearchStart = CpLimSty(selCur.ww, selCur.doc, vcpMatchFirst,
						styChar, fFalse);
			continue;
			}


		if (vfbReplace.fText)
			{
			/* Set up a SEL for the fSimple case of FDelCkBlockPsel (we
			guarantee no block sel, no table cell marks are included,
			and no selection hilight is on the screen */

			sel.doc = selCur.doc;
			sel.cpFirst = vcpMatchFirst;
			sel.cpLim = vcpMatchLim;
			sel.sk = skSel;
			if (!FDelCkBlockPsel(&sel, rpkText, NULL, fTrue, fTrue /* fSimple */))
				{
				cpSearchStart = CpLimSty(selCur.ww, selCur.doc, vcpMatchFirst,
						styChar, fFalse);
				continue;
				}
			}

		if ((ccpReplace =
				CpDoReplace(vcpMatchFirst, vcpMatchLim, prcb, &fDidIt)) == cpNil)
			{
			EnableInval();
			/* NOTE: we MUST do the whole doc; FQueryAbortCheck can cause a
			   call to UpdateWw which can redraw the OLD text and erase the
			   edl.fDirty bits, thus leaving the screen incorrect */
			InvalCp1Sub(PcaSetWholeDoc(&caT, selCur.doc));
			InvalCaFierce();
			if (vfbReplace.fPap && vfbReplace.papGray.stc != -1)
				DirtyOutline(caT.doc);
			fRet = fFalse;
			goto LRet;
			}

		cpSearchStart = vcpMatchFirst + ccpReplace;
		cpSearchLim += cpSearchStart - vcpMatchLim; /* dcpAdjust */

		if (fDidIt)
			prcb->iChange++;
		}

	if (vprri->cpLimWrap == cpNil)
		vprri->cpLimWrap = vcpLimWrap;

	EnableInval();
	/* NOTE: we MUST do the whole doc; FQueryAbortCheck can cause a
	   call to UpdateWw which can redraw the OLD text and erase the
	   edl.fDirty bits, thus leaving the screen incorrect */
	InvalCp1Sub(PcaSetWholeDoc(&caT, selCur.doc));
	InvalCaFierce();
	if (vfbReplace.fPap && vfbReplace.papGray.stc != -1)
		DirtyOutline(caT.doc);

	/* abort came from lower level (CpSearchSz) - we do not distinguish
		between aborting and not finding the text with the cpNil return
		value; however FQueryAbortCheck() called a second time here will
		return true if user aborted in CpSearchSz() */
	if (FQueryAbortCheck())
		{
		fRet = fFalse;
		goto LRet;
		}

/* check for going around the end of the document */
	if (!vfNoWrap)
		{
/* need to update ww because FIdle won't be executed until after dialog
box is long gone, and we want to see results of replace(s) immediately */
		UpdateWw( wwCur, fFalse );
		if (IdMessageBoxMstRgwMb(vfFwd ? mstSearchWrapFwd : mstSearchWrapBkwd,
				NULL, MB_YESNOQUESTION) == IDYES)
			{
			cpNextRpt = 0;
			vprri->cpHighRpt = cpSearchLim = vprri->cpLimWrap;
			cpSearchStart = cp0;
			vprri->cpLowRpt = vprri->cpLimWrap - CpMacDoc(selCur.doc); /* intentionally negative */
			vfNoWrap = fTrue;
			goto LRestartChangeAll;
			}
		}

	Select(&selCur, caAdjust.cpFirst, caAdjust.cpLim);

LRet:
	StopProgressReport(hppr, pdcAdvise);
/* Note: NOT PcaSetWholeDoc - that only goes up to CpMacDocEdit! */
	SetUndoAfter(PcaSet( &caT, selCur.doc, cp0, CpMacDoc(selCur.doc)));
	EndLongOp( fFalse );
	return fRet;
}


/* C P  D O  R E P L A C E */
/* Does one replacement.  Called multiple times for a global replace.
	cpMatchFirst, cpMatchLim are the bounds of the text to be replaced
	in the doc (selCur.doc assumed).  Replacement string is in
	prcb->stReplace.  Other necessary info for the replace is in
	vfbReplace.

	Returns the number of cp's to advance to start searching for next
	occurrence.
*/

/* %%Function:CpDoReplace %%Owner:rosiep */
CP CpDoReplace(cpMatchFirst, cpMatchLim, prcb, pfDidIt)
CP cpMatchFirst, cpMatchLim;
struct RCB *prcb;
BOOL *pfDidIt;
{
	extern int CacheParaPRC();

	CP dcpMatch;
	CP cpIns;
	CP dcpIns;
	CP dcpAdjust = 0;
	CP dcpEopAdjust = 0;
	FC cfcReplace = 0;
	int fTableBeginsScrap; 
	struct SRD *psrd;
	struct SRD *rgsrd = prcb->rgsrd;  /* local for speed */
	int doc;
	struct CA caIns;
	struct CA caT1, caT2;
	CP dcpT;
	BOOL fInTable;

	*pfDidIt = fFalse;
	fInTable = FInTableDocCp(selCur.doc, cpMatchFirst);

	/* don't allow replacing a cell mark */
	if (vfConfirm && selCur.fTable && vfbReplace.fText)
		{
		/* Search should never find us a cell mark to replace, but just
			in case, we have this message and we bag out. */
/* FUTURE (rp): If the Report can't be caused to fail, lets remove the message */
		ReportSz("Should never get here!");
		ErrorEid(eidCantReplaceCellMark, "CpDoReplace");
		return (cpMatchLim - cpMatchFirst);
		}

	/* don't allow replacing a table into a table */
	if (vfbReplace.fReplScrap && FInTableDocCp(selCur.doc, cpMatchFirst)
			&& FTableInPca(PcaSet(&caT1, docScrap, cp0, CpMacDocEdit(docScrap))))
		{
		ErrorEid(eidIllegalTextInTable, "CpDoReplace");
		return (cpMatchLim - cpMatchFirst);
		}

	/* OK to change formatting of final Eop, but not OK to include it in
		text replacement */
	if (vfbReplace.fText && cpMatchLim > CpMacDocEdit(selCur.doc))
		{
		CP cpT = CpMacDocEdit(selCur.doc);
		dcpEopAdjust = cpMatchLim - cpT;
		cpMatchLim = cpT;
		}

	dcpMatch = cpMatchLim - cpMatchFirst;

	if (vfConfirm)
		{
		PcaSetDcp(&caT1, selCur.doc, cpMatchFirst, dcpMatch+dcpEopAdjust);
		if (vfbReplace.fPap)
			{
			ExpandCaCache(&caT1, &caT2, &caPara, sgcPap, CacheParaPRC);
			caT1 = caT2;
			}
		if (!FSetUndoB1(vfbReplace.fText ? imiReplace : ucmFormatting,
				vfbReplace.fText ? uccPaste : uccFormat, &caT1))
			{
			return cpNil;
			}
		}

	if (vfbReplace.fText)
		{
		cfcReplace = (FC) prcb->stReplace[0];
		if (!FAssignFnFcForStReplace(prcb, selCur.doc, cpMatchFirst,
				cpMatchLim, vfSearchCase))
			{
			return cpNil;
			}

		PcaSetDcp(&caT2, selCur.doc, cpMatchFirst, dcpMatch);

		/* if there was a ^m in the replacement text, we'll need to
			squirrel away the text we are replacing for insertion later */

		if (vfbReplace.fReplMatch)
		/* Note: NOT PcaSetWholeDoc - that only goes up to CpMacDocEdit! */
			if (!FReplaceCps(PcaSet(&caT1, vdocScratch, cp0,
					CpMacDoc(vdocScratch)), &caT2))
				{
				return cpNil;
				}

		/* the text in fnReplace has had all ^m and ^c characters squeezed
			out already (by SetParaReplace); we will be filling in those gaps
			later */

		/* Do the actual replace */
		if (!FReplaceRM(&caT2, prcb->fnReplace, prcb->fcReplace, cfcReplace))
			return cpNil;
		dcpMatch = DcpCa(&caT2);

		/* now replace instances where ^m and ^c were with matched text
			(in vdocScratch) and clipboard contents, respectively */
		fTableBeginsScrap = FInTableDocCp(docScrap, cp0); 
		for (psrd = rgsrd, cpIns = cpMatchFirst;
				psrd->srt != srtNil && psrd - rgsrd < csrdMax;
				psrd++, cpIns += dcpIns, dcpAdjust += dcpIns)
			{
			if (psrd->srt == srtClipboard)
				{
				doc = docScrap;
				dcpIns = CpMacDocEdit(doc);
				}
			else
				{
				Assert (psrd->srt == srtMatched);
				doc = vdocScratch;
				dcpIns = CpMacDoc(doc);
				}

			Assert (doc != docNil);
			
			cpIns += psrd->dcp;
			if (fTableBeginsScrap && psrd->srt == srtClipboard &&
				FEopBeforePca(PcaPoint(&caT1, selCur.doc, cpIns)) &&
				caT1.cpFirst != cpIns)
				{
				cpIns += ccpEop; 
				dcpAdjust += ccpEop; 
				}

			if (!FReplaceCpsRM(PcaSetDcp(&caT1, selCur.doc, cpIns, cp0),
					PcaSet(&caT2, doc, cp0, dcpIns)))
				{
				return cpNil;
				}

			AssureNestedPropsCorrect(PcaSetDcp(&caT1, selCur.doc, cpIns, dcpIns), fTrue);
			}

		if (FRareT(reMemAlert6, vmerr.fMemFail))
			{
			FlushPendingAlerts();
			return (cpNil);
			}
		}

	/* at this point, dcpMatch contains the length of the portion of the
		matched string which is still there (wrt revision marking) - if
		we're only replacing properties, this is the entire text */

	/* caIns is strictly the new text */
	dcpIns = cfcReplace + dcpAdjust + dcpEopAdjust;
	PcaSetDcp( &caIns, selCur.doc, cpMatchFirst, dcpIns +
			(vfbReplace.fText ? 0 : dcpMatch));

	/* replace any properties if applicable */
	if (vfbReplace.fChp || vfbReplace.fPap)
		ReplacePropsCa(prcb->prpp, &caIns);

	/*  assure dead field properties correct */
	AssureNestedPropsCorrect(&caIns, fInTable);

	if (vfConfirm)
		{
		/* Undo must include Ins text AND what was left from Rev Marking! */
		PcaSetDcp( &caT1, caIns.doc, cpMatchFirst, dcpIns + dcpMatch );
		if (vfbReplace.fPap)
			{
			ExpandCaCache(&caT1, &caT2, &caPara, sgcPap, CacheParaPRC);
			caT1 = caT2;
			}
		SetUndoAfter( &caT1 );
		}

	/* adjust for expansion/contraction of replacement text */
	/* ...take into account revision marking */
	if (vfbReplace.fText)
		{
		dcpT = dcpIns + dcpMatch - (cpMatchLim + dcpEopAdjust - cpMatchFirst);
		ReplAdjust(&vcpSearchLim, dcpT, cpMatchLim);
		ReplAdjust(&vprri->cpHighRpt, dcpT, cpMatchLim);
		ReplAdjust(&vprri->cpLimWrap, dcpT, cpMatchLim);
		}

	/* distance to advance to start searching for next occurrence */
	*pfDidIt = fTrue;
	return (dcpIns + dcpMatch);
}


/* R E P L  A D J U S T */
/* Adjust cp if necessary (dcp can be negative) */
/* %%Function:ReplAdjust %%Owner:rosiep */
ReplAdjust(pcp, dcp, cpLim)
CP *pcp, dcp, cpLim;
{
	if (*pcp >= cpLim)
		*pcp += dcp;
	else  if (*pcp > cpLim + dcp) /* (dcp < cp0) implied */
		*pcp = cpLim + dcp;
}


/* note: change the defines below if you change this structure */
csconst char rgsprmReplace[] =
{
	sprmCFBold,
			sprmCFItalic,
			sprmCFSmallCaps,
			sprmCFVanish,
			sprmCFtc,
			sprmCKul,
			sprmCHps,
			sprmCHpsPos,
			sprmCIco,
			sprmPStc,
			sprmPJc,
			sprmPDyaLine,
			sprmPDyaBefore
};


#define isprmCMin   0
#define isprmCMax   9
#define isprmPMin   9
#define isprmPMax  13

/*
*  B U I L D  R E P L A C E  G R P P R L S
*
*  Builds grpprls to apply for replacing properties based on the info
*  in vfbSearch and vfbReplace.  Only properties which are ungray in
*  either vfbSearch or vfbReplace will be touched.  They will be
*  replaced with the value in vfbReplace.chp (or .pap).  There are
*  two exceptions to this rule.  If ftc or hps is gray in vfbReplace,
*  they will not be changed regardless of their grayness or lack
*  thereof in vfbSearch.  This is because there would be no reasonable
*  value to change them to, whereas with all the other properties, it
*  would make the most sense to turn them off, or give them 0 as a
*  value.  (6/19/89 - there are now more exceptions, properties for
*  which gray in the replace properties means "leave it alone", not
*  "turn it off").
*
*  We figure out a separate grpprl for the character properties and
*  the paragraph properties.  They must be done separately because
*  ApplyGrpprlSelCur (called from ReplacePropsCa) will expand the ca
*  to an integral # of paras and we don't want that for the character
*  properties.
*
*/

/* %%Function:BuildReplaceGrpprls %%Owner:rosiep */
BuildReplaceGrpprls(prpp)
struct RPP *prpp;
{
	char prl[4];
	char sprm;
	int cchPrl, val, isprm;
	char *grpprlChp = prpp->grpprlChp;
	char *grpprlPap = prpp->grpprlPap;

	prpp->cbgrpprlChp = 0;
	prpp->cbgrpprlPap = 0;

	/* do character properties if there are any */
	if (vfbReplace.fChp || vfbSearch.fChp)
		{
		for (isprm = isprmCMin; isprm < isprmCMax; isprm++)
			{
			sprm = rgsprmReplace[isprm];

			/* if prop is gray in both Search and Replace chp, ignore it;
				also ignore special case of ftc and hps mentioned above */

			if (ValFromPropSprm(&vfbReplace.chpGray, sprm) &&
					(ValFromPropSprm(&vfbSearch.chpGray, sprm) ||
					sprm == sprmCFtc || sprm == sprmCHps))
				{
				continue;
				}

			/* build up the grpprl */
			val = ValFromPropSprm(&vfbReplace.chp, sprm);
			cchPrl = CchPrlFromSprmVal (prl, sprm, val);
			grpprlChp = bltbyte(prl, grpprlChp, cchPrl);
			prpp->cbgrpprlChp += cchPrl;
			}
		}

	/* do paragraph properties if there are any */
	if (vfbReplace.fPap | vfbSearch.fPap)
		{
		for (isprm = isprmPMin; isprm < isprmPMax; isprm++)
			{
			sprm = rgsprmReplace[isprm];

			/* if prop is gray in both Search and Replace pap, ignore it;
			also ignore special cases for which gray in the replace
			properties means "leave it alone", not "turn it off" */

			if (ValFromPropSprm(&vfbReplace.papGray, sprm) &&
					(ValFromPropSprm(&vfbSearch.papGray, sprm) ||
					sprm == sprmPJc || sprm == sprmPDyaLine ||
					sprm == sprmPDyaBefore))
				{
				continue;
				}

			/* build up the grpprl */
			val = ValFromPropSprm(&vfbReplace.pap, sprm);
			cchPrl = CchPrlFromSprmVal (prl, sprm, val);
			grpprlPap = bltbyte(prl, grpprlPap, cchPrl);
			prpp->cbgrpprlPap += cchPrl;
			}
		}
}


/* I N I T  R E P L A C E  F O R M A T T I N G */
/* Initialize formatting block for Replace */

/* %%Function:InitReplaceFormatting %%Owner:rosiep */
InitReplaceFormatting()
{
	InitSearchFormatting();

	if (!vfbReplace.fInfo || (*hwwdCur)->wk == wkMacro)
		{
		InitSRChps(fFalse /* fSearch */);
		InitSRPaps(fFalse /* fSearch */);
		}

	vfbSearch.chp.fRMark = 0;
	vfbSearch.chp.fStrike = 0;
}


/* %%Function:WCaseCp %%Owner:rosiep */
NATIVE WCaseCp(doc, cp, dcp)
int doc;
CP  cp;
CP dcp;
{
/* Determines capitalization pattern in a piece of text.  Used when doing
	replace to match existing pattern.  returns an int which is one of:
	0 - Not initial capital
	1 - Initial Capital but lower case appears later
	2 - Initial Capital and no lower case in the string
	Assumes a valid cp, dcp pair.
*/
	int ichDoc;
	CP cpRun;

	CachePara(doc, cpRun = cp);
	/* Note: can do FetchCp here because we know we have visible chars at
		this point */
	FetchCp(doc, cp, fcmChars + fcmParseCaps);
	if (!FUpper(vhpchFetch[0]))
		return(0);

	/* we now know there is an initial cap.  Are there any lower case chars? */
	for (ichDoc=1; vcpFetch+ichDoc < cp + dcp;)
		{
		if (ichDoc >= vccpFetch)
			{
			CachePara(doc, cpRun += vccpFetch);
			FetchCp(docNil, cpNil, fcmChars + fcmParseCaps);
			ichDoc = 0;
			continue;
			}
		if (FLower(vhpchFetch[ichDoc++]))
			return(1);
		}
	/* No lower case letters were found. */
	return(2);
}


/* A S S I G N  F N  F C  F O R  S T  R E P L A C E */
/* %%Function:FAssignFnFcForStReplace %%Owner:rosiep */
BOOL FAssignFnFcForStReplace(prcb, doc, cpMatchFirst, cpMatchLim, fSearchCase)
struct RCB *prcb;
int doc;
CP cpMatchFirst, cpMatchLim;
BOOL fSearchCase;
{{ /* NATIVE - FAssignFnFcForStReplace */
	int iCurCase;
	int cch;
	int ich;
	struct CHP chp;
	struct CHP chpT;
	BOOL fSuccess = fTrue;

	CachePara(doc, cpMatchFirst);

	/*  this gets the correct chp for an insertion */
	Assert( doc == selCur.doc );
	StartUMeas(umAssignFnFc1);
	GetPchpDocCpFIns (&chp, doc, cpMatchFirst, fFalse, selCur.ww);
	if (vfbReplace.fChp)
		MergePropsPropGray(&chp, &vfbReplace.chp, &vchpSRGray, cwCHP);
	StopUMeas(umAssignFnFc1);

	/* if we're distinguishing between upper and lower case we don't
		have to pay attention to capitalization pattern. if we don't
		distinguish between upper and lower case for search, we call
		WCaseCp to determine the capitalization pattern of the matched
		string */
	StartUMeas(umAssignFnFc2);
	iCurCase = (fSearchCase ? 0 : WCaseCp(doc, cpMatchFirst, cpMatchLim -
			cpMatchFirst));

	/* if the new capitalization pattern of the matched string
		doesn't match the current contents of stRealReplace,
		copy the replacement string to stRealReplace and transform
		stRealReplace to conform to the new pattern */
	cch = prcb->stReplace[0];
	if (iCurCase != prcb->iLastCase)
		{
/*
		case 0:   no special capitalization required
		case 1:   first character of string must be capitalized
		case 2:   all characters must be capitalized
*/
		if (iCurCase == 2)
			{    /* all characters must be capitalized */
			for (ich = 1; ich < cch+1; ich++)
				stRealReplace[ich] = ChUpper(prcb->stReplace[ich]);
			}
		else
			{       /* no special capitalization required */
			bltb(prcb->stReplace, stRealReplace, cch+1);
			if (iCurCase == 1)
				/* first character of string must be capitalized */
				stRealReplace[1] = ChUpper(prcb->stReplace[1]);
			}
		}
	StopUMeas(umAssignFnFc2);

	/* if the capitalization pattern has changed OR
			the character properties of the replacement text don't match
			those of the last insert        OR
			the replace string contains para marks, THEN
	
			1) call FNewChpIns to write a run describing the character
			properties of the previous insertion text,
			2) call FcAppendRgchToFn to write the characters of the
			replacement string to the scratch file,
			3) if we are replacing paragraph marks, call InsertPapsForReplace
			to write runs describing each paragraph in the replacement
			string */

	StartUMeas(umAssignFnFc3);
	if (iCurCase != prcb->iLastCase ||
			(CbGenChpxFromChp(&chpT, &chp, &vchpStc, fFalse),
			FNeRgch(&vfkpdChp.chp, &chpT, cbCHP)) || prcb->fParaReplace)
		{
		if (!FNewChpIns(doc, cpMatchFirst, &chp, stcNil))
			fSuccess = fFalse;
		else
			{
			prcb->fnReplace = fnScratch;
			prcb->fcReplace = FcAppendRgchToFn(fnScratch, stRealReplace+1, cch);
			if (prcb->fParaReplace)
				if (!FInsertPapsForReplace(doc, stRealReplace, prcb->fcReplace))
					fSuccess = fFalse;
			}
		}
	StopUMeas(umAssignFnFc3);

	prcb->iLastCase = iCurCase; /* record new capitalization pattern */

LRet:
	return fSuccess;

}}


/* S E T  P A R A  R E P L A C E */
/* Sets the global fParaReplace if the user wants to insert Paragraph breaks
	(since special insertion code must be run). */

/* %%Function:SetParaReplace %%Owner:rosiep */
NATIVE SetParaReplace(stReplace, pfParaReplace, rgsrd)
char stReplace[];
int *pfParaReplace;
struct SRD rgsrd[];
{
	char *rgch = stReplace;
	int ich = 0;
	int cch;
	char ch;
	char chNew;
	char chPattern;
	int i;
	int isrd = 0;

	*pfParaReplace = fFalse;

	cch = stReplace[0];
	for (ich = 1; ich < cch + 1; ich++)
		{
		ch = stReplace[ich];
		switch (ch)
			{
		default:
			break;
		case chPrefixMatch:
			switch (rgch[ich+1])
				{
			default:
				chPattern = rgch[ich+1];
				if (FIsDigitS(chPattern))
					{
					int cchT;

					/* parse and count the digits */
					cchT = CchChFromAnsiOrOem(rgch, ich + 1, cch + 1, &chNew);
					bltb(&rgch[ich + cchT + 1], &rgch[ich + 1], cch - ich - cchT);
					cch -= cchT;
					goto LPrefixMatch;
					}
				/* just escaping the next char */
				else  if (rgch[ich+1] == '\0')
					chNew = chPrefixMatch;
				else
					chNew = rgch[ich+1];
				break;
			case chMatchNBS:
				chNew = chNonBreakSpace;
				break;
			case chMatchTab:
				chNew = chTab;
				break;
			case chMatchNewLine:
				chNew = chCRJ;
				break;
			case chMatchNRH:
				chNew = chNonReqHyphen;
				break;
			case chMatchNBH:
				chNew = chNonBreakHyphen;
				break;
			case chMatchSect:
				chNew = chSect;
				break;
			case chMatchEol:
				chNew = chEol;
				break;
#ifdef MACONLY
			case '\\':
				chNew = chFormula;
				break;
#endif /* MACONLY */
			case chUseMatched:
				vfbReplace.fReplMatch = fTrue;
				/* fall through */
			case chUseScrap:
				if (isrd == 0)
					rgsrd[isrd].dcp = ich - 1; /* -1 so we don't count the
							first byte of st */
				else  if (isrd >= csrdMax)
					continue;
				else
					rgsrd[isrd].dcp = ich - rgsrd[isrd-1].dcp - 1;

				rgsrd[isrd++].srt = (rgch[ich+1] == chUseMatched ?
						srtMatched : srtClipboard);

				if (rgch[ich+1] == chUseScrap)
					vfbReplace.fReplScrap = fTrue;

				bltbyte(&(rgch[ich+2]), &(rgch[ich]), cch-ich-1);
				stReplace[0] = (cch -= 2);
				ich--;  /* squeeze this char out, so don't increment */
				continue;
				}
#ifdef CRLF
			if (chNew != chEol)
				{
				bltbyte(&(rgch[ich+1]),&(rgch[ich]), cch-ich);
				cch--;
				}
#else
			bltbyte(&(rgch[ich+1]),&(rgch[ich]), cch-ich);
			cch--;
#endif /* CRLF */
LPrefixMatch:
			stReplace[0] = cch;

			if (chNew == chEol)
				{
				*pfParaReplace = fTrue;
#ifdef CRLF
				rgch[ich++] = chReturn;
#endif
				}
			rgch[ich] = chNew;
			break;
		case chEol:
			*pfParaReplace = fTrue;
			break;
			}
		}

	if (isrd != csrdMax)
		rgsrd[isrd].srt = srtNil;  /* must do this to mark the end of valid
			SRD entries */
	return;
}



/**  ROUTINES FOR CONFIRM REPLACE DIALOG  **/


/* %%Function:TmcConfirmReplace %%Owner:rosiep */
TmcConfirmReplace()
{
	int tmc;
	int cwAlloc;
	CMB cmb;
	CABCONFIRMREPL * pcabConfirmRepl;
	CHAR dlt [sizeof(dltConfirmRepl)];  /* local copy of dlt structure */

	/* local copy of csconst dgd structure */
	BltDlt(dltConfirmRepl, dlt);

	/* initialize Command Argument Block */
	if ((cmb.hcab = HcabAlloc(cabiCABCONFIRMREPL)) == hNil)
		return tmcError;
	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	pcabConfirmRepl = *cmb.hcab;
	pcabConfirmRepl->fConfirm = fTrue;

	tmc = TmcOurDoDlg(dlt, &cmb);
	FreeCab(cmb.hcab);

	return tmc;
}


/* %%Function:FDlgConfirmRepl %%Owner:rosiep */
BOOL FDlgConfirmRepl(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{
	static BOOL fInProcess;
	HWND hwnd;
	struct RC rcDlg, rcApp;

	switch (dlm)
		{
	case dlmInit:

		if (FIsDlgDying())  /* dialog will be brought down anyway */
			return (fTrue);

		/* position the dialog out of the way */
		GetWindowRect(vhwndApp, (LPRECT) &rcApp);
		hwnd = HwndFromDlg(HdlgGetCur());
		GetWindowRect(hwnd, (LPRECT) &rcDlg);
		MoveDlg(rcDlg.xpLeft, rcApp.ypBottom - vsci.dypBdrCapt -
				rcDlg.ypBottom + rcDlg.ypTop);

		fInProcess = fFalse;
		SetDialogSearching();
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */

		if (fInProcess)
			break;  /* will return true and stop idle */

		/* we should only get here the first time the dialog gets
			this message; this is the processing that starts the
			first search; we don't do it at dlmInit because the
			dialog (saying "Searching..." isn't visible yet */

		if (FNextMatch(fFalse))
			{
			SetDialogFound();
			fInProcess = fTrue;
			}
		else
			{
			EndDlg(tmcCancel);  /* ok to call EndDlg here */
			return fTrue;
			}
		return fFalse;  /* keep idling */

	case dlmClick:
		if (tmc == tmcReplConfirm)
			SetTmcText(tmcReplText, ValGetTmc(tmc) ? SzSharedKey("Replace selection?", ReplaceSelection) :
					SzSharedKey("Replace all?", ReplaceAll));
		break;

	case dlmTerm:
		switch (tmc)
			{
		case tmcReplYes:
		case tmcReplNo:
			if (!(vfConfirm = ValGetTmc(tmcReplConfirm)))
				break;
			SetDialogSearching();
			if (tmc == tmcReplYes ? FChangeOne() : FNextMatch(fTrue))
				{  /* found another occurrence */
				SetDialogFound();
				return fFalse; /* do not dismiss dialog */
				}
			/* else that was the last one */
			return fTrue; /* do dismiss dialog */

		default:
			break;
			}
		break;
	default:
		break;
		}

	return (fTrue);
}



/* %%Function:SetDialogFound %%Owner:rosiep */
SetDialogFound()
{
	SetTmcText(tmcReplText, SzSharedKey("Replace selection?",ReplaceSelection));
	EnableTmc(tmcReplYes, fTrue);
	SetDefaultTmc(tmcReplYes);
	EnableTmc(tmcReplNo, fTrue);
	EnableTmc(tmcReplConfirm, fTrue);
}


/* %%Function:SetDialogSearching %%Owner:rosiep */
SetDialogSearching()
{
	SetTmcText(tmcReplText, SzSharedKey("Searching...",SearchingNow));
	SetDefaultTmc(tmcCancel);
	EnableTmc(tmcReplNo, fFalse);
	EnableTmc(tmcReplYes, fFalse);
	EnableTmc(tmcReplConfirm, fFalse);
}


/* F  I N S E R T  P A P S  F O R  R E P L A C E */
/* do AddRunScratch for every distinct paragraph in stReplace  */
/* This is only needed when stReplace contains one or more chEols */

/* %%Function:FInsertPapsForReplace %%Owner:rosiep */
BOOL FInsertPapsForReplace(doc, stReplace, fc)
int doc;
char stReplace[];
FC fc;
{
	int cchInsTotal = 0;
	char *pchTail;
	char *pchHead;
	int cchPapx;
	char papx[cchPapxMax+1];
	char *szReplace;

	struct PAP papStd;


	szReplace = stReplace + 1;
	szReplace[stReplace[0]] = 0;
	for (;;)
		{
		int cch;

		pchHead = szReplace + cchInsTotal;
		pchTail = index(pchHead, chEol);
		if (pchTail == 0)
			return fTrue;
		cch = pchTail - pchHead + 1; /* cch is count including chEol */

		fc += cch;
		cchInsTotal += cch;
		SetWords(&papStd, 0, cwPAP);
		MapStc(PdodDoc(doc), vpapFetch.stc, 0, &papStd);
		/* FUTURE: am setting clMac to 0 so layout stuff can be
			tested. When we have code to tell us the current
			PHE for a paragraph, it should be called here. */
/* rosiep 9/2/87 - "code" referred to above still doesn't exist in MacWord */
		vpapFetch.phe.clMac = 0;
		cchPapx = CbGenPapxFromPap(&papx, &vpapFetch, &papStd, fFalse);


		if (!FAddRun(fnScratch,
				fc,
				&papx,
				cchPapx,
				&vfkpdPap,
				fTrue /* PAP */,
				fTrue /* alloc at fcMac */,
				fTrue,
				fFalse))
			{
			return fFalse;
			}
		}

	return fTrue;
}


/* %%Function:CmdReplaceChar %%Owner:rosiep */
CMD CmdReplaceChar(pcmb)
CMB * pcmb;
{
	InitReplaceFormatting();

	if (pcmb->fDefaults)
		GetCharDefaults(pcmb, &vfbReplace.chp, &vfbReplace.chpGray);

	if (pcmb->fAction)
		{
		CabToPropPropGray(pcmb->hcab, &vfbReplace.chp, 
				&vfbReplace.chpGray, sgcChp);
		CheckForFormatting();
		}

	return cmdOK;
}


/* %%Function:CmdReplacePara %%Owner:rosiep */
CMD CmdReplacePara(pcmb)
CMB * pcmb;
{
	InitReplaceFormatting();

	if (pcmb->fDefaults)
		if (!FGetParaDefaults(pcmb, &vfbReplace.pap,
				&vfbReplace.papGray, fTrue /* fPaps */, fFalse /* fStyList */))
			{
			return cmdError;
			}

	if (pcmb->fAction)
		{
		CabToPropPropGray(pcmb->hcab, &vfbReplace.pap, 
				&vfbReplace.papGray, sgcPap);
		CheckForFormatting();
		}

	return cmdOK;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Replace_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Replace_Last() */
