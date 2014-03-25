/* priv.h: Definitions used throughout the EL compiler (but not outside it).
*/
#define OPUS
#ifdef OPUS

#ifdef DEBUG
#ifndef ASSERTS
#define ASSERTS
#endif
#endif

typedef double TBNUM;

# ifndef InvalidateEnv
# include <qsetjmp.h>
# endif

#else /* OPUS */
#include <qsetjmp.h>

#endif /* OPUS */


typedef WORD VAL;	/* operand value */

/* ---------------------------------------------------------------------- */
/* EL global variables
/* ---------------------------------------------------------------------- */

EXTERN struct ELGD **helgdGlobal;
EXTERN struct ELLD *pelldGlobal;
EXTERN SB sbTds;
EXTERN SB sbFrame;
EXTERN SB sbNtabFirst;
EXTERN SB sbStrings;
EXTERN SB sbArrays;

/* ---------------------------------------------------------------------- */
/* Random constants
/* ---------------------------------------------------------------------- */

#define cargMaxProc	20	/* max # args to EL procedure */

/* ---------------------------------------------------------------------- */
/* Interpreter performance
/* ---------------------------------------------------------------------- */

#define celtExpStackMax	256	/* size of operator stack */
#define ceviExpStackMax	256	/* size of operand stack */

#define cchTokenBuf	256	/* how much to read from token stream at once */

#define cbHeapInit	4096	/* initial size of sbTds, sbStrings, sbArrays */

#define cedcCache	5	/* how many ELDI records remembered at once */

/* ---------------------------------------------------------------------- */
/* Globally used data structures
/* ---------------------------------------------------------------------- */

/* AI (Array Instance) -- This is an AD with an ELV attached to it; i.e. a
* typed array.  These things are moved around enough that it makes sense to
* make structs out of them.
*/
struct AI
	{
	AD ad;		/* can be 0 if nonexistent */
	ELV elv;	/* element type -- never elvNil */
	};

/* EV (Expression Value) -- This is here so that it can be part of the ELGD
* structure.
*/
typedef WORD EVI;		/* EV Index */
struct EV
	{
	int elv : 7;
	BOOL fResolved : 1;
	BOOL fAggregate : 1;		/* valid if fResolved */
	BOOL fFree : 1;			/* valid if fResolved */
	BOOL fElx : 1;			/* valid if !fResolved */
	ELK elk : 13;
	union	{
		union	{	/* fResolved = TRUE */
			struct	{	/* fAggregate = TRUE */
				ELV elvAggr;	/* elvArray or record ELV */
				union	{
					struct	{
						struct VAR *pvarAdAggr;
						WORD ibAggr;
						};
					struct	{
						RL rlAggr;
						RPS rpsAggr;
						};
					};
				};
			union	{	/* fAggregate = FALSE */
				NUM num;		/* elvNum */
				int w;			/* elvInt */
				struct	{		/* elvSd */
					SD sd;
					BOOL fFree;
					char **hst;		/* temp */
					};
				RL rl;		/* record locator */
				};
			};
		struct	{	/* fResolved = FALSE */
			ELX elx;		/* fElx = TRUE */
			BOOL fInteractive;	/* fElx = TRUE */
			struct SY *psy;		/* fElx = FALSE */
			};
		};
	};
					

#define cbEv	sizeof(struct EV)
#define eviNil	0xffff
#define ibNil	0xffff	/* ibArray == ibNil means "not an array" */

#define EviOfIev(iev)	((iev) * sizeof(struct EV))
#define IevOfEvi(evi)	((evi) / sizeof(struct EV))


/* EL Global Data -- data shared by all invocations of the EL interpreter.
* When sbTds is allocated, this data block is created, and the global
* variable "hpelgdGlobal" is set to point to it.
*/
struct ELGD
	{
	ELI **heliFirst;	/* most recent invocation */
	ENV env;		/* to suspend all EL invocations */
	struct SSB **hssb;	/* if suspended, points to saved stack */

	/* Eventually, a separate "rerr" value will be defined for each
		* possible error message.  At present, 
	char szError[80];

	/* Pointer to a stack of Control Structure Records, stored in sbTds.
		* See "exec.h" for the definition of these.
		*/
	struct CSR *hcsrStack;	/* Control Structure Records; see EXEC */
	int icsrStackPtr;	/* stack ptr */

	/* For EXP: "elt" and "evi" stacks.  These stacks are allocated in
		* sbTds.
		*/
	unsigned **heltStack;			/* stack of operators */
	int ieltStackPtr;			/* ELT stack pointer */
	int ievStackPtr;			/* EV stack pointer */

	/* Cache of ELDI records (copied from the code segment containing
		* dialog info).
		*/
	struct EDC	/* Element of Dialog Cache */
		{
		ELDI **heldi;
		WORD ieldi;
		} rgedc[cedcCache];
	int iedcNextFree;

	/* State of the SDM callbacks.
		*/
	HID hidSave;			/* dialog we expect a CAB from */
	HCAB hcabSave;			/* saved CAB */
	TMC tmcSave;			/* saved TMC (push button) */

#ifdef DEBUG
	/* Test output flags */

	BOOL fToMainElgd, fToExecElgd, fToExpElgd, fToSymElgd, fToAllocElgd;
	BOOL fToVarElgd, fToCtrlElgd, fToReadElgd, fToArraysElgd;
	BOOL fToCoroutElgd;
	int ichCur;

	int cadAllocated, csdAllocated;
#endif

	/* Stack of EV's (Expression Values).
		*/
	int ievMac;		/* how many EV's are allocated here */
	struct EV rgev[];	/* global list of EV's */
	};
