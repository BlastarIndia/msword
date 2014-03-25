/* S E L E C T T B . C */

#ifdef MAC
#define EVENTS
#include "toolbox.h"
#endif

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

#ifdef WIN
#include "cmdtbl.h"
#define REPEAT
#include "ruler.h"
#include "keys.h"
#include "help.h"

#define chmNil	kcNil
#endif

extern struct CA        caPara;
extern struct CA	caTap;
extern struct CA        caTable;
extern struct TCC       vtcc;

extern struct TAP       vtapFetch;
extern CP               vmpitccp[];

extern struct SEL       selCur;

extern BOOL				vfRightClick;

extern int              wwCur;
extern struct WWD       **hwwdCur;
extern CP 				vcpFirstTablePara;
extern struct PAP		vpapFetch;

#ifdef MAC
extern struct EVENT     event;
#endif /* MAC */

#ifdef WIN
extern RRF              rrf;
extern HWND             vhwndRibbon;
extern HWND             vhwndCBT;
extern struct SCI       vsci;
extern BOOL             vfTableKeymap;
extern int		         vcConseqScroll;
extern BOOL             vfHelp;
extern long             vcmsecHelp;
extern HCURSOR          vhcColumn;
extern BOOL		vfAnimate;
#endif /* WIN */



/* F  C H A N G E  S E L  T O  S T Y  C O L  */
/* return fTrue when an skSel selection should be changed to a skColumn
	selection when selection is anchored in a table */
/* %%Function:FChangeSelToStyCol %%Owner:davidlu */
FChangeSelToStyCol(psel, pcpFirst, xw, hpldr, pidr, dl, pcaAnchorCell, pcaInTable, fDrInTable)
struct SEL *psel;
CP *pcpFirst;
int xw;
struct PL **hpldr;
int *pidr;
int dl;
struct CA *pcaAnchorCell;
struct CA *pcaInTable;
int fDrInTable;
{
	CP cpScan, cpScanLim;
	int fTableSel;
	int xp;
	CP cpFirst;
	int flt;
	struct FLSS flss;
	struct DRF drfFetch;

	cpFirst = CpPlc(PdrFetch(hpldr, *pidr, &drfFetch)->hplcedl, dl);
	FreePdrf(&drfFetch);
	if ((psel->sty < styCol || psel->sty > styWholeTable) &&
			FInCa(psel->doc, cpFirst, pcaAnchorCell))
		{
		*pcpFirst = cpFirst;
		return fFalse;
		}
	if (!fDrInTable && psel->sty != styDoc)
		{
		xp = max(0, XpFromXw(hpldr, *pidr, xw));
		cpFirst = CpFromDlXp(wwCur, hpldr, *pidr, dl, xp, &flt, &flss);
		if (psel->cpAnchor < cpFirst)
			{
			cpFirst -= ccpEop;
			if (cpFirst < cp0)
				cpFirst = cp0;
			}
		}

	Assert (psel->doc == pcaInTable->doc);
	fTableSel = FParasUpToCpInTable(pcaInTable, cpFirst);
	if (fTableSel && !fDrInTable && psel->sty != styDoc)
		{
		if (!psel->fWithinCell)
			{
			*pidr = (psel->fTable) ?
					((psel->fRightward) ? psel->itcLim - 1 : psel->itcFirst) :
			psel->itcOppositeAnchor;
			}
		else
			{
			CacheTc(wwNil, psel->doc, psel->cpAnchor, fFalse, fFalse);
			*pidr = vtcc.itc;
			}
		cpFirst = CpFirstForItc(psel->doc, cpFirst, *pidr);
		}
	*pcpFirst = cpFirst;
	return (fTableSel);
}


/* F  P A R A S  U P  T O  C P  I N  T A B L E  */
/* scans from the bounds of pcaInTable to cp and returns fTrue if every
	paragraph encountered has pap.fInTable == fTrue. returns fFalse otherwise.
	the bounds of pcaInTable are maintained as we scan. */
/* %%Function:FParasUpToCpInTable %%Owner:davidlu */
FParasUpToCpInTable(pcaInTable, cp)
struct CA *pcaInTable;
CP cp;
{
	int doc = pcaInTable->doc;
	CP cpScan;

	if (FInCa(doc, cp, pcaInTable))
		return fTrue;

	if (cp < pcaInTable->cpFirst)
		{
		cpScan = pcaInTable->cpFirst - 1;
		while (cpScan >= cp)
			{
			if (!FInTableDocCp(doc, cpScan))
				return fFalse;
			pcaInTable->cpFirst = cpScan = vcpFirstTablePara;
			cpScan--;
			}
		}
	else
		{
		cpScan = pcaInTable->cpLim;
		while (cpScan <= cp)
			{
			CachePara(doc, cpScan);
			if (!FInTableDocCp(doc, cpScan))
				return fFalse;
			cpScan = pcaInTable->cpLim = caPara.cpLim;
			}
		}
	return fTrue;
}


