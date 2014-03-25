#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dnatfile.h"

/*
*  Dump NATive FILE utility
*
*  By      Greg Cox
*  Date    11/19/86
*
*
*  Modifications for new file format
*         By           Alan Ezekiel
*         Date           6/22/88
*  Updated structures
*         By           Tony Krueger
*         Date           11/7/89
*
*
*  Copyright (C)  Microsoft Corporation, 1986
*/


struct FIB Fib;             /* file header */
unsigned char DataBuff[cbBuffMax];   /* file data buffer */
int iDataBuff;
int cbBuffMac = 0;

unsigned char *pErrorMess[] =
	{
	"Read Error - trying to read the file header!",
	"Source File Name Not Given!",
	"Unrecognized Command Line Arguement Given!",
	"Unable To Open Source File!",
	"Seek Error - trying to seek to the file header!",
	"Seek Error - trying to seek to text part of file!",
	"Seek Error - trying to seek to character properties part of file!",
	"Read Error - trying to read character properties part of file!",
	"Error - Illegal sprm code!",
	"Error - Not enough bytes to finish sprm args!",
	"Seek Error - trying to seek to paragraph properties part of file!",
	"Read Error - trying to read paragraph properties part of file!",
	"Seek Error - trying to seek to style sheet part of file!",
	"Read Error - trying to read style sheet part of file!",
	"Error - stsh length inconsistant with stsh data!",
	"Error - sttb length inconsistant with sttb data!",
	"Error - sttbName st length inconsistant with bMax!",
	"Error - sttbChpx length inconsistant with bMax!",
	"Error - sttbPapx length inconsistant with bMax!",
	"Seek Error - trying to seek to Assoc part of file!",
	"Read Error - trying to read Assoc part of file!",
	"Seek Error - trying to seek to bookmark part of file!",
	"Read Error - trying to read bookmark part of file!",
	"Error - plc length inconsistant with plc data!",
	"Read Error - trying to read text part of file!",
	"Seek Error - trying to seek to section descriptor part of file!",
	"Read Error - trying to read section descriptor part of file!",
	"Seek Error - trying to seek to page descriptor part of file!",
	"Read Error - trying to read page descriptor part of file!",
	"Seek Error - trying to seek to header descriptor part of file!",
	"Read Error - trying to read header descriptor part of file!",
	"Seek Error - trying to seek to document properties of file!",
	"Read Error - trying to read document properties!",
	"Seek Error - trying to seek to paragraph properties location table (PLCBTE)!",
	"Read Error - trying to read paragraph properties location table (PLCBTE)!",
	"Seek Error - trying to seek to complex part of file",
	"Read Error - trying to read complex part of file"
	};


int Radix = 16;                 /* default radix */
BOOL fDumpPict = fFalse;        /* dump pictures? */
BOOL fDumpTextLong = fFalse;    /* dump text? */
BOOL fSubstCh = fFalse;         /* substitute '.' for text chars < 0x20 or > 0x7f? */
BOOL fVerbose = fFalse;         /* dump in verbose mode? */
BOOL fHeaderOnly = fFalse;      /* dump header info only */

FILE *pFile;

int LinePos;



/* This array includes all possible sprms and their corresponding */
/* numbers of arguments.  It is compiled from the definition of   */
/* ESPRM in PRCSUBS.C.                                            */

#define sprmMax         166             /* total number of sprms */
#define sprmNoop        0

struct SPRMDEF SprmDef[sprmMax] =
	{
	{ "sprmNoop"                         ,0 },
	{ "sprmTleReplace"                   ,3 },
	{ "sprmPStc"                         ,2 },
	{ "sprmPStcPermute"                  ,0 },
	{ "sprmPIncLvl"                      ,2 },
	{ "sprmPJc"                          ,2 },
	{ "sprmPFSideBySide"                 ,2 },
	{ "sprmPFKeep"                       ,2 },
	{ "sprmPFKeepFollow"                 ,2 },
	{ "sprmPFPageBreakBefore"            ,2 },
	{ "sprmPBrcl"                        ,2 },
	{ "sprmPBrcp"                        ,2 },
	{ "sprmPNfcSeqNumb"                  ,2 },
	{ "sprmPNoSeqNumb"                   ,2 },
	{ "sprmPFNoLineNumb"                 ,2 },
	{ "sprmPChgTabsPapx"                 ,0 },
	{ "sprmPDxaRight"                    ,3 },
	{ "sprmPDxaLeft"                     ,3 },
	{ "sprmPNest"                        ,3 },
	{ "sprmPDxaLeft1"                    ,3 },
	{ "sprmPDyaLine"                     ,3 },
	{ "sprmPDyaBefore"                   ,3 },
	{ "sprmPDyaAfter"                    ,3 },
	{ "sprmPChgTabs"                     ,0 },
	{ "sprmPFInTable"                    ,2 },
	{ "sprmPFTtp"                        ,2 },
	{ "sprmPDxaAbs"                      ,3 },
	{ "sprmPDyaAbs"                      ,3 },
	{ "sprmPDxaWidth"                    ,3 },
	{ "sprmPPc"                          ,2 },
	{ "sprmPBrcTop"                      ,3 },
	{ "sprmPBrcLeft"                     ,3 },
	{ "sprmPBrcBottom"                   ,3 },
	{ "sprmPBrcRight"                    ,3 },
	{ "sprmPBrcBetween"                  ,3 },
	{ "sprmPBrcBar"                      ,3 },
	{ "sprmPFromText"                    ,3 },
	{ "sprmPSpare2"                      ,0 },
	{ "sprmPSpare3"                      ,0 },
	{ "sprmPSpare4"                      ,0 },
	{ "sprmPSpare5"                      ,0 },
	{ "sprmPSpare6"                      ,0 },
	{ "sprmPSpare7"                      ,0 },
	{ "sprmPSpare8"                      ,0 },
	{ "sprmPSpare9"                      ,0 },
	{ "sprmPSpare10"                     ,0 },
	{ "sprmPSpare11"                     ,0 },
	{ "sprmPSpare12"                     ,0 },
	{ "sprmPSpare13"                     ,0 },
	{ "sprmPSpare14"                     ,0 },
	{ "sprmPSpare15"                     ,0 },
	{ "sprmPSpare16"                     ,0 },
	{ "sprmPRuler"                       ,0 },
	{ "sprmCFStrikeRM"                   ,2 },
	{ "sprmCFRMark"                      ,2 },
	{ "sprmCFFldVanish"                  ,2 },
	{ "sprmCSpare0"                      ,0 },
	{ "sprmCDefault"                     ,0 },
	{ "sprmCPlain"                       ,1 },
	{ "sprmCMajority"                    ,0 },
	{ "sprmCFBold"                       ,2 },
	{ "sprmCFItalic"                     ,2 },
	{ "sprmCFStrike"                     ,2 },
	{ "sprmCFOutline"                    ,2 },
	{ "sprmCFShadow"                     ,2 },
	{ "sprmCFSmallCaps"                  ,2 },
	{ "sprmCFCaps"                       ,2 },
	{ "sprmCFVanish"                     ,2 },
	{ "sprmCFtc"                         ,3 },
	{ "sprmCKul"                         ,2 },
	{ "sprmCSizePos"                     ,4 },
	{ "sprmCQpsSpace"                    ,2 },
	{ "sprmCSpare000"                    ,0 },
	{ "sprmCIco"                         ,2 },
	{ "sprmCSpare1"                      ,0 },
	{ "sprmCSpare2"                      ,0 },
	{ "sprmCSpare3"                      ,0 },
	{ "sprmCSpare4"                      ,0 },
	{ "sprmCSpare5"                      ,0 },
	{ "sprmCSpare6"                      ,0 },
	{ "sprmCSpare7"                      ,0 },
	{ "sprmCSpare8"                      ,0 },
	{ "sprmCSpare9"                      ,0 },
	{ "sprmCSpare10"                     ,0 },
	{ "sprmCSpare11"                     ,0 },
	{ "sprmCSpare12"                     ,0 },
	{ "sprmCSpare13"                     ,0 },
	{ "sprmCSpare14"                     ,0 },
	{ "sprmCSpare15"                     ,0 },
	{ "sprmCSpare16"                     ,0 },
	{ "sprmCSpare17"                     ,0 },
	{ "sprmCSpare18"                     ,0 },
	{ "sprmCSpare19"                     ,0 },
	{ "sprmCSpare20"                     ,0 },
	{ "sprmPicBrcl"                      ,2 },
	{ "sprmPicScale"                     ,0 },
	{ "sprmPicSpare0"                    ,0 },
	{ "sprmPicSpare1"                    ,0 },
	{ "sprmPicSpare2"                    ,0 },
	{ "sprmPicSpare3"                    ,0 },
	{ "sprmPicSpare4"                    ,0 },
	{ "sprmPicSpare5"                    ,0 },
	{ "sprmPicSpare6"                    ,0 },
	{ "sprmPicSpare7"                    ,0 },
	{ "sprmPicSpare8"                    ,0 },
	{ "sprmPicSpare9"                    ,0 },
	{ "sprmPicSpare10"                   ,0 },
	{ "sprmPicSpare11"                   ,0 },
	{ "sprmPicSpare12"                   ,0 },
	{ "sprmPicSpare13"                   ,0 },
	{ "sprmPicSpare14"                   ,0 },
	{ "sprmPicSpare15"                   ,0 },
	{ "sprmPicSpare16"                   ,0 },
	{ "sprmPicSpare17"                   ,0 },
	{ "sprmPicSpare18"                   ,0 },
	{ "sprmPicSpare19"                   ,0 },
	{ "sprmPicSpare20"                   ,0 },
	{ "sprmSBkc"                         ,2 },
	{ "sprmSFTitlePage"                  ,2 },
	{ "sprmSCcolumns"                    ,3 },
	{ "sprmSDxaColumns"                  ,3 },
	{ "sprmSFAutoPgn"                    ,2 },
	{ "sprmSNfcPgn"                      ,2 },
	{ "sprmSDyaPgn"                      ,3 },
	{ "sprmSDxaPgn"                      ,3 },
	{ "sprmSFPgnRestart"                 ,2 },
	{ "sprmSFEndnote"                    ,2 },
	{ "sprmSLnc"                         ,2 },
	{ "sprmSGrpfIhdt"                    ,2 },
	{ "sprmSNLnnMod"                     ,3 },
	{ "sprmSDxaLnn"                      ,3 },
	{ "sprmSDyaHdrTop"                   ,3 },
	{ "sprmSDyaHdrBottom"                ,3 },
	{ "sprmSLBetween"                    ,2 },
	{ "sprmSVjc"                         ,2 },
	{ "sprmSLnnMin"                      ,3 },
	{ "sprmSPgnStart"                    ,3 },
	{ "sprmSSpare2"                      ,0 },
	{ "sprmSSpare3"                      ,0 },
	{ "sprmSSpare4"                      ,0 },
	{ "sprmSSpare5"                      ,0 },
	{ "sprmSSpare6"                      ,0 },
	{ "sprmSSpare7"                      ,0 },
	{ "sprmSSpare8"                      ,0 },
	{ "sprmSSpare9"                      ,0 },
	{ "sprmSSpare10"                     ,0 },
	{ "sprmTJc"                          ,3 },
	{ "sprmTDxaLeft"                     ,3 },
	{ "sprmTDxaGapHalf"                  ,3 },
	{ "sprmTBrcOutside"                  ,3 },
	{ "sprmTBrcInsideHoriz"              ,3 },
	{ "sprmTBrcInsideVert"               ,3 },
	{ "sprmTDefTable"                    ,0 },
	{ "sprmTDyaRowHeight"                ,3 },
	{ "sprmTSpare2"                      ,0 },
	{ "sprmTSpare3"                      ,0 },
	{ "sprmTSpare4"                      ,0 },
	{ "sprmTSpare5"                      ,0 },
	{ "sprmTInsert"                      ,5 },
	{ "sprmTDelete"                      ,3 },
	{ "sprmTDxaCol"                      ,5 },
	{ "sprmTMerge"                       ,3 },
	{ "sprmTSplit"                       ,3 },
	{ "sprmTSetBrc"                      ,6 },
	{ "sprmCHps"                         ,2 },
	{ "sprmCHpsPos"                      ,2 }
	};	





