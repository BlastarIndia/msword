/* R U L R I B . C */
/*  Non-core commands invoked through ruler or ribbon. */


#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "doc.h"
#include "props.h"
#include "style.h"
#include "format.h"
#include "screen.h"
#include "sel.h"
#include "cmdtbl.h"
#include "cmd.h"
#include "prompt.h"
#define RULRIBC
#include "keys.h"
#include "wininfo.h"
#include "iconbar.h"
#include "ibdefs.h"
#define REPEAT
#define RULER
#include "ruler.h"
#include "prm.h"
#include "debug.h"
#include "error.h"
#include "help.h"
#include "border.h"
#include "ch.h"
#include "cmdlook.h"

/* WARNING!!! This is stolen from sdm.h */
#define	uNinch		(0xffff)	// Unsigned.
#define	wNinch		(-32767)	// Ints. 

extern struct CA caTap;
extern HWND   vhwndApp;
extern HWND   vhwndRibbon;
extern struct MERR vmerr;
extern BOOL   vfRecording;
extern HANDLE vhInstance;
extern CHAR   szEmpty[];
extern char   (**vhmpiststcp)[];
extern int    vdocStsh;
extern int    wwCur;
extern int    wwMac;
extern struct MWD ** mpmwhmwd[];
extern struct CA     caPara;
extern struct UAB    vuab;
extern struct WWD    ** mpwwhwwd[];
extern int           wwCreating;
extern RRF	     rrf;
extern struct WWD  **hwwdCur;
extern struct MWD  **hmwdCur;
extern KMP         **hkmpCur;
extern struct SEL    selCur;
extern struct SCI    vsci;
extern struct RULSS  vrulss;
extern struct PREF   vpref;
extern struct SEP    vsepFetch;
extern struct FTI    vfti;
extern struct FLI    vfli;
extern BOOL         vfInitializingRuler;
extern CHAR         szEmpty[];
extern HCURSOR      vhcArrow;
extern HWND         vhwndAppModalFocus;
extern struct CA    caSect;
extern HWND         vhwndStatLine;
extern HWND         vhwndCBT;
extern struct TCC   vtcc;
extern struct TCXS  vtcxs;
extern int	vssc;
#ifdef WIN23
extern HBRUSH vhbrLtGray;
#endif /* WIN23 */
/* R U L E R  F U N C T I O N S */

extern struct REB * PrebInitPrebHrsd ();
extern int ParseStyle();
InvalStyle();

extern int     ypScaleBottom;
extern int     ypMarkBottom;
extern int     dypMark;
extern int     dypMajorTick;
extern int     dypMinorTick;
extern int     dypTinyTick;
extern struct CHP  chpRibbon;   /* these hold current state of ribbon */
extern struct CHP  chpRibbonGray;


extern struct REB * vpreb;


/* Map of drag marks to the resulting sprm */
csconst CHAR mpimksprm [] =
{
	sprmPDxaLeft1,   /* imkIndLeft1 */
	sprmPDxaLeft,    /* imkIndLeft */
	sprmPDxaRight,   /* imkIndRight */
	sprmNoop,        /* imkDefTab */
	sprmTDxaLeft,    /* imkTLeft */
	sprmNoop,        /* imkTableCol */
	sprmSDxaColumns, /* imkLeftMargin */
	sprmSDxaColumns  /* imkRightMargin */
};



/*  1/2 width of cursor in RulerEditMode, distance considered "close"  */
csconst int mputdxaCursorDiv2 [] =
{ 
	czaInch/16, czaPoint*5, 142/*czaCm/8*/, czaPicas/2 };


#define ddxaCmErrorCurDiv2 567

/*  movement distance on ctrl-arrow in RulerEditMode */
csconst int mputdxaLarge [] =
{ 
	czaInch, czaPoint*50, czaCm*2, czaInch };


/* %%Function:CacheParaRUL  %%Owner:peterj */
CacheParaRUL(doc,cp)
/* hack so we can pass the address of a hand native coded proc */
int doc;
CP cp;
{
	CachePara(doc,cp);
}


/* I B  P R O C  R U L E R  C M D */
/* %%Function:IbProcRulerCmd  %%Owner:peterj */
IbProcRulerCmd(hwndRuler, hrsd, ibm, idRul, wParam, lParam)
HWND hwndRuler;
struct RSD **hrsd;
WORD ibm;
WORD idRul;
WORD wParam;
LONG lParam;
{

	switch (ibm)
		{
	case ibmDblClk:
		DoRulerDblClick(MAKEPOINT(lParam), fFalse);
		break;

	case ibmInit:
#ifdef DBGIB
		CommSz(SzShared("Ruler inited.\r\n"));
#endif /* DBGIB */
			/* terminate Dyadic Move Mode etc if on. */
		SpecialSelModeEnd();
		SetVdocStsh(selCur.doc);
		if ((vhmpiststcp = HAllocateCw(cwMpiststcp)) == hNil)
			TerminateIbDlgMode(HibsFromHwndIb(hwndRuler), fFalse);
		else
			{
			GenLbStyleMapping();
			InvalidateIibb(hwndRuler, idRulStyle);
			}
		break;

	case ibmTerm:
#ifdef DBGIB
		CommSz(SzShared("Normal "));
#endif /* DBGIB */
#ifdef RSH
		LogUa(uaApplyStyleIb);
#endif /* RSH */

			/* parse the stuff from the combos and apply it! */
			{
			CHAR szStyle[cchMaxStyle+1];
			Assert (vssc == sscNil);
			GetTextIibbHwndIb(hwndRuler, idRulStyle, szStyle, sizeof(szStyle));
			/* wParam == fTrue if message results from KILLFOCUS (use system
				modal messages) */
			ApplyRulerStyle(szStyle, wParam);
			}
		/* fall through */
	case ibmCancel:
#ifdef DBGIB
		CommSz(SzShared("ruler terminated.\r\n"));
#endif /* DBGIB */
		Assert (vssc == sscNil);
		InvalStyle ();
		FreePh(&vhmpiststcp);
		break;
		}

}


/* D O  R U L E R  D B L  C L I C K */
/* %%Function:DoRulerDblClick  %%Owner:peterj */
DoRulerDblClick(pt, fMark)
struct PT pt;
BOOL fMark;
{
	int rk = (*HrsdFromHwndRuler((*hmwdCur)->hwndRuler))->rk;

	TermCurIBDlg(fFalse);
	CmdExecBcmKc((fMark && pt.yp >= ypMarkTop && pt.yp <= ypMarkBottom) ? 
			(rk == rkNormal ? bcmTabs : (rk == rkTable ? bcmFormatTable :
			bcmSection)) : 
	imiParagraph, kcNil);
}


/* C M D  R U L E R  M O D E */
/* %%Function:CmdRulerMode  %%Owner:peterj */
CMD CmdRulerMode(pcmb)
CMB *pcmb;
{
	int rk;
	struct RSD **hrsd;
	HWND hwndRuler;
	struct SEL *psel = PselActive();

	if (hmwdCur == hNil || (hwndRuler = (*hmwdCur)->hwndRuler) == NULL)
		{
		Beep();
		return cmdError;
		}

	hrsd = HrsdFromHwndRuler(hwndRuler);

	rk = ((*hrsd)->rk+1) % rkMax;
	Assert(rkMax-1 != rkTable);

	if (rk == rkTable && !(psel->fTable || psel->fWithinCell))
		rk++;

	UpdateRuler(hmwdCur, fFalse, rk, fTrue /*fAbortOK*/);

	return cmdOK;
}


/* KEYBOARD EDIT RULER MODE */

/* %%Function:REReturn  %%Owner:peterj */
void REReturn()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, fTrue, 0L);
}




/* %%Function:REEscape  %%Owner:peterj */
void REEscape()
{
	PostMessage (vhwndAppModalFocus, AMM_TERMINATE, fFalse, 0L);
}


/* %%Function:RERight  %%Owner:peterj */
void RERight()
{
	MoveRulerLeftRight(fTrue, vfControlKey);
}


