/* opl.h: definitions for routines which maintain Object Property Lists
*/
typedef unsigned ATMT;
#define atmtNil		0
#define atmtKeyword	1
#define atmtString	2
#define atmtInt		3
#define atmtLParen	4	/* for tokenizer only */
#define atmtRParen	5	/* for tokenizer only */
#define atmtEof		6	/* for tokenizer only */

typedef struct _atm
	{
	ATMT atmt;
	union	{
		char *stKeyword;
		char *stString;
		int wInt;
		} u;	/* boo, hiss */
	} ATM;

typedef struct _opl
	{
	struct _opl *poplPropFirst;	/* first entry in property list */
	struct _opl *poplNext;
	int catmFields;			/* how many fields are defined */
	ATM rgatmFields[2];		/* array of fields */
	} OPL;
#define CbOplOfCatm(catm)	(sizeof (OPL) + ((catm) - 2) * sizeof(ATM))
#define iatmMax		20

/* Random stuff that shouldn't be here ...
*/
#define TRUE	1
#define FALSE	0

/* Procedure templates
*/
OPL *PoplGet(void);
void PrintPopl(OPL *, FILE *);
void FreePopl(OPL *);

