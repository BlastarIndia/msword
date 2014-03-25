/* F I L E . H */

#define fnScratch 1

/* Max number of files that may be open at one time */
#define cfcbOpenMax     10 /* WIN only */

#define osfnNil (-1)
#define volNil  0

#define fcMax	    	((FC)(32l*1024l*cbSector)) /* based on pn limitation */
#define cpWarnTooBig 	((CP)fcMax)

/* size of Word file header. cbFIB expanded to 128byte boundry */
#define cbFileHeader    (((cbFIB + 127) >>7) <<7)

/* F I B  V E R S I O N S */

/* Early MAC version history:
	0  used FIB30
	1  moved to current FIB format.
	2  removed restriction that fcClx + cbClx is the current cbMac and
		started to store current cbMac in fib.cbMac.
	3  next file stored in sttbFlc
	4  changed cbPGD
	5  changed WSS because SELS grew
	6  added fib.fHasPic
	7  added fib.cpnBteChp and fib.cpnBtePap
	8  changed phe structure
	9  changed length in PAPXs stored in FKPs to be a count of words
	10 thru 24 skipped to catch up to OPUS nFibCurrent
	25 adopt OPUS nFibCurrent
*/

/* Fib version numbers */
/* We write files with version Current.  We read files with
	nFib >= nFibMin or whose nFibBack <= nFibCurrent.
*/
	/* history:
		WIN Only:
		0   all previous versions
		1   pnNext and fGlsy added (pj 4/1/87)
		2   cbMac added (pj 4/15/87)
		3   added mcr stuff (bac 4/23/87)
		4   added second bkmk plc, rearranged (pj 5/1/87)
		5   field plc completely reworked (pj 5/27/87)
		6   changed to "status" byte, added nLocal (pj 8/05/87)
		7   changed sttbfAssoc & dop location (pj 08/24/87)
		8   changed order of character sprm's, nukes complex files
				(dsb 8/26/87)
		9   used spares for ccpAtn, fcPlcfandRef, cbPlcfandRef, fcPlcfandTxt
				cbPlcfandTxt (ccc 9/3/87)
		10  use fcSpare2 and cbSpare2 for Print Environment gunk (dsb 9/87)
	11  Mac Word Code Merge (jbl 12/11/87)
	12  Added PlcffldMom/Hdr/Ftn/Atn/Mcr (pj 12/29/87)
		13  removed printer/port/driver from hsttbAssoc (dsb 2/8/88)
	14  cWords in Dop grew by 2 bytes. (yxy 3/4/88)
	15  incremented to avoid problems caused by bugs pre rel n(pj 4/6/87)
		16  fib rearranged some w/new merge (pj 4/21/88)
	17  macro format changed (bac 6/5/88)
		18  annotation format changed (pj 6/15/88)
		19  sprmCPlain changed (pj 6/17/88)
		20  invalidate old macros (pj 7/10/88)
	21  added fib.fHasPic (dll 7/14/88)
	22  added fib.cpnBteChp and fib.cpnBtePap (dll 7/18/88)  
		23  changes to PHE structure (dsb 7/28/88)
		24  invalidate old macros (tdk 8/8/88)
		MAC Only:
		0   used FIB30
		1   moved to current FIB format.
		2   removed restriction that fcClx + cbClx is the current cbMac and
				started to store current cbMac in fib.cbMac.
		3   next file stored in sttbFlc
		4   changed cbPGD
		5   changed WSS because SELS grew
		6   added fib.fHasPic
		7   added fib.cpnBteChp and fib.cpnBtePap
		8   changed phe structure
		9   changed length in PAPXs stored in FKPs to be a count of words
		10-24 skipped to catch up to OPUS
		Common:
	25  change length of PAPXs stored in FKP to be a count of words
			instead of a count of bytes (dll 8/22/88).
	26  Mac Word hsttbFlc stored as sttbf; cbExtra sttb's allowed on file
				(cm ????)
		27  invalidate old macros (pj 9/24/88)
		28  Opus Beta version (pj 11/18/88)
		29  Opus version 2806 (pj 02/02/89)
		30  Opus version 3018 (pj 03/15/89)
		31  changed Opus font structure (pj 08/07/89)
		32  added fib.pnChpFirst and fib.pnPapFirst (dl 08/15/89) 
		33  added fib.cQuickSaves (pj 9/22/89)
	*/
#define nFibCurrent  33

/* change these numbers to invalidate old formats */
#define nFibMinDot   27

		/* YOU MUST TALK TO PETERJ BEFORE CHANGING THESE VALUES! */
