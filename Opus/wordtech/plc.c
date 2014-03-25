/* P L C . C */

/* Note: For WIN this file is #included into util.c */


#ifndef WIN
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "debug.h"
#ifdef WIN
#include "heap.h"
#endif
#endif /* ~WIN */

#ifdef	MAC
NATIVE CopyRgbLcb(); /* WINIGNORE - unused in WIN */
#endif	/* MAC */

/* E X T E R N A L S */
extern struct FPC vfpc;
extern struct DRF *vpdrfHead;

#ifdef DEBUG
extern int vfCheckPlc;  /* check plc.iMac references */
int vcIInPlcCalls = 0;
int vcIInPlcHintSuccess = 0;
int vcIInPlcHintPlus = 0;
#endif

#ifdef WIN
extern int		dypCS;
#endif


/* the threshhold for sequential seatches */
#define		icpLinSrchThreshold	25


/***************************************************************
	T H E   W O R L D - F A M O U S   P L C    R O U T I N E S
***************************************************************/

#ifdef MAC /* in fetchn.asm if WIN */
/* %%Function:LprgcpForPlc %%Owner:davidlu */
HANDNATIVE CP far *LprgcpForPlc(pplc)
struct PLC *pplc;
{
	CP far *lprgcp;

	if (pplc->fExternal)
		lprgcp = (CP far *)LpFromHq(pplc->hqplce);
	else
		lprgcp = (CP far *)pplc->rgcp;
	return(lprgcp);
}


#endif /* MAC */

#ifdef MAC /* in fetchn.asm if WIN */
/* D C P  A D J U S T */
/* in NATIVE code, +DcpAdjust() should be coded open in most cases as:
if icp >= icpAdjust then cpInRegister += dcpAdjust;
*/
/* %%Function:DcpAdjust %%Owner:davidlu */
HANDNATIVE CP DcpAdjust(pplc, icp)
struct PLC *pplc;
int icp;
{
	CP cp = (icp < pplc->icpAdjust) ? cp0 : pplc->dcpAdjust;

	return (cp);
}


#endif /* MAC */

