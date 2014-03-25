/* P R O M P T . H */


#define cchPromptMax    100        /* no prompt may be longer than this */
#define usecPromptTimeout 5000    /* fifteen seconds */

#define nIncrPercent    10


/* prompt input record.

	when an TmcInputPrompt() is active, this structure holds information required
	by the message processors to deal with input.

			|  <prompt text>  |  <input text>
			^                 ^
	index:  1                 ichFirst


*/

struct PIR
	{
	int ichFirst;     /* index of the first char of input */
	int ichMax;       /* max length allowed */
	BOOL fOn;         /* caret is displayed */
	};


/* P R O M P T  P R O G R E S S  R E P O R T */
/*  Structure used by prompt code when reporting progress (% complete, page
	numbers, etc).
*/
struct PPR
	{
	int ich;
	int cch;
	unsigned nLast;
	unsigned nIncr;
	BOOL fAbortCheck;
	struct PPR **hpprPrev;
	char **hstPrev;
#ifdef WIN23
	short xp;				/* pixel offset of field in window */
#endif /* WIN23 */
	};


/* P R O M P T  I N P U T  S T A T U S */
#define pisNormal       0
#define pisInput        1
#define pisModal        2
#define pisAbortCheckMin 3  /* Abort Check active if >= 3 */
							/* aborted if !pis&1 */



/* Prompt Display Class Masks */
#define pdcmSL          (0x0001)
#define pdcmPmt         (0x0002)
#define pdcmCreate      (0x0004)
#define pdcmImmed       (0x0010)
#define pdcmFullWidth	(0x0020)
#define pdcmPermanent   (0x0100)
#define pdcmRestOnInput (0x0200)
#define pdcmTimeout     (0x0400)
#define pdcmRestore	(0x8000)

/* Prompt Display Classes */
#define pdcInput        (pdcmPmt | pdcmCreate | pdcmPermanent)
#define pdcAbortCheck   (pdcmPmt | pdcmCreate | pdcmPermanent)
#define pdcCkReport     (pdcmPmt | pdcmPermanent)
#define pdcReport       (pdcmPmt | pdcmCreate | pdcmPermanent)
#define pdcAdvise       (pdcmSL | pdcmImmed | pdcmTimeout | pdcmRestOnInput)
#define pdcAdvise2      (pdcmSL | pdcmImmed | pdcmPmt | pdcmCreate | pdcmTimeout | pdcmRestOnInput)
#define pdcMode         (pdcmSL | pdcmImmed | pdcmPmt | pdcmCreate | pdcmPermanent)
#define pdcPic          (pdcmImmed | pdcmPmt | pdcmCreate | pdcmPermanent)
#define pdcMenuHelp	(pdcmSL | pdcmImmed | pdcmRestOnInput | pdcmFullWidth)
#define pdcDefault      (pdcmSL | pdcmImmed | pdcmTimeout)
#define pdcNotify       (pdcAdvise | pdcmPmt | pdcmCreate)
#define pdcRestore      (pdcmRestore)
#define pdcRestoreImmed (pdcmRestore|pdcmImmed)


/* Modal Mode Options */
#define mmoNormal           0x0000
#define mmoFalseOnMouse     0x0100
#define mmoBeepOnMouse      0x0400
#define mmoTrueOnMouse	    0x0800
#define mmoUpdateWws        0x1000
#define mmoNewestCmg        0x00ff

#define MmoFromCmg(cmg) ((cmg) & mmoNewestCmg)

struct PPR **HpprStartProgressReport();
NATIVE FQueryAbortCheckProc();
EXPORT AdjustPrompt();

extern int vpisPrompt;
#define FQueryAbortCheck() (vpisPrompt==pisNormal?fFalse:FQueryAbortCheckProc())
#ifdef WIN23
/* These give better spacing for prompts */
#define ypPromptTop		2
#define xpPromptLeft	5
#endif /* WIN23 */

