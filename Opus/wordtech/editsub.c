/* E D I T S U B . C */
/*  Non-core edit/fetch routines */


#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "ch.h"
#include "scc.h"
#include "format.h"
#include "prm.h"
#include "ruler.h"
#include "layout.h"
#include "file.h"
#include "cmd.h"
#include "style.h"
#include "border.h"
#ifdef WIN
#include "strtbl.h"
#include "status.h"
#define REVMARKING
#include "compare.h"
#define AUTOSAVE
#include "autosave.h"
#include "idle.h"
#include "field.h"
#endif
#include "error.h"
#include "debug.h"
#include "dde.h"


/* E X T E R N A L S */

extern struct CA	caPara;
extern struct CA	caSect;
extern struct CA	caPage;
extern struct CA	caHdt;
extern struct FLS	vfls;
extern struct SEL	selCur;
extern struct SEL	selDotted;
extern struct WWD	**mpwwhwwd[];
extern struct DOD	**mpdochdod[];
extern struct UAB	vuab;
extern struct PREF	vpref;
extern struct SAB	vsab;
extern struct SEP	vsepFetch;
extern int		wwMac;
extern int		wwCur;
extern int              vdocHdrEdit;
extern struct WWD	**hwwdCur;
extern int		vfSeeSel;
extern int		vifnd;
extern struct CA	caTap;
extern struct TCC   vtcc;
extern struct FLI	vfli;
extern struct SCC	vsccAbove;
extern struct SCC	*vpsccBelow;
extern struct MERR	vmerr;
extern struct AAB	vaab;
extern struct RULSS	vrulss;
extern struct RULDS	vrulds;
extern CP		vcpFetch;
extern int		vdocFetch;
extern struct RC	rcScreen;
extern int		vdocTemp;
extern BOOL		vfAwfulNoise;
extern int  	    	vccpFetch;
extern CHAR HUGE        *vhpchFetch;
extern struct SPX	mpiwspxSep[];
extern struct PAP   	vpapFetch;
extern int  	    	vdocPapLast;
extern int		vstcLast;
extern int              docMac;
#ifdef WIN
extern IDF              vidf;
extern struct CA	caAdjust;
extern int		vdocScratch;
extern struct PLDDLI	**vhplddliServer;
extern BOOL		vfDirtyLinks;
extern BOOL		vfInvalidDdli;
extern BOOL		vfDdeIdle;
extern struct LCB	vlcb;
extern ASD              asd;      /* AutoSave Descriptor. */
#ifdef DEBUG
extern int		cCaAdjust;
#endif /* DEBUG */
#endif /* WIN */

#ifdef MAC
extern struct LLC	vllc;
extern int  	    	vwwHdr;
extern struct CHP	vchpStc;
extern char		rgchEop[];
#endif

#ifdef DEBUG
extern struct DBS	vdbs;
#endif



#ifdef MAC
/* For compatibility. these functions are either no longer used by
Opus or are in the new edit.c. */
/*  %%Function:ReplaceRm  %%Owner:Notused  */
ReplaceRM(pca, fn, fc, dfc) /* no longer used by Opus */
struct CA *pca;
int fn;
FC fc, dfc;
{
	FReplaceRM(pca, fn, fc, dfc);
}


/*  %%Function:DeleteRM %%Owner:notused  */
DeleteRM(pca) /* no longer used by Opus */
struct CA *pca;
{
	FDeleteRM(pca);
}


/*  %%Function:FDelete  %%Owner:notused  */
FDelete(pca) /* in new edit.c */
struct CA *pca;
{
	return DcpCa(pca) == cp0 || FReplace( pca, fnNil, fc0, fc0 );
}


/*  %%Function:FReplaceCpsRM  %%Owner:peterj  */
FReplaceCpsRM(pcaDel, pcaIns) /* in new edit.c */
struct CA *pcaDel, *pcaIns;
{
	int matPrev = vmerr.mat;
	BOOL fSucceed;

	vmerr.mat = matNil;
	ReplaceCpsRM(pcaDel, pcaIns);
	fSucceed = !vmerr.fMemFail;
	if (matPrev != matNil)
		vmerr.mat = matPrev;
	return fSucceed;
}


