/*
	stringpp -- string pre-processor

	takes a string file as input in the following format:

		suffix, string

	where suffix is appended to "ids" to get the identifier and
	the string is everything in the line after the comma.  everything from
	the comma past will be inserted into the strings.c file.

	comment lines begin with ';'

	WARNING: the line "suffix, foo" will create a string with a starting 
	space!

	usage:	stringpp infile keyfile outfile

	where infile is the input file, keyfile contains a list of key phrases
	for compression, and outfile is the output file with no extensions.
	outfile.c, outfile.h, and outfile.inc will be created.

	10/28/87	johng
		Significant changes have been made.  Stdin is no longer a valid input
		stream since 2 passes are required on the input file.  The countfile
		option has been added so that keywdopt can get the number of bytes
		saved from stringpp.  Tokens are now replaced with either a single
		byte (0x01 to 0x1f) or two byte (0x00 + 0x20 to 0xff) token.  The
		tokens are sorted by frequency of use so that the most common
		tokens are replaced by single byte tokens.  The tokens can now
		contain tokens themselves.
*/
#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define ichMax 128
#define cchTokenMax	50
#define cTokenMax	128

FILE *pfIn;		/* input file */
FILE *pfKey;	/* key words file */
FILE *pfC;		/* new .c file */
FILE *pfH;		/* header file */
FILE *pfDC;		/* disk swap .c file */
FILE *pfI;		/* include file */

int cchUnpacked = 0, cchPacked = 0;	/* total characters if packed/not packed */
int fTooManyTokens = FALSE;
int ids;		/* current id value */

int cchStringMax = 0;
char rgszToken[cTokenMax][cchTokenMax];
unsigned char rgExtChar[32];	/* remapped extended characters */
int	rgcExtChar[128];			/* count of extended character instances */
int cExtCharMac;
int cTokenMac = 0;
int fCompress = TRUE;

char szStr[ichMax];		/* current string */

main(argc, argv)
int argc;
char **argv;
{
	int w, i;
	char sz[ichMax];

	w = 1;
	if (argc != 4)
		{
		printf("stringpp:  usage:  stringpp <infile> <keyfile> <outfile>\n");
		goto error;
		}
	if ((pfIn = fopen(argv[1], "r")) == NULL)
		{
		printf("stringpp:  can't open input file: %s\n", argv[1]);
		goto error;
		}
	if ((pfKey = fopen(argv[2], "r")) == NULL)
		{
		printf("stringpp:  can't open key words file: %s\n", argv[2]);
		goto closein;
		}
	SzAppend(argv[3], ".c", sz);
	pfC = fopen(sz, "w");
	if (pfC == NULL)
		{
		printf("stringpp:  can't create file: %s\n", sz);
		goto closekey;
		}
	SzAppend(argv[3], ".h", sz);
	pfH = fopen(sz, "w");
	if (pfH == NULL)
		{
		printf("stringpp:  can't create file: %s\n", sz);
		goto closerc;
		}
	SzAppend(argv[3], ".inc", sz);
	pfI = fopen(sz, "w");
	if (pfI == NULL)
		{
		printf("stringpp:  can't create file: %s\n", sz);
		goto closeh;
		}
	SzAppend(argv[3], "d.c", sz);
	pfDC = fopen(sz, "w");
	if (pfDC == NULL)
		{
		printf("stringpp:  can't create file: %s\n", sz);
		goto closei;
		}

	/* read tokens into rgszToken */
	GetTokens();

	/* scan input file and set up count of each token used (rgcToken) */
	for (i = 0; i < 128; i++)
		{
		rgcExtChar[i] = 0;
		}
	DoKeyTokens(NULL);				/* scan for ext chars within tokens	*/
	DoStrings("", "ids", NULL);		/* scan for ext chars within strings	*/
	DoStrings("Prompt", "idp", NULL);	/* scan for ext chars within prompts	*/
	fseek(pfIn, 0L, SEEK_SET);		/* reset input file for second pass		*/

	/* remap and output extended characters */
	DoExtChars(pfC);

	DoKeyTokens(pfC);

	w = 0;

	DoStrings("", "ids", pfC);

	DoStrings("Prompt", "idp", pfC);

	fCompress = FALSE;

	DoStrings("Disk", "idd", pfDC);

	cchStringMax++;	/* Take account of zero terminator. */

	/* header max string size */
	sprintf(sz, "#define cchStringMax %d\n", cchStringMax);
	PutSzPf(sz, pfH);

	/* include max string size */
	sprintf(sz, "cchStringMax EQU %d\n", cchStringMax);
	PutSzPf(sz, pfI);

	/* finish the .c file.  have to add the CchGetString() proc */
	fprintf(pfC, "#define cTokenMax %d\n\n", cTokenMax);

	fclose(pfDC);
closei:
	fclose(pfI);
closeh:
	fclose(pfH);
closerc:
	fclose(pfC);
closekey:
	fclose(pfKey);
closein:
	fclose(pfIn);

	printf("stringpp:  Saved %d bytes.\n", cchUnpacked - cchPacked);
error:
	exit(w);
}


