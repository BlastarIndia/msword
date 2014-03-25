/* wordwin.h -- Win-specific sidecar to word.h */

#include "toolbox.h"
#ifndef NOSETJMP
#include "qsetjmp.h"
#endif

/* Far heaps/etc */
#define hqNil	    	hpNil
#define sbNil		0
#define hpNil 	    	(HpOfSbIbConst(sbNil, 0))
SB  	    	SbNext();


#ifdef SCREENDEFS
#include "screen.h"
#endif

#define fWin	fTrue	    /* used for WORDTECH shared code */
#define fMac	fFalse
#define Win(f)	f
#define Mac(f)

#define VOID    void
#define VAL	int
#define MD	int
#define IDFLD	int
#define IDSTR	int

#define ipstMemWarn         15   /* from syschg.c */
#define ipstCoreLoad        16
#define ipstConversion      17
#define ipstDrawName        18
#define ipstDrawClass       19
#define ipstNovellNet       24
#define ipstAs400				 25


#define pgnMaxStart (5000)

struct DTTM    /* Opus internal date format */
		{
		union
		{
		long lDttm;
		struct
			{
			unsigned mint:	 6; /* 0-59 */
			unsigned hr:	 5; /* 0-23 */
			unsigned dom:	 5; /* 1-31 */
			unsigned mon:	 4; /* 1-12 */
			unsigned yr:	 9; /* (1900-2411)-1900 */
			unsigned wdy:	 3; /* 0(Sun)-6(Sat) */
			};
		};
		};

#define dyrBase 		1900
#define dttmYrFirst		dyrBase
#define dttmYrLast		2411
#define dttmMonFirst		1    /* Of course, the first month is January */
#define dttmMonLast		12   /* and the last month is December.       */

#define DttmOfMDY(m,d,y)    ((((y)&0x1ffL)<<20)+(((m)&0xfL)<<16)+(((d)&0x1fL)<<11))

struct DTTM DttmCur();

string CHAR *SzMap();
string CHAR *StMap();

#define ichMaxBufDlg	256	/* size for SDM string buffers */
#define ichMaxNum	11	/* Longest dialog box number expansion string incl null term*/
#define ichChSpecMax 	20
#define cchMaxInt	 7
#define cchMaxLong	11
#define cchGlsyMax 	32

/* misc defines taken from macword code */
/* standard tm values */
#define tmvalNinch      (-1)
#define tmvalDisable    (-2)

/* number of fonts noticed  (added so I could get rtf.c to compile - GregC) */
#define iftcMax 30

#define PcabFromHcab(hcab)	((AG *)&(*((HCAB)hcab))->rgag [0])
#define CopySt( stFrom, stTo )	bltbyte( (stFrom), (stTo), ((CHAR *)(stFrom)) [0] + 1)
#define FStSame( st1, st2 )	!FNeRgch( (st1), (st2), (st1) [0] + 1 )  /* until we write one */
#define FEqualSt( st1, st2 )	FStSame( st1, st2 )
#define FNoHeap(h)		((int)(h) == hNil)

#define FEqNcSz(sz1,sz2)        (!FNeNcSz(sz1,sz2))
#define FEqNcSt(st1,st2)        (!FNeNcSt(st1,st2))
#define FEqNcRgch(pch1,pch2,cch) (!FNeNcRgch(pch1,pch2,cch))

/* Intl macro - for localizing source files */
#ifdef INTL
#define Intl(value, token)  token
#else
#define Intl(value, token)  value
#endif

/*  ******************************************************************
	*****
	***** This section is copied from \ps\include\uops.h and
	*****  must be kept up-to-date with that module. We have
	*****  redefined the blt routines in lower case to match the
	*****  widespread Opus usage.
	*****
	****************************************************************** */

/*
	uops.h * Include file for Windows UOPS *
	Compiles with CS only !
*/

