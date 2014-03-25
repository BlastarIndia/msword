/*  code to handle prompt-line input & output  */


#ifdef DEBUG
#ifdef PCJ
/* #define SHOWDISPATCH */
/* #define SHOWDROP */
/* #define SHOWWNDPROC */
#endif /* PCJ */
#endif

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "prompt.h"
#include "screen.h"
#include "wininfo.h"
#include "cmdtbl.h"
#include "opuscmd.h"
#include "keys.h"
#include "props.h"
#include "sel.h"
#include "status.h"
#include "help.h"
#include "core.h"
#include "message.h"
#include "idle.h"
#include "dde.h"
#include "doc.h"
#include "file.h"
#include "filecvt.h"
#include "print.h"
#ifdef WIN23
#include "resource.h"
#endif /* WIN23 */
#ifdef PROTOTYPE
#include "prompt.cpt"
#endif /* PROTOTYPE */

/* G L O A B L S */
HWND            vhwndPrompt = (HWND) NULL;
HWND            vhwndPromptHidden = (HWND) NULL;
long            vusecTimeoutPrompt = 0L;
BOOL            vfRestPmtOnInput = fFalse;
CHAR            vstPrompt [cchPromptMax];
int             vpisPrompt = pisNormal;
struct PIR     *vppirPrompt = NULL;
int             vcchStatLinePrompt = 0;
struct PPR    **vhpprPRPrompt = hNil;
#ifdef WIN23
extern int          vdbmgDevice;
extern int		vdxpDigit;
#endif /* WIN23 */

/* if non-nil, return the focus to this window when returning
					to the application */
HWND            vhwndAppModalFocus = NULL;


/* E X T E R N A L S */
extern struct SCI  vsci;
extern struct SEL  selCur;
extern BOOL        vfDdeIdle;
extern int         vssc;
extern HANDLE      vhInstance;
extern HWND        vhwndApp;
extern int         vfAwfulNoise;
extern int         vfDeactByOtherApp;
extern MSG         vmsgLast;
extern HWND        vhwndAppModalFocus;
extern int         vhwndPrevFocus;
extern KMP **      hkmpCur;
extern CHAR        stEmpty[];
extern CHAR        szEmpty[];
extern HWND        vhwndStatLine;
extern HWND        vhwndCBT;
extern HCURSOR     vhcArrow;
extern struct PREF vpref;
extern int	   wwCur;
extern char szClsPrompt[];
extern HWND        vhwndPrompt;
extern HWND        vhwndPromptHidden;
extern long        vusecTimeoutPrompt;
extern BOOL        vfRestPmtOnInput;
extern CHAR        vstPrompt[];
extern int         vpisPrompt;
extern struct PIR *vppirPrompt;
extern int         vcchStatLinePrompt;
extern struct PPR **vhpprPRPrompt;
extern IDF	   vidf;
extern BOOL	   fElActive;
extern BOOL	   vfElDisableInput;
extern struct MERR  vmerr;
extern struct PRI vpri;
extern int vfInFormatPage;

#ifdef WIN23
extern HFONT	   hfontStatLine;
#endif /* WIN23 */


/* S E T  P R O M P T  S T */
/*  Sets the prompt to read st with pdc.
*/
/* %%Function:SetPromptSt %%Owner:PETERJ */
SetPromptSt (st, pdc)
CHAR *st;
int pdc;

{
	int cch = min (*st+1, cchPromptMax);
	bltb (st, vstPrompt, cch);
	vstPrompt[0] = cch-1;

	DisplayPrompt (pdc, hNil);
}



/* S E T  P R O M P T  M S T */
/*  Set the prompt text to be mst (mst is not built).
*/
/* %%Function:SetPromptMst %%Owner:PETERJ */
SetPromptMst (mst, pdc)
MST mst;
int pdc;

{
	bltbx ((CHAR FAR *)mst.st, (CHAR FAR *)vstPrompt, *(mst.st)+1);
	DisplayPrompt (pdc, hNil);
}



/* S E T  P R O M P T  B U I L T  M S T  W */
/*  Builds prompt string based on mst and w.  Displays in prompt according to
	pdc.
*/

/* %%Function:SetPromptWMst %%Owner:PETERJ */
SetPromptWMst (mst, w, pdc)
MST mst;
unsigned w;
int pdc;

{
	BuildStMstRgw (mst, &w, vstPrompt, cchPromptMax, hNil);
	DisplayPrompt (pdc, hNil);
}



/* S E T  P R O M P T  B U I L T  M S T  R G W */
/*  Builds prompt based on mst and rgw.  Displays in prompt according to pdc.
*/
/* %%Function:SetPromptRgwMst %%Owner:PETERJ */
SetPromptRgwMst (mst, rgw, pdc)
MST mst;
unsigned *rgw;
int pdc;

{
	BuildStMstRgw (mst, rgw, vstPrompt, cchPromptMax, hNil);
	DisplayPrompt (pdc, hNil);
}



/* B U I L D  S T  M S T  R G W */
/*  Builds an st based on mst and data in rgw.  See comment and usage in
	message.h.  If a "marker" flag is in mst and hppr is not hNil it will
	be set up for reporting.
*/
/* %%Function:BuildStMstRgw %%Owner:PETERJ */
BuildStMstRgw (mst, rgw, st, cchMax, hppr)
MST mst;
unsigned *rgw;
CHAR *st;
int cchMax;
struct PPR **hppr;
{
	int cch;
	CHAR chCode, chWidth;
	CHAR *pch, *pchMac;
	CHAR *pchMstSt;
	CHAR *pchMstStMac;
	CHAR szName[ichMaxFile];
#ifdef WIN23
	HFONT hfontOld;
	HDC hdc;
#endif /* WIN23 */

	pch = st + 1;
	pchMac = pch + cchMax - 1;
	pchMstSt = mst.st + 1;
	pchMstStMac = mst.st + (*mst.st + 1);

