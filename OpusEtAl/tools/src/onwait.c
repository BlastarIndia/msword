/* O N W A I T . C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <io.h>
#include <errno.h>

long time();

/* #define DEBUG */

#define fTrue 1
#define fFalse 0

int wRandConst;

main(cArg, rgpchArg)
int cArg;
char *rgpchArg[];
{
	srand((int)(time(NULL)%0x7ffeL));

	wRandConst = (rand()/2) & 0x7fff;

#ifdef DEBUG
	fprintf(stderr, "wRandConst = %d\n", wRandConst);
#endif /* DEBUG */

	if (cArg < 2)
		Error("Missing argument");

	if (rgpchArg[1][0] != '-' || rgpchArg[1][2] != 0)
		Error("Unknown argument");

	switch (rgpchArg[1][1])
		{
	case 'u':
		Usage();
		break;

	case '0':
			/* output between 1 and 15 blank lines */
			{
			int cLines = ((rand()/2)%15);
			while (cLines-- >= 0)
				fprintf(stderr, "\n");
			break;
			}

	case '1':
		fprintf(stderr, "Waiting until after 1am\r\n");
		while (WHour() < 1 || WHour() >= 7)
			Delay(10, 60);
		break;

	case '2':
		fprintf(stderr, "Waiting for DEBUG.SNT\r\n");
		while (! FCanOpenFile("debug.snt"))
			{
			fprintf(stderr, ".");
			Delay(5, 30);
			}
		while (! FCanOpenFile("opus.exe"))
			{
			fprintf(stderr, ",");
			Delay(5, 20);
			}
		break;

	case '3':
		fprintf(stderr, "Waiting until after 7am\r\n");
		while (WHour() < 7)
			Delay(20, 20);
		break;

	case '4':
		if (cArg < 3)
			Error("Missing argument");

		if (FFileHasBody(rgpchArg[2]))
			exit (0);
		else
			exit (1);

	case '5':
		if (cArg < 4)
			Error("Missing argument(s)");

		if (FDirTextHasCKb(rgpchArg[2], atoi(rgpchArg[3])))
			exit (0);
		else
			exit (1);

#ifdef DEBUG
	case 'h':
		printf("WHour = %d\r\n", WHour());
		break;

	case 'd':
			{
			int i;
			for (i = 1; i < 5; i++)
				{
				printf("Delay(0, %d) ... ", i);
				Delay(0, i);
				}
			break;
			}
#endif /* DEBUG */

	default:
		Error("Unknown argument");
		break;
		}
	exit(0);
}


int WHour()
{
	long lTime;
	time(&lTime);
	return ((struct tm *)localtime(&lTime))->tm_hour;
}


int FCanOpenFile(szFile)
char *szFile;
{
	extern int errno;
	int nFile = sopen(szFile, O_RDONLY, SH_DENYRW);
	if (nFile != -1)
		{
		close(nFile);
		return fTrue;
		}
#ifdef DEBUG
	else  if (errno != EACCES)
		{
		fprintf(stderr, "sopen returned %d\r\n", errno);
		exit(1);
		}
#endif /* DEBUG */
	return fFalse;
}


int FFileHasBody(sz)
char *sz;
{
	FILE *fopen();
	FILE *fp;
	int fReturn = fFalse;

	if ((fp = fopen(sz, "r")) == NULL)
		return fFalse;
	if (!fseek(fp, 0L, SEEK_END) && ftell(fp) > 1)
		fReturn = fTrue;
	fclose(fp);
	return fReturn;
}


Delay(wMin, wMax)
int wMin, wMax;
{
	int dSecRand = (wMax - wMin) * 30;
	long lStop = time(NULL) + (wMin * 60);

	if (dSecRand)
		lStop += (((rand()/2)&0x7fff) % dSecRand);

	if (dSecRand)
		lStop += (wRandConst % dSecRand);

#ifdef DEBUG
	fprintf(stderr, "delay = %d:%.2d\r\n", 
			(int)(lStop - time(NULL))/60, (int)(lStop - time(NULL))%60);
#endif /* DEBUG */

	while (time(NULL) < lStop)
		;
}


Error(sz)
char *sz;
{
	fprintf(stderr, "onwait: error: %s\r\n", sz);
	Usage();
	exit(1);
}


Usage()
{
	printf("ontest -n [arg1] [arg2]\nwhere n is one of:\n");
	printf("    0  display a random number of blank lines (1-15)\n");
	printf("    1  wait until after 1am\n");
	printf("    2  wait until .\\DEBUG.SNT exist and OPUS.EXE can be read\n");
	printf("    3  wait until after 7am\n");
	printf("    4  determine if [arg] exists and contains text\n");
	printf("    5  determine if [arg1] is dir with >= [arg2]K available\n");
#ifdef DEBUG
	printf("    h  print current hour\n");
	printf("    d  delay from 1 to four min\n");
#endif
	printf("\n");
}


int FDirTextHasCKb(sz, cKb)
char *sz;
int cKb;
{
	FILE *fopen();
	FILE *fp;
	int fReturn = fFalse;
	char *pch;
	char rgchLine [80];

#define szMarker "ile(s) "

	if ((fp = fopen(sz, "r")) == NULL)
		return fFalse;
	do
		{
		fgets(rgchLine, 80, fp);
		if ((pch = strstr(rgchLine, szMarker)) != NULL)
			{
			pch += sizeof(szMarker);
			if ((atol(pch)/1024L) >= cKb)
				fReturn = fTrue;
			break;
			}
		}
	while (!feof(fp));
	fclose(fp);
	return fReturn;
}