/*
	Macros Defined :

	LowWord(l)		: return low word (int) of long
	HighWord(l)		: return high word (int) of long
	MakeLong(lo, hi)	: make a long out of two words

	BLTB(pb1, pb2, cb)	: move from p1 to p2 (near pointers)
	BLTBX(lpb1, lpb2, cb)	: move from lp1 to lp2 (FAR pointers)

	BLTCX(w, lpw, cw)	: fill *lpw++ with w (cw = word count)
	BLTBCX(b, lpb, cb)	: fill *lpb++ with b (cb = byte count)

	BLTBV(pb1, pb2, cb)	: like BLTB but return pb2+cb
	BLTBXV(lpb1, lpb2, cb)	: like BLTBXV but return lpb2+cb

		ALLOCA(cb)              : alloca() frame allocation

*/
	/* see "huge.h" for BLTBH uop. */


uop void _UOP_VOID();
uop int _UOP_INT();
uop long _UOP_LONG();
uop CHAR *_UOP_PB();
uop CHAR FAR *_UOP_LPB();
sys void * _SYS_PV();

#define LowWord(l) _UOP_INT(2,0xe0,0xda, (long) (l))
#define HighWord(l) _UOP_INT(1,0xda, (long) (l))
#define MakeLong(lo,hi) _UOP_LONG(0, (int) (hi), (int) (lo))

#ifdef RCODE
#define BLTB(p1,p2,cb) _UOP_VOID(1,0xa6, (CHAR *) (p1), (CHAR *) (p2), (int) (cb))
#else
#define BLTB(p1,p2,cb) _UOP_VOID(2,0xe6,0xe, (CHAR *) (p1), (CHAR *) (p2), (int) (cb))
#endif /* RCODE */
#define BLTBX(lp1,lp2,cb) _UOP_VOID(2,0xe6,0xf, (CHAR FAR *) (lp1), (CHAR FAR *) (lp2), (int) (cb))

#define BLTCX(w,lpw,cw) _UOP_VOID(2,0xe6,0xc, (int) (w), (int FAR *) (lpw), (int) (cw))
#define BLTBCX(b,lpb,cb) _UOP_VOID(2,0xe6,0xd, (CHAR) (b),(CHAR FAR *) (lpb), (int) (cb))

#ifdef RCODE /* for the up and coming new PCODE compiler */
#define BLTBV(p1,p2,cb) _UOP_PB(1,0xa3, (CHAR *) (p1), (CHAR *) (p2), (int) (cb))
#else
#define BLTBV(p1,p2,cb) _UOP_PB(2,0xe6,0x20, (CHAR *) (p1), (CHAR *) (p2), (int) (cb))
#endif /* RCODE */
#define BLTBXV(lp1,lp2,cb) _UOP_LPB(2,0xe6,0x24, (CHAR FAR *) (lp1), (CHAR FAR *) (lp2), (int) (cb))

#define ALLOCA(cb)  _SYS_PV(7, (unsigned) (cb))
#define OurAllocA(cb)  ALLOCA(cb)

#undef LOWORD
#undef HIWORD
#undef MAKELONG
#define MAKELONG(lo,hi) (__FNATIVE__ ? ((long)(((unsigned)(lo)) | ((unsigned long)((unsigned)(hi))) << 16)) : MakeLong(lo,hi))
#define LOWORD(l)	(__FNATIVE__ ? ((WORD)(l)) : LowWord(l))
#define HIWORD(l)	(__FNATIVE__ ? ((WORD)(((DWORD)(l) >> 16) & 0xffff)) : HighWord(l))

#define bltbyte( pbFrom, pbTo, cb )    (__FNATIVE__ ? bltbyteNat( pbFrom, pbTo, cb ) : BLTBV( pbFrom, pbTo, cb ))
#define bltb bltbyte
#define bltbx( lpbFrom, lpbTo, cb )    (__FNATIVE__ ? bltbxNat(lpbFrom, lpbTo, cb ) : BLTBXV( lpbFrom, lpbTo, cb ))
#define bltbh( hpbFrom, hpbTo, cb )   (BLTBH( (CHAR HUGE *)(hpbFrom), (CHAR HUGE *)(hpbTo), cb ))
#define blt( pwFrom, pwTo, cw )    (int *)(__FNATIVE__ ? bltNat(pwFrom, pwTo, cw) : BLTBV( pwFrom, pwTo, (cw) * sizeof(int) ))
#define bltx( lpwFrom, lpwTo, cw )  (int FAR *)(__FNATIVE__ ? bltxNat(lpwFrom, lpwTo, cw) : BLTBXV( lpwFrom, lpwTo, (cw) * sizeof(int) ))
#define bltbcx( bFill, lpbTo, cb )	 (__FNATIVE__ ? bltbcxNat( bFill, lpbTo, cb ) : BLTBCX( bFill, lpbTo, cb ))
#define bltcx( wFill, lpwTo, cw )     (__FNATIVE__ ? bltcxNat(wFill, lpwTo, cw) : BLTCX( wFill, lpwTo, cw ))

