#define sgcNil -1
#define sgcPap 0
#define sgcChp 1
#define sgcSep 2
#define sgcPic 3
#define sgcTap 4

struct CHP
		{
/* word 0: */
		int     fBold : 1;
		int     fItalic : 1;
		int     fStrike : 1;
		int     fOutline : 1;
#ifdef WIN
	int     fFldVanish: 1;
#else /* MAC */
		int     fShadow : 1;
#endif
		int     fSmallCaps : 1;
		int     fCaps : 1;
		int     fVanish : 1;
	int     fRMark : 1;
	int     fSpec : 1;
/* style modify bits for multi-bit fields */
	int     fsIco : 1;
		int     fsFtc : 1;
		int     fsHps : 1;
		int     fsKul : 1;
		int     fsPos : 1;
		int     fsSpace : 1;

/* word 1: */
		uns     ftc;
/* word 2: */
/* font size in half points */
		unsigned char     hps;
/* 2's complement signed number in half point units. >0 means superscript */
/* 256 - hpsPos is the absolute value for negative numbers */
		unsigned char     hpsPos;
/* word 3: */
/* following field encodes in quarter point units the space following
the character in the range -7 though +56 q.p.'s. */
		int     qpsSpace : 6;
	int     wSpare2 : 2;
/* color; encoded as index into the old-color table rgco */
	int     ico : 4;
	int	kul : 3;
/* the following bit is used only internally to communicate from sprm's
to FetchCp */
		int     fSysVanish : 1;
/* word 4-5: */
/* words to be used by fSpecial characters for their properties
		chPicture:      fc of the picture body
		WARNING: MACHINE DEPENDENT and order is significant!
			fnPic is high byte of fcPic and fDirty is highbit of
				entire long FC
*/
		union {
#ifdef WIN
				union {
						struct {
							int     dummy1;
							unsigned char     dummy2; 
							unsigned char     fnPic;       /* fn/fc coexist */
							};
						FC      fcPic;
						};
				struct {
						int     docPic;
						int     dummy : 15;
						int     fDirty : 1;
						};
#else /* ~WIN */
			/* MAC has abandoned the docPic concept */
		union {
			unsigned char     fnPic;       /* fn/fc coexist */
			FC      fcPic;
			};
#endif /* ~WIN */
/* used by large symbols in formulas (grpchrp only) if hps == hpsSeelarge */
		int hpsLarge;
				};
		};

#define cbCHP sizeof(struct CHP)
#define cwCHP (sizeof(struct CHP) / sizeof(int))
#define cwCHPBase       4
#define cbCHPBase       cwCHPBase * sizeof(int)

#ifdef MAC
#define maskFs          0x3f    /* WARNING: Machine-dependent */
#define maskFSpec       0x40    /* WARNING: Machine-dependent */
/* User cannot modify fSpec or fRMark */
#define maskFNonUser    0xC0    /* WARNING: Machine-dependent */

#define maskfBold       (0x8000) /* WARNING: Machine-dependent */
#define maskfItalic     (0x4000) /* WARNING: Machine-dependent */
#define maskfStrike     (0x2000) /* WARNING: Machine-dependent */
#define maskfOutline    (0x1000) /* WARNING: Machine-dependent */ 
#define maskfShadow     (0x0800) /* WARNING: Machine-dependent */
#define maskfSmallCaps  (0x0400) /* WARNING: Machine-dependent */
#define maskfCaps       (0x0200) /* WARNING: Machine-dependent */
#define maskfVanish     (0x0100)
#define maskMajorityBits (0xFF00) /* WARNING: Machine-dependent */
#else
#define maskFs          0xFC00  /* WARNING: Machine-dependent */
#define maskFSpec       0x0200  /* WARNING: Machine-dependent */
/* User cannot modify fSpec, fStrike, fRMark or fFldVanish */
#define maskFNonUser    0x0314  /* WARNING: Machine-dependent */

