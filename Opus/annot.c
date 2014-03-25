#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "ch.h"
#include "debug.h"
#include "cmd.h"
#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "field.h"
#include "error.h"
#include "rareflag.h"

#include "footnote.hs"
#include "footnote.sdm"

extern struct MWD       **hmwdCur;
extern struct WWD       **hwwdCur;
extern int              wwCur;
extern int              vifnd;
extern int              vfSeeSel;
extern CHAR             rgchEop[];
extern struct MERR      vmerr;
extern struct DOD       **mpdochdod[];
extern struct SEL       selCur;
extern struct CHP       vchpStc;
extern struct UAB       vuab;
extern struct CA        caSect;
extern struct SEP       vsepFetch;
extern struct PREF      vpref;
extern CHAR             szEmpty[];
extern BOOL             vfRecording;
extern BOOL		    fElActive;

extern CP CpFirstBlock();


csconst CHAR szArgs[] = SzSharedKey(" \\# \"\'Page: \'#\'\013\'\"",Args); /*  \# "'Page: '#'chCRJ'" */

#define cchArgs (18)
#define cchFtnRefMax    10
#define iagStRef Iag(CABINSERTFTN, hstRef)

csconst CHAR szAtn[] = {
	chAtn, '\0'};


/* %%Function:CmdViewRef %%Owner:chic */
CMD CmdViewRef(wk, edc)
int wk;
int edc;
{
	BOOL fRecordingSav;
	CMD cmd = cmdError;

	Assert (PselActive() == &selCur);

	InhibitRecorder(&fRecordingSav, fTrue);

	if ((!vrf.fVetoViewRef || PdodDoc(selCur.doc)->fShort) &&
			((*hmwdCur)->wwLower != wwNil && 
			PwwdWw((*hmwdCur)->wwLower)->wk == wk))
		{
		KillSplit(hmwdCur, fTrue /* fLower */, fFalse /*fAdjustWidth*/);
		cmd = cmdOK;
		}
	else  if (!FHasRefs(DocMother(DocBaseWw(wwCur)), edc))
		{
		ErrorEid(wk == wkFtn ? eidNoFNToShow : eidNoANNToShow,"CmdViewRef");
		}
	else  if (!FReportSplitErr(wk))
		{
		EnsureSelInWw(fFalse, &selCur);
		cmd = FViewFtnAtn(edc) ? cmdOK : cmdError;
		}

	InhibitRecorder(&fRecordingSav, fFalse);

	return cmd;
}



/* %%Function:CmdViewAnnotation %%Owner:chic */
CMD CmdViewAnnotation(pcmb)
CMB *pcmb;
{
	Assert(hmwdCur && hwwdCur);
	Assert(PdodDoc(selCur.doc)->fAtn || PdodDoc(selCur.doc)->fMother);
	return CmdViewRef(wkAtn, edcDrpAtn);
}