/* %%Function:CacheTable %%Owner:davidlu */
CacheTable(doc, cp)
int doc;
CP cp;
{
	if (FInCa(doc, cp, &caTable))
		return;
	CachePara(doc, cp);
	AssertDo(FInTableDocCp(doc, cp));
	caTable.doc = doc;
	caTable.cpFirst = vcpFirstTablePara;
	caTable.cpLim = caPara.cpLim;
	FParasUpToCpInTable(&caTable, cp0);
	FParasUpToCpInTable(&caTable, cpMax);
}


/* F  D O  C O N T E N T  H I T  C O L U M N */
/* mouse down event with column select in a table.
Follow drawing motion of mouse and return fColumn selection.
*/
/* %%Function:FDoContentHitColumn %%Owner:davidlu */
FDoContentHitColumn(psel, cp, idr, pcaInTable, pdl, ppt, phpldr, pidr, pspt, pcaTapAnchor, itcAnchor, cpAnchorNew)
struct SEL *psel;
CP cp;
int idr;
struct CA *pcaInTable;
int *pdl;
struct PT *ppt;
struct PLDR ***phpldr;
int *pidr;
struct SPT *pspt;
struct CA *pcaTapAnchor;
int itcAnchor;
CP cpAnchorNew;
{
	struct PT pt;
	CP cpFirst;
	struct WWD *pwwd;
	int doc = psel->doc;
	int dl;
	struct PLDR **hpldr;
	int xp;
	struct SPT spt;
	int idrPrev = itcMax;
	int fPageView;
	int ipgd;
	CP cpFirstPrev = cpNil;
	int dysMinScroll, dysMaxScroll;
#ifdef WIN
	struct PLCEDL **hplcedl;
	int dyw;
	BOOL fForward = fFalse;
	int dlMax;
#endif
	struct DRF drfFetch;


	/* WARNING: the idr passed in may not be inside the hpldr passed in */ 

	Assert(wwCur == psel->ww);

	pwwd = *hwwdCur;
	fPageView = pwwd->fPageView;
#ifdef WIN
	dyw = pwwd->ywMac - pwwd->ywMin;
	hplcedl = pwwd->rgdr[0].hplcedl;
	Assert(hplcedl != NULL);
	dlMax = (*hplcedl)->dlMax;
	SetCapture((*hwwdCur)->hwnd);
	vcConseqScroll = 0;

/* initialize pt to something reasonable because Win version of
	FStillDownReplay can return true without setting pt, due to
	weirdness in message order */
	pt = *ppt;
#else
	dysMinScroll = dysMinAveLineSci;
	dysMaxScroll = dysMacAveLineSci;
#endif

	if (!psel->fTable)
		{
		if (psel->fWithinCell || (FInCa(psel->doc, psel->cpFirst, pcaTapAnchor) &&
				FInCa(psel->doc, CpMax(psel->cpFirst, psel->cpLim - 1), pcaTapAnchor) &&
				(psel->sty < styCol || psel->sty > styWholeTable) &&
				(psel->cpFirst > pcaTapAnchor->cpFirst || psel->cpLim < pcaTapAnchor->cpLim)))
			ColumnSelBegin(psel, cp, idr, pcaTapAnchor, itcAnchor, cpAnchorNew);
		else
			{
/* this branch is taken when we have a table selection that was extended
	outside of the table, and is now being shrunk to fit inside of the table
	again. */
			ShrinkSelToTableSty(psel, itcAnchor, idr, pcaTapAnchor);
			cpFirst = cp;
			goto LSelect;
			}
		}
	else
		{
		cpFirst = cp;

		goto LSelect;
		}

	while (FStillDownReplay( &pt, vfRightClick ))
		{
		pwwd = *hwwdCur;
#ifdef MAC
		if (pt.yw >= pwwd->ywMac)
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
				ScrollUp(wwCur, dysMinScroll, dysMaxScroll);
				pt.yp = (*hwwdCur)->ywMac - 1;
				goto DoCont1;
				}
			else  if (pt.yp < pwwd->ywMin)
				{
				pwwd = *hwwdCur;
				if (fPageView &&  pwwd->rcePage.yeTop - dypPastPageSci >= 0)
					{
					if ((ipgd = IpgdPrevWw(wwCur)) == ipgdNil)
						continue;
					SetPageDisp(wwCur, ipgd, YeBottomPage(wwCur), fFalse, fFalse);
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
				pt.yp = (*hwwdCur)->ywMin + 1;
DoCont1:                
				UpdateWw(wwCur, fFalse);
#ifdef WIN
				if (vcConseqScroll < 0x7FFF)
					{
					vcConseqScroll++;
					}
#endif
				}
			else  if (pt.xp < pwwd->xwMin &&
					FAnyDlNotInXw(wwCur, fTrue))
				{
				ScrollRight(wwCur, dxpMinScrollSci);
				pt.xp = (*hwwdCur)->xwMin;
				goto DoCont2;
				}
			else  if (pt.xp >= pwwd->xwMac &&
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
				}
		dl = DlWherePt(wwCur, &pt, &hpldr, &idr, &spt, fTrue, fFalse);
#ifdef WIN
		/* refetch the dlMax from the outter dr for ScrollDelta */
		if ((*hpldr)->hpldrBack != NULL)
			{
			Assert((*hpldr)->idrBack != idrNil);
			hplcedl = PdrFetch((*hpldr)->hpldrBack, (*hpldr)->idrBack, &drfFetch)->hplcedl;
			Assert(hplcedl != NULL);
			AssertH(hplcedl);
			dlMax = (*hplcedl)->dlMax;
			FreePdrf(&drfFetch);
			}
#endif
		if (dl != dlNil && doc == PdrFetchAndFree(hpldr, idr, &drfFetch)->doc)
			{
/* WARNING: FChangeSelToStyCol may change idr to be no longer inside the hpldr
passed to it if it return TRUE. */
			if (!FChangeSelToStyCol(psel, &cpFirst, pt.xw, hpldr, &idr, dl,
					0 /* pcaAnchorCell unused*/, pcaInTable, spt.fInTable))
				{
				*ppt = pt;
				*phpldr = hpldr;
				*pidr = idr;
				*pdl = dl;
				*pspt = spt;
				return fFalse;
				}
LSelect:
			if (psel->sty == styRow)
				{
				CpFirstTap(doc, cpFirst);
				cpFirst = caTap.cpLim - 1;
				idr = itcMax - 1;
				}
			else  if (psel->sty == styColAll || psel->sty == styWholeTable)
				{
				CacheTable(doc, cpFirst);
				*pcaInTable = caTable;
				if (psel->sty == styWholeTable)
					idr = itcMax - 1;
				cpFirst = CpFirstForItc(doc, caTable.cpLim - 1, idr);
				}
			if (cpFirst != cpFirstPrev || idr != idrPrev)
				SelectColumnTo(psel, cpFirst, idr, pcaTapAnchor, itcAnchor);
			cpFirstPrev = cpFirst;
			idrPrev = idr;
			if (psel->sty == styRow)
				psel->sk = skRows;
			}
		}
#ifdef WIN
	ReleaseCapture();
	Assert (psel->fTable);
	if (vhwndCBT)
		/* Send CBT a message explaining what we've selected */
		CBTTblSelectPsel(psel);

#ifdef NOT_NECESSARY
	If psel->fTable is always true here, this is unnecessary.  -awe
				{
		if (psel->fTable)
			CBTTblSelectPsel(psel);
		else
			CBTSelectPsel(psel);
		}
#endif /* NOT_NECESSARY */

#endif /* WIN */
	return fTrue;
}


