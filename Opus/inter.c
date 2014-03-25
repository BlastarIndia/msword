/* I N T E R . C */

/*  This file is #include'd into other modules, not compiled on
	its own.
*/

/*  This file contains code that will/may have to be changed for
	specific international versions.  By including it, a different
	version can be substituted for different localized versions.
*/





#ifdef FIELDPIC_C  

/* Number->text translation code */

/* Required entry points:
	CchFormatLongOrdinal(l, ppch)
		format l in the form 1st, 2nd,...
	CchStuffLongText(l, ppch, fOrdinal)
		format l in the form First, Second,... (if fOrdinal)
		One, Two,... (if !fOrdinal)
	CchInsertFract(doc, cp, pnum, pchp)
		insert the fractional portion of *pnum at doc,cp
*/



csconst char  rgszOrdinal [][] =
{
	"th",
			"st",
			"nd",
			"rd"
};


/* C C H  F O R M A T  L O N G	O R D I N A L */
/* %%Function:CchFormatLongOrdinal %%Owner:PETERJ */
int CchFormatLongOrdinal(l, ppch)
long l;
char **ppch;

{
	int cch = CchLongToPpch(l, ppch);
	long lAbs = l < 0 ? -l : l;
	int i = 0, nOnes = lAbs%10L, nTens = lAbs%100L;
	char szBuffer [cchMaxOrdApp];

	/* figure out which ordinal extension to use */
	if (nOnes <= 3 && (nTens < 11 || nTens > 13))
		i = nOnes;

	bltbx ((char far *) rgszOrdinal [i], (char far *)szBuffer,
			cchMaxOrdApp);
	return cch + CchPchToPpch (szBuffer, ppch);
}


csconst char rgszNumText [][] =     /* These need not use SzKey() since this
	whole file gets localized. */
{
	"Zero",
			" Hundred",
			" Thousand",
#define iszZero     0
#define iszHundred  1
#define iszThousand 2

#define dszTens     1
			"Twenty",
			"Thirty",
			"Forty",
			"Fifty",
			"Sixty",
			"Seventy",
			"Eighty",
			"Ninety",

#define dszOnes     10
			"One",
			"Two",
			"Three",
			"Four",
			"Five",
			"Six",
			"Seven",
			"Eight",
			"Nine",
			"Ten",
			"Eleven",
			"Twelve",
			"Thirteen",
			"Fourteen",
			"Fifteen",
			"Sixteen",
			"Seventeen",
			"Eighteen",
			"Nineteen"
};


csconst char rgszOrdText [][] =
{
	"Zeroth",
			" Hundredth",
			" Thousandth",

			"Twentieth",
			"Thirtieth",
			"Fortieth",
			"Fiftieth",
			"Sixtieth",
			"Seventieth",
			"Eightieth",
			"Ninetieth",

			"First",
			"Second",
			"Third",
			"Fourth",
			"Fifth",
			"Sixth",
			"Seventh",
			"Eighth",
			"Ninth",
			"Tenth",
			"Eleventh",
			"Twelfth",
			"Thirteenth",
			"Fourteenth",
			"Fifteenth",
			"Sixteenth",
			"Seventeenth",
			"Eighteenth",
			"Nineteenth"
};



/* C O P Y  T E X T  P P C H */
/* %%Function:CopyTextPpch %%Owner:PETERJ */
CopyTextPpch(isz, ppch, fOrd)
int isz;
char **ppch;
BOOL fOrd;

{
	char szBuffer [cchMaxText];

	bltbx ((char far *) (fOrd ? rgszOrdText : rgszNumText) [isz],
			(char far *) szBuffer, cchMaxText);

	CchPchToPpch (szBuffer, ppch);
}



/* C C H  S T U F F  L O N G  T E X T */
/*  Fill ppch with the textual form of l.  If fOrd the result is of
	the form First, Second, Third,... else it is of the form One, Two,
	Three,....	Input must be non-negative and less than 1,000,000.
*/

/* %%Function:CchStuffLongText %%Owner:PETERJ */
int CchStuffLongText(l, ppch, fOrd)
LONG l;
char **ppch;
BOOL fOrd;

{
	char *pch = *ppch;
	LONG lT;
	BOOL fStarted = fFalse;

	Assert (l >= nMinText && l < nMaxText);
	if (l == 0)
		CopyTextPpch (iszZero, ppch, fOrd);

	else
		{
		if (l >= 1000)
			{ /* THOUSAND */
			if (l >= 100000)
				{ /* HUNDRED THOUSAND */
				lT = l / 100000L;
				FormatTensPpch ((int)lT, ppch, fFalse);
				CopyTextPpch (iszHundred, ppch, fFalse);
				l = l % 100000L;
				fStarted = fTrue;
				}

			if (l >= 1000)
				{ /* TENS OF THOUSANDS */
				if (fStarted)
					*(*ppch)++ = ' ';
				lT = l / 1000L;
				FormatTensPpch ((int)lT, ppch, fFalse);
				l = l % 1000L;
				fStarted = fTrue;
				}

			CopyTextPpch (iszThousand, ppch, (fOrd && l == 0));
			}

		if (l >= 100)
			{ /* HUNDRED */
			if (fStarted)
				*(*ppch)++ = ' ';
			lT = l / 100L;
			FormatTensPpch ((int)lT, ppch, fFalse);
			l = l % 100L;
			CopyTextPpch (iszHundred, ppch, (fOrd && l == 0));
			fStarted = fTrue;
			}

		if (l > 0)
			{ /* TENS */
			if (fStarted)
				*(*ppch)++ = ' ';
			FormatTensPpch ((int)l, ppch, fOrd);
			}
		}

	return *ppch - pch;
}



/* F O R M A T	T E N S  P P C H */
/*  Format a number between One and Ninety-Nine as text.  If fOrd then
	make it ordinal (First, Ninety-Nineth).
*/

/* %%Function:FormatTensPpch %%Owner:PETERJ */
FormatTensPpch(n, ppch, fOrd)
int n;
char **ppch;
BOOL fOrd;

{

	Assert (n > 0 && n < 100);

	if (n >= 20)
		{    /* ten amount (twenty, thirty,...) */
		int nT = n % 10;
		CopyTextPpch ((n/10)+dszTens, ppch, (fOrd && nT == 0));
		if (nT == 0)
			return;

		*(*ppch)++ = '-';
		n = nT;
		}

	/* ones amount (one, two, ..., nineteen */
	CopyTextPpch (n+dszOnes, ppch, fOrd);
}


#define cchFract    11	/* and xx/100 */
#define ichNumFract 5

csconst CHAR szFractText [] = " and NO/100";


/* %%Function:CchInsertFract %%Owner:PETERJ */
int CchInsertFract(doc, cp, pnum, pchp)
int doc;
CP cp;
long *pnum;
struct CHP *pchp;

{
	int cch;
	CHAR szFract [3];
	CHAR szBuffer [cchFract];

	bltbx ((char far *)szFractText, (char far *)szBuffer, cchFract);

	if ((cch = CchSzFractNum (pnum, szFract, 3)) != 0)
		{
		Assert (cch < 3);
		szBuffer [ichNumFract]   = szFract [0];
		szBuffer [ichNumFract+1] = cch > 1 ? szFract [1] : '0';
		}

	return FInsertRgch (doc, cp, szBuffer, cchFract, pchp, NULL) ?
			cchFract : 0;
}


#endif /* FIELDPIC_C */
