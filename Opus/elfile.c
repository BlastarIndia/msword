/* elfile.c -- Opel functions for the Document File Interface */


#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "el.h"
#include "doc.h"
#include "rerr.h"
#include "heap.h"
#include "doslib.h"
#include "prompt.h"
#include "version.h"		/* For szVersionNum */
#include "disp.h"   		/* For MWD */
#include "props.h"
#include "sel.h"
#include "format.h"		/* for FLI */
#include "file.h"
#include "print.h"
#include "macrocmd.h"
#include "resource.h"


#ifdef PROTOTYPE
#include "elfile.cpt"
#endif /* PROTOTYPE */

extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern CHAR             szNone[];
extern struct FLI vfli;
extern struct SELS selCur;
extern struct STTB ** vhsttbOpen;
extern int vcElParams;
extern struct PRI       vpri;
extern HWND vhwndDeskTop;
extern SB sbStrings;


/* GLOBALS */

int vcelof = 0;	/* Count of files open by macros */

struct ELOF  **mpstmM1hELOF[stmLast] = { hNil, hNil, hNil, hNil };


struct FINDFILE **hDTA = hNil;
char ** hstFilePath;


#ifdef DEBUG
/* For debugging purposes, shows contents of all ELOF's */
/* %%Function:CommAllELOF %%Owner:bradch */
CommAllELOF()
{
	struct ELOF ** hELOF;
	int stm;

	CommSzNum("Displaying all ELOF's, 1 through ", stmLast);
	for (stm = stmMin; stm < stmMax; stm++)
		{
		CommSzNum("  ELOF number ", stm);
		hELOF = mpstmM1hELOF[stm - 1];
		if (hELOF == hNil)
			{
			CommSz("    hELOF is hNil.\r\n");
			}
		else
			{
			CommSzNum("    hFile=", (*hELOF)->hFile);
			CommSzNum("    ofs.fFixedDisk=", (unsigned) (*hELOF)->ofs.fFixedDisk);
			CommSzNum("    ofs.nErrCode=", (*hELOF)->ofs.nErrCode);
			CommSzSz("    ofs.szFile=", (*hELOF)->ofs.szFile);
			switch ((*hELOF)->wMode)
				{
			case OF_READ:
				CommSz("    wMode=OF_READ\r\n");
				break;
			case OF_WRITE:
				CommSz("    wMode=OF_WRITE\r\n");
				break;
			default:
				CommSz("    wMode=unknown...\r\n");
				break;
				}
			CommSzLong("    lcch=", (*hELOF)->lcch);
			}
		}
}


#endif



