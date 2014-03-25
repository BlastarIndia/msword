#ifdef MAC
#define WINDOWS
#define CONTROLS
#include "toolbox.h"
#else
#define NOGDICAPMASKS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOMENUS
#define NORASTEROPS
#define NOSYSMETRICS
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMETALFILE
#define NOVIRTUALKEYCODES
#define NOREGION
#define NOSOUND
#define NOKANJI
#define NOCOMM
#endif

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "ch.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "prm.h"
#include "layout.h"
#include "inter.h"
#include "format.h"
#include "field.h"
#include "debug.h"
#include "cmd.h"
#include "sel.h"
#include "message.h"
#include "error.h"

#ifdef MAC
#include "mac.h"
#else
#include "resource.h"
#endif


#ifdef PROTOTYPE
#include "hdd.cpt"
#endif /* PROTOTYPE */

#ifdef MAC
extern int              vwwHdr;
#endif
extern int              wwMac;
extern struct WWD       **mpwwhwwd[];
extern struct PAP       vpapFetch;
extern int              vdocHdrEdit;
extern int              wwCur;
extern int              vccpFetch;
extern CP               vcpFetch;
extern struct WWD       **hwwdCur;
extern struct CHP       vchpFetch;
extern struct SEP       vsepFetch;
extern struct MWD       **mpmwhmwd[];
extern struct DOD       **mpdochdod[];
extern int              docMac;
extern struct SEL       selCur;
extern struct CA        caSect;
extern struct MERR      vmerr;
extern CHAR             rgchEop[];
extern int              vfSeeSel;
extern struct DBS	vdbs;
extern struct ESPRM     dnsprm[];


#ifdef MAC
/* these map an ihdt to an offset from ucmOpenHeader */
	int mpihdtducm[] = { 
	0, 0, 1, 1, 2, 3 	};


	int mpihdtducmFacing[] = { 
	4, 6, 5, 7, 2, 3 	};


#endif

/* indicates header which may be empty and may not be displayed, but should 
	not be deleted by FSaveHeader */
int	vihdtNew = ihdtNil;

/********************************/
/* I H D D  F R O M  D O C  C P */
/* returns sum of ihdt bits from cp1 up to and not including cp2.
Optionally returns ihdd sum up to and not including cp1.
The optional array mp... will be filled in with the partial sums.
Plan:
init ihdd = number of non-default footnote hdd's in dod.
for all divisions up to cp do
	for all ihdt's
		if bit ihdt is set in sect
			store ihdd for the bit in mpihdtihdd.
			update ihdd.
*/
/* %%Function:IhddFromDocCp %%Owner:chic */
IhddFromDocCp(pca, pihdd, mpihdtihdd)
struct CA *pca;	/* DON'T pass &caSect here - this routine modifies caSect */
int *pihdd, mpihdtihdd[];
{
	int doc = pca->doc;
	struct DOD *pdod = PdodDoc(doc);
	int grpfIhdt = pdod->dop.grpfIhdt;
	int docHdr = pdod->docHdr;
	int ihdd = 0, ihdt, ihddMac;
	struct CA caT;

	caT = *pca; /* in case someone pass in &caSect */
	Assert(pdod->fMother);
	if (docHdr == docNil)
		{
		ihddMac = -1;
		goto LSetIhdd;
		}
	ihddMac = IMacPlc(PdodDoc(docHdr)->hplchdd);

	for (; grpfIhdt != 0; grpfIhdt >>= 1)
		if ((grpfIhdt & 1) && ++ihdd == ihddMac)
			{
			Assert(fFalse);
			break;
			}
LSetIhdd:
	if (pihdd != 0)
		*pihdd = ihdd;
	if (ihdd >= ihddMac)
		return(-1);

/* start caching from cp0 instead of cp1 because ihdd has to be 
accumulated from the very beginning */
	for (CacheSectSedMac(doc, cp0); caSect.cpFirst < caT.cpLim; CacheSectSedMac(doc, caSect.cpLim))
		{
		if (pihdd != 0 && caSect.cpFirst <= caT.cpFirst && caSect.cpLim > caT.cpFirst)
			{
			*pihdd = ihdd;
			pihdd = 0;
			}
		grpfIhdt = vsepFetch.grpfIhdt;
		for (ihdt = 0; grpfIhdt != 0; grpfIhdt >>= 1, ihdt++)
/* bits in grpfIhdt are numbered 5, 4, 3, 2, 1, 0 left to right, right flush */
			if (grpfIhdt & 1)
				{
				if (mpihdtihdd)
					mpihdtihdd[ihdt] = ihdd;
				if (++ihdd == ihddMac)
					{
					Assert(fFalse);
					caSect.doc = docNil;
					return(-1);
					}
				}
		if (caSect.cpLim >= caT.cpLim)
			break;
		}
	caSect.doc = docNil;
	return ihdd;
}


