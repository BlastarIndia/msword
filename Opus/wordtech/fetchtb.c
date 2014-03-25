#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "file.h"
#include "props.h"
#include "border.h"
#include "fkp.h"
#include "ch.h"
#include "format.h"
#include "layout.h"
#include "prm.h"
#include "sel.h"
#include "disp.h"
#include "inter.h"
#include "cmd.h"
#include "debug.h"
#include "screen.h"
#ifdef MAC
#include "mac.h"
#include "toolbox.h"
#include "print.h"
#endif

#ifdef WIN
#include "field.h"
#ifdef PROTOTYPE
#include "fetch1.cpt"
#endif /* PROTOTYPE */
#include "status.h"
#endif  /* WIN */

/* E X T E R N A L S */

extern BOOL             vfEndFetch;
extern struct DOD       **mpdochdod[];

extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct FKPD      vfkpdChp;

extern char HUGE        *vhpchFetch;
extern int              vdocFetch;
extern CP               vcpFetch;
extern int              vccpFetch;
extern int              vdocTemp;

extern char             rgchInsert[];
extern int              ichInsert;
extern struct SELS      rgselsInsert[];

extern struct FCB       **mpfnhfcb[];

extern struct CA        caPara;
extern struct CA	caTap;

extern struct FKPD      vfkpdPap;

extern struct CA        caSect;
extern int              vised;
extern struct CA        caPage;
extern int              vipgd;
extern CP               vcpFirstLayout;

extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern int              prmFetch;
extern struct LLC       vllc;
extern struct TAP       vtapStc;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern CP               vmpitccp[];
extern int              vitcMic;

extern char             (**vhgrpchr)[];
extern int              vbchrMax;
extern int              vbchrMac;

extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct PREF      vpref;
extern struct SEL       selCur;

extern struct FLS       vfls;
extern struct MERR      vmerr;
extern struct DBS       vdbs;

extern int              wwCur;
extern struct WWD       **hwwdCur;

extern struct ITR       vitr;

extern BOOL             vfShowAllStd;
extern struct SCI       vsci;

extern int              vjnRare;
extern int              vjcRare;
extern int              vlm;

extern struct TCXS		vtcxs;
#ifdef WIN
extern struct LCB  vlcb;
extern int              vflm;
extern int		vlmSave;
extern int		vflmSave;
extern int				vitcMicAux;
extern CP				vmpitccpAux[];
extern struct CA		caTapAux;
extern struct TAP		vtapFetchAux;

extern CP vcpFirstTablePara;
extern CP vcpFirstTableCell;
#endif

#ifdef MAC
extern int		vftcHelvetica;
extern struct PRSU      vprsu;
#endif

#ifdef JR
extern int              vfJRProps;
#endif /* JR */


#ifdef WIN
/* F  I N  T A B L E  D O C  C P  N O  T A P */
/* Returns whether a doc,cp is in a table according to field structures.
	vcpFirstTablePara is set to the cpFirst of the table in this paragraph.
	vcpFirstCellPara is set to the cpFirst of the table cell.
*/
/* %%Function:FInTableDocCpNoTap %%Owner:davidlu */
NATIVE BOOL FInTableDocCpNoTap(doc, cp)
int doc;
CP cp;
{
	extern struct CA caPara;
	extern struct PAP vpapFetch;
	extern int vdocFetch;
	extern CP vcpFetch;
	extern int fcmFetch;
	CP cpFirstCell, cpLimCell;
	struct PLC **hplcfld;
	struct FLD fld;
	BOOL fInTable;
	int ifld, ifldMin, ifldLast;
	CP cpFirst;

	CachePara(doc, cp);
	vcpFirstTableCell = vcpFirstTablePara = caPara.cpFirst;
	if (!vpapFetch.fInTable)
		return fFalse;

	/* find the cell limit */
	for (;;)
		{
		if (!vpapFetch.fInTable)
			{
			cpLimCell = caPara.cpFirst - 1;
			break;
			}
		FetchCp(doc, caPara.cpLim-1, fcmChars);
		if (*vhpchFetch == chTable)
			{
			cpLimCell = caPara.cpLim - 1;
			break;
			}
		Assert(caPara.cpLim < CpMacDoc(doc));
		CachePara(doc, caPara.cpLim);
		}
	CachePara(doc, cp);

	/* find the cell start */
	for (;;)
		{
		if (caPara.cpFirst == cp0)
			{
			cpFirstCell = cp0;
			break;
			}
		CachePara(doc, caPara.cpFirst-1);
		if (!vpapFetch.fInTable || 
				ChFetch(doc, caPara.cpLim-1, fcmChars) == chTable)
			{
			cpFirstCell = caPara.cpLim;
			break;
			}
		}

	CachePara(doc, cp);

	Assert(FInCa(doc, cp, &caPara));
	Assert(vpapFetch.fInTable);
	fInTable = fTrue;
	cpFirst = cpFirstCell;

	if ((hplcfld = PdodDoc(doc)->hplcfld) != hNil)
		{
		int c = 0;

		ifldMin = IInPlcRef(hplcfld, cpFirstCell);
		ifld = ifldLast = IInPlcCheck(hplcfld, cpLimCell);

		while (ifld >= ifldMin)
			{
			Assert(ifld >= 0 && ifldMin >= 0 && ifld < IMacPlc(hplcfld));
				GetPlc(hplcfld, ifld, &fld);
				if (fld.ch == chFieldEnd)
				c++;
			else  if (!c)
				{
				/* this covers the case when an open
				   field brace occurs right before
				   a table (no eop between) */
				cpFirst = CpPlc(hplcfld, ifld) + 1;
				break;
				}
			else  if (fld.ch == chFieldBegin && --c == 0)
				ifldLast = ifld - 1;
				ifld--;
			}
		Assert(c >= 0);
		if (c > 0)
			cpFirst = CpPlc(hplcfld, ifldLast) + 1;
		fInTable = cp >= cpFirst;
		}

	vcpFirstTablePara = FInCa(doc, cpFirst, &caPara) ? cpFirst : caPara.cpFirst;
	vcpFirstTableCell = cpFirst;

	return fInTable;
}
#endif /* WIN */