/* %%Function:RELeft  %%Owner:peterj */
void RELeft()
{
	MoveRulerLeftRight(fFalse, vfControlKey);
}


/* %%Function:MoveRulerLeftRight  %%Owner:peterj */
MoveRulerLeftRight(fRight, fLarge)
BOOL fRight, fLarge;
{
	int dxaMove = fLarge ? mputdxaLarge [vpref.ut] : 
	(mputdxaCursorDiv2 [vpref.ut] * 2);
	int wSign = fRight ? 1 : -1;

	MoveCursorToXa(vpreb, vpreb->xaCursor + (dxaMove * wSign));
}


/* %%Function:REHome  %%Owner:peterj */
void REHome()
{
	MoveCursorToXa(vpreb, 0);
}


/* %%Function:REEnd  %%Owner:peterj */
void REEnd()
{
	int dxaColWidth = (*vpreb->hrsd)->dxaColumnNorm;
	if (dxaColWidth >= 0)
		MoveCursorToXa(vpreb, 
				dxaColWidth * (vpreb->xaCursor / dxaColWidth + 1));
}


/* %%Function:RETab  %%Owner:peterj */
void RETab()
{
	MoveCursorToXa(vpreb, 
			XaNextTab(vpreb->hrsd, vpreb->xaCursor, vfShiftKey ? -1 : 1));
}


/* %%Function:RETabDecimal  %%Owner:peterj */
void RETabDecimal()
{
	DoRulerCommand(vpreb->hrsd, idRulTabDecimal);
}


/* %%Function:RETabRight  %%Owner:peterj */
void RETabRight()
{
	DoRulerCommand(vpreb->hrsd, idRulTabRight);
}


/* %%Function:RETabCenter  %%Owner:peterj */
void RETabCenter()
{
	DoRulerCommand(vpreb->hrsd, idRulTabCenter);
}


/* %%Function:RETabLeft  %%Owner:peterj */
void RETabLeft()
{
	DoRulerCommand(vpreb->hrsd, idRulTabLeft);
}


/* %%Function:REDelete  %%Owner:peterj */
void REDelete()
{
	InsDelTab(fFalse);
}


/* %%Function:REInsert  %%Owner:peterj */
void REInsert()
{
	InsDelTab(fTrue);
}


/* %%Function:InsDelTab  %%Owner:peterj */
InsDelTab(fIns)
BOOL fIns;
{
	SetEditModeUndo(vpreb);
	RulerEditTab(vpreb->hrsd, fIns, vpreb->xaCursor);
	PaintRuler(vpreb->hrsd, rpcMarkArea);
}


/* %%Function:REIndRight  %%Owner:peterj */
void REIndRight()
{
	SetIndent(imkIndRight);
}


/* %%Function:REIndLeft  %%Owner:peterj */
void REIndLeft()
{
	SetIndent(imkIndLeft);
}


/* %%Function:REIndLeft1  %%Owner:peterj */
void REIndLeft1()
{
	SetIndent(imkIndLeft1);
}


/* %%Function:SetIndent  %%Owner:peterj */
SetIndent(imk)
int imk;
{
	SetEditModeUndo (vpreb);
	ChangeRulerImk (vpreb->hrsd, imk, 0, vpreb->xaCursor, 0);
	PaintRuler (vpreb->hrsd, rpcMarkArea);
}


/* %%Function:REGetHelp  %%Owner:peterj */
void REGetHelp()
{
	GetHelp(cxtRulerMode);
}


/* NOTE: this table must be sorted by kc's */
csconst KME rgKmeRulerEdit[] =
{
/* Defined in keys.h for localization */

rgKmeRulerDef
};


#define ckmeRulerEdit (sizeof(rgKmeRulerEdit)/sizeof(KME))



/*  CmdEditRuler
	this function places the user into keyboard edit ruler tabs mode.  This
	will bring up the ruler, place a "cursor" in the mark area and make the
	ruler application modal.  The user will then be able to edit tabs and
	change indents in the
	mark area.  if the user types <cr> his/her changes will be saved; <esc> will
	ignore his/her changes.
*/
/* %%Function:CmdEditRuler   %%Owner:peterj */
CMD CmdEditRuler (pcmb)
CMB *pcmb;
{
	HWND hwndRuler;
	KMP **hkmp, *pkmp;
	struct CA ca;
	struct RSD ** hrsd;
	struct REB *preb, reb;
	int wLongOp;
	extern int vfMouseExist;

	if (hmwdCur == hNil)
		/* no child to put a ruler on */
		return cmdError;

	/* this creates ruler if necessary */
	ShowRulerWwCur (fTrue /*fCreate*/, fTrue/* fCmd */);

	if ((hwndRuler = (*hmwdCur)->hwndRuler) == NULL)
		/* couldn't bring up ruler, nothing to do */
		return cmdError;
	else
		vpref.fRuler = fTrue;

	hrsd = HrsdFromHwndRuler(hwndRuler);

	if ((*hrsd)->rk != rkNormal)
		/* force normal mode */
		UpdateRuler(hmwdCur, fFalse, rkNormal, fFalse/*fAbortOK*/);

	/* get a reb for the ruler.  point vpreb to it */
	preb = PrebInitPrebHrsd (&reb, hrsd, fTrue/*fGlobal*/);

	/* put the cursor in its initial position */
	MoveCursorToXa (preb, XaMarkFromXpMark(0, hrsd) + 
			mputdxaCursorDiv2[vpref.ut]);

		/* BLOCK */
		{
		int kc;

		if ((hkmp = HkmpCreate(ckmeRulerEdit, kmfStopHere)) == hNil)
			return cmdNoMemory;

		pkmp = *hkmp;
		bltx((int FAR *)rgKmeRulerEdit, (int FAR *)pkmp->rgkme, 
				ckmeRulerEdit * cwKME);
		pkmp->ikmeMac = ckmeRulerEdit;

		kc = KcFirstOfBcm(hkmpCur, bcmEditRuler);
		while (kc != kcNil)
			{
			void REReturn();

			AddKeyPfn(hkmp, kc, REReturn);
			kc = KcNextOfBcm(bcmEditRuler);
			}

		(*hkmp)->hkmpNext = hkmpCur;
		hkmpCur = hkmp;
		}

	if (vhwndStatLine != NULL)
		DisplayHelpPrompt(fTrue);

	/* enter edit ruler mode ... (returns when we exit mode) */
	GetLongOpState(&wLongOp);
	EndLongOp(fTrue);
	SetCursor(vfMouseExist ? vhcArrow : NULL);
	if (!WAppModalHwnd (hwndRuler, mmoTrueOnMouse+mmoUpdateWws))
		/* Escape */
		if ( preb->fUndoSet )
			{  /* if anything was done, undo it */
			CallCmd(bcmUndo);
			/* there is not now any undo state */
			SetUndoNil();
			}

	if (vhwndStatLine != NULL)
		DisplayHelpPrompt(fFalse);

	/* remove keymap */
	RemoveKmp(hkmpCur);
	AssertH(hkmpCur);
	(*hwwdCur)->hkmpCur = hkmpCur;

	/* reset cursor */
	preb->xaCursor = xaNil;

	/* ensure the ruler is up to date (no cursor, undos done) */
	PaintRuler (hrsd, rpcMarkArea);

	/* frees resources obtained for preb & resets vpreb */
	FreePreb (preb);

	ResetLongOpState(wLongOp);
	return cmdOK;
}



/*  SetEditModeUndo
	sets up the edit mode undo information before any changes are actually
	made.
*/
/* %%Function:SetEditModeUndo   %%Owner:peterj */
SetEditModeUndo (preb)
struct REB * preb;
{
	struct CA ca;

	/* if we have not already added a grpprl to the ruler sprm, we must
		ensure any old grpprl has been flushed.  Note that the first
		AddRulerGrpprl call after a FlushRulerSprms call will set undo. */

	if (!preb->fUndoSet)
		{
		FlushRulerSprms ();
		SetUndoNil();
		preb->fUndoSet = fTrue;
		}
}