/* Some of this was taken from Excel's GetInfo.c.
* TonyK 8/08/88 */
/* Brad: This doesn't really belong here.  I'd put it in elmisc, but I get OOM. */
/* %%Function:ElSdAppInfo %%Owner:bradch */
EL SD ElSdAppInfo(type)
int type;
{
	int w = 0;
	long l = 0L;
	int cch;
	char rgch[256];
	char *pch;
	int dxp, dyp;
	struct RC rc;

	long LcbUnusedEmm();
	extern HWND vhwndApp;
	extern struct MWD ** hmwdCur;
	extern int vsasCur;
	extern int nDateBuilt;
	extern BOOL vfLargeFrameEMS;
	extern BOOL vfSmallFrameEMS;
	extern BOOL vfSingleApp;	/* fTrue if using Windows Runtime */
	extern int vssc;		/* mode */
	extern BOOL f8087;		/* fTrue if Opus is using 8087 */
	extern int vfMouseExist;	/* fTrue if a mouse exists */
	extern int vwWinVersion;
	extern struct BPTB vbptbExt;
	extern int vcparaReserve;
	extern int vcparaMaxReqLast;
	extern int vcparaRequestLast;
	extern int vcparaSwapLast;

	pch = rgch;
	rgch[0] = '\0';

	switch (type)
		{
	case 1: /* The environment, e.g. "Windows 2.03" */
		cch = CchCopySz(vfSingleApp ?
				SzSharedKey("Runtime Windows ", runtimeEnv) :
				SzSharedKey("Windows ", Env), rgch);
		/* We'll assume that we're in Windows 9.x or previous... */
		Assert(HIBYTE(vwWinVersion) < 10);
		rgch[cch++] = HIBYTE(vwWinVersion) + '0';
		rgch[cch++] = '.';
		/* Since version is in binary-coded decimal, take high order
			4 bits for "tenths" and low order 4 bits for "hundredths" */
		rgch[cch++] = (LOBYTE(vwWinVersion) >> 4) + '0';
		if (w = LOBYTE(vwWinVersion) & 0xF)	/* low-order 4 bits */
			rgch[cch++] = w + '0';
		rgch[cch] = '\0';
		break;

	case 2: /* The version number of WinWord e.g. "1.00" */
		CchCopySz(SzShared(szVersionNum), rgch);
		break;

	case 3: /* True (-1) if WinWord is in CopyText, MoveText,
		or CopyLooks mode */
		rgch[CchIntToPpch(vssc != sscNil ?
				elFTrue : elFFalse, &pch)] = '\0';
		break;

	case 4:/* X position of WinWord Application window, measured
				in points from left of screen */
		GetWindowRect(vhwndApp, (LPRECT) &rc);
		/* After converting from pixels to points, create string */
		rgch[CchIntToPpch(DxpFromDxs(rc.xpLeft), &pch)] = '\0';
		break;

	case 5:/* Y position of WinWord Application window, measured
				in points from top of screen */
		GetWindowRect(vhwndApp, (LPRECT) &rc);
		/* After converting from pixels to points, create string */
		rgch[CchIntToPpch(DypFromDys(rc.ypTop), &pch)] = '\0';
		break;

	case 6:/* Width of the desktop area, measured in points */
		GetWindowRect(vhwndDeskTop, (LPRECT) &rc);
		w = rc.xpRight - rc.xpLeft + 1;
		/* After converting from pixels to points, create string */
		rgch[CchIntToPpch(DxpFromDxs(w), &pch)] = '\0';
		break;

	case 7:/* Height of the desktop area, measured in points */
		GetWindowRect(vhwndDeskTop, (LPRECT) &rc);
		w = rc.ypBottom - rc.ypTop + 1;
		/* After converting from pixels to points, create string */
		rgch[CchIntToPpch(DypFromDys(w), &pch)] = '\0';
		break;

	case 8:/* Is application maximized? */
		rgch[CchIntToPpch((GetWindowLong(vhwndApp, GWL_STYLE)
				& WS_MAXIMIZE) != 0L ? elFTrue : elFFalse,
				&pch)] = '\0';
		break;

	case 9:/* Total conventional memory in Kbytes */
		rgch[CchLongToPpch((GlobalCompact(0L) + MemUsed(1)) >> 10,
				&pch)] = '\0';
		break;

	case 10:/* Total conventional memory available in Kbytes */
		rgch[CchLongToPpch(GlobalCompact(0L) >> 10, &pch)] = '\0';
		break;

	case 11:/* Total expanded memory in Kbytes */
		rgch[CchLongToPpch(smtMemory==smtLIM ? CbFreeEmm() + MemUsed(2) >> 10 : 0L, &pch)] = '\0';
		break;

	case 12:/* Total expanded memory available in Kbytes */
		rgch[CchLongToPpch(smtMemory==smtLIM ?
				(CbFreeEmm()+LcbUnusedEmm()) >> 10 : 0L,
								&pch)] = '\0';
		break;

	case 13:/* True (-1) if a math coprocessor is installed */
			/*  initialize the mathpack if we haven't done so 
			already to get f8087 */
			{
			extern BOOL fInitialized;
			if (!fInitialized) InitMath();
			}
		rgch[CchIntToPpch(f8087 ?
				elFTrue : elFFalse, &pch)] = '\0';
		break;

	case 14:/* True (-1) if a mouse is present */
		rgch[CchIntToPpch(vfMouseExist ?
				elFTrue : elFFalse, &pch)] = '\0';
		break;

	case 15:/* Available disk space, default drive */
		rgch[CchLongToPpch(LcbDiskFreeSpace(0) >> 10,
				&pch)] = '\0';
		break;




/* ---------------------------------------------------------------------
	Note: the following are for debugging purposes only, but we want them
	in the NON-DEBUG version.  (this is a way of getting system info from
	the fast version, even after shipping). 
*/

		/* general info */
	case 651:/* opus version */
		CchCopySz(SzShared(szVersionDef), rgch);
		SzSzAppend(rgch, SzShared(", "));
		SzSzAppend(rgch, SzShared(szVerDateDef));
		break;

	case 652:/* type of version and build date */
#ifdef SHIP
		SzSzAppend(rgch, SzShared("shipping "));
#endif /* SHIP */
#ifdef DEMO
		SzSzAppend(rgch, SzShared("demonstration "));
#endif /* DEMO */
#ifdef RSH
		SzSzAppend(rgch, SzShared("research "));
#endif /* RSH */
#ifdef MKTGPRVW
		SzSzAppend(rgch, SzShared("marketing preview "));
#endif /* MKTGPRVW */
#ifdef DEBUG
		SzSzAppend(rgch, SzShared("debug"));
#endif /* DEBUG */
#ifdef HYBRID
		SzSzAppend(rgch, SzShared("hybrid"));
#endif /* HYBRID */
#ifdef FAST
		SzSzAppend(rgch, SzShared("fast"));
#endif /* FAST */
		SzSzAppend(rgch, SzShared(" version--built "));
		SzSzAppend(rgch, SzShared(szDateBuiltDef));
		break;

	case 653:/* printer info */
		SzSzAppend(rgch, vpri.hszPrinter?*vpri.hszPrinter:szNone);
		SzSzAppend(rgch, SzShared(" ("));
		SzSzAppend(rgch, vpri.hszPrDriver?*vpri.hszPrDriver:szNone);
		SzSzAppend(rgch, SzShared(") on "));
		SzSzAppend(rgch, vpri.hszPrPort?*vpri.hszPrPort:szNone);
		break;

	case 654:/* actual version number */
		pch += CchCopySz(SzShared(szVersionNum), rgch);
		*pch++ = '.';
		CchIntToPpch(nRelProduct, &pch);
		*pch = 0;
		break;
		
		
	case 655:
	case 656:
	case 657:
	case 658:
	case 659:
	case 660:
		break;

		/* Memory Info */
	case 661:/* value of smtMemory */
		rgch[CchIntToPpch(smtMemory, &pch)] = '\0';
		break;

	case 662:/* is the kernel using lim4? */
		rgch[CchIntToPpch(vfLargeFrameEMS||vfSmallFrameEMS ?
				elFTrue : elFFalse, &pch)] = '\0';
		break;

	case 663:/* is the kernel using LargeFrame? */
		rgch[CchIntToPpch(vfLargeFrameEMS ?
				elFTrue : elFFalse, &pch)] = '\0';
		break;

	case 664:/* is Opus using EMM? */
		rgch[CchIntToPpch(smtMemory==smtLIM ?
				elFTrue : elFFalse, &pch)] = '\0';
		break;

	case 665:/* how big is the file cache */
		rgch[CchIntToPpch(vbptbExt.ibpMac, &pch)] = '\0';
		break;

	case 666:/* win3 free space */
		if (vwWinVersion >= 0x0300)
			{
			long (FAR PASCAL *lpfn)();
			Assert(vwWinVersion >= 0x0300);

			lpfn = GetProcAddress(GetModuleHandle(SzFrame("KERNEL")),MAKEINTRESOURCE(idoGetFreeSpace));
			Assert(lpfn != NULL);

			rgch[CchLongToPpch((*lpfn)(0), &pch)] = 0;
			}
		else
			rgch[0] = 0;
		break;

	case 667:/* startup - csbMaxAvail */
		{
		extern int vcsbMaxAvailInit;
		rgch[CchIntToPpch(vcsbMaxAvailInit, &pch)] = 0;
		break;
		}
	case 668:
	case 669:
	case 670:
		break;

		/* Swap Area Info */
	case 671:/* returns string of vsasCur */
		rgch[CchIntToPpch(vsasCur, &pch)] = '\0';
		break;

	case 672:/* vcparaReserve */
		rgch[CchIntToPpch(vcparaReserve, &pch)] = '\0';
		break;

	case 673:/* from last SetOurSwapArea(fTrue) call: cparaMaxReq */
		rgch[CchIntToPpch(vcparaMaxReqLast, &pch)] = '\0';
		break;

	case 674:/* from last SetOurSwapArea(fTrue) call: cparaRequest */
		rgch[CchIntToPpch(vcparaRequestLast, &pch)] = '\0';
		break;

	case 675:/* from last SetOurSwapArea(fTrue) call: cparaSwap */
		rgch[CchIntToPpch(vcparaSwapLast, &pch)] = '\0';
		break;


	case 676:
	case 677:
	case 678:
	case 679:
	case 680:
		break;

		/* Spares */
	case 681:
	case 682:
	case 683:
	case 684:
	case 685:
	case 686:
	case 687:
	case 688:
	case 689:
		break;

		/* Last debugging entry */
	case 690:
		break;
/* ------------------------------------------------------------------------ */

	default:	/* Invalid type */
		RtError(rerrOutOfRange);
		break;
		}
	return SdCopySz(rgch);
}