#define nFibMinDoc	18 /* identifies readable files */
#define nFibBackCurrent 25 /* current can't be read by creators before this */



/* F I L E   C A C H E   P A R A M E T E R S   A N D   C O N S T A N T S */

/* A cbSector refers to the page size used in the file cache.  The
	length of formatted files are always in increments of cbSector. */
#define cbSector                512
#define cwSector                (cbSector / sizeof (int))
#define shftSector              9
#define maskSector              0x01ff

/* the sector size used in versions of Word previous to version 3.5 */
#define cbSectorPre35           128
#define shftSectorPre35         7
#define maskSectorPre35         0x007f
#define shftPoPn                2

#define ibpNil                  (-1)
/* largest permissible value for a time stamp (unsigned integer) */
#define tsMax                   ((unsigned) 65535)
/* Hash function used for locating proper cached file page */
#define IibpHash(fn, pn, iibpHashMax) (((fn) * 33 + (pn) * 5) & ((iibpHashMax) - 1))


/* Number of internal cache pages which cannot hold scratch file contents
unless vmerr.fDiskEmerg is set.
*/
#define cbpReserve              5

#ifdef MAC
#define wMagic          0xfe37          /* Magic word for Mac Word 4.0 */
#define wMagic30        0xfe34          /* Magic word for Mac Word 3.0 */
#define wMagicPrd       0xfe32          /* PRD file magic word */
#endif /* MAC */
#ifdef WIN
/* Magic word for Opus */
#define wMagic          0xA59B
#define wMagicPmWord    0xA59C
#endif /* WIN */

/* typedefs for locked-down far pointers to buffer page caches */
typedef CHAR (far *LPRGBPEXT) [cbSector];
/* unlocked huge pointer to same */
typedef CHAR (HUGE *HPRGBPEXT) [cbSector];

struct BPTB
	{
/* number of pages in cache */
	int     ibpMax;
/* current number of pages may be less than max, during storage brownouts */
	int     ibpMac;
/* size of hash lookup table */
	int     iibpHashMax;
/* location of hash table */
	int HUGE *hprgibpHash;
/* location of buffer page descriptors (each BPS contains info about what
	is stored in a particular page in the cache - fn, pn, etc.) */
	struct BPS HUGE *hpmpibpbps;
/* where the cached pages' contents are stored. */
#ifdef MAC
	char (HUGE *HUGE *hqrgbpExt)[cbSector];
#endif
	char (HUGE *hprgbpExt)[cbSector];
	int     cbBp; /* cbSector */
	uns	cbpChunk;/* allocation chunk for rgbp; chunks past
							the first are in sequential sb's */
/* Most Recently Used Buffer Page timestamp; used to
	implement a LRU strategy for swapping cached pages. */
	TS      tsMruBps;
#ifdef WIN
	uns	cqbpspn;  /* number of quarter sectors in a span */
	TS	HUGE *hpmpispnts;     /* time stamps for span */
#endif
	};


/* Buffer Page Status - describes contents of a cache page */

struct BPS
	{
	PN      pn;
/* will we have to write this page out to disk? */
	unsigned char fDirty;
	unsigned char fn;
/* time stamp for this cache page - updated whenever page is used.
	When ts is compared to the tsMruBps field in a BPTB, the result is the
	cache page's "age" since it was last used. */
	TS      ts;
/* The hash function takes fn and pn as inputs.  If more than one cached page
	happens to hash to the same slot in the hash table, ibpHashNext is used
	to implement a linked-list. */
	int     ibpHashNext;
	};

#define cbBPS (sizeof (struct BPS))

/* bin tables */
struct BTE
	{
	PN      pn;
	};
#define cbBTE (sizeof (struct BTE))
#define cwBTE (cbBTE / sizeof (int))

struct PLCBTE
	{
	uns     ibteMac;
	uns     ibteMax;
	uns     cb;
	};

#ifdef WIN

struct OFH  {       /* Open File structure: This parallels Windows OFSTRUCT
						but w/o the filename field. */
		int     ofDummy1: 8;
		int     fFixedDisk: 8;          /* non-zero if file located on non- */
										/* removeable media */
		WORD    nErrCode;               /* DOS error code if OpenFile fails */
		BYTE    rgbTimeDate [4];        /* File Time/Date */
		};