/*  RulerEditTab
	performs either a tab insertion or a tab deletion at xa.  Deletions delete
	over an entire range.
*/
/* %%Function:RulerEditTab   %%Owner:peterj */
RulerEditTab (hrsd, fInsert, xa)
struct RSD ** hrsd;
BOOL fInsert;
int xa;
{
	struct TBD tbd;
	int dxaIns = xaNil;
	int dxaDel = xaNil;
	int dxaClose = 0;

	if (fInsert && (*hrsd)->pap.itbdMac >= itbdMax)
		{  /* there are too many tabs to insert another */
		Beep ();
		return;
		}

	/* set up the new tab */
	tbd.tlc = tlcDefault;
	tbd.jc = (*hrsd)->jcTabCur;

	if (fInsert)
		dxaIns = xa;
	else
		{
		dxaDel = xa;
		/* delete over an area xa +- dxaCursor/2 */
		dxaClose = mputdxaCursorDiv2 [vpref.ut];
		}

	ChangeRulerTab (hrsd, dxaIns, dxaDel, tbd, dxaClose);
}



/*  MoveCursorToXa
	repositions the cursor from whereever it was before to xa.
*/
/* %%Function:MoveCursorToXa   %%Owner:peterj */
MoveCursorToXa (preb, xa)
struct REB * preb;
int xa;
{
	struct RSD ** hrsd = preb->hrsd;
	HDC hdc = NULL;
	int xp;
	int dxpWidth = preb->xpRight - preb->xpLeft;
	int xpScroll = -(*hrsd)->xpZero;
	int dxpScroll;
	int ddxpCursor = DxsFromDxa (0/*dummy*/, mputdxaCursorDiv2[vpref.ut]);
#ifdef REVIEW
#ifdef WIN23
	HBRUSH hbrOld;
#endif /* WIN23 */
#endif /* REVIEW */

	/* provide feedback if they have gone too far */
	if (xa < xaMin || xa > xaMax)
		Beep ();

	if (xa < 0 && xpScroll >= 0 && !vfShiftKey)
		{  /* special case:  if we are not already scrolled negative, you
				cannot scroll negative (except with shift key) */
		Beep ();
		xa = 0;
		}

	/* where are we going */
	xa = XaQuantize (xa, mputdxaCursorDiv2 [vpref.ut]*2, 
			vpref.ut==utCm?ddxaCmErrorCurDiv2:0);

	if (preb->xaCursor == xa)
		/* no need to move */
		return;

	/* get the dc's */
	if (! FSetDcAttribs (hdc = GetDC ((*hrsd)->hwndMark), dccRuler))
		if (hdc != NULL)
			{
			ReleaseDC ((*hrsd)->hwndMark, hdc);
			hdc = NULL;
			}

	/* delete the old cursor */
	if (hdc)
		{
#ifdef REVIEW
#ifdef WIN23
		if (vsci.fWin3Visuals )
			hbrOld = SelectObject(hdc, vhbrLtGray);
#endif /* WIN23 */
#endif /* REVIEW */
		DrawRulerCursor(hdc, preb, preb->xaCursor);
		preb->xaCursor = xaNil; /* none displayed now */
		}

	/* if necessary, scroll to force this point visible */
	xp = DxsFromDxa (0/*dummy*/, xa);

	/* the following code figures out how much to scroll in one direction or
		the other.  The criteria:  make both ends of the cursor visible except
		when we are on one end or the other; do not attempt to scroll beyond
		xaMin or xaMax;  if we can do so without scrolling beyond an end,
		scroll at least xpRulerScroll */

	if (xp - ddxpCursor <= xpScroll)
		{
		xp = (abs(xp) >= ddxpCursor) ? xp - ddxpCursor : 0;
		dxpScroll = max ((xpScroll >= xpRulerScroll) ? xpRulerScroll :
				0, xpScroll - xp);
		if (dxpScroll > 0)
			{
			ScrollRight (wwCur, dxpScroll);
			UpdateWw(wwCur, fTrue);
			}
		}
	else  if (xp - xpScroll + ddxpCursor >= dxpWidth)
		{
		xp = min (vsci.xpScrollMax + dxpWidth, xp + ddxpCursor);
		dxpScroll = max ( ((vsci.xpScrollMax < xpRulerScroll +
				xpScroll) ? 0 : xpRulerScroll), (xp - dxpWidth - xpScroll) );
		if (dxpScroll > 0)
			{
			ScrollLeft (wwCur, dxpScroll);
			UpdateWw(wwCur, fTrue);
			}
		}

	/* see if the new location is on the screen */
	if ((xp = XpMarkFromXaMark(xa, hrsd)) < preb->xpLeft ||
			xp > preb->xpRight)
		{
		Beep();
		xp = xp < preb->xpLeft ? preb->xpLeft : preb->xpRight;
		xa = XaMarkPxp(&xp, hrsd, xaMin, xaMax);
		}

	/* new location */
	preb->xaCursor = xa;

	/* display the new cursor */
	if (hdc != NULL)
		{
		DrawRulerCursor(hdc, preb, xa);
#ifdef REVIEW
#ifdef WIN23
		if (vsci.fWin3Visuals && hbrOld != NULL)
			SelectObject(hdc, hbrOld);
#endif /* WIN23 */
#endif /* REVIEW */
		ReleaseDC ((*hrsd)->hwndMark, hdc);
		}
	else
		PaintRuler (hrsd, rpcMarkArea);
}




/*  XaNextTab
	returns the next set tab in direction wDir (+xa's if pos).  If
	there are no set tabs in that direction, returns the next default tab in
	that direction.
*/
/* %%Function:XaNextTab   %%Owner:peterj */
XaNextTab (hrsd, xa, wDir)
struct RSD ** hrsd;
int xa, wDir;
{
	int itbd = -1;
	int itbdEnd;
	int dxaTab = (*hrsd)->dxaTab;
	int itbdMac = (*hrsd)->pap.itbdMac;
	int *pdxa;

	Assert (wDir == 1 || wDir == -1);

	/* start our search 1/2 quantum away from where we are */
	xa += (mputdxaCursorDiv2[vpref.ut]) * wDir;

	if (itbdMac > 0)
		/* there are some tabs */
		if (wDir > 0)
			{  /* search for tab to right */
			itbd = 0;
			itbdEnd = itbdMac -1;
			pdxa = &(*hrsd)->pap.rgdxaTab [itbd];
			while (itbd <= itbdEnd && *pdxa <= xa)
				{
				pdxa++;
				itbd++;
				}
			if (itbd > itbdEnd)
				itbd = -1;
			}
		else
			{  /* search for tab to left */
			itbd = itbdMac -1;
			itbdEnd = 0;
			pdxa = &(*hrsd)->pap.rgdxaTab [itbd];
			while (itbd >= itbdEnd && *pdxa >= xa)
				{
				pdxa--;
				itbd--;
				}
			/* if itbd < itbdEnd then itbd < 0 */
			}

	if (itbd >= 0)
		/* we found a tab, use it */
		return *pdxa;

	/* else use next default tab */
	if (wDir > 0)
		xa += dxaTab;

	return dxaTab * (xa / dxaTab);
}




/*  DrawRulerCursor
	places a cursor (an inverted rec) or removes a cursor (same action) on the
	ruler at xa.  The cursor is used in keyboard EditTabs mode.
*/

/* %%Function:DrawRulerCursor   %%Owner:peterj */
DrawRulerCursor (hdc, preb, xa)
HDC hdc;
struct REB *preb;
int xa;

{
	struct RC rc;
	int xpCenter, dxa = mputdxaCursorDiv2[vpref.ut];

	xpCenter = XpMarkFromXaMark (xa, preb->hrsd);

	rc.ypTop = ypMarkTop;
	rc.ypBottom = ypMarkBottom;
	rc.xpLeft = XpMarkFromXaMark (xa - dxa, preb->hrsd);
	rc.xpRight = XpMarkFromXaMark (xa + dxa, preb->hrsd);

	if (xa < xaMin || xa > xaMax || rc.xpRight < preb->xpLeft
			|| rc.xpLeft > preb->xpRight)
		/* nothing to display */
		return;

	InvertRect (hdc, (LPRECT) &rc);
}


