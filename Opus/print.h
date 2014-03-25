
/*
*  iidBinManual through iidBinMixed must match order of strings in
*  dlbenum.c
*/
#define iidBinManual   0
#define iidBinEnvelope 1
#define iidBinTractor  2
#define iidBinAuto     3
#define iidBin1        4
#define iidBin2        5
#define iidBin3        6
#define iidBinMax      7  /* must match cBinsMax below */
#define iidBinMixed    7  /* DO NOT make < iidBinMax...this one doesn't have
                             an entry in vpri.rgidBin[]. */
#define iidBinNil      8
#define idBinNil      -1

struct PRI  /* Printer Info */
	{
	struct FTI *pfti;
	HDC	hdc;
	char    (**hszPrinter)[];       /* Printer name string */
	char    (**hszPrPort)[];        /* Printer port string */
	char    (**hszPrDriver)[];      /* Printer driver string */
	uns     wmm : 8;                /* window message mask indicates
									 * pending win.ini change messages
									 */
	uns     fDCFree : 1;            /* DC has been freed; it's not needed */
	uns     fPrErr : 1;
	uns     fIC : 1;                /* Whether hdc is DC or IC */
	uns     fGraphics : 1;          /* from BANDINFO escape */
	uns     fText : 1;
	uns     fHaveEnv : 1;           /* whether or not hprenv is valid.
										NOTE: hprenv != hNil is not a
										valid test! */
	uns     fPrintingAtn : 1;       /* true iff Atn doc in docUndo */
	uns     fColor : 1;             /* printer supports color */
	
	void    *plr;      /* pointer to current LR...used by low-level
							PostScript.  Yes, it's very sleazy, but
							we're not wasting space by storing the
							whole friggin' LR when all we need are a
							couple of fields. */

	CP	cpAdjust;  /* when using docUndo for printing a portion
					* of a document (printing a selection from
					* a file or printing annotations), set
					* cpAdjust to cpFirst of the selection.
					*/
/* BEGINNING OF PRINTER INFO THAT IS STORED IN USER STATE FILE */
	int	dxuInch;
	int	dyuInch;
	int 	dxpRealPage;	/* page size in device units */
	int 	dypRealPage;
	int     dxmmRealPage;	/* Width of display device in mm */
	int     dymmRealPage;	/* Height of display device in mm */
	struct PT ptScaleFactor; /* Printer scale factor - 0's if none */
	struct PT ptUL; /* Upper-left corner of printable area */
	int     cBins;   /* count of paper sources on current device */
	short   idBinCur;
	short   rgidBin[iidBinMax];
	int     dxpPrintable;  /* printable page size in device units */
	int     dypPrintable;
	long    rgbText;       /* default text color for device */
	int	fSupportPubChars;	/* Whether printer supports pub chars */
/* END OF PRINTER INFO THAT IS STORED IN USER STATE FILE */

	struct STTB **hsttbPaf;	/* fonts available on this printer */
							/* really PAF's, not st's */
	char 	**hprenv;	/* handle to current print environment */
	HBRUSH  hbrText;        /* brush to printer's default text color */
	int     fHaveBinInfo;
	int	wwInit;      /* save ww passed to FInitWwLbsForRepag so
						FResetWwLayout generates correct visi flags */
	int	cInitNest;   /* semaphore for nested calls to FInitWwLbs... */
	int fInPrintDisplay;
	int fDPR;		/* does this printer support DRAWPATTERNRECT? */
	int	rgwSpare1[3];
	};
#define cbPRI (sizeof(struct PRI))

#define ichMaxProfileSz     256     /* Longest user profile string */

#define bMinPriFile	offset(PRI,dxuInch)
#define bMaxPriFile	offset(PRI,hsttbPaf)

/* masks/values for collecting pending winsyschange messages */
#define wmmWinIniChange     1
#define wmmDevModeChange    2
#define wmmFontChange       4
#define wmmSysColorChange   8

#define cBinsMax            7  /* must match iidBinMax above */
typedef struct
	{
	short rgidBin[cBinsMax];
	char szBin[cBinsMax][24];
	} BINS;

/* Printer Available Font */
/* Note that this is structured like an st for storage in an sttb */
struct PAF
	{
	CHAR cbPafM1;   /* st convention; # of bytes excluding this one */
	CHAR ibst;      /* identifies font name and family */
	CHAR rghps[];   /* array of available heights for this face name */
	};

struct BINF
	{
	BOOL fGraphics;
	BOOL fText;
	struct RC   rcGraphics;
	};

#define cbPafLast 256
#define IhpsMacPpaf(ppaf) ((ppaf)->cbPafM1 + (1-offset(PAF,rghps)))

/* print range choices */
#define prngAll 0
#define prngSel 1
#define prngPages 2

/* print source choices */
#define istDocument 0
#define istSummary 1
#define istAnnotation 2
#define istStyles 3
#define istGlossary 4
#define istKeys 5

/* printer setup dialog box state */
struct PRSU
	{
	uns    prng : 2;
	uns    istPrSrc : 3;
	uns    fReverse : 1;
	uns    fSeeHidden : 1;
	uns    fSummary : 1;
	uns    fDraft : 1;
	uns    fShowResults : 1;
	uns    fAnnotations : 1;
	uns    fPrintAllPages : 1;
	uns    iidBinPrFeed : 4;
	uns    cCopies : 15;
	uns    fRefresh : 1;
	};

#define cwPRSU          (sizeof (struct PRSU) / sizeof (int))


/* Escapes that have not made it into qwindows.h */
#define GETSETPAPERBINS  29
#define ENUMPAPERBINS    31
#define SETCHARSET	772

typedef struct _cepr
	{
	int fPrintMerge : 1;
	int fFileFind : 1;
	} CEPR;


/* Double underline modes */
#define	ulmNil			-1
#define	ulmNormal		0
#define	ulmAlwaysSpace	1
#define  ulmNeverSpace	2

/* Define color modes */
#define colFetch 0
#define colAuto  1
#define colText  2
