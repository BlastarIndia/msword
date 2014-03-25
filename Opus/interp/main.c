/* misc. OpEL functions */

#include <qwindows.h>
#include <qsetjmp.h>
#include <el.h>
#include <uops.h>
#include "eltools.h"
#include <rerr.h>
#include "ibcm.h"

#define EXTERN	extern	/* disable global defs */
#include "priv.h"
#include "sym.h"
#include "exp.h"
#include "_exec.h"

#define pvNil	((void *) 0)

extern RERR vrerr;

unsigned vibLineStart = 0;
int vcElParams;
BOOL vfElFunc;

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





/* %%Function:DefProcedure %%Owner:bradch */
DefProcedure()
	/* Reads a SUB or FUNCTION definition line.  If we are seeing this code for
	* the first time, defines the SUB or FUNCTION.
	*
* On exit, Global(eltCur) is terminator of the SUB/FUNCTION line.
	*/
{
	long libProc;
	ELT eltProc, eltCur;
	int carg;
	struct SY *psyProc;
	struct PROC *pproc, huge *hpproc;
	struct ARG rgarg[cargMaxProc];

	Assert(Global(eltCur) == eltSub || Global(eltCur) == eltFunction);
	eltProc = Global(eltCur);

	if ((eltCur = EltGetCur()) != eltIdent)
		{
		/*HpeliFirst()->lib = LibCur();*/
		/* Error: Illegal procedure definition/1 */
		RtError(rerrSyntaxError);
		}
	psyProc = Global(psyCur);
	
	libProc = LibCur();
	
	switch (EltGetCur())
		{
	default:
		/* Error: Illegal procedure definition/2 */
		RtError(rerrSyntaxError);
	case eltColon:
	case eltLineIdent:
	case eltLineNolabel:
	case eltLineInt:
		/* NYI: If label definition, define the label properly ...
			*/
		/* Procedure name not followed by parentheses, so there are
			* no arguments.
			*/
		carg = 0;
		break;

	case eltLParen:
		/* Read the argument list.
			*/
		carg = 0;
		while (1)
			{
			int elvArg;
			int fArray = FALSE;

			if ((eltCur = EltGetCur()) != eltIdent)
				{
				/* Error: Illegal procedure definition/3 */
				RtError(rerrSyntaxError);
				}
			
			if (carg >= cargMaxProc)
				{
				/* Error: Only cargMaxProc arguments allowed */
				RtError(rerrArgCountMismatch);
				}
			
			rgarg[carg].psy = Global(psyCur);

			elvArg = HpsyOfPsy(Global(psyCur))->elv;
			if ((eltCur = EltGetCur()) == eltLParen)
				{
				if (EltGetCur() != eltRParen)
					ElSyntaxError(0);
				rgarg[carg].fArray = TRUE;
				eltCur = EltGetCur();
				}
			else
				rgarg[carg].fArray = FALSE;

			if (eltCur == eltAs)
				{
				if (EltGetCur() != eltElx)
					{
					/* Error: Invalid use of AS */
					RtError(rerrSyntaxError);
					}
				
				elvArg = ElvFromIeldi(IeldiFromElx(PelcrCur()->elx));
				eltCur = EltGetCur();
				}
			
			rgarg[carg++].elv = elvArg;

			if (eltCur == eltRParen)
				break;
			
			if (eltCur != eltComma)
				{
				/* Error: Illegal dummy argument */
				RtError(rerrTypeMismatch);
				}
			}

		libProc = LibCur();
		break;
	/* end switch (EltGetCur()) */
		}

	hpproc = HpOfSbIb(Global(sbNtab), pproc = PprocFromPsy(psyProc, carg));

	if (hpproc->lib != libNil)		/* already defined? */
		{
		/* Error: SUB or FUNCTION defined twice */
		RtError(rerrDuplicateLabel);
		}
	hpproc->lib = libProc;	/* PROC is now defined */

	/* Copy the arg list to the new PROC, and set the return value.
		*/
	BLTBX(LpOfHp(HpOfSbIb(1, &rgarg)), LpOfHp(hpproc->rgarg),
			carg * sizeof(struct ARG));
	hpproc->elv = (eltProc == eltFunction ? HpsyOfPsy(psyProc)->elv
							: elvNil);

	*HpNtab(pprocScan) = pproc;	/* remember what PROC we're in */
	*(LIB huge *)HpNtab(libScan) = libProc;

	EltGetCur();	/* pre-fetch (defining any labels in new proc) */
}


		



#ifdef NOTUSED
/* %%Function:TrappableError %%Owner:NOTUSED */
TrappableError(rerr)
/* Triggers an ON ERROR jump if one is defined; otherwise halts the EL
* invocation.
*/
RERR rerr;
{
	ENV *penvFrame;

	/* Jump to the error handler if there is one defined, and the ERR
		* variable has not already been set (indicating processing of a
		* previous error).
		*/
	if ((penvFrame = PenvGetCur(FALSE)) != 0 && FSetErrRerr(rerr))
		{
		ENV env;

#ifdef DEBUG
		if (fToExec)
			PrintT("EXEC: found an error handler\n", 0);
#endif
		BLTBH(HpOfSbIb(sbFrame, penvFrame), (char huge *)&env,
				sizeof(ENV));
		DoJmp(&env, 1);
		/*NOTREACHED*/
		}
	else
		{
#ifdef DEBUG
		if (fToExec)
			PrintT("EXEC: no error handler\n", 0);
#endif
		StopHeli(ElGlobal(heliFirst), rerr);
		/*NOTREACHED*/
		}
}


