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

#include "el.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "crmgr.h"
#include "spell.h"
#include "rerr.h"
#include "opuscmd.h"
#include "rareflag.h"

#include "spell.hs"
#include "spell.sdm"


extern int vrerr;
extern BOOL		fElActive;
extern BOOL		vfRecording;
extern CHAR             szEmpty[];
extern int              wwCur;
extern struct SEL       selCur;
extern int              vccpFetch;
extern struct CHP       vchpFetch;
extern struct MERR      vmerr;
extern HWND             vhwndApp;
extern struct PREF	vpref;
extern struct CA 	caPara;
extern struct SCD 	*vpscd;

BOOL FMySpellInit(HANDLE);
BOOL FMySpellCleanUp(void);
NATIVE unsigned long LMyLookUpWord(HANDLE,HANDLE); /* DECLARATION ONLY */
NATIVE void MyResetRepeatWord(); /* DECLARATION ONLY */
void MyGetAlternates(HANDLE);
BOOL FMySetDictAux(int, HANDLE);
BOOL FMyAddWordToDictAux(int, HANDLE);
BOOL FMyDeleteWordFromDictAux(int, HANDLE);
BOOL FMyAddWordToCacheAux(int, HANDLE, HANDLE);
HANDLE GhMySpellGetrgchCase();
void MyResetRepeatWord(void);

char *PchNormDict (char * );

#define ichLexCh1	4	/* location of wildcards in lex-?? filename */
#define ichLexCh2	5
#define ichLexDee	6
#define chDee		'd'	/* "d" for default in Lex string below */

csconst CHAR szDefaultLex[]             = "lex-??d.dll";
csconst CHAR szAllLex[]                 = "lex-??.dll";
csconst CHAR szAllDic[]                 = "*.dic";
csconst CHAR szUpdateDictCS[]           = szUpdateDictDef;
			   
/* following hardcoded ordinals must match definitions in spell.def */

#define idoHStackSpell              1
#define idoFSpellInit               2
#define idoFSpellCleanup            3
#define idoLLookUpWord              4
#define idoGetAlternates            5
#define idoFSetDictAux              6
#define idoFAddWordToDictAux	    7
#define idoFDeleteWordFromDictAux   8
#define idoFAddWordToCacheAux       9
#define idoGhSpellGetrgchCase       10
#define idoResetRepeatWord          11


	typedef union {		/* LanGuage Code (designates a dict file) */
	WORD	wLgc;
	struct	
		{
		int ilng: 8;
		int fDee: 8;
		};
	} LGC;

struct LNG 
	{
	CHAR ch1;
	CHAR ch2;
	CHAR sz[];
};

csconst struct LNG rglng[] = 
	{
	0x41, 0x4d, "English (US)",	     /*  0  - American English	 */
	0x42, 0x52, "English (UK)",	     /*  1  - British English	 */
	0x46, 0x52, "Fran\347ais",	     /*  2  - French		 */
	0x47, 0x45, "Deutsch",		     /*  3  - German		 */
	0x53, 0x50, "Espa\361ol",	     /*  4  - Spanish		 */
	0x4e, 0x4c, "Nederlands",	     /*  5  - Dutch		 */
	0x49, 0x54, "Italiano", 	     /*  6  - Italian		 */
	0x53, 0x57, "Svensk",		     /*  7  - Swedish		 */
	0x46, 0x43, "Fran\347ais/Canadien",  /*  8  - French Canadian	 */
	0x44, 0x41, "Dansk",		     /*  9  - Danish		 */
	0x4e, 0x4f, "Norsk Bokm\345l",	     /*  10 - Norwegain Bokmal	 */
	0x4e, 0x4e, "Norsk Nynorsk",	     /*  11 - Norwegian Nynorsk  */
	0x46, 0x49, "Suomi",		     /*  12 - Finnish		 */
	0x50, 0x42, "Portugu\352s (BR)",	     /*  13 - Brazilian Portuguese */
	0x50, 0x54, "Portugu\352s (POR)",	     /*  14 - European Portuguese  */
	0x45, 0x41, "Macquarie",	     /*	 15 - Australian English   */
	0x43, 0x53, "Catalán"		     /*  16 - Catalan Spanish	  */
	};


#define ilngMax		17
#define ilngNil		ilngMax
#define wLgcNil		ilngMax

struct SPV 	*vpspv;
struct SPL 	*vpspl;
CHAR 		**hszUserDict = hNil;
LGC		lgcCur = { 
	0 };	/* dict to try first, to avoid slow 
	scan; assume american dict to start off */


/* ****
*
*  Spelling dialog functions
*
** ***/

/* ****
*
	Function: CmdSpelling
*  Author:
*  Copyright: Microsoft 1986
*  Date: 8/31/87
*
*  Description: Menu level command function for "Spelling" dialog
*
** ***/

