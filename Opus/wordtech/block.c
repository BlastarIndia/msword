#ifdef MAC
#define EVENTS
#include "toolbox.h"
#endif

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "props.h"
#include "format.h"
#include "sel.h"
#include "cmd.h"

#include "screen.h"

#ifdef MAC
#include "mac.h"
#endif
#include "debug.h"
#include "error.h"

#ifdef WIN
extern int		vcConseqScroll;
extern BOOL	    	vfRecording;
extern struct SEP   	vsepFetch;
extern struct SCI  	vsci;
#endif /* WIN */

extern struct UAB   vuab;
extern int              vssc;
extern int              wwCur;
extern char             (**vhgrpchr)[];
extern struct PAP       vpapFetch;
extern struct SEL       selCur;
extern struct WWD       **hwwdCur;
extern struct FLI       vfli;
extern struct MERR      vmerr;
extern struct PREF      vpref;
extern struct SAB       vsab;
extern struct CA        caPara;
extern struct CA        caTap;
extern struct TAP       vtapFetch;
extern CP               vmpitccp[];
extern BOOL		vfRightClick;
extern char             rgchEop[];

#ifdef MAC
extern struct EVENT     event;
#endif /* MAC */

CP CpLimBlock();
CP CpFirstBlock();
CP CpFromXpVfli();
CP CpFirstDlFromWwCpNS();
extern CP CpFromDlXp();
#ifdef WIN
extern CP CpToFromCurs();
#endif

/****
Some facts about block selection:
..cpFirst is always < cpLast no matter how you make the selection.
..cpFirst is the beginning cp of the first line.
..cpLast is the beginning cp of the last line.
..cpFirst and cpLast are used to find out which dl the selection is.
..with a dl and xpFirst(or xpLast), you can find out the exact cp of the block
	by CpFromDlXp.
..cpFirstLine is the beginning cp of the line where cpAnchor lies.
	(Note that cpFirstLine in an extended selection is the beginning 
	cp of the line where the final position is.)
****/


/* D O	C O N T E N T  H I T  B L O C K */
/* mouse down event with block select.
Follow drawing motion of mouse and return fBlock selection.
*/
/* %%Function:DoContentHitBlock %%Owner:davidlu */
DoContentHitBlock(psel, pt)
struct SEL *psel;
struct PT pt;
{
	CP cpFirst = psel->cpFirst;
	int fPageView;
	struct CA caNotTable;
	struct WWD *pwwd;
	struct PT ptT;
	int ipgd;
	int dysMinScroll, dysMaxScroll;
#ifdef WIN
	struct PLCEDL **hplcedl;
	int dyw;
	BOOL fForward = fFalse;
#endif

/* graphics are selected at first click - make it an insertion point at start */
	if (psel->fGraphics)
		SelectIns(psel, psel->cpFirst);
	Assert(psel->fIns || psel->fBlock);
	Assert(wwCur == psel->ww);
	if (WinMac((*hwwdCur)->fOutline, (vpref.fVol1 || (*hwwdCur)->fOutline || vssc != sscNil)))
		{
		Beep(); 
		return;
		}

	if (psel->fIns && !psel->fHidden) /* added for bug fix */
		ClearInsertLine(&selCur);

	caNotTable.doc = docNil;
	if (!FSelectBlockTo(psel, pt, &caNotTable, fTrue))
		return;
#ifdef WIN
	SetCapture((*hwwdCur)->hwnd);
	vcConseqScroll = 0;
	hplcedl = (*hwwdCur)->rgdr[0].hplcedl;
	fPageView = (*hwwdCur)->fPageView;
	dyw = (*hwwdCur)->ywMac - (*hwwdCur)->ywMin;
#else
	dysMinScroll = dysMinAveLineSci;
	dysMaxScroll = dysMacAveLineSci;
#endif
	while (FStillDownReplay( &pt, vfRightClick ))
		{
		pwwd = *hwwdCur;
		ptT = pt; /* for bug fix in OPUS */
		if (pt.yp >= pwwd->ywMac)
			{
			if (fPageView &&
					pwwd->ywMin + pwwd->rcePage.yeBottom + dypPastPageSci <= pwwd->ywMac)
				{
				if ((ipgd = IpgdNextWw(wwCur, fTrue)) == ipgdNil)
					continue;
				SetPageDisp(wwCur, ipgd, YeTopPage(wwCur), fFalse, fFalse);
				Mac( SetCrs(crsIbeam) );
				}

#ifdef WIN
			if (!fForward)
				vcConseqScroll = 0;
			fForward = fTrue;
			ScrollDelta(1, vcConseqScroll + 1, (*hplcedl)->dlMax,
					dyw, &dysMinScroll, &dysMaxScroll);
#endif

			if (fPageView || !FEndVisible(wwCur))
				ScrollUp(wwCur, dysMinScroll, dysMaxScroll);
			ptT.yp = (*hwwdCur)->ywMac - 1;
			goto DoCont1;
			}
		else  if (pt.yp < pwwd->ywMin)
			{
			if (fPageView &&  pwwd->rcePage.yeTop - dypPastPageSci >= 0)
				{
				if ((ipgd = IpgdPrevWw(wwCur)) == ipgdNil)
					continue;
				SetPageDisp(wwCur, ipgd, YeBottomPage(wwCur), fFalse, fFalse);
				Mac( SetCrs(crsIbeam) );
				}

#ifdef WIN
			if (fForward)
				vcConseqScroll = 0;
			fForward = fFalse;
			ScrollDelta(1, vcConseqScroll + 1, (*hplcedl)->dlMax,
					dyw, &dysMinScroll, &dysMaxScroll);
#endif
			ScrollDown(wwCur, dysMinScroll, dysMaxScroll);
			ptT.yp = (*hwwdCur)->ywMin/* + 1 WHY? */;
DoCont1:		
			UpdateWw(wwCur, fFalse);
#ifdef WIN
			if (vcConseqScroll < 0x7FFF)
				vcConseqScroll++;
#endif
			}
		else  if (pt.xp < pwwd->xwMin && pwwd->xeScroll < 0)
			{
			ScrollRight(wwCur, min(-pwwd->xeScroll, dxpMinScrollSci));
			Win(vcConseqScroll = -1);
			ptT.xp = (*hwwdCur)->xwMin;
#ifdef WIN
			goto DoCont1;
#else
			goto DoCont2;
#endif
			}
		else  if (pt.xp >= pwwd->xwMac)
			{
			ScrollLeft(wwCur, dxpMinScrollSci);
			Win(vcConseqScroll = -1);
			ptT.xp = (*hwwdCur)->xwMac - 1;
			Win( goto DoCont1 );
DoCont2:                
			;
#ifdef MAC
			if (GetNextEventReplay(1<<etUpdateEvt, &event, fFalse))
				DoUpdate(&event);
#endif /* MAC */
			Win( UpdateAllWws(fFalse) );
			}
		if (!FSelectBlockTo(psel, ptT, &caNotTable, fFalse))
			{
			Win(ReleaseCapture());
			return;
			}
		}

	Win(ReleaseCapture());
	/* a zero-width block selection is invisible and very confusing */
	if (selCur.xpFirst == selCur.xpLim)
		SelectIns(psel, cpFirst);
}


