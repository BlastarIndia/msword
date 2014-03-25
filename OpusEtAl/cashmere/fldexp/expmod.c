#define DEBUG
#define SHOWLEX
#include "fldexp.h"

extern BOOL    fRowCol;

	char *rgszErrMsg[] = {
	"Parser Stack Overflow.\n",
	"Syntax Error.\n",
	"Token too long; truncated.\n",
	"Look-Ahead Buffer Truncated.\n"
	};


/************************ Lexical Analyzer code. ****************************/

error(iErr)
int iErr;
{
	printf(rgszErrMsg[iErr]);
}


main(argc, argv)
int   argc;
char *argv[];
{
	int   fAccepted;

	while (fAccepted = FParseExp())
		{
		printf("Accepted!\n");
		}
	printf("------ Aborted --------\n");
}



char szToken[cchTokenMax];
int  ichTokenMac;
char rgchLkAhd[cchLkAhdMax];
int  ichLkAhdMin = 0;
int  ichLkAhdMac = 0;
BOOL fLookingAhead = fFalse;

BOOL fErrReported;

#define IsUpperAlpha(_ch) ('A' <= (_ch) && (_ch) <= 'Z')
#define IsLowerAlpha(_ch) ('a' <= (_ch) && (_ch) <= 'z')
#define IsNum(_ch) ('0' <= (_ch) && (_ch) <= '9')
#define IsAlphaNum(_ch) (IsUpperAlpha(_ch) || IsNum(_ch))
#define ChUpper(_ch) (IsLowerAlpha(_ch) ? ('A' + ((_ch) - 'a')) : (_ch))
#define SkipWhiteSpace(_ch) while ((_ch) == chSpace || (_ch) == chTab) \
								{                                      \
								(_ch) = ChFetch();                     \
								}

#define ResetTokenBuf() (ichTokenMac = 0)
/* As long as PutChInTokenBuf() is used, the following macro is safe. */
#define NullTerminateToken() (szToken[ichTokenMac] = '\0')

char ChFetch();


/* lexical analysis routine */
int TiLexF(fNumInParen)
BOOL    fNumInParen;
{
	char ch, chSave;
	int  fInteger;
	int  fHasADigit;
	int  ich;

	if (!fNumInParen)
		{
		ResetTokenBuf();
		fErrReported = fFalse;
		}
	fInteger = fTrue;
	fHasADigit = fFalse;

	/* Skip leading spaces */
	ch = ChFetch();
	SkipWhiteSpace(ch);

	if (IsUpperAlpha(ch))
		{
		if (fRowCol)
			{
			switch (ch)
				{
			case chRow:
				return (tiROW);
			case chCol:
				return (tiCOL);
			default:
				return (tiBOOKMARK);
				}
			}
		else
			{
			do 
				{
				PutChInTokenBuf(ch);
				ch = ChFetch();
				} 
			while (IsAlphaNum(ch));

			PushBackCh(ch);
			NullTerminateToken();
			return(TiClassAlphaTokenSz(szToken));
			}
		}
	else
		{
		switch (ch)
			{
		case '\n':
			return (tiEOI);  /* end marker */
		case '=':
			return (tiEQ);
		case '<':
			ch = ChFetch();
			switch (ch)
				{
			case '=':
				return (tiLE);
			case '>':
				return (tiNE);
			default:
				PushBackCh(ch);
				return (tiLT);
				}
		case '>':
			ch = ChFetch();
			if (ch == '=')
				{
				return(tiLE);
				}
			else
				{
				PushBackCh(ch);
				return (tiLT);
				}
		case chList:
			return (tiCOMSEP);
		case '-':
		case '+':
		case '*':
		case '/':
		case '[':
		case ']':
			return (ch);
		case '(':
			if (!fNumInParen)
				{
				int ti;

				StartLkAhdBuf();
				ti = TiLexF(fTrue);
				if (ti == tiINTEGER || ti == tiNUMBER)
					{
					ch = ChFetch();
					SkipWhiteSpace(ch);

					if (ch == ')')
						{
						/* Multiply by -1 */
						AcceptLkAhd();
						return (tiNUMBER);
						}
					}
				BackTrackLkAhd();
				}
			return ('(');
		default:
			if (!IsNum(ch))
				{
				return (ch); /* This should make the parser puke. */
				}

			while (IsNum(ch) || ch == ch1000th)
				{
				if (ch == ch1000th)
					{
					ch = ChFetch();
					if (!IsNum(ch))
						{
						PushBackCh(ch);
						ch = ch1000th;
						break;
						}
					fInteger = fFalse;
					}
				fHasADigit = fTrue;
				PutChInTokenBuf(ch);
				ch = ChFetch();
				}

			if (ch == chDecimal)
				{
			case chDecimal:
				fInteger = fFalse;
				PutChInTokenBuf(ch);

				ch = ChFetch();
				while (IsNum(ch))
					{
					fHasADigit = fTrue;
					PutChInTokenBuf(ch);
					ch = ChFetch();
					}
				}

			if (!fNumInParen && fHasADigit)
				{
				SkipWhiteSpace(ch);
				if (ch == chPercent)
					{
					fInteger = fFalse;
					/* divide by 100 here.... */
					ch = ChFetch();
					}
				}
			PushBackCh(ch);
			NullTerminateToken();
#ifdef DEBUG
			printf("Number: %s\n", szToken);
#endif
			if (fHasADigit)
				return (fInteger ? tiINTEGER : tiNUMBER);
			else
				return (chDecimal); /* This should make parse puke. */
			}
		}
}


