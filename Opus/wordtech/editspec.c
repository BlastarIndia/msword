/* E D I T S P E C . C */
#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "ch.h"
#include "disp.h"
#include "file.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "prm.h"
#include "cmd.h"
#include "ruler.h"
#include "layout.h"
#include "message.h"
#include "format.h"
#include "scc.h"
#ifdef WIN
#include "dde.h"
#include "idle.h"
#include "field.h"
#include "status.h"
#define REVMARKING
#include "compare.h"
#define AUTOSAVE
#include "autosave.h"
#include "cmdtbl.h"
#endif
#include "table.h"
#include "debug.h"
#include "error.h"
#include "rareflag.h"

extern struct CA        caPara;
extern struct CA        caHdt;
extern struct PREF      vpref;
extern int              cHpFreeze;
extern int              vifnd;
extern struct SAB       vsab;
extern CHAR HUGE *      vhpchFetch;
extern struct CA        caSect;
extern struct CA        caPage;
extern struct CA        caTap;
extern struct CA        caTable;
extern struct FLS       vfls;
extern struct TCXS      vtcxs;
extern struct TCC       vtcc;
extern struct RULSS	vrulss;
extern struct TAP       vtapFetch;
extern struct PAP       vpapFetch;
extern struct PAP       vpapFetch;
extern int              fnFetch;
extern struct CHP       vchpFetch;
extern CP               vmpitccp[];
extern struct SCC       vsccAbove;
extern struct SCC       *vpsccBelow;
extern struct MERR      vmerr;
extern struct DOD       **mpdochdod[];
extern struct CA        caSect;
extern struct CA        caPara;
extern struct SEP       vsepFetch;
extern struct SPX       mpiwspxSep[];
extern int              wwCur;
extern struct SEL       selCur;
extern int              docMac;
extern struct UAB       vuab;
extern int              vitcMic;
extern struct SELS      rgselsInsert[];
extern struct FLS       vfls;
extern struct SEL       selCur;
extern struct WWD       **mpwwhwwd[];
extern struct DOD       **mpdochdod[];
extern CP               vcpFetch;
extern int              vdocFetch;
extern struct RC	rcScreen;
extern int              vdocTemp;
extern BOOL             vfAwfulNoise;
extern struct FLI       vfli;
extern struct AAB       vaab;
extern int              wwScrap;

#ifdef WIN
extern struct CA        caAdjust;
extern struct CA	caAdjustL;
extern int              vdocScratch;
extern struct PLDDLI     **vhplddliServer;
extern BOOL             vfDirtyLinks;
extern BOOL             vfInvalidDdli;
extern BOOL             vfDdeIdle;
extern struct LCB       vlcb;
extern ASD              asd;      /* AutoSave Descriptor. */
extern struct DDES      vddes;
extern BOOL             vfDdeIdle;
extern IDF		vidf;
extern int		docSeqCache;
extern CP		cpSeqCache;
extern int				vitcMicAux;
extern CP				vmpitccpAux[];
extern struct CA		caTapAux;
extern struct TAP		vtapFetchAux;
#ifdef DEBUG
extern int              cCaAdjust;
extern int		cCaAdjustL;
#endif /* DEBUG */
extern HWND				vhwndCBT;
#endif /* WIN */


/* I N V A L  A U T O  R E F E R E N C E S */
/* invalidate other references which may be auto and hence may have to
be renumbered */
/*  %%Function:InvalAutoReferences %%Owner:peterj %%reviewed: 6/28/89 */
InvalAutoReferences(doc, iRef, edcDrp)
int doc, iRef, edcDrp;
{
	struct DRP *pdrp;
	int docSub;
	struct CA caT;
	struct PLC **hplcfnd;

	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	docSub = pdrp->doc;
	Assert(!PdodDoc(doc)->fShort);

	if (iRef < IMacPlc(pdrp->hplcRef))
		InvalCp(PcaSet(&caT, doc, CpPlc(pdrp->hplcRef, iRef), CpMac2Doc(doc)));
	hplcfnd = PdodDoc(docSub)->hplcfnd;
	AssertH(hplcfnd);
	if (iRef < IMacPlc(hplcfnd))
		InvalCp(PcaSet(&caT, docSub, CpPlc(hplcfnd, iRef), CpMac2Doc(docSub)));
}


/* C O R R E C T  D O D  P L C S */
/*  %%Function:CorrectDodPlcs %%Owner:davidlu */
EXPORT CorrectDodPlcs(doc, cpMacCorrect)
int doc;
CP cpMacCorrect;
{
	struct DOD *pdod = PdodDoc(doc);
	/* note that hplcglsy shares a slot in the DOD with
		hplcfnd and hplchdd. We avoid trouble beacuse in the
		main doc it is only possible to have hplcglsy filled in */
	CorrectPlcCpMac(pdod->hplcglsy, cpMacCorrect);
#ifdef WIN
	CorrectPlcCpMac(pdod->hplcfld, cpMacCorrect);
#endif /* WIN */
	CorrectPlcCpMac(pdod->hplcpgd, cpMacCorrect);
	CorrectPlcCpMac(pdod->hplcphe, cpMacCorrect);
	if (!pdod->fShort)
		{
		CorrectPlcCpMac(pdod->hplcfrd, cpMacCorrect);
/* only the hplcsed may describe the hidden section mark at the end of the
	doc. */
		CorrectPlcCpMac(pdod->hplcsed, cpMacCorrect);
		if (pdod->fSea)
			CorrectPlcCpMac(pdod->hplcsea, cpMacCorrect);
		CorrectPlcCpMac(pdod->hplcpad, cpMacCorrect);
#ifdef WIN
		CorrectPlcCpMac(pdod->hplcatrd, cpMacCorrect);
		CorrectPlcCpMac(pdod->hplcsea, cpMacCorrect);
		/* these can have an entry at CpMacDoc */
		CorrectPlcCpMac(pdod->hplcbkf, cpMacCorrect);
		CorrectPlcCpMac(pdod->hplcbkl, cpMacCorrect);
#endif /* WIN */
		}
}


/* C O R R E C T  P L C  C P  M A C */
/*  %%Function:CorrectPlcCpMac %%Owner:davidlu */
CorrectPlcCpMac(hplc, cpMacCorrect)
struct PLC **hplc;
CP cpMacCorrect;
{
	if (hplc != hNil)
		{
		CompletelyAdjustHplcCps(hplc);
		PutCpPlc(hplc, IMacPlc(hplc), cpMacCorrect);
		}
}




#ifdef WIN

/* I N V A L  D D E */
/*  Marks dirty any server links which are active in the range
	cpFirst-cpLim.  Does not actually send the data.
*/
/*  %%Function:InvalDde %%Owner:peterj %%reviewed: 6/28/89 */
InvalDde (pca)
struct CA *pca;

{
	int iddli, iddliMac;
	struct DDLI ddli;

	if (vddes.hplddli == hNil || !PdodDoc(pca->doc)->fMother ||
			PdodDoc (pca->doc)->hplcbkf == hNil)
		return;

	iddliMac = IMacPl(vddes.hplddli);

	for (iddli = 0; iddli < iddliMac; iddli++)
		{
		GetPl( vddes.hplddli, iddli, &ddli );
		if (!ddli.fDirty && ddli.ibkf != ibkmkNil &&
				PdcldDcl (ddli.dcl)->doc == pca->doc)
			/*	this link is not already dirty, is not invalidated and
			refers to something in this doc */
			{
			CP cpFirstBkmk, cpLimBkmk;
			BkmkCpsFromIbkf (pca->doc, ddli.ibkf, &cpFirstBkmk, &cpLimBkmk);
			if (pca->cpLim > cpFirstBkmk && pca->cpFirst < cpLimBkmk)
				/* this bookmark must be dirtied */
				{
				ddli.fDirty = fTrue;
				vddes.fDirtyLinks = fTrue;
				PutPl( vddes.hplddli, iddli, &ddli );
				}
			}
		}

	if (vddes.fDirtyLinks)
		vfDdeIdle = fTrue;

}


/* I N V A L  F L D */
/*  Invalidation of fields.  Invalidate the type of any field whose
	instructions are invalidated.  Mark the results edited/dirty for fields
	whose result portions are invalidated.
*/
/*  %%Function:InvalFld %%Owner:peterj %%reviewed: 6/28/89 */
InvalFld (pca, fEdit)
struct CA *pca;
BOOL fEdit;

{
	int ifld = ifldNil;
	int ifldParse = IfldFromDocCp (pca->doc, pca->cpFirst, fFalse);
	CP cpT;
	struct FLCD flcd;
	extern BYTE vfNoInvalField;

	if (ifldParse != ifldNil && !vfNoInvalField)
		{
		GetIfldFlcd (pca->doc, ifldParse, &flcd);
		if (pca->cpFirst > flcd.cpFirst && 
				pca->cpFirst < flcd.cpFirst + flcd.dcpInst)
			{  /* edit in instruction text of ifldParse */
			flcd.fDirty = fTrue;
			flcd.bData = bDataInval;
			SetFlcdCh (pca->doc, &flcd, 0);
			if (vddes.cDclClient > 0)
				/* may have changed dde link info, check later */
				vfDdeIdle = fTrue;
			}
		}
	else  if (!FFieldsInPca(pca))
		return;

		{{ /* NATIVE - InvalFld */
		while ((ifld = IfldAfterFlcd (pca->doc, ifld, &flcd)) != ifldNil)
			{
			if (pca->cpLim > (cpT = flcd.cpFirst + flcd.dcpInst)
					&& pca->cpFirst < cpT + flcd.dcpResult)
				{
				flcd.fResultDirty = fTrue;
				if (fEdit)
					flcd.fResultEdited = fTrue;
				SetFlcdCh (pca->doc, &flcd, chFieldEnd);
				}
			if (flcd.bData != bDataInval && ifld != ifldParse
					&& pca->cpLim > flcd.cpFirst &&
					pca->cpFirst < flcd.cpFirst + flcd.dcpInst)
				{
				flcd.bData = bDataInval;
				SetFlcdCh (pca->doc, &flcd, chFieldSeparate);
				if (vddes.cDclClient > 0)
					/* may have changed dde link info, check later */
					vfDdeIdle = fTrue;
				}
			}
		}}
}



/* I N V A L  T E X T */
/*  Advises those who want to know when text or formatting change.  Not
	a subsitute for InvalCp() which should be called too.
	checks in interval [pca->cpFirst,pca->cpLim + 1)
*/
/*  %%Function:InvalText %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT InvalText (pca, fEdit)
struct CA *pca;
BOOL fEdit;

{
	struct CA caT;

	PcaSet(&caT, pca->doc, pca->cpFirst, pca->cpLim+1);

	InvalDde (&caT);

	if (PdodDoc (pca->doc)->hplcfld != hNil)
		InvalFld (&caT, fEdit);

}


#endif /* WIN */

/* A D J U S T  I P G D S */
/* some cpgd pgd's following ipgdFirst are deleted (fDel) or inserted (!fDel).
Displays following ipgdFirst are invalidated and ipgd's are adjusted
to point to the page containing the "same" text assuming that text propagation
will be arrested by some fixpoint in between - a reasonable approximation.
*/
/*  %%Function:AdjustIpgds %%Owner:peterj %%reviewed: 6/28/89 */
AdjustIpgds(doc, ipgdFirst, cpgd, fDel)
{
	int ww;
	if (cpgd == 0 || PdodDoc(doc)->fShort/* to avoid dbl adjust */)
		return;
	for (ww = WwDisp(doc, wwNil, fFalse); ww != wwNil; ww = WwDisp(doc, ww, fFalse))
		{
		struct WWD *pwwd = PwwdWw(ww);
		int ipgd;

		if (pwwd->fPageView && (ipgd = pwwd->ipgd) >= ipgdFirst)
			{
			if (fDel)
				pwwd->ipgd = max(ipgdFirst, ipgd - cpgd);
			else  if (ipgd > ipgdFirst)
				pwwd->ipgd = ipgd + cpgd;
			pwwd->fDrDirty = fTrue;
			pwwd->fDirty = fTrue;
			}
		}
}


