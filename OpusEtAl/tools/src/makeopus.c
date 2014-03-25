/* #define DEBUG */

/*	MAKEOPUS.C  -- Make shell for building OPUS

	History:

	Created 5/23/86 by Bryan Loofbourrow
	6/18/86  fixed setting of COMSPEC - Peter Jackson
	8/1/86   partial conversion to OPUS.  still using \cashmere dir -peterj
	9/8/86   removed RAMDISK support.
	10/17/86 assume current drive instead of explicit drive C - fordm
	4/9/87   more thorough elimination of debugging modules with -s (bryanl)
	         no more explicit cashmere dir
	11/12/87 munged so generates batch files instead of directly
	         executing makes - gregc
	12/4/87  added "echoerr *** Linking..." - rosiep
	???????  made makes table driven; added + options and target spec. (bradch)
	1/27/88  added -p flag  - rosiep
	2/4/88   added -x , -t flags  - fordm
	2/17/88  fixed problem with full make of el and sdm libs (bradch)
	4/29/88  converted to nmake.exe - gregc
	5/19/88  made -x almost identical to -s - bradv
	10/7/88  changed I option to use new ilink bz
	8/17/89  added fos2opt and other changes for doing an os2 make; also
	         added -q flag and INC2_DIR; quoting is by default on - t-johnlo
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <spawn.h>
#include <string.h>
#include <malloc.h>

#define fFalse  0
#define fTrue   1

char *getenv();

extern int errno;


int fFoption=fFalse;
int fKoption=fFalse;
int fLoption=fFalse;
int fSoption=fFalse;
int fVoption=fFalse;
int fNoption=fFalse;
int fPoption=fFalse;
int fQoption=fTrue;
int fXoption=fFalse;
int fToption=fFalse;
int fIoption=fTrue;
int f386Max=fFalse;
int fos2opt=fFalse;

/* LIST OF "DFLAGS" PASSED TO MAKE VIA THE COMMAND LINE */
/* These are given to the compiler, and act as though a #define had been done */
/* extra space is allowed to handle command-line additions & "DEBUG" */

#define cszDefineInit   2
#define cszDefineMax    15

	char    *rgszDefine [cszDefineMax] = { 
	"CRLF", "WIN" 	};


int     iszDefineMac = cszDefineInit;


char *rgszMassMake1[] =
	{
	"rm",
	"*.obj",
	"*.res",
	"*.snt",
	"*.sdm",
	"*.hs",
	"*.hb",
	"*.hg",
	"*.hc",
	"*.hi",
	"*.im",
	"*.sy",
	"*.lnk",
	"*.tbx",
	NULL
	};


char *rgszMassMake2[] =
	{
	"rm",
	"sdm.lib",
	"menuhelp.h",
	"menuhelp.txt",
	"elxdefs.h",
	"verdate.h",
	"opuscmd.asm",
	"opusndb.def",
	NULL
	};


#define ichCmdLineMax   120
char rgchCmdLine [ichCmdLineMax * 2];

/* REVIEW - fix this */

/* rgszPmap1 and rgszPmap4 are initialized in the main function itself */

char *rgszPmap1[6];
	char *rgszPmap2[] = { 
	"rm", "pcodemap.obj", NULL 	};


	char *rgszPmap3[] = { 
	"rm", "dbg.snt", NULL 	};


#define iszPmapF  4   /* index for -f flag (if present) of pmap */

char *rgszPmap4[8];

#define szMax 256

	char szDefIni[] = { 
	"makeopus.ini" 	};


	char szFileCmd[] = { 
	"makeopus.cm" 	};


FILE *hFileOut;
FILE *hFileCmd;
FILE *hFileIni = NULL;

	char szDefWordtech[] = { 
	"wordtech" 	};


	char szDefLib[] = { 
	"lib" 	};


char szCashmere[szMax];
char szBuild[szMax];
char szNam[szMax];
char szDirCurr[szMax];
char szLineIn[szMax];
char szToolsDir[szMax];
	char szTarget [64] = { 
	0 	};


char szAppName[] = "makeopus";
char szMakeCmd [130];


int CchCopySz(char *, char *);