#define maskfBold       (0x0001) /* WARNING: Machine-dependent */
#define maskfItalic     (0x0002) /* WARNING: Machine-dependent */
#define maskfSmallCaps  (0x0020) /* WARNING: Machine-dependent */
#define maskfVanish     (0x0080) /* WARNING: Machine-dependent */
#define maskMajorityBits (0x00A3) /* WARNING: Machine-dependent */
#endif

#define hpsSeeLarge 255
#define hpsOldMax 254  /* limit used by RTF	if it gets a larger size */

/* underline codes */
#define kulNone         0
#define kulSingle       1
#define kulWord         2
#define kulDouble       3
#define kulDotted       4

#define cbitsKul        3

/* color codes */
#ifdef MAC
#define icoBlack        0
#define icoBlue         1
#define icoCyan         2
#define icoGreen        3
#define icoMagenta      4
#define icoRed          5
#define icoYellow       6
#define icoWhite        7
#else /* WIN */
#define icoAuto         0   /* WIN can't have Black by default */
#define icoBlack        1
#define icoBlue         2
#define icoCyan         3
#define icoGreen        4 
#define icoMagenta      5
#define icoRed          6
#define icoYellow       7
#define icoWhite        8

#endif /* WIN */

#define icoMax          (icoWhite + 1)

/* a table cell */
struct TC
	{
	union
		{
		int grpf;
		struct
			{
			int fFirstMerged : 1;           /* First cell to be merged */
			int fMerged : 1;                /* Cell is merged with others */
			int fUnused : 14;
			};
		};
	union
		{
		int     rgbrc[4];
		struct
			{
			int brcTop;     /* the order of these is assumed by the code */
			int brcLeft;
			int brcBottom;
			int brcRight;
			};
		};
	};
#define cbTC sizeof(struct TC)
#define cwTC (sizeof(struct TC)/sizeof(int))
#define ibrcTcLim 4
#define itcMax  32

/* selectors for the TC brcs */
#define brckTop         1
#define brckLeft        2
#define brckBottom      4
#define brckRight       8

/* NOTE: changes must be recorded in mpiwspxTap in insSubs.c */
struct TAP              /* Table properties */
	{
	int jc;                 /* Justification code */
	int dxaGapHalf;         /* Gap/2 */
	int dyaRowHeight;

	/* these (private) bits are set in vtapFetch by the CpFirstTap routines */
	union {
		int	grpfTap; /* also a field of TCXS for cache validation */
		struct {
			int fCaFull : 1;		/* set if caTap is fully extended */
			int fFirstRow : 1;		/* if fCaFull, set if first row in table */
			int fLastRow : 1;		/* set iff last row in table */
			int fOutline : 1;		/* row was cached for outline mode */
			int : 12;
			};
		};

	int itcMac;
	int dxaAdjust;                /* adjust rgdxaCenter for justification, computed when the TAP is cached */
	int rgdxaCenter[itcMax + 1];  /* Center positions for gaps */
	struct TC rgtc[itcMax];       /* Array of cell properties */
	};
#define cbTAP (sizeof(struct TAP))
#define cwTAP (cbTAP / sizeof (int))
#define cwTAPBase 6

#ifdef MAC
/* still around to support 3.0 file format */
#define cbPAPH  sizeof(int)
#endif

#define itbdMax 50
#define ibrcPapLim 6