struct OFS  {       /* Open File structure: This parallels Windows OFSTRUCT. */
		int     ofDummy1: 8;
		int     fFixedDisk: 8;          /* non-zero if file located on non- */
										/* removeable media */
		WORD    nErrCode;               /* DOS error code if OpenFile fails */
		BYTE    rgbTimeDate [4];        /* File Time/Date */
		CHAR    szFile [120];
		};

#endif /* WIN */


struct FCB      /* file control block */
	{
	int     fHasFib: 1;
#ifdef MAC
	int     fRefDocScrap : 1;
	int     fRefNonScrap : 1;
#else
		int     fRef : 1;
		int     : 1;
#endif /* MAC */
	int     fDict : 1;
	int     fGlsy : 1;
	int     fDoc : 1;
	int     fTemp : 1;
	int	fReadOnly : 1; 	/* true iff read-only file */
#ifdef MAC
	int     fDontCloseOsfn : 1;     /* osfn cannot safely be closed */
	int     fRequestReadOnly: 1;    /* read only BY REQUEST */
	int	fPrevSave: 1;	/* used by event record/replay featue */
	int     fWord3File : 1; /* true when file is in Word 3.0 format */
	int	fMailFile : 1;	/* True if this file was read in by mail */
	int   fDirty : 1; 
	int     : 2;
#else
	int     fCompound: 1;   /* true if fn contains two docs (2 fibs) */
	int	fOpened : 1;	/* true means use OF_REOPEN flag */
	int	fDMEnum: 1;
	int	fForeignFormat: 1;
	int   fDirty : 1; 
	int	: 3;
#endif
	FC      cbMac;          /* logical end of file */
#ifdef WIN
		FC      fcMacFile;      /* physical end of file (use in filewin.c only!) */
#endif /* WIN */
	FC      fcPos;
	int     osfn;   /* operating system file number */
#ifdef MAC
/* volume reference number of disk where this file is stored. */
	int     vol;
#endif
/* time stamp - used to implement a LRU strategy for opened files.  We
	want to have a maximum of cfcbOpenMax files actually opened in the
	Mac file system at any time.  See OsfnFileOpen in file.c */
	TS      ts;
/* A disk I/O error on a (cbSector) page with a page # less than this quantity
	is recoverable, an error retrieving a page with a page # equal or greater
	than the below quantity is an unrecoverable error.
	For the scratch file, this quantity should be zero - a disk I/O error
	anywhere in the file is unrecoverable.
	For unformatted files, this quantity will be a max integer.
*/
	PN      pnXLimit;
/* reference to (fc -> pn of FKP record containing CHP's) mapping table */
	struct PLCBTE   **hplcbteChp;
/* reference to (fc -> pn of FKP record containing PAP's) mapping table */
	struct PLCBTE   **hplcbtePap;
#ifdef WIN
/* structure passed to Windows OpenFile call, less name name (stored below) */
	struct OFH ofh;
#endif
/* last pn and ifc retrieved by FetchCp */
	PN     pnChpHint;
	int    ifcChpHint;

/* last pn and ifc retrieved during loop to find caPara.cpFirst in CachePara*/
	PN     pnPapHint1;
	int    ifcPapHint1;

/* last pn and ifc retrieved during loop to find caPara.cpLim in CachePara */
	PN     pnPapHint2;
	int    ifcPapHint2;
	FC     fcMaxText;       /* ccpText at time of save */
	int    nFib;        /* fib version number for file */
	int    wSpare1;   /* spare for future use */
	char   stFile[2];  /* expands to size of file name when FRenameFn
					is called. Guaranteed to be <= ichMaxFile in size. */
	};

#define cwFCB	(sizeof (struct FCB) / sizeof (int))

#define cchStFileNextMax 31


