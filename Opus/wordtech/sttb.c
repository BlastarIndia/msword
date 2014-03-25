#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "doc.h"
#include "file.h"


#ifdef PROTOTYPE
#include "sttb.cpt"
#endif /* PROTOTYPE */

extern struct MERR      vmerr;
extern CHAR             stEmpty[];

#ifdef WIN
extern int vfInCommit;
#else
#define vfInCommit fFalse   /* Mac does not have this (yet?) */
#endif /* WIN */

struct STTB	HUGE	*vhpsttbSort;  /* sttb being sorted */

/* %%Function:HpbstFirstSttb %%Owner:davidlu */
int HUGE *HpbstFirstSttb(hsttb)
struct STTB **hsttb;
{
	struct STTB *psttb = *hsttb;

	if (!psttb->fExternal)
		return (int HUGE *)psttb->rgbst;
	else
		return (int HUGE *)HpOfHq(psttb->hqrgbst);
}


/* H S T T B  I N I T  1 */
/* %%Function:HsttbInit1 %%Owner:davidlu */
struct STTB **HsttbInit1(cwEstimate, ibstMax, cbExtra, fExternal, fStyleRules)
int cwEstimate;
int ibstMax;
int cbExtra;
int fExternal;
int fStyleRules;
{
	/* WARNING: if STTB structure or size changes, update FInitCmds and mkcmd.c (WIN) */
	struct STTB **hsttb, *psttb;

	Assert(!vfInCommit);

	hsttb = (struct STTB **) HAllocateCw(!fExternal ? (cwSTTBBase +  max(ibstMax, cwEstimate)) : cwSTTB);
	if (hsttb == hNil)
		return (hNil);
	psttb = *hsttb;
	psttb->fExternal = fExternal;
	psttb->bMax = ibstMax * sizeof(int);
	psttb->cbExtra = cbExtra;
	psttb->ibstMac = 0;
	psttb->ibstMax = ibstMax;
	psttb->fStyleRules = fStyleRules;
	psttb->fNoShrink = fFalse;
	if (fExternal)
		{
		if (((*hsttb)->hqrgbst =    /* HM */
				HqAllocLcb((long)max(cwEstimate * 2, max(psttb->bMax, 8)))) == hqNil)
			{
			FreeH(hsttb);
			return (hNil);
			}
		}
	return(hsttb);
}


/* H S T T B  I N I T */
/* %%Function:HsttbInit %%Owner:davidlu */
struct STTB **HsttbInit(cwEstimate, fExternal)
int cwEstimate;
int fExternal;
{
	return HsttbInit1(cwEstimate, 0, 0, fExternal, fFalse);
}


/* %%Function:FreePhsttb %%Owner:davidlu */
FreePhsttb(phsttb)
struct STTB ***phsttb;
{
	if (*phsttb != hNil)
		{
		FreeHsttb(*phsttb);
		*phsttb = hNil;
		}
}


/* %%Function:FreeHsttb %%Owner:davidlu */
FreeHsttb(hsttb)
struct STTB **hsttb;
{
	struct STTB *psttb;
	if (hsttb == hNil)
		return;
	psttb = *hsttb;
	if (psttb->fExternal && psttb->hqrgbst != hqNil)
		FreeHq(psttb->hqrgbst);
	FreeH(hsttb);
}


/* C L O S E  U P  S T T B */
/* Remove unneeded space from hsttb.
*/
/* %%Function:CloseUpSttb %%Owner:davidlu */
EXPORT CloseUpSttb(hsttb)
struct STTB **hsttb;
{
	struct STTB *psttb = *hsttb;

	AssertH(hsttb);
	Assert(!vfInCommit);

	psttb->fNoShrink = fFalse;
	if (psttb->fExternal)
		{
		HQ hq = psttb->hqrgbst;
		AssertDo(FChngSizePhqLcb(&hq, (long)psttb->bMax));
		(*hsttb)->hqrgbst = hq;
		}
	else
		AssertDo(FChngSizeHCw(hsttb, CwFromCch(psttb->bMax+cbSTTBBase), 
				fTrue));
}


