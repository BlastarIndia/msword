/* S C R O L L . C */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "format.h"
#include "disp.h"
#include "doc.h"
#include "screen.h"
#include "scc.h"
#include "sel.h"
#include "layout.h"
#include "debug.h"
#include "keys.h"
#ifdef MAC
#include "mac.h"
#define CONTROLS
#define WINDOWS
#include "toolbox.h"
#endif


/* E X T E R N A L S */

extern int              vssc;
extern struct SCC       vsccAbove;
extern struct SCC       *vpsccBelow;
extern int              vcConseqScroll;
extern int              sbScroll;
extern int              sbScrollLast;
extern struct WCC       vwcc;
extern struct FLI       vfli;
extern struct CA        caPara;
extern struct CA        caTap;
extern struct DBS       vdbs;
extern struct MERR      vmerr;
extern CP               vcpFetch;
extern int              vccpFetch;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct PREF      vpref;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vfLastScrollDown;
extern int              vfLastScroll;
extern int              vfSeeSel;
extern int              wwMac;
extern struct WWD       **mpwwhwwd[];
extern int              vfCheckPlc;
extern int              wwCur;

extern struct DR *PdrWw();

#ifdef WIN
extern int              vlm;
extern int              vflm;
extern struct SCI       vsci;
int cdlScroll;
extern BOOL fCdlScrollSet;
#endif /* WIN */

#ifdef MAC
extern int vwwScroll;
extern int wwHelp;
#endif



#ifdef NOTUSED
/* S C R O L L  U P  E N D */
/* scroll up, but check for endmark not moving higher than midpoint */
/* %%Function:ScrollUpEnd %%Owner:NOTUSED */
ScrollUpEnd(ww, dyp)
int dyp;
{
	int dl;
	struct WWD *pwwd = PwwdWw(ww);
	struct EDL edl;

	Assert(!pwwd->fPageView);
	if ((dl = DlFromCpCheck(HwwdWw(ww), 0, CpMacDoc(PdrGalley(pwwd)->doc), fFalse)) > 0)
		{
		GetPlc(PdrGalley(pwwd)->hplcedl, dl - 1, &edl);
		dyp = min(dyp, max(0, edl.ypTop + edl.dyp -
				PdrGalley(pwwd)->dyl / 2));
		}
	ScrollUp(ww, dyp, dyp + WinMac(dysMinAveLineSci, 24));
}


#endif /* NOTUSED */


/* S C R O L L  U P */
/* scrolls current window contents up (down in the document) by
at least dypLow but at most dypHigh. Within these constraints procedure
will find the least amount to scroll to a even line boundary if
possible.

Plan:
	1. find dl with top yp pos at least dypLow below the start of the
	window. Choose dypAboveNew depending on dypHigh. Do not scroll
	below fEnd dl.
	2. let dlFirstFull be first full height line. Copy lines
	[dlFirstFull, dl] to sccAbove.
		To do this, we have to determine number of dl's to be
		deleted from the front of the scc, taking into account
		the dl and yp limits.
	3. scroll window contents so that dl will be first line and
	its top position will be dypAbove above ypMin.
	cpFirst of window to get cpMin of dl0.
	Window is marked dirty so that UpdateWindow can do the rest.
*/
/* %%Function:ScrollUp %%Owner:bryanl */
ScrollUp(ww, dypLow, dypHigh)
int dypLow, dypHigh;
{
	int dlMacOld;
	struct WWD *pwwd, **hwwd;
	int ypLow;
	int dypAbove, dypVisi;
	int dl, cdl, cdlAboveNew;
	int dlMac, cdlT;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	Debug(int fCheckPlcSave = vfCheckPlc);

	vfLastScroll = fTrue;
	vfLastScrollDown = fFalse;
	vfSeeSel = fFalse;
	UpdateWw(ww, fFalse);


	pwwd = *(hwwd = HwwdWw(ww));
	if (pwwd->fPageView)
		{
		ScrollPageDye(ww, dypLow);
		return;
		}

#ifdef WIN
	cdlScroll=0;
	fCdlScrollSet=fTrue;
#endif

	Debug(vfCheckPlc = fFalse);
	hplcedl = PdrGalley(pwwd)->hplcedl;
	if (hplcedl == hNil)
		return; /* can happen coming out of pageview from low in memory */
	FreezeHp();

	ClearInsertLine(&selCur);
	ClearInsertLine(&selDotted);
	ypLow = dypLow;
	cdlT = IMacPlc(hplcedl);
	if (cdlT == 0)
		{
		MeltHp();
		Beep();
		return;
		}
		{{ /* NATIVE - ScrollUp */
		for (dl = 0; cdlT-- > 0; dl++)
			{
			GetPlc(hplcedl, dl, &edl);
			if (edl.fEnd || edl.ypTop + edl.dyp >= ypLow)
				break;
			}
		}}
	if (edl.fEnd)
		{
/* scroll end dl to the top */
		dypAbove = 0;
/* but don't be scrollin' past CpMacDoc */
		if (CpPlc(hplcedl, dl) > CpMacDoc(PdrGalley(pwwd)->doc))
			--dl;
		Assert(CpPlc(hplcedl, dl) <= CpMacDoc(PdrGalley(pwwd)->doc));
		}
	else  if ((dypVisi = edl.ypTop + edl.dyp - dypHigh) > 0)
/* scroll to line dl, with dypAbove to limit scrolling */
		{
		dypAbove = edl.dyp - dypVisi;
		Assert(dypAbove > 0 && dypAbove < edl.dyp);
		}
	else
		{
/* scroll to line dl+1, with dypAbove=0 */
		dl++;
		dypAbove = 0;
		}
/* scroll line dl to the top with dypAbove hanging out */
#ifdef MAC
	if (ww == wwHelp && FScrollHelp(dl, dypAbove))
		return;	/* FScrollHelp melted heap */
#endif

/* save full height lines in sccAbove. NOTE: dlFirstFull is 0/1 depending
on pwwd->dypAbove != 0. cdlAboveNew means that the line dl itself
will have to be saved in the cache above if the new dypAbove is not 0 */
	cdlAboveNew = dypAbove != 0;
	if ((cdl = dl - (PdrGalley(pwwd)->dypAbove != 0) + cdlAboveNew) > 0) /* # of dl's to be copied */
		{
#ifdef MAC
		if (vpsccBelow)
/* take care of bitmaps */
			MoveToSccAbove(ww, dl - cdl + cdlAboveNew, dl + cdlAboveNew);
		else
#endif
				{
				if (PdrGalley(pwwd)->cpFirst != vsccAbove.rgcp[vsccAbove.plcedl.dlMac])
					PutIMacPlc(vsccAbove.hplcedl, 0);
				if (vsccAbove.plcedl.dlMac + cdl > vsccAbove.plcedl.dlMax - 1)
					{
					cdl = min(cdl, vsccAbove.plcedl.dlMax - 1);
				/* make room in dndl by shoving dl's toward 0 */
					dlMacOld = vsccAbove.plcedl.dlMac;
					PutIMacPlc(vsccAbove.hplcedl, vsccAbove.plcedl.dlMax - 1 - cdl);
					Assert(vsccAbove.plcedl.dlMac >= 0);
					MoveEdls(&vsccAbove, &vsccAbove, dlMacOld - vsccAbove.plcedl.dlMac, 0,
							vsccAbove.plcedl.dlMac);
					}
/* now there is dl room above dlMac for cd dl's. Copy edl's from window
to cache starting at dl - cdl + cdlAboveNew. Note: cdl might have been clipped by
dlMax */

				MoveEdls(pwwd, &vsccAbove, dl - cdl + cdlAboveNew,
						vsccAbove.plcedl.dlMac, cdl);
				PutIMacPlc(vsccAbove.hplcedl, vsccAbove.plcedl.dlMac + cdl);
				Assert(vsccAbove.plcedl.dlMac >= 0);
				}
		}
/* now scroll window contents */
	PdrGalley(pwwd)->cpFirst = CpPlc(hplcedl, dl);
	if (dl != 0)
		{
		GetPlc(hplcedl, dl - 1, &edl);
		PdrGalley(pwwd)->dcpDepend = edl.dcpDepend;
		}

	MeltHp();
	dlMac=IMacPlc(hplcedl);
	if (dl < dlMac && FScrollOK(ww))
		{
		GetPlc(hplcedl, dl, &edl);

		ScrollDrUp(ww, hwwd, 0, PdrGalley(*hwwd)->hplcedl, dl, 0 /*dlTo*/,
				edl.ypTop, -dypAbove /*ypTo*/,
				YpFromYw(hwwd, 0, (*hwwd)->ywMin) /* ypFirstShow */,
				YpFromYw(hwwd, 0, (*hwwd)->ywMac) /*ypLimShow*/);
		}
#ifdef WIN
	if (dl <= dlMac)	cdlScroll=dl;
#endif
	pwwd = *hwwd;
	pwwd->fDirty = fTrue;
	PdrGalley(pwwd)->dypAbove = dypAbove;
	pwwd->fSetElev = fTrue;
	Debug(vfCheckPlc = fCheckPlcSave);
#ifdef MAC
	if (vpsccBelow && FScrollOK(ww))
		ScrollUpLarge(ww);
#endif	/* MAC */
	SynchCheck(ww, fFalse /*fCkVisibility*/);
}