/* NOTE: changes must be recorded in mpiwspxPap in insSubs.c */
struct PAP
		{
		struct PAPS   /* PAP Short */
				{
				unsigned char     stc;
				unsigned char     jc;         /* justification code */
		unsigned char     fSideBySide;
				unsigned char     fKeep;        /* Keep lines together. */
				unsigned char     fKeepFollow;  /* Keep together with the following para. */
				unsigned char     fPageBreakBefore;
				union {
			struct {
				int     fBrLnAbove : 1;
				int     fBrLnBelow : 1;
				int     fUnused : 2;
				int     pc : 4; /* positioned relative to */
				unsigned char     brcp;
				};
			struct {
				int     : 4;
				int     pcVert : 2;
				int     pcHorz : 2;
				int     : 8;
				};
			};
				unsigned char     brcl;
				unsigned char     nfcSeqNumb; /* auto numbering format code */
				unsigned char     nnSeqNumb;  /* auto numbering option */
				unsigned char     fNoLnn;/* no line numbering for this para */
		
				int     dxaRight;
				int     dxaLeft;
/* Left1 is the first line indent, signed number relative to dxaLeft */
				int     dxaLeft1;
				int     dyaLine;
				uns     dyaBefore;
				uns     dyaAfter;
		struct PHE phe;
				};
	char fInTable;
	char fTtp;
	struct TAP      *ptap;
/* absolute positioning: dxaAbs and dyaAbs
	== 0 means normal (left or in-line)
	> 0  means that the value -1 is the position (0 is encoded as 1)
	< 0  means one of the special positions defined below
	the object is not normal if (dxaAbs != 0 || dyaAbs != 0 || pcHorz != 2
		|| dxaWidth != 0).
		pcVert is not significant unless dyaAbs != 0; pcHorz is significant
		even when dxaAbs == 0 because it can mean left in page or in margin
*/
	int     dxaAbs;         /* horizontal abs. position */
	int     dyaAbs;         /* vertical abs. position */
/* abs width DOES imply absolute positioning, but the converse is not true */
	int     dxaWidth;       /* absolute width */
/* MAC: the brc's must occur in the same order and quantity as they do in 
	ARG_PARAGRAPH and ARG_PARABORDERS */
	union
			{
		int rgbrc[ibrcPapLim];
			struct
				{
			int     brcTop;     /* Borders on four sides */
			int     brcLeft;
			int     brcBottom;
			int     brcRight;
			int     brcBetween;     /* And between the paragraphs */
			int     brcBar;         /* And on facing pages */
				};
			};
	int     dxaFromText;    /* distance between text and abs obj */
	int     wSpare1; /* spare */
	int     wSpare2; /* spare */
/* this is amount of pap to scan for papx */
#define cwPAPBaseScan (offset(PAP, wSpare2) / sizeof(int)) /* spare */

/* number of tab stops. */
		int     itbdMac;
		int     rgdxaTab[itbdMax];
		char    rgtbd[itbdMax];
		};

#define cbPAP (sizeof (struct PAP))
#define cwPAP (cbPAP / sizeof (int))
#define cbPAPS (sizeof (struct PAPS))
#define cwPAPS (cbPAPS / sizeof (int))
#define cbPAPBase (cbPAP - (itbdMax * 3))
#define cwPAPBase (cbPAPBase / sizeof (int))

/* absolute positioning */
#define pcVMargin       0
#define pcVPage         1
#define pcHColumn       0
#define pcHMargin       1
#define pcHPage         2

			/* these are multiples of 4 because of tmvalNinch */
#define dxaAbsLeft       0
#define dxaAbsCenter    -4
#define dxaAbsRight     -8
#define dxaAbsInside    -12
#define dxaAbsOutside   -16

#define dyaAbsInline     0
#define dyaAbsTop       -4
#define dyaAbsCenter    -8
#define dyaAbsBottom    -12

/* tab descriptor */
struct TBD
		{
		int     jc : 3;
		int     tlc : 3;  /* leader dot code */
		int     wSpare1 : 2;
		};

/* constructor for TBD type --WARNING: MACHINE DEPENDENT-- */
#ifdef MAC
#define Tbd(jc, tlc) ((jc << 5) + (tlc << 2))
#else
#define Tbd(jc, tlc) ((jc) + ((tlc) << 3))
#endif

/* Justification codes: must agree with menu.mod */
#define jcLeft          0
#define jcCenter        1
#define jcRight         2
#define jcBoth          3