/* %%Function:CmdInsAnnotation %%Owner:chic */
CMD CmdInsAnnotation(pcmb)
CMB *pcmb;
{
	int doc = selCur.doc;
	int docAtn;
	struct WWD *pwwdCur;
	struct DOD *pdod = PdodDoc(doc);
	CP cpT, cpRef = selCur.cpFirst;
	CP cpAtn;
	int cstcStd;
	int iand;
	BOOL fAtn, ifld;
	struct CA ca, caT;
	int cmdRet = cmdOK;
	BOOL fRecordingSav;
	struct CHP chpText; /* chp for annotation text */
	struct CHP chpRef; /* chp for annotation mark in annotation doc */
	struct CHP chpRefDoc; /* chp for annotation mark in mother doc - fVanish is true */
	struct PAP papText;
	struct PLC **hplcatrd;
	struct PLC **hplcand;
	struct ATRD atrd;
	struct FLCD flcd;
	CHAR szKeyword [cchMaxFieldKeyword + 1];

	Assert(doc != docNil);

/* only allow insert annotation in mother doc or annotation pane */
	if (!pdod->fMother && !pdod->fAtn)
		{
		Beep();
		return cmdError;
		}

	if (FReportSplitErr(wkAtn))
		return cmdError;

	StartLongOp();

	if (fAtn = (pdod = PdodDoc(doc))->fAtn)
		{
		docAtn = doc;
		doc = pdod->doc;
		}

/* Get the exact cpFirst of the document, i,e. in case of a block selection.
If in annotataion pane, ignore selCur anyway since always insert at beginning
of annotation text.
*/
	if (selCur.fBlock && !pdod->fAtn)
		cpRef = CpFirstBlock();

/* get user initials to compose annotation ref mark */

	Assert(vpref.stUsrInitl[0] < ichUsrInitlMax);
	CopySt(vpref.stUsrInitl, &atrd);

/* Prepare chp desired for reference and pap and chp desired for annotation text.
*/
	TurnOffSel(&selCur);
	if ((!FStcDefined(doc, stcAtnRef) && 
			!FEnsureStcDefined(doc, stcAtnRef, &cstcStd)) ||
			(!FStcDefined(doc, stcAtnText) &&
			!FEnsureStcDefined(doc, stcAtnText, &cstcStd)))
		{
LError: 
		cmdRet = cmdError;
		goto LRet;
		}
	pdod = PdodDoc(doc);

/* chp and pap are used temporarily by the next line for the reference */
	MapStc(pdod, stcAtnRef, &chpRefDoc, &papText);
	chpRefDoc.fSpec = fTrue;
	chpRef = chpRefDoc;
	chpRefDoc.fVanish = fTrue; /* make it hidden text */

	MapStc(pdod, stcAtnText, &chpText, &papText);
	papText.stc = stcAtnText;
	chpText.fRMark = chpRef.fRMark = chpRefDoc.fRMark = PdodMother(doc)->dop.fRevMarking;

	if (fAtn)
		{
/* in annotation document: insert ref in front of the annotation text */
		if (cpRef >= CpMacDocEdit(docAtn))
			ErrorEid(eidNotValidForAnn,"CmdInsAnnotation");
		else
			{
			hplcand = PdodDoc(docAtn)->hplcand;
			cpAtn = CpPlc( hplcand, IInPlc(hplcand, cpRef));
			if (FSetUndoB1(bcmInsAnnotation, uccPaste, PcaPoint(&caT, docAtn, cpAtn)))
				{
				FInsertRgch(docAtn, cpAtn, SzNear(szAtn), 1, &chpRef, 0);
				SetUndoAfter(PcaSetDcp(&caT, docAtn, cpAtn, (CP)1));
				}
			else
				cmdRet = cmdCancelled;
			}
		goto LRet;
		}

	if ((docAtn = DocSubEnsure(doc,edcDrpAtn)) == docNil ||
			!FSetUndoBefore(bcmInsAnnotation, uccInsert))
		{
LNoMem: 
		cmdRet = cmdNoMemory;
		goto LRet;
		}

/* no autodelete */
/* insert annotation reference to mother doc at cpRef */
	if (!FInsertRgch(doc, cpRef, SzNear(szAtn), 1, &chpRefDoc, 0))
		goto LNoMem;

/* correct for dead field */
	AssureNestedPropsCorrect(PcaSet(&caT, doc, cpRef, cpRef+1), fTrue);

	cpAtn = CpFirstRef(doc, cpRef, &cpT, edcDrpAtn);

/* Insert in docAtn */
/* we now have: vifnd, cpAtn of the next annotation text. Our annotation
should appear in front of this.
Insert eop for a start following cpAtn.
*/
	if (!FInsertRgch(docAtn, cpAtn, rgchEop, (int)ccpEol, &chpText, &papText))
		goto LNoMem;

/* Update hplcand's and hplcatrd's in both documents */
	hplcatrd = PdodDoc(doc)->hplcatrd;
	hplcand = PdodDoc(docAtn)->hplcand;
	if (!FInsertInPlc(hplcatrd, vifnd, cpRef, &atrd))
		goto LNoMem;
	if (!FOpenPlc(hplcand, vifnd, 1))
		{
		FOpenPlc(hplcatrd, vifnd, -1); /* delete what just inserted */
		goto LNoMem;
		}
	PutCpPlc( hplcand, vifnd, cpAtn );
/* cause FInsertRgch is inserting at the
	wrong annotation (cpAtn is of the next annotation text)  */
	PutCpPlc( hplcand, vifnd + 1, CpPlc( hplcand, vifnd + 1) + ccpEol );
	iand = vifnd;

/* Insert reference mark in front of the annotation text */
	Assert(!chpRef.fVanish);
/* Insert a page field for simplicity in print */
	if (!FInsertRgch(docAtn, cpAtn, SzNear(szAtn), 1, &chpRef, 0) ||
			!FInsertFieldDocCp(docAtn, cpAtn, fFalse/* fSeparator */))
		{
		goto LNoMem; /* so we end up with no page field and/or no atn mark */
		}
	GetFltKeyword(fltPage, szKeyword);
	if (!FInsertRgch(docAtn, cpAtn+1, SzNear(szArgs), cchArgs, &chpText, NULL) ||
			!FInsertRgch(docAtn, cpAtn+1, szKeyword, CchSz(szKeyword)-1, &chpText, NULL))
		goto LNoMem;

	ifld = IfldFromDocCp(docAtn, cpAtn, fTrue);
	Assert(ifld != ifldNil);
	GetIfldFlcd(docAtn, ifld, &flcd);
	flcd.flt = fltPage;
	flcd.fDirty = fFalse;
	flcd.fDiffer = fFalse;
	SetFlcdCh(docAtn, &flcd, 0);

/* select insertion point after the reference */
	cpRef ++;
	SelectIns(&selCur, cpRef);

/* invalidate other references to be renumbered */
	cpT = CpPlc( hplcatrd, iand + 1 );
	InvalCp( PcaSet( &ca, doc, cpT, CpMacDocEdit(doc)));
	cpT = CpPlc( hplcand, iand + 1 );
	InvalCp(PcaSet( &ca, docAtn, cpT, CpMacDocEdit(docAtn)));

	PushInsSelCur();
	NormCp(wwCur, selCur.doc,cpRef, ncpHoriz, 0, selCur.fInsEnd);

	InhibitRecorder(&fRecordingSav, fTrue);
	FShowFtnAtn( doc, wkAtn ); /* ok if fail */
	InhibitRecorder(&fRecordingSav, fFalse);

/* if annotation window exists, make it current and select the end of the
annotation text */
	pwwdCur = *hwwdCur;
	if (pwwdCur->wk == wkAtn)
		{
		CP cp;
/* note: wwCur = annotation pane */
/* select in front of terminating eop. Note cpRef points to after the
reference now. */
		Assert( (*hmwdCur)->fSplit );
		cp = CpFirstRef(doc, cpRef, &cpT,edcDrpAtn) - ccpEop;
		SelectIns(&selCur, cp);
		vfSeeSel = fTrue;
		}
	Assert(cpRef > cp0);
	SetUndoAfter(PcaSetDcp(&caT, doc, cpRef-1, (CP)1));
LRet:
	EndLongOp(fFalse /*fAll*/);
	return cmdRet;
}