#endif /* MAC */
/* F  R E P L A C E  R M */
/* General replace routine, takes revision marking into account. */
/* adjusts pca to what's left after deletion. */
/*  %%Function:FReplaceRm  %%Owner:peterj  */
FReplaceRM(pca, fn, fc, dfc)
struct CA *pca;
int fn;
FC fc, dfc;
{
	CP cpLim;
	struct CA caT;
	char grpprl[2];

	if (vmerr.fMemFail)
		return fFalse;

#ifdef WIN
	if (PdodMother(pca->doc)->dop.fRevMarking)
		{
		AssureLegalSel(pca);
		if (DcpCa(pca) != cp0)
			{
			if (!FRemoveRevMarking(pca, rrmDelete, fFalse /*fSubDocs*/))
				return fFalse;
			}
		if (dfc != fc0)
			{
			caT = *pca;
			caT.cpLim = pca->cpFirst;
			if (!FReplace(&caT, fn, fc, dfc))
				return fFalse;
			caT.cpLim = caT.cpFirst + dfc;
			grpprl[0] = sprmCFRMark;
			grpprl[1] = fTrue;
			ApplyGrpprlCa(grpprl, sizeof(grpprl), &caT);
			}
		}
	else
#endif
			{
			if (!FReplace(pca, fn, fc, dfc))
				return fFalse;
			pca->cpLim = pca->cpFirst;
			}

	return !vmerr.fMemFail;
}


/*  %%Function:FDeleteRm  %%Owner:peterj  */
FDeleteRM(pca)
struct CA *pca;
{
	return DcpCa(pca) == cp0 || FReplaceRM( pca, fnNil, fc0, fc0 );
}



#ifdef NOTUSED
/* R E P L I C A T E   C P S */
/*  %%Function:ReplicateCps  %%Owner:notused  */
ReplicateCps(doc, cp, ccp)
int doc;
CP cp, ccp;
{
/* create a duplicate of a sequence of characters immediately in front of
	itself */
	struct CA caT1, caT2;

	FReplaceCps(PcaSetWholeDoc(&caT1, vdocTemp), PcaSetDcp(&caT2, doc, cp, ccp));
	FReplaceCps(PcaPoint(&caT1, doc, cp), PcaSetDcp(&caT2, vdocTemp, cp0, ccp));
}


#endif

/* D I R T Y  D O C */
/*  Make doc dirty */
/*  %%Function:DirtyDoc  %%Owner:peterj  */

DirtyDoc (doc)
int doc;
{
	PdodDoc (doc)->fDirty = fTrue;
}


#ifdef WIN
/* F  S E A R C H  B O O K M A R K */
/*  Search for a bookmark with the name st in doc.  If found return
	the cpFirst and cpLim.
*/
/*  %%Function:FSearchBookmark  %%Owner:peterj  */

FSearchBookmark (doc, st, pcpFirst, pcpLim, pibkmk, bmc)
int doc;
char *st;
CP *pcpFirst, *pcpLim;
int *pibkmk;
int bmc;
{
	int ibkmk;
	struct CA ca;

	Assert (!PdodDoc (doc)->fShort);

	Assert (!(bmc&bmcSelect) || doc == selCur.doc);

	if (*st && st[1] == '\\' && (bmc&(bmcDoc|bmcSelect)))
		/* special bookmark */
		{
		StToSzInPlace(st);
		ibkmk = IdFromSzgSz (szgBkmkSpecial, st);
		SzToStInPlace(st);
		if ((ibkmk > ibkmkSpecialDocMax && !(bmc&bmcSelect)) ||
				ibkmk == iNil || 
				!FBkmkCpsIbkmkSpecial(doc, ibkmk, &ca.cpFirst, &ca.cpLim))
			ibkmk = ibkmkNil;
		}
	else
		{
		struct STTB **hsttb = PdodDoc (doc)->hsttbBkmk;
		if (hsttb != hNil && FSearchSttbUnsorted (hsttb, st, &ibkmk))
			BkmkCpsFromIbkf(doc, ibkmk, &ca.cpFirst, &ca.cpLim);
		else
			ibkmk = ibkmkNil;
		}

	if (pibkmk != NULL)
		*pibkmk = ibkmk;

	if (ibkmk != ibkmkNil && pcpFirst != NULL)
		{
		Assert(pcpLim != NULL);
		*pcpFirst = ca.cpFirst;
		*pcpLim = ca.cpLim;
		}

	return ibkmk != ibkmkNil;
}