#ifdef MAC      /* ********** MAC ********** */
struct FIB
	{
/* wIdent is magic number. Is 0xfe37 for MacWord4, 0xfe34 for MacWord3,
	0xfe32 for MacWord1, 0xbe31 for PC Word3 */
	int     wIdent;
	int     nFib;         /* fib version written */
	int     nProduct;     /* product version written by*/
	int     nLocale;      /* language stamp--localized version */
/* note: compound files are an OPUS construct which we may adopt in a
	future version. */
/* compound files are files where two complete docs have been appended
together.  In fact that is a simplification, the second doc is written
after the first complete with its own FIB.  But the first will contain
some information about the second.  Notes: they share common plcbtes
(written with the second) and will have the same cbMac (end of second).
Compound documents cannot be complex.  pnNext is the page on which the FIB
of the second starts.  */
	PN      pnNext;         /* OPUS:if has file appended, where it starts */
	int     fDot : 1;       /* OPUS: file is a DOT */
	int     fGlsy : 1;      /* OPUS : file is a glossary co-doc */
	int     fComplex : 1;   /* file piece table/etc stored (FastSave) */
	int     fDefaultFormat : 1; /* recommend this file format next time
												document is saved. */
	int     fHasPic : 1;            
	int     : 11; 				/* other *file* status bits */

		int     nFibBack;       /* how backwards compatiable is this format? */

	int     rgwSpare1[3];

/* fcMin allows new structures to be stored in front of the text.*/
		FC      fcMin;

/* points to last char in file + 1 after simple save,
not changed during complex save. Last chars must be rgchEol */
		FC      fcMac;

/* last character written to file (same in both fibs if compound) */
		FC      cbMac;

		FC      fcSpare;

/* the initial document is represented by fcMin through fcMac if !fComplex
otherwise by the piece table at pnComplex.
The initial document comprises all text that is stored. It must be subdivided
into several documents:
		Main
		Footnotes
		Headers
		Macros
		Annotations
*/
		CP      ccpText;
		CP      ccpFtn;
		CP      ccpHdd;
	CP      ccpMcr;      /* OPUS only */
	CP      ccpAtn;      /* OPUS only */
		CP      ccpSpare2;
		CP      ccpSpare3;

	FC      fcStshfOrig;    /* original allocation for STSH in file */
	uns     cbStshfOrig;

/*      code in quicksave.c depends on the order of the fields from
	fcStshf up to fcSpare. If these fields are rearranged the
	quicksave.c code should be changed also. */

	FC      fcStshf;        /* style sheet */
	uns     cbStshf;


	FC      fcPlcffndRef;    /* footnote reference table */
	uns     cbPlcffndRef;

	FC      fcPlcffndTxt; /* footnote text table */
	uns     cbPlcffndTxt;

	FC      fcPlcfsed;      /* section table */
	uns     cbPlcfsed;

	FC      fcPlcfpgd;      /* page table */
	uns     cbPlcfpgd;

	FC      fcSttbfglsy;    /* glossary sttb */
	uns     cbSttbfglsy;

	FC      fcPlcfglsy;     /* glossary table */
	uns     cbPlcfglsy;

	FC      fcPlcfhdd;      /* header table */
	uns     cbPlcfhdd;

	FC      fcPlcfbteChpx;  /* bin tables */
	uns     cbPlcfbteChpx;

	FC      fcPlcfbtePapx;  /* bin tables */
	uns     cbPlcfbtePapx;

	FC      fcPlcfsea;
	uns     cbPlcfsea;

	FC      fcRgftc;
	uns     cbRgftc;

	FC      fcPrr;
	uns     cbPrr;

	FC      fcPlcfphe;   /* current paragraph heights */
	uns     cbPlcfphe;

		FC      fcDop;          /* document properties sheet */
		uns     cbDop;

	FC      fcSttbFlc;	/* associated strings */
	uns     cbSttbFlc;

	FC      fcWss;          /* window save state */
	uns     cbWss;

	FC      fcClx;          /* complex part */
	uns     cbClx;

	FC      fcPlcfpgdFtn;   /* footnote page table */
	uns     cbPlcfpgdFtn;

	PN  cpnBteChp; 
	PN  cpnBtePap; 
	int     rgwSpare2[34];
	};
#define cbFIB (sizeof (struct FIB))
#define cwFIB (cbFIB / sizeof (int))



/* DOP30 and SELS30 are snapshots of the DOP and SELS made at the time
	those structures were moved out of the FIB. The interesting values are
	cbDOP30 and cbSELS30. These values are used when we translate a FIB30
	into a FIB. */
struct DOP30
	{
	uns     yaPage;
	uns     xaPage;
	int     dyaTop;
	uns     dxaLeft;
	int     dyaBottom;
	uns     dxaRight;
	uns     dxaGutter;
	uns     dxaTab;
	int     fFacingPages : 1;
	int     fWidowControl : 1;
/* identifies the printer with which the para heights are associated; this
	plus fBestQuality below are used to invalidate all heights */
	int     pridPaph : 3;
	int     fpc : 2;
	int     fWide : 1;
/* bits are right flush 2,1,0, set for each ihdt*Ftn that has its own ihdd */
	char    grpfIhdt;

	int     fFtnRestart : 1;
	int     nFtn : 15;
	int     nLnn;
	int     pgn;
	union
		{
		int     wEndBase;
		struct
			{
/* the following are used with pridPaph to invalidate all para heights when
	printer changes */
			int     fDontUse : 1; /* used to be fBestQuality : 1; */
/* copy of vpref.fSeeHidden for last repag */
			int     fSeeHidden : 1;
			int     fTallAdj : 1;
/* the following are used with pridLaser */
			int     fNotSmooth : 1;
			int     fNotFontSubst : 1;
			int     fBackup : 1;    /* make backup on save */
			int     fFracWidth : 1; /* fractional widths enabled */
			int     prReduce : 9;   /* for pridLaser: 0 or 100 means 100 */
			};
		};
	};
