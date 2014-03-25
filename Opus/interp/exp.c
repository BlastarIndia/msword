/* exp.c: top level routines for the EXP subsystem.
*/
#include <qwindows.h>
#include <qsetjmp.h>
#include <uops.h>
#include <el.h>
#include "eltools.h"
#include <rerr.h>

#define EXTERN	extern	/* disable global defs */
#include "priv.h"
#include "exp.h"
#include "sym.h"
#include "_exp.h"


extern WORD WParseDttm();

EVI EviPop(), EviTop(), EviPush();
ELD EldExecutePproc();
unsigned IbRefArray();
ELV ElvFromIeldiElk();
RPS RpsFromIelfd();
VOID StoreItemToRl(), FreeRek(), InitHprek();
ELV ElvReadItemFromRl();
ELV ElvOfRl();
REK huge *HprekOfRl();


typedef int (* PFN)();
extern PFN PfnWParseFromHidIag();


csconst int mpntelv[] = {
	elvNum, elvNum, elvSd};


csconst int mpelvcbVal[] =
{
	0, sizeof(NUM), sizeof(int), sizeof(SD), sizeof(AD),
			sizeof(struct VAR *), sizeof(ENV)};


#define CbValFromElv(elv)	(FRecordElv(elv) ? sizeof(REK)		\
							: mpelvcbVal[elv])

/* Default ELV associated with an ELP.
*/
csconst BYTE mpelpelv[] =
{
	elvNum, elvInt, elvSd, elvSd, elvNil, elvNil, elvNil, elvSd};


#define opcNil	1000

#ifdef OLD_AND_BOGUS
csconst int mpeltopc[] =
{
	opcNil,	/* eltNil */
	1,		/* eltMinus */
	1,		/* eltNot */
	6,		/* eltAnd */
	7,		/* eltOr */
	12,		/* eltLParen */
	11,		/* eltRParen */
	3,		/* eltAdd */
	3,		/* eltSubtract */
	4,		/* eltDiv */
	4,		/* eltMul */
	4,		/* eltMod */
	5,		/* eltEq */
	5,		/* eltNe */
	5,		/* eltLt */
	5,		/* eltGt */
	5,		/* eltLe */
	5,		/* eltGe */
	10,		/* eltComma */
	12,		/* eltSubscript */	/* was 0 */
	opcNil,		/* eltOperand */
	9,		/* eltElkGets */
	20};		/* eltEndExp */


#endif

csconst int mpeltopc[] =
{
	opcNil,	/* eltNil */
	1,		/* eltMinus */
	6,		/* eltNot */

	7,		/* eltAnd */
	8,		/* eltOr */

	12,		/* eltLParen */
	11,		/* eltRParen */

	4,		/* eltAdd */
	4,		/* eltSubtract */

	2,		/* eltDiv */
	2,		/* eltMul */

	3,		/* eltMod */

	5,		/* eltEq */
	5,		/* eltNe */
	5,		/* eltLt */
	5,		/* eltGt */
	5,		/* eltLe */
	5,		/* eltGe */

	10,		/* eltComma */
	12,		/* eltSubscript */	/* was 0 */
	opcNil,		/* eltOperand */
	9,		/* eltElkGets */
	20};		/* eltEndExp */




#define AssertStacksEmpty()						\
	Assert(ElGlobal(ieltStackPtr) == Global(ieltStackBase) &&	\
			ElGlobal(ievStackPtr) == Global(ievStackBase))


/* %%Function:InitExp %%Owner:bradch */
InitExp(heliPrev)
/* Initializes EXP data structures in the ELLD block.
*/
ELI **heliPrev;
{
	/* Initialize global EXP variables.
		*/
	Global(ieltStackBase) = ElGlobal(ieltStackPtr);
	Global(ievStackBase) = ElGlobal(ievStackPtr);
	Global(fExpectOpd) = TRUE;
	Global(fExpectAsst) = FALSE;
	Global(fCall) = FALSE;
#ifdef DEBUG
	Global(fImplicitParens) = fInvalid;
#endif
}


/* %%Function:ClearExp %%Owner:bradch */
ClearExp()
	/* Clears the state of the EXP subsystem (assumed to have already been
	* initialized).  Used for ON ERROR.
	*/
{
	ElGlobal(ieltStackPtr) = Global(ieltStackBase);
	PopToIev(Global(ievStackBase));		/* pop and free strings */
	Global(fExpectOpd) = TRUE;
	Global(fExpectAsst) = FALSE;
	Global(fCall) = FALSE;
}


/* %%Function:SaveExp %%Owner:bradch */
SaveExp(pesb)
/* Saves the state of the EXP subsystem in the specified block, and sets up
* EXP to read an expression from the beginning.  Used for EL procedure calls.
*/
struct ESB *pesb;
{
	pesb->ievStackBase = Global(ievStackBase);
	pesb->ieltStackBase = Global(ieltStackBase);
	pesb->fExpectOpd = Global(fExpectOpd);
	pesb->fExpectAsst = Global(fExpectAsst);
	pesb->fImplicitParens = Global(fImplicitParens);
	pesb->fCall = Global(fCall);

	Global(ievStackBase) = ElGlobal(ievStackPtr);
	Global(ieltStackBase) = ElGlobal(ieltStackPtr);
	Global(fExpectOpd) = TRUE;
	Global(fExpectAsst) = FALSE;
	Global(fCall) = FALSE;
#ifdef DEBUG
	Global(fImplicitParens) = fInvalid;
#endif
}


/* %%Function:RestoreExp %%Owner:bradch */
RestoreExp(pesb)
/* Restores the EXP state from the specified ESB.  Used when an EL procedure
* returns.
*/
struct ESB *pesb;
{
	Global(ievStackBase) = pesb->ievStackBase;
	Global(ieltStackBase) = pesb->ieltStackBase;
	Global(fExpectOpd) = pesb->fExpectOpd;
	Global(fExpectAsst) = pesb->fExpectAsst;
	Global(fImplicitParens) = pesb->fImplicitParens;
	Global(fCall) = pesb->fCall;

	/* Assert that the stacks are empty.
		*/
	Assert(Global(ievStackBase) == pesb->ievStackBase &&
			Global(ieltStackBase) == pesb->ieltStackBase);
}


/* %%Function:FInitExpTds %%Owner:bradch */
FInitExpTds()
{
	ElGlobal(ievMac) = cevQuantum;		/* EV pool size */
	ElGlobal(ieltStackPtr) = -1;
	ElGlobal(ievStackPtr) = -1;

	return (ElGlobal(heltStack) = PpvAllocCb(sbTds,
			sizeof(ELT) * celtExpStackMax)) != 0;
}


