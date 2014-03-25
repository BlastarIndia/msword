/* C L S P L C . C */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "file.h"
#include "disp.h"
#include "debug.h"
#include "error.h"
#include "ch.h"

#ifdef MAC
#define OSSTUFF
#include "toolbox.h"
#else
#include "dde.h"
#endif /* MAC */

int     docMac = 1;
struct **mpdochdod[docMax];
int     fnMac = 1;
struct **mpfnhfcb[fnMax];
int     wwMac = 1;
struct **mpwwhwwd[wwMax];
int     mwMac = 1;
struct **mpmwhmwd[mwMax];
#ifdef WIN
int	dclMac = 1;
struct	**mpdclhdcld[dclMax];
#endif
extern  struct MERR     vmerr;

struct CLSE dncls[clsMax] =
	{ 
	{ &mpdochdod, &docMac, docMax, cwDOD, offset(MERR, fDocFull) },
	
		{ &mpfnhfcb, &fnMac, fnMax, cwFCB, offset(MERR, fFnFull) },
	
		{ &mpwwhwwd, &wwMac, wwMax, cwWWD, offset(MERR, fWwFull) },
	
		{ &mpmwhmwd, &mwMac, mwMax, cwMWD, offset(MERR, fMwFull) }
#ifdef WIN
		,{ &mpdclhdcld, &dclMac, dclMax, cwDCLD, offset(MERR, fDclFull) }
#endif
	};



int vfUrgentAlloc = fTrue;

struct PL **HplInit();

#ifdef WIN
extern int vfInCommit;
#else
#define vfInCommit fFalse   /* Mac does not have this (yet?) */
#endif /* WIN */

#ifdef MAC
#ifdef DEBUG
extern HANDLE vhqEmerg;
extern int	vfAllocFail;
extern int	vFailEvery;
extern int	vFailAlloc;
extern int	vCurAlloc;
extern int	vHeap;
#endif
#endif /* MAC */

/* I  A L L O C  C L S */
/* %%Function:IAllocCls %%Owner:davidlu */
int IAllocCls(cls, cwFood)
int cls;
int cwFood;
{
	return IAllocClsH( cls, hNil, cwFood );
}


/* I  A L L O C  C L S */
/* allocate an instance of the requested cls (DOC, FCB, WWD) and return
	the instance's index number.
Returns 0 iff out of memory.
*/
/* %%Function:IAllocClsH %%Owner:davidlu */
int IAllocClsH(cls, h, cwFood)
int cls;
uns **h;
int cwFood;
{
	struct CLSE *pclse;
	int *mpfoohfood;
	int fooMac;
	int fExhaustFooMac;
	int foo;
	int fooLastFree;
	int cfooFree;

	Assert(!vfInCommit);

	Assert(cls < clsMax);
	pclse = &dncls[cls];
	mpfoohfood = pclse->mpfoohfood;
	fooMac = *(pclse->pfooMac);
	fExhaustFooMac = fFalse;


	/* find an unallocated foo in the slots in [0, fooMac). */
	cfooFree = CFreeCls(cls, &fooLastFree);

	if (cfooFree == 0)
		{
#ifdef MAC
		/* it is illegal to call this routine when all foos are
		allocated, so there should be room. */
		Assert(*(pclse->pfooMac) < pclse->fooMax);
#else  /* not illegal -- just act like OOM, but report the error manually */
		if (*(pclse->pfooMac) >= pclse->fooMax)
			{
			ErrorEid(eidComplexSession,"IAllocClsH'");
			SetErrorMat(matMem); /* lie - simplify code by simulating OOM */
			vmerr.fHadMemAlert = fTrue;
			vmerr.fErrorAlert = fTrue;
			return 0;
			}
#endif /* MAC */
		/* if no free slots were found within fooMac slots,
			take slot fooMac as a free slot and push fooMac. */
		fooLastFree = (*(pclse->pfooMac))++;
		cfooFree++;
		}

	/* allocate foo and clear it*/
	if (h == hNil)
		{
		h = HAllocateCw(cwFood);
		if (h == 0) return 0;
		SetWords(*h, 0, cwFood);
		}
	mpfoohfood[fooLastFree] = (int) h;

/* if we have used up all of the free slots in the mp table, set the full
	flag.  To simplify error recovery, an extra slot is reserved for one more
	chance. */

	if (cfooFree + pclse->fooMax - *(pclse->pfooMac) <= 3)/*reserve 2*/
		{
		/* Can't tell people to close a window when they are opening one. */
		if (!*((int *)((char *)&vmerr + pclse->bfFooFull)) && cls != clsMWD)
			ErrorEid(eidComplexSession,"IAllocClsH");
		*((int *)((char *)&vmerr + pclse->bfFooFull)) = fTrue;
		}

	return (fooLastFree);
}



