#ifdef MAC	 
#define WINDOWS /* SelectWindow */
#include "toolbox.h"
#endif

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "doc.h"
#include "sel.h"
#include "inter.h"
#include "ch.h"
#include "disp.h"
#include "format.h"
#include "border.h"
#ifdef MAC
#include "mac.h"
#endif
#define USEBCM
#include "cmd.h"
#include "debug.h"
#include "screen.h"
#ifdef WIN
#include "help.h"
#endif

extern int              vfSeeSel;
extern int              vfSelAtPara;
extern struct CHP       vchpFetch;
extern CP               vcpFetch;
extern uns              vccpFetch;
extern char             *vpchFetch;
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern BOOL             vfEndFetch;
extern struct CA        caPara;
extern struct CA        caSect;

extern struct FLI       vfli;
extern struct SEL       selCur;
extern struct SEL       selDotted;

extern int              wwCur;
extern struct WWD       **hwwdCur;
extern int		vwwScroll;
extern int              vfLastCursor;
extern int              vxwCursor;
extern BOOL		vfEndCursor;
extern int 		vfLastDrCursor;
extern int		vylDrCursor;
extern int              vgrpfKeyPrefix;
extern BOOL             vfNumLock;
extern struct PLDR	**vhpldrRetSel;
extern int		vidrRetSel;
extern struct CA			caTap;
extern struct TCC			vtcc;
extern struct TAP			vtapFetch;
extern CP					vmpitccp[];
extern struct UAB       vuab;
extern struct PREF      vpref;

extern struct SELS      rgselsInsert[];

extern int              wwMac;
extern int              wwCur;
extern struct WWD       **mpwwhwwd[];
extern int              **vhfParaScanLooks;
extern struct ITR       vitr;
#ifdef WIN
extern BOOL             vfExtendSel;
extern HWND             vhwndCBT;
extern BOOL             vfRecording;
extern int		vcConseqScroll;
extern int		dypCS;
extern struct FTI       vfti;
#endif
extern struct SCI       vsci;
extern CHAR		szEmpty[];
extern BOOL		fElActive;

#ifdef BRYANL
/* #define DCURS */
#endif

#ifdef SUBDRAW
extern int	fSubdraw;
#endif

#ifdef MAC
/* L E F T  R I G H T  S E L */
/* business end of KeyCmdLeftRight, except for fRound: if true, do not
extend selection if already at sty boundary.
*/
/* %%Function:LeftRightSel %%Owner:NOTUSED */
LeftRightSel(psel, sty, fRound, fExtend, fFwdKey)
struct SEL *psel;
int sty, fRound, fExtend, fFwdKey;
{
	struct PLCEDL **hplcedl;
	int idr;
	int dl;
	BOOL fInsEnd = fFalse;
	struct PLDR **hpldr;
	int doc = psel->doc, docT;
	int ww = psel->ww;
	CP cp, cpT;
	CP cpLim = CpMacDocEdit(doc);
	struct CA caInTable;
	struct DRF drfFetch;
	struct EDL edl;
	struct WWD *pwwd;

	if (PdodDoc(doc)->fFtn)
		cpLim -= ccpEop;

/* Find cp to start extension from */
	if (fExtend && psel->fTable && (psel->sty == styRow || psel->sty == styWholeTable))
		return;

/* this is for the case when extended selection is collapsed and one half
of the fRound case. Max is to protect from underflow with block selections. */
	cp = fFwdKey ? CpMax(cp0, psel->cpLim - 1) : psel->cpFirst + 1;
	if (fRound)
		{
		if (fFwdKey)
			{
			if (--cp < 0)
				return;
			}
		}
	else  if (psel->fIns || (fExtend && !psel->fTable))
		cp = psel->fForward ? psel->cpLim : psel->cpFirst;

	if (fFwdKey)
		{
		if (cp > cpLim)
			goto LLim;
		if (sty == styLine && !fExtend)
	/* Code backs off insertion point from the end of a line, unless the line is full.
*/
			{
			CP cpT;
			struct DR *pdr;
			struct DRF drf;
			if (cp >= cpLim)
				goto LLim; /* already just before end */
			cpT = CpFirstSty(ww, doc, cp, sty, fFalse);
/* also returns hpldr, idr */
			FormatLineDr(ww, cpT, 
					PdrFetchAndFree(vhpldrRetSel, vidrRetSel, &drf));
			cpT = vfli.chBreak != chNil ? vfli.cpMac - ccpEop :
					vfli.cpMac;
			if (cp == cpT && vfli.cpMac < CpMacDoc(doc))
				{
				cpT = CpFirstSty(ww, doc, vfli.cpMac, sty, fFalse);
				FormatLineDr(ww, cpT, 
						PdrFetchAndFree(vhpldrRetSel, vidrRetSel, &drf));
				cpT = vfli.chBreak != chNil ? vfli.cpMac - ccpEop :
						vfli.cpMac;
				}
			cp = cpT;
			if (cp == vfli.cpMac)
				fInsEnd = fTrue;
			}
		else
			{
			if (!psel->fTable)
				{
				cp =  CpLimSty(ww, doc, cp, sty, fFalse);
				if (FInTableDocCp(doc, cp))
					{
LCacheTc:
					CacheTc(wwNil, doc, cp, fFalse, fFalse);
					CpFirstTap(doc, cp);
					if (vtapFetch.rgtc[vtcc.itc].fMerged)
						{
						cp = vmpitccp[vtcc.itc + 1];
						goto LCacheTc;
						}
					}
				}
			else
				{
				CpFirstTap(doc, (psel->sty != styWholeTable) ? ((psel->fTableAnchor) ? psel->cpAnchor : psel->cpAnchor - 1) : 
				cp);
				cp = vmpitccp[min(psel->itcLim, vtapFetch.itcMac + 1)];
				}
			}
		if (cp > cpLim)
			{
			if (!fExtend)
				goto LLim;
			goto LHaveCp;
			}
#ifdef MAC
/* make sure we are not in the middle of hidden text */
		if (!vpref.fSeeHidden)
			{
			docT = doc;
			for (;;)
				{
				if (cp > cpLim)
					goto LLim;
				CachePara(doc, cp);
				FetchCp(docT, cp, fcmProps);
				if (!vchpFetch.fVanish)
					break;
				cp = vcpFetch + vccpFetch;
				docT = docNil;
				}
			}
#else /* WIN */
/* make sure we are in front of any hidden text - take care of fields too */
		cp = CpFirstSty(ww, doc, cp, styChar, fTrue);
#endif

/* check for: cp running into no-show outline */
#ifndef JR
		if ((*hwwdCur)->fOutline)
			{
			Assert(!(*hwwdCur)->fPageView);
			for (;;)
				{
				UpdateWw(wwCur, fFalse);
				if (!FCpVisible(wwCur, doc, cp, psel->fIns & psel->fInsEnd, fFalse, fFalse))
					{
					NormCp(wwCur, doc, cp, 2, 0, fFalse);
					continue;
					}
				hplcedl = PdrGalley(*hwwdCur)->hplcedl;
				cp = CpMax(CpPlc(hplcedl, 0), cp);
				dl = DlWhereDocCp(wwCur, doc, cp, fFalse, &hpldr, &idr,
						NULL, NULL, fTrue);
				if (dl == dlNil)
					return;
				GetPlc(hplcedl = (PdrFetch(hpldr, idr, &drfFetch))->hplcedl, dl, &edl);
				if (cp >= CpPlc(hplcedl, dl) + edl.dcp)
/* cp is in the invisible gap after the line dl. */
					{
					cp = CpPlc(hplcedl, dl + 1);
					FreePdrf(&drfFetch);
/* check going beyond the last line */
					if (!FCpVisible(wwCur, doc, cp, fFalse, fFalse, fFalse))
						continue;
					}
				else
					FreePdrf(&drfFetch);
				break;
				}
			}
#endif /* JR */
		if (cp > cpLim)
			goto LLim;
		}
	else
		{
		if (cp == cp0 && (!fExtend || !psel->fWithinCell))
			{
LLim:                   
			Beep();
			return;
			}
		cp = (!psel->fTable) ? CpFirstSty(ww, doc, CpMax(cp - 1, cp0), sty, fFalse) :
				CpFirstForItc(doc, (psel->sty != styWholeTable) ? psel->cpAnchor :
				CpMax(cp - 1, cp0), psel->itcFirst);
		if ((cp = CpBackOverInvisible(ww, doc, cp)) == cpNil)
			goto LLim;
		}
LHaveCp:
	if (!(fExtend && psel->fTable) && cp == (psel->fForward ? psel->cpFirst : psel->cpLim) &&
			!(cp == cp0 && psel->fWithinCell && psel->fIns && fExtend))
		{
/* new selection reduced to an insertion point */
		if (cp > cpLim)
			goto LLim;
		if (!fRound)
			Select(psel, cp, cp);
		}
	else  if (fExtend)
		{
		if (psel->fWithinCell)
			{
			struct CA caTapAnchor;
			CacheTc(wwNil, doc, psel->cpAnchor, fFalse, fFalse);
			if (!FInCa(doc, cp, &vtcc.ca) ||
					(!fFwdKey && 
					((psel->fForward && !psel->fIns) ? psel->cpLim : psel->cpFirst) == cp0))
				{
				CpFirstTap(doc, psel->cpAnchor);
				caTapAnchor = caTap;
				ColumnSelBegin(psel, vtcc.cpFirst, vtcc.itc, &caTapAnchor,
						vtcc.itc, vtcc.cpFirst);
				}
			}
		if (psel->fTable)
			{
			struct CA caTapAnchor;
			int itcAnchor;
			CP cpExtend;
			int itcExtend;

			CacheTc(wwNil, doc, psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1, fFalse, fFalse);
			CpFirstTap(doc, psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1);
			caTapAnchor = caTap;
			itcAnchor = vtcc.itc;
			CpFirstTap(doc, cpExtend = psel->fForward ? psel->cpLim - 1 :
					psel->cpFirst);
			itcExtend = psel->fRightward ? min(psel->itcLim, vtapFetch.itcMac + 1) :
					psel->itcFirst + 1;
			if (!fFwdKey)
				itcExtend = max(0, itcExtend - 2);
			cpExtend = CpFirstForItc(doc, cpExtend, itcExtend);
			SelectColumnTo(psel, cpExtend, itcExtend, &caTapAnchor, itcAnchor);
			return;
			}
/* drag selection edge to new bound */
/* this "if" prevents the collapsing of the forward rounding of word backup
as over a sequence of EOP's for example. */
		else  if (!(fFwdKey && cp < psel->cpFirst))
			{
			int fTableBefore = psel->fTable;
			ChangeSel(psel, cp, styNil, fFalse, fFalse);
			if (fTableBefore && !psel->fTable && psel->fTableAnchor && !fFwdKey && psel->fForward &&
					(CachePara(psel->doc, cpT = CpMax(cp0, cp - 1)),FInTableVPapFetch(psel->doc, cpT)) &&
					FParasUpToCpInTable(PcaPoint(&caInTable, psel->doc, psel->cpAnchor), cpT))
				{
				int itcAnchor, itc;
				struct CA caTapAnchor;
				CpFirstTap(psel->doc, cpT);
				itc = vtapFetch.itcMac + 1;
				CacheTc(wwNil, psel->doc, psel->cpAnchor, fFalse, fFalse);
				CpFirstTap(psel->doc, psel->cpAnchor);
				psel->cpAnchor = vtcc.cpFirst;
				itcAnchor = vtcc.itc;
				caTapAnchor = caTap;
				ShrinkSelToTableSty(psel, itcAnchor, itc, &caTapAnchor);
				}
			}
		}
	else
		Select1(psel, cp, cp, (fInsEnd ? maskInsEnd : 0) | maskSelChanged);
	if (psel->fIns)
		vxwCursor = psel->xw;
	if (fRound)
		return;
/* make sure active end is visible vertically/forward. Backward or
horizontal visibility will be ensured in Idle. */
	if (fFwdKey && sty != styPara)
		{
		if (psel->fForward)
			{
			cp = psel->cpLim;
			if (!psel->fIns || psel->fInsEnd)
				cp--;
			}
		else
			cp = psel->cpFirst;

/* char cp must be completely visible. if in page view NormCp call in
	Idle will handle this case. */
		if (!(*hwwdCur)->fPageView)
			{
			hplcedl = PdrGalley(*hwwdCur)->hplcedl;
			GetPlc(hplcedl, dl = IMacPlc(hplcedl) - 1, &edl);
			if (edl.ypTop + edl.dyp > (*hwwdCur)->ywMac && dl > 0)
				--dl;
/* dl is now last full line */
			if (cp >= CpPlc(hplcedl, dl + 1))
				ScrollUp(wwCur, 10, 30);
			}
		}
	if (!FMsgPresent(0))
		SeeSel1(fTrue);

LRet:
	return;
}
#endif /* MAC */