/* %%Function:FExpIn %%Owner:bradch */
BOOL FExpIn(eltNew)
/* Parses a token into an expression being built.  Returns TRUE if the
* token was an expression token, or FALSE if it was something else.
*
* The special elt value eltEndExp causes FExpIn() to finish parsing the
* expression.  The result ends up on the EV stack, from where it can be
* retrieved using EviPop().
*/
ELT eltNew;
	/* CSNative as called repeatedly while parsing macro commands */
{{  /* NATIVE - FExpIn */
	Assert(SbCur() == 1);
LTop:
	switch (eltNew)
		{
	default:	/* an operator */
			{
			int opcNew;
			int eltTop, ielt;

			if (eltNew >= eltOprLim)
				return FALSE;		/* not a valid operator */

			if (Global(fExpectOpd))
				{
				switch (eltNew)
					{
				case eltRParen:
						{
						ELT eltT;

						eltT = RgeltStack()[ElGlobal(ieltStackPtr)];
						if (eltT == eltSubscript || eltT == eltLParen)
							{
							struct EV huge *hpevNew;

					/* Call with no arguments -- push a
						* null EV ...
						*/
							hpevNew = HpevOfEvi(EviPush());
							hpevNew->fResolved = TRUE;
							hpevNew->fAggregate = FALSE;
							hpevNew->elv = elvNil;
							break;
							}
						}
				/* fall through */
				default:
					/* Error: illegal expression (elt eltNew) */
					RtError(rerrSyntaxError);

				case eltComma:
						{
						struct EV huge * hpevNew;

						hpevNew = HpevOfEvi(EviPush());
						hpevNew->fResolved = TRUE;
						hpevNew->fFree = FALSE;
						hpevNew->fAggregate = FALSE;
						hpevNew->elv = elvNil;
						hpevNew->elk = elkNil;
						}
					break;

				case eltSubtract:
					eltNew = eltMinus;	/* unary minus */
				/* FALL THROUGH */

				case eltLParen:
				case eltNot:
					break;
					}
				}
			else
				{
				if (eltNew == eltLParen)
					eltNew = eltSubscript;
				}

			if (eltNew == eltLParen || eltNew == eltSubscript)
				opcNew = 0;	/* push as highest precedence */
			else
				opcNew = mpeltopc[eltNew];

		/* Pushing an operator.  If it's not a unary operator, we
			* first apply all operators on the stack with a lesser "opc"
			* value than that of eltNew.
			*/
			if (eltNew < eltUnaryLim)
				{
				ielt = ElGlobal(ieltStackPtr);
				goto DontApply;		/* indentation control */
				}

			for (ielt = ElGlobal(ieltStackPtr);
					ielt > Global(ieltStackBase) &&
					mpeltopc[eltTop = RgeltStack()[ielt]] <= opcNew;
					ielt--)
				{
				switch (eltTop)		/* operator to apply */
					{
				case eltComma:
				/* Don't take any action for comma; it gets
					* dealt with later.
					*/
					goto BreakForIelt;	/* need C# */

				case eltLParen:
					RtError(rerrMissingRightParen);
					break;

				case eltRParen:
				/* eltRParen can only get on the ELT stack if
					* it is pushed by DoSubscript() to indicate
					* that a list of expressions has been parsed.
					*/
					if (eltNew != eltEndExp)
						{
					/* Don't allow any more operators.
						*/
						return FALSE;
						}
					break;

				case eltElkGets:
						{
						EVI eviElk, eviValue;
						struct EV huge *hpevElk, huge *hpevValue;

				/* Assign a value to an argument-name keyword.
					* The top of the EV stack contains the value.
					* Top-1 contains an EV labelled with the elk,
					* and containing its initial value of -1.
					*/
						hpevValue = HpevOfEvi(eviValue = EviPop());
						hpevElk = HpevOfEvi(eviElk = EviTop());

				/* If the left operand isn't a keyword,
					* eltElkGets should not be possible.
					*/
						Assert(hpevElk->elk != elkNil);

				/* Check for conflicting keywords, or two
					* assignments to a single keyword.
					*/
						if (hpevValue->elk != elkNil ||
								!(hpevElk->elv == elvInt &&
								hpevElk->w == 1))
							{
					/* Error: invalid argument name */
							RtError(rerrSyntaxError);
							}

				/* Attach the keyword to the value, and leave
					* the combination on top of the stack.
					*/
						hpevValue->elk = hpevElk->elk;
						BLTBH(hpevValue, hpevElk, sizeof(struct EV));
						break;
						}
				default:
						{
						int elvArgs;
						struct EV huge *hpevRight;
						NUM num1, num2;

#ifdef DEBUG
						if (fToExp)
							DumpApply(eltTop);
#endif

				/* Apply a normal left-associative operator
					* (unary or binary).
					*/
						hpevRight = HpevOfEvi(EviTop());
						if (hpevRight->elv == elvSd)
							{
							elvArgs = elvSd;
							}
						else
							{
							elvArgs =
								/*eltTop == eltMod || */
							(eltTop >= eltNot && eltTop <= eltOr)
									? elvInt : elvNum;
							}

						switch (elvArgs)
							{
						default:
					/* Error: operations on elv=elvArgs NYI */
							RtError(rerrInternalError);
						case elvNum:
							ApplyNumElt(eltTop);
							break;
						case elvSd:
							ApplySdElt(eltTop);
							break;
						case elvInt:
							ApplyIntElt(eltTop);
							break;
							}
						break;
						}
			/* end switch (eltTop) */
					}
				}
BreakForIelt:
			ElGlobal(ieltStackPtr) = ielt;

DontApply:
			if (eltNew == eltEq)
				{
				struct EV huge *hpevTop;

			/* Check whether to treat "=" as an assignment
				* operator instead of a comparison.
				*/
				if (Global(fExpectAsst) &&
						ielt == Global(ieltStackBase))
					{
				/* Assignment at the top level in an
					* expression; reject the eltEq so that EXEC
					* can see it.
					*/
					return FALSE;
					}

			/* Look at the last operand to see if it's an ELK.
				* (There'd better be an operand, since this is a
				* binary operator).
				*/
				if (HpevOfEvi(EviTop())->elk != elkNil)
					eltNew = eltElkGets;
				}

		/* Push eltNew on the operator stack.
			*/
			Global(fExpectOpd) = TRUE;
			switch (eltNew)
				{
			default:
				/* REVIEW cslbug: two lines for native compiler */
				ElGlobal(ieltStackPtr) = ++ielt;
				if (ielt >= celtExpStackMax)
				/* Error: expression too complex */
					RtError(rerrExpressionTooComplex);
				RgeltStack()[ielt] = eltNew;
				break;

			case eltEndExp:
			/* End of expression.
				*/
#ifdef DEBUG
				if (fToExp)
					DumpEvi(EviTop());
#endif
				break;

			case eltRParen:
			/* The elt stack should contain either eltLParen or
				* eltSubscript (possibly with a succession of
				* eltComma's above it).
				*/
				Global(fExpectOpd) = FALSE;

				if (ielt <= Global(ieltStackBase))
					{
				/* eltRParen without eltLParen: reject this token.
					*/
					return FALSE;
					}

				if (eltTop == eltLParen)
					{
				/* Parentheses around 0 or 1 items ...
					*/
					EVI evi;

					evi = EviOfIev(ElGlobal(ievStackPtr));
					if (HpevOfEvi(evi)->elv != elvNil)
						{
						if (!Global(fCall))
							{
					/* Parentheses around a single item,
						* so resolve it (pass by value).
						*/
							ResolveEvi(EviOfIev(ElGlobal(
									ievStackPtr)),
									elvNil,
									hpNil);
							}

					/* Pop the eltLParen.
						*/
						ElGlobal(ieltStackPtr)--;

						break;
						}
					}
			/* Execute an array reference or function call.
				*/
				if (!FDoSubscript())
					return FALSE;
				break;

		/* end switch (eltNew) */
				}

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif

			break;
			}

	case eltHash:
		/* Just ignore this as long as it's 
			not being used as an operator */
		if (!Global(fExpectOpd))
			RtError(rerrSyntaxError);
		return TRUE;

	case eltInt:
			{
		/* FUTURE: This is just a hack to get integer tokens to
			be accepted.  They are converted to num's for
			the rest of the interpreter. */
			struct EV huge *hpevNew;
			NUM num;

			if (!Global(fExpectOpd))
			/* Error: illegal expression */
				RtError(rerrSyntaxError);
			Global(fExpectOpd) = FALSE;

			hpevNew = HpevOfEvi(EviPush());
			hpevNew->elv = elvNum;
			hpevNew->fResolved = TRUE;
			hpevNew->fAggregate = FALSE;
			hpevNew->elk = elkNil;

		/* convert our int to a num! */
			CNumInt(PelcrCur()->wInt);
			StiNum(&num);

			BLTBX((char far *) &num, LpOfHp(&hpevNew->num), sizeof(NUM));

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif
			break;
			}

	case eltNum:
			{
			struct EV huge *hpevNew;

			if (!Global(fExpectOpd))
			/* Error: illegal expression/1 */
				RtError(rerrSyntaxError);
			Global(fExpectOpd) = FALSE;

			hpevNew = HpevOfEvi(EviPush());
			hpevNew->elv = elvNum;
			hpevNew->fResolved = TRUE;
			hpevNew->fAggregate = FALSE;
			hpevNew->elk = elkNil;

			BLTBX((char far *)&PelcrCur()->num, LpOfHp(&hpevNew->num),
					sizeof(NUM));

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif
			break;
			}
	case eltString:
			{
			SD sd;
			struct EV huge *hpevNew;

			if (!Global(fExpectOpd))
			/* Error: illegal expression/2 */
				RtError(rerrSyntaxError);
			Global(fExpectOpd) = FALSE;

			if (PelcrCur()->cchString == 0)
				sd = 0;
			else if ((sd = SdCopySt(&PelcrCur()->cchString)) == sdNil)
				RtError(rerrOutOfMemory);

			hpevNew = HpevOfEvi(EviPush());

			hpevNew->fFree = TRUE;
			hpevNew->fResolved = TRUE;
			hpevNew->fAggregate = FALSE;
			hpevNew->elv = elvSd;
			hpevNew->sd = sd;
			hpevNew->elk = elkNil;

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif
			break;
			}
	case eltIdent:
			{
			struct SY huge * hpsy;
			struct EV huge * hpevNew;

			if (!Global(fExpectOpd))
			/* Error: illegal expression/3 */
				RtError(rerrSyntaxError);
			Global(fExpectOpd) = FALSE;

			hpevNew = HpevOfEvi(EviPush());
			hpevNew->fElx = FALSE;
			hpevNew->fResolved = FALSE;
			hpevNew->psy = Global(psyCur);
/* PrintT("pushing ident; psy = %u", hpevNew->psy);
/* PrintT("; name = ", 0);
/* PrintHst(((struct SY huge *)HpOfSbIb(Global(sbNtab), hpevNew->psy))->st);
/* PrintT("\n", 0);
/**/
			hpevNew->elv = HpsyOfPsy(hpevNew->psy)->elv;
			hpevNew->elk = elkNil;

		/* Handle no-parameter function calls */
			if (!Global(fEnableCall))
				break;
			hpsy = HpsyOfPsy(hpevNew->psy);
			if (hpsy->syt == sytProc)
				{
				struct PROC huge * hpproc;

				hpproc = HpOfSbIb(Global(sbNtab), hpsy->pproc);
				if (hpproc->carg != 0 || hpproc->elv == elvNil)
					break;
				}
			else  if (hpsy->syt == sytDkd)
				{
				DKD huge * hpdkd;

				hpdkd = HpOfSbIb(Global(sbNtab), hpsy->pdkd);
				if (hpdkd->idktMacParam != 0 || 
						hpdkd->dktRet == dktNone)
					break;
				}
			else
				break;

			FExpIn(eltLParen);
			eltNew = eltRParen;
			goto LTop;

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif
			break;
			}

	case eltSpell:
	case eltThes:
			{
			int w;
			struct EV huge *hpevNew;
			NUM num;

			if (!Global(fExpectOpd))
				{
				/* Error: illegal expression */
				RtError(rerrSyntaxError);
				}

			Global(fExpectOpd) = TRUE;
			
			ElGlobal(ieltStackPtr) += 1;
			if (ElGlobal(ieltStackPtr) >= celtExpStackMax)
				/* Error: expression too complex */
				RtError(rerrExpressionTooComplex);
			
			/* Poor csl can't compile this line in native... */
			{{ RgeltStack()[ElGlobal(ieltStackPtr)] = eltNew; }}

			{
			int iev;
			struct ESB esb;
			iev = ElGlobal(ievStackPtr);
			SaveExp(&esb);
			w = eltNew == eltSpell ? WDoElSpell(TRUE) : WDoElThes(TRUE);
			PopToIev(iev);
			RestoreExp(&esb);
			}
		
			ElGlobal(ieltStackPtr) -= 1;
		
			Global(fExpectOpd) = FALSE;

			hpevNew = HpevOfEvi(EviPush());
			hpevNew->elv = elvNum;
			hpevNew->fResolved = TRUE;
			hpevNew->fAggregate = FALSE;
			hpevNew->elk = elkNil;
			hpevNew->fElx = FALSE;

			/* convert our int to a num! */
			CNumInt(w);
			StiNum(&num);

			BLTBX((char far *) &num, LpOfHp(&hpevNew->num), sizeof(NUM));

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif

			break;
			}

	case eltElx:	/* e.g. ".NAME" */
			{
			struct EV huge *hpevNew;

			if (!Global(fExpectOpd))
			/* Error: illegal expression/4 */
				RtError(rerrSyntaxError);
			Global(fExpectOpd) = FALSE;

			hpevNew = HpevOfEvi(EviPush());
			hpevNew->fResolved = FALSE;
			hpevNew->fElx = TRUE;
			hpevNew->elx = PelcrCur()->elx;
			hpevNew->fInteractive = FALSE;
			hpevNew->elv = elvNil;
			hpevNew->elk = elkNil;

#ifdef DEBUG
			if (fToExp)
				DumpOpr(eltNew);
#endif
			break;
			}

	case eltUserElk:
	case eltElk:
			{
			ELK elk;
			ELT elt;
			struct EV huge *hpevNew;


			elk = (eltNew == eltElk) ? 
					PelcrCur()->elk : /* std elk */
			ElkUserSt(&PelcrCur()->cch); /* usr elk */

			if (Global(fExpectOpd))
				{
			/* ELK when an operand is expected.  This is used to
				* assign values to ELK's during an ELX call, e.g.
				*	OPENFILE .NAME = "Zeke.txt"
				*/
				Global(fExpectOpd) = FALSE;

				if (ElGlobal(ieltStackPtr) != Global(ieltStackBase))
					{
					/* REVIEW cslbug: two lines for native compiler */
					elt = RgeltStack()[ElGlobal(ieltStackPtr)];
					if (elt != eltComma&&
							elt != eltLParen && elt != eltSubscript)
						{
				/* Error: illegal use of argument-name kwd */
						RtError(rerrSyntaxError);
						}
					}

				hpevNew = HpevOfEvi(EviPush());
				hpevNew->fResolved = TRUE;
				hpevNew->fAggregate = FALSE;
				hpevNew->elk = elk;
				hpevNew->elv = elvInt;
				hpevNew->w = 1;

#ifdef DEBUG
				if (fToExp)
					DumpOpr(eltNew);
#endif
				}
			else
				{
			/* ELK when an operator is expected.  This is used to
				* set or reference a field from a record, e.g.
				*	X$ = OPENREC .NAME (or "OPENREC.NAME")
				*/
				struct EV huge *hpevTop;
				EVI evi;
				ELV elv;
				int ielfd;
				RPS rps;

			/* Resolve the EV so that hpevTop->elv is a record type
				* (we don't care what record type).
				*/
				ResolveEvi(evi = EviTop(),
						ElvFromIeldi(ieldiNil), hpNil);
				hpevTop = HpevOfEvi(evi);

				if (!hpevTop->fAggregate)
					{
					RL rl;

				/* This is one of the two places fAggregate
					* gets turned on; the other is DoSubscript().
					*/
					hpevTop->fAggregate = TRUE;
					rl = hpevTop->rl;

					hpevTop->elvAggr = hpevTop->elv;
					hpevTop->rlAggr = rl;
					}

			/* Verify that elk is a valid field of record type
				* hpevTop->elv, and determine the elv of the field.
				*/
				elv = ElvFromIeldiElk(IeldiFromElv(hpevTop->elv),
						elk, &ielfd, &rps);
				hpevTop = HpevOfEvi(evi);	/* re-validate */
				hpevTop->elv = elv;
				hpevTop->rpsAggr = rps;

				Assert(!Global(fExpectOpd));	/* leave it FALSE */
				}
			break;
			}
	/* end switch (eltNew) */
		}

	return TRUE;
}}


