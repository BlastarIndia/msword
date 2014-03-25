/* D I A L O G 2 . C */
/* Code used to interact with and dismiss a dialog */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "error.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "resource.h"
#include "wininfo.h"
#include "opuscmd.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "version.h"
#include "screen.h"
/*#include "el.h"*/
#include "idd.h"
#include "prompt.h"
#include "message.h"
#include "doslib.h"
#include "keys.h"
#include "rareflag.h"
#include "style.h"
#include "props.h"
#include "sel.h"
#include "inter.h"
#include "format.h"
#include "prm.h"
#include "dlbenum.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#define chPath ('\\')

extern BOOL fElActive;
extern struct PREF vpref;
extern struct ITR vitr;
extern HWND vhwndStartup;
extern HWND vhwndStatLine;
extern struct MERR  vmerr;
extern HWND vhwndApp;
extern int vdocStsh;
extern struct SEL selCur;
extern char (**vhmpiststcp)[];
extern int vstcBackup;
extern int vdocStsh;
extern int vistLbMac;
extern int vfShowAllStd;
extern CHAR    *mputsz[];
extern struct RF	vrf;
extern struct SEL selCur;
extern struct CA caSect;
extern struct CA caPara;
extern struct SEP vsepFetch;
extern struct PAP vpapFetch;
extern BOOL vfDefineStyle;
extern struct ESPRM dnsprm [];
extern struct PREF vpref;
extern HWND vhwndApp;
extern HWND vhwndPgPrvw;
extern int vstcStyle;
extern int vlm;
extern int vfShowAreas;
extern char szEmpty [];
extern struct STTB ** vhsttbFont;
extern struct PRI vpri;
extern struct PLAOR **vhplaor;
extern struct LBS vlbsText, vlbsText2;
extern struct MERR vmerr;
extern KMP **hkmpCur;
extern void ToggleShowStd();
extern struct SCI  vsci;
extern HWND vhwndCBT;


/*  %%Function:  ChangeCancelToClose  %%Owner:  bobz       */

ChangeCancelToClose()
{
	SetTmcText(tmcCancel, SzSharedKey("Close",Close));
}


/*  %%Function:  CmdDlgAction  %%Owner:  bobz       */

CMD CmdDlgAction(pfn, tmc)
CMD (* pfn )();
TMC tmc;
{
	CMB cmb;
	HCAB hcab;

	if ((hcab = HcabFromDlg(fFalse)) == hcabNull
			|| hcab == hcabNotFilled)
		return cmdError;
	AssertDo(FInitCmb(&cmb, PcmbDlgCur()->bcm, hcab, cmmAction | cmmBuiltIn));

	Assert(PcmbDlgCur()->hcab == cmb.hcab);

	cmb.tmc = tmc;
	cmb.pv = PcmbDlgCur()->pv;
	return (*pfn)(&cmb);
}


/*  %%Function:  RangeError  %%Owner:  bobz       */

RangeError(wLow, wHigh, fZa, ut)
int wLow, wHigh, ut;
BOOL fZa;
{
	int rgw [2];
	char szLow [ichMaxNum + 1];  /* leave room for null term */
	char szHigh [ichMaxNum + 1];
	char * pch;
	int utT = vpref.ut;

#ifdef DEBUG
	int cch;
#endif /* DEBUG */


	if (fZa)
		{
		vpref.ut = ut;
		pch = szLow;
		/* Note: for these ranges we want to round the low value
		high and the high value low, so that a reported out of
		range value will always be outside the range
		*/
#ifdef DEBUG
		cch = 
#endif /* DEBUG */
				CchExpZaRnd(&pch, wLow, ut, ichMaxNum + 1, fTrue, grpfRndHigh);
		Assert (cch <= ichMaxNum + 1);
		pch = szHigh;
#ifdef DEBUG
		cch = 
#endif /* DEBUG */
				CchExpZaRnd(&pch, wHigh, ut, ichMaxNum + 1, fTrue, grpfRndLow);
		Assert (cch <= ichMaxNum + 1);

		vpref.ut = utT;
		rgw[0] = szLow;
		rgw[1] = szHigh;
		}
	else
		{
		rgw[0] = wLow;
		rgw[1] = wHigh;
		}

	ErrorEidRgw((fZa ? eidZaOutOfRange : eidOutOfRange), rgw, "RangeError");
}





/* NOTE: Not a real parse function!  This is called by parse functions. */
/*  %%Function:  WParseIntRange  %%Owner:  bobz       */

WORD WParseIntRange(tmm, sz, ppv, bArg, tmc, wLow, wHigh)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
int wLow, wHigh;
{
	int w;
	DPV dpv;
	int cch;
	char * pch;
	WORD ninch;

	ninch = wLow >= 0 ? uNinch : wNinch;

	switch (tmm)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif

	case tmmParse:
		dpv = DpvParseFdxa(&w, tmc, sz, wLow, wHigh,
				dpvBlank, eidOutOfRange, fFalse, vpref.ut);
		if (dpv == dpvNormal)
			{
			SetPpvBToW(ppv, bArg, w);
			return fTrue;
			}
		else  if (dpv == dpvError)
			/* error reporting already handled */
			/* **ppval is set to something different than the gray
				value since GetTmcVal pays no attention to the 
				return value */
			{
			SetPpvBToW(ppv, bArg, wError);
			return fFalse;
			}
		else  /* blank or null string - treat as unchanged value */			
			{
			SetPpvBToW(ppv, bArg, ninch);
			return fTrue;
			}
		/* NOT REACHED */

	case tmmFormat:
		w = WFromPpvB(ppv, bArg);
		if (w == ninch)
			*sz = 0;
		else
			{
			Assert(w >= wLow && w <= wHigh);
			pch = sz;
			cch = CchIntToPpch(w, &pch);
			sz[cch] = 0;
			}
		break;

#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif
		}

	return 0;
}


