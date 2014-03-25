/* O U T L I N E . C */
/* this module should contain the cmd functions used by outlining that may
	also be used when not in outline mode.
*/

#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "prm.h"
#include "sel.h"
#include "outline.h"
#include "format.h"
#include "ch.h"
#include "error.h"
#define USEBCM
#include "cmd.h"
#include "debug.h"
#ifdef MAC
/* to be removed if FStillDownReplay can be placed in word.h */
#include "mac.h"
#include "iconbar.h"
#endif /* MAC */
#ifdef WIN
#include "ibdefs.h"
#include "idle.h"
#include "help.h"
#endif


extern struct UAB       vuab;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA        caTap;
extern struct CA		caTable;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern int              fcmFetch;
extern struct MERR      vmerr;
extern struct SEL       selCur;
extern int              wwCur;
extern struct WWD       **hwwdCur;
extern struct SCI       vsci;
extern struct FLI       vfli;
extern struct DOD       **mpdochdod[];
extern struct PREF      vpref;
extern int              vfSeeSel;
extern BOOL             vfDoubleClick;
extern struct WWD **    mpwwhwwd[];
extern struct FTI       vfti;
extern CP               vcpFetch;
extern CHAR             rgchEop[];

#ifdef WIN
extern IDF		vidf;
extern int		icnOtl;
extern HCURSOR		vhcOtlVert;
extern HCURSOR		vhcOtlHorz;
extern int		vcConseqScroll;
#endif

extern int		wwMac;


struct CAP              vcap;

#ifndef JR
/* C L E A R  C A P */
/* clears vcap cache */
/* %%Function:ClearCap %%Owner:davidlu */
EXPORT ClearCap()
{
	vcap.doc = docNil;
}


/* L V L  F R O M  I P A D */
/* returns the effective lvl of PAD at ipad.
For fBody entries, level is given by previous !fBody PAD .lvl + 1, if any
else effective lvl is 1.
Cache vcap is used to reduce backward search. Cache must be cleared before
any series of calls to this proc.
If fInsert is set, the level is calculated for an imaginary fBody entry
at ipad.
*/
/* %%Function:LvlFromIpad %%Owner:davidlu */
LvlFromIpad(doc, ipad, fInsert)
int doc, ipad;
{
	int lvl;
	struct PLCPAD **hplcpad = PdodDoc(doc)->hplcpad;
	struct PAD pad;
	int ipadT;

	if (fInsert)
		if (--ipad < 0) return 1;
	GetPlc(hplcpad, ipad, &pad);
	if (pad.fBody)
		{
/* calculate effective lvl */
		lvl = 1;
		for (ipadT = ipad - 1; ipadT >= 0; ipadT--)
			{
			GetPlc(hplcpad, ipadT, &pad);
			if (!pad.fBody && !pad.fInTable)
				{
				lvl = pad.lvl + 1;
				break;
				}
			else  if (ipadT == vcap.ipad && vcap.doc == doc)
				return vcap.lvl;
			}
		vcap.doc = doc;
		vcap.ipad = ipad;
		return vcap.lvl = lvl;
		}
	else
		return(pad.lvl + fInsert);
}



/* C P  B A C K  V I S I B L E  O U T L I N E */
/* if cp is not visible WRP outlines, scan back until it is or 0 */
/* %%Function:CpBackVisibleOutline %%Owner:davidlu */
CP CpBackVisibleOutline(doc, cp)
CP cp;
{
	struct PLCPAD **hplcpad;
	if (hplcpad = PdodDoc(doc)->hplcpad)
		{
/* search backwards for next visible paragraph */
		int ipad = IInPlc(hplcpad, cp);
		struct PAD pad;
		while (ipad >= 0)
			{
			GetPlc(hplcpad, ipad, &pad);
			if (pad.fShow) break;
			--ipad;
			}
		cp = ipad < 0 ? cpNil :
				CpPlc(hplcpad, ipad);
		}
	return cp;
}