/* following constant belongs in file.n, but is widely used */
#define ichMaxFile	120     /* same as windows OFSTRUCT szPathName */
#define cchMaxFile	ichMaxFile

#define dtickYield      15       /* frequency of PeekMessage calls */

/* Message Types, for FMsgPresent */
#define mtyAny		0x00
#define mtyNoTimer	0x01
#define mtyYield	0x04
#define mtyDoBlink	0x08
#define mtyAllPeek	0x10
#define mtySomePeek	0x20
#define mtyMouseMove    0x40
#define mtyIgnoreMouse  0x80
/* mty's for some callers */
#define mtyIdle 	(mtyDoBlink | mtyYield | mtySomePeek | mtyMouseMove)
#define mtyUpdateWw	(mtyNoTimer | mtyIgnoreMouse)
#define mtyLongDisp     (mtyNoTimer | mtyIgnoreMouse | mtyYield | mtySomePeek)
#define mtyTyping	(mtyNoTimer | mtySomePeek | mtyIgnoreMouse)

/* FormatLine modes */

#define flmDisplay	    0x00  /* values assumed in FlmDisp... macro in screen.h */
#define flmDisplayAsPrint   0x01
#define flmPrint	    0x02
#define flmRepaginate	    0x03  /* like flmPrint, but IC instead of DC */

#define flmIdle 	    0x04  /* > 2 bits, can't be stored in grpfvisi */
#define flmTossPrinter	    0x05 

/* Conversions between measures */

/* these macros convert between twips and SCREEN units.  All values
	are considered to be SIGNED INTEGERS. */

#define DxsFromDxa(pwwd, dxa) NMultDiv((dxa), vfli.dxsInch, czaInch )
#define DysFromDya(dya) NMultDiv((dya), vfli.dysInch, czaInch )
#define DxaFromDxs(pwwd, dxs) NMultDiv((dxs), czaInch, vfli.dxsInch )
#define DyaFromDys(dys) NMultDiv((dys), czaInch, vfli.dysInch )

#define DyuFromDya(dya) NMultDiv((dya), vfli.dyuInch, czaInch)
#define DxuFromDxa(dxa) NMultDiv((dxa), vfli.dxuInch, czaInch)

#define DypFromDya(dya) NMultDiv((dya), vfti.dypInch, czaInch)

/* These macros convert between points and screen units (pixels) */
#define cptInch 72	/* points / inch */
#define DxpFromDxs(dxs) NMultDiv((dxs), cptInch, vfli.dxsInch)
#define DypFromDys(dys) NMultDiv((dys), cptInch, vfli.dysInch)
#define DxsFromDxp(dxp) NMultDiv((dxp), vfli.dxsInch, cptInch)
#define DysFromDyp(dyp) NMultDiv((dyp), vfli.dysInch, cptInch)

	/* **** -+-utility-+- **** */

/* Units */
#define utInch          0 /* used for '"' string */
#define utPt            1
#define utCm            2
#define utPica          3
#define utMaxUser       4 /* largest ut for vpref.ut */
#define utP10           4
#define utP12           5
#define utLine          6

#ifndef INTL  /* FUTURE make sure statline.c, dialog3 tables updated */
#define utMax           7
#else
#define utInch2         7  /* used for "in" string */
#define utMax           8
#endif /* INTL */

/* M E A S U R E S */
/* definition of xa unit */
#define dxaInch 1440
#define czaInch 1440
#define czaMax  (22 * czaInch + 1) /* For xa items, we allow up to 22" */

#define CHILDWW_STYLE (WS_CHILD | WS_CAPTION | WS_BORDER | WS_SIZEBOX | \
	WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZEBOX)
#define WWPANE_STYLE (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)

#define xpStartFromcStagger(cStagger) vdxpStagger+(cStagger)*2*vdxpStagger
#define ypStartFromcStagger(cStagger) vdypStagger+(cStagger)*2*vdypStagger

