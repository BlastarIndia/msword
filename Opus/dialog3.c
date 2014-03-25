/* D I A L O G 3 . C */
/* Rare or special purpose dialog functions */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "error.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "resource.h"
#include "wininfo.h"
#include "opuscmd.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "version.h"
#include "screen.h"
#include "idd.h"
#include "prompt.h"
#include "message.h"
#include "doslib.h"
#include "keys.h"
#include "rareflag.h"
#include "props.h"
#include "format.h"
#include "sel.h"
#include "inter.h"
#include "style.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"



#define chPath ('\\')

#define dttmNinch (0)
#define dttmError (1)

extern struct SCI  vsci;
extern struct SEL  selCur;
extern int vfShowAllStd;
extern char (**vhmpiststcp)[];
extern int vstcBackup;
extern int vdocStsh;
extern int vistLbMac;
extern struct MERR  vmerr;
extern BOOL fElActive;
extern struct MERR  vmerr;
extern struct ITR       vitr;
extern struct PREF vpref;
extern int         wwCur;
extern BOOL fElActive;


/* For dialogs where the grayness of a button depends on more than one
	edit control not being blank */
/*  %%Function:  GrayRgtmcOnBlank  %%Owner:  bobz       */

VOID GrayRgtmcOnBlank(rgtmc, itmcMac, tmcButton)
TMC rgtmc[];
int itmcMac;
TMC tmcButton;
{
	char sz [ichMaxBufDlg];
	int itmc;
	BOOL fEnable = fTrue;

	for (itmc = 0; itmc < itmcMac; itmc++)
		{
		GetTmcText(rgtmc[itmc], sz, ichMaxBufDlg);
		fEnable = fEnable && (CchStripString(sz, CchSz(sz) - 1) != 0);
		}
	EnableTmc(tmcButton, fEnable);
}



/*  table of unit names. Order must be same as ut* order
	(utInch = 0, utPt = 1, etc.) in disp.h */

extern CHAR    *mputsz[utMax];

/*  table of unit values. Order must be same as ut* order (utInch = 0,
		utPt = 1, etc.) in disp.h */

unsigned mputczaUt[utMax] =
	{
	czaInch, czaPoint, czaCm, czaPicas, czaP10, czaP12, czaLine

#ifdef INTL	 /* FUTURE make sure wordwin.h, statline tables updated */
		, czaInch
#endif /* INTL */

	};




#ifndef INTL
   /* eventually use the INTL form for all. At ship time did not
	  want to risk using the rewrite bobz

	  This is the current US form of the function, with hard coded strings
   */

/* ****
*  Description:  Return za in *pza from string representation in ss.
*     (ss can be sz or st since cch is given). Returns true if valid za
*      This is the general all-purpose unit-terminated string to integer
*      routine.
** **** */
/*  %%Function:  FZaFromSs  %%Owner:  bobz       */

int FZaFromSs(pza, ss, cch, ut, pfOverflow)
int     *pza;
CHAR    ss[];
int     cch, ut;
BOOL	*pfOverflow;
{
	long    lza      = 0;
	register CHAR    *pch    = ss;
	register CHAR  *pchMac = &ss[cch];
	int     ch;
	unsigned czaUt;
	int     fNeg;

	*pfOverflow = fFalse;
	if (cch <= 0)
		return fFalse;

	switch (ChLower((*--pchMac)))
		{ /* Check for units */
	case 'n': /* inch */
		if (ChLower(*--pchMac) != 'i')
			goto NoUnits;
	case '"': /* inch */
		ut = utInch;
		break;
	case '0': /* p10 */
		if (*--pchMac != '1' || ChLower(*--pchMac) != 'p')
			goto NoUnits;
		ut = utP10;
		break;
	case '2': /* p12 */
		if (*--pchMac != '1' || ChLower(*--pchMac) != 'p')
			goto NoUnits;
		ut = utP12;
		break;
	case 'i': /* line or pica */
		if (ChLower(*--pchMac) == 'l')
			ut = utLine;
		else  if (ChLower(*pchMac) == 'p')
			ut = utPica;
		else
			goto NoUnits;
		break;
	case 't': /* pt */
		if (ChLower(*--pchMac) != 'p')
			goto NoUnits;
		ut = utPt;
		break;
	case 'm': /* cm */
		if (ChLower(*--pchMac) != 'c')
			goto NoUnits;
		ut = utCm;
		break;
	default:
		++pchMac;
		break;
NoUnits:
		pchMac = &ss[cch];
		}

	while (pch < pchMac && *(pchMac - 1) == chSpace)
		--pchMac;

	czaUt = mputczaUt[ut];

/* extract leading blanks */
	while (*pch == ' ')
		pch++;

	fNeg = *pch == '-';
	if (fNeg) ++pch;        /* skip past minus sign */
	while ((ch = *pch++) != vitr.chDecimal)
		{
		if (ch < '0' || ch > '9')
			return fFalse;
		lza = lza * 10 + (ch - '0') * czaUt;
		if (lza >= (long) czaMax)
			goto NumOvfl;
		if (pch >= pchMac)
			goto GotNum;
		}

	while (pch < pchMac)
		{
		ch = *pch++;
		if (ch < '0' || ch > '9')
			return fFalse;
		lza += ((ch - '0') * czaUt + 5) / 10;
		if (lza >= (long) czaMax)
			goto NumOvfl;
		czaUt = (czaUt + 5) / 10;
		}

GotNum:
	if (lza >= (long) czaMax)
		{
NumOvfl:
		*pfOverflow = fTrue;
		return fFalse;
		}

	*pza = fNeg ? (int) -lza : (int) lza;
	return fTrue;
}

#else  /* INTL form. uses table rather than hard coded strings */