/* %%Function:ElFIsDirty %%Owner:bradch */
EL ElFIsDirty()
{
	extern MES ** vhmes;
	int imei;

	Assert(selCur.doc != docNil);

	return ((imei = ImeiFromDoc(selCur.doc)) != iNil ?
			(PmeiImei(imei)->fDirty || PdodDoc(selCur.doc)->fDirty) :
			DiDirtyDoc(selCur.doc)) ? -1 : 0;
}


/* %%Function:ElSetDirty %%Owner:bradch */
EL ElSetDirty(fDirty)
BOOL fDirty;
{
	struct DOD * pdod;

	Assert(selCur.doc != docNil);

	if (vcElParams == 0)
		fDirty = fTrue;

	if (fDirty)
		fDirty = 1;

	pdod = PdodDoc(selCur.doc);
	pdod->fDirty = fDirty;
	pdod->fStshDirty = fDirty;
}


/* %%Function:ElSdFileName %%Owner:bradch */
EL SD ElSdFileName(iMRU)
int iMRU;
{
	Assert(vhsttbOpen != hNil);

	if (iMRU == 0)
		{
		char st [256];

		st[0] = 0;
		GetDocSt(selCur.doc, st, gdsoFullPath);
		return SdCopySt(st);
		}

	iMRU -= 1; /* internally, we want it based at 0 */

	if (iMRU < 0 || iMRU >= (*vhsttbOpen)->ibstMac)
		{
		RtError(rerrOutOfRange);
		return 0;
		}

	return SdCopyHsttbIst(vhsttbOpen, iMRU);
}


/* %%Function:ElWCountFiles %%Owner:bradch */
EL int ElWCountFiles()
{
	Assert(vhsttbOpen != hNil);
	return (*vhsttbOpen)->ibstMac;
}



/* %%Function:ElSdFiles %%Owner:bradch */
EL SD ElSdFiles(hstFilename)
char ** hstFilename;
{
	int fResult, cch;
	char * pchSep, * pch;
	char szBuf [ichMaxFile];

	if (vcElParams == 1)
		{
		StToSzInPlace(*hstFilename);

		if (**hstFilename == '.' && (*hstFilename)[1] == '\0')
			{
			extern CHAR stDOSPath [];

			UpdateStDOSPath();
			return SdCopySt(stDOSPath);
			}

		if (hstFilePath != hNil)
			FreeH(hstFilePath);
		
		/* Find the end of the path part... */
		pch = *hstFilename; /* really an sz by now */
		pchSep = 0;
		while (*pch != '\0')
			{
			if (*pch == '/' || *pch == '\\' || *pch == ':')
				pchSep = pch + 1;
			pch += 1;
			}
		
		if (pchSep != 0 && (cch = pchSep - *hstFilename) > 0)
			{
			if ((hstFilePath = HAllocateCb(cch + 1)) == hNil)
				{
				RtError(rerrOutOfMemory);
				Assert(fFalse); /* NOT REACHED */
				}
			bltb(*hstFilename, *hstFilePath + 1, cch);
			**hstFilePath = cch;
			}
		else
			{
			hstFilePath = hNil;
			}
		}

	if (hDTA == hNil)
		{
		if (vcElParams == 0 || **hstFilename == '\0')
			RtError(rerrIllegalFunctionCall);

		if ((hDTA = HAllocateCb(sizeof(struct FINDFILE))) == hNil)
			{
			if (hstFilePath != hNil)
				FreePh(&hstFilePath);
			RtError(rerrOutOfMemory);
			Assert(fFalse); /* NOT REACHED */
			}
		}

	if (**hstFilename == 0)
		fResult = FNext(*hDTA);
	else
		fResult = FFirst(*hDTA, *hstFilename, DA_NORMAL | DA_READONLY);

	szBuf[0] = '\0';
	
	if (!fResult)
		{
		pch = szBuf;
		if (hstFilePath != hNil)
			{
			bltb(*hstFilePath + 1, szBuf, **hstFilePath);
			pch += **hstFilePath;
			}
		
		CchCopySz((*hDTA)->szFileName, pch);
		AnsiUpper(szBuf);
		}
	
	return SdCopySz(szBuf);
}



