/* ***************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ***************************************************************************
*
*  Module: rtfgen.c rtf table generator/checker
*
*    Input: rtftbl.h  (with modifications)
*    Output: corrected rtftbl.h on stdout
*
*    generates table of isz indexes from a table of keywords, and  generates
*    irrb indexes. Checks that the keywords are in sort, and that the
*    sym table is in keyword order. Generates a file that contains everything
*    in rtftbl.h with corrected indexes.
*
*    This order is assumed in the input file: misc tables and defines
*    needed below but unmolested, keyword table rfszRtfSym,
*    isz defines, irrb defines, sym table rgrsymWord, other stuff,
*    rgrrbword, mpirrbiszWord.
*
*    to review out a keyword and have this work, ifdef out the line, then:
*     in rgszRtfSym, start the line with a non " char. use slash-asterisk 
*        comments 
*     in rgrsymWord, break up the iszRTF comment, which rtfgen keys on.
*        e.g. /* iszRTFLF becomes /* isz---RTFLF
*    this way we won't recognize the lines and try to process them
*    (not the world's most sophisticated parser here) 
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
** 9/1/87       bz              cloned fron cabgen.c
**
** ************************************************************************ */

#include <stdio.h>
#include <ctype.h>

#define fTrue 1
#define fFalse 0
#define OPUS

/* #define DEBUG  */
/* #define DEBUGZ  */

#define strlenTblMax 20
#define strinMax 400
#define strkwdMax 400
#define strirrbMax 200

char szNum[20];
FILE *stream;    /* file handle for rtftbl.h */

/* BIG tables! */
char rgszName[strkwdMax][strlenTblMax];  /* holds all the keyword strings */
char rgszIrrb[strirrbMax][strlenTblMax];  /* holds all the irrb strings */

char szStartName[] = "csconst char rgszRtfSym";
int cbStartName = sizeof (szStartName) - 1;

char szStartSym[] = "csconst struct RSYM rgrsymWord";
int cbStartSym = sizeof (szStartSym) - 1;

char szStartRrbWord[] = "csconst struct RRB rgrrbWord";
int cbStartRrbWord = sizeof (szStartRrbWord) - 1;

char szStartIrrbIsz[] = "csconst int mpirrbiszWord";
int cbStartIrrbIsz = sizeof (szStartIrrbIsz) - 1;

char szEnd[] = "};";
int cbEnd = sizeof (szEnd) - 1;

char szDefineImp[] = "#define imp";
int cbDefineImp = sizeof (szDefineImp) - 1;

char szDefineIsz[] = "#define iszRTF";
int cbDefineIsz = sizeof (szDefineIsz) - 1;

char szDefineIrrb[] = "#define irrb";
int cbDefineIrrb = sizeof (szDefineIrrb) - 1;

char szIrrbRIgnore[] = "irrbRIgnore";
int cbIrrbRIgnore = sizeof (szIrrbRIgnore) - 1;

char szIsz[] = "iszRTF";
int cbIsz = sizeof (szIsz) - 1;

char szIrrb[] = "irrb";
int cbIrrb = sizeof (szIrrb) - 1;

char szDefineIszMax[] = "#define iszRTFMax";

int iItem = 0;
int fImpFirst = fTrue;
int fIrrbFirst = fTrue;
int fNoIncrNum = fFalse;
int iSpecLast = -1;

char *rgSpec[] =
	{
	"\\012",
	"\\015",
	"'",
	"*",
	"-",
	":",
	"\\134",
	"_",
	"{",
	"|",    
	"}",
	"~"
	};


int crgSpec = sizeof(rgSpec) / sizeof (char *);

char *rgSpecConv[] = 
	{
	"LF",
	"CR",
	"Quote",
	"Asterisk",
	"Hyphen",
	"Colon",
	"Backslash",
	"Underscore",
	"LBracket", 
	"Formula", 
	"RBracket", 
	"Tilde"
	};


int crgSpecConv = sizeof(rgSpecConv) / sizeof (char *);

char rgLine[strinMax];