/* F  C A C H E  T A P  C O R E */
/* Description: Given a cp in a paragraph with the fInTable property,
/*	cache the whole table row. The corresponding TAP is then loaded 
/* into vtapFetch, get the maximum range in caTap.
/**/
/* %%Function:FCacheTapCore %%Owner:tomsax */
NATIVE FCacheTapCore(doc, cp, fOutline)
int doc;
CP cp;
BOOL fOutline;
{
	int ipcd, prm;
	int itc;
	int fCaFull, fFirstRow, fLastRow;
	BOOL fInCa;
	struct PLC **hplcpcd;
	struct PCD pcd;
	BOOL fInTable;
	CP cpFirstPara;
	extern CP vcpFirstTablePara;

	if ((fInCa=FInCa(doc,cp,&caTap)) && vtapFetch.fOutline==fOutline)
		return fTrue;

	/* make a couple of guesses, to be corrected later */
	fFirstRow = cp == cp0;
	fLastRow = fFalse;

	/* NOTE: we check a possibly bogus cpLim before checking the doc
	/* since if either of the first two terms are going to fail it
	/* is more likely to be the cp check.
	/**/
	fCaFull = ((cp == caTap.cpLim && doc == caTap.doc) || cp == cp0);

	itc = 0; /* we'll verify/adjust this later */

	CachePara(doc, cp);
#ifdef MAC
	fInTable = FInTableDocCp(doc, cp);
	Assert (fInTable);
#else /* WIN */
	if (!(fInTable = FInTableDocCpNoTap(doc, cp)))
		return fFalse;	
#endif /* WIN */

	cp = cpFirstPara = vcpFirstTablePara;
#ifdef MAC
	if (caTap.doc == doc)
		{
#endif /* MAC */
		if (fInCa)
			{
			/* TAP is cached, but needs adjustment */
			fCaFull = vtapFetch.fCaFull;
			fFirstRow = vtapFetch.fFirstRow;
			fLastRow = vtapFetch.fLastRow;
			goto LAdjustTap;
			}
#ifdef MAC
		}
	else
#endif /* MAC */
		{
		PcaPoint(&caTap,doc,cp);
		vitcMic = itcMax + 1;
		}
	for (;;)
		{
		if (!fInTable)
			{
			/* error condition, which we probably won't survive, but let's try */
			Assert(fFalse);
			caTap.cpLim = cpFirstPara;
			return fTrue;
			}
		if (vpapFetch.fTtp)
			break;

		/* check for cpLim of current cell, cpFirst of next cell */
		Assert(caPara.cpLim >= 1);
		if (ChFetch(doc, caPara.cpLim-1,fcmChars) == chTable)
			vmpitccp[++itc] = caPara.cpLim;

		/* We used to extend the current TAP backward if fCaFull wasn't
		   set the first time through CacheTap.  Now we always fully
		   cache the TAP so that code is no longer necessary.  The call
		   to FInCa in the assert below was used to determine if the cache
		   should be extended.  Brad Verheiden 9-13-90 */
		Assert (!FInCa(doc,cpFirstPara,&caTap));

		CachePara(doc, caPara.cpLim);
		/* we can use vpapFetch.fInTable here (don't need to call
			FInTableDocCp) because we know we'll stop at an fTtp para */
		fInTable = vpapFetch.fInTable;
		cpFirstPara = caPara.cpFirst;
		}
	caTap.cpFirst = cp;
	vmpitccp[++itc] = caTap.cpLim = caPara.cpLim;

	Assert(caPara.cpFirst == caPara.cpLim-ccpEop);

	ipcd = IInPlc((hplcpcd = (*mpdochdod[doc])->hplcpcd), caPara.cpFirst);
	GetPlc(hplcpcd, ipcd, &pcd);
	prm = pcd.prm;

	vtapFetch = vtapStc;
	if (prm != prmNil)
		DoPrmSgc(prm, &vtapFetch, sgcTap);

	Assert(caTap.cpLim < CpMacDoc(doc));
	CachePara ( doc, caTap.cpLim );
#ifdef MAC
	fLastRow = !FInTableVPapFetch(doc, caTap.cpLim);
#else /* WIN */
	fLastRow = !(vpapFetch.fInTable && FInTableDocCpNoTap(doc, caTap.cpLim));
#endif /* WIN */

	/* We have filled in "itc" entries, the last of which belongs in position
	/* "vtapFetch.itcMac + 1" (the fTtp paragraph corresponds to the itcMac
	/* cell, add one for its cpLim). Thus the first entry we made belongs at
	/* position "(vtapFetch.itcMac+1) - (itc-1)".
	/**/
	vitcMic = vtapFetch.itcMac - itc + 2;

LAdjustTap:
	vtapFetch.fOutline = fOutline;	/* verbose for native compiler bug */
	if (fOutline)
		{
		struct DOD *pdod;
		pdod = PdodMother(doc); /* verbose for native compiler */
		vtapFetch.dxaAdjust = dxaOtlMark + pdod->dop.dxaTab / 2
				- vtapFetch.rgdxaCenter[0] - vtapFetch.dxaGapHalf;
		goto LCheckAdjustment;
		}
	else  if (vtapFetch.jc == jcLeft)
		vtapFetch.dxaAdjust = 0;
	else
		{
		CacheSect(doc,caTap.cpLim-1);

		vtapFetch.dxaAdjust = vsepFetch.dxaColumnWidth - vtapFetch.rgdxaCenter[vtapFetch.itcMac];
		if (vtapFetch.jc == jcCenter)
			vtapFetch.dxaAdjust /= 2;

LCheckAdjustment:
		if (vtapFetch.dxaAdjust + vtapFetch.rgdxaCenter[0] < xaLeftMinSci)
			vtapFetch.dxaAdjust = xaLeftMinSci - vtapFetch.rgdxaCenter[0];
		}

LAdjustMpitccp:

	vtapFetch.fCaFull = fCaFull;
	vtapFetch.fFirstRow = fFirstRow; /* fFalse value valid iff fCaFull is fTrue */
	vtapFetch.fLastRow = fLastRow;

	/* We have filled positions [1,..,itc] of vmpitccp[] with CPs. The cp
	/* we put into slot 1 really belongs in slot vitcMic.
	/**/
	if (vitcMic != 1)
		bltbyte((BYTE*)&vmpitccp[1], (BYTE*)&vmpitccp[vitcMic], sizeof(CP)*itc);
	if (fCaFull)
		vmpitccp[vitcMic=0] = caTap.cpFirst;
	return fTrue;
}