/* C  F R E E  C L S  */
/* returns count of empty entries whose index is less than iMac. If count 
	is greater than 0, returns the index of the last free entry found*/
/* %%Function:CFreeCls %%Owner:davidlu */
int CFreeCls(cls, pcfooLastFree)
int cls;
int *pcfooLastFree;
{
	int *mpfoohfood;
	int foo, fooMac;
	int cfooFree, fooLastFree;
	struct CLSE *pclse;

	pclse = &dncls[cls];
	mpfoohfood = pclse->mpfoohfood;
	fooMac = *(pclse->pfooMac);
	/* find an unallocated foo. */
	for (foo = 1, cfooFree = 0, fooLastFree = 0; foo < fooMac; foo++)
		{
		if (mpfoohfood[foo] == 0)
			{
			/* if we find a free slot, remember the slot index,
				and add 1 to the count of free slots. */
			fooLastFree = foo;
			cfooFree++;
			}
		}
	*pcfooLastFree = fooLastFree;
	return cfooFree;
}


/* F R E E  C L S */
/* %%Function:FreeCls %%Owner:davidlu */
FreeCls(cls, foo)
int     cls;
int     foo;
{
	struct CLSE    *pclse;
	int     *mpfoohfood;
	int     fooT;

	if (foo == 0)
		return;

	Assert(cls < clsMax);
	pclse = &dncls[cls];

	mpfoohfood = pclse->mpfoohfood;
	FreePh(&mpfoohfood[foo]);

	/* if fooMac must change, find the new fooMac */
	if (foo == *(pclse->pfooMac) - 1)
		{
		for (fooT = --foo; fooT > 0; fooT--)
			{
			if (mpfoohfood[fooT] != 0)
				break;
			}
		*(pclse->pfooMac) = fooT + 1;
		}

	/* set FooFull flag in vmerr to false */
	*((int *)((char *)&vmerr + pclse->bfFooFull)) = fFalse;
}


/* H P L C  I N I T */
/* allocate a PLC on the heap whose entries are cbPlc long, with space
	allocted for ifooMaxInit+1 cp entries and ifooMaxInit foo entries.
	Set rgcp[0] of the PLC to cpLim. */

/* %%Function:HplcInit %%Owner:davidlu */
NATIVE struct PLC **HplcInit(cbPlc, ifooMaxInit, cpLim, fExtRgFoo)
int     cbPlc;
uns     ifooMaxInit;
CP      cpLim;
int	fExtRgFoo;
{
	struct PLC **hplc, *pplc;
	long    lcbPLCEntries;
	long    lcbExtra;
#ifdef MAC
	extern  void   BltbxProc();
#endif /* MAC */

	Assert(!vfInCommit);

	Assert(ifooMaxInit >= 0);
LAlloc:
	lcbPLCEntries = (long)ifooMaxInit * (sizeof(CP) + cbPlc) + sizeof(CP);
	lcbExtra = (!fExtRgFoo) ? lcbPLCEntries : (sizeof(HQ));
	if (lcbExtra+cbPLCBase >= 0x0000ffff)
		/* assure we don't roll over! (local case) */
		return hNil;
	if ((hplc = (struct PLC **)
			HAllocateCw(CwFromCch(cbPLCBase + (uns)lcbExtra))) == hNil)
		return(hNil);
	pplc = *hplc;
	SetBytes(&pplc->rgcp, 0, (uns)lcbExtra);
		
	pplc->fExternal = fExtRgFoo; /* two lines for NATIVE */
	if (pplc->fExternal) 
		{			    /* WARNING: heap may move */
		if (((*hplc)->hqplce = HqAllocLcb(lcbPLCEntries)) == 0L)
			{
			FreeH(hplc);
			return (hNil);
			}
		pplc = *hplc;
#ifdef MAC
		Debug(LoadSeg(BltbxProc));
#endif /* MAC */
/* initialize external PLC entries */
		bltbcx(0, LpFromHq(pplc->hqplce), (uns)lcbPLCEntries);
		}

	pplc->fExtBase = fFalse;
	pplc->fGrowGt64K = fFalse;
	Win(pplc->fMult = fFalse);
	pplc->iMac = 0;
	/* make sure that iMax is at least one greater than iMac, since we
		store 1 more cp than we have foos. */
	pplc->iMax = ifooMaxInit + 1;
	pplc->icpAdjust = pplc->iMac + 1;
	pplc->dcpAdjust = cp0;
	PutCpPlc(hplc, 0, cpLim);
	pplc->icpHint = 0;
	pplc->cb = cbPlc;
	return (hplc);
}