/* max size of a generic name */
#define cchNameMax  21

/* UpdateStatusLine Options */
#define usoNormal  0
#define usoToggles 1
#define usoCache   2

/* DocOpen Flags */

#define dofNormal	0	/* Normal (like the command) */
#define dofNoWindow	1	/* Do not open a window */
#define dofNoErrors	2	/* Do not report errors */
#define dofNoMemory	4	/* Do not look for doc in memory */
#define dofCmdNewOpen	8	/* use NEW or OPEN dialog if no file */
#define dofReadOnly	16	/* open the file internal read-only */
#define dofSearchDot	32	/* used to open dot-causes search of several
					directories (specified, template, current)*/
#define dofNativeOnly   64      /* causes open to fail if file hasn't a fib */
#define dofNormalized   128     /* filename is already normalized */

#define dofBackground	(dofNoWindow|dofNoErrors)

/* GetDocSt Options */
#define gdsoNoUntitled	0x0001  /* return empty st instead of "Untitled" */

#define gdsoFullPath	0x0000	/* normal */
#define gdsoRelative  	0x0010
#define gdsoShortName 	0x0020
#define gdsoNotRelDot   0x0040  /* hack! */


/* T O O L B O X */
/* Hungarian names for Windows structures */

struct PT {
	union	{
		struct	{
			int xp;
			int yp;
			};
		struct  {
			int xl;
			int yl;
			};
		struct	{
			int xw;
			int yw;
			};
		};
	};

/* Rectangle  - with assorted flavors of units */
struct RC {
	union {
		struct {
			int	xpLeft;
			int	ypTop;
			int	xpRight;
			int	ypBottom;
			};
		struct {
			struct PT ptTopLeft;
			struct PT ptBottomRight;
			};
		struct {  /* window relative */
			int	xwLeft;
			int	ywTop;
			int	xwRight;
			int	ywBottom;
			};
		struct {  /* page relative */
			int	xlLeft;
			int	ylTop;
			int	xlRight;
			int	ylBottom;
			};
		struct {  /* window text area relative */
			int	xeLeft;
			int	yeTop;
			int	xeRight;
			int	yeBottom;
			};
		};
	};
#define cbRC (sizeof(struct RC))
#define cwRC (cbRC / sizeof(int))

/* Rectangle specified with dimensions */
struct DRC {
	union {
		struct {
			int	xp;
			int	yp;
			int	dxp;
			int	dyp;
			};
		struct {
			struct PT ptTopLeft;
			struct PT ptDxDy;
			};
		struct {  /* window relative */
			int	xw;
			int     yw;
			int	dxw;
			int	dyw;
			};
		struct {  /* page relative */
			int	xl;
			int     yl;
			int	dxl;
			int	dyl;
			};
		struct {  /* window text area relative */
			int	xe;
			int     ye;
			int	dxe;
			int	dye;
			};
		};
	};
#define cbDRC (sizeof(struct DRC))
#define cwDRC (cbDRC / sizeof(int))


/* FAppClose() parameter values.  NOTE: EL depends on these values! */
#define acQuerySave     0
#define acSaveNoQuery   1
#define acNoSave        2
/* not used by FAppClose */
#define acAutoSave      3
#define acSaveAll       4

/* MessageBox() flags for a save query. */
#define mbQuerySave (MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONQUESTION)


#include "cmb.h"

#define CallCmd(bcm)	    \
		{ FExecCmd(bcm); }
#define ChainCmd(bcm)	    \
		PostMessage(vhwndApp, WM_COMMAND, bcm, 2L)

#define ChainCmdCab(bcm)	\
		PostMessage(vhwndApp, WM_COMMAND, bcm, 1L)

#define BcmOfSz(sz)	BcmOfSt(StShared(sz))


/* Key Map Entry */
typedef struct _kme
	{
	int kc : 12;
	int kt : 3;
	union {
		int w;		/* generic */
		BCM bcm;	/* ktMacro */
		int ibst;	/* (not used) */
		int tmc;	/* (not used) */
		PFN pfn;	/* ktFunc */
		CHAR ** hst;	/* ktKeyMacro (not used) */
		int ch;		/* ktInsert */
		};
	} KME;

#define cwKME (sizeof(KME) / sizeof(int))