#ifdef MAC	/* Replaced by all-in-one function for vmerr.mat in Opus */
/* F  R E P L A C E  F A I L */
/* clears fReplaceFail bit by giving error msg. Returns old value */
/*  %%Function:FReplaceFail %%Owner:Notused  */
FReplaceFail()
{
	int mat;

	if (vmerr.mat == matReplace && !vmerr.fHadMemAlert)
		{
		ErrorEid(eidTooManyEdits,"ReplaceCpsRM");
		Debug(mat = vmerr.mat);
		Debug(vmerr.mat = matNil);
		SetUndoNil();
		Debug(vmerr.mat = mat);
		vmerr.fHadMemAlert = fTrue;
		return fTrue;
		}
	return fFalse;
}


#endif /* MAC */

#ifdef DEBUGORNOTWIN
/* A D J U S T  C P */
/* Adjust all cp's > cpFirst by dcpIns - dcpDel.
Invalidation must have been done prior to this.
Piece tables are adjusted elsewhere.
*/
/*  %%Function:AdjustCp %%Owner:peterj %%reviewed: 6/28/89 */
#ifdef MAC
NATIVE AdjustCp(pca, dcpIns) /* WINIGNORE - MAC only */
#else /* WIN */
HANDNATIVE C_AdjustCp(pca, dcpIns)
#endif /* WIN */
struct CA *pca;
CP dcpIns;
{
	int ww;
	int doc = pca->doc;
	struct WWD *pwwd;
	struct DOD *pdod, *pdodMother;
	int fFixMacEntry;
	int isels;
	int idr, idrMac;
	struct DR *pdr;
	struct SELS *pselsT;
	struct PLC **hplc;
	CP dcpAdj = dcpIns - DcpCa(pca);
	CP cp1, cpFirstDel, cpLimDel, cpMac, cpT;
	extern struct PL **vhplbmc;

	Break3();
	Scribble(ispAdjustCp,'A');

	if (dcpAdj == 0)
		return;

	Assert(cHpFreeze==0);
	FreezeHp();

	pdod = PdodDoc(doc);

	fFixMacEntry = (pca->cpFirst == CpMac2Doc(doc));

	cpMac = (pdod->cpMac += dcpAdj);

	if (cpMac >= cpWarnTooBig || cpMac < 0)
		{
		vmerr.fWarnDocTooBig = fTrue;
		if (cpMac < 0 || cpMac >= cpMax)
			/* this alert will probably be our last act... */
			ErrorEid(eidCpRollOver, "AdjustCp");
		}

	cp1 = pca->cpFirst + 1;
	
	/* note: this corrects hplchdd, hplcfnd, hplcglsy which all reside at
		the same dod location */
	AdjustHplc(pdod->hplchdd, cp1, dcpAdj, -1);
	AdjustHplc(hplc = pdod->hplcphe, cp1, dcpAdj, -1);
	if (hplc && (CpPlc(hplc, 0) < cp0))
		PutCpPlc(hplc, 0, cp0);

	AdjustHplc(pdod->hplcpgd, cp1, dcpAdj, -1);
	Win(AdjustHplc(pdod->hplcfld, pca->cpFirst, dcpAdj, -1));

	if (!pdod->fShort)
		{
#ifdef WIN
		AdjustHplc(pdod->hplcbkf, cp1, dcpAdj, -1);
		AdjustHplc(pdod->hplcbkl, cp1, dcpAdj, -1);
		AdjustHplc(pdod->hplcatrd, pca->cpFirst/*sic*/, dcpAdj, -1);
#endif
		AdjustHplc(pdod->hplcfrd, pca->cpFirst/*sic*/, dcpAdj, -1);
		AdjustHplc(pdod->hplcsed, cp1, dcpAdj, -1);
		if (pdod->fSea)
			AdjustHplc(pdod->hplcsea, cp1, dcpAdj, -1);
		AdjustHplc(pdod->hplcpad, cp1, dcpAdj, -1);
		}

	/* if we were inserting at end of document, the CP for iMac will still
		be set to cpFirst for those PLCs for which no entries were added.
		We force the iMac CP for each PLC to be the newly adjusted
		CpMacDoc() value. */
	if (fFixMacEntry)
		CorrectDodPlcs(doc, CpMac2Doc(doc));

	/* no need to adjust caRulerSprm, it should have been flushed */
	Assert( doc == docUndo || vrulss.caRulerSprm.doc == docNil );

	cpFirstDel = pca->cpFirst; 
	cpLimDel = pca->cpLim;
	AdjustCa(&caPara, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&caPage, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&caSect, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&caHdt, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&vfls.ca, doc, cpFirstDel, cpLimDel, dcpAdj);

	AdjustCa(&caTable, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&caTap, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&vtcc.ca, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&vtcc.caTap, doc, cpFirstDel, cpLimDel, dcpAdj);
#ifdef MAC
	AdjustCa(&vtcc.caTapPrev, doc, cpFirstDel, cpLimDel, dcpAdj);
#else /* WIN */
	AdjustCa(&caTapAux, doc, cpFirstDel, cpLimDel, dcpAdj);
#endif /* WIN */
	AdjustCa(&vtcxs.ca, doc, cpFirstDel, cpLimDel, dcpAdj);

#ifdef WIN
	AdjustCa(&vlcb.ca, doc, cpFirstDel, cpLimDel, dcpAdj);
	AdjustCa(&vsab.caCopy, doc, cpFirstDel, cpLimDel, dcpAdj);

	Assert(caAdjust.doc == docNil || cCaAdjust == 1);
	AdjustCa(&caAdjust, doc, cpFirstDel, cpLimDel, dcpAdj);
	Assert(caAdjustL.doc == docNil || cCaAdjustL == 1);
	AdjustCa(&caAdjustL, doc, cpFirstDel, cpLimDel, dcpAdj);

#endif
	
	if (vhplbmc != hNil)
		{
		int ibmc;
		struct BMC *pbmc;

		FreezeHp();
		for (ibmc = (*vhplbmc)->iMac - 1, pbmc = PInPl(vhplbmc,ibmc);
			ibmc >= 0; ibmc--, pbmc--)
			{
			AdjustCa(&pbmc->ca, doc, cpFirstDel, cpLimDel, dcpAdj);
			}
		MeltHp();
		}

		/* BLOCK - Adjust vmpitccp used to cache known cell boundaries for 
				the current table row. */
		{
		CP *pcp, *pcpMac;
		int ccp;

		if ( caTap.doc==doc && caTap.cpLim >= cp1 &&
				(ccp=vtapFetch.itcMac+1+1-vitcMic) > 0 )
			/* plus 1 for ttp, 1 for fencepost rule */
			{
			pcpMac = (pcp = &vmpitccp[vitcMic]) + ccp;
			while ( *(pcp++) < cp1 )
				;
			--pcp;
			while ( pcp < pcpMac )
				*(pcp++) += dcpAdj;
			}
#ifdef WIN
		if ( caTapAux.doc==doc && caTapAux.cpLim >= cp1 &&
				(ccp=vtapFetchAux.itcMac+1+1-vitcMicAux) > 0 )
			/* plus 1 for ttp, 1 for fencepost rule */
			{
			pcpMac = (pcp = &vmpitccpAux[vitcMicAux]) + ccp;
			while ( *(pcp++) < cp1 )
				;
			--pcp;
			while ( pcp < pcpMac )
				*(pcp++) += dcpAdj;
			}
#endif /* WIN */
		}

	for (isels = 0, pselsT = &rgselsInsert[0]; isels < iselsInsertMax; isels++, pselsT++)
		{
		if (pselsT->doc == doc)
			{
			AdjustSels(pselsT, pca->cpFirst, pca->cpLim, dcpAdj);
			if (pselsT->sk == skBlock)
				pselsT->sk = skSel;	/* prevent illegal block sel */
			}
		}

	FreezeHp();
	/* adjust all dr's in all wwd's displaying doc */
	pdodMother = PdodMother(doc);
	for (ww = pdodMother->wwDisp; ww != wwNil; ww = pwwd->wwDisp)
		{
		pwwd = PwwdWw(ww);
		idrMac = pwwd->idrMac;
		for (pdr = pwwd->rgdr, idr = 0; idr < idrMac; ++idr, ++pdr)
			{
			if (pdr->doc != doc)
				continue;
			/* check dr.cpFirst */
			if (pdr->cpFirst >= cp1)
				{
				if (pdr->cpFirst < pca->cpLim)
					{
					pdr->cpFirst = pca->cpFirst;
					pdr->fCpBad = fTrue;
					pwwd->fSetElev = fTrue;
					}
				else
					pdr->cpFirst += dcpAdj;
				}
			/* check dr.cpLim */
			if (pdr->cpLim != cpNil && pdr->cpLim >= cp1)
				{
				if (pdr->cpLim < pca->cpLim && pdr->idrFlow != idrNil)
					{
					pdr->cpLim = cpNil;
					}
				else
					{
					/* adjust dr.cpLim; it is now suspect */
					pdr->cpLim += dcpAdj;
					pdr->fLimSuspect = pdr->idrFlow != idrNil;
					}
				}
			if (pdr->hplcedl != hNil)
				AdjustHplcedlCps(pdr->hplcedl, cp1, dcpAdj);
			} /* next dr */

		if (doc == pwwd->sels.doc)
			{
			AdjustSels(&pwwd->sels, pca->cpFirst, pca->cpLim, dcpAdj);
			if (pwwd->sels.sk == skBlock)
				pwwd->sels.sk = skSel;	/* prevent illegal block sel */
			}
		} /* next ww */
	MeltHp();

	if (doc == selCur.doc)
		{
		selCur.cpFirstLine = cpNil;     /* no anchor for block sel */
		AdjustSels(&selCur, pca->cpFirst, pca->cpLim, dcpAdj);
		}
#ifdef MAC
	if (vpsccBelow && doc == vpsccBelow->doc)
		AdjustHplcedlCps(vpsccBelow->hplcedl, cp1, dcpAdj);
#endif
	if (doc == vsccAbove.doc)
		AdjustHplcedlCps(vsccAbove.hplcedl, cp1, dcpAdj);
/* invalidate sequential fetch */
	if (doc == vdocFetch)
		vdocFetch = docNil;
#ifdef WIN
#ifdef SHOWCPMAC
	CommSzLong(SzShared("AdjustCp: cpMac = "), cpMac);
#endif /* SHOWCPMAC */
#endif /* WIN */

	MeltHp();

	Scribble(ispAdjustCp,' ');
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* A D J U S T  S E L S */
/* adjust selection
*/
/*  %%Function:AdjustSels %%Owner:peterj %%reviewed: 6/28/89 */
AdjustSels(psels, cpFirst, cpLim, dcpAdj)
struct SELS *psels;
CP cpFirst, cpLim, dcpAdj;
{
	Assert(psels->doc != docNil);
	if (psels->cpFirst >= cpFirst && psels->cpFirst < cpLim ||
			psels->fTable && psels->cpLim > cpFirst && psels->cpFirst <= cpLim)
/* part of selection has been deleted */
		{
		psels->cpFirst = cpFirst;
		psels->cpLim = cpFirst;
		psels->cpAnchor = cpFirst;
#ifdef WIN
		psels->cpAnchorShrink = cpFirst;
#endif
		/*psels->sk = skIns; done below */
		}
	else
		{
		if (psels->cpLim >= cpFirst && psels->cpLim < cpLim)
			dcpAdj = cpFirst  - psels->cpLim; 	
		AdjustCa(&psels->ca, psels->doc, cpFirst, cpLim, dcpAdj);
		if (psels->cpAnchor >= cpFirst + 1)
			psels->cpAnchor += dcpAdj;
#ifdef WIN
		if (psels->cpAnchorShrink >= cpFirst + 1)
			psels->cpAnchorShrink += dcpAdj;
#endif
		}

	/*	"Correct" behavior is not well-defined here. We have edited
	/*	characters within the selection. To be safe, we turn the
	/*	selection into and insertion point so as to not cause problems
	/*	elsewhere (particulary with table and block selections).
	/**/
	if (psels->sk != skIns && psels->cpFirst == psels->cpLim)
		{
		psels->sk = skIns;
/* selCur at Mac can happen temporarily because of deletions at the end
of the doc */
		if (psels->cpFirst < CpMacDoc(psels->doc))
			SetSelCellBits(psels);
		if (psels == &selCur)
			GetSelCurChp(fTrue);
		}
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* A D J U S T  C A */
/* adjust all cp's >= cpFirst by dcp
*/
/*  %%Function:AdjustCa %%Owner:peterj %%reviewed: 6/28/89 */
#ifdef	BOGUS
AdjustCa(pca, doc, cpFirst, dcp)
struct CA *pca;
int doc;
CP cpFirst, dcp;
{
	if (pca->doc == doc)
		{
		if (pca->cpFirst >= cpFirst) pca->cpFirst += dcp;
		if (pca->cpLim >= cpFirst) pca->cpLim += dcp;
		}
}
#endif

/*	We have just replaced [cpFirstDel,cpLimDel) with new text
/* which has "dcp" more characters than the original. (dcp will
/* be negative if the replacement was shorter.) We now have some
/* CA which may have to be adjusted. If the deleted range and the
/* CA range overlap, then we must leave CA pointing to "safe" CPs.
/* There are six ways in which the deleted and CA ranges may
/* intersect.
/*
/*        cp0    [ inserted )           .         -> cpMac
/*               .          | <- dcp -> |
/*               [ deleted              )
/*   Case:       .                      .                  Action:
/*     1   [)    .                      .          Do nothing
/*     2   [     .)                     .          cpLim = cpFirstDel
/*     3   [     .                      )          cpLim += dcp
/*     4         [)                     .          set both CPs to cpFirstDel
/*     5         [                      )          cpFirst = cpFirstDel
/*     6         .                      [      )   adjust both CPs by dcp
/*
/*   Each "case" is represented by the left-most element of the class.
/*   That is, each '[' can be moved to the right until it hits the ')'.
/*   If the '[' in the next case begins in the same position, then the ')'
/*   in the current case can advance to just in front of the ')' in the
/*   next case; otherwise it can advance arbitrarily.
/*
/**/
AdjustCa(pca, doc, cpFirstDel, cpLimDel, dcp)
struct CA *pca;
int doc;
CP cpFirstDel, cpLimDel, dcp; 
{
	Assert (pca->cpFirst <= pca->cpLim);
	Assert((cpLimDel - cpFirstDel) + dcp >= cp0);
	
	if (pca->doc == doc)
		{
		if (pca->cpFirst > cpFirstDel)
			{
			if (pca->cpFirst < cpLimDel)
				pca->cpFirst = cpFirstDel;
			else
				pca->cpFirst += dcp;
			}
		if (pca->cpLim > cpFirstDel)
			{
			if (pca->cpLim < cpLimDel)
				pca->cpLim = cpFirstDel;
			else
				pca->cpLim += dcp;
			}
		}
}
#endif /* DEBUGORNOTWIN */

/* I N V A L  C A */
/* invalidate ca's that are affected by the edit
*/
/* %%Function:InvalCa %%Owner:peterj %%reviewed: 6/28/89 */
#ifdef MAC
NATIVE /* WINIGNORE - MAC only */
#endif /* MAC */
InvalCa(pca, doc, cpFirst, dcp)
struct CA *pca; 
int doc; 
CP cpFirst, dcp;
{
	if (pca->doc == doc)
		{
		if (pca->cpLim > cpFirst && pca->cpFirst < cpFirst + dcp)
			pca->doc = docNil;
		}
}


#ifdef DEBUGORNOTWIN /* in editn.asm if WIN */
/* I n v a l   C a   F i e r c e */
/*  %%Function:InvalCaFierce %%Owner:peterj %%reviewed: 6/28/89 */
#ifdef MAC
NATIVE InvalCaFierce() /* WINIGNORE - MAC only */
#else /* WIN */
HANDNATIVE C_InvalCaFierce()
#endif /* WIN */
{
/* unconditionally invalidate CAs and other important docs */
	InvalLlc();
	vdocFetch = docNil;
	caSect.doc = docNil;
	caPara.doc = docNil;
	caPage.doc = docNil;
	caTable.doc = docNil;
	caTap.doc = docNil;
	caHdt.doc = docNil;
	vtcc.ca.doc = docNil;
	vtcc.caTap.doc = docNil;
#ifdef MAC
	vtcc.caTapPrev.doc = docNil;
#else /* WIN */
	caTapAux.doc = docNil;
#endif /* WIN */
	vtcxs.ca.doc = docNil;
	Win( vlcb.ca.doc = docNil );
	Win( InvalVisiCache() );
	Win( InvalCellCache() );
	Win( docSeqCache = docNil );
}


#endif /* DEBUGORNOTWIN */

/* invalidate the caches which depend on table properties */
#ifdef MAC
NATIVE /* WINIGNORE - MAC only */
#endif /* MAC */
/*  %%Function:InvalTableProps %%Owner:peterj %%reviewed: 6/28/89  */
InvalTableProps(doc,cpFirst,dcp,fFierce)
int doc;
CP cpFirst, dcp;
BOOL fFierce;
{
	int itc;
	CP cpLim;

	/* If the invalidation area lies strictly within a cell of the
	/* cached row, then we don't have to invalidate the caTap/vtapFetch,
	/* or anything which depends on it. (Things that don't depend on it
	/* get missed in cp space, so we don't have to dirty them either.)
	/**/
	if (!fFierce && caTap.doc == doc && vitcMic == 0 &&
			cpFirst >= caTap.cpFirst
			&& (cpLim = cpFirst + dcp) < caTap.cpLim)
		{
		for (itc = vtapFetch.itcMac; itc; --itc)
			if (cpLim >= vmpitccp[itc])
				break;
		if (cpFirst >= vmpitccp[itc])
			return;
		}

	/* table properties implicitly depend on the character before and
	/* after the CA (for calculating the fFirstRow and fLastRow flags),
	/* So, we extend the invalidation interval by one in each direction
	/* to make sure we invalidate enough.
	/**/
	if (cpFirst > cp0)
		{
		--cpFirst;
		dcp += 2;
		}
	else
		++dcp;

	InvalCa(&caTap, doc, cpFirst, dcp); /* NOTE - this invalidates vmpitccp by association */
	InvalCa(&vtcc.ca, doc, cpFirst, dcp);
	InvalCa(&vtcc.caTap, doc, cpFirst, dcp);
#ifdef MAC
	InvalCa(&vtcc.caTapPrev, doc, cpFirst, dcp);
#else /* WIN */
	InvalCa(&caTapAux, doc, cpFirst, dcp);
#endif /* WIN */
	InvalCa(&vtcxs.ca, doc, cpFirst, dcp);
	InvalCa(&caTable, doc, cpFirst, dcp);
}


/* I N V A L  C P */
/* dl's depending on characters [cpFirst, cpFirst + dcp) are invalidated */
/*  %%Function:InvalCp %%Owner:peterj %%reviewed: 6/28/89  */
InvalCp(pca)
struct CA *pca;
{
	InvalCp2(pca->doc, pca->cpFirst, DcpCa(pca), fTrue);
}


/* I N V A L  C P  1 */
/* same, but with ca + 1 */
/*  %%Function:InvalCp1 %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT InvalCp1(pca)
struct CA *pca;
{
	CP	dcp;

	dcp = DcpCa(pca);
	InvalCp2A(pca->doc, pca->cpFirst, dcp + 1, fTrue);
	InvalTableProps(pca->doc,pca->cpFirst,dcp,fFalse/*fFierce*/);
}


