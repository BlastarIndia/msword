/* -----------------------------------------------------------------------
	etcmd.c - Electronic Thesaurus (ET) command interface code

	Revisions:
	09/15/87    FM  Created
	05/04/88    FM  Major update, most functionality added.
	07/27/88    TK  Added support for the EL UtilGetSynonyms function.

--------------------------------------------------------------------------*/


#ifdef COMMENT
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NODRAWFRAME
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
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
#define NOGDI
#define NOHDC
#define NOMB
/* #define NOMEMMGR */
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
/* #define NOMSG */
/* #define NOOPENFILE */
#define NOPEN
/* #define NOPOINT  */
/* #define NORECT  */
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#endif
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doslib.h"
#include "ch.h"
#include "wininfo.h"
#include "doc.h"
#include "debug.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "file.h"
#include "opuscmd.h"
#include "field.h"
#include "etdefs.h"
#include "error.h"
#include "rerr.h"
#include "rareflag.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "el.h"

#include "thesaur.hs"
#include "thesaur.sdm"
#include "crmgr.h"
#include "screen.h"


BOOL vfFillArray = fFalse;
AD vad;
int vcadEntries;


/* EXTERNALS */

extern HWND		vhwndCBT;
extern HWND             vhwndApp;
extern CHAR             szEmpty[];
extern int              wwCur;
extern struct MERR      vmerr;
extern struct SEL       selCur;
extern struct WWD       **hwwdCur;
extern SB               sbArrays;
extern struct SCI	vsci;
extern struct CA	caPara;
extern BOOL		vfMcrRunning;
extern CP               vcpFetch;

BOOL  FMyInitET(HANDLE);
int   WMyETLookup(HANDLE,int,HANDLE,HANDLE);
BOOL  FMyCloseET(void);


/* Part-of-speech (POS) string definitions. The order of these corresponds
	to the numbering given by HM functions:
		0 - adjective
		1 - noun
		2 - adverb
		3 - verb
	(These should all have a trailing space)
*/
csconst CHAR rgposDef [][] =
{
	SzKey("Adj.: ",Adj),
			SzKey("Noun: ",Noun),
			SzKey("Adv.: ",Adv),
			SzKey("Verb: ",Verb),
			""
};


/*------------------------------------------------------------*/
csconst CHAR szExtDat[]             = "dat";
#define idoHStackET	1
#define idoFInitET	2
#define idoWETLookup	3
#define idoFCloseET	4

#define cchExt 3                    /* Length of file extension */
#define szDefaultExe "syn-am.dll"   /* Name of thesaurus executable */
#define szWildExe "syn-??.dll"   	/* Name of thesaurus executable */


	struct GHD {
	HANDLE ghsz;
	int ichMac;
	int ichMax;
};

struct ETLIB
	{
	HANDLE  hLib;
	HANDLE  hstack;
	struct GHD  ghdWord;
	struct GHD  ghdWorkspace;
	struct GHD  ghdRgpos;
	struct CA ca;
	HSTACK (FAR PASCAL *lpfnHStackET)();
	BOOL  (FAR PASCAL *lpfnFInitET)();
	int   (FAR PASCAL *lpfnWETLookup)();
	BOOL  (FAR PASCAL *lpfnFCloseET)();
};

struct ETLIB *vpetlib;
	struct {
	unsigned    fOriginal:1,
	fLookupDirty:1,
	fDefDirty:1,
	fNoSyn:1;

} grpfET = 0;



/* C M D  T H E S A U R U S */
/* NOTE: Because there are no EL accessable parameters, this command is NOT
considered to be a dialog command.  It must therefore manage it's own cab! */
/* %%Function:CmdThesaurus  %%Owner:bryanl */
CMD CmdThesaurus (pcmb)
CMB * pcmb;
{
	int	cmdRet = cmdOK;
	int tmc;
	CABTHESAURUS	*pcabThesaurus;
	struct ETLIB etlib;

	if ((pcmb->hcab = HcabAlloc(cabiCABTHESAURUS)) == hNil)
		return cmdNoMemory;

	vpetlib = &etlib;

	if (!FInitThesaurus())
		{
		FreeCab(pcmb->hcab);
		pcmb->hcab = hNil;
		/* increase the swap area size */
		GrowSwapArea();
		EndLongOp(fFalse);
		return cmdRet;
		}

		{
		/* initialize the cab */
		pcabThesaurus = (CABTHESAURUS *) *pcmb->hcab;
		(int)(pcabThesaurus->uSynList) = LB_ERR;
		(int)(pcabThesaurus->uDefList) = LB_ERR;
		if (!FSetCabSz(pcmb->hcab, szEmpty, Iag(CABTHESAURUS, hszLookup)))
			return cmdNoMemory;
		}

		{
		CHAR dlt[sizeof(dltThesaurus)];

		BltDlt(dltThesaurus, dlt);

		/* do the dialog */
		if ((tmc = TmcOurDoDlg(dlt, pcmb)) == tmcError)
			cmdRet = cmdNoMemory;
		else if (tmc == tmcCancel)
			cmdRet = cmdCancelled;
		}

	DiscardThesaurus();

	FreeCab(pcmb->hcab);
	pcmb->hcab = hNil;

	/*Increase the swap area size */
	GrowSwapArea();
	return cmdRet;
}