/*  %%Function:FSearchSttbUnsorted  %%Owner:peterj  */
FSearchSttbUnsorted(hsttb, st, pibkf)
struct STTB **hsttb;
CHAR *st;
int *pibkf;
{
	int ibkf = (*hsttb)->ibstMac;
	char stSttb[cchMaxSz];

	AssertH(hsttb);
	while (--ibkf > iNil)
		{
		GetStFromSttb(hsttb, ibkf, stSttb);
		if (FEqNcSt(st, stSttb))
			break;
		}
	*pibkf = ibkf;
	Assert(pibkf != NULL);
	return ibkf != iNil;
}


/* F  B K M K  C P S  I B K M K  S P E C I A L */
/*  %%Function:FBkmkCpsIbkmkSpecial  %%Owner:peterj  */
FBkmkCpsIbkmkSpecial(doc, ibkmk, pcpFirst, pcpLim)
int doc, ibkmk;
CP *pcpFirst, *pcpLim;
{
	extern struct CA caPage, caTable;
	extern struct TCC vtcc;
	extern struct SELS rgselsInsert [];
	struct SELS selsCur, *psels;
	int isels, disels = 3;
	int sty;

	switch (ibkmk)
		{
	case ibkmkSODoc:
		*pcpFirst = *pcpLim = cp0;
		break;

	case ibkmkEODoc:
		*pcpFirst = *pcpLim = CpMacDocEdit(doc);
		break;

	case ibkmkDoc:
		*pcpFirst = cp0;
		*pcpLim = CpMacDocEdit(doc);
		break;

	case ibkmkSel:
		Assert(doc == selCur.doc);
		if (selCur.fColumn)
			{
			/* A selection of (non-row) cells within a table or a block
			 * selection is not valid as a bookmark because of the cp
			 * representation */
			 return fFalse;
			}
		*pcpFirst = selCur.cpFirst;
		*pcpLim = selCur.cpLim;
		break;

	case ibkmkPrevSel2:
		disels = 2;
	case ibkmkPrevSel1:
		selsCur = *(struct SELS *)&selCur;
		/* supress fields that are unimportant in the comparison. */
		NormalizeSels(&selsCur);
		for (isels = 0, psels = &rgselsInsert[0]; isels < iselsInsertMax; isels++)
			{
			if (!FNeRgw(&((psels++)->ca), &(selsCur.ca), cwCA))
				break;
			}

		psels = &rgselsInsert[(isels + disels) % iselsInsertMax];
		if (psels->doc == doc)
			{
			*pcpFirst =	CpMin(psels->cpFirst, CpMacDocEdit(psels->doc));
			*pcpLim = psels->fIns ? *pcpFirst :
				CpMin(psels->cpLim, CpMacDoc(psels->doc));
			}
		else
			return fFalse;
		break;

	case ibkmkLine:
		sty = styLine;
		goto LSty;

	case ibkmkPar:
		sty = styPara;
		goto LSty;

	case ibkmkSect:
		sty = stySection;
		goto LSty;

	case ibkmkChar:
		sty = styChar;
LSty:
		Assert(doc == selCur.doc);
		*pcpFirst = CpFirstSty(wwCur, doc, selCur.cpFirst,
				sty, fFalse /* fEnd */ );
		*pcpLim = CpLimSty(wwCur, doc, selCur.cpFirst,
				sty, fFalse /* fEnd */ );
		break;

	case ibkmkPage:
		Assert(doc == selCur.doc);
		CachePage(doc, selCur.cpFirst);
		*pcpFirst = caPage.cpFirst;
		*pcpLim = CpMin(caPage.cpLim, CpMacDoc(doc));
		break;

	case ibkmkSOSel:
		Assert(doc == selCur.doc);
		*pcpFirst = *pcpLim = selCur.cpFirst;
		break;

	case ibkmkEOSel:
		Assert(doc == selCur.doc);
		*pcpFirst = *pcpLim = CpMin(selCur.cpLim, CpMacDocEdit(doc));
		break;

	case ibkmkTable:
	case ibkmkCell:
		Assert(doc == selCur.doc);
		if (!FInTableDocCp(doc, selCur.cpFirst) ||
				(selCur.cpFirst < selCur.cpLim &&
				!FInTableDocCp(doc, selCur.cpLim - 1)))
			{
			return fFalse;
			}
		else  if (ibkmk == ibkmkCell)
			{
			CacheTc(wwNil, doc, selCur.cpFirst, fFalse, fFalse);
			*pcpFirst = vtcc.ca.cpFirst;
			*pcpLim = vtcc.ca.cpLim;
			}
		else
			{
			CacheTable(doc, selCur.cpFirst);
			*pcpFirst = caTable.cpFirst;
			*pcpLim = caTable.cpLim;
			}

		break;

	case ibkmkHeading:
			{
			int ipadFirst, ipadMac, ipad;
			int lvl;
			struct PLC **hplcpad;
			struct PAD pad;

			Assert(doc == selCur.doc);
			if (!FUpdateHplcpad(doc))
				return fFalse;
			hplcpad = PdodDoc(doc)->hplcpad;
			ipadFirst = IInPlcCheck(hplcpad, selCur.cpFirst);
			ipadMac = IMacPlc(hplcpad);
			lvl = 0x7fff;
			while (ipadFirst >= 0)
				{
				GetPlc(hplcpad, ipadFirst, &pad);
				if (!pad.fBody)
					{
					lvl = pad.lvl;
					break;
					}
				ipadFirst--;
				}
			for (ipad = ipadFirst+1; ipad < ipadMac; ipad++)
				{
				GetPlc(hplcpad, ipad, &pad);
				if (!pad.fBody && pad.lvl <= lvl)
					break;
				}
			*pcpFirst = ipadFirst >= 0 ? CpPlc(hplcpad, ipadFirst) : cp0;
			*pcpLim = ipad < ipadMac ? CpPlc(hplcpad, ipad) : CpMacDoc(doc);
			break;
			}

	default:
		Assert(fFalse);
		return fFalse;
		}

	return fTrue;
}