#endif /* NOT USED */




/* %%Function:FExecOnError %%Owner:bradch */
BOOL FExecOnError(pplab)
/* Parses an ON ERROR statement.  Returns TRUE to enable the error handler, or
* FALSE to disable it.
*
* If TRUE is returned, *pplab is the label to jump to when an error occurs.
* If *pplab is 0, the statement was "ON ERROR RESUME NEXT".
*/
struct LAB **pplab;
{
	BOOL fEnable;

	if (EltGetCur() == eltGoto)
		{
		Global(fExpectLabel) = TRUE;
		switch (EltGetCur())
			{
		default:
			ElSYntaxError(0);

		case eltIdent:
			*pplab = PlabFromPsy(Global(psyCur), 0,
							Global(pprocCur));
			fEnable = TRUE;
			break;
		case eltInt:
			if (PelcrCur()->wInt == 0)
				fEnable = FALSE;
			else
				{
				*pplab = PlabFromPsy((struct SY *)0,
								PelcrCur()->wInt,
								Global(pprocCur));
				fEnable = TRUE;
				}
			break;
			}
		Global(fExpectLabel) = FALSE;
		}
	else if (Global(eltCur) == eltResume)
		{
		if (EltGetCur() != eltNext)
			{
			/* Error: Expected RESUME NEXT */
			RtError(rerrNoResumeNext);
			}
		*pplab = 0;
		fEnable = TRUE;
		}

	EltGetCur();		/* pre-fetch */
	return fEnable;
}


/* %%Function:HpevParseExp %%Owner:bradch */
struct EV huge * HpevParseExp()
{
	ELT elt;
	EVI eviFirst;
	struct EV huge * hpev;
	BOOL fExprOk = FALSE;
	BOOL fRet = TRUE;
	int cParen = 0;

	/* The stack had better be empty, since everything on the stack after
		* the parse will be assumed to be part of the expression.
		*/
	Assert(ElGlobal(ievStackPtr) == Global(ievStackBase));

	while ((elt = Global(eltCur)) != eltComma || cParen != 0)
		{
		switch (elt)
			{
		case eltLParen:
			cParen += 1;
			break;
			
		case eltRParen:
			cParen -= 1;
			break;
			}
		if (!FExpIn(elt))
			break;
		fExprOk = TRUE;
		if (elt == Global(eltCur))
			EltGetCur();
		}

	if (!fExprOk)		/* didn't accept any tokens? */
		return hpNil;

	if (!FExpIn(eltEndExp))
		Assert(FALSE);

	if (ElGlobal(ievStackPtr) - Global(ievStackBase) == 1 &&
		HpevOfEvi(EviOfIev(ElGlobal(ievStackPtr)))->elv == elvNil)
		{
		/* There's a single null expression on the EV stack.  This
			* is how an empty expression list is represented.
			*/
		EviPop();
		fRet = FALSE;
		}
	eviFirst = EviOfIev(Global(ievStackBase) + 1);

	/* What's on the ELT stack will be eltLParen, followed by a lot of
		* commas.  Delete it all ...
		*/
	ElGlobal(ieltStackPtr) = Global(ieltStackBase);
	
	if (!fRet)
		return hpNil;

	hpev = HpevOfEvi(eviFirst);
	ResolveEvi(eviFirst, elvNil, hpNil);
	
	return hpev;
}


