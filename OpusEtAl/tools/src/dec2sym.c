#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINE  130

/* 
*	filter to take input from decode and give back just the symbol names.
*
*  danp      1 jun 88
*/

char szPubDef[] = "PUBDEF";
char szExtDef[] = "EXTDEF";
char szPeriod[] = ".";

main()
{
	char  szCur[MAXLINE];

	gets( szCur);
	while ( !feof(stdin) )
		{
		if ( strstr( szCur, szPubDef) != NULL && strstr( szCur, szPeriod) == NULL)
			{
			gets( szCur);
			GetLines();
			}
		else  if ( strstr( szCur, szExtDef) != NULL && strstr( szCur, szPeriod) == NULL)
			GetLines();
		gets( szCur);
		}
}


GetLines()
{
	char  szT[MAXLINE];
	char  *pszT,
	*pszT2;


	gets(szT);
	while  ( *szT != NULL)
		{
		pszT = pszT2 = szT;
		while ( *pszT++ != '"');
		while ( *pszT != '"')
			*pszT2++ = *pszT++;
		*pszT2 = '\0';
		puts(szT);
		gets(szT);
		}
}