/* S C R O L L  D O W N */

/* Scrolls current window contents down (up in the document) by at least
dypLow but at most dypHigh. Within these constraints procedure
will find the least amount to scroll to a even line boundary if
possible.

Plan:
	1. if first line in ww is partially visible, consider next line
	as dlFirst and increment parameters by the dyp that is visible.
	so that params are always relative to a base dl called dlFirst.
	2. determine the actual cdlTotal and total height dypTotal
	taken up by the lines we have to scroll over. This is done by
	looping within the cache sccAbove, and if there is not sufficient
	data in there, reload cache, decrement parameters by amount
	already seen, and loop.
	3. once we have the precise height we need to back up, the
	window can be scrolled by that much. UpdateWindow will take care
	of the rest. The empty space will be invalidated and in large
	systems refilled from sccAbove.

*/
/* %%Function:ScrollDown %%Owner:bryanl */
ScrollDown(ww, dypLow, dypHigh)
int ww, dypLow, dypHigh;
{
	struct WWD *pwwd = PwwdWw(ww);

	vfLastScroll = fTrue;
	vfLastScrollDown = fTrue;
	vfSeeSel = fFalse;
	if (pwwd->fPageView)
		{
		ScrollPageDye(ww, -dypHigh);
		return;
		}

	ScrollDown2(ww, 0, dypLow, dypHigh, WinMac( fFalse, vpsccBelow && !pwwd->fOutline));
}


/* %%Function:ScrollDown2 %%Owner:bryanl */
ScrollDown2(ww, idr, dypLow, dypHigh, fUseScc)
int ww, idr, dypLow, dypHigh, fUseScc;
{
	int dl, dlFirst;
	int ypFirst;
	int dypVisi, dypTotal, dypAbove, ypLimWw;
	int cdlTotal;
	struct PLCEDL **hplcedl;
	struct WWD *pwwd = PwwdWw(ww);
	BOOL fDirty;
	struct EDL *dndl;
	int ypTo;
	int dlMac;
	struct DR *pdr;
	struct EDL edl;
	Debug(int fCheckPlcSave = vfCheckPlc;)

			Win( Assert( fUseScc == fFalse ) );
	if (idr >= pwwd->idrMac)
		return;	/* failsafe for page view problems; 0 may not exist */
	Debug(vfCheckPlc = fFalse);
	fDirty = pwwd->fDirty;
	hplcedl = PdrWw(ww, idr)->hplcedl;
	if (hplcedl == hNil)
		return;  /* can happen coming out of pageview from low in memory */

	dlFirst = 0;
	dypVisi = 0;
	ypFirst = 0;

#ifdef WIN
	cdlScroll = 0;
	fCdlScrollSet = fTrue;
#endif

	ClearInsertLine(&selCur);
	ClearInsertLine(&selDotted);
	if (PdrWw(ww, idr)->dypAbove)
		{
		GetPlc(hplcedl, 0, &edl);
		ypFirst = edl.ypTop + edl.dyp;
		dypVisi = ypFirst;
		dlFirst = 1;
		dypLow += dypVisi;
		dypHigh += dypVisi;
		}
	dypTotal = DypScanAbove(ww, dypLow, &cdlTotal);
	if (dypLow == 0)
		dypTotal = cdlTotal = 0;
	dypAbove = 0;
	if (dypTotal > dypHigh)
		{
		/* High limitation exceeded, show only part of last line */
		dypAbove = dypTotal - dypHigh; 
		dypTotal = dypHigh;
		}
	if (dlFirst != 0)
		{
		cdlTotal--;
		Break2();
		dypTotal -= dypVisi;
		Assert(dypTotal >= 0);
		}
/* now we have: dypTotal, cdlTotal describing precise amount to scroll down
from dl0 and ypMin, the desired dypAbove, and the cp of the proposed top line
in pwwd->cpFirst. */
/* ensure there are at least cdlTotal + dlFirst dl's in plcedl */
	dlMac = IMacPlc(hplcedl);
	if (!FRareF(300, FStretchPlc(hplcedl,
			cdlTotal + dlFirst - dlMac)))
		goto LEnd;

	if (dypTotal < PdrWw(ww, idr)->dyl && FScrollOK(ww))
		{
/* prepare UpdateWindow by scrolling down window contents and invalidating
the first cdlTotal lines. The above "if" means no sense in scrolling if the
whole window is invalid anyway */
		ypTo = ypFirst + dypTotal;
#ifdef MAC
		if (fUseScc && !fDirty)
			MoveToSccBelow(ww, dlFirst, ypTo);
#endif
		if (dypTotal != 0 && dlFirst < dlMac)
	/* check is here to prevent displaying for "cpFirst blessing" scrolls by 0,
eg. after window splits. */
			{
			pwwd = PwwdWw(ww);
			GetPlc(hplcedl, dlFirst, &edl);
			ypLimWw = YpFromYw(HwwdWw(ww), idr, pwwd->ywMac);


			ScrollDrDown(ww, HwwdWw(ww), idr, hplcedl, dlFirst, dlFirst + cdlTotal /*dlTo*/,
					edl.ypTop, ypTo,
					YpFromYw(HwwdWw(ww), idr, pwwd->ywMin) /*ypFirstShow*/,
					ypLimWw, ypLimWw);
			}
/* line 0 is always invalid, dlFirst may be 0 or 1 */
		for (dl = 0; dl < cdlTotal + dlFirst; dl++)
			{
			GetPlc(hplcedl, dl, &edl);
			edl.fDirty = fTrue;
			PutPlcLast(hplcedl, dl, &edl);
			}
		}
#ifdef WIN
	if (cdlTotal <= dlMac)
		cdlScroll = cdlTotal;
#endif
LEnd:
	PwwdWw(ww)->fDirty = fTrue; /* for the benefit of the cpFirst == 0 case */
	pdr = PdrWw(ww, idr);
	pdr->fCpBad = fFalse;
	pdr->dypAbove = dypAbove;
	Debug(vfCheckPlc = fCheckPlcSave);
#ifdef MAC
	if (fUseScc)
		ScrollDownLarge(ww, cdlTotal + dlFirst);
#endif
	if (dypLow)
		{
		SynchCheck(ww, fFalse/*fCkVisibility*/);
		PwwdWw(ww)->fSetElev = fTrue;
		}
}


