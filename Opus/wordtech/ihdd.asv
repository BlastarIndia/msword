/* I H D D . C */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#ifdef WIN
#include "prompt.h"
#endif
#include "ch.h"
#include "props.h"
#include "doc.h"
#include "prm.h"
#include "format.h"
#include "file.h"
#include "heap.h"
#include "layout.h"
#include "preview.h"
#include "disp.h"
#include "sel.h"
#include "debug.h"
#include "print.h"
#include "field.h"



#ifdef PROTOTYPE
#include "ihdd.cpt"
#endif /* PROTOTYPE */

extern struct MERR      vmerr;
extern struct FLI       vfli;
extern struct PRSU      vprsu;
extern struct PREF      vpref;
extern int              vdocTemp;
extern int              wwCur;
extern int              vlm;
extern int              vssc;
extern PVS              vpvs;
extern struct DOD       **mpdochdod[];
extern struct CA	caPara;
extern struct PAP	vpapFetch;
extern struct ESPRM	dnsprm[];
extern int              vssc;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern struct WWD       **hwwdCur;
extern struct MERR	vmerr;
extern int              wwMac;
extern struct WWD       **mpwwhwwd[];
extern struct MWD       **hmwdCur;
#ifdef WIN
extern struct PRI       vpri;
extern HWND		vhwndPgPrvw;
extern uns              cbMemChunk;
#else
extern uns              *pfgrMac;
#endif


/**********************************/
/* C a  F r o m  I h d d  S p e c */
/*
Packs the doc and the pair of cp's at ihdd into the *pca.
doc passed is a main doc. ihdd may refer to Null or default footnote
haders.
docNil may be returned in the ca meaning Null.
Default footnotes are presented in a temp document that should be scanned
immediately, since the doc is shared with page numbers and pictures.
*/
/* %%Function:CaFromIhddSpec %%Owner:chic */
CaFromIhddSpec(doc, ihdd, pca)
int doc, ihdd;
struct CA *pca;
{
	CP *pcp;
	struct DOD *pdod;

	SetWords(pca, 0, cwCA);
/*      pca->doc = docNil;      */
	if (ihdd == ihddNil)
		return;
	if (ihdd == ihddTFtn)
		{
		if (FCreateChSpec(doc, chTFtn))
			goto LFtn;
		}
	else  if (ihdd == ihddTFtnCont)
		{
		if (FCreateChSpec(doc, chTFtnCont))
			{
LFtn:
			PcaSet( pca, vdocTemp, cp0, (CP)(ccpEop+1) );
			}
		}
	else  if ((doc = PdodDoc(doc)->docHdr) != docNil)
		CaFromIhdd(doc, ihdd, pca);
}


/* C A  F R O M  I H D D */
/* Packs the doc and the pair of cp's at ihdd into the *pca.
	Note that the extra Eop added by InsertIhdd is not considered part of the
	header
	doc passed is the docHdr subdoc
*/
/* %%Function:CaFromIhdd %%Owner:chic */
CaFromIhdd(doc, ihdd, pca)
int doc, ihdd;
struct CA *pca;
{
	struct PLC **hplchdd = PdodDoc(doc)->hplchdd;

	SetWords(pca, 0, cwCA);
	pca->doc = doc;
	Assert(doc != docNil);
	if (doc == docNil || hplchdd == hNil || ihdd == ihddNil)
		return;
	Assert(ihdd < IMacPlc(hplchdd));
	if (ihdd < IMacPlc(hplchdd))
		{
		pca->cpFirst = CpPlc(hplchdd, ihdd);
		pca->cpLim = CpPlc(hplchdd, ihdd + 1) - ccpEop;
		}
}


/* This stuff is here to avoid bringing Printsub in during save. */


/* char sprms that do NOT modify height */
csconst char rgsprmCharSafe[] = {
	sprmCFStrikeRM, sprmCFStrike, sprmCIco
};


#define csprmSafeChar   sizeof(rgsprmCharSafe)

/* para sprms that do NOT modify height */
csconst char rgsprmParaSafe[] = {
	sprmPJc, sprmPFSideBySide, sprmPFKeep, sprmPFKeepFollow,
			sprmPFPageBreakBefore, sprmPNfcSeqNumb, sprmPNoSeqNumb,
			sprmPFNoLineNumb
};


#define csprmSafePara   sizeof(rgsprmParaSafe)


