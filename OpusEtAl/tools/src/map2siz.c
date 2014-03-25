/* M A P 2 S I Z E */

/* Given a map file, create a list of segment sizes */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#define fTrue 1
#define fFalse 0

#define ichMaxLine 256

FILE *fopen();

int nFormat = 0;

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	FILE *fpMap;

	if (cArg < 2)
		Error("missing filename");

	if (cArg > 2 && rgszArg[2][0] == '-')
		nFormat = atoi (&rgszArg[2][1]);

	if ((fpMap = fopen(rgszArg[1], "rt")) == NULL)
		Error("cannot open map file");

	if (!FSkipToSegs(fpMap))
		Error("map not well formed");

	ProcessSegs(fpMap);

	fcloseall();
	exit (0);
}


FSkipToSegs(fpMap)
FILE *fpMap;
{
	char rgchLine[ichMaxLine];
#define szStart " Start     Length     Name"

	for (;;)
		{
		fgets(rgchLine, ichMaxLine, fpMap);
		if (feof(fpMap))
			return fFalse;
		if (!strnicmp(rgchLine, szStart, sizeof(szStart)-1))
			return fTrue;
		}
}


ProcessSegs(fpMap)
FILE *fpMap;
{
	int nSeg, nLength;
	int nSegCur = -1, nLengthSeg = 0;
	int fEnd = fFalse;
	char szName[30], szClass[30], szNameSeg[30];
	char rgchLine[ichMaxLine];

	while (!fEnd)
		{
		fgets(rgchLine, ichMaxLine, fpMap);
		if (rgchLine[0] == ' ' && isdigit(rgchLine[1]))
			sscanf(rgchLine, "%X:%*X %XH %30s %30s", &nSeg, &nLength, 
					szName, szClass);
		else
			{
			fEnd = fTrue;
			nSeg = -2;
			}
		if (nSeg != nSegCur)
			{
			if (nSegCur >= 0)
				switch (nFormat)
					{
				case 0: /* # name len */
					printf(" %2.2X   %-16s  %5d\n", nSegCur,
							szNameSeg, nLengthSeg);
					break;
				case 1: /* len name # */
					printf("%5d   %-14s (%2.2X)\n", nLengthSeg, 
							szNameSeg, nSegCur);
					break;
				case 2: /* name # len */
					printf(" %-14s(%2.2X)   %5d\n", szNameSeg,
							nSegCur, nLengthSeg);
					break;
					}
			if (!strcmp(szClass, "BEGDATA"))
				strcpy(szNameSeg, "Data");
			else
				strcpy(szNameSeg, szName);
			nSegCur = nSeg;
			nLengthSeg = nLength;
			}
		else
			nLengthSeg += nLength;
		}
}




Error(sz)
char *sz;
{
	fprintf(stderr, "map2siz: error: %s\n\n", sz);
	fprintf(stderr, "usage: map2siz <map_file_name> [-0|1|2]\n");
	fprintf(stderr, "   -0 seg#   name  length\n");
	fprintf(stderr, "   -1 length name  seg#\n");
	fprintf(stderr, "   -2 name   seg#  length\n");
	exit (1);
}





