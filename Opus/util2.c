/* U T I L 2 . C */
/*  Non-core utilities */

#define NOCTLMGR
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOMENUS
#define NOCOMM
#define NOSOUND
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "disp.h"
#include "ch.h"
#include "doc.h"
#include "doslib.h"
#include "props.h"
#include "sel.h"
#include "file.h"
#include "inter.h"
#include "prm.h"
#include "sorttbl.h"
#include "ourmath.h"
#include "keys.h"
#include "wininfo.h"
#include "debug.h"
#include "field.h"
#include "print.h"
#include "prompt.h"
#include "message.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmparse.h"
#include "sdmtmpl.h"
#include "idd.h"
#include "tmc.h"

#include "prompt.hs"
#include "prompt.sdm"


extern BOOL		fElActive;
extern int		docGlobalDot;
extern HANDLE           vhInstance;
extern int         vcxtHelp;
extern HCURSOR     vhcArrow;
extern HWND        vhwndApp;
extern HWND vhwndStartup;
extern CHAR        vstPrompt[];
extern int         vpisPrompt;
extern struct PIR *vppirPrompt;
extern HWND        vhwndPrompt;
extern struct DOD       **mpdochdod[];
extern struct ITR       vitr;
extern int              vfMouseExist;
extern ENV		*penvMathError;
extern struct WWD       **hwwdCur;
extern int              wwCur;
extern struct SEL	selCur;
extern struct MERR      vmerr;
extern struct PRI       vpri;
extern struct PREF      vpref;
extern KMP **      hkmpCur;
extern BOOL		vfElOom;
extern int		vwWinVersion;

CHAR *PchFillPchMst( /*- PCH, mst, int -*/ );


EXPORT FAR PASCAL MathError(); /* DECLARATION ONLY */


/* **** +-+utility+-+string **** */

/*---------------------------------------------------------------------------
-- Routine: WCompSz(psz1,psz2)
-- Description and Usage:
	Alphabetically compares the two null-terminated strings lpsz1 and lpsz2.
	Upper case alpha characters are mapped to lower case.
	Comparison of non-alpha characters is by ascii code.
	Returns 0 if they are equal, a negative number if lpsz1 precedes lpsz2, and
		a non-zero positive number if lpsz2 precedes lpsz1.
-- Arguments:
	psz1, psz2  - pointers to two null-terminated strings to compare
-- Returns:
	a short     - 0 if strings are equal, negative number if lpsz1 precedes lpsz2,
		and non-zero positive number if psz2 precedes psz1.
-- Side-effects: none
-- Bugs:
-- History:
	3/14/83     - created (tsr)
----------------------------------------------------------------------------*/
/*  %%Function: WCompSz  %%Owner: bobz  */

WCompSz(psz1,psz2)
register PCH psz1;
register PCH psz2;
{
	/* does not go through the sort tables! */

	int ch1;
	int ch2;

	for (ch1=ChUpper(*psz1++), ch2=ChUpper(*psz2++); ch1==ch2; ch1=ChUpper(*psz1++), ch2=ChUpper(*psz2++))
		{
		if (ch1 == '\0')
			return(0);
		}

	return(ch1-ch2);

} /* end of  W C o m p S z   */

#ifdef DEBUG
CkSortOrdinals()
{
	int chT, ordN, ordC;

	if (vwWinVersion < 0x0300)
		{
		for (chT = 0; chT < 4*256; chT++)
			{
			ordN = N_OrdCh(chT & 0x00FF, chT & 0x0100, chT & 0x0200);
			ordC = C_OrdCh(chT & 0x00FF, chT & 0x0100, chT & 0x0200);
			Assert (ordN == ordC);
			}
		}
}
#endif /* DEBUG */

#ifdef DEBUG
C_OrdCh(ch, fCase, fScan)
int ch;
int fCase;
int fScan;
{
	CHAR FAR *mpchord = fScan ? mpchordScan : mpchordNorm;
	if (!fCase)
		ch = ChLower(ch);
	return (mpchord[ch]);
}
#endif /* DEBUG */

/* **** +-+utility+-+string **** */

#ifdef DEBUG
/*---------------------------------------------------------------------------
-- Routine: WCompSzSrt(psz1,psz2,fCase)
-- Description and Usage:
	Alphabetically compares the two null-terminated strings lpsz1 and lpsz2.
	Upper case alpha characters are mapped to lower case iff fCase
	Comparison of non-alpha characters is by ascii code.
	Returns 0 if they are equal, a negative number if lpsz1 precedes lpsz2, and
		a non-zero positive number if lpsz2 precedes lpsz1.
	If 2 strings compare equal, we do an ansi compare and return that value
		so we handle Apple befor apple and strings identical except
		for accents (intl).
    We now set negative values so that prefixes and < are negative but
        distinct, so callers who care about prefix get the info, but
        others can just check < 0.
-- Arguments:
	psz1, psz2  - pointers to two null-terminated strings to compare
-- Returns:
	a short     - 0 if strings are equal, -2 if psz1 precedes psz2,
        -1 if psz1 is a prefix of psz2,
		and non-zero positive number if psz2 precedes psz1.
-- Side-effects: none
-- Bugs:
-- History:
	3/14/83     - created (tsr)
----------------------------------------------------------------------------*/
/*  %%Function: WCompSzSrt  %%Owner: bobz  */