/********************************/
/* F  G e t  V a l i d  P h e */
/* %%Function:FGetValidPhe %%Owner:chrism */
NATIVE int FGetValidPhe(doc, cp, pphe)
int doc;
CP cp;
struct PHE *pphe;
/* return fTrue if a valid phe is found and
		the valid phe, or a zero phe if none
	caPara should be set before calling - can't assert here because of 
	fields (caParaL in layout) 
*/
{
	struct DOD *pdod, *pdodMother;
	int fValidHeight;
	struct PLC **hplcphe;
	int iphe;
	int ipcd;
	struct PLC **hplcpcd;
	struct PHE phe;
	struct PCD pcd;

/* phe in plcphe, if present, always overrides file */
	SetWords(pphe, 0, cwPHE);
	pdod = PdodDoc(doc);
	Mac( Assert(FInCa(doc, cp, &caPara)) );
	if ((hplcphe = pdod->hplcphe) != hNil &&
			(iphe = IInPlcCheck(hplcphe, caPara.cpFirst)) >= 0 &&
			CpPlc(hplcphe, iphe) == caPara.cpFirst)
		{
		fValidHeight = fFalse;
		GetPlc(hplcphe, iphe, &phe);
		if (!phe.fUnk && phe.dxaCol != 0 && phe.dylHeight != 0)
			{
			fValidHeight = fTrue;
			*pphe = phe;
			}
		else
			{
			/* optimization: if we know height is bad, set it to
				good but clear phe; this prevents FOKSectHeights
				next time */
			fValidHeight = fFalse;
			if (phe.fUnk)
				{
				phe.fUnk = fFalse;
				PutPlc(hplcphe, iphe, pphe);
				}
			}
		return (fValidHeight);
		}

/* check for obvious things wrong:
	1. printer has changed
	2. stored height is zero */
	pdodMother = PdodMother(doc);
	if (pdodMother->fLRDirty || vpapFetch.phe.dylHeight == 0)
		return(fFalse);

/* para has a valid height; check that
	1. it exists on the mother doc's file (not just ANY file - we don't trust 
		heights stored on any other file)
	2. para has not been split by editing */
	hplcpcd = pdod->hplcpcd;
	GetPlc(hplcpcd, ipcd = IInPlc(hplcpcd, caPara.cpFirst), &pcd);
	if (pcd.fn <= fnScratch || pcd.fn != pdodMother->fn ||
			ipcd != IInPlc(hplcpcd, caPara.cpLim - 1))
		{
		return(fFalse);
		}

/* para is all in one piece; if the piece bounds are not the para bounds, we
	know that no editing has been done at the boundaries */

/* checking the para end is unecessary because
	any editing that creates a new para invalidates the new para's height.
	Start of para is different because you're just shortening it. */
	if (CpPlc(hplcpcd, ipcd) >= caPara.cpFirst)
		{
/* special cases for start of document: piece table singularities */
		if (caPara.cpFirst > cp0 || pcd.fc != (FC) cbFileHeader)
			return(fFalse);
		}

/* para has not been edited and we think we know its height; check for
	character or para sprms that would invalidate height */
LCheckSprms:
	return(FOKParaHeight(pcd.prm, pphe));
}


/***********************/
/* F   S a v e   P h e */
/* %%Function:FSavePhe %%Owner:chrism */
EXPORT FSavePhe(pphe)
struct PHE *pphe;
{
/* saves the phe for the paragraph described by caPara */
/* NOTE: always returns true, because who cares. Don't change mat.
	Can't change name because it's time to coddle Mac Word */
	int iphe, matSave = vmerr.mat;
	struct DOD **hdod = mpdochdod[caPara.doc];
	struct PLC **hplcphe = (*hdod)->hplcphe;

	Assert(!pphe->fUnk);
	if (vmerr.fSavePheFailed)
		return(fTrue);
	if (hplcphe == hNil)
		{
		if ((hplcphe = HplcInit(cbPHE, 10, (*hdod)->cpMac, fTrue /* ext rgFoo */)) == hNil)
			goto LNoSave;
		Win((*hplcphe)->fMult = fTrue);
		(*hdod)->hplcphe = hplcphe;
		(*hplcphe)->fGrowGt64K = fTrue;
		}
	iphe = IInPlcRef(hplcphe, caPara.cpFirst);
	if (iphe == -1)
		{
		/* higher than end of plc, shouldn't happen (set cpMac above) */
		Assert(fFalse);
		iphe = IMacPlc(hplcphe);
		}
	if (iphe < IMacPlc(hplcphe) && caPara.cpFirst == CpPlc(hplcphe, iphe))
		PutPlc(hplcphe, iphe, pphe);
#ifdef WIN
	else  if (CbOfHplc(hplcphe) >= cbMinBig-30)
		/* keep it from getting too big & taking low memory */
		goto LNoSave;
#endif
	else  if (FInsertInPlc(hplcphe, iphe, caPara.cpFirst, pphe))
		(*hdod)->fRepag |= vlm != lmBRepag;
	else
		{
LNoSave:
		vmerr.fSavePheFailed = fTrue;
		}
	vmerr.mat = matSave;	/* failure, if any, is not important */
	return fTrue;
}