#define cbDOP30 (sizeof (struct DOP30))

struct SELS30
	{
/* means selection is marked in a special way because it is a picture
or the like */
	union {
		struct {
			int     fSpecial : 8;
			int     fIns : 8;       /* set iff cpFirst == cpLim */
			};
		struct {
			int fGraphics : 1;
			int fBlock : 1;
			int fRightward : 1;
			int fNil : 1;
			int : 4;
			int : 8;
			};
		};
/* direction of selection, used for choosing anchor for extend */
	int     fForward : 8;
/* if on, the insertion is drawn after the last char of line (dl-1)
rather than before the first char of line dl (same cp position!) */
	int     fInsEnd : 8;

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
	int     xpFirst;
	int     xpLim;
	CP      cpFirstLine;
	};

#define cbSELS30 (sizeof (struct SELS30))


/* Word 3 fib */
struct FIB30
	{
	int     wIdent; /* magic number XFE34 (XFE35 in W1) */
	int     w1;
	int     w2;     /* magic number AB00 */
	int     w3;
	int     w4;
	int     fComplex;       /* =1 iff piece table/etc is stored */
	int     fDefaultFormat; /* when file is saved again propose same file
										format */
/* points to last char in file + 1 after simple save,
not changed during complex save. Last char must be a 0D chEol */
	FC      fcMac;
	int     w9;
	int     w10;
	int     w11;

	FC      fcPlcfphe;   /* current paragraph heights */
	int     cbPlcfphe;

	FC      fcStshfOrig;    /* original allocation for STSH in file */
	int     cbStshfOrig;

/*      code in quicksave.c depends on the order of the fields from
	fcStshf up to fcSpare. If these fields are rearranged the
	quicksave.c code should be changed also. */

	FC      fcStshf;        /* style sheet */
	int     cbStshf;


	FC      fcPlcffndRef;    /* footnote reference table */
	int     cbPlcffndRef;

	FC      fcPlcffndTxt; /* footnote text table */
	int     cbPlcffndTxt;

	FC      fcPlcfsed;      /* section table */
	int     cbPlcfsed;

	FC      fcPlcfpgd;      /* page table */
	int     cbPlcfpgd;

	FC      fcSttbfglsy;    /* glossary sttb */
	int     cbSttbfglsy;

	FC      fcPlcfglsy;     /* glossary table */
	int     cbPlcfglsy;

	FC      fcPlcfhdd;      /* header table */
	int     cbPlcfhdd;

	FC      fcPlcfbteChpx;  /* bin tables */
	int     cbPlcfbteChpx;

	FC      fcPlcfbtePapx;  /* bin tables */
	int     cbPlcfbtePapx;

	FC      fcPlcfsea;
	int     cbPlcfsea;

	FC      fcRgftc;
	int     cbRgftc;

	FC      fcPrr;
	int     cbPrr;

	FC      fcClx;          /* complex part */
	int     cbClx;

/* fcMin allows new structures to be stored in front of the text. Should
be kept at X80 for as long as possible for W1 compatibility.
*/
	FC      fcMin;
/* the initial document is represented by fcMin through fcMac if !fComplex
otherwise by the piece table at pnComplex.
The initial document comprises all text that is stored. It must be subdivided
into several documents:
	Main
	Footnotes
	Headers
*/
	CP      ccpText;
	CP      ccpFtn;
	CP      ccpHdd;
	CP      ccpSpare;
	CP      ccpSpare2;
/* last selection */
	struct SELS30 sels;
	int     rgwSelsSpare[3];
/* document properties */
	struct DOP30 dop;
	int     rgwDopSpare[13];
	char    stFileNext[cchStFileNextMax + 1];
	};
#define cbFIB30 (sizeof (struct FIB30))
#define cwFIB30 (cbFIB30 / sizeof (int))