main(argc, argv)
int argc;
unsigned char *argv[];
{
	int iArgList;       /* index into the command line arguement list */

	/* abort if no command line arguements */
	if (argc < 2)
		BadCmdLine(SCE_FN_NOT_GIVEN);

	/* find out what flags the user passed */
	for (iArgList = 1; (iArgList < argc) && (*argv[iArgList] == '/');)
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

		case 'P':
		case 'p':
			fDumpPict = fTrue;
			break;

		case 'S':
		case 's':
			fSubstCh = fTrue;
			break;

		case 'T':
		case 't':
			fDumpTextLong = fTrue;
			break;

		case 'V':
		case 'v':
			fVerbose = fTrue;
			break;

		case 'N':
		case 'n':
			fHeaderOnly = fTrue;
			break;

		default:
			BadCmdLine(BAD_CMD_LINE_ARG);
			}
		if (*(argv[iArgList] + 2) == '/')
			argv[iArgList] += 2;
		else
			iArgList++;
		}

	if (argc == iArgList)
		BadCmdLine(SCE_FN_NOT_GIVEN);

	printf("\nOPUS file dump utility.\nFor OPUS fib versions %i through %i (decimal).\n",
			nFibVersionMin, nFibVersionLast);

	switch (Radix)
		{
	case OCTAL:
		printf("Radix is OCTAL\n\n");
		break;

	case DECIMAL:
		printf("Radix is DECIMAL\n\n");
		break;

	case HEX:
		printf("Radix is HEX\n\n");
		break;
		}


	OpenFile(argv[iArgList]);   /* open the source file */

	DumpHeader();               /* dump the header of the source file */
	if (!fHeaderOnly)
		{
		DumpDop();                  /* dump the document properties */
		DumpText();                 /* dump the text of the file */
		if (Fib.cbStshf)
			DumpStshf();            /* dump style sheet */
		if (Fib.cbSttbfAssoc)
			DumpSttbfAssoc();       /* Associated strings */
		if (Fib.cbPlcffndRef || Fib.cbPlcffndTxt)
			DumpPlcffnd();          /* dump footnote tables */
		if (Fib.cbPlcfandRef || Fib.cbPlcfandTxt)
			DumpPlcfand();          /* dump annotations */
		if (Fib.cbPlcfsed)
			DumpPlcfsed();          /* dump section table */
		if (Fib.cbPlcfpgd)
			DumpPlcfPgd();          /* dump page table */
		if (Fib.cbPlcffldMom || Fib.cbPlcffldHdr || Fib.cbPlcffldFtn ||
				Fib.cbPlcffldAtn || Fib.cbPlcffldMcr)
			DumpPlcffld();          /* dump fields */
		if (Fib.cbPlcfglsy || Fib.cbSttbfglsy)
			Dumpglsy();             /* dump glossary tables */
		if (Fib.cbPlcfhdd)
			DumpPlcfhdd();          /* dump header table */
		DumpChpx();                 /* dump character properties */
		DumpPlcfbtePapx();
		DumpPapx();                 /* dump paragraph properties */
		if (Fib.cbSttbfbkmk || Fib.cbPlcfbkf || Fib.cbPlcfbkl)
			Dumpbkmk();             /* dump bookmark properties */
		/* REVIEW: Cmds */
		/* REVIEW: Plcmcr, Sttbfmcr */
		if (Fib.cbClx)
			DumpClx();              /* dump complex part */
		}

	fcloseall();
}





DumpHeader()
	/* dump the contents of the file header */