/* D Y P  S C A N  A B O V E */
/* retreats over lines > dyp in height, # of lines and actual height
skipped over are returned. wwdCur.cpFirst, fCpDirty are set to start
of "last" line skipped over.
Skipping starts just above the first full line in the window.
*/
/* %%Function:DypScanAbove %%Owner:bryanl */
int DypScanAbove(ww, dyp, pcdl)
int ww;
int dyp, *pcdl;
{
	CP CpFirstFull();
	int dypTotal = 0;
	int cdlTotal = 0;
	int cdl, dl;
	int wwT;
	struct EDL *pedl;
	struct WWD *pwwd = PwwdWw(ww);
	CP cpFirst = CpFirstFull(pwwd);
	int fSynchScc = fFalse;
	struct DRF drfFetch;
	struct DR *pdr;

	if (cpFirst == 0 || pwwd->idrMac <= 0)
		{
		*pcdl = 0;
		return 0;
		}

	if (ww != vsccAbove.ww)
		{
		wwT = vsccAbove.ww;
		SynchSccWw(&vsccAbove, ww);
		fSynchScc = fTrue;
		}
/* first we ensure that the contents of sccAbove has a seamless join
with the first full line in the current window */
	Assert (PdrFetchAndFree(HwwdWw(ww), 0 /* idr */, &drfFetch)->doc == vsccAbove.doc);
	if (vsccAbove.plcedl.dlMac > 0)
		{
		if (vsccAbove.rgcp[vsccAbove.plcedl.dlMac] != cpFirst)
			{
/* if desired point is in the cache but not at its end.. */
			PutIMacPlc(vsccAbove.hplcedl, max(0,
					IInPlcRef(vsccAbove.hplcedl, cpFirst)));
			Assert(vsccAbove.plcedl.dlMac >= 0);
			if (vsccAbove.fBreak && vsccAbove.dlBreak >= vsccAbove.plcedl.dlMac)
				{
				vsccAbove.fBreak = fFalse;
				vsccAbove.dlBreak = 0;
				}
			}
#ifdef MAC
		if (vpsccBelow)
/* eliminate fBreak in the cache */
			if (vsccAbove.fBreak && vsccAbove.plcedl.dlMac != 0)
				{
				Assert(!vsccAbove.fBreak || vsccAbove.dlBreak <= vsccAbove.plcedl.dlMac);
				ScrollScc(&vsccAbove, vsccAbove.dlBreak,
						-vsccAbove.dlBreak, 0);
				vsccAbove.fBreak = fFalse;
				}
#endif
		}

	vsccAbove.fFull = fFalse;
	cdl = vsccAbove.plcedl.dlMac;
	for (;;)
		{
		/* for each cache load [0, cdl) */
		pdr = PdrFetch(HwwdWw(ww), 0 /* idr */, &drfFetch);
		for (dl = cdl - 1; dl >= 0; dl--)
			{
			if (dyp == 0 && cpFirst == vsccAbove.rgcp[cdl])
				goto LRet2;
			cdlTotal++;
			pedl = &(vsccAbove.dndl[dl]);
			if (((dypTotal += pedl->dyp) >= dyp)
					|| (vsccAbove.rgcp[dl] == cp0))
				/* line found in cache */
				{
LRet1:

				pdr->cpFirst = vsccAbove.rgcp[dl];
/* 0 should really be vsccAbove.dcpDepend, and the latter should be maintained */
				pdr->dcpDepend = dl == 0 ? 0 :
						vsccAbove.dndl[dl -1].dcpDepend;
				PwwdWw(ww)->fDirty = fTrue;
LRet2:
				pdr->fCpBad = fFalse;
				*pcdl = cdlTotal;
				FreePdrf(&drfFetch);

				if (fSynchScc)
					SynchSccWw(&vsccAbove, wwT);
				return (dypTotal);
				}
			}
		FreePdrf(&drfFetch);
		/* not in cache, must extend further backward */
/* cache is loaded backwards "starting" from cpFirst of the current window */
		cdl = CdlEnrichSccAbove(ww, cpFirst);
		if (cdl == 0)
			{
			dl = 0;
			pdr = PdrFetch(HwwdWw(ww), 0 /* idr */, &drfFetch);
			goto LRet1;
			}
		}

	if (fSynchScc)
		SynchSccWw(&vsccAbove, wwT);

}


/* C D L  E N R I C H  S C C  A B O V E */
/*
Extends the sccAbove cache backwards by inserting some more dl's in front.
Returns the number of new edls. First new edl will be at dl = 0;
As dl's accumulate, excess will be pushed off the high end, then when
the point of insertion reaches the high end, excess is pushed off the low end.

Follows EnrichSccAbove plan loosely.
Uses wwCur. Assumes sccAbove matches wwCur.
Strats from cpFirst if cache is empty.
*/
/* %%Function:CdlEnrichSccAbove %%Owner:bryanl */
CdlEnrichSccAbove(ww, cpFirst)
int ww;
CP cpFirst;
{
	CP CpFirstFull();
	int dlMax = vsccAbove.dlMax;
	int dlMac = vsccAbove.plcedl.dlMac; /* will be kept up to date */
	int cdl = 0; /* total number of lines inserted */
	int dl;
	int ipad;
	struct EDL *pedl;
	BOOL fOutline;
	int dyp;
	int dcpDepend;
	CP cpT, cpLim, cpMin;
	int dypT1, dypT2;
#ifdef MAC
	struct WDS wds;
#endif

	fOutline = PwwdWw(ww)->fOutline;

	cpLim = dlMac == 0 ? cpFirst : vsccAbove.rgcp[0];
/* skip over invisible stuff directly above cpFirst */
	if (fOutline && cpLim != cp0)
		{
		struct PLCPAD **hplcpad = (PdodDoc(PdrGalley(PwwdWw(ww))->doc)->hplcpad);
		int ipad;
		struct PAD pad;
		if (hplcpad != hNil)
			{
			ipad = IInPlc(hplcpad, cpLim - 1);

			while (ipad >= 0)
				{
				GetPlc(hplcpad, ipad, &pad);
				if (pad.fShow)
					break;
				cpLim = CpPlc(hplcpad, ipad--);
				}
			}
		}
	if (cpLim == 0)
		return 0;

#ifdef MAC
	SaveWds(&wds);
	PwwdSetWw(ww, cptDoc);
	cpLim = CpMin(cpLim, CpMacDoc(vsccAbove.doc));
	BackOneNotFlatPara(vsccAbove.doc, cpLim);
	cpT = caPara.cpFirst;
#else
	cpLim = CpMin(cpLim, CpMacDoc(vsccAbove.doc));
	cpT = CpFormatFrom( ww, vsccAbove.doc, cpLim-1);
#endif
	dl = 0;
	if (fOutline)
		ClearCap();
	for (;;)
		{
		int dlT = dl;
		if (fOutline)
			cpT = CpOutlineAdvance(vsccAbove.doc, cpT);
		if (cpT >= cpLim) break;
/* pointer to the EDL which is to be set up from formatting results */
		pedl = &(vsccAbove.dndl[dlT]);
		if (dl < dlMax)
			{
			int cdlT = (dlMac - dl);
			cdl++;
			if (dlMac < dlMax)
/* move all EDL's up one, increment dlMac */
				{
				dlMac++;
				PutIMacPlc(vsccAbove.hplcedl, dlMac);
				}
			else  /* dlMac = dlMax */
/* move all EDL's up, overwriting last line */
				cdlT--;
			MoveEdls(&vsccAbove, &vsccAbove, dl, dl + 1, cdlT);
			dl++;
			}
		else  /* dl == dlMax */			
			{
/* move all EDL's down, losing the first one */
			pedl--; 
			dlT--;
			MoveEdls(&vsccAbove, &vsccAbove, 1, 0, dlMax - 1);
			}
		CachePara(vsccAbove.doc, cpT);
		if (!FInTableVPapFetch(vsccAbove.doc, cpT))
			{
/* format line starting at cpT, to be stored at dl in sccAbove.dndl */
			FormatLine(ww, vsccAbove.doc, cpT);
			cpMin = vfli.cpMin;
			cpT = vfli.cpMac;
			dcpDepend = vfli.dcpDepend;
			dyp = vfli.dypLine;
			}
		else
			{
/* format table row to determine same quantities */
			CpFirstTap(vsccAbove.doc, cpT);
			cpMin = caTap.cpFirst;
			cpT = caTap.cpLim;
			dcpDepend = 0;
			dyp = DysHeightTable(ww, vsccAbove.doc, cpT - 1,
					fFalse, fFalse, &dypT1, &dypT2);
			dyp += dypT1 + dypT2;
			}
/* set up EDL */
		vsccAbove.rgcp[dlT] = cpMin;
		vsccAbove.rgcp[dlT + 1] = cpT;
		pedl->dcp = (uns) (cpT - cpMin);

		pedl->ypTop = 0;
		pedl->dyp = dyp;
		pedl->fHasBitmap = fFalse;
		pedl->fDirty = fFalse;
		pedl->dcpDepend = dcpDepend;
		}
	SetMacsScc(&vsccAbove);
#ifdef MAC
	RestoreWds(&wds);
#endif
	return (min(cdl, dlMax));
}


