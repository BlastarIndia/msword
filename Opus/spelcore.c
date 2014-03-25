#define NOVIRTUALKEYCODES
/* #define NOWINMESSAGES */
#define NONCMESSAGES
/* #define NOWINSTYLES */
#define NOSYSMETRICS
#define NODRAWFRAME
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
/* #define NOSHOWWINDOW */
#define OEMRESOURCE
#define NOSYSMETRICS
/* #define NOATOM */
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
/* #define NOCTLMGR */
#define NODRAWTEXT
#define NOFONT
/* #define NOGDI */
#define NOHDC
/* #define NOMB */
/* #define NOMEMMGR */
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
/* #define NOMSG */
/* #define NOOPENFILE */
#define NOPEN
/* #define NOPOINT */
/* #define NORECT */
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doslib.h"
#include "debug.h"
#include "error.h"
#include "field.h"
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "prompt.h"
#include "message.h"
#include "disp.h"
#include "print.h"
#include "splshare.h"
#include "wininfo.h"
#include "screen.h"
#include "idd.h"
#include "file.h"
#include "ch.h"
#include "opuscmd.h"
#include "el.h"
/* #include "sdmdefs.h"	* included in el.h */
/* #include "sdmver.h"	* included in el.h */
/* #include "sdm.h"	* included in el.h */
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "spellmm.hs"
#include "spellmm.sdm"
#include "crmgr.h"
#include "spell.h"
#include "keys.h"

#include "rareflag.h"
extern CHAR             szEmpty[];
extern int              wwCur;
extern struct SEL       selCur;
extern int              vccpFetch;
extern struct CHP       vchpFetch;
extern struct MERR      vmerr;
extern HWND             vhwndApp;
extern struct PREF	vpref;
extern struct WWD       **hwwdCur;
extern struct SPV 	*vpspv;
extern struct SPL 	*vpspl;
extern struct CA	caAdjust;
extern struct SCI	vsci;
extern struct CA	caPara;
extern CP		vcpFetch;
extern CHAR HUGE	*vhpchFetch;
extern KMP		**hkmpCur;

	typedef union {		/* LanGuage Code (designates a dict file) */
	WORD	wLgc;
	struct	
		{
		int ilng: 8;
		int fDee: 8;
		};
	} LGC;

extern LGC		lgcCur;
#define FINN		12  /* place on list of dicts in spell.c */

NATIVE void MyResetRepeatWord(); /* DECLARATION ONLY */
NATIVE unsigned long LMyLookUpWord(HANDLE,HANDLE); /* DECLARATION ONLY */
void MyGetAlternates(HANDLE);
BOOL FMySetDictAux(int, HANDLE);
BOOL FMyAddWordToDictAux(int, HANDLE);
BOOL FMyDeleteWordFromDictAux(int, HANDLE);
BOOL FMyAddWordToCacheAux(int, HANDLE, HANDLE);
HANDLE GhMySpellGetrgchCase();

csconst CHAR szWordCont[] = SzSharedKey ("...",WordCont);

struct SCD *vpscd;

NATIVE FCopySzToGhd( char *, struct GHD * ); /* DECLARATION ONLY */

/* ****
*
	Function: TmcGosubSpellMM
*  Author:
*  Copyright: Microsoft 1986
*  Date: 8/31/87
*
*  Description: Command function for "Spelling Missmatched Word" dialog
*
** ***/

/* %%Function:TmcGosubSpellMM %%Owner:bryanl */
int TmcGosubSpellMM()
{
	int tmc = tmcError;
	CHAR dlt[sizeof(dltSpellerMM)];
	DLT *pdlt = dlt;
	CABSPELLERMM *pcabspellerMM;
	HCABSPELLERMM hcabspellerMM;
	CMB cmb;

	BltDlt(dltSpellerMM, dlt);

	if ((hcabspellerMM = HcabAlloc(cabiCABSPELLERMM)) == hNil)
		return tmc;

	FSetCabSz(hcabspellerMM, szEmpty, Iag(CABSPELLERMM, hszSplMMChangeTo));

	pcabspellerMM = *hcabspellerMM;

	pcabspellerMM->iSplMMAddToBox = vrf.udcDefault;
	pcabspellerMM->fSplMMAutoSugg = vpref.fSplAutoSugg;
	pcabspellerMM->fSplMMIgnoreCaps = vrf.fSplIgnoreCaps;
	(int)(pcabspellerMM->uSplMMSuggList) = uNinchList;

	cmb.hcab = hcabspellerMM;
	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	/* fMemFail will be true if FSetCabSz fails */
	if (!vmerr.fMemFail)
		tmc = TmcOurDoDlg(dlt, &cmb);

#ifdef DEBUG
	else
		Assert(tmc == tmcError);
#endif /* DEBUG */

	FreeCab(hcabspellerMM);

	return tmc;
}


/* dialog function for MisMatched word dialog */