/* C U R S  U P  D O W N */
/*
move cursor up or down by line or page.
Plan:
0. choose an "opposite" cp for extension or initial xp.
1. initially, collapse sel if necessary, remember xw, yw in vptCursor;
2. calculate new yw.
3. make sure desired point is on the screen.
3. select at desired point.
*/
/* %%Function:CursUpDown %%Owner:davidlu */
CursUpDown(sty, dSty, fDrag)
int sty, dSty;
BOOL fDrag;
{
	int idr = 0;
	int dl;
	int flt;
	int xw, yw = -1;
	BOOL fChangedPage;
	struct PLCEDL **hplcedl;
	struct PLDR **hpldr = hwwdCur;
	struct SEL *psel = PselActive();
	BOOL fAlreadyChanged = fTrue;
	BOOL fEnd;
	BOOL fUp = dSty < 0;
	BOOL fCaChanged;
	BOOL fInsEndSave;
	int ywBottomNew;
	int fScrolled;
	int ywMacDr;
	int xp, xpDummy, ichDummy;
	int fHandled;
	int itcAnchor;
	int fTableSave;
	int itcFirstSave;
	int itcLimSave;
	CP cpVisi; 
	struct CA caCell, caTapAnchor, ca;
	struct WWD *pwwd = *hwwdCur;
	int ywMin = pwwd->ywMin;
	int ywMac = pwwd->ywMac;
	int cSty = (fUp) ? -dSty : dSty;
	int cTableNorm = 0; 
	BOOL cTwice = 4;
	BOOL fPageView = pwwd->fPageView;
	int ywBottom, ywTop;
	int dlMac, dlFirst, dlLim;
	CP cp, cpFirst, cpLim, cpTo;
	CP cpFirstCell;
	int itcFirst, itcLim;
	struct RC rcwDr; 
	struct CA caTable;
	struct PT pt, ptT;
	struct EDL edl, edlNext, edlPrev;
	struct CA caSave;
	struct CA caInTable;
	struct SPT spt;
	struct DRF drfFetch;

	if ((*hwwdCur)->fOutline && FOutlineEmpty(wwCur, fTrue))
		return;
	if ((*hwwdCur)->fDirty)
		UpdateWw(wwCur, fFalse);
#ifdef DCURS
		{
		int rgnum [3];

		rgnum [0] = sty;
		rgnum [1] = dSty;
		rgnum [2] = fDrag;
		CommSzRgNum( SzShared( "CmdUpDown( sty, dSty, fDrag ): " ), rgnum, 3 );
		}
#endif
/* prepare for checking whether selection changed */
	caSave = psel->ca;
	fInsEndSave = psel->fInsEnd;
	fTableSave = psel->fTable;
	itcFirstSave = psel->itcFirst;
	itcLimSave = psel->itcLim;
	fAlreadyChanged = fTrue;
#ifdef WIN
	if (vfRecording && !fElActive)
		RecordCursor(sty, dSty, fDrag);
#endif

	if (sty == styDoc)
		{
		PushInsSelCur();
		if ((*hwwdCur)->fPageView && PdodDoc(psel->doc)->dk == dkDispHdr && !fDrag)
			{
			TurnOffSel(psel);
			psel->doc = PmwdWw(wwCur)->doc;
			psel->sk = skNil;
			}
		cpTo = fUp ? cp0 : CpMacDocEdit(psel->doc);

		if (fDrag)
			{
			if (!fUp)
				{
				if (PdodDoc(psel->doc)->fFtn)
					cpTo -= ccpEop;
				else  if (psel->cpAnchor < CpMacDocEdit(psel->doc))
					cpTo = CpMacDoc(psel->doc);
				}
			if (FInTableDocCp(psel->doc, cpTo) &&
					FInTableDocCp(psel->doc, psel->cpFirst) &&
					(caTable = caPara, FParasUpToCpInTable(&caTable, cpTo)))
				{
				Assert(fUp);
				if (fUp)
					{
					cpFirst = cpTo;
					itcFirst = 0;
					CacheTc(wwNil, psel->doc, psel->cpFirst, fFalse, fFalse);
					CpFirstTap(psel->doc, psel->cpFirst);
					cpLim = caTap.cpLim;
					itcLim = vtcc.itc + 1;
					SelectColumn(psel, cpFirst, cpLim, itcFirst, itcLim);
					}
				}
			else
				ChangeSel(psel, cpTo, styNil, fTrue /* fVisibleOnly */, fFalse);
			}
		else
			{
			if (!fUp && PdodDoc(psel->doc)->fFtn)
				cpTo -= ccpEop;	/* within last ftn */
			SelectIns(psel, cpTo);
			}
		if (!FNeRgw(&psel->ca, &caSave, cwCA))
			Beep();
		NormCp(wwCur, psel->doc, CpMin(cpTo, CpMacDocEdit(psel->doc)), ncpHoriz,
				DyOfRc(&(*hwwdCur)->rcwDisp) >> 2, fFalse);
		/* make sure no one changed it out from under us */
		Assert (psel == PselActive());
		return;
		}
/* in pageview, home will scroll the page and select the first cp in dr, or the
fisrtt cp in the last line of the last dr. */
	if (fPageView)
		{
		if (sty == styHome)
			{
			CmdDrCurs1(fUp ? ucmDrFirst : ucmDrLast, fDrag, !fUp);
			return;
			}
#ifdef BOGUS
		else  if (sty == styScreen)
			{
			ScrollSpc(fUp ? spcChunkUp : spcChunkDown);
			return;
			}
#endif
		}
	if (sty == styHome)
		{
		fAlreadyChanged = fFalse; /* should the be initied to false? CS */
		if (fUp)
			{
			yw = ywMin;
			xw = (*hwwdCur)->xwMin;
			}
		else
			{
			yw = ywMac - 1;
			xw = (*hwwdCur)->xwMac - 1;
			}
LHomeOnXwYw:	/* restrict {xw,yw} to vertically visible dl's, then click */

		FullDlBoundsDr(wwCur, hpldr, idr, &dlFirst, &cpFirst, 
				&dlLim, &cpFirst);
		if (yw >= ywMac - 1 && dlLim > 0)
			{
			YwTopForDl(hpldr, idr, dlLim - 1, &yw);
			yw--;
			}
		else  if (dlFirst < dlLim)
			{
			ywTop = YwTopForDl(hpldr, idr, dlFirst, &ywBottom);
			if (yw < ywTop)
				yw = ywTop;
			}
		vxwCursor = xw;
		goto LClick;
		}
#ifdef WIN
	if (sty == styScreenEnd)
		{
		xw = fUp ? (*hwwdCur)->xwMin : (*hwwdCur)->xwMac - 1;
		yw = fUp ? ywMin : ywMac - 1;
		goto LClick;
		}
#endif
LAgain:   /* for dSty > 1 (from macro language) - should not affect Mac */

	/* make sure no one changed it out from under us */
	Assert (psel == PselActive());

/* this is the base cp from where we are going to move the cursor */
	fEnd = fFalse;
	cp = (fDrag  && !(*hwwdCur)->fOutline ? psel->fForward : !fUp) ?
			(fEnd = vfLastCursor ? vfEndCursor : fTrue,
			psel->cpLim) : psel->cpFirst;
	if (!fUp)
		{
		GetCaCharBlock(wwCur, psel->doc, cp, &ca, &cpVisi); 
		if (cp != cpVisi)
			cp = (cpVisi != cpNil) ? cpVisi : ca.cpLim; 
		}
			
	if (psel->fIns)
		fEnd = psel->fInsEnd;
	else
		{
		if (psel->fNil)
			/* last visible selection was really selCur, not selDotted */
			{
			Assert (psel == &selDotted);
			fEnd = selCur.fInsEnd;
			}
		}
	if ((*hwwdCur)->fDirty)
		UpdateWw(wwCur, fFalse);
	
	dl = DlWhereDocCp(wwCur, psel->doc, cp, fEnd, &hpldr, &idr,
			&cpFirst, NULL, fTrue);
#ifdef DCURS
		{
		int rgw [3];

		rgw [0] = (int)cp;
		rgw [1] = fEnd;
		rgw [2] = dl;
		CommSzRgNum( SzShared("  cpFrom, fEndFrom, dlFrom: "), rgw, 3 );
		}
#endif

/*
up/down:
	if sel is visible,
		if top/bottom of screen, scroll in docu up/down, click there
		else click above/below selection.
		if at the top/bottom of page, go to prev/next page, bottom/top.
		click there.
	else (sel is not visible)
		click at bottom/top of screen (left if xw not available)
	if selection did not take and not already scrolled or paged
		scroll up/down one line, page if necessary. Beep if at start/end
	the point is that each command is guaranteed to change the state.
*/
	if (sty == styScreen)
		{
		if (!fPageView)
			{
			hpldr = hwwdCur;
			idr = 0; /* table DR might be gone */
			}
		YwScrollForCursor(sty, fUp, 1, &fScrolled,yw);
		/* ywscroll could have conceivably killed dyadic move
		and thrown out selDotted, which may be psel.
		so rebuild it. danger danger bz
		*/
		psel = PselActive();
		UpdateWw(wwCur, fFalse);
		fAlreadyChanged = fTrue;
		yw = ywMin;
		xw = vxwCursor;
		if (!fScrolled)
			{
			if (cp == (fUp ? cp0 : 
					(fDrag ? CpMacDoc(psel->doc) : 
					(psel->fIns ? CpMacDocEdit(psel->doc) : cp+1))))
				{
				Beep();
				goto LEnd;
				}
			if (fUp)
				xw = 0;
			else
				{
				yw = ywMac - 1;
				xw = (*hwwdCur)->xwMac-1;
				}
			}
		else  if (fDrag)
			{
			if (!fPageView)
				{
/* If dragging, show 1 hilited dl */
				if (dl == dlNil)
					goto LBeep;
				ywTop = YwTopForDl(hpldr, idr, 0, &ywBottom);
				yw += ywBottom - ywTop;
				}
			}

		if (fPageView)
			{
			yw += dypMinWwInitSci;
			pt.yw = yw;
			pt.xw = xw;
			if (DlWherePt(wwCur, &pt, &hpldr, &idr,
					&spt, fTrue, fFalse) == dlNil)
				{
				return;
				}
			pt.xw = xw;
			}
		goto LHomeOnXwYw;
		}

	if (dl != dlNil)
		{
		if ((psel->fTable || (fDrag && !fUp && !psel->fWithinCell)) && FInTableDocCp(psel->doc, cpFirst))
			{
			int itc;
			itc = (psel->fTable) ? ((!fDrag ? psel->fRightward : !psel->fRightward) ?
					psel->itcFirst : psel->itcLim - 1) :
			(fUp ? 0 : itcMax);
			cpFirst = (fDrag || (psel->fForward ? !fUp : fUp)) ?
					CpFirstForItc(psel->doc, cpFirst, itc) :
					(psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1);
			dl = DlWhereDocCp(wwCur, psel->doc, cpFirst, fFalse, &hpldr, &idr,
					&cpFirst, NULL, fTrue);
			if (dl == dlNil)
				goto LBeep;
			fEnd = fFalse;
			if (fUp)
				dl = 0;
			else
				{
				dl = max(0, IMacPlc(hplcedl = (PdrFetch(hpldr, idr, &drfFetch))->hplcedl) - 1);
				cpFirst = CpPlc(hplcedl, dl);
				FreePdrf(&drfFetch);
				}
			cp = cpFirst;
			vfLastCursor = fFalse;
			}
		if (!fPageView &&
				((fUp && cpFirst == cp0 && PdrGalley(*hwwdCur)->dypAbove == 0)
				|| 
				(!fUp && ((fDrag && cp >= CpMacDoc(psel->doc)) ||
				(!fDrag && cp >= CpMacDocEdit(psel->doc) && psel->fIns)))))
			{
			goto LBeep;
			}

		{ /* block for usage of pdr */
		struct DR *pdr = PdrFetch(hpldr, idr, &drfFetch);

		hplcedl = pdr->hplcedl;
		GetPlc(hplcedl, dl, &edl);
		yw = YwFromYp(hpldr, idr, edl.ypTop);
		ywMacDr = YwFromYp(hpldr, idr, pdr->dyl);
		if (!vfLastCursor)
			{
			FormatLineDr(wwCur, cpFirst, pdr);
			vxwCursor = XwFromXp(hpldr, idr,
 					XpFromDcp(cp0, cp - vfli.cpMin,
					&xpDummy, &ichDummy));
			
#ifdef BOGUS
			XpFromDcp(cp0, cp - vfli.cpMin, &xp, &ichDummy);
			vxwCursor = XwFromXp(hpldr, idr, xp); 
#endif					
			vfEndCursor = fTrue;
			}
		dlMac = IMacPlc(hplcedl);
#ifdef DCURS
		CommSzNum( SzShared( "  dlMac = "), dlMac );
#endif	
		if (!fPageView && (*hpldr)->hpldrBack != hNil &&
			dl == dlMac - 1)
			{
			DrclToRcw(hpldr, &pdr->drcl, &rcwDr);
			ywBottomNew = rcwDr.ywTop + DylMacForTableDr(pdr->cpFirst, hpldr); 
			edl.dyp = ywBottomNew - yw;
			}

		FreePdrf(&drfFetch);
		} /* block for usage of pdr */

		xw = vxwCursor;
		if (yw + edl.dyp >= ywMin && yw < ywMac)
			{
/* cursor is visible before the move */
			if (sty == styLine)
				{
				CP cpLimLine;
				BOOL fVisiAfter = fTrue;
				if (fDrag && psel->sty == styLine)
					{
					int fUseMac = psel->fForward;
					hplcedl = (PdrFetch(hpldr, idr, &drfFetch))->hplcedl;
					cpLimLine = CpPlc(hplcedl, dl + 1);
					FreePdrf(&drfFetch);
					if ((psel->fForward) ? fUp : !fUp)
						{
						if (psel->cpLim <= cpLimLine)
							fUseMac = !psel->fForward;
						}
					xw = fUseMac ? ((*hwwdCur)->xwMac - 1) : (*hwwdCur)->xwMin;
					}

/* 4's ensure jumping over borders in tables */
				if (fUp)
					{
					if (dl > 0)
						{
						hplcedl = (PdrFetch(hpldr, idr, &drfFetch))->hplcedl;
						GetPlc(hplcedl, dl - 1, &edlPrev);
						FreePdrf(&drfFetch);
						if (edlPrev.hpldr != hNil)
							{
							hpldr = edlPrev.hpldr; 
							idr = 0; 
							hplcedl = (PdrFetch(hpldr, idr, &drfFetch))->hplcedl;
							dl = IMacPlc(hplcedl);
							FreePdrf(&drfFetch);
							}
						if (dl > 0)
							YwTopForDl(hpldr, idr, dl - 1, &yw);
						}
					yw -= dyTableBorderMax;
					if (yw < ywMin)
						fVisiAfter = fFalse;
					}
				else
					{
					if (dl < dlMac - 1)
						{
						int dysT;

						hplcedl = (PdrFetch(hpldr, idr, &drfFetch))->hplcedl;
						GetPlc(hplcedl, dl + 1, &edlNext);
						FreePdrf(&drfFetch);
						if (edlNext.hpldr != hNil)
							{
							hpldr = edlNext.hpldr; 
							idr = 0; 
							dl = -1; 
							}

						yw = YwTopForDl(hpldr, idr, dl + 1, &ywBottom);
						if (!edlNext.fEnd && ywBottom > ywMac &&
								!(fDrag && xw <= 
								XwFromXp(hpldr, idr, edlNext.xpLeft)
								&& dl+1 != DlPartial( hpldr, idr, &dysT)))
							fVisiAfter = fFalse;
						}
					else
						{
						if ((*hpldr)->hpldrBack != NULL)
							{
							dl = DlWhereDocCp(wwCur, psel->doc, cp, fEnd, &hpldr, &idr,
									&cpFirst, NULL, fFalse);
							if (dl == dlNil)
								goto LBeep;
							}
						else  if (fPageView)
							{
							yw = ywMacDr;
							fAlreadyChanged = fFalse;
							goto LClick;
							}
						YwTopForDl(hpldr, idr, dl, &yw);
						}
					yw += dyTableBorderMax;
					if (yw >= ywMac)
						fVisiAfter = fFalse;
					}
				if (!fVisiAfter)
/* cursor would not be visible after the move */
LScrollAgain:
					{
					int ipgd = (*hwwdCur)->ipgd;
#ifdef DCURS
					CommSzSz( SzShared( "  cursor would not be visible after move, scrolling "),szEmpty);
#endif
					if (--cTwice <= 0)
								return; 
					if (!fDrag)
						TurnOffSel(psel);
					yw = YwScrollForCursor(sty, fUp, 1,&fScrolled,
													yw-dyTableBorderMax);
					/* ywscroll could have conceivably killed dyadic move
					and thrown out selDotted, which may be psel.
					so rebuild it. danger danger bz
					*/
					psel = PselActive();
					if (!fScrolled)
							Beep();
					fAlreadyChanged = fTrue;
					if ((*hwwdCur)->fPageView && (*hwwdCur)->ipgd != ipgd)
								{
								hpldr = hNil;
								goto LClick;
								}

					goto LAgain;
					}
				else
					fAlreadyChanged = fFalse;
				}
			}
		else
			goto LNotVisible;
		}
	else
		{
LNotVisible:
/* cursor is not visible before the move */
#ifdef DCURS
		CommSzSz( SzShared( "  Cursor NOT visible before move "),szEmpty);
#endif
		fAlreadyChanged = fFalse;
		
		if (psel->fTable)
			{
			NormCp(wwCur, psel->doc, cp,
					ncpVisifyPartialDl + ncpHoriz,
					(DyOfRc(&(*hwwdCur)->rcwDisp) * 3) / 4,
					psel->fInsEnd);
			if (++cTableNorm > 4)
				{
				Beep(); 
				return; 
				}

			goto LAgain; 
			}
		if (dl != dlNil && fPageView)
			goto LScrollAgain;
		if (dl == dlNil && hpldr == hNil)
			goto LEnd;
		xw = vxwCursor;
		yw = (dl == dlNil && 
				cp > PdrFetchAndFree(hpldr, idr, &drfFetch)->cpFirst) ||
				( dl != dlNil && 
				YwTopForDl(hpldr, idr, dl, &ywBottom) > ywMin) ?
				ywMac - 1 : ywMin;
		goto LHomeOnXwYw;
		}
LClick:
	ptT.yw = yw; 
	ptT.xw = xw;
	if (fPageView && hpldr != hNil)
		{
		DrclToRcw(hpldr, &(PdrFetchAndFree(hpldr, idr, &drfFetch)->drcl), &rcwDr);
		if (xw > rcwDr.xwRight)
			ptT.xw = rcwDr.xwRight - 1; 	 
		}

#ifdef DCURS
	CommSzRgNum( SzShared("  CLICK (xw,yw): "),&pt, 2 );
#endif
	while (((dl = DlWherePt(wwCur, &ptT, &hpldr, &idr, &spt, fTrue, fFalse)) == dlNil || !spt.fInDr) &&
			fPageView && (fUp ? (ptT.yw > (*hwwdCur)->ywMin) :
			(ptT.yw < (*hwwdCur)->ywMac)))
		{
		ptT.xw = xw;
		ptT.yw += (fUp ? -4 : 4);
		}
#ifdef DCURS
	CommSzNum( SzShared("  Click dl = "),dl );
#endif
	pt.yw = ptT.yw; 
	pt.xw = (sty != styLine || !fDrag) ? xw : ptT.xw; 
		
/* click at dl, xw. We want something to change after every call of CursUpDown
if fAlreadyChanged is set, something already has, otherwise if sel did not change,
cause a scroll.
*/
	if (dl != dlNil)
		{
		struct DR *pdr = PdrFetch(hpldr, idr, &drfFetch);
		int docDr = pdr->doc;
		struct PLCEDl **hplcedl = pdr->hplcedl;
		CP cpFirst = CpPlc(hplcedl, dl);
		int ywLine;


/* the following if(){} is probably bogus given that pt.xw is re-assigned above. */
		if (IMacPlc(hplcedl) > dl + 1)
			{
			GetPlc(hplcedl, dl + 1, &edl);
			if (edl.fEnd && YwFromYp(hpldr, idr, edl.ypTop) <= yw)
				{
/* next line is endmark. xw is not modified by DlWhere if xw was in sel bar. */
				pt.xw = xwMaxSci;
				cpFirst = CpPlc(pdr->hplcedl, dl + 1);
				fAlreadyChanged = fTrue; /* prevent scrolling */
				}
			}
#ifdef WIN
		if (sty == styScreenEnd && dSty > 0)
			{	/* back off to last fully visible dl */
			int dysT;

			Assert( dlNil < 0 );
			if (DlPartial( hpldr, idr, &dysT ) > 0)
				{
				Assert( dl > 0 );
				dl--;
				}
			}
#endif
		GetPlc(hplcedl, dl, &edl);
		FreePdrf(&drfFetch);
		ywLine = YwFromYp(hpldr, idr, edl.ypTop);

#ifdef BOGUS
/* checks for weird reversals due to page layout (bug 3173) */
		if (sty == styLine && ((fUp && ywLine > yw) || (!fUp &&
				ywLine + edl.dyp < yw)))
			goto LNoClick;
#endif
		/* make sure no one changed it out from under us */
		Assert (psel == PselActive());
		if (psel->doc != docDr)
			{
			if (fDrag)
				{
LBeep:
				/* make sure no one changed it out from under us */
				Assert (psel == PselActive());
				Beep();
				return;
				}
			if (!psel->fNil)
				TurnOffSel(psel);
			else
				{
				/* the displayed cursor is really selCur and we don't
					want to change it to be dotted yet
				*/
				Assert (psel == &selDotted);
				TurnOffSel(&selCur);
				}
			psel->sk = skNil;
			psel->doc = docDr;
			}
		fHandled = fFalse;
		if (fDrag && FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor))
			{
			caInTable = psel->fTable ? psel->ca : caCell;
			if (!(!spt.fInTable && psel->fTable) && FChangeSelToStyCol(psel, &cpFirstCell, pt.xw, 
					hpldr, &idr, dl, &caCell, &caInTable, 
					spt.fInTable || (!psel->fTable && !spt.fInTable && (psel->fForward ? !fUp : fUp))))
				{
				fHandled = FDoContentHitColumn(psel, cpFirstCell, idr, &caInTable, &dl,
						&pt, &hpldr, &idr, &spt, &caTapAnchor, itcAnchor, caCell.cpFirst);
				}
			}
		if (!fHandled)
			{
			int fMakeNonTable = fFalse;
			if (fDrag && psel->fTableAnchor && psel->fIns && !FInCa(psel->doc, cpFirstCell, &caCell))
				{
				CP cpAnchor;
				struct CA caTapAnchor;
				int itcAnchor;

				CpFirstTap(psel->doc, cpAnchor = (psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1));
				caTapAnchor = caTap;
				CacheTc(wwNil, psel->doc, cpAnchor, fFalse, fFalse);
				itcAnchor = vtcc.itc;
				ColumnSelBegin(psel, cpAnchor, itcAnchor, &caTapAnchor, itcAnchor, cpAnchor);
				}
			if (psel->fTable && fDrag)
				{
				MakeSelNonTable(psel, CpPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl,dl));
				FreePdrf(&drfFetch);
				fMakeNonTable = fTrue;
				}
			if (fUp)
				{
				int xpT, ichT;
				CP cpT; 
				struct DR *pdr = PdrFetch(hpldr, idr, &drfFetch);
				struct PLCEDl **hplcedl = pdr->hplcedl;
				struct EDL edl;
				GetPlc(hplcedl, dl, &edl);
			 	if ((cpT =  CpPlc(hplcedl,dl)) < CpMacDocEdit(pdr->doc))
					{ 
					FormatLineDr(wwCur, CpPlc(hplcedl,dl),pdr);
					pt.xp = min(pt.xp, XwFromXp(hpldr, idr, XpFromDcp(edl.dcp, edl.dcp, &xpT, &ichT)) - vfli.rgdxp[vfli.ichMac - 1]);
					} 
				FreePdrf(&drfFetch);
				}
			FSelectDlPt(psel, hpldr, idr, dl, pt, (psel->sty != styLine || !fDrag) ? styChar : styLine, fDrag,
					 fFalse);
			if (fMakeNonTable &&
					(psel->fForward ? (psel->cpLim == caInTable.cpLim) : (psel->cpFirst == caInTable.cpFirst)))
				{
				ChangeSel(psel, psel->fForward ? psel->cpLim + 1 : psel->cpFirst - 1,
						styChar, fTrue, fFalse);
				}
			}
		vfEndCursor = psel->cpLim != cpFirst;
		}