#ifdef MAC
NATIVE /* WINIGNORE - MAC only */
#endif /* MAC */
/* I N V A L  C P  2 */
/*  %%Function:InvalCp2 %%Owner:peterj %%reviewed: 6/28/89 */
InvalCp2(doc, cpFirst, dcp, fSetPlcUnk)
int doc; 
CP cpFirst, dcp;
int fSetPlcUnk;
{
	InvalCp2A(doc, cpFirst, dcp, fSetPlcUnk);
	InvalTableProps(doc, cpFirst, dcp, fFalse/*fFierce*/);
}


#ifdef MAC
NATIVE /* WINIGNORE - MAC only */
#endif /* MAC */
/*  %%Function:InvalCp2A %%Owner:peterj %%reviewed: 6/28/89 */
InvalCp2A(doc, cpFirst, dcp, fSetPlcUnk)
int doc; 
CP cpFirst, dcp;
int fSetPlcUnk;
{
	int ww;
	struct WWD **hwwd;
	int isels, ised;
	struct SELS *psels;
	struct DOD *pdod;
	extern struct PL **vhplbmc;

	pdod = PdodDoc(doc);
	for (ww = WwDisp(doc, wwNil, fTrue); ww != wwNil; ww = WwDisp(doc, ww, fTrue))
		{
		hwwd = HwwdWw(ww);
		if (!(*hwwd)->fOutline)
			pdod->fOutlineDirty = fTrue;
		Mac( (*hwwd)->fFullSizeUpd = fTrue );

		InvalCp3(hwwd, doc, cpFirst, dcp, fFalse);
		(*hwwd)->fDirty = fTrue;
		} /* next ww */

	InvalCa(&caPara, doc, cpFirst, dcp);
	InvalCa(&caPage, doc, cpFirst, dcp);
	InvalCa(&caSect, doc, cpFirst, dcp);

	if (caSect.doc == doc && cpFirst + dcp >= CpMac1Doc(doc))
		caSect.doc = docNil;
#ifdef WIN
	InvalCa(&vsab.caCopy, doc, cpFirst, dcp);
	if (doc == vlcb.ca.doc)
/* don't check for cp ranges -- need more severe inval than that */
		vlcb.ca.doc = docNil;

#endif /* WIN */

	if (vhplbmc != hNil && !vrf.fBorderCropPic)
		InvalBmc(doc, cpFirst, dcp);

/* layout caches need to be disposed of when the doc is changed -- this is
	heavy-handed, but it's expensive to determine if it's really necessary */
	if (doc == vfls.ca.doc)
		vfls.ca.doc = docNil;
	if (doc == caHdt.doc)
		caHdt.doc = docNil;

#ifdef BOGUS  /* InvalCp shouldn't be blowing GoBack cache away */
	for (isels = 0, psels = &rgselsInsert[0]; isels < iselsInsertMax; isels++, psels++)
		InvalCa(&psels->ca, doc, cpFirst, dcp);
#endif

	if (vfli.doc == doc)
		InvalFli();
/* invalidate scc's */
	if (vsccAbove.doc == doc &&
			vsccAbove.dlMac != 0 &&
			CpMacPlc(vsccAbove.hplcedl) > cpFirst &&
			vsccAbove.cpFirst < cpFirst + dcp)
		{
		vsccAbove.dlMac = 0;
		PutIMacPlc( vsccAbove.hplcedl, 0 );
		vsccAbove.fFull = fFalse;
		}
#ifdef MAC
	if (vpsccBelow)
		InvalSccLarge(doc, cpFirst, dcp);
#endif	

#ifdef WIN
	/* if the current selection is in the invalidated section, cause the
		pap and chp flags to be set--change may have cause one of them to
		change */
	if (doc == selCur.doc && cpFirst <= selCur.cpLim &&
			cpFirst + dcp >= selCur.cpFirst)
		InvalSelCurProps (fFalse /*fSelChanged*/);

	InvalVisiCache();
	InvalCellCache();
	if (doc == docSeqCache && cpFirst <= cpSeqCache)
		docSeqCache = docNil;
#endif      

	FreezeHp();
	pdod = PdodDoc(doc);
/* invalidate plcpgd and plcpad */
	if (fSetPlcUnk)
		{
		if (pdod->hplcphe != hNil)
			SetPlcUnk(pdod->hplcphe, cpFirst, cpFirst + dcp);
		if (pdod->hplcpgd != hNil)
			SetPlcUnk(pdod->hplcpgd, cpFirst, cpFirst + dcp);
		if (!pdod->fShort)
			{
			if (pdod->hplcpad != hNil)
				SetPlcUnk(pdod->hplcpad, cpFirst, cpFirst + dcp);
			if (pdod->hplcsed != hNil)
				SetPlcUnk(pdod->hplcsed, cpFirst, cpFirst + dcp);
			}
		else  if (pdod->dk == dkDispHdr && (ised = pdod->ised) >= 0)
			{
			struct DOD *pdodMother = PdodMother(doc);
			if (pdodMother->hplcpgd != hNil)
				{
				/* inval mother doc's page table */
				SetPlcUnk(pdodMother->hplcpgd, 
						CpPlc(pdodMother->hplcsed, ised), 
						CpMacDoc(DocMother(doc)));
				caHdt.doc = docNil;
				}
			}
		}
	MeltHp();

	if (!selCur.fBlock && !selCur.fTable)
		{
		selCur.xpFirst = xpNil;
		selCur.xpLim = xpNil;
		}

/* invalidate sequential fetch */
	if (doc == vdocFetch)
		vdocFetch = docNil;
}