/*  C P  F I R S T  T A P
/**/
/* %%Function:CpFirstTap %%Owner:tomsax */
NATIVE CP CpFirstTap(doc,cpFirst)
int doc;
CP cpFirst;
{
	return CpFirstTap1(doc,cpFirst,caTap.doc==doc?vtapFetch.fOutline:fFalse);
}


/*  C P  F I R S T  T A P 1
/*	Description: Caches the tap for the given cp, then scans backward in
/*	the document to extend caTap to the full (implicit) range of influence
/*	of the cached TAP
/**/
/* %%Function:CpFirstTap1 %%Owner:tomsax */
NATIVE CP CpFirstTap1(doc, cpFirst, fOutline)
int doc;
CP cpFirst;
{
	CP cpFirstPara;
	BOOL fInTable, fInCa;
	extern CP vcpFirstTablePara;

#ifdef WIN
	if (FInCa(doc,cpFirst,&caTapAux))
		{
		int				itcMicSav;
		CP				mpitccpSav[itcMax+1];
		struct CA		caTapSav;
		struct TAP		tapFetchSav;

		itcMicSav = vitcMic;
		bltb(&vmpitccp, &mpitccpSav, sizeof(CP) * (itcMax+1));
		caTapSav = caTap;
		tapFetchSav = vtapFetch;

		vitcMic = vitcMicAux;
		bltb(&vmpitccpAux, &vmpitccp, sizeof(CP) * (itcMax+1));
		caTap = caTapAux;
		vtapFetch = vtapFetchAux;

		vitcMicAux = itcMicSav;
		bltb(&mpitccpSav, &vmpitccpAux, sizeof(CP) * (itcMax+1));
		caTapAux = caTapSav;
		vtapFetchAux = tapFetchSav;
		}
#endif /* WIN */

	if ((fInCa = FInCa(doc,cpFirst,&caTap)) && vtapFetch.fCaFull
			&& vtapFetch.fOutline==fOutline)
		return caTap.cpFirst;

#ifdef WIN
	if (!fInCa)
		{
		vitcMicAux = vitcMic;
		bltb(&vmpitccp, &vmpitccpAux, sizeof(CP) * (itcMax+1));
		caTapAux = caTap;
		vtapFetchAux = vtapFetch;
		}
#endif /* WIN */

	if (!FCacheTapCore(doc,cpFirst,fOutline))
		return cpNil;
	Assert(FInCa(doc,cpFirst,&caTap));

	if (vtapFetch.fCaFull)
		goto LExit;

	/* scan backward until we find the beginning of the table row */
	Assert(vitcMic > 0);
	cpFirst = vmpitccp[vitcMic] - ccpEop;
	Assert(cpFirst >= cp0);
#ifdef MAC
	fInTable = FInTableDocCp(doc, cpFirst);
#else /* WIN */
	fInTable = FInTableDocCpNoTap(doc, cpFirst);
#endif /* WIN */
	cpFirstPara = vcpFirstTablePara;
	Assert(fInTable && (!vpapFetch.fTtp||vitcMic==vtapFetch.itcMac+1));
	for (;;)
		{
		if (cpFirstPara == cp0)
			{
			caTap.cpFirst = cp0;
			break;
			}
		/* FInTableDocCp caches para */
#ifdef MAC
		fInTable = FInTableDocCp(doc, cpFirstPara-1);
#else /* WIN */
		fInTable = FInTableDocCpNoTap(doc, cpFirstPara-1);
#endif /* WIN */
		cpFirstPara = vcpFirstTablePara;
		if (vpapFetch.fTtp || !fInTable)
			{
			caTap.cpFirst = (cpFirstPara == caPara.cpFirst ?
					caPara.cpLim : cpFirstPara);
			break;
			}
		Assert(caPara.cpLim >= 1);
		if (ChFetch(doc, caPara.cpLim-1, fcmChars) == chTable)
			vmpitccp[--vitcMic] = caPara.cpLim;
		}
	Assert(vitcMic == 1);
	vmpitccp[--vitcMic] = caTap.cpFirst;

	vtapFetch.fCaFull = fTrue;

	if (caTap.cpFirst == cp0)
		vtapFetch.fFirstRow = fTrue;
	else
		{
		CachePara(doc, caTap.cpFirst - 1 /* not ccpEop */);
#ifdef MAC
		vtapFetch.fFirstRow = !FInTableVPapFetch(doc,
				caTap.cpFirst - 1 /* not ccpEop */);
#else /* WIN */
		vtapFetch.fFirstRow = !(vpapFetch.fInTable && FInTableDocCpNoTap(doc,
				caTap.cpFirst - 1 /* not ccpEop */));
#endif /* WIN */
		}
LExit:
	Assert (caTap.cpFirst == vmpitccp[0]);
	return caTap.cpFirst;

}