/* ****
*  Description:  Return za in *pza from string representation in ss.
*     (ss must be sz or even though cch is given).
*      FUTURE: rename FzaFromSz. All current callers pass in sz's.
*      Returns true if valid za
*      This is the general all-purpose unit-terminated string to integer
*      routine.
** **** */
/*  %%Function:  FZaFromSs  %%Owner:  bobz       */

int FZaFromSs(pza, sz, cch, ut, pfOverflow)
int     *pza;
CHAR    sz[];
int     cch, ut;
BOOL	*pfOverflow;
{
	long    lza      = 0;
	register CHAR    *pch;
	register CHAR  *pchMac;
	int     ch;
	unsigned czaUt;
	int     fNeg;
	int utT;
	BOOL fRet = fFalse;
	int cchStrip;


#ifdef USA	/* old one only work with us strings */
#ifdef DEBUG
     /* call the old one and compare later */
	int zaT;
    BOOL fOverflowT;
	BOOL fT = FZaFromSsVfy(&zaT, sz, cch, ut, &fOverflowT);

#endif /* DEBUG */
#endif /* USA */



	*pfOverflow = fFalse;

	/* extract units string if any. Set pchMac to last digit in string.
	   set ut based on units string */

     /* strip trailing spaces */
	pchMac = &sz[cch];
	Assert (*pchMac == 0);  /* must be an sz, cch must point to end */
	while (sz < pchMac && *(pchMac - 1) == chSpace)
		--pchMac;
	cchStrip = pchMac - sz;
	if (cchStrip <= 0)
		goto LRet;
	if (cchStrip < cch)	  /* need an sz; restore space later */
		sz[cchStrip] = 0;

#ifdef BZ
		CommSzNum(SzShared("FZaFromSz pchMac "),pchMac);
		CommSzRgCch(SzShared("FZaFromSz sznumber 1st strip "), sz, pchMac - sz);
#endif /* BZ */


	   /* set ut if string matches table entry. Must back up length
		  of unit string for compare
	   */
	for (utT = 0; utT < utMax; utT++)
		{
		int cchUt;

		/* FUTURE (bobz)make mputsz into mputstz so we can get length easier */
		/* we are requiring that ss is an sz to avoid bltting each time */
		cchUt = CchSz(mputsz[utT]) - 1;
		if (cchStrip < cchUt)
			continue;
		pch = pchMac - cchUt;

#ifdef BZ
		CommSzSz(SzShared("FZaFromSz szUnit "), pch);
		CommSzSz(SzShared("FZaFromSz mputsz "), mputsz[utT]);
#endif /* BZ */


		if (!WCompSzSrt(pch,mputsz[utT],fFalse /* fCase */))
			{
#ifdef BZ
			CommSzSz(SzShared("FZaFromSz match "), "");
#endif /* BZ */
			ut = utT;
			pchMac = pch;  /* units found, back up before string */
     			/* strip spaces between number and unit string */
			while (sz < pchMac && *(pchMac - 1) == chSpace)
				--pchMac;

			break;
			}
		}

	czaUt = mputczaUt[ut];

#ifdef BZ
		CommSzRgCch(SzShared("FZaFromSz sznumber after 2nd strip "), sz, pchMac - sz);
#endif /* BZ */

	pch = sz;
/* extract leading blanks */
	while (*pch == ' ')
		pch++;

	fNeg = *pch == '-';
	if (fNeg)
		++pch;        /* skip past minus sign */
	while ((ch = *pch++) != vitr.chDecimal)
		{
		if (ch < '0' || ch > '9')
			goto LRet;
		lza = lza * 10 + (ch - '0') * czaUt;
		if (lza >= (long) czaMax)
			goto NumOvfl;
		if (pch >= pchMac)
			goto GotNum;
		}

	while (pch < pchMac)
		{
		ch = *pch++;
		if (ch < '0' || ch > '9')
			goto LRet;
		lza += ((ch - '0') * czaUt + 5) / 10;
		if (lza >= (long) czaMax)
			goto NumOvfl;
		czaUt = (czaUt + 5) / 10;
		}

GotNum:
	if (lza >= (long) czaMax)
		{
NumOvfl:
		*pfOverflow = fTrue;
		goto LRet;
		}

	*pza = fNeg ? (int) -lza : (int) lza;
	fRet = fTrue;
LRet:


#ifdef USA
#ifdef DEBUG
      /* compare with previous results */
	if (fRet != fT)
		{
#ifdef BZ 
		CommSzNum(SzShared("FZaFromSz Old ret: "), fT);
		CommSzNum(SzShared("FZaFromSz New ret: "), fRet);
#endif /* BZ */
		Assert (fFalse);
		}
	else if (fRet)
		{
		if (zaT != *pza || fOverflowT != *pfOverflow)
			{
#ifdef BZ 
			CommSzNum(SzShared("FZaFromSz Old za: "), zaT);
			CommSzNum(SzShared("FZaFromSz New za: "), *pza);
			CommSzNum(SzShared("FZaFromSz Old fOverflow: "), fOverflowT);
			CommSzNum(SzShared("FZaFromSz New fOverflow: "), *pfOverflow);
#endif /* BZ */
			Assert (fFalse);
			}
		}
#endif /* DEBUG */
#endif /* USA */



	if (cchStrip < cch)	  /* restore old sting value */
		sz[cchStrip] = chSpace;

	return fRet;
}





#ifdef USA
#ifdef DEBUG
  /* compare values from intl and us versions. Should be same */