#endif /* WIN */

/* F E T C H  R G C H */
/* fetch [cp, cpLim) in doc to pch but not more than cchMax; return cch
in pcch.
*/
/*  %%Function:FetchRgch  %%Owner:peterj  */
FetchRgch(pcch, pch, doc, cp, cpLim, cchMax)
int *pcch, doc, cchMax;
char *pch;
CP cp, cpLim;
{
	int cch = 0;

	FetchCp(doc, cp, fcmChars);

	while (cch < cchMax && vcpFetch < cpLim)
		{
#ifdef INEFFICIENT
		int ccp = (int) CpMin((typeCP) min(cchMax - cch, vccpFetch),
				cpLim - vcpFetch);
#endif
		int ccp = cchMax - cch;
		if (ccp > vccpFetch)
			ccp = vccpFetch;
		if (ccp > cpLim - vcpFetch)
			ccp = cpLim - vcpFetch;

		bltbh(vhpchFetch, pch, ccp);
		pch += ccp;
		cch += ccp;

		if (ccp < vccpFetch)
			break; /* Save some work */
		FetchCp(docNil, cpNil, fcmChars);
		}
	*pcch = cch;
}


#ifdef MAC
/* C M D  I N S E R T  S E C T */
/*  %%Function:CmdInsertSect  %%Owner:notused  */
CmdInsertSect(pcmb)
CMB *pcmb;
{
	int cmd;
	struct SEP sep;
	struct CA ca, caRM;
	struct PAP pap;
	struct CHP chp;
	CP cpFirst;

	if (PdodDoc(selCur.doc)->fShort)
		{
		ErrorEid(eidNoSectAllowed,"CmdInsertSect");
		return cmdError;
		}
/* MACREVIEW davidlu (cc): this can fail */
	FSetUndoBefore(ucmInsertSect, uccInsert);
	if (!FSelWithinTable(&selCur))
		{
		if ((cmd = CmdAutoDelete(&caRM)) != cmdOK)
			return cmd;
		}
	else
		{
		if (!FInTableDocCp(selCur.doc, selCur.cpFirst))
			{
			SetUndoNil();
			Beep();
			return cmdCancelled;
			}
		cpFirst = selCur.cpFirst;
		TurnOffSel(&selCur);
		cpFirst = CpFirstTap(selCur.doc, cpFirst);
		MapStc(PdodDoc(selCur.doc), stcNormal, 0, &pap);
		chp = vchpStc;
		InvalPageView(selCur.doc);
		InvalTableProps(selCur.doc,cpFirst,ccpSect,fTrue/*fFierce*/);
		SelectIns(&selCur, cpFirst);
		}

/* Code merge bug fix: passing &vsepFetch to CmdInsertSect1 when the 
	content is not even valid is a NO NO! */
	CacheSect(selCur.doc, selCur.cpFirst);
	bltb(&vsepFetch, &sep, cbSEP);
/* MACREVIEW davidlu (cc): this can fail */
	CmdInsertSect1(&selCur.ca, &sep, 0/*pchp*/, 0/*ppap*/, fFalse /* fSepNext */, fFalse /*fRM...FUTURE*/);
	SelectIns(&selCur, selCur.cpFirst + 1);
	SetUndoAfter(PcaSet(&ca, selCur.doc, selCur.cpFirst-ccpSect, selCur.cpFirst));
	SetAgainUcm(ucmNewSect, ascEdit);
	vfSeeSel = fTrue;
	return cmdOK;
}