/* C P  F I R S T  F U L L */
/* returns the cpMin of the first full line. Note: does not assume that
dlMac > 1! */
/* %%Function:yCpFirstFull %%Owner:bryanl */
CP CpFirstFull(pwwd)
struct WWD *pwwd;
{
	return (PdrGalley(pwwd)->dypAbove == 0 ?
			PdrGalley(pwwd)->cpFirst :
			CpPlc(PdrGalley(pwwd)->hplcedl, 1));
}


/* T H U M B  T O  C P */
/* Scrolls to a given cp. Searches all caches for the cp to make the
operation more efficient.
Uses wwCur.
Plan:
	1. Is cp in the window? if so, we have new cpFirst, set fDirty, return
	2. Is it in sccAbove? if so, ScrollDown the appropriate amount.
	(3. Large sys only. Is it in sccBelow? if so, ScrollUp.)
	4. Otherwise set fCpBad and let UpdateWw handle it.

fNoOpt is used in help to avoid scrolling the endmark to the upper part of the
screen.
ncp is either 0 or ncpForceYPos. the latter releases the restriction at the 
end of document.
*/

/* %%Function:ThumbToCp %%Owner:bryanl */
ThumbToCp(ww, cp, fEnd, fNoOpt, ncp)
CP cp; 
int fEnd;
{
	struct EDL *dndl;
	int dyp, dypBound;
	int dl;
	struct WWD *pwwd;
	struct PLCEDL **hplcedl;

	UpdateWw(ww, fFalse);
	vfSeeSel = fFalse;
	pwwd = PwwdWw(ww);
	if (pwwd->idrMac <= 0)
		return;

	Assert(!pwwd->fPageView);
	hplcedl = PdrGalley(pwwd)->hplcedl;
	if (hplcedl == hNil)
		return; /* can happen coming out of pageview from low in memory */

	dypBound = ncp & ncpForceYPos ? 0 : PdrGalley(pwwd)->dyl / 2;
	if (!fNoOpt && (dl = DlFromCpCheck(HwwdWw(ww), 0, cp, fEnd)) >= 0)
		{
		struct EDL edl;
		int dyp = -PdrGalley(pwwd)->dypAbove, dypEnd;
		int dlT;
		for (dlT = 0; dlT < dl; dlT++)
			{
			GetPlc(hplcedl, dlT, &edl);
			dyp += edl.dyp;
			}
/* search for end mark. do not scroll it above midpoint. */
		for (dypEnd = dyp; dlT < IMacPlc(hplcedl); dlT++)
			{
			GetPlc(hplcedl, dlT, &edl);
			if (edl.fEnd)
				{
				dyp = min(dyp, max(0, dypEnd - dypBound));
				break;
				}
			dypEnd += edl.dyp;
			}
		GetPlc(hplcedl, 0, &edl);
		if (dyp >= edl.dyp /* will scroll over at least first line */)
			ScrollUp(ww, dyp, dyp + WinMac(dysMinAveLineSci, 18));
		goto LSynch;
		}
	else
		{
		CP cpMac = CpMacDoc(PdrGalley(pwwd)->doc);
/* limitation on scrolling endpoint into upper half of screen. Do this only
when near the end of the document because it is relatively expensive */
		if (cpMac - 400 < cp)
			{
			int cdlT;
			PdrGalley(pwwd)->cpFirst = cpMac;
			pwwd->fDirty = fTrue;
			PdrGalley(pwwd)->dypAbove = 0;
			DypScanAbove(ww, dypBound, &cdlT);
			if (cp >= PdrGalley(PwwdWw(ww))->cpFirst)
				goto LSynch;
			}
		else
			{
			struct SCC *psccAbove = &vsccAbove;
			if (vsccAbove.ww == ww &&
					(dl = DlFromCpCheck(&psccAbove, 0, cp, fEnd)) >= 0)
				{
/* check if above cache matches ww seamlessly */
				if (vsccAbove.rgcp[vsccAbove.plcedl.dlMac] ==
						PdrGalley(pwwd)->cpFirst)
					{
					for (dyp = 0; dl < vsccAbove.plcedl.dlMac; dl++)
						dyp += vsccAbove.dndl[dl].dyp;

					ScrollDown(ww, dyp, dyp + WinMac(dysMinAveLineSci, 24));
					goto LSynch;
					}
				}
#ifdef MAC
			if (vpsccBelow && !PwwdWw(ww)->fOutline)
				{
				ThumbToCpLarge(ww, 0,  cp, fEnd);
				goto LSynch;
				}
#endif	/* MAC */
			}
		ClearSccs();
		pwwd = PwwdWw(ww);
		PdrGalley(pwwd)->cpFirst = cp;
		PdrGalley(pwwd)->fCpBad = fTrue;
		}
	pwwd->fDirty = fTrue;
LSynch:
	pwwd = PwwdWw(ww);
	if (PdrGalley(pwwd)->dypAbove)
		PdrGalley(pwwd)->dypAbove = 0, pwwd->fDirty = fTrue;
	Mac( PwwdWw(ww)->fSetElev = fFalse );
	Win( PwwdWw(ww)->fSetElev = fTrue );
	SynchCheck(ww, fFalse/*fCkVisibility*/);
}