/* %%Function:CmdSpelling %%Owner:bryanl */
CMD CmdSpelling(pcmb)
CMB * pcmb;
{
	int cmd = cmdOK;
	CHAR szFileT [ichMaxFile];
	CHAR dlt[sizeof(dltSpeller)];
	struct SPV spv;
	struct SPL spl;
	struct SCD scd;

	vpspv = &spv;
	vpspl = &spl;

	if (!FInitSpell())
		{
LError:	
		cmd = cmdError;
		goto LRet;
		}

/* try the current dict */
	if (!FScanForPlgc( &lgcCur ))
		{
/* no luck, scan to see what else is out there */
		lgcCur.wLgc = wLgcNil;
		if (!FScanForPlgc( &lgcCur ))
			{
			ErrorEid(eidSpellNoDict, "");
			goto LError;
			}
		}

	if (!FSpellDllLoad( lgcCur ))
		goto LError;

	vpscd = &scd;
	if (!FInitVpscd())
		goto LError;

	if (pcmb->fDefaults)
		{	/* called in both EL and user-command case */
		CABSPELLER *pcabspeller;

		CchCopySz( (vrf.udcDefault == udcUpdate || hszUserDict == hNil) ?
				spv.szUpdateDict : *hszUserDict, szFileT );

		FSetCabSz(pcmb->hcab, szFileT, Iag(CABSPELLER, hszSplADictBox));
		if (FPickOneWord(fFalse))
			FSetCabSz(pcmb->hcab, vpscd->szTestWord, Iag(CABSPELLER, hszSplWord));

		pcabspeller = (CABSPELLER *) *pcmb->hcab;
		pcabspeller->sab = 0;
		pcabspeller->fSplAutoSugg = vpref.fSplAutoSugg;
		pcabspeller->fSplIgnoreCaps = vrf.fSplIgnoreCaps;
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			{
			cmd = cmdNoMemory;
			goto LDiscardAndRet;
			}

		}

	if (!pcmb->fDialog && pcmb->fAction && pcmb->tmc == tmcSplRemove)
		{
/* special case for macro delete word from dict */
		int ilb, isz;
		int spc;
		CABSPELLER *pcab;
#ifdef BRYANL
		CommSz( SzShared( "Macro delete case reached\r\n"));
#endif
		Assert( fElActive );	/* only time this should happen */
		cmd = cmdError;

		/* set up user dictionary */

		/* verify update dictionary (STDUSER.DIC) */

		if (!FUpdateDictOK())
			goto LDiscardAndRet;

		GetCabSz( pcmb->hcab, szFileT, ichMaxFile, Iag(CABSPELLER,hszSplADictBox));
		if (!FSetUserDict( szFileT, fFalse ))
			goto LDiscardAndRet;

		if (!FNeNcSz(szFileT, vpspv->szUpdateDict))
/* have specified user dict on previous spell run, but have restored
	STDUSER.DIC as the default */
			vrf.udcDefault = udcUpdate;

		/* change to alternate speller DLL/Main dictionary if specified */

		/* call list box fill proc to fill in structures for
		   main dictionary so IlbLngFromSz will work */
		for ( isz = 0 ; 
			  WDDLBoxSpellMDict( tmmText, szFileT, isz, 0, 0, 0 );
			  isz++ )
			;
		GetCabSz( pcmb->hcab, szFileT, ichMaxFile, Iag(CABSPELLER,hszSplMDictBox));
		ilb = IlbLngFromSz( szFileT );
		if (!FSetMainDict(ilb))
			goto LDiscardAndRet;

		if (!FTellDllAboutDicts())
			goto LDiscardAndRet;

		GetCabSz( pcmb->hcab, vpscd->szTestWord, cbWordMax + 2, Iag(CABSPELLER,hszSplWord) );
		if (!FValidateWord())
			goto LDiscardAndRet;

		/* look it up to see where to delete it from, then delete if possible */

		switch (spc = SpcCheckWord())
			{
			int udc;
		default:
		case spcNotFound:	/* can't delete it if it's not there */
		case spcMainDict:	/* can't delete from main dictionary */
			goto LDiscardAndRet;

		case spcUserDict:
			udc = udcUser;
			goto LTryRemove;

		case spcUpdateDict:
			udc = udcUpdate;
LTryRemove:
			if (vpspv->mpudcfRO [udc])
				goto LDiscardAndRet;
			if (!FMyDeleteWordFromDictAux(udc, vpspl->ghdWord.ghsz))
				goto LDiscardAndRet;
			break;
			}
		cmd = cmdOK;
		goto LDiscardAndRet;
		}


	if (pcmb->fDialog || pcmb->fAction)
		{
		BltDlt(dltSpeller, dlt);

		if (TmcOurDoDlg(dlt, pcmb) != tmcError)
			{
			if (spv.fStartScan)
				FScanDoc();
			}
		else
			cmd = cmdError;
		}

LDiscardAndRet:
	DiscardSpeller();
LRet:
	/* increase the swap area size */
	GrowSwapArea();
	if (cmd == cmdOK)
		return cmdOK;

	if (fElActive)
		RtError( rerrIllegalFunctionCall );	/* does not return */
	return cmd;
}


/* ****
*
	Function: CmdSpellDoc
*  Author: Greg Cox
*  Copyright: Microsoft 1988
*  Date: 3/1/88
*
*  Description: Function Key level command function for "Spelling" dialog
*
** ***/

/* %%Function:CmdSpellDoc %%Owner:bryanl */
CMD CmdSpellDoc(pcmb)
CMB *pcmb;
{
	int cmd = cmdError;
	struct SPV spv;
	struct SPL spl;
	struct SCD scd;

	vpspv = &spv;
	vpspl = &spl;

	if (!FInitSpell())
		goto LRet;

/* try the current dict */
	if (!FScanForPlgc( &lgcCur ))
		{
/* no luck, scan to see what else is out there */
		lgcCur.wLgc = wLgcNil;
		if (!FScanForPlgc( &lgcCur ))
			{
			ErrorEid(eidSpellNoDict, "");
			goto LRet;
			}
		}

	if (!FSpellDllLoad( lgcCur ))
		goto LRet;

	if (!FUpdateDictOK() ||
			!FTellDllAboutDicts() ||
			(vpscd = &scd, !FInitVpscd()))
		goto LRet;

	cmd = cmdOK;
	if (selCur.fIns)
		{
		struct CA ca;

		if (!FPickOneWord(fTrue))
			{
			Beep();
			goto LRet0;
			}
		Select( &selCur, vpscd->cpTestWord, vpscd->cpLimWord );
		}
	FScanDoc();

LRet0:
	DiscardSpeller();

LRet:
/* increase the swap area size */
	GrowSwapArea();
	return cmd;
}


/* %%Function:FPickOneWord %%Owner:bryanl */
FPickOneWord(fIns)
int fIns;		/* whether to try to pick a word near an ins pt */
{
	CP cpFirst;

	if (!fIns && selCur.fIns)
		{
LNoWord:
		vpscd->cpTestWord = vpscd->cpLimWord = selCur.cpFirst;
		vpscd->szTestWord [0] = '\0';
		return fFalse;
		}

/* Try word starting from beginning of current sty */

	cpFirst = CpFirstSty( selCur.ww, selCur.doc, selCur.cpFirst, styWord, selCur.fInsEnd );
	vpscd->cpScan = cpFirst;
	CachePara( selCur.doc, cpFirst );
	vpscd->cpScanLocalMac = caPara.cpLim;
	if (!FGetNextWordFromDoc())
		{
/* no luck, try previous sty */
		if ((vpscd->cpScan = CpFirstSty( selCur.ww, selCur.doc, 
				selCur.cpFirst, styWord, fTrue )) >= cpFirst ||
				!FGetNextWordFromDoc())
			{
			SelectIns( &selCur, selCur.cpFirst );
			goto LNoWord;
			}
		}

/* OK, we got a word */
	Assert( vpscd->cpLimWord > vpscd->cpTestWord );

	if (!selCur.fIns && (vpscd->cpTestWord > selCur.cpFirst ||
			(selCur.cpLim > vpscd->cpLimWord  &&
			selCur.cpLim > CpLimSty( selCur.ww, selCur.doc, selCur.cpFirst,
			styWord, fFalse ))))
		{
/* but  there's more selected than just the word */
		goto LNoWord;
		}

	return fTrue;
}