/* C M D  M O V E  U P  D O W N */
/* move the paras containing selCur and following invisible
	sub-paras up or down by one visible para.
	Following styOutline: go to next paragraph with same or lower level.
	If not in outline mode: just go to next paragraph.
*/
/* %%Function:CmdMoveUpDn %%Owner:davidlu */
CmdMoveUpDn(fUp)
BOOL fUp;
{
	CP cpDest;
	struct CA caFrom, caT;
	int fTableSel;

	caFrom = selCur.ca;
/* figure out what to move */
	ExpandOutlineCa(&caFrom, fFalse);

/* find the place to put it */
	if (fUp)
		{
		if (caFrom.cpFirst == cp0)
			{
LBeep:                  
			Beep();
			return(cmdError);
			}
		CachePara(caFrom.doc, caFrom.cpFirst - 1);
		cpDest = caPara.cpFirst;
		if (selCur.sty >= styOutline)
			cpDest = CpFirstSty(selCur.ww, caFrom.doc,
					caFrom.cpFirst, selCur.sty, fFalse);
/* if in outline mode and paragraph is invisible, find visible para */
		cpDest = CpBackVisibleOutline(caFrom.doc, cpDest);
		if (cpDest == cpNil) cpDest = cp0;
		}
	else  /* ! fUp */		
		{
/* find the end of the next para including invisible sub paras */
		cpDest = caFrom.cpLim;
		if (caFrom.cpLim >= CpMacDoc(caFrom.doc)) goto LBeep;
		if (selCur.sty >= styOutline)
			cpDest = CpLimSty(selCur.ww, caFrom.doc, caFrom.cpLim, selCur.sty, fFalse);
/* extend to invisible sub-text */
		ExpandOutlineCa(PcaPoint(&caT, caFrom.doc, cpDest), fFalse);
		cpDest = caT.cpLim;
		}
	return CmdMoveUpDn1(&caFrom, cpDest, fUp ? ucmOtlUp : ucmOtlDown);
}


/* C M D  M O V E  U P  D O W N  1 */
/* same as above, except cpDest is explicit */
/* %%Function:CmdMoveUpDn1 %%Owner:davidlu */
CmdMoveUpDn1(pcaFrom, cpDest, ucm)
struct CA *pcaFrom; 
CP cpDest;
{
	int doc = pcaFrom->doc;
	struct CA caTo, caT;
	CP dcp;
	int fTableSel;
	int fDirtyAfterMove;

#ifdef WIN
	if (!FLegalSel(pcaFrom->doc, pcaFrom->cpFirst, pcaFrom->cpLim) ||
			/* for WIN we can't allow chSect to be the last para mark
	(not ccpEop wide)
			*/
	!FDeleteableEop(pcaFrom, rpkNil, NULL) ||
			!FDeleteableEop(PcaPoint(&caTo, pcaFrom->doc, cpDest), rpkOutline, pcaFrom))
		{
		Beep();
		/* To force the redraw of the highlight. */
		InvalCp(pcaFrom);
		return cmdError;
		}

#endif
	if ((FInTableDocCp(doc, CpMax(cp0, pcaFrom->cpLim - ccpEop)) &&
			cpDest > CpMacDocEdit(doc)) ||
			(pcaFrom->cpLim >= CpMacDoc(doc) && pcaFrom->cpFirst != 0 &&
			FInTableDocCp(doc, pcaFrom->cpFirst - 1)))
		{
		FetchCpAndPara(doc, CpMacDocEdit(doc), fcmProps);
		if (!FInsertRgch(doc, CpMacDocEdit(doc), rgchEop, cchEop, &vchpFetch, &vpapFetch))
			return cmdNoMemory;
		DirtyOutline(doc);
		}
/* if the character in front of a table is included within the first paragraph
	of the table (can happen when a field contains a table) set fTrue so we
	can dirty the hplcpad. */
	fDirtyAfterMove = (!FInTableDocCp(doc, CpMax(cp0, selCur.cpFirst-1)) && vpapFetch.fInTable);

	FSetUndoBefore(ucm, uccMove);
	dcp = cpDest - ((cpDest <= pcaFrom->cpFirst) ? 
			pcaFrom->cpFirst : pcaFrom->cpLim);
	TurnOffSel(&selCur);
	if (!FMoveCopyFromTo(fTrue, fWin /* fRM */, pcaFrom, &caTo, fTrue))
		return cmdNoMemory;
/* displace the selection by the amount the text is displaced by */
	PcaSet(&caT, selCur.doc, caTo.cpFirst, caTo.cpLim);

	if (fDirtyAfterMove)
		DirtyOutline(doc);
	fTableSel = fFalse;
	if (FInTableDocCp(selCur.doc, caT.cpFirst))
		fTableSel = FCaInTable(&caT);

	if (fTableSel)
		SelectRow(&selCur, caT.cpFirst, caT.cpLim);
	else
		{
		Select(&selCur, caT.cpFirst, caT.cpLim);
		TurnOnSel(&selCur);
		}
	SetUndoAfter(&caTo);
	SetUndoAfterMove(pcaFrom, wwCur);
	vuab.fOutlineInval = fTrue; 
	Mac( SetAgainUcm(ucm, ascFormat) );
	vfSeeSel = fTrue;
	return cmdOK;
}