LNoClick:
	if (!FNeRgw(&psel->ca, &caSave, cwCA) && fInsEndSave == psel->fInsEnd &&
			psel->fTable == fTableSave &&
			(!psel->fTable || (psel->itcFirst == itcFirstSave && psel->itcLim == itcLimSave)))
		{
		if (!fAlreadyChanged)
			{
			int ipgd = (*hwwdCur)->ipgd;
/* scroll unless the end of the document has already been reached */
			yw = YwScrollForCursor(sty, fUp, 1, &fScrolled,yw);
			/* ywscroll could have conceivably killed dyadic move
			and thrown out selDotted, which may be psel.
			so rebuild it. danger danger bz
			*/
			psel = PselActive();
			if (!fScrolled)
				Beep();
			fAlreadyChanged = fTrue;
			if ((*hwwdCur)->fPageView && (*hwwdCur)->ipgd != ipgd)
				{
				hpldr = hNil;
				goto LClick;
				}
			}
		}
	else
		{
		if (fPageView && psel->fIns && sty != styScreen &&
				!FYwInWw(*hwwdCur, psel->yw,
				psel->yw - psel->dyp, fFalse))
			{
			SeeSel();
			}
		}
LEnd:
	/* make sure no one changed it out from under us */
	Assert (psel == PselActive());