{
	BOOL fVerboseSave;
	int cbRead;


	fVerboseSave = fVerbose;
	fVerbose = fTrue;


	/* get the header from the file */
	SeekFile(0L, SEEK_ERR_HDR);
	if (ReadFile(cbFIB, &Fib) != cbFIB)
		{
		Error(READ_ERR_HDR, fTrue);
		exit(1);
		}

	if (Fib.nFib < nFibVersionMin || Fib.nFib > nFibVersionLast)
		printf("WARNING: This file's fib version (%d) not understood by this version of DNATFILE.\n\n", Fib.nFib);


	printf("******************  HEADER DATA  *****************\n\n");

	printUns ("wIdent           = ", Fib.wIdent, 2, 2, fTrue);

	printUns ("nFib             = ", Fib.nFib, 1, 1, fTrue);
	printUns ("nProduct         = ", Fib.nProduct, 0, 0, fTrue);
	printf("  [%d.%2.2d (%d%s)]\n", (Fib.nProduct>>13)&0x07,
			(Fib.nProduct>>7)&0x3f,(Fib.nProduct>>1)&0x3f,
			(Fib.nProduct&1)? "XX":"00");
	printUns ("nLocale          = ", Fib.nLocale, 1, 1, fTrue);

	printUns ("pnNext           = ", Fib.pnNext, 1, 1, fTrue);
	printBool("fDot             = ", Fib.fDot, 1, 1, fTrue);
	printBool("fGlsy            = ", Fib.fGlsy, 1, 1, fTrue);
	printBool("fComplex         = ", Fib.fComplex, 1, 1, fTrue);
	printBool("fHasPic          = ", Fib.fHasPic, 1, 1, fTrue);
	printUns ("cQuickSaves      = ", Fib.cQuickSaves, 1, 1, fTrue);
	printUns ("nFibBack         = ", Fib.nFibBack, 2, 2, fTrue);

	if (fHeaderOnly)
		return;

	if (fVerboseSave)
		{
		printUns ("rgwSpare0[0]     = ", Fib.rgwSpare0[5], 0, 0, fTrue);
		printUns ("rgwSpare0[1]     = ", Fib.rgwSpare0[1], 1, 1, fTrue);
		printUns ("rgwSpare0[2]     = ", Fib.rgwSpare0[2], 0, 0, fTrue);
		printUns ("rgwSpare0[3]     = ", Fib.rgwSpare0[3], 1, 1, fTrue);
		printUns ("rgwSpare0[4]     = ", Fib.rgwSpare0[4], 2, 2, fTrue);
		}

	printLong("fcMin            = ", Fib.fcMin, 1, 1, fTrue);
	printLong("fcMac            = ", Fib.fcMac, 2, 2, fTrue);

	printLong("cbMac            = ", Fib.cbMac, 2, 2, fTrue);

	if (fVerboseSave)
		{
		printLong ("fcSpare0         = ", Fib.fcSpare0, 0, 0, fTrue);
		printLong ("fcSpare1         = ", Fib.fcSpare1, 1, 1, fTrue);
		printLong ("fcSpare2         = ", Fib.fcSpare2, 0, 0, fTrue);
		printLong ("fcSpare3         = ", Fib.fcSpare3, 2, 2, fTrue);
		}

	printLong("ccpText          = ", Fib.ccpText, 1, 1, fTrue);
	printLong("ccpFtn           = ", Fib.ccpFtn, 1, 1, fTrue);
	printLong("ccpHdd           = ", Fib.ccpHdd, 1, 1, fTrue);
	printLong("ccpMcr           = ", Fib.ccpMcr, 1, 1, fTrue);
	printLong("ccpAtn           = ", Fib.ccpAtn, 2, 2, fTrue);
	if (fVerboseSave)
		{
		printLong("ccpSpare0        = ", Fib.ccpSpare0, 1, 1, fTrue);
		printLong("ccpSpare1        = ", Fib.ccpSpare1, 1, 1, fTrue);
		printLong("ccpSpare2        = ", Fib.ccpSpare2, 1, 1, fTrue);
		printLong("ccpSpare3        = ", Fib.ccpSpare3, 2, 2, fTrue);
		}

	printLong("fcStshfOrig      = ", Fib.fcStshfOrig, 0, 0, fTrue);
	printUns ("cbStshfOrig      = ", Fib.cbStshfOrig, 1, 1, fTrue);

	printLong("fcStshf          = ", Fib.fcStshf, 0, 0, fTrue);
	printUns ("cbStshf          = ", Fib.cbStshf, 1, 1, fTrue);

	printLong("fcPlcffndRef     = ", Fib.fcPlcffndRef, 0, 0, fTrue);
	printUns ("cbPlcffndRef     = ", Fib.cbPlcffndRef, 1, 1, fTrue);

	printLong("fcPlcffndTxt     = ", Fib.fcPlcffndTxt, 0, 0, fTrue);
	printUns ("cbPlcffndTxt     = ", Fib.cbPlcffndTxt, 1, 1, fTrue);

	printLong("fcPlcfandRef     = ", Fib.fcPlcfandRef, 0, 0, fTrue);
	printUns ("cbPlcfandRef     = ", Fib.cbPlcfandRef, 1, 1, fTrue);

	printLong("fcPlcfandTxt     = ", Fib.fcPlcfandTxt, 0, 0, fTrue);
	printUns ("cbPlcfandTxt     = ", Fib.cbPlcfandTxt, 1, 1, fTrue);

	printLong("fcPlcfsed        = ", Fib.fcPlcfsed, 0, 0, fTrue);
	printUns ("cbPlcfsed        = ", Fib.cbPlcfsed, 1, 1, fTrue);

	printLong("fcPlcfpgd        = ", Fib.fcPlcfpgd, 0, 0, fTrue);
	printUns ("cbPlcfpgd        = ", Fib.cbPlcfpgd, 1, 1, fTrue);

	printLong("fcPlcfphe        = ", Fib.fcPlcfphe, 0, 0, fTrue);
	printUns ("cbPlcfphe        = ", Fib.cbPlcfphe, 1, 1, fTrue);

	printLong("fcSttbfglsy      = ", Fib.fcSttbfglsy, 0, 0, fTrue);
	printUns ("cbSttbfglsy      = ", Fib.cbSttbfglsy, 1, 1, fTrue);

	printLong("fcPlcfglsy       = ", Fib.fcPlcfglsy, 0, 0, fTrue);
	printUns ("cbPlcfglsy       = ", Fib.cbPlcfglsy, 1, 1, fTrue);

	printLong("fcPlcfhdd        = ", Fib.fcPlcfhdd, 0, 0, fTrue);
	printUns ("cbPlcfhdd        = ", Fib.cbPlcfhdd, 1, 1, fTrue);

	printLong("fcPlcfbteChpx    = ", Fib.fcPlcfbteChpx, 0, 0, fTrue);
	printUns ("cbPlcfbteChpx    = ", Fib.cbPlcfbteChpx, 1, 1, fTrue);

	printLong("fcPlcfbtePapx    = ", Fib.fcPlcfbtePapx, 0, 0, fTrue);
	printUns ("cbPlcfbtePapx    = ", Fib.cbPlcfbtePapx, 1, 1, fTrue);

	printLong("fcPlcfsea        = ", Fib.fcPlcfsea, 0, 0, fTrue);
	printUns ("cbPlcfsea        = ", Fib.cbPlcfsea, 1, 1, fTrue);

	printLong("fcSttbfffn       = ", Fib.fcSttbfffn, 0, 0, fTrue);
	printUns ("cbSttbfffn       = ", Fib.cbSttbfffn, 1, 1, fTrue);

	printLong("fcPlcffldMom     = ", Fib.fcPlcffldMom, 0, 0, fTrue);
	printUns ("cbPlcffldMom     = ", Fib.cbPlcffldMom, 1, 1, fTrue);

	printLong("fcPlcffldHdr     = ", Fib.fcPlcffldHdr, 0, 0, fTrue);
	printUns ("cbPlcffldHdr     = ", Fib.cbPlcffldHdr, 1, 1, fTrue);

	printLong("fcPlcffldFtn     = ", Fib.fcPlcffldFtn, 0, 0, fTrue);
	printUns ("cbPlcffldFtn     = ", Fib.cbPlcffldFtn, 1, 1, fTrue);

	printLong("fcPlcffldAtn     = ", Fib.fcPlcffldAtn, 0, 0, fTrue);
	printUns ("cbPlcffldAtn     = ", Fib.cbPlcffldAtn, 1, 1, fTrue);

	printLong("fcPlcffldMcr     = ", Fib.fcPlcffldMcr, 0, 0, fTrue);
	printUns ("cbPlcffldMcr     = ", Fib.cbPlcffldMcr, 1, 1, fTrue);

	printLong("fcSttbfbkmk      = ", Fib.fcSttbfbkmk, 0, 0, fTrue);
	printUns ("cbSttbfbkmk      = ", Fib.cbSttbfbkmk, 1, 1, fTrue);

	printLong("fcPlcfbkf        = ", Fib.fcPlcfbkf, 0, 0, fTrue);
	printUns ("cbPlcfbkf        = ", Fib.cbPlcfbkf, 1, 1, fTrue);

	printLong("fcPlcfbkl        = ", Fib.fcPlcfbkl, 0, 0, fTrue);
	printUns ("cbPlcfbkl        = ", Fib.cbPlcfbkl, 1, 1, fTrue);

	printLong("fcCmds           = ", Fib.fcCmds, 0, 0, fTrue);
	printUns ("cbCmds           = ", Fib.cbCmds, 1, 1, fTrue);

	printLong("fcPlcmcr         = ", Fib.fcPlcmcr, 0, 0, fTrue);
	printUns ("cbPlcmcr         = ", Fib.cbPlcmcr, 1, 1, fTrue);

	printLong("fcSttbfmcr       = ", Fib.fcSttbfmcr, 0, 0, fTrue);
	printUns ("cbSttbfmcr       = ", Fib.cbSttbfmcr, 1, 1, fTrue);

	printLong("fcPrEnv          = ", Fib.fcPrEnv, 0, 0, fTrue);
	printUns ("cbPrEnv          = ", Fib.cbPrEnv, 1, 1, fTrue);

	printLong("fcWss            = ", Fib.fcWss, 0, 0, fTrue);
	printUns ("cbWss            = ", Fib.cbWss, 1, 1, fTrue);

	printLong("fcDop            = ", Fib.fcDop, 0, 0, fTrue);
	printUns ("cbDop            = ", Fib.cbDop, 1, 1, fTrue);

	printLong("fcSttbfAssoc     = ", Fib.fcSttbfAssoc, 0, 0, fTrue);
	printUns ("cbSttbfAssoc     = ", Fib.cbSttbfAssoc, 1, 1, fTrue);

	printLong("fcClx            = ", Fib.fcClx, 0, 0, fTrue);
	printUns ("cbClx            = ", Fib.cbClx, 1, 1, fTrue);

	printLong("fcPlcfpgdFtn     = ", Fib.fcPlcfpgdFtn, 0, 0, fTrue);
	printUns ("cbPlcfpgdFtn     = ", Fib.cbPlcfpgdFtn, 1, 1, fTrue);

	if (fVerboseSave)
		{
		printLong("fcSpare4         = ", Fib.fcSpare4, 0, 0, fTrue);
		printUns ("cbSpare4         = ", Fib.cbSpare4, 1, 1, fTrue);

		printLong("fcSpare5         = ", Fib.fcSpare5, 0, 0, fTrue);
		printUns ("cbSpare5         = ", Fib.cbSpare5, 1, 1, fTrue);

		printLong("fcSpare6         = ", Fib.fcSpare6, 0, 0, fTrue);
		printUns ("cbSpare6         = ", Fib.cbSpare6, 1, 1, fTrue);

		printInt ("wSpare4          = ", Fib.wSpare4, 1, 1, fTrue);
		}


		printLong("fcSpare7         = ", Fib.fcSpare4, 0, 0, fTrue);
		printInt ("cbSpare7         = ", Fib.cbSpare4, 1, 1, fTrue);

	printUns ("pnChpFirst       = ", Fib.pnChpFirst, 0, 0, fTrue);
	printUns ("pnPapFirst       = ", Fib.pnPapFirst, 1, 1, fTrue);

	printUns ("cpnBteChp        = ", Fib.cpnBteChp, 0, 0, fTrue);
	printUns ("cpnBtePap        = ", Fib.cpnBtePap, 1, 1, fTrue);

	fVerbose = fVerboseSave;
}



DumpText()
	/* dump out the text of the file */
{
	unsigned char szOut[70];
	int iBuff;
	long cbDumped;
	int cLines;
	FC fcCur;


	printf("\n\n\n*********************  TEXT  *********************\n");

	/* seek to the start of the text area of the file */
	SeekFile(Fib.fcMin, SEEK_ERR_TEXT);

	for (cLines = 0, cbDumped = 0, iBuff = cbBuffMax; cbDumped < Fib.ccpText;
			cLines++)
		{


		if (iBuff >= cbBuffMax)
			{
			/* get text from the file */
			if (ReadFile(cbBuffMax, DataBuff) != cbBuffMax)
				{
				Error(READ_ERR_TEXT, fTrue);
				exit(1);
				}


			if (fSubstCh)
				{
				for (iBuff = 0; iBuff < cbBuffMax; iBuff++)
					if ((DataBuff[iBuff] < 0x20) || (DataBuff[iBuff] > 0x7f))
						DataBuff[iBuff] = '.';
				}

			iBuff = 0;
			}

		if ((cLines % 8) == 0)
			switch (Radix)
				{
			case OCTAL:
				printf("              0-------1-------2-------3-------4-------5-------6-------7-------\n");
				break;

			case DECIMAL:
				printf("             0---------1---------2---------3---------4---------5---------6---\n");
				break;

			case HEX:
				printf("           0-------|-------1-------|-------2-------|-------3-------|-------\n");
				break;
				}

		memcpy(szOut, &DataBuff[iBuff], 64);

		if ((cbDumped + 64) > Fib.ccpText)
			szOut[Fib.ccpText - cbDumped] = 0;
		else
			szOut[64] = 0;

		switch (Radix)
			{
		case OCTAL:
			printf("%11.11lo ", cbDumped + Fib.fcMin);
			break;

		case DECIMAL:
			printf("%10.10ld ", cbDumped + Fib.fcMin);
			break;

		case HEX:
			printf("%8.8lx ", cbDumped + Fib.fcMin);
			break;
			}
		printf("  %s\n", szOut);

		iBuff += 64;
		cbDumped += 64;
		}

	if (fDumpTextLong)
		{
		printf("\n\n");

		/* seek to the start of the text area of the file */
		SeekFile(Fib.fcMin, SEEK_ERR_TEXT);

		fcCur = Fib.fcMin;

		while ((fcCur + Radix) < Fib.fcMac)
			{
			PrintLineTD(fcCur, Radix);
			fcCur += Radix;
			}

		PrintLineTD(fcCur, Fib.fcMac - fcCur);
		}
}