/* %%Function:FDlgSpellerMM %%Owner:bryanl */
int FDlgSpellerMM(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	unsigned long lResult;
	CHAR szReplaceWord[cbReplaceWordMax + 2];

	switch (dlm)
		{
		int SplIgnoreOnce();
		int SplChangeOnce();

	case dlmInit:
		AddKeyPfn(hkmpCur, KcCtrl('I'), SplIgnoreOnce);
		AddKeyPfn(hkmpCur, KcCtrl('C'), SplChangeOnce);
			{
			struct RC rcApp, rcDlg;
			HWND hwnd;
			HDLG hdlg;
			int ilng = lgcCur.ilng;

			if( ilng == FINN )
				EnableTmc(tmcSplMMSugg, fFalse);

			 /* MoveDlg not safe if dialog coming down */
			if (!FIsDlgDying())
				{
				/* position the dialog out of the way */
				GetWindowRect(vhwndApp, (LPRECT) &rcApp);
				if ((hdlg = HdlgGetCur()) != hdlgNull)
					{
					hwnd = HwndFromDlg(hdlg);
					if (hwnd != NULL)
						{
						GetWindowRect(hwnd, (LPRECT) &rcDlg);
						MoveDlg(rcDlg.xpLeft, rcApp.ypBottom - vsci.dypBdrCapt -
							rcDlg.ypBottom + rcDlg.ypTop);
						}
					}
				}
			}
		SetupMMWord();
		break;

	case dlmTerm:
		if (tmc == tmcCancel)
			vpspv->fSplCancel = fTrue;

		if (vpspv->fInScanDoc)
			{
			/* if we are canceling and scanning doc, TermScanDoc will be
			   called in FScanDoc on return from TmcOurDoDlg. This will cause
			   the screen to redraw only after the dialog has come down. bz
			*/
			if (!vpspv->fSplCancel)
				return fFalse;	/* Stay in dialog */
			}
		break;


	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (vpspv->fSeeSel)
			{
			CP cpT = vpscd->cpTestWord;

/* HACK FIX (bug 5617): for repeated word, show previous word too
	to get the full context */
			if (vpspv->LookupError == REPEAT_WORD)
				cpT = CpFirstSty( selCur.ww, selCur.doc, 
						cpT, styWord, fTrue );

			AssureCpAboveDyp(cpT, vsci.dysMacAveLine, fFalse );
			vpspv->fSeeSel = fFalse;
			}
		return fFalse;  /* keep idling */

	case dlmClick:
		if (tmc == tmcSplMMChangeTo)
			break;
		/* else fall thru to dlmChange*/

	case dlmChange:
		switch (tmc)
			{
		case tmcSplMMAddToBox:
			if ((vrf.udcDefault = ValGetTmc(tmcSplMMAddToBox)) == uNinchList)
				SetTmcVal(tmcSplMMAddToBox, vrf.udcDefault = udcUpdate);
			if (vpspv->LookupError != REPEAT_WORD)
				EnableTmc( tmcSplMMAdd, !vpspv->mpudcfRO [vrf.udcDefault] );
			break;

		case tmcSplMMAutoSugg:
			vpref.fSplAutoSugg = !vpref.fSplAutoSugg;
			SetTmcVal(tmcSplMMAutoSugg, vpref.fSplAutoSugg);
			if (FEnabledTmc(tmcSplMMSugg))
				FillSuggBox(fTrue);
			break;

		case tmcSplMMIgnoreCaps:
			vrf.fSplIgnoreCaps = !vrf.fSplIgnoreCaps;
			SetTmcVal(tmcSplMMIgnoreCaps, vrf.fSplIgnoreCaps);
			break;

		case tmcSplMMSuggList:
			if (vpspv->fNoSugg)
				{
				SetTmcVal(tmcSplMMSuggList, uNinchList);
				break;
				}
			GetTmcText(tmcSplMMSuggList, vpspv->szT, cbWordMax);
			SetTmcText(tmcSplMMChangeTo, vpspv->szT);
			FEnableMMChange();
			break;

		case tmcSplMMIgnore:
			if (vpspv->fInScanDoc)
				{
				FCopySzToGhd( vpscd->szTestWord, &vpspl->ghdWord);
#ifdef BRYANL
				CommSzSzAnsi( SzShared("Adding IGNORE word to cache: "), vpscd->szTestWord );
#endif
				if (vpspv->fInScanDoc && vpspv->LookupError != REPEAT_WORD)
					vpspv->fSplCancel = !FMyAddWordToCacheAux(spcIgnore,
							vpspl->ghdWord.ghsz, NULL );
				SplTermOrScan();
				}
			break;

		case tmcSplMMSugg:
			FillSuggBox(fTrue);
			break;

		case tmcSplMMAdd:
			FCopySzToGhd( vpscd->szTestWord, &vpspl->ghdWord);
#ifdef BRYANL
			CommSzSzAnsi( SzShared("Adding ADD word to dictionary: "), vpscd->szTestWord );
#endif
			vpspv->fSplCancel = !FMyAddWordToDictAux(ValGetTmc(tmcSplMMAddToBox),
					vpspl->ghdWord.ghsz);
			SplTermOrScan();
			break;

		case tmcSplMMChangeTo:
			FEnableMMChange();
			break;

		case tmcSplMMChange:
			FCopySzToGhd( vpscd->szTestWord, &vpspl->ghdWord );

/* long replacement, do not cache */
			GetTmcText(tmcSplMMChangeTo, szReplaceWord, cbReplaceWordMax);
			if (CchSz(szReplaceWord) >= cbWordMax)
				{
				ReplaceScanWord( szReplaceWord );
				if (vmerr.fMemFail || vmerr.fDiskFail)
					goto LCancel;
				SplTermOrScan();
				break;
				}

/* for miscapitalizion, accept the user's precise input unconditionally */
/* also do not bother to look up replacement word if it's too long */
			if ((vpspv->LookupError == CAP_REQUIRED)
					|| (vpspv->LookupError == ALLCAPS_REQUIRED)
					|| (vpspv->LookupError == IMPROP_CAP))
				{
LReplaceXact:		
				ReplaceScanWord( szReplaceWord );
				FCopySzToGhd( szReplaceWord, &vpspl->ghdReplaceWord );
#ifdef BRYANL
				CommSzSzAnsi( SzShared("Adding CHANGE word to cache: "), vpscd->szTestWord );
				CommSzSzAnsi( SzShared("  Change To: "), szReplaceWord );
#endif
/* if changing word from "Check" action, we're done */
				if (!vpspv->fInScanDoc || vmerr.fMemFail || vmerr.fDiskFail)
					goto LCancel;
/* whatever we do to repeated words, don't add it to the cache */
				if (vpspv->LookupError != REPEAT_WORD
						&& !FMyAddWordToCacheAux(spcReplaceXact,
						vpspl->ghdWord.ghsz,
						vpspl->ghdReplaceWord.ghsz))
					{
					goto LCancel;
					}
				SplTermOrScan();
				break;
				}

/* for NOT FOUND errors:
		look up the replacement word
			if found in aux cache
			use user's exact input & add it to aux cache
			else if caps error
			add corrected caps word to doc and aux cache
			else if found in dictionary
			add to dictionary with spcReplace
			else
			add to dictionary with spcReplaceXact
*/

			MyResetRepeatWord();
			lResult = LMyLookUpWord( vpspl->ghdReplaceWord.ghsz, 
					vpspl->ghdT.ghsz );
			MyResetRepeatWord();
			switch (LOWORD(lResult))
				{
			default:
				goto LReplaceXact;
			case spcNotFound:
				switch ( HIWORD(lResult) )
					{
				case ABORT_SPELLER:
					goto LCancel;
				case CAP_REQUIRED:
				case ALLCAPS_REQUIRED:
				case IMPROP_CAP:
					FCopyGhdToSz(&vpspl->ghdT, szReplaceWord);
					goto LReplaceXact;
					}
				goto LReplaceXact;

			case spcMainDict:
			case spcUserDict:
			case spcUpdateDict:
				GetTmcText(tmcSplMMChangeTo, szReplaceWord, cbWordMax);
#ifdef BRYANL
				CommSzSzAnsi( SzShared("Adding FOUND word to cache: "), vpscd->szTestWord );
				CommSzSzAnsi( SzShared("  Change To: "), szReplaceWord );
#endif
				ReplaceScanWord( szReplaceWord );
				FCopySzToGhd( szReplaceWord, &vpspl->ghdReplaceWord );
/* if changing word from "Check" action, we're done */
				if (!vpspv->fInScanDoc || vmerr.fMemFail || vmerr.fDiskFail)
					goto LCancel;
				if (vpspv->LookupError != REPEAT_WORD &&
						!FMyAddWordToCacheAux(spcReplace,
						vpspl->ghdWord.ghsz,
						vpspl->ghdReplaceWord.ghsz))
					{
LCancel:       	                
					vpspv->fSplCancel = fTrue;
					break;
					}

				break;
				}
			SplTermOrScan();
			}
		break;

	case dlmDblClk:
		if ((tmc == tmcSplMMSuggList) && vpspv->fNoSugg)
			{
			SetTmcVal(tmcSplMMSuggList, uNinchList);
			return fFalse;
			}
		break;
		}
	return (fTrue);
}


