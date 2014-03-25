/* elcore.c -- resident portion of OpEL */

#include <qwindows.h>
#include <qsetjmp.h>
#include <el.h>
#include "eltools.h"
#include <uops.h>
#include <rerr.h>

#define EXTERN	extern
#include "priv.h"
#include "sym.h"
#include "exp.h"
#include "_exec.h"
#ifdef DEBUG
#define WCompChCh(ch1, ch2) \
		S_WCompChCh(ch1, ch2)
#else /* !DEBUG */
#define WCompChCh(ch1, ch2) \
		N_WCompChCh(ch1, ch2)
#endif /* !DEBUG */
#include "toolbox.h"

#define kcNil	0xffff /* stolen from keys.h */
#define bcmNil  0xffff
#define pvNil	((void *) 0)

extern RERR vrerr;
extern unsigned vibLineStart;
extern int vcElParams;
extern BOOL vfElFunc;

extern WORD mpsbps[];


AD AdCreate(int, int, AS *);
ELT EltGetCur();
BOOL FSaveElHcab(HCAB, HID, TMC);
BOOL FExecOnError(struct LAB **);
VOID ExpectHeldi(), InitHprek();

#define LibCur()	(Global(libBufStart) + Global(ibBufCur))

#define FLineElt(elt)	(eltLineNolabel <= (elt) && (elt) < eltLineInt)

struct PROC *PprocFindPsy();
char *PchRefillBuf();
ELV ElvCallElx();
REK huge *HprekOfRl();

#ifdef DEBUG
extern int vfInCmd;
#define Debug(e) (e)
#else
#define Debug(e)
#endif /* DEBUG */


csconst BYTE mpelcccb[] = {
	CbElcr(rgchEndLineNolabel),
			CbElcr(rgchEndLineIdent), CbElcr(rgchEndLineInt), CbElcr(rgchEndElt),
			CbElcr(rgchEndElx), CbElcr(rgchEndElk), CbElcr(rgchEndNum),
			CbElcr(rgchEndIdent), CbElcr(rgchEndString), CbElcr(rgchEndRem),
			CbElcr(rgchEndInt), CbElcr(rgchEndOther), CbElcr(rgchEndOther),
			CbElcr(rgchEndBadSyntax), CbElcr(rgchEndSpace), CbElcr(rgchEndRem),
			CbElcr(rgchEndOther), CbElcr(rgchEndUserElk), CbElcr(rgchEndOther),
};


csconst BYTE mpelrelv[] =
{
	elvNil, elvNum, elvInt, elvSd, elvArray, elvNil};


csconst NUM numOne = {
	0, 0, 0, 0, 0, 0, 240, 63};	/* 1 */



