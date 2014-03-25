/*
	makekeys --
		Takes a bunch of strings ("strings" file) and a bunch of candidates
		for keywords ("candidates" file).  Processes the keyword candidates
		to find the best keywords.  The keywords are output to stdout.  During
		processing, any candidate that will save less than cbSaveMin bytes
		will be thrown out.  A larger cbSaveMin will significantly speed up
		the run time.

		To compile with mkey.asm (for MUCH faster processing):
			msc -Oat -Gs -AC makekeys;
			masm makekeyn;
			link makekeys+makekeyn;

		To compile without mkey.asm:
			msc -DNOASM -Oat -Gs -AC makekeys;
			link makekeys;

		The following compaction algorithm is assumed:
			1)	Most used extended characters are remapped to bytes 1 - 31.
			2)	Any other extended characters are prefixed with a 0 byte.
			3)	Keywords are replaced with bytes 128 - 255.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#ifndef	NOASM
#define	CbScanKeySt	CbScanKeyStNat
#endif

#define TRUE	1
#define FALSE	0
#define	BYTE	unsigned char

#define	iKeyMax			128
#define	iStrMax			4096
#define	iCandMax		8192
#define cchCandMin		3		/* toss candidate if less than cchCandMin */

int		CbScanKeySt();
BYTE	*DupSt();
BYTE	*SzFromSt();

int		fVerbose;		/* if TRUE, print stats */

BYTE	*rgpStr[iStrMax+1];
BYTE	*rgpCand[iCandMax+1];
BYTE	*rgpKey[iKeyMax+1];
int		iStrMac;
int		iCandMac;
int		cbSaveMin;			/* toss candidate if bytes saved < cbSaveMin */

unsigned char rgExtChar[32];	/* remapped extended characters */
int		rgcExtChar[128];		/* count of extended character instances */
int		cExtCharMac;

int		iCandBest;
int		cbSaveMac;

main(argc, argv)
int argc;
char **argv;
{
	BYTE ch;
	int	iKey, cbSaveTotal;
	FILE *pfStr;
	BYTE *pch, *pchT;
	BYTE sz[256];

	if (argc < 4 || sscanf(argv[3], "%d", &cbSaveMin) != 1)
		Error("syntax:  makekeys strings candidates cbSaveMin [-v]");

	fVerbose = (argc == 5 && strcmp(argv[4], "-v") == 0);

	/* read in the strings */
	ReadStr(argv[1]);

	/* read in the keyword candidates */
	ReadCand(argv[2]);

	if (fVerbose)
		fprintf(stderr, "NUMBER\tBYTES SAVED\tKEYWORD\n");

	cbSaveTotal = 0;
	for (iKey = 0; iKey < iKeyMax; iKey++)
		{
		int	i;

		GetBestCand();

		if (cbSaveMac == 0)
			{
			if (fVerbose)
				fprintf(stderr, "makekeys:  less than %d keywords.\n", iKeyMax);
			break;
			}

		cbSaveTotal += cbSaveMac;

		rgpKey[iKey] = DupSt(rgpCand[iCandBest]);

		SzFromSt(rgpCand[iCandBest], sz);
		puts(sz);

		if (fVerbose)
			fprintf(stderr, "%5d\t%7d\t\t \"%s\"\n", iKey, cbSaveMac, sz);

		for (i = 0; i < iStrMac; i++)
			InsertKeyword(rgpCand[iCandBest], rgpStr[i], iKey);

		for (i = 0; i < iCandMac; i++)
			if (iCandBest != i)
				InsertKeyword(rgpCand[iCandBest], rgpCand[i], iKey);

		RemoveCand(iCandBest);
		}

	if (fVerbose)
		fprintf(stderr, "Saved approximately %d bytes.\n", cbSaveTotal);

	exit(0);
}