/* S p l  T e r m  O r  S c a n */
/* Scan for next word, set up dialog accordingly.  if cancelled, do nothing */
/* %%Function:SplTermOrScan %%Owner:bryanl */
SplTermOrScan()
{
	int ilng = lgcCur.ilng;
	int spc;

	if (vpspv->fInScanDoc && !vpspv->fSplCancel)
		{
		SetFocusTmc( tmcCancel );
		EnableTmc(tmcSplMMChange, fFalse);
		EnableTmc(tmcSplMMSugg, fFalse);
		EnableTmc(tmcSplMMIgnore, fFalse);
		EnableTmc(tmcSplMMAdd, fFalse);
		StartLongOp();
		spc = SpcScanDocNextWord( fTrue /*fDialog*/, fFalse );
		EndLongOp( fFalse );
		if (spc == spcDone || spc == spcAbort)
			{
			TermScanDoc( spc == spcDone );
			return;
			}
		if( ilng != FINN )
			EnableTmc(tmcSplMMSugg, fTrue);
		EnableTmc(tmcSplMMIgnore, fTrue);
		SetupMMWord();
		}
}


/* S e t u p  M M  W o r d */
/* Set up MisMatched word dialog controls based on the bad word and on the
	problem detected with it. */

/* %%Function:SetupMMWord %%Owner:bryanl */
SetupMMWord()
{
	CHAR szT[cbWordMax + 20];
	CHAR szReplaceWord[cbWordMax + 2];
	int ilng = lgcCur.ilng;

	szReplaceWord [0] = '\0';
	if (CchCopySz( vpscd->szTestWord, szT) > 30)
		CchCopySz( SzSharedKey("...",WordCont), &szT [27] );

	SetTmcText(tmcSplMMWord, szT);
	if( ilng != FINN )
		EnableTmc( tmcSplMMSugg, vpspv->LookupError != REPEAT_WORD);

	FillSuggBox( vpref.fSplAutoSugg && FEnabledTmc(tmcSplMMSugg));

	switch (vpspv->LookupError)
		{
	default:
		SetTmcText( tmcSplMMErr, SzSharedKey("Not In Dictionary:", NotInDict ));
		goto LAddOK;
	case REPEAT_WORD:
		SetTmcText( tmcSplMMErr, SzSharedKey("Repeated Word:",RepeatWord));
		EnableTmc(tmcSplMMAdd, fFalse );
		break;
	case CAP_REQUIRED:
		SetTmcText( tmcSplMMErr, SzSharedKey("First Letter Not Capitalized:", NotCap));
		goto LAlternate;
	case ALLCAPS_REQUIRED:
		SetTmcText( tmcSplMMErr, SzSharedKey("All Letters Not Capitalized:", NotAllCap));
		goto LAlternate;
	case IMPROP_CAP:
		SetTmcText( tmcSplMMErr, SzSharedKey("Uncommon Capitalization:",FreakyCap));
LAlternate:
		if (!vpref.fSplAutoSugg)
			{
			FCopyGhdToSz(&vpspl->ghdReplaceWord, szReplaceWord);
			}
LAddOK:	
		EnableTmc(tmcSplMMAdd, !vpspv->mpudcfRO [vrf.udcDefault] );
		break;
		}
	if (!vpref.fSplAutoSugg || vpspv->LookupError == REPEAT_WORD)
		SetTmcText( tmcSplMMChangeTo, szReplaceWord );
	SetFocusTmc( tmcSplMMChangeTo );
	FEnableMMChange();
	Select(&selCur,vpscd->cpTestWord, vpscd->cpLimWord );
/* store back selCur which resulted, in case AssureLegalSel actions 
   expanded the sel to include fields. */
	vpscd->cpTestWord = selCur.cpFirst;
	vpscd->cpLimWord = selCur.cpLim;
	vpspv->fSeeSel = fTrue;
}


