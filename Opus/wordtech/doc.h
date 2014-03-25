/* D O C . H */
/* communication between docCreateFn and its callers.  Indicates the USER
	opted to cancel the create. */
#define docCancel       -1

#define fnInsert  0
#define fnSpec  0
/* within fnInsert, fc's larger than ccpInsert have special meanings:
	1. Eop to terminate document
	2. piece to be substituted for section of piece table when there is
		no room.
*/
#define fcSpecEop ((FC)100)
/* the next fc is ccpEop larger to allow for the existence of fcSpecEop+1 */
#define fcSpecNoRoom ((FC)102)

#define cchInsertMax    32      /* Length of quick insert block */

/* buffer to keep capitalized text for small caps or caps */
#define ichCapsMax 20

/* number of insertion points remembered */
#define iselsInsertMax 4

/* FetchCp Modes.
Particular values are necessary for hand-native code */
#define fcmChars        0x8000
#define fcmProps        0x80
#define fcmParseCaps    1  /* enforce fCaps and fSmallCaps */

/* generic PLex */
struct PL
	{
	int     iMac;
	int     iMax;
	int     cb;
	int     brgfoo;
	int	fExternal;
	char    rgbHead[1];
	/*    char    rgfoo[]; */
	};
#define cbPLBase (offset(PL, rgbHead))

/* PL in settings file */
struct PLE
	{
	int	iMac;	
	int	brgfoo;
	int	cb;		/* cb of rgbHead + rgfoo */
	};
#define cbPLE	sizeof(struct PLE)


struct PLW
	{
	int     iMac;
	int     iMax;
	int     cb;
	int     brgw;
	int	fExternal;
	int     rgw[1];
	};

struct PLCE
	{
	CP      rgcp[1];
	/* struct FOO rgfoo[] */
	};

/* generic PLC */
struct PLC
	{
	uns     iMac;
	uns     iMax;
	int     cb;
	uns     icpAdjust;
	CP      dcpAdjust;
	uns     icpHint;
	int	fExternal : 1;
	int	fExtBase : 1;
	int	fGrowGt64K : 1;
	int	fMult : 1;
	int	: 12;
	union   {
		HQ      hqplce;   /* when fExternal true */
		CP      rgcp[1];            /* when fExternal false */
		/*FOO   rgfoo[]*/
		};
	};
#define cbPLCBase (sizeof (struct PLC) - sizeof(CP))

/* macro PLC */
typedef struct _mcr
		{
		uns     bcm;
		} MCR;

#define cbMCR (sizeof (MCR))

/* piece descriptor */

struct PCD
	{
	int     fNoParaLast : 1;
	int     fPaphNil : 1;
#ifdef DEBUG
	int     : 5;
	int     fNoParaLastValid: 1;
#else /* not DEBUG */
	int     : 6;
#endif /* DEBUG */
	unsigned char fn;
	FC      fc;
	int/*struct PRM*/ prm;
	};
#define cbPCD (sizeof (struct PCD))
#define cwPCD (sizeof (struct PCD) / sizeof (int))


/* footnote table:
FND is <empty>, rgcp in the plc carries all the information. In a fDoc doc
the cp's are references, in fFtn doc, the cp's are footnote bounds.
*/
#define cbFND 0

struct FRD
	{
	int     fAuto;
	};
#define cbFRD  (sizeof (struct FRD))


/* annotation reference description */
struct ATRD 
		{
		CHAR stUsrInitl[ichUsrInitlMax];
		};

#define cbAND 0
#define cbATRD (sizeof (struct ATRD)) 




/* section (division) table */

struct SED
	{
	int     fSpare : 1;
	int     fUnk : 1;
	int     fn : 14;
	FC      fcSepx;
	};
#define cbSED  (sizeof (struct SED))
#define cwSED  (sizeof (struct SED) / sizeof(int))