/***************************************************************************/
/* hstWord will be NULL is we're supposed to grab the nearest word */
/* %%Function:ElWUtilGetSynonyms %%Owner:bryanl */
EL int ElWUtilGetSynonyms(ad, hstWord)
AD ad;
char ** hstWord;
{
	char szWord[cchMaxETWord];
	struct ETLIB etlib;

	vad = ad;
	Clear1DArray(vad);

	vcadEntries = 0; /* Number of synonyms in vad */

	vpetlib = &etlib;

	vfFillArray = fTrue;    /* We want to fill the array vad with
		synonyms, rather than showing a dialog. */

	if (!FInitThesaurus())
		{
		/* increase the swap area size */
		vfFillArray = fFalse;
		GrowSwapArea();
		EndLongOp(fFalse);
		return elFFalse;
		}

	if (hstWord != hNil && **hstWord != 0)
		{
		if (**hstWord < cchMaxETWord)
			StToSz(*hstWord, szWord);
		else
			{
			DiscardThesaurus();
			vfFillArray = fFalse;
			GrowSwapArea();
			RtError(rerrStringTooBig);
			}
		}
	else
		FillTmcLookup(szWord);  /* Get the word from the document */

	FillTmcDefList(szWord); /* Fill vad with synonyms for szWord */
	/* vcadEntries will contain # of synonyms */

	DiscardThesaurus();
	vfFillArray = fFalse;

	/*Increase the swap area size */
	GrowSwapArea();

	return vcadEntries > 0 ? elFTrue : elFFalse;
}


/***************************************************************************/


/* F  I N I T   T H E S A U R U S */

/* %%Function:FInitThesaurus  %%Owner:bryanl */
BOOL FInitThesaurus ()
{

	StartLongOp();

	vpetlib->hLib           = (HANDLE)NULL;
	vpetlib->ghdWord.ichMax = vpetlib->ghdWorkspace.ichMax = 
			vpetlib->ghdRgpos.ichMax = 0;

	/* decrease the swap area size */
	ShrinkSwapArea();

	return FLoadThesaurus();
}


/* F  L O A D  T H E S A U R U S */

/* %%Function:FLoadThesaurus  %%Owner:bryanl */
BOOL FLoadThesaurus ()
{
	HANDLE  hLib;
	int     cchPath;
	CHAR stPath[ichMaxFile+1];

	/* Load the Thesaurus Library code */

	if (!FFindFileSpec(StSharedKey(szDefaultExe,DefaultExe), stPath, grpfpiUtil, nfoNormal))
		{
		if (!FFindFileSpec(StSharedKey( szWildExe, WildExe), stPath, grpfpiUtil, nfoPathWildOK))
			goto NoLoad;
		}

	cchPath = stPath[0];
	stPath[cchPath+1] = NULL;
	hLib = LoadLibrary(stPath+1);

	if (hLib < 32)
		{
		if (hLib == 8)  goto NoMem;
		else  
			goto NoLoad;
		}

	vpetlib->hLib = hLib;

	/* Get Thesaurus Entry Point Addresses */

	vpetlib->lpfnHStackET = GetProcAddress(hLib, MAKEINTRESOURCE(idoHStackET));
	if (vpetlib->lpfnHStackET == NULL)
		goto NoLoad;

	vpetlib->lpfnFInitET = GetProcAddress(hLib, MAKEINTRESOURCE(idoFInitET));
	if (vpetlib->lpfnFInitET == NULL)
		goto NoLoad;

	vpetlib->lpfnWETLookup = GetProcAddress(hLib, MAKEINTRESOURCE(idoWETLookup));
	if (vpetlib->lpfnWETLookup == NULL)
		goto NoLoad;

	vpetlib->lpfnFCloseET = GetProcAddress(hLib, MAKEINTRESOURCE(idoFCloseET));
	if (vpetlib->lpfnFCloseET == NULL)
		goto NoLoad;

	if ((vpetlib->hstack = (*vpetlib->lpfnHStackET)()) == NULL)
		goto NoLoad;

	/* Initialize the Thesaurus (Opens the synonym file) */

	bltbx((LPSTR)szExtDat, (LPSTR)&stPath[cchPath+1 - cchExt], cchExt);
	if (!FCopySzToGhdET(stPath+1, &vpetlib->ghdWorkspace) ||
			!FMyInitET(vpetlib->ghdWorkspace.ghsz))
		goto NoLoad;

	EndLongOp(fFalse);
	return fTrue;

NoMem:
	ErrorEid(eidEtNoLoadMem, "");
	goto ErrorExit;

NoLoad:
	ErrorEid(eidEtNoLoad, "");

ErrorExit:
	DiscardThesaurus();

	EndLongOp(fFalse);
	return fFalse;
}



