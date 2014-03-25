#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <search.h>


#define cchStrMax   100     /* max allowable string length */

#define cszSymbInit 2000

#define fFalse      0
#define fTrue       1

typedef int BOOL;

FILE * (near pStmSrc);          /* stream pointer of source .map file */
FILE * (near pStmDst);          /* stream pointer of destination .map file */
FILE * (near pStmSymb);         /* stream pointer of symbols to look for file */

#define cszSymbMax  10000       /* total count of symbols max */
#define cSegMax      1000       /* total count of segmants max */

char near rgSegHit[cSegMax];    /* flag whether seg has an entry or not */

char far *(near rglpszSymb[cszSymbMax]);
int near cszSymb;               /* total count of symbols */

char near szBuff[cchStrMax];    /* general input/output buffer */

int near cCodeSeg;              /* total count of code segments */


BOOL near fInclude = fTrue;

GetIntFromHexSz(char near *);

main(argc, argv)
int argc;
unsigned char *argv[];
{
	int cbsz;
	int iargv = 1;
	int iSeg;

	if ((argc != 4) && (argc != 5))
		BadCommandLine();

	if ((*(argv[iargv]) == '/') || (*(argv[iargv]) == '-'))
		{
		if ((*(argv[iargv] + 1) == 'E') || (*(argv[iargv] + 1) == 'e'))
			{
			fInclude = fFalse;
			iargv++;
			}
		else
			BadCommandLine();
		}

	/* open the various files */
	if ((pStmSrc = fopen(argv[iargv++], "rt")) == NULL)
		Error("Cannot open source .map file");
	if ((pStmDst = fopen(argv[iargv++], "wt")) == NULL)
		Error("Cannot open destination.map file");
	if ((pStmSymb = fopen(argv[iargv++], "rt")) == NULL)
		Error("Cannot open symbol list file");

	for (iSeg = 0; iSeg < cSegMax; iSeg++)
		rgSegHit[iSeg] = 0;

	ReadSymbFile();

	/* copy exactly the segment and export definitions */
	while (fTrue)
		{
		if (fgets(szBuff, cchStrMax, pStmSrc) == NULL)
			Error("Read error in source map file");

		if (fputs(szBuff, pStmDst) == EOF)
			Error("Write error to opusnew.map");

		cbsz = strlen(szBuff);
		szBuff[cbsz - 1] = 0;   /* kill line feed */

		if (strcmp(&szBuff[45], "BEGDATA") == 0)
			cCodeSeg = GetIntFromHexSz(szBuff) + 1;

		if (strcmp(szBuff, "  Address         Publics by Name") == 0)
			{
			fprintf(pStmDst, "\n");
			break;
			}

		if (strcmp(szBuff, "  Address         Publics by Value") == 0)
			{
			fprintf(pStmDst, "\n");
			goto SkipSearch;
			}
		}


	while (fTrue)
		{
		if (fgets(szBuff, cchStrMax, pStmSrc) == NULL)
			Error("Read error in source map file");

		cbsz = strlen(szBuff);
		szBuff[cbsz - 1] = 0;   /* kill line feed */

		if (strcmp(szBuff, "  Address         Publics by Value") == 0)
			{
			fprintf(pStmDst, "\n%s\n\n", szBuff);
			break;
			}
		}

SkipSearch:

	printf("Search through the symbols and output the ones we want...\n");
	/* search through the symbols and output the ones we want */
	while (fTrue)
		{
		if (fgets(szBuff, cchStrMax, pStmSrc) == NULL)
			Error("Read error in source map file");

		if (strncmp(szBuff, "Program entry point at", 22) == 0)
			break;

		cbsz = strlen(szBuff);
		szBuff[cbsz - 1] = 0;   /* kill line feed */

		if (fIsSymbToSave())
			{
			rgSegHit[GetIntFromHexSz(szBuff)] = 1;

			szBuff[cbsz - 1] = '\n';    /* restore line feed */
			if (fputs(szBuff, pStmDst) == EOF)
				Error("Write error to opusnew.map");
			}
		}

	for (iSeg = 0; iSeg < cCodeSeg; iSeg++)
		{
		if (rgSegHit[iSeg] == 0)
			fprintf(pStmDst, " %4.4X:0000       DUMMY%4.4X\n", iSeg, iSeg);
		}

	fprintf(pStmDst, "\n%s\n", szBuff);

	printf("Completed successfully!!\n\n");
	exit(0);
}