SzAppend(sz1, sz2, szDes)
/* append sz2 to sz1 and put the result in szDes.
*/
char *sz1, *sz2, *szDes;
{
	char *pch;

	pch = sz1;
	while (*pch != '\0')
		*szDes++ = *pch++;
	pch = sz2;
	while (*pch != '\0')
		*szDes++ = *pch++;
	*szDes++ = '\0';
}


GetTokens()
{
	int ich;
	char szIn[ichMax];
	char szT[ichMax];

	while ((fgets(szIn, ichMax, pfKey) != NULL))
		{
		/* strip newline from line */
		/* if using stdin then have to strip CRLF */
		ich = CchFromSz(szIn) - 1;
		while (szIn[ich] == '\n' || szIn[ich] == '\r')
			szIn[ich--] = '\0';

		if (szIn[0] == '\0' || szIn[0] == ';')
			/* ignore blank lines */
			continue;

		AddToken(szIn);
		}
}


AddToken(sz)
char *sz;
{
	char *pchToken;
	char szT[ichMax];

	if (cTokenMac >= cTokenMax)
		{
		if (!fTooManyTokens)
			{
			fTooManyTokens = TRUE;
			printf("stringpp:  WARNING:  ignoring extra key words.\n");
			}
		return;
		}

	pchToken = rgszToken[cTokenMac++];
	while (*pchToken++ = *sz++)
		;
}


DoKeyTokens(pf)
FILE *pf;
{
	int iToken;

	PutSzPf("CODE char rgszToken[][] = {\n", pf);
	for (iToken = 0; iToken < cTokenMac; iToken++)
		{
		PutToken(iToken, pf);
		}
	PutSzPf("\t};\n\n", pf);
}


PutToken(iTokenFirst, pf)
int iTokenFirst;
FILE *pf;
{
	int iToken;
	char *pchStart;
	char sz[ichMax];
	char szT[ichMax];
	char *pch, *pchT;

	pch = rgszToken[iTokenFirst];
	pchT = sz;
	while (*pchT++ = *pch++)
		;

	pchStart = pch = sz;
	PutSzPf("\tSt(\"", pf);

	while ((iToken = ITokenMatch(&pch, iTokenFirst)) != -1)
		{
		if (pf == NULL)
			ScanString(pchStart);
		PutSzPf(pchStart, pf);
		sprintf(szT, "\\%03o", 256-cTokenMax+iToken);
		PutSzPf(szT, pf);

		if (pf == pfC)
			cchPacked += CchFromSz(pchStart) + 1;

		pchStart = pch;
		}
	sprintf(szT, "%s\"),\n", pch);
	if (pf == NULL)
		ScanString(szT);
	PutSzPf(szT, pf);

	/* add string length + 1 for count byte + 2 for pointer to string */
	if (pf == pfC)
		cchPacked += CchFromSz(pch) + 3;
}


