/* F I L E C V T . C */
/* Library file conversion code */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "screen.h"
#include "doc.h"
#include "pic.h"
#include "ch.h"
#include "props.h"
#include "opuscmd.h"
#include "format.h"
#include "splitter.h"
#include "print.h"
#include "doslib.h"
#include "sel.h"
#include "debug.h"
#include "file.h"
#include "inter.h"
#include "rtf.h"
#include "filecvt.h"
#include "dlbenum.h"
#include "message.h"
#include "prompt.h"
#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "vrfcnvtr.hs"
#include "vrfcnvtr.sdm"
#include "error.h"
#include "rtf.h"
#include "version.h"
#include "rareflag.h"

#ifdef PCJ
#define SHOWCVT
#define SHOWADDCVTR
#endif

/* G L O B A L S */
/* Converter information from win.ini */
struct STTB **vhsttbCvt = hNil;
int vfConversion = -1;


/* E X T E R N A L S */
extern struct MERR      vmerr;
extern struct PPR     **vhpprPRPrompt;
extern struct FLI       vfli;
extern struct PRI       vpri;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct SCI       vsci;
extern int              vwWinVersion;
extern int              vfConversion;
extern struct PREF      vpref;
extern int              vfnNoAlert;
extern struct SEL       selCur;
extern struct UAB       vuab;
extern struct DOD       **mpdochdod[];
extern struct WWD       **mpwwhwwd[];
extern struct WWD       **hwwdCur;
extern int              wwCur;
extern int              wwMac;
extern struct SEL       selCur;
extern struct MWD       **mpmwhmwd[];
extern struct MWD       **hmwdCur;
extern struct MWD       **hmwdCreate;
extern int              mwCur;
extern struct STTB **   vhsttbWnd;
extern struct RC        vrcUnZoom;
extern struct FCB       **mpfnhfcb[];
extern HWND             vhwndStatLine;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern HWND             vhwndAppIconBar;
extern HWND             vhwndDeskTop;
extern HWND             vhwndPrompt;
extern HWND             vhwndStatLine;
extern int              vflm;
extern BOOL             vfInLongOperation;
extern int              vfInitializing;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern CHAR		szApp[];
extern HBRUSH           vhbrGray;
extern struct ITR	vitr;
extern struct STTB **vhsttbCvt;


csconst CHAR	szCvtNum[]	= SzShared("CONVNUM");
csconst CHAR	szCvt[]		= SzShared("CONV");

#define cbCvtNum	sizeof(szCvtNum)
#define cbCvt		sizeof(szCvt)

#define ichMaxCvtKey (cbCvt + ichMaxNum)

CHAR *PchSkipStrictlySpacesPch(CHAR *);

struct EXCR *vpexcr;  /* pointer to currently active External Converter Record */

void AppendRgchToFnRtf(struct RARF *, char *, int);

/* F  I N I T  C V T */
/* Initialize file converter string table. */
/* %%Function:FInitCvt %%Owner:peterj */
BOOL FInitCvt()
{
	BOOL    fRetry = fTrue;

LRetry:

	if (vhsttbCvt == hNil)
		{
		int     i, iMac;
		BOOL    fValid;
		CHAR   *pchApnd, *pchApndSv;
		int     cch;
		CHAR    szKey[ichMaxCvtKey];
		CHAR    szBuf[ichMaxCvt];

		if ((vhsttbCvt = HsttbInit(0, fTrue/*fExt*/)) == hNil)
			return fFalse;

		Assert(cbCvtNum < ichMaxCvtKey);
		bltbx((CHAR FAR *) szCvtNum, (CHAR FAR *) szKey, cbCvtNum);

		iMac = GetProfileInt((CHAR FAR *) szApp, (CHAR FAR *) szKey, 0);
		bltbx((CHAR FAR *) szCvt, (CHAR FAR *) szKey, cbCvt);

		if (iMac >= 0)
			{
			pchApndSv = szKey + cbCvt - 1;
			for (i = 0; i < iMac; i++)
				{
				pchApnd = pchApndSv;
				CchIntToPpch(i + 1, &pchApnd);
				*pchApnd = '\0';
				cch = GetProfileString((CHAR FAR *) szApp, 
						(CHAR FAR *) szKey,
						(CHAR FAR *) szEmpty,
						(CHAR FAR *) szBuf, ichMaxCvt);
#ifdef SHOWCVT
				CommSzSz(SzShared("Converter String Read: "), szBuf);
#endif
				if ((cch = CchStripString(szBuf, cch)) == 0 ||
						ItMaxInSz(szBuf) < itCvtExtMin)
					{
					continue;
					}

				SzToStInPlace(szBuf);
				if (IbstAddStToSttb(vhsttbCvt, szBuf) == ibstNil)
					break;
				}
			}
		}

	if (vhsttbCvt != hNil && (*vhsttbCvt)->ibstMac == 0 && vpexcr == NULL
			&& fRetry)
		{
		BOOL fAnyFound;
		struct EXCR excr;
		fRetry = fFalse;
		InitExcr(&excr);
		fAnyFound = FFindOtherConvtrs();
		FreeExcr(&excr);
		if (fAnyFound)
			{
			FreePh(&vhsttbCvt);
			goto LRetry;
			}
		}

	return fTrue;
}



/* I T  M A X  D F F */
/* returns the number of token in converter string dff. */
/* return itNil if dff is out of range. */
/* %%Function:ItMaxDff %%Owner:peterj */
int ItMaxDff(dff)
int dff;
{
	CHAR   *pch;
	CHAR    szBuf[ichMaxCvt];

	Assert(dff >= 0);
	if (vhsttbCvt == hNil || dff >= (*vhsttbCvt)->ibstMac)
		return (itNil);

	GetSzFromSttb(vhsttbCvt, dff, szBuf);
	return (ItMaxInSz(szBuf));
}