DumpStshf()
	/* dump style sheet */
{
	int istcp, stcpMac;
	int cstcStd;
	int NamebCur, ChpxbCur, PapxbCur;
	int NamebMax, ChpxbMax, PapxbMax;
	unsigned char st[256];
	char Chpx[sizeof(struct CHP)];
	struct PAPX Papx;
	int cbstshfMac = 0;
	int cbPapx;
	int cbChpx;
	int stcNext;
	int stcBase;

	printf("\n\n\n***************  STYLE SHEET DATA  ***************\n");

	LinePos = 0;

	if (Fib.cbStshf == 0)
		return;

	/* seek to the Stshf structure from the file */
	SeekFile(Fib.fcStshf, SEEK_ERR_STSHF);
	GetBytes(&cstcStd, sizeof(int), READ_ERR_STSHF);
	printInt ("cstcStd          = ", cstcStd, 1, 1, fTrue);
	cbstshfMac += sizeof(int);
	if (cbstshfMac > Fib.cbStshf)
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

	GetBytes(&NamebMax, sizeof(int), READ_ERR_STSHF);
	printInt ("sttbName.bMax    = ", NamebMax, 1, 1, fTrue);
	cbstshfMac += sizeof(int);
	if (cbstshfMac > Fib.cbStshf)
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

	for (NamebCur = 2; NamebCur < NamebMax;)
		{
		GetSt(st, READ_ERR_STSHF);

		if (st[0] == 255)
			{
			cbstshfMac++;
			NamebCur++;
			}
		else
			{
			cbstshfMac += st[0] + 1;
			NamebCur += st[0] + 1;
			}

		if (cbstshfMac > Fib.cbStshf)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}

		if (NamebCur > NamebMax)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}

		LinePos = 2;
		printf("\n");
		printst("sttbName.st      = ", st, 1, 0);
		}
	printf("\n\n");
	LinePos = 0;
	if ((NamebCur != NamebMax) && (NamebMax > 2))
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}


	GetBytes(&ChpxbMax, sizeof(int), READ_ERR_STSHF);
	printInt ("sttbChpx.bMax    = ", ChpxbMax, 1, 0, fTrue);
	cbstshfMac += sizeof(int);
	if (cbstshfMac > Fib.cbStshf)
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

	for (ChpxbCur = 2; ChpxbCur < ChpxbMax;)
		{
		printf("\n  ");
		LinePos = 2;

		cbChpx = GetByte(READ_ERR_STSHF);
		if (cbChpx == 255)
			{
			ChpxbCur++;
			cbstshfMac++;
			}
		else
			{
			ChpxbCur += cbChpx + 1;
			cbstshfMac += cbChpx + 1;
			}
		if (cbstshfMac > Fib.cbStshf)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}
		if (ChpxbCur > ChpxbMax)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}

		if ((cbChpx != 0) && (cbChpx != 255))
			{
			GetBytes(Chpx, cbChpx, READ_ERR_STSHF);
			PrintCHPX(Chpx, cbChpx);
			}
		}
	printf("\n\n");
	LinePos = 0;

	if ((ChpxbCur != ChpxbMax) && (ChpxbMax > 2))
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}


	GetBytes(&PapxbMax, sizeof(int), READ_ERR_STSHF);
	printInt ("sttbPapx.bMax    = ", PapxbMax, 1, 1, fFalse);
	cbstshfMac += sizeof(int);
	if (cbstshfMac > Fib.cbStshf)
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

	for (PapxbCur = 2; PapxbCur < PapxbMax;)
		{
		cbPapx = (GetByte(READ_ERR_STSHF) % 256);
		Papx.cw = cbPapx;    /* cw is used as a cb in this case; (is this even used?) */
		if (cbPapx == 255)
			{
			PapxbCur++;
			cbstshfMac++;
			}
		else
			{
			PapxbCur += cbPapx + 1;
			cbstshfMac += cbPapx + 1;
			}
		if (cbstshfMac > Fib.cbStshf)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}
		if (PapxbCur > PapxbMax)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}

		if ((cbPapx != 0) && (cbPapx != 255))
			{
			GetBytes(&Papx.stc, cbPapx, READ_ERR_STSHF);
			PrintPAPX(&Papx, fTrue);
			}

		if (LinePos != 0)
			{
			printf("\n");
			LinePos = 0;
			}
		}
	printf("\n");
	LinePos = 0;
	if ((PapxbCur != PapxbMax) && (PapxbMax > 2))
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

	GetBytes(&stcpMac, sizeof(int), READ_ERR_STSHF);
	printInt ("plestcp.stcpMac = ", stcpMac, 1, 1, fFalse);
	cbstshfMac += sizeof(int);
	if (cbstshfMac > Fib.cbStshf)
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}
	for (istcp = 0; istcp < stcpMac; istcp++)
		{
		stcNext = GetByte(READ_ERR_STSHF);
		stcBase = GetByte(READ_ERR_STSHF);
		cbstshfMac += 2;
		if (cbstshfMac > Fib.cbStshf)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}
		printInt("plestcp.dnstcp.stcNext = ",stcNext, 1, 1, fTrue);
		printInt("plestcp.dnstcp.stcBase = ",stcBase, 1, 1, fTrue);
		printf("\n");
		}
	if (cbstshfMac != Fib.cbStshf)
		Error(BAD_STTB_LENGTH, fFalse);
}



DumpSttbfAssoc()
{
	int NamebCur;
	int NamebMax;
	unsigned char st[256];

	printf("\n\n\n*************  ASSOCIATED NAMES DATA  ************\n");

	LinePos = 0;

	if (Fib.cbSttbfAssoc == 0)
		return;

	/* seek to the SttbfAssoc structure from the file */
	SeekFile(Fib.fcSttbfAssoc, SEEK_ERR_ASSOC);

	GetBytes(&NamebMax, sizeof(int), READ_ERR_ASSOC);
	printInt ("sttbfAssoc.bMax     = ", NamebMax, 1, 1, fTrue);
	if (Fib.cbSttbfAssoc != NamebMax)
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

	for (NamebCur = 2; NamebCur < NamebMax;)
		{
		GetSt(st, READ_ERR_ASSOC);

		if (st[0] == 255)
			NamebCur++;
		else
			NamebCur += st[0] + 1;

		if (NamebCur > NamebMax)
			{
			Error(BAD_STTB_LENGTH, fFalse);
			return;
			}

		LinePos = 2;
		printf("\n");
		printst("sttbfAssoc.st    = ", st, 1, 0);
		}

	printf("\n");
	LinePos = 0;
	if ((NamebCur != NamebMax) && (NamebMax > 2))
		{
		Error(BAD_STTB_LENGTH, fFalse);
		return;
		}

}


DumpPlcffnd()
	/* dump footnote tables */
{
	printf("\n\n\n****************  FOOTNOTE TABLES  ***************\n");
	printf("****************  NOT IMPLEMENTED  ***************\n\n");

}


DumpPlcfand()
	/* dump annotation tables */
{
	printf("\n\n\n****************  ANNOTATION TABLES  ***************\n");
	printf("****************  NOT IMPLEMENTED  ***************\n\n");

}


DumpPlcffld()
	/* dump field tables */
{
	printf("\n\n\n****************  FIELD TABLES  ***************\n");
	printf("****************  NOT IMPLEMENTED  ***************\n\n");

}


