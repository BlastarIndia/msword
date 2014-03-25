/* D E F I N E S */

#ifndef PC_EXPORT
#define PC_EXPORT
#endif

#define lnnNil  (-1)
#define ilrNil  (-1)
#define ylLarge ypLarge

#define SetIMacPllr(plbs, i)    ((*(plbs)->hpllr)->ilrMac = i)
#define IMacPllr(plbs)          (*(plbs)->hpllr)->ilrMac

/* layout break codes */

#define lbcNil                  0       /* paragraph fit in full, no break necessary */
#define lbcPageBreakBefore      1       /* broke in front of para with this prop */
#define lbcEndOfPage            2       /* broke after para with chSect last char
						but not a section */
#define lbcEndOfSection         3       /* broke after last para of section */
#define lbcEndOfDoc             4       /* broke after last para in doc */
#define lbcYlLim                5       /* broke because ywLim would have been
						exceeded by further progress */
#define lbcEndOfColumn          6       /* broke because chSect encountered in
						multi-column section */
#define lbcAbort                7       /* ran out of memory or user abort */


#ifdef MAC
/* MacWord uses yl expressed in points, no danger of overflow */
typedef int         YLL;
#define yllLarge    ylLarge
#else
/* PCWord uses yl expressed in twips, WinWord uses pixels. Overflow is a 
	potential problem. */	
typedef	long        YLL;
#define yllLarge    0x7FFFFFFF
#endif /* !MAC */

#ifdef PCWORD
#define HANDNATIVE
#endif /* PCWORD */


#ifndef PCWORD
#define U0
#define U1
#define U2
#define U3
#define U4
#define U5
#define U6
#define U7
#define U8
#define U9

#define S1
#define S2
#define S3
#define S4
#define S5
#define S6
#define S7
#define S8
#define S9

typedef int BF;

#endif


/* layout rectangle */
struct LR
{
	union {
		struct DRC drcl;
		struct {
#ifdef MAC
			int yl;         /* y position on page */
			int xl;         /* x position on page corresponding to galley xa = 0 */
			int dyl;        /* height */
			int dxl;        /* width */
#else
			int xl;
			int yl;
			int dxl;
			int dyl;
#endif
			} S1;
		} U1;
	union {
		struct CA ca;
		struct {
/* NOTE: these are not raw CPs -- use the corresponding cl to find raw cp */
			CP cp;   /* block start */
			CP cpLim;
/* document distinguishes text, footnote, header.  Note: doc < 0 means
	format special char -doc, e.g. -chPage */
			int doc;
			} S2;
		} U2;
	int     clFirst;    /* corresponding to cp, cpLim */
	int     clLim;
	uns     lnn;        /* first line of LR, -1 when sect does not have Lnn prop) */
	int     ihdt;       /* only when PdodDoc(doc)->fHdr */
				/* PCWORD - ihdt is used for all LRs
				should be set to ihdtNil for normal text */
	BF      fGraphics : 1;  /* fTrue iff LR contains RevBar, brcp, or picture */
				/* PCWORD - fTrue iff LR has graphics inclusion file */
	BF      fForceFirstRow : 1;	/* The table row beginning in this LR should
									be treated as a first row wrt borders. */
	BF	     fFixedXl : 1;	/* lr.xl is fixed (for column apos) */
	BF      fColAfterAbs : 1;	/* column break encountered after this apo */
	BF      fPageAfterAbs : 1;	/* page break encountered after this apo */
	BF      fInline : 1;	/* in-line absolute object */
	BF      fConstrainLeft : 1;     /* constrained by abs on left */
	BF      fConstrainRight : 1;    /* on right */
	BF      fConstrainTop : 1;	/* on top */
	BF      fChain : 1;    /* this LR is part of a chain */
	BF		fSpaceBefore : 1;	/* pap.dyaBefore must be ignored */
	BF : 1;
	BF      tSoftBottom : 4; /* if tPos, ok to go past lim on last line */
	char    lrs;        /* layout rectangle status */
	char    lrk;        /* type of layout rectangle */
	int     ilrNextChain;   /* next lr in horizontal chain */
#ifdef WIN
	CP      cpMacCounted;  /* counted up to but excluding this cp. */
#endif  /* WIN */
};
#define cwLR    (sizeof(struct LR) / sizeof(int))

