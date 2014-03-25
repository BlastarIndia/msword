/* D I S P T B L . C */

#ifdef MAC
#include "toolbox.h"
#endif /* MAC */
#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "ch.h"
#include "inter.h"
#include "doc.h"
#include "format.h"
#include "scc.h"
#include "file.h"      
#include "layout.h"
#include "print.h"
#ifdef MAC
#include "iconbar.h"
#include "mac.h"
#else
#define REVMARKING
#include "compare.h"
#endif


/* E X T E R N A L S */

extern struct SCC       *vpsccBelow;
extern struct TAP       vtapFetch;
extern struct TCC	vtcc;
extern struct FLI       vfli;
extern struct FTI	vfti;
extern struct CA        caSect;
extern struct PAP       vpapFetch;
extern struct CHP       vchpFetch;
extern struct PREF      vpref;
extern struct PRSU      vprsu;
extern CP               vcpFetch;
extern int              vccpFetch;
extern struct CA        caPara;
extern struct CA        caTap;
extern struct CA		caParaL;
extern uns              vcbBmb;
extern uns              vcbBmbPerm;
extern int              ftcLast;
extern char             (**vhgrpchr)[];
extern int              vbchrMac;
extern int              vbchrMax;
extern int              vstcLast;
extern struct FLS       vfls;
extern int				vlm;

extern struct SCC       vsccAbove;

extern struct CA        caPage;
extern struct CA        caPage2;
extern int              vipgd;
extern int              vised;
extern CP               cpInsert;

extern struct MERR      vmerr;

extern struct SAB       vsab;
extern struct FMTSS	vfmtss;

extern struct ITR       vitr;

extern int              cHpFreeze;
extern int              vfShowAllStd;
extern int              vfLastScrollDown;

extern int              vcmcCur;
extern int              vihpldrMac;
extern struct PLDR		**vrghpldr[];

extern struct RC        rcScreen;
#ifdef WIN
extern int              vflm;
extern struct SCI       vsci;
extern struct TAP		vtapFetchAux;
extern struct CA		caTapAux;
extern HBRUSH		      vhbrGray;
#else
extern QDG		vqdg;
#endif

#ifdef DEBUG
extern struct DBS       vdbs;
extern int              vcTestx;
extern int              vfCheckPlc;

#ifdef WIN
int vfDypsInScreenUnits = fFalse;
Debug(extern struct PRI	vpri);
#endif /* WIN */

#endif /* DEBUG */

#ifdef MAC
NATIVE FrameTable(); /* DECLARATION ONLY */
NATIVE FrameEasyTable(); /* DECLARATION ONLY */
NATIVE FUpdTableDr(); /* DECLARATION ONLY */
#endif /* MAC */
NATIVE SetPenForBrc(); /* DECLARATION ONLY */

/*
/* F  U P D A T E  T A B L E
/*
/*  Description: Given the cp beginning a table row, format and display
/*    that table row. Returns fTrue iff the format was successful, with
/*    cp and ypTop correctly advanced. If format fails, cp and ypTop
/*    are left with their original values.
/**/
#ifdef DEBUGORNOTWIN
#ifdef MAC
NATIVE FUpdateTable ( ww, doc, hpldr, idr, pcp, pypTop, hplcedl, dlNew, dlOld, dlMac, /* WINIGNORE - MAC only */
ypFirstShow, ypLimWw, ypLimDr, rcwInval, fScrollOK )
#else /* !MAC */
/* %%Function:FUpdateTable %%Owner:tomsax %%Reviewed:7/6/89 */
HANDNATIVE C_FUpdateTable ( ww, doc, hpldr, idr, pcp, pypTop, hplcedl, dlNew, dlOld, dlMac,
ypFirstShow, ypLimWw, ypLimDr, rcwInval, fScrollOK )
#endif /* !MAC */
int ww, doc, idr, dlNew, dlOld, dlMac;
int ypFirstShow, ypLimWw, ypLimDr, fScrollOK;
struct PLCEDL **hplcedl;
struct PLDR **hpldr;
CP *pcp;
int *pypTop;
struct RC rcwInval;
{
	int idrTable, idrMacTable, itcMac;
	int dylOld, dylNew, dylDr, dylDrOld;
	int dylAbove, dylBelow, dylLimPldr, dylLimDr;
	int dylBottomFrameDirty;
	int dyaRowHeight, ypTop, ylT;
	Mac(int cbBmbSav;)
			int dlLast, dlMacOld, lrk;
	BOOL fIncr, fTtpDr, fReusePldr, fFrameLines, fOverflowDrs;
	BOOL fSomeDrDirty, fLastDlDirty, fLastRow, fFirstRow, fAbsHgtRow;
	BOOL fOutline, fPageView, fDrawBottom;
	Win(int fRMark;)
			CP cp;
	struct WWD *pwwd;
	struct DR *pdrT, *pdrTable;
	struct PLDR **hpldrTable, *ppldrTable;
	struct RC rcwTableInval;
	struct CA caTapCur;
	struct EDL edl, edlLast, edlNext;
	struct DRF drfT, drfFetch;
	struct TCX tcx;
	struct PAP papT;

	ypTop = *pypTop;	/* assume & assert *pypTop doesn't change until the end. */
	cp = *pcp;
	dylBottomFrameDirty = Win(fRMark =) 0;
	pwwd = PwwdWw(ww);
	fOutline = pwwd->fOutline;
	fPageView = pwwd->fPageView;

	CacheTc(ww,doc,cp,fFalse,fFalse);	/* Call this before CpFirstTap for efficiency */
	CpFirstTap1(doc, cp, fOutline);
	Assert(cp == caTap.cpFirst);
	caTapCur = caTap;
	itcMac = vtapFetch.itcMac;
	fAbsHgtRow = (dyaRowHeight = vtapFetch.dyaRowHeight) < 0;
	Assert ( FInCa(doc,cp,&caTapCur) );

	pdrT = PdrFetchAndFree(hpldr, idr, &drfT);
	lrk = pdrT->lrk;
	DrclToRcw(hpldr,&pdrT->drcl,&rcwTableInval);

	/* check to see if we need to force a first row or last row condition */
	fFirstRow = fLastRow = fFalse;	/* assume no override */
	if (fPageView)
		{
		if (pdrT->fForceFirstRow)
			{
			if (caTap.cpFirst == pdrT->cpFirst || dlNew == 0)
				fFirstRow = fTrue;
			else
				{
				Assert(dlNew > 0);
				dlLast = dlNew - 1;
				GetPlc(hplcedl,dlLast,&edlLast);
				if (caTapCur.cpFirst != CpPlc(hplcedl,dlLast) + edlLast.dcp)
					fFirstRow = fTrue;
				}
			}
		if (pdrT->cpLim != cpNil && pdrT->cpLim <= caTap.cpLim)
			{
			if (pdrT->idrFlow < 0)
				fLastRow = fTrue;
			else
				{
				/* use the pdrTable and drfFetch momentarily... */
				pdrTable = PdrFetchAndFree(hpldr, pdrT->idrFlow, &drfFetch);
				Assert(pdrT->doc == pdrTable->doc);
				fLastRow = pdrT->xl != pdrTable->xl;
				}
			}
		else
			{
			if (FInTableDocCp(doc,caTapCur.cpLim))
				{
				CachePara(doc, caTapCur.cpLim);
				papT = vpapFetch;
				CachePara(doc, cp);
				if (!FMatchAbs(caPara.doc, &papT, &vpapFetch))
					fLastRow = fTrue;
				}
			}
		}
	else  if (pwwd->fOutline)
		{
		if (!FShowOutline(doc, CpMax(caTap.cpFirst - 1, cp0)))
			fFirstRow = fTrue;
		if (!FShowOutline(doc, CpMin(caTap.cpLim, CpMacDocEdit(doc))))
			fLastRow = fTrue;
		}

	/* Rebuild the cache if we need to override. */
	if ((fFirstRow && !vtcc.fFirstRow) || (fLastRow && !vtcc.fLastRow))
		CacheTc(ww,doc,cp,fFirstRow,fLastRow);
	fFirstRow = vtcc.fFirstRow;
	fLastRow = vtcc.fLastRow;
	dylAbove = vtcc.dylAbove;
	dylBelow = vtcc.dylBelow;

	/* NOTE: The height available for an non-incremental update is extended
	/* past the bottom of the bounding DR so that the bottom border
	/* or frame line will not be shown if the row over flows.
	/**/
	fFrameLines = FDrawTableDrsWw(ww);
	dylLimPldr = pdrT->dyl - ypTop + dylBelow;
	if (dylBelow == 0 && fFrameLines && !fPageView)
		dylLimPldr += DyFromBrc(brcNone,fTrue/*fFrameLines*/);
	if (fAbsHgtRow)
		dylLimPldr = min(dylLimPldr,DypFromDya(-dyaRowHeight) + dylBelow);
	dylLimDr = dylLimPldr - dylAbove - dylBelow;

	/* Check for incremental update */
	Break3();
	if (dlOld == dlNew && dlOld < dlMac && cp == CpPlc(hplcedl,dlNew))
		{
		/* we are about to either use dlOld, or trash it. The next
		/* potentially useful dl is therefore dlOld+1
		/**/
		GetPlc ( hplcedl, dlNew, &edl );
		if ( edl.ypTop == ypTop && edl.hpldr != hNil )
			{
			Assert ( !edl.fDirty );
			hpldrTable = edl.hpldr;
			Assert((*hpldrTable)->idrMac == itcMac + 1);
			fIncr = fTrue;
			++dlOld;
			goto LUpdateTable;
			}
		}
	if (dlNew == dlOld && dlOld < dlMac) /* existing edl is useless, free it */
		FreeEdl(hplcedl, dlOld++);

	Break3();
	fIncr = fReusePldr = fFalse;

	/* new table row; init edl */
	if (vihpldrMac > 0)
		{
		hpldrTable = vrghpldr[--vihpldrMac];
		vrghpldr[vihpldrMac] = hNil;	/* we're gonna use or lose it */
		ppldrTable = *hpldrTable;
		if (ppldrTable->idrMax != itcMac+1 || !ppldrTable->fExternal)
			{
			/* If ppldrTable->idrMac == 0, LcbGrowZone freed up all of
			/* the far memory associated with this hpldr. We now need to
			/* free only the near memory. */
			if (ppldrTable->idrMax > 0)
				{
				FreeDrs(hpldrTable, 0);
				if ((*hpldrTable)->fExternal)
					FreeHq((*hpldrTable)->hqpldre);
				}
			FreeH(hpldrTable);
			hpldrTable = hNil;
			}
		else
			{
			Assert(ppldrTable->brgdr == offset(PLDR, rgdr));
			fReusePldr = fTrue;
			}
		}
	if (!fReusePldr)
		{
		hpldrTable = HplInit2(sizeof(struct DR),offset(PLDR, rgdr), itcMac+1, fTrue);
		Assert((*hpldrTable)->idrMac == 0);
		}

	edl.hpldr = hpldrTable;
	if (hpldrTable == hNil || vmerr.fMemFail)
		{
		SetErrorMat(matDisp);
		return fFalse; /* operation failed */
		}

	edl.ypTop = ypTop;
	/* this allows us to clobber (probably useless) dl's below */
	dylDrOld = dylLimDr;
	edl.dyp = dylLimPldr;
	edl.dcpDepend = 1;
	edl.dlk = dlkNil;
	Assert(FInCa(doc,cp,&vtcc.ca) && vtcc.itc == 0);
	edl.xpLeft = vtcc.xpCellLeft;
	PutCpPlc ( hplcedl, dlNew, cp );
	PutPlc ( hplcedl, dlNew, &edl );
	ppldrTable = *hpldrTable;
	ppldrTable->hpldrBack = hpldr;
	ppldrTable->idrBack = idr;
	ppldrTable->dyl = edl.dyp;
	ppldrTable->ptOrigin.yp = edl.ypTop;
	ppldrTable->ptOrigin.xp = 0;

LUpdateTable:
	Break3();
	fSomeDrDirty = fFalse;	/* for second pass */
	CpFirstTap1( doc, cp, fOutline );
	idrMacTable = fIncr ? itcMac + 1 : 0;
	dylNew = DypFromDya(abs(dyaRowHeight)) - dylAbove - dylBelow;
	dylOld = (*hpldrTable)->dyl;

	for ( idrTable = fTtpDr = 0; !fTtpDr; ++idrTable )
		{
		fTtpDr = idrTable == itcMac;
		Assert ( FInCa(doc,cp,&caTapCur) );
		if ( idrTable >= idrMacTable )
			{	/* this is a new cell in the table */
			if (fReusePldr)
				{
				pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
				Assert(drfFetch.pdrfUsed == 0);
				PutIMacPlc(drfFetch.dr.hplcedl, 0);
				}
			else
				{
				SetWords ( &drfFetch, 0, sizeof(struct DRF) / sizeof(int) );
				}

			/* NOTE We are assuming that the 'xl' coordiates in the table frame (PLDR)
			/* are the same as the 'xp' coordinates of the bounding DR.
			/**/
			ItcGetTcxCache(ww, doc, cp, &vtapFetch, idrTable/*itc*/, &tcx);
			drfFetch.dr.xl = tcx.xpDrLeft;
			drfFetch.dr.dxl = tcx.xpDrRight - drfFetch.dr.xl;
			drfFetch.dr.dxpOutLeft = tcx.dxpOutLeft;
			drfFetch.dr.dxpOutRight = tcx.dxpOutRight;

			drfFetch.dr.yl = fTtpDr ? max(dyFrameLine,dylAbove) : dylAbove;
			drfFetch.dr.dyl = edl.dyp - drfFetch.dr.yl - dylBelow;
			drfFetch.dr.cpLim = cpNil;
			drfFetch.dr.doc = doc;

			/* verbose for native compiler bug */
			drfFetch.dr.fDirty = fTrue;
			drfFetch.dr.fCpBad = fTrue;
			drfFetch.dr.fInTable = fTrue;
			drfFetch.dr.fForceWidth = fTrue;
			drfFetch.dr.fCantGrow = fPageView || fAbsHgtRow;

			drfFetch.dr.idrFlow = idrNil;
			drfFetch.dr.lrk = lrk;

			drfFetch.dr.fBottomTableFrame = fFalse; /* we'll set this correctly later */
			if (!fReusePldr)
				{
				if (!FInsertInPl ( hpldrTable, idrTable, &drfFetch.dr )
						|| !FInitHplcedl(1, cpMax, hpldrTable, idrTable))
					{
LAbort:
					FreeDrs(hpldrTable, 0);
					FreeHpl(hpldrTable);
					edl.hpldr = hNil;
					PutPlc(hplcedl, dlNew, &edl);
					return fFalse; /* operation failed */
					}
				pdrTable = PdrFetch(hpldrTable,idrTable,&drfFetch);
				}

			++idrMacTable;
			Assert (fReusePldr || (*hpldrTable)->idrMac == idrMacTable );
			}
		else
			pdrTable = PdrFetch(hpldrTable,idrTable,&drfFetch);

		if (pdrTable->fCpBad || pdrTable->cpFirst != cp)
			{
			pdrTable->cpFirst = cp;
			pdrTable->fCpBad = fFalse;
			pdrTable->fDirty = fTrue;
			}
		if (pdrTable->yl != (ylT = fTtpDr ? max(dylAbove,dyFrameLine) : dylAbove))
			{
			pdrTable->yl = ylT;
			FreeEdls(pdrTable->hplcedl,0,IMacPlc(pdrTable->hplcedl));
			PutIMacPlc(pdrTable->hplcedl,0);
			pdrTable->fDirty = fTrue;
			}
		else  if (pdrTable->fBottomTableFrame != fLastRow || pdrTable->fDirty)
			{
			if ((dlLast = IMacPlc(pdrTable->hplcedl) - 1) >= 0)
				{
				GetPlc(pdrTable->hplcedl,dlLast,&edlLast);
				edlLast.fDirty = fTrue;
				PutPlcLast(pdrTable->hplcedl,dlLast,&edlLast);
				}
			pdrTable->fDirty = fTrue;
			}

		if (fTtpDr)
			{
			if (fAbsHgtRow)
				dylNew = DypFromDya(abs(dyaRowHeight)) + dylBelow;
			else
				dylNew += dylAbove + dylBelow;
			pdrTable->dyl = min(pdrTable->dyl, dylNew - pdrTable->yl - dylBelow);
			}
		Assert ( pdrTable->cpFirst == cp );

		if (pdrTable->fDirty)
			{
			/* set DR to full row height */
			if (fIncr)
				{
				/* we're betting that the row height doesn't change.
				/* record some info to check our bet and take corrective
				/* measures if we lose.
				/**/
				dlMacOld = IMacPlc(pdrTable->hplcedl);
				dylDrOld = pdrTable->dyl;
				pdrTable->fCantGrow = pdrTable->dyl >= dylLimDr && (fPageView || fAbsHgtRow);
				pdrTable->dyl = edl.dyp - pdrTable->yl - dylBelow;
				}
			/* enable drawing the bottom table frame */
			pdrTable->fBottomTableFrame = fLastRow;
			/* write any changes we have made, native code will need them */
			pdrTable = PdrFreeAndFetch(hpldrTable,idrTable,&drfFetch);
			if (!FUpdTableDr(ww,hpldrTable,idrTable))
				if (!FUpdateDr(ww,hpldrTable,idrTable,rcwInval,fFalse,udmodTable,cpNil))
					{
LFreeAndAbort:
					FreePdrf(&drfFetch);
					goto LAbort;
					}
#ifdef ENABLE
/* FUTURE: this is bogus for windows, since DypHeightTc returns the height
/* in printer units, not screen units. Further, it is questionable to call
/* DypHeightTc without setting up vlm and vflm.
/*
/* Enable this next line to check that FUpdate table yields the same height
/* as does DypHeightTc using the FormatLine and PHE's, slows down redrawing.
/**/
#ifdef DEBUG
			CacheTc(ww, doc, pdrTable->cpFirst, fFirstRow, fLastRow);
			Assert ( DypHeightTc(ww,doc,pdrTable->cpFirst) == pdrTable->dyl );
#endif
#endif
			/* check for height change */
			pdrTable->fCantGrow = pdrTable->dyl >= dylLimDr && (fPageView || fAbsHgtRow);
			if (pdrTable->dyl != dylDrOld)
				{
				if (fIncr && fLastRow && dylDrOld == dylOld - dylAbove - dylBelow
						&& IMacPlc(pdrTable->hplcedl) >= dlMacOld)
					{
					/* we probably have a dl that has a frame line in the
					/* middle of a DR.
					/**/
					pdrTable->fDirty = fTrue;
					GetPlc(pdrTable->hplcedl,dlMacOld-1,&edlLast);
					edlLast.fDirty = fTrue;
					PutPlcLast(pdrTable->hplcedl,dlMacOld-1,&edlLast);
					}
				}

			fSomeDrDirty |= pdrTable->fDirty && !fTtpDr;
			}

		if (!fTtpDr)
			dylNew = max(dylNew, pdrTable->dyl);

		Win(fRMark |= pdrTable->fRMark;)

				/* advance the cp */
		cp = CpMacPlc ( pdrTable->hplcedl );
		FreePdrf(&drfFetch);

		} /* end of for idrTable loop */

