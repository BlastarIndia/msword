/* I N S S U B S . C */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "file.h"
#include "props.h"
#include "fkp.h"
#include "ch.h"
#include "prm.h"
#include "sel.h"
#include "doc.h"
#include "debug.h"
#ifdef WIN
#include "heap.h"
#include "format.h"
#include "print.h"
#endif


#ifdef PROTOTYPE
#include "inssubs.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */
extern struct PRI       vpri;
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct FCB       **mpfnhfcb[];
extern struct DOD       **mpdochdod[];
extern struct MERR      vmerr;
extern int              vibp;
extern struct SEL       selCur;
extern struct ESPRM     dnsprm[];
extern struct SEP	vsepFetch;

extern PN PnAlloc();
#ifdef MAC
extern FC FcAppendRgchToFn();
#endif /* MAC */

extern struct FKPD      vfkpdPap;
extern struct FKPD      vfkpdChp;
extern struct FKPDT     vfkpdText;

#ifdef WIN 
#undef ScanFnForBytes
#define ScanFnForBytes(fn,hpch,cch,fWrite) \
		C_ScanFnForBytes(fn,hpch,cch,fWrite)

#endif /* WIN */

/* table lists sprms describing up to two fields in word iw of a structure,
in any order, Noop means only one field in word, Noop must be second in pair.
*/
struct SPX mpiwspxSep[] =
	{ 
	{ sprmSBkc, sprmSFTitlePage },
	
		{ sprmSCcolumns, sprmNoop },
	
		{ sprmSDxaColumns, sprmNoop },
	
#ifdef WIN
		{ sprmSNfcPgn, sprmNoop },
	
		{ sprmSPgnStart, sprmNoop },
	
		{ sprmNoop, sprmNoop },
	
#else
		{ sprmSFAutoPgn, sprmSNfcPgn },
	
		{ sprmSDyaPgn, sprmNoop },
	
		{ sprmSDxaPgn, sprmNoop },
	
#endif
		{ sprmSFPgnRestart, sprmSFEndnote },
	
		{ sprmSLnc, sprmSGrpfIhdt },
	
		{ sprmSNLnnMod, sprmNoop },
	
		{ sprmSDxaLnn, sprmNoop },
	
		{ sprmSDyaHdrTop, sprmNoop },
	
		{ sprmSDyaHdrBottom, sprmNoop },
	
#ifdef WIN
		{ sprmSLBetween, sprmSVjc },
	
		{ sprmSLnnMin, sprmNoop }
	};


#endif 


struct SPX mpiwspxPap[] =
	{
			/* note: we ignore sprmPStc in word 0 since the stc is always
			recorded in byte 1 of a PAPX. */
		{ sprmPJc,
			sprmNoop },
	
		{ sprmPFSideBySide, sprmPFKeep },
	
		{ sprmPFKeepFollow, sprmPFPageBreakBefore },
	
		{ sprmPBrcp, sprmPPc },
	
		{ sprmPBrcl, sprmPNfcSeqNumb },
	
		{ sprmPNoSeqNumb, sprmPFNoLineNumb },
	
		{ sprmPDxaRight, sprmNoop },
	
		{ sprmPDxaLeft, sprmNoop },
	
		{ sprmPDxaLeft1, sprmNoop },
	
		{ sprmPDyaLine, sprmNoop },
	
		{ sprmPDyaBefore, sprmNoop },
	
		{ sprmPDyaAfter, sprmNoop },
	
		{ sprmNoop, sprmNoop },
	
		{ sprmNoop, sprmNoop },
	
		{ sprmNoop, sprmNoop },
	
		{ sprmPFInTable, sprmPFTtp },
	
		{ sprmNoop, sprmNoop },
	
		{ sprmPDxaAbs, sprmNoop },
	
		{ sprmPDyaAbs, sprmNoop },
	
		{ sprmPDxaWidth, sprmNoop },
	
		{ sprmPBrcTop, sprmNoop },
	
		{ sprmPBrcLeft, sprmNoop },
	
		{ sprmPBrcBottom, sprmNoop },
	
		{ sprmPBrcRight, sprmNoop },
	
		{ sprmPBrcBetween, sprmNoop },
	
		{ sprmPBrcBar, sprmNoop },
	
		{ sprmPFromText, sprmNoop }
	};	



struct SPX mpiwspxTap[] =
	{
			{ sprmTJc, sprmNoop },
	
		{ sprmTDxaGapHalf, sprmNoop },
	
		{ sprmTDyaRowHeight, sprmNoop },
	
		{ sprmNoop, sprmNoop },
	
		{ sprmNoop, sprmNoop },
	
		{ sprmNoop, sprmNoop }
	};		


#ifdef MAC
/* Word 3 compatibility */
CS struct SPX mpiwspxSepEquivW3[] =
{ 
	{ sprmSBkcW3, sprmSFTitlePageW3 },
	
		{ sprmSCcolumnsW3, sprmNoop },
	
		{ sprmSDxaColumnsW3, sprmNoop },
	
		{ sprmSFAutoPgnW3, sprmSNfcPgnW3 },
	
		{ sprmSDyaPgnW3, sprmNoop },
	
		{ sprmSDxaPgnW3, sprmNoop },
	
		{ sprmSFPgnRestartW3, sprmSFEndnoteW3 },
	
		{ sprmSLncW3, sprmSGrpfIhdtW3 },
	
		{ sprmSNLnnModW3, sprmNoop },
	
		{ sprmSDxaLnnW3, sprmNoop },
	
		{ sprmSDyaHdrTopW3, sprmNoop },
	
		{ sprmSDyaHdrBottomW3, sprmNoop }
	};	


CS struct SPX mpiwspxPapW3[] =
{
			/* note: we ignore sprmPStc in word 0 since the stc is always
			recorded in byte 1 of a PAPX. */
		{ sprmPJc,
			sprmNoop },
	
		{ sprmPFSideBySide, sprmPFKeep },
	
		{ sprmPFKeepFollow, sprmPFPageBreakBefore },
	
		{ sprmPBrcp, sprmNoop },
	
		{ sprmPBrcl, sprmPNfcSeqNumb },
	
		{ sprmPNoSeqNumb, sprmPFNoLineNumb },
	
		{ sprmPDxaRight, sprmNoop },
	
		{ sprmPDxaLeft, sprmNoop },
	
		{ sprmPDxaLeft1, sprmNoop },
	
		{ sprmPDyaLine, sprmNoop },
	
		{ sprmPDyaBefore, sprmNoop },
	
		{ sprmPDyaAfter, sprmNoop }
	};	


CS struct SPX mpiwspxPapEquivW3[] =
{
			/* note: we ignore sprmPStc in word 0 since the stc is always
			recorded in byte 1 of a PAPX. */
		{ sprmPJcW3,
			 sprmNoop },
	
		{ sprmPFTableW3, sprmPFKeepW3 },
	
		{ sprmPFKeepFollowW3, sprmPFPageBreakBeforeW3 },
	
		{ sprmPBrcpW3, sprmNoop },
	
		{ sprmPBrclW3, sprmPNfcSeqNumbW3 },
	
		{ sprmPNoSeqNumbW3, sprmPFNoLineNumbW3 },
	
		{ sprmPDxaRightW3, sprmNoop },
	
		{ sprmPDxaLeftW3, sprmNoop },
	
		{ sprmPDxaLeft1W3, sprmNoop },
	
		{ sprmPDyaLineW3, sprmNoop },
	
		{ sprmPDyaBeforeW3, sprmNoop },
	
		{ sprmPDyaAfterW3, sprmNoop }
	};	