#endif	/* MAC */

/* C M D  I N S E R T  S E C T  1 */
/*  %%Function:CmdInsertSect1  %%Owner:peterj  */
CmdInsertSect1(pca, psep, pchp, ppap, fSepNext, fRM)
struct CA *pca;
struct SEP *psep;
struct CHP *pchp;
struct PAP *ppap;
int fSepNext, fRM;
	/* if true (for all but rtf), sep if for text that FOLLOWS inserted chSect. 
		For rtf, it is for text that precedes chSect.
		!! MAC always passes fFalse !! */
{
	int doc = pca->doc;
	int cchSepx;
	int grpfIhdt;
	struct PLC **hplcsed;
	int ised;
	int ihddFirst, ihddLim;
	struct SED sed;
	FC fc;
	FC fcMac;
	FC fcFirstNextPage;
	struct FCB *pfcb;
	char rgch[2];
	char rgchSepx[cchSepxMax];
	Win ( struct CA caSectOld; )
			struct SEP sepStandard;
	struct CHP chp;
	struct PAP pap;
	struct SEP *psepIns;

/* insert a section mark at pca->cpFirst with sep's in psep. Plan:
	1. write sepx to scratch file (mixed with text, not props)
	2. write chSect to scratch file as end of paragraph.
	3. make room in plcsed and insert new sed.
	4. set up undo record.
*/
	Mac(Assert(!fSepNext));

	FCleanUpHdr(pca->doc, fFalse, fFalse);	/* save page view headers */

	fc = fcNil;

	StandardSep(&sepStandard);
	CacheSectCa(pca);
	if (fSepNext)
		psepIns = &vsepFetch;
	else
		psepIns = psep;
	grpfIhdt = vsepFetch.grpfIhdt;
#ifdef MAC
	if (grpfIhdt)
		{
		ihddLim = IhddFromDocCp(&caSect, &ihddFirst, 0);
		if (ihddLim <= ihddFirst)
			{
			Assert(fFalse);
			grpfIhdt = 0;	/* bullet-proof */
			}
		}
#endif


/* Generate sepx */
	cchSepx = CbGrpprlProp(fTrue, rgchSepx+1, cchSepxMax-1, psepIns,
			&sepStandard, cwSEP - 1, mpiwspxSep, 0);

	if (cchSepx != 0)
		{
		if ((PfcbFn(fnScratch)->cbMac & (cbSector - 1)) + cchSepx + 1
				> cbSector)
			AlignPn(fnScratch);
		rgchSepx[0] = cchSepx;
		fc = FcAppendRgchToFn(fnScratch, rgchSepx, cchSepx + 1);
		}

	Win( CacheSectSedMac(doc, pca->cpFirst) );
	Win( caSectOld = caSect );
	Win( caSect.doc = docNil );

/* insert chSect */
	if (ppap == 0)
		{
		CacheParaCa(pca);
		pap = vpapFetch;
		pap.fInTable = fFalse;
		pap.fTtp = fFalse;
		ppap = &pap;
		}
	if (pchp == NULL)
		GetPchpDocCpFIns (pchp = &chp, doc, pca->cpFirst, fTrue, wwNil);

	if (fRM)
		pchp->fRMark = PdodMother(doc)->dop.fRevMarking;

	rgch[0] = chSect;
	if (!FInsertRgch(doc, pca->cpFirst, rgch, 1, pchp, ppap))
		return cmdNoMemory;

	hplcsed = PdodDoc(doc)->hplcsed;

	Assert(hplcsed != hNil);

	if (!FOpenPlc(hplcsed, ised = IInPlc(hplcsed, pca->cpFirst + 1), 1))
		return cmdNoMemory;


	PutCpPlc(hplcsed, ised, CpPlc(hplcsed, ised + 1));
	PutCpPlc(hplcsed, ised + 1, pca->cpFirst + 1);
	sed.fn = fnScratch;
	sed.fcSepx = fc;
	PutPlc(hplcsed, ised, &sed);
/* For rtf, put in ised, other uses changed so that the passed in sep has 
	the new setting and should be set to the new section */

#ifdef MAC
/* now we have a valid section, except for the headers. copy the headers
	for the new section */
	if (grpfIhdt)
		CloneHdrText(doc, ihddFirst, ihddLim);
	else
		vdocHdrEdit = doc;	/* empty header may be open */
#else /* WIN */
	if (fSepNext)
		{
/* make sure new section had the property set in the piece table */
		struct CA caSectNew;
		caSectNew = caSectOld;
		caSectOld.cpLim = pca->cpFirst+1;
		caSectNew.cpFirst = caSectOld.cpLim;
		caSectNew.cpLim++;

/* header text are linked by default in WIN */
		if (grpfIhdt)
			psep->grpfIhdt = 0;

		CacheSectCa(&caSectNew); /* get vsepFetch as base sep */
		if ((rgchSepx[0] = 
				CbGrpprlProp(fTrue, rgchSepx+1, cchSepxMax-1, psep,
				&vsepFetch, cwSEP - 1, mpiwspxSep, 0)) != 0)
			{
			ApplyGrpprlCa(&rgchSepx[1], rgchSepx[0], &caSectNew);
			}
		}
#endif

/* invalidate doc/page display */
	InvalLlc();
	caSect.doc = caPage.doc = docNil;
/* update hplcpad */
#ifndef JR
	if (wwCur != wwNil && (*hwwdCur)->fOutline)
		UpdateHplcpadSingle(doc, pca->cpFirst + 1);
#endif /* JR */
	Win( InvalFtn(doc, pca->cpFirst) );
	PdodDoc(doc)->fFormatted = fTrue;
	InvalPageView(doc);
	return(cmdOK);
}



