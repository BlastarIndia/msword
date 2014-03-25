#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "ch.h"
#include "disp.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "debug.h"
#include "cmd.h"
#include "error.h"

#ifdef WIN
#include "message.h"
#endif /* WIN */


#ifdef PROTOTYPE
#include "footnote.cpt"
#endif /* PROTOTYPE */

extern char             rgchEop[];
extern struct MERR      vmerr;
extern struct DOD       **mpdochdod[];
extern struct SEL       selCur;
extern struct WWD       **hwwdCur;
extern int              wwCur;
extern int              vfSeeSel;
extern struct MWD       **hmwdCur;
extern int          	vdocScratch;
extern struct CA        caPara;
extern struct PREF	vpref;
extern struct UAB	vuab;

#define cchFtnRefMax    10      /* see also footnCmd.c */


/* G L O B A L S */
int                     vifnd; /* returned from CpFirstRef */

/* C M D  F O O T N O T E  1 */
/* %%Function:CmdFootnote1 %%Owner:chic */
CmdFootnote1(cpRefNew, fAuto, stRefDia)
CP cpRefNew; /* caller adjusted selCur.cpFirst to cpRefNew if it is block selection*/
BOOL fAuto; 
char *stRefDia;
{
	int doc = selCur.doc, docFtn;
	BOOL fFtn;
	struct DOD *pdod;
	struct PLC **hplcfrd;
	struct PLC **hplcfnd;
	int cchRefDoc, cchRefDiaSave, cchRefFtn;
	int cstcStd;
	int ifnd;
	BOOL fReplaceRef;
	CP cpRef, cpFtn, cpFtnNew, cpT;
	struct FRD frd;
	struct CHP chpText;
	struct CHP chpRef, chpSpace;
	struct PAP papText;
	struct CA caT1, caT2;
	char stRefDoc[cchFtnRefMax + 1]; /* reference in document */
	char stRefFtn[cchFtnRefMax + 1];

	if (fAuto)
		{
		Assert(stRefDia[0] == 0);
		stRefDia[0] = 1;
		stRefDia[1] = chFootnote;
		}
#ifdef DEBUG
	else
		Assert(stRefDia[0] > 0 && stRefDia[0] <= cchFtnRefMax);
#endif

/* we now have the desired reference in stRefDia.
Prepare chp desired for reference and pap and chp desired for footnote text.
*/
	if (fFtn = (pdod = PdodDoc(doc))->fFtn)
		{
		docFtn = doc;
		doc = pdod->doc;
		}
	TurnOffSel(&selCur);
/* failure is harmless, maps to stcNormal */
	if (!FStcDefined(doc, stcFtnRef))
		FEnsureStcDefined(doc, stcFtnRef, &cstcStd);
	if (!FStcDefined(doc, stcFtnText))
		FEnsureStcDefined(doc, stcFtnText, &cstcStd);
	pdod = PdodDoc(doc);
/* chp and pap are used temporarily by the next line for the reference */
	MapStc(pdod, stcFtnRef, &chpRef, &papText);
	chpRef.fSpec = stRefDia[1] == chFootnote;

	MapStc(pdod, stcFtnText, &chpText, &papText);
	papText.stc = stcFtnText;
	Win(chpText.fRMark = chpRef.fRMark = PdodMother(doc)->dop.fRevMarking);

/* special conditions */
	if (fFtn)
		{
/* in footnote document: insert ref in front of the footnote text */
		if (cpRefNew >= CpMacDocEdit(docFtn))
			{
			ErrorEid(eidInvalForFN,"CmdFootnote1");
			return cmdError;
			}
		else
			{
			hplcfnd = PdodDoc(docFtn)->hplcfnd;
			cpFtnNew = CpPlc(hplcfnd, IInPlc(hplcfnd, cpRefNew));
/* two lines because of compiler bug */
			PcaPoint(&caT1, docFtn, cpFtnNew);
			if (!FSetUndoB1(ucmFootnote, uccPaste, &caT1))
				return cmdCancelled;
			if (!FInsertRgch(docFtn, cpFtnNew,
					&stRefDia[1], stRefDia[0], &chpRef, 0))
				{
				return(cmdNoMemory);
				}
			SetUndoAfter(PcaSetDcp(&caT1, docFtn, cpFtnNew, (CP) stRefDia[0]));
			}
		return cmdOK;
		}

/* replace reference at selCur.cpFirst or insert reference there,
given stRefDia */
	if ((docFtn = DocSubEnsure(doc, edcDrpFtn)) == docNil)
		return cmdNoMemory;

	cpFtn = CpFirstRef(doc, cpRefNew, &cpRef, edcDrpFtn);

/* replace reference at cpRefNew or insert reference there given stRefDia */
	if (fReplaceRef = (cpRef == cpRefNew && !selCur.fIns))
		{
/* Replace existing reference which we fetch into stRefDoc */
		cchRefDoc = CchFetchFtnRef(doc,
				cpRefNew, &stRefDoc[1]);

/* Set undo to replace of ref mark */
		if (!FSetUndoB1(ucmFootnote, uccPaste,
				PcaSetDcp(&caT1, doc, cpRef, (CP)cchRefDoc)))
			return cmdCancelled;
/* acquire vdocScratch for use as a buffer later */
		if (DocCreateScratch(doc) == docNil)
			return cmdNoMemory;
		ReleaseDocScratch();
/* ref at cpRefNew, cchRefDoc long. Replace reference with different
reference pointing to the same footnote text.
We will insert new reference after old one, and create an empty new
footnote text in which a copy of the old text may be copied. This will
continue below after a common portion with the new reference case.
*/
		cpRefNew += (CP)cchRefDoc;
		stRefDoc[0] = cchRefDoc;
		if (!FNeSt(stRefDoc, stRefDia))
			goto LHaveCpRefNew;
		}
	else
		{
		if (!FSetUndoB1(ucmFootnote, uccInsert,
				PcaPoint(&caT1, doc, cp0)))
			return(cmdCancelled);
		}

/* insert footnote reference stRefDia at cpRefNew */
	if (!FInsertRgch(doc, cpRefNew, &stRefDia[1], stRefDia[0], &chpRef, 0))
		return cmdNoMemory;
/* correct for dead fields */
	Win(AssureNestedPropsCorrect(PcaSetDcp(&caT1,doc, cpRefNew, (CP)stRefDia[0]), fTrue));

/* for non-auto refs insert a space in with non-superscript chps */
	cchRefDiaSave = stRefDia[0]; /* used for repeated insertion below */
	if (stRefDia[1] != chFootnote)
		{
		GetSelCurChp(fFalse); /* get the correct chp to use to insert text */
		chpSpace = selCur.chp;
		if (fReplaceRef)
			{
			CachePara(doc, cpRefNew);
			GetPchpDocCpFIns(&chpSpace, doc, CpMin(cpRefNew+1, caPara.cpLim), fFalse, PmwdWw(wwCur)->wwUpper);
			}
		InsertFtnSpace(doc, cpRefNew, stRefDia, &chpSpace);
		}
	cpFtnNew = CpFirstRef(doc, cpRefNew, &cpT, edcDrpFtn);
	ifnd = vifnd;
/* we now have: vifnd, cpFtnNew of the next footnote text. Our footnote
should appear in front of this. (for replacing old ref, appears after)
Insert eop for a start following cpFtnNew.
*/
/* still OK to bail out, just leaves ftn ref char behind */
	if (!FInsertRgch(docFtn, cpFtnNew, rgchEop, (int)ccpEol, &chpText, &papText))
		return cmdNoMemory;

/* Update hplcfnd's and hplcfrd's in both documents */
	hplcfrd = PdodDoc(doc)->hplcfrd;
	hplcfnd = PdodDoc(docFtn)->hplcfnd;
	frd.fAuto = fAuto;
	if (!FStretchPlc(hplcfrd, 1) || !FStretchPlc(hplcfnd, 1))
		return cmdNoMemory;

/* we know these won't fail because of FStretchPlc calls above */
	FInsertInPlc(hplcfrd, ifnd, cpRefNew, &frd);
	FOpenPlc(hplcfnd, ifnd, 1);
	/* PutCpPlc(hplcfrd, ifnd, cpRefNew); */
	PutCpPlc(hplcfnd, ifnd, cpFtnNew);
	PutCpPlc(hplcfnd, ifnd + 1, CpPlc(hplcfnd, ifnd + 1) + ccpEol);

	if (fReplaceRef)
		{
		AssertDo(DocCreateScratch(doc) != docNil);
/* Continue to replace existing reference in stRefDoc.
Copy old footnote text (that starts at cpFtn) to new place.
vdocScratch (not docUndo, will mess up undo) is used as a temporary buffer since replace from doc to doc is not
permitted.
*/
/* bug fix: copy the chEop to preserve paragraph properties */
		SetWholeDoc(vdocScratch, PcaSet(&caT1, docFtn, cpFtn, cpFtnNew));
/* make a copy of the footnote text and insert in new position */
		if (vmerr.fMemFail ||
				!FReplaceCps(PcaSetDcp(&caT1, docFtn, cpFtnNew, ccpEop),
				PcaSetWholeDoc(&caT2, vdocScratch)))
			{
			ReleaseDocScratch();
			return cmdNoMemory;
			}
		ReleaseDocScratch();
/* Delete old ref mark & footnote */
		FDelete(PcaSetDcp(&caT1, doc, cpRef, (CP)cchRefDoc));
/* determine if reference in front of footnote matches stRefDoc */
		FetchRgch(&cchRefFtn, &stRefFtn[1], docFtn, cpFtn,
				CpLimSty(wwCur, docFtn, cpFtn, styWord, fFalse),
				cchFtnRefMax);
/* note: we are using the doc'd cch to check for identical initial segments */
		stRefFtn[0] = cchRefDoc;
		if (!FNeSt(stRefFtn, stRefDoc))
			if (!FDelete(PcaSetDcp(&caT2,docFtn,cpFtn,(CP)cchRefDoc)))
				goto LBackOut;
		if (!FInsertRgch(docFtn, cpFtn,
				&stRefDia[1], cchRefDiaSave, &chpRef, 0))
			{
			goto LBackOut;
			}
/* if replacing auto, insert space after the reference in front of footnote */
		if (stRefDia[1] != chFootnote)
			{
			stRefDia[0] = cchRefDiaSave;
			InsertFtnSpace(docFtn, cpFtn, stRefDia, &chpText);
			}
		Select(&selCur, cpRef, cpRefNew = cpRef + cchRefDiaSave);
		SetUndoAfter(PcaSetDcp(&caT1, doc, cpRef, (CP)stRefDia[0]));
		ifnd--; /* we inserted new after old, then deleted */
		}
	else
	/* a new footnote reference and a new footnote text is inserted in front
	of selCur.cpIns.
*/
		{
/* Insert reference mark in front of the footnote text */
		stRefDia[0] = cchRefDiaSave;
		if (!FInsertRgch(docFtn, cpFtnNew, &stRefDia[1], stRefDia[0], &chpRef, 0))
			{
LBackOut:
			FOpenPlc(hplcfrd, ifnd, -1);
			FOpenPlc(hplcfnd, ifnd, -1);
			return(cmdNoMemory);
			}
		if (stRefDia[1] != chFootnote)
			InsertFtnSpace(docFtn, cpFtnNew, stRefDia, &chpText);
/* select insertion point after the reference */
		cpRefNew += stRefDia[0];
		Select(&selCur, cpRefNew, cpRefNew);
		SetUndoAfter(PcaSetDcp(&caT1, doc, cpRefNew - (CP)stRefDia[0], (CP)stRefDia[0]));
		if ((*hwwdCur)->fPageView)
			{
			FtnPageView(docFtn, cpFtnNew+stRefDia[0], &chpText, &papText, cpRefNew);
			if ((*hwwdCur)->ipgd == ipgdNil)
				return cmdNoMemory;
			}
		}
/* invalidate other references which may be auto and hence may have to
be renumbered */
	if (ifnd+1 < IMacPlc(hplcfrd))
		{
		InvalCp(PcaSet(&caT1, doc, CpPlc(hplcfrd, ifnd + 1),
				CpMacDocEdit(doc)));
		InvalCp(PcaSet(&caT1, docFtn, CpPlc(hplcfnd, ifnd + 1),
				CpMacDocEdit(docFtn)));
		}

	PushInsSelCur();
LHaveCpRefNew:
	WinMac(FShowFtnAtn(doc, wkFtn), ShowFtnPane());
/* if footnote window exists, make it current and select the end of the
footnote text */
	if ((*hwwdCur)->wk == wkFtn)
		{
		CP cp;
/* note: wwCur = footnote pane */
/* select in front of terminating eop. Note cpRefNew points to after the
reference now. */
		Assert(PmwdWw(wwCur)->fSplit);
		cp = CpFirstRef(doc, cpRefNew, &cpT, edcDrpFtn) - ccpEol;
		SelectIns(&selCur, cp);
		}
	else  if ((*hwwdCur)->fPageView)
		{
		CP cp;

		TurnOffSel(&selCur);
		selCur.doc = docFtn;
		selCur.sk = skNil;
		cp = CpFirstRef(doc, cpRefNew, &cpT, edcDrpFtn) - ccpEol;
		SelectIns(&selCur, cp);
		}
	vfSeeSel = fTrue;
	return cmdOK;
}