HANDNATIVE C_WCompSzSrt(psz1,psz2,fCase)
register PCH psz1;
register PCH psz2;
int fCase;
{
	int ord1;
	int ord2;
	int dch;
	PCH pszOrig1 = psz1;
	PCH pszOrig2 = psz2;
	CHAR FAR *mpchord = vitr.fScandanavian ? mpchordScan : mpchordNorm;

#ifdef DBGSORTTBL
	Assert(sizeof(mpchordScan) == 256);
	Assert(sizeof(mpchordNorm) == 256);
	CheckMpchord(mpchord);
#endif

	/* Note: ChLower used intentionally, rather than ChUpper so that
		French comparisons use the sort table values, not the stripped
		upper case values bz
	*/

	for (ord1 = mpchord[fCase ? *psz1++ : ChLower(*psz1++)],
	     ord2 = mpchord[fCase ? *psz2++ : ChLower(*psz2++)];
	     ord1 == ord2;
	     ord1 = mpchord[fCase ? *psz1++ : ChLower(*psz1++)],
	     ord2 = mpchord[fCase ? *psz2++ : ChLower(*psz2++)])
		{
		if (ord1 == '\0')
			{
			psz1 = pszOrig1;
			psz2 = pszOrig2;
			for (ord1=*psz1++, ord2=*psz2++; ord1==ord2; ord1=*psz1++, ord2=*psz2++)
				{
				if (ord1 == '\0')
					return(0);
				}
			break;
			}
		}
	if ((dch = ord1 - ord2) > 0)
		return dch;
	else  if (ord1 != '\0')
		return -2;  /* psz1 < psz2 */
	else
		return -1;  /* != but *psz1 == 0; psz1 is a prefix of psz2 */

} /* end of  W C o m p S z Srt  */
#endif /* DEBUG */

#ifdef DEBUG
/* W  C O M P  C H  C H */
/*  Compare two characters using the table in this module.  Return zero
	if ch1 == ch2, negative if ch1 < ch2, and positive if ch1 > ch2.
*/
/*  %%Function: WCompChCh   %%Owner: bobz  */

HANDNATIVE C_WCompChCh (ch1, ch2)
CHAR ch1, ch2;

{
	CHAR FAR *mpchord = vitr.fScandanavian ? mpchordScan : mpchordNorm;
	return mpchord[ch1] - mpchord[ch2];
}
#endif /* DEBUG */


#ifdef NOASM
/* L b c  C m p  L b o x */
/* from lbox.h */
#define	lbcEq		0
#define	lbcPrefix	1
#define	lbcLt		2
#define	lbcGt		3

/*  Callback compare rtn for sdm listboxes
    Compare two characters using the table in this module.
    Return lbc codes above
*/
/*  %%Function: LbcCmpLbox   %%Owner: bobz  */

HANDNATIVE LbcCmpLbox (wUser, pstz1, pstz2)
WORD wUser;
CHAR **pstz1;
CHAR **pstz2;
{
	int w;

	w = WCompSzSrt(*pstz1 + 1,*pstz2 + 1, fFalse);	/* skip over st size */
	if (w == 0)
		{
		return (lbcEq);
		}
	else  if (w > 0)
		return (lbcGt);
	else  if (w == -2)
		return (lbcLt);
	else
		{
		Assert (w == -1);
		return (lbcPrefix);
		}
}
#endif /* NOASM */



/* C C H   I N T   T O   A S C I I   H E X */
/*  %%Function: CchIntToAsciiHex  %%Owner: bobz  */

int CchIntToAsciiHex(n, ppch, cLevel)
unsigned n;          /* number to convert to hex */
char **ppch;
int  cLevel;
	{ /*
	DESCRIPTION:
	Convert integer n into ascii hex.  Output is stuffed into
	**ppch and (*ppch) is incremented by the number of hex digits
	that were necessary to represent the number.
	RETURNS:
	Length of output (number of hex digits).
*/
	int cch = 0;
	cLevel--;
	if (n >= 0x10 || cLevel > 0)
		{
		cch += CchIntToAsciiHex(n / 0x10, ppch, cLevel);
		n %= 0x10;
		}

	if (n < 0xa)
		*(*ppch)++ = '0' + n;
	else
		*(*ppch)++ = 'A' + (n - 10);
	return(cch + 1);

}       /* end CchIntToAsciiHex */



/* C C H  U N S  T O  P P C H  L E V E L */
/*  Places ascii decimal form of n into ppch.  On return ppch points to
	character following last written.  Puts out leading zeros to fill at
	least cLevel digits.
*/
/*  %%Function: CchUnsToPpchLevel   %%Owner: bobz  */


int CchUnsToPpchLevel (n, ppch, cLevel)
unsigned n;
CHAR **ppch;
int cLevel;

{
	int cch = 1;
	cLevel--;
	if (n >= 10 || cLevel > 0)
		{
		cch += CchUnsToPpchLevel (n/10, ppch, cLevel);
		n %= 10;
		}
	*(*ppch)++ = '0' + n;
	return cch;
}


/* **** +-+utility+-+string **** */

/* ****
*  Description: Returns TRUE if ch is a space, FALSE otherwise.
*   "Space" is defined as in standard C library: x09-x0d, x20
** **** */
/*  %%Function: FSpace  %%Owner: peterj  */

FSpace(ch)
uns ch;
{
	return((ch==0x20) || 
			((uns)(ch - 0x09) <= (0x0d - 0x09)));
}


/*  %%Function: FAuthor  %%Owner: chic  */


FAuthor(doc)
int doc;
{
	CHAR stAuthor[cchMaxSt];
	GetStFromSttb(PdodDoc(doc)->hsttbAssoc, ibstAssocAuthor, stAuthor);
	return(!FNeSt(&vpref.stUsrName[0], stAuthor));
}


/* F  V A L I D  E X T */
/* Is it a valid file extension (".xxx")? */
/*  %%Function: FValidExt  %%Owner: peterj  */

BOOL FValidExt(szExt)
CHAR    *szExt;
{
	if (CchSz(szExt) > ichMaxExtension)
		{
		return fFalse;
		}
	else  if (*szExt++ != '.')
		{
		return fFalse;
		}

	while (FAlphaNum(*szExt++));

	return (*(szExt - 1) == '\0');
}