	for (;;)  /* process chars & each special object */
		{
		Assert (pch < pchMac);
			{{ /* NATIVE - BuildStMstRgw */
			while (pchMstSt < pchMstStMac && (*pch++ = *pchMstSt++) >= ' '
					&& pch < pchMac)
				;
			}}

		if (pchMstSt >= pchMstStMac)
			break;

		chCode = *(--pch);
		chWidth = min (*pchMstSt++, pchMac - pch - 1);

		switch (chCode)
			{
		case '\037':     /* reporting marker */
				{
				if (hppr != hNil)
					{
					(*hppr)->ich = pch - st;
					(*hppr)->cch = chWidth;
					Assert(chWidth > 0 && chWidth <= 6);
#ifdef WIN23
					/*
					 * To get the correct position for placing update
					 * numbers, we have to get the pixel offset for the
					 * field + the dead space at the beginning of line.
					 */
					if (vsci.fWin3)
						{
						if ((hdc = GetDC( vhwndPrompt )) != NULL)
							{
							if (hfontStatLine)
								hfontOld = SelectObject( hdc, hfontStatLine );
							(*hppr)->xp = xpPromptLeft + 1+ LOWORD( 
								GetTextExtent( hdc, (LPSTR) (st+1), (pch - st - 2) ));
							if (hfontStatLine && hfontOld)
								SelectObject( hdc, hfontOld );
							ReleaseDC( vhwndPrompt, hdc );
							}
						}
#endif /* WIN23 */
					}
				break;
				}

		case '\001':     /* *rgw is psz.  Truncate to chWidth */
				{
				CHAR *psz = *rgw++;
				if (psz)
					{
					cch = CchSz (psz) - 1;
					if (chWidth && cch > chWidth)
						cch = chWidth;
					cch = min (cch, pchMac-pch);
					bltb (psz, pch, cch);
					pch += cch;
					}
				break;
				}

		case '\010':   /* *rgw is an optional int */
			if (!*rgw)
				{
				*rgw++;
				break;
				}
			/* else fall through */

		case '\002':     /* *rgw is int. Right justify in field chWidth */
				{
				CHAR *pchT = pch;
				cch = CchIntToPpch(*rgw++, &pchT);
				goto LRJustField;
				}

		case '\003':    /* at rgw is long.  Right justify in field chWidth */
				{
				CHAR *pchT = pch;
				cch = CchLongToPpch(*(long *)rgw, &pchT);
				rgw += sizeof(long)/sizeof(int);
				}
LRJustField:
				{
				int di;
				Assert (pch + chWidth < pchMac);
				if (chWidth && cch < chWidth)
					{
					bltb (pch, pch + (di = chWidth - cch), cch);
					SetBytes (pch, ' ', di);
					}
				pch += chWidth ? chWidth : cch;
				break;
				}

		case '\004':     /* *rgw is pstFile. Truncate LEAF to chWidth */
				{
				if (*rgw)
					{
					SzShortFromStNormFile (szName, *rgw++);
					goto InsertDocName;
					}
				break;
				}

		case '\005':     /* *rgw is za. Right justify in field chWidth */
				{
				CHAR *pchT = pch;
				cch = CchExpZa(&pchT, *rgw++, vpref.ut, pchMac-pch, fTrue);
				goto LRJustField;
				}

		case '\006':    /* *rgw is doc, get short form name */
				{
				GetDocSt(DocMother(*rgw++), szName, gdsoShortName);
				StToSzInPlace(szName);
InsertDocName:
				cch = CchSz (szName) - 1;
				if (chWidth && cch > chWidth)
					cch = chWidth;
				cch = min (cch, pchMac-pch);
				bltb (szName, pch, cch);
				pch += cch;
				break;
				}

		case '\007':   /* *rgw is pst.  Truncate to chWidth */
				{
				CHAR *pst = *rgw++;
				if (pst)
					{
					cch = *pst;
					if (chWidth && cch > chWidth)
						cch = chWidth;
					cch = min (cch, pchMac-pch);
					bltb(pst+1, pch, cch);
					pch += cch;
					}
				break;
				}

		default:
			Assert (fFalse);
			break;
			}

		} /* for (;;) */

	Assert (pch <= pchMac);
	st[0] = pch - st - 1;
}




/* %%Function:AbortPmt %%Owner:PETERJ */
void AbortPmt ()
	/*  used on ESCAPE during abort check */
{
	Assert (vpisPrompt >= pisAbortCheckMin);
	if (vpisPrompt&1)
		vpisPrompt++;
}


/* I N I T  A B O R T  C H E C K */
/*  Initializes a modeless abort check mechanism.
	Call SetPromptMst or SetPromptSt before or after this call to set the
	prompt text.  WARNING:  do not RestorePrompt() or destroy the prompt
	before calling TerminateAbortCheck().
*/
/* %%Function:InitAbortCheck %%Owner:PETERJ */
InitAbortCheck ()
{
	if (vpisPrompt != pisNormal)
		/* nested abort check */
		{
		Assert (vpisPrompt >= pisAbortCheckMin);
		vpisPrompt += 2;
		return;
		}

	vpisPrompt = pisAbortCheckMin;
}



/* T E R M I N A T E  A B O R T  C H E C K */
/*  Terminates the AbortCheck mode & sets prompt to go away according
	to pdc.  If pdc & pdcmRestore, restores the prompt.
*/
/* %%Function:TerminateAbortCheck %%Owner:PETERJ */
TerminateAbortCheck (pdc)
int pdc;
{
	if (vpisPrompt < pisAbortCheckMin)
		/*  can't terminate mode we are not in */
		return;

	if (vpisPrompt >= pisAbortCheckMin + 2)
		/*  we are in nested abortchecks */
		{
		vpisPrompt -= 2;
		return;
		}

	vpisPrompt = pisNormal;

	SetPromptRestore (pdc);
}



/* F  Q U E R Y  A B O R T  C H E C K */
/*  Check to see if there are any messages that should abort processing.
	Return TRUE if the caller SHOULD abort.
*/
/* %%Function:FQueryAbortCheckProc %%Owner:PETERJ */
NATIVE FQueryAbortCheckProc ()
{
	int w;

	Assert(vpisPrompt >= pisAbortCheckMin);

	OurYield(fFalse);

	if (FCheckAbortKey())
		AbortPmt();

	Assert (vpisPrompt >= pisAbortCheckMin);
	return (!(vpisPrompt & 1));
}


/* %%Function:FCheckAbortKey %%Owner:PETERJ */
FCheckAbortKey()
	/* CSNative as checked before executing every macro command */
{{ /* NATIVE - FCheckAbortKey */

	int message;
	MSG msg;
	int fCtrlKey, fAltKey;

	/* OurYield is called first and will flush out other
			keys in the buffer. It will flush syskeydowns other
			than alt, tab, escape and shift 
		*/
	/* note bz. This checks if the keys were pressed since the last
			time we called this routine.  We must call GetAsyncKeyState
		everytime in order to get "correct" results, hence the use
		of the seemingly unnecessary variables.
		*/

	fAltKey = GetAsyncKeyState(VK_MENU);
	fCtrlKey = GetAsyncKeyState(VK_CONTROL);

	if ( !vfDeactByOtherApp && vhwndCBT == NULL &&
			(GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_CANCEL)) )
		{  /* flush the queue.... */
		if (fAltKey || fCtrlKey)
			return fFalse;

		while (PeekMessage((LPMSG) &msg, NULL, 
				WM_NCMOUSEMOVE, WM_MOUSELAST, PM_REMOVE))
			;

		return fTrue;
		}


	return fFalse;
}}


/* F O R C E  A B O R T  C H E C K */
/*  Cause future calls to FQueryAbortCheck to return true.  Simulates the
	user typing ESCAPE.
*/
/* %%Function:ForceAbortCheck %%Owner:PETERJ */
ForceAbortCheck ()
{
	if (vpisPrompt >= pisAbortCheckMin && (vpisPrompt&1))
		vpisPrompt++;
}



/*  %%Function: HstCreate   %%Owner: peterj  */
CHAR (**HstCreate(st))[]
CHAR st[];
{
	CHAR (**hst)[];

	int cch = st[0] + 1;
	hst = (CHAR (**)[]) HAllocateCb(cch);
	if (hst != hNil)
		bltbyte(st, **hst, cch);
	return hst;
}


/* P R O G R E S S  R E P O R T I N G */