/* D I S C A R D  T H E S A U R U S */

/* %%Function:DiscardThesaurus  %%Owner:bryanl */
DiscardThesaurus ()
{
	StartLongOp();

	if (vpetlib->ghdWord.ichMax > 0)
		{
		Assert( vpetlib->ghdWord.ghsz != NULL );
		GlobalFree( vpetlib->ghdWord.ghsz );
		vpetlib->ghdWord.ichMax = 0;
		}
	if (vpetlib->ghdWorkspace.ichMax > 0)
		{
		Assert( vpetlib->ghdWorkspace.ghsz != NULL );
		GlobalFree( vpetlib->ghdWorkspace.ghsz );
		vpetlib->ghdWorkspace.ichMax = 0;
		}
	if (vpetlib->ghdRgpos.ichMax > 0)
		{
		Assert( vpetlib->ghdRgpos.ghsz != NULL );
		GlobalFree( vpetlib->ghdRgpos.ghsz );
		vpetlib->ghdRgpos.ichMax = 0;
		}
	if (vpetlib->hLib != (HANDLE)NULL)
		{
		FMyCloseET();
		FreeLibrary(vpetlib->hLib);
		vpetlib->hLib = (HANDLE)NULL;
		}

	EndLongOp(fFalse);
}


/* F  D L G  T H E S A U R U S */

/* %%Function:FDlgThesaurus %%Owner:bryanl */
BOOL FDlgThesaurus(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD  wNew, wOld, wParam;
{
	int     idef;
	CHAR	szLookup[cchMaxSz];
	CHAR	szT[cchMaxSz];

	switch (dlm)
		{
	case dlmInit:

			{
			struct RC rcApp, rcDlg;
			HWND hwnd;

			if (vhwndCBT == NULL)
				{
			/* position the dialog out of the way */
			/* MoveDlg not safe if dialog coming down */
				if (!FIsDlgDying()) 
					{
					GetWindowRect(vhwndApp, (LPRECT) &rcApp);
					hwnd = HwndFromDlg(HdlgGetCur());
					GetWindowRect(hwnd, (LPRECT) &rcDlg);
					MoveDlg(rcDlg.xpLeft, 
						rcApp.ypBottom - vsci.dypBdrCapt -
						rcDlg.ypBottom + rcDlg.ypTop);
					}
				}
			}

		grpfET.fNoSyn = fFalse;
		FillTmcLookup(szLookup);
		grpfET.fOriginal = !(*szLookup == NULL);
		LookupSz(szLookup);
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcSynList:
			if (grpfET.fNoSyn)
				{
				SetTmcVal(tmcSynList, uNinchList);
				break;
				}
			break;
		case tmcSynonym:
			GetTmcText( (grpfET.fLookupDirty || 
					ValGetTmc(tmcSynList) == uNinchList) ?
					tmcLookup : tmcSynList,
					szLookup,
					cchMaxSz );
			goto LLookup;
			break;
		case tmcDefList:
			if (vfMcrRunning)
				{
				GetTmcText(tmcDefList, szLookup, cchMaxSz);
				FillTmcDefinition(szLookup);
				idef = ValGetTmc(tmcDefList);
				GetTmcText(tmcLookup, szLookup, cchMaxSz);
				FillTmcSynList(szLookup, idef, fFalse /*fFocusChangeOK*/);
				grpfET.fDefDirty = fFalse;
				}
			else
				grpfET.fDefDirty = fTrue;
			break;
		case tmcOriginal:
			FillSzFromPca(&selCur.ca,szLookup,cchMaxETWord+1,fFalse/*fRetCaFetch*/);
LLookup:	
			LookupSz(szLookup);
			break;
			}
		break;

	case dlmDblClk:
		if (tmc == tmcSynList)
			{
			if (grpfET.fNoSyn)
				SetTmcVal(tmcSynList, uNinchList);
			else
				{
				GetTmcText(tmcSynList, szLookup, cchMaxSz);
				LookupSz(szLookup);
				}
			}
		return fFalse;	/* fFalse means: do not push default button */

	case dlmChange:
		/* the text in the EC has changed.  */
		if (tmc == tmcSynList && grpfET.fNoSyn)
			{
			SetTmcVal(tmcSynList, uNinchList);
			break;
			}
		if (tmc == tmcLookup)
			{
			grpfET.fLookupDirty = fTrue;
			SetDefaultTmc(tmcSynonym);
			FEnableTmcOriginal();
			}
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */

		if (grpfET.fDefDirty)
			{
			grpfET.fDefDirty = fFalse;
			GetTmcText(tmcDefList, szLookup, cchMaxSz);
			FillTmcDefinition(szLookup);
			idef = ValGetTmc(tmcDefList);
			GetTmcText(tmcLookup, szLookup, cchMaxSz);
			FillTmcSynList(szLookup, idef, fFalse);
			}
		return (fFalse); /* so we keep idling */

	case dlmTerm:
		switch (tmc)
			{
		case tmcCancel:
			break;
		case tmcReplace:
			if (ValGetTmc(tmcSynList) == uNinchList)
				break;
				{
				CHAR ch, *pch;
				int cchUpper = 0;
				int cchReplace;
				FC fcReplace;
				struct CA caT;

				caT = vpetlib->ca;
				GetTmcText(tmcSynList, szLookup, cchMaxSz);
/* convert replace string to first-letter-upper or all-upper, as appropriate */
				Assert( DcpCa(&caT) > 0);
				if (FUpper( ChFetch( caT.doc, caT.cpFirst, fcmChars )))
					{
					cchUpper = 1;
					if (DcpCa( &caT ) > (CP) 1 && 
							FUpper( ChFetch( caT.doc, caT.cpFirst+1, fcmChars )))
						{
						cchUpper = CchSz( szLookup ) - 1;
						}
					}

				if (!FSetUndoB1(imiReplace, uccPaste, &caT))
					break;
				for ( pch = szLookup ; cchUpper-- ; pch++ )
					*pch = ChUpper( *pch );
				cchReplace = CchSz(szLookup) - 1;
				GetSelCurChp(fFalse);
				if (!FNewChpIns( caT.doc, caT.cpFirst, &selCur.chp, stcNil ))
					break;
				fcReplace = FcAppendRgchToFn(fnScratch, szLookup, cchReplace);
				if (!FReplaceRM(&vpetlib->ca, fnScratch, fcReplace,
						(FC)cchReplace))
					break;
				caT.cpLim = caT.cpFirst + cchReplace;
				SetUndoAfter(&caT);
				}
			break;
			}
		}

	return fTrue;
}