/* O U R  R E S I Z E  D L G */
/* Resize the current dialog with dialog style coordinates (1/4 x 1/8 char) */
/*  %%Function:  OurResizeDlg  %%Owner:  bobz       */

OurResizeDlg(dx, dy)
int dx, dy;
{
	struct RC rc;

			 /* ResizeDlg not safe if dialog coming down */
	if (!FIsDlgDying())
		{
		rc.xpLeft = rc.ypTop = 0;
		rc.xpRight = dx;
		rc.ypBottom = dy;
		SdmScaleRec(&rc);
		ResizeDlg(rc.xpRight, rc.ypBottom);
		}
}



/* F T R U N C A T E  T M C  S Z  */
/* If a string placed in tmc (generally static text) would be too
	long, truncate it and add elipises.  Returns TRUE if truncated.
*/
/*  %%Function:  FTruncateTmcSz  %%Owner:  bobz       */

BOOL FTruncateTmcSz(tmc, sz)
TMC tmc;
CHAR *sz;
{
	int cchTmc;
	struct RC rc;

	if (FIsDlgDying())	/* gettmcrc will return 0 then */
		return fFalse;

	/* get rect for tmc in template coords */
	GetTmcRec(tmc, &rc);
	if (vsci.dxpTmWidth)
		cchTmc = (rc.xpRight - rc.xpLeft) / vsci.dxpTmWidth;
	else
		{
		cchTmc = (rc.xpRight - rc.xpLeft) / 8; /* TOTALLY abritrary guess bz */
		Assert (fFalse);
		}

	if (CchSz(sz)-1 > cchTmc)
		{
		Assert(cchTmc > 3);
		sz[cchTmc--] = 0;
		sz[cchTmc--] = '.';
		sz[cchTmc--] = '.';
		sz[cchTmc--] = '.';
		return fTrue;
		}
	return fFalse;
}



/* ****
*  Description: Given an ASCII string containing a (base 10) number,
*      return the number represented.  Ignores leading and trailing spaces.
** **** */
/*  %%Function:  WFromSzNumber  %%Owner:  bobz       */


int WFromSzNumber( szNumber )
CHAR * szNumber;
{
	CHAR * PchSkipSpacesPch();
	unsigned w = 0;
	BOOL fNeg = fFalse;
	int ch;

	szNumber = PchSkipSpacesPch( szNumber );

	if (*szNumber == '-')
		{
		fNeg = fTrue;
		szNumber = PchSkipSpacesPch(szNumber + 1);
		}

	while ( ((ch = *szNumber++) >= '0') && (ch <= '9') )
		w = (w * 10) + (ch - '0');

	return fNeg ? -w : w;
}


/*  %%Function:  PchSkipPath  %%Owner:  bobz       */

char * PchSkipPath(sz)
char * sz;
{
	int ch;
	char * pch;

	pch = sz;
	while ((ch = *sz++) != 0)
		if (ch == chPath || ch == ':')
			pch = sz;
	return pch;
}


/*  %%Function:  FNewPropFromRgProp  %%Owner:  bobz       */


FNewPropFromRgProp(rgProp, fCab, mpiagspnt)
WORD *rgProp;  /* actually hcab if fCab */
int fCab;
SPNT mpiagspnt [];
{
	int cb;
	int fChp;
	char grpprl [cbMaxGrpprl];

	/* this should virtually never happen, but we have a flaky
		bug where valfrompropsprm claims fUpdatePap is false
		so I am forcing the issue. Note that in the chp case, we might
		be doing in unecessarily at this point, but it lets us do
		this is one place for everyone who needs it.
	*/
	if (!vfDefineStyle && selCur.fUpdatePap)
		AssertDo(FGetParaState(fFalse /* fAll */ , fFalse /* fAbortOk */));

	NewGrpprlFromRgProp(rgProp, fCab, mpiagspnt, grpprl, &cb,
			vfDefineStyle, &fChp);

	if (cb != 0)
		{
		/* grpprl contains grpprl describing properties */
		if (vfDefineStyle)
			{
			extern BOOL fElActive;
			ApplyGrpprlToStshPrope(grpprl, cb, 
					Usgn(fChp), !fElActive);
			}
#ifdef FUTURE
	/* somewhat buggy feature disabled for version 1.0 */
		else  if (vlm == lmPreview)
			{
			int fFail = fFalse;
			HDC hdc = GetDC(vhwndPgPrvw);

			CacheAbsObj();
			ApplyGrpprlCa(grpprl, cb, &caPara);
			ShowAreas(hdc, fFalse);  /* erase */
			if (!(fFail = !FChangePrvwDoc(hdc, fFalse, fFalse)))
				ShowAreas(hdc, fFalse);  /* draw */
			ReleaseDC(vhwndPgPrvw, hdc);
			if (fFail)
				{
				EndPreviewMode();
				Assert(vmerr.fMemFail);
				goto LFail;
				}
			}
#endif
		else
			{
			if (*grpprl == sprmPStc)
				CmdApplySprmPStc(grpprl, cb);
			else
				{
				ApplyGrpprlSelCur(grpprl, cb, fTrue);
				InvalSoftlyPgvwPara(selCur.doc, &grpprl, cb);
				}
			}
		}
LFail:
	return (vmerr.fMemFail ? fFalse : fTrue);
}


