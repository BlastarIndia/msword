/* S E L E C T . C */

#ifdef MAC
#define EVENTS
#include "toolbox.h"
#endif

#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "ch.h"
#include "doc.h"
#ifdef MAC
#include "mac.h"
#endif /* MAC */
#include "disp.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "format.h"
#include "inter.h"
#include "field.h"
#include "cmd.h"
#include "debug.h"
#include "screen.h"
#include "error.h"
#include "table.h"

#ifdef WIN
#include "cmdtbl.h"
#define REPEAT
#include "ruler.h"
#include "keys.h"
#include "help.h"

#include "el.h"
#include "macrocmd.h"
#include "rareflag.h"

#define chmNil	kcNil
#endif

#ifdef DEBUG
extern struct DRF	*vpdrfHead;
#endif /* DEBUG */

extern int		vcBtnFldClicks;
extern int              vfSeeSel;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA	caTap;
extern struct CA        caTable;
extern struct TCC       vtcc;

extern CP               vcpFetch;
extern char HUGE        *vhpchFetch;
extern int              vccpFetch;
extern int              fnFetch;
extern BOOL             vfEndFetch;
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern struct TAP       vtapFetch;
extern CP               vmpitccp[];
extern CP				vcpFirstTablePara;

extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vxpFirstMark;
extern int              vxpLimMark;

extern BOOL             vfDoubleClick;
extern BOOL		vfRightClick;

extern char             (**vhgrpchr)[];
extern struct FLI       vfli;
extern struct DOD       **mpdochdod[];
extern int              wwCur;
extern struct WWD       **hwwdCur;
extern struct MWD       **hmwdCur;
extern struct WWD       **mpwwhwwd[];
extern struct PLDR	**vhpldrRetSel;
extern int		vidrRetSel;

extern int              idCursorSave;
extern struct PREF      vpref;

extern int              vssc;   /* secondary selection mode */

extern int              vfExtendSel;
extern int              vfBlockSel;

extern int              vxpFirstMark;
extern int              vxpLimMark;
extern int		vxwCursor;

#ifdef MAC
extern struct EVENT     event;
#define	FSscClick()	(fFalse)
#endif /* MAC */
#ifdef WIN

BOOL    vfTableKeymap = fFalse;
extern  KMP ** hkmpTable;

extern KMP ** vhkmpUser;
extern RRF              rrf;
extern HWND             vhwndRibbon;
extern HWND             vhwndCBT;
extern struct SCI       vsci;
extern BOOL             vfTableKeymap;
extern int		         vcConseqScroll;
extern BOOL             vfHelp;
extern long             vcmsecHelp;
extern HCURSOR          vhcColumn;
extern MES **			vhmes;
extern struct SEL       selMacro;
#endif /* WIN */

extern CP CpFirstBlock();
extern CP CpFromXpVfli();
CP CpCheckEndRight();
CP CpLimCharBlock();

#ifdef WIN
CP CpVisiForStyPara();
#else
#define CpVisiForStyPara(ww, doc, cp)	(cp)
#endif

/* G L O B A L S */

/* true iff the last selection was made by an Up/Down cursor key */
int                     vfLastCursor;
int 			vfLastDrCursor;


/* S E L E C T */
/* change a selection to a new interval.
psel must have at least ww, fNil initialized. if !fNil, other
information must describe actual display highlight.

Plan:
	check if selection is legal, if not, enlarge selection or
	leave selection alone, beep and return.
	if fNil then fHidden = fFalse.
	else clear non skSel markings
	if !fHidden
		make sure display is up to date
		change old sel markings into new sel markings
		(skIns, skSel, or skGraphics)
	store new cp's into psel, store new sk.
*/



#ifdef SLOW  /* replace by a word.h macro */
/* %%Function:Select %%Owner:chic */
Select(psel, cpFirst, cpLim)
struct SEL *psel; 
CP cpFirst, cpLim;
{
	Select1(psel, cpFirst, cpLim, maskSelChanged);
}


#endif

/* S E L E C T  I N S */
/* %%Function:SelectIns %%Owner:chic */
SelectIns(psel, cpFirst)
struct SEL *psel; 
CP cpFirst;
{
	Select1(psel, cpFirst, cpFirst, maskSelChanged);
}


/* S E L E C T  1 */
/* same as above, but fInsEnd may be set */
/* %%Function:Select1 %%Owner:chic */
Select1(psel, cpFirst, cpLim, grpf)
struct SEL *psel;
CP cpFirst, cpLim;
int grpf;
{
	int skNew, skOld;
	CP cpFirstOld;
	CP cpLimOld;
	CP cpFirstTable, cpLimTable;
	int itcFirst, itcLim;
	struct CA caTable;
	int fTableAnchorNew;
	struct CA caT;
	int fWithinCellNew;
	CP cpLimM1;
	int fWithinCellChanges = fFalse, fTableNew;
	struct SELS selsT;
	CP cpImport;  /* if not cpNil after FCaIsGraphics, then sel is an import graphics field */
	Debug(int docSel;)

			Assert(psel != NULL); /* catch callers using old args */

/* notation:    + add highlight
		- remove highlight
		.. leave alone
		00 common portion
*/

	Assert(cpLim <= CpMacDoc(psel->doc) || psel->fHidden);
	Assert(cpFirst <= cpLim);

#ifdef MAC
/* invalidate any dialog bitmap caches on top of wwCur */
	InvalidateDbcWw(psel->ww);
#endif /* MAC */
	if (!(grpf & maskTableExempt) &&
			cpFirst < cpLim && FInTableDocCp(psel->doc, cpLimM1 = cpLim - 1) &&
			FInTableDocCp(psel->doc, cpFirst))
		{
		CacheTc(wwNil, psel->doc, cpFirst, fFalse, fFalse);
		CpFirstTap(psel->doc, cpFirst);
		caTable = caTap;
		cpFirstTable = caTap.cpFirst;
		itcFirst = vtcc.itc;
		if (!FInCa(psel->doc, cpLim, &vtcc.ca) &&
				(FInCa(psel->doc, cpLimM1, &caTable) || FParasUpToCpInTable(&caTable, cpLimM1)))
			{
			CacheTc(wwNil, psel->doc, cpLimM1, fFalse, fFalse);
			CpFirstTap(psel->doc, cpLimM1);
			itcLim = vtcc.itc + 1;
			cpLimTable = caTap.cpLim;
			if (!psel->fTable && psel->cpFirst <= cpFirstTable &&
					cpLimTable <= psel->cpLim)
				SelectRow(psel, cpFirstTable, cpLimTable);
			SelectColumn(psel, cpFirstTable, cpLimTable, itcFirst, itcLim);
			return;
			}
		}

	skNew = cpFirst == cpLim ? skIns : skSel;

	PcaSet(&caT, psel->doc, cpFirst, cpLim);
#ifdef WIN
/*  Assure that the requested selection is valid WRT fields.  That means
	that the selection cannot contain a field deliminator (chFieldBegin or
	chFieldEnd) without containing the matching deliminator.

	This function changes cpFirst and/or cpLim if necessary to make the
	selection legal.  The changes will enlarge the selection, not shrink it.
*/
	if (skNew == skSel)
		{
		AssureLegalSel(&caT);
		cpFirst = caT.cpFirst;
		cpLim = caT.cpLim;
		}
#endif

/* check for making a new graphics selection */


	if (psel == &selCur)
		if (FCaIsGraphics(&caT, psel->ww, &cpImport))
			skNew = skGraphics;



#ifdef MAC
	/* bz - Opus allows overlaps during dyadic operations until the operation
	is actually performed, when we complain and throw out the 2ndary
	selection. This helps the case of page-upping into a large
	selection. Charles said to leave this in for mac to avoid
	potential bugs with highlighting.
	
	Note I don't know of any other situations besides dyadic move
	where this overlap can occur, so the code below is probably
	overcautious, since only selDotted can overlap selCur, but I am
	leaving it as is.
	*/
	/* check for overlaps with selections */
	if ( (psel != &selCur && !selCur.fNil &&
			FOverlapCa(&caT, &selCur.ca)) ||
			(psel != &selDotted && !selDotted.fNil &&
			FOverlapCa(&caT, &selDotted.ca)))
		{
		Beep(); 
		return;
		}
#endif

	cpFirstOld = psel->cpFirst;
	cpLimOld = psel->cpLim;
	skOld = psel->sk;

	if (psel->fNil || !psel->fHidden)
		if (PwwdWw(psel->ww)->fDirty)
			UpdateWw(psel->ww, fFalse);
/* note that UpdateWw can change selCur.doc! Yes! If selCur is in a 
   header in page view but the current page doesn't show the header,
   the preceding UpdateWw forces selCur into the main doc */
	Debug(docSel = psel->doc;)
/* turn off existing selection which is not skSel */
	if (psel->fNil)
		{
		/*psel->fHidden = fFalse;*/
		psel->fOn = fFalse;
		skOld = skGraphics; /* set to be != skSel */
		}
	else  if (!psel->fHidden)
		{
		CachePara(psel->doc, cpFirst);
		fTableAnchorNew = FInTableVPapFetch(psel->doc, cpFirst);
		switch (skOld)
			{
		case skSel:
			if (skNew == skSel)
				{
				if (!psel->fTableAnchor && !fTableAnchorNew)
					break;
				fWithinCellNew = fFalse;
				if (fTableAnchorNew)
					{
					selsT.doc = psel->doc;
					selsT.cpFirst = selsT.cpAnchor = cpFirst;
					selsT.cpLim = cpLim;
					selsT.fForward = fTrue;
					SetSelCellBits(&selsT);
					fWithinCellNew = selsT.fWithinCell;
					}
				if (psel->fWithinCell == fWithinCellNew)
					break;
				}
			fWithinCellChanges = fTrue;
/* when the old sel is within a table sel and the new selection is not, we
	need to turn off the old selection so marking happens correctly. */
			skOld = skColumn; /* i.e. != skSel */
			goto LSelOff;
		case skIns:
/* conditions are: repeated, insert point, ON, at desired end of line */
			if (!(fWithinCellChanges = (psel->fWithinCell || (!psel->fWithinCell &&
					!psel->fTableAnchor && fTableAnchorNew))) && cpFirst == cpFirstOld && 
					skNew == skIns && psel->fOn && psel->fInsEnd == grpf & maskInsEnd)
				{
				psel->fForward = psel->fRightward =  fTrue;
				psel->cpFirstLine = cpNil; /* erase hint for block sel */
				return;
				}
			goto LSelOff;
		case skColumn:
		case skRows:
			fWithinCellChanges = fTrue;
			/* FALL THRU */
		case skBlock:
		case skGraphics:
LSelOff:                
			TurnOffSel(psel);
			psel->fHidden = fFalse;
			psel->fOn = fFalse; /* safety */
			psel->xpFirst = xpNil;
			psel->xpLim = xpNil;
/* when we have a table selection we must determine psel->fWithinCell before
	we call ToggleSel. */
			if (fWithinCellChanges)
				{
				psel->cpFirst = psel->cpAnchor = cpFirst;
				psel->cpLim = cpLim;
				Assert(psel->doc == docSel);
				psel->fForward = fTrue;
				SetSelCellBits(psel);
				}
			}
/* at this point selection marking are off unless psel->sk==skSel */
		}

	psel->fForward = psel->fRightward =  fTrue;
	psel->sk = skNew;
	psel->cpFirstLine = cpNil; /* erase hint for block sel */
	psel->fInsEnd = grpf & maskInsEnd;
/* these will be set below so that xp cache can be still used if skSel */
	/*psel->cpFirst = cpFirst;
	psel->cpLim = cpLim;*/

/* mark the new selection according to skNew */
	if (!psel->fHidden && skNew != skGraphics)
		{
		if (skOld != skSel || skNew == skIns)
			goto LSelOn;
		Assert(skOld == skSel && skNew == skSel);
		if (cpFirst < cpFirstOld)
			{ /* +++... */
			if (cpLim <= cpFirstOld)
				{ /* +++   --- */
				goto LSeparateSels;
				}
			else
				{ /* +++000... */
				ToggleSel(psel, cpFirst, cpFirstOld);
				psel->xpFirst = vxpFirstMark;
				if (cpLim < cpLimOld)
					{ /* +++000--- */
					ToggleSel(psel, cpLim, cpLimOld);
					psel->xpLim = vxpFirstMark;
					}
				else  if (cpLim > cpLimOld)
					{ /* +++000+++ */
					ToggleSel(psel, cpLimOld, cpLim);
					psel->xpLim = vxpLimMark;
					}
				}
			}
		else
			{ /* ---... */
			if (cpLimOld <= cpFirst)
				{ /* --- +++ */
LSeparateSels:                  
				ToggleSel(psel, cpFirstOld, cpLimOld);
LSelOn:                         
				ToggleSel(psel, cpFirst, cpLim);
				psel->xpFirst = vxpFirstMark;
				psel->xpLim = vxpLimMark;
				}
			else
				{ /* ---000... */
				if (cpLimOld < cpLim)
					{ /* ---000+++ */
					ToggleSel(psel, cpLimOld, cpLim);
					psel->xpLim = vxpLimMark;
					}
				else  if (cpLimOld > cpLim)
					{ /* ---000--- */
					ToggleSel(psel, cpLim, cpLimOld);
					psel->xpLim = vxpFirstMark;
					}
				if (cpFirstOld != cpFirst)
					{
					ToggleSel(psel, cpFirstOld, cpFirst);
					psel->xpFirst = vxpLimMark;
					}
				}
			}
		}
	else
		psel->xpFirst = psel->xpLim = xpNil;
#ifdef WINBOGUS
	/* NO! this is weird. Do the setagain in AddRulerGrpprl
		so again is correct even when typing w/o selecting
	*/
	/* Record whether the selection has moved in rrf for repeat
		of ruler/ribbon purpose. */
	if (selCur.doc != rrf.doc ||
			selCur.cpFirst != cpFirst || selCur.cpLim != cpLim)
		{
		rrf.fSelMoved = fTrue;
		rrf.doc = selCur.doc;
		if (rrf.fTouched &&
				((rrf.fRibbon && vhwndRibbon != NULL) ||
				(!rrf.fRibbon && (*hmwdCur)->hwndRuler != NULL)))
			{
			SetAgain(bcmRRFormat);
			}
		}
#endif /* WINBOGUS */

	psel->cpFirst = psel->cpAnchor = cpFirst;
	psel->cpLim = cpLim;
	Assert(psel->doc == docSel);
	psel->sty = (cpFirst == cpLim) ? styNil : styChar;

/* determine values of fTableAnchor and fWithinCell bits in the sel */
	SetSelCellBits(psel);

/* selCur properties maintenance */
	if (psel == &selCur)
		{
		/* make us get new selection properites */
		InvalSelCurProps(grpf & maskSelChanged);
		if (skNew == skIns)
			GetSelCurChp(grpf & maskSelChanged);

		else  if (skNew == skGraphics)
			{
#ifdef WIN
			if (cpImport == cpNil)   /* only if not an import field */
				GetSelCurChp(fTrue);
#endif /* WIN */
#ifdef MAC
			/* set the chp properties and update fnPic */
			FetchCpAndParaCa(&selCur.ca, fcmProps);
			selCur.chp = vchpFetch;
			selCur.chp.fnPic = fnFetch;
			selCur.chp.fSpec = fFalse;
			selCur.fUpdateChp = fFalse;
#endif /* MAC */
			}
		}
	if (!psel->fHidden)
		{
		if (skNew == skGraphics)
			{
			HilitePicSel(psel, 0);
			}
		}
	vfLastCursor = vfLastDrCursor = fFalse;
#ifdef WIN
#ifdef DEBUG
	/* verify that the final selection does not break up a
		CRLF pair. */
/* when inserting in outline view, it's possible to do insertion at CpMacDoc */
	if (psel->fIns && vdbs.fCkText && psel->cpFirst > cp0 &&
		psel->cpFirst < CpMacDoc(psel->doc))
		{
		ChFetch(psel->doc, psel->cpFirst-1, fcmChars);
		Assert(*vhpchFetch != chReturn);
		ChFetch(psel->doc, psel->cpFirst, fcmChars);
		Assert(*vhpchFetch != chEol && *vhpchFetch != chTable);
		}
#endif /* DEBUG */
#endif
}


/* S E T  S E L  C U R	S E L S */
/* makes a sel or block sel in selCur.
used to re-establish a general selection which was saved in a sels. */
/* SetSelCurSels(psels) */
/* This is a macro in word.h that calls SetPselSels */