/* Converts an sd to either all uppercase or all lowercase. */
/* Called by ElSdUcase and ElSdLcase. */
/* %%Function:SdChgCaseHpsd %%Owner:bradch */
SdChgCaseHpsd(hpsd, fUpper)
SD huge * hpsd;
BOOL fUpper;	/* If true, translates to Uppercase, otherwise to Lowercase */
{
	SD sdResult;
	uns cch;

	sdResult = SdDupSd(*hpsd);
	if (sdResult == sdNil)
		{
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}
	cch = CchFromSd(sdResult);
	bltbh(HpchFromSd(sdResult), HpchFromSd(sdResult) - 1, cch);
	*(HpchFromSd(sdResult) + cch - 1) = '\0';
	if (fUpper)
		AnsiUpper(LpFromHp(HpchFromSd(sdResult) - 1));
	else
		AnsiLower(LpFromHp(HpchFromSd(sdResult) - 1));
	bltbh(HpchFromSd(sdResult) - 1, HpchFromSd(sdResult), cch);
	/* now restore length */
	CchFromSd(sdResult) = cch;
	return sdResult;

}


/* %%Function:ElSdUcase %%Owner:bradch */
EL SD ElSdUcase(hpsd)
SD huge * hpsd;
{
	return sdChgCaseHpsd(hpsd, fTrue /* i.e., chg to upper */);
}



/* %%Function:ElSdLcase %%Owner:bradch */
EL SD ElSdLcase(hpsd)
SD huge * hpsd;
{
	return sdChgCaseHpsd(hpsd, fFalse /* i.e., chg to lower */);
}


/* FUTURE: reimplement the file interface routines so that they
sit on top of Opus's FN code. */

/* FCheckSzFileInUse(sz):
*  Returns whether the file sz is currently open, either by a macro or
*  by Opus in some other way.   Calls FNormalizeSzFile to get the absolute
*  filename, and then checks that against all the filenames currently open
*  by Macros (which are accessed via mpstmM1hELOF), and FHasFcbFromSt,
*  which tells if file is in use by Opus.
* Called by ElOpen, ElKill, ElName.
* Local function.
*/

/* %%Function:FCheckSzFileInUse %%Owner:bradch */
BOOL FCheckSzFileInUse(sz)
char * sz;
{
	char stzFile[ichMaxFile + 1];
	int stm;

	if (!FNormalizeSzFile(sz, &stzFile[1], nfoNormal))
		{
		/* Filename has some error */
		RtError(rerrBadFileName);
		/* NOT REACHED */
		Assert(fFalse);
		}

	stzFile[0] = CchSz(&stzFile[1])-1;	/* Make it an stz */

	for (stm = stmMin; stm < stmMax; stm++)
		{
		if (mpstmM1hELOF[stm - 1] != hNil)
			{
			if (!FNeNcSz(&stzFile[1], (*mpstmM1hELOF[stm-1])->ofs.szFile))
				return fTrue;
			}
		}
	if (FHasFcbFromSt(stzFile))	/* File in use by Opus */
		return fTrue;
	return fFalse;
}



/* ElSdInputn(cchReq, stm):
*  Reads cchReq bytes from the open file referenced via stm.
*  Returns sd.
* EL function INPUT$(cch, stm).
*/
/* %%Function:ElSdInputn %%Owner:bradch */
EL SD ElSdInputn(cchReq, stm)
int cchReq;
int stm;
{
	SD sd;
	int cchAct;
	CHAR HUGE *hpch;

	/* No need to check for a max string length, since an SD uses a
		int also as it's length */
	if (cchReq < 0)
		{
		RtError(rerrBadParam);
		Assert(fFalse);	/* NOT REACHED */
		}
	CheckStmOpen(stm);

	if ((sd=SdFromCch(cchReq))==sdNil)
		{
		RtError(rerrOutOfMemory);
		Assert(fFalse); /* NOT REACHED */
		}

	hpch=HpchFromSd(sd);
	cchAct = CchReadDoshnd((*mpstmM1hELOF[stm - 1])->hFile,LpLockHp(hpch),cchReq);
	UnlockHp(hpch);
	if (cchAct < 0)
		{
		/* error reading file: negative result is neg. of error code */
		/* only known error is 5, handle not open for reading */
		RtError(rerrBadFileMode);
		Assert(fFalse);	/* NOT REACHED */
		}
	CloseFloppyStm(stm);
	if (cchAct != cchReq)
		{
		RtError(rerrInputPastEndOfFile);
		Assert(fFalse);	/* NOT REACHED */
		}
	return sd;
}