/* F  S T R E T C H  S T T B */
/*  Stretch sttb appropriately for cst st's to be added each with
cb characters (cb does not include count byte, cbExtra or bst) 
WARNING: the stretch is done by growing the heap object without storing any
information about the growth.  A subsequent call to add or delete a string will
reset the sttb to it's "correct" size.  After that nothing is guaranteed if 
any allocations are done to other heap objects.
*/
/* %%Function:FStretchSttb %%Owner:davidlu */
FStretchSttb(hsttb, cst, cb)
struct STTB **hsttb;
uns cst, cb;
{
	return FStretchSttbCb(hsttb, cst * (3 + cb + (*hsttb)->cbExtra));
}


/* F  S T R E T C H  S T T B  C B */
/* %%Function:FStretchSttbCb %%Owner:davidlu */
EXPORT FStretchSttbCb(hsttb, cb)
struct STTB **hsttb;
uns cb;
{
	uns cbWanted;
	struct STTB *psttb;

	if (!hsttb)
		return fFalse;
	AssertH(hsttb);
	psttb = *hsttb;
	cbWanted = psttb->bMax + cb;

	Assert(!vfInCommit);

	if (psttb->fExternal)
		{
		HQ hq = psttb->hqrgbst;
		if (!psttb->fNoShrink || cbWanted > CbOfHq(hq))
			{
			if (!FChngSizePhqLcb(&hq, (long)cbWanted))
				return fFalse;
			}
		(*hsttb)->hqrgbst = hq;
		return fTrue;
		}
	else
		{
		return FChngSizeHCw(hsttb, CwFromCch(cbWanted)+cwSTTBBase, fFalse);
		}
}


/* F  E N S U R E  S T T B  C B */
/* %%Function:FEnsureSttbCb %%Owner:davidlu */
FEnsureSttbCb(hsttb, cb)
struct STTB **hsttb;
uns cb;
{
	if ((*hsttb)->bMax >= cb)
		return (fTrue);
	return (FStretchSttbCb(hsttb, cb - (*hsttb)->bMax));
}


/* %%Function:IbstAddStToSttb %%Owner:davidlu */
EXPORT IbstAddStToSttb(hsttb, pst)
struct STTB **hsttb;
char   *pst;
{
	int ibst;

	AssertH(hsttb);
	if (FInsStInSttb(hsttb, ibst = (**hsttb).ibstMac, pst))
		return (ibst);
	else
		return (ibstNil);
}


/* %%Function:FInsStInSttb %%Owner:davidlu */
FInsStInSttb(hsttb, ibstNew, pst)
struct STTB **hsttb;
int    ibstNew;
char   *pst;
{
	return (FInsStInSttb1(hsttb, ibstNew, pst, 0));
}