/* %%Function:DoElPrint %%Owner:bradch */
DoElPrint(elt)
ELT elt;
{
	SD sdPrint;
	BOOL fNL;
	int stm, iev;
	struct EV huge * hpev;
	int ievSave;
	BOOL fWrite;
	char stBuf [256];
	
	Assert(elt == eltPrint || elt == eltWrite);
	fWrite = elt == eltWrite;
	
	stm = 0;
	fNL = TRUE;

	EltGetCur();
	if (Global(eltCur) == eltHash)
		{
		/* pick out file number */
		ParseStm(&stm);

		if (Global(eltCur) != eltComma)
			{
			/* Error: comma expected */
			RtError(rerrCommaMissing);
			}
		EltGetCur();
		}

	if ((sdPrint = SdFromCch(0)) == sdNil)
		RtError(rerrOutOfMemory);
	
	{{ /* NATIVE - DoElPrint */
		for (;;)
		{
		switch (Global(eltCur))
			{
		default:
			ievSave = ElGlobal(ievStackPtr);

			if ((hpev = HpevParseExp()) == hpNil)
				{
				PopToIev(ievSave);
				ElSyntaxError(0);
				break;
				}
			
			switch (hpev->elv)
				{
			default:
				ElSyntaxError(-1);
				break;
						
			case elvNum:
				{
				NUM num;
				
				/* convert num to string */
				num = hpev->num;
				stBuf[0] = CchPnumToPch(&num, stBuf + 1);
				goto LAppendSt;
				}

			case elvInt:
				{
				/* convert int to string */
				char * pch = stBuf + 1;
				stBuf[0]=0;

				/* insert leading space for +ve nos */
				if (!fWrite && (long)hpev->w >= 0)
					{
					*pch++ = ' ';
					stBuf[0] = 1;
					}
					
				stBuf[0] += CchLongToPpch((long)hpev->w, &pch);
				}
LAppendSt:
				AppendStToSd(stBuf, sdPrint);
				break;
					
			case elvSd:
				/* append to working string */
				if (fWrite)
					{
					stBuf[0] = 1;
					stBuf[1] = '"';
					AppendStToSd(stBuf, sdPrint);
					}
				AppendSdToSd(hpev->sd, sdPrint);
				if (fWrite)
					AppendStToSd(stBuf, sdPrint);
				break;

			case eltElseif:
			case eltElse:
			case eltColon:
			case eltLineIdent:
			case eltLineInt:
			case eltLineNolabel:
			case eltEof:
					{{
					goto LOut;
					}} /* !NATIVE - DoElPrint */
				}
					
			PopToIev(ievSave);
			fNL = TRUE;
			break;
					
		case eltComma:
			if (!fWrite)
				{
				/* append tab to working string */
				stBuf[0] = 1;
				stBuf[1] = '\t';
				AppendStToSd(stBuf, sdPrint);
				}
			/* FALL THROUGH */
					
		case eltSemi:
			if (fWrite)
				{
				/* append comma to working string */
				stBuf[0] = 1;
				stBuf[1] = ',';
				AppendStToSd(stBuf, sdPrint);
				}
			
			EltGetCur();
			fNL = FALSE;
			break;
					
		case eltElseif:
		case eltElse:
		case eltColon:
		case eltLineIdent:
		case eltLineInt:
		case eltLineNolabel:
		case eltEof:
			{{goto LOut;}} /* !NATIVE - DoElPrint */
			}
		}
	}}

LOut:
	if (fWrite || fNL)
		{
		stBuf[0] = 2;
		stBuf[1] = '\r';
		stBuf[2] = '\n';
		AppendStToSd(stBuf, sdPrint);
		}
	
	ElPrint(stm, (SD huge *) &sdPrint);

	FreeSd(sdPrint);
}


/* %%Function:DoElClose %%Owner:bradch */
DoElClose()
{
	int stm;
	
	stm = 0;
	Assert((42 == 42) == 1);
	if (vcElParams = (!FTermElt(EltGetCur(), TRUE)))
		ParseStm(&stm);
	ElClose(stm);
}


/* %%Function:DoElOpen %%Owner:bradch */
DoElOpen()
{
	SD sd;
	int stm, fom;
	RERR rerr;

	sd = sdNil;
	
	EltGetCur();
	ParseValue(elvSd, (SD huge *) &sd, eltComma);
	
	rerr = rerrNil;

	if (Global(eltCur) == eltComma)
		{
		char chMode;
		
		/* OPEN "mode", [#]stm, "name" */
		if (CchFromSd(sd) != 1)
			{
			rerr = rerrBadFileMode;
			goto LReturn;
			}
		
		chMode = (char) AnsiUpper((long) *HpchFromSd(sd));
		
		switch (chMode)
			{
		default:
			rerr = rerrBadFileMode;
			goto LReturn;
		
		case 'O':
			fom = fomOutput;
			break;
			
		case 'I':
			fom = fomInput;
			break;
			
		case 'A':
			fom = fomAppend;
			break;
			}
		
		EltParseComma();
		ParseStm(&stm);
		EltParseComma();
		ParseValue(elvSd, (SD huge *) &sd, eltComma);
		}
	else if (Global(eltCur) == eltFor)
		{
		/* OPEN "name" FOR mode AS [#]stm */
		switch (EltGetCur())
			{
		default:
			rerr = rerrBadFileMode;
			goto LReturn;
		
		case eltOutput:
			fom = fomOutput;
			break;

		case eltInput:
			fom = fomInput;
			break;
			
		case eltAppend:
			fom = fomAppend;
			break;
			}
		
		if (EltGetCur() != eltAs)
			{
			rerr = rerrSyntaxError;
			goto LReturn;
			}
		
		EltGetCur();
		ParseStm(&stm);
		}
	else
		{
		/* Error: Bad syntax */
		rerr = rerrSyntaxError;
		goto LReturn;
		}
	
	ElOpen(fom, stm, sd);
	
LReturn:
	if (sd != sdNil)
		FreeSd(sd);
	
	if (rerr != rerrNil)
		RtError(rerr);
}


/* %%Function:ParseStm %%Owner:bradch */
ParseStm(pstm)
int * pstm;
{
	if (Global(eltCur) == eltHash)
		EltGetCur();
	ParseValue(elvInt, (int huge *) pstm, eltComma);
}