/* L O O K U P  S Z */

/* %%Function:LookupSz  %%Owner:bryanl */
LookupSz (szWord)
CHAR *szWord;
	/* Look up new definitions for the word in szWord. Then look up the
		synonyms for the first definition (if there are any).
	*/
{
	int     idef;
	CHAR    szDefinition[cchMaxSz];

	if (CchSz(szWord) >= cchMaxETWord)
		*szWord = NULL;

	SetTmcText(tmcLookup, szWord);
	FillTmcDefList(szWord);
	idef = ValGetTmc(tmcDefList);
	FillTmcSynList(szWord, idef, fTrue);
	grpfET.fLookupDirty = fFalse;
	GetTmcText(tmcDefList, szDefinition, cchMaxSz);
	FillTmcDefinition(szDefinition);
	FEnableTmcOriginal();
}


/* F  E n a b l e  T m c  O r i g i n a l */
/* lookup word changed. set Original to enabled if 
	editcontrol word does not match original word; nonenabled otherwise */

/* %%Function:FEnableTmcOriginal %%Owner:bryanl */
FEnableTmcOriginal()
{
	int f;
	char szT1 [cchMaxSz], szT2 [cchMaxSz];

	FillSzFromPca(&selCur.ca,szT1,cchMaxETWord+1,fFalse/*fRetCaFetch*/);
	GetTmcText(tmcLookup, szT2, cchMaxSz);
	f = FNeRgch(szT1, szT2, CchSz( szT1 ));
	EnableTmc( tmcOriginal, f );
	return f;
}


/* F I L L  T M C   L O O K U P */