/* F  E n a b l e  M M  C h a n g e */
/* set up state of Change button in MisMatched word dialog according
	to current contents of Change To editctl */
/* %%Function:FEnableMMChange %%Owner:bryanl */
FEnableMMChange()
{
	int f;
	CHAR szT [cbWordMax + 2];
	CHAR stT [cbWordMax + 2];

	GetTmcText(tmcSplMMChangeTo, szT, cbWordMax);
	SzToStInPlace( szT );

	SzToSt( vpscd->szTestWord, stT );
	EnableTmc( tmcSplMMChange, 
			f = FNeSt( stT, szT ) && vpscd->cpLimWord > vpscd->cpTestWord );
	SetDefaultTmc( f && szT [0] != 0 ? tmcSplMMChange : tmcSplMMIgnore );
	return f;
}


/* S p l  I g n o r e  O n c e */
/* called for Ctrl-I hack which means ignore misspelling, but do not add to cache */
/* %%Function:SplIgnoreOnce %%Owner:bryanl */
SplIgnoreOnce()
{
#ifdef BRYANL
	CommSz( SzShared( "IGNORE ONCE!!!\r\n" ) );
#endif
	if (vpspv->fInScanDoc)
		SplTermOrScan();

	if (!vpspv->fInScanDoc)	/* all done, take down dialog */
		EndDlg (tmcOK);		/* gotta do it manually cause SDM doesn't
	know about this key */
	/* this will NOT send a dlmTerm (sez bobz) */
}


/* S p l  C h a n g e  O n c e */
/* called for Ctrl-C hack (in 1.00a) which means change misspelled 
word once, i.e. do not add to cache.
*/ 
/* %%Function:SplChangeOnce %%Owner:chic */
SplChangeOnce()
{
	CHAR szReplaceWord[cbReplaceWordMax + 2];

	FCopySzToGhd( vpscd->szTestWord, &vpspl->ghdWord );
	GetTmcText(tmcSplMMChangeTo, szReplaceWord, cbReplaceWordMax);
	ReplaceScanWord(szReplaceWord);
	if (vmerr.fMemFail || vmerr.fDiskFail)
		vpspv->fSplCancel = fTrue;
	SplTermOrScan();
	if (!vpspv->fInScanDoc)	/* all done, take down dialog */
		EndDlg (tmcOK);		/* gotta do it manually cause SDM doesn't
	know about this key */
	/* this will NOT send a dlmTerm */
}


/* 
/* W  C o m b o  S p e l l  M M */
/* Combo box fill routine for user dictionary combo in Mismatched Word dialog */

/* %%Function:WComboSpellMM %%Owner:bryanl */
EXPORT WORD WComboSpellMM(tmm, sz, isz, filler, tmc, wParam)
TMM	tmm;
CHAR *	sz;
int	isz;
WORD	filler;
TMC	tmc;
WORD	wParam;
{
	extern CHAR **hszUserDict;

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		switch (isz)
			{
		case udcUpdate:
			CchCopySz(vpspv->szUpdateDict, sz);
			return fTrue;

		case udcUser:
			if (hszUserDict != hNil)
				{
				CchCopySz(*hszUserDict, sz);
				return fTrue;
				}

			/* fall thru to default */

		default:
			return fFalse;
			}

	default:
		return 0;
		}
}



/* %%Function:FCopyGhdToSz %%Owner:bryanl */
FCopyGhdToSz( pghd, sz )
struct GHD *pghd;
char *sz;
{
	CHAR FAR *lpch;
	Assert( pghd->ghsz != NULL );
	Assert( pghd->ichMax >= pghd->ichMac );
	Assert( pghd->ichMac > 0 );

	if ((lpch = GlobalLockClip( pghd->ghsz )) == NULL)
		{
		SetErrorMat( matMem );
		sz [0] = '\0';
		return fFalse;
		}
/* note: string may be larger than ichMac because entire GHD is not
	passed to speller */
	while ((*sz++ = *lpch++) != '\0')
		;
	GlobalUnlock( pghd->ghsz );
	return fTrue;
}


/* %%Function:FCopySzToGhd %%Owner:bryanl */
NATIVE FCopySzToGhd( sz, pghd )
char *sz;
struct GHD *pghd;
{
	int cch = CchSz(sz);
	CHAR FAR *lpch;

	if (pghd->ichMax == 0)
		{
		if ((pghd->ghsz = OurGlobalAlloc( GMEM_MOVEABLE, (DWORD)cch )) == NULL)
			goto LError;
		goto LNewSiz;
		}
	else  if (pghd->ichMax < cch)
		{
		if (OurGlobalReAlloc(pghd->ghsz, (DWORD)cch, GMEM_MOVEABLE) == NULL)
			{
LError:		
			SetErrorMat( matMem );
			return fFalse;
			}
LNewSiz:
		pghd->ichMax = cch;
		}