/* S E T  P S E L  S E L S */
/* makes a sel or block sel in selCur.
used to re-establish a general selection which was saved in a sels. */
/* %%Function:SetPselSels %%Owner:chic */
SetPselSels(psel, psels)
struct SEL *psel;
struct SELS *psels;
{
	PwwdSetWw(psel->ww, cptDoc);
	Assert(DocMother(psels->doc) == PmwdWw(psel->ww)->doc
			|| PdodDoc(PmwdWw(psel->ww)->doc)->fShort); /* header */
	if (psels->doc != psel->doc)
		{
		TurnOffSel(psel);
		psel->sk = skNil;
		psel->doc = psels->doc;
		/* doc has changed, reset cp in case someone misused it */
		psel->cpFirst = psel->cpLim = cp0;
		}
	if (psels->fTable)
		{
		SelectColumn(psel, psels->cpFirst, psels->cpLim, psels->itcFirst, psels->itcLim);
		if (psels->sk == skRows)
			psel->sk = skRows;
		}
	else  if (psels->fBlock)
		{
		TurnOffSel(psel);
		MarkSelsBlock(psel->ww, psels);
		if (psels != psel)
			bltbyte(psels, psel, sizeof(struct SELS));
		psel->fOn = psel->fHidden = fFalse;
		}
	else
		{
		Assert(PwwdWw(psel->ww)->fPageView || psels->doc == psel->doc);
		/* don't clobber ins pt props if sel has not changed */
		Select1(psel, psels->cpFirst, psels->cpLim,
				(psels->fInsEnd ? maskInsEnd : 0) |
				(FNeRgw(&psel->ca, &psels->ca, cwCA) ? maskSelChanged : 0));
		}
	psel->sty = psels->sty;
	psel->cpAnchor = psels->cpAnchor;
	if (!FInTableDocCp(psel->doc, psels->cpAnchor))
		psel->fTableAnchor = fFalse;
	psel->fRightward = psels->fRightward;
	psel->fForward = psels->fForward;
}


/* S E T  S E L  W W */
/* sets the ww and doc fields of sel */
/* %%Function:SetSelWw %%Owner:chic */
SetSelWw(psel, ww)
struct SEL *psel; 
int ww;
{
	psel->ww = ww;
	psel->doc = (ww == wwNil ? docNil : PwwdWw(ww)->sels.doc);
}


/* T U R N  O F F  S E L */
/* Remove sel highlighting from screen */
/* %%Function:TurnOffSel %%Owner:chic */
TurnOffSel(psel)
struct SEL *psel;
{
	if (!psel->fHidden)
		{
		switch (psel->sk)
			{
		case skGraphics:
			HilitePicSel(psel, 0);
			break;
		case skBlock:
			MarkSelsBlock(psel->ww, psel);
			break;
		case skNil:
			return;
		case skIns:
			ClearInsertLine(psel);
			break;
		default:
			ToggleSel(psel, psel->cpFirst, psel->cpLim);
			}
		psel->fHidden = fTrue;
		if (!psel->fTable && !psel->fBlock)
			psel->xpFirst = psel->xpLim = xpNil;
		}
}


#ifdef WIN
/* T U R N  O N  S E L  C U R */
/* Put up sel highlighting on screen */
/* can't be a macro because used in opuscmd */

/* %%Function:TurnOnSelCur %%Owner:chic */
TurnOnSelCur()
{
	TurnOnSel(&selCur);
}


#endif

#ifdef WIN
/* T U R N  O N  S E L  */
/* See TurnOffSel */
/* %%Function:TurnOnSel %%Owner:chic */
TurnOnSel(psel)
struct SEL *psel;
{
	struct SELS sels;

	if (!psel->fNil && psel->fHidden)
		{
#ifdef DEBUG
		/* psel should be in the current window unless it is
			selCur and there is another selection active in another
			window or we are animating a macro
		*/
#ifdef WIN
		if (!(*vhmes)->fAnimate)
#endif
			if (!(psel == &selCur && vssc != sscNil))
				{
				Assert (psel->ww == wwCur);
				}
#endif /* DEBUG */

		blt(psel, &sels, cwSELS);
		psel->sk = skNil;
		psel->fHidden = fFalse;
		SetPselSels(psel, &sels);
		}
}


#endif

#ifdef MAC /* moved to disp1.c for WIN */
/* T O G G L E  S E L */
/* Flip selection highlighting on/off
psel provides pa, ww...
*/
/* %%Function:ToggleSel %%Owner:NOTUSED */
ToggleSel(psel, cpFirst, cpLim)
struct SEL *psel;
CP cpFirst, cpLim; /* bounds */
{

#ifdef MAC
	InvalidateDbcWw(psel->ww);  /* invalidate dialog bit-caches */
#endif

	vxpFirstMark = vxpLimMark = xpNil;
	ToggleSel1(psel, HwwdWw(psel->ww), cpFirst, cpLim);
}


/* T O G G L E  S E L 1 */
/* used to implement the recursive call for nested frames */
/* %%Function:ToggleSel1 %%Owner:NOTUSED */
ToggleSel1(psel, hpldr/*hwwd*/, cpFirst, cpLim)
struct SEL *psel;
struct PLDR **hpldr;
CP cpFirst, cpLim; /* bounds */
{
	struct PLCEDL **hplcedl;
	int idr, idrMac;
	struct DR *pdr;
	int dl, dlMac;
	CP cpT;
	struct RC rc;
	struct DRF drfFetch;

	idrMac = (*hpldr)->idrMac;
	for (idr = 0; idr < idrMac; FreePdrf(&drfFetch), idr++)
		{
		pdr = PdrFetch(hpldr, idr, &drfFetch);
		if (pdr->doc != psel->doc || pdr->hplcedl == hNil)
			continue;
		dlMac = IMacPlc(hplcedl = pdr->hplcedl);
		if (idrMac != 1)
			{
			if (pdr->cpFirst > cpLim ||
					CpPlc(hplcedl, dlMac) < cpFirst) continue;
			}
		if ((cpT = CpPlc(hplcedl, 0)) > cpFirst)
			dl = (cpT > cpLim) ? dlMac : 0;
		else if ((dl = IInPlcCheck(hplcedl, cpFirst)) == -1)
			dl = dlMac;
		if (dl != 0 && psel->fInsEnd && psel->fIns)
			dl--;
		for ( ; dl < dlMac; dl++)
			{
			struct EDL edl;

			if ((cpT = CpPlc(hplcedl, dl)) > cpLim)
				break;
			GetPlc(hplcedl, dl, &edl);
			if (edl.hpldr != hNil)
				ToggleSel1(psel, edl.hpldr, cpFirst, cpLim);
			else  if (edl.fEnd ||
#ifdef MAC
					FMarkLine(psel, hpldr, idr, &edl, cpFirst, cpLim, cpT, fFalse))
#else
				FMarkLine(psel, hpldr, idr, &edl, cpFirst, cpLim, cpT))
#endif
						break;
			}
		}
}


#endif /* MAC */

#ifdef MAC  /* MOVED */
/* C L E A R  I N S E R T  L I N E */
/* %%Function:ClearInsertLine %%Owner:NOTUSED */
ClearInsertLine(psel) /* moved to disp1.c in WIN */
struct SEL *psel;
{
	if (psel->fIns && psel->fOn && !psel->fNil)
		DrawInsertLine(psel);
}


#endif

/* C P  F R O M  D L  X P */
/*	Given the dl and xp, returns the nearest valid cp, also return
/*	the pointer to a run in vhgrpchr via ppchr.
/*	Assumes hpldr is in wwCur!
/*	If pflss is not NULL, save vfli.cpMac, vfli.chBreak, etc. in *pflss
/*	for later use.  Note: CpFromXpVfli() in outline clobbers vfli
/*	by calling CpLimSty().
/**/
/* %%Function:CpFromDlXp %%Owner:chic */
CP CpFromDlXp(ww, hpldr, idr, dl, xp, pflt, pflss)
struct PLDR **hpldr;
int idr;
int dl;
int xp;
int *pflt;
struct FLSS *pflss;
{
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	CP cp;
	struct EDL edl;
	struct DRF drfFetch;
	struct CHR *pchr;

	pdr = PdrFetch(hpldr, idr, &drfFetch);
	if (pdr->doc < 0)
		goto LRet;
	hplcedl = pdr->hplcedl;
	if (IMacPlc(hplcedl) == 0)
		dl = 0;
	cp = CpPlc(hplcedl, dl); /* cpFirstLine */
	Assert(dl < IMacPlc(hplcedl));

	FormatLineDr(ww, cp, pdr);
	if (IMacPlc(hplcedl) == 0) return cp;

	if (pflss != NULL)
		{
		pflss->cpMin = vfli.cpMin;
		pflss->cpMac = vfli.cpMac;
		pflss->fSplatDots = vfli.fSplatDots;
		pflss->chBreak = vfli.chBreak;
		}

	if (vfli.fSplatBreak && !pdr->fInTable &&
			(!PwwdWw(ww)->fPageView || vfli.ichMac == 1))
		/* Selecting in division/page break */
		goto LRet;

	GetPlc(hplcedl, dl, &edl);
	Assert(vfli.xpLeft == edl.xpLeft);

	if (xp >= edl.xpLeft + edl.dxp)
		{
		/* Out of bounds right */
		cp = (vfBlockSel) ? cpNil : vfli.cpMac;
		}
	else  if (xp > edl.xpLeft)
		{
/* we need to scan now to get past hidden text even if we are to the left
	of the edl's xpLeft */
		cp = CpFromXpVfli(xp, pflt, fFalse /* fTabTrail */, 
				WinMac(fTrue, !vpref.fNoItalicCrs));
		}
#ifdef MAC /* too random for WIN, can split CRLF, we do it from CpFirst/LimSty */
	else
		{
		pchr = &(**vhgrpchr)[0];
		if (pchr->chrm == chrmVanish)
			cp += ((struct CHRV *)pchr)->dcp;
		}
#endif
LRet:
	FreePdrf(&drfFetch);
	return cp;
}


/* C P  F R O M  X P  V F L I */
/* IMPORTANT: this routine expects that vfli and caPara are set up */
/* %%Function:CpFromXpVfli %%Owner:chic */
CP CpFromXpVfli(xp, pflt, fTabTrail, fAdjustItalic)
int xp;
int *pflt;
BOOL fTabTrail; /* includes trailing tabs even if < 1/2 in xp  if true */
BOOL fAdjustItalic;     /* Adjust for italics */
{
	CP cp;
	int xpT;
	int *pdxp;
	int chrm;
	int ich, ichNext;
	struct CHR *pchr;
	struct CHP *pchp;

	Assert( vfli.doc != docNil );

/* scan line to find cp to the left or to the right of the character
over xp, depending whichever is closer to the middle point of the char */
	pchr = &(**vhgrpchr)[0];
	xpT = vfli.xpLeft;
	cp = vfli.cpMin;
	pdxp = &vfli.rgdxp[0];

	for (ich = 0;;)
		{
/* scan until beginning of chr is reached */
		ichNext = pchr->ich;

/* next while loop is speed critical section */
		while (ich < ichNext)
			{
/* we want to return the cp left to the vanish run, therefore the >= instead of > */
/* Midpoints of chars are used to determine whether to include them. */
/* if fTabTrail, take trailing tabs even if less than midpoint is picked up */

			if ( (xpT += *pdxp) >= xp)
				{
				uns dxpT = (uns) *pdxp;
				/* for large pictures, click anywhere in middle of pic gives
					entire picture */
				if (dxpT > 20 && xpT - 8 >= xp && (int)(xpT - dxpT + 8) <= (int)xp)
					{
					if (vfli.rgch[ich] == chPicture && pchp->fSpec)
						{
						*pflt = fltGraphics;
						return cp;
						}
					}

				if (pchp->fSpec && vfli.rgch[ich] == chFootnote && xpT > xp)
					*pflt = fltFootnote;
#ifdef WIN
				else  if (pchp->fSpec && vfli.rgch[ich] == chAtn && xpT > xp)
					*pflt = fltAnnotation;
#endif
				else  if (fAdjustItalic && pchp->fItalic && !fTabTrail)
					{
#define dxpCrsBend 4
					if ((int)(xpT - dxpT/2) <= (int)(xp-dxpCrsBend))
						cp++;
					}
				else  if ( ((int)(xpT - dxpT / 2) <= (int)xp) ||
						(fTabTrail && (pchr->chrm == chrmTab)) )
					cp++;
				goto LHaveCp;
				}

			cp++;
			pdxp++;
			ich++;
			}
		if ((chrm = pchr->chrm) == chrmVanish)
			cp += ((struct CHRV *)pchr)->dcp;
		else  if (pchr->chrm == chrmChp)
			pchp = &pchr->chp;

#ifdef WIN
		else  if (chrm == chrmDisplayField)
			{
			ich++;  /* skip chDisplayField */
			pdxp++;
			if ((xpT += ((struct CHRDF *)pchr)->dxp) > xp)
				{
				*pflt = ((struct CHRDF *)pchr)->flt;
				goto LHaveCp;
				}
			cp += ((struct CHRDF *)pchr)->dcp;
			}
		else  if (chrm == chrmFormatGroup)
			/*  skip over a whole block of chrs and ichs */
			{
			if ((xpT += ((struct CHRFG *)pchr)->dxp) > xp)
				{
				*pflt = ((struct CHRFG *)pchr)->flt;
#ifdef DFLDSELECT
				CommSzLong (SzShared("CpFromXpVfli: "), cp);
#endif
				goto LHaveCp;
				}
			cp += ((struct CHRFG *)pchr)->dcp;
			(char *)pchr += ((struct CHRFG *)pchr)->dbchr;
			ich = pchr->ich;
			pdxp = &vfli.rgdxp[ich];
			continue;
			}

		Assert (chrm != chrmFormula); /* handled in FormatGroup */
#endif
#ifdef MAC
		if (chrm == chrmFormula)
			xpT += ((struct CHRF *)pchr)->dxp;
#endif
		if (chrm == chrmEnd) /* can happen with scaled fonts! */
			{
			cp = vfli.cpMac;
			break;
			}
/* note how chrm is cb of the variant chr structure! */
		(char *)pchr += chrm;
		}

LHaveCp:
#ifdef WIN
	/*  assure cp returned is visible (esp wrt CRLF) */
	return (cp == vfli.cpMac ? cp :
			CpFirstSty (vfli.ww, vfli.doc, cp, styChar, fFalse));
#else
	return cp;
#endif
}


