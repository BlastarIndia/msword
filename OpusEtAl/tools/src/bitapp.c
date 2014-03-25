#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitapp.h"

#define	BLow(n)	(n&0xff)
#define BHigh(n) ((n>>8)&0xff)

#define MAXLINE  130

/* 
*  danp      11 Aug 88
*/

FILE 	*fpOrig = NULL;
FILE	*fpDest = NULL;
FILE 	*fopen();

BOOL	vfMakeGrey = FALSE;

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	/* open files */
	int wFigure;

	ParseCmdLine(cArg, rgszArg);

	fprintf(fpDest, "/* THIS IS A GENERATED FILE -- DO NOT EDIT */\n\n");

	switch (wFigure = GetFigureType())
		{
	case FIG_BITMAP:
		CreateBitmapHeaderFile();
		break;
	case FIG_CURSOR:
	case FIG_ICON:
		CreateFigureHeaderFile(wFigure);
		break;

otherwise:
		Usage(BADFIGURE);
		}

	fcloseall();
	exit(0);
}


ParseCmdLine(cArg, rgszArg)
int		cArg;
char	*rgszArg[];
{
	int	cCurArg = 1;

	while (cCurArg < cArg)
		{
		if (*(rgszArg[cCurArg]) == '/')
			SetFlag(*(rgszArg[cCurArg]+1));
		else
			OpenFile(rgszArg[cCurArg]);
		cCurArg++;
		}
	if ((fpOrig == NULL) || (fpDest == NULL))
		Usage(WRONGPARAM);						/* short a file or two */
}


OpenFile(psz)
char *psz;
{
	static int iPass = 0;

	switch (iPass)
		{
	case 0:
		if ((fpOrig = fopen(psz, "rb")) != NULL)
			iPass++;
		else
			{
			printf("\nError: cannot open %s\r\n", psz);
			Usage(BADFILE);
			}
		break;
	case 1:
		if ((fpDest = fopen(psz, "wt")) != NULL)
			iPass++;
		else
			{
			printf("\nError: cannot open %s\r\n", psz);
			Usage(BADFILE);
			}
		break;
	default:
		Usage(WRONGPARAM);
		break;
		}
}


SetFlag( ch )
char	ch;
{
	switch (ch)
		{
	case	'g':
	case	'G':
		vfMakeGrey = TRUE;
		break;

	default:
		Usage(BADSWITCH);
		break;
		}
}


GetFigureType()
{
	char	rgch[2];

	fread( rgch, sizeof(char), 2, fpOrig);
	return rgch[0];
}


CreateBitmapHeaderFile()
{
	int	cRows, cWordsWidth;
	int	cSigBit, cbTotal;

	fprintf(fpDest, "{\n\t");

	DumpBitmapParameters(&cRows, &cWordsWidth, &cSigBit, &cbTotal);

	fprintf(fpDest, "\t{\t");
	DumpBits(cRows, cWordsWidth, cSigBit);
	fprintf(fpDest, " \n\t},\n");

	fprintf(fpDest, "\t%u,\n",cbTotal);

	fprintf(fpDest, "},\n");
}


CreateFigureHeaderFile(wFigure)
int	wFigure;
{
	int    cRows, cWordsWidth;

	fprintf(fpDest, "{\n\t{\n\t\t");

	DumpFigureParameters(&cRows, &cWordsWidth);

	fprintf(fpDest, "\n\t\t");
	DumpBits(cRows, cWordsWidth, 0);
	fprintf(fpDest, ",\n");

	fprintf(fpDest, "\t\t");
	DumpBits(cRows, cWordsWidth, 0);
	fprintf(fpDest, " \n\t},\n");

	fprintf(fpDest, "\t%u, \n", 12+(4*cRows*cWordsWidth));

	fprintf(fpDest, "},\n");
}


DumpBitmapParameters(pcRows, pcWordsWidth, pcSigBit, pcbTotal)
int	*pcRows, *pcWordsWidth;
int	*pcSigBit, *pcbTotal;
{
	BITMAP	bm;

	fread(&bm, sizeof(BITMAP), 1, fpOrig);

	*pcRows = bm.bmHeight;
	*pcWordsWidth = bm.bmWidthBytes / 2;

	if (vfMakeGrey)
		{
		*pcSigBit = 16-((*pcWordsWidth*16)-bm.bmWidth);
		bm.bmWidthBytes = (((bm.bmWidth*2)+15)/16)*2;
		bm.bmWidth *= 2;
		}
	*pcbTotal = bm.bmWidthBytes * bm.bmHeight;

	fprintf(fpDest, "{ %u, %u, %u, %u, %u, %u, NULL },\n", bm.bmType,
			bm.bmWidth, bm.bmHeight, bm.bmWidthBytes, bm.bmPlanes,
			bm.bmBitsPixel );
}


DumpFigureParameters(pcRow,pcWordsWidth)
int	*pcRow, *pcWordsWidth;
{
	int ich;
	RCI rci;
	char *pch;

	fread(&rci, sizeof(RCI), 1, fpOrig);
	for (ich = 0, pch = (char *)&rci; ich < sizeof(RCI); ich++)
		fprintf(fpDest, " %u,", *pch++);
	*pcRow = rci.cy;
	*pcWordsWidth = rci.WidthBytes / 2;
}