	idrMacTable = (*hpldrTable)->idrMac = itcMac + 1;

	/* adjust the TTP DR to the correct height, don't want it to dangle
	/* below the PLDR/outer EDL
	/**/
	pdrTable = PdrFetch(hpldrTable, itcMac, &drfFetch);
	if (pdrTable->yl + pdrTable->dyl + dylBelow > dylNew)
		{
		pdrTable->dyl = dylNew - pdrTable->yl - dylBelow;
		Assert(IMacPlc(pdrTable->hplcedl) == 1);
		if (dylOld >= dylNew)
			{
			GetPlc(pdrTable->hplcedl, 0, &edlLast);
			edlLast.fDirty = fFalse;
			PutPlcLast(pdrTable->hplcedl, 0, &edlLast);
			pdrTable->fDirty = fFalse;
			}
		}
	FreePdrf(&drfFetch);

	Assert ( cp == caTap.cpLim );

	/* At this point, we have scanned the row of the table,
	/* found the fTtp para, and updated the cell's DR's up
	/* to the height of the old EDL. We know
	/* the number of cells/DRs in the row of the table. We now
	/* adjust the height of the mother EDL to the correct row height.
	/**/

	if (fAbsHgtRow)
		{
		dylNew -= dylAbove + dylBelow;
		for ( idrTable = itcMac; idrTable--; FreePdrf(&drfFetch))
			{
			/* this depends on the theory that we never need a second
			/* pass for an absolute height row
			/**/
			pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
			if (pdrTable->dyl >= dylNew)
				{
				pdrTable->fCantGrow = fTrue;
				pdrTable->dyl = dylNew;
				}
			else
				pdrTable->fCantGrow = fFalse;
			pdrTable->fDirty = fFalse;
			}
		dylNew += dylAbove + dylBelow;
		}

	/* update hpldr and edl to correct height now for second scan,
		old height still in dylOld */
	(*hpldrTable)->dyl = edl.dyp = dylNew;

	if (dylNew == dylOld || (!fSomeDrDirty && !fLastRow))
		goto LFinishTable;

	if (fOverflowDrs = (fPageView && dylNew > dylLimPldr - dylBelow))
		{
		pdrT = PdrFetchAndFree(hpldr,idr,&drfT);
		if ((dylNew < pdrT->dyl || pdrT->lrk == lrkAbs) && !pdrT->fCantGrow)
			/* force a repagination next time through */
			{
			/* to avoid infinite loops, we won't repeat if the
				DRs are brand new */
			pwwd = PwwdWw(ww);	/* verbose for native compiler bug */
			pwwd->fDrDirty = !pwwd->fNewDrs;
			}
		for ( idrTable = itcMac; idrTable--; FreePdrf(&drfFetch))
			{
			pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
			if (pdrTable->dyl > dylLimDr)
				pdrTable->dyl = dylLimDr;
			}
		/* doctor dylNew and fix earlier mistake */
		dylNew = dylLimPldr;
		(*hpldrTable)->dyl = edl.dyp = dylNew;
		}

	if (fLastRow && fFrameLines)
		{
/* Here we decide what cells have to be drawn because of the bottom frame
/* line. State: All cells are updated to the original height of the edl,
/* the last EDL of every cell that was drawn in the first update pass
/* has been set dirty (even if the fDirty bit for the DR is not set)
/* and does not have a bottom frame line.
/**/
		pwwd = PwwdWw(ww);
		fDrawBottom = dylBelow == 0 && YwFromYl(hpldrTable,dylNew) <= pwwd->rcwDisp.ywBottom;

		Mac(cbBmbSav = vcbBmb);
		fLastDlDirty = fFalse;
		for ( idrTable = 0; idrTable < itcMac/*skip TTP*/; FreePdrf(&drfFetch),++idrTable )
			{
			pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
			if (pdrTable->fDirty)
				{
				/* If the DR is dirty, there is little chance of a frame line
				/* problem. The test below catches the possible cases.
				/**/
				if (pdrTable->yl + pdrTable->dyl >= dylOld && dylOld < dylNew)
					dylBottomFrameDirty = 1;
				continue;
				}

			dylDr = pdrTable->yl + pdrTable->dyl;
			GetPlc ( pdrTable->hplcedl, dlLast = IMacPlc(pdrTable->hplcedl) - 1, &edlLast );
			/* does the last dl of the cell lack a needed bottom frame ? */
			if ( (dylDr == dylNew && dylNew < dylOld && fDrawBottom)
					/* OR does the last dl have an unneeded bottom frame? */
			|| (dylDr == dylOld && dylOld < dylNew ) )
				{
				/* verbose for native compiler bug */
				fLastDlDirty = fTrue;
				edlLast.fDirty = fTrue;
				pdrTable->fDirty = fTrue;
				PutPlcLast ( pdrTable->hplcedl, dlLast, &edlLast );
				}

			} /* end of for pdrTable loop */

		/* If we are doing the second update scan only to redraw the last
		/* rows with the bottom border, then have DisplayFli use an
		/* off-screen buffer to avoid flickering.
		/**/
		if ( fLastDlDirty && !fSomeDrDirty )
			{
			Mac(vcbBmb = vcbBmbPerm);
			fSomeDrDirty = fTrue;
			}
		}

	/* If some DRs were not fully updated, finish updating them */
	if (fSomeDrDirty)
		{
		Break3();

		/* If there's a potentially useful dl that's about to get
		/* blitzed, move it down some.
		/**/
		if (dylNew > dylOld && dlOld < dlMac && dlNew < dlOld && fScrollOK)
			{
			GetPlc ( hplcedl, dlOld, &edlNext );
			if (edlNext.ypTop < ypTop + dylNew)
				ScrollDrDown ( ww, hpldr, idr, hplcedl, dlOld,
						dlOld, edlNext.ypTop, ypTop + dylNew,
						ypFirstShow, ypLimWw, ypLimDr);
			dlMac = IMacPlc ( hplcedl );
			}

		rcwTableInval.ywTop += ypTop + min(dylNew,dylOld) - dylBottomFrameDirty;
		rcwTableInval.ywBottom += ypTop + max(dylNew,dylOld);

		DrawEndmark ( ww, hpldr, idr, ypTop + edl.dyp, ypTop + dylNew, emkBlank );

		for ( idrTable = 0; idrTable < idrMacTable; ++idrTable )
			{
			pdrTable = PdrFetch(hpldrTable,idrTable,&drfFetch);
			if ( pdrTable->fDirty )
				{
				/* Since we have already filled in the PLCEDL for the DR,
				/* the chance of this failing seems pretty small. It won't
				/* hurt to check, however.
				/**/
				if (fIncr || !FUpdTableDr(ww,hpldrTable,idrTable))
					if (!FUpdateDr(ww,hpldrTable,idrTable,rcwTableInval,fFalse,udmodTable,cpNil))
						goto LFreeAndAbort;
#ifdef ENABLE
/* Enable this next line to check that FUpdate table yields the same height
/* as does DypHeightTc using the FormatLine and PHE's; slows down redrawing.
/**/
#ifdef DEBUG
				CacheTc(ww, doc, pdrTable->cpFirst, fFirstRow, fLastRow);
				Assert ( DypHeightTc(ww,doc,pdrTable->cpFirst) == pdrTable->dyl );
#endif
#endif
				if (fOverflowDrs && pdrTable->dyl > dylLimDr)
					pdrTable->dyl = dylLimDr;
				}
			Win(fRMark |= pdrTable->fRMark;)
					FreePdrf(&drfFetch);
			}
#ifdef	MAC
		if ( fLastRow )
			Mac(vcbBmb = cbBmbSav);
#endif	/*MAC*/
		} /* end of if fSomeDrDirty */

LFinishTable:
	/* Now, clear out bits in edl not already drawn and draw cell
	/* borders.
	/**/
	Assert(edl.dyp == dylNew);
	FrameTable(ww,doc,caTapCur.cpFirst,hpldrTable, &edl, fFirstRow, fLastRow );

	/* update edl and pldrT to reflect what we have done */
	edl.fDirty = edl.fTableDirty = fFalse;
	edl.dcp = cp - CpPlc(hplcedl,dlNew);

	/* because of the way borders and frame lines are drawn,
	/* a table row always depends on the next character
	/**/
	edl.dcpDepend = 1;

	PutPlc ( hplcedl, dlNew, &edl );

#ifdef WIN
	if (fRMark)
		DrawTableRevBar(ww, idr, dlNew);

/* draw the style name area and border for a table row */
	if (PwwdWw(ww)->xwSelBar)
		DrawStyNameFromWwDl(ww, hpldr, idr, dlNew);
#endif

	/* advance the caller's cp and ypTop */
	*pcp = cp;
	Assert(ypTop == *pypTop);
	*pypTop += dylNew;
	return fTrue; /* it worked! */
}


#endif /*DEBUGORNOTWIN*/

/*	F  U P D  T A B L E  D R
/*
/*  Description: Attempt to handle the most common table dr update
/*  in record time by simply calling FormatLine and DisplayFli.
/*  If simple checks show we have updated the entire DR, return
/*  fTrue, else record the EDL we wrote and return fFalse so that
/*  FUpdateDr() gets called to finish the job.
/**/
#ifdef DEBUGORNOTWIN
#ifdef MAC
NATIVE FUpdTableDr(ww, hpldr, idr) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:FUpdTableDr %%Owner:tomsax %%Reviewed:7/6/89 */
HANDNATIVE C_FUpdTableDr(ww, hpldr, idr)
#endif /* !MAC */
int ww;
struct PLDR **hpldr;
int idr;
{
	BOOL fSuccess, fOverflow;
	struct PLC **hplcedl;
	struct DR *pdr;
	struct WWD *pwwd;
	struct EDL edl;
	struct DRF drf;

	pdr = PdrFetch(hpldr, idr, &drf);
	fSuccess = fFalse;

	hplcedl = pdr->hplcedl;
	Assert(hplcedl != hNil && (*hplcedl)->iMax > 1);
	if (IMacPlc(hplcedl) > 0)
		{
		GetPlc(hplcedl, 0, &edl);
		Assert(edl.hpldr == hNil);	/* no need to FreeEdl */
		if (!edl.fDirty)
			goto LRet;
		}

	FormatLineDr(ww, pdr->cpFirst, pdr);

	/* cache can be blown by FormatLine */
	CpFirstTap(pdr->doc, pdr->cpFirst);

	pwwd = PwwdWw(ww);	/* verbose for native compiler bug */
	fOverflow = vfli.dypLine > pdr->dyl;
	if (fSuccess = (vfli.fSplatBreak && vfli.chBreak == chTable
			&& (!fOverflow || idr == vtapFetch.itcMac || YwFromYp(hpldr,idr,pdr->dyl) >= pwwd->rcwDisp.ywBottom)))
		{
		DisplayFli(ww, hpldr, idr, 0, vfli.dypLine);

		pdr->dyl = vfli.dypLine;
		pdr->fDirty = fOverflow;
		pdr->fRMark = vfli.fRMark;

		DlkFromVfli(hplcedl, 0);
		PutCpPlc(hplcedl, 1, vfli.cpMac);
		PutIMacPlc(hplcedl, 1);
		}

LRet:
	FreePdrf(&drf);
	return fSuccess;
}
#endif /*DEBUGORNOTWIN*/