/*	F S E L E C T  D L  P T
/*	Move cursor to the nearest valid CP and select unit.
/*	returns false if special mouse action took place.
/*
/**/
/* %%Function:FSelectDlPt %%Owner:chic */
FSelectDlPt(psel, hpldr, idr, dl, pt, sty, fExtend, fMouse)
struct SEL *psel;
struct PLDR **hpldr;
int idr, dl, sty;
struct PT pt;
BOOL fExtend, fMouse;
{
	int xp;
	int flt = fltNil;
	BOOL fInsEnd;
	BOOL fVisibleOnly = fTrue; /* do not select vanished text */
	struct PLCEDL **hplcedl;
	int doc = psel->doc;
	BOOL fMouseAction = fFalse;
	CP cp, cpAnchor, cpFirstLine, cpNextLine;
	struct FLSS flss;
	struct DRF drfFetch;
	struct EDL edl;
	CP cpT;

/* window must be up to date so that old selection can be precisely erased */
	UpdateWw(wwCur, fFalse);
	xp = XpFromXw(hpldr, idr, pt.xp);

	cp = CpFromDlXp(wwCur, hpldr, idr, dl, xp, &flt, &flss);

	hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
	if (psel->sty == styPara)
		{
		GetPlc(hplcedl, dl, &edl);
		cpFirstLine = CpFromDlXp(wwCur, hpldr, idr, dl, edl.xpLeft, &flt, &flss);
		}
	else
		cpFirstLine = CpPlc(hplcedl, dl);
	cpNextLine = CpPlc(hplcedl, dl + 1);

#ifndef JR
	if ((*hwwdCur)->fOutline && !fExtend)
		{
		switch (sty)
			{
		case styLine:
			psel->sty = sty = styPara;
			break;
		case styPara:
/* for outline selections sty encodes the lvl as well */
			psel->sty = sty = styOutline + LvlFromDocCp(doc, cp);
			}
		}
#endif /* JR */

/* check for double-clicking on splats and prop mark */
	if (vfli.fSplats && fMouse && !fExtend && psel == &selCur
			&& !(*hwwdCur)->fOutline)
		{
		if (vfli.fPropMark && vfDoubleClick)
			{
			int yp = YpFromYw(hpldr, idr, pt.yp);
			int ypBase;
			int xpBase;
			struct EDL edl;

			GetPlc(hplcedl, dl, &edl);
			ypBase = edl.ypTop + edl.dyp
					- vfli.dypBase - vfli.dypFont / 4;
			xpBase = min(0, vfli.xpLeft);
/* sensitive area is a little larger than the mark itself so that the I beam
cursor can be also used to click on it when the parais indented. */
			if (yp >= ypBase - 6 && yp <= ypBase + 1 &&
					xp >= xpBase - 7 && xp < xpBase)
				{
				CachePara(vfli.doc, vfli.cpMin);
				FreePdrf(&drfFetch);
				FExecUcm(FAbsPap(vfli.doc, &vpapFetch) ? ucmFormatPosition : 
						ucmParagraphs, chmNil, fFalse);
				return fFalse;
				}
			}
		if (vfli.fSplatBreak && (sty == styPara || sty == styWord) &&
				cp >= cpNextLine - ccpSect)
			{
			CacheSect(vfli.doc, vfli.cpMin);
			if (vfli.cpMac == caSect.cpLim)
				{
				FreePdrf(&drfFetch);
				FExecUcm(ucmSections, chmNil, fFalse);
				return fFalse;
				}
			}
		}

	FreePdrf(&drfFetch);
	/* valid iff fExtend */
	cpAnchor = (psel->fForward ? psel->cpFirst : psel->cpLim);

#ifdef WIN
	/* fMouseAction is either mouse copy or move or copy
	formatting. If sty is other than stychar, we use
	para formatting instead of char for copy looks, for
	copy and move we will insert the text at the start
	if the line/para/doc
	
	Check for selDotted below handles case of right button down
	with ins pt, then drag so called again from within
	DoContentHit.
	
	For Mac, DSscClick is false, so fMouseAction will be false
	(may change if Mac adopts some variant of the quickops) 
	*/

	fMouseAction = fMouse && FSscClick();

	/* caller should have thrown out right clicks that don't
		met these requirements, but there are cases of being
		inside FStillDownReplay and ending back here with an
		invalid state, so check again. bz
	*/

	if (fMouseAction &&  (EidMouseAction() != eidNil))
		goto MouseErrRet;

	if (!fMouseAction)
#endif /* WIN */
		/* NOTE: this is for both Mac and Win */
		{
		if (sty >= styPara)
			/* Selecting a paragraph, line, doc */
			goto LHaveCp;
		}

#ifdef WIN
	/* if mouse move, copy or copy formatting, do that action rather
		than the flt stuff. If stc != stychar, we still set psel
		to an ins point, so be sure the called rtns can handle
		that.
		bz 3/3/88
	*/

	if (fMouseAction)
		{
		struct SEL selT;
		int fMove = !vfShiftKey;

		/* set up psel */
		Assert (psel == &selDotted);
		Assert (psel->fNil);  /* don't set until after overlap check */
		/* psel.doc already set before call to be the doc in the dr */
		psel->ww = wwCur;
		/* keep from moving past or onto end para mark */
		/* extend never true w/ mouseaction; turn off in case
			shift for quickcopy set
		*/
		/* FUTURE: CpCheck.... may not limit the cp if sty
		   > stySent, as styPara is. We use styPara in the mouseaction
		   stuff only as a flag for copylooks for formatting para props;
		   We could probably do something else, like use styWord for a flag;
		   However, the ultimate result is that we should never be past
		   cpMacDocEdit, so adding a limit here	bz 11/12/89
		*/
		cp = CpMin (CpCheckEndRight(cp, cpAnchor, &flss, psel, &sty,
				fFalse /* fExtend */), CpMacDocEdit(psel->doc));
		psel->cpFirst = psel->cpLim = cp;

		/* have to make this check after setting up selDotted
			because an error message out of FDelChk... will cause
			focus loss, and PSelActive will get confused since selCur
			is not in wwCur.
		*/

		/* after this action, selT will be selCur with the endpoint
			modified to exclude the final para mark
		*/
		if (fMove)
			{
			/* if at end of doc, FDeleteCheck... will back up
				the selection to exclude the last para mark.
				We use selT here so we leave selCur intact if we
				bag out.
			*/
			selT = selCur;
			vssc = sscMove; /* for PSelActive's sake */
			if ( !FDelCkBlockPsel(&selT, rpkNil, NULL, fTrue /*fReportErr*/,
					fFalse /* fSimple */))
				{
MouseErrRet:
				/* if we clicked in another pane and NewCurWw left selCur in the
					old pane and we reject the click, set wwCur back to the old pane
				*/
				if (selCur.ww != wwCur)
					NewCurWw(selCur.ww, fFalse /* fDoNotSelect */);

				/* may have turned off sel highlight; this will
				restore it in idle if we bag out. Note: since
				we were using selT, selT, not selCur may have
				been hidden, so restore so we will update properly
				*/
				selCur.fHidden = selT.fHidden;
				vfSeeSel = fTrue;
				vssc = sscNil;
				Assert (psel->fNil);
				return (fFalse);  /* special mouse action */
				}
			else
				selCur = selT;
			}

#ifdef NOTUSED
		/* bz - we allow overlaps during dyadic operations until the operation
			is actually performed, when we complain and throw out the 2ndary
			selection. This helps the case of page-upping into a large
			selection
		*/
		/* cannot move/copy into itself */

		if (FOverlapCa(&selCur.ca, &selDotted.ca))
			{
			Beep();
			goto MouseErrRet;
			}
#endif
		/* have to do this so table props, etc, will be
		set up properly in selDotted */
		SelectIns(psel, cp);


		/* We are going throught the normal dyadic move/copy
			code by setting up selCur and selDotted, and setting the
			vssc. EndSsc will take care or resetting selections, etc
		*/

		if (fMove)
			{
			Assert( !vfShiftKey && vfControlKey );
			vssc = sscMove;
			}
		else  if (!vfRightClick)
			{
			Assert( vfControlKey && vfShiftKey);
			/* reset from selectins so para prop copy will work */

			psel->sty = sty > styChar ? styPara : styChar;
			vssc = sscCopyLooks;
			}
		else
			{
			Assert( vfControlKey && vfShiftKey);
			vssc = sscCopy;
			}

		/* this will invoke CmdDoDyadic and set up repeat, etc
			correctly */
		CmdExecBcmKc(bcmOK, kcNil);
		return (fFalse);  /* special mouse action */
		}
#endif /* WIN */

	switch (flt)
		{
/*  place HIT code or calls to HIT functions for each display field
here. */
	default:
#ifdef WIN
		Assert ( fFalse );
	case fltNil:
#endif
		break;
	case fltAnnotation:
	case fltFootnote:
		if (fMouse && sty == styWord && !fExtend && psel == &selCur)
			{
			CMB cmb;
			Select(psel, cp, cp + 1);
#ifdef MAC
			InitCmb(&cmb, ucmFootnote, 1, hNil);
			cmb.fDialog = fFalse;

			CmdFootnote(&cmb);
#else /* WIN */
			/* Give CBT veto power on this action */
			if (vhwndCBT == NULL ||
					SendMessage(vhwndCBT, WM_CBTSEMEV, smvAnnFNMark, 
					MAKELONG((int)cp, (flt == fltFootnote ? 0 : 1))))
				{
				vrf.fVetoViewRef = fTrue;
				FExecCmd(flt == fltFootnote ? bcmViewFootnote : bcmViewAnnotation);
				vrf.fVetoViewRef = fFalse;
				}
#endif
			return fFalse;
			}
		break;
	case fltGraphics:
		/* select as a picture if it is a 1 char sel
			not already selected as a picture, regardless
			of whether you are extending, or if
			the sel is not the same 1 sel cp range and
			not extending.  If extending and cp's not
			same as sel, continue on - not sel as pic.
		
			EXCEPT if in dyadic move/copy state, always
			continue and do a normal select.
		*/

		if (vssc == sscNil)  /* true if not in dyadic state */
			{
			if (psel->cpFirst == cp && psel->cpLim == cp + 1)
				{
				if (psel->sk != skGraphics)
LSelPic:
					Select(psel, cp, cp + 1);
				return fTrue;
				}
			else  if (!fExtend)
				goto LSelPic;
			}
		break;

#ifdef WIN
	case fltHyperText:
	case fltMacroText:
		Assert(styChar == 1); /* asserts for sty == vcBtnFldClicks */
		Assert(styWord == 2);
		if (sty == vcBtnFldClicks && !fExtend && fMouse)
			{
			Select (psel, cp, cp+1);
			if (flt == fltHyperText)
				DoHyperTextHit (selCur.doc, cp);
			else
				DoMacroTextHit (selCur.doc, cp);
			return fFalse;
			}
		goto LDisplayField;

	case fltMessage:
	case fltFormula:
	case fltSeqLevOut:
	case fltSeqLevNum:
	case fltSeqLevLeg:
	case fltImport:
LDisplayField:
		/*  don't want to call the CpFirst/LimSty's below */
		if (sty == styChar || sty == styWord)
			{
			sty = styNil;
			if ((!fExtend || cp == cpAnchor) && fMouse)
				{
				Select (psel, cp, cp+1);
				return fTrue;
				}
			}

		if (fExtend)
			cp++; /* cp in center of field causes
		whole field to be selected */
		fVisibleOnly = fFalse; /* thinks field is vanished */
		break;
#endif	/* WIN */
		} /* end of switch(flt) */


/* check for special case: insert point will not be placed to the right of
	an end of paragraph */
	cp = CpCheckEndRight(cp, cpAnchor, &flss, psel, &sty, fExtend);

LHaveCp:
	if (psel->fNil /*&& psel->fHidden*/) psel->fHidden = fFalse;
/* fix having cpMacDoc if last line or select next line if line too short 
and off screen */
	Assert(!fMouseAction);
	if (sty == styLine || sty == styPara)
		cp = cpFirstLine;

	/* Now we have cp, sty, fExtend, cpAnchor, dl
	and can actually make the selection */
	fInsEnd = fFalse;
	if (!fExtend || (sty == styChar && cp == cpAnchor))
		{
		CP cpLim;
		if (sty < styPara)
			{
			if (cp == cpNextLine)
				fInsEnd = fTrue;
#ifdef WIN
			/* Outline mode, beyond right margin
				and in the body text. */
			if (flss.fSplatDots && cp == flss.cpMac)
				fInsEnd = fTrue;
#endif /* WIN */
			}
		cp = CpMin(cp, CpMacDoc(psel->doc));
		if (sty == styChar) /* insertion point */
			{
			CP (*pfnCp)();
			pfnCp = fInsEnd ? CpLimSty : CpFirstSty;
			cp = cpLim = (*pfnCp)(wwCur, psel->doc, cp, styChar, fInsEnd);
			sty = styNil;
			}
		else
			{
			cpLim = CpLimSty(wwCur, psel->doc, 
					(sty == styPara ? CpVisiForStyPara(wwCur, psel->doc, cp) :
					cp), sty, fInsEnd);
			cp = CpFirstSty(wwCur, psel->doc, cp, sty, fInsEnd);
			}

/* limit word select on end of table cells (see similar limitations
in ChangeSel) */
		if ((sty == styWord || sty == stySent) && psel->fTableAnchor)
			{
			if (FInTableDocCp(doc, cp))
				{
				CacheTc(wwNil, doc, cp, fFalse, fFalse);
				if (cpLim == vtcc.ca.cpLim)
					{
					cpLim -= ccpEop;
					cp = cpLim = CpFirstSty(wwCur, psel->doc, cpLim, styChar, fFalse);
					}
				}
			}
		cp = CpMin(cp, CpMacDocEdit(psel->doc));
		Select1(psel, cp, cpLim, (fInsEnd ? maskInsEnd : 0) | maskSelChanged);
		psel->sty = sty;
#ifdef WIN
		if (!(*hwwdCur)->fPageView)
			SeeSel();
#endif
		}
	else
		{
		if ((sty == styWord || sty == stySent) && psel->fTableAnchor && cp > psel->cpAnchor)
			{
			if (FInTableDocCp(doc, psel->cpAnchor))
				{
				CacheTc(wwNil, doc, psel->cpAnchor, fFalse, fFalse);
				if (cp == vtcc.ca.cpLim)
					cp -= ccpEop;
				}
			}

/* cpT is used to account for visiblity of styPara */
		cpT = (sty == styPara ? CpVisiForStyPara(wwCur, psel->doc, cp) : cp);
		cp = (cp >= psel->cpAnchor) ?
				CpLimSty(wwCur, psel->doc, cpT, sty,
				sty < styPara || (sty >= styCol && sty <= styWholeTable)) :
				CpFirstSty(wwCur, psel->doc, cp, sty, cp >= CpMacDoc(psel->doc));
/* need to ensure that we don't collapse sel to an insertion pt beyond 
	CpMacDocEdit. */
		if (psel->cpAnchor > CpMacDocEdit(psel->doc) &&
				cp > CpMacDocEdit(psel->doc))
			cp = CpMacDocEdit(psel->doc);
		ChangeSel(psel, cp,
				(sty == styChar ? styNil : sty), fVisibleOnly, fMouse);
		}
	selCur.cpFirstLine = cpFirstLine;       /* clobbered by Select1 */
	return fTrue;

} /* end of FSelectDlPt */


/* C P  C H E C K  E N D  R I G H T */
/* %%Function:CpCheckEndRight %%Owner:chic */
CP CpCheckEndRight(cp, cpAnchor, pflss, psel, psty, fExtend)
CP cp;
CP cpAnchor;
struct FLSS *pflss;
struct SEL *psel;
int *psty;
BOOL fExtend;
{

	/* check for special case: insert point will not be placed to the right of
		an end of paragraph */
	int chBreak = pflss->chBreak;
	if (cp == pflss->cpMac && (chBreak == chEop ||
			((*hwwdCur)->fPageView && chBreak == chSect) || chBreak == chTable
			|| chBreak == chCRJ) &&
			(psel->fSelAtPara || (*psty <= stySent &&
			!fExtend || cp < cpAnchor/* backward extension */)))
		/* Return insert point before paragraph mark */
		{
		psel->fSelAtPara = fTrue;
		if (!fExtend || cpAnchor >= pflss->cpMin)
			{
#ifdef WIN
			if (chBreak == chSect)
				*psty = styNil;

			cp = CpFirstSty( wwCur, psel->doc, pflss->cpMac,
					styChar, fTrue );
#else
			int ccp;
			if (chBreak == chSect)
				{
				ccp = ccpSect;
				*psty = styNil;
				}
			else  if (chBreak == chCRJ)
				ccp = ccpCRJ;
			else
				ccp = ccpEop;

			cp = CpLimSty( wwCur, psel->doc, pflss->cpMac - ccp,
					styChar, fTrue );
#endif
			}
		}
	return (cp);
}



/* O U T L I N E  S E L  C H E C K */
/* %%Function:OutlineSelCheck %%Owner:chic */
OutlineSelCheck(psel)
struct SEL *psel;
{
#ifndef JR
	CP cpLimM1, cpLimTable, cpFirstTable;
	struct CA caTable;
	BOOL fForwardSave;
	if (PwwdWw(psel->ww)->fOutline && (!fWin ||
			psel->sty <= styPara || psel->sty == styLine))
		{
/* check proposed extension reaching para bounds in outline windows:
extend to para bounds */
		struct CA ca;
/* ca is proposed selection */
		CachePara(psel->doc, psel->cpFirst);
		if (psel->cpLim >= caPara.cpLim)
			{
			ca = psel->ca;
	/* expand selection to integral paragraphs */
			FUpdateHplcpad(ca.doc);
			fForwardSave = psel->fForward;
			ExpandOutlineCa(&ca, fFalse);
			if (FInTableDocCp(ca.doc, cpLimM1 = ca.cpLim - 1) &&
					FInTableDocCp(ca.doc, ca.cpFirst))
				{
				CacheTc(wwNil, ca.doc, ca.cpFirst, fFalse, fFalse);
				caTable = caTap;
				cpFirstTable = caTap.cpFirst;
				if (!FInCa(ca.doc, cpLimM1, &vtcc.ca) &&
						(FInCa(ca.doc, cpLimM1, &caTable) || FParasUpToCpInTable(&caTable, cpLimM1)))
					{
					CpFirstTap(ca.doc, cpLimM1);
					cpLimTable = caTap.cpLim;
					SelectRow(psel, cpFirstTable, cpLimTable);
					return;
					}
				}
			Select(psel, ca.cpFirst, ca.cpLim);
			psel->cpAnchor = fForwardSave ? ca.cpFirst : ca.cpLim;
			psel->fForward = fForwardSave;
			psel->sty = styPara;
			SetSelCellBits(psel);
			}
		}
#endif /* JR */
}