#ifdef MAC
/* B L O C K  S E L  B E G I N */
/* %%Function:BlockSelBegin %%Owner:NOTUSED */
BlockSelBegin(psel)
struct SEL *psel;
{
	CP cpFirst = psel->cpFirst;
	CP cpFirstLine;
	int doc = psel->doc;
	int dl;
	int fEnd = fFalse;
	int ichT, idrT, dlT;
	struct PLDR **hpldr;

	TurnOffSel(psel);

/* cannot simply use cpFirstLine because it may not be the cpFirstLine
where selCur.cpFirst lies, e.g. after a dragged selection */
/* make the line containing cpFirst scrolled in to 
make sure DlFromCpCheck returns a good dl */
	if ((cpFirstLine = psel->cpFirstLine) == cpNil)
		{
		NormCp(psel->ww, doc, cpFirst, ncpHoriz, dysMacAveLineSci, fEnd);
		dlT = DlWhereDocCp(psel->ww, doc, cpFirst, fEnd,
				&hpldr, &idrT, &cpFirstLine, NULL, fTrue);
		if (dlT == dlNil)
			return;
		Assert (cpFirstLine != cpNil);
		}

	SelectIns(psel, cpFirst);
	FormatLine(psel->ww, doc, cpFirstLine);
	XpFromDcp(cpFirst - cpFirstLine, cp0, &psel->xpFirst, &ichT);
	psel->xpFirst = max(0, psel->xpFirst);
	psel->cpFirstLine = psel->cpAnchor = cpFirstLine;
	psel->fForward = psel->fRightward = fTrue;
	psel->xpLim = psel->xpFirst;
	psel->fHidden = fFalse;
	psel->sk = skBlock;
}


#else /* WIN */
/* B L O C K  S E L  B E G I N */
/* Can't share with Mac yet because we use CpFirstDlFromWwCpNS.
Difference between Mac and Win is that we do not set the fBlock bit and
we do not scroll the dl in view to get the cpFirstLine. */
/* %%Function:BlockSelBegin %%Owner:davidlu */
BlockSelBegin(psel)
struct SEL *psel;
{
/* collapsed to psel->cpFirst no matter which direction the selection was made */
/* don't set fBlock yet */

	CP cpFirst = psel->cpFirst;
	CP cpFirstDl;
	int ichT;

	TurnOffSel(psel);
	SelectIns(psel, cpFirst);

/* set up psel->cpFirstLine so as to set up psel->xpFirst and
	psel->xpLim for block selection */
/* cannot simply use psel->cpFirstLine because it is the cpFirst of
	the line where the mouse button is lift in a drag selection, therefore
	it is not the cpFirst of the line where psel->cpFirst lies if 
	it is a downward selection.  
*/
	if ((cpFirstDl = CpFirstDlFromWwCpNS(psel->ww, psel->doc, cpFirst)) != cpNil)
		psel->cpFirstLine = cpFirstDl;
	else  /* the worst is we used a wrong cpFirstLine */
		psel->cpFirstLine = cp0;
	Assert (psel->cpFirstLine != cpNil);
	FormatLine(psel->ww, psel->doc, psel->cpFirstLine);
	XpFromDcp(psel->cpFirst - psel->cpFirstLine, cp0, &psel->xpFirst, &ichT);
	psel->cpAnchor = psel->cpFirstLine;
	psel->fForward = psel->fRightward = fTrue;
	psel->xpLim = psel->xpFirst;
	psel->fHidden = fFalse;
	psel->sk = skBlock;
}


#endif


