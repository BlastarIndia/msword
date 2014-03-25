
#define cchPassMax  16 /* Max length password, inlcuding null terminator. */
#define CBENCMAX    16 /* Length encryption array. Must be a power of 2 */
#define IBENCMASK (CBENCMAX-1) /* Mask for looping index to rgbEncrypt. */
#define CBENCMASK (CBENCMAX-1)

typedef struct
	{
	int    eidED;
	int    inmEd;
	int    inmLast;
	int    inmLim;
	int    (*pfnEd)();

	uns    fMovePage:1;
	uns    fInactive:1;
	uns    fUseScreen:1;
	uns    fScrollDown:1;
	uns    fSingleRef:1;
	uns    fBlankLine:1;
	uns    fEmpty:1;
	uns    fRedrawEd:1;

	uns    fAutoRedraw:1;
	uns    csyHeader:7;

	uns    csxMargin:4;
	uns    unused:12;

	int    inmPage;
	int    inmRef;
	CHAR cnmLine;
	CHAR csxNm;
	CHAR padEd;    /* unsed */
	CHAR cnmGrp;    /* The number of names in each group. */
	int    w1Ed;
	int    w2Ed;
	int    w3Ed;
	int    w4Ed;
	int    w5Ed;
	} ED;
#define    cbED  (sizeof (ED))
#define    cwED  (sizeof (ED) / sizeof(int))

typedef struct
	{
	/* general information used while reading in file */
	int doc;    /* destination doc */
	int fnRead; /* source fn */
	FC fcRead;
	int fnTemp; /* temp file created for reading in foreign file */
	int dff;    /* dffOpenBiff, dffOpenWks, or dffOpenMP */
	CP cpCur;   /* current location in destination doc */
	FC fcNames;

	/* info for dealing with password protected files */
	int wKey1;
	int wKey2;
	int fEncrypted : 1;
	int fHavePassword : 1;
	int fPublic : 1;
	int fEncHDS : 1;
	int fDefSet : 1;
	int : 1;
	int fBiffMP : 1; /* true if multiplan biff */
	int f1904 : 1;
	int ibEncrypt;
	int ibEncryptNames;
	char szPassword[cchPassMax];

	/* info describing user selections */
	unsigned *prgsRow;
	unsigned *prgsCol;
	int isMac;   /* must be power of 2. isMac/2 is # of user selections */

	/* extra MultiPlan garbage */
	int fNumExtern;
	int fmtDef;
	int cdgsDef;
	ED edMenu;
	} FF;     /* Foreign File */

typedef struct
	{
	/* rectangular array...bounds are inclusive */
	unsigned uColLeft;
	unsigned uColRight;
	unsigned uRowTop;
	unsigned uRowBottom;
	} URC;

#define    cchNil    (-1)
#define    inmNil    (-1)
#define isMax 40

/* ------------ Lotus defines --------------------------------------------- */

/* Worksheet codes: */

#define MAXCODE     47
#define BEGIN       0
#define END         1
#define TOTAL_SIZE  6
#define NAMED_RANGE 11
#define BLANK       12
#define INTEGER     13
#define REAL        14
#define LABEL       15
#define FORMULA     16
#define CURR_GRAPH  45
#define NAMED_GRAPH 46
#define GRAPH2      64
#define GRAPHNAME   65
#define NNAME       71
#define wksPASSWORD 75
#define STRING      51

#define FFFF    65535   /* hex ffff marker for Entire Wks */
#define FFFE    65279   /* fffe marker for Current Graph (lo byte first) */

/* Excel Biff defines */
#define rtDimensions 0
#define rtBlank     1
#define rtInteger   2
#define rtNumber    3
#define rtLabel     4
#define rtBoolErr   5
#define rtFormula   6
#define rtString    7

#define rtBOF       9	    	/* also in file.h */
#define rtEOF       10
#define rtLbl       24
#define rtArray     33
#define rt1904      34
#define rtFilePass  47


#define ipicGeneral    	0
#define ipicFixed0      1
#define ipicFixed2    	2
#define ipicComma0      3
#define ipicComma2      4
#define ipicDollar0    	5
#define ipicDollar0R    6
#define ipicDollar2    	7
#define ipicDollar2R    8
#define ipicPercent0    9
#define ipicPercent2   10    
#define ipicExp        11

#define ipicMMDDYY     12
#define ipicDDMMMYY    13
#define ipicDDMMM      14
#define ipicMMMYY      15
#define ipicHHMMAP     16
#define ipicHHMMSSAP   17
#define ipicHHMM       18
#define ipicHHMMSS     19
#define ipicMDYHM      20
#define ipicMMDD       21

#define ipicXLMax		21
#define ipicMPMax		25

#define fRwRel 0x8000
#define fColRel 0x4000
#define grbitFRel (fRwRel|fColRel)
#define grbitRw 0x3fff

#define ptgRefN 0x2c
#define ptgAreaN 0x2d

typedef struct
		{
		unsigned char grbit;
		unsigned char grbitPli;
		char chKey;
		unsigned char cch;
		unsigned char cce;
		unsigned char rgce[1];
		}
	RTNAME;     /* rtLbl */

typedef struct
		{
		unsigned long phLblNext;
		unsigned char cch;
		union
			{
			unsigned long phTemp;
			unsigned long phLblSh;
			unsigned long phlblad;
			unsigned long phlbleval;
			struct
				{
				int wTemp;
				unsigned char bTemp;
				};
			};
		unsigned char cce;
		unsigned long phlbluse;
		struct
			{
			unsigned fChart:1;
			unsigned fProc:1;
			unsigned fCalcExp:1;
			unsigned fPad:1;
			unsigned fCopySh:1;
			unsigned fRef:1;
			unsigned fValue:2;
			} grbit;
		unsigned char rgce[2];
		}
	ELBL;   /* Our name for a Biff LBL */