/* %%Function:FExpInPev %%Owner:bradch */
BOOL FExpInPev(pev)
/* Pushes the EV on the EV stack.
*/
struct EV *pev;
{
	Assert(pev->fResolved && Global(fExpectOpd));
	BLTBH((char huge *)pev, HpevOfEvi(EviPush()), sizeof(struct EV));
	Global(fExpectOpd) = FALSE;

#ifdef DEBUG
	if (fToExp)
		PrintT("EXP: push EV\n", 0);
#endif
	return TRUE;
}


/* %%Function:EviPop %%Owner:bradch */
EVI EviPop()
	/* Pops the top EV off the expression stack and returns it.
	*/
{
	EVI evi;

	Assert(ElGlobal(ievStackPtr) > Global(ievStackBase));

	return EviOfIev(ElGlobal(ievStackPtr)--);
}


/* %%Function:PopToIev %%Owner:bradch */
PopToIev(ievDest)
/* Pops EV's off the EV stack, freeing SD's as necessary, until ievDest is
* reached.  The EV stack pointer ends up set to ievDest.
*/
int ievDest;
{
	int ievT;

#ifdef DEBUG
	if (fToExp)
		PrintT("EXP: PopToIev (ievDest = %u)\n", ievDest);
#endif
	Assert(ievDest <= ElGlobal(ievStackPtr));
	for (ievT = ElGlobal(ievStackPtr); ievT > ievDest; ievT--)
		{
		struct EV huge *hpevT = HpevOfEvi(EviOfIev(ievT));

		if (hpevT->fResolved && !hpevT->fAggregate &&
				hpevT->fFree)
			{
			switch (hpevT->elv)
				{
			default:
				/* No need to free an EV containing a record,
					* since it only contains a reference to a
					* record variable ...
					*/
				break;
			case elvSd:
				FreeSd(hpevT->sd);
				break;
				}
			}
		}

#ifdef DEBUG
	if (fToExp)
		PrintT("EXP: end PopToIev()\n", 0);
#endif
	ElGlobal(ievStackPtr) = ievDest;
}


/* %%Function:EviTop %%Owner:bradch */
EVI EviTop()
	/* Returns a pointer to the top EV on the expression stack.
	*/
{
	if (ElGlobal(ievStackPtr) == Global(ievStackBase))
		RtError(rerrSyntaxError);
	return EviOfIev(ElGlobal(ievStackPtr));
}


/* %%Function:FreeExpEvi %%Owner:bradch */
VOID FreeExpEvi(evi)
/* Frees an expression tree (i.e. one EV and all subtrees).
*/
EVI evi;
{
	RtError(rerrInternalError);
}


/* ---------------------------------------------------------------------- */
/* Private EXP procedures
/* ---------------------------------------------------------------------- */


/* %%Function:IeldiFromElx %%Owner:bradch */
WORD IeldiFromElx(elx)
ELX elx;
{
	extern ModeError();
	ELE ele;

	(*HpeliFirst()->pfnGetInfoElx)(elx, &ele, ElGlobal(heliFirst), FALSE, cmmBuiltIn);

	if (ele.pfn == ModeError)
		RtError(rerrModeError);

	if (ele.elr != elrDialog)
		RtError(rerrTypeMismatch);

	return ele.ieldi;
}