/* I N S E R T  F T N  S P A C E */
/* inserts plain space following user defined footnote references. This is
to make it easier for the user to append plain text to the reference.
Local to CmdFootnote.
*/
/* %%Function:InsertFtnSpace %%Owner:chic */
InsertFtnSpace(doc, cp, stRef, pchp)
int doc; 
CP cp; 
char *stRef; 
struct CHP *pchp;
{
	char rgch[2];
	Assert (stRef[1] != chFootnote);
	rgch[0] = ' ';
	if (ChFetch(doc, cp + stRef[0], fcmChars) != ' ')
		{
		/* this is merely cosmetic, doesn't matter if it fails */
		if (FInsertRgch(doc, cp + stRef[0], rgch, 1, pchp, 0))
			stRef[0]++;
		}
}


#define ceopPgvwFtn     4

/* F T N  P A G E  V I E W */
/* Deals with going into a footnote in page view */
/* %%Function:FtnPageView %%Owner:chic */
FtnPageView(doc, cpFtn, pchp, ppap, cpRef)
int doc;
CP cpFtn, cpRef;
struct CHP *pchp;
struct PAP *ppap;
{
	int ceop, ipgd;
	CP cpCur = cpFtn;
	struct CA caT;

#ifdef BOGUS
	if (!vpref.fBkgrndPag)
		for (ceop = ceopPgvwFtn; ceop-- > 0; cpCur += ccpEop)
			{
			/** OK for this to fail, although it bodes ill for repag */
			if (!FInsertRgch(doc, cpCur, rgchEop, (int)ccpEop, pchp, ppap))
				break;
			}
#endif