/* F  S E L E C T  B L O C K  T O */
/* extend the block selection to pt.
*/
/* %%Function:FSelectBlockTo %%Owner:davidlu */
FSelectBlockTo(psel, pt, pcaNotTable, fNormTableBound)
struct SEL *psel;
struct PT pt;
struct CA *pcaNotTable;
int fNormTableBound;
{
	int idr;
	int dl;
	BOOL fForwardNew, fRightwardNew;
	BOOL fForward, fRightward;
	int xp;
	int xpT;
	int fBefore;
	int ww = psel->ww;
	int doc = psel->doc;
	struct PLDR **hpldr;
	CP cp, cpFirst;
	int fChangedPage;
	struct SPT spt;
	struct CA caOld;
	struct PT ptT;
	struct DRF drfFetch;


/* fix up cpFirst and cpLast to be cpFirstLine */
/* necessary since we could have gone into block selection mode
and somehow make a zero-width selection causing us to force it 
into an insertion point again. */
	if (psel->fIns)
		{
/* Win only do BlockSelBegin in here when cpFirstLine needs to be set up again, 
otherwise it is already called in CmdBlkExtend. */
#ifdef WIN
		if (psel->cpFirstLine == cpNil || vfRightClick)
#endif
			BlockSelBegin(psel);
		if (psel->sk != skBlock)
			goto LNoBlockSel;
		Assert(psel->cpAnchor != cpNil);
/* don't allow block selection to be started within a table */
		if (FInTableDocCp(doc, psel->cpAnchor))
			{
			psel->sk = skIns;
LNoBlockSel:
			Beep();
			return fFalse;
			}
		psel->cpFirst = psel->cpLast = psel->cpAnchor;
		}
	if (pcaNotTable->doc == docNil)
		{
		extern CP vcpFirstTablePara;
		AssertDo(!FInTableDocCp(doc, psel->cpAnchor));
		*pcaNotTable = caPara;
		if (vcpFirstTablePara != caPara.cpFirst)
			pcaNotTable->cpLim = vcpFirstTablePara;
		}
	ptT = pt; /* DlWherePt can change pt.xw to xwMaxSci - very bad */
	dl = DlWherePt(ww, &ptT, &hpldr, &idr, &spt, fTrue, fFalse);
	if (dl == dlNil || PdrFetchAndFree(hpldr, idr, &drfFetch)->doc != doc) return (fTrue);
/* make selection into a block selection */
	psel->sk = skBlock;
#ifdef TEST
	xp = max(0, XpFromXw(hpldr,idr,pt.xw)); /* clip out sel bar area */
#else
	xp = XpFromXw(hpldr,idr,pt.xw);
#endif /* TEST */
	cp = CpPlc(PdrFetch(hpldr,idr,&drfFetch)->hplcedl, dl);
	FreePdrf(&drfFetch);
/* make sure that we don't extend block select into a table. */
	if (!FParasUpToCpNotTable(pcaNotTable, cp, &fBefore))
		{
		xp = (psel->fRightward) ? psel->xpLim : psel->xpFirst;
		cp = fBefore ? pcaNotTable->cpFirst : pcaNotTable->cpLim - 1;
/* norm to the table bound if requested or if we are extending to higher cps
	and are not in page view (so DlWhereDocCp will succeed). */
		if (fNormTableBound || (!fBefore && !PwwdWw(ww)->fPageView))
			{
			if (fNormTableBound)
				Beep();
			NormCp(ww, doc, cp, ncpHoriz, dysMacAveLineSci, fFalse);
			}
/* if we extend toward higher cps and run into a table, we need to find the
	cp of the beginning of the line in front of the table */
		if (!fBefore || fNormTableBound)
			{
			dl = DlWhereDocCp(ww, doc, cp, fFalse,
					&hpldr, &idr, &cpFirst, &fChangedPage, fTrue);
			if (dl == dlNil)
				{
				Beep();
				return (vmerr.fMemFail);
				}
			cp = cpFirst;
			}
		}
/* extend block selection to cp, xp.
1. Calculate quadrant of desired extension point in anchor based
coordinate system.
	anchor point is one corner of the block, depending on
	fRightward and fForward.
2. if quadrant (=anchor position) unchanged, update block by differences
3. else erase old block, update cp's xp's, draw new block.
*/
	fRightward = psel->fRightward;
	fForward = psel->fForward;
	fRightwardNew = xp >= (/*xpAnchor*/fRightward ? psel->xpFirst :
			psel->xpLim);
	fForwardNew = cp >= (/*cpAnchor*/fForward ? psel->cpFirst :
			psel->cpLast);
	if (fRightward == fRightwardNew && fForward == fForwardNew)
		{
		/* adjust width */
		struct SELS sels;
		sels = *(struct SELS *) psel;
		if (fRightward)
			{
			sels.xpFirst = min(sels.xpLim, xp);
			sels.xpLim = max(sels.xpLim, xp);
			psel->xpLim = xp;
			}
		else
			{
			sels.xpLim = max(sels.xpFirst, xp);
			sels.xpFirst = min(sels.xpFirst, xp);
			psel->xpFirst = xp;
			}
		MarkSelsBlock(ww, &sels);
		/* adjust height */
		sels = *((struct SELS *)psel);
		sels.xpFirst = min(sels.xpFirst, xp);
		sels.xpLim = max(sels.xpLim, xp);
		caOld = psel->ca;
		if (fForward)
			{
			struct PLDR **hpldrT = hpldr;
			int idrT = idr;
			CP cpT = CpMin(sels.cpLast, cp);
			int fDoFormatLine = fTrue;
/* if we're in galley view or the result of the CpMin is equal to cpMin,
	then we also have the hpldrT and idrT we need to do FormatLineDr. If not
	we do DlWhereDocCp to find the hpldrT and idrT corresponding to cpT */
			if (PwwdWw(ww)->fPageView && cpT != cp)
				{
				if (DlWhereDocCp(ww, sels.doc, cpT, fFalse, &hpldrT, &idrT, 0, 0, fTrue) == dlNil)
					fDoFormatLine = fFalse;
				}
			if (fDoFormatLine)
				{
				FormatLineDr(ww, cpT, PdrFetch(hpldrT, idrT, &drfFetch));
				FreePdrf(&drfFetch);
				sels.cpFirst = vfli.cpMac;
				}
			else
				{
/* if we weren't able to find a hpldrT and idrT, that was because 
	sels.cpLast was on a previous page. In that case we'll just have to mark
	from the beginning of the current page. */
				struct PLC **hplcpgd = PdodDoc(sels.doc)->hplcpgd;
				sels.cpFirst = CpPlc(hplcpgd, IInPlc(hplcpgd, cpT));
				}
			sels.cpLast = CpMax(sels.cpLast, cp);
			psel->cpLast = cp;
			}
		else
			{
			sels.cpLast = CpMax(sels.cpFirst, cp) - 1;
			sels.cpFirst = CpMin(sels.cpFirst, cp);
			psel->cpFirst = cp;
			}
		if (FNeRgw(&caOld, &psel->ca, cwCA))
			MarkSelsBlock(ww, &sels);
		}
	else
		{
/* turn off old selection */
		MarkSelsBlock(ww, psel);
/* assign new value */
		if (fRightward != fRightwardNew)
			{
			if (psel->fRightward = fRightwardNew)
				{
				psel->xpFirst = psel->xpLim;
				psel->xpLim = xp;
				}
			else
				{
				psel->xpLim = psel->xpFirst;
				psel->xpFirst = xp;
				}
			}
		else
/* fRightward not changed, but the xpFirst or xpLim should be updated */
			{
			if (fRightward)
				psel->xpLim = xp;
			else
				psel->xpFirst = xp;
			}

		if (fForward != fForwardNew)
			{
			if (psel->fForward = fForwardNew)
				{
				psel->cpFirst = psel->cpLast;
				psel->cpLast = cp;
				}
			else
				{
				psel->cpLast = psel->cpFirst;
				psel->cpFirst = cp;
				}
			}
		else
	/* fForward not changed, but the cp that governs the yp position should be
	updated.  */
			{
			if (fForward) /* we are still below the anchor point */
				psel->cpLast = cp;
			else  /* we are above the anchor point */
				psel->cpFirst = cp;
			}
		MarkSelsBlock(ww, psel);
		}
	InvalSelCurProps (fTrue /* fStatLineToo */);
	return fTrue;
}


/* F  P A R A S  U P  T O  C P  N O T  T A B L E  */
/* scans from the bounds of pcaNotTable to cp and returns fTrue if every
	paragraph encountered has pap.fInTable == fFalse. returns fFalse otherwise.
	the bounds of pcaNotTable are maintained as we scan. */
/* %%Function:FParasUpToCpNotTable %%Owner:davidlu */
FParasUpToCpNotTable(pcaNotTable, cp, pfBefore)
struct CA *pcaNotTable;
CP cp;
int *pfBefore;
{
	int doc = pcaNotTable->doc;
	CP cpScan;
	CP cpT;

	if (FInCa(doc, cp, pcaNotTable))
		return fTrue;

	if (*pfBefore = (cp < pcaNotTable->cpFirst))
		{
		cpScan = pcaNotTable->cpFirst - 1;
		while (cpScan >= cp)
			{
			extern CP vcpFirstTablePara;
			if (FInTableDocCp(doc, cpScan))
				return fFalse;
			cpScan = vcpFirstTablePara;
			pcaNotTable->cpFirst = cpScan;
			cpScan--;
			}
		}
	else
		{
		cpScan = pcaNotTable->cpLim;
		while (cpScan <= cp)
			{
			extern CP vcpFirstTablePara;
			if (FInTableDocCp(doc, cpScan))
				return fFalse;
			cpScan = pcaNotTable->cpLim = 
					((vcpFirstTablePara == cpScan) ?
					caPara.cpLim : vcpFirstTablePara);
			}
		}
	return fTrue;
}


/* T U R N  O F F  B L O C K */
/* converts a block selection into an insertion point to upper left.
*/
/* %%Function:TurnOffBlock %%Owner:davidlu */
TurnOffBlock(psel)
struct SEL *psel;
{
	CP cp, dcp;

	Assert(psel->fBlock);
/* get real first cp on first line */
	cp = CpFirstBlock();
	cp = CpMin(cp, CpMacDocEdit(psel->doc));
	Assert(cp != cpNil);
	SelectIns(psel, cp);
}


/* M A R K  S E L S  B L O C K */
/* inverts a rectangular area described by cpFirst, cpLast, xpFirst and
xpLim in sel.
*/
/* %%Function:MarkSelsBlock %%Owner:davidlu */
MarkSelsBlock(ww, psels)
int ww;
struct SELS *psels;
{
	struct WWD **hwwd = HwwdWw(ww);
	int ypFirst = 0;
	int ypLim = 0;
	int dl;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct RC rc;
	int idr;
	struct DR *pdr;
	struct DRF drfFetch;
	struct DRC drcp;
#ifdef MAC
	struct WDS wds;