/********************************/
/* F  O K  P a r a  H e i g h t */
/* %%Function:FOKParaHeight %%Owner:chrism */
NATIVE int FOKParaHeight(prm, pphe)
struct PRM prm;
struct PHE *pphe;
	/* return fFalse if a height-destructive sprm is found */
{
	int cch;
	char *pprl;
	struct ESPRM esprm;
	int cchSprm, isprm;

	*pphe = vpapFetch.phe;

	if (prm.prm == prmNil)
		return (fTrue);

	if (!prm.fComplex)
		{
		cch = 1;        /* will pick up 1 sprm, no matter its length */
		pprl = (struct PRL *) &prm;
		}
	else
		{
		struct PRC *pprc = *HprcFromPprmComplex(&prm);
		cch = pprc->bprlMac;
		pprl = pprc->grpprl;
		}

/* check for height-destructive sprms, as defined in the tables */
	while (cch > 0)
		{
		esprm = dnsprm[*pprl];
		if ((cchSprm = esprm.cch) == 0)
			cchSprm = *(pprl + 1) + 2;  /* variable-length type */
		if (esprm.sgc == sgcChp)
			{
			/* check sprm against list of non-height destructives */
			for (isprm = 0; isprm < csprmSafeChar; isprm++)
				if (*pprl == rgsprmCharSafe[isprm])
					break;
			if (isprm == csprmSafeChar)
				return(fFalse);
			}
		else  if (esprm.sgc == sgcPap)
			{
			/* check sprm against list of non-height destructives */
			for (isprm = 0; isprm < csprmSafePara; isprm++)
				if (*pprl == rgsprmParaSafe[isprm])
					break;
			if (isprm == csprmSafePara)
				return(fFalse);
			}
		cch -= cchSprm;
		pprl += cchSprm;
		}
	return(fTrue);
}


/* swaptune: moved from hdd.c to avoid bringing that in during save, quit, etc */

/*******************************/
/* F   C l e a n   U p   H d r */
/* %%Function:FCleanUpHdr %%Owner:chic */
int FCleanUpHdr(doc, fForce, fKillEmpty)
int doc;	/* main doc or any subdoc */
int fForce, fKillEmpty;
{
/* save any headers that are being displayed, dispose of any header display
	docs that are not being used */
	int docHdr, docDisp;
	int ww;
	struct DOD **hdod;
	struct WWD **hwwd;
	struct WWD *pwwd;

	doc = DocMother(doc);
	docHdr = PdodDoc(doc)->docHdr;
	if (docHdr == docNil)
		return(fTrue);
	hdod = mpdochdod[docHdr];
	while ((docDisp = (*hdod)->docHdr) != docNil)
		{
		ww = WwDisp(docDisp, wwNil, fTrue);
		if (fForce || (ww == wwNil && !PdodDoc(docDisp)->fDoNotDispose))
			{
			/* can't just check for PSelActive if we have
			selCur in a header and selDotted in main doc
			*/
			if (docDisp == selCur.doc ||
					(vssc != sscNil && docDisp == selDotted.doc))
				ForceSelOutOfHdr(docDisp, doc);
			(*hdod)->docHdr = PdodDoc(docDisp)->docHdr;
/* fix up any hwwd's sels before dispose it */
			for (ww = wwDocMin; ww < wwMac; ww++)
				{
				if ((hwwd = mpwwhwwd[ww]) != hNil &&
						(pwwd = *hwwd)->sels.doc == docDisp)
					{
					pwwd->sels.doc = doc;
					pwwd->sels.cpFirst = pwwd->sels.cpLim = cp0;
					}
				}
			if (vssc != sscNil && (docDisp == selCur.doc || docDisp == selDotted.doc))
				WinMac(CancelDyadic(), EndSsc(fFalse));
			TrashRuler();
			DisposeDoc(docDisp);
			}
		else  if (!FSaveHeader(docDisp, fKillEmpty))
			return(fFalse);
		else
			{
			if (docDisp == PselActive()->doc)
				{
				while (ww != wwCur && ww != wwNil)
					ww = WwDisp(docDisp, ww, fTrue);
				if (ww == wwNil)
					ForceSelOutOfHdr(docDisp, doc);
				}
			hdod = mpdochdod[docDisp];
			}
		}

	return(fTrue);
}