/* ElOpen(fom, stm, sd)
*  Opens a file in specified mode (fom) by calling OpenFile, then stores
*  the information in a ELOF accessed via mpstmM1hELOF[stm - 1].
*  Errors are handled via RtError.
* EL function OPEN "mode", stm, "file name"
*          or OPEN "file name" FOR "mode" AS #stm.
*/
/* %%Function:ElOpen %%Owner:bradch */
ElOpen(fom, stm, sd)
int fom;
int stm;
SD sd;
{
	HANDLE hFile;
	struct OFS ofsT;
	struct ELOF ** hELOF;
	int of;
	long lcch;
	char szFilename[ichMaxFile];
	char szAbsFilename[ichMaxFile];
	char ** hsz;
	int wMode;

	if (stm < stmMin || stm >= stmMax)
		{
		RtError(rerrBadFileNameOrNumber);
		Assert(fFalse);	/* NOT REACHED */
		}

	if (mpstmM1hELOF[stm - 1] != hNil)
		{
		RtError(rerrFileAlreadyOpen);
		Assert(fFalse);	/* NOT REACHED */
		}

	if (CchFromSd(sd) >= sizeof (szFilename))
		{
		RtError(rerrStringTooBig);
		Assert(fFalse); /* NOT REACHED */
		}

	bltbh(HpchFromSd(sd), (char huge *) szFilename, CchFromSd(sd));
	szFilename[CchFromSd(sd)] = '\0';

	if (!FNormalizeSzFile(szFilename, szAbsFilename, nfoNormal))
		{
		/* Filename has some error */
		RtError(rerrBadFileName);
		Assert(fFalse);	/* NOT REACHED */
		}

	/* NOTE: Since it already does this, the error check is redundant. */
	if (FCheckSzFileInUse(szFilename) == fTrue)
		{
		/* Not allowed to open file more than once... */
		RtError(rerrFileAlreadyOpen);
		Assert(fFalse);	/* NOT REACHED */
		}

	if ((hELOF = HAllocateCb(cbELOF)) == hNil)
		{
		RtError(rerrOutOfMemory);
		}

	mpstmM1hELOF[stm - 1] = hELOF;
	vcelof++;	/* Increment number of files open by macros */

	switch (fom)
		{
	case fomOutput:	/* set flags to open file for output only */
		of = OF_CREATE | OF_WRITE;
		wMode = OF_WRITE;
		break;
	case fomAppend:	/* set flags to open file for append (output only) */
		of = wMode = OF_WRITE;
		break;
	case fomInput:	/* set flags to open file for input only */
		of = wMode = OF_READ;
		break;
	default:	/* Invalid mode specified */
		RtError(rerrBadFileMode);
		Assert(fFalse);	/* NOT REACHED */
		}

	hFile = OpenFile((LPSTR) szAbsFilename, (LPOFSTRUCT) &ofsT, of);
	if (hFile==-1 && fom==fomAppend)
		hFile = OpenFile((LPSTR) szAbsFilename, (LPOFSTRUCT) &ofsT, OF_CREATE);
	if (((*hELOF)->hFile=hFile)==-1)
		{
		/* DOS error code contained in ofsT.nErrCode */
		switch (ofsT.nErrCode)
			{
		case 1: 
		case 12:
			/* Invalid Function Code */
			RtError(rerrDeviceIOError);
		case 2:
			/* The file specified is invalid or doesn't exist */
			RtError(rerrFileNotFound);
		case 3:
			/* The path specified is invalid or doesn't exist */
			RtError(rerrPathNotFound);
		case 4:
			/* No handles are available in the current process
				or the internal system tables are full */
			RtError(rerrTooManyFiles);
		case 5:
			/* The program attempted to open a directory or
				Volume-ID, or open a read-only file for writing */
			RtError(rerrPathFileAccessError);
		default:
			/* Unknown error! */
			RtError(rerrDeviceIOError);
			}
		/* NOT REACHED */
		Assert(fFalse);
		}

	(*hELOF)->wMode = wMode;
	(*hELOF)->ofs = ofsT;

	if (fom == fomAppend)
		{
		/* for append, seek to end of file */
		(*hELOF)->lcch = DwSeekDw((*hELOF)->hFile, 0, 2);
		}
	else
		{
		(*hELOF)->lcch = 0L;	/* We're at the beginning of the file */
		Assert((*hELOF)->lcch == DwSeekDw((*hELOF)->hFile, 0, 1));
		}

	CloseFloppyStm(stm);
}



/* ElClose(stm)
*  Closes the file referenced via stm, or all open files if stm
*  unspecified.   Not an error to close a stm that is not open.
* EL function CLOSE([stm])
*/
/* %%Function:ElClose %%Owner:bradch */
ElClose(stm)
int stm;
{
	int istm;
	int istmFirst;
	int istmLast;
	int err;
	HANDLE hFile;

	if (vcElParams == 0)	/* stm unspecified, close all */
		{
		istmFirst = stmMin;
		istmLast = stmLast;
		}
	else
		{
		if (stm < stmMin || stm > stmLast)
			{
			RtError(rerrBadFileNameOrNumber);
			Assert(fFalse);	/* NOT REACHED */
			}
		istmFirst = istmLast = stm;
		}
	for (istm = istmFirst; istm <= istmLast; istm++)
		{
		if (mpstmM1hELOF[istm - 1] != hNil) /* if file is open, close it */
			{
			hFile = (*mpstmM1hELOF[istm - 1])->hFile;
			if (hFile != -1) /* if error opening file, hFile will be -1 */
				{
				FCloseDoshnd(hFile);
				/* We ignore errors closing files */
				vcelof--;	/* Decrement number of files open by macros */
				}
			/* Free ELOF from heap */
			FreePh(&mpstmM1hELOF[istm - 1]);
			}
		}
}



/* CheckStmOpen(stm)
*  Checks the stm number and calls RtError if the stm is out of range
*  OR that stm is not currently open.  If the file is on a floppy disk,
*  reopens the file (which should have been closed by CloseFloppyStm).
* Called by ElSdInputn, GetPosInfo, ElWEof, ElNumLof.
* Local function.
*/
/* %%Function:CheckStmOpen %%Owner:bradch */
CheckStmOpen(stm)
int stm;
{
	struct ELOF ** hELOF;
	long lcch;
	struct OFS ofsT;

	if (stm < stmMin || stm >= stmMax)
		{
		RtError(rerrBadParam);
		Assert(fFalse);	/* NOT REACHED */
		}

	hELOF = mpstmM1hELOF[stm - 1];
	if (hELOF == hNil)
		{
		RtError(rerrBadFileMode);
		Assert(fFalse);	/* NOT REACHED */
		}

	if (!(*hELOF)->ofs.fFixedDisk)
		{
		ofsT = (*hELOF)->ofs;
		if (((*hELOF)->hFile = OpenFile((LPSTR) ofsT.szFile,
				(LPOFSTRUCT) &ofsT,
				(*hELOF)->wMode | OF_REOPEN | OF_PROMPT | OF_CANCEL))
				== -1)
			{
			RtError(rerrFileNotFound);
			}
		(*hELOF)->ofs = ofsT;
		lcch = DwSeekDw((*hELOF)->hFile, (*hELOF)->lcch, 0);
		Assert(lcch == (*hELOF)->lcch);
		}
}



/* Returns current position in file.   stm is assumed to be valid. */
/* %%Function:LcchCurPosStm %%Owner:bradch */
long LcchCurPosStm(stm)
int stm;
{
	Assert(stm >= stmMin && stm < stmMax && mpstmM1hELOF[stm - 1] != hNil);
	return DwSeekDw((*mpstmM1hELOF[stm - 1])->hFile, 0, 1);
}