/* %%Function:FillTmcLookup  %%Owner:bryanl */
FillTmcLookup (szLookup)
CHAR *szLookup;
/*
Get the selected word from the document and put into tmcLookup. If the
selection covers more than one word, empty the dialog and return 
with *szLookup = NULL.
*/
{
	CP cpFirst;
	int ccpFetch;
	struct CA *pca = &vpetlib->ca;
	struct FVB fvb;
	struct CR rgcr[ccrArgumentFetch];
	CHAR stT[cchMaxETWord + 1];

	szLookup [0] = '\0';

	FetchCpPccpVisible(selCur.doc, selCur.cpFirst, &ccpFetch, wwCur/*fvcScreen*/, fFalse);
	if (ccpFetch == 0)
		goto LBackup;
	else
		cpFirst = vcpFetch; /* use the visible one */

	PcaSet( pca,selCur.doc, 
			CpFirstSty(wwCur, selCur.doc, cpFirst, styWord, selCur.fInsEnd),
			CpLimSty  (wwCur, selCur.doc, cpFirst, styWord, selCur.fInsEnd));
	pca->cpLim = CpLimNoSpaces(pca);
	if (DcpCa(pca) == cp0 || 
			(CachePara( pca->doc, pca->cpFirst ), caPara.cpLim == pca->cpLim))
		{
LBackup:
		cpFirst = CpFirstSty(wwCur, selCur.doc, selCur.cpFirst, styWord, fTrue);
		FetchCpPccpVisible(selCur.doc, cpFirst, &ccpFetch, wwCur/*fvcScreen*/, fFalse);
		if (vcpFetch != cpFirst)
			cpFirst = vcpFetch; /* again, use the visible one */
		PcaSet( pca,selCur.doc,
				cpFirst,
				CpLimSty  (wwCur, selCur.doc, selCur.cpFirst, styWord, fTrue));
		pca->cpLim = CpLimNoSpaces(pca);
		if (CachePara( pca->doc, pca->cpFirst ), caPara.cpLim == pca->cpLim)
			PcaPoint( pca, selCur.doc, selCur.cpFirst );
		}

	/* Adjust selection into view & to span the current word. */
	AssureCpAboveDyp(selCur.cpFirst, 
			vsci.dysMacAveLine, fFalse );

	/* Fetch the word between pca->cpFirst and pca->cpLim */
	FillSzFromPca(pca,szLookup,cchMaxETWord+1,fTrue/*fRetCaFetch*/);
	Select(&selCur,pca->cpFirst,pca->cpLim);
	*pca = selCur.ca;
Exit:
	if (!vfFillArray)   /* If we are filling an array, we don't have a dialog */
		SetTmcText(tmcLookup, szLookup);
}



/* F I L L  S Z  F R O M  P C A */
/* ****
*  Description: given a pointer to cache and a pointer to string
*               it makes the latter point to the selected word
*               in the cache.
*               pca->cpFirst and pca->cpLim are set to correspond
*               to the selected word.
*
*  Input: pointer to cache pca, pointer to string sz.
*
*  Output:
*     sz points to the selected word in the cache pointed to by pca.
*
** **** */
/*  %%Function: FillSzFromPca  %%Owner: bryanl  */


FillSzFromPca(pca,sz,ichMac,fRetCaFetch)
struct CA *pca;
CHAR *sz;
uns ichMac;
BOOL fRetCaFetch; /* if true, will modify input pca */
{
	struct FVB fvb;
	struct CR rgcr[ccrArgumentFetch];
	CHAR stT [cchMaxSz];

	Assert( ichMac <= cchMaxSz );

	/* Fetch the word between cpFirst and cpLim. */
	InitFvb(&fvb);
	InitFvbBufs(&fvb, stT+1, ichMac - 1, rgcr, ccrArgumentFetch);
	fvb.doc = pca->doc;
	fvb.cpFirst = pca->cpFirst;
	fvb.cpLim   = pca->cpLim;
	FetchVisibleRgch(&fvb, wwCur /*fvc*/, fFalse /*fProps*/, fFalse /*fNested*/);
	if (fvb.cch <= ichMac - 1)
		stT[0] = fvb.cch;

	if (fRetCaFetch)
		{
		if (fvb.cch > 0)
			{
			pca->cpFirst = CpFromIchPcr(0, rgcr, fvb.ccr);
			pca->cpLim = CpFromIchPcr(fvb.cch-1, rgcr, fvb.ccr)+1;
			Assert(pca->cpLim <= CpMacDoc(pca->doc));
			}
		else
			pca->cpLim = pca->cpFirst;
		}

		/* Copy stT to sz, removing all nondisplayable characters */
		{
		int cch = stT[0];
		CHAR *pchFrom = stT+1;
		CHAR *pchTo = sz;

		while (cch--)
			{
			int ch = *pchFrom++;
			if (!FDisplayableCh(ch))
				continue;
			*pchTo++ = ch;
			}
		*pchTo = NULL;
		}
}


/* F I L L  T M C  D E F I N I T I O N */

