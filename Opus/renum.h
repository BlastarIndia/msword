#define levMax  (stcLevLast - stcLevMin + 1)
/*
*  nil number not -1 because 0 number temporarily stored decremented
*  All the others must have the same value.
*/
#define nNil -2
#define levNil  -2
#undef nfcNil
#define nfcNil -2
#undef chNil
#define chNil -2

/*
*  Renumber Paragraph States...states used in a finite state machine to
*  scan alphabetic, arabic, and roman numerals.
*/
/* state => formatting map (mprpsnfc)...depends on order of rps codes below */
/* If you change these orders, change mprpsnfc in renum.c */
#define rpsStart        0
#define rpsArabic       1
#define rpsUCLetter     2
#define rpsLCLetter     3
#define rpsUCRoman      4
#define rpsLCRoman      5
#define rpsPunc         6
#define rpsEnd          7
#define rpsTextMax      rpsPunc /* end of letters and numbers */


/*
*  Renumber Paragraph Modes...used in deciding how many level numbers to
*  generate for a given paragraph.  Set from dialog box and passed to
*  RenumParas.  Must start at 0 and must be consecutive to satisfy some
*  coding assumptions.
*/
#define rpmSingle       0
#define rpmMulti        1
#define rpmLearn        2
#define rpmRemove       3

#define rppAll          0
#define rppNumbered     1
#define rppRemove       2

#define rpfLegal        1
#define rpfOutline      2
#define rpfSequence     3
#define rpfLearn        4

#define rpfOther       -1
#define rpfMin          1
#define rpfMaxAutomatic 4
#define rpfMax          5

#define chNix           0

struct NPP              /* numbered paragraph properties */
		{
		int dxa;        /* indent of paragraph */
		int clev;       /* how many levels of numbers for paragraph */
		int n;          /* current number at lev */
		int nfc;        /* numeric format code for paragraph */
		int chEnd;      /* ending character for number list */
		};
#define cbNPP   (sizeof(struct NPP))
#define cwNPP   (sizeof(struct NPP) / sizeof(int))