#ifndef JR
	if (!psel->fIns && (*hwwdCur)->fOutline)
		{
		CP cpFirstT = psel->cpFirst; 
		CP cpLimT = psel->cpLim; 
		OutlineSelCheck(psel);
		if (psel->cpFirst != cpFirstT || psel->cpLim != cpLimT)
			SeeSel1(fTrue); ; 
		}
#endif
	if (--cSty > 0)
		goto LAgain;
	if (psel->fHidden)
		TurnOnSel(psel);
#ifdef WIN
	if (vhwndCBT)
		/* Send CBT a message explaining what we've selected */
		{
		if (psel->fTable)
			CBTTblSelectPsel(psel);
		else
			CBTSelectPsel(psel);
		}
#endif /* WIN */
	vfLastCursor = fTrue;
}


/* %%Function:YwScrollForCursor %%Owner:davidlu */
int YwScrollForCursor(sty, fUp, cSty, pfScrolled,yw)
int sty;
BOOL fUp;
int cSty;
int *pfScrolled;
int	yw;
{
	struct WWD *pwwd = *hwwdCur;
	int ywMin = pwwd->ywMin;
	int ywMac = pwwd->ywMac;
	BOOL fPageView = pwwd->fPageView;
	int ipgd;
	int dysDisp;

#ifdef DCURS
		{
		int rgw [3];

		rgw [0] = sty;
		rgw [1] = fUp;
		rgw [2] = cSty;
		CommSzRgNum( SzShared( "YwScrollForCursor(sty,fUp,cSty): "), rgw, 3 );
		}
#endif
	*pfScrolled = fTrue;
	if (fPageView && fUp && pwwd->yeScroll >= dypGrayOutsideSci)
		{
/* already at the top of the page, go to prev page */
		if ((ipgd = IpgdPrevWw(wwCur)) == ipgdNil)
			goto LNoScroll;
		SetPageDisp(wwCur, ipgd, YeBottomPage(wwCur), fFalse, fFalse);
		return ywMac - 1;
		}
	else  if (fPageView &&
			!fUp && pwwd->yeScroll <= YeBottomPage(wwCur))
		{
/* already at the bottom of the page, go to next page */
		if ((ipgd = IpgdNextWw(wwCur, fTrue)) == ipgdNil)
			goto LNoScroll;
		SetPageDisp(wwCur, ipgd, YeTopPage(wwCur), fFalse, fFalse);
		return ywMin;
		}
	else
		{
#ifndef WIN
		int dysMin = dysMinAveLineSci;
		int dysMac = dysMacAveLineSci;
#else /* WIN */
		int dysMin, dysMac;

		dypCS = -1;
		if (!fPageView && sty == styLine)
			{
			struct PLCEDL **hplcedl;
			int		dypWw;

			hplcedl = PdrGalley(*hwwdCur)->hplcedl;
			dypWw = (*hwwdCur)->ywMac - (*hwwdCur)->ywMin;
			ScrollDelta(cSty, vcConseqScroll + 1,
					(*hplcedl)->dlMax, dypWw, &dysMin, &dysMac);
			dypCS = 0;
			}
		else
			{
			dysMin = dysMinAveLineSci;
			dysMac = dysMacAveLineSci;
			}
#endif
		dysDisp = DyOfRc(&(*hwwdCur)->rcwDisp);
		if (sty == styScreen)
			{
			dysMac = dysDisp;
			dysMin = max(min(dysMac, 10), dysMac - dysMacAveLineSci);
			}
		if (fPageView)
			{
			dysDisp = fUp ? -(dysDisp>>1) : (yw<0 ? (dysDisp>>1)
													: min(dysDisp>>1,yw));
			if(dysDisp)
				if (FScrollPageDyeHome(wwCur,
					dysDisp, sty == styScreen, 0, fFalse))
					fUp = !fUp;
			}
		else
			{
			struct DR *pdr = PdrGalley(pwwd);
			if ((fUp && pdr->cpFirst == cp0 && PdrGalley(*hwwdCur)->dypAbove == 0) ||
					!fUp &&	CpMacPlc(pdr->hplcedl) >= CpMacDoc(pdr->doc) &&
					DlPartial( hwwdCur, 0, &dysMin ) == dlNil )
				{
LNoScroll:                      
				*pfScrolled = fFalse;
				}
			else
				{
#ifdef WIN
				Assert(vcConseqScroll >= 0);
				if (!fPageView && vcConseqScroll < 0x7FFF )
					vcConseqScroll++;
#endif
#ifdef DCURS
					{
					int rgw [2];

					rgw [0] = dysMin;
					rgw [1] = dysMac;
					CommSzRgNum( SzShared( "ScrollUp/Down(dysMin, dysMac): "), rgw, 2 );
					}
#endif
				if (fUp)
/* scroll up one line in the document and click at the top */
					ScrollDown(wwCur, dysMin, dysMac);
				else
					ScrollUp(wwCur, dysMin, max(dysMin, dysMac));
				}
			}
		}
#ifndef WIN
	return fUp ? ywMin : ywMac - 1;
#else
	if (dypCS < 0)
		{
		dypCS = 0;
		}
	return fUp ? ywMin + dypCS : ywMac - dypCS - 1;
#endif
}