int FZaFromSsVfy(pza, ss, cch, ut, pfOverflow)
int     *pza;
CHAR    ss[];
int     cch, ut;
BOOL	*pfOverflow;
{
	long    lza      = 0;
	register CHAR    *pch    = ss;
	register CHAR  *pchMac = &ss[cch];
	int     ch;
	unsigned czaUt;
	int     fNeg;

	*pfOverflow = fFalse;

     /* strip trailing spaces ADDED TO VFY ONLY. not in original; should be */
	while (ss < pchMac && *(pchMac - 1) == chSpace)
		--pchMac;
	cch = pchMac - ss;
	if (cch <= 0)
		return fFalse;

	switch (ChLower((*--pchMac)))
		{ /* Check for units */
	case 'n': /* inch */
		if (ChLower(*--pchMac) != 'i')
			goto NoUnits;
	case '"': /* inch */
		ut = utInch;
		break;
	case '0': /* p10 */
		if (*--pchMac != '1' || ChLower(*--pchMac) != 'p')
			goto NoUnits;
		ut = utP10;
		break;
	case '2': /* p12 */
		if (*--pchMac != '1' || ChLower(*--pchMac) != 'p')
			goto NoUnits;
		ut = utP12;
		break;
	case 'i': /* line or pica */
		if (ChLower(*--pchMac) == 'l')
			ut = utLine;
		else  if (ChLower(*pchMac) == 'p')
			ut = utPica;
		else
			goto NoUnits;
		break;
	case 't': /* pt */
		if (ChLower(*--pchMac) != 'p')
			goto NoUnits;
		ut = utPt;
		break;
	case 'm': /* cm */
		if (ChLower(*--pchMac) != 'c')
			goto NoUnits;
		ut = utCm;
		break;
	default:
		++pchMac;
		break;
NoUnits:
		pchMac = &ss[cch];
		}

	while (pch < pchMac && *(pchMac - 1) == chSpace)
		--pchMac;

	czaUt = mputczaUt[ut];

/* extract leading blanks */
	while (*pch == ' ')
		pch++;

	fNeg = *pch == '-';
	if (fNeg) ++pch;        /* skip past minus sign */
	while ((ch = *pch++) != vitr.chDecimal)
		{
		if (ch < '0' || ch > '9')
			return fFalse;
		lza = lza * 10 + (ch - '0') * czaUt;
		if (lza >= (long) czaMax)
			goto NumOvfl;
		if (pch >= pchMac)
			goto GotNum;
		}

	while (pch < pchMac)
		{
		ch = *pch++;
		if (ch < '0' || ch > '9')
			return fFalse;
		lza += ((ch - '0') * czaUt + 5) / 10;
		if (lza >= (long) czaMax)
			goto NumOvfl;
		czaUt = (czaUt + 5) / 10;
		}

GotNum:
	if (lza >= (long) czaMax)
		{
NumOvfl:
		*pfOverflow = fTrue;
		return fFalse;
		}

	*pza = fNeg ? (int) -lza : (int) lza;
	return fTrue;
}

#endif /* DEBUG */
#endif /* USA */


#endif /* INTL */


/* ****
*  Description: Stuff the expansion of linear measure za in unit ut into pch.
		Return # of chars stuffed.  Don't exceed cchMax. This is the general
*       all-purpose integer to unit-terminated string routine...but doesn't
*       put the units on if !fUnit
*       WARNING: this does not null terminate strings unless fUnit is
*         true and cchMax is at least ichMax + 1 characters long!

** **** */

/*  %%Function:  CchExpZaRnd  %%Owner:  bobz       */

int CchExpZaRnd(ppch, za, ut, cchMax, fUnit, grpfRnd)
CHAR **ppch;
int ut, cchMax;
int za;
int fUnit, grpfRnd;
{
	register int cch = 0;
	unsigned czaUt;
	int zu;

	/* If not in point mode and even half line, display as half lines v. points */
	if (ut == utPt && vpref.ut != utPt &&
			(za / (czaLine / 2) * (czaLine / 2)) == za)
		ut = utLine;

	czaUt = mputczaUt[ut];
	if (cchMax < ichMaxNum)  /* need room for # plus term */
		{
		if (cchMax > 0)
			**ppch = 0; /* just to guarantee null term in case it is expected */

		Assert (fFalse);  /* we should stop any of these if we can bz */
		return 0;
		}
	if (za < 0)
		{ /* Output minus sign and make positive */
		*(*ppch)++ = '-';
		za = -za;
		cch++;
		/* invert meaning of hi/lo rounding for neg #'s */
		if (grpfRnd == grpfRndHigh)
			grpfRnd = grpfRndLow;
		else  if (grpfRnd == grpfRndLow)
			grpfRnd = grpfRndHigh;

		}

		{
		long lza;

		/* note: to avoid losing decimals on 1/8" units, we multiply
			by 100 before the first division, then scale down by 10
			Otherwise, dividing czaInch by 200 gives 7.2 -> 7 and we
			round to .12 instead of .13 bz
		*/

		lza = ((long)za * 100);	/* To avoid overflow in subsequent operations. */
		/* if rounding, add .005; rounding high add ~.0099 ((czaUt - 1) /100),
			rounding low add nothing */
		if (grpfRnd == grpfRndRnd)
			lza += czaUt / 2;      /* round off to two decimal places */
			/* -1 lets x.0 not round to x.01 */
		else  if (grpfRnd == grpfRndHigh)
			lza += czaUt - 1;

		zu = (int) (lza / ((long)czaUt * 100));	/* Get integral part */
		cch += CchIntToPpch(zu, ppch);	/* Expand integral part */

		/* Long to avoid overflow in subsequent operations. */
		lza -=  ((long)zu * czaUt * 100); 	/* Retain fraction part */
		za = (int)(lza / 10);
		}

	/* note: za now 10 * original za */

	Assert (za < czaUt * 10);

	if (za >= czaUt || za * 10 >= czaUt)
		{ /* Check *10 first because of possible overflow */
		*(*ppch)++ = vitr.chDecimal;
		cch++;
		zu = za / czaUt;
		*(*ppch)++ = '0' + zu;
		cch++;
		zu = ((za - zu * czaUt) * 10) / czaUt;
		if (zu != 0)
			{
			*(*ppch)++ = '0' + zu;
			cch++;
			}
		}

	if (fUnit)
		cch += CchStuff(ppch, mputsz[ut], cchMax - cch);  /* will null term */
	else
		**ppch = 0; /* just to guarantee null term in case it is expected */
	return cch;
}