	Assert( pghd->ichMax >= cch );

	if ((lpch = GlobalLock( pghd->ghsz )) != NULL)
		{
		bltbx( (char far *)sz,lpch , pghd->ichMac = cch );
		GlobalUnlock( pghd->ghsz );
		return fTrue;
		}
	else
		return fFalse;
}


/* F i l l  S u g g  B o x */

/* %%Function:FillSuggBox %%Owner:bryanl */
FillSuggBox(fSuggest)
int fSuggest;
{
	CHAR ch;
	CHAR *pch;
	CHAR FAR *lpsz;
	HANDLE hctl;
	HANDLE hsz;
	CHAR szAlt[cbWordMax + 2];

	StartListBoxUpdate( tmcSplMMSuggList );
	if (!fSuggest)
		goto LRet;

	if ((hsz = OurGlobalAlloc( GMEM_MOVEABLE, 1L )) == NULL)
		goto LRet;

	StartLongOp();
	MyGetAlternates(hsz);
	lpsz = GlobalLock( hsz );

	if (*lpsz == 0)
		{
		AddListBoxEntry(tmcSplMMSuggList, SzSharedKey("(No Suggestions)",NoSugg));
		vpspv->fNoSugg = fTrue;

		SetDefaultTmc(tmcSplMMIgnore);
		SetTmcText(tmcSplMMChangeTo, vpscd->szTestWord);
		SetFocusTmc(tmcSplMMChangeTo);
		}
	else
		{
		for (pch = szAlt, ch = -1; ch != 0; lpsz++)
			{
			ch = *pch = *lpsz;
			if ((ch == 0) || (ch == 1))
				{
				*pch = 0;
				AddListBoxEntry(tmcSplMMSuggList, szAlt);
				pch = szAlt;
				}
			else
				pch++;
			}

		SetTmcVal(tmcSplMMSuggList, 0);
		/* get text of 1st entry */
		GetListBoxEntry(tmcSplMMSuggList, 0, szAlt, sizeof (szAlt));
		SetTmcText(tmcSplMMChangeTo, szAlt);
		vpspv->fNoSugg = fFalse;
		SetFocusTmc(tmcSplMMSuggList);
		}

	FEnableMMChange();
	EnableTmc(tmcSplMMSugg, fFalse);
	GlobalUnlock(hsz);
	EndLongOp(fFalse);
	GlobalFree(hsz);
LRet:
	EndListBoxUpdate( tmcSplMMSuggList );
}


/* FFillArrayWithSugg is similar to FillSuggBox, except that it puts
* the suggestions in the string array accessed via ad.   It puts as
* many suggestions as will fit in the array, or as many as are available.
* It returns fFalse if not enough memory, fTrue if OK.
*/
/* %%Function:FFillArrayWithSugg %%Owner:bryanl */
FFillArrayWithSugg(ad)
AD ad;
{
	extern SB sbArrays;	/* For the HpahrOfAd macro */
	CHAR ch;
	CHAR *pch;
	CHAR FAR *lpsz;
	HANDLE hsz;
	CHAR szAlt[cbWordMax + 2];
	int cadEntries;
	int cadEntriesMax;
	BOOL fRet;

	fRet = fTrue;

	if ((hsz = OurGlobalAlloc( GMEM_MOVEABLE, 1L )) == NULL)
		return fFalse;

	MyGetAlternates(hsz);

	lpsz = GlobalLock(hsz);
	Assert(lpsz != NULL);


	if (*lpsz != 0)
		{
		cadEntries = 0;
		cadEntriesMax = HpahrOfAd(ad)->rgas[0];

		for (pch = szAlt, ch = -1; ch != 0 && cadEntries < cadEntriesMax; lpsz++)
			{
			ch = *pch = *lpsz;
			if ((ch == 0) || (ch == 1))
				{
				*pch = 0;
				if ((*((SD huge *) &HpahrOfAd(ad)->rgas[1] + cadEntries++) = 
						SdCopySz(szAlt)) == sdNil)
					{
					fRet = fFalse;
					break;
					}
				pch = szAlt;
				}
			else
				pch++;
			}
		}
	GlobalUnlock(hsz);
	GlobalFree(hsz);

	return fRet;
}


/* %%Function:FScanDoc %%Owner:bryanl */
FScanDoc()
{
	int f = fTrue;
	struct CA caT;
#ifdef PROFILE
	int spc;
#endif

	InitScanDoc( fTrue );

	if (!FSetUndoB1(imiSpelling, uccPaste, PcaSetWholeDoc( &caT, selCur.doc )))
		goto LAbort;
#ifdef WINPROFILER /* not ifdef PROFILE because this causes GP faults */
	StartProfiler( 30, NULL );
	spc = SpcScanDocNextWord( fTrue/*fDialog*/, fFalse );
	StopProfiler();
	switch (spc)
#else /* !WINPROFILER */
		switch ( SpcScanDocNextWord( fTrue/*fDialog*/, fFalse ) )
#endif /* !WINPROFILER */
			{
		case spcDone:
			TermScanDoc( fTrue );
			break;
		case spcAbort:
LAbort:
			TermScanDoc( fFalse );
			f = fFalse;
			break;
		default:
			TerminateAbortCheck(pdcRestoreImmed);
			vpscd->fPrompt = fFalse;
			TmcGosubSpellMM();
/* following may occur on allocation failure because dlmTerm stuff does
	not get done. May also happen if we cancel out bz */
			if (vpspv->fInScanDoc)
				TermScanDoc( fTrue );
			break;
			}
	if (vpscd->fDirtyDoc)
		SetUndoAfter(PcaSetWholeDoc( &caT, selCur.doc ));
	return f;
}