/* open file codes */
#define ofcDoc          1
#define ofcGlsy         2
#define ofcTemp         3
#define ofcDict         4
#define ofcOther        5       /* hyphenation */

struct BKINFO
	{	/* Backup information structure */
	char stBak[cchMaxFile + 32]; /* Name of backup file */
	long lCreation; /* Creation date */
	int fCanUndoBackup;
	int fSetFinderInfo;
	int fn; /* if not fnNil, fn of existing file of same name */
/* the following has the same structure as FInfo in a GetFileInfo IOB
	(see toolbox.h) */
	union
		{
		int rgwFInfo[8];
		struct
			{
			long ftg;       /* Four character file type tag */
			long sig;
			int grpf;
			struct PT ptInFolder;
			int iFolder;    /* Folder number */
			};
		};
/* end of FInfo */
	};

/* file type tags */
#define ftgWDBN ((long)0x5744424e)      /* normal word docs */
#define ftgGLOS ((long)0x474c4f53)      /* glossary */
#define ftgPICT ((long)0x50494354)      /* pictures (scrap type only) */
#define ftgRTF  ((long)0x52544620)      /* RTF (scrap type only) */
#define ftgTEXT ((long)0x54455854)      /* text docs */
#define ftgWPRD ((long)0x57505244)      /* printer descriptor file */
#define ftgWTMP ((long)0x57544d50)      /* temp file */
#define ftgWORD ((long)0x574f5244)      /* MacWrite file */
#define ftgWHLP ((long)0x57484c50)      /* help file */
#define ftgDICT ((long)0x44494354)      /* user dictionary */
#define ftgDCTU ((long)0x44435455)      /* user dictionary */
#define ftgDCT5 ((long)0x44435435)      /* main dictionary */
#define ftgWSET ((long)0x57534554)      /* settings file */
#define ftgBINA ((long)0x42494e41)      /* standard binary format */
#define ftgAWWP ((long)0x41575750)      /* Works */
#define ftgPNTG ((long)0x504e5447)      /* MacPaint file */
#define ftgLINK ((long)0x4c494e4b)      /* link info (scrap type only) */

/* application signatures */
#define sigWORD1 ((long)0x574f5244)     /*  Word 1.0's application signature */
#define sigWORD ((long)0x4d535744)      /*  Word's application signature */
#define sigMACA ((long)0x4d414341)      /*  MacWrite's application signature */

/* disk error codes */

#define ecDiskFull      (-34)


/* file conversion message numbers (used with StartConvertDlg) */
#define icvmMacWrite	0
#define icvmWorks       1
#define icvmRtf         2
#define icvmPCWord      3
#define icvmWord1       4
#define	icvmCmdList     5
#define icvmExtScrap    6


#else           /*********** WIN *************/