/* %%Function:ElvFromIeldiElk %%Owner:bradch */
ELV ElvFromIeldiElk(ieldi, elk, pielfd, prps)
/* Verifies that "elk" is a field of the specified dialog, and returns the
* elv of that field.
*/
WORD ieldi;
ELK elk;
int *pielfd;
RPS *prps;
{
	ELDI **heldi, *peldi;
	int ielfd;

	Assert(SbCur() == 1);

#ifdef DEBUG
	if (fToExp)
		{
		PrintT("EXP: ElvFromIeldiElk(%u", ieldi);
		PrintT(", %u) = ", elk);
		}
#endif

	heldi = HeldiFromIeldi(ieldi);

	SetSbCur(sbTds);					/* /SB\ */
	peldi = *heldi;

	/* Look for the field in the ELDI record.
		*/

	for (ielfd = 0; ielfd < peldi->celfd; ielfd++)
		{
		if (peldi->rgelfd[ielfd].elk == elk)
			{
			ELFD *pelfd;
			ELV elv;
			WORD iag;
			TMC tmc;

			pelfd = &peldi->rgelfd[ielfd];
			elv = pelfd->elv;
			iag = pelfd->iag;
			tmc = pelfd->tmc;
			ResetSbCur();				/* \SB/ ... */

			*pielfd = ielfd;
			if (elv == elvNil)
				{	/* -- push button field -- */
				prps->ielfd = ielfd;
				elv = elvInt;
				}
			else		/* -- normal field -- */				
				{
				prps->ielfd = ielfdNil;
				prps->iag = iag;
				}
#ifdef DEBUG
			if (fToExp)
				{
				PrintElv(elv);
				PrintT("\n", 0);
				}
#endif
			return elv;
			}
		}
	ResetSbCur();						/* \SB/ */

	/* We didn't find it ...
		*/
#ifdef DEBUG
	if (fToExp)
		PrintT("\n", 0);
#endif
	/* Error: undefined field of dialog record */
	RtError(rerrUndefDlgRecField);
}


/* %%Function:ApplyNumElt %%Owner:bradch */
ApplyNumElt(elt)
/* Applies an operator (accepting NUM args) to the top two EV's on the EV
* stack.  The result is pushed on the EV stack.
*/
ELT elt;
	/* CSNative as used repeatedly for evaluating numerical and
		logical operations of Opel */
{{ /* NATIVE - ApplyNumElt */

	EVI eviResult;
	struct EV huge *hpevLeft;
	ELV elvResult;
	NUM numT, numLeft, numRight;
	EVI eviLeft, eviRight;

	if (elt >= eltUnaryLim)
		eviRight = EviPop();
	eviLeft = EviTop();

	ResolveEvi(eviLeft, elvNum, (NUM huge *)&numLeft);

	if (elt >= eltUnaryLim)
		ResolveEvi(eviRight, elvNum, (NUM huge *)&numRight);

	elvResult = (elt >= eltEq && elt <= eltGe) ? elvInt : elvNum;

	hpevLeft = HpevOfEvi(eviLeft);
	Assert(hpevLeft->fResolved && !hpevLeft->fAggregate);
	hpevLeft->elv = elvResult;

	switch (elt)
		{
		MTC mtc;
#ifdef DEBUG
	default:
		Assert(FALSE);
		RtError(rerrInternalError);
#endif

	case eltEq:
		mtc = Cmp2Num(&numLeft, &numRight);
		hpevLeft->w = (mtc.w2 == 0) ? -1 : 0;
		break;

	case eltNe:
		mtc = Cmp2Num(&numLeft, &numRight);
		hpevLeft->w = (mtc.w2 != 0) ? -1 : 0;
		break;

	case eltLt:
		mtc = Cmp2Num(&numLeft, &numRight);
		hpevLeft->w = (mtc.w2 < 0) ? -1 : 0;
		break;

	case eltLe:
		mtc = Cmp2Num(&numLeft, &numRight);
		hpevLeft->w = (mtc.w2 <= 0) ? -1 : 0;
		break;

	case eltGt:
		mtc = Cmp2Num(&numLeft, &numRight);
		hpevLeft->w = (mtc.w2 > 0) ? -1 : 0;
		break;

	case eltGe:
		mtc = Cmp2Num(&numLeft, &numRight);
		hpevLeft->w = (mtc.w2 >= 0) ? -1 : 0;
		break;

	case eltAdd:
		Add2Num(&numLeft, &numRight);
		break;

	case eltSubtract:
		Sub2Num(&numLeft, &numRight);
		break;

	case eltMul:
		Mul2Num(&numLeft, &numRight);
		break;

	case eltDiv:
		Div2Num(&numLeft, &numRight);
		break;

	case eltMinus:
		LdiNum(&numLeft);
		NegNum();
		break;

	case eltMod:
			{
			int w, w1, w2;

			LdiNum(&numLeft);
			LdiNum(&numRight);
		/* need to round up (because QB does) */
			w2 = CIntNum();
			CNumInt(1);
			CNumInt(2);
			DivNum();
			AddNum();
			w1 = CIntNum();
			if (w2 == 0)
				RtError(rerrDivisionByZero);
			w = w1 % w2;
			CNumInt(w);
			}
		break;
		}

	if (elvResult == elvNum)
		{
		StiNum(&numT);
		BLTBH((char huge *)&numT, &hpevLeft->num, cbNUM);
		}
}}


/* %%Function:ApplySdElt %%Owner:bradch */
ApplySdElt(elt)
/* Applies an operator (accepting SD args) to the top two entries on the EV
* stack.  The result is pushed on the EV stack.
*/
ELT elt;
{
	EVI eviLeft, eviRight;
	struct EV huge *hpevLeft, huge *hpevRight;
	SD sdLeft, sdRight;
	BOOL fFreeLeft, fFreeRight;

	if (elt < eltUnaryLim)	/* no unary string operators */
		goto LTypeError;

	/* Get EVI's pointing to the two arguments, but leave them both on the
		* stack.  That way, if we get an error trying to do the operation,
		* both arguments will be freed if necessary.
		*/
	eviRight = EviTop();
	eviLeft = eviRight - EviOfIev(1);

	ResolveEvi(eviRight, elvSd, (SD huge *)&sdRight);
	hpevRight = HpevOfEvi(eviRight);
	fFreeRight = hpevRight->fFree;

	ResolveEvi(eviLeft, elvSd, (SD huge *)&sdLeft);
	hpevLeft = HpevOfEvi(eviLeft);
	fFreeLeft = hpevLeft->fFree;

	switch (elt)
		{
	default:
		/* Error: illegal string operator */
LTypeError:
		RtError(rerrTypeMismatch);
	case eltAdd:
		hpevLeft->sd = SdConcat(sdLeft, sdRight);
		hpevLeft->fFree = TRUE;
		break;
	case eltEq:
		hpevLeft->elv = elvInt;
		hpevLeft->w = (WCmpSdSd(sdLeft, sdRight) == 0) ? -1 : 0;
		break;
	case eltNe:
		hpevLeft->elv = elvInt;
		hpevLeft->w = (WCmpSdSd(sdLeft, sdRight) != 0) ? -1 : 0;
		break;
	case eltLt:
		hpevLeft->elv = elvInt;
		hpevLeft->w = (WCmpSdSd(sdLeft, sdRight) < 0) ? -1 : 0;
		break;
	case eltLe:
		hpevLeft->elv = elvInt;
		hpevLeft->w = (WCmpSdSd(sdLeft, sdRight) <= 0) ? -1 : 0;
		break;
	case eltGt:
		hpevLeft->elv = elvInt;
		hpevLeft->w = (WCmpSdSd(sdLeft, sdRight) > 0) ? -1 : 0;
		break;
	case eltGe:
		hpevLeft->elv = elvInt;
		hpevLeft->w = (WCmpSdSd(sdLeft, sdRight) >= 0) ? -1 : 0;
		break;
		}
	/* If we get here, the string operation succeeded.
		*/
	EviPop();	/* pop eviRight */

#ifdef DEBUG
	if (fToExp)
		{
		PrintT("EXP: applying SD op ...", 0);
		if (fFreeLeft)
			PrintT(" free left ...", 0);
		if (fFreeRight)
			PrintT(" free right ...", 0);
		PrintT("\n", 0);
		}
#endif

	if (fFreeLeft)
		FreeSd(sdLeft);
	if (fFreeRight)
		FreeSd(sdRight);
}


/* %%Function:ApplyIntElt %%Owner:bradch */
ApplyIntElt(elt)
/* Applies an operator (accepting INT args) to the top two entries on the EV
* stack.  The result is pushed on the EV stack.
*/
ELT elt;
{
	WORD wLeft, wRight;
	EVI eviLeft;
	struct EV huge *hpevLeft;

	if (elt >= eltUnaryLim)
		ResolveEvi(EviPop(), elvInt, (WORD huge *)&wRight);
	ResolveEvi(eviLeft = EviTop(), elvInt, (WORD huge *)&wLeft);
	hpevLeft = HpevOfEvi(eviLeft);

	Assert(hpevLeft->fResolved && !hpevLeft->fAggregate);
	Assert(hpevLeft->elv == elvInt);

	switch (elt)
		{
#ifdef DEBUG
	default:
		Assert(FALSE);
		RtError(rerrInternalError);
#endif

	case eltNot:
		hpevLeft->w = ~wLeft;
		break;

	case eltAnd:
		hpevLeft->w = wLeft & wRight;
		break;

	case eltOr:
		hpevLeft->w = wLeft | wRight;
		break;
		}
}