/* I N V A L  C P  3 */
/* used to invalidate within a hpldr */
/* if fDrDirty is set, invalidation is for changing embedded dr properties */
/*  %%Function:InvalCp3 %%Owner:peterj %%reviewed: 6/28/89 */
InvalCp3(hpldr, doc, cpFirst, dcp, fDrDirty)
struct PLDR **hpldr;
int doc;
CP cpFirst, dcp;
{
	struct PLCEDL **hplcedl;
	int dlMac, dl;
	int idrMac, idr;
	struct DR *pdr;
	CP cpMac, cpT;
	struct EDL edl;
	BOOL fPrevRowDirty;
	struct DRF drfFetch, drfFetchT;

	/* check all dr's in wwd */
	idrMac = (*hpldr)->idrMac;
	for (idr = 0; idr < idrMac; FreePdrf(&drfFetch), ++idr)
		{
		pdr = PdrFetch(hpldr, idr, &drfFetch);
		/* hplcedl can be hNil if window is not visible */
		if (pdr->doc != doc || (hplcedl = pdr->hplcedl) == hNil)
			continue;

		dlMac = IMacPlc(hplcedl);
		if (dlMac == 0)
			continue;
		dl = dlMac - 1;

		/* check all edl's in dr */
		cpMac = CpPlc(hplcedl, dlMac);
		GetPlc(hplcedl, dl, &edl);
/* invalid area beyond displayed/dependent range */
/* or area is before first char displayed. This is strictly speaking
wrong because the changed text may have an effect of cpFirst of the prd.
We depend on a post-invalidation normalize to right things up.
*/
		if (cpFirst >= cpMac + edl.dcpDepend || pdr->cpFirst >= cpFirst + dcp)
			continue;

/* compute first dl in scan */
		if (cpFirst < cpMac) /* else dl is correct */
			{
			if (cpFirst < pdr->cpFirst + pdr->dcpDepend)
/* invalidation intersects area on which pdr->cpFirst depends */
				{
				pdr->fCpBad = fTrue;
				if ((*hpldr)->hpldrBack == hNil)
					{
					struct DR *pdrT;
					int idrT;
					((struct WWD *)(*hpldr))->fSetElev = fTrue;
					idrT = IdrPrevFlow(hpldr, idr);
					if (idrT != idrNil)
						{
						pdrT = PdrFetch(hpldr, idrT, &drfFetchT);
						pdrT->fDirty = fTrue;
						DirtyEdl(pdrT->hplcedl,
								IMacPlc(pdrT->hplcedl) - 1);
						FreePdrf(&drfFetchT);
						}
					}
				dl = 0;
				}
			else
/* know: cpFirst is in [pdr->cpFirst, rgcp[dlMac]) */
				{
/* in outline mode, rgcp[0] may be > dr.cpFirst!, e.g. when FOutlineEmpty */
				dl = max(IInPlcCheck(hplcedl, cpFirst), 0);
				if (dl != 0)
					{
					/* AdjustCp can leave identical, consecutive cp's in 
					   hplcedl; we want the first one. This is slower but
					   cleaner than kludges to get the fMult plc processing */
					for (cpT = CpPlc(hplcedl, dl); dl > 0 && cpT == CpPlc(hplcedl, dl - 1); dl--)
						;
					if (dl != 0)
						{
						GetPlc(hplcedl, dl - 1, &edl);
						if (cpFirst < CpPlc(hplcedl, dl) + edl.dcpDepend)
							dl--;
						}
					}
				}
			}
		Mac(Debug(Break1()));
		for ( fPrevRowDirty = fFalse; dl < dlMac; dl++)
			{
			if (cpFirst + dcp <= (cpT = CpPlc(hplcedl, dl)))
				{
				if (fPrevRowDirty && dl < dlMac)
					{
					GetPlc(hplcedl, dl, &edl);
					if (edl.hpldr != hNil)
						{
						edl.fTableDirty = fTrue;
						PutPlcLast(hplcedl, dl, &edl);
						}
					}
				break;
				}
			GetPlc(hplcedl, dl, &edl);
			if (edl.hpldr != hNil && !fDrDirty
					/* does the invalid region lie within the table? */
			&& ((cpFirst >= cpT && cpFirst + dcp < cpT + edl.dcp)
					/* or miss it completely (just touches the dcpDepend region) */
			|| cpFirst >= cpT + edl.dcp))
				{
				edl.fTableDirty = fTrue;
				PutPlcLast(hplcedl, dl, &edl);
				InvalCp3(edl.hpldr, doc, cpFirst, dcp, fFalse);
				}
			else
				{
				fPrevRowDirty = edl.fDirty = fTrue;
				PutPlcLast(hplcedl, dl, &edl);
				}
			}
		pdr->fDirty = fTrue;
		} /* next dr */
}


/* I N V A L  P A R A  S E C T */
/* Invalidate para containing cpDest if docSrc[cpFirstSrc:cpLimSrc)
contains a paragraph return.
Also invalidate whole section if inserting/deleting section mark
fSame is set iff pca's are the same and hence the focus is on deleting this
range of cp's.
*/
/*  %%Function:InvalParaSect %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT InvalParaSect(pcaDest, pcaSrc, fSame)
struct CA *pcaDest, *pcaSrc;
{
	int docDest = pcaDest->doc;
	int docSrc;
	CP cpDest = pcaDest->cpFirst;
	CP cpFirst;
	struct CA caT;

/* if the doc is the undo document there is no need to do
display invalidation since the undo document is never displayed.
So return. */
	if (docDest == docUndo)
		return;

	if (!PdodDoc(pcaSrc->doc)->fShort && !PdodDoc(docDest)->fShort)
		{
		CacheSectCa(pcaSrc);
		if (pcaSrc->cpLim >= caSect.cpLim)
			{ /* Sel includes sect mark */
			CacheSectCa(pcaDest);
			/* only text before a deleted section mark is affected */
			InvalCp(PcaSet(&caT, docDest, caSect.cpFirst, cpDest));
			}
#ifdef WIN
/* can restart footnote number for each section, therefore need InvalFtn. */
		InvalFtn(docDest, cpDest);
#endif
		}

	CacheParaCa(pcaSrc);
	if (pcaSrc->cpLim >= caPara.cpLim)
		{ /* Diddling with a para return */
		if (fSame)
			/* pcaDest == pcaSrc */
			{
			struct PAP pap;
			pap = vpapFetch;
			CachePara(caPara.doc, pcaDest->cpLim);
			pap.phe = vpapFetch.phe;
			Assert (pcaDest->cpLim <= caPara.cpLim);
			if (!FNeRgw(&vpapFetch, &pap, cwPAP))
				return;
			}
		CacheParaCa(pcaDest);
/* invalidate at least from cpFirst to cpDest (+-1 for "box" borders) */
		cpDest = CpMin(CpMac2Doc(docDest), cpDest + 1);
		cpFirst = CpMax((CP)1, caPara.cpFirst) - 1;
		CachePara(pcaDest->doc, pcaDest->cpLim);

		/* yes, really use vpapFetch.fInTable directly! */
		if (vpapFetch.fInTable)
			cpDest = CpMax(cpDest, caPara.cpLim);
		InvalCp(PcaSet(&caT, docDest, cpFirst, cpDest));
		}
}


/* I N V A L  C P  1  S U B */
/* as invalcp1 but also invalidates footnote/ annotation subdocs */
/*  %%Function:InvalCp1Sub %%Owner:peterj %%reviewed: 6/28/89 */
EXPORT InvalCp1Sub(pca)
struct CA *pca;
{
	struct CA caT;
	struct CA caSub;

	caT = *pca;
	caT.cpFirst = CpMax(caT.cpFirst - 1L, 0L);
	InvalParaSect(&caT, &caT, fTrue /* fSame */);
	InvalCp1(&caT);

	if (FInvalSub(&caT, &caSub, edcDrpFtn))
		InvalCp1(&caSub);
#ifdef WIN
	if (FInvalSub(&caT, &caSub, edcDrpAtn))
		InvalCp1(&caSub);
#endif

}


/* F  I N V A L  S U B */
/* return true if there is something to invalidate. pcaSub is ca to inval */
/* we will invalidate if the mother ca includes footnote/ annot references */
/*  %%Function:FInvalSub %%Owner:peterj %%reviewed: 6/28/89 */
FInvalSub(pcaMom, pcaSub, edc)
struct CA *pcaMom;
struct CA *pcaSub;
int edc;
{
	CP cpRef;
	int docSub;
	struct DOD *pdod = PdodDoc(pcaMom->doc);

	if (pdod->fShort)
		return fFalse;

	Assert (edc == edcDrpFtn Win(|| edc == edcDrpAtn));

#ifdef MAC
	docSub = pdod->docFtn;
#else
	docSub = (edc == edcDrpFtn) ? pdod->docFtn : pdod->docAtn;
#endif /* MAC */

	if (docSub != docNil)
		{
		pcaSub->cpFirst = CpFirstRef(pcaMom->doc, pcaMom->cpFirst,
				&cpRef, edc);
		if (cpRef < pcaMom->cpLim)  /* there are footnote/anntns in the sel */
			{
			pcaSub->doc = docSub;
			pcaSub->cpLim = CpFirstRef(pcaMom->doc, pcaMom->cpLim,
					&cpRef, edc);
			return fTrue;
			}

		}
	return fFalse;  /* nothing to inval */
}


/* S E T  P L C  U N K  */
/* invalidate all pgd/pad/sed/phe entries between cpFirst and cpLim */
/*  %%Function:SetPlcUnk %%Owner:peterj %%reviewed: 6/28/89 */
SetPlcUnk(hplcfoo, cpFirst, cpLim)
struct PLC **hplcfoo;
CP cpFirst, cpLim;
{
	int ifoo, ifooLim;
	int ifooMac = IMacPlc(hplcfoo);
#ifdef DEBUG
	int ifooT, ifooLimT;
#endif /* DEBUG */
	union 
		{
		struct PAD pad;
		struct PGD pgd;
		struct PHE phe;
		struct SED sed;
		} foo;

	Assert(cbPGD != cbPAD && cbPGD != cbPHE && cbPGD != cbSED);/* has to be unique */
	if (ifooMac == 0)
		return;
	ifoo = IInPlcMult(hplcfoo, CpMax(cpFirst, CpPlc(hplcfoo, 0)));
	if (ifoo > 0 && (*hplcfoo)->cb == cbPGD)
		{
		/* dcpDepend for pgd; PREVIOUS page has dcp for which this
			page needs to worry */
		GetPlc(hplcfoo, ifoo - 1, &foo);
		if (CpPlc(hplcfoo, ifoo) + foo.pgd.dcpDepend >= cpFirst)
			ifoo--;
		}
#ifdef WIN
#ifdef DEBUG
	ifooLimT = ifooT = ifoo;
	for (; ifooT < ifooMac && cpLim > CpPlc(hplcfoo, ifooT); ifooT++)
		ifooLimT = ifooT + 1;
#endif /* DEBUG */
	if ((ifooLim = IInPlcRef(hplcfoo, cpLim)) < 0)
		ifooLim = ifooMac;
	Assert (ifooLim == ifooLimT || ifooLim < ifoo);
	MiscPlcLoops(hplcfoo, ifoo, ifooLim,
			0 /* pResult */, 0 /* SetPlcUnk */);
#else /* !WIN */
	for (; ifoo < ifooMac && cpLim > CpPlc(hplcfoo, ifoo); ifoo++)
		{
		GetPlc(hplcfoo, ifoo, &foo);
		foo.pad.fUnk = fTrue;
		PutPlcLast(hplcfoo, ifoo, &foo);
		}
#endif /* !WIN */
}