/* C A  F R O M  I H D D  F I R S T  L I M*/
/* Packs the doc, the cpFirst for ihddFirst, and the cpLim of ihddLim into the
	*pca. Note that the extra Eop added by InsertIhdd is not considered part of
	the header but is included within the *pca produced because the callers
	of this routine are moving/adding/deleting entire groups of header entries.
	doc passed is the docHdr subdoc
*/
/*  %%Function:CaFromIhddFirstLim  %%Owner:peterj  */
CaFromIhddFirstLim(doc, ihddFirst, ihddLim,  pca)
int doc, ihddFirst, ihddLim;
struct CA *pca;
{
	struct PLC **hplchdd = PdodDoc(doc)->hplchdd;

	SetWords(pca, 0, cwCA);
	pca->doc = doc;
	Assert(doc != docNil);
	if (doc == docNil || hplchdd == hNil || ihddFirst == ihddNil ||
			ihddLim == ihddNil)
		return;
	Assert(ihddFirst < IMacPlc(hplchdd) && ihddLim <= IMacPlc(hplchdd));
	if (ihddFirst < IMacPlc(hplchdd) && ihddLim <= IMacPlc(hplchdd))
		{
		pca->cpFirst = CpPlc(hplchdd, ihddFirst);
		pca->cpLim = CpPlc(hplchdd, ihddLim);
		}
}


/* R E P L A C E  I H D D */
/*
Replaces the header/glsy text in doc at ihdd by the text
described in pca.
*/
/*  %%Function:ReplaceIhdd  %%Owner:peterj  */
ReplaceIhdd(doc, ihdd, pca)
int doc, ihdd;
struct CA *pca;
{
	struct CA caOld;

	Assert(ihdd < IMacPlc(PdodDoc(doc)->hplchdd));

	CaFromIhdd(doc, ihdd, &caOld);
	caHdt.doc = docNil;
/* OK if this fails, we're just changing the header text - no catastrophe */
	if (FReplaceCps(&caOld, pca) && pca->doc != docUndo)
		Win( CopyStylesFonts (pca->doc, doc, caOld.cpFirst, DcpCa(pca)) );
}