#ifdef NOTUSED

/* C P  L I M  T A B L E  C E L L
/*
/* Description: Given a doc,cp within a table row, return the cpLim
/* for the cell. The caller may assume that the last paragraph in
/* the cell is in caPara/vpapFetch upon exit.
/**/
/* %%Function:CpLimTableCell %%Owner:tomsax */
NATIVE CP CpLimTableCell(doc, cp) /* WINIGNORE - NOTUSED */
int doc;
CP cp;
{
	CachePara(doc, cp);
	for (;;)
		{
		Assert(FInTableDocCp(doc, cp));
		Assert(caPara.cpLim >= 1);
		if (ChFetch(doc, caPara.cpLim-1, fcmChars) == chTable)
			break;
		Assert(caPara.cpLim < CpMacDoc(doc));
		CachePara(doc, caPara.cpLim);
		}
	return caPara.cpLim;
}


#endif /* NOTUSED */


/* C A C H E  T C
/*
/* Description: caches info about a cell of a table, including assocaited
/* cp range, horizontal and width, and tap of previous row for border info.
/* Sequential calls are relatively cheap.
/* To get just cp information (not the xp,yp information), call with wwNil.
/*
/* WARNING: FInCa(doc,cp,&vtcc.ca) does not guarantee that the cell dimensions
/* are correct for the given cp, only that the cp info is correct.
/**/
/* %%Function:CacheTc %%Owner:tomsax */
NATIVE CacheTc(ww, doc, cp, fForceFirst, fForceLast)
int ww, doc;
CP cp;
BOOL fForceFirst, fForceLast;
{
	int itc, itcMac;
	BOOL fFirstRow, fLastRow, fOutline, fRowCached;
	CP *pcp;
	struct WWD *pwwd;

	/* is the info already there ? */
	if ((fRowCached = FInCa(doc,cp,&vtcc.caTap)) && FInCa(doc,cp,&vtcc.ca))
		{
		if (ww == wwNil)
			return;
		pwwd = PwwdWw(ww);
		if (vtcc.fXpValid && pwwd->fOutline == vtcc.fOutline
#ifdef WIN
				&& (vfti.dxpInch == vtcc.dxpInch)
#endif
				&& (!fForceFirst || vtcc.fFirstRow)
				&& (!fForceLast || vtcc.fLastRow))
			return;
		}

	if (ww == wwNil)
		fOutline = caTap.doc==doc?vtapFetch.fOutline:fFalse;
	else
		{
		pwwd = PwwdWw(ww);
		fOutline = pwwd->fOutline;
		}

	/* If we are on a different row or any of the caller's preferences
	/* have changed, invalidate our previous calculations
	/**/
	if (!fRowCached || fOutline != vtcc.fOutline
			|| (fForceFirst && !vtcc.fFirstRow)
			|| (fForceLast && !vtcc.fLastRow))
		{
		vtcc.cpFirst = vtcc.cpLim = cpNil;
		vtcc.itc = -1; /* optimizes cp search on first cell */
		vtcc.fXpValid = vtcc.fDylValid = fFalse;

#ifdef MAC
		if (cp > cp0 && !FInCa(doc,cp,&caTap) && FInCa(doc,cp-1,&caTap))
			{
			/* we just ran off the end of the table row we have been
			/* working on. Save the current TAP in vtcc.tapPrev, just in
			/* in case we need it later.
			/**/
			vtcc.caTapPrev = caTap;
			vtcc.tapPrev = vtapFetch;
			}
#endif /* MAC */
		}

LComputeTcc:

	Break1();
	vtcc.fOutline = fOutline;
	vtcc.doc = doc;

	CpFirstTap1(doc,cp,fOutline);
	vtcc.caTap = caTap;
	Assert(vitcMic == 0);
	Assert(vmpitccp[0] <= cp && cp < vmpitccp[vtapFetch.itcMac+1]);

	if (cp == vmpitccp[itc=vtcc.itc+1])
		{
		vtcc.cpFirst = cp;
		vtcc.cpLim = vmpitccp[itc+1];
		}
	else
		{
		for (pcp = vmpitccp; *pcp <= cp; pcp++)
			;
		vtcc.cpLim = *(pcp--);
		vtcc.cpFirst = *pcp;
		itc = pcp - &vmpitccp[0];
		}
	Assert(vitcMic <= itc && itc <= vtapFetch.itcMac);
	vtcc.itc = itc;

	if (ww == wwNil)
		{
		vtcc.fXpValid = fFalse;
		return;
		}

	if (vtcc.fDylValid)
		goto LGetSize;

/* State: vtcc.ca is valid, and caTap is set for the entire current row.
/* Goal: if necessary, fill in info on prev row.
/**/

	vtcc.fFirstRow = fFirstRow = fForceFirst || vtapFetch.fFirstRow;
	vtcc.fLastRow = fLastRow = fForceLast || vtapFetch.fLastRow;

#ifdef MAC
	if (!fFirstRow && !(FInCa(doc,caTap.cpFirst-ccpEop,&vtcc.caTapPrev) && fOutline == vtcc.tapPrev.fOutline))
		{
#ifdef	SLOW
		Assert(caTap.cpFirst >= ccpEop);

		CpFirstTap1(doc, CpMax(cp0,caTap.cpFirst - ccpEop), fOutline);
		vtcc.caTapPrev = caTap;
		vtcc.tapPrev = vtapFetch;
		CpFirstTap1(doc, caTap.cpLim, fOutline);
#else /* SLOW */
		struct CA caTapSav;
		struct TAP tapSav;
		CP mpitccpSav[itcMax+1];

		Assert(caTap.cpFirst >= ccpEop);

		/* save the current stuff */
		Assert(vitcMic == 0);
		caTapSav = caTap;
		tapSav = vtapFetch;
		blt(vmpitccp, mpitccpSav, (itcMax+1)*cwCP);

		/* get the dirt on the previous row */
		Assert(caTap.cpFirst >= ccpEop);
		CpFirstTap1(doc, caTap.cpFirst - ccpEop, fOutline);
		vtcc.caTapPrev = caTap;
		vtcc.tapPrev = vtapFetch;

		/* restore to the current row */
		caTap = caTapSav;
		vtapFetch = tapSav;
		blt(mpitccpSav, vmpitccp, (itcMax+1)*cwCP);
		vitcMic = 0;
#endif /* SLOW */
		}
#else /* WIN */
	if (!fFirstRow && !(FInCa(doc,caTap.cpFirst - 1, &caTapAux)
		&& fOutline == vtapFetchAux.fOutline))
		{
		Assert(caTap.cpFirst >= 1);

		CpFirstTap1(doc, CpMax(cp0,caTap.cpFirst - 1), fOutline);
		CpFirstTap1(doc, caTap.cpLim, fOutline);
		Assert(FInCa(doc, CpMax(cp0, caTap.cpFirst - 1), &caTapAux));
 		}
#endif /* WIN */

	if (FEasyBorders(fFirstRow, fLastRow, vtcc.rgbrcEasy))
		{
		vtcc.fEasyBorders = fTrue;	/* verbose for NATIVE compiler */
		vtcc.dylAbove = DyFromBrc(vtcc.rgbrcEasy[ibrcTop],fFalse);
		vtcc.dylBelow = fLastRow ? DyFromBrc(vtcc.rgbrcEasy[ibrcBottom],fFalse) : 0;
		}
	else
		{
		vtcc.fEasyBorders = fFalse;	/* verbose for NATIVE compiler */
#ifdef MAC
		CalcHorizTableBorders(ww,&vtapFetch,&vtcc.tapPrev,fFirstRow,fLastRow,&vtcc.dylAbove,&vtcc.dylBelow);
#else /* WIN */
		CalcHorizTableBorders(ww,&vtapFetch,&vtapFetchAux,fFirstRow,fLastRow,&vtcc.dylAbove,&vtcc.dylBelow);
#endif /* WIN */
		}

	vtcc.fDylValid = fTrue;


/* State: vtcc.ca, fFirstRow, fLastRow, itc, and (when fFirstRow, vtcc.caTapPrev
/* and tapPrev) are valid, and caTap is set for the entire current row.
/* Goal: Compute dimensions of cell for current itc.
/**/
LGetSize:
	Break1();

	ItcGetTcxCache(ww, caTap.doc, caTap.cpFirst, &vtapFetch, itc, &vtcc.tcx);

	vtcc.fXpValid = fTrue;
#ifdef	WIN
	vtcc.dxpInch = vfti.dxpInch;
#endif
}