/* %%Function:EldExecutePproc %%Owner:bradch */
ELD EldExecutePproc(pproc, cevArgs, eviFirstArg, hpval)
/* Executes a call to an EL procedure, or module-level code (psyCall == 0).
* Returns an ELD indicating what happened (eldNormalStep == procedure
* returned normally).
*
* If pproc == 0, module-level code is run, starting at lib = 0L.
* If pproc != 0, a call is made to the SUB or FUNCTION described by
*	pproc, with the specified arguments.
*
* If the call is to a FUNCTION, the result (of whatever type) will
* be stored in *hpval (if the function assigns any values to the function-
* name).
*/
struct PROC *pproc;
int cevArgs;
EVI eviFirstArg;
VAL huge *hpval;
{
	struct SY *psyCurProc;
	int celtSingleIf = 0;	/* how many nested single-line IF's we are
	* currently inside */
	struct LAB *plabOnError;
	long libOnError;
	struct ESB esb;
	int icsrSavStackPtr;
	BOOL fCallOut, fCallOutNext;

	if (CbStackAvailable(&pproc) < cbProcStackNeeded)
		{
		/* Error: Insufficient stack space to start EL procedure */
		RtError(rerrOutOfMemory);
		}

	Assert(pproc != 0);

	psyCurProc = ((struct PROC huge *)
			HpOfSbIb(Global(sbNtab), pproc))->psy;

#ifdef DEBUG
	if (fToExec)
		PrintT("EXEC: EldExecutePproc ...\n", 0);
#endif

	SetupCall(psyCurProc, cevArgs, eviFirstArg, libNil);

	/* Save the current CSR stack pointer and state of the EXP subsystem.
		*/
	SaveExp(&esb);
	icsrSavStackPtr = ElGlobal(icsrStackPtr);	/* for ON ERROR */

	/* If this is a function, initialize it's return value */
	if (hpval != hpNil)
		{
		struct PROC huge * hpproc;

		hpproc = HpprocOfPproc(pproc);

		if (hpproc->elv == elvNil)
			ElSyntaxError(0);

		if (hpproc->elv == elvSd)
			{
			*((SD huge *) hpval) = 0;
			}
		else
			{
			Assert(hpproc->elv == elvNum);
			BLTBCX(0, LpOfHp(hpval), sizeof (NUM));
			}
		}

	HpeliFirst()->lib = LibCur();

	Assert(HpeliFirst()->pfnDebug != NULL);
	(*HpeliFirst()->pfnDebug)(eldSubIn, ElGlobal(heliFirst));

	fCallOutNext = FALSE;
	fCallOut = FALSE;

	*HpNtab(pprocScan) = pproc;
	EltGetCur(); /* pre-fetch */

		/* CSNative as main statement reading loop */
		{{ /* NATIVE - EldExecutePproc */

		for (;;)
			{
			LIB libEnd;
			ELT eltStmt;

		/* This is the main statement-reading loop; each statement is
			* a case of the following switch.  From statement code ...
			*
			* - To accept a statement terminator, pre-fetch a token and
			*   "break".
			* - To accept another statement without reading a terminator
			*   (perhaps because SetCurLib() has been done), pre-fetch
			*   a token and "continue".
			*/

			if (fCallOut)
				(*HpeliFirst()->pfnDebug)
						(eldNormalStep, ElGlobal(heliFirst));

			fCallOut = fCallOutNext;
			fCallOutNext = FALSE;

			Assert(Global(eltCur) != eltSpace);
			Assert(Global(eltCur) != eltComment);
			Assert(Global(eltCur) != eltRem);
			Assert(Global(eltCur) != eltBadSyntax);

		/* (BAC) skip multiple colon's */
			while (Global(eltCur) == eltColon)
				EltGetCur();

			switch (eltStmt = Global(eltCur))
				{

			default:
				ElSyntaxError(0);

			case eltLineIdent:
			case eltLineInt:
			case eltLineNolabel:
			/* Abandon all single-line IF statements we are
				* currently in (i.e. between THEN and ELSE).  If any
				* other control structures are in the way, i.e.
				*	IF x = 0 THEN WHILE y < 5
				* an error will occur.  NOTE: currently, we will
				* still accept constructions like:
				*	IF x = 0 THEN x = 1 ELSE WHILE y < 7
				* because there is no cstSingleEndif.
				*/
				while (celtSingleIf > 0)
					{
					IcsrPop(cstSingleElse);
					celtSingleIf--;
					}

			/* Call the debugging function if it is defined.
				*/
				Assert(HpeliFirst()->pfnDebug != NULL);

			/* -1 so that next line is not selected for blank 
				lines when tracing (lib is approximate anyway)... */
				HpeliFirst()->lib = LibCur() - 1;

				EltGetCur();
				fCallOut = !FLineElt(Global(eltCur));
				continue;

		/* Control structure */
			case eltIf:
LElseIf:
					{
					CST cst;
					EVI evi;
					int wTest;
					LIB libAfterThen;

					HpeliFirst()->lib = LibCur();
					
					EltGetCur();	/* go past "IF" */
					ParseValue(elvInt, (int huge *)&wTest, eltNil);

					if (Global(eltCur) != eltThen)
						ElSyntaxError(0);
					libAfterThen = LibCur();
					EltGetCur();

					/* ElseIf's Then must end the line! */
					if (eltStmt == eltElseif && !FLineElt(Global(eltCur)))
						ElSyntaxError(1);

					if (FLineElt(Global(eltCur)))
							{
							/* Multi-line IF block.	*/
							LIB libIf = LibCur();
							
							SetCurLib(libAfterThen);
							
							if (wTest)
								{
								IcsrPushCst(cstBlockElse, libIf);
								continue;	/* next stmt */
								}

							/* Test false -- skip to the ELSE, if any. */
							if (FSkipToLine(eltElse, 0))
								{
								if (Global(eltCur) == eltElseif)
									{
									eltStmt = eltElseif;
									goto LElseIf;
									}
								
								IcsrPushCst(cstBlockEndif, libIf);
								}
							
							EltGetCur();	/* pre-fetch */
							break;		/* get terminator */
							}

					/* Single-line IF. */
					if (wTest)
						{
						IcsrPushCst(cstSingleElse, LibCur());
						celtSingleIf++;
						}
					else
						{
						SkipSingleIf(TRUE);
						if (Global(eltCur)==eltElse)
							{
							HpeliFirst()->lib = LibCur(); 
							EltGetCur(); /* pre-fetch */
							}
						}
					continue;
					}
				Assert(FALSE); /* NOT REACHED */

			case eltElse:
				EltGetCur();
				/* FALL THROUGH */
			
			case eltElseif:
				/* Block Else or ElseIf */
				IcsrPop(cstBlockElse);
				FSkipToLine(eltIf, 0);
				EltGetCur(); /* pre-fetch */
				break;


			case eltWend:
					{
					struct CSR huge * hpcsr;

			/* Check for BREAK key */
					fCallOutNext = TRUE;

					hpcsr = HpcsrOfIcsr(IcsrPop(cstWend));
					libEnd = LibCur();
					SetCurLib(hpcsr->libStart);
					HpeliFirst()->lib = LibCur();
					}
			/* FALL THROUGH */

			case eltWhile:
					{
					LIB libExpr;
					int wTest;

					libExpr = LibCur();
					EltGetCur();	/* go past "WHILE" */

					ParseValue(elvInt, (int huge *) &wTest, eltNil);

					if (wTest)
						{
						int icsr;

				/* Push a CSR and go read the terminator. */
						icsr = IcsrPushCst(cstWend, libExpr);
						break;
						}
					else
						{
				/* Skip to right after the Wend. */
						if (eltStmt == eltWend)
							{
					/* We already know where the
						end is. */
							SetCurLib(libEnd);
							}
						else
							{
					/* First time through... */
							SkipToElt(eltWend);
							}

				/* Pre-fetch a token at the new location */
						EltGetCur();
						continue;
						}

					Assert(FALSE); /* NOT REACHED */
					}

			case eltFor:
					{
					struct VAR *pvar;
					int icsr;
					struct CSR huge *hpcsr;
					struct EV *evi;
					NUM numTo, numStep;
					int sgnStep;

					if (EltGetCur() != eltIdent)
						ElSyntaxError(1);

					pvar = PvarFromPsy(Global(psyCur), elvNil, FALSE);
					if (HpvarOfPvar(pvar)->elv == elvFormal)
						pvar = HpvarOfPvar(pvar)->pvar;

					if (EltGetCur() != eltEq)
						ElSyntaxError(2);
					EltGetCur();	/* pre-fetch */
					
					ExecGets(pvar, EviParse(FALSE, FALSE, FALSE));

					if (Global(eltCur) != eltTo)
						ElSyntaxError(3);

					EltGetCur();
					ParseValue(elvNum, (NUM huge *)&numTo, eltNil);

					if (Global(eltCur) == eltStep)
						{
						MTC mtc;

						EltGetCur();
						ParseValue(elvNum, (NUM huge *)&numStep, eltNil);

						LdiNum(&numStep);
						CNumInt(0);
						mtc = CmpNum();

						sgnStep = (mtc.w2 < 0) ? -1 : 1;
						}
					else
						{
						BLTBX(&numOne, (NUM far *)&numStep, cbNUM);
						sgnStep = 1;
						}
					icsr = IcsrPushCst(cstNext, LibCur());
					hpcsr = HpcsrOfIcsr(icsr);
					hpcsr->pvarNext = pvar;
					BLTBH(&numTo, &hpcsr->numNextTo, cbNUM);
					BLTBH(&numStep, &hpcsr->numNextStep, cbNUM);
					hpcsr->sgnNextStep = sgnStep;

						{
						NUM numVar, numT;
						struct CSR huge * hpcsr = HpcsrOfIcsr(icsr);
						struct VAR huge * hpvar = HpOfSbIb(sbFrame, 
								hpcsr->pvarNext);
						MTC mtc;
						int sgn;

						numVar = hpvar->num;
						numT = hpcsr->numNextTo;
						mtc = Cmp2Num(&numVar, &numT);
						sgn = mtc.w2;
						if (!(hpcsr->sgnNextStep == 1 ? sgn <= 0 : sgn >= 0))
							{
							IcsrPop(cstNext);	/* pop our CSR */
							SkipToElt(eltNext);	/* skip loop */
							VerifyNext(pvar);	/* check the NEXT */
							}
						}

					break;
					}

			case eltNext:
					{
					int icsr = IcsrPop(cstNext);
					struct CSR huge *hpcsr = HpcsrOfIcsr(icsr);
					struct VAR huge *hpvar = 
					HpOfSbIb(sbFrame, hpcsr->pvarNext);
					NUM numVar, numT;
					MTC mtc;
					int sgn;

			/* Check for BREAK key */
					fCallOut = TRUE;

					VerifyNext(hpcsr->pvarNext);

					numVar = hpvar->num;
					numT = hpcsr->numNextStep;

					Add2Num(&numVar, &numT);
					StiNum(&numVar);
					hpvar->num = numVar;
#ifdef DEBUG
					if (fToVar)
						{
						OutExecGets(hpcsr->pvarNext, elvNum, 
								&hpvar->num);
						}
#endif


					numT = hpcsr->numNextTo;

					mtc = Cmp2Num(&numVar, &numT);
					sgn = mtc.w2;
					if (hpcsr->sgnNextStep == 1 ? sgn <= 0 : sgn >= 0)
						{
				/* Go back up to after the FOR statement.
					*/
						PushExistingIcsr(icsr);
						SetCurLib(hpcsr->libStart);
						HpeliFirst()->lib = LibCur();

						EltGetCur();
						continue;	/* read first stmt in loop */
						}
					break;
					}

			case eltSelect:
					{
					NUM numValue;
					struct EV evSwitch;
					EVI eviSwitch;
					LIB libSwitch;
					BOOL fCase;

					if (EltGetCur() != eltCase)
						{
				/* Error: Invalid SELECT CASE */
						RtError(rerrSyntaxError);
						}
					libSwitch = LibCur();

					EltGetCur();

					ResolveEvi(eviSwitch = EviParse(FALSE, FALSE, FALSE), 
							elvNil, hpNil);
					BLTBH(HpevOfEvi(eviSwitch), (char huge *)&evSwitch,
							sizeof(struct EV));
					if (evSwitch.elv == elvSd)
						evSwitch.fFree = FALSE;

					do
						{
						NUM numLow, numHigh;

				/* Set this so errors appear in a reasonable
					place */
						HpeliFirst()->lib = LibCur();

						if (Global(eltCur) != eltComma)
							{
							SkipToElt(eltCase);
							if (Global(eltCur) == eltSelect)
								{
						/* found END SELECT */
								RtError(rerrCaseElseExpected);
								}
							}

						HpeliFirst()->lib = LibCur();

						EltGetCur(); /* fetch token past Case or , */

						if (Global(eltCur) == eltElse)
							{
							EltGetCur();
							break;
							}

						if (Global(eltCur) == eltIs)
							{
							ELT elt;

							switch (elt = EltGetCur())
								{
							default:
						/* Error: Invalid Case Is */
								RtError(rerrSyntaxError);
							case eltEq:
							case eltNe:
							case eltGt:
							case eltGe:
							case eltLt:
							case eltLe:
									{
									struct EV evTest;
									EVI eviTest;

									EltGetCur();
									ResolveEvi(eviTest =
											EviParse(FALSE, 
											TRUE, FALSE),
											elvNil,
											hpNil);
									BLTBH(HpevOfEvi(eviTest),
											(char huge *)&evTest,
											sizeof(struct EV));

									if (!FExpInPev(&evSwitch) ||
											!FExpIn(elt) ||
											!FExpInPev(&evTest) ||
											!FExpIn(eltEndExp))
										{
										Assert(FALSE);
										}
									break;
									}
					/* end switch (elt) */
								}
							}
						else
							{
							struct EV evLow;
							EVI eviLow;

							eviLow = EviParse(FALSE, TRUE, FALSE);
							ResolveEvi(eviLow, elvNil, hpNil);
							BLTBH(HpevOfEvi(eviLow),
									(char huge *)&evLow,
									sizeof(struct EV));

							if (Global(eltCur) == eltTo)
								{
								struct EV evHigh;
								EVI eviHigh;

								EltGetCur();
								eviHigh = EviParse(FALSE, TRUE, FALSE);
								ResolveEvi(eviHigh, elvNil,
										hpNil);
								BLTBH(HpevOfEvi(eviHigh),
										(char huge *)&evHigh,
										sizeof(struct EV));

								if (!FExpInPev(&evSwitch) ||
										!FExpIn(eltGe) ||
										!FExpInPev(&evLow) ||
										!FExpIn(eltAnd) ||
										!FExpInPev(&evSwitch) ||
										!FExpIn(eltLe) ||
										!FExpInPev(&evHigh) ||
										!FExpIn(eltEndExp))
									{
									Assert(FALSE);
									}
								}
							else
								{
								if (!FExpInPev(&evSwitch) ||
										!FExpIn(eltEq) ||
										!FExpInPev(&evLow) ||
										!FExpIn(eltEndExp))
									{
									Assert(FALSE);
									}
								}
							}

				/* The result of the test is now on the EXP
					* stack.
					*/
						ResolveEvi(EviPop(), elvInt,
								(BOOL huge *)&fCase);
						}
					while (!fCase);

					if (Global(eltCur) == eltComma)
						{
						while (!FTermElt(EltGetCur(), FALSE))
							;
						}

					IcsrPushCst(cstCase, libSwitch);
					break;	/* go read terminator after CASE */
					}

			case eltCase:
				IcsrPop(cstCase);

			/* Skip ahead to the END SELECT.
				*/
				do
					{
					EltGetCur();	/* skip past current CASE */
					SkipToElt(eltCase);
					}
				while (Global(eltCur) != eltSelect);
				EltGetCur();
				break;	/* go read terminator */

			case eltEof:
			/* Error: End of file in procedure */
				RtError(rerrUnexpectedEndOfMacro);
				Assert(FALSE); /* NOT REACHED */

			case eltEnd:
				switch (EltGetCur())
					{
				default:
#ifdef DEBUG
					if (fToExec)
						PrintT("EXEC: END\n", 0);
#endif
					ElSyntaxError(0);
					Assert(FALSE); /* NOT REACHED */

				case eltDialog:
					ElEndDialog();
					break;

				case eltIf:
					IcsrPop(cstBlockEndif);
					break;

				case eltSelect:
					IcsrPop(cstCase);
					break;

				case eltSub:
					if (hpval != hpNil)
						{
					/* Error: END SUB encountered in FUNCTION */
						RtError(rerrSyntaxError);
						}

				/* We will be returning to somewhere in EXEC,
					* so pre-fetch a terminator token.
					*/
					EltGetCur();

					ExecEndProc();
						{{
						goto EndProc;
						}} /* !NATIVE - EldExecutePproc */

				case eltFunction:
					if (hpval == hpNil)
						{
					/* Error: END FUNCTION encountered in SUB */
						RtError(rerrSyntaxError);
						}
					ExecEndProc();
						{{
						goto EndProc;
						}} /* !NATIVE - EldExecutePproc */
					}

				EltGetCur();	/* pre-fetch */
				break;

			case eltStop:
#ifdef DEBUG
				if (fToExec)
					PrintT("EXEC: STOP\n", 0);
#endif

			/* Suspend the current EL invocation.
				*/
				StopHeli(ElGlobal(heliFirst), rerrStop);

			/* We were resumed (i.e. RerrRunHeli()) was called
				* again.
				*/
				EltGetCur();	/* pre-fetch */
				break;

			case eltGoto:
					{
					struct LAB *plab, huge *hplab;
					LIB libOld;

#ifdef DEBUG
					if (fToExec)
						PrintT("EXEC: executing GOTO\n", 0);
#endif

					fCallOut = TRUE;

			/* Tell EltGetCur() not to append a type-character.
				*/
					Global(fExpectLabel) = TRUE;
					libOld = LibCur(); /* save position for errors */
					switch (EltGetCur())
						{
					default:
						ElSyntaxError(0);

					case eltIdent:
						plab = PlabFromPsy(Global(psyCur), 0,
								Global(pprocCur));
						break;
					case eltInt:
						plab = PlabFromPsy((struct SY *)0,
								PelcrCur()->wInt,
								Global(pprocCur));
						break;
						}

					Global(fExpectLabel) = FALSE;

					hplab = HpOfSbIb(Global(sbNtab), plab);
					if (hplab->lib == libNil &&
							!FSkipToLine(eltLineIdent, plab))
						{
						SetCurLib(libOld);
						HpeliFirst()->lib = LibCur();
				/* Error: Label not found/1 */
						RtError(rerrLabelNotFound);
						}
					SetCurLib(hplab->lib);
					HpeliFirst()->lib = hplab->lib;
					EltGetCur();	/* pre-fetch */
					continue;	/* do stmt at new location */
					}

		/* Simple statements */
			case eltOn:		/* ON ERROR */
					{
					ENV env;
					struct LAB huge *hplab;
					LIB libOld;

			/* Set an ENV to jump to when we get the error.
				*/
					if (EltGetCur() != eltError)
						ElSyntaxError(0);

					libOnError = LibCur() - 1;
					if (!FExecOnError(&plabOnError))
						{
				/* Disable the error handler (ON ERROR GOTO 0).
					*/
						ENV *penvFrame = PenvGetCur(FALSE);

						if (penvFrame != 0)
							{
							ENV huge *hpenv;

					/* Invalidate an existing ENV.
						*/
							hpenv = HpOfSbIb(sbFrame, penvFrame);
							hpenv->fPcodeEnv = 0;
							hpenv->snEnv = 0;
							FSetErrRerr(rerrNil);
							}
						break;		/* done */
						}

			/* Enable the error handler for this procedure.
				*/
					if (SetJmp(&env) == 0)
						{
						ENV *penvFrame = PenvGetCur(TRUE);

				/* Copy the ENV to the frame.
					*/
						BLTBH((char huge *)&env,
								HpOfSbIb(sbFrame, penvFrame),
								sizeof(ENV));
						break;		/* end statement */
						}

			/* Error handler -- any trapped error gets
				* here.
				*/
#ifdef DEBUG
					if (fToExec)
						PrintT("EXEC: *** trapped run-time error\n", 0);
#endif
			/* Clear any expression \*or control
				* structure*\ that was in progress
				* when the error occurred.
				*/
					ClearExp();
/* We no longer clear control structures... 14dec88 bac */
/*			ElGlobal(icsrStackPtr) = icsrSavStackPtr; */

					if (plabOnError == 0)
						{
				/* Do a RESUME NEXT.
					*/
						FSetErrRerr(rerrNil);
						if (!FSkipToLine(eltLineNolabel, 0))
							Assert(FALSE);
						break;	/* read terminator token */
						}
					else
						{
				/* Go to plabOnError.
					*/
						hplab = HpOfSbIb(Global(sbNtab),
								plabOnError);
						libOld = LibCur();

						if (hplab->lib == libNil &&
								!FSkipToLine(eltLineIdent,
								plabOnError))
							{
							HpeliFirst()->lib = libOnError;
							SetCurLib(libOnError);
					/* Error: Label not found/2 */
							RtError(rerrLabelNotFound);
							}

						SetCurLib(hplab->lib);
						EltGetCur();	/* pre-fetch */
						continue;	/* do stmt at new lib */
						}
					}
				Assert(FALSE); /* NOT REACHED */

			case eltDim:
			case eltRedim:
				ExecDim(Global(eltCur) == eltRedim);
				break;

			case eltSuper:
				if (EltGetCur() != eltElx)
					{
					ElSyntaxError(0);
					Assert(FALSE); /* NOT REACHED */
					}
			/* FALL THROUGH */

			case eltElx:
					{
					ELX elx = PelcrCur()->elx;
					int cevArgs;
					EVI eviFirstArg;
					int ievSave = ElGlobal(ievStackPtr);

					EltGetCur();
					cevArgs = CevParseList(&eviFirstArg, (LIB *)0);

					if (!FTermElt(Global(eltCur), TRUE))
						ElSyntaxError(0);
					ElvCallElx(elx, cevArgs, eviFirstArg, hpNil, eltStmt == eltSuper);
					PopToIev(ievSave);	/* pop args */

					break;
					}

			case eltDialog:
				CallCmd(cmmDialog | cmmBuiltIn);
				break;

			case eltGetCurVals:
				CallCmd(cmmDefaults | cmmBuiltIn);
				break;

			case eltError:
					{
					int rerr;
					EltGetCur();
					ParseValue(elvInt, (int huge *) &rerr, eltNil);
					RtError(rerr);
					}
				break;

			case eltCall:
			case eltLet:
				EltGetCur();
			/* FALL THROUGH */

			case eltIdent:
					{
					EVI evi;
					struct EV huge *hpev;

					hpev = HpevOfEvi(evi = EviParse(TRUE, FALSE, 
							eltStmt == eltCall));

					if (hpev->fResolved && !hpev->fAggregate &&
							hpev->elv == elvNil)
						{
				/* We made a SUB call; go read the
					* statement terminator.
					*/
						if (eltStmt == eltLet)
							{
					/* Error: Invalid LET/1 */
							RtError(rerrSyntaxError);
							}
						break;
						}
					if (eltStmt == eltCall)
						{
				/* Error: Invalid CALL */
						RtError(rerrSyntaxError);
						}
					if (Global(eltCur) != eltEq)
						{
				/* Require an assignment statement, if it's
					* not a SUB call.
					*/
				/* Error: Invalid statement */
						RtError(rerrSyntaxError);
						}
					EltGetCur();

					if (hpev->fResolved)
						{
						if (!hpev->fAggregate)
							{
					/* Error: Invalid LET/2 */
							RtError(rerrSyntaxError);
							}

						switch (hpev->elvAggr)
							{
						default:
								{
								RL rlDstRek;
								RPS rps;
								ELV elv;
								ELV elvAggr;

								Assert(FRecordElv(hpev->elvAggr));

					/* Save fields out of the EV, since we
						* are about to write over it by
						* parsing another one.
						*/
								rlDstRek = hpev->rlAggr;
								rps = hpev->rpsAggr;
								elv = hpev->elv;
								elvAggr = hpev->elvAggr;
					/* Store the assignment value in the
						* record field.
						*/
								hpev = HpevOfEvi(evi = EviParse(FALSE, FALSE, FALSE));
								ResolveEvi(evi, elvNil, hpNil);
					/*if (elv == elvNil)	/* no type? */
								elv = hpev->elv;

								StoreItemToRl(rlDstRek, rps, elv,
										&hpev->num, HeldiFromIeldi(
										IeldiFromElv(elvAggr)));

					/* If we stored a string into the
						* record, we can free the SD.
						*/
								if (hpev->elv == elvSd && hpev->fFree)
									FreeSd(hpev->sd);

								break;
								}

						case elvArray:
								{
								struct AHR *pahr, huge *hpahr;
								VAL *pval;
								SD sdOld;

								pahr = PahrOfAd(
										HpvarOfPvar(hpev->pvarAdAggr)
										->ai.ad);
								hpahr = HpahrOfPahr(pahr);
								pval = (char *)
										PvalOfPahr(pahr, hpahr->cas) +
										hpev->ibAggr;

								Assert(hpev->elv != elvNil);
								if (hpev->elv == elvSd)
									{
									sdOld = *HpOfSbIb(sbArrays,
											pval);
									}
								else
									sdOld = 0;

								ParseValue(hpev->elv,
										HpOfSbIb(sbArrays, pval), eltNil);
								if (sdOld != 0)
									FreeSd(sdOld);
								break;
								}
				/* end switch (hpev->elvAggr) */
							}
						}
					else
						{
						struct VAR *pvar;

						if (hpev->fElx)
							ElSyntaxError(0);

						if (hpev->psy == psyCurProc)
							{
					/* Function value assignment. */
							struct PROC huge * hpproc;

							hpproc = HpprocOfPproc(pproc);

					/* Error if proc is not a function */
							if (hpproc->elv == elvNil)
								ElSyntaxError(0);

							ParseValue(hpproc->elv, hpval, eltNil);
							}
						else
							{
					/* Scalar variable assignment.
						*/
							pvar = PvarFromPsy(hpev->psy, elvNil,
									FALSE);
							if (HpvarOfPvar(pvar)->elv == elvFormal)
								pvar = HpvarOfPvar(pvar)->pvar;
							ExecGets(pvar, EviParse(FALSE, FALSE, 
									FALSE));
							}
						}
					break;
					}

			case eltOpen:
				DoElOpen();
				break;

			case eltClose:
				DoElClose();
				break;

			case eltWrite:
			case eltPrint:
				DoElPrint(Global(eltCur));
				break;

			case eltRead:
			case eltInput:
				DoElInput(FALSE /* fLine */, Global(eltCur));
				break;

			case eltLine:
				if (EltGetCur() != eltInput)
					{
				/* Error: Expected INPUT */
					RtError(rerrInputMissing);
					}
				DoElInput(TRUE /* fLine */, Global(eltCur));
				break;

			case eltName:
				DoElName();
				break;

			case eltSpell:
				WDoElSpell(FALSE);
				break;

			case eltThes:
				WDoElThes(FALSE);
				break;

			case eltBegin:
				if (EltGetCur() != eltDialog)
					ElSyntaxError(0);
				DoElBeginDlg();
				break;

			case eltTextBox:
				DoElEditText();
				break;

			case eltCheckBox:
				DoElCheckBox();
				break;

			case eltOptGroup:
				DoElOptionGroup();
				break;

			case eltListBox:
			case eltComboBox:
				DoElListBox(Global(eltCur));
				break;
				}

		/* Read a statement terminator.
			*/
			switch (Global(eltCur))
				{
			default:
#ifdef DEBUG
				PrintT(" (elt = %d)\n", Global(eltCur));
#endif
			/* Error: Statement not terminated properly */
				HpeliFirst()->lib = LibCur();
				RtError(rerrSyntaxError);
				break;

			case eltElseif:
			case eltElse:
			/* Encountered an ELSE while executing statements.
				* Make sure we were expecting one, then skip ahead
				* to the end of the line.
				*/
				IcsrPop(cstSingleElse);
				SkipSingleIf(FALSE);
				celtSingleIf--;
				break;

			case eltColon:
			case eltComment:
			case eltNil:
				EltGetCur();
			/* FALL THROUGH */

			case eltLineIdent:
			case eltLineInt:
			case eltLineNolabel:
			case eltEof:

				break;		/* parse another line */
				}
			}
		}}
	Assert(FALSE); /* NOT REACHED */

EndProc:
	RestoreExp(&esb);
	(*HpeliFirst()->pfnDebug)(eldSubOut, ElGlobal(heliFirst));
	return eldNormalStep;

}