#define lrkNormal       0
#define lrkAbs          1       /* absolutely positioned */
#define lrkAbsHit       2       /* width affected by abs pos */
#define lrkSxs		3       /* containes or is affected by side-by-side */

#define lrsNormal       0
#define lrsFrozen       1       /* no more text allowed */
#define lrsIgnore       2       /* does not contain anything useful */

#ifdef PCWORD
#define lrsPgn			3		/* holds page number */
#endif /* PCWORD */

/* LR plex
	some caveats: 1. don't count on the order of the LRs returned from LbcFormatPage
			2. yw + dyw may not match the yw of any LR (even in the
			middle of a page)
*/
struct PLLR
{
	uns ilrMac;
	uns ilrMax;
	int cb;
	int brglr;
#ifndef PCWORD
	int fExternal;
#endif
	struct LR rglr[1];
} ;

/* height structure for plclnh */
struct LNH
	{
	int fSplatOnly:1;	/* line is only a splat */
	int fDypBeforeAdded:1; /* A non-zero dypBefore was added into the line height */
	int	: 6;
	char	chBreak;
	YLL	yll;    /* this height may span more than one page, so it
							must be stored as a long for PCWord and WinWord */
	};

/* header layout, precalculated for page */
struct HDT
{
#ifndef PCWORD
	int ihdd;               /* which header in subdoc */
#endif /* !PCWORD */
	int yl;                 /* top of first LR */
	int dyl;                /* total height */
	struct PLLR **hpllr;    /* (if dyl != 0) */
} ;
#define cwHDT 	(sizeof(struct HDT) / sizeof(int))

/* footnote reference layout
	plcfrl may be "empty". cpMac = cpLim of block. yl's are between lines
*/
struct FRL
{
	int ylReject;   /* cleave point if reference is not to be included */
	int ylAccept;   /* cleave point if reference is to be included */
	int ifnd;       /* footnote index */
	char fNormal;   /* ylAccept/ylReject are approximate */
	char chSpare;  /*  spare */
} ;
#define cbFRL sizeof(struct FRL)
#define cwFRL (cbFRL / sizeof(int))

/* format lines state
	a cache holding the breakdown of a paragraph into lines
*/
struct FLS
{
	struct CA ca;           /* paragraph for which this is valid */
	int ww;
	struct PLC **hplclnh;   /* holds cp, dyl pairs, dyl's address bottoms
					of lines, 0 being the top of the para (before
					lead). cp's are starts of the lines.
					plc.iylMac may be < clMac because the scan
					may have been stopped */
	struct PHE phe;         /* height structure */
	int clMac;              /* # of lines in para or clNil if unknown.
					calculated with phe */
	char chBreak;           /* if fEndBreak, this is the guilty party */
	char fVolatile;         /* one or more lines have volatile text */
	char fFirstColumn;      /* first column of page */
	char fFirstPara;        /* first para of column */
	char fPageBreak;        /* plclnh contains a page break */
	char fBreakLast;        /* last line in plclnh caused a splat */
	char fEndBreak;         /* ClFormatLines stopped due to splat */
	char fOutline;          /* formatting in outline */
	int dxl;                /* width for which lines are valid */
};