/* S H R I N K  S E L  T O  T A B L E  S T Y  */
/* %%Function:ShrinkSelToTable %%Owner:davidlu */
ShrinkSelToTableSty(psel, itcAnchor, idr, pcaTapAnchor)
struct SEL *psel;
int itcAnchor;
int idr;
struct CA *pcaTapAnchor;
{
	CP	cpAnchor = psel->cpAnchor;
	CP cpFirstTap;
	int fTableAnchor = psel->fTableAnchor;
	int sty = psel->sty;
	int doc = psel->doc;
	int itcFirst, itcLim;
	struct CA caBlock;

	if (sty < styCol || sty > styWholeTable)
		sty = styCol;
	CachePara(doc, cpAnchor);
	caBlock = caPara;
	if (vpapFetch.fInTable && !FInTableVPapFetch(doc, caPara.cpFirst))
		caBlock.cpFirst = vcpFirstTablePara;
	if (caBlock.cpLim > psel->cpLim)
		caBlock.cpLim = psel->cpLim;
	if (caBlock.cpFirst > psel->cpLim)
		caBlock.cpFirst = psel->cpLim;

	FParasUpToCpInTable(&caBlock, psel->cpFirst);
	FParasUpToCpInTable(&caBlock, psel->cpLim - 1);
	Select1(psel, caBlock.cpFirst, caBlock.cpLim, maskSelChanged | maskTableExempt);
	psel->sk = skRows;
	psel->itcFirst = 0;
	psel->itcLim = itcMax;
	if (sty == styCol || sty == styColAll)
		{
		cpFirstTap = pcaTapAnchor->cpFirst;
		itcFirst = min(itcAnchor, idr);
		itcLim = max(itcAnchor, idr) + 1;
		SelectColumn(psel, psel->cpFirst, psel->cpLim,
				itcFirst, itcLim);
		psel->fForward = (cpFirstTap <= psel->cpFirst);
		psel->fRightward = (itcAnchor == psel->itcFirst);
		}
	else
		{
		psel->fForward = (cpAnchor <= psel->cpFirst);
		psel->fRightward = fTrue;
		}
	psel->cpAnchor = cpAnchor;
	psel->fTableAnchor = fTableAnchor;

