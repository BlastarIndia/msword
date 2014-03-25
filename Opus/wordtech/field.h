/* F I E L D . H */
/* field related structures & definitions */


/* C O N S T A N T S */

/*  Misc constants */
/*  Many of these constants are (somewhat) arbitrary */
#define cchMaxFieldKeyword  cchNameMax
#define cchBackFetchChunk   32
#define ccrArgumentFetch    24
#define cchArgumentFetch    128
#define cNestFieldMax       20
#define ccpMaxRun           (0x7fff)
#define cchMaxPic           64
#define ccrMaxPic           16
#define cchSequenceIdMax    cchNameMax
#define cSwitchMax          10 /* more switches are ignored */

#define bDataInval          0xFF


/*  ifld values */
#define ifldNil             (-1)


/*  calculation Masks */
#define frmNone             0
#define frmUser             1
#define frmPrint            2
#define frmPaginate         4
#define frmHdrAll           8
#define frmHdr1st           16
#define frmMerge            32
#define frmDdeHot           64


/* F I E L D  F O R M A T  C O D E S */
#define ffcSkip         0
#define ffcShow         1
#define ffcDisplay      2
#define ffcBeginGroup   3
#define ffcEndGroup     4
#define ffcWrap         5
#define ffcRestart      6


/* F I E L D  C A L C U L A T O R  R E T U R N  V A L U E S */
#define fcrNormal       0   /* delete old result */
#define fcrNumeric      1   /* result is number (in vnumCalc) */
#define fcrDateTime     2   /* result is date/time (in vnumCalc) */
#define fcrError       -1   /* delete old result, no post formatting */
#define fcrKeepOld     -2   /* keep the old result, no post formatting */


/* F I E L D  T Y P E S */

/*  To add a field type:
		place a #define here.  For most flt's be sure it is >= fltMinParse
			and adjust fltMax.
		place a corresponding entry in dnflt below
		place the keyword into szgFltNormal (or single) in strtbl.h
	
	If the new field is of RESULT type:
		add a calculation function and include a pointer to the function in
			the dnflt entry
			
	If the new field is of DISPLAY type:
		add a case to the following switches:
			FFormatDisplay:  determine size of display
			DisplayField:  draw the display field to an hdc
			SelectDlPt:  actions for a mouse hit to display field
*/

/* WARNING: flts are part of the file format.  Do not change them. */

#define fltNil              -1
#define fltSys              0
#define fltMin              0
#define fltMinParse         3
#define fltMax              56  /* if you change this, recompile debugstr! */
								/* and change consts.inc! */

#define fltInfoMin          fltInfo
#define fltInfoMax          fltDot
#define fltInfoMaxRTF       fltNumChars

/* display flts -- not for real fields, but things that use similar code */
#define fltGraphics         255
#define fltMessage          254
#define fltFootnote         253
#define fltAnnotation       252

/* special flts */
#define fltUnknownKwd       1
#define fltPosBkmk          2

/* "normal" flts */
#define fltBkmkRef          3
#define fltXe               4
#define fltFtnRef           5
#define fltSet              6
#define fltIf               7
#define fltIndex            8
#define fltTce              9
#define fltStyleRef         10
#define fltRefDoc           11
#define fltSequence         12
#define fltToc              13
#define fltInfo             14  /* do not change order: Info->Dot */
#define fltTitle            15
#define fltSubject          16
#define fltAuthor           17
#define fltKeyWords         18
#define fltComments         19
#define fltLastRevBy        20
#define fltCreateDate       21
#define fltRevDate          22
#define fltPrintDate        23
#define fltRevNum           24
#define fltEditTime         25
#define fltNumPages         26
#define fltNumWords         27
#define fltNumChars         28
#define fltFileName         29
#define fltDot              30  /* end of do-not-change-order */  
#define fltDate             31  
#define fltTime             32  
#define fltPage             33
#define fltExp              34
#define fltQuote            35
#define fltInclude          36
#define fltPageRef          37
#define fltAsk              38
#define fltFillIn           39
#define fltPMData           40
#define fltPMNext           41
#define fltPMNextIf         42
#define fltPMSkipIf         43
#define fltPMRec            44
#define fltDde              45
#define fltDdeHot           46
#define fltGlsy             47
#define fltPrint            48
#define fltFormula          49
#define fltHyperText        50
#define fltMacroText        51
#define fltSeqLevOut        52
#define fltSeqLevLeg        53
#define fltSeqLevNum        54
#define fltImport           55