/* F   I N I T	 H P L C E D L */
/* allocate a PLC on the heap whose entries are cbPlc long, with space
	allocted for ifooMaxInit+1 cp entries and ifooMaxInit foo entries.
	Set rgcp[0] of the PLC to cpLim. */

/* %%Function:FInitHplcedl %%Owner:davidlu */
NATIVE BOOL FInitHplcedl(ifooMaxInit, cpLim, hpldr, idr)
int     ifooMaxInit;
CP      cpLim;
struct	PLDR **hpldr;
int	idr;
{
	struct PLC **hplcedl;
	struct DR  *pdr;
	struct DRF *pdrf;
	struct DRF drfFetch;

	if ((hplcedl = HplcInit(cchEDL, ifooMaxInit, cpLim, fTrue)) == hNil)
		return fFalse;
	pdr = PdrFetch(hpldr, idr, &drfFetch);
	Assert (pdr->hplcedl == hNil);
	pdr->hplcedl = hplcedl;
	/* Needed now for MacWord because table drs are far */
	if ((*hpldr)->fExternal)
		{
		(*hplcedl)->fExtBase = fTrue;
		bltb(*hplcedl, &pdr->plcedl, sizeof(struct PLC));
		FreeH(hplcedl);
		pdrf = (struct DRF *)(((char *)pdr) - offset(DRF, dr));
		Assert (pdrf->hpldr == hpldr);
		Assert (pdrf->idr == idr);
		Assert (pdrf->wLast == 0xABCD);
		pdrf->dr.hplcedl = &pdrf->pplcedl;
		pdrf->pplcedl = &pdrf->dr.plcedl;
		}
	FreePdrf(&drfFetch);
	return fTrue;
}


/* F R E E  H P L C  */
/* %%Function:FreeHplc %%Owner:davidlu */
FreeHplc(hplc)
struct PLC **hplc;
{
	if (hplc == hNil)
		return;
	if (((*hplc)->fExternal) && (*hplc)->hqplce != 0L)
		FreeHq((*hplc)->hqplce);
	if (!(*hplc)->fExtBase)
		FreeH(hplc);
}


/* F R E E  P H P L C  */
/* %%Function:FreePhplc %%Owner:davidlu */
FreePhplc(phplc)
struct PLC ***phplc;
{
	FreeHplc(*phplc);
	*phplc = hNil;
}


/* H P L  I N I T 2  */
/* %%Function:HplInit2 %%Owner:davidlu */
EXPORT struct PL **HplInit2(cbPl, cbHead, ifooMaxInit, fExternal)
int     cbPl;
int 	cbHead;  /* base size of PL.  (i.e. brgfoo) */
int     ifooMaxInit;
int	fExternal;
{
	struct PL **hpl, *ppl;
	long   lcbPLEntries, lcbExtra;
	char	*rgfoo;

	Assert(!vfInCommit);

	Assert(cbHead >= cbPLBase);
	lcbPLEntries = (long)ifooMaxInit * cbPl;
	lcbExtra = (!fExternal) ? lcbPLEntries : (sizeof(HQ));
	if (lcbExtra + cbHead >= 0x0000ffff)
		{
		/* assure we don't roll over! (local case) */
		Assert(fFalse);
		return hNil;
		}
	if ((hpl = (struct PL **)
			HAllocateCw(CwFromCch(cbHead + (uns)lcbExtra))) == hNil)
		return(hNil);
	ppl = *hpl;
	rgfoo = ((char *)ppl) + cbHead;
	SetBytes(&ppl->rgbHead, 0, (uns)lcbExtra + cbHead - cbPLBase);

	if (ppl->fExternal = fExternal)
		{			    /* WARNING: heap may move */
		HQ hqple;

		if ((hqple = HqAllocLcb(lcbPLEntries)) == 0L)
			{
			FreeH(hpl);
			return (hNil);
			}
		ppl = *hpl;
		rgfoo = ((char *)ppl) + cbHead;
		*((HQ *)rgfoo) = hqple;
#ifdef MAC
		Debug(LoadSeg(BltbxProc));
#endif /* MAC */
/* initialize external PL entries */
		bltbcx(0, LpFromHq(*((HQ *)rgfoo)), (uns)lcbPLEntries);
		}
	ppl->iMac = 0;
	ppl->iMax = ifooMaxInit;
	ppl->cb = cbPl;
	ppl->brgfoo = cbHead;
	return (hpl);
}


