/* F I E L D C R . C */
/* Core functions related to fields */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "doc.h"
#include "format.h"
#include "ch.h"
#include "disp.h"
#include "sel.h"
#include "outline.h"
#include "screen.h"
#include "print.h"
#include "debug.h"

#ifdef DEBUG
#define FIELDFMT  /* get the dnflt */
#endif /* DEBUG */
#include "field.h"


#ifdef PROTOTYPE
#include "fieldfmt.cpt"
#endif /* PROTOTYPE */

#ifdef PCJ
/* #define DFORMULA */
#endif /* PCJ */

#ifdef DEBUG
CP C_DcpSkipFieldChPflcd();
CP C_CpVisibleCpField();
#endif /* DEBUG */



/* G L O B A L S */
int             vifldInsert = ifldNil;   /* insertion is into this field */
int             vwFieldCalc = 0;         /* (no)fields are being calculated */
CHAR            vrrut;                   /* result type */
int             vfCustomDTPic = -1;      /* existence if Date/time pics */
int		vcBtnFldClicks = 2;	 /* Clicks to activate button field */
struct CA	vcaCell;		 /* result of GetCellCps */


/* E X T E R N A L S */

extern struct FLI   vfli;
extern struct FMTSS vfmtss;
extern int          vdocFetch;
extern CP           vcpFetch;
extern struct CHP   vchpFetch;
extern int          vccpFetch;
extern CHAR HUGE    *vhpchFetch;
extern struct PAP   vpapFetch;
extern struct SEL   selCur;
extern int          fcmFetch;
extern BOOL         vfInsertMode;
extern CP           cpInsert;
extern CP			vcpFirstTablePara;
extern struct WWD **hwwdCur;
extern int          wwCur;
extern struct FFS  *vpffs;
extern struct DOD  **mpdochdod[];
extern char		(**vhgrpchr)[];
extern struct SCI	vsci;
extern struct PRI	vpri;
extern struct FTI	vfti;
extern struct FTI	vftiDxt;
extern int		vdocTemp;
extern struct CHP	vchpStc;
extern struct MERR	vmerr;
extern struct CA        caPara;

#ifdef DEBUG
int docLastFetchVisi = docNil;
CP cpFirstNextFetchVisi = cpNil;
CP CpNextVisiInOutline();
#endif /* DEBUG */

#ifdef DEBUG
/* I F L D  F R O M  D O C  C P */
/*  Searches the plcfld for the doc in reverse order.
	Returns the innermost field containing cp.  If fMatch then returns a valid
	field iff cp is one of the fields special characters (chFieldBegin,
	chFieldSeparate or chFieldEnd).  If cp is not in a live field or fMatch
	and cp is not a special field character then returns ifldNil.
*/

/* %%Function:C_IfldFromDocCp %%Owner:peterj */
HANDNATIVE C_IfldFromDocCp (doc, cp, fMatch)
int doc;
CP cp;
BOOL fMatch;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	int ifld;
	int cChBegin = 0;
	struct FLD fld;

	if (hplcfld == hNil)
		return ifldNil;

	/* largest i s.t. rgcp[i] <= cp */
	ifld = IInPlcCheck (hplcfld, cp);
	if (ifld < 0 || (fMatch && CpPlc( hplcfld, ifld ) != cp))
		return ifldNil;

	GetPlc( hplcfld, ifld, &fld );
	if (fld.ch == chFieldEnd && CpPlc(hplcfld, ifld) == cp)
		ifld--;

	while (ifld >= 0)
		{
		GetPlc( hplcfld, ifld, &fld );
		if (fld.ch == chFieldBegin && cChBegin == 0)
			break;

		if (fld.ch == chFieldBegin)
			cChBegin++;  /* nested field's begin */
		else  if (fld.ch == chFieldEnd)
			cChBegin--;  /* nested field */
		ifld--;
		}

	return ifld;
}


#endif /* DEBUG */


#ifdef DEBUG
/* I F L D  I N S E R T  D O C  C P */
/*  For an insertion point at doc, cp, what field is it in?  For this
	purpose (invalidation in insert mode) an insertion point is in a 
	field iff the field plc entry immediately preceeding cp is a field begin.
*/

/* %%Function:C_IfldInsertDocCp %%Owner:peterj */
HANDNATIVE C_IfldInsertDocCp (doc, cp)
int doc;
CP cp;

{
	int ifld;
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLD fld;

	if (hplcfld == hNil)
		return ifldNil;

	ifld = IInPlcRef (hplcfld, cp);
	if (--ifld >= 0)
		{
		GetPlc( hplcfld, ifld, &fld );
		if (fld.ch == chFieldBegin)
			return ifld;
		}

	return ifldNil;
}


#endif /* DEBUG */


#ifdef DEBUG
/* C O R R E C T  F  N E S T E D  B I T S */
/*  Correct all field fNested bits in the specified doc.
*/

/* %%Function:CorrectFNestedBits %%Owner:peterj */
HANDNATIVE CorrectFNestedBits(doc)
int doc;
{
	int cNested = 0;
	int ifld, ifldMac;
	struct PLC **hplcfld = PdodDoc(doc)->hplcfld;
	struct FLD fld;

	PdodDoc(doc)->fFldNestedValid = fTrue;
	if (hplcfld == hNil)
		return;

	ifldMac = IMacPlc(hplcfld);

	for (ifld = 0; ifld < ifldMac; ifld++)
		{
		GetPlc (hplcfld, ifld, &fld);
		if (fld.ch == chFieldBegin)
			cNested++;
		else  if (fld.ch == chFieldEnd)
			{
			if (--cNested)
				fld.fNested = fTrue;
			else
				fld.fNested = fFalse;
			PutPlcLast(hplcfld, ifld, &fld);
			}
		}

	Assert(!cNested);
}


#endif /* DEBUG */