/* F I E L D  F O R M A T  F U N C T I O N S */
#define ffnUpper        0
#define ffnLower        1
#define ffnFirstcap     2
#define ffnCaps         3
#define ffnOrdinal      4
#define ffnRoman        5
/* ffnRoman+1 reserved */
#define ffnAlphabetic   7
/* ffnAlphabetic+1 reserved */
#define ffnCardtext     9
#define ffnOrdtext      10
#define ffnHex          11
#define ffnArabic       12
#define ffnDollartext   13
#define ffnCharFormat   14
#define ffnMergeFormat  15


/* F I E L D  P I C T U R E  T O K E N S */
	/* common tokens */  
#define fptCommonMin    20
#define fptEof          20
#define fptError        21
#define fptQuote        22
#define fptSequence     23
#define fptCommonMax    24
	/* numeric tokens */
#define fptReqDigit     10
#define fptOptDigit     11
#define fptTruncDigit   12
#define fptDecimal      13
#define fptThousand     14
#define fptNegSign      15
#define fptReqSign      16
	/* date/time tokens */
#define fptMonth1       30  /* 1-12 */
#define fptMonth2       31  /* 01-12 */
#define fptMonth3       32  /* Jan-Dec */
#define fptMonth4       33  /* January-December */
#define fptDay1         34  /* 1-31 */
#define fptDay2         35  /* 01-31 */
#define fptDay3         36  /* Sun-Sat */
#define fptDay4         37  /* Sunday-Saturday */
#define fptYear1        38  /* 00-99 */
#define fptYear2        39  /* 1900-2156 */
#define fptHour1        40  /* 1-12 */
#define fptHour2        41  /* 01-12 */
#define fptHour3        42  /* 0-23 */
#define fptHour4        43  /* 00-23 */
#define fptMinute1      44  /* 0-59 */
#define fptMinute2      45  /* 00-59 */
#define fptAMPMLC       46  /* am/pm, a/p */
#define fptAMPMUC       47  /* AM/PM, A/P */


/* I N F O  F I E L D  C O D E S */
/*  Sources */
#define ifcSttbAssoc    1
#define ifcDop          2
#define ifcEditTime     3
#define ifcDocName      4

/*  Formats */
#define ifcPst          1
#define ifcInt          2
#define ifcDate         3
#define ifcFileName     4
#define ifcLong         5

/* Destinations */
#define ifcOther        0
#define ifcRTF          1
#define ifcFieldCalc    2

#define cchMaxInfoObj   (sizeof (struct DTTM))



/* F E T C H  O P T I O N  C O D E S */
/* tell fetch code how to deal with special options (switches & nested fields) */
#define focNone         -1  /* pass switches as text, stop on nested fields) */
#define focStopOnSw     -2  /* stop when switch found */
#define focNormal       0   /* record switch & arg location & go on */
/* >0==> Stop & foc/(cSwitchMax+1) = iSysNext, foc%(cSwitchMax+1) = iFltNext */


/* F I E L D  T Y P E  G R O U P S */
/*  used to group field types for show results/instructions */

#define fltgAll             (-1)
#define fltgOther           (0)


/* these are used to read and set bits in a grpf */
#define FFromIGrpf(i,grpf)      (((1<<i) & grpf) ? fTrue : fFalse)


/*  These are hard-coded grpfShowResults's used for special purpose 
	display modes. They replace or are "and" masks for the field in 
	the pref structure. */