/* I T  M A X  I N  S Z */
/* returns the number of chSpace separated words in
   sz. Assumes leading and trailing spaces in sz are
   stripped off. */
/* %%Function:ItMaxInSz %%Owner:peterj */
int ItMaxInSz(sz)
CHAR    sz[];
{
	int     itMax;
	CHAR    szBuf[ichMaxCvt];

	/* FUTURE: terribly inefficient (n2) */
	for (itMax = 0; CchCvtTknOfSz(sz, itMax, szBuf, ichMaxCvt) != 0;
			itMax++);

#ifdef DISABLE_PJ
#ifdef SHOWCVT
	CommSzNum(SzShared("itMax: "), itMax);
#endif
#endif /* DISABLE_PJ */
	return itMax;
}


/* C C H  C V T  T K N  O F  D F F */
/* Get the it-th token in converter string dff in the converter list. */
/* If such token does not exist, it returns the empty string in sz. */
/* %%Function:CchCvtTknOfDff %%Owner:peterj */
int CchCvtTknOfDff(dff, it, sz, ichMax)
int  dff;
int  it;
CHAR sz[];
int  ichMax;
{
	CHAR    szBuf[ichMaxCvt];

	Assert(dff >= 0);
	if (vhsttbCvt == hNil || dff >= (*vhsttbCvt)->ibstMac)
		{
		sz[0] = '\0';
		return (0);
		}

	GetSzFromSttb(vhsttbCvt, dff, szBuf);
	return CchCvtTknOfSz(szBuf, it, sz, ichMax);
}


/* C C H  C V T  T K N  F R O M  S Z */
/* Get the it-th token in sz taken from the converter string.       */
/* If such token does not exist, it returns the empty string in sz. */
/* %%Function:CchCvtTknOfSz %%Owner:peterj */
int CchCvtTknOfSz(szCvt, it, sz, ichMax)
CHAR *szCvt;
int  it;
CHAR sz[];
int  ichMax;
{
	CHAR   *pch, *pchTkn, *pchMac;
	int     itCur, ich;
	int     cchCopy;

	itCur = 0;
	pch = szCvt;
	do 
		{
		pchTkn = PchSkipStrictlySpacesPch(pch);
		if (*pchTkn == '\0')
			{
			goto lblRetEmpty;
			}
		if (*pchTkn == '\"')
			{
			pch = ++pchTkn;
			for ( ; ; )
				{
				pch = index(pch, '\"');
				if (pch == NULL || *(pch + 1) != '\"')
					{
					break;
					}
				pch += 2;
				}
			pchMac = pch;
			if (pch != NULL)
				{
				pch++;
				}
			}
		else
			{
			pchMac = pch = index(pchTkn, chSpace);
			}
		if (pch == NULL)
			{
			if (itCur >= it)
				{
				pchMac = szCvt + CchSz(szCvt) - 1;
				}
			else
				{
				goto lblRetEmpty;
				}
			}
		} 
	while (itCur++ < it);

	cchCopy = min(pchMac - pchTkn, ichMax-1);
	for (ich = 0; ich < cchCopy && pchTkn < pchMac; ich++, pchTkn++)
		{
		if (*pchTkn == '\"')
			{
			sz[ich] = '\"';
			pchTkn++;
			}
		else
			{
			sz[ich] = *pchTkn;
			}
		}
	sz[ich] = '\0';
#ifdef DISABLE_PJ
#ifdef SHOWCVT
	CommSzSz(SzShared("Token: "), sz);
#endif
#endif /* DISABLE_PJ */
	return (ich);

lblRetEmpty:
	sz[0] = '\0';
	return (0);
}



/* P C H  S K I P  S T R I C T L Y  S P A C E S  P C H */
/*  PchSkipSpacesPch() could not be used */
/*  because CchStipString() does not skip */
/*  tabs. */

/* %%Function:PchSkipStrictlySpacesPch %%Owner:peterj */
CHAR *PchSkipStrictlySpacesPch( pch )
CHAR *pch;
{
	for ( ;; )
		switch (*pch)
			{
		default:
			return pch;
		case ' ':
			pch++;
			break;
			}
}



/* L I B R A R Y  I N T E R F A C E  C O D E */


/* I N I T  E X C R */
/* %%Function:InitExcr %%Owner:peterj */
InitExcr(pexcr)
struct EXCR *pexcr;
{
	Assert(vpexcr == NULL);
	vpexcr = pexcr;
	SetBytes(pexcr, 0, sizeof(struct EXCR));
}


/* F R E E  E X C R */
/* %%Function:FreeExcr %%Owner:peterj */
FreeExcr(pexcr)
struct EXCR *pexcr;
{
	Assert(pexcr == vpexcr);
	if (pexcr->fInUse)
		DiscardConvtrLib();
	vpexcr = NULL;
}


/* F F N  T O  G H S Z */
/* %%Function:FFnToGhsz %%Owner:peterj */
FFnToGhsz(fn, ghsz)
int fn;
HANDLE ghsz;
{
	struct FCB *pfcb = PfcbFn(fn);

	Assert(fn != fnNil && ghsz != NULL);

	if (pfcb->osfn != osfnNil)
		FCloseDoshnd( pfcb->osfn );
	pfcb->osfn = osfnNil;

	return FCopyStToGhsz(pfcb->stFile, ghsz);
}