/* Classification of ucm's int ucc's (Undo Classification Codes):

uccCopy: save scrap in undo
	ucmCopy:
uccCut: save scrap in undo, save operand in scrap.
	ucmCut:
uccDeleteSel: save operand in undo, except if !fDelete.
	ucmDeleteSel:

uccFormat: save operand in undo, ca "after" is same.
	ucmItalic, etc.
	ucmFormatTable:

uccPaste: save "before" in undo, save ca "after"
	ucmInsertSect:
	ucmMergeCells:
	ucmSplitCells:
	ucnDeleteRows:
	ucmDeleteColumns:
	ucmDeleteCellsHoriz:
	ucmDeleteCellsVert:
	ucmCopySp:
	ucmDeleteSel:
	ucmFormatting:
	ucmGlsyExpand:
	ucmInsertSpec:
	ucmPaste:
	ucmPasteBlock:
	ucmNewTable:
	ucmInsColumns:
	ucmInsCellsHoriz:
	ucmInsCellsVert:
	ucmInsGraphic:

uccInsert: if fDelete, save "delete" operand in undo. save ca "after".
	ucmTyping:
	ucmIndex:
	ucmInsRows:

uccMove: special case
	ucmMove:

uccMoveNil: special case, same as uccMove, except setting undo is delayed.
	ucmMove* in outline

*/

/* F  S E T  U N D O  B E F O R E */
/* returns fFalse iff command is to be cancelled because of user option.
If user elects to proceed without undo, this will be marked in
vuab.fNoUndo. fNoUndo is inited to fFalse, uac is inited to uacNil.

This procedure is to be called before the command is executed.
It will:
	save selCur
	save the fDirty and fFormatted bits of selCur.doc.
	possibly save the range selCur (psel in SetUndoBefore1) in the undo
		doc and also set vuab.ca.
		for umcCopy and umcCut, the scrap needs to be saved in
		the undo buffer.

To arm the undo, SetUndoAfter will also have to be called.
*/
/*  %%Function:FSetUndoBefore %%Owner:???? */
FSetUndoBefore(ucm, ucc)
{
	return (FSetUndoB1(ucm, ucc, &selCur.ca));
}


/* F  S E T  U N D O  B  1 */
/*  %%Function:FSetUndoB1 %%Owner:???? */
FSetUndoB1(ucm, ucc, pca)
struct CA *pca;
{
	int doc = pca->doc;
	struct CA caT;

	if (vmerr.fMemFail)
		return fFalse;

#ifdef WIN
	if (vrulss.caRulerSprm.doc != docNil)
		{
		FlushRulerSprms();
		}
#endif
	vuab.uac = uacNil;
	vuab.cpFrom = cpNil;
	vuab.fRedo = fFalse;
	vuab.fNoUndo = fFalse;
	vuab.fDelete = fFalse;
	vuab.fOutlineDirty = fFalse;
	vuab.fOutlineInval = fFalse;
	vuab.selsBefore = *((struct SELS *)&selCur);
	vuab.wwBefore = selCur.ww;
	if (pca != NULL)
		{
		/* this includes fFormatted */
		vuab.grpfDirty = PdodDoc(doc)->grpfDirty;
		vuab.fFormatted = PdodDoc(doc)->fFormatted;
		vuab.fMayHavePic = (doc == docScrap) ? vsab.fMayHavePic : 
				PdodMother(doc)->fMayHavePic;
		vuab.ca = *pca;
		AccumulateDcpEdit(DcpCa(pca));
		}
	vuab.caMove.doc = docNil;
	vuab.ucm = ucm;
	vuab.ucc = ucc;
	vuab.docInvalPageView = docNil;

	switch (ucc)
		{
		struct CA caT;
	case uccCopy:
/* copy into scrap */
	case uccCut:
/* copy or delete into scrap; save scrap. If external scrap, it is
still there, so it is effectively already saved */
		PcaSetWholeDoc(&caT, docScrap);
		/* for Opus, fExt will always be false */
		Assert (!fWin || vsab.fExt == fFalse && vuab.fExt == fFalse);
		if (vuab.fExt = vsab.fExt)
			PcaSetNil(&caT);
/* save fExt, fPict, fBlock, fFormatted */
		vuab.sabt = vsab.sabt;
		SetWholeDoc(docUndo, &caT);
		break;
	case uccPaste:
	case uccFormat:
/* save selection in undo buffer */
		SetWholeDoc(docUndo, pca);
		/* FALL THROUGH */
	case uccDeleteSel:
	case uccMove:
	case uccInsert:
		break;
	default:
		vuab.fNoUndo = fTrue;
		return(fTrue);
		}

	if (vmerr.fMemFail)
		{
		if (vhwndCBT)
			{
			/* Terminate CBT, since they aren't expecting this and can't
				handle it.  And don't continue with operation, so return fFalse */
			CBTMemError(eidNoMemCBT /* anything non-zero */);
			return fFalse;
			}
		else
			{
#ifdef MAC
			if (!FAlert(SzSharedKey("No room for Undo. Continue command without Undo?", NoRoomForUndo)))
#else /* WIN */
			if (IdMessageBoxMstRgwMb(mstUndo, NULL, MB_YESNO) == IDNO)
#endif /* MAC */
				{
				vmerr.fHadMemAlert = fTrue;
				return fFalse;
				}
			vmerr.mat = matNil;
			vuab.fNoUndo = fTrue;
			}
		}
	return fTrue;
}


/* F  S E T  U N D O  D E L E T E */
/* procedure is called after a neutral FSetUndoBefore, if the command
decides to delete its operand.
Returns false iff operation is cancelled. */
/*  %%Function:FSetUndoDelete %%Owner:????  */
FSetUndoDelete(pca, fScrap)
struct CA *pca;
BOOL fScrap;
{
	BOOL f;
	int ucm = vuab.ucm;
	int ucc = vuab.ucc;
	struct SELS sels;

	sels = vuab.selsBefore; /* save because current sels may be already
	adjusted in Insert */
	if (ucm == ucmPasteBlock) return fTrue;
	f = FSetUndoB1(ucmCut, fScrap ? uccCut : uccPaste, pca);

	vuab.ucm = ucm;
	vuab.ucc = ucc;
	vuab.selsBefore = sels;
	vuab.fDelete = fTrue;
	return f;
}


/* F  S E T  U N D O  D E L E T E  A D D */
/* if more of the operand is deleted, it can be prepended or appended to
the undo buffer.
If the replace fails, failure is returned, but no other action is taken
(as opposed to FSetUndoBefore)
*/
/*  %%Function:FSetUndoDeleteAdd %%Owner:???? */
FSetUndoDelAdd(pca, fAppend)
struct CA *pca; 
BOOL fAppend;
{
	struct CA caT;

	if (vmerr.fMemFail)
		return fFalse;

#ifdef WIN
	if (!vuab.fDelete)
		return (FSetUndoDelete(pca, fFalse));
#else
	if (!vuab.fDelete)
		return (!FSetUndoDelete(pca, fFalse));
#endif

	return FReplaceCps(PcaPoint(&caT, docUndo, fAppend ?
			CpMacDocEdit(docUndo) : cp0), pca);
}


/* S E T  U N D O  A F T E R */
/*
Procedure is to be called to arm the undo of the command, once the
final selection is set up (not necessarily marked on screen) and the
command has been completed satisfactorily in all respects.

Procedure will:
	arm the undo (by setting the proper uac)
	save selCur for use by redo.
	possibly save supplementary information passed in a ca.
*/
/*  %%Function:SetUndoAfter %%Owner:???? */
SetUndoAfter(pca)
struct CA *pca;
{
	int uac = uacNil;

/* set document dirty if not a Copy command. */
	if (vuab.ucm != ucmCopy && vuab.ca.doc != docNil)
		PdodDoc(vuab.ca.doc)->fDirty = fTrue;

	if (vuab.fNoUndo || vmerr.fMemFail)
		{
		vuab.ucm = ucmNil;
		return;
		}

	vuab.selsAfter = *((struct SELS *)&selCur);
	vuab.wwAfter = wwCur;

	switch (vuab.ucc)
		{
	case uccCopy:
		uac = uacCopy;
		break;
	case uccCut:
		uac = uacScrap;
		/* fall through */
	case uccDeleteSel:
#ifdef WIN
		if (pca != NULL)	/* For rev mark case */
			vuab.ca = *pca;
		else
#endif
			PcaPoint(&vuab.ca, vuab.ca.doc, vuab.ca.cpFirst);
		if (vuab.ucc == uccDeleteSel)
			goto LUndoOrNil;
		break;
	case uccFormat:
		uac = uacUndo;
		break;
	case uccPaste:
		vuab.fDelete = fTrue;
	case uccInsert:
	case uccMove:
		Assert(pca != 0);
		AccumulateDcpEdit(DcpCa(pca));
		vuab.ca = *pca;
LUndoOrNil:
		uac = uacUndo;
		if (!vuab.fDelete) uac = uacUndoNil;
		break;
	default:
		vuab.fNoUndo = fTrue;
		vuab.ucm = ucmNil;
		return; /* no arming for unknown ucm */
		}
	vuab.uac = uac;
}


/* S E T   U N D O   N I L */
/*  %%Function:SetUndoNil %%Owner:???? */
SetUndoNil()
{
	int mat;
	struct CA caT;

	mat = vmerr.mat;
	vmerr.mat = matNil;
	Win(DisableInval());

	SetWholeDoc(docUndo, PcaSetNil(&caT));

	Win(EnableInval());
	vmerr.mat = mat;

	SetUndoN1();
}


/* S E T   U N D O   N 1 */
/*  %%Function:SetUndoN1 %%Owner:???? */
SetUndoN1()
{
	vuab.ucm = ucmNil;
	vuab.uac = uacNil;
	vuab.fNoUndo = fTrue;
#ifdef FUTURE /* (cc) looks like we should set docNil here, see bug #3349 */
/* did someone rely on vuab.ca.doc untouched after this routine? */ 
	vuab.ca.doc = vuab.caMove.doc = docNil;
#endif

	Mac(vaab.asc = ascNil);
	Mac(vaab.acc = accNil);
	Mac(vaab.ucm = ucmNil);
}


#ifdef NOTUSED
/* R E S E T  U N D O */
/* used to initialize undo state before a command */
/*  %%Function:ResetUndo %%Owner:NOTUSED  */
ResetUndo()
{
	vuab.ucm = ucmNil;
}


#endif


/* S E T  W H O L E  D O C */
/* Replace contents of docDest with docSrc[cp:cp+dcp)
	Note that this works only on full docs */
/*  %%Function:SetWholeDoc %%Owner:davidlu */
SetWholeDoc(docDest, pca)
int docDest;
struct CA *pca;
{
	struct PLC **hplcpcdDest;
	struct DOD **hdod = mpdochdod[docDest];
	int ipcdLast;
	int docSrc = pca->doc, docSub;
	struct SED sed;
	struct CA caT;
	struct PCD pcd;

	Assert(!(*hdod)->fShort);
	ZeroDocTables(docDest); /* So ReplaceCps doesn't worry about them */
	(*hdod)->fSedMacEopCopied = fFalse;

/* destroy section sprms attached to document end */
	hplcpcdDest = PdodDoc(docDest)->hplcpcd;
	ipcdLast = IMacPlc(hplcpcdDest) - 2;
	Assert(CpPlc(hplcpcdDest, ipcdLast) + ccpEop ==
			CpMac1Doc(docDest));
	GetPlc(hplcpcdDest, ipcdLast, &pcd);
	pcd.prm = 0;
	PutPlc(hplcpcdDest, ipcdLast, &pcd);

/* demolish subdocs if any -- must remove the subdoc from the dod before
	calling DisposeDoc, in case mother doc is docUndo -- DisposeDoc calls
	SetUndoNil! */
	if ((docSub = (*hdod)->docHdr) != docNil)
		{
		(*hdod)->docHdr = docNil;
		DisposeDoc(docSub);
		}
	if ((docSub = (*hdod)->docFtn) != docNil)
		{
		(*hdod)->docFtn = docNil;
		DisposeDoc(docSub);
		}
#ifdef WIN
	if ((docSub = (*hdod)->docAtn) != docNil)
		{
		(*hdod)->docAtn = docNil;
		DisposeDoc(docSub);
		}
#endif