#ifdef MAC /* in fetchn.asm if WIN */
/* I  I N  P L C */
/* Binary search plc table for cp; return largest i s.t. rgcp[i] <= cp,
/* taking into account the cp adjusting.
/* NOTE, search will not work for cp's less than rgcp[0].
/**/
/* %%Function:IInPlc %%Owner:davidlu */
HANDNATIVE int IInPlc(hplc, cp)
struct PLC **hplc;
CP cp;
{
	struct PLC *pplc = *hplc;
	CP far *lprgcp = LprgcpForPlc(pplc);

#ifdef DEBUG
	if (WinMac(!pplc->fMult, fTrue)
			Assert(cp >= lprgcp[0] + DcpAdjust(pplc, 0));
#endif /* DEBUG */

			return IcpInRgcpAdjusted(lprgcp, pplc, cp);
}


#endif /* MAC */

#ifdef MAC /* in fetchn.asm if WIN */
/* I C P  I N  R G C P  A D J U S T E D */
/* Searches appropriate range of rgcp (depending on icpAdjust) for
/* cp, returns largest icp s.t.
/*	rgcp[icp] + DcpAdjust(icp) <= cp
/**/
/* %%Function:IcpInRgcpAdjusted %%Owner:davidlu */
HANDNATIVE IcpInRgcpAdjusted(lprgcp, pplc, cp)
CP far *lprgcp;
struct PLC *pplc;
CP cp;
{
	int icp, icpAdjustM1, icpAdjust, icpHint;
			long lcpHint, lcpAdjustM1, lcp;
			CP cpTrans;
			CP far *lpcp;

			Debug(vcIInPlcCalls++);
			icpHint = pplc->icpHint;
			Assert(icpHint <= pplc->iMac);

			if (pplc->iMac == 0)
			return 0;

			lcpHint = (long) icpHint;
			lpcp = (CP far *)((char far *)lprgcp + lcpHint*sizeof(CP));
			if (*lpcp + DcpAdjust(pplc, icpHint) <= cp &&
			cp < *(lpcp + 1) + DcpAdjust(pplc, icpHint + 1))
				{
		Debug(vcIInPlcHintSuccess++);
#ifdef WIN
				goto LMult;
#else /* !WIN */
				return icpHint;
#endif /* !WIN */
		}

	icpAdjust = pplc->icpAdjust;
			icpAdjustM1 = icpAdjust - 1;
			lcpAdjustM1 = icpAdjustM1;
			lpcp = (CP far *)((char far *)lprgcp + lcpAdjustM1 * sizeof(CP));
			cpTrans = cp - pplc->dcpAdjust;
			if (icpAdjust > 0)
				{
		if (cp <= *lpcp)
					{
			icp =  IcpInRgcp(lprgcp, icpAdjustM1, cp);
					goto LMult;
			}
		if (cpTrans < *(lpcp + 1))
					{
			icp = icpAdjustM1;
					goto LMult;
			}
		}
	icp = IcpInRgcp(lpcp + 1, pplc->iMac - icpAdjust,
			cpTrans) + icpAdjust;
LMult:
#ifdef WIN
			if (pplc->fMult)
				{
		lcp = icp;
				for (lpcp = (char far *)lprgcp + sizeof(CP) * lcp;
				icp > 0 && *(lpcp - 1) + DcpAdjust(pplc, icp - 1)
				== *lpcp + DcpAdjust(pplc, icp);
				icp--, lpcp--)
		}
#endif /* WIN */
	Debug(if (icpHint + 1 == icp) vcIInPlcHintPlus++);
			pplc->icpHint = icp;
			return (icp);
}


#endif /* MAC */

#ifdef MAC /* in fetchn.asm if WIN */
/* I  I N  P L C  2 */
/* Like IInPlc, but looks only at cp's after iFirst.
/* NOTE: search will not work for cp's less that rgcp[iFirst] + dcpAdjust(icp)
/**/
/* %%Function:IInPlc2 %%Owner:davidlu */
HANDNATIVE int IInPlc2(hplc, cp, iFirst)
struct PLC **hplc;
CP cp;
{
	int icpAdjustM1, icpAdjust;
			CP cpTrans;
			struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			CP far *lpcp, far *lpcpAdjust;
			int iMac = pplc->iMac;
			long lFirst, lMac, lcpAdjust;

			lFirst = iFirst;
			lMac = iMac;
#ifdef DEBUG
			if (WinMac(!pplc->fMult, fTrue)
				{
		lpcp = (char far *)lprgcp + sizeof(CP) * lFirst;
				Assert(!vfCheckPlc || cp >= *lpcp + DcpAdjust(pplc, iFirst));
		}
	lpcp = (char far *)lprgcp + sizeof(CP) * lMac;
			Assert(!vfCheckPlc || (iFirst<=iMac && cp<=*lpcp+DcpAdjust(pplc,iMac)) );
#endif /* DEBUG */

			lpcp = (char far *)lprgcp + sizeof(CP) * lMac;
			if (iMac == 0 || (cp == lpcp + DcpAdjust(pplc, iMac))) return iMac;

			icpAdjust = pplc->icpAdjust;
			Assert(icpAdjust <= iMac + 1);
			icpAdjustM1 = icpAdjust - 1;
			lcpAdjust = icpAdjust;
			lpcpAdjust = (char far *)lprgcp + lcpAdjust * sizeof(CP);
			cpTrans = cp - pplc->dcpAdjust;
			if (icpAdjust > 0)
				{
		if (cp <= *(lpcpAdjust - 1))
					{
			lpcp = (char far *)lprgcp + sizeof(CP) * lFirst;
					icpAdjust = IcpInRgcp(lpcp, icpAdjustM1 - iFirst, cp) + iFirst;
					goto LRet;
			}
		lpcpAdjust = (char far *)lprgcp + lcpAdjust * sizeof(CP);
				if (cpTrans < *lpcpAdjust)
					{
			icpAdjust = icpAdjustM1;
					goto LRet;
			}
		}
	if (iFirst > icpAdjust)
				{
		icpAdjust = iFirst;
				lcpAdjust = icpAdjust;
				lpcpAdjust = (char far *)lprgcp + lcpAdjust * sizeof(CP);
		}
	icpAdjust = IcpInRgcp(lpcpAdjust, iMac - icpAdjust, cpTrans) + icpAdjust;
LRet:
#ifdef WIN
			if (pplc->fMult)
				{
		lcpAdjust = icpAdjust;
				for (lpcp = (char far *)lprgcp + sizeof(CP) * lcpAdjust;
				icpAdjust < iMac
				&& *lpcp + DcpAdjust(pplc, icpAdjust)
				== *(lpcp + 1) + DcpAdjust(pplc, icpAdjust + 1);
				icpAdjust++, lpcp++)
		}
#endif /* WIN */
	return(icpAdjust);
}


#endif /* MAC */

#ifdef MAC /* in fetchn.asm if WIN */
/* I  I N  P L C  C H E C K */
/* as above, except -1 is returned if cp is not in range */
/* %%Function:IInPlcCheck %%Owner:davidlu */
HANDNATIVE int IInPlcCheck(hplc, cp)
struct PLC **hplc;
CP cp;
{
	int icpAdjustM1, icpAdjust;
			CP cpTrans;
			struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			CP far *lpcp;
			int iMac = pplc->iMac;
			long lMac;

			lMac = iMac;
			lpcp = (char far *)lprgcp + lMac * sizeof(CP);
			if (cp < lprgcp[0] + DcpAdjust(pplc, 0) || cp >= *lpcp + DcpAdjust(pplc, iMac))
			return (-1);

			icpAdjust = IcpInRgcpAdjusted(lprgcp, pplc, cp);
			return(icpAdjust);
}


#endif /* MAC */

#ifdef MAC /* in fetchn.asm if WIN */
/* I  I N  P L C  R E F  */
/* returns the smallest i s.t. rgcp[i] >= cp. returns -1 if cp > rgcp[iMac]
*/
/* %%Function:IInPlcRef %%Owner:davidlu */
HANDNATIVE int IInPlcRef(hplc, cpFirst)
struct PLC **hplc;
CP cpFirst;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			int icp;
			long lMac, lcp;
			CP far *lpcp;

			if (lprgcp[0] + DcpAdjust(pplc, 0) >= cpFirst)
			return 0;
			lMac = pplc->iMac;
			lpcp = (char far *)lprgcp + sizeof(CP) * lMac;
			if (*lpcp + DcpAdjust(pplc, pplc->iMac) < cpFirst)
			return -1;
			icp = IInPlc(hplc, cpFirst);
			lcp = icp;
			lpcp = (char far *)lprgcp + sizeof(CP) * lcp;
			while (*lpcp + DcpAdjust(pplc, i) < cpFirst)
				{
		lpcp++;
				i++;
		}
	return i;
}


#endif /* MAC */

#ifdef MAC /* not used if WIN */
/* I  I N  P L C  M U L T */
/* Binary search plc.rgcp for cp, yields icpFound; return smallest icp s.t.
/* rgcp[icp] + DcpAdjust(icp) == rgcp[icpFound] + DcpAdjust(icpFound)
/* That is, if the icp found is part of a string of identical cp's,
/* return the index to the first one in the set.
/**/
/* %%Function:IInPlcMult %%Owner:NOTUSED */
NATIVE int IInPlcMult(hplc, cp) /* WINIGNORE - unused in WIN */
struct PLC **hplc;
CP cp;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			CP far *lpcp;
			int icp;
			long lcp;

			if (lprgcp[0] + DcpAdjust(pplc, 0) >= cp)
			return 0;
			icp = IInPlc(hplc, cp);
			lcp = icp;

			for (lpcp = (char far *)lprgcp + sizeof(CP) * lcp;
			icp > 0 && *(lpcp - 1) + DcpAdjust(pplc, icp - 1)== *lpcp + DcpAdjust(pplc, icp);
			icp--, lpcp--)
			;
			return(icp);
}