/* F  L O A D  C O N V T R  L I B */
/* %%Function:FLoadConvtrLib %%Owner:peterj */
FLoadConvtrLib(pszFnLib)
char *pszFnLib;
{
	extern struct MERR vmerr;
	int eid = eidConvtrNoLoad;
	HANDLE hLib;
	FARPROC lpfnInitConvtr;

	Assert(vpexcr != NULL);

#ifdef SHOWCVT
	CommSzSz(SzShared("Loading Converter: "), pszFnLib);
#endif /* SHOWCVT */

	if (vpexcr->hLib != NULL)
		{
		FreeLibrary(vpexcr->hLib);
		vpexcr->hLib = NULL;
		}

	/* reduce our swap area to allow the library in */
	if (!vpexcr->fInUse)
		{
		ShrinkSwapArea();
		}
#ifdef DEBUG
	else 		
		{
		extern int vsasCur;
		Assert(vsasCur >= sasMin);
		}
#endif /* DEBUG */
	vpexcr->fInUse = fTrue;

	vpexcr->hLib = hLib = RunApp(pszFnLib, SW_SHOW);

	if (hLib < 32)
		{
		vpexcr->hLib = NULL;
		if (hLib == 8)
			goto NoMem;
		else
			goto NoLoad;
		}

	vpexcr->lpfnFIsFormatCorrect = 
			GetProcAddress(hLib, (LPSTR)SzShared("ISFORMATCORRECT"));
	if (vpexcr->lpfnFIsFormatCorrect == 0)
		goto NoLoad;

	vpexcr->lpfnForeignToRtf = 
			GetProcAddress(hLib, (LPSTR)SzShared("FOREIGNTORTF"));
	if (vpexcr->lpfnForeignToRtf == 0)
		goto NoLoad;

	vpexcr->lpfnRtfToForeign = 
			GetProcAddress(hLib, (LPSTR)SzShared("RTFTOFOREIGN"));
	if (vpexcr->lpfnRtfToForeign == 0)
		goto NoLoad;

	vpexcr->lpfnGetIniEntry = (vwWinVersion < 0x0210) ? NULL :
			GetProcAddress(hLib, (LPSTR)SzShared("GETINIENTRY"));

	if (vpexcr->ghszFn == NULL && 
			!(vpexcr->ghszFn = GlobalAlloc(gmemLibShare, 1L)))
		goto NoMem;

	if (vpexcr->ghszSubset == NULL &&
			!(vpexcr->ghszSubset = GlobalAlloc(gmemLibShare, 1L)))
		goto NoMem;

	if (vpexcr->ghBuff == NULL &&
			!(vpexcr->ghBuff = GlobalAlloc(gmemLibShare, 1L)))
		goto NoMem;

	if (vpexcr->ghszVersion == NULL && 
			!(vpexcr->ghszVersion = GlobalAlloc(gmemLibShare, 1L)))
		goto NoMem;

	/* call converter init routine */
	/* NOTE: "INITCONVTR" is an entry point in converters pre 8/29/89
	   they expect only one word of argument.  "INITCONVERTER"
	   is the new entry point which expects two words of arguments. 
	*/
	if ((lpfnInitConvtr = 
			GetProcAddress(hLib, (LPSTR)SzShared("INITCONVERTER"))) == NULL)
		/* can't use converter */
		{
		goto NoLoad;
		}
	else
		/* new call format */
		{
		if ((vpexcr->hStack = (*lpfnInitConvtr)(HstackMyData(),vhwndApp)) == 0)
			/* init failed */
			goto NoLoad;
		}

	return fTrue;

NoMem:
	eid = eidConvtrNoLoadMem;
NoLoad:
		{
		CHAR stFile[ichMaxFile];
		SzToSt(pszFnLib, stFile);
		ErrorEidW(eid, stFile, "FLoadConvtrLib");
		}
	DiscardConvtrLib();
	return fFalse;
}


/* D I S C A R D  C O N V T R  L I B */
/* %%Function:DiscardConvtrLib %%Owner:peterj */
DiscardConvtrLib()
{
	if (vpexcr->hLib != NULL)
		FreeLibrary(vpexcr->hLib);

	if (vpexcr->ghszFn != NULL)
		GlobalFree(vpexcr->ghszFn);

	if (vpexcr->ghszSubset != NULL)
		GlobalFree(vpexcr->ghszSubset);

	if (vpexcr->ghBuff != NULL)
		GlobalFree(vpexcr->ghBuff);

	if (vpexcr->ghszVersion != NULL)
		GlobalFree(vpexcr->ghszVersion);

	Assert(vpexcr->fInUse);

	SetBytes(vpexcr, 0, sizeof(struct EXCR));

	/* reacquire the swap area we want */
	GrowSwapArea();
}


/* D I A L O G */

/* D F F  V E R I F Y  D F F */
/*  Bring up a dialog to verify that the file should be
    converted accoring to format dff.  Return dffNil on cancel or the
    dff requested.
*/

/* %%Function:DffVerifyDff %%Owner:peterj */
DffVerifyDff(dff, fn)
int dff, fn;
{
	int tmc;
	DLT *pdlt;
	char dlt[sizeof(dltVerifyConvtr)];
	HCABEXCR hcab;
	CMB cmb;

	if (vhsttbCvt == hNil)
		FInitCvt();

	BltDlt(dltVerifyConvtr, dlt);

	if ((hcab = HcabAlloc(cabiCABEXCR)) == hNil)
		return dff;

	(*hcab)->iFmt = dff - dffOpenMin;


#ifdef NOTUSED  /* feature removed bz 10/30/89 */
	(*hcab)->fConversion = fTrue;
#endif /* NOTUSED */


	/* do the dialog */
	cmb.hcab = hcab;
	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	switch (TmcOurDoDlg(dlt, &cmb))
		{
	case tmcOK:
		dff = (*hcab)->iFmt + dffOpenMin;

#ifdef NOTUSED  /* feature removed bz 10/30/89 */
		if (!(*hcab)->fConversion)
			{
			WriteProfileString(szApp, SzFrameKey("Conversion",Conversion),
					SzFrameKey("No", ConversionNo));
			vfConversion = fFalse;
			}
#endif /* NOTUSED */

		break;

	case tmcError:
	default:
		dff = dffNil;
		break;
		}

	FreeCab(hcab);

	return dff;
}



