/* D I S P . H */
/* note: you don't want these too big, so 1/2 of max positive value: */
#define xpMaxAll       0x4000 /* used for invalidation */
#define ypMaxAll       0x4000 /* used for invalidation */

/* size of lines for dlMax estimation purposes */
#define dypAveInit      12

/* number of quanta in elevator control */
#define dqMax           256
/* number of quanta to save in Pgv elevator control */
#define dqPgvwElevSpace  11

/* structure represents a single line on the display */
struct EDL
	{
	union {
		int grpfEdl;
		struct {
			unsigned char      dcpDepend;
			int     dlk : 3;
			int     fDirty : 1;
			int     fHasBitmap : 1; /* used in scc only */
			int     fTableDirty : 1;
			int     fColorMFP: 1;   /* color metafile (WIN) */
			int     fNeedEnhance: 1;  /* postponed display (WIN) */
		};
		struct {
			int     : 8;
/* set iff edl is the endmark. Other fields are then:
cp = cpMac of doc, dcpMac = 0 */
			int     fEnd : 1;
			int     fEndPage : 1;
			int     fEndDr : 1;
			int     : 1;
			/* Used for rev marking in tables. This is pretty kludgy 
			/* in that it overloads fHasBitmap which is never used for
			/* inner EDLs
			/**/
			int     fRMark : 1; 
			int     : 3;
		};
	};
	struct {
		struct PLDR **hpldr;  /* used for tables */
		union
			{
			/* rectangle containing dl */
			struct DRC      drcp;
			struct {
#ifdef WIN
				int             xpLeft;
				int             ypTop; /* top of dl */
				int             dxp;
				int             dyp; /* height of dl */
#else
				int             ypTop; /* top of dl */
				int             xpLeft;
				int             dyp; /* height of dl */
				int             dxp;
#endif
				};
			};
		};
/* representing cpMac */
	CP              dcp;
	};

/* largest value which can be dcpDepend + 1 */
#define dcpDependMax 256

#define cchEDL  (sizeof (struct EDL))
#define cwEDL   (cchEDL / sizeof(int))

/* dlk values and flags */
#ifdef MAC
#define dlkEnd          4
#define dlkEndPage      2
#define dlkEndDr        1
#define dlkNil          0
#else
#define dlkEnd          1
#define dlkEndPage      2
#define dlkEndDr        4
#define dlkNil          0
#endif /* MAC */

/* params to DrawEndmark */
#define emkBlank    0
#define emkEndmark  1
#define emkSplat    2


/* WARNING!! - Changes in this structure should be duplicated in
	the SCC structure */
struct PLCEDL
	{
	int     dlMac;
	int     dlMax;
	int     cb;
	int     icpAdjust;
	CP      dcpAdjust;
	int     icpHint;
	int	fExternal : 1;
	int	fExtBase : 1;
	int	: 14;
	union   {
		HQ      hqplce;   /* when fExternal true */
		CP      rgcp[1];            /* when fExternal false */
		/*struct EDL   rgedl[]*/
		};
	};

/* display rectangle structure (organizer on pages, container of dl's) */

/* value means that flow is terminating */
#define idrNil  (-1)
/* value means that flow is continuing to a next page */
#define idrNextPage (-2)