/* %%Function:FInsStInSttb1 %%Owner:davidlu */
int FInsStInSttb1(hsttb, ibstNew, pst, pchExtra)
struct STTB **hsttb;
int    ibstNew;
char   *pst;
char   *pchExtra;
{
	uns     cbStNew;
	uns     cbNew;
	uns     cbNewBst;
	uns     cbTotal;
	int     ibstMac;
	uns     cbExtra;
	int dibst;
	uns cbBuffer = 0;
	struct STTB    *psttb;
	int     ibst;
	char HUGE *hpchMaxMove;
	char HUGE *hpchExtra;
	int HUGE  *hpbstNew;
	int HUGE *hpbst;
	uns     cbBlock;
	char HUGE *hpstNew;
	int HUGE *hpbstFirst;

	AssertH(hsttb);
	psttb = *hsttb;
	cbExtra = psttb->cbExtra;
/* a string length of 255 indicates a nil st. */
	if ((cbStNew = *pst) == 255 && psttb->fStyleRules)
		cbStNew = 0;
	cbStNew += 1;

/* reserve extra space to be maintained immediately after each st */
	cbTotal = cbStNew + cbExtra;

/* when Mac < Max, space is already reserved for new bst entry */
	dibst = ((ibstMac = psttb->ibstMac) < psttb->ibstMax) ? 0 : 1;
	cbNewBst = dibst * sizeof(int);

	/* can insert in middle or at the end, but not beyond the end! */
	Assert(ibstNew <= ibstMac);

/* expand STTB to accomodate new entry */
	if (!psttb->fExternal)
		{
		if (vfInCommit)
			Assert(CbOfH(hsttb) >= (**hsttb).bMax + 
					cbNewBst + cbTotal + cbSTTBBase);
		else if (!FChngSizeHCw(hsttb, CwFromCch((**hsttb).bMax + 
				cbNewBst + cbTotal) + cwSTTBBase + cbBuffer, fFalse))
			return (fFalse);
		psttb = *hsttb;
		}
	else
		{
		HQ hq = psttb->hqrgbst;     /* HM */
		if (vfInCommit)
			Assert(CbOfHq(hq) >= psttb->bMax + cbNewBst + cbTotal);
		else
			{
			long lcbChng = (long)(psttb->bMax + cbNewBst + cbTotal);
			if (!psttb->fNoShrink || lcbChng > CbOfHq(hq))
				{
				if (!FChngSizePhqLcb(&hq, lcbChng + cbBuffer))
					return (fFalse);
				}
			}
		psttb = *hsttb;
		psttb->hqrgbst = hq;
		}

	hpbstFirst = HpbstFirstSttb(hsttb);
	if (dibst)
		{
/* if we needed to allocate more space in rgbst, we will have to shift
	rgbst AND the strings that follow. */
		cbNewBst = dibst * sizeof(int);
#ifdef MAC
		for (ibst = 0, hpbst = hpbstFirst; ibst < ibstMac; ibst++, hpbst++)
			*hpbst += cbNewBst ;
#else /* WIN */
		AddDcbToLprgbst(LpOfHp(hpbstFirst), ibstMac, cbNewBst, 0);
#endif /* WIN */
		hpchMaxMove = ((char HUGE *)hpbstFirst) + psttb->bMax;
		hpbstNew = (int HUGE *)&hpbstFirst[psttb->ibstMax];
		cbBlock = hpchMaxMove - (char HUGE *)hpbstNew;

		if (cbBlock)
			bltbh(hpbstNew, hpbstNew + dibst, cbBlock);
		psttb->bMax += cbNewBst;
		psttb->ibstMax += dibst;
		}

/* point to location where new bst will be placed */
	hpbstNew = (int HUGE *)&hpbstFirst[ibstNew];
	hpchMaxMove = (char HUGE *)&hpbstFirst[ibstMac];

/* calc the cb of the data that must be shifted up to make room
	for the new bst. */
	cbBlock = hpchMaxMove - (char HUGE *)hpbstNew;

	if (cbBlock)
		bltbh(hpbstNew, hpbstNew + 1, cbBlock);

/* record offset where new string will be stored */
	*hpbstNew = psttb->bMax;

/* and calculate address of new string position */
	hpstNew = ((char HUGE *)hpbstFirst) + psttb->bMax;

/* copy new string */
	bltbh((char HUGE *)pst, hpstNew, cbStNew);
	hpchExtra = hpstNew + cbStNew;

/* initialize the extra words after the st */
	if (cbExtra)
		{
		if (pchExtra)
			bltbh((char HUGE *)pchExtra, hpchExtra, cbExtra);
		else
			bltbcx(0, LpFromHp(hpchExtra), cbExtra);
		}
/* extend bMax by length of new string plus extra words if any*/
	psttb->bMax += cbTotal;

	psttb->ibstMac++;

	return fTrue;
}


/* %%Function:FExpandSttbRgbst %%Owner:davidlu */
FExpandSttbRgbst(hsttb, ibstMaxNew)
struct STTB **hsttb;
int ibstMaxNew;
{
	int ibst;
	struct STTB *psttb = *hsttb;
	int ibstMac = psttb->ibstMac;
	int fNoShrinkSave;
	char HUGE *hpchMaxMove;
	int HUGE  *hpbstNew;
	int HUGE *hpbst;
	uns     cbNewBst, cbBlock;
	char HUGE *hpstNew;
	int HUGE *hpbstFirst;
	int dibst = ibstMaxNew - psttb->ibstMax;

	if (dibst <= 0)
		return (fTrue);
	fNoShrinkSave = psttb->fNoShrink;
	psttb->fNoShrink = fTrue;

	if (!FStretchSttbCb(hsttb, cbNewBst = dibst * sizeof(int)))
		{
		(*hsttb)->fNoShrink = fNoShrinkSave;
		return (fFalse);
		}
	psttb = *hsttb;
	psttb->fNoShrink = fNoShrinkSave;

	hpbstFirst = HpbstFirstSttb(hsttb);
/* we need to allocate more space in rgbst and we will have to shift
	rgbst AND the strings that follow. */
#ifdef MAC
	for (ibst = 0, hpbst = hpbstFirst; ibst < ibstMac; ibst++, hpbst++)
		*hpbst += cbNewBst ;
#else /* WIN */
	AddDcbToLprgbst(LpOfHp(hpbstFirst), ibstMac, cbNewBst, 0);
#endif /* WIN */
	hpchMaxMove = ((char HUGE *)hpbstFirst) + psttb->bMax;
	hpbstNew = (int HUGE *)&hpbstFirst[psttb->ibstMax];
	cbBlock = hpchMaxMove - (char HUGE *)hpbstNew;

	if (cbBlock)
		bltbh(hpbstNew, hpbstNew + dibst, cbBlock);
	psttb->bMax += cbNewBst;
	psttb->ibstMax += dibst;
	return fTrue;
}


