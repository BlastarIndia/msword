/* Token ID's */
#define tiNull      (-1)
#define tiEOI        (0)
#define tiNonCharMin 256
#define tiERRCODE    256
#define tiCOMSEP     257
#define tiEQ         258
#define tiLT         259
#define tiLE         260
#define tiGT         261
#define tiGE         262
#define tiNE         263
#define tiNUMBER     264
#define tiBOOKMARK   265
#define tiRDCFNC     266
#define tiBINFNC     267
#define tiUNRFNC     268
#define tiIF         269
#define tiROW        270
#define tiCOL        271
#define tiINTEGER    272
#define tiFLDESC     273
#define tiPRE_CURRENCY  274
#define tiPOST_CURRENCY 275
#define tiNUMINPAREN 276
#define tiBOOL	     277
#define tiUMINUS     278
/*---- Used by lexical analyzer internally ----*/
#define tiBOOLFALSE  279
#define tiBOOLTRUE   280
/*---- ch equivalent ti's --------*/
/* If you change any of these, you must change fldexp\fldexp.grm and
	regenerate the tables defined below. */
#define tiPlus      ((short) '+')
#define tiMinus     ((short) '-')
#define tiMultiply  ((short) '*')
#define tiDivide    ((short) '/')
#define tiExp       ((short) '^')
#define tiLParen    ((short) '(')
#define tiRParen    ((short) ')')
#define tiLBracket  ((short) '[')
#define tiRBracket  ((short) ']')
#define tiCln       ((short) chColon)

#ifndef STRTBL
#define iNestMax       10
#define icsMax         30  /* Evaluation stack depth. */
#define ictMax         311 /* corresponds to YYLAST */
#define ICtDef     (-1000)
/* Some special production id's. */
#define pdException  (-2)
#define pdError         0

/* YACC generated tables. */
#define rgepDef		/* corresponds to yyexca */	\
	-1, 1,						\
	0, -1,						\
	-2, 0

#define mpictcsNewDef  /* corresponds to yyact */		\
	10,  18,  89,  41,  88,   6,  40,  80,   3,  19,	\
	42,  20, 101,  17,  10,  89,  73,  88,  43,   6,	\
100,  71,  72,   2,  98,   4,  68,  32,  77,  85,	\
	80,  62,  97,  30,  59, 110,  30,  28,  31,  29,	\
	56,  31,  70,  30,  28,  60,  29,  95,  31,  66,	\
	84,  62, 103,  30,  28,  64,  29,  39,  31, 102,	\
	30,  28,  37,  29,  36,  31,  16,  30,  28,  15,	\
	29,  57,  31,  75,  30,  28,  38,  29,  34,  31,	\
	55,  30,  28,   8,  29,  32,  31,   1,  32,  30,	\
	28,   0,  29,  69,  31,  32,  30,  28,  83,  29,	\
	78,  31,  90,  81,  67,  32,   0,  94,   0,  82,	\
	79,   0,  32,   0,   0,   0,  99,   0,   0,  32,	\
	0,   0,   0,   0,   0, 106,  32,  96, 107,   0,	\
	0,   0,   0,  32, 109,   0,   0,   0, 105,   0,	\
	61,  32,   0,   0,   5,  21,   0,  33,  32,   0,	\
	0,  35,   0,   0,   0,   0,   0,   0,   0,   0,	\
	0,   0,   0,  44,  45,  46,  47,  48,  49,  50,	\
	51,  52,  53,  54,   0,   0,   0,   0,  63,  73,	\
	65,   0,   0,   0,  71,  72,   0,   0,   0,   0,	\
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	\
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	\
	0,   0,   0,   0,   0,  91,  92,  93,   0,   0,	\
	0,   0,   0,   0,  18,  58,  11,  12,  13,  14,	\
	86,  87,  19,   0,  20,   0,  17,   7,  18,   9,	\
	11,  12,  13,  14,  87, 108,  19,   0,  20,   0,	\
	17,   7,  22,  23,  24,  25,  26,  27, 104,  22,	\
	23,  24,  25,  26,  27,   0,   0,   0,   0,  22,	\
	23,  24,  25,  26,  27,   0,  22,  23,  24,  25,	\
	26,  27,  76,  22,  23,  24,  25,  26,  27,  74,	\
	22,  23,  24,  25,  26,  27,   0,  22,  23,  24,	\
	25,  26,  27,   0,   0,  22,  23,  24,  25,  26,	\
	27