#endif /* MAC */

#ifdef MAC /* not used if WIN */
/* I  I N  P L C  2  M U L T */
/* Do an IInPlc2 for cp w/iFirst, yields icpFound; return largest icp s.t.
/* rgcp[icp] + DcpAdjust(icp) == rgcp[icpFound] + DcpAdjust(icpFound)
/* That is, if the icp found is part of a string of identical cp's,
/* return the index to the last one in the set.
/**/
/* %%Function:IInPlc2Mult %%Owner:NOTUSED */
NATIVE int IInPlc2Mult(hplc, cp, iFirst) /* WINIGNORE - unused in WIN */
struct PLC **hplc;
CP cp;
{
	int icpAdjust;
			struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			int icp, iMac = IMacPlc(hplc);
			long lFirst;
			CP far *lpcp;
			long lcp;

			lFirst = iFirst;
			lpcp = (char far *)lprgcp + sizeof(CP) * lFirst;
			if (*lpcp + DcpAdjust(pplc, iFirst) >= cp)
				{
		Assert(iFirst == 0 || cp > *(lpcp-1)+DcpAdjust(pplc,iFirst-1));
				return iFirst;
		}
	icp = IInPlc2(hplc, cp, iFirst);
			lcp = icp;

			for (lpcp = (char far *)lprgcp + sizeof(CP) * lcp;
			icp < iMac && *lpcp + DcpAdjust(pplc, icp) == *(lpcp + 1) + DcpAdjust(pplc, icp+ 1);
			icp++, lpcp++)
			;
			return(icp);
}


#endif /* MAC */

#ifdef MAC /* not used if WIN */
/* I   I N   P L C   Q U I C K */
/* Do a fast look up, knowing that the plc is not adjusted.
/* used only in saveFast, in plc which is never edited */
/**/
/* %%Function:IInPlcQuick %%Owner:NOTUSED */
NATIVE int IInPlcQuick(hplc, cp) /* WINIGNORE - unused in WIN */
struct PLC **hplc;
CP cp;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			int iMac = pplc->iMac;
			int icp;

		/* two lines for native compiler */
	Assert((*hplc)->dcpAdjust == cp0);
			if (iMac == 0 || cp < lprgcp[0])
			return(-1);
			icp = IcpInRgcp(lprgcp, iMac, cp);
			return(icp);
}


#endif /* MAC */

#ifdef DEBUGORNOTWIN /* in fetchn.asm if WIN */
/* I C P  I N  R G C P */
/* note, search will not work for cp's less than rgcp[0] */
/* %%Function:IcpInRgcp %%Owner:davidlu */
HANDNATIVE int IcpInRgcp(lprgcp, icpLim, cp)
CP far *lprgcp;
CP cp; int icpLim;
{
	int icpMin = 0;
			long lcpLim = icpLim;
			CP far *lpcp;

#ifdef DEBUG
			lpcp = (char far *)lprgcp + sizeof(CP) * lcpLim;
			Assert(!vfCheckPlc || cp <= *lpcp);
#endif

			icpLim++;
			while (icpMin + 1 < icpLim)
				{
		long lcpGuess = (icpMin + icpLim) >> 1;
				lpcp = (char far *)lprgcp + sizeof(CP) * lcpGuess;
				if (*lpcp <= cp)
				icpMin = lcpGuess;
				else
			icpLim = lcpGuess;
		}
	return icpMin;
}


