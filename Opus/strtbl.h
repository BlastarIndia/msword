/* S T R T B L . H */
/*  Contains definitions and strings for the string lookup table */


/* S T R I N G  G R O U P S */
#define szgFltSingle    0
#define szgFltNormal    1
#define szgFormatFunc   2
#define szgFltExpFunc   3
#define szgCondOpCodes  4
#define szgDdeSysTopic  5
#define szgRenumFormat  6
#define szgParseKeys    7
#define szgBkmkSpecial  8
#define szgPrintPs      9
#define szgMax          10



/*
	TABLE ENTRIES MUST BE IN SORTED (INCREASING) ORDER WITHIN EACH SZG.

	BE SURE TO KEEP THE INDICIES IN mpszgiste UP TO DATE.

	ALL ENTRIES MUST BE UPPER CASE.

	(upper case is intentional to support french-style (drop accents)
	 keyword lookup - jurgenl 08-21-89)
*/

#ifdef STRTBL

struct STE
	{  /* String Table Entry */
	int id;
	CHAR sz [];
	};

csconst struct STE rgste [] =
	{  /* id,	    sz		    */

/* S Z G  F L T  S I N G L E */
		/*  these strings are special field keywords that are only
			one character and don't have to be followed by a space */
		{ fltExp,	"="		},
#define isteSzgFltSingle 0
#define csteSzgFltSingle 1


/* S Z G  F L T  N O R M A L */
		/*  these are the mainstream field keywords */
		{ fltAsk,	"ASK"		},
		{ fltAuthor,	"AUTHOR"	},
		{ fltSeqLevNum, "AUTONUM"	},
		{ fltSeqLevLeg, "AUTONUMLGL"	},
		{ fltSeqLevOut, "AUTONUMOUT"	},

		{ fltComments,	"COMMENTS"	},
		{ fltCreateDate,"CREATEDATE"	},
		{ fltPMData,	"DATA"		},
		{ fltDate,	"DATE"		},
		{ fltDde,	"DDE"		},

		{ fltDdeHot,	"DDEAUTO"	},
		{ fltEditTime,	"EDITTIME"	},
		{ fltFormula,	"EQ"		},
		{ fltFileName,	"FILENAME"	},
		{ fltFillIn,	"FILLIN"	},

		{ fltFtnRef,	"FTNREF"	},
		{ fltGlsy,	"GLOSSARY"	},
		{ fltHyperText, "GOTOBUTTON"	},
		{ fltIf,	"IF"		},
		{ fltImport,	"IMPORT"	},

		{ fltInclude,	"INCLUDE"	},
		{ fltIndex,	"INDEX" 	},
		{ fltInfo,	"INFO"		},
		{ fltKeyWords,	"KEYWORDS"	},
		{ fltLastRevBy, "LASTSAVEDBY"	},

		{ fltMacroText, "MACROBUTTON"	},
		{ fltPMRec,	"MERGEREC"	},
		{ fltPMNext,	"NEXT"		},
		{ fltPMNextIf,	"NEXTIF"	},
		{ fltNumChars,	"NUMCHARS"	},

		{ fltNumPages,	"NUMPAGES"	},
		{ fltNumWords,	"NUMWORDS"	},
		{ fltPage,	"PAGE"		},
		{ fltPageRef,	"PAGEREF"	},
		{ fltPrint,	"PRINT" 	},

		{ fltPrintDate, "PRINTDATE"	},
		{ fltQuote,	"QUOTE" 	},
		{ fltRefDoc,	"RD"		},
		{ fltBkmkRef,	"REF"		},
		{ fltRevNum,	"REVNUM"	},

		{ fltRevDate,	"SAVEDATE"	},
		{ fltSequence,	"SEQ"		},
		{ fltSet,	"SET"		},
		{ fltPMSkipIf,	"SKIPIF"	},
		{ fltStyleRef,	"STYLEREF"	},

		{ fltSubject,	"SUBJECT"	},
		{ fltTce,	"TC"		},
		{ fltDot,	"TEMPLATE"	},
		{ fltTime,	"TIME"		},
		{ fltTitle,	"TITLE" 	},

		{ fltToc,	"TOC"		},
		{ fltXe,	"XE"		},
#define isteSzgFltNormal (isteSzgFltSingle+csteSzgFltSingle)
#define csteSzgFltNormal 52


/* S Z G  F O R M A T  F U N C */
		/* these are the arguments to the field format switch */
		{ ffnAlphabetic,"ALPHABETIC"	},
		{ ffnArabic,	"ARABIC"	},
		{ ffnCaps,	"CAPS"		},
		{ ffnCardtext,	"CARDTEXT"	},
		{ ffnCharFormat,"CHARFORMAT"	},
		{ ffnDollartext,"DOLLARTEXT"	},
		{ ffnFirstcap,	"FIRSTCAP"	},
		{ ffnHex,	"HEX"		},
		{ ffnLower,	"LOWER" 	},
		{ ffnMergeFormat,"MERGEFORMAT"	},
		{ ffnOrdinal,	"ORDINAL"	},
		{ ffnOrdtext,	"ORDTEXT"	},
		{ ffnRoman,	"ROMAN" 	},
		{ ffnUpper,	"UPPER" 	},
#define isteSzgFormatFunc (isteSzgFltNormal+csteSzgFltNormal)
#define csteSzgFormatFunc 14


/* S Z G  F L T  E X P  F U N C */
		/*  these are the functions available for the expression field */
		{ tfiABS,	"ABS"		},
		{ tfiAND,	"AND"		},
		{ tfiAVERAGE,	"AVERAGE"	},
		{ tfiCOUNT,	"COUNT" 	},
		{ tfiDEFINED,	"DEFINED"	},
		{ tfiFALSE,	"FALSE" 	},
		{ tfiIF,	"IF"		},
		{ tfiINT,	"INT"		},
		{ tfiMAX,	"MAX"		},
		{ tfiMIN,	"MIN"		},
		{ tfiMOD,	"MOD"		},
		{ tfiNOT,	"NOT"		},
		{ tfiOR,	"OR"		},
		{ tfiPRODUCT,	"PRODUCT"	},
		{ tfiROUND,	"ROUND" 	},
		{ tfiSIGN,	"SIGN"		},
		{ tfiSUM,	"SUM"		},
		{ tfiTRUE,	"TRUE"		},
#define isteSzgFltExpFunc  (isteSzgFormatFunc + csteSzgFormatFunc)
#define csteSzgFltExpFunc  18

/* S Z G  C O N D  O P  C O D E S */
		/*  these are the op codes allowed in conditional fields */
		{ cocLT,	"<"		},
		{ cocLE,	"<="		},
		{ cocNE,	"<>"		},
		{ cocEQ,	"="		},
		{ cocGT,	">"		},
		{ cocGE,	">="		},
#define isteSzgCondOpCodes (isteSzgFltExpFunc + csteSzgFltExpFunc)
#define csteSzgCondOpCodes 6


/* S Z G  D D E  S Y S  T O P I C */
		/* these are the system topic items we offer for dde */
		{ stiFormats,	"FORMATS"	},
		{ stiSysItems,	"SYSITEMS"	},
		{ stiTopics,	"TOPICS"	},
#define isteSzgDdeSysTopic (isteSzgCondOpCodes + csteSzgCondOpCodes)
#define csteSzgDdeSysTopic 3

		
/* S Z G  R E N U M  F O R M A T */
		/* formats for the renumber combo box */
		{ rpfLearn,	"LEARN" 	},
		{ rpfLegal,	"LEGAL" 	},
		{ rpfOutline,	"OUTLINE"	},
		{ rpfSequence,	"SEQUENCE"	},
#define isteSzgRenumFormat (isteSzgDdeSysTopic+csteSzgDdeSysTopic)
#define csteSzgRenumFormat 4


/* S Z G  P A R S E  K E Y S */
		/*  keywords recognized for SendKeys and Execute */
	{ VK_BACK,	"BACKSPACE"	},
	{ VK_BACK,	"BKSP"		},
	{ VK_CANCEL,	"BREAK" 	},
	{ VK_BACK,	"BS"		},
	{ VK_CAPITAL,	"CAPSLOCK"	},
	{ VK_CLEAR,	"CLEAR" 	},
	{ VK_DELETE,	"DEL"		},
	{ VK_DELETE,	"DELETE"	},
	{ VK_DOWN,	"DOWN"		},
	{ VK_END,	"END"		},
	{ VK_RETURN,	"ENTER" 	},
	{ VK_ESCAPE,	"ESC"		},
	{ VK_ESCAPE,	"ESCAPE"	},
	{ VK_F1,	"F1"		},
	{ VK_F10,	"F10"		},
	{ VK_F11,	"F11"		},
	{ VK_F12,	"F12"		},
	{ VK_F13,	"F13"		},
	{ VK_F14,	"F14"		},
	{ VK_F15,	"F15"		},
	{ VK_F16,	"F16"		},
	{ VK_F2,	"F2"		},
	{ VK_F3,	"F3"		},
	{ VK_F4,	"F4"		},
	{ VK_F5,	"F5"		},
	{ VK_F6,	"F6"		},
	{ VK_F7,	"F7"		},
	{ VK_F8,	"F8"		},
	{ VK_F9,	"F9"		},
	{ VK_HELP,	"HELP"		},
	{ VK_HOME,	"HOME"		},
	{ VK_INSERT,	"INSERT"	},
	{ VK_LEFT,	"LEFT"		},
	{ VK_NUMLOCK,	"NUMLOCK"	},
	{ VK_NEXT,	"PGDN"		},
	{ VK_PRIOR,	"PGUP"		},
	{ VK_PRINT,	"PRTSC" 	},
	{ VK_RIGHT,	"RIGHT" 	},
	{ VK_TAB,	"TAB"		},
	{ VK_UP,	"UP"		},
#define isteSzgParseKeys (isteSzgRenumFormat + csteSzgRenumFormat)
#define csteSzgParseKeys 40

/* S Z G  B K M K  S P E C I A L */
	/*  Names of special bookmarks */
	{ ibkmkCell,	"\\CELL"	},
	{ ibkmkChar,	"\\CHAR"	},
	{ ibkmkDoc,	"\\DOC" 	},
	{ ibkmkEODoc,	"\\ENDOFDOC"	},
	{ ibkmkEOSel,	"\\ENDOFSEL"	},
	{ ibkmkHeading, "\\HEADINGLEVEL"},
	{ ibkmkLine,	"\\LINE"	},
	{ ibkmkPage,	"\\PAGE"	},
	{ ibkmkPar,	"\\PARA"	},
	{ ibkmkPrevSel1,"\\PREVSEL1"	},
	{ ibkmkPrevSel2,"\\PREVSEL2"	},
	{ ibkmkSect,	"\\SECTION"	},
	{ ibkmkSel,	"\\SEL" 	},
	{ ibkmkSODoc,	"\\STARTOFDOC"	},
	{ ibkmkSOSel,	"\\STARTOFSEL"	},
	{ ibkmkTable,	"\\TABLE"	},
#define isteSzgBkmkSpecial (isteSzgParseKeys + csteSzgParseKeys)
#define csteSzgBkmkSpecial 16

/* S Z G  P R I N T  P S */
	/* postscript arguments to the print field */
	{ idPrintPsCell, "CELL" 	},
	{ idPrintPsDict, "DICT" 	},
	{ idPrintPsPage, "PAGE" 	},
	{ idPrintPsPara, "PARA" 	},
	{ idPrintPsPic,  "PIC"		},
	{ idPrintPsRow,  "ROW"		},
#define isteSzgPrintPs (isteSzgBkmkSpecial + csteSzgBkmkSpecial)
#define csteSzgPrintPs 6


#define isteMax (isteSzgPrintPs + csteSzgPrintPs)
	};


/*  this contains the starting iste for each szg */
csconst int mpszgiste [] =
	{
	isteSzgFltSingle,
	isteSzgFltNormal,
	isteSzgFormatFunc,
	isteSzgFltExpFunc,
	isteSzgCondOpCodes,
	isteSzgDdeSysTopic,
	isteSzgRenumFormat,
	isteSzgParseKeys,
	isteSzgBkmkSpecial,
	isteSzgPrintPs,
	isteMax
	};

#endif /* STRTBL */