/* %%Function:EltParseComma %%Owner:bradch */
EltParseComma()
{
	if (Global(eltCur) != eltComma)
		{
		/* Error: Comma expected */
		RtError(rerrCommaMissing);
		/* NOT REACHED */
		Assert(FALSE);
		}
	return EltGetCur();
}


/* INPUT [ [[#]stm,] | ["prompt",] ] var {,var} */
/* LINE INPUT  [ [[#]stm,] | ["prompt",] ] string-var  */
/* READ [ [[#]stm,] var {,var} */

/* %%Function:DoElInput %%Owner:bradch */
DoElInput(fLine, elt)
BOOL fLine;
ELT elt;
{
	SD sdPrompt, sdInput;
	int stm;
	int ievSave, cev, iev;
	int ich, cch;
	EVI eviFirstArg;
	char huge * hpch, huge * hpchStart;
	int fQuest;
	void huge * hpval;
			
	sdPrompt = sdNil;
	stm = 0;
	fQuest = !fLine;
			
	EltGetCur();
	
	if (elt == eltRead && Global(eltCur) != eltHash)
		ElSyntaxError(0);
	
	switch (Global(eltCur))
		{
	case eltLParen:
		ElSyntaxError(0);
	
	case eltString:
		ParseValue(elvSd, (SD huge *) &sdPrompt, eltComma);
		if (Global(eltCur) != eltComma && Global(eltCur) != eltSemi)
			RtError(rerrSyntaxError);
		EltGetCur();
		break;
			
	case eltHash:
		/* file number */
		EltGetCur();
		ParseValue(elvInt, (int huge *) &stm, eltComma);
		EltParseComma();
		break;
		}

	ievSave = ElGlobal(ievStackPtr);
	cev = CevParseList(&eviFirstArg, (LIB *) 0);
	
	for (iev = 0; iev < cev; iev += 1)
		{
		struct EV huge * hpev;

		hpev = HpevOfEvi(eviFirstArg + EviOfIev(iev));
		if (hpev->fResolved && !hpev->fAggregate)
			goto LBadVar;
		}
	
	if (cev == 0 || 
		fLine && (cev != 1 || HpevOfEvi(eviFirstArg)->elv != elvSd))
		{
		PopToIev(ievSave);
LBadVar:
		/* Error: Bad variable(s) (INPUT) */
		RtError(rerrSyntaxError);
		/* NOT REACHED */
		Assert(FALSE);
		}
	
	if ((sdInput = SdFromCch(0)) == sdNil)
		RtError(rerrOutOfMemory);
	
	{
	char * stPrompt = "\002? ";
	
	if (sdPrompt == sdNil)
		{
		if ((sdPrompt = SdCopySt(stPrompt)) == sdNil)
			RtError(rerrOutOfMemory);
		}
	else if (fQuest)
		AppendStToSd(stPrompt, sdPrompt);
	}
	
	ElInput(stm, (SD huge *) &sdPrompt, (SD huge *) &sdInput);

	FreeSd(sdPrompt);
	
	/* parse the input string into the list of variables */
	cch = CchFromSd(sdInput);
	ich = 0;
	for (iev = 0; iev < cev; iev += 1)
		{
		struct VAR * pvar, huge * hpvar;
		struct EV huge * hpev;
		ELV elv;
	
		hpev = HpevOfEvi(eviFirstArg + EviOfIev(iev));
		
		if (hpev->fResolved)
			{
			struct AHR * pahr, huge * hpahr;
			VAL * pval;

					Assert(hpev->fAggregate);
					if (hpev->elvAggr != elvArray)
								RtError(rerrSyntaxError);			

			pahr = PahrOfAd(HpvarOfPvar(hpev->pvarAdAggr)->ai.ad);
			hpahr = HpahrOfPahr(pahr);
			pval = (char *) PvalOfPahr(pahr, hpahr->cas) +
				hpev->ibAggr;
			
			hpval = HpOfSbIb(sbArrays, pval);
			elv = hpev->elv;
			
			if (elv == elvSd)
				FreeSd(*((SD huge *) hpval));
			}
		else
			{
			pvar = PvarFromPsy(hpev->psy, elvNil, FALSE);
			if (HpvarOfPvar(pvar)->elv == elvFormal)
				pvar = HpvarOfPvar(pvar)->pvar;
			hpvar = HpvarOfPvar(pvar);
			hpval = &hpvar->num;
			elv = hpvar->elv;
			if (elv == elvSd && hpvar->sd != 0)
				{
				FreeSd(hpvar->sd);
				hpvar->sd = sdNil;
				}
			}

		switch (elv)
			{
		default:
			goto LBadVar;
		
		case elvNum:
			if (fLine)
				goto LBadVar;
			
			{
			unsigned ch, cchT;
			unsigned char * pch;
			NUM num;
			char szBuf [256];
			
			hpchStart = hpch = HpchFromSd(sdInput) + ich;
			while (ich++ < cch && *hpch++ != ',')
				;
			cchT = (int) (hpch - hpchStart);
			if (*(hpch - 1) == ',')
				cchT -= 1;
			BLTBH(hpchStart, (char huge *) szBuf, cchT);
			szBuf[cchT] = '\0';
			for (pch = szBuf; (ch = *pch) <= ' '; ++pch)
				;
			CchPackSzPnum(pch, &num);
			*((NUM huge *) hpval) = num;
			}
			break;
		
		case elvSd:
			if (fLine)
				{
				*((SD huge *) hpval) = sdInput;
				sdInput = sdNil;
				break;
				}
			
			if (elt == eltRead)
				{
				int ichStart;
				SD sdT;
				int cchItem;
				BOOL fQuoted;
				char chFind;
				
				hpch = HpchFromSd(sdInput) + ich;
				
				if (fQuoted = *hpch == '"')
					{
					ich += 1;
					hpch += 1;
					}

				hpchStart = hpch;
				ichStart = ich;
				
				chFind = fQuoted ? '"' : ',';
				while (ich < cch && *hpch != chFind)
					{
					ich += 1;
					hpch += 1;
					}

				cchItem = ich - ichStart;
				
				if (ich < cch && fQuoted && *hpch == '"')
					{
					ich += 1;
					hpch += 1;
					}
				
				if (ich < cch && *hpch == ',')
					ich += 1;

				if (cchItem > 0)
					{
					if ((sdT = SdFromCch(cchItem)) == sdNil)
						RtError(rerrOutOfMemory);
					BLTBH(HpchFromSd(sdInput) + ichStart, HpchFromSd(sdT), 
						cchItem);
					}
				else
					sdT = 0;
				
				*((SD huge *) hpval) = sdT;
				}
			else
				{
				SD sdT;
				int cchT;
				int ichStart;
				
				/* copy from ich in sdInput to end or comma */
				ichStart = ich;
				hpch = HpchFromSd(sdInput) + ich;
				while (ich++ < cch && *hpch++ != ',')
					;
				cchT = ich - 1 - ichStart;
				if (*(hpch - 1) == ',')
					cchT -= 1;
			
				if ((sdT = SdFromCch(cchT)) == sdNil)
					RtError(rerrOutOfMemory);
			
				BLTBH(HpchFromSd(sdInput) + ichStart, HpchFromSd(sdT), cchT);
				*((SD huge *) hpval) = sdT;
				}
			break;
			}
		}
	
	if (sdInput != sdNil)
		FreeSd(sdInput);
	
	PopToIev(ievSave);
}