/*  Cut to Clipboard */
#define maskGrpfSRClipText  (0 | 1 << fltgOther) 

/*  Default */
#define grpfSRDefault       (0 | 1 << fltgOther)


/* F I E L D  V I S I B I L I T Y  C O D E S */
/*  these are used to determine what "visible" means WRT fields &
	hidden text */
#define fvcmFields  	    0x0100
#define fvcmProps   	    0x0400
#define fvcmWw	    	    0x00ff

/*#define fvcScreen           use ww != wwNil */
#define fvcInstructions     0x8100
#define fvcResults          0x8000
#define fvcHiddenVisi       0x8400
#define fvcHidResults	    (fvcHiddenVisi|fvcResults)

/* F I E L D  F E T C H  C O D E S */
#define ffeNested	    0x0001
#define ffeNoFSpec	    0x0002

/* C O N D I T I O N A L  O P  C O D E S */
#define cocEQ               0
#define cocGT               1
#define cocLT               2
#define cocGE               3
#define cocLE               4
#define cocNE               5

/* P O S T  S C R I P T  A R G U M E N T  I D S */
#define idPrintPsPage       0
#define idPrintPsPara       1
#define idPrintPsPic        2
#define idPrintPsCell       3
#define idPrintPsRow        4
#define idPrintPsDict       5

/* E X P R E S S I O N  D A T A  T Y P E S */
#define edtNumeric          1
#define edtString           2
#define edtStringField      3
#define edtText             4
#define edtNumericBkmk      5
#define edtTextBkmk         6


/* F I E L D  C A L C  L E V E L */
/* masks applied to vwFieldCalc */
#define fclFunc             0x0001
#define fclCa               0x0002
#define fclSel              0x0004
#define fclInsert           0x0100
#define fclSpecialFunc      0x1000


/* S T R U C T U R E S */

/*  Information stored in plcfld for each field character */
struct FLD
	{  /* FieLd Descriptor */
	struct
		{
		int ch : 7;
		int fDirty : 1;      /* field must be reparsed */
		};
	union
		{
		char flt;    /* chFieldBegin */
		char bData;  /* chFieldSeparate */
		struct
			{           /* chFieldEnd */
			int fDiffer : 1;    /* this field in ! show results mode */
			int : 1;
			int fResultDirty:1; /* user has edited or formatted the result */
			int fResultEdited:1;/* user has inserted or deleted to/fm result */
			int fLocked : 1;    /* this field is locked from recalc */
			int fPrivateResult:1;/* never show the result of this field */
			int fNested : 1;    /* field is nested within another field */
			int : 1;            /* spare */
			};
		char grpf;   /* chFieldEnd */
		};
	};

#define cbFLD (sizeof (struct FLD))


/*  Used to cache information about a field */
struct FLCD
	{  /* FieLd Composite Descriptor */
	CP cpFirst;   /* cp where field begins */
	CP dcpInst;   /* includes both {} and all inst text */
	CP dcpResult; /* includes | and all result text */
	BOOL fDirty;  /* field is dirty and must be reparsed */
	int flt;      /* field's type */
	int bData;    /* flt specific cached data */
	int ifldChBegin;
	int ifldChSeparate;
	int ifldChEnd;
	union
		{
		struct
			{
			int fDiffer : 1;    /* this field in ! show results mode */
			int : 1;
			int fResultDirty:1; /* user has edited or formatted the result */
			int fResultEdited:1;/* user has inserted or deleted to/fm result */
			int fLocked : 1;    /* this field is locked from recalc */
			int fPrivateResult:1;/* never show the result of this field */
			int fNested : 1;    /* field is nested within another field */
			int : 1;            /* spare */
			};
		int grpf : 8;
		};
	};

#define cbFLCD (sizeof (struct FLCD))