DumpPlcfsed()
	/* dump section table */
{
	FC fc;
	FC fcPlcfsedrgfc;
	FC fcPlcfsedrgsed;
	int csed, ised;
	unsigned char sprm;
	unsigned char cbSprmArg;
	struct SED sed;
	unsigned char SprmArg;
	char cbsepx;

	printf("\n\n\n*****************  SECTION TABLE  ****************\n");

	LinePos = 0;

	if (Fib.cbPlcfsed == 0)
		return;

	if (((Fib.cbPlcfsed - sizeof(FC)) % (sizeof(FC) + cbSED) != 0))
		{
		Error(BAD_PLC_LENGTH, fFalse);
		return;
		}

	csed = (Fib.cbPlcfsed - sizeof(FC)) / (sizeof(FC) + cbSED);

	fcPlcfsedrgfc = Fib.fcPlcfsed;
	fcPlcfsedrgsed = Fib.fcPlcfsed + ((csed + 1) * sizeof(FC));

	for (ised = 0; ised < csed; ised++)
		{
		SeekFile(fcPlcfsedrgfc, SEEK_ERR_SED);
		GetBytes(&fc, sizeof(FC), READ_ERR_SED);
		printLong("Run Start = ", fc, 0, 0, fTrue);
		fcPlcfsedrgfc += sizeof(FC);
		GetBytes(&fc, sizeof(FC), READ_ERR_SED);
		printLong("        Run End = ", fc - 1, 1, 1, fTrue);

		SeekFile(fcPlcfsedrgsed, SEEK_ERR_SED);
		GetBytes(&sed, cbSED, READ_ERR_SED);
		fcPlcfsedrgsed += cbSED;

		printBool("sed.fSpare       = ", sed.fSpare, 0, 0, fFalse);
		printBool("sed.fUnk         = ", sed.fUnk, 1, 1, fFalse);
		printInt ("sed.fn           = ", sed.fn, 1, 1, fFalse);

		if (sed.fcSepx == 0xffffffff)
			printf("  Default Section Property.");
		else
			{
			SeekFile(sed.fcSepx, SEEK_ERR_SED);
			GetBytes(&cbsepx, 1, READ_ERR_SED);

			while (cbsepx > 0)
				{
				GetBytes(&sprm, 1, READ_ERR_SED);
				cbsepx--;
				if (sprm >= sprmMax)
					{
					Error(BAD_SPRM_CODE, fFalse);
					printInt("Sprm code = ", sprm, 1, 1, fFalse);
					break;
					}

				printf("    %s    ", SprmDef[sprm].pSprmName);
				cbSprmArg = SprmDef[sprm].cbSprmArg;
				if (cbSprmArg == 0)
					{
					GetBytes(&cbSprmArg, 1, READ_ERR_SED);
					if (--cbsepx < 0)
						{
						Error(BAD_SPRM_LENGTH, fFalse);
						printInt("Sprm length = ", cbSprmArg, 2, 2, fFalse);
						break;
						}

					if (cbSprmArg == 0)
						{
						Error(BAD_SPRM_LENGTH, fFalse);
						printInt("Sprm length = ", cbSprmArg, 2, 2, fFalse);
						break;
						}
					}
				else
					cbSprmArg--;

				if ((cbsepx -= cbSprmArg) < 0)
					{
					Error(BAD_SPRM_LENGTH, fFalse);
					printInt("Sprm length = ", cbSprmArg, 2, 2, fFalse);
					break;
					}

				while (cbSprmArg-- > 0)
					{
					GetBytes(&SprmArg, 1, READ_ERR_SED);
					switch (Radix)
						{
					case OCTAL:
						printf("%o  ", SprmArg);
						break;

					case DECIMAL:
						printf("%d  ", SprmArg);
						break;

					case HEX:
						printf("%x  ", SprmArg);
						break;

						}
					}
				printf("\n");
				}
			}
		printf("\n");
		}
}


DumpPlcfPgd()
	/* dump page table */
{
	CP cp;
	int cpgd, ipgd;
	struct PGD pgd;


	printf("\n\n\n******************  PAGE TABLE  ******************\n");

	LinePos = 0;

	if (Fib.cbPlcfpgd == 0)
		return;

	if (((Fib.cbPlcfpgd - sizeof(CP)) % (sizeof(CP) + cbPGD) != 0))
		{
		Error(BAD_PLC_LENGTH, fFalse);
		return;
		}

	cpgd = (Fib.cbPlcfpgd - sizeof(CP)) / (sizeof(CP) + cbPCD);

	/* seek to the page table in the file */
	SeekFile(Fib.fcPlcfpgd, SEEK_ERR_PGD);

	for (ipgd = 0; ipgd < (cpgd + 1); ipgd++)
		{
		GetBytes(&cp, sizeof(CP), READ_ERR_PGD);
		printLong("Plcfpgd.cp       = ", cp, 1, 1, fTrue);
		}

	for (ipgd = 0; ipgd < cpgd; ipgd++)
		{
		GetBytes(&pgd, cbPGD, READ_ERR_PGD);
		printBool("  pgd.fGhost         = ", pgd.U1.S1.fGhost, 1, 0, fFalse);

		printBool("  pgd.fContinue      = ", pgd.U1.S2.fContinue, 1, 0, fFalse);
		printBool("  pgd.fUnk           = ", pgd.U1.S2.fUnk, 1, 0, fFalse);
		printBool("  pgd.fRight         = ", pgd.U1.S2.fRight, 1, 0, fFalse);
		printBool("  pgd.fPgnRestart    = ", pgd.U1.S2.fPgnRestart, 1, 0, fFalse);
		printBool("  pgd.fEmptyPage     = ", pgd.U1.S2.fEmptyPage, 1, 0, fFalse);
		printBool("  pgd.fAllFtn        = ", pgd.U1.S2.fAllFtn, 1, 0, fFalse);
		printInt ("  pgd.bkc            = ", pgd.U1.S2.bkc, 1, 0, fFalse);

		printUns ("  pgd.lnn            = ", pgd.lnn, 1, 0, fFalse);
		printInt ("  pgd.cl             = ", pgd.cl, 1, 0, fFalse);
		printUns ("  pgd.pgn            = ", pgd.pgn, 1, 0, fFalse);
		printInt ("  pgd.dcpDepend      = ", pgd.dcpDepend, 1, 0, fFalse);
/*
		printBool("  pgd.fUnk1          = ", pgd.U1.S1.fUnk1, 1, 0, fFalse);
		printBool("  pgd.fUnk2          = ", pgd.U1.S1.fUnk2, 1, 0, fFalse);
		printInt ("  pgd.fUnks          = ", pgd.U1.S2.fUnks, 1, 0, fFalse);
		printBool("  pgd.fPending       = ", pgd.U1.S2.fPending, 1, 0, fFalse);
		printBool("  pgd.fRight         = ", pgd.U1.S2.fRight, 1, 0, fFalse);
		printBool("  pgd.fPgnRestart    = ", pgd.U1.S2.fPgnRestart, 1, 0, fFalse);
		printInt ("  pgd.bkc            = ", pgd.U1.S2.bkc, 1, 0, fFalse);
		printUns ("  pgd.lnn            = ", pgd.lnn, 1, 0, fFalse);
		printInt ("  pgd.cl             = ", pgd.cl, 1, 0, fFalse);
		printInt ("  pgd.pgn            = ", pgd.pgn, 1, 0, fFalse);
*/
		}
}


Dumpglsy()
	/* dump glossary tables */
{
	printf("\n\n\n****************  GLOSSARY TABLES  ***************\n");
	printf("****************  NOT IMPLEMENTED  ***************\n\n");
}


DumpPlcfhdd()
	/* dump header table */
{
	printf("\n\n\n*****************  HEADER TABLE  *****************\n");
	printf("****************  NOT IMPLEMENTED  ***************\n\n");
}


DumpChpx()
	/* dump character properties */
{
	int cfkp;
	int ifkp;
	FC fcpn;
	PN pn;
	int crun, irun;
	unsigned offsetRun;
	struct FKP *pfkp;


	printf("\n\n\n*************  CHARACTER PROPERTIES  *************\n");

	LinePos = 0;

	if (Fib.cbPlcfbteChpx == 0)
		return;

	cfkp = (Fib.cbPlcfbteChpx - sizeof(FC)) / (sizeof(FC) + sizeof(PN));

	/* get the file position of the start of the list of Paragraph Numbers
		* of FKP structures
		*/
	fcpn = Fib.fcPlcfbteChpx + ((cfkp + 1) * sizeof(FC));

	for (ifkp = 0; ifkp < cfkp; ifkp++)
		{

		/* seek to get the next FKP from the file */
		SeekFile(fcpn, SEEK_ERR_CHP);

		/* get Paragraph Number of next FKP to dump */
		if (ReadFile(sizeof(PN), &pn) != sizeof(PN))
			{
			Error(READ_ERR_CHP, fTrue);
			exit(1);
			}

		/* seek to the next FKP in the file */
		SeekFile((FC)pn * (FC)cbSector, SEEK_ERR_CHP);

		/* get next FKP from the file */
		if (ReadFile(cbSector, DataBuff) != cbSector)
			{
			Error(READ_ERR_CHP, fTrue);
			exit(1);
			}

		pfkp = (struct FKP *)DataBuff;

		/* get count of runs in this FKP */
		crun = DataBuff[cbSector - 1];

		for (irun = 0; irun < crun; irun++)
			{
			if (fVerbose)
				{
				printLong("Run Start = ", pfkp->rgfc[irun], 0, 0, fTrue);
				printLong("        Run End = ", pfkp->rgfc[irun + 1] - 1, 1, 1, fTrue);
				}
			else
				{
				LinePos = 2;
				printLong("", pfkp->rgfc[irun], 0, 0, fTrue);
				printLong("-  ", pfkp->rgfc[irun + 1] - 1, 1, 1, fTrue);
				}

			offsetRun = 2 * DataBuff[((crun + 1) * sizeof(FC)) + irun];
			/* 512 byte page requires doubled offsets */

			if (offsetRun == 0)
				printf("  Default Character Attributes.");
			else
				{
				if (fVerbose)
					printInt("Run exception description length = ", DataBuff[offsetRun], 1, 1, fFalse);
				PrintCHPX(&DataBuff[offsetRun + 1], DataBuff[offsetRun]);
				}
			printf("\n\n");
			LinePos = 0;
			}
		fcpn += sizeof(PN);     /* point to next pn */
		}
}


DumpPlcfbtePapx()
{
	int  ifkp;
	int  cfkp;
	FC   fcfc;
	FC   fcpn;
	FC   fc;
	PN   pn;


	printf("\n\n\n ************* PARAGRAPH PROPERTIES PLCBTE TABLE ************* \n");

	LinePos = 0;

	printInt ("Fib.cbPlcfbtePapx     = ", Fib.cbPlcfbtePapx, 1, 1, fTrue);

	if (Fib.cbPlcfbtePapx == 0)
		return;

	cfkp = (Fib.cbPlcfbtePapx - sizeof(FC)) / (sizeof(FC) + sizeof(PN));

	fcfc = Fib.fcPlcfbtePapx;

	fcpn = Fib.fcPlcfbtePapx + ((cfkp + 1) * sizeof(FC));

	for (ifkp = 0; ifkp < cfkp; ifkp++)
		{
		SeekFile(fcfc, SEEK_ERR_PLCBTE);
		if (ReadFile(sizeof(FC), &fc) != sizeof(FC))
			{
			Error(READ_ERR_PLCBTE, fTrue);
			exit(1);
			}
		printLong("fc             = ", fc, 0, 0, fTrue);

		SeekFile(fcpn, SEEK_ERR_PLCBTE);
		if (ReadFile(sizeof(PN), &pn) != sizeof(PN))
			{
			Error(READ_ERR_PLCBTE,fTrue);
			exit(1);
			}
		printInt ("pn             = ", pn, 1, 1, fTrue);

		fcfc += sizeof(FC);
		fcpn += sizeof(PN);

		}

	SeekFile(fcfc, SEEK_ERR_PLCBTE);
	if (ReadFile(sizeof(FC), &fc) != sizeof(FC))
		{
		Error(READ_ERR_PLCBTE, fTrue);
		exit(1);
		}
	printLong("fc             = ", fc, 1, 1, fTrue);
}