/* Macro for accessing a grbit as an lbl.grbit */
#define PlblFromPgrbit(pgrbit)  ((ELBL *)((unsigned char *)(pgrbit)-((int)&((ELBL *)0)->grbit)))

/*---------- Multiplan Structures -------------------------------------*/

typedef struct
	{
	unsigned    bp:12,
				icty:4;
	} mpCP;

typedef struct
	{
	unsigned    cwExp:8,
			fProtected:1,
			format:3,
			digits:4,
			cbVal:8,
			fDeleted:1,
			fCommon:1,
			fChanged:1,
			justify:3,
			valtype:2,
			fUnMark:1,
			fUnk:1,
			fNewexp:1,
			fLinkSib:1,
			rwRet:12,
			colRet:8,
			bopdRet:7,
			fChangedSib:1;
	}
ENT20;


typedef struct
	{
	unsigned    row0,       /* starting row of reference        */
		row1,               /* lim row of reference             */
		col0,               /* starting column of reference     */
		col1;               /* lim column of reference          */
	}
EREF;                       /* Extended Reference. Same as REF, */
	/* but capable of handling sheets larger than 63 by 255.    */


typedef struct
	{
	unsigned        ht;
	unsigned        cw;
	unsigned        cbUsed;
	}
HDS20;


typedef struct
	{
	unsigned        ht,
			cw;
	}
HEAPHEAD20;

typedef struct
	{
	unsigned namlen : 5,
				refcnt : 5,
				fMacro : 1,
				dead1 : 5,
				szval : 13,
				dead2 : 3,
				chFirst : 8,
				chSecond : 8,
				fNewlbl : 1,
				dead3 : 15;
	}
LBL20;

typedef struct
	{
	int             r0, r1;
	CHAR            c0, c1;
	}
REF20;

/*---------- Multiplan Cell Types -----------------------------------------*/

#define EXNUMBER        0
#define EXSTRING        1
#define EXERROR         2
#define EXUNDEF         3       /* Used both for blank cells and booleans.*/



/*---------- Multiplan Cell Error Values ----------------------------------*/

/* These also just conveniently happen to be the indices into the table of  */
/* error strings, and thus should not be changed lightly                    */
/* WARNING!! Multiplan assumes that EXERR_REF is the fourth error           */

#define EXERR_NULL      0
#define EXERR_DIV0      EXERR_NULL+7
#define EXERR_VAL       EXERR_DIV0+8
#define EXERR_REF       EXERR_VAL+8
#define EXERR_UDF       EXERR_REF+6
#define EXERR_OVER      EXERR_UDF+7
#define EXERR_NA        EXERR_OVER+6



/*---------- Multiplan Alignment Definitions ------------------------------*/

#define bentFirst       1           /* Blank byte at beginning of entry table */
#define blblFirst       1           /* Blank byte at beginning of label table */

#define BLBLFIRST12     2
#define BENTFIRST20     sizeof(HDS20)

/*---------- MP Binary File Constants -------------------------------------*/

#define FNSIZE          31
#define ICTYMAX         16

/* constants for 2.0 files      */
#define cbEID20         3
#define cbRWHEAD20      8
#define CbFExp20(fExpr) ((unsigned)((fExpr)?8:4))
/* This is the reserved sector for the BCD register */
#define posReg20 8

/*---------- MP Binary File Offsets ---------------------------------------*/

#define ichMaxPrint 20              /* size of print setup              */
#define ichMaxPrint20 31            /* size of print setup */

#define iwnMax      8               /* max numberof windows in MP       */

/* offsets for 2.0 files */
#define ibRgsz20    (13*2)
#define ibRwMac20   (8*FNSIZE+ibRgsz20)
#define ibmpfo20    (936 + ichMaxPrint20 + cbBCDReg20)
#define ibmprwfo20  (1131 + ichMaxPrint20 + cbBCDReg20)
#define cbBCDReg20  185 + ichMaxPrint20 - 31
#define ibFmtDef20  (0x3bf)     /* Position of default cell info */


#define FMTDEFAULT  0
#define FMTEXTENDED 1
#define FMTEXPONENT 2
#define FMTFIXED    3
#define FMTGENERAL  4
#define FMTDOLLAR   5
#define FMTGRAPH    6
#define FMTPERCENT  7
#define FMTIGNORED  8

#define FMTDATEMIN	12
#define FMTMMDDYY 	12		/* biff date formats */
#define FMTDDMMMYY 	13
#define FMTDDMMM 	14
#define FMTMMMYY 	15
#define FMTHHMMAP 	16
#define FMTHHMMSSAP	17
#define FMTHHMM 	18
#define FMTHHMMSS 	19
#define FMTMDYHM 	20
#define FMTMMDD 	21
#define FMTDATEMAX	22
#define FMTError	255

typedef {
	int             ierefMac;
	EREF            rgeref[20];
	}
	SEQEREF;

typedef	struct 
	{
	int fn;
	FC  fc;
	FC  fcMac;
	}
	STM;

#define cwMPGlobals  (112)
#define BaseYearOffset  1461    /* 1904 - 1900 - 1 in days */
#define DaySpreadsheetMax  65381 /* Excel shows ######## */
#define BaseYear  1900
#define tOff 0
#define tAM  1
#define tPM  2