/* RULER MARK DRAG COMMAND */


/*  DragMark
	This function is called on a button down in the ruler area.  It allows
	the user to move a tab or an indent or to insert a new tab.
*/

/* %%Function:DragMark   %%Owner:peterj */
DragMark (hrsd, ptMouse)
struct RSD ** hrsd;
struct PT ptMouse;

{
	int xa;            /* current mark position */
	int xaLast;        /* where mark was on last pass through loop */
	int xaOld;         /* where the mark started out */
	int ms;            /* old mark state (normal, gray or inverted) */
	int rk;            /* ruler mode */

	int imk = iNil;    /* type of mark being dragged */
	int i = -1;        /* if the mark is a tab/col/tc which one is it */
	int iMac;          /* number of defined tabs/cols/tcs */

	int fOnTrack;      /* is the mark still on the ruler or is it dragged off */
	int fOnTrackLast;  /* was it on track during the last pass */

	int fCanScroll=fFalse;/* don't scroll until you have moved somewhere */
	int fLockOnLeft1=fFalse;/* indicates left & left1 are locked together */
	int xaLeft1;       /* used when left & left1 are locked together */
	int xaLeft1Last;   /* previous left1 position */
	int xaLeft1Old;    /* original dxaLeft1+dxaLeft val */
	int msLeft1;       /* mark state of left1 */
	int xaLow = xaMin; /* low value permitted */
	int xaHigh = xaMax;/* high value permitted */

	int ddypMark;      /* used to differentiate left from left1 vertically */
	int dxaLeft1;      /* old value */

	int xpRight;       /* width of screen (for scrolling) */

	HDC hdc = NULL;    /* screen dc */
	struct REB *preb;
	struct RSD *prsd;
	struct TBD tbd;    /* tab descriptor for tab being dragged */
	struct REB reb;    /* ruler edit bolck */

	Assert (vpreb == NULL);

	UpdateRuler(hmwdCur, fFalse /*!fInit*/, -1/*rk*/, fFalse /*fAbortOK*/);

	FreezeHp();
	prsd = *hrsd;

	/* convert the mouse xp to a quantized mark xa */
	xa = XaMarkPxp (&ptMouse.xp, hrsd, xaMin, xaMax);

	ddypMark = dypMark / 2;
	dxaLeft1 = prsd->pap.dxaLeft1;
	xaLeft1Old = xaLeft1 = dxaLeft1 + prsd->pap.dxaLeft;


	/* figure out what we are dragging */

	if ((rk = prsd->rk) == rkNormal)
		{
		int *pdxa;
		/* first: check for an existing tab */
		for (i = 0, iMac = prsd->pap.itbdMac, pdxa = prsd->pap.rgdxaTab;
				i < iMac; i++, pdxa++)
			if (FPointNearMark (*pdxa, xa))
				{  /* this is it */
				xaOld = *pdxa;
				tbd = prsd->pap.rgtbd [i];
				imk = tbd.jc;
				ms = prsd->fTabGray ? msGray : msNormal;
				goto LHaveMark;
				}
		i = -1;

		/* next check indents */

		/* check for Left1 */
		/* Note:  we consider the mouse to be on Left1 iff 1) xa is near to
			Left1 and not near to Left OR 2) xa is near to both and above
			the center of the mark area */

		if (FPointNearMark (xaLeft1, xa) &&
				(!FPointNearMark (prsd->pap.dxaLeft, xa) ||
				ptMouse.yp < ypMark + ddypMark))
			{
			imk = imkIndLeft1;
			xaOld = xaLeft1;
			ms = prsd->papGray.dxaLeft1 ? msGray : msNormal;
			goto LHaveMark;
			}

		/* now check left */
		if (FPointNearMark (prsd->pap.dxaLeft, xa))
			{
			imk = imkIndLeft;
			xaOld = prsd->pap.dxaLeft;
			ms = prsd->papGray.dxaLeft ? msGray : msNormal;
			if (fLockOnLeft1 = !vfShiftKey)
				/* left1 moves with left */
				msLeft1 = prsd->papGray.dxaLeft1 ? msGray : msNormal;
			goto LHaveMark;
			}

		/* last check if it is right */
		if (FPointNearMark (prsd->dxaColumnNorm -
				prsd->pap.dxaRight, xa))
			{
			imk = imkIndRight;
			xaOld = prsd->dxaColumnNorm - prsd->pap.dxaRight;
			ms = prsd->papGray.dxaRight ? msGray : msNormal;
			goto LHaveMark;
			}

		/* not an indent => must be a new tab.  Insert one if we can */
		if (prsd->pap.itbdMac < itbdMax)
			{
			xaOld = xaNil;  /* flags new tab */
			ms = prsd->fTabGray ? msGray : msNormal;
			imk = tbd.jc = prsd->jcTabCur;
			tbd.tlc = tlcDefault;
			goto LHaveMark;
			}
		}

	else  if (rk == rkTable)
		{
		int *pdxa;
		ms = prsd->fTcGray ? msGray : msNormal;

		/* left marker */
		if (FPointNearMark((xaOld = prsd->rgdxaCenter[0] +
				prsd->dxaGapHalf), xa))
			{
			imk = imkTLeft;
			xaLow = xaLeftMinSci + prsd->dxaGapHalf;
			goto LHaveMark;
			}

		for (i = 0, iMac = prsd->itcMac, pdxa = &prsd->rgdxaCenter[1]; 
				i < iMac; i++, pdxa++)
			if (FPointNearMark(*pdxa, xa))
				{
				xaOld = *pdxa;
				xaLow = prsd->rgdxaCenter[i] + prsd->dxaGapHalf*2;
				imk = imkTableCol;
				goto LHaveMark;
				}
		}

	else  /* rk == rkPage */		
		{
		int dxaCol = prsd->dxaColumnPage;
		int ddxaCol = prsd->ddxaColumns;
		int icol, icolMac = prsd->ccolM1;
		ms = msNormal;

		xaLow = prsd->dxaExtraLeft;
		xaHigh = prsd->xaPage-prsd->dxaExtraRight;

		/* right margin? */
		if (FPointNearMark((xaOld = prsd->xaPage-prsd->dxaRightMar
				-prsd->dxaExtraRight), xa))
			{
			i = 1;
			imk = imkRightMargin;
			xaLow += prsd->dxaLeftMar+
					PdodDoc(DocMother(selCur.doc))->dop.dxaTab + 100;
			goto LHaveMark;
			}

		/* left margin? */
		xaOld = prsd->dxaLeftMar+prsd->dxaExtraLeft;
		if (FPointNearMark(xaOld, xa))
			{
			i = 0;
			imk = imkLeftMargin;
			xaHigh -= prsd->dxaRightMar+
					PdodDoc(DocMother(selCur.doc))->dop.dxaTab + 100;
			goto LHaveMark;
			}

		if (!PdodDoc(selCur.doc)->fShort)
			{
			/* column? */
			for (icol = 0, i = 2; icol < icolMac; icol++)
				{
				xaOld += dxaCol;
				i++;
				if (FPointNearMark(xaOld, xa))
					{
					imk = imkRightMargin;
					goto LHaveMark;
					}
				xaOld += ddxaCol;
				i++;
				if (FPointNearMark(xaOld, xa))
					{
					imk = imkLeftMargin;
					goto LHaveMark;
					}
				}
			}
		}

	/* could not find a mark to drag or could not insert a tab */
	Beep();
	MeltHp();
	return;


LHaveMark:
	MeltHp();
	/* At this point:
		imk = the mark being dragged or the tab type
		i = the index of an existing tab or -1
		xaOld = the original xa of the mark were dragging. xaNil for new tab
		xaLow = minimum permissible value
		xaHigh = maximum permissible value
		xaLeft1Old = the original xa for Left1
		dxaLeft1 = the original (*hrsd)->dxaLeft1 value
		ms = the current state of the mark being dragged
		msLeft1 = the current state of Left1 (only if fLockOnLeft1)
	*/

#ifdef RSH
	LogUa(uaDragMark + imk);
#endif /* RSH */

