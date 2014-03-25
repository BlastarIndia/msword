/*-----------------------------------------------------------------------
|																		|
|   slice.c -- breaks up a file into "phrases" and "words"				|
|																		|
|	To make slice.exe:  cc -Oat -Gs -AS slice.c							|
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|	SLICE breaks up stdin into phrases and words terminated by			|
|		newlines.														|
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|  Modification History													|
|	11/05/87	johng -- from pslice.c									|
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define cchKeyMax 256

#define	FIsPunct(ch)  (	(ch) == ' ' || (ch) == ':' || (ch) == '\\' ||\
						(ch) == '.' || (ch) == ',')

char rgch[256];


main()
{
	setmode(fileno(stdin), O_BINARY);

	for (;;)
		{
		GetSz();
		PutPhrase();		/* phrases						*/
		PutWordsPunct();	/* words including punctuation	*/
		PutWords();			/* words without punctuation	*/
		}
}


PutPhrase()
{
	char *pch, ch;
	int cch;

	pch = rgch;
	cch = 0;
	while (ch = *pch++)
		{
		if (cch == (cchKeyMax - 1) || (ch == '.' && *pch == ' '))
			{
			putchar('\n');
			cch = 0;
			while (*pch == ' ')
				pch++;
			}
		else
			{
			putchar(ch);
			cch++;
			}
		}
	putchar('\n');
}


PutWordsPunct()
{
	char *pch, ch;
	int cch;

	pch = rgch;
	cch = 0;
	while (ch = *pch++)
		{
		if (cch == cchKeyMax - 1)
			{
			putchar('\n');
			cch = 0;
			while (*pch == ' ')
				pch++;
			}
		else  if (FIsPunct(ch))
			{
			putchar(ch);
			putchar('\n');
			cch = 0;
			while (*pch == ' ')
				pch++;
			}
		else
			{
			putchar(ch);
			cch++;
			}
		}
	putchar('\n');
}


PutWords()
{
	char *pch, ch;
	int cch;

	pch = rgch;
	cch = 0;
	while (ch = *pch++)
		{
		if (cch == (cchKeyMax - 1) || FIsPunct(ch))
			{
			putchar('\n');
			cch = 0;
			while (*pch == ' ')
				pch++;
			}
		else
			{
			putchar(ch);
			cch++;
			}
		}
	putchar('\n');
}


GetSz()
{
	char ch, *pchS, *pchD;
	if (gets(rgch) == NULL)
		exit(0);

	/* strip out all carriage returns and line feeds */
	pchS = pchD = rgch;
	while ((ch = *pchD++ = *pchS++) != 0)
		if (ch == 0x0a || ch == 0x0d)
			pchD--;
}


