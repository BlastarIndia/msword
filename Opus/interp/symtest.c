#ifdef DEBUG
/* symtest.c: test output module for SYM subsystem.
*/
#include <qwindows.h>
#include <qsetjmp.h>
#include <el.h>
#include "eltools.h"

#define EXTERN	extern
#include "priv.h"
#include "sym.h"
#include "_sym.h"


csconst BYTE mpelvcbVar[] =
{
	0, cbNumVar, cbIntVar, cbSdVar, cbAdVar, cbFormalVar, cbEnvVar};


#define CbVarFromElv(elv)	(FRecordElv(elv) ? cbRecordVar		\
							: mpelvcbVar[elv])
csconst char mpsytsz[][] = {
	"sytNil", "sytProc", "sytVar", "sytExternal"};


/* %%Function:PrintSyt %%Owner:bradch */
PrintSyt(syt)
{
	PrintTLsz(mpsytsz[syt], 0);
}


/* %%Function:ToPsy %%Owner:bradch */
ToPsy(psy)
/* Prints an ASCII representation of a newly created SY.
*/
struct SY *psy;
{
	struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), psy);
	unsigned ich, cch;

	PrintT("SYM: created SY \"", 0);

	cch = hpsy->st[0];
	if (cch > cchIdentMax)
		cch = cchIdentMax;		/* in case data is trashed */
	PrintHst(hpsy->st);
	PrintT("\" at offset %u\n", (WORD)psy);
	DumpNtab();
}


/* %%Function:DumpSymbols %%Owner:bradch */
DumpSymbols()
	/* Prints an ASCII representation of the symbol table.
	*/
{
	DumpNtab();
	DumpFrame();
}


/* %%Function:DumpNtab %%Owner:bradch */
DumpNtab()
{
	int iwHash;

	PrintT("SYM: Symbol table dump ...\n", 0);

	for (iwHash = 0; iwHash < cwHash; iwHash++)
		{
		struct SY *psy;

		psy = *(HpNtab(rgpsyHash) + iwHash);

		if (psy == 0)
			continue;

		do
			{
			int ich;
			struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), psy);

			PrintHst(hpsy->st);
			PrintT(" (", 0);
			PrintSyt(hpsy->syt);
			PrintT(")\n", 0);

			psy = *HpOfSbIb(Global(sbNtab), &psy->psyNext);
			}
		while (psy != 0);
		}

	PrintT("SYM: end of symbol table dump.\n", 0);
}


/* %%Function:DumpFrame %%Owner:bradch */
DumpFrame()
{
	struct FH huge *hpfh;
	struct VAR huge *hpvar;

	PrintT("SYM: ||| FRAME DUMP, pfh = %u\n", (WORD)Global(pfhCurFrame));

	hpfh = HpOfSbIb(sbFrame, Global(pfhCurFrame));
	for (hpvar = hpfh->rgvar; IbOfHp(hpvar) < *HpFrame(ibMac);
			hpvar = (char huge *)hpvar + CbVarFromElv(hpvar->elv))
		{
		PrintT("SYM: |||   VAR \"", 0);
		if (hpvar->psy == psyNil)
			PrintT("(temp)", 0);
		else
			{
			PrintHst(((struct SY huge *)
					HpOfSbIb(Global(sbNtab), hpvar->psy))->st);
			}
		PrintT("\" (", 0);
		PrintElv(hpvar->elv);
		PrintT(")", 0);

		switch (hpvar->elv)
			{
		default:
			break;
		case elvFormal:
				{
				struct VAR huge *hpvarRef = HpvarOfPvar(hpvar->pvar);

				PrintT(" --> (", 0);
				PrintElv(hpvarRef->elv);
				PrintT(")", 0);
				switch (hpvarRef->elv)
					{
				default:
					break;
				case elvNum:
					PrintT(" = ", 0);
					OutHpnum(&hpvarRef->num);
					break;
				case elvSd:
					PrintT(" @ %u", hpvarRef->sd);
					PrintT(" = ", 0);
					PrintHs16(Hs16FromSd(hpvarRef->sd));
					break;
					}
				break;
				}
		case elvNum:
			PrintT(" = ", 0);
			OutHpnum(&hpvar->num);
			break;
			}
		PrintT("\n", 0);
		}

	PrintT("SYM: ||| END OF FRAME DUMP\n", 0);
}


/* %%Function:OutPvar %%Owner:bradch */
OutPvar(pvar)
struct VAR *pvar;
{
	struct VAR huge *hpvar = HpOfSbIb(sbFrame, pvar);

	PrintT("VAR: created variable \"", 0);
	if (hpvar->psy == psyNil)
		PrintT("(temp)", 0);
	else
		{
		struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), hpvar->psy);
		PrintHst(hpsy->st);
		}
	PrintT("\" (", 0);
	PrintElv(hpvar->elv);
	PrintT(")\n", 0);
}


/* %%Function:OutPlab %%Owner:bradch */
OutPlab(plab, psy)
struct LAB *plab;
{
	struct LAB huge *hplab = HpOfSbIb(Global(sbNtab), plab);
	struct SY huge *hpsy = HpOfSbIb(Global(sbNtab), psy);

	PrintT("SYM: defined label \"", 0);
	PrintHst(hpsy->st);
	PrintT("\"; plab = %u\n", plab);
}


#endif /* DEBUG */
