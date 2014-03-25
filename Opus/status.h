#ifdef WIN23
/*
 * The status line currently looks like this (with X=digit, ^ = space field
 * for PMWORD (space char for Opus), and "|" = separator line for Opus,
 * "raised" 3-D separator for PMWORD.  Spaces between toggles are also raised
 * separators in PMWORD):
 *
 * Pg^XX^^Sec^XX^^XXXX/XXXX^|^
 * 					At^XX.X^unit^^Ln^XXX^^Col^XXX^|^EXT^^CAPS^^NUM^^OVR^^REC
 *													COL			  	MRK
 */
#endif /* WIN23 */
/* Note:  ITEM and FIELD are used interchangeably when talking about
	status line items. Not to be confused with fields of structures. */

struct SLS   /*  Status Line State */
	{
	int pgn;       /* page number (within current section) */
	int isedP1;    /* section number */
	int ipgdDocP1; /* page number within document */
	int ipgdMac;   /* total number of pages in document */
	int yaPp;      /* position of cursor from top of printed page */ 
	int lnn;       /* line number (relative to top of printed page) */
	int col;       /* column number (relative to left margin) */
	int ExBl;      /* whether we are in extend mode or block select mode */
	int fCL;       /* whether the caps lock key is toggled */
	int fNL;       /* whether the num lock key is toggled */
	int OtMr;      /* whether we are in overtype or mark revisions mode */
	int fRec;      /* whether we are in macro-recording mode */
#ifdef WIN23
	int sp;		   /* = -1, blanks are ficStaticQ, so this erases the field */
#endif /* WIN23 */
	};

#define cwSLS  (sizeof(struct SLS) / sizeof(int))

#define bwSlsNil  cwSLS     /* == bwSlsMax */

#ifdef STATLINE
/* These are the isi's (Index of Statline Item) for the status line.
	Really, only the ones for flag fields are used explicitly, but
	they're all here to show the order of the fields. An "S" after the
	"isi" and before a qualifier means a static text field*/

#define isiNil      -1
#define isiSPg       0  /* "Pg" */
#define isiPgn       1  /* sls.pgn */
#define isiSSec      2  /* "Sec" */
#define isiIsedP1    3  /* sls.isedP1 */
#define isiIpgdDocP1 4  /* sls.ipgdDocP1 */
#define isiSSlash    5  /* "/" */
#define isiIpgdMac   6  /* sls.ipgdMac */
#define isiSep1      7  /* Separator 1 */
#define isiSAt       8  /* "At" */
#define isiYaPp      9  /* sls.yaPp */
#define isiSLn      10  /* "Ln" */
#define isiLn       11  /* sls.ln */
#define isiSCol     12  /* "Col" */
#define isiCol      13  /* sls.col */
#define isiSep2     14  /* Sep2 */
#define isiExBl     15  /* sls.ExBl */
#define isiCL       16  /* sls.fCL */
#define isiNL       17  /* sls.fNL */
#define isiOtMr     18  /* sls.OtMr */
#define isiRec      19  /* sls.fRec */
#define isiLast     20  /* represents space to right of last entry */
#define isiMax     (isiLast + 1)

#ifdef WIN23
/* Win 3 isi's */
#define isiSep03			0	/* Raised section at left of stat line */
#define isiSPg3			1  /* "Pg" */
#define isiPgn3			2  /* sls.pgn */
#define isiSSec3			3  /* "Sec" */
#define isiIsedP13		4  /* sls.isedP1 */
#define isiIpgdDocP13	5  /* sls.ipgdDocP1 */
#define isiSSlash3		6  /* "/" */
#define isiIpgdMac3		7  /* sls.ipgdMac */
#define isiSep13			8  /* Separator 1 */
#define isiSAt3			9  /* "At" */
#define isiYaPp3			10  /* sls.yaPp */
#define isiSLn3			11  /* "Ln" */
#define isiLn3			12  /* sls.ln */
#define isiSCol3			13  /* "Col" */
#define isiCol3			14  /* sls.col */
#define isiSep23			15  /* Sep2 */
#define isiExBl3			16  /* sls.ExBl */
#define isiExBl_CLSp3	17	/* Space between ExBl and CL */
#define isiCL3			18  /* sls.fCL */
#define isiCL_NLSp3		19	/* Space between CL and NL */
#define isiNL3			20  /* sls.fNL */
#define isiNL_OtMrSp3	21	/* Space between NL and OtMr */
#define isiOtMr3			22  /* sls.OtMr */
#define isiOtMr_RecSp3	23	/* Space between OtMr and Rec */
#define isiRec3			24  /* sls.fRec */
#define isiLast3     	25  /* represents space to right of last entry */
#define isiMax3     (isiLast3 + 1)