#define iArgMax 30

main( ipchMaxParm, ppchParm )
int   ipchMaxParm;
char  **ppchParm;
{
	int ich;
	int isz;
	char *pch;
	char *pchT;
	char *rgpch [30];
	char **ppch;
	int ipchMax;
	char *pchXtraArgs;

#ifdef DEBUG
	static char *rgszRAM [2] = 
		{ 
		"MAPMEM", NULL 		};

	spawnvp( P_WAIT, rgszRAM [0], rgszRAM );
#endif

	getcwd(szDirCurr, szMax);
	strupr(szDirCurr);
	CchCopySz(szDirCurr, szCashmere);
	CchCopySz(szDirCurr, szBuild);

	putenv("MAKEOPUS=123456789012345678901234567890123456789012345678901234567890");

	/* add extra command line arguments if any */

	blt( ppchParm, ppch = rgpch, ipchMax = ipchMaxParm );

#ifdef DEBUG
		{
		int ipch;

		printf("Command arguments: ");
		for ( ipch = 0; ipch < ipchMax-1; ipch++ )
			printf("%u:%s,",ipch,rgpch [ipch]);
		printf("%u:%s\r\n",ipch,rgpch [ipch]);
		}
#endif

	/* process command line arguments */

	ppch++;  
	ipchMax--;    /* Skip bogus "command name" entry */

	if ((ipchMax > 0) && (*ppch)[0] == '@')
		{
		if ((*ppch)[1] != '\0')
			if ((hFileIni = fopen(&(*ppch)[1], "rt")) == NULL)
				{
				GiveUsage();
				exit(1);
				}
		ppch++;
		ipchMax--;
		}
	else
		hFileIni = fopen(szDefIni, "rt");

	if ((isz = FindStrInFile("OPUS")) != -1)
		{
		CchCopySz(&szLineIn[isz], szCashmere);
		CchCopySz(szCashmere, szBuild);
		}

	CchCopySz(szCashmere, szToolsDir);

	if ((isz = FindStrInFile("BUILD")) != -1)
		CchCopySz(&szLineIn[isz], szBuild);

	if ((isz = FindStrInFile("NAM")) != -1)
		CchCopySz(&szLineIn[isz], szNam);

	if ((isz = FindStrInFile("MAKEOS2")) != -1)
		{
		fos2opt = fTrue;
		strcat(szToolsDir, "\\tools\\os2");
		}
	else
		strcat(szToolsDir, "\\tools\\dos");

	if (strcmp(szDirCurr, szBuild) != 0)
		if (!fChdirszBuild(szBuild))
			Abort();

	if (fos2opt)
		hFileOut = fopen("mo1.cmd", "wt");
	else
		hFileOut = fopen("mo1.bat", "wt");

	hFileCmd = fopen(szFileCmd, "wt");

	if ((isz = FindStrInFile("OPFL")) != -1)
		{
		int iszEnd;
		int iArg;
		int iArgMac;
		int cbsz;
		char *psz;
		char *rgArg[iArgMax];
		char ch;

		for (iArgMac = 0; iArgMac < iArgMax; iArgMac++)
			{
			for (; szLineIn[isz] == ' '; isz++)
				;

			if (szLineIn[isz] == '\0')
				break;

			for (iszEnd = isz + 1; ((ch = szLineIn[iszEnd]) != '\0') &&
					(ch != ' '); iszEnd++)
				;

			cbsz = iszEnd - isz;
			psz = rgArg[iArgMac] = malloc(cbsz + 1);
			memcpy(psz, &szLineIn[isz], cbsz);
			*(psz + cbsz) = '\0';
			isz = iszEnd + 1;
			}

		SetSwitch(rgArg, iArgMac);
		}

	SetSwitch(ppch, ipchMax);

	if (fXoption)
		fSoption = fTrue;

	if (fSoption && !fXoption && szTarget[0] != 0)
		{
		puts("Target specification is not allowed for Ship version!");
		Abort();
		}

	/* define the DEBUG flag if this is not a shipped version */

	if (!fSoption)
		rgszDefine [iszDefineMac++] = "DEBUG";

	if (fXoption)
		rgszDefine [iszDefineMac++] = "HYBRID";

	if (fSoption && !fXoption)
		rgszDefine [iszDefineMac++] = "FAST";

	if (fNoption)
		rgszDefine [iszDefineMac++] = "NONATIVE";

	if (fVoption)
#ifdef MAKEOPUS
		printf( "MAKEOPUS - Make shell for building OPUS\r\n\r\n" );
#endif
#ifdef MAKEO2
	printf( "MAKEO2 - Make shell for building OPUS\r\n\r\n" );
#endif

	/* if -f, force a mass make by deleting all generated files */
	if (fFoption)
		{
		if (getenv("SLM") != NULL)
			fprintf( stderr, "\007warning: net is up!\n" );

		if (!FPathSpawn( rgszMassMake1 ) || !FPathSpawn( rgszMassMake2 ))
			Abort();
		}


#ifdef REVIEW
	/* do these regardless whether -p or not; this sets up empty batch file */

	if (fos2opt)
		{
		rgszPmap1 = 
			{ 
			"cmd.exe", "/c", "echo.", "\>",
					"makepmap.cmd", NULL 			};
		rgszPmap4 = 
			{ 
			"cmd.exe", "/c", "echo", "pmap", "", ">>",
					"makepmap.cmd", NULL 			};
		}
	else
		{
		rgszPmap1 = 
			{ 
			"command", "/c", "echo.", "\>",
					"makepmap.bat", NULL 			};
		rgszPmap4 = 
			{ 
			"command", "/c", "echo", "pmap", "", ">>",
					"makepmap.bat", NULL 			};
		}

	if (!fSoption && !FSpawnRgsz( rgszPmap1[0], rgszPmap1 ))
		Abort();

	/* if -p, force pcodemap.txt to be remade for profiling */

	if (fQoption && (!fSoption || fXoption))	/* ignore for ship make */
		{
		if (fPoption)
			rgszPmap4[iszPmapF] = "-f";
		if (!FPathSpawn( rgszPmap2 ) || !FPathSpawn( rgszPmap3 ) ||
				!FSpawnRgsz( rgszPmap4[0], rgszPmap4 ))
			Abort();
		}
#endif /* REVIEW */

	if (!fVoption)
		fprintf(hFileOut, "echo off\n");

	fprintf(hFileOut, "set MAKEOPUS=\n");

	strcpy(rgchCmdLine, "PATH=");
	strcat(rgchCmdLine, szToolsDir);
	strcat(rgchCmdLine, ";");
	strcat(rgchCmdLine, szCashmere);
	strcat(rgchCmdLine, "\\tools");

	fprintf(hFileOut,"set %s\n", rgchCmdLine);

	strcpy(rgchCmdLine, "LIB=");
	if ((isz = FindStrInFile("INC_DIR")) != -1)
		{
		strcat(rgchCmdLine, &szLineIn[isz]);
		strcat(rgchCmdLine, ";");
		}
	strcat(rgchCmdLine, szCashmere);
	strcat(rgchCmdLine, "\\");
	strcat(rgchCmdLine, szDefLib);
	if ((isz = FindStrInFile("INC2_DIR")) != -1)
		{
		strcat(rgchCmdLine, ";");
		strcat(rgchCmdLine, &szLineIn[isz]);
		}

	fprintf(hFileOut,"set %s\n", rgchCmdLine);

	strcpy(rgchCmdLine, "INCLUDE=");
	if ((isz = FindStrInFile("INC_DIR")) != -1)
		{
		strcat(rgchCmdLine, &szLineIn[isz]);
		strcat(rgchCmdLine, ";");
		}

	strcat(rgchCmdLine, szCashmere);
	strcat(rgchCmdLine, ";");

	if ((isz = FindStrInFile("WORDTECH")) != -1)
		strcat(rgchCmdLine, &szLineIn[isz]);
	else
		{
		strcat(rgchCmdLine, szCashmere);
		strcat(rgchCmdLine, "\\");
		strcat(rgchCmdLine, szDefWordtech);
		}
	strcat(rgchCmdLine, ";");
	strcat(rgchCmdLine, szCashmere);
	strcat(rgchCmdLine, "\\lib;");
	strcat(rgchCmdLine, szCashmere);
	strcat(rgchCmdLine, "\\asm");

	if ((isz = FindStrInFile("INC2_DIR")) != -1)
		{
		strcat(rgchCmdLine, ";");
		strcat(rgchCmdLine, &szLineIn[isz]);
		}

	fprintf(hFileOut,"set %s\n", rgchCmdLine);

#ifdef DEBUG
	printf( "%s\r\n", rgchCmdLine );
	system ("set");
#endif

	DoMake();

#ifdef MAKEOPUS
	fprintf( stderr, "*** Make Complete\007\r\n" );
#endif
	exit (0);
}