main()
{
	int iKwdMax;
	int i;

	fprintf (stderr,"RTF keyword index generator\n\n");

	if ((stream = fopen("\\cashmere\\rtftbl.h", "r")) == NULL)
		{
		fprintf (stderr,"Could not open \\cashmere\\rtftbl.h\n\n");
		return;
		}

	/* test sort order of internal specconv table */
	for (i = 1; i < crgSpec; i++)
		if (*(rgSpec[i]) <= *(rgSpec[i - 1]) && *(rgSpec[i - 1]) != '\\')
			{
			fprintf (stderr,"*rgspec[i],%c\n\n",*(rgSpec[i]));
			fprintf (stderr,"*rgspec[i - 1],%c\n\n",*(rgSpec[i - 1]));
			fprintf (stderr,"You fool! Your internal rgSpec is out of order. Fix it!\n\n");
			return;
			}

	if (iKwdMax = WGetNames())
		{
		GenNameIndexes(iKwdMax);
		if (FRgsymWord())
			{
			FIrrbTable(szStartRrbWord, cbStartRrbWord);
			FIrrbTable(szStartIrrbIsz, cbStartIrrbIsz);
			}
		}
	/* flush remainder of file, if any */
	while (fgets(rgLine, strinMax, stream) != NULL)
		{
		printf ("%s", rgLine);
		}

	fclose(stream);
}




int WGetNames()
{
	char szT[256];
	int fInName = fFalse;

	while (fgets(rgLine, strinMax, stream) != NULL)
		{
#ifdef DEBUG
		fprintf (stderr,"token captured: %s", rgLine);
#endif

		if (!fInName && FStartName (rgLine))
			{
			fInName = fTrue;

			fprintf (stderr,"    generating RTF keyword indexes, testing table sortedness...\n\n");


#ifdef DEBUG
			fprintf (stderr,"start symbol detected\n");
#endif

			printf ("%s", rgLine);  /* dump 1st line */
			continue;
			}

		if (!fInName)
			{
			printf ("%s", rgLine); /* dump out leading lines */
			continue;
			}


		if (FEndStruct (rgLine))
			{

			char * pch;

#ifdef DEBUG
			fprintf (stderr,"end symbol detected\n");
#endif

			printf ("%s", rgLine);
			return (iItem);
			}


		if (!FArgName (rgLine))
			return (0);

		printf ("%s", rgLine);
		}
}



int FStartName (szLine)
char *szLine;
{

	register char *pch, *pLine;

	pLine = szLine;
	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

#ifdef DEBUG
	fprintf (stderr,"string to compare to start: %s\n", pLine);
#endif

	if (!strncmp(pLine, szStartName, cbStartName))  /* strings equal up to length */
		{
		iItem = 0;
		return fTrue;
		}
	else
		return fFalse;
}



int FEndStruct (sz)
char *sz;
{
	register char *pLine;

	pLine = sz;
	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

	if (!strncmp(pLine, szEnd, cbEnd))  /* strings equal */
		return fTrue;
	else
		return fFalse;
}



int FArgName (sz)
char *sz;
{

	char szT [256];
	int i;
	int j;
	int fSpec;

	char *pch, *pLine, *pchText;
	int cch;

	/* what this does: we are now inside the rtf rgszRtfSym keyword
		table. We extract the name, converting some special character
		names (CR, LF) to names, and store the results in a BIG table.
		We check to see if the names are in sorted order, handling the
		special names specially, and noting the the special names start
		with CAPS and the normal names don't so the 1st normal name is
		> than the previous special name because of case and the end
		special chars are all past normal names...
	
		Anyway, we see that the names are in order and fil out rgszName.
		We will use the table of names in rgszName later to generate the
		indexes and check that the rgsymWord file is in sync with the
		keyword table.
	*/

	pLine = sz;

	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

	if (*pLine++ != '"')
		return(fTrue);

	if (iItem >= strkwdMax - 1)
		{
		fprintf (stderr,"Error - too many strings (> 400)\n\n");
		return (fFalse);
		}

	/* copy string into rgszName */
	pchText = &rgszName[iItem][0];    /* extract text from inside "" and store */
	pch = pchText;
	cch = 0;
	while ((*pch++ = *pLine++) != '"')
		++cch;
	*--pch = '\0';
	if (cch > strlenTblMax)
		fprintf (stderr,"Error - keyword too long for table and for rtfline\n\n");

#ifdef DEBUG
	fprintf (stderr,"keyword string %s\n", pchText);
#endif
	/* is the string a special character? */

	fSpec = fFalse;
	for (i = 0; i < crgSpec; i++)
		{
		pch = rgSpec[i];
		if (!strcmp(pchText, pch))  /* strings equal */
			{
			fSpec = fTrue;
			/* change to conversion string */
			strcpy(pchText, rgSpecConv[i]);

			/* is this in order? Assume order in spec table is
				correct sort order */

			if (i != iSpecLast + 1)
				{
				fprintf (stderr,"Error - keyword: %s out of sort\n", pchText);
				return (fFalse);
				}
			iSpecLast = i;
			}
		}

	if (!fSpec)
		/* test normal keywords sort order. Note that the
			first normal kw following a special will claim to
			be in order because the special names all start with
			a UC letter which is before any lc letter */
		{
		if (iItem < 1)
			{
			fprintf (stderr,"Error - bogus tables\n\n");
			return (fFalse);
			}

		pch = &rgszName[iItem - 1][0];

#ifdef DEBUG
		fprintf (stderr,"comparing normal strings %s and %s\n", pchText, pch);
#endif
		if (strcmp(pchText, pch) <= 0)
			/* new string not > previous string */
			{
			fprintf (stderr,"Error - keyword: %s out of sort\n", pchText);
			return (fFalse);
			}

		}

	iItem++;
}