/*******************************************/
/* F o r c e   S e l   O u t   O f   H d r */
/* %%Function:ForceSelOutOfHdr %%Owner:chic */
ForceSelOutOfHdr(docDisp, doc)
int docDisp, doc;
{
/* if the header in docDisp is not being displayed in wwCur any more,
	force the selection to a safe CP in the main doc, namely cp0 */
	struct SEL *psel;

	if (wwCur != wwNil && (*hwwdCur)->fPageView && 
			(*hwwdCur)->mw != mwNil && doc == DocBaseWw(wwCur))
		{
		/* selection in hdr for page view */
LRestart:
		psel = PselActive();
		TurnOffSel(psel);
		psel->doc = doc;
		/* this changes the sel without UpdateWw,
			which can be fatal here */
		psel->sk = skIns;
		psel->fHidden = fTrue;
		SelectIns(psel, cp0);
		if (vssc != sscNil)
			{
			WinMac(CancelDyadic(), EndSsc(fFalse));
			if (docDisp == selCur.doc)
				/* now we have to do it again */
				goto LRestart;
			}
		}
}


/*****************************************/
/* W w   H d r   F r o m   D o c   M o m */
/* %%Function:WwHdrFromDocMom %%Owner:chic */
WwHdrFromDocMom(docMom, pdocHdrDisp)
int docMom;
int *pdocHdrDisp;
{
/* returns ww in which the next header is being displayed (may be header
	window/pane or page view); returns wwNil if end of headers is reached.
	returns display doc in *pdocHdrDisp. Start the search with
	*pdocHdrDisp = docNil */
	int doc = PdodDoc(docMom)->docHdr, ww;

	if (doc == docNil)
		return(wwNil);
	doc = PdodDoc((*pdocHdrDisp == docNil) ? doc : *pdocHdrDisp)->docHdr;
	for (; doc != docNil; doc = PdodDoc(doc)->docHdr)
		{
		if ((ww = WwDisp(doc, wwNil, fTrue)) != wwNil)
			{
			*pdocHdrDisp = doc;
			return(ww);
			}
		}
	return(wwNil);
}


#ifdef WIN

/*******************************************/
/* F  I n i t  W w  L b s  F o r  R e p a g */
/* %%Function:FInitWwLbsForRepag %%Owner:davidbo %%reviewed: 7/10/89 */
FInitWwLbsForRepag(ww, doc, lm, plbsText, plbsFtn)
int ww, doc, lm;
struct LBS *plbsText, *plbsFtn;
{
	int wwPagevw, flm, fPagevw, fSeeHidden, fShowResults;
	int docMother = DocMother(doc);
	GRPFVISI grpfvisi;
	struct WWD *pwwd;

	flm = lm == lmPrint ? flmPrint : flmRepaginate;
	if (plbsText != NULL)
		{
		Assert(plbsFtn != NULL);
		SetWords(plbsText, 0, cwLBS);
		SetWords(plbsFtn, 0, cwLBS);
		}

	SetFlm(flm);
	if (vmerr.fPrintEmerg)
		{
		if (lm == lmPagevw)
			{
			flm = flmDisplay;
			SetFlm(flm);
			vfli.dxuInch = vfli.dxsInch;
			vfli.dyuInch = vfli.dysInch;
			}
		else
			return fFalse;
		}

	InvalFli();
	vlm = lm;
#ifdef DAVIDBO
	CommSzNum(SzShared("FInit vlm: "), vlm);
#endif /* DAVIDBO */
	fPagevw = FPagevw(docMother, &wwPagevw);
	switch (lm)
		{
	default:
		Assert(fFalse);
		/* FALL THROUGH */
	case lmBRepag:
		Assert(docMother != docNil);
		ww = fPagevw ? wwPagevw : WwDisp(docMother, wwNil, fFalse);
LCommon:
		grpfvisi = (ww != wwNil) ? PwwdWw(ww)->grpfvisi : vpref.grpfvisi;
		fSeeHidden = grpfvisi.fSeeHidden || grpfvisi.fvisiShowAll;
		fShowResults = FFromIGrpf(fltgOther, grpfvisi.grpfShowResults) && !grpfvisi.fvisiShowAll;
		break;
	case lmPrint:
	case lmPreview:
		fSeeHidden = vprsu.fSeeHidden;
		fShowResults = vprsu.fShowResults;
		break;
	case lmPagevw:
		goto LCommon;
	case lmFRepag:
		if (fPagevw)
			ww = wwPagevw;
		goto LCommon;
		}