/* C H A N G E  S E L */
/* make psel expand or contract to cp in quanta of sty.
*/
/* %%Function:ChangeSel %%Owner:chic */
ChangeSel(psel, cp, sty, fVisibleOnly, fMouse)
struct SEL *psel; 
CP cp; 
int sty; 
BOOL fVisibleOnly;
{
	int ww = psel->ww;
	int fForward;
	int doc;
	int styT;
	int fTableAnchor;
	int itc, itcAnchor;
	int fExtendToTap;
	int fCpInAnchorCell;
	CP cpFirst, cpLim, cpAnchor, cpNew;
	CP cpFirstIfChanged, cpLimIfChanged;
	struct CA caTapCp, caTapAnchor, caInTable;
	CP cpT;

	/* nothing to do, so return (this is to fix bug #7493) */
	if (selCur.fIns && cp == psel->cpFirst)
		return;

	doc = psel->doc;
	cpFirst = psel->cpFirst;
	cpLim = psel->cpLim;
	cpAnchor = psel->cpAnchor;
	fForward = psel->fForward;
	fTableAnchor = psel->fTableAnchor;
/* initially assume that if new cp is in table, we will have to extend
	selection to either the cpFirst or cpLim of the enclosing tap */
	CachePara(doc, cpNew = ((cp <= cpAnchor) ? cp : cp - 1));
	fExtendToTap = FInTableVPapFetch(doc, cpNew);
/* if the anchor of the selection is within a table, we may have to alter
	the selection before we enter the normal code for changing the selection.*/
	if (fTableAnchor)
		{
		CpFirstTap(doc, cpAnchor);
		caInTable = caTap;
		caTapAnchor = caTap;
		CacheTc(wwNil, doc, cpAnchor, fFalse, fFalse);
		itcAnchor = vtcc.itc;
		if (fMouse && cp == vtcc.ca.cpLim)
			cp -= ccpEop;
/* if the previous selection was anchored in a table and extended out of
	a cell and the new selection will be entirely contained in the cell,
	we need to shrink one side of the selection so it is within the anchor
	cell */
		if ((fCpInAnchorCell = FInCa(doc, cp, &vtcc.ca)) && !psel->fWithinCell)
			{
			TurnOffSel(psel);
			cpFirst = CpMin(cpAnchor, cp);
			cpLim = CpMax(cpAnchor, cp);
			SelectIns(psel, cpFirst = CpFirstSty(ww, doc, cpFirst, sty, fFalse));
			psel->fHidden = fFalse;
			cpLim = CpMin(CpLimSty(ww, doc, cpLim, sty, fFalse), vtcc.ca.cpLim - ccpEop);
			Select(psel, cpFirst, cpLim);
			psel->cpAnchor = cpAnchor;
			psel->fTableAnchor = fTrue;
			psel->fForward = (cp >= cpAnchor);
			psel->sty = sty;
			return;
			}
/* if we've selected out of the anchor cell, we're using the cursor keys and
	the new cp is still within the table, convert the selection to a table
	column select and return */
		else  if (!fCpInAnchorCell && !fMouse && 
				FParasUpToCpInTable(&caInTable, cpNew))
			{
			if (psel->sty < styCol && psel->sty > styWholeTable)
				TurnOffSel(psel);
			CacheTc(wwNil, doc, cpNew, fFalse, fFalse);
			itc = vtcc.itc;
			ColumnSelBegin(psel, cpNew, itc, &caTapAnchor, itcAnchor, cpAnchor);
			return;
			}
/* if the resulting selection would not be entirely within a cell and
	the previous selection is entirely within a cell we need to expand
	selection to the containing row. if the resulting selection is anchored
	in a table and would not be entirely within a cell and the direction is
	changed from the previous selection, we must shrink the selection down
	to the enclosing row. */
else  if (!fCpInAnchorCell &&
(psel->fWithinCell ||
psel->fForward != (cp >= cpAnchor)))
{
	styT = psel->sty;
	if (psel->fWithinCell)
		{
		SelectRow(psel, caTap.cpFirst, caTap.cpLim);
		psel->itcOppositeAnchor = itcAnchor;
		}
	else
		{
		if (styT == styColAll || styT == styWholeTable)
			{
			CacheTable(psel->doc, cpAnchor);
			cpFirst = caTable.cpFirst;
			cpLim = caTable.cpLim;
			}
		else
			{
			cpFirst = caTap.cpFirst;
			cpLim = caTap.cpLim;
			}
		Select(psel, cpFirst, cpLim);
		}
	psel->sk = skSel;
	fForward = psel->fForward = (cp >= cpAnchor);
	cpFirst = psel->cpFirst;
	cpLim = psel->cpLim;
/* cpAnchor was saved to undo changes made by Select */
	psel->cpAnchor = cpAnchor;
	psel->sty = styT;
	psel->fTableAnchor = fTableAnchor;
}
else  if (fCpInAnchorCell)
/* if new point is in same cell of a table as cpAnchor we don't want to
	extend to tap boundaries. */
fExtendToTap = fFalse;
		}

/* Make selCur move, expand or contract to cp.
sty is unit to keep in case of movement or flipped selection */

	Assert(cp >= cp0 && cp <= CpMacDoc(doc));

/* we set up both contingencies because it's less work than testing which case
	we actually have. */
	cpFirstIfChanged = cpLimIfChanged = cp;
	if (fExtendToTap)
		{
		CpFirstTap(doc, cpNew);
		caTapCp = caTap;
		cpFirstIfChanged = caTap.cpFirst;
		cpLimIfChanged = caTap.cpLim;
		}
	if (cp <= cpFirst)
		{ /* Extend backwards */
/* checsk is not for speed but to avoid NormCp when
sty == styLine */
		if (fForward)
			{ /* Selection flipped */
/* cpT is used to account for visibility of styPara */
			cpT = (psel->sty == styPara ? CpVisiForStyPara(ww, doc, cpFirst) : cpFirst);
			cpAnchor = cpLim = CpMin (cpLim,
					styLine == psel->sty  || 
					((*hwwdCur)->fOutline && psel->sty > stySent) ? cpLim :
					CpLimSty (ww, doc, cpT, sty,
					fFalse));

/* we don't extend to tap boundaries if sty would take us out of table */
			cpFirst = (fExtendToTap && sty != styDoc &&
					sty != stySection) ?  caTap.cpFirst :
					CpFirstSty (ww, doc, cp, sty, fFalse);
			}
		else
			cpFirst = cpFirstIfChanged;
		fForward = fFalse;
		}
	else  if (cp >= cpLim)
		{ /* Extend forwards */
		if (!fForward && cpLim > cp0)
			{ /* Selection flipped */
			cpAnchor = cpFirst = CpMax (cpFirst,
					styLine == psel->sty ||
					((*hwwdCur)->fOutline && psel->sty > stySent) ? cpFirst :
					CpFirstSty (ww, doc,
					sty > styChar ? cpLim - 1 : cpLim,
					sty, fTrue));

/* we don't extend to tap boundaries if sty would take us out of table */
			cpLim = (fExtendToTap && sty != styDoc &&
					sty != stySection) ?  caTapCp.cpLim :
					((psel->sty == styLine) ? cpLimIfChanged :
					CpLimSty(ww, doc, cp, sty, sty < styPara));
			}
		else
			cpLim = cpLimIfChanged;
		fForward = fTrue;
		}
	else  if (fForward)
		/* Shrink a forward selection */
		cpLim = cpLimIfChanged;
	else
		{ /* Shrink a backward selection */
		cpFirst = cpFirstIfChanged;
/* check is here to avoid CpFirstSty normalizing to the end of the selection
in order to determine when the selection should be turned around.
*/
		if (sty != styNil && cpFirst >= (sty == styLine ? cpLim :
				CpFirstSty(ww, doc, cpLim - 1, sty, fTrue)))
			fForward = fTrue;
		}

#ifdef WIN
	if (cpFirst != cpLim && fVisibleOnly)
		/*  make extended selection include only what it seems */
		{
		Assert (cpFirst < cpLim);
		/* new cpLim <= old cpLim, cpFirst >= old cpFirst */
		cpLim = CpLimSty (ww, doc, cpLim - 1, styChar, fFalse);
		cpFirst = CpFirstSty (ww, doc, cpFirst, styChar, fFalse);
		if (cpLim < cpFirst)
			cpLim = cpFirst;
		}
#endif
	cpAnchor = CpMin(cpLim, CpMax(cpFirst, cpAnchor));
	cpFirst = CpMin(cpFirst, CpMacDocEdit(psel->doc));

	if (cpFirst != psel->cpFirst || cpLim != psel->cpLim ||
			(fTableAnchor && !fCpInAnchorCell))
		{
		Select(psel, cpFirst, cpLim);
		if (psel->fTable)
			fTableAnchor = fTrue; 
		}

	if (psel->fTable && !FInTableDocCp(psel->doc, cpAnchor))
		{
		Assert(!fForward);
		cpAnchor -= ccpEop; 
		Assert(FInTableDocCp(psel->doc, cpAnchor)); 
		}
	psel->cpAnchor = cpAnchor;
	psel->fForward = fForward;
	psel->fTableAnchor = fTableAnchor;
	psel->sty = sty;
}


#ifdef NOTUSED
/* D O C  A C T I V E  */
/* returns selCur.doc or selDotted.doc depending on ssc state. */
/* %%Function:DocActive %%Owner:NOTUSED */
DocActive()
{
	return ((PselActive())->doc);
}


#endif

/* D O  C O N T E N T  H I T */
/* on mouse down, begin tracking selection within wwCur's content region
paying attention to modifier keys */
/* %%Function:DoContentHit %%Owner:chic */
DoContentHit(pt, fShift, fOption, fCommand)
struct PT pt;
{
	struct PLDR **hpldr;
	int idr;
	int dl;
	int fPageView;
	int fSelBar;
	int fDocEq = fFalse;
	int docDr;
	int fExtend;
	int fTableAnchor, fTableSel;
	struct WWD *pwwd;
	int xp;
	int flt, ipgd;
	int itcAnchor;
	struct SEL *psel;
	int xwPtOrig;
	CP cpFirst, cpLim;
	struct CA caCell, caInTable, caTapAnchor, caT;
	struct FLSS flss;
	struct SPT spt;
	struct PT ptT;
	struct DR *pdr;
	int dysMinScroll, dysMaxScroll;
#ifdef 	WIN
	int dyw, dlMax;
	BOOL fForward = fFalse;

	/* hack to fool FStillDownReplay into checking for right button when
			we want it to behave like the left button */
	BOOL fRightIsLeft = fFalse;
#endif
	struct DRF drfFetch;
	int iPlc;

#ifdef WIN
	fExtend = !FSscClick() && (fShift || vfExtendSel);
#else
	fExtend = fShift;
#endif /* WIN */

#ifdef RSH
	SetUatMode(uamNavigation);
	StopUatTimer();
#endif /* RSH */

	/* if in dyadic move, use selDotted, ELSE if right button
		clicked (for mouse copy/move/copy format) and there
		is a selection in selCur use selDotted,
		else use selCur. Note that in dyadic move mode or if
		selCur.fIns we cancel on a right click.
	
		Mac note: vfRightClick/FSscClick() will always be false, so
		this is equivalent to setting psel= PSelActive()
	*/

#ifdef WIN
		{
		int eid;

		/* invalid to do right click (block sel) in dyadic mode or to
			do dyadic mouse ops when in dyadic mode or with a block sel */

		if ((eid = EidMouseAction()) != eidNil)
			{
			ErrorEid(eid, "DoContentHit");
			goto LRetClick;
			}
		}
#endif
	/* note: for Opus mouse ops, vssc will be sscMouse, so
		selDotted will be used */

	psel = ((vssc != sscNil) || FSscClick()) ?
			&selDotted : &selCur;

	if ((*hwwdCur)->fOutline && (FOutlineEmpty(wwCur, fTrue) || 
			(psel == &selCur &&
			FOutlineSelect(pt, fExtend, fOption, fCommand))))
		{
LRetClick:
		/* if we clicked in another pane and NewCurWw left selCur in the
				old pane and we reject the click, set wwCur back to the old pane
		*/

#ifdef WIN
		if (vssc == sscMouse)
			{
			vssc = sscNil;
			selDotted.sk = skNil;
			}
#endif /* WIN */

		if (FSscClick())
			if (selCur.ww != wwCur)
				NewCurWw(selCur.ww, fFalse /* fDoNotSelect */);
		return;
		}
	fPageView = (*hwwdCur)->fPageView;
	if (!psel->fNil && psel->fHidden)
		{ /* Turn on selection highlight */
		psel->fNil = fTrue;
		Select(psel, psel->cpFirst, psel->cpLim);
		}

/* don't allow selection to start in ruler area */
/* or in split bar line */
	ptT = pt;
	if ((dl = DlWherePt(wwCur, &ptT, &hpldr, &idr, &spt, fTrue, fFalse)) == dlNil)
		{
		/* hit outside any DR */
		if (psel->fNil && psel == &selCur) /* just return if selDotted */
			{
			struct SELS *psels = &(*hwwdCur)->sels;
			if (psel->doc == psels->doc)
				Select1(psel, psels->cpFirst, psels->cpLim,
						(psels->fInsEnd ? maskInsEnd : 0) | maskSelChanged);
			else
				Select1(psel, cp0, cp0, maskSelChanged);
			}
		goto LRetClick;
		}
	Assert((*hpldr)->hpldrBack == hNil || (*hpldr)->hpldrBack == hwwdCur);
	dlMax = IMacPlc(PdrFetch(hwwdCur, (*hpldr)->hpldrBack ? (*hpldr)->idrBack : idr, &drfFetch)->hplcedl);
	FreePdrf(&drfFetch);
	/*  page view: check double-click and bring up document dlg */
	if (fPageView && vfDoubleClick && !spt.fInDr && !FSscClick())
		{
		struct RC rcw;
		GetMarginsRcw(&rcw, wwCur);
		if ((pt.xw < rcw.xwLeft || pt.xw > rcw.xwRight) &&
				(pt.yw < rcw.ywTop || pt.yw > rcw.ywBottom))
			{
			FExecUcm(ucmDocument, chmNil, fFalse);
			return;
			}
		}
	if (idr == idrNil)
		return;
	fSelBar = spt.fSelBar;
	pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
	fDocEq = ((docDr = pdr->doc) == psel->doc);
	Assert(fPageView || fDocEq || psel->fNil);
	Assert(!(fPageView && spt.fInStyArea));
/* when we are in page view, mousing into a different doc than the current
	selection and not extending the selection, we will turn off the current
	selection and change psel->doc. */
	if (!fDocEq && !fExtend && docDr != docNil)
		{
		TurnOffSel(psel);
		psel->sk = skNil;
		psel->doc = docDr;
		}
	if (pt.yp < (*hwwdCur)->ywMin || pt.yp >= (*hwwdCur)->ywMac)
		goto LRetClick;

	iPlc = IMacPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl);
	FreePdrf(&drfFetch);
	if (iPlc == 0)
		goto LRetClick;

	vfLastCursor = vfLastDrCursor = fFalse;

	/**********************************************************/
	/**** ASSERT: no heap pointer held in rest of routine *****/
	/**********************************************************/

	if (psel->fGraphics && FContentHitPic(psel, &pt, fExtend, fOption, fCommand))
		return; /* picture click handled */

	psel->fSelAtPara = fFalse;

	if ((WinMac(!FSscClick() && vfRightClick, fOption) || psel->fBlock) && 
			fExtend && !psel->fTableAnchor)
		{
		if (psel->sk == skNil)
			{
			AssertDo(FSelectDlPt(psel, hpldr, idr, dl, ptT,
					styChar, fFalse, fTrue/*fMouse*/));
			}
		if (psel->sk != skSel)
			{
			if (!FInTableDocCp(psel->doc, psel->cpFirst))
				{
				Assert(vpdrfHead == NULL);
				DoContentHitBlock(psel, pt);
				return;
				}
			BlockModeEnd(); 
			}
		WinMac( vfRightClick, fOption ) = fFalse;
		}

#ifdef WIN
	if (vfBlockSel && !psel->fTableAnchor)
		{
		if (psel->sk == skNil)
			{
/* need to mask vfBlockSel to prevent CpFromDlXp call in FSelectDlPt from
	returning cpNil when selected to right of line */
			vfBlockSel = fFalse;
			AssertDo(FSelectDlPt(psel, hpldr, idr, dl, ptT,
					styChar, fFalse, fTrue/*fMouse*/));
			vfBlockSel = fTrue;
			psel->cpFirstLine = cpNil;
			}
		if (!FInTableDocCp(psel->doc, psel->cpFirst))
			{
			Assert(!FSscClick());
			Assert(vpdrfHead == NULL);
			DoContentHitBlock(psel, pt);
			return;
			}
		BlockModeEnd(); 
		}
#endif /* WIN */

	if (fDocEq && psel->fBlock && !psel->fTable)
/* clear block selection, if any */
		TurnOffBlock(psel);
/* if whole column select, select the requested column and return */
	cpFirst = PdrFetchAndFree(hpldr, idr, &drfFetch)->cpFirst;
	fTableAnchor = fFalse;

#ifdef WIN
	if (spt.fInTable && !FSscClick() && vfRightClick)
		{
		SetCursor(vhcColumn);
		goto LColSelect;
		}