/* E X P A N D  O U T L I N E  C A */
/* expands ca to integral paragraphs and also includes invisible
sub text immediately following.
*/
/* %%Function:ExpandOutlineCa %%Owner:davidlu */
ExpandOutlineCa(pca, fLeftRight)
struct CA *pca;
int fLeftRight;	/* fTrue for all visible subtext */
{
	CP cpT;
	struct PLCPAD *hplcpad;
/* this expands *pca to paragraph bounds */
	CacheParaCa(pca);
	cpT = caPara.cpFirst;
	if (vpapFetch.fInTable)
		{
		hplcpad = PdodDoc(pca->doc)->hplcpad;
		AssertH(hplcpad);
		cpT = CpPlc(hplcpad, max(IInPlcCheck(hplcpad, cpT),0));
		CachePara(pca->doc, cpT);
		cpT = caPara.cpFirst;
		}
	pca->cpFirst = CpMin(cpT, pca->cpFirst);
	CachePara(pca->doc, CpMax(pca->cpFirst, pca->cpLim - 1));
	pca->cpLim = CpMax(caPara.cpLim, pca->cpFirst);
	pca->cpLim = CpLimSubText(pca, fLeftRight, fLeftRight);

	if (!FLegalSel(pca->doc, pca->cpFirst, pca->cpLim))
		{
		AssureLegalSel(pca);
		ExpandOutlineCa(pca, fLeftRight);
		}

}


/* C P  L I M  S U B  T E X T */
/* returns the cpLim of the visible or invisible sub-text following
selCur or visible body text following selCur (must pass true,true).
visi    body
false   false
	Expands to following invisible sub text (expanding selection)
true    false
	Expands to following sub text, visible or invisible for -
true    true
	Expands to following visible body text, for left/right invalidation
*/
/* %%Function:CpLimSubText %%Owner:davidlu */
CP CpLimSubText(pca, fVisi, fBody)
struct CA *pca;
BOOL fVisi, fBody;
{
	struct PLCPAD **hplcpad;
	int ipad, ipadMac, lvl;
	int doc = pca->doc;
	struct PAD pad;

/* some outlining commands may be executed from the keyboard even
when outlining mode is not on. */
	if ((hplcpad = PdodDoc(doc)->hplcpad) == 0)
		return pca->cpLim;
/* find i/ppad of first para following the selection */
	ipadMac = IpadMac(hplcpad);
/* find lvl of first item */
	ipad = IInPlc(hplcpad, pca->cpFirst);
	GetPlc(hplcpad, ipad, &pad);
	lvl = pad.lvl;
	if (fVisi && fBody && ipad != 0)
		{
/* expand to previous item for level change invalidation (its +/- mark
may have to be changed */
		GetPlc(hplcpad, ipad - 1, &pad);
		if (!pad.fInTable && !pad.fBody)
			pca->cpFirst = CpPlc(hplcpad, ipad - 1);
		}
	ipad = IInPlc(hplcpad, CpMax(pca->cpFirst, pca->cpLim - ccpEop)) + 1;
	while (ipad < ipadMac)
		{
		GetPlc(hplcpad, ipad, &pad);
		if (!fBody)
			{
/* include visible/invisible sub-text */
/* fVisi => dont care, !fVisi => fShow must be 0 */
			if (fVisi)
				{
				if (pad.lvl <= lvl) break;
				}
			else  if (pad.fShow || pad.lvl < lvl)
				break;
			}
		else  if (!pad.fBody) break;
		++ipad;
		}
	return CpPlc(hplcpad, ipad);
}