	SaveWds(&wds);
	PwwdSetWw(ww, cptDoc);
#endif

	for (idr = 0; idr < (*hwwd)->idrMac; FreePdrf(&drfFetch), ++idr)
		{
		pdr = PdrFetch(hwwd, idr, &drfFetch);
		if (pdr->doc != psels->doc || (hplcedl = pdr->hplcedl) == hNil ||
				IMacPlc(hplcedl) == 0)
			continue;
		hplcedl = pdr->hplcedl;
/* get ypFirst, ypLim from cpFirst, cpLast */
		if (psels->cpFirst >= CpPlc(hplcedl, 0))
			{
			if ((dl = IInPlcRef(hplcedl, psels->cpFirst)) < 0
					|| dl >= IMacPlc(hplcedl))
/* selection wholly below dr */
				continue;
			GetPlc(hplcedl, dl, &edl);
			ypFirst = edl.ypTop;
			}
		else
			ypFirst = 0;
		if (psels->cpLast < CpPlc(hplcedl, 0))
/* selection wholly above dr */
			continue;
		if ((dl = IInPlcCheck(hplcedl, psels->cpLast)) >= 0)
			{
			GetPlc(hplcedl, dl, &edl);
			ypLim = edl.ypTop + edl.dyp;
			}
		else  if ((dl = IMacPlc(hplcedl)) > 0)
			{
			GetPlc(hplcedl, dl - 1, &edl);
			ypLim = edl.ypTop + edl.dyp;
			}
		else
			ypLim = 0;

		SetRect(&rc,
				psels->xpFirst,
				ypFirst,
				psels->xpLim,
				ypLim);

		/* We need to confine this to the dr bounds */
		drcp = rc;
		drcp.dxp -= drcp.xp;
		drcp.dyp -= drcp.yp;


		ClipRectFromDr(hwwd, hwwd, idr, &drcp, &rc);
		WinMac(InvertRect((*hwwd)->hdc, &rc), HiliteRc(&rc));
		}
	Mac( RestoreWds(&wds) );
}


/*  C P F I R S T  B L O C K  */
/* %%Function:CpFirstBlock %%Owner:davidlu */
CP CpFirstBlock()
{
/* Get cpFirst of the first line of the block */
	struct BKS bks;
	CP cpFirst, dcp;

	InitBlockStat(&bks);
	return(FGetBlockLine(&cpFirst, &dcp, &bks) ? cpFirst : selCur.cpFirst);
}


/* C P  L I M  B L O C K */
/* %%Function:CpLimBlock %%Owner:davidlu */
CP CpLimBlock()
{
/* returns cpLim of the current block selection; selCur.cpLast is the
	cpFirstLine of the last line of the block */
	CP cp, dcp;
	struct BKS bks;

	InitBlockStat(&bks);
	bks.cpFirst = selCur.cpLast;
	/* bks.cpFirst was set to vfli.cpLim of last line in block */
	return((FGetBlockLine(&cp, &dcp, &bks)) ? bks.cpFirst : selCur.cpLast);
}