	if (docMother != docNil)
		LinkDocToWw(docMother, wwLayout, wwNil);
	pwwd = PwwdWw(wwLayout);
	pwwd->grpfvisi.w = 0;
	pwwd->grpfvisi.fSeeHidden = fSeeHidden;
	pwwd->grpfvisi.grpfShowResults = fShowResults ? ~0 : 0;
	pwwd->grpfvisi.fForceField = fTrue;
	pwwd->grpfvisi.flm = flm;
	pwwd->fOutline = fFalse;
	if (lm == lmPreview || (lm == lmPrint && vprsu.istPrSrc == istDocument))
		pwwd->fOutline = (hwwdCur != hNil && (*hwwdCur)->fOutline);

	if (plbsText != NULL)
		{
		Assert(plbsFtn != NULL);
		plbsText->ww = plbsFtn->ww = wwLayout;
		plbsText->fOutline = plbsFtn->fOutline = pwwd->fOutline;
		plbsText->doc = doc; /* REVIEW davidbo: docMother? */
		}

	/* if grpfvisi flags have changed since last repag, blow pgd/phe */
	if (docMother != docNil)
		CheckPagEnv(docMother, fSeeHidden, fShowResults);

	if (vpri.cInitNest == 0)
		vpri.wwInit = ww;
	vpri.cInitNest++;
#ifdef DEBUG
	if (vpri.cInitNest > 1)
		ReportSz("Nested call to FInitWwLbsForRepag");
#endif

	return fTrue;
}


/*  F  P A G E  V I E W  */
/* returns true if any ww displaying this doc is in pageview mode. */
/* %%Function:FPagevw %%Owner:davidbo %%reviewed: 7/10/89 */
FPagevw(docMother, pww)
int docMother, *pww;
{
	int ww = wwNil;

	*pww = ww;
	if (docMother == docNil)
		return fFalse;

	Assert(docMother == DocMother(docMother));
	while ((ww = WwDisp(docMother, ww, fFalse)) != wwNil)
		{
		if (PwwdWw(ww)->fPageView)
			{
			*pww = ww;
			return fTrue;
			}
		}
	return fFalse;
}


/* %%Function:CheckPagEnv %%Owner:davidbo */
CheckPagEnv(doc, fSeeHidden, fShowResults)
int doc;
int fSeeHidden, fShowResults;
{
	int docSub;
	struct DOD *pdod, *pdodSub;
	struct PLCPGD **hplcpgd;

	Assert (doc == DocMother(doc));
	pdod = PdodDoc(doc);
	if (pdod->dop.fPagHidden != fSeeHidden ||
			pdod->dop.fPagResults != fShowResults ||
			pdod->fEnvDirty)
		{
		if ((hplcpgd = pdod->hplcpgd) != hNil)
			SetPlcUnk(hplcpgd, cp0, pdod->cpMac);

		pdod =PdodDoc(doc);
		if ((docSub = pdod->docFtn) != docNil)
			{
			pdodSub = PdodDoc(docSub);
			if ((hplcpgd = pdodSub->hplcpgd) != hNil)
				SetPlcUnk(hplcpgd, cp0, pdodSub->cpMac);
			pdodSub = PdodDoc(docSub);
			pdodSub->fLRDirty = fTrue;
			FreePhplc(&pdodSub->hplcphe);
			}

		pdod = PdodDoc(doc);
		if ((docSub = pdod->docHdr) != docNil)
			{
			pdodSub = PdodDoc(docSub);
			pdodSub->fLRDirty = fTrue;
			FreePhplc(&pdodSub->hplcphe);
			}

		pdod = PdodDoc(doc);
		if (pdod->hplcphe != hNil)
			FreePhplc(&pdod->hplcphe);

		pdod = PdodDoc(doc);
		pdod->fLRDirty = pdod->fOutlineDirty = pdod->fRepag = fTrue;
		pdod->fEnvDirty = fFalse;
		pdod->dop.fPagHidden = fSeeHidden;
		pdod->dop.fPagResults = fShowResults;
		}
}