#ifdef DEBUG
/* C H E C K  F  N E S T E D  B I T S */
/*  Assert that all fNested bits are correct
*/

/* %%Function:CheckFNestedBits %%Owner:peterj */
HANDNATIVE CheckFNestedBits(doc)
int doc;
{
	int cNested = 0;
	int ifld, ifldMac;
	struct PLC **hplcfld = PdodDoc(doc)->hplcfld;
	struct FLD fld;

	if (hplcfld == hNil || !PdodDoc(doc)->fFldNestedValid || (!vdbs.fCkDoc
			&& !vdbs.fCkFldIdle))
		return;

	ifldMac = IMacPlc(hplcfld);

	for (ifld = 0; ifld < ifldMac; ifld++)
		{
		GetPlc (hplcfld, ifld, &fld);
		if (fld.ch == chFieldBegin)
			cNested++;
		else  if (fld.ch == chFieldEnd)
			{
			if (--cNested)
				Assert(fld.fNested);
			else
				Assert(!fld.fNested);
			}
		}

	Assert(!cNested);
}


#endif /* DEBUG */





/* V I S I B L E  C H A R A C T E R  F E T C H  F U N C T I O N S */


#ifdef NOASM
/* I N I T  F V B */
/*  Initialize a Fetch Visible Block.
*/

/* %%Function:InitFvb %%Owner:peterj */
InitFvb (pfvb)
struct FVB *pfvb;

{
	/*  assumes: docNil == cp0 == NULL == fFalse == 0 */
	SetBytes (pfvb, 0, sizeof (struct FVB));
}


#endif /* NOASM */


#ifdef NOASM
/* I N I T  F V B  B U F S */
/*  Set up the buffers for a fetch visible block.
*/

/* %%Function:InitFvbBufs %%Owner:peterj */
InitFvbBufs (pfvb, rgch, cchMax, rgcr, ccrMax)
struct FVB *pfvb;
CHAR *rgch;
int cchMax;
struct CR *rgcr;
int ccrMax;

{
	pfvb->rgch = rgch;
	pfvb->cchMax = cchMax;
	pfvb->rgcr = rgcr;
	pfvb->ccrMax = ccrMax;
}


#endif /* NOASM */


#ifdef DEBUG
/* F E T C H  V I S I B L E  R G C H */
/*  Fetch a buffer full of characters from the doc.  Arguments are passes in
	an FVB.  Specifically: doc, cpFirst, cpLim -- range to be fetched,
	rgch, cchMax, cch -- buffer for chars, size of buffer & number of chars
	fetched, rgcr, ccrMax, ccr -- buffer for cp runs, size of buffer &
	number of cr's returned.  Note that both rgch & rgcr are required by
	this function.  Fetch stops when either buffer full of cpLim reached.
	cpFirst & fOverflow are set on return.  If fProps, runs in rgcr will
	have the same properties.  If ffe & ffeNested then if cpFirst is nested
	in an invisible portion of a field, you will still get those characters.
*/

/* %%Function:C_FetchVisibleRgch %%Owner:peterj */
HANDNATIVE C_FetchVisibleRgch (pfvb, fvc, fProps, ffe)
struct FVB * pfvb;
int fvc;
BOOL fProps;
int ffe;

{
	int icr = -1;
	CHAR * pch = pfvb->rgch;
	int ccp;
	int ccpFetch;
	CP cpCur = pfvb->cpFirst;

	pfvb->ccr = pfvb->cch = 0;

	while (cpCur < pfvb->cpLim && pfvb->cch < pfvb->cchMax)
		{
		FetchCpPccpVisible ((icr<0 ? pfvb->doc : docNil),
				cpCur, &ccpFetch, fvc, ffe);

		if (ccpFetch == 0)
			/* reached end of document */
			break;

		if (cpCur != vcpFetch || icr < 0 || fProps)
			{
			if (++icr >= pfvb->ccrMax)
				break;

			pfvb->ccr++;
			pfvb->rgcr [icr].cp = vcpFetch;
			pfvb->rgcr [icr].ccp = 0;
			}

		ccp = pfvb->cchMax - pfvb->cch;
		if (ccp > ccpFetch)
			ccp = ccpFetch;
		if (ccp > pfvb->cpLim - vcpFetch)
			ccp = CpMax (pfvb->cpLim - vcpFetch, cp0);

		bltbh(vhpchFetch, pch, ccp);
		pch += ccp;
		pfvb->cch += ccp;
		pfvb->rgcr [icr].ccp += ccp;

		cpCur = vcpFetch + ccp;
		}

	pfvb->cpFirst = cpCur;
	pfvb->fOverflow = (ccpFetch != 0 && cpCur < pfvb->cpLim);

}


#endif /* DEBUG */

int vdocFetchVisi = docNil;
CP vcpFetchVisi;
int vfvcFetchVisi;
CP vcpFetchFirstVisi;

#ifdef DEBUG
int vcVisiCalls = 0;
int vcVisiUsedCache = 0;

/* F E T C H  C P  P C C P  V I S I B L E */
/*  Fetch a run of characters which are visible according to fvc.  Fvc can
	be fvcScreen, which is what the user actually sees, or fvcInstructions
	or fvcResults which shows instructions/results always.  Takes vanished
	text, dead fields and fields instructions/results into consideration.

	*pccp will be filled with the number of characters fetched.  This is the
	correct number to use, NOT vccpFetch.  All other v*Fetch values are
	valid.

	For sequential fetch use doc==docNil and assure that *pccp has not
	been modified since previous fetch!

	If ffe & ffeNested then if cp is nested in a vanished portion of a field will
	still fetch cp and characters at the same nesting level (used when
	getting instructions).
*/