	IpgdPldrFromDocCp(wwCur, DocBaseWw(wwCur), cpRef);

#ifdef BOGUS
	if (!vpref.fBkgrndPag)
		FDelete(PcaSet(&caT, doc, cpFtn, cpCur));
#endif

	/* so that undo ftn will redrawn pgview pane and erase footnote separator. */
	vuab.docInvalPageView = DocMother(doc);
}


/* C C H  F E T C H  F T N  R E F */
/* fetch the footnote ref text starting at cp into rgch. Length
of ref is returned.
Ref is defined by max length, not more than a word, not intruding into
next ref; auto ref is 1 long.
*/
/* %%Function:CchFetchFtnRef %%Owner:chic */
CchFetchFtnRef(doc, cpFirst, rgch)
int doc; 
CP cpFirst; 
char *rgch;
{
	int cchRef;
	CP cpLim;

	Assert(PdodDoc(doc)->docFtn != docNil);
/* do not let ref go beyond next ref in footnote table */
	CpFirstRef(doc, cpFirst + 1, &cpLim, edcDrpFtn);
	FetchRgch(&cchRef, rgch, doc, cpFirst,
			CpMin(selCur.cpLim, CpMin(cpLim, CpLimSty(wwCur, doc, cpFirst, styWord, fFalse))),
			cchFtnRefMax);
	return rgch[0] == chFootnote ? 1 : cchRef;
}