DumpBits(cRows, cWordsWidth, cSigBit)
int	cRows, cWordsWidth;
int	cSigBit;
{
	int    iRow = 0;

	fprintf(fpDest, "\n\t\t");

	while (iRow < cRows)
		{
		if (iRow < cRows)
			fprintf(fpDest, "\/*%u*\/\t", iRow);

		if (vfMakeGrey)
			DumpRowAndGreyBits(cWordsWidth, iRow, cSigBit);
		else
			DumpRowBits(cWordsWidth);

		iRow++;
		if (iRow < cRows)
			fprintf(fpDest, ",\n\t\t");
		}
}


DumpRowBits(cWordsWidth)
int	cWordsWidth;
{
	WORD    wCurByts;
	int	iWordOut = 0;
	int	iWordPat = 0;

	while (iWordPat < cWordsWidth)
		{

		fread(&wCurByts, sizeof(WORD), 1, fpOrig);
		if (feof(fpOrig))
			Usage(BADEOF);

		fprintf( fpDest, "0x%.2x, 0x%.2x",
				BLow(wCurByts), BHigh(wCurByts));
		if (++iWordPat != cWordsWidth)
			fprintf( fpDest, ", ");

		iWordOut++;

		if ((iWordOut == 4) && (iWordPat != cWordsWidth))
			{
			fprintf(fpDest, "\n\t\t\t");
			iWordOut = 0;
			}
		}
}


DumpRowAndGreyBits(cWordsWidth, iRow, cSigBit)
int	cWordsWidth, iRow;
int	cSigBit;
{
	WORD	wCurByts;
	int	iWordOut = 0;
	int	iWordPat = 0;

	WORD	rgwGrey[100];
	BYTE 	irgw = 0;

	/* kcmod */
	static WORD	rgBitMask[17] = 
		{ 
		0x0000, 0x8000, 0xc000, 0xe000, 
		0xf000,
		0xf800, 0xfc00, 0xfe00, 0xff00,
		0xff80, 0xffc0, 0xffe0, 0xfff0,
		0xfff8, 0xfffc, 0xfffe, 0xffff 		};

	while (iWordPat < cWordsWidth)
		{

		fread(&wCurByts, sizeof(WORD), 1, fpOrig);
		wCurByts = (BLow(wCurByts) << 8) | BHigh(wCurByts);
		if (feof(fpOrig))
			Usage(BADEOF);

		rgwGrey[irgw++] = wCurByts
				| ((iRow & 1) ? ODD_MASK : EVEN_MASK);

		if (iWordPat+1 < cWordsWidth)
			{
			fprintf(fpDest, "0x%.2x, 0x%.2x, ", 
					BHigh(wCurByts), BLow(wCurByts));
			if (++iWordOut == 4)
				{
				fprintf(fpDest, "\n\t\t\t");
				iWordOut = 0;
				}
			}
		iWordPat++;
		}

	wCurByts = (rgBitMask[cSigBit] & wCurByts) | 
			((rgwGrey[0] >> cSigBit) & ~rgBitMask[cSigBit]);
	fprintf(fpDest, "0x%.2x, 0x%.2x, ",
			BHigh(wCurByts), BLow(wCurByts));
	if (++iWordOut == 4)
		{
		fprintf(fpDest, "\n\t\t\t");
		iWordOut = 0;
		}

	rgwGrey[irgw] = 65535;
	if (cSigBit < 9) irgw--;
	for ( iWordPat = 0; iWordPat < irgw; iWordPat++)
		{
		wCurByts = (rgwGrey[iWordPat] << (16-cSigBit)) |
				((rgwGrey[iWordPat+1] >> cSigBit) & ~rgBitMask[cSigBit]);
		fprintf( fpDest, "0x%.2x, 0x%.2x",
				BHigh(wCurByts), BLow(wCurByts));
		if (iWordPat + 1 < irgw)
			fprintf( fpDest, ", ");
		if ((++iWordOut == 4) && (iWordPat + 1 < irgw))
			{
			fprintf(fpDest, "\n\t\t\t");
			iWordOut = 0;
			}
		}

}


Usage(error)
int error;
{
	switch (error)
		{
	case WRONGPARAM:
		printf("\n Wrong Number of Arguments!  Usage:\n");
		printf("\n bitapp [\g] file1 file2\n");
		printf("\n \g    - Created Greyed Bitmap.\n");
		printf("\n file1 - original bitmap file.\n");
		printf(" file2 - created header file.\n");
		break;

	case BADFILE:
		/* error is reported in the FOpenFile function */
		break;

	case BADFIGURE:
		printf("\n Input file is not a resource file!\n");
		break;

	case BADSWITCH:
		printf("\n Invalid Switch on command line.\n");
		break;

	case BADEOF:
		printf("\n Unexpected End Of File Reached in Input file!\n");
		}

	exit(1);
}