/* W  L I S T  D F F */
/* enumeration function for SaveAs and VerifyFormat dialogs
*/

/* %%Function:WListDff %%Owner:peterj */
EXPORT WORD WListDff(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	int dff, iT;
	int iSwitch = wParam==iEntblDffSave ? -dffSaveMin : -dffOpenMin;

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		if (isz < iSwitch)
			return (FEnumIEntbl(wParam, isz, sz));

		if (vhsttbCvt == hNil ||
				(dff = isz - iSwitch) >= (*vhsttbCvt)->ibstMac)
			return fFalse;

		return (CchCvtTknOfDff(dff, itCvtName, sz, ichMaxBufDlg) != 0);
		}

	return 0;
}


/* C H E C K  F O R M A T */


/* D F F  F I N D  F M T */
/* %%Function:DffFindFmt %%Owner:peterj */
DffFindFmt(fn)
int fn;
{
	extern struct STTB **vhsttbCvt;
	int dffMac;
	int dff;
	int iTkn, iTknMac;
	BOOL fRetry = fTrue;
	char szExt[cchszExtMax];
	char szConvtrPrev[ichMaxFile];
	char szExtTest[cchszExtMax];
	char szConvtr[ichMaxFile];
	char szVersion[cchMaxSz];
	struct EXCR excr;
	Debug(int cRetry = 0);

	if (vhsttbCvt == hNil || (*vhsttbCvt)->ibstMac == 0)
		{
		Assert(fFalse);/* assured by caller */
		return dffNil;
		}

	/* block: get extension of file */
		{
		struct FNS fns;
		StFileToPfns((*(mpfnhfcb[fn]))->stFile, &fns);
		StToSz(fns.stExtension, szExt);
		}

	/* set up global state */
	InitExcr (&excr);

LRetry:
	Assert(cRetry++ <= 1);
	szConvtrPrev[0] = 0;

	/* search through all dff for one with an extension that matches */
	for (dff = (dffMac = (*vhsttbCvt)->ibstMac)-1; dff >= 0; dff--)
		{
		CchCvtTknOfDff(dff, itCvtPath, szConvtr, ichMaxFile);
		if (FEqNcSz(szConvtr, szConvtrPrev))
			/* we already queried this converter, don't bother again */
			continue;

#ifdef SHOWADDCVTR
		CommSzSz(SzShared("Checking converter: "), szConvtr);
#endif /* SHOWADDCVTR */

		/* see if this dff has this extension */
		iTknMac = ItMaxDff(dff);
		for (iTkn = 2; iTkn < iTknMac; iTkn++)
			{
			CchCvtTknOfDff(dff, iTkn, szExtTest, cchszExtMax);
			if (FEqNcSz(szExt, &szExtTest[1]))
				/* extensions match, get the converter to verify */
				{
				/* remember so we don't ask twice */
				CchCopySz(szConvtr, szConvtrPrev);
				if (FTryConvtr(fn, szConvtr, szVersion))
					/* converter accepts file */
					{
					int dffT;
					if (szVersion[0])
						/* see which version they returned (if a dff) */
						for (dffT = dff; dffT >= 0; dffT--)
							{
							CHAR szVersionT[50];
							CchCvtTknOfDff(dffT, itCvtName, szVersionT, 50);
							if (FEqNcSz(szVersionT, szVersion))
								{
								CchCvtTknOfDff(dffT, itCvtPath, szConvtr, ichMaxFile);
								if (FEqNcSz(szConvtr, szConvtrPrev))
									/* we have a dff for the returned version */
									{
									dff = dffT;
									goto LRet;
									}
								}
							}
					goto LRet;
					}
				else
					break;
				}
			}
		}

	/* none matched.  see if there are converters out there that we don't
	       know about. */

	if (fRetry && FFindOtherConvtrs())
		{
		FreePh(&vhsttbCvt);
		fRetry = fFalse;
		if (FInitCvt() && vhsttbCvt != hNil && (*vhsttbCvt)->ibstMac > 0)
			goto LRetry;
		}

	dff = dffNil;
LRet:
	FreeExcr(&excr);
	return dff;
}


/* F  T R Y  C O N V T R */
/* %%Function:FTryConvtr %%Owner:peterj */
FTryConvtr(fn, pszConvtr, pszVersion)
int fn;
char *pszConvtr;
char *pszVersion;
{

	if (!FLoadConvtrLib(pszConvtr) || !FFnToGhsz(fn, vpexcr->ghszFn))
		return fFalse;

	if (CallFIsFormatCorrect(vpexcr->ghszFn, vpexcr->ghszVersion))
		{
		CopyGhszToSz(vpexcr->ghszVersion, pszVersion);
		return fTrue;
		}
	return fFalse;
}