/*	C A L C  H O R I Z  T A B L E  B O R D E R S
/*
/*	Description: Given the tap for the current row (and if fFirstRow
/*	the tap for the previous row), determine the amount of y-space that
/*	should be allocated to draw the (shared) above and below borders.
/**/
/* %%Function:CalcHorizTableBorders %%Owner:tomsax */
NATIVE CalcHorizTableBorders(ww,ptapCur,ptapPrev,fFirstRow,fLastRow,pdylAbove,pdylBelow)
int ww;
struct TAP *ptapCur, *ptapPrev;
BOOL fFirstRow;
int *pdylAbove, *pdylBelow;
{
	int dylAbove, dylBelow;
	int itcT, itcMacT;
	struct TCX tcxT;
	int fUseCache = (ptapCur == &vtapFetch);

	/* NOTE - Here we call DxFromBrc with fFrameLines=fFalse,
	/* because we want to allocate vertical space for the table
	/* borders iff there really are borders. If vpref.fDrawTableDrs
	/* and there is no gap, then DisplayFli will draw the
	/* frame lines within its domain.
	/**/

	/* scan bottom borders of previous row */
	dylAbove = dylBelow = 0;
	if (!fFirstRow)
		{
		itcMacT = ptapPrev->itcMac;
		/* have to scan forward to parse merged cells correctly */
		for (itcT = 0; itcT < itcMacT; )
			{
			itcT = ItcGetTcx(ww, ptapPrev, itcT, &tcxT);
			dylAbove = max(dylAbove,DyFromBrc(tcxT.brcBottom,fFalse));
			}
		}

	/* scan borders of current row */
	itcMacT = ptapCur->itcMac;
	/* have to scan forward to parse merged cells correctly */
	for ( itcT = 0; itcT < itcMacT; )
		{
		if (fUseCache)
			itcT = ItcGetTcxCache ( ww, caTap.doc, caTap.cpFirst, ptapCur, itcT, &tcxT);
		else
			itcT = ItcGetTcx ( ww, ptapCur, itcT, &tcxT);

		dylAbove = max(dylAbove,DyFromBrc(tcxT.brcTop,fFalse));
		if ( fLastRow )
			dylBelow = max(dylBelow,DyFromBrc(tcxT.brcBottom,fFalse));
		}

	*pdylAbove = dylAbove;
	*pdylBelow = dylBelow;
}