PutChInTokenBuf(ch)
char   ch;
{
	extern int  ichTokenMac;
	extern BOOL fErrReported;
	extern char szToken[];

	if (ichTokenMac >= cchTokenMax - 1)
		{
		if (!fErrReported)
			{
			error(ILONGTOKEN);
			fErrReported = fTrue;
			}
		}
	else
		{
		szToken[ichTokenMac++] = ch;
		}
}


RW      rgrw[] = {
	{ "ABS",     tiUNRFNC },
	{ "AND",     tiBINFNC },
	{ "AVERAGE", tiRDCFNC },
	{ "COUNT",   tiRDCFNC },
	{ "FALSE",   tiBOOLFALSE },
	{ "IF",      tiIF },
	{ "INT",     tiUNRFNC },
	{ "MAX",     tiRDCFNC },
	{ "MIN",     tiRDCFNC },
	{ "MOD",     tiBINFNC },
	{ "OR",      tiBINFNC },
	{ "PRODUCT", tiRDCFNC },
	{ "ROUND",   tiBINFNC },
	{ "SIGN",    tiUNRFNC },
	{ "SUM",     tiRDCFNC },
	{ "TRUE",    tiBOOLTRUE }
	};			


#define irwMax  (sizeof(rgrw) / sizeof(RW))

/* Z Y | X W V U | T S R Q | P O N M | L K J I | H G F E | D C B A */
/*     |         | * * *   | * *   * |       * |     *   |   *   * */
/* 0 0 | 0 0 0 0 | 1 1 1 0 | 1 1 0 1 | 0 0 0 1 | 0 0 1 0 | 0 1 0 1 */
/*  0  |    0    |    E    |    D    |    1    |    2    |    5    */
long    grpfReserved = 0x000ED125L;

int TiClassAlphaTokenSz(sz)
char    *sz;
{
	int      ti;

#ifdef DEBUG
	printf("Alpha Token: %s\n", sz);
#endif
	if (grpfReserved & (0x00000001L << (*sz - 'A')))
		{
		int irwMin, irwCur, irwMac;

		irwMin = 0;
		irwMac = irwMax;

		ti = tiBOOKMARK;
		while (irwMin < irwMac)
			{
			int      w;

			irwCur = (irwMin + irwMac) >> 1;

			if ((w = WCompSz(sz, rgrw[irwCur].szName)) == 0)
				{
				ti = rgrw[irwCur].ti;
				break;
				}
			else  if (w < 0)
				{
				irwMac = irwCur;
				}
			else
				{
				irwMin = irwCur + 1;
				}
			}

		if (ti == tiBOOLFALSE || ti == tiBOOLTRUE)
			{
			/* This is where we can get a corresponding boolean value
				for TRUE and FALSE. */
			ti = tiNUMBER;
			}
#ifdef SHOWLEX
		printf("Returning: %d\n", ti);
#endif
		return (ti);
		}
	else
		{
#ifdef SHOWLEX
		printf("Returning: %d\n", tiBOOKMARK);
#endif
		return (tiBOOKMARK);
		}
}


int WCompSz(sz1, sz2)
char *sz1;
char *sz2;
{
	int   ch1, ch2;
	for (ch1 = *sz1++, ch2 = *sz2++; ch1 == ch2; ch1 = *sz1++, ch2 = *sz2++)
		{
		if (ch1 == 0)
			return(0);
		}
	return(ch1 - ch2);
} /* end of  W C o m p S z   */



