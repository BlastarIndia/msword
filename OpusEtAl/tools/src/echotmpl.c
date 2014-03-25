/* E C H O T M P L . C */
/* usage: template-file source-file
Read template file.  Each line is of the form:
	<cItem> <Item1> <Item2> ... <Itemn> <Template>
Item is a variable found in source-file (results in int).
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


#define cItemMax 8

FILE *fpTemplate;
FILE *fpSource;
FILE *fpAlternate;

int     nSecond;
int     nMinute;
int     nHour;
int     nDay;
int     nMonth;
int     nYear;

char szBuffer[256];

main(cArg, rgpchArg)
int cArg;
char *rgpchArg[];
{
	GetTime();

	if (cArg < 3)
		{
		fprintf(stderr, "usage: echotmpl template-file sourcefile");
		exit(1);
		}

	if ((fpTemplate = fopen(rgpchArg[1], "rt")) == NULL)
		ErrorSz("cannot open ", rgpchArg[1]);

	if ((fpSource = fopen(rgpchArg[2], "rt")) == NULL)
		ErrorSz("cannot open ", rgpchArg[2]);

	if (cArg > 3 && (fpAlternate = fopen(rgpchArg[3], "rt")) == NULL)
		ErrorSz("cannot open ", rgpchArg[3]);

	while (!feof(fpTemplate))
		ProcessTemplate();

	fclose(fpTemplate);
	fclose(fpSource);

	exit(0);
}


GetTime()
{
	long lTime;
	struct tm *ptm;

	time(&lTime);
	ptm = localtime(&lTime);

	nSecond = ptm->tm_sec;
	nMinute = ptm->tm_min;
	nHour = ptm->tm_hour;
	nDay = ptm->tm_mday;
	nMonth = ptm->tm_mon+1;
	nYear = ptm->tm_year;
}


ProcessTemplate()
{
	char rgchItem[256];
	int rgnItem[cItemMax];
	int cItems, i;

	if (fscanf(fpTemplate, "%d ", &cItems) == EOF)
		return;

	if (!cItems--)
		{
		printf("\n");
		return;
		}

	if (cItems > cItemMax)
		ErrorSz("too many items","");

	if (cItems > 0)
		{
		for (i = 0; i < cItems; i++)
			{
			fscanf(fpTemplate, "%s ", rgchItem);
			if (rgchItem[0] == '\\')
				rgnItem[i] = NGetDefinedItem(rgchItem);
			else
				rgnItem[i] = NGetItem(rgchItem);
			}
		}

	fgets(rgchItem, 256, fpTemplate);

	switch (cItems)
		{
	case 0:
		printf(rgchItem);
		break;
	case 1:
		printf(rgchItem, rgnItem[0]);
		break;
	case 2:
		printf(rgchItem, rgnItem[0], rgnItem[1]);
		break;
	case 3:
		printf(rgchItem, rgnItem[0], rgnItem[1], rgnItem[2]);
		break;
	case 4:
		printf(rgchItem, rgnItem[0], rgnItem[1], rgnItem[2], rgnItem[3]);
		break;
	case 5:
		printf(rgchItem, rgnItem[0], rgnItem[1], rgnItem[2], rgnItem[3], rgnItem[4]);
		break;
	case 6:
		printf(rgchItem, rgnItem[0], rgnItem[1], rgnItem[2], rgnItem[3], rgnItem[4],
				rgnItem[5]);
		break;
	case 7:
		printf(rgchItem, rgnItem[0], rgnItem[1], rgnItem[2], rgnItem[3], rgnItem[4],
				rgnItem[5], rgnItem[6]);
		break;
	case 8:
		printf(rgchItem, rgnItem[0], rgnItem[1], rgnItem[2], rgnItem[3], rgnItem[4], 
				rgnItem[5], rgnItem[6], rgnItem[7]);
		break;
		}
}


ErrorSz(pch1, pch2)
char *pch1, *pch2;
{
	fprintf(stderr, "echotmpl: error: %s%s\r\n", pch1, pch2);
	exit(2);
}


int NGetItem(pch)
char *pch;
{

	int n;
	char rgchBuf1[80];
	char rgchBuf2[80];

	if (fseek(fpSource, 0l, SEEK_SET) != 0)
		ErrorSz("cannot seek file", "");

	for (;;)
		{
		if (fscanf(fpSource, "%s %s %d", rgchBuf1, rgchBuf2, &n) == EOF)
			ErrorSz("tag not found: ", pch);

		if (!strcmp(pch, rgchBuf2))
			return n;
		}
}


int NGetDefinedItem(pch)
char *pch;
{
	pch++;

	if (!strncmp(pch, "se", 2))
		return nSecond;
	else  if (!strncmp(pch, "mi", 2))
		return nMinute;
	else  if (!strncmp(pch, "hr", 2))
		return nHour;
	else  if (!strncmp(pch, "da", 2))
		return nDay;
	else  if (!strncmp(pch, "mo", 2))
		return nMonth;
	else  if (!strncmp(pch, "yr", 2))
		return nYear;
	else  if (!strncmp(pch, "cd", 2))
		return CDaysFromPch(pch+2);
	else  if (*pch > '0' && *pch <= '9')
		{
		int iLine = atoi(pch);
		int i;
		if (fpAlternate == NULL)
			ErrorSz("no alternate file specified for ", pch);

		for (i = 0; i < iLine; i++)
			{
			fgets(szBuffer, 256, fpAlternate);
			if (feof(fpAlternate))
				ErrorSz("cannot seek to line ", pch);
			}
		for (i = 0; szBuffer[i] != 0; i++)
			if (szBuffer[i] == '\n' || szBuffer[i] == '\r')
				szBuffer[i] = 0;
		return (int) (char *)szBuffer;
		}

	else
		ErrorSz("Unknown defined item: ", pch);
}


struct DTTM
	{
	int yr;
	int mon;
	int dom;
};

#define dyrBase 		1900
#define dttmYrFirst		dyrBase
#define dttmYrLast		2411
#define dttmMonFirst		1    /* Of course, the first month is January */
#define dttmMonLast		12   /* and the last month is December.       */