#define mpcsictDef  /* corresponds to yypact */			\
-250,-1000,-233, -26, -26,  47, -26,-1000,-263,-1000,		\
-26,  24,  22,-1000,  17,-1000,-1000,-1000,-269,-272,		\
-254,  47, -26, -26, -26, -26, -26, -26, -26, -26,		\
-26, -26, -26,-1000,-1000,  39, -40, -26,  15, -26,		\
-1000,-1000,-1000,-1000,  54,  54,  54,  54,  54,  54,		\
	-9,  -9, -67, -67, -67,-1000,   8,-1000, -60,-231,		\
-249,  47,-1000,  32,-1000,  25,-1000, -86, -40, -63,		\
	-8, -41, -28,-1000, -26, -26, -26,-1000, -63, -11,		\
-1000,-1000,-1000,-1000,-249,-247, -28,-1000,-252,-260,		\
-1000,  18,  11,   1,-1000,-249, -63,-1000, -28,-1000,		\
-1000,-1000,-1000,-1000, -26, -63,-1000,-1000,  -6,-1000,	\
-1000

#define rgdictDef    /* corresponds to yypgo */			\
	0,  87, 140,  83,  40,  76,  73,  71,  45,  28,		\
	34,  32,  42,  29,  66,  69

#define mppddigtDef  /* corresponds to yyr1 */			\
	0,   1,   1,   2,   2,   2,   2,   2,   2,   2,		\
	2,   2,   2,   2,   2,   2,   2,   2,   2,   2,		\
	2,   5,   6,   2,   2,   4,   4,   4,   8,   9,		\
	7,   7,   7,   7,  11,  11,  11,  11,  11,  11,		\
	11,  11,  12,  12,  13,  13,  13,  10,  10,  14,		\
	14,  14,  14,  15,  15,  15,  15,   3,   3

#define mppdcSymDef  /* corresponds to yyr2 */			\
	0,   3,   2,   3,   3,   3,   3,   3,   3,   3,		\
	3,   3,   3,   3,   2,   1,   1,   1,   3,   4,		\
	6,   0,   0,   6,   8,   1,   3,   3,   1,   1,		\
	4,   6,   3,   5,   4,   2,   2,   3,   3,   2,		\
	1,   1,   1,   1,   1,   2,   2,   1,   1,   2,		\
	2,   2,   2,   1,   1,   1,   1,   2,   1

#define mpcstiCheckDef      /* corresponds to yychk */			\
-1000,  -1, 273, 258, 258,  -2,  45, 277,  -3, 265,		\
	40, 266, 267, 268, 269, -15, -14, 276, 264, 272,		\
274,  -2, 258, 259, 260, 261, 262, 263,  43,  45,		\
	42,  47,  94,  -2, -15,  -2,  40,  40,  -5,  40,		\
275, 275, 264, 272,  -2,  -2,  -2,  -2,  -2,  -2,		\
	-2,  -2,  -2,  -2,  -2,  41,  -4,  -7, 265, -10,		\
	-8,  -2,  91,  -2,  40,  -2,  41,  -8, 257, -11,		\
-12, 270, 271, 265, 257,  -6, 257,  -9, -11, -12,		\
	93, -10,  -4,  -9,  58, -13, 271, 272,  45,  43,		\
-13,  -2,  -2,  -2,  -9,  58, -12, -11, 271, -13,		\
272, 272,  41,  41, 257, -12,  -9, -13,  -2,  -9,		\
	41