struct DR
	{
	union
		{
		struct DRC drcl;
		struct {
#ifdef WIN
			int     xl;
			int     yl;
			int     dxl;
			int     dyl;
#else
			int     yl;
			int     xl;
			int     dyl;
			int     dxl;
#endif
			};
		};
	union
		{
		struct {
			CP      cpFirst;
			CP      cpLim;
			int     doc;    /* <0 for fSpec such as auto pgn */
			};
		struct CA ca;
		};
	unsigned char      dcpDepend;
	int     fDirty : 1;
	int     fCpBad : 1;
	int     fLimSuspect : 1;
	int     fNoParaStart : 1;
/* when true last dl in hplcedl is not necessarily the last dl of the dr.
	when false last dl in hplcedl is the last dl of the dr. */
	int     fIncomplete : 1;
/* if DR is a cell in a table, Format and Display code treat fSplatBreak,
	ichMac and borders special */
	int	fInTable : 1;
/* if the cell belongs to the last row of a table, borders are handled even
	more differently */
	int	fBottomTableFrame : 1;
/* set if text is to be set to the dr width, not to the section width */
	int     fForceWidth : 1;
	struct PLCEDL **hplcedl;
	int     dypAbove;
	int     dxpOutLeft;
	int     dxpOutRight;
	int     idrFlow;        /* text should flow to this dr */
	char    ccolM1;		/* number of columns -1 allowed for this dr */
	char    lrk;            /* type of lr from which this came */
/* redundant bit: 1 means there is no dr with this dr as successor */
	int     fNoPrev : 1;    
	int     fConstrainLeft : 1;     /* constrained by abs on left */
	int     fConstrainRight : 1;    /* on right */
	int     fNewColOnly : 1;	/* only new column sections can start here */
	int     fFatLine : 1; /* does the last EDL have an fFat line? valid iff fPageView */
	int     fCantGrow : 1; /* DR is prevented from growing by other DRs */
	int     fHdrFtr : 1;  /* DR is a header or footer */
	int     fForceFirstRow : 1; /* treat table row at top of DR as a first row */
	int     fRMark : 1; /* DR contains revised dl, used only for fInTable DRs */
	int     fSpaceBefore : 1;  /* space before at top of DR should be ignored */
	int     : 6;  /* for future use */
	int	xwLimScroll;	/* rightmost limit of useful info in this DR */
	int	dxaBetween;	/* space between cols allowed for this dr */
	struct PLCEDL plcedl;
	int	dxa; /* width of dr in twips */
	int	ihdt; /* header/footer ftn separator type */
	};
#define cwDR   (sizeof(struct DR) / sizeof(int))

#ifdef MAC
#define	dywFatLine	2	/* width of the fFat line at the bottom of a DR */
#else /* WIN */
#define	dywFatLine	2 * vsci.dypBorder	/* width of the fFat line at the bottom of a DR */
#endif

struct DRF
	{
	struct PLDR **hpldr;
	int	    idr;
	struct DR   dr;
	struct PLCEDL *pplcedl;
	struct DRF  *pdrfNext;
	struct DRF  *pdrfUsed;
	unsigned    wLast;
	};
#define cbDRF  (sizeof(struct DRF))
#define cwDRF  (sizeof(struct DRF) / sizeof(int))

struct PLDR
	{
	int     idrMac;
	int     idrMax;
	int     cbDr;   /* set to cbDR */
	int     brgdr;  /* set to point to rgdr */
	int	fExternal;
	struct PLDR **hpldrBack;
	int     idrBack;
	struct PT ptOrigin;
	int     dyl;
	union   {
		HQ	hqpldre;    /* when fExternal true */
		struct DR rgdr[];   /* when fExternal false */
		};
	};
#define cwPLDR   (sizeof(struct PLDR) / sizeof(int))


#ifdef WIN
struct DRDL
	{
	int     idr;
	struct PLDR **hpldr;
	int     dl;
	};
#endif /* WIN */

/* underline state, holds starting parameters for underline segment */
struct ULS
	{
	union
		{
		int     grpfUL; /* look at fStrike/kul together */
		struct
			{
/* NOTE: hand native requires following flag to be encoded as 0/non 0 */
			char    fStrike;/* strikethrough is present */
			char    kul;    /* coded underline is present */
			};
		};
	struct PT pt;   /* starting point */
	int     xwLim;  /* ending x coordinate + 1 */
#ifdef WIN     
		HDC hdc;
		int ww;
	int dypPen;
#endif /* WIN */
	struct CHP *pchp;
	};


#define wwNil           0
#define wwTemp          1
#define wwPreview       2

#ifdef MAC
#define wwSccAbove	3
#define wwSccBelow	4
#define wwDocMin	5
/* Window types for the macintosh */
#define wkDoc           0x00
#define wkOutline       0x01
#define wkHdr           0x02
#define wkFtn           0x04
#define wkScc           0x08
#define wkPage          0x10
#define wkClip          0x20
#define wkNil           0x80
#endif
#ifdef WIN
#define wwLayout	3
#define wwSccAbove	4
#define wwDocMin	5
#define wkDoc		0x0000
#define wkHdr		0x4000
#define wkFtn		0x2000
#define wkOutline	0x8000
#define wkScc		0x1000
#define wkPage		0x0800
#define wkClipboard	0x0001
#define wkMacro		0x0002
#define wkDebug		0x0004
#define wkAtn		0x0008
#define wkSDoc		(wkAtn+wkFtn+wkHdr)
#endif

/* note that WWD is a subclass of PL. Also SCC is a subclass of WWD */
/* NOTE : If the WWD structure changes, the SCC needs to duplicate
		those changes */
