typedef long CP;
typedef long FC;
typedef unsigned uns;
typedef int BOOL;
typedef unsigned PN;

#define fFalse              0
#define fTrue               1

#define cbSector          512
#define cbBuffMax           cbSector

/* for complex part */
#define clxtPrc             1
#define clxtPlcpcd          2

#define READ_ERR_HDR        0
#define SCE_FN_NOT_GIVEN    1
#define BAD_CMD_LINE_ARG    2
#define FILE_NOT_FND        3
#define SEEK_ERR_HDR        4
#define SEEK_ERR_TEXT       5
#define SEEK_ERR_CHP        6
#define READ_ERR_CHP        7
#define BAD_SPRM_CODE       8
#define BAD_SPRM_LENGTH     9
#define SEEK_ERR_PAPX      10
#define READ_ERR_PAPX      11
#define SEEK_ERR_STSHF     12
#define READ_ERR_STSHF     13
#define BAD_STSH_LENGTH    14
#define BAD_STTB_LENGTH    15
#define ERR_NAME_BMAX      16
#define ERR_CHPX_BMAX      17
#define ERR_PAPX_BMAX      18
#define SEEK_ERR_ASSOC     19
#define READ_ERR_ASSOC     20
#define SEEK_ERR_BKMK      21
#define READ_ERR_BKMK      22
#define BAD_PLC_LENGTH     23
#define READ_ERR_TEXT      24
#define SEEK_ERR_SED       25
#define READ_ERR_SED       26
#define SEEK_ERR_PGD       27
#define READ_ERR_PGD       28
#define SEEK_ERR_HDD       29
#define READ_ERR_HDD       30
#define SEEK_ERR_DOP       31
#define READ_ERR_DOP       32
#define SEEK_ERR_PLCBTE    33
#define READ_ERR_PLCBTE    34
#define SEEK_ERR_CLX       35
#define READ_ERR_CLX       36



#define OCTAL               8
#define DECIMAL            10
#define HEX                16


struct SPRMDEF
	{
	unsigned char *pSprmName;
	int cbSprmArg;
	};


struct CA
		{
		CP      cpFirst;
		CP      cpLim;
		int     doc;
		};



struct DTTM    /* Opus internal date format */
		{
#ifdef REVIEW
		union
			{
			long lDttm;
			struct
				{
				unsigned mint:    6; /* 0-59 */
				unsigned hr:      5; /* 0-23 */
				unsigned dom:     5; /* 1-31 */
				unsigned mon:     4; /* 1-12 */
				unsigned yr:      9; /* (1900-2411)-1900 */
				unsigned wdy:     3; /* 0(Sun)-6(Sat) */
				};
			};
#else
		long lDttm;
#endif /* REVIEW */
		};

struct DOP
		{                   /* add new fields to the end of structure,
								it is part of file! */
		unsigned int   fFacingPages : 1;
		unsigned int   fWidowControl : 1;
		unsigned int   : 3;
		unsigned int   fpc : 2;
		unsigned int   fWide:1;  /* True == landscape. */
/* bits are right flush 2,1,0, set for each ihdt*Ftn that has its own ihdd */
		unsigned int   grpfIhdt : 8;

		unsigned int   fFtnRestart : 1;
		unsigned int   nFtn : 15;

		unsigned int   irmBar : 8;
		unsigned int   irmProps : 7;
		unsigned int   fRevMarking : 1;

		unsigned int   fBackup : 1;    /* make backup on save */
		unsigned int   fExactCWords : 1;
		unsigned int   fPagHidden : 1;
		unsigned int   fPagResults : 1;
		unsigned int   fLockAtn : 1;
		unsigned int   fMirrorMargins : 1;
		unsigned int   :10;

		unsigned int   fSpares:16;


/* measurements */
		uns   yaPage;
		uns   xaPage;
		int   dyaTop;
		uns   dxaLeft;
		int   dyaBottom;
		uns   dxaRight;
		uns   dxaGutter;  /* == dxaInside */
		uns   dxaTab;
		uns   wSpare;
		uns   dxaHotZ;
		uns   rgwSpare[2];

/* document summary info */
		struct  DTTM    dttmCreated;
		struct  DTTM    dttmRevised;
		struct  DTTM    dttmLastPrint;
		int             nRevision;
		long            tmEdited;
		long            cWords;
		long            cCh;
		int             cPg;
		int             rgwSpareDocSum[2];

		};

/*  This FIB is for opus nFib... */
#define nFibVersionMin  25
#define nFibVersionLast 33

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
		int     cbStshfOrig;

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

		FC      fcPlcffldMom;   /* field plc, mother doc */
		uns     cbPlcffldMom;

		FC      fcPlcffldHdr;   /* field plc, header/footer doc */
		uns     cbPlcffldHdr;

		FC      fcPlcffldFtn;   /* field plc, footnote doc */
		uns     cbPlcffldFtn;

		FC      fcPlcffldAtn;   /* field plc, annotation doc */
		uns     cbPlcffldAtn;

		FC      fcPlcffldMcr;   /* field plc, macro doc */
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
		PN      pnChpFirst;
		PN      pnPapFirst;

		PN      cpnBteChp;
		PN      cpnBtePap;