	FReplaceCps(PcaSetWholeDoc(&caT, docDest), pca);

/* if the scrap is visible somewhere on the screen, update it */
	if (docDest == docScrap)
		{
		/* vsab.fPict = vsab.fBlock = vsab.fExt = 0 */ vsab.sabt = 0;
		vsab.fExtEqual = fFalse;
		if (pca->cpFirst == pca->cpLim || docSrc == docNil)
			vsab.fFormatted = vsab.fMayHavePic = fFalse;
		else
			{
			vsab.fFormatted = PdodDoc(docSrc)->fFormatted;
			vsab.fMayHavePic = PdodMother(docSrc)->fMayHavePic;
			}
#ifdef MAC
		if (wwScrap != wwNil)
			InvalidateWindow(PwwdWw(wwScrap)->wwptr);
		FreePh(&vsab.hstLink);
#endif
		}
}


/* Z E R O  D O C  T A B L E S */
/* Remove all footnote & section references from doc to simplify
writing over its contents */
/*  %%Function:ZeroDocTables %%Owner:davidlu */
ZeroDocTables(doc)
{
	struct DOD **hdod = mpdochdod[doc];
	struct PLC **hplcsed;
	struct SED sed;
#ifdef WIN
	FreePhplc(&(*hdod)->hplcfld);
	FreePh(&(*hdod)->hsttbBkmk);
	FreePhplc(&(*hdod)->hplcbkf);
	FreePhplc(&(*hdod)->hplcbkl);
	FreePhplc(&(*hdod)->hplcatrd);
#endif
	FreePhplc(&(*hdod)->hplcphe);
	FreePhplc(&(*hdod)->hplcfnd);
	FreePhplc(&(*hdod)->hplcfrd);
	Assert( (*hdod)->hplcsed != hNil );
	hplcsed = (*hdod)->hplcsed;
	Assert( (*hplcsed)->iMac >= 2 );
	FOpenPlc( hplcsed, 0, -((*hplcsed)->iMac - 2) );
	PutCpPlc(hplcsed, 0, cp0);
	PutCpPlc(hplcsed, 1, CpMac1Doc(doc));
	sed.fUnk = fFalse;
	sed.fn = fnScratch;
	sed.fcSepx = fcNil;
	PutPlc(hplcsed, 0, &sed);
#ifdef DEBUG
	GetPlc(hplcsed, 1, &sed);
	Assert(sed.fcSepx == fcNil && sed.fn == fnScratch);
#endif /* DEBUG */
	if ((*hdod)->fSea)
		FreePhplc(&(*hdod)->hplcsea);
	FreePhplc(&(*hdod)->hplcpad);
	FreePhplc(&(*hdod)->hplcpgd);
}


/* F  D E L E T E  C H E C K */
/* Check the proposal to delete text selected by selcur.
Return true iff proposal is acceptable.
*pca may be changed to exclude the last Eop in the document.
Paragraph marks between footnotes may not be deleted.
*/
/*  %%Function:FDeleteCheck %%Owner:davidlu */
int FDeleteCheck(fPaste, rpk, pcaIns)
int fPaste;     /* Is this from the paste command?? */
struct CA *pcaIns;
{
	CP cpFirst, cpLim;

	if (selCur.fTable && !fPaste Mac(&& wwCur != wwScrap))
		{
		CpFirstTap(selCur.doc, selCur.cpFirst);
		if (vtapFetch.itcMac <= selCur.itcFirst)
			goto LRetFalse;
		cpFirst = CpFirstForItc(selCur.doc, selCur.cpFirst, selCur.itcFirst);
		cpLim = vmpitccp[min(vtapFetch.itcMac, selCur.itcFirst+1)] - ccpEop;
		Select(&selCur, cpFirst, cpLim);
		return fTrue;
		}
	if ( Mac(wwCur == wwScrap || )
			(selCur.fBlock && !selCur.fTable))
		goto LRetFalse;
	if ((selCur.fWithinCell || selCur.fTable) && !selCur.fIns)
		{
		CachePara(selCur.doc, selCur.cpLim-1);
		if (FInTableVPapFetch(selCur.doc, selCur.cpLim-1))
			{
			/* disallow deleting a cell mark */
			CacheTc(wwNil, selCur.doc, selCur.cpLim-1, fFalse, fFalse);
			if (vtcc.cpLim == selCur.cpLim)
				goto LRetFalse;
			}
		}
#ifdef MAC
	else  if (!selCur.fIns && FBogusTblSel(&selCur))
		goto LRetFalse;
#endif /* MAC */

	/* since not block, we can call this */
	return (FDelCkBlockOk(rpk, pcaIns));
LRetFalse:
	Beep();
	return fFalse;
}


/* F  D E L E T E  C H E C K  B L O C K  O K */
/*
Like FDeleteCheck, but also supports block selections, by giving
footnote check the start and end cp's in the block, and by checking
for partial fields with multiple FLegalSel calls.
*/
/*  %%Function:FDelCkBlockOk %%Owner:davidlu */
BOOL FDelCkBlockOk(rpk, pcaIns)
struct CA *pcaIns;
{
	return FDelCkBlockPsel(&selCur, rpk, pcaIns, fTrue /* fReportErr */,
			fFalse /* fSimple */);
}


#ifdef MAC
/* F   B o g u s   T b l  S e l */
/*  %%Function:FBogusTblSel %%Owner:Notused */
int FBogusTblSel(psel)
struct SEL *psel;
{
/* failsafe in case an invalid table selection is made */
	if (selCur.fIns || selCur.fTable || selCur.doc == docNil)
		return(fFalse);
	if (FInTableDocCp(psel->doc, psel->cpFirst))
		{
		CacheTc(wwNil, psel->doc, psel->cpFirst, fFalse, fFalse);
		CpFirstTap(psel->doc, psel->cpFirst);
		if (psel->cpLim < vtcc.cpLim)
			return(fFalse);
		if (psel->cpFirst > caTap.cpFirst || psel->cpLim < caTap.cpLim)
			return(fTrue);
		}
	if (FInTableDocCp(psel->doc, CpMax(cp0, psel->cpLim-1)))
		{
		CpFirstTap(psel->doc, psel->cpLim-1);
		if (psel->cpLim < caTap.cpLim)
			return(fTrue);
		}
	return(fFalse);
}


#endif /* MAC */
/* F  D E L  C K  B L O C K  P S E L */

/* NOTE:  fSimple is true if we know psel is skSel and if it is in a table,
	we have already guaranteed that it doesn't include any cell marks.  If
	fSimple, the only fields in the psel that are guaranteed to be set up
	correctly are: doc, cpFirst, cpLim, and sk (or any of the bits that make
	up sk).  Any code inserted in this routine which accesses other fields
	in psel should be skipped.  fSimple is currently just used for global
	replace, which can't take the time to make a new selection at each place
	it wants to replace, but it does make sure the table thing isn't being
	violated.  If you add code in here that is not executed if fSimple, please
	check the replace code to see if anything needs to be tweaked.  -rp
*/

/*  %%Function:FDelCkBlockPsel %%Owner: davidlu */
BOOL FDelCkBlockPsel(psel, rpk, pcaIns, fReportErr, fSimple)
struct SEL *psel; 
struct CA *pcaIns;
int fReportErr;
{
	int doc = psel->doc;
	BOOL fFtn = PdodDoc(doc)->fFtn;
	BOOL fAtn = PdodDoc(doc)->fAtn;
	CP cpFirstFirstLine = psel->cpFirst;
	CP cpLimLastLine = psel->cpLim;
	CP dcp, cpFirst;
	CP cpMac = CpMacDoc(doc);
	struct BKS bks;
	CP cpLim = cpLimLastLine;

	Assert(!fSimple || psel->sk == skSel);

#ifdef MAC
	if (wwCur == wwScrap)
		{
		Assert (fFalse);  /* see if we can see when it happens */
		goto LBeepReturn;
		}
#endif	/* MAC */

#ifdef WIN
	/* can't delete from doc locked for editing */
	if (PdodDoc(doc)->fLockForEdit)
		{
		if (fReportErr)
			ErrorEid(eidInvalidSelection, "FDelCkBlockPsel");
		return fFalse;
		}
#endif /* WIN */

/* don't need to worry about including the final para mark
	in a block sel since FGetBlockLine will omit ANY para marks
	when cutting.
*/
	if (!psel->fBlock)
		{
/* this return is here: for speed, for the cpFirst>CpMacDoc in outline case,
and to avoid resetting fInsEnd below */
		if (psel->fIns)
			{
			if (!psel->fWithinCell)
				goto LCheckFtnAtn;
			CachePara(psel->doc, psel->cpFirst);
			if (!vpapFetch.fTtp)
				{
				if (FInTableVPapFetch(psel->doc, psel->cpFirst))
					{
					CacheTc(wwNil, psel->doc, psel->cpFirst, fFalse, fFalse);
					CpFirstTap(psel->doc, psel->cpFirst);
					if (vtapFetch.rgtc[vtcc.itc].fMerged)
						{
						goto LBeepReturn;
						}
					}

LCheckFtnAtn:			
				if (!fFtn && !fAtn)
					return fTrue;
				}
			else
				{
				if (fReportErr)
					ErrorEid(eidNotValidEOR,"FDelCkBlkPsel");
				return fFalse;
				}
			}
		if (!FDeleteableEop(&psel->ca, rpk, pcaIns))
			{
			CP ccp = DcpCa(&psel->ca);
			if (cpLim == cpMac)  /* final EOP */
				{
				/* ccp of chSect is 1, not ccpEop! */
				cpLim = cpLimLastLine = cpMac - (ccp < ccpEop ? 1 : ccpEop);
/* this is so that the selection highlight will not be a factor. */
				if (!fSimple)
					TurnOffSel(psel);
				Assert (cpLim >= psel->cpFirst);
				}
			else  /* another EOP in the document */
				goto LBeepReturn;
			}
#ifdef WIN
	/* checks that sel is legal WRT fields */
		/* note cpLim, not selCur.cpLim used here, so if
			we fail, selCur.cpLim is unchanged
		*/
		if (!FLegalSel (doc, psel->cpFirst, cpLim))
			{
			goto LBeepReturn;
			}
#endif
		}
		else  /* block selection */			/* block facts:
			after InitBlockStat, bks.cpFirst will be the first cp
			in the block. After each FGetBlockLine, bks.cpFirst is
			set to vfli.cpLim of the line. When the last line is
			gotten, bks.cpFirst is set to the last cp of the last
			line of the block,  and the next call will return false
			without resetting bks.cpFirst, so we can grab it to be
			the cpLim of the last line of the block.
		
			When doing the footnote calc below, we give it the
			cpFirst and cpLim of the block, which really gives it
			more cp's than the block may contain, but that is ok
			for the purposes of that test.
		*/

		{
		Assert(!fSimple);
		InitBlock1(&bks, psel);
		Assert (bks.cpFirst <= bks.cpLast);  /* bogus block sel */

		cpFirstFirstLine = bks.cpFirst;  /* first cp in line in block */
		/* heap movement! */
		while (FGetBlockLine(&cpFirst, &dcp, &bks))
			{
#ifdef WIN
			if (!FLegalSel (doc, cpFirst, cpFirst + dcp))
				{
				goto LBeepReturn;
				}
#endif
			}
		cpLimLastLine = bks.cpFirst;  /* last cp in block */
		}

/* if valid, and at end of doc for non block sel, back up before
last para mark. This way sel is not changed if a test fails */

	if (!psel->fBlock)
		{
		psel->cpLim = cpLim;
		if (psel->cpFirst == psel->cpLim)
			{
			psel->sk = skIns;
			psel->fInsEnd = fFalse; /* bug fix */
			GetSelCurChp(fFalse);
			}
		}

	/* does not cross footnote/annotation boundary? */
	if (fFtn || fAtn)
		return FCheckRefDoc(doc, cpFirstFirstLine, cpLimLastLine, fReportErr);

	return fTrue;
LBeepReturn:
	if (fReportErr)
		Beep();
	return fFalse;
}


