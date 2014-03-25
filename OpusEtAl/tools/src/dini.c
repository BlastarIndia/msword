#include "stdio.h"
#include "dini.h"

/**********************************************************/
/**  DINI.C                                              **/
/**                                                      **/
/**  This program will decode .ini files for OPUS        **/
/**  (aka Microsoft Word for Windows) and print          **/
/**  them out in human-readable format.                  **/
/**                                                      **/
/**                                                      **/
/**  Date:  8/10/88                                      **/
/**  By:    Alan Ezekiel                                 **/
/**                                                      **/
/**  Modification History:                               **/
/**   Date         Who             Comments              **/
/**  11/7/89    Tony Krueger    Updated to new version   **/
/**                                                      **/
/**                                                      **/
/**     Copyright (C)  Microsoft Corporation, 1988       **/
/**********************************************************/


#define errFileOpen             0
#define errReadPrefd            1
#define errOldIniFormat         2
#define errNewIniFormat         3
#define errReadSttbFileCache    4
#define errBadSttbFileCache     5
#define errReadStDMQPath        6
#define errBadStDMQPath         7
#define errReadPrNameAndDriver  8
#define errBadPrNameAndDriver   9
#define errReadPrinterMetrics   10
#define errBadPrinterMetrics    11
#define errReadPrenv            12
#define errBadPrenv             13
#define errReadSttbFont         14
#define errBadSttbFont          15
#define errReadSttbPaf          16
#define errBadSttbPaf           17
#define errReadRgfce            18
#define errBadRgfce             19
#define errReadFontWidths       20
#define errBadFontWidths        21
#define errUnknownErr           22
#define errMajorTrouble         23
#define errMax                  24

char  *rgError[errMax] =
	{
	"Error trying to open file",
	"Error trying to read PREFD structure",
	"Incorrect version number: this is an outdated .INI format",
	"Incorrect version number: This .INI format not understood by DINI",
	"Error trying to read SttbFileCache",
	"Bad sttb in SttbFileCache",
	"Error trying to read StDMQPath",
	"Bad st in StDMQPath",
	"Error trying to read sz group PrNameAndDriver",
	"Bad sz group in PrNameAndDriver",
	"Error trying to read printer metrics",
	"Bad printer metrics",
	"Error trying to read Prenv",
	"Bad Prenv",
	"Error trying to read SttbFont",
	"Bad SttbFont",
	"Error trying to read SttbPaf",
	"Bad SttbPaf",
	"Error trying to read fce array",
	"Bad fce array",
	"Error trying to read font width arrays",
	"Bad font width arrays",
	"Unknown error has occured in DINI",        /* should never happen */
	"Major trouble - error in DINI, cannot recover"
	};


unsigned char Buff[cbBuffMax];                        /* file data buffer */
unsigned char PrefdBuff[sizeof(struct PREFD) + 1];    /* buff for prefd */
struct PREFD *prefd;
bool rgfFontWidths [cFceMax];
int cFce;
int Radix           = HEX;           /* default radix */
bool fSubstCh       = fFalse;        /* substitute '.' for text chars < 0x20 or > 0x7f? */
bool fVerbose       = fFalse;        /* dump in verbose mode? */
bool fIgnoreVerNum  = fFalse;        /* ignore incorrect version numbers */

FILE *pFile;