/* %%Function:C_FetchCpPccpVisible %%Owner:peterj */
HANDNATIVE C_FetchCpPccpVisible (doc, cp, pccp, fvc, ffe)
int doc;
CP cp;
int *pccp, fvc;
int ffe;
{
	int docFetch = doc;
	CP cpMac;
	CP cpIn;
	BOOL fUseCache = fFalse, fMayUseCache;

	Assert ((fvc & fvcmWw && !(fvc & ~fvcmWw)) || !(fvc & fvcmWw));
	Assert (wwMax <= fvcmWw && wwNil == 0);

	Debug(vcVisiCalls++);

	/*  sequential fetch, figure out where we are */
	if (doc == docNil)
		{
		doc = vdocFetch;
		cp = vcpFetch + *pccp;
		if (*pccp != vccpFetch)
			docFetch = doc;

		Assert (docLastFetchVisi == doc);
		Assert (cpFirstNextFetchVisi == cp);
		}
	else  if (!(ffe & ffeNested))
		/*  assures that cp is "visible" */
		cp = CpVisibleCpField (doc, cp, fvc, fFalse /* fIns */);

	cpIn = cp;

	/*  can the cache be used? */
	fMayUseCache = (cp < vcpFetchFirstVisi && doc == vdocFetchVisi &&
			fvc == vfvcFetchVisi);

#ifdef DFCPV
	CommSzLong(SzShared("FetchCpPccpVisible: cp = "), cp);
#endif

	Debug (docLastFetchVisi = doc);
	cpMac = CpMacDoc (doc);

	for (;;)
		{
LFetch:
		if (fMayUseCache && cp >= vcpFetchVisi)
			/* have entered cached area, jump forward */
			{
			cp = vcpFetchFirstVisi;
			fMayUseCache = fFalse;
			fUseCache = fTrue;
			docFetch = doc;
#ifdef DFCPV
			CommSz(SzShared("using cache!\r\n"));
#endif
			Debug(vcVisiUsedCache++);
			}

		if (cp >= cpMac)
			/*  end of document reached */
			{
			*pccp = 0;
			Debug (cpFirstNextFetchVisi = cp);
			goto LSetCache;
			}

		CachePara(doc, cp);
		FetchCp (docFetch, cp, fcmChars+fcmProps);

		/*  run of vanished text, skip whole run */
		if ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
				( vchpFetch.fSysVanish || 
				(!(fvc & fvcmWw) && !(fvc & fvcmProps)) || 
				(fvc & fvcmWw &&
				!(PwwdWw(fvc/*ww*/)->grpfvisi.fSeeHidden || 
				PwwdWw(fvc/*ww*/)->grpfvisi.fvisiShowAll)
				)
				)
				)
			{
			if (FInTableDocCp(doc, vcpFetch) &&
					vcpFetch + vccpFetch == caPara.cpLim &&
					*(vhpchFetch+vccpFetch-1) == chTable)
				{
				if (vccpFetch > 1)
					{
					cp += (vccpFetch-1);
					docFetch = doc;
					continue;
					}
				/* else: cannot hide chTable */
				}
			else
				{
LSkipRun:       
				cp += vccpFetch;
				docFetch = docNil;
				continue;
				}
			}

		/*  fSpec run--see how much, if any, we can use.
			CASES:
				1) cannot use any: first char in run is not visible.
					skip the correct number of chars and do random fetch.
				2) can use some: non-zero number of chars are visible,
					but some char in run is not visible. Set *pccp to
					include only visible ones.
				3) entire run is usable:  all fSpec characters are visible.
					return this run in its entirety.
		*/
		if (vchpFetch.fSpec)
			{
			CHAR HUGE * hpch = vhpchFetch;
			CHAR HUGE * hpchLim = hpch + vccpFetch;
			int ifld;
			CP dcp;
			struct FLCD flcd;

			if (ffe & ffeNoFSpec)
				goto LSkipRun;

			while (hpch < hpchLim)
				{
				ifld = IfldFromDocCp (doc, cp, fTrue);
				if (ifld != ifldNil)
					{
					GetIfldFlcd (doc, ifld, &flcd);
					if ((dcp = DcpSkipFieldChPflcd (*hpch, &flcd,
							FShowResultPflcdFvc (&flcd, fvc),
							(fvc&fvcmWw?1:0)/*fFetch*/)) != 0)
						{
						if (hpch == vhpchFetch)
							{
							/*  skip vanished portion of field */
							cp += dcp;
							docFetch = doc;
							goto LFetch;
							}
						/*  part of run useable */
						*pccp = hpch - vhpchFetch;
						Assert(cp == vcpFetch + *pccp);
						goto LCheckOutline;
						}
					}
				hpch++;
				cp++;
				}
			} /* if (vchpFetch.fSpec) */

		*pccp = vccpFetch;

LCheckOutline:
	/* See if we have to screen out some characters due to outline mode */
		if ((fvc & fvcmWw) && PwwdWw(fvc)->fOutline)
			{
			CP cpFirstInvisi = cpNil;

			cp = vcpFetch;

			if (!FCpVisiInOutline(fvc /*ww*/, doc, cp, *pccp, &cpFirstInvisi))
				{
				if (cpFirstInvisi == cp)
					{ /* all invisible */
					docFetch = doc;
					/* skip all hidden level text */
					cp = CpNextVisiInOutline(fvc /*ww*/, doc, cp);
					goto LFetch;
					}
				else
					{
					/* part of the run is useable */
					Assert(cpFirstInvisi != cpNil);
					Assert(cpFirstInvisi <= cp+*pccp);
					*pccp = cpFirstInvisi - cp;
					}
				}
			/* else all visible */

		/* this is to restore FetchCp globals because FCpVisiInOutline
			calls FetchCp with possibly something other than cp */
			FetchCpAndPara(doc, cp, fcmChars+fcmProps);
			}

		Debug (cpFirstNextFetchVisi = vcpFetch + *pccp);

LSetCache:
		if (!fUseCache || cpIn < vcpFetchVisi)
			{
			vdocFetchVisi = doc;
			vcpFetchVisi = cpIn;
			vfvcFetchVisi = fvc;
			vcpFetchFirstVisi = *pccp ? vcpFetch : cpMac;
			}

		return;

		}  /* for (;;) */
}