/* D Y P  H E I G H T  T C
/*
/* Description: Find the height of the cell containing the given cp; height
/* does not include space for borders.
/**/
/* NOTE - TC must already be cached
/* WIN NOTE - vflm must be set to flmRepaginate or flmPrint so that height
/* calculations take place in printer units.
/**/
/* %%Function:DypHeightTc %%Owner:tomsax %%Reviewed:7/6/89 */
NATIVE DypHeightTc ( ww, doc, cp )
int ww, doc;
CP cp;
{
	struct PHE phe;
	CP cpLim;
	int dxa, dyaRowHeight, dxl, dyp = 0, dypT, fLayout;
	int clLim;
	int fGetStorePhes;
	struct LNH lnh;
	struct LBS lbs;
	struct WWD *pwwd;

	Assert(FInTableDocCp(doc, cp));
	Assert(FInCa(doc, cp, &caTap));
	Assert(FInCa(doc, cp, &vtcc.ca));
	Assert(vtcc.fXpValid && vtcc.fDylValid);
	Debug(CheckFlmState());

	dxl = vtcc.xpDrRight - vtcc.xpDrLeft;
	dxa = DxaFromDxp ( PwwdWw(ww), dxl );
	if ((dyaRowHeight = vtapFetch.dyaRowHeight) < 0)
		return DypFromDya(-dyaRowHeight - vtcc.dylAbove);

	SetWords(&lbs, 0, cwLBS);
	lbs.ww = ww;
	lbs.doc = doc;

	/* verbose for nightmarish native compiler from hell */
	pwwd = PwwdWw(ww);
	lbs.fOutline = pwwd->fOutline;
	fGetStorePhes = ((vflm == flmRepaginate || vflm == flmPrint) && !lbs.fOutline);
	/* save and restore...ClFormatLines turns it off! */
	fLayout = vfli.fLayout;

	for (lbs.cp = vtcc.cpFirst, cpLim = vtcc.cpLim; lbs.cp < cpLim; )
		{
		dypT = dyp;
		/* CacheParaL sets up caParaL, caPara, and vpapFetch. These latter
		/* two are munged to make them palatable to the layout routines.
		/**/
		CacheParaL(doc, lbs.cp, lbs.fOutline);
		if (fGetStorePhes && FGetValidPhe(doc, lbs.cp, &phe) && dxa == phe.dxaCol)
			{
			if (phe.fDiffLines)
				dyp += WinMac(YlFromYa(phe.dyaHeight), phe.dylHeight);
			else  if (phe.dylLine == 0)
				goto LCalcHeight;
			else
				dyp += phe.clMac * WinMac(YlFromYa(phe.dyaLine),phe.dylLine) +
						YlFromYa(vpapFetch.dyaBefore) +
						YlFromYa(vpapFetch.dyaAfter);
			Debug(vfls.ca.doc = docNil);/* so we know we were here */
			lbs.cp = caParaL.cpLim;
			}
		else
			{
LCalcHeight:
			/* CacheParaL sets up caParaL, caPara, and vpapFetch. These latter
			/* two are munged to make them palatable to the layout routines.
			/**/
			CacheParaL(doc, lbs.cp, lbs.fOutline);
			/* calculate height of table paragraph from scratch */
			clLim = ClFormatLines(&lbs, caParaL.cpLim, ypLarge, ypLarge,
					clMax, dxl, fFalse/*fRefLine*/, fFalse/*fStopAtPageBreak*/);
			if (clLim != 0)
				{
				GetPlc(vfls.hplclnh, clLim - 1, &lnh);
				dyp += lnh.yll;
				if (lnh.yll > ypLarge)
					dyp = ypLarge;
				if (!vfls.fVolatile && fGetStorePhes)
					/* OK if this fails - we just calculate it again */
					FSavePhe(&vfls.phe);
				}
			else  if (vpapFetch.fTtp)
				{
				/* trailer all alone */
				FormatLine(ww, doc, lbs.cp);
				dyp += vfli.dypLine;
				}
			lbs.cp = vfls.ca.cpLim;
			}
		/* Check to make sure we stay small */
		if (dyp >= ypLarge)
			{
			caPara.doc = docNil; /* because of munging by CacheParaL */
			dyp = ypLarge - 1;
			goto LRet;
			}
		}
	if (dyaRowHeight > 0)
		dyp = max(dyp, DypFromDya(dyaRowHeight));
	caPara.doc = docNil; /* because of munging by CacheParaL */
LRet:
	vfli.fLayout = fLayout;
	return dyp;
}


/* Like DypHeightTable, but returns screen units rather than printer units */
/* %%Function:DysHeightTable %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT DysHeightTable ( ww, doc, cp, fForceTop, fForceBottom, pdypAbove, pdypBelow )
int ww, doc;
CP cp;
int fForceTop, fForceBottom, *pdypAbove, *pdypBelow;
{
	int dyp;
	int docMother; 

#ifdef WIN
	vfls.ca.doc = docNil;
	Debug(vfDypsInScreenUnits = fTrue);
/* need to make wwLayout equivalent to passed ww so that CacheParaL calls 
   within the scope of FResetWwLayout work properly. */
	docMother = DocMother(doc);
 	FResetWwLayout(docMother, vflm, vlm);

#endif /* WIN */
	dyp = DypHeightTable(ww, doc, cp, fForceTop, fForceBottom, pdypAbove, pdypBelow);

#ifdef	WIN
	Debug(vfDypsInScreenUnits = fFalse);
	
	vfls.ca.doc = docNil;
#endif /* WIN */

	return dyp;
}


EXPORT /* DypHeightTable */
/* %%Function:DypHeightTable %%Owner:tomsax %%Reviewed:7/6/89 */
DypHeightTable ( ww, doc, cp, fForceTop, fForceBottom, pdypAbove, pdypBelow )
int ww, doc;
CP cp;
int fForceTop, fForceBottom, *pdypAbove, *pdypBelow;
{
	int dypText;

	AssertDo(FTableHeight ( ww, doc, cp, fFalse/*fAbortOK*/, fForceTop, fForceBottom, &dypText, pdypAbove, pdypBelow));
	return dypText;
}


/* %%Function:FTableHeight %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT BOOL
FTableHeight ( ww, doc, cp, fAbortOK, fForceTop, fForceBottom, pdypText, pdypAbove, pdypBelow)
int ww, doc;
CP cp;
int fAbortOK, fForceTop, fForceBottom, *pdypText, *pdypAbove, *pdypBelow;
{
	int dypText;
	int itcMac;
	Win(int lmSav;)
			int flmSav;
	BOOL fMsgPresent;
	CP cpLim;

	Assert(FInTableDocCp(doc, cp));
	Debug(CheckFlmState());

	CacheTc(ww,doc,cp,fForceTop,fForceBottom);
	cp = CpFirstTap(doc,cp);

	itcMac = vtapFetch.itcMac;
	if (vtcc.itc != 0)
		CacheTc(ww,doc,cp,fForceTop,fForceBottom);

	*pdypAbove = vtcc.dylAbove;
	*pdypBelow = vtcc.dylBelow;
	dypText = DypFromDya(vtapFetch.dyaRowHeight);
	if (dypText < 0)
		{
		*pdypText = -dypText - vtcc.dylAbove;
		return fTrue;
		}

#ifdef	WIN
	flmSav = vflm;
	lmSav = vlm;
#endif	/*WIN*/
	while (vtcc.itc < itcMac)
		{
		cpLim = vtcc.cpLim; /* DypHeightTc can cause vtcc to change */
		dypText = max(dypText,DypHeightTc(ww,doc,cp));
		if (fAbortOK)
			{
			fMsgPresent = FMsgPresent(mtyIdle);
			Assert(vlm == lmSav);
#ifdef	WIN
			Assert(!vfDypsInScreenUnits);
			if (vflm != flmSav)
				FResetWwLayout(DocMother(doc), flmSav, lmSav);
#endif	/*WIN*/
			if (fMsgPresent)
				return fFalse;
			}
		cp = cpLim;
		CacheTc(ww,doc,cp,fForceTop,fForceBottom);
		}
	*pdypText = dypText;
	return fTrue;
}


/* 	S C A N  T A B L E  R O W   */
/* scans a table containing cp in doc; caches each tc in the table and
	passes non-compressed (non-empty) cells to the supplied routine. That
	routine may assume that vtcc is correct */
/* NOTE for grep, possible values for pfn: DisplayLrTable and DisplayTcLines */
/* %%Function:ScanTableRow %%Owner:tomsax %%Reviewed:7/6/89 */
ScanTableRow ( ww, doc, cp, pfn, p1, p2 )
int ww, doc;
CP cp;
int (*pfn)();
int p1, p2;
{
	CP cpLim;

	Assert(FInTableDocCp(doc, cp));

	cp = CpFirstTap(doc,cp);
	cpLim = caTap.cpLim;

	while ( cp < cpLim )
		{
		CacheTc(ww, doc, cp, fFalse, fFalse);
		cp = vtcc.cpLim;
		if (vtcc.xpDrLeft != vtcc.xpDrRight)
			(*pfn)(ww, p1, p2);
		}

}


/* D R A W  B R C
/* Description: Given sufficient information, draw a border (or a frame line)
/* between cells in a table.
/*
/* Game Plan: Divide up the rcw which we have to draw into strips running
/* parallel to the "line". Assign to each strip a color/pattern. Then
/* loop through the strips, painting them as required. This includes clearing
/* those bits that are to be white. Under this scheme, every bit gets drawn
/* precisely once.
/*
/* If dbmod is dbmodHoriz, then rgbrc contains information about the lines
/* which butt up against the left end of the current line. If dbmodHoriz,
/* then rgbrc[0] contians the current code; the others are undefined.
/*
/*
/*						|     |
/*						|     |
/*						| rgbrc[ibrcTopButt] 
/*						|     |
/*						|     |
/* -------------------------------------------------------------
/*	rgbrc[ibrcLeftButt]	|    rgbrc[ibrcDraw] (draw this one)		|
/* -------------------------------------------------------------
/*						|     |
/*						|     |
/*						| rgbrc[ibrcBottomButt] 
/*						|     |
/*						|     |
/*
/*
/*
/*
/**/
/* Opus version is in dispspec.c */
#ifdef MAC
/* %%Function:DrawBrc %%Owner:NOTUSED */
DrawBrc ( ww, rgbrc, rcw, dbmod, fFrameLines )
int rgbrc[];
struct RC rcw;
BOOL fFrameLines;
{
	int bl, blMac;
	int brcCur, brcButt;
	int bitStart, bitCur, rgbitMatch;
	int brcMatch, cbrcMatch;
	int ibrc, ibrcT;
	int fLeft, fRight, fTop, fBottom;
	int dz, dzT;
	int cbrcT, rgbitT, brcT;
	int xw1, xw2, yw1, yw2;
	int dxwBottomButt, dxwWidth;
	BOOL fVertical, fEraseBits, fLaserWriter, fHairline;
	struct RC rcwCorner;
	struct BRK brk, brkCorner;
	int rgbrcT[cbrcCorner];
	int mpblz[blMax+1];
	int mpblblp[blMax+1];

	if (FEmptyRc(&rcw))
		return;

	fLaserWriter = vlm == lmPrint && vprsu.prid == pridLaser;
	if ( !(fEraseBits = vlm != lmPrint && vlm != lmPreview) )
		fFrameLines = fFalse;
	fVertical = (dbmod & dbmodHV) == dbmodVertical;

	/* initialize things */
	PenNormal();
	brcCur = rgbrc[ibrcDraw];

	/* decode the brc info */
	BrkFromBrc ( brcCur, &brk, fFrameLines, dbmod & dbmodReverse );

	/* copy the endpoints of the strip (width direction) into array */
	if (fVertical)
		{
		mpblz[0] = rcw.xwLeft;
		mpblz[brk.blMac+1] = rcw.xwRight;
		}
	else  /* dbmod == dbmodHoriz */		
		{
		mpblz[0] = rcw.ywTop;
		mpblz[brk.blMac+1] = rcw.ywBottom;
		}
	Assert ( (mpblz[brk.blMac+1] - mpblz[0]) >= brk.dxWidth );

	/* decide what to do with the shadow code */
	if (brk.fShadow)
		{
		if (dbmod & dbmodReverse)
			{
			Assert(brk.mpblblp[0] == blpShadow);
			brk.mpblblp[0] = blpWhite;
			}
		else
			{
			if (fLaserWriter && fVertical && brk.mpblblp[0] == blpLaser)
				{
				Assert(brk.blMac == 2);
				brk.mpblblp[0] = blpBlack;
				brk.mpblblp[1] = blpWhite;
				}
			else
				{
				Assert(brk.mpblblp[brk.blMac-1] == blpShadow);
				brk.mpblblp[brk.blMac-1] = blpBlack;
				}
			}
		}

	/* build array of internal strip limits */
	for ( bl = 0; bl < brk.blMac; ++bl )
		{
		mpblz[bl+1] = mpblz[bl] + brk.mpbldx[bl];
		mpblblp[bl] = brk.mpblblp[bl];
		}
	Assert ( (mpblz[brk.blMac] - mpblz[0]) == brk.dxWidth );

	blMac = brk.blMac;

	/* if dbmodVertical, there are no connections to deal with */
	if (fVertical)
		goto LDrawBrc;

	/* search the brc array for matches among the neighbors */
	Break3();

	cbrcMatch = CbrcMatch(rgbrc,fFrameLines,&brcMatch,&rgbitMatch);

	fHairline = fLaserWriter && (brcMatch & ~(brcfShadow | brcDxpSpace)) == brcHairline;

	if (cbrcMatch == 0)
		{
		/* If, ignoring frame lines, the current border has zero-width,
		/* and the left border has non-zero width, extend the left border
		/* into the current area by an amount equal to the max width of the
		/* top and bottom borders. This makes a border above a given cell
		/* left/right symmetrical if there is nothing around to interfere.
		/**/
		if (DxFromBrc(brcCur,fFalse/*fFrameLines*/) == 0
				&& DxFromBrc(rgbrc[ibrcLeftButt],fFalse/*fFrameLines*/) != 0)
			{
			/* This is a kludge to trick the corner-drawing block
			/* into extending the border on the left into the
			/* (non-existant) current border.
			/**/
			brcMatch = rgbrc[ibrcLeftButt];
			if (fLaserWriter && brcMatch == brcHairline
					&& (dxwWidth=DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/)) > 0)
				{
				fHairline = fTrue;
				goto LExtendBottom;
				}
			rgbitMatch = 0x0B;
			BrkFromBrc ( brcMatch,&brkCorner,fFrameLines,fFalse );
			rcwCorner = rcw;
			rcwCorner.xwRight = rcw.xwLeft +=
					max(DxFromBrc(rgbrc[ibrcTopButt],fFrameLines),
					DxFromBrc(rgbrc[ibrcBottomButt],fFrameLines));
			if (brkCorner.dxWidth == 0
					|| (dxwWidth = rcwCorner.xwRight - rcwCorner.xwLeft) <= 0
					|| rcwCorner.xwRight > rcw.xwRight)
				{
				rcw.xwLeft = rcwCorner.xwLeft; /* restore rcw */
				goto LDrawBrc;
				}
			else  if (fLaserWriter
					&& (rgbrc[ibrcTopButt] == brcHairline || rgbrc[ibrcBottomButt] == brcHairline)
					&& (rgbrc[ibrcTopButt] == brcHairline || DxFromBrc(rgbrc[ibrcTopButt],fFalse) == 0)
					&& (rgbrc[ibrcBottomButt] == brcHairline || DxFromBrc(rgbrc[ibrcBottomButt],fFalse) == 0))
				{
				/* mis-use a dxw as a dyw */
				rcw.xwLeft = rcwCorner.xwLeft; /* restore rcw */
LDrawHairyCorner:
				SetHairline();
				dxwWidth = DxFromBrc(rgbrc[ibrcBottomButt],fFalse) == 0
						? max(DyFromBrc(rgbrc[ibrcLeftButt],fFalse),DyFromBrc(brcCur,fFalse))
						: rcw.ywBottom-rcw.ywTop;
				MoveTo(rcw.xwLeft,rcw.ywTop);
				Line(0,dxwWidth - 1);
				ResetHairline();

				goto LDrawBrc;
				}

			goto LProcessCorner;
			}
		else  if (DyFromBrc(brcCur,fTrue/*fFrameLines*/) < rcw.ywBottom - rcw.ywTop
				&& DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/) != 0)
			{
			/* Bring the border below up to join with the current border */
			rcwCorner = rcw;	/* misuse a variable */
			rcwCorner.ywTop += DyFromBrc(brcCur,fFrameLines);
			rcwCorner.xwRight = rcwCorner.xwLeft + DxFromBrc(rgbrc[ibrcBottomButt],fFrameLines);
			DrawBrc(ww,&rgbrc[ibrcBottomButt],rcwCorner,dbmodLeft,fFrameLines);
			SetWords(rgbrcT,brcNil,cbrcCorner);
			rgbrcT[ibrcDraw] = brcCur;
			rcwCorner.ywBottom = rcwCorner.ywTop;
			rcwCorner.ywTop = rcw.ywTop;
			DrawBrc(ww,rgbrcT,rcwCorner,dbmodTop,fFrameLines);
			rcw.xwLeft = rcwCorner.xwRight;
			goto LDrawBrc;
			}
		else  if (fLaserWriter)
			{
			if (brcCur == brcHairline && (dxwWidth=DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/)) != 0)
				{
				fHairline = fTrue;
				goto LExtendBottom;
				}
			else  if ((rgbrc[ibrcTopButt] & ~(brcfShadow | brcDxpSpace)) == brcHairline
					|| (rgbrc[ibrcBottomButt] & ~(brcfShadow | brcDxpSpace)) == brcHairline)
				goto LDrawHairyCorner;
			}
		goto LDrawBrc;
		}

	/* we have found some matches among the brc's. Draw the left-most portion
	/* of the rcw to make the connection. The kludgy conditional is to get
	/* the BRK reversed for shadows under the necessary circumstances.
	/**/
	Break3();
	if (fHairline)
		{
		if (brcCur != brcMatch && DzFromBrc(brcCur,fFalse) > 0)
			{
			if ((rgbitMatch & 0xC) == 0x0C) /* top and bottom are hairline */
				{
				SetHairline();
				MoveTo(rcw.xwLeft,rcw.ywTop);
				LineTo(rcw.xwLeft,rcw.ywBottom-1);
				ResetHairline();
				}
			goto LDrawBrc;
			}
		else  if ((dxwWidth=DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/)) > 0
				&& (rgbrc[ibrcBottomButt] & ~(brcfShadow | brcDxpSpace)) != brcHairline)
			{
LExtendBottom:
			/* draw the hairline for the corner segment */
			if (fHairline || brcCur != brcMatch && DzFromBrc(brcCur,fFalse) > 0)
				{
				if (brcCur == brcNone || brcCur == brcNil ||
						rcw.xwLeft + dxwWidth >= rcw.xwRight)
					dxwWidth--;
				SetHairline();
				MoveTo(rcw.xwLeft,rcw.ywTop);
				Line(max(1,dxwWidth-1),0);
				ResetHairline();
				}

			/* bring the vertical border below up to join with the hairline */
			fHairline = fFalse;
			rgbitMatch = 0xC; /* top and bottom */
			brcMatch = rgbrc[ibrcBottomButt];
			cbrcMatch = 1;
			}
		}
	if (fHairline && brcMatch == (brcHairline | brcfShadow))
		{
		if ((rgbitMatch & 0x6) == 0x6)
			{
			fHairline = fFalse;
			brcMatch = brcSingle;
			}
		else  if ((dbmod & dbmodReverse) == 0)
			{
			int rgbrcT[cbrcCorner];

			SetHairline();
			MoveTo(rcw.xwLeft+1,rcw.ywTop);
			Line(1,0);
			ResetHairline();

			rgbrcT[0] = brcSingle;
			SetWords(&rgbrcT[1],brcNone,cbrcCorner-1);
			rcw.xwLeft += max(DxFromBrc(rgbrc[ibrcTopButt],fFalse),DxFromBrc(rgbrc[ibrcBottomButt],fFalse));
			DrawBrc(ww,rgbrcT,rcw,dbmod,fFrameLines);
			return;
			}
		else  if (brcCur == brcNil)
			{
			SetHairline();
			MoveTo(rcw.xwLeft,rcw.ywBottom-1);
			Line(0,1);
			ResetHairline();
			return;
			}
		}
	BrkFromBrc(brcMatch,&brkCorner,fFrameLines,
			(rgbitMatch&0x0c)==0x0c && brcCur!=brcNil/*fReverse*/);

	rcwCorner = rcw; /* unless changed in the next block... */

	rcwCorner.xwRight = rcwCorner.xwLeft + brkCorner.dxWidth; /*assumed later*/
	dxwWidth = brkCorner.dxWidth;