#endif /* WIN23 */
/* isz's used for Overtype/Mark Revisions field, which can take on more
	than one string value. */

#define iszOtMrNothing 0
#define iszOtMrOT      1 
#define iszOtMrMR      2

/* isz's used for Extend/Block Select field, which can take on more than
	one string value. */

#define iszExBlNothing 0
#define iszExBlEX      1
#define iszExBlBL      2

struct SI   /*  Statline Item */
	{
	int dch;  /* position where item is drawn */	

	union /* need this whole silly union because one of the bitfields
				has two names */
		{
		struct
			{
			int fic        : 3;  /* Field Identifier Code */
			int fRightJust : 1;
			int bwSls      : 5;  /* word offset of this item's field in SLS;
										used by sls-fields only (obviously) */
			int iszStatic  : 4;  /* For static text fields, it is the isz
									into rgszStatLine where we can find the
									text to display; for indexed fields it
									is an iszStart  */
			int fHilite    : 1;  /* show text inverted */
			int            : 2;  /* spare */
			};
		struct
			{
			int            : 9;
			int iszStart   : 4;  /* isz into rgszStatLine where the strings
									that this field can display start */
			int            : 3;
			};
		};
	};
#ifdef WIN23
struct SI3   /*  Statline Item for Win 3 */
	{
	union /* need this whole silly union because one of the bitfields
				has two names */
		{
		struct
			{
			int fic        : 3;  /* Field Identifier Code */
			int fRightJust : 1;
			int bwSls      : 5;  /* word offset of this item's field in SLS;
										used by sls-fields only (obviously) */
			int iszStatic  : 4;  /* For static text fields, it is the isz
									into rgszStatLine where we can find the
									text to display; for indexed fields it
									is an iszStart  */
			int fHilite    : 1;  /* show text inverted */
			int            : 2;  /* spare */
			};
		struct
			{
			int            : 9;
			int iszStart   : 4;  /* isz into rgszStatLine where the strings
									that this field can display start */
			int            : 3;
			};
		};
	};
#endif /* WIN23 */

/* Field identifier codes */

#define ficInt        0     /* integer */
#define ficStatic     1     /* static text */
#define ficSeparator  2     /* vertical separator */
#define ficMeasure    3     /* measurement */
#define ficIndexed    4     /* indexed text */
#define ficIntQ       5     /* conditional int (don't show if -1) */
#define ficStaticQ    6     /* conditional static text (don't show if -1) */
#ifdef WIN23
#define ficSepToggle	7		/* separator between toggle fields */
#endif /* WIN23 */

#define BwSLS(field)  (offset(SLS, field) / sizeof(int))

/* indices into rgszStatLine */

#define iszSPg          0
#define iszSSec         1
#define iszSSlash       2
#define iszSAt          3
#define iszSLn          4
#define iszSCol         5
#define iszIExBl        6
#define iszICl          8
#define iszINl          9
#define iszIOtMr        10
#define iszIRec         12
#define iszStatlineMax  13
#define iszNil          iszStatlineMax

/* position of each field, expressed as an offset in number of
	characters. */
/* These need to be internationalized. */

#define dchSPg       0	/* "Pg"   */
#define dchPgn       4	/* pgn    */
#define dchSSec      8	/* "Sec"  */
#define dchIsedP1    12	/* isedP1 */
#define dchIpgdDocP1 15	/* ipgdDocP1 */
#define dchSSlash    19	/* "/"    */
#define dchIpgdMac   20	/* ipgdMac*/
#define dchSep1      24	/* Sep1   */
#define dchSAt       25	/* "At"   */
#define dchYaPp      28	/* yaPp   */
#define dchSLn       34	/* "Ln"   */
#define dchLn        37	/* ln     */
#define dchSCol      41	/* "Col"  */
#define dchCol       45	/* col    */
#define dchSep2      48	/* Sep2   */
#define dchExBl      49	/* ExBl   */
#define dchCL        53	/* fCL    */
#define dchNL        58	/* fNL    */	
#define dchOtMr      62	/* OtMr   */
#define dchRec       66	/* fRec   */
#define dchAfterRec  69	
#define dchMax	     400


/* Array of strings for static and indexed text fields.  For indexed
	fields with more than one possible string, all the string values
	that that field can take on are listed in order.  The isz's below
	are used for multiple-string indexed text fields. */