/* H P P R  S T A R T  P R O G R E S S  R E P O R T */
/*  Sets up hppr and prompt for subsequent calls to ProgressReportPercent or
	ChangeProgressReport.
*/
/* %%Function:HpprStartProgressReport %%Owner:PETERJ */
struct PPR **HpprStartProgressReport (mst, rgw, nIncr, fAbortCheck)
MST mst;
unsigned *rgw;
int nIncr;
BOOL fAbortCheck;
{
	struct PPR **hppr;

	Assert (nIncr != 0);

	if (fElActive && vfElDisableInput)
		{
		fAbortCheck = fFalse;
		GetMstStNoAbort(&mst);
		}

	FAbortNewestCmg (cmgPromptCode, fTrue, fFalse);
	FAbortNewestCmg (fAbortCheck?cmgPromptUtilAC:cmgPromptUtilNAC, fTrue, fFalse);

	if ((hppr = HAllocateCb(sizeof(struct PPR))) == hNil)
		return hNil;
	SetBytes (*hppr, 0, sizeof(struct PPR));

	if (vhpprPRPrompt)
		/* save away previous state */
		{
		Assert(vhwndPrompt);
		(*hppr)->hpprPrev = vhpprPRPrompt;
		/* it's ok if this fails */
		(*hppr)->hstPrev = HstCreate(vstPrompt);
		}
	BuildStMstRgw (mst, rgw, vstPrompt, cchPromptMax, hppr);
	Assert ((*hppr)->cch != 0 || (*hppr)->ich == 0);
	vhpprPRPrompt = hppr;
	DisplayPrompt (pdcAbortCheck, hppr);
	AssertH(vhpprPRPrompt); /* in case someone accidentally destroyed it*/
	if (vhwndPrompt == NULL)
		{
		FreePh(&vhpprPRPrompt);
		vstPrompt[0] = 0;
		return hNil;
		}
	(*hppr)->nIncr = nIncr;
	(*hppr)->nLast = 0;
	if ((*hppr)->fAbortCheck = fAbortCheck)
		InitAbortCheck ();
	return hppr;
}


#ifdef NOASM
/* L  M U L T  D I V */
long LMultDiv (l, lNumer, lDenom)
long l, lNumer, lDenom;
{
	Assert (lDenom != 0);
	return ((l * lNumer)/lDenom);
}


/* P R O G R E S S  R E P O R T  P E R C E N T */
/* %%Function:ProgressReportPercent %%Owner:PETERJ */
ProgressReportPercent (hppr, lLow, lHigh, l, plNext)
struct PPR **hppr;
long lLow, lHigh, l, *plNext;
{
	if (hppr == hNil) return;
	ChangeProgressReport (hppr, (uns)LMultDiv (l-lLow, (long)100, lHigh-lLow));
	if (plNext != NULL)
		*plNext = LMultDiv(lHigh-lLow, (*hppr)->nLast+((*hppr)->nIncr>>1), 100)+lLow;
}


/* C H A N G E  P R O G R E S S  R E P O R T */
/* %%Function:ChangeProgressReport %%Owner:PETERJ */
ChangeProgressReport (hppr, nNew)
struct PPR **hppr;
unsigned nNew;
{
	struct PPR *pppr;
	unsigned nIncr;
	CHAR rgch [6], *pch;
	int cch;

	if (hppr == hNil) return;

	if (hppr != vhpprPRPrompt)
		PopToHppr(hppr);

	pppr = *hppr;
	nIncr = pppr->nIncr;

	Assert (pppr->cch <= 6);

	if (nIncr != 1)
		nNew = (((nNew + (nIncr>>1)) / nIncr) * nIncr);
	if (nNew != pppr->nLast)
		{
		pppr->nLast = nNew;
		pch = &rgch[6];
		cch = pppr->cch;
		do
			{
			*(--pch) = '0' + nNew%10;
			nNew /= 10;
			cch--;
			}
		while (nNew && cch);

		while (cch--)
			*(--pch) = ' ';
		Assert (&rgch[6] - pch == pppr->cch);
#ifdef WIN23
		AdjustPrompt (pppr->ich, pppr->cch, pppr->xp, pch);
#else
		AdjustPrompt (pppr->ich, pppr->cch, pch);
#endif /* WIN23 */
		}
}


#endif /* NOASM */



/* S T O P  P R O G R E S S  R E P O R T */
/* %%Function:StopProgressReport %%Owner:PETERJ */
StopProgressReport (hppr, pdc)
struct PPR **hppr;
{
	PopToHppr(hppr == hNil ? hNil : (*hppr)->hpprPrev);
	if (!vhpprPRPrompt)
		SetPromptRestore (pdc);
}


/* P O P  T O  H P P R */
/* %%Function:PopToHppr %%Owner:PETERJ */
EXPORT PopToHppr (hppr)
struct PPR **hppr;
{
	struct PPR **hpprPrev = hNil;

	while (vhpprPRPrompt != hppr)
		{
		if (!vhpprPRPrompt)
			{
			Assert(fFalse);
			break;
			}
		if ((*vhpprPRPrompt)->fAbortCheck)
			TerminateAbortCheck (0);
		if ((hpprPrev = (*vhpprPRPrompt)->hpprPrev) != hNil)
			{
			CHAR **hst = (*vhpprPRPrompt)->hstPrev;
			if (hpprPrev == hppr)
				if (hst != hNil)
					{
					**hst = min(**hst, cchPromptMax-1);
					CopySt (*hst, vstPrompt);
					}
				else
					vstPrompt[0] = 0;
			FreeH (hst);
			}
		FreeH (vhpprPRPrompt);
		vhpprPRPrompt = hpprPrev;
		}
	if (vhpprPRPrompt)
		DisplayPrompt(pdcAbortCheck, vhpprPRPrompt);
}



/* A D J U S T  P R O M P T */
/*  Changes cch characters starting at ich in the current prompt to text
	in pch.
*/
/* %%Function:AdjustPrompt %%Owner:PETERJ */
#ifdef WIN23
AdjustPrompt2 (ich, cch, pch)
#else
EXPORT AdjustPrompt (ich, cch, pch)
#endif /* WIN23 */
int ich, cch;
CHAR *pch;
{
	struct RC rc;
	int xp;
	HDC hdc = NULL;

	bltb (pch, &vstPrompt[ich], cch);

	Assert (ich != 0);

	if (vhwndPrompt != NULL &&
			FSetDcAttribs ((hdc = GetDC(vhwndPrompt)), dccPrompt))
		{
		GetClientRect (vhwndPrompt, (LPRECT)&rc);
		xp = ((ich-1) * vsci.dxpTmWidth) + vsci.dxpBorder;
		rc.xpLeft = xp;
		rc.xpRight = xp + (cch * vsci.dxpTmWidth);
		ExtTextOut (hdc, xp, vsci.dypBorder, ETO_OPAQUE, (LPRECT)&rc,
				(LPSTR)pch, cch, NULL);
		UpdateWindow(vhwndPrompt);
		}
	if (hdc != NULL)
		ReleaseDC (vhwndPrompt, hdc);
}
#ifdef WIN23
/**************************************************************************
Switch routine for Win2/Win3 versions.
**************************************************************************/

EXPORT AdjustPrompt (ich, cch, xp, pch)
int ich, cch;
short xp;
CHAR *pch;
{
	if (vsci.fWin3)
		AdjustPrompt3 (ich, cch, xp, pch);
	else
		AdjustPrompt2 (ich, cch, pch);
}