/* Key Map Flags */
#define kmfPlain	0	/* NOTICE: bit-order specific! */
#define kmfStopHere	1
#define kmfModal	2
#define kmfBeep 	4
#define kmfTranslate	8
#define kmfAppModal	16


/* Key MaP */
typedef struct _kmp
	{
	struct _kmp ** hkmpNext;
	union {
		int grpf;
		struct {
			int fStopHere : 1;
			int fModal : 1;
			int fBeep : 1;
			int fTranslate : 1;
			int fAppModal : 1;
			int : 11;
			};
		};
	int ikmeMac;
	int ikmeMax;
	KME rgkme[0 /*ikmeMax*/];
	} KMP;

#define cwKMP (sizeof(KMP) / sizeof(int))

#define cchMaxSzKey     32 /* for FKcToSz() buffers */



/* Timer Ids for our WM_TIMER messages */
#define tidFlash        0x7f01
#define tidInterrupt    0x0101


/* Document properties */

/* this structure is now stored in file in "table space."  fields can be
	added to the end w/o changing file format (or anything in the fib). */
/* WARNING : DOP is a field of DOD, if size is changed, make sure it is updated
	in structs.inc in both DOD and DOP */
struct DOP
	{		   /* add new fields to the end of structure,
				it is part of file! */
/* options */
	int	fFacingPages : 1;
	int	fWidowControl : 1;
	int	: 3;
	int	fpc : 2;
	int	fWide:1;  /* True == landscape. */
/* bits are right flush 2,1,0, set for each ihdt*Ftn that has its own ihdd */
	int	grpfIhdt : 8;

	int	fFtnRestart : 1;
	int	nFtn : 15;

	int	irmBar : 8;
	int	irmProps : 7;
	int	fRevMarking : 1;

	int     fBackup : 1;    /* make backup on save */
	int	fExactCWords : 1;
	int	fPagHidden : 1;
		int     fPagResults : 1;
		int     fLockAtn : 1;
		int     fMirrorMargins : 1;
		int     :10;

		int     fSpares:16;


/* measurements */
	uns	yaPage;
	uns	xaPage;
	int	dyaTop;
	uns	dxaLeft;
	int	dyaBottom;
	uns	dxaRight;
	uns	dxaGutter;
	uns	dxaTab;
	uns	wSpare;
	uns	dxaHotZ;
		uns     rgwSpare[2];

/* document summary info */
	struct	DTTM	dttmCreated;
	struct	DTTM	dttmRevised;
	struct	DTTM	dttmLastPrint;
	int	nRevision;
	long	tmEdited;
	long	cWords;
	long	cCh;
	int	cPg;
		int     rgwSpareDocSum[2];

	};
/* WARNING : DOP is a field of DOD, if size is changed, make sure it is updated
	in structs.inc in both DOD and DOP */

#define cwDOP (sizeof (struct DOP) / sizeof (int))
#define cbDOP (sizeof (struct DOP))

#define ibstMaxFileCache	4
#define ichUsrNameMax	50

/* change this when changing PREF structure to invalidate opus.ini files! */
#define nPrefVerCur	18

/*  ***********   WARNING!!!!!	  *******************************
	PREF is also in structs.inc. If you change here, change there
	too!!!!
		**************************************************************
*/


typedef union  {
	struct {
		int	   fvisiTabs:1;
		int	   fvisiSpaces:1;
		int	   fvisiParaMarks:1;
		int	   fvisiFtnRefMarks:1;
		int	   fvisiCondHyphens:1;
		int	   fvisiShowAll:1;
		int	   fNoShowPictures:1;
		int	   fSeeHidden:1;
		int	   flm: 2;
		int	   : 1;
		/* These bits are for fields: if true, show results
			for that field group, if false, show instructions */
		int	   grpfShowResults : 2;
			int	   fDrawTableDrs: 1;
			int	   fDrawPageDrs: 1;
		/*  if on, ignore field's fDiffer flag */
		int	   fForceField : 1;
		};
	int	w;
	} GRPFVISI;

