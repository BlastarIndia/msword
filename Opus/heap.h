/* H E A P . H */

#ifdef DEBUG
/* #define DLMEM */
#ifdef PCJ
/* #define DLMEM */
/* #define DLMEMFAIL */
/* #define SHOWEMM */
#define DCORELOAD
#endif
#endif

#ifndef fcmpCompact
#include "lmem.h"
#endif

#define LpFromHp(hp)	LpConvHp(hp)
#define FreeHp(hp)	FreeSb(SbOfHp(hp))
#define HpOfHq(hq)  	(HpOfSbIb(SbOfHp(hq),*(HQ)(hq)))
#define CbOfHq(hq)  	(*((uns HUGE *)HpOfHq(hq)-1))
#define LpFromHq(hq)  	LpOfHp(HpOfHq(hq))
#define LpLockHq(hq)    LpLockHp(HpOfHq(hq))
#define UnlockHq(hq)    UnlockHp(HpOfHq(hq))
#define HpFromPch(pch)	(HpOfSbIb(sbDds, pch))

HQ 	    	HqAllocLcb();


#ifdef NOTUSED   /* bz killed need for win heap  use fedt for mle 9/8/89 */
#define cbWindowsHeap	1568
#endif /* NOTUSED */

#define cbMaxSbDds  	65536 /* never bigger than this! */
#define cbExtraHeap     1024  /* normal amount of space in heap (>514) */
#define cbExtraDS   	0x1000 /* ask for more than required for our DS */

#define hmemLmemHeap	    HMEM_MOVEABLE | HMEM_EMM
#define hmemLmemHeapRealloc HMEM_MOVEABLE 

#define cbLmemHeap	    (cbMemChunk)
/* if an object is this big or bigger, it is given its own sb */
#define cbMinBig    	(cbLmemHeap - 200)
#define dcbBigSmall	    100 /* keep from bouncing small-big-small-big... */

/* must account for need to allocate handle (in 64 byte blocks) plus overhead
   of a single heap block (size byte, rounding) */
#define cbOverheadMax   (72)

/* most sb grabbed */
#define csbGrabMax          10
/* largest file cache */
#define csbCacheMax         57
/* largest number of sb's for other uses */
#define csbStructsMax       25
/* smallest amount of emm we will even look at */
#define csbEmmMin           4
/* amount of EMM reserved for other apps */
#define csbEmmReserve       16

/* non-emm cache size */
#define ibpMaxNoEmm         24
/* number of bp pages in an sb */
#define cbpChunkNoEmm	(24)	    /* size of entire cache */
#define cbpChunkWEmm	(31)

/* hash table size */
#define iibpHashMaxEmm      1024
#define iibpHashMaxNoEmm    16

#define lcbMaxFarObj	    (0x0000fff0L)

#define CbOfH(h)    	    	(CbSizePpvSbCur(h))
/* CwOfPfgr has +1 for Mac compatibility */
#define CwOfPfgr(h)	    	((CbOfH(h)+3)>>1)

#define FreeH(h)    	    	FreePpv(sbDds,(h))
#define HAllocateCw(cw)     	HAllocateCb((cw)<<1)
#define FChngSizeHCw(h,cw,f)  	FChngSizeHCb((h),(cw)<<1,(f))

#ifdef DEBUG
#define FreezeHp()          	(cHpFreeze++?0:LockHeap(sbDds))
#define MeltHp()            	(--cHpFreeze?0:UnlockHeap(sbDds))
#define MeltHpToZero()          (cHpFreeze?(UnlockHeap(sbDds),cHpFreeze=0):0)
#define CkHeap()    	    	CheckHeap(sbDds)
#define ShakeHeap()	    	ShakeHeapSb(sbDds)
#define HAllocateCb(cb)     	HAllocateCbProc(SzFrame(__FILE__),__LINE__,(cb))
#define FChngSizeHCb(h,cb,f) 	FChngSizeHCbProc(SzFrame(__FILE__),__LINE__,\
								(h),(cb),(f))
#else /* not DEBUG */
#define FreezeHp()
#define MeltHp()
#define MeltHpToZero()
#define CkHeap() 
#define ShakeHeap()
#define HAllocateCb(cb)	    	PpvAllocCb(sbDds,(cb))
#define FChngSizeHCb(h,cb,f)	FChngSizeHCbProc((h),(cb),(f))
#endif /* not DEBUG */


struct SBHI
	{	/* sb heap info */
	SB sb;
	uns fHasFixed : 1;
	uns fHasHeap : 1;
	uns ch : 12;
	uns : 2;
	};

#define cbSBHI (sizeof (struct SBHI))

struct PLSBHI
	{
	int isbhiMac;
	int isbhiMax;
	int cb;
	int brgsbhi;
	struct SBHI rgsbhi [1];
	};

/*  Heap handle compaction algorithms.  This allows a (local) heap handle
	to be stored in 12 or fewer bits (as an Hc).
*/
#define HcCompactH(h) (((uns)(h)-(uns)PpvMinSb(sbDds)) >>1)
#define HExpandHc(hc) ((uns)PpvMinSb(sbDds)+((uns)(hc) <<1))


struct PLF    /* PLex Far */
	{
	int isbMac;  /* same as Generic PLex */
	int isbMax;
	int cbSb;
	int brgsb;
	int fExternal;
	int iMac;       /* foo's in use */
	int iMax;       /* allocated space for foo's */
	int cbFoo;      /* size of a foo */
	int cFooChunk;  /* number of foo's per sb */

	SB  rgsb[];
	};

#define  cbPLFBase (offset(PLF, rgsb))

CHAR HUGE *HpInPlf();
struct PLF **HplfInit();
SB SbAllocEmmCb();

#define ReturnOnNoStack(cbNeeded, wRet, fReport)                \
	{                                                           \
	extern WORD pStackTop;                                      \
	int wDummy;                                                 \
	if ((WORD)&wDummy - pStackTop < cbNeeded + 1024)            \
		{                                                       \
		ReportSz("Not enough stack space, bagging out!");       \
		Debug(CommSzNum(SzShared("off by: "),                   \
			(cbNeeded + 1024)-((WORD)&wDummy - pStackTop)));    \
		if (fReport)                                            \
			ErrorEid(eidNoStack,"ReturnOnNoStack");             \
		return wRet;                                            \
		}                                                       \
	}