/*****************************/
/* I h d t  F r o m  I h d d */
/* %%Function:IhdtFromIhdd %%Owner:chic */
IhdtFromIhdd(doc, ihdd, st)
int doc, ihdd;	/* main doc, not header */
char *st;       /* optional string for name of header - not used by WIN */
{
/* returns the ihdt for the specified ihdd and optionally a string describing
	the ihdt (e.g. "First Header")
	if ihdt < ihdtMaxSep, leaves caSect describing the section which owns the 
		header
*/
	int ihdt = ihdtMaxSep, ihddT;
	int *mpihdtducmT, grpfIhdt;
	CP cpMac;
	int mpihdtihdd[ihdtMaxSep];
	struct CA caSectT, caT;

/* look in dop for ihdd */
	if (ihdd < chdtFtn)
		{
		grpfIhdt = PdodDoc(doc)->dop.grpfIhdt;
		for (ihddT = 0; grpfIhdt != 0; grpfIhdt >>= 1, ihdt++)
			if ((grpfIhdt & 1) && ihddT++ == ihdd)
				return(ihdt);
		ihdt = ihdtMaxSep;
		}

/* scan sections until one containing the desired header is found */
	for (caSectT.cpLim = cp0, cpMac = CpMacDoc(doc); caSectT.cpLim < cpMac; )
		{
		CacheSect(doc, caSectT.cpLim);
		caSectT = caSect;
		SetWords(&mpihdtihdd, ihddNil, ihdtMaxSep);
		if (IhddFromDocCp(PcaSet(&caT, doc, cp0, caSectT.cpLim), 0, mpihdtihdd) == -1)
			break;
		/* check the hdd's for this section */
		for (ihdt = 0; ihdt < ihdtMaxSep && mpihdtihdd[ihdt] != ihdd; ihdt++)
			;
		if (ihdt < ihdtMaxSep)
			break;
		}
	Assert(ihdt < ihdtMaxSep);
#ifdef MAC
	/* now we have ihdt; create the name string */
	if (st)
		{
		mpihdtducmT = (PdodDoc(doc)->dop.fFacingPages) ? mpihdtducmFacing : mpihdtducm;
		GetHeaderSt(doc, caSectT.cpFirst, ucmOpenHeader + *(mpihdtducmT + ihdt), st);
		}
#endif
	return(ihdt);
}