#define mpcspdDef    /* corresponds to yydef */			\
	0,  -2,   0,   0,   0,   2,   0,  15,  16,  17,		\
	0,   0,   0,  21,   0,  58,  53,  54,  55,  56,		\
	0,   1,   0,   0,   0,   0,   0,   0,   0,   0,		\
	0,   0,   0,  14,  57,   0,   0,   0,   0,   0,		\
	50,  52,  49,  51,   3,   4,   5,   6,   7,   8,		\
	9,  10,  11,  12,  13,  18,  47,  25,  17,   0,		\
	0,  48,  28,   0,  22,   0,  19,   0,   0,  42,		\
	0,  40,  41,  43,   0,   0,   0,  26,  42,   0,		\
	29,  27,  47,  32,   0,  35,  39,  44,   0,   0,		\
	36,   0,   0,   0,  30,   0,   0,  42,  38,  37,		\
	45,  46,  20,  23,   0,   0,  33,  34,   0,  31,		\
	24

/*------------------- Non yacc generated constants ---------------*/

#define chEOI       ((CHAR) 1)
#define chAbbrevCur ((CHAR) 2)


#define cchNumMax   cchMaxSig
#define cchTokenMax ((cchBkmkMax > cchNumMax) ?	\
				cchBkmkMax : cchNumMax)
#define ichLkAhdMax (2*(cchTokenMax + (2 * cchMaxCurrency) + 2))
#define ccrFebMax   5

typedef struct _rw {
			CHAR     szName[];
			int      ti;
			CHAR     fi;
			}   RW;
#endif /* not STRTBL */


/* Function ID's. */
#define fiNULL       255
#define fiABS          0
#define fiAND          1
#define fiAVERAGE      2
#define fiCOUNT        3
#define fiDEFINED      4
#define fiIF           5
#define fiINT          6
#define fiMAX          7
#define fiMIN          8
#define fiMOD          9
#define fiNOT         10
#define fiOR          11
#define fiPRODUCT     12
#define fiROUND       13
#define fiSIGN        14
#define fiSUM         15
/* fi's for infix operators. */
#define fiPLUS		16
#define fiMINUS		17
#define fiMULTIPLY	18
#define fiDIVIDE	19
#define fiInfixPreloadMin   fiEXP
#define fiEXP		20
#define fiEQ		21
#define fiLT		22
#define fiLE		23
#define fiGT		24
#define fiGE		25
#define fiNE		26

#define Tfi(_fi, _ti)       ((((_ti) - tiNonCharMin) << 8) | (_fi))
#define FiFromTfi(_tfi)     ((_tfi) & 0x00FF)
#define TiFromTfi(_tfi)     ((((_tfi) & 0xFF00) >> 8) + tiNonCharMin)

#define tfiABS       Tfi(fiABS,     tiUNRFNC)
#define tfiAND       Tfi(fiAND,     tiBINFNC)
#define tfiAVERAGE   Tfi(fiAVERAGE, tiRDCFNC)
#define tfiCOUNT     Tfi(fiCOUNT,   tiRDCFNC)
#define tfiDEFINED   Tfi(fiDEFINED, tiUNRFNC)
#define tfiFALSE     Tfi(fiNULL,    tiBOOLFALSE)
#define tfiIF        Tfi(fiIF,      tiIF)
#define tfiINT       Tfi(fiINT,     tiUNRFNC)
#define tfiMAX       Tfi(fiMAX,     tiRDCFNC)
#define tfiMIN       Tfi(fiMIN,     tiRDCFNC)
#define tfiMOD       Tfi(fiMOD,     tiBINFNC)
#define tfiNOT       Tfi(fiNOT,     tiUNRFNC)
#define tfiOR        Tfi(fiOR,      tiBINFNC)
#define tfiPRODUCT   Tfi(fiPRODUCT, tiRDCFNC)
#define tfiROUND     Tfi(fiROUND,   tiBINFNC)
#define tfiSIGN      Tfi(fiSIGN,    tiUNRFNC)
#define tfiSUM       Tfi(fiSUM,     tiRDCFNC)
#define tfiTRUE      Tfi(fiNULL,    tiBOOLTRUE)


#ifndef STRTBL
typedef struct _fm {
			int     fmCDig: 4;
			int     fmGrpf: 4;
			int     iOp1:   4;
			int     iOp2:   4;
			}   FM;