/*  This structure provides general information about a specific field type */
struct EFLT
	{  /* Element of dnflt */
	int fDead : 1;          /* kill a field of this type if live */
	int fLive : 1;          /* resurrect a field if dead */
	int fResult : 1;        /* this field may have a result */
	int fDisplay : 1;       /* this field displays something (not text) */
	int : 1;                /* spare */
	int fltg : 2;           /* field type group, for show results */        
	int : 1;                /* spare */
	int frm : 8;            /* refresh time mask */
	PFN pfnCalcFunc;        /* pointer to recalc function */
	};


/*  this is used to map ich's to cp's */
struct CR
	{  /* CP Run */
	CP cp;
	int ccp;
	};



/*  holds information about switches found during a fetch */
struct FSF
	{  /* Field Switch Fetched */
	int  c;                 /* how many were fetched */
	CHAR rgch [cSwitchMax]; /* the switches themselves */
	CP   rgcpSw [cSwitchMax]; /* cp of the switches */
	CP   rgcp [cSwitchMax]; /* cpNil or the cp of the switch's arg */
	};


/*  stores information about all defined field switches */
struct FSI
	{  /* Field Switch Information */
	int ch : 7;    /* the switch character, note: packed to 7 bits */
	int fArg : 1;  /* switch expects an argument */
	int flt : 8;   /* what flt does this switch apply to */
	};


/*  this is used by FetchFromField and functions that call it */
struct FFB
	{  /* Field Fetch Block */
	union
		{
		/*  this is used also by FetchVisibleRgch */
		struct FVB
			{  /* Fetch Visible Block */
			int  doc;     /* document fetching from */
			CP   cpFirst; /* first cp to fetch in next fetch (modified) */
			CP   cpLim;   /* limit of fetches */
			
			int  cch;     /* number of ch's read if rgch. undef if !rgch */
			int  ccr;     /* number of cr's corresponding to ch's read */

			union
				{
				struct FBB
					{  /* Fetch Block Buffers */
					int  cchMax;  /* max number of ch to place in rgch */
					CHAR *rgch;   /* buffer in which to place chars */
					
					int  ccrMax;  /* max number of cr's to place in rgcr */
					struct CR *rgcr;/* buffer in which to place cp runs */
					};
				struct FBB fbb;
				};

			BOOL fOverflow; /* fetch ovrflwd cchMax or ccrMax, fetch again */
			};
		struct FVB fvb;
		};

	union
		{
		struct FSFB
			{  /* Field Specific Fetch Block */
			CP   cpField;       /* cpFirst of the field being fetched from */
			/* inputs */
			int  flt;           /* what flt is the field being fetched */
			BOOL fFetchVanished;/* don't ignore vanished text */
			BOOL fGroupInternal;/* insert "group internal" characters */
			BOOL fPMergeRules;  /* use the Print Merge argument rules */
			BOOL fFetchTable;   /* first char of field in table */
			int  foc;           /* how to treat fetch options */
			/* outputs */
			BOOL fGrouped;      /* indicates returned argument was grouped */
			BOOL fNested;       /* fetched data contained fields */
			BOOL fNoArg;        /* there was no argument fetched */
			/* state (be very careful modifying) */
			int cchFieldBegin;  /* count of chfieldBegin's to be matched */
			BOOL fGroupPending; /* indicates inside chGroupExternal pair */
			BOOL fTableRules;   /* use table rules for this argument (PMerge) */
			struct FSF fsfFlt;  /* flt specific switches found */ 
			struct FSF fsfSys;  /* system switches found */
			};
		struct FSFB fsfb;
		};
	};


/* this is used in picture string parsing */
struct PPB
	{  /* Picture Parse Block */
	CHAR *pchNext;
	int wArg;
	CP cpArg;
	CHAR *pchArg;
	int pos;
	int posBlw;
	CHAR *rgch;   /* should be cchMaxPic+1 */
	struct CR *rgcr;  /* should be ccrMaxPic */
	BOOL fThouSep;
	BOOL fImpSign;
	};