#define jcDecimal       3
#define jcBar           4
/* used internally in FormatLine only */
#define jcDecTable	5

/* Vertical justification codes */
#define vjcTop          0
#define vjcCenter       1
#define vjcBoth         2
#define vjcBottom       3    /* for completeness, but we don't use this one */

/* Tab leader codes */
#define tlcNone         0
#define tlcDot          1
#define tlcHyphen       2
#define tlcSingle       3
#define tlcHeavy        4
#define tlcDefault      tlcNone

/* standard stc codes */
#define stcNormal  0

#ifdef MAC
#define stcLevLast 255
#define stcLev1    255
#define stcLev2    254
#define stcLev3    253
#define stcLev4    252
#define stcLev5    251
#define stcLev6    250
#define stcLev7    249
#define stcLev8    248
#define stcLev9    247
#define stcLevMin  stcLev9

#define stcFtnText 246
#define stcFtnRef  245
#define stcHeader  244
#define stcFooter  243
#define stcPgn     242
#define stcLnn     241

#define stcIndexLast 240
#define stcIndexMin  234
#define stcIndex1  240
#define stcIndex2  239
#define stcIndex3  238
#define stcIndex4  237
#define stcIndex5  236
#define stcIndex6  235
#define stcIndex7  234

#define stcTocLast 233
#define stcTocMin  225
#define stcToc1    233
#define stcToc2    232
#define stcToc3    231
#define stcToc4    230
#define stcToc5    229
#define stcToc6    228
#define stcToc7    227
#define stcToc8    226
#define stcToc9    225

#define stcPostScript 224
#define stcNormIndent 223

#else

#define stcNormIndent 255
#define stcLevLast 254
#define stcLevMin  246
#define stcLev1    254
#define stcLev2    253
#define stcLev3    252
#define stcLev4    251
#define stcLev5    250
#define stcLev6    249
#define stcLev7    248
#define stcLev8    247
#define stcLev9    246

#define stcFtnText 245
#define stcFtnRef  244
#define stcHeader  243
#define stcFooter  242
#define stcIndexHeading  241
#define stcLnn     240

#define stcIndexLast 239
#define stcIndexMin  233
#define stcIndex1  239
#define stcIndex2  238
#define stcIndex3  237
#define stcIndex4  236
#define stcIndex5  235
#define stcIndex6  234
#define stcIndex7  233

#define stcTocLast 232
#define stcTocMin  225
#define stcToc1    232
#define stcToc2    231
#define stcToc3    230
#define stcToc4    229
#define stcToc5    228
#define stcToc6    227
#define stcToc7    226
#define stcToc8    225

#define stcAtnText 224
#define stcAtnRef  223

#endif /* ~MAC */
#define cstcMax (256)

/* WARNING: if stcStdMin changes, old Word files will not be readable! */
#define stcStdMin  222  /* Null stc; that which stcNormal is based on */
/* nil value used for MapStc cache in CachePara, and all over the style code */
#define stcNil 256
#define stcpNil (-1)
#ifdef WIN
/* MACREVIEW davidlu...use this rather than the 10 in styedit/stydefn and the
	12 in create.c...and don't forget to break too long based on chains in
	create.c (dsb) */
#define cstcMaxBasedOn 10
#endif


#define lvlMax (stcLevLast + 1 - stcLevMin)
#define lvlNil (-1)

/* border codes */

#define brcpNone   0
#define brcpBox    15
#define brcpBar    16
#define brcpAbove  1
#define brcpBelow  2

#define brclSingle 0
#define brclThick  1
#define brclDouble 2
#define brclShadow 3
#define brclNone   4
#define brclInval (-1)

#ifdef WIN
	/* a more convenient mapping for the para and tables dialogs in Win */
#define ibrclNone   0
#define ibrclSingle 1
#define ibrclThick  2
#define ibrclDouble 3
#define ibrclShadow 4
	/* note the following 2 are unused in Win except in rtf */