main(argc, argv)
int argc;
unsigned char *argv[];
{
	int iFce;

	printf("\n     *************************************************************\n");
	printf("     ***  Opus .INI dump utility for Pref Version %d (decimal) ***\n",
			nPrefdVerDini);
	printf("     *************************************************************\n\n");

	ParseCmdLine(argc,argv);            /* also opens source file */

	switch (Radix)
		{
	case OCTAL:
		printf(" - Radix is OCTAL -\n\n");
		break;
	case DECIMAL:
		printf(" - Radix is DECIMAL -\n\n");
		break;
	case HEX:
		printf(" - Radix is HEX -\n\n");
		break;
		}

	ReadPrefdFromFile();
	prefd = (struct PREFD *) PrefdBuff;
	if (!fIgnoreVerNum)
		CheckVersionNumber();
	DumpPrefd();

	if (prefd->cbSttbFileCache)
		{
		printf("\n-------------------------- SttbFileCache ------------------------------\n");
		printSzInt("cbSttbFileCache = ",prefd->cbSttbFileCache);
		printf("\n");
		if (prefd->cbSttbFileCache > cbBuffMax)
			ReadAndDumpLargeSttb(prefd->cbSttbFileCache,errBadSttbFileCache);
		else  /* normal case */			
			{
			ReadFile(prefd->cbSttbFileCache,errReadSttbFileCache);
			DumpSttb(prefd->cbSttbFileCache,errBadSttbFileCache);
			}
		}

	if (prefd->cbStDMQPath)
		{
		printf("\n---------------------------- StDMQPath --------------------------------\n");
		printSzInt("cbStDMQPath = ",prefd->cbStDMQPath);
		printf("\n");
		ReadFile(prefd->cbStDMQPath,errReadStDMQPath);
		DumpStDMQPath(prefd->cbStDMQPath,errBadStDMQPath);
		}


	printf("\n*************************************************************************\n");
	printf("************************ Printer Information ****************************\n");
	printf("*************************************************************************\n\n");

	if (prefd->cbPrNameAndDriver)
		{
		printf("\n----------------------- grpszPrNameAndDriver ---------------------------\n");
		printSzInt("cbPrNameAndDriver = ",prefd->cbPrNameAndDriver);
		printf("\n");
		ReadFile(prefd->cbPrNameAndDriver,errReadPrNameAndDriver);
		DumpGrpsz(prefd->cbPrNameAndDriver,errBadPrNameAndDriver);
		}

	if (prefd->cbPrinterMetrics)
		{
		printf("\n-------------------------- PrinterMetrics ------------------------------\n");
		printSzInt("cbPrinterMetrics = ",prefd->cbPrinterMetrics);
		printf("\n");
		ReadFile(prefd->cbPrinterMetrics,errReadPrinterMetrics);
		DumpPrinterMetrics(prefd->cbPrinterMetrics,errBadPrinterMetrics);
		}

	if (prefd->cbPrenv)
		{
		printf("\n------------------------------ Prenv ----------------------------------\n");
		printSzInt("cbPrenv = ",prefd->cbPrenv);
		printf("\n");
		ReadFile(prefd->cbPrenv,errReadPrenv);
		DumpPrenv(prefd->cbPrenv, errBadPrenv);
		}

	if (prefd->cbSttbFont)
		{
		printf("\n----------------------------- SttbFont ---------------------------------\n");
		printSzInt("cbSttbFont = ",prefd->cbSttbFont);
		printf("\n");
		if (prefd->cbSttbFont > cbBuffMax)
			ReadAndDumpLargeSttb(prefd->cbSttbFont,errBadSttbFont);
		else  /* normal case */			
			{
			ReadFile(prefd->cbSttbFont,errReadSttbFont);
			DumpSttb(prefd->cbSttbFont,errBadSttbFont);
			}
		}

	if (prefd->cbSttbPaf)
		{
		printf("\n----------------------------- SttbPaf ---------------------------------\n");
		printSzInt("cbSttbPaf = ",prefd->cbSttbPaf);
		printf("\n");
		if (prefd->cbSttbFont > cbBuffMax)
			ReadAndDumpLargeSttb(prefd->cbSttbPaf,errBadSttbPaf);
		else  /* normal case */			
			{
			ReadFile(prefd->cbSttbPaf,errReadSttbPaf);
			DumpSttb(prefd->cbSttbPaf,errBadSttbPaf);
			}
		}

	if (prefd->cbRgfce)
		{
		printf("\n------------------------------ Rgfce ----------------------------------\n");
		printSzInt("cbRgfce = ",prefd->cbRgfce);
		printf("\n");
		ReadFile(prefd->cbRgfce,errReadRgfce);
		DumpRgfce(prefd->cbRgfce,errBadRgfce);
		}

	if (prefd->cbFontWidths)
		{
		printf("\n---------------------------- FontWidths --------------------------------\n");
		printSzInt("cbFontWidths = ",prefd->cbFontWidths);
		printf("\n");
		for (iFce = 0 ; iFce < cFce ; iFce++)
			if (rgfFontWidths[iFce])
				{
				ReadFile(cbFontWidth,errReadFontWidths);
				if (fVerbose)
					DumpFontWidths(cbFontWidth,errBadFontWidths);
				}
		if (!fVerbose)
			printf("  (Font width data not shown in non-verbose mode)");
		}

	printf("\n\n-------------------------- End of Source File -------------------------\n\n");
	CloseFile();                /* close the source file */
}