/* If file is on a non-fixed disk, close the file's Dos handle, but leave
* the ELOF information intact so the file can be reopened.  This prevents
* problems if the user swaps disks. */
/* %%Function:CloseFloppyStm %%Owner:bradch */
CloseFloppyStm(stm)
int stm;
{
	Assert(stm >= stmMin && stm < stmMax && mpstmM1hELOF[stm - 1] != hNil);

	if (!(*mpstmM1hELOF[stm - 1])->ofs.fFixedDisk)
		{
		(*mpstmM1hELOF[stm - 1])->lcch = LcchCurPosStm(stm);
		FCloseDoshnd((*mpstmM1hELOF[stm - 1])->hFile);
		}
}



/* GetPosInfo(stm, plcchCurrent, plcchMax)
*  Places the current file position in *plcchCurrent and the maximum
*  file position in *plcchMax.   File is referenced via stm.
* stm is assumed to be valid.
* Called by ElWEof, ElWLof.
*/
/* %%Function:GetPosInfo %%Owner:bradch */
GetPosInfo(stm, plcchCurrent, plcchMax)
long * plcchCurrent;
long * plcchMax;
{
	long lcchT;
	HANDLE hFile;

	Assert(stm >= stmMin && stm < stmMax);

	hFile = (*mpstmM1hELOF[stm - 1])->hFile;
	/* get current position in file (offset=0, mode=current) */
	*plcchCurrent = DwSeekDw(hFile, 0, 1);

	/* get one past last valid position in file (offset=0, mode=end) */
	*plcchMax = DwSeekDw(hFile, 0, 2);

	/* now seek back to original position (offset=lcchCurrent, mode=beginning) */
	lcchT = DwSeekDw(hFile, *plcchCurrent, 0);

	Assert(*plcchCurrent >= 0 && *plcchMax >= 0 && lcchT >= 0 && *plcchCurrent == lcchT);
}



/* ElWEof(stm)
*  Returns -1 if eof reached in file, 0 if not.   File referenced via stm.
* EL function EOF(stm).
*/
/* %%Function:ElWEof %%Owner:bradch */
int ElWEof(stm)
int stm;
{
	long lcchCurrent;
	long lcchMax;

	CheckStmOpen(stm);
	GetPosInfo(stm, &lcchCurrent, &lcchMax);
	CloseFloppyStm(stm);
	return (lcchCurrent >= lcchMax) ? elFTrue : elFFalse;
}



/* ElWLof(stm)
*  Returns length of file in bytes.   File referenced via stm.
* EL function LOF(stm).
*/
/* %%Function:ElNumLof %%Owner:bradch */
NUM ElNumLof(stm)
int stm;
{
	NUM numRet;
	long lcchCurrent;
	long lcchMax;

	CheckStmOpen(stm);
	GetPosInfo(stm, &lcchCurrent, &lcchMax);
	CloseFloppyStm(stm);
	NumFromL(lcchMax, &numRet);
	return (numRet);
}



/* ElNumSeek(stm, numPos)
*  Can be called either as a function or a statement.
*  As a function, returns current position in file referenced via stm.
*  As a statement, sets file pointer (position) to numPos.
* EL function SEEK(stm) / SEEK stm, position
*/
/* %%Function:ElNumSeek %%Owner:bradch */
NUM ElNumSeek(stm, numPos)
int stm;
NUM numPos;
{
	long lcchCurrent;
	long lcchLast;
	NUM numRet;
	extern int vcElParams;
	extern BOOL vfElFunc;
	extern long LWholeFromNum();
	long lPos = LWholeFromNum (&numPos, fFalse);
	HANDLE hFile;

	CheckStmOpen(stm);
	hFile = (*mpstmM1hELOF[stm - 1])->hFile;
	if (vfElFunc)
		{
		/* function, return current position in file */
		lcchCurrent = DwSeekDw(hFile, 0, 1);
		lPos = lcchCurrent;
		}
	else  /* called as a statement to seek to position lPos */		
		{
		/* DwSeekDw parameters: dos file handle, seek position, mode (0 = relative to beginning of file) */
		lcchLast = DwSeekDw(hFile, 0, 2);
		if (lPos < 0 || lPos > lcchLast)
			{
			/* Error: invalid position requested */
			RtError(rerrOutOfRange);
			}
		lcchCurrent = DwSeekDw(hFile, lPos, 0);
		}
	CloseFloppyStm(stm);
	if (lcchCurrent < 0 || lPos != lcchCurrent)
		{
		/* Error: DwSeekDw returned an error or not the appropriate position */
		RtError(rerrDeviceIOError);
		/* NOT REACHED */
		Assert(fFalse);
		}
	NumFromL(lPos, &numRet);
	return (numRet);
}



/* ElKill(hstFile)
*  Deletes the file *hstFile.   Not allowed to delete a file that
*  is currently open.
* EL function KILL "file name".
*/
/* %%Function:ElKill %%Owner:bradch */
ElKill(hstFile)
char ** hstFile;
{
	struct FINDFILE ** hDTA;
	int err;
	char szAbsFilename [ichMaxFile];

	StToSzInPlace(*hstFile);
	if ((hDTA = HAllocateCb(sizeof(struct FINDFILE))) == hNil)
		{
		RtError(rerrOutOfMemory);
		Assert(fFalse); /* NOT REACHED */
		}

	if (FFirst(*hDTA, *hstFile, DA_NORMAL | DA_READONLY))
		goto LNotFound;

	do
		{
		MakeFileName(*hstFile,(*hDTA)->szFileName, szAbsFilename);
		if (FCheckSzFileInUse(szAbsFilename) == fTrue)
			{
			/* Not allowed to delete a file that 
				is currently open */
			RtError(rerrFileAlreadyOpen);
			Assert(fFalse); /* NOT REACHED */
			}

		if ((err = EcDeleteSzFfname(szAbsFilename)) < 0)
			{
			switch (err)
				{
			case -2:
				/* Path is invalid or file doesn't exist */
LNotFound:
				FreePh(&hDTA);
				RtError(rerrFileNotFound);

			case -5:
				/* Path specifies a directory 
					or read-only file */
				FreePh(&hDTA);
				RtError(rerrPathFileAccessError);

			default:
				/* Unknown error! */
				FreePh(&hDTA);
				RtError(rerrDeviceIOError);
				}
			}
		}
	while (!FNext(*hDTA));

	FreePh(&hDTA);
}