/* C M D  P A S T E  B L O C K */
/* %%Function:CmdPasteBlock %%Owner:davidlu */
CmdPasteBlock()
{
	int cl, clMac, clLast;
	int fEndDoc, cmd;
	CP **hcp;
	struct DOD *pdod;
	struct PLC **hplcfnd;
	int idrT, xp;	/* these are throwaway return values */
	struct PLDR **hpldr;
	CP cpSrcIns, cpFirst, cpNext, cpLim;
	CP dcp, dcpInsert, dcpDelete;
	CP cpMacScrap = CpMacDocEdit(docScrap);
	CP cpFirstDest, cpLimDest;
	struct BKS bks;
	struct CA caT1, caT2, caRM;
	int ich;
	int fRefText;
	struct PAP pap;

	Assert(vsab.fBlock);
	Assert(!selCur.fBlock);

	if (cpMacScrap == cp0)
		return(cmdError);


#ifdef MAC
	if (FInTableDocCp(selCur.doc, selCur.cpFirst))
		{
		ErrorEid(eidIllegalPasteToTable, "CmdPasteBlock");
		return cmdError;
		}
#else
	/* win checks before getting to here (CmdPaste) that if selCur.cpFirst is
		in a table, autodelete is on and the selection goes beyond
		the table. So the tableness will be destroyed by autodelete
		and we don't need to worry about it. Heavy asserts to assure
		noone messes with the assumptions. bz
	*/

#ifdef DEBUG
	if (FInTableDocCp(selCur.doc, selCur.cpFirst))
		{
		Assert (vpref.fAutoDelete);
		Assert (!selCur.fWithinCell && !selCur.fTable);
		}
#endif /* DEBUG */

#endif /* MAC */


/* determine the number of lines in the scrap and allocate an rgcp */
	for (cpLim = cp0, clMac = 0; cpLim < cpMacScrap; clMac++)
		{
		CachePara(docScrap, cpLim);
		cpLim = caPara.cpLim;
		}
	if ((hcp = HAllocateCw(clMac * sizeof(CP) / sizeof(int))) == hNil)
		{
		SetErrorMat( matLow );
		return(cmdNoMemory);
		}
#ifdef BOGUS
	TurnOffSel(&selCur);
/* determine the point where paste will start without actually deleting
the current sel */
	dcp = !fWin || vpref.fAutoDelete ? cp0 :
			CpMin(selCur.cpLim, CpMacDocEdit(selCur.doc)) - selCur.cpFirst;
	cpFirstDest = selCur.cpFirst + dcp;
#endif
	dcpDelete = !fWin || vpref.fAutoDelete ? 
			CpMin(selCur.cpLim, CpMacDocEdit(selCur.doc)) - selCur.cpFirst :
			cp0;
	cpFirstDest = selCur.cpFirst;

	if (!FSetUndoB1(ucmPasteBlock, uccPaste,
			PcaSetWholeDoc(&caT1, selCur.doc)))
		{
		cmd = cmdCancelled;
		goto LNoPaste;
		}

	InvalAgain();
	if ((cmd = CmdAutoDelete(&caRM)) != cmdOK)
		goto LNoPaste;

/* bring the insertion point into the window and find its xp and the CP of
	the start of the line */
	SetBytes(&bks, 0, cbBKS);
	bks.doc = selCur.doc;
	bks.cpLast = CpMacDocEdit(selCur.doc);
	NormCp(wwCur, selCur.doc, selCur.cpFirst, 2, 20, selCur.fInsEnd);
	UpdateWw(wwCur, fFalse);
	if (DlWhereDocCp(wwCur, selCur.doc, cpFirstDest, selCur.fInsEnd,
		&hpldr, &idrT, &bks.cpFirst, NULL, fTrue) == dlNil)
		{
		Beep();	/* we may or may not get an alert; NormCp could have failed */
		cmd = cmdError;
		goto LNoPaste;
		}
	FormatLine(wwCur, selCur.doc, bks.cpFirst);
	bks.xpFirst = XpFromDcp(cp0, cpFirstDest - bks.cpFirst, &xp, &ich);
	bks.xpLim = bks.xpFirst + 32;	/* arbitrary number */
	bks.ww = wwCur;

/* determine cpLim of insertion */
	if (!(fRefText = ((pdod = PdodDoc(selCur.doc))->fFtn || pdod->fAtn)))
		cpLimDest = CpMacDoc(selCur.doc);
	else
		{
		/* prevent paste past end of footnote or annotation */
		/* Note - hplcfnd and hplcand are stored in the same location */
		hplcfnd = pdod->hplcfnd;
		cpLimDest = CpPlc(hplcfnd, IInPlc(hplcfnd, cpFirstDest) + 1);
		}
	cpLimDest -= ccpEop;

/* fill the rgcp with the CPs where insertion will occur */
	**hcp = cpLimDest;	/* in case no lines */
	for (cl = 0; cl < clMac && FGetBlockLine(&cpFirst, &dcp, &bks); )
		{
		if (FInTableDocCp(selCur.doc, cpFirst))
			{
			cpLimDest = CpTableFirst(selCur.doc, cpFirst) - ccpEop;
			if (cpLimDest < cp0)
				{
				/* insert a para mark in front of the table so that
				/* the code below can pretend that it is at the end of
				/* the document.
				/**/
				CachePara(selCur.doc, cp0);
				pap = vpapFetch;
				pap.fInTable = fFalse;
				if (!FInsertRgch(selCur.doc, cp0, rgchEop, cchEop, 0, &pap))
					{
					cmd = cmdNoMemory;
					goto LNoPaste;
					}
				InvalTableProps(selCur.doc,cp0,2*ccpEop,fTrue/*fFierce*/);
				cpLimDest = cp0;
				}
			break;
			}
		if ((*(*hcp + cl++) = cpFirst) >= cpLimDest)
			break;
		}

	if (fEndDoc = (cl < clMac))
		{
		/* ran out of lines in doc, the rest will be inserted after */
		clMac = cl;
/* this eop is for the last line because we only use up to CpMacDocEdit of
docScrap which does not have a terminating eop */
		cpMacScrap += ccpEop;
/* but don't replace the eop at the end of a footnote */
		if (!fRefText)
			cpLimDest += ccpEop;

		}

/* insert from the scrap into the document */
	cpLim = **hcp;
	cpSrcIns = dcp = cp0;
	if (clMac != 0)
		{
/* dcp is set to 0 or length of deleted selection. It will be maintained
as the accumulated adjustement relative to the initial situation. */
		for (cl = 0; cl < clMac; cl++, cpSrcIns = cpNext)
			{
			if (vmerr.fMemFail)
				{
				cmd = cmdNoMemory;
LNoPaste:
				FreeH(hcp);
				return(cmd);
				}
			if (cpSrcIns >= cpMacScrap)
				{
				fEndDoc = fFalse; 
				break; 
				}
			CachePara(docScrap, cpSrcIns);
			AssureLegalSel(PcaSet(&caT2,docScrap,cpSrcIns,
					caPara.cpLim-ccpEop));
			Assert(caT2.cpFirst==cpSrcIns);
			while (CachePara(docScrap, caT2.cpLim),
					(caT2.cpLim < caPara.cpLim-ccpEop))
				{
				caT2.cpLim = caPara.cpLim-ccpEop;
				AssureLegalSel(&caT2);
				}
			cpNext = caPara.cpLim;
			Assert(**hcp >= cp0);
			cpLim = *(*hcp + cl) + dcp;
			dcpInsert = cpNext - cpSrcIns - ccpEop;
			FReplaceCpsRM(PcaPoint(&caT1, selCur.doc, cpLim),
					PcaSetDcp(&caT2, docScrap, cpSrcIns,dcpInsert));
#ifdef WIN
			if (vsab.fFormatted && !vmerr.fMemFail)
				{
				struct CA caT;

				CopyFonts(DocMother (vsab.docStsh != docNil ?
						vsab.docStsh : docScrap),
						PcaSetDcp(&caT,selCur.doc,cpLim,dcpInsert));
				}
#endif
			dcp += dcpInsert;
			cpLimDest += dcpInsert;
			}
		}
	FreeH(hcp);

/* if end of doc reached, dump the rest at the end of the doc */
	if (fEndDoc)
		{
		if (fRefText)
			{
			/* Add an extra EOP to the footnote text */
			CachePara(selCur.doc, cpLimDest);
			pap = vpapFetch;
			if (!FInsertRgch(selCur.doc, cpLimDest, rgchEop, cchEop, 0, &pap))
				return cmdNoMemory;
			cpLimDest += ccpEop;
			}
		if (!FReplaceCpsRM(PcaPoint(&caT1, selCur.doc, cpLimDest), 
				PcaSet(&caT2, docScrap, cpSrcIns, cpMacScrap)))
			return cmdNoMemory;
		dcpInsert = cpMacScrap - cpSrcIns;
		if (vsab.fFormatted)
			CopyStylesFonts(vsab.docStsh != docNil ? vsab.docStsh :
					docScrap, selCur.doc, cpLimDest, dcpInsert);
		cpLim = cpLimDest + dcpInsert;
		}
	CachePara(selCur.doc, cpLim);
	SetUndoAfter(PcaSetWholeDoc(&caT1, selCur.doc));
	return cmdOK;
}


/* C M D  C U T  B L O C K */
/* %%Function:CmdCutBlock %%Owner:davidlu */
CmdCutBlock(fScrap)
int fScrap;
{
/* Note: this routine handles both undo and selection (Turnoffblock does
	a selectins on cpFirst before the deletion) so callers should not do a
	SetUndoAfter or a selectpostdelete on return bz
*/
	BOOL fDelete;
	int clBlock;
	CP cp, cpFirst, dcp;
	struct CA caT, caUndo;
	struct BKS bks;

	/* confirm deletion */
	if (!FDelCkBlockOk(rpkNil, NULL))
		return(cmdError);

	/* find end of undo area */
	/* Note: set to whole doc because of limitations of copying fields */
	if (!FSetUndoB1(ucmCutBlock, uccPaste,
#ifdef WIN
			PcaSetWholeDoc(&caUndo, selCur.doc)))
#else
		PcaSet(&caUndo, selCur.doc, 
				selCur.cpFirst, CpLimBlock())))
#endif
				return(cmdCancelled);

	InvalAgain();
	/* copy block to clipboard */
	if (fScrap)
		if (ClBlockSelToScrap() == 0)
			return(cmdError);
	if (vmerr.fMemFail)
		return(cmdNoMemory);

	InitBlockStat(&bks);
	TurnOffBlock(&selCur);
	while (FGetBlockLine(&cp, &dcp, &bks))
		{
		if (!FDeleteRM(PcaSetDcp(&caT, selCur.doc, cp, dcp)))
			return(cmdNoMemory);
		/* reduce dcp by what's left after FDeleteRM */
		dcp -= DcpCa(&caT);
		bks.cpFirst -= dcp;
		bks.cpLast -= dcp;
#ifdef WIN
		}
	SetUndoAfter(PcaSetWholeDoc(&caUndo, selCur.doc));
#else
	caUndo.cpLim -=dcp;
}


SetUndoAfter(&caUndo);
#endif
return cmdOK;
}