LProcessCorner:
	/* If we're drawing a hairline on the LaserWriter, we must skip the
	/* various conversions that normally occur in the next block.
	/**/
	if (fHairline)
		goto LDrawCorner;

	switch ( rgbitMatch )
		{
		/* bits are: bottom - top - left - right */
	default:
		Assert ( fFalse );
		break;

	case 0x3: /* 0011 left and right */
	case 0x7: /* 0111 top, left and right */
	case 0xB: /* 1011 bottom, left and right */
		rgbitMatch = 0xB;
		/* fall through to next block */

	case 0x6: /* 0110 top and left */
		if (brkCorner.fShadow)
			{
			Assert(brkCorner.mpblblp[brkCorner.blMac-1]==blpShadow);
			brkCorner.mpblblp[brkCorner.blMac-1] = blpBlack;
			}
		/* fall through to next block */

	case 0x5: /* 0101 top and right */

		/* Bring the bottom brc up to butt with the corner we are
		/* about to draw. Then slice that portion off from rcwCorner.
		/**/
		rcwCorner.ywTop += brkCorner.dxWidth;

		dxwBottomButt = DxFromBrc(rgbrc[ibrcBottomButt],fFrameLines);
		if (fEraseBits && dxwBottomButt < dxwWidth)
			{
			/* erase bits below the impending corner connection and
			/* to the right of the impending bottom butt connection.
			/**/
			rcwCorner.xwLeft += dxwBottomButt;
			EraseRc(ww, &rcwCorner);
			rcwCorner.xwLeft -= dxwBottomButt;
			}
		rcwCorner.xwRight = rcwCorner.xwLeft + dxwBottomButt;
		if (rgbrc[ibrcBottomButt] != brcNil && rcwCorner.ywTop < rcwCorner.ywBottom)
			{
			DrawBrc (ww,&rgbrc[ibrcBottomButt],rcwCorner,dbmodVertical, fFrameLines);
			if (dxwBottomButt > dxwWidth && fEraseBits)
				{
				rcw.xwLeft = rcwCorner.xwLeft + dxwBottomButt;
				rcw.ywTop = rcwCorner.ywTop;
				EraseRect(&rcw);
				rcw.ywBottom = rcw.ywTop;
				rcw.ywTop -= brkCorner.dxWidth;
				rcw.xwLeft = rcwCorner.xwLeft + dxwWidth;
				}
			}
		rcwCorner.xwRight = rcwCorner.xwLeft + dxwWidth;
		rcwCorner.ywBottom = rcwCorner.ywTop;
		rcwCorner.ywTop -= brkCorner.dxWidth;
		break;

	case 0x9: /* 1001 bottom and right */
	case 0xA: /* 1010 bottom and left */
		break;

	case 0xD: /* 1101 bottom, top and right */
	case 0xC: /* 1100 bottom and top */
	case 0xE: /* 1110 bottom, top and left */
LTopAndBottom:
		if (brkCorner.fShadow && brcCur == brcNil
				&& brkCorner.mpblblp[brkCorner.blMac-1] == blpShadow)
			{
			brkCorner.mpblblp[brkCorner.blMac-1] = blpBlack;
			}
		rgbitMatch = 0xE;
		break;
	case 0xF: /* 1111 bottom, top, left and right */
		if ( brk.blMac == 3 )
			{
			/* FUTURE: this code should be setting the pen pattern */
			/*    This will be important if/when we allow more brc's.
			/*    - code assumes that the middle stripe is white
			/**/
			Assert ( brkCorner.mpblblp[1] == blpWhite );
			if (fEraseBits)
				EraseRc ( ww, &rcwCorner );

			/* draw the upper right and lower left corners */
			dz = brkCorner.mpbldx[0];
			PenSize ( dz, dz );
			MoveTo ( rcwCorner.xwRight - dz, rcwCorner.ywTop );
			Line ( 0, 0 );
			MoveTo ( rcwCorner.xwLeft, rcwCorner.ywTop + brk.dxWidth - dz );
			LineTo ( rcwCorner.xwLeft, rcwCorner.ywBottom - dz );

			/* draw the lower right and upper left corners */
			dz = brkCorner.mpbldx[2];
			PenSize ( dz, dz );
			MoveTo ( rcwCorner.xwLeft, rcwCorner.ywTop );
			Line ( 0, 0 );
			MoveTo ( rcwCorner.xwRight - dz, rcwCorner.ywTop + brk.dxWidth - dz );
			LineTo ( rcwCorner.xwRight - dz, rcwCorner.ywBottom - dz );

			/* let everyone know this chunk is taken care of */
			rcw.xwLeft += brkCorner.dxWidth;
			rgbitMatch = 0x0; /* the corner is drawn */
			}
		else
			goto LTopAndBottom; /* treat line top/bottom match */
		break;
		}

	PenNormal();

	if ( rgbitMatch == 0 )
		goto LDrawBrc;

LDrawCorner:
	rcw.xwLeft = rcwCorner.xwRight;

	if (fHairline)
		{
		/* more hacking for the LaserWriter */
		fRight = fBottom = fFalse;
		fLeft = rgbitMatch & 0x8;
		fTop = rgbitMatch & 0x01;
		}
	else
		{
		fRight = !(rgbitMatch & 0x1);
		fLeft = !(rgbitMatch & 0x2);
		fTop = !(rgbitMatch & 0x4);
		fBottom = !(rgbitMatch & 0x8);
		}

	Break3();
	for ( bl = brkCorner.blMac; bl-- > 0; )
		{
		dz = brkCorner.mpbldx[bl];
		BreakW(6);
		switch ( brkCorner.mpblblp[bl] )
			{
		default:
			Assert ( fFalse );
			break;
		case blpShadow:
			/* this means we don't really want to draw the shadow */
			/* so, FALL THROUGH */
		case blpWhite:
			if (fEraseBits)
				PenPat ( WinMac(&patWhite, &vqdg.patWhite) );
			else
				goto LNextCornerLine;
			break;
		case blpGrey:
			PenPatGray(ww);
			break;
		case blpBlack:
			PenPat (WinMac(&patBlack, &vqdg.patBlack));
			break;
		case blpLaser:
			if (fLaserWriter)
				{
				Assert (!(fRight | fBottom));
				SetHairline();
				if (fTop)
					{
					Assert(rcwCorner.xwLeft < rcwCorner.xwRight);
					MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
					LineTo(rcwCorner.xwRight,rcwCorner.ywTop);
					}
				if (fLeft)
					{
					Assert(rcwCorner.ywTop < rcwCorner.ywBottom);
					MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
					LineTo(rcwCorner.xwLeft,rcwCorner.ywBottom);
					}
				ResetHairline();
				Win(PenPat(&patBlack));
				goto LNextCornerLine;
				}
			PenPat(WinMac(&patBlack, &vqdg.patBlack));
			break;
			}
		Assert(!FEmptyRc(&rcwCorner));
		if (fTop)
			{
			PenSize(1,dz);
			MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
			LineTo(rcwCorner.xwRight-1,rcwCorner.ywTop);
			}
		if (fBottom)
			{
			PenSize(1,dz);
			MoveTo(rcwCorner.xwLeft,rcwCorner.ywBottom-dz);
			LineTo(rcwCorner.xwRight-1,rcwCorner.ywBottom-dz);
			}
		if (fLeft)
			{
			PenSize(dz,1);
			MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
			LineTo(rcwCorner.xwLeft,rcwCorner.ywBottom-1);
			}
		if (fRight)
			{
			PenSize(dz,1);
			MoveTo(rcwCorner.xwRight-dz,rcwCorner.ywTop);
			LineTo(rcwCorner.xwRight-dz,rcwCorner.ywBottom-1);
			}
LNextCornerLine:
		rcwCorner.xwLeft += (fLeft) ? dz : 0;
		rcwCorner.xwRight -= (fRight) ? dz : 0;
		rcwCorner.ywTop += (fTop) ? dz : 0;
		rcwCorner.ywBottom -= (fBottom) ? dz : 0;
		}

	/* white out any left over space in the corner */
	if (fEraseBits && !FEmptyRc(&rcwCorner))
		FillRect ( &rcwCorner, WinMac(&patWhite, &vqdg.patWhite) );

LDrawBrc:
	if (FEmptyRc(&rcw))
		return;	/* nothing left to draw */

	PenNormal();

	if (fVertical)
		{
		yw1 = rcw.ywTop;
		yw2 = rcw.ywBottom - 1;
		/* slice off what we are about to draw */
		rcw.xwLeft = mpblz[blMac];
		}
	else  /* horizontal */		
		{
		xw1 = rcw.xwLeft;
		xw2 = rcw.xwRight - 1;
		/* slice off what we are about to draw */
		rcw.ywTop = mpblz[blMac];
		}
	for ( bl = 0; bl < blMac; ++bl )
		{
		if (fVertical)
			{
			xw1 = xw2 = mpblz[bl];
			PenSize(mpblz[bl+1]-xw1,1);
			}
		else  /* dbmodHoriz */			
			{
			yw1 = yw2 = mpblz[bl];
			PenSize(1, mpblz[bl+1]-yw1);
			}
		BreakW(7);
		switch ( mpblblp[bl] )
			{
		default:
			Assert ( fFalse );
			goto LContinue;
		case blpWhite:
			if (fEraseBits)
				PenPat(WinMac(&patWhite, &vqdg.patWhite));
			else
				goto LContinue;
			break;
		case blpGrey:
			PenPatGray(ww);
			break;
		case blpBlack:
			PenPat ( WinMac(&patBlack, &vqdg.patBlack));
			break;
		case blpLaser:
			if (fLaserWriter)
				{
				/* more hacking for the LaserWriter */
				SetHairline();
				if (fVertical)
					{
					MoveTo(xw1,yw1-1);
					LineTo(xw1,yw2+1);
					}
				else
					{
					MoveTo(xw1,yw1);
					LineTo(xw2+1,yw1);
					}
				ResetHairline();
				Win(PenPat(&patBlack));
				goto LContinue;
				}
			PenPat (WinMac(&patBlack, &vqdg.patBlack));
			break;
			}
		if (xw1 <= xw2 && yw1 <= yw2)
			{
			MoveTo(xw1,yw1);
			LineTo(xw2,yw2);
			}
LContinue:
		continue;
		}

	if (fEraseBits)
		{
		if (!FEmptyRc(&rcw)) /* guards against bogus rcw */
			EraseRc (ww, &rcw);
		}
	PenNormal();
}


/* %%Function:CbrcMatch %%Owner:NOTUSED */
CbrcMatch(rgbrc,fFrameLines,pbrcMatch,prgbitMatch)
int	rgbrc[];
BOOL fFrameLines;
int	*pbrcMatch;
int *prgbitMatch;
{
	int	bitStart, bitCur;
	int	ibrc, ibrcT, cbrcMatch, cbrcT;
	int rgbitMatch, rgbitT;
	int	brcMatch, brcT;
	int fRealBorders;

	bitStart = 1;
	cbrcMatch = 0;
	fRealBorders = fFalse;
	brcMatch = brcNil;
	for ( ibrc = 0; ibrc < 3; ++ibrc, bitStart <<= 1 )
		{
		if ((brcT=rgbrc[ibrc]) == brcNil)
			continue;
		if (brcT == brcNone)
			{
			if (fRealBorders)
				continue;
			}
		else
			{
			fRealBorders = fTrue;
			if (brcMatch == brcNone)
				cbrcMatch = 0;
			}
		cbrcT = 0;
		bitCur = (rgbitT = bitStart) << 1;
		for ( ibrcT = ibrc + 1; ibrcT < 4; ++ibrcT, bitCur <<= 1 )
			if ( rgbrc[ibrcT] == brcT )
				{
				++cbrcT;
				rgbitT |= bitCur;
				}
		if (cbrcT > 0)
			{
			if ( DxFromBrc(brcT,fFrameLines) >= DxFromBrc(brcMatch,fFrameLines) )
				{
				brcMatch = brcT;
				cbrcMatch = cbrcT;
				rgbitMatch = rgbitT;
				if (cbrcMatch > 1 && brcMatch != brcNone)
					break;
				}
			}
		}
	if (brcMatch == brcNone && fRealBorders)
		cbrcMatch = 0;
	else
		{
		*prgbitMatch = rgbitMatch;
		*pbrcMatch = brcMatch;
		}
	return cbrcMatch;
}


#endif	/* MAC */

