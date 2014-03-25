/* I N T E R . H */

/* Characters in Search patterns */
#define chPrefixMatch        '^'
#define chMatchAny           '?'
#define chMatchWhite         'w'
#define chMatchTab           't'
#define chMatchEol           'p'
#define chMatchNewLine       'n'
#define chMatchSect          'd'
#define chMatchNBS           's'
#define chMatchNRH           '-'
#define chMatchNBH           '~'
#define chMatchStyle         'y'
#define chUseScrap           'c'
#define chUseMatched         'm'

/* special "looks" keys; for Search/Replace dialog and DefineStyle dialog */
#define vkFont          'F'
#define vkPoint         'P'
#define vkStrike        'Z'
#define vkRMark         'N'
#define vkShowStd       'A'
#define vkColor         'V'

#define US				(1)	  /* from DOS country codes */
#define CANADA			(2)
#define LATINAMERICA	(3)
#define NETHERLANDS		(31)
#define FRANCE  		(33)
#define ITALY			(39)
#define UK      		(44)
#define DENMARK 		(45)
#define SWEDEN  		(46)
#define NORWAY  		(47)
#define PERU			(51)
#define MEXICO			(52)
#define ARGENTINA		(54)
#define BRAZIL			(55)
#define CHILE			(56)
#define VENEZUELA		(58)
#define FINLAND 		(358)
#define ICELAND 		(354)


#define cchMaxMornEve   5
#define cchMaxCurrency  4
/* international formatting information */
struct ITR
{
	CHAR chDecimal;         /* decimal point char [.] */
	CHAR chThousand;        /* thousands separator [,] */
	CHAR chList;            /* list separator [,] */
	CHAR szCurrency[cchMaxCurrency];  /* currency symbol [$] */
	CHAR fCurSepBlank;      /* whether a separating blank should
							be provided between a number and
					a currency sign. */
	/* fTrue iff the setting of above 4 characters */
	/* ambiguates a computed field parsing. */
	CHAR fInvalidChSetting;
	CHAR iDate;             /* date order */
	CHAR iTime;		    /* time setting */
	CHAR iLDate;            /* Long format date order */
	CHAR chDate;            /* date separator [/] */
	CHAR chTime;            /* time separator [:] */
	CHAR rgszMornEve [2][cchMaxMornEve];/* am/pm strings */
	CHAR fCurPostfix;	    /* Postfix Currency sign */
	CHAR fLZero;            /* Give a leading zero for date and time */
	CHAR iDigits;	    /* # of digits under decimal */

	CHAR fMetric;	/* Like MAC */
	CHAR chPageNo;		/* 'P' for Page */
	CHAR chSectNo;		/* 'S' for Section */

	CHAR fScandanavian;	/* For sort table use */
	CHAR fFrench;	/* For upper case table use */
	CHAR fUseA4Paper;	/* for default page size */
/* this list should grow */
};

#define iDateMDY 0
#define iDateDMY 1
#define iDateYMD 2

#define iLDateDMY 0
#define iLDateMDY 1

#define iszMorn  0
#define iszEve   1

#define	iTime12	 0
#define	iTime24	 1

/* def for date seperator strings */
#define rgstSepIntlDef        "\001 ", "\002, ", "\0"
#define iLDateDef             iLDateMDY   /* For US. */

/* default DOP values */
#define xaPageNonA4Def		(8 * dxaInch + dxaInch / 2)
#define yaPageNonA4Def		(11 * dxaInch)

#define xaPageA4Def			(11906) /* 8.27" */
#define yaPageA4Def			(16838) /* 11.69" */

