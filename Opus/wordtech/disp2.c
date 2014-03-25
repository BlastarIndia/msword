/* D I S P 2 . C */

#ifdef MAC
#define	COLOR
#include "toolbox.h"
#endif /* MAC */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "sel.h"
#include "doc.h"
#include "ch.h"
#include "format.h"
#include "screen.h"
#include "scc.h"
#include "layout.h"

#ifdef MAC 
#include "outline.h"	/* included for native #define generation */
#include "help2.h"
#endif


#ifdef PROTOTYPE
#include "disp3.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */

extern struct SCC       *vpsccBelow;
extern struct TCC	vtcc;
extern struct FLI       vfli;
extern struct PAP       vpapFetch;
extern struct TAP       vtapFetch;
extern struct PREF      vpref;
extern CP               vcpFirstLayout;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA        caTap;

extern struct MERR      vmerr;

extern struct SAB       vsab;

#ifdef WIN
extern struct SCI       vsci;
extern struct FTI       vfti;
extern int		vflm;
extern int		dypCS;
extern HBRUSH		vhbrGray;
extern HBRUSH		vhbrBlack;
extern HBRUSH		vhbrWhite;
#endif

#ifdef	MAC
extern QDG		vqdg;
extern PATTERN          patGray2;
#endif

#ifdef DEBUG
extern struct DBS       vdbs;
extern int              vfCheckPlc;
#endif /* DEBUG */



BOOL                    vfEndPage;