/* C A L L  F  I S  F O R M A T  C O R R E C T */
/* %%Function:CallFIsFormatCorrect %%Owner:peterj */
CallFIsFormatCorrect(ghszFn, ghszVersion)
HANDLE ghszFn;
HANDLE ghszVersion;
{
	extern FARPROC lppFWHKey;
	extern FARPROC vlppFWHChain;

	int wRet;

	UnhookWindowsHook(WH_MSGFILTER, lppFWHKey);

    vrf.fInExternalCall = fTrue;
	wRet = WCallOtherStack(vpexcr->lpfnFIsFormatCorrect, vpexcr->hStack, 
			&ghszVersion, 2);
    vrf.fInExternalCall = fFalse;

	vlppFWHChain = SetWindowsHook(WH_MSGFILTER, lppFWHKey);
	return wRet;
}


csconst CHAR stCsConv[] = St("CONV-*");


/* F  F I N D  O T H E R  C O N V T R S */
/* %%Function:FFindOtherConvtrs %%Owner:peterj */
BOOL FFindOtherConvtrs()
{
	CHAR stConvtr[ichMaxFile];
	struct FINDFILE ffl;
	struct FNS fns, fnsT;
	int dff;
	CHAR rgchT[ichMaxFile];
	CHAR stPath[ichMaxFile];
	BOOL fSomeAdded = fFalse;

	if (FFindFileSpec(StShared("CONV-*.DLL"), stConvtr, grpfpiUtil, nfoWildOK))
		{
		StFileToPfns(stConvtr, &fns);
		/* get the path out */
		CopySt(fns.stPath, stPath);
		/* put the wildcard back */
		CopyCsSt(stCsConv, fns.stShortName);
		PfnsToStFile(&fns, stConvtr);
		StToSz(stConvtr, rgchT);
		AnsiToOem(rgchT, rgchT);

		AssertDo(FFirst(&ffl, rgchT, 0) == 0);

		for (;;)
			{
			OemToAnsi(ffl.szFileName, rgchT);
			SzToStInPlace(rgchT);
			StFileToPfns(rgchT, &fns);
			/* put in the correct path */
			CopySt(stPath, fns.stPath);
			PfnsToStFile(&fns, rgchT);

			/* normalize and break up converter name */
			if (!FNormalizeStFile(rgchT, stConvtr, nfoNormal))
				goto LNext;
			StFileToPfns(stConvtr, &fns);

#ifdef SHOWADDCVTR
			CommSzSt(SzShared("found: "), stConvtr);
#endif /* SHOWADDCVTR */

			/* see if this converter is already in WIN.INI */
			for (dff = 0; dff < (*vhsttbCvt)->ibstMac; dff++)
				{
				CHAR rgchT2[ichMaxFile];
				CchCvtTknOfDff(dff, itCvtPath, rgchT, ichMaxFile);
				SzToSt(rgchT, rgchT2);
				if (!FNormalizeStFile(rgchT2, rgchT, nfoNormal))
					continue;
				StFileToPfns(rgchT, &fnsT);
				if (FEqNcSt(fns.stShortName, fnsT.stShortName))
					goto LNext;
				}

			/* at this point we have a converter (in stConvtr) which is not
			               in our win.ini.  load it and get it's string. */

			fSomeAdded |= FAddStringsForConvtr(stConvtr);

LNext:
			if (FNext(&ffl))
				break;
			}
		}
	return fSomeAdded;
}


/* C A L L  G E T  I N I  E N T R Y */
/* %%Function:CallGetIniEntry %%Owner:peterj */
CallGetIniEntry(ghIniName, ghIniExt)
HANDLE ghIniName, ghIniExt;
{
	extern FARPROC lppFWHKey;
	extern FARPROC vlppFWHChain;

	UnhookWindowsHook(WH_MSGFILTER, lppFWHKey);

	if (vpexcr->lpfnGetIniEntry != NULL)
        {
        vrf.fInExternalCall = fTrue;
		CallOtherStack(vpexcr->lpfnGetIniEntry, vpexcr->hStack, 
				(WORD *)&ghIniExt, 2);
        vrf.fInExternalCall = fFalse;
        }

	vlppFWHChain = SetWindowsHook(WH_MSGFILTER, lppFWHKey);
}




/* F  A D D  S T R I N G S  F O R  C O N V T R */
/* %%Function:FAddStringsForConvtr %%Owner:peterj */
FAddStringsForConvtr(rgch)
CHAR *rgch;  /* pass in an st, immediately converted to sz */
{
	CHAR sz[ichMaxFile];
	HANDLE ghIniName, ghIniExt;
	CHAR FAR *lpch, FAR *lpchName, FAR *lpchExt;
	CHAR szName[40], szExt[40];
	CHAR *pch, *pchLast;
	BOOL fSomeAdded = fFalse;

#ifdef SHOWADDCVTR
	CommSzSt(SzShared("Adding strings for: "), rgch);
#endif /* SHOWADDCVTR */

	StToSzInPlace(rgch);
	if (!FLoadConvtrLib(rgch))
		return fFalse;

	ghIniName = OurGlobalAlloc(gmemLibShare, 1L);
	ghIniExt = OurGlobalAlloc(gmemLibShare, 1L);

	if (ghIniName == NULL || ghIniExt == NULL)
		goto LRet;

	lpch = GlobalLockClip(ghIniName);
	*lpch = 0;
	GlobalUnlock(ghIniName);

	lpch = GlobalLockClip(ghIniExt);
	*lpch = 0;
	GlobalUnlock(ghIniExt);

	CallGetIniEntry(ghIniName, ghIniExt);

	lpchName = GlobalLockClip(ghIniName);
	lpchExt = GlobalLockClip(ghIniExt);

	for (;;)
		{
		pch = szName;
		pchLast = pch + sizeof(szName) - 1;

		while (pch < pchLast && (*pch++ = *lpchName++) != 0)
			;
		*pch = 0;

		pch = szExt;
		pchLast = pch + sizeof(szExt) - 1;

		while (pch < pchLast && (*pch++ = *lpchExt++) != 0)
			;
		*pch = 0;

		if (szExt[0] == 0 || szName[0] == 0)
			/* done */
			break;

		/* form win.ini string="szName" szConvtr szExt */
		pch = sz;
		*pch++ = '"';
		pch += CchCopySz(szName, pch);
		*pch++ = '"';
		*pch++ = ' ';
		pch += CchCopySz(rgch, pch);
		*pch++ = ' ';
		CchCopySz(szExt, pch);

		AddConvtrString(sz);
		fSomeAdded = fTrue;
		}

	GlobalUnlock(ghIniName);
	GlobalUnlock(ghIniExt);

LRet:
	if (ghIniName != NULL)
		GlobalFree(ghIniName);
	if (ghIniExt != NULL)
		GlobalFree(ghIniExt);
	return fSomeAdded;
}


