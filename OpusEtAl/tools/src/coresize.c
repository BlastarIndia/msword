/* C O R E S I Z E . C */
/*  this program reports on the current size of the Opus core. */

#define NDEBUG

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define fTrue 1
#define fFalse 0

#define cchArgMax   13 /* filename.obj */
#define cSizeCallsMax 10
#define cArgCallMax 6
#define cArgMax (cArgCallMax*cSizeCallsMax)
#define cchBufferMax 256

#define szListStart "!grouplists"
#define szListEnd "!endlists"
#define szPrefix  "ihcd"
#define szGroupEnd "End"


char szTempFile[13] = "coresize.tmp";
char szSpawnPath[] = "command";

char rgszArg[cArgMax][cchArgMax];
	char *rgpchArg[12] = { 
	szSpawnPath, "/c", "size" 	};


int cArg = 0;
int iArg;
#define ipchArgFirstOpt 3
#define ipchArgLimOpt 9

FILE *fpCore = NULL;

char *pchApp = NULL;




main (argc, argv)
int argc;
char *argv[];

{
	char **ppchGroup = &argv[1];
	int cGroup = argc - 1;
	pchApp = argv[0];

	if (!cGroup)
		/*  use *.obj */
		{
		bltb ("*.obj", rgszArg[0], sizeof("*.obj"));
		cArg = 1;
		goto LProcessArgs;
		}
	else  
		while (cGroup--)
			/* argv[1] is group name, get list of files in group */
			{
			ReadGroup (*ppchGroup++);
			if (cArg == 0)
				{
				Warning ("no names in group");
				continue;
				}

LProcessArgs:
			iArg = 0;
			while (iArg < cArg)
				ExecSize ();

			ProcessSizeOutput();
			}
#ifdef NDEBUG
	remove (szTempFile);
#endif /* NDEBUG */
	exit (0);
}


int FTokenCh (ch)
char ch;
{
	switch (ch)
		{
	case ' ':
	case '\t':
	case 0:
	case ',':
	case '\n':
	case '\r':
		return fFalse;
	default:
		return fTrue;
		}
}


int FPrefixPpch (pch, ppch)
char *pch, **ppch;

{
	while (*pch && *pch == **ppch)
		{
		pch++;
		(*ppch)++;
		}
	return !*pch;
}


char *PchGetToken (fSearchBang, fp, rgch, ppchRest)
int fSearchBang;
FILE *fp;
char *rgch;
char **ppchRest;
{
	char *pch1, *pch2;

	do
		{
		if ((pch1 = fgets (rgch, cchBufferMax, fp)) == NULL)
			return NULL;

		if (*pch1 == '!')  /* comment */
			continue;

		while (*pch1 && 
				(!FTokenCh(*pch1) || (fSearchBang && *pch1 != '!')))
			pch1++;
		pch2 = pch1;
		while (FTokenCh (*pch2))
			pch2++;

		} 
	while (pch1 == pch2);
	*pch2 = 0;
	*ppchRest = pch2+1;
	return pch1;
}


int FTokenEq (pch1, pch2)
char *pch1, *pch2;
{
	return !strcmpi (pch1, pch2);
}


/* open core.h and get the list of obj files for pchGroup */
ReadGroup (pchGroup)
char *pchGroup;
{
	int cch;
	char *pch, *pchRest;
	char rgchBuffer [cchBufferMax];

#ifndef NDEBUG
	fprintf (stderr, "ReadGroup ( %s )\n", pchGroup);
#endif /* NDEBUG */

	cArg = 0;

	if (fpCore == NULL && (fpCore = fopen ("core.h", "r")) == NULL)
		Error ("cannot open core.h");

	if (fseek (fpCore, 0l, SEEK_SET) != 0)
		Error ("cannot seek core.h");

	/* advance to EOF or begining of list */
	for (;;)
		{
		if ((pch = PchGetToken (fTrue, fpCore, rgchBuffer, &pchRest))
				== NULL)
			Error ("core list not found in core.h");
		if (*pch == '!' && FTokenEq (pch+1, szListStart))
			break;
		}

	/* at beginning of list, find group */
	for (;;)
		{
		if ((pch = PchGetToken (fTrue, fpCore, rgchBuffer, &pchRest))
				== NULL)
			Error ("unexpected end of core.h");
#ifndef NDEBUG
		fprintf (stderr, "group token = %s\n", pch);
#endif /* NDEBUG */
		if (*pch == '!')
			if (FTokenEq (pch+1, pchGroup))
				break;
			else  if (FTokenEq (pch+1, szListEnd))
				Error ("cannot find group");
		}

	/* at beginning of group, accumlate names */
	for (;;)
		{
		if ((pch = PchGetToken (fFalse, fpCore, rgchBuffer, &pchRest))
				== NULL)
			{
			Warning ("core list not properly terminated");
			break;
			}
#ifndef NDEBUG
		fprintf (stderr, "element = %s\n", pch);
#endif /* NDEBUG */
		if (*pch == '!')
			break;

		/*  advances over prefix "ihcd" */
		if (!FPrefixPpch (szPrefix, &pch))
			continue;

		if (FTokenEq (pch, szGroupEnd))
			/* reached ihcdEnd */
			break;

		while (*pchRest && *pchRest != '@')
			pchRest++;

		if (*pchRest == '@')
			/* optional @<filespec> specified */
			{
			pch = ++pchRest;
			while (FTokenCh (*pchRest))
				*pchRest++;
			*pchRest = 0;
#ifndef NDEBUG
			fprintf (stderr, "special = %s\n", pch);
#endif /* NDEBUG */
			}

		if (cArg >= cArgMax)
			{
			Warning ("too many modules in group, extras ignored");
			break;
			}

		cch = CchSz (pch);
		cch = max (cch, 9);
		bltb (pch, rgszArg[cArg], cch);
		AppendSzToSz (".obj", rgszArg[cArg++]);
		}
}