/* PLCs with empty foos */
#define cbHDD 0
#define cbBKL   (0)

/* Aldus properties table */
struct SEA
	{
	char    rgch[6];
	};
#define cbSEA  (sizeof (struct SEA))
#define cwSEA  (sizeof (struct SEA) / sizeof(int))


/* page table */

#define ipgdNil         -1

struct PGD
	{
	union {
		struct {
			int     : 5;
			/* blank page or footnote-only page */
			int     fGhost : 2;
			int     : 9;
			};
		struct {
			int	fContinue : 1;	/* ftn only: cont. from prev page */
			int     fUnk : 1;	/* dirty */
			/* the following flags are not used for footnotes */
			int     : 1;
			int     fRight : 1;     /* right-hand side page */
			int     fPgnRestart: 1; /* start with page 1 again */
			int     fEmptyPage : 1; /* section break forced empty */
			int     fAllFtn : 1;    /* nothing but footnotes */
			int     : 1;
			int     bkc      : 8;   /* section break code */
			};
		};
	uns     lnn;    /* line number of 1st line, -1 if no line numbering */
	int     cl;     /* count of lines into paragraph for 1st line */
	uns     pgn;    /* page number as printed */
/* number of cp's considered for this page; note that the CPs described
	by dcpDepend in rgpgd[0] reside on page described by rgcp[1] */
	int	dcpDepend;
	};
#define cbPGD (sizeof(struct PGD))
#define cwPGD (sizeof(struct PGD) / sizeof(int))


/* outline table */

struct PAD
	{
	struct {
		int	fInTable : 1;
		int     fUnk : 1;
		int     fShow : 1;
		int     fBody : 1;
		int     lvl : 4;
		int     stc : 8;
		};
	};

#define cbPAD (sizeof(struct PAD))

/* live field information */
/* struct FLD define in field.n */

/*  BooKmark cpFirst table */
struct BKF
	{
	int ibkl;   /* index of this bkmk's cpLim in plcbkl */
	};

#define cbBKF   (sizeof (struct BKF))

struct UAB
	{ /* UNDO Action Block */
	int     uac;
	int     fRedo;
	int     fExt;
	int     fOutlineDirty;  /* call to DirtyOutline() required */
	union {
		int     ucm;    /* MAC version */
		int     bcm;    /* WIN synonym */
		};
	int     sabt;   /* scrap type */
	int     grpfDirty;
	int     fFormatted ;	/* redundant: also in grpfDirty, but needed by again */
	int     fNoUndo;
	int     fDelete; /* set after FSetUndoBefore if selection is deleted */
	int     wwBefore;
	int     wwAfter;
	struct CA       ca;
	struct CA       caMove;
	int     wwTo; /* used by uacMove */
	struct SELS     selsBefore;
	struct SELS     selsAfter;
	int	fOutlineInval;	/* inval according to ExpandOutlineCa */
	int     ucc;
		CP	cpFrom; /* used by uacMove */
	int	docInvalPageView; /* !=docNil if the operation does InvalPageView */
	int	fMayHavePic; 
		int     wUnused; 
	};

#define uacNil          0

#define uacCopy         1
#define uacMove         2
#define uacDel          3
#define uacScrap        4
#define uacUndo         5
#define uacUndoInval    6
#define uacUndoNil      7
#define uacMoveNil      8

#define uccCopy         1
#define uccMove         2
#define uccDeleteSel    3
#define uccFormat       4
#define uccPaste        5
#define uccInsert       6
#define uccCut          7

/* Again action codes */
#define accNil			0
#define accUcm			1
#define accDlgHcab		2
#define accDlgHcabGrpprl	3
#define accGrpprl		4
#define accProp			5
#define accInsert		6
#define accFindAgain		7
#define accDiadic		8
#define accGlsy			9
#define accOutline		10

