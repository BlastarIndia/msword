/* ***************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ***************************************************************************
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

#define strlenTblMax 20  /* matches value in rtfgen.c */

#define chReturn        13
#define chEol           10
#define chBackslash     92
#define chRTFOpenBrk     '{'
#define chRTFCloseBrk    '}'

char szLn[100];   /* also overkill in size */


main()
{
	int ich;
	int ch;
	char *pch;

	ich = 0;
	/* break lines being sure that no rtf keyword + terminating space
		is split apart
	*/

	while ((ch = getchar()) != EOF)
		{
		/*  let longest known keyword + slack fit */
		/* 72 is edge, 5 for num arg, 1 for term space */
		if (ich > 72 - strlenTblMax - 5 - 1)
			{      /* break on unit if possible */
			if (ch == chRTFOpenBrk || ch == chBackslash
					|| ich > 72)
				{
				szLn[ich] = 0;
				printf ("%s\n", szLn);
				ich = 0;
				}
			}
		szLn[ich++] = ch;
		}

	szLn[ich] = 0;
	printf ("%s\n", szLn);    /* last line */

}