/* contains state information during PrintMerge */
struct PMD
	{  /* Print Merge Data */
	int iRec;       /* next record to merge */
	int docData;    /* the source data document */
	CP cpData;      /* current location in data document */
	BOOL fAbortPass;/* flag to abort this pass */
	BOOL fFirstPass;/* indicates current pass is the first print merge pass */
	BOOL fEndMerge; /* End PrintMerge */
	struct STTB **hsttbHeader; /* the header names to apply to the data */
	};

/* contains information used to format Format Group fields (formulas, etc) */
struct FFS
	{  /* Field Formatting State */
	BOOL        fFormatting;
	int         flt;
	int         ifldFormat;
	int         bchr;
	int         fValidEnd;
	int         xp;
	int         ifldWrap;
	int         ifldError;
	};

/* cache for CpLimCharBlock */
#define icpCBCMax   3
struct CBC
	{ /* Char Block Cache */
	union
		{
		struct
			{
			int ww : 6;
			int doc : 6;
			int iNext : 4;
			};
		int w;
		};

	CP rgcp[icpCBCMax];
	CP rgcpLim[icpCBCMax];
	};

/* M A C R O S */

#define IifdFromFlt(flt) (flt - (fltInfo + 1))

extern int vdocFetchVisi;
extern struct CBC vcbc;
#define InvalVisiCache() vdocFetchVisi = docNil; vcbc.w = 0
extern struct CA vcaCell;
#define InvalCellCache() (vcaCell.doc = docNil)



#define nMaxRoman   4000  /* largest RN we can make is 3999 */
#define nMinRoman   1
#ifndef INTL
#define cchMaxOrdApp 3
#define cchMaxText  15	/* largest element of rgszNumText & rgszOrdText */
#else
#define cchMaxOrdApp 5
#define cchMaxText  23
#endif
#define cchMaxTextNum (cchMaxText * 9)
#define nMinText    0
#define nMaxText    1000000

/* F U N C T I O N S */

CP CpFirstField ();
CP CpLimInstField ();
CP CpLimField ();
CP CpLimDeadField ();
CP CpMatchFieldBack ();
CP DcpUnHideFld ();
CP DcpUnHideFld1 ();
#ifdef NOASM
CP CpFromIchPcr ();
CP CpFormatFrom ();
CP CpVisibleBackCpField();
#endif /* NOASM */
CP DcpCopyArgument ();
CP DcpCopyIchCchPcr ();
CP DcpApplySysSwitches ();
CP DcpApplyPictureSw ();
CP DcpApplyFormatSw ();
CP DcpReplaceWithError ();
CHAR ChFetchSwitch ();
struct EFLT EfltFromFlt ();
struct FLD * PfldDocIfldCh ();
#ifdef NOASM
NATIVE CP DcpSkipFieldChPflcd ();
NATIVE CP CpVisibleCpField ();
NATIVE IfldFromDocCp ();
NATIVE IfldInsertDocCp ();
NATIVE FFormatFieldPdcp ();
NATIVE FillFlcdIfld ();
#endif /* NOASM */
NATIVE CorrectFNestedBits();
NATIVE FAddChCpToPfvb();
NATIVE FetchFromField();
NATIVE FWhite();
NATIVE FPMergeTerm();
#ifdef NOASM
NATIVE FFillRgwWithLevelSeqs();
#endif /* NOASM */




/* T A B L E S */

#ifdef FIELDFMT   /* dnflt goes in fieldfmt.c, here for convenience */

FcrCalcFltBkmkRef ();
FcrCalcFltSet ();
FcrCalcFltIf();
FcrCalcFltIndex ();
FcrCalcFltToc();
FcrCalcFltStyleRef ();
FcrCalcFltSequence ();
FcrCalcFltInfo ();
FcrCalcFltPage ();
FcrCalcFltDateTime ();
FcrCalcFltExp ();
FcrCalcFltQuote ();
FcrCalcFltInclude ();
FcrCalcFltRefPage ();
FcrCalcFltAskFill ();
FcrCalcFltPMNextData ();
FcrCalcFltPMSkipIf ();
FcrCalcFltPMRec ();
FcrCalcFltDde ();
FcrCalcFltGlsy ();
FcrCalcFltImport ();
FcrCalcFltFtnRef ();