/* Returns fTrue iff the elt can terminate a statement. */
/* %%Function:FTermElt %%Owner:bradch */
FTermElt(elt, fElseOK)
ELT elt;
{
	switch (elt)
		{
	case eltElse:
		return fElseOK;

	case eltColon:
	case eltComment:
	case eltNil:
	case eltLineIdent:
	case eltLineInt:
	case eltLineNolabel:
	case eltEof:
		return TRUE;
		}

	return FALSE;
}


/* (pj 3/23) takes 11% of macro start/run */
/* %%Function:PelcrGet %%Owner:bradch */
NATIVE PelcrGet(pchBufStart, pibBufCur, pchBufLim)
/* This is the low-level buffer-reading routine, which may eventually be
* hand-coded.  It produces a pelcr, pointing somewhere into the token
* buffer, or NULL at end of file.
*/
char *pchBufStart;
WORD *pibBufCur;
char *pchBufLim;
{
	char *pchCur;
	char elcc;
	struct ELCR *pelcr;
	int cbElcr, ib;
	BOOL fLoop, fExtLine, fCnvtLine, fComment;

	/* First find the beginning of the next ELCR. */
	fComment = fCnvtLine = fExtLine = FALSE;
	do
		{
		fLoop = FALSE;
		pchCur = Global(rgchBuf) + *pibBufCur;
		if (pchCur >= pchBufLim)
			{
			pchBufLim = PchRefillBuf(pchBufStart);
			pchCur = pchBufStart;
			}

		pelcr = pchCur;
		elcc = *pchCur++;

		Assert(FIsElccCh(elcc));

		/* Get the whole fixed-size part loaded into the buffer. */
		cbElcr = CbFromElcc(elcc);
		if ((char *) pelcr + cbElcr > pchBufLim)
			{
			BLTB(pelcr, pchBufStart, pchBufLim - (char *)pelcr);
			pchBufLim = PchRefillBuf(pchBufStart + 
					(pchBufLim - (char *) pelcr));
			pelcr = pchBufStart;
			}
		Assert((char *) pelcr + cbElcr <= pchBufLim);

		if (fExtLine && !FLineElt(EltFromElcc(elcc)))
			{
			HpeliFirst()->lib = LibCur() - 1;
			ElSyntaxError(0);
			fExtLine = FALSE;
			}

		/* We've now got the fixed-size part.  If there's a 
			variable-sized part, read that. */

		switch (elcc)
			{
		default:
			goto ReturnPelcr;

		case elccIdent:
			cbElcr += pelcr->cchIdent;
			break;

		case elccLineIdent:
			cbElcr += pelcr->cchLineIdent;
			/* FALL THROUGH */

		case elccLineInt:
			/* LineIdent and LineInt must be converted to 
				Ident and Int respectively iff fExtLine. */
			if (!(fCnvtLine = fExtLine))
				vibLineStart = pelcr->ib;
			fComment = FALSE;
			break;

		case elccLineNolabel:
			vibLineStart = pelcr->ib;
			fLoop = fExtLine;
			fExtLine = FALSE;
			fComment = FALSE;
			break;

		case elccString:
			cbElcr += pelcr->cchString;
			break;

		case elccBadSyntax:
			HpeliFirst()->lib = LibCur();
			ElSyntaxError(0);
			Assert(FALSE); /* NOT REACHED */

		case elccRem:
		case elccComment:
			fComment = TRUE;
			/* FALL THROUGH */

		case elccSpace:
			fLoop = TRUE;
			/* FALL THROUGH */

		case elccUserElk:
			cbElcr += pelcr->cch;
			break;

		case elccExtLine:
			fExtLine = TRUE;
			fLoop = TRUE;
			break;
			}

		/* Do the same thing again ... make sure there's room 
			for the whole ELCR in the buffer.  (Compile this with 
			-oq maybe?) */
		if ((char *) pelcr + cbElcr > pchBufLim)
			{
			BLTB(pelcr, pchBufStart, pchBufLim - (char *)pelcr);
			pchBufLim = PchRefillBuf(pchBufStart + 
					(pchBufLim - (char *)pelcr));
			pelcr = pchBufStart;
			}
		Assert((char *) pelcr + cbElcr <= pchBufLim);

ReturnPelcr:
		*pibBufCur = (char *) pelcr + cbElcr - Global(rgchBuf);
		Global(ibElcrCur) = (char *) pelcr - Global(rgchBuf);
		} 
	while (elcc != elccEof && 
			(fLoop || (fComment && !FLineElt(EltFromElcc(elcc)))));

	if (fCnvtLine)
		{
		struct ELCR * pelcrT;

		/* NOTE: 3 here is the difference between the length of a
			elccLineInt and elccInt or elccLineIdent and elccIndent
			elcr.  This value is not computed as it involves a table
			lookup and would be less efficient. */
		pelcrT = (struct ELCR *) ((char *) pelcr + 3);
		pelcrT->elcc = elcc == elccLineInt ? elccInt : elccIdent;
		pelcr = pelcrT;
		}

	return pelcr;
}