/* Macro function UtilGetSpelling(FillArray$(), Word$, MainDictCode$, UserDict$)
*
* If Word$ is specified, looks up that word, otherwise searches forward in
*   the document for a possible misspelling.   Fills string array with as
*   many of the spelling suggestions as will fit.
* MainDictCode$ is an optional parameter, two letters specifying which
*   LEX-xy.DLL dictionary to use as the main dictionary.
* UserDict$ is an optional parameter, the filename of a user dictionary.
*   If it is not found, it will be created.
* fIgnoreAllCaps is an optional parameter used if Word$ not specified;
*   Ignores all caps words when scanning doc.
* fContinueAtBeginning is an optional parameter used if Word$ not specified;
*   Continues scan at beginning of document if end of doc reached.
*
* Note that no dialogs should pop up during the Macro call, and errors
* should be reported via RtError.
*/
/* %%Function:ElWUtilGetSpelling %%Owner:bryanl */
EL int ElWUtilGetSpelling(ad, hstWord, hstMainDictCode, hstSuppDict, fIgnoreAllCaps, fContinueAtBeginning)
AD ad;
char **hstWord, **hstMainDictCode, **hstSuppDict;
BOOL fIgnoreAllCaps, fContinueAtBeginning;
{
	BOOL fWordSpecified, fMisspellFound, fSplIgnoreCapsPrev;
	BOOL fCheckUserDict = fFalse;

	struct SPV spv;
	struct SPL spl;
	struct SCD scd;
	extern struct WWD **hwwdCur;
	extern struct CA caAdjust;

	CHAR szFileName[ichMaxFile];
	unsigned long lResult;
	int rerr;
	LGC lgc;

	vpspv = &spv;
	vpspl = &spl;

	lgc.wLgc = wLgcNil;
	/* Clear strings from array (also assures that array is one-dimensional) */
	Clear1DArray(ad);

	if (hstMainDictCode != hNil)
		{
		if (**hstMainDictCode < 2)
			RtError(rerrCannotOpenDictionary);

		lgc.ilng = IlngFromCh1Ch2((*hstMainDictCode)[1], (*hstMainDictCode)[2]);
		if (lgc.ilng == ilngNil || !FScanForPlgc( &lgc ))
			RtError(rerrCannotOpenDictionary);
		}

	if (!FInitSpell())
		{
		/* Something didn't work! */
LError: 
		rerr = rerrUnableToLoadSpeller;
		goto ElUGSRetErr;
		}

/* try the current dict */
	if (!FScanForPlgc( &lgc ))
		RtError(rerrCannotOpenDictionary);

	if (!FSpellDllLoad( lgc ))
		goto LError;

	if (vpscd = &scd, !FInitVpscd())
		{
		/* Something didn't work! */
		rerr = rerrUnableToLoadSpeller;
		goto ElUGSRetErr;
		}

	if (hstSuppDict != hNil)
		{
		if (**hstSuppDict >= ichMaxFile)	/* Error: supp dict file name too long */
			RtError(rerrStringTooBig);	/* does not return */

		StToSz(*hstSuppDict, szFileName);
		if ((hszUserDict = HszCreate(PchNormDict(szFileName))) == hNil)
			{
			/* Out of memory */
			rerr = rerrOutOfMemory;
			goto ElUGSRetErr;
			}

		fCheckUserDict = fTrue;
		}

	if (!FUpdateDictOK() || 
			(fCheckUserDict && !FUserDictOK(fFalse /* fDialogs */)))
		{
		rerr = rerrCannotOpenDictionary;
		goto ElUGSRetErr;
		}

	EndLongOp(fFalse);

	if (!FTellDllAboutDicts())
		{
		rerr = rerrCannotOpenDictionary;
		goto ElUGSRetErr;
		}

	fWordSpecified = (hstWord != hNil && **hstWord != 0);

	if (fWordSpecified)
		{
		if (**hstWord > cbWordMax)
			{
			rerr = rerrStringTooBig;
			goto ElUGSRetErr;
			}

		StToSz(*hstWord, vpscd->szTestWord);
		if (!FValidateWord())
			{
			rerr = rerrIllegalFunctionCall;
			goto ElUGSRetErr;
			}
		FCopySzToGhd(vpscd->szTestWord, &vpspl->ghdWord);

		MyResetRepeatWord();
		lResult = LMyLookUpWord(vpspl->ghdWord.ghsz, vpspl->ghdReplaceWord.ghsz);
		fMisspellFound = ((unsigned) lResult == spcNotFound);

		if (HIWORD(lResult) == ABORT_SPELLER)
			{
			/* Error in speller */
			DiscardSpeller();
			goto ElUGSRetErr;
			}

		StartLongOp();
		}
	else
		{
		int spc;
		/* No word specified, scan doc for possible misspelling */

		if (selCur.doc == docNil)
			ModeError();

		InitScanDoc( fFalse );

		/* Set fSplIgnoreCaps to fIgnoreAllCaps for document scan.
			* Restore previous value upon termination of scan. */
		fSplIgnoreCapsPrev = vrf.fSplIgnoreCaps;
		vrf.fSplIgnoreCaps = fIgnoreAllCaps;

		spc = SpcScanDocNextWord(fFalse, fContinueAtBeginning);

		vrf.fSplIgnoreCaps = fSplIgnoreCapsPrev;

		ReleaseCaAdjust();

		if (spc == spcAbort)
			{
			rerr = rerrOutOfMemory;
			DiscardSpeller();
			goto ElUGSRetErr;
			}

		if (fMisspellFound = (spc == spcNotFound))
			{
			/* Select possible misspelling, and bring it into view. */
			Select(&selCur, vpscd->cpTestWord, vpscd->cpLimWord);
			NormCp(wwCur, selCur.doc, vpscd->cpTestWord, 3,
					((*hwwdCur)->ywMac - (*hwwdCur)->ywMin) / 3, fFalse);
			}
		}

	/* No alternates are looked for when the document was scanned
		(i.e. !fWordSpecified) and no misspellings were found. */

	if (fWordSpecified || fMisspellFound)
		if (!FFillArrayWithSugg(ad))
			{
			rerr = rerrOutOfMemory;
			DiscardSpeller();
			goto ElUGSRetErr;
			}

	EndLongOp(fFalse);
	DiscardSpeller();

	/* increase the swap area size */
	GrowSwapArea();

	return fMisspellFound ? elFTrue : elFFalse;

ElUGSRetErr:    /* An error occurred, call RtError to return rerr. */

	GrowSwapArea();
	EndLongOp(fFalse);
	RtError(rerr);

	/* NOT REACHED */
	Assert(fFalse);
}


/* F  I n i t  S p e l l */
/* initialize spell frame vars (except vpscd, which requires the DLL, which
	is not yet loaded), and improve the global memory environment */

/* %%Function:FInitSpell %%Owner:bryanl */
FInitSpell()
{
	extern struct SCI       vsci;
	extern struct BMI       vbmiEmpty;
	extern struct PRI       vpri;

	int ilng;

	StartLongOp();

/* init *vpspv, *vpspl */
	SetBytes( vpspv, 0, sizeof (struct SPV ) );
	SetBytes( vpspl, 0, sizeof (struct SPL ) );
	bltbx((LPSTR)szUpdateDictCS, (LPSTR)vpspv->szUpdateDict, sizeof(szUpdateDictCS));
	SetWords( vpspv->mpilngiLB, ilngNil, ilngMax );

/* free up uses of global memory */
	if ((vpri.pfti != NULL) && (vpri.hdc != NULL))
		SetFlm(flmTossPrinter);
	FSetPmdcdPbmi(&vsci.mdcdScratch, &vbmiEmpty);
	if (vsci.mdcdBmp.hdc != NULL)
		FSetPmdcdPbmi(&vsci.mdcdBmp, &vbmiEmpty);
	ResetFont(fFalse /* fPrinterFont */);
	ShrinkSwapArea();

/* make sure there are non-NULL handles in the GHDs, so we don't pass a 
	null one to LMyLookUpWord */
	if (!FCopySzToGhd( szEmpty, &vpspl->ghdWord )
			|| !FCopySzToGhd( szEmpty, &vpspl->ghdReplaceWord )
			|| !FCopySzToGhd( szEmpty, &vpspl->ghdT ))
		return fFalse;
	return fTrue;
}




