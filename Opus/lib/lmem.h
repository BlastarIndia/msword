/*
	LMEM.H : Local Memory Manager Exports
*/

/*****************************************************************************/
/* Mac / PC API differences (redundant definition from CSTD.H) */

#ifndef __API_MACROS__
#ifdef	MAC
#define	STD_API(ret)	NATIVE ret
#else
#define STD_API(ret)	ret FAR PASCAL
#endif
#endif /*!__API_MACROS__*/

#ifndef CC
#ifndef PROCS
#define NOPROCS
#endif
#endif /*CC*/

/*****************************************************************************/

/* Generic near handle */
#ifndef CSTD_H
typedef	VOID NEAR *		PV;
#endif /*CSTD_H*/
typedef	VOID NEAR * NEAR *	PPV;

#define	fcmpNoCompact		0	/* don't compact */
#define	fcmpCompact		1	/* compact data */
#define	fcmpCompactHandles	2	/* compact handles */

/*	* Out Of Memory Errors	*/
#define	merrAllocMoveable	1	/* allocating a moveable block */
#define	merrAllocFixed		2	/* allocating a fixed block */
#define	merrReallocMoveable	3	/* growing a moveable block */
#define	merrAllocBlock		4	/* allocating block of handles */

/*	* Special Zeros		*/
/* NOTE: hard coded numbers to get around compiler limitations */
#define pvZero		((PV) 0x12)
				/* a pointer to 0 length block, 1st byte is 0 */
#define ppvZero		((PPV) 0x14)
				/* a pointer to pvZero */

/*	* Allocating a fixed block in a new heap always returns pvFixedMin */
/* NOTE: hard coded numbers to get around compiler limitations */
#ifdef	MAC
#define pvFixedMin	((PV)0x18)		/* old value */
#else	/* MAC */

#ifdef	DEBUG

#ifdef	ADJUST
#define pvFixedMin ((PV)0x36)
#else	/* ADJUST */
#define pvFixedMin ((PV)0x2c)
#endif	/* ADJUST */

#else	/* DEBUG */

#ifdef	ADJUST
#define pvFixedMin ((PV)0x2e)
#else	/* ADJUST */
#define pvFixedMin ((PV)0x26)
#endif	/* ADJUST */

#endif	/* DEBUG */
#endif	/* MAC */

/* global flag set after every compact */
extern BOOL PASCAL fCompactedHeap;


#ifdef DEBUG

/*	Debug global variables */
extern BOOL PASCAL fShakeHeap;
extern BOOL PASCAL fCheckHeap;
extern BOOL PASCAL fCheckHeapFree;	/* extra free checking -- slow */

/*	* function for introducing memory failures */
#ifndef C_PLUS_PLUS
#ifndef MKHDR
extern BOOL (FAR PASCAL *lpfnFailLmemFilter)(WORD, SB, WORD, WORD);	/*OPTIONAL*/
#endif
#endif


/*****************************************************************************/
/*	* Heap Info structure */

typedef struct _ckl
	{
	unsigned cblkFixed;
	unsigned cblkMoveable;
	unsigned chUsed;
	unsigned chFree;
	unsigned cbFixed;
	unsigned cbMoveable;
	} CKL;

#endif /* DEBUG */

/* Error trapping */
extern int pascal cfailLmemError;
#define	DisarmLmemError()	(cfailLmemError++)
#define	RearmLmemError()	(cfailLmemError--)

/* Fast way to find length of block (in current SB only) */
#define	CbSizePpvSbCur(ppv) 	(*((*(WORD **)(ppv))-1))

/*****************************************************************************/
/* Procedure Prototypes */
#ifndef NOPROCS
STD_API(VOID)		CreateHeap(SB);
STD_API(WORD)		CbCompactHeap(SB, WORD);
STD_API(WORD)		CbAvailHeap(SB);
STD_API(PPV)		PpvAllocCb(SB, WORD);
STD_API(BOOL)		FReallocPpv(SB, PPV, WORD);
STD_API(VOID)		FreePpv(SB, PPV);
STD_API(WORD)		CbSizePpv(SB, PPV);			/* CC ONLY */
STD_API(PV)		PvAllocFixedCb(SB, WORD);
STD_API(WORD)		CbSizeFixedPv(SB, PV);			/* CC ONLY */
STD_API(VOID)		LockHeap(SB);				/* OPTIONAL */
STD_API(VOID)		UnlockHeap(SB);				/* OPTIONAL */

STD_API(BOOL)		FCreateWindowsHeap(SB, WORD);

STD_API(PPV)		HFirstAllocBlock(SB, WORD);		/* OPTIONAL */
STD_API(VOID)		FreeHandleBlock(SB, PPV);		/* OPTIONAL */
STD_API(VOID)		FreeHandleBlockCh(SB, PPV, WORD);	/* OPTIONAL */
STD_API(VOID)		SwapHandles(SB, PPV, PPV);		/* OPTIONAL */

STD_API(BOOL)		FResizePpv(SB, PPV, WORD);		/* OPTIONAL */
STD_API(VOID)		FreeDataPpv(SB, PPV);			/* OPTIONAL */
STD_API(WORD)		CbCoalesceMove(SB);			/* OPTIONAL */

#ifdef ADJUST
STD_API(PPV)		PpvAllocAdjustCb(SB, WORD);		/* OPTIONAL */
STD_API(BOOL)		FResizeAdjustPpv(SB, PPV, WORD);	/* OPTIONAL */
STD_API(BOOL)		FReallocAdjustPpv(SB, PPV, WORD);	/* OPTIONAL */
STD_API(VOID)		FreeAdjustPpv(SB, PPV);			/* OPTIONAL */
STD_API(WORD)		CbCoalesceAdjust(SB);			/* OPTIONAL */
#endif

#ifdef DEBUG
STD_API(VOID)		CheckHeap(SB);				/* DEBUG */
STD_API(VOID)		ShakeHeapSb(SB);			/* DEBUG */
STD_API(VOID)		GetHeapInfo(SB, CKL FAR *);		/* DEBUG */
STD_API(BOOL)		FCheckHandle(SB, PPV);			/* DEBUG */
#endif /* DEBUG */

/* Pseudo debug functions */
STD_API(PPV)		PpvWalkHeap(SB, PPV);			/* DEBUG */
STD_API(PV)		PvWalkFixedHeap(SB, PV);		/* DEBUG */
#endif /*!NOPROCS*/