#endif /* DEBUG */



#ifdef DEBUG
/* G E T  C P  F I R S T  C P  L I M  D I S P L A Y  P A R A */
/*  Return cpLim, the smallest cp > cp such that:
	CpFormatFrom(cpLim) != (cpFirst = CpFormatFrom (cp))
*/
/* %%Function:C_GetCpFirstCpLimDisplayPara %%Owner:peterj */
HANDNATIVE void C_GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim)
int ww;
int doc;
CP cp;
CP *pcpFirst, *pcpLim;

{
	CP cpFrom;
	CP cpMac = CpMacDoc (doc);
	CP cpIn = cp;
	CP cpT, cpTable;
	int fAbs, fInTable, fInTableStart, ccp;
	int pap[cwPAPBaseScan];

	Assert (cp < cpMac);

	CachePara(doc, cp);
	if (fAbs = FAbsPap(doc, &vpapFetch))
		blt(&vpapFetch, pap, cwPAPBaseScan);
	fInTableStart = FInTableVPapFetch(doc, cp);
	cpFrom = *pcpFirst = CpFormatFrom (ww, doc, cp);
	do
		{
		cpT = cp;
		cp = CpVisibleCpField (doc, cp, ww, fFalse);
		CachePara (doc, cp);
		if (vpapFetch.fInTable)
			{
			/* table starts in middle of para (because of fields) */
			fInTable = FInTableDocCp(doc,cp);
			cpTable = vcpFirstTablePara;
			if (!fInTableStart)
				{
				FetchCpPccpVisible(doc, cpIn, &ccp, ww /* fvc */, 0 /* ffe */);
				if (vcpFetch >= cpTable && cpTable == CpVisibleCpField(doc, cpTable, ww, fTrue))
					{
					/* invisible characters in front of visible table */
					cp = cpTable;
					break;
					}
				}
			cp = cpTable;
			if (!FVisibleCp(ww, doc, cp) && cp > cpT)
				continue;
			/* second call to FInTable... must NOT pass &cp - it is to
				check for the case of all vanished text from cpT to the
				actual beginning of table (which will be cp) */
			if (!fInTable || !FInTableDocCp(doc,cpT))
				break;
			}
		else  if (fAbs != FAbsPap(doc, &vpapFetch) ||
				fAbs && !FMatchAbs(doc, &vpapFetch, pap))
			{
			/* apo change: from apo to non, non to apo, or apo type.
				if we've passed the starting cp, pull back cpLim and stop.
				otherwise, push cpFirst. */
			if (caPara.cpLim > cpIn)
				{
				cp = caPara.cpFirst; /* back up, don't include this one */
				break;
				}
			*pcpFirst = caPara.cpLim;
			}
		cp = caPara.cpLim;
		}
	while (cp < cpMac && cpFrom == CpFormatFrom (ww, doc, cp));

	*pcpLim = cp;
}


#endif /* DEBUG */


#ifdef DEBUG
/* C P  F O R M A T  F R O M */
/*  This function returns the first cp at or before cp from which
	formatLine's can begin.  
*/
/* %%Function:C_CpFormatFrom %%Owner:peterj */
HANDNATIVE CP C_CpFormatFrom (ww, doc, cp)
int ww;
int doc;
CP cp;

{
	GRPFVISI grpfvisi;
#ifdef NOASM
	CP CpVisibleCpField ();
#endif /* NOASM */
	int pap[cwPAPBaseScan];
	int tAbs, fTable;


	grpfvisi = PwwdWw(ww)->grpfvisi;

	/* case of a table nested within a field */
	if ((fTable = FInTableDocCp(doc, cp)) && vcpFirstTablePara != caPara.cpFirst &&
			CpVisibleCpField (doc, cp, ww/*fvcScreen*/, fFalse /* fIns */) == cp)
		{
		return vcpFirstTablePara;
		}

	for (tAbs = tNeg; ; )
		{
		while ((cp = CpVisibleBackCpField(doc, caPara.cpFirst, ww)) != caPara.cpFirst)
			CachePara (doc, cp);

		if (cp == cp0 || PwwdWw(ww)->fOutline) /* don't care for hidden eop in outline */
			break;
		if (tAbs == tNeg && (tAbs = FAbsPap(doc, &vpapFetch)))
			blt(&vpapFetch, pap, cwPAPBaseScan);

		CachePara (doc, cp-1);

		/* make sure the preceeding EOP is not vanished */
		FetchCp (doc, cp-1, fcmProps + fcmChars);

		if ((!vchpFetch.fVanish && !vchpFetch.fFldVanish) ||
				((grpfvisi.fSeeHidden || grpfvisi.fvisiShowAll || *vhpchFetch == chTable) &&
				!vchpFetch.fSysVanish))
			{
			break;
			}
		if (fTable && !FInTableVPapFetch(doc, cp-1) ||
				tAbs != FAbsPap(doc, &vpapFetch) ||
				tAbs && !FMatchAbs(doc, &vpapFetch, pap))
			{
			break;
			}
		}

	return cp;
}


#endif /* DEBUG */


#ifdef DEBUG
/* C P	V I S I B L E  B A C K	C P  F I E L D */
/* %%Function:C_CpVisibleBackCpField %%Owner:peterj */
HANDNATIVE CP C_CpVisibleBackCpField(doc, cp, fvc)
int doc;
CP cp;
int fvc;