/* F  I n i t  V p s c d */
/* Initialize SCD structure */
/* set to all 0's except rgchCase, which is fetched from speller */

/* %%Function:FInitVpscd %%Owner:bryanl */
FInitVpscd()
{
	HANDLE ghsz;
	char far *lprgch;

	Assert( vpscd != NULL );
	Assert( sizeof (struct SCD) == 
			sizeof (int) * (sizeof (struct SCD)  / sizeof (int)));

	SetWords( vpscd, 0, sizeof (struct SCD) / sizeof (int) );
	if ((ghsz = GhMySpellGetrgchCase()) == NULL || 
			(lprgch = GlobalLockClip(ghsz)) == NULL)
		return fFalse;

	bltx(lprgch, (CHAR FAR *)&vpscd->rgchCase, 256/sizeof(int));
	GlobalUnlock(ghsz);
	GlobalFree( ghsz );
	return fTrue;
}


/* F  D l g  S p e l l e r */
/* SDM Dialog callback function for speller dialog (the one you see when
	Utilities Spell is invoked from the menu) */

/* %%Function:FDlgSpeller %%Owner:bryanl */
int FDlgSpeller(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	extern int vfAwfulNoise;
	int f;
	struct CA caT;
	CHAR szFileT [ichMaxFile];

	switch (dlm)
		{
	case dlmInit:
		vpspv->hDlgSpeller = HdlgGetCur();

		SetTmcVal(tmcSplMDictBox, 0); /* HACK: cause the list box to 
			be created so the next call will work! */
		if (fElActive)
			{
			int ilb;

			ilb = IlbLngFromSz(*((CABSPELLER *) *PcmbDlgCur()->hcab)->
					hszSplMDictBox);
			if (ilb == uNinchList)
				{
				extern int vrerr;
				vrerr = rerrIllegalFunctionCall;
				return fFalse;
				}
			SetTmcVal(tmcSplMDictBox, ilb);
			}

		LimitTextTmc(tmcSplWord, cbWordMax);
		goto LCheckSplWord;

	case dlmTerm:
		if (vpspv->fCancelTerminate)
			{
			vpspv->fCancelTerminate = fFalse;
			return fFalse;
			}
		break;

	case dlmClick:
		if (tmc == tmcSplWord)
			break;
		/* else fall thru to dlmChange*/

	case dlmChange:
		switch (tmc)
			{
		case tmcSplOptions:
			 /* FSetDlgSab not safe if dialog coming down */
			if (FIsDlgDying() || !FSetDlgSab(sabSPLOptions))
				{
				Beep();
				return fFalse;
				}
			SetFocusTmc(tmcSplMDictBox);
			EnableTmc(tmcSplMDictBox, fTrue);
			EnableTmc(tmcSplADictBox, fTrue);
			EnableTmc(tmcSplAutoSugg, fTrue);
			EnableTmc(tmcSplIgnoreCaps, fTrue);
			EnableTmc(tmcSplOptions, fFalse);
			return fFalse; /* so this will not get recorded */


		case tmcSplMDictBox:
			if (ValGetTmc(tmcSplMDictBox) == uNinchList)
				SetTmcVal(tmcSplMDictBox, 0);
			break;

		case tmcSplWord:
LCheckSplWord:
			GetTmcText(tmcSplWord, vpscd->szTestWord, cbWordMax + 2);
LHighLt:
			f = FValidateWord();
			EnableTmc(tmcSplCheck, f);
			EnableTmc(tmcSplRemove, f);
			SetDefaultTmc( f ? tmcSplCheck : tmcSplStart );
			break;

		case tmcSplCheck:
			StartLongOp();

			if (!FSetUndoB1(imiSpelling, uccPaste, 
					PcaSetWholeDoc( &caT, selCur.doc )))
				{
LCancel:
				vpspv->fCancelTerminate = fTrue;
				EndLongOp(fFalse);
				break;
				}

			if (!FDlgCheckDicts())
				goto LCancel;

			switch (SpcCheckWord())
				{
			case spcNotFound:
				if (vpspv->LookupError == ABORT_SPELLER)
					{
LCancel1:			
					EndDlg(tmcOK);   /* ok to call EndDlg here */

					EndLongOp(fFalse);
					return (fTrue);
					}

				TmcGosubSpellMM();
				if (vpscd->fDirtyDoc)
					{
					SetUndoAfter(PcaSetWholeDoc( &caT, selCur.doc ));
					/* HACK: to get the spelling call recorded - only case where
					   we "cancel" the operation (after "change" we go straight
						back to the document) but we still want to record the
						spelling call. */
					if (vfRecording)
						FRecordCab(HcabFromDlg(fFalse), IDDSpeller, tmcOK, fFalse);
					goto LCancel1;
					}
				SetFocusTmc(tmcSplWord);
				if (vpspv->fSplCancel)
					goto LCancel1;
				SetTmcVal(tmcSplAutoSugg, vpref.fSplAutoSugg);
				SetTmcVal(tmcSplIgnoreCaps, vrf.fSplIgnoreCaps);
				SetFocusTmc(tmcSplWord);
				break;

			case spcMainDict:
				/* display: "Word was found in Main dictionary" */
				ErrorEid(eidSpellWdFndMain, "");
				break;

			case spcUpdateDict:
				/* display: "Word was found in Main Update dictionary" */
				ErrorEid(eidSpellWdFndUpdate, "");
				break;

			case spcUserDict:
				/* display: "Word was found in User dictionary" */
				ErrorEid(eidSpellWdFndAux, "");
				break;

			default:
				break;
				}

			EndLongOp(fFalse);
			break;

		case tmcSplRemove:
			if (!FDlgCheckDicts())
				goto LCancel;
			switch (SpcCheckWord())
				{
				int udc;
			case spcNotFound:
				/* display: "Word was not found" */
				ErrorEid(eidSpellNotFound, "");
				break;

			case spcMainDict:
LNoRemove:		/* display: "Cannot remove word from dictionary" */
				ErrorEid(eidSpellDictRO, "");
				break;

			case spcUpdateDict:
				udc = udcUpdate;
LTryRemove:
				if (vpspv->mpudcfRO [udc])
					goto LNoRemove;
				if (!FMyDeleteWordFromDictAux(udc, vpspl->ghdWord.ghsz))
					{
					EndDlg(tmcOK);    /* ok to call EndDlg here */

					return (fTrue);
					}
				break;

			case spcUserDict:
				udc = udcUser;
				goto LTryRemove;

			default:
				break;
				}
			SetFocusTmc(tmcSplWord);
			break;

		case tmcSplAutoSugg:
			vpref.fSplAutoSugg = !vpref.fSplAutoSugg;
			SetTmcVal(tmcSplAutoSugg, vpref.fSplAutoSugg);
			break;

		case tmcSplIgnoreCaps:
			vrf.fSplIgnoreCaps = !vrf.fSplIgnoreCaps;
			SetTmcVal(tmcSplIgnoreCaps, vrf.fSplIgnoreCaps);
			break;

		case tmcSplStart:
			if (!FDlgCheckDicts())
				break;
			vpspv->fStartScan = fTrue;
			break;
			}
		break;
		}
	return (fTrue);
}