/* S E T  M A C S  S C C */
/* sets ypMac and rgcp[dlMac] from dl's; copy dlMac to plcedl. */
/* %%Function:SetMacsScc %%Owner:bryanl */
SetMacsScc(pscc)
struct SCC *pscc;
{
	int dlMac = pscc->plcedl.dlMac;
	int ywMac = 0;
	CP cpMac = 0;
	int dl;
	if (dlMac == 0) pscc->fBreak = fFalse;
	if (dlMac && pscc == &vsccAbove)
		{{ /* NATIVE - SetMacsScc */
		struct EDL *pedl;

		pedl = &(pscc->dndl[0]);
		for (dl = 0 ; dl < dlMac ; dl++, pedl++)
			{
			if (pedl->fHasBitmap)
				ywMac += pedl->dyp;
			}
		pedl = &(pscc->dndl[dlMac - 1]);
		cpMac = pscc->rgcp[dlMac - 1] + pedl->dcp;
		}}
	else  if (dlMac)
		{
		struct EDL *pedl = &(pscc->dndl[dlMac - 1]);
		if (pedl->fHasBitmap)
			ywMac = pedl->ypTop + pedl->dyp;
		cpMac = pscc->rgcp[dlMac - 1] + pedl->dcp;
		}

	Assert(ywMac >= 0 && dlMac >= 0);
	pscc->rgcp[dlMac] = cpMac;
	pscc->ywMac = ywMac;
	PutIMacPlc(pscc->hplcedl, dlMac);
}


/* M O V E  E D L S */
/* blt's edl's and cp's in plcedl. Move is assumed to end at dlMac, hence
the number of cp's moved is one greater than cdl */
/* %%Function:MoveEdls %%Owner:bryanl */
MoveEdls(pwwdFrom, pwwdTo, dlFrom, dlTo, cdl)
struct WWD *pwwdFrom, *pwwdTo;
int dlFrom, dlTo, cdl;
{
	struct EDL *pedl, *pedlLim;
	struct PLCEDL *pplcedl;
#ifdef DEBUG
	int *pdlMacT = &((*(PdrGalley(pwwdFrom)->hplcedl))->dlMac);
	int dlMacT = *pdlMacT;
	*pdlMacT = *(pdlMacT + 1);
#endif

	/* DavidMck says this needs to be rethought for Mac.
	/* Remember that edl.dyl for a table row can be artificially
	/* small if the row was clipped to the parent DR
	/**/
	Mac(Assert(fFalse));
	CopyMultPlc(cdl, PdrGalley(pwwdFrom)->hplcedl, dlFrom,
			PdrGalley(pwwdTo)->hplcedl, dlTo, cp0, 0, 1);

	Assert(pwwdTo == &vsccAbove Mac(|| pwwdTo == vpsccBelow));
	pplcedl = *PdrGalley(pwwdTo)->hplcedl;
	Assert(!pplcedl->fExternal);
	pedl = (struct EDL *)&pplcedl->rgcp[pplcedl->dlMax];
	pedlLim = pedl + cdl;
	for ( ;pedl < pedlLim; ++pedl)
		pedl->hpldr = hNil;
	Debug(*pdlMacT = dlMacT);
}


/* S Y N C H  C H E C K */
/* check if SynchRef or SynchOutline should be called
after scrolling ww.
*/
/* %%Function:SynchCheck %%Owner:bryanl */
SynchCheck(ww, fCkVisibility)
int ww;
BOOL fCkVisibility;
{
	struct WWD *pwwd = PwwdWw(ww);
	struct WWD *pwwdOther;

	if (PmwdWw(ww)->fSplit && !pwwd->fPageView && !(pwwdOther =  PwwdOther(ww))->fPageView)
		{
		if (pwwd->wk == wkFtn || pwwdOther->wk == wkFtn)
			SynchRef(ww, fCkVisibility);
#ifdef WIN
		else  if (pwwd->wk == wkAtn || pwwdOther->wk == wkAtn)
			SynchRef(ww, fCkVisibility);
		else  if (pwwd->wk == wkHdr || pwwdOther->wk == wkHdr)
			return;
#endif
		else  if (pwwd->fOutline != pwwdOther->fOutline)
			SynchOutline(ww);
		}
}


#ifdef MAC
/* B A C K  O N E  N O T  F L A T  P A R A */
/* A flat paragraph is one consisting of all vanished characters.
Prec returns greatest caPara.cpFirst < cpLim not preceded by flat para
and followed by exactly one non-flat para.
Used to back over text which may contain fully vanished paragraphs. */
/* %%Function:BackOneNotFlatPara %%Owner:NOTUSED */
BackOneNotFlatPara(doc, cpLim)
int doc; 
CP cpLim;
{
	BOOL fOneSeen = fFalse;
	Assert (cpLim > cp0);
	for (;;)
		{
		CachePara(doc, cpLim - 1);
		if (vpref.fSeeHidden) return;
		FetchCp(doc, caPara.cpFirst, fcmProps);
		for (;;)
			{
			if (!vchpFetch.fVanish)
				{
				if (fOneSeen)
					{
					CachePara(doc, caPara.cpLim);
					return;
					}
				else
					{
					fOneSeen = fTrue;
					goto LLoop;
					}
				}
			if (vcpFetch + vccpFetch >= caPara.cpLim) break;
			FetchCp(docNil, cpNil, fcmProps);
			}
/* flat paragraph or first non-flat finished. Try previous. */
LLoop:          
		if ((cpLim = caPara.cpFirst) == cp0)
			return;
		}
}


#endif /* MAC */

/* S E T  W W  C P  F I R S T */
/* class store routine for wwd.cpFirst */
/* %%Function:SetWwCpFirst %%Owner:bryanl */
SetWwCpFirst(ww, cpFirst)
int ww; 
CP cpFirst;
{
	struct WWD *pwwd = PwwdWw(ww);

	PdrGalley(pwwd)->cpFirst = cpFirst;
	PdrGalley(pwwd)->dypAbove = 0;
	PdrGalley(pwwd)->fCpBad = fTrue;
	pwwd->fDirty = fTrue;
	pwwd->fSetElev = fTrue;
}


/* S Y N C H  R E F */
/* synch other pane with wwMaster. Know: pwwd->fFtn/pwwd->fAtn.
if master is main text:
	find first ref r with cp>=first cp displayed.
	find corresponding footnote text cpFirst.
	if cpFirst not visible: scroll to cpFirst
else master is footnote text
	find cpFirst of reference to first displayed footnote text.
	return if none displayed.
	if cpFirst not visible: scroll to cpFirst
*/
/* %%Function:SynchRef %%Owner:bryanl */
SynchRef(ww, fCkVisibility)
int ww;
BOOL fCkVisibility;
{
	struct WWD *pwwd = PwwdWw(ww);
	CP cpFirst;
	int docSync;
	CP cpRefMom;
	struct SEL *psel = PselActive();

	Assert(!pwwd->fPageView);
/* first set cpFirst to the master ww */
	if (fCkVisibility && FCpVisible(ww, psel->doc, psel->cpFirst, fFalse, fFalse, fFalse))
		{
		cpFirst = CpActiveCur();
		pwwd = PwwdWw(ww);
		}
	else
		{
		pwwd = PwwdWw(ww);
		cpFirst = PdrGalley(pwwd)->cpFirst;
		}

	if (cpFirst >= CpMacDoc(PdrGalley(pwwd)->doc)) return;

	if ((docSync = DocCpRefFromWw(ww, &cpFirst, &cpRefMom)) == docNil)
		return;

	if (PmwdWw(ww)->wwUpper == ww && fCkVisibility &&
			!FCpVisible(ww, psel->doc, cpRefMom, fFalse, fFalse, fFalse))
/* cpRef in mother doc is not yet in window, don't have to scroll ref text in yet */
		return;

	if (PwwdOther(ww)->fDirty || !FCpVisible(WwOther(ww), docSync, cpFirst, fFalse, fFalse, fFalse))
		SetWwCpFirst(WwOther(ww), cpFirst);
}




