/* D U M P  R S H . C */
/*  This program converts a .RSH file into a more readable form. */

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
FILE *fpBcm = NULL;

long rgsecTotal[7] = { 0l, 0l, 0l, 0l };

main (argc, argv)
int argc;
char *argv[];
{
	if (argc < 2 || (fpIn = fopen(argv[1], "rb")) == NULL)
		Error ("Cannot open input file");

	if ((fpBcm = fopen(argc < 3 ? "WINWORD.BCM" : argv[2], "rb")) == NULL)
		Error("Cannot find or open WINWORD.BCM");

	DumpHeader();
	while(!feof(fpIn))
		DumpItem();

	DumpTotalTimes();

	fcloseall();
	exit(0);
}

Error(pch)
char *pch;
{
	fprintf (stderr, "error: %s.\n", pch);
	fcloseall();
	exit(1);
}

char *mpwdysz[] =
	{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
	};


/* WARNING: Structure based on WinWord 1.0 (1.0.5900) sources!!! */
struct UAH 
	{
	int nVer; /* magic number */
	unsigned mint:	 6; /* 0-59 */
	unsigned hr:	 5; /* 0-23 */
	unsigned dom:	 5; /* 1-31 */
	unsigned mon:	 4; /* 1-12 */
	unsigned yr:	 9; /* (1900-2411)-1900 */
	unsigned wdy:	 3; /* 0(Sun)-6(Sat) */
	char szUsrName[50];
	unsigned fIncr : 1;
	unsigned nRev : 6;
	unsigned nMinor : 6;
	unsigned nMajor : 3;
	};

#define nVerUahCur 0x4322

DumpHeader()
{
	struct UAH uah;

	if (fread(&uah, sizeof(uah), 1, fpIn) != 1)
		Error("file corrupted");

	if (uah.nVer != nVerUahCur)
		Error("File has wrong magic number");

	fprintf(stdout, "\nSession started on %s %2.2d/%2.2d/%2.2d at %2.2d:%2.2d.\n",
			mpwdysz[uah.wdy], uah.mon, uah.dom, uah.yr, uah.hr, uah.mint);
	fprintf(stdout, "Running version %d.%d.%d.%d.\n", uah.nMajor, uah.nMinor,
			uah.nRev, uah.fIncr);
	fprintf(stdout, "Operator: %s.\n\n", uah.szUsrName);

}

FFindBcm(bcm, pbme)
unsigned bcm;
struct BME *pbme;
{
	if (fseek(fpBcm, 0l, SEEK_SET) != 0)
		Error("Cannot seek WINWORD.BCM");

	while (!feof(fpBcm))
		{
		if (fread(pbme, cbBME, 1, fpBcm) != 1)
			Error("Read error on WINWORD.BCM");
		if (pbme->bcm == bcm)
			return fTrue;
		}
	return fFalse;
}

char *mpwszDefined[] =
	{
		"Context Sensitive Help",	/* 0x0000 */
		"Template Macro",			/* 0x0001 */
		"Global Macro",				/* 0x0002 */
		"Typing",					/* 0x0003 */
		"\t(Cancelled or Error)",	/* 0x0004 */
		"Iconbar Dialog Get Focus", /* 0x0005 */
		"Iconbar Dialog Lose Focus",/* 0x0006 */
		"Backspace",				/* 0x0007 */
		"Style applied from ruler", /* 0x0008 */
		"Font/Pts applied from ribbon",/*0x0009*/
		"Quicksave requested",		/* 0x000a */
		"Fullsave requested",		/* 0x000b */
		"Idle",							/* 0x000c */
		"\tStart Dialog Session",		/* 0x000d */
		"\tStop Dialog Session",		/* 0x000e */
	};
#define wDefinedMax sizeof(mpwszDefined)/sizeof(char *)

char *mpwszCursor[] =
	{
	/* NOTE: order matched rgkcCursor in command2.c */
		"NextPara",
		"PrevPara",
		"WordRight",
		"WordLeft",
		"WordRightAlt",
		"WordLeftAlt",
		"EndLine",
		"BeginLine",
		"Right",
		"Left",
		"TopScreen",
		"EndScreen",
		"Down",
		"Up",
		"PageDown",
		"PageUp",
		"EndDoc",
		"TopDoc",
		"Clear",
	};
#define wCursorMax sizeof(mpwszCursor)/sizeof(char *)

