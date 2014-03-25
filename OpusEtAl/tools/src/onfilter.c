/* O N F I L T E R . C */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#define ichMaxLine 256
#define szGood "Completed normally"

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	char szLine[ichMaxLine];
	int nTest, nTestLast = 0;
	int nGood = 0, nBad = 0;
	int nSumGood = 0, nSumBad = 0, nSumMixed = 0;
	int nTotalRuns = 0, nTotalTests = 0;


	for (;;)
		{
		gets(szLine);

		if (feof(stdin))
			break;
		nTotalRuns++;

		nTest = atoi(szLine);

		if (nTestLast != 0 && nTest != nTestLast)
			{
			puts("");
			nTotalTests++;
			if ( (nGood!=0) && (nBad!=0) )
				nSumMixed++;
			if ( (nGood!=0) && (nBad==0) )
				nSumGood++;
			if ( (nGood==0) && (nBad!=0) )
				nSumBad++;
			nGood=nBad=0;
			}

		if (!strnicmp(szLine+31,szGood,sizeof(szGood)))
			nGood++;
		else
			nBad++;

		puts(szLine);
		nTestLast = nTest;
		}

	if ( (nGood!=0) && (nBad!=0) )
		nSumMixed++;
	if ( (nGood!=0) && (nBad==0) )
		nSumGood++;
	if ( (nGood==0) && (nBad!=0) )
		nSumBad++;

	fprintf(stdout,"\nTotal number of runs:  %d\n",nTotalRuns);
	fprintf(stdout,"Total tests executed:  %d\n",++nTotalTests);
	fprintf(stdout,"      All successful:  %d\n",nSumGood);
	fprintf(stdout,"      Mixed success:   %d\n",nSumMixed);
	fprintf(stdout,"      All runs failed: %d\n",nSumBad);
}