/* %%Function:DoElName %%Owner:bradch */
DoElName()
{
	SD sdOldName, sdNewName;

	EltGetCur();
	
	/* get string */
	ParseValue(elvSd, (SD huge *) &sdOldName, eltComma);

	/* expect eltAs */
	if (Global(eltCur) != eltAs)
		ElSyntaxError(0);
	EltGetCur();

	/* get string */
	ParseValue(elvSd, (SD huge *) &sdNewName, eltComma);

	/* call the rename function */
	ElName((SD huge *) &sdOldName, (SD huge *) &sdNewName);
	FreeSd(sdOldName);
	FreeSd(sdNewName);
}



/*
Syntax: DECLARE [SUB|FUNCTION] name <stuff>

<stuff> (may be specified in any order)
	( <parameters> )
	AS <type>
	ALIAS alias$
	LIB lib$
*/
/* %%Function:DoElDeclare %%Owner:bradch */
DoElDeclare()
{
	struct SY * psy;
	DKT dktRet;
	BOOL fFunction;
	SD sdAlias;
	int cch, idktMacParam;
	char szAlias [cchIdentMax + 20];
	char szLib [cchIdentMax + 20];
	DKT rgdktParam [20];
	BOOL fDontFetch = FALSE;
	
	fFunction = FALSE;
	idktMacParam = 0;
	dktRet = dktNone;
	szLib[0] = '\0';
	
	switch (EltGetCur())
		{
	case eltFunction:
		fFunction = TRUE;
		/* FALL THROUGH */
	
	case eltSub:
		EltGetCur();
		break;
		}

	if (Global(eltCur) != eltIdent)
		{
		ElSyntaxError(1);
		}
	psy = Global(psyCur);
	
	/* Copy name of symbol to szAlias in case an Alias is not given */
	{
	int cch;
	struct SY huge * hpsy;
	
	hpsy = HpsyOfPsy(psy);
	/* subtract 1 from cch below to kill off type character */
	BLTBH(hpsy->st + 1, szAlias, (cch = hpsy->st[0] - 1));
	szAlias[cch] = '\0';
	}

	/* Setup default return type in case one is not specified with AS */
	if (fFunction)
		{
		struct SY huge * hpsy;
		
		hpsy = HpsyOfPsy(psy);
		switch (hpsy->st[hpsy->st[0]])
			{
		case '$':
			dktRet = dktString;
			break;
				
		case '%':
			dktRet = dktInt;
			break;
				
		default:
			dktRet = dktDouble;
			break;
			}
		}

	for (;;)
		{
		if (fDontFetch)
			fDontFetch = FALSE;
		else
			EltGetCur();

		switch (Global(eltCur))
			{
		case eltAlias:
			EltGetCur();
			ParseValue(elvSd, (SD huge *) &sdAlias, eltLParen);

			if ((cch = CchFromSd(sdAlias)) > sizeof (szAlias) - 1)
				{
				FreeSd(sdAlias);
				RtError(rerrStringTooBig);
				}
			
			BLTBH(HpchFromSd(sdAlias), 
				(char huge *) szAlias, cch);
			szAlias[cch] = '\0';

			FreeSd(sdAlias);
			fDontFetch = TRUE;
			break;
			
		case eltLParen:
			while (EltGetCur() == eltIdent)
				{
				int dkt, ch;
				char huge * hpst;
			
				hpst = HpsyOfPsy(Global(psyCur))->st;
				ch = hpst[*hpst];
			
				if (EltGetCur() == eltAs)
					{
					dkt = DktFromElt(EltGetCur());
					EltGetCur();
					}
				else
					{
					switch (ch)
						{
					case '$':
						dkt = dktString;
						break;
					
					case '%':
						dkt = dktInt;
						break;
					
					default:
						dkt = dktDouble;
						break;
						}
					}
			
				rgdktParam[idktMacParam++] = dkt;
			
				if (Global(eltCur) != eltComma)
					break;
				}
		
			if (Global(eltCur) != eltRParen)
				{
				ElSyntaxError(99);
				}
			break;
			
		case eltAs:
			if (!fFunction)
				{
				ElSyntaxError(100);
				}
			
			dktRet = DktFromElt(EltGetCur());
			break;
			
		case eltLib:
			{
			SD sdLib;
		
			EltGetCur();
			ParseValue(elvSd, (SD huge *) &sdLib, eltLParen);
			/* - 5 below for .EXE\0 */
			if ((cch = CchFromSd(sdLib)) > sizeof (szLib) - 5)
				{
				FreeSd(sdLib);
				RtError(rerrStringTooBig);
				}
			
			BLTBH(HpchFromSd(sdLib), (char huge *) szLib, cch);
			szLib[cch] = '\0';
			FreeSd(sdLib);
			fDontFetch = TRUE;
			}
			break;
			
		default:
			goto LDone; /* break out of for (;;) */
			}
		}
LDone:

	/* Must specify a library! */
	if (szLib[0] == '\0')
		{
		ElSyntaxError(0);
		}

	DeclDkd(psy, szLib, szAlias, dktRet,
		idktMacParam, rgdktParam);
}