csconst CHAR rgstStatLine[][] = {StKey(" Pg ",Pg),
				StKey("Sec ",Sec),
				StKey("/",SlashFoo),
				StKey("At ",AtFoo),
				StKey("Ln ",LnFoo),
				StKey("Col ",Col),
				StKey("EXT",EXT),
				StKey("COL",COL),  /* block selection (COLumn) */
				StKey("CAPS",CAPS),
				StKey("NUM",NUM),
				StKey("OVR",OVR),
				StKey("MRK",MRK),
				StKey("REC",REC) };
#ifdef WIN23

csconst CHAR rgstStatLine3[][] = {StKey("Pg",Pg3),
				StKey("Sec",Sec3),
				StKey("/",SlashFoo3),
				StKey("At",AtFoo3),
				StKey("Ln",LnFoo3),
				StKey("Col",Col3),
				StKey("EXT",EXT3),
				StKey("COL",COL3),  /* block selection (COLumn) */
				StKey("CAPS",CAPS3),
				StKey("NUM",NUM3),
				StKey("OVR",OVR3),
				StKey("MRK",MRK3),
				StKey("REC",REC3) };

#endif /* WIN23 */
/*  rgsi: 

	This is the biggie, the array with all the info used in drawing
	the status line.  It is the basis of the code's table-driven-ness.
	The array contains info for the fields in left-to-right order.

*/

csconst struct SI rgsi[isiMax] = { 

/* dch, fic, fRightJust, bwSls, isz, fHilite */
{dchSPg,    ficStaticQ,  fFalse, BwSLS(pgn),    iszSPg,   fFalse},
{dchPgn,    ficIntQ,     fFalse, BwSLS(pgn),    iszNil,   fFalse},
{dchSSec,   ficStatic,   fFalse, bwSlsNil,      iszSSec,  fFalse},
{dchIsedP1, ficIntQ,     fFalse, BwSLS(isedP1), iszNil,   fFalse},
{dchIpgdDocP1, ficIntQ,  fTrue,  BwSLS(ipgdDocP1), iszNil,fFalse},
{dchSSlash, ficStaticQ,  fFalse, BwSLS(ipgdMac),iszSSlash,fFalse},
{dchIpgdMac,ficIntQ,     fFalse, BwSLS(ipgdMac),iszNil,   fFalse},
{dchSep1,   ficSeparator,fFalse, bwSlsNil,      iszNil,   fFalse},
{dchSAt,    ficStatic,   fFalse, bwSlsNil,      iszSAt,   fFalse},
{dchYaPp,   ficMeasure,  fFalse, BwSLS(yaPp),   iszNil,   fFalse},
{dchSLn,    ficStatic,   fFalse, bwSlsNil,      iszSLn,   fFalse},
{dchLn,     ficIntQ,     fFalse, BwSLS(lnn),    iszNil,   fFalse},
{dchSCol,   ficStatic,   fFalse, bwSlsNil,      iszSCol,  fFalse},
{dchCol,    ficIntQ,     fFalse, BwSLS(col),    iszNil,   fFalse},
{dchSep2,   ficSeparator,fFalse, bwSlsNil,      iszNil,   fFalse},
{dchExBl,   ficIndexed,  fFalse, BwSLS(ExBl),   iszIExBl, fTrue},
{dchCL,     ficIndexed,  fFalse, BwSLS(fCL),    iszICl,   fTrue},
{dchNL,     ficIndexed,  fFalse, BwSLS(fNL),    iszINl,   fTrue},
{dchOtMr,   ficIndexed,  fFalse, BwSLS(OtMr),   iszIOtMr, fTrue},
{dchRec,    ficIndexed,  fFalse, BwSLS(fRec),   iszIRec,  fTrue},

/* placeholder; forces erasure of no-man's-land to right */
/* HACK: dchMax is large enough to guarantee erasure, too small to
	force signed-comparison bugs; StaticQ fic works for erasure for bwSlsNil */

{dchMax,       ficStaticQ,  fFalse, bwSlsNil,      iszNil, fFalse}};