/*  %%Function:  NewGrpprlFromRgProp  %%Owner:  bobz       */

NewGrpprlFromRgProp(rgProp, fCab, mpiagspnt, grpprl, pcb, fStsh, pfChp)
WORD *rgProp;
int fCab;
SPNT mpiagspnt [];
char * grpprl;
int * pcb;
int fStsh;
int * pfChp;
{
	int sprm;
	int iag;
	int iagMac;
	int val;
	int fNinch = fFalse;
	int fChp = fFalse;
	int cchgrpprlAll = 0;
	char prl[4];
	struct MPRC prc;
	struct ESPRM esprm;
	struct SIAP siap;

	if (fCab)
		/* rgProp is really hcab in this case */
		iagMac = ((CABH *) *(HCAB)rgProp)->cwSimple;
	else
		iagMac = *rgProp;    /* 1st entry in prb is number of entries */

	fNinch = FNinchForSprmType(mpiagspnt[0].sprm);

	/* build grpprl */
	for (iag = 0; iag < iagMac; ++iag)
		{
		if (fCab)
			val = *((WORD *) (*(HCAB)rgProp) + cwCabMin + iag);
		else
			val = *(rgProp + iag + 1);

		if ((sprm = mpiagspnt[iag].sprm) == sprmNoop)
			{
			continue;  /* skip this ag */
			}

		/* Translate val if necessary... */
		if (sprm == sprmPStc)
			val = vstcStyle;

		if (val == mpiagspnt[iag].wNinchVal)
			{
			continue;  /* skip any ninch ag values */
			}


		/* these two sprms are inactive in DefineStyle dlg up */
		if (fStsh && (sprm == sprmPStc || sprm == sprmPStcPermute))
			{
			continue;
			}

		fChp = dnsprm[sprm].sgc == sgcChp;
		if (!fStsh && !fNinch)
			{
			int valCur;

			if (selCur.fIns && fChp)
				{
				Assert (!selCur.fUpdateChp);
				valCur = ValFromPropSprm(&selCur.chp, sprm);
				}
			else
				valCur = ValFromSprm(sprm);
			if (val == valCur)
				{
				/* skip this ag because it didn't change */
				continue;
				}
			}

		/* build prl */
		prl[0] = sprm;
		esprm = dnsprm[sprm];
		Assert(esprm.cch >= 1 && esprm.cch <= 3);
		switch (esprm.cch)
			{
		/* case 1: no argument to go with sprm */
		case 2:
			/* byte quantity */
			prl[1] = val;
			break;
		case 3:
			/* word quantity */
			bltb(&val, &prl[1], sizeof(int));
			break;
			}
		/* now we have: prl == prl to be merged */
		MergeGrpprls(&prc.grpprl, &cchgrpprlAll, prl, dnsprm[prl[0]].cch);
		}

	/* grpprlEarlier contains grpprl describing properties */
	*pcb = cchgrpprlAll;
	bltb(prc.grpprl, grpprl, cchgrpprlAll);
	*pfChp = fChp;
}


/*  %%Function:  CabToPrb  %%Owner:  bobz       */

CabToPrb(prb, hcab, iagStart, ciag)
WORD *prb;
HCAB hcab;
int iagStart;  /* starting iag for blt */
int ciag;      /* iags to copy */
{
	/* copy entries from cab into prb.  Assumes in sync. Assert in caller */

	blt ( ((WORD *) (*hcab)) + cwCabMin + iagStart, (prb + 1 + iagStart),	
			ciag );
}




/* from FEDT.C */

#include "fedt.h"

extern struct MERR  vmerr;
extern int vgrpfKeyBoardState;

/* Macros to keep the rest of this module as Excel-like as reasonable */

#define fAlt        (wKbsOptionMask)
#define fShift      (wKbsShiftMask)
#define fControl    (wKbsControlMask)
#define FOREVER     for ( ;; )
#define chLF        (chEol)
#define chBackSpace (chBS)
#define chNull      '\0'
#define SzLen(sz)   (CchSz(sz)-1)
#define GrfGetShift() ( SetOurKeyState(), vgrpfKeyBoardState )

#ifdef DEBUG
extern int wErrFedt;
#endif

extern HCURSOR      vhcIBeam;
extern int ichMinRef;

long FedtNotifyParent();
char *PchLastFit();
#ifdef WIN23
char *PchLastFit3();
#endif /* WIN23 */
char *PchPrevTextBrk();
char *PchNextTextBrk();

#define pfedt (*hfedt)



/*----------------------------------------------------------------------------
|    FFedtKeybd
|
|    Handles keyboard interface for the fedt control.
|
|    Arguments:
|        hfedt
|        wm
|        vk
|
|    Returns:
|        TRUE if processed.
|
|    WARNING!! - the passed hfedt may be invalid after calling this
|    routine.
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtKeybd  %%Owner:  bobz       */