DoExtChars(pf)
FILE *pf;
{
	int	i, j;
	int	mpT[128];
	char szT[ichMax];

	for (i = 0; i < 128; i++)
		{
		mpT[i] = i + 128;
		}

	/* sort to find most used extended characters */
	Sort(rgcExtChar, mpT, FALSE);

	rgExtChar[0] = 0;
	for (i = 1, j = 0; i < 32; i++)
		{
		if (rgcExtChar[j] == 0)
			break;
		if (i == 0x0a || i == 0x0d)
			rgExtChar[i] = i;
		else
			rgExtChar[i] = mpT[j++];
		}
	cExtCharMac = i;

	/* if any extended characters, create rgExtChar[] */
	if (cExtCharMac > 1)
		{
		sprintf(szT, "#define cExtCharMac %d\n\n", cExtCharMac);
		PutSzPf(szT, pf);

		PutSzPf("CODE char rgExtChar[] = {\n", pf);

		for (i = 0; i < cExtCharMac; i++)
			{
			sprintf(szT, "\t%3u,\n", rgExtChar[i]);
			PutSzPf(szT, pf);
			cchPacked++;
			}

		PutSzPf("\t};\n\n", pf);
		}
}


Sort(rgKey, rgw, fAscending)		/* do a simple Shell sort */
int	rgKey[], rgw[], fAscending;
{
	int gap, i, j, temp;

	for (gap = 128/2; gap > 0; gap /= 2)
		for (i = gap; i < 128; i++)
			for (j = i - gap;
					j >= 0 && (fAscending ?
					(rgKey[j] > rgKey[j+gap]) : (rgKey[j] < rgKey[j+gap]));
					j -= gap)
				{
				temp = rgKey[j];
				rgKey[j] = rgKey[j+gap];
				rgKey[j+gap] = temp;
				temp = rgw[j];
				rgw[j] = rgw[j+gap];
				rgw[j+gap] = temp;
				}
}


DoStrings(szArray, szId, pf)
/* returns non-zero if error.
*/
char *szArray, *szId;
FILE *pf;
{
	int ich;
	char szIn[ichMax];
	char szT[ichMax];

	ids = 0;

	sprintf(szT, "CODE char rgsz%s[][] = {\n", szArray);
	PutSzPf(szT, pf);

	while ((fgets(szIn, ichMax, pfIn) != NULL) && (szIn[0] != '*'))
		{
		/* strip newline from line */
		/* if using stdin then have to strip CRLF */
		ich = CchFromSz(szIn) - 1;
		while (szIn[ich] == '\n' || szIn[ich] == '\r')
			szIn[ich--] = '\0';
		if (szIn[0] == '\0' || szIn[0] == ';')
			/* ignore blank lines */
			continue;
		AddString(szIn, szId, pf);
		}

	PutSzPf("\t};\n\n", pf);
}