/* %%Function:PchRefillBuf %%Owner:bradch */
char *PchRefillBuf(pchBufStart)
/* Specialized buffer-refill function.  Refills the token buffer starting at
* pchStart (a position within the buffer).  Sets Global(ibBufLim) to the
* new end-of-buffer position.
*
* Returns the new value of Global(ibBufLim).
*
* If no more text can be read, elccEof is stored into the buffer.
*/
char *pchBufStart;
{
	int cchWanted = &Global(rgchBuf)[cchTokenBuf] - pchBufStart;
	int cchGot;

#ifdef DEBUG
	if (fToRead)
		{
		PrintT("READ: refilling token buffer from offset %u\n",
				pchBufStart - Global(rgchBuf));
		PrintT("READ: requesting %d bytes", cchWanted);
		PrintT(" from lib %u\n", (unsigned)Global(libBufEnd));
		}
#endif

	HpeliFirst()->lib = Global(libBufEnd);
	cchGot = (*HpeliFirst()->pfnCchReadSource)
			(pchBufStart, cchWanted, ElGlobal(heliFirst));

	Assert(cchGot >= 0);
	Assert(cchGot <= cchWanted);

	if (cchGot == 0)
		{
LEof:
		pchBufStart[cchGot++] = elccEof;
		}

	Global(libBufStart) = Global(libBufEnd) -
			(pchBufStart - Global(rgchBuf));
	Global(libBufEnd) += cchGot;

	Global(ibBufLim) = pchBufStart - Global(rgchBuf) + cchGot;

#ifdef DEBUG
	if (fToRead)
		{
		int ich;

		PrintT("READ: got %u bytes: ", cchGot);
		for (ich = 0; ich < cchGot; ich++)
			{
			if (ich >= 16)
				{
				PrintT("...", 0);
				break;
				}
			PrintT("%02x ", pchBufStart[ich]);
			}
		PrintT("\n", 0);
		}
#endif

	return pchBufStart + cchGot;
}


/* %%Function:EltGetCur %%Owner:bradch */
ELT EltGetCur()
	/* Reads the next ELT from the token buffer.  If a label or procedure
	* definition is found which has not already been processed, it is processed
	* now.
	*/
	/* (pj 3/23) takes 7% of macro start/run time */
{{ /* NATIVE - EltGetCur */
	struct ELCR *pelcr;
	ELT elt;
	LIB libStart, libCur;
	ELCC elcc;

	if (fToExec)
		libStart = LibCur();

	pelcr = PelcrGet(Global(rgchBuf), &Global(ibBufCur),
			Global(rgchBuf) + Global(ibBufLim));

	Global(ibElcrCur) = (char *)pelcr - Global(rgchBuf);

	switch (elcc = pelcr->elcc)
		{
	case elccIdent:
		Global(psyCur)
				= PsyLookupSt(&pelcr->cchIdent, Global(fExpectLabel));
		Global(fExpectLabel) = FALSE;
		elt = eltIdent;
		break;

	case elccLineNolabel:
		if ((libCur = LibCur()) > *(LIB huge *)HpNtab(libScan))
			*(LIB huge *)HpNtab(libScan) = libCur;
		elt = eltLineNolabel;
		break;

	case elccElt:
		elt = pelcr->elt;
		break;

	case elccLineIdent:
	case elccLineInt:
		if ((libCur = LibCur()) > *(LIB huge *)HpNtab(libScan))
			{
			if (elcc == elccLineIdent)
				{
				DefLabel(PsyLookupSt(&pelcr->cchLineIdent,
						TRUE),
						0, libCur);
				}
			else
				DefLabel((struct SY *)0, pelcr->ln, libCur);

			*(LIB huge *)HpNtab(libScan) = libCur;
			}
		/* FALL THROUGH */

	default:
		Assert(elcc >= elccMin && elcc < elccLim);
		elt = EltFromElcc(elcc);
		break;
		}

#ifdef DEBUG
	if (fToExec)
		OutElt(elt, libStart);
#endif

	Assert(elt != eltComment && elt != eltSpace);
	return Global(eltCur) = elt;
}}


/* %%Function:SetCurLib %%Owner:bradch */
SetCurLib(lib)
LIB lib;
{
	LIB libBufStart = Global(libBufStart);

#ifdef DEBUG
	if (fToExec)
		PrintT("EXEC: set lib = %u\n", (WORD)lib);
#endif

	if (lib >= libBufStart && lib <= Global(libBufEnd))
		{
		/* Found the location within the current buffer.
			*/
		Global(ibBufCur) = (WORD)(lib - libBufStart);
		}
	else
		{
		/* We will need to refill the buffer.  Set the "Cur" and "Lim"
			* pointers to the start of the buffer (LibCur() will now
			* return the correct value, even before the refill).
			*/
		Global(ibBufLim) = Global(ibBufCur) = 0;
		Global(libBufStart) = Global(libBufEnd) = lib;
		}
}


/* Control Structure handlers */


/* I C S R  P O P */
/* Pops a CSR off the CSR stack. */
/* %%Function:IcsrPop %%Owner:bradch */
IcsrPop(cstExpected)
CST cstExpected;
{
	int icsr;
	struct CSR huge *hpcsr;

	Assert(ElGlobal(icsrStackPtr) >= 0);

	while (1)
		{
		icsr = ElGlobal(icsrStackPtr);
		hpcsr = HpcsrOfIcsr(icsr);

		if (hpcsr->cst == cstExpected ||
				cstExpected == cstBlockEndif && hpcsr->cst == cstBlockElse)
			{
			ElGlobal(icsrStackPtr) -= 1;
			break;		/* found what we want */
			}

		if (hpcsr->cst == cstEndProc)
			{
			RERR rerr;

			/* Oops -- what we're looking for wasn't there. */

			switch (cstExpected)
				{
			default:
				rerr = rerrSyntaxError;
				break;

			case cstWend:
				/* Error: WEND without WHILE */
				rerr = rerrWendWithoutWhile;
				break;

			case cstNext:
				/* Error: NEXT without FOR */
				rerr = rerrNextWithoutFor;
				break;

			case cstSingleElse:
			case cstBlockElse:
				rerr = rerrElseWithoutIf;
				break;

			case cstBlockEndif:
				/* Error: ELSE or END IF without IF */
				rerr = rerrEndIfWithoutIf;
				break;
				}

			RtError(rerr);
			}

		ElGlobal(icsrStackPtr) -= 1;
		}

#ifdef DEBUG
	if (fToCtrl)
		DumpIcsr(icsr, FALSE);
#endif

	return icsr;
}


/* %%Function:IcsrPushCst %%Owner:bradch */
int IcsrPushCst(cst, libStart)
/* Pushes a new CSR on the CSR stack, with type CST.  A pointer to the new
* CSR is returned, so that the caller can fill in the fields.
*/
CST cst;
LIB libStart;
{
	int icsrNew, icsrLast;
	struct CSR huge *hpcsrOld, huge *hpcsrNew;

	icsrLast = ElGlobal(icsrStackPtr);

	/* If there's already a CSR (in the current procedure) with the same
		* libStart, delete it.
		*/
	if (libStart != libNil)
		{
		int icsrOld;

		for (icsrOld = icsrLast;
				icsrOld >= 0 &&
				((hpcsrOld = HpcsrOfIcsr(icsrOld))->cst != cstEndProc);
				icsrOld--)
			{
			if (hpcsrOld->libStart == libStart)
				{
				BLTBH(hpcsrOld + 1, hpcsrOld,
						(icsrLast - icsrOld)*sizeof(struct CSR));
				icsrLast--;
				break;
				}
			}
		}

	if (icsrLast >= ccsrStackMax - 1)
		RtError(rerrTooManyCtrlStructs);

	icsrNew = icsrLast + 1;
	ElGlobal(icsrStackPtr) = icsrNew;

	(hpcsrNew = HpcsrOfIcsr(icsrNew))->cst = cst;
	hpcsrNew->libStart = libStart;

#ifdef DEBUG
	if (fToCtrl)
		DumpIcsr(icsrNew, TRUE);
#endif

	return icsrNew;
}


/* %%Function:PushExistingIcsr %%Owner:bradch */
PushExistingIcsr(icsr)
/* Pushes a CSR that was just popped off the top of the CSR stack.
*/
int icsr;
{
	ElGlobal(icsrStackPtr)++;

	Assert(icsr == ElGlobal(icsrStackPtr));

#ifdef DEBUG
	if (fToCtrl)
		DumpIcsr(icsr, TRUE);
#endif
}


/* %%Function:SkipToElt %%Owner:bradch */
SkipToElt(eltWanted)
/* Skips the body of a "WHILE" or "FOR" loop.  "elt" should be either eltWend
* or eltNext.  Also deals with eltCase! 
*/
ELT eltWanted;
{
	long libSave;
	ELT eltGot;
	int celtNested = 0;

	Assert(eltWanted == eltWend || 
			eltWanted == eltNext ||
			eltWanted == eltCase);

	libSave = LibCur();

	for (eltGot = Global(eltCur);
			!(eltGot == eltWanted && celtNested == 0); 
			eltGot = EltGetCur())
		{
		switch (eltGot)
			{
		case eltFor:
			if (eltWanted == eltNext)
				celtNested++;
			break;

		case eltWhile:
			if (eltWanted == eltWend)
				celtNested++;
			break;

		case eltSelect:
			if (eltWanted == eltCase)
				celtNested++;
			break;

		case eltWend:
		case eltNext:
			if (eltGot == eltWanted)
				celtNested--;
			break;

		case eltEnd:
			switch (EltGetCur())
				{
			case eltSub:
			case eltFunction:
				goto LMissing;

			case eltSelect:
				if (eltWanted == eltCase)
					{
					if (celtNested-- == 0)
						return;
					}
				/* break; */
				}
			break;

		case eltSub:
		case eltFunction:
LMissing:
			SetCurLib(libSave);
			HpeliFirst()->lib = libSave - 1;
			RtError((eltWanted == eltCase) ? 
					rerrMissingEndSelect : rerrMissingNextOrWend);
			Assert(FALSE); /* NOT REACHED */
			break;

		case eltEof:
			SetCurLib(libSave);
			HpeliFirst()->lib = libSave - 1;
			/* Error: Unexpected EOF in control structure */
			RtError(rerrUnexpectedEndOfMacro);
			/* break; */
			}
		}

	/* Found it ... */
}


/* %%Function:SkipSingleIf %%Owner:bradch */
SkipSingleIf(fSeekElse)
/* Skips some code, in response to a single-line IF.  If fSeekElse, skipping
* stops on the ELSE; otherwise, the whole rest of the line is skipped.
*/
BOOL fSeekElse;
{
	int celtNestedIfs = 0;
	ELT elt;

	for (elt = Global(eltCur); ; elt = EltGetCur())
		{
		switch (elt)
			{
		case eltLineIdent:
		case eltLineNolabel:
		case eltLineInt:
			return;

		case eltIf:
			celtNestedIfs++;
			break;

		case eltElse:
			if (fSeekElse && celtNestedIfs == 0)
				return;
			celtNestedIfs--;
			break;

		case eltSub:
		case eltFunction:
			ElSyntaxError(0);
			Assert(FALSE); /* NOT REACHED */

		case eltEof:
			/* Error: Unexpected EOF */
			RtError(rerrUnexpectedEndOfMacro);
			/* break; */
			}
		}
}