/*************************/
/* N e w  T e x t  H d r */
/* %%Function:NewTextHdr %%Owner:chic */
NewTextHdr(doc, cpFirst, docDisp, ihdt, wwHdr, fSelect)
int doc;	/* mother doc */
CP cpFirst;
int docDisp, ihdt, wwHdr, fSelect;
{
/* Bring in the header/footer or footnote separator text */
	int fSameAsPrev;
	int stc;
	int cstcStd;
	int grpfIhdt, ihdd, ihdtT, ised;
	struct DOD *pdod;
	char rgb[3];
	struct CA ca, caT;

/* determine "same as prev" status and get the bounds of the hdr/ftr */
	if (ihdt < ihdtMaxSep)
		{
		fSameAsPrev = FCaFromIhdt(doc, cpFirst, ihdt, fFalse, &ca, &ihdd);
		if (!FStcDefined(doc, 
				stc = (FHeaderIhdt(ihdt)) ? stcHeader : stcFooter))
			FEnsureStcDefined(doc, stc, &cstcStd);
		ised = IInPlc(PdodDoc(doc)->hplcsed, cpFirst);
		}
	else
		{
		/* ftn separator */
		stc = stcNormal;
#ifdef MAC
		grpfIhdt = PdodDoc(doc)->dop.grpfIhdt;
		for (ihdd = 0, ihdtT = ihdtTFtn; ihdtT < ihdt; ihdtT++, grpfIhdt >>= 1)
/* bits in grpfIhdt are numbered  2, 1, 0 left to right, right flush */
			if (grpfIhdt & 1)
				ihdd++;
		if ((grpfIhdt & 1) == 0)
			ihdd = (ihdt == ihdtTFtnCont) ? ihddTFtnCont :
					((ihdt == ihdtTFtn) ? ihddTFtn : ihddNil);
#else /* WIN - made the above into routine used somewhere also */
		ihdd = IhddFromIhdtFtn(doc, ihdt);
#endif
		CaFromIhddSpec(doc, ihdd, &ca);
		fSameAsPrev = (ihdd < 0);
		ised = -1;
		}
#ifdef MAC
#ifndef JR
/* dim button if already same as prev; not shown in vol1 */
	if (wwHdr != wwNil && wwHdr == vwwHdr)
		ShowHideSameAsPrev(fSameAsPrev);
#endif
#endif

/* bring in the header text */
	pdod = PdodDoc(docDisp);
	if (pdod->fGetHdr || pdod->ised != ised || pdod->ihdt != ihdt)
		{
		pdod->dk = dkHdr; /* avoid mother doc page table inval */
		if (!FReplaceCps(PcaSetWholeDoc(&caT, docDisp), &ca))
			ca.cpLim = ca.cpFirst;
		Win(ClearFDifferFltg (docDisp, fltgAll));
		if (ca.cpLim == ca.cpFirst)
			{
			/* when header is null, make sure chEop has proper style */
			rgb[0] = sprmPStc;
			rgb[1] = stc;
			rgb[2] = sprmCPlain;
			ApplyGrpprlCa(rgb, 3, PcaSet(&ca, docDisp, cp0, ccpEop));
			InvalCaFierce();
			}
		else
			{
			/* if hdr/ftr is not nil, it contains an Eop with correct
			looks - delete the "standard" end-of-doc Eop which has
			unknown looks */
			Assert(PdodDoc(docDisp)->fShort);
			FDelete(PcaSetDcp(&caT, docDisp, CpMacDocEdit(docDisp), ccpEop));
			/* AdjustSels does not know it needs to backup selCur because deleted
			text is to the right of the selection, but the selection may be
			at cpMacDoc! */
			if (selCur.doc == docDisp && selCur.fIns && 
					selCur.cpFirst >= CpMacDoc(docDisp))
				selCur.cpFirst = selCur.cpLim = CpMacDocEdit(docDisp);
			}
		pdod = PdodDoc(docDisp);
		pdod->fDirty = pdod->fHdrSame = fFalse;
		pdod->ihdt = ihdt;
		pdod->ised = ised;
		pdod->dk = dkDispHdr;	/* restore doc type */
		if (wwHdr != wwNil)
			PwwdWw(wwHdr)->fSetElev = fTrue;
		}
	if (fSelect && docDisp == selCur.doc)
		SelectIns(&selCur, cp0);
	vdocHdrEdit = docNil;
	PdodDoc(docDisp)->fGetHdr = fFalse;
}