/*  ***** IF YOU CHANGE THE FIB ***** */
/* Change the fib-version numbers above (top of file)!! */
struct FIB
		{
		uns     wIdent;         /* magic number */
		uns     nFib;           /* fib version as written */
		uns     nProduct;       /* product version written by */
		uns     nLocale;        /* language stamp--localized version */

/* compound files are files where two complete docs have been appended
together.  In fact that is a simplification, the second doc is written
after the first complete with its own FIB.  But the first will contain
some information about the second.  Notes: they share common plcbtes
(written with the second) and will have the same cbMac (end of second).
Compound documents cannot be complex.  pnNext is the page on which the FIB
of the second starts.  */
		PN      pnNext;         /* if has file appended, where it starts */
		uns     fDot : 1;       /* file is a DOT */
		uns     fGlsy : 1;      /* file is a glossary co-doc */
		uns     fComplex : 1;   /* file piece table/etc stored (FastSave) */
		uns     fHasPic : 1;    /* one or more graphics in file */
		uns     cQuickSaves : 4;/* count of times file quicksaved */
		uns     : 8;
		uns     nFibBack;       /* how backwards compatiable is this format? */

		uns     rgwSpare0 [5];

/* fcMin allows new structures to be stored in front of the text.*/
/* FIB is defined to extend from pnFib to fcMin */
		FC      fcMin;

/* points to last char in file + 1 after simple save,
not changed during complex save. Last chars must be rgchEol */
		FC      fcMac;

/* last character written to file (same in both fibs if compound) */
		FC      cbMac;

		FC      fcSpare0;
		FC      fcSpare1;
		FC      fcSpare2;
		FC      fcSpare3;

/* the initial document is represented by fcMin through fcMac if !fComplex
otherwise by the piece table at pnComplex.
The initial document comprises all text that is stored. It must be subdivided
into several documents:
		Main
		Footnotes
		Headers
		Macros
		Annotations
*/
		CP      ccpText;
		CP      ccpFtn;
		CP      ccpHdd;
		CP      ccpMcr;
		CP      ccpAtn;
		CP      ccpSpare0;
		CP      ccpSpare1;
		CP      ccpSpare2;
		CP      ccpSpare3;

		FC      fcStshfOrig;    /* original allocation for STSH in file */
		uns     cbStshfOrig;

/* code in quicksave.c depends on the order of the fields from
here up to the end. If these fields are rearranged the
quicksave.c code should be changed also. */

		FC      fcStshf;        /* style sheet */
		uns     cbStshf;

		FC      fcPlcffndRef;   /* footnote reference table */
		uns     cbPlcffndRef;

		FC      fcPlcffndTxt;   /* footnote text table */
		uns     cbPlcffndTxt;

		FC      fcPlcfandRef;   /* annotation reference table */
		uns     cbPlcfandRef;

		FC      fcPlcfandTxt;   /* annotation text table */
		uns     cbPlcfandTxt;

		FC      fcPlcfsed;      /* section table */
		uns     cbPlcfsed;

		FC      fcPlcfpgd;      /* page table */
		uns     cbPlcfpgd;

	FC      fcPlcfphe;      /* current paragraph heights */
	uns     cbPlcfphe;

		FC      fcSttbfglsy;    /* glossary sttb */
		uns     cbSttbfglsy;

		FC      fcPlcfglsy;     /* glossary table */
		uns     cbPlcfglsy;

		FC      fcPlcfhdd;      /* header table */
		uns     cbPlcfhdd;

		FC      fcPlcfbteChpx;  /* bin tables */
		uns     cbPlcfbteChpx;

		FC      fcPlcfbtePapx;  /* bin tables */
		uns     cbPlcfbtePapx;

		FC      fcPlcfsea;
		uns     cbPlcfsea;

		FC      fcSttbfffn;     /* font Table */
		uns     cbSttbfffn;

		FC      fcPlcffldMom;      /* field plc, mother doc */
		uns     cbPlcffldMom;

		FC      fcPlcffldHdr;      /* field plc, header/footer doc */
		uns     cbPlcffldHdr;

		FC      fcPlcffldFtn;      /* field plc, footnote doc */
		uns     cbPlcffldFtn;

		FC      fcPlcffldAtn;      /* field plc, annotation doc */
		uns     cbPlcffldAtn;

		FC      fcPlcffldMcr;      /* field plc, macro doc */
		uns     cbPlcffldMcr;

		FC      fcSttbfbkmk;    /* bookmark sttb */
		uns     cbSttbfbkmk;

		FC      fcPlcfbkf;      /* bookmark plc of cpFirsts */
		uns     cbPlcfbkf;      

		FC      fcPlcfbkl;      /* bookmark plc of cpLims */
		uns     cbPlcfbkl;

		FC      fcCmds;         /* command table */
		uns     cbCmds;

		FC      fcPlcmcr;       /* macro text (only in document types) */
		uns     cbPlcmcr;

		FC      fcSttbfmcr;     /* macro names (only in document types) */
		uns     cbSttbfmcr;

		FC      fcPrEnv;        /* print environment */
		uns     cbPrEnv;

		FC      fcWss;          /* window save state */
		uns     cbWss;

		FC      fcDop;          /* document properties sheet */
		uns     cbDop;

		FC      fcSttbfAssoc;   /* associated strings */
		uns     cbSttbfAssoc;

		FC      fcClx;          /* complex part */
		uns     cbClx;

		FC      fcPlcfpgdFtn;
		uns     cbPlcfpgdFtn;

		FC      fcSpare4;
		uns     cbSpare4;

		FC      fcSpare5;
		uns     cbSpare5;

		FC      fcSpare6;
		uns     cbSpare6;

		int     wSpare4;
		PN		pnChpFirst; 	
		PN      pnPapFirst;


			PN  cpnBteChp; 
			PN  cpnBtePap; 

/* end of 1-1 correspondence to QSIB */

		};

/*  ***** IF YOU CHANGE THE FIB ***** */
/* Change the fib-version numbers above (top of file)!! */

#define cbFIB (sizeof (struct FIB))
#define cwFIB (cbFIB / sizeof (uns))