#ifdef DEBUGORNOTWIN
/* F  U P D A T E  D R  
Updates dr specified by idr in hpldr(or hwwd) in window ww.
Returns true if update was completed with no errors or interruptions.

ww: drawing environment, + fOutline, fPageView, etc. modes.
hpldr: a hpldr or hwwd in which the dr lives
idr: the index of the dr
rcwInval : explicit invalid RC to be redrawn

udmodNormal: update whole dr whether dl's are visible or not. Stop after
cpUpd has been included in hplcedl ( < Mac!) if cpUpd is != cpNil.
udmodLastDr: stops when dl's which are below the visible area are encountered.
(fComplete will be set to fFalse in that case)
umodTable: updating will continue below the dyl limit of the dr, although
dl's will not be displayed.  Dl's not drawn will be left dirty. height of
dr will be updated to full height.  Scan will end with end of dr/end of
page, end of document character only.
umodNoDisplay: only dirty edl's are created with correct cp's and yp,
with no display. We can assume dt starts empty in this case and that
fPageView is true.
udmodLimit: update is stoped after cpUpd is updated.

Also returns:
fComplete in dr is set iff all dl's in the dr are updated. This need not
be the case in the last dr on a page which will not be normally updated
where it is not visible.
end of scan is returned in cpLim of hplcedl of dr (or cpNil if dlMac == 0)
if udmodTable, dyl of dr will be updated to actual height. if this is
shorter than previous height, space between will be blanked.
vfEndPage is set iff the following dr's in the text stream should be
cleared because this dr ended with an end page character.
*/
/*************************/
/* F  U P D A T E  D R */
#ifndef WIN
/* %%Function:FUpdateDr %%Owner:tomsax */
NATIVE int FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd) /* WINIGNORE - MAC only */
#else /* WIN */
HANDNATIVE int C_FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd)
#endif /* WIN */
struct PLDR **hpldr;
struct RC rcwInval;
CP cpUpd;
{
	struct WWD **hwwd, *pwwd;
	struct DR *pdr;
	int dlMac;
	int dlOld;
	int dlNew;
	int doc, docMother;
	struct PLCEDL **hplcedl;
	int ypTop;
	int dyl;
	int ypFirstInval;
	int ypLimInval;
	int ypBottomOld;
	int ypMacOld;
	int dlMacT;
	int emk;
	BOOL fPageView, dlk = dlkNil;
	BOOL fPVNTS; /* PageViewNotTable and Successor */
	BOOL fCacheValid, fLimSuspect;
	BOOL fOutline;
	BOOL fScrollOK, fIncomplete;
	BOOL fReturn;
	BOOL fRMark;
	int ypFirstShow, ypLimShow, ypLimWw;
	struct MWD *pmwd;
	struct EDL *pedl, edl;
	struct DOD *pdod;
	BOOL fFtn;
	CP cp, cpT, cpLim, cpMac;
	struct DRF drfFetch;

#ifdef DEBUG
	int fCheckPlcSave = vfCheckPlc;
	BOOL fInTable;
#endif
	Debug(vfCheckPlc = fFalse);
	hwwd = HwwdWw(ww);
	vfEndPage = fRMark = fFalse;

	pdr = PdrFetch(hpldr, idr, &drfFetch);
LRestart:
	if (pdr->doc == docNil)
		{
		/* header couldn't allocate a doc */
		pdr->fCpBad = pdr->fDirty = fFalse;
		Debug(vfCheckPlc = fCheckPlcSave);
		fReturn = fTrue;
		goto LRet;
		}
	if ( !pdr->fDirty )
		{
		/* don't need to update a DR that's not dirty! */
		Assert ( !pdr->fCpBad );
		Debug(vfCheckPlc = fCheckPlcSave);
		fReturn = fTrue;
		goto LRet;
		}

	Win( pdr->xwLimScroll = 0 );
/* create plcedl if there is none */
	if (pdr->hplcedl == hNil)
		{
		if (!FInitHplcedl(1, pdr->cpFirst, hpldr, idr))
			{
			fReturn = fFalse;
			goto LRet;
			}
		}
	hplcedl = pdr->hplcedl;
	pwwd = *hwwd;
	fOutline = pwwd->fOutline;
	Mac(vfli. fStopAtPara =) fPageView = pwwd->fPageView;
	fScrollOK = FScrollOK(ww);

	Mac(fCacheValid = !fPageView && vpsccBelow && vpsccBelow->ww == ww &&
			vpsccBelow->doc == pdr->doc && !fOutline && fScrollOK);
	fPVNTS = fPageView && udmod != udmodTable && pdr->idrFlow != idrNil;

	fIncomplete = fFalse;
/* displacement to convert yw to yl */
	dlMac = IMacPlc(hplcedl);
	doc = pdr->doc;
	if (doc < 0)
		{
		cpMac = ccpEop + 1;
		fFtn = fFalse;
		}
	else
		{
		cpMac = CpMacDoc(doc);
		pdod = PdodDoc(doc);	/* verbose for native compiler bug */
		fFtn = fPageView && pdod->fFtn;
		}
	fLimSuspect = pdr->fLimSuspect;

	if (pdr->cpLim == cpNil || (pdr->fIncomplete && fLimSuspect))
		{
		cpLim = cpMac;
		if (fFtn && dlMac > 0)
			cpLim = CpLimFtnCp(doc, CpMax(cp0, CpPlc(hplcedl, dlMac - 1) - 1));
		}
	else
		{
		cpLim = pdr->cpLim;
		if (fFtn && fLimSuspect)
			{
			fLimSuspect = fFalse;
			cpLim = CpLimFtnCp(doc, cpLim - 1);
			}
		}
	ypFirstShow = max(0, YpFromYw(hpldr, idr,
			pwwd->ywMin + max(0,pwwd->rcePage.yeTop)));
	ypLimShow = min(dyl = pdr->dyl,
			ypLimWw = YpFromYw(hpldr, idr,
			min(pwwd->ywMac, pwwd->ywMin + pwwd->rcePage.yeBottom)));
	ypFirstInval = YpFromYw(hpldr, idr, rcwInval.ywTop);
	ypLimInval =  YpFromYw(hpldr, idr, rcwInval.ywBottom);
#ifdef MAC
	if (vsab.fExt && doc == docScrap)
		{
		/* drawing contents of external scrap */
		Assert(!fPageView);
		if (FDrawExtScrap())
			{
			(*hwwd)->fDirty = fFalse;
			Debug(vfCheckPlc = fCheckPlcSave);
			fReturn = fTrue;
			goto LRet;
			}
		goto LRestart;
		}
#endif
	Assert(!pdr->fCpBad);

/* calculate desired elevator position dq. */
	if (pwwd->fSetElev)
		{
		pwwd->fSetElev = fFalse;
		SetElevWw(ww);
		pwwd = *hwwd;
		}

	ypTop = - pdr->dypAbove;
	if (dlMac == 0)
		ypMacOld = 0;
	else
		{
		GetPlc(hplcedl, dlMac - 1, &edl);
		ypMacOld = min(dyl, edl.ypTop + edl.dyp);
		}
	dlNew = 0;
	if ((cp = pdr->cpFirst) == cpNil)
		goto LEndDr;
	if (pdr->fNoParaStart)
		{
		CachePara(doc, cp);
		if (cp == caPara.cpFirst)
			goto LEndDr;
		}
	if (fOutline)
		{
		pdod = PdodDoc(doc);
		if (pdod->fOutlineDirty)
			FUpdateHplcpad(doc);
		ClearCap();
		}

	for (dlOld = 0;;)
	/* we have: cp points to text desired on the coming line dlNew
	ypTop: desired position for top of dlNew
	dlOld: next line to be considered for re-use
*/
		{
		if (fOutline && udmod != udmodTable)
			cp = CpOutlineAdvance(doc, cp);
/* check for having to extend hplcedl array */
/* call ShakeHeap() here because we check if hplcedl needs to be enlarged
in this routine rather than always calling a routine which calls
ShakeHeap() for us. */
		Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
		if (dlNew >= (*hplcedl)->dlMax - 1)
			{
/* extend the array with uninitialized dl's, increment max, break if no space.
We assume that dlMac(Old) was <= dlMax, so the dl's will not be looked at
but used only to store new lines */
			PutIMacPlc(hplcedl, (*hplcedl)->dlMax - 1);
			if (!FOpenPlc(hplcedl, dlMacT = IMacPlc(hplcedl), 1/*cdlIncr*/))
				{
				dlk = dlkEndPage;
				SetErrorMat(matDisp);
				goto LEndDr;
				}
			InitPlcedl(hplcedl, dlMacT, 1);
			}
/* discard unusable dl's */
		for (; dlOld < dlMac; dlOld++)
			{
/* Set dlOld and edl to the next good dl */
			GetPlc(hplcedl, dlOld, &edl);

			Break1();
/* loop if: invalid or passed over in cp space or passed over in dl space,
passed over in yp space or in invalid band */
			cpT = CpPlc(hplcedl, dlOld);
			if (edl.fDirty || dlOld < dlNew
					|| cpT < cp
					|| edl.ypTop < ypTop
					|| (edl.ypTop + edl.dyp > ypFirstInval
					&& edl.ypTop < ypLimInval))
				{
				if (dlOld >= dlNew && edl.hpldr)
					FreeEdl(hplcedl, dlOld);
				continue;
				}
/* now we have dlOld, an acceptable if not necessarily useful dl.
now compute dlNew either from scratch or by re-using dlOld. To be
re-useable, dlOld must start at the right cp. */
			if (cpT == cp)
				{
/* Re-use this dl */
#ifdef BRYANL
				int xw = pdr->xwLimScroll;
#endif
				Win( pdr->xwLimScroll = max( pdr->xwLimScroll, 
						XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp )) );
#ifdef BRYANL
				if (xw != pdr->xwLimScroll)
					CommSzNum( SzShared( "DR xwLimScroll increased in FUpdateDr to: "), pdr->xwLimScroll );
#endif
/* if new place is not visible */
				if ((ypTop >= ypLimShow || ypTop + edl.dyp <= ypFirstShow) &&
						(edl.hpldr == hNil || !edl.fTableDirty))
					{
					if (dlNew != dlOld)
						{
						struct PLDR **hpldrT;

						PutCpPlc(hplcedl, dlNew, cp);
						PutPlc(hplcedl, dlNew, &edl);
						hpldrT = edl.hpldr;
						edl.hpldr = hNil;
						PutPlc(hplcedl, dlOld, &edl);
						edl.hpldr = hpldrT;
						}
					}
				else  if (dlOld != dlNew || edl.ypTop != ypTop)
					{
/* if old place is not completely visible below */
					if (edl.ypTop + edl.dyp > ypLimShow || !fScrollOK)
						continue;
					Assert(udmod != udmodNoDisplay);
					ScrollDrUp(ww, hpldr, idr, hplcedl, dlOld, dlNew, edl.ypTop, ypTop, ypFirstShow,
							ypLimShow);
					dlMac = IMacPlc(hplcedl);
					dlOld = dlNew;
					}
/* If the dl is for a complex object (i.e. table row), and its internal
/* structure has been dirtied, break out and use this dl to perform an
/* incremental update.
/**/
				if (edl.hpldr != hNil && edl.fTableDirty)
					{
					Assert(dlOld == dlNew);
					break;
					}

				if (fPVNTS && ypTop + edl.dyp > dyl && dlNew != 0)
					goto LEndDr;
				cp = cpT + edl.dcp;
				Assert(cp >= cp0);
				ypTop += edl.dyp;
				++dlOld;
				dlk = edl.dlk;
				fRMark |= edl.fRMark;
				goto LNextDlNew;
				}
			break;
			}
/* cpMin > cp, the line is not anywhere on the screen so either:
line is the max height end line that is new, or was moving up in
	the dr, so it was not scrolled
or line exists in sccBelow
or it will have to be formatted from scratch.
At any rate, advance cp and ypTop.
*/
		if (fPageView && ypTop > ypLimShow)
			{
/* line is completely below visible area. update depends on various params */
			if (udmod == udmodLastDr ||
					(cpUpd != cpNil && cp > cpUpd))
				{
				fIncomplete = fTrue;
				goto LEndDr;
				}
			}
LCheckCpMac:
		if (cp >= cpMac)
			{
			if (fPageView)
				{
				dlk = dlkEndPage;
				goto LEndDr;
				}
/* draw tall white block with the endmark in the upper left corner */
			emk = emkEndmark;
			edl.dlk = dlkEnd;
			edl.fDirty = fFalse;
			edl.dcp = 1;
			edl.dcpDepend = 0;
			edl.ypTop = ypTop;
			edl.dyp = 0x4000 - ypTop;
			edl.hpldr = hNil;

			Win( pdr->xwLimScroll = max( pdr->xwLimScroll, 
					XwFromXp( hpldr,
					idr, 
					(edl.xpLeft = 0) + (edl.dxp = vsci.dxpScrlBar) )) );

			PutCpPlc(hplcedl, dlNew, cp);
			PutPlc(hplcedl, dlNew, &edl);

			DrawEndmark(ww, hpldr, idr, ypTop, dyl, emk);
			cp++; 
			dlNew++; 
			ypTop = 0x4000;
			goto LEndDr;
			}
#ifdef MAC
		else  if (fCacheValid && cp >= vpsccBelow->cpFirst &&
				DlInScc(ww, vpsccBelow, cp, &pedl) >= 0 &&
				pedl->fHasBitmap)
/* line is in the cache with edl at pedl. if line fits, transfer it */
			DisplayFromScc(ww, hpldr, idr, dlNew, vpsccBelow,
					pedl, &cp, &ypTop);
#endif	/* MAC */
		else  if (doc < 0)
			goto LFSpec;
		else
			{
			Assert(cp >= cp0);
			CachePara(doc, cp);
			Debug(fInTable = FInTableVPapFetch(doc, cp));
			Assert ( udmod != udmodTable || fInTable );

			/* pdr->fInTable should be set iff udmod == udmodTable */
#ifdef DEBUG
			Assert (pdr->fInTable == (udmod == udmodTable));
#endif

			if (FInTableVPapFetch(doc, cp) && udmod != udmodTable)
				{
				if (fPageView && pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch))
					{
					if (pdr->lrk == lrkAbs)
						/* normal text can't flow into abs */
						goto LEndDr;
					/* skip abs object embedded in a non-abs table DR;
						it is described in a different DR */
					CpFirstTap(doc, cp);
					if (cp == pdr->cpFirst)
						pdr->cpFirst = caTap.cpLim;
					cp = caTap.cpLim;
					goto LCheckCpMac;
					}
				if (udmod == udmodNoDisplay)
					{
/* create fake vfli "describing" the whole table row */
					int dypT1, dypT2;

					vfli.dypLine = DysHeightTable(ww, doc, cp, fFalse, fFalse, &dypT1, &dypT2);
					vfli.cpMin = cp;
					CpFirstTap(doc, cp);
					vfli.cpMac = caTap.cpLim;
					vfli.doc = docNil;
					vfli.fSplats = fFalse;
					goto LGotFli;
					}
				if ( FUpdateTable(ww, doc, hpldr, idr, &cp, &ypTop,
						hplcedl, dlNew, dlOld, dlMac, ypFirstShow,
						ypLimWw, dyl, rcwInval, fScrollOK ) )
					{
#ifdef BRYANL
					int xw = pdr->xwLimScroll;
#endif
#ifdef WIN				
					GetPlc(hplcedl, dlNew, &edl);
					pdr->xwLimScroll = max( pdr->xwLimScroll,
							XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ) );
#endif
#ifdef BRYANL
					if (xw != pdr->xwLimScroll)
						CommSzNum( SzShared( "DR xwLimScroll increased in FUpdateDr (TABLE CASE) to: "),
								 pdr->xwLimScroll);
#endif

					if (dlOld == dlNew)
						dlOld++;
					if (ypTop <= 0)
						{
	/* this is a case where dypAbove was left larger than the height of the
	first line (which recently had its height reduced by some edit.) In such
	a case, we reset dypAbove and start again */
						pdr->dypAbove = 0;
			/*			Assert(dlNew == 0); */
						if (dlNew > 0)
							{
							Assert(!fPageView);
							pdr->cpFirst = CpPlc(hplcedl, dlNew);
							pwwd->fSetElev = fFalse;
							}
						goto LRestart;
						}

					goto LNextDlNew;
					}

				if (vmerr.fMemFail)
					{
					SetErrorMat(matDisp);
					pdr->fDirty = fFalse;
					fReturn = fFalse;
					FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);        /* Free the rest of the edls */
					PutIMacPlc(hplcedl, dlNew);
					PutCpPlc(hplcedl, dlNew, cp);
					goto LRet;
					}
				}