/* F  S C R O L L  P A G E  D Y E  H O M E */
/* scrolls vertically, with stops at the "home" positions, and with
automatic paging forward and backward. Returns true iff paging is called.
(dye<0 means text is moved down in the window)
if fHome is false, stop at home point is suppressed.
if ncp has ncpForceYPos set, limitation on gray area is suppressed in up.
*/
/* %%Function:FScrollPageDyeHome %%Owner:bryanl */
FScrollPageDyeHome(ww, dye, fHome, ncp, fNoNewPage)
{
	struct WWD *pwwd = PwwdWw(ww);
	int dyeHome = pwwd->yeScroll - YeTopPage(ww);
	int w;
	BOOL fSmallPage = FSmallPage(ww, &w, &w);

#ifdef WIN
	int ipgdNew;
#endif
	if (dye >= 0)
		{
		if ((ncp & ncpForceYPos) == 0)
			dye = min(dye, DyOfRc(&pwwd->rcePage) + dypGrayOutsideSci
					- DyOfRc(&pwwd->rcwDisp) + pwwd->yeScroll);
		if (dye	<= 0 || fSmallPage)
			{
			if (fNoNewPage) return fTrue;
#ifdef MAC
			vwwScroll = ww;
			ScrollSpc(spcPgvPageDown);
#else /* WIN */
			if ((ipgdNew = IpgdNextWw(ww, fTrue)) != ipgdNil)
				SetPageDisp(ww, ipgdNew, YeTopPage(ww), fFalse, fFalse);
			else
				Beep(); /* Have to stop at the last page */
#endif /* ~MAC */
			return fTrue;
			}
		else
			{
/* how far are we from home point ? */
			if (dyeHome > 0 && fHome)
				dye = min(dye, dyeHome); /* if cross => go to home */
			ScrollPageDye(ww, dye);
			}
		}
	else
		{
		if ((dye = min(-dye, dypGrayOutsideSci - pwwd->yeScroll)) <= 0
				|| fSmallPage)
			{
#ifdef MAC
			vwwScroll = ww;
			ScrollSpc(spcPgvPageUp);
#else /* WIN */
			if ((ipgdNew = IpgdPrevWw(ww)) != ipgdNil)
				SetPageDisp(ww, ipgdNew, YeBottomPage(ww), fFalse, fFalse);
			else
				Beep(); /* Have to stop at the first page */
#endif /* ~MAC */
			return fTrue;
			}
		else
			{
/* how far are we from home point ? */
			if (dyeHome < 0 && fHome)
				dye = min(dye, -dyeHome); /* if cross => go to home */
			ScrollPageDye(ww, -dye);
			}
		}
	return fFalse;
}



/* S C R O L L  P A G E  D Y E  */
/* Scrolls page vertically in PageView by dye.
(dye<0 means text is moved down in the window)
*/
/* %%Function:ScrollPageDye %%Owner:bryanl */
ScrollPageDye(ww, dye)
{
	int w;

	Assert(cHpFreeze == 0);
	vfLastScroll = fTrue;
	Assert(PwwdWw(ww)->fPageView);
	if (FSmallPage(ww, &w, &w))
		{ 
		Beep(); 
		return; 
		}

	ClearInsertLine(&selCur);
	ClearInsertLine(&selDotted);

	ScrollWwDyw(ww, dye, PwwdWw(ww)->rcwDisp.ywTop);
	OffsetRect(&PwwdWw(ww)->rcePage, 0, -dye);
	/* redraw uncovered area */
	UpdateWindow(WinMac(PwwdWw(ww)->hwnd, PwwdWw(ww)->wwptr));
	SetElevWw(ww);
#ifdef MAC
	DrawControls(PwwdSetWw(ww, cptAll)->wwptr);
#endif
}



/* R E F R E S H  P A G E  - erase, trash, and redraw window */
/* (typically used after positioning to a new page) */
/* %%Function:RefreshPage %%Owner:chic %%reviewed: 7/10/89 */
RefreshPage(ww)
{
	struct WWD *pwwd;
	struct RC rcw;
	Mac( struct WDS wds; )

			Mac( SaveWds(&wds); )
			pwwd = WinMac(PwwdWw(ww), PwwdSetWw(ww, cptAll));
	rcw = pwwd->rcwDisp;
	rcw.xwLeft = 0;
	if (vssc != sscNil && ww == selDotted.ww)
		ClearInsertLine(&selDotted);
	if (ww == selCur.ww)
		ClearInsertLine(&selCur);

	/* set scrollbar values */
	SetElevWw(ww);

	/* update llc if this is wwCur */
	if (ww == wwCur)
		CacheLlc(cpNil);  /* WIN: does nothing */

	TrashWw(ww);
#ifdef MAC
	if (FRulerUp(ww))
		UpdateRuler();
#endif
	UpdateWindow(WinMac(PwwdWw(ww)->hwnd, PwwdWw(ww)->wwptr));
	Mac( RestoreWds(&wds); )
}



/* S E T  S C R O L L  H O R I Z */
/* Changes the horizontal scroll of ww by dxp */
/* %%Function:SetScrollHoriz %%Owner:bryanl */
SetScrollHoriz(ww, dxpScroll)
int ww, dxpScroll;
{
	struct WWD *pwwd = PwwdWw(ww);
	int dxp = dxpScroll;

	if (pwwd->fPageView)
		{
#ifdef MAC		
		OffsetRect(&pwwd->rcePage, -dxpScroll, 0);
#else
		OffsetRect((LPRECT) &pwwd->rcePage, -dxpScroll, 0);
#endif
		pwwd->xhScroll = pwwd->xeScroll;
		}
	else
		{
		/* Change rcwDisp depending on Scrolling */
		if (pwwd->xhScroll < 0 && pwwd->xhScroll - dxpScroll >= 0)
			{
			pwwd->rcwDisp.xwLeft -= dxwSelBarSci;
			dxp -= dxwSelBarSci;
			}
		else  if (pwwd->xhScroll >= 0 && pwwd->xhScroll - dxpScroll < 0)
			{
			pwwd->rcwDisp.xwLeft += dxwSelBarSci;
			dxp += dxwSelBarSci;
			}
#ifdef MAC		
		OffsetRect(&pwwd->rcePage, -dxp, 0);
#else
		OffsetRect((LPRECT) &pwwd->rcePage, -dxp, 0);
#endif
		pwwd->xhScroll -= dxpScroll;
		}


}