ParseCmdLine(argc, argv)
int argc;
unsigned char *argv[];
{
	int iArgList;       /* index into the command line arguement list */

	/* abort if no command line arguements */
	if (argc < 2)
		BadCmdLine();

	/* find out what flags the user passed */
	for (iArgList = 1; (iArgList < argc) && ((*argv[iArgList] == '/') ||
			(*argv[iArgList] == '-')) ; )
		{
		switch (*(argv[iArgList] + 1))
			{
		case 'D':
		case 'd':
			Radix = DECIMAL;
			break;
		case 'H':
		case 'h':
			Radix = HEX;
			break;
		case 'O':
		case 'o':
			Radix = OCTAL;
			break;
		case 'S':
		case 's':
			fSubstCh = fTrue;
			break;
		case 'V':
		case 'v':
			fVerbose = fTrue;
			break;
		case 'I':
		case 'i':
			fIgnoreVerNum = fTrue;
			break;
		default:
			BadCmdLine();
			}
		if (*(argv[iArgList] + 2) == '/')
			argv[iArgList] += 2;
		else
			iArgList++;
		}
	if (argc == iArgList)
		BadCmdLine();
	OpenFile(argv[iArgList]);   /* open the source file */
}


CheckVersionNumber()
{
	int nVer = (int) PrefdBuff[0];

	if (nPrefPrefdVerCur != nPrefdVerDini)
		{
		printf("WARNING: DINI version does not match PREFD version number.\n");
		printf("         Program continuing, data may be unreliable.\n\n");
		}
	if (nPrefdVerDini > nVer)
		Error(errOldIniFormat);
	if (nPrefdVerDini < nVer)
		Error(errNewIniFormat);
}


/* DumpFoo functions assume that Buff contains enough data for them */