#endif /* MAC */

#ifdef MAC /* in editn.asm if WIN */
/* A D J U S T  H P L C */
/* does lazy adjustment of hplc by maintaining an adjustment interval in PLC.
	the adjustment interval is [pplc->icpAdjust, pplc->iMac]. The amount to
	bias the rgcp entries in this interval is pplc->dcpAdjust. */
/* %%Function:AdjustHplc %%Owner:davidlu */
HANDNATIVE AdjustHplc(hplc, cp, dcpNew, icpCertain)
struct PLC **hplc; CP cp, dcpNew;
int icpCertain; /* when not = -1, new adjustment index is known. */
{
	struct PLC *pplc;
			int icpFenceOld, icpAdjustNew, icpFenceAfter;
			CP  dcpOld, dcpAdjustAfter;
			int icpAdjustFirst, icpAdjustLim;
			CP  dcpAdjust;
			int iMac;
			CP far *lprgcp;

			if ((int)hplc == 0 || dcpNew == 0)
			return;

			pplc = *hplc;
			iMac = pplc->iMac;
#ifdef MAC
			/* If WIN we don't implement linear searches for IInPlc so
				we don't need this code either. (bcv 9-26-88) */
/* icpLinSrchThreshold is the icpLim below which IInPlc performs linear
	searches. When IInPlc does linear searches it's just as expensive as 
			AdjustHplcCps() so lazy adjustment doesn't pay.*/
	if (iMac < icpLinSrchThreshold && icpCertain == -1)
				{
		if (pplc->icpAdjust <= iMac)
				CompletelyAdjustHplcCps(hplc);
				AdjustHplcCps(hplc, cp, dcpNew, 0);
				return;
		}
#endif /* MAC */
	icpFenceOld = pplc->icpAdjust;
			dcpOld = pplc->dcpAdjust;
			/* this is verbose because of native compiler bug */
	if (icpCertain != -1)
			icpAdjustNew = icpCertain;
			else  if ((icpAdjustNew = IInPlcRef(hplc, cp)) == -1)
			return; /* nothing needs to be done */
	lprgcp = LprgcpForPlc(pplc);

			dcpAdjustAfter = dcpOld;
			dcpAdjust = dcpNew;
			icpFenceAfter = icpFenceOld;

		/* this is the heuristic which decides whether the rgcp range in the
	middle of the plc or the rgcp range at the tail of the plc is incremented
	by cpAdjust. Executing either branch will produce correct results. We may
	try to develop heuristic which accounts for change of locality of new
			adjustments. */
	if (iMac < 2 * max(icpFenceOld, icpAdjustNew) -
			min(icpFenceOld, icpAdjustNew))
				{
/* this branch executed when we are going to execute AddDcpToCps for the
			tail of the plc. */
		icpAdjustLim = iMac + 1;
				if (icpAdjustNew <= icpFenceOld)
					{
			dcpAdjustAfter = dcpNew;
					dcpAdjust = dcpOld;
					icpFenceAfter = icpAdjustNew;
			}
		}
	else
		{
/* this branch executed when we are going to execute AddDcpToCps for the
	middle part of the plc. The tail will be the new adjustment interval.
			It's dcp will be the sum of the new and old dcp. */
		dcpAdjustAfter = dcpOld + dcpNew;
				if (icpAdjustNew <= icpFenceOld)
				icpAdjustLim = icpFenceOld;
				else
			{
			icpAdjustLim = icpAdjustNew;
					dcpAdjust = dcpOld;
					icpFenceAfter = icpAdjustNew;
			}
		}
	icpAdjustFirst = icpAdjustNew + icpFenceOld -
			(pplc->icpAdjust = icpFenceAfter);
			pplc->dcpAdjust = dcpAdjustAfter;

			AddDcpToCps(lprgcp, icpAdjustFirst, icpAdjustLim, dcpAdjust);
}


#endif /* MAC */

#ifdef MAC /* in editn.asm if WIN */
/*  C O M P L E T E L Y  A D J U S T   H P L C  C P S */
/* adds pplc->dcpAdjust to rgcp entries in interval
	[pplc->icpAdjust, pplc->iMac] and invalidates the adjustment interval
	recorded in plc. Should be called for each document's plcs when we
	return to the Idle loop. */
/* %%Function:CompletelyAdjustHplcCps %%Owner:davidlu */
HANDNATIVE CompletelyAdjustHplcCps(hplc)
struct PLC **hplc;
{
	AdjustHplcCpsToLim(hplc, (*hplc)->iMac + 1);
}


#endif /* MAC */

#ifdef MAC /* in editn.asm if WIN */
/* A D J U S T  H P L C  C P S  T O  L I M */
/* adds pplc->dcpAdjust to rgcp entries in the interval
	[pplc->icpAdjust, icpLim). if rgcp[iMac] was adjusted we invalidate the
	plc's adjustment interval. */