#define fmNoop          0
#define fmClr           1
#define fmSpec          2
#define fmCopy          3
#define fmGrtr          4
#define fmOr            4

#define iOpNone         0

/* Format Composition table. */
#define mppdfmDef                                         \
			/*  1: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/*  2: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/*  3: */ {fmClr,  fmClr,  iOpNone, iOpNone}, \
			/*  4: */ {fmClr,  fmClr,  iOpNone, iOpNone}, \
			/*  5: */ {fmClr,  fmClr,  iOpNone, iOpNone}, \
			/*  6: */ {fmClr,  fmClr,  iOpNone, iOpNone}, \
			/*  7: */ {fmClr,  fmClr,  iOpNone, iOpNone}, \
			/*  8: */ {fmClr,  fmClr,  iOpNone, iOpNone}, \
			/*  9: */ {fmGrtr, fmOr,         1,       3}, \
			/* 10: */ {fmGrtr, fmOr,         1,       3}, \
			/* 11: */ {fmGrtr, fmOr,         1,       3}, \
			/* 12: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 13: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 14: */ {fmCopy, fmCopy,       2, iOpNone}, \
			/* 15: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 16: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 17: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 18: */ {fmCopy, fmCopy,       2, iOpNone}, \
			/* 19: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 20: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 21: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 22: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 23: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 24: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 25: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 26: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 27: */ {fmGrtr, fmOr,         1,       3}, \
			/* 28: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 29: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 30: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 31: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 32: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 33: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 34: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 35: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 36: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 37: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 38: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 39: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 40: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 41: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 42: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 43: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 44: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 45: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 46: */ {fmNoop, fmNoop, iOpNone, iOpNone}, \
			/* 47: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 48: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 49: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 50: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 51: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 52: */ {fmSpec, fmSpec, iOpNone, iOpNone}, \
			/* 53: */ {fmCopy, fmCopy,	     1, iOpNone}, \
			/* 54: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 55: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 56: */ {fmCopy, fmCopy,       1, iOpNone}, \
			/* 57: */ {fmGrtr, fmOr,	     1,       2}, \
			/* 58: */ {fmCopy, fmCopy,       1, iOpNone}

#define rhoStarStar	0x0000
#define rhoRow		0x0001
#define rhoCol		0x0002
#define rhoRowCol	(rhoRow | rhoCol)

/* Table Reference with 2 indices. */
typedef struct _tr2 {
		int	doc;
		CP	cpTable;
		int	iRowMin;
		int	iColMin;
		int	iRowLast;
		int	iColLast;
		int	fRel:	 1;
		int	fBkmk:	 1;
		int	fSpare: 14;
		}	TR2;
#define iStar		0x7FFF

/* TR2 or evaluated Expression.  Used for <texp> list. */
typedef struct _tre {
		int	fTR2;
		union {
			TR2	tr2;
			struct {
				NUM	 num;
				CHAR	 cDigBlw;
				CHAR	 gfi;
					};
				};
		} TRE;
#define cwTRE	CwFromCch(sizeof(TRE))
#define cTREAlloc 5

#define cchErrTknMax   ((sizeof(CP) * 2 + sizeof(int)) / sizeof(CHAR))
#define cchMaxCurLkAhd (cchMaxCurrency + 1)
typedef struct _ev {
		int	iszErrMsg;  /* == iszNoError if the content of the following
							union struct is defined */
		/* The value for the left hand side of a production uses one
				of these fields. */
		union	{
				NUM         v;
				int	    i;
				struct {
						CP  cpFirstBkmk;
						CP  cpLimBkmk;
						int ibkf;
						};
				TR2	    tr2;

				struct	{
						int		itreMax;
						int	    itreMac;
						TRE	  **hrgtre;
						};

				struct	{
						int	    fRel;
						int	    loc;
						};

				CHAR szErrTkn[cchErrTknMax];
				};

		/* Intermediate formatting information. */
		/* Following two fields must match CGFI */
		unsigned            cDigBlwDec:  8;
		union	{
				unsigned        gfi:	     8;
				/* This ordering must match gfi masks in inter.h */
				struct	{
						unsigned    fComma:      1;
						unsigned    fCurrency:   1;
						unsigned    fNegInParen: 1;
						unsigned    fDecimal:    1;
						unsigned    fSpare:      4;
						};
				};
		} EV;