/* H P L  I N I T */
/* %%Function:HplInit %%Owner:davidlu */
EXPORT struct PL **HplInit(cbPl, ifooMaxInit)
int     cbPl;
int     ifooMaxInit;
{
	return HplInit2(cbPl, cbPLBase /* cbHead */, ifooMaxInit, fFalse);
}


/* F R E E  H P L  */
/* %%Function:FreeHpl %%Owner:davidlu */
EXPORT FreeHpl(hpl)
struct PL **hpl;
{
	struct PL *ppl;

	if (hpl == hNil)
		return;
	ppl = *hpl;
	if ((ppl)->fExternal)
		{
		HQ hqple;

		hqple = *((HQ *)(((char *)ppl) + ppl->brgfoo));
		if (hqple != 0L)
			FreeHq(hqple);
		}
	FreeH(hpl);
}


/* F R E E  P H P L  */
/* %%Function:FreePhpl %%Owner:davidlu */
FreePhpl(phpl)
struct PL ***phpl;
{
	FreeHpl(*phpl);
	*phpl = hNil;
}



#ifdef DEBUGORNOTWIN
/* F  I N S E R T  I N  P L C */
/* open space in PLC and copy passed cp to plc.rgcp[i], and move structure
	foo pointed to by pch to the ith entry in the range of foos. */