/* %%Function:FValidateWord %%Owner:bryanl */

FValidateWord()
{
	int ich, ichMac;
	char *pch;
/* check word for validity.
	This is more than just a user convenience, speller code
	will crash if asked for alternates for a word consisting of
	punctuation (e.g. ".,.") */

	if ((ichMac = CchSz(vpscd->szTestWord)-1) < cbWordMin ||
					ichMac > cbWordMax)
		{
		return fFalse;
		}

	pch = vpscd->szTestWord;
	if (!(vpscd->rgchCase [*pch++] & CASE_START))
		return fFalse;
	for ( ich = 1 ; ich < ichMac ; ich++ )
		if (!(vpscd->rgchCase [*pch++] & CASE_VALID))
			return fFalse;
	return fTrue;
}




/* %%Function:FDlgCheckDicts %%Owner:bryanl */
FDlgCheckDicts()
{
	int ilb;
	CHAR szDict [ichMaxFile];

	if (!FUpdateDictOK())
		{
		if (FIsVisibleTmc( tmcSplMDictBox ))
			SetFocusTmc( tmcSplMDictBox );
		goto LCancel;
		}

	GetTmcText(tmcSplADictBox, szDict, ichMaxFile);
	if (!FSetUserDict( szDict, fTrue ))
		{
		if (FIsVisibleTmc( tmcSplADictBox ))
			SetFocusTmc( tmcSplADictBox );
LCancel:
		vpspv->fCancelTerminate = fTrue;
		return fFalse;
		}

	if (!FNeNcSz(szDict, vpspv->szUpdateDict))
/* have specified user dict on previous spell run, but have restored
	STDUSER.DIC as the default */
			vrf.udcDefault = udcUpdate;

	if ((ilb = ValGetTmc( tmcSplMDictBox )) == uNinchList)
		SetTmcVal( tmcSplMDictBox, ilb = 0 );
	if (!FSetMainDict( ilb ))
		goto LCancel;
	if (!FTellDllAboutDicts())
		goto LCancel;

	return fTrue;
}



/* %%Function:SpcCheckWord %%Owner:bryanl */
int SpcCheckWord()
{
	unsigned long lResult;
	CHAR szFileT [ichMaxFile];

	FCopySzToGhd( vpscd->szTestWord, &vpspl->ghdWord );
	MyResetRepeatWord();
	lResult = LMyLookUpWord(vpspl->ghdWord.ghsz,
			vpspl->ghdReplaceWord.ghsz);

	MyResetRepeatWord();
	vpspv->LookupError = HIWORD(lResult);
	Assert( vpspv->LookupError == 0 || LOWORD(lResult) == spcNotFound );
	return LOWORD(lResult);
}


/* %%Function:FSetUserDict( szDict ) */
FSetUserDict( szDict, fDialogs )
char *szDict;
int fDialogs;	/* whether to display query dialogs; false is for EL */
{
	PchNormDict( szDict );
	if (FNeNcSz(szDict, vpspv->szUpdateDict))
		{
		CHAR stT [ichMaxFile];

		SzToStInPlace(szDict);
		if (!FValidFilename( szDict, stT, NULL, NULL, NULL, nfoNormal ))
			{
			ErrorEid(eidBadFileDlg, "FDlgSpellerOK");
			return fFalse;
			}
		StToSzInPlace(szDict);
		FreePh( &hszUserDict );
		/* don't worry abt failure, no real nasty consequences */
		hszUserDict = HszCreate( szDict );
		}
	return FUserDictOK( fDialogs );
}

FSetMainDict( ilb )
int ilb;	/* index into "list box" */
{
	LGC lgc;
	int ilng;
#ifdef NOASM
	NATIVE IScanLprgw(); /* DECLARATION ONLY */
#endif /* NOASM */

	if ((uns)ilb >= ilngMax)
		goto LError;

	ilng = IScanLprgw( (int far *) vpspv->mpilngiLB, ilb, ilngMax );

	if ((uns)ilng >= ilngMax)
		goto LError;

	if (ilng != lgcCur.ilng)
		{
#ifdef BRYANL
		CommSzNum( SzShared( "Speller ilng changed to: "),ilng );
#endif
		SpellDllFree();
		lgc.ilng = ilng;
		lgc.fDee = vpspv->mpilngfDee [ilng];
		if (!FSpellDllLoad(lgc))
			{
LError:		ErrorEid(eidSpellNoDict, "");
			SpellDllFree();
			return fFalse;
			}
		}
	return fTrue;
}