/* %%Function:FSkipToLine %%Owner:bradch */
BOOL FSkipToLine(eltObject, plab)
/* Skips to some construct at the beginning of a line.  eltObject should be
* one of:
*	eltElse -- look for the ELSE in a multi-line IF; return TRUE if we
*		found it, or FALSE if "end if" was found.
*	eltIf -- look for the "end if" in a multi-line IF.
*	eltEnd -- look for "end sub" or "end function".
*	eltSub -- stop after any SUB or FUNCTION definition.
*	eltLineNolabel -- stop at the next line, whatever it is.
*	eltLineIdent -- stop when plab becomes defined.
*/
ELT eltObject;
struct LAB *plab;
	/* takes 7.03% of macro run time */
{{ /* NATIVE - FSkipToLine */
	long libSave;
	ELT elt;
	int celtNested;
	struct LAB huge * hplab;
	BOOL fThenForElseIf;

	Assert(eltObject == eltElse || 
			eltObject == eltIf ||
			eltObject == eltEnd || 
			eltObject == eltSub ||
			eltObject == eltLineIdent || 
			eltObject == eltLineNolabel);

	celtNested = 0;

	libSave = LibCur();

	fThenForElseIf = FALSE;

	for (;;)
		{
		switch (Global(eltCur))
			{
		case eltEnd:
			switch (EltGetCur())
				{
			case eltIf:
				if (celtNested == 0 &&
						(eltObject == eltElse ||
						eltObject == eltIf))
					{
					return FALSE;
					}
				celtNested -= 1;
				break;

			case eltSub:
			case eltFunction:
				if (eltObject == eltEnd)
					return TRUE;

				if (eltObject == eltLineIdent)
					return FALSE;

				EltGetCur();	/* ignore it */
				break;
				}
			break;

		case eltElseif:
			fThenForElseIf = TRUE;
			break;

		case eltIf:
			fThenForElseIf = FALSE;
			break;

		case eltLineIdent:
		case eltLineNolabel:
		case eltLineInt:
			if (eltObject == eltLineNolabel)
				return TRUE;

			if (((elt=EltGetCur()) == eltElse || elt == eltElseif) &&
					eltObject == eltElse && 
					celtNested == 0)
				{
				return TRUE;
				}

			if (eltObject == eltLineIdent)
				{
				hplab = HpOfSbIb(Global(sbNtab), plab);
				Assert(hplab != NULL);
				if (hplab->lib != libNil)
					return TRUE;
				}
			continue;	/* already got next elt */

		case eltThen:
			if (eltObject == eltElse || eltObject == eltIf)
				{
				ELT elt = EltGetCur();

				if (FLineElt(elt))
					{
					if (!fThenForElseIf)
						celtNested += 1;
					}
				else  if (fThenForElseIf)
					ElSyntaxError(0);
				fThenForElseIf = FALSE;
				continue;	/* already got next elt */
				}
			break;

		case eltSub:
		case eltFunction:
			if (eltObject == eltEnd)
				{
				/* Error: Nested SUB or FUNCTION definition */
				RtError(rerrNestedSubOrFunc);
				}

			if (eltObject == eltSub)
				{
				EltGetCur();	/* pre-fetch */
				return TRUE;
				}
			break;

		case eltEof:
			switch (eltObject)
				{
			default:
				SetCurLib(libSave);
				HpeliFirst()->lib = LibCur() - 1;
				/* Error: Unexpected EOF  */
				RtError(rerrUnexpectedEndOfMacro);

			case eltSub:
				Assert(FALSE);
#ifdef HISTORY
				SetCurLib(libSave);
				HpeliFirst()->lib = LibCur() - 1;
				/* Error: Undefined SUB or FUNCTION */
				RtError(rerrSubprogNotDefined);
#endif
			case eltLineIdent:
			case eltLineNolabel:
				return FALSE;
				}
			Assert(FALSE); /* NOTREACHED */
			}

		EltGetCur();
		}
}}





/* %%Function:VerifyNext %%Owner:bradch */
VerifyNext(pvar)
/* Called when elgCur is eltNext; reads the variable in the NEXT statement,
* and makes sure it's the same as pvar.
*/
struct VAR *pvar;
{
	if (EltGetCur() == eltIdent)
		{
		if (PvarFromPsy(Global(psyCur), elvNil, FALSE) != pvar)
			RtError(rerrNextWithoutFor);

		EltGetCur();
		}
}


/* %%Function:DefLabel %%Owner:bradch */
DefLabel(psy, ln, lib)
struct SY *psy;
WORD ln;
LIB lib;
{
	struct LAB *plab, huge *hplab;

	plab = PlabFromPsy(psy, ln, *HpNtab(pprocScan));
	hplab = HpOfSbIb(Global(sbNtab), plab);

#ifdef DEBUG
	if (fToExec)
		{
		PrintT("EXEC: defining label ", 0);
		if (psy != 0)
			{
			PrintT("\"", 0);
			PrintHst(HpsyOfPsy(psy)->st);
			PrintT("\"", 0);
			}
		else
			PrintT("#%u", ln);

		PrintT(" at %u\n", (WORD)lib);
		}
#endif

	if (hplab->lib == libNil)
		hplab->lib = lib;
	else  if (hplab->lib != lib)
		{
		/* Error: Label defined twice */
		RtError(rerrDuplicateLabel);
		}
}


/* %%Function:ExecDim %%Owner:bradch */
ExecDim(fRedim)
BOOL fRedim;
{
	ELX elx = elxNil;
	BOOL fShared = FALSE, fElx = FALSE;

	if (EltGetCur() == eltShared)
		{
		if (Global(pprocCur) != 0)
			ElSyntaxError(0);
		fShared = TRUE;
		EltGetCur();
		}

	while (Global(eltCur) == eltIdent)
		{
		struct SY *psyDim = Global(psyCur);
		int fArray;
		ELV elv;
		struct VAR huge *hpvar;
		int cbElement, cev;
		AS rgas[casMax];

		elv = elvNil;		/* use SY type as base-type */

		if (fArray = EltGetCur() == eltLParen)
			{	/* -- array DIM -- */
			int iev;
			EVI eviFirst;

			EltGetCur();		/* fetch token after LParen */
			cev = CevParseList(&eviFirst, (LIB *)0);
			if (cev == 0 || cev > casMax)
				{
				/* Error: Invalid number of array dimensions (cev) */
				RtError(rerrWrongNumOfDimensions);
				}
			if (Global(eltCur) != eltRParen)
				{
				/* Error: Missing ')' in DIM statement */
				RtError(rerrMissingRightParen);
				}
			EltGetCur();

			/* Store the array dimensions into rgas.
				*/
			for (iev = 0; iev < cev; iev++)
				{
				ResolveEvi(eviFirst + EviOfIev(iev), elvInt,
						(VAL huge *)&rgas[iev]);

				/* Increment the dimension, so we get BASIC-
					* style arrays (i.e. "dim x(10)" produces 11
					* elements x(0) ... x(10)).
					*/
				rgas[iev]++;
				}
			PopToIev(IevOfEvi(eviFirst) - 1);
			}

		if (Global(eltCur) == eltAs)
			{
			/* DIM var AS elx */
			EltGetCur();
			if (Global(eltCur) == eltDialog)
				EltGetCur();

			if (Global(eltCur) != eltElx)
				{
				/* Error: Illegal type in DIM ... AS ... */
				RtError(rerrTypeMismatch);
				}
			elv = ElvFromIeldi(IeldiFromElx(elx = PelcrCur()->elx));
			EltGetCur();
			fElx = TRUE;
			}

		/* We now have elv and fArray; create the variable.
			*/
		hpvar = HpvarOfPvar(PvarFromPsyElx(psyDim, elv, fArray, elx));

		/* Redimension a record variable */
		if (fElx && hpvar->elx != elx)
			{
			if (!fRedim)
				{
				/* Error: Redimensioned array; use REDIM */
				RtError(rerrArrayAlreadyDimensioned);
				}

			if (hpvar->rek.hcab != 0)
				{
				FreeCab(hpvar->rek.hcab);
				hpvar->rek.hcab = 0;
				}

			hpvar->rek.ielfdButton = 0;
			hpvar->elx = elx;
			hpvar->elv = elv;
			}

		if (fShared)
			hpvar->fShared = TRUE;

		if (fArray)
			{
			if (hpvar->ai.ad != 0)
				{
				if (!fRedim)
					{
					/* Error: Redimensioned array; use REDIM */
					RtError(rerrArrayAlreadyDimensioned);
					}
				if (hpvar->ai.elv == elvSd)
					FreeStringsAd(hpvar->ai.ad);
				FreeAd(hpvar->ai.ad);
				hpvar->ai.ad = 0;
				}

			if (elv == elvNil)
				elv = hpvar->ai.elv;
			else
				Assert(hpvar->ai.elv = elv);

			switch (elv)
				{
			default:
				Assert(FRecordElv(elv));
				cbElement = sizeof(REK);
				break;
			case elvNum:	
				cbElement = sizeof(NUM);	
				break;
			case elvSd:	
				cbElement = sizeof(SD);		
				break;
				}

			hpvar->ai.ad = AdCreate(cbElement, cev, rgas);
			}

		switch (Global(eltCur))
			{
		case eltComma:
			if (EltGetCur() != eltIdent)
				{
				/* Error: Invalid DIM statement/1 */
				RtError(rerrSyntaxError);
				}
			break;
		case eltIdent:
			/* Error: Invalid DIM statement/2 */
			RtError(rerrSyntaxError);
			break;
			}
		}
}


/* %%Function:PprocFindPsy %%Owner:bradch */
struct PROC *PprocFindPsy(psy)
/* Locates a SUB or FUNCTION definition for psy, searching the program text
* if necessary.
*/
struct SY *psy;
{
	struct PROC *pproc;
	LIB libSave;

	if ((pproc = PprocFromPsy(psy, -1)) != 0)
		return pproc;

	/* Error: Undefined SUB or FUNCTION/2 */
	RtError(rerrSubprogNotDefined);
}


