/* I F D E F S
the following can be specified on the make statement to control compilation:
	DEBUG	  - enables debugging code
	DEMO	  - generates a demonstration version
	FOREIGN   - enables non-US code
	JR	  - generates MS Write version (note: Write has now been
			split off from Word; the #ifdefs remain as hints)
	NAMESTAMP - enables personalization code
*/

#ifdef WIN
#ifdef DEBUG
#define DEBUGASSERTSZ csconst CHAR szAssertFile[] = __FILE__;
#endif /* DEBUG */
#endif /* WIN */
#ifndef DEBUGASSERTSZ
#define DEBUGASSERTSZ
#endif

#ifndef WIN
#define DEBUGORNOTWIN
#endif
#ifdef DEBUG
#define DEBUGORNOTWIN
#endif

#ifdef	WIN
#define HANDNATIVE EXPORT
#else  /* MAC */
#define HANDNATIVE NATIVE
#endif /* MAC */

#ifdef WIN
#define NOMINMAX
#ifdef NONATIVE /* temporary--disable cs native until we are more stable */
#define NATIVE	EXPORT
#endif
#include "qwindows.h"	/* The standard p-code windows.h */
#include "sbmgr.h"
#endif

#ifndef NATIVE
#define NATIVE	native
#endif

#ifdef	WIN
#	define	Win(foo)	foo
#	define	Mac(foo)
#else
#	define	Win(foo)
#	define	Mac(foo)	foo
#endif

/* T Y P E S  */

typedef long CP;
#define cwCP (sizeof(CP) / sizeof(int))

typedef long FC;

typedef unsigned PN;  /* 512 byte page */
typedef unsigned PO; /* 128 byte page (Old page format) */
typedef unsigned TS;	    /* TS = time stamp */

typedef unsigned uns;
typedef unsigned char CHAR;

typedef int (*PFN)();  /* pointer to function */
typedef CHAR far *LPCH;
typedef CHAR *PCH;

/* generic huge pointer stuff */


/* on the Mac we allocate far * ptrs for file system structures and
	far * far * ptrs for external PLCs. In WIN everything is allocated as
	huge *. We will use these conventions:
		for hungarian hp we will use declaration HUGE which will resolve to
		far * for the Mac. */
#ifdef MAC
#define HUGE	far
typedef int far * far * HQ;
#define hqNil	0L
#define LpFromHp(hp)	((int far *)(hp))
#define LpFromHq(hq)	(*(int far * far *)(hq))
#define LpLockHp(hp)	((int far *) (hp))
int far *LpLockHq();
#define UnlockHp(hp)
#define LockHq(hq)	LockQq(hq)
#define UnlockHq(hq)	UnlockQq(hq)
#define HpAlloc(l)	QAlloc(l)
#define HqAllocLcb(l)	QqAlloc(l)
#define FreeHp(hp)	FreeQ(hp)
#define FreeHq(hq)	FreeQq(hq)
#define HpBaseForIbp(ibp) ((char far *)&vbptbExt.hprgbpExt[ibp])
#define WinMac(win, mac)   (mac)
#else /* WIN */
#define WinMac(win, mac)   (win)
typedef IB HUGE * HQ;
#endif

/* for source compatibilty with Opus huge block manipulations */
struct BPS HUGE *HpbpsIbp();



/* T Y P E D  V A L U E S */

#define fTrue 1
#define fFalse 0

/* these define the possible values for a tri-state flag */
#define tPos	1
#define tZero	0
#define tNeg	(-1)
/* alternate interpretation... */
#define tYes	1      /* much like fTrue */
#define tNo		0      /* much like fFalse */
#define tMaybe	(-1)   /* the "different" state */

#define fnNil 0
#define wwNil 0
#define mwNil 0
#define pnNil (-1)
#define dlNil (-1)
#define idrNil (-1)

#define pfnNil ((PFN)0)
#define prmNil 0
#define docNil 0
#define cpNil ((CP) -1)
#define cpMax 0x7FFFFFFFL
#define cp0 ((CP) 0)
#define pn0 ((PN) 0)
#define fc0 ((FC) 0)
#define fcNil ((FC) -1)
#define bNil 0
#define hNil 0
#define iNil (-1)
#define pNil 0
#define valNil (-1)
#define idNil (-1)

#ifdef WIN
#define ftcSymbol	1
#define ftcHelv 	2
#define ftcMinUser	3 /* ftc passed system fonts (tms rmn, sym, helv */
#else
#define ftcSymbol      23
#define ftcHelv 	vftcHelvetica
#define ftcMinUser	1 /* ftc passed system fonts */
#endif


/* layout modes */
#define lmNil		0
#define lmPrint 	1
#define lmPreview	2
#define lmFRepag	3
#define lmBRepag	4
#define lmPagevw	5


/* M A C R O S */

/* an empty pragmat to mark locally used procedures */
#define private

#define SetWords(pw, w, cw) bltcx(w, (int far *)(pw), cw)