ReadStr(szFile)
BYTE *szFile;
{
	int	i = 0;
	BYTE ch, *pch;
	FILE *pf;
	BYTE sz[256];
	BYTE st[256];

	if ((pf = fopen(szFile, "r")) == NULL)
		Error("could not open strings file");

	for (i = 0; i < 128; i++)
		{
		rgcExtChar[i] = 0;
		}

	/* count all of the extended characters */
	i = 0;
	while (fgets(sz, 250, pf) != NULL)
		{
		if (sz[0] != '\0' && sz[0] != '\n')
			{
			if (i == iStrMax)
				Error("too many strings");
			pch = sz;
			while (ch = *pch++)
				if (ch & 0x80)
					rgcExtChar[ch & 0x7f]++;
			}
		}

	MapExtChars();

	fseek(pf, 0L, SEEK_SET);		/* reset input file for second pass		*/

	i = 0;
	while (fgets(sz, 250, pf) != NULL)
		{
		if (sz[0] != '\0' && sz[0] != '\n')
			{
			StFromSz(sz, st);
			rgpStr[i++] = DupSt(st);
			}
		}

	iStrMac = i;
	rgpStr[i] = NULL;

	fclose(pf);
}


ReadCand(szFile)
BYTE *szFile;
{
	int	i = 0;
	FILE *pf;
	BYTE sz[256];
	BYTE st[256];

	if ((pf = fopen(szFile, "r")) == NULL)
		Error("could not open candidates file");

	while (fgets(sz, 250, pf) != NULL)
		{
		if (i == iCandMax)
			Error("too many candidates");
		StFromSz(sz, st);
		if (st[0] >= cchCandMin && (i == 0 || !FCmpSt(rgpCand[i-1], st)))
			rgpCand[i++] = DupSt(st);
		}

	iCandMac = i;
	rgpCand[i] = NULL;

	fclose(pf);
}


MapExtChars()
{
	int	i, j;
	int	mpT[128];

	for (i = 0; i < 128; i++)
		{
		mpT[i] = i + 128;
		}

	/* sort to find most used extended characters */
	Sort(rgcExtChar, mpT, FALSE);

	rgExtChar[0] = 0;
	for (i = 1, j = 0; i < 32; i++)
		{
		if (rgcExtChar[j] == 0)
			break;
		if (i == 0x0a || i == 0x0d)
			rgExtChar[i] = i;
		else
			rgExtChar[i] = mpT[j++];
		}

	cExtCharMac = i;
}


Sort(rgKey, rgw, fAscending)		/* do a simple Shell sort */
int	rgKey[], rgw[], fAscending;
{
	int gap, i, j, temp;

	for (gap = 128/2; gap > 0; gap /= 2)
		for (i = gap; i < 128; i++)
			for (j = i - gap;
					j >= 0 && (fAscending ?
					(rgKey[j] > rgKey[j+gap]) : (rgKey[j] < rgKey[j+gap]));
					j -= gap)
				{
				temp = rgKey[j];
				rgKey[j] = rgKey[j+gap];
				rgKey[j+gap] = temp;
				temp = rgw[j];
				rgw[j] = rgw[j+gap];
				rgw[j+gap] = temp;
				}
}


GetBestCand()
{
	register int cbSave;
	int	iCand;
	BYTE **pstCand;
	register BYTE *pch;

	iCand = cbSaveMac = 0;
	pstCand = rgpCand;
	while ((pch = *pstCand) != NULL)
		{
		/* fprintf(stderr, "%5d\r", iCandMac - iCand); */

		cbSave = CbScanKeySt(pch, rgpStr, FALSE) - *pch - 3;

		if (cbSave <= cbSaveMin)
			{
			RemoveCand(iCand);
			}
		else
			{
			if (cbSaveMac < cbSave)
				{
				cbSaveMac = cbSave;
				iCandBest = iCand;
				}
			iCand++;
			pstCand++;
			}
		}
}


RemoveCand(iCand)
int iCand;
{
	int i;

	free(rgpCand[iCand]);
	for (i = iCand; i < iCandMac; i++)
		rgpCand[i] = rgpCand[i+1];
	iCandMac--;
}