/* layout state: parallel pointers into the stream, into y space and into rglr.
	Note: the LBS contains the state required to format the NEXT page, except
		for the following, which describe the page just formatted:
		hpllr
		bkc
		fEmptyPage, fCpOnPage
		ihdtTop, ihdtBottom
		lbc
		vfmtss.pgn is the real page number of the page just formatted
*/
struct LBS
{
	int     doc;
	CP      cp;             /* beginning of a line */
	int     cl;             /* line index starting at cp */
	int     yl;             /* of the top of the line (cp,cl) */
	uns     pgn;            /* pgn for (cp,cl) */
	int     ilrCur;         /* ilr being filled */
	struct PLLR **hpllr;
	int     ylColumn;       /* top of current column */
	int     ylMaxBlock;     /* absolute bottom of block */
	int     dylOverlap;     /* amount to ignore between lr's */
	int	dylUsedCol;	/* amount of space used in column */
	int     xl;
	int     dxlColumn;      /* width of column */
	int     ww;             /* current window for doc */
	uns     lnn;            /* defined only if sect has Lnn prop */
	int     ihdt;           /* ihdt being filled */
	int	dcpDepend;	/* amount past plbs->cp (ignore cl) which 
								was considered */
/* input state flags */
	char    fRight;         /* right/left page for facing pages */
	char    fPgnRestart;    /* restart with page 1 again */
	char    fFacingPages;   /* reflected hdrs/ftrs or margins, or gutter */
	char    fOutline;       /* what vfli.fOutline should be set to */
	char    fSimpleLr;		/* one para per LR */
	char	fPostScript;	/* collect PostScript paras */
/* output state flags */
	union
		{
		/* lbsText output flags */
		struct
			{
			char    lbc;            /* break code for this page */
			char    bkc;            /* section break code */
			char    ihdtTop;        /* ihdts used for this page */
			char    ihdtBottom;
			char    fEmptyPage;     /* pgn/break forced an empty page */
			char    fCpOnPage;      /* indicates main text placed on page */
			} S1;
		/* lbsFtn output flags */
		struct
			{
			char 	fContinue;	/* footnote not complete */
			char	fSpare;/*  spare */
			int	wSpare1;/*  spare */
			int	wSpare2;/*  spare */
			} S2;
		} U1;
/* internal state flags */
	char    fFirstColumn;   /* indicates first column of page */
	char    fFirstPara;     /* indicates first para of column */
	char    fEndnotes;      /* indicates endnotes must be handled */
	char    fNoBalance;     /* indicates balancing is not allowed */
	char    fAbsPresent;    /* abs. objects in pllr */
	char    fOnLbsStack;	/* has a copy on lbs stack */
/* chaining; min ilr for chain start is 1 because abs obj must precede */
	int     ilrFirstChain;  /* first lr in chain */
	int     ilrCurChain;    /* current lr in chain */
	int     dylChain;       /* height of one line of chain */
	int     dylBaseChain;   /* baseline for chain */
/* table state */
/* NOTE: because dylBelowTable and fPrevTable are not LR fields, an absolute
	object between two rows of a table will cause a gap */
	int     dylBelowTable;  /* dylBelow from previous table row */
	char    fPrevTable;     /* previous para was table row */
	char	fBkRepagStop;	/* used for background repag to force redisplay */
#ifdef WIN
	struct PLLNBC **hpllnbc; /* defined iff section has vert. line on */
#endif
/* used to create an average line height for balancing columns */
	int	clTotal;	/* number of lines in column */
	YLL	dyllTotal;	/* height of those lines */
	int	ylMaxLr;
	int	ylMaxColumn;	/* used to detect APO at bottom of column */

	int	dylBeforeUsed;	/* had to back up the LR by this much */
} ;
#define cbLBS   sizeof(struct LBS)
#define cwLBS   (cbLBS / sizeof(int))


struct PLLBS
{
	uns ilbsMac;
	uns ilbsMax;
	int cb;
	int brglbs;
	struct LBS rglbs[1];
} ;


/* repagination limits - conditions under which UpdateHplcpgd should stop.
	ipgd/pgn are both inputs and outputs */
struct RPL
	{
	int ised;
	uns pgn;
	int ipgd;       /* input/output */
	CP cp;          /* this is a Last, not a Lim */
#ifdef MAC
	int crs;		/* cursor id to use */
#endif
	};
#define cwRPL   (sizeof(struct RPL) / sizeof(int))


#ifdef WIN

/* Column Vertical Line */
struct LNBC
	{
	int xl;
	int yl;
	int dyl;
	};
#define cwLNBC (sizeof(struct LNBC) / sizeof(int))

struct PLLNBC
	{
	uns ilnbcMac;
	uns ilnbcMax;
	int cb;
	int brglr;
	int fExternal;
	struct LNBC rglnbc[1];
	};

/* Vertical Alignment stuff */
struct ALC
	{
	int clr;        /* # of LRs in column */
	int ilrFirst;   /* index into pllr of first LR of column */
	int lbc;        /* lbc for column */
	};
#define cwALC (sizeof(struct ALC) / sizeof(int))

struct ALG
	{
	int ylTop;            /* where group of columns starts on the page */
	int dylMac;           /* height of tallest column in alc */
	int dylAvail;         /* vertical space available for columns */
	int vjc;              /* vertical alignment code for alg */
	int calc;             /* # of columns in rgalc */
	struct ALC rgalc[1];  /* info for columns in alg */
	};