int GenNameIndexes(iKwdMax)
int iKwdMax;
{
	char szT[256];
	int i;
	char *pchBase;
	char *pch;

	/* the table endif will be thrown out so put in another here */
	printf ("\n#endif /* RTFSUBS */\n");
	printf ("\n#endif /* DEBUG */\n");
	printf ("\n\n/* id's corresponding to keywords defined above. Must be kept in sync! */\n");
	printf ("/* Generate these #'s with rtfgen, which reads and remakes rtftbl.h */\n\n");

	/* output define string */

	strcpy(szT, szDefineIsz);
	pchBase = szT + cbDefineIsz;


	for (i=0; i < iKwdMax; i++)
		{
		strcpy(pchBase, &rgszName[i][0]);
		strcat(pchBase, "  \t");
		pch = szNum;
		IntToPpch(i, &pch);
		*pch = '\0';
		strcat(pchBase, szNum);
		printf ("%s\n", szT);
		}

	/* output define for iszRTFMax */

	strcpy(szT, szDefineIszMax);
	strcat(szT, "\t");
	pch = szNum;
	IntToPpch(iKwdMax, &pch);
	*pch = '\0';
	strcat(szT, szNum);
	printf ("\n%s\n\n", szT);


}





int FRgsymWord()
{
	int fInSym = fFalse;
	iItem = 0;

	while (fgets(rgLine, strinMax, stream) != NULL)
		{
#ifdef DEBUG
		fprintf (stderr,"token captured: %s", rgLine);
#endif

		if (!fInSym && FStartSym (rgLine))
			{
			fprintf (stderr,"    testing order of sym table...\n\n");
			fInSym = fTrue;

#ifdef DEBUG
			fprintf (stderr,"start symbol detected\n");
#endif

			printf ("%s", rgLine);
			continue;
			}

		if (!fInSym)
			{
			FGetIrrb(rgLine);
			continue;
			}

		if (FEndStruct (rgLine))
			{

#ifdef DEBUG
			fprintf (stderr,"end symbol detected\n");
#endif

			printf ("%s", rgLine);
			return (fTrue);
			}


		if (!FArgSym (rgLine))
			return (fFalse);

		printf ("%s", rgLine);
		}
}



int FStartSym (szLine)
char *szLine;
{

	register char *pch, *pLine;

	pLine = szLine;
	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

#ifdef DEBUG
	fprintf (stderr,"string to compare to start: %s", pLine);
#endif

	if (!strncmp(pLine, szStartSym, cbStartSym))  /* strings equal up to length */
		{
		iItem = 0;
		return fTrue;
		}
	else
		return fFalse;
}