DumpPapx()
{
	int  ifkp;
	int  cfkp;
	FC   fcpn;
	FC   fc;
	PN   pn;


	printf("\n\n\n ************* PARAGRAPH PROPERTIES FKP PAGES ************* \n");

	LinePos = 0;

/*    printInt ("Fib.cbPlcfbtePapx     = ", Fib.cbPlcfbtePapx, 1, 1, fTrue); */

	if (Fib.cbPlcfbtePapx == 0)
		return;

	cfkp = (Fib.cbPlcfbtePapx - sizeof(FC)) / (sizeof(FC) + sizeof(PN));

	fcpn = Fib.fcPlcfbtePapx + ((cfkp + 1) * sizeof(FC));

	for (ifkp = 0; ifkp < cfkp; ifkp++)
		{
		SeekFile(fcpn, SEEK_ERR_PLCBTE);
		if (ReadFile(sizeof(PN), &pn) != sizeof(PN))
			{
			Error(READ_ERR_PLCBTE,fTrue);
			exit(1);
			}
/*        printInt ("pn             = ", pn, 1, 1, fTrue); */

		DumpPapxfkp(pn);

		fcpn += sizeof(PN);

		}
}



DumpPapxfkp(pnfkp)
PN    pnfkp;
	/* This is an expanded routine to dump paragraph properties.  It dumps */
	/* an entire Papx fkp page.  It is passed the page number of the fkp.  */
{
	int  crun;
	int  irun;
	int  offset;
	struct FKP *pfkp;


	printInt ("PAGE NUMBER ", pnfkp, 1, 1, fTrue);

	SeekFile((FC)pnfkp * (FC)cbSector, SEEK_ERR_PAPX);
	if (ReadFile(cbSector, DataBuff) != cbSector)
		{
		Error(READ_ERR_PAPX, fTrue);
		exit(1);
		}

	pfkp = (struct FKP *)DataBuff;
	crun = DataBuff[cbSector-1];
	printInt ("    crun        = ", crun, 1, 1, fTrue);

	for (irun = 0; irun < crun; irun++)
		{
		offset = (Fib.nFib >= 25 ? 2 : 1) * 
				DataBuff[((crun + 1) * sizeof(FC)) + irun];
		printf("\n");
		LinePos = 2;
		printLong("", pfkp->rgfc[irun], 0, 0, fTrue);
		printLong("-  ", pfkp->rgfc[irun + 1] - 1, 1, 1, fTrue);
		printInt ("offset  = ", offset, 1, 1, fTrue);
		if (offset == 0)
			printf("(Default Paragraph Attributes)");
		else
			{
/*              printInt ("cb       = ", DataBuff[offset], 1, 1, fTrue); */
			PrintPAPX(&DataBuff[offset], fFalse);
			printf("\n");
			}
		}

}


Dumpbkmk()
	/* dump bookmark properties */
{
	printf("\n\n\n**************  BOOKMARK PROPERTIES  *************\n");
	printf("****************  NOT IMPLEMENTED  ***************\n\n");
}


DumpClx()
	/* dump complex part */
{
	int cb;         /* The number of bytes still to be read */
	int cbRead;     /* cb to read for a given section */
	int cPcd;
	int i;
	int iPcdFirst;
	CP cp;
	struct PCD pcd;

	printf("\n\n\n*****************  COMPLEX PART  *****************\n");

	cb = Fib.cbClx;

	SeekFile((FC)Fib.fcClx, SEEK_ERR_CLX);
	if (ReadFile(1, DataBuff) != 1)
		{
		Error(READ_ERR_CLX, fTrue);
		exit(1);
		}
	cb -= 1;

	while (DataBuff[0] == clxtPrc)
		{
		printInt ("  clx.clxt           = ", clxtPrc, 1, 1, fTrue);
		if (ReadFile(2, DataBuff) != 2)     /* cb to read */
			{
			Error(READ_ERR_CLX, fTrue);
			exit(1);
			}
		cbRead = *(int *)DataBuff;
		printInt ("  clx.cbgrpprl       = ", cbRead, 1, 1, fTrue);
		if (ReadFile(cbRead, DataBuff) != cbRead)
			{
			Error(READ_ERR_CLX, fTrue);
			exit(1);
			}
		cb -= (cbRead + 2);
		printGrpprl (DataBuff, cbRead);
		if (ReadFile(1, DataBuff) != 1)        /* Read the next clxt */
			{
			Error(READ_ERR_CLX, fTrue);
			exit(1);
			}
		cb -= 1;
		}
	if (DataBuff[0] != clxtPlcpcd)
		{
		printf ("\n * Can't find the start of the plcpcd *\n");
		Error(READ_ERR_CLX, fTrue);
		exit(1);
		}
	printf("\nThe PlcPcd structure:\n");
	printInt ("  clx.clxt           = ", clxtPlcpcd, 1, 1, fTrue);
	if (ReadFile(2, DataBuff) != 2)     /* cb to read */
		{
		Error(READ_ERR_CLX, fTrue);
		exit(1);
		}
	cbRead = *(int *)DataBuff;
	printInt ("  clx.cbplcpcd       = ", cbRead, 2, 2, fTrue);
	if (ReadFile(cbRead, DataBuff) != cbRead)
		{
		Error(READ_ERR_CLX, fTrue);
		exit(1);
		}
	cb -= (cbRead + 2);

	cPcd = (cbRead - sizeof(CP)) / (sizeof(CP) + sizeof(struct PCD));

	iPcdFirst = (cPcd + 1) * sizeof(CP);
	for (i = 0 ; i < (cPcd + 1) ; i++)
		{
		cp = * (long *) &DataBuff[i * sizeof(CP)];
		printInt ("  clx.cp              = ", cp, 1, 1, fTrue);
		}
	printf("\n");
	for (i = 0 ; i < cPcd ; i++)
		{
		pcd = *(struct PCD *)&DataBuff[iPcdFirst + (i * sizeof(struct PCD))];
		printBool ("  clx.pcd.fNoParaLast = ", pcd.fNoParaLast, 0, 0, fTrue);
		printBool ("  clx.pcd.fPaphNil    = ", pcd.fPaphNil, 1, 1, fTrue);
		printInt  ("  clx.pcd.fn          = ", pcd.fn, 0, 0, fTrue);
		printLong ("  clx.pcd.fc          = ", (long)pcd.fc, 0, 0, fTrue);
		printInt  ("  clx.pcd.prm         = ", (int)pcd.prm, 2, 2, fTrue);
		}
/*    DumpRawDataTable(Fib.cbClx);   REVIEW: remove this */
}


DumpDop()
	/* dump document properties */
{
	struct DOP dop;
	printf("\n\n\n****************  DOCUMENT PROPERTIES  ***************\n");

	if (Fib.cbDop == 0)
		return;

	SeekFile (Fib.fcDop, SEEK_ERR_DOP);
	GetBytes (&dop,
			(Fib.cbDop > sizeof (struct DOP) ? sizeof (struct DOP) : Fib.cbDop)
			, READ_ERR_DOP);

	printBool("dop.fFacingPages     = ", dop.fFacingPages, 1, 1, fFalse);
	printBool("dop.fWidowControl    = ", dop.fWidowControl, 1, 1, fFalse);
	printInt ("dop.fpc              = ", dop.fpc, 1, 1, fFalse);
	printBool("dop.fWide            = ", dop.fWide, 1, 1, fFalse);
	printInt ("dop.grpfIhdt         = ", dop.grpfIhdt, 1, 1, fFalse);

	printBool("dop.fFtnRestart      = ", dop.fFtnRestart, 1, 1, fFalse);
	printInt ("dop.nFtn             = ", dop.nFtn, 1, 1, fFalse);

	printInt ("dop.irmBar           = ", dop.irmBar, 1, 1, fFalse);
	printInt ("dop.irmProps         = ", dop.irmProps, 1, 1, fFalse);
	printBool("dop.fRevMarking      = ", dop.fRevMarking, 1, 1, fFalse);

	printBool("dop.fBackup          = ", dop.fBackup, 1, 1, fFalse);
	printBool("dop.fExactCWords     = ", dop.fExactCWords, 1, 1, fFalse);
	printBool("dop.fPagHidden       = ", dop.fPagHidden, 1, 1, fFalse);
	printBool("dop.fPagResults      = ", dop.fPagResults, 1, 1, fFalse);
	printBool("dop.fLockAtn         = ", dop.fLockAtn, 1, 1, fFalse);
	printBool("dop.fMirrorMargins   = ", dop.fMirrorMargins, 1, 1, fFalse);

	printInt ("dop.fSpares          = ", dop.fSpares, 1, 1, fFalse);

/* measurements */
	printUns ("dop.yaPage           = ", dop.yaPage, 1, 1, fFalse);
	printUns ("dop.xaPage           = ", dop.xaPage, 1, 1, fFalse);
	printInt ("dop.dyaTop           = ", dop.dyaTop, 1, 1, fFalse);
	printUns ("dop.dxaLeft          = ", dop.dxaLeft, 1, 1, fFalse);
	printInt ("dop.dyaBottom        = ", dop.dyaBottom, 1, 1, fFalse);
	printUns ("dop.dxaRight         = ", dop.dxaRight, 1, 1, fFalse);
	printUns ("dop.dxaGutter        = ", dop.dxaGutter, 1, 1, fFalse);
	printUns ("dop.dxaTab           = ", dop.dxaTab, 1, 1, fFalse);
	printUns ("dop.wSpare           = ", dop.wSpare, 1, 1, fFalse);
	printUns ("dop.dxaHotZ          = ", dop.dxaHotZ, 1, 1, fFalse);
	printUns ("dop.rgwSpare[1]      = ", dop.rgwSpare[1], 0, 0, fFalse);
	printUns ("dop.rgwSpare[2]      = ", dop.rgwSpare[2], 1, 1, fFalse);

	printLong("dop.dttmCreated      = ", dop.dttmCreated, 1, 1, fFalse);
	printLong("dop.dttmRevised      = ", dop.dttmRevised, 1, 1, fFalse);
	printLong("dop.dttmLastPrint    = ", dop.dttmLastPrint, 1, 1, fFalse);
	printInt ("dop.nRevision        = ", dop.nRevision, 1, 1, fFalse);
	printLong("dop.tmEdited         = ", dop.tmEdited, 1, 1, fFalse);
	printLong("dop.cWords           = ", dop.cWords, 1, 1, fFalse);
	printLong("dop.cCh              = ", dop.cCh, 1, 1, fFalse);
	printInt ("dop.cPg              = ", dop.cPg, 2, 2, fFalse);
	printInt ("dop.rgwSpareDocSum[0]= ", dop.rgwSpareDocSum[0], 0, 0, fFalse);
	printInt ("dop.rgwSpareDocSum[1]= ", dop.rgwSpareDocSum[1], 0, 0, fFalse);

}