#define SetBytes(pb, w, cb) bltbcx(w, (char far *)(pb), cb)

#define CwFromCch(cch)	(((unsigned)((cch) + sizeof (int) - 1))/sizeof (int))

#define offset(s, field)      ((uns)&(((struct s *)0)->field))
#define cbof(foo)  sizeof(foo)
#define cwof(foo)  CwFromCch(cbof(foo))


#ifdef MAC
#define WriteHprgchToFn(fn, hpch, cch) ScanFnForBytes(fn, hpch, cch, fTrue)
#define ReadHprgchFromFn(fn, hpch, cch) ScanFnForBytes(fn, hpch, cch, fFalse)
#define WriteRgchToFn(fn, pch, cch)	ScanFnForBytes(fn, (char HUGE *)(pch), cch, fTrue)
#define ReadRgchFromFn(fn, pch, cch) ScanFnForBytes(fn, (char HUGE *)(pch), cch, fFalse)
#define FDrawTableDrsWw(ww)  FDrawTableDrsPref
#define FDrawPageDrsWw(ww)   FDrawPageDrsPref
#endif /* MAC */

#ifdef WIN
#define InvalLlc()
#define UnprotectLlc()
#define Break1()
#define Break2()
#define Break3()
#define BreakW(w)
#define SetHairline()
#define ResetHairline()
#else
#define InvalLlc()	InvalLlcProc()
#ifdef DEBUG
#define Break1()	Break1Proc()
#define Break2()	Break2Proc()
#define Break3()	Break3Proc()
#define BreakW(w)	BreakWProc(w)
#else
#define Break1()
#define Break2()
#define Break3()
#define BreakW(w)
#endif
#endif


/* interpreter flag for address checking */
#ifdef MAC
extern int		fCheckPtr;
#else
#define fCheckPtr	*((*(char far *far *) 0xa78) - 114)
#endif

#ifdef DEBUG
#define TurnOnPtrCheck()	(fCheckPtr = fTrue)
#define TurnOffPtrCheck()	(fCheckPtr = fFalse)
#else
#ifdef CHECKPTR
#define TurnOnPtrCheck()	(fCheckPtr = fTrue)
#define TurnOffPtrCheck()	(fCheckPtr = fFalse)
#else
#define TurnOnPtrCheck()
#define TurnOffPtrCheck()
#endif
#endif

/* MACROS FOR ROUTINES ONCE IN SELECT.C */

#define SetSelCurSels(psels)   (SetPselSels(&selCur, psels))
#define Select(psel, cpFirst, cpLim)	(Select1(psel, cpFirst, cpLim, maskSelChanged))
#ifdef INEFFECIENT
#define SelectIns(psel, cpFirst)    (Select1(psel, cpFirst, cpFirst, maskSelChanged))
#endif

/* CODE-SEGMENT CONSTANT STRING ALLOCATIONS */

/* These macros initialize st or sz.  When executed, they copy string to
	frame and return a near pointer to it.
*/
string char *StringMap();
#define StFrame(s)	StringMap(s, 1, 1)  /* init st, copy to frame */
#define SzFrame(s)	StringMap(s, 0, 1)  /* init sz, copy to frame */
#define StShared(s)	StringMap(s, 1, 2)  /* init st, copy to shared frame */
#define SzShared(s)	StringMap(s, 0, 2)  /* init st, copy to shared frame */

/* keyed versions of above */
#define StFrameKey(s, key)	StringMap(s, 1, 1, key)
#define SzFrameKey(s, key)	StringMap(s, 0, 1, key)
#define StSharedKey(s, key)	StringMap(s, 1, 2, key)
#define SzSharedKey(s, key)	StringMap(s, 0, 2, key)

/* Used to initialize non-local cs st's */
#define St(s)		StringMap(s, 1, 1)
#define StKey(s, key)	StringMap(s, 1, 1, key)
#define SzKey(s, key)	StringMap(s, 0, 1, key)

/* Used to initialize st's in global DATA space */
#define StGlobal(s)	StringMap(s, 1, 0)  /* init st in global space */
#define StGlobalKey(s, key)	StringMap(s, 1, 0, key)
#define SzGlobalKey(s, key)	StringMap(s, 0, 0, key)

/* Casts of cs strings which perform implicit copies to frame */
#define StNear(s)	((char *)(s))	/* copies global cs string to frame */
#define SzNear(s)	((char *)(s))	/* copies global cs string to frame */

/* Copies st instance from code space to a local st */
#define CopyCsSt(qstFrom, pstTo)  bltbx(qstFrom, (char far *)pstTo, *qstFrom + 1)
#define CopyCsSz(qszFrom, pszTo)  bltbx(qszFrom, (char far *)pszTo, sizeof(qszFrom))

#ifdef WIN
#define MovePwToPch(pw, pch)	*(int *)pch = *(pw)
#else /* MAC */
#define MovePwToPch(pw, pch)	bltb((pw), pch, sizeof(int))
#endif

