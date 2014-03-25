/* R U L E R . H */

#ifdef DEBUG
#ifdef PCJ
/* #define DBGRUL */
#endif /* PCJ */
#endif /* DEBUG */

/* Misc Constants */

#define xpRulerScroll  (vsci.dxpMinScroll)

#define dyScale        7
#define dyMark         8
#define dxMark         8
#define dyMajorTick    7
#define dyMinorTick    3
#define dyTinyTick     1

#ifdef WIN23
#define dxMark3        9
#endif /* WIN23 */

/*  STRUCTURES  */




/*  Ruler Edit Block
	this block is set up for 1) any edit action (drag or keyboard edit) and
	2) for drawing the ruler.  

	note that for a paint alone the hdcscratch & hbmscratch are not considered
	worth being set up.

*/

struct REB
	{

	struct RSD ** hrsd;/* the ruler to which the following apply */
	int cInit;         /* no. times this reb initialized */
	int rpcSetup;      /* what this reb is set up for */

	int wSpare;
	int xpLeft;        /* left clip point */
	int xpRight;       /* right clip point */

	int imkNoDraw;     /* indicates do not draw this-for scrolling & dragging */
	int iNoDraw;       /* dont draw this tab/column/cellmark */

	int xaCursor;      /* where is the user in EdTabs mode */
	int fUndoSet;      /* have we setundo for this edit */
	struct UAB uabOld; /* copy of the old uab */

	};



/*  Ruler State Descriptor
	contains current state information about a single ruler.
	the displayed information reflects the state of the cpFirst of the current
	selection.  the information is Grayed if that information is not the same
	over the entire selection.  if any tab information differs, all tabs are
	said to be gray.
*/

struct RSD
	{
	HWND hwndRuler;      /* the ruler's window handle */
	HWND hwndMark;       /* mark area window handle */
	int xpLeft;          /* left edge of mark window */
	int xpRight;         /* right edge of mark window */
	int xpZero;          /* where is zero point of ruler (relative to xpLeft) */
	int rpc;             /* used to force the updateruler code to redraw a
							specific portion of the ruler on next update */
	int jcTabCur;        /* currently selected tab type */
	CHAR rk;             /* ruler type */
	CHAR irgTmw;         /* sdm construct: 1 per ruler */
	int ut;              /* currently displayed unit */
	int idr;             /* current dr displayed for */
	struct PAP pap;      /* current paragraph styles state */
	struct PAP papGray;  /* paragraph properties which differ over the sel */
	union
		{
		struct  /* rkNormal */
			{
			int fTabGray;        /* true iff some tab is gray */
			int dxaTab;          /* default tab increment */
			int itcCur;          /* cell if in table */
			int dxaCellLeft;     /* added space for table */
			int fInTable;        /* displayed selection is in a table */
			int dxaColumnNorm;   /* current column width */
			};
		struct  /* rkTable */
			{
			int fTcGray;
			int itcMac;
			int rgdxaCenter[itcMax+1];
			int dxaGapHalf;
			};
		struct  /* rkPage */
			{
			int xaPage;
			int dxaRightMar;
			int dxaLeftMar;
			int ccolM1;
			int ddxaColumns;
			int dxaExtraRight; /* gutter, etc. */
			int dxaExtraLeft; /* gutter, etc. */
			BOOL fMirrorMargins;
			int dxaColumnPage;  /* current column width */
			};
		};
	};



/*  Ruler Sprm State
	this structure contains a handle to the global grpprl which is referenced
	by sprmPRuler.
*/

struct RULSS
	{
	struct CA caRulerSprm;/* expanded ca of the selection grpprl applies to */
	int cbGrpprl;         /* count of bytes in grpprl */
	char ** hgrpprl;      /* handle to the grpprl */
	struct RSD ** hrsd;   /* the rsd of the ruler which set up the grpprl */
	int sk;               /* type of selection covered by ruler */
	int itcFirst;         /* used only if selection is fTable */
	int itcLim;           /* used only if selection is fTable */
	};






#ifdef RULER

	/* CONSTANTS/MACROS USED ONLY BY RULER CODE */


/* Ruler types */
#define rkNormal    0
#define rkTable     1
#define rkPage      2
#define rkMax       3


/* Marks (things that one can drag) */
/* 0 through imkNormMax -1 correspond to indicies into idrbRulerMarks */
#define imkTabLeft     0
#define imkTabCenter   1
#define imkTabRight    2
#define imkTabDecimal  3

#define imkTabMax      (imkTabDecimal + 1)
#define imkIndMin      (imkIndLeft1)

#define imkIndLeft1    4
#define imkIndLeft     5
#define imkIndRight    6

#define imkDefTab      7

#define imkTLeft       8
#define imkTableCol    9
#define imkLeftMargin  10
#define imkRightMargin 11

#define imkNormMax     12

#define imkIndCombo    imkNormMax  /* flag:  indicates drag of left & left1 */
#define imkNil         -1



/*  Derived Constants/Macro Synonyms
	the following are macros for using the ruler sizing information
*/

#define ypScaleTop       0
#define ypScale          (ypScaleBottom-1) 
#define ypMarkTop        ypScaleBottom
#define ypMark           ypScaleBottom

#define HrsdFromHwndRuler(hwnd) ((struct RSD **)WRefFromHwndIb(hwnd))

/* Ruler Paint Code Masks */
#define rpcmPaintStr   0x1000

#define rpcmScale      0x0001
#define rpcmMark       0x0002
#define rpcmColumn     0x0004

/* Ruler Paint Codes */

#define rpcScaleArea   (rpcmScale | rpcmColumn)
#define rpcMarkArea    (rpcmMark | rpcmColumn)
#define rpcColumnMark  (rpcmColumn | rpcScaleArea | rpcMarkArea)
#define rpcBltMark     (0x0000)

#define rpcNil         (0x0000)
#define rpcNoPaint     (rpcNil)
#define rpcAllPaint    (rpcScaleArea | rpcMarkArea | rpcColumnMark)
#define rpcPaintMsg    (rpcmPaintStr | rpcAllPaint)



/* Ruler Erasure Codes */
#define recSelBar      0
#define recScale       1
#define recMark        2



/* Mark Styles */
#define msNormal       0
#define msGray         1
#define msInvert       2

#endif  /* RULER */


/* Used by Ribbon/Ruler repeat code. */
#ifdef REPEAT
typedef struct
	{
	int	doc;
	int	fSelMoved:	1;
	int	fDirty:		1;
	int	fNoClear:	1;
	int	fRibbon:	1;
	int	fTouched:	1;
	/* To record formatting through ribbon. */
	int	fBold:		1;
	int	fItalic:	1;
	int	fSCaps:		1;
	int	kul:		3;
	int	iSuperSub:	2;
	int	Spare:		3;
	int	ftc;
	int	hps;
	/* To record formatting through ruler. */
	int	wSpare;
	int	cbGrpprl;
	CHAR	**hgrpprl;
	}	RRF;

#define iSSNormal	0
#define iSSSuper 	1
#define iSSSub		2

#endif /* REPEAT */

NATIVE DrawMinorTicks();
NATIVE XaMarkFromXpMark ();
NATIVE XpMarkFromXaMark ();
NATIVE XaQuantize ();   