Error(ErrorMessNo, fSysError)
int ErrorMessNo;
BOOL fSysError;
{
	if (fSysError)
		perror(pErrorMess[ErrorMessNo]);
	else
		printf("\n\n%s\n", pErrorMess[ErrorMessNo]);
}




OpenFile(pszFn)
/* open the given file for reading */
unsigned char *pszFn;
{
	if ((pFile = fopen(pszFn, "rb")) == NULL)
		{
		Error(FILE_NOT_FND, fTrue);
		exit(1);
		}
}


SeekFile(SeekPos, iError)
/* seek to a file position */
long SeekPos;
{
	cbBuffMac = 0;

	if (fseek(pFile, SeekPos, SEEK_SET) == 0)
		return;
	else
		{
		Error(iError, fTrue);
		exit(1);
		}
}



ReadFile(cbRead, pBuff)
/* read cbRead bytes into pBuff */
int cbRead;
unsigned char *pBuff;
{
	return(fread(pBuff, 1, cbRead, pFile));
}



GetByte(iError)
int iError;
{
	if ((iDataBuff == cbBuffMac) || (cbBuffMac == 0))
		{
		cbBuffMac = ReadFile(cbBuffMax, DataBuff);
		iDataBuff = 0;

		if (cbBuffMac == 0)
			{
			Error(iError, fTrue);
			exit(1);
			}
		}

	return(DataBuff[iDataBuff++]);
}



GetBytes(rgDst, cbDst, iError)
char rgDst[];
int cbDst;
int iError;
{
	int iDst;

	for (iDst = 0; iDst < cbDst; iDst++)
		rgDst[iDst] = GetByte(iError);
}


GetSt(pst, iError)
unsigned char *pst;
int iError;
{
	*pst = GetByte(iError);
	if (*pst == 255)
		return;
	GetBytes(pst + 1, *pst, iError);
}



BadCmdLine(ErrorCode)
/* something wrong with the command line - print an error message
* and then print the usage for the user
*/
int ErrorCode;
{
	printf("%s\n\n", pErrorMess[ErrorCode]);
	printf("Usage:\n");
	printf("    DNATFILE [/d/h/o/p/s/t/v] Source_File_Name\n\n");
	printf("    Flags:\n");
	printf("        /d - decimal notation\n");
	printf("        /h - hexadecimal notation\n");
	printf("        /o - octal notation\n");
	printf("        /p - dump pictures\n");
	printf("        /s - substitute all text chars < 0x20 or > 0x7f with a '.' in the dump\n");
	printf("        /t - dump the text part of the file in the huge mode\n");
	printf("        /n - dump header start only\n");
	printf("        /v - verbose mode\n\n\n");
	exit(1);
}



CheckLineWrap(cb)
int cb;
{
	if ((LinePos + cb) > 72)
		{
		printf("\n      ");
		LinePos = 6;
		}
}


printst(pszPrompt, pstOut, cLFv, cLF)
unsigned char *pszPrompt;
unsigned char *pstOut;
int cLFv;
int cLF;
{
	unsigned char szOut[255];
	unsigned char *pszOut;
	unsigned char szTemp[255];

	if (*pstOut != 255)
		{
		strncpy(szTemp, pstOut + 1, *pstOut);
		szTemp[*pstOut] = 0;
		}

	if (fVerbose)
		{
		if (*pszPrompt != 0)
			printf("%s", pszPrompt);

		if ((*pstOut == 0) || (*pstOut == 255))
			printf("***** NULL STRING *****");
		else
			printf("%s", szTemp);

		while (cLFv-- > 0)
			printf("\n");
		}
	else
		{
		strcpy(szOut, "  ");

		if (*pszPrompt != 0)
			{
			strcat(szOut, pszPrompt);

			if ((pszOut = strchr(szOut, '=')) != NULL)
				{
				while (*(--pszOut) == ' ')
					;
				*(pszOut + 1) = 0;
				strcat(szOut, " = ");
				}
			}

		if ((*pstOut == 0) || (*pstOut == 255))
			strcat(szOut, "***** NULL STRING *****");
		else
			strcat(szOut, szTemp);

		CheckLineWrap(strlen(szOut));
		printf("%s", szOut);
		LinePos += strlen(szOut);

		while (cLF-- > 0)
			{
			printf("\n");
			LinePos = 0;
			}
		}

}


printBool(psz, fvalue, cLFv, cLF, fForcePrint)
unsigned char *psz;
BOOL fvalue;
int cLFv;
int cLF;
BOOL fForcePrint;
{
	unsigned char szOut[60];
	unsigned char *pszOut;

	if (fVerbose)
		{
		if (*psz != 0)
			printf("%s", psz);

		if (fvalue)
			printf("fTrue      ");
		else
			printf("fFalse     ");

		while (cLFv-- > 0)
			printf("\n");
		}
	else
		{
		if (fvalue || fForcePrint)
			{
			strcpy(szOut, "  ");
			strcat(szOut, psz);

			if ((pszOut = strchr(szOut, '=')) != NULL)
				{
				while (*(--pszOut) == ' ')
					;
				*(pszOut + 1) = 0;
				}

			CheckLineWrap(strlen(szOut));
			printf("%s", szOut);
			LinePos += strlen(szOut);
			}
		else if (LinePos == 0)
			cLF--;

		while (cLF-- > 0)
			{
			printf("\n");
			LinePos = 0;
			}
		}
}


printInt(psz, value, cLFv, cLF, fForcePrint)
unsigned char *psz;
int value;
int cLFv;
int cLF;
BOOL fForcePrint;
{
	unsigned char szOut[60];
	unsigned char szNum[60];
	unsigned char *pszOut;

	if (fVerbose)
		{
		if (*psz != 0)
			printf("%s", psz);

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

		while (cLFv-- > 0)
			printf("\n");
		}
	else
		{
		if ((value != 0) || fForcePrint)
			{
			strcpy(szOut, "  ");

			if (*psz != 0)
				{
				strcat(szOut, psz);

				if ((pszOut = strchr(szOut, '=')) != NULL)
					{
					while (*(--pszOut) == ' ')
						;
					*(pszOut + 1) = 0;
					strcat(szOut, " = ");
					}
				}

			switch (Radix)
				{
			case OCTAL:
				sprintf(szNum, "%o", value);
				break;

			case DECIMAL:
				sprintf(szNum, "%d", value);
				break;

			case HEX:
				sprintf(szNum, "%x", value);
				break;
				}

			strcat(szOut, szNum);
			CheckLineWrap(strlen(szOut));
			printf("%s", szOut);
			LinePos += strlen(szOut);
			}
		else if (LinePos == 0)
			cLF--;

		while (cLF-- > 0)
			{
			printf("\n");
			LinePos = 0;
			}
		}
}


printUns(psz, value, cLFv, cLF, fForcePrint)
unsigned char *psz;
unsigned value;
int cLFv;
int cLF;
BOOL fForcePrint;
{
	unsigned char szOut[60];
	unsigned char szNum[60];
	unsigned char *pszOut;

	if (fVerbose)
		{
		if (*psz != 0)
			printf("%s", psz);

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

		while (cLFv-- > 0)
			printf("\n");
		}
	else
		{
		if ((value != 0) || fForcePrint)
			{
			strcpy(szOut, "  ");

			if (*psz != 0)
				{
				strcat(szOut, psz);

				if ((pszOut = strchr(szOut, '=')) != NULL)
					{
					while (*(--pszOut) == ' ')
						;
					*(pszOut + 1) = 0;
					strcat(szOut, " = ");
					}
				}

			switch (Radix)
				{
			case OCTAL:
				sprintf(szNum, "%o", value);
				break;

			case DECIMAL:
				sprintf(szNum, "%u", value);
				break;

			case HEX:
				sprintf(szNum, "%x", value);
				break;
				}

			strcat(szOut, szNum);
			CheckLineWrap(strlen(szOut));
			printf("%s", szOut);
			LinePos += strlen(szOut);
			}
		else if (LinePos == 0)
			cLF--;

		while (cLF-- > 0)
			{
			printf("\n");
			LinePos = 0;
			}
		}
}