	psel->sty = sty;
}


/* C O L U M N  S E L  B E G I N  */
/* routine converts an skSel selection to a column selection whose anchor
	cell contains the anchor cp of the selection and whose other corner
	is the cell that contains cpNew. */
/* %%Function:ColumnSelBegin %%Owner:davidlu */
ColumnSelBegin(psel, cpNew, itc, pcaTapAnchor, itcAnchor, cpAnchorNew)
struct SEL *psel;
CP cpNew;
int itc;
struct CA *pcaTapAnchor;
int itcAnchor;
CP cpAnchorNew;
{
	CP cpFirstNew;
	CP cpAnchor;
	int fToggle;
	int doc = psel->doc;
	struct CA caTapNew;

/* we don't want to toggle the current selection if the cell mark at the end
	of the sel has already been selected. In this case the selection already
	looks the way we want it to look. */
	fToggle = psel->fWithinCell || psel->fNil || 
			!FInCa(psel->doc, psel->cpFirst, pcaTapAnchor) ||
			!FInCa(psel->doc, CpMax(psel->cpFirst, psel->cpLim - 1), pcaTapAnchor);

	if (!psel->fHidden && fToggle)
		{
		TurnOffSel(psel);
		psel->fHidden = fFalse;
		}
	if (psel->fNil)
		psel->fHidden = fFalse;
	psel->fWithinCell = fFalse;

/* determine itc of cell containing cpNew.*/
	CpFirstTap(doc, cpNew);
	caTapNew = caTap;
	CacheTc(wwNil, doc, cpNew, fFalse, fFalse);
	cpFirstNew = vtcc.cpFirst;


	psel->itcFirst = min(itcAnchor, itc);
	psel->itcLim = max(itcAnchor, itc) + 1;

	psel->fRightward = (itcAnchor < itc);
	psel->fForward = (cpAnchorNew <= cpFirstNew);

	psel->sk = skColumn;
	psel->sty = styCol;

	psel->cpFirst = CpMin(caTapNew.cpFirst, pcaTapAnchor->cpFirst);
	psel->cpLim = CpMax(caTapNew.cpLim, pcaTapAnchor->cpLim);

	if (!psel->fHidden && fToggle)
		ToggleSel(psel, psel->cpFirst, psel->cpLim);
/* psel->fHidden = fFalse;	*/
	psel->cpAnchor = cpAnchorNew;
	psel->fTableAnchor = fTrue;
}




/* S E L E C T  C O L U M N  T O */
/* routine extends an existing column selection to the cell that contains
	cpNew  */
