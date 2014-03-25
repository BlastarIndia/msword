/*      types:
				idb : index data block
				hte : hash table entry (currently an HPIDB)
				fpgn: formatted page number
				pnb : page number block
*/

/*  Necessary constants:  */

#define cLevels         7             /*  Number of levels possible  */
				/*  The code is written so that cLevels can be  */
				/*  changed and the program will still work.  However,  */
				/*  the data structure will waste a word per entry when  */
				/*  cLevels=2 and two words per entry when cLevels=1  */

#define ihteBuckets     127           /*  Length of the hash table  */
				/*  Note: ihteBuckets should be prime!!  */

#define ifpgnMax        4             /*  Number of fpgn's per block  */
				/*  Note: ifpgnMax must be at least 2  */

#define cchMaxEntry     256  /* maximum length of index entry string */
#define cchMaxSuggest    64  /* max length of suggested entry (from sel) */
#define cchNil          -1

#define chSQuote        '\''

#define ihteNil         -1
#define pgnNil          -1

#define fpgnNil         4095           /*  NIL value for a 12-bit INT */

/* Here are some funny nfc's. nfcRange in a FPGN entry indicates that the
	previous pgn was the first of a pair, and this is the second (the nfc
	used to format is the same as previous).

	nfcExplicit is used for explicit references. these always start a new block
	of FPGNs. 4 words are used (see EXT structure)
*/
#define nfcRange        15               /*  NFC to indicate a range  */
#define nfcExplicit     14               /*  explicit reference, not pgn */
#define nfcError        13               /*  error in XE field */
#define nfcIndexMin     13               /*  change this if you add nfc's */

/*  Makeup of memory areas:  */

/* W A R N I N G!! Bit order dependency: this structure has
	bitfields reversed on Mac.  Code depends on pgn being the
	high order 12 bits of the word. */
struct  FPGN                            /*  Formatted Page Number  */
		{
		uns nfc : 4;     /* bit order dependency!! */
		uns pgn : 12;
		};

#define fpgnEmpty       0xfff0          /* unused fpgn entry */

/* explicit reference - same as 2 fpgn's */
struct EXT
		{
		union
				{
				struct FPGN fpgn;
				struct
						{
						/* W A R N I N G:  bit order dependency! */
						uns : 4;
						uns cch : 12;   /* ch's in reference text */
						};
				};
		int doc;                        /* location of reference text */
		CP cp;
		};
#define cbEXT   sizeof(struct EXT)
#define cchMaxExt   4095

typedef struct PNB HUGE *HPPNB;
struct PNB                              /*  Page Number Block  */
		{
		struct FPGN rgfpgn[ifpgnMax];
		int rgwSequence[ifpgnMax];     
/* (grpf & (1<<2*n)) is bold for rgfpgn[n]; (grpf & (1<<2*n+1)) is italic */
		uns grpf;
		struct PNB HUGE *hppnbNext;
		};

#define cbPNB   sizeof(struct PNB)

/* extra flags */
#define bitBold         1
#define bitItalic       2
#define bitRangeBegin   4
#define bitRangeEnd     8
#define bitExplicit     16


/* Note: the doc and cp stored in IDB are superfluous for now, though
	we will want to use them later for pictures, so I've left them in
	the structure, along with the code that stores them */

typedef struct IDB HUGE *HPIDB;
struct  IDB                             /*  Index Data Block  */
		{
		struct PNB      pnb;
		HPIDB           hpidbSub;
		HPIDB           hpidbNext;
		int             doc;
		CP              cp;
		uns             fQuoted : 1;
		uns             fRange : 1;
		uns             : 6;
		uns             cch : 8;
		char            rgch[];
		};

#define cbIDB   sizeof(struct IDB)

/*  Macro for managing the data structure. */
/*  (note that this is not always used instead of in-line code,
		only when the macro is equally efficient)  */

#define HpidbFromIhte(ihte)     ((*prghpIndex)[ihte])


/* Radio button indexes */

#define iTypeNested    0
#define iTypeRunin     1

#define iHeadingNone   0
#define iHeadingBlank  1
#define iHeadingLetter 2

/* Huge pointer stuff */
#define cwHP    (sizeof(HP) / sizeof(int))

#define cchMaxIndexArgs  8  /* string built up by dialog function - must
								allow for maximum string "\r \h A" (7 chars)
								includes cch */

#define chIndexLetter  'A'  /* Default picture string for heading letters */

/* these are pretty arbitrary - can be increased if need be */
#define cchMaxSeparator  4
#define cchMaxPartial    6
#define cchMaxHeading    33

#define rdNormal  0
#define rdError   1
#define rdAbort   2

/* Also in toc.h - must match */
#define sdeIndex   0
#define sdeToc     1
#define sdeOutline 2


struct IIB      /* Index Info Block - all the global stuff */
	{
	int  docMain;            /* doc to run index on */
	int  fRunin       : 1;   /* \r switch was specified */
	int  fPartial     : 1;   /* \p switch was specified */
	int  fSequence    : 1;   /* \s switch was specified */
	int  iHeading     : 2;   /* heading letters: 0 none, 1 blank, 2 letter */
	int  fHeadingLast : 1;   /* true if last thing output was a heading */
	int  fTable       : 1;   /* index is in a table */
	int   : 8;
	int  docIndexScratch;    /* used for EXT's and pictures */
	CP   cpIndexScratch;
	int  istErrIndex;
	int  pgnError;           /* page number where we encounter first error */
	CHAR chMinPartial;       /* range for partial index */
	CHAR chMaxPartial;
	CHAR szSequence[cchSequenceIdMax];     /* sequence id */
	CHAR stListSep[cchMaxSeparator];     /* separators... */
	CHAR stEntrySep[cchMaxSeparator];
	CHAR stSeqSep[cchMaxSeparator];
	CHAR stRangeSep[cchMaxSeparator];
	CHAR stHeading[cchMaxHeading];  /* heading letter string */
	};