/* %%Function:FDoSubscript %%Owner:bradch */
FDoSubscript()
	/* Called when the expression-parser detects the end of a subscript expression
	* (i.e. array reference or procedure call).  The EV stack contains:
	*	top:   N entries containing the N arguments.
	*	top-N: EV containing array or procedure name
	* The ELT stack contains an eltComma for each argument, with eltSubscript
	* below that.
	*
* On exit, the result of the subscript is on top of the EV stack.  This is
	* either a constant-valued EV (result of a procedure call), or an fAggregate
	* EV with elvAggr == elvRecord*.
	*/
{
	ELT elt;
	int cevArgs;
	EVI eviSym, eviFirstArg;
	struct EV huge *hpevSym;

	cevArgs = 1;
	while ((elt = RgeltStack()[ElGlobal(ieltStackPtr)--]) == eltComma)
		cevArgs++;
	if (elt == eltLParen)
		{
		/* Found a list of expressions which is not a subscript
		* operation.  Push eltRParen (which will cause us to reject
		* any additional tokens).
		*/
		RgeltStack()[++ElGlobal(ieltStackPtr)] = eltRParen;
		return TRUE;
		}

	if (elt != eltSubscript)
		return FALSE;

	eviFirstArg = EviOfIev(ElGlobal(ievStackPtr) - cevArgs + 1);
	eviSym = eviFirstArg - EviOfIev(1);

	/* If the only argument is null, get rid of it.  This is generated
	* when "()" appears.
	*/
	if (cevArgs == 1 && (hpevSym = HpevOfEvi(eviFirstArg))->elv == elvNil)
		{
		/* Check for the other case for which elv == elvNil ...
		*/
		if (!hpevSym->fResolved && hpevSym->fElx)
			RtError(rerrSyntaxError);
		cevArgs = 0;
		eviFirstArg = eviNil;
		}

	if ((hpevSym = HpevOfEvi(eviSym))->fResolved)
		{
		/* Array within a record -- not implemented
			*/
		/* Error: illegal function call */
		RtError(rerrIllegalFunctionCall);
		}
	else  if (hpevSym->fElx)
		{
		ELV elvT;
		NUM numT;

		elvT = ElvCallElx(hpevSym->elx, cevArgs, eviFirstArg,
				(BYTE huge *)&numT, FALSE);
		(hpevSym = HpevOfEvi(eviSym))->elv = elvT;   /* re-validate */
		BLTBH(&numT, &hpevSym->num, sizeof(NUM));
		hpevSym->fResolved = TRUE;
		hpevSym->fAggregate = FALSE;
		if (hpevSym->elv == elvSd || FRecordElv(hpevSym->elv))
			hpevSym->fFree = TRUE;
		}
	else
		{
		struct VAR *pvarArray;

		Global(fEnableDefine) = FALSE;
		pvarArray = PvarFromPsy(hpevSym->psy, elvNil, TRUE);
		Global(fEnableDefine) = TRUE;

		if (pvarArray != 0)
			{
			/* Array reference.
				*/
			int iev;
			struct VAR huge *hpvarArray;

			hpvarArray = HpvarOfPvar(pvarArray);

			hpevSym->fResolved = hpevSym->fAggregate = TRUE;
			hpevSym->ibAggr = IbRefArray(hpvarArray->ai.ad,
					cevArgs, eviFirstArg);
			hpevSym->elvAggr = elvArray;
			hpevSym->pvarAdAggr = pvarArray;
			hpevSym->elv = hpvarArray->ai.elv;
			}
		else  if (HpsyOfPsy(hpevSym->psy)->syt == sytDkd)
			{
			DKD * pdkd, huge * hpdkd;
			ELV elv;
			int iev;
			BOOL fSubCall;
			long lT;
			int ielp;
			int cwArgs;
			int * pwArgs;
			struct EV huge * hpev;
			RERR rerr = rerrNil;
			int rgwArgs [celpMax];
			char ** rghszArgs [celpMax];

			pdkd = HpsyOfPsy(hpevSym->psy)->pdkd;
			Assert(pdkd != 0);
			hpdkd = HpOfSbIb(Global(sbNtab), pdkd);

			if (hpdkd->idktMacParam != cevArgs)
				RtError(rerrArgCountMismatch);

			fSubCall = Global(fExpectAsst) &&
					ElGlobal(ieltStackPtr) == Global(ieltStackBase);

			if ((hpdkd->dktRet == dktNone) != fSubCall)
				RtError(rerrIllegalFunctionCall);

			/* Intialize the type of the return value, and default
				* its value to 0.
				*/
			hpevSym->fResolved = TRUE;
			hpevSym->fAggregate = FALSE;
			if ((elv = hpevSym->elv = ElvFromDkt(hpdkd->dktRet)) == elvSd)
				hpevSym->fFree = TRUE;
			BLTBCX(0, LpOfHp(&hpevSym->num), sizeof(NUM));

			BLTBCX(0, (char far *) rghszArgs, sizeof (rghszArgs));
			
			/* Resolve arguments and store results in rgwArgs. */
			cwArgs = 0;
			pwArgs = rgwArgs;
			for (ielp = 0; ielp < cevArgs; ielp += 1)
				{
				DKT dkt;
				ELV elv;

				dkt = hpdkd->rgdkt[ielp];

				ResolveEvi(eviFirstArg + EviOfIev(ielp), 
						dkt == dktInt ? elvInt : 
						(dkt == dktString ? elvSd : elvNum), 
						hpNil);
				hpev = HpevOfEvi(eviFirstArg + EviOfIev(ielp));
				elv = hpev->elv;
				if (elv != elvInt && elv != elvNum && 
						elv != elvSd)
					{
LTmError:
					RtError(rerrTypeMismatch);
					Assert(FALSE); /* NOT REACHED */
					}

				/* make sure types match! */
				if ((elv == elvSd) != (dkt == dktString))
					goto LTmError;

				switch (dkt)
					{
#ifdef DEBUG
				default:
					Assert(FALSE);
#endif

				case dktInt:
					*pwArgs++ = hpev->w;
					break;

				case dktLong:
						{
						extern long LWholeFromNum();
						NUM numT;
						BLTBH(&hpev->num, &numT, cbNUM);
						*((long *) pwArgs)++ = 
								LWholeFromNum(&numT, TRUE);
						}
					break;

				case dktDouble:
					*((NUM *) pwArgs)++ = hpev->num;
					break;

				case dktString:
					{
					SD sd;
					int cch;
					LPSTR lpstr;
					char ** hsz;
					
					sd = hpev->sd;
					cch = CchFromSd(sd);
					
					if (cch > 255)
						RtError(rerrStringTooBig);
					
					if ((rghszArgs[ielp] = hsz = PpvAllocCbWW(sbDds, 256)) == NULL)
						{
						rerr = rerrOutOfMemory;
						goto LRerr;
						}

					BLTBH(HpchFromSd(sd), (char huge *) *hsz, cch);
					(*hsz)[cch] = '\0';
					
					lpstr = (LPSTR) *hsz;
					*pwArgs++ = HIWORD(lpstr);
					*pwArgs++ = LOWORD(lpstr);
					}
					break;
					}
				}
			cwArgs = pwArgs - rgwArgs;

			Assert(hpdkd->lppasproc != NULL);
			Assert(hpdkd->hLib != NULL);

			/* Push the args, call the func, and pop the args. */
			lT = LPushMacroArgs(hpdkd->lppasproc, rgwArgs, cwArgs);

			if (!fSubCall)
				{
				NUM numT;

				hpevSym = HpevOfEvi(eviSym);

				switch (hpdkd->dktRet)
					{
				case dktInt:
					hpevSym->w = (int) lT;
					break;

				case dktString:
						{
						long cch;
						char ** hsz;
						char far * lpch;

						for (lpch = (LPSTR) lT; *lpch != '\0'; lpch += 1)
							;
						if ((cch = ((long) lpch - lT)) > 32767)
							{
							rerr = rerrStringTooBig;
							goto LRerr;
							}

						if ((hsz = PpvAllocCb(sbDds, (int) cch + 1)) == NULL)
							{
							rerr = rerrOutOfMemory;
							goto LRerr;
							}

						BLTBX((LPSTR) lT, (LPSTR) *hsz, (int) cch + 1);
						hpevSym->sd = SdCopySz(*hsz);
						FreePpv(sbDds, hsz);
						if (hpevSym->sd == sdNil)
							goto LRerr;
						}
					break;

				case dktLong:
					NumFromL(lT, &numT);
					BLTBH(&numT, &hpevSym->num, sizeof(NUM));
					break;

				case dktDouble:
					BLTBX((LPSTR) lT, LpOfHp(&hpevSym->num), sizeof(NUM));
					break;
					}
				}
LRerr:
			/* Copy strings parameters back into their sd's and free the
				heap strings */
			for (ielp = 0; ielp < cevArgs; ielp += 1)
				{
				hpev = HpevOfEvi(eviFirstArg + EviOfIev(ielp));
				if (hpev->elv == elvSd && rghszArgs[ielp] != 0)
					{
					int cch;
					
					cch = CchSz(*rghszArgs[ielp]) - 1;
					if (FReallocSd(hpev->sd, cch))
						{
						BLTBH((char huge *) *rghszArgs[ielp], 
							HpchFromSd(hpev->sd), cch);
						}

					FreePpv(sbDds, rghszArgs[ielp]);
					}
				}

			if (rerr != rerrNil)
				RtError(rerr);
			}
		else
			{
			struct PROC *pproc, huge *hpproc;
			int iev;
			BOOL fSubCall;
			NUM numT;
			ELT eltCurSav;

			/* This is a function call.
				*/
			fSubCall = Global(fExpectAsst) &&
					ElGlobal(ieltStackPtr) == Global(ieltStackBase);

			if ((pproc = PprocFromPsy(hpevSym->psy, -1)) == 0)
				{
				int bcm;
				char stName [cchIdentMax];

				/* Try to run macro... */
				BLTBH(HpsyOfPsy(hpevSym->psy)->st, stName,
				      HpsyOfPsy(hpevSym->psy)->st[0]+1);
				stName[0] -= 1; /* Remove type char */

				if ((bcm = BcmOfSt(stName)) == -1)
					RtError(rerrSubprogNotDefined);

				if (cevArgs > 0)
					RtError(rerrArgCountMismatch);

				if (!fSubCall)
					RtError(rerrIllegalFunctionCall);

				hpevSym->fResolved = TRUE;
				hpevSym->fAggregate = FALSE;
				hpevSym->elv = elvNil;

				CmdRunBcmEl(bcm);

				goto LDone;
				}

			eltCurSav = Global(eltCur);

			hpproc = HpOfSbIb(Global(sbNtab), pproc);

			if ((hpproc->elv == elvNil) != fSubCall)
				RtError(rerrIllegalFunctionCall);

			/* Intialize the type of the return value, and default
				* its value to 0.
				*/
			hpevSym->fResolved = TRUE;
			hpevSym->fAggregate = FALSE;
			if ((hpevSym->elv = hpproc->elv) == elvSd)
				hpevSym->fFree = TRUE;
			BLTBCX(0, LpOfHp(&hpevSym->num), sizeof(NUM));

			/* Call the function.
				*/
			EldExecutePproc(pproc, cevArgs, eviFirstArg,
					fSubCall ? hpNil : (BYTE huge *)&numT);
			Global(eltCur) = eltCurSav;

			if (!fSubCall)
				{
				hpevSym = HpevOfEvi(eviSym);
				BLTBH(&numT, &hpevSym->num, sizeof(NUM));
				}
			}
		}
LDone:
	PopToIev(IevOfEvi(eviSym));

	/* The EV containing the return value (or elvNil if a statement call)
		* is now on top of the stack.
		*/
	return TRUE;
}