/* C U R S O R S */
#define crsArrow	0	/* entries in rgcurs in main.c */
#define crsIbeam	1
#define crsHourglass	2
#define crsBackarrow	3
#define crsPlus 	4
#define crsMinus	5
#define crsDownArrow	6
#define crsSplit	7
#define crsOutline	8
#ifdef MAC
#define crsOtlVert	9
#define crsOtlHorz	10
#define crsKeyPlus	13
#endif
#define crsItalicIbeam	11
#define crsHelp 	12
#define crsCross	101	/* module-specific cursors */
#define crsPageno	102
#ifdef WIN
#define crsOtlVert	vhcOtlVert
#define crsOtlHorz	vhcOtlHorz
#endif


/* P E R F O R M A N C E */

/* base length of chr buffer */
#define bchrMaxInit 100
#define bchrMaxFree 100 /* maximum number of unused bytes before dealloc */

/* max byte size of strings in strings.m4 */
#define ichMaxIDSTR 100

#define cchMaxLong	11	/* max length of char. translations of longs*/
#define cchBkmkMax	21	/* Length of bookmark text */
/* max number of bookmarks in a document */
#define ibkmkMax    16381 /* (2^14)-30: dde str has only 14 bits */
/* nil value for DDE (stored in 14 bits) */
#define ibkmkNil    ibkmkMax
/* bookmark classes */
#define bmcUser     1 /* user defined */
#define bmcDoc	    2 /* document pre-defined */
#define bmcSelect   4 /* selection pre-defined */
/* special pre-defined bookmarks for DDE and macros*/
#define ibkmkSpecialMin ibkmkMax
#define ibkmkSODoc    (ibkmkSpecialMin+1)
#define ibkmkEODoc    (ibkmkSpecialMin+2)
#define ibkmkDoc      (ibkmkSpecialMin+3)
#define ibkmkSpecialDocMax ibkmkDoc
#define ibkmkSel      (ibkmkSpecialMin+4)
#define ibkmkPrevSel1 (ibkmkSpecialMin+5)
#define ibkmkPrevSel2 (ibkmkSpecialMin+6)
#define ibkmkLine     (ibkmkSpecialMin+7)
#define ibkmkPar      (ibkmkSpecialMin+8)
#define ibkmkChar     (ibkmkSpecialMin+9)
#define ibkmkSect     (ibkmkSpecialMin+10)
#define ibkmkHeading  (ibkmkSpecialMin+11)
#define ibkmkPage     (ibkmkSpecialMin+12)
#define ibkmkTable    (ibkmkSpecialMin+13)
#define ibkmkSOSel    (ibkmkSpecialMin+14)
#define ibkmkEOSel    (ibkmkSpecialMin+15)
#define ibkmkCell     (ibkmkSpecialMin+16)

/* class maxima */
#define docMax	50
#define fnMax	50
#define wwMax	30
#define mwMax	WinMac(15,30)

/* heap emergency allocations */
/* since plc's are not stored in the local heap, not much point in having
	big numbers here */
#define cwEmerg1	WinMac(100,200)
#define cwEmerg2	240
#define cwEmerg3	120
#define lcbEmergSave	1024L
#define lcbEmergFar	8192L

/* special ch extensions */
#define cchChSpecMax 30

/* C L A S S  T Y P E S */
#define clsDOD 0
#define clsFCB 1
#define clsWWD 2
#define clsMWD 3
#ifdef WIN
#	define clsDCLD 4
#	define clsMax 5
#else
#	define clsMax 4
#endif

/* Footnote position codes */
#define fpcEndnote	0
#define fpcBottomPage	1
#define fpcBeneathText	2
#define fpcEndDoc	3

/* stages */
#define istgInit	0	/* initialization */
#define istgMain	1	/* normal operation */
#define istgQuit	2	/* quitting */
#define istgInitPrint	3	/* printing from init */
#define istgSuspend	4	/* suspended by Switcher */
#define istgPrint	5	/* printing */


/* M E A S U R E S */
/* definition of xa unit */
#define dxaInch 1440
/* conversion from ya to points/vertical screen pixels */
#define dyaPoint 20

/* arbitrary base for fRightAbs right margin */
#define xaRightStd (6*dxaInch)


#define PTRUE (-1)	/* Pascal fTrue - used for toolbox calls */
#define NULL 0
#define LNULL 0L

#define cchINT	(sizeof (int))

#define cchMaxSt	256
#define cchMaxSz	256
#define cchMaxNum	10

#define ichUsrInitlMax	6 /* stz size of user initial */

/* S T R U C T U R E S */
/* class definition structure */
struct CLSE
	{
	int	*mpfoohfood;
	int	*pfooMac;
	int	fooMax;
	int	cwFood;
	int	bfFooFull;
	};


/* WIN stores STTB's in winword.ini, invalidate that if you change
	this structure */