#ifdef WIN
#ifdef DEBUG
/* %%Function:DxyFromBrc %%Owner:tomsax %%Reviewed:7/6/89 */
DxyFromBrc(brc, fFrameLines, fWidth)
struct BRC brc;
int fFrameLines, fWidth;
{
	Assert ((fFrameLines & 0xFFFE) == 0);
	Assert ((fWidth & 0xFFFE) == 0);
	return (WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1)));
}


/* %%Function:DxyOfLineFromBrc %%Owner:tomsax %%Reviewed:7/6/89 */
DxyOfLineFromBrc(brc, fFrameLines, fWidth)
struct BRC brc;
int fFrameLines, fWidth;
{
	Assert ((fFrameLines & 0xFFFE) == 0);
	Assert ((fWidth & 0xFFFE) == 0);
	return (WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1) | 4/*fLine*/));
}


#endif /* DEBUG */
#endif /* WIN */


/* W I D T H  H E I G H T  F R O M  B R C */
/* Returns either the width or the height for the brc in device units.
/* If fLine, ignore initial whitespace.
/* */
#ifdef DEBUGORNOTWIN
#ifdef MAC
WidthHeightFromBrc(brc, fFrameLines, fWidth, fLine) /* WINIGNORE - MAC only */
struct BRC brc;
int fFrameLines, fWidth, fLine;
#else /* WIN */
/* %%Function:WidthHeightFromBrc %%Owner:tomsax %%Reviewed:7/6/89 */
HANDNATIVE C_WidthHeightFromBrc(brc, grpf)
struct BRC brc;
int grpf;
#endif /* WIN */
{
	int dzp, dz;
#ifdef WIN
	int fFrameLines, fWidth, fLine;

	fFrameLines = grpf & 1;
	fWidth = grpf & 2;
	fLine = grpf & 4;
#endif /* WIN */

	dzp = fWidth ? dxpBorderFti : dypBorderFti;

	if ((int) brc == brcNone && fFrameLines)
		return dzp;
	if ((int) brc == brcNone || (int) brc == brcNil)
		return 0;

	/* All the more difficult cases */
	dz = 0;
	if (!fLine)
#ifdef MAC
		dz = brc.dxpSpace;
#else /* WIN */
	/* brc.dxpSpace is in points - Hungarian name is wrong */
	dz = NMultDiv(brc.dxpSpace,
			fWidth ? vfti.dxpInch : vfti.dypInch,
			cptInch);
#endif
	if (brc.fShadow)
		dz += fWidth ? dxShadow : dyShadow;
	switch (brc.brcBase)
		{
	case brcSingle:
		return dz + dzp;
	case brcTwoSingle:
		return dz + 3 * dzp;
	case brcThick:
		return dz + 2 * dzp;
#ifdef MAC
	case brcDotted:
	case brcHairline:
		return dz + dzp;
#endif /* MAC */
		}

	Assert(fFalse);
	return 0;
}
#endif /*DEBUGORNOTWIN*/


/* B R K  F R O M  B R C
/*
/* Description: Decodes a brc into the a brk for drawing. Pass
/* fTrue for fFrameLines if a brcNone should be converted to a
/* frame line.
/**/
/* %%Function:BrkFromBrc %%Owner:tomsax %%Reviewed:7/6/89 */
BrkFromBrc ( brc, pbrk, fFrameLines, fReverse )
struct BRK *pbrk;
int fReverse;   /* Reverse the bits? */
{
	int blMac, wT, bl, blMirror, dz;
	int dxWidth, dyHeight;
	int *mpbldx, *mpbldy, *mpblblp;

	/* for faster access... */
	mpbldx = pbrk->mpbldx;
	mpbldy = pbrk->mpbldy;
	mpblblp = pbrk->mpblblp;

	pbrk->fShadow = fFalse; /* until proven otherwise */

	switch ( brc )
		{
	case brcNone:
		if ( fFrameLines )
			{
			/* override with a dotted line for the cell frame */
			pbrk->blMac = 1;
			mpbldx[0] = pbrk->dxWidth = dxpBorderFti;
			mpbldy[0] = pbrk->dyHeight = dypBorderFti;
			mpblblp[0] = blpGrey;
			return;
			}
		/* else - fall through to brcNil case */
	case brcNil:
		pbrk->dyHeight = pbrk->dxWidth = 0;
		pbrk->blMac = 0;
		return;
	default:
		blMac = 0;
		if (brc & brcDxpSpace)
			{
			dz = (brc & brcDxpSpace) >> shiftDxpSpaceBrc;
			/* dz is in points */
			mpbldx[blMac] =
					WinMac(NMultDiv(dz, vfti.dxpInch, cptInch), dz);
			mpbldy[blMac] =
					WinMac(NMultDiv(dz, vfti.dypInch, cptInch), dz);
			mpblblp[blMac++] = blpWhite;
			}
		mpblblp[blMac] = blpBlack;
		dz = (brc & brcLine1) >> shiftLine1Brc;
#ifdef	MAC
		if (dz >= 6)    /* Dotted line or Hair line */
			{
			Assert(dz == 6 || dz == 7);
			mpblblp[blMac] = dz == 6 ? blpGrey : blpLaser;
			dz = 1;
			}
#endif
		mpbldx[blMac] = dz*dxpBorderFti;
		mpbldy[blMac++] = dz*dypBorderFti;
		if (brc & brcSpace2 != 0)
			{
			mpblblp[blMac] = blpWhite;
			dz = (brc & brcSpace2) >> shiftSpace2Brc;
			mpbldx[blMac] = dz * dxpBorderFti;
			mpbldy[blMac++] = dz * dypBorderFti;
			}
		if (dz = (brc & brcLine2))
			{
			mpblblp[blMac] = blpBlack;
#ifdef	MAC
			Assert(dz == 1 || dz == 6 || dz == 7);
			if (dz == 6)    /* Dotted line */
				mpblblp[blMac] = blpGrey;
			else  if (dz == 7)       /* Hairline */
				mpblblp[blMac] = blpLaser;
#else
			Assert(dz == 1);
#endif
			mpbldx[blMac] = dxpBorderFti;
			mpbldy[blMac++] = dypBorderFti;
			}
		if (pbrk->fShadow = ((brc & brcfShadow) != 0))
			{
			mpbldx[blMac] = dxShadow;
			mpbldy[blMac] = dyShadow;
			mpblblp[blMac++] = blpShadow;
			}
		if (fReverse && blMac > 1)
			{       /* Reverse those bits */
			for (bl = blMac>>1; bl-- > 0; )
				{
				blMirror = blMac - 1 - bl;
				wT = mpbldx[bl];
				mpbldx[bl] = mpbldx[blMirror];
				mpbldx[blMirror] = wT;

				wT = mpbldy[bl];
				mpbldy[bl] = mpbldy[blMirror];
				mpbldy[blMirror] = wT;

				wT = mpblblp[bl];
				mpblblp[bl] = mpblblp[blMirror];
				mpblblp[blMirror] = wT;
				}
			}
		pbrk->blMac = blMac;
		}

	/* NOTE: we are about to destroy mpbldx, and mpbldy */
	dxWidth = dyHeight = 0;
	for (bl = pbrk->blMac; bl-- > 0 ; )
		{
		dxWidth += *mpbldx++;
		dyHeight += *mpbldy++;
		}
	pbrk->dxWidth = dxWidth;
	pbrk->dyHeight = dyHeight;
}


/* F R A M E  T A B L E
/*
/* Description: Draws the portions of the table not covered by the (extended)
/* cell DRs; including drawing frame lines or borders and clearing bits not
/* already drawn.
/**/
/* %%Function:FrameTable %%Owner:tomsax %%Reviewed:7/6/89 */
NATIVE
FrameTable ( ww, doc, cp, hpldr, pedl, fFirstRow, fLastRow )
int ww, doc;
CP cp;
struct PLDR **hpldr;
struct EDL *pedl;
BOOL fFirstRow, fLastRow;
{
	struct WWD *pwwd;
	struct DR *pdrParent, *pdrT;
	struct PLDR *ppldr;
	int itc, itcNext, itcMac;
	int dyl, dylAbove, dylBelow;
	int dxpBrcLeft;
	int brcBottomPrev = brcNil;
	BOOL fFrameLines, fPageView, fDrFrameLines;
	BOOL fOutline;
	int xlRowLeft;
	struct RC rcw, rcl, rclDrawn;
	struct TCX tcx;
	int rgbrc[cbrcCorner];
	struct DRF drfFetch, drfFetchT;
#ifdef WIN
	HDC hdc;
	int iLevel = -1;
	struct RC rcwDisp;
#endif

	dyl = pedl->dyp;
	pwwd = PwwdSetWw(ww, cptDoc);
	fOutline = pwwd->fOutline;
	fPageView = pwwd->fPageView;
	Win(hdc = pwwd->hdc;)
			fFrameLines = FDrawTableDrsWw(ww);
	fDrFrameLines = fPageView && FDrawPageDrsWw(ww);
	pdrParent = PdrFetch((*hpldr)->hpldrBack,(*hpldr)->idrBack,&drfFetch);
	if (fPageView)
		{
		RcwPgvTableClip(ww, (*hpldr)->hpldrBack, (*hpldr)->idrBack, &rcw);
#ifdef MAC 
		ClipRc(&rcw);
#endif /* MAC */
		}

	/* NOTE We are assuming that the 'xl' coordinates in the table frame (PLDR)
	/* are the same as the 'xp' coordinates of the bounding DR.
	/**/
	Assert ( (*hpldr)->ptOrigin.xp == 0 );

	/* call CacheTc to get the border calculations and info on cell 0 */
	CacheTc ( ww, doc, cp, fFirstRow, fLastRow);
	Assert ( vtcc.itc == 0 );

	CpFirstTap1(doc, cp, fOutline);
	Assert(cp == caTap.cpFirst);
	itcMac = vtapFetch.itcMac;
	xlRowLeft = vtcc.xpDrLeft;

	if (vtcc.fEasyBorders)
		{
		dylAbove = vtcc.dylAbove;
		FrameEasyTable(ww, doc, cp, hpldr, &rclDrawn, pdrParent, dyl, fFrameLines, fDrFrameLines, fFirstRow, fLastRow);
		goto LFinish;
		}

#ifdef WIN
/* when it is not the easy case, set up clip rect 
(because it is expansive for WINDOW)
rcw has already been clipped to the dr within the page */
	pwwd = PwwdWw(ww);
	rcwDisp = fPageView ? rcw : pwwd->rcwDisp;
	if ((iLevel = SaveDC(hdc)) > 0)
		IntersectClipRect(hdc, rcwDisp.xwLeft, rcwDisp.ywTop,
				rcwDisp.xwRight, rcwDisp.ywBottom);
#endif

	/* draw the top borders */
	dylAbove = DylDrawTableTop(ww,hpldr,&rclDrawn,fFrameLines,fDrFrameLines,fFirstRow,fLastRow);

	PrcSet(&rcl,
			-pdrParent->dxpOutLeft,
			dylAbove,
			vtcc.xpDrLeft - vtcc.dxpOutLeft - DxFromBrc(vtcc.brcLeft,fTrue),
			dyl);

	/* carefully clear bits to the left of the table row
	/* and below the top border */
	ClearRclInParentDr(ww,hpldr,rcl,NULL);

	/* Draw the vertical borders/frame lines,
	/* clear pixels between dr and bottom frame.
	/**/
	for ( itc = 0; itc <= itcMac; itc = itcNext )
		{
		dylBelow = vtcc.dylBelow; /* needs to be initialized for each pass */
		itcNext = ItcGetTcxCache ( ww, caTap.doc, caTap.cpFirst, &vtapFetch, itc, &tcx);
		dxpBrcLeft = DxFromBrc(tcx.brcLeft,fTrue);
		rcl.xlRight = tcx.xpDrLeft - tcx.dxpOutLeft;
		rcl.xlLeft = rcl.xlRight - dxpBrcLeft;
		rcl.ylTop = vtcc.dylAbove;	/* NOT the local dylAbove */
		rcl.ylBottom = dyl - dylBelow;
		RclToRcw ( hpldr, &rcl, &rcw );

		rgbrc[ibrcDraw] = tcx.brcLeft;
		DrawBrc ( ww, rgbrc, rcw, dbmodVertical, fFrameLines);

		/* expand the rcw to the full cell width */
		rcw.xwRight += tcx.xpDrRight + tcx.dxpOutRight - (tcx.xpDrLeft - tcx.dxpOutLeft);

		/* crop the DR from the rcw, clear bits above the bottom frame */
		/* note that itc serves as an idr */
		pdrT = PdrFetchAndFree(hpldr,itc,&drfFetchT);	/* verbose for native compiler */
		rcw.ywTop = YwFromYp(hpldr,itc,pdrT->dyl);
		if ( rcw.ywTop < rcw.ywBottom )
			{
			/* If there was no space alloted at the bottom of the PLDR,
			/* and this is the last row in a table, and this cell's DR
			/* does not "touch the bottom" AND the cell is not the fTtp,
			/* then we have to lie a little to get the frame line drawn.
			/**/
			if (fFrameLines && fLastRow && dylBelow == 0)
				rcw.ywBottom -= dylBelow = 1;
			rcw.xwLeft += dxpBrcLeft;
			EraseRc ( ww, &rcw );
			rcw.xwLeft -= dxpBrcLeft;
			}

		/* if needed, draw the bottom border or frame */
		if ( dylBelow > 0 )
			{
			rcw.ywTop = rcw.ywBottom;
			rcw.ywBottom += dylBelow;
			brcBottomPrev = BrcDrawOneBottomBorder(ww,rcw,&tcx,brcBottomPrev,itc == vtapFetch.itcMac/*fLastCorner*/,
					fFrameLines);
			}
		else
			brcBottomPrev = brcNone;	/* NOT brcNil */

#ifdef	DEBUG
		while (++itc < itcNext)
			{
			pdrT = PdrFetchAndFree(hpldr,itc,&drfFetchT);
			Assert(pdrT->dxl == 0);
			}
#endif
		}

	Assert (itc == (*hpldr)->idrMac);

	/* erase the space above the TTP DR */
	if (dylAbove == 0)
		{
		PrcSet(&rcl,
				tcx.xpDrLeft - tcx.dxpOutLeft,
				0,
				tcx.xpDrRight + tcx.dxpOutRight,
				dyFrameLine);
		ClearRclInParentDr(ww,hpldr,rcl,NULL);
		}

	/* carefully clear the bits to the right of the space covered by the PLDR */
	PrcSet(&rcl,
			tcx.xpDrRight + tcx.dxpOutRight,
			dylAbove,
			pdrParent->dxl + pdrParent->dxpOutRight,
			dyl);
	ClearRclInParentDr(ww,hpldr,rcl,NULL);

	rclDrawn.xlRight = max(rclDrawn.xlRight,rcl.xlLeft);

LFinish:
	pedl->xpLeft = rclDrawn.xlLeft;
	pedl->dxp = rclDrawn.xwRight - rclDrawn.xlLeft;

	/* now, if we're showing DR frame lines, redraw them over
	/* (almost) everything else
	/**/
	if (fDrFrameLines)
		{
		int ylTop, ylBottom;

		ppldr = *hpldr;
		ylTop = 0;
		ylBottom = ppldr->dyl;
		if (ylTop < ylBottom)
			FrameDrLine(ww, ppldr->hpldrBack, ppldr->idrBack,
					YwFromYl(hpldr,ylTop)/*ywTop*/, YwFromYl(hpldr,ylBottom)/*ywBottom*/,
					fFalse /*fFat*/, fTrue/*fForceLine*/, fFalse/*fInnerDr*/);
		}

	PenNormal();
	pwwd = PwwdWw(ww);
	if (pwwd->fOutline)
		{
		int xwT, ywT;
		OtlMarkPos(XwFromXl(hpldr, xlRowLeft),
				YwFromYl(hpldr, dylAbove), dyl, &xwT, &ywT);
		DrawVisiBitsOtlMarkBody(ww, xwT, ywT);
		}
	else
		{
/* check for soft page breaks */
#ifdef MAC
		PATTERN patNew;
#endif
		int xw, xwT, yw;
		struct DOD *pdod;

		pdod = PdodDoc(doc);
		if (!fPageView && !pdod->fShort && pdod->hplcpgd)
			{
			if (!FInCa(doc, cp, &caPage))
				CachePage(doc, cp);
/* we are now guaranteed that cp is within the cached page */
			if (caTap.cpLim == caPage.cpLim &&
					caTap.cpLim < CpMacDocEdit(doc))
				{
/* last line of page. Show page break */
				yw = YwFromYl(hpldr,dyl-vtcc.dylBelow-1);
				xw = XwFromXp((*hpldr)->hpldrBack,(*hpldr)->idrBack,-pdrParent->dxpOutLeft);
				pedl->xpLeft = -pdrParent->dxpOutLeft;
#ifdef MAC
				ScrollPat(&vqdg.patLtGray, xw, yw, &patNew);
				PenPat(&patNew);
#endif
				ItcGetTcxCache ( ww, doc, cp, &vtapFetch, 0, &tcx);
				xwT = XwFromXl(hpldr,tcx.xpDrLeft-tcx.dxpOutLeft-DxFromBrc(tcx.brcLeft,fTrue));
				if (xw < xwT)
					{
#ifdef MAC
					MoveTo(xw, yw);
					LineTo(xwT - 1, yw);
#else /* WIN */
					DrawPatternLine(hdc, xw, yw, xwT - xw,
							ipatHorzLtGray, pltHorz);
#endif
					}
				xw += pdrParent->dxpOutLeft + pdrParent->dxl + pdrParent->dxpOutRight;
				ItcGetTcxCache ( ww, doc, cp, &vtapFetch, vtapFetch.itcMac, &tcx);
				xwT = XwFromXl(hpldr,tcx.xpDrLeft-tcx.dxpOutLeft);
				if (xwT < xw)
					{
#ifdef MAC
					MoveTo(xwT, yw);
					LineTo(xw-1, yw);
#else /* WIN */
					DrawPatternLine(hdc, xwT, yw, xw - xwT,
							ipatHorzLtGray, pltHorz);
					/* To ensure correct "scrolling optimization" of
						xwLimScroll */
					pedl->dxp = pdrParent->dxpOutLeft + pdrParent->dxl + pdrParent->dxpOutRight;
#endif
					}
#ifdef MAC
				PenPat(&vqdg.patBlack);
#endif
				}
			}
		}

#ifdef WIN
	if (iLevel > 0)
		RestoreDC(hdc, iLevel);
#endif
	FreePdrf(&drfFetch);
}