/* %%Function:IbRefArray %%Owner:bradch */
unsigned IbRefArray(ad, cevArgs, eviFirstArg)
/* Returns the offset (from the beginning of the array record) to the array
* element which is being accessed.
*/
AD ad;
int cevArgs;
EVI eviFirstArg;
{
	struct AHR huge * hpahr;
	unsigned ib;
	int cbElement;
	int iev;

	hpahr = HpOfSbIb(sbArrays, PahrOfAd(ad));

	if (cevArgs != hpahr->cas)
		/* Error: wrong number of array subscripts */
		RtError(rerrWrongNumOfDimensions);

	ib = 0;
	cbElement = hpahr->cbElement;

	for (iev = cevArgs - 1; iev >= 0; --iev)
		{
		int as;

		ResolveEvi(eviFirstArg + EviOfIev(iev), 
				elvInt, (int huge *) &as);

		if (iev != cevArgs - 1)
			ib *= hpahr->rgas[iev];

		if (as < 0 || as >= hpahr->rgas[iev])
			RtError(rerrSubscriptOutOfRange);

		ib += as;
		}

	ib *= cbElement;

	return ib;
}


/* %%Function:EviPush %%Owner:bradch */
EVI EviPush()
{{ /* NATIVE - EviPush */
	if (ElGlobal(ievStackPtr) >= ElGlobal(ievMac))
		/* Error: expression too complicated (stack) */
		RtError(rerrExpressionTooComplex);

	return EviOfIev(++ElGlobal(ievStackPtr));
}}


/* %%Function:ResolveEvi %%Owner:bradch */
ResolveEvi(evi, elv, hpvalRet)
/* Casts the value in the EV to type "elv".  If the EV represents a variable,
* the value is fetched.
*
* If IbOfHp(hpvalRet) != 0, the value is stored in *hpvalRet.  In any case,
* the EV is updated to reflect the type and value.
*
* This routine may affect the string and array local-heaps, but is guaranteed
* not to affect the local heap in sbTds.
*/
EVI evi;
ELV elv;
char huge *hpvalRet;
	/* CSNative as used repeatedly while running */
{{ /* NATIVE - ResolveEvi */

	struct EV huge *hpev = HpevOfEvi(evi);

	if (hpev->elk != elkNil)
		/* Error: illegal use of argument-name keyword */
		RtError(rerrSyntaxError);

	if (!hpev->fResolved)
		{	/* -- get the value from a scalar variable -- */
		struct VAR *pvar, huge *hpvar;

		if (hpev->fElx)
			{
			/* If this was a valid ELX call, DoSubscript()
				* would have made the call when the end of
				* the arg list was seen.
				*/
			/* Error: missing args to built-in function */
			RtError(hpev->elv == elvNil ? 
					rerrSyntaxError : rerrArgCountMismatch);
			}

		pvar = PvarFromPsy(hpev->psy, FRecordElv(elv) ? elvNilRecord
				: elvNil,
				FALSE);
		hpvar = HpvarOfPvar(pvar);

		if (hpvar->elv == elvFormal)
			{	/* -- dereference formal parameter -- */
			hpvar = HpvarOfPvar(pvar = hpvar->pvar);
			}

		hpev->elv = hpvar->elv;
		switch (hpev->elv)
			{
		default:
			if (!FRecordElv(hpev->elv))
				/* Error: operations on elv hpev->elv NYI */
				RtError(rerrInternalError);

			hpev->rl.pvar = pvar;	/* indirect reference to rek */
			hpev->rl.ib = ibNil;	/* pvar is not an array */
			break;
		case elvNum:
			hpev->num = hpvar->num;
			break;
		case elvInt:
			hpev->w = hpvar->w;
			break;
		case elvSd:
			hpev->sd = hpvar->sd;
			hpev->fFree = FALSE;
			break;
		case elvNil:
			RtError(rerrSyntaxError);
			Assert(FALSE);	/* Not Reached */
			}
		hpev->fResolved = TRUE;
		hpev->fAggregate = FALSE;
		}
	else  if (hpev->fAggregate)
		{	/* -- get a value from an array or record -- */
		hpev->fAggregate = FALSE;
		switch (hpev->elvAggr)
			{
		default:
			Assert(FRecordElv(hpev->elvAggr));

			/* Get a value out of a record into hpev.  hpev->elv
				* is the type of the value in the record (determined
				* when fAggregate was set).
				*/
			hpev->elv = ElvReadItemFromRl(hpev->rlAggr, 
					hpev->rpsAggr, hpev->elv, &hpev->num, 
					HeldiFromIeldi(IeldiFromElv(hpev->elvAggr)));

			switch (hpev->elv)
				{
			default:
				break;
			case elvSd:
				hpev->fFree = TRUE;	/* just created SD */
				break;
#ifdef DEBUG
			case elvNil:
				Assert(FALSE);
#endif
				}
			break;

		case elvArray:
				{	/* -- array reference -- */
				struct AHR *pahr, huge *hpahr;
				VAL *pval;

				pahr = PahrOfAd(HpvarOfPvar(hpev->pvarAdAggr)->ai.ad);
				hpahr = HpahrOfPahr(pahr);
				pval = (char *)PvalOfPahr(pahr, hpahr->cas) +
						hpev->ibAggr;

				if (FRecordElv(elv) &&
						((REK huge *)HpOfSbIb(sbArrays, pval))->hcab == 0)
					{
					InitHprek(HpOfSbIb(sbArrays, pval), elv);
					hpev = HpevOfEvi(evi);	/* re-validate */
					}

				if (FRecordElv(elv))
					{
				/* Getting a record out of an array; the result
					* is a reference to the array location.
					*/
					hpev->rl.ib = hpev->ibAggr;
					hpev->rl.pvar = hpev->pvarAdAggr;
					Assert(hpev->ibAggr != ibNil);
					}
				else
					{
				/* Getting a normal value out of an array; just
					* copy it.
					*/
					BLTBH(HpOfSbIb(sbArrays, pval), &hpev->num,
							CbValFromElv(hpev->elv));
					hpev->fFree = FALSE;	/* in case it's elvSd */
					}
				break;
				}
		/* end switch (hpev->elvAggr) */
			}
		}

	/* Convert types here if necessary... */
	if (hpev->elv != elv && elv != elvNil)
		{
		if (FRecordElv(elv) || FRecordElv(hpev->elv))
			{
			if (!FRecordElv(hpev->elv) ||
					elv != ElvFromIeldi(ieldiNil))
				{
#ifdef DEBUG
				PrintT("casting elv %d", hpev->elv);
				PrintT(" to elv %d\n", elv);
#endif
				ElSyntaxError(0);
				}

			/* Cast to null record type; i.e. we want a
					* record but we don't care what ELX it is.
					*/
			}
		else
			{
			if (hpev->elv == elvNil)
				{
				/* Null arg.  Must create 0 or "" */
				hpev->w = 0;
				switch (elv)
					{
#ifdef DEBUG
				default:
					Assert(FALSE);
#endif
				case elvInt:
					break;

				case elvNum:
					BLTBCX(0, LpOfHp(&hpev->num), cbNUM);
					break;

				case elvSd:
					if ((hpev->sd = SdFromCch(0)) == sdNil)
						RtError(rerrOutOfMemory);
					hpev->fFree = TRUE;
					break;
					}

				if (elv == elvIntOrSd)
					goto LIntOrSd;
				}
			else
				{
				if (elv == elvIntOrSd)
					{
LIntOrSd:
					if (hpev->elv == elvNum)
						{
						elv = elvInt;
						goto LNumToInt;
						}
					return;
					}

				switch (ElvcOfElvElv(hpev->elv, elv))
					{
				case elvcNumToInt:
						{
						NUM numT;
LNumToInt:
						BLTBX(LpOfHp(&hpev->num), 
								(char far *)&numT, cbNUM);

						LdiNum(&numT);

						hpev->w = CIntNum();
						}
					break;

				case elvcIntToNum:
						{
						NUM numT;

						CNumInt(hpev->w);
						StiNum(&numT);
						BLTBX((char far *)&numT, 
								LpOfHp(&hpev->num), cbNUM);
#ifdef DEBUG
						if (fToExp)
							PrintT("EXP: cast IntToNum(%d)\n", hpev->w);
#endif
						}
					break;

				default:
					/* Error: type mismatch (elv) */
					RtError(rerrTypeMismatch);
					}
				}

			hpev->elv = elv;
			}
		}

	if (IbOfHp(hpvalRet) != 0)
		{
		if (elv == elvNil)
			elv = hpev->elv;

		BLTBH(&hpev->num, hpvalRet, CbValFromElv(elv));
		}
}}