/* A D D  C O N V T R  S T R I N G */
/* %%Function:AddConvtrString %%Owner:peterj */
AddConvtrString(sz)
CHAR *sz;
{
	int iMac;
	CHAR szKey[ichMaxCvtKey];
	CHAR szNum[10];
	CHAR *pch;

#ifdef SHOWADDCVTR
	CommSzSz(SzShared("Adding INI entry: "), sz);
#endif /* SHOWADDCVTR */

	Assert(cbCvtNum < ichMaxCvtKey);
	bltbx((CHAR FAR *) szCvtNum, (CHAR FAR *) szKey, cbCvtNum);
	iMac = GetProfileInt((CHAR FAR *) szApp, (CHAR FAR *) szKey, 0);

	iMac++;
	pch = szNum;
	CchIntToPpch(iMac, &pch);
	*pch = 0;

	/* write "convnum=<iMac+1>" */
	WriteProfileString((CHAR FAR *)szApp, (CHAR FAR *)szKey,
			(CHAR FAR *)szNum);

	bltbx((CHAR FAR *) szCvt, (CHAR FAR *) szKey, cbCvt);

	pch = szKey + cbCvt - 1;
	CchIntToPpch(iMac, &pch);
	*pch = 0;

	/* write "conv<iMac+1>=<sz>" */
	WriteProfileString((CHAR FAR *)szApp, (CHAR FAR *)szKey,
			(CHAR FAR *)sz);
}


/* I N */

EXPORT FAR PASCAL ForeignToRtfIn(); /* DECLARATION ONLY */

/* D O C  C R E A T E  F N  D F F */
/* %%Function:DocCreateFnDff %%Owner:peterj */
DocCreateFnDff(fn, dff, stSubset)
int fn;
int dff;
CHAR *stSubset;
{
	int doc = docCancel;
	int wRet;
	char szConvtr[ichMaxFile];
	char szVersion[256];
	struct DOD *pdod;
	struct EXCR excr;

	StartLongOp();

	InitExcr(&excr);

	CchCvtTknOfDff(dff, 1, szConvtr, ichMaxFile);
	if (!FLoadConvtrLib(szConvtr))
		goto LDone;

	if ((vpexcr->hribl = HriblCreateDoc(docNil)) == hNil)
		goto LDone;

	pdod = PdodDoc ((*(struct RIBL **)vpexcr->hribl)->doc);
	pdod->fn = fn;
	pdod->fFormatted = fTrue;
	Assert (pdod->dk == dkDoc);

	excr.hppr = HpprStartProgressReport(mstConverting, NULL, 
			2*NIncrFromL(PfcbFn(fn)->cbMac), fTrue);

	if (!FCopyStToGhsz(stSubset, vpexcr->ghszSubset))
		goto LDone;

	CchCvtTknOfDff(dff, itCvtName, szVersion, 256);
	if (!FCopySzToGhsz(szVersion, vpexcr->ghszVersion))
		goto LDone;

	if (!FFnToGhsz(fn, vpexcr->ghszFn))
		goto LDone;

	wRet = CallForeignToRtf(vpexcr->ghszFn, vpexcr->ghBuff,
			vpexcr->ghszVersion, vpexcr->ghszSubset, (FARPROC)ForeignToRtfIn);

	if (wRet == fceUserCancel)
		vmerr.fErrorAlert = fTrue; /* supress any error message */

	if (excr.hLib != NULL)  /* give us some more room */
		{
		FreeLibrary(excr.hLib);
		excr.hLib = NULL;
		}

	doc = DocCreateRtf1(fn, vpexcr->hribl, wRet/*fFail*/);

	if (doc > 0)
		{
		ChangeProgressReport(excr.hppr, 100);
		/* files read in through external converters are considered dirty */
		DirtyDoc(doc);
		}
	/* progress report will be stopped later */

LDone:
	FreeExcr(&excr);

	EndLongOp(fFalse);

	if (doc == docCancel)
		return docNil;
	else
		return doc;
}


/* C A L L  F O R E I G N  T O  R T F */
/* %%Function:CallForeignToRtf %%Owner:peterj */
CallForeignToRtf(ghszFn, ghBuff, ghszVersion, ghszSubset, lpfnIn)
HANDLE ghszFn;
HANDLE ghBuff;
HANDLE ghszVersion;
HANDLE ghszSubset;
FARPROC lpfnIn;
{
	extern FARPROC lppFWHKey;
	extern FARPROC vlppFWHChain;

	int wRet;

	UnhookWindowsHook(WH_MSGFILTER, lppFWHKey);

    vrf.fInExternalCall = fTrue;
	wRet = WCallOtherStack(vpexcr->lpfnForeignToRtf, vpexcr->hStack, 
			(WORD *) &lpfnIn, 6);
    vrf.fInExternalCall = fFalse;

	vlppFWHChain = SetWindowsHook(WH_MSGFILTER, lppFWHKey);
	return wRet;
}