/* format and display the next line */
			if (fPageView && udmod != udmodTable &&
					(pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch)))
				{
				if (pdr->lrk == lrkAbs)
					/* normal text can't flow into abs */
					goto LEndDr;
				/* skip abs object embedded in a non-abs DR;
					it is described in a different DR */
				if (cp == pdr->cpFirst)
					pdr->cpFirst = caPara.cpLim;
				cp = caPara.cpLim;
				goto LCheckCpMac;
				}
			else  if (doc >= 0)
				{
				FormatLineDr(ww, cp, pdr);
				/* FormatLine can blow the caTap, etc. */
				if (udmod == udmodTable)
					CpFirstTap(doc, cp);
				if (vfli.ichMac == 0)
					{
					/* note: fParaStopped does not mean
						there are no characters on the 
						line */
					cp = vfli.cpMac;
					goto LCheckCpMac;
					}
				}
			else
				{
				CP cpFirstSave;
				/* dr is an fSpec character, -doc is the ch.
					need to set vcpFirstLayout so section
					props are obtained from mom */
LFSpec:                         
				Assert(fPageView);
				cpFirstSave = vcpFirstLayout;
				pmwd = PmwdWw(ww);
				docMother = pmwd->doc;
				pdod = PdodDoc(docMother);
				vcpFirstLayout = CpPlc(pdod->hplcpgd, (*hwwd)->ipgd);
				if (!FFormatLineFspec(ww, docMother, pdr->dxl, -doc))
					{
					vcpFirstLayout = cpFirstSave;
					goto LEndDr;
					}
				vcpFirstLayout = cpFirstSave;
				}
			if (fPageView)
				{
				if (dlNew == 0 && pdr->fSpaceBefore)
					ypTop -= vfli.dypBefore;
				/* have to subtract dypAfter because layout allows a 
					line when dypAfter is only reason to reject it */
LGotFli:                        
				if (fPVNTS && (ypTop + vfli.dypLine - vfli.dypAfter > dyl
						&& dlNew != 0) &&
						!((vfli.fSplatBreak || vfli.fSplatColumn) && vfli.ichMac == 1))
					goto LEndDr;
				ypTop += vfli.dypLine;
				}
			else
				{
				ypTop += vfli.dypLine;
				if (ypTop <= 0)
					{
/* this is a case where dypAbove was left larger than the height of the
first line (which recently had its height reduced by some edit.) In such
a case, we reset dypAbove and start again */
					pdr->dypAbove = 0;
					Assert(dlNew == 0);
					goto LRestart;
					}
				}
			cp = vfli.cpMac;