/* %%Function:ParseValue %%Owner:bradch */
ParseValue(elv, hpval, eltStopParse)
ELV elv;
VAL huge *hpval;
ELT eltStopParse;
	/* CSNative as used frequently while parsing */
{{ /* NATIVE - ParseValue */
	ELT elt;
	EVI evi;
	int iNest;
	
	iNest = 0;
	while ((elt = Global(eltCur)) != eltStopParse)
		{
		if (elt == eltLParen)
			iNest += 1;
		else if (elt == eltRParen)
			{
			if (iNest == 0)
				break;
			iNest -= 1;
			}
		
		if (!FExpIn(elt))
			break;
		
		if (elt == Global(eltCur))
			EltGetCur();
		}

	/* Make sure what we have is a valid expression.
		*/
	if (!FExpIn(eltEndExp))
		Assert(FALSE);

	ResolveEvi(evi = EviPop(), elv, hpval);

	if (elv == elvSd || FRecordElv(elv))
		{
		struct EV huge *hpev = HpevOfEvi(evi);

		/* Produce an SD which can be freed.  It is the caller's
			* responsibility to assure that the SD ends up in a place
			* where it gets freed.
			*/
		if (!hpev->fFree)
			{
			if ((*(SD huge *)hpval = SdDupSd(hpev->sd)) == sdNil)
				RtError(rerrOutOfMemory);
			}
		}

	/* If there's still anything on the stack, it's because there were
		* commas in the expression.
		*/
	if (ElGlobal(ievStackPtr) != Global(ievStackBase))
		/* Error: illegal use of comma */
		RtError(rerrSyntaxError);
}}


/* %%Function:EviParse %%Owner:bradch */
EviParse(fLhs, fStopAtComma, fCall)
/* Parses a single expression and returns an EVI to the result.  If fLhs, the
* expression is the first thing in a line (i.e. left-hand side of an
* assignment statement) -- in this case, there need not be parentheses
* around a statement call.
*/
BOOL fLhs;
BOOL fStopAtComma;
BOOL fCall;
{
	BOOL fCallSav;
	EVI evi;
	ELT elt;


	if (fLhs)
		Global(fImplicitParens) = FALSE;

#ifdef DEBUG
	if (fToExp)
		PrintT("EXP: entered EviParse()\n", 0);
#endif

	Assert(ElGlobal(ievStackPtr) == Global(ievStackBase));

	if (fCall)
		{
		fCallSav = Global(fCall);
		Global(fCall) = TRUE;
		}

	if (fLhs)
		{
		if (Global(eltCur) == eltIdent)
			{
			Global(fEnableCall) = FALSE;
			if (!FExpIn(eltIdent))
				Assert(FALSE);
			switch (EltGetCur())
				{
			default:
				Global(fImplicitParens) = TRUE;
				if (!FExpIn(eltLParen))
					Assert(FALSE);
			case eltEq:
			case eltLParen:
			case eltElk:
			case eltUserElk:
				break;
				}
			Global(fEnableCall) = TRUE;
			}
		Global(fExpectAsst) = TRUE;
		}

	elt = Global(eltCur);
	for (;;)
		{
		if ((elt == eltComma && fStopAtComma) || !FExpIn(elt))
			break;
		if (elt == Global(eltCur))
			elt = EltGetCur();
		else
			elt = Global(eltCur);
		}

#ifdef OLD
	while ((!fStopAtComma || Global(eltCur) != eltComma) && 
			FExpIn(Global(eltCur)))
		EltGetCur();
#endif

	if (fCall)
		Global(fCall) = fCallSav;

	if (fLhs && Global(fImplicitParens))
		{
		if (!FExpIn(eltRParen))
			/* Error: invalid statement */
			RtError(rerrSyntaxError);
		}

#ifdef DEBUG
	Global(fImplicitParens) = fInvalid;
#endif

	if (!FExpIn(eltEndExp))
		Assert(FALSE);
	if (fLhs)
		Global(fExpectAsst) = FALSE;

	evi = EviPop();

	/* If there's still anything on the stack, it's because there were
		* commas in the expression.
		*/
	if (ElGlobal(ievStackPtr) != Global(ievStackBase))
		/* Error: illegal use of comma! */
		RtError(rerrSyntaxError);

	return evi;
}


/* %%Function:MakeRecordPvar %%Owner:bradch */
MakeRecordPvar(pvar, heldi, pele, cevArgs, eviFirstArg)
/* Accepts an EVI for a dialog record; processes a list of arguments
* (positional or keyword) to a dialog statement, and adds the specified
* items to the record.
*/
struct VAR *pvar;
ELDI **heldi;
ELE *pele;
int cevArgs;
EVI eviFirstArg;
{
	int iev;
	BOOL fPositionalOk;
	ELDI huge *hpeldi;
	RL rl;

	rl.ib = ibNil;
	rl.pvar = pvar;

	hpeldi = HpeldiFromHeldi(heldi);
	fPositionalOk = TRUE;
	for (iev = 0; iev < cevArgs; iev++)
		{
		struct EV huge *hpevArg;
		ELK elkArg;
		unsigned ib, ielfd;
		ELV elv;
		RPS rps;
		REK huge * hprek;

		hpevArg = HpevOfEvi(eviFirstArg + EviOfIev(iev));
		if ((elkArg = hpevArg->elk) == elkNil)
			{
			/* Positional argument -- no ELK.
				*/
			if (!fPositionalOk)
				/* Error: invalid argument list */
				RtError(rerrSyntaxError);
			ielfd = iev;
			elv = hpeldi->rgelfd[ielfd].elv;
			}
		else
			{
			/* Keyword argument -- find the field with the
				* appropriate ELK value.
				*/
			fPositionalOk = FALSE;
			for (ielfd = 0; ielfd < hpeldi->celfd; ielfd++)
				{
				if (hpeldi->rgelfd[ielfd].elk == elkArg)
					break;
				}
			if (ielfd == hpeldi->celfd)
				{
				/* Error: undefined record field */
				RtError(rerrUndefDlgRecField);
				}

			elv = hpeldi->rgelfd[ielfd].elv;

			hprek = HprekOfRl(rl);
			if (hprek->hcab == 0)
				InitHprek(hprek, ElvOfRl(rl));
			hpevArg->elk = elkNil;

			if (elv == elvSd && hpeldi->rgelfd[ielfd].iag >= 
					((hpeldi->cabi >> 8) & 0xff))
				elv = elvIntOrSd;
			}

		/* ielfd and elv now describe the argument to be added.  We now
			* obtain a value of type elv, and store it into the record.
			*/
		ResolveEvi(eviFirstArg + EviOfIev(iev), 
				/*elv == elvNil ? elvInt : elv*/elvNil, hpNil);
		rps = RpsFromIelfd(ielfd, heldi);
		
		/* Catch buttons specified positionaly */
		if (rps.ielfd != ielfdNil && elkArg == elkNil)
			RtError(rerrIllegalFunctionCall);
			
		hpevArg = HpevOfEvi(eviFirstArg + EviOfIev(iev));
		StoreItemToRl(rl, rps, hpevArg->elv, &hpevArg->num, heldi);
		}
}



/* ---------------------------------------------------------------------- */
/* Routines for handling CAB's as REK's
/* ---------------------------------------------------------------------- */

/* %%Function:InitHprek %%Owner:bradch */
VOID InitHprek(hprek, elv)
/* Allocates a CAB for a record variable.
* SHAKES: sbDds, sbTds (via HeldiFromIeldi()).
*/
REK huge *hprek;
ELV elv;
{
	ELDI huge *hpeldi;

	hpeldi = HpeldiFromHeldi(HeldiFromIeldi(IeldiFromElv(elv)));

	if ((hprek->hcab = HcabAlloc(hpeldi->cabi)) == 0)
		RtError(rerrOutOfMemory);
	hprek->ielfdButton = ielfdNil;

#ifdef DEBUG
	if (fToExp)
		{
		PrintT("EXP: REK ... init, cabi = %u", hpeldi->cabi);
		PrintT(" (hcab = %u)\n", hprek->hcab);
		}
#endif
}