#define cwALG (sizeof(struct ALG) / sizeof(int))

#endif


CP CpFromCpCl(), CpFromIpgd();

#define clMaxLbs		400	/* limit on cl's stored in lbs */

/****************************/
/* system-dependent defines */
/****************************/

/****************************/
/* MAC */
/****************************/
#ifdef MAC
#define IfMacElse(a, b)        	(a)
#define IfWinElse(a, b)         (b)
#define IfPCWordElse(a, b)      (b)
#define NATIVE                  native
#define dxaAbsMin               1
#define dyaAbsMin               1
#define YlFromYa(ya)            ((ya) / dyaPoint)
#define XlFromXa(xa)            (NMultDiv(xa, vfli.dxsInch, dxaInch)) 
#define XaFromXl(xl)            (NMultDiv(xl, dxaInch, vfli.dxsInch))
#define ddylRelaxDef            15      /* amount by which to relax when balancing */
#define FormatLineDxaL(ww, doc, cp, dxa) FormatLineDxa(ww, doc, cp, dxa)
#define SetLayoutAbort()        SetJmp(&venvLayout)
#define FSxsLrk(lrk)		(lrk == lrkSxs)

#define FAbsPapM(doc, ppap)	FAbsPap(doc, ppap)
#define FEmerg1(merr)           (merr.hrgwEmerg1 == hNil)
#define CColSep(sep)            (sep.ccolM1 + 1)
#define CColM1Sep(sep)          (sep.ccolM1)
#define FTitlePageSep(sep)      (sep.fTitlePage)
#define FEndnoteSep(sep)        (sep.fEndnote)
#define FPgnRestartSep(sep)     (sep.fPgnRestart)
#define FPageBreakBeforePap(pap)  (pap.fPageBreakBefore)
#define FFacingPagesPdod(pdod)	(pdod->dop.fFacingPages)
#define FFtnDoc(doc)            (PdodDoc(doc)->fFtn)
#define DocFtn(doc)             (PdodDoc(doc)->docFtn)
#define FFtnsPdod(pdod)         (pdod->hplcfrd != hNil && pdod->docFtn != docNil)
#define FInTablePap(pap)        (pap.fInTable)
#define FKeepPap(pap)           (pap.fKeep)
#define FLnnSep(sep)            (sep.nLnnMod)
#define FLnnPap(pap)            (!pap.fNoLnn)
#define DypBaseFli(fli)         (fli.dypBase)

#define CpMacDocEditL(doc, cp)  CpMacDocEdit(doc)
#define CpMacDocEditPlbs(plbs)  CpMacDocEdit(plbs->doc)
#define CpMacDocPlbs(plbs)      CpMacDoc(plbs->doc)
#define DxlTooNarrow            ((int)vfli.dxsInch)			

#ifdef JR
#define FWidowPdod(pdod)        (fFalse)
#else
#define FWidowPdod(pdod)        (pdod->dop.fWidowControl)
#endif /* !JR */

#ifdef DEBUG
#define DEBUGMAC
#endif /* DEBUG */

#define StartProfile()	
#define StopProfile()

#define LRP			struct LR *
#define LrpInPl(hpllr, ilr)	((struct LR *)PInPl(hpllr, ilr))
#define bltLrp(lrp1, lrp2, cb)	blt(lrp1, lrp2, cb / sizeof(int))
#define HpllrInit(cb, ilrMac)	HplInit(cb, ilrMac)
#endif /*MAC*/


/****************************/
/* WIN */
/****************************/
#ifdef WIN
NATIVE int FAllocAlg();
NATIVE struct ALG *PalgFromHgrpalgBalg();
NATIVE struct ALG *PalgNextFromHgrpalgPbalg();

#define IfMacElse(a, b)        	(b)
#define IfWinElse(a, b)			(a)
#define IfPCWordElse(a, b)		(b)

#define YlFromYa(ya)           (NMultDiv(ya, vfti.dypInch, czaInch))
#define XlFromXa(xa)           (NMultDiv(xa, vfti.dxpInch, czaInch)) 
#define YaFromYl(yl)           (NMultDiv(yl, czaInch, vfti.dypInch))
#define XaFromXl(xl)           (NMultDiv(xl, czaInch, vfti.dxpInch))