struct STTB
	{
	uns	bMax;
	int	ibstMac;
	int	ibstMax;
	union
		{
		struct {
			int	cbExtra : 13;
			int	fNoShrink : 1;
			int	fExternal : 1;
			int	fStyleRules : 1;
			};
		int cbExtraOld;
		};
	union
		{
		int	rgbst[2];
		HQ	hqrgbst;
		};
	/*char	grpst*/
	};
#define ibstNil -1
#define cbSTTBBase offset(STTB, rgbst)
#define cwSTTBBase (cbSTTBBase / sizeof(int))
#define cbSTTB (sizeof(struct STTB))
#define cwSTTB (sizeof(struct STTB)/ sizeof(int))

struct CA
	{
	CP	cpFirst;
	CP	cpLim;
	int	doc;
	};

#define cwCA   (sizeof (struct CA) / sizeof (int))

/* short selection is a subset of SEL that is remembered in windows */
/* 8 bit booleans are used for better code compression. These fields
are near the beginning of the structure so than LDIBn may be used */

/* NOTE if fields are added to the SELS, the size of fib.rgwSelsSpare must be
	reduced by the amount added. (for MAC only) */
struct SELS
	{
/* means selection is marked in a special way because it is a picture
or the like */
	union {
		struct {
			int	: 10;
			int	sk : 6;
			};
		struct {
			int fRightward : 1;
			int fSelAtPara: 1;
			int fWithinCell : 1;
			int fTableAnchor : 1;
			int : 6;
			int fColumn : 1;
			int fTable :  1;
			int fGraphics : 1;
			int fBlock : 1;
			int fNil : 1;
			int fIns : 1;
			};
		};
/* direction of selection, used for choosing anchor for extend */
	int	fForward : 8;
/* if on, the insertion is drawn after the last char of line (dl-1)
rather than before the first char of line dl (same cp position!) */
	int	fInsEnd : 8;

	union {
		struct {
			CP	cpFirst;
			union {
				CP	cpLim;
				CP	cpLast;
				};

			int	doc;
			};
		struct CA	ca;
		};
	union
		{
		struct
			{
			int	xpFirst;
			int	xpLim;
			};
		struct
			{
			int	itcFirst;
			int	itcLim;
			};
		};
	CP	cpAnchor;
	int	sty;
	int	itcOppositeAnchor;
#ifdef WIN
	CP	cpAnchorShrink; /* anchor point for shrink selection */
#endif
	};

#define cwSELS (sizeof (struct SELS) / sizeof (int))

struct MERR
	{
	int  hrgwEmerg1;   /*  if hNil, we have stage 1 heap emergency */
	int  hrgwEmerg2;   /*	for use by Save   */
	int  fDocFull;	   /* maximum number of docs has been allocated */
	int  fFnFull;	   /* maximum number of fns has been allocated */
	int  fWwFull;	   /* maximum number of wws has been allocated */
	int  fMwFull;	   /* maximum number of mws has been allocated */
	struct	{
		int  fSystemLocked : 1; /* system disk was locked at init time */
		int  fWordLocked : 1;	/* Word disk was locked at init time */
		int  fLockAlert : 1;	/* system locked alert has been given */
		int  fScratchFileInit : 1; /* initial state of vfScratchFile */
		int  fUndoCancel : 1;	/* user wants to cance op if no room for undo */
		int  fNovellNet : 1;    /* dumb user is using a Novell Network! */
		int  fSaveFail : 1;	/* save was not successful */
		int  fPostponeScratch : 1;/* WIN: have not tried to create
		                        scratch file, always FALSE for MAC */
		int  fErrorAlert: 1;	/* alert has been reported */
		int  fHadMemAlert: 1;	/* error message for mat failure has been reported */
		int  fInhibit: 1;	/* inhibit error reporting if true */
		int  fHadPrintAlert : 1; /* reported disk error for printing */
		int  fNoHdrDr : 1;	/* header DR not created due to doc shortage */
		int  fHadDispAlert: 1;	/* reported matDisp (once on, always on) */
		int  fSdmInit : 1;	/* sdm has been initialized */
		int  fReclaimHprcs: 1;	/* set when we change piece tables or record a
											new prc, so that orphaned prcs can be
											reclaimed. */
		};
	union {
		int  fMemFail;
		int  mat;		/* Memory Alert Type */
		};
        union {
                int  fDiskFail; /* something is wrong on disk */
                struct {
                    int fDiskEmerg : 1; /*  reserve buffers are released for use by fnScratch */
                    int fDiskAlert : 1; /* disk full alert has been given  */
                    int fDiskWriteErr : 1; /* disk write I/O error detected */
                    int : 13;
                    };
                };

#ifdef WIN
	int fPrintEmerg;    /* Set if printer DC is unavailable */
	int fDclFull;	    /* maximum number of dcls (dde channels) allocated */
#endif
	int  hrgwEmerg3;   /* for use by EmergSave   */
	int  fWarnDocTooBig;   /* set if some doc has grown beyond cpWarnTooBig */
#ifdef MAC
	long qqrgwEmergSave;	/* Mac - to make sure we are able to save */
#endif
	int prmDontReclaim;   /* used by ReplaceCps */
	union
        {
        struct
            {
    		int fKillRuler : 1;
    		int fKillRibbon : 1;
            int fFmtFailed : 1;       /* indicates failure of ApplyGrpprlCa/PrmAppend/PrmFromPrc */
    	    int fNoDrs : 1;
    	    int fSavePheFailed : 1;	/* para height save failed; reset next layout */
			int fCompressFailed : 1; /* piece table compression failed--don't try again */
    		int fCBTMacroMemErr : 1; /* had a mem err while running a macro inside CBT */
			int fSentCBTMemErr : 1;  /* already sent CBT the mem err CBTTERM */
			int : 8;
    		};
    	struct 
    		{
    		int	fKillRulRib : 2;
    		int	: 14;
            };
	    };
	};