/* F  E A S Y  B O R D E R S
/*
/**/
/* %%Function:FEasyBorders %%Owner:tomsax */
NATIVE BOOL FEasyBorders(fFirstRow, fLastRow, rgbrc)
BOOL fFirstRow, fLastRow;
int	rgbrc[];
{
	int	itcMac;
	int rgbrcCur[cbrcCellBorders],rgbrcPrev[cbrcCellBorders];

	itcMac = vtapFetch.itcMac;

	if (!fFirstRow)
		{
#ifdef MAC
		if (itcMac != vtcc.tapPrev.itcMac 
				|| FNeRgw(vtapFetch.rgdxaCenter, vtcc.tapPrev.rgdxaCenter, itcMac+1)
				|| vtapFetch.dxaAdjust != vtcc.tapPrev.dxaAdjust)
#else /* WIN */
		if (itcMac != vtapFetchAux.itcMac 
				|| FNeRgw(vtapFetch.rgdxaCenter,
					vtapFetchAux.rgdxaCenter,
					itcMac+1)
				|| vtapFetch.dxaAdjust
					!= vtapFetchAux.dxaAdjust)
#endif /* WIN */
			return fFalse;
		}

	GetRowBorders(rgbrc,&vtapFetch,0/*itcFirst*/,itcMac,fTrue/*fStrict*/);

	if (!fFirstRow)
		{
#ifdef MAC
		GetRowBorders(rgbrcPrev,&vtcc.tapPrev,0/*itcFirst*/,itcMac,fTrue/*fStrict*/);
#else /* WIN */
		GetRowBorders(rgbrcPrev,
			&vtapFetchAux,
			0/*itcFirst*/,itcMac,fTrue/*fStrict*/);
#endif /* WIN */
		if (rgbrc[ibrcLeft] != rgbrcPrev[ibrcLeft]
				|| rgbrc[ibrcRight] != rgbrcPrev[ibrcRight]
				|| rgbrc[ibrcInside] != rgbrcPrev[ibrcInside])
			return fFalse;
		if (rgbrc[ibrcTop] == brcNone)
			{
			/* expedient lie */
			rgbrc[ibrcTop] = rgbrcPrev[ibrcBottom]; /* brcNil will cause a false return */
			}
		}

	if (FEasyBrc(rgbrc[ibrcTop]) &&	FEasyBrc(rgbrc[ibrcLeft])
			&&	FEasyBrc(rgbrc[ibrcRight])
			&& (!fLastRow || FEasyBrc(rgbrc[ibrcBottom]))
			&& (itcMac == 1 || FEasyBrc(rgbrc[ibrcInside])))
		return fTrue;

	return fFalse;
}


/* %%Function:FEasyBrc %%Owner:tomsax */
NATIVE FEasyBrc(brc)
int brc;
{
	Mac(Assert(brc == brcNil || (brc & brcfShadow) == 0));

	/* note: brcNil fails the first test in the if */
	if ((brc & (brcDxpSpace | brcfShadow)) != 0	|| brc == brcTwoSingle)
		return fFalse;

	/* these are the brc's that SetPenForBrc() can handle */
	Assert(brc == brcNone
			|| brc == brcSingle
			|| brc == brcThick
			/* SetPenForBrc() will never get called to draw a real hairline */
	Mac( || brc == brcHairline)
			|| brc == brcDotted);
	return fTrue;
}


#ifdef DEBUGORNOTWIN
/* I T C  G E T  T C X  C A C H E 
* 
*  Allows us to cache the calls to ItcGetTcx so repeated calls
*  for the same information are not needed
*
*/
#ifdef MAC
NATIVE ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:ItcGetTcxCache %%Owner:tomsax */
HANDNATIVE C_ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx)
#endif /* !MAC */
int ww, doc, itc;
CP cp;
struct TAP *ptap;
struct TCX *ptcx;
{
	int fCache = ww != wwNil;
	int itcNext;
	int itcxc;

	if (!FInCa(doc, cp, &vtcxs.ca) || vtcxs.grpfTap != ptap->grpfTap
#ifdef WIN
			|| vtcxs.dxpInch != vfti.dxpInch
#endif
			)
		{
		if (!fCache || !FInCa(doc, cp, &caTap))
			fCache = fFalse;
		else
			{
			for (itcxc = 0 ; itcxc < itcxcMax ; itcxc++)
				vtcxs.rgtcxc[itcxc].fValid = fFalse;
			vtcxs.ca = caTap;
			vtcxs.grpfTap = ptap->grpfTap;
#ifdef WIN
			vtcxs.dxpInch = vfti.dxpInch;
#endif
			}
		}
	else  if (itc < itcxcMax && vtcxs.rgtcxc[itc].fValid)
		{
		bltb(&vtcxs.rgtcxc[itc].tcx, ptcx, sizeof(struct TCX));
		return (vtcxs.rgtcxc[itc].itcNext);
		}

	itcNext = ItcGetTcx(ww, ptap, itc, ptcx);
	if (fCache && itc < itcxcMax)
		{
		Break1();
		bltb(ptcx, &vtcxs.rgtcxc[itc].tcx, sizeof(struct TCX));
		vtcxs.rgtcxc[itc].itcNext = itcNext;
		vtcxs.rgtcxc[itc].fValid = fTrue;
		}

	return itcNext;

}


#endif /* DEBUGORNOTWIN */

#define	dxpTtpDr	(8*dxpBorderFti)