/* end of 1-1 correspondence to QSIB */

		};


#define cbFIB (sizeof (struct FIB))



struct CHP
		{
/* word 0: */
		unsigned int     fBold : 1;
		unsigned int     fItalic : 1;
		unsigned int     fStrike : 1;
		unsigned int     fOutline : 1;

		unsigned int     fFldVanish : 1;

		unsigned int     fSmallCaps : 1;
		unsigned int     fCaps : 1;
		unsigned int     fVanish : 1;
		unsigned int     fRMark : 1;
		unsigned int     fSpec : 1;
/* style modify bits for multi-bit fields */
		unsigned int     fsIco : 1;
		unsigned int     fsFtc : 1;
		unsigned int     fsHps : 1;
		unsigned int     fsKul : 1;
		unsigned int     fsPos : 1;
		unsigned int     fsSpace : 1;

/* word 1: */
		uns     ftc;
/* word 2: */
/* font size in half points */
		unsigned int     hps:8;
/* 2's complement signed number in half point units. >0 means superscript */
/* 256 - hpsPos is the absolute value for negative numbers */
		unsigned int     hpsPos : 8;
/* word 3: */
/* following field encodes in quarter point units the space following
the character in the range -7 though +56 q.p.'s. */
		unsigned int     qpsSpace : 6;
		unsigned int     wSpare2 : 6;
		unsigned int     ico : 4;
		unsigned int     kul : 3;
/* the following bit is used only internally to communicate from sprm's
to FetchCp */
		unsigned int     fSysVanish : 1;
/* word 4-5: */
/* words to be used by fSpecial characters for their properties
		chPicture:      fc of the picture body
		WARNING: MACINE DEPENDENT and order is significant!
			fnPic is high byte of fcPic and fDirty is highbit of
				entire long FC
*/
#ifdef REVIEW
		union {
				union {
						struct {
							int      dummy1;
							unsigned int     dummy2: 8; 
							unsigned int     fnPic: 8;       /* fn/fc coexist */
							};
#endif /* REVIEW */
						FC      fcPic;
#ifdef REVIEW
						};
				struct {
						int     docPic;
						unsigned int     dummy : 15;
						unsigned int     fDirty : 1;
						};
				};
#endif /* REVIEW */
		};


#define cbCHP sizeof(struct CHP)


/*
Paragraph properties
*/
/* paragraph height, fC = 1 */
#ifdef REVIEW            /* see new phe definition following */
struct PAPH
		{
		union {
				struct {
						unsigned int     fH : 1;
						unsigned int     clMac : 7;
						unsigned int     ps : 8;
						}S1;
				struct {
						unsigned int     fH : 1;
						unsigned int     dyaHeight : 15;
						}S2;
				struct {
						unsigned int     fValid : 8;
						unsigned int     ps : 8;
						}S3;
				int fStyleDirty;
				}U1;
		};
#endif

/* paragraph height, version for Word 4.0 */
struct PHE
		{
		unsigned int        fSpare : 1;
		unsigned int     fUnk : 1;       /* phe entry is invalid */
		unsigned int     : 14;
		unsigned int     fDiffLines : 1; /* total height is known, lines are different */
		unsigned int     dxaCol : 15;
		union   {
				/* when !fDiffLines, this variant is used. */
				struct  {
						unsigned int     clMac : 8;
						unsigned int     dylLine : 8;
						} S1;
				/* when fDiffLines, this variant is used */
				uns     dyaHeight;      /* WIN version of dylHeight */
				/* when papxs are stored in STSH this variant is used */
				int     fStyleDirty;
				} U1 ;
		};
#define cbPHE (sizeof(struct PHE))
#define cwPHE (cbPHE / sizeof(int))

#define cbMaxGrpprl  384 /* REVIEW  cbSprmSimple + (cbCHP + 1) + (cbPAP + 2) */

struct PAPX
		{
		char    cw;     /* Number of bytes stored in rest of PAPX */
		char    stc;
		struct PHE phe;    /* Used to be paph */
		unsigned char grpprl[cbMaxGrpprl];
		};


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

#define ibstNil -1
#define cwSTTBBase (offset(STTB, rgbst) / sizeof(int))
#define cbSTTB (sizeof(struct STTB))
#define cwSTTB (sizeof(struct STTB)/ sizeof(int))


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
		int     fExternal;
		struct ESTCP dnstcp[1];
		};

/* style sheet state */
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


struct PRM
		{ /* PropeRty Modifier */
		union
				{
				int     prm;
				struct  {
						unsigned int     fComplex : 1; /* == fFalse */
						unsigned int     sprm : 7;
						unsigned int     val : 8;
						}S1;
				struct
						{
						unsigned int     fComplex : 1; /* == fTrue */
/* We can't store the hprc because we have only 15 bits. Instead we do:
		pprc = (struct PRC *) *(pwMax - prm.cfgr);
This result is invariant even as the finger table expands toward lower
addresses */
						unsigned int     cfgrPrc : 15;
						}S2;
				}U1;
		};