#endif



/* S E T  F N  P O S */
/* %%Function:SetFnPos %%Owner:davidlu */
SetFnPos(fn, fc)
int     fn;
FC      fc;
{
	struct FCB *pfcb = *mpfnhfcb[fn];
	if (fc > pfcb->cbMac)
		pfcb->cbMac = fc;
	pfcb->fcPos = fc;
}


#ifdef DEBUGORNOTWIN
/* F N E W  C H P  I N S */
/* Make forthcoming inserted characters have the look in pchp.
Plan:
	ensure vchpStc set.
	compute chpT that steps vchpStc to *pchp.
	if different then current run, start new run.
*/
#ifdef MAC
/* %%Function:FNewChpIns %%Owner:davidlu */
FNewChpIns(doc, cp, pchp, stc)
#else /* !MAC */
/* %%Function:C_FNewChpIns %%Owner:davidlu */
HANDNATIVE C_FNewChpIns(doc, cp, pchp, stc)
#endif /* !MAC */
int doc; 
CP cp;
struct CHP *pchp;
int stc;
{
	int cb;
	struct CHP chpT;
	FC fcMac;
	struct CHP *pchpBase;
	struct CHP chpBase;
	struct PAP papT;

	if (stc == stcNil)
		{
		CachePara(doc, cp);
		pchpBase = &vchpStc;
		}
	else
		{
		MapStc(PdodDoc(doc), stc, &chpBase, &papT);
		pchpBase = &chpBase;
		}

	CbGenChpxFromChp(&chpT, pchp, pchpBase, fFalse);
	if (FNeRgw(&vfkpdChp.chp, &chpT, cwCHP))
		{ /* Add the run for the previous insertion; our looks differ. */
		fcMac = (**mpfnhfcb[fnScratch]).cbMac;
		cb = CchNonZeroPrefix(&vfkpdChp.chp, cbCHP);
		if (vfkpdChp.fcFirst < vfkpdText.fcLim)
			{
			if (!FAddRun(fnScratch,
					vfkpdText.fcLim,
					&vfkpdChp.chp,
					cb,
					&vfkpdChp,
					fFalse /* CHP run */,
					fTrue /* allocate new pns at fcMac */,
					fTrue /* plcbte must expand */,
					fFalse /* not Word3 format */))
				return fFalse;
			}
		vfkpdChp.chp = chpT;
		}
	return fTrue;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* F I N S E R T  R G C H */
/* Insert cch characters from rgch into doc before cp with props pchp.
A chEop may be inserted as the last char in rgch if ppap != 0.
*/
#ifdef MAC
/* %%Function:FInsertRgch %%Owner: davidlu */
FInsertRgch(doc, cp, rgch, cch, pchp, ppap)
#else /* !MAC */
/* %%Function:C_FInsertRgch %%Owner:davidlu */
HANDNATIVE C_FInsertRgch(doc, cp, rgch, cch, pchp, ppap)
#endif /* !MAC */
int doc, cch;
CP cp;
char rgch[];
struct CHP *pchp;
struct PAP *ppap;
{
	int cchPapx;
	CP cpM1;
	FC fc;
	struct CHP chp;
	struct PAP papStd;
	struct TAP tapStd;
	struct CA caT;
	char papx[cchPapxMax];

	/* First finish off the previous char run if necessary */
	if (pchp == 0)
		{
		GetPchpDocCpFIns(&chp, doc, cp, fTrue, wwNil);
		pchp = &chp;
		}

	/* set insertion point properties */
	if (!FNewChpIns(doc, cp, pchp, (ppap == 0) ? stcNil : ppap->stc))
		return fFalse;

	/* Now write the characters to the scratch file */
	fc = FcAppendRgchToFn(fnScratch, rgch, cch);
	if (vmerr.fDiskEmerg)
		return fFalse;

	/* Now insert a paragraph run if we inserted an EOL */
	if (ppap != 0)
		{ /* Inserting EOL--must be last character of rgch */
#ifdef WIN
		Assert( rgch [cch-1] == chEol || rgch [cch-1] == chSect ||
				rgch [cch-1] == chTable );
#endif
		MapStc(*mpdochdod[doc], ppap->stc, 0, &papStd);
		cchPapx = CbGenPapxFromPap(&papx, ppap, &papStd, fFalse);
		if (ppap->fTtp)
			{
			SetWords(&tapStd, 0, cwTAP);
			cchPapx = CbAppendTapPropsToPapx(&papx, cchPapx, ppap->ptap, &tapStd, cchPapxMax - cchPapx);
			}
		if (!FAddRun(fnScratch,
				vfkpdText.fcLim,
				&papx,
				cchPapx,
				&vfkpdPap,
				fTrue /* para run */,
				fTrue /* alloc at fcMac */,
				fTrue /* plcbte must expand */,
				fFalse /* not Word 3 format */))
			return fFalse;
		}

	/* Finally, insert the piece into the document */
	return FReplace(PcaPoint(&caT, doc, cp), fnScratch, fc, (FC) cch);

}


#endif /* DEBUGORNOTWIN */


#ifdef WIN /* not used in Mac Word */
/* I N S E R T  M U L T I  L I N E  R G C H */
/* Same as InsertRgch, but deals with chEop anywhere in rgch.  (If
ppap==NULL, uses vpapFetch at doc,cp).
*/

/* %%Function:InsertMultiLineRgch %%Owner:davidlu */
InsertMultiLineRgch(doc, cp, rgch, cch, pchp, ppap)
int doc, cch;
CP cp;
char rgch[];
struct CHP *pchp;
struct PAP *ppap;
{
	CHAR *pch, *pchLast, *pchBase;
	struct PAP pap;

	if (ppap == NULL)
		{
		extern struct PAP vpapFetch;
		CachePara(doc, cp);
		pap = vpapFetch;
		ppap = &pap;
		}

	for (pchBase = pch = rgch, pchLast = pch + cch - 1; pch <= pchLast; pch++)
		if (*pch == chEol || pch == pchLast)
			{
			int cchT = pch - pchBase + 1;
			if (!FInsertRgch(doc, cp, pchBase, cchT, pchp, 
					(*pch == chEol) ? ppap : NULL))
				return;
			cp += cchT;
			pchBase = pch + 1;
			}
}


#endif /* WIN */


#ifdef DEBUGORNOTWIN
#ifdef MAC
/* %%Function: AddRun %%Owner: davidlu */
NATIVE AddRun(fn, fcLim, pchProp, cchProp, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) /* WINIGNORE - MAC only */
#else /* WIN */
/* %%Function:C_AddRun %%Owner:davidlu */
HANDNATIVE C_AddRun(fn, fcLim, pchProp, cchProp, pfkpd, fPara, fAllocMac, fPlcMustExp)
#endif /* WIN */
int fn; 
FC fcLim; 
char *pchProp; 
int  cchProp;
struct FKPD *pfkpd; 
int fPara; 
int fAllocMac; 
int fPlcMustExp;
#ifdef MAC
int fWord3;
#endif /* MAC */
{
	AssertDo(FAddRun(fn, fcLim, pchProp, cchProp, pfkpd, fPara, 
			fAllocMac, fPlcMustExp, fWord3));
}


#endif /* DEBUGORNOTWIN */

#ifdef WIN
#ifdef DEBUG
/* %%Function:FAddRunCkProc %%Owner:davidlu */
NATIVE FAddRunCkProc(fn, fcLim, pchProp, cchProp, pfkpd, fPara, fAllocMac, fPlcMustExp) /* WINIGNORE - DEBUG only */
int fn;                 /* file */
FC fcLim;               /* last file character described by this run */
char *pchProp;          /* pointer to PAPX or CHPX to add */
int  cchProp;           /* size of PAPX or CHPX */
struct FKPD *pfkpd;     /* Contains data about current fkp - current FKP page,
run insertion point, property insertion point. */
int fPara;              /* fTrue if adding PAP, otherwise adding CHP */
int fAllocMac;          /* when fTrue allocate new pn at file's fcMac. when
false allocate new pn using file's current fcPos.*/
int fPlcMustExp;        /* when fTrue, will cause fFalse return whenever it
is impossible to expand the hplcbte.  */
{
	int grpf = (fPara << 2) + (fAllocMac << 1) + fPlcMustExp;

	Assert((fPara & 0xFFFE) == 0);
	Assert((fAllocMac & 0xFFFE) == 0);
	Assert((fPlcMustExp & 0xFFFE) == 0);
	return (S_FAddRun(fn, fcLim, pchProp, cchProp, pfkpd, grpf));
}


#endif /* DEBUG */
#endif /* WIN */

#ifdef DEBUGORNOTWIN
/* F  A D D  R U N */
#ifdef MAC
/* %%Function:FAddRun %%Owner:davidlu */
NATIVE FAddRun(fn, fcLim, pchProp, cchProp, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) /* WINIGNORE - MAC only */
#else /* WIN */
/* %%Function:C_FAddRun %%Owner:davidlu */
HANDNATIVE C_FAddRun(fn, fcLim, pchProp, cchProp, pfkpd, grpf)
#endif /* WIN */
int fn;                 /* file */
FC fcLim;               /* last file character described by this run */
char *pchProp;          /* pointer to PAPX or CHPX to add */
int  cchProp;           /* size of PAPX or CHPX */
struct FKPD *pfkpd;     /* Contains data about current fkp - current FKP page,
run insertion point, property insertion point. */
#ifdef MAC
int fPara;              /* fTrue if adding PAP, otherwise adding CHP */
int fAllocMac;          /* when fTrue allocate new pn at file's fcMac. when
false allocate new pn using file's current fcPos.*/
int fPlcMustExp;        /* when fTrue, will cause fFalse return whenever it
is impossible to expand the hplcbte.  */
int fWord3;             /* when true create a FKP whose length is 128 bytes
as expected by Word 3.0 */
#else /* WIN */
int grpf;
#endif /* WIN */
	{ /*
	DESCRIPTION:
	Add a char or para run to an fn.
	Use, and update, the information in the pertinent FKPD (Formatted disK Page
	Descriptor).
*/
#ifdef WIN
	int fPara = grpf & 4;
	int fAllocMac = grpf & 2;
	int fPlcMustExp = grpf & 1;
#endif /* WIN */
	struct FKP HUGE *hpfkp;
	struct FCB *pfcb;
	int cchPropShare; /* = cchProp or 0 if shared */
	struct PLCBTE **hplcbte;
	FC fcFirst;
	char HUGE *hpch;
	int bCur;
	int bNewLim;
	int crun;
	int cbInc;
	char HUGE *hpcrun, HUGE *hpchPropLim;
	int cbPage;
	int cchPropTot;
	int cchStored;
	int fStoreCw = WinMac(fPara, !fWord3 && fPara);
	char HUGE *hpchFprop;
	char HUGE *hpbFirst;
	char HUGE *hpb;

	pfcb = *mpfnhfcb[fn];
	hplcbte = fPara ? pfcb->hplcbtePap :
			pfcb->hplcbteChp;

	if (fn == fnScratch && vmerr.fDiskEmerg)
		return fFalse;

#ifdef MAC
	cbPage = (!fWord3) ? cbSector : cbSectorPre35;
#else /* WIN */
	cbPage = cbSector;
#endif /* WIN */
	if (pfkpd->bFreeLim == 0)
		goto LNewPage; /* initial state */
	else
		{
/* if this is a dry run, just use the passed dummy page */
#ifdef MAC
		hpfkp = (!fWord3) ? (struct FKP HUGE *)HpchGetPn(fn, pfkpd->pn) :
				(struct FKP HUGE *)HpchGetPo(fn, pfkpd->pn);
#else /* WIN */
		hpfkp = (struct FKP HUGE *)HpchGetPn(fn, pfkpd->pn);
#endif /* WIN */
		}
	Assert(((int)hpfkp & 1) == 0);
/* this loop searches for a the current or next page that has enough room */
	for (;;)
		{
#ifdef MAC
		if (!fWord3)
#endif /* MAC */
			crun = *(hpcrun = hpchPropLim = &hpfkp->crun);
#ifdef MAC
		else
			{
			crun = *(hpcrun = &((struct FKPO HUGE *)hpfkp)->crun);
			hpchPropLim = hpcrun - 1;
			}
#endif /* MAC */
		/* If cchProp is zero, pchProp is the standard property.
			Don't add prop to FKP, just add a run */
		cchPropShare = cchProp;
		if (cchPropShare == 0)
			bCur = bNil;
		else
			{
			/* Attempt to have the present run "share" a property by
				searching through all of the existing props in the FKP
				page for an identical copy of pchProp.
			*/
/* if we must store a cw at the front of a PAPX, make sure we pad the
	papx to an even length. */
			if (fStoreCw && (cchPropShare & 1) != 0)
				{
/* for the comparison in the loop to work correctly, the papx stored at
	pchProp must be followed by a byte of 0 */
				Assert(*(pchProp + cchProp) == 0);
				cchPropShare += 1;
				}
			if (crun > 0)
				{
				hpch = (char HUGE *)hpfkp + pfkpd->bFreeLim;
				while (hpch < hpchPropLim)
					{
					cchStored = *hpch;
					if (fStoreCw)
						cchStored <<= 1;
					if ((cchStored == cchPropShare) &&
							!FNeHprgch((char HUGE *)pchProp, hpch + 1,
							cchPropShare))
						{ /* share existing property */
						bCur = hpch - hpfkp;
						cchPropShare = 0;
						break;  /* exit while */
						}
					/* inc. to look at next property */
					hpch += (cbInc = (cchStored + 1));
/* in the new format the properties must begin on word boundaries. */
#ifdef MAC
					if (!fWord3 && ((long)hpch & 1) != 0)
#else /* WIN */
						if (((long)hpch & 1) != 0)
#endif /* WIN */
							{
							*hpch++;
							}
					}       /* end while (properties remaining in FKP) */
				}
			}       /* end else (cchPropShare > 0) */


		cchPropTot = (cchPropShare > 0) ? cchPropShare + 1 : 0;
		/* If RUN and FPAP/FCHP (PAPX/CHPX with prepended one byte cch) will
			NOT fit on this FKP page, set allocate new page */
		bNewLim = pfkpd->bFreeLim - cchPropTot;
#ifdef MAC
		if (!fWord3 && cchPropTot != 0 && (bNewLim & 1) != 0)
#else /* WIN */
			if (cchPropTot != 0 && (bNewLim & 1) != 0)
#endif /* WIN */
				bNewLim--;
		if (pfkpd->bFreeFirst + sizeof(FC) + 1 > bNewLim)
			{
			int pn;
LNewPage: 
			fcFirst = pfkpd->fcFirst;
			/* if no dry run, get the pointer to the proper bin table,
				and get the bin table Mac. Otherwise claim we have 1
				bin table entry for a dry run. */

			if (fn == fnScratch)
				{
/* check if partial page should be stored for future use */
				FC fcMac;
				int pn = PnFromFc(fcMac = pfcb->cbMac);
				int b;
				if ((b = (fcMac & (cbSector - 1))))
					{
					vfkpdText.bFreeFirst = b;
					vfkpdText.pn = pn;
					}
				}

/* PnAlloc is divided into allocate and increment phases so that HpchGetPn
get operate efficiently beyond the Eof */
#ifdef MAC
			pn = PnAlloc1(fn, fAllocMac, fWord3);
#else /* WIN */
			pn = PnAlloc1(fn, fAllocMac);
#endif /* WIN */
			if (fPlcMustExp || !pfkpd->fPlcIncomplete)
				{
				if (FRareT(600,!FInsertInPlc(hplcbte, IMacPlc(hplcbte),
						fcFirst, &pn)))
					{
					if (fPlcMustExp)
						return fFalse;
					else
						pfkpd->fPlcIncomplete = fTrue;
					}
				}
			pfkpd->pn = pn;

#ifdef MAC
			hpfkp = (!fWord3) ? (struct FKP HUGE *)HpchGetPn(fn, pfkpd->pn) :
					(struct FKP HUGE *)HpchGetPo(fn, pfkpd->pn);
#else
			hpfkp = (struct FKP HUGE *)HpchGetPn(fn, pfkpd->pn);
#endif /* MAC */

#ifdef MAC
			PnAlloc2(fn, pfkpd->pn, fAllocMac, fWord3);
#else /* WIN */
			PnAlloc2(fn, pfkpd->pn, fAllocMac);
#endif /* WIN */
			bltbcx(0, LpFromHp(hpfkp), cbPage);
			hpfkp->rgfc[0] = fcFirst;
			pfkpd->bFreeFirst = sizeof(FC);
			/* pfkp->crun = 0 */
			pfkpd->bFreeLim = cbPage - 1;
			continue;
			}
		if (cchPropShare > 0)
			{
		/* store papx or chpx below bFreeLim */
			bCur = pfkpd->bFreeLim = bNewLim;
			hpchFprop = (char HUGE *) hpfkp + bCur;
			*hpchFprop = (!fStoreCw) ? cchProp : (cchPropShare >> 1);
			bltbh(pchProp, hpchFprop + 1, cchProp);
			}
		/* create a run */
		hpbFirst = (char HUGE *) &hpfkp->rgfc[crun + 1];
		bltbh(hpbFirst, hpbFirst + sizeof(FC), crun);
#ifdef MAC
		*(hpbFirst + sizeof(FC) + crun) = (!fWord3) ? (bCur / sizeof(int)) : bCur;
#else /* WIN */
		*(hpbFirst + sizeof(FC) + crun) = bCur / sizeof(int);
#endif /* WIN */
		*((FC HUGE *)hpbFirst) = fcLim;
		pfkpd->bFreeFirst += sizeof(FC) + 1;
		(*hpcrun)++;
		Assert(pfkpd->bFreeFirst == *hpcrun * (sizeof(FC) + 1) +
				sizeof(FC));
		pfkpd->fcFirst = fcLim;
		SetDirty(vibp);
		PutCpPlc(hplcbte, IMacPlc(hplcbte), fcLim);
		return (fn != fnScratch || !vmerr.fDiskEmerg);
		}
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* F C  A P P E N D  R G C H  T O  F N */
/* Appends characters pointed to by pch, length cch, to end of file fn.
Returns first fc written.
If there is a free hole of sufficient size in vfkpdText, it will be
used instead.
fcLim updated to point to after the last char written.
*/
#ifdef MAC
/* %%Function:FcAppendRgchToFn %%Owner:davidlu */
FC FcAppendRgchToFn(fn, pch, cch)
#else /* !MAC */
/* %%Function:C_FcAppendRgchToFn %%Owner:davidlu */
HANDNATIVE FC C_FcAppendRgchToFn(fn, pch, cch)
#endif /* !MAC */
int fn;
char *pch;
int cch;
{
	FC fc = (*mpfnhfcb[fn])->cbMac;
	if (fn == fnScratch && vfkpdText.pn != pnNil
			&& (cbSector - vfkpdText.bFreeFirst) >= cch)
		{
		fc = FcFromPn(vfkpdText.pn) + vfkpdText.bFreeFirst;
		vfkpdText.bFreeFirst += cch;
		}
	else
/* maintain fc's monotonically increasing */
		vfkpdText.pn = pnNil;
	if (fn == fnScratch)
		vfkpdText.fcLim = fc + cch;

	SetFnPos(fn, fc);
	WriteRgchToFn(fn, pch, cch);
	return (fc);
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC /* in savetext.c if WIN */
/* F C  A P P E N D  H P R G C H  T O  F N */
/* Appends characters pointed to by hpch, length cch, to end of file fn.
Returns first fc written. */

/* %%Function:FcAppendHprgchToFn %%Owner:davidlu */
NATIVE FC FcAppendHprgchToFn(fn, hpch, cch) /* WINIGNORE - MAC only */
int fn;
char HUGE *hpch;
int cch;
{
	struct FCB *pfcb = *mpfnhfcb[fn];
	pfcb->fcPos = pfcb->cbMac;
	WriteHprgchToFn(fn, hpch, cch);
	return (pfcb->cbMac);
}


#endif /* MAC */


#ifdef DEBUGORNOTWIN
/* S C A N   F N   F O R   B Y T E S */
/* Beginning with the current fcPos for fn, cch bytes of fn are paged into
	memory. If fWrite is true the contents of the rgch pointed to by pch
	are copied to the file. If fWrite is false, the rgch pointed to by pch
	is filled with data read from the file. */
/* %%Function:ScanFnForBytes %%Owner:davidlu */
HANDNATIVE ScanFnForBytes(fn, hpch, cch, fWrite)
int fn;
char HUGE *hpch;
uns cch;
int fWrite;
{
	struct FCB *pfcb = *mpfnhfcb[fn];
	FC fc    = pfcb->fcPos;
	FC fcCur = fc;
	PN pnCur = PnFromFc(fcCur);

	/* write range of characters out to file page cache memory.  First
		fill the current last page before adding new pages to the end of
		the file. */
	while (cch > 0)
		{
		char HUGE *hpchBp = HpchGetPn(fn, pnCur);
		uns cchBp;
		uns cchBlt;

		cchBp = fcCur & ((long) maskSector);
		cchBlt = umin(cbSector - cchBp, cch);

		if (fWrite)
			{
			bltbh(hpch, hpchBp + cchBp, cchBlt);
			SetDirty(vibp);
			}
		else
			bltbh(hpchBp + cchBp, hpch, cchBlt);
		fcCur += cchBlt;
		if (fcCur > pfcb->cbMac)
			{
			Assert(fWrite);
			if (fWrite)
				pfcb->cbMac = fcCur;
			}
		pnCur++;
		hpch += cchBlt;
		cch -= cchBlt;
		}       /* end while (characters still to be written) */
	pfcb->fcPos = fcCur;
}


#endif /* DEBUGORNOTWIN */



#ifdef DEBUGORNOTWIN
/* C B	G R P P R L  P R O P */
/* generates a list of prl's that expresses the differences of prop
from propBase, when the prop's bytes are encoded according to
mpiwspx (which describes the sprm describing the two bytes of word iw in
prop.)
fNorm means that the sprm's must be sorted in normal order (necessary
if grpprl is to go in a prc, not necessary if grpprl is to be aprt of a
papx or sepx.)
Does not do tbd's in pap's (see below.)
*/

/* OPUS comment: mpiwspx will need to be a FAR variable if we change the
	structures to be stored as csconst */

#ifdef MAC
/* %%Function:CbGrpprlProp %%Owner:davidlu */
NATIVE int CbGrpprlProp(fNorm, pgrpprl, cbMax, pprop, ppropBase, cwProp, mpiwspx, mpiwspxW3) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_CbGrpprlProp %%Owner:davidlu */
HANDNATIVE int C_CbGrpprlProp(fNorm, pgrpprl, cbMax, pprop, ppropBase, cwProp, mpiwspx)
#endif /* !MAC */
BOOL fNorm;
char *pgrpprl;
int cbMax;
char *pprop;
char *ppropBase;
int cwProp;
struct SPX mpiwspx[];
#ifdef MAC
struct SPX mpiwspxW3[];
#endif /* MAC */
{
	struct SPX *pspx;
	int *pw;
	int *pwBase;
	int iw;
	int isprm, sprm;
	int cbPrl;
	char rgb[3];
	struct SEBL sebl;

	sebl.cbEarlier = sebl.cbLater = sebl.cbMerge = 0;
	sebl.cbMergeMax = cbMax;
	sebl.pgrpprlLater = rgb;
	sebl.pgrpprlMerge = pgrpprl;

	for (iw = 0, pw = (int *)pprop, pwBase = (int *) ppropBase;
			iw < cwProp; iw++, pw++, pwBase++)
		{
		if (*pw != *pwBase)
			{
			pspx = &mpiwspx[iw];
			for (isprm = 0; isprm < 2 &&
					(sprm = pspx->rgsprm[isprm]) != sprmNoop;
					isprm++)
				if (cbPrl = CbGenPrl(pprop, ppropBase, sprm, rgb))
					{
#ifdef MAC
					if (mpiwspxW3)
						rgb[0] = mpiwspxW3[iw].rgsprm[isprm];
#endif /* MAC */
					if (fNorm)
						AddPrlSorted(&sebl, &rgb[0], sebl.cbLater = cbPrl);
					else
						{
						sebl.pgrpprlMerge =
								bltbyte(&rgb[0], sebl.pgrpprlMerge, cbPrl);
						sebl.cbMerge += cbPrl;
						}
					}
			}
		}
	return sebl.cbMerge;
}


#endif /* DEBUGORNOTWIN */

#ifdef DEBUGORNOTWIN
/* C B  G E N  P R L */
/* return prl if values described does not match base value.
prl is put in rgb, cb of prl is returned.
If values match, return 0.
*/
#ifdef MAC
/* %%Function:CbGenPrl %%Owner:davidlu */
NATIVE int CbGenPrl(pprop, ppropBase, sprm, rgb) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_CbGenPrl %%Owner:davidlu */
HANDNATIVE int C_CbGenPrl(pprop, ppropBase, sprm, rgb)
#endif /* !MAC */
char *pprop;
char *ppropBase;
int sprm;
char rgb[];
{
	int val;
	int cbPrl;

	val = ValFromPropSprm(pprop, sprm);
	if (val != ValFromPropSprm(ppropBase, sprm))
		{
		rgb[0] = sprm;
		cbPrl = dnsprm[sprm].cch;
		Assert(cbPrl == 2 || cbPrl == 3);
		if (cbPrl == 2)
			rgb[1] = val;
		else
			bltb(&val, &rgb[1], 2);
		return cbPrl;
		}
	return 0;
}


#endif /* DEBUGORNOTWIN */

#define itbdMax30 30

/* C B  G R P P R L  F R O M  P A P */
/* does the CbGrpprlProps, and also tbd's */
/* %%Function:CbGrpprlFromPap %%Owner:davidlu */
NATIVE int CbGrpprlFromPap(fNorm, pgrpprl, ppap, ppapBase, fWord3)
BOOL fNorm;	/* when fTrue, grpprl is to be applied to piece. 
		when fFalse, grpprl is to be stored in PAPX */
char *pgrpprl;
struct PAP *ppap;
struct PAP *ppapBase;
int fWord3;
{
	int     itbdMac;
	int     itbdAddMac;
	int     itbdDelMac;
	int     dxaLater;
	int     itbdDel;
	int     itbdAdd;
	int     cbPrl;
	int     cb;
	int     cw;
	int     *pdxaEarlier;
	int     *pdxaEarlierMac;
	char    *ptbdEarlier;
	int     *pdxaLater;
	int     *pdxaLaterMac;
	char    *ptbdLater;
	int     rgdxaAdd[itbdMax];
	char    rgtbdAdd[itbdMax];
	int     rgdxaDel[itbdMax];
	struct SPX *mpiwspx;
	struct SPX *mpiwspxEquiv;

#ifdef WIN
	cb = CbGrpprlProp(fNorm, pgrpprl, cchPapxMax, ppap, ppapBase,
			cwPAPBaseScan, mpiwspxPap, 0);
#else
	if (!fWord3)
		{
		mpiwspx = mpiwspxPap;
		mpiwspxEquiv = 0;
		cw = cwPAPBaseScan;
		}
	else
		{
		mpiwspx = mpiwspxPapW3;
		mpiwspxEquiv = mpiwspxPapEquivW3;
		cw = cwPAPBaseScanW3;
		}
	cb = CbGrpprlProp(fNorm, pgrpprl, cchPapxMax, ppap, ppapBase,
			cw, mpiwspx, mpiwspxEquiv);
#endif

	if ((itbdMac = ppap->itbdMac) == ppapBase->itbdMac)
		{
/* return if tab tables are equal */
		if (!FNeRgch((char *)ppap->rgdxaTab, (char *)ppapBase->rgdxaTab,
				itbdMac * sizeof(int)) &&
				!FNeRgch((char *)ppap->rgtbd, (char *)ppapBase->rgtbd,
				itbdMac))
			return cb;
		}
		{{ /* !NATIVE - CbGrpprlFromPap */
/* generate differential tab tables */
		itbdAddMac = 0;
		itbdDelMac = 0;

		pdxaEarlier = ppapBase->rgdxaTab;
		pdxaEarlierMac = pdxaEarlier + ppapBase->itbdMac;
		ptbdEarlier = ppapBase->rgtbd;

		pdxaLater = ppap->rgdxaTab;
		pdxaLaterMac = pdxaLater + ppap->itbdMac;
		ptbdLater = ppap->rgtbd;

		for (;;)
			{
			if (pdxaEarlier < pdxaEarlierMac)
/* E not empty */
				{
				if (pdxaLater < pdxaLaterMac)
					{
/* E not empty && L not empty */
					bltb(pdxaLater, &dxaLater, 2);
					if (!fNorm ? (*pdxaEarlier == dxaLater) :
							FCloseXa(*pdxaEarlier, dxaLater))
						{
/* L and E exactly match, then don't include either in exception sprm */
						int fTbdNE = (*ptbdEarlier != *ptbdLater);
						pdxaEarlier++;
						ptbdEarlier++;
/* if tbd are not equal, then just copy the later */
						if (fTbdNE)
							goto LL;
						pdxaLater++;
						ptbdLater++;
						continue;
						}
					else  if (*pdxaEarlier < dxaLater)
						{
/* E is the result */
LE:                                     
						itbdDel = itbdDelMac++;
						Assert(itbdDelMac <= itbdMax);
						rgdxaDel[itbdDel] = *pdxaEarlier++;
						Assert(itbdDel == 0 || rgdxaDel[itbdDel] > rgdxaDel[itbdDel - 1]);
						*ptbdEarlier++;
						continue;
						}
					else
						{
/* L is the result */
LL:                                     
						itbdAdd = itbdAddMac++;
						Assert(itbdAddMac <= itbdMax);
						rgdxaAdd[itbdAdd] =  *pdxaLater++;
						Assert(itbdAdd == 0 || rgdxaAdd[itbdAdd] > rgdxaAdd[itbdAdd - 1]);
						rgtbdAdd[itbdAdd] = *ptbdLater++;
						continue;
						}
					}
/* E not empty, L empty */
				else
					goto LE;
				}
else if /* E is empty */
			(pdxaLater < pdxaLaterMac)
/* E is empty, L is not empty */
				goto LL;
			else
/* E and L both empty, so done */
				break;

			}

		pgrpprl += cb;

		if (itbdAddMac || itbdDelMac)
			{
			int cbExcess;
			char *pchGrpprl;
			struct SEBL sebl;
			char rgb[cbMaxPChgTabs];

			pchGrpprl = fNorm ? rgb : pgrpprl;
			sebl.cbMerge = cb;

#ifdef MAC
			Assert(!fWord3 || !fNorm);
			if (fWord3)
				{
				*pchGrpprl++ = sprmPChgTabsW3;
				itbdAddMac = min(itbdAddMac, itbdMax30);
				itbdDelMac = min(itbdDelMac, itbdMax30);
				}
			else
#endif /* MAC */
				*pchGrpprl++ = (!fNorm) ? sprmPChgTabsPapx : sprmPChgTabs;

/* the delete array will always be included as part of the sprmPChgTabsPapx prl */
			cb += 2 + ((!fNorm) ? 2 : 4) * itbdDelMac;
/* if including the add array as part of the sprmPChgTabsPapx prl would cause
	the generation of a papx that is too large to store in a papx, throw
	away entries from the end of the add array so that the prl fits. */
			if (!fNorm &&
					(cbExcess = cb + 2 + 3 * itbdAddMac - cbMaxGrpprlPapx) > 0)
				{
				itbdAddMac = max(0, itbdAddMac - (cbExcess + 2) / 3);
				}
			cb += 3 * itbdAddMac;
			Assert(fNorm || cb - sebl.cbMerge <= 255);
			*pchGrpprl++ = min (255, cb - sebl.cbMerge);
			cb += 2;
			Assert( cb < cbMaxGrpprlPapx );
			*pchGrpprl++ = itbdDelMac;
			if (itbdDelMac != 0)
				{
				pchGrpprl = bltbyte(rgdxaDel, pchGrpprl, itbdDelMac * 2);
				if (fNorm)
					{
					int itbd;
					int dxaClose = dxaCloseMin;
					for (itbd = 0 ; itbd < itbdDelMac; itbd++)
						pchGrpprl =  bltbyte(&dxaClose, pchGrpprl, sizeof(uns)); /* rgdxaDel */
					}
				}
			*pchGrpprl++ = itbdAddMac;
			if (itbdAddMac != 0)
				{
				pchGrpprl = bltbyte(rgdxaAdd, pchGrpprl, itbdAddMac * 2);
				pchGrpprl = bltbyte(rgtbdAdd, pchGrpprl, itbdAddMac);
				}
/* if the resulting grpprl must have sprms in normal order, make call
	to merge sprmPChgTabs into its correct position within *pgrpprl. */
			if (fNorm)
				{
				sebl.pgrpprlMerge = pgrpprl;
				sebl.cbMergeMax = cbMaxGrpprl;
				AddPrlSorted(&sebl, rgb, cb - sebl.cbMerge);
				}
			}
		}}
	return cb;
}




#ifdef DEBUGORNOTWIN
/* C B  G E N  C H P X  F R O M  C H P */
/* generate a CHPX which expresses the differences between *pchpBase and
*pchp  */
#ifdef MAC
/* %%Function:CbGenChpxFromChp %%Owner:davidlu */
NATIVE int CbGenChpxFromChp(pchpResult, pchp, pchpBase, fWord3) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_CbGenChpxFromChp %%Owner:davidlu */
HANDNATIVE int C_CbGenChpxFromChp(pchpResult, pchp, pchpBase)
#endif /* !MAC */
struct CHP *pchpResult;
struct CHP *pchp;
struct CHP *pchpBase;
#ifdef MAC
int fWord3;
#endif /* MAC */
{
	int cch;
	SetWords(pchpResult, 0, cwCHP);
	*(int *)pchpResult = ((*(int *)pchp) ^ (*(int *)pchpBase)) & (~maskFs);
	if (pchp->ftc != pchpBase->ftc)
		{
		pchpResult->ftc = pchp->ftc;
		pchpResult->fsFtc = fTrue;
		}
	if (pchp->kul != pchpBase->kul)
		{
		pchpResult->kul = pchp->kul;
		pchpResult->fsKul = fTrue;
		}
	if (pchp->hps != pchpBase->hps)
		{
		pchpResult->hps = pchp->hps;
		pchpResult->fsHps = fTrue;
		}
	if (pchp->hpsPos != pchpBase->hpsPos)
		{
		pchpResult->hpsPos = pchp->hpsPos;
		pchpResult->fsPos = fTrue;
		}
	if (pchp->qpsSpace != pchpBase->qpsSpace)
		{
		pchpResult->qpsSpace = pchp->qpsSpace;
		pchpResult->fsSpace = fTrue;
		}

	Mac ( if (!fWord3) )
		if (pchp->ico != pchpBase->ico)
			{
			pchpResult->ico = pchp->ico;
			pchpResult->fsIco = fTrue;
			}

	if (pchp->fSpec)
		pchpResult->fcPic = pchp->fcPic;
	return CchNonZeroPrefix(pchpResult, cbCHP);
}


#endif /* DEBUGORNOTWIN */

#ifdef DEBUGORNOTWIN
/* C B  G E N  P A P X  F R O M  P A P */
/* creates a papx from grpprl.
*/
#ifdef MAC
/* %%Function:CbGenPapxFromPap %%Owner:davidlu */
int CbGenPapxFromPap(ppapx, ppap, ppapBase, fWord3)
#else /* !MAC */
/* %%Function:C_CbGenPapxFromPap %%Owner:davidlu */
HANDNATIVE int C_CbGenPapxFromPap(ppapx, ppap, ppapBase)
#endif /* !MAC */
char *ppapx;
struct PAP *ppap;
struct PAP *ppapBase;
#ifdef MAC
int fWord3;
#endif /* MAC */
{
#ifdef MAC
	int cbHeight = (!fWord3) ? cbPHE : cbPAPH;
	int cb = CbGrpprlFromPap(fFalse, ppapx + cbHeight + 1, ppap, ppapBase, fWord3) + cbHeight + 1;
#endif /* MAC */
#ifdef WIN
	int cb = CbGrpprlFromPap(fFalse, ppapx + cbPHE + 1, ppap, ppapBase, fFalse) + cbPHE + 1;
#endif /* WIN */
	*ppapx = ppap->stc;

#ifdef MAC
	if (fWord3)
		SetBytes(ppapx + 1, 0, cbPAPH);
	else
#endif
			{
			bltb(&ppap->phe, ppapx + 1, sizeof(struct PHE));
/* encode distinguished "standard" papx as length 0 */
/* must be consistent with "bpapx == 0" case in CachePara */
/* constant "240" or "15" must match corresponding value in CachePara */
			if (cb == cbPHE + 1 && ppap->stc == 0 && ppapBase->stc == 0 &&
					!ppap->phe.fDiffLines == 0 && ppap->phe.clMac == 1 &&
					ppap->phe.dylLine == WinMac(240, 15) &&
					ppap->phe.dxaCol == 7980)
				return 0;
			/* dylLine == 50 is 12 points on HP LaserJet */
			}
/* we set the byte after the papx to 0, so FAddRun can properly check for
	duplicates in FKP. */
	*(ppapx + cb) = 0;
	return cb;
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC  /* located in CREATE.C for Opus */
/* A P P L Y  D O C  S E P */
/* applies section properties of docSrc to doc */
/* %%Function:ApplyDocSep %%Owner:davidlu */
ApplyDocSep(doc, docSrc)
int doc;
int docSrc;
{
	int cchSepx;
	struct CA ca;
	struct SEP sepStandard;
	char grpprlSepx[cbMaxGrpprl];

	StandardSep(&sepStandard);
	CacheSect(docSrc, (CP)CpMacDoc(docSrc));
	cchSepx = CbGrpprlProp(fTrue, grpprlSepx, cbMaxGrpprl, 
			&vsepFetch, &sepStandard, cwSEP - 1, mpiwspxSep, 0);
	if (cchSepx)
		{
		ca.doc = doc;
		ca.cpFirst = CpMacDoc(doc);
		ca.cpLim = ca.cpFirst + ccpEop;
		ApplyGrpprlCa(grpprlSepx, cchSepx, &ca);
		}
}


#endif /* MAC */

/* %%Function:CbAppendTapPropsToPapx %%Owner:davidlu */
EXPORT CbAppendTapPropsToPapx(rgbPapx, cbPapx, ptap, ptapBase, cbMax)
char *rgbPapx;
int cbPapx;
struct TAP *ptap;
struct TAP *ptapBase;
int cbMax;
{
	char *pch, *pchTc, *pchTcBase, *pchT;
	int cbTap, cbDxa, cbTc, cbDefTable, cbSprm, cbTot;
	int itcMac;
/* since CachePara ignores the properties of ttp marks, we will simply
	tromp on the papx passed to us, ensuring that the style is Normal, and
	that sprmPFInTable and sprmPFTtp are recorded in the PAPX we generate. */
	cbPapx = cbPHE + 5;
	*rgbPapx = stcNormal;
	pch = rgbPapx + cbPHE + 1;
	*pch++ = sprmPFInTable;
	*pch++ = 1;
	*pch++ = sprmPFTtp;
	*pch++ = 1;

/* find end of papx that was passed to us so we can append sgcTap sprms */
/* now generate a grpprl that encodes the contents of *ptap */
	cbTap = CbGrpprlProp(fFalse, pch, cbMax, ptap, ptapBase, cwTAPBase,
			mpiwspxTap, 0);
	pch += cbTap;
	itcMac = ptap->itcMac;
/* we will attempt to generate a sprmPDefTable if there are differences
	in the dxaCenter or rgtc */
	if ((FNeRgch(&ptap->rgdxaCenter, &ptapBase->rgdxaCenter,
			cbDxa = sizeof(int)*(itcMac + 1)) ||
			FNeRgch(&ptap->rgtc, &ptapBase->rgtc,
			sizeof(struct TC) * itcMac)))
		{
/* we only have to store the prefix of ptap->rgtc that is different from
	ptapBase->rgtc */
		pchT = (char *)ptap->rgtc;
		for (pchTc = ((char *)&ptap->rgtc[itcMac]) - 1,
				pchTcBase = ((char *)&ptapBase->rgtc[itcMac]) - 1;
				pchTc >= pchT && *pchTc == *pchTcBase;
				pchTc--, pchTcBase--)
			;
		cbTc = pchTc - pchT + 1;
/* we will generate sprmTDefTable if there is room to store all of the sprm */
		if (cbTap + (cbDefTable = 4 + cbDxa + cbTc)
				<= cbMax)
			{
			*pch++ = sprmTDefTable;
/* note: this is the only sprm with a two-byte length field. */
			cbSprm = cbDefTable - sizeof(int);
			pch = bltbyte(&cbSprm, pch, sizeof(int));
			*pch++ = itcMac;
			pch = bltbyte(&ptap->rgdxaCenter, pch, cbDxa);
			if (cbTc > 0)
				bltb(ptap->rgtc, pch, cbTc);
			cbTap += cbDefTable;
			}
		}
/* we set the byte after the papx to 0, so FAddRun can properly check for
	duplicates in FKP. */
	*(rgbPapx + (cbTot = cbPapx + cbTap)) = 0;
	return (cbTot);
}


#ifdef WIN

#include "ff.h"

/*  The following routines were written to provide an interface between */
/*  Opus' file I/O and that expected by the Wks/Biff/Mp reading code.   */

/* current position in file. Modified by SetStmPos. */

STM stmGlobal = {
	fnNil, fc0, fcNil}; /* fn, fc, fcMac */


/* %%Function:SetStmPos %%Owner:davidlu */
SetStmPos(fn, fc)
int	fn;
FC	fc;
{
	stmGlobal.fn = fn;
	stmGlobal.fc = fc;
}


/* %%Function:BFromVfcStm %%Owner:davidlu */
BFromVfcStm()
{
	/* get char then increment fc */
	return ((int)*(CHAR HUGE *)HpchFromFc(stmGlobal.fn, stmGlobal.fc++));
}


/* %%Function:WFromVfcStm %%Owner:davidlu */
WFromVfcStm()
{
	CHAR rgb[2];

	SetFnPos(stmGlobal.fn, stmGlobal.fc);
	ReadRgchFromFn(stmGlobal.fn, rgb, sizeof(int));
	stmGlobal.fc += sizeof(int);
	return (*(int *)rgb);
}


/* %%Function:RgbFromVfcStm %%Owner:davidlu */
RgbFromVfcStm(rgb, cb)
char	*rgb;
int	cb;
{
	SetFnPos(stmGlobal.fn, stmGlobal.fc);
	ReadRgchFromFn(stmGlobal.fn, rgb, cb);
	stmGlobal.fc += cb;
}


/* %%Function:RgbToVfcStm %%Owner:davidlu */
RgbToVfcStm(rgb, cb)
CHAR *rgb;
int cb;
{
	SetFnPos(stmGlobal.fn, stmGlobal.fc);
	WriteRgchToFn(stmGlobal.fn, rgb, cb);
	stmGlobal.fc += cb;
}


#endif /* WIN */

#ifdef NOTUSED
/* P N  A L L O C */
/* advance cbMac to the next sector bound and return the pn.
In the scratch file, remember partial text page for future filling.
*/
/* %%Function:PnAlloc %%Owner:davidlu */
PN PnAlloc(fn, fAllocMac)
int fn;
BOOL fAllocMac; /* when fTrue, allocate according to cbMac else use fcPos. */
{
#ifdef MAC
	return PnAlloc2(fn, PnAlloc1(fn, fAllocMac, fFalse), fAllocMac, fFalse);
#else /* WIN */
	return PnAlloc2(fn, PnAlloc1(fn, fAllocMac), fAllocMac);
#endif /* WIN */
}


#endif /* NOTUSED */


/* %%Function:PnAlloc1 %%Owner:davidlu */
EXPORT PnAlloc1(fn, fAllocMac)
int fn; 
BOOL fAllocMac;
{
	FC fc;
	struct FCB *pfcb = *mpfnhfcb[fn];

	fc = (fAllocMac ? pfcb->cbMac : pfcb->fcPos) + cbSector - 1;
	return (PnFromFc(fc));
}


#ifdef MAC
/* %%Function:PnAlloc2 %%Owner:davidlu */
EXPORT PnAlloc2(fn, pn, fAllocMac, fWord3)
int fn, pn; 
BOOL fAllocMac;
int fWord3;
#else /* WIN */
/* %%Function:PnAlloc2 %%Owner:davidlu */
EXPORT PnAlloc2(fn, pn, fAllocMac)
int fn, pn; 
BOOL fAllocMac;
#endif /* MAC */
{
#ifdef MAC
	FC fc = (!fWord3) ? FcFromPn(pn + 1) : FcFromPo(pn + 1);
#else /* WIN */
	FC fc = FcFromPn(pn + 1);
#endif /* WIN */
	struct FCB *pfcb = *mpfnhfcb[fn];

	if (fAllocMac)
		pfcb->cbMac = fc;
	else
		{
		pfcb->fcPos = fc;
		if (fc > pfcb->cbMac)
			pfcb->cbMac = fc;
		}
	return pn;
}



/* F  E N V  D I R T Y */
/* checks saved print environment against current print environment. */
/* returns TRUE if different, FALSE if the same. */
/* %%Function:FEnvDirty %%Owner:davidlu */
FEnvDirty(fn, fcEnv, cbEnv)
int fn;
FC fcEnv;
int cbEnv;
{
	int ih, ihMax, cbBuf, cbChecked;
	FC fcCur;
	char *pchEnv, *pchEnvMac, *pchBuf;
	char **rgh[4];
	int rgcb[4];
	char stBuf[cchMaxSz];

	if (vpri.hszPrinter == hNil || vpri.hszPrPort == hNil ||
			vpri.hszPrDriver == hNil || vpri.hprenv == hNil)
		return TRUE;
	rgcb[0] = CchSz(*vpri.hszPrinter);
	rgcb[1] = CchSz(*vpri.hszPrPort);
	rgcb[2] = CchSz(*vpri.hszPrDriver);
	rgcb[3] = CbOfH(vpri.hprenv);
	/* if saved env is different length than current no need to compare */
	if (cbEnv != rgcb[0] + rgcb[1] + rgcb[2] + rgcb[3])
		return TRUE;

	cbChecked = cbBuf = 0;
	pchEnv = pchEnvMac; /* value does not matter */
	ih = -1;
	ihMax = 4;
	rgh[0] = vpri.hszPrinter;
	rgh[1] = vpri.hszPrPort;
	rgh[2] = vpri.hszPrDriver;
	rgh[3] = vpri.hprenv;

	FreezeHp();
	while (ih < ihMax)
		{
		if (cbBuf == 0)
			{
			SetFnPos(fn, fcEnv + cbChecked);
			cbBuf = min(cchMaxSz, cbEnv - cbChecked);
			pchBuf = stBuf;
			ReadRgchFromFn(fn, pchBuf, cbBuf);
			}

		Assert(cbBuf > 0);
		while (cbBuf > 0 && pchEnv < pchEnvMac)
			{
			if (*pchBuf++ != *pchEnv++)
				{
				MeltHp();
				return TRUE;
				}
			cbChecked++;
			cbBuf--;
			}

		if (pchEnv >= pchEnvMac && ++ih < ihMax)
			{
			pchEnv = *rgh[ih];
			pchEnvMac = pchEnv + rgcb[ih];
			}
		}
	MeltHp();
	Assert(cbChecked == cbEnv);
	Assert(cbBuf == 0);
	return FALSE;
}



#define isprmMaxChp 14
#define isprmMaxF 8

csconst char rgsprmChp[isprmMaxChp] = {
	sprmCFBold,
			sprmCFItalic,
			sprmCFStrike, /* WIN - mask bit ignores this */
	sprmCFOutline,
			sprmCFShadow, /* WIN - mask bit ignores this */
	sprmCFSmallCaps,
			sprmCFCaps,
			sprmCFVanish,
			sprmCFtc,
			sprmCKul,
			sprmCQpsSpace,
			sprmCIco,
			sprmCHps,
			sprmCHpsPos
};



/* C B  G R P P R L  F R O M  C H P */
/* generates a list of prl's that expresses the differences of chp
from chpBase as a grpprl.
This routine does not have to be very fast, so accent is on compactness.
*/
/* %%Function:CbGrpprlFromChp %%Owner:davidlu */
CbGrpprlFromChp(pgrpprl, pchp, pchpBase)
char *pgrpprl;
struct CHP *pchp, *pchpBase;
{
	int cb = 0;
	int cbPrl, isprm, w;
	char rgb[3];

	if (w = ((*((int *)pchp) ^ *((int *)pchpBase)) & (~(maskFs | maskFNonUser))) )
		{
/* some bitfield is different from base */
#ifdef MAC   /* this is bitfield order dependent! */
		for (isprm = 0; w != 0; (uns)w <<= 1, isprm++)
			if (w < 0 &&
#else  /* WIN */			    
					for (isprm = 0; w != 0; (uns)w >>= 1, isprm++)
					if (w & 1 &&
#endif /* MAC */
					(cbPrl = CbGenPrl(pchp, pchpBase, rgsprmChp[isprm], rgb))
					!= 0)
						{
				pgrpprl = bltbyte(&rgb[0], pgrpprl, cbPrl);
						cb += cbPrl;
				}
		}
		/* compare other fields */
	for (isprm = isprmMaxF; isprm < isprmMaxChp; isprm++)
			if ((cbPrl = CbGenPrl(pchp, pchpBase, rgsprmChp[isprm], rgb))
			!= 0)
				{
		pgrpprl = bltbyte(&rgb[0], pgrpprl, cbPrl);
				cb += cbPrl;
		}
	return cb;
}


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Inssubs_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Inssubs_Last() */