/* cp, ypTop have been advanced */
			if (dlOld < dlMac)
				{
				if (CpPlc(hplcedl, dlOld) == cp && fScrollOK)
					{
					int ddl = 0;
					GetPlc(hplcedl, dlOld, &edl);
	/* line at dlOld is a valid, existing line that will abutt the line just about
	to be displayed. */
					if (dlOld == dlNew)
	/* the line about to be overwritten will be re-used in the next time around
	in the loop. Hence, it is worthwhile to save this line and its dl */
						ddl = 1;
	/* else ddl = 0. Move the next line to its abutting position. We know that
	it has not yet been overwritten (yp > ypTop and dlOld > dlNew) */
					if (ddl != 0 || ypTop > edl.ypTop)
						{
						ypMacOld += ypTop - edl.ypTop;
						ScrollDrDown(ww, hpldr, idr, hplcedl,
								dlOld, dlOld + ddl,
								edl.ypTop,
								max(ypTop, edl.ypTop),
								ypFirstShow, ypLimWw, dyl);
						dlMac = IMacPlc(hplcedl);
						dlOld += ddl;
						}
					}
				if (dlOld == dlNew)
					FreeEdl(hplcedl, dlOld++);
				}

			fRMark |= vfli.fRMark;
			if (udmod == udmodTable ? ypTop <= ypLarge : ypTop - vfli.dypLine < dyl)
				{
				DisplayFli(udmod == udmodNoDisplay ? wwNil : ww,
						hpldr, idr, dlNew, ypTop);
				if (ypTop > dyl && (fPageView || udmod == udmodTable))
	/* means that line was admitted only because it fit w.o. dypAfter. But we do not
	want to reuse it, ever. In the udmodTable case, we may be clipping a partial
line because of abs row height setting */
					{
					/* grab the edl that DisplayFli put in, we may be
					/* interested in some of the info there.
					/**/
					GetPlc(hplcedl, dlNew, &edl);
					goto LSetDirty;
					}
				}
			else
	/* special case for lines for tables which would expand the table row.
	dl's are left dirty so that next scan can update them, could also be
	a partial line at the bottom of an abs height row, in which case we
	will need additional fields in the edl to be reasonable.
*/
				{
				PutCpPlc(hplcedl, dlNew, vfli.cpMin);
				edl.hpldr = hNil;
				edl.ypTop = ypTop - vfli.dypLine;
				edl.dyp = vfli.dypLine;
				edl.xpLeft = vfli.xpLeft;
				edl.dxp = vfli.xpRight - edl.xpLeft;
#ifdef FOO
				edl.dxp = ((vfli.fRight || vfli.fTop || vfli.fBottom) ?
						vfli.xpMarginRight + dxpLeftRightSpace +  DxFromBrc(vfli.brcRight,fFalse/*fFrameLines*/):
						vfli.xpRight) - (edl.xpLeft = vfli.xpLeft);
#endif
				edl.dcp = vfli.cpMac - vfli.cpMin;
				edl.dcpDepend = vfli.dcpDepend;
				edl.grpfEdl = 0;
/* NOTE - Anyone who jumps to this label had better have already
/* set the above fields in the EDL.
/**/
LSetDirty:
				edl.fDirty = fTrue;
				PutPlc(hplcedl, dlNew, &edl);
				}
			if (vfli.fSplatBreak)
				{
				pdod = PdodDoc(vfli.doc);
				if (fPageView && !pdod->fShort && pdr->lrk != lrkAbs ||
						udmod == udmodTable && vfli.chBreak == chTable)
					dlk = DlkFromVfli(hplcedl, dlNew);
				}
			}