DumpPrefd()
{
	printf("\n\n------------------------------- Prefd ------------------------------------\n\n");

	printSzInt      ("prefd.nPrefPrefdVer               = ",prefd->nPrefPrefdVer);
	printf("\n");
	printSzInt      ("prefd.pref.ut                     = ",prefd->pref.ut);
	printSzBoolForce("prefd.pref.fRibbon                = ",prefd->pref.fRibbon,fFalse);
	printSzBoolForce("prefd.pref.fStatLine              = ",prefd->pref.fStatLine,fFalse);
	printSzBoolForce("prefd.pref.fPromptSI              = ",prefd->pref.fPromptSI,fFalse);
	printSzBoolForce("prefd.pref.fBkgrndPag             = ",prefd->pref.fBkgrndPag,fFalse);
	printSzBoolForce("prefd.pref.fAutoDelete            = ",prefd->pref.fAutoDelete,fFalse);
	printSzBoolForce("prefd.pref.fShortMenus            = ",prefd->pref.fShortMenus,fFalse);
	printSzBoolForce("prefd.pref.fRuler                 = ",prefd->pref.fRuler,fFalse);
	printSzBoolForce("prefd.pref.fHorzScrollBar         = ",prefd->pref.fHorzScrollBar,fFalse);
	printSzBoolForce("prefd.pref.fVertScrollBar         = ",prefd->pref.fVertScrollBar,fFalse);
	printSzBoolForce("prefd.pref.fPageView              = ",prefd->pref.fPageView,fFalse);
	printf("\n");
	printSzBoolForce("prefd.pref.grpfvisi.fvisiTabs     = ",prefd->pref.grpfvisi.S1.fvisiTabs,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fvisiSpaces   = ",prefd->pref.grpfvisi.S1.fvisiSpaces,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fvisiParaMarks = ",prefd->pref.grpfvisi.S1.fvisiParaMarks,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fvisiFtnRefMarks = ",prefd->pref.grpfvisi.S1.fvisiFtnRefMarks,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fvisiCondHyphens = ",prefd->pref.grpfvisi.S1.fvisiCondHyphens,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fvisiShowAll  = ",prefd->pref.grpfvisi.S1.fvisiShowAll,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fNoShowPictures = ",prefd->pref.grpfvisi.S1.fNoShowPictures,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fSeeHidden    = ",prefd->pref.grpfvisi.S1.fSeeHidden,fFalse);
	printSzInt      ("prefd.pref.grpfvisi.flm           = ",prefd->pref.grpfvisi.S1.flm);
	printSzInt      ("prefd.pref.grpfvisi.grpfShowResults = ",prefd->pref.grpfvisi.S1.grpfShowResults);
	printSzBoolForce("prefd.pref.grpfvisi.fDrawTableDrs = ",prefd->pref.grpfvisi.S1.fDrawTableDrs,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fDrawPageDrs  = ",prefd->pref.grpfvisi.S1.fDrawPageDrs,fFalse);
	printSzBoolForce("prefd.pref.grpfvisi.fForceField   = ",prefd->pref.grpfvisi.S1.fForceField,fFalse);
	printf("\n");
	printSzBoolForce("prefd.pref.fDisplayAsPrint        = ",prefd->pref.fDisplayAsPrint,fFalse);
	printSzBoolForce("prefd.pref.fSplAutoSugg           = ",prefd->pref.fSplAutoSugg,fFalse);
	printSzBoolForce("prefd.pref.fShowAllMacros         = ",prefd->pref.fShowAllMacros,fFalse);
	printSzBoolForce("prefd.pref.fDraftView             = ",prefd->pref.fDraftView,fFalse);
	printSzBoolForce("prefd.pref.fZoomApp               = ",prefd->pref.fZoomApp,fFalse);
	printSzBoolForce("prefd.pref.fZoomMwd               = ",prefd->pref.fZoomMwd,fFalse);
	printSzBoolForce("prefd.pref.fEllip                 = ",prefd->pref.fEllip,fFalse);
	printSzBoolForce("prefd.pref.fShowF                 = ",prefd->pref.fShowF,fFalse);
	printSzBoolForce("prefd.pref.fPrDirty               = ",prefd->pref.fPrDirty,fFalse);
	printSzBoolForce("prefd.pref.fSpare                 = ",prefd->pref.fSpare,fFalse);
	printSzBoolForce("prefd.pref.fPrvwTwo               = ",prefd->pref.fPrvwTwo,fFalse);
	printSzUns      ("prefd.pref.dxaStyWnd              = ",prefd->pref.dxaStyWnd);
	printSzSt       ("prefd.pref.stUsrInitl             = ",prefd->pref.stUsrInitl);
	printSzSt       ("prefd.pref.stUsrName              = ",prefd->pref.stUsrName);
	printf("\n");
	printSzInt      ("prefd.cbSttbFileCache             = ",prefd->cbSttbFileCache);
	printSzInt      ("prefd.cbStDMQPath                 = ",prefd->cbStDMQPath);
	printSzInt      ("prefd.cbPrNameAndDriver           = ",prefd->cbPrNameAndDriver);
	printSzInt      ("prefd.cbPrinterMetrics            = ",prefd->cbPrinterMetrics);
	printSzInt      ("prefd.cbPrenv                     = ",prefd->cbPrenv);
	printSzInt      ("prefd.cbSttbFont                  = ",prefd->cbSttbFont);
	printSzInt      ("prefd.cbSttbPaf                   = ",prefd->cbSttbPaf);
	printSzInt      ("prefd.cbRgfce                     = ",prefd->cbRgfce);
	printSzInt      ("prefd.cbFontWidths                = ",prefd->cbFontWidths);
}


DumpSttb(cb,err)
int     cb;             /* amount of valid data in Buff */
int     err;            /* Error to report if mangled data */
{
	struct STTB *sttb = (struct STTB *) Buff;
	int iBuff;
	int iBuffMac;
	struct RGCHECK rgcheck [cStInSttbMax];
	int irgCur;
	int irg;

	printSzInt       ("  sttb.bMax              = ",sttb->bMax);
	printSzInt       ("  sttb.ibstMac           = ",sttb->ibstMac);
	printSzInt       ("  sttb.ibstMax           = ",sttb->ibstMax);
	printSzUns       ("  sttb.cbExtra           = ",sttb->U2.S2.cbExtra);
	printSzBool      ("  sttb.fNoShrink         = ",sttb->U2.S2.fNoShrink);
	printSzBool      ("  sttb.fExternal         = ",sttb->U2.S2.fExternal);
	printSzBool      ("  sttb.fStyleRules       = ",sttb->U2.S2.fStyleRules);
	if (sttb->bMax+cbBaseSttb > cb)
		{
		printf(" - sttb.bMax doesn't equal expected value -\n");
		Error(err);
		}

	iBuffMac = * (int *) Buff;

	/* Check the array, see if the strings are packed in correctly */
	for (irgCur = 0 ; irgCur < sttb->ibstMac ; irgCur++)
		{
		/** find and check string start location **/
		rgcheck[irgCur].start = sttb->U1.rgbst[irgCur];
		if (rgcheck[irgCur].start < 0 || rgcheck[irgCur].start > iBuffMac)
			Error(err);
		/** find and check string end location **/
		rgcheck[irgCur].end = rgcheck[irgCur].start + 
				Buff[rgcheck[irgCur].start+cbBaseSttb];
		if (rgcheck[irgCur].end < 0 || rgcheck[irgCur].end > iBuffMac)
			Error(err);
		rgcheck[irgCur].match = fFalse;         /* initialize */
		/** Match to previously discovered strings **/
		for (irg = 0 ; irg < irgCur ; irg++)
			{
			if (rgcheck[irgCur].end + 1 == rgcheck[irg].start)
				rgcheck[irg].match = fTrue;
			if (rgcheck[irgCur].start == rgcheck[irg].end + 1)
				rgcheck[irgCur].match = fTrue;
			}
		if (rgcheck[irgCur].start == (int) &sttb->U1.rgbst[sttb->ibstMax] - 
				((int)Buff+cbBaseSttb))
			rgcheck[irgCur].match = fTrue;  /* It's the first one */
		}
	/** now examine rgcheck, inform user if any unmatched strings exist **/
	for (irgCur = 0 ; irgCur < sttb->ibstMac ; irgCur++)
		{
		if (!(rgcheck[irgCur].match))
			{
			printf("\n - Strings are not correctly packed in sttb -\n");
			Error(err);
			}
		}
	/** Everything's okay, now let's print the st's or paf's **/
	for (irgCur = 0 ; irgCur < sttb->ibstMac ; irgCur++)
		{
		if (err == errBadSttbPaf)   /* in SttbPaf, we read Paf's, not st's */
			printPaf (&Buff[rgcheck[irgCur].start]);
		else
			printSzSt  ("  sttb.st                = ",
					&Buff[rgcheck[irgCur].start]+cbBaseSttb);
		}
	printf("\n");
}


ReadAndDumpLargeSttb(cb,err)
/* Used only when Sttb too large for Buff */
int     cb;             /* size of expected sttb */
int     err;            /* Error to report if mangled data */
{
	struct STTB *sttb = (struct STTB *) Buff;
	int iBuff;
	int cSt;            /* number of strings we will read */
	int cStRead = 0;    /* number of strings read so far */
	int cbRead;         /* number of bytes read so far */
	int cch;            /* length of st */

	printf("WARNING: Cannot check sttb structure for correctness due to\n");
	printf("         its very large size.  Trying to continue...\n\n");

	/** First, we'll read in and print the header information **/
	ReadFile(sizeof(int) * 4,err);
	cbRead = sizeof(int) * 4;
	printSzInt       ("  sttb.bMax              = ",sttb->bMax);
	printSzInt       ("  sttb.ibstMac           = ",(cSt = sttb->ibstMac));
	printSzInt       ("  sttb.ibstMax           = ",sttb->ibstMax);
	printSzUns       ("  sttb.cbExtra           = ",sttb->U2.S2.cbExtra);
	printSzBool      ("  sttb.fNoShrink         = ",sttb->U2.S2.fNoShrink);
	printSzBool      ("  sttb.fExternal         = ",sttb->U2.S2.fExternal);
	printSzBool      ("  sttb.fStyleRules       = ",sttb->U2.S2.fStyleRules);
	if (sttb->bMax+cbBaseSttb > cb)
		{
		printf(" - sttb.bMax doesn't equal expected value -\n");
		Error(err);
		}

	/** Next, we will read past the array of offsets **/
	cbRead += sttb->ibstMax * sizeof(int);
	ReadFile(sttb->ibstMax * sizeof(int),err);

	/** Now, let's output the strings **/
	while (cbRead < cb && cStRead < cSt)
		{
		cch = fgetc(pFile);
		ungetc (cch,pFile);     /* Now that we've seen it, put it back */
		cbRead += cch + 1;
		if (cbRead > cb)
			{
			printf("\n - String continues past end of structure -\n");
			Error(err);
			}
		ReadFile(cch + 1, err);
		if (err == errBadSttbPaf)   /* in SttbPaf, we read Paf's, not st's */
			printPaf (Buff);
		else
			printSzSt  ("  sttb.st                = ",Buff);
		cStRead += 1;
		}
	if (cStRead == cSt)
		printf("\nCorrect number of strings were read.\n");
	else
		printf("\n - %d strings were read, %d were expected (decimal numbers) -\n");
	if (cbRead < cb)
		ReadFile(cb - cbRead, err);        /* dead space between structs */
	else  if (cbRead != cb)
		{
		printf("\n - Structure size does not match expected byte count -\n");
		Error(err);
		}
	printf("\n");
}


DumpStDMQPath(cb,err)
int cb;
int err;
{
	if (cb > Buff[0] + 1 || cb < Buff[0] - 1)
		{          /* cb may be rounded to nearest even value */
		printf(" - Unexpected length of string -\n");
		Error(err);
		}
	printSzSt ("  stDMQPath   = ",Buff);
	printf("\n");
}


DumpGrpsz(cb,err)
int cb;
int err;
{
	int iBuff = 0;

	while (iBuff < cb)
		{
		printf ("  grpsz.sz               = \"");
		while (Buff[iBuff] != '\0')
			{
			if (iBuff >= cb)
				Error(err);
			printChar(Buff[iBuff++]);
			}
		printf("\"\n");
		iBuff++;            /* Advance past the \0 */
		}
	printf("\n");
}


DumpPrinterMetrics(cb,err)
int cb;
int err;
{
	if (cb != cbSizeOfPrinterMetrics)
		{          /* cb may be rounded to nearest even value */
		printf(" - Unexpected size for printer metrics data -\n");
		Error(err);
		}
	printSzInt ("  pri.dxuInch            = ",* (int *) Buff[0]);
	printSzInt ("  pri.dyuInch            = ",* (int *) Buff[2]);
	printSzInt ("  pri.dxpRealPage        = ",* (int *) Buff[4]);
	printSzInt ("  pri.dypRealPage        = ",* (int *) Buff[6]);
	printSzInt ("  pri.dxmmRealPage       = ",* (int *) Buff[8]);
	printSzInt ("  pri.dymmRealPage       = ",* (int *) Buff[10]);
	printSzInt ("  pri.ptScaleFactor.x    = ",* (int *) Buff[12]);
	printSzInt ("  pri.ptScalefactor.y    = ",* (int *) Buff[14]);
	printSzInt ("  pri.ptUL.x             = ",* (int *) Buff[16]);
	printSzInt ("  pri.ptUL.y             = ",* (int *) Buff[18]);
	printSzInt ("  pri.cBins              = ",* (int *) Buff[20]);
	printSzInt ("  pri.idBinCur           = ",* (int *) Buff[22]);
	printSzInt ("  pri.rgidBin[0]         = ",* (int *) Buff[23]);
	printSzInt ("  pri.rgidBin[1]         = ",* (int *) Buff[24]);
	printSzInt ("  pri.rgidBin[2]         = ",* (int *) Buff[25]);
	printSzInt ("  pri.rgidBin[3]         = ",* (int *) Buff[26]);
	printSzInt ("  pri.rgidBin[4]         = ",* (int *) Buff[27]);
	printSzInt ("  pri.rgidBin[5]         = ",* (int *) Buff[28]);
	printSzInt ("  pri.rgidBin[6]         = ",* (int *) Buff[29]);
	printSzInt ("  pri.dxpPrintable       = ",* (int *) Buff[30]);
	printSzInt ("  pri.dypPrintable       = ",* (int *) Buff[32]);
	printSzInt ("  pri.rgbText            = ",* (int *) Buff[34]);
	/* There are four unused int locations here (8 bytes) */
	printf("\n");
}


DumpPrenv(cb,err)
int cb;
int err;
{
	int iBuff = 0;

	printf("PrEnv is composed of binary printer environment data");
	if (fVerbose)
		{
		printf(":\n");
		DumpRawDataTable(iBuff, cb, fTrue);
		}

	printf("\nFirst element in prenv may be an sz:\n");
	printf ("  sz          = \"");
	while (Buff[iBuff] != '\0' && iBuff < cb)
		printChar(Buff[iBuff++]);
	printf("\"\n\n");
}


DumpRawDataTable(iBuff,cb,fSmall)
/* Dumps raw data in easy-to-read table format */
int     iBuff;
int     cb;
bool    fSmall;
	/*   This function prints tables of ints.  It can  do    */
	/*   either 1-byte or 2-byte ints, adjusting all the     */
	/*   table spacing accordingly.  If fSmall, we want to   */
	/*   make a 1-byte int table, otherwise ints will be 2   */
	/*   bytes long.                                         */
{
	int i;
	int cIntsOnLine;           /* # of ints to put on each line */
	int cInt;                  /* # of ints in table */

	cIntsOnLine = (Radix == HEX && !fSmall ? 8 : Radix);
	/* 16 2-byte ints will not fit on a line */
	cInt = (fSmall ? cb : cb / 2);

	/** index along the top ("  0  1  2  3  4...")   **/
	printf((fSmall ? "\n          " : "\n            "));
	for (i = 0 ; i < cIntsOnLine ; i++)
		{
		if (fSmall)
			printSmallInt(i);
		else
			printTableInt(i);
		}
	/** Line of dashes separating index from data **/
	printf("\n  ----");
	for (i = 0 ; i < cIntsOnLine + 1 ; i++)
		{
		if (fSmall)
			printf("----");
		else
			printf("------");
		}
	/** Now let's print the numbers **/
	for (i = iBuff ; i < cInt ; i++)
		{
		if ((i - iBuff) % cIntsOnLine == 0)      /* do we need a new line? */
			{
			printf("\n  ");
			if (fSmall)
				printSmallInt(i);
			else
				printTableInt(i);
			printf("  : ");
			}
		if (fSmall)                             /* print a number */
			printSmallInt(Buff[i]);
		else
			printTableInt(* (int *) &Buff[i*2]);
		}
	printf("\n");
}


DumpRgfce(cb,err)
int cb;
int err;
{
	struct FCE *fce;
	int iFce;

	cFce = cb / sizeof(struct FCE);
	if (cFce * sizeof(struct FCE) != cb)
		{
		printf("\n - Incorrect rgfce structure size (not an even number of fce's) -\n");
		Error(err);
		}

	printSzInt ("Number of fce structures in rgfce: ",cFce);

	for (iFce = 0 ; iFce < cFce ; iFce++ )
		{
		fce = (struct FCE *) &Buff[iFce * sizeof(struct FCE)];
		printSzInt  ("\nfce number ",iFce + 1);
		printf      ("  fce.fcidActual:\n");
		DumpFcid (fce->fcidActual);
		printSzInt  ("  fce.dxpOverhang        = ",fce->dxpOverhang);
		printSzInt  ("  fce.dypAscent          = ",fce->dypAscent);
		printSzInt  ("  fce.dypDescent         = ",fce->dypDescent);
		printSzInt  ("  fce.dypExtraDescent    = ",fce->dypXtraDescent);
		printSzBool ("  fce.fPrinter           = ",fce->fPrinter);
		printSzBool ("  fce.fVisiBad           = ",fce->fVisiBad);
		printSzBool ("  fce.fFixedPitch        = ",fce->fFixedPitch);
		printSzLong ("  fce.dxpWidth           = ",fce->dxpWidth);
		printf      ("  fce.fcidRequest:\n");
		DumpFcid (fce->fcidRequest);
		printSzInt  ("  fce.pfceNext           = ",(int) fce->pfceNext);
		printSzInt  ("  fce.pfcePrev           = ",(int) fce->pfcePrev);
		printSzInt  ("  fce.hfont              = ",(int) fce->hfont);
		/* We'll need this later: */
		rgfFontWidths[iFce] = fce->fPrinter && !fce->fFixedPitch;
		}
	printf("\n");
}


DumpFcid(fcid)
struct FCID fcid;
{
	printSzBool ("    fcid.fBold           = ",fcid.fBold);
	printSzBool ("    fcid.fItalic         = ",fcid.fItalic);
	printSzBool ("    fcid.fStrike         = ",fcid.fStrike);
	printSzInt  ("    fcid.kul             = ",fcid.kul);
	printSzBool ("    fcid.fFixedPitch     = ",fcid.fFixedPitch);
	printSzInt  ("    fcid.hps             = ",fcid.hps);
	printSzInt  ("    fcid.ibstFont        = ",fcid.ibstFont);
}


DumpFontWidths(cb,err)
int cb;
int err;
{
	printf("Font width data:");
	DumpRawDataTable(0, cb, fFalse);
	printf("\n");
}


ReadPrefdFromFile()
{
	int cbRead;

	if ( (cbRead = fread(PrefdBuff, 1, sizeof(struct PREFD), pFile))
			== sizeof(struct PREFD) )
		return(cbRead);
	if (cbRead > sizeof(int))
		{
		printf("Incorrect PrefD structure size:\n");
		printSzInt("apparent PrefD version number is ",(int)PrefdBuff[0]);
		}
	Error(errReadPrefd);
}


OpenFile(pszFn)
/* open the given file for reading */
unsigned char *pszFn;
{
	if ((pFile = fopen(pszFn, "rb")) == NULL)
		{
		Error(errFileOpen);
		exit(1);
		}
}


CloseFile()
{
	fclose(pFile);
}


ReadFile(cb, err)
/* read cb bytes */
int cb;
int err;
{
	int cbRead;

	if (cb > cbBuffMax)
		{
		printf("\n - Maximum buffer size exceed - tried to read too many bytes -\n");
		Error(err);
		}
	cbRead = fread(Buff, 1, cb, pFile);
	if (cb == cbRead)
		return;
	Error(err);
}


BadCmdLine()
	/* something wrong with the command line - print an error message
	* and then print the usage for the user
	*/
{
	printf("Usage:\n");
	printf("    DINI [/d/h/o/s/v/i] Source_File_Name\n\n");
	printf("    Flags:\n");
	printf("        /d - decimal notation\n");
	printf("        /h - hexadecimal notation\n");
	printf("        /o - octal notation\n");
	printf("        /s - substitute all text chars < 0x20 or > 0x7f with a '.' in the dump\n");
	printf("        /v - verbose mode\n");
	printf("        /i - ignore incorrect version numbers\n\n\n");
	exit(1);
}


printSmallInt(value)
int value;
{
	switch (Radix)
		{
	case OCTAL:
		printf("%4o", value);
		break;
	case DECIMAL:
		printf("%4d", value);
		break;
	case HEX:
		printf("%4x", value);
		break;
		}
}


printTableInt(value)
int value;
{
	switch (Radix)
		{
	case OCTAL:
		printf("%6o", value);
		break;
	case DECIMAL:
		printf("%6d", value);
		break;
	case HEX:
		printf("%6x", value);
		break;
		}
}


printInt(value)
int value;
{
	switch (Radix)
		{
	case OCTAL:
		printf("%-14o", value);
		break;
	case DECIMAL:
		printf("%-14d", value);
		break;
	case HEX:
		printf("%-14x", value);
		break;
		}
}


printSzInt(psz,value)
char    *psz;
int     value;
{
	printf("%s",psz);
	printInt(value);
	printf("\n");
}


printUns(value)
unsigned    value;
{
	switch (Radix)
		{
	case OCTAL:
		printf("%-14o", value);
		break;
	case DECIMAL:
		printf("%-14u", value);
		break;
	case HEX:
		printf("%-14x", value);
		break;
		}
}


printSzUns(psz,value)
char        *psz;
unsigned    value;
{
	printf("%s",psz);
	printUns(value);
	printf("\n");
}


printLong(value)
long    value;
{
	switch (Radix)
		{
	case OCTAL:
		printf("%-14lo", value);
		break;
	case DECIMAL:
		printf("%-14ld", value);
		break;
	case HEX:
		printf("%-14lx", value);
		break;
		}
}


printSzLong(psz,value)
char    *psz;
long    value;
{
	printf("%s",psz);
	printLong(value);
	printf("\n");
}


printBool(value)
bool    value;
{
	if (value)
		printf ("true");
	else
		printf ("false");
}


printSzBoolForce(psz,value,fForcePrint)
char    *psz;
bool    value;
{
	if (fForcePrint || fVerbose || value)
		{
		printf("%s",psz);
		printBool(value);
		printf("\n");
		}
}


printChar(ch)
char    ch;
{
	if (fSubstCh && (ch < (char)32 || ch > (char)126))
		printf(".");
	else
		printf("%c",ch);
}


printSt(pst)
char    *pst;
{
	int     i = (int) *pst;
	char    *pch;

	for ( pch = pst + 1 ; i > 0 ; i--)
		printChar(*pch++);
}


printSzSt(psz,pst)
char    *psz;
char    *pst;
{
	printf("%s\"",psz);
	printSt(pst);
	printf("\"\n");
}


printPaf(pPaf)
char    pPaf[];
{
	int cbPaf = (int) pPaf[0];
	int i;

	printSzInt ("  sttb.Paf.ibst          = ",(int) pPaf[1]);
	for ( i = 1 ; i < cbPaf ; i++)
		printSzInt ("      .Paf.rghps[]       = ",(int)pPaf[i+1]);
}


Error(ierr)
int     ierr;
{
	if (ierr < 0 || ierr > errMax)
		ierr = errUnknownErr;
	printf("\n** %s **\n\n", rgError[ierr]);
	exit(1);
}


