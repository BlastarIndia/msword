/*
	sbmgr.h - Include file for sb manager
	Include after qwindows.h or cstd.h

	Define:

	CC 	- compilation with CMerge compiler
	EMM	- emm routines (not available for MAC)
	MAC	- mac version (cs compiler only)
	NOPROCS - no function prototypes (automatic for non-Mac Pcode)
*/

/*****************************************************************************/
/* Mac / PC API differences (redundant definition from CSTD.H) */

#ifndef __API_MACROS__
#ifdef	MAC
#define	STD_API(ret)	NATIVE ret
#else
#define STD_API(ret)	ret FAR PASCAL
#endif /*!MAC*/
#endif /*!__API_MACROS__*/

#ifndef CC
#ifndef PROCS
#define NOPROCS
#endif
#endif /*CC*/

/*****************************************************************************/
/* Basic Types */

typedef unsigned SB;
typedef unsigned IB;

#ifndef HUGE
#define HUGE huge
#endif

#ifdef SBMGR_CORRECT_HP		/* correct HP */
typedef void huge * HP;		/* generic huge pointer */
#else
/* for backward compatibility (until CS fixed) */
#ifndef CC
typedef int huge * HP;		/* generic huge pointer */
#else
typedef unsigned long HP;	/* generic huge pointer -- 32 bits long */
#endif
#endif /*!SBMGR_CORRECT_HP*/

#define HMEM_FIXED		0x0000
#define	HMEM_MOVEABLE		0x0002
#define	HMEM_NOCOMPACT		0x0010
#define	HMEM_ZEROINIT		0x0040
#define HMEM_DISCARDABLE	0x0f00

#define	HMEM_EMM		0x0004
#define	HMEM_EMM_SMALL		0x0001
#define	HMEM_EMM_LARGE		0x0000
#define HMEM_EMM_ONLY		0x0008

typedef struct
	{
	unsigned ib:14;
	unsigned f3:1;
	unsigned f2:1;
	unsigned sb:7;
	unsigned f1:1;
	} PH;		/* Packed Huge pointer */

typedef struct
	{
	HP hp;
	BOOL f1;
	BOOL f2;
	BOOL f3;
	} UH;		/* Unpacked Huge pointer */

#define sbNull	0		/* always 0 */
/* #define sbNil	-- application defined value */
#define sbDds	1		/* default data segment: aka. sbData */

#ifdef MAC
extern SB sbMax;
extern VOID far *mpsblpv[];
#else /* !MAC */
#define sbMax ((int)&_sbMax)
extern char PASCAL _sbMax;
extern WORD PASCAL mpsbps[];
extern WORD PASCAL pid16Emm;		/* EMM handle  */
#endif /* !MAC */
extern SB PASCAL sbMac;

#define LpOfHp(hp)	LpConvHp(hp)
#define HpOfSbIbConst(sb,ib) ((HP)((long)(((unsigned)ib) + (((unsigned long)((unsigned)sb)) << 16))))


/*****************************************************************************/
/* SB Manager Routines */

#ifndef NOPROCS

/* Init/Term */
STD_API(BOOL)	FInitSegTable(SB);

#ifdef EMM
STD_API(unsigned) CpageAvailStartEmm(void);
STD_API(unsigned) CbInitEmm(unsigned, BOOL, unsigned);
STD_API(VOID)	EndEmm(void);
STD_API(VOID)	CompactEmm(void);
STD_API(long)	CbFreeEmm(void);
#endif /* EMM */

/* SB Allotment */
STD_API(SB)		SbScanNext(BOOL);
STD_API(VOID)		BltSb(SB, SB, int);

/* Alloc/Realloc/Free */
STD_API(unsigned)	CbAllocSb(SB, unsigned, int);
STD_API(unsigned)	CbReallocSb(SB, unsigned, int);
STD_API(VOID)		FreeSb(SB);
STD_API(unsigned)	CbSizeSb(SB);

/* > 64K blocks (rare) */
STD_API(long)		LcbAllocSb(SB, long, int);		/* OPTIONAL */
STD_API(long)		LcbReallocSb(SB, long, int);		/* OPTIONAL */
STD_API(long)		LcbSizeSb(SB);				/* OPTIONAL */

/* Locking */
STD_API(VOID FAR *)	LpLockHp(HP);
STD_API(VOID)		UnlockHp(HP);
STD_API(VOID FAR *)	LpLockEs(VOID NEAR *);
STD_API(VOID)		UnlockEs(void);

/* Copy huge blocks */
STD_API(VOID)		_BLTBH(HP, HP, unsigned);

/* Packed Huge Pointers */
STD_API(VOID)		_PACKH(HP, UH NEAR *);
STD_API(VOID)		_UNPACKH(UH NEAR *, HP);

