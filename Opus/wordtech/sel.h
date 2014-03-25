
/* sk values correspond to bits in sel below */

#ifdef	MAC
#define skNil 2
#define skBlock 4
#define skGraphics 8
#define skSel 0
#define skIns 1
#define skRows          16
#define skColumn        52
#else
#define skNil		0x10
#define skBlock		0x08
#define skGraphics	0x04
#define skSel		0x00
#define skIns		0x20
#define skRows      0x02
#define skColumn    0x0B
#endif

struct SEL
	{
	union {
		struct {
			int     : 10;
			int     sk : 6;
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
	unsigned char      fForward;
	unsigned char      fInsEnd;

	union {
		struct {
			CP      cpFirst;
			union {
				CP      cpLim;
				CP      cpLast;
				};
			int     doc;
			};
		struct CA       ca;
		};
	union
		{
		struct
			{
			int     xpFirst;
			int     xpLim;
			};
		struct
			{
			int     itcFirst;
			int     itcLim;
			};
		};
	CP      cpAnchor;
	int     sty;
	int 	itcOppositeAnchor; 
#ifdef WIN
	CP	cpAnchorShrink; /* anchor point for shrink selection, has to be in SELS */
#endif
/* End of SELS */
	int     ww; /* where selection is shown */
	int     pa;

	CP      cpFirstLine; /* hint for establishment of block selection */
/* if false, selection exists but not drawn */
	int     fHidden;
/* fOn is used for blinking the fIns cursor */
	int     fOn;
/* these are used only if fIns. Not valid if fHidden. */
/* position of cursor. xp is scrolled. yp is bottom of cursor */
	int     xw;
	int     yw;
	int     dyp;
/* tick at which cursor was last blinked */
	long    tickOld;
	int     fUpdateChp : 1;
	int     fUpdateChpGray : 1;
	int     fUpdatePap : 1;
	int     fUpdateRibbon : 1;
	int     fUpdateRuler : 1;
	int     fUpdateStatLine : 1;
	int     fUpdateStatLine2: 1;

#ifdef	MAC
	int     fNinchChp : 1;
	int     fNinchPap : 1;
	int     fNinchSep : 1;
	int     fItalic : 1;
	int     : 5;
#else
	int		: 9;
#endif	/* MAC */
	struct RC       rcwClip;
	struct  CHP  chp;
	};

#define cwSEL   (sizeof (struct SEL) / sizeof (int))
#define xpNil 0x8000

/* highlighting patterns */
#define paInvert 0
#define paDotted 1

/* Selection types */
#define styNil          0
#define styChar         1
#define styWord         2
#define stySent         3
#define styPara         4
#define styLine         5
#define styDoc          6
#define styWordNoSpace  7
#define styLineEnd      8
#define styScreen       9
#define stySection      10
#define styHome         11
#define styCol          12
#define styRow          13
#define styColAll       14
#define styWholeTable   15
#define styCRLF	        16
#define styScreenEnd	17
/* must be the last one since outline level is encoded as an increment */
#define styOutline      18


/* For grpf parameter in Select1 */
#define maskInsEnd  	 0x0001
#define maskSelChanged   0x0002
#define maskTableExempt  0x0004   /* ignore special table handling */

/* For grpf parameter in MakeExtendSel */
#define maskFDoAdjust			0x0001
#define maskFDoMakeSelCurVisi	0x0002


#define wbWhite         0       /* Word break types */
#define wbText          1
#define wbPunct         2

/* block decomposition state */
struct BKS
	{
	union
		{
		struct {
			CP cpFirst;     /* current position in block */
			CP cpLast;      /* last line to be considered */
			int doc;
			};
		struct CA ca;
		};
	int xpFirst;    /* limits of block */
	int xpLim;
	int itcFirst;   /* limits of column selectiom */
	int itcLim;
	int ww;		/* ww displaying */
	int fColumn : 1;
	int fLastRow : 1;
	int fReturnEmptyCell : 1;	/* Return if empty cell found */
	int : 13;
	};
#define cbBKS (sizeof(struct BKS))

/* secondary selection mode commands */

#define sscNil 0
#define sscMove 1
#define sscCopy 2
#define sscCopyLooks 3
#define sscMouse 4     /* WIN only - special case while switching windows for mouse dyadic op */


/* selection position type - identifies which area of DR is currently being
	touched by mouse down -- returned by DlWherePt */
struct SPT
	{
	int fSelBar : 1;
	int fInTable : 1;
	int fWholeColumn : 1;
	int fWholeRow : 1;
	int fLeftBorder : 1;
	int fRightBorder : 1;
	int fInStyArea: 1;
	int fInDr: 1; /* false iff nearest dr had to be used */
	unsigned char  itc;
	};

#define dxwBorder 4

struct CLPT /* column selection "point" */
	{
	CP cp;
	int itc;
	};

struct FLSS
	{
	CP	cpMin;
	CP	cpMac;
	BOOL	fSplatDots;
	CHAR    chBreak;
	};

#define itcNil -1

#ifdef MAC
#define dypCursBmpMax		30
#endif

#ifdef WIN
#define skNotGlsy (0x20|0x10|0x08|0x01)
#endif /* WIN */