struct PREF
	{
	int		wSpare;
	int		ut: 4;
	int		: 2;
	int		fRibbon:1;
	int		fStatLine:1;
		/* used in customize dialog */
	int		fPromptSI: 1;
	int		fBkgrndPag:1;
	int		fAutoDelete:1;

	int		fShortMenus : 1;
	int		fRuler : 1;
	int		fHorzScrollBar:1;
	int		fVertScrollBar:1;
	int		fPageView : 1; /* if true, New/Open default in page view */

/* Following field is copied into every ww */
	GRPFVISI	grpfvisi;

	int	   fDisplayAsPrint: 1 ;
	int	   fSplAutoSugg: 1 ; 
	int	   fShowAllMacros: 1 ; 
	int	   : 1 ; /* spare */
		/* used in customize dialog */
	int	   iAS: 2; /* Auto Save frequency index. */

	int	   fDraftView: 1;	
	int	   fZoomApp: 1;
	int	   fZoomMwd: 1;
	int	   fEllip: 1;
	int	   fShowF: 1;
	int	   fPrDirty: 1;    
	int	   fSpare: 3;
	int	   fPrvwTwo : 1;  /* one or two pages in print preview */
	unsigned   dxaStyWnd;  /* width of style name area */

/* -------- END of SHORT PREF structure ---------------------------------*/
/* Any fields below this line will not be cleared when starting tutorial */
/* Add new fields above this line unless tutorial needs to preserve them */
/* See FReadUserState in initwin.c, and make sure any new fields are set */
/* properly for the tutorial.                                            */

	CHAR	   stUsrInitl [ichUsrInitlMax];
	CHAR	   stUsrName [ichUsrNameMax];
	int	   rgwSpare [6];
	};

#define cbPREF (sizeof (struct PREF))
#define cwPREF (sizeof (struct PREF) / sizeof (int))
#define cbPREFShort (offset(PREF, stUsrInitl))

/* Help Context Conversions */

#define CxtFromIdd(idd)      ((idd) + 0x5000)
#define CxtFromHid(hid)      (CxtFromIdd(hid))
#define CxtFromEid(eid)      ((eid) + 0x6000)
/* cxts between 7000 and 7FFF are reserved for mst's (see message.h) */
/* #define CxtFromIdpmt(idpmt)  ((idpmt) + 0x5000)     OBSOLETE */
/* #define CxtFromMst(mst)      ((mst) + 0xB000)       OBSOLETE */

void SetOurKeyState();
NATIVE CP FAR *LprgcpForPlc();
NATIVE DeleteFnHashEntries();

/* Some useful composite message box codes */

#define MB_MESSAGE	  (MB_OK | MB_APPLMODAL | MB_ICONASTERISK)
#define MB_YESNOQUESTION  (MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION)
#define MB_ERROR	  (MB_OK | MB_ICONEXCLAMATION)
#define MB_TROUBLE	  (MB_OK | MB_SYSTEMMODAL | MB_ICONHAND)
#define MB_DEFYESQUESTION (MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONQUESTION)
#define MB_DEFNOQUESTION  (MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_APPLMODAL | MB_ICONQUESTION)


NATIVE EnsureFocusInPane();
CP CpFirstDl();
FC FcMacFn();
#ifdef OBSOLETE
LONG UsecTickCount();
#endif
struct CA *PcaField();
struct CA *PcaFieldResult();
#ifdef DEBUG
HANDNATIVE CP	   C_CpSearchSz();
HANDNATIVE CP	   C_CpSearchSzBackward();
HANDNATIVE CP	   C_DcpSkipFieldChPflcd();
HANDNATIVE int	   C_IfldInsertDocCp();
HANDNATIVE CP	   C_CpVisibleCpField();
HANDNATIVE PN	   C_PnFromPlcbteFc();
HANDNATIVE CHAR HUGE *C_HpchFromFc();
HANDNATIVE CHAR HUGE *C_HpchGetPn();
HANDNATIVE struct DR *C_PdrFetch();
HANDNATIVE struct DR *C_PdrFetchAndFree();
HANDNATIVE struct DR *C_PdrFreeAndFetch();
HANDNATIVE struct PT  C_PtOrigin();
HANDNATIVE CP	   C_CpFormatFrom();
HANDNATIVE CP	   C_CpVisibleBackCpField();
HANDNATIVE FC	   C_FcAppendRgchToFn();
HANDNATIVE char   *C_PchSzRtfMove();
HANDNATIVE struct FCE *C_PfceLruGet();
#endif /* DEBUG */