{
	int ifld;
	struct FLCD flcd;

	while (cp > cp0 && CpVisibleCpField (doc, cp, fvc, fTrue) != cp)
		/*  cp is NOT visible by field rules, go back further */
		{
		ifld = IfldFromDocCp(doc, cp, fFalse);
		Assert(ifld != ifldNil);/* if ifldNil, field rules don't apply */
		GetIfldFlcd(doc, ifld, &flcd);
		if (flcd.cpFirst == cp || 
				flcd.cpFirst + flcd.dcpInst + flcd.dcpResult - 1 == cp)
			/* cp is cpFirst or cpLast of field */
			cp--;
		else  if (flcd.cpFirst + flcd.dcpInst > cp)
			/* cp is in instructions */
			cp = flcd.cpFirst;
		else
			/* cp is in results */
			cp = flcd.cpFirst + flcd.dcpInst - 1;
		}
	Assert(cp >= cp0);
	return cp;
}


#endif /* DEBUG */


#ifdef DEBUG
/* C P  V I S I B L E  C P  F I E L D */
/*  Return the first visible cp at or after cp.  Visibility is by field
	nesting only (not by character properties) and according to fvc.
	If fIns then returns an insertion point that should be visible.  If !fIns
	then a character that should be visible.
*/

/* %%Function:C_CpVisibleCpField %%Owner:peterj */
HANDNATIVE CP C_CpVisibleCpField(doc, cp, fvc, fIns)
int doc;
CP cp;
int fvc;
BOOL fIns;
{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	CP cp2;
	CP Cp1VisibleCpField();

	if (hplcfld == hNil)
		return cp;

	if (!PdodDoc(doc)->fFldNestedValid)
		CorrectFNestedBits(doc);

#ifdef DEBUG
	Assert(PdodDoc(doc)->fFldNestedValid);
	CheckFNestedBits(doc);
#endif /* DEBUG */

	while((cp2 = Cp1VisibleCpField(hplcfld, cp, fvc, fIns)) != cp)
		cp = cp2;

	return cp;
}


/* C P  1  V I S I B L E  C P  F I E L D */
/* %%Function:Cp1VisibleCpField %%Owner:peterj */
CP Cp1VisibleCpField(hplcfld, cp, fvc, fIns)
struct PLC **hplcfld;
CP cp;
int fvc;
BOOL fIns;
{

	int ifldBegin, ifldLim;
	CP cpNextVisi;
	CP cpEnd;
	int cChBegin;
	int ifld;
	int ifldBeginNext;
	int ifldLimM1Next;
	struct FLCD flcd;
	struct FLD fld;
#ifdef DEBUG 
	int ifldMac;
#endif /* DEBUG */

	Debug (ifldMac = IMacPlc(hplcfld));
	ifldLim = IInPlcCheck (hplcfld, cp);
	ifldLim++;
	ifldBegin = 0;

	/* search for ifldBegin, the largest ifldBegin which is not nested and
	   is <= cp. */
	if (ifldLim > 0)
		{
		for (ifldBegin = ifldLim - 1; ifldBegin > 0; ifldBegin--)
			{
			GetPlc( hplcfld, ifldBegin, &fld );
			if (fld.ch == chFieldEnd && !fld.fNested &&
					CpPlc(hplcfld, ifldBegin) < cp)
				{
				ifldBegin++;
				break;
				}
			}
		}

/* ifldBegin is the ifld of the last chFieldBegin we found at the current
   level while we were searching for chFieldEnd in the next higher level.
   If it is less than ifldLim (and so appears before cp), we need to
   see if cp lies within the field starting at ifldBegin, and if so, we
   have to determine if that field hides cp or not. */
	while ( ifldBegin < ifldLim )
		{
		ifldBeginNext = 0x7FFF;
		ifldLimM1Next = 0x7FFE;
		GetPlc( hplcfld, ifldBegin, &fld );

		/* At this point ifldBegin may be a misnomer.  Our loop
		   above and the code after LEndFound really set ifldBegin
		   to an ifldEnd+1.  That may be a ifldFieldSeparate,
		   however, so we check for that possibility now and
		   adjust ifldBegin if necessary. */
		if (fld.ch == chFieldSeparate)
			{
			ifldBegin++;
			continue;
			}

		Assert (fld.ch == chFieldBegin);

		cChBegin = 0;
		ifld = ifldBegin;

		/* find the limit of the field at ifldBegin.  Along the way
		   save away ifldBegin for the next level down, so if we do
		   have to go down another level we can same some work. */
		for (;;)
			{
			GetPlc( hplcfld, ++ifld, &fld );
			Assert (ifld < ifldMac);
			switch (fld.ch)
				{
			case chFieldBegin:
				cChBegin++;
				if (ifldBeginNext == 0x7FFF)
					ifldBeginNext = ifld;
				break;

			case chFieldEnd:
				if (cChBegin-- == 0)
					{
					cpEnd = CpPlc( hplcfld, ifld);
					goto LEndFound;
					}
				ifldLimM1Next = ifld;
				break;

#ifdef DEBUG
			default:
				Assert(fld.ch == chFieldSeparate);
#endif /* DEBUG */
				}
			}

LEndFound:
		if (cpEnd + 1 < cp+!fIns)
			{
			Assert (ifld > ifldBegin);
			ifldBegin = ifld + 1;
			continue;
			}

		/* Limit search for lower level if this level is not low enough */
		ifldLim = min(ifldLimM1Next + 1, ifldLim);
		FillIfldFlcd (hplcfld, ifldBegin, &flcd);
		Assert (flcd.cpFirst <= cp);

		if (!FCpVisibleInPflcd (cp, &flcd, &cpNextVisi, fvc, fIns))
			return cpNextVisi;

		ifldBegin = ifldBeginNext;
		}

	return cp;
}


#endif /* DEBUG */


#ifdef DEBUG
/* F  C P  V I S I B L E  I N  P F L C D */
/*  Returns true if cp is visible within pflcd according to fvc.  If false
	then *pcpNextVisi is the first visible cp after cp.
*/