	/* set up the dc, reb and the vpreb for the drag */

	/* initialize the preb & point vpreb to it */
	preb = PrebInitPrebHrsd (&reb, hrsd, fTrue /* fGlobal */ );
	xpRight = preb->xpRight;

	/* set the do-not-draw flags so that, if we scroll, we won't draw
		the mark we are dragging */
	preb->imkNoDraw = fLockOnLeft1 ? imkIndCombo : imk;
	preb->iNoDraw = i; /* if indent or new tab; == -1 */

	/* get the dc's */
	if (! FSetDcAttribs (hdc = GetDC ((*hrsd)->hwndMark), dccRuler))
		{
		ErrorNoMemory(eidNoMemDisplay);
		if (hdc != NULL)
			ReleaseDC ((*hrsd)->hwndMark, hdc);
		FreePreb (preb);
		return;
		}

	/* get all mouse messages */
	SetCapture ((*hrsd)->hwndMark);

	/* if the mark exists on the screen now, erase it */
	if (xaOld >= xaMin)
#ifdef WIN23
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xaOld, hrsd),
				imk, ms, fTrue);
#else
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xaOld, hrsd),
				imk, ms);
#endif /* WIN23 */

	/* erase Left1 if we are moving it too */
	if (fLockOnLeft1)
#ifdef WIN23
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1Old, hrsd),
				imkIndLeft1, msLeft1, fTrue);
#else
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1Old, hrsd),
				imkIndLeft1, msLeft1);
#endif /* WIN23 */

	ptMouse.yp = ypMark; /* forces fOnTrack initially true */
	fOnTrack = fFalse;   /* causes the mark to be drawn */


	/* DRAG LOOP */
	do
		{
		/* save the state on the last pass */
		xaLast = xa;
		fOnTrackLast = fOnTrack;
		xaLeft1Last = xaLeft1;

		/* is the mouse now On Track (over the mark area) */
		/* note:  only tabs can be dragged off of the ruler */

		fOnTrack = imk >= imkIndMin || (ptMouse.yp <= ypMarkBottom + ddypMark
				&& ptMouse.yp >= ypMarkTop - ddypMark);

		/* if we are beyond the left or right edge, scroll */

		if (fOnTrack && fOnTrackLast && fCanScroll &&
				((ptMouse.xp <= 0 && xa > xaLow &&
				((*hrsd)->xpZero != 0 || GetKeyState(VK_SHIFT) < 0)) ||
				(ptMouse.xp >= xpRight && xa < xaHigh)))
			{
			/* fOnTrackLast guarantees that the mark is currently visible,
				so erase it so that it is not scrolled. */

#ifdef WIN23
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLast, hrsd),
					imk, msInvert, fTrue);
#else
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLast, hrsd),
					imk, msInvert);
#endif /* WIN23 */

			/* if were dragging it, erase left1 too */
			if (fLockOnLeft1)
#ifdef WIN23
				BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1Last, hrsd),
						imkIndLeft1, vsci.fWin3Visuals ? msInvert : msLeft1, fTrue);
#else
				BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1Last, hrsd),
						imkIndLeft1, msLeft1);
#endif /* WIN23 */

			fOnTrackLast = fFalse; /*  so we don't try to erase it again */

			if (ptMouse.xp <= 0)
				{
				ptMouse.xp = 0;
				ScrollRight (wwCur, xpRulerScroll);
				UpdateWw(wwCur, fTrue);
				}
			else  /* ptMouse.xp >= xpRight */				
				{
				ptMouse.xp = xpRight -1;
				ScrollLeft (wwCur, xpRulerScroll);
				UpdateWw(wwCur, fTrue);
				}
			}

		/* quantize the point & get its xa */
		/* this is done after the scroll so that the xa will reflect the new
			position (if any) */
		xa = XaMarkPxp (&ptMouse.xp, hrsd, xaLow, xaHigh);

		/* prevent hyperactive scrolling */
		ptMouse.xp = min(max(ptMouse.xp, 0), xpRight-1);

		if (fLockOnLeft1)
			xaLeft1 = xa + dxaLeft1;

		/* determine if we need to erase, draw or move the mark */

		if (xa != xaLast || fOnTrack != fOnTrackLast)
			/* either it has moved or appeared or disappeared */
			{

			/* allow user to scroll window left/right only after they have
				made some explicit movement (prevents scroll on click) */
			if (!fCanScroll && xa != xaLast)
				fCanScroll = fTrue;

			if (fOnTrackLast)
				{  /* erase from old location */
#ifdef WIN23
				BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLast, hrsd),
						imk, msInvert, fTrue);
#else
				BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLast, hrsd),
						imk, msInvert);
#endif /* WIN23 */

				/* erase Left1 too, if it is locked on */
				if (fLockOnLeft1)
#ifdef WIN23
					BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1Last,
							hrsd), imkIndLeft1, vsci.fWin3Visuals ? msInvert : msLeft1, fTrue);
#else
					BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1Last,
							hrsd), imkIndLeft1, msLeft1);
#endif /* WIN23 */
				}

			if (fOnTrack)
				{  /* draw in its current location */
#ifdef WIN23
				BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
						imk, msInvert, fFalse);
#else
				BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd),
						imk, msInvert);
#endif /* WIN23 */

				/* draw Left1 too, if it is locked on */
				if (fLockOnLeft1)
#ifdef WIN23
					BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1,
							hrsd), imkIndLeft1,
							vsci.fWin3Visuals ? msInvert : msLeft1, fFalse);
#else
					BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1,
							hrsd), imkIndLeft1, msLeft1);
#endif /* WIN23 */
				}
			}

		/*  END OF DRAG LOOP  */

		/* this gets all mouse messages & updates ptMouse */
		} 
	while (FStillDownReplay (&ptMouse,fFalse));


	/* clean up display */

	if (fOnTrack)
		{  /* remove inverted mark & display new mark */

		/* remove old mark */
#ifdef WIN23
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd), imk,
				msInvert, fTrue);
		if (fLockOnLeft1)
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1,
					hrsd), imkIndLeft1, msInvert, fTrue);
#else
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd), imk,
				msInvert);
		/* note:  if fLockOnLeft1 Left1 is already in the right place in the
					right state so we don't care about it */
#endif /* WIN23 */

		/* if the mark was a tab then it goes back in the same state as
			it started out.  Any other mark goes out
			normal. */

		/* draw new mark */
#ifdef WIN23
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd), imk,
				(imk > imkIndMin ? msNormal : ms), fFalse);
		if (fLockOnLeft1)
			BltRulerMark (hdc, preb, XpMarkFromXaMark (xaLeft1,
					hrsd), imkIndLeft1, msLeft1, fFalse);
#else
		BltRulerMark (hdc, preb, XpMarkFromXaMark (xa, hrsd), imk,
				(imk > imkIndMin ? msNormal : ms));
#endif /* WIN23 */
		}
	else
		/* no new tab or tab removed */
		xa = xaNil;

	/* if 1) a new tab was not inserted (xa = xaOld = xaNil) or
		2) an existing mark that was not gray was returned to the
		same position it started (xa = xaOld && ms = msNormal)
		then the drag has no effect and no change to the ruler
		sprm is required. */
	/* We want to let CBT veto any changes, if CBT is active. */

	if ( ! (xa == xaOld  && (xa == xaNil || ms == msNormal)) )
		if (imk < imkTabMax)
			{
			/* tab */
			if (!vhwndCBT || FCBTChangeTab(xa, xaOld))
				ChangeRulerTab (hrsd, xa, xaOld, tbd, 0);
			}
		else
			{
			/* indent/column/etc */
			if (imk == imkTableCol)
				{
				long dxaT;
				int iT = i;

				while ((long)(*hrsd)->rgdxaCenter[iT] == xaNil)
					iT--;

				if ((dxaT = (long) xa - (long) (*hrsd)->rgdxaCenter[iT])	> (long) xaRightMaxSci ||
						!FOkExpandTable(&selCur.ca, fFalse, i, i + 1,
						(int) dxaT, wNinch, NULL, NULL, NULL))
					{
					Beep();
					goto LEndDrag;
					}
				}
			else  if (imk == imkTLeft)
				{
				if (!FOkExpandTable(&selCur.ca, fFalse, -1, 0, uNinch, xa, NULL, NULL, NULL))
					{
					Beep();
					goto LEndDrag;
					}
				}

			if (!vhwndCBT || FCBTChangeRuler(imk, xa, xaOld, fLockOnLeft1))
				{
				ChangeRulerImk(hrsd, imk, i, xa, xaOld);
				if (imk == imkIndLeft && !fLockOnLeft1)
					ChangeRulerImk(hrsd, imkIndLeft1, i, xaLeft1Old, xaOld);
				}
			}

