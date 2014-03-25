#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long time();

main(cArg, rgpchArg)
int cArg;
char *rgpchArg[];
{
	int nWait = 30;
	char *pchFile = NULL;
	int iArg;

	for (iArg = 1; iArg < cArg; iArg++)
		if (rgpchArg[iArg][0] == '/' || rgpchArg[iArg][0] == '-')
			switch (rgpchArg[iArg][1])
				{
			case 'S': 
			case 's':
				nWait = atoi(&rgpchArg[iArg][2]);
				break;
			default:
				Error("Unknown switch.");
				}
		else  if (pchFile == NULL)
			pchFile = rgpchArg[iArg];
		else
			Error("Too many arguments.");

	printf("Waiting for %s (%d second sleeps).\r\n", pchFile, nWait);

	while (!FExists(pchFile))
		Sleep(nWait);

	exit(0);
}


Error(pch1)
char *pch1;
{
	fprintf(stderr, "waitfile: error: %s\r\n", pch1);
	exit(2);
}


int FExists(szFile)
char *szFile;
{
	FILE *fopen();
	int fReturn;
	fprintf(stderr, ".");
	fReturn = fopen(szFile, "r") != NULL;
	fcloseall();
	return fReturn;
}


Sleep(nSec)
int nSec;
{
	long lStop = time(NULL) + nSec;
	while (time(NULL) < lStop)
		;
}



