/* vgrep.c */

/* this program reads two files and sends to the third file everything
   in the first file which is not in the second file.

   Usage: vgrep srcfile lstfile outfile

	PLAN: since this doesn't have to be very efficient, I will do it
	the easy way.  for each element in the first file, search the
	second file.  if the element is not found, send it to outfile.
*/

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define fTrue 1
#define fFalse 0

char *szName;

main(cArg, rgpchArg)
int cArg;
char *rgpchArg[];
{
	FILE *fpSrc, *fpLst, *fpOut;
	char szLine[128];

	szName = rgpchArg[0];
	
	if (cArg < 4)
		Error("incorrect number of arguments.\nusage: vgrep srcfile lstfile outfile", "");

	if ((fpSrc = fopen(rgpchArg[1], "rt")) == NULL)
		Error("cannot open source file: ", rgpchArg[1]);

	if ((fpLst = fopen(rgpchArg[2], "rt")) == NULL)
		Error("cannot open list file: ", rgpchArg[2]);

	if ((fpOut = fopen(rgpchArg[3], "wt")) == NULL)
		Error("cannot create/open output file: ", rgpchArg[3]);

	while(!feof(fpSrc) && fgets(szLine, sizeof(szLine), fpSrc))
		if (!FSzInFp(szLine, fpLst))
			fputs(szLine, fpOut);

	fcloseall();
	exit(0);
}

Error(sz1, sz2)
char *sz1, *sz2;
{
	fprintf(stderr, "%s: error: %s%s\n\n", szName, sz1, sz2);
   fcloseall();
	exit(1);
}

FSzInFp(sz, fp)
char *sz;
FILE *fp;
{
	char szList[128];

	if (fseek(fp, 0L, SEEK_SET))
		Error("cannot seek list file");

	while(!feof(fp) && fgets(szList, sizeof(szList), fp))
		if (!strcmpi(sz, szList))
			return fTrue;

	return fFalse;
}