LEndDrag:
	/* regardless of any changes we have or have not made, we would like the
		ruler mark area to be redrawn.  To allow for the case where the user is
		clicking multiple times in the ruler we don't want to do so here.  These
		two flags cause the ruler to update and to guarantee that it will redraw
		the mark area on the next time through idle.  */

	(*hrsd)->rpc = rpcMarkArea;
	selCur.fUpdateRuler = fTrue;

	/* releases drawing resources obtained during drag */
	FreePreb (preb);

	/* when mouse is captured we miss keyboard messages. assure kbd state */
	ReleaseCapture ();
	SetOurKeyState ();

	ReleaseDC ((*hrsd)->hwndMark, hdc);
}


/* RULER CHANGE UTILITIES */



/*  ChangeRulerTab
	changes one tab, applying to rulergrpprl, hrsd and selcur.  sets
	fUpdatePap to ensure that, on next idle, we recompute grayness
*/

/* %%Function:ChangeRulerTab   %%Owner:peterj */
ChangeRulerTab (hrsd, dxaIns, dxaDel, tbd, dxaClose)
struct RSD ** hrsd;
int dxaIns, dxaDel;
struct TBD tbd;
int dxaClose;

{
	extern BOOL vfSeeSel;
	CHAR prl [15];
	int cch;

	/* get the prl */
	cch = CchPrlChangeOneTab (prl, dxaIns, dxaDel, tbd, dxaClose);
	Assert (cch <= 15);

	/* apply the prl */
	AddRulerGrpprl (hrsd, prl, cch);

	/* update the hrsd & selCur.  note we are not changing the papGrays.
		They will be updated at idle. */
	UpdateSelCurProp (sgcPap, prl, cch, NULL, 0);
	UpdateRulerProp (hrsd, prl, cch, NULL, 0);

	/* this assures we will update at idle */
	selCur.fUpdatePap = fTrue;

	/*  don't want to scroll unnecessarily */
	vfSeeSel = fFalse;

	if (vfRecording)
		RecordRulerTab(dxaIns, dxaDel, tbd);
}





/*  ChangeRulerImk
	Modifies a single ruler item.
*/

/* %%Function:ChangeRulerImk   %%Owner:peterj */
ChangeRulerImk (hrsd, imk, i, xa, xaOld)
struct RSD ** hrsd;
int imk, i, xa, xaOld;
{
	struct SEL *psel = PselActive();
	int doc = psel->doc;
	struct RSD *prsd = *hrsd;
	BOOL fInval = fFalse;
	int val;
	CHAR sprm = mpimksprm [imk - imkIndMin];
	CHAR prl [5], *pprl;
	struct CA caInval;

	pprl = prl;

	switch (imk)
		{
	case imkIndLeft:
		val = xa;
		break;

	case imkIndLeft1:
		val = xa - prsd->pap.dxaLeft;
		break;

	case imkIndRight:
		val = prsd->dxaColumnNorm - xa;
		break;

	case imkLeftMargin:
	case imkRightMargin:
		if (i >= 2)
			/* change column width */
			{
			int xaT, dxaCol;
			if (imk == imkLeftMargin)
				xaT = xaOld - xa;
			else
				xaT = xa - xaOld;
			dxaCol = prsd->dxaColumnPage + xaT;
			xaT = prsd->xaPage - prsd->dxaExtraRight - prsd->dxaRightMar
					- prsd->dxaLeftMar - prsd->dxaExtraLeft;
			xaT -= dxaCol * (prsd->ccolM1 + 1);
			Assert(prsd->ccolM1 > 0);
			val = max(xaT / prsd->ccolM1, 0);
			CacheSect(doc, psel->cpFirst);
			PcaSet(&caInval, doc, caSect.cpFirst, caSect.cpLim);
			fInval = fTrue;
			}
		else
			{
			struct DOD *pdod = PdodMother(doc);
			struct DOP *pdop = &pdod->dop;

			FreezeHp();
			pdod->fLRDirty = fTrue;
			pdod->fDirty = fTrue;
			sprm = sprmNoop;
			PcaSet(&caInval, doc, cp0, CpMacDoc(doc));
			fInval = fTrue;
			if (i == 1)
				/* change right margin */
				val = pdop->dxaRight = prsd->xaPage - prsd->dxaExtraRight - xa;
			else  /* i == 0 */
				/* change left margin */
				val = pdop->dxaLeft = xa - prsd->dxaExtraLeft;
			MeltHp();

			if (vfRecording)
				RecordRulerDoc(i, val);
			}
		break;

	case imkTLeft:
		val = xa;
		break;

	case imkTableCol:
			{
			int xaT;
			int iT = i;

			while (prsd->rgdxaCenter[iT] == xaNil)
				iT--;

			xaT = xa - prsd->rgdxaCenter[iT];
			prl[0] = sprmTDxaCol;
			prl[1] = i;
			prl[2] = i+1;
			bltb(&xaT, &prl[3], sizeof(int));
			pprl += 5;

			/* invalidate table caches */
			vtcc.ca.doc = docNil;
			vtcxs.ca.doc = docNil;

			if (vfRecording)
				RecordRulerMargins(sprmTDxaCol, xaT);
			break;
			}

#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */
		}

	if (sprm != sprmNoop)
		{
		if (vfRecording)
			RecordRulerMargins(sprm, val);

		pprl += CchPrlFromSprmVal (pprl, sprm, val);
		}

	Assert (pprl - prl <= 5);

	if (prsd->rk == rkNormal)
		/* apply ruler sprm */
		{
		int fUpdatePap;
		CHAR prlGray [3], *pprlGray;

		pprlGray = prlGray;
		pprlGray += CchPrlFromSprmVal (pprlGray, sprm, 0);
		Assert (pprlGray - prlGray <= 3);

		/* When we exit this routine we want fUpdatePap to be what ever it was when
			we started.  If it is true it indicates that the ruler was out of sync
			and so the ruler will still be out of sync.  If it is false then the
			ruler is in sync and it still will be when we finish because we call
			UpdateSelCurProp */

		fUpdatePap = selCur.fUpdatePap;

		/* apply to the selection via sprmPRuler */
		AddRulerGrpprl (hrsd, prl, pprl - prl);

		/* avoid the need to update the ruler by directly updating selCur & hrsd */
		UpdateSelCurProp (sgcPap, prl, pprl - prl, prlGray, pprlGray - prlGray);
		UpdateRulerProp (hrsd, prl, pprl - prl, prlGray, pprlGray - prlGray);

		/* fUpdatePap remains in its previous state */
		selCur.fUpdatePap = fUpdatePap;
		}

	else  if (pprl - prl > 0)
		/* apply to selCur */
		ApplyGrpprlSelCur(prl, pprl - prl, fTrue);

	else
		SetUndoNil();

	if (fInval)
		{
		InvalCp(&caInval);
		InvalDoc(doc);
		InvalPageView(doc);
		}
}