typedef struct _cgfi {
	unsigned cDigBlwDec: 8;
	unsigned gfi:	     8;
	} CGFI;

typedef struct _ep {
			short    ti;
			short    cs;
			}   EP;


/* Heap Allocation Chain */
/* Used to keep track of heap object allocated for hrgtre's during
	parsing but not have freed due to syntax errors. */
typedef struct _hac {
		char		**h;
		struct _hac	**hhacNext;
		} HAC;
#define cwHAC	CwFromCch(sizeof(HAC))


/* Applied(?) Value: Used to represent the result of a reduction function. */
typedef struct _av {
		NUM		num;
		unsigned	c;
		unsigned	cDigBlw: 8;
		unsigned	gfi:	 8;
		} AV;


/* Lexical Analyzer Buffers. */
typedef struct _fibk {
			struct FVB fvb;
			int        tiPrev;
			EV         evPrev;
			CHAR       szTknPrev[cchTokenMax];
			int        ichTknMacPrev;
			unsigned   fPrevNum:      1;
		unsigned   fPrevCur:      1;
			unsigned   :      1;
			unsigned   fFirst:        1;
			unsigned   fBkmkSub:      1;
		unsigned   fVanished:     1;
			unsigned   fSpare:        2;
			unsigned   cParenSkipped: 8;
			}   FIBK;

typedef struct _fiblk {
			/* These first 10 entries must be exactly the same as FIBK
				for overloading of TiLexSubCalc to work. */
			struct FVB fvb;
			int        tiPrev;
			EV         evPrev;
			CHAR       szTknPrev[cchTokenMax];
			int        ichTknMacPrev;
			unsigned   fPrevNum:      1;
			unsigned   fPrevCur:      1;
			unsigned   :      1;
			unsigned   fFirst:        1;
			unsigned   fBkmkSub:      1;
			unsigned   fVanished:     1;
			unsigned   fSpare:        2;
			unsigned   cParenSkipped: 8;

			/* Only used in fetching from a block selection stream. */
			struct BKS bks;
			BOOL       fMoreBlock;
			}   FIBLK;

/* Field Expression evaluator block. */
typedef struct _feb {
			union {
				struct FFB  *pffb;
				FIBK        *pfibk;
				FIBLK       *pfiblk;
				};
			int          ichFetch;
			CHAR         (*pfnChFetch)();
			short        (*pfnTiLex)();

			/*------ Parser Stuff ------*/
			int          ti;    /* current input token number */
			EV           evLex;
			EV           evResult;
			/* cp of the beginning of the enclosing table. */
			/* If not in table, cpNil. */
			CP		 cpFirstTable;
			CP		 cpFirstRel;
			HAC		**hhac;

			/*------ Lexical Analyzer Stuff -------*/
			CHAR		szToken[cchTokenMax];
			int			ichTokenMac;
			CHAR		rgchLkAhd [ichLkAhdMax];
			int			ichLkAhdMin;
			int			ichLkAhdMac;
#ifdef DEBUG
			BOOL		fLookingAhead;
#endif	/* DEBUG */
			BOOL		fNoImplicitSubCalc;
			BOOL		fSubCalc;
			BOOL		fCurAlphaLastCh;
			/* ------- Currency Look Ahead stuff -------- */
			CHAR         szCurLkAhd[cchMaxCurLkAhd];
#ifdef INTL
			/* to prevent catching currency in the middle of Bookmarks */
			BOOL		fTestForCurrency;
#endif /* INTL */
			} FEB;

/* Parse Evaluation Stack */
typedef struct _pes {
		int	iNest;
		int	icsMac;
		CHAR	rgcs[icsMax];
		EV	rgev[icsMax];
		}	PES;
#define InitPpes(_ppes)	((_ppes)->iNest = (_ppes)->icsMac = 0)