#ifdef WIN
int dxpPenBrc, dypPenBrc;
HBRUSH hbrPenBrc;
#endif /* WIN */

/*	F R A M E  E A S Y  T A B L E
/*	Description: Under certain (common) circumstances, it is possible
/*	to draw the table borders without going through all of the hoops
/*	to correctly join borders at corners. This routines handles those
/*	cases.
/*
/*	Also uses info in caTap, vtapFetch and vtcc (itc = 0 cached).
/**/
#ifdef	DEBUGORNOTWIN
#ifdef MAC
NATIVE FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdrParent, dyl, fFrameLines, fDrFrameLines, fFirstRow, fLastRow) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:FrameEasyTable %%Owner:tomsax %%Reviewed:7/6/89 */
HANDNATIVE C_FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdrParent, dyl, fFrameLines, fDrFrameLines, fFirstRow, fLastRow)
#endif /* !MAC */
int ww, doc;
CP cp;
struct PLDR **hpldr;
struct RC *prclDrawn;
struct DR *pdrParent;
int dyl;
BOOL fFrameLines, fDrFrameLines, fFirstRow, fLastRow;
{
	int dxwBrcLeft, dxwBrcRight, dxwBrcInside, dywBrcTop, dywBrcBottom;
	int dxLToW, dyLToW;
	int xwLeft, xwRight, dylDrRow, ywTop;
	int itc, itcNext, itcMac;
	int brcCur;
	Mac(int dylDrRowM1;)
	BOOL fRestorePen, fBottomFrameLine;
	struct DR *pdr;
	struct RC rclErase, rcw;
	struct TCX tcx;
	struct DRF drf;
#ifdef WIN
	HDC hdc = PwwdWw(ww)->hdc;
	struct RC rcDraw, rcwClip;
	int xwT, ywT;
	struct WWD *pwwd = PwwdWw(ww);

	if (pwwd->fPageView)
		RcwPgvTableClip(ww, (*hpldr)->hpldrBack, (*hpldr)->idrBack, &rcwClip);
	else
		rcwClip = pwwd->rcwDisp;
#endif

	Assert(FInCa(doc, cp, &vtcc.ca));
	Assert(vtcc.itc == 0);
	Assert(dyl > 0);

	dxLToW = XwFromXl(hpldr, 0);
	dyLToW = YwFromYl(hpldr, 0);

	dxwBrcLeft = DxFromBrc(vtcc.rgbrcEasy[ibrcLeft],fTrue/*fFrameLines*/);
	dxwBrcRight = DxFromBrc(vtcc.rgbrcEasy[ibrcRight],fTrue/*fFrameLines*/);
	dxwBrcInside = DxFromBrc(vtcc.rgbrcEasy[ibrcInside],fTrue/*fFrameLines*/);
	dywBrcTop = vtcc.dylAbove;
	dywBrcBottom = vtcc.dylBelow;

	Assert(dywBrcTop == DyFromBrc(vtcc.rgbrcEasy[ibrcTop],fFalse/*fFrameLines*/));
	Assert(dywBrcBottom == (fLastRow ? DyFromBrc(vtcc.rgbrcEasy[ibrcBottom],fFalse/*fFrameLines*/) : 0));

	ywTop = dyLToW + dywBrcTop;
	dylDrRow = dyl - dywBrcTop - dywBrcBottom;
	Mac(dylDrRowM1 = dylDrRow - 1);
	itcMac = vtapFetch.itcMac;

	/* erase bits to the left of the PLDR */
	PrcSet(&rclErase,
			-pdrParent->dxpOutLeft,
			0,
			vtcc.xpDrLeft - vtcc.dxpOutLeft - dxwBrcLeft,
			dyl);
	xwLeft = rclErase.xlRight + dxLToW;	/* will be handly later */
	ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);

	PenNormal();

	/* the left border */
	SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcLeft], fFalse/*fHoriz*/, fFrameLines);
#ifdef MAC	
	MoveTo(xwLeft, ywTop);
	Line(0, dylDrRowM1);
#else /* WIN */
	PrcSet(&rcDraw, xwLeft, ywTop,
			xwLeft + dxpPenBrc, ywTop + dylDrRow);
	SectRc(&rcDraw, &rcwClip, &rcDraw);
	FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
#endif

	/* the inside borders */
	itc = 0;
	itcNext = ItcGetTcxCache ( ww, doc, cp, &vtapFetch, itc, &tcx);

	/* pre-compute loop invariant */
	fBottomFrameLine = fLastRow && fFrameLines && dywBrcBottom == 0;

	if (itcNext >= itcMac)
		{
		/* BEWARE the case of a single cell row, we have to hack things
		/* up a bit to avoid trying to draw a between border.
		/**/
		if (brcCur != brcNone)
			SetPenForBrc(ww, brcCur = brcNone, fFalse/*fHoriz*/, fFrameLines);
		}
	else  if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcInside])
		SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcInside], fFalse/*fHoriz*/, fFrameLines);
	for ( ; ; )
		{
		pdr = PdrFetchAndFree(hpldr, itc, &drf);
		if (pdr->dyl < dylDrRow)
			{
			DrclToRcw(hpldr, &pdr->drcl, &rcw);
			rcw.xwLeft -= pdr->dxpOutLeft;
			rcw.xwRight += pdr->dxpOutRight;
			rcw.ywTop += pdr->dyl;
			rcw.ywBottom += dylDrRow - pdr->dyl;
#ifdef WIN
			SectRc(&rcw, &rcwClip, &rcw);
#endif
			if (fBottomFrameLine)
				{
				--rcw.ywBottom;
				if (fRestorePen = brcCur != brcNone)
					SetPenForBrc(ww, brcCur = brcNone, fFalse/*fHoriz*/, fFrameLines);
#ifdef MAC
				MoveTo(rcw.xwLeft,rcw.ywBottom);
				LineTo(rcw.xwRight-1,rcw.ywBottom);
#else /* WIN */
				FillRect(hdc, (LPRECT)PrcSet(&rcDraw,
						rcw.xwLeft, rcw.ywBottom,
						rcw.xwRight, rcw.ywBottom + dypPenBrc),
						hbrPenBrc);
#endif
				if (fRestorePen)
					SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcInside], fFalse/*fHoriz*/, fFrameLines);
				if (rcw.ywTop >= rcw.ywBottom)
					goto LCheckItc;
				}
			EraseRc(ww, &rcw);
			}
LCheckItc:
		if (itcNext == itcMac)
			break;
#ifdef MAC
		MoveTo(tcx.xpDrRight + tcx.dxpOutRight + dxLToW, ywTop);
		Line(0, dylDrRowM1);
#else /* WIN */
		xwT = tcx.xpDrRight + tcx.dxpOutRight + dxLToW;
		PrcSet(&rcDraw, xwT, ywTop,
				xwT + dxpPenBrc, ywTop + dylDrRow);
		SectRc(&rcDraw, &rcwClip, &rcDraw);
		FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
#endif
		itc = itcNext;
		itcNext = ItcGetTcxCache ( ww, doc, cp, &vtapFetch, itc, &tcx);
		}

LTopBorder:
	Assert(itcNext == itcMac);
	xwRight = tcx.xpDrRight + tcx.dxpOutRight + dxLToW;

	/* top */
	if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcTop])
		SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcTop], fTrue/*fHoriz*/, fFrameLines);
	if (dywBrcTop > 0)
		{
#ifdef MAC
		MoveTo(xwLeft, dyLToW);
		LineTo(xwRight + dxwBrcRight - 1, dyLToW);
#else /* WIN */
		PrcSet(&rcDraw, xwLeft, dyLToW,
				xwRight + dxwBrcRight, dyLToW + dypPenBrc);
		SectRc(&rcDraw, &rcwClip, &rcDraw);
		FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
#endif
		}

	/* right */
	if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcRight])
		SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcRight], fFalse/*fHoriz*/, fFrameLines);
#ifdef MAC	
	MoveTo(xwRight, ywTop);
	Line(0, dylDrRowM1);
#else /* WIN */
	PrcSet(&rcDraw, xwRight, ywTop,
			xwRight + dxwBrcRight, ywTop + dylDrRow);
	SectRc(&rcDraw, &rcwClip, &rcDraw);
	FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
#endif

	/* bottom */
	if (dywBrcBottom > 0)
		{
		Assert(fLastRow);
		if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcBottom])
			SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcBottom], fTrue/*fHoriz*/, fFrameLines);
#ifdef MAC
		MoveTo(xwLeft, dyLToW + dyl - dywBrcBottom);
		Line(xwRight + dxwBrcRight - xwLeft - 1, 0);
#else /* WIN */
		ywT = dyLToW + dyl - dywBrcBottom;
		PrcSet(&rcDraw, xwLeft, ywT,
				xwRight + dxwBrcRight, ywT + dypPenBrc);
		SectRc(&rcDraw, &rcwClip, &rcDraw);
		FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
#endif
		}

	/* clear any space above or below the fTtp DR */
	pdr = PdrFetchAndFree(hpldr, itcMac, &drf);
	DrcToRc(&pdr->drcl, &rclErase);
	rclErase.xlLeft -= pdr->dxpOutLeft;
	rclErase.xlRight += pdr->dxpOutRight;
	if (fFrameLines && rclErase.ylTop == 0)
		rclErase.ylTop = dyFrameLine;
	if (rclErase.ylTop > 0)
		{
		rclErase.ylBottom = rclErase.ylTop;
		rclErase.ylTop = 0;
		ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
		rclErase.ylBottom += pdr->drcl.dyl;
		}
	if (rclErase.ylBottom < dyl)
		{
		rclErase.ylTop = rclErase.ylBottom;
		rclErase.ylBottom = dyl;
		ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
		}

	PrcSet(prclDrawn,
			xwLeft - dxLToW,
			0,
			rclErase.xlRight,
			dywBrcTop);

	/* erase bits to the right of the PLDR */
	if(!FEmptyRc(PrcSet(&rclErase,
			rclErase.xlRight,	0,	pdrParent->dxl + pdrParent->dxpOutRight, dyl)))
		{
		ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
		}
}


#endif	/*DEBUGORNOTWIN*/

#ifdef MAC

/* %%Function:SetPenForBrc %%Owner:NOTUSED */
NATIVE SetPenForBrc(ww, brc, fHoriz, fFrameLines) /* WINIGNORE - MAC only */
int ww;
int brc;
BOOL fHoriz, fFrameLines;
{
	if (brc == brcNone && !fFrameLines)
		PenPat(&vqdg.patWhite);
	else  if (brc == brcNone || brc == brcDotted)
		PenPatGray(ww);
	else
		{
		Assert(brc == brcSingle || brc == brcHairline || brc == brcThick);
		Assert(brc != brcHairline || vlm != lmPrint || vprsu.prid != pridLaser);
		PenPat(&vqdg.patBlack);
		}

	if (brc == brcThick)
		{
		if (fHoriz)
			PenSize(1,2);
		else
			PenSize(2,1);
		}
	else
		PenSize(1,1);
}


#else /* WIN */
#ifdef	DEBUGORNOTWIN
/* %%Function:SetPenForBrc %%Owner:tomsax %%Reviewed:7/6/89 */
HANDNATIVE SetPenForBrc(ww, brc, fHoriz, fFrameLines)
int ww;
int brc;
BOOL fHoriz, fFrameLines;
{
	extern HBRUSH vhbrGray;

	switch (brc)
		{
	default:
		Assert(fFalse);
	case brcSingle:
	case brcHairline:
	case brcThick:
		hbrPenBrc = vsci.hbrText;
		break;
	case brcNone:
		if (!fFrameLines)
			{
			hbrPenBrc = vsci.hbrBkgrnd;
			break;
			}
		/* else fall through */
	case brcDotted:
		hbrPenBrc = vhbrGray;
		break;
		}

	dxpPenBrc = dxpBorderFti;
	dypPenBrc = dypBorderFti;

	if (brc == brcThick)
		{
		if (fHoriz)
			dypPenBrc = 2 * dypBorderFti;
		else
			dxpPenBrc = 2 * dxpBorderFti;
		}
}


#endif	/* DEBUGORNOTWIN */
#endif /* ! MAC */


/*	NOTE - this is somewhat up in the air at the moment, to make scrolling
/* work, we have to be able to undraw frame lines at the top and bottom
/* of the DR, which in turn means we are forced to draw them over top
/* of table frame lines/borders. See the following routine for the old version.
/**/
/* %%Function:ClearRclInParentDr %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT
ClearRclInParentDr(ww,hpldr,rcl,prcwClip)
int ww;
struct PLDR **hpldr;
struct RC rcl, *prcwClip;
{
	struct RC rcw;
#ifdef WIN
	struct RC rcwPage;
	HDC hdc = PwwdWw(ww)->hdc;
	struct WWD *pwwd;

#endif

	RclToRcw ( hpldr, &rcl, &rcw );
	if (prcwClip)
		SectRc(prcwClip, &rcw, &rcw);

	if (FEmptyRc(&rcw))
		return;

#ifdef WIN
	if ((pwwd = PwwdWw(ww))->fPageView)
		{
		RceToRcw(pwwd, &pwwd->rcePage, &rcwPage);
		SectRc(&rcw, &rcwPage, &rcw);
		}
	else
		{
		/* clip style name area if any */
		rcw.xwLeft = max(pwwd->rcwDisp.xwLeft, rcw.xwLeft);
		}
#endif	
	EraseRc (ww, &rcw);

}