/*  A S S U R E  C P  A B O V E  D Y P  */
/*  Mostly for use with Hyphenation and Replace, this function will make
	sure the given cp is above the given dyp on the screen.  If it's
	already there, nothing happens.  If it is not currently on the screen
	or if it is below the requested dyp, it scrolls to the top dl (not
	the requested dyp).  This is because it is assumed that the caller
	will need to keep the found cp visible in a certain portion of the
	screen and will not want to keep calling this function if it can
	be avoided.  Putting the cp at dyp = 0 will optimize the case where
	there are frequent calls to AssureCpAboveDyp per screenful.  Note,
	this function also forces the cp to be visible horizontally.
*/
/*  %%Function: AssureCpAboveDyp  %%Owner: rosiep  */

AssureCpAboveDyp(cp, dyp, fEnd)
CP cp;
int dyp;
BOOL fEnd;
{
	struct PLCEDL **hplcedl;
	struct PLDR **hpldr;
	struct EDL edl;
	CP cpFirst;
	int dl, idr, xw;
	BOOL fT;
	struct DRF drfFetch;

	if ((*hwwdCur)->fDirty /* || (*hwwdCur)->fPageView */)
		UpdateWw(wwCur, fFalse);

	if ((dl = DlWhereDocCp(wwCur, selCur.doc, cp, fEnd, &hpldr, &idr,
			&cpFirst, &fT, fTrue)) >= 0)
		{
		hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
		GetPlc(hplcedl, dl, &edl);
		FreePdrf(&drfFetch);
		if (YwFromYp( hpldr, idr, edl.ypTop + edl.dyp ) <= 
				(*hwwdCur)->ywMin + dyp)
			{
			xw = XwFromWwCp(wwCur, hpldr, idr, selCur.doc, cp, cpFirst);
			if (xw < (*hwwdCur)->xwMin || xw >= (*hwwdCur)->xwMac)
				goto LNorm;
			return;
			}
		}

LNorm:
	NormCp(wwCur, selCur.doc, cp, ncpForceYPos | ncpHoriz, 0, fEnd);
}