/* Misc CMerge Extensions */
#ifdef CC
STD_API(VOID)		SetEs(SB);				/* CC ONLY */
STD_API(HP)		LpReloadHp(HP);				/* CC ONLY */
STD_API(HP)		HpConvLp(HP);				/* CC ONLY */
STD_API(VOID)		BLTBH(HP, HP, unsigned);		/* CC ONLY */
STD_API(VOID)		PACKH(HP, UH NEAR *);			/* CC ONLY */
STD_API(VOID)		UNPACKH(UH NEAR *, HP);			/* CC ONLY */
#endif

#endif /*!NOPROCS*/

/*****************************************************************************/
/* Current SB */

#ifndef MAC
/* psCur -- PC current physical segment */
extern unsigned PASCAL psCur;		/* really extern segment psCur */
extern unsigned PASCAL psCur2;		/* really extern segment psCur */
extern SB PASCAL sbCur2;
#endif /*!MAC*/

/* SbCur -- a different way to get the current SB */
#define	SbCur()		sbCur
#define	SbCur2()	sbCur2

/* "sb-cur" aliases for "es" names */
#define SetSbCur(sb)	SetEs(sb)
#define ResetSbCur()	ResetEs()
#define PresetSbCur()	PresetEs()

#ifndef CC
extern SB _sbCur;
#define sbCur (__FNATIVE__ ? _sbCur : SBUOP_HUGE(2, 0xe6, 0x25))

#define SetEs(sb)	VSYS_HUGE(257, (SB) (sb))
#define ResetEs()	VSYS_HUGE(256)
#define PresetEs()	VSYS_HUGE(258)

#define LpConvEs(pv)	LPUOP_HUGE(1, 0xea, (VOID NEAR *)(pv))
#define HpConvEs(pv)	HpOfSbIb(sbCur, (IB)pv)

#else
/* CMerge SbCur Control */

extern SB PASCAL sbCur;

#define ResetEs()	SetEs(1)
#define	PresetEs()	/* makes no sense in CMerge */

#define LpConvEs(pv) ((DWORD)psCur<<16 | (WORD)(pv))
#define HpConvEs(pv) HpOfSbIb(sbCur, (IB)(pv))

#endif /*CC*/

#define SetSbCur2(sb)	SetEs2(sb)
#define ResetSbCur2()	SetEs2(sbDds)

/*****************************************************************************/
/*****************************************************************************/
/* CS HP Manipulation Macros */

#ifndef CC

/* UOPS for CS code */
#ifndef CS_PLUS_PLUS
uop HP HPUOP_HUGE();
uop SB SBUOP_HUGE();
uop IB IBUOP_HUGE();
uop void far *LPUOP_HUGE();
uop VOID VUOP_HUGE();
sys VOID VSYS_HUGE();
#endif

/* HP creation / decomposition */
#ifndef LOWORDX
/* define these if not including standard header */
#define LOWORDX(l)	 ((WORD)(l))
#define HIWORDX(l)	 ((WORD)(((DWORD)(l) >> 16) & 0xffff))
#endif

#define HpOfSbIb(sb,ib) HPUOP_HUGE(0, (SB) (sb), (IB) (ib))

#ifdef MAC
#define SbOfHp(hp) (__FNATIVE__?LOWORDX((HP)(hp)):SBUOP_HUGE(1,0xda,(HP)(hp)))
#define IbOfHp(hp) (__FNATIVE__?HIWORDX((long)(hp)):IBUOP_HUGE(2,0xe0,0xda,(HP)(hp)))
#else
#define SbOfHp(hp) (__FNATIVE__?HIWORDX((HP)(hp)):SBUOP_HUGE(1,0xda,(HP)(hp)))
#define IbOfHp(hp) (__FNATIVE__?LOWORDX((long)(hp)):IBUOP_HUGE(2,0xe0,0xda,(HP)(hp)))
#endif /* MAC */

/* Limited lifetime far pointer */
#define LpConvHp(hp) LPUOP_HUGE(2,0xe6,0x2b, (HP) (hp))

/* Interpreter entry points for huge pointer routines. */

/* BLT */
#define BLTBH(hp1,hp2,cb) (__FNATIVE__?_BLTBH((HP)(hp1),(HP)(hp2),(int)(cb)):VUOP_HUGE(2,0xe6,0x21, (HP) (hp1), (HP) (hp2), (unsigned) (cb)))