/*-----------------------------------------------------------------------
 * A D J U S T  P R O M P T 
 * %%Function:AdjustPrompt %%Owner:RICKS
 *
 * Changes cch characters starting at ich in the current prompt to text
 *	in pch.  Position of field in window begins at xpStart.
 * Since WIN3uses proportional spacing, the xp position of the update
 * field is also passed in.  
 *-----------------------------------------------------------------------*/
AdjustPrompt3 (ich, cch, xp, pch)
int ich, cch;
short xp;
CHAR *pch;
{
	struct RC rc;
	HDC hdc = NULL;
	HFONT hfontOld;

	bltb (pch, &vstPrompt[ich], cch);

	Assert (ich != 0 && xp != 0 && cch != 0);

	if (vhwndPrompt != NULL &&
			FSetDcAttribs ((hdc = GetDC(vhwndPrompt)), dccPrompt))
		{	
		GetClientRect (vhwndPrompt, (LPRECT)&rc);		  /* sets y coords */
		rc.xpRight = (rc.xpLeft = xp) + cch * vdxpDigit;  /* sets x coords */
		
		if (hfontStatLine)
			hfontOld = SelectObject( hdc, hfontStatLine );
		if (vsci.fWin3Visuals)
			{
			rc.ypTop = 1;	/* don't step on white line */
	 		SetBkColor( hdc, vdbmgDevice != dbmgEGA3  ? rgbLtGray : rgbGray);
			}

		/* Right justify the update field */
		for( ; *pch == ' ' && --cch; pch++ )
			 xp += vdxpDigit;

		ExtTextOut (hdc, xp, ypPromptTop, ETO_OPAQUE, 
				(LPRECT)&rc,(LPSTR)pch, cch, NULL);
		if (hfontStatLine && hfontOld )
			SelectObject( hdc, hfontOld );

		UpdateWindow(vhwndPrompt);
		}
	if (hdc != NULL)
		ReleaseDC (vhwndPrompt, hdc);
}


#endif /* WIN23 */
/* D I S P L A Y  P R O M P T */
/*  Displays the current prompt (vstPrompt) in mode pdc.
*/
/* %%Function:DisplayPrompt %%Owner:PETERJ */
DisplayPrompt (pdc, hppr)
int pdc;
{
	BOOL fExisted = vhwndPrompt != NULL;

	Assert (!(pdc & pdcmRestore));
	Assert (pdc & (pdcmPermanent | pdcmRestOnInput | pdcmTimeout));

	Assert(hppr == hNil || hppr == vhpprPRPrompt);
	if (vhpprPRPrompt != hppr)
		PopToHppr(hppr);

	if ((pdc & pdcmSL && vhwndStatLine) && !(pdc & pdcmPmt && fExisted))
		/* use the status line prompt */
		{
		DisplayStatLinePrompt(pdc);
		return;
		}

	if (!(pdc & pdcmPmt) || (!fExisted && !(pdc & pdcmCreate)))
		/*  no place to display */
		{
		RestorePrompt ();
		return;
		}

	/* else: display on prompt, create if necessary */

	/* remove unwanted statline prompt */
	if (vcchStatLinePrompt)
		DisplayStatLinePrompt (pdcRestore);

	/* set up prompt removal info */
	SetPromptRestore (pdc);

	if (!fExisted)
		/* no window, create one */
		{
		struct PT ptUL, ptSize;
		struct RC rc;

		/* figure out where to put it and how big to make it */

		/* determine the location of the parent client area */
		GetClientRect (vhwndApp, (LPRECT) &rc);

		/* determine size of prompt */
		ptSize.yp = vsci.dypStatLine;
		ptSize.xp = rc.xpRight - rc.xpLeft + vsci.dxpBorder << 1;

		/* calculate the upper left corner of the new window */
		ptUL.xp = rc.xpLeft - vsci.dxpBorder;
		ptUL.yp = rc.ypBottom + vsci.dypBorder - ptSize.yp;

		if (vhwndPromptHidden == NULL)
			{
			vhwndPromptHidden = vhwndPrompt = CreateWindow ((LPSTR)szClsPrompt, 
					(LPSTR) NULL,
					WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_VISIBLE |
					WS_CLIPCHILDREN, ptUL.xp, ptUL.yp, ptSize.xp, ptSize.yp,
					vhwndApp, NULL, vhInstance, (LPSTR) NULL);
			}
		else
			{
			SetWindowPos((vhwndPrompt = vhwndPromptHidden), NULL, 
					ptUL.xp, ptUL.yp, ptSize.xp, ptSize.yp,
					SWP_NOACTIVATE);
			SetWindowPos(vhwndPrompt, NULL, 0, 0, 0, 0,
					SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOACTIVATE);
			}
		if (vhwndPrompt == NULL)
			return;

		vpisPrompt = pisNormal;
		}

	else
		/*  prompt window already exists, update it */
		{
		Assert (vhwndPrompt != NULL);
		Assert (vpisPrompt != pisInput && vpisPrompt != pisModal);
		InvalPrompt();
		}

	BringWindowToTop (vhwndPrompt);

	StartUMeas(umPromptUpdateWin);
	/* cause window to be painted */
	UpdateWindow (vhwndPrompt);
	StopUMeas(umPromptUpdateWin);
}


/* I N V A L  P R O M P T */
/*  Invalidates the text of the prompt.
*/
InvalPrompt()
{
	struct RC rc;

	GetClientRect (vhwndPrompt, (LPRECT)&rc);
	InvalidateRect (vhwndPrompt, (LPRECT) &rc, fTrue);
}



