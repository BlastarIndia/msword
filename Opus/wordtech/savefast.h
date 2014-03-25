/* S A V E F A S T . H */

#define icpPicMax 20
struct QSIB
	{ /* Quick Save Info Block */
	/* fTrue if an error was encountered while quicksaving */
	int fError;

	int docSave;
	int fnDest;

	/* fTrue when all of docSave is to be copied to fnDest */
	int fCompleteSave;

	/* fTrue when style sheet must be rewritten */
	int fRewriteStsh;

	/* fTrue when must add extra eop for complete save of docFtn and
		docHdd */
	int fExtraPiece;
	/* records the maximum extent of new extension */
	FC  fcDestLim;

		/* point after the FIB where text starts */
		FC fcMin;

	/* set to fTrue when there is still room for text in the previous
		extension */
	int fRoomInPrevExt;
	/* fc in previous extension where new text may be added */
	FC  fcPrevExtLim;
	/* fc of the end of the last text page in the previous extension */
	FC  fcPrevExtPageLim;


	FC fcBegText;
	FC fcMac;
	FC fcBegOfTables;
	FC fcDocLim;

	int fAlreadyWritten;

	/*  fc of the begin of the original style sheet allocation for doc */
	FC  fcStshOrigFirst;
	/*  fcLim of the original style sheet allocation for doc */
	FC  fcStshOrigLim;

	/*  fcFirst where new style sheet will be written  */
	FC  fcDestStshFirst;

	/*  last CHP FKP for previous extension */
	PN  pnChpExtFirst;
	/*  count of new CHP FKPs required for new extension */
	PN  cpnChp;

	/* last PAP FKP for previous extension */
	PN  pnPapxExtFirst;
	/* count of new PAP FKPs required for new extension */
	PN  cpnPapx;

	uns ipcd;
	uns ised;
	uns iprc;

	CP  cpPicFetch;
	CP  cpPicFetchLim;

	uns icpPic;
	uns icpPicMac;
	CP  rgcpPic[icpPicMax];

#ifdef MAC
	/* fc, cb pairs recording where tables will be stored in new  file */
	FC  fcStshf;
	uns cbStshf;
	FC  fcPlcffndRef;
	uns cbPlcffndRef;
	FC  fcPlcffndTxt;
	uns cbPlcffndTxt;
	FC  fcPlcfsed;
	uns cbPlcfsed;
	FC  fcPlcfpgd;
	uns cbPlcfpgd;
	FC  fcSttbfglsy;
	uns cbSttbfglsy;
	FC  fcPlcfglsy;
	uns cbPlcfglsy;
	FC  fcPlcfhdd;
	uns cbPlcfhdd;
	FC  fcPlcfbteChpx;
	uns cbPlcfbteChpx;
	FC  fcPlcfbtePapx;
	uns cbPlcfbtePapx;
	FC  fcPlcfsea;
	uns cbPlcfsea;
	FC  fcRgftc;
	uns cbRgftc;
	FC  fcPrr;
	uns cbPrr;
	FC  fcPlcfphe;
	uns cbPlcfphe;
	FC  fcDop;
	uns cbDop;
	FC  fcSttbFlc;
	uns cbSttbFlc;
	FC  fcWss;
	uns cbWss;
	FC  fcClx;
	uns cbClx;
	FC  fcPlcfpgdFtn;
	uns cbPlcfpgdFtn;
	PN  cpnBteChp; 
	PN  cpnBtePap; 
#endif /* MAC */
#ifdef WIN
	int fNonPieceTableItem;
	FC  fcNonPieceLim;

		/* fc, cb pairs recording where tables will be stored in new  file */

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

		FC      fcPrEnv;        /* pruns environment */
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

		FC      fcSpare1;
		uns     cbSpare1;

		FC      fcSpare2;
		uns     cbSpare2;

		FC      fcSpare3;
		uns     cbSpare3;

		int     wSpare4;
		PN		pnChpFirst; 	
		PN      pnPapFirst;
			
			PN  cpnBteChp; 
			PN  cpnBtePap; 
			
/* end of 1-1 correspondence to FIB */
#endif /* WIN */

		uns     wDummy;     /* must follow fc/cb pairs */

		FC  fcPlcfpcd;
		uns cbPlcfpcd;

#ifdef WIN
		/*  compound document stuff */
		BOOL fFirstOfTwo;   /* TRUE when writing a doc which will
								have another appended to it. */
		BOOL fWritingSecond;/* TRUE when writing a file which is
								appended a previous file. */
#else
	int fNonPieceTableItem;
	FC  fcNonPieceLim;

	FC  cbMac;
	/* Set fTrue on first pass, fFalse on second. */
	int fFirstPass;
#endif /* ~WIN */
		PN pnFib;           /* page at which to write FIB */
	int fWord3;
	int fPicFound;           /* set to fTrue when picture found during save */
	int fHasPicFib;         
	long lcbAvailBte; 
	int fBteNoLim; 
	};


struct DSR
		{       /* Document Save Record (for one doc) */
		int doc;
		CP ccpText;
		CP ccpHdr;
		CP ccpFtn;
#ifdef WIN
		CP ccpMcr;
		CP ccpAtn;
		int fFkpdChpIncomplete; 
		int fFkpdPapIncomplete; 
#else
	int 	  fWord3; 
#endif /* ~WIN */
		};
#define cbDSR (sizeof (struct DSR))


struct MSR
		{       /*  Multi-document Save Record (saves state between two
					calls to FQuicksave). */
		int pnFibSecond;
#ifdef WIN
		struct FIB fibFirst;
#endif /* WIN */
		};

struct FEQ
	{
	FC      fcDest;
	int     fAlreadyWritten;
	};
#define cbFeq (sizeof (struct FEQ))


struct PLCFEQ
	{
	uns     ifeqMac;
	uns     ifeqMax;
	uns     cb;
	};

struct PEQ
	{
	FC      fcDest;
	int     iprc;
	};
#define cbPeq (sizeof (struct PEQ))

#define cbMinPlcpcd ((cbPCD + sizeof(CP)) * 3) + sizeof(CP)