AddString(sz, szIds, pf)
/* add the string to the resource file and the identifier to the
	header file.
*/
char *sz;
char *szIds;
FILE *pf;		/* if NULL, do not write, just update rgcToken[] */
{
	int cch;
	char *pch;
	char *pchStart;
	int iToken;
	int cchLast, cchAppend;
	char szId[ichMax];
	char szT[ichMax];

	/* if prompt string, leave room for the period. */
	if (szIds[2] == 'p')
		{
		cchLast = 79;
		cchAppend = 1;		/* a period will be appended at run-time */
		}
	else
		{
		cchLast = 80;
		cchAppend = 0;
		}

	if (!fCompress)
		{
		if (sz[0] == '%')
			sz++;
		else
			{
			printf("stringpp:  Invalid disk swap string\n");
			exit(1);
			}
		}
	GetSzStr(sz);

	/* NOTE: szStr includes a leading and trailing quote. */
	pch = szStr+1;
	cch = CchFromSz(pch) - 1;
	if (cch > cchLast)
		{
		printf("stringpp:  WARNING:  truncating string to 80 characters.\n");
		cch = cchLast;
		}
	pch[cch] = '\0';
	if (cch + cchAppend > cchStringMax)
		cchStringMax = cch + cchAppend;

	if (pf == pfDC)
		PutSzPf("\tSz(\"", pf);
	else
		PutSzPf("\tSt(\"", pf);

	if (pf == pfC)
		cchUnpacked += CchFromSz(pch) + 3;

	pchStart = pch;
	if (fCompress)
		{
		while ((iToken = ITokenMatch(&pch, -1)) != -1)
			{
			if (pf == NULL)
				ScanString(pchStart);
			PutSzPf(pchStart, pf);
			sprintf(szT, "\\%03o", 256-cTokenMax+iToken);
			PutSzPf(szT, pf);

			if (pf == pfC)
				cchPacked += CchFromSz(pchStart) + 1;

			pchStart = pch;
			}
		}
	sprintf(szT, "%s", pch);
	ScanString(szT);
	PutSzPf(szT, pf);

	if (pf == pfC)
		cchPacked += CchFromSz(pch) + 3;

	PutSzPf("\"),\n", pf);

	if (pf != NULL)
		{
		/* header entry */
		sprintf(szT, "#define %s%s %d\n", szIds, sz, ids);
		PutSzPf(szT, pfH);

		/* include file entry */
		sprintf(szT, "%s%s EQU %d\n", szIds, sz, ids++);
		PutSzPf(szT, pfI);
		}
}


ITokenMatch(ppch, iInvalidToken)
char **ppch;
int iInvalidToken;
{
	char *pch;
	char *pchStart;
	char *pchToken;
	int ch;
	int iToken;

	for (pchStart = *ppch; *pchStart != '\0'; pchStart++)
		{
		for (iToken = 0; iToken < cTokenMac; iToken++)
			{
			if (iToken == iInvalidToken)
				continue;
			pch = pchStart;
			pchToken = rgszToken[iToken];
			while ((ch = *pchToken) && ch == *pch++)
				pchToken++;

			if (!ch)
				{
				*ppch = pch;
				*pchStart = '\0';
				return (iToken);
				}
			}
		}

	return (-1);
}


ScanString(pch)
char *pch;
{
	char ch;
	while (ch = *pch++)
		if (ch & 0x80)
			rgcExtChar[ch & 0x7f]++;
}


CchFromSz(sz)
char *sz;
{
	int cch = 0;

	while (*sz++ != '\0')
		cch++;
	return(cch);
}


GetSzStr(sz)
/* removes everything after the comma into szStr.  skips any white
	space after the comma.
*/
char *sz;
{
	char ch, *pch;

	szStr[0] = '\0';
	while (*sz != '\0')
		{
		if (*sz == ',')
			{
			*sz = '\0';
			/* skip comma and white space */
			do
				sz++;
			while (*sz == ' ' || *sz == '\t');

			pch = szStr;
			while (*sz != '\0')
				if ((ch = *pch++ = *sz++) == '\\')
					{
					/* may have found a character in the form of "\xxx" */
					unsigned int w;
					if (sscanf(sz, "%3o", &w) == 1 && w > 127 && w < 256)
						{
						*(pch-1) = w;
						sz += 3;
						}
					}
			*pch = '\0';
			break;
			}
		sz++;
		}
}


PutSzPf(sz, pf)
char *sz;
FILE *pf;
{
	char ch, *pch;
	int i;
	char szT[ichMax];

	pch = szT;

	if (pf != NULL)
		{
		while (ch = *pch++ = *sz++)
			{
			if (ch & 0x80)
				{
				/* found an extended character */
				pch--;
				for (i = 0; i < cExtCharMac; i++)
					if (ch == rgExtChar[i])
						break;

				if (i == cExtCharMac)
					{
					i = ch;
					sprintf(pch, "\\000");
					pch += 4;
					cchPacked++;
					}

				sprintf(pch, "\\%03o", i);
				pch += 4;
				}
			}

		fputs(szT, pf);
		}
}