csconst CHAR szElipses[] = SzKey("...",Elipses);

/* ****
*  Description: Copy sz into ppch. Copy no more than cchMax characters.
*    If all of sz cannot fit, replace the last 4 chars in ppch with "...\0"
*    Since code is counting on this routine to null-terminate the string
*    this will guarantee that a null terminator will be loaded into the string
*    It does require the buffer to be at least 4 characters, even if less than that
*    is available (so the ellipses won't trash anything.
** **** */
/*  %%Function:  CchStuff  %%Owner:  bobz       */

int CchStuff(ppch, sz, cchMax)
CHAR **ppch, sz[];
int cchMax;
{
	register int cch = 0;
	register CHAR *pch = *ppch;

	/* will copy in null term if room */
	while (cchMax-- > 0 && (*pch = *sz++) != 0)
		{
		cch++;
		pch++;
		}

	/* !cchMax case: if we could copy the string but not the null terminator */
	if (cchMax < 0 || (!cchMax && *pch != 0))
		bltbx(szElipses, (pch - (sizeof(szElipses))), sizeof(szElipses));

	*ppch = pch;
	return cch;
}


/*  %%Function:  WParseOpt  %%Owner:  bobz       */

WORD WParseOpt(tmm, sz, ppv, bArg, tmc, opt)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD opt; /* wParam */
{
	int wMin;
	int wMax;
	DPV dpv;

	/* only care about min, max for parse */
	if (tmm == tmmParse)
		{
		if (FUnitOpt(opt))
			{
			wMin = !FNonNegOpt(opt) ? -czaMax : (FNZPosOpt(opt) ? 1:0);
			wMax = czaMax;
			}
		else
			{
			wMin = !FNonNegOpt(opt) ? -32765 : (FNZPosOpt(opt) ? 1:0);
			wMax = 32767;
			}
		}
	else
		{
		wMin = wMax = 0;
		}

	return (WParseOptRange(tmm, sz, ppv, bArg, tmc, opt, wMin, wMax));
}


csconst CHAR stAuto[] = StKey("Auto",AutoDef);


/*  %%Function:  WParseOptRange  %%Owner:  bobz       */

WORD WParseOptRange(tmm, sz, ppv, bArg, tmc, opt, wMin, wMax)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD opt; /* wParam */
int wMin, wMax;
{
	int w;
	DPV dpv;
	WORD ninch;
	int ut;

	ninch = FNonNegOpt(opt) ? uNinch : wNinch;

	Assert(opt != optNil);

	switch (tmm)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif

	case tmmParse:
		if (FAutoOpt(opt) && FCheckForAuto(sz))
			{
			SetPpvBToW(ppv, bArg, 0);
			return fTrue;
			}

		if (FUnitOpt(opt))
			{
			dpv = DpvParseFdxa(&w, tmc, sz, wMin, wMax,
					dpvBlank | dpvSpaces, eidOutOfRange, 
					fTrue, FLineOpt(opt) ? utLine : vpref.ut);
			}
		else
			{
			Assert(FIntOpt(opt));
			dpv = DpvParseFdxa(&w, tmc, sz, wMin, wMax,
					dpvBlank, eidOutOfRange, fFalse, vpref.ut);
			}

		if (dpv == dpvNormal)
			{
			SetPpvBToW(ppv, bArg, w);
			return fTrue;
			}
		else  if (dpv == dpvError)
			/* error reporting already handled */
			/* **ppv is set to something different than the 
				gray value since GetTmcVal pays no attention 
				to the return value */
			{
			SetPpvBToW(ppv, bArg, wError);
			return fFalse;
			}
		else  /* blank or null string - treat as unchanged value */			
			{
			SetPpvBToW(ppv, bArg, ninch);
			return fTrue;
			}
		/* NOT REACHED */

	case tmmFormat:
		w = WFromPpvB(ppv, bArg);

		Assert(w >= 0 || !FNonNegOpt(opt) || w == ninch);

		if (w == ninch)
			{
			*sz = 0;
			}
		else  if (FAutoOpt(opt) && w == 0)
			{
			bltbx((CHAR FAR *) stAuto + 1, (LPSTR) sz, *stAuto);
			sz[*stAuto] = '\0';
			}
		else
			{
			if (FUnitOpt(opt))
				{
				if (FLineOpt(opt))
					/* show whole or half lines as utLine,
						else vpref.ut
					*/
					{
					int wRem = abs(w) % czaLine;
					if (!wRem || wRem == czaLine >> 1)
						ut = utLine;
					else
						ut = vpref.ut;
					}
				else
					ut = vpref.ut;

				CchExpZa(&sz, w, ut, ichMaxNum, fTrue);
				}
			else
				{
				char * pch;

				Assert(FIntOpt(opt));
				pch = sz;
				sz[CchIntToPpch(w, &pch)] = '\0';
				}
			}
		break;

#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif
		}

	return 0;
}


/*  %%Function:  FCheckForAuto  %%Owner:  bobz       */

BOOL FCheckForAuto(sz)
CHAR sz [];
{
	CHAR * pch;
	CHAR * pchEnd;
	int cch;
	CHAR szCopy [ichMaxNum];
	CHAR szAutoCopy [ichMaxNum];


	/* Strip leading white spaces. */
	pch = &sz[0];
	while ((*pch == chSpace || *pch == chTab) && *pch != '\0')
		pch++;

	/* Strip trailing white spaces. */
	pchEnd = pch + CchSz(pch) - 2;
	while (pch <= pchEnd && (*pchEnd == chSpace || *pchEnd == chTab))
		pchEnd--;

	/* pchEnd is pointing one past the desired end. */
	pchEnd++;
	if ((cch = pchEnd - pch) >= ichMaxNum)
		{
		/* Input string too long.  Wimp out and let our authoritative
			parse function handle the situation. */
		return fFalse;
		}

	bltb(pch, szCopy, cch);
	szCopy[cch] = '\0';
	bltbx((char far *) stAuto + 1, (char far *) szAutoCopy, *stAuto);
	szAutoCopy[*stAuto] = '\0';
	return FEqNcSz(szCopy, szAutoCopy);
}