/* Again menu string codes */
#define ascNil			0
#define ascFormat		1
#define ascEdit			2
#define ascChange		3
#define ascGlossary		4
#define ascFootnote		5
#define ascCalculate		6
#define ascRenumber		7
#define ascSort			8
#define ascHyphenate		9
#define ascQuickSwitch		10
#define ascCopy			11


#define cbMaxAgainGrpprl     512


struct AAB {
	/* Again action block */
	union {
		int     ucm;    /* MAC version */
		int     bcm;    /* WIN synonym */
		};
	int		acc;
	int	  	asc;	/* String code for again in menu */
	union {
		int     ucm2;    /* MAC version */
		int     bcm2;    /* WIN synonym */
		};
	CP      cpLow;
	union {
		struct CA	ca;
		struct {
			CP  cpFirst;
			CP  cpLim;
			int doc;
			};
		};

#ifdef WIN
	struct CA caAgain;  /* for dyadic copy, move */

#endif

	int 	iglsy;		/* Glossary entry */
	char    **hgrpprl;
	int     cbGrpprl;
	void    **hcab;
	};


/* properties of a stc */
struct ESTCP
	{
	char    stcNext;
	char    stcBase;
	};
#define cbESTCP (sizeof(struct ESTCP))
#define cwESTCP (sizeof(struct ESTCP) / sizeof(int))

struct PLESTCP
	{
	int     stcpMac;
	int     stcpMax;
	int     cb;
	int     bdnstcp;
	int	fExternal;
	struct ESTCP dnstcp[1];
	};

/* style sheet state, part of DOD */
struct STSH
	{
/* the number of standard stc's defined; not includung stcNormal.
Other arrays are indexed by (stc + cstcStd) mod 2^8 */
	int     cstcStd;
	struct STTB **hsttbName;
	struct STTB **hsttbChpx;
	struct STTB **hsttbPapx;
	struct PLESTCP **hplestcp;
	};
#define cbSTSH (sizeof(struct STSH))

/* style name matching */
struct CNK
	{
	int fMatchedOne : 1;
	int fMultipleNames : 1;
	int fMatchStd : 1;
	int fMismatch : 1;
	int fInvalid : 1;
	int : 11; 
	}; 

#define mtUpperLower 1
#define mtPartialMatch 2

/* Document Reference Plex set (footnote or annotation) */

struct DRP {
	int		doc;	/* subdoc to which hplc refers */
	struct PLC	**hplcRef;
	};

#define diNotDirty 0
/* Dirtyness Id Masks */
#define dimDoc   0x01
#define dimStsh  0x02
#define dimFtn   0x04
#define dimHdr   0x08
#define dimGlsy  0x10
#define dimRepag 0x20
#define dimAtn   0x40


#ifdef MAC
/* DoKument types */
#define dkDoc   0x8000
#define dkDot   0x4000
#define dkGlsy  0x2000
#define dkAtn   0x0080
#define dkMcr   0x0040
#define dkHdr   0x0020
#define dkFtn   0x0010
#define dkSDoc  0x0008
#define dkDispHdr 0x0004

/* NOTE: this needs to be changed when MAC gets templates */
#define udtNil		7	/* used to signify "not untitled" */
#endif /* MAC */

/*  Untitled Document Type */
#define udtInternal     0
#define udtMacroEdit    2
/* unused internal      4 */
/* unused internal      6 */
#define udtGlobalDot    1
#define udtDocument     3
#define udtFormLetter   5
#define udtTemplate     7

#ifdef WIN
/* DoKument types */
#define dkDoc   0x0001
#define dkDot   0x0002
#define dkGlsy  0x0004
#define dkAtn   0x0100
#define dkMcr   0x0200
#define dkHdr   0x0400
#define dkFtn   0x0800
#define dkSDoc  0x1000
#define dkDispHdr 0x2000
#endif /* WIN */