/****************************/
/* F  C a  F r o m  I h d t */
/* %%Function:FCaFromIhdt %%Owner:chic */
FCaFromIhdt(doc, cp, ihdtHdr, fSkip, pca, pihdd)
int doc;
CP cp;
int ihdtHdr;
int fSkip;
struct CA *pca;
int *pihdd;
{
/* fill a cache with data bounds for the header specified by ihdtHdr, at
	or before the section indicated by (doc,cp). if fSkip is true, a non-empty
	header in the starting section will be ignored. returns true if the header
	is empty (i.e. same as previous) */
	int fSect0;
	int ihdd = ihddNil, ihddSect;
	int grpfIhdt;
	CP cpNext;
	struct CA caT;
	int mpihdtihdd[ihdtMaxSep];

	Assert(ihdtHdr < ihdtMaxSep); /* not for footer separator */
	CacheSect(doc, cp);
	fSect0 = (caSect.cpFirst == cp0);
	for (; ; cp = cpNext, fSkip = fFalse)
		{
		if (cp == caSect.cpFirst) /* moved from outside the loop in case cp0 is section break */
			++cp;   /* so IhddFromDocCp will include this section */
		/* identify the headers for this section */
		cpNext = caSect.cpFirst - 1;
		grpfIhdt = vsepFetch.grpfIhdt;
		SetWords(&mpihdtihdd, ihddNil, ihdtMaxSep);
		if ((ihddSect = IhddFromDocCp(PcaSet(&caT, doc, cp0, cp), 0, mpihdtihdd)) == -1)
			break;
		ihdd = mpihdtihdd[ihdtHdr];
		if (fSkip)
			ihdd = ihddNil;
		else  if (ihdd != ihddNil)
			break;
#ifdef MAC
/* Opus doesn't want to map to the current section's odd header,
each type of header if empty is linked to the previous section
of the same type. */
		/* even header/footer is empty, try the odd/right one */
		if (ihdtHdr == ihdtTLeft)
			{
			ihdd = mpihdtihdd[ihdtTRight];
			if (fSkip && ihdd < ihddSect && caSect.cpFirst > cp0)
				ihdd = ihddNil;
			}
		else  if (ihdtHdr == ihdtBLeft)
			{
			ihdd = mpihdtihdd[ihdtBRight];
			if (fSkip && ihdd < ihddSect && caSect.cpFirst > cp0)
				ihdd = ihddNil;
			}
		if (ihdd != ihddNil)
			break;
#endif
		/* all empty, back up a section */
		if (cpNext < cp0)
			break;
		CacheSect(doc, cpNext);
		}
	CaFromIhdd(PdodDoc(doc)->docHdr, *pihdd = ihdd, pca);
/* OPUS doesn't care for the return value */
	/* same as prev if all sections have nil header, or if this section
		doesn't have the header, or if this is a right header in section 1 */
	return(ihdd == ihddNil || !(grpfIhdt & (1 << ihdtHdr)) ||
			fSect0 && (ihdtHdr == ihdtTRight || ihdtHdr == ihdtBRight));
}




#ifdef WIN /* not used in MacWord */
#ifdef DEBUG
/* %%Function:CkHplcHdd %%Owner:chic */
CkHplcHdd(doc)
int doc;
{
	struct DOD *pdod = PdodDoc(doc);
	int docHdr = pdod->docHdr;
	CP cpMac = CpMacDoc(doc);
	struct PLC **hplchdd;
	int grpfIhdt;
	int ihdd;

	if (!vdbs.fCkDoc)  return;
	Assert(pdod->fMother);
	if (doc < docMinNormal || docHdr == docNil)
		return;

	Assert ((hplchdd = PdodDoc(docHdr)->hplchdd) != hNil);

	grpfIhdt = pdod->dop.grpfIhdt;
	for (ihdd = 0; grpfIhdt != 0; grpfIhdt >>= 1)
		if (grpfIhdt & 1)
			ihdd++; /* sum up footnote separators */

	for (CacheSect(doc, cp0); caSect.cpFirst < cpMac;
			CacheSect(doc, caSect.cpLim))
		{
#ifdef DBGCHIC
		CommSzNum(SzShared("grpfIhdt = "), (int)vsepFetch.grpfIhdt);
#endif
		for (grpfIhdt = vsepFetch.grpfIhdt; grpfIhdt != 0; grpfIhdt >>= 1)
			if (grpfIhdt & 1)
				ihdd++;
		if (caSect.cpLim >= cpMac)
			break;
		}
	Assert(ihdd+1 == (*hplchdd)->iMac);
	caSect.doc = docNil;
}