/* T H U M B  P A G E */
/* Handles vertical thumbing in PageView mode */
/* %%Function:ThumbPage %%Owner:bryanl */
ThumbPage(ww, dqThumb, dqThumbOld)
int ww, dqThumb, dqThumbOld;
{
	int dyeScrollMac, dyeScrollOld, dyeScrollNew, dyeT, fSetElev = fFalse;
	struct WWD *pwwd;
	int doc, dye, ipgd;
	CP cpNew, cpCurPage, cpNextPage, cpMac;
	extern CP CpFromIpgd();

	Assert(cHpFreeze == 0);
	if (dqThumb == dqThumbOld)
		goto LSetReturn;        /* Didn't move */

	/* And now it's time to Guess That Cp! */
	cpMac = CpMacDoc((doc = DocBaseWw(ww)));
	cpNew = CpScaleThumb(ww, dqThumb);
	if (cpNew < cp0)
		cpNew = cp0;
	if ((ipgd = IpgdNextWw(ww, fFalse)) == ipgdNil)
		cpNextPage = cpMac;
	else
		cpNextPage = CpFromIpgd(doc, ww, ipgd); /* HM */
	cpCurPage = CpFromIpgd(doc, ww, max(0, PwwdWw(ww)->ipgd)); /* HM */

	if ((cpNew > cpNextPage)
			|| cpNew < cpCurPage)
		{       /* Go to another page */
		BOOL fNormHoriz = FNormHoriz(ww);
		struct LBS lbsText, lbsFtn;

		SetPageDisp(ww,
				IpgdRepagToCpIpgd(ww, doc, cpNew, ipgdNil, &lbsText, &lbsFtn),
				YeTopPage(ww), fTrue, fNormHoriz);
		/* Clear up these structures */
		EndLayout(&lbsText, &lbsFtn);
		fSetElev = fTrue;
		}
	else
		{
		int dqCurPage, dqNextPage;
		/* We stay on the same page but scroll up or down
			This is tricky.  We assume that the cps are continuous
			over the page and interpolate to find where we
			"want" to be.  That is compared with the current
			scroll and, presto, we scroll */
		pwwd = PwwdWw(ww);
		dyeScrollMac = DyOfRc(&pwwd->rcePage) - DyOfRc(&pwwd->rcwDisp);
		dyeScrollOld = max(0, -pwwd->rcePage.yeTop);
		dqCurPage = DqRatio(cpCurPage, dqMax - dqPgvwElevSpace, cpMac);
		dqNextPage = DqRatio(cpNextPage, dqMax - dqPgvwElevSpace, cpMac);
		dyeScrollNew = NMultDiv(min(dqThumb - dqCurPage, dqNextPage - dqCurPage),
				dyeScrollMac, max(1, dqNextPage - dqCurPage));
		dyeScrollNew = max(-YeTopPage(ww), dyeScrollNew);
		if (dqThumb > dqThumbOld)
			{
			dyeT = pwwd->rcePage.yeBottom + pwwd->rcwDisp.ywTop
					- pwwd->rcwDisp.ywBottom;
			dye = min(dyeT, dyeScrollNew - dyeScrollOld);
			if (dye > 0)
				ScrollPageDye(ww, dye);
			}
		else
			{
/* move up on the page (move text down, looking at higher portions of page) */
			if (cpCurPage == cp0 && pwwd->rcePage.yeTop
					>= YeTopPage(ww))
				dye = -pwwd->rcePage.yeTop + dypGrayOutsideSci;
			else
				{
				dyeT = pwwd->ywMin - pwwd->rcePage.yeTop;
				dye = min(dyeT, dyeScrollOld - dyeScrollNew);
				}
			if (dye > 0)
				ScrollPageDye(ww, -dye);
			}
		}

LSetReturn:
	/* Set the elevator if needed */
	if (dqThumb > dqMax - dqPgvwElevSpace || fSetElev)
		{
		PwwdWw(ww)->dqElevator = dqThumb;
		SetElevWw(ww);
		}
}




/* D O C  C P R E F  F R O M  W W */
/* Find the doc and cp in the wwOther to be sync with ww wrt footnote/annotation */
/* If there is no need to sync, return docNil. */
/* return cpRef in the mother doc also - only if there is a doc to sync to */
/* %%Function:DocCpRefFromWw %%Owner:bryanl */
DocCpRefFromWw(ww, pcp, pcpMom)
int ww;
CP *pcp;
CP *pcpMom;
{
	int wwOther = WwOther(ww);
	struct WWD *pwwd = PwwdWw(ww);
	struct DOD *pdod = PdodDoc(DocBaseWw(ww));
	CP cpFirst = *pcp;
	struct PLC **hplcDn, **hplcRg;
	int iFooFirst;
	struct DRP *pdrp;
	int docSync;
	int edc;
	int wk;

	if (wwOther == wwNil)
		return docNil;

	FreezeHp();
	if ((wk = pwwd->wk) == wkFtn Win(|| wk == wkAtn))
		{
/* subdoc text to reference mapping */
/* pdod points to sub dod with the fnd. */
		edc = wk == wkFtn ? edcDrpFtn : edcDrpAtn;
		Mac(Assert(edc == edcDrpFtn));
		hplcDn = pdod->hplcfnd;
		iFooFirst = IInPlc(hplcDn, cpFirst);
		if (iFooFirst >= IMacPlc(hplcDn) - 1 ||
				iFooFirst < 0 /* bullet proof */)
			goto LRetDocNil;
		pdrp =  ((int *)PdodDoc(docSync = pdod->doc)) + edc;
		hplcRg = pdrp->hplcRef;
		*pcpMom = CpPlc(hplcRg, iFooFirst);
		}
	else  if ((wk = PwwdWw(wwOther)->wk) == wkFtn Win(|| wk == wkAtn))
		{
/* references to footnote/annotation text mapping */
/* pdod points to main dod with the frd. */
		edc = wk == wkFtn ? edcDrpFtn : edcDrpAtn;
		Mac(Assert(edc == edcDrpFtn));
		pdrp = ((int *)pdod) + edc;
		hplcDn = pdrp->hplcRef;
		docSync = pdrp->doc;
		iFooFirst = IInPlcRef(hplcDn, cpFirst);
		if (iFooFirst == IMacPlc(hplcDn) ||
				iFooFirst < 0 /* bullet proof */)
			goto LRetDocNil;
		hplcRg = PdodDoc(docSync)->hplcfnd;
		*pcpMom = CpPlc(hplcDn, iFooFirst);
		}
	else
		{
LRetDocNil:
		MeltHp();
		return docNil;
		}

	*pcp = CpPlc(hplcRg, iFooFirst);
	MeltHp();
	return docSync;
}


/* C P  S C A L E  T H U M B */
/* %%Function:CpScaleThumb %%Owner:bryanl */
CP CpScaleThumb(ww, dq)
{
	CP cpMac = CpMacDoc(DocBaseWw(ww));
	int dqScale = cpMac > 0xffff ? 128 : 1;
	int dqLim = PwwdWw(ww)->fPageView ? dqMax - dqPgvwElevSpace : 
	dqMax - 1;
	return CpMin((cpMac / dqScale
			* dq + (dqLim >> 1))  / dqLim * dqScale, cpMac - 1);
}


/* S E T  P A G E  D I S P */
/* procedure can scroll in a ww to a given ipgd, scroll within the ipgd to a
given ye (and normalize xe or leave it unchanged).
Page display is refreshed.
*/
/* %%Function:SetPageDisp %%Owner:chic %%reviewed: 7/10/89 */
SetPageDisp(ww, ipgd, ye, fSetNorm, fNorm)
int ww; 
int ipgd; 
int ye; 
BOOL fSetNorm, fNorm;
{
	struct RC *prce;
	if (!fSetNorm) fNorm = FNormHoriz(ww);

	if (ipgd != ipgdNil)
		{
		if (ipgd != IpgdCurrentWw(ww))
			IpgdPldrFromIpgd(ww, ipgd);
		else if (ye == PwwdWw(ww)->yeScroll)
			{
			/* we are in the requested page and ye position, just set
			the dirty bit to let UpdateWw later have a chance to examine
			and update if any bad cps in the DR */
			PwwdWw(ww)->fDirty = fTrue;
			return;
			}
		}

	if (PwwdWw(ww)->ipgd == ipgdNil)
		{
		/* Not enough memory to create drs */
		return;
		}

	/* scroll page rect vertically as requested. */
	if (fNorm)
		TrashRuler();

	prce = &(PwwdWw(ww))->rcePage;
	MoveRc(prce, fNorm ? XeLeftPage(ww) : prce->xeLeft, ye);
	if (fNorm)
		PwwdWw(ww)->xhScroll = PwwdWw(ww)->xeScroll;

	/* refresh display */
	RefreshPage(ww);

	if (fNorm)
		{
		PwwdSetWw(ww, cptAll);
		SyncSbHor(ww);
		}
}


#ifdef WIN