struct WWD
	{ /* Window descriptor */
	int     idrMac;
	int     idrMax;
	int     cbDr;   /* set to cbDR */
	int     brgdr;  /* set to point to rgdr */
	int	fExternal;
	int     hpldrBack;  /* set to hNil to distinguish WWD from PLDR */

	union
		{
		int     wk;       /* window type */
		struct {
			int fClipboard : 1;
						int fMacro : 1;
						int fDebug : 1;
						int fAtn : 1;
						int :7;
			int fPageView : 1;
			int fScc : 1;
						int fFtn : 1;
						int fHdr : 1;
			int fOutline : 1;
				};
		};
	int     fDrDirty : 1;   /* DRs need to be recalculated */
	int     fDirty : 1;     /* ww needs updating */
	int     fBackgroundDirty : 1;/* drs changed in page view but not redrawn */
	int     fHadRuler : 1;  /* remember ruler across outline mode */
	int     fSetElev : 1;   /* move elevator according to cpFirst */
#ifdef MAC
	int     fFullSizeUpd: 1;/* on if rcInval is full size of window */
				/* Mac - used as an optimization in InvalCp:
				/* signals that the window is dirtier than rcwInval,
				/* so we can't clip to the update region */
#else
	int	: 1;
#endif

	int     fLower : 1;     /* on iff lower pane */
	
#ifdef MAC
	int     fNotAllVis: 1;  /* set if window not entirely on screen */
	int     fNoLlcStyle: 1; /* OK to show style llc */
	int     fNewDrs : 1;	/* new DRs were just created */
	int     fRuler : 1;     /* Draw tab and margin ruler */
	int     rk : 2;         /* Current ruler state */
		int     : 3;
	int /*GRAFPORT*/ far *wwptr; /* copy of Mac window pointer */
	int /* struct IB ** */ hibList;/* handle to list of iconbars */
#endif /* MAC */
#ifdef WIN
	int     fEllip : 1;     /* Show ellipses at the end of body text. */
		int     fShowF : 1;     /* Show character formatting in outline. */
	int	fNeedEnhance : 1;/* May need to redisplay postponed obj */
	int	fDisplayAsPrint: 1;
	int     fNewDrs : 1;	/* new DRs were just created */
	int	: 4;
	GRPFVISI	grpfvisi;   

		HWND    hwnd;           /* Windows window handle */
		HDC     hdc;            /* DC for window */
		HWND    hwndVScroll;    /* Windows window handle for vert scroll bar */
		KMP **  hkmpCur;        /* Current keymap for this pane */
		HWND    hwndIconBar;
	HWND	hwndPgvUp;	/* handle to PAGEUP icon in page view */
	HWND	hwndPgvDown;	/* handle to PAGEDOWN icon in page view */
#endif /* WIN */

	int     mw; /* back pointer to mw */
	union {
		struct RC rcwDisp;
/* note: text display area starts at: xpSelBar, see dispdefs */
		struct {
#ifdef MAC
			int     ywMin; /* pos of top of writeable area of window */
			int     xwMin;
			int     ywMac; /* pos of bottom of window */
			int     xwMac; /* window rel position of last displayable pixel +1 */
#else /* WIN */
			int     xwMin;
			int     ywMin; /* pos of top of writeable area of window */
			int     xwMac; /* window rel position of last displayable pixel +1 */
			int     ywMac; /* pos of bottom of window */
#endif /* ~MAC */
			};
		};
/* The rectangle of the page, in xe,ye coordinates.  Determines scroll state */
	union
		{
		struct RC       rcePage;
		struct {
#ifdef MAC
			int	yeScroll;
			int	xeScroll;
#else
			int	xeScroll;
			int	yeScroll;
#endif /* MAC */
			};
		};

	struct SELS     sels;

/* Invalid area of window */
	union
		{
		struct RC rcwInval; 
		struct  /* note order of xp and yp's in rect */
			{
#ifdef WIN
			int     xwFirstInval;
			int     ywFirstInval;
			int     xwLimInval;
			int     ywLimInval;
#else /* macintosh */
			int     ywFirstInval;
			int     xwFirstInval;
			int     ywLimInval;
			int     xwLimInval;
#endif
			};
		};

#ifdef MAC
	int		bppCur;			/* Current number of bits per pixel */
#endif
	int     wwDisp;     /* chain of ww's displaying doc */
	int     dqElevator; /* dr where elevator is currently */
	int     xwSelBar;   /* where selbar started, == dxwStyWnd */
#ifdef WIN
		int     dxwStyWnd;  /* WIN remember width of styname area in galley vw */
#endif
	int     ipgd;       /* in page view, page table index */
	int     xhScroll;   /* Absolute horizontal scroll */
#ifdef WIN
	int	xwLimScroll;
#else
	int     wSpare2;   /* spare for future use */
#endif
/* FUTURE tomsax(pj): make this a ww for the WWD too? */
	int     wReserved;  /* is ww in SCC */
	struct DR rgdr[1];      /* WWD is a pldr */
	};