/* C M D  O U T L I N E  U C M */
/* %%Function:CmdOutlnUcm %%Owner:davidlu */
CmdOutlnUcm(ucm)
int ucm;
{
	struct CA caOp;
	int cmd;

	if (PdodDoc(selCur.doc)->fShort)
		{
		Beep();
		return cmdError;
		}

	if (!FUpdateHplcpad(selCur.doc))
		return cmdNoMemory;
	switch (ucm)
		{
	case ucmOtlDown:
		cmd = CmdMoveUpDn(fFalse);
		break;
	case ucmOtlUp:
		cmd = CmdMoveUpDn(fTrue);
		break;
	default:
		caOp = selCur.ca;
		cmd = CmdOutlnUcm1(ucm, &caOp, selCur.sty, 0);
		break;
		}
	Win(MakeSelCurVisi(fFalse /*fForceBlockToIp*/));
	return cmd;
}


/* C M D  O U T L I N E  U C M  1 */
/* as above, but useful for collapsing, expanding or level change
for the operand pcaOp.
ucmOtlLeftRightBody is a dummy ucm used for level change by val,
as well as internally in the procedure.
*/
/* %%Function:CmdOutlnUcm1 %%Owner:davidlu */
CmdOutlnUcm1(ucm, pcaOp, sty, val)
int ucm; 
struct CA *pcaOp; 
int sty, val;
{
	int lvl, lvlPlusMinus = -1, lvlLastSet, lvlBody, lvlFirst;
	int ipad, ipadFirst, ipadLim;
	int ww;
	BOOL fBody;
	struct PLCPAD **hplcpad;
	int fLeft, sprm;
	int cstcStd;
	BOOL fAllShow = fFalse;
	BOOL fShowAll = fFalse;
	int cmd;
	int doc = pcaOp->doc;
	struct PAD pad;
	struct CA caT; /* the operand */
	extern KMP **vhkmpOutline;
	struct DOD **hdod;

#ifdef WIN
	if (!FLegalSel(pcaOp->doc, pcaOp->cpFirst, pcaOp->cpLim))
		{
		Beep();
		/* To force the redraw of the highlight. */
		InvalCp(pcaOp);
		return cmdError;
		}
#endif
	if (!FUpdateHplcpad(doc))
		return cmdNoMemory;
	if (vmerr.fMemFail) return cmdError;

/* invalidation is effective through CpOutlineAdvance */
	(*hwwdCur)->fDirty = fTrue;
	hplcpad = PdodDoc(doc)->hplcpad;
/* at this point hplcpad is expected to be up to date. */

	sprm = sprmPIncLvl;
	Mac(SetAgainUcmVal(ucm, ascFormat, accOutline, val));

	switch (ucm)
		{
	case ucmOtlLeft:
/* < icon on the outline icon bar.
When applied to a single paragraph, it will change body text into heading
of lvl of the previous non-body paragraph.
*/
		fLeft = 1;
		val = -1;
		goto LLeftRight;
	case ucmOtlRight:
		fLeft = 0;
		val = 1;
LLeftRight:
		if (sty >= styOutline)
/* adjust selection type */
			selCur.sty = max(0, min(lvlMax - 1, selCur.sty - styOutline + val))
					+ styOutline;
		ExpandOutlineCa(pcaOp, fFalse);

		ipadFirst = IInPlc(hplcpad, pcaOp->cpFirst);
		ipadLim = IInPlc2(hplcpad, pcaOp->cpLim, ipadFirst);
		for (ipad = ipadFirst; ipad < ipadLim; ipad++)
			{
			GetPlc(hplcpad, ipad, &pad);
			if (!pad.fBody) goto LLeftRightBody;
			}
/* selection is completely body text. Set appropriate level */
		ClearCap();
		lvlBody = min(LvlFromIpad(doc, ipadFirst, fFalse) - fLeft, lvlMax - 1);
		Assert(lvlBody >= 0);
		if ((cmd = CmdOutlineLeftRight(pcaOp, sprmPStc,
				stcLevLast - lvlBody, ucm)) != cmdOK)
			return cmd;
		FEnsureStcDefined(doc, stcLevLast - lvlBody, &cstcStd);
		ucm = ucmOtlBodyLeftRight;
		goto LAdjustLvl;
	case ucmOtlVal:
		/* special entry to apply caOp a level change by val */

		ExpandOutlineCa(pcaOp, fFalse);
/* have: caOp, sprm/val to apply */
LLeftRightBody:
		if ((cmd = CmdOutlineLeftRight(pcaOp, sprm, val, ucm)) != cmdOK)
			return cmd;
		goto LAdjustLvl;

	case ucmOtlBody:
/* >> icon on the outline icon bar changing selection to body text */
		ExpandOutlineCa(pcaOp, fFalse);
		sprm = sprmPStc;
		val = stcNormal;
		goto LLeftRightBody;

/* val is the lvl to be applied to the operand */
	case ucmOtlLvl:
		ExpandOutlineCa(pcaOp, fFalse);
		sprm = sprmPStc;
		val = stcLevLast - val;
		FEnsureStcDefined(doc, val, &cstcStd);
		goto LLeftRightBody;

#ifdef MAC
	case ucmOtlSHFmt:
		vpref.fOtlShowFormat = !vpref.fOtlShowFormat;
		goto LInvalAllDocs;
	case ucmOtlSHBody:
		vpref.fOtlShowBody = !vpref.fOtlShowBody;
LInvalAllDocs:
		for (doc = docMinNormal; doc < docMax; doc++)
			if ((hdod = mpdochdod[doc]) != hNil && !(*hdod)->fShort)
				InvalOutlineWws(doc, fFalse);
		return cmdOK;
#else
	case ucmOtlSHBody:
		ToggleESF(wwCur);
		return cmdOK;
#endif
	case ucmOtlAll:
		fShowAll = fTrue;
		/* FALL THROUGH */
	default: /* ucmOtl1, etc., ucmOtlAll: */
		Mac( InvalOutlineWws(selCur.doc, fFalse); )
				Win( InvalOutlineWws(selCur.doc) );
		StartLongOp();
		lvlLastSet = fShowAll ? 100 : LvlFromUcm(ucm);
		fAllShow = fTrue;
		for (;;)
			{
			int ipadMac = IpadMac(hplcpad);
			for (ipad = 0; ipad < ipadMac; ipad++)
				{
				GetPlc(hplcpad, ipad, &pad);
				fAllShow &= pad.fShow;
				pad.fShow = pad.fBody || pad.fInTable ?
						fShowAll : pad.lvl <= lvlLastSet;
				PutPlcLast(hplcpad, ipad, &pad);
				}
			if (fAllShow && ucm == ucmOtlAll)
				{
/* if all paras are already shown, show all except for body text */
				fShowAll = fFalse;
				lvlLastSet = lvlMax - 1;
				ucm = WinMac(icn1, ucmOtl1); /* this prevents repeats and uses lvlLastSet */
				continue;
				}
			break;
			}
#ifdef WIN
			{
			int	lvlNew;

			Assert(icn9 + 1 == icnAll);
			if (icnOtl == icnAll && fAllShow)
				{
				lvlNew = icn9 - icn1;
				}
			else
				{
				lvlNew = icnOtl - icn1;
				}
			PdodDoc(doc)->lvl = lvlNew;
#ifdef WIN23
			if (vsci.fWin3Visuals)
				UpdateOutlineIconBar();
#endif /* WIN23 */
			}
#endif
		vfSeeSel = fTrue;
		break;
/* can't undo these - they're view modes */
	case ucmOtlMinusDirect:
		ExpandOutlineCa(pcaOp, fFalse);
		InvalCp(PcaSetDcp(&caT, doc, CpMax(pcaOp->cpFirst - 1, cp0), (CP)1));
		goto LAdjustLvl;
	case ucmOtlPlus:
		lvlPlusMinus = 100;
		fAllShow = fTrue;
	case ucmOtlMinus:
		/* for Minus: lvlPlusMinus = -1; */
/* expand to visible sub-text */
		pcaOp->cpLim = CpLimSubText(pcaOp, fTrue, fFalse);
		ExpandOutlineCa(pcaOp, fFalse);
/* invalidate so that splat lines are refreshed */
		InvalCp(pcaOp);
		if (sty >= styPara)
			ucm = ucm == ucmOtlPlus ? ucmOtlMinusDirect :
					ucmOtlMinusAll;

LAdjustLvl:
		Win(PdodDoc(doc)->lvl = lvlNone);
		ipadFirst = IInPlc(hplcpad, pcaOp->cpFirst);
		ipadLim = IInPlc2(hplcpad, pcaOp->cpLim, ipadFirst);
		GetPlc(hplcpad, ipadFirst, &pad);
		lvlFirst = pad.lvl;

/*
Plus Plan:
	for all paras: find min lvlPlusMinus > lvlFirst that is not shown
Minus Plan:
	for all paras: find max lvlPlusMinus > lvlFirst that is shown
Both:   set/reset fShow for paras with lvl == lvlPlusMinus
*/
LPass2:
		for (ipad = ipadFirst; ipad < ipadLim; ipad++)
			{
			GetPlc(hplcpad, ipad, &pad);
			lvl = pad.lvl;
			fBody = pad.fBody;
			switch (ucm)
				{
			case ucmOtlPlus:
				if (!pad.fShow && lvl >= lvlFirst)
					lvlPlusMinus = min(lvlPlusMinus, lvl);
				continue;
			case ucmOtlMinus:
				if (pad.fShow && lvl > lvlFirst)
					lvlPlusMinus = max(lvlPlusMinus, lvl);
				continue;
/* the following cases adjust pad's after level changes */
			case ucmOtlRight:
			case ucmOtlVal:
				if (pad.fInTable)
					{
					EnsureTableStc(doc, hplcpad, ipad);
					break;
					}
				if (!fBody && lvl < lvlMax - val)
					FEnsureStcDefined(doc,
							pad.stc = stcLevLast - (pad.lvl = lvl + val),
							&cstcStd);
				else  if (ipadLim == ipadFirst + 1)
					{
LBeep:
					Beep();
					SetUndoNil();
					}
				break;
			case ucmOtlBody:
/* beep if sigle para, already body */
				if (ipadLim == ipadFirst + 1 && fBody)
					goto LBeep;
				pad.fBody = fTrue;
				pad.lvl = lvlMax;
				pad.stc = stcNormal;
				break;
			case ucmOtlLeft:
				if (pad.fInTable)
					{
					EnsureTableStc(doc, hplcpad, ipad);
					break;
					}
				if (!fBody && lvl > 0)
					FEnsureStcDefined(doc,
							pad.stc = stcLevLast - (pad.lvl = lvl - 1),
							&cstcStd);
				else  if (ipadLim == ipadFirst + 1)
					goto LBeep;
				break;
			case ucmOtlBodyLeftRight:
				if (pad.fInTable)
					{
					EnsureTableStc(doc, hplcpad, ipad);
					break;
					}
				pad.fBody = fFalse;
				pad.stc = stcLevLast - (pad.lvl = lvlBody);
				break;
			case ucmOtlLvl:
				if (pad.fInTable)
					{
					EnsureTableStc(doc, hplcpad, ipad);
					break;
					}
				pad.fBody = fFalse;
				pad.stc = val;
				pad.lvl = stcLevLast - val;
				break;
			case ucmOtlPass2:
				if (lvl != lvlPlusMinus)
					break;
				/* fall through */
			case ucmOtlMinusDirect: /* also PlusAll */
				pad.fShow = fAllShow;
				break;
			case ucmOtlMinusAll:
				if (lvl > lvlFirst)
					pad.fShow = fAllShow;
				break;
#ifdef DEBUG
			default:
				Assert(fFalse);
				break;
#endif
				}
			PutPlc(hplcpad, ipad, &pad);
			}
		if (ucm == ucmOtlPlus || ucm == ucmOtlMinus)
			{
			ucm = sty >= styPara ?
					(ucm == ucmOtlPlus ? ucmOtlMinusDirect :
					ucmOtlMinusAll)
: 
					ucmOtlPass2;
			goto LPass2;
			}
		}
	return cmdOK;
}