BOOL FFedtKeybd(hfedt, wm, vk)
FEDT **hfedt;
int wm;
int vk;
{
	int cch;
	int ichCaret;
	int ichMicSel, ichMacSel;
	int li;
	unsigned grf;
	BOOL fShowSav;
	char rgch[8];
	Debug(CheckFedt(hfedt));
	switch (wm)
		{
	case WM_CHAR:
		grf = GrfGetShift();
#ifdef BRYANL
		if (grf & fAlt)
			CommSz( SzShared("Alt key in FEDT!\r\n") );
#endif
		switch (vk)
			{
		case chBackSpace:
			ichCaret = pfedt->sel.rgichSel[pfedt->fMacCaret];
DoBacksp:
			fShowSav = pfedt->fShowSel;
			FedtShowSel(hfedt, FALSE);
			FedtSetSel(hfedt, ichCaret-1, ichCaret);
			cch = 0;
			goto ReplaceSel2;
		case chReturn:
			vk = chLF;
		default:
			if (vk == chLF && pfedt->fSingleLine)
				return fFalse;
			cch = 1;
			rgch[0] = vk;
ReplaceSel:
			fShowSav = pfedt->fShowSel;
ReplaceSel2:
			FFedtReplSel(&hfedt, (char far *)rgch, cch);
			if (hfedt != NULL)
				{
				FedtShowSel(hfedt, fShowSav);
				FedtAdjustSel(hfedt);
				}
			break;
			}
		return(TRUE);

	case WM_KEYDOWN:
		grf = GrfGetShift();
		switch (vk)
			{
		default:
			if ((uns)(vk - '0') <= ('9' - '0') ||
					(uns)(vk - 'A') <= ('Z' - 'A'))
				{
/* ignore ctrl-alpha/num; must enter ascii codes < 0x32 via alt-# interface */
				if (grf & fControl)
					return fTrue;
/* don't bother checking simple alpha-numeric keys */
				return fFalse;
				}

			return(FFedtKeybdEdit(hfedt, vk, grf));

		case VK_DELETE:
			if (grf & fAlt)
				break;
			ichMicSel = pfedt->ichMicSel;
			ichMacSel = pfedt->ichMacSel;
			if (grf & fControl)
				{
				/* clear to end of line */
				li = pfedt->fMacCaret ? pfedt->liCaret : LiFedtFromIch(hfedt, ichMacSel);
				FedtSetSel(hfedt, ichMicSel, pfedt->mpliichBrk[li]);
				}
			else  if (ichMicSel == ichMacSel)
				{
				/* backwards backspace */
				ichCaret = ichMicSel + 1;
				goto DoBacksp;
				}
			SendMessage(pfedt->hwnd, (grf&fShift) ? WM_CUT : WM_CLEAR, 0, 0L);
			/* WARNING!! - hfedt invalid */
			return(TRUE);

		case VK_INSERT:
			if (grf & fAlt)
				break;
			if (grf & fShift)
				SendMessage(pfedt->hwnd, WM_PASTE, 0, 0L);
			else  if (grf & fControl)
				{
				if (pfedt->ichMicSel != pfedt->ichMacSel)
					SendMessage(pfedt->hwnd, WM_COPY, 0, 0L);
				}
			else
				{
				if ((pfedt->fOverType = !pfedt->fOverType) != 0)
					FedtAdjustSel(hfedt);
				else
					{
					ichCaret = pfedt->ichMicSel;
					if (ichCaret == pfedt->ichMacSel-1)
						FedtSetSel(hfedt, ichCaret, ichCaret);
					}
				}
			/* WARNING!! - hfedt invalid */
			return(TRUE);
			}
		break;
		}

	return(FALSE);
}


/*----------------------------------------------------------------------------
|    FFedtKeybdEdit
|
|    Keyboard interface specific to fedt control edit mode.
|
|    Arguments:
|        hfedt
|        vk
|        grf
|
|    Returns:
|        TRUE if the key was handled.
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtKeybdEdit  %%Owner:  bobz       */

BOOL FFedtKeybdEdit(hfedt, vk, grf)
FEDT **hfedt;
int vk;
unsigned grf;
{
	int ichFix, ichCaret;
	int liCaret;
	char *pch, *pchMic, *pchMac;
	char *pchMin, *pchMax;
	struct PT pt;
	char *PchNextHardBrk(), *PchPrevHardBrk();

	Debug(CheckFedt(hfedt));

	if ((uns)(vk - VK_END) > (VK_DOWN - VK_END) || (grf & fAlt))
		return(FALSE);

	ichFix = pfedt->ichMicSel;
	ichCaret = pfedt->ichMacSel;
	liCaret = pfedt->liCaret;
	if (!pfedt->fMacCaret)
		SwapPw2(&ichFix, &ichCaret);
	if (!(grf & fShift))
		ichFix = ichCaret;
	switch (vk)
		{
	case VK_END:
		/* end of line/end of text */
		if (grf & fControl)
			liCaret = pfedt->liMac-1;
		ichCaret = pfedt->mpliichBrk[liCaret];
		if (FFedtIchAfterHardBrk(hfedt, ichCaret, liCaret))
			ichCaret--;
		break;
	case VK_HOME:
		/* beginning of line/beginning of text */
		if (grf & fControl)
			liCaret = 0;
		ichCaret = pfedt->mpliichBrk[liCaret-1];
		break;
	case VK_LEFT:
	case VK_RIGHT:
		if (grf & fControl)
			{
			pchMin = PchFromHsz(pfedt->hszText);
			pchMax = pchMin + pfedt->cchText;
			pch = pchMin + ichCaret;
			if (vk == VK_LEFT)
				{
				/* previous word */
				pchMic = PchPrevHardBrk(pch - 1, pchMin) + 1;
				if (pch > pchMin && pch == pchMic)
					{
					pch--;
					pchMic = PchPrevHardBrk(pch - 1, pchMin) + 1;
					}

				pch = PchPrevTextBrk(pch - 1, pchMic, FALSE);
				pch = PchPrevTextBrk(pch, pchMic, TRUE) + 1;
				}
			else
				{
				/* next word */
				pchMac = PchNextHardBrk(pch, pchMax);
				pch = PchNextTextBrk(pch, pchMac, TRUE);
				pch = PchNextTextBrk(pch, pchMac, FALSE);
				if (*pch == chLF)
					pch++;
				}
			ichCaret = pch - pchMin;
			Assert(FInRange(ichCaret, 0, pfedt->cchText));
			}
		else
			{
			/* next/previous character */
			ichCaret += (vk-VK_LEFT) - 1;
			}
		liCaret = -2;
		break;
	case VK_UP:
	case VK_DOWN:
		/* next/previous line */
		pt.xp = DxFedtPos(hfedt, liCaret, ichCaret);
		if (pfedt->fOverType && ichCaret < pfedt->cchText)
			pt.xp += DxFedtText(hfedt, ichCaret, 1)/2;
		pt.yp = pfedt->rcFmt.ypTop + vsci.dypTmHeight * (liCaret + (vk-VK_UP)-1);
		FedtHitTest(hfedt, pt, &ichCaret, &liCaret);
		break;
	default:
		return(FALSE);
		}

	if (!(grf & fShift))
		ichFix = ichCaret;
	FedtSetSel2(hfedt, ichFix, ichCaret, ichCaret, liCaret, FALSE);
	FedtAdjustSel(hfedt);
	return(TRUE);
}