/* NOTE: this function is pretty bogus!  spaces are legal in the middle
of a number! This functionality is also duplicated elsewhere! */
/*-------------------------------------------------------------------
		Purpose:    Parse a string in the given buffer as a number.
		Method:        Scan for digits and ignore white space.
		Returns:    True if parsed as a valid number, else false.
					Character pointer past last one read in *ppch.
					Number parsed is returned in w.  Note that if only a
					prefix is a valid number we return false, with *ppch
					set to the first offending character.
--------------------------------------------------------------mck--*/

#define smInit 0
#define smDig 1
#define smBody 2
#define intPosLast 0x7FFFL

/*  %%Function:  FPwParsePpchPch  %%Owner:  bobz       */

FPwParsePpchPch(pw, ppch, pchMax, pfOverflow)
int *pw;
CHAR **ppch;
CHAR *pchMax;
int *pfOverflow;
{
	char * pch = *ppch;      /* Local buffer pointer */
	unsigned int ch;        /* Character being examined */
	int fNeg = fFalse;
	long lNum = 0;
	BOOL fError = fFalse;
	int sm = smInit;

	*pfOverflow = fFalse;

	while (!fError && !(*pfOverflow) && pch < pchMax)
		{
		ch = *pch;
		if (ch == chSpace)
			pch++;
		else
			switch (sm)
				{
			case smInit:
				if (ch == '-')
					{
					fNeg = fTrue;
					pch++;
					}
				sm = smDig;
				break;

			case smDig:
				if (FDigit(ch))
					sm = smBody;
				else
					fError = fTrue;
				break;

			case smBody:
				if (FDigit(ch))
					{
					/* Overflow? */
					if ((lNum = 10 * lNum + (ch - '0')) > intPosLast)
						*pfOverflow = fTrue;
					else
						pch++;
					}
				else
					fError = fTrue;
				/* break; */
				}
		}

	*ppch = pch;
	*pw = (int) (fNeg ? -lNum : lNum);

	return !fError;
}


/*  %%Function:  DpvParseFdxa  %%Owner:  bobz       */


DPV DpvParseFdxa(pw, tmc, szItem, wLow, wHigh, dpvMask, eid, fDxa, ut)
int * pw;   /* Return value */
TMC tmc;       /* sdm Item number (ignored if tmcNil) */
char * szItem;  /* string to be parsed */
int wLow;      /* Smallest and largest allowed values */
int wHigh;
DPV dpvMask;     /* Bit mask for allowed variations */
int eid;        /* Id of error string if out of range */
int fDxa;      /* Parse as dxa (otherwise int) */
int ut;        /* Units to use as default if fDxa */
{
	char * pch;     /* Parse pointer */
	char * pchEnd;  /* End of buffer */
	char * pchError;/* Position of parse error */
	int fParsed;    /* Parses as number/dxa */
	int fOverflow;	/* True if the number is parsed but it overflow */
	DPV dpvGood;	/* return after good range check */
	int cchItem;     /* length of item string */

	fOverflow = fFalse;
	dpvGood = dpvNormal;

	/* Get the size of the text */
	cchItem = CchSz(szItem) - 1;

	/* See if blank (null line) */
	if ((dpvMask & dpvBlank) && cchItem == 0)
		{
		*pw = valNil;
		return dpvBlank;
		}

	pch = szItem;

	/* See if all spaces  */
	if ((dpvMask & dpvBlank) && (dpvMask & dpvSpaces))
		{
		int ch;
		int fAllSpaces = fTrue;

		while ((ch = *pch++) != 0)
			{
			if (ch != ' ')
				{
				fAllSpaces = fFalse;
				break;
				}
			}

		if (fAllSpaces)
			{
			*pw = valNil;
			return dpvSpaces;
			}
		}

	pch = szItem;
	pchEnd = pch + cchItem;

	/* It parses as a number ... */
	fParsed = fDxa ? FZaFromSs(pw, pch, cchItem, ut, &fOverflow)
			: FPwParsePpchPch(pw, &pch, pchEnd, &fOverflow);

	if (!fDxa && (dpvMask & dpvDouble))
		{
		(*pw) *= 2;
		wLow *= 2;
		wHigh *= 2;
		if (!fParsed)
			{
			/* Check if ".5" was reason for bad parse. */
			if (pch != pchEnd && *pch == vitr.chDecimal)
				{
				pch++;
				/* Allow "ddddd.0*" */
				pchError = pch;
				if (FAllZeroPpchPch(&pchError, pchEnd))
					fParsed = fTrue;
				/* Allow "ddddd.50*" */
				else  if (pch != pchEnd && *pch == '5' &&
						(pch++, FAllZeroPpchPch(&pch, pchEnd)))
					{
					(*pw)++;
					fParsed = fTrue;
					dpvGood = dpvDouble;
					}

				/* Mark furthest error condition */
				else  if (pchError > pch)
					pch = pchError;
				}
			}
		}


	if (fParsed && !fOverflow && *pw >= wLow && *pw <= wHigh)
		return dpvGood;


/* All attempts failed - show user where they went wrong vis the
	attempted number parse. */

	if (tmc != tmcNull && !vmerr.fErrorAlert && !vmerr.fInhibit && !fElActive)
		{
		SetTmcTxs(tmc, TxsOfFirstLim(fParsed ? 0 : pch - szItem, 
				ichLimLast));
		}

	/* overflow will also give a range message */
	if ((fParsed || fOverflow) && (eid == eidOutOfRange || eid == eidDxaOutOfRange))
		{
		if (!fDxa && (dpvMask & dpvDouble))
			{        /* restore original range for error reporting */
			wLow >>= 1;
			wHigh >>= 1;
			}
		RangeError(wLow, wHigh, fDxa, ut);
		}
	else
		ErrorEid(fDxa ? eidNOTDXA : eidNOTNUM, " DpvParseFdxa");

	return dpvError;
}