/* %%Function:DeclDkd %%Owner:bradch */
DeclDkd(psy, szLib, szAlias, dktRet, idktMacParam, rgdktParam)
struct SY * psy;
char * szLib;
char * szAlias;
DKT dktRet;
int idktMacParam;
DKT * rgdktParam;
{
	HANDLE hModule;
	FARPROC lpproc;
	char * pch;
	BOOL fExt;
	int ch;
	
	fExt = FALSE;
	pch = szLib;
	while ((ch = *pch) != '\0')
		{
		if (ch == '.')
			fExt = TRUE;
		else if (ch == '/' || ch == '\\')
			fExt = FALSE;
		pch += 1;
		}
	
	if (!fExt)
		{
		Assert(*pch == '\0');
		*pch++ = '.';
		*pch++ = 'E';
		*pch++ = 'X';
		*pch++ = 'E';
		*pch = '\0';
		}
		
	/* Magic number 32 is from windows documentation... */
	if ((hModule = LoadLibrary((LPSTR) szLib)) < 32)
		{
		/* no msg 'cause windows already gave one */
		RtError(hModule == 2 ? rerrHalt : rerrOutOfMemory);
		}
	
	AnsiUpper((LPSTR) szAlias);
	if ((lpproc = GetProcAddress(hModule, (LPSTR) szAlias)) == NULL)
		{
		FreeLibrary(hModule);
		RtError(rerrIllegalFunctionCall);
		}

	PdkdFromPsy(psy, idktMacParam, hModule, dktRet, lpproc, rgdktParam, 
		TRUE);
}


/* %%Function:DktFromElt %%Owner:bradch */
DktFromElt(elt)
ELT elt;
{
	switch (elt)
		{
	case eltInteger:
		return dktInt;
	
	case eltLong:
		return dktLong;
	
	case eltSingle:
		return dktSingle;
	
	case eltDouble:
		return dktDouble;
	
	case eltStringT:
		return dktString;
	
	case eltAny:
		return dktAny;
		}
	
	ElSyntaxError(5);
}


/* %%Function:ElvFromDkt %%Owner:bradch */
ElvFromDkt(dkt)
DKT dkt;
{
	switch (dkt)
		{
	case dktInt:
		return elvInt;
	
	case dktLong:
	case dktDouble:
		return elvNum;
	
	case dktString:
		return elvSd;
	
	case dktNone:
		return elvNil;
		}
	
	Assert(FALSE);
	return elvNil;
}


/* %%Function:ElSyntaxError %%Owner:bradch */
ElSyntaxError(w)
{
	/* Error: Bad syntax (w) */
	RtError(rerrSyntaxError);
}


