/* M K  B C M  M A P . C */
/*  Creates winword.bcm from opuscmd.asm for use by dumprsh.exe.
*/

/* #define DEBUG */

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "bcmmap.h"

#define fTrue 1
#define fFalse 0

FILE *fpIn = NULL;
FILE *fpOut = NULL;

#define ichMaxLine 120
char rgch[ichMaxLine];
long iLineIn = 0;


main (argc, argv)
int argc;
char *argv[];
{
	if (argc < 3)
		Error("usage: mkbcmmap opuscmd.asm winword.bcm");

	if ((fpIn = fopen(argv[1], "r")) == NULL)
		Error("Cannot open input file");

	if ((fpOut = fopen(argv[2], "w+b")) == NULL)
		Error("Cannot create output file"); 

	while(!feof(fpIn))
		ProcessBcm();

	fcloseall();
	exit(0);
}

Error(pch)
char *pch;
{
	fprintf (stderr, "error: %s.\n", pch);
	if (rgch[0] != 0 || iLineIn > 0)
		fprintf(stderr, "At line # %ld: %s\n", iLineIn, rgch);
	fcloseall();
	exit(1);
}

#define szBsyMatch "; bsy = "
#define ichBsy sizeof(szBsyMatch)-1


ProcessBcm()
{
	struct BME bme;
	int i;
	char *pch, *pch2, *pch2Mac;

	/* find a line of the form "; bsy = " */
	for (;;)
		{
		if (!fgets(rgch, ichMaxLine, fpIn))
			{
			if (feof(fpIn))
				return;
			else
				Error("input file error - 1");
			}
		iLineIn++;
		if (!strncmp(rgch, szBsyMatch, ichBsy))
			break;
		}

#ifdef DEBUG
	fprintf(stdout, "processing: %s", rgch);
#endif /* DEBUG */

	bme.bcm = (unsigned)atol(&rgch[ichBsy]);

	/* now find the name (always 6 lines later) */
	for (i = 0; i < 6; i++)
		{
		if (!fgets(rgch, ichMaxLine, fpIn))
			Error("input file error - 2");
		iLineIn++;
		}

	if (strncmp(rgch, "\tdb\t", 4))
		Error("improperly formed input");

	pch2 = bme.szName;
	pch2Mac = pch2 + ichBcmNameMax - 1;

	for (pch = &rgch[4]; *pch != 0 && *pch != '\''; pch++)
		;

	if (*pch == '\'')
		{
		while (*++pch != '\'' && pch2 < pch2Mac)
			*pch2++ = *pch;
		*pch2 = 0;
		}

	else
		/* linked list! Assume we already read in the name */
		{
		unsigned bcm = bme.bcm;

		/* next item is who it is linked too */
		if (!fgets(rgch, ichMaxLine, fpIn))
			Error("input file error - 3");
		iLineIn++;

#ifdef DEBUG
		fprintf(stdout, "\t** searching back for %s", &rgch[4]);
#endif /* DEBUG */

		if (!FFindBcm((unsigned)atol(&rgch[4]), &bme))
			Error("forward reference in linked list or malformed");

		bme.bcm = bcm;
		}

#ifdef DEBUG
	fprintf(stdout, "\tstring = %s\n", bme.szName);
#endif /* DEBUG */

	if (fwrite(&bme, cbBME, 1, fpOut) != 1)
		Error("output file write error");
}

FFindBcm(bcm, pbme)
unsigned bcm;
struct BME *pbme;
{
	if (fseek(fpOut, 0l, SEEK_SET) != 0)
		Error("cannot seek output file to beginning");

	while (pbme->bcm != bcm && !feof(fpOut))
		if (fread(pbme, cbBME, 1, fpOut) != 1)
			Error("read error on output file");

	if (fseek(fpOut, 0l, SEEK_END) != 0)
		Error("cannot seek output file to end");

	return pbme->bcm == bcm;
}
	