#endif /* WIN */

	if (spt.fInTable && fOption && !fExtend)
		{
		if (vfDoubleClick)
			{
			CacheTable(psel->doc, cpFirst);
			SelectRow(psel, caTable.cpFirst, caTable.cpLim);
			psel->sty = styWholeTable;
			return;
			}
		goto LColSelect;
		}
	if (spt.fWholeColumn && !fExtend && !FSscClick())
		{
LColSelect:
		CacheTable(psel->doc, cpFirst);
		SelectColumn(psel, caTable.cpFirst, caTable.cpLim,
				idr, idr + 1);
		psel->sty = styColAll;
		goto LInTable;
		}
	if (spt.fInTable && spt.fSelBar && !fExtend && !fCommand)
		{
		Assert (!FSscClick());
		cpFirst = CpFirstTap(psel->doc, cpFirst);
		if (!vfDoubleClick)
			{
			SelectColumn(psel, caTap.cpFirst, caTap.cpLim,
					idr, idr + 1);
			psel->sty = styCol;
			}
		else
			SelectRow(psel, caTap.cpFirst, caTap.cpLim);
		goto LInTable;
		}

#ifdef WIN
	if (spt.fInTable && vfBlockSel)
		goto LInTable;
#endif /* WIN */

	/* if Ssc command clicking, sty is either char or para. No preserve
		sty, no special command or shift options bz. Again,
		FSscClick() is false for Mac    */
	if (FSscClick())
		{
		psel->sty = !fSelBar ? styChar : styPara;
		}
	else  if (!fExtend)
/* preserve sty if extending */
		{
		Break1();
		psel->sty = (WinMac(vfRightClick, fOption)) ? styNil :
				(!fSelBar ? (fCommand ? stySent : (vfDoubleClick ? styWord : styChar))
: 
				fCommand ? styDoc : (vfDoubleClick || spt.fInStyArea ? styPara : styLine) );
		}
	else  if (fDocEq)
		{
		cpFirst = PdrFetchAndFree(hpldr, idr, &drfFetch)->cpFirst;
LInTable:
		if (fTableAnchor = FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor))
			{
			caInTable = (psel->fTable) ? psel->ca : caCell;
			if (FChangeSelToStyCol(psel, &cpFirst, pt.xw, 
					hpldr, &idr, dl, &caCell, &caInTable, spt.fInTable))
				{
				if (FDoContentHitColumn(psel, cpFirst,	idr, &caInTable, &dl,
						&ptT, &hpldr, &idr, &spt, &caTapAnchor, itcAnchor, caCell.cpFirst))
					return;
				}
			}
		if (psel->fTable)
			{
			MakeSelNonTable(psel, CpPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl,dl));
			FreePdrf(&drfFetch);
			fExtend = fTrue;
			if (vfRightClick)
				{
				/* from here on pretend left button is down */
				vfRightClick = fFalse;
				fRightIsLeft = fTrue;
				}
			}
		}

	if (!fExtend || fDocEq)
		{
		int fNil = psel->fNil;
		psel->doc = PdrFetchAndFree(hpldr, idr, &drfFetch)->doc;
		if (!FSelectDlPt(psel, hpldr, idr, dl, ptT, psel->sty,
				fExtend, fTrue/*fMouse*/))
			/* special action was taken care of */
			return;
#ifdef MAC
/* when we start the selDotted selection for dyadic operations, we want to
	update the ruler and style area in llc. */
		if (fNil && vssc != sscNil)
			{
			if (FRulerUp(wwCur))
				UpdateRuler();
			DrawLlcStyle();
			}
#endif /* MAC */
		}

	if (WinMac( vfRightClick, fOption) && !psel->fTableAnchor)
		{
		/* Win: shouldn't get here if mouse dyadic op  bz */
		Assert (!FSscClick());
		DoContentHitBlock(psel, pt);
		return;
		}

/* record whether selection is anchored in a table */
	if (fTableAnchor = FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor))
		{
		caInTable = caCell;
/* need to know if selection has grown to encompass cell's cell mark. If so
	must convert selection into a table selection */
		if (!psel->fWithinCell && (psel->sty < styCol || psel->sty > styWholeTable) &&
				psel->sty != styDoc && psel->sty != stySection &&
				FInCa(psel->doc, psel->cpFirst, &caTapAnchor) &&
				FInCa(psel->doc, CpMax(psel->cpLim - 1, psel->cpFirst), &caTapAnchor))
			{
			CacheTc(wwNil, psel->doc, psel->cpAnchor, fFalse, fFalse);
			idr = vtcc.itc;
			cpFirst = CpFirstForItc(psel->doc, psel->cpFirst, idr);
			goto LMakeTableSel;
			}
		}

	/* Note: At this point hpldr may have been destroyed by a previous
	call to FSelectDlPt.  We should not use hpldr until it is recomputed
	below in DlWherePt. */
#ifdef WIN
	SetCapture((*hwwdCur)->hwnd);
	vcConseqScroll = 0;
	pwwd = *hwwdCur;
	dyw = pwwd->ywMac - pwwd->ywMin;
#else
	dysMinScroll = dysMinAveLineSci;
	dysMaxScroll = dysMacAveLineSci;
#endif
	xwPtOrig = pt.xw;
	while (FStillDownReplay(&pt, fRightIsLeft) && !psel->fNil/*??? HOW COULD IT BE NIL*/)
		{
		pwwd = *hwwdCur;	/* used only in if clauses */
#ifdef MAC
		if (pt.yw >= pwwd->ywMac-1)
#else
/* -2 to allow mouse drag scroll even when doublezoomed and no horz scroll bar */
			if (pt.yw >= pwwd->ywMac-(dysMinAveLineSci/2))
#endif
				{
				pwwd = *hwwdCur;
				if (fPageView &&
						pwwd->ywMin + pwwd->rcePage.yeBottom + dypPastPageSci <= pwwd->ywMac)
					{
					if ((ipgd = IpgdNextWw(wwCur, fTrue)) == ipgdNil)
						continue;
					SetPageDisp(wwCur, ipgd, YeTopPage(wwCur), fFalse, fFalse);
					Mac( SetCrs(crsIbeam) );
					}
#ifdef 	WIN
				if (!fForward)
					{
					vcConseqScroll = 0;
					}
				fForward = fTrue;
				ScrollDelta(1, vcConseqScroll + 1, dlMax,
						dyw, &dysMinScroll, &dysMaxScroll);
#endif
				if (fPageView || !FEndVisible(wwCur))
					ScrollUp(wwCur, dysMinScroll, dysMaxScroll);
				goto DoCont1;
				}
			else  if (pt.yw < pwwd->ywMin)
				{
				pwwd = *hwwdCur;
				if (fPageView &&  pwwd->rcePage.yeTop - dypPastPageSci >= 0)
					{
					if ((ipgd = IpgdPrevWw(wwCur)) == ipgdNil)
						continue;
					SetPageDisp(wwCur, ipgd, YeBottomPage(wwCur), fFalse, fFalse);
					Mac( SetCrs(crsIbeam) );
					}
#ifdef WIN
				if (fForward)
					{
					vcConseqScroll = 0;
					}
				fForward = fFalse;
				ScrollDelta(1, vcConseqScroll + 1, dlMax,
						dyw, &dysMinScroll, &dysMaxScroll);
#endif
				ScrollDown(wwCur, dysMinScroll, dysMaxScroll);

DoCont1:                
				UpdateWw(wwCur, fFalse);
#ifdef WIN
				if (vcConseqScroll < 0x7FFF)
					{
					vcConseqScroll++;
					}
#endif
				}
			else  if (pt.xw < pwwd->xwMin &&
		/* last clause is to prevent very quick double-clicks from being interpreted
		as draws */
			FAnyDlNotInXw(wwCur, fTrue) && xwPtOrig >= pwwd->xwMin)
				{
				ScrollRight(wwCur, dxpMinScrollSci);
				pt.xp = (*hwwdCur)->xwMin;
				goto DoCont2;
				}
			else  if (pt.xw >= pwwd->xwMac &&
					FAnyDlNotInXw(wwCur, fFalse))
				{
				ScrollLeft(wwCur, dxpMinScrollSci);
				pt.xp = (*hwwdCur)->xwMac - 1;
DoCont2:                
				;
#ifdef MAC
				if (GetNextEventReplay(1<<etUpdateEvt, &event, fFalse))
					DoUpdate(&event);
#endif /* MAC */
				Win( UpdateAllWws( fFalse ) );
				}
		ptT = pt;
		ptT.yw = min(ptT.yw, (*hwwdCur)->ywMac - 1);
		dl = DlWherePt(wwCur, &ptT, &hpldr, &idr, &spt, fTrue, fFalse);
		if (dl != dlNil && psel->doc == PdrFetchAndFree(hpldr, idr, &drfFetch)->doc)
			{
			Mac( int crsCur );

			if (fTableAnchor && psel->sty != styDoc && psel->sty != stySection &&
					FChangeSelToStyCol(psel, &cpFirst, ptT.xw, hpldr,
					&idr, dl, &caCell, &caInTable, spt.fInTable))
				{
LMakeTableSel:
				if (FDoContentHitColumn(psel, cpFirst, idr, &caInTable,
						&dl, &ptT, &hpldr, &idr, &spt, &caTapAnchor, itcAnchor, caCell.cpFirst))
					return;
				}
			if (psel->fTable)
				{
				MakeSelNonTable(psel, CpPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl,dl));
				FreePdrf(&drfFetch);
				}
			/* special mouse actions already taken care of */
			AssertDo(FSelectDlPt(psel, hpldr, idr, dl, ptT,
					psel->sty, fTrue, fTrue/*fMouse*/));
#ifdef MAC
		/* Check the cursor and see if we need to change it */
			if ((crsCur = CrsCurrent()) == crsIbeam || crsCur == crsItalicIbeam)
				{
				if (!vpref.fNoItalicCrs && FItalicCrs(wwCur, &ptT, hpldr, idr, dl))
					SetCrs(crsItalicIbeam);
				else
					SetCrs(crsIbeam);
				}
#endif
			}
		}
	OutlineSelCheck(psel);
/* upclick of the mouse */
#ifdef WIN
	ReleaseCapture();
	if (vhwndCBT)
		{
		/* Send CBT a message explaining what we've selected */
		if (psel->fTable)
			CBTTblSelectPsel(psel);
		else
			CBTSelectPsel(psel);
		}

#endif /* WIN */
/* cause cursor, if any, to start blinking */
	psel->tickOld = 0;
	if (psel->fIns) vxwCursor = psel->xw;
	return;
}


/* F  S E L  A N C H O R E D  I N  T A B L E */
/* returns fTrue when the anchoring cp of the selection is within a table */
/* %%Function:FSelAnchoredInTable %%Owner:chic */
FSelAnchoredInTable(psel, pcaCell, pcaTapAnchor, pitcAnchor)
struct SEL *psel;
struct CA *pcaCell;
struct CA *pcaTapAnchor;
int *pitcAnchor;
{
	int doc;
	int fInTable;
	CP cpAnchor;
	if (fInTable = psel->fTableAnchor)
		{
		CpFirstTap(doc = psel->doc, cpAnchor = psel->cpAnchor);
		*pcaTapAnchor = caTap;
		CacheTc(wwNil, doc, cpAnchor, fFalse, fFalse);
		*pcaCell = vtcc.ca;
		*pitcAnchor = psel->fTable ? ((psel->fRightward) ? psel->itcFirst : psel->itcLim - 1) :
		vtcc.itc;
		}
	return (fInTable);
}


/* L  D I S T  2  R C  P T */
/* calculates the square of the minimulm distance between a rectangle and 
a point. We assume point is outside of the rect. */
/* %%Function:LDist2RcPt %%Owner:chic */
long LDist2RcPt(prc, pt)
struct RC *prc; 
struct PT pt;
{
	int d1, d2;
	int xwRc, ywRc;

	if (pt.yw < prc->ywTop)
		ywRc = prc->ywTop;
	else  if (pt.yw <= prc->ywBottom)
		ywRc = pt.yw;
	else
		ywRc = prc->ywBottom;

	if (pt.xw < prc->xwLeft)
		xwRc = prc->xwLeft;
	else  if (pt.xw <= prc->xwRight)
		xwRc = pt.xw;
	else
		xwRc = prc->xwRight;

	d1 = abs(pt.yw - ywRc);
	d2 = abs(pt.xw - xwRc);
	return (long)((long)d1 * (long)d1) + (long)((long)d2 * (long)d2);

}


/* G E T  S E L  C U R  C H P */
/*  Get the correct chp for inserting at selCur iff either the current
one is not up to date of if fGetInsPtProps.
Call with fFalse: just before selCur.chp would be used.
Call with fTrue: whenever selCur.fIns and the formatting state of the
keyboard (as represented in selCur.chp) is to change.
*/

/* %%Function:GetSelCurChp  %%Owner:chic */
EXPORT GetSelCurChp (fGetInsPtProps)
BOOL fGetInsPtProps;
{
	if (selCur.sk != skNil &&
			(selCur.fUpdateChp || fGetInsPtProps))
		{
/* no assignment is made if in insertion mode and props are fSysVanish */
		GetPchpDocCpFIns (&selCur.chp, selCur.doc, 
				CpMin(selCur.cpFirst, CpMacDocEdit(selCur.doc)),
				selCur.fIns, selCur.ww);
		selCur.fUpdateChp = fFalse;
		selCur.fUpdateChpGray = fTrue;
		}

	Assert (!selCur.chp.fSpec); /* should never be true */
	Assert (!selCur.chp.fRMark);
	Assert (!selCur.chp.fStrike);
}


/* G E T  P C H P  D O C  C P  F I N S */
/*  Get the correct chp to use to insert text at doc, cp.  If fIns the
	properties are based on previous character else they are based on cp.

	No assignment is made if called to refresh insertion point during
	insertion (fSysVanish is set.)

	WIN version: If ww != wwNil, vanished property set only if vanished text is visible.
*/

/* %%Function:GetPchpDocCpFIns  %%Owner:chic */
EXPORT GetPchpDocCpFIns (pchp, doc, cp, fIns, ww)
struct CHP * pchp;
int doc;
CP cp;
BOOL fIns;
int ww;		/* ww unused, for OPUS compatability */
{
	CP cpT;
	CachePara (doc, cp);

LBegin:
	cpT = CpMax(caPara.cpFirst, cp - fIns);
	FetchCp (doc, cpT, fcmChars+fcmProps);
	if (fIns)
		{
		if (vchpFetch.fSysVanish) return;

/* footnote character: use the next character instead!! */
/* note: pseudo recurse since next character can also be footnote */
		if (vchpFetch.fSpec && (*vhpchFetch == chFootnote
#ifdef WIN
				|| *vhpchFetch == chAtn
#endif
				))
			{
			cp++;
			goto LBegin;
			}
		}
	*pchp = vchpFetch;
	/* Note: picture chars have fcPic and fSpec set. When inserted, they
		also get other current char props, so just return the other
		props and leave the other fields zeroed out
	*/
	pchp->fSpec = fFalse;  /* never inherit these */
	pchp->fRMark = fFalse;
	Win(pchp->fStrike = fFalse;)

#ifdef DEBUG
			/* flush out bugs from places that are not setting these guys up
		before using them - everyone should be */
	pchp->fcPic = ((FC) 0);   /* better than fcNil in this case */
	pchp->fnPic = fnNil;
#endif

	if (vchpFetch.fVanish)
		{
		pchp->fVanish =
#ifdef WIN
				(ww == wwNil || !fIns || PwwdWw(ww)->grpfvisi.fSeeHidden
				|| PwwdWw(ww)->grpfvisi.fvisiShowAll);
#else
		(vpref.fSeeHidden || !fIns);
#endif
		}

#ifdef WIN
	/*  always be sure this is set correctly. */
	pchp->fFldVanish = vchpFetch.fFldVanish ?
			FNestedInDeadField (doc, cp) : fFalse;
#endif
	Assert (!pchp->fSysVanish);  /* should never be set */
}