/* memory alert types */

#define matNil		0
#define matLow		1	/* equivalent to old vmerr.fLowAlert */
#define matReplace	2	/* equivalent to old vmerr.fReplaceFail */
#define matMem		3	/* equivalent to old vmerr.fMemAlert. WARNING: */
				/*  matMem also declared in asm\consts.inc !   */
#define matDisp         4       /* can't update display */
#define matMenu         5       /* can't drop menu */
#define matFont         6       /* can't realize font */
#define matCBT		7   /* OOM inside tutorial - have to terminate CBT */

#ifdef DEBUG
extern BOOL vfNovellNetInited; /* indicates vmerr.fNovellNet is valid */
#endif /* DEBUG */
	
struct TAD  /* Text Append Descriptor */
	{
	char *pchBegin;
	int cch;
	int cchMax;
	int fDidntAppend;
	int fOverflow;
	};

struct FRC /* Fraction */
	{
	int numer;
	int denom;
	};

/* L A Y O U T */
/* used to signal default footnote header and continuation header */
#define ihddTFtn	(-2)
#define ihddTFtnCont	(-3)

#define ihdtNil 	(-1)
#define clNil		(-1)
#define ihddNil 	(-1)
#define clMax		0x7fff
#define pgnMax		0x7fff
#define ypLarge 	0x3fff	/* bigger than any page; *2 is still positive */

/* constant indices into rghdt */
#define ihdtTLeft	0	/* SEP */  /* even header */
#define ihdtTRight	1	/* odd header */
#define ihdtBLeft	2	/* even footer */
#define ihdtBRight	3	/* odd footer */
#define ihdtTFirst	4
#define ihdtBFirst	5
#define ihdtTFtn	6	/* DOP */
#define ihdtTFtnCont	7
#define ihdtBFtnCont	8

#define ihdtMaxSep	6	/* number from SEP */
#define ihdtMax 	9	/* number total */
#define chdtFtn 	3


/*
Paragraph properties
*/
/* paragraph height, version for Word 4.0 */
struct PHE
	{
	union
		{
		struct
			{
			int	fSpare : 1;
			int     fUnk : 1;       /* phe entry is invalid */
			int     fDiffLines : 1; /* total height is known, lines are different */
			int     : 5;
			int     clMac : 8;
			} ;
		int	w0;
		};
	int	dxaCol;

	union	{
		/* when !fDiffLines, this variant is used. */
		uns	dylLine;
		uns	dyaLine;	/* WIN version of dylHeight */
		/* when fDiffLines, this variant is used */
		uns	dylHeight;
		uns	dyaHeight;	/* WIN version of dylHeight */
		/* when papxs are stored in STSH this variant is used */
		int	fStyleDirty;
		};
	};
#define cbPHE (sizeof(struct PHE))
#define cwPHE (cbPHE / sizeof(int))


/* P R O C E D U R E S */
FC FcAppendRgchToFn();
NATIVE FC FcAppendHprgchToFn();

FC FcBegOfExt();
NATIVE FC FcFromPo();