/* F  I N S E R T  I H D D */
/* Inserts a new entry into doc's hplchdd at ihdd representing empty text. */
/*  %%Function:FInsertIhdd  %%Owner:peterj  */
FInsertIhdd(doc, ihdd)
int doc, ihdd;
{
	struct PLC **hplchdd = PdodDoc(doc)->hplchdd;
	struct CA caOld;
	CHAR rgch[cbDDLI];

	caHdt.doc = docNil;
	CaFromIhdd(doc, ihdd, &caOld);
	/* an extra Eop is kept between each hdd entry; it prevents two
		adjoining entries from having the same cp if the first is nil */
	caOld.cpLim = caOld.cpFirst;
	if (!FReplace(&caOld, fnSpec, (FC) fcSpecEop, ccpEop))
		return(fFalse);
	/* AdjustCp didn't adjust cp for (new) ihdd+1 */
	PutCpPlc(hplchdd, ihdd, CpPlc(hplchdd, ihdd) + ccpEop);
	Assert((*hplchdd)->cb <= cbDDLI);
	if (FInsertInPlc(hplchdd, ihdd, caOld.cpFirst, rgch))
		return(fTrue);

	caOld.cpLim = caOld.cpFirst + ccpEop;
	FDelete(&caOld);
	return(fFalse);
}


/* I N V A L  D O C */
/*  %%Function:InvalDoc  %%Owner:peterj  */
InvalDoc(doc)
int doc;
{
/* invalidate all windows owned directly or indirectly by doc */
	int docFtn, docHdr;
#ifdef WIN
	int	docAtn;
#endif
	struct DOD *pdod;

	TrashWwsForDoc(doc);
	pdod = PdodDoc(doc);
	if (!pdod->fShort)
		{
		docFtn = pdod->docFtn;
		docHdr = pdod->docHdr;
		if (docFtn)
			TrashWwsForDoc(docFtn);
#ifdef WIN
		docAtn = pdod->docAtn;
		if (docAtn)
			TrashWwsForDoc(docAtn);
#endif
		if (docHdr)
			{
			int docHdrDisp;

			TrashWwsForDoc(docHdr);
			docHdrDisp = PdodDoc(docHdr)->docHdr;
			while (docHdrDisp != docNil)
				{
				TrashWwsForDoc(docHdrDisp);
				docHdrDisp = PdodDoc(docHdrDisp)->docHdr;
				}
			}
		}
	InvalCaFierce();
	vstcLast = stcNil;
}


#ifdef WIN
#define FRulerUp(ww)    (PmwdWw(ww)->wwRuler == ww)
#endif /* WIN */

/*********************************/
/* I n v a l   P a g e   V i e w */
/*  %%Function:InvalPageView  %%Owner:peterj  */
InvalPageView(doc)
int doc;
{
	int ww;
	struct WWD *pwwd;
	struct DOP *pdop = &PdodMother(doc)->dop;

	vuab.docInvalPageView = DocMother(doc);
	for (ww = WwDisp(doc, wwNil, fFalse); ww != wwNil; ww = WwDisp(doc, ww, fFalse))
		{
		pwwd = PwwdWw(ww);
		if (pwwd->fPageView)
			{
			if (ww == wwCur && FRulerUp(ww))
				{
				Mac(vrulds.stc = -1);
				Mac(vrulds.dxaColumnWidth = -1);
				Win(selCur.fUpdateRuler = fTrue);
				}
			pwwd->fDirty = pwwd->fDrDirty = fTrue;
/* it's possible that page setup recorded changes to the page size, so
	recalculate pwwd->rcePage. */
			pwwd->rcePage.xeRight = pwwd->rcePage.xeLeft +
					DxsFromDxa(pwwd, pdop->xaPage);
			pwwd->rcePage.yeBottom = pwwd->rcePage.yeTop +
					DysFromDya(pdop->yaPage);
			TrashWw(ww);
			}
		}
}


/*  %%Function:GetPl  %%Owner:peterj  */
GetPl( hpl, ifoo, pfoo )
struct PL **hpl;
int ifoo;
char *pfoo;
{
	struct PL *ppl = *hpl;

	AssertH( hpl );
	Assert( ifoo < IMacPl( hpl ) );
	bltbyte( ppl->rgbHead + (ifoo * ppl->cb), pfoo, ppl->cb );
}