/* %%Function:FTdsInUse %%Owner:bradch */
FTdsInUse()
{
	return (sbTds != 0 && ElGlobal(heliFirst) != NULL);
}


/* %%Function:FParseRect %%Owner:bradch */
FParseRect(px, py, pdx, pdy, fPointOK)
int * px, * py, * pdx, * pdy;
BOOL fPointOK;
{
	ParseValue(elvInt, (int huge *) px, eltComma);
	if (Global(eltCur) != eltComma)
		ElSyntaxError(0);
	EltGetCur();
	ParseValue(elvInt, (int huge *) py, eltComma);

	if (Global(eltCur) != eltComma)
		{
		if (!fPointOK)
			{
			ElSyntaxError(1);
			Assert(FALSE); /* NOT REACHED */
			}
		return FALSE; /* just a point */
		}
	EltGetCur();
	ParseValue(elvInt, (int huge *) pdx, eltComma);

	if (Global(eltCur) != eltComma)
		ElSyntaxError(2);
	EltGetCur();
	ParseValue(elvInt, (int huge *) pdy, eltComma);

	return TRUE; /* full rect */
}


/* ListBox x, y, dx, dy, var$(), .field */
/* ComboBox x, y, dx, dy, var$(), text$, .field */
/* %%Function:DoElListBox %%Owner:bradch */
DoElListBox(elt)
ELT elt;
{
	int x, y, dx, dy;
	ELK elk;
	AD ad;
	struct VAR huge * hpvar;
	
	EltGetCur();
	FParseRect(&x, &y, &dx, &dy, FALSE);
	if (Global(eltCur) != eltComma)
		ElSyntaxError(3);
	if (EltGetCur() != eltIdent)
		goto LBadId;
	hpvar = HpvarOfPvar(PvarFromPsy(Global(psyCur), elvNil, TRUE));
	ad = hpvar->ai.ad;
	if (hpvar->ai.elv != elvSd)
		goto LBadId;
	if (EltGetCur() != eltLParen || EltGetCur() != eltRParen)
		{
LBadId:
		ElSyntaxError(0);
		}
	if (EltGetCur() != eltComma)
		ElSyntaxError(7);
	EltGetCur();
	switch (Global(eltCur))
		{
	default:
		ElSyntaxError(9);
		/* NOT REACHED */
		Assert(FALSE);
		
	case eltElk:
		elk = PelcrCur()->elk;
		break;
		
	case eltUserElk:
		elk = ElkUserSt(&PelcrCur()->cch);
		break;
		}
	

	if (elt == eltComboBox)
		{
		ElComboBox(x, y, dx, dy, ad, elk);
		}
	else
		{
		/* Call ListBox stmt proc. */
		ElListBox(x, y, dx, dy, ad, elk);
		}
	EltGetCur();
}


/* UtilGetSynonyms var$() [, word$] */
/* UtilGetSynonyms(var$() [, word$]) */
/* %%Function:WDoElThes %%Owner:bradch */
WDoElThes(fFunc)
{
	struct VAR huge * hpvar;
	AD ad;
	SD sd;
	char ** hst;
	int wRet;
	BOOL fParens;
	
	if (fParens = (EltGetCur() == eltLParen))
		EltGetCur();
	
	if (Global(eltCur) != eltIdent)
		{
		goto LBadId;
		}
	hpvar = HpvarOfPvar(PvarFromPsy(Global(psyCur), elvNil, TRUE));
	ad = hpvar->ai.ad;
	if (hpvar->ai.elv != elvSd)
		{
		goto LBadId;
		}
	if (EltGetCur() != eltLParen || EltGetCur() != eltRParen)
		{
LBadId:
		ElSyntaxError(0);
		}
	if (EltGetCur() != eltComma)
		{
		sd = sdNil;
		}
	else
		{
		EltGetCur();
		ParseValue(elvSd, (SD huge *) &sd, eltNil);
		}
	
	if (fParens)
		{
		if (Global(eltCur) != eltRParen)
			ElSyntaxError(0);
		EltGetCur();
		}
	
	hst = sd == sdNil ? 0 : HstzOfSd(sd);

	wRet = ElWUtilGetSynonyms(ad, hst);

	if (hst != 0)
		FreePpv(sbDds, hst);
	if (sd != sdNil)
		FreeSd(sd);
	
	return wRet;
}