/* F  D E L E T E A B L E  E O P */
/* returns true iff ca does not include last eop OR if the document
integrity is maintained.  Prevents deleting the EOP before a table if
that would split a field.
rpk encodes the proposed replacement action: 
rpkNil: delete
rpkText: replace with unknown text
rpkCa: replace with caIns.
rpkOutline : replace with caIns but do allow tables to move to end of doc
*/
/*  %%Function:FDeleteableEop %%Owner: davidlu */
FDeleteableEop(pcaDel, rpk, pcaIns)
struct CA *pcaDel, *pcaIns;
{
	int doc;
	CP cpLim;
	CHAR chFetch;

	if (pcaDel->cpLim >= CpMacDoc(pcaDel->doc))
		{
		if (rpk == rpkText || pcaDel->cpFirst == cp0)
			return fFalse;
		if (rpk == rpkNil || pcaIns->cpLim == pcaIns->cpFirst)
			{
			doc = pcaDel->doc;
			cpLim = pcaDel->cpFirst;
			}
		else
			{
			doc = pcaIns->doc;
			cpLim = pcaIns->cpLim;
			}
		CachePara(doc, cpLim - 1);
		/* don't leave cell mark  or sect mark as last para terminator in doc */
		if (caPara.cpLim != cpLim ||
				(chFetch = ChFetch(doc, cpLim - 1, fcmChars)) == chSect ||
#ifdef CRLF
				/* make sure new eod is crlf pair */
				(cpLim < (CP)2 || chFetch != chEop || ChFetch(doc, cpLim - 2, fcmChars) != chReturn) ||
#endif
				(rpk != rpkOutline && FInTableVPapFetch(doc, cpLim - 1)))
			return fFalse;

		/* a new or struck-through eop cannot be the last one either */
		Assert(caPara.cpLim == cpLim);
		FetchCp(doc, cpLim - 1, fcmProps);
		return (!vchpFetch.fStrike && !vchpFetch.fRMark);
		}

	/* Prevent pasting unknown text over EOP before a table */
	if (rpk != rpkNil && FInTableDocCp(pcaDel->doc, pcaDel->cpLim))
		{
		CpFirstTap(pcaDel->doc, pcaDel->cpLim);
		if (pcaDel->cpLim == caTap.cpFirst && pcaDel->cpFirst != pcaDel->cpLim)
			return fFalse;
		}

	/* FInTableDocCp caches the para */
	if (FInTableDocCp(pcaDel->doc, pcaDel->cpLim) &&
			caPara.cpFirst != pcaDel->cpFirst)
		{
		/* FInTableDocCp caches the para */
		if ((!FInTableDocCp(pcaDel->doc, pcaDel->cpFirst) &&
				caPara.cpFirst != pcaDel->cpFirst) &&
				(!FLegalSel(pcaDel->doc, caPara.cpFirst, pcaDel->cpFirst)))
			{
			return fFalse;
			}
		}

	return fTrue;
}


#ifdef NOTUSED
/* C P  M A C  D O C  H E A D E R */
/* CpMacDoc, but with special casing for header documents, so that the end
of the current header is returned */
/*  %%Function:CpMacDocHeader %%Owner:notused  */
CpMacDocHeader(doc)
{
	struct DOD *pdod = PdodDoc(doc);
	if (!pdod->fHdr)
		return CpMacDoc(doc);
}


#endif

/* S E L E C T  P O S T  D E L E T E */
/*  %%Function:SelectPostDelete %%Owner:???? */
SelectPostDelete(cp)
CP cp;
{
	SelectIns(&selCur, CpMin(cp, CpMacDocEdit(selCur.doc)));
}


/* C H E C K  C O L L A P S E  B L O C K */
/*  Called by commands that don't understand block selections, collapses
selCur to an insertion point if it is a block selection.
*/
/*  %%Function:CheckCollapseBlock %%Owner:davidlu */
CheckCollapseBlock()
{
	if (selCur.fBlock)
		SelectIns(&selCur, selCur.cpFirst);
}


/* C M D  A U T O  D E L E T E */
/* for WIN, if autodelete preference is turned on, delete the selection; else
	make an insertion point at the beginning of the selection.
	for MAC, always delete the current selection (call CmdDeleteSelCur,
	see below)
*/
/*  %%Function:CmdAutoDelete %%Owner:???? */
CmdAutoDelete(pcaRM)
struct CA * pcaRM;
{
	int cmd = cmdOK;
	struct CA caT;
	struct CHP chpT;

#ifdef WIN
	if (selCur.fIns || !vpref.fAutoDelete)
		/* need to check for legal insertion (otherwise done in
			CmdDeleteSelCur) */
		{
		struct DOD *pdod = PdodDoc(selCur.doc);
		if ((pdod->fAtn || pdod->fFtn) &&
				!FCheckRefDoc(selCur.doc, selCur.cpFirst, selCur.cpFirst, 
				fTrue/*fReportErr*/))
			return cmdCancelled;
		}
	if (selCur.fIns)
		{
		*pcaRM = selCur.ca;
		}
	else
		{
		GetSelCurChp (fFalse);
		chpT = selCur.chp;
		if (!vpref.fAutoDelete)
			{
			PcaPoint(pcaRM, selCur.doc, selCur.cpFirst);
			goto LCont;
			}
/* the assumption here is that CmdAutoDelete is called in preparation of
a paste of a non-paragraph which can not replace the paragraph mark
at the end of the document; hence the rpkText parameter */
		if ((cmd = CmdDeleteSelCur(fFalse, fTrue, pcaRM, rpkText)) != cmdOK)
			{
LCont:
			FSetUndoDelete(PcaPoint(&caT, selCur.doc,
					selCur.cpFirst), fFalse);
			SelectIns (&selCur, selCur.cpFirst);
			}
		selCur.chp = chpT;
		selCur.fUpdateChp = fFalse; /* props are valid */
		AssureNestedPropsCorrect(pcaRM, fFalse);
		}
#else
	GetSelCurChp(fTrue);
	chpT = selCur.chp;
	if (selCur.sk == skGraphics)
		{
		/* set the chp properties and update fnPic */
		FetchCpAndParaCa(&selCur.ca, fcmProps);
		selCur.chp = vchpFetch;
		selCur.chp.fnPic = fnFetch;
		selCur.chp.fSpec = fFalse;
		}
	cmd = CmdDeleteSelCur(fFalse, fFalse, pcaRM, rpkText);
	selCur.chp = chpT;
	selCur.fUpdateChp = fFalse; /* props are valid */
	PcaPoint(pcaRM, pcaRM->doc, pcaRM->cpFirst);
#endif
	return cmd;
}


/* C M D  D E L E T E  S E L  C U R */
/* Delete selCur. Deleted text goes to scrap iff fScrap.
FSetUndoBefore and SetUndoAfter are not done.
pcaRM set to what's left after delete.
rpk specifies if the delete is complete (rpkNil) or a prelude for a
replecement by something non-paragraph (rpkText) so that the deletion of
the trailing paragraphs will not be enabled by FDeleteCheck.
*/
/*  %%Function:CmdDeleteSelCur %%Owner:bobz */
CmdDeleteSelCur(fScrap, fRM, pcaRM, rpk)
int fScrap, fRM;
struct CA *pcaRM;
{
	struct CA caT;

	/* Note: FDeleteCheck may change the selection if at the end
		of the doc, and we want the last para mark in the scrap and in
		undo but not cut.
	*/

	caT = selCur.ca;	/* use original ca for copy */
	PcaSetNil(pcaRM);
/* rpk can enable the deletion of trailing paragraphs in the document. Don't
do this with revision marking. */
	if (!FDeleteCheck(fFalse, fRM ? rpkText : rpk, NULL))
		return cmdError;
	if (!FSetUndoDelete(&selCur.ca, fScrap))
		return cmdCancelled;
	if (vmerr.fMemFail)
		return cmdNoMemory;
	if (fScrap)
		CmdCopy1(&selCur, &caT);
	if (vmerr.fMemFail) return cmdNoMemory;

	*pcaRM = selCur.ca;

	TurnOffSel(&selCur); /* avoid problems with unselecting after del */

	if (selCur.fTable)      /* Special case */
		return (CmdCutTable(&selCur, (fScrap) ? docScrap : docNil, fFalse));

	/* CmdCutTable will turn off sel itself */
	TurnOffSel(&selCur); /* avoid problems with unselecting after del */

	if (fRM)
		FDeleteRM(pcaRM);
	else
		{
		FDelete(pcaRM);
		pcaRM->cpLim = pcaRM->cpFirst;
		}
	return vmerr.fMemFail ? cmdNoMemory : cmdOK;
}


#ifdef WIN
/* Win version has restart footnote number for each section, therefore need this routine. */
/*  %%Function:InvalFtn %%Owner:chic */
InvalFtn(doc, cp)
int doc;
CP cp;
{
	struct DOD *pdod;
	struct PLC **hplc;
	int ifnd;

/* Check to see if have to invalidate footnote or not */

	Assert(doc != docNil);
	pdod = PdodDoc(doc);
	if (pdod->fMother && pdod->docFtn != docNil &&
			(hplc = pdod->hplcfrd) != hNil && pdod->dop.fFtnRestart &&
			(ifnd = IInPlcRef(hplc, cp)) >= 0 && ifnd < IMacPlc(hplc))
		InvalAutoReferences(doc, ifnd, edcDrpFtn);

}


#endif

/*  %%Function:CpTail %%Owner:davidlu */
CP CpTail(doc)
int doc;
{
	return (PdodDoc(doc)->fGuarded ? CpMacDocEdit(doc) :
			CpMacDoc(doc));
}


/*  %%Function:FSectLimAtCp %%Owner:davidlu */
EXPORT FSectLimAtCp(doc, cp)
int doc;
CP cp;
{
	int ised;
	CP cpSectLim;
	struct DOD *pdod = PdodDoc(doc);
	struct PLC **hplcsed;

	if (pdod->fShort || (hplcsed = pdod->hplcsed) == hNil)
		cpSectLim = CpMac1Doc(doc);
	else
		{
		ised = IInPlc(hplcsed, cp);
		cpSectLim = CpPlc(hplcsed, ised + 1);
		}
	return (cp == cpSectLim);
}


/* D I R T Y  E D L */
/* sets fDirty in hplcedl[dl] */
/*  %%Function:DirtyEdl %%Owner:tomsax */
DirtyEdl(hplcedl, dl)
struct PLCEDL **hplcedl; 
int dl;
{
	struct EDL edl;
	if (dl < 0) return;
	GetPlc(hplcedl, dl, &edl);
	edl.fDirty = fTrue;
	PutPlcLast(hplcedl, dl, &edl);
}


/* I D R  P R E V  F L O W */
/* returns idr of dr in current page previous in text flow.
returns idrNil, if no such dr.
*/
/*  %%Function:IdrPrevFlow %%Owner:chic */
IdrPrevFlow(hpldr, idr)
struct PLDR **hpldr;
{
	int idrT, idrMac;
	struct DR *pdr;
	struct DRF drfFetch;

	if (idr != 0)
		{
		idrMac = (*hpldr)->idrMac;
		for (idrT = 0; idrT < idrMac; FreePdrf(&drfFetch), idrT++)
			{
			pdr = PdrFetch(hpldr, idrT, &drfFetch);
			if (pdr->idrFlow == idr)
				{
				FreePdrf(&drfFetch);
				return(idrT);
				}
			}
		}
	return(idrNil);
}


/* F  C H E C K  R E F  D O C */
/*  %%Function:FCheckRefDoc %%Owner:chic */
FCheckRefDoc(doc, cpFirst, cpLim, fReportErr)
int doc;
CP cpFirst, cpLim;
int fReportErr;
{
	int edc = PdodDoc(doc)->fFtn ? edcDrpFtn : edcDrpAtn;

	Mac(Assert(edc == edcDrpFtn));
	if ((cpFirst >= CpMacDocEdit(doc)) ||
			CpRefFromCpSub(doc, cpFirst,edc) !=
			CpRefFromCpSub(doc, CpMax(cpFirst, cpLim - 1),edc) ||
			CpPlc(PdodDoc(doc)->hplcfnd, vifnd + 1) == cpLim)
		{
		if (fReportErr)
			ErrorEid(edc == edcDrpFtn ? eidInvalForFN : eidInvalForANN,"FCheckRefDoc");
		return fFalse;
		}
	return fTrue;
}