/* %%Function:FillTmcDefinition  %%Owner:bryanl */
FillTmcDefinition (szFullDef)
CHAR *szFullDef;                    /* PartOfSpeech: Definition */
{
	CHAR    *pchDef, *pchPos;
	CHAR    szT[cchMaxSz];

	CchCopySz(szFullDef, szT);
	pchDef = index(szT, ':');
	if (pchDef == NULL)
		{
		pchDef = szT;
		pchPos = szEmpty;
		}
	else
		{
		*(++pchDef) = NULL;
		while (*(++pchDef) == ' ')
			;                       /* Skip spaces following colon */
		pchPos = szT;
		}

	SetTmcText(tmcPartOfSpeech, pchPos);
	SetTmcText(tmcDefinition, pchDef);
}



/* F I L L  T M C  S Y N  L I S T */
/* Modified for EL UtilGetSynonyms */

/* %%Function:FillTmcSynList  %%Owner:bryanl */
FillTmcSynList (szWord, idef, fFocusChangeOK)
CHAR *szWord;
int idef;
int fFocusChangeOK;
	/* Fill synonym list box with synonymns for word szWord, 
		definition number idef
	*/
{
	int  isyn, csyn, cchList, csynMax;
	CHAR *pchSyn, *pchNext;
	CHAR szLBEntry[cchMaxSz];
	CHAR szOemWord[cchMaxSz];
	CHAR szSynonyms[cchMaxETString];

	grpfET.fNoSyn = fFalse;
	if (!vfFillArray)   /* If we are filling an array, we don't have a dialog */
		{
		SetTmcVal(tmcSynList, uNinchList);

		/* Disable LB drawing while updating */
		StartListBoxUpdate(tmcSynList);
		}

	if (*szWord == '\0' || idef < 0)
		goto LNoSyn;

	/* Copy szWord to global heap */
	AnsiToOem(szWord, szOemWord);
	if (!FCopySzToGhdET(szOemWord, &vpetlib->ghdWord))
		goto LNoSyn;

	/* Allocate and null-fill space for return strings */
	if (!FClearGhd(&vpetlib->ghdWorkspace, cchMaxETString + 1))
		goto LNoSyn;

	idef++;                             /* HM defn numbers are 1-based */

	Assert(idef > 0);
	if (!FClearGhd(&vpetlib->ghdRgpos, 1))     /* rgpos not used when idef != 0 */
		goto LNoSyn;

	/* Lookup the synonyms */
	csyn = WMyETLookup(vpetlib->ghdWord.ghsz, idef, 
			vpetlib->ghdWorkspace.ghsz, 
			vpetlib->ghdRgpos.ghsz);

	if (csyn == 0)
		{
LNoSyn:
		grpfET.fNoSyn = fTrue;

		if (!vfFillArray)
			{
			AddListBoxEntry(tmcSynList, SzFrameKey("(No Synonyms)",NoSyn));
			SetTmcVal(tmcSynList, uNinchList);
			SetDefaultTmc( tmcSynonym );
			if (FEnableTmcOriginal())
				SetDefaultTmc( tmcOriginal );
			if (fFocusChangeOK || TmcGetFocus() == tmcReplace)
				SetFocusTmc( tmcLookup );
			EnableTmc( tmcReplace, fFalse );
			}
		goto Exit;
		}

	/* Copy Synonymns from the global heap */
	FCopyGhdToSzET(&vpetlib->ghdWorkspace, szSynonyms);

	pchSyn = szSynonyms;
	cchList = CchSz(szSynonyms);

	/* If we are filling an array, set csynMax to be the max number of
		* synonyms we can put in the array vad.   Note that vcadEntries is number
		* of synonyms currently in array vad. */
	if (vfFillArray)
		{
		csynMax = HpahrOfAd(vad)->rgas[0];
		}

	for (isyn = 0;  ;isyn++)
		{
		int fLast = fFalse;
		int cchSyn;
		int ich;

		pchNext = pchSyn;
		for (ich = 0; ich < cchList; ich++)
			{
			int ch = *pchNext;
			if ((ch == chEndSyn) || (ch == chEndSynAlt))
				break;
			if (ch == chEndDef)
				{
				fLast = fTrue;
				break;
				}
			++pchNext;
			}
		cchSyn = pchNext - pchSyn;

		/* Copy synonym string to LB entry */
		bltbyte (pchSyn, szLBEntry, cchSyn);

		/* Null-terminate the entry */
		szLBEntry[cchSyn] = NULL;

		OemToAnsi(szLBEntry, szLBEntry);

		/* If called by EL, put resulting entry in array to be returned,
			otherwise add the entry to the list box */

		if (vfFillArray)
			{
			/* If array is not full, fill in next array entry with szLBEntry */
			if (vcadEntries < csynMax)
				{
				extern BOOL fElActive;
				Assert(fElActive);
				if ((*((SD huge *) &HpahrOfAd(vad)->rgas[1] + vcadEntries++) =
						SdCopySz(szLBEntry)) == sdNil)
					{
					RtError(rerrOutOfMemory);
					}
				}
			/* If array is now full, end */
			if (!fLast && vcadEntries >= csynMax)
				fLast = fTrue;
			}
		else
			AddListBoxEntry(tmcSynList, szLBEntry);

		if (fLast)
			break;
		else
			pchSyn = pchNext+1;
		}

	if (!vfFillArray)   /* If we are filling an array, we don't have a dialog */
		{
		SetTmcVal(tmcSynList, 0);
		if (!grpfET.fOriginal)
			SetDefaultTmc( tmcSynonym );
		EnableTmc(tmcReplace, grpfET.fOriginal);
		if (grpfET.fOriginal)
			SetDefaultTmc( tmcReplace );
		if (fFocusChangeOK)
			SetFocusTmc(tmcSynList);
		}

Exit:
	/* Redisplay the list box */
	if (!vfFillArray)
		EndListBoxUpdate(tmcSynList);
}