#ifdef DEBUGORNOTWIN
/*	I T C  G E T  T C X
/*
/*	Description: Given needed information, calculate the information needed
/*	to determine a cell's size and draw it (including any needed border).
/*	Call with ww == wwNil to get only border info, no size info.
/*
/*	RETURN VALUE is the itc of the next non-merged cell, this makes it easy
/*	to run through the "real" cells for display.
/*
/**/
#ifdef MAC
NATIVE ItcGetTcx(ww, ptap, itc, ptcx) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:ItcGetTcx %%Owner:tomsax */
HANDNATIVE C_ItcGetTcx(ww, ptap, itc, ptcx)
#endif /* !MAC */
struct TAP *ptap;
struct TCX *ptcx;
{
	struct WWD *pwwd;
	int itcMac;
	int xaFirst, xaLim;
	int itcNext;
	int dxa, dxp;
	Debug ( BOOL fRestarted = fFalse; )

LRestart:
			Assert ( 0 <= itc && itc <= ptap->itcMac );

	itcMac = ptap->itcMac;
	xaFirst = ptap->rgdxaCenter[itc];
	itcNext = itc + 1;

	if (itc == itcMac)
		{
		/* handle the fTtp cell */
		Assert(itc > 0);
		ptcx->brcLeft = ptap->rgtc[itc-1].brcRight;
		ptcx->brcRight =
				ptcx->brcTop =
				ptcx->brcBottom = brcNil;
		if (ww == wwNil)
			return itcNext;
		xaLim = xaFirst+DxaFromDxp(PwwdWw(ww),dxpTtpDr+1+DxFromBrc(ptcx->brcLeft,fTrue)/2);
		goto LCalcCellSize; /* skip around border calculation */
		}
	else  if ( ptap->rgtc[itc].fFirstMerged )
		{
		/* the first in a merged series */
		while ( itcNext < itcMac && ptap->rgtc[itcNext].fMerged )
			itcNext++;
		}
	else  if ( ptap->rgtc[itc].fMerged )
		{
		/* a cell in the middle of merged series, doesn't get displayed */
		xaLim = xaFirst;
		ptcx->brcTop = ptcx->brcBottom = ptcx->brcLeft = ptcx->brcRight = brcNone;
		goto LCalcCellSize; /* skip around border calculation */
		}

	xaLim = ptap->rgdxaCenter[itcNext];

	/* Get the various border codes.
	/**/
	if ( (ptcx->brcLeft = ptap->rgtc[itc].brcLeft) == brcNone)
		if (itc > 0)
			{
#ifdef DISABLE_DRM
			int itcT = itc;
			while (ptap->rgtc[--itcT].fMerged)
				;
			Assert(itcT >= 0);
#endif
			ptcx->brcLeft = ptap->rgtc[itc-1].brcRight;
			}
	if (itcNext >= itcMac || (ptcx->brcRight = ptap->rgtc[itcNext].brcLeft) == brcNone)
		ptcx->brcRight = ptap->rgtc[itcNext-1].brcRight;
	ptcx->brcTop = ptap->rgtc[itc].brcTop;
	ptcx->brcBottom = ptap->rgtc[itc].brcBottom;

LCalcCellSize:

	if (ww == wwNil)
		return itcNext;

	/* Compute the xp's from the xa's. Note that here is where we make
	/* the adjustment for the user's justification code.
	/**/
	pwwd=PwwdWw(ww);
	ptcx->xpCellLeft = DxpFromDxa(pwwd,xaFirst += ptap->dxaAdjust);
	ptcx->xpCellRight = DxpFromDxa(pwwd,xaLim += ptap->dxaAdjust);
	Assert (ptcx->xpCellLeft <= ptcx->xpCellRight);

	if (itc == itcMac)
		{
		ptcx->xpDrLeft = (ptcx->xpDrRight = DxpFromDxa(pwwd,xaLim)) - dxpTtpDr;
		Assert(ptcx->xpCellLeft+DxFromBrc(ptcx->brcLeft,fTrue)/2 <= ptcx->xpDrLeft);
		}
	else
		{
		if (ptap->rgtc[itc].fMerged)
			{
			ptcx->xpDrLeft = ptcx->xpDrRight = ptcx->xpCellLeft;
			ptcx->dxpOutRight = ptcx->dxpOutLeft = 0;
			goto LExit;
			}
		ptcx->xpDrLeft = DxpFromDxa(pwwd,min(xaLim,xaFirst+ptap->dxaGapHalf));
		ptcx->xpDrRight = DxpFromDxa(pwwd,max(xaFirst,xaLim-ptap->dxaGapHalf));
		}

	/* NOTE - for the vertical stripes, we call DxFromBrc with 
	/* fDrawDrs=fTrue so that we allocate room for frame lines. This
	/* assumes that dxaGap translates into at least one dxpBorderFti of gap.
	/* NOTE - Split the border down the rgdxaCenter lines so that uneven
	/* borders spill more over onto the left side, quantizing in dxpBorderFti
	/* units.
	/* CLEVERNESS WARNING: The trickery below assumes that
	/* DxFromBrc(brc,fTrue) returns either 1, 2, or 3 times dxpBorderFti.
	/**/
	ptcx->dxpOutLeft = ptcx->xpDrLeft - ptcx->xpCellLeft;
	if (DxFromBrc(ptcx->brcLeft,fTrue) > dxpBorderFti)
		ptcx->dxpOutLeft -= dxpBorderFti;

	if (itc < itcMac)
		{
		int dxpT;
		dxpT = DxFromBrc(ptcx->brcRight,fTrue);
		ptcx->dxpOutRight = ptcx->xpCellRight - ptcx->xpDrRight - dxpT;
		if (dxpT > dxpBorderFti)
			ptcx->dxpOutRight += dxpBorderFti;
		}
	else
		ptcx->dxpOutRight = 0;

	/* If the user has created a cell without enough room to hold the
	/* border (or potential frame line), shrink the cell.
	/* If this action causes the cell width to become negative,
	/* adjust the tap to make room for a wider cell.
	/**/

