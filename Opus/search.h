#define ccrMaxDef 16

/* maximum length of static text for Search/Replace dialogs */
#define cchMaxSearchBanter 48
#define cchMaxReplaceBanter 55

#define cchFetchChunk  256

/* cbMax...Chp is the maximum size grpprl holding replacements of character
properties; it is obtained by summing (1 + dnsprm[].cch) over all the sprms
that can be applied through the Replace dialog; currently those are:
	sprmCFBold
	sprmCFItalic
	sprmCFStrikeRM
	sprmCFSmallCaps
	sprmCFVanish
	sprmCFtc
	sprmCKul
	sprmCHps
	sprmCHpsPos
*/
#define cbMaxGrpprlReplaceChp  19

/* cbMax...Pap is the maximum size grpprl holding replacements of paragraph
properties; it is obtained by summing (1 + dnsprm[].cch) over all the sprms
that can be applied through the Replace dialog; currently those are:
	sprmPJc
	sprmPDyaLine
	sprmPDyaBefore
*/
#define cbMaxGrpprlReplacePap  8

/* NOTE: code assumes cchSearchMax >= cchReplaceMax - 2;
	the fact that cchSearchMax > cchReplaceMax is inherited from
	MacWord; who knows why they had it that way */

#define  cchSearchMax   257
#define  cchReplaceMax  256
#define  cchConfirmMax  10   /* string user types to confirm change */
#define  wMatchAny      0x073f            /* 0x0700 | chMatchAny       */
#define  wMatchWhite    0x0777            /* 0x0700 | chMatchWhite    */

/* for search direction radio buttons */
#define  iUp    0
#define  iDown  1

struct FB  /* Formatting Block */
	{
	union
		{
		BYTE	fInfo;	 /* true iff we have any search (replace) info */
		struct
			{
			int    fChp   : 1;   /* true iff any zero bit in chpGray */
			int    fPap   : 1;   /* true iff any zero bit in papGray */
			int    fText  : 1;   /* true iff there is non-style text in edit control */
			int	   spare1 : 5;
			};
		};

	/* used in vfbReplace only */
	int    fReplMatch :1; /* true iff any ^m in replacement string */
	int    fReplScrap :1;  /* true iff any ^c in replacement string */
	int    spare2 : 6;

	struct CHP chp;
	struct PAPS pap;
	struct CHP chpGray;
	struct PAPS papGray;
	};


/* SRD is used to store info about special replacement sequences in
the replace text (^c for clipboard contents; ^m for matched text, or
their international equivalents) */

struct SRD  /* Special Replace Descriptor */
	{
	int     srt:2;  /* Special Replace Type: srtNil, srtClipboard, srtMatch */
	int     dcp:14; /* dcp in replace string between cp where current special
						replace sequence gets inserted and previous one (or
						beginning of the string for the first one); e.g.:

						replace string = foobar^mcat^cdog
						squeezed replace string = foobarcatdog

						rgsrd[0].dcp = 6
						rgsrd[1].dcp = 3

						get it? */
	};
						
#define csrdMax 10 /* max number of special replace chars in replace text */
				
#define srtNil       0  /* used to mark unused slots in rgsrd */
#define srtClipboard 1  /* replace with clipboard contents */
#define srtMatched   2  /* replace with matched text */

#ifdef FINDNEXT
typedef struct
		{
		int	is: 3; /* Do Search, Goto, Next/Prev Field 
						or just beep */
		int	doc: 13;
		union {
			int	chGoto;
			int	ibkf;
			int	wPercent;
			};
		} FNI;	/* Find Next Info. */

#define isNever		0
#define isSearch	1
#define isGotoChar	2
#define isGotoBkmk	3
#define isGotoPercent   4
#define isNextFld	5
#define isPrevFld	6
#ifdef DEBUG
#define isGotoCP	7
#endif
#endif /* FINDNEXT */




struct BMIB        /* Boyer-Moore Info Block */
	{
	int rgwSearch[cchSearchMax];
	int rgwOppCase[cchSearchMax];
	int cwSearch;
	int fNotPlain;  /* fFalse iff search file is unformatted and search
				and replace FB's are text only */
	CHAR mpchdcp[256];          /* table for search algorithm */
	/* fChTableInPattern is true iff one of the characters in rgwSearch
		is ^07.	Searching for this pattern in a formatted file must always
		return false or table selection rules may be violated allowing the
		user to corrupt tables. */
	int fChTableInPattern;
	};


struct RPP        /* RePlace Prls */
	{
	char grpprlChp[cbMaxGrpprlReplaceChp];
	char grpprlPap[cbMaxGrpprlReplacePap];
	int cbgrpprlChp;
	int cbgrpprlPap;
	};

struct RCB       /* Replace confirm dialog Communication Block */
	{
	struct BMIB *pbmib;
	struct RPP  *prpp;
	char *stReplace;
	int fnReplace;
	FC fcReplace;
	BOOL fParaReplace;
	int iChange;
	int iLastCase;
	struct SRD rgsrd[csrdMax];  /* Special Replace coDes in replace text */
	};

struct RRI      /* Replace Report Info */
	{
	CP cpLowRpt;
	CP cpHighRpt;
	CP cpLimWrap;
	};
