/*
*          SymStrip   UTILITY to strip symbols listed in a file from another
*                             file.
*
*    danp   3 Jun 88
*    
*    usage: symstrip full remove dest
*        removes lines from full that appear in remove
*        places result in dest
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fTrue 1
#define fFalse 0

#define cchMaxLine 130

FILE *fpFull, *fpRemove, *fpDest;
FILE *fopen();

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	int wComp;
	int cSkipped = 0;
	char szFull[cchMaxLine];
	char szRemove[cchMaxLine];

	/* open files */
	if (cArg != 4 || !FOpenFiles(rgszArg))
		Usage();

	/* make sure source files are sorted properly! */
	else  if (FCheckSort(fpFull, 1) && FCheckSort(fpRemove, 2))
		{
		/* get current szRemove */
		fgets(szRemove, cchMaxLine, fpRemove);

		/* copy from full to dest, except remove */
		fgets(szFull, cchMaxLine, fpFull);
		while (!feof(fpFull))
			{
			while (!feof(fpRemove) && (wComp = strcmp(szFull, szRemove)) > 0)
				/* advance szRemove to catch up with szFull */
				fgets(szRemove, cchMaxLine, fpRemove);

			if (feof(fpRemove) || wComp != 0)
				fputs(szFull, fpDest);
			else
				cSkipped++;

			fgets(szFull, cchMaxLine, fpFull);
			}
		printf("Removed %d symbols\r\n", cSkipped);
		}

	fcloseall();
}


FOpenFiles(psz)
char **psz;
{
	if ((fpFull = fopen(*(++psz), "r")) != NULL &&
			(fpRemove = fopen(*(++psz), "r")) != NULL &&
			(fpDest = fopen(*(++psz), "w")) != NULL)
		return fTrue;

	else
		{
		printf("Error: cannot open %s\r\n", *psz);
		fcloseall();
		return fFalse;
		}
}


Usage()
{
	printf("usage: symstrip <full> <remove> <dest>\r\n");
	printf("\t<dest> = <full> - <remove>\r\n");
	printf("\tboth <full> and <remove> MUST be sorted\r\n");
	printf("\tremoval is case sensitive\r\n");
}


FCheckSort(fp, iArg)
FILE *fp;
int iArg;
{
	int cErrors = 0;
	char sz1[cchMaxLine];
	char sz2[cchMaxLine];
	char *pch1=sz1, *pch2=sz2, *pchT;

	fgets(pch1, cchMaxLine, fp);
	while (!feof(fp))
		{
		fgets(pch2, cchMaxLine, fp);

		/* pch1 is previous string, pch2 is next string */
		if (!feof(fp) && strcmp(pch1, pch2) > 0)
			{
			printf("symstrip: sort order error: \r\n\t%s\t%s", pch1, pch2);
			cErrors++;
			}
		pchT = pch1;
		pch1 = pch2;
		pch2 = pchT;
		}

	if (cErrors)
		printf("%d errors on argument %d\r\n", cErrors, iArg);
	else
		fseek(fp, 0L, SEEK_SET);

	return !cErrors;
}