/* %%Function:SelectColumnTo %%Owner:davidlu */
SelectColumnTo(psel, cpNew, itc, pcaTapAnchor, itcCpAnchor)
struct SEL *psel;
CP cpNew;
int itc;
struct CA *pcaTapAnchor;
int itcCpAnchor;
{
	int fRightward, fRightwardNew;
	int fForward, fForwardNew;
	int fRightwardNE, fForwardNE;
	int fInSharedRows;
	int doc = psel->doc;
	struct PLDR **hpldrFirstLine;
	int dlT;
	CP cp;
	CP cpT;
	CP cpFirstCommon, cpLimCommon;
	int fChangedPage;
	struct SEL selCol, selRow;
	struct CA caTapNew, caTapFirstCommon;

	Win(InvalSelCurProps(fTrue));

/* turn into a column selection so ToggleSel will turn off the right columns.
	caller will have to make sel a row selection if that is necessary. */
	psel->sk = skColumn;

/* determine itc of cell containing cp.*/
	CpFirstTap(doc, cpNew);
	caTapNew = caTap;
	CacheTc(wwNil, doc, cpNew, fFalse, fFalse);
	cp = vtcc.cpFirst;

/* record original orientation of selection. */
	fRightward = psel->fRightward;
	fForward = psel->fForward;

/* determine desired selection orientation. */
	fRightwardNew = itc >= (fRightward ? psel->itcFirst : psel->itcLim);
	fForwardNew = cp >= psel->cpAnchor;

	fForwardNE = (fForward != fForwardNew);
	fRightwardNE = (fRightward != fRightwardNew);

	if (psel->fHidden)
		goto LRecSel;
	caTapFirstCommon.doc = docNil;

/* determine cp bounds of the rows that are always shared in common by the
	old and new selection. Columns in these rows may need to be highlighted/
	unhighlighted. */
	selCol = *psel;
	cpFirstCommon = CpMax(psel->cpFirst,
			(fForwardNew ? psel->cpAnchor : cp));
/* if we've already cached the bounds of the proper row, use that CA's cpFirst*/
	if (FInCa(doc, cpFirstCommon, &caTapNew))
		selCol.cpFirst = caTapNew.cpFirst;
	else  if (FInCa(doc, cpFirstCommon, pcaTapAnchor))
		selCol.cpFirst = pcaTapAnchor->cpFirst;
	else
		{
/* if row not cached for cpFirstCommon, do it and save bounds in
	caTapFirstCommon */
		CpFirstTap(doc, cpFirstCommon);
		selCol.cpFirst = caTap.cpFirst;
		caTapFirstCommon = caTap;
		}

	cpLimCommon = CpMin(psel->cpLim - 1,
			(fForwardNew ? cp : psel->cpAnchor));
	if (FInCa(doc, cpLimCommon, &caTapNew))
		selCol.cpLim = caTapNew.cpLim;
	else  if (FInCa(doc, cpLimCommon, pcaTapAnchor))
		selCol.cpLim = pcaTapAnchor->cpLim;
	else  if (FInCa(doc, cpLimCommon, &caTapFirstCommon))
		selCol.cpLim = caTapFirstCommon.cpLim;
	else
		{
		CpFirstTap(doc, CpMin(psel->cpLim - 1,
				(fForwardNew ? cp : psel->cpAnchor)));
		selCol.cpLim = caTap.cpLim;
		}

	fInSharedRows = (selCol.cpFirst <= cp && cp < selCol.cpLim);

/* if we are reversing the direction of the selection horizontally, there
	may be columns in the shared rows of the old selection that must be
	unhighlighted. */
	if (fRightwardNE)
		{
		if (fRightward)
			selCol.itcFirst++;
		else
			selCol.itcLim--;
		if (selCol.itcFirst < selCol.itcLim)
			ToggleSel(&selCol, selCol.cpFirst, selCol.cpLim);
		}
/* if we are reversing the direction of the selection vertically or are
	reversing the selection direction horizontally and the resulting selection
	is contained by the shared rows, any row in the old selection that is not
	part of the shared rows must be unhighlighted. */
	if (fForwardNE || (fRightwardNE && fInSharedRows))
		{
		selRow = *psel;
		if (fForward)
			selRow.cpFirst = selCol.cpLim;
		else
			selRow.cpLim = selCol.cpFirst;

		if (selRow.cpFirst < selRow.cpLim)
			ToggleSel(&selRow, selRow.cpFirst, selRow.cpLim);
		}
/* columns in the shared rows may need to be highlighted/unhighlighted in
	order to represent the new selection. */
	if (fRightwardNew)
		{
		selCol.itcFirst = min(itc + 1, psel->itcLim);
		selCol.itcLim = max(itc + 1, psel->itcLim);
		}
	else
		{
		selCol.itcFirst = min(itc, psel->itcFirst);
		selCol.itcLim = max(itc, psel->itcFirst);
		}
	if (selCol.itcFirst < selCol.itcLim)
		ToggleSel(&selCol, selCol.cpFirst, selCol.cpLim);

/* if we haven't changed direction horizontally and shrunk the selection so
	it is entiely contained in the shared rows (a case handled earlier),
	any entire rows in the difference between the old selection
	and the new selection must be highlighted/unhighlighted. */
	if (!(fRightwardNE && fInSharedRows))
		{
		selRow = *psel;
		if (fRightwardNew)
			{
			selRow.itcLim = fInSharedRows ?
					psel->itcLim : itc + 1;
			if (fRightwardNE)
				selRow.itcFirst = itcCpAnchor;
			}
		else
			{
			selRow.itcFirst = fInSharedRows ?
					psel->itcFirst :  itc;
			if (fRightwardNE)
				selRow.itcLim = itcCpAnchor + 1;
			}
		if (fForwardNew)
			{
			selRow.cpFirst = selCol.cpLim;
			selRow.cpLim = CpMax(psel->cpLim, caTapNew.cpLim);
			}
		else
			{
			selRow.cpFirst = CpMin(psel->cpFirst, caTapNew.cpFirst);
			selRow.cpLim = selCol.cpFirst;
			}
		if (selRow.cpFirst < selRow.cpLim)
			ToggleSel(&selRow, selRow.cpFirst, selRow.cpLim);
		}

LRecSel:
/* finally, record changed sel in *psel */
	if (fRightwardNew)
		{
		psel->itcLim = itc + 1;
		if (fRightwardNE)
			psel->itcFirst = itcCpAnchor;
		}
	else
		{
		psel->itcFirst = itc;
		if (fRightwardNE)
			psel->itcLim = itcCpAnchor + 1;
		}
	if (fForwardNew)
		{
		psel->cpLim = caTapNew.cpLim;
		if (fForwardNE)
			psel->cpFirst = pcaTapAnchor->cpFirst;
		}
	else
		{
		psel->cpFirst = caTapNew.cpFirst;
		if (fForwardNE)
			psel->cpLim = pcaTapAnchor->cpLim;
		}
	psel->fRightward = fRightwardNew;
	psel->fForward = fForwardNew;
}


