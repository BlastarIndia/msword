/* C O N V T E S T . C */
/* Sample converter for use with Microsoft Word for Windows */


#include <windows.h>
#define CC
#include <crmgr.h>


#define fFalse  0
#define fTrue   1

#define ERR_OutOfGlobalMemory    0
#define ERR_GlobalMemory         1

extern HSTACK hStackCaller;
extern HWND hwndCaller;  /* Word's main app window handle, for use with any
                            dialogs the converter must put up */

char *ErrorMess[] =
	{
	"Out of global memory",
	"Error accessing global memory",
	};

struct FOREIGNTORTFPARM
	{
	int PercentComplete;
	int cbPass;
	};

struct RTFTOFOREIGNPARM
	{
	int PercentComplete;
	BOOL fPicture;
	};
	
long DwSeekDw(int, long, int);

PASCAL LibMessageBox(hwnd, lpsz1, lpsz2, word)
HWND hwnd;
LPSTR lpsz1;
LPSTR lpsz2;
WORD word;
{
	return WCallOtherStack((PFN_CRMGR)MessageBox, hStackCaller, &word, 6);
}


LONG PASCAL LibGlobalLock(gh)
HANDLE gh;
{
	return LCallOtherStack((PFN_CRMGR)GlobalLock, hStackCaller, (WORD *)&gh, 1);
}


PASCAL LibGlobalUnlock(gh)
HANDLE gh;
{
	return WCallOtherStack((PFN_CRMGR)GlobalUnlock, hStackCaller, (WORD *)&gh, 1);
}


PASCAL LibOpenFile(lpstrFile, lpBuff, wMode)
LPSTR lpstrFile;
LPOFSTRUCT lpBuff;
WORD wMode;
{
	return WCallOtherStack((PFN_CRMGR)OpenFile, hStackCaller, &wMode, 5);
}


PASCAL LibGlobalReAlloc(gh, lcb, w)
HANDLE gh;
LONG lcb;
WORD w;
{
	return WCallOtherStack((PFN_CRMGR)GlobalReAlloc, hStackCaller, &w, 4);
}


/* This converter deals with a simple variation on RTF.  The string below
   is prepended to the RTF which is otherwise unmodified. */

char szFilePrefix[] = "Foreign File Test ";
#define cchFilePrefix (sizeof(szFilePrefix) - 1)


/* I S  F O R M A T  C O R R E C T */
/* Return fTrue if the passed file is of a format understood by this converter. 
If this converter deals with multiple formats, return the specific format this
file is in in *ghszConvtrVersion.
*/
BOOL PASCAL IsFormatCorrect(ghszFile, ghszConvtrVersion)
HANDLE ghszFile;
HANDLE ghszConvtrVersion;   /* return a null string in ghszConvtrVersion if
							 * don't know which version */
{
	int hFile;
	char szFileName[67];
	OFSTRUCT ReOpenBuff;
	char Buff[30];

	if (!FCopyGhszToSz(ghszFile, szFileName))
		return fFalse;

	if ((hFile = LibOpenFile((LPSTR)szFileName, (LPOFSTRUCT)&ReOpenBuff, OF_READWRITE)) == -1)
		return fFalse;

	DwSeekDw(hFile, 0L, 0);

	CchReadDoshnd(hFile, (char FAR *)Buff, 30);

	FCloseDoshnd(hFile);

	if (strncmp(szFilePrefix, Buff, cchFilePrefix) != 0)
		return fFalse;

	/* this converter always claims files are in the format for version 1.5 */
	FCopySzToGhsz("Test Converter 1.5", ghszConvtrVersion);

	return fTrue;
}


/* F O R E I G N  T O  R T F */
/* Convert a file from this converter's format to RTF using the format specified
in ghszConvtrVersion.
*/
BOOL PASCAL ForeignToRtf(ghszFile, ghBuff, ghszConvtrVersion, ghszSubset, lpfnIn)
HANDLE ghszFile;
HANDLE ghBuff;
HANDLE ghszConvtrVersion;
HANDLE ghszSubset;
int (FAR PASCAL *lpfnIn)();
{
	int hFile;
	char far *lpBuff;
	char szFileName[67];
	OFSTRUCT ReOpenBuff;
	long cbFile;
	long cbOrig;
	int cbPass;
	int wRet = 0;
	struct FOREIGNTORTFPARM ForeignToRtfParm;
	char Buff[30];



	if (!FCopyGhszToSz(ghszFile, szFileName))
		{
		return -8;
		}

	if ((hFile = LibOpenFile((LPSTR)szFileName, (LPOFSTRUCT)&ReOpenBuff, OF_READWRITE)) == -1)
		{
		return -3;
		}

	/* NOTE: cannot assume that IsFormatCorrect was called before.  Must check
	   again. */
	DwSeekDw(hFile, 0L, 0);
	CchReadDoshnd(hFile, (char FAR *)Buff, 30);
	if (strncmp(szFilePrefix, Buff, cchFilePrefix) != 0)
		{
		FCloseDoshnd(hFile);
		return -5;
		}

	cbOrig = cbFile = DwSeekDw(hFile, 0L, 2) - (long)cchFilePrefix;
	DwSeekDw(hFile, (long)cchFilePrefix, 0);

	for (; cbFile > 0 && wRet >= 0; cbFile -= (long)cbPass)
		{
		cbPass = (cbFile > 2048L) ? 2048 : (int)cbFile;

		if (LibGlobalReAlloc(ghBuff, (long)cbPass, GMEM_MOVEABLE) == NULL)
			{
			Error(ERR_OutOfGlobalMemory, MB_OK);
			FCloseDoshnd(hFile);
			return -8;
			}

		lpBuff = (char far *)LibGlobalLock(ghBuff);

		CchReadDoshnd(hFile, lpBuff, cbPass);
		LibGlobalUnlock(ghBuff);
		ForeignToRtfParm.cbPass = cbPass;
		ForeignToRtfParm.PercentComplete = ((cbOrig-cbFile)*100)/cbOrig;
		wRet = (int)WCallOtherStack((PFN_CRMGR)lpfnIn, hStackCaller, 
				(WORD *)&ForeignToRtfParm,
				sizeof (struct FOREIGNTORTFPARM) / sizeof (int));
		}

	FCloseDoshnd(hFile);

	return wRet;
}