/* E N S U R E  T A B L E  S T C */
/* needed because plcpad has only one entry per table, but there many
be many paragraphs */
/* %%Function:EnsureTableStc %%Owner:davidlu */
EnsureTableStc(doc, hplcpad, ipad)
{
	int cstcStd;
	CP cp = CpPlc(hplcpad, ipad);
	CP cpLim = CpPlc(hplcpad, ipad + 1);
	do
		{
		CachePara(doc, cp);
		cp = caPara.cpLim;
		FEnsureStcDefined(doc, vpapFetch.stc, &cstcStd);
		}
	while (cp < cpLim);
}


/* C M D  O U T L I N E  L E F T  R I G H T */
/* %%Function:CmdOutlineLeftRight %%Owner:davidlu */
CmdOutlineLeftRight(pcaOp, sprm, val, ucm)
struct CA *pcaOp;
{
	char grpprl[2];
	CP dcp;
	struct CA ca, caInval;

	grpprl[0] = sprm;
	grpprl[1] = val;
	ca = *pcaOp;
	Win(AssureLegalSel(&ca));
	caInval.doc = ca.doc;
	ExpandCaSprm(&ca, &caInval, grpprl);
	caInval.cpLim = CpLimSubText(&caInval, fTrue, fTrue);
	if (!FSetUndoB1(ucm, uccFormat, &ca))
		return cmdCancelled;
/* inval whole first paragraph, even though prm was applied starting only
at cpLim - 1 to save prc space.
Also invalidate visible body text following because its indentation may
change with the change of the operand text */
	InvalCp(&caInval);
#ifdef WIN
/*  dirty the result portion of any enclosing field */
	InvalText(&ca, fFalse /* fEdit */);
/* invalidate all subsequent auto-numbered paragraphs (seqlevs) */
	if (PdodDoc(ca.doc)->hplcfld != hNil && !PdodDoc(ca.doc)->fHasNoSeqLev)
		PdodDoc(ca.doc)->fInvalSeqLev = vidf.fInvalSeqLev = fTrue;
#endif /* WIN */
	ApplyGrpprlCa(grpprl, 2, &ca);
	if (selCur.fIns)
		GetSelCurChp(fTrue);
	vuab.fOutlineInval = fTrue;	/* force CpLimSubText on undo */
	SetUndoAfter(0);
	return cmdOK;
}