/* %%Function:CpFirstForItc %%Owner:davidlu */
CP CpFirstForItc(doc, cp, itc)
int doc;
CP cp;
int itc;
{
	CP cpFirst = CpFirstTap(doc, cp);
	int itcLim = min(itc, vtapFetch.itcMac);

	return vmpitccp[itcLim];
}


#ifdef NOTUSED
/* ... and inefficient besides. If this needs to be done for
/* some reason, I can put something in fetch1 that uses some
/* of CpFirstTap's private info to do this much more efficiently.
/* (use vmpitccp, with the proper tap sufficiently cached)
/*		tomsax
/**/
/* %%Function:CacheTcForItc %%Owner:NOTUSED */
CacheTcForItc(ww, doc, cp, itc)
int ww, doc;
CP cp;
int itc;
{
	int itcT;
	CP cpFirst = CpFirstTap(doc, cp);
	int itcLim = min(itc, vtapFetch.itcMac - 1);

	CacheTc(ww, doc, cpFirst, fFalse, fFalse);
	for (itcT = 0; itcT < itcLim; itcT++)
		CacheTc(ww, doc, vtcc.cpLim, fFalse, fFalse);
}


#endif


/* %%Function:SelectRow %%Owner:davidlu */
SelectRow(psel, cpFirst, cpLim)
struct SEL *psel;
CP cpFirst;
CP cpLim;
{
	SelectColumn(psel, cpFirst, cpLim, 0, itcMax);
	psel->sk = skRows;
	psel->sty = styRow;
}