/* F O R E I G N  T O  R T F  I N */
/* %%Function:ForeignToRtfIn %%Owner:peterj */
EXPORT FAR PASCAL ForeignToRtfIn(cch, nPercentComplete)
unsigned cch;
int nPercentComplete;
{
	unsigned bRun = 0;
	unsigned cchRun;
	char far *lpTextRtf;
	char rgch[cchRTFBuffMax];

	ChangeProgressReport(vpexcr->hppr, nPercentComplete);

	while (cch != 0)
		{
		if ((lpTextRtf = GlobalLockClip(vpexcr->ghBuff)) == NULL)
			return fceNoMemory;

		cchRun = (cch > cchRTFBuffMax) ? cchRTFBuffMax : cch;

		bltbx(lpTextRtf + bRun, (char far *)&rgch, cchRun);
		GlobalUnlock(vpexcr->ghBuff);

		RtfIn(vpexcr->hribl, rgch, cchRun);

		cch -= cchRun;
		bRun += cchRun;

		if (vmerr.fMemFail)
			return fceNoMemory;
		if (vmerr.fDiskFail)
			return fceDiskError;
		if (FQueryAbortCheck())
			return fceUserCancel;
		}
	return 0;
}


/* O U T */

EXPORT FAR PASCAL GetRTFForConvtr(); /* DECLARATION ONLY */

/* F  W R I T E  F N  D S R S  F O R E I G N */
/* %%Function:FWriteFnDsrsForeign %%Owner:peterj */
BOOL FWriteFnDsrsForeign(dff, fn, doc)
int dff;
int fn;
int doc;
{
	BOOL fSuccess = fFalse;
	int wRet;
	char szConvtr[ichMaxFile];
	struct EXCR excr;
	char stJunk[ichMaxFile + 1];
	extern struct PPR **vhpprSave;

	StartLongOp();

	InitExcr(&excr);
	excr.ca.doc = doc;

	if (vhpprSave != NULL && !(*vhpprSave)->fAbortCheck)
		{
		InitAbortCheck();
		(*vhpprSave)->fAbortCheck = fTrue;
		}

	stJunk[0] = '\0';

	CchCvtTknOfDff(dff, itCvtPath, szConvtr, ichMaxFile);
	if (!FLoadConvtrLib(szConvtr))
		goto LRet;

	if ((excr.fnTempRTF = FnOpenSt(stJunk, fOstCreate, ofcTemp, NULL)) == fnNil)
		goto LRet;

	vfnNoAlert = excr.fnTempRTF;

	if (!FFnToGhsz(fn, vpexcr->ghszFn))
		goto LRet;

	CchCvtTknOfDff(dff, itCvtName, stJunk, ichMaxFile);
	if (!FCopySzToGhsz(stJunk, vpexcr->ghszVersion))
		goto LRet;

	/* since this progress report is so jerky, force it to be bigger */
	if (vhpprPRPrompt != hNil)
		(*vhpprPRPrompt)->nIncr *= 2;

	if (fSuccess = ((wRet = CallRtfToForeign(vpexcr->ghszFn, vpexcr->ghBuff, 
			vpexcr->ghszVersion, (FARPROC)GetRTFForConvtr)) == 0 
			&& !vmerr.fDiskFail && !vmerr.fMemFail))
		ReportSavePercent(cp0, (CP)1, (CP)1, fFalse);

	if (wRet == fceUserCancel)
		vmerr.fErrorAlert = fTrue; /* supress any error message */

LRet:
	if (excr.fnTempRTF != fnNil)
		DeleteFn(excr.fnTempRTF, fTrue/*fDelete*/);
	vfnNoAlert = fnNil;
	FreeExcr(&excr);
	EndLongOp(fFalse);
	return fSuccess;
}


/* C A L L  R T F  T O  F O R E I G N */
/* %%Function:CallRtfToForeign %%Owner:peterj */
CallRtfToForeign(ghszFn, ghBuff, ghszVersion, lpfnIn)
HANDLE ghszFn;
HANDLE ghBuff;
HANDLE ghszVersion;
FARPROC lpfnIn;
{
	extern FARPROC lppFWHKey;
	extern FARPROC vlppFWHChain;

	int wRet;

	UnhookWindowsHook(WH_MSGFILTER, lppFWHKey);

    vrf.fInExternalCall = fTrue;
	wRet =  WCallOtherStack(vpexcr->lpfnRtfToForeign, vpexcr->hStack, 
			(WORD *)&lpfnIn, 5);
    vrf.fInExternalCall = fFalse;

	vlppFWHChain = SetWindowsHook(WH_MSGFILTER, lppFWHKey);
	return wRet;
}