char *mpwszLooksKey[] =
	{
	/* NOTE: order must match ilcd tables in cmdlook.h */
		"Bold",
		"Italic",
		"SmallCaps",
		"SingleUnderline",
		"WordUnderline",
		"DoubleUnderline",
		"Superscript",
		"Subscript",
		"Vanish",
		"PlainTextForStyle",
		"Strikethrough",
		"RevisionMark",
		"NormalParaForStyle",
		"ParaJustifyLeft",
		"ParaJustifyCenter",
		"ParaJustifyRight",
		"ParaJustifyBoth",
		"ParaSpace1",
		"ParaSpace1.5",
		"ParaSpace2",
		"ParaClose",
		"ParaOpen",
	};
#define wLooksKeyMax sizeof(mpwszLooksKey)/sizeof(char *)

char *mpwszRulerDrag[] =
	{
	/* NOTE: order must match imk definitions in ruler.h */
		"LeftTab",
		"CenterTab",
		"RightTab",
		"DecimalTab",
		"FirstlineIndent",
		"LeftIndent",
		"RightIndent",
		"DefaultTab (?!)",
		"LeftTableIndent",
		"TableColumn",
		"LeftMargin",
		"RightMargin",
		"LeftAndFirstIndents",
	};
#define wRulerDragMax sizeof(mpwszRulerDrag)/sizeof(char *)

char *mpwszMessage[] =
	{
	/* NOTE: order must match rgwmRsh in wproc.c */
		"QueryEndSession",
		"EndSession",
		"ActivateApp",
		"LeftButtonDown",
		"RightButtonDown",
		"LeftDoubleClick",
		"RightDoubleClick",
		"LeftButtonDown",
		"RightButtonDown",
		"LeftDoubleClick",
		"RightDoubleClick",
		"Close",
		"VerticalScroll",
		"HorizontalScroll",
	};
#define wMessageMax sizeof(mpwszMessage)/sizeof(char *)

char *mpwszCommand[] =
	{
	/* NOTE: order must match rgscRsh in wproc.c */
		"Size",
		"Move",
		"Minimize",
		"Maximize",
		"NextWindow",
		"PrevWindow",
		"Close",
		"VerticalScroll",
		"HorizontalScroll",
		"MouseMenu",
		"KeyMenu",
		"Restore"
	};
#define wCommandMax sizeof(mpwszCommand)/sizeof(char *)

char *mpwszKeyState[] =
	{
	/* NOTE: based on code in wproc.c */
	"(none)",
	"Shift",
	"Alt",
	"Alt+Shift",
	"Ctrl",
	"Ctrl+Shift",
	"Ctrl+Alt",
	"Ctrl+Alt+Shift"
	};
#define wKeyStateMax sizeof(mpwszKeyState)/sizeof(char *)

char *rgszTime[] =
	{
		"Idle          ",
		"Command       ",
		"Navigation    ",
		"Typing        ",
		"Menu Idle     ",
		"Dlg Overhead  ",
		"Dialog Session",
	};