/* %%Function:SelectColumn %%Owner:davidlu */
SelectColumn(psel, cpFirst, cpLim, itcFirst, itcLim)
struct SEL *psel;
CP cpFirst, cpLim;
int itcFirst, itcLim;
{
	int iclpt, iclptOpp;
	CP cpFirstCell, cpOpposite;
	int doc = psel->doc;
	int fNil = psel->fNil;
	CP cpFirstInter, cpLimInter;
	int itcFirstInter, itcLimInter;
	struct CLPT *pclpt;
	struct CLPT rgclptSrc[4], rgclptDest[4];
	int itcAnchorReal, itcCpAnchor, itcCpAnchor2;
	struct CA caTapAnchorReal, caTapAnchor, caTapAnchor2;

	Win(InvalSelCurProps(fTrue));

/* find the cp of the cell that will be made the anchor cell. */
	cpFirstCell = CpFirstForItc(doc, cpFirst, itcAnchorReal = itcFirst);
	caTapAnchorReal = caTap;

	if (!psel->fTable || psel->fHidden)
		{
LSelNotCommon:
/* if not a column select or source and destination selections are disjoint,
	we simply select the upper left cell and pass cp of lower right cell
	to ColumnSelBegin. */
		if (!psel->fHidden)
			{
			TurnOffSel(psel);
			psel->fHidden = fFalse;
			}
		SetSelsIns(psel, psel->doc, cpFirstCell);
		if (!psel->fHidden)
			psel->sk = skNil;
		ColumnSelBegin(psel, CpFirstForItc(doc, CpMax(cpFirst, cpLim - 1), itcLim - 1), itcLim - 1, &caTapAnchorReal,
				 itcAnchorReal, cpFirstCell);
		psel->sk = skColumn;
		psel->sty = styCol;
		return;
		}
/* initialize the rgclpt's which represent the rectangles of the source and
	destination selections. */
	rgclptDest[0].cp = rgclptDest[1].cp = cpFirst;
	rgclptDest[2].cp = rgclptDest[3].cp = cpLim - 1;
	rgclptDest[0].itc = rgclptDest[3].itc = itcFirst;
	rgclptDest[1].itc = rgclptDest[2].itc = itcLim - 1;

LSrc:
	rgclptSrc[0].cp = rgclptSrc[1].cp = psel->cpFirst;
	rgclptSrc[2].cp = rgclptSrc[3].cp = psel->cpLim - 1;
	rgclptSrc[0].itc = rgclptSrc[3].itc = psel->itcFirst;
	rgclptSrc[1].itc = rgclptSrc[2].itc = psel->itcLim - 1;

/* first try to find a corner of the source selection rectangle which is
	inside the destination selection rectangle. if successful, record
	which corner in iclpt. */
	if ((iclpt = IclptPtInRect(rgclptSrc, rgclptDest)) != -1)
		{
		;
		}
/* if unsuccessful, try to find a corner of the destination selection rect
	which is inside the source selection rect. if successful, record the
	opposite corner in iclpt. */
	else  if ((iclpt = IclptPtInRect(rgclptDest, rgclptSrc)) != -1)
		{
		if ((iclpt += 2) >= 4)
			iclpt -= 4;
		}
	else
		{
/* find the coordinates of the intersection rectangle of the two selections.*/
		cpFirstInter = CpMax(rgclptDest[0].cp, rgclptSrc[0].cp);
		cpLimInter = CpMin(rgclptDest[3].cp, rgclptSrc[3].cp) + 1;

		itcFirstInter = max(rgclptDest[0].itc, rgclptSrc[0].itc);
		itcLimInter = min(rgclptDest[2].itc, rgclptSrc[2].itc) + 1;

/* if the selection's intersection is nil, just turn off the source selection
	and branch to make the destination selection */
		if (cpLimInter <= cpFirstInter ||
				itcLimInter <= itcFirstInter)
			{
			goto LSelNotCommon;
			}
/* if the intersection is non-nil, we recursively call SelectColumn to
	transform the src selection into the intersection rectangle. Then we
	we branch back to LSrc to transform the intersection rectangle into
	the destination selection. */
		SelectColumn(psel, cpFirstInter, cpLimInter, itcFirstInter,
				itcLimInter);
		goto LSrc;
		}
/* make the cell of the source selection that is in the corner represented by
	iclpt, the anchor point of the selection. */
	pclpt = &rgclptSrc[iclpt];

	psel->fTableAnchor = fTrue;
	psel->cpAnchor = CpFirstForItc(doc, pclpt->cp, itcCpAnchor = pclpt->itc);
	caTapAnchor = caTap;
	psel->fForward = (iclpt == 0 || iclpt == 1);
	psel->fRightward = (iclpt == 0 || iclpt == 3);

/* reposition the corner of the selection that is opposite the anchor cell to
	be the corner of the destination selection that is in the opposite quadrant
	from iclpt. */
	if ((iclptOpp = iclpt + 2) >= 4)
		iclptOpp -= 4;
	pclpt = &rgclptDest[iclptOpp];
	cpOpposite = CpFirstForItc(doc, pclpt->cp, itcCpAnchor2 = pclpt->itc);
	caTapAnchor2 = caTap;
	SelectColumnTo(psel, cpOpposite, pclpt->itc, &caTapAnchor, itcCpAnchor);

/* now make the cell of the dest selection that is in the corner represented
	by iclptOpp, the anchor point of the selection. */
	psel->cpAnchor = cpOpposite;
	psel->fForward = (iclptOpp == 0 || iclptOpp == 1);
	psel->fRightward = (iclptOpp == 0 || iclptOpp == 3);

/* reposition the corner of the selection that is opposite the new anchor cell
	to be the corner of the destination selection in the original quadrant. */
	pclpt = &rgclptDest[iclpt];
	SelectColumnTo(psel, CpFirstForItc(doc, pclpt->cp, pclpt->itc), pclpt->itc, &caTapAnchor2, itcCpAnchor2);

/* make the upper left corner cell the anchor for the transformed selection */
	psel->cpAnchor = cpFirstCell;
	psel->fForward = fTrue;
	psel->fRightward = fTrue;
	psel->sk = skColumn;
	psel->sty = styCol;

/* Voila! We have smoothly transformed the source selection into the
	destination selection. */
	Assert(psel->cpFirst == cpFirst && psel->cpLim == cpLim &&
			psel->itcFirst == itcFirst && psel->itcLim == itcLim);
}