/* P R O M P T  W N D  P R O C */
/*  This is the WndProc for the prompt window.  It receives and handles all
	messages bound for the prompt window.
*/
/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:PromptWndProc %%Owner:PETERJ */
EXPORT LONG FAR PASCAL PromptWndProc (hwnd, message, wParam, lParam)
register HWND hwnd;
register unsigned message;
WORD wParam;
LONG lParam;
{

#ifdef SHOWWNDPROC
	ShowMsg ("PP", hwnd, message, wParam, lParam);
#endif /* SHOWWNDPROC */

	switch (message)
		{
		/*  this keeps the ALT key from going into MENU mode */
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) != SC_KEYMENU)
			return DefWindowProc(hwnd, message, wParam, lParam);
		break;

	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		break;

	case WM_PAINT:
			{
			PAINTSTRUCT ps;
			struct RC rc;
#ifdef WIN23
			HFONT hfontOld;
			BOOL fProgRep;
			int ich, cch;
			short xp;
#endif /* WIN23 */
			BeginPaint (hwnd, (LPPAINTSTRUCT) &ps);

#ifdef WIN23
			if (vsci.fWin3)
				{
				if (FSetDcAttribs (ps.hdc, dccPrompt))
					{
					if (hfontStatLine)
						hfontOld = SelectObject( ps.hdc, hfontStatLine );
					GetClientRect (hwnd, (LPRECT) &rc);

					if (vsci.fWin3Visuals)
						{
						SetBkColor( ps.hdc, rgbLtGray );
						/* Put a white line across the top */
						PatBlt( ps.hdc, 0, 0, rc.xpRight, 1, WHITENESS );	
						rc.ypTop = 1;		/* don't clobber the white line */
						}
					xp = xpPromptLeft;
					ich = 1;
				
					/* If there is a progress report, just do "before" */
					if (fProgRep = (vhpprPRPrompt && (*vhpprPRPrompt)->ich))
						cch = (*vhpprPRPrompt)->ich - 1;
					else
						cch = vstPrompt[0];
	
					ExtTextOut( ps.hdc, xp, ypPromptTop, ETO_OPAQUE,
								(LPRECT)&rc, (LPSTR)&vstPrompt[ich], cch, NULL );

					if (fProgRep) {
						/* 
					 	* display progress field -- don't worry about erasing
					 	* to the end of the line: there's nothing there.
					 	* Start erase rect just past previous field.
					 	* For each space in the progress field, increment xp
					 	* by the size of a digit.  Only print the numbers.
					 	*/
						ich = cch + 1;			/* cch is the index from above */
						rc.xpLeft = xp = (*vhpprPRPrompt)->xp;

						for( cch = (*vhpprPRPrompt)->cch;
						 	vstPrompt[ich] == ' ' && --cch;
						 	ich++ )
							 	xp += vdxpDigit;
					
						/* this line should QUOTE to the line above */
						ExtTextOut( ps.hdc, xp, ypPromptTop, ETO_OPAQUE,
								(LPRECT)&rc, (LPSTR)&vstPrompt[ich], cch, NULL );
					
						/* display "after progress" */
						rc.xpLeft = (xp += cch * vdxpDigit);
						ich += cch;
						cch = vstPrompt[0] - ich;		/* display what's left */
						Assert(cch >= 0);
						/* this line should QUOTE to the line above */
						ExtTextOut( ps.hdc, xp, ypPromptTop, ETO_OPAQUE,
								(LPRECT)&rc, (LPSTR)&vstPrompt[ich], cch, NULL );

						}

					if (hfontStatLine && hfontOld)
						SelectObject( ps.hdc, hfontOld );
					if (vpisPrompt == pisInput)
						BlinkPromptCaret (ps.hdc, fFalse /* fToggle */);
					}
				else  if (vpisPrompt == pisInput)
					SetErrorMat(matMem);
				}
			else
				{
				if (FSetDcAttribs (ps.hdc, dccPrompt))
					{
					GetClientRect (hwnd, (LPRECT) &rc);
					ExtTextOut (ps.hdc, vsci.dxpBorder, vsci.dypBorder, 
							ETO_OPAQUE, (LPRECT)&rc, (LPSTR)&vstPrompt[1], 
							vstPrompt[0], NULL);
					if (vpisPrompt == pisInput)
						BlinkPromptCaret (ps.hdc, fFalse /* fToggle */);
					}
				else  if (vpisPrompt == pisInput)
					SetErrorMat(matMem);
				}
#else
			if (FSetDcAttribs (ps.hdc, dccPrompt))
				{
				GetClientRect (hwnd, (LPRECT) &rc);
				ExtTextOut (ps.hdc, vsci.dxpBorder, vsci.dypBorder, 
						ETO_OPAQUE, (LPRECT)&rc, (LPSTR)&vstPrompt[1], 
						vstPrompt[0], NULL);
				if (vpisPrompt == pisInput)
					BlinkPromptCaret (ps.hdc, fFalse /* fToggle */);
				}
			else  if (vpisPrompt == pisInput)
				SetErrorMat(matMem);
#endif /* WIN23 */
			EndPaint (hwnd, (LPPAINTSTRUCT) &ps);
			break;
			}

	case WM_DESTROY:
		if (vhpprPRPrompt != hNil)
			PopToHppr(hNil);
		Assert (vpisPrompt == pisNormal);
		Assert(vidf.fDead);
		vhwndPrompt = NULL;
		vhwndPromptHidden = NULL;
		break;

	case WM_TIMER:
		if (vpisPrompt == pisInput)
			BlinkPromptCaret (NULL, fTrue /* fToggle */);
		break;

	case WM_KEYDOWN:
		SetOurKeyState();
		if (wParam == VK_F1 && !vfControlKey)
			{
			if (vfShiftKey)
				CmdHelpContext(NULL);
			else
				CmdHelp(NULL);
			}
		else  if (!FExecKc (KcModified (wParam)) && vpisPrompt != pisInput)
			Beep ();
		break;

	case WM_CHAR:
		if (vpisPrompt == pisInput)
			{
			Assert(vppirPrompt != NULL);

			/* CASE:  BACKSPACE */
			if (wParam == VK_BACK)
				if (vstPrompt[0] >= vppirPrompt->ichFirst)
					{
					vstPrompt [0]--;
					goto LUpdate;
					}
				else
					{  /* backspace too far */
					Beep ();
					vfAwfulNoise = fFalse;
					}

			/* CASE:  CHARACTER */
			else  if (wParam >= ' ')
				if (vstPrompt[0] < vppirPrompt->ichMax - 1)
					{
					vstPrompt [++vstPrompt [0]] = wParam;
LUpdate:
					InvalPrompt();
					UpdateWindow (vhwndPrompt);
					}
				else
					{  /* too many characters */
					Beep ();
					vfAwfulNoise = fFalse;
					}
			}
		break;

	case WM_SETFOCUS:
		if (vpisPrompt == pisInput)
			SetTimer (hwnd, hwnd, GetCaretBlinkTime (), (LPSTR) NULL);
		/* FALL THROUGH */

	case WM_SIZE:
		BringWindowToTop (hwnd);
		break;

	case WM_KILLFOCUS:
		if (vpisPrompt == pisInput)
			KillTimer (vhwndPrompt, vhwndPrompt);
		break;

#ifdef DEBUG
	default:	    /* handled by NatPromptWndProc */
		Assert(fFalse);
#endif /* DEBUG */
		} /* switch (message) */

	return (LONG) fTrue;
}




/* B L I N K  P R O M P T  C A R E T */
/*  Toggles the state of the prompt caret, if fToggle.  Otherwise
	draws the caret if it is on.
*/
/* %%Function:BlinkPromptCaret %%Owner:PETERJ */
BlinkPromptCaret (hdc, fToggle)
HDC hdc;
BOOL fToggle;
{
	BOOL fLocalHdc = hdc == NULL;
	int xp;
	HFONT hfontOld;

	/*  if no caret or caret is off and not toggling it, done */
	if (vppirPrompt == NULL || (!vppirPrompt->fOn && !fToggle))
		return;

	if (fLocalHdc)
		if (!FSetDcAttribs ((hdc = GetDC (vhwndPrompt)), dccPrompt))
			{
			SetErrorMat(matMem);
			goto LDone;
			}

#ifdef WIN23
	if (vsci.fWin3)
		{
		if (!hdc) 
			hdc = GetDC( vhwndPrompt );
		if (hdc)
			{
			/* Determine the size of the string so far. */
			if (hfontStatLine)
				hfontOld = SelectObject( hdc, hfontStatLine );
			xp = GetTextExtent( hdc, (LPSTR) &vstPrompt[1], (short) vstPrompt[0] ) +
					xpPromptLeft;
			if (hfontStatLine && hfontOld)
				SelectObject( hdc, hfontOld);
			}
		}
	else
#endif /* WIN23 */
		xp = vstPrompt[0] * vsci.dxpTmWidth + vsci.dxpBorder;

	PatBlt (hdc, xp, vsci.dypBorder, vsci.dxpBorder, vsci.dypTmHeight,
			DSTINVERT);

	if (fToggle)
		vppirPrompt->fOn ^= 1;

LDone:
	if (fLocalHdc && hdc != NULL)
		ReleaseDC (vhwndPrompt, hdc);
}