#ifdef WIN23
csconst struct SI3 rgsi3[isiMax3] = { 
/* fic, 	fRightJust, bwSls, 		isz, 		fHilite */
{ficSeparator,	fFalse,	bwSlsNil,		iszNil,		fFalse},	/* Sep0 */
{ficStaticQ,	fFalse, BwSLS(pgn),    	iszSPg,   	fFalse},	/* Pg */
{ficIntQ,		fFalse, BwSLS(pgn),    	iszNil,   	fFalse},
{ficStatic,		fFalse, bwSlsNil,      	iszSSec,  	fFalse},	/* Sec */
{ficIntQ,		fFalse, BwSLS(isedP1), 	iszNil,   	fFalse},
{ficIntQ,		fTrue,  BwSLS(ipgdDocP1),iszNil,	fFalse},
{ficStaticQ,	fFalse, BwSLS(ipgdMac),	iszSSlash,	fFalse},
{ficIntQ,		fFalse, BwSLS(ipgdMac),	iszNil,   	fFalse},
{ficSeparator,	fFalse, bwSlsNil,      	iszNil,   	fFalse},	/* Sep1 */
{ficStatic,   	fFalse, bwSlsNil,      	iszSAt,   	fFalse},	/* At */
{ficMeasure,  	fFalse, BwSLS(yaPp),   	iszNil,   	fFalse},
{ficStatic,   	fFalse, bwSlsNil,      	iszSLn,   	fFalse},	/* Ln */
{ficIntQ,     	fFalse, BwSLS(lnn),    	iszNil,   	fFalse},
{ficStatic,   	fFalse, bwSlsNil,      	iszSCol,  	fFalse},	/* Col */
{ficIntQ,     	fFalse, BwSLS(col),    	iszNil,   	fFalse},
{ficSeparator,	fFalse, bwSlsNil,      	iszNil,   	fFalse},
{ficIndexed,  	fFalse, BwSLS(ExBl),   	iszIExBl, 	fTrue},		/* Ex_Bl */
{ficSepToggle,	fFalse,	bwSlsNil,		iszNil,		fFalse},
{ficIndexed,  	fFalse, BwSLS(fCL),    	iszICl,   	fTrue},		/* CL */
{ficSepToggle,	fFalse,	bwSlsNil,		iszNil,		fFalse},
{ficIndexed,  	fFalse, BwSLS(fNL),    	iszINl,   	fTrue},		/* NL */
{ficSepToggle,	fFalse,	bwSlsNil,		iszNil,		fFalse},
{ficIndexed,  	fFalse, BwSLS(OtMr),   	iszIOtMr, 	fTrue},		/* OtMr */
{ficSepToggle,	fFalse,	bwSlsNil,		iszNil,		fFalse},
{ficIndexed,  	fFalse, BwSLS(fRec),   	iszIRec,  	fTrue},		/* Rec */
{ficSepToggle, 	fFalse, bwSlsNil,      	iszNil, 	fFalse}};	/* isiLast */
#endif /* WIN23 */
#endif

#define cchMaxField cchPromptMax  /* max # of characters in any field */

#define sivNil (-1)


#define lnnStatlineLast	999

struct LCB {        /* Line Cache Block, for status line row, col, ya display */
	struct CA ca;
	int yaPp;
	int lnn;
	int ich;
	int fInsideCell;	/* whether ca is in a cell */
	int ww;
};

#ifdef WIN23
/*
 * Status Line Measurement
 * Note: Assumes that the width of a field is never greater than 256 units.
 */
typedef struct {
	BYTE dxpFld;		/* field width: logical units(pixels in text mode) */
	int  dxpOff;		/* offset from the left side of the screen */
} SLM;					/* Status Line Measurement */

extern void InitStatLine( HDC );

/* Size of separator for 3-D effect */
#define dxpSep0				17		/* ----------G------ */
#define dxpSep0Gray			10		/* ..........^       */
#define dxpSep				17		/* W---GBW---G------ */
#define dxpSep2				13		/* W---GBW---G--     */
#define dxpSepGray1			5		/* ....^             */
#define dxpSepBlack			6		/* .....^            */
#define dxpSepWhite			7		/* ......^           */
#define dxpSepGray2			10		/* ..........^       */

#define dxpSepToggle		10		/* --W----G-- */
#define dxpSepToggleWhite	3		/* ..^        */
#define dxpSepToggleGray	8		/* .......^   */
#define dxpSepToggleEnd		2
#define dxpSepToggleEnd2	4		/* blanks on both ends of SepToggle */
#define dxpFldMax			255
#define dxpLabelField		6		/* value arbitrarily chosen */
#define dxpBetwItems		8		/* value arbitrarily chosen */


	/*
	 * An FT is divided into three bit fields (starting at low order):
	 *	0-2		basic type of field: num, separator, text, double toggle
	 *				(i.e. a toggle with two possible values, like Ex_Bl)
	 *	3-7		data specific to type.  num: number of digits, separator:
	 *				size of separator, text: index into string array
	 */
typedef BYTE FT;					/* type: field type for InitStatLine */

#define ftNum			0
#define ftSep			1
#define	ftText			2
#define ftTogDub		3
#define ftTog			4

#define fftType			0x07
#define fftData			0xF8

#define cchDig2			2
#define cchDig3			3
#define cchDig4			4

#endif /* WIN23 */