/* %%Function:ElvCallElx %%Owner:bradch */
ELV ElvCallElx(elx, cevArgs, eviFirstArg, hpvalReturn, fSuper)
ELX elx;
int cevArgs;
EVI eviFirstArg;
VAL huge *hpvalReturn;
BOOL fSuper;
{
	extern ModeError();
	extern int cElParams;
	ELE ele;
	ELR elr;
	int iev, ielp, cevPositional, cwPushed;
	BOOL fPushingArgs, fFreeHeapStrings = FALSE;
	BOOL fElElx = FElElx(elx);
	BOOL fRecordArg, fDoCall;
	RL rl;	/* identifies the record passed to a dialog statement */
	CMB cmb;
	ELDI **heldi;

	if (Global(wStackStart) - (WORD)&elx > cbElStackUsage)
		{
		/* Error: Insufficient stack space to call ELX */
		RtError(rerrOutOfMemory);
		}

	cElParams = cevArgs; /* BAC */

#ifdef DEBUG
	if (fToExec)
		{
		PrintT("EXEC: ElvCallElx(elx = %d", elx);
		PrintT(", cevArgs = %d ..., ", cevArgs);
		if (hpvalReturn == hpNil)
			PrintT("hpNil)\n", 0);
		else
			PrintT("hpval)\n", 0);
		}
#endif
	(*HpeliFirst()->pfnGetInfoElx)(elx, &ele, ElGlobal(heliFirst),
			(vfElFunc = hpvalReturn != hpNil), fSuper ? cmmSuper : 0);

	if (ele.pfn == ModeError)
		RtError(rerrModeError);

	if (hpvalReturn == hpNil ? !ele.fStmt : !ele.fFunc)
		{
		/* Error: Invalid call to built-in statement */
		RtError(rerrSyntaxError);
		}

	Assert(ele.pfn != 0);
	Assert(!(ele.elr == elrDialog && ele.celpMin != ele.celpMac));

	/* Ensure that if this is being called as a function that it's return
		type is not void */
	elr = ele.elr;
	if (elr != elrDialog &&
			(elr == elrVoid) != (hpvalReturn == hpNil))
		{
		ElSyntaxError(0);
		}

	/* Resolve (and check the types of) any positional arguments
		* which the ELX expects.
		*/

	for (ielp = 0; ielp < ele.celpMac; ielp++)
		{
		struct EV huge *hpev;
		ELV elvArg;
		ELP elp;

		if (ielp >= cevArgs)
			break;

		hpev = HpevOfEvi(eviFirstArg + EviOfIev(ielp));

		if (hpev->elk != elkNil || FRecordElv(hpev->elv))
			break;		/* no more positional args */

		switch (elp = ele.rgelp[ielp])
			{
		default:
			Assert(FALSE);
		case elpInt:	
			elvArg = elvInt;	
			break;
		case elpNum:	
			elvArg = elvNum;	
			break;
		case elpHst:
		case elpHpsd:	
			elvArg = elvSd;		
			break;
		case elpHpsdNum:
			if (hpev->elv == elvSd)
				elvArg = elvSd;
			else
				elvArg = elvNum;
			break;
			}

		ResolveEvi(eviFirstArg + EviOfIev(ielp), elvArg, hpNil);

		if (elp == elpHst)
			{
			/* Allocate a handle to the string on
				* the application heap.
				*/
			hpev->hst = HstzOfSd(hpev->sd);
			fFreeHeapStrings = TRUE;
			}
		}

	/* If there is one extra argument to a dialog statement, check to see
		* whether it's a dialog record -- if so, fRecordArg becomes TRUE.
		*/
	fRecordArg = FALSE;		/* default */
	if (elr == elrDialog && cevArgs - ele.celpMac == 1)
		{
		struct EV huge *hpev;

		hpev = HpevOfEvi(eviFirstArg + EviOfIev(ele.celpMac));
		if (hpev->elk == elkNil)
			{
			if (!hpev->fResolved)
				{
				/* It's a variable -- attempt to resolve it as
					* a record variable.
					*/
				BOOL fEnableOn = Global(fEnableDefine);
				/* probably unnecessary to save this */

				Global(fEnableDefine) = FALSE;
				fRecordArg = !hpev->fElx &&
						FRecordPsy(hpev->psy, FALSE);
				Global(fEnableDefine) = fEnableOn;
				}
			else
				fRecordArg = FRecordElv(hpev->elv);
			}
		}

	if (ele.elr != elrDialog)
		{
		if (cevArgs < ele.celpMin || cevArgs > ele.celpMac)
			RtError(rerrArgCountMismatch);

		/* Tried to use a keyword argument in a non-dialog statement */
		if (cevArgs > ielp)
			RtError(rerrBadParam);
		}

	vcElParams = ielp;
	cevPositional = ielp;

	if (elr == elrDialog)
		{
		heldi = HeldiFromIeldi(ele.ieldi);

		if (fRecordArg)
			{
			/* The record has been prepared by the programmer.
				*/
			ResolveEvi(eviFirstArg, ElvFromIeldi(ele.ieldi),
					hpNil);
			rl = HpevOfEvi(eviFirstArg)->rl;
			}
		else
			{
			/* No record is explicitly specified, so we will set
				* up a temporary one to use as an argument.
				*/
			rl.pvar = Global(pvarTmpRek);
			rl.ib = ibNil;
			FreeRek(HpvarOfPvar(rl.pvar)->rek); /* BAC */
			}
		}

	for (fDoCall = !(elr == elrDialog && !fRecordArg); fDoCall < 2;
			fDoCall++)
		{
		/* If fDoCall, we are pushing the arguments to call an ELX.
			*
			* If !fDoCall, we are pushing args to get the default record
			* for a dialog call.  After getting the default record, we
			* modify it using the keyword arguments which were specified,
			* and repeat the loop (which will push args and make the call).
			*/
		HCAB hcabArg;

		if (!fDoCall)
			{
			/* Simulate the non-interactive function form ...
				*/
			ExpectHeldi(heldi);
			}
		else  if (elr == elrDialog)
			{
			/* Set up the appropriate SDM state for the dialog
				* call ...
				*/
			ELDI huge *hpeldi;
			WORD ielfdButton;
			TMC tmc;

			hpeldi = HpeldiFromHeldi(heldi);
			ielfdButton = HprekOfRl(rl)->ielfdButton;
			if (ielfdButton == ielfdNil)
				tmc = tmcOK; /* BAC: was tmcNil */
			else
				tmc = hpeldi->rgelfd[ielfdButton].tmc;
			cmb.tmc = tmc;

			/* If we are calling the statement with the CAB from a
				* permanent variable, duplicate it into the temporary
				* CAB variable.
				*/
			if (rl.pvar != Global(pvarTmpRek))
				{
				struct VAR huge *hpvarT;

				hpvarT = HpvarOfPvar(Global(pvarTmpRek));
				hpvarT->elv = ElvFromIeldi(ele.ieldi);
				hpvarT->rek = *HprekOfRl(rl);
				if (hpvarT->rek.hcab != 0)
					{
					if ((hpvarT->rek.hcab =
							HcabDupeCab(hpvarT->rek.hcab)) == NULL)
						RtError(rerrOutOfMemory);
					}
				rl.pvar = IbOfHp(hpvarT);
				rl.ib = ibNil;
				}
			hcabArg = HprekOfRl(rl)->hcab;
			}

		/* Push all positional arguments on the Pcode stack.  No
			* procedure calls are allowed during this loop (because there
			* is stuff on the stack).
			*/
		cwPushed = 0;
		for (ielp = 0; ielp < ele.celpMac; ielp++)
			{
			struct EV huge *hpev;
			ELV elvArg;
			ELP elp;

			if (ielp >= cevPositional)
				{	/* -- missing optional arg -- */
				/* Push a null value for the argument.
					*/
				switch (ele.rgelp[ielp])
					{
				default:
					Assert(FALSE);
				case elpInt:
				case elpHst:
					StackW(0);
					cwPushed++;
					break;
				case elpHpsd:
					StackL(hpNil);
					cwPushed++;
					break;
				case elpHpsdNum:
					StackL(hpNil);
					cwPushed += 2;
					/* fall through */
				case elpNum:
					StackW(0);
					StackW(0);
					StackW(0);
					StackW(0);
					cwPushed += 4;
					break;
					}
				continue;	/* minimize indentation */
				}

			hpev = HpevOfEvi(eviFirstArg + EviOfIev(ielp));

			switch (elp = ele.rgelp[ielp])
				{
			default:
				Assert(FALSE);
			case elpInt:	
				elvArg = elvInt;	
				break;
			case elpNum:	
				elvArg = elvNum;	
				break;
			case elpHst:
			case elpHpsd:	
				elvArg = elvSd;		
				break;
			case elpHpsdNum:
				if (hpev->elv == elvSd)
					elvArg = elvSd;
				else
					elvArg = elvNum;
				break;
				}

			switch (elp)
				{
				NUM numT;

			default:
				while (cwPushed-- > 0)
					WordFromStack();
				Assert(FALSE);
			case elpInt:
				StackW(hpev->w);
				cwPushed += 1;
				break;
			case elpHst:
				StackW(hpev->hst);
				cwPushed += 1;
				break;
			case elpHpsd:
				StackL(&hpev->sd);
				cwPushed += 2;
				break;
			case elpHpsdNum:
				if (elvArg == elvSd)
					{
					/* Push the hpsd, and a
						* garbage NUM.
						*/
					StackL(&hpev->sd);
					StackNum(numT);
					cwPushed += 2 +
							cbNUM/sizeof(int);
					break;
					}
				/* Push hpNil, and the NUM
					* value.
					*/
				StackL(hpNil);
				cwPushed += 2;
				/* fall through */
			case elpNum:
				BLTBH(&hpev->num,
						(NUM huge *)&numT, cbNUM);
				StackNum(numT);
				cwPushed +=
						sizeof(NUM) / sizeof(int);
				break;
				}
			}	/* end for (ielp) */

		/* The arguments are now pushed on the Pcode stack.  Make the
			* call and store the value.
			*/
		switch (elr)
			{
		default:
			Assert(FALSE);
		case elrDialog:
				{
				HCAB hcab;

				if (!fDoCall)
					{
					int cmd;
				/* Get the CAB containing the default values
					* from the application.
					*/
					REK huge *hprek;
					ELDI huge * hpeldi;

					hpeldi = HpeldiFromHeldi(heldi);
					cmb.pv = pvNil;
					cmb.cmm = cmmDefaults | cmmBuiltIn;
					if ((cmb.hcab = HcabAlloc(hpeldi->cabi)) == NULL)
						RtError(rerrOutOfMemory);
					cmb.bcm = BcmFromIbcm(elx);

					Debug(vfInCmd++);
					vrerr = rerrNil;
					cmd = (*(int (*)())ele.pfn)(&cmb);
					Debug(vfInCmd--);

					if (cmd != cmdOK)
						{
						RtError(vrerr == rerrNil ?
								rerrCommandFailed : vrerr);
						}

					FSaveElHcab(cmb.hcab, 
							HpeldiFromHeldi(heldi)->hid, tmcNil);

					Assert(rl.pvar == Global(pvarTmpRek));
					hprek = HprekOfRl(rl);
					hprek->hcab = ElGlobal(hcabSave);
					hprek->ielfdButton = ielfdNil;

				/* If there are any dialog-item arguments, add
					* their values to the record now.
					*/
					if (cevArgs > cevPositional)
						{
						MakeRecordPvar(rl.pvar, heldi, &ele,
								cevArgs - cevPositional,
								eviFirstArg +
								EviOfIev(cevPositional));
						}
					}
				else
					{
					int cmd;
				/* A dialog record is now available (either
					* given in the BASIC program, or constructed
					* via MakeRecordPvar().
					*/
					cmb.pv = pvNil;
					cmb.cmm = cmmAction | cmmCheck;
					if (fSuper)
						cmb.cmm |= cmmSuper;
					cmb.hcab = hcabArg;
					cmb.bcm = BcmFromIbcm(elx);

					Debug(vfInCmd++);
					vrerr = rerrNil;
					cmd = (*(int (*)())ele.pfn)(&cmb);
					Debug(vfInCmd--);

					if (cmd != cmdOK)
						{
						SetAgain(bcmNil);
						RtError(vrerr == rerrNil ?
								rerrCommandFailed : vrerr);
						}
						
					ElSetAgainCab(cmb.bcm, cmb.hcab);

					if (hpvalReturn != hpNil)
						{
					/* Set up the temporary record variable
						* to contain the record, and return it.
						*/
						struct VAR huge *hpvarTmpRek;

						((RL huge *)hpvalReturn)->pvar
								= Global(pvarTmpRek);
						((RL huge *)hpvalReturn)->ib = ibNil;

						hpvarTmpRek =
								HpvarOfPvar(Global(pvarTmpRek));
						hpvarTmpRek->rek.hcab =
								ElGlobal(hcabSave);
						hpvarTmpRek->rek.ielfdButton =
								IelfdFrTmc(ElGlobal(tmcSave),
								heldi);

						return ElvFromIeldi(ele.ieldi);
						}
					else
						return elvNil;
					}
				break;
				}

		case elrVoid:
			if (fElElx)
				(*(void (*)())ele.pfn)();
			else
				{
				int cmd;

				BLTBCX(0, (char far *) &cmb, sizeof (CMB));
				cmb.bcm = BcmFromIbcm(elx);
				cmb.kc = kcNil;
				cmb.cmm = cmmAction;
				Debug(vfInCmd++);
				vrerr = rerrNil;
				cmd = (*(int (*)())ele.pfn)(&cmb);
				Debug(vfInCmd--);
				if (cmd != cmdOK)
					{
					SetAgain(bcmNil);
					RtError(vrerr == rerrNil ?
							rerrCommandFailed : vrerr);
					}
				ElSetAgainCab(cmb.bcm, cmb.hcab);
				}
			break;

		case elrNum:
				{
				if (SbOfHp(hpvalReturn) == 1)
					{
					*(NUM *)IbOfHp(hpvalReturn) =
							(*(NUM (*)())ele.pfn)();
					}
				else
					{
					NUM numT;

					numT = (*(NUM (*)())ele.pfn)();
					BLTBH((NUM huge *)&numT, hpvalReturn, cbNUM);
					}
				break;
				}
		case elrInt:
		case elrSd:
			*(WORD huge *)hpvalReturn = (*(WORD (*)())ele.pfn)();
			if (elr == elrSd && 
					*((SD huge *) hpvalReturn) == sdNil)
				{
				RtError(rerrOutOfMemory);
				}
			break;
			}

		/* If we created any local-heap-strings, free them now.
			*/
		if (fFreeHeapStrings)
			{
			for (ielp = 0; ielp < cevArgs; ielp++)
				{
				if (ele.rgelp[ielp] == elpHst)
					{
					struct EV huge *hpevT;

					hpevT = HpevOfEvi(eviFirstArg + EviOfIev(ielp));
					Assert(hpevT->elv == elvSd);
					FreePpv(sbDds, hpevT->hst);
					}
				}
			}
		}	/* for (fDoCall) */

	return mpelrelv[elr];
}


