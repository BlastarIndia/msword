
/** These depend on the file structure for Opus .INI files **/
#define nPrefdVerCur   	    11
#define nPrefVerCur	18
#define nPrefdVerDini           (nPrefdVerCur+nPrefVerCur)
#define cbSizeOfPrinterMetrics  (23 * sizeof(int))
#define cbFontWidth             (256*sizeof(int))

/** These are arbitrary, can be increased if not big enough **/
#define cbBuffMax           4096
#define cStInSttbMax        100
#define cFceMax             100

/** These are for internal use in DINI **/
#define bool int
#define DECIMAL 10
#define HEX     16
#define OCTAL   8
#define fFalse  0
#define fTrue   1
#define printSzBool(psz,value)  printSzBoolForce(psz,value,fFalse)

typedef unsigned int uns;

/** These structures should correspond to structures in Opus **/
typedef long HQ;

struct STTB
	{
	int     bMax;
	int     ibstMac;
	int     ibstMax;
		union
				{
				struct {
					uns	cbExtra : 13;
					uns	fNoShrink : 1;
					uns	fExternal : 1;
					uns	fStyleRules : 1;
					} S2;
				int cbExtraOld;
				} U2;
	union
		{
		int     rgbst[2];
		HQ      hqrgbst;
		} U1;
	/*char  grpst*/
	};

#define cbBaseSttb 8

struct FCID 
		{ 
				/* wProps */
		unsigned int             fBold: 1;   /* bold, italics in same position as chp */
		unsigned int             fItalic: 1;
		unsigned int             fStrike: 1;
		unsigned int             kul: 3;
		unsigned int             fFixedPitch: 1;
		unsigned int             fSpare2: 1;
		unsigned int             hps: 8;
				/* wExtra */
		unsigned int             ibstFont: 8;    /* index into master font table */
		unsigned int	            : 8;
		};

/* F o n t  C a c h e  E n t r y */
struct FCE  
		{
		struct FCID fcidActual; /* what this entry really contains */
		int dxpOverhang;        /* overhang for italic/bold chars */
		int dypAscent;          /* ascent */
		int dypDescent;         /* descent */
		int dypXtraDescent;     /* descent, but with recommended external
									leading added for printer fonts only */                                     
		unsigned int fPrinter: 1;        /* Is this cache entry for a printer font? */
		unsigned int fVisiBad: 1;        /* fTrue if space wd != visi space wd */
		unsigned int fFixedPitch: 1;	
		unsigned int : 13;        /* spare */
		/* End of same as FTI structure */

		/* union */
		long dxpWidth;          /* Width for fixed pitch font */

		struct FCID fcidRequest; /* request this entry satisfied */
		struct FCE *pfceNext;   /* next entry in lru list */
		struct FCE *pfcePrev;   /* prev entry in lru list */
		int hfont;            /* windows' font object */
		};

/** These are excerpts from wordwin.h **/

#define ichUsrInitlMax	6
#define ichUsrNameMax	50



typedef union  {
	struct {
		unsigned int	   fvisiTabs:1;
		unsigned int	   fvisiSpaces:1;
		unsigned int	   fvisiParaMarks:1;
		unsigned int	   fvisiFtnRefMarks:1;
		unsigned int	   fvisiCondHyphens:1;
		unsigned int	   fvisiShowAll:1;
		unsigned int	   fNoShowPictures:1;
		unsigned int	   fSeeHidden:1;
		unsigned int	   flm: 2;
		unsigned int	   : 1;
		/* These bits are for fields: if true, show results
			for that field group, if false, show instructions */
		unsigned int	   grpfShowResults : 2;
			unsigned int	   fDrawTableDrs: 1;
			unsigned int	   fDrawPageDrs: 1;
		/*  if on, ignore field's fDiffer flag */
		unsigned int	   fForceField : 1;
		} S1;
	int	w;
	} GRPFVISI;