/* %%Function:InitScanDoc %%Owner:bryanl */
InitScanDoc(fPrompt)
int fPrompt;
{
	extern struct CA caAdjust;
	struct WWD *pwwd;

	pwwd = PwwdWw(wwTemp);
	pwwd->grpfvisi = (*hwwdCur)->grpfvisi;
	pwwd->grpfvisi.fSeeHidden = fTrue;
	pwwd->grpfvisi.fForceField = fTrue;
	pwwd->grpfvisi.grpfShowResults = ~0;

	LinkDocToWw(selCur.doc, wwTemp, wwNil);

	AcquireCaAdjust();
	caAdjust = selCur.ca;

	vpscd->cpScanStart = vpscd->cpScan = 
			CpFirstSty( selCur.ww, selCur.doc, selCur.cpFirst, styWord, selCur.fInsEnd );

	if (selCur.fIns)
		vpscd->cpScanLim = (vpscd->cpScanStart == cp0) ?
				CpMacDoc(selCur.doc) : vpscd->cpScanStart;
	else
		vpscd->cpScanLim = selCur.cpLim;

	vpscd->fCheckWrap = vpscd->cpScanLim <= vpscd->cpScanStart;
	vpscd->cpScanLocalMac = vpscd->fCheckWrap ? CpMacDoc(selCur.doc) :
			vpscd->cpScanLim;
	vpscd->fPrompt = fPrompt;

	if (fPrompt)
		{
		SetPromptMst(mstSpellCheck, pdcAbortCheck);
		InitAbortCheck();
		}
	vpspv->fInScanDoc = fTrue;
	vpscd->fDirtyDoc = fFalse;
	StartLongOp();
	return fTrue;
}


/* %%Function:TermScanDoc %%Owner:bryanl */
TermScanDoc(fNoError)
int fNoError;
{
	TrashAllWws();
	Assert(caAdjust.doc == selCur.doc);
	if (caAdjust.cpFirst < cp0 || caAdjust.cpLim < cp0)
		PcaPoint(&caAdjust, caAdjust.doc, cp0);
	Select( &selCur, caAdjust.cpFirst, caAdjust.cpLim );
	NormCp(wwCur, selCur.doc, selCur.cpFirst, 3, ((*hwwdCur)->ywMac - (*hwwdCur)->ywMin) / 3, fFalse);
	TurnOnSelCur();
	if (vpscd->fPrompt)
		TerminateAbortCheck(pdcRestoreImmed);

	if (fNoError && vpscd->fPrompt)
		SetPromptMst(mstSpellDone, pdcAdvise2);

	ReleaseCaAdjust();
	vpspv->fInScanDoc = fFalse;
	EndLongOp(fFalse /* fAll */);
}



/* S p c  S c a n  D o c  N e x t  W o r d */
/* Perform next stage of scan defined by InitScanDoc.  
	Handles ignored and Replace words silently, returns in the following 
	cases:

	spcDone - scan complete
	spcAbort - user performed early abort of the scan
	other spc values: word is in error or not found

	The parameter fContAtBeginning is only checked when considering continuing
	scan at beginning of document.   If vpscd->fPrompt is false,
	fContAtBeginning is used, otherwise a dialog box is popped up to allow
	the user to choose.   The purpose of this is to allow silent (no
	dialogs) operation for macros.
*/


