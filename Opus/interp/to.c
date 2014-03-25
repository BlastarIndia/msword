/* to.c: test output module for EL compiler.
*/
#ifdef DEBUG	/* whole file */

#include <qwindows.h>
#include <qsetjmp.h>
#include <uops.h>
#include <el.h>
#include "eltools.h"

#define EXTERN	extern
#include "priv.h"
#include "sym.h"

extern HWND vhwndApp;


/* %%Function:ClearTestOptions %%Owner:bradch */
ClearTestOptions()
	/* Turns off all test output flags.
	*/
{
	fToMain = fToExp = fToCorout = FALSE;
	fToExec = fToSym = fToAlloc = FALSE;
	fToVar = fToCtrl = fToRead = fToArrays = FALSE;
}


/* %%Function:SetElTestOptions %%Owner:bradch */
SetElTestOptions(sz)
/* Turns on a selected set of the test output flags.
*/
char *sz;
{
	char *pch = sz;

	while (1)
		{
		switch (*pch++)
			{
		default:
			break;		/* meant for someone else */
		case 'm':	
			fToMain = TRUE; 
			break;
		case 'e':	
			fToExp = TRUE; 
			break;
		case 'x':	
			fToExec = TRUE; 
			break;
		case 's':	
			fToSym = TRUE; 
			break;
		case 'a':	
			fToAlloc = TRUE; 
			break;
		case 'c':	
			fToCtrl = TRUE; 
			break;
		case 'v':	
			fToVar = TRUE; 
			break;
		case 'r':	
			fToRead = TRUE; 
			break;
		case 'A':	
			fToArrays = TRUE; 
			break;
		case 'C':	
			fToCorout = TRUE; 
			break;
		case '+':
			fToMain = fToExp = TRUE;
			fToExec = fToSym = fToAlloc = TRUE;
			fToVar = fToCtrl = fToRead = fToArrays = TRUE;
			break;
		case '\0':
			return;
			}
		}
/*	ElGlobal(ichCur) = 0;	/* initialize PrintT */
/**/
}



/* %%Function:PrintHp %%Owner:bradch */
PrintHp(hpb)
BYTE huge *hpb;
{
	PrintT("%04x", SbOfHp(hpb));
	PrintT(":%04x", IbOfHp(hpb));
}


/* %%Function:PrintTLsz %%Owner:bradch */
PrintTLsz(lsz, wArg)
char far *lsz;
WORD wArg;
{
	char rgch[100];
	BLTBX(lsz, rgch, 100);
	PrintTSz(rgch, wArg);
}


/* %%Function:PrintHst %%Owner:bradch */
PrintHst(hst)
char huge *hst;
{
	unsigned ich, cch = hst[0];

	for (ich = 0; ich < cch; ich++)
		{
		unsigned ch;

		if ((ch = hst[ich + 1]) > '\x20')
			PrintT("%c", ch);
		else
			PrintT("<\\x%02x>", ch);
		}
}


/* %%Function:OutHpnum %%Owner:bradch */
OutHpnum(hpnum)
NUM huge *hpnum;
{
	char rgchT[40];
	NUM numT;

	BLTBX(LpOfHp(hpnum), (char far *)&numT, sizeof(NUM));

	UnpackPnumPch(&numT, rgchT);
	PrintT("%s", rgchT);
}


/* %%Function:PrintHs16 %%Owner:bradch */
PrintHs16(hs16)
HS16 hs16;
{
	unsigned ich, cch = hs16->cch;

	for (ich = 0; ich < cch; ich++)
		{
		unsigned ch;

		if ((ch = hs16->rgch[ich]) > '\x20')
			PrintT("%c", ch);
		else
			PrintT("<\\x%02x>", ch);
		}
}


/* %%Function:UnpackPnumPch %%Owner:bradch */
UnpackPnumPch(pnum, pch)
/* Converts a number to a zero-terminated string.
*/
char *pch;
NUM *pnum;
{
	int wExp, wSign;
	char rgchMantissa[20];
	unsigned cchMantissa;
	int ich;

	LdiNum(pnum);
	UnpackNum(&wExp, &wSign, rgchMantissa);

	if (wSign < 0)
		*pch++ = '-';
	if (wExp >= -2 && wExp < 17)
		{	/* -- reasonable-valued number -- */
		/* Print the part before the decimal point.
			*/
		if (wExp <= 0)
			*pch++ = '0';
		else
			{
			for (ich = 0; ich < wExp; ich++)
				{
				if (rgchMantissa[0] <= ich)
					*pch++ = '0';
				else
					*pch++ = rgchMantissa[ich + 1];
				}
			}

		/* Print four digits after the decimal point.
			*/
		*pch++ = '.';

		for (ich = wExp; ich < wExp + 4; ich++)
			{
			if (ich < 0 || rgchMantissa[0] <= ich)
				*pch++ = '0';
			else
				*pch++ = rgchMantissa[ich + 1];
			}
		}
	else
		{	/* -- unreasonable-valued number -- */
		*pch++ = '0';
		*pch++ = '.';
		for (ich = 0; ich < 8; ich++)
			{
			if (rgchMantissa[0] <= ich)
				*pch++ = '0';
			else
				*pch++ = rgchMantissa[ich + 1];
			}
		*pch++ = 'E';
		*pch++ = (wExp < 0 ? '-' : '+');
		wExp = (wExp < 0 ? -wExp : wExp);
		*pch++ = wExp/100 + '0';
		*pch++ = (wExp % 100)/10 + '0';
		*pch++ = (wExp % 10) + '0';
		}

	*pch = '\0';
}


/* %%Function:PrintTSz %%Owner:bradch */
PrintTSz(sz, val)
/* Prints out the string, keeping track of the current character position.
*/
char *sz;
VAL val;
{
	CommSzNum(sz, val);
}


#endif