/* A P P L I C A T I O N   M O D A L   F U N C T I O N S */


/*  SOMEWHAT HISTORICAL:
	This function contains the equilivant of a WinMain loop for the WInputPrompt
	window when that window takes the focus.  It causes messages to be
	processed until fEnd becomes true. GetMessage will return false if it has
	received a WM_QUIT message.  This will cause WInputPrompt to return fFalse,
	which is supposed to cause the caller to do nothing.  When mmw.c's WinMain
	finally gets around to calling GetMessage, it too will return false and our
	application will quit.  (We have been assured that once GetMessage returns
	false, all subsequent calls will cause it to return false).
*/




/*  WAppModalHwnd
	this function makes hwnd appear application modal.  It will trap all
	future inputs to other windows so as to restrict the user from changing
	the focus to another window.  the calling window had better be prepared 
	to get paint messages.

	mmo specifies Modal Mode Options:
	mmoFalseOnMouse - WAppModalHwnd returns false on any mouse actions
			outside of hwnd,
	mmoTrueOnMouse - like above but returns true
	mmoBeepOnMouse - Beep on any mouse actions in hwnd,
	mmoUpdateWws - will update windows if there's nothing else going on
*/

/* %%Function:WAppModalHwnd %%Owner:PETERJ */
WAppModalHwnd (hwnd, mmo)
HWND hwnd;
int mmo;

{
	int wReturn = fFalse;
	uns cIdle = 0;

	SetModalFocusHwnd (hwnd);

	/*  returns true except on AMM_TERMINATE message. */
	while (FProcessMessageQueue (hwnd, &wReturn, fTrue, mmo) && !vmerr.fMemFail)
		{
		if (mmo & mmoUpdateWws)
			UpdateAllWws(fTrue/*fAbortOK*/);
		if ((mmo & mmoNewestCmg) && cIdle++ == 1)
			if (FAbortNewestCmg((mmo & mmoNewestCmg), fTrue, fTrue))
				cIdle--;
		}

	RestoreInputFocus ();
	return wReturn;

}


#ifdef COMMENT
/*   If you call WAppModalHwnd, the window becoming modal must have the
		following in its WndProc.
*/
/*  this keeps the ALT key from going into MENU mode */
case WM_SYSCOMMAND:
if ((wParam & 0xfff0) != SC_KEYMENU)
return DefWindowProc(hwnd, message, wParam, lParam);
break;
#endif /* COMMENT */




/*  SetModalFocusHwnd
	Sets an application modal focus to window hwnd.

	vhwndAppModalFocus is refered to in wproc.c in the WndProc's for
	the pane and doc windows and for the app.  These points assure that
	the focus never wanders away and that it comes back here when returning
	from another app.
*/

/* %%Function:SetModalFocusHwnd %%Owner:PETERJ */
SetModalFocusHwnd (hwnd)
HWND hwnd;
{
	vhwndAppModalFocus = hwnd;
	SetFocus(hwnd);
}



/*  RestoreInputFocus
	Restore the input focus to the current window and disable the modal
	focus window.    
*/

/* %%Function:RestoreInputFocus %%Owner:PETERJ */
RestoreInputFocus ()

{
	Assert (vhwndAppModalFocus != NULL);
	vhwndAppModalFocus = NULL;
	SetOurKeyState ();
	NewCurWw(wwCur, fFalse /*fDoNotSelect*/);
}


/*  FProcessMessageQueue
	Processes all messages on the message queue.  Returns False on
	a terminate condition, else returns true.
	If fAppModal then terminates on an AMM_TERMINATE.  In any case
	terminates on a WM_QUIT (but won't return!).
*/

/* %%Function:FProcessMessageQueue %%Owner:PETERJ */
FProcessMessageQueue (hwnd, pw, fAppModal, mmo)
HWND hwnd;
int *pw;
BOOL fAppModal;
int mmo;

{

	/* trap messages for windows other than the document window
		so status window, etc will get cleared */
	while (PeekMessage( (LPMSG)&vmsgLast, NULL, NULL, NULL, PM_REMOVE ) &&
			!vmerr.fMemFail)
		{
		/*  check for quit.  Should do the same thing as WinMain. */
		if (vmsgLast.message == WM_QUIT)
			{
			QuitExit ();
			Assert (fFalse);
			}

		/*  check for app modal terminate */
		/* Warning: I'm not looking at the handle.  I'm assuming the only
			AMM_TERMINATE message will be the one we want.  That assumes that
			AMM_TERMINATE is unique */
		if (fAppModal && vmsgLast.message == AMM_TERMINATE)
			{
			*pw = vmsgLast.wParam;
			return fFalse;
			}

		if (FAppModalFilter (hwnd, &vmsgLast, mmo))
			/* this is a message that needs to be passed on */
			{
#ifdef SHOWDISPATCH
			ShowMsg ("mp", vmsgLast.hwnd, vmsgLast.message, vmsgLast.wParam,
					vmsgLast.lParam);
#endif
			/* ensure keyboard state is up to date */
			/* FUTURE SetOurKeyState in FCheck is valid even for win3 since
			   msg has been removed. If we remove the call in FCheckToggle,
			   add it here bz
			*/
			FCheckToggleKeyMessage ((LPMSG) &vmsgLast);
			TranslateMessage ((LPMSG) &vmsgLast);
			DispatchMessage ((LPMSG) &vmsgLast);
			}

		/* else:  drop the message */

#ifdef SHOWDROP
		else
			ShowMsg ("md", vmsgLast.hwnd, vmsgLast.message, vmsgLast.wParam,
					vmsgLast.lParam);
#endif
		}

	return fTrue;
}





/*  FAppModalFilter
	this is the general message filter for application-modal mode.  It will
	return fTrue if a message should be translated and dispatched.  it will
	return fFalse if a message should be dropped.

	In general most messages for the window with the focus, and all non-input
	messages will be allowed to pass.  If the application has been deactivated
	then all messages will be processed.  An alt-tab will also be let through.

	Some input messages that we don't allow will cause a Beep().
*/

/* %%Function:FAppModalFilter %%Owner:PETERJ */
FAppModalFilter (hwnd, pmsg, mmo)
HWND hwnd;
MSG * pmsg;
int mmo;