/* G E T  R T F  F O R  C O N V T R */
/* %%Function:GetRTFForConvtr %%Owner:peterj */
EXPORT FAR PASCAL GetRTFForConvtr(fPicture, nUnused)
BOOL fPicture;
int nUnused;
{
	char far *lpch;
	struct FCB **hfcb = mpfnhfcb[vpexcr->fnTempRTF];
	int cchFarBuff;
	int cchCopy;
	CP cpMac = CpMacDoc(vpexcr->ca.doc);
	FC fcMac, fcCur;
	FC fcCurT, fcMacT;
	int fn = vpexcr->fnTempRTF;
	CP cpEffective;
	int roo;

	if (OurGlobalReAlloc(vpexcr->ghBuff, (DWORD)cchRTFPassMax, 
			GMEM_MOVEABLE) == NULL)
		return fceNoMemory;

	fcMac = (*hfcb)->cbMac;
	fcCur = (*hfcb)->fcPos;
	if (fcCur == fcMac)
		{
		if (fcMac != fc0)
			{
			DeleteFnHashEntries(fn);
			(*hfcb)->fcPos = (*hfcb)->cbMac = fc0;
			}

		if ((vpexcr->ca.cpFirst = vpexcr->ca.cpLim) == cpMac)
			/* end of file reached */
			return 0;

		vpexcr->ca.cpLim = CpMin(vpexcr->ca.cpFirst+(smtMemory==smtLIM ?
				cchRTFToFnMaxEmm : cchRTFToFnMaxNE), cpMac);
#ifdef DEBUG
      /* testing: force rtf dump every 20 chars */
		if (vdbs.fCompressOften)
			vpexcr->ca.cpLim = CpMin(vpexcr->ca.cpFirst+20, cpMac);
#endif /* DEBUG */

			/* rtf start requirements */
			{
			Debug(CP cpT=vpexcr->ca.cpFirst);
			AssureLegalSel(&vpexcr->ca);
			Assert(vpexcr->ca.cpFirst == cpT);
			}

		roo = 
#ifdef SHOWCVT
				0 | 
#else
				rooInternal | 
#endif /* SHOWCVT */
				(fPicture ? 0 : rooNoPict) | 
				(vpexcr->ca.cpFirst == cp0 ? rooBegin|rooDoc : 0) |
				(vpexcr->ca.cpLim == cpMac ? rooEnd : 0);

		RtfOut(&vpexcr->ca, &AppendRgchToFnRtf, &vpexcr->fnTempRTF, roo);

		if (vmerr.fMemFail)
			return fceNoMemory;
		if (vmerr.fDiskFail)
			return fceDiskError;
		if (FQueryAbortCheck())
			return fceUserCancel;

		fcCur = (*hfcb)->fcPos = fc0;
		fcMac = (*hfcb)->cbMac;
		}

	Assert (DcpCa(&vpexcr->ca)  < 65535); /* to avoid shifts below */

	/* only report % for main doc, not subdocs */
	Assert((PdodDoc(vpexcr->ca.doc))->fMother);

	fcCurT = fcCur;
	fcMacT = fcMac;
	/* make fcMac, fcCur valid unsigneds to avoid overflow */
	while (fcMacT > 65535)
		{
		fcMacT >>= 1;
		fcCurT >>= 1;
		}
	cpEffective = vpexcr->ca.cpFirst +
			(fcMacT == (FC)0 ? (CP)0 :
			(CP)(UMultDiv((int)DcpCa(&vpexcr->ca), (int)fcCurT, (int)fcMacT)));
	ReportSavePercent(cp0, cpMac, CpMin(cpEffective, cpMac), fFalse);

	if ((lpch = GlobalLockClip(vpexcr->ghBuff)) == NULL)
		return fceNoMemory;

	for (cchFarBuff = 0; (fcCur < fcMac) && (cchFarBuff < cchRTFPassMax);
			fcCur += (FC)cchCopy, cchFarBuff += cchCopy)
		{
		if (FQueryAbortCheck() || vmerr.fMemFail || vmerr.fDiskFail)
			break;
		cchCopy = (fcMac - fcCur) > cbSector ? cbSector : (int)(fcMac - fcCur);
		/* leave this one as a bltbx since converting an lpch to
		               an hp may not work; otherwise this would be a bltbh */
		bltbx(LpFromHp(HpchFromFc(fn, fcCur)),
				(char far *)(lpch + (FC)cchFarBuff), cchCopy);
		}

	(*hfcb)->fcPos = fcCur;

	GlobalUnlock(vpexcr->ghBuff);

	if (vmerr.fMemFail)
		return fceNoMemory;
	if (vmerr.fDiskFail)
		return fceDiskError;
	if (FQueryAbortCheck())
		{
		vmerr.fErrorAlert = fTrue; /* supress error message */
		return fceUserCancel;
		}

	return cchFarBuff;
}



/* %%Function:CopyGhszToSz %%Owner:peterj */
CopyGhszToSz(ghsz, sz)
HANDLE ghsz;
CHAR *sz;
{
	CHAR FAR *lpsz;

	AssertDo( lpsz = GlobalLock(ghsz) );
	while ((*sz++ = *lpsz++) != 0)
		;

	GlobalUnlock(ghsz);
}


/* %%Function:FCopySzToGhsz %%Owner:peterj */
NATIVE FCopySzToGhsz(sz, ghsz)
CHAR *sz;
HANDLE ghsz;
{
	int cbsz;
	CHAR FAR *lpsz;

	cbsz = CchSz(sz);

	if (!OurGlobalReAlloc(ghsz, (DWORD)cbsz, GMEM_MOVEABLE))
		return fFalse;

	AssertDo( lpsz = GlobalLock(ghsz) );

	bltbx((LPSTR)sz, lpsz, cbsz);

	GlobalUnlock(ghsz);
	return fTrue;
}


/* %%Function:FCopyStToGhsz %%Owner:peterj */
FCopyStToGhsz(st, ghsz)
char st[];
HANDLE ghsz;
{
	char far *lpsz;

	if (!OurGlobalReAlloc(ghsz, (DWORD)(st[0] + 1), GMEM_MOVEABLE))
		return fFalse;

	AssertDo( lpsz = GlobalLock(ghsz) );

	bltbx((char FAR *)&st[1], lpsz, st[0]);
	*(lpsz + st[0]) = '\0';

	GlobalUnlock(ghsz);
	return fTrue;
}