struct PCD
		{
		unsigned int     fNoParaLast : 1;
		unsigned int     fPaphNil : 1;
		unsigned int     : 6;
		unsigned int     fn : 8;
		FC       fc;
		int      prm;                      /*struct PRM*/ 
		};

#define cbPCD (sizeof (struct PCD))


/* page table */

struct PGD
		{
		union {
				struct {
						unsigned int     : 5;
						unsigned int     fGhost : 2;		/* blank page or footnote-only page */
						unsigned int     : 9;
						} S1;
				struct {
						unsigned int     fContinue : 1;	/* ftn only: cont. from prev page */
						unsigned int     fUnk : 1;      /* dirty */
						unsigned int     : 1;
						unsigned int     fRight : 1;     /* right-hand side page */
						unsigned int     fPgnRestart: 1; /* start with page 1 again */
						unsigned int     fEmptyPage : 1;	/* section break forced empty */
						unsigned int     fAllFtn : 1;		/* nothing but footnotes */
						unsigned int     : 1;
						unsigned int     bkc      : 8;   /* section break code */
						} S2;
				} U1;
		uns     lnn;    /* line number of 1st line, -1 if no line numbering */
		int     cl;     /* count of lines into paragraph for 1st line */
		uns     pgn;    /* page number as printed */
/* number of cp's considered for this page; note that the CPs described
	by dcpDepend in rgpgd[0] reside on page described by rgcp[1] */
		int        dcpDepend;
		};

#define cbPGD (sizeof(struct PGD))

#ifdef REVIEW
						/* This def replaced by def above */
struct PGD
		{
		union {
				struct {
						unsigned int     fUnk1 : 1;
						unsigned int     fUnk2 : 1;
						unsigned int     : 14;
						}S1;
				struct {
						unsigned int     fUnks    : 2;   /* dirty */
						unsigned int     fPending : 1;   /* this page affects next */
						unsigned int     fRight   : 1;   /* right-hand side page */
						unsigned int     fPgnRestart: 1; /* start with page 1 again */
						unsigned int     : 3;
						unsigned int     bkc      : 8;   /* section break code */
						}S2;
				}U1;
		uns     lnn;    /* line number of 1st line, -1 if no line numbering */
		int     cl;     /* count of lines into paragraph for 1st line */
		int     pgn;    /* page number as printed */
		};
#define cbPGD (sizeof(struct PGD))
#endif

/* section (division) table */

struct SED
		{
		unsigned int     fSpare : 1;
		unsigned int     fUnk : 1;
		unsigned int     fn : 14;
		FC      fcSepx;
		};
#define cbSED  (sizeof (struct SED))
#define cwSED (cbSED / sizeof (int))


/* formatting bin */
#define cbFkp   (512/*cbSector*/ - sizeof(FC) - 1)
struct FKP
		{
		FC      rgfc[1]; /* crun + 1 entries from fcFirst to fcLim */
		/*char  rgb[1];   crun entries based on FKP, points to chpx or papx*/
		char    rgb[cbFkp];
		char    crun;   /* number of runs */
		};
#ifdef MAC
#define ifcFkpNil (-1)        /* native code depends on this */
#else
#define ifcFkpNil (512 /*cbSector*/ / sizeof(FC))
#endif

#define cbFkpPre35   (128/*cbSector*/ - sizeof(FC) - 1)
struct FKPO
		{
		FC      rgfc[1]; /* crun + 1 entries from fcFirst to fcLim */
		/*char  rgb[1];   crun entries based on FKP, points to chpx or papx*/
		char    rgb[cbFkpPre35];
		char    crun;   /* number of runs */
		};

/* There are never more than three of these: 2 global for the scratch file
		(chp and pap), and one for the file we are currently writing. */
struct FKPD
		{ /* FKP Descriptor (used for maintaining insert properties) */
		int     bFreeFirst;   /* offset to next run to add */
		int     bFreeLim;       /* offset to byte after last unused byte */
		PN      pn;     /* pn of working FKP in scratch file */
		FC      fcFirst;
		int     fPlcIncomplete; 
		struct  CHP     chp;
		};

struct FKPDP
		{ /* subset for Paras */
		int     bFreeFirst;
		int     bFreeLim;
		PN      pn;
		FC      fcFirst;
		};

struct FKPDT
		{ /* subset for Text */
		int     bFreeFirst;     /* first unused byte on page pn if != pnNil */
		int     bFreeLim;       /* not used */
		PN      pn;             /* if != pnNil, pn of partially full page */
		FC      fcLim;          /* end of last written rgch. */
		};


#ifdef COMMENT
/* since the following "stuctures" are not word aligned in an FKP,
the definition here is just a comment */
/* Character properties encoded as a differential */
struct CHPX
		{
		char    cb;     /* Number of bytes stored in chp */
		struct CHP chp;
		};

/* Paragraph properties encoded as a differential */
struct PAPX
		{
		char    cb;     /* Number of bytes stored in rest of PAPX */
		char    stc;
		int     paph;
		struct PRL grpprl;
		};
#endif