/* R T F  T O  F O R E I G N */
/*  Write RTF out in the appropriate foreign format.
*/
PASCAL RtfToForeign(ghszFile, ghBuff, ghszConvtrVersion, lpfnIn)
HANDLE ghszFile;
HANDLE ghBuff;
HANDLE ghszConvtrVersion;
int (FAR PASCAL *lpfnIn)();
{
	int cchBuf;
	int hFile;
	char far *lpBuff;
	char szFileName[67];
	OFSTRUCT ReOpenBuff;
	struct RTFTOFOREIGNPARM RtfToForeignParm;

	RtfToForeignParm.fPicture = fFalse;

	if (!FCopyGhszToSz(ghszFile, szFileName))
		{
		return -8;
		}

	if ((hFile = LibOpenFile((LPSTR)szFileName, (LPOFSTRUCT)&ReOpenBuff, OF_READWRITE)) == -1)
		{
		return -3;
		}

	RtfToForeignParm.PercentComplete = 0;

	CchWriteDoshnd(hFile, (LPSTR)szFilePrefix, cchFilePrefix);

	while ((cchBuf = (int)WCallOtherStack((PFN_CRMGR)lpfnIn, hStackCaller, 
			(WORD *)&RtfToForeignParm,
			sizeof (struct RTFTOFOREIGNPARM) / sizeof (int))) > 0)
		{
		lpBuff = (char far *)LibGlobalLock(ghBuff);
		CchWriteDoshnd(hFile, lpBuff, cchBuf);
		LibGlobalUnlock(ghBuff);
		}

	FCloseDoshnd(hFile);

	return cchBuf;
}


/* F  C O P Y  G H S Z  T O  S Z */
/* Copy a zero terminated string from a global handle to a near buffer. 
*/
FCopyGhszToSz(ghsz, sz)
HANDLE ghsz;
char *sz;
{
	char far *lpsz;

	lpsz = (char far *)LibGlobalLock(ghsz);
	if (lpsz == (char far *)NULL)
		{
		Error(ERR_GlobalMemory, MB_OK);
		return fFalse;
		}

	while ((*sz++ = *lpsz++) != 0)
		;

	LibGlobalUnlock(ghsz);
	return fTrue;
}


char grpszIniName[] = "Test Converter 1.0\000Test Converter 1.5\000\
Test Converter 2.0\000";

char grpszIniExt[] = "^.foo\000^.foo ^.bar\000^.foo\000";

/* G E T  I N I  E N T R Y */
/* Pass information appropriate for this converter's WIN.INI entries back to
the caller.
*/
PASCAL GetIniEntry(ghIniName, ghIniExt)
HANDLE ghIniName;
HANDLE ghIniExt;
{
	char far *lpch;
	char *pch, *pchMac;

	if (LibGlobalReAlloc(ghIniName, (long)sizeof(grpszIniName), GMEM_MOVEABLE) == NULL)
		return;

	if (LibGlobalReAlloc(ghIniExt, (long)sizeof(grpszIniExt), GMEM_MOVEABLE) == NULL)
		return;

	lpch = (char far *)LibGlobalLock(ghIniName);

	pch = grpszIniName;
	pchMac = pch + sizeof(grpszIniName);

	while (pch < pchMac)
		*lpch++ = *pch++;

	LibGlobalUnlock(ghIniName);

	lpch = (char far *)LibGlobalLock(ghIniExt);

	pch = grpszIniExt;
	pchMac = pch + sizeof(grpszIniExt);

	while (pch < pchMac)
		*lpch++ = *pch++;

	LibGlobalUnlock(ghIniExt);
}



/* F  C O P Y  S Z  T O  G H S Z */
/*  Copy a near string to a far memory block (resizing as necessary). 
*/
FCopySzToGhsz(sz, ghsz)
char *sz;
HANDLE ghsz;
{
	char far *lpsz;

	if (LibGlobalReAlloc(ghsz, (unsigned long)strlen(sz), GMEM_MOVEABLE) == NULL)
		{
		Error(ERR_OutOfGlobalMemory, MB_OK);
		return fFalse;
		}

	lpsz = (char far *)LibGlobalLock(ghsz);
	if (lpsz == (char far *)NULL)
		{
		Error(ERR_GlobalMemory, MB_OK);
		return fFalse;
		}

	while ((*lpsz++ = *sz++) != 0)
		;

	LibGlobalUnlock(ghsz);
	return fTrue;
}


/* E R R O R */
/*  Put up an error message.
*/
Error(ErrorCode, MB_Type)
int ErrorCode;
unsigned MB_Type;
{
	extern char *ErrorMess[];

	/* note: dlls must always use system modal message boxes */
	return (LibMessageBox((HWND)NULL, (LPSTR)(ErrorMess[ErrorCode]),
			(LPSTR)"CONVTEST ERROR", MB_Type|MB_SYSTEMMODAL));
}