#ifdef	REFERENCE
/*	NOTE - this is somewhat up in the air at the moment, to make scrolling
/* work, we have to be able to undraw frame lines at the top and bottom
/* of the DR, which in turn means we are forced to draw them over top
/* of table frame lines/borders.
/*
/* "Description": does a careful erase of a frame-relative DR in nested
/* hpldr. "Careful" means that any frame lines for the imbedded DR are not
/* erased, in fact they are redrawn without flicker.
/**/
/* %%Function:ClearRclInParentDr %%Owner:unused */
EXPORT
ClearRclInParentDr(ww,hpldr,fFrameLines,rcl)
int ww;
struct PLDR **hpldr;
BOOL fFrameLines;
struct RC rcl;
{
	int xwLeft, xwRight;
	int xwT, dxwT;
	struct PLDR **hpldrParent;
	struct RC rcwParent, rcw;
	struct DRF drfFetch;
#ifdef WIN
	struct RC rcDraw, rcwPage;
	HDC hdc = PwwdWw(ww)->hdc;
	struct WWD *pwwd;

#endif

	RclToRcw ( hpldr, &rcl, &rcw );

	if (fFrameLines)
		{
#ifdef MAC
		PenNormal();
		PenPatGray(ww);
#endif

		hpldrParent = (*hpldr)->hpldrBack;
		DrclToRcw(hpldrParent,
				&PdrFetchAndFree(hpldrParent,(*hpldr)->idrBack,&drfFetch)->drcl,&rcwParent);

		if (rcw.ywTop <= rcwParent.ywTop)
			{
			/* remove the top line from the rcw to be cleared */
			if ((rcw.ywTop = rcwParent.ywTop) + dyFrameLine > rcw.ywBottom)
				return;
			if ((xwT = rcw.xwLeft) < rcwParent.xwLeft)
				{
				/* erase bits to left of parent DR, remove that
				/* portion from the area remaining to clear.
				/**/
				xwRight = rcw.xwRight;	/* save for restore */
				rcw.xwRight = min(rcw.xwRight,rcwParent.xwLeft);
				EraseRc(ww, &rcw);
				xwT = rcw.xwLeft = rcwParent.xwLeft;
				rcw.xwRight = xwRight;	/* restore */
				}
			if ((dxwT = rcw.xwRight) > rcwParent.xwRight)
				{
				/* erase bits to right of parent DR, remove that
				/* portion from the area remaining to clear.
				/**/
				xwLeft = rcw.xwLeft;	/* save for restore */
				rcw.xwLeft = max(rcw.xwLeft,rcwParent.xwRight);
				EraseRc(ww, &rcw);
				dxwT = rcw.xwRight = rcwParent.xwRight;
				rcw.xwLeft = xwLeft;	/* restore */
				}
			if ((dxwT -= xwT) > 0)
				{
#ifdef MAC
				MoveTo(xwT, rcwParent.ywTop);
				Line(dxwT-1,0);
#else /* WIN */
				FillRect(hdc, (LPRECT)PrcSet(&rcDraw,
						xwT, rcwParent.ywTop,
						xwT + dxwT,
						rcwParent.ywTop + vsci.dypBorder),
						vhbrGray);
#endif
				}
			rcw.ywTop += dyFrameLine;
			}

		if (rcw.ywBottom >= rcwParent.ywBottom-dyFrameLine)
			if ((rcw.ywBottom=rcw.ywBottom-dyFrameLine) < rcw.ywTop)
				return;
			else
				{
				xwT = max(rcw.xwLeft,rcwParent.xwLeft);
				dxwT = min(rcw.xwRight,rcwParent.xwRight) - dxFrameLine - xwT;
				if (dxwT >= 0)
					{
#ifdef MAC
					MoveTo(xwT, rcwParent.ywBottom-dyFrameLine);
					Line(dxwT,0);
#else /* WIN */
					FillRect(hdc, (LPRECT)PrcSet(&rcDraw,
							xwT,
							rcwParent.ywBottom - dyFrameLine,
							xwT + dxwT,
							rcwParent.ywBottom - dyFrameLine + vsci.dypBorder),
							vhbrGray);
#endif
					}
				}

		if (rcw.xwLeft <= rcwParent.xwLeft)
			{
			xwRight = rcw.xwRight; /* save for restore */
			rcw.xwRight = min(rcw.xwRight,rcwParent.xwLeft);
			EraseRc(ww, &rcw);
			rcw.xwLeft = rcw.xwRight + dxFrameLine;
			if (rcw.xwLeft > xwRight)
				return;
			rcw.xwRight = xwRight;
#ifdef MAC
			MoveTo(rcwParent.xwLeft,rcw.ywTop);
			LineTo(rcwParent.xwLeft,rcw.ywBottom-1);
#else /* WIN */
			FillRect( hdc, (LPRECT)PrcSet(&rcDraw,
					rcwParent.xwLeft, rcw.ywTop,
					rcwParent.xwLeft + dxpBorderFti,
					rcw.ywBottom),
					vhbrGray );
#endif
			}

		if (rcw.xwRight >= rcwParent.xwRight - dxFrameLine)
			{
			xwLeft = rcw.xwLeft; /* save for restore */
			rcw.xwLeft = max(rcw.xwLeft,rcwParent.xwRight);
			EraseRc(ww, &rcw);
			rcw.xwRight = rcwParent.xwRight - dxFrameLine;
			if (xwLeft > rcw.xwRight)
				return;
			rcw.xwLeft = xwLeft;
#ifdef MAC
			MoveTo(rcw.xwRight,rcw.ywTop);
			LineTo(rcw.xwRight,rcw.ywBottom-1);
#else /* WIN */
			FillRect( hdc, (LPRECT)PrcSet(&rcDraw,
					rcw.xwRight,
					rcw.ywTop,
					rcw.xwRight + dxpBorderFti,
					rcw.ywBottom),
					vhbrGray );
#endif
			}
		PenNormal();
		}

#ifdef WIN
	if ((pwwd = PwwdWw(ww))->fPageView)
		{
		RceToRcw(pwwd, &pwwd->rcePage, &rcwPage);
		SectRc(&rcw, &rcwPage, &rcw);
		}
	else
		{
		/* clip style name area if any */
		rcw.xwLeft = max(pwwd->rcwDisp.xwLeft, rcw.xwLeft);
		}
#endif	
	EraseRc (ww, &rcw);

}


#endif	/*REFERENCE*/


/* D Y L  D R A W  T A B L E  T O P
/*
/* Description: Draws the area allocated above the cell DR's in a table row.
/* Returns: width of strip it drew; this width is taken care of completely
/* in both x-directions.
/*	NOTE: vtcc must be set up for the current row before calling this.
/**/
/* %%Function:DxyDrawTableTop %%Owner:tomsax %%Reviewed:7/6/89 */
DylDrawTableTop(ww,hpldr,prclDrawn,fFrameLines,fDrFrameLines,fFirstRow,fLastRow)
int ww;
struct PLDR **hpldr;
struct RC *prclDrawn;
BOOL fFrameLines, fDrFrameLines, fFirstRow, fLastRow;
{
	int dyl;
	struct DR *pdr;
	struct RC rcl, rcw;
	struct TCX tcx;
	struct DRF drfFetch;

	Assert(FInCa(caTap.doc, caTap.cpFirst, &vtcc.caTap));
	Assert(FInCa(vtcc.doc, vtcc.cpFirst, &vtcc.caTap));

	/* steal some information from RclToRcw() */
	rcl.ptTopLeft = 0L;
	RclToRcw(hpldr,&rcl,&rcw);

	dyl = DylDrawTblTop1( ww, fFirstRow, fLastRow, fFrameLines, vtcc.dylAbove,
			rcw.xwLeft /*dxLtoW*/, rcw.ywTop /*dyLtoW*/, prclDrawn);

	if (dyl == 0)
		{
		if (fDrFrameLines)
			{
			/* This nonsense is to ensure that if this row of the table is
			/* at the top of its parent DR then that portion of the outer DR's
			/* frame line gets drawn.
			/**/
			ItcGetTcxCache ( ww, caTap.doc, caTap.cpFirst, &vtapFetch, vtapFetch.itcMac, &tcx);
			PrcSet(&rcl,
					tcx.xpDrLeft - tcx.dxpOutLeft,
					0,
					tcx.xpCellRight,
					dyFrameLine);
			ClearRclInParentDr(ww,hpldr,rcl,NULL);
			}
		goto LExit;
		}

	/* clear bits to the left of the table border */
	pdr = PdrFetchAndFree((*hpldr)->hpldrBack,(*hpldr)->idrBack,&drfFetch);
	rcl.xlLeft = -pdr->dxpOutLeft;
	rcl.xlRight = prclDrawn->xlLeft;
	rcl.ylTop = 0;
	rcl.ylBottom = dyl;
	ClearRclInParentDr(ww,hpldr,rcl,NULL);

	/* clear bits to the right of the table */
	Break3();
	rcl.xlRight = pdr->dxl + pdr->dxpOutRight;
	rcl.xlLeft = prclDrawn->xlRight;
	rcl.ylTop = 0;
	rcl.ylBottom = dyl;
	ClearRclInParentDr(ww,hpldr,rcl,NULL);

LExit:
	return dyl;
}


/* D Y L  D R A W  T B L  T O P  1
/*	NOTE: vtcc must be set up for the current row before calling this.
/*
/* FUTURE: There are several blocks of code which do identical things
/* the the row above and below the vertical border we are filling in.
/* Many of the local variables in this routine could be lumped into
/* parallel structs, then passed into mini-routines that do the processing.
/* This is particularly true of the while loops which "maintain the
/* invariant" for the above and below rows.
/*
/**/
/* %%Function:DylDrawTblTop1 %%Owner:tomsax %%Reviewed:7/6/89 */
DylDrawTblTop1(ww,fFirstRow,fLastRow,fFrameLines,dylAbove,dxLtoW,dyLtoW,prclDrawn)
int ww;
BOOL fFirstRow, fLastRow, fFrameLines;
int dxLtoW, dyLtoW;
struct RC *prclDrawn;
{
	int xpMin, xpCur, xpMac;
	int itcAboveCur, itcAboveNext, itcAboveMac;
	int itcBelowCur, itcBelowNext, itcBelowMac;
	int xpAboveFirst, xpAboveLim, xpBelowFirst, xpBelowLim;
	int brcPrev = brcNil;
	int dxpMin;
	BOOL fDrawBrcAbove;
#ifdef WIN
#ifdef DEBUG
	CP cpFirstSav;
#endif /* DEBUG */
#endif /* WIN */
	struct RC rcw;
	int rgbrc[cbrcCorner];
	struct TCX tcxAbove, tcxBelow;

	Assert(FInCa(caTap.doc, caTap.cpFirst, &vtcc.caTap));
	Assert(FInCa(vtcc.doc, vtcc.cpFirst, &vtcc.caTap));

	itcBelowMac = vtapFetch.itcMac;
	if ( fFirstRow )
		{
		if ( dylAbove == 0 )
			goto LExit; /* nothing for us to draw */
		ItcGetTcx ( ww, &vtapFetch, vtapFetch.itcMac, &tcxBelow);
		xpMac = tcxBelow.xpDrLeft - tcxBelow.dxpOutLeft;
		itcAboveCur = itcAboveNext = itcAboveMac = 0;
		tcxAbove.xpCellRight = tcxAbove.xpCellLeft = xpMac;
		tcxAbove.brcLeft = tcxAbove.brcBottom = brcNil;
		itcBelowNext = ItcGetTcx ( ww, &vtapFetch, itcBelowCur=0, &tcxBelow);
		xpMin = xpCur = xpBelowFirst = tcxBelow.xpDrLeft - tcxBelow.dxpOutLeft - DxFromBrc(tcxBelow.brcLeft,fTrue);
		xpBelowLim = tcxBelow.xpDrRight + tcxBelow.dxpOutRight;
		xpAboveFirst = xpAboveLim = xpMac;
		}
	else
		{
#ifdef MAC
		itcAboveMac = vtcc.tapPrev.itcMac;
		ItcGetTcx ( ww, &vtcc.tapPrev, itcAboveMac, &tcxAbove);
#else /* WIN */
		Debug(cpFirstSav = caTap.cpFirst);
		CpFirstTap(caTap.doc, caTap.cpFirst-1);
		CpFirstTap(caTap.doc, caTap.cpLim);
		Assert(caTap.cpFirst == cpFirstSav);
		Assert (FInCa(caTap.doc, caTap.cpFirst-1, &caTapAux));
		itcAboveMac = vtapFetchAux.itcMac;
		ItcGetTcx ( ww, &vtapFetchAux, itcAboveMac, &tcxAbove);
#endif /* WIN */
		ItcGetTcxCache ( ww, caTap.doc, caTap.cpFirst, &vtapFetch, vtapFetch.itcMac, &tcxBelow);
		xpAboveLim = tcxAbove.xpDrLeft - tcxAbove.dxpOutLeft;
		xpBelowLim = tcxBelow.xpDrLeft - tcxBelow.dxpOutLeft;
		xpMac = max(xpAboveLim,xpBelowLim);

#ifdef MAC
		itcAboveNext = ItcGetTcx ( ww, &vtcc.tapPrev, itcAboveCur=0, &tcxAbove);
#else /* WIN */
		Assert (FInCa(caTap.doc, caTap.cpFirst-1, &caTapAux));
		itcAboveNext = ItcGetTcx ( ww, &vtapFetchAux, itcAboveCur=0, &tcxAbove);
#endif /* WIN */
		itcBelowNext = ItcGetTcxCache ( ww, caTap.doc, caTap.cpFirst, &vtapFetch, itcBelowCur=0, &tcxBelow);
		xpAboveFirst = tcxAbove.xpDrLeft - tcxAbove.dxpOutLeft - DxFromBrc(tcxAbove.brcLeft,fTrue);
		xpBelowFirst = tcxBelow.xpDrLeft - tcxBelow.dxpOutLeft - DxFromBrc(tcxBelow.brcLeft,fTrue);
		xpMin = xpCur = min(xpAboveFirst,xpBelowFirst);
		if ( dylAbove == 0 )
			{
			if (!fFrameLines)
				goto LExit; /* nothing for us to draw or clear */

			/* Here we have the possibility of needing to draw some frame
			/* lines. Since FrameTableLine has drawn the frame lines
			/* corresponding to the top line of each cell in the below row,
			/* the only ones left are frame lines for the bottoms of cells
			/* belonging to the top row which overhang the below row.
			/**/
			if (xpAboveFirst < xpBelowFirst)
				{
				/* need to draw frame lines for left-overhanging row above */
				dylAbove = dyFrameLine;
				rgbrc[ibrcTopButt] = rgbrc[ibrcDraw] = brcNone;
				rgbrc[ibrcLeftButt] = rgbrc[ibrcBottomButt] = brcNil;
				PrcSet(&rcw,
						xpAboveFirst,
						0,
						min(xpBelowFirst, xpAboveLim),
						dylAbove);
				TranslateRc(&rcw,&rcw,dxLtoW,dyLtoW);
				DrawBrc(ww,rgbrc,rcw,dbmodTop,fFrameLines);
				}
			if (xpAboveLim > xpBelowLim)
				{
				/* need to draw frame lines for left-overhanging row above */
				dylAbove = dyFrameLine;
				rgbrc[ibrcBottomButt] = rgbrc[ibrcDraw] = brcNone;
				rgbrc[ibrcLeftButt] = rgbrc[ibrcTopButt] = brcNil;
				PrcSet(&rcw,
						max(xpBelowLim,xpAboveFirst),
						0,
						xpAboveLim,
						dylAbove);
				TranslateRc(&rcw,&rcw,dxLtoW,dyLtoW);
				DrawBrc(ww,rgbrc,rcw,dbmodTop,fFrameLines);
				}
			if (dylAbove > 0
					&& (xpAboveLim < xpBelowFirst || xpBelowLim < xpAboveFirst)
					&& vlm != lmPrint && vlm != lmPreview)
				{
				/* The weirdest case of all; two "contiguous" rows do not
				/* intersect in x-space. We have to erase the bits which
				/* lie bewteen the two rows in x-space and are in our y-space.
				/**/
				PrcSet(&rcw,
						min(xpBelowLim,xpAboveLim),
						0,
						max(xpBelowFirst,xpAboveFirst),
						dylAbove);
				TranslateRc(&rcw,&rcw,dxLtoW,dyLtoW);
				EraseRc (ww, &rcw);
				}
			goto LExit; /* nothing left to draw */
			}
		if (xpCur < xpAboveFirst)
			{
			xpAboveLim = xpAboveFirst;
			itcAboveCur = -1; 
			itcAboveNext = 0;
			SetWords(tcxAbove.rgbrc,brcNone,cbrcTcx);
			}
		else
			xpAboveLim = tcxAbove.xpDrRight + tcxAbove.dxpOutRight;
		if (xpCur < xpBelowFirst)
			{
			xpBelowLim = xpBelowFirst;
			itcBelowCur = -1; 
			itcBelowNext = 0;
			SetWords(tcxBelow.rgbrc,brcNone,cbrcTcx);
			}
		else
			xpBelowLim = tcxBelow.xpDrRight + tcxBelow.dxpOutRight;
		}