/* indices into table of associated strings */
/* #define ibstAssocFileNext   0 *//* unused */
/* code in create.c depends on order of dot->comments */
#define ibstAssocDot        1
#define ibstAssocTitle      2
#define ibstAssocSubject    3
#define ibstAssocKeyWords   4
#define ibstAssocComments   5
#define ibstAssocAuthor     6
#define ibstAssocLastRevBy  7
#define ibstAssocMax        8

/* open file codes */
#define ofcDoc 1
/* #define ofcDot 2  */
#define ofcTemp 3
#define ofcDMEnum 4

/* Flags for FnOpenSt */

#define fOstCreate          0x01
#define fOstSearchPath      0x02
#define fOstReportErr       0x04
#define fOstFormatted       0x08
#define fOstReadOnly        0x10
#define fOstQuickRead       0x20
#define fOstNativeOnly      0x40
#define fOstNamed           0x80

/* FnOpenSt error return values (order important) */
#define foseCantOpenAny 1    /* memory, fn space limits */
#define foseBadFile     2    /* old fib/future fib */
#define foseNoCreate    3
#define foseNoAccess    4


/*  Magic numbers for other formats */
#define	wMagicMPV20	((unsigned)0xEC08)   /* MP 2.0 binary magicword */
#define	wMagicMPV30	((unsigned)0xED08)   /* MP 3.0 binary magicword */
#define wrtBegin	        0x0000  /* Wks */
#define rtBOF       9	    	/* also in ff.h */

#define chDOSWildAll    '*'  /* DOS File name wild card. */
#define chDOSWildSingle '?'

#ifdef INSFILE
typedef struct
		{
		CHAR **hstFile;
		int    iDirList;
		CHAR **hstRange;
		BOOL   fLink;
		} CABINSFILE;
#endif


struct EFPI
	/* (Element) File Path Identifier */
	{
	uns fInit : 1;
	uns fNone : 1;
	uns hcPath : 14; /* handle stored with HcCompactH() */
	};

#define fpiDotPath  	0
#define fpiIniPath  	1
#define fpiUtilPath 	2
#define fpiProgram   	3
#define fpiCurrent  	4
#define fpiOwn	    	5

#define fpiNil	    	0xf
#define fpiStoreMax	(fpiProgram+1)

#define Grpfpi(fpi1,fpi2,fpi3,fpi4)    (((fpi1)&0xf) | (((fpi2)&0xf)<<4) | \
										(((fpi3)&0xf)<<8) | (((fpi4)&0xf)<<12))

#define grpfpiUtil Grpfpi(fpiUtilPath, fpiProgram, fpiCurrent, fpiNil)
#define grpfpiIni  Grpfpi(fpiIniPath, fpiCurrent, fpiProgram, fpiNil)
#define grpfpiDot  Grpfpi(fpiOwn, fpiDotPath, fpiProgram, fpiCurrent)
#define grpfpiOwn  Grpfpi(fpiOwn, fpiNil, fpiNil, fpiNil)

/* Macro Language structures & constants */

struct ELOF	/* Extended Language Open File structure */
{
	HANDLE hFile;	/* DOS File handle of open stream */
	struct OFS ofs;/* structure returned by OpenFile */
	int wMode;		/* Open mode, OF_READ or OF_WRITE */
	long lcch;		/* file position (for files on floppy disks) */
};

#define cbELOF	(sizeof(struct ELOF))

#define stmMin		1	/* Stm constants for files open by Macro language */
#define stmLast	4
#define stmMax		5
#define stmMinM1	0
#define stmMaxM1	stmLast

#endif /* WIN */

/* file cache page limit block */
struct PLB
	{
	int	fnDest;	 /* protected fn for saving, or fnNil if none */
	int	fNoTsNonDest;  /* don't update tsMru on fns != fnDest */
	};


/* fast io block buffer */
struct FBB
	{
	int		fnDest; /* fn to write to */
	uns		cchBuf;	/* size of io buffer */
	HQ		hqBuf;	/* handle to io buffer */ 
	FC		fcDest; /* current position to append to */
	uns		bMac;	/* size of data in buffer */
	};


/* M A C R O S  */



/* H P B P S  H P  I B P  - returns far addr of bps corresponding to ibp */
/* macro used for speed */
#define HpbpsHpIbp(hpmpibpbps, ibp)  \
	(&((struct BPS HUGE *)(hpmpibpbps))[ibp])



#ifdef WIN
#define PfcbFn(fn)	((struct FCB *)N_PfcbFn(fn))
#endif /* WIN */