/* C P  L I M  S T Y */
/* %%Function:CpLimSty %%Owner:chic */
CP CpLimSty(ww, doc, cp, sty, fEnd)
int ww;
int doc;
CP cp;
int sty;
int fEnd;
{ /* Return the first cp which is not part of the same sty unit */
	/* iff cp is on the borderline between two sty's fEnd tells whether
		to regard cp as being at the end of the previous sty (fEnd true) or
		at the beginning of the following sty (fEnd false) */

	/* WIN's styChar is visibility sensitive */

	int wb, ch, ich;
	int ccpFetch;
	int idr;
	CP cpMac = CpMacDoc(doc);
	struct PLCEDL **hplcedl;
	struct PLDR **hpldr;
	CP cpVisi;
	struct EDL edl;
	struct CA caCharBlock;
	struct DRF drfFetch;
	CP cpT;
	CP cpNextFetch;

	if (cp >= cpMac && (sty != styCRLF || !fEnd))
		return cpMac;

	switch (sty)
		{
		int dl, spt;
		struct PT pt;

	case styCRLF:
		/* don't care about ww */
#ifdef WIN
		cpT = cp;
		cp = CpFirstSty(ww, doc, cp, styCRLF, fFalse);
		if (fEnd && cp == cpT)
			return cp;
		if (cp < cpMac)
			{
			FetchCpAndPara(doc, cp, fcmChars);
			if (*vhpchFetch == chReturn)
				{
LCheckEol:
				FetchCpAndPara(doc, cp+1, fcmChars);
				if (*vhpchFetch == chEol || *vhpchFetch == chTable)
					return cp+ccpEop;
				}
			}
#endif
		return cp + 1;

	case styPara:
		CachePara (doc, cp);
		if (cp == caPara.cpFirst && fEnd && cp > cp0)
			CachePara (doc, cp - 1);
		return caPara.cpLim;

	case stySection:
		Assert(!PdodDoc(doc)->fShort);
		CacheSect(doc, cp);
		if (cp == caSect.cpFirst && fEnd && cp > cp0)
			CacheSect(doc, cp - 1);
		return caSect.cpLim;


	case styChar:
	case styCol:
	case styRow:
	case styColAll:
	case styWholeTable:
#ifdef WIN
		/* returned cp WRT visible characters: */

		/* 1. Normalize cp wrt CRLF
			2. GetCaCharBlock
			3. Figure which end of CharBlock
			4. Adjust for fEnd
			5. CpFirst or CpLim
		*/
		cp=CpFirstSty(ww, doc, cp, styCRLF, fFalse);
		GetCaCharBlock(ww, doc, cp, &caCharBlock, &cpVisi);
		if (cpVisi != cpNil)
			cp = (cp <= cpVisi) ? caCharBlock.cpFirst : caCharBlock.cpLim;
		else
			cp = caCharBlock.cpFirst;
		if (fEnd)
			return ((cp <= cp0) ? cp0 : cp);
		else
			return caCharBlock.cpLim;
#else
		return fEnd ? cp : cp + 1;
#endif /* WIN */
	case styNil:
		return cp;
	case styLine:
	case styLineEnd:
		Assert(!PwwdWw(ww)->fDirty);
		NormCp(ww, doc, cp, ncpAnyVisible, dysMinAveLineSci, fEnd);
		idr = idrNil; /* initialize in case it is not filled */
		dl = DlWhereDocCp(ww, doc, cp, fEnd, &hpldr, &idr, NULL, NULL, fTrue);
		if (idr == idrNil || dl == dlNil)
/* this is possible in page view when NormCp is unable to go beyond the end
of a dr, e.g. extending to cpMac in a header. punt. */
			return cp;
		hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
		GetPlc(hplcedl, dl, &edl);
		cp = CpPlc(hplcedl, dl) + edl.dcp;
		FreePdrf(&drfFetch);
		return cp;

	case styDoc:
		return cpMac;

	case styWord:
	case styWordNoSpace:
	case stySent:
		break;

#ifndef JR
	default:
		Assert (sty >= styOutline);
/* must be outline to lvl = sty - styOutline. If text's lvl < lvl, stop
at start of paragraph. Else extend to sub-text */
		CachePara(doc, cp);
		for (;;)
			{
			if (caPara.cpLim >= CpMacDoc(doc) ||
					(LvlFromDocCp(doc, caPara.cpLim)
					<= (sty - styOutline)))
				return caPara.cpLim;
			CachePara(doc, caPara.cpLim);
			}
#endif
		} /* end of switch sty */

	/* Must be word or sentence */
	Assert(sty == styWord || sty == styWordNoSpace || sty == stySent);
#ifdef WIN
	if (fEnd && cp > cp0)
		cp = CpFirstSty (ww, doc, cp, styChar, fTrue);

	FetchCpPccpVisible (doc, cp, &ccpFetch, ww/*fvcScreen*/, fFalse);
	if (ccpFetch == 0)
		return cp;

	wb = WbFromCh(ch = vhpchFetch[ich = 0]);
#ifdef CRLF
	if (ch == chReturn)
		{
		cp = vcpFetch;
		goto LCheckEol;
		}
#endif
	if (FWhiteSpaceCh(ch))
		/* white space is its own unit */
		return vcpFetch + 1;

	if (wb == wbWhite && sty == stySent)
		{ /* Might be between sentences; go back to text */
		FetchCpPccpVisible (doc, CpFirstSty (ww, doc, cp, styWord, fTrue),
				&ccpFetch, ww/*fvcScreen*/, fFalse);
		if (ccpFetch == 0)
			return cp;

		wb = WbFromCh(ch = vhpchFetch[ich = 0]);
		if (FWhiteSpaceCh(ch))
/* cp is actually at the beginning of a sentence which starts with real spaces. */
			{
			FetchCpPccpVisible (doc, cp, &ccpFetch,	ww/*fvcScreen*/, fFalse);
			if (ccpFetch == 0)
				return cp;
			}
		}

	cpNextFetch = vcpFetch + ccpFetch;
	for (;;)
		{
		if (++ich >= ccpFetch)
			{ /* Get next line and set up */
			FetchCpPccpVisible (docNil, cpNil, &ccpFetch, ww/*fvcScreen*/, fFalse);
			if (vfEndFetch)
				return vcpFetch; /* End of doc */
			else  if (ccpFetch == 0) /* no more visi char */
				return (CpLimSty(ww, doc, cpNextFetch, styChar, fTrue));
			cpNextFetch = vcpFetch + ccpFetch;
			ich = 0;
			}
		if (sty == stySent)
			switch (ch)
				{
			case '.':
			case '!':
			case '?':
				sty = styWord;
				wb = wbPunct;
				}
		if (FWhiteSpaceCh(ch = vhpchFetch[ich]))
			break;
		if (sty == styWord || sty == styWordNoSpace)
			{ /* Word ends after white space or on text/punct break */
			int wbT = WbFromCh(ch);
			if (wb != wbT &&
					(sty == styWordNoSpace || (wb = wbT) != wbWhite))
				break;
			}
		}
	/*  why can you assert this in the fEnd case??? */
	Assert(vcpFetch+ich >= cp);
	return CpLimSty (ww, doc, vcpFetch + ich, styChar, fTrue);
#endif /*WIN */

#ifdef MAC
	FetchCp(doc, cp, fcmChars);

	/* Must be word or sentence */
	wb = WbFromCh(ch = vhpchFetch[ich = 0]);
#ifdef CRLF
	if (ch == chReturn)
		return vcpFetch + 2;
#endif
	if (FWhiteSpaceCh(ch))
		{
#ifdef BOGUS
/* we can't allow the chTable at the end of a table cell to be selected via
	styWord, stySent, et. al.*/
		CachePara(doc, cp);
		if (FInTableVPapFetch(doc, cp))
			{
			CacheTc(wwNil, doc, cp, fFalse, fFalse);
			if (cp + ccpEop == vtcc.ca.cpLim)
				return vcpFetch;
			}
#endif
		/* white space is its own unit */
		return vcpFetch + 1;
		}

	if (wb == wbWhite && sty == stySent)
		{ /* Might be between sentences; go back to text */
		FetchCp(doc, CpFirstSty(ww, doc, cp, styWord, fTrue), fcmChars);
		wb = WbFromCh(ch = vhpchFetch[ich = 0]);
		}

	for (;;)
		{
		if (++ich >= vccpFetch)
			{ /* Get next line and set up */
			FetchCp(docNil, cpNil, fcmChars);
			if (vfEndFetch)
				return vcpFetch; /* End of doc */
			ich = 0;
			}
		if (sty == stySent)
			switch (ch)
				{
			case '.':
			case '!':
			case '?':
				sty = styWord;
				wb = wbPunct;
				}
		if (FWhiteSpaceCh(ch = vhpchFetch[ich]))
			break;
		if (sty == styWord)
			{ /* Word ends after white space or on text/punct break */
			int wbT = WbFromCh(ch);
			if (wb != wbT && (wb = wbT) != wbWhite)
				break;
			}
		}
	return vcpFetch + ich;
#endif /* MAC */
}


/* F  W H I T E  S P A C E  C H */
/* %%Function:FWhiteSpaceCh %%Owner:chic */
NATIVE int FWhiteSpaceCh(ch)
int ch;
{
	switch (ch)
		{
	case chEop:
	case chCRJ:
	case chSect:
	case chTable:
	case chTab:
#ifdef WIN
	case chColumnBreak:
#endif
#ifdef CRLF
	case chReturn:
#endif
		return fTrue;
		}
	return fFalse;
}


/* C P  F I R S T  S T Y */
/* %%Function:CpFirstSty %%Owner:chic */
CP CpFirstSty(ww, doc, cp, sty, fEnd)
int ww;
int doc;
CP cp;
int sty, fEnd;
{ /* Return the first cp of this sty unit. */
/* iff cp is on the borderline between two sty's fEnd tells whether
to regard cp as being at the end of the previous sty (fEnd true) or
at the beginning of the following sty (fEnd false) */
	int wb, ich, ch, dcpChunk;
	int ccpFetch;
	int idr;
	int dl;
	struct PLCEDL **hplcedl;
	struct PLDR **hpldr;
	struct EDL edl;
	CP cpMac = CpMacDoc(doc);
	CP cpSent;
	CP cpT, cpBegin;
	struct CA caCharBlock;
	CP cpVisi, cpOld;
#define dcpAvgChar      5
#define dcpAvgWord      10
#define dcpAvgSent      100
#define dcpAvgVan       100
	CHAR rgch[dcpAvgSent];
	struct CR rgcr [dcpAvgVan];/* guarantee not a limit to fetch! */
	struct FVB fvb;
	BOOL fText;

	if (cp == cp0 && (sty == styCRLF || fEnd))
		return cp;

	switch (sty)
		{
		int spt;
		struct PT pt;

	case styCRLF:
		/* don't care about ww */
		if (fEnd && cp > 0)
			cp--;

#ifdef WIN
		if (cp < cpMac)
			{
			FetchCpAndPara(doc, cp, fcmChars);
			if ((*vhpchFetch == chEol || *vhpchFetch == chTable)
					&& cp > cp0)
				{
				FetchCpAndPara(doc, cp-1, fcmChars);
				if (*vhpchFetch == chReturn)
					return cp-1;
				}
			}
#endif
		return cp;

	case styPara:
		if (cp >= cpMac)
			{
			Assert (fEnd);
			cp = cpMac - 1;
			}

		CachePara (doc, cp);
		if (cp == caPara.cpFirst && fEnd && cp > cp0)
			CachePara (doc, cp - 1);
		return caPara.cpFirst;

	case stySection:
		if (cp >= cpMac)
			{
			Assert (fEnd);
			return cpMac - ccpEop;
			}

		CacheSect (doc, cp);
		if (cp == caSect.cpFirst && fEnd && cp > cp0)
			CacheSect (doc, cp - 1);
		return caSect.cpFirst;


	case styChar:
	case styCol:
	case styRow:
	case styColAll:
	case styWholeTable:
#ifdef MAC
		return(cp);
#else /* WIN */
		/* returned cp WRT visible characters: */

		/* 1. Normalize cp wrt CRLF 
			2. GetCaCharBlock
			3. Figure which end of CharBlock
			4. Adjust for fEnd
			5. CpFirst or CpLim
		*/

		cp=CpFirstSty(ww, doc, cp, styCRLF, fFalse);
		if (cp < cpMac)
			{
			GetCaCharBlock(ww, doc, cp, &caCharBlock, &cpVisi);
			if (cpVisi != cpNil)
				cp = (cp <= cpVisi) ? caCharBlock.cpFirst : caCharBlock.cpLim;
			else
				cp = caCharBlock.cpFirst;
			}
		if (cp >= cpMac)
			{
			if (!fEnd)
				return cpMac;
			cp = cpMac - ccpEop;
			if (sty == styChar)
				cp = CpFirstSty(ww, doc, cp, sty, fFalse);
			return cp;
			}

		if (fEnd && cp > cp0)
			{
			GetCaCharBlock(ww, doc, cp-1, &caCharBlock, &cpVisi);
			return caCharBlock.cpFirst;
			}
		else
			return cp;
#endif /* WIN */

	case styNil:
		return cp;
	case styDoc:
		return cp0;
	case styLine:
	case styLineEnd:
		vhpldrRetSel = HwwdWw(ww); 
		vidrRetSel = 0;
		NormCp(ww, doc, cp, ncpAnyVisible, dysMinAveLineSci, fEnd);
		if (DlWhereDocCp(ww, doc, cp, fEnd, &vhpldrRetSel, &vidrRetSel, &cpT, NULL, fTrue)
				== dlNil)
			/* failed due to out of memory */
			return cp;
		return cpT;
	case stySent:
		dcpChunk = dcpAvgSent;
		break;
	case styWord:
		dcpChunk = dcpAvgWord;
#ifndef JR
		break;
	default:
		Assert (sty >= styOutline);
/* must be outline to lvl = sty - styOutline. Stop at start of a
paragraph with text lvl <= lvl. */
		CachePara(doc, cp);
		for (;;)
			{
			if (LvlFromDocCp(doc, caPara.cpFirst) <= (sty - styOutline)
					|| caPara.cpFirst == 0)
				return caPara.cpFirst;
			CachePara(doc, caPara.cpFirst - 1);
			}
#endif
		} /* end of switch sty */

	Assert( sty == styWord || sty == stySent );

#ifdef WIN
	if (fEnd)
		cp = CpFirstSty (ww, doc, cp, styChar, fTrue);

	cpOld = cp;
	fText = (WbFromCh(ChFetch(doc, cp, fcmChars)) == wbText);
	InitFvb (&fvb);
	InitFvbBufs (&fvb, rgch, dcpChunk, rgcr, dcpAvgVan);
	fvb.doc = doc;
	fvb.cpLim = ++cp;
	fvb.cpFirst = cp = (cp > dcpChunk) ? cp - dcpChunk : cp0;

	FetchVisibleRgch (&fvb, ww/*fvcScreen*/, fFalse, fFalse);
	if (fvb.cch)
		{
		CP cpIchPcr;

		ich = fvb.cch - 1;
		wb = WbFromCh(ch = rgch[ich]);
/* use of CpFirstSty(styCRLF) below is to prevent splitting table cell mark.
A table cell mark contains chReturn/chTable pair, when a table cell mark
is format hidden, chReturn is invisible but chTable is still returned
from FetchVisibleRgch.
*/
		cpIchPcr = CpFirstSty(ww, doc, 
				CpFromIchPcr(ich, rgcr, fvb.ccr), styCRLF, fFalse);

		if (FWhiteSpaceCh(ch))
			{
			if (fText && !fEnd)
LAlreadyThere:
/* already at sty boundary; make sure it's visible */
				return CpFirstSty(ww, doc, cpOld, styChar, fFalse);
			return cpIchPcr;
			}

		if (wb == wbText)
			cpSent = cpIchPcr;
		else
			cpSent = cpNil;
		}
	else
		ich = 0;

	for (;;)
		{
		if (ich == 0)
			{
			if (cp == cp0)
				return cp0; /* beginning of doc */
			fvb.cpLim = cp;
			fvb.cpFirst = cp = (cp > dcpChunk) ? cp - dcpChunk : cp0;
			FetchVisibleRgch (&fvb, ww/*fvcScreen*/, fFalse, fFalse);
			if (!fvb.cch)
				continue;
			/* No decement by 1.  Because it will be pre-decremented before used. */
			ich = fvb.cch;
			}
		ch = rgch[--ich];
		if (FWhiteSpaceCh(ch))
			break; /* Always ends a unit */
		if (sty == styWord)
			{
			if (wb != wbWhite)
				{
				if (WbFromCh(ch) != wb)
					break;
				}
			else
				wb = WbFromCh(ch);
			}
		else
			{ /* Test for sentence. */
			switch (ch)
				{
			case '.':
			case '!':
			case '?':
				if (cpSent != cpNil)
					return cpSent;
				}
			switch (WbFromCh(ch))
				{
			case wbText:
				cpSent = CpFromIchPcr (ich, rgcr, fvb.ccr);
				wb = wbText;
				break;
			case wbPunct:
				switch (wb)
					{
				case wbWhite:
					wb = wbPunct;
					break;
				case wbText:
					cpSent = CpFromIchPcr (ich, rgcr, fvb.ccr);
					}
				break;
			case wbWhite:
				if (wb == wbPunct)
					cpSent = CpFromIchPcr (ich, rgcr, fvb.ccr) + 1;
				wb = wbWhite;
				break;
				}
			}
		} /* end of for (;;) */
	return CpFirstSty (ww, doc, CpFromIchPcr (ich, rgcr, fvb.ccr) + 1,
			styChar, fFalse);
#endif /* WIN */
#ifdef MAC
	cp++; /* = Lim */
	cpBegin = (cp > dcpChunk) ? cp - dcpChunk : cp0;

	FetchRgch(&ich, rgch, doc, cpBegin, cp, dcpChunk);
	wb = WbFromCh(ch = rgch[--ich]);

	if (FWhiteSpaceCh(ch))
		return cpBegin + ich;

	if (wb == wbText)
		cpSent = cpBegin + ich;
	else
		cpSent = cpNil;

	for (;;)
		{
		if (ich == 0)
			{
			if (cpBegin == cp0)
				return cp0; /* beginning of doc */
			cpBegin = (cpBegin > dcpChunk) ? cpBegin - dcpChunk : cp0;
			FetchRgch(&ich, rgch, doc, cpBegin, cp, dcpChunk);
			}
		ch = rgch[--ich];
		if (FWhiteSpaceCh(ch))
			break; /* Always ends a unit */
		if (sty == styWord)
			{
			if (wb != wbWhite)
				{
				if (WbFromCh(ch) != wb)
					break;
				}
			else
				wb = WbFromCh(ch);
			}
		else
			{ /* Test for sentence. */
			switch (ch)
				{
			case '.':
			case '!':
			case '?':
				if (cpSent != cpNil)
					return cpSent;
				}
			switch (WbFromCh(ch))
				{
			case wbText:
				cpSent = cpBegin + ich;
				wb = wbText;
				break;
			case wbPunct:
				switch (wb)
					{
				case wbWhite:
					wb = wbPunct;
					break;
				case wbText:
					cpSent = cpBegin + ich;
					}
				break;
			case wbWhite:
				if (wb == wbPunct)
					cpSent = cpBegin + ich + 1;
				wb = wbWhite;
				break;
				}
			}
		}
	return cpBegin + ich + 1;
#endif /* MAC */
}