#ifdef MAC
/* C M D  O U T L I N E  A C T I O N 
*  Perform outline ucm action.  Called by FExecUcm.
*/
/* %%Function:CmdOutlineAction %%Owner:NOTUSED */
CmdOutlineAction(pcmb)
CMB *pcmb;
{
	return CmdOutlnUcm(pcmb->ucm);
}


#endif



/* F  C A  I N  T A B L E */
/* Returns true if the ca lies entirely within one table */
/* %%Function:FCaInTable %%Owner:davidlu */
FCaInTable(pca)
struct CA *pca;
{
	CP cpCur, cpLim = pca->cpLim;
	int doc = pca->doc;

	cpCur = pca->cpFirst;
	if (!FInTableDocCp(doc, CpMax(cpLim - 1, cpCur)))
		return fFalse;

	while (cpCur < cpLim)
		{
		if (!FInTableDocCp(doc, cpCur))
			return fFalse;
		cpCur = caPara.cpLim;
		}

	return fTrue;
}


/* %%Function:CmdPromote %%Owner:davidlu */
CMD CmdPromote(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmPromote));
}


/* %%Function:CmdDemote %%Owner:davidlu */
CMD CmdDemote(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmDemote));
}


/* %%Function:CmdMoveUp %%Owner:davidlu */
CMD CmdMoveUp(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmMoveUp));
}