#endif
#endif /* WIN */




/*****************************/
/* F   H e a d e r   I h d t */
/* %%Function:FHeaderIhdt %%Owner:chic */
FHeaderIhdt(ihdt)
int ihdt;
{
	return(ihdt == ihdtTLeft || ihdt == ihdtTRight || ihdt == ihdtTFirst);
}


/*********************************/
/* D o c   D i s p l a y   H d r */
/* %%Function:DocDisplayHdr %%Owner:chic */
int DocDisplayHdr(doc, cp, ihdt)
int doc;	/* mother doc for which header is needed */
CP cp;
int ihdt;
{
/* find or create a display document for the indicated header. must be
	followed by NewTextHdr call */
	int docHdr, docDisp;
	struct DOD *pdod;

/* check existing display document */
	if ((docDisp = DocDispFromDocCpIhdt(doc, cp, ihdt)) != docNil)
		return docDisp;

/* create one */
	if ((docDisp = DocCreateSub(doc, dkDispHdr)) == docNil)
		return(docNil);

	docHdr = PdodDoc(doc)->docHdr;
	Assert(docHdr != docNil);
	pdod = PdodDoc(docDisp);
	pdod->docHdr = PdodDoc(docHdr)->docHdr;
	pdod->ised = (ihdt >= ihdtMaxSep) ? -1 :
			IInPlc(PdodDoc(doc)->hplcsed, CpMin(cp, CpMacDoc(doc)));
	pdod->ihdt = ihdtNil;	/* so NewTextHdr fills in text */
	PdodDoc(docHdr)->docHdr = docDisp;
	return(docDisp);
}


/*********************************/
/* C p   M o m   F r o m   H d r */
/* %%Function:CpMomFromHdr %%Owner:chic */
CP CpMomFromHdr(doc)
int doc;	/* header display doc */
{
/* returns cpFirst for the section which owns the header being displayed in 
	doc */
	struct DOD *pdod = PdodDoc(doc);
	struct PLC **hplcsed;

	if (pdod->ihdt >= ihdtMaxSep)
		return(cp0);
	hplcsed = PdodMother(doc)->hplcsed;
	return(CpPlc(hplcsed, max(0, min(pdod->ised, IMacPlc(hplcsed) - 2))));
}


/* %%Function:DocDispFromDocCpIhdt %%Owner:chic */
DocDispFromDocCpIhdt(doc, cp, ihdt)
int doc;
CP cp;
int ihdt;
{
/* find any existed display document for the indicated header. */
	int docHdr, docDisp;
	int ised;
	struct DOD *pdod;

	Assert(!PdodDoc(doc)->fShort);
	docHdr = docDisp = PdodDoc(doc)->docHdr;
	Assert(docHdr != docNil);
	ised = (ihdt >= ihdtMaxSep) ? -1 :
			IInPlc(PdodDoc(doc)->hplcsed, CpMin(cp, CpMacDoc(doc)));
	pdod = PdodDoc(docHdr);
	while ((docDisp = pdod->docHdr) != docNil)
		{
		pdod = PdodDoc(docDisp);
		if (pdod->ised == ised && pdod->ihdt == ihdt)
			return(docDisp);
		}
	return docNil;
}