/* %%Function:DeleteIFromSttb %%Owner:davidlu */
DeleteIFromSttb(hsttb, i)
struct STTB    **hsttb;
int     i;
{
	Assert(i < (*hsttb)->ibstMac);
	/* shrinking, won't fail */
	AssertDo(FChangeStInSttb(hsttb, i, (char *) 0));
}



/* %%Function:FChangeStInSttb %%Owner:davidlu */
EXPORT FChangeStInSttb(hsttb, ibst, pst)
struct STTB **hsttb;
int    ibst;
char   *pst;
{
	struct STTB  *psttb;
	uns     bstOld;
	char HUGE *hpstOld;
	uns     cbOld;
	uns     cbStNew;
	uns     cbNew;
	int     dcb;
	uns     bMaxNew;
	uns     bstNext;
	uns     cbBlock;
	int     ibstMac;
	int HUGE *hpbstFirst;
	int HUGE *hpbst;
	int HUGE *hpbstNext;
	char HUGE *hpstNext;

	psttb = *hsttb;

	Assert(ibst <= psttb->ibstMac);

	if (ibst == psttb->ibstMac)
		{
		Assert(pst != NULL);
		return (FInsStInSttb(hsttb, ibst, pst));
		}

	hpbstFirst = HpbstFirstSttb(hsttb);

	bstOld = hpbstFirst[ibst];
	/* get offset of the old string which occupies the ibst position. */

	if ((cbOld = *(((char HUGE *)hpbstFirst) + bstOld)) == 255
			&& psttb->fStyleRules)
		cbOld = 0;
	cbOld += (1 + psttb->cbExtra);
	bstNext = bstOld + cbOld;

	if (pst != 0)
		{
		if ((cbStNew = *pst) == 255 && psttb->fStyleRules)
			cbStNew = 0;
		cbStNew += 1;
		cbNew = cbStNew + psttb->cbExtra;
		}
	else
		cbNew = 0;
	dcb = cbNew - cbOld;
	bMaxNew = psttb->bMax + dcb;
	if (dcb > 0)
		{
		if (!psttb->fExternal)
			{
			if (vfInCommit)
				Assert(CbOfH(hsttb) >= bMaxNew+cbSTTBBase);
			else
				{
				if (!FChngSizeHCw(hsttb, cwSTTBBase+
						CwFromCch(bMaxNew), fFalse))
					return fFalse;
				}
			}
		else
			{
			HQ hq = psttb->hqrgbst;/* HM */
			if (vfInCommit)
				Assert(CbOfHq(hq) >= bMaxNew);
			else
				{
				if (!psttb->fNoShrink || bMaxNew > CbOfHq(hq))
					{
					if (!FChngSizePhqLcb(&hq, (long)bMaxNew))
						return fFalse;
					}
				}
			(*hsttb)->hqrgbst = hq;
			}
		hpbstFirst = HpbstFirstSttb(hsttb);
		psttb = *hsttb;
		}
	/* if there are any strings beyond the ibst string,
		shift them up(or down). */
	if (dcb != 0 && (cbBlock = psttb->bMax - bstNext))
		{
		hpstNext = ((char HUGE *)hpbstFirst) + bstNext;
		bltbh(hpstNext, hpstNext + dcb, cbBlock);
		}

	/* move in new string */
	if (pst)
		bltbh((char HUGE *)pst,
				((char HUGE *)hpbstFirst + bstOld), cbStNew);

	if (dcb < 0)
		{
		/* STTB must shrink */
		if (pst == 0)
			{
			hpbstNext = (int HUGE *)&hpbstFirst[ibst+1];
			cbBlock = (char HUGE *)&hpbstFirst[psttb->ibstMac] - (char HUGE *)hpbstNext;
			if (cbBlock > 0L)
				bltbh(hpbstNext, hpbstNext - 1, cbBlock);
			psttb->ibstMac--;
			}
		if (!psttb->fNoShrink)
			{
			if (!psttb->fExternal)
				{	  /* REVIEW cslbug(pj): opus native bug */
				if (vfInCommit)
					Assert(CbOfH(hsttb) >= cbSTTBBase+bMaxNew);
				else if (!FChngSizeHCw(hsttb, cwSTTBBase+CwFromCch(bMaxNew),
						fTrue))
					return fFalse;
				}
			else
				{
				HQ hq = psttb->hqrgbst;  /* HM */
				if (vfInCommit)
					Assert(CbOfHq(hq) >= bMaxNew);
				else if (!FChngSizePhqLcb(&hq, (long)bMaxNew))
					return fFalse;
				(*hsttb)->hqrgbst = hq;
				}
			}
		hpbstFirst = HpbstFirstSttb(hsttb);
		psttb = *hsttb;
		}
	if (dcb != 0)
		{
#ifdef MAC
		for (ibst = 0, ibstMac = psttb->ibstMac, hpbst = hpbstFirst;
				ibst < ibstMac; ibst++, hpbst++)
			{
			if (*hpbst >= bstNext)
				*hpbst += dcb;
			}
#else /* WIN */
		AddDcbToLprgbst(LpOfHp(hpbstFirst), psttb->ibstMac,
				dcb, bstNext);
#endif /* WIN */
		}
	psttb->bMax = bMaxNew;

	return fTrue;
}