{
	if (vfDeactByOtherApp)
		return fTrue;


	/* check for a special case message */

	if (pmsg->hwnd == hwnd)
		switch (pmsg->message)
			{
#ifdef INTL
		/* REVIEW FUTURE tonykr: Special fix to allow number pad
			keys while in modal window.  This fix will be put in INTL
			versions first, then added to US for version 2.0.
			See bug # 4360 in Opus10.bug.
		*/
		case WM_SYSKEYDOWN:

			if (pmsg->wParam == VK_TAB || pmsg->wParam == VK_MENU ||
					pmsg->wParam == VK_ESCAPE || pmsg->wParam == VK_F10 ||
					(pmsg->wParam >= VK_NUMPAD0 && pmsg->wParam <= VK_NUMPAD9))
				/* switch focus to other app */
				/* or register the alt key */
				return fTrue;

			Beep ();

			return fFalse;

		case WM_SYSKEYUP:
			if (pmsg->lParam & 0x20000000L)	/* Was Alt key down? */
				return fTrue;
			/* FALL THROUGH */

		case WM_SYSCHAR:
		case WM_SYSDEADCHAR:
			/* we ignore these */

			return fFalse;

#else	/* !INTL */
		case WM_SYSKEYDOWN:

			if (pmsg->wParam == VK_TAB || pmsg->wParam == VK_MENU ||
					pmsg->wParam == VK_ESCAPE || pmsg->wParam == VK_F10)
				/* switch focus to other app */
				/* or register the alt key */
				return fTrue;

			Beep ();

			/* FALL THROUGH */


		case WM_SYSKEYUP:
		case WM_SYSCHAR:
		case WM_SYSDEADCHAR:

			/* we ignore these */

			return fFalse;
#endif /* !INTL */

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:

			/*  Exit on mouse action if requested, otherwise beep */
			if (mmo & mmoFalseOnMouse)
				{
				PostMessage (hwnd, AMM_TERMINATE, fFalse, 0L);
				return fFalse;
				}
			else  if (mmo & mmoTrueOnMouse)
				{
				PostMessage (hwnd, AMM_TERMINATE, fTrue, 0L);
				return fFalse;
				}
			else  if (mmo & mmoBeepOnMouse)
				{
				Beep ();
				return fFalse;
				}
			else
				break;
			}
	else
		switch (pmsg->message)
			{

		case WM_SYSKEYDOWN:

			if (pmsg->wParam == VK_TAB || pmsg->wParam == VK_MENU ||
					pmsg->wParam == VK_ESCAPE || pmsg->wParam == VK_F10)
				/* switch focus to other app */
				/* or register the alt key */
				return fTrue;

			/* else:  FALL THROUGH */


		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:

			/* these messages we ignore but we also beep so
					that the user knows that we received the message
					and that it is not allowed.
					EXCEPT with mmoFalseOnMouse in which case we cause an
					eventual terminate with False.
				*/
			if (mmo & mmoFalseOnMouse)
				PostMessage (hwnd, AMM_TERMINATE, fFalse, 0L);
			else  if (mmo & mmoTrueOnMouse)
				PostMessage (hwnd, AMM_TERMINATE, fTrue, 0L);
			else
				Beep ();

			/* FALL THROUGH */


		case WM_SYSKEYUP:
		case WM_SYSCHAR:
		case WM_SYSDEADCHAR:

			/* we ignore these */

			return fFalse;

			}




	/* check to see if the message is an input message: if so, it is allowed
		to pass iff it is going to the modal window */

	if (

			/* is a mouse message */
	(pmsg->message <= WM_MOUSELAST && pmsg->message
			>= WM_LBUTTONDOWN)

			/* is a mouse message for menus etc. */
	|| (pmsg->message <= WM_NCMBUTTONDBLCLK && pmsg->message
			>= WM_NCLBUTTONDOWN)

			/* is keyboard message */
	|| (pmsg->message <= WM_KEYLAST && pmsg->message >= WM_KEYFIRST)

			)

		return (pmsg->hwnd == hwnd);



	/* what are left are non-input messages--let them go */

	return fTrue;

}


/* G E T M S T S T N O A B O R T */
/* It clips the tail of the message string to remove "Press Esc to cancel." */
/* from the message string when the user disables input with a macro.	    */

/* %%Function:GetMstStNoAbort %%Owner:PETERJ */
GetMstStNoAbort(pmst)
MST  * pmst;
{
	switch (pmst->cxt)
		{
	case 0x7008 /* mstSearching */        :
	case 0x700A /* mstSpellCheck */       :
	case 0x700D /* mstBuildingIndex */    :
	case 0x700E /* mstRefreshIndex */     :
	case 0x700F /* mstCalcFields */       :
	case 0x7011 /* mstReplacing */        :
	case 0x7012 /* mstHyphenate */        :
	case 0x7013 /* mstPrintingPage */     :
	case 0x7019 /* mstMergingRecord */    :
	case 0x7023 /* mstRepaginating */     :
	case 0x7024 /* mstScanningIndex */    :
	case 0x7025 /* mstPrintingSub	*/      :
	case 0x7026 /* mstRenumber */	        :
	case 0x7027 /* mstBuildingToc */      :
	case 0x7028 /* mstConverting */       :
	case 0x703A /* mstPrinting */	        :
	case 0x703D /* mstSort */	        :

		pmst->st[0] -= 20; /* clip the tail */
	default :
		return;
		}
}