/* %%Function:FreeRek %%Owner:bradch */
VOID FreeRek(rek)
/* Frees data structures associated with a REK.
*/
REK rek;
{
#ifdef DEBUG
	if (fToExp)
		PrintT("EXP: REK ... free (hcab = %u)\n", rek.hcab);
#endif
	if (rek.hcab != 0)
		{
		FreeCab(rek.hcab); /* BAC */
		rek.hcab = 0;
		}
}


/* %%Function:ElvReadItemFromRl %%Owner:bradch */
ELV ElvReadItemFromRl(rl, rps, elv, hpval, heldi)
RL rl;
RPS rps;
ELV elv;
NUM huge *hpval;
{
	BOOL fParse;
	PFN pfnWParse;
	int tmc, opt;
	BOOL fPt;
	void * pv;
	WORD w;
	LONG l;
	ELDI huge *hpeldi;
	REK rek, huge *hprek;
	struct VAR huge *hpvar;

	fParse = fPt = FALSE;
	hpeldi = HpeldiFromHeldi(heldi);

	if (elv == elvInt && rps.iag < ((hpeldi->cabi >> 8) & 0xff))
		{
		w = *(int huge *)hpval;
		hpval = (int huge *) &w;

		if ((pfnWParse = PfnTmcOptFromHidIag(hpeldi->hid, 
				rps.iag, &tmc, &opt, &fPt)) == NULL)
			RtError(rerrTypeMismatch);

		if (fPt)
			w = w * 20; /* convert points to twips */
		}
	else  if (elv == elvSd && rps.iag >= ((hpeldi->cabi >> 8) & 0xff))
		{
		fParse = TRUE;
		elv = elvInt;
		}

	hprek = HprekOfRl(rl);
	if (hprek->hcab == 0)
		{
		InitHprek(hprek, ElvOfRl(rl));
		}
	rek = *hprek;

#ifdef DEBUG
	if (fToExp)
		{
		PrintT("EXP: REK ... read: ", 0);
		PrintElv(elv);
		PrintT(" (hcab = %u", rek.hcab);
		PrintT(", iag = %u)", rps.iag);
		if (rps.ielfd != ielfdNil)
			PrintT(" <button>\n", 0);
		else
			PrintT(" <CAB item>\n", 0);
		}
#endif

	if (rps.ielfd != ielfdNil)
		{
		Assert(elv == elvInt);
		if (rek.ielfdButton == rps.ielfd)
			*(int huge *)hpval = -1;	/* button pushed */
		else
			*(int huge *)hpval = 0;		/* button not pushed */
		}
	else
		{
		switch (elv)
			{
#ifdef DEBUG
		default:
			Assert(FALSE);
#endif

		case elvInt:
			if (fParse)
				{
				if ((pfnWParse = PfnTmcOptFromHidIag(
						hpeldi->hid, rps.iag, &tmc, &opt, &fPt))
						== NULL)
					RtError(rerrTypeMismatch);
				}

			if (pfnWParse == WParseDttm)
				{
				l = *(long *)PvFromCab(rek.hcab, rps.iag);
				pv = &l;
				}
			else
				{
				w = *(int *)PvFromCab(rek.hcab, rps.iag);
				pv = &w;
				}

			if (fParse)
				{
				int cch;
				SD sd;
				char szBuf [256];

				(*pfnWParse)(tmmFormat, szBuf, &pv, 
						0, tmc, opt);
				if ((*(SD huge *)hpval = SdCopySz(szBuf)) == sdNil)
					RtError(rerrOutOfMemory);
				elv = elvSd;
				break;
				}

			if (fPt)
				w /= 20;

			*(int huge *)hpval = w;

			break;

		case elvNum:
			/* Error: NUM's in CAB's NYI */
			RtError(rerrInternalError);

		case elvSd:
				{
				HS16 hs16;
				SD sd;
				char stT[cchDlgStringMax + 1];

			/* If the handle in the CAB is NULL, produce a null
				* string; otherwise read the string from the CAB.
				*/
				if (PpvFromCab(rek.hcab, rps.iag) == 0)
					*(SD huge *)hpval = (SD)0;
				else
					{
					GetCabSt(rek.hcab, stT, cchDlgStringMax,
							rps.iag);
					if ((*(SD huge *)hpval = SdCopySt(stT)) == sdNil)
						RtError(rerrOutOfMemory);
					}
				break;
				}
		/* end switch */
			}
		}

	return elv;
}


/* %%Function:StoreItemToRl %%Owner:bradch */
VOID StoreItemToRl(rl, rps, elv, hpval, heldi)
RL rl;
RPS rps;
ELV elv;
NUM huge *hpval;
ELDI ** heldi;
{
	ELDI huge * hpeldi;
	REK huge * hprek;

	hpeldi = HpeldiFromHeldi(heldi);
	hprek = HprekOfRl(rl);

	if (hprek->hcab == 0)
		InitHprek(hprek, ElvOfRl(rl));
	Assert(hprek->hcab != 0);

	if (rps.ielfd != ielfdNil)
		{
		int w;

		/* Deal with a push button */

		switch (elv)
			{
			NUM numT;

		case elvNum:
			BLTBH(hpval, (NUM huge *) &numT, sizeof (NUM));
			LdiNum(&numT);
			w = CIntNum();
			break;

		case elvInt:
			w = *((int huge *)hpval);
			break;

		case elvNil:
			w = 1;
			break;

		default:
			RtError(rerrTypeMismatch);
			Assert(FALSE); /* NOT REACHED */
			}

		if (w != 0)
			{
			/* Press the button */
			hprek->ielfdButton = rps.ielfd;
			}
		else  if (hprek->ielfdButton == rps.ielfd)
			{
			/* Unpress the button */
			hprek->ielfdButton = ielfdNil;
			}
		}
	else
		{
		SetCabVal(hpeldi->hid, hprek->hcab, rps.iag, hpval, elv);
		}
}


/* %%Function:RpsFromIelfd %%Owner:bradch */
RPS RpsFromIelfd(ielfd, heldi)
/* Given a dialog and field, finds the RPS (Record PoSition) for the field.
*/
WORD ielfd;
ELDI **heldi;
{
	ELFD huge *hpelfd;
	RPS rpsReturn;

	hpelfd = &HpeldiFromHeldi(heldi)->rgelfd[ielfd];
	if (hpelfd->elv == elvNil)
		{	/* -- push button -- */
		rpsReturn.ielfd = ielfd;
		rpsReturn.iag = -1;
		}
	else		/* -- non-push-button -- */		
		{
		rpsReturn.ielfd = ielfdNil;
		rpsReturn.iag = hpelfd->iag;
		}

	return rpsReturn;
}


/* %%Function:IelfdFromIag %%Owner:bradch */
IelfdFromIag(hpeldi, iag)
ELDI huge * hpeldi;
int iag;
{
	int ielfd, celfd;

	celfd = hpeldi->celfd;
	for (ielfd = 0; ielfd < celfd; ielfd += 1)
		if (hpeldi->rgelfd[ielfd].iag == iag && hpeldi->rgelfd[ielfd].elv != elvNil)
			return ielfd;
	return ielfdNil;
}


/* %%Function:IelfdFrTmc %%Owner:bradch */
WORD IelfdFrTmc(tmc, heldi)
TMC tmc;
ELDI **heldi;
{
	ELDI *peldi;
	ELFD *pelfd;
	WORD ielfd;

	if (tmc == tmcNil)
		return ieldiNil;

	SetSbCur(sbTds);					/* /SB\ */

	peldi = *heldi;
	for (ielfd = 0; ielfd < peldi->celfd; ielfd++)
		{
		pelfd = &peldi->rgelfd[ielfd];

		if (pelfd->elv == elvNil && pelfd->tmc == tmc)
			{
			ResetSbCur();	    			/* \SB/ ... */
			return ielfd;
			}
		}
	ResetSbCur();						/* \SB/ */
	
	return ieldiNil;
}


/* %%Function:HprekOfRl %%Owner:bradch */
REK huge *HprekOfRl(rl)
RL rl;
{
	if (rl.ib == ibNil)
		{
		return &HpvarOfPvar(rl.pvar)->rek;
		}
	else
		{
		struct AHR *pahr, huge *hpahr;

		hpahr = HpahrOfPahr(pahr = PahrOfAd(HpvarOfPvar(rl.pvar)->ai.ad));
		return HpOfSbIb(sbArrays,
				(char *)PvalOfPahr(pahr, hpahr->cas) + rl.ib);
		}
}


/* %%Function:ElvOfRl %%Owner:bradch */
ELV ElvOfRl(rl)
RL rl;
{
	if (rl.ib == ibNil)
		return HpvarOfPvar(rl.pvar)->elv;
	else
		return HpvarOfPvar(rl.pvar)->ai.elv;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Exp_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Exp_Last() */