#ifndef WIN	/* moved to resident */
/* P S T  F R O M  S T T B
*  returns a pointer to the i'th st in the sttb specified by hsttb */
NATIVE char *PstFromSttb(hsttb, i) /* WINIGNORE - !WIN */
struct STTB **hsttb;
int i;
{
	struct STTB *psttb = *hsttb;
	char *pst;
	Assert(!psttb->fExternal);
	pst = psttb->rgbst[i] + (char *)(psttb->rgbst);
	return (pst);
}


#endif

/* %%Function:HpchExtraFromSttb %%Owner:davidlu */
CHAR HUGE *HpchExtraFromSttb(hsttb, i)
struct STTB **hsttb;
int i;
{
	CHAR HUGE *hpst = HpstFromSttb(hsttb, i);
	int cbSt = *hpst;

	if (cbSt == 255 && (*hsttb)->fStyleRules)
		cbSt = 0;
	cbSt++;
	return hpst + cbSt;
}



#ifdef MAC /* in resident.asm if WIN */
/* I B S T  F I N D  S T */
/* %%Function:IbstFindSt %%Owner:davidlu */
EXPORT IbstFindSt(hsttb, st)
struct STTB **hsttb;
char *st;
{
	struct STTB *psttb = *hsttb;
	int ibst;
	int cch = *st;
	CHAR HUGE *hpst2;

	for (ibst = 0; ibst < psttb->ibstMac; ibst++)
		{
		hpst2 = HpstFromSttb(hsttb, ibst);
		if (*hpst2 == cch && !FNeHprgch((CHAR HUGE *)st+1, hpst2+1,
				cch))
			return(ibst);
		}
	return(-1);
}


#endif /* MAC */


/* F  S E A R C H  S T T B 
*  Binary-search the sttb returning true if the string is found. 
*  In any case return the index of where the string would be in rgbst at *pibst.
*/
/* %%Function:FSearchSttb %%Owner:davidlu */
FSearchSttb(hsttb, st, pibst, pfnWCompSt)
struct STTB **hsttb;
char *st;
int *pibst;
PFN pfnWCompSt;
{
	int ibstMin = 0;
	int ibstLim = (*hsttb)->ibstMac;
	int ibst = ibstMin;
	int wCompGuess;
	CHAR stT[cchMaxSt];

	if (hsttb == 0)
		return *pibst = iNil, fFalse; /* no hsttb to search */

	while (ibstMin < ibstLim)
		{
		ibst = (ibstMin + ibstLim) >> 1;
		GetStFromSttb(hsttb, ibst, stT);
		if ((wCompGuess = (*pfnWCompSt)(st, stT)) == 0)
			return *pibst = ibst, fTrue; /* found: return index */
		else  if (wCompGuess < 0)
			ibstLim = ibst;
		else
			ibstMin = ++ibst;
		}
	return *pibst = ibst, fFalse; /* not found: return insert point */
}


