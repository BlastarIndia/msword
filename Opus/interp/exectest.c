#ifdef DEBUG
/* exectest.c: Test routines for EXEC subsystem.
*/
#include <qwindows.h>
#include <qsetjmp.h>
#include <el.h>

#define EXTERN	extern
#include "priv.h"
#include "sym.h"
#include "_exec.h"


csconst char mpeltsz[][] = {
	"eltNil", "eltMinus", "eltNot", "eltAnd", "eltOr", "eltLParen",
			"eltRParen", "eltAdd", "eltSubtract", "eltDiv", "eltMul", "eltMod",
			"eltEq", "eltNe", "eltLt", "eltGt", "eltLe", "eltGe", "eltComma",
			"eltSubscr", "eltOperand", "eltElkGets", "eltEndExp",
			"eltEol", "eltResume", "eltColon", "eltEnd", "eltSub",
			"eltFunction", "eltIf", "eltThen", "eltElseif", "eltElse", "eltWhile",
			"eltWend", "eltFor", "eltTo", "eltStep", "eltNext", "eltExit",
			"eltRem", "eltCall", "eltGoto", "eltStop", "eltOn", "eltError",
			"eltLet", "eltDim", "eltShared", "eltSelect", "eltIs", "eltCase",
			"eltAs", "eltRedim", "eltLineNolabel", "eltLineIdent", "eltLineInt",
			"eltElt", "eltElx", "eltElk", "eltNum", "eltIdent", "eltString",
			"eltComment", "eltInt", "eltEof"};


csconst char mpcstsz[][] = {
	"cstNil", "cstBlockElse", "cstBlockEndif", "cstSingleElse", "cstNext",
			"cstWend", "cstCase", "cstEndProc"
};


/* %%Function:PrintElt %%Owner:bradch */
PrintElt(elt)
{
	PrintTLsz(mpeltsz[elt], 0);
}


/* %%Function:PrintCst %%Owner:bradch */
PrintCst(cst)
{
	PrintTLsz(mpcstsz[cst], 0);
}



/* %%Function:OutElt %%Owner:bradch */
OutElt(elt, lib)
ELT elt;
LIB lib;
{
	PrintT("EXEC:\t\t\t\t", 0);
	PrintElt(elt);
	PrintT(" (%d)", elt);
	PrintT(" at %u\n", (WORD)lib);
}


/* %%Function:OutExecCall %%Owner:bradch */
OutExecCall()
{
	PrintT("EXEC: call ...\n", 0);
}


/* %%Function:OutExecGets %%Owner:bradch */
OutExecGets(pvar, elvValue, hpval)
struct VAR *pvar;
ELV elvValue;
BYTE huge *hpval;
{
	struct VAR huge *hpvar = HpOfSbIb(sbFrame, pvar);
	struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), hpvar->psy);

	PrintT("VAR: assign ", 0);

	switch (elvValue)
		{
	default:
		Assert(FRecordElv(elvValue));
		PrintT("rec%u\n", IeldiFromElv(elvValue));
		break;
	case elvSd:
		PrintT("\"", 0);
		if (*(SD huge *)hpval != 0)
			PrintHs16(Hs16FromSd(*(SD huge *)hpval));
		PrintT("\"", 0);
		break;
	case elvNum:
		OutHpnum(hpval);
		break;
		}

	if (pvar != 0)
		{
		PrintT(" to ", 0);
		if (hpvar->psy == psyNil)
			PrintT("TEMP\n", 0);
		else
			{
			hpsy = HpsyOfPsy(hpvar->psy);
			PrintT("\"", 0);
			PrintHst(hpsy->st);
			PrintT("\"\n", 0);
			}
		}
	else
		PrintT("\n", 0);
}


/* %%Function:DumpIcsr %%Owner:bradch */
DumpIcsr(icsrIn, fPush)
int icsrIn;
BOOL fPush;
{
	struct CSR huge *hpcsrIn = HpcsrOfIcsr(icsrIn);
	int icsrT;

	if (fPush)
		PrintT("CTRL: push CSR (", 0);
	else
		PrintT("CTRL: pop CSR (", 0);
	PrintCst(hpcsrIn->cst);

	PrintT(", libStart = %u)", (WORD)hpcsrIn->libStart);
	PrintT(" --> %d\n", ElGlobal(icsrStackPtr));

	PrintT("CTRL:\t\tcontrol stack dump ...\n", 0);
	for (icsrT = 0; icsrT < ElGlobal(icsrStackPtr) + 1; icsrT++)
		{
		struct CSR huge *hpcsrT;

		hpcsrT = HpcsrOfIcsr(icsrT);

		PrintT("CTRL:\t\t\tCSR ", 0);
		PrintCst(hpcsrT->cst);
		PrintT(" (libStart = %u)\n", (WORD)hpcsrT->libStart);
		}
	PrintT("CTRL:\t\tend of control stack dump\n", 0);
}


#endif /* DEBUG */
