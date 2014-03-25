/* C H . H */
#define chNil           (-1)

#define chFormula       chFieldEscape
#define chBS            8
#define chTab           9

#define chReturn        13
#define chCRJ           11

#define chTable         7       /* MAC/WIN: must be ccpEop chars long */
#define chSect          12
#define chColumnBreak   14
#define ccpSect         1L
#define cchSect         1
#define ccpCRJ          1L

/* used to distinguish CR and CR/LF systems. */
#ifdef CRLF
#define cchEop          2
#define ccpEop          2L
#define ccpEol          2L
#define chEop           10
#define chEol           10
#else
#define cchEop          1
#define ccpEop          1L
#define ccpEol          1L
#define chEop           13
#define chEol           13      /* in unformatted files */
#endif

#define chNonReqHyphen  31
#define chNonBreakSpace 160
#define chNonBreakHyphen 30
#define chSpace         32
#define chHyphen        '-'
#define chColon         ':'
#ifndef MATHPACK
#define chNegative      '-'     /* for sort.c */
#endif /* MATHPACK */

#define chSkill         0x5e
#define chVisCRJ        0xAC
#define chVisEop        0xB6
#define chVisSpace      0xB7
#define chVisNonReqHyphen       0xAC    /* logical NOT symbol */
#define chVisNonBreakSpace      0xB0    /* degree symbol */

#define chVisCellEnd    0xA4



/* fSpec cch definitions */
/* printSubs.c, dispspec.c rely on the order (chLnn last and small) */
#define chPicture       1
#define chFootnote      2
#define chTFtn          3
#define chTFtnCont      4
#define chAtn           5
#define chLnn           6
#define chSpecFirst   (chPicture)
#define chSpecLast    (chLnn)



/* prefix key flags */
#define fKeyPrefixMore 1
#define fKeyPrefixOutline 2
#define fKeyLastScroll 4
#define fKeyPrefixScanTo 8



/* display characters from the ANSI set */

#define chEMark         (CHAR)'\244'


/* characters used by FIELDS */

#define chFieldBegin        19
#define chFieldSeparate     20
#define chFieldEnd          21

#define chFieldMin          19
#define chFieldMax          22

#define chFieldBeginDisp    '{'
#define chFieldSeparateDisp '|'
#define chFieldEndDisp      '}'

#define chDisplayField      17
#define chGroupInternal     18
#define chGroupExternal     '"'
#define chFieldEscape       '\\'

/* A character used to denote the beginning of
	a field expression error message. It can not
	be chFEPH defined in fltexp.h.*/
#define chFEErrMsg          '!'

/* various #defines for use in the document retrieval code */
#define chWild          '*'
#define chComma         ','
#define chFldSep        chComma
#define chLParen        '('
#define chRParen        ')'
#define chDQuote        '"'
#define chQMark         '?'
#define chCaret         '^'
#define chSemi          ';'
#define chDot           '.'
#define chAsterisk      '*'
#define chBackSlash     '\\'

/* Character shared by fltexp and fieldpic */
#define chPercent	'%'

/* Character used when parsing DOS command line */
#define chCmdSwitch	'/'


/* Publishing chars, unused holes in ANSI set handled specially by FormatLine */

#ifdef WIN
#define chPubMin	147
#define chPubLDblQuote	147
#define chPubRDblQuote	148
#define chPubBullet	149
#define chPubEmDash	150
#define chPubEnDash	151
#define chPubEmDashPS	151	// Postscript driver reverses the codes
#define chPubEnDashPS	150
#define chPubMax	152

#define chLQuote	0x91
#define chRQuote	0x92
#define chSimBullet	'o'
#endif