struct DOD
		{ /* Document Descriptor */
/* WARNING: Make sure you updated structs.inc if you changed anything */
		union {      /*  various way of looking at the type of the doc */
			struct {            /* LONG DOD TYPES */
				int  fDoc : 1;  /* normal document */
				int  fDot : 1;  /* document type */
				int  fGlsy : 1; /* glossary document */
				int  :1;
				int  : 4;
								/* SHORT DOD TYPES */
				int  fAtn : 1;  /* annotation document */
				int  fMcr : 1;  /* macro document */
				int  fHdr : 1;  /* header/footer document */
				int  fFtn : 1;  /* footnote document */
				int  fSDoc : 1; /* short DOD GP document */
				int  fDispHdr: 1; /* doc showing header/footer text */
				int  : 2;
				};   /* above are all mutually exclusive !! */
			struct {            /* size of the DOD */
				unsigned char fMother;/* is a mother doc, else mother is doc */
				unsigned char fShort;/* has a short DOD */
				};
			int dk;         /* single field for type of doc */
			};
/* WARNING: Make sure you updated structs.inc if you changed anything */
	union {
		struct {
				int     fDirty : 1;     /* Document has been edited */
				int     fStshDirty : 1; /* document's style sheet has been edited */
				int     fLRDirty : 1;   /* left/right margins changed, paph's bad */
				int     fOutlineDirty : 1;/* edit made in non-outline mode, plcpad bad */
				int     fMotherStsh : 1;/* use dod.doc's stsh */
				int     fFormatted : 1; /* on if document has been formatted */
				int     fSea : 1;       /* set if hplcsea is valid */
				int     fRepag : 1;     /* page table info's changed */
				int     fLockForEdit:1; /* true if this doc cannot be edited (WIN)*/ 
				int     fPromptSI:1;	/* Prompt for summary info at save (WIN)*/
				int     fGuarded : 1; /* end of doc protected with extra chEop.
											examples: docUndo, docScrap. */
				int     fSedMacEopCopied: 1; /* fTrue, when hidden section mark is
												copied to a fGuarded doc. */
				int     fEnvDirty : 1;  /* fTrue when environment changes or when
											doc is loaded. (WIN)*/
			int     fDocClosed : 1;	/* during quit, close was attempted (MAC)*/
				int     fFldNestedValid : 1; /* indicates hplcfld fNested bits valid */
			int     fDefaultFormat:1;/* recommend dod.sff as the file format to use */
			};
		struct {
			/* this includes some stuff that isn't really
				necessary for undo, but doesn't hurt -
				specifically, fMotherStsh, fSea */
			int	grpfDirty : 8;
			int	: 8;
			};
		};

	int	fHdrSame : 1;	/* same as previous set for header */
	int     fMayHavePic : 1;/* document may contain a picture */
	int	fGetHdr: 1;     /* get hdr text from docHdr again, used by fDispHdr doc */
		int     fInvalSeqLev:1; /* update all SEQLEV fields */
		int     fHasNoSeqLev:1; /* doc does not contain SEQLEV fields */
		int     fHadPicAlert:1; /* pic in doc gave out of memory msg */
	int     fStyDirtyBroken : 1; /* style dirty status in STSH was lost
					when recovering from OOM */
	int	fDoNotDispose: 1; /* set only for DocHdrDisp which is about to be 
					displayed, may or may not has ww point to it yet */
	int	: 8;
		int     wwDisp : 8;     /* start of chain of ww's displaying doc */
		int     fn : 8;         /* "Mother" fn of the doc, fnNil if new */
		CP      cpMac;          /* number of cp's in doc */
		int     doc;            /* Mother doc if !fMother */

/* WARNING: Make sure you updated structs.inc if you changed anything */
		struct  PLC     **hplcpcd;  /* Piece table */
	struct  PLC     **hplcphe;  /* current paragraph height state */
		struct  PLC     **hplcpgd;  /* page table */
		struct  PLC     **hplcfld;  /* live fields */
		union {
			struct PLC     **hplcfnd;   /* iff fFtn */
			struct PLC     **hplchdd;   /* iff fHdr */
			struct PLC     **hplcglsy;  /* iff fGlsy */
			struct PLC     **hplcmcr;   /* iff fMacro */
			struct PLC     **hplcddli;   /* for docDde (global) only */
			struct PLC     **hplcand;   /* iff fAtn */
			};
		int     crefLock;	/* document lock - prevents DisposeDoc */
		int     docHdr;	/* link to header document */

	union	{
		struct DRP	drpFtn;
		struct	{
				int		docFtn;
			struct PLC	**hplcfrd;
			};
		/* short docs only */
		struct {
			int	ised;	/* which section */
			int	ihdt;	/* type of header */
			};
		};
/*************************************************************/
/* end of short dod, following fields are in long dod's only */
/*************************************************************/

/* WARNING: Make sure you updated structs.inc if you changed anything */
	union	{
		struct DRP	drpAtn;
		struct	{
				int		docAtn;
			struct PLC	**hplcatrd;
			};
		};
		union   {
			struct {
				int     udt : 3;    /* if fn==fnNil, what is name */
				int     iInstance:13;/* instance # for Untitled1... */
				};
			struct {
				int     fUserDoc:1; /* if false, doc for internal use only */
				int     :15;
				};
			};
		struct  PLC     **hplcsed;  /* section table */
		struct  PLC     **hplcpad;  /* outline table */
		struct  STSH    stsh;       /* style sheet state */
		struct  STTB    **hsttbChpe;/* style sheet, expanded chp's */
		struct  STTB    **hsttbPape;/* style sheet, expanded pap's */
/* WARNING: Make sure you updated structs.inc if you changed anything */
#ifdef WIN
		struct  DTTM    dttmOpened; /* date/time stamp when opened */
		struct  STTB    **hsttbAssoc;/* Associated strings */
		struct  PLC     **hplcsea;  /* Aldus structure */
		struct  PLC     **hplcbkf;  /* Bookmark cpFirsts */
		struct  PLC     **hplcbkl;  /* Bookmark cpLims */
		struct  STTB    **hsttbBkmk;/* Bookmark names */
		/* IBSTFONT */
		CHAR    (**hmpftcibstFont)[];/* map font codes into master font table */
		int     ftcMac;
		int     ftcMax;
		union {
			int docDot;             /* iff fDoc */
			struct STTB **hsttbGlsy;/* iff fGlsy */
			struct {                /* iff fDot */
				int     docGlsy;
				KMP     **hkmpUser;
				int     **hmudUser; /* MUD **, don't want to include menu2 */
				int     docMcr;
				};
			};
	int lvl: 4;
	int wSparebits2: 12;   /* for future use */
#endif /* WIN */
#ifdef MAC
	int sff : 8;	/* file format used as default */
	int : 8;
	union {
		struct STTB **hsttbGlsy; /* glossary name table */
		struct PLC **hplcsea; /* Aldus structure */
		};
	/* PRR far *far * */ char far *far *qqprr;   /* print record */
	struct STTB **hsttbFlc;	/* file locators (e.g. next file) */
	int wSpare2;   /* for future use */
#endif /* MAC */
	int wSpare1;   /* for future use */
		struct  DOP     dop;        /* document props */
		};