/* C M D  D R  C U R S K E Y S */
/* %%Function:CmdDrCurskeys %%Owner:davidlu */
CmdDrCurskeys(pcmb)
CMB *pcmb;
{
	return (CmdDrCurs1(pcmb->ucm, WinMac(vfExtendSel,pcmb->fExtend), fFalse));
}

/* C M D  D R  C U R S  1 */
/* %%Function:CmdDrCurs1 %%Owner:davidlu */
CmdDrCurs1(ucm, fExtend, fLastDlCp)
BOOL fExtend, fLastDlCp;
{
#define xlMax 65535
#define ywMax 65535
	CP cp;
	int idr, idrNext, idrMac, idrT, doc;
	int dl, dlNext;
	int ywLine;
	struct WWD **hwwd;
	struct DR *pdr, *pdrT;
	int dxl, dyl;
	int xwT;
	int fInTable;
	int fIntersect;
	int fRight, fFirst, fIntoDr;
	struct PLDR **hpldr, **hpldrNext, **hpldrOrg;
	int idrOrg;
	struct SEL *psel = PselActive();
	struct EDL edl;
	struct SPT spt;
	struct PLDR **hpldrNextEdl;
	int idrNextEdl = idrNil;
	struct PLDR **hpldrNextDr;
	int idrNextDr = idrNil;
	uns dxlNextEdl = xlMax, dxlNextDr = xlMax;
	int dylNextEdl, dylNextDr;
	uns dywNextEdl = ywMax, dywNextDr = ywMax;
	int dxwNextEdl, dxwNextDr;
	int dxw, dyw, dywDrCriteria, yw;
	struct PLCEDL **hplcedl;
	int ylExt, xlExt;
	int docStart;
	CP cpFirst;
	int fDown, xpDummy, ichDummy;
	struct PLDR **hpldrT, **hpldrPrev;
	struct PT pt;
	struct RC rclEdl, rclDr, rcl, rcwDr, rcw, rcwSect;
	struct FLSS flss;
	struct DRF drfFetch, drfFetchT;
	CP cpStart;

/* this is the base cp from where we are going to move the cursor */
	cp = CpMin(CpMacDocEdit(psel->doc),
			psel->fForward ? psel->cpLim : psel->cpFirst);
	if (psel->fInsEnd && cp > cp0)
		cp--;

/* find if current page contains the selection. if not just start with dr 0,
	dl 0. */
	if ((dl = DlWhereDocCp(wwCur, psel->doc, cp, fFalse, &hpldr, &idr,
			&cpFirst, NULL, fTrue)) < 0)
		{
		hpldr = hwwdCur;
		idr = 0;
		dl = 0;
		pdr = PdrFetch(hpldr, idr, &drfFetch);
		if (pdr->doc < 0)
			goto LBeep;
		cpFirst = pdr->cpFirst;
		}
	else
		pdr = PdrFetch(hpldr, idr, &drfFetch);
	for (;;)
		{
		if (pdr->fInTable)
			{
			CacheTc(wwNil, pdr->doc, pdr->cpFirst, fFalse, fFalse);
			CpFirstTap(pdr->doc, pdr->cpFirst);
			if (!vtapFetch.rgtc[vtcc.itc].fMerged)
				break;
			NextPrevHpldrIdr(&hpldr, &idr, fFalse);
			pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
			continue;
			}
		break;
		}
	cpStart = selCur.cpFirst;
	docStart = selCur.doc;
	switch (ucm)
		{
	case ucmNextDr:
	case ucmPrevDr:
/* find the yw coordinate of the top of the line */
		GetPlc(pdr->hplcedl, dl, &edl);
		ywLine = YwFromYp(hpldr, idr, edl.ypTop + edl.dyp);

/* calculate next idr */
		idrMac = (*hpldr)->idrMac;
		fInTable = pdr->fInTable;
		hpldrNext = hpldr;
		for (idrNext = idr; ; )
			{
			NextPrevHpldrIdr(&hpldrNext, &idrNext, 
					ucm == ucmNextDr);

			if (idrNext == idr && hpldr == hpldrNext)
				{
LBeep:				
				FreePdrf(&drfFetch);
				Beep();
				return cmdError;
				}
			hpldr = hpldrNext;
			pdr = PdrFreeAndFetch(hpldr, idrNext, &drfFetch);
			if (FSelectableDr(pdr))
				break;
			}
		cp = pdr->cpFirst;
		break;
	case ucmDrRight:
	case ucmDrLeft:
LLeftRight:
		fRight = (ucm == ucmDrRight);
		fInTable = pdr->fInTable;
		if (!vfLastDrCursor)
			{
/* get rcls of the dl and dr that contain cp */
			GetPlc(pdr->hplcedl, dl, &edl);
			DrcpToRcl(hpldr, idr, &edl.drcp, &rclEdl);
			vylDrCursor = rclEdl.ylTop;
			}
		DrclToRclOuter(hpldr, &pdr->drcl, &rclDr);
/* if inside of a table and we're moving right and we have not exhausted DRs
	in table row, we know proper DR is the table DR to the right. */
		if (fInTable && fRight && idr + 1 < (*hpldr)->idrMac)
			{
			hpldrNextEdl = hpldr;
			idrNextEdl = idr + 1;
			}
/* if inside of a table and we're moving left and we have not exhausted DRs
	in table row, we know proper DR is the table DR to the left. */
		else  if (fInTable && !fRight && idr > 0)
			{
			hpldrNextEdl = hpldr;
			idrNextEdl = idr - 1;
			}
		else
	/* we need to visit every DR on the page till we get back to the currently
	selected DR */
			{
			for (hpldrT = hpldr, idrT = idr, 
					NextPrevHpldrIdr(&hpldrT, &idrT, fTrue);
					hpldrT != hpldr || idrT != idr; 
					NextPrevHpldrIdr(&hpldrT, &idrT, fTrue))
				{
				pdrT = PdrFetchAndFree(hpldrT, idrT, &drfFetchT);
				if (!FSelectableDr(pdrT))
					continue;
/* get rcl of the dr we're testing */
				DrclToRclOuter(hpldrT, &pdrT->drcl, &rcl);
/* only candidates are drs that begin to the right/left of the dr that contains	
	the selection */
				dxl = (fRight ? (rcl.xlLeft - rclDr.xlRight) : (rclDr.xlLeft - rcl.xlRight));
/* we look first for a DR that envelops vylDrCursor which is closest to the
	currently selected DR in xl-space and whose ylTop is closest to vylDrCursor*/
				if (dxl >= 0)
					{
/* check if dr envelopes the top of the EDL containing the selection in y-space*/
					if (rcl.ylTop <= vylDrCursor && vylDrCursor < rcl.ylBottom)
						{
						dyl = vylDrCursor - rcl.ypTop;
						fIntoDr = fFalse;
/* we will record the closest dr in x-space */
						if (dxl <= dxlNextEdl ||
								(fIntoDr = (!fRight && pdrT->fInTable && (*hpldrT)->hpldrBack == hpldrNextEdl &&
								(*hpldrT)->idrBack == idrNextEdl)))
							{
							if (dxl < dxlNextEdl || fIntoDr || dyl < dylNextEdl)
								{
								hpldrNextEdl = hpldrT;
								idrNextEdl = idrT;
								dxlNextEdl = dxl;
								dylNextEdl = dyl;
								}
							}
						}
/* as second priority we look for a DR whose extent in yl-space intersects
	the extent of the currently selected DR, that is closest to the current
	DR in xl-space, whose top is closest to vylDrCursor */
					else  if (max(rcl.ylTop, rclDr.ylTop) < 
							min(rcl.ylBottom, rclDr.ylBottom))
						{
						dyl = abs(rcl.ylTop - vylDrCursor);
						fIntoDr = fFalse;
/* we give precedence to the dr that is closest to the selection dr in x-space*/
						if (dxl <= dxlNextDr ||
								(fIntoDr = (!fRight && pdrT->fInTable && 
								(((*hpldrT)->hpldrBack == hpldrNextDr &&
								(*hpldrT)->idrBack == idrNextDr) ||
								(dxlNextEdl < xlMax && (*hpldrT)->hpldrBack == hpldrNextEdl &&
								(*hpldrT)->idrBack == idrNextEdl)))))
							{
							if (dxl < dxlNextDr || fIntoDr || dyl < dylNextDr)
								{
								hpldrNextDr = hpldrT;
								idrNextDr = idrT;
								dxlNextDr = dxl;
								dylNextDr = dyl;
								}
							}
						}
					}
				}
			}
		goto LChooseHpldrIdr;
	case ucmDrUp:
	case ucmDrDown:
		fDown = (ucm == ucmDrDown);
		hplcedl = pdr->hplcedl;
		GetPlc(hplcedl, dl, &edl);
		yw = YwFromYp(hpldr, idr, edl.ypTop);

		if (!vfLastCursor)
			{
			FormatLineDr(wwCur, cpFirst, pdr);
			vxwCursor = XwFromXp(hpldr, idr,
					XpFromDcp(cp0, cp - vfli.cpMin,
					&xpDummy, &ichDummy));
			}
		DrclToRcw(hpldr, &pdr->drcl, &rcwDr);
		if (ucm == ucmDrUp && pdr->fInTable)
			{
			if (idr + 1 == (*hpldr)->idrMac)
				rcwDr.ywTop--;
			rcwDr.ywBottom = rcwDr.ywTop + DylMacForTableDr(pdr->cpFirst, hpldr);
			}
		dywDrCriteria = (fDown ? rcwDr.ywBottom - yw : yw - rcwDr.ywTop);

		for (hpldrT = hpldr, idrT = idr, 
				NextPrevHpldrIdr(&hpldrT, &idrT, fTrue);
				hpldrT != hpldr || idrT != idr; 
				NextPrevHpldrIdr(&hpldrT, &idrT, fTrue))
			{
			pdrT = PdrFetchAndFree(hpldrT, idrT, &drfFetchT);
			if (!FSelectableDr(pdrT))
				continue;
/* get rcl of the dr we're testing */
			DrclToRcw(hpldrT, &pdrT->drcl, &rcw);
			if (ucm == ucmDrUp && pdrT->fInTable)
				{
				if (idrT + 1 == (*hpldrT)->idrMac)
					rcw.ywTop--;
				rcw.ywBottom = rcw.ywTop + DylMacForTableDr(pdrT->cpFirst, hpldrT);
				}
			fIntersect = (SectRect(&rcwDr, &rcw, &rcwSect) && 
					!FNeRgch(&rcwDr, &rcwSect, sizeof(struct RC)));

/* only candidates are drs that begin to the right/left of the dr that contains	
	the selection */
			dyw = fDown ? (fIntersect ? (rcw.ywBottom - yw) : (rcw.ywTop - yw)) :
			(fIntersect ? (yw - rcw.ywTop) : (yw - rcw.ywBottom));

/* we look first for a DR that envelops vxwCursor which is closest to the
	currently selected DR in yw-space and whose xwLeft is closest to vxwCursor*/
			if (dyw >= 0)
				{
				dxw = abs(rcw.xwLeft - vxwCursor);
/* check if dr envelopes the top of the EDL containing the selection in x-space*/
				if (rcw.xwLeft <= vxwCursor && vxwCursor < rcw.xwRight)
					{
/* we will record the closest dr in yw-space. if the closest we've found so
	far is the mother DR of the current DR and we're moving up, we use the
	current DR */
					fIntoDr = fFalse;
					if (dyw <= dywNextEdl || (fIntoDr = (!fDown && pdrT->fInTable && (*hpldrT)->hpldrBack == hpldrNextEdl &&
							(*hpldrT)->idrBack == idrNextEdl)))
						{
/* if we have equality in yw-space, we will pick the closest in xw-space */
						if (dyw < dywNextEdl || fIntoDr || dxw < dxwNextEdl)
							{
							hpldrNextEdl = hpldrT;
							idrNextEdl = idrT;
							dywNextEdl = dyw;
							dxwNextEdl = dxw;
							}
						}
					}
/* as second priority we look for a DR that is closest to the current
	DR in yw-space, whose left is closest to vxwCursor */
				else  if (dyw >= dywDrCriteria)
					{
					fIntoDr = fFalse;
					if (dyw <= dywNextDr ||
							(fIntoDr = (!fDown && pdrT->fInTable &&
							(((*hpldrT)->hpldrBack == hpldrNextDr &&
							(*hpldrT)->idrBack == idrNextDr) ||
							(dywNextEdl < ywMax && (*hpldrT)->hpldrBack == hpldrNextEdl &&
							(*hpldrT)->idrBack == idrNextEdl)))))
						{
						if (dyw < dywNextDr || fIntoDr || dxw < dxwNextDr)
							{
							hpldrNextDr = hpldrT;
							idrNextDr = idrT;
							dxwNextDr = dxw;
							dywNextDr = dyw;
							}
						}
					}
				}
			}
LChooseHpldrIdr:
		if (idrNextEdl != idrNil)
			{
			if (idrNextDr != idrNil && dywNextDr < dywNextEdl)
				goto LNextDr;
			hpldr = hpldrNextEdl;
			idr = idrNextEdl;
			if (ucm == ucmDrLeft || ucm == ucmDrRight)
				{
				pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
				if (pdr->fInTable)
					{
					CacheTc(wwNil, pdr->doc, pdr->cpFirst, fFalse, fFalse);
					CpFirstTap(pdr->doc, pdr->cpFirst);
					if (vtapFetch.rgtc[vtcc.itc].fMerged)
						{
						cpFirst = pdr->cpFirst;
						vfLastDrCursor = fTrue;
						goto LLeftRight;
						}
					}
				}
			}
		else
			{
LNextDr:
			hpldr = hpldrNextDr;
			idr = idrNextDr;
			}
		if (idr == idrNil)
			goto LBeep;
		pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
		if (ucm == ucmDrLeft || ucm == ucmDrRight)
			cp = pdr->cpFirst;
		else
			{
			int fltDummy;
			int fContained = fFalse;
			CP cpLim = CpPlc(pdr->hplcedl, IMacPlc(pdr->hplcedl));
			CachePara(pdr->doc, cpLim - 1);
			if (caPara.cpLim == cpLim)
				cpLim -= ccpEop;
			else
				cpLim--;
			GetPlc(pdr->hplcedl, 0, &edl);
			if (edl.hpldr != hNil)
				{
				int idrLastNonMerge = 0;
				hpldr = edl.hpldr;
				idrMac = (*hpldr)->idrMac;
				for (idr = 0; idr < idrMac; idr++)
					{
					pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
					if (pdr->fInTable)
						{
						CacheTc(wwNil, pdr->doc, pdr->cpFirst, fFalse, fFalse);
						CpFirstTap(pdr->doc, pdr->cpFirst);
						if (vtapFetch.rgtc[vtcc.itc].fMerged)
							continue;
						}
					idrLastNonMerge = idr;
					DrclToRcw(hpldr, &pdr->drcl, &rcw);
					if (rcw.xwLeft <= vxwCursor && vxwCursor < rcw.xwRight)
						goto LSearchCp;
					}
				idr = (vxwCursor >= rcw.xwRight)	? idrLastNonMerge : 0;
				}
LSearchCp:
			cp = CpMin(CpFromDlXp(wwCur, hpldr, idr, 0, XpFromXw(hpldr, idr, vxwCursor),
					&fltDummy, &flss), cpLim);
			if (cp == flss.cpMac && (flss.chBreak == chEop || flss.chBreak == chTable))
				cp -= ccpEop;
			}
		break;
	case ucmDrFirst:
	case ucmDrLast:
		fFirst = (ucm == ucmDrFirst);
		DrclToRclOuter(hpldr, &pdr->drcl, &rcl);
		ylExt = rcl.ylTop;
		xlExt = rcl.xlLeft;
		for (hpldrT = hpldrOrg = hpldr, idrT = idrOrg = idr, 
				NextPrevHpldrIdr(&hpldrT, &idrT, fTrue);
				hpldrT != hpldrOrg || idrT != idrOrg; 
				hpldrPrev = hpldrT, NextPrevHpldrIdr(&hpldrT, &idrT, fTrue))
			{
			pdrT = PdrFetchAndFree(hpldrT, idrT, &drfFetchT);
			if (!FSelectableDr(pdrT))
				continue;
/* get rcl of the dr we're testing */
			DrclToRclOuter(hpldrT, &pdrT->drcl, &rcl);
			if (fFirst)
				{
				if (rcl.ylTop <= ylExt)
					{
					if (rcl.ylTop < ylExt || rcl.xlLeft < xlExt)
						{
						hpldr = hpldrT;
						idr = idrT;
						ylExt = rcl.ylTop;
						xlExt = rcl.xlLeft;
						}
					}
				}
			else
				{
				if (rcl.ylTop >= ylExt)
					{
					if (rcl.ylTop > ylExt || rcl.xlLeft > xlExt)
						{
						hpldr = hpldrT;
						idr = idrT;
						ylExt = rcl.ylTop;
						xlExt = rcl.xlLeft;
						}
					}
				}
			}
		FreePdrf(&drfFetch);
		pdr = PdrFetch(hpldr, idr, &drfFetch);
		cp = pdr->cpFirst;
		if (fLastDlCp)
			{
			struct PLCEDL **hplcedl = pdr->hplcedl;
			if (hplcedl && IMacPlc(hplcedl))
				cp = CpPlc(hplcedl, IMacPlc(hplcedl) - 1);
			}
		break;
		}
	FreePdrf(&drfFetch);
	NormCp(wwCur, doc = pdr->doc, cp, ncpHoriz, 20, fFalse);
	if (cpStart == cp && docStart == doc)
		Beep();
	/* make sure no one changed it out from under us */
	Assert (psel == PselActive());
	if (WinMac(vfExtendSel, fExtend) && psel->doc == doc)
		ChangeSel(psel, cp, styNil, fFalse, fFalse);
	else
		{
		if (psel->doc != doc)
			TurnOffSel(psel);
		psel->doc = doc;
		SelectIns(psel, cp);
		}
	if (ucm == ucmDrLeft || ucm == ucmDrRight)
		vfLastDrCursor = fTrue;
	if (ucm == ucmDrUp || ucm == ucmDrDown)
		vfLastCursor = fTrue;
	return cmdOK;
}