/* F I L L  T M C  D E F  L I S T */
/* Modified for EL UtilGetSynonyms */
/* %%Function:FillTmcDefList  %%Owner:bryanl */
FillTmcDefList (szWord)
CHAR *szWord;
	/* Fill definitions list box with definitions for word szWord.
	*/
{
	CHAR *pchDef, *pchNext;
	int  cdef;
	int  idef;
	int  cchpos;
	CHAR szLBEntry[cchMaxSz];
	CHAR szOemWord[cchMaxSz];
	int  rgpos[idefMax];
	CHAR szDefinitions[cchMaxETString];

	if (!vfFillArray)   /* If filling an array, we don't have a dialog */
		{
		SetTmcVal(tmcDefList, uNinchList);

		/* Disable LB drawing while updating */
		StartListBoxUpdate(tmcDefList);
		}

	if (*szWord == NULL)
		goto Exit;

	/* Copy szWord to global heap */
	AnsiToOem(szWord, szOemWord);
	if (!FCopySzToGhdET(szOemWord, &vpetlib->ghdWord))
		goto Exit;

	/* Allocate and null-fill space for return strings */
	if (!FClearGhd(&vpetlib->ghdWorkspace, cchMaxETString + 1))
		goto Exit;
	if (!FClearGhd(&vpetlib->ghdRgpos, sizeof(rgpos) + 1))
		goto Exit;

	/* Lookup the synonyms */
	cdef = WMyETLookup(vpetlib->ghdWord.ghsz, 0,
			vpetlib->ghdWorkspace.ghsz, 
			vpetlib->ghdRgpos.ghsz);

	if (cdef == 0) goto Exit;

	if (vfFillArray)
		{
		/* If we're filling an array with synonyms, we want all the synonyms
			we can fit in the array, for all definitions.   Since we don't
			need the text of the definitions, we don't bother parsing them,
			instead we call FillTmcSynList for each definition number.
			Since vfFillArray is true, FillTmcSynList fills in the array,
			not the tmc.   FillTmcSynList returns when the array is full
			or it runs out of synonyms for the current definition. */
		for (idef = 0; idef < cdef && vcadEntries < HpahrOfAd(vad)->rgas[0]; idef++)
			FillTmcSynList(szWord, idef, fTrue /*fFocusChangeOK*/);
		goto Exit;
		}

	/* Copy Definitions from the global heap */
	if (!FCopyGhdToSzET(&vpetlib->ghdWorkspace, szDefinitions))
		goto Exit;

	/* Copy rgpos (parts of speech IDs) from the global heap */
	cchpos = cdef << 1;
	if (!FCopyGhdToRg(&vpetlib->ghdRgpos, rgpos, cchpos))
		goto Exit;

	pchDef = pchNext = szDefinitions;
	for (idef = 0; idef < cdef; idef++)
		{
		int iposDef = rgpos[idef];
		int cchDef;

		cchpos = rgposDef[iposDef+1] - rgposDef[iposDef];

		pchNext = index(pchDef, chEndDef);
		if (pchNext == 0)
			break;
		while (fTrue)
			{
			char *pchT;
			int chNext = *(++pchNext);

			if (FUpper(chNext) || (chNext == chEndDef))
				/* Found end of definition */
				break;
			pchT = pchNext;
			pchNext = index(pchNext, chEndDef);
			if (pchNext == 0)
				{
				pchNext = pchT;
				break;
				}
			}

		cchDef = pchNext - pchDef - 1;

		/* Copy part-of-speech string to LB entry */
		bltbx ((LPSTR) rgposDef[iposDef], (LPSTR) szLBEntry, cchpos);

		/* Append definition string to LB entry */
		bltbyte (pchDef, &szLBEntry[cchpos-1], cchDef);

		/* Null-terminate the entry */
		szLBEntry[cchpos + cchDef - 1] = NULL;

		/* Add the entry to the list box */
		OemToAnsi(szLBEntry, szLBEntry);
		AddListBoxEntry(tmcDefList, szLBEntry);

		pchDef = pchNext;
		}

	SetTmcVal(tmcDefList, 0);

Exit:
	/* If we are filling a tmc, rather than array, redisplay the list box */
	if (!vfFillArray)
		EndListBoxUpdate(tmcDefList);
}