PushBackCh(ch)
char    ch;
{
	if (fLookingAhead)
		{
		/* The character to be pushed back is already in
			the look ahead buffer. */
		Assert(ichLkAhdMin != 0, "Look Ahead Underflow!");
		ichLkAhdMin--;
		}
	else
		{
		Assert(ichLkAhdMac < cchLkAhdMax, "Look Ahead Overflow!");
		if (ichLkAhdMin < ichLkAhdMac)
			{
			CopyPchCch(&rgchLkAhd[ichLkAhdMin], &rgchLkAhd[ichLkAhdMin + 1],
					ichLkAhdMac - ichLkAhdMin);
			}
		rgchLkAhd[ichLkAhdMin] = ch;
		ichLkAhdMac++;
		}
}


char ChFetch()
{
	char ch;

#ifdef SHOWLEX
	ShowLkAhdBuf();
#endif
	if (ichLkAhdMac <= ichLkAhdMin)
		{
		ch = getchar();
		ch = ChUpper(ch);
		if (ch == chCurrency)
			{
			ch = chSpace;
			}
		if (fLookingAhead)
			{
			ExtendLkAhd(ch);
			}
		else
			{
			ichLkAhdMac = ichLkAhdMin = 0;
			}
		}
	else
		{
		ch = rgchLkAhd[ichLkAhdMin++];
		if (!fLookingAhead & ichLkAhdMin == ichLkAhdMac)
			{
			ichLkAhdMac = ichLkAhdMin = 0;
			}
		}

	return (ch);
}



StartLkAhdBuf()
{
	if (ichLkAhdMin == ichLkAhdMac)
		{
		ichLkAhdMin = ichLkAhdMac = 0;
		}
	else
		{
		CopyPchCch(&rgchLkAhd[ichLkAhdMin], &rgchLkAhd[0],
				ichLkAhdMac = ichLkAhdMac - ichLkAhdMin);
		ichLkAhdMin = 0;
		}
	fLookingAhead = fTrue;
	return;
}


AcceptLkAhd()
{
	if (ichLkAhdMin != ichLkAhdMac)
		{
		CopyPchCch(&rgchLkAhd[ichLkAhdMin], &rgchLkAhd[0],
				ichLkAhdMac = ichLkAhdMac - ichLkAhdMin);
		}
	ichLkAhdMac -= ichLkAhdMin;
	ichLkAhdMin = 0;
	fLookingAhead = fFalse;
	return;
}


BackTrackLkAhd()
{
	ichLkAhdMin = 0;
	fLookingAhead = fFalse;
	return;
}


ExtendLkAhd(ch)
char    ch;
{
	if (ichLkAhdMac >= cchLkAhdMax)
		{
		error(ITRUNCLKAHD);
		rgchLkAhd[ichLkAhdMac - 2] = rgchLkAhd[ichLkAhdMac - 1];
		rgchLkAhd[ichLkAhdMac - 1] = ch;
		}
	else
		{
		if (ichLkAhdMin < ichLkAhdMac)
			{
			CopyPchCch(&rgchLkAhd[ichLkAhdMin], &rgchLkAhd[ichLkAhdMin + 1],
					ichLkAhdMac - ichLkAhdMin);
			}
		rgchLkAhd[ichLkAhdMin++] = ch;
		ichLkAhdMac++;
		}
}


CopyPchCch(pchSrc, pchDest, cch)
char    *pchSrc;
char    *pchDest;
int      cch;
{
	int      ich;

	if (pchDest < pchSrc + cch)
		{
		for (pchSrc += cch - 1, pchDest += cch - 1, ich = cch;
				ich != 0;
				*pchDest-- = *pchSrc--, ich--);
		}
	else
		{
		for (ich = cch; ich != 0; *pchDest++ = *pchSrc++, ich--);
		}
}


#ifdef SHOWLEX
ShowLkAhdBuf()
{
	char szBuf[cchLkAhdMax + 2];
	int  ich;

	for (ich = 0; ich < cchLkAhdMax + 1; szBuf[ich++] = chSpace);

	CopyPchCch(rgchLkAhd, szBuf, cchLkAhdMax);
	szBuf[ichLkAhdMac] = '\0';

	printf("Buffer: %s\n", szBuf);

	for (ich = 0; ich < cchLkAhdMax + 2; szBuf[ich++] = chSpace);
	if (ichLkAhdMin == ichLkAhdMac)
		{
		szBuf[ichLkAhdMac] = 'X';
		}
	else
		{
		szBuf[ichLkAhdMin] = 'm';
		szBuf[ichLkAhdMac] = 'M';
		}
	szBuf[ichLkAhdMac + 1] = '\0';

	printf("Points: %s\n", szBuf);
}


#endif