	/* In order to maintain enough room to draw the borders, we must
	/* maintain the two quantities
	/*		xpDrLeft - dxpOutLeft - xpCellLeft
	/*		xpCellRight - (xpDrRight + dxpRight)
	/**/
	if (ptcx->dxpOutLeft < 0)
		{
		ptcx->xpDrLeft -= ptcx->dxpOutLeft;
		ptcx->dxpOutLeft = 0;
		}
	if (ptcx->dxpOutRight < 0)
		{
		ptcx->xpDrRight += ptcx->dxpOutRight;
		ptcx->dxpOutRight = 0;
		}
	if ((dxp = ptcx->xpDrLeft - ptcx->xpDrRight) > 0)
		{
		Assert(itc < itcMac);

		/* Add one to the adjustment so that dxa/dxp conversion
		/* roundoff can't hose things.
		/**/
		dxa = DxaFromDxp(pwwd,dxp+1);

		for ( ; itcNext <= itcMac; ++itcNext )
			ptap->rgdxaCenter[itcNext] += dxa;
		Assert(!fRestarted); /* no infinite loops allowed */
		Debug ( fRestarted = fTrue; )

				/* it is now easier to re-compute with the adjusted TAP than to try
		/* to figure out how to adjust things correctly
		/**/
		goto LRestart;
		}
LExit:
	Assert (ptcx->dxpOutLeft >= 0 && ptcx->dxpOutRight >= 0);
	Assert (ptcx->xpCellLeft <= ptcx->xpDrLeft - ptcx->dxpOutLeft);
	Assert (ptcx->xpDrLeft <= ptcx->xpDrRight);
	Assert (ptcx->xpDrRight + ptcx->dxpOutRight <= ptcx->xpCellRight);

	return itcNext;

}


#endif /* DEBUGORNOTWIN */

/* G E T  R O W  B O R D E R S
/*
/* If fStrict, brc's must match up exactly. I.e., pass fFalse for displaying
/* the current borders, but pass fTrue when deciding what needs to get set.
/**/
/* %%Function:GetRowBorders %%Owner:tomsax */
GetRowBorders(rgbrc,ptap,itcFirst,itcLim,fStrict)
int rgbrc[], itcFirst, itcLim;
struct TAP *ptap;
BOOL fStrict;
{
	int brcTop, brcLeft, brcBottom, brcRight, brcInside;
	int itcNext;
	struct TCX tcx;
	BOOL fUseCache;
	int doc;
	CP cp;

	if (fUseCache = ptap == &vtapFetch)
		{
		doc = caTap.doc;
		cp = caTap.cpFirst;
		itcNext = ItcGetTcxCache(wwNil,doc,cp,ptap,itcFirst,&tcx);
		}
	else
		itcNext = ItcGetTcx(wwNil,ptap,itcFirst,&tcx);

	brcTop = tcx.brcTop;
	brcLeft = tcx.brcLeft;
	brcBottom = tcx.brcBottom;

	if (itcNext >= itcLim)
		{
		brcInside = brcNil;
		goto LExit;
		}

	if (fUseCache)
		itcNext = ItcGetTcxCache(wwNil,doc,cp,ptap,itcNext,&tcx);
	else
		itcNext = ItcGetTcx(wwNil,ptap,itcNext,&tcx);
	brcInside = tcx.brcLeft;
	while (fTrue)
		{
		if (tcx.brcTop != brcTop)
			brcTop = brcNil;
		if (tcx.brcBottom != brcBottom
				&& (fStrict || tcx.brcBottom != brcNone))
			brcBottom = brcNil;
		if (tcx.brcLeft != brcInside
				&& (fStrict || tcx.brcLeft != brcNone))
			brcInside = brcNil;
		if (itcNext >= itcLim)
			break;
		if (fUseCache)
			itcNext = ItcGetTcxCache(wwNil,doc,cp,ptap,itcNext,&tcx);
		else
			itcNext = ItcGetTcx(wwNil,ptap,itcNext,&tcx);
		}

LExit:
	rgbrc[ibrcRight] = tcx.brcRight;
	rgbrc[ibrcTop] = brcTop;
	rgbrc[ibrcLeft] = brcLeft;
	rgbrc[ibrcBottom] = brcBottom;
	rgbrc[ibrcInside] = brcInside;

}


#ifdef WIN
/* T  G E T  S T A T L I N E  T A B L E */
/*  code for tables from FCacheLine. return tYes to "continue" for loop,
	tNo to proceed normally, tMaybe to abort.
*/
/* %%Function:TGetStatlineTable %%Owner:tomsax */
int
TGetStatlineTable(doc, pcpScan, cp, pyaPp, plnn, cpTable)
int doc, *pyaPp, *plnn;
CP *pcpScan, cp, cpTable;
{
	extern struct CA caTap;
	int dyu, dyuAbove, dyuBelow, dya;

	CpFirstTap( doc, *pcpScan );
	if (FInCa( doc, cp, &caTap ))
		{	/* cursor is inside the table row */
		CacheTc(wwNil, doc, cp, fFalse, fFalse);
		Assert( FInCa( doc, cp, &vtcc.ca ) );
		*pcpScan = vtcc.ca.cpFirst;
		vlcb.fInsideCell = fTrue;
		return tYes; /* continue */
		}
	else
		{	/* cursor is beyond the table row */
		if (!FTableHeight(wwLayout, doc, cpTable, fTrue, fFalse, fFalse,
				&dyu, &dyuAbove, &dyuBelow))
			return tMaybe; /* numbers returned are garbage */
		dya = UMultDiv( dyu+dyuAbove+dyuBelow, czaInch, vfli.dyuInch );
		*pyaPp += dya;
		*plnn += dya / czaLine;
		*pcpScan = caTap.cpLim;
		return tNo;
		}
}


#endif /* WIN */


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Fetchtb_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Fetchtb_Last() */