int FArgSym (sz)
char *sz;
{

	char szT [256];
	int i;
	int valRet;

	char *pch, *pLine, *pchText;


	/* what this does: we are now inside the rtf rgsymWord table. 
		We extract the isz comment, and compare it to the keyword
		we stored earlier in rgszName. They should be in the same
		order or we will stop.
	*/

	pLine = sz;

	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

	/* only deal with comment lines - pline will point after * */

	if (!(*pLine++ == '/' && *pLine++ == '*'))
		return(fTrue);

	if (iItem >= (strkwdMax - 1))
		{
		fprintf (stderr,"Error - too many strings (> 400)\n\n");
		return (fFalse);
		}

	while (*pLine && *pLine != 'i')   /* look for isz string */
		pLine++;

	if (strncmp(pLine, szIsz, cbIsz))  /* string != iszRTF ? */
		{
		return fTrue;
		}
	pLine += cbIsz;  /* point to keyword */

	/* copy string into szT */
	/* note: tab/space/* for terminator */

	pch = szT;
	while ((*pch = *pLine++) != ' ' && *pch != '\t' && *pch != '*')
		pch++;
	*pch = '\0';

#ifdef DEBUG
	fprintf (stderr,"keyword string from rgsym %s\n", szT);
#endif

	pch = &rgszName[iItem][0];

#ifdef DEBUG
	fprintf (stderr,"comparing keyword/sym values %s and %s\n", pch, szT);
#endif
	if ((valRet = strcmp(szT, pch)) != 0)
		/* sym string not = name string */
		{
#ifdef DEBUG
		fprintf (stderr,"strcmp fail ret val %d\n", valRet);
		fprintf (stderr,"size of szT: %d \n", strlen(szT));
		fprintf (stderr,"size of table value: %d \n", strlen(pch));
#endif
		fprintf (stderr,"Error - sym table keyword: %s out of sync with keywords\n", szT);

#define CONTINUE   /* keep dumping even if errors */
#ifdef CONTINUE
		iItem++;
		return (fTrue);
#else
		return (fFalse);
#endif
		}

	iItem++;

	return fTrue;
}



int FGetIrrb(pLine)
char *pLine;

{

	char *pch, *pch1, *pch2, *pch3;
	char szT[256];
	char szTokens[256];
	char szNum[10];
	int cbBase;
	char *strtok();

	/* we have finished the names and not yet reached the rgrsymWord table.
		skip over iszRTF defines and comments,  and regenerate numbers
		for the irrb strings.
	*/

/* the numbers for the irrb defines are generated by rtfgen.c. It takes an 
	existing table and starts at 0 and increments on each define except:
	following irrb...First or irrb...Lim or irrb...Max... we don't increment,
	and the value for irrb...First is not incremented.  If the existing
	value is irrbRIgnore we leave it.
*/


	if (strncmp(pLine, szDefineIrrb, cbDefineIrrb))  /* string != #define irrb? */
		{
		if (!fIrrbFirst)
			printf ("%s", pLine);

		return fTrue; /* skip over until we get one */
		}

	if (fIrrbFirst)
		{
		fIrrbFirst = fFalse;
		printf ("\n\n/* Rtf Record Block indexes - match properties to structures.\n\n");


		printf("   the numbers for these defines are generated by rtfgen.c. It takes an \n");
		printf("   existing table and starts at 0 and increments on each define except:\n");
		printf("   following irrb...First or irrb...Lim or irrb...Max... we don't increment,\n");
		printf("   and the value for irrb...First is not incremented.\n");
		printf("*/\n\n");
		}

	/* now we have a #define irrb string -  split into tokens */

	/* copy pLine stuff into temp as strtok will
		mess with it skip stuff we don't care about */
	strcpy (szTokens, (pLine + cbDefineIrrb));

#ifdef DEBUG
	fprintf (stderr,"pLine before strtok: %s", pLine);
#endif

	pch1 = strtok(szTokens, " ");    /* rrb name */
	pch2 = strtok(NULL, " ");   /* value - this is how it gets next token */

#ifdef DEBUGZ
	fprintf (stderr,"strtok tokens: %s and %s\n", pch1, pch2);
	fprintf (stderr,"line tested: %s", pLine);
#endif
	/* if value is irrbRIgnore, dump to file */

#ifdef DEBUG
	fprintf (stderr,"pLine after strtok: %s", pLine);
#endif

	if (!strncmp(pch2, szIrrbRIgnore, cbIrrbRIgnore))
		{
		fNoIncrNum = fFalse;
		printf ("%s", pLine);
		}
	else
		{
		if ( FStrSub (pch1, "Lim") ||
				FStrSub (pch1, "Max") ||
				FStrSub (pch1, "First") )
			{
			fNoIncrNum = fTrue;
			}

		/* put new number on string */
		strcpy (szT, szDefineIrrb);
		strcat(szT, pch1);
		strcat(szT, "  \t");
		pch = szNum;
		IntToPpch(iItem, &pch);
		*pch = '\0';
		strcat(szT, szNum);
		printf ("%s\n", szT);
		}

	if (fNoIncrNum)
		fNoIncrNum = fFalse;
	else
		{
		/* copy string into rgszIrrb for later checking */
		strcpy(&rgszIrrb[iItem][0], pch1);
		iItem++;
		}
}