/* C L  B L O C K  S E L  T O  S C R A P */
/* %%Function:ClBlockSelToScrap %%Owner:davidlu */
ClBlockSelToScrap()
{
	CP dcp, cp = cp0, cpFirst;
	int clBlock = 0;
	struct CA caT1, caT2;
	struct BKS bks;

	SetWholeDoc(docScrap, PcaSetNil(&caT1));
	InitBlockStat(&bks);
	while (!vmerr.fMemFail &&
			FGetBlockLine(&cpFirst, &dcp, &bks))
		{
		if (dcp)
			{
			if (!FReplaceCps(PcaPoint(&caT1, docScrap, cp),
					PcaSetDcp(&caT2, selCur.doc, cpFirst, dcp)))
				break;
/* take care of dead field properties, etc. */
			Win(AssureNestedPropsCorrect(PcaSetDcp(&caT1, docScrap, cp, dcp), fTrue));
			cp += dcp;
			clBlock++;
			}
		if (!FReplace(PcaPoint(&caT1, docScrap, cp),
				fnSpec, (FC) fcSpecEop, ccpEop))
			break;
		cp += ccpEop;
		}
	if (clBlock && !vmerr.fMemFail)
		{
		/* remove the extra chEop after the last line */
		FDelete(PcaSet(&caT1, docScrap, cp - ccpEop, cp));
		vsab.fBlock = fTrue;
		}
	vsab.docStsh = DocMother(selCur.doc);
	vsab.fFormatted = PdodDoc(vsab.docStsh)->fFormatted;
	vsab.fMayHavePic = PdodMother(vsab.docStsh)->fMayHavePic;
	vsab.fTable = fFalse;
	Win( vsab.doc = docNil); /* for paste link */
	Win( ChangeClipboard() );
	return clBlock;
}


/* I N I T  B L O C K  S T A T */
/* %%Function:InitBlockStat %%Owner:davidlu */
InitBlockStat(pbks)
struct BKS *pbks;
	/* initialize block-selection decomposition from selCur */
{
	InitBlock1(pbks, &selCur);
}


/* I N I T  B L O C K  1 */
/* %%Function:InitBlock1 %%Owner:davidlu */
InitBlock1(pbks, psel)
struct BKS *pbks; 
struct SEL *psel;
{
	Assert(psel->fBlock);
	pbks->ca = psel->ca;
	if (!(pbks->fColumn = psel->fColumn))
		{
		pbks->xpFirst = psel->xpFirst;
		pbks->xpLim = psel->xpLim;
		}
	else
		{
		pbks->itcFirst = psel->itcFirst;
		pbks->itcLim = psel->itcLim;
		pbks->fLastRow = fFalse;
		pbks->fReturnEmptyCell = fFalse;
		}
	pbks->ww = psel->ww;
}


/* F  G E T  B L O C K  L I N E */
/* %%Function:FGetBlockLine %%Owner:davidlu */
int FGetBlockLine(pcpFirst, pdcp, pbks)
CP *pcpFirst, *pdcp;
struct BKS *pbks;
{
/* gives the relevant CPs of the next line of a block as defined by pbks.
	guarantees that no chEop is contained in the CP range.
	*pdcp will be zero if the block contained no CPs on this line (e.g. short
	line); in this case, *pcpFirst is vfli.cpMac - ccpEop.
	returns fFalse at end of block
*/
	struct DOD *pdod;
	int iT, fGetCell;
	CP cpFirst, cpLim;
	int ww, fPageview;
	struct WWD *pwwd;

	if (pbks->fColumn)
		{
		fGetCell = FGetCell(pcpFirst, pdcp, pbks);
		return (fGetCell);      /* possible compiler problem */
		}

	if (pbks->cpFirst > pbks->cpLast)
		return(fFalse);         /* done */
	pdod = PdodDoc(pbks->doc);      /* separate stmt for native compiler */
	if ((pdod->fFtn || pdod->fAtn) && pbks->cpFirst >= CpMacDocEdit(pbks->doc))
		return(fFalse);         /* done */

/* check for empty slice of block */
	ww = pbks->ww;
	pwwd = PwwdWw(ww);
	fPageview = pwwd->fPageView;
	if (!fPageview)
		FormatLine(pbks->ww, pbks->doc, pbks->cpFirst);
	else
		{
		struct PLDR **hpldrT;
		int idrT;
		int fChangedPageT;
		struct DRF drfFetch;

		if (DlWhereDocCp(pbks->ww, pbks->doc, pbks->cpFirst, fFalse, 
				&hpldrT, &idrT, 0, &fChangedPageT, fTrue) == dlNil)
			return fFalse;
		FormatLineDr(pbks->ww, pbks->cpFirst, PdrFetch(hpldrT, idrT, &drfFetch));
		FreePdrf(&drfFetch);
		}
	if (vfli.fSplatBreak || pbks->xpLim <= vfli.xpLeft)
		{
		*pdcp = 0;
		*pcpFirst = vfli.cpMin;
		goto LHaveDcp;
		}
	else  if (pbks->xpFirst >= vfli.xpRight)
		{
		*pdcp = 0;
		*pcpFirst = vfli.cpMac - ccpEop;
		goto LHaveDcp;
		}

	CacheParaCa(&pbks->ca); /* for CpFromXpVfli */

/* we know that at least one end of the block is inside the line.  We now
use CpFromXpVfli to get the first and last cp of the block.  This is less
efficient than writing a routine to get both cp's at once, since to get the
2nd cp we start at the front of vfli again, but it allows us to centralize
the rather complex walk-the-chr loop. */

	if (pbks->xpFirst <= vfli.xpLeft)
	/* block starts before beginning of line */
		cpFirst = vfli.cpMin;
	else
		cpFirst = CpFromXpVfli(pbks->xpFirst, &iT, fTrue /* fTabTrail */, fFalse);

	if (pbks->xpLim >= vfli.xpRight)
/* block goes to end of line */
/* CpFirstSty takes care of the CR/LF problem at eop. fEnd is true if break
is eop or CRJ, so we get the start CP of the break character */
		cpLim = CpFirstSty (wwCur, pbks->doc, vfli.cpMac, styChar,
				(vfli.chBreak == chEop || vfli.chBreak == chCRJ) /* fEnd */);
	else
		cpLim = CpFromXpVfli(pbks->xpLim, &iT, fTrue /* fTabTrail */, fFalse);

#ifdef MAC
	if (cpLim >= vfli.cpMac && (vfli.chBreak == chEop || vfli.chBreak == chCRJ))
		cpLim = vfli.cpMac - (vfli.chBreak == chEop ? ccpEop : 1);
#else /* WIN */
	if (cpLim >= vfli.cpMac && (vfli.chBreak == chEop || vfli.chBreak == chCRJ))
		cpLim = CpFirstSty(wwCur, pbks->doc, vfli.cpMac, styCRLF, fTrue);
#endif /* MAC */

/* cheated somehow because CpFromXpVfli never returns cpNil, and
cpLim that we get back may not be cpMac if it is a paragraph mark,
therefore we can get a situation where cpFirst > cpLim */
	if (cpFirst > cpLim) /* from OPUS */
		cpFirst = cpLim;

	Assert(cpLim <= vfli.cpMac && cpLim >= cpFirst);
	Assert(cpFirst >= 0 && cpFirst <= cpLim);
	*pcpFirst = cpFirst;
	*pdcp = cpLim - cpFirst;

LHaveDcp:
	pbks->cpFirst = vfli.cpMac;
	Assert (*pdcp >= 0); /* del'd conditional which was present in MacWord 3 code */
	return(fTrue);
}