CP CpInPage();
CP CpInNextPage();
CP CpFirstPage();
CP CpMacDocEdit(), CpMacDoc(), CpMac1Doc(), CpMac2Doc();
CP CpLimSty(), CpFirstSty();
CP CpLimStyLine(), CpFirstStyLine();
CP CpTail();
CP CpFirstForItc();
#ifdef DEBUG
long MenuSelectProc();
long HpmSaveDbcProc();
long GrowWindowProc();
long DragGrayRgnProc();
#endif /* DEBUG */
struct DOD *PdodDoc();
struct DOD *PdodMother();
struct WWD *PwwdWw();
struct WWD **HwwdWw();
struct DR *PdrWw();
struct WWD *PwwdSetWw();
struct WWD *PwwdUpper();
struct WWD *PwwdOther();
struct WWD *PwwdRulerWw();
struct MWD *PmwdWw();
struct MWD *PmwdMw();
struct FCB *PfcbFn();
CP CpFirstIdrDlFromDocCp();
CP CpBackVisibleOutline();
CP CpLimBlock();
CP CpBackOverInvisible();
struct EDL *PedlCpFromWwDl();
struct EDL *PedlLastPwwd();
CP CpOutlineAdvance();
CP CpFirstFull();
int far * QAlloc();
int far * far *QqAlloc();
long LcbQq();
long LcbAvailForVol();
long LcbFreeMem();
CP CpActiveCur();
CP CpLimSubText();
CP CpFirstRef();
CP CpRefFromCpSub();
int far * far *QqSaveDbc();
int far * far *QqSaveSFDbc();
CP CpLimDl();
CP CpLimNoSpaces();
long /* PIXMAPHANDLE */ HNewPixMap();
long /* PIXMAPHANDLE */ HGetGPixMap();
long /* PIXMAPHANDLE */ HGetPixMap();
long /* PIXMAPHANDLE */ HpmSaveDbc();
long /* PIXMAPHANDLE */ HpmSaveSFDbc();
struct CA *PcaSetWholeDoc();
struct CA *PcaSetNil();
struct CA *PcaPoint();
CP CpFromDlXp();
CP CpFromXpVfli();
CP CpFirstTap();
CP CpFirstTap1();
CP CpTableFirst();
CP CpTableLim();
struct SEL *PselActive();
CP CpSkipFormula();
CP CpScaleThumb();
struct PLC **HplcedlWw();
CP CpLimFtnCp();


int HUGE *HpibpHash();


/* S T R U C T U R E   C O D E S */
/* for HCopyHeapBlock */
#define strcPL	    1
#define strcPLC     2
#define strcSTTB    3
#define strcPLAIN   4

/* Copied from syscodes.h */

uop int UOP();
uop void VUOP();


/* clipping types */
#define cptNil		-1	/* unknown rectangle */
#define cptAll		1	/* whole window contents */
#define cptDoc		2	/* only document contents for pane */
#define cptDocSelBar	3	/* document and selbar contents for pane */
#define cptSame 	4	/* don't change clipping */
#define cptAllButScroll 5	/* all of window except scroll bars */


/* NATIVE routines */

/* clsplc.c */
NATIVE BOOL FInitHplcedl();
NATIVE struct PLC **HplcInit();

/* chkc.s */
#ifdef DEBUG
NATIVE AssertNative();
#endif
NATIVE CopyRgch();
NATIVE CopyQrgch();

/* debug1.c */
#ifdef DEBUG
NATIVE MarkProc();
#endif /* DEBUG */

/* debughp.c */
#ifdef MAC
#ifdef DEBUG
NATIVE CkHeap();
native ShakeHeap();
#endif /* DEBUG */
#endif /* MAC */

/* disp1.c */
NATIVE DisplayFli();
NATIVE DisplayFliCore();
NATIVE MarkSel();
NATIVE int C_FMarkLine();
NATIVE AddVisiSpaces();
NATIVE DrawBarTabs();
NATIVE int WBeginCursorProtect();
NATIVE EndCursorProtect();
NATIVE MoveToByDxu();
NATIVE PatBltRc();

/* disp2.c */
NATIVE FUpdateDr();
NATIVE ScrollDrUp();


/* disp3.c */
NATIVE ScrollDrDown();
#ifdef MAC
NATIVE GetXlMargins();
#endif /* MAC */
NATIVE DlkFromVfli();
NATIVE FreeEdls();
NATIVE FreeEdl();

/* disptbl.c */
NATIVE FUpdateTable();
NATIVE FrameTableLine();

/* dlgSubs.c */
NATIVE int GetItValue();
NATIVE SetItValue();
NATIVE GetItTextValue();
NATIVE SetItTextValue();
NATIVE SetRadValue();
NATIVE int GetRadValue();
NATIVE DisableIt();
NATIVE EnableIt();
NATIVE GetDialogHtm();
NATIVE int FItDisable();
NATIVE WToTmc();
NATIVE WToItm();
NATIVE int ItmFromTmc();
NATIVE IagFromItm();

/* editspec.c */
NATIVE SetPlcUnk();
NATIVE InvalTableProps();
NATIVE InvalCp2();

/* fetch.c */
NATIVE FetchCp();
NATIVE ChLower();
NATIVE ChFetch();
NATIVE CachePara();
NATIVE FInTableDocCp();
NATIVE StandardChp();
NATIVE StandardPap();
NATIVE StandardSep();
NATIVE CHAR HUGE *HpchGetPn();

/* fetch1.c */
#ifdef MAC
NATIVE StStAppend();
NATIVE int WCompSt();
NATIVE int WCompSzCase();
#endif /* MAC */
#ifdef DEBUG
NATIVE int FRareProc();
#endif /* DEBUG */
NATIVE GenStyleNameForStcp();
NATIVE CP CpFromCpCl();
NATIVE int DxtLeftOfTabStop();

/* fetchtb.c */
NATIVE CacheTapCore();
NATIVE CP CpFirstTap1();
NATIVE CP CpLimTableCell();
NATIVE CacheTc();
NATIVE CalcHorizTableBorders();
NATIVE FEasyBorders();
NATIVE FEasyBrc();
NATIVE ItcGetTcxCache();
NATIVE ItcGetTcx();