/* %%Function:CmdMoveDown %%Owner:davidlu */
CMD CmdMoveDown(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmMoveDown));
}


/* %%Function:CmdConvertToBody %%Owner:davidlu */
CMD CmdConvertToBody(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmConvertToBody));
}


/* %%Function:CmdExpand %%Owner:davidlu */
CMD CmdExpand(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmExpand));
}


/* %%Function:CmdCollapse %%Owner:davidlu */
CMD CmdCollapse(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmCollapse));
}


/* %%Function:CmdShowToLvl1 %%Owner:davidlu */
CMD CmdShowToLvl1(pcmb)
CMB * pcmb;
{
	icnOtl = icn1;
	return (CmdOutlnUcm(bcmShowToLevel1));
}


/* %%Function:CmdShowToLvl2 %%Owner:davidlu */
CMD CmdShowToLvl2(pcmb)
CMB * pcmb;
{
	icnOtl = icn2;
	return (CmdOutlnUcm(bcmShowToLevel2));
}


/* %%Function:CmdShowToLvl3 %%Owner:davidlu */
CMD CmdShowToLvl3(pcmb)
CMB * pcmb;
{
	icnOtl = icn3;
	return (CmdOutlnUcm(bcmShowToLevel3));
}


/* %%Function:CmdShowToLvl4 %%Owner:davidlu */
CMD CmdShowToLvl4(pcmb)
CMB * pcmb;
{
	icnOtl = icn4;
	return (CmdOutlnUcm(bcmShowToLevel4));
}