#ifdef MAC  /* Not used by WIN */
/* FGtUns(pw1, pw2)
uns *pw1, *pw2;
{
	return (*pw1 > *pw2);
}
*/
#endif /* MAC */

#ifdef MAC  /* Not used by WIN */
/* %%Function:WCompHpst %%Owner:NOTUSED */
NATIVE int WCompHpst(hpst1, hpst2) /* WINIGNORE - MAC only */
char HUGE *hpst1, HUGE *hpst2;
{
	int cchSt, cchSt1, cchSt2;
	int dcch = (cchSt1 = *hpst1++) - (cchSt2 = *hpst2++);
	for (cchSt = min(cchSt1, cchSt2); cchSt-- != 0; ++hpst1, ++hpst2)
		{
		int dch = ChLower(*hpst1) - ChLower(*hpst2);
		if (dch != 0) return dch;
		}
	return dcch;
}


#endif /* MAC */

#ifdef MAC  /* Not used by WIN */
/* %%Function:FGtBst %%Owner:NOTUSED */
FGtBst(hpbst1, hpbst2)
uns HUGE *hpbst1, HUGE *hpbst2;
{
	char HUGE *hpbase = vhpsttbSort;
	return (WCompHpst(hpbase + *hpbst1, hpbase + *hpbst2) > 0);
}


#endif /* MAC */



#ifdef MAC  /* Not used by WIN */
/* S O R T  S T T B  */
/* sorts strings in sttb by alphabetic order */
/* %%Function:SortSttb %%Owner:NOTUSED */
SortSttb(hsttb)
struct STTB **hsttb;
{
	struct STTB *psttb = *hsttb;
	int HUGE *hpbst;
	extern struct STTB HUGE *vhpsttbSort;

	if (psttb->fExternal)
		{
		LockHq(psttb->hqrgbst);
		vhpsttbSort = *psttb->hqrgbst;
		hpbst = (int HUGE *) vhpsttbSort;
		}
	else
		{
		vhpsttbSort = (struct STTB HUGE *)psttb;
		hpbst = (int HUGE *)psttb->rgbst;
		}
	SortRg(hpbst, psttb->ibstMac, sizeof(int), FGtBst);
	if (psttb->fExternal)
		UnlockHq(psttb->hqrgbst);
}


#endif /* MAC */


#ifdef MAC  /* Not used by WIN */
/* used in SortRg and SortSiftUpRg */
#define Hpfoo(i)		((char HUGE *)rgfoo + (cbFoo * (i)))
#endif /* MAC */

#ifdef MAC  /* Not used by WIN */
/* S O R T  R G */
/* sorts the array rgfoo with respect to pfnFGt */
/* pfnFGt is called thus: FGt(pfoo1, pfoo2) - ie. passed pointers to foo's */
/* %%Function:SortRg %%Owner:NOTUSED */
SortRg(rgfoo, ifooMac, cbFoo, pfnFGt)
long HUGE *rgfoo; /* array of foo to sort */
int ifooMac; /* number of foo's */
int cbFoo;   /* size of each foo */
PFN pfnFGt;  /* pointer to FGt function FGt(pfoo1, pfoo2) */
{
	long lTemp;
	int ifoo, ifoo1;
	int fLong = cbFoo == sizeof(long);
	char HUGE *hpfoo0, HUGE *hpfoo1;
	char rgchBuffer[64]; /* 64 bytes is largest foo */

	Assert(cbFoo <= 64);
	if (ifooMac < 2) return;
	for (ifoo = ifooMac>>1; ifoo >= 2; --ifoo)
		SortSiftUpRg(rgfoo, ifoo, ifooMac, pfnFGt, cbFoo, rgchBuffer);
	for (ifoo = ifooMac; ifoo >= 2; --ifoo)
		{
		SortSiftUpRg(rgfoo, 1, ifoo, pfnFGt, cbFoo, rgchBuffer);

		/* special case long size for speed */
		if (fLong)
			{
			lTemp = rgfoo[0];
			rgfoo[0] = rgfoo[ifoo1 = ifoo - 1];
			rgfoo[ifoo1] = lTemp;
			}
		else
			{
			/* swap rgfoo[0] and rgfoo[ifoo - 1] */
			bltbh(hpfoo0 = Hpfoo(0), (char HUGE *)rgchBuffer, cbFoo);
			bltbh(hpfoo1 = Hpfoo(ifoo - 1), hpfoo0, cbFoo);
			bltbh((char HUGE *)rgchBuffer, hpfoo1, cbFoo);
			}
		}
}