/* %%Function:EndFliLayout %%Owner:davidbo %%reviewed: 7/10/89 */
EndFliLayout(lm, flm)
int lm, flm;
{
	InvalFli();
	/* if still in preview, wwLayout must be set up correctly */
	if (lm == lmPreview)
		{
		/*
		* If preview mode killed while printing from preview,
		* which happens in low mem situations, do not set back
		* to preview mode!
		*/
		if (vhwndPgPrvw == NULL)
			{
			lm = vpvs.lmSave;
			flm = vpvs.flmSave;
			goto LSetFlm;
			}
		FResetWwLayout(vpvs.docPrvw, flm, lmPreview);
		}
	else
LSetFlm:
		SetFlm(flm);
	if (--vpri.cInitNest != 0)
		vpri.wwInit = wwNil;
	vlm = lm;
}


#endif /* WIN */


/* L I N K   D O C   T O   W W */
/* %%Function:LinkDocToWw %%Owner:davidbo %%reviewed: 7/10/89 */
EXPORT LinkDocToWw(doc, wwTo, wwFrom)
int doc, wwTo, wwFrom;
{
	struct DR *pdr = &(PwwdWw(wwTo)->rgdr[0]);

	Win(Assert(wwTo == wwTemp || wwTo == wwLayout));
	Mac(Assert(wwTo == wwTemp));

	pdr->doc = PmwdWw(wwTo)->doc = doc;
	PwwdWw(wwTo)->fPageView = (wwFrom != wwNil && wwFrom != wwTo) ? 
			PwwdWw(wwFrom)->fPageView : fFalse;
	Assert(PwwdWw(wwTo)->idrMac <= 1);
}


/* %%Function:FResetWwLayout %%Owner:davidbo */
EXPORT FResetWwLayout(doc, flm, lm)
int doc, flm, lm;
{
#ifdef DEBUG
	extern HDC hdcPrint;
#endif
	int fSeeHidden, fShowResults, fOutline;
	GRPFVISI grpfvisi;
	struct WWD *pwwd;

	SetFlm(flm);
	if (vmerr.fPrintEmerg)
		{
		if (lm != lmPagevw)
			return fFalse;
		SetFlm(flm = flmDisplay);
		vfli.dxuInch = vfli.dxsInch;
		vfli.dyuInch = vfli.dysInch;
		}

	vlm = lm;
	if (lm == lmPrint || lm == lmPreview)
		{
		Assert(doc == DocMotherLayout(doc));
		Assert(vpri.hdc == hdcPrint || lm == lmPreview);
		fSeeHidden = vprsu.fSeeHidden;
		fShowResults = vprsu.fShowResults;
		}
	else
		{
		if (vpri.wwInit != wwNil)
			grpfvisi = PwwdWw(vpri.wwInit)->grpfvisi;
		else
			grpfvisi = vpref.grpfvisi;
		fSeeHidden = grpfvisi.fSeeHidden || grpfvisi.fvisiShowAll;
		fShowResults = !grpfvisi.fvisiShowAll && FFromIGrpf(fltgOther, grpfvisi.grpfShowResults);
		}
	LinkDocToWw(doc, wwLayout, wwNil);
	pwwd = PwwdWw(wwLayout);
	pwwd->grpfvisi.w = 0;
	pwwd->grpfvisi.fSeeHidden = fSeeHidden;
	pwwd->grpfvisi.grpfShowResults = fShowResults ? ~0 : 0;
	pwwd->grpfvisi.fForceField = fTrue;
	pwwd->grpfvisi.flm = flm;
	pwwd->fOutline = fFalse;
	if (lm == lmPreview || (lm == lmPrint && vprsu.istPrSrc == istDocument))
		pwwd->fOutline = (hwwdCur != hNil && (*hwwdCur)->fOutline);
	CheckPagEnv(doc, fSeeHidden, fShowResults);
	return fTrue;
}


/* %%Function:DocMotherLayout %%Owner:davidbo */
EXPORT DocMotherLayout(doc)
int doc;
{
	int docMother = DocMother(doc);
	if (doc == docUndo || docMother == docUndo)
		{
		Assert(PdodDoc(docUndo)->doc != docNil);
		return (DocMother(PdodDoc(docUndo)->doc));
		}
	return docMother;
}