/*-------------------------------------------------------------------
		Purpose:    Make sure all characters in buffer are spaces or 0's.
		Returns:    *ppch contains first bad character if fFalse returned.
		History:
			10/ 9/84:    Created.
--------------------------------------------------------------mck--*/
/*  %%Function:  FAllZeroPpchPch  %%Owner:  bobz       */

FAllZeroPpchPch(ppch, pchMax)
char ** ppch;        /* Bound of character buffer */
char * pchMax;
{
	char * pch = *ppch;

	while (pch < pchMax)
		{
		if (*pch == '0' || *pch == ' ')
			pch++;
		else
			{
			*ppch = pch;
			return fFalse;
			}
		}

	return fTrue;
}



/**  F  D L G  D I R  A D D  S R C H **/
/* ****
* Description:  Given filename or search spec or partial search spec,
*  add appropriate search string. Used for directory combos and listboxes
*  only.
*
*  Returns false if there was not room to hold the string + search string.
*
*  One assumption is made: that the szEdit is exactly ichMaxText long.
*  We are making the assumption
*  that this is a directory name, adding the search string when possible, and
*  then we will try to open the string as a directory in dlgdirlist. If that
*  fails, we know we have a file name, not a directory. There does not
*  seem to be a more direct way to prove this.
*
*  So, if it is obviously a directory, or may be a directory or file name
*  (or drive name) we set it up as a directory and add the search string.
*
*  We want to add search string to the string - i.e., we are seeing if string
*  szEdit is a directory. If the string is .. or ends in: or \, we know we
*  have a directory name, not a file name, and so add search str to the
*  string. Otherwise, if szEdit has no wildcard characters, add string
*  to the string, even if the string contains a period (directories can have
*  extensions).
*
*  Note the implicit assumption here that \ and not / will be used as the
*  path character. It is held in the defined variable chPath, but we
*  don't handle DOS setups where the / is path and - is the switch character.
* ****  */
/*  %%Function:  FDlgDirAddSrch  %%Owner:  bobz       */

FDlgDirAddSrch(szEdit, szSearch, cchMax)
char *szEdit;
char *szSearch;
int cchMax;
{
	char * pchLast;
	int cbEdit;
	int fAddPathChar;
	char sz [ichMaxBufDlg];

	cbEdit = CchSz(szEdit) - 1;
	Assert(cbEdit < cchMax);

	pchLast = szEdit + cbEdit - 1;

	/* if not ending in \ or : we may have to add a \ */

	fAddPathChar = (*pchLast != chPath && *pchLast != ':');

	if (fAddPathChar && FWildSpec(szEdit))/* leave if already wild card */
		return fTrue;

	/* be sure there is room for search string */
	/* note assumption with fAddPathChar that true == 1, false == 0 */
	Assert(fAddPathChar ? fAddPathChar == 1 : fAddPathChar == 0);

	if ((cbEdit + fAddPathChar + CchSz(szSearch)) >= cchMax)
		return (fFalse);

	if (fAddPathChar)
		*++pchLast = chPath;

	CchCopySz(szSearch, pchLast + 1);
	return fTrue;
}


/*  %%Function:  FWildSpec  %%Owner:  bobz       */

BOOL FWildSpec(sz)
char * sz;
{
	int ichErr;

	return (FntSz(sz, CchSz(sz) - 1, &ichErr, nfoPathWildOK)
			== fntValidWild);
}


#define	lbcEq		((LBC)0)
#define	lbcPrefix	((LBC)1)
#define	lbcLt		((LBC)2)
#define	lbcGt		((LBC)3)

typedef struct
	{
	TMC tmc;
	int cbHeap;
	} EML;


/*  %%Function:  EfMacroList  %%Owner:  bradch */

EfMacroList(lpsy, bsy, stName, peml)
SY FAR *lpsy;
unsigned bsy;
char * stName;
EML * peml;
{
	int iEntry;
	int cb;
	LBC lbc;
	
	cb = stName[0] + 25; /* 25 is a "random" estimate of the overhead */

	if (peml->cbHeap < cb && (peml->cbHeap = CbAvailHeap(sbDds)-1024) < cb)
		{
		ErrorEid(eidLowMemLBox, "EfMacroList");
		return fFalse;
		}

	StToSzInPlace(stName);
	lbc = lbcLt;
	iEntry = IEntryFindListBox(peml->tmc, stName, &lbc);
	if (lbc != lbcEq)
		{
		InsertListBoxEntry(peml->tmc, stName, iEntry);
		peml->cbHeap -= cb;
		}
	
	return fTrue;
}


/*  %%Function:  FillMacroListTmc  %%Owner:  bradch       */

FillMacroListTmc(tmc, mct)
TMC tmc;
int mct;
{
	EML eml;
	
	eml.tmc = tmc;
	eml.cbHeap = 0;
	StartListBoxUpdate(tmc);
	FEnumMacros(EfMacroList, &eml, mct);
	EndListBoxUpdate(tmc);
}


/* B C M  C U R  S E L */
/*  %%Function:  BcmCurSelTmc  %%Owner:  bobz       */

BcmCurSelTmc(tmc)
TMC tmc;
{
	char st [cchMaxSyName + 2];

	st[0] = CchGetTmcText(tmc, st + 1, cchMaxSyName);

	return BcmOfSt(st);
}