/* WARNING: Make sure you updated structs.inc if you changed anything */


#define cbDOD (sizeof (struct DOD))
#define cwDOD (cbDOD / sizeof (int))
#define cbDODShort (offset(DOD, docAtn))
#define cwDODShort (cbDODShort / sizeof (int))

/* Macro to access the hmud field of the DOD */
#define HmudUserPdod(pdod)      ((MUD **) (pdod)->hmudUser)

/* Macro to access the dop field of the DOD starting from a doc (heap pointer!) */
#ifdef DEBUG
struct DOP *PdopDoc();
#else
#define	PdopDoc(doc) (&(PdodDoc(doc)->dop))
#endif


/* codes used to update hplc's.
NOTE: these are word displacements of the corresponding fields in a DOD
*/

#define edcNone 0
#define edcPhe  (offset(DOD, hplcphe) / sizeof(int))
#define edcSed  (offset(DOD, hplcsed) / sizeof(int))
#define edcPgd  (offset(DOD, hplcpgd) / sizeof(int))
#define edcSea  (offset(DOD, hplcsea) / sizeof(int))
#define edcFrd  (offset(DOD, hplcfrd) / sizeof(int))
#define edcPad  (offset(DOD, hplcpad) / sizeof(int))
#define edcAtrd (offset(DOD, hplcatrd)/sizeof (int))
#define edcFnd  (offset(DOD, hplcfnd) / sizeof(int))
#define edcHdd  -1       /* changed to edcFnd at creation */
#define edcMcr  -2       /* changed to edcFnd at creation */
#define edcGlsy -3       /* changed to edcFnd at creation */
#define edcDdli  -4       /* changed to edcFnd at creation */