#define dyaTopDopDef        (dxaInch)
#define dyaBottomDopDef     (dxaInch)
#define dxaLeftDopDef       (dxaInch + dxaInch / 4)
#define dxaRightDopDef      (dxaInch + dxaInch / 4)
#define dxaGutterDopDef     (0)
#define dxaTabDopDef        (dxaInch / 2)
#define fFacingPagesDopDef  (fFalse)
#define fWidowControlDopDef (fTrue)
#define fpcDopDef           (fpcBottomPage)
#define fWideDopDef         (fFalse)
#define grpfIhdtDopDef      (0)
#define fFtnRestartDopDef   (fFalse)
#define nFtnDopDef          (1)
#define nLnnDopDef          (1)
#define pgnDopDef           (1)
#define fRevMarkingDef      (0)
#define irmBarDef           (irmBarOutside)
#define irmPropsDef         (irmPropsUnder)
#define dxaHotZDopDef       (dxaInch / 4)

#define dxaPosFromTextDef   (czaInch >> 3)

/* Half default distance between columns i.e. half of .15" */
#define dxaDefaultGapHalf   (dxaInch * 15 / 200)

/* goto characters */
#define chGGSection         's'
#define chGGPage            'p'
#define chGGLine            'l'
#define chGGFtn             'f'
#define chGGAtn             'a'
#define chGotoPercent       '%'
#define chGoBackRel         '-'
#define chGoFrwdRel         '+'

/* field switches */
/* system switches */
#define chFldSwSysNumeric   '#'
#define chFldSwSysDateTime  '@'
#define chFldSwSysFormat    '*'
#define chFldSwSysLock      '!'
/* flt specific switches */
#define chFldSwStyleRefLast 'l'
#define chFldSwAskOnce      'o'
#define chFldSwDefault      'd'
#define chFldSwConverter    'c'
#define chFldXeText         't'
#define chFldXeRange        'r'
#define chFldXeBold         'b'
#define chFldXeItalic       'i'
#define chFldIndexRunin     'r'
#define chFldIndexBookmark  'b'
#define chFldIndexSeqname   's'
#define chFldIndexSeqSep    'd'
#define chFldIndexListSep   'l'
#define chFldIndexRangeSep  'g'
#define chFldIndexHeading   'h'
#define chFldIndexPartial   'p'
#define chFldIndexEntrySep  'e'
#ifdef ENABLE
#define chFldIndexConcord   'c'
#endif /* ENABLE */
#define chFldSeqNext        'n'
#define chFldSeqReset       'r'
#define chFldSeqCurrent     'c'
#define chFldSeqHidden      'h'
#define chFldTocOutline     'o'
#define chFldTocFields      'f'
#define chFldTocBookmark    'b'
#define chFldTocSeqname     's'
#define chFldTocSeqSep      'd'
#define chFldTceTableId     'f'
#define chFldTceLevel       'l'
#define chFldImportBogus    'c'
#define chFldPrintPs        'p'


/* field picture/format argument option characters */
#define chFldPicReqDigit    '0'
#define chFldPicOptDigit    '#'
#define chFldPicTruncDigit  'x'
#define chFldPicNegSign     '-'
#define chFldPicSign        '+'
#define chFldPicExpSeparate ';'
#define chFldPicQuote       '\''
#define chFldPicSequence    '`'
#define chFldPicMonth       'm'  /* uppercase used */
#define chFldPicDay         'd'
#define chFldPicYear        'y'
#define chFldPicHour        'h'
#define chFldPicMinute      'm'  /* lowercase used */
#define chFldPicSeconds     's'
#define chFldPicAMPMSep     '/'
#define MONTHMINUTESAME  /* indicates that month and minute are same ch.
							case is used to disambiguate. */