/* %%Function:AdjustHplcCpsToLim %%Owner:davidlu */
HANDNATIVE AdjustHplcCpsToLim(hplc, icpLim)
struct PLC **hplc;
int icpLim;
{
	struct PLC *pplc;
			if (hplc == 0 || (*hplc)->dcpAdjust == cp0)
			return;
			pplc = *hplc;
			AddDcpToCps(LprgcpForPlc(pplc), pplc->icpAdjust, icpLim, pplc->dcpAdjust);
			if ((pplc->icpAdjust = icpLim) == pplc->iMac + 1)
			pplc->dcpAdjust = cp0;
}


#endif /* MAC */

#ifdef MAC	/* not used by WIN */
/* A D J U S T  H P L C  C P S  */
/* %%Function:AdjustHplcCps %%Owner:NOTUSED */
native AdjustHplcCps(hplc, cp, dcp, ipcdMin) /* WINIGNORE - unused in WIN */
struct PLC **hplc;
CP cp;
CP dcp;
int ipcdMin;
{
	CP far *lprgcp;
			CP far *lpcp, far *lpcpMin;
			struct PLC *pplc;

			if ((int)hplc == 0 || dcp == 0) return;

			pplc = *hplc;
			lprgcp = LprgcpForPlc(pplc);

			lpcp = &lprgcp[pplc->iMac];
			lpcpMin = &lprgcp[ipcdMin];
			while (lpcp >= lpcpMin && *lpcp >= cp)
			*(lpcp--) += dcp;
}


#endif /* MAC */

#ifdef DEBUGORNOTWIN /* in editn.asm if WIN */
/* A D J U S T  H P L C E D L  C P S  */
/* hand native */
/* adjusts all cp's with i >= 0 from (>=) cp by dcp. Backward scan
is for efficiency. */
#ifdef WIN
/* %%Function:C_AdjustHplcedlCps %%Owner:davidlu */
HANDNATIVE C_AdjustHplcedlCps(hplcedl, cp, dcp)
#else
/* %%Function:AdjustHplcedlCps %%Owner:NOTUSED */
NATIVE AdjustHplcedlCps(hplcedl, cp, dcp) /* WINIGNORE - "C_" if WIN */
#endif
struct PLCEDL **hplcedl;
CP cp;
CP dcp;
{
	CP cpAdjust; 
			int dl; 
			struct EDL edl;
			struct DR *pdr; 
			struct DRF drfFetch;

			if ((int)hplcedl == 0 || dcp == 0)
			return;
			cpAdjust = CpMacPlc(hplcedl); 
			dl = IMacPlc(hplcedl); 
			goto LTest; 

			while (dl >= 0)
				{
/* note: the first entry which is not adjusted may be still adjusted in its
interior. Eg. entry is from [2, 4), adjust >= 3, still has to adjust what is
		in the inside. */
		GetPlc(hplcedl, dl, &edl); 
				if (edl.hpldr != hNil)
					{
			int idr;
					int idrMac = (*((struct PLDR **)edl.hpldr))->idrMac;
					struct DR HUGE *hpdr = HpInPl(edl.hpldr, 0);
					Assert((*((struct PLDR **)edl.hpldr))->fExternal);
					for (idr = 0; idr < idrMac; hpdr++, idr++)
						{
				CP HUGE *hpcp, HUGE *hpcpMin;
						Assert(hpdr->hplcedl != hNil);
						if (hpdr->cpFirst >= cp)
							{
					if (vpdrfHead == NULL)
							hpdr->cpFirst += dcp;
							else
						{
						pdr = PdrFetch(edl.hpldr, idr, &drfFetch);
								pdr->cpFirst += dcp;
								FreePdrf(&drfFetch);
						}
					}
				Assert(hpdr->plcedl.fExternal);
						hpcpMin = HpOfHq(((struct PLC HUGE *)&hpdr->plcedl)->hqplce);
						hpcp = &hpcpMin[hpdr->plcedl.dlMac];
		/* copied from AdjustHplcCps */
				while (hpcp >= hpcpMin && *hpcp >= cp)
						*(hpcp--) += dcp;
				}
			}
		cpAdjust = CpPlc(hplcedl, dl); 
LTest:
				if (cpAdjust < cp)
				break; 
				cpAdjust = CpPlc(hplcedl, dl) + dcp;  
				if (cpAdjust < cp - 1)
				cpAdjust = cp - 1;
				PutCpPlc(hplcedl, dl--, cpAdjust); 
		}

#ifdef INTERNAL  
	CP *rgcp;
			CP *pcp, *pcpMin;
			struct PLC *pplc;
			struct EDL *pedl;
			struct DR *pdr;
			struct DRF drfFetch;

			if ((int)hplcedl == 0 || dcp == 0)
			return;

			pplc = *hplcedl;
			Assert(!pplc->fExternal);
			rgcp = &pplc->rgcp[0];

			pcp = &rgcp[pplc->iMac];
			pedl = (struct EDL *)(&rgcp[pplc->iMax]) + pplc->iMac;
			pcpMin = &rgcp[0];
			if (*pcp >= cp)
				{
		*(pcp--) += dcp;
				while (pcp >= pcpMin)
					{
/* note: the first entry which is not adjusted may be still adjusted in its
interior. Eg. entry is from [2, 4), adjust >= 3, still has to adjust what is
		in the inside. */
			pedl--;
					if (pedl->hpldr != hNil)
						{
				int idr;
						int idrMac = (*((struct PLDR **)(pedl->hpldr)))->idrMac;
						for (idr = 0; idr < idrMac; FreePdrf(&drfFetch), idr++)
							{
					pdr = PdrFetch(pedl->hpldr, idr, &drfFetch);
							if (pdr->cpFirst >= cp)
							pdr->cpFirst += dcp;
							AdjustHplcedlCps(pdr->hplcedl, cp, dcp);
					}
				}
			if (*pcp < cp) break;
					*(pcp--) += dcp;
			}
		}
#endif /* INTERNAL */
}


