/*
*          RevCnt   UTILITY to count reviews in code by email name.
*
*    danp   3 Jun 88
*    
*    usage: revcnt names reviews 
*        counts occurances of each name in names in file reviews.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define fTrue 1
#define fFalse 0

#define ichMaxLine 256
#define iMaxNames	50

FILE *fpName, *fpReview;
FILE *fopen();

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	int fFound = fFalse;
	int cUnknown = 0;
	int cLines = 0;
	int iName, iszMaxName;
	char szReview[ichMaxLine];
	char *rgszName[iMaxNames];
	int  rgcName[iMaxNames];
	int fShowUnknown = fFalse;
	char *pchShowName = NULL;
	int cShown = 0;

	/*init array*/
	for (iName=0; iName<iMaxNames; rgcName[iName++]=0);

	if (cArg == 4 && rgszArg[3][0] == '-' && rgszArg[3][1] == 'u')
		fShowUnknown = fTrue;

	else  if (cArg == 4 && rgszArg[3][0] == '-' && rgszArg[3][1] == 'n')
		{
		pchShowName = &rgszArg[3][2];
		strlwr(pchShowName);
		}

	/* open files */
	if (cArg < 3 || !FOpenFiles(rgszArg))
		Usage();

	else  if ( (iszMaxName = FillNames(rgszName)) != fFalse)
		{

		/* get current szReview */
		fgets(szReview, ichMaxLine, fpReview);
		strlwr(szReview);
		KillCr(szReview);

		while (!feof(fpReview))
			{
			cLines++;

			if (pchShowName && FNameInLine(pchShowName,szReview))
				{
				cShown++;
				printf("%.79s\n",szReview);
				}

			for (iName = 0; iName < iszMaxName; iName++)
				if (FNameInLine(rgszName[iName],szReview))
					fFound = ++rgcName[iName];

			if (!fFound)
				{
				cUnknown++;
				if (fShowUnknown)
					{
					printf("%.79s\n",szReview);
					cShown++;
					}
				}

			/* reset for next line */
			fFound = fFalse;
			fgets(szReview, ichMaxLine, fpReview);
			strlwr(szReview);
			KillCr(szReview);
			}

		if (!fShowUnknown && !pchShowName)
			{
			Sort(rgcName, rgszName, iszMaxName);
			printf("\nSummary:\n");
			for (iName = 0; iName < iszMaxName; iName++)
				if (rgcName[iName] != 0)
					printf("%s     \t%d\n",rgszName[iName],rgcName[iName]);
			printf("\nUnknown      \t%d\n",cUnknown);
			printf("TOTAL      \t%d\n\n",cLines);
			}
		else
			printf("Total      \t%d\n\n",cShown);
		}

	fcloseall();
}


KillCr(pch)
char *pch;
{
	register int ch;
	while ((ch = *pch) != 0)
		{
		if (ch == '\r' || ch == '\n')
			{
			*pch = 0;
			return;
			}
		if (ch == '\t')
			*pch = ' ';
		pch++;
		}
}


/* Quick and Dirty (and not too efficient) Sort to decending order */
Sort(rgwKey, rgw2, cw)
int *rgwKey, *rgw2, cw;
{
	int *pw1, *pw2, *pwMac, iw1, iw2;

	for (pw1 = rgwKey, iw1 = 0, pwMac = rgwKey+cw; pw1 < pwMac; pw1++, iw1++)
		if (*pw1)
			for (pw2 = pw1+1, iw2 = iw1+1; pw2 < pwMac; pw2++, iw2++)
				if (*pw2 && *pw2 > *pw1)
					Swap(rgwKey, rgw2, iw1, iw2);
}


Swap(rgwKey, rgw2, iw1, iw2)
int *rgwKey, *rgw2, iw1, iw2;
{
	int wKey, w2;
	wKey = rgwKey[iw1];
	w2 = rgw2[iw1];
	rgwKey[iw1] = rgwKey[iw2];
	rgw2[iw1] = rgw2[iw2];
	rgwKey[iw2] = wKey;
	rgw2[iw2] = w2;
}


FOpenFiles(psz)
char **psz;
{
	if ((fpName = fopen(*(++psz), "r")) != NULL &&
			(fpReview = fopen(*(++psz), "r")) != NULL)
		return fTrue;

	else
		{
		printf("Error: cannot open %s\r\n", *psz);
		fcloseall();
		return fFalse;
		}
}


FillNames(rgszName)
char *rgszName[];
{
	int iName = 0;
	int cCh;
	char szName[ichMaxLine];
	char *pch;

	fgets(szName, ichMaxLine, fpName);

	while (!feof(fpName) && (iName < iMaxNames))
		{
		cCh = strlen(szName);
		pch = malloc(cCh);
		szName[cCh-1] = '\0';
		strlwr(szName);
		strcpy(pch,szName);
		rgszName[iName++] = pch;
		fgets(szName, ichMaxLine, fpName);
		}

	if (!feof(fpName))
		{
		printf("\n ! Too many names in name file. !\n");
		return fFalse;
		}
	else
		return iName;
}


FNameInLine(pchName, pchLine)
char *pchName, *pchLine;
{
	int cchName = strlen(pchName);
	char chFirstName = *pchName;

	/* move past file name */
	while (*pchLine && *pchLine != ':')
		pchLine++;

	for (;;)
		{
		/* skip non-alpha characters (unless name is non-alpha) */
		while (*pchLine && !(isalpha(*pchLine) || *pchLine == chFirstName))
			pchLine++;

		/* have a name, is it the one we wnat? */
		if (*pchLine)
			if (!strncmp(pchName, pchLine, cchName) && !isalpha(pchLine[cchName]))
				/* yes! */
				return fTrue;
			else
				/* skip to next non-alpha character to find next name */
				while (*++pchLine && isalpha(*pchLine))
					;
		else
			return fFalse;
		}
}


Usage()
{
	printf("usage: revcnt <names> <reviews> \r\n");
	printf("\toccurances of names in reviews are counted.\r\n");
	printf("\tlines with no names are counted in unknown.\r\n");
}