/* D N F L T */
/*  The following structure contains control information for individual
	field types. */

csconst struct EFLT dnflt [fltMax] =

{
					/* fDead, fLive, fResult, fDisplay, fltg, frm, 
						pfnCalcFunc */

/* unused */        { 0, 0, 0, 0, fltgOther, frmNone,
						NULL            },
/* fltUnknownKwd */ { 0, 0, 0, 0, fltgOther, frmNone,    
						NULL            },
/* fltPosBkmk */    { 0, 1, 1, 0, fltgOther, frmUser|frmMerge|frmHdr1st,
						FcrCalcFltBkmkRef},
/* fltBkmkRef */    { 0, 1, 1, 0, fltgOther, frmUser|frmMerge|frmHdr1st,
						FcrCalcFltBkmkRef},
/* fltXe */         { 1, 0, 0, 0, fltgOther, frmNone,    
						NULL            },
/* fltFtnRef */     { 0, 1, 1, 0, fltgOther, frmUser|frmPaginate,
						FcrCalcFltFtnRef},
/* fltSet */        { 0, 1, 0, 0, fltgOther, frmUser|frmMerge,    
						FcrCalcFltSet   },
/* fltIf */         { 0, 1, 1, 0, fltgOther, frmUser|frmMerge|frmHdr1st,    
						FcrCalcFltIf    },
/* fltIndex */      { 0, 1, 1, 0, fltgOther, frmUser,    
						FcrCalcFltIndex },
/* fltTce */        { 1, 0, 0, 0, fltgOther, frmNone,    
						NULL            },
/* fltStyleRef */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdrAll|frmMerge,
						FcrCalcFltStyleRef},
/* fltRefDoc */     { 1, 0, 0, 0, fltgOther, frmNone,    
						NULL            },
/* fltSequence */   { 0, 1, 1, 0, fltgOther, frmUser|frmMerge|frmHdrAll,
						FcrCalcFltSequence},
/* fltToc      */   { 0, 1, 1, 0, fltgOther, frmUser,
						FcrCalcFltToc},
		/* NOTE: order important fltInfo through fltDot */