/* %%Function:FViewFtnAtn %%Owner:chic */
FViewFtnAtn(edcDrp)
int edcDrp;
{ /* View either footnote or annotation */

	int doc = selCur.doc;
	CP cpRefDoc = selCur.cpFirst;
	int wk = (edcDrp == edcDrpFtn ? wkFtn : wkAtn);
	CP cp, cpT;

	if (FShowFtnAtn(doc, wk))
		{
		if (PwwdWw((*hmwdCur)->wwActive)->wk == wk)
			{
/* note: wwCur = footnote pane or annotaion pane */
/* select in front of footnote or annotation text */
			cp = CpFirstRef(doc, cpRefDoc, &cpT, edcDrp);
			}
		else  if ((*hwwdCur)->fPageView)
			{
			Assert(edcDrp == edcDrpFtn);
			if (PdodDoc(doc)->fShort)
				{
				cp = CpRefFromCpSub(doc, cpRefDoc, edcDrp);
				doc = DocMother(doc);
				cp = CpMin(CpMacDocEdit(doc), cp);
				}
			else
				{
				cp = CpFirstRef(doc, cpRefDoc, &cpT, edcDrp);
				doc = PdodDoc(doc)->docFtn;
				Assert(doc != docNil);
				if (cp == CpMacDocEdit(doc) && (cp > cp0 /* has footnote */))
					{
					/* in pageview, cpMacDocEdit of footnote is not showing
					back up cp into the last footnote */

					struct PLC **hplc = PdodDoc(doc)->hplcfnd;
					int iLast = IMacPlc(hplc) - 2;

					AssertH(hplc);
					Assert(iLast >= 0);
					cp = CpMin(cp, CpPlc(hplc, iLast));
					}
				}
			if (IpgdPldrFromDocCp(wwCur, doc, cp) == ipgdNil)
				return fFalse;
			TurnOffSel(&selCur);
			selCur.sk = skNil;
			selCur.doc = doc;
			}
		SelectIns(&selCur, cp);
		vfSeeSel = fTrue; /* also cause mother doc in sync in idle */
		return fTrue;
		}
	return fFalse;
}