/* D O   M A K E */
/* Actual make process is done here. */

DoMake()
{
	char *pchT;
	char *pch;
	int isz;
	int csz = 0;
	int RetVal;

	/* MAKE */

	/* quietly remove error.snt */
	fprintf(hFileOut, "touch error.snt\n");
	fprintf(hFileOut, "del error.snt\n");


	GenMakeCmdSz(szMakeCmd, szTarget);
	WriteMake();

	if (!fLoption)
		{
		/* Don't force a full link */
		fprintf(hFileCmd, "\"LINK_MODE=INC_LINK\"\n");
		}
	else
		{
		fprintf(hFileCmd, "\"LINK_MODE=FULL_LINK\"\n");
		}
	if (fIoption)
		fprintf(hFileCmd, "\"LINK_FLAG=/INC:16000\"\n");

	fcloseall();
	fflush(stdout);

#ifdef MAKEOPUS
	if (fos2opt)
		RetVal = spawnlp(P_WAIT, "cmd.exe", "cmd.exe", "/c mo1.cmd", NULL);
	else
		RetVal = spawnlp(P_WAIT, "command.com", "command.com", "/c mo1.bat", NULL);
	if (RetVal != 0)
		printf("spawnlp = %d    errno = %d\n", RetVal, errno);
#endif
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
#ifdef MAKEOPUS
	puts("Usage: makeopus [@ini_file_name] [+-d<FLAG>] [+-options]");
#endif
#ifdef MAKEO2
	puts("Usage: makeo2 [@ini_file_name] [+-d<FLAG>] [+-options]");
#endif
	puts("         +d<FLAG>  define <FLAG> for all compiles");
	puts("         -d<FLAG>  undefine a defined <FLAG>");
	puts("         +option   turn option on");
	puts("         -option   turn option off\n");
	puts("Options:");
	puts("         f         force mass compile (+k implied)");
	puts("         i<file>   specify file for /INC: switch to link4");
	puts("         k         do not stop for errors");
	puts("         l         force full link (via link4, not ilink)");
	puts("         n         disable cs native code generation");
	puts("         q         turn quoting on");
	puts("         s         generate ship (no DEBUG) version");
	puts("         t         test makeopus; do not execute mo2.bat(mo2.cmd) files");
	puts("         v         verbose");
	puts("         x         make hybrid version (debug, but without -DDEBUG)");
	puts("         3         use 386max openhigh feature during build\n");
	puts("In .ini file:");
	puts("   OPFL=<cmd line> commandline args to makeopus (as above)");
	puts("   NAM=<filename>  name for executable (without .exe)");
	puts("   EXE_DIR=<dir>   where to place executable");
	puts("   BUILD=<dir>     where to place objects");
	puts("   OPUS=<dir>      where to find sources");
	puts("   WORDTECH=<dir>  where to find wordtech sources");
	puts("   CMD=<dir>       where to find command table files");
	puts("   RES=<dir>       where to find resources");
	puts("   DLG=<dir>       where to find dialog descriptions");
	puts("   DATE=<string>   format of date to pass to echotime");
	puts("   USER=<string>   username to pass to echotime");
	puts("   INC_DIR=<dir>   directory to prepend to INCLUDE and LIB paths");
	puts("   INC2_DIR=<dir>  directory to append to INCLUDE and LIB paths");
	puts("   UF_CSL=<flags>  additional flags to pass to csl compiler");
	puts("   UF_ILINK=<flags>additional flags to pass to ilink");
	puts("   UF_LINK4=<flags>additional flags to pass to link4");
	puts("   UF_WINMARK=<flg>additional flags to pass to winmark");
	puts("   CHKMEM=<cmd>    command to execute to determine available memory");
	puts("   MMEM=           enables use of mmem for compiles");
	puts("   MAKEOS2=        allows the make to be done in protect mode");
	puts("   NOAPPLOADER=    disables the Apploader (normally enabled for non debug)");
	puts("   COVER=          leaves .ps, .im and .sy files in BUILD for Code Coverage\n");
#ifdef MAKEO2
	puts("After running MAKEO2, execute: command /c mo1");
#endif
}