/* %%Function:FSpellDllLoad %%Owner:bryanl */
FSpellDllLoad(lgc)
LGC lgc;
{
	char *PchAdjustDictFileName();
	HANDLE hLib;
	BOOL fResult;
	int iMDictBox;
	int cbDictPath;
	CHAR stLex [ichMaxFile];
	CHAR szFileName[ichMaxFile];
	CHAR szDictPath[ichMaxFile];

	lgcCur.wLgc = wLgcNil;
	LgcToStLex( lgc, stLex );
	StToSzInPlace( stLex );
#define szLex	stLex
	vpspl->hLib = hLib = LoadLibrary( (LPSTR)PchAdjustDictFileName(szLex));
#undef szLex

	if (hLib < 32)
		{
		vpspl->hLib = (HANDLE)NULL;
		if (hLib == 8)
			goto NoMem;
		else
			goto NoLoad;
		}

	vpspl->lpfnHStackSpell = GetProcAddress(hLib, MAKEINTRESOURCE( idoHStackSpell));
	if (vpspl->lpfnHStackSpell == 0)
		goto NoLoad;

	vpspl->lpfnFSpellInit = GetProcAddress(hLib, MAKEINTRESOURCE( idoFSpellInit));
	if (vpspl->lpfnFSpellInit == 0)
		goto NoLoad;

	vpspl->lpfnFSpellCleanUp = GetProcAddress(hLib, MAKEINTRESOURCE( idoFSpellCleanup));
	if (vpspl->lpfnFSpellCleanUp == 0)
		goto NoLoad;

	vpspl->lpfnLLookUpWord = GetProcAddress(hLib, MAKEINTRESOURCE( idoLLookUpWord));
	if (vpspl->lpfnLLookUpWord == 0)
		goto NoLoad;

	vpspl->lpfnGetAlternates = GetProcAddress(hLib, MAKEINTRESOURCE( idoGetAlternates));
	if (vpspl->lpfnGetAlternates == 0)
		goto NoLoad;

	vpspl->lpfnFSetDictAux = GetProcAddress(hLib, MAKEINTRESOURCE( idoFSetDictAux));
	if (vpspl->lpfnFSetDictAux == 0)
		goto NoLoad;

	vpspl->lpfnFAddWordToDictAux = GetProcAddress(hLib, MAKEINTRESOURCE( idoFAddWordToDictAux));
	if (vpspl->lpfnFAddWordToDictAux == 0)
		goto NoLoad;

	vpspl->lpfnFDeleteWordFromDictAux = GetProcAddress(hLib, MAKEINTRESOURCE( idoFDeleteWordFromDictAux));
	if (vpspl->lpfnFDeleteWordFromDictAux == 0)
		goto NoLoad;

	vpspl->lpfnFAddWordToCacheAux = GetProcAddress(hLib, MAKEINTRESOURCE( idoFAddWordToCacheAux));
	if (vpspl->lpfnFAddWordToCacheAux == 0)
		goto NoLoad;

	vpspl->lpfnGhSpellGetrgchCase = GetProcAddress(hLib, MAKEINTRESOURCE( idoGhSpellGetrgchCase));
	if (vpspl->lpfnGhSpellGetrgchCase == 0)
		goto NoLoad;

	vpspl->lpfnResetRepeatWord = GetProcAddress(hLib, MAKEINTRESOURCE( idoResetRepeatWord));
	if (vpspl->lpfnResetRepeatWord == 0)
		goto NoLoad;

	if ((vpspl->hstack = (*vpspl->lpfnHStackSpell)()) == NULL)
		goto NoLoad;

	StToSzInPlace( vpspl->stPath );
	FCopySzToGhd(vpspl->stPath, &vpspl->ghdT);
	SzToStInPlace( vpspl->stPath );
	if (!FMySpellInit(vpspl->ghdT.ghsz))
		goto ErrorExit;

	lgcCur = lgc;

	EndLongOp(fFalse);
	return fTrue;

NoMem:
	ErrorEid(eidSpellNoLoadMem, "");
	goto ErrorExit;

NoLoad:
	ErrorEid(eidSpellNoLoad, "");

ErrorExit:
	DiscardSpeller();

	EndLongOp(fFalse);
	return fFalse;
}


/* %%Function:FTellDllAboutDicts %%Owner:bryanl */
FTellDllAboutDicts()
{
	char *PchAdjustDictFileName();
	int ilng, iMDictBox;
	CHAR szFileName[ichMaxFile];

	StartLongOp();
	CchCopySz(vpspv->szUpdateDict, szFileName);
	FCopySzToGhd(PchAdjustDictFileName(szFileName), &vpspl->ghdT);
	if (!FMySetDictAux(udcUpdate, vpspl->ghdT.ghsz))
		goto ErrorExit;

	if (hszUserDict != hNil)
		{
		CchCopySz( *hszUserDict, szFileName );
		PchAdjustDictFileName(szFileName);
		}
	else
		szFileName[0] = 0;

	FCopySzToGhd(szFileName, &vpspl->ghdT);
	if (!FMySetDictAux(udcUser, vpspl->ghdT.ghsz))
		goto ErrorExit;
	EndLongOp(fFalse);

	return fTrue;

ErrorExit:
	DiscardSpeller();
	EndLongOp(fFalse);
	return fFalse;
}






/* %%Function:DiscardSpeller %%Owner:bryanl */
DiscardSpeller()
{
	StartLongOp();

	FreeGhd( &vpspl->ghdWord );
	FreeGhd( &vpspl->ghdReplaceWord );
	FreeGhd( &vpspl->ghdT );

	SpellDllFree();
	EndLongOp(fFalse);
}



/* %%Function:FreeGhd %%Owner:bryanl */
FreeGhd( pghd )
struct GHD *pghd;
{
	if (pghd->ichMax > 0)
		{
		Assert( pghd->ghsz != NULL );
		GlobalFree( pghd->ghsz );
		pghd->ichMax = 0;
		}
}



/* %%Function:PchAdjustDictFileName %%Owner:bryanl */
char *PchAdjustDictFileName(szFileName)
CHAR szFileName[];
{
	CHAR szDictPath [ichMaxFile];
	int cbDictPath;
	int isz;
	CHAR ch;
	struct FNS fns;

	SzToStInPlace( szFileName );
#define stFileName szFileName
	StFileToPfns( stFileName, &fns );
	if (fns.stPath [0] == 0)
		CopySt( vpspl->stPath, fns.stPath );
	if (fns.stExtension [0] == 0)
		SzToSt( SzSharedKey( ".DIC", DicExt), fns.stExtension );
	PfnsToStFile( &fns, stFileName );
	StToSzInPlace( stFileName );
#undef stFileName
	return szFileName;
}


/* %%Function:PchNormDict %%Owner:bryanl */
char *PchNormDict( szDict )
char *szDict;
{
	struct FNS fns;

	SzToStInPlace( szDict );
	StFileToPfns( szDict, &fns );
	if (!FNeNcSt( fns.stPath, vpspl->stPath ))
		fns.stPath [0] = 0;
	if (fns.stExtension [0] == 0)
		SzToSt( SzSharedKey( ".DIC", DicExt), fns.stExtension );
	PfnsToStFile( &fns, szDict );
	StToSzInPlace( szDict );
	return szDict;
}


/* %%Function:FUpdateDictOK %%Owner:bryanl */
static BOOL FUpdateDictOK()
{
	CHAR szFileName[ichMaxFile];
	OFSTRUCT ofs;
	int osfn;

/* Check whether STDUSER.DIC exists, create it silently if not */

	vrf.udcDefault = udcUpdate;
	if (!FTryDict( vpspv->szUpdateDict, szFileName, &vpspv->mpudcfRO [udcUpdate] ))
		{
		if (!FCreateDict( szFileName ))
			{
			ErrorEid(eidSpellUpdateFail, "FUpdateDictOK");
			return fFalse;
			}
		}

	return fTrue;
}