	/* draw the border lines */
	for ( ; xpCur < xpMac; )
		{
		Break3();
		/* maintain the invariant for the Above row */
		while ( xpCur >= xpAboveLim )
			{
			if ( itcAboveNext < itcAboveMac )
				{
#ifdef MAC
				itcAboveNext = ItcGetTcx(ww,&vtcc.tapPrev,itcAboveCur=itcAboveNext,&tcxAbove);
#else /* WIN */
				Assert (FInCa(caTap.doc, caTap.cpFirst-1, &caTapAux));
				itcAboveNext = ItcGetTcx(ww,&vtapFetchAux,itcAboveCur=itcAboveNext,&tcxAbove);
#endif /* WIN */
				xpAboveFirst = xpAboveLim;
				xpAboveLim = tcxAbove.xpDrRight + tcxAbove.dxpOutRight;
				}
			else  if ( itcAboveNext == itcAboveMac )
				{
				xpAboveFirst = xpAboveLim;
				xpAboveLim += DxFromBrc(tcxAbove.brcRight,fTrue);
				tcxAbove.brcLeft = tcxAbove.brcRight;
				itcAboveCur = itcAboveNext++;
				}
			else
				{
				xpAboveFirst = xpAboveLim = xpMac;
				itcAboveCur = itcAboveNext;
				SetWords(tcxAbove.rgbrc,brcNone,cbrcTcx);
				}
			}

		/* maintain the invariant for the Below row */
		while ( xpCur >= xpBelowLim )
			{
			if ( itcBelowNext < itcBelowMac )
				{
				itcBelowNext = ItcGetTcxCache(ww,caTap.doc, caTap.cpFirst, &vtapFetch,itcBelowCur=itcBelowNext,
						&tcxBelow);
				xpBelowFirst = xpBelowLim;
				xpBelowLim = tcxBelow.xpDrRight + tcxBelow.dxpOutRight;
				}
			else  if ( itcBelowNext == itcBelowMac )
				{
				xpBelowFirst = xpBelowLim;
				xpBelowLim += DxFromBrc(tcxBelow.brcRight,fTrue);
				tcxBelow.brcLeft = tcxBelow.brcRight;
				itcBelowCur = itcBelowNext++;
				}
			else
				{
				xpBelowFirst = xpBelowLim = xpMac;
				itcBelowCur = itcBelowNext;
				SetWords(tcxBelow.rgbrc,brcNone,cbrcTcx);
				}
			}

		/* calculate info for next segment to be drawn */
		Break3();
		rcw.xlLeft = xpCur;

		if (xpCur < xpBelowFirst && xpCur < xpAboveFirst)
			{
			/* we've just jumped into the void between two rows,
				erase the corresponding area */
			Assert(itcBelowCur > itcBelowMac && itcAboveCur == -1
					|| itcAboveCur > itcAboveMac && itcBelowCur == -1 );
			rcw.xlRight = xpCur = min(xpBelowFirst,xpAboveFirst);
			if (vlm != lmPrint && vlm != lmPreview)
				{
				rcw.ylTop = 0;
				rcw.ylBottom = dylAbove;
				TranslateRc(&rcw,&rcw,dxLtoW,dyLtoW);
				EraseRc(ww, &rcw);
				}
			continue;	/* with for loop */
			}

		/* decide which brc is going to get drawn into the current segment */
		fDrawBrcAbove = fFalse;	/* assume we're drawing for the below row */
		if (itcBelowCur < itcBelowMac && (xpBelowFirst == xpCur || itcAboveCur >= itcAboveMac))
			{
			if ((rgbrc[ibrcDraw] = tcxBelow.brcTop) == brcNone)
				if (itcAboveCur < itcAboveMac)
					{
					fDrawBrcAbove = fTrue;
					rgbrc[ibrcDraw] = tcxAbove.brcBottom;
					}
			}
		else
			{
			rgbrc[ibrcDraw] = brcNone;
			if (itcAboveCur < itcAboveMac)
				if ((rgbrc[ibrcDraw] = tcxAbove.brcBottom) != brcNone)
					fDrawBrcAbove = fTrue;
				else  if (itcBelowCur < itcBelowMac)
					rgbrc[ibrcDraw] = tcxBelow.brcTop;
			}

		/* Determine if there are connecting borders above or below the end of
		/* the current segment. If so store them in the rgbrc[] and advance
		/* enough to ensure that there is enough room to draw the connection.
		/* This is needed to avoid drawing successive corner connections on top
		/* of each other.
		/**/
		dxpMin = 0;
		if (itcAboveCur <= itcAboveMac && xpCur == xpAboveFirst)
			{
			rgbrc[ibrcTopButt] = tcxAbove.brcLeft;
			dxpMin = DxFromBrc(tcxAbove.brcLeft,fFrameLines);
			}
		else
			rgbrc[ibrcTopButt] = brcNil;

		if (itcBelowCur <= itcBelowMac && xpCur == xpBelowFirst)
			{
			rgbrc[ibrcBottomButt] = tcxBelow.brcLeft;
			dxpMin = max(dxpMin,DxFromBrc(tcxBelow.brcLeft,fFrameLines));
			}
		else
			rgbrc[ibrcBottomButt] = brcNil;

		rgbrc[ibrcLeftButt] = brcPrev;
		brcPrev = rgbrc[ibrcDraw];

		/* Ideally, we advance to the next place a border starts. However if
		/* doing so doesn't give us enough room to draw the (potential) corner
		/* connection for this joint, we pass it up.
		/**/
		if ((rcw.xlRight = min(xpAboveLim,xpBelowLim)) < xpCur + dxpMin
				&& (rcw.xlRight = fDrawBrcAbove?xpAboveLim:xpBelowLim) < xpCur + dxpMin)
			{
			/* this little hack forces the corner we are going to so
			/* much trouble to draw to get priority in the border
			/* matching code...
			/**/
			if ((rgbrc[ibrcBottomButt] == rgbrc[ibrcTopButt] && rgbrc[ibrcTopButt] != brcNil)
					|| (rgbrc[ibrcTopButt] == rgbrc[ibrcLeftButt]
					|| rgbrc[ibrcBottomButt] == rgbrc[ibrcLeftButt])
					&& rgbrc[ibrcLeftButt] != brcNil)
				rgbrc[ibrcDraw] = brcNil;
			rcw.xlRight = xpCur+dxpMin;
			}
		xpCur = rcw.xlRight;
		rcw.ylTop = 0;
		rcw.ylBottom = dylAbove;
		TranslateRc(&rcw,&rcw,dxLtoW,dyLtoW);
		DrawBrc(ww,rgbrc,rcw,dbmodTop,fFrameLines);
		}

LExit:
	if (prclDrawn != NULL)
		PrcSet(prclDrawn, xpMin, 0, xpMac, dylAbove);
	return dylAbove;
}


/*	B R C  D R A W  O N E  B O T T O M  B O R D E R
/*
/*	Description: Draw one border at the bottom of a table cell.
/*  Needs to have the TCX description of the cell, plus the
/*  previous bottom border (use brcNil for the first cell's brcPrev).
/*
/*  RETURNS - brc drawn for use in successive calls
/**/
/* %%Function:BrcDrawOneBottomBorder %%Owner:tomsax %%Reviewed:7/6/89 */
BrcDrawOneBottomBorder(ww,rcw,ptcx,brcPrev,fLastCorner,fFrameLines)
int ww;
struct RC rcw;
struct TCX *ptcx;
int brcPrev;
BOOL fLastCorner, fFrameLines;
{
	int rgbrc[cbrcCorner];

	if (ptcx->xpCellLeft < ptcx->xpCellRight || fLastCorner)
		rgbrc[ibrcDraw] = ptcx->brcBottom;
	else
		return brcNil;

	rgbrc[ibrcLeftButt] = brcPrev;
	rgbrc[ibrcTopButt] = ptcx->brcLeft;
	rgbrc[ibrcBottomButt] = brcNil;
	DrawBrc ( ww, rgbrc, rcw, dbmodTop, fFrameLines);

	return rgbrc[ibrcDraw];
}


/* %%Function:TranslateRc %%Owner:tomsax %%Reviewed:7/6/89 */
TranslateRc(prcSrc,prcDst,dx,dy)
struct RC *prcSrc, *prcDst;
int dx, dy;
{
	PrcSet(prcDst,
			prcSrc->xlLeft + dx,
			prcSrc->ylTop + dy,
			prcSrc->xlRight + dx,
			prcSrc->ylBottom + dy);
}


/*	P R I N T  T A B L E  B O R D E R S  */
/*
/*	Description: Draw table borders for printing. Differs from normal
/*	drawing routines in that bits not covered by border lines do not
/*	need to be erased, and frame lines need not be drawn.
/*
/* NOTE: caTap and vtcc must be set up for the table row.
/**/
/* %%Function:PrintTableBorders %%Owner:tomsax %%Reviewed:7/6/89 */
PrintTableBorders(ww, xlLeft, ylTop, dylText, dylAbove, dylBelow, fFirstRow, fLastRow)
int xlLeft, ylTop; /* upper left corner of table row */
int dylText; /* the total height of the table row */
int dylAbove, dylBelow;
BOOL fFirstRow, fLastRow;
{
	int itc, itcMac;
	int brcPrev;
	int ylBottom;
	int dxBrc;
	BOOL fTtpCell;
	struct RC rcl;
	int rgbrc[cbrcCorner];
	struct TCX tcx;

	/* Keep our callers honest, verify that the table caches make sense */
	Assert(FInCa(vtcc.doc, vtcc.cpFirst, &caTap));
	Assert(vtcc.fXpValid && vtcc.fDylValid);
	Assert(fFirstRow || !vtcc.fFirstRow);
	Assert(fFirstRow || FInCa(caTap.doc, caTap.cpFirst-1,
		WinMac(&caTapAux, &vtcc.caTapPrev)));

	AssertDo(DylDrawTblTop1(ww, fFirstRow, fLastRow, fFalse/*fFrameLines*/,
			dylAbove,xlLeft/*dxLtoW*/,ylTop/*dyLtoW*/,NULL/*prclDrawm*/)==dylAbove);

	itcMac = vtapFetch.itcMac;
	brcPrev = brcNil;
	/* Now, sports fans, we build a rectangle that covers the table
	/* row, excluding any top border area drawn by DylDrawTblTop1.
	/* The area allocated for the bottom is also excluded, and stashed
	/* away for use later.
	/**/
	ylBottom = 
			(rcl.ylBottom = (rcl.ylTop = ylTop += dylAbove) + dylText) + dylBelow;

	if (dylBelow == 0)
		fLastRow = fFalse;	/* no point in drawing a 0-height bottom border */

	itc = 0;
	do	
		{
		fTtpCell = itc == itcMac;
		itc = ItcGetTcxCache(ww,caTap.doc,caTap.cpFirst,&vtapFetch,itc,&tcx);
		/* Recall that rcw's are arranged to leave space for vertical frame
		/* lines, hence we ask for DxFromBrc to allow for them to get the rcw's
		/* right, then call DrawBrc with fFrameLines = fFalse so that
		/* they don't actually get drawn.
		/**/
		dxBrc = DxFromBrc(tcx.brcLeft,fTrue/*fFrameLines*/);
		rcl.xlRight = xlLeft + tcx.xpDrLeft - tcx.dxpOutLeft;
		rcl.xlLeft = rcl.xlRight - dxBrc;
		rgbrc[ibrcDraw] = tcx.brcLeft;
		DrawBrc ( ww, rgbrc, rcl, dbmodVertical, fFalse/*fFrameLines*/);
		if (fLastRow)
			{
			rcl.ylTop = rcl.ylBottom;
			rcl.ylBottom = ylBottom;
			rcl.xlRight = xlLeft + tcx.xpDrRight + tcx.dxpOutRight;
			brcPrev = BrcDrawOneBottomBorder(ww,rcl,&tcx,brcPrev,fTtpCell/*fLastCorner*/,fFalse/*fFrameLines*/);

			/* restore the rect for the next cell */
			rcl.ylBottom = rcl.ylTop;
			rcl.ylTop = ylTop;
			}
		} 
	while (!fTtpCell);

}


/* %%Function:RcwPgvTableClip %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT
RcwPgvTableClip(ww, hpldr, idr, prcwClip)
int ww;
struct PLDR **hpldr; /* should be HwwdWw, the parent DR */
int idr;             /* the parent idr */
struct RC *prcwClip;
{
	struct DRC drcp;
	struct DRF drfFetch;
	struct DR *pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);

	drcp.ptDxDy = pdr->drcl.ptDxDy;
	/* Make it a drcp rather than a drcl */
	drcp.xp = -pdr->dxpOutLeft;
	drcp.yp = 0;
	drcp.dxp += pdr->dxpOutRight - drcp.xp;
	ClipRectFromDr(HwwdWw(ww), hpldr, idr, &drcp, prcwClip);

}


/* D R A W  T A B L E  R E V  B A R */
/* %%Function:DrawTableRevBar %%Owner:tomsax %%Reviewed:7/6/89 */
export DrawTableRevBar(ww, idr, dl)
int ww, idr, dl;
{
	int xwRevBar, irmBar;
	struct WWD **hwwd;
	struct DR *pdr;
	struct RC *prcwClip;
	struct RC rcwDr, rcwClip;
	struct EDL edl;
	struct DRF drf;

	pdr = PdrFetch(hwwd = HwwdWw(ww), idr, &drf);
	if ((irmBar = PdodMother(pdr->doc)->dop.irmBar) == irmBarNone)
		goto LFreeDrf;

	GetPlc(pdr->hplcedl, dl, &edl);
	/* keep our callers honest... */
	Assert(edl.hpldr != hNil && (*(edl.hpldr))->hpldrBack == hwwd);

	if (!(*hwwd)->fPageView)
		{
		prcwClip = NULL;
		xwRevBar = (*hwwd)->xwSelBar + dxwSelBarSci / 2;
		}
	else
		{
		Assert(PwwdWw(ww)->fPageView);
		prcwClip = &rcwClip;
		RcwPgvTableClip(ww, hwwd, idr, prcwClip);
		DrclToRcw(hwwd, &pdr->drcl, &rcwDr);

		switch (irmBar)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			/* fall through */
#endif
		case irmBarOutside:
			if (vfmtss.pgn & 1 && FFacingPages(DocMother(pdr->doc)))
				goto LRight;
			/* fall through */
		case irmBarLeft:
			xwRevBar = rcwDr.xwLeft - (dxwSelBarSci / 2);
			break;
		case irmBarRight:
LRight:
			xwRevBar = rcwDr.xwRight + (dxwSelBarSci / 2);
			break;
			}
		}
	DrawRevBar(PwwdWw(ww)->hdc, xwRevBar, YwFromYl(edl.hpldr,0), edl.dyp, prcwClip);
LFreeDrf:
	FreePdrf(&drf);
}


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Disptbl_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Disptbl_Last() */