/* %%Function:FCpVisibleInPflcd %%Owner:peterj */
FCpVisibleInPflcd (cp, pflcd, pcpNextVisi, fvc, fIns)
CP cp;
struct FLCD * pflcd;
CP * pcpNextVisi;
int fvc;
BOOL fIns;

{
	CP cpSeparator = pflcd->cpFirst + pflcd->dcpInst - 1 + fIns;
	CP cpLast = cpSeparator + pflcd->dcpResult;
	int ifld;

	Assert (fIns == 1 || fIns == 0);

	switch (WPflcdDispFvc (pflcd, fvc))
		{
	case -2:     /* show single character (display field) */
		*pcpNextVisi = cpLast;
		return (fIns && pflcd->cpFirst == cp) || cp == cpLast;

	case -1:     /* show nothing: cp not visible unless ins pt before
			cpFirst, cpLim is next visi */
		*pcpNextVisi = cpLast + 1;
		return (fIns && pflcd->cpFirst == cp);

	case 0:      /* show instructions: cp visible if < sep || last,
								next visi is last */
		*pcpNextVisi = cpLast;
		return (cp < cpSeparator || cp >= cpLast);

	case 1:      /* show result: cp visi if in result, next visi may
								be first of result or lim */
		if (cp == cpLast)
			*pcpNextVisi = cpLast + 1;

		else  if (cp <= cpSeparator - fIns && 
				(!fIns || cp > pflcd->cpFirst))
			*pcpNextVisi = cpSeparator + 1;

		else  /* in result */
			return fTrue;

		return fFalse;
		}

}


#endif /* DEBUG */


#ifdef DEBUG
/* W  P F L C D  D I S P  F V C */
/*  Returns a code indicating what portion of field pflcd should be
	displayed.  Code is:
		-2:  display a single character for the field (indicates display
								field in results mode).
		-1:  display nothing: indicates show results with either !fResult
								or no last result.
		0:   display field instructions: indicates show instructions mode.
		1:   display field result: indicates show results && fResult &&
								there is a last result.
*/

/* %%Function:WPflcdDispFvc %%Owner:peterj */
WPflcdDispFvc (pflcd, fvc)
struct FLCD * pflcd;
int fvc;

{
	if (FShowResultPflcdFvc (pflcd, fvc))
		if (pflcd->dcpResult && dnflt[pflcd->flt].fResult)
			{
			return 1 /*result*/;
			}

		else
			return dnflt[pflcd->flt].fDisplay ? -2/*disp*/ : -1/*nothing*/;

	else
		return 0 /*instructions*/;

}


#endif /* DEBUG */


#ifdef NOASM
/* C P  F R O M  I C H  P C R */

/* %%Function:CpFromIchPcr %%Owner:peterj */
CP CpFromIchPcr (ich, pcr, ccr)
int ich;
struct CR *pcr;
int ccr;

{
	while (ich >= pcr->ccp && --ccr > 0)
		ich -= (pcr++)->ccp;

	return pcr->cp + ich;
}


#endif /* NOASM */


#ifdef DEBUG
/* F  V I S I B L E  C P */
/*  Returns True iff cp should be displayed on the screen in ww (based
	on view prefs 
*/

/* %%Function:C_FVisibleCp %%Owner:peterj */
HANDNATIVE C_FVisibleCp (ww, doc, cp)
int ww;
int doc;
CP cp;

{
	BOOL fShowInOutline = fTrue;
	CP cpT;

	if (CpVisibleCpField (doc, cp, ww/*fvcScreen*/, fFalse /* fIns */) != cp)
		return fFalse;

	if (PwwdWw(ww)->fOutline)
		fShowInOutline = FCpVisiInOutline(ww, doc, cp, 1/*ccp*/, &cpT);

	CachePara (doc, cp);
	FetchCp (doc, cp, fcmProps);

	return fShowInOutline &&
			! ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
			(vchpFetch.fSysVanish ||
			!(PwwdWw(ww)->grpfvisi.fSeeHidden || PwwdWw(ww)->grpfvisi.fvisiShowAll) ));
}


#endif /* DEBUG */



#ifdef ENABLE  /* these are here if anyone needs them */
/* I C R  F R O M  I C H  P C R */
/* %%Function:IcrFromIchPcr %%Owner:peterj */
IcrFromIchPcr (ich, pcr, ccr)
int ich;
struct CR *pcr;
int ccr;

{
	struct CR *pcrFirst = pcr;

	while (ich >= pcr->ccp && --ccr > 0)
		ich -= (pcr++)->ccp;

	return pcr - pcrFirst;
}




/* C C R  T R U N C A T E  R G C R */
/* %%Function:CcrTruncateRgcr %%Owner:peterj */
CcrTruncateRgcr (rgcr, ichLast, ccr)
struct CR *rgcr;
int ichLast, ccr;

{
	int icr = 0;

	while (ichLast >= rgcr [icr].ccp && --ccr)
		ichLast -= rgcr [icr++].ccp;

	Assert (ccr > 0);

	rgcr [icr].ccp = ichLast + 1;

	return icr + 1;

}


#endif /* ENABLE */



/* O T H E R  U T I L I T I E S */