/* %%Function:FUserDictOK %%Owner:bryanl */
static BOOL FUserDictOK(fDialogs)
BOOL fDialogs;
{
	CHAR szFileName[ichMaxFile];
	OFSTRUCT ofs;
	int osfn;

/* Check whether user-specified dictionary exists, prompt for creation if not. */

	if (hszUserDict == hNil)
		return fTrue;

	CchCopySz(*hszUserDict, szFileName);

	Assert( FNeNcSz( szFileName, vpspv->szUpdateDict ) );

	if (!FTryDict( szFileName, szFileName, &vpspv->mpudcfRO [udcUser] ))
		{
	/* ask: "User dictionary not found. Create?" */
	/* If !fDialogs, assume Yes */
		if (!fDialogs || IdMessageBoxMstRgwMb(mstSpellCreateDict, NULL, 
				MB_DEFYESQUESTION) == IDYES)
			{
			if (!FCreateDict( szFileName ))
				{
				ErrorEid(eidBadFileDlg, "FUserDictOK");
				goto LError;
				}
			}
		else
			{
LError: 	
			FreePh(&hszUserDict);
			return fFalse;
			}
		}

	vrf.udcDefault = udcUser;
	return fTrue;
}


/* %%Function:FTryDict %%Owner:bryanl */
static FTryDict( szFile, szAdjust, pfRO )
char *szFile;
char *szAdjust;
int *pfRO;
{
	OFSTRUCT ofs;

	*pfRO = fFalse;
	CchCopySz( szFile, szAdjust );
	if (OpenFile( (LPSTR)PchAdjustDictFileName(szAdjust), (LPOFSTRUCT)&ofs, 
			OF_READWRITE | OF_EXIST) != -1)
		return fTrue;

	if (OpenFile( (LPSTR)szAdjust, (LPOFSTRUCT)&ofs, OF_READ | OF_EXIST) != -1)
		{
		*pfRO = fTrue;
		return fTrue;
		}
	return fFalse;
}


/* %%Function:FCreateDict %%Owner:bryanl */
static FCreateDict( szFile )
char *szFile;
{
	OFSTRUCT ofs;
	int osfn;

	if ((osfn = OpenFile( (LPSTR)szFile, (LPOFSTRUCT)&ofs,
			OF_READWRITE | OF_CREATE)) == -1)
		return fFalse;

	FCloseDoshnd(osfn);
	return fTrue;
}


/* W  C o m b o  S p e l l  A D i c t */
/* list box enumeration callback function for user dictionary list */

/* %%Function:WComboSpellADict %%Owner:bryanl */
EXPORT WORD WComboSpellADict(tmm, sz, isz, filler, tmc, wParam)
TMM	tmm;
CHAR *	sz;
int	isz;
WORD	filler;
TMC	tmc;
WORD	wParam;
{
	int RetCod;
	CHAR szDictPath [ichMaxFile];
	CHAR szFile [ichMaxFile];
	int cbDictPath;

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
			{
			switch (vpspv->StatLdADictFn)
				{
			case 0:
				CchCopySz(vpspv->szUpdateDict, sz);
				vpspv->StatLdADictFn++;
				return fTrue;

			case 1:
				if (hszUserDict != hNil)
					{
					CchCopySz(*hszUserDict, sz);
					vpspv->StatLdADictFn++;
					return fTrue;
					}
				vpspv->StatLdADictFn++;

			case 2:
				Assert( vpspl->stPath [0] < ichMaxPath );
				StToSz( vpspl->stPath, szDictPath );
				cbDictPath = vpspl->stPath [0];
				bltbx((LPSTR)szAllDic,(LPSTR)(&szDictPath[cbDictPath]),sizeof(szAllDic));
				RetCod = FFirst(&(vpspv->FindFile), szDictPath, 1);
				vpspv->StatLdADictFn++;
				goto TryThis;

			default:
				while (fTrue)
					{
					RetCod = FNext(&(vpspv->FindFile));
TryThis:
					if (RetCod == 0)
						{
						CchCopySz( vpspv->FindFile.szFileName, szFile );
						OemToAnsi( szFile, szFile );
						AnsiUpper( szFile );
						if (FEqNcSz(szFile, vpspv->szUpdateDict))
							continue;
						if ((hszUserDict != hNil) &&
								FEqNcSz(szFile, *hszUserDict))
							continue;

						CchCopySz(szFile, sz);
						return fTrue;
						}
					else
						return fFalse;
					}
				}
			}

	default:
		return 0;
		}
}


/* Fill combo box in Util Spelling Dialog with language names for
	all DLL's found on the disk that match szLexDefault or szLexAll */


/* %%Function:WDDLBoxSpellMDict %%Owner:bryanl */
EXPORT WORD WDDLBoxSpellMDict(tmm, sz, isz, filler, tmc, wParam)
TMM	tmm;
CHAR *	sz;
int	isz;
WORD	filler;
TMC	tmc;
WORD	wParam;
{
	static BOOL fScanningDee;
	int fNotFound;
	int cbDictPath;
	int ilng;
	CHAR szDictPath[ichMaxFile];

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
			{
			switch (isz)
				{
			case 0:
				ilng = lgcCur.ilng;
				Assert( (uns) ilng < ilngMax );
				vpspv->mpilngiLB [ilng] = 0;
				bltbx((LPSTR)(rglng [ilng].sz), (LPSTR)sz, cbszLangMax);
				return fTrue;

			case 1:
				fScanningDee = fTrue;
				Assert( vpspl->stPath [0] < ichMaxPath );
				StToSz( vpspl->stPath, szDictPath );
				cbDictPath = vpspl->stPath [0];
				bltbx((LPSTR)szDefaultLex,(LPSTR)(&szDictPath[cbDictPath]),sizeof(szDefaultLex));
				fNotFound = FFirst(&(vpspv->FindFile), szDictPath, 1);
				goto CheckFile;

			default:
				while (fTrue)
					{
					fNotFound = FNext(&(vpspv->FindFile));
CheckFile:
					if ((fNotFound != 0) && fScanningDee)
						{
						fScanningDee = fFalse;
						Assert( vpspl->stPath [0] < ichMaxPath );
						StToSz( vpspl->stPath, szDictPath );
						cbDictPath = vpspl->stPath [0];
						bltbx((LPSTR)szAllLex,(LPSTR)(&szDictPath[cbDictPath]),sizeof(szAllLex));
						fNotFound = FFirst(&(vpspv->FindFile), szDictPath, 1);
						}

					if (fNotFound != 0)
						return fFalse;

					if ((ilng = ILngFromCh1Ch2(
							vpspv->FindFile.szFileName [ichLexCh1],
							vpspv->FindFile.szFileName [ichLexCh2])) != ilngNil)
						{
						if (ilng == lgcCur.ilng)
							continue;
						if (fScanningDee)
							vpspv->mpilngfDee [ilng] = fTrue;
						vpspv->mpilngiLB [ilng] = isz;
						bltbx((LPSTR)(rglng [ilng].sz), (LPSTR)sz, cbszLangMax);
						return fTrue;
						}
					}
				}
			}

	default:
		return 0;
		}
}


/* F  S c a n  F o r  P l g c */
/* given a language code, hunt for an appropriate DLL .DLL to match.

	Inputs:		plgc->ilng	language designator, may be ilngNil
			plgc->fDee	whether it's a "default" dict 

	Outputs:	plgc->ilng	if dict found, ilng for it, else undef
			plgc->fDee	if Dict found, fDee for it, else undef

	Return Value:	true if dict found, false if not
*/