/* fieldfmt.c */
NATIVE FFormatFieldPdcp();
NATIVE CP DcpSkipFieldChPflcd();
NATIVE IfldFromDocCp();
NATIVE FillIfldFlcd();
NATIVE IfldInsertDocCp();
NATIVE FetchCpPccpVisible();
NATIVE CP CpVisibleCpField();

/* format.c */
NATIVE	FormatLine();
NATIVE	FormatLineDr();
NATIVE	FormatLineDxa();
NATIVE	DxpFromCh();
NATIVE	DxpFromChNat();
NATIVE	LoadFont();
NATIVE	DxuExpand();
NATIVE	DxuExpandNat();
NATIVE	XpFromDcp();
NATIVE	long InitFormat();

/* hdd.c */
CP CpMomFromHdr();

/* heap.c */
NATIVE int **HAllocate();
NATIVE struct HH *PhhAllocate();
NATIVE FPhhAlloc1();
NATIVE RemoveFree();
NATIVE FreeH();
NATIVE FreePh();
NATIVE FreePhh();
NATIVE FChngSizeH();

/* ihdd.c */
NATIVE int FGetValidPhe();

/* index2.c */
NATIVE int FRgchCompare();

/* indexSub.c */
NATIVE int FTermChar();

/* insSubs.c */
NATIVE AddRun();
NATIVE ScanFnForBytes();
NATIVE int CbGrpprlProp();
NATIVE int CbGenPrl();
NATIVE int CbGenChpxFromChp();

/* layout.c */
#ifdef MAC
NATIVE int LbcFormatPage();
NATIVE int ClFormatLines();
#endif /* MAC */

/* loadfont.c */
NATIVE SetLaserFti();

/* main.c */
NATIVE SzToSt();

/* menu */
NATIVE GetParaState();
NATIVE int ValFromSprm();
NATIVE GetUcmInfo();
NATIVE UcmFromChm();
NATIVE MnyFromUcm();
NATIVE unsigned long CesCurrent();

/* openW1.c */
NATIVE int IrhctFromPap();
NATIVE int FFodcFromFpropStream();
NATIVE FindFodForFc();
NATIVE CopyFodToCache();
NATIVE FlipLongWord();

/* prcSubs.c */
NATIVE int ValFromPropSprm();
NATIVE ConvHpsToSiapPrl();

/* printSub.c */
NATIVE int FGetValidPaph();
NATIVE int FOKParaHeight();

/* res.c */
NATIVE HQ HqAllocLcb();
NATIVE SB SbFindCb();

/* rtf.c */
NATIVE int FSearchPlftcm();
NATIVE int FSearchRgrsym();
NATIVE FlipWord();

/* rulerDrw.c */
NATIVE DrawMinorTicks();

/* saveW1.c */
NATIVE int CchDiffer();

/* scc.c */
NATIVE int DlUpdateFromScc();

/* search.c */
NATIVE CP CpSearchSz1();

/* select.c */
NATIVE int FWhiteSpaceCh();
NATIVE int WbFromCh();
NATIVE IdrFromPt();

/* sort.c */
NATIVE FCompareSzSz();

/* sttb */
NATIVE FChangeStInSttb();
#ifdef MAC
NATIVE char *PstFromSttb();
#endif /* MAC */
CHAR HUGE *HpchExtraFromSttb();
NATIVE GetStFromSttb();
NATIVE SortSiftUpRg();
NATIVE SortRg();

/* stysubs.c */
NATIVE FChkNamesMatchOneStyle();
NATIVE FCopySubname();
NATIVE FMatchDefinedAndStdStyles();
NATIVE FMatchSubnamesInStExact();
NATIVE FMatchSubnamesInSt();
NATIVE FAppendToStyleName();
NATIVE TranslateStUpper();

/* plc.c */
NATIVE	CP far * LprgcpForPlc();
NATIVE	CP DcpAdjust();
NATIVE	IcpInRgcpAdjusted();
NATIVE	IInPlc();
NATIVE	IInPlc2();
NATIVE	IInPlcCheck();
NATIVE	IInPlcRef();
NATIVE	IInPlcMult();
NATIVE	IInPlc2Mult();
NATIVE	IInPlcQuick();
NATIVE	IcpInRgcp();
NATIVE	AdjustHplc();
NATIVE	CompletelyAdjustHplcCps();
NATIVE	AdjustHplcCpsToLim();
NATIVE	AdjustHplcCps();
NATIVE	AdjustHplcedlCps();
NATIVE	AddDcpToCps();
NATIVE	IMacPlc();
NATIVE	PutIMacPlc();
NATIVE	GetPlc();
NATIVE	PutPlc();
NATIVE	PutPlcLastDebugProc();
NATIVE	PutPlcLastProc();
NATIVE	char far * QInPlc	();
NATIVE	CP CpPlc();
NATIVE	CP CpMacPlc();
NATIVE	PutCpPlc();
NATIVE	CopyMultPlc();
NATIVE	char * PInPl();
NATIVE	BltInPlc();
NATIVE	InitPlcedl();
NATIVE	long InitPlc();