#ifdef DEBUG
/* F C P V I S I  I N  O U T L I N E  */
/* Given a cp range within the same pad, return true if all is visible
	WRT outline.  Also return the cpFirstInvis if part of it is not visible.
*/
/* %%Function:C_FCpVisiInOutline %%Owner:peterj */
HANDNATIVE C_FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi)
int ww;
int doc;
CP cp;
int ccp;
CP *pcpFirstInvisi;
{
	struct DOD	   *pdod;
	struct PLC **hplcpad;
	int 	    ipad;
	CP cpLim = cp + ccp;
	struct PAD	    pad;
	BOOL fShowInOutline;

	Assert (PwwdWw(ww)->fOutline);

	pdod = PdodDoc(doc);
	Assert(pdod->fMother);
	if (pdod->hplcpad == NULL || pdod->fOutlineDirty)
		{
		if (!FUpdateHplcpad(doc))
			return fTrue; /* HM */
		}
	hplcpad = PdodDoc(doc)->hplcpad;
	ipad = IInPlc(hplcpad, cp);
	Assert(ipad == IInPlc(hplcpad, cpLim-1));
	GetPlc(hplcpad, ipad, &pad);
	fShowInOutline = pad.fShow;

#ifdef NOTANYMORE
/* hidden paragraph mark can cause noshow paragraphs showing */
	if (!PwwdWw(ww)->grpfvisi.fSeeHidden &&
			!PwwdWw(ww)->grpfvisi.fvisiShowAll)
		{
		int ipadShow = ipad;
		CP cpT;
		while (!pad.fShow && ipadShow != 0)
			GetPlc(hplcpad, --ipadShow, &pad);

		cpT = CpPlc(hplcpad, ipadShow + 1) - 1;
		Assert(cpT >= cp0);

		if (ipad != ipadShow)
			{
			FetchCpAndPara(doc, cpT, fcmProps);
			if (vchpFetch.fVanish)
				fShowInOutline = fTrue;
			}
		}
#endif

	if (!fShowInOutline)
		{
		*pcpFirstInvisi = cp;
		}
	else  if (pad.fBody && PwwdWw(ww)->fEllip)
		{
		CachePara(doc, cp);
		FormatLine(ww, doc, CpFormatFrom(ww, doc, cp));
		fShowInOutline = (cpLim - 1) < vfli.cpMac;
		*pcpFirstInvisi = CpMax(cp, vfli.cpMac);
		}

	return fShowInOutline;
}


#endif /* DEBUG */



/* C P  L I M  N O  S P A C E S */
/* ****
*  Description: Truncate trailing spaces unless only spaces are in sel.
		Guarantees to look at the last 60 chars of the cache.
** **** */
/* %%Function:CpLimNoSpaces %%Owner:peterj */
CP CpLimNoSpaces(pca)
struct CA *pca;
{
	CHAR rgch[60];
	struct CR rgcr [60];
	struct FVB fvb;

	InitFvb (&fvb);
	InitFvbBufs (&fvb, rgch, 60, rgcr, 60);
	fvb.doc = pca->doc;
	fvb.cpLim = pca->cpLim;
	fvb.cpFirst = CpMax(pca->cpFirst, pca->cpLim - 60);

	FetchVisibleRgch (&fvb, wwCur/*fvcScreen*/, fFalse, fFalse);

	Assert(!fvb.fOverflow);

	while (fvb.cch-- > 0 && (rgch[fvb.cch] == chSpace || rgch [fvb.cch] == chNonBreakSpace))
		fvb.cpLim = CpFromIchPcr(fvb.cch, rgcr, fvb.ccr);
	return (fvb.cch < 0) ? pca->cpLim : fvb.cpLim;
}



/* F  L E G A L  S E L */
/*  Return true if cpFirst, cpLim is a legal selection WRT fields.

	USE FCkFldForDelete() for almost the same effect for debugging.
*/
/* %%Function:FLegalSel %%Owner:peterj */
FLegalSel (doc, cpFirst, cpLim)
int doc;
CP cpFirst, cpLim;
{
	struct CA ca1, ca2;

	PcaSet( &ca1, doc, cpFirst, cpLim );
	ca2 = ca1;
	AssureLegalSel( &ca1 );
	return !FNeRgch( &ca1, &ca2, sizeof (struct CA));
}


/* A S S U R E  L E G A L  S E L */
/*  Assure that *pca is a legal selection.  A selection is legal if it
	does not contain an unmatched field end, and if neither endpoint
	splits a CR-LF pair.  If selection is not legal, it is expanded to
	make it legal.

*/

/* %%Function:AssureLegalSel %%Owner:peterj */
EXPORT AssureLegalSel(pca)
struct CA *pca;
{

	Assert (pca->cpFirst <= pca->cpLim);
	Assert (pca->cpFirst >= cp0);
	Assert (pca->cpLim <= CpMac2Doc (pca->doc));

	if (DcpCa(pca) > 0)
		{
		/*  FPossibleDeadMismatch is true iff it is possible that cpFirst or
			cpLim are inside dead fields, based on the first & last character's
			properties and those just outside the selection. */

		if (FPossibleDeadMismatch(pca))
			/* grunt match field character method */
			ExpandToSameField(pca->doc, &pca->cpFirst, &pca->cpLim);

		else  if (PdodDoc (pca->doc)->hplcfld != hNil)
			/* efficient match field character method */
			ExpandToSameLiveField(pca);
		}

	/* CpFirstSty/CpLimSty don't use ww in styCRLF case */
	pca->cpFirst = CpFirstSty(wwNil, pca->doc, pca->cpFirst, styCRLF, fFalse);
	pca->cpLim = CpLimSty(wwNil, pca->doc, pca->cpLim, styCRLF, fTrue);
}


/* F  P O S S I B L E  D E A D  M I S M A T C H */
/*  Determine if it is possible that cpFirst or cpLim -1 are in dead fields.
	There is a possible mismatch if one of the end characters has fFldVanish
	*and* the character outside the range next to that character also has
	it.
*/

/* %%Function:FPossibleDeadMismatch %%Owner:peterj */
FPossibleDeadMismatch (pca)
struct CA *pca;
{
	int doc = pca->doc;

	Assert (pca->cpFirst >= cp0 && pca->cpFirst <= CpMac2Doc (doc));

	if (pca->cpFirst != cp0 &&
			FFldVanishDocCp (doc, pca->cpFirst, fTrue /*fFetch*/) &&
			FFldVanishDocCp (doc, pca->cpFirst - 1, fFalse))
		return fTrue;

	else
		return (FFldVanishDocCp (doc, pca->cpLim -1, fFalse) &&
				FFldVanishDocCp (doc, pca->cpLim, fFalse));

}


