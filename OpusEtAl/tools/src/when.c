#include <stdio.h>
#include <time.h>

#define PASCAL pascal
#define FAR far

void            FAR PASCAL OsTime(char *);
void            FAR PASCAL OsDate(char *);

	struct TIM {                    /* Time structure returned by OsTime */
	char minutes, hour, hsec, sec;
};

	struct DAT {                    /* Date structure returned by OsDate */
	int  year;
	char month, day, dayOfWeek;
};

char rgch[256];
char chDelim;


main(argc, argv)
int argc;
char *argv[];

{
	long ltime;

	if (argc == 1)
		{
		time(&ltime);
		printf("%s", ctime(&ltime));
		}
	else  if ((chDelim = (*++argv)[0]) == '-' || chDelim == '/')
		{
		switch (*(argv[0] + 1))
			{
		case 'n':    /* only arg is n - C format w/ newline */
			time(&ltime);
			printf("%s\n", ctime(&ltime));
			break;
		case 'd':
			GetSzDate(rgch);
			if  ( (*(argv[0] + 2)) == 'n')
				printf("%s\n", rgch);
			else
				printf("%s", rgch);
			break;
		case 't':
			GetSzTime(rgch);
			if  ( (*(argv[0] + 2)) == 'n' )
				printf("%s\n", rgch);
			else
				printf("%s", rgch);
			break;
		default:
			printf("%s\n", "usage: when [-|/d|t]n");
			printf("%s\n", " -d prints date in [m]m-dd-yy");
			printf("%s\n", " /d prints date in [m]m/dd/yy");
			printf("%s\n", " -/t prints time in [h]h:mm a|p.m.");
			printf("%s\n", " -/n causes a newline to print after string");
			printf("%s\n", " no flag prints time date in C format");
			break;
			}
		}
	return;
}





/* ****
*
	Module: GetSzTime(pch)
*  Description: put a string containing the time in the form [h]h:mm {A|P}M
*               into a buffer pointerd to by pch.
*  Output: string in buffer
** ***/

GetSzTime(pch)
char *pch;
{
	struct TIM tim;
	register int cDigits;

	OsTime( (char *)&tim );   /* get current time */

	/* move in hours */
	cDigits = CchIntToPPch((tim.hour <= 12 ? tim.hour : tim.hour - 12),
			&pch);
	*(pch++) = ':';
	if (tim.minutes < 10)
		*(pch++) = '0';   /* force in leading 0 for minutes */

	cDigits = CchIntToPPch(tim.minutes, &pch);
	*(pch++) = ' ';

	if (tim.hour > 12)
		*(pch++) = 'p';
	else
		*(pch++) = 'a';

	*(pch++) = '.';
	*(pch++) = 'm';
	*(pch++) = '.';
	*(pch++) = 0;

}


/* ****
*
	Module: GetSzDate(pch)
*  Description: put a string containing the Date in the form [M]M-[D]D-YY
*               into a buffer pointed to by pch.
*        INCREDIBLE HACK - the switch character is used as the date delimiter
*          call when /d to get mm/dd/yy or when -d for mm-dd-yy
*  Output: string in buffer
** ***/

GetSzDate(pch)
char *pch;
{
	struct DAT dat;

	OsDate( (char *)&dat );   /* get current Date */

	CchIntToPPch((int)dat.month, &pch);
	*(pch++) = chDelim;

	CchIntToPPch((int)dat.day, &pch);
	*(pch++) = chDelim;

	dat.year -= 1900;  /* just want yy */
	CchIntToPPch(dat.year, &pch);
	*(pch++) = 0;

}



int CchIntToPPch(n, ppch)
register int n;
char **ppch;
{
	register int cch = 0;

	if (n < 0)
		{
		*(*ppch)++ = '-';
		n = -n;
		++cch;
		}

	if (n >= 10)
		{
		cch += CchIntToPPch(n / 10, ppch);
		n %= 10;
		}

	*(*ppch)++ = '0' + n;
	return cch + 1;
}




/* fake stuck here for easy linking - not called */

int ANSITOOEM()
{
}