/* check for yp overflow in galley mode and cpLim overflow in Pageview */
LNextDlNew:
		if (udmod == udmodTable && ypTop > ypLarge)
			/* Oops!  We are going to have too large a table cell */
			{
			CacheTc(wwNil, doc, cp, fFalse, fFalse);
			cp = vtcc.cpLim;
			ypTop -= edl.dyp;
			break;
			}
		dlNew++;
		if (dlk != dlkNil)
			break;
		if (udmod != udmodTable)
			{
			if (fPageView && cp >= cpLim)
				{
				if (!fLimSuspect || ypTop >= ypMacOld)
					break;
				pdr->cpLim = cpNil;
				cpLim = cpMac;
				if (cp >= cpMac)
					{
					dlk = dlkEndPage;
					break;
					}
				}
			if (ypTop > dyl)
				break;
			if (ypTop == dyl)
				{
				int ch;
				/* we have to allow a splat at the beginning
				   of a para to be tacked on even if no room */
				if (!fPageView)
					break;
				CachePara(doc, cp);
				if (cp != caPara.cpFirst)
					break;
				CacheSect(doc, cp);
				if (cp == caSect.cpLim - ccpSect ||
					(ch = ChFetch(doc, cp, fcmChars)) != chSect && ch != chColumnBreak)
					{
					break;
					}
				}
			}

		/* Before doing the next line, check for user abort. */
		/* Check only every other line, because it is so slow to check. */
		if (fAbortOK && (dlNew & 1) && FRareT(5, FMsgPresent(mtyUpdateWw)))
			goto LRetInval;

		}