SetSwitch(ppch, ipchMax)
int   ipchMax;
char  **ppch;
{
	int ich;
	char ch;
	while (ipchMax > 0)
		{
		if ((*ppch)[0] == '+')
			{
			char ch;

			for (ich = 1; (*ppch)[ich]; ich++ )
				{
				switch (toupper((*ppch)[ich]))
					{
				default:
					GiveUsage();
					exit(1);
					/* break; */

				case '3':
					f386Max = fTrue;
					break;

				case 'F':
					fFoption = fTrue;
					fLoption = fTrue;
					/* FALL THROUGH */

				case 'K':
					fKoption = fTrue;
					break;

				case 'L':
				case '1':
					fLoption = fTrue;
					break;

				case 'S':
					fSoption = fTrue;
					fXoption = fFalse;
					fLoption = fTrue;
					fIoption = fFalse;
					break;

				case 'V':
					fVoption = fTrue;
					break;

				case 'N':
					fNoption = fTrue;
					break;

				case 'P':
					fPoption = fTrue;
					/* fall through */

				case 'Q':
					fQoption = fTrue;
					break;

				case 'D':
					rgszDefine[iszDefineMac++] = &(*ppch)[ich + 1];
					goto LNextArg;

				case 'X':
					fXoption = fTrue;
					fSoption = fFalse;
					fLoption = fTrue;
					fIoption = fFalse;
					break;

				case 'T':
					fToption = fTrue;
					break;

				case 'I':
					fIoption = fTrue;
					break;
					}
				}
			}
		else  if ((*ppch)[0] == '-')
			{
			char ch;

			for (ich = 1; (*ppch)[ich]; ich++ )
				{
				switch (toupper((*ppch)[ich]))
					{
				default:
					GiveUsage();
					exit(1);
					/* break; */

				case '3':
					f386Max = fTrue;
					break;

				case 'F':
					fFoption = fFalse;
					break;

				case 'K':
					fKoption = fFalse;
					break;

				case 'L':
				case '1':
					fLoption = fFalse;
					break;

				case 'S':
					fSoption = fFalse;
					break;

				case 'V':
					fVoption = fFalse;
					break;

				case 'N':
					fNoption = fFalse;
					break;

				case 'P':
					fPoption = fFalse;
					/* fall through */

				case 'Q':
					fQoption = fFalse;
					break;

				case 'D':
						{
						int isz;

						for (isz = 0; isz < iszDefineMac; isz++)
							{
							if (strcmp(rgszDefine[isz], &(*ppch)[ich + 1]) == 0)
								{
								*rgszDefine[isz] = '\0';
								break;
								}
							}
						goto LNextArg;
						}

				case 'X':
					fXoption = fFalse;
					break;

				case 'T':
					fToption = fFalse;
					break;

				case 'I':
					fIoption = fFalse;
					break;
					}
				}
			}
		else
			{
			strcat(szTarget, *ppch);
			strcat(szTarget, " ");
			}

LNextArg:
		ppch++;  
		ipchMax--;
		}
}