/*  CchPrlChangeOneTab
	creates a prl to change one tab.  may have zero or one deletions and may
	have zero or one additions.
*/
/* %%Function:CchPrlChangeOneTab   %%Owner:peterj */
CchPrlChangeOneTab (prl, dxaIns, dxaDel, tbd, dxaClose)
char * prl;
int dxaIns, dxaDel;
char tbd;
int dxaClose;
{
	char * pprl = &prl [2];

	prl [0] = sprmPChgTabs;

	if (dxaDel >= xaMin)
		{  /* delete tab at dxaDel */
		pprl [0] = 1;  /* idxaDelMax */
		bltb (&dxaDel, &pprl [1], sizeof (uns));   /* rgdxaDel */
		bltb (&dxaClose, &pprl [3], sizeof (uns)); /* rgdxaClose */
		pprl += 5;
		}
	else
		/* no tab to delete */
		*pprl++ = 0;

	if (dxaIns >= xaMin)
		{  /* add tab tbd at dxaIns */
		pprl [0] = 1;  /* idxaAddMax */
		bltb (&dxaIns, &pprl[1], sizeof (uns));  /* rgdxaAdd */
		pprl [3] = tbd;  /* rgtbdAdd */
		pprl += 4;
		}
	else
		/* no tab to add */
		*pprl++ = 0;

	prl [1] = pprl - &prl [2];

	return pprl - prl;
}


/*  XaMarkPxp
	quantize xp to the nearest dxaQuantum.  store the xp for the point in pxp
	and return the xa
*/
/* %%Function:XaMarkPxp   %%Owner:peterj */
XaMarkPxp (pxp, hrsd, xaLow, xaHigh)
int *pxp;
struct RSD ** hrsd;
int xaLow;
int xaHigh;
{
	int xa;
	int xp;

	if (vpreb != NULL)
		xp = max( min( *pxp, vpreb->xpRight ), 0 );
	else
		xp = *pxp;

	xa = min(max(xaLow, XaMarkFromXpMark (xp, hrsd)), xaHigh);
	*pxp = XpMarkFromXaMark (xa, hrsd);

	return xa;
}



/*  FPointNearMark
	returns true if xa1 is "close" to xa2
*/
/* %%Function:FPointNearMark   %%Owner:peterj */
FPointNearMark (xa1, xa2)
int xa1, xa2;
{
	int dxa = mputdxaCursorDiv2[vpref.ut];
	return ((xa1 >= xa2 - dxa) && (xa1 <= xa2 + dxa));
}





/* R I B B O N */


/* I B  P R O C  R I B B O N  C M D */
/* %%Function:IbProcRibbonCmd  %%Owner:peterj */
IbProcRibbonCmd(hwnd, ibm, iibb, wParam, lParam)
HWND hwnd;
WORD ibm;
WORD iibb;
WORD wParam;
LONG lParam;
{
	switch (ibm)
		{
	case ibmDblClk:
		TermCurIBDlg(fFalse);  /* put focus back in doc */
		CmdExecBcmKc(bcmCharacter, kcNil);
		break;

	case ibmInit:
			/* terminate Dyadic Move Mode etc if on. */
		SpecialSelModeEnd();
		if (selCur.fUpdateChpGray)
			FGetCharState(fFalse, fFalse);
		if (selCur.fUpdateRibbon)
			UpdateRibbon(fFalse);
		/* force pts to refill since we may not get an itmfocus message */
		InvalidateIibb(hwnd, IDLKSPOINT);
		break;

	case ibmTerm:
#ifdef DBGIB
		CommSz(SzShared("Normal termination on ribbon.\r\n"));
#endif /* DBGIB */
#ifdef RSH
		LogUa(uaFontPtIb);
#endif /* RSH */

			/* parse the stuff from the combos and apply it! */
			{
			CHAR grpprl[8];
			int cch=0;
			int ftc = ValGetIibbHwndIb(hwnd, IDLKSFONT);
			int hps = ValGetIibbHwndIb(hwnd, IDLKSPOINT);

			Assert (vssc == sscNil);
			if (ftc >= 0)
				{
				cch = CchPrlFromSprmVal(grpprl, sprmCFtc, ftc);
				rrf.ftc = ftc;
				}
			if (hps >= 0)
				{
				cch += CchPrlFromSprmVal(grpprl+cch, sprmCHps, hps);
				rrf.hps = hps;
				}
			if (cch > 0)
				{
				ApplyGrpprlSelCur(grpprl, cch, fTrue);
				if (!vmerr.fMemFail)
					FSetAgainGrpprl(grpprl, cch, bcmFormatting);

				if (vfRecording)
					{
					char sz[cchMaxSz];

					if (ftc >= 0) 
						{
						FlushVrac();
						RecordSt(StSharedKey("Font ", CmdFont));
						GetTextIibbHwndIb(hwnd, IDLKSFONT, sz, cchMaxSz);
						SzToStInPlace(sz);
						RecordQuotedSt(sz);
						if (hps < 0) RecordEop();
						}

					if (hps >= 0) 
						{
						if (ftc >= 0) 
							{
							RecordComma();
							GetTextIibbHwndIb(hwnd, IDLKSPOINT, sz, cchMaxSz);
							RecordSz(sz);
							RecordEop();
							}
						else  
							{
							FlushVrac();
							RecordSt(StSharedKey("FontSize ", CmdFontSize));
							GetTextIibbHwndIb(hwnd, IDLKSPOINT, sz, cchMaxSz);
							RecordSz(sz);
							RecordEop();
							}
						}
					}

				ClearRulerRrf();
				}
			/*  get the correct values displayed */
			InvalidateFontPointLH();

			break;
			}

	case ibmCancel:
#ifdef DBGIB
		CommSz(SzShared("Ribbon cancelled.\r\n"));
#endif /* DBGIB */
		Assert (vssc == sscNil);
		InvalidateFontPointLH();
		break;
		}
}




/* RULER GRPPRL UTILITIES */


/*  AddRulerGrpprl
	this function adds a grpprl to the global ruler grpprl.  if the global
	grpprl no longer applies to the current selection it is flushed to make
	room for the new grpprl.  if this happens send a sprmPRuler to the current
	selection so that it will know to quote the ruler grpprl.
*/
/* %%Function:AddRulerGrpprl   %%Owner:peterj */
AddRulerGrpprl (hrsd, prl, cb)
struct RSD ** hrsd;
char * prl;
int cb;
{
	extern BOOL vfSeeSel;
	int cbPrc;
	struct CA caSel, caInval;
	struct MPRC prc;
	CP cpFirst, dcp;
	struct CA caT;

	/* table selection */
	if (selCur.sk == skColumn)
		{
		ApplyGrpprlSelCur(prl, cb, fTrue);
		cpFirst = CpFirstTap(selCur.doc, selCur.cpFirst);
		CpFirstTap(selCur.doc, selCur.cpLim - (selCur.fIns ? 0 : ccpEop));
		dcp = caTap.cpLim - cpFirst;
		InvalTableCp(selCur.doc, cpFirst, dcp);
		InvalText(PcaSetDcp(&caT, selCur.doc, cpFirst, dcp), fFalse);
		goto LRet;
		}

	if (selCur.fBlock)  /* note: table case of fBlock handled by skColumn above */
		{
		/* turn the block selection into a normal selection */
		Select(&selCur, selCur.cpFirst, CpLimBlock());
		}

	caSel = selCur.ca;

	ExpandCaCache (&caSel, &caInval, &caPara, sgcPap, CacheParaRUL);

	if (FNeRgch(&vrulss.caRulerSprm, &caSel, sizeof (struct CA))
			|| vrulss.hrsd != hrsd)
		{  /* the grpprls in vrulss do not apply to this selection */
		int rgb[2];

		rgb[0] = sprmPRuler;
		rgb[1] = 0;

		if (vrulss.caRulerSprm.doc != docNil)
			FlushRulerSprms ();
		ApplyGrpprlSelCur (&rgb, 2, fTrue);
		if (vmerr.fMemFail)
			/* ApplyGrpprl was cancelled, bag out */
			{
			SetAgain(bcmNil);
			return;
			}
		vrulss.caRulerSprm = caSel;
		vrulss.sk = selCur.sk;
		if (selCur.fTable)
			{
			vrulss.itcFirst = selCur.itcFirst;
			vrulss.itcLim = selCur.itcLim;
			}
		vrulss.cbGrpprl = 0;
		vrulss.hrsd = hrsd;
		}

	Assert (vrulss.caRulerSprm.doc != docNil);

	/* set up for & perform the merge */

	if ((cbPrc = vrulss.cbGrpprl) > 0)
		blt (*vrulss.hgrpprl, &prc.grpprl, CwFromCch (cbPrc));

	MergeGrpprls(&prc.grpprl, &cbPrc, prl, cb);

	if (cbPrc <= cbMaxGrpprl)
		/* we didn't overflow the grpprl.  use the merged grpprl */
		{
		if (vrulss.hgrpprl == hNil)
			/* no old hgrpprl, allocate a new one */
			vrulss.hgrpprl = HAllocateCw (CwFromCch (cbPrc));

		if (vrulss.hgrpprl != hNil && 
				/* assure hgrpprl is big enough for new grpprl */
		FChngSizeHCw (vrulss.hgrpprl, CwFromCch (cbPrc),
				fFalse /*fShrink*/ ))
			{
			bltb (prc.grpprl, *vrulss.hgrpprl, cbPrc);
			vrulss.cbGrpprl = cbPrc;
			}
		}

	/* do this before we return for table code */
	InvalSoftlyPgvwPara(selCur.doc, prl, cb);

	/* cause paras selected to reflect the change */
	InvalCp (&caInval);

	/*  advise anyone else who cares */
	Assert(caSel.doc == selCur.doc);
	InvalText(&caSel, fFalse /*fEdit*/);

	vfSeeSel = fTrue;

LRet:
	rrf.fSelMoved = fTrue;
	rrf.doc = selCur.doc;
	SetAgain(bcmRRFormat);


}