/* at the end of the dr: clear remaining white space (if contents shrunk)

We have:
	cp: to be stored as limit for last dl
	dlNew: new dlMac
	Reason for ending dr is in dlk:
		End     end of document
		EndPage
		EndDr
		Nil     normal i.e. dl or yp full
*/
LEndDr:
	FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);        /* Free the rest of the edls */
	PutIMacPlc(hplcedl, dlNew);
	PutCpPlc(hplcedl, dlNew, cp);
	pdr->fIncomplete = fIncomplete;
	pdr->fRMark = fRMark && udmod == udmodTable;
	if (udmod != udmodNoDisplay)
		{
		int fFat;
		pdr->fDirty = fFalse;
		if (udmod != udmodTable)
			{
			fFat = pdr->idrFlow == idrNil && doc >= 0 && 
					((cp != cpNil && cp < pdr->cpLim) || ypTop > dyl);
			if (fPageView && (fFat || FDrawPageDrsWw(ww)))
				FrameDr( ww, hpldr, idr, fFat);
			}
		}
/* clear area at bottom of PageView display which stopped early.
We must do this even if NoDisplay mode is on, otherwise the screen state
will not be accurately reflected by the dr with respect to the trailing
white band.
*/
	if (ypTop < ypMacOld)
		DrawEndmark(ww, hpldr, idr, ypTop, ypMacOld, emkBlank);
	if (udmod == udmodTable)
/* save new height in dr. if container's height is exceeded, set dirty bit */
		{
		if (ypTop > pdr->dyl)
			pdr->fDirty = fTrue;
		pdr->dyl = ypTop;
		}
	vfEndPage = dlk == dlkEndPage;
	Debug(vfCheckPlc = fCheckPlcSave);
	fReturn = !vmerr.fMemFail;
	goto LRet;

/* before returning from an interrupt, invalidate lines that were
overwritten within the present update */
LRetInval:
	for (; dlOld < dlMac; dlOld++)
		{
		GetPlc(hplcedl, dlOld, &edl);
		if (edl.ypTop < ypTop)
			edl.fDirty = fTrue;
		else
			break;
		PutPlcLast(hplcedl, dlOld, &edl);
		}
	PutIMacPlc(hplcedl, dlMac = max(dlMac, dlNew));
	if (dlNew == dlMac)
		PutCpPlc(hplcedl, dlNew, cp);
	if (dlMac > 0)
		{
		GetPlc(hplcedl, dlMac - 1, &edl);
		if (ypTop > ypMacOld)
			edl.ypTop = ypTop - edl.dyp;

		edl.fDirty = fTrue;
		PutPlcLast(hplcedl, dlMac - 1, &edl);
		}

/* advance invalid band so that update can resume after an interruption */
	(*hwwd)->ywFirstInval = YwFromYp(hpldr, idr, ypTop);
	Debug(vfCheckPlc = fCheckPlcSave);
	fReturn = fFalse;
LRet:
	Mac(vfli.fStopAtPara = fFalse);
	FreePdrf(&drfFetch);
	return fReturn;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/* S C R O L L  D R  U P */
/* ypLimShow needs to define visibility limits, not necessarily dr limits */
#ifndef WIN
/* %%Function:ScrollDrUp %%Owner:tomsax */
NATIVE ScrollDrUp(ww, hpldr, idr, hplcedl, dlFrom, dlTo, ypFrom, ypTo, ypFirstShow, ypLimShow) /* WINIGNORE - MAC only */
#else /* WIN */
HANDNATIVE C_ScrollDrUp(ww, hpldr, idr, hplcedl, dlFrom, dlTo, ypFrom, ypTo, ypFirstShow, ypLimShow)
#endif /* WIN */
struct PLDR **hpldr;
struct PLCEDL **hplcedl;
{
	int ypLim, dypChange;
	int dl, dlMac;
	struct EDL edl;

	Assert(dlTo <= dlFrom);

	ypLim = ypFrom;
	dypChange = ypTo - ypFrom;
	dlMac = IMacPlc(hplcedl);
	FreeEdls(hplcedl, dlTo, dlFrom);
	PutIMacPlc(hplcedl, dlMac + dlTo - dlFrom);