/* Pack / unpack / 3 byte pointers */
#define UNPACKH(pUh,hpPh) (__FNATIVE__?_UNPACKH((UH NEAR *)(pUh),(HP)(hpPh)):VUOP_HUGE(2,0xe6,0x28, (UH NEAR *) (pUh), (PH HUGE *) (hpPh)))
#define PACKH(hpPh,pUh) (__FNATIVE__?_PACKH((HP)(hpPh),(UH NEAR *)(pUh)):VUOP_HUGE(2,0xe6,0x29, (PH HUGE *) (hpPh), (UH NEAR *) (pUh)))

#define LDI3HS(hp,off) HPUOP_HUGE(2,0xf6,off, (HP) (hp))
#define STI3HS(hpDest,off,hp) VUOP_HUGE(3,0xe6,0x1a,off, (HP) (hp), (HP) (hpDest))

#endif /*!CC*/

/*****************************************************************************/
/* CC HP Manipulation */

#ifdef CC

#define HpOfSbIb(sb,ib) ((HP)((long)(((unsigned)ib) | ((unsigned long)((unsigned)sb)) << 16)))
#define SbOfHp(hp) ((WORD)(((DWORD)hp >> 16) & 0xffff))
#define IbOfHp(hp) ((WORD)hp)

#define LpConvSbIb(sb,ib) ((VOID FAR *)(((long)mpsbps[sb]<<16) | (ib)))
#define LpConvHp(hp) ((VOID FAR *) (((long)mpsbps[SbOfHp(hp)]<<16) | IbOfHp(hp)))
#define LpLoadHp(hp) ((mpsbps[SbOfHp(hp)]&1) ? LpConvHp(hp) : LpReloadHp(hp))
#define LpLoadSbIb(sb,ib) ((mpsbps[sb]&1) ? LpConvSbIb(sb, ib) : LpReloadHp(HpOfSbIb(sb, ib)))

#define LpLockSbIb(sb,ib) LpLockHp(HpOfSbIb(sb,ib))

#define UnlockSb(sb) UnlockHp((HP)(((unsigned long)((unsigned)sb)) << 16))

#define HpLoadHp(hp) ((LpLoadHp(hp)), (hp))
#define HpReloadHp(hp) ((LpReloadHp(hp)),(hp))

#endif /*CC*/

/*****************************************************************************/
/* Pcode Segment Aliasing (really part of Pcode Interpreter) */

#define snAliasMin ((int)&_snAliasMin)
extern char PASCAL _snAliasMin;
STD_API(VOID)	AliasSnSb(WORD, SB);				/* OPTIONAL */

/*****************************************************************************/
/* type of SB block */

STD_API(WORD)	FsbTypeSb(SB);					/* OPTIONAL */

#define	fsbNormal	0		/* block in normal memory */
#define	fsbEmm		1		/* block in EMM */
#define	fsbDiscarded	2		/* block is currently discarded */
#define	FInEmmSb(sb)		(FsbTypeSb(sb) == fsbEmm)

/*****************************************************************************/
/* Swapped SB Support */

extern SB	sbDiscardSpecial;				/* OPTIONAL */
STD_API(VOID)		DiscardSb(SB);				/* OPTIONAL */

/*****************************************************************************/
/*	* function for introducing memory failures */

/*	* Out Of Memory Errors	*/

/* same values as LMEM values with similar names */
#define	merrSbAllocMoveable	1	/* Windows memory is full */
#define	merrSbAllocFixed	2	/* Windows memory is full */
#define	merrSbReallocMoveable	3	/* Windows memory is full */
#define	merrSbReallocFixed	4	/* Windows memory is full */

/* SBMGR specific errors */
#define	merrSbAllocEmm		5	/* EMM is full (moveable only) */
#define	merrSbReallocEmm	6	/* EMM is full (moveable only) */

#ifndef C_PLUS_PLUS
#ifndef MKHDR
extern BOOL (FAR PASCAL *lpfnFailSbFilter)(WORD, SB, long, WORD);	/*OPTIONAL*/
#endif
#endif

/*****************************************************************************/
/* SBSAVE functionality */

STD_API(BOOL)	FInitSbSave(SB);				/* OPTIONAL */
/* -- call before first allocation, sb is special sbSave (say 2) */

STD_API(BOOL)	FSaveSbState(SB);				/* OPTIONAL */
/* -- save state (could fail if too large or non-EMM sbs */

STD_API(BOOL)	FRestoreSbState(void);				/* OPTIONAL */
/* -- try and restore (call before first allocation), will fail if the
*	SB state is corrupt or non-existant
*/

#define HemmGetSb() (pid16Emm)
/* -- get the value of the EMM handle currently allocated to the sbmgr. */

#define SetHemmSb(hemm) (pid16Emm = (hemm))
/* -- set the value of the EMM handle (do before CbInitEmm call). */

/*****************************************************************************/

