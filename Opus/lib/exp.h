/* exp.h: definitions for EXP subsystem */

/* EV (Expression Value) definition -- moved to "priv.h" so that the ELGD
* can contain an array of EV's.
*/
/* #define HpevOfPev(pev)	((struct EV huge *)HpOfSbIb(sbTds, (pev))) */


struct ESB	/* Exp Save Block */
	{
	int ievStackBase, ieltStackBase;
	int fExpectOpd, fExpectAsst, fImplicitParens, fCall;
	};


/* EXP interface procedures */

struct EV *PevParse();
VOID FreePev(struct EV *);