/*  %%Function:  FGetDirListEntry  %%Owner:  bobz       */

BOOL FGetDirListEntry(tmc, szBuf, ichMax)
TMC tmc;
char * szBuf;
int ichMax;
{
	char * pchStart;
	int cch;

	cch = CchGetTmcText(tmc, szBuf, ichMax);
	/* we assume that this is a list box of only directories and drives */
	/* strip off brackets and - */
	if (cch == 0 || szBuf[0] != '[')
		{
#ifdef BZ
		CommSzSz(SzShared("FGetDirListEntry fail string: "), szBuf);
		CommSzNum(SzShared("FGetDirListEntry fail cch: "), cch);
#endif /* BZ */

		return fFalse;
		}
	/* Selection is drive */
	if (szBuf[1] == '-')
		{
		pchStart = szBuf + 2;  /* skip over leading [- */
		szBuf[3] = ':';
		szBuf[4] = 0;
		cch -= 1;
		}
	else  /* directory */		
		{
		Assert(cch > 2);
		pchStart = szBuf + 1;  /* skip over leading [ */
		szBuf[cch - 1] = '\\';
		}

	bltbyte(pchStart, szBuf, cch);
	return fTrue;
}


/* F T R U N C A T E  T M C  PATH P S Z  */
/* If a filename placed in tmc (generally static text) would be too
	long, truncate it and add elipises to front. Updates pch to point
	to new start of string.    Returns TRUE if truncated.
*/
/*  %%Function:  FTruncateTmcPathPsz  %%Owner:  bobz       */

BOOL FTruncateTmcPathPsz(tmc, ppch)
TMC tmc;
CHAR **ppch;
{
	int cchTmc;
	struct RC rc;

	/* get rect for tmc in windows coords */
	if (FIsDlgDying())	/* gettmcrec will return 0 then */
		return fFalse;

	GetTmcRec(tmc, &rc);

	if (vsci.dxpTmWidth)
		cchTmc = (rc.xpRight - rc.xpLeft) / vsci.dxpTmWidth;
	else
		{
		cchTmc = (rc.xpRight - rc.xpLeft) / 8; /* TOTALLY abritrary guess bz */
		Assert (fFalse);
		}

	if (cchTmc <= 6)
		return (fFalse);

	return (FTruncatePathCbPsz(cchTmc, ppch));
}


/* F T R U N C A T E  P A T H  C B  P S Z  */
/* If a filename would be too
	long, truncate it and add elipises to front. Updates pch to point
	to new start of string.    Returns TRUE if truncated.
*/
/*  %%Function:  FTruncatePathCbPsz  %%Owner:  bobz       */

BOOL FTruncatePathCbPsz(cbBuf, ppch)
int cbBuf;
CHAR **ppch;
{
	int cchTmc, cch;
	struct RC rc;
	CHAR chDrive;
	CHAR *pch = *ppch;

	if ((cch = (CchSz(pch)-1)) > cbBuf)
		{
		chDrive = *pch;
		Assert(cbBuf > 6);
		cbBuf -= 6;  /* to leave room for ellipses, etc */
		while (cbBuf < cch)
			{
			while (cch-- > 0 && *pch++ != '\\')
				;
			}
		/* prepend <drive>:\... */
		if (cch >= 0)
			pch--; /* backup over backslash if found */
		*--pch = '.';
		*--pch = '.';
		*--pch = '.';
		*--pch = '\\';
		*--pch = ':';
		*--pch = chDrive;
		*ppch = pch;
		return fTrue;
		}

	return fFalse;
}



/*  %%Function:  FGtStyleNames  %%Owner:  bobz       */

FGtStyleNames(stcp1, stcp2)
int stcp1, stcp2;
{
	struct STSH stsh;
	char stName1[cchMaxStyle];
	char stName2[cchMaxStyle];

	Assert(vdocStsh != docNil);
	stsh = (PdodDoc(vdocStsh))->stsh;
	GenStyleNameForStcp(stName1, stsh.hsttbName, stsh.cstcStd, stcp1);
	GenStyleNameForStcp(stName2, stsh.hsttbName, stsh.cstcStd, stcp2);
	return(WCompSt(stName1, stName2) > 0 ? TRUE : FALSE);
}


/* fAll: list all standard styles */
/* fFillMap: vhmpiststcp is valid, fill map with style names.   If this
*  is fFalse, only vistLbMac will be updated. */
/*  %%Function:  GenLbStyMapForDoc  %%Owner:  bobz       */

GenLbStyMapForDoc(doc, fAll, fFillMap)
{
	char *mpiststcp;
	struct DOD *pdod;
	struct STTB **hsttb;
	int ibstMac;
	int stcp, stcpStart, cstcStd, stcpLev3, stcpLev1;

	if (fFillMap)
		{
		Assert(vhmpiststcp != hNil);
		mpiststcp = *vhmpiststcp;
		}
	vistLbMac = 0;
	pdod = PdodDoc(doc);
	cstcStd = pdod->stsh.cstcStd;
	hsttb = pdod->stsh.hsttbName;
	ibstMac = (*hsttb)->ibstMac;
	/* don't use StcpFromStc...need to use negative number if
		that's the way it works out! */
	stcpLev3 = cstcStd - (cstcMax - stcLev3);
	stcpLev1 = stcpLev3 + 2;
	stcpStart = fAll ? (stcStdMin - (cstcMax - 1) + cstcStd) :
			(stcpLev3 < 0 ? stcpLev3 : 0);
	for (stcp = stcpStart; stcp < ibstMac; stcp++)
		{
		if ((vstcBackup == stcNil || stcp != StcpFromStc(vstcBackup,cstcStd)) &&
				((fAll && stcp < cstcStd) ||
				(stcp >= stcpLev3 && stcp <= stcpLev1) ||
				(stcp >=0 && !FStcpEntryIsNull(hsttb, stcp))))
			{
			if (fFillMap)
				mpiststcp[vistLbMac++] = stcp;
			else
				vistLbMac++;
			}
		}
	if (fFillMap)
		SortRgb(mpiststcp, vistLbMac, FGtStyleNames);
}