/* %%Function:FGetCell %%Owner:davidlu */
FGetCell(pcpFirst, pdcp, pbks)
CP *pcpFirst, *pdcp;
struct BKS *pbks;
{
	int doc;
	CP cpLim;

LTryAgain:
	if (pbks->cpFirst >= pbks->cpLast || pbks->fLastRow)
		return fFalse;
	doc = pbks->doc;
	*pcpFirst = CpFirstForItc(doc, pbks->cpFirst, pbks->itcFirst);
	pbks->cpFirst = caTap.cpLim;
	pbks->fLastRow = vtapFetch.fLastRow;
	if (pbks->itcFirst > vtapFetch.itcMac && !pbks->fReturnEmptyCell)
		goto LTryAgain;

	cpLim = vmpitccp[min(vtapFetch.itcMac+1, pbks->itcLim)];
	*pdcp = cpLim - *pcpFirst;

	return fTrue;
}



#ifdef WIN 
/* FUTURE: When MacWord has field, this can be ported (cc) */
/* %%Function:CpFirstDlFromWwCpNS %%Owner:davidlu */
CP CpFirstDlFromWwCpNS(ww, doc, cp) /* NS - No Scroll */
int ww;
int doc;
CP cp;
{
/* This routine does not have to scroll the cp into display rect in order
to get the cpFirstLine*/

	struct PLC **hplc = HplcInit(0, 5, cp0, fTrue /* far plc */);
	CP cpFirstLine = cpNil;

	Assert(ww != wwNil && cp != cpNil);

	if (hplc != hNil && FFormatDisplayPara(ww, doc, CpFormatFrom(ww, doc, cp), cp+1, hplc))
		{
		Assert(IMacPlc(hplc)-1 >= 0);
		cpFirstLine = CpPlc(hplc, IMacPlc(hplc)-1);
		}
	FreeHplc( hplc );
	return cpFirstLine;
}


#endif


#ifdef WIN
/* C U R S  B L O C K  L E F T  R I G H T */
/* Move or select (fDrag) dSty sty units from the current position. */
/* %%Function:CursBlockLeftRight %%Owner:davidlu */
CursBlockLeftRight(sty, dSty, fDrag)
int sty, dSty;
BOOL fDrag; /* ignored */
{

	CP cpFrom, cpTo, cpT;
	CP cpFirstLine, cpMacLine, cpFirstT;
	BOOL fForward = selCur.fForward;
	BOOL fBlock = selCur.fBlock;
	BOOL fEnd;
	struct PT pt;
	struct PLCEDL **hplcedl;
	struct PLDR **hpldr;
	int dl;
	int xpFrom = selCur.fRightward ? selCur.xpLim : selCur.xpFirst;
	int xpMargin, xpScroll, xpWwMac;
	int iT, ichT;
	BOOL fSplat;
	struct EDL edl;
	struct CA caNotTable;
	int idr;
	BOOL fChangedPage;
	struct DRF drfFetch;

	UpdateWw(wwCur, fFalse/*fAbortOK*/);

/* make one end of the selection visible */
	cpT = (fForward ? selCur.cpLim : selCur.cpFirst);
	fEnd = selCur.fBlock ? fFalse : selCur.fInsEnd;

/* cpFrom, cpFirstLine, dl, pt.yp, cpMacLine, xpMargin */

	if (!FCpVisible(wwCur, selCur.doc, cpT, fEnd, fFalse /*fHoriz*/, fFalse))
		NormCp(wwCur, selCur.doc,cpT, 2/*grpfNorm*/, vsci.dysMacAveLine, fEnd);

	dl = DlWhereDocCp(wwCur, selCur.doc, cpT, fEnd, &hpldr, &idr,
			&cpFirstLine, &fChangedPage, fTrue);
	if (dl == dlNil)
		{
		Beep();	/* NormCp couldn't drag line into window */
		return;
		}
	if (vmerr.fMemFail)
		return;
	Assert(dl >= 0);
	Assert(idr != idrNil);
	hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
	Assert(hplcedl);
	if (fBlock)
		{
		cpFrom = CpFromDlXp(wwCur, hpldr, idr, dl, xpFrom, &iT, NULL); /* HM */
		}
	else
		cpFrom = cpT;

	GetPlc( hplcedl, dl, &edl );
	pt.yp = edl.ypTop + edl.dyp - 1;
	cpMacLine = CpPlc( hplcedl, dl ) + edl.dcp;

	CacheSect(selCur.doc, cpFirstLine);
	xpMargin = NMultDiv(vsepFetch.dxaColumnWidth, vfli.dxsInch, czaInch);
	/* DREWS - right side of page can now go into negative coordinates */
	/* so xpScroll value must be flipped */
	xpScroll = -(*hwwdCur)->xeScroll;
	xpWwMac = (*hwwdCur)->xwMac;

/* fEnd for cpFrom */
	fEnd = dSty < 0; /* general case */
	if (fBlock)
		{
		if (cpFrom == selCur.cpFirst || cpFrom == selCur.cpLim)
			fEnd = fFalse;
		else  if (dSty > 0 && cpFrom == cpMacLine)
			fEnd = fTrue; /* at end of line */
		}
	else
		{
		if (cpFrom == cpFirstLine)
			fEnd = fFalse;
		}

/* cpTo, pt.xp */
	if (sty == styLineEnd) /* Home/End - simple case */
		{
		cpTo = dSty < 0 ? cpFirstLine : cpMacLine;
		FormatLine(selCur.ww, selCur.doc, cpFirstLine);
		XpFromDcp(cpTo - cpFirstLine, cp0, &pt.xp, &ichT);
		}
	else
		{
		if (cpFrom != cpNil)
			{
			cpTo = CpToFromCurs(&selCur, cpFrom, cpNil, sty, dSty, fEnd,
					fTrue /*fDrag*/, &fSplat);
			Assert(cpTo != cpNil);
			}
		else
			cpTo = cpNil;

		if (cpFrom != cpTo)
			{
			Assert(cpTo != cpNil);
			FormatLine(selCur.ww, selCur.doc, cpFirstLine);
			XpFromDcp(cpTo - cpFirstLine, cp0, &pt.xp, &ichT);
			}
		else  /* cpFrom == cpTo => reach boundary */			
			{
			if (dSty > 0) /* move right one */
				pt.xp = max(xpFrom, min(xpMargin, 
						xpFrom + vsci.dxpTmWidth));
			else  if (cpTo == cpFirstLine)
				{
				FreePdrf(&drfFetch);
				goto LRet; /* can't move left anymore */
				}
			else  /* move left one */
				pt.xp = max(0, xpFrom - vsci.dxpTmWidth);
			}
		}

/* check if flipping is necessary to avoid zero width block */
	if (pt.xp == (selCur.fRightward ? selCur.xpFirst : selCur.xpLim)/*opposite of xpFrom*/)
		{
		if (pt.xp == 0)
/* no place to go left, stay 1 char width */
			{
			if (cpTo == cpFirstLine || cpTo == cpNil)
				cpTo = CpToFromCurs(&selCur, cpFirstLine, cpNil,
						styChar, 1/*dSty*/, fFalse/*fEnd*/, 
						fTrue/*fDrag*/, &fSplat);
			Assert(cpTo != cpNil);
			FormatLine(selCur.ww, selCur.doc, cpFirstLine);
			XpFromDcp(cpTo - cpFirstLine, cp0, &pt.xp, &ichT);
			}
		else  /* flip */			
			{
			cpFrom = cpTo;
			if (cpFrom != cpNil && cpFrom != cpMacLine)
				{
				cpTo = CpToFromCurs(&selCur, cpFrom, cpNil,
						sty, dSty, fEnd, fTrue/*fDrag*/, &fSplat);
				Assert(cpTo != cpNil);
				XpFromDcp(cpTo - cpFirstLine, cp0, &pt.xp, &ichT);
				}
			else  /* special flip case - no cp to base */				
				{
				if (dSty > 0)
					pt.xp = min(xpMargin, pt.xp + vsci.dxpTmWidth);
				else
					pt.xp = max(0, pt.xp - vsci.dxpTmWidth);
				}
			}
		}

	/*  free before scroll */
	FreePdrf(&drfFetch);
/* line xp to screen xp */
	pt.xp = XwFromXp(hpldr/*hwwd*/, idr, pt.xp);
	pt.yp = YwFromYp(hpldr, idr, pt.yp);

/* horizontal scroll if necessary */
/* DREWS - Coordinate system has flipped so sign for (*hwwdCur)->xeScroll */
/*	has been changed in all the following expressions */
	if (pt.xp < xpScroll && XwFromXp(hpldr, idr, 0) < PwwdWw(wwCur)->xwMin)
		{
		ScrollRight(wwCur, min(xpScroll, (xpScroll - pt.xp + vsci.dxpMinScroll)));
		goto LUpdate;
		}
	else  if (pt.xp >= (xpWwMac = ((*hwwdCur)->xwMac - (*hwwdCur)->xwMin + xpScroll)))
		{
		ScrollLeft(wwCur, pt.xp - xpWwMac + vsci.dxpMinScroll);
LUpdate:	
		UpdateWw(wwCur, fFalse);
		}

	if (selCur.fIns && !selCur.fHidden)
		ClearInsertLine(&selCur);
	caNotTable.doc = docNil;
	FSelectBlockTo(&selCur, pt, &caNotTable, fTrue);
LRet:
	if (vfRecording)
		RecordCursor(sty, dSty, fFalse/*fDrag*/);
}