/* F  N O  B A C K  S P A C E  R E F */
/* returns true iff cp is 1st or 2nd char of footnote text */
/* %%Function:FNoBackSpaceRef %%Owner:chic */
FNoBackSpaceRef(hdod, cp)
struct DOD **hdod; 
CP cp;
{
	struct PLC **hplcfnd = (*hdod)->hplcfnd;
	int ifnd = IInPlc(hplcfnd, cp);
	return (cp <= (CpPlc(hplcfnd, ifnd) + 1));
}


/* F  C H E C K  L A R G E  E D I T  F T N */
/* footnote docs can't take large edits. return true iff can continue.
Must check pdod->fFtn||pdod->fAtn before call.
Returns "allow undo" flag in pf.
*/
/* %%Function:FCheckLargeEditFtn %%Owner:chic */
FCheckLargeEditFtn(pca, pfAllowUndo)
struct CA *pca; 
BOOL *pfAllowUndo;
{
	*pfAllowUndo = fTrue;
	if (!FCheckRefDoc(pca->doc, pca->cpFirst, pca->cpLim, fFalse))
		{
		*pfAllowUndo = fFalse;
#ifdef MAC
		return (FAlertYesNo(SzSharedKey("Can't Undo this operation. Continue without Undo?",
				CantUndoChangeAll)));
#else
		return (IdMessageBoxMstRgwMb(mstCantUndoOp, NULL,
				MB_ICONQUESTION|MB_YESNO) == IDYES);
#endif /* MAC */
		}
	return fTrue;
}


/* C P  L I M  F T N  C P */
/* Return cp lim of footnote containing given cp.
*/
/* %%Function:CpLimFtnCp %%Owner:chic */
EXPORT CP CpLimFtnCp(docSub, cp)
int docSub;
CP cp;
{
	struct DOD *pdodSub = PdodDoc(docSub);
	return (CpPlc(pdodSub->hplcfnd, IInPlcMult(pdodSub->hplcfnd, cp) + 1));
}