csconst CHAR mpmonddom[][] =
/* 1   2   3   4   5   6   7   8   9  10  11  12 */
{{ 
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};


/* The number of days from December 31, 1899, Sunday. */
/*  %%Function: LDaysFrom1900Dttm  %%Owner: peterj  */

long LDaysFrom1900Dttm(dttm)
struct DTTM dttm;
{
	int           imon, imonCur;
	long          lDays;
	CHAR FAR     *rgddom;

	lDays = 0L;
	rgddom = (CHAR FAR *) &mpmonddom[FLeapYr(dttm.yr + dyrBase)][0];
	imonCur = dttm.mon - 1;
	for (imon = 0; imon < imonCur; lDays += rgddom[imon++]);
	lDays += dttm.dom;

	if (dttm.yr != 0)
		{
		int yr;
		int cLeapYear;

		yr = dttm.yr - 1;
		if (yr + dyrBase < 2000)  /* year 1900 through 1999 */
			{
			cLeapYear = yr / 4;
			}
		else
			{
			yr += dyrBase - 2000;
			cLeapYear = 25 + (yr / 400) + 24 * (yr / 100) + ((yr % 100) / 4);
			}
		lDays += 365 * ((long) dttm.yr) + cLeapYear;
		}
	return (lDays);
}


/*  %%Function: LDaysFromDttm  %%Owner: peterj  */


long LDaysFromDttm(dttmFrom, dttmTo)
struct DTTM dttmFrom, dttmTo;
{
	return (LDaysFrom1900Dttm(dttmTo) - LDaysFrom1900Dttm(dttmFrom));
}


/*  %%Function: CMinutesFromDttms  %%Owner: peterj  */

int CMinutesFromDttms(dttmFrom, dttmTo)
struct DTTM dttmFrom, dttmTo;
{
	int           cMinFrom, cMinTo;
	long          lDays;


	lDays = LDaysFromDttm(dttmFrom, dttmTo);
	cMinFrom = dttmFrom.hr * 60 + dttmFrom.mint;
	cMinTo   = dttmTo.hr * 60 + dttmTo.mint;

	if (lDays == 0)
		{
		return (cMinTo - cMinFrom);
		}
	else
		{
		lDays--;
		return (60 * 24 * lDays + cMinTo + ((60 * 24) - cMinFrom));
		}
}


/*  %%Function: FLeapYr  %%Owner: peterj  */

BOOL FLeapYr(yr)
int	    yr;
{
	return (((yr & 0x0003) == 0 && yr % 100 != 0) || yr % 400 == 0);
}


#define dowOrigin     0   /* 12/31/1899 was Sunday. */
/*  %%Function: DowFromDttm  %%Owner: peterj  */

int DowFromDttm(dttm)
struct DTTM dttm;
{
	extern long LDaysFrom1900Dttm();

	return ((dowOrigin + LDaysFrom1900Dttm(dttm)) % 7);
}


/*  %%Function: FValidDttm   %%Owner: peterj  */


BOOL FValidDttm (dttm)
struct DTTM dttm;
{
	int         yr;

#ifdef DISABLE
	CommSzLong(SzShared("FValidDttm: dttm = "), dttm);
#endif

	yr = dttm.yr + dyrBase;
	return ((0 <= dttm.mint           && dttm.mint < 60)             &&
			(0 <= dttm.hr             && dttm.hr   < 24)             &&
			(dttmYrFirst <= yr        && yr <= dttmYrLast)           &&
			(dttmMonFirst <= dttm.mon && dttm.mon <= dttmMonLast)    &&
			(1 <= dttm.dom            &&
			dttm.dom  <= mpmonddom[FLeapYr(yr)][dttm.mon - 1]) &&
			(0 <= dttm.wdy            && dttm.wdy <= 6));
}


/* **** +-+utility+-string **** */

/* ****
*  Description: Return a pointer to the first character in the string
*      at pch that is either null or non-whitespace.
** **** */
/*  %%Function: PchSkipSpacesPch  %%Owner: rosiep  */


CHAR *PchSkipSpacesPch( pch )
CHAR *pch;
{
	for ( ;; )
		switch (*pch) 
			{
		default:
			return pch;
		case ' ':
		case 0x09:
			pch++;
			break;
			}
}




/* **** +-+utility+-+string **** */

/* ****
*  Description: Append sz2 onto sz1.  Caller assumes responsibility for space
*      requirements in sz1.
** **** */
/*  %%Function: SzSzAppend  %%Owner: rosiep  */


SzSzAppend( sz1, sz2 )
CHAR sz1[], sz2[];
{
	char *pch;
	int cch = CchSz( sz2 );

	Assert( cch >= 1 );

	for (pch = sz1; *pch != 0; pch++)
		;
	bltbyte( sz2, pch, cch );
}


/* H S T  C R E A T E */
/* Creates a heap block containing st */

/* F R E E  U N U S E D  P R C S */
/*  %%Function: FreeUnusedPrcs  %%Owner: davidlu  */

FreeUnusedPrcs()
{
	extern int docMac;
	extern struct DOD **mpdochdod[];
	extern struct PRC **vhprc;

	int doc;
	struct DOD *pdod, **hdod;
	struct PLC **hplcpcd;
	int ipcd;
	struct PCD *ppcd;

	struct PRC *pprc;
	struct PRC **hprc;
	struct PRC **hprcCur;
	struct PRC ***phprcPrev;
	BOOL fNoRoom; /* dummy */
	extern BOOL vfInCommit;

	/* make sure we can never get in here during the second half of
		bifurcated operation */
	Assert(!vfInCommit);

	/* if there are no PRCs allocated it's pointless to go on */
	if (vhprc == 0 || !vmerr.fReclaimHprcs)
		return;

	/* scan thru the document descriptors */
	for (doc = 1; doc < docMac; doc++)
		{
		if (hdod = mpdochdod[doc])
			/* if the descriptor is allocated ... */
			{
			int ipcdMac;
			struct PCD pcd;
			pdod = *hdod;
			/* point to the document's piece table */
			if ((hplcpcd = pdod->hplcpcd) == 0)
				continue;
			ipcdMac = IMacPlc(hplcpcd);
			/* scan thru the piece descriptors in the piece table*/
			for (ipcd = 0; ipcd < ipcdMac; ipcd++)
				{
				/* if the prm stored in the piece is complex,
					find the PRC in the heap that the prm points
					to, and set the PRC's fRef flag */
				GetPlc(hplcpcd, ipcd, &pcd);
				if (((struct PRM *)&pcd.prm)->fComplex)
					{
					pprc = *HprcFromPprmComplex((struct PRM *)&pcd.prm);
					pprc->fRef = fTrue;
					}
				}
			}
		}

	if (vmerr.prmDontReclaim != prmNil && 
			((struct PRM *)&vmerr.prmDontReclaim)->fComplex)
		{
		pprc = *HprcFromPprmComplex((struct PRM *)&vmerr.prmDontReclaim);
		pprc->fRef = fTrue;
		}


	/* Now scan thru the list of PRCs chained from vhprc. */
	hprc = vhprc;
	phprcPrev = &vhprc;

	/* loop while hprc is non-nil */
	while (hprc)
		{
		pprc = *hprc;
		/* save hprc value in case we have to free the PRC block */
		hprcCur = hprc;
		/* save the pointer to the next PRC block */
		hprc = pprc->hprcNext;

		if (pprc->fRef)
			{
			/* if the current PRC has a reference within a piece
				store a pointer to the hprc field  of the current
				PRC and reset the fRef flag */
			phprcPrev = &pprc->hprcNext;
			pprc->fRef = fFalse;
			}
		else
			{
			/* else remove the current PRC from the list, and
				free the PRC block. */
			*phprcPrev = pprc->hprcNext;
			FreeH(hprcCur);
			}
		}
	vmerr.fReclaimHprcs = fFalse;
}





#ifdef DEBUGFAST
/*  %%Function: CommSzFast  %%Owner: rosiep  */

CommSzFast(sz)
CHAR	*sz;
{
	extern HWND vhwndMsgBoxParent;
	MessageBox(vhwndMsgBoxParent, (LPSTR) sz, (LPSTR) NULL, MB_OK);
}


/*  %%Function: CommSzSzFast  %%Owner: rosiep  */

CommSzSzFast(sz1, sz2)
CHAR	*sz1, *sz2;
{
	CHAR	*pch, rgchT[128];

	pch = rgchT + CchCopySz(sz1, &rgchT[0]);
	CchCopySz(sz2, pch);
	CommSzFast(rgchT);
}


/*  %%Function: CommSzNumFast  %%Owner: rosiep  */

CommSzNumFast(sz, w)
CHAR	*sz;
int	w;
{
	CHAR	*pch, rgchT[128];

	pch = rgchT + CchCopySz(sz, &rgchT[0]);
	CchIntToPpch(w, &pch);
	*pch = 0;
	CommSzFast(rgchT);
}


/*  %%Function: CommSzPnumFast  %%Owner: rosiep  */

CommSzPnumFast(sz, pnum)
CHAR *sz;
NUM *pnum;
{
	CHAR	*pch;
	struct UPNUM upnum;
	CHAR	rgchT[128];

	LdiNum(pnum);
	UnpackNum(&(upnum.wExp), &(upnum.wSign), upnum.rgch);
	pch = rgchT + CchCopySz(sz, &rgchT[0]);
	pch += CchCopySz(SzShared(" s: "), pch);
	CchIntToPpch(upnum.wSign, &pch);
	pch += CchCopySz(SzShared(" m: "), pch);
	bltbyte(&upnum.rgch[1], pch, (int) (upnum.rgch[0]));
	pch += upnum.rgch[0];
	pch += CchCopySz(SzShared(" e: "), pch);
	CchIntToPpch(upnum.wExp, &pch);
	*pch = '\0';
	CommSzFast(rgchT);
}


#endif /* DEBUGFAST */

/* ****
*  Description: Append sz onto st.  Caller assumes responsibility for space
*      requirements in st.
** **** */
/*  %%Function: StSzAppend  %%Owner: rosiep  */

StSzAppend( st, sz )
CHAR st[];
CHAR sz[];
{

	int cch = CchSz( sz ) - 1;

	Assert( cch >= 0 );
	Assert( cch + st [0] < 256 );

	bltbyte( sz, &st [ st [0] + 1 ], cch );
	st [0] += cch;
}


/* ****
*  Description: Beep once.  Don't beep again until vfAwfulNoise is
*               reset in Idle() or otherwise.
** **** */
/*  %%Function: Beep  %%Owner: bryanl  */

Beep()
{
#define MB_STANDARD (MB_OK | MB_SYSTEMMODAL | MB_ICONEXCLAMATION)
	extern int vfAwfulNoise;

	if (!vfAwfulNoise && !fElActive)
		{
		vfAwfulNoise = fTrue;
#ifdef BRYANL	/* Artificial ears; Bryan's home Compaq doesn't beep */
			{
			extern CHAR szEmpty[];
			CommSzSz( SzShared( "BEEEP!!" ),szEmpty );
			}
#endif
		MessageBeep( MB_STANDARD );
		}
}


#ifdef DBGSORTTBL
/*  %%Function: CheckMpchord  %%Owner: bobz  */

CheckMpchord(mp)
CHAR    mp[];
{
#define CHMAX   256
	BOOL    mpchf[CHMAX];
	int     i;

	for (i = 0; i < CHMAX; mpchf[i++] = fFalse)
		;

	CommSz("Starting mpchord check!\n\r");
	if (mp[0] != 0)
		{
		CommSz("Ordinal Number for 0 must be 0\n\r");
		}

	for (i = 0; i < CHMAX; i++)
		{
		int iFromMp;

		iFromMp = mp[i];
		if (iFromMp < 0 || CHMAX <= iFromMp)
			{
			CommSzNum("Invalid Ordinal Number at: ", i);
			}
		else  if (mpchf[iFromMp])
			{
			CommSzNum("Duplicate Ordinal Number at: ", i);
			}
		else
			{
			mpchf[iFromMp] = fTrue;
			}
		}

	for (i = 0; i < CHMAX; i++)
		{
		if (!mpchf[i])
			{
			CommSzNum("Unused Ordinal Number at: ", i);
			}
		}
	CommSz("End of mpchord check!\n\r");
}


#endif /* DBGSORTTBL */


#ifdef UNUSED
/* This has become a macro */
/*  %%Function: FPtInRect  %%Owner: notused  */

FPtInRect( pt, prc )
struct PT pt;
struct RC *prc;
{
	return PtInRect( (LPRECT)prc, pt );

}


#endif /* UNUSED */




/*  %%Function: PrettyUpPath  %%Owner: davidbo  */


PrettyUpPath(stPath)
char stPath[];
{
	char *pch;
	/*  rules (like DOS cd rules):
			if path ends not in \, put null after last letter
			if path ends in \ and is <drive>:\ , terminate after \
			if path ends in \ and is not <drive>:\ , terminate on top of \
	
			st count should not include trailing 0
	
			this code probably belongs in the asm file, but the call is not
			used many places and I am in a hurry, so...
	*/

	pch = &stPath[stPath[0]];  /* last char of path string */
	if (*pch != '\\')
		*(pch + 1) = 0;
	else  /* ends in \ */		
		{
		if (stPath[0] == 3 && stPath[1] == ':') /* cur on drive, leave \ */
			*(pch + 1) = 0;
		else
			{
			*pch = 0;  /* overwrite trailing \ */
			stPath[0]--;  /* remove \ from count */
			}
		}
}


/*  %%Function: CheckEnvPending  %%Owner: bobz  */

CheckEnvPending()
{
	/* deal with pending messages */
	if (vpri.wmm & wmmSysColorChange)
		{
		Debug(vpri.wmm &= ~wmmSysColorChange);
		AppWinSysChange(WM_SYSCOLORCHANGE, 0L);
		Assert(!(vpri.wmm & wmmSysColorChange)); /* be sure it really happened */
		}
	if (vpri.wmm & wmmWinIniChange)
		{
		Debug(vpri.wmm &= ~wmmWinIniChange);
		AppWinSysChange(WM_WININICHANGE, 0L);
		Assert(!(vpri.wmm & wmmWinIniChange)); /* be sure it really happened */
		}
	if (vpri.wmm & wmmDevModeChange)
		{
		Debug(vpri.wmm &= ~wmmDevModeChange);
		AppWinSysChange(WM_DEVMODECHANGE, 0L);
		Assert(!(vpri.wmm & wmmDevModeChange)); /* be sure it really happened */
		}
	if (vpri.wmm & wmmFontChange)
		{
		Debug(vpri.wmm &= ~wmmFontChange);
		AppWinSysChange(WM_FONTCHANGE, 0L);
		Assert(!(vpri.wmm & wmmFontChange)); /* be sure it really happened */
		}
	vpri.wmm = 0;
}


/* forward declarations */
KMP ** HKmpNew ();
CMD CmdPmtCmdKc ();
void OkPmt();
void CancelPmt();


/* F  P R O M P T  D I A L O G */
/*  Put up a dialog with szPrompt as prompt text, szInput as default input
	text.  Dialog has edit control, Ok and cancel buttons.
*/
/*  %%Function: FPromptDialog   %%Owner: peterj  */

FPromptDialog (szPrompt, szInput, cchMax)
CHAR *szPrompt, *szInput;
int cchMax;
{
	HCAB hcab;
	BOOL fRet = fFalse;
	CMB cmb;
	char dlt [sizeof (dltPrompt)];

	if ((hcab = HcabAlloc(cabiCABPROMPT)) == hNil)
		return fFalse;

		{
		CHAR szPromptClean[cchMaxSz];
		SanitizeSz(szPrompt, szPromptClean, cchMaxSz, fTrue);
		XlateCRLFToLFSz(szInput);	/* CRLF --> LF */
		if (!FSetCabSz(hcab, szPromptClean, Iag(CABPROMPT, hszPrompt)) ||
				!FSetCabSz(hcab, szInput, Iag(CABPROMPT, hszInput)))
			goto LRet;
		}

	cmb.hcab = hcab;
	cmb.cmm = cmmNoHelp;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	BltDlt(dltPrompt, dlt);
	if (TmcOurDoDlg(dlt, &cmb) == tmcOK)
		{
		GetCabSz(hcab, szInput, cchMax, Iag(CABPROMPT, hszInput));
		fRet = fTrue;
		}
LRet:
	XlateLFToCRJSz( szInput );	/* LF --> CRJ */
	FreeCab(hcab);

	return fRet;
}


/* X L A T E  L F  T O  C R J */
/*  %%Function: XlateLFToCRJ   %%Owner: tonykr  */
XlateLFToCRJSz( sz )
CHAR *sz;
{
	CHAR *pch;

	pch = sz;

	while (*pch != '\0')
		{
		if (*pch == chEol)
			*pch = chCRJ;
		pch++;
		}
}


/* X L A T E  C R L F  T O  L F  S Z */
/*  %%Function: XlateCRLFToLFSz   %%Owner: tonykr  */
/* Also translates chCRJ to LF, and <CR><chTable> to LF */
XlateCRLFToLFSz( sz )
CHAR *sz;
{
	CHAR * pchS, * pchD;
	CHAR ch;

	pchS = pchD = sz;
	while ((ch = *pchD = *pchS++) != '\0')
		{
		if (ch == chReturn)
			{
			*pchD = chEol;
			if (*pchS == chEol || *pchS == chTable)
				pchS++;
			}
		else
			{
			if (ch == chCRJ)
				*pchD = chEol;
			}
		pchD++;
		}
}

#ifdef NOTUSED
/* These two translate functions were used when windows edit controls
 * (which use CRLF's in the text box) were used instead of fedits. */

/* X L A T E  C R J  T O  C R L F  S Z  C C H */
/*  %%Function: XlateCRJToCRLFSzCch   %%Owner: tonykr  */
/* Note: Also translates <CR><chTable> to <CR><LF> */
XlateCRJToCRLFSzCch( sz, cchMax )
CHAR *sz;
int cchMax;
{
	CHAR *pch;
	CHAR *pchMac, *pchMax;

	pch = sz;
	pchMac = pch + CchSz(sz);
	pchMax = pch + cchMax;

	while (pch < pchMac)
		{
		if (*pch == chCRJ)
			{
			if (pchMac != pchMax)
				{
				*pch++ = chReturn;
				bltb(pch, pch + 1, (pchMac++) - pch);
				*pch = chEol;
				}
			}
		else if (*pch == chTable)
			{
			if (pch != sz && *(pch - 1) == chReturn)
				*pch = chEol;
			}
		pch++;
		}
}


/* X L A T E  C R L F  T O  C R J  S Z */
/*  %%Function: XlateCRLFToCRJSz   %%Owner: tonykr  */
XlateCRLFToCRJSz( sz )
CHAR *sz;
{
	CHAR * pchS, * pchD;

	pchS = pchD = sz;
	while ((*pchD = *pchS) != '\0')
		{
		if (*pchS == chReturn)
			{
			*pchD = chEop;
#ifdef CRLF
			if (*pchS != chEol && *pchS != chTable)
#else
			if (*pchS != chTable)
#endif
				pchD++;
			}
		else
			pchD++;
		pchS++;
		}
}
#endif /* NOTUSED */

/* I D  M E S S A G E  B O X  M S T  R G W  M B */
/*  Put up a windows message box with string based on pmtSpec and rgw.
	Return the returned windows ID.
*/
/*  %%Function: IdMessageBoxMstRgwMb   %%Owner: peterj  */

IdMessageBoxMstRgwMb (mst, rgw, mb)
MST mst;
int *rgw;
int mb;

{
	extern HWND vhwndStatLine;
	extern HWND vhwndPrompt;
	extern BOOL vfHelpPromptOn;
	extern BOOL vfLargeFrameEMS;
	CHAR szMessage [cchMaxSz];
	int id;
	int cxtSav;
	BOOL fSetPrompt = (!(mb & MB_SYSTEMMODAL) && vhwndStatLine && !vfHelpPromptOn && 
			!vhwndPrompt);

	/* WINBUG! */
	if ((mb & MB_SYSTEMMODAL) && vfLargeFrameEMS && vwWinVersion < 0x0299)
		mb = mb & ~MB_ICONHAND;

	BuildStMstRgw (mst, rgw, szMessage, cchMaxSz, hNil);
	StToSzInPlace (szMessage);
	cxtSav = vcxtHelp;
	vcxtHelp = mst.cxt;
	if (fSetPrompt)
		DisplayHelpPrompt(fTrue);
	id = IdPromptBoxSz (szMessage, mb);
	if (fSetPrompt)
		DisplayHelpPrompt(fFalse);
	vcxtHelp = cxtSav;
	return id;
}



/* T M C  I N P U T  P R O M P T  M S T */
/*  This function performs prompt line input.  mst is the prompt number,
	szInput is the buffer in which to place the input, ichMax is the max
	number of characters (including null) to write to szInput. If bcmKc !=
	bcmNil then tmcCmdKc will be returned if any kc mapped to bcmKc
	is pressed. tmcOK is returned on RETURN, tmcCancel is returned on ESCAPE.
*/
/*  %%Function: TmcInputPmtMst   %%Owner: peterj  */

TmcInputPmtMst (mst, szInput, ichMax, bcmKc, mmo)
MST mst;
CHAR * szInput;
int ichMax;
int bcmKc;
int mmo;

{
	int cxtT, tmc;

	cxtT = vcxtHelp;
	vcxtHelp = mst.cxt;
	tmc = TmcInputPmtSt(mst.st, szInput, ichMax, bcmKc, mmoBeepOnMouse|mmo);
	vcxtHelp = cxtT;

	return tmc;
}




/* T M C  I N P U T  P R O M P T  S T */
/*  Receive input on the prompt line.  stPrompt is the prompt text, szInput
	will get the result (may also contain default input).
*/
/*  %%Function: TmcInputPmtSt  %%Owner: peterj  */

TmcInputPmtSt(stPrompt, szInput, ichMax, bcmKc, mmo)
char * stPrompt;
char * szInput;
int ichMax;
BCM bcmKc;
int mmo;
{
	KMP ** hkmp;
	struct PIR pir;
	int tmc;
	int ich;
	CHAR *pch;
	int wLongOp;
	extern int vfMouseExist;

	SetOurKeyState();

	GetLongOpState(&wLongOp);
	EndLongOp(fTrue);
	SetCursor(vfMouseExist ? vhcArrow : NULL);

	pir.ichFirst = ich = min (cchPromptMax, *stPrompt+1);
	bltb (stPrompt, vstPrompt, ich);

	/* determine max input & copy initial input */
	pch = szInput;
	pir.ichMax = min (cchPromptMax, ich+ichMax);

	while (*pch && ich < pir.ichMax)
		vstPrompt [ich++] = *pch++;

	Assert (ich <= cchPromptMax);
	Assert (pir.ichMax <= cchPromptMax);
	vstPrompt[0] = ich-1;

	pir.fOn = fFalse;

	Assert (vppirPrompt == NULL);
	vppirPrompt = &pir;

	/* display the prompt */
	DisplayPrompt (pdcInput, hNil);
	if (!vhwndPrompt)
		goto LRetFailed;

	/* set up the key map */
	if ((hkmp = HKmpCreate (bcmKc != bcmNil ? 3 : 2, kmfStopHere)) == hNil)
		goto LRetFailed;
	AddKeyPfn (hkmp, kcReturn, OkPmt);
	AddKeyPfn (hkmp, kcEscape, CancelPmt);
	if (bcmKc != bcmNil)
		{
		int kc;

		kc = KcFirstOfBcm(hkmpCur, bcmKc);
		while (kc != kcNil)
			{
			void PmtCmdKc();

			AddKeyPfn(hkmp, kc, PmtCmdKc);
			kc = KcNextOfBcm(bcmKc);
			}
		}
	(*hkmp)->hkmpNext = hkmpCur;
	hkmpCur = hkmp;

	Assert (vpisPrompt == pisNormal);
	vpisPrompt = pisInput;

	/* this returns when the input is complete */
	if ((tmc = WAppModalHwnd (vhwndPrompt, mmo)) == fFalse)
		tmc = tmcCancel;

	/* remove the key map */
	RemoveKmp (hkmp);

	/* copy the result to szInput */
	pch = szInput;
	for (ich = pir.ichFirst; ich < vstPrompt[0]+1; ich++)
		*pch++ = vstPrompt [ich];
	*pch = '\0';

	vpisPrompt = pisNormal;
	vppirPrompt = NULL;
	RestorePrompt ();

	Assert (tmc == tmcOK || tmc == tmcCancel ||
			(bcmKc != bcmNil && tmc == tmcCmdKc) );

	ResetLongOpState(wLongOp);
	return tmc;

LRetFailed:
	/* something failed, abort */
	vppirPrompt = NULL;
	vpisPrompt = pisNormal;
	RestorePrompt ();
	ResetLongOpState(wLongOp);
	return tmcCancel;
}




/* P R O M P T  C A L L  B A C K  F U N C T I O N S */
/*  Used in keymaps set up during prompt functions.
*/
/*  %%Function: OkPmt   %%Owner: peterj  */


void OkPmt ()
	/*  used on RETURN during input */
{
	PostMessage (vhwndPrompt, AMM_TERMINATE, tmcOK, 0L);
}


/*  %%Function: CancelPmt   %%Owner: peterj  */

void CancelPmt ()
	/*  used on ESCAPE during input */
{
	PostMessage (vhwndPrompt, AMM_TERMINATE, tmcCancel, 0L);
}


/*  %%Function: PmtCmdKc  %%Owner: peterj  */


void PmtCmdKc()
	/*  used on key previously mapped to bcmKc on input */
{
	PostMessage (vhwndPrompt, AMM_TERMINATE, tmcCmdKc, 0L);
}


/*  BOGUS Routines required to link with the SDM library */
/*  %%Function: HbmpFromIBmp  %%Owner: bobz  */

HANDLE HbmpFromIBmp(ibmp)
int ibmp;
{
	Assert(fFalse);
	return NULL;
}


/*  %%Function: HbrsGetTmcHdc  %%Owner: bobz  */

HANDLE HbrsGetTmcHdc(tmc, hdc)
TMC tmc;
HDC hdc;
{
	Assert(fFalse);
	return NULL;
}


/*  %%Function: IdPromptBoxSz  %%Owner: peterj  */

IdPromptBoxSz(sz, mb)
char * sz;
int mb;
{
	extern char szApp [];
	int id;
	BOOL fDisable;

	id = IdOurMessageBox(sz, szAppTitle, mb);

	return id;
}


/*  %%Function: IdOurMessageBox  %%Owner: peterj  */

IdOurMessageBox(szText, szCaption, mb)
char * szText, * szCaption;
int mb;
{
	return IdOurFarMessageBox((LPSTR) szText, (LPSTR) szCaption, mb);
}


/*  %%Function: IdOurFarMessageBox  %%Owner: peterj  */


IdOurFarMessageBox(lpszText, lpszCaption, mb)
char far * lpszText, far * lpszCaption;
int mb;
{
	extern BOOL vfDeactByOtherApp;
	extern int  vcInMessageBox;
	extern BOOL vnErrorLevel;
	extern HCURSOR vhcArrow;
	extern int vfMouseExist;
	extern BOOL vfLargeFrameEMS;

	int id;
	int wLongOp;

/* If we are in batch mode or debug mode (BATCH is always defined in DEBUG
 * mode, but not vice versa) fill with default response. */
#ifdef BATCH
	/* respond with the default response */
#ifdef DEBUG
	/* vnErrorLevel is only defined in DEBUG mode */
	if (vnErrorLevel >= 2)
#else
		if (vfBatchMode)
#endif /* DEBUG */
			switch (mb & MB_TYPEMASK)
				{
			case MB_OK:
				return IDOK;
			case MB_OKCANCEL:
				return mb & MB_DEFBUTTON2 ? IDCANCEL : IDOK;
			case MB_ABORTRETRYIGNORE:
				return mb & MB_DEFBUTTON2 ? IDRETRY : 
						mb & MB_DEFBUTTON3 ? IDIGNORE : IDABORT;
			case MB_YESNOCANCEL:
				return mb & MB_DEFBUTTON2 ? IDNO : 
						mb & MB_DEFBUTTON3 ? IDCANCEL : IDYES;
			case MB_YESNO:
				return mb & MB_DEFBUTTON2 ? IDNO : IDYES;
			case MB_RETRYCANCEL:
				return mb & MB_DEFBUTTON2 ? IDCANCEL : IDRETRY;
			default:
				Assert(fFalse);
				return IDOK;
				}

	/* if we are in batchmode, don't put up the message box (no one
	around to answer it!) */
	if (vfBatchMode)
		{
		CHAR sz[cchMaxSz];
		CchCopyLpszCchMax(lpszText, sz, cchMaxSz);
		BatchModeError(SzFrame("MessageBox: "), sz, 0, 0);
		}
#endif /* BATCH */

	if (vhwndStartup != NULL)
		EndStartup();

	GetLongOpState(&wLongOp);
	EndLongOp(fTrue);

	/* flash title bar until user makes us the active app */
	if (vfDeactByOtherApp)
		{
		WaitActivation();
		if (InSendMessage ())   /* must be sys modal */
			{
			mb &= ~MB_ICONMASK;
			mb |= MB_SYSTEMMODAL|MB_ICONHAND;
			/* WINBUG! */
			if (vfLargeFrameEMS && vwWinVersion < 0x0299)
				mb = mb & ~MB_ICONHAND;
			}
		}

	/* close any open floppies in case user switches disks */
	CloseEveryFn(fFalse);

	OurSetCursor(vfMouseExist ? vhcArrow : NULL);
	vcInMessageBox++;
LTryAgain:
	if (vhwndApp == NULL || FIsDlgDying() || vfElOom
        || !vmerr.fSdmInit)
		/* SDM not yet initialized, put up message ourselves
           don't try to init sdm at this point; if the init fails we
           would loop forever bz
        */
		{
		id = MessageBox(vhwndApp, lpszText, lpszCaption, mb);
		}
	else
		id = IdDoMsgBox(lpszText, lpszCaption, mb);
	if (!id && !(mb & MB_SYSTEMMODAL))
		/* MessageBox failed, use system modal version */
		{
		mb &= ~MB_ICONMASK;
		mb |= MB_SYSTEMMODAL|MB_ICONHAND;
		/* WINBUG! */
		if (vfLargeFrameEMS && vwWinVersion < 0x0299)
			mb = mb & ~MB_ICONHAND;
		goto LTryAgain;
		}
	vcInMessageBox--;
	Assert (vcInMessageBox >= 0);

	  /* returning from a nested mb will reenable Opus; we don't want this.
		 Same is true after a dialog (a lot of these only happen at
		 WM_QUERYABORTSESSION when a dialog or mb is up and we click on the
		 dos exec. So if we are still in an mb, redisable Opus  bz  9/19/89
	  */
	if (vcInMessageBox)
		EnableWindow(vhwndApp, fFalse);

	/* clear the key state, in case we are in abortable process */
	GetAsyncKeyState(VK_ESCAPE);
	GetAsyncKeyState(VK_CANCEL);

	SetOurKeyState();

	ResetLongOpState(wLongOp);  /* resets long op state when entered */

	return id;
}




struct PLSBHI ** vhplsbhi = hNil;

/*  %%Function: LcbUnusedEmm  %%Owner: peterj  */


long LcbUnusedEmm()
{
	long cbFarAvail = 0;
	int isb;
	SB sb;
	extern int vrgsbReserve[];

	if (vhplsbhi != hNil)
		{
		struct SBHI *psbhi = PInPl(vhplsbhi, 0);
		struct SBHI *psbhiMac = psbhi + (*vhplsbhi)->isbhiMac;

		while (psbhi < psbhiMac)
			{
			cbFarAvail += (psbhi->fHasHeap ? CbAvailHeap(psbhi->sb) : 0);
			psbhi++;
			}
		}
	for (isb = 0; isb < csbGrabMax && (sb = vrgsbReserve[isb]) != sbNil; isb++)
		if (sb > 1 && sb < sbMax)
			cbFarAvail += CbSizeSb(sb);

	return cbFarAvail;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Util2_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Util2_Last() */