/* Exec command /c size foo.obj >[>] tempfile */
ExecSize ()
{
	int ipchArg;
	int fAppend = iArg;

	for (ipchArg = ipchArgFirstOpt; 
			ipchArg < ipchArgLimOpt && iArg < cArg; )
		{
		rgpchArg [ipchArg++] = rgszArg [iArg++];
		}

	rgpchArg [ipchArg++] = fAppend ? ">>" : ">";
	rgpchArg [ipchArg++] = szTempFile;
	rgpchArg [ipchArg] = NULL;

#ifndef NDEBUG
	fprintf (stderr, "\n");
	for (ipchArg = 0; rgpchArg[ipchArg]; ipchArg++)
		fprintf (stderr, "\t\t%s\n", rgpchArg[ipchArg]);
	fprintf (stderr, "\n");
#endif /* NDEBUG */

	if (spawnvp (P_WAIT, szSpawnPath, rgpchArg) == -1)
		Error ("cannot spawn size.exe");
}


char *PchGetLineAndNum (fp, rgch, pcb)
FILE *fp;
char *rgch;
int *pcb;
{
	char *pch1, *pch2;

	if (fgets (rgch, cchBufferMax, fp) == NULL)
		return NULL;

	pch1 = rgch;
	/* advance to whitespace */
	while (*pch1 && *pch1 != ' ' && *pch1 != '\t' && *pch1 != '\n')
		pch1++;
	/* advance to what follows whitespace */
	while (*pch1 == ' ' || *pch1 == '\t')
		*pch1++;
	/* advance over numbers */
	pch2 = pch1;
	while (isdigit (*pch2))
		pch2++;
	*pch2 = 0;
	*pcb = (pch1 != pch2) ? atoi (pch1) : 0;

	return rgch;
}



/* Open the tempfile and process each line.  Close the file when done. */
ProcessSizeOutput()
{
	int cbLine;
	long cbTotal = 0;
	char *pch;
	FILE *fpTemp;
	char rgchBuffer [cchBufferMax];

	if ((fpTemp = fopen (szTempFile, "r")) == NULL ||
			fseek (fpTemp, 0l, SEEK_SET) != 0)
		Error ("cannot open temporary file: coresize.tmp");

	while ((pch = PchGetLineAndNum (fpTemp, rgchBuffer, &cbLine)) != NULL)
		{
		printf ("\t%s\n", pch);
		cbTotal += cbLine;
		}

	printf ("\n\tTotal:\t\t%ld.\n\n", cbTotal);

	fclose (fpTemp);
}


/* report an error.  never returns. */
Error (pch)
char *pch;
{
	fprintf (stderr, "%s: error: %s.\n", pchApp, pch);
	exit (1);
}


/* warn the user, continues */
Warning (pch)
char *pch;
{
	fprintf (stderr, "%s: warning: %s.\n", pchApp, pch);
}


/* count of characters in string, includes trailing 0 */
int CchSz (pch)
char *pch;
{
	int cch = 1;
	while (*pch++ != 0)
		cch++;
	return cch;
}


bltb (pch1, pch2, cch)
char *pch1, *pch2;
int cch;
{
	while (cch--)
		*pch2++ = *pch1++;
}


AppendSzToSz (pch1, pch2)
char *pch1, *pch2;
{
	while (*pch2)
		pch2++;
	while (*pch1)
		*pch2++ = *pch1++;
	*pch2 = 0;
}