#define dxaAbsMin               1
#define dyaAbsMin               1
#define FAbsPapM(doc, ppap)	FAbsPap(doc, ppap)
#define ddylRelaxDef            NMultDiv(240, vfti.dypInch, czaInch)
#define FormatLineDxaL(ww, doc, cp, dxa)	FormatLineDxa(ww, doc, cp, dxa)
#define SetLayoutAbort()        SetJmp(&venvLayout)
#define FEmerg1(merr)           (merr.hrgwEmerg1 == hNil)
#define CColSep(sep)            (sep.ccolM1 + 1)
#define CColM1Sep(sep)          (sep.ccolM1)
#define FTitlePageSep(sep)      (sep.fTitlePage)
#define FEndnoteSep(sep)        (sep.fEndnote)
#define FPgnRestartSep(sep)     (sep.fPgnRestart)
#define FWidowPdod(pdod)        (pdod->dop.fWidowControl)
#define FPageBreakBeforePap(pap) (pap.fPageBreakBefore)
#define FFacingPagesPdod(pdod)	(pdod->dop.fFacingPages)
#define FFtnDoc(doc)            (PdodDoc(doc)->fFtn)
#define DocFtn(doc)             (PdodDoc(doc)->docFtn)
#define FFtnsPdod(pdod)         (pdod->hplcfrd != hNil && pdod->docFtn!=docNil)
#define FSxsLrk(lrk)		fFalse

#define FInTablePap(pap)        (pap.fInTable)
#define FKeepPap(pap)           (pap.fKeep)
#define FLnnSep(sep)            (sep.nLnnMod)
#define FLnnPap(pap)            (!pap.fNoLnn)
#define DypBaseFli(fli)         (fli.dypBase)

#define CpMacDocEditL(doc, cp)  CpMacDocEdit(doc)
#define CpMacDocEditPlbs(plbs)  CpMacDocEdit(plbs->doc)
#define CpMacDocPlbs(plbs)      CpMacDoc(plbs->doc)
#define DxlTooNarrow            ((int)vfli.dxuInch)			

#define StartProfile()	
#define StopProfile()

#ifdef DEBUG
#define DEBUGWIN
#endif /* DEBUG */

#define LRP			struct LR HUGE *
#define LrpInPl(hpllr, ilr)	((struct LR HUGE *)HpInPl(hpllr, ilr))
#define bltLrp(lrp1, lrp2, cb)	bltbh(lrp1, lrp2, cb)
#define HpllrInit(cb, ilrMac)	HplInit2(cb, cbPLBase, ilrMac, fTrue /* fExternal */)
#endif /* WIN */

/****************************/
/* PC */
/****************************/
#ifdef PCWORD
/*Plan of action: PCWORD code is considerably more compatible with
	MAC print code than with WIN print code.  So to start with, I
	will try to take the MAC portion of this module and see what happens -davidb*/

#define IfMacElse(a, b)        	(b) 
#define IfWinElse(a, b)         (b)
#define IfPCWordElse(a, b)      (a)
#define Mac(a)
#define fWin					fFalse
#define DEBUGORNOTWIN

#define CacheSectL(doc, cp, fOutline) CacheSectBody(doc, cp)

extern int vcyInch, vcxInch;
#define YlFromYa(ya)            YpFromYa(ya) 
#define XlFromXa(xa)            (MultDiv(xa, vcxInch, czaInch)) 
#define XaFromXl(xl)            (MultDiv(xl, czaInch, vcxInch))
#define ddylRelaxDef            (MultDiv(czaLine, vcyInch, czaInch))
#define dxaColumnWidth          dxaText

#define FAbsPapM(doc, ppap)	FAbsPap(ppap)
#define FEmerg1(merr)           (merr.fError || merr.fMemAlert)
#define WinMac(win, mac)        (mac)
#define dxaInch                 czaInch
#define dxaAbsMin               0
#define dyaAbsMin               0
#ifdef CC
extern int NEAR SetJmpLayout();
#define SetLayoutAbort()        SetJmpLayout(&venvLayout)
#else
#define SetLayoutAbort()        SetJmp(&venvLayout)
#endif
#define PdodMother(doc)         PdodDoc(doc)
#define CColSep(sep)            (sep.cColumns)
#define CColM1Sep(sep)          (sep.cColumns - 1)
#define FTitlePageSep(sep)      (fTrue)
#define FEndnoteSep(sep)        (fTrue)
#define FPgnRestartSep(sep)     ((int)sep.pgnStart != iNil)
#define FWidowPdod(pdod)        (vprsu.fWidowControl)
#define FSxsLrk(lrk)		(lrk == lrkSxs)