#ifdef	NOASM
int CbScanKeySt(stKey, pstStr, fSingleString)
BYTE *stKey;	/* key string to look for	*/
BYTE **pstStr;	/* array of strings to scan */
int  fSingleString;
{
	int cbKey, cb, cbSave;
	register int cbScan;
	register BYTE chFirst;
	BYTE *pch, *pchScan, *pchKey;

	if (stKey == NULL)
		return;

	cbKey = (int)*stKey - 1;
	cbSave = 0;
	chFirst = *(stKey+1);

	while ((pchScan = *pstStr++) != NULL)
		{
		cbScan = (int)*pchScan++;
		for (;;)
			{
			while (cbScan-- > 0)
				if (*pchScan++ == chFirst)
					break;
			if (cbScan < cbKey)
				break;
			cb = cbKey;
			pchKey = stKey + 2;
			pch = pchScan;
			while (cb-- > 0)
				if (*pchKey++ != *pch++)
					break;
			if (cb == -1)
				{
				cbSave += cbKey;
				pchScan = pch - 1;
				cbScan -= cbKey;
				}
			}
		if (fSingleString)
			break;
		}

	return(cbSave);
}


#endif


InsertKeyword(stKey, st, iKey)
BYTE *stKey;	/* key string to insert */
BYTE *st;		/* string to insert key into */
int  iKey;
{
	BYTE *pchT, *pchDst;
	int  cbScan;
	BYTE stT[256];

	if (CbScanKeySt(stKey, &st, TRUE) != 0)
		{
		pchDst = st;
		pchT = stT;
		for (cbScan = *pchT++ = *pchDst++; cbScan > 0; cbScan--)
			*pchT++ = *pchDst++;

		pchDst = st + 1;
		pchT = stT;
		for (cbScan = (int)*pchT; cbScan > 0; cbScan--)
			{
			*pchDst++ = *(pchT+1);
			*pchT = *stKey;
			if (FCmpSt(stKey, pchT))
				{
				*(pchDst-1) = (BYTE)iKey | 0x80;
				pchT += *stKey;
				cbScan -= (int)*stKey - 1;
				}
			else
				pchT++;
			}
		*st = pchDst - st - 1;
		}
}


int FCmpSt(st1, st2)
BYTE *st1, *st2;
{
	BYTE ich;

	if ((ich = *st1++) != *st2++)
		return(FALSE);

	for ( ; ich > 0; ich--)
		if (*st1++ != *st2++)
			return(FALSE);

	return(TRUE);
}


BYTE *DupSt(st)
BYTE *st;
{
	BYTE *pch, *pchT, ich;

	if ((pch = malloc(*st + 1)) == NULL)
		Error("out of memory");

	pchT = pch;
	for (ich = *pchT++ = *st++; ich > 0; ich--)
		*pchT++ = *st++;

	return(pch);
}


BYTE *SzFromSt(stSrc, szDst)
BYTE *stSrc, *szDst;
{
	BYTE ch, ich;

	for (ich = *stSrc++; ich > 0; ich--)
		{
		ch = *szDst++ = *stSrc++;
		if (ch == 0)
			{
			*(szDst-1) = *stSrc++;
			ich--;
			}
		else  if (ch < cExtCharMac)
			{
			*(szDst-1) = rgExtChar[ch];
			}
		else  if (ch > 0x7f)
			{
			/* recurse to expand the keyword */
			szDst = SzFromSt(rgpKey[ch & 0x7f], szDst-1);
			}
		}

	*szDst = 0;
	return(szDst);
}


StFromSz(szSrc, stDst)
BYTE *szSrc, *stDst;
{
	BYTE cb, ch, *pch, *pchSrc;
	int i;

	cb = 0;
	pchSrc = szSrc;
	pch = stDst+1;
	while ((ch = *pchSrc++) != '\0' && ch != '\n')
		{
		if (ch & 0x80)
			{
			/* found an extended character */
			for (i = 0; i < cExtCharMac; i++)
				if (ch == rgExtChar[i])
					break;

			if (i == cExtCharMac)
				{
				*pch++ = '\0';
				}
			else
				{
				ch = i;
				}
			}

		*pch++ = ch;
		if (cb++ == 255)
			{
			fprintf(stderr, "\"%s\"\n", szSrc);
			Error("string is too long");
			}
		}
	*stDst = cb;
}


Error(szErr)
BYTE *szErr;
{
	BYTE sz[81];

	fprintf(stderr, "makekeys:  %s.\n", szErr);
	exit(1);
}