GenMakeCmdSz(szBuf, szExtra)
char * szBuf;
char * szExtra;
{
	register char * pch;
	register int isz;

	pch = szBuf;
	pch += CchCopySz("NMAKE -nc @", pch);
	pch += CchCopySz(szFileCmd, pch);
	pch += CchCopySz(" -f ", pch);
	pch += CchCopySz(szCashmere, pch);
	pch += CchCopySz("\\makeopus ", pch);
	if ((szExtra != NULL) && (strlen(szExtra) > 0))
		pch += CchCopySz(szExtra, pch);
	if (fos2opt)
		pch += CchCopySz(" >> mo2.cmd", pch);
	else
		pch += CchCopySz(" >> mo2.bat", pch);

	fprintf(hFileCmd, "-r\n");

	fprintf(hFileCmd, "\"CHKERR=if errorlevel 1 ");

	if (fKoption)
		fprintf(hFileCmd, "makeerr %s\\error.snt\"\n", szBuild);
	else
		fprintf(hFileCmd, "goto error\"\n");

	if (fSoption && !fXoption)
		fprintf(hFileCmd, "\"VERSION=nondbg\"\n");
	if (fXoption)
		fprintf(hFileCmd, "\"VERSION=hybrid\"\n");

	fprintf(hFileCmd, "\"DFLAGS=");
	for (isz = 0; isz < iszDefineMac; isz++)
		if (*rgszDefine[isz] != '\0')
			fprintf(hFileCmd, " -D%s", rgszDefine[isz]);
	if (fXoption)
		fprintf(hFileCmd, " -DPROFILE");
	fprintf(hFileCmd, "\"\n");

	fprintf(hFileCmd, "\"PCF=");
	if (fQoption)
		{
		if (fSoption)
			fprintf(hFileCmd, "-O ");
		else
			fprintf(hFileCmd, "-oq ");
		}
	else if (fSoption)
		fprintf(hFileCmd, "-Oq ");
	if (fNoption)
		fprintf(hFileCmd, "-nn "); /* disable double bracketing */
	if (!fSoption || fXoption)
		fprintf(hFileCmd, "-nm");
	fprintf(hFileCmd, "\"\n");

	if ((isz = FindStrInFile("OPUS")) == -1)
		fprintf(hFileCmd, "\"OPUS=%s\"\n", szCashmere);

	if (hFileIni != NULL)
		{
		fseek(hFileIni, 0L, 0);
		while (fgets(szLineIn, szMax, hFileIni) != NULL)
			{
			/* REVIEW -- 
			 			strupr(szLineIn);
			upper casing here is NOT cool. It screws csl parameters.
			*/
			if (strncmp("OPFL", szLineIn, 4) != 0)
				{
				char *psz;

				for (psz = szLineIn; (*psz != '\0') && (*psz != '\r') &&
						(*psz != '\n'); psz++)
					;
				*psz = '\0';
				if (strlen(szLineIn) > 0)
					fprintf(hFileCmd, "\"%s\"\n", szLineIn);
				}
			}
		}
}