#define chPicDayHldr        ((CHAR) 1)
#define chPicMonthHldr	    ((CHAR) 2)
#define chPicYearHldr	    ((CHAR) 3)
#define chPicMinHldr	    ((CHAR) 4)
#define chPicHourHldr	    ((CHAR) 5)
#define chPic24HourHldr	    ((CHAR) 6)
#define chPicAPDUCHldr	    ((CHAR) 7)
#define chPicAPDLCHldr	    ((CHAR) 8)
#define chPicAPSUCHldr      ((CHAR) 9)
#define chPicAPSLCHldr	    ((CHAR) 10)
#define chDateSepHldr       '?'
#define chTimeSepHldr       ':'
#define chPicReqDigHldr     chFldPicReqDigit
#define chPicOptDigHldr     chFldPicOptDigit
#define chPicTruncDigHldr   chFldPicTruncDigit
#define chPicNegSignHldr    chFldPicNegSign
#define chPicPosSignHldr    chFldPicSign
#define chPicExpSepHldr     chFldPicExpSeparate
#define chPicThousandHldr   ((CHAR) 11)
#define chPicDecimalHldr    ((CHAR) 12)
#define chPicPreCurHldr     ((CHAR) 13)
#define chPicPostCurHldr    ((CHAR) 14)
#define chPicFractDigHldr   ((CHAR) 15)

/* These masks must match the ordering of gfi in EV defined in fltexp.h */
#define gfiComma	    1
#define gfiCurrency	    2
#define gfiNegInParen	    4
#define gfiDecimal	    8
#define gfiPercent	   16   /* Not used by EV */

#define szTplDMYDef         {chPicDayHldr, chDateSepHldr,	\
					chPicMonthHldr, chDateSepHldr,	\
					chPicYearHldr, '\0'}
#define szTplMDYDef         {chPicMonthHldr, chDateSepHldr,	\
								chPicDayHldr, chDateSepHldr,	\
					chPicYearHldr, '\0'}
#define szTplYMDDef         {chPicYearHldr, chDateSepHldr,	\
					chPicMonthHldr, chDateSepHldr,	\
					chPicDayHldr, '\0'}
#define szTplTime12Def      {chPicHourHldr, chTimeSepHldr,	\
					chPicMinHldr, ' ',			\
					chPicAPDUCHldr, '\0'}
#define szTplTime24Def      {chPic24HourHldr, chTimeSepHldr,	\
					chPicMinHldr, '\0'}

/* wildcards used in field conditional expressions */
#define chCondMatchOne      '?'
#define chCondMatchMany     '*'


/* SendKeys/Execute information */
#ifdef ELDDE
/* these are defined in below as ichExFoo */
#define ichExShift	1
#define ichExCtrl	2
#define ichExAlt	3
#define ichExOpStart	4
#define ichExOpEnd	5
#define ichExKeyStart	6
#define ichExKeyEnd	7
#define ichExGrpStart	8
#define ichExGrpEnd	9
#define ichExEnter	10
#define ichSpace    	11
#define ichTab	    	12
#define ichExLast	12

csconst BYTE stEx[] =
	{
		ichExLast,
		'+',		/* Shift Character */
		'^',		/* Control Character */
		'%',		/* Alt Character */
		'[',		/* Begin Opcode */
		']',		/* End Opcode */
		'{',		/* Begin Keyword */
		'}',		/* End Keyword */
		'(',		/* Begin Group */
		')',		/* End Group */
		'~',		/* Enter */
		' ',	    	/* Space */
		'\t',	    	/* Tab */
	};
#endif /* DRIVE */

#ifdef AVGWORD
/* The number of characters in an average word of a language, plus one
	for a separating space. (multiply by 10)*/
/* In English, the average is 4.7. */
#define cchAvgWord10Def	57
/* The number indicating the difference between the exact word count
	stored in a document and our approximate word count, above which
	we assume the word count in the document is invalid. */
#define dcwMin		1000L
#endif /* AVGWORD */

/* utDefault now computed based on country code bz */

/* CBT has a separate default unit of measurement */
#define utCBT     utInch

/* From fltexp.c: longest error message (they all begin with !) */
#define cchFEMaxIntl	80

/* For row and column selection in tables and spreadsheets */
#define chRow	'R'
#define chCol	'C'

/* command line switches from init2.c and initwin.c */
#define chCmdLineMacro		'M'
#define chCmdLineTutorial1	't'
#define chCmdLineTutorial2	'T'
#define chCmdLineNoFile1	'n'
#define chCmdLineNoFile2	'N'

/* Section Identifief which appears in the header/footer Icon bar */
#define chSectionID	'S'