DumpItem()
{
	unsigned w;
	unsigned timestamp;
	struct BME bme;
	char *pch1 = "UNKNOWN";
	char *pch2 = "";
	char *pch3 = " ";

	if (fread(&w, sizeof(unsigned), 1, fpIn) != 1)
		return;

	else if (fread(&timestamp, sizeof(unsigned), 1, fpIn) != 1)
		return;

	else if ((w&0xf000) == 0x0000 && (w&0x0fff) < wDefinedMax)
		/* defined */
		pch1 = mpwszDefined[w&0x0fff];

						/* 0x1000 - unused */

	else if ((w&0xf000) == 0x2000 && (w&0x0fff) < wCursorMax)
		/* cursor */
		{
		pch1 = "Curs ";
		pch2 = mpwszCursor[w&0x0fff];
		}

	else if ((w&0xf000) == 0x3000 && (w&0x0fff) < wCursorMax)
		/* curs extend */
		{
		pch1 = "Curs Extend ";
		pch2 = mpwszCursor[w&0x0fff];
		}

						/* 0x4000 - unused */

	else if ((w&0xf000) == 0x5000 && (w&0x0fff) < wLooksKeyMax)
		/* looks key */
		{
		pch1 = "Looks Key ";
		pch2 = mpwszLooksKey[w&0x0fff];
		}

	else if ((w&0xf000) == 0x6000 && (w&0x0fff) < wRulerDragMax)
		/* ruler drag */
		{
		pch1 = "Ruler Drag ";
		pch2 = mpwszRulerDrag[w&0x0fff];
		}

	else if ((w&0xf000) == 0x7000)
		/* windows message */
		{
		switch (w&0x0C00)
			{
		case 0x0000:
			pch1 = "Win Message ";
			break;
		case 0x0400:
			pch1 = "App Message ";
			break;
		case 0x0800:
			pch1 = "Mwd Message ";
			break;
		case 0x0C00:
			pch1 = "Wwd Message ";
			}
		if ((w&0x00ff) < wMessageMax)
			pch2 = mpwszMessage[w&0x00ff];
		else
			pch2 = "?";
		}

	else if ((w&0xf000) == 0x8000)
		/* sys command */
		{
		switch (w&0x0C00)
			{
		case 0x0000:
			pch1 = "Win Command ";
			break;
		case 0x0400:
			pch1 = "App Command ";
			break;
		case 0x0800:
			pch1 = "Mwd Command ";
			break;
		case 0x0C00:
			pch1 = "Wwd Command ";
			}
		if ((w&0x00ff) < wCommandMax)
			pch2 = mpwszCommand[w&0x00ff];
		else
			pch2 = "?";
		}

	else if ((w&0xf000) == 0x9000 && (w&0x0fff) < wKeyStateMax)
		{
		pch1 = "\tKey State: ";
		pch2 = mpwszKeyState[w&0x0fff];
		}

	else if ((w&0xf000) == 0xf000)
		/* special handling */
		{
		switch (w&0x0fff)
			{
		default:
			pch1 = "?";
			break;
		case 0x0001:
			/* BCM Follows */
			{
			unsigned bcm;
			if (fread(&bcm, sizeof(unsigned), 1, fpIn) != 1)
				Error("Ill formed BcmFollows");
			if (fread(&timestamp, sizeof(unsigned), 1, fpIn) != 1)
				Error("No timestamp after bcm");
			if (!FFindBcm(bcm, &bme))
				pch1 = "UNKNOWN BCM (check for correct version of WINWORD.BCM)";
			else
				pch1 = bme.szName;
			w = bcm;
			pch3 = "b";
			break;
			}
		case 0x0002:
			/* Times follow */
			{
			int rgsec[7];
			long rgsecLong[7];
			int i;

			if (fread(rgsec, sizeof(int), 7, fpIn) != 7)
				Error("Ill formed TimesFollow");

			for (i = 0; i < 7; i++)
				{
				rgsecTotal[i] += rgsec[i];
				rgsecLong[i] = rgsec[i];
				}

			fprintf(stdout, "\t\tTime marker.  Elapsed times:\n");

			DumpTimes("\t\t", rgsecLong);

			return;
			}
			}
		}

	DumpTimestamp(timestamp);
	fprintf(stdout, "\t(%4.4x)%s\t%s%s\n", w, pch3, pch1, pch2);
}


DumpTimestamp(timestamp)
unsigned timestamp;
{
	fprintf(stdout, "%3u:%2.2u", timestamp/60, timestamp%60);
}

DumpTime(sec, secTotal)
long sec, secTotal;
{
	fprintf(stdout, "%3ld:%2.2ld", sec/60l, sec%60l);
	if (secTotal != 0l)
		fprintf(stdout, " (%2ld%%)", ((sec*100l)+(secTotal/2l))/secTotal);
	fprintf(stdout, "\n");
}

DumpTimes(szPrefix, rgsec)
char *szPrefix;
long rgsec[];
{

	int i;
	long secTotal = 0;

	for (i = 0; i < 4; i++)
		secTotal += rgsec[i];

	for (i = 0; i < 4; i++)
		{
		fprintf(stdout, "%s\t%s", szPrefix, rgszTime[i]);
		DumpTime((long)rgsec[i], secTotal);
		}

	fprintf(stdout, "%s\tTotal         ", szPrefix);
	DumpTime(secTotal, 0l);

	fprintf(stdout, "%s    Sub timers:\n", szPrefix);

	for (i = 4; i < 7; i++)
		{
		fprintf(stdout, "%s\t%s", szPrefix, rgszTime[i]);
		DumpTime((long)rgsec[i], secTotal);
		}
}



DumpTotalTimes()
{
	fprintf(stdout, "\nEnd of session.  Total elapsed times:\n");
	DumpTimes("", rgsecTotal);
}

