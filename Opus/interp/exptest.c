#ifdef DEBUG
/* exptest.c: test routines for EXP subsystem.
*/
#include <qwindows.h>
#include <qsetjmp.h>
#include <el.h>
#include "eltools.h"
#include <uops.h>

#define EXTERN	extern
#include "priv.h"
#include "exp.h"
#include "sym.h"
#include "_exp.h"

csconst char mpelvsz[][] = {
	"elvNil", "elvNum", "elvInt", "elvSd", "elvArray",
			"elvFormal", "elvEnv", "elvEndStack", "elvEndModuleFrame",
			"elvEndProcFrame"};


csconst char mpelksz[][] = {
	"elkNil", "elkName"};


#define elkMax	2


/* %%Function:PrintElv %%Owner:bradch */
PrintElv(elv)
ELV elv;
{
	if (elv == elvNilRecord)
		PrintT("elvNilRecord", 0);
	else  if (FRecordElv(elv))
		PrintT("(rec%u)", IeldiFromElv(elv));
	else
		PrintTLsz(mpelvsz[elv], 0);
}


/* %%Function:PrintElk %%Owner:bradch */
PrintElk(elk)
ELK elk;
{
	PrintTLsz(mpelksz[elk], 0);
}


/* %%Function:DumpEvi %%Owner:bradch */
DumpEvi(evi)
EVI evi;
{
	struct EV huge *hpev = HpevOfEvi(evi);

	PrintT("EXP: EV ", 0);
	DumpAuxEvi(evi);
	PrintT("\n", 0);
}


/* %%Function:DumpAuxEvi %%Owner:bradch */
DumpAuxEvi(evi)
EVI evi;
{
	struct EV huge *hpev = HpevOfEvi(evi);

	if (hpev->fResolved)
		{
		if (hpev->fAggregate)
			{
			switch (hpev->elvAggr)
				{
			default:
				Assert(FRecordElv(hpev->elvAggr));
				PrintT("(record-reference)", 0);
				break;
			case elvArray:
				PrintT("(array-reference)", 0);
				break;
				}
			}
		else
			{
			switch (hpev->elv)
				{
			default:
				PrintT("(constant ", 0);
				PrintElv(hpev->elv);
				PrintT(")", 0);
				break;
			case elvNum:
				OutHpnum(&hpev->num);
				break;
			case elvInt:
				PrintT("(elvInt: %d)", hpev->w);
				break;
				}
			}
		}
	else
		{
		struct SY huge *hpsy;

		if (hpev->fElx)
			PrintT("<ELX %u>", hpev->elx);
		else
			{
			Assert(hpev->psy != 0);

			hpsy = HpsyOfPsy(hpev->psy);
			PrintT("<ident \"", 0);
			PrintHst(hpsy->st);
			PrintT("\">", 0);
			}
		}
	if (hpev->elk != elkNil)
		{
		if (hpev->elk >= elkMax)
			PrintT("{%u}", hpev->elk);
		else
			{
			PrintT("{", 0);
			PrintElk(hpev->elk);
			PrintT("}", 0);
			}
		}
}


/* %%Function:DumpOpr %%Owner:bradch */
DumpOpr(elt)
ELT elt;
{
	int ielt, iev;

	Assert(SbCur() == 1);

	PrintT("EXP: opr/opd ", 0);
	PrintElt(elt);
	PrintT("\t|", 0);
	for (ielt = ElGlobal(ieltStackPtr);
			ielt > Global(ieltStackBase);
			ielt--)
		{
		PrintElt(RgeltStack()[ielt]);
		PrintT(" ", 0);
		}

	PrintT("\n", 0);
	PrintT("EXP: %d\t\t\t|", Global(ievStackBase));

	for (iev = ElGlobal(ievStackPtr);
			iev > Global(ievStackBase);
			iev--)
		{
		struct EV huge *hpev;

		hpev = HpevOfEvi(EviOfIev(iev));
		if (hpev->fResolved)
			{
			if (hpev->fAggregate)
				PrintT("(aggregate-ref)", 0);
			else
				{
				switch (hpev->elv)
					{
				case elvNum:
					OutHpnum(&hpev->num);
					break;
				case elvSd:
					PrintT("\"", 0);
					if (hpev->sd != 0)
						PrintHs16(Hs16FromSd(hpev->sd));
					PrintT("\"", 0);
					break;
				case elvInt:
					PrintT("(int: %d)", hpev->w);
					break;
				default:
					PrintT("<constant ", 0);
					PrintElv(hpev->elv);
					PrintT(">", 0);
					break;
					}
				}
			}
		else
			{
			struct SY huge *hpsy;

			if (hpev->fElx)
				PrintT("(ELX %u)", hpev->elx);
			else
				{
				hpsy = HpOfSbIb(Global(sbNtab), hpev->psy);
				PrintT("(SY ", 0);
				PrintHst(hpsy->st);
				PrintT(":", 0);
				PrintElv(hpev->elv);
				PrintT(")", 0);
				}
			}
		if (hpev->elk != elkNil)
			{
			if (hpev->elk >= elkMax)
				PrintT("{%u}", hpev->elk);
			else
				{
				PrintT("{", 0);
				PrintElk(hpev->elk);
				PrintT("}", 0);
				}
			}
		PrintT(" ", 0);
		}
	PrintT("\n", 0);
}


/* %%Function:DumpApply %%Owner:bradch */
DumpApply(elt)
{
	PrintT("EXP: applying operator ", 0);
	PrintElt(elt);
	PrintT("\n", 0);
}


#endif /* DEBUG */