#define ibrclDotted 5
#define ibrclHairline 6

#define ibrcpNone   0
#define ibrcpBox    1
#define ibrcpBar    2
#define ibrcpAbove  3
#define ibrcpBelow  4
#endif

/* header specification */
struct HDS
		{
		int     fNotSameAsPrev : 1;
		int     ihddP1 : 7;
		};


/* NOTE: changes must be recorded in mpiwspxSep in insSubs.c */
struct SEP
		{
		unsigned char     bkc;    /* break code: Line Column Page Recto Verso */
		unsigned char     fTitlePage;
		int     ccolM1;
		int     dxaColumns;	/* dialogs parse as unsigned; signed for efficient arithmetic */

	unsigned char     fAutoPgn; /* if 1, auto page numbers will be shown */
/* page number format: Arabic UCRoman LCRoman UCLetter LCLetter */
		unsigned char     nfcPgn;
#ifdef MAC
	uns     dyaPgn;
	uns     dxaPgn;
#else
		uns     pgnStart; /* starting page number - user specified */
		uns     wSpare1; /* spare */
#endif /* ~MAC */
		unsigned char     fPgnRestart;

		unsigned char     fEndnote;

		char    lnc; /* line numbering code */
/* bits are right flush 5,4,3,2,1,0, set for each ihdt that has its own ihdd */
		char    grpfIhdt;
		uns     nLnnMod; /* line numbering if not 0 */
		int     dxaLnn;

		uns     dyaHdrTop;
		uns     dyaHdrBottom;

#ifdef WIN
		char    fLBetween;
		char    vjc; /* Vertical Justification Code. */
		int     lnnMin;
#endif

/* dependent quantities - not stored in file */
		int     dxaColumnWidth;	/* dialogs parse as unsigned; signed for efficient arithmetic */
		};

#define cbSEP (sizeof (struct SEP))
#define cwSEP (cbSEP / sizeof (int))

#define nfcNil          (-1)
#define nfcArabic       0
#define nfcUCRoman      1
#define nfcLCRoman      2
#define nfcUCLetter     3
#define nfcLCLetter     4
#define nfcOrdinal      5
#define nfcCardtext     6
#define nfcOrdtext      7
#define nfcHex          8
#define nfcLast         8   /* change this if you add nfc's */
	/* WARNING - the higher nfc's (13 - 15 as of this writing) are used
		by the Index code - don't add new nfc's beyond nfcIndexMin (index.h) 
		without redesigning the index data structures to include more bits
		for nfc and modifying the code appropriately */

#define bkcNoBreak      0
#define bkcNewColumn    1
#define bkcNewPage      2
#define bkcEvenPage     3
#define bkcOddPage      4

#define lncPerPage      0
#define lncRestart      1
#define lncContinue     2

/* values used by the ruler to set toggles & when acting on toggles */
#define dyaSpace1       (czaLine)
#define dyaSpace15      (czaLine*3/2)
#define dyaSpace2       (czaLine*2)
#define dyaParaClose    (0)
#define dyaParaOpen     (czaLine)

/* case swapping codes */
#define wcsAllLower     0
#define wcsAllUpper     1
#define wcsCapWords     2
#define wcsFirstCap     3

/* style scrap state */
struct STYS
	{
	int stcSrc;
	int stcNext;
	int stcBase;
	struct CHP chp;
	struct PAP pap;
	};
#define cwSTYS (sizeof (struct STYS) / sizeof (int))

#define RecordStshForDocNoExcp(doc, pstsh) bltb(&PdodDoc(doc)->stsh, pstsh, cbSTSH)
#define SetVdocStsh(doc) (vdocStsh = DocMother(doc))
#define StcpFromStc(stc, cstcStd) (((stc) + (cstcStd)) & 255)
#define StcFromStcp(stcp, cstcStd) (((stcp) - (cstcStd)) & 255)

#ifdef MAC
#define cwPAPBaseScanW3 12
#endif

