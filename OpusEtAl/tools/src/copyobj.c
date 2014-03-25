/*  COPYOBJ.EXE  -- Copy obj utility for OPUS

	History:

	5/05/88  Created - gregc
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <spawn.h>
#include <string.h>
#include <malloc.h>

#define fFalse  0
#define fTrue   1

#define BOOL    int

extern int errno;


char *rgszArg[15];

#define szMax 256

	char szDefIni[] = { 
	"makeopus.ini" 	};


FILE *hFileIni = NULL;
char szCashmere[szMax];
char szWordtech[szMax];
char szResource[szMax];
char szBuild[szMax];
char szDirCurr[szMax];
char szLineIn[szMax];

main( ipchMaxParm, ppchParm )
int   ipchMaxParm;
char  **ppchParm;
{
	int iArg;
	int isz;
	char sz[szMax];
	int RetVal;
	BOOL fPutobj;

	getcwd(szDirCurr, szMax);
	strupr(szDirCurr);
	strcpy(szCashmere, szDirCurr);
	strcpy(szBuild, szDirCurr);

	/* process command line arguments */

	ipchMaxParm--;
	ppchParm++;

	if (ipchMaxParm < 1)
		GiveUsage();

	rgszArg[0] = "command.com";
	if ((*ppchParm)[0] != '-')
		GiveUsage();
	switch ((*ppchParm)[1])
		{
	case 'p':
	case 'P':
		fPutobj = fTrue;
		rgszArg[1] = "/c pobj.bat";
		break;

	case 't':
	case 'T':
		fPutobj = fFalse;
		rgszArg[1] = "/c tobj.bat";
		break;

	default:
		GiveUsage();
		}

	ipchMaxParm--;
	ppchParm++;

	if ((ipchMaxParm > 0) && (*ppchParm)[0] == '@')
		{
		if ((*ppchParm)[1] != '\0')
			if ((hFileIni = fopen(&(*ppchParm)[1], "rt")) == NULL)
				GiveUsage();
		ipchMaxParm--;
		ppchParm++;
		}
	else
		hFileIni = fopen(szDefIni, "rt");

	for (iArg = 2; ipchMaxParm > 0; iArg++, ipchMaxParm--, ppchParm++)
		{
		char *pch;

		pch = rgszArg[iArg] = *ppchParm;
		if (*pch++ == '-')
			{
			for (; *pch; pch++)
				{
				switch (*pch)
					{
				case 'f':
				case 'F':
					if (!fPutobj)
						GiveUsage();
					break;

				case 'd':
				case 'D':
				case 's':
				case 'S':
					if (fPutobj)
						GiveUsage();
					break;

				case 'v':
				case 'V':
					break;

				default:
					GiveUsage();
					}
				}
			}
		else if (ipchMaxParm != 1)
			GiveUsage();
		}
	rgszArg[iArg] = NULL;

	if ((isz = FindStrInFile("OPUS")) != -1)
		{
		strcpy(szCashmere, &szLineIn[isz]);
		strcpy(szBuild, szCashmere);
		}

	strcpy(szWordtech, szCashmere);
	strcat(szWordtech, "\\WORDTECH");
	strcpy(szResource, szCashmere);
	strcat(szResource, "\\RESOURCE");

	if ((isz = FindStrInFile("BUILD")) != -1)
		strcpy(szBuild, &szLineIn[isz]);
	if ((isz = FindStrInFile("WORDTECH")) != -1)
		strcpy(szWordtech, &szLineIn[isz]);
	if ((isz = FindStrInFile("RESOURCE")) != -1)
		strcpy(szResource, &szLineIn[isz]);

	strcpy(sz, "OPUS_DIR=");
	strcat(sz, szCashmere);
	strcpy(szCashmere, sz);
	if (putenv(szCashmere) != 0)
		{
		fprintf (stderr, "ERROR - Out of environment space.\n");
		exit (1);
		}

	strcpy(sz, "WORDTECH_DIR=");
	strcat(sz, szWordtech);
	strcpy(szWordtech, sz);
	if (putenv(szWordtech) != 0)
		{
		fprintf (stderr, "ERROR - Out of environment space.\n");
		exit (1);
		}

	strcpy(sz, "BUILD_DIR=");
	strcat(sz, szBuild);
	strcpy(szBuild, sz);
	if (putenv(szBuild) != 0)
		{
		fprintf (stderr, "ERROR - Out of environment space.\n");
		exit (1);
		}

	strcpy(sz, "START_DIR=");
	strcat(sz, szDirCurr);
	strcpy(szDirCurr, sz);
	if (putenv(szDirCurr) != 0)
		{
		fprintf (stderr, "ERROR - Out of environment space.\n");
		exit (1);
		}

	strcpy(sz, "RESOURCE_DIR=");
	strcat(sz, szResource);
	strcpy(szResource, sz);
	if (putenv(szResource) != 0)
		{
		fprintf (stderr, "ERROR - Out of environment space.\n");
		exit (1);
		}

	RetVal = spawnvp( P_WAIT, rgszArg[0], rgszArg);
	if (RetVal != 0)
		printf("spawnvp = %d    errno = %d\n", RetVal, errno);
}


FindStrInFile(szStr)
char *szStr;
{
	int isz, iszT;
	int cbsz = strlen(szStr);
	char ch;

	if (hFileIni != NULL)
		{
		fseek(hFileIni, 0L, 0);
		while (fgets(szLineIn, szMax, hFileIni) != NULL)
			{
			strupr(szLineIn);
			if (strncmp(szStr, szLineIn, cbsz) == 0)
				{
				for (isz = cbsz; (ch = szLineIn[isz++]) != '=';)
					if (ch == '\0')
						return -1;
				for (; szLineIn[isz] == ' '; isz++)
					;
				if (szLineIn[isz] == '\0')
					return -1;
				for (iszT = isz; ch = szLineIn[iszT]; iszT++)
					{
					if ((ch == '\n') || (ch == '\r'))
						{
						szLineIn[iszT] = '\0';
						break;
						}
					}
				return isz;
				}
			}
		}
	return -1;
}



GiveUsage()
{
	puts("Usage:   copyobj direction_switch [@<ini_file_name>] [switches] [user name]");
	puts("");
	puts("Direction switch:");
	puts("      -p       copy objs to server (put)");
	puts("      -t       copy objs from server (take)");
	puts("");
	puts("Ini_file_name:");
	puts("   use same file used in makeopus. e.g., ");
	puts("      copyobj -p @debug.inf -v ");
	puts("   if omitted, makeopus.inf used ");
	puts("");
	puts("Switches:");
	puts("   Direction = put");
	puts("      -f       force");
	puts("      -v       verbose (leaves echo on)");
	puts("   Direction = take");
	puts("      -d       just remove obj of currently checked out file (don't take obj's)");
	puts("      -s       synced - does not delete any objects after copy");
	puts("      -v       verbose (leaves echo on)");
	puts("");
	puts("User name:");
	puts("   if used, objects put in \\private\\usr\\<username>\\obj instead of \\private\\obj");
	puts("\n\n");
	exit(1);
}