printLong(psz, value, cLFv, cLF, fForcePrint)
unsigned char *psz;
unsigned long value;
int cLFv;
int cLF;
BOOL fForcePrint;
{
	unsigned char szOut[60];
	unsigned char szNum[60];
	unsigned char *pszOut;

	if (fVerbose)
		{
		if (*psz != 0)
			printf("%s", psz, fFalse);

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

		while (cLFv-- > 0)
			printf("\n");
		}
	else
		{
		if ((value != 0) || fForcePrint)
			{
			strcpy(szOut, "  ");

			if (*psz != 0)
				{
				strcat(szOut, psz);

				if ((pszOut = strchr(szOut, '=')) != NULL)
					{
					while (*(--pszOut) == ' ')
						;
					*(pszOut + 1) = 0;
					strcat(szOut, " = ");
					}
				}

			switch (Radix)
				{
			case OCTAL:
				sprintf(szNum, "%lo", value);
				break;

			case DECIMAL:
				sprintf(szNum, "%ld", value);
				break;

			case HEX:
				sprintf(szNum, "%lx", value);
				break;
				}

			strcat(szOut, szNum);
			CheckLineWrap(strlen(szOut));
			printf("%s", szOut);
			LinePos += strlen(szOut);
			}
		else if (LinePos == 0)
			cLF--;

		while (cLF-- > 0)
			{
			printf("\n");
			LinePos = 0;
			}
		}
}



PrintCHPX(pChp, cbChpx)
/* print out a part or a whole CHP structure */
struct CHP *pChp;
int cbChpx;
{
	struct CHP Chp;

	memset(&Chp, 0, cbCHP);
	memcpy(&Chp, pChp, cbChpx);

	printBool("fBold        = ", Chp.fBold, 1, 0, fFalse);
	printBool("fItalic      = ", Chp.fItalic, 1, 0, fFalse);
	printBool("fStrike      = ", Chp.fStrike, 1, 0, fFalse);
	printBool("fOutline     = ", Chp.fOutline, 1, 0, fFalse);

	printBool("fFldVanish   = ", Chp.fFldVanish, 1, 0, fFalse);

	printBool("fSmallCaps   = ", Chp.fSmallCaps, 1, 0, fFalse);
	printBool("fCaps        = ", Chp.fCaps, 1, 0, fFalse);
	printBool("fVanish      = ", Chp.fVanish, 1, 0, fFalse);
	if (--cbChpx == 0)
		return;

	printBool("fRMark       = ", Chp.fRMark, 1, 0, fFalse);
	printBool("fSpec        = ", Chp.fSpec, 1, 0, fFalse);
	printBool("fsIco        = ", Chp.fsIco, 1, 0, fFalse);
	printBool("fsFtc        = ", Chp.fsFtc, 1, 0, fFalse);
	printBool("fsHps        = ", Chp.fsHps, 1, 0, fFalse);
	printBool("fsKul        = ", Chp.fsKul, 1, 0, fFalse);
	printBool("fsPos        = ", Chp.fsPos, 1, 0, fFalse);
	printBool("fsSpace      = ", Chp.fsSpace, 1, 0, fFalse);
	if (--cbChpx == 0)
		return;

	printUns ("ftc          = ", Chp.ftc, 1, 0, fFalse);
	cbChpx -= 2;
	if (cbChpx <= 0)
		return;

	printInt ("hps          = ", Chp.hps, 1, 0, fFalse);
	if (--cbChpx == 0)
		return;

	printInt ("hpsPos       = ", Chp.hpsPos, 1, 0, fFalse);
	if (--cbChpx == 0)
		return;

	printInt ("qpsSpace     = ", Chp.qpsSpace, 1, 0, fFalse);
	printInt ("wSpare2      = ", Chp.wSpare2, 1, 0, fFalse);
	printInt ("ico          = ", Chp.ico, 1, 0, fFalse);
	printInt ("kul          = ", Chp.kul, 1, 0, fFalse);
	printBool("fSysVanish   = ", Chp.fSysVanish, 1, 0, fFalse);
	cbChpx -= 2;
	if (cbChpx <= 0)
		return;

	printLong("fcPic        = ", Chp.fcPic, 1, 0, fFalse);

	return;
}


PrintPAPX(pPapx, fByte)
/* print out a part or a whole PAPX structure */
struct PAPX *pPapx;
BOOL fByte;   /* true if pPapx->cw contains a byte count, not a word count */
{
	int cb;                /* minimum value should be 7 */

	cb = (fByte ? pPapx->cw : pPapx->cw * 2);

	if (fByte)
		printInt ("cb           = ", pPapx->cw, 1, 0, fTrue);
	else
		printInt ("cw           = ", pPapx->cw, 1, 0, fTrue);
	if (cb == 0)
		return;
	printInt ("         stc          = ", pPapx->stc, 1, 1, fTrue);
	if (--cb == 0)
		return;

	printBool("    phe.fSpare       = ", pPapx->phe.fSpare, 1, 0, fFalse);
	printBool("    phe.fUnk         = ", pPapx->phe.fUnk, 1, 1, fFalse);
	printBool("    phe.fDiffLines   = ", pPapx->phe.fDiffLines, 1, 0, fFalse);
	printInt ("    phe.dxaCol       = ", pPapx->phe.dxaCol, 1, 1, fFalse);
	printInt ("    phe.clMac        = ", pPapx->phe.U1.S1.clMac, 1, 0, fFalse);
	printInt ("    phe.dylLine      = ", pPapx->phe.U1.S1.dylLine, 1, 1, fFalse);
	printUns ("    phe.dyaHeight    = ", pPapx->phe.U1.dyaHeight, 1, 0, fFalse);
	printInt ("    phe.fStyleDirty  = ", pPapx->phe.U1.fStyleDirty, 1, 1, fFalse);
	cb -= 6;
	if (cb <= 0 || (cb == 1 && pPapx->grpprl[0] == sprmNoop))
		return;

	printGrpprl(pPapx->grpprl, cb);
}


printGrpprl(grpprl, cb)
unsigned char *grpprl;
int     cb;
{
	int igrpprl;
	int sprm;
	int cbSprmArg;

	printf("  Sprm List:\n");
	LinePos = 0;
	for (igrpprl = 0; igrpprl < cb;)
		{
		sprm = (int)grpprl[igrpprl++];
		if (sprm >= sprmMax)
			{
			Error(BAD_SPRM_CODE, fFalse);
			printInt("Sprm code = ", sprm, 2, 2, fFalse);
			break;
			}

		printf("    %s    ", SprmDef[sprm].pSprmName);
		cbSprmArg = SprmDef[sprm].cbSprmArg;
		if (cbSprmArg == 0)
			if (sprm == sprmNoop)
				cbSprmArg = 0;
			else
				cbSprmArg = grpprl[igrpprl++];  /* REVIEW: why? */
		else
			cbSprmArg--;

		if ((igrpprl + cbSprmArg) > cb)
			{
			Error(BAD_SPRM_LENGTH, fFalse);
			printInt("Sprm length = ", cbSprmArg, 2, 2, fFalse);
			break;
			}

		for (; cbSprmArg > 0; cbSprmArg--, igrpprl++)
			switch (Radix)
				{
			case OCTAL:
				printf("%o  ", grpprl[igrpprl]);
				break;

			case DECIMAL:
				printf("%d  ", grpprl[igrpprl]);
				break;

			case HEX:
				printf("%x  ", grpprl[igrpprl]);
				break;

				}
		printf("\n");
		LinePos = 0;
		}
}


PrintLineTD(fcCur, cbText)
FC fcCur;
int cbText;
{
	int i, iText;
	unsigned char rgText[18];

	if (cbText == 0)
		return;

	GetBytes(rgText, cbText, READ_ERR_TEXT);

	switch (Radix)
		{
	case OCTAL:
		printf("%11.11lo ", fcCur);
		break;

	case DECIMAL:
		printf("%10.10ld ", fcCur);
		break;

	case HEX:
		printf("%8.8lx ", fcCur);
		break;
		}

	for (iText = 0; iText < cbText; iText++)
		{
		switch (Radix)
			{
		case OCTAL:
			printf(" %3.3o", rgText[iText]);
			break;

		case DECIMAL:
			printf(" %3.3d", rgText[iText]);
			break;

		case HEX:
			printf(" %2.2X", rgText[iText]);
			break;
			}
		}

	for (iText = 0; iText < cbText; iText++)
		if ((rgText[iText] < 0x20) || (rgText[iText] > 0x7f))
			rgText[iText] = '.';

	for (i = Radix - cbText; i > 0; i--)
		if (Radix == HEX)
			printf("   ");
		else
			printf("    ");

	rgText[cbText] = 0;
	printf("  %s\n", rgText);
	LinePos = 0;
}


DumpRawDataTable(cb)
/* Dumps raw data in easy-to-read table format */
int     cb;
	/*   This function prints tables of ints.  It can  do    */
	/*   either 1-byte or 2-byte ints, adjusting all the     */
	/*   table spacing accordingly.  If fSmall, we want to   */
	/*   make a 1-byte int table, otherwise ints will be 2   */
	/*   bytes long.                                         */
	/* REVIEW: This is a debugging tool, need not be left in */
	/*         the final version of Dnatfile.                */
{
	int i;
	int cIntsOnLine;           /* # of ints to put on each line */
	int cInt;                  /* # of ints in table */

	cIntsOnLine = Radix;
	cInt = cb;

	/** index along the top ("  0  1  2  3  4...")   **/
	printf("\n          ");
	for (i = 0 ; i < cIntsOnLine ; i++)
		printSmallInt(i);
	/** Line of dashes separating index from data **/
	printf("\n  ----");
	for (i = 0 ; i < cIntsOnLine + 1 ; i++)
		printf("----");
	/** Now let's print the numbers **/
	for (i = 0 ; i < cInt ; i++)
		{
		if (i % cIntsOnLine == 0)      /* do we need a new line? */
			{
			printf("\n  ");
			printSmallInt(i);
			printf("  : ");
			}
		printSmallInt((int) DataBuff[i]);
		}
	printf("\n");
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