/* %%Function:SpcScanDocNextWord %%Owner:bryanl */
NATIVE SpcScanDocNextWord(fDialog, fNoDlgContAtBeginning)
BOOL fDialog, fNoDlgContAtBeginning;
{
	long lResult;
	int cPass = 0;
	CHAR szReplaceWord[cbWordMax + 2];

	for ( ;; )
		{
	/* every few passes, see if the user wants to abort */

		if (!(++cPass & 0x0f) && vpscd->fPrompt && FQueryAbortCheck())
			return spcAbort;

		/* if we ran off the end of the document, see if we should wrap */

		if (!FGetNextWordFromDoc())
			{{ /* !NATIVE - SpcScanDocNextWord */
			if (!vpscd->fCheckWrap)
				return spcDone;

			if (vpscd->fPrompt)
				TerminateAbortCheck(pdcRestoreImmed);

			EndLongOp(fFalse);

		/* If fPrompt is true, bring up a dialog box r.e. continue at
					* beginning.   Otherwise, use fNoDlgContAtBeginning to decide. */
			if (fDialog)
				{
			/* ask: "Continue checking at beginning of document?" */
				if (IdMessageBoxMstRgwMb( mstSpellWrapToBegin, 
						NULL, 
						MB_YESNOQUESTION) == IDNO)
					return spcAbort;
				}
			else  if (!fNoDlgContAtBeginning)
				return spcAbort;

			if (vpscd->fPrompt)
				{
				SetPromptMst(mstSpellCheck, pdcAbortCheck);
				InitAbortCheck();
				}
			StartLongOp();

			vpscd->cpScan = cp0;
			vpscd->cpScanLocalMac = vpscd->cpScanLim;
			vpscd->fCheckWrap = fFalse;
				{{ 
				continue; 
				}} /* !NATIVE - SpcScanDocNextWord */
			}}

		/* check to see if the word is beyond the scan limit */

		if (!vpscd->fCheckWrap && vpscd->cpTestWord >= vpscd->cpScanLim)
			return spcDone;


			/* we have a word to look up! */

#ifdef BRYANL
			{
			char szT [cbWordMax + 2];

			AnsiToOem( vpscd->szTestWord, szT );
			CommSzSzAnsi( SzShared("Looking up Word: "), szT );
			}
#endif
		FCopySzToGhd(vpscd->szTestWord, &vpspl->ghdWord);
LTryAgain:
		lResult = LMyLookUpWord( vpspl->ghdWord.ghsz,
				vpspl->ghdReplaceWord.ghsz);

		switch ((uns)lResult)
			{
		default:
			Assert( fFalse );
#ifdef DEBUG
		case spcMainDict:
#ifdef BRYANL
			CommSzSz( SzShared("  Result: FOUND IN MAIN"), szEmpty );
			break;
#endif
		case spcUpdateDict:
#ifdef BRYANL
			CommSzSz( SzShared("  Result: FOUND IN UPDATE"), szEmpty );
			break;
#endif
		case spcUserDict:
#ifdef BRYANL
			CommSzSz( SzShared("  Result: FOUND IN USER"), szEmpty );
			break;
#endif
		case spcIgnore:
#ifdef BRYANL
			CommSzSz( SzShared("  Result: IGNORE"), szEmpty );
			break;
#endif
#endif	/* DEBUG */
			break;

		case spcNotFound:
#ifdef BRYANL
			CommSzSz( SzShared("  Result: NOT FOUND"), szEmpty );
#endif
			if ((vpspv->LookupError = HIWORD(lResult)) == ABORT_SPELLER)
				return spcAbort;
			else  if (vpspv->LookupError == REPEAT_WORD)
				{
				CachePara( selCur.doc, vpscd->cpTestWord );
				if (caPara.cpFirst == vpscd->cpTestWord)
					{
					MyResetRepeatWord();
					goto LTryAgain;
					}
				}
			return (uns)lResult;

		case spcReplace:
		case spcReplaceXact:
			if (FCopyGhdToSz(&vpspl->ghdReplaceWord, szReplaceWord))
				{
#ifdef BRYANL
				if ((uns)lResult == spcReplace)
					CommSzSzAnsi( SzShared("  Result: REPLACE WITH: "), szReplaceWord );
				else
					CommSzSzAnsi( SzShared("  Result: REPLACE XACT WITH: "), szReplaceWord );
#endif
				ReplaceScanWord( szReplaceWord );
				}
			break;
			}
		}
	Assert( fFalse );
}



/**********************************************/
/* F  G e t  N e x t  W o r d  F r o m  D o c */
/*
	Inputs:
	vpscd - pointer to frame SCD
	vpscd->cpScan  - cp at which to start scan
	vpscd->cpScanLocalMac - return false if next word starts beyond
				this cp
	vpscd->rgchCase is filled out from the speller 

	Outputs:

	if return value is TRUE, fills out the following:
	vpscd->cpTestWord - cp of start of returned word
	vpscd->cpLimWord - cp of end of returned word
	vpscd->szTestWord - the word

	if return value is FALSE, no more words are available
*/
/* Profiling reveals that it takes 25 times longer to check the word
	than is spent in this routine, so coding this in assembler would
	not currently speed up the speller.	(bradv 3-10-89)
*/

/* %%Function:FGetNextWordFromDoc %%Owner:bryanl */
NATIVE FGetNextWordFromDoc()
{
	int ich = 0;
	uns ch;
	CHAR bCase;
	int ccp;
	CHAR HUGE *hpch;
	CHAR HUGE *hpchLim;
	char *szWord = &vpscd->szTestWord [0];

	goto LFirstRun;
	for ( ; ; )
		{
/* optimization note: segment should match; only lowwords need be compared */
		if (hpch >= hpchLim)
			{
LNextRun:	
			vpscd->cpScan = vcpFetch + ccp;
LFirstRun:	
			FetchCpPccpVisible( selCur.doc, vpscd->cpScan,
					&ccp, selCur.ww, 0);
			hpch = vhpchFetch;
			if (ccp <= 0 ||	vpscd->cpScanLocalMac <= vcpFetch)
				{	/* ran out of chars -- see if we have a word */
				if ((uns) ich - cbWordMin <= cbWordMax - cbWordMin)
					goto LHaveWord;
				return fFalse;
				}

			hpchLim = vhpchFetch + ccp;
/* treat super/subscript and fSpec as non-word text, to avoid
	deleting footnotes, pictures, etc, either as "words" by themselves
	or as imbedded in words. Same for struct thru text, cause it's not really
	there. */
			if (vchpFetch.hpsPos != 0 || vchpFetch.fSpec || vchpFetch.fStrike)
				{
				if ((uns) ich - cbWordMin <= cbWordMax - cbWordMin)
					goto LHaveWord;
				hpch = hpchLim;		/* skip run */
				goto LNewWord;
				}
			}
		ch = *hpch++;
		bCase = vpscd->rgchCase [ch];
		if (ich == 0)
			{	/* no word pending -- see if this ch starts one */
			if (bCase == CASE_NONE)
				MyResetRepeatWord();
			if (bCase & CASE_START)
				{
				vpscd->cpTestWord = vcpFetch + (hpch - vhpchFetch - 1);
				szWord [ich++] = ch;
				}
			}
		else
			{
			if (!(bCase & CASE_VALID))
				{	/* ok, we have a word... */
				hpch--;
				if ((uns)ich - cbWordMin > cbWordMax - cbWordMin)
					{	/* ...but it should be ignored */
					MyResetRepeatWord();
LNewWord:			
					ich = 0;
					continue;
					}
LHaveWord:		
				Assert( (uns)ich - cbWordMin <= cbWordMax - cbWordMin );
				vpscd->cpLimWord = 
						vpscd->cpScan = vcpFetch + (hpch - vhpchFetch);
				szWord [ich] = '\0';
				return fTrue;
				}
			if (ich <= cbWordMax - 1 &&
					(!vrf.fSplIgnoreCaps || !(bCase & CASE_UPPER)))
				{
				szWord [ich++] = ch;
				}
			else
				ich = cbWordMax + 1;	 /* means: skip this word */
			}
		}
	Assert( fFalse );
}