/* %%Function:DrclToRclOuter %%Owner:davidlu */
DrclToRclOuter(hpldr, pdrcl, prcl)
struct PLDR **hpldr;
struct DRC *pdrcl;
struct RC *prcl;
{
	struct PT pt;
	pt = PtOrigin(hpldr, -1);
	*prcl = *pdrcl;
	prcl->ylTop += pt.yl;
	prcl->xlLeft += pt.xl;
	DrcToRc(prcl, prcl);
}


/* N E X T  P R E V  H P L D R  I D R */
/* %%Function:NextPrevHpldrIdr %%Owner:davidlu */
NextPrevHpldrIdr(phpldr, pidr, fNext)
struct PLDR ***phpldr;
int *pidr;
int fNext;
{
	struct PLDR **hpldr;
	int idr;
	CP cp;
	int fInTable;
	int dl, dlMac;
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct DRF drfFetch;

	hpldr = *phpldr;
	idr = *pidr;

	EnsureDrComplete(hpldr, idr);
	pdr = PdrFetch(hpldr, idr, &drfFetch);
LSkipDr:
	fInTable = pdr->fInTable;
	hplcedl = pdr->hplcedl;
	dl = fNext ? 0 : - 1;

LRestart:
	if (fNext)
		{
		if (!fInTable)
			{
			for (dlMac = IMacPlc(hplcedl); dl < dlMac; dl++)
				{
				GetPlc(hplcedl, dl, &edl);
				if (edl.hpldr)
					{
					*phpldr = edl.hpldr;
					*pidr = 0;
					goto LRet;
					}
				}
			}
		idr++;
		if (idr >= (*hpldr)->idrMac)
			{
			if (!fInTable)
				idr = 0;
			else
				goto LContainingDr;
			}
		}
	else
		{
		if (fInTable || dl < 0)
			{
			idr--;
			if (idr < 0)
				{
				if (!fInTable)
					idr = (*hpldr)->idrMac - 1;
				else
					{
LContainingDr:
					cp = pdr->cpFirst;
					idr = (*hpldr)->idrBack;
					hpldr = (*hpldr)->hpldrBack;
					EnsureDrComplete(hpldr, idr);
					pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
					dl = IInPlc(hplcedl = pdr->hplcedl, cp);
					fInTable = fFalse;
					if (fNext)
						dl++;
					else
						dl--;
					goto LRestart;
					}
				}
			EnsureDrComplete(hpldr, idr);
			pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
			hplcedl = pdr->hplcedl;
			dl = IMacPlc(hplcedl) - 1;
			}
		if (!fInTable)
			{
			for (; dl >= 0; dl--)
				{
				GetPlc(hplcedl, dl, &edl);
				if (edl.hpldr)
					{
					*phpldr = edl.hpldr;
					*pidr = (*(struct PLDR **)edl.hpldr)->idrMac - 1;
					goto LRet;
					}
				}
			}
		}
	*phpldr = hpldr;
	*pidr = idr;
LRet:
	EnsureDrComplete(hpldr, idr);
	pdr = PdrFreeAndFetch(hpldr, idr, &drfFetch);
	if (pdr->fInTable)
		{
		CacheTc(wwNil, pdr->doc, pdr->cpFirst, fFalse, fFalse);
		CpFirstTap(pdr->doc, pdr->cpFirst);
		if (vtapFetch.rgtc[vtcc.itc].fMerged)
			goto LSkipDr;
		}
	FreePdrf(&drfFetch);
}