/* %%Function:CevParseList %%Owner:bradch */
int CevParseList(peviFirst, plibTerminator)
/* An interface procedure between EXEC and EXP.  Parses a list of expressions
* separated by commas.  If plibTerminator != 0, the LIB of the terminating
* token is stored there.
*/
EVI *peviFirst;
LIB *plibTerminator;
{
	struct EV huge * hpev;
	int cev;
	int fExprOk = FALSE;

	/* The stack had better be empty, since everything on the stack after
		* the parse will be assumed to be part of the expression.
		*/
	Assert(ElGlobal(ievStackPtr) == Global(ievStackBase));

	while (1)
		{
		if (plibTerminator != 0)
			*plibTerminator = LibCur();
		if (!FExpIn(Global(eltCur)))
			break;
		fExprOk = TRUE;
		EltGetCur();
		}

	if (!fExprOk)		/* didn't accept any tokens? */
		{
		*peviFirst = eviNil;
		return 0;
		}

	if (!FExpIn(eltEndExp))
		Assert(FALSE);

	cev = ElGlobal(ievStackPtr) - Global(ievStackBase);
	if (cev == 1 &&
			(hpev = HpevOfEvi(EviOfIev(ElGlobal(ievStackPtr))))->elv == elvNil)
		{
		/* Function with no parens... */
		if (hpev->fElx && !hpev->fResolved /*RgeltStack()[ElGlobal(ieltStackPtr)] == eltElx*/)
			ElSyntaxError(0);

		/* There's a single null expression on the EV stack.  This
			* is how an empty expression list is represented.
			*/
		cev = 0;		/* ignore it */
		EviPop();
		}
	*peviFirst = EviOfIev(Global(ievStackBase) + 1);

	/* What's on the ELT stack will be eltLParen, followed by a lot of
		* commas.  Delete it all ...
		*/
	ElGlobal(ieltStackPtr) = Global(ieltStackBase);

	return cev;
}


/* %%Function:CallCmd %%Owner:bradch */
CallCmd(cmm)
int cmm;
{
	struct VAR huge * hpvar;
	CMB cmb;
	HCAB hcab;
	ELX elx;

	if (EltGetCur() != eltIdent)
		{
		/* Error: identifier expected */
		RtError(rerrIdentifierExpected);
		}

	hpvar = HpvarOfPvar(PvarFromPsy(Global(psyCur), 
			ElvFromIeldi(ieldiNil), FALSE));

	if (!FRecordElv(hpvar->elv))
		{
		/* Error: dialog record variable expected */
		RtError(rerrDlgRecVarExpected);
		}

	cmb.bcm = BcmFromIbcm(hpvar->elx);
	if ((cmb.hcab = hpvar->rek.hcab) == 0)
		{
		ELDI huge * hpeldi;
		hpeldi = HpeldiFromHeldi(HeldiFromIeldi(IeldiFromElx(hpvar->elx)));
		if ((cmb.hcab = hpvar->rek.hcab = HcabAlloc(hpeldi->cabi)) == 0)
			RtError(rerrOutOfMemory);
		hpvar->rek.ielfdButton = ielfdNil; /* since we are bypassing
		InitHprek call */
		}

	cmb.cmm = cmm;
	cmb.kc = kcNil;
	cmb.pv = pvNil;
	cmb.tmc = tmcOK;

#ifdef REMOVE /* No longer necessary! */
	if (cmb.fDialog && !FCheckCab(&cmb))
		RtError(rerrIllegalFunctionCall);
#endif

	if (CmdExecCmb(&cmb))
		{
		/* Error: command failed */
		RtError(rerrCommandFailed);
		}
	
	hpvar = HpvarOfPvar(PvarFromPsy(Global(psyCur), 
		ElvFromIeldi(ieldiNil), FALSE));
	hpvar->rek.ielfdButton = IelfdFrTmc(cmb.tmc, 
		HeldiFromIeldi(IeldiFromElx(hpvar->elx)));

	EltGetCur();
}




/* ---------------------------------------------------------------- */
/* --------------------------- STRINGS ---------------------------- */
/* ---------------------------------------------------------------- */

/* %%Function:SdFromCch %%Owner:bradch */
SD SdFromCch(cch)
unsigned cch;
{
	SD sd;

	if (cch + 2 < cch)
		RtError(rerrStringTooBig);

	if ((sd = PpvAllocCbWW(sbStrings, cch + 2)) == 0)
		return sdNil;

	CchFromSd(sd) = cch;

#ifdef DEBUG
	ElGlobal(csdAllocated)++;
	if (fToArrays)
		{
		PrintT("ADSD: create SD %u", sd);
		PrintT(" (cch = %d)\n", cch);
		}
#endif
	return sd;
}


/* %%Function:FReallocSd %%Owner:bradch */
FReallocSd(sd, cchNew)
SD sd;
unsigned cchNew;
{
	Assert(sd != 0);

	if (cchNew + 2 < cchNew) /* check for string too long */
		RtError(rerrStringTooBig);

	if (!FReallocPpv(sbStrings, sd, cchNew + 2))
		return FALSE;

	CchFromSd(sd) = cchNew;

	return TRUE;
}


/* %%Function:ReallocSd %%Owner:bradch */
ReallocSd(sd, cchNew)
SD sd;
unsigned cchNew;
{
	if (!FReallocSd(sd, cchNew))
		RtError(rerrOutOfMemory);
}


/* %%Function:SdCopySz %%Owner:bradch */
SD SdCopySz(sz)
char *sz;
{
	unsigned cch;
	char *pchT, **h;

	if (*sz == '\0')
		return 0;

	pchT = sz;
	while (*pchT != '\0')
		pchT++;
	cch = pchT - sz;

	if (cch + 2 < cch)
		RtError(rerrStringTooBig);

	if ((h = PpvAllocCbWW(sbStrings, cch + 2)) == 0)
		return sdNil;

	CchFromSd(h) = cch;
	BLTBH((char huge *)sz, HpchFromSd(h), cch);

#ifdef DEBUG
	ElGlobal(csdAllocated)++;
	if (fToArrays)
		{
		PrintT("ADSD: create SD %u", pchT);
		PrintT(" (cch = %d)\n", cch);
		}
#endif
	return h;
}


/* %%Function:SdCopySt %%Owner:bradch */
SD SdCopySt(st)
char *st;
{
	unsigned cch;
	char **h;

	cch = st[0];

	if (cch == 0)
		return 0;

	if ((h = PpvAllocCbWW(sbStrings, cch + 2)) == 0)
		return sdNil;

	CchFromSd(h) = cch;
	BLTBH((char huge *)st + 1, HpchFromSd(h), cch);

#ifdef DEBUG
	ElGlobal(csdAllocated)++;
	if (fToArrays)
		{
		PrintT("ADSD: create SD %u", h);
		PrintT(" (cch = %d)\n", cch);
		}
#endif

	return h;
}


/* %%Function:SdCopyLpchCch %%Owner:bradch */
SD SdCopyLpchCch(lpch, cch)
/* Another way of creating an SD.  If the cch is -1, the string is assumed
* to be zero-terminated.
*/
char far *lpch;
unsigned cch;
{
	char ** h;

	if (cch == -1)
		{	/* -- count length of sz --- */
		for (cch = 0; lpch[cch] != '\0'; cch++)
			;
		}

	if (cch == 0)
		return 0;

	if (cch + 2 < cch)
		RtError(rerrStringTooBig);

	if ((h = PpvAllocCbWW(sbStrings, cch + 2)) == 0)
		return sdNil;

	CchFromSd(h) = cch;
	BLTBX(lpch, LpOfHp(HpchFromSd(h)), cch);

#ifdef DEBUG
	ElGlobal(csdAllocated)++;
	if (fToArrays)
		{
		PrintT("ADSD: create SD %u", h);
		PrintT(" (cch = %d) from lpch\n", cch);
		}
#endif
	return h;
}


/* %%Function:SdDupSd %%Owner:bradch */
SD SdDupSd(sd)
SD sd;
{
	char **h;

#ifdef DEBUG
	if (sd != 0)
		ElGlobal(csdAllocated)++;
#endif
	if (sd == 0)
		return 0;

	if ((h = PpvAllocCbWW(sbStrings, CchFromSd(sd) + 2)) == 0)
		return sdNil;

	BLTBH(Hs16FromSd(sd), Hs16FromSd(h), CchFromSd(sd) + 2);

#ifdef DEBUG
	if (fToArrays)
		{
		PrintT("ADSD: SdDupSd(%d)", sd);
		PrintT(" = %u", h);
		PrintT(" (cch = %d)\n", h == 0 ? 0 : CchFromSd(h));
		}
#endif

	return h;
}


/* %%Function:AppendStToSd %%Owner:bradch */
AppendStToSd(st, sd)
char * st;
SD sd;
{
	unsigned cchSt, cchSd;
	char huge * hpch;

	if ((cchSt = *st) == 0)
		return;

	cchSd = CchFromSd(sd);

	if (cchSd + cchSt + 2 < cchSd)
		RtError(rerrStringTooBig);

	ReallocSd(sd, cchSd + cchSt);
	BLTBH((char huge *) st + 1, HpchFromSd(sd) + cchSd, cchSt);
}


/* %%Function:AppendSdToSd %%Owner:bradch */
AppendSdToSd(sd1, sd2)
SD sd1, sd2;
{
	unsigned cch1, cch2, cchTotal;

	if ((cch1 = CchFromSd(sd1)) == 0)
		return;

	cch2 = CchFromSd(sd2);

	cchTotal = cch1 + cch2;
	if (cchTotal < cch1 || cchTotal < cch2)
		RtError(rerrStringTooBig);

	ReallocSd(sd2, cchTotal);
	BLTBH(HpchFromSd(sd1), HpchFromSd(sd2) + cch2, cch1);
}


/* %%Function:SdConcat %%Owner:bradch */
SD SdConcat(sd1, sd2)
SD sd1, sd2;
{
	unsigned cch1, cch2, cchTotal;
	char ** sdResult;

	cch1 = (sd1 == 0) ? 0 : CchFromSd(sd1);
	cch2 = (sd2 == 0) ? 0 : CchFromSd(sd2);

	cchTotal = cch1 + cch2;
	if (cchTotal < cch1 || cchTotal < cch2)
		RtError(rerrStringTooBig);

	if (cchTotal == 0)
		return (SD) 0;
	if ((sdResult = PpvAllocCbWW(sbStrings, cch1 + cch2 + 2)) == 0)
		RtError(rerrOutOfMemory);

	CchFromSd(sdResult) = cchTotal;

	if (cch1 != 0)
		BLTBH(HpchFromSd(sd1), HpchFromSd(sdResult), cch1);
	if (cch2 != 0)
		BLTBH(HpchFromSd(sd2), HpchFromSd(sdResult) + cch1, cch2);

#ifdef DEBUG
	ElGlobal(csdAllocated)++;
	if (fToArrays)
		{
		PrintT("ADSD: (concat) create SD %u", sdResult);
		PrintT(" (cch = %d)\n", cch1 + cch2);
		}
#endif

	return sdResult;
}


/* %%Function:FreeSd %%Owner:bradch */
FreeSd(sd)
SD sd;
{
	if (sd != 0 && sd != sdNil)
		{
#ifdef DEBUG
		ElGlobal(csdAllocated)--;
		if (fToArrays)
			PrintT("ADSD: free SD %u\n", sd);
#endif
		FreePpv(sbStrings, sd);
		}
}


/* %%Function:HstzOfSd %%Owner:bradch */
HstzOfSd(sd)
/* Creates and returns a handle to a counted string on the application heap.
*/
SD sd;
{
	unsigned cch = CchFromSd(sd);
	char **h;

	if (cch >= 256)
		RtError(rerrStringTooBig);

	if ((h = PpvAllocCbWW(sbDds, cch + 2)) == 0)
		RtError(rerrOutOfMemory);

	**h = cch;
	BLTBH(HpchFromSd(sd), (char huge *)(*h + 1), cch);
	(*h)[cch + 1] = '\0';

	return h;
}