#ifdef MAC
/* %%Function:FInsertInPlc %%Owner:NOTUSED */
EXPORT FInsertInPlc(hplc, i, cp, pch) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_FInsertInPlc %%Owner:davidlu */
HANDNATIVE C_FInsertInPlc(hplc, i, cp, pch)
#endif /* !MAC */
struct PLC **hplc;
int i;
CP cp;
char *pch;
{
	Assert(hplc);

	if (!FOpenPlc(hplc, i, 1))
		return fFalse;
	Assert((*hplc)->cb == 0 || pch != NULL);
	if ((*hplc)->cb) PutPlc(hplc, i, pch);
	PutCpPlc(hplc, i, cp);
	return fTrue;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* F  O P E N  P L C */
/* when di > 0, FOpenPlc inserts di empty entries at position i in the PLC.
	when di < 0, FOpenPlc deletes the -di entries beginning at position i
	in the PLC. */
#ifdef MAC
/* %%Function:FOpenPlc %%Owner:NOTUSED */
NATIVE FOpenPlc(hplc, i, di) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_FOpenPlc %%Owner:davidlu */
HANDNATIVE C_FOpenPlc(hplc, i, di)
#endif /* !MAC */
struct PLC **hplc;
int i;
int di;
{
	struct PLC *pplc;
	int cpPlc;
	int iMacOld;
	int iMaxNew;
	int cbPlc;

/* if no change requested, there's nothing to do. return. */
	if (di == 0)
		return(fTrue);

/* must unwind the fence up to pplc->rgcp[i], because caller is signaling
	intention to alter plc beginning with ith entry (even when di == 0). */
	pplc = *hplc;
	if (pplc->icpAdjust < i)
		AdjustHplcCpsToLim(hplc, i);

	Assert(i <= pplc->iMac);

	cbPlc = pplc->cb;
	iMacOld = pplc->iMac;

	if (di > 0)
		{
		/* we are expanding the plc */
		if (!FStretchPlc(hplc, di))
			return fFalse;
		pplc = *hplc;
/* move rgcp entries */
		BltInPlc(bpmCp, hplc, i, di, 0, iMacOld - i + 1);
/* move rgfoo entries */
		BltInPlc(bpmFoo, hplc, i, 0, di, iMacOld - i);
		pplc->iMac = iMacOld + di;
		if (pplc->icpAdjust < i)
			pplc->icpAdjust = i;
		pplc->icpAdjust += di;
		}
	else  if (di < 0)
		{
		/* in this case we will be removing di entries */
		di = -di;
		Assert(i + di <= iMacOld);

		iMaxNew = pplc->iMax;

		/* if the Mac is less than half of the Max, we will later
			shift rgcp and foo entries in the PLC and reduce the
			size of the allocation */
		if ((iMacOld - di) * 2 <= iMaxNew)
			iMaxNew = iMacOld - di;
		if (pplc->icpAdjust > i + di)
			pplc->icpAdjust -= di;
		else  if (pplc->icpAdjust > i)
			pplc->icpAdjust = i;
		ShrinkPlc(hplc, iMaxNew, i, di);
/* blow the hint for the binary search. */
		pplc = *hplc;
		if (pplc->icpHint >= pplc->iMac)
			pplc->icpHint = 0;
		}
	return (fTrue);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* S H R I N K  P L C */
/* shrink size to iMaxNew while deleting di entries starting with i */
#ifdef MAC
/* %%Function:ShrinkPlc %%Owner:NOTUSED */
ShrinkPlc(hplc, iMaxNew, i, di)
#else /* !MAC */
/* %%Function:C_ShrinkPlc %%Owner:davidlu */
HANDNATIVE C_ShrinkPlc(hplc, iMaxNew, i, di)
#endif /* !MAC */
struct PLC **hplc; 
int iMaxNew, i, di;
{
	struct PLC *pplc = *hplc;
	int iMaxOld = pplc->iMax;
	int iMacOld = pplc->iMac;
	int iLim = i + di;
	int cbPlc = pplc->cb;
	int dicp;

	pplc->iMac = iMacOld - di;
	/* shift down rgcp[j] for j >= i + di */
	BltInPlc(bpmCp, hplc, iLim, -di, 0, (iMacOld + 1) - iLim);

	/* shift down rgfoo[j] for 0 <=j < i to cover any rgcp entries
		that must be reclaimed. */

	Assert(iMaxNew <= iMaxOld);
	iMaxNew = min(iMaxNew + 5, iMaxOld); /* leave some room */

	dicp = iMaxOld - iMaxNew;
	if (dicp && i > 0)
		BltInPlc(bpmFoo, hplc, 0, -dicp, 0, i);
	/* shift down rgfoo[i] for j >= i + di to cover space reclaimed
		from deleted rgfoos */

	BltInPlc(bpmFoo, hplc, iLim, -dicp, -di, iMacOld - iLim);

	pplc->iMax = iMaxNew;
	if (iMaxNew < iMaxOld && !vfInCommit)
		{
		if (!pplc->fExternal)
			FChngSizeHCw(hplc, CwFromCch(cbPLCBase + (iMaxNew-1)
					* (sizeof(CP)+cbPlc) + sizeof(CP)), fTrue);
		else
			{
			HQ hqplce = pplc->hqplce; /* WARNING: heap may move */
			FChngSizePhqLcb(&hqplce, (((long)iMaxNew - 1) *
					(sizeof(CP) + cbPlc) + sizeof(CP)));
			(*hplc)->hqplce = hqplce;
			}
		}
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* F  S T R E T C H  P L C */
/* Assure room in hplc for di more entries.  Changes iMax if necessary but does
not change any other data.  Returns fFalse iff no room.
*/
#ifdef MAC
/* %%Function:FStretchPlc %%Owner:NOTUSED */
EXPORT FStretchPlc(hplc, di) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_FStretchPlc %%Owner:davidlu */
HANDNATIVE C_FStretchPlc(hplc, di)
#endif /* !MAC */
struct PLC **hplc;
int di;
{
	struct PLC *pplc = *hplc;
	int diNeeded, diRequest;

	Assert(vfUrgentAlloc);
	AssertH(hplc);

	if ((diNeeded = di - (pplc->iMax - pplc->iMac - 1)) <= 0)
/* there is already sufficient space, do nothing */
		return fTrue;

	else
/* we need to expand beyond current max */
		{
		if (di == 1 && !vfInCommit && 
				(diRequest = pplc->iMax/4) > diNeeded)
/* if just growing by one, try to grow by a larger increment first */
			{
			BOOL f;
			int matSave;
/* we don't want to hand over swap space just to give a plc extra entries so 
	we declare that the first allocation we will try is non-urgent. */
			vfUrgentAlloc = fFalse;
			matSave = vmerr.mat;
			f = FStretchPlc2(hplc, diRequest);
			vfUrgentAlloc = fTrue;
			if (f)
				return fTrue;

			/* if 1st alloc failed, restore vmerr flag in case
			2nd try succeeds.
			*/
			vmerr.mat = matSave;
			}
		return FStretchPlc2(hplc, diNeeded);
		}
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* F  S T R E T C H  P L C  2 */
/* Grows iMax by exactly di.  Returns fFalse iff no room.
*/
/* %%Function:FStretchPlc2 %%Owner:davidlu */
FStretchPlc2(hplc, di)
struct PLC **hplc;
int di;
{
	struct PLC *pplc = *hplc;
	long lcb;
#ifdef MAC
	long iMaxNew;
#else /* WIN */
	int iMaxNew;
#endif /* WIN */

/* don't let plc overflow iMac */
#ifdef MAC
	if ((iMaxNew = (long)pplc->iMax + di) > 32767)
		{
		SetErrorMat(matMem);
		return fFalse;
		}
#else /* WIN */
	if ((iMaxNew = pplc->iMax + di) < 0)
		{
		SetErrorMat(matMem);
		return fFalse;
		}
#endif /* WIN */

	lcb = (long)(iMaxNew-1)*(pplc->cb + sizeof(CP)) + sizeof(CP);
#ifdef MAC 
	if (lcb > 65535L && !pplc->fGrowGt64K)
		{
		SetErrorMat(matMem);
		return fFalse;
		}
#endif /* MAC */

	if (pplc->fExternal)
		if (vfInCommit)
/* should already be big enough */
			Assert(CbOfHq(pplc->hqplce) >= lcb);
		else
			{
			HQ hq = pplc->hqplce;
			if (!FChngSizePhqLcb(&hq, lcb)) /* HM */
				return fFalse;
			(*hplc)->hqplce = hq;
			}
	else if (vfInCommit)
/* should already be big enough */
		Assert(CbOfH(hplc) >= lcb + cbPLCBase);
	else
		{
/* protect against cb overflow */
		if ((lcb += cbPLCBase) > 0x00007fff)
			{
			SetErrorMat(matMem);
			return fFalse;
			}
		if (!FChngSizeHCw(hplc, CwFromCch((uns)lcb), fFalse))
			return fFalse;
		}
	pplc = *hplc;
/* push rgfoo tables from old pos, up by di CP's */
	BltInPlc(bpmFoo, hplc, 0, di, 0, pplc->iMac);
	pplc->iMax += di;
	return fTrue;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN /* coded in-line in editn.asm if WIN */
/* C L O S E  U P  P L C */
/* Remove unneeded space from hplc.
*/
/* %%Function:CloseUpPlc %%Owner:davidlu */
HANDNATIVE CloseUpPlc(hplc)
struct PLC **hplc;
{
	struct PLC *pplc = *hplc;
	long lcb = (long)(pplc->iMax-1)*(pplc->cb+sizeof(CP)) + sizeof(CP);

	AssertH(hplc);
	Assert(!vfInCommit);

	if (pplc->fExternal)
		{
		HQ hq = pplc->hqplce;
		if (CbOfHq(hq) > lcb+20)
			/* don't bother if it is close */
			{
			AssertDo(FChngSizePhqLcb(&hq, lcb)); /* HM */
			(*hplc)->hqplce = hq;
			}
		}
	else
		AssertDo(FChngSizeHCw(hplc, CwFromCch((uns)lcb+cbPLCBase), 
				fTrue));
}


#endif /* DEBUGORNOTWIN */


/* F  I N S E R T  I N  P L */
/* inserts foo at pch in pl at index i. Rest is pushed up and storage is
allocated (small allocation - always succeeds.)
*/
/* %%Function:FInsertInPl %%Owner:davidlu */
EXPORT FInsertInPl(hpl, i, pch)
struct PL **hpl;
int i;
char *pch;
{
	struct PL *ppl;
	int brgfoo;
	int iMac;
	char HUGE *hpchFoo;
	long lcb, lcbPl;

	/*  PL's not currently supported in commit code */
	Assert(!vfInCommit);

	ppl = *hpl;
	lcb = ppl->cb;
	brgfoo = ppl->brgfoo;

	if ((iMac = ppl->iMac++) == ppl->iMax)
		{
		ppl->iMax++;
		lcbPl = lcb * ppl->iMax;
		if (!ppl->fExternal)
			{
			if (lcbPl + brgfoo >= 0x0000ffffL)
				/* don't roll over!! (local heap) */
				goto LAbort;
			if (!FChngSizeHCw(hpl,
					CwFromCch(brgfoo + (uns)lcbPl),
					fFalse))
				goto LAbort;
			}
		else
			{
			HQ hq = *((HQ *)(((char *)ppl) + ppl->brgfoo));
			/* FChngSizePhqLcb may cause heap movement! */
			if (!FChngSizePhqLcb(&hq, lcbPl))
				{
LAbort:
				(*hpl)->iMac--;
				(*hpl)->iMax--;
				return fFalse;
				}
			ppl = *hpl;
			*((HQ *)(((char *)ppl) + ppl->brgfoo)) = hq;
			}
		}
	hpchFoo = HpInPl(hpl, i);
	if (iMac > i)
		bltbh(hpchFoo, hpchFoo + lcb, (iMac - i) * lcb);
	bltbh(pch, hpchFoo, lcb);
	return fTrue;
}


/* D E L E T E  F R O M  P L */
/* %%Function:DeleteFromPl %%Owner:davidlu */
EXPORT DeleteFromPl(hpl, i)
struct PL **hpl;
int i;
{
	struct PL *ppl;
	int brgfoo;
	int iMac;
	char HUGE *hpchFoo;
	long lcb;

	ppl = *hpl;
	lcb = ppl->cb;
	brgfoo = ppl->brgfoo;
	iMac = ppl->iMac;
	hpchFoo = HpInPl(hpl, i);
	Assert(i < iMac);
	if (i + 1 < iMac)
		bltbh(hpchFoo + lcb, hpchFoo, (iMac - i - 1) * lcb);
	ppl->iMac--;
}


#ifdef MAC
/* %%Function:FChngSizePhqLcb %%Owner:NOTUSED */
FChngSizePhqLcb(phq, lcb)
HQ *phq;
long lcb;
{
	int ec;
	REGS regs;

	Assert(lcb >= 0);

	regs.a0 = *phq;
	regs.d0 = lcb;

#ifdef DEBUG
	if ((vfAllocFail) && ((vHeap == 1) || (vHeap == 2)) && (vCurAlloc-- == 0))
		{
		(vFailEvery) ? (vCurAlloc = vFailEvery - 1) : (vfAllocFail = fFalse);
		/* we want to guarantee an allocation failure 
			if we are expanding the block */
		if (lcb > LcbQq(*phq))
			regs.d0 = 1024L * 1024L;
		}
#endif

	if (ec = OsTrap(&regs, _SETHANDLESIZE))
		SetErrorMat(matMem);
	return(ec == 0);
}


#endif /* MAC */


/* H P L C  C R E A T E  E D C */
/* create an empty plc of type edc and store the hplc in the
appropriate word (addressed by edc itself) in the dod.

Format of the empty plc:
Assume E M 1 2  for CpMacDocEdit, CpMacDoc, CpMac1Doc and CpMac2Doc respectively,

	Initial                 With
				one footnote, ending at E
FND:    0, M=2                  0, E, M=2
				one section ending at M
SED:    0, 1, 2                 0, M, 1, 2
				one paragraph starting at A
PHE:    2                       A, 2
				second page starting at A
PGD:    0, 2                    0, A, 2
				one paragraph in outline mode
PAD:    0, 2                    0, M, 2
HDD:    same as FND

SEA:    0, 2                    ?

*/
/* %%Function:HplcCreateEdc %%Owner:davidlu */
struct PLC *HplcCreateEdc(hdod, edc)
struct DOD **hdod; 
int edc;
{
	struct PLC **hplc;
	int cb;
	CP cpMac2Doc;
	int rgw[5];

	/* CpMac2Doc(doc) */
	cpMac2Doc = (*hdod)->cpMac;

	switch (edc)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif
	case edcFnd:
		cb = cbFND;
		Win(Assert(cbFND == cbAND)); /* shared with annotation */
		break;
	case edcSed:
		((struct SED *)rgw)->fn = fnScratch;
		((struct SED *)rgw)->fcSepx = fcNil;
		cb = cbSED;
		break;
	case edcPgd:
		cb = cbPGD;
		goto LPgd;
	case edcPad:
		(*hdod)->fOutlineDirty = fTrue;
		cb = cbPAD;
		SetWords(rgw, 0, cbPAD / sizeof(int));  /* set = 0 */
		((struct PAD *)rgw)->fShow = fTrue;
LPgd:           
		((struct PGD *)rgw)->fUnk = 1;
		break;
	case edcHdd:
		cb = 0;
		edc = edcFnd;   /* same location as hplcfnd */
		break;
	case edcSea:
		cb = cbSEA;
		break;
#ifdef WIN
	case edcMcr:
		cb = cbMCR;
		edc = edcFnd;	/* same location as hplcfnd */
		break;
	case edcGlsy:
		cb = 0;
		edc = edcFnd;	/* same location as hplcfnd */
		break;
	case edcDdli:
		cb = cbDDLI;
		edc = edcFnd;	/* same location as hplcfnd */
		break;
#endif /* WIN */
		}

	/*  don't overwrite an existing plc! */
	Assert(*((int *)(*hdod) + edc) == hNil);

	if ((hplc = HplcInit(cb, 2, cpMac2Doc, fTrue /* ext rgFoo */)) == hNil)
		return hNil;

	*((int *)(*hdod) + edc) = hplc;
	/* guaranteed by the init call above */
	AssertDo(FInsertInPlc(hplc, 0, cp0, rgw));

	if (edc == edcSed)
		{
		struct SED sed;
		sed.fUnk = fFalse;
		sed.fn = fnScratch;
		sed.fcSepx = fcNil;
		/* guaranteed by the init call above */
		AssertDo (FInsertInPlc(hplc, 1, cpMac2Doc - ccpEop, &sed));
		}

	return hplc;
}


/* H  C O P Y  H E A P	B L O C K */
/* %%Function:HCopyHeapBlock %%Owner:davidlu */
uns **HCopyHeapBlock(hOld, strc)
char **hOld;
int strc;
{
	char **hNew;
	int cb;
	int fExternal;
	int bhq;

	if (hOld == hNil)
		return (hNil);

	Assert(!vfInCommit);

	/* first copy the base */
	hNew = HAllocateCb(cb = CbOfH(hOld));
	if (hNew)
		{
		bltb(*hOld, *hNew, cb);

/* if object has external component, copy it too */
		switch (strc)
			{
		case strcPL:
			bhq = ((struct PL *)(*hNew))->brgfoo;
			fExternal = ((struct PL *)(*hNew))->fExternal;
			break;
		case strcPLC:
			bhq = offset(PLC, hqplce);
			fExternal = ((struct PLC *)(*hNew))->fExternal;
			break;
		case strcSTTB:
			bhq = offset(STTB, hqrgbst);
			fExternal = ((struct STTB *)(*hNew))->fExternal;
			break;
		default:
			fExternal = fFalse;
			}
		if (fExternal)
			{
			HQ hqOld = *((HQ *)(*hNew+bhq));
			long lcb = CbOfHq(hqOld);
			HQ hqNew = HqAllocLcb(lcb); /* HM */
			if (hqNew == 0)
				{
				FreeH(hNew);
				return (hNil);
				}
			Assert(lcb < 0x00010000);
			bltbh(HpOfHq(hqOld), HpOfHq(hqNew), (uns)lcb);
			*((HQ *)(*hNew+bhq)) = hqNew;
			}
		}
	return(hNew);
}

#ifdef WIN
#include "util.c"
#endif /* WIN */

#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Clsplc_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Clsplc_Last() */