SortSymb(ppsz1, ppsz2)
char **ppsz1;
char **ppsz2;
{
	return strcmp(*ppsz1, *ppsz2);
}


ReadSymbFile()
{
	int cbsz;
	BOOL fSort = fFalse;
	char *pszLast = "";

	for (cszSymb = 0; fgets(szBuff, cchStrMax, pStmSymb) != NULL;)
		{
		cbsz = strlen(szBuff);

		if (cbsz < 2)
			continue;

		if (cszSymb > (cszSymbMax - 2))
			Error("Out of memory");

		if (szBuff[cbsz - 1] == '\n')
			szBuff[--cbsz] = 0;   /* trash line feed */

		if (!fSort && (strcmp(szBuff, pszLast) < 0))
			fSort = fTrue;

		/* allocate memory for the symbol */
		if ((pszLast = malloc(cbsz + 1)) == NULL)
			Error("Out of memory");
		rglpszSymb[cszSymb] = pszLast;

		/* save the symbol */
		strcpy(pszLast, szBuff);
		cszSymb++;
		}

	if (!feof(pStmSymb))
		Error("Read error in symbols file");

	if (fSort)
		qsort(rglpszSymb, cszSymb, sizeof (char *), SortSymb);
}


fIsSymbToSave()
{
	register int iszTest;
	register int irgTest;
	int iszSymb;
	int irgLow, irgHigh;
	int cmpRet;
	char szTest[cchStrMax];

	if (strlen(szBuff) < 17)
		return (fFalse);

	/* return true for all DWINTER symbols */
	if (strncmp(szBuff, " 0001:", 6) == 0)
		return (fTrue);

	/* return true for the symbol the pcode debugger uses to get the data seg */
	if (strcmp(&szBuff[17], "mpsbps") == 0)
		return (fTrue);

	strcpy(szTest, &szBuff[17]);

	for (iszTest = 0; (szTest[iszTest] != 0) && (szTest[iszTest] != ' '); iszTest++)
		;

	szTest[iszTest] = 0;

	for (irgLow = 0, irgHigh = cszSymb - 1; (irgHigh - irgLow) >= 2;)
		{
		irgTest = ((irgHigh - irgLow) / 2) + irgLow;
		if ((cmpRet = strcmp(rglpszSymb[irgTest], szTest)) == 0)
			return fInclude;
		if (cmpRet > 0)
			irgHigh = irgTest;
		else
			irgLow = irgTest;
		}
	for (irgTest = irgLow; irgTest <= irgHigh; irgTest++)
		if (strcmp(rglpszSymb[irgTest], szTest) == 0)
			return fInclude;

	if ((szTest[0] == 'Q') && (szTest[1] == '_'))
		{
		for (irgLow = 0, irgHigh = cszSymb - 1; (irgHigh - irgLow) >= 2;)
			{
			irgTest = ((irgHigh - irgLow) / 2) + irgLow;
			if ((cmpRet = strcmp(rglpszSymb[irgTest], &szTest[2])) == 0)
				return fInclude;
			if (cmpRet > 0)
				irgHigh = irgTest;
			else
				irgLow = irgTest;
			}
		for (irgTest = irgLow; irgTest <= irgHigh; irgTest++)
			if (strcmp(rglpszSymb[irgTest], &szTest[2]) == 0)
				return fInclude;
		}
	return !fInclude;
}



Error(pszError)
char *pszError;
{
	printf("\n\nError - %s!\n\n", pszError);
	exit(1);
}


GetIntFromHexSz(psz)
char near *psz;
{
	int i;

	while (!isxdigit(*psz))
		psz++;

	for (i = 0; isxdigit(*psz); psz++)
		{
		i *= 16;
		if (*psz > '9')
			i += *psz - ('A' - 10);
		else
			i += *psz - '0';
		}

	return (i);
}


BadCommandLine()
{
	printf("\n\nBad command line args\n\n");
	printf("To execute use the command line:\n");
	printf("  stripmap [/e] fn_source_map fn_destination_map fn_symbols_list\n\n\n");
	exit(1);
}