#define edcDrpFtn	(offset(DOD,drpFtn)/sizeof(int))
#define edcDrpAtn	(offset(DOD,drpAtn)/sizeof(int))



#define clxtPrc 1
#define clxtPlcpcd 2

/* standard doc's */

#define docNew          1
#define docScrap        2
#define docUndo         3
#define docScratch      4
#define docMinNormal    5

/* scrap state */
struct SAB
	{
	int     docStsh;
	union
		{
		struct  {
			int     fExt : 1;
			int     fPict : 1;
			int     fBlock : 1;
			int     fFormatted : 1;     /* formatted text (styles etc.) */
			int     fTable : 1;

			int     fExtEqual : 1;
			int     fOwnClipboard : 1;        /* OPUS only */
			int     fDontDestroyClip : 1;     /* OPUS only */
			int     fMayHavePic : 1; 
			int	  fRows : 1;
			int     fSpare : 6;
			};
		struct  {
			int     sabt : 5; /* five bits above as sab type */
			int     : 11;
			};
		};
	union {                 /* OPUS: where scrap came from, for DDE LINK */
		struct CA caCopy;
		struct {
			CP  cpFirst;
			CP  cpLim;
			int doc;
			};
		};
#ifdef MAC
	char	**hstLink;	/* link info, if any */
#endif

	};
#define cwSAB   sizeof(struct SAB) / sizeof(int)

#define cfNil 0

#define	bpmCp	0
#define bpmFoo	1

/* fast plc cache (used for PutPlcLast) */
struct FPC
	{
	struct PLC **hplc;      /* the hplc in question */
	char	*pchFoo;	/* points to caller's local buffer */
	int	cbFoo;		/* size of foo */
#ifdef	MAC
	long	bfoo;		/* offset of foo relative to LprgcpForPlc */
#else /* WIN */
	uns	bfoo;		/* offset of foo relative to LprgcpForPlc */
#endif
#ifdef DEBUG
	int	ifoo;		/* used for validating the fpc */
#endif
	};

/* debug and non-debug macros for PutPlcLast */
#ifdef DEBUG
#	define PutPlcLast(h, i, pch)  PutPlcLastDebugProc(h, i, pch)
#else
#	define PutPlcLast(h, i, pch)  PutPlcLastProc()
#endif

#ifdef WIN
/* H A N D  N A T I V E */

#define PdodDoc(doc)    ((struct DOD *)(__FNATIVE__?N_PdodDoc(doc):C_PdodDoc(doc)))
#define PdodMother(doc) ((struct DOD *)N_PdodMother(doc))
#endif  /* WIN */

/* mode control for FDeleteableEop */
#define rpkNil 0
#define rpkText 1
#define rpkCa 2
#define rpkOutline 3