/* fltInfo */       { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltTitle */      { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltSubject */    { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltAuthor */     { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltKeyWords */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltComments */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltLastRevBy */  { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltCreateDate */ { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltRevDate */    { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltPrintDate */  { 0, 1, 1, 0, fltgOther, frmUser|frmPrint,    
						FcrCalcFltInfo  },
/* fltRevNum */     { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltEditTime */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltNumPages */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltNumWords */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltNumChars */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltFileName */   { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltDot */        { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st|frmMerge,    
						FcrCalcFltInfo  },
/* fltDate */       { 0, 1, 1, 0, fltgOther, frmUser|frmPrint,
						FcrCalcFltDateTime},
/* fltTime */       { 0, 1, 1, 0, fltgOther, frmUser|frmPrint,
						FcrCalcFltDateTime},
/* fltPage */       { 0, 1, 1, 0, fltgOther, frmUser|frmPaginate,   
						FcrCalcFltPage  }
/* fltExp */        { 0, 1, 1, 0, fltgOther, frmUser|frmHdrAll|frmMerge,    
						FcrCalcFltExp   },
/* fltQuote */      { 0, 1, 1, 0, fltgOther, frmUser|frmMerge|frmHdr1st,    
						FcrCalcFltQuote },
/* fltInclude */    { 0, 1, 1, 0, fltgOther, frmUser|frmMerge,
						FcrCalcFltInclude},
/* fltPageRef */    { 0, 1, 1, 0, fltgOther, frmUser|frmPaginate,
						FcrCalcFltRefPage},
/* fltAsk */        { 0, 1, 0, 0, fltgOther, frmUser|frmMerge,
						FcrCalcFltAskFill},
/* fltFillIn */     { 0, 1, 1, 0, fltgOther, frmUser|frmMerge|frmHdr1st,
						FcrCalcFltAskFill},
/* fltPMData */     { 0, 1, 0, 0, fltgOther, frmMerge,
						FcrCalcFltPMNextData},
/* fltPMNext */     { 0, 1, 0, 0, fltgOther, frmMerge,
						FcrCalcFltPMNextData},
/* fltPMNextIf */   { 0, 1, 0, 0, fltgOther, frmMerge,
						FcrCalcFltPMNextData},
/* fltPMSkipIf */   { 0, 1, 0, 0, fltgOther, frmMerge,
						FcrCalcFltPMSkipIf},
/* fltPMRec */      { 0, 1, 1, 0, fltgOther, frmMerge,
						FcrCalcFltPMRec},
/* fltDde */        { 0, 1, 1, 0, fltgOther, frmUser,
						FcrCalcFltDde},
/* fltDdeHot */     { 0, 1, 1, 0, fltgOther, frmUser|frmDdeHot,
						FcrCalcFltDde},
/* fltGlsy */       { 0, 1, 1, 0, fltgOther, frmUser|frmHdr1st,
						FcrCalcFltGlsy},
/* fltPrint */      { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltFormula */    { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltHyperText */  { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltMacroText */  { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltSeqLevOut */  { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltSeqLevLeg */  { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltSeqLevNum */  { 0, 1, 0, 1, fltgOther, frmNone,
						NULL},
/* fltImport */     { 0, 1, 0, 1, fltgOther, frmUser,
						FcrCalcFltImport},

};
#endif /* FIELDFMT */


#ifdef FIELDCMD
/*  Field Calculation Error strings */
#define stErrorDef StFrameKey ("Error! ",FieldErrorSpace)
csconst CHAR rgstFldError [][] =
	{
		StKey ("Bookmark not defined.",BookmarkNotDefined),
		StKey ("No bookmark name given.",NoBookmarkNameGiven),
		StKey ("No style name given.",NoStyleNameGiven),
		StKey ("Style not defined.",StyleNotDefined),
		StKey ("No text of specified style in document.",NoTextOfSpecifiedStyle),
		StKey ("No sequence specified.",NoSequenceSpecified),
		StKey ("Unknown info keyword.",UnknownInfoKeyword),
		StKey ("Not a valid Hex digit.",NotValidHexDigit),
		StKey ("Digit expected.",DigitExpected),
#ifdef CRLF
		StKey ("Cannot insert CR or LF characters.",CannotInsertCRorLF),
#else
		StKey ("Cannot insert return character.",CannotInsertReturn),
#endif /* CRLF */
		StKey ("File name not specified.",FileNameNotSpecified),
		StKey ("Cannot open file.",CannotOpenFile),
		StKey ("Picture switch must be first formatting switch.",PictureSwitch),
		StKey ("Switch argument not specified.",SwitchArgumentNotSpecified),
		StKey ("Unknown switch argument.",UnknownSwitchArgument),
		StKey ("Number cannot be represented in specified format.",NumberCannotBeRepresented),
		StKey ("Cannot read or display file.",CannotReadOrDisplayFile),
		StKey ("Picture string contains unmatched quotes.",PictureStringUnmatchedQuotes),
		StKey ("Unknown character in picture string.",UnknownCharacterInPictureString),
		StKey ("Not a valid result for table.",NotValidResultForTable),
		StKey ("Not a valid document self-reference.",NotValidDocumentReference),
		StKey ("Not a valid bookmark self-reference.",NotValidBookmarkReference),
		StKey ("Missing test condition.",ConditionalExpressionMissing),
		StKey ("Missing second part of test condition.",ConditionalExpressionMissing2ndArg),
		StKey ("Missing comparison.",MissingOpcodeForConditional),
		StKey ("Unknown op code for conditional.",UnknownOpcodeForConditional),
		StKey ("Trailing portion of string too long for wildcard match.",StringTooLongForWildcardMatch),
		StKey ("No heading paragraphs found.",NoHeadingParagraphsFound),
		StKey ("Include may not refer to itself.",IncludeMayNotReferToItself),
		StKey ("No application specified.",NoAppSpecified),
		StKey ("No topic specified.",NoTopicSpecified),
		StKey ("No table of contents entries found.",NoTOCEntriesFound),
		StKey ("Glossary not defined.",GlossaryNotDefined),
		StKey ("No glossary specified.",NoGlossarySpecified),
		StKey ("Not a valid range of characters.",NotValidRangeOfCharacters),
		StKey ("Not a valid bookmark in index entry on page ",NotValidBookmarkOnPage),
		StKey ("No filename specified in document reference on page ",NoFilenameInDocRef),
		StKey ("Cannot open file referenced on page ",CannotOpenFileRef),
		StKey ("Index not allowed in footnote, header, or annotation.",IndexNotAllowed),
		StKey ("Table not allowed in footnote, header, or annotation.",TableNotAllowed),
		StKey ("Not a valid heading level range.",NotValidHeadingLevelRange),
		StKey ("No index entries found.",NoIndexEntriesFound),
		StKey ("Not a valid document self-reference on page ",NotValidDocRefOnPage),
		StKey ("Not a valid heading level in TOC entry on page ",NotValidHeadingLevelTC),
		StKey ("Cannot repaginate; no printer installed.",NoPrinterTocField),
	};
#endif /* FIELDCMD */

#define istNil                      (-1)
#define istErrBkmkNotDef            0
#define istErrNoBkmkName            1
#define istErrNoStyleName           2
#define istErrStyleNotDefined       3
#define istErrNoStyleText           4
#define istErrNoSeqSpecified        5
#define istErrUnknownInfoKwd        6
#define istErrInvalidHexDigit       7
#define istErrInvalidDigit          8
#define istErrNoInsertCRLF          9
#define istErrNoFileName            10
#define istErrCannotOpenFile        11
#define istErrPicSwFirst            12
#define istErrNoSwitchArgument      13
#define istErrUnknownSwitchArg      14
#define istErrInvalidNumber         15
#define istErrCannotImportFile      16
#define istErrUnmatchedQuote        17
#define istErrUnknownPicCh          18
#define istErrTableAction           19
#define istErrIncludeSelfRef        20
#define istErrBkmkSelfRef           21
#define istErrMissingCondExp1       22
#define istErrMissingCondExp2       23
#define istErrMissingCondOpCode     24
#define istErrUnknownCondOpCode     25
#define istErrWildMatchTooLong      26
#define istErrNoOutlineLevels       27
#define istErrNoSelfRefInclude      28
#define istErrNoDdeApp              29
#define istErrNoDdeTopic            30
#define istErrNoTocEntries          31
#define istErrGlsyNotDefined        32
#define istErrNoGlsyName            33
#define istErrInvalidCharRange      34
#define istErrInvalidBkmkXE         35
#define istErrNoFileRD              36
#define istErrBadFileRD             37
#define istErrSubDocIndex           38
#define istErrSubDocToc             39
#define istErrInvalidOutlineRange   40
#define istErrNoIndexEntries        41
#define istErrSelfRefRD             42
#define istErrInvalidLevelTC        43
#define istErrCantRepag				44



#ifdef FORMATSP
/* Display Field Messages */
csconst CHAR rgstFltMessage [][] =
	{
		StKey ("Error!",FieldError),
		StKey ("Main Document Only.",MainDocOnly),
		StKey ("###",NoMemNumber),
	};
#endif /* FORMATSP */

#define istMsgError                 0
#define istMsgNotMother             1
#define istMsgCannotFormat          2
								