/*----------------------------------------------------------------------------
|   OurYield
|
|    fNotDde means do not pass dde messages
----------------------------------------------------------------------------*/
/* %%Function:OurYield %%Owner:PETERJ */
OurYield(fNotDde)
BOOL fNotDde;
{
	MSG msg;
	extern BOOL vfFocus;
	extern BOOL vfDeactByOtherApp;
	extern long tickNextYield;
	long tick = GetTickCount();

	if (!vfDeactByOtherApp && tick <= tickNextYield)
		return;

	tickNextYield = tick + ((vfFocus && fElActive) ? 50 * dtickYield : dtickYield);

	if (!vfDeactByOtherApp)
		while (PeekMessage((MSG far *)&msg, NULL, 0, 0, PM_NOREMOVE))
			{
#ifdef SHOWYIELDWAITMSG
			ShowMsg ("aa", msg.hwnd, msg.message, msg.wParam,
					msg.lParam);
#endif /* SHOWYIELDWAITMSG */
			switch (msg.message)
				{
			default:
				return;

			case WM_PAINT:
				if (vpri.fInPrintDisplay || vfInFormatPage)
					return;
			case WM_NCPAINT:
			case WM_PAINTICON:
				UpdateWindow(vhwndApp);
				return;

			case WM_SYSKEYUP:
				if (msg.wParam == VK_MENU)
					goto LEatIt;
				goto Dispatch;

			case WM_KEYDOWN:
				if (msg.wParam == VK_TAB || msg.wParam == VK_ESCAPE)
					return;
				/* fall through to handle Alt and Shift keys */
			case WM_SYSKEYDOWN:
				switch (msg.wParam)
					{
				default:
					return;

				case VK_MENU:
				case VK_TAB:
				case VK_SHIFT:
				case VK_ESCAPE:

					/* If the following (F5-F10) cause ANY
					problems, nuke 'em   -- bac */
				case VK_F5:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
					if (msg.message != WM_SYSKEYDOWN)
						return;
					break;
					}
				/* fall through to dispatch Alt, Tab, Shift and Escape keys */
			case WM_KEYUP:
			case WM_MOUSEMOVE:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_NCMOUSEMOVE:
			case WM_NCLBUTTONUP:
			case WM_NCRBUTTONUP:
			case WM_NCMBUTTONUP:
			case WM_DDE_TERMINATE:
			case WM_SYSTEMERROR:
				goto Dispatch;

				/* Here we attempt to let the user do standard app 
				window things during long ops without causing the 
				appearance of a hang.  If this causes the slightest
				problem, remove it! -- bac */
#define USESYSSTUFF
#ifdef USESYSSTUFF
			case WM_NCLBUTTONDBLCLK:
				if (msg.wParam != HTSYSMENU)
					goto Dispatch;
				/* FALL THROUGH */

			case WM_NCLBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_MOUSEACTIVATE:
LEatIt:
/* We disable the window here because silly windows will send an activate
message directly to the window during removal of WM_LBUTTONDOWN messages. */
				EnableWindow(msg.hwnd, fFalse);
				PeekMessage((MSG far *) &msg, msg.hwnd, 
						msg.message, msg.message, PM_REMOVE);
				EnableWindow(msg.hwnd, fTrue);
				return;

			case WM_SYSCOMMAND:
				switch (msg.wParam)
					{
				default:
					goto LEatIt;

				case SC_SIZE:
				case SC_MOVE:
				case SC_MINIMIZE:
				case SC_MAXIMIZE:
				case SC_NEXTWINDOW:
				case SC_PREVWINDOW:
				case SC_RESTORE:
					break;
					}
				/* FALL THROUGH */
#endif /* USESYSSTUFF */

Dispatch:
				if (PeekMessage((MSG far *)&msg, msg.hwnd,
						msg.message, msg.message, PM_REMOVE))
					{
					TranslateMessage((MSG far *)&msg);
					DispatchMessage((MSG far *)&msg);
					}
				break;

			case WM_DDE_ADVISE:
			case WM_DDE_UNADVISE:
			case WM_DDE_ACK:
			case WM_DDE_DATA:
			case WM_DDE_REQUEST:
			case WM_DDE_POKE:
			case WM_DDE_EXECUTE:
				if (!fNotDde)
					goto Dispatch;
				return;
				}
			}
	else
		while (vfDeactByOtherApp && 
				PeekMessage((MSG far *)&msg, NULL, 0, 0, PM_NOREMOVE))
			{
			GetMessage((MSG far *)&msg, NULL, 0, 0);
			/* Activated by clicking on our non-client area; don't allow
				menus to drop down. */ /* Don't accept command messages
				unless we're active either... */
			if ((msg.message != WM_COMMAND && msg.message != WM_NCLBUTTONDOWN)
					|| msg.hwnd != vhwndApp)
				{
				if (msg.message == WM_PAINT &&
						(vpri.fInPrintDisplay || vfInFormatPage))
					return;
#ifdef SHOWYIELDWAITMSG
				ShowMsg ("na", msg.hwnd, msg.message, msg.wParam,
						msg.lParam);
#endif /* SHOWYIELDWAITMSG */
				TranslateMessage((MSG far *)&msg);
				DispatchMessage((MSG far *)&msg);
				}
			}
}



#ifdef PCJ
#define SHOWCOMPRESS
#endif /* PCJ */
#ifdef BZ
#define SHOWCOMPRESS
#endif /* BZ */


int vfnNonDel;   /* fn not to delete during saves */

#ifdef DEBUG
int cCompressions = 0;
int nCompressionStop = 0;
#endif /* DEBUG */

/* C H E C K  C O M P R E S S  D O C */
/* If piece table compression is advised for doc, do a full save of doc to
   a temp file.  patch things up so that the resulting doc still points to 
   the previous fn (keeps the old name). */
/* %%Function: CheckCompressDoc  %%Owner: peterj */
CheckCompressDoc(doc)
int doc;
{
	int ipcdLimit = smtMemory == smtProtect ? 4608/*54K*/ : 1280/*15K*/;
	int ipcdMac;
	int fnPreloadSave;
	struct PLC **hplcpcd;
	struct DOD *pdod;
	int udt, iInstance;
	CHAR stTemp[ichMaxFile];

	extern vfnPreload;
	extern vcPreload;

#ifdef DEBUG
	if (vdbs.fCompressOften)
		ipcdLimit = 100;
#endif /* DEBUG */

	stTemp[0] = 0;

	/* if the piece table is getting big, do a full save */
	if ((ipcdMac = IMacPlc(hplcpcd = PdodDoc(doc)->hplcpcd)) < ipcdLimit
			|| vmerr.fCompressFailed || !FCreateStFile(stTemp))
		return; /* don't need to save */

#ifdef SHOWCOMPRESS
	CommSzNum(SzShared("Full saving doc to reduce piece table with iMac: "), 
			ipcdMac);
#endif

	Scribble(ispFieldCalc1, 'S');

	pdod = PdodDoc(doc);
	Assert(!pdod->fShort);
	vfnNonDel = pdod->fn;
	iInstance = pdod->iInstance;
	udt = pdod->udt;

	fnPreloadSave = vfnPreload;

		/* figure out which fn to preload */
		{
		int fnPreload = fnNil;
		int ipcd;
		struct PCD pcd;

		/* plan: take a sampling of the fns in doc.  if any are not
			fnScratch and not pdod->fn, use it (assumed to be result
			of previous save), else if any of them are pdod->fn use
			it, else use fnScratch if any pieces are from it, finally
			leave it as it was if none of the above.
		*/

		for (ipcd = 0; ipcd < ipcdMac; ipcd+=8)
			{
			GetPlc(hplcpcd, ipcd, &pcd);
			if (pcd.fn > fnScratch)
				{
				fnPreload = pcd.fn;
				if (pcd.fn != vfnNonDel)
					break;
				}
			else  if (fnPreload == fnNil && pcd.fn == fnScratch)
				fnPreload = fnScratch;
			}
		if (fnPreload != fnNil)
			vfnPreload = fnPreload; /* overrides normal EnablePreload */
		vcPreload++; /* so we don't get overrided */

#ifdef SHOWCOMPRESS
		CommSzNum(SzShared("\tpreloading: "),vfnPreload);
		CommSzNum(SzShared("\tpdod->fn: "),vfnNonDel);
		CommSzNum(SzShared("\tprevious preload: "),fnPreloadSave);
#endif /* SHOWCOMPRESS */
		}

#ifdef DEBUG
	if (++cCompressions == nCompressionStop)
		DebugBreak(2);
#endif /* DEBUG */

	if (FFlushDoc(doc, stTemp, dffSaveNative, fFalse /*fReport*/))
		{
		struct FCB *pfcb = PfcbFn(PdodDoc(doc)->fn);
		/* so save temp files will be deleted */
		pfcb->fTemp = fTrue;
		pfcb->fDoc = fFalse;
		Assert (IMacPlc(PdodDoc(doc)->hplcpcd) < ipcdLimit);
		}
	else
		vmerr.fCompressFailed = fTrue;

	/* restore normal EnablePreload */
	vfnPreload = fnPreloadSave;
	vcPreload--;
	Assert(vcPreload >= 0);

	/* restore doc's name (user does not know it was saved) */
	pdod = PdodDoc(doc);
	pdod->fn = vfnNonDel;
	pdod->iInstance = iInstance;
	pdod->udt = udt;
	vfnNonDel = fnNil;

	Scribble(ispFieldCalc1, ' ');
}

#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Prompt_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Prompt_Last() */