/*----------------------------------------------------------------------------
|    FedtTrackSel
|
|    Tracks a mouse drag selection in a fedt control.
|
|    Arguments:
|        hfedt
|        ptHit
|        grf
|        fWordSel
|
|    Returns:
|        nothing.
----------------------------------------------------------------------------*/
/*  %%Function:  FedtTrackSel  %%Owner:  bobz       */

FedtTrackSel(hfedt, ptHit, grf, fWordSel)
FEDT **hfedt;
struct PT ptHit;
unsigned grf;
BOOL fWordSel;
{
	int ichMicHit, ichMacHit;
	int ichMicFix, ichMacFix;
	int liHit;
	HWND hwnd;
	MSG msg;

	Debug(CheckFedt(hfedt));

	hwnd = pfedt->hwnd;
	FedtHitTest(hfedt, ptHit, &ichMicHit, &liHit);
	FedtAdjustWordSel(hfedt, &ichMicHit, &ichMacHit, fWordSel);
	if (grf & MK_SHIFT)
		{
		ichMicFix = ichMacFix = pfedt->sel.rgichSel[1-pfedt->fMacCaret];
		}
	else
		{
		ichMicFix = ichMicHit;
		ichMacFix = ichMacHit;
		}
	FedtExtendSel(hfedt, ichMicFix, ichMacFix, ichMicHit, ichMacHit, liHit);
	FedtAdjustSel(hfedt);
LComboMouse:
	SetFocus(hwnd);
	/* WARNING!! - hfedt invalid */
	if (GetFocus() != hwnd)
		return(0L);
	hfedt = HfedtFromHwnd(hwnd);
	SetCapture(hwnd);
	FOREVER
				{
		if (!PeekMessage((LPMSG)&msg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			{
			msg.message = WM_MOUSEMOVE;
			*(struct PT *)&msg.lParam = ptHit;
			}

		/* WM_QUEUSYNC must get through to CBT or we'll end up in an infinite
			loop, since when they have their journal playback hook installed,
			all hardware input is disabled, and we'll never get the mouse up
			message */
		if (vhwndCBT &&
				PeekMessage((LPMSG)&msg, hwnd, WM_QUEUESYNC, WM_QUEUESYNC, PM_REMOVE))
			{
			ReleaseCapture();
			DispatchMessage((LPMSG)&msg);
			return 0L;
			}

		switch (msg.message)
			{
		case WM_LBUTTONUP:
			ReleaseCapture();
			return(0L);
		case WM_MOUSEMOVE:
			ptHit = *(struct PT *)&msg.lParam;
			FedtHitTest(hfedt, ptHit, &ichMicHit, &liHit);
			FedtAdjustWordSel(hfedt, &ichMicHit, &ichMacHit, fWordSel);
			FedtExtendSel(hfedt, ichMicFix, ichMacFix, ichMicHit, ichMacHit, liHit);
			FedtAdjustSel(hfedt);
			break;
		default:
			TranslateMessage((LPMSG)&msg);
			DispatchMessage((LPMSG)&msg);
			/* just to be safe, re-establish hfedt */
			hfedt = HfedtFromHwnd(hwnd);
			break;
			}
		}
	Assert( fFalse );
}


/*----------------------------------------------------------------------------
|    FedtHitTest
|
|    Determines where the heck the guy clickhed his mouse in the fedt
|    control.
|
|    Arguments:
|        hfedt        fedt to hit test on
|        pt        point to hit test for
|        pich        place to put character position of hit
|        pli        place to put line number of hit
----------------------------------------------------------------------------*/
/*  %%Function:  FedtHitTest  %%Owner:  bobz       */

FedtHitTest(hfedt, pt, pich, pli)
FEDT **hfedt;
struct PT pt;
int *pich;
int *pli;
{
	int li;
	int dx;
	char *pch, *pchMin, *pchMic, *pchMac;
#ifdef WIN23
	int fReleaseHdc;
#endif /* WIN23 */
	Debug(CheckFedt(hfedt));

	li = PegRange((pt.yp - pfedt->rcFmt.ypTop) / vsci.dypTmHeight,
			0, pfedt->liMac-1);
	pchMin = PchFromHsz(pfedt->hszText);
	pchMic = pchMin + pfedt->mpliichBrk[li-1];
	pchMac = pchMin + pfedt->mpliichBrk[li];
	/* don't allow hits on a hard break character */
	if (pchMac > pchMic && *(pchMac-1) == chLF)
		pchMac--;
	pt.xp -= pfedt->rcFmt.xpLeft;
	dx = pt.xp;
#ifdef WIN23
	if (vsci.fWin3)
		{
		if ((fReleaseHdc = (*hfedt)->hdc == NULL) && !FFedtValidateDC(hfedt))
			pch = pchMic;
		else
			{
			pch = PchLastFit3(pfedt->hdc, vsci.dxpTmWidth, pchMic, pchMac, &dx);
			if (fReleaseHdc)
				FedtReleaseDC(hfedt);
			}
		}
	else
#endif /* WIN23 */
	pch = PchLastFit(vsci.dxpTmWidth, pchMic, pchMac, &dx);
	if (pch < pchMac)
		{
		if (pfedt->fOverType || dx + DxFedtText(hfedt, pch - pchMin, 1)/2 < pt.xp)
			pch++;
		}

	*pich = pch - pchMin;
	*pli = li;
}


/*----------------------------------------------------------------------------
|    FedtAdjustSel
|
|    Adjusts the selection in the fedt control, taking overtype mode
|    into account.
|
|    Arguments:
|        hfedt        fedt control whose selection needs adjusting
----------------------------------------------------------------------------*/
/*  %%Function:  FedtAdjustSel  %%Owner:  bobz       */

FedtAdjustSel(hfedt)
FEDT **hfedt;
{
	int ichSel;

	Debug(CheckFedt(hfedt));
	ichSel = pfedt->ichMicSel;
	if (pfedt->fOverType && abs(ichSel - pfedt->ichMacSel) <= 1)
		{
		FedtSetSel2(hfedt, ichSel, ichSel+1, ichSel, pfedt->liCaret, FALSE);
		}
}


/*----------------------------------------------------------------------------
|    FedtAdjustWordSel
|
|    Adjusts selection points to be a full word selection.
|
|    Arguments:
|        hfedt
|        pichMic        on entry, contains hit point
|        pichMac
|        fWordSel
----------------------------------------------------------------------------*/
/*  %%Function:  FedtAdjustWordSel  %%Owner:  bobz       */

FedtAdjustWordSel(hfedt, pichMic, pichMac, fWordSel)
FEDT **hfedt;
int *pichMic, *pichMac;
BOOL fWordSel;
{
	char *pchText, *pchHit;
	char *pchMic, *pchMac;
	char *PchNextHardBrk(), *PchPrevHardBrk();

	Debug(CheckFedt(hfedt));
	if (!fWordSel)
		{
		*pichMac = *pichMic;
		return;
		}
	pchText = PchFromHsz(pfedt->hszText);
	pchHit = pchText + *pichMic;
	pchMic = PchPrevHardBrk(pchHit - 1, pchText) + 1;
	pchMac = PchNextHardBrk(pchHit, pchText + pfedt->cchText);
	*pichMic = max(PchPrevTextBrk(pchHit - 1, pchMic, TRUE) + 1 - pchText, 0);
	*pichMac = min(PchNextTextBrk(pchText + *pichMic, pchMac, TRUE) - pchText, pfedt->cchText);
}


/*----------------------------------------------------------------------------
|    FedtExtendSel
|
|    Extends the selection away from a fixed range to include a new
|    range.
|
|    Arguments:
|        hfedt
|        ichMicFix    fixed selection range
|        ichMacFix
|        ichMic        new selection range to include
|        ichMac
|        liHit        line position of last hit point
----------------------------------------------------------------------------*/
/*  %%Function:  FedtExtendSel  %%Owner:  bobz       */

FedtExtendSel(hfedt, ichMicFix, ichMacFix, ichMic, ichMac, liHit)
FEDT **hfedt;
int ichMicFix, ichMacFix, ichMic, ichMac;
int liHit;
{
	int ichCaret;

	Debug(CheckFedt(hfedt));
	ichCaret = 0x7fff;
	if (ichMic > ichMicFix)
		ichMic = ichMicFix;
	else
		ichCaret = ichMic;
	if (ichMac < ichMacFix)
		ichMac = ichMacFix;
	else
		ichCaret = ichMac;
	FedtSetSel2(hfedt, ichMic, ichMac, ichCaret, liHit, TRUE);
}


/*----------------------------------------------------------------------------
|    FedtScroll
|
|    Scrolls a FEDT control.
|
|    Arguments:
|        hfedt        fedt control to scroll
|        dx        horizontal distance to scroll
|        dy        vertical distance to scroll
|
|    Returns:
|        nothing.
----------------------------------------------------------------------------*/
/*  %%Function:  FedtScroll  %%Owner:  bobz       */

FedtScroll(hfedt, dx, dy)
FEDT **hfedt;
int dx, dy;
{
	struct RC rc;
	struct RC far *lprc;

	Debug(CheckFedt(hfedt));
	if (dx == 0 && dy == 0)
		return;
	/* BUG!! - we really should use scrolling to do this */
	OffsetRect( (LPRECT *)&pfedt->rcFmt, dx, dy );
	if (pfedt->fSingleLine)
		pfedt->rcFmt.xpRight = pfedt->rcView.xpRight;
	FedtComputeRgnSel(hfedt);
	HideCaret(pfedt->hwnd);
	FedtRedraw(hfedt, 0, pfedt->cchText);
	ShowCaret(pfedt->hwnd);
	Debug(CheckFedt(hfedt));
}


/*----------------------------------------------------------------------------
|    FFedtReplSel
|
|    Replaces the selection of the given fedt control with the given text.
|
|    Arguments:
|        phfedt        fedt control to replace text in.  if
|                	reallocated, will return the new hfedt;
|			WARNING: May return NULL if OOM notification
|			to SDM parent caused window to be destroyed.
|        lpch        pointer to new text
|        cch        length of text in lpch
|
|    Returns:
|        TRUE if successful.
----------------------------------------------------------------------------*/
/*  %%Function:  FFedtReplSel  %%Owner:  bobz       */


BOOL FFedtReplSel(phfedt, lpch, cch)
FEDT ***phfedt;
char far *lpch;
int cch;
{
	int ich, ichBrk, ichMicDraw, ichMacDraw;
	FEDT *pfedtT;
	int dch, cchSel;
	struct PT pt;
	int li, liOld;
	struct RC rc;
	BOOL fShowSav, fSuccess;
	char *pchText, *pchMic, *pchMac;
	char **hrgch;

	Debug(CheckFedt(*phfedt));

/* grow the text buffer to hold the new text, if necessary */

	pfedtT = **phfedt;
	cchSel = pfedtT->ichMacSel - pfedtT->ichMicSel;
	dch = cch - cchSel;
	if (dch > 0)
		{
		int cchNew = pfedtT->cchText + dch + 1;

		FOREVER
					{
			if (FChngSizeHCw( (**phfedt)->hszText, CwFromCch(cchNew),fFalse))
				break;
			switch (LOWORD(FedtNotifyParent(phfedt, FN_ERRSPACE|FN_OOMTEXT)))
				{
			case FN_OOMALERT:
				SetErrorMat(matMem);
			case FN_OOMABORT:
			case FN_OOMIGNORE:
				return(FALSE);
			case FN_OOMRETRY:
				break;
				}
			Assert( *phfedt != NULL && IsWindow( (**phfedt)->hwnd ) );
			}
		}

/* turn off the selection while we're mangling ourselves */

	fShowSav = (**phfedt)->fShowSel;
	FedtShowSel(*phfedt, FALSE);

/* patch up text buffer, line break array, and move the selection */

	pfedtT = **phfedt;
	ichMicDraw = pfedtT->ichMicSel;
	ichMacDraw = pfedtT->ichMacSel;
	liOld = LiFedtFromIch(*phfedt, ichMicDraw);
	pchText = PchFromHsz(pfedtT->hszText);
	pchMic = pchText + ichMicDraw;
	pchMac = pchText + ichMacDraw;
	bltb(pchMac, pchMac + dch, pfedtT->cchText - ichMacDraw + 1);
	bltbx(lpch, (char far *)pchMic, cch);
	pfedtT->cchText += dch;
	/* patch up line break array */
	for (li = liOld; li < pfedtT->liMac; li++)
		pfedtT->mpliichBrk[li] += dch;
	/* set selection in-line for speed and to bypass validity checks */
	pfedtT->ichMicSel = pfedtT->ichMacSel = ichMicDraw + cch;
	pfedtT->fMacCaret = TRUE;

/* reformat and redisplay the control, and turn the selection back on */

	fSuccess = FFedtReformat(phfedt, ichMicDraw, max(ichMacDraw, ichMacDraw + dch));
	if (*phfedt == NULL)
		return fFalse;
	FedtShowSel(*phfedt, fShowSav);
	FedtNotifyChange(*phfedt);

/* we made it! - return success */

	Debug(CheckFedt(*phfedt));
	return(fSuccess);
}


/*----------------------------------------------------------------------------
|    PchPrevHardBrk
|
|    Scans for previous hard break character.
|
|    Arguments:
|        pch        place to start scan
|        pchMic        last character position to check
|
|    Returns:
|        pointer to previous hard break character, or pchMic-1 if
|        none found.
----------------------------------------------------------------------------*/
/*  %%Function:  PchPrevHardBrk  %%Owner:  bobz       */

char *PchPrevHardBrk(pch, pchMic)
char *pch;
char *pchMic;
{
	while (pch >= pchMic)
		{
		if (*pch == chLF)
			break;
		pch--;
		}
	return(pch);
}


/*---------------------------------------------------------------------*/
/*----------------- Excel-equivalent routines -------------------------*/
/*---------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
|    CchCountCh
|
|    Counts characters in a string
|
|    Arguments:
|        pch        pointer to string
|        pchMax        end of string
|        ch        character to count
|
|    Returns:        number of ch characters in string
----------------------------------------------------------------------------*/
/*  %%Function:  CchCountCh  %%Owner:  bobz       */

CchCountCh(pch, pchMax, ch)
char *pch;
char *pchMax;
int ch;
{
	int cch=0;

	while (pch<pchMax)
		{
		if (*pch++==ch)
			cch++;
		}
	return(cch);
}


/*  %%Function:  SwapPw2  %%Owner:  bobz       */

SwapPw2(pw1, pw2)
int *pw1, *pw2;
{
	int w = *pw1;
	*pw1 = *pw2;
	*pw2 = w;
}


/*------------------------------------------------------------------------
|    SizeWnRc
|
|    Little routine for moving and sizing a window.
|
|    Arguments:
|        hwnd        Window to resize
|        prc        Bounding rect of the new window size/pos
|        fRedraw     To redraw the window when we're done
------------------------------------------------------------------------*/
/*  %%Function:  SizeWnRc  %%Owner:  bobz       */

SizeWnRc(hwnd, prc, fRedraw)
HWND hwnd;
struct RC *prc;
BOOL fRedraw;
{
	Assert(hwnd != NULL);
	SetWindowPos(hwnd, NULL, prc->xpLeft, prc->ypTop,
			prc->xpRight - prc->xpLeft, prc->ypBottom - prc->ypTop,
			fRedraw ? SWP_NOZORDER|SWP_NOACTIVATE :
			SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
}


/*----------------------------------------------------------------------------
|	PchLastFit
|
|	Calculates the position of the last character that will fit in 
|	the given range.
|
|	Arguments:
|		dxChar		width of character
|		pchMic		first character in of string
|		pchMac		last character of string
|		pdxMax		width of area to do checks in - returns
|				width of text that fit
|
|	Returns:
|		pointer to last character that fits, pchMac if they
|		all fit.
|
|	Note:
|		The speed of this routine is critical to fedt control 
|		snap-and-feel, so we go to some effort to be fast here.
----------------------------------------------------------------------------*/
/*  %%Function:  PchLastFit  %%Owner:  bobz       */

char *PchLastFit(dxChar, pchMic, pchMac, pdxMax)
int dxChar;
char *pchMic, *pchMac;
int *pdxMax;
{{ /* NATIVE - PchLastFit */
	int dx, dxMac, dxMax;
	char *pchGuess, *pchMin;

	Assert(pchMac >= pchMic);
	dxMax = *pdxMax;
	pchMin = pchMic;
	pchMic += dxMax / dxChar;
	if (pchMic > pchMac)
		pchMic = pchMac;
	if (pchMic < pchMin)
		pchMic = pchMin;
	*pdxMax = (pchMic - pchMin) * dxChar;
	return(pchMic);
}}

#ifdef WIN23
char *PchLastFit3(hdc, dxChar, pchMic, pchMac, pdxMax)
HDC hdc;
int dxChar;
char *pchMic, *pchMac;
int *pdxMax;
{{ /* NATIVE - PchLastFit */
	int dx, dxMic, dxMac, dxMax;
	char *pchGuess, *pchMin;

	Assert(pchMac >= pchMic);
	dxMic = 0;
	dxMax = *pdxMax;
	pchMin = pchMic;

/* if fixed pitch font, just use a division to find fit point */

#ifdef REVIEW  /* let's not special case fixed-pitch for now */
	if (dxChar < 0)
		{
		pchMic += dxMax / (-dxChar);
		if (pchMic > pchMac)
			pchMic = pchMac;
		if (pchMic < pchMin)
			pchMic = pchMin;
		dxMic = (pchMic - pchMin) * (-dxChar);
		goto Return;
		}
#endif // REVIEW

	dxMac = dxChar * (pchMac - pchMic);

	while (pchMac > pchMic)
		{

/* guess the fit position - KLUDGE!! - binary searches that guess either
   end-point of the search range don't work; we, however, will guess pchMac 
   because it's a fabulous optimization in the short text case */

		pchGuess = pchMic + NMultDiv(pchMac - pchMic, dxMax - dxMic,
				dxMac - dxMic);
		if (pchGuess > pchMac)
			pchGuess = pchMac;
		if (pchGuess < pchMic + 1)
			pchGuess = pchMic + 1;
TryAgain:
		dx = LOWORD(GetTextExtent(hdc, (LPSTR)pchMin, pchGuess - pchMin));
		if (dx > dxMax)
			{

/* guess is too long - if we added 1 char and now don't fit, we're done */

			if (pchGuess == pchMic + 1)
				break;
			dxMac = dx;
			/* if our pchGuess == pchMac kludge optimization has
			   screwed us over, try again like we should have in 
			   the first place */
			if (pchMac == pchGuess)
				{
				pchGuess--;
				goto TryAgain;
				}
			pchMac = pchGuess;
			}
		else
			{

/* guess was too short - move Mic end-point */

			pchMic = pchGuess;
			dxMic = dx;
			}
		}

/* dxMic == actual length of text, which might be useful to someone */
Return:
	*pdxMax = dxMic;
	return(pchMic);
}}
#endif /* WIN23 */

/*  %%Function:  PchPrevTextBrk  %%Owner:  bobz       */

char *PchPrevTextBrk(pch, pchMic, fBreak)
char *pch;
char *pchMic;
BOOL fBreak;
{{ /* NATIVE - PchPrevTextBrk */
	int ch;

	for ( ; pch >= pchMic; pch--)
		{
		ch = *pch;
		if ((ch == ' ' || ch == '-') == fBreak)
			break;
		}
	return(pch);
}}


/*  %%Function:  PchNextTextBrk  %%Owner:  bobz       */

char *PchNextTextBrk(pch, pchMac, fBreak)
char *pch;
char *pchMac;
BOOL fBreak;
{{ /* NATIVE - PchNextTextBrk */
	int ch;

	for ( ; pch < pchMac; pch++)
		{
		ch = *pch;
		if ((ch == ' ' || ch == '-') == fBreak)
			break;
		}
	return(pch);
}}