/* %%Function:CmdShowToLvl5 %%Owner:davidlu */
CMD CmdShowToLvl5(pcmb)
CMB * pcmb;
{
	icnOtl = icn5;
	return (CmdOutlnUcm(bcmShowToLevel5));
}


/* %%Function:CmdShowToLvl6 %%Owner:davidlu */
CMD CmdShowToLvl6(pcmb)
CMB * pcmb;
{
	icnOtl = icn6;
	return (CmdOutlnUcm(bcmShowToLevel6));
}


/* %%Function:CmdShowToLvl7 %%Owner:davidlu */
CMD CmdShowToLvl7(pcmb)
CMB * pcmb;
{
	icnOtl = icn7;
	return (CmdOutlnUcm(bcmShowToLevel7));
}


/* %%Function:CmdShowToLvl8 %%Owner:davidlu */
CMD CmdShowToLvl8(pcmb)
CMB * pcmb;
{
	icnOtl = icn8;
	return (CmdOutlnUcm(bcmShowToLevel8));
}


/* %%Function:CmdShowToLvl9 %%Owner:davidlu */
CMD CmdShowToLvl9(pcmb)
CMB * pcmb;
{
	icnOtl = icn9;
	return (CmdOutlnUcm(bcmShowToLevel9));
}


/* %%Function:CmdExpandAll %%Owner:davidlu */
CMD CmdExpandAll(pcmb)
CMB * pcmb;
{
	icnOtl = icnAll;
	return (CmdOutlnUcm(bcmExpandAll));
}


/* %%Function:CmdToggleEllip %%Owner:davidlu */
CMD CmdToggleEllip(pcmb)
CMB * pcmb;
{
	return (CmdOutlnUcm(bcmToggleEllip));
}


#ifdef ESFICON
/* %%Function:CmdShowFormat %%Owner:NOTUSED */
CMD CmdShowFormat()
{
	return (CmdOutlnUcm(bcmShowFormat));
}


#endif /* ESFICON */


/* I N V A L  O U T L I N E  W W S */
/* Invalidates all outline views, due to change in preferences of
	Show/Hide Formatting/Body Text
*/
/* %%Function:InvalOutlineWws %%Owner:davidlu */
InvalOutlineWws(doc)
int doc;
{
	int ww;
	for (ww = WwDisp(doc, wwNil, fFalse); ww != wwNil; ww = WwDisp(doc, ww, fFalse))
		{
		if (PwwdWw(ww)->fOutline)
			/* trash window */
			TrashWw(ww);
		}
}


/* L V L  F R O M  D O C  C P */
/* returns raw lvl of pad at cp in doc */
/* %%Function:LvlFromDocCp %%Owner:davidlu */
LvlFromDocCp(doc, cp)
int doc; 
CP cp;
{
	struct PLCPAD **hplcpad = PdodDoc(doc)->hplcpad;
	struct PAD pad;

	Assert(hplcpad);
	GetPlc(hplcpad, IInPlc(hplcpad, cp), &pad);
	return (pad.lvl);
}


#endif /* !JR */