/* F  C L E A R  G H D */
/*   Allocate and null-fill ghd */

/* %%Function:FClearGhd  %%Owner:bryanl */
FClearGhd (pghd, cch)
struct GHD *pghd;
int cch;
{
	CHAR FAR * lpch;

	if (pghd->ichMax == 0)
		{
		if ((pghd->ghsz = OurGlobalAlloc( GMEM_MOVEABLE, (long)(uns)cch )) == NULL)
			goto LError;
		goto LNewSiz;
		}
	else  if (pghd->ichMax < cch)
		{
		if (OurGlobalReAlloc(pghd->ghsz, (long)(uns)cch, GMEM_MOVEABLE) == NULL)
			{
LError:		
			SetErrorMat( matMem );
			return fFalse;
			}
LNewSiz:
		pghd->ichMax = cch;
		}

	Assert( pghd->ichMax >= cch );

	if ((lpch = GlobalLockClip( pghd->ghsz )) == NULL)
		goto LError;

	SetBytes(lpch, 0, pghd->ichMac = cch);
	GlobalUnlock( pghd->ghsz );
	return fTrue;
}




/* F  C O P Y  G H D  T O  R G */

/* %%Function:FCopyGhdToRg %%Owner:bryanl */
FCopyGhdToRg (pghd, rg, cch)
struct GHD *pghd;
char *rg;
int cch;
{
	LPSTR   lp;

	if ((lp = GlobalLockClip(pghd->ghsz)) == NULL)
		{
		SetErrorMat(matMem);
		return fFalse;
		}

	while (cch--)
		*rg++ = *lp++;

	GlobalUnlock(pghd->ghsz);
	return fTrue;
}


/* %%Function:FMyInitET %%Owner:bryanl */
BOOL FMyInitET(ghszPath)
HANDLE ghszPath;
{
    BOOL f;

	Assert( vpetlib->hstack != NULL );
    vrf.fInExternalCall = fTrue;
	f = WCallOtherStack( vpetlib->lpfnFInitET, vpetlib->hstack, &ghszPath, 1 );
    vrf.fInExternalCall = fFalse;
    return f;
}


/* %%Function:WMyETLookup  %%Owner:bryanl */
int WMyETLookup (ghszWord, iopt, ghszWorkspace, ghrgpos)
HANDLE ghszWord;
int    iopt;
HANDLE ghszWorkspace;
HANDLE ghrgpos;
{
    BOOL f;
	Assert( vpetlib->hstack != NULL );
    vrf.fInExternalCall = fTrue;
	f = WCallOtherStack( vpetlib->lpfnWETLookup, vpetlib->hstack, &ghrgpos, 4 );
    vrf.fInExternalCall = fFalse;
    return f;
}


/* %%Function:FMyCloseET %%Owner:bryanl */
BOOL FMyCloseET()
{
    BOOL f;
	Assert( vpetlib->hstack != NULL );
    vrf.fInExternalCall = fTrue;
    f = WCallOtherStack( vpetlib->lpfnFCloseET, vpetlib->hstack, NULL, 0 );
    vrf.fInExternalCall = fFalse;
    return f;
}


/* %%Function:FCopyGhdToSzET %%Owner:bryanl */
FCopyGhdToSzET( pghd, sz )
struct GHD *pghd;
char *sz;
{
	CHAR FAR *lpch;
	Assert( pghd->ghsz != NULL );
	Assert( pghd->ichMax >= pghd->ichMac );
	Assert( pghd->ichMac > 0 );

	if ((lpch = GlobalLockClip( pghd->ghsz )) == NULL)
		{
		SetErrorMat(matMem);
		return fFalse;
		}

	bltbx( lpch, (char far *)sz, pghd->ichMac );
	GlobalUnlock( pghd->ghsz );
	return fTrue;
}


/* %%Function:FCopySzToGhdET %%Owner:bryanl */
FCopySzToGhdET( sz, pghd )
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

	if ((lpch = GlobalLockClip( pghd->ghsz )) != NULL)
		{
		bltbx( (char far *)sz, lpch, pghd->ichMac = cch );
		GlobalUnlock( pghd->ghsz );
		return fTrue;
		}
	else
		return fFalse;

}