struct PREF
	{
	int		wSpare;
	unsigned int		ut: 4;
	unsigned int		: 2;
	unsigned int		fRibbon:1;
	unsigned int		fStatLine:1;
		/* used in customize dialog */
	unsigned int		fPromptSI: 1;
	unsigned int		fBkgrndPag:1;
	unsigned int		fAutoDelete:1;

	unsigned int		fShortMenus : 1;
	unsigned int		fRuler : 1;
	unsigned int		fHorzScrollBar:1;
	unsigned int		fVertScrollBar:1;
	unsigned int		fPageView : 1; /* if true, New/Open default in page view */

/* Following field is copied into every ww */
	GRPFVISI	grpfvisi;

	unsigned int	   fDisplayAsPrint: 1 ;
	unsigned int	   fSplAutoSugg: 1 ; 
	unsigned int	   fShowAllMacros: 1 ; 
	unsigned int	   : 1 ; /* spare */
		/* used in customize dialog */
	unsigned int	   iAS: 2; /* Auto Save frequency index. */

	unsigned int	   fDraftView: 1;	
	unsigned int	   fZoomApp: 1;
	unsigned int	   fZoomMwd: 1;
	unsigned int	   fEllip: 1;
	unsigned int	   fShowF: 1;
	unsigned int	   fPrDirty: 1;    
	unsigned int	   fSpare: 3;
	unsigned int	   fPrvwTwo : 1;  /* one or two pages in print preview */
	unsigned   dxaStyWnd;  /* width of style name area */

	char	   stUsrInitl [ichUsrInitlMax];
	char	   stUsrName [ichUsrNameMax];
	int	   rgwSpare [6];
	};

/** This structure is used internally by DINI **/
struct RGCHECK
		{
		int     start;          /* where this st starts */
		int     end;            /* and where it ends */
		bool    match;          /* can we match this st's beginning? */
		};

/* definitions for user preference file */
/* The user preference file is laid out as follows:

	PREF structure	     \
	header info	     /	PREFD structure

	file cache
	doc mgmt query path
	printer name sz and driver name sz
	printer metrics for vpri
	font name sttb
	printer font enumeration STTB
	Font descriptors (portion of rgfce) (size prefd.cbFontCache)
	Font Width Table (512 bytes for each !fFixedWidth FCE)
*/

#define cchMaxFileCache 	(ibstMaxFileCache*ichMaxFile)

/* NOTE : If you change this structure, change nPrefdVer */
#define nPrefdVerCur   	    11
#define nPrefPrefdVerCur    (nPrefdVerCur+nPrefVerCur)

struct PREFD {
/* version first so it is invariant between versions! */
	int	   nPrefPrefdVer; /* contains nPrefdVer + nPrefVer */
	struct PREF 	pref;
/* room on file for PREF expansion in future versions */
/* This is one spare allocation that should NOT be deleted when we ship */
	int	  rgwSpare [16];	
/* fields describing remainder of preference file */
		int	   cbSttbFileCache;
	int	   cbStDMQPath;
	int	   cbPrNameAndDriver;
	int	   cbPrinterMetrics;	
	int	   cbPrenv;
	int	   cbSttbFont;
	int	   cbSttbPaf;
	int	   cbRgfce;
	int	   cbFontWidths;
	/* struct STTB  sttbFileCache;          file MRU list */
	/* CHAR         stSMQPath[]             document management path */
	/* CHAR		grpszPrNameAndDriver 	printer name and driver name */
	/* int		rgwPrinterMetrics[];	device  metrics for printer */
	/* struct PRENV	prenv;			printer environment */
	/* struct STTB  sttbFont;		font name table */
	/* struct STTB  sttbPaf;		printer font enumeration */
	/* CHAR		grpstFonts[]; 		Face names & PAF from printer enum */
	/* struct FCE	rgfce[]; 		Printer font cache */
	/* int		rgrgdxp [] [256]; 	Widths for cache */
	};
/* NOTE : If you change this structure, change nPrefdVer (above) */


#define cbPREFD (sizeof(struct PREFD))