#endif /* DEBUGORNOTWIN */

#ifdef MAC /* in editn.asm if WIN */
/* A D D  D C P  T O  C P S  */
/* add dcp to all entries of lprgcp within the interval [icpFirst, icpLim). */
/* %%Function:AddDcpToCps %%Owner:davidlu */
HANDNATIVE AddDcpToCps(lprgcp, icpFirst, icpLim, dcp)
CP far *lprgcp;
int icpFirst;
int icpLim;
CP dcp;
{
	CP far *lpcp, far *lpcpLim;
			long lcpFirst, lcpLim;

			lcpFirst = icpFirst;
			lcpLim = icpLim;
			for (lpcp = (char far *)lprgcp + sizeof(CP) * lcpFirst,
			lpcpLim = (char far *)lprgcp + sizeof(CP) * lcpLim;
			lpcp < lpcpLim;  lpcp++)
			*lpcp += dcp;
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* I  M A C  P L C */
/* %%Function:IMacPlc %%Owner:davidlu */
HANDNATIVE int IMacPlc(hplc)
struct PLC **hplc;
{
	if (hplc == hNil)
			return 0;

			/* scc has an hplcedl that is not a true h */
	AssertH(hplc);
			return (*hplc)->iMac;
}


#endif /* MAC */

/* P U T  I  M A C  P L C */
/* Notice that PutIMacPlc calls CompletelyAdjustHplcCps, which unwinds
	lazy adjustment. PutIMacPlc should not be used in mainline editing 
	routines since this would remove the benefit of lazy adjustment.
	It is used by routines that construct or delete PLC entries
	in non-standard ways.  */
/* %%Function:PutIMacPlc %%Owner:davidlu */
NATIVE PutIMacPlc(hplc, iMac)
struct PLC **hplc;
int iMac;
{
	struct PLC *pplc;

		/* users of PutIMacPlc frequently add new entries immediately after the old
	iMac entry and then call PutIMacPlc to register them as actually belonging
	to the plc. The old adjustment interval has to be unwound so the newly
			added CP are not adjusted. */
	if (iMac)
			CompletelyAdjustHplcCps(hplc);

			pplc = *hplc;
			pplc->iMac = iMac;
			pplc->icpAdjust = iMac + 1;
			if (pplc->icpHint > iMac)
			pplc->icpHint = 0;
}


#ifdef MAC /* in resident.asm if WIN */
/* G E T  P L C */
/* %%Function:GetPlc %%Owner:davidlu */
HANDNATIVE GetPlc(hplc, i, pchFoo)
struct PLC **hplc;
char *pchFoo;
{
		/* also update vfpc with computed values for subsequent PutPlcLast call */
	struct PLC *pplc = *(vfpc.hplc = hplc);
			CP far *lprgcp = LprgcpForPlc(pplc);
			char far *lpfoo;
			long iT;
			long lMax;

			Assert(!vfCheckPlc || (i >= 0 && i < pplc->iMac));

			vfpc.cbFoo = pplc->cb;

			iT = i;
			lMax = pplc->iMax;
			bltbx(lpfoo = ((char far *)lprgcp + sizeof(CP) * lMax) + (vfpc.cbFoo * iT),
			(char far *)(vfpc.pchFoo = pchFoo), vfpc.cbFoo);
			vfpc.bfoo = (long)lpfoo;
			vfpc.bfoo -= (long)lprgcp;
			Debug(vfpc.ifoo = i);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P U T  P L C */
/* %%Function:PutPlc %%Owner:davidlu */
HANDNATIVE PutPlc(hplc, i, pchFoo)
struct PLC **hplc;
char *pchFoo;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			int cbPlc = pplc->cb;
			long iT;
			long lMax;

			Assert(!vfCheckPlc || (i >= 0 && i < pplc->iMac));

			iT = i;
			lMax = pplc->iMax;
			bltbx((char far *)pchFoo, ((char far *)lprgcp + sizeof(CP) * lMax) + (cbPlc * iT), cbPlc);
}


#endif /* MAC */

#ifdef DEBUG
/* P U T  P L C  L A S T  D E B U G  P R O C  - debug version of PutPlcLast */
/* %%Function:PutPlcLastDebugProc %%Owner:davidlu */
NATIVE PutPlcLastDebugProc(hplc, i, pchFoo) /* WINIGNORE - DEBUG only */
struct PLC **hplc;
int i;
char *pchFoo;
{
	long iMaxT, iT;
			/* use cached info in vfpc for hplc, pchFoo, bfoo, and cbFoo */
			/* first, assert that vfpc is correct */
	Assert(hplc == vfpc.hplc && i == vfpc.ifoo && pchFoo == vfpc.pchFoo);
			Assert(!vfCheckPlc || i < IMacPlc(hplc));
			iT = i;
			iMaxT = (*hplc)->iMax;
#ifndef WIN	/* Does not compile for WIN due to CS native bug (bl) */
			Assert(vfpc.bfoo - iMaxT*sizeof(CP) - iT*vfpc.cbFoo == 0L);
#endif


			PutPlcLastProc();
}


#endif /* DEBUG */

#ifdef MAC /* in resident.asm if WIN */
/* P U T  P L C  L A S T  P R O C */
/* %%Function:PutPlcLastProc %%Owner:davidlu */
HANDNATIVE PutPlcLastProc()
{
	CP far *lprgcp = LprgcpForPlc(*vfpc.hplc);
			/* use cached info in vfpc for hplc, pchFoo, bfoo, and cbFoo */
	bltbx((char far *)vfpc.pchFoo, (char far *) lprgcp + vfpc.bfoo, vfpc.cbFoo);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* Q  I N  P L C */
/* returns far pointer to i'th foo in plc */
/* %%Function:QInPlc %%Owner:davidlu */
HANDNATIVE char far *QInPlc(hplc, i)
struct PLC **hplc;
int i;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			char far *qfoo;
			long iT;	
			long lMax;

			Assert(!vfCheckPlc || (i >= 0 && i < (pplc)->iMac));
			iT = i;
			lMax = pplc->iMax;
			qfoo = ((char far *)lprgcp + sizeof(CP) * lMax) + iT * pplc->cb;
			return(qfoo);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C P  P L C */
/* %%Function:CpPlc %%Owner:davidlu */
HANDNATIVE CP CpPlc(hplc, i)
struct PLC **hplc;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			CP cp;
			long lT;
			CP far *lpcp;

			Assert(!vfCheckPlc || (i >= 0 && i <= (*hplc)->iMac));
			lT = i;
			lpcp = (char far *)lprgcp + lT * sizeof(CP);
			cp = *lpcp + DcpAdjust(pplc, i);
			return(cp);
}


#endif /* MAC */

/* C P  M A C  P L C  */
/* %%Function:CpMacPlc %%Owner:davidlu */
NATIVE CP CpMacPlc(hplc)
struct PLC **hplc;
{
	return (CpPlc(hplc, IMacPlc(hplc)));
}


#ifdef MAC /* in resident.asm if WIN */
/* P U T  C P  P L C */
/* %%Function:PutCpPlc %%Owner:davidlu */
HANDNATIVE int PutCpPlc(hplc, i, cp)
struct PLC **hplc;
CP cp;
{
	struct PLC *pplc = *hplc;
			CP far *lprgcp = LprgcpForPlc(pplc);
			CP far *lpcp;
			long l;

			Assert(!vfCheckPlc || (i >= 0 && i <= (pplc)->iMac));
			l = i;
			lpcp = (char far *)lprgcp + sizeof(CP) * l;
			*lpcp = cp - DcpAdjust(pplc, i);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C O P Y  M U L T  P L C */
/* this routine can copy a block of entries from one plc to another (needs
separate base registers in 8086 architecture.
Dcp optionally (!=0) added to the copied cp's.
di (0 or 1) is optional displacement for cp indices. In some plc's it is
necessary to move cp[i + 1] with foo[i].
dc (0 or 1) is optional extra count for cp copying. In some plc's it is
necessary to move one more cp's than foo's.
*/
/* %%Function:CopyMultPlc %%Owner:davidlu */
HANDNATIVE CopyMultPlc(cFoo, hplcSrc, ifooSrc, hplcDest, ifooDest, dcp, di, dc)
int cFoo;
struct PLC **hplcSrc, **hplcDest;
int ifooSrc, ifooDest;
CP dcp;
int di, dc;
{

#ifdef WIN
/*There are some serious problems with this source code for WIN.

	It is possible that the call later on to LprgcpForPlc for hplcDest
	would force out the sb for hplcSrc.  This would happen if now
	sbSrc was oldest on the LRU list but still swapped in, and sbDest
	was swapped out.  In that event ReloadSb would not be called for
	sbSrc, the LRU entry would not get updated, and the call to ReloadSb
	for sbDest would swap out sbSrc.

	In the assembler version to work around this LMEM quirk we call
	LprgpcForPlc(*hplcDest) before making the call for hplcSrc.

	Another difference from the C source is that we also don't call
	LprgcpForPlc until after the calls to AdjustHplcCpsToLim
	are performed (since those calls could invalidate the LP's).

	I have not changed the C version however since it is always ifdef'ed
	out for WIN and I don't want to give the MAC guys compiler errors.
	(bv 10-4-88).
		*/
#endif /* WIN */
	CP far *lprgcpSrc = LprgcpForPlc(*hplcSrc);
			CP far *lprgcpDest = LprgcpForPlc(*hplcDest);
			long lcb;
			int cb, cBlt;
			long lfooSrc, lfooDest;

#ifdef DEBUG
			if (cFoo == 0)
			return; /* so that QInPlc will not reach beyond mac */
#endif
			/* transfer foo's */
	lcb = (*hplcSrc)->cb;
			lcb *= cFoo;
			CopyRgbLcb(QInPlc(hplcSrc, ifooSrc), QInPlc(hplcDest, ifooDest), lcb);

			ifooSrc += di; ifooDest += di;
			cFoo += dc;
		/* unwind the fenced PLC optimization, so that cps are accurate. */
	if ((*hplcDest)->icpAdjust < ifooDest)
			AdjustHplcCpsToLim(hplcDest, ifooDest);
			if ((*hplcSrc)->icpAdjust < ifooSrc + cFoo)
			AdjustHplcCpsToLim(hplcSrc, ifooSrc + cFoo);

		/* we must move the destination adjustment fence past the end of the range
			we're going to write over. */
	if ((*hplcDest)->icpAdjust < ifooDest + cFoo)
			(*hplcDest)->icpAdjust = ifooDest + cFoo;

			/* transfer cp's */
	lfooSrc = ifooSrc;
			lfooDest = ifooDest;
			lcb = cFoo;
			lcb *= sizeof(CP);
			CopyRgbLcb((char far *)(lprgcpSrc) + lfooSrc * sizeof(CP),
			(char far *)(lprgcpDest) + lfooDest * sizeof(CP),
			lcb);

			if (dcp != cp0)
			while (cFoo--)
				{
		PutCpPlc(hplcDest, ifooDest, CpPlc(hplcDest, ifooDest) + dcp);
				ifooDest++;
		}
}


#endif /* MAC */

#ifdef MAC /* In resident.asm if WIN */
/* P  I N  P L  */
/* returns near pointer to i'th foo in pl */
/* %%Function:PInPl %%Owner:davidlu */
HANDNATIVE char  *PInPl(hpl, i)
struct PL **hpl;
int i;
{
	struct PL *ppl = *hpl;
			char *pfoo;
			long iT;

			Assert(i >= 0 && i < (*hpl)->iMac);
			iT = i;
			pfoo = ((char  *)ppl + ppl->brgfoo) + iT * ppl->cb;
			return(pfoo);
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* B L T  I N  P L C */
/* a blt within a plc for portability */
/* blt "c" (bpm==bpmCps?CPs:FOOs)
/* ("dicp" CP units) + (difoo FOO units) forward
/* beginning at position "i".
/**/
/* %%Function:BltInPlc %%Owner:davidlu */
HANDNATIVE BltInPlc(bpm, hplc, i, dicp, difoo, c)
int bpm;
struct PLC **hplc;
uns i, c;
int dicp, difoo;
{
	long	ib, dib, lcb, lcbFoo;
			struct PLC *pplc;
#ifdef MAC
			Debug(LoadSeg(BltbxProc));
#endif /* MAC */

			pplc = *hplc;
			ib = i;
			dib = dicp;
			dib <<= 2;
			lcb = c;
			if (bpm == bpmCp)
				{
		Assert(difoo == 0);
				ib <<= 2;
				lcb <<= 2;
		}
	else
		{
		ib *= pplc->cb;
				ib += pplc->iMax << 2;
				lcbFoo = pplc->cb;
				lcb *= lcbFoo;
				dib += difoo*lcbFoo;
		}
	if (!pplc->fExternal)
				{
		char *pchFrom = (char *)pplc + cbPLCBase + ib;
				/* can't blt that much in near mem */
		Assert((ib & 0xFFFF0000) == 0 && -32767 <= dib && dib <= 32768
				&& (lcb & 0xffff0000) == 0L);
				bltb(pchFrom, pchFrom + dib, (uns)lcb);
		}
	else
		{
		REGS regs;
				char HUGE *hpchFrom = (char HUGE *)*pplc->hqplce + ib;
				CopyRgbLcb(hpchFrom, hpchFrom + dib, lcb);
		}
}


#endif /* MAC */

/* I N I T  P L C E D L */
/* Just sets bytes to zero for us */
/* %%Function:InitPlcedl %%Owner:davidlu */
EXPORT InitPlcedl(hplcedl, dlStart, cdl)
struct PLCEDL **hplcedl;
int dlStart, cdl;
{
	struct PLCEDL *pplcedl;
			int far *qwBase; 
			long lwCP;

			pplcedl = *hplcedl;
			lwCP = sizeof(CP)/sizeof(int);

			qwBase = (!pplcedl->fExternal) ? 
			(int far *)pplcedl + CwFromCch(cbPLCBase) :
			LpFromHq(((struct PLC *)pplcedl)->hqplce); 

			bltcx(0, (int far *)qwBase + pplcedl->dlMax * lwCP +
			dlStart * ((long)cwEDL),
			cdl * cwEDL);
}


#ifdef	MAC
/***************************/
/* %%Function:LpInitPlc %%Owner:NOTUSED */
native long LpInitPlc(lpmh4) /* WINIGNORE - unused in WIN */
char far *lpmh4;
{
		/* fixed segment initialization routine */
	return(0L);
}


/******
	WARNING: LpInitXxxx must be the last routine in this module
	because it is a fixed segment
******/
#endif	/* MAC */