#ifdef WIN /* not used in MacWord */
/* F  M O V E  O K  F O R  T A B L E */
/*  Returns false if moving pca to doc, cp would be illegal because of table
	restrictions.
*/
/*  %%Function:FMoveOkForTable %%Owner:rosiep */
FMoveOkForTable(doc, cp, pca)
int doc;
CP cp;
struct CA *pca;
{

	if (FInTableDocCp(doc, cp))
		return (TckCkTextOkForTable(pca) == tckNil);
	else
		return fTrue;
}


/* A S S U R E  N E S T E D  P R O P S  C O R R E C T */
/*  Used to assure that text moved from one place to another has the
	properties that it needs for its new location.
	If doc, cpFirst is nested within a dead field, apply fFldVanish to
	cpFirst:cpLim.  If fTable, take care of table properties too.
*/
/*  %%Function:AssureNestedPropsCorrect %%Owner:rosiep */
AssureNestedPropsCorrect(pca, fTable)
struct CA *pca;
BOOL fTable;
{
	CP cpFirst, cpLim, cpCur;

	if (pca->doc == docNil)
		return;

	/* Check for the weirdo case of a dead field that got split, and fix it */
	cpCur = pca->cpFirst;

	/* We can't get this case anymore; FDeleteableEop now prevents it */
	Assert (!FDeadFieldSplitByTable(pca->doc, cpCur, &cpFirst));
#ifdef DEBUG
	if (FDeadFieldSplitByTable(pca->doc, cpCur, &cpFirst))
		{
		UnfieldifyDocCp(pca->doc, cpFirst);
		cpCur = cpFirst;
		return;
		}
#endif

	if (pca->cpFirst >= pca->cpLim)
		return;

	/*  check fFldVanish */
	if (FFldVanishDocCp (pca->doc, pca->cpFirst-1, fTrue/*fFetch*/) &&
			CpMatchFieldBack (pca->doc, pca->cpFirst, fTrue/*fDead*/, 0) != pca->cpFirst)
		/*  we are nested in a dead field */
		{
		Assert (FFldVanishDocCp (pca->doc, pca->cpLim, fFalse));
		ApplyFFldVanish (pca, fTrue/*fVanish*/);
		}
	else  if (FFldVanishDocCp (pca->doc, pca->cpFirst, fFalse)
			&& !(vchpFetch.fSpec && 
			*(vhpchFetch + (int)(pca->cpFirst-vcpFetch)) == chFieldBegin))
		/*  we are NOT in a dead field */
		{
		Assert (FFldVanishDocCp (pca->doc, pca->cpLim-1, fFalse));
		ApplyFFldVanish (pca, fFalse);
		}

	if (fTable && DcpCa(pca) > 0)
		{
		if (FInTableDocCpNoTap(pca->doc, pca->cpLim))
			ApplyTableProps(pca, fTrue);
		else  if (FTableMismatch(pca))
			ApplyTableProps(pca, fFalse);
		}

}


/* F  T A B L E  M I S M A T C H */
/* Return fTrue if there is any fInTable paragraph in pca but no fTtp
paragraph.  This is so paragraphs from inside a table, when moved to
a place outside of a table, will have the fInTable property turned off
by AssureNestedPropsCorrect or SetFieldResultProps.
*/
/*  %%Function:FTableMismatch %%Owner:rosiep */
FTableMismatch(pca)
struct CA *pca;
{
	CP cp = pca->cpFirst;
	CP cpLim = pca->cpLim;
	int doc = pca->doc;
	BOOL fTable = fFalse;
#ifdef DEBUG
	BOOL fChTable = fFalse;
#endif

	while (cp < cpLim)
		{
		CachePara(doc, cp);
		if (vpapFetch.fTtp)
			return fFalse;
		if (vpapFetch.fInTable)
			{
			fTable = fTrue;
#ifdef DEBUG
			if (ChFetch(doc, caPara.cpLim-1, fcmChars) == chTable)
				fChTable = fTrue;
#endif
			}
		cp = caPara.cpLim;
		}

	/* If we found a cell mark, we'd better have the whole row,
		so we would have found a Ttp and returned already. */
	Assert(!fChTable);

	return fTable;
}


/*  A P P L Y  T A B L E  P R O P S  */
/*  This routine should only be passed a pca that is entirely outside
	a table or entirely within a table cell.  It makes sure the fInTable
	property is turned on or off in the paragraphs within the pca, based
	on the parameter passed in.  In addition, it makes sure that fTtp is
	off.
*/
/*  %%Function:ApplyTableProps %%Owner:rosiep */
ApplyTableProps(pca, fInTable)
struct CA *pca;
BOOL fInTable;
{
	char prl[2];

	prl[0] = sprmPFInTable;
	Assert((fInTable & 0xfffe) == 0);
	prl[1] = fInTable;
	ApplyGrpprlCa(&prl, 2, pca);
}


/* T R A S H  F L T G */
/*  Invalidate the display of all fields in type group fltg in all docs.
*/
/*  %%Function:TrashFltg %%Owner:peterj */
TrashFltg (fltg)
int fltg;
{
	int doc;

	for (doc = docMinNormal; doc < docMac; doc++)
		{
		if (mpdochdod [doc] == hNil || PdodDoc (doc)->hplcfld == hNil)
			continue;
		TrashFltgDoc(doc, fltg);
		}

}


/* T R A S H  F L T G  D O C*/
/*  Invalidate the display of all fields in type group fltg in doc.
*/
/*  %%Function:TrashFltgDoc %%Owner:peterj */
TrashFltgDoc(doc, fltg)
int doc, fltg;
{
	int ifld;
	struct CA ca;
	struct FLCD flcd;
	BOOL fFirst = fTrue;

	PcaPoint( &ca, doc, cpNil );
	ifld = ifldNil;

	while ((ifld = IfldAfterFlcd (doc, ifld, &flcd)) != ifldNil)
		{
		if (FFltInFltg (flcd.flt, fltg))
			{
			if (fFirst)
				{
				int ww = wwNil;
				while ((ww = WwDisp(doc, ww, fFalse)) != wwNil)
					if (PwwdWw(ww)->fPageView)
						{
						InvalPageView(doc);
						break;
						}
				fFirst = fFalse;
				}
			if (ca.cpFirst == cpNil)
				{
				ca.cpFirst = flcd.cpFirst;
				ca.cpLim = flcd.cpFirst + flcd.dcpInst + flcd.dcpResult;
				}
			else  if (ca.cpLim < flcd.cpFirst)
				{
				CheckInvalCpFirst(&ca);
				InvalCp (&ca);
				ca.cpFirst = flcd.cpFirst;
				ca.cpLim = flcd.cpFirst + flcd.dcpInst + flcd.dcpResult;
				}
			}
		}

	if (ca.cpFirst != cpNil)
		{
		CheckInvalCpFirst(&ca);
		InvalCp (&ca);
		}
}


/*  %%Function:CheckInvalCpFirst %%Owner:peterj */
CheckInvalCpFirst (pca)
struct CA *pca;
{
	int ww;
	int doc = pca->doc;

	for  ( ww = wwNil ; (ww = WwDisp(doc,ww,fTrue)) != wwNil; )
		InvalHpldrCpFirst(HwwdWw(ww), doc, pca);
}


/*  %%Function:InvalHpldrCpFirst %%Owner:peterj */
InvalHpldrCpFirst(hpldr, doc, pca)
struct PLDR **hpldr;
int doc;
struct CA *pca;
{
	int idr, idrMac;
	struct EDL **hplcedl;
	int dl, dlMac;
	struct EDL edl;

	Assert(hpldr);
	idrMac = (*hpldr)->idrMac;

	for (idr = 0; idr < idrMac; idr++)
		{
		struct DR *pdr;
		struct DRF drfFetch;

		pdr = PdrFetch(hpldr, idr, &drfFetch);
		if (pdr->doc == doc && pdr->cpFirst > cp0 && FInCa(doc, pdr->cpFirst, pca))
			pdr->fCpBad = fTrue;
		if ((hplcedl = pdr->hplcedl) != hNil)
			{
			dlMac = IMacPlc(hplcedl);
			for (dl = 0; dl < dlMac; dl++)
				{
				GetPlc(hplcedl, dl, &edl);
				if (edl.hpldr != hNil)
					InvalHpldrCpFirst(edl.hpldr, doc, pca);
				}
			}
		FreePdrf(&drfFetch);
		}
}


#ifdef DEBUG
/*  F  D E A D  F I E L D  S P L I T  B Y  T A B L E  */
/*  Checks for the very unusual case of a dead field becoming split by
	a table (field begin char is in and field end char is not) which
	would occur if a user deletes a paragraph mark in a dead field if
	that para mark was followed by a table.  Returns true if this odd
	split happens.  Returns in *pcpFirst the cpFirst of the dead field
	(only guaranteed not to be garbage if fTrue is returned).
*/
/*  %%Function:FDeadFieldSplitByTable %%Owner:rosiep */
FDeadFieldSplitByTable(doc, cp, pcpFirst)
int doc;
CP cp, *pcpFirst;
{
	CP cpLim;

	if (!FNestedInDeadField(doc, cp) || !FInTableDocCp(doc, cp))
		return fFalse;

	/* check for a live field inside a dead field */
	if (IfldFromDocCp(doc, cp, fFalse) != ifldNil)
		return fFalse;

	*pcpFirst = CpMatchFieldBack(doc, cp, fTrue, 0);
	cpLim = CpLimDeadField(doc, *pcpFirst);
	return (FInTableDocCp(doc, cpLim-1) !=
			FInTableDocCp(doc, *pcpFirst));
}


#endif /* DEBUG */

/* I N V A L  S E Q  L E V  F I E L D S */
/*  Invalidate the display of all SEQLEV??? fields.
*/
/*  %%Function:InvalSeqLevFields %%Owner:peterj */
InvalSeqLevFields ()
{
	int doc;
	struct DOD *pdod, **hdod;
	struct PLC **hplcfld;
	extern int docSeqCache;

	vidf.fInvalSeqLev = fFalse;

	/*  invalidate the cache */
	docSeqCache = docNil;

		{{ /* NATIVE - InvalSeqLevFields */
		for (doc = docMinNormal; doc < docMac; doc++)
			if ((hdod = mpdochdod[doc]) != hNil && (pdod = *hdod)->fInvalSeqLev)
				{
				pdod->fInvalSeqLev = fFalse;
				if (pdod->fMother && (hplcfld = pdod->hplcfld) != hNil)
					{
					int ifld, ifldMac;
					int cFld;
					struct FLD fld;
					struct CA caT;

					for (cFld = 0, ifld = 0, ifldMac = IMacPlc(hplcfld); 
							ifld < ifldMac; ifld++)
						{
						GetPlc(hplcfld, ifld, &fld);

						if (fld.ch == chFieldBegin && 
								(uns)(fld.flt-fltSeqLevOut)<=(fltSeqLevNum-fltSeqLevOut))
							{
							if (cFld++ > 50)
								/* too many, just invalidate everything */
								{
								InvalCp(PcaSet(&caT, doc, caT.cpLim, 
										CpPlc(hplcfld, ifldMac-1)));
								break;
								}
							InvalCp (PcaSet(&caT, doc, CpPlc(hplcfld, ifld), CpPlc(hplcfld, ifld+1)));
							}
						}
					PdodDoc(doc)->fHasNoSeqLev = !cFld;
					}
				}
		}}
}


/* F  F I E L D S  I N  P C A */
/*  return fTrue if there are any field characters in pca */
/*  %%Function:FFieldsInPca %%Owner:peterj %%reviewed: 6/28/89 */
BOOL FFieldsInPca(pca)
struct CA *pca;
{
	struct PLC **hplcfld = PdodDoc(pca->doc)->hplcfld;

	return (hplcfld != hNil &&
			IInPlcRef(hplcfld, pca->cpFirst) <=
			IInPlcCheck(hplcfld, CpMax(0,pca->cpLim-1)));
}


#endif /* WIN */


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Editspec_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Editspec_Last() */