/* %%Function:FShowFtnAtn %%Owner:chic */
FShowFtnAtn(doc, wk)
int doc;
int wk;
{
/* 
	Kill the lower pane if it is not the same wk pane.
	Create wk pane if not splitted. 
	Move focus to it if wk pane already exist. 
*/
	struct WWD *pwwdCur;

	if ((pwwdCur = PwwdWw(wwCur))->fPageView && wk == wkFtn)
		{
		if (PdodMother(doc)->dop.fpc == fpcBottomPage)
			ScrollPageDye(wwCur, max(0, 
					pwwdCur->rcePage.yeBottom - pwwdCur->rcwDisp.ywBottom));
		return fTrue;
		}
	else
		{
		if ((*hmwdCur)->fSplit)
			{
			int wwLower = (*hmwdCur)->wwLower;
			int wkLower = PwwdWw(wwLower)->wk;
			struct WWD *pwwd;
			struct PLCEDL	**hplcedl;

			if (wk != wkLower)
				KillSplit(hmwdCur, fTrue /*fLower*/, fFalse /*fAdjustWidth*/);
			else  if (wkLower == wkHdr && wk == wkHdr)
				{
				RemoveWwDisp(PdrGalley(PwwdWw(wwLower))->doc, wwLower);
				AppendWw(doc, wwLower);

				FreezeHp();
				pwwd = PwwdWw(wwLower);
				PdrGalley(pwwd)->cpFirst = cp0;
				if ((hplcedl = PdrGalley(pwwd)->hplcedl) != 0)
					{
					MeltHp(); /* freeing doesn't move heap */
					FreeEdls(hplcedl, 0, IMacPlc(hplcedl));
					PutIMacPlc(hplcedl, 0);
					FreezeHp();
					}
				MeltHp();
				NewCurWw(wwLower, fFalse);
				pwwd = PwwdWw(wwLower);
				PdrGalley(pwwd)->doc = pwwd->sels.doc = selCur.doc = doc;
				TrashWw(wwLower); /* make sure old content get erased */
/* make sure iconbar contents are correct */
				SetupIconBarButton(wwLower, DocMother(doc),
						PwwdWw(WwOther(wwLower))->sels.cpFirst, PdodDoc(doc)->ihdt);
				}
			else
				NewCurWw(wwLower, fFalse);

			}
/* only let a real doc passed to FCreateSplit for creating a header pane */
		if (wk != wkHdr)
			doc = docNil;
#ifdef DEBUG
		else
			Assert(PdodDoc(doc)->fDispHdr);
#endif

		if (!(*hmwdCur)->fSplit)
			{
			if (FCanSplit(hmwdCur, wk))
				{
				pwwdCur = *hwwdCur;
				return (FCreateSplit(doc, pwwdCur->ywMac - DyOfRc(&pwwdCur->rcwDisp)/3, wk));
				}
			else
				{
				ErrorEid(eidWindowTooSmall,"FShowFtnAtn");
				return fFalse;
				}
			}
		}
	return fTrue;
}