/* %%Function:MwdVertScroll %%Owner:bryanl */
void MwdVertScroll(ww, code, dqThumb)
int ww;
WORD code;
int dqThumb; /* new position */
{
	int	(*pfScroll)();
	int dypPage, dysMinScroll, dysMaxScroll;
	struct PLCEDL **hplcedl;
	CP cpMac;
	int dye;
	extern int ScrollUp(), ScrollDown();
	extern int vkcPrev;

	struct WWD *pwwd = PwwdWw(ww);

	Assert( mpwwhwwd[ww] != hNil );

	pfScroll = (code == SB_LINEDOWN) ? ScrollUp : ScrollDown;
	sbScrollLast = sbScroll;
	sbScroll = code;
	fCdlScrollSet=fFalse;

	if (sbScrollLast != sbScroll)
		{
		vcConseqScroll = 0;
		vkcPrev = kcNil;
		}

	switch (code) 
		{
	default:
		/* case SB_THUMBTRACK:*/
		return;
	case SB_LINEUP:
	case SB_LINEDOWN:
		Profile(vpfi == pfiScroll ? StartProf(30) : 0);
		if (vcConseqScroll < 0x7FFF)
			{
			vcConseqScroll++;
			}

		if (pwwd->fPageView)
			{
			/* pick random dlMax because pageview do not have that information */
			ScrollDelta(1, vcConseqScroll, 30 /* dlMax */,
					pwwd->ywMac - pwwd->ywMin,
					&dysMinScroll, &dysMaxScroll);
			dye = dysMinScroll;
			FScrollPageDyeHome(ww, code == SB_LINEUP ? -dye : dye, fTrue, 0, fFalse);
			}
		else
			{
			hplcedl = PdrGalley(pwwd)->hplcedl;
			if (hplcedl == hNil)
				break; /* can happen coming out of pageview from low in memory */

			ScrollDelta(1, vcConseqScroll, (*hplcedl)->dlMax,
					pwwd->ywMac - pwwd->ywMin,
					&dysMinScroll, &dysMaxScroll);
			(*pfScroll)( ww, dysMinScroll, dysMaxScroll);
			}
		break;
	case SB_PAGEUP:
	case SB_PAGEDOWN:
		Profile(vpfi == pfiScroll ? StartProf(30) : 0);
		if (pwwd->fPageView)
			{
			dye = DyOfRc(&pwwd->rcwDisp) - vsci.dysMinAveLine;
			FScrollPageDyeHome(ww, code == SB_PAGEUP ? -dye : dye, fTrue, 0, fFalse);
			}
		else
			{
			dypPage = pwwd->ywMac - pwwd->ywMin;
			if (dypPage > vsci.dysMacAveLine)
				dypPage -= vsci.dysMacAveLine;
			(*(code == SB_PAGEUP ? ScrollDown : ScrollUp))
					( ww, dypPage, dypPage + vsci.dysMacAveLine - vsci.dysMinAveLine );

			}
		break;
	case SB_THUMBPOSITION:
		Profile(vpfi == pfiScroll ? StartProf(30) : 0);
		if (pwwd->fPageView)
			{
			int dqThumbOld = SpsFromHwndRSB(pwwd->hwndVScroll);
			ThumbPage(ww, dqThumb, dqThumbOld);
			}
		else
			{
			ThumbToCp( ww, CpScaleThumb(ww, dqThumb), fFalse, fFalse, 0 );
			}
		break;
		}

	UpdateWindowWw( ww );
	Profile(vpfi == pfiScroll ? StopProf() : 0);
}



/* %%Function:MwdHorzScroll %%Owner:bryanl */
void MwdHorzScroll(ww, code, dqThumb)
int ww;
WORD code;
int dqThumb; /*New position */
{
/* the mod is used so that small changes in rcwDisp caused by the scroll
itself do not change the amount of scroll, but larger changes in window
size do. Otherwise Left scroll would not be the inverse of Right scroll.
Note: dxpMinScrollVsci must be divisible by 16! (to avoid truncation by
scroll) */
	struct WWD *pwwd = PwwdWw(ww);
	int dxpScroll = DxOfRc(&pwwd->rcwDisp) /
	(dxpMinScrollSci / 2) * (dxpMinScrollSci / 2);
#ifdef CHIC
	int dxpWwScroll = NMultDiv(DxOfRc(&pwwd->rcwDisp), 2, 3);
	int dxpScroll = vsci.dxpMinScroll;
#endif
	BOOL fLeft = fFalse;
	int xpThumb;

	Assert( mpwwhwwd[ww] != hNil );
	switch (code) 
		{
	default:
		/* case SB_THUMBTRACK: */
		return;
	case SB_LINEUP:
		SetOurKeyState();
		dxpScroll = vsci.dxpMinScroll;
		break;
	case SB_LINEDOWN:
		fLeft = fTrue;
		dxpScroll = vsci.dxpMinScroll;
		break;
	case SB_PAGEUP:
		break;
	case SB_PAGEDOWN:
		fLeft = fTrue;
		break;
	case SB_THUMBPOSITION:
		if (pwwd->fPageView)
			{
			xpThumb = NMultDiv(dqThumb,
					max(XeLeftPage(ww), 
					DxOfRc(&pwwd->rcePage) - DxOfRc(&pwwd->rcwDisp)
					+ dxpGrayOutsideSci)
					+ dxpGrayOutsideSci,
					vsci.xpRightMax)
					- dxpGrayOutsideSci;
			}
		else
			{
			xpThumb = NMultDiv(dqThumb,
					vsci.xpRightMax - DxOfRc(&pwwd->rcwDisp),
					vsci.xpRightMax);
			}
		dxpScroll = xpThumb + pwwd->xhScroll;

		if (dxpScroll > 0)
			fLeft = fTrue;
		else
			dxpScroll = -dxpScroll;
		break;
		}

	if (dxpScroll == 0)
		{
		Beep();
		return;
		}

	if (fLeft)
		ScrollLeft(ww, dxpScroll);
	else
		{
/* if not pageview, at or left of home pos, no dl's hanging to the left,
and no shift key depressed => not allowed. */
		if (!pwwd->fPageView && DxpScrollHorizWw(ww) >= 0 &&
				!FAnyDlNotInXw(ww, fTrue) && 
				!(vgrpfKeyBoardState & wKbsShiftMask))
			Beep();
		else
			ScrollRight(ww, dxpScroll);
		}


/* Only force update if we have at least 1/2 inch wide invalid rect */
/* or if we have scrolled to dxpScroll == 0; prevents weird failure
	to update until user lets up on the mouse */

	pwwd = PwwdWw(ww);
	UpdateWindowWw(ww);

#ifdef CHIC
	UpdateWw( ww, (DxpScrollHorizWw(ww) != 0) &&
			(pwwd->rcwInval.xpRight - pwwd->rcwInval.xpLeft
			< (vfli.dxsInch >> 1)) );
#endif
}



/* S C R O L L  W W  D Y W
*  Scrolls window vertically below ywTop by dyw pixels.
*  Invalidates uncovered area.  Window is NOT updated here.
*/
/* %%Function:ScrollWwDyw %%Owner:bryanl */
ScrollWwDyw(ww, dyw, ywTop)
{
	struct RC rcw, rcwUpdate;
	struct WWD *pwwd = PwwdWw(ww);

	GetClientRect(pwwd->hwnd, (LPRECT)&rcw);
	rcw.ywTop = ywTop;
	rcw.ywBottom = pwwd->ywMac;

	ScrollDC( pwwd->hdc, 0, -dyw, (LPSTR) &rcw,(LPRECT)&rcw,
			(HRGN) NULL, (LPRECT)&rcwUpdate );

/* InvalWwRc will not work because rcePage has not been offset yet, 
it is done in caller */
	InvalidateRect(PwwdWw(ww)->hwnd, (LPRECT)&rcwUpdate, fTrue);
}


#endif /* WIN */