#ifdef WIN /* not used in MacWord */
/*  %%Function:PutPl  %%Owner:peterj  */
PutPl( hpl, ifoo, pfoo )
struct PL **hpl;
int ifoo;
char *pfoo;
{
	struct PL *ppl = *hpl;

	AssertH( hpl );
	Assert( ifoo < IMacPl( hpl ) );
	bltbyte( pfoo, ppl->rgbHead + (ifoo * ppl->cb), ppl->cb );
}


#endif /* WIN */


#ifdef WIN
/* F  F O R M A T  D I S P L A Y  P A R A */
/*  Format all lines in the paragraph containing cp.  Build a plc
	containing the cpFirsts of each line.
*/
/*  %%Function:FFormatDisplayPara  %%Owner:peterj  */
FFormatDisplayPara (ww, doc, cp, cpLim, hplc)
int ww;
int doc;
CP cp, cpLim;
struct PLC **hplc;

{
	extern int vflm;
	int i = 0;

	if (hplc == hNil)
		return fFalse;

	Assert (cp == CpFormatFrom (ww, doc, cp));

	PutIMacPlc( hplc, 0 );
	PutCpPlc( hplc, 0, cpLim );

	while (cp < cpLim)
		{
		/* failure handled below */
		FInsertInPlc (hplc, i++, cp, NULL);
		FormatLine (ww, doc, cp);
		cp = vfli.cpMac;
		if (vmerr.fMemFail)
			return fFalse;
		}

	return fTrue;

}


/* S C R A T C H  B K M K S */
/* delete all bookmarks which are fully in the range
cpFirst to cpLim.  adjust all bookmarks partially
in the range so that they are not in the range. 
*/
/*  %%Function:ScratchBkmks  %%Owner:peterj  */
ScratchBkmks (pca)
struct CA *pca;
{
	XDeleteBkmks(fFalse, NULL, pca, fTrue);
}


/* B K M K  C P S  F R O M  I B K F */
/*  %%Function:BkmkCpsFromIbkf  %%Owner:peterj  */
BkmkCpsFromIbkf (doc, ibkf, pcpFirst, pcpLim)
int doc, ibkf;
CP *pcpFirst, *pcpLim;

{
	struct DOD *pdod = PdodDoc (doc);
	struct PLC **hplcbkf = pdod->hplcbkf;
	struct BKF bkf;
	CP cpLast;

	Assert (!pdod->fShort);
	Assert (ibkf < IMacPlc(hplcbkf));

	*pcpFirst = CpPlc(hplcbkf, ibkf);
	GetPlc(hplcbkf, ibkf, &bkf);
	*pcpLim = CpPlc(pdod->hplcbkl, bkf.ibkl);

	Assert(*pcpFirst >= cp0 && *pcpLim >= *pcpFirst);

	if (*pcpFirst < *pcpLim)
		/* Bookmark is not an insertion point--make sure that if it crosses
		   a cell bound (or includes the cell mark) that it incorporates 
		   whole rows */
		{
		if (FInTableDocCp(doc, *pcpFirst))
			{
			CacheTc(wwNil, doc, *pcpFirst, fFalse, fFalse);
			if (*pcpLim >= vtcc.cpLim)
				/* assure bookmark begins at beginning of row */
				*pcpFirst = CpFirstTap(doc, *pcpFirst);
			}
		if (*pcpLim > cp0 && FInTableDocCp(doc, (cpLast = *pcpLim-1)))
			{
			CacheTc(wwNil, doc, cpLast, fFalse, fFalse);
			if (*pcpFirst < vtcc.cpFirst || *pcpLim >= vtcc.cpLim)
				/* assure bookmark ends at the end of row */
				{
				CpFirstTap(doc, cpLast);
				*pcpLim = caTap.cpLim;
				}
			}
		}
	Assert(*pcpFirst >= cp0 && *pcpLim >= *pcpFirst);

	return fTrue;
}


/* F  L E G A L  B K M K  N A M E */
/*  Return true iff st is a legal bookmark name
*/

/*  %%Function:FLegalBkmkName  %%Owner:peterj  */
FLegalBkmkName (st)
char *st;

{
	char *pch = st+1;
	char *pchLim = st + *st + 1;

	if (*st >= cchBkmkMax || *st <= 0 || !FAlpha (*pch))
		return fFalse;

	while (++pch < pchLim)
		/*	these are the legal bookmark characters */
		if (!FAlpha (*pch) && !FDigit (*pch) && *pch != '_')
			return fFalse;

	return fTrue;
}


#endif /* WIN */