/* %%Function:ReplaceScanWord %%Owner:bryanl */
ReplaceScanWord( szReplaceWord )
char *szReplaceWord;
{
	CP dcpReplaceWord = (CP)(CchSz(szReplaceWord) - 1);
	CP dcpAdjust, dcpT;
	int ccp;
	struct CA caWord, caRead, caWrite, caT;
	struct CHP chp;

	PcaSet( &caWord, selCur.doc, vpscd->cpTestWord, vpscd->cpLimWord );
	AssureLegalSel(&caWord);

/* if we are deleting instead of replacing, delete trailing whitespace too */

	if (szReplaceWord [0] == '\0')
		{
		FetchCpPccpVisible (selCur.doc, caWord.cpLim, &ccp, selCur.ww, fFalse);
		if (ccp > 0 && *vhpchFetch == ' ')
			caWord.cpLim = vcpFetch + 1;
		}
	Assert( DcpCa(&caWord) > (CP) 0 );

/* Insert new word, tracking formatting along with old word */

	PcaSetDcp( &caRead, caWord.doc, caWord.cpFirst, 
			CpMin( DcpCa(&caWord), dcpReplaceWord));
	PcaPoint( &caWrite, caWord.doc, caWord.cpFirst );

	for ( ;; )
		{
		FetchCpAndPara( caRead.doc, caRead.cpFirst, fcmChars | fcmProps );
		chp = vchpFetch;
		chp.fSpec = fFalse;
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		dcpT = CpMin( (CP)vccpFetch, DcpCa(&caRead ));
		if (dcpT > cp0 &&
				FInsertRgch( caWrite.doc, caWrite.cpLim, 
				szReplaceWord, (int)dcpT, &chp, NULL /*ppap*/ ))
			{
			caWrite.cpLim += dcpT;
			caRead.cpFirst += dcpT;
			caRead.cpLim += dcpT;
			}

		szReplaceWord += (int)dcpT;
		caRead.cpFirst += dcpT;

		if (DcpCa( &caRead ) <= 0)
			{	/* write remainder & we're done */
			if ((dcpT = dcpReplaceWord - DcpCa( &caWrite )) > 0)
				{
				if (FInsertRgch(caWrite.doc, caWrite.cpLim, szReplaceWord, 
						(int) dcpT, &chp, 0))
					caWrite.cpLim += dcpT;
				}
			PcaSetDcp( &caWord, caWord.doc, caWrite.cpLim, DcpCa( &caWord ) );
			break;
			}
		}

	caT = caWord;
/* if this fails, no matter, caller will pick it up & abort the spell check */
	FDeleteRM( &caT );

	if ((dcpAdjust = dcpReplaceWord + DcpCa(&caT) - DcpCa(&caWord)) != cp0)
		{
		CP cpLimWord = caWord.cpLim + dcpAdjust - dcpReplaceWord;

		if (!vpscd->fCheckWrap)
			vpscd->cpScanLim += dcpAdjust;
		vpscd->cpScan = cpLimWord;
		vpscd->cpScanLocalMac += dcpAdjust;
		}

	vpscd->fDirtyDoc = fTrue;
}


/* Dispatch to library functions */

/* %%Function:LMyLookUpWord %%Owner:bryanl */
NATIVE unsigned long LMyLookUpWord( ghszWord, ghszWordReplace )
HANDLE ghszWord;
HANDLE ghszWordReplace;
{
	Assert( vpspl->hstack != NULL );
	return LCallOtherStack( vpspl->lpfnLLookUpWord, vpspl->hstack, &ghszWordReplace, 2 );
}


/* %%Function:MyGetAlternates %%Owner:bryanl */
void MyGetAlternates(ghszAlts)
HANDLE ghszAlts;
{
	Assert( vpspl->hstack != NULL );
	CallOtherStack( vpspl->lpfnGetAlternates, vpspl->hstack, &ghszAlts, 1 );
}


/* %%Function:FMySetDictAux %%Owner:bryanl */
BOOL FMySetDictAux(udc, ghszDictFileName)
int udc;
HANDLE ghszDictFileName;
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnFSetDictAux, vpspl->hstack, &ghszDictFileName, 2 );
}


/* %%Function:FMyAddWordToDictAux %%Owner:bryanl */
BOOL FMyAddWordToDictAux(udc, ghszWord)
int udc;
HANDLE ghszWord;
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnFAddWordToDictAux, vpspl->hstack, &ghszWord, 2 );
}


/* %%Function:FMyDeleteWordFromDictAux %%Owner:bryanl */
BOOL FMyDeleteWordFromDictAux(udc, ghszWord)
int udc;
HANDLE ghszWord;
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnFDeleteWordFromDictAux, vpspl->hstack, &ghszWord, 2 );
}



/* %%Function:FMyAddWordToCacheAux %%Owner:bryanl */
BOOL FMyAddWordToCacheAux(mode, ghszWord, ghszReplaceWord)
int mode;
HANDLE ghszWord;
HANDLE ghszReplaceWord;
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnFAddWordToCacheAux, vpspl->hstack, &ghszReplaceWord, 3 );
}


/* %%Function:GhMySpellGetrgchCase %%Owner:bryanl */
HANDLE GhMySpellGetrgchCase()
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnGhSpellGetrgchCase, vpspl->hstack, NULL, 0 );
}