/* %%Function:MakeFileName %%Owner:bradch */
MakeFileName(szFullName, szLastName, szNewName)
char *szFullName;
char *szLastName;
char szNewName[];
{
	int ichbsl=-1; /* the index where the last \ is present in szFullName */
	int ich;
	char *szT;

	for (ich=0, szT=szFullName; *szT != '\0'; ich++, szT++)
		if (*szT=='\\' && ich > ichbsl) ichbsl=ich;
	for (ich=0, szT=szFullName; ich <= ichbsl; ich++)
		{
		szNewName[ich] = *szT;
		szT++;
		}
	szT = szLastName;
	while (*szT != '\0')
		{
		szNewName[ich] = *szT;
		ich++;
		szT++;
		}
	szNewName[ich] = '\0';
}







/* ElName(hpsdCurrent, hpsdNew)
*  Renames the file *hpsdCurrent to *hpsdNew.   Not allowed to rename
*  a file that is currently open.
* EL function NAME "current name", "new name".
*
* NOTE: this is called directly from from the interpreter!
*/
/* %%Function:ElName %%Owner:bradch */
ElName(hpsdCurrent, hpsdNew)
SD huge * hpsdCurrent;
SD huge * hpsdNew;
{
	int err, cchOldName, cchNewName;
	char szOldName [ichMaxFile];
	char szNewName [ichMaxFile];

	if ((cchOldName = CchFromSd(*hpsdCurrent)) > ichMaxFile - 1 || 
			(cchNewName = CchFromSd(*hpsdNew)) > ichMaxFile - 1)
		{
		RtError(rerrStringTooBig);
		}

	BLTBH(HpchFromSd(*hpsdCurrent), (CHAR huge *) szOldName, cchOldName);
	szOldName[cchOldName] = '\0';

	BLTBH(HpchFromSd(*hpsdNew), (CHAR huge *) szNewName, cchNewName);
	szNewName[cchNewName] = '\0';

	if (FCheckSzFileInUse(szOldName) == fTrue)
		{
		/* Not allowed to rename a file that is currently open */
		RtError(rerrFileAlreadyOpen);
		/* NOT REACHED */
		Assert(fFalse);
		}
	if ((err = EcRenameSzFfname(szOldName, szNewName)) < 0)
		{
		switch (-err)
			{
		case 2:
			/* One of the paths is invalid or not open */
			RtError(rerrPathFileAccessError);
		case 5:
			/* The first pathname specifies a directory,
				the second pathname specifies an existing
				file, or the second directory entry could
				not be opened */
			RtError(rerrPathFileAccessError);
		case 17:
			/* Both files are not on the same drive */
			RtError(rerrRenameAcrossDisks);
		default:
			/* Unknown error! */
			RtError(rerrDeviceIOError);
			}
		/* NOT REACHED */
		Assert(fFalse);
		}
}



/* ElChdir(hstPath)
*  Changes to the directory *hstPath.   Calls FSetCurStzPath.
* EL function CHDIR "dir name" 
*/
/* %%Function:ElChdir %%Owner:bradch */
ElChdir(hstPath)
char **hstPath;
{
	int err;
	char stzPath [ichMaxFile+1];

	if (**hstPath > ichMaxFile)
		RtError(rerrBadParam);

	CopySt(*hstPath, stzPath);	/* Copy st to stz buffer */
	stzPath[stzPath[0] + 1] = '\0';	/* Then terminate st with null to get stz */
	if (!FSetCurStzPath(stzPath))
		{
		/* Error: The pathname doesn't exist or specifies a file,
			not a directory */
		RtError(rerrPathNotFound);
		/* NOT REACHED */
		Assert(fFalse);
		}
	UpdateStDosPath();	/* So Opus knows what current directory is */
	UpdateTitles();		/* Updates titles of docs, menus, etc. */
}



/* ElMkdir(hstPath)
*  Creates the directory *hstPath.   Calls FMakeStzPath.
* EL function MKDIR "dir name"
*/
/* %%Function:ElMkdir %%Owner:bradch */
ElMkdir(hstPath)
char **hstPath;
{
	int err;
	char stzPath [ichMaxFile+1];

	if (**hstPath > ichMaxFile)
		RtError(rerrBadParam);

	CopySt(*hstPath, stzPath);	/* Copy st to stz buffer */
	stzPath[stzPath[0] + 1] = '\0';	/* Then terminate st with null to get stz */
	if ((err = FMakeStzPath(stzPath)) < 0)
		{
		switch (-err)
			{
		case 3:
			/* Path not found */
			RtError(rerrPathNotFound);
		case 5:
			/* No room in the parent directory, a file with the
				same name exists in the current directory, or the
				path specifies a device. */
			RtError(rerrBadFileNameOrNumber);
		default:
			/* Unknown error! */
			RtError(rerrDeviceIOError);
			}
		/* NOT REACHED */
		Assert(fFalse);
		}
}



/* ElRmdir(hstPath)
*  Deletes the directory *hstPath.   Calls FRemoveStzPath.
* EL function RMDIR "dir name"
*/
/* %%Function:ElRmdir %%Owner:bradch */
ElRmdir(hstPath)
char **hstPath;
{
	int err;
	char stzPath [ichMaxFile+1];

	if (**hstPath > ichMaxFile)
		RtError(rerrBadParam);

	CopySt(*hstPath, stzPath);	/* Copy st to stz buffer */
	stzPath[stzPath[0] + 1] = '\0';	/* Then terminate st with null to get stz */
	if ((err = FRemoveStzPath(stzPath)) < 0)
		{
		switch (-err)
			{
		case 3:
			/* Path not found */
			RtError(rerrPathNotFound);
		case 5:
			/* The directory isn't empty; or the path doesn't
				specify a directory, specifies the root directory,
				or is invalid. */
			RtError(rerrPathFileAccessError);
		case 16:
			/* The path specifies the current directory */
			RtError(rerrPathFileAccessError);
		default:
			/* Unknown error! */
			RtError(rerrDeviceIOError);
			}
		/* NOT REACHED */
		Assert(fFalse);
		}
}