	for (dl = dlFrom; dl < dlMac; dl++)
		{
		PutCpPlc(hplcedl, dlTo, CpPlc(hplcedl, dl));
		GetPlc(hplcedl, dl, &edl);

#ifdef WIN
		if (edl.fEnd)
			{
			int ypT = edl.ypTop - dypChange + 10 * vsci.dypBorder;

			if (ypT <= ypLimShow)
				{
/* endmark now showing is tall enough so that scrolling it will do all blanking */
#ifdef BRYANL
				CommSzNum( SzShared( "Scrolling endmark up, scroll height: "),ypT - edl.ypTop );
#endif
/* we can accomplish all necessary blanking by scrolling the endmark */
				ypLim = ypT;
				}
			else	
/* can't do any good by scrolling it, so dirty it */
				edl.fDirty = fTrue;
			}
		else
#endif
			if (edl.ypTop + edl.dyp <= ypLimShow)
				ypLim = edl.ypTop + edl.dyp;
			else  if (edl.ypTop + dypChange < ypLimShow)
				edl.fDirty = fTrue;
		edl.ypTop += dypChange;
		if (edl.hpldr != hNil)
			(*((struct PLDR **)edl.hpldr))->ptOrigin.yp = edl.ypTop;
		PutPlc(hplcedl, dlTo++, &edl);
		}
	PutCpPlc(hplcedl, dlTo, CpPlc(hplcedl, dl));
	Assert(ypLim >= ypFrom);
	Win(dypCS = dypChange);
	if (dypChange != 0)
		{
		if (ypTo < ypFirstShow)
			{
			ypFrom += ypFirstShow - ypTo;
			ypTo = ypFirstShow;
			}
		ScrollWw(ww, hpldr, idr, ypFrom, ypTo, ypLim - ypFrom);
		}
}


#endif /* DEBUGORNOTWIN */