/* util.c */
NATIVE	struct DOD * PdodDoc();
NATIVE	DocMother();
NATIVE	struct DOD * PdodMother();
#ifdef MAC
NATIVE	DocBaseWw();
#endif /* MAC */
NATIVE	CP CpMacDoc();
NATIVE	CP CpMac1Doc();
NATIVE	CP CpMacDocEdit();
NATIVE	CP CpMac2Doc();
#ifdef MAC
NATIVE	IpadMac();
#else /* WIN */
#define IpadMac(hplcpad) (IMacPlc(hplcpad)-2)
#endif /* WIN */
NATIVE	struct CA *PcaSet();
NATIVE	struct CA *PcaSetDcp();
NATIVE	CP DcpCa();
NATIVE	struct CA *PcaSetWholeDoc();
NATIVE	struct CA *PcaSetNil();
NATIVE	struct CA *PcaPoint();
NATIVE	InvalCa();
NATIVE	FInCa();
NATIVE	FOverlapCa();
NATIVE	FNeRgw();
NATIVE	FNeSt();
NATIVE	FNeRgch();
NATIVE	FNeHprgch();
NATIVE	FNeChp();
NATIVE	CP CpMax();
NATIVE	CP CpMin();
NATIVE	min();
NATIVE	max();
NATIVE	abs();
NATIVE	uns umax();
NATIVE	uns umin();
NATIVE	CchNonZeroPrefix();
NATIVE	FIsectIval();
NATIVE	DrcToRc();
NATIVE	RcToDrc();
#ifdef MAC
NATIVE	DyOfRc();
NATIVE	DxOfRc();
NATIVE	FEmptyRc();
NATIVE	FSectRc();
#endif /* MAC */
#ifdef WIN
#define SectRc(prc1,prc2,prcDest) FSectRc(prc1,prc2,prcDest)
#endif /* WIN */
NATIVE	UnionRc();
#ifdef MAC
NATIVE	SectRc();
NATIVE	MoveRc();
#endif /* MAC */
NATIVE	struct PT PtOrigin();
NATIVE	RclToRcw();
NATIVE	RcwToRcl();
NATIVE	RcwToRcp();
NATIVE	RcpToRcw();
NATIVE	RceToRcw();
NATIVE	DrclToRcw();
NATIVE	DrcpToRcl();
NATIVE	DrcpToRcw();
NATIVE	XlFromXw();
NATIVE	YlFromYw();
NATIVE	XwFromXl();
NATIVE	XeFromXl();
NATIVE	XlFromXe();
NATIVE	YwFromYl();
NATIVE	XwFromXp();
NATIVE	YwFromYp();
NATIVE	XpFromXw();
NATIVE	YpFromYw();
NATIVE	XwFromXa();
NATIVE	XaFromXw();
NATIVE	DxaFromDxp();
NATIVE	DxpFromDxa();
NATIVE	struct WWD *PwwdWw();
NATIVE	struct WWD **HwwdWw();
NATIVE	struct MWD *PmwdMw();
NATIVE	struct MWD *PmwdWw();
NATIVE	FKcmInter();
#ifdef MAC
NATIVE	struct DR *PdrWw();
#endif /* MAC */
NATIVE	WwFromHwwd();
NATIVE	struct DR *PdrFetchAndFree();
NATIVE	struct DR *PdrFreeAndFetch();
NATIVE	struct DR *PdrFetch();
NATIVE	FreePdrf();
NATIVE	int HUGE * HpInPl();
NATIVE	CHAR HUGE *HpstFromSttb();
NATIVE	GetStFromSttb();
NATIVE	FStcpEntryIsNull();
NATIVE	DxsSetDxsInch();
NATIVE	long InitUtil();
NATIVE	InvalCaFierce();
#ifdef WIN
#define FSetRgwDiff(pwBase, pwNew, pwDiff, cw) \
	FSetRgchDiff(pwBase, pwNew, pwDiff, cw<<1)
#endif /* WIN */

/* wwact.c */
NATIVE FreeDrs();

#define PsccWw(ww)	((struct SCC *)PwwdWw(ww))

#ifdef DEBUG
#ifdef WIN
extern BOOL vfAllocGuaranteed;
#define StartGuaranteedHeap()	(vfAllocGuaranteed++)
#define EndGuarantee()		(vfAllocGuaranteed--)
#else /* MAC */
#define StartGuaranteedHeap()
#define EndGuarantee()
#endif
#else /* NONDEBUG */
#define StartGuaranteedHeap()
#define EndGuarantee()
#endif

/* Additional, system-specific definitions */

#ifdef MAC
#include "wordmac.h"
#else
#include "wordwin.h"
#endif