/* %%Function:ElPrint %%Owner:bradch */
EL ElPrint(stm, hpsd)
int stm;
SD huge * hpsd;
{
	/* extern char szApp [];
	char ** hsz; */

	if (stm == 0)
		{
		char * pch, huge * hpch;
		int cch, cchTab, cchSrc;
		char stBuf [256];
		
		cch = 0;
		pch = &stBuf[1];
		hpch = HpchFromSd(*hpsd);
		if ((cchSrc = CchFromSd(*hpsd)) > 255)
			cchSrc = 255;
		while (cch < cchSrc)
			{
			char ch;
			
			switch (ch = *hpch++)
				{
			case '\r':
			case '\n':
				goto LEndOfLine;
			
			case '\t':
				cchTab = 8 - (cch % 8);
				if (cch + cchTab > 255)
					goto LEndOfLine;
				SetBytes(pch, ' ', cchTab);
				cch += cchTab;
				pch += cchTab;
				cchSrc = min(255, cchSrc+cchTab-1);
				break;
			
			default:
				*pch++ = ch;
				cch += 1;
				break;
				}
			}
LEndOfLine:
		Assert(cch <= 255);
		stBuf[0] = cch;
		SetPromptSt(stBuf, pdcMenuHelp | pdcmCreate | pdcmPmt);
	
#ifdef DEBUG
		if (vdbs.fCommPrint)
			CommSzSt(szEmpty, stBuf);
#endif
		}
	else  /* send the string to a file... */		
		{
		HANDLE hFile;
		int cchReq, cchAct;

		CheckStmOpen(stm);
		if ((*mpstmM1hELOF[stm - 1])->wMode != OF_WRITE)
			RtError(rerrBadFileMode);
		hFile = (*mpstmM1hELOF[stm - 1])->hFile;
		cchReq = CchFromSd(*hpsd);
		if ((cchAct = CchWriteDoshnd(hFile, LpOfHp(HpchFromSd(*hpsd)), 
				cchReq)) != cchReq)
			{
#ifdef ELFILEDBG
			CommSzNum("CchWriteDoshnd() returned ", cchAct);
#endif
			RtError(rerrDeviceIOError);
			}
		CloseFloppyStm(stm);
		}
}


/* %%Function:ElInput %%Owner:bradch */
ElInput(stm, hpsdPrompt, hpsdInput)
int stm;
SD huge * hpsdPrompt;
SD huge * hpsdInput;
{
	int cch;
	char ** hstPrompt;
	char szInput [256];

	if (stm == 0)
		{
		szInput[0] = '\0';
		hstPrompt = HstzOfSd(*hpsdPrompt);
		TmcInputPmtSt(*hstPrompt, szInput, sizeof (szInput), bcmNil,
				mmoBeepOnMouse);
		FreeH(hstPrompt);
		cch = CchSz(szInput) - 1;
		ReallocSd(*hpsdInput, cch);
		BLTBH((char huge *) szInput, HpchFromSd(*hpsdInput), cch);
		}
	else
		{
		char * pch;
		int cchTotal;
		HANDLE hFile;

		CheckStmOpen(stm);
		hFile = (*mpstmM1hELOF[stm - 1])->hFile;
		pch = szInput;
		cchTotal = 0;
		while (cchTotal < 256 &&
				(cch = CchReadDoshnd(hFile,
				(char far *) pch, 1)) == 1 && *pch != '\n')
			{
			if (*pch != '\r') /* ignore CRs */
				{
				pch += 1;
				cchTotal += 1;
				}
			}

		CloseFloppyStm(stm);
		if (cch == 0 && cchTotal == 0)
			RtError(rerrInputPastEndOfFile);
		else  if (cch < 0)
			RtError(rerrBadFileMode);
		else  if (cchTotal == 256)
			RtError(rerrStringTooBig);
		else
			{
			ReallocSd(*hpsdInput, cchTotal);
			BLTBH((char huge *) szInput, 
					HpchFromSd(*hpsdInput), cchTotal);
			}
		}
}


#ifdef DEBUG
extern int vfElFunc;

/* %%Function:ElMemAllocFail %%Owner:bradch */
EL ElMemAllocFail (cSuc, cFail)
{
	extern struct MERR vmerr;

	if (vfElFunc)
		{
		int cCalls = vdbs.cLmemCalls;
		vdbs.cLmemCalls = 0;
		return cCalls;
		}
	else
		{
		vdbs.cLmemSucceed = max(0, cSuc);
		vdbs.cLmemFail = max(0, cFail);
		vmerr.mat = matNil;
		return 0;
		}
}


/* %%Function:ElWinAllocFail %%Owner:bradch */
EL ElWinAllocFail (cSuc, cFail)
{
	if (vfElFunc)
		{
		int cCalls = vdbs.cWinCalls;
		vdbs.cWinCalls = 0;
		return cCalls;
		}
	else
		{
		vdbs.cWinSucceed = max(0, cSuc);
		vdbs.cWinFail = max(0, cFail);
		return 0;
		}
}


int vnErrorLevel = 0;

/* 0 - no special processing
	1 - ignore errors in commands
	2 - give message boxes default response
*/

/* %%Function:ElDbgErrorLevel %%Owner:bradch */
EL ElDbgErrorLevel(nLevel)
int nLevel;
{
	if (vfElFunc)
		return vnErrorLevel;

	if (nLevel < 0 || nLevel > 2)
		RtError(rerrOutOfRange);

	vnErrorLevel = nLevel;

	return 0;
}


/* %%Function:ElRefreshRate %%Owner:bradch */
EL ElRefreshRate(nRate)
int nRate;
{
	/* no longer implemented */
	return 0;
}




/*  performes idle time debug checks */
/* %%Function:ElDoDebugTests %%Owner:bradch */
EL ElDoDebugTests()
{
	DoDebugTests(fFalse);
	return 0;
}



#endif /* DEBUG */