#endif /* MAC */

#ifdef MAC  /* Not used by WIN */
/* S O R T  S I F T  U P  R G */
/* see Floyd, Robert W. Algorithm 245 TREESORT 3 [M1] CACM 7, December 1964. */
/* %%Function:SortSiftUpRg %%Owner:NOTUSED */
SortSiftUpRg(rgfoo, ifooI, ifooN, pfnFGt, cbFoo, rgchBuffer)
long HUGE *rgfoo;
int ifooI, ifooN;
PFN pfnFGt;
int cbFoo;
char *rgchBuffer;
{
	int ifooJ;
	int fLong = cbFoo == sizeof(long);
	long lCopy;

	/* special case long for speed */
	if (fLong)
		lCopy = rgfoo[ifooI - 1];
	else
		bltbh(Hpfoo(ifooI - 1), (char HUGE *)rgchBuffer, cbFoo);
Loop:
	ifooJ = 2 * ifooI;
	if (ifooJ <= ifooN)
		{
		if (ifooJ < ifooN)
			{
			if ((*pfnFGt)(Hpfoo(ifooJ), Hpfoo(ifooJ - 1)))
				ifooJ++;
			}
		if ((*pfnFGt)(Hpfoo(ifooJ - 1), fLong? &lCopy : (char HUGE *)rgchBuffer))
			{
			/* special case long for speed */
			if (fLong)
				rgfoo[ifooI - 1] = rgfoo[ifooJ - 1];
			else
				bltbh(Hpfoo(ifooJ - 1), Hpfoo(ifooI - 1), cbFoo);
			ifooI = ifooJ;
			goto Loop;
			}
		}
	if (fLong)
		rgfoo[ifooI - 1] = lCopy;
	else
		bltbh((char HUGE *)rgchBuffer, Hpfoo(ifooI - 1), cbFoo);
}


#endif /* MAC */


#ifdef MAC  /* Not used by WIN */
/* %%Function:EmptySttb %%Owner:NOTUSED */
EmptySttb(hsttb)
struct STTB **hsttb;
{
	struct STTB *psttb = *hsttb;
	int far *lprgbst;

	psttb->ibstMac = 0;
/* clear sttb without shrinking heap */
	if (psttb->fExternal)
		{
		lprgbst = (int far *)LpFromHq(psttb->hqrgbst);
		psttb->bMax = (char far *)&lprgbst[psttb->ibstMax] - 
				(char far *)lprgbst;
		}
	else
		psttb->bMax = (char *)&psttb->rgbst[psttb->ibstMax] - 
				(char *)(psttb->rgbst);
}


#endif /* MAC */

#ifdef MAC  /* Not used by WIN */
/* %%Function:StToSzInPlace %%Owner:NOTUSED */
StToSzInPlace( pch )
CHAR *pch;
{
	int cch = *pch;

	bltbyte( pch+1, pch, cch );
	pch [cch] = '\0';
}


#endif /* MAC */


#ifdef WIN
/*  %%Function:HsttbAssocEnsure %%Owner:peterj  */
/* H S T T B  A S S O C  E N S U R E */
/*  Returns the existing hsttbAssoc for doc, if any.  If none, creates
	one, placing the handle in doc's DOD and returning the handle.
*/

HsttbAssocEnsure (doc)
int doc;

{
	struct STTB **hsttbAssoc;
	int ibst;

	if ((hsttbAssoc = PdodDoc (doc)->hsttbAssoc) != hNil)
		{
		Assert ((*hsttbAssoc)->ibstMac == ibstAssocMax);
		return hsttbAssoc;
		}

/* 3 * because an STTB entry needs at least 3 bytes */
	if ((hsttbAssoc = HsttbInit(0, fTrue/*fExt*/)) == hNil)
		return hNil;

/* create dummy entries in hsttbAssoc so as to use ibst constants */
	for ( ibst = 0 ; ibst < ibstAssocMax; ibst++ )
		{
		if (IbstAddStToSttb(hsttbAssoc, stEmpty) == ibstNil)
			{
			FreeHsttb(hsttbAssoc);
			return hNil;
			}
		}
	PdodDoc(doc)->hsttbAssoc = hsttbAssoc;

	return hsttbAssoc;
}
#endif /* WIN */



#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Sttb_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Sttb_Last() */