WriteMake()
{
	if (fos2opt)
		{
		fprintf(hFileOut, "echo rem > mo2.cmd\n");
		if (!fVoption)
			fprintf(hFileOut, "echo echo off >> mo2.cmd\n");
		}
	else
		{
		if (f386Max)
			fprintf(hFileOut, "386max openhigh\n");
		fprintf(hFileOut, "echo rem > mo2.bat\n");
		if (!fVoption)
			fprintf(hFileOut, "echo echo off >> mo2.bat\n");
		}

	fprintf(hFileOut, "%s\n", szMakeCmd);
	fprintf(hFileOut, "if errorlevel 1 goto error\n");

	if (fos2opt)
		{
		fprintf(hFileOut, "echo goto done >> mo2.cmd\n");
		fprintf(hFileOut, "echo :error >> mo2.cmd\n");
		fprintf(hFileOut, "echo makeerr %s\\error.snt >> mo2.cmd\n",	szBuild);
		fprintf(hFileOut, "echo :done >> mo2.cmd\n");
		if (strcmpi(szDirCurr, szBuild) != 0)
			fprintf(hFileOut, "echo call to %s >> mo2.cmd\n", szDirCurr);
		fprintf(hFileOut, "grep \"is up-to-date\" mo2.cmd > NUL\n");
		}
	else
		{
		fprintf(hFileOut, "echo goto done >> mo2.bat\n");
		fprintf(hFileOut, "echo :error >> mo2.bat\n");
		fprintf(hFileOut, "echo makeerr %s\\error.snt >> mo2.bat\n", szBuild);
		fprintf(hFileOut, "echo :done >> mo2.bat\n");
		if (f386Max)
			fprintf(hFileOut, "echo 386max closehigh >> mo2.bat\n");
		if (strcmpi(szDirCurr, szBuild) != 0)
			fprintf(hFileOut, "echo to %s >> mo2.bat\n", szDirCurr);
		fprintf(hFileOut, "grep \"is up-to-date\" mo2.bat > NUL\n");
		}

	fprintf(hFileOut, "if not errorlevel 1 goto skip\n");
	if (fToption)
		fprintf(hFileOut, "rem ");
	fprintf(hFileOut, "mo2\n");
	if (fToption)
		fprintf(hFileOut, "goto skip\n");
	fprintf(hFileOut, ":error\n");
	fprintf(hFileOut, "makeerr %s\\error.snt\n", szBuild);
	fprintf(hFileOut, ":skip\n");
	if (fos2opt)
		{
		if (strcmpi(szDirCurr, szBuild) != 0)
			fprintf(hFileOut, "call to %s\n", szDirCurr);
		}
	else
		{
		if (f386Max)
			fprintf(hFileOut, "386max closehigh\n");
		if (strcmpi(szDirCurr, szBuild) != 0)
			fprintf(hFileOut, "to %s\n", szDirCurr);
		}
}