/* %%Function:IclptPtInRect %%Owner:davidlu */
IclptPtInRect(rgclpt1, rgclpt2)
struct CLPT rgclpt1[];
struct CLPT rgclpt2[];
{
	int iclpt;
	struct CLPT *pclpt1;

	for (iclpt = 0, pclpt1 = rgclpt1; iclpt < 4; iclpt++, pclpt1++)
		{
		if (rgclpt2[0].cp <= pclpt1->cp && pclpt1->cp <= rgclpt2[2].cp &&
				rgclpt2[0].itc <= pclpt1->itc && pclpt1->itc <= rgclpt2[2].itc)
			return iclpt;
		}
	return -1;
}


/* %%Function:MakeSelNonTable %%Owner:davidlu */
MakeSelNonTable(psel, cpNew)
struct SEL *psel;
CP cpNew;
{
	int sty = psel->sty;
	CP cpAnchor = psel->cpAnchor;
	int fTableAnchor = psel->fTableAnchor;
	CP cpFirst, cpLim;

	psel->itcOppositeAnchor = (psel->fRightward) ? psel->itcLim - 1 : psel->itcFirst;
	if ((psel->fForward) ? (cpNew < cpAnchor) : (cpAnchor <= cpNew))
		{
		if (sty == styColAll || sty == styWholeTable)
			{
			CacheTable(psel->doc, psel->fTableAnchor ? cpAnchor : cpAnchor - 1);
			cpFirst = caTable.cpFirst;
			cpLim = caTable.cpLim;
			}
		else
			{
			CpFirstTap(psel->doc, psel->fTableAnchor ? cpAnchor : cpAnchor - 1);
			cpFirst = caTap.cpFirst;
			cpLim = caTap.cpLim;
			}
		SelectRow(psel, cpFirst, cpLim);
		}
	else  if (psel->fColumn)
		{
		SelectRow(psel, psel->cpFirst, psel->cpLim);
		}
	psel->cpAnchor = cpAnchor;
	psel->fTableAnchor = fTableAnchor;
	psel->sk = skSel;
	psel->sty = sty;
	psel->fForward = (psel->cpAnchor <= cpNew);
}


#ifdef DEBUG
/*  C K  T A B L E  S E L  */
/* Check the selection with respect to tableness */
/* %%Function:CkTableSel %%Owner:rosiep */
CkTableSel()
{

#ifdef DISABLE    /* DavidLu will be working on table selection;
	until then, some of these asserts are bogus -rp */
			if (selCur.doc == docNil)  /* e.g., if no docs open */
		return;

	if (FInTableDocCp(selCur.doc, selCur.cpFirst) &&
			FInTableDocCp(selCur.doc, CpMax(cp0, selCur.cpLim-1)))
		{
		CacheTc(wwNil, selCur.doc, selCur.cpFirst, fFalse, fFalse);
		if (selCur.cpLim >= vtcc.cpLim &&
				selCur.cpLim <= CpTableLim(selCur.doc, selCur.cpFirst))
			Assert(selCur.fTable);
		}

	if (selCur.fTable)
		{
		Assert(FInTableDocCp(selCur.doc, selCur.cpFirst));
		Assert(FInTableDocCp(selCur.doc, CpMax(cp0, selCur.cpLim-1)));
		}
#endif
}


#endif /* DEBUG */