/* C U R S  B L O C K  U P  D O W N */
/* %%Function:CursBlockUpDown %%Owner:davidlu */
CursBlockUpDown(sty, dSty, fDrag)
int sty, dSty;
BOOL fDrag; /* ignored */
{
	int dl;
	struct WWD *pwwd;
	struct PLCEDL **hplcedl;
	struct PLDR **hpldr;
	int idr;
	int dypWw;
	CP cpFirstLine, cpFirstT;
	int fEnd;
	int cThumb = 0;
	struct PT pt;
	struct EDL edl;
	struct CA caNotTable;
	struct DRF drfFetch;

	if (selCur.fIns)
		{ /* zero width block */
		Beep();
		return;
		}

	if (vfRecording)
		RecordCursor(sty, dSty, fFalse);

LStart0:
	Assert(selCur.fBlock);
	fEnd = fFalse;
	cpFirstLine = (selCur.fForward ? selCur.cpLim : selCur.cpFirst);

LStart:
	UpdateWw(wwCur, fFalse/*fAbortOK*/);
	dl = DlWhereDocCp(wwCur, selCur.doc, cpFirstLine, fEnd, &hpldr, &idr,
			&cpFirstLine, NULL, fTrue);
	if (dl < 0)
		{
/* the desired end of selCur is not on screen */
		if (vmerr.fMemFail)
			return;
		if (!PwwdWw(wwCur)->fPageView)
			ThumbToCp(wwCur, cpFirstLine, fEnd, fFalse, 0);
		else
			NormCp(wwCur, selCur.doc, cpFirstLine, 2, 0, fFalse);

		if (cThumb++ > 0)
			{
			Beep();
			return;
			}
		goto LStart;
		}
	Assert(idr != idrNil);
	hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
	Assert(hplcedl);

	pwwd = *hwwdCur;
/* DREWS - Coordinate system has flipped so sign for pwwd->xeScroll */
/*	has been changed in the following expression */
	pt.xp = XwFromXp(hpldr, idr, (selCur.fRightward ? selCur.xpLim : selCur.xpFirst));
	dypWw = pwwd->ywMac - pwwd->ywMin;

/* calculate desired dl from current dl */
	switch (sty)
		{
	case styLine:
		if (dSty < 0)
			{
			if (cpFirstLine == cp0)
				goto LBeep;

			if (dl == 0)
				{
				if (PwwdWw(wwCur)->fPageView)
					goto LBeep;
				FreePdrf(&drfFetch);
				ScrollDown(wwCur,vsci.dysMinAveLine,
						vsci.dysMacAveLine);
				goto LStart;
				}
			else  /* normal case, simply go up one line */
				--dl;
			dSty++;
			}
		else
			{
			int fEndDr;
			GetPlc( hplcedl, dl, &edl );
			if ((fEndDr = (dl == IMacPlc(hplcedl) - 1)) ||
					(YwFromYp(hpldr/*hwwd*/, idr, edl.ypTop + edl.dyp) > pwwd->ywMac - vsci.dysMinAveLine))
				{
				if (fEndDr && PwwdWw(wwCur)->fPageView)
					goto LBeep;
				FreePdrf(&drfFetch);
				ScrollUp( wwCur,vsci.dysMinAveLine,
						vsci.dysMacAveLine );
				goto LStart;
				}
			GetPlc( hplcedl, ++dl, &edl );
			if (edl.fEnd)
				{
LBeep:
				Beep();
				goto LRet;
				}
			--dSty;
			}
		FreePdrf(&drfFetch);
		break;

	case styScreen:
			{
			extern ScrollUp(), ScrollDown();
			FreePdrf(&drfFetch);
			(*(dSty < 0 ? ScrollDown : ScrollUp))
					(wwCur,max(dypWw - vsci.dysMacAveLine,5),
					max(dypWw - vsci.dysMacAveLine + vsci.dysMinAveLine, 10));
			if (dSty > 0)
				dSty--;
			else
				dSty++;
			break;
			}

	default:
		Beep();
LRet:
		FreePdrf(&drfFetch);
		return;
		}

/* do the selection */
	UpdateWw(wwCur, fFalse/*fAbortOK*/);
	selCur.fSelAtPara = fFalse;

	hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
	if (dl < 0 || dl >= IMacPlc(hplcedl))
		{
		Beep();
		FreePdrf(&drfFetch);
		}
	else
		{
		GetPlc( hplcedl, dl, &edl );
		pt.yp = edl.ypTop + edl.dyp - 1;
		DlWhereDocCp(wwCur, selCur.doc, CpPlc(hplcedl, dl), fEnd, &hpldr, &idr,
				&cpFirstT, NULL, fTrue);

		pt.yp = YwFromYp(hpldr/*hwwd*/, idr, pt.yp);

		caNotTable.doc = docNil;
		FreePdrf(&drfFetch);
		FSelectBlockTo(&selCur, pt, &caNotTable, fTrue);
		}
	if (dSty != 0)
		goto LStart0;
}


#endif /* WIN */