/* W B  F R O M  C H */
/* %%Function:WbFromCh %%Owner:chic */
NATIVE int WbFromCh(ch)
int ch;
{ /* Return word-breakness of ch */

	if (FLower(ch) || FUpper(ch) || (ch >= '0' && ch <= '9'))
		return wbText;

	switch (ch)
		{
	case chSpace:
	case chEop:
#ifdef CRLF
	case chReturn:
#endif
	case chTable:
	case chSect:
#ifdef WIN
	case chColumnBreak:
#endif
	case chTab:
	case chCRJ:
	case chNonBreakSpace:
		return wbWhite;

	case 047:       /* '\'' */
#ifdef MAC
	case 0136:      /* circumflex */
	case 0137:      /* _ */
	case 0140:      /* grave accent */
	case 0176:      /* tilde */
	case 0247:      /* German sharp s */
	case 0243:      /* acute accent */
	case 0254:      /* umlaut */
	case 0330:      /* y umlaut */
	case 0325:      /* right ' */
#endif
	case chNonReqHyphen:
		return wbText;
	default:
		return wbPunct;
		}
}


/* F  E N D  V I S I B L E */
/* returns fEnd bit of last edl. Know: !fPageView */
/* %%Function:FEndVisible %%Owner:chic */
BOOL FEndVisible(ww)
{
	int dlMac;
	struct PLCEDL **hplcedl;
	struct EDL edl;

	if ((dlMac = IMacPlc(hplcedl = PdrWw(ww, 0)->hplcedl)) == 0)
		return fFalse;
	GetPlc(hplcedl, dlMac - 1, &edl);
	return edl.fEnd;
}


/* F  A N Y  D L  N O T  I N  X W */
/* returns true iff any dl in ww hangs out to the left (right) of
wwd.rcDisp */
/* %%Function:FAnyDlNotInXw %%Owner:chic */
FAnyDlNotInXw(ww, fLeft)
{
	struct WWD *pwwd = PwwdWw(ww);
	struct WWD *hpldr = HwwdWw(ww);
	int idrMac = pwwd->idrMac;
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	int dlMac;
	int idr, dl;
	int xwLeft;
	int xwMin = pwwd->xwMin;
	struct EDL edl;

	if (pwwd->fOutline) xwMin += DxpFromDxa(pwwd, dxaOtlMark);
	pdr = PdrWw(ww, 0);
	for (idr = 0; idr < idrMac; idr++, pdr++)
		{
		if (pdr->hplcedl == hNil)
			continue;
/* check for dr being entirely within range */
		if (fLeft)
			{
			if (XwFromXp(hpldr, idr, -pdr->dxpOutLeft)
					>= xwMin) continue;
			}
		else
			{
			if (XwFromXp(hpldr, idr, pdr->dxl + pdr->dxpOutRight)
					<= pwwd->xwMac) continue;
			}
		hplcedl = pdr->hplcedl;
		dlMac = IMacPlc(hplcedl);
		for (dl = 0; dl < dlMac; dl++)
			{
			GetPlc(hplcedl, dl, &edl);
			if (edl.dlk)
				break;
			xwLeft = XwFromXp(hpldr, idr, edl.xpLeft);
			if (fLeft)
				{
				if (xwLeft >= xwMin) continue;
				}
			else
				{
				if (xwLeft + edl.dxp
						<= pwwd->xwMac) continue;
				}
			return fTrue;
			}
		}
	return fFalse;
}


/* %%Function:SetSelCellBits %%Owner:davidlu */
EXPORT SetSelCellBits(psel)
struct SEL *psel;
{
	struct CA caTablePara;
	CP cpOpposite;

/* if the selection is anchored within a table cell, check to see if the
	entire selection is within the cell. */
	psel->fWithinCell = fFalse;
	if (psel->fTableAnchor = FInTableDocCp(psel->doc, psel->cpAnchor))
		{
		CachePara(psel->doc, psel->cpAnchor);
		cpOpposite = (psel->fForward) ? psel->cpLim : psel->cpFirst;
		caTablePara = caPara;
		caTablePara.cpFirst = vcpFirstTablePara;
		if (FInCa(psel->doc, cpOpposite, &caTablePara))
			psel->fWithinCell = fTrue;
		else
			{
			CacheTc(wwNil, psel->doc, psel->cpAnchor, fFalse, fFalse);
			if (FInCa(psel->doc, cpOpposite, &vtcc.ca))
				psel->fWithinCell = fTrue;
			}
		}
#ifdef WIN
	if (psel == &selCur)
		{
		if (psel->fWithinCell || psel->fTable)
			{
			if (!vfTableKeymap)
				{
				/* don't care if this fails here */
				FHookTableKeymap();
				}
			}
		else  if (vfTableKeymap)
			UnhookTableKeymap();
		}
#endif /* WIN */
}


#ifdef WIN
/* H O O K  T A B L E  K E Y M A P */
/* %%Function:FHookTableKeymap %%Owner:rosiep */
BOOL FHookTableKeymap()
{
	Assert(hkmpTable);

	/* hook it in before the user's keymap */
	(*hkmpTable)->hkmpNext = (*vhkmpUser)->hkmpNext;
	(*vhkmpUser)->hkmpNext = hkmpTable;

	vfTableKeymap = fTrue;
}


/* U N H O O K  T A B L E  K E Y M A P */
/* %%Function:UnhookTableKeymap %%Owner:rosiep */
UnhookTableKeymap()
{
	AssertDo(FDislinkHkmp(hkmpTable));
	vfTableKeymap = fFalse;
}


#endif /* WIN */


/* %%Function:SetSelsIns %%Owner:chic */
SetSelsIns(psels, doc, cp)
struct SELS *psels;
int doc;
CP cp;
{
	SetWords(psels, 0, cwSELS);
	psels->doc = doc;
	psels->fForward = fTrue;
	psels->sk = skIns;
	psels->fInsEnd = fFalse;
	psels->cpFirst = psels->cpLim = psels->cpAnchor = cp;
	psels->xpFirst = psels->xpLim = xpNil;
	psels->sty = styNil;
	SetSelCellBits(psels);
}


#ifdef WIN
/* %%Function:MakeExtendSel %%Owner:chic */
MakeExtendSel(psel, cpFirst, cpLim, grpf)
struct SEL *psel;
CP cpFirst;
CP cpLim;
int grpf;
{
	BOOL fDoAdjust = grpf & maskFDoAdjust;
	BOOL fDoMakeSelCurVisi = grpf & maskFDoMakeSelCurVisi;

	if (fDoAdjust)
		{
		cpFirst = CpFirstSty(psel->ww, psel->doc, cpFirst, styChar, fFalse);
		cpLim = CpLimSty(psel->ww, psel->doc, cpLim, styChar, fTrue);
		}

	if (psel->cpFirst == cpFirst && psel->cpLim == cpLim && !psel->fTable)
		return;

	if (vfExtendSel)
		{
		ChangeSel(psel,(psel->cpLim >= cpLim ? cpFirst : cpLim), styNil, fTrue, fFalse);  /* calls select() */
		OutlineSelCheck(psel);
		NormCp(psel->ww, psel->doc, CpActiveCur(), ncpVisifyPartialDl+ncpHoriz, 0, psel->fInsEnd);
		}
	else
		{
		Select(psel, cpFirst, cpLim);
		if (fDoMakeSelCurVisi)
			MakeSelCurVisi(fTrue /*fForceBlockToIp*/);
		OutlineSelCheck(psel);
		vfSeeSel = fTrue;
		}

}


/* C P  L A S T  S K I P  O V E R */
/*  Return cpLast of outermost field enclosing cp1 not enclosing
	cp2.
*/

/* %%Function:CpLastSkipOver %%Owner:peterj */
CP CpLastSkipOver(doc, cp1, cp2)
int doc;
CP cp1, cp2;
{
	struct PLC **hplcfld = PdodDoc(doc)->hplcfld;
	struct FLD fld;
	int ifld, ifldMin;
	CP cpLast;

	if (hplcfld != hNil)
		{
		int c = 0;
		ifld = IInPlcCheck(hplcfld, cp2-1);
		ifldMin = IInPlcRef(hplcfld, cp1);

		while (ifld >= ifldMin)
			{
			Assert(ifld >= 0 && ifldMin >= 0 && ifld < IMacPlc(hplcfld));
			GetPlc(hplcfld, ifld, &fld);
			if (fld.ch == chFieldEnd)
				{
				if (!c++)
					cpLast = CpPlc(hplcfld, ifld);
				}
			else  if (fld.ch == chFieldBegin && c != 0)
				c--;
			ifld--;
			}
		if (c)
			return cpLast;
		}
	return cp1;
}



/* %%Function:EidMouseAction %%Owner:bobz */
EidMouseAction()
	/* returns eidNil if valid mouse action, else error id */
{
	int eid;

	/* invalid to do right click (block sel) in dyadic mode or to
		do dyadic mouse ops when in dyadic mode or with a block sel */

	if ( ((vssc != sscNil && vssc != sscMouse) && vfRightClick)
			|| (FSscClick() && ((vssc != sscNil && vssc != sscMouse)
			|| selCur.fBlock)) )
		eid = eidInvalidSelection;
	else  if (FSscClick() && (vfRightClick && selCur.fIns))
		eid = eidMakeSelection;
	else
		eid = eidNil;

	return (eid);
}


#endif /* WIN */



#ifdef WIN
/* %%Function:FSscClick %%Owner:bobz */
FSscClick()
{
	return vfControlKey && (vfRightClick || vfShiftKey);
}



/* M A K E  S E L  C U R  V I S I */
/*  Makes the current selection visible (wrt hidden text, dead fields, outline). 
*/
/* %%Function:MakeSelCurVisi %%Owner:chic */
MakeSelCurVisi(fForceBlockToIp)
BOOL fForceBlockToIp;
{
	CP cpFirstNew;
	CP cpLastSel;
	int ccpFetch;
	CP cpImport;
	int fFirstVisi;
	struct CA caCharBlock;
	CP cpVisi;
	CP cpFirst = selCur.cpFirst;
	CP cpLim;
	struct CA caTableCell;
	struct CA caT;

	InvalVisiCache();

	if (selCur.fTable)
		{
		/* get the cpFirst of the table cell */

		for ( ; ; )
			{
			CpFirstTap(selCur.doc, cpFirst); /* filled caTap */
			if (vtapFetch.itcMac >= selCur.itcFirst)
				break;
			cpFirst = caTap.cpLim;
			Assert(FInTableDocCp(selCur.doc, cpFirst));
			}

		Assert(cpFirst < selCur.cpLim);
		caT = caTap;
		PcaColumnItc(&caTableCell, &caT, selCur.itcFirst);
		cpFirst = caTableCell.cpFirst;
		}
	else  if (selCur.fBlock)
		{
		if (fForceBlockToIp)
			{
/* erase block sel before new edls info set in UpdateWw in SelectIns */
			TurnOffSel(&selCur);
			SelectIns(&selCur, selCur.cpFirst);
			}
		else
			cpFirst = CpFirstBlock();
		}

	/* need to check visibility of the first of the selection. 
		if insertion point precede visible text, no need to change it */

	FetchCpPccpVisible(selCur.doc, cpFirst, &ccpFetch, wwCur, fFalse);
	if (ccpFetch > 0)
		{
		fFirstVisi = (vcpFetch == cpFirst);
		cpFirstNew = CpFirstSty(wwCur, selCur.doc, vcpFetch, styChar, fFalse);
		}
	else
		{
		GetCaCharBlock(wwCur, selCur.doc, cpFirst, &caCharBlock, &cpVisi);
		cpFirstNew = cpVisi != cpNil ? caCharBlock.cpFirst :
				(caCharBlock.cpFirst == cp0 ? cp0 :
				CpFirstSty(wwCur, selCur.doc, caCharBlock.cpFirst, styChar, fTrue));
		fFirstVisi = (cpFirst == cpVisi);
		}
	fFirstVisi |= (cpFirst == cpFirstNew);

	if (fFirstVisi && selCur.fIns)
		return;
	/* if first was visible, must check the last for visibility */
	if (fFirstVisi)
		{
		if (selCur.fTable)
			{
			CpFirstTap(selCur.doc, selCur.cpLim-1);
			caT = caTap;
			PcaColumnItc(&caTableCell, &caT, min(selCur.itcLim, vtapFetch.itcMac));
			cpLim = caTableCell.cpFirst;
			}
		else  if (selCur.fBlock)
			{
			CP cp, dcp;
			struct BKS bks;

			InitBlockStat(&bks);
			bks.cpFirst = selCur.cpLast;
			if (FGetBlockLine(&cp, &dcp, &bks))
				cpLim = cp+dcp;
			else  
				cpLim = bks.cpFirst;
			}
		else
			cpLim = selCur.cpLim;

		cpLastSel = CpFirstSty(wwCur, selCur.doc, cpLim, styCRLF, fTrue);
		FetchCpPccpVisible(selCur.doc, cpLastSel, &ccpFetch, wwCur, fFalse);
		}

	if (!fFirstVisi || ccpFetch == 0 || (vcpFetch != cpLastSel && 
			cpLim != CpLimSty(wwCur, selCur.doc, cpLastSel, styChar, fTrue)))
		SelectIns(&selCur, cpFirstNew);
	else
		{
		/* when toggling a picture import field, we might not call select,
				and so the graphics bit would not be set when it should be
				and vice versa
		*/
		if (FCaIsGraphics(&selCur.ca, selCur.ww, &cpImport))
			selCur.sk = skGraphics;
		else
			/* can't change the sk in this case, don't know what to
				set other bits to
			*/
			selCur.fGraphics = fFalse;
		}

/* destroy the hidden transient property if hidden text is not shown */
	if (selCur.chp.fVanish)
		selCur.chp.fVanish =
				(wwCur == wwNil || !selCur.fIns || 
				PwwdWw(wwCur)->grpfvisi.fSeeHidden ||
				PwwdWw(wwCur)->grpfvisi.fvisiShowAll);

}


/* C P  L A S T  V I S I  O U T L I N E */
/* Should only be called in outline mode
	Return the last cp that is visible in the pad (cpLastVisi)
	and the last cp that is invisible after it but not in the
	same pad as the next visi character (cpLastInvisi does
	not have to be within the same pad as cpLastVisi)

Note: cpLastVisi returned can be the same as cpLastInvisi in cases 
where there is no hidden text in between.
*/
/* %%Function:CpLastVisiOutline %%Owner:chic */
CP CpLastVisiOutline(ww, doc, cp, pcpLastInvisi)
int ww;
int doc;
CP cp;
CP *pcpLastInvisi;
{
	CP cpLastVisi = cp;
	CP cpFirstPara;
	CP cpLimPara;
	int ccpFetch;

	Assert(PwwdWw(ww)->fOutline && !PdodDoc(doc)->fShort);

/* use the cpLim from visi para instead of cp from the next pad because
field result may also had pad entry but it is not showing in outline. */
	GetCpFirstCpLimDisplayPara(ww, doc, cp, &cpFirstPara, &cpLimPara);

	FetchCpPccpVisible(doc, cp, &ccpFetch, ww, fFalse);
	while (ccpFetch > 0 && vcpFetch < cpLimPara)
		{
		CP cpNext;

		cpNext = vcpFetch + ccpFetch;
		cpLastVisi = cpNext - 1;
		FetchCpPccpVisible(doc, cpNext, &ccpFetch, ww, fFalse);
		}
/* got cpLastVisi,if nothing is visible in that pad, return the input cp */
	Assert(cpLastVisi >= cp0 && cpLastVisi < cpLimPara);

/* find cpLastInvisi:
1. find the pad that the next visible cp belongs to.
2. find the cpFirst of the pad.
3. the cp before the cpFirst of the pad is cpLastInvisi
*/
	FetchCpPccpVisible(doc, CpLimSty(ww, doc, cpLastVisi, styCRLF, fFalse),
			&ccpFetch, ww, fFalse);
	if (ccpFetch > 0)
		{
		struct PLCPAD **hplcpad = PdodDoc(doc)->hplcpad;
		int ipadFromCp = IInPlc(hplcpad, cp);
		int ipadFromCpFetch = IInPlc(hplcpad, vcpFetch);
		CP cpLastInvisi;

		/* ipadFromCpFetch can be the same as ipadFromCp when
		you are in a table, each table cell can terminate a paragraph 
		but all remain in a single pad, in that case, use cpLimPara
		*/
		cpLastInvisi = ((ipadFromCpFetch > ipadFromCp) ?
			CpPlc(hplcpad, ipadFromCpFetch) : cpLimPara) - 1;
 
		Assert(cpLastInvisi >= cp0 && cpLastInvisi <= CpMacDocEdit(doc));
		*pcpLastInvisi = CpFirstSty(ww, doc, cpLastInvisi, styCRLF, fFalse);
		Assert(*pcpLastInvisi <= CpMacDocEdit(doc));
		}
	else
		*pcpLastInvisi = CpMacDocEdit(doc);

	return CpFirstSty(ww, doc, cpLastVisi, styCRLF, fFalse);
}