WriteErrorCheck()
{
	if (!fKoption)
		fprintf(hFileOut, "if errorlevel 1 goto error\n");
}


CchCopySz( szSource, szDest )
char *szSource, *szDest;
{
	strcpy( szDest, szSource );
	return strlen( szSource );
}


blt( pwSrc, pwDest, cw )
int *pwSrc, *pwDest;
int cw;
{
	while (cw--)
		*pwDest++ = *pwSrc++;
}


Abort()
{
	perror( szAppName );
	fprintf (stderr, "\007*** Make Failed\007\r\n");
	exit (1);
}


/* F   P A T H   S P A W N */

FPathSpawn( rgsz )
char *rgsz[];
	{       /* puts the correct path at the beginning of rgsz[0]
           and calls FSpawnRgsz */
	char *rgsz0;

	strcpy(rgsz0, szToolsDir);
	strcat(rgsz0, "\\");
	strcat(rgsz0, rgsz[0]);
	return FSpawnRgsz(rgsz0, rgsz);
}


/* F   S P A W N   R G S Z */

FSpawnRgsz( rgsz0, rgsz )
char *rgsz0;
char *rgsz[];
	{       /* Spawn program indicated by rgsz0, with arguments
           from rgsz [1] to the first NULL entry in rgsz */
	int RetCode;

	if (fVoption)
		{
		int cch;
		char **psz;

		cch = strlen(rgsz0) + 1;
		printf( "%s ", rgsz0 );
		for ( psz = &rgsz[1]; *psz != NULL; psz++ )
			{
			printf( "%s ", *psz );
			cch += strlen(*psz) + 1;
			}

		printf( "\r\n" );

		if (cch > 128)
			{
			printf("Generated command line too long!\n");
			Abort();
			}
		}

	if (fToption)
		/* Test option - don't actually execute the command string */
		return fTrue;

	RetCode = spawnvp( P_WAIT, rgsz0 , rgsz );
	if (RetCode != 0)
		printf("spawnvp: file = %s error = %d  errno = %d\n", rgsz0, RetCode, errno);
	return RetCode == 0;
}


/* F   C H D I R   S Z B U I L D */

fChdirszBuild( szBuild )
char szBuild[szMax];
{
	int RetCode;

	if (fToption)
		/* Test option - don't actually execute the command string */
		return fTrue;

	RetCode = chdir( szBuild );

	if (RetCode != 0)
		printf("chdir: error = %d errno %d\n", RetCode, errno);
	else  if (szBuild[1] == ':')
		{
		char szT[3];
		szT[0] = szBuild[0];
		szT[1] = szBuild[1];
		szT[2] = 0;
		RetCode = system (szT);
		if (RetCode != 0)
			printf("system: error = %d errno %d\n", RetCode, errno);
		}

	return RetCode == 0;
}