/* %%Function:FScanForPlgc %%Owner:bryanl */
FScanForPlgc(plgc)
LGC *plgc;
{
	LGC lgc;
	int cbPath;
	CHAR stFind[16];

	lgc = *plgc;
	if (lgc.wLgc == wLgcNil)
		{
/* no initial try provided, must do a scan */
		lgc.fDee = fTrue;	/* scan for LEX-??D.DLL first */
		LgcToStLex( lgc, stFind );
		if (!FFindFileSpec(stFind, vpspl->stPath, grpfpiUtil, nfoPathWildOK))
			{
			lgc.fDee = fFalse;	/* scan for LEX-??.DLL next */
			LgcToStLex( lgc, stFind );
			if (!FFindFileSpec(stFind, vpspl->stPath, grpfpiUtil, nfoPathWildOK))
				return fFalse;
			}
		cbPath = (vpspl->stPath [0] -= stFind [0] );
		lgc.ilng = ILngFromCh1Ch2( vpspl->stPath [cbPath + ichLexCh1 + 1], 
				vpspl->stPath [cbPath + ichLexCh2 + 1] );
		if (lgc.ilng == ilngNil)
			return fFalse;
		}
	else
		{
		LgcToStLex( lgc, stFind );
		if (!FFindFileSpec( stFind, vpspl->stPath, grpfpiUtil, nfoNormal ))
			{
	/* dict not found for language; try other case of fDee */
			lgc.fDee = !lgc.fDee;
			LgcToStLex( lgc, stFind );
			if (!FFindFileSpec( stFind, vpspl->stPath, grpfpiUtil, nfoNormal ))
				return fFalse;
			}
		vpspl->stPath [0] -= stFind [0];
		}

	*plgc = lgc;
	return fTrue;
}


/* Generate appropriate dict filename string from lgc */

/* %%Function:LgcToStLex %%Owner:bryanl */
LgcToStLex( lgc, stLex )
LGC lgc;
char stLex[];
{
	if (lgc.fDee)
		bltbx( (LPSTR)szDefaultLex, 
				(LPSTR)&stLex [1], 
				stLex [0] = sizeof(szDefaultLex) - 1);
	else
		bltbx( (LPSTR)szAllLex, 
				(LPSTR)&stLex [1], 
				stLex [0] = sizeof(szAllLex) - 1);

	if (lgc.ilng != ilngNil)
		{
		stLex [ichLexCh1 + 1] = rglng [lgc.ilng].ch1;
		stLex [ichLexCh2 + 1] = rglng [lgc.ilng].ch2;
		}
}


/* %%Function:ILngFromCh1Ch2 %%Owner:bryanl */
ILngFromCh1Ch2( ch1, ch2 )
CHAR ch1, ch2;
{
	int ilng;

	for ( ilng = 0 ; ilng < ilngMax ; ilng++ )
		if (rglng [ilng].ch1 == ch1 && rglng [ilng].ch2 == ch2)
			return ilng;

	return ilngNil;
}


/* %%Function:SpellDllFree %%Owner:bryanl */
static SpellDllFree()
{
	if (vpspl->hLib != NULL)
		{
		FMySpellCleanUp();
		FreeLibrary(vpspl->hLib);
		vpspl->hLib = (HANDLE)NULL;
		}
}


/* Dispatch to library functions */

/* %%Function:FMySpellInit %%Owner:bryanl */
BOOL FMySpellInit(ghszPath)
HANDLE ghszPath;
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnFSpellInit, vpspl->hstack, &ghszPath, 1 );
}


/* %%Function:FMySpellCleanUp %%Owner:bryanl */
BOOL FMySpellCleanUp()
{
	Assert( vpspl->hstack != NULL );
	return WCallOtherStack( vpspl->lpfnFSpellCleanUp, vpspl->hstack, NULL, 0 );
}


/* %%Function:MyResetRepeatWord %%Owner:bryanl */
NATIVE void MyResetRepeatWord()
{
	Assert( vpspl->hstack != NULL );
	CallOtherStack( vpspl->lpfnResetRepeatWord, vpspl->hstack, NULL, 0 );
}



/* %%Function:IlbLngFromSz %%Owner:bryanl */
IlbLngFromSz(sz)
char * sz;
{
	int ilng;
	int cch;
	int ch1, ch2;

	Assert(fElActive);
	Assert(vpspv != NULL);

	if ((cch = CchSz(sz)) != 3) /* CchSz() counts '\0' */
		{
		char * pch;
		char far * lpch;

		/* We allow the string to match the first (default) entry
			in the list box or be empty... */
		if (cch > 1)
			{
			pch = sz;
			lpch = rglng[0].sz;
			while (cch-- > 0)
				{
				if (ChLower(*pch++) != ChLower(*lpch++))
					goto LFnError;
				}
			}

		return 0;
		}

	ch1 = ChUpper(*sz++);
	ch2 = ChUpper(*sz);

	for (ilng = 0; (ch1 != rglng[ilng].ch1 && ch2 != rglng[ilng].ch2); 
			ilng += 1)
		{
		if (ilng == ilngMax)
			{
LFnError:
			return uNinchList;
			}
		}

	return vpspv->mpilngiLB[ilng];
}

/* F S Z  L N G  F R O M  S Z */
/*
 * This routine take the sz of the form from rglng, "English (US)" for
 * example, and converts it into a two-character sz (three characters with
 * NULL) of the two-letter code, ch1, ch2, "AM".  Unmatched sz map to the
 * default.  Note: input sz is modified and must be at least three characters
 * long.
 */
/* %%Function:FSzLngFromSz %%Owner:edt */
BOOL
FSzLngFromSz(sz, cchBuffer)
CHAR *sz;
int   cchBuffer;
{
	int cch;

	if (cchBuffer < 3)
		return fFalse;

/*	If the two-character sz was passed, skip to the end */
	if ((cch = CchSz(sz)) != 3) /* CchSz() counts '\0' */
		{
		int		  		 ilng;
		CHAR     		*pch;
		CHAR far 		*lpch;

		if (cch == 1)
			ilng = 0;
		else
			{

/*			Convert the sz to upper-case */
			for (pch = sz; *pch; pch++)
				*pch = ChUpper(*pch);

			for (ilng = 0; ilng < ilngMax; ilng++)
				{
				for (lpch = rglng[ilng].sz, pch = sz; *lpch; lpch++, pch++)
					{
					if (ChUpper(*lpch) != *pch)
						break;
					}
/*				If it was a complete match, move on */
				if (!(*pch) && !(*lpch))
					break;
				}
			}

/*		If no match was found, use the default */
		if (ilng == ilngMax)
			ilng = 0;

/*		Update the sz with the two-letter code */
		pch = sz;
		*pch++ = rglng[ilng].ch1;
		*pch++ = rglng[ilng].ch2;
		*pch   = NULL;
		}
	return fTrue;
}