#define cwWWD   (sizeof(struct WWD) / sizeof(int))
#define cwWWDShort      (offset(WWD, mw) / sizeof(int))

#define wwptrNil        (int far *) 0

#define mwNone		-1	/* parm to SetActiveMw, means "nothing is active" */
#define mwNil           0
#define mwTemp		1
#define mwPreview	2
#ifdef WIN
#define mwLayout	3
#define mwDocMin	4
#else /* MAC */
#define mwDocMin	3
#endif

/* whole window structure */

struct MWD
	{
	int	fSplit: 1;      /* i.e. wwLower is not nil */
	int	fActive: 1;     /* refers to mwd */
	int	fAtnVisiSaved: 1; /* saved the fSeeHidden before atn pane is created */
	int	wwActive : 5;
	int	wwRuler : 5;    /* (WIN) */
		int     : 3;

	union {
		struct {
			unsigned char   wwUpper;
			unsigned char   wwLower;
			};
		unsigned char rgww[2];
		};

	int	doc;
#ifdef WIN
	HWND 	hwndRuler;
	HWND	hwnd;
	struct /* for macro windows */
		{
		int docMcr;
		int wSpare0; /* FUTURE: Not removed yet to avoid probs with release */
		};
	int	fHorzScrollBar:1;
	int	fVertScrollBar:1;
	int 	lvlOutline    : 4;  /* lvl shown on the outline iconbar. */
		int     fCanSplit : 1; /* window can be split */
	int	:9;                /* spares */
		HWND	hwndHScroll;        /* Windows window handle for scroll bar */
	HWND	hwndSplitBox;
	HWND	hwndSizeBox;
	HWND	hwndSplitBar;
	unsigned ypSplit;           /* split position if fSplit is true */
/* position (in desktop client coordinates) of MWD */
	int xp;
	int yp;
	int dxp;
	int dyp;
#else  /* MAC */
	int /*GRAFPORT*/ far *wwptr;/* Macintosh window handle */
	struct RC rc;               /* not large size, global coordinates */
#endif
	int wSpare2;   /* spare for future use */
	int wSpare1;   /* spare for future use */
	};
#define cbMWD		sizeof(struct MWD)
#define cwMWD		CwFromCch(cbMWD)


struct WSS /* Window Save State */
	{
	struct SELS sels;
#ifdef MAC
	struct RC rcWindow;
	struct RC rcMwd;
#endif /* MAC */
	};
#define cbWSS (sizeof (struct WSS ))
#define cwWSS (cbWSS / sizeof(int))

/* cza for various ut's */
#define czaInch         1440
#define czaP10          144
#define czaPoint        20
#define czaCm           567
#define czaP12		240
#define czaLine		240
#define czaPicas        240


/* NormCp flags */
#define ncpForceYPos 1
#define ncpHoriz 2
#define ncpVisifyPartialDl 4
#define ncpAnyVisible 8


/* UpdateDr modes */
#define udmodNormal     0
#define udmodLastDr     1
#define udmodTable      2
#define udmodNoDisplay  3
#define udmodLimit	4

#ifdef WIN
struct WSI  /* Window Size Information */
	{
	union
		{
		HWND		hwnd;
	int		ww;
		};
	struct RC *prc;
	BOOL      fPane;   /* whether window is a pane or not */
	};
#endif

#ifdef DEBUG
struct DR *PdrGalley();
int IpgdCurrentWw();
#else
#define PdrGalley(pwwd)         (&(pwwd)->rgdr[0])
#define IpgdCurrentWw(ww)	(PwwdWw(ww)->ipgd)
#endif

#ifdef WIN
/* H A N D  N A T I V E */
#define PmwdMw(mw)	((struct MWD *)N_PmwdMw(mw))
#define PwwdWw(ww)	((struct WWD *)N_PwwdWw(ww))
#define PmwdWw(ww)	((struct MWD *)N_PmwdWw(ww))
#define HwwdWw(ww)	((struct WWD **)N_HwwdWw(ww))
#else
#include "screen.h"
#endif