typedef double TBNUM; /* NUM type for the toolbox */


/* MAC WORD COMPATIBILITY DEFINITIONS */
#define FDrawTableDrsPref  (vpref.grpfvisi.fDrawTableDrs || vpref.grpfvisi.fvisiShowAll)
#define FDrawPageDrsPref   (vpref.grpfvisi.fDrawPageDrs || vpref.grpfvisi.fvisiShowAll)
#define FDrawTableDrsWw(ww)  (PwwdWw(ww)->grpfvisi.fDrawTableDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)
#define FDrawPageDrsWw(ww)   (PwwdWw(ww)->grpfvisi.fDrawPageDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)

#define SectRect(prcSrc1, prcSrc2, prcDest) (IntersectRect((LPRECT)prcDest, (LPRECT)prcSrc1, (LPRECT)prcSrc2))

#define	PwwdSetWw(ww, cptSame)	PwwdWw(ww)
#define FNinchForSprmType(sprm)	fFalse

/* note: ChUpper, QszUpper now functions  */
#define ChLower(ch)	    ((CHAR)AnsiLower((LPSTR)(unsigned long)ch))
#define QszLower(pch)	    (AnsiLower((LPSTR)pch))
#define SaveWds(pwds)
#define RestoreWds(pwds)
#define CacheLlc(cp)
#define FcFromPo(po)  0
#define PoFromFc(fc)  0
#define PenNormal()
#define DrawLlcStyle1(stc)
#define SetCrs(crs)		OurSetCursor(crs)
#define AnimateLocalRect(ww, prcBegin, prcAnim)	\
		AnimateRect(PwwdWw(ww)->hdc, prcBegin, prcAnim)
/* For Mac it should be EraseRect(_prc). */
#define EraseRc(ww, _prc)	\
	PatBltRc(PwwdWw(ww)->hdc, (_prc), vsci.ropErase)
#define SetLlcPercentCp(cpFirst, cpCur, cpLim)

#define TrashRuler()    (selCur.fUpdateRuler = fTrue)
#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))

#define ichMaxCmdLine   128      /* Longest command line accepted */
#define ipchArgMax      10       /* Most command line arguments accepted */

#define FPtInRect(pt, prc)	PtInRect((LPRECT)prc, pt)

#define MwFromHwndMw(hwnd) ((hwnd)==NULL ? mwNil : GetWindowWord((hwnd),IDWMW))
#define WwFromHwnd(hwnd) (GetWindowWord((hwnd),IDWWW))

#define CchIntToPpch(n,ppch) (CchLongToPpch((long)(int)(n),ppch))
#define CchUnsToPpch(n,ppch) (CchLongToPpch((long)(uns)(n),ppch))


	/* rounding values used in CchExpZa */
#define grpfRndRnd (0)
#define grpfRndLow (1)
#define grpfRndHigh (2)

#define CchExpZa(ppch, za, ut, cchMax, fUnit) (CchExpZaRnd(ppch, za, ut, cchMax, fUnit, grpfRndRnd))

/* max allowable numeric digits for formatting */
#define cchFormatNumMax 100

extern int vfNoInval;
#define DisableInval() (vfNoInval++)
#define EnableInval() (AssertDo(vfNoInval--))

extern struct CA caSect;
#define CacheSect(doc,cp) (FInCa((doc),(cp),(&caSect))?0:CacheSectProc((doc),(cp)))

NATIVE int IpcdSplit();
NATIVE FOpenPlc();
NATIVE FMsgPresent();

/* Swap Area Size */
#define sasMin      8
#define sasOK       3
#define sasFull     0

extern int vcShrinkSwapArea;
#define ShrinkSwapArea()    {vcShrinkSwapArea++; OurSetSas(sasMin);}
#define GrowSwapArea()      if (!--vcShrinkSwapArea) OurSetSas(sasFull);

/* System Memory Types */
extern int smtMemory;
#define smtReal     0 /* 640K system */
#define smtLIM      1 /* LIM 3.2+ system */
#define smtProtect  2 /* Protect mode */
HANDLE OurGlobalAlloc(WORD, DWORD);
HANDLE OurGlobalReAlloc(HANDLE, DWORD, WORD);