int FIrrbTable(szStart, cbStart)
char *szStart;
int cbStart;
{
	int finRrb = fFalse;

	iItem = 0;     /* for irrb's */
	while (fgets(rgLine, strinMax, stream) != NULL)
		{
#ifdef DEBUG
		fprintf (stderr,"token captured: %s", rgLine);
#endif

		if (!finRrb && FStartRrb (rgLine, szStart, cbStart))
			{
			fprintf (stderr,"    testing order of %s...\n\n", szStart);
			iItem = 0;     /* reset - was changed doing the defines */
			finRrb = fTrue;

#ifdef DEBUG
			fprintf (stderr,"start symbol detected\n");
#endif

			printf ("%s", rgLine);
			continue;
			}

		if (!finRrb)
			{
			printf ("%s", rgLine);
			continue;
			}

		if (FEndStruct (rgLine))
			{

#ifdef DEBUG
			fprintf (stderr,"end symbol detected\n");
#endif

			printf ("%s", rgLine);
			return (fTrue);
			}


		if (!FArgRrb (rgLine, szStart))
			return (fFalse);

		printf ("%s", rgLine);
		}
}


int FStartRrb (szLine, szStart, cbStart)
char *szLine;
{

	register char *pch, *pLine;

	pLine = szLine;
	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

#ifdef DEBUG
	fprintf (stderr,"string to compare to start: %s", pLine);
#endif

	if (!strncmp(pLine, szStart, cbStart))  /* strings equal up to length */
		{
		iItem = 0;
		return fTrue;
		}
	else
		return fFalse;
}


int FArgRrb (sz, szStart)
char *sz;
{

	char szT [256];
	int i;
	int valRet;

	char *pch, *pLine, *pchText;


	/* what this does: we are now inside the rtf rgrrbWord table. 
		We extract the irrb comment, and compare it to the keyword
		we stored earlier in rgszIrrb. They should be in the same
		order or we will stop.
	*/

	pLine = sz;

	while (*pLine == ' '|| *pLine == '\t')   /* skip leading white */
		pLine++;

	/* only deal with comment lines - pline will point after * */

	if (!(*pLine++ == '/' && *pLine++ == '*'))
		return(fTrue);

	if (iItem >= strkwdMax - 1)
		{
		fprintf (stderr,"Error - too many strings (> 400)\n\n");
		return (fFalse);
		}

	while (*pLine && *pLine != 'i')   /* look for isz string */
		pLine++;

	if (strncmp(pLine, szIrrb, cbIrrb))  /* string != irrb ? */
		{
		return fTrue;
		}
	pLine += cbIrrb;  /* point to keyword */

	/* copy string into szT */
	/* note: tab/space/* for terminator */

	pch = szT;
	while ((*pch = *pLine++) != ' ' && *pch != '\t' && *pch != '*')
		pch++;
	*pch = '\0';

#ifdef DEBUG
	fprintf (stderr,"keyword string from rgsym %s\n", szT);
#endif

	pch = &rgszIrrb[iItem][0];

#ifdef DEBUGZ
	fprintf (stderr,"comparing keyword/sym values %s and %s\n", pch, szT);
#endif
	if ((valRet = strcmp(szT, pch)) != 0)
		/* sym string not = name string */
		{
#ifdef DEBUG
		fprintf (stderr,"strcmp fail ret val %d\n", valRet);
		fprintf (stderr,"size of szT: %d \n", strlen(szT));
		fprintf (stderr,"size of table value: %d \n", strlen(pch));
#endif
		fprintf (stderr,"Error - %s keyword: %s out of sync with irrb defines\n", szStart, szT);

#define RCONTINUE   /* keep dumping even if errors */
#ifdef RCONTINUE
		iItem++;
		return (fTrue);
#else
		return (fFalse);
#endif
		}

	iItem++;

	return fTrue;
}





IntToPpch(n, ppch)
register int n;
char **ppch;
{
	if (n >= 10)
		{
		IntToPpch(n / 10, ppch);
		n %= 10;
		}

	*(*ppch)++ = '0' + n;
}



int FStrSub (sz, szPatt)
char *sz, *szPatt;
{
	char *pchT;

#ifdef DEBUG
	fprintf (stderr,"FStrSub comparing %s and %s\n", sz, szPatt);
#endif

	while (*sz)
		{
		if (*sz != *szPatt)
			sz++;
		else
			{
			pchT = szPatt;  /* skip previous match */
			pchT++;
			sz++;
			while (*pchT)  /* must match whole string */
				{
				if (*pchT == *sz)
					{
					sz++;
					pchT++;
					}
				else
					break;
				}
			if (!*pchT)
				return (fTrue);

			}
		}
	return (fFalse);
}