/* C M D  I N S  F O O T N O T E */
/* %%Function:CmdInsFootnote %%Owner:chic */
CmdInsFootnote(pcmb)
CMB * pcmb;
{
	BOOL fRecordingSav;
	int doc = selCur.doc;
	BOOL fAuto;
	struct DOD *pdod = PdodDoc(doc);
	CP cpRef, cpRefNew = selCur.cpFirst;
	int tmc;
	int cmdRet;
	CHAR stRefDia[cchFtnRefMax + 1]; /* reference in dialog box */

/* Only allow insert footnote to mother doc or footnote pane */
	if (!pdod->fMother && !pdod->fFtn)
		{
		Beep();
		return cmdError;
		}

	if (FReportSplitErr(wkFtn))
		return cmdError;

/* Get the exact cpFirst of the document, i,e. in case of a block selection.
If in footnote pane, ignore selCur anyway since always insert at beginning
of footnote text.
*/
	if (selCur.fBlock && !pdod->fFtn)
		cpRefNew = CpFirstBlock();

	if (pcmb->fDefaults)
		{
/* get proposed values for footnote dialog box */
		stRefDia[0] = 0;
		fAuto = fTrue;
		pdod = PdodDoc(doc);

		if (!pdod->fShort && pdod->docFtn != docNil && !selCur.fIns)
			{
			CpFirstRef(doc, cpRefNew, &cpRef,edcDrpFtn);
			Assert(cpRef != cpNil);

			if (cpRefNew == cpRef)
				{
				int cchRef;
				cchRef = CchFetchFtnRef(doc, cpRefNew, &stRefDia[1]);
				if (stRefDia[1] != chFootnote)
					{
					stRefDia[0] = cchRef;
					fAuto = fFalse;
					}
				}
			}

		((CABINSERTFTN *) *pcmb->hcab)->fAuto = fAuto;
		FSetCabSt(pcmb->hcab, stRefDia, iagStRef);
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltInsertFtn)];

		BltDlt(dltInsertFtn, dlt);

		switch (tmc = TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcCancel:
			return cmdCancelled;

		case tmcError:
			return cmdNoMemory;

		case tmcSeparator:
		case tmcContSeparator:
		case tmcNoticeSeparator:
		case tmcOK:
			break;
			}
		}
	else
		tmc = pcmb->tmc;


	if (!pcmb->fAction)
		return cmdOK;

	/* Cheap hack to keep pane switching and window activation out
		of the recorded macro. */
	InhibitRecorder(&fRecordingSav, fTrue);
	StartLongOp();
	if (tmc != tmcOK)
		{ /* footnote separator */
		int iFtnSepSt;

		/* separator button pressed */
		Assert(tmc == tmcSeparator || tmc == tmcContSeparator ||
				tmc == tmcNoticeSeparator);
		iFtnSepSt = tmc - tmcSeparator;
		Assert(iFtnSepSt >= 0 && iFtnSepSt <= 2);
		if (FOpenHeaderIhtst(iFtnSepSt, fFalse /* fEditHdr */))
			cmdRet = cmdOK;
		else
			{
			cmdRet = cmdError;
			SetErrorMat(matMem);
			}
		}
	else
		{ /* footnote */
		/* get values from cab */
		GetCabSt(pcmb->hcab, stRefDia, cchFtnRefMax + 1, iagStRef);
		fAuto = (stRefDia[0] = CchStripString(&stRefDia[1], stRefDia[0])) == 0;
		cmdRet = CmdFootnote1(cpRefNew, fAuto, stRefDia);
		}

	EndLongOp(fFalse /*fAll*/);
	InhibitRecorder(&fRecordingSav, fFalse);

	return cmdRet;
}




/* %%Function:FDlgInsertFtn %%Owner:chic */
BOOL FDlgInsertFtn(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CHAR sz [256];

	if (fElActive && dlm == dlmInit)
		{
		EnableTmc(tmcSeparator, fFalse);
		EnableTmc(tmcContSeparator, fFalse);
		EnableTmc(tmcNoticeSeparator, fFalse);
		return fTrue;
		}

	if (dlm == dlmClick && tmc == tmcFAuto)
		{
		if (ValGetTmc(tmcFAuto))
			/* turn on Auto, blank ref mark string */
			SetTmcText(tmcRefMark, szEmpty);
		else
			{
			/* turn off Auto, insert ref mark string */
			SetTmcText(tmcRefMark, SzShared("*"));
			SetTmcTxs(tmcRefMark, TxsOfFirstLim(0, ichLimLast));
			}
		}
	else  if (dlm == dlmChange && tmc == tmcRefMark)
		{
		GetTmcText(tmcRefMark, sz, sizeof (sz));
		/* strip string */
		SetTmcVal(tmcFAuto, CchStripString(sz, CchSz(sz)-1 ) == 0);
		}

	return fTrue;
}





/* %%Function:CmdViewFootnote  %%Owner:chic */
CmdViewFootnote (pcmb)
CMB *pcmb;
{
	Assert(PdodDoc(selCur.doc)->fFtn || PdodDoc(selCur.doc)->fMother);
	Assert(hmwdCur);

	return CmdViewRef(wkFtn, edcDrpFtn);
}


/* %%Function:FReportSplitErr %%Owner:chic */
FReportSplitErr(wk)
int wk;
{
	if (!(*hmwdCur)->fSplit && !(*hwwdCur)->fPageView && !FCanSplit(hmwdCur, wk))
		{
		ErrorEid(eidWindowTooSmall, "");
		return fTrue;
		}
	return fFalse;
}