/*  FlushRulerSprms
	this function is called to cause the global ruler sprm to be actually
	applied to the ca to which it refers (clobbering the sprmPRuler which is
	there now).
*/
/* %%Function:FlushRulerSprms   %%Owner:peterj */
EXPORT FlushRulerSprms ()
{
	struct SEL selT;
	CHAR grpprl [cbMaxGrpprl];

	if (vrulss.caRulerSprm.doc == docNil)
		return;

	/* the ca in vrulss has already been expanded to its para boundries.  No
		call to ExpandCaCache is needed */

	/* by definition of the use of sprmPRuler, applying this grpprl will not
		change the appearance of anything on the screen so no invalidation is
		needed */

	AssertH(vrulss.hgrpprl);

	bltb (*vrulss.hgrpprl, grpprl, vrulss.cbGrpprl);

	selT = selCur;
	selCur.ca = vrulss.caRulerSprm;
	selCur.sk = vrulss.sk;
	if (selCur.fTable)
		{
		selCur.itcFirst = vrulss.itcFirst;
		selCur.itcLim = vrulss.itcLim;
		}

	/* Invalidate BEFORE calling ApplyGrpprlSelCur */
	vrulss.caRulerSprm.doc = docNil;
	ApplyGrpprlSelCur(grpprl, vrulss.cbGrpprl, fFalse /* fSetUndo */);
	selCur = selT;

	if (rrf.fSelMoved & !rrf.fNoClear)
		{
		ClearRrf();
		}

	if (rrf.hgrpprl != hNil)
		{
		FreePh(&rrf.hgrpprl);
		rrf.hgrpprl = hNil;
		rrf.cbGrpprl = 0;
		}
	if (vrulss.cbGrpprl != 0)
		{
		rrf.hgrpprl = HAllocateCw(CwFromCch(vrulss.cbGrpprl));
		if (rrf.hgrpprl != hNil)
			{
			bltbyte(grpprl, *(rrf.hgrpprl),
					rrf.cbGrpprl = vrulss.cbGrpprl);
			ClearRibbonRrf();
			}
		}

	/* NOTE:  since we might very well immediately add a grpprl to the
				ruler grpprl, we don't invoke the overhead of heap actions
				by freeing here.  Instead the ruler grpprl will be checked
				late in idle and any unused space will be freed.
	*/

	vrulss.cbGrpprl = 0;
	vrulss.hrsd = hNil;
}



/*  CheckRulerGrpprlSize
	called from idle, this function determines if the ruler grpprl is any
	larger then it needs to be and if it is shrinks or frees it.
*/
/* %%Function:CheckRulerGrpprlSize   %%Owner:peterj */
CheckRulerGrpprlSize ()
{
	if (vrulss.hgrpprl != hNil)
		if (vrulss.cbGrpprl != 0)
			FChngSizeHCw (vrulss.hgrpprl, CwFromCch (vrulss.cbGrpprl),
					fTrue /* fShrink */ );
		else
			FreePh (&vrulss.hgrpprl);
}


/* InvalStyle

	sets the style entry in the current window's ruler to an invalid value and
	sets the selCur.fUpdateRuler to ensure that the field will be restored
	next time through idle.

*/

/* %%Function:InvalStyle  %%Owner:peterj */
InvalStyle()
{
	struct RSD ** hrsd;
	HWND hwndRuler;

	if (hmwdCur == hNil)	/* happens if WindowClose when combo dropped */
		return;

	if ((hwndRuler = (*hmwdCur)->hwndRuler) != NULL)
		{
		hrsd = HrsdFromHwndRuler(hwndRuler);
		(*hrsd)->pap.stc = stcStdMin;
		selCur.fUpdateRuler = fTrue;
		}
}


/* InvalidateFontPointLH
	sets the font and point entries in chpRibbon to invalid values and
	sets the selCur.fUpdateChp to ensure that the fields will be restored
	next time through idle.
*/
/* %%Function:InvalidateFontPointLH  %%Owner:peterj */
InvalidateFontPointLH()
{
	chpRibbon.ftc = valNil;
	chpRibbon.hps = valNil;
	selCur.fUpdateRibbon = fTrue;
}



/* D I R T Y  R I B B O N  F O N T  L I S T S */
/* %%Function:DirtyRibbonFontLists  %%Owner:peterj */
DirtyRibbonFontLists()
{
	if (vhwndRibbon != NULL)
		{
		InvalidateIibb(vhwndRibbon, IDLKSFONT);
		InvalidateIibb(vhwndRibbon, IDLKSPOINT);

/*  Historical Note: bz sdm fixed the problem so the update was removed:

	force total ribbon update.  For some reason,
	the above Invalidate call changes the editcontrol contents when all we are 
	trying to do is force refilling of the list box (bug 4182). I have added 
	the following call to force the editcontrol back to its correct contents
	although it may briefly show the wrong contents. 
	(InvalidateIibb calls RedisplayTmc in SDM which eventually calls FTopList 
	with top==topRepaint, which resets the contents of the combo editcontrol
	intentionally for reasons unknown. */

		}
}


/* D O  R U L E R  C O M M A N D */
/* %%Function:DoRulerCommand  %%Owner:peterj */
DoRulerCommand(hrsd, idRul)
struct RSD **hrsd;
int idRul;
{
	int val;
	int ilcd;

	/* terminate Block or Extend or dyadic move mode if on */
	SpecialSelModeEnd();

	switch (idRul)
		{

		/* icon bar button--send to general looks processor */
	default:
		DoLooks ((ilcd = IlcdFromIdRul (idRul)), fTrue/*fUpdate*/, 
				(HANDLE) hrsd);
		if (vfRecording)
			RecordLooks(ilcd);
		break;

	case idRulTabLeft:
	case idRulTabCenter:
	case idRulTabRight:
	case idRulTabDecimal:
		val =  idRul - idRulTabLeft + jcLeft;
		(*hrsd)->jcTabCur = val;
		UpdateRulerToggle (hrsd, val, fFalse /*!gray*/, idRul);
		break;
#ifdef DEBUG
	case idRulMode:
	case idRulMark:
	case idRulStyle:
		Assert(fFalse);
#endif /* DEBUG */
		}
}
