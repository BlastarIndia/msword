/* eldes.c */

/* This program is intended to look through ".des" files to show the
	names of the HELP_ID, followed by all the EL_NAME items.   If the item
	is preceded by a PUSH_BUTTON, the name is preceded by a "+", and
	followed by the name following the PUSH_BUTTON call.   This is to
	check that the PUSH_BUTTON call *precedes* the EL_NAME, since if it
	does not, or the pushbutton does not actually apply to the EL_NAME,
	the "+" will be shown before the name erroneously.   
	If the EL_NAME is preceded by a PARSE_PROC call, the EL_NAME is shown
	with a "*" before it.   Again, if the PARSE_PROC call comes after the
	EL_NAME call, the output can be messed up, applying the "*" to the
	wrong call.

	Tony Krueger 6/23/88 */

#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <string.h>

extern FILE * fopen();

typedef int WORD;

typedef int F;
#define fFalse 0
#define fTrue 1


F FWild();
F FEnumWild();


#define szIntl  "Intl"

F fQuiet = fFalse;
int lno = 1;


FEWild(szFile, wParam)
char * szFile;
WORD wParam;
{
	FILE * pfl;

	if ((pfl = fopen(szFile, "r")) == NULL)
		{
		fprintf(stderr, "Can't open %s!\n", szFile);
		return fFalse;
		}

	printf("\nProcessing file \"%s\"...\n", szFile);

	if (!FProcessFile(pfl))
		return fFalse;

	fclose(pfl);

	return fTrue;
}


main(argc, argv)
int argc;
char * argv [];
{
	FILE * pfl;
	int argi;

	if (argc == 1)
		UsageExit(argv[0]);

	printf("ELDES: * = Parse_Proc, + = Push_Button.\n");

	for (argi = 1; argi < argc; ++argi)
		{
		if (FWild(argv[argi]))
			{
			FEnumWild(argv[argi], FEWild, 0);
			}
		else
			{
			FEWild(argv[argi], 0);
			}

		fclose(pfl);
		}

	if (!fQuiet)
		putchar('\n');
}


char * PchGetNextword (pchStart)
char * pchStart;
{
	char * pchNextStart;
	char * pch;

	/* get location of first space after pchStart */
	if ((pchNextStart = strchr(pchStart, ' ')) == NULL)
		{
		printf("ERROR: keyword found, but no space afterward!\n");
		return NULL;
		}
	pchNextStart++; /* increase pointer to one character past space */
	if ((pch = strpbrk(pchNextStart, " \n\t\r\f")) == NULL)
		{
		printf("ERROR: next word past keyword not found!\n");
		return NULL;
		}
	/* put a \0 to terminate string at the end of the "next word" */
	*pch = '\0';
	return pchNextStart;
}


char * PchRemoveBackquotes(pchStart)
char * pchStart;
{
	char * pch;

	if (*pchStart == '`')
		{
		/* increase pchStart to point one character past backquote */
		pchStart++;
		pch = pchStart;
		/* search until next backquote found or end of string */
		while (*pch != '`' && *pch != '\0')
			{
			++pch;
			}
		/* terminate string at that point */
		*pch = '\0';
		}
	return pchStart;
}


FProcessFile(pfl)
FILE * pfl;
{
	char ch;
	char * pchLine;
	char * pchKeyword;
	char * pchNextword;
	char szLine [256];
	char fParseProc = fFalse;
	char fPushButton = fFalse;
	char szButtonName [256];
	char * pchButtonName;

	while (!feof(pfl))
		{
		pchLine = fgets(szLine, 256, pfl);
		pchButtonName = szButtonName;

		if (pchLine != NULL)
			{
			/* search Line for "EL_NAME" */
			if ((pchKeyword = strstr(pchLine, "EL_NAME")) != NULL)
				{
				if ((pchNextword = PchGetNextword(pchKeyword)) == NULL)
					{
					printf("ERROR: EL_NAME read, but PchGetNextword returned NULL\n");
					}
				else
					{
					pchNextword = PchRemoveBackquotes(pchNextword);
					if (fParseProc == fTrue)
						{
						printf("\t*");
						fParseProc = fFalse;
						}
					else
						{
						printf("\t ");
						}
					if (fPushButton == fTrue)
						{
						printf("+");
						}
					else
						{
						printf(" ");
						}
					printf("%-20s", pchNextword);
					if (fPushButton == fTrue && pchButtonName != NULL)
						{
						printf("(button name is %s)\n", pchButtonName);
						fPushButton = fFalse;
						}
					else
						{
						printf("\n");
						}
					}
				}
			else  if ((pchKeyword = strstr(pchLine, "PARSE_PROC")) != NULL)
				{
				if (fParseProc == fTrue)
					{
					printf("ERROR: fParseProc already true when setting to true.\n");
					}
				fParseProc = fTrue;
				}
			else  if ((pchKeyword = strstr(pchLine, "PUSH_BUTTON")) != NULL)
				{
				if (fPushButton == fTrue)
					{
					printf("ERROR: fPushButton already true when setting to true.\n");
					}
				fPushButton = fTrue;
				if ((pchButtonName = PchGetNextword(pchKeyword)) == NULL)
					{
					printf("ERROR: PUSH_BUTTON read, but PchGetNextword returned NULL\n");
					}
				else
					{
					pchButtonName = strcpy(szButtonName, pchButtonName);
					}
				}
			else  if ((pchKeyword = strstr(pchLine, "HELP_ID")) != NULL)
				{
				if ((pchNextword = PchGetNextword(pchKeyword)) == NULL)
					{
					printf("ERROR: HELP_ID read, but PchGetNextword returned NULL\n");
					}
				else
					{
					pchNextword = PchRemoveBackquotes(pchNextword);
					printf("%s\n", pchNextword);
					}
				}
			}
		} /* end while */
	if (fParseProc == fTrue)
		{
		printf("ERROR: fParseProc still true at EOF.\n");
		}
	if (fPushButton == fTrue)
		{
		printf("ERROR: fPushButton still true at EOF.\n");
		}
	return fTrue;
} /* end fn */


UsageExit(szApp)
char * szApp;
{
	printf("Usage: %s <files> [>output]\n", szApp);
	exit(1);
}



/* F  E N U M  W I L D */
/* Enumerate a wild card calling the given function with an open file for
each match.  Returns fTrue unless the given function returns fFalse or
there are no matches. */
F FEnumWild(szWild, pfn, wParam)
char * szWild;
F (* pfn)();
WORD wParam;
{
#ifdef QC
	printf("Sorry, QC won't let me expand wild cards!\n");
	exit(1);
#else
	struct find_t ftb;

	if (_dos_findfirst(szWild, _A_NORMAL | _A_RDONLY, &ftb) != 0)
		return fFalse;

	if (!(*pfn)(ftb.name, wParam))
		return fFalse;

	while (_dos_findnext(&ftb) == 0)
		{
		if (!(*pfn)(ftb.name, wParam))
			return fFalse;
		}

	return fTrue;
#endif
}


F FWild(sz)
char * sz;
{
	register char ch;
	register char * pch;

	pch = sz;
	while ((ch = *pch++) != '\0')
		{
		if (ch == '*' || ch == '?')
			return fTrue;
		}

	return fFalse;
}