/*  %%Function:  GenLbStyleMapping  %%Owner:  bobz       */

GenLbStyleMapping()
{
	GenLbStyMapForDoc(vdocStsh, vfShowAllStd, fTrue);
}


/* F E N U M  M A C R O S */
/* return false in OOM problems */

/* %%Function:FEnumMacros %%Owner:krishnam */
FEnumMacros(pfn, wParam, mct)
int (*pfn)();
int wParam;
MCT mct;
{
	extern HPSYT vhpsyt;
	extern HWND	 vhwndCBT;
	extern int	 docGlobalDot;
	BOOL fWantCmds, fWantGlobalMacros, fWantTemplateMacros;
	uns bsy, bsyMac;
	int docDot;
	SY FAR *lpsy;
	SYT FAR *lpsyt;
	char FAR * lpgrpsy;
	char stName [cchMaxSyName];

	docDot = selCur.doc == docNil ? docNil : DocDotMother(selCur.doc);

	lpsyt = LpLockHp(vhpsyt);

	fWantCmds = fWantGlobalMacros = fWantTemplateMacros = fFalse;
	switch (mct)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
#endif

	case mctAll:		/* GLOBAL, TEMPLATE, COMMANDS */
		fWantCmds = fWantGlobalMacros = fWantTemplateMacros = fTrue;
		break;

	case mctMacros:		/* GLOBAL, TEMPLATE */
		fWantGlobalMacros = fTrue;
		/* FALL THROUGH */

	case mctMacro:		/* TEMPLATE */
		fWantTemplateMacros = fTrue;
		break;

	case mctGlobal:		/* GLOBAL, COMMANDS */
		fWantCmds = fTrue;
		/* FALL THROUGH */

	case mctGlobalMacro:	/* GLOBAL */
		fWantGlobalMacros = fTrue;
		break;
		}

		/* BLOCK */
		{
		lpgrpsy = lpsyt->grpsy;
		bsyMac = lpsyt->bsy;
		bsy = (mct == mctMacros) ? bcmMacStd : 0;
		for ( ; bsy < bsyMac; bsy += CbLpsy(lpsy))
			{
			MCT mctT;
			lpsy = (SY FAR *) (lpgrpsy + bsy);
			mctT = lpsy->mct;

			switch (mctT)
				{
#ifdef DEBUG
			default:
				Assert(fFalse);
#endif

			case mctNil:
			case mctEl:
				continue;

			case mctCmd:
			case mctSdm:
				if (!fWantCmds)
					continue;
				break;

			case mctGlobalMacro:
				if (!fWantGlobalMacros ||
		/* below case can happen with a macro that was in normal.dot while we
   are in tutorial; we switch to cbtnorm.dot for global template, but
   old macros (which we don't want to display) are still in the
		   command table */
				vhwndCBT && lpsy->docDot != docGlobalDot)
					{
					continue;
					}
				break;

			case mctMacro: /* Template macro */
				if (!fWantTemplateMacros || 
						lpsy->docDot != docDot)
					continue;
				break;
				}

			if (lpsy->stName[0] == 0)
				{
				uns bcm;
				char FAR * lpst;

				bcm = *((int FAR *) (lpsy->stName + 1));
				lpst = ((SY FAR *)
						(lpsyt->grpsy + bcm))->stName;
				bltbx(lpst, (char FAR *) stName, *lpst + 1);
				}
			else
				{
				bltbx(lpsy->stName, (char FAR *) stName,
						lpsy->stName[0] + 1);
				}

			if (!(*pfn)(lpsy, bsy, stName, wParam))
				break;
			if (vmerr.fMemFail)  /* if in dialog, dialog probably destroyed */
				break;
			}
		}

	UnlockHp(vhpsyt);
	return (!vmerr.fMemFail);
}


/*  %%Function:  WParseDttm  %%Owner:  bobz       */

WParseDttm(tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	int cch;
	DPV dpv;
	struct DTTM * pdttm;

	pdttm = ((struct DTTM *) PvParseArg(ppv, bArg));

	switch (tmm)
		{
	case tmmParse:
		if ((cch = CchStripString(sz, CchSz(sz) - 1)) == 0)
			{
			*(long *) pdttm = dttmNinch;
			return fTrue;
			}

		if (!FDttmFromSzPl(sz, pdttm, (long *) NULL))
			{
			ErrorEid(eidInvalidDate, "WParseDttm");

			if (!fElActive)
				SetTmcTxs(tmc, TxsOfFirstLim(0, cch));

			*(long *) pdttm = dttmError;
			return fFalse;
			}

		return fTrue;

	case tmmFormat:
		/* no error reporting for this case */
		if (*(long *) pdttm == dttmNinch)
			*sz = 0;
		else
			{
			PdttmToSt(pdttm, sz, ichMaxBufDlg);
			StToSzInPlace(sz);
			}
		break;

#ifdef tmmCwVal 
	case tmmCwVal:
		Assert(sizeof (struct DTTM) == sizeof (long));
		return (CwFromCch(sizeof (struct DTTM)));
#endif
		}

	return 0;
}


/*  %%Function: FDisableTemplate  %%Owner: peterj  */

FDisableTemplate(doc)
int doc;
{
	int docDot;

	return (doc == docNil || doc == docGlobalDot || 
			PdodDoc(doc)->fLockForEdit ||
			(docDot = DocDotMother(doc)) == docNil ||
			docDot == docGlobalDot ||
			PdodDoc(docDot)->fLockForEdit);
}