/* %%Function:DylMacForTableDr %%Owner:davidlu */
DylMacForTableDr(cp, hpldr)
CP cp;
struct PLDR **hpldr;
{
	struct PLDR **hpldrT;
	int idrT;
	struct DR *pdrT;
	int dl;
	struct PLCEDL **hplcedl;
	struct DRF drfFetch;
	struct EDL edl;

	idrT = (*hpldr)->idrBack;
	hpldrT = (*hpldr)->hpldrBack;
	EnsureDrComplete(hpldrT, idrT);
	pdrT = PdrFetch(hpldrT, idrT, &drfFetch);
	dl = IInPlc(hplcedl = pdrT->hplcedl, cp);
	GetPlc(hplcedl, dl, &edl);
	FreePdrf(&drfFetch);
	return edl.dyp;
}


/* %%Function:EnsureDrComplete %%Owner:davidlu */
EnsureDrComplete(hpldr, idr)
struct PLDR **hpldr;
int idr;
{
	int ww, wwT;
	struct DRF drfFetch;
	struct DR *pdr = PdrFetch(hpldr, idr, &drfFetch);

	if (pdr->fIncomplete)
		{
#ifdef MAC
		ww = WwFromHwwd(hpldr);
#else /* WIN */
	/* Since this is the only caller of WwFromHwwd, we do it in line */
		ww = wwNil;
		for (wwT = 0; wwT < wwMax; wwT++)
			if (mpwwhwwd[wwT] == hpldr)
				{
				ww = wwT;
				break;
				}
#endif /* WIN */
		pdr->fDirty = fTrue;
/* this return is only used by cursor key routines, which can tolerate the
	DR being left incomplete when FUpdateDr returns fFalse. */
		FUpdateDr(ww, hpldr, idr, PwwdWw(ww)->rcwInval,
				fFalse /* abort not allowed*/,
				udmodNormal, cpNil);
		}
	FreePdrf(&drfFetch);
}