#define cevQuantum	128
#define CbOfElgdCev(cev)	(sizeof(struct ELGD) +			\
					(cev) * sizeof(struct EV))

#define HpevOfEvi(evi)	((struct EV huge *)				\
				((char huge *)ElGlobal(rgev) + (WORD)(evi)))
#define PevOfEvi(evi)	((struct EV *)IbOfHp((char huge *)ElGlobal(rgev) +  \
							(WORD)(evi)))


/* EL Local Data -- data associated with a single invocation of the
* EL interpreter; available anywhere within that invocation.
*/
struct ELLD
	{
	/* ENV to jump to if an error occurs.  (If the macro is suspended,
		* we jump instead (saving the stack) to the ElGlobal(env).
		*/
	ENV envError;
	WORD wStackStart;		/* for stack-space checking */

	/* EXEC data.
		*/
	ELT eltCur;			/* token last read */
	struct SY *psyCur;		/* stored when eltCur = eltIdent */

	/* Store the current locations of the bottom of the ELT and EV stacks.
		*/
	int ieltStackBase, ievStackBase;	/* current bottom of stacks */

	LIB libBufStart, libBufEnd;
	BYTE rgchBuf[cchTokenBuf];
	WORD ibBufCur, ibBufLim;
	WORD ibElcrCur;
	int fExpectLabel;		/* used before calling EltGetCur() */

	/* EXP data.
		*/
	int fExpectOpd;		/* whether to expect an operand next */
	int fExpectAsst;	/* whether to treat first "=" as asst. */
	BOOL fEnableCall;	/* whether to call function */
	BOOL fImplicitParens;	/* whether SUB call lacked outer parentheses */
	struct VAR *pvarTmpRek;	/* VAR which holds a temporary record */

	/* SYM data.
		*/
	SB sbNtab;			/* sb for the name table */
	struct FH *pfhCurFrame;		/* ptr to current frame header */
	struct FH *pfhGlobalFrame;	/* ptr to global frame */
	struct FH *pfhFirstProcFrame;	/* after end of global frame */
	struct PROC *pprocCur;
	BOOL fEnableDefine;		/* for PvarFromPsy */
	BOOL fCall;
	};

/* Define accessor macros for global variables.
*/
#define Global(var)	(pelldGlobal->var)
#define ElGlobal(var)	(((struct ELGD huge *)HpOfSbIb(sbTds,		       \
								*HpOfSbIb(sbTds,	       \
									helgdGlobal)))\
				->var)

#define HpeliOfHeli(heli)	((ELI huge *)HpOfSbIb(sbTds,		\
								*HpOfSbIb(sbTds, (heli))))
#define HpeliFirst()	HpeliOfHeli(ElGlobal(heliFirst))
#define PelcrCur()	((struct ELCR *)(Global(rgchBuf) + Global(ibElcrCur)))

/* TdsVar() is only usable when sbCur is sbTds.
*/
#define TdsVar(var)	((*helgdGlobal)->var)

/* Random macros.
*/
#define CwOfCb(cb)	(((cb) + sizeof(int) - 1)/sizeof(int))

string char *PchString();
string char far *LpchString();
VOID ErrorLsz();

#define StData(sz)	PchString((sz), 1)
#define SzFrame(sz)     PchString((sz), 0, 1)
#define SzCode(sz)	PchString((sz), 0, 2)
#define StCode(sz)	PchString((sz), 1, 2)
#define LszCode(sz)	LpchString((sz), 0, 1)
#define LstCode(sz)	LpchString((sz), 1, 1)
#define Error(sz,arg)	ErrorLsz(LszCode(sz), (arg))

/* Stack space measurement.
*/
extern WORD pstackTop;
#define CbStackAvailable(pbFrame)	((WORD)pbFrame - pstackTop)

/* Definitions for test output.
*/
#ifdef DEBUG

#define fToMain		ElGlobal(fToMainElgd)
#define fToLex		ElGlobal(fToLexElgd)
#define fToExp		ElGlobal(fToExpElgd)
#define fToExec		ElGlobal(fToExecElgd)
#define fToSym		ElGlobal(fToSymElgd)
#define fToAlloc	ElGlobal(fToAllocElgd)
#define fToVar		ElGlobal(fToVarElgd)
#define fToCtrl		ElGlobal(fToCtrlElgd)
#define fToRead		ElGlobal(fToReadElgd)
#define fToArrays	ElGlobal(fToArraysElgd)
#define fToCorout	ElGlobal(fToCoroutElgd)

#else

#define fToMain		0
#define fToLex		0
#define fToExp		0
#define fToExec		0
#define fToSym		0
#define fToAlloc	0
#define fToVar		0
#define fToCtrl		0
#define fToRead		0
#define fToArrays	0
#define fToCorout	0

#endif

/* Define PrintT() whether DEBUG is set or not.
*/
#define PrintT(sz,arg)		PrintTLsz(LszCode(sz), (arg))
#define ETPrintT(sz,arg)	ETPrintTLsz(LszCode(sz), (arg))	/* test app */

/* Definition of the Assert() macro
*/
#ifdef ASSERTS

#define Assert(f)	((f) ? 0 : AssertProc(SzCode(__FILE__), __LINE__))
#define AssertSz(f,sz)  ((f) ? 0 : AssertSzProc(SzFrame(sz),SzFrame(__FILE__),__LINE__))

#else

#define Assert(f)
#define AssertSz(f,sz)

#endif

#define fInvalid	3	/* invalid value for assert checking */




#ifdef OPUS
#include "cmb.h"
#endif /* OPUS */