/* %%Function:HszOfSd %%Owner:bradch */
char ** HszOfSd(sd)
/* Creates and returns a handle to a counted string on the application heap.
*/
SD sd;
{
	unsigned cch = CchFromSd(sd);
	char **h;

	if ((h = PpvAllocCbWW(sbDds, cch + 1)) == 0)
		RtError(rerrOutOfMemory);

	BLTBH(HpchFromSd(sd), (char huge *) *h, cch);
	(*h)[cch] = '\0';

	return h;
}


/* %%Function:WCmpSdSd %%Owner:bradch */
WCmpSdSd(sd1, sd2)
/* String compare.  Returns:
* positive	if sd1 > sd2
* zero		if sd1 = sd2
* negative	if sd1 < sd2
*/
SD sd1, sd2;
{
	unsigned char huge *hpch1, huge *hpch2;
	unsigned cch1, cch2;

	if (sd1 == sd2)
		return 0;
	if (sd1 == 0)
		return -1;
	else  if (sd2 == 0)
		return 1;

	cch1 = CchFromSd(sd1);
	hpch1 = HpchFromSd(sd1);
	cch2 = CchFromSd(sd2);
	hpch2 = HpchFromSd(sd2);

	/* Loop while both strings have another character left.
		*/
	while (cch1 != 0 && cch2 != 0)
		{
		int wDiff;

		if ((wDiff = WCompChCh(*hpch1++, *hpch2++)) != 0)
			return wDiff;
		cch1--;
		cch2--;
		}
	return cch1 - cch2;
}


/* ---------------------------------------------------------------- */
/* --------------------------- ARRAYS ----------------------------- */
/* ---------------------------------------------------------------- */

/* %%Function:AdCreate %%Owner:bradch */
AD AdCreate(cbElement, cas, rgas)
int cbElement, cas;
AS *rgas;
{
	int ias;
	long cbData = cbElement;
	AD adNew;
	struct AHR huge *hpahr;

	Assert(cas > 0);
	for (ias = 0; ias < cas; ias++)
		{
		cbData *= rgas[ias];
		if (cbData > 0xffffL)
			RtError(rerrOutOfMemory);
		}

	if ((adNew = PpvAllocCbWW(sbArrays, CbOfAhr(cas, (int) cbData))) == 0)
		RtError(rerrOutOfMemory);

	hpahr = HpahrOfAd(adNew);

	hpahr->cas = cas;
	hpahr->cbElement = cbElement;
	for (ias = 0; ias < cas; ias++)
		hpahr->rgas[ias] = rgas[ias];
	BLTBCX(0, LpOfHp(HpOfSbIb(sbArrays, PvalOfPahr(PahrOfAd(adNew), cas))),
			(int) cbData);

#ifdef DEBUG
	if (fToArrays)
		{
		PrintT("ARRAYS: created AD %u(", adNew);
		for (ias = 0; ias < cas; ias++)
			{
			if (ias == cas - 1)
				PrintT("%d", rgas[ias]);
			else
				PrintT("%d, ", rgas[ias]);
			}
		PrintT(")\n", 0);
		}
#endif

#ifdef DEBUG
	ElGlobal(cadAllocated)++;
#endif
	return adNew;
}


/* %%Function:FreeAd %%Owner:bradch */
FreeAd(ad)
{
	if (ad != 0)
		{
#ifdef DEBUG
		ElGlobal(cadAllocated)--;
#endif
		FreePpv(sbArrays, ad);
		}
}


/* %%Function:FreeStringsAd %%Owner:bradch */
FreeStringsAd(ad)
/* Frees all the SD's in an array of SD's.
*/
AD ad;
{
	struct AHR huge *hpahr = HpahrOfAd(ad);
	int csd = 0;
	int ias, isd;
	AD huge *hrgsd;

	if (ad == 0)
		return;

	csd = 1;
	for (ias = 0; ias < hpahr->cas; ias++)
		csd *= hpahr->rgas[ias];

	hrgsd = HpOfSbIb(sbArrays, PvalOfPahr(PahrOfAd(ad), hpahr->cas));
	for (isd = 0; isd < csd; isd++)
		{
		SD sd = hrgsd[isd];

		if (sd != 0)
			FreeSd(sd);
		}
}


#ifdef DEBUG
/* %%Function:PrintTSd %%Owner:bradch */
PrintTSd(sd)
{
	PrintT("\"", 0);
	if (sd != 0)
		PrintHs16(Hs16FromSd(sd));
	PrintT("\"", 0);
}


#endif


/* %%Function:SetupCall %%Owner:bradch */
SetupCall(psy, cevArgs, eviFirstArg, libReturn)
/* Calls the SUB or FUNCTION named psy, with parameters pevParams.
* If hpvalReturn == hpNil, we are calling a SUB; otherwise a FUNCTION.
*/
struct SY *psy;
int cevArgs;
EVI eviFirstArg;
LIB libReturn;
{
	struct PROC *pproc, huge *hpproc;
	int iev;
	int fNewProc;
	struct VAR *pvarTempSave = 0;
	struct VAR *rgpvarRef[cargMaxProc];
	LIB libCall;

#ifdef DEBUG
	if (fToExec)
		{
		PrintT("EXEC: SetupCall() with %d args", cevArgs);
		PrintT(" starting at evi = %u\n", eviFirstArg);
		}
#endif
	if (libReturn == libNil)
		libReturn = LibCur();

	libCall = LibCur();
	pproc = PprocFindPsy(psy);
	Assert(pproc != 0);
	hpproc = HpOfSbIb(Global(sbNtab), pproc);

	if (cevArgs != hpproc->carg)
		{
		/* Error: Wrong number of arguments (cevArgs) to SUB or FUNCTION */
		RtError(rerrArgCountMismatch);
		}

	if (cevArgs == 1 && hpproc->elv == elvNil && 
			!Global(fImplicitParens) && !Global(fCall))
		{
		/* The call is of the form:
			*	SUBNAME (ARG)
			* i.e. the argument should be passed by value.
			*/
		ResolveEvi(eviFirstArg, elvNil, hpNil);
		}

	/* Set up the arguments (creating temporary VAR's when necessary).
		* We end up with rgpvarRef containing the pvar's to reference.
		*/
	for (iev = 0; iev < cevArgs; iev++)
		{
		ELV elv;
		BOOL fArray;
		struct EV huge *hpev;
		struct VAR *pvarRef;
		EVI evi;

		elv = hpproc->rgarg[iev].elv;	/* required argument type */
		fArray = hpproc->rgarg[iev].fArray;
		hpev = HpevOfEvi(evi = (eviFirstArg + EviOfIev(iev)));
		if (hpev->fResolved)
			{
			if (pvarTempSave == 0)
				pvarTempSave = PvarBeginTemps();

			/* Create a temporary variable and assign the
				* argument value to it.
				*/
			pvarRef = PvarFromPsy(psyNil, elv, fArray);
			ExecGets(pvarRef, evi);
			}
		else
			{
			struct VAR huge *hpvar;

			if (hpev->fElx)
				ElSyntaxError(0);

			Assert(hpev->psy != psyNil);
			pvarRef = PvarFromPsy(hpev->psy, elvNil, fArray);
			if ((hpvar = HpvarOfPvar(pvarRef))->elv == elvFormal)
				{
				/* Dereference formal parameter.
					*/
				hpvar = HpvarOfPvar(pvarRef = hpvar->pvar);
				}

			if (fArray ? hpvar->elv != elvArray &&
					hpvar->ai.elv != elv
					: hpvar->elv != elv)
				{
				/* Error: Wrong argument type */
				RtError(rerrTypeMismatch);
				}
			}
		rgpvarRef[iev] = pvarRef;
		}

	/* We now have (in rgpvarRef) a list of the variables which should be
		* passed by reference to the procedure.
		*/
	BeginPproc(pproc);
	for (iev = 0; iev < cevArgs; iev++)
		{
		struct VAR huge *hpvar;

		hpvar = HpvarOfPvar(PvarFromPsy(hpproc->rgarg[iev].psy,
				elvFormal, FALSE));
		hpvar->pvar = rgpvarRef[iev];
#ifdef DEBUG
		if (fToVar)
			{
			OutExecGets(IbOfHp(hpvar),
					HpvarOfPvar(rgpvarRef[iev])->elv,
					&HpvarOfPvar(rgpvarRef[iev])->num);
			}
#endif
		}

#ifdef DEBUG
	if (fToExec)
		OutExecCall();
#endif

	HpcsrOfIcsr(IcsrPushCst(cstEndProc, libReturn))->pvarEndProc
			= pvarTempSave;
	SetCurLib(hpproc->lib);
}


/* %%Function:ExecEndProc %%Owner:bradch */
ExecEndProc()
	/* Handles the return from a SUB or FUNCTION.
	*/
{
	CST cst;
	int icsr;
	struct CSR huge *hpcsr;

	switch ((hpcsr = HpcsrOfIcsr(ElGlobal(icsrStackPtr)))->cst)
		{
#ifdef NOTUSEFUL
	case cstBlockElse:
	case cstBlockEndif:
		break;

	case cstNext:
	case cstWend:
		RtError(rerrMissingNextOrWend);
#endif

	case cstCase:
		SetCurLib(hpcsr->libStart);
		HpeliFirst()->lib = hpcsr->libStart - 1;
		RtError(rerrMissingEndSelect);
		}

	hpcsr = HpcsrOfIcsr(icsr = IcsrPop(cstEndProc));	/* pop CSR */
	EndProcedure();				/* destroy procedure's frame */
	if (hpcsr->pvarEndProc != 0)		/* delete temps? */
		EndTemps(hpcsr->pvarEndProc);	/* yes */
	SetCurLib(hpcsr->libStart);	/* go back to where we called from */
	HpeliFirst()->lib = hpcsr->libStart - 1;
	Global(eltCur) = eltNil;
}


/* %%Function:ExecGets %%Owner:bradch */
ExecGets(pvar, eviValue)
/* Assigns eviValue to the scalar variable at pvar.
*/
struct VAR *pvar;
EVI eviValue;
{
	struct VAR huge *hpvar = HpvarOfPvar(pvar);
	struct EV huge *hpevValue;
	int elv = hpvar->elv;
	
	Assert(eviValue != eviNil);

	/* If a string assignment, free the old value of the variable.
		*/
	if (elv == elvSd)
		{
		FreeSd(hpvar->sd);
		hpvar->sd = 0;
		}
	else  if (FRecordElv(elv))
		FreeRek(hpvar->rek);

	ResolveEvi(eviValue, elv, &hpvar->num);

	/* If this is a record assignment, convert the pvar to the actual REK.
		*/
	hpevValue = HpevOfEvi(eviValue);
	if (FRecordElv(elv))
		{
		/* Record assignment -- convert the RL to the actual REK.
			*/
		if (hpevValue->rl.pvar == Global(pvarTmpRek))
			{
			/* The REK was the temporary one, so there's no need
				* to copy it.
				*/
			struct VAR huge *hpvarT;

			hpvarT = HpvarOfPvar(Global(pvarTmpRek));
			hpvar->rek = hpvarT->rek;
			hpvarT->rek.hcab = 0;		/* use it up */
			}
		else
			{
			/* Assignment from one record variable to another --
				* we have to copy the record.
				*/
			hpvar->rek = *HprekOfRl(hpevValue->rl);
			if (hpvar->rek.hcab == 0)
				{
				hpvar->rek.hcab = 
						HcabDupeCab(hpvar->rek.hcab);
				}
			}
		}
	else  if (hpevValue->elv == elvSd)
		{
		/* String assignment -- make sure the resulting string can be
			* freed.
			*/
		if (!hpevValue->fFree)
			{
			if ((hpvar->sd = SdDupSd(hpevValue->sd)) == sdNil)
				RtError(rerrOutOfMemory);
			}
		else
			hpevValue->fFree = FALSE;	/* preserve it */
		}

#ifdef DEBUG
	if (fToVar)
		OutExecGets(pvar, elv, &hpvar->num);
#endif
}