/* Error Message id's. */
#define iszNoError            0
#define iszStackOvfl          1
#define iszSyntaxErr          2
#define iszZeroDiv            3
#define iszInvalChSetting     4
#define iszEmptyExpression    5
#define iszUnexpEOI           6
#define iszFEOverflow	      7
#define iszFEUnderflow        8
#define iszFETrans            9
#define iszNegFractExp	     10
#define iszFEStUnderflow     11
#define iszIllegalIndex	     12
#define iszNotInTable	     13
#define iszMixedRelAbs	     14
#define iszIllegalIxComb     15
#define iszIndexTooLarge     16
#define iszOutOfMemory       17
#define iszMissingBinOp	     18
#define iszSyntaxErrTkn      19
#define iszUndBkmkTkn        20
#define iszFESyntaxAtBkmk    21
#define iszNotTableRef	     22

#define iszErrMsgTknMin   iszSyntaxErrTkn

#define FEVUndefined(_ev) ((_ev).iszErrMsg)

#define chFEPH            '\\'


/* Macros for those functions too trivial to make them functions. */
#define IchMod(_ich)	((_ich) % ichLkAhdMax)
#define InitLkAhd(_pfeb) ((_pfeb)->ichLkAhdMin = (_pfeb)->ichLkAhdMac = 0)
#define BackTrackLkAhd(_pfeb, ich)                                         \
	{                                                 \
	(_pfeb)->ichLkAhdMin = ich;                       \
	BryanL( CommSzNum( SzShared( "Backtrack LkAhd to ich: "), (_pfeb)->ichLkAhdMin )); \
	Debug((_pfeb)->fLookingAhead--);                  \
	}
#define StartLkAhd(_pfeb, pich)						\
	{						\
	BryanL( CommSzNum( SzShared( "Start LkAhd at ich == "), (_pfeb)->ichLkAhdMin ) ); \
	Debug( (_pfeb)->fLookingAhead++ );		\
	*(pich) = (_pfeb)->ichLkAhdMin;			\
	}
#define AcceptLkAhd(_pfeb, ichMin)     /* no action in nondebug! */	\
	{						\
	BryanL(CommSz( SzShared( "Accept LkAhd\r\n") )); \
	Debug( (_pfeb)->fLookingAhead-- );		\
	}
#define SkipWhiteSpaces(_ch, _pfeb)                                        \
	while (FWhite((_ch)))                             \
		{                                             \
		(_ch) = GetNextCh((_pfeb));                   \
		}
#define ResetTokenBuf(_pfeb) ((_pfeb)->ichTokenMac = 0)
#define PutChInTokenBuf(_ch, _pfeb)                                        \
	if ((_pfeb)->ichTokenMac < cchTokenMax - 1)       \
		{                                             \
		((_pfeb)->szToken)[(_pfeb)->ichTokenMac++] =  (_ch); \
		}
#define ShrinkTokenBuf(_pfeb) ((_pfeb)->ichTokenMac--)
/* As long as PutChInTokenBuf() is used, the following macro is safe. */
#define NullTerminateToken(_pfeb)                                          \
	(((_pfeb)->szToken)[(_pfeb)->ichTokenMac] = '\0')


#define FBinOpTi(_ti)   ((_ti) == tiPlus     || (_ti) == tiMinus  || \
							(_ti) == tiMultiply || (_ti) == tiDivide || \
							(_ti) == tiExp)
#define FAlphaFE(ch)		(FAlpha((ch)) || ((ch) == '_'))
#define FAlphaNumFE(ch)		(FAlphaFE((ch)) || FDigit((ch)))

/* Function declarations. */
CHAR   ChFetchFromFe();
CHAR   ChFetchFromBkSel();
CHAR   ChFetchFromBlock();
int    TiLexFPfeb();
int    TiLexSubCalc();
CHAR   GetNextCh();
void   NumExp();
NATIVE SynthesizePevFormat();
CHAR   ChFetchExp();
int    WApproxCmpPnum();
CP     CpFirstTableFromDocCp();
TRE    **HTreAllocCw();
#endif /* not STRTBL */