char mpmonddom[2][12] =
/* 1   2   3   4   5   6   7   8   9  10  11  12 */
	{{ 
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	}};


/* The number of days from December 31, 1899, Sunday. */
long LDaysFrom1900Dttm(dttm)
struct DTTM dttm;
{
	int           imon, imonCur;
	long          lDays;
	char          *rgddom;

	lDays = 0L;
	rgddom = &mpmonddom[FLeapYr(dttm.yr + dyrBase)][0];
	imonCur = dttm.mon - 1;
	for (imon = 0; imon < imonCur; lDays += rgddom[imon++]);
	lDays += dttm.dom;

	if (dttm.yr != 0)
		{
		int yr;
		int cLeapYear;

		yr = dttm.yr - 1;
		if (yr + dyrBase < 2000)  /* year 1900 through 1999 */
			{
			cLeapYear = yr / 4;
			}
		else
			{
			yr += dyrBase - 2000;
			cLeapYear = 25 + (yr / 400) + 24 * (yr / 100) + ((yr % 100) / 4);
			}
		lDays += 365 * ((long) dttm.yr) + cLeapYear;
		}
	return (lDays);
}


long LDaysFromDttm(dttmFrom, dttmTo)
struct DTTM dttmFrom, dttmTo;
{
	return (LDaysFrom1900Dttm(dttmTo) - LDaysFrom1900Dttm(dttmFrom));
}


FLeapYr(yr)
int	    yr;
{
	return (((yr & 0x0003) == 0 && yr % 100 != 0) || yr % 400 == 0);
}


CDaysFromPch(pch)
char *pch;
{
	struct DTTM dttm1, dttm2;

	if (fscanf(fpTemplate, " %d %d %d ", &dttm1.mon, &dttm1.dom, &dttm1.yr)
			== EOF)
		return 0;

	dttm2.yr = nYear;
	dttm2.mon = nMonth;
	dttm2.dom = nDay;

	return (int)LDaysFromDttm(dttm2, dttm1);
}