/* C M D  P R E V  I N S E R T */
/* goto previous insertion point key.
Plan:
0. if previous command was a scroll, just normalize.
1. if selCur is eq any in rgselsInsert, then select one valid before it in round
robin.
2. else, jam selCur as last, goto 1.
3. beep if selCur is not changed.
*/
/* %%Function:CmdPrevInsert %%Owner:rosiep */
CmdPrevInsert(pcmb)
CMB *pcmb;
{
	int isels;
	int doc;
	int ww;
	int iselsCount = 0;
	struct SELS *psels;
	struct SELS selsCur, selsNew;

#ifdef DCURS
		{
		int rgw [iselsInsertMax*2];
		int i, *pw;

		for ( i = 0, pw = &rgw [0] ; i < iselsInsertMax ; i++ )
			{
			*pw++ = (int) rgselsInsert [i].cpFirst;
			*pw++ = (int) rgselsInsert [i].cpLim;
			}

		CommSzSz( SzShared( "CmdPrevInsert, cache is: "),rgw, iselsInsertMax*2);
		}
#endif
#ifdef WIN
	if (!fElActive) /* always goback if running macro */
#endif
		{
		UpdateWw(wwCur, fFalse);
		if (!FCpVisible(wwCur, selCur.doc, CpActiveCur(), selCur.fIns & selCur.fInsEnd, fTrue, fFalse))
			{
			vfSeeSel = fTrue;
			return cmdOK;
			}
		}
/* supress fields in selCur that are unimportant in the comparison. */
	selsCur = *(struct SELS *)&selCur;
	NormalizeSels(&selsCur);
	for (;;)
		{
		for (isels = 0, psels = &rgselsInsert[0]; isels < iselsInsertMax; isels++)
			{
			if (!FNeRgw(&((psels++)->ca), &(selsCur.ca), cwCA))
				goto LFound;
			}
/* not found */
		PushInsSelCur();
#ifdef DCURS
			{
			int rgw [iselsInsertMax*2];
			int i, *pw;

			for ( i = 0, pw = &rgw [0] ; i < iselsInsertMax ; i++ )
				{
				*pw++ = (int) rgselsInsert [i].cpFirst;
				*pw++ = (int) rgselsInsert [i].cpLim;
				}

			CommSzRgNum( SzShared( "  not found, push, cache is: "),rgw, iselsInsertMax*2);
			}
#endif
		}
LFound:
#ifdef DCURS
	CommSzNum( SzShared( "  LFound: isels is "), psels - rgselsInsert );
#endif
	for (;;)
		if ((doc = (psels = &rgselsInsert[((uns)(iselsInsertMax + (--isels)))
			% iselsInsertMax])->doc) != docNil &&
				(ww = WwCouldDisp(doc)) != wwNil)
			break;
#ifdef DCURS
	CommSzNum( SzShared( "  after doc loop: isels is "), psels - rgselsInsert );
#endif
	if (FNeRgw(psels, &selsCur, cwSELS))
		{
		selsNew = *psels;
		selsNew.cpFirst = CpMin(selsNew.cpFirst, CpMacDocEdit(selsNew.doc));
		selsNew.cpLim = selsNew.fIns ? selsNew.cpFirst : CpMin(selsNew.cpLim,
			CpMacDoc(selsNew.doc));

		if (doc != selCur.doc)
/* previous insertion point is in a different window */
			{
			Mac(SelectWindow(PwwdWw(ww)->wwptr));
			NewCurWw(ww, fTrue);
			/* this will make sure selCur.chp gets updated, even if
			selsNew == the new selCur (SetPselSels won't inval props
			if sel is same. bz
			*/
			InvalSelCurProps (fTrue /* fSelChanged */);
			}
#ifdef DCURS
		CommSzRgNum( SzShared( "  setting sel (cpFirst,cpLim): "), &selsNew.cpFirst, 4 );
#endif
		SetSelCurSels(&selsNew);
		vfSeeSel = fTrue;
		}
	else
		{
		if (++iselsCount < iselsInsertMax)
			goto LFound;
		Beep();
		}
	return cmdOK;
}


/* W W  C O U L D  D I S P */
/* %%Function:WwCouldDisp %%Owner:davidlu */
WwCouldDisp(doc)
{
	int ww;
	if ((ww = WwDisp(doc, wwNil, fTrue)) != wwNil)
		return ww;
	if (PdodDoc(doc)->dk == dkDispHdr)
		return wwNil;
	for (ww = WwDisp(DocMother(doc), wwNil, fTrue); ww != wwNil; 
			ww = PwwdWw(ww)->wwDisp)
		if (PwwdWw(ww)->fPageView)
			break;
	return ww;
}


#ifdef MAC
/* C P  B A C K  O V E R  I N V I S I B L E */
/* check for backing into hidden text or no-show outline.
	outline state taken from ww.
*/
/* %%Function:CpBackOverInvisible %%Owner:NOTUSED */
CP CpBackOverInvisible(ww, doc, cp)
int ww;
int doc;
CP cp;
{
#ifndef JR
#ifdef MAC	
	if (!vpref.fSeeHidden)
		{
		CachePara(doc, cp);
		FetchCp(doc, cp, fcmProps);
		if (vchpFetch.fVanish)
			{
/* back up to first hidden char. This will be done by probing backwards
in 20 cp steps until a not hidden char is found and then finding the
trailing connected hidden runs in the 20. */
			while (cp > cp0)
				{
				CP cpP20 = cp;
				cp = CpMax(cp0, cp - 20);
				CachePara(doc, cp);
				FetchCp(doc, cp, fcmProps);
				if (!vchpFetch.fVanish)
					{
					for (;;)
						{
						CP cpT = vcpFetch + vccpFetch;
						if (!vchpFetch.fVanish)
							cp = cpT;
						if (cpT >= cpP20)
							break;
						CachePara(doc, cpT);
						FetchCp(docNil, cpNil, fcmProps);
						}
					break;
					}
				}
			if (cp > cp0)
				cp--;
			}
		}
#else /* WIN */
	cp = CpLimSty(ww, doc, cp, styChar, fTrue);
#endif
	if (FInTableDocCp(doc, cp))
		{
LCacheTc:
		CacheTc(wwNil, doc, cp, fFalse, fFalse);
		CpFirstTap(doc, cp);
		if (vtapFetch.rgtc[vtcc.itc].fMerged)
			{
			cp = vmpitccp[vtcc.itc] - ccpEop;
			goto LCacheTc;
			}
		}

/* check for outline no show paras and not-first line of body text para */
	if (PwwdWw(ww)->fOutline)
		{
		struct PLCPAD **hplcpad = PdodDoc(doc)->hplcpad;
		int ipad = IInPlc(hplcpad, cp);
		int ipadShow = ipad;
		CP cpT;
		struct PAD pad;
		GetPlc(hplcpad, ipad, &pad);
			{
			while (!pad.fShow && ipad != 0)
				{
				cp = CpPlc(hplcpad, ipad) - ccpEop;
				GetPlc(hplcpad, --ipad, &pad);
				}
			}
		if (!pad.fShow)
			return cpNil;
		if (pad.fBody && WinMac(PwwdWw(ww)->fEllip, !vpref.fOtlShowBody))
			{
/* are we beyond the first line of the paragraph?
yes => reduce cp to point to last char */
			FormatLine(ww, doc, CpPlc(hplcpad, ipad));
			cp = CpMin(cp, vfli.cpMac - 1);
			}
		}
#endif /* JR */
	return cp;
}


#endif


#ifdef MAC
/* %%Function:CmdSelectAll %%Owner:NOTUSED */
CmdSelectAll(pcmb)
CMB *pcmb;
{
	int ihdd;
	struct PLC **hplchdd;

#ifdef SUBDRAW
	if (fSubdraw)
		{
		SubdrawCmdSelectAll();
		return cmdOK;
		}
#endif /* SUBDRAW */

	PushInsSelCur();
	if (!PdodDoc(selCur.doc)->fHdr)
		{
		Select(&selCur, cp0, CpMacDoc(selCur.doc));
		selCur.sty = styDoc;
		}
	else
		{
		ihdd = IInPlc(hplchdd = PdodDoc(selCur.doc)->hplchdd, selCur.cpFirst);
		Select(&selCur, CpPlc(hplchdd, ihdd), CpPlc(hplchdd, ihdd+1)-1);
		}

	return cmdOK;

}


/* %%Function:CmdScrollUpDown %%Owner:NOTUSED */
CmdScrollUpDown(pcmb)
CMB *pcmb;
{
	vwwScroll = wwCur;
	if (pcmb->ucm == ucmScrollUp)
		ScrollSpc((*hwwdCur)->fPageView? spcPgvLineUp : spcLineUp);
	else
		ScrollSpc((*hwwdCur)->fPageView? spcPgvLineDown : spcLineDown);
	UpdateWw(wwCur, fFalse);
	return cmdOK;
}


#endif

/* D L  P A R T I A L */
/* if dl at bottom of hpldr is clipped at the bottom, return the 
	dl and the amount of it that's clipped off; else return dlNil */

/* %%Function:DlPartial %%Owner:davidlu */
DlPartial(hpldr, idr, pdys)
struct PLDR **hpldr;
int idr;
int *pdys;
{
	struct DRF drfFetch;
	struct DR *pdr = PdrFetch(hpldr, idr, &drfFetch);
	struct PLCEDl **hplcedl = pdr->hplcedl;
	int dl;
	int ywBottom, dys;
	struct EDL edl;

	if ((dl = IMacPlc(hplcedl) - 1) < 0)
		dl = dlNil;
	else
		{
		GetPlc(hplcedl, dl, &edl);
		if (edl.fEnd || 
				(YwTopForDl(hpldr, idr, dl, &ywBottom), 
				(dys = ywBottom - (*hwwdCur)->ywMac) <= 0))
			dl = dlNil;
		}

	FreePdrf(&drfFetch);
	if (dl != dlNil)
		*pdys = dys;
	return dl;
}