#define FPageBreakBeforePap(pap) (fFalse)
#define DxaAllColumnsSep(sep)	(sep.cColumns * (sep.dxaText + sep.dxaGutter)\
									- sep.dxaGutter)
#define FFacingPagesPdod(pdod)	(fTrue)
#define FFtnDoc(doc)			(XXX) /* not supported for PCWORD */
#define DocFtn(doc)             (HplcfrdGet(doc) == 0 ? docNil : doc)
#define FInTablePap(pap)        (fFalse)
#define FInTableVPapFetch(doc, cp) (fFalse)
#define FKeepPap(pap)           (pap.fKeep || pap.btc == btcBox)
#define FLnnSep(sep)            (sep.fLnn)
#define FLnnPap(pap)            (fTrue)
#define DypBaseFli(fli)         (0)
#define DocMother(doc)          (doc)
#define FFtnsPdod(pdod)         (pdod->hplcfnd != hNil)
#define CpFirstRef(doc, cp, pcp, edc) CpFirstFtn(doc, cp, pcp, fFalse)

#define CpMacDocEditL(doc, cp)  CpMacSubDoc(doc, cp)
#define CpMacDocEditPlbs(plbs)  CpMacSubDoc(plbs->doc, plbs->cp)
#define CpMacDocPlbs(plbs)      CpMacSubDoc(plbs->doc, plbs->cp)
#define CpRefFromCpSub(doc, cp, edc) CpRefFromCpFtn(doc, cp)
#define DxlTooNarrow            ((int)(vcxInch >> 1))			
#define FNilHdt(ihdt)           (vrghdt[ihdt].hpllr == hNil ||\
									IMacPlc(vrghdt[ihdt].hpllr) == 0)

#define FAbortLayout(fOutline)	FAbortRequest()
#define FreePhpl(phpl)          FreePh(phpl)

#ifdef DEBUG
#define DEBUGPC
#endif /* DEBUG */

#define LRP			struct LR *
#define LrpInPl(hpllr, ilr)	((struct LR *)PInPl(hpllr, ilr))
#define bltLrp(lrp1, lrp2, cb)	bltbyte(lrp1, lrp2, cb)
#define HpllrInit(cb, ilrMac)	HplInit(cb, ilrMac)
#endif /* PCWORD */


/* layout.c */
#ifndef WIN
NATIVE int LbcFormatColumn();
NATIVE int LbcFormatBlock();
NATIVE int LbcFormatPara();
#endif /* !WIN */

/* layout2.c */
#ifndef WIN
NATIVE int ClFormatLines();
NATIVE FillLinesUntil();
NATIVE int FAssignLr();
NATIVE int FWidowControl();
NATIVE int IfrdGatherFtnRef();
NATIVE int FGetFtnBreak();
NATIVE CopyHdtLrs();
NATIVE CopyLrs();
NATIVE ReplaceInPllr();
#endif /* !WIN */
#ifdef MAC
NATIVE CacheParaL();
NATIVE CacheSectL();
#endif /* MAC */
#ifndef WIN
NATIVE PushLbs();
NATIVE int PopLbs();
NATIVE CopyLbs();
NATIVE UnstackLbs();
#endif /* !WIN */
#ifdef MAC
NATIVE FAbortLayout();
#endif /* MAC */

/* layoutap.c */
#ifndef WIN
NATIVE SetAbsLr();
NATIVE int CheckAbsHit();
NATIVE int LbcPositionAbs();
NATIVE CheckAbsCollisions();
NATIVE CopyAbsLrs();
NATIVE FSectDrc();
NATIVE int LbcFormatChain();
NATIVE DxlFromTable();
#endif /* !WIN */

#define	irtnAssignLr   0
#define	irtnFormPara   1
#define	irtnFormChain  2

/* flags for FPrepareDocFieldsForPrint (added together) */
#define pfpNormal		0
#define pfpPrinting		1
#define pfpRefreshAll	2