/* E X P A N D  T O  S A M E  L I V E  F I E L D */
/*  Scans the plcfld and adjusts pca to assure that it
	does not contain any unmatched field ends.
*/

/* %%Function:ExpandToSameLiveField %%Owner:peterj */
ExpandToSameLiveField(pca)
struct CA *pca;
{
	int doc = pca->doc;
	int cChBegin = 0, ifld, ifldBegin;
	struct PLC **hplcfld = PdodDoc(doc)->hplcfld;
	struct FLD fld;

	Assert (DcpCa(pca) > cp0);
	Assert (pca->cpFirst >= cp0);
	Assert (pca->cpLim <= CpMac2Doc (doc));

	ifld = ifldBegin = IInPlcRef (hplcfld, pca->cpFirst);
	Assert (ifld >= 0 && ifld <= IMacPlc( hplcfld ));

	if (ifld >= IMacPlc( hplcfld ))
		return;

	while (CpPlc(hplcfld, ifld) < pca->cpLim)
		{
		GetPlc(hplcfld, ifld, &fld);
		if (fld.ch == chFieldBegin)
			cChBegin++;
		else  if (fld.ch == chFieldEnd)
			cChBegin--;

		ifld++;

		if (cChBegin < 0)
			{  /* move cpFirst back */
			while (cChBegin < 0)
				{
				if (ifldBegin <= 0)
					{
					Assert(fFalse);
					break;
					}
				GetPlc(hplcfld, --ifldBegin, &fld);
				if (fld.ch == chFieldBegin)
					cChBegin++;
				else  if (fld.ch == chFieldEnd)
					cChBegin--;
				}
			Assert (!cChBegin);
			pca->cpFirst = CpPlc(hplcfld, ifldBegin);
			}
		}

	/* adjust if unbalanced */
	if (cChBegin > 0)
		{  /* advance cpLim */
		while (cChBegin > 0)
			{
			if (ifld >= IMacPlc(hplcfld))
				{
				Assert(fFalse);
				break;
				}
			GetPlc(hplcfld, ifld++, &fld);
			if (fld.ch == chFieldBegin)
				cChBegin++;
			else  if (fld.ch == chFieldEnd)
				cChBegin--;
			}
		Assert (!cChBegin);
		pca->cpLim = CpPlc(hplcfld, ifld - 1) + 1;
		}

}


/* F  F L D  V A N I S H  D O C  C P */
/*  Returns the state of fFldVanish for doc, cp.  Uses previously fetched
	information in an intellegent manner.
*/

/* %%Function:FFldVanishDocCp %%Owner:peterj */
FFldVanishDocCp (doc, cp, fFetch)
int doc;
CP cp;
BOOL fFetch;

{
	if (cp < cp0 || cp >= CpMacDoc (doc))
		{
		if (fFetch) vdocFetch = docNil; /* blow the cache */
		return fFalse;
		}

	if (fFetch || doc != vdocFetch || cp < vcpFetch
			|| cp >= vcpFetch + vccpFetch || fcmFetch != fcmProps+fcmChars)
		{
		FetchCpAndPara (doc, cp, fcmProps+fcmChars);
		}

	return vchpFetch.fFldVanish;

}


#ifdef DEBUG /* coded in line in fieldcrn.asm */
/* return the cpFirst of the next pad after cp that is fShow */
/* %%Function:CpNextVisiInOutline %%Owner:peterj */
HANDNATIVE CP CpNextVisiInOutline(ww, doc, cp)
int ww;
int doc;
CP cp;
{
	int ipad;
	int ipadMac;
	struct PAD pad;
	struct PLC **hplcpad = PdodDoc(doc)->hplcpad;


	Assert(PwwdWw(ww)->fOutline && hplcpad != hNil);

	ipad = IInPlc(hplcpad, cp);
	ipadMac = IMacPlc(hplcpad);

	while (++ipad < ipadMac)
		{
		GetPlc(hplcpad, ipad, &pad);
		if (pad.fShow)
			return CpPlc(hplcpad, ipad);
		}
	return CpMacDoc(doc);
}


#endif /* DEBUG */


/* A S S U R E	A L L  F I E L D S  P A R S E D */
/*  Scan through doc and parse all fields that are dirty
*/

/* %%Function:AssureAllFieldsParsed   %%Owner:peterj */
AssureAllFieldsParsed (doc)
int doc;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	int ifld = 0;
	struct FLD fld;

	if (hplcfld != hNil)
		{
		while (ifld < IMacPlc( hplcfld )) /* ifldMac may change! */
			{
			GetPlc( hplcfld, ifld, &fld );
			if (fld.ch == chFieldBegin && fld.fDirty)
				{
				FltParseDocCp (doc, CpPlc( hplcfld, ifld ),
						ifld, fFalse/*fChgView*/, fFalse /* fEnglish */);
				/* use same ifld in case killed */
				}
			else
				ifld++;
			}
		}

	/* parse sub-docs */
	if (!PdodDoc (doc)->fShort)
		{
		if (PdodDoc (doc)->docFtn != docNil)
			AssureAllFieldsParsed (PdodDoc (doc)->docFtn);
		if (PdodDoc (doc)->docAtn != docNil)
			AssureAllFieldsParsed (PdodDoc (doc)->docAtn);
		if (PdodDoc (doc)->docHdr != docNil)
			AssureAllFieldsParsed (PdodDoc (doc)->docHdr);
		if (PdodDoc (doc)->fDot)
			{
			if (PdodDoc (doc)->docGlsy != docNil)
				AssureAllFieldsParsed (PdodDoc (doc)->docGlsy);
			if (PdodDoc (doc)->docMcr != docNil)
				AssureAllFieldsParsed (PdodDoc (doc)->docMcr);
			}
		}
}



#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Fieldcr_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Fieldcr_Last() */