/* UtilGetSpelling var$() [[[, word$], dic$], sdic$] */
/* UtilGetSpelling(var$() [[[, word$], dic$], sdic$]) */
/* %%Function:WDoElSpell %%Owner:bradch */
WDoElSpell(fFunc)
{
	struct VAR huge * hpvar;
	AD ad;
	SD sd, sdDic, sdSupDic;
	char ** hst, ** hstDic, ** hstSupDic;
	int wRet;
	BOOL fIgnoreCaps, fCont, fParens;
	
	fIgnoreCaps = fCont = FALSE;
	
	if (fParens = (EltGetCur() == eltLParen))
		EltGetCur();
	
	if (Global(eltCur) != eltIdent)
		{
		goto LBadId;
		}
	hpvar = HpvarOfPvar(PvarFromPsy(Global(psyCur), elvNil, TRUE));
	ad = hpvar->ai.ad;
	if (hpvar->ai.elv != elvSd)
		{
		goto LBadId;
		}
	if (EltGetCur() != eltLParen || EltGetCur() != eltRParen)
		{
LBadId:
		ElSyntaxError(0);
		}
	
	sd = sdDic = sdSupDic = sdNil;
	
	if (EltGetCur() != eltComma)
		goto LHaveParams;

	EltGetCur();
	ParseValue(elvSd, (SD huge *) &sd, eltComma);
	sdDic = sdSupDic = sdNil;
	if (Global(eltCur) == eltComma)
		{
		EltGetCur();
		ParseValue(elvSd, (SD huge *) &sdDic, eltComma);
		if (Global(eltCur) == eltComma)
			{
			EltGetCur();
			ParseValue(elvSd, (SD huge *) &sdSupDic, eltComma);
			
			if (Global(eltCur) == eltComma)
				{
				EltGetCur();
				ParseValue(elvInt, (int huge *) fIgnoreCaps,
					eltComma);
				
				if (Global(eltCur) == eltComma)
					{
					EltGetCur();
					ParseValue(elvInt, 
						(int huge *) fCont, eltComma);
					}
				}
			}
		}
	
LHaveParams:
	if (fParens)
		{
		if (Global(eltCur) != eltRParen)
			ElSyntaxError(0);
		EltGetCur();
		}
	
	hst = sd != sdNil ? HstzOfSd(sd) : 0;
	hstDic = sdDic != sdNil ? HstzOfSd(sdDic) : 0;
	hstSupDic = sdSupDic != sdNil ? HstzOfSd(sdSupDic) : 0;

	wRet = ElWUtilGetSpelling(ad, hst, hstDic, hstSupDic, 
		fIgnoreCaps, fCont);
	
	if (hst != 0)
		FreePpv(sbDds, hst);
	if (hstDic != 0)
		FreePpv(sbDds, hstDic);
	if (hstSupDic != 0)
		FreePpv(sbDds, hstSupDic);
	if (sd != sdNil)
		FreeSd(sd);
	if (sdDic != sdNil)
		FreeSd(sdDic);
	if (sdSupDic != sdNil)
		FreeSd(sdSupDic);
	
	return wRet;
}


/* %%Function:DoElBeginDlg %%Owner:bradch */
DoElBeginDlg()
{
	int x, y, dx, dy;
	BOOL fAutoPos;
	
	if (EltGetCur() != eltElx || PelcrCur()->elx != ibcmUserDialog)
		{
		ElSyntaxError(0);
		}

	EltGetCur();
	if (fAutoPos = !FParseRect(&x, &y, &dx, &dy, TRUE))
		{
		/* we want auto-positioning */
		dx = x;
		dy = y;
		x = y = 0;
		}

	ElBeginDialog(x, y, dx, dy, fAutoPos);
}


/* %%Function:DoElEditText %%Owner:bradch */
DoElEditText()
{
	int x, y, dx, dy;
	ELK elk;
	
	EltGetCur();
	FParseRect(&x, &y, &dx, &dy, FALSE);
	if (Global(eltCur) != eltComma)
		ElSyntaxError(0);
	if (EltGetCur() != eltUserElk && Global(eltCur) != eltElk)
		ElSyntaxError(2);
	if (Global(eltCur) == eltUserElk)
		{
		elk = ElkUserSt(&PelcrCur()->cch);
		}
	else
		{
		elk = PelcrCur()->elk;
		}
	EltGetCur();
	
	ElEditText(x, y, dx, dy, elk);
}



/* %%Function:DoElCheckBox %%Owner:bradch */
DoElCheckBox()
{
	int x, y, dx, dy;
	ELK elk;
	SD sd;
	
	EltGetCur();
	FParseRect(&x, &y, &dx, &dy, FALSE);

	if (Global(eltCur) != eltComma)
		ElSyntaxError(0);
	EltGetCur();

	ParseValue(elvSd, (SD huge *) &sd, eltComma);
	if (Global(eltCur) != eltComma)
		ElSyntaxError(1);

	if (EltGetCur() != eltUserElk && Global(eltCur) != eltElk)
		ElSyntaxError(2);

	if (Global(eltCur) == eltUserElk)
		{
		elk = ElkUserSt(&PelcrCur()->cch);
		}
	else
		{
		elk = PelcrCur()->elk;
		}
	EltGetCur();

	ElCheckBox(x, y, dx, dy, sd, elk);
	FreeSd(sd);
}



/* %%Function:DoElOptionGroup %%Owner:bradch */
DoElOptionGroup()
{
	ELK elk;
	
	if (EltGetCur() != eltUserElk && Global(eltCur) != eltElk)
		ElSyntaxError(2);
	if (Global(eltCur) == eltUserElk)
		{
		elk = ElkUserSt(&PelcrCur()->cch);
		}
	else
		{
		elk = PelcrCur()->elk;
		}
	EltGetCur();
	
	ElOptionGroup(elk);
}




/****************************************************************************/