/* F R A M E   D R   L I N E */
/* Draws the part of the DR frame for the current line at ywLine */
#ifdef MAC
NATIVE /* WINIGNORE - MAC only */
#else
EXPORT /* FrameDrLine */
#endif
/* %%Function:FrameDrLine %%Owner:tomsax */
FrameDrLine(ww, hpldr, idr, ywTop, ywBottom, fFat, fForceLine, fInnerDr)
int ww;
struct PLDR **hpldr;
int idr, ywTop, ywBottom;
BOOL fFat, fForceLine;
{
	struct RC rcw;
	int dyw;
	BOOL fFrameLines;
	struct DRF drf;
	struct DR *pdr;
	struct WWD *pwwd;
	struct DR *pdrT;

#ifdef WIN
	struct RC rcDraw, rcwClip;
	HBRUSH hbr, hbrT;
	HDC hdc = PwwdWw(ww)->hdc;
#else
	PENSTATE penState;
#endif

	pdr = PdrFetchAndFree(hpldr, idr, &drf);

	if (fInnerDr)
		{
#ifdef	DEBUG
		CpFirstTap(pdr->doc, pdr->cpFirst);
		Assert((*hpldr)->idrMax == vtapFetch.itcMac+1);
#endif
		if (idr == (*hpldr)->idrMax - 1)
			return;
		}

	DrclToRcw(hpldr, &pdr->drcl, &rcw);
	if (fInnerDr)
		{
		rcw.ywTop -= pdr->yl;
		if (pdr->fBottomTableFrame && !fFat)
			rcw.ywBottom = rcw.ywTop + (*hpldr)->dyl;
		rcw.xwLeft -= pdr->dxpOutLeft;
		rcw.xwRight += pdr->dxpOutRight;
		}
	else
		{
		rcw.xwLeft += dxFrameLine;
		rcw.xwRight -= dxFrameLine;
		}
#ifdef WIN
	if (PwwdWw(ww)->fPageView)
		RcwPgvTableClip(ww, hpldr, idr, &rcwClip);
	else
		rcwClip = PwwdWw(ww)->rcwDisp;
#endif

/* NOTE: to avoid drawing corner pixels twice,
/*       horizontal borders are drawn within "rcw",
/*       vertical borders are drawn just outside "rcw".
/**/

#ifdef	MAC
	GetPenState(&penState);
	PenNormal();
	PenPatGray(ww);
#endif	/* MAC */

	/* For this ^%$&*#* Native compiler */
	pwwd = PwwdWw(ww);
	if (fFrameLines = fInnerDr ? FDrawTableDrsWw(ww) : pwwd->fPageView && FDrawPageDrsWw(ww))
		{
		if ((dyw = ywBottom - ywTop) > 0 && !fInnerDr)
			{
			/* draw left and right lines */
#ifdef MAC
			MoveTo(rcw.xwLeft - dxFrameLine, ywTop );
			Line(0, dyw -= dyFrameLine);
			MoveTo(rcw.xwRight, ywTop );
			Line(0, dyw);
#else /* WIN */
			if (rcw.xwLeft > rcwClip.xwLeft)
				{
				SetRect(&rcDraw, rcw.xwLeft - vsci.dxpBorder, ywTop,
						rcw.xwLeft, ywTop + dyw);
				SectRc(&rcDraw, &rcwClip, &rcDraw);
				FillRect(hdc, (LPRECT) &rcDraw, vhbrGray);
				}
			if (rcw.xwRight >= rcwClip.xwLeft)
				{
				SetRect(&rcDraw, rcw.xwRight, ywTop,
						rcw.xwRight + vsci.dxpBorder, ywTop + dyw);
				SectRc(&rcDraw, &rcwClip, &rcDraw);
				FillRect(hdc, (LPRECT) &rcDraw, vhbrGray);
				}
#endif /* MAC/WIN */
			}
		if (ywTop <= rcw.ywTop)
			{
#ifdef MAC			
			if (!fForceLine)
				PenMode(patXor);
			MoveTo(rcw.xwLeft, rcw.ywTop);
			LineTo(rcw.xwRight - 1, rcw.ywTop);
#else /* WIN */
			SetRect(&rcDraw, rcw.xwLeft, ywTop,
					rcw.xwRight, ywTop + vsci.dypBorder);
			SectRc(&rcDraw, &rcwClip, &rcDraw);
			if (fForceLine)
				FillRect(hdc, (LPRECT) &rcDraw, vhbrGray);
			else
				{
				hbrT = SelectObject(hdc, vhbrGray);
				PatBltRc(hdc, &rcDraw, PATINVERT);
				if (hbrT != NULL)
					SelectObject(hdc, hbrT);
				}
#endif /* MAC/WIN */
			/* leave pen changed */
			}
		}

	/* NOTE - PenMode is left in unknown state */

	/* if we are asked to draw a fat line, then we're at the bottom of the DR */
	Assert ( !fFat || ywBottom >= rcw.ywBottom );

	/* we were called either to draw frame lines, or a fat line */
	Assert ( fFat || (fInnerDr ? FDrawTableDrsWw(ww) : FDrawPageDrsWw(ww)));

	if (ywBottom >= rcw.ywBottom && (!fInnerDr || fFat || pdr->fBottomTableFrame))
		{
		pdrT = PdrFetch(hpldr, idr, &drf);
		pdrT->fFatLine = fFat ? 1 : 0;
		FreePdrf(&drf);
		if (fFat)
			{
			if ((dyw = min(dywFatLine,ywBottom-ywTop)) <= 0
					|| !(fFrameLines))
				goto LRestorePen;
			if (fInnerDr)
				{
				struct WWD * pwwd;

				pwwd = PwwdWw(ww);	/* verbose for native compiler bug */
				if (!pwwd->fPageView)
					{
					pdr = PdrFetchAndFree((*hpldr)->hpldrBack, (*hpldr)->idrBack, &drf);
					if ((*hpldr)->ptOrigin.yp + (*hpldr)->dyl == pdr->dyl)
						goto LRestorePen;
					}
				}
#ifdef MAC
			PenSize(1, dyw);
			PenPat(&vqdg.patBlack);
#else /* WIN */
			hbr = vhbrWhite; /* not vhbrBlack, WIN's black is zero, white is 1 */
#endif
			}
		else  /* !fFat implies fDrawDrs */			
			{
			dyw = WinMac(vsci.dypBorder, 1);
			Win(hbr = vhbrGray);
			}
		Assert ( dyw >= 1 );
#ifdef MAC
		if (!fForceLine || fFat)
			PenMode(patXor);
		MoveTo(rcw.xwLeft, rcw.ywBottom - dyw);
		LineTo(rcw.xwRight - 1, rcw.ywBottom - dyw);
#else /* WIN */
		SetRect(&rcDraw, rcw.xwLeft, rcw.ywBottom - dyw, rcw.xwRight, rcw.ywBottom);
		SectRc(&rcDraw, &rcwClip, &rcDraw);
		if (!fForceLine || fFat)
			{
			hbrT = SelectObject(hdc, hbr);
			PatBltRc(hdc, &rcDraw, PATINVERT);
			if (hbrT != NULL)
				SelectObject(hdc, hbrT);
			}
		else
			FillRect(hdc, (LPRECT) &rcDraw, hbr);
#endif /* MAC/WIN */
		/* note: pen is left potentially altered since no more drawing occurs */
		}

LRestorePen:
	Mac(SetPenState(&penState));
	return;  /* for WIN so label has a line of code */
}


#ifdef MAC
/* P E N  P A T  G R A Y */
/* sets the pen to a gray pattern aligned for the WWD */
/* %%Function:PenPatGray %%Owner:NOTUSED */
NATIVE PenPatGray(ww) /* WINIGNORE - MAC only */
int	ww;
{
	struct WWD *pwwd;
	if (ww == wwNil)
		{
		/* we don't have to synch gray lines in dialog boxes */
		PenPat(&vqdg.patGray);
		return;
		}
	pwwd = PwwdWw(ww);
	PenPat((pwwd->xeScroll ^ pwwd->yeScroll) & 1 ? &vqdg.patGray : &patGray2);
}


#endif /* MAC */

#ifdef	MAC
/* %%Function:LpInitDisp2 %%Owner:NOTUSED */
/***************************/
native long LpInitDisp2(lpmh4) /* WINIGNORE - MAC only */
char far *lpmh4;
{
/* fixed segment initialization routine */
	return(0L);
}


/******
	WARNING: LpInitXxxx must be the last routine in this module
	because it is a fixed segment
******/
#endif	/* MAC */


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Disp2_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Disp2_Last() */