/*  C P L I M  C H A R  B L O C K  */
/*  Find the cpLim of a char unit, taking hidden text, outline visibility, field
	character, table in field into account.
*/

/* %%Function:CpLim2CharBlock %%Owner:chic */
CP CpLim2CharBlock(ww, doc, cp)
int ww;
int doc;
CP cp;
{
	CP cpNext = CpLimSty(ww, doc, cp, styCRLF, fFalse);
	CP cpLim = cpNext;
	CP cp2;
	int ccpFetch;
	CP cpLastInvisi;
	extern CP vcpFirstTableCell;

	if (cp < CpMacDocEdit(doc))
		{
		FetchCpPccpVisible(doc, cpNext, &ccpFetch, ww /*fvcScreen*/, fFalse /*fNested*/);
		if (ccpFetch > 0 && (cp2 = vcpFetch) != cpNext)
			{
			if (PwwdWw(ww)->fOutline && 
					cp == CpLastVisiOutline(ww, doc, cp, &cpLastInvisi))
				{
				cp = cpLastInvisi;
				Assert(cp <= cp2);
				}

/* CpLastSkipOver returns cpLast of outermost field enclosing cp but not
enclosing cp2 */
			cpLim = CpLimSty(ww, doc, CpLastSkipOver(doc, cp, cp2), styCRLF, fFalse);
			if (FInTableDocCp(doc, cp2) && vcpFirstTableCell > cpLim)
				cpLim = vcpFirstTableCell;
			}
		else  if (ccpFetch == 0)
			cpLim = (PwwdWw(ww)->fOutline) ? CpMacDocEdit(doc) : cpNext;
		}

	return CpMin(cpLim, CpMacDoc(doc)); /* safety */
}


/* cache for CpLimCharBlock (stores up to 3 cp pairs) */
struct CBC vcbc;

#ifdef DEBUG
int vcCBCUsed = 0;
int vcCBCCalls = 0;
int vcCBCReset = 0;
#endif /* DEBUG */

/* %%Function:CpLimCharBlock %%Owner:chic */
CP CpLimCharBlock(ww, doc, cp)
int ww;
int doc;
CP cp;
{
	int icp;
	CP cpLim;

	Debug(vcCBCCalls++);

	if (ww == vcbc.ww && doc == vcbc.doc)
		/* look for it in the cache */
		{
		for (icp = 0; icp < icpCBCMax; icp++)
			if (cp >= vcbc.rgcp[icp] && cp < vcbc.rgcpLim[icp])
				/* found it -- use it */
				{
				Debug(vcCBCUsed++);
				return vcbc.rgcpLim[icp];
				}
		}
	else
		/* reset the cache */
		{
		Debug(vcCBCReset++);
		vcbc.ww = ww;
		vcbc.doc = doc;
		vcbc.iNext = 0;
		for (icp = 1; icp < icpCBCMax; icp++)
			vcbc.rgcp[icp] = vcbc.rgcpLim[icp] = cp0;
		}

	/* not found, look it up */
	cpLim = CpLim2CharBlock(ww, doc, cp);

	/* add it to the cache */
	Assert(vcbc.iNext >= 0 && vcbc.iNext < icpCBCMax);
	vcbc.rgcp[vcbc.iNext] = cp;
	vcbc.rgcpLim[vcbc.iNext] = cpLim;
	vcbc.iNext = (vcbc.iNext+1)%icpCBCMax;

	Assert(vcbc.iNext >= 0 && vcbc.iNext < icpCBCMax);

	return cpLim;
}



/* C P V I S I  B E F O R E  C P */
/* return the nearest visible cp to the left of the given cp */
/* %%Function:CpVisiBeforeCp %%Owner:chic */
CP CpVisiBeforeCp(ww, doc, cp)
int ww;
int doc;
CP cp;
{
#define dcpAvgChar  5
#define dcpAvgVan   100
	int dcpChunk;
	CP cpSave = cp;
	CHAR rgch[dcpAvgVan+1];
	struct CR rgcr[dcpAvgVan+1];
	struct FVB fvb;

	if (PwwdWw(ww)->fOutline)
		{
		struct PLC **hplcpad = PdodDoc(doc)->hplcpad;
		int ipad = IInPlc(hplcpad, cp);
		struct PAD pad;
		CP cpT;

		GetPlc(hplcpad, ipad, &pad);
		if (cp == CpPlc(hplcpad, ipad) || !pad.fShow)
			{
			if (pad.fShow && ipad > 0)
				GetPlc(hplcpad, --ipad, &pad);
			if (!pad.fShow)
				{
				while (!pad.fShow)
					{
					if (ipad > 0)
						{
						cp = CpPlc(hplcpad, --ipad);
						GetPlc(hplcpad, ipad, &pad);
						}
					else
						return cpNil;
					}
				Assert(ipad != IInPlc(hplcpad, cpSave));
				GetPlc(hplcpad, ipad, &pad);
				if (pad.fBody && PwwdWw(ww)->fEllip)
					{
					CachePara(doc, (cpT = CpPlc(hplcpad, ipad)));
					FormatLine(ww, doc, CpFormatFrom(ww, doc, cpT));
					cp = vfli.cpMac;
					}
				else
					cp = CpPlc(hplcpad, ++ipad);
#ifdef DEBUG
				GetPlc(hplcpad, ipad, &pad);
				Assert(!pad.fShow);
#endif
				}
			}
		} /* outline */

	if (cp == cp0)
		return cpNil;

/* Walk backward to find the nearest visible cp */
	InitFvb(&fvb);
	InitFvbBufs(&fvb, rgch, dcpAvgVan, rgcr, dcpAvgVan);
	fvb.doc = doc;
	dcpChunk = dcpAvgChar;

	do 
		{
		fvb.cpLim = cp;
		cp = CpVisibleBackCpField(doc, cp, ww);
		if ((fvb.cpFirst = (cp -= dcpChunk)) < cp0)
			fvb.cpFirst = cp0;
		else
/*  in general case will be getting 1ch so use
small dcp.  If that fails, increase dcp. */
			dcpChunk = dcpAvgVan;
		FetchVisibleRgch (&fvb, ww/*fvcScreen*/, fFalse, fFalse);
		}
	while (fvb.cch == 0 && cp > cp0);

	if (fvb.cch != 0)
		cp = CpFromIchPcr (fvb.cch-1, rgcr, fvb.ccr);
	else 
/*  Special case : no visi character before, return cpNil */
		cp = cpNil;


	return cp;
}


/*  G E T  C A  C H A R  B L O C K  */
/* Input : ww, doc, cp
	Output : pca, pcpVisi
	Find the ca surrounding cp that is consider a styChar unit.
	Takes into account of fields and table.

	Within this ca, only 1 cp is visible WRT screen and is returned in pcpVisi
	Two exceptions are :
	1) when there is no visible text between cp0 and CpMacDoc, 
	pca->cpFirst is cp0, pca->cpLim is cpMacDocEdit, cpVisi is cpNil.
	2) When there is no visible text after cp, 
	pca-cpFirst is cpInVisi after cp,
	pca->cpLim is CpMacDocEdit, cpVisi is cpNil.

	In general, invisible text to the left of a visible cp is grouped as
	one charblock.  
	Exceptions are:
	1) field in result mode.  The closing field bracket (invisible) is grouped
	with the visible cp to the left.  
	2) outline with ellipse.  The last visible cp before the ... is grouped to the
	right with the not showing characters as one charblock.  
	3) outline with hidden paragraph mark.  The hidden paragraph mark is grouped
	with the visible char to the left.
	4) table in a result field.  The first visible cp in the table does not group to 
	the left of the invisible field characters.  This is to allow insertion to the
	beginning of the table.
	5) in outline mode, when there is no visible text before the interesting cp,
	the hidden text to the left is not included as part of the character block.
*/
/* %%Function:GetCaCharBlock %%Owner:chic */
GetCaCharBlock(ww, doc, cp, pca, pcpVisi)
int ww;
int doc;
CP cp;
struct CA *pca;
CP *pcpVisi;
{
	CP cpFirst, cpLim;
	CP cpVisi1, cpVisi2, cpVisiBefore;
	int ccpFetch;
	extern CP vcpFirstTableCell;

	if (cp >= CpMacDoc(doc))
		{
		/* quick case - beyond cpMacDoc */
		pca->doc = doc;
		pca->cpFirst = pca->cpLim = CpMacDoc(doc);
		*pcpVisi = cpNil;
		return;
		}

/* Find cpLim, also get a potential cpVisi */
	cp = CpFirstSty(ww, doc, cp, styCRLF, fFalse);
	FetchCpPccpVisible(doc, cp, &ccpFetch, ww /*fvcScreen*/, fFalse/*fNested*/);
	if (ccpFetch == 0)
		{ /* nothing after is visible */
		cpVisi1 = cpNil;
		cpLim = CpMacDocEdit(doc);
		}
	else
		{
		cpVisi1 = CpMin(vcpFetch, CpMacDocEdit(doc));
		cpLim = CpLimCharBlock(ww, doc, cpVisi1);
		}

/* Find cpFirst */
	cpVisi2 = CpVisiBeforeCp(ww, doc, cp);
	if (cpVisi2 == cpNil)
		{ /* nothing in front is visible */
		if (cpVisi1 != cpNil && FInTableDocCp(doc, cpVisi1))
			/* this allow ip at beginning of table immediately after hidden text */
			cpFirst = vcpFirstTableCell;
		else  if (!PwwdWw(ww)->fOutline || cpVisi1 == cpNil)
			cpFirst = cp0;
		else  /* special case for outline because cp0 may not be in edl at all */
			{
			CachePara(doc, cpVisi1);
			cpFirst = caPara.cpFirst;
			}
		}
	else
		{
		/* potential cpVisi */
		cpVisi2 = CpFirstSty(ww, doc, cpVisi2, styCRLF, fFalse);
		/* cpLim of the previous CharBlock is the cpFirst of this CharBlock */
		cpFirst = CpLimCharBlock(ww, doc, cpVisi2);
		if (cpFirst > cp)
			{
			cpLim = cpFirst;
			if ((cpVisiBefore = CpVisiBeforeCp(ww, doc, cpVisi2)) == cpNil)
				cpFirst = cp0;
			else 		
				{
				cpVisiBefore = CpFirstSty(ww, doc, cpVisiBefore, styCRLF, fFalse);
				cpFirst = CpLimCharBlock(ww, doc, cpVisiBefore);
				}
			}
		}

/* Determine cpVisi */

	if (cpFirst != cpLim)
		{
		if (cpVisi1 != cpNil && cpVisi1 >= cpFirst && cpVisi1 < cpLim)
			*pcpVisi = cpVisi1;
		else  if (cpVisi2 != cpNil && cpVisi2 >= cpFirst && cpVisi2 < cpLim)
			*pcpVisi = cpVisi2;
		else
			*pcpVisi = cpNil;
		}
	else
		{
		*pcpVisi = cpNil;
		}

/* Got ca */
	Assert(cpFirst != cpNil);
	pca->doc = doc;
	pca->cpFirst = cpFirst;
	pca->cpLim = cpLim;
	Assert(cpFirst <= cpLim);

}


#endif



/* %%Function:FCaIsGraphics %%Owner:bobz */
FCaIsGraphics(pca, ww, pcpImport)
struct CA *pca;
int ww;
/* pcpImport is a "curious tristate". if NULL, it is unused, else we return
	the chPicture CP or cpNil through it. */
CP *pcpImport;
{
	struct CA caT;

	if (pcpImport != NULL)
		*pcpImport = cpNil;  /* so it always has a value */

	/* note: this wipes out any FetchCp's done earlier */
	if (pca->cpFirst + 1 == pca->cpLim)
		{
		if (ChFetch(pca->doc, pca->cpFirst, fcmChars + fcmProps) ==
				chPicture && vchpFetch.fSpec)
			{
			return (fTrue);
			}
		}

#ifdef WIN
	/* import field: if pcpImport is not Null, check to see if the ca is
		a full fltImport field selection in result mode that encompasses
		only the field. We are special casing that as a graphic selection
		for tiff files. Return true or false in *pcpImport
	*/
		{
		int ifld;
		struct FLCD flcd;

		if (pcpImport == NULL)
			return (fFalse);

		ifld = IfldFromDocCp (pca->doc, pca->cpFirst, fTrue);
		if (ifld == ifldNil)
			goto LRetNoGr;

		GetIfldFlcd (pca->doc, ifld, &flcd);
		if (flcd.flt != fltImport)
			goto LRetNoGr;

		/* whole field visible and in result mode? */
		if (pca->cpFirst == flcd.cpFirst &&
				pca->cpLim == flcd.cpFirst + flcd.dcpInst + flcd.dcpResult &&
				/* field itself is visible   */
		(CpVisibleCpField(pca->doc, pca->cpFirst, ww, fTrue) == pca->cpFirst) &&
				FShowResultPflcdFvc (&flcd, ww /*fvcScreen*/))
			{
			*pcpImport = flcd.cpFirst + flcd.dcpInst; /* cp of chPic char */
			/* if error in field, will not have fspec chpicture, so
			not a graphics selection. Recursive call. */
			if (!FCaIsGraphics(PcaSet(&caT, pca->doc, *pcpImport, *pcpImport + 1), ww, NULL))
				{
				*pcpImport = cpNil;
				return (fFalse);
				}

			return (fTrue);
			}

		/* fall through from above intentional */
		}
#endif

LRetNoGr:
	/* *pcpImport set to  cpNil at start */
	return (fFalse);
}


#ifdef WIN
/* Mac simply returns cp */
/* %%Function:CpVisiForStyPara %%Owner:chic */
CP CpVisiForStyPara(ww, doc, cp)
int ww;
int doc;
CP cp;
{
/* return the first visible cp that you can do a CpLimSty of styPara on */

	return CpFirstSty(ww, doc, CpLimSty(ww, doc, cp, styChar, fFalse), styCRLF, fTrue);
}


#endif


/* E N S U R E  S E L  I N  W W */
/* Check if pselActive is in wwCur */
/* If not, select first cp in ww.  
	fExtend is true iff we want to keep the cpAnchor off screen.
*/

/* %%Function:EnsureSelInWw %%Owner:rosiep */
EnsureSelInWw(fExtend, psel)
BOOL fExtend;
struct SEL *psel;
{
	int itc, itcLim; 
	struct PT pt;
	CP cpFirst;
	
#ifdef WIN
	{
	extern BOOL vfRecording, fElActive;
	/* Don't do this for for macros so they will be a little more
		deterministic... */
	if (vfRecording || fElActive)
		return;
	}
#endif

	if ((*hwwdCur)->fPageView)
		return; 
	if ((*hwwdCur)->fDirty)
		UpdateWw(wwCur, fFalse);
	cpFirst = PdrGalley(*hwwdCur)->cpFirst; 
	if (psel->cpFirst <= cpFirst && psel->cpLim > cpFirst)
		return; 
	/* if either the beginning... */
	if (FCpVisible(wwCur, psel->doc, psel->cpFirst, (psel->fIns) ? psel->fInsEnd : fFalse, fFalse, fTrue))
		return;
	if (!psel->fIns)
		{
		/* ...or the end of the selection is displayed, do nothing */
		if (FCpVisible(wwCur, psel->doc, psel->cpLim, fTrue, fFalse, fTrue))
			return;
		}
	

	cpFirst = CpMin(PdrGalley(PwwdWw(wwCur))->cpFirst,
				CpMacDocEdit(psel->doc));

	if (fExtend)
		MakeExtendSel(psel, cpFirst, cpFirst, maskFDoAdjust /*grpf*/);
	else
		SelectIns(psel, cpFirst);
}