#ifdef WIN
/* return ihdd of footnote separators */
/* %%Function:IhddFromIhdtFtn %%Owner:chic */
IhddFromIhdtFtn(doc, ihdt)
int doc;
int ihdt;
{
	int grpfIhdt, ihdtT;
	int ihdd;

	Assert(!PdodDoc(doc)->fShort);
	Assert(ihdt >= ihdtMaxSep);

	grpfIhdt = PdodDoc(doc)->dop.grpfIhdt;
	for (ihdd = 0, ihdtT = ihdtTFtn; ihdtT < ihdt; ihdtT++, grpfIhdt >>= 1)
/* bits in grpfIhdt are numbered  2, 1, 0 left to right, right flush */
		if (grpfIhdt & 1)
			ihdd++;
	if ((grpfIhdt & 1) == 0)
		ihdd = (ihdt == ihdtTFtnCont) ? ihddTFtnCont :
				((ihdt == ihdtTFtn) ? ihddTFtn : ihddNil);
	return ihdd;
}


#endif



#ifdef WIN
/* F  S A V E  H E A D E R */
/*
The contents of the header window, if dirty, are moved back to the 
header document of mother as a new header.
If the document is not dirty and fHdrSame is true (=>Link To Prev/Reset button
was clicked), we have to delete the original header/footnote separator. 
Returns false if save was cancelled.
*/
/* %%Function:FSaveHeader %%Owner:chic */
FSaveHeader(docHdrDisp, fKillEmpty)
int docHdrDisp, fKillEmpty;
{
	int docMom = DocMother(docHdrDisp);
	int docHdr, ww;
	struct DOD *pdod;
	int ihdtHdr;
	int fHdrSame;
	int fEmptyHdr = fFalse;
	int ihdd, ihdt;
	CP cp;
	int grpfIhdt, grpfIhdtT, di;
	struct PAP pap;
	struct CA ca, caApply, caT;

	di = DiDirtyDoc(docHdrDisp);
	pdod = PdodDoc(docHdrDisp);
	fHdrSame = pdod->fHdrSame && di == diNotDirty;
	pdod->fHdrSame = fFalse;
	ihdtHdr = pdod->ihdt;

	if (CpMacDocEdit(docHdrDisp) == cp0)
		{
		CachePara(docHdrDisp, cp0);
		pap = vpapFetch; /* takes care of unused rgtbd entries */
		MapStc(PdodDoc(docMom), vpapFetch.stc, 0, &pap);
		pap.phe = vpapFetch.phe;	/* this doesn't matter */
		fEmptyHdr = !FNeRgw(&vpapFetch, &pap, cwPAP);
		}
	if (fEmptyHdr)
		{
		/* reset fEmptyHdr if we're not allowed to delete empties,
			or if more than one window is showing it */
		if (!fKillEmpty || (ww = WwDisp(docHdrDisp, wwNil, fTrue)) != wwNil &&
				(ww = WwDisp(docHdrDisp, ww, fTrue)) != wwNil)
			fEmptyHdr = fHdrSame;
		}
	else
		fEmptyHdr = fHdrSame;

	if (di == diNotDirty && !fEmptyHdr)
		return (fTrue);

/* now we know hdr is dirty or empty */
	docHdr = PdodDoc(docMom)->docHdr;
	Assert(docHdr);

	if (ihdtHdr >= ihdtMaxSep) /* footnote separator */
		{
		grpfIhdt = PdodDoc(docMom)->dop.grpfIhdt;
		ihdd = 0;
		ihdt = ihdtTFtn;
		}
	else
		{
		/* use ...SedMac because caApply used by SetGrpfIhdtPca to do
			applygrpprl */
		CacheSectSedMac(docMom, cp = CpMomFromHdr(docHdrDisp));
		caApply = caSect;
		caApply.cpFirst = caApply.cpLim - (caApply.cpLim == CpMac1Doc(docMom) ? ccpEop : 1);
		caSect.doc = docNil; /* invalidate because SedMac used */
		grpfIhdt = vsepFetch.grpfIhdt;
		ihdt = 0;
/* gets starting ihdd in this section */
		if ((ihdd = IhddFromDocCp(PcaSet(&caT, docMom, cp0, caSect.cpFirst),
				0 /*pihddFirst*/, 0 /*mpihdtihdd*/)) == ihddNil)
			grpfIhdt = 0;
		}

/* save header window contents in docHdr of docMom for section with
	wwd.sels.cpFirst, ihdt is in ihdtHdr.
	ihdd is last one used before the current section. Enumerate bits
	in current section to find ihdd for ihdtHdr and whether the header
	is a new one or an old one.
*/
	for (grpfIhdtT = grpfIhdt; grpfIhdt != 0 && ihdt < ihdtHdr; grpfIhdtT >>= 1, ihdt++)
		if (grpfIhdtT & 1)
			ihdd++;

	if (grpfIhdtT & 1)
		{
		if (fEmptyHdr && ihdtHdr != vihdtNew)
			{
/* delete an existing entry because it is empty or Link/Reset */
			CaFromIhdd(docHdr, ihdd, &ca);
			ca.cpLim += ccpEop;
			if (!FDelete(&ca))
				return fFalse;
			else
				{
				AssertDo(FOpenPlc(PdodDoc(docHdr)->hplchdd, ihdd, -1));
				if (ihdtHdr >= ihdtMaxSep)
					PdodDoc(docMom)->dop.grpfIhdt ^= 1 << (ihdtHdr - ihdtTFtn);
				else
					{
					grpfIhdt ^= 1 << ihdtHdr;
					SetGrpfIhdtPca(grpfIhdt, &caApply);
					}
				}
			goto LDirty;
			}
		}
	else  if (fEmptyHdr)
		return fTrue;
else /* enter new header or ftn separator entry */
		{
		if (!FInsertIhdd(docHdr, ihdd)) /* make new entry, copy later */
			return fFalse;
		if (ihdtHdr >= ihdtMaxSep) /* ftn separator */
			PdodDoc(docMom)->dop.grpfIhdt |= 1 << (ihdtHdr - ihdtTFtn);
		else
			{ /* header text */
			grpfIhdt |= 1 << ihdtHdr;
			SetGrpfIhdtPca(grpfIhdt, &caApply);
			}
		} /* new entry */

/* copy all of docHdrDisp to docHdr, ihdd */
	ca.doc = docHdrDisp;
	ca.cpFirst = cp0;
	ca.cpLim = CpMacDoc(docHdrDisp);
	ReplaceIhdd(docHdr, ihdd, &ca);

LDirty:
	PdodDoc(docHdrDisp)->fDirty = fFalse;
	PdodDoc(docMom)->fDirty = fTrue;
	if (ihdtHdr >= ihdtMaxSep)
		InvalPageView(docMom); /* usually not a doc, just fSpec char */
	else  if (!fEmptyHdr)
		{
		/* if no page view window displays this header, inval page 
			view so that they will */
		for (ww = WwDisp(docHdrDisp, wwNil, fTrue); ww != wwNil; 
				ww = WwDisp(docHdrDisp, ww, fTrue))
			if (PwwdWw(ww)->fPageView)
				break;
		if (ww == wwNil)
			InvalPageView(docMom);
		}

	Debug( vdbs.fCkDoc ? CkHplchdd(docMom) : 0);
	return(fTrue);
}


/* %%Function:SetGrpfIhdtPca %%Owner:chic */
SetGrpfIhdtPca(grpfIhdt, pca)
int grpfIhdt;
struct CA *pca;
{
	CHAR rgb[2];
	struct ESPRM *pesprm;

	Assert(!PdodDoc(pca->doc)->fShort);
#ifdef DEBUG
	pesprm = &dnsprm[sprmSGrpfIhdt];
	Assert(pesprm->b == offset(SEP,grpfIhdt));
	Assert(pesprm->sgc == sgcSep);
	Assert(pesprm->cch == 2);
#endif
	rgb[0] = sprmSGrpfIhdt;
	rgb[1] = (CHAR)grpfIhdt;
	ApplyGrpprlCa(rgb, 2, pca);
	if (caSect.doc == pca->doc)
		caSect.doc = docNil; /* blow cache */
}


#endif /* WIN */


