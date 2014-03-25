/* D I S P 3 . C */

#ifdef MAC
#define	COLOR
#define CONTROLS
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
#include "cmd.h"

#ifdef MAC 
#include "help2.h"
#include "mac.h"
#endif


/* E X T E R N A L S */

extern int		vfColorQD;
extern int              vlm;
extern int              vfSeeSel;
extern int              wwCur;
extern int		vssc;
extern struct WWD       **hwwdCur;
extern struct WWD       **mpwwhwwd[];
extern struct SCC       *vpsccBelow;
extern struct SEP       vsepFetch;
extern struct FLI       vfli;
extern struct CA        caSect;
extern struct PAP       vpapFetch;
extern struct CA	caTap;
extern struct PREF      vpref;
extern struct CA        caPara;
extern struct FMTSS     vfmtss;
extern int              wwMac;
extern struct SCC       vsccAbove;
extern int				vwwhgd;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern struct SEL			selMacro;
extern CP               cpInsert;
extern BOOL		vfInFormatPage;
extern struct CHP	vchpStc;
extern struct CHP	vchpFetch;
extern CHAR HUGE	*vhpchFetch;

extern struct SAB       vsab;
extern struct UAB       vuab;

extern int              vfLastScrollDown;
extern int              vfLastScroll;


#ifdef MAC
extern int		wwScrap;
extern int              ftcLast;
extern int		vcbppScc;
extern long             LPixMapSize();
extern struct BITMAP far *far *vqqpm;
extern GDHANDLE			vhgdev;
extern GDHANDLE			GetDeviceWw();
extern int wwHelp;
extern int		cHpFreeze;
extern int		vdocHelpView;
#endif

#ifdef WIN
extern struct SCI       vsci;
extern struct FTI       vfti;
extern int		vflm;
extern int		dypCS;
extern HRGN		vhrgnGrey;
extern HRGN		vhrgnPage;
#endif

#ifdef DEBUG
extern struct DBS       vdbs;
extern int              vcTestx;
extern int              vfCheckPlc;
extern struct DRF	*vpdrfHead;
#endif /* DEBUG */

extern BOOL             vfEndPage;
extern struct MERR      vmerr;

extern long LDist2RcPt();


/* short-circuits UpdateWw until file problems have been resolved */
/* int vfInFileAlert = fFalse; NEVER SET TRUE */

/* store a hpldr in hopes of reusing it */
#define     ihpldrMax	2
int         vihpldrMac = 0;
struct PLDR **vrghpldr[ihpldrMax];


#ifdef WIN
/* %%Function:UpdateAllWws %%Owner:bryanl */
UpdateAllWws( fAbortOK )
int fAbortOK;
{
	int ww;
	struct WWD *pwwd;

/* Update current window first */

	if (wwCur != wwNil)
		{
		Assert (mpwwhwwd [wwCur] != hNil);
		/* go through FExecCmd so that special mode get terminated.
		   undo, repeat, recording are also taken care of.  FLegalBcm
		   will allow PageView if current window is already in page view */
		if ((*hwwdCur)->fPageView && (*hwwdCur)->ipgd == ipgdNil)
			FExecCmd(bcmPageView);

		UpdateWw(wwCur, fAbortOK);
		if (PwwdWw(wwCur)->fDirty)
			return; /* update has been interrupted */
		}

/* Update all document windows */

	for (ww = wwDocMin; !(fAbortOK && FMsgPresent(mtyUpdateWw)) && ww < wwMac; ww++)
		if (mpwwhwwd[ww] && ww != wwCur) /* window exists */
			{
			if ((pwwd = PwwdWw(ww))->wk == wkClipboard)
				{
/* clipboard window should get painted through the CLIPBRD.EXE application */
            /* FUTURE (bobz 10/30/89) the better thing to do would be to destroy
               the mw for the clipboard in DestroyClip, and set vwwClipboard to nil.
               Since we are near release and it is not clear that we would
               catch all the uses, we are doing this simpler thing.
               We check owner becauses if we had created the window, then lost
               ownership (pby copying from another app), the ww is not destroyed
               and we will keep pumping out invalidate rects for the clipboard
               whenever we updateww and the rcwInval is nonzero. We expect Paint
               Clipboard to undirty the window, but we never get it so keep
               invalidating forever...
            */
	           if (vsab.fOwnClipboard)
				    InvalidateRect( pwwd->hwnd, (LPRECT)&pwwd->rcwInval, fFalse );
				}
			else
				{
				if (pwwd->fPageView && pwwd->ipgd == ipgdNil)
					CmdPageViewOff(ww);
				UpdateWw(ww, fAbortOK);
				if (PwwdWw(ww)->fDirty)
					return; /* update has been interrupted */
				}
			}
}


/* U P D A T E  I N V A L I D  W W */
/* Find out what Windows considers to be the invalid range of
	the passed window.  Mark it invalid in WWD & blank the area on the screen */

/* %%Function:UpdateInvalidWw  %%Owner:chic */
UpdateInvalidWw(ww, prcInval)
int ww;
struct RC *prcInval; /* return inval rect in here */
{
	struct WWD *pwwd;
	HANDLE hwnd;
	struct RC rcwStrip;
	BOOL fResetRcToEmpty = fFalse;

	FreezeHp();
	pwwd = PwwdWw(ww);
	hwnd = pwwd->hwnd;
	GetUpdateRect( hwnd, (LPRECT) prcInval, fTrue /* fErase */);

	if (!pwwd->fPageView)
		{
		/* paint the tiny strip above ywMin if invalid */
		SetRect( &rcwStrip, 0, 0, pwwd->xwMac, pwwd->ywMin );
		if (FSectRc( &rcwStrip, prcInval, &rcwStrip ))
			PatBltRc( pwwd->hdc, &rcwStrip, vsci.ropErase );

		/* if scrolled, DisplayFli won't erase the sel bar if invalid; do so here */
		PrcSelBar( ww, &rcwStrip );
		if (pwwd->xhScroll != 0 && FSectRc( &rcwStrip, prcInval, &rcwStrip ))
			PatBltRc( pwwd->hdc, &rcwStrip, vsci.ropErase );

		/* draw portion of style name window border that extends above ypMin */
		/* This is the only drawing/invalidation that we have to do in this area */
		if (pwwd->xwSelBar)
			PaintStyStub( ww, prcInval );
		prcInval->ypTop = max( pwwd->ywMin, prcInval->ypTop );
		}
	if (prcInval->ypTop < prcInval->ypBottom)
		{
		InvalWwRc(ww, prcInval);
		if (pwwd->fPageView)
			{
			fResetRcToEmpty = pwwd->fBackgroundDirty;
			/* got inval rect, force DrawBlankPage */
			pwwd->fBackgroundDirty = fTrue;
			}
		}

	/* Since we have found out the invalid rect, and marked it invalid
	in our structures, we don't want to hear about it again,
	so we tell windows that we have made everything valid */
	ValidateRect( hwnd, (LPRECT) NULL );
	
	{
	/* HACK (bl) if called from FCreateMw, make invalid rect empty to support
	endmark draw optimizations.plcedl state will force apropriate draws anyway */
	extern int vywEndmarkLim;
	if (!pwwd->fPageView && vywEndmarkLim == 0)
		SetWords( &pwwd->rcwInval, 0, sizeof (struct RC)/sizeof(int) );
	}

	if (fResetRcToEmpty)
		/* although we have an invalid rect but if the backgroundirty bit
		is already on, we really want DrawBlankPage to erase the whole page,
		e.g. dls covered by a dialog box are now pushed to the next page,
		so part of these old dls will not be cleaned up. 
		*/
		SetBytes(prcInval, 0, sizeof(struct RC));

	MeltHp();
}
#endif /* WIN */


/* U P D A T E  W W */
/* Redisplay ww as necessary.  */
/* %%Function:UpdateWw %%Owner:bryanl */
UpdateWw(ww, fAbortOK)
{
/* The display consists of layers:                      drawn by:
	BackMost:    gray outside of page (page view only)   DrawBlankPage
		white page background (page view only)  DrawBlankPage
		white dr background                     EraseInDr
		dr text                                 DisplayFliCore
		dr borders (page view & tables only)    FrameDrLine
		margins (page view only)                FrameMarginsWw
	Frontmost:   selection highlighting                  FMarkLine
*/
	struct WWD *pwwd, **hwwd = HwwdWw(ww);
	int idr, idrMac, idrFlow, ipgd;
	int dlMac;
	BOOL fCpBad;
	struct DR *pdr;
	struct PLCEDL **hplcedl;
#ifdef WIN
	int xwLimScroll = 0;
	struct RC rcInval;
	BOOL fUseRcInval;
#endif
	CP cp, cpMac, cpMacDoc;
	struct DRF drf;
	int matOld = vmerr.mat;

	AssertH(hwwd);
#ifdef WIN
	UpdateInvalidWw(ww, &rcInval); /* mark what WINDOWS think is invalid */
	fUseRcInval = !FEmptyRc(&rcInval);
#endif

/* vfInFormatPage ON means we are recursing in background repagination --
	page 1 was recalculated and DRs were dirty, page 2 is being laid out */
	if (!(*hwwd)->fDirty || vfInFormatPage /*|| vfInFileAlert NEVER SET TRUE */)
		return;

	if (FWindowHidden(ww))
		{
		(*hwwd)->fDirty = fFalse;
		return;
		}
#ifdef MAC
	/* See if we have the right number of colors in the cache */
	if (vfColorQD)
		{
		int bppT;

		if (vsccAbove.ww == wwCur && (bppT = BitsPerPixelWw(vsccAbove.ww)) != 0 &&
				vcbppScc != bppT)
			{
			struct RC rcT;

			ClearSccs();
			SynchSccWw(&vsccAbove, wwCur);
			SynchSccWw(vpsccBelow, wwCur);
			vcbppScc = bppT;
			}
		(*hwwd)->bppCur = BitsPerPixelWw(ww);
		if ((vhgdev = GetDeviceWw(ww)) != 0L)
			vwwhgd = ww;
		else
			vwwhgd = wwNil;
		}
	ftcLast = -1;  /* blow font cache since it may have changed as a
	result of an update event */
#endif /* MAC */

#ifdef WIN
	if ((*hwwd)->grpfvisi.flm != vflm)
		{
		SetFlm( (*hwwd)->grpfvisi.flm );
		}
#endif

	Scribble(ispUpdateWw, 'U');

	pwwd = PwwdSetWw(ww, cptDoc);
	if (!FEmptyRc(&pwwd->rcwInval))
		{
		pwwd->xwLimScroll = max(pwwd->xwLimScroll, pwwd->rcwInval.xwRight);
		}

#ifdef MAC
	if (!FScrollOK(ww))
/* updating a window while not on the top:
invalidate any bitmap caches of this window.
*/
		InvalidateDbcWw(ww);
	if (vfLastScroll)
		{
		SetBmb(fFalse);
		vfLastScroll = fFalse;
		}
#endif

#ifdef WIN
/* If updating a macro window, we will lose any animation selection (since
 * the update just looks at selCur, not selMacro), so turn off selMacro here
 * to avoid thinking that it is still on. */
	if (selMacro.doc != docNil && selMacro.ww == ww)
		{
		TurnOffSel(&selMacro);
		}
#endif

/* can not maintain the redundant position information of the cursor */
	if (selCur.ww == ww)
		ClearInsertLine(&selCur);
	if (selDotted.ww == ww)
		ClearInsertLine(&selDotted);

	FreezeHp();
	pwwd = *hwwd;
	if (!pwwd->fPageView)
/* window dirty => dummy dr dirty */
		PdrGalley(pwwd)->fDirty = fTrue;
	else
		{
		struct PGD pgd;
		if (pwwd->fDrDirty)
			{
			MeltHp();
			/* DRs need to be recreated */
LDrBad:
			ipgd = IpgdPldrFromIpgd(ww, pwwd->ipgd);
			FreezeHp();
			pwwd = *hwwd;
			pwwd->fBackgroundDirty = fFalse;
			Mac(pwwd->fFullSizeUpd = fTrue);
			WinMac(DrawBlankPage(ww, fUseRcInval ? &rcInval : NULL), DrawBlankPage(ww));
			Mac(SetBmb(fFalse));
			if (ipgd == ipgdNil)
				{
				MeltHp();
				FreeDrs(hwwd, 1);
				(*hwwd)->fDirty = fFalse;
				(*hwwd)->ipgd = ipgdNil;	/* get out of page view */
				return;	/* out of memory */
				}
			}
		else  if (pwwd->fBackgroundDirty)
			{
			WinMac(DrawBlankPage(ww, fUseRcInval ? &rcInval : NULL), DrawBlankPage(ww));
			pwwd->fBackgroundDirty = fFalse;
			Mac(SetBmb(fFalse));
			}
		/* bind page number */
		GetPlc(PdodDoc(DocBaseWw(ww))->hplcpgd, (*hwwd)->ipgd, &pgd);
		vfmtss.pgn = pgd.pgn;
		}

	idrMac = pwwd->idrMac;
	MeltHp();
/* clean up cpFirst's for all dr's */
	if (idrMac > 0)
		pdr = PdrWw(ww, 0);

	for (idr = 0, cpMac = cpNil; idr < idrMac; idr++, pdr++)
		if (pdr->fNoPrev && pdr->fCpBad)
			{
			if ((*hwwd)->fPageView)
				{
				vuab.docInvalPageView = DocBaseWw(ww);
				goto LDrBad;
				}
			vfLastScrollDown = fTrue;
			vfSeeSel = fFalse;
			pdr->dypAbove = 0;
			(*hwwd)->fSetElev = fTrue;
/* create plcedl if there is none */
			if (pdr->hplcedl == hNil)
				{
				if (!FInitHplcedl(1, pdr->cpFirst, hwwd, idr))
					goto LRet;
				}
			ScrollDown2(ww, idr, 0, dysMacAveLineSci, fFalse);
			PwwdSetWw(ww, cptDoc);	/* SynchSccWw's can change */
			break;
			}
	vfEndPage = fFalse;
/* update each dr */
	for (idr = 0; idr < idrMac; idr++)
		{
		int f;

		idrFlow = ((struct DR *)PInPl(hwwd, idr))->idrFlow;
		/* if can't abort, we have to calculate true info by forcing udmodNormal
			or else udmodLastDr will delete dirty edl that is not visible when
			it has no dr to flow to */ 
#ifdef MAC
		if (!FUpdateDr(ww, hwwd, idr, (*hwwd)->rcwInval, fAbortOK,
				!fAbortOK || idrFlow >= 0 ? udmodNormal : udmodLastDr, cpNil))
			break;
#else
		f = FUpdateDr(ww, hwwd, idr, (*hwwd)->rcwInval, fAbortOK,
				!fAbortOK || idrFlow >= 0 ? udmodNormal : udmodLastDr, cpNil);
#ifdef BRYANL
			{
			int xw = xwLimScroll;
#endif
			Win( xwLimScroll = max( xwLimScroll, 
					((struct DR *)PInPl(hwwd,idr))->xwLimScroll ));
#ifdef BRYANL
			if (xw != xwLimScroll)
				CommSzNum( SzShared( "WW xwLimScroll increased in UpdateWw to: "), xwLimScroll );
			}
#endif
		if (!f)
			break;
#endif	/* WIN */
		if (idrFlow < 0)
			continue;
/* check match with successor dr */
		Assert(idrFlow < idrMac);
		pdr = PInPl(hwwd, idr);
		/* we know hplcedl exists or FUpdateDr would have returned fFalse */
		hplcedl = pdr->hplcedl;
		dlMac = IMacPlc(hplcedl);
		if (vfEndPage)
			cp = cpMac = cpNil;
		else  if ((cp = CpPlc(hplcedl, dlMac)) == cpNil)
			/* a dr with cpFirst=cpNil intervened; try
				the cp at which we stopped before */
			cp = cpMac;
		else
			/* save stopping point */
			cpMac = cp;
		pdr = PInPl(hwwd, idrFlow);
		FreezeHp();
		if (fCpBad = pdr->cpFirst != cp)
			{
			if (cp != cpNil)
				{
				cpMacDoc = CpMacDoc(pdr->doc);
#ifdef MAC
				while (cp < cpMacDoc)
					{
					CachePara(pdr->doc, cp);
					/* reproduce the algorithm used by layout to
						skip PostScript paras */
					if (vpapFetch.stc == stcPostScript &&
							!vpref.fSeeHidden && vchpStc.fVanish)
						{
						while (vpapFetch.stc == stcPostScript &&
								!FInTableVPapFetch(caPara.doc,caPara.cpFirst))
							{
							cp = caPara.cpLim;
							if (cp >= cpMacDoc)
								goto LSetCpBad;
							CachePara(caPara.doc, cp);
							}
						}
					if (!vpref.fSeeHidden)
						{
						FetchCp(caPara.doc, cp, fcmProps);
						if (vchpFetch.fVanish)
							{
							MeltHp();
							pdr = PdrFetch(hwwd, idr, &drf);
							Mac(vfli.fStopAtPara = fTrue);
							while (fTrue)
								{
								FormatLineDr(ww, cp, pdr);
								if (vfli.ichMac > 0 || !vfli.fParaStopped
										|| (cp = vfli.cpMac) >= cpMacDoc)
									break;
								}
							Mac(vfli.fStopAtPara = fFalse);
							FreePdrf(&drf);
							FreezeHp();
							pdr = PInPl(hwwd, idrFlow);
							if (cp >= cpMacDoc)
								goto LSetCpBad;
							CachePara(pdr->doc, cp);
							}
						}
					if (!FAbsPap(caPara.doc, &vpapFetch))
						break;
					while (FAbsPap(caPara.doc, &vpapFetch))
						{
						if (FInTableVPapFetch(caPara.doc,caPara.cpFirst))
							{
							CpFirstTap(caPara.doc, caPara.cpFirst);
							cp = caTap.cpLim;
							}
						else
							cp = caPara.cpLim;
						if (cp >= cpMacDoc)
							goto LSetCpBad;
						CachePara(caPara.doc, cp);
						}
					}
LSetCpBad:
#else
				CachePara(pdr->doc, cp);
				while (FAbsPap(caPara.doc, &vpapFetch))
					{
					if (FInTableVPapFetch(caPara.doc,caPara.cpFirst))
						{
						CpFirstTap(caPara.doc, caPara.cpFirst);
						cp = caTap.cpLim;
						}
					else
						cp = caPara.cpLim;
					if (cp >= cpMacDoc)
						break;
					CachePara(caPara.doc, cp);
					}
#endif /* MAC */
				fCpBad = pdr->cpFirst != cp;
				}
			pdr->fCpBad = fCpBad;
			}
		MeltHp();
		if (pdr->fCpBad)
/* successor dr exists, does not match desired cp */
			{
			Assert(pdr->doc > 0);
			pdr->fCpBad = fFalse;
			if (pdr->cpFirst == cpNil && pdr->cpLim != cpNil && cp >= pdr->cpLim)
				goto LSetDcpDepend;
			pdr->cpFirst = cp;
			pdr->cpLim = cpNil;
			pdr->fDirty = fTrue;
			if (cp == cpNil)
				goto LSetDcpDepend;
			CachePara(pdr->doc, cp);
			CacheSect(pdr->doc, cp);
			pdr = PInPl(hwwd, idrFlow);
			if (vpapFetch.fPageBreakBefore)
				{
				/* can't have PBB except in 1st dr */
				pdr->cpFirst = cpNil;
				}
			else  if (pdr->fNoParaStart)
				{
				/* can't accept para start */
				if (cp == caPara.cpFirst)
					pdr->cpFirst = cpNil;
/* win breaks on page breaks in 2-column text also */
				else  if (WinMac(fTrue, vsepFetch.ccolM1 == 0))
					{
					/* block page breaks in 2nd half of chain */
					FetchCp(caSect.doc, cp - ccpSect, fcmChars);
					pdr = PInPl(hwwd, idrFlow);
					if (*vhpchFetch == chSect)
						pdr->cpFirst = cpNil;
					}
				}
			else  if (cp == caSect.cpFirst)
				{
				/* check for dr intolerant to
					section start */
				if ((pdr->fNewColOnly && vsepFetch.bkc != bkcNewColumn) ||
						vsepFetch.ccolM1 != pdr->ccolM1 ||
						vsepFetch.dxaColumns != pdr->dxaBetween)
					{
					pdr->cpFirst = cpNil;
					}
				}
/* update dcpDepend under all circumstances */
LSetDcpDepend:
			if (cp == cpNil || dlMac == 0)
				pdr->dcpDepend = 0 ;
			else
				{
				struct EDL edl;
				GetPlc(hplcedl, dlMac - 1, &edl);
				pdr->dcpDepend = edl.dcpDepend;
				}
			}
		}
LRet:
	FreezeHp();
	pwwd = *hwwd;
	/* avoid continuously trying to update if we don't have enough memory */
	if (vmerr.mat == matMem && matOld != vmerr.mat)/* oom during this update*/
		vmerr.mat = matDisp; /* substitute matMem by matDisp to get correct msg */
	if (vmerr.mat == matDisp)
		{
		pwwd->fDirty = fFalse;
		Win( pwwd->xwLimScroll = 0x7FFF );
		}
	if (idr == idrMac)  /* completely updated */
		{
/* draw the 4 corner margins in pageview */
		if (pwwd->fPageView && FDrawPageDrsWw(ww))
			FrameMarginsWw(ww);
/* reset invalid indications */
		pwwd->fDirty = pwwd->fDrDirty && pwwd->fPageView;
		pwwd->ywFirstInval = ypMaxAll;
		pwwd->ywLimInval = 0; /* so that max in InvalBand will work */
		Mac( pwwd->fFullSizeUpd = fFalse );
		pwwd->fNewDrs = fFalse;
		Win( pwwd->xwLimScroll = xwLimScroll );
/* update cpFirst of sccBelow */
#ifdef MAC
		if (!pwwd->fPageView && vpsccBelow) SccCpFirst(HwwdWw(ww));
#endif	/* MAC */
		}
#ifdef WIN
	else
		pwwd->xwLimScroll = max( xwLimScroll, pwwd->xwLimScroll );
#endif		
	MeltHp();

	Mac(SetBmb(fTrue));
	if (selCur.ww == ww && vssc != sscNil)
		ShowInsertLine(&selCur);
#ifdef MAC
	if (DocBaseWw(ww) == vdocHelpView)
		PwwdSetWw(ww, cptAll);	/* restore help dlg clipping */
#endif
	Scribble(ispUpdateWw, ' ');
}


#ifdef	WIN
/* D R A W   B L A N K   P A G E */
/* %%Function:DrawBlankPage %%Owner:chic %%reviewed 7/10/89 */
DrawBlankPage(ww, prcInval)
int ww;
struct RC *prcInval;
{
	struct WWD *pwwd;
	HDC hdc;
	int iLevel;
	int iRet;
	struct RC rcwDisp, rcwPage, rcwClip;


/* get region from display rectangle */
	pwwd = PwwdSetWw(ww, cptDoc);
	hdc = pwwd->hdc;
	rcwDisp = pwwd->rcwDisp;

/* set region from page rect */
	RceToRcw(pwwd, &pwwd->rcePage, &rcwPage);
	if (prcInval != NULL)
		SectRc(prcInval, &rcwPage, &rcwClip);
	else
		rcwClip = rcwPage;
	InflateRect((LPRECT)&rcwPage, vsci.dxpBorder, vsci.dypBorder);
	Assert (vhrgnGrey != NULL && vhrgnPage != NULL);
	SetRectRgn(vhrgnPage, rcwPage.xwLeft, rcwPage.ywTop, rcwPage.xwRight, rcwPage.ywBottom);
	SetRectRgn(vhrgnGrey, rcwDisp.xwLeft, rcwDisp.ywTop, rcwDisp.xwRight, rcwDisp.ywBottom);

	iRet = CombineRgn(vhrgnGrey/*dest*/, vhrgnGrey/*src1*/, vhrgnPage/*src2*/, RGN_DIFF);
	if (iRet != ERROR && iRet != NULLREGION)
		FillRgn(hdc, vhrgnGrey, vsci.hbrDesktop);

/* clear ip before erase page, in case someone later called ClearInsertLine which would have inverted a ghost cursor */
	ClearInsertLine(PselActive());

/* erase the page to white */
	if (!FEmptyRc(&rcwClip))
		PatBltRc(hdc, &rcwClip, vsci.ropErase);
/* draw the page border */
	FrameRect(hdc, (LPRECT) &rcwPage, vsci.hbrBorder);


}
#endif	/*WIN*/

/* S C R O L L  D R  D O W N */
/* ypFirstShow must be visibility limit (not necessarily dr limit)
*/
/* %%Function:ScrollDrDown %%Owner:bryanl */
NATIVE ScrollDrDown(ww, hpldr, idr, hplcedl, dlFrom, dlTo, ypFrom, ypTo, ypFirstShow,
ypLimWw, ypLimDr)
struct PLDR **hpldr;
struct PLCEDL **hplcedl;
{
	int ypLim, dypChange, ypTopNew;
	int dl, dlMac, dlDest, dlMacOld, dlMax, dlStart;
	int ddlChange = dlTo - dlFrom;
	struct EDL edl;
	struct PLDR **hpldrT;
	struct DR *pdr;
	struct WWD *pwwd = PwwdWw(ww);
	BOOL fNoLimSet = pwwd->fPageView;
	int ywT, ypT;
	int idrT;
	struct DRF drfFetch;

	dypChange = ypTo - ypFrom;
	Assert(dypChange >= 0);

	if ((*hpldr)->hpldrBack != hNil)
		{
		/* Check the dr above for ypLimDr */
		hpldrT = (*hpldr)->hpldrBack;
		idrT = (*hpldr)->idrBack;
		pdr = PdrFetchAndFree(hpldrT, idrT, &drfFetch);
		ypT = pdr->dyl;
		ywT = YwFromYp(hpldrT, idrT, ypT);
		ypT = YpFromYw(hpldr, idr, ywT);
		if (ypT < ypLimDr)
			ypLimDr = ypT;
		}

	ypLim = min(ypLimWw, ypLimDr) - dypChange;

	if ((dlMacOld = IMacPlc(hplcedl)) + ddlChange > (dlMax = (*hplcedl)->dlMax - 1))
		FreeEdls(hplcedl, dlMax - ddlChange, dlMacOld);

	PutIMacPlc(hplcedl,
			dlMac = min(dlMacOld + ddlChange, dlMax));
	dlDest = dlMac;
	dl = dlDest - ddlChange;
	PutCpPlc(hplcedl, dlDest--, CpPlc(hplcedl, dl--));

	for (dlStart = dl; dl >= dlFrom; dl--, dlDest--)
		{
/* copy edl at dl to dlDest (dlDest < dl) 
In pageview mode, the bottom of the last line which will be fully copied 
within the dr will be found and stored in ypLim. Lines below ypLim will
not be moved (to ypTopNew).
*/
		PutCpPlc(hplcedl, dlDest, CpPlc(hplcedl, dl));
		GetPlc(hplcedl, dl, &edl);
		if (!fNoLimSet)
			ypTopNew = edl.ypTop + dypChange;
		else
			{
			ypTopNew = edl.ypTop;
			if (edl.ypTop + edl.dyp + dypChange <= ypLimDr)
				{
				fNoLimSet = fFalse;
				ypLim = min(ypLim, edl.ypTop + edl.dyp);
				ypTopNew += dypChange;
				}
			}
		if (edl.hpldr != hNil)
			{
			hpldrT = edl.hpldr;
			edl.hpldr = hNil;
			PutPlc(hplcedl, dl, &edl);
			edl.hpldr = hpldrT;
			(*hpldrT)->ptOrigin.yp = ypTopNew;
			}
		if (edl.ypTop >= ypFirstShow)
			ypFrom = edl.ypTop;
		else  if (edl.ypTop + edl.dyp + dypChange > ypFirstShow)
			edl.fDirty = fTrue;
		edl.ypTop = ypTopNew;
#ifdef WIN	/* optimize out scroll of portion of endmark that's
		all background anyway */
				if (edl.fEnd)
			{
#ifdef BRYANL
			CommSzNum( SzShared( "Scrolling endmark down, scroll ht reduced by: "), ypLim - edl.ypTop - 10 * vsci.dypBorder );
#endif
			ypLim = min( ypLim, edl.ypTop + 10 * vsci.dypBorder );
			}
#endif
		PutPlc(hplcedl, dlDest, &edl);
		}
	/* Clean out dangling handles */
	for (dl = dlFrom ; dl < dlTo ; dl++)
		{
		GetPlc(hplcedl, dl, &edl);
		edl.hpldr = hNil;
		PutPlcLast(hplcedl, dl, &edl);
		}
	Win(dypCS = dypChange);
	if (dypChange != 0 && ypLim > ypFrom && dlStart >= dlFrom)
		ScrollWw(ww, hpldr, idr, ypFrom,
				ypFrom + dypChange, ypLim - ypFrom);
}


/* D L K  F R O M  V F L I */
/* decides on kind of dlk which vfli describes. Also stores dlk in edl */
/* %%Function:DlkFromVfli %%Owner:bryanl */
NATIVE DlkFromVfli(hplcedl, dl)
struct PLCEDL **hplcedl;
{
#ifdef REFERENCE
	/* when we make a distinction between page and column eject,
		this will make sense; for now, any column/page eject just ends
		the dr */
	int dlk = vfli.cpMac == caSect.cpLim ?
	dlkEndDr : dlkEndPage;
#else
	int dlk = dlkEndDr;
#endif
	struct EDL edl;

	GetPlc(hplcedl, dl, &edl);
	edl.dlk = dlk;
	PutPlcLast(hplcedl, dl, &edl);
	return dlk;
}


/* T R A S H  S C C  F L I */
/* %%Function:TrashSccFli %%Owner:bryanl */
TrashSccFli()
{
	ClearSccs();
	InvalFli();
}


/* C L E A R  S C C S */
/* %%Function:ClearSccs %%Owner:bryanl */
ClearSccs()
{
	ClearSccAbove();
#ifdef MAC
	if (vpsccBelow)
		ClearSccsLarge();
#endif	/* MAC */
}


/* C L E A R  S C C  A B O V E */
/* %%Function:ClearSccAbove %%Owner:bryanl */
ClearSccAbove()
{
	PutIMacPlc(vsccAbove.hplcedl, 0);
	vsccAbove.fFull = fFalse;
	vsccAbove.fBreak = fFalse;
	vsccAbove.ywMac = 0;
}


/* I N V A L  B A N D */
/* invalidate the band ywFirst, ywLim */
/* %%Function:InvalBand %%Owner:bryanl */
InvalBand(ww, ywFirst, ywLim)
int ww;
int ywFirst, ywLim;
{
	struct WWD *pwwd = PwwdWw(ww);
	struct RC rc;

#ifdef	MAC
/* the If covers some peculiar rects received from update event after a
window resize by 1 pixel. CS */
	if (ywLim < 0 || ywFirst == ywLim) return;
#endif	/* MAC */

	rc.xwLeft = 0;
	rc.xwRight = pwwd->xwMac;
	rc.ywTop = ywFirst;
	rc.ywBottom = ywLim;

	InvalWwRc(ww, &rc);
}


/* I N V A L  W W  R C */
/* Marks window area prcwInval as dirty.
*  Three levels of dirtyness are set:
*       wwd.fDirty, dr.fDirty, and edl.fDirty.
*  This routine only invalidates things that are visible within rcwDisp.
*/
/* %%Function:InvalWwRc %%Owner:bryanl */
InvalWwRc(ww, prcwInval)
struct RC *prcwInval;  /* invalid rect in window relative units */
{
	int idrMac;
	int idr;
	struct DR *pdr;
	struct WWD **hwwd = HwwdWw(ww);
	struct WWD *pwwd = *hwwd;
	struct PLCEDL **hplcedl;
	int dl, dlMac;
	struct RC rcwInval, rclInval, rcpInval;
	struct RC rclDr;
	struct RC rcpEdl;
	struct EDL edl;
#ifdef WIN
	struct RC rcDummy;
#endif /* WIN */

/* affect all edl's, not just displayable ones - page view edl's must be 
	invalidated whether visible or not */
	rcwInval = *prcwInval;
	if (FEmptyRc(&rcwInval))
		{
#ifdef WIN /* if only style name area is exposed */
		if (!pwwd->fPageView && pwwd->xwSelBar > 0 && pwwd->xwMac <= pwwd->xwSelBar)
			{
			rcwInval.xpRight = pwwd->xwMin+1; /* force invalidate */
			}
		else
#endif
			return;  /* rcwInval does not intersect with rcwDisp */
		}

	pwwd->fDirty = fTrue;

	/* grow wwd.rcwInval to include new rcwInval */
	UnionRc(&pwwd->rcwInval, &rcwInval, &pwwd->rcwInval);

	/* convert rcwInval into page coordinates */
	RcwToRcl(hwwd, &rcwInval, &rclInval);

	/* invalidate dr's that intersect with rclInval */
	pdr = pwwd->rgdr;
	for (idrMac = pwwd->idrMac, idr = 0; idr < idrMac; ++idr, ++pdr)
		{
		if (pdr->hplcedl == hNil)
			continue;
		DrcToRc(&pdr->drcl, &rclDr);
		rclDr.xwLeft -= pdr->dxpOutLeft;
		rclDr.xwRight += pdr->dxpOutRight;
#ifdef MAC
		if (!FSectRc(&rclDr, &rclInval))
#else /* WIN */
			if (!FSectRc(&rclDr, &rclInval, &rcDummy))
#endif /* WIN */
				continue;
		/* dr is (at least partially) invalid */
		pdr->fDirty = fTrue;
		RcwToRcp(hwwd, idr, &rcwInval, &rcpInval);
		/* invalidate edl's that intersect with rclInval */
		dlMac = IMacPlc(hplcedl = pdr->hplcedl);
		for (dl = 0; dl < dlMac; ++dl)
			{
			GetPlc(hplcedl, dl, &edl);
			/* see if edl intersects with rcwInval */
			DrcToRc(&edl.drcp, &rcpEdl);
#ifdef MAC
			if (FSectRc(&rcpEdl, &rcpInval))
#else /* WIN */
				if (FSectRc(&rcpEdl, &rcpInval, &rcDummy))
#endif /* WIN */
					{
				/* this edl is invalid */
					edl.fDirty = fTrue;
					PutPlcLast(hplcedl, dl, &edl);
					}
			} /* next dl */
		} /* next dr */
}


#ifdef WIN /* Located in util.c if MAC */
/* U N I O N  R C  - produce union of rc1 and rc2 in rcDest */
/* rcDest may be the same rectangle as either rc1 or rc2 */
/* %%Function:UnionRc %%Owner:bryanl */
UnionRc(prc1, prc2, prcDest)
struct RC *prc1, *prc2, *prcDest;
{
	prcDest->ypTop = min(prc1->ypTop, prc2->ypTop);
	prcDest->ypBottom = max(prc1->ypBottom, prc2->ypBottom);
	prcDest->xpLeft = min(prc1->xpLeft, prc2->xpLeft);
	prcDest->xpRight = max(prc1->xpRight, prc2->xpRight);
}


#endif	/* WIN */


/* C P  A C T I V E  C U R */
/* cp at active end of selCur */
/* %%Function:CpActiveCur %%Owner:bryanl */
CP CpActiveCur()
{
	struct SEL *psel = PselActive();
	CP cp = psel->cpFirst;
	int doc = psel->doc;
/* this if is protecting CpLimBlock below which should take a psel */
	if (psel->fTable && psel != &selCur)
		return psel->cpLim - 1;
	if (psel->fBlock)
		return(CpMin(CpLimBlock(), CpMacDocEdit(doc)));
	if (psel->fForward)
		{
		cp = psel->cpLim;
		if ((!psel->fIns || psel->fInsEnd) && cp != cp0)
			cp--;
		}
	return cp;
}


/* I D R  F R O M  H P L D R  D O C  C P */
/* returns idr of dr containing doc, cp in hpldr. If flag is set,
cp is known to be contained in the hpldr (this makes the search simpler.)
Completes update of incomplete dr's if needed.
*/
/* %%Function:IdrFromHpldrDocCp %%Owner:chrism */
IdrFromHpldrDocCp(hpldr /* hwwd */, ww, doc, cp, fAssumeIn, fVisibleOnly)
struct PLDR **hpldr;
int ww, doc;
CP cp;
BOOL fAssumeIn;
{
	int idr, idrMac, idrIn;
	int fPageView;
	CP cpT;
	struct DR *pdr;
	struct WWD *pwwd;
	struct PLDR ** hpldrBack = (*(struct WWD **) hpldr)->hpldrBack;
	struct DRF drfFetch;

/* quick return for outer DRs in galley */
	if (!(pwwd = PwwdWw(ww))->fPageView)
		{
		if (pwwd->idrMac == 0)
			return idrNil;
		if (hpldrBack == hNil)
			return(0);
		}
	if (pwwd->fDrDirty)
		return idrNil;

/* find the best candidate -- absolute dr's win if the cp is in two dr's */
	idrMac = (*hpldr)->idrMac;
	idrIn = idrNil;
	for (idr = 0; idr < idrMac; idr++)
		{
		pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
		if (doc == pdr->doc && pdr->cpFirst != cpNil &&	cp >= pdr->cpFirst)
			{
			if (pdr->lrk != lrkAbs || hpldrBack != hNil)
			/* DRs are in cp order, so last fit is best */
				idrIn = idr;
			else  if (cp < pdr->cpLim)
				{
				/* cpLim is dependable only for APOs which are not inner DRs */
				idrIn = idr;
				break;
				}
			}
		}

/* now we have best fit for cp; see if it's within bounds */
	Assert(idrIn != idrNil || !fAssumeIn);
	if (!fAssumeIn && idrIn != idrNil)
		{
		pdr = PdrFetch(hpldr, idrIn, &drfFetch);
		if (pdr->fCpBad)
			{
			/* UpdateWw needed */
			FreePdrf(&drfFetch);
			return idrNil;
			}
		if (pdr->hplcedl != hNil && cp < CpMacPlc(pdr->hplcedl))
			{
			FreePdrf(&drfFetch);
			return idrIn;
			}
		if (pdr->fIncomplete && !fVisibleOnly)
			{
#ifdef MAC
			struct WDS wds;

			SaveWds(&wds);
			PwwdSetWw(ww, cptAll);
#endif
			pdr->fDirty = fTrue;
			if (!FUpdateDr(ww, hpldr, idrIn, PwwdWw(ww)->rcwInval,
					fFalse /* abort not allowed*/,
					udmodNoDisplay, cp))
				{
				FreePdrf(&drfFetch);
				Mac(RestoreWds(&wds));
				return idrNil; /* out of memory */
				}
			Mac(RestoreWds(&wds));
			}
		if (pdr->hplcedl == hNil || cp >= CpMacPlc(pdr->hplcedl))
			if (!pdr->fFatLine || !pdr->fCantGrow)
				idrIn = idrNil;
		FreePdrf(&drfFetch);
		}
	return idrIn;
}


/* D L  W H E R E  D O C  C P */
/* returns cpFirst in hpldr, idr, dl of line containing doc, cp */
/* kludge alert: fInner may be 2 for a call from EndInsert, meaning that the
caller wants pdr->fIncomplete to be blocked in IdrFromHpldrDocCp */
/* %%Function:DlWhereDocCp %%Owner:chrism */
DlWhereDocCp(ww, doc, cp, fEnd, phpldr, pidr, pcpFirst, pfChangedPage, fInner)
CP cp; 
struct PLDR ***phpldr; 
int *pidr; 
CP *pcpFirst; 
BOOL *pfChangedPage;
BOOL fInner;
{
	int idr, ipgd;
	int dl;
	struct WWD *pwwd;
	struct DR *pdr;
	struct PLDR **hpldr = HwwdWw(ww);
	struct PLCEDL **hplcedl;
	CP cpT;
	struct EDL edl;
	struct DRF drfFetch;

	dl = dlNil;

	if (pfChangedPage != NULL)
		*pfChangedPage = fFalse;
	if ((idr = IdrFromHpldrDocCp(hpldr, ww, doc,
			cpT = cp - ((cp != cp0 && fEnd) ? 1 : 0), fFalse, fInner == 2/* kludge */)) == idrNil)
		{
		if (pfChangedPage == NULL || !PwwdWw(ww)->fPageView)
			return dlNil;
/* cp is on a different page so must determine ipgd of page containing cp,
	calculate new DRs describing that page and then determine the idr of the
	DR containing cp. */
		*pfChangedPage = fTrue;
		if ((ipgd = IpgdPldrFromDocCp(ww, doc, cpT)) == ipgdNil)
			return dlNil;
		if ((idr = IdrFromHpldrDocCp(hpldr, ww, doc, cpT, fFalse, fFalse)) == idrNil)
			return dlNil;
		}

LGetDl:	
	if ((dl = DlFromCpCheck(hpldr, idr, cp, fEnd)) < 0)
		return dlNil;
	hplcedl = (pdr = PdrFetch(hpldr, idr, &drfFetch))->hplcedl;
	GetPlc(hplcedl, dl, &edl);
	if (edl.hpldr != hNil && fInner && 
			(dl != 0 || !((struct WWD *)(*hpldr))->fOutline ||
			cp < pdr->cpFirst || cp >= CpPlc(hplcedl, 0)))
		{
		hpldr = edl.hpldr;
		idr = IdrFromHpldrDocCp(hpldr, ww, doc, cpT, fTrue, fFalse);
		FreePdrf(&drfFetch);
		if (idr == idrNil)
			return dlNil;
		goto LGetDl;
		}
	if (pcpFirst != NULL && dl != dlNil)
		*pcpFirst = CpPlc(hplcedl, dl);
	FreePdrf(&drfFetch);
	*pidr = idr;
	*phpldr = hpldr;
	return dl;
}


/* D L  F R O M  C P  C H E C K */
/* Returns dl containing cp, or -1 if cp is not visible.
Note; also used with pscc's */
/* %%Function:DlFromCpCheck %%Owner:chrism */
DlFromCpCheck(hpldr, idr, cp, fEnd )
struct PLDR **hpldr; 
int idr; 
CP cp;  
int fEnd;
{
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	int dlReturn;
	struct EDL edl;
	struct DRF drfFetch;
	BOOL fOutline;

	if (idr >= (*hpldr)->idrMac)
		return(dlNil);	/* failsafe for page view problems; 0 may not exist */
	pdr = PdrFetch(hpldr, idr, &drfFetch);
	hplcedl = pdr->hplcedl;

	if (hplcedl == hNil)
		{
		/* can happen when ending insert and a window is hidden */
		dlReturn = dlNil;
		goto LRet;
		}

/* situation may arise in outline mode. */
	fOutline = (*hpldr)->hpldrBack == hNil &&
			((struct WWD *)(*hpldr))->fOutline;

	if (fOutline && cp >= pdr->cpFirst && cp < CpPlc(hplcedl, 0))
		dlReturn =  0;
	else
		{
		CP cpT;
		dlReturn = IInPlcCheck(hplcedl, cpT = cp - (fEnd ? 1 : 0));
		if (dlReturn != dlNil && !fOutline)
			{
			GetPlc(hplcedl, dlReturn, &edl);
			if (cpT >= CpPlc(hplcedl, dlReturn) + edl.dcp)
				dlReturn = dlNil;
			}
		}

LRet:
	FreePdrf(&drfFetch);
	return dlReturn;
}



#ifdef BOGUS
/* C R E A T E  E D L S  T I L L  C P */
/* %%Function:CreateEdlsTillCp %%Owner:NOTUSED */
CreateEdlsTillCp(ww, idr, cp, pdl)
int ww;
int idr;
CP cp;
int *pdl;
{
	CP cpCur;
	int dl;
	int ypTop;
	int fFillBefore = fFalse;
	struct RC *prce;
	struct WWD *pwwd = PwwdWw(ww);
	struct DR *pdr = PdrWw(ww, idr);
	struct PLCEDL **hplcedl = pdr->hplcedl;
	int doc = pdr->doc;
	int dyl = pdr->dyl;
	struct RC rcePageOrig;
	struct EDL edl;
#ifdef DEBUG
	int fCheckPlcSave = vfCheckPlc;
#endif
	Debug(vfCheckPlc = fFalse);

/* first backup original page rectangle and then temporarily change page
	rectangle so no part of it is visible so that the calls below to
	DisplayFli will do nothing but record the edls produced. */
	rcePageOrig = pwwd->rcePage;
	prce = &pwwd->rcePage;
	MoveRc(prce, prce->xeLeft, pwwd->rcePage.yeTop - pwwd->rcePage.yeBottom);

	cpCur = CpPlc(hplcedl, dl = IMacPlc(hplcedl));
	Assert(cp >= cpCur);

	ypTop = (dl == 0) ? 0 : (GetPlc(hplcedl, dl - 1, &edl), edl.ypTop + edl.dyp);

	*pdl = -1;
	for ( ; ypTop < dyl; dl++)
		{
		if (dl >= (*hplcedl)->dlMax - 1)
			{
			PutIMacPlc(hplcedl, (*hplcedl)->dlMax - 1);
			if (!FOpenPlc(hplcedl, dl, 1))
				break;
			InitPlcedl(hplcedl, dl, 1);
			}
		FormatLine(ww, doc, cpCur);
		ypTop += vfli.dypLine;
		DisplayFli(ww, HwwdWw(ww), idr, dl, ypTop);
		GetPlc(hplcedl, dl, &edl);
		edl.fDirty = fTrue;
		PutPlcLast(hplcedl, dl, &edl);
		PutIMacPlc(hplcedl, dl + 1);
		if ((cpCur += edl.dcp) > cp)
			{
			*pdl = dl;
			break;
			}
		}
	PwwdWw(ww)->rcePage = rcePageOrig;
	Debug(vfCheckPlc = fCheckPlcSave);
}


#endif

/* F  C P  V I S I B L E */
/* returns true iff cp is visible in window pwwd */
/* %%Function:FCpVisible %%Owner:bryanl */
FCpVisible(ww, doc, cp, fEnd, fHoriz, fAnyVisible)
CP cp; 
BOOL fEnd, fHoriz, fAnyVisible;
{
	int idr;
	int xw;
	int dl;
	struct WWD *pwwd;
	struct PLDR **hpldr;
	struct PLCEDL **hplcedl;
	CP cpFirst;
	struct EDL edl;
	struct RC rcw;
	struct DRF drfFetch;

/* this can return dlNil due to out-of-memory; fFalse OK in that case */
	if ((dl = DlWhereDocCp(ww, doc, cp, fEnd,
			&hpldr, &idr, &cpFirst, NULL, fTrue)) == dlNil)
		return fFalse;

	pwwd = PwwdWw(ww);
	hplcedl = PdrFetch(hpldr, idr, &drfFetch)->hplcedl;
	GetPlc(hplcedl, dl, &edl);
	FreePdrf(&drfFetch);
	DrcpToRcw(hpldr, idr, &edl.drcp, &rcw);
	if (!FYwInWw(pwwd, rcw.ywBottom, rcw.ywTop, fAnyVisible))
		return fFalse;
	if (fHoriz)
		{
/* check if line dl is wider than the screen or scrolling is in effect
in which case a more thorough check will have to be made. */
		if (rcw.xwLeft < pwwd->rcwDisp.xwLeft ||
				rcw.xwRight > pwwd->rcwDisp.xwRight)
			{
/* call FormatLine and determine precise xp position of cp */
			xw = XwFromWwCp(ww, hpldr, idr, doc, cp, cpFirst);
			pwwd = PwwdWw(ww);
			if (xw < pwwd->rcwDisp.xwLeft ||
					xw > pwwd->rcwDisp.xwRight)
				return fFalse;
			}
		}
	return fTrue;
}


/* X W  F R O M  W W  C P */
/* returns xp of left edge of cp in ww (must be vertically visible!) */
/* %%Function:XwFromWwCp %%Owner:bryanl */
int XwFromWwCp(ww, hpldr, idr, doc, cp, cpFirst)
int ww; 
int doc; 
CP cp, cpFirst;
struct PLDR **hpldr;
{
	int xpT, ichT;
	struct DRF drfFetch;

	if (PwwdWw(ww)->fOutline)
		{
		struct PLC **hplcpad = PdodDoc(doc)->hplcpad;
		struct PAD pad;
		if (hplcpad != hNil)
			{
			GetPlc(hplcpad, IInPlc(hplcpad, cp), &pad);
			if (!pad.fShow)
/* neutral return, pretend that xw is visible (cp won't be made any more
visible just by scrolling than it already is) */
				return PwwdWw(ww)->rcwDisp.xwLeft;
			}
		}
	PwwdSetWw(ww, cptSame);	/* ensure valid drawing environment */
	FormatLineDr(ww, cpFirst, PdrFetch(hpldr, idr, &drfFetch));
	FreePdrf(&drfFetch);
	return XwFromXp(hpldr, idr,
			XpFromDcp(cp0, cp - vfli.cpMin, &xpT, &ichT));
}


/* N O R M  C P */
/* normalize document in window to make cp visible.
ncp contains independent mode bits:
	ncpForceYPos: force cp to given position in window even if already visible.
	ncpHoriz: scroll window horizontally if necessary to make cp visible.
	ncpAnyVisible: it is satisfactory to have any part of the line
		to be visible at all.
Display line containing cp so that ypWanted (dyp+ypMin) is within the line and
the display starts on a line boundary, or if that is not possible,
show the line starting at ypWanted from the top and let the first line be
split where it may.
Beep if none of this is possible because of document limits and do
the best you can.

Plan:
	1. update window if necessary.
	2. is cp in the window?
		yes: !fForce => return;
		ypTop = actual top of line, dyp = line height.
		dypScroll = ypTop - ypWant;
		if dypScroll < 0
			ScrollDown(wwCur, max(0, -dypScroll - dyp), -dypScroll)
		else
			ScrollUp(wwCur, dypScroll, dypScroll + dyp);
		return;
	3. cp is not in the window.
	if ypWanted = 0 => ThumbToCp(cp)
	ClearSccs();
	we need to do an invisible scroll down on the screen to ypWanted
	after a virtual thumb to cp.
	set wwdCur.cpFirst = cp
	DypScanAbove(ypWanted - ypMin of wwdCur, ..)
	4. normalize horizontally if desired.
		find cpFirst of line
		Format line to find xp of cp
		scroll horizontally to show xp in window.
*/
/* %%Function:NormCp %%Owner:bryanl */
NormCp(ww, doc, cp, ncp, dyw, fEnd)
int ww; 
int doc; 
CP cp; 
int ncp; 
int dyw; 
int fEnd;
{
	struct WWD **hwwd = HwwdWw(ww), *pwwd;
	struct PLCEDL **hplcedl;
	int ywWanted;
	int ywTop;
	int ypBottom;
	int dypLine;
	int fFatLine; 
	int idr, idrT;
	int dl, dlT;
	int xw, xwT, xeHome, dxeScroll;
	int ywT, yeHome, dyeScroll;
	int ywBottom, ywBottomNew;
	struct PT ptT;
	BOOL fPageView = (*hwwd)->fPageView;
	int dyl; 
	BOOL fNormHoriz;
	BOOL fChangedPage;
	struct PLDR **hpldr, **hpldrT;
	struct DR *pdr;
	BOOL fNotInRc, fHome;

	struct RC rcw, rcwDr; 
	CP cpFirst;
	struct SPT spt;
	struct EDL edl;
	struct DRF drfFetch;
	int iRepeat = 0;

#ifdef MAC
	if ((wwCur == wwScrap) && vsab.fExt)
		return;
#endif /* MAC */
	if (fPageView)
		fNormHoriz = FNormHoriz(ww);

LRepeat:
	if ((*hwwd)->fDirty && !(fPageView && (*hwwd)->fDrDirty))
		{
		UpdateWw(ww, fFalse);
		Assert(iRepeat == 0 || !(*hwwd)->fDrDirty);
		if (iRepeat > 0 && (*hwwd)->fDrDirty)
/* let idle pick up, prevent infinite loop within here */
			return;
		}
	if (iRepeat > 3)
		return;
	if ((*hwwd)->fOutline && FOutlineEmpty(ww, fFalse))
		return;

	dl = DlWhereDocCp(ww, doc, cp, fEnd, &hpldr, &idr, &cpFirst,
			&fChangedPage, fTrue);

	if (vmerr.fMemFail)
		return;


	ywWanted = (*hwwd)->ywMin + dyw;
	if (fPageView)
		ywWanted += dypMinWwInitSci;
	if (dl >= 0)
		{
		pdr = PdrFetch(hpldr, idr, &drfFetch);
/* we want to detect if dl is past the bottom boundary of the DR. if so we
   want to use the last visible dl in the DR for our norming. */
		DrclToRcw(hpldr, &pdr->drcl, &rcw);
 		fFatLine = pdr->fFatLine; 
		FreePdrf(&drfFetch);

		ptT.xw = rcw.xwLeft;
		ptT.yw = rcw.ywBottom - 1;

		if (!FWindowHidden(ww) && fFatLine &&
			(dlT = DlWherePt(ww, &ptT, &hpldrT, &idrT, &spt, fTrue, fTrue)) != 0)
			{
			if (dlT != dlNil && hpldrT == hpldr && idrT == idr && dlT < dl)
				dl = dlT;
			}
		pdr = PdrFetch(hpldr, idr, &drfFetch);
/* if dl is only partially visible at the bottom of the window, make
it visible */
		if (ncp & ncpVisifyPartialDl && !fPageView &&
				(*hpldr)->hpldrBack == hNil)
			{
			
			hplcedl = pdr->hplcedl;
			if (dl == IMacPlc(hplcedl) - 1 && dl > 0)
				{
				GetPlc(hplcedl, dl, &edl);
				ypBottom = edl.ypTop + edl.dyp;
				if (ypBottom > pdr->dyl)
					{
					FreePdrf(&drfFetch);
					ScrollUp(ww, dysMinAveLineSci, dysMacAveLineSci);
					iRepeat++;
					goto LRepeat;
					}
				}
			
			}
		ywTop = YwTopForDl(hpldr, idr, dl, &ywBottom);
		dyeScroll = ywTop - ywWanted;
/* if our cp is in the last dl of a table DR, we want to treat the bottom of
   the DR as the bottom of the dl. */
		if (!fPageView && (*hpldr)->hpldrBack != hNil &&
			dl == IMacPlc(pdr->hplcedl) - 1)
			{
			DrclToRcw(hpldr, &pdr->drcl, &rcwDr);
			ywBottomNew = rcwDr.ywTop + DylMacForTableDr(pdr->cpFirst, hpldr); 

			if (ywBottomNew - ywTop > (*hwwd)->ywMac - (*hwwd)->ywMin)
				ywBottom = ywBottomNew; 
			}
		FreePdrf(&drfFetch);

/* calculate amount to scroll by */
		pwwd = *hwwd;
		if (fPageView)
			{
			yeHome = YeTopPage(ww);
/* position of top if we scrolled to Home position */
			ywT = ywTop + yeHome - pwwd->yeScroll;
			fHome = (ywT >= pwwd->ywMin &&
					(ywT + ywBottom - ywTop) < pwwd->ywMac);
			}
		fNotInRc = !FYwInWw(pwwd, ywBottom, ywTop,
				ncp & ncpAnyVisible);
		if (fChangedPage)
			{
			if (fHome)
				dyeScroll = pwwd->yeScroll - yeHome;
			/* scroll page rect vertically as requested. */
			SetPageDisp(ww, IpgdCurrentWw(ww),
					fNotInRc ? max(pwwd->yeScroll - dyeScroll,
					YeBottomPage(ww)) : pwwd->yeScroll,
					fTrue, fNormHoriz);
			iRepeat++;
			goto LRepeat;
			}
		else  if (fNotInRc && fPageView &&
				((ncp & ncpForceYPos) == 0))
			{
			if (fHome)
				dyeScroll = pwwd->yeScroll - yeHome;
			if (dyeScroll != 0)
				FScrollPageDyeHome(ww, dyeScroll, fFalse, ncp, fTrue);
			}
		else  if (ncp & ncpForceYPos || fNotInRc)
			{
			dypLine = ywBottom - ywTop;
/* scroll by dyeScroll (<0 means down on screen. Line bottom is in dypLine */
			if (dyeScroll < 0)
				ScrollDown(ww, max(0, -dyeScroll - dypLine), -dyeScroll);
			else  if (dyeScroll != 0)
				ScrollUp(ww, dyeScroll, dyeScroll +
						((*hpldr)->hpldrBack != hNil ? 0 : 
						min(30, dypLine - 1)));
			}
		}
	else
		{
/* the following return is generated if some operation (deleting an EOP, 
for example) creates a situation where the selection is in a dr which is
not visible on the page because the page has not been layed out yet. */
		if (fPageView)
			return;
		if (dyw == 0)
			ThumbToCp(ww, cp, fEnd, fFalse, ncp);
		else
			{
			int cdlT;
			int dywScan;
			struct DR *pdr;
			struct DRF drf;

			pdr = PdrFetch(hwwd, 0, &drf);
			pdr->cpFirst = cp;
			pdr->dypAbove = 0;
			FreePdrf(&drf);
			(*hwwd)->fDirty = fTrue;
			dywScan = DypScanAbove(ww, dyw, &cdlT);
			pdr = PdrFetch(hwwd, 0, &drf);
			if (dywScan - dyw > ((*hwwd)->ywMac - (*hwwd)->ywMin)/2)
				pdr->dypAbove = dywScan - dyw;
			FreePdrf(&drf);
			}
		}
	(*hwwd)->fSetElev = fTrue;
/* now treat horizontal direction */
	if (ncp & ncpHoriz)
		{
		if (cp > CpMacDocEdit(doc))
			xw = XwFromXp(HwwdWw(ww), 0, 0);
		else
			{
			if ((*hwwd)->fDirty)
				UpdateWw(ww, fFalse);
			dl = DlWhereDocCp(ww, doc, cp, fEnd,
					&hpldr, &idr, &cpFirst, &fChangedPage, fTrue);
			/* this can fail (only) if you run out of memory
			or if you are in the invisible tail of a fixed height
			table row */
			if (dl == dlNil)
				{
				goto LExit;
				}
			xw = XwFromWwCp(ww, hpldr, idr, doc, cp, cpFirst);
			}

/* rule is:
0. if xw is visible, no action.
1. if xw is visible at "home" scroll, scroll home
	home scroll is min(0, vfli.dxpLeft) in galley view.
	in page view if current dr is entirely visible when page is aligned at
	page left margin, home scroll is xlLeftMargin,
	if dr not entirely visible, home scroll is pdr->xl.
2. else line it up to middle of pane width
*/
		pwwd = *hwwd;
		dxeScroll = 0;
		if (xw < pwwd->xwMin || xw >= pwwd->xwMac)
			{
			if (fPageView)
				{
				xeHome = XeLeftPage(ww);
				pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
/* would all of dr width be visible if xeHome were the scroll? */
				if (pdr->xl < xeHome ||
						pdr->xl + pdr->dxl >= xeHome +
						DxOfRc(&pwwd->rcwDisp))
					/*no:*/xeHome = -pdr->xl + dxwSelBarSci;
				}
			else
				xeHome = max(0, -vfli.xpLeft) + dxwSelBarSci;
/* xeHome is proposed xeScroll. Would xw be on the screen if they were ==? */
			xwT = xw + xeHome - pwwd->xeScroll;
			if (xwT >= pwwd->xwMin && xwT < pwwd->xwMac)
/* yes, scroll text right to xeHome */
				dxeScroll = pwwd->xeScroll - xeHome;
			else
/* scroll text to middle (2/3's for WIN)*/
#ifdef WIN
				dxeScroll = xw - ((pwwd->xwMac - pwwd->xwMin)*2)/3;
#else
			dxeScroll = xw - (pwwd->xwMac - pwwd->xwMin) / 2;
#endif /* WIN */
			}
/* scroll by dxeScroll (>0 means text is scrolled to the left) */
		if (dxeScroll)
			{
			if (dxeScroll < 0)
				ScrollRight(ww, -dxeScroll);
			else
				ScrollLeft(ww, dxeScroll);
			}
		}

LExit:
	if ((*hwwd)->fDirty)
		UpdateWw(ww, fFalse);

	if (!fPageView)
		{
/* clear sccAbove if QD insert stuff has been loaded into it. This situation
	can happen when typein occurs towards the bottom of a window. */
		if (cpInsert != cpNil && cpInsert < vsccAbove.rgcp[vsccAbove.dlMac])
			ClearSccs();
/* sccAbove does not have QD insert stuff loaded into it. */
		Assert(!(cpInsert != cpNil && cpInsert < vsccAbove.rgcp[vsccAbove.dlMac]) || (vsccAbove.dlMac == 0));
		}
}


/* F  Y W  I N  W W */
/* returns true iff any part of the band is visible in ww (if fAnyVisible)
otherwise iff at least 3/4th of the band is visible or 3/4 of window is
already occupied by band.
*/
/* %%Function:FYwInWw %%Owner:bryanl */
FYwInWw(pwwd, ywBottom, ywTop, fAnyVisible)
struct WWD *pwwd;
{
	int ywMin = pwwd->ywMin;
	int ywMac = pwwd->ywMac;

	if (!fAnyVisible)
		{
		int dywQ;
		if (min(ywMac, ywBottom) - max(ywMin, ywTop) >=
				NMultDiv(ywMac - ywMin, 3, 4))
			return fTrue;
		dywQ = NMultDiv(ywBottom - ywTop, 3, 4);
		ywBottom -= dywQ;
		ywTop += dywQ;
		}
	return ywBottom >= ywMin && ywTop < ywMac;
}


/* Y W  T O P  F O R  D L */
/* returns the yw of the top of dl in hpldr[idr].hplcedl and returns
	the edl referenced by dl in *pedl. */
/* %%Function:YwTopForDl %%Owner:bryanl */
YwTopForDl(hpldr, idr, dl, pywBottom)
struct PLDR **hpldr;
int idr, dl;
int *pywBottom;
{
	int ywTop;
	struct EDL edl;
	struct DRF drfFetch;

	GetPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl, dl, &edl);
	FreePdrf(&drfFetch);
	ywTop = YwFromYp(hpldr, idr, edl.ypTop);
	*pywBottom = ywTop + edl.dyp;
	return ywTop;
}


/* S E E  S E L */
/* %%Function:SeeSel %%Owner:bryanl */
SeeSel() { 
	SeeSel1(fFalse); 
}


/* %%Function:SeeSel1 %%Owner:bryanl */
SeeSel1(fAlways)
{
	struct SEL *psel = PselActive();
	int dyp = DyOfRc(&(*hwwdCur)->rcwDisp) / 4;
	int wwOther;

	Assert(hwwdCur);
	if (psel->fIns)
		NormCp(wwCur, psel->doc, psel->cpFirst, ncpHoriz, dyp,
				psel->fInsEnd);
	else
		{
/* FCaVisible returns true as long as part of the ca is on the screen,
it does not care if the active pt is off the screen - this is by design.
If you want the active pt to be visible, call NormCp yourself */
		if (fAlways || !FCaVisible(wwCur, &psel->ca))
			NormCp(wwCur, psel->doc, CpActiveCur(),
					ncpVisifyPartialDl + ncpHoriz,
					dyp,
					psel->fInsEnd);
		}

/* Sync ftn/atn window as well, so that GOTO will synch */
	if ((wwOther = WwOther(wwCur)) != wwNil)
		{
		/* don't sync if either ww is in outline */
		if (!PwwdWw(wwCur)->fOutline && !PwwdWw(wwOther)->fOutline)
			SynchCheck(wwCur, fTrue/*fCkVisibility*/);
		}
}


/* S H O W  I N S E R T  L I N E */
/* will cause insertion point to be turned on if presently off */
/* %%Function:ShowInsertLine %%Owner:bryanl */
ShowInsertLine(psel)
struct SEL *psel;
{
	if (psel->fIns && !psel->fOn && !psel->fNil)
		ToggleSel(psel, psel->cpFirst, psel->cpFirst);
}


/* F  O U T L I N E  E M P T Y */
/* returns true iff ww contains a completely supressed view of the
document */
/* %%Function:FOutlineEmpty %%Owner:bryanl */
FOutlineEmpty(ww, fBeep)
BOOL fBeep;
{
#ifndef JR
	struct WWD *pwwd = PwwdWw(ww);
	struct EDL edl;
	struct DR *pdr;

	if (!pwwd->fOutline) return fFalse;
	GetPlc((pdr = PdrGalley(pwwd))->hplcedl, 0, &edl);
	if (edl.fEnd)
		{
		if (CpBackVisibleOutline(pdr->doc,
				CpMacDocEdit(pdr->doc)) == cpNil)
			{
			if (fBeep) Beep();
			return fTrue;
			}
		}
#endif /* JR */
	return fFalse;
}


#ifdef MAC /* not used in OPUS */
/* F  N E A R  R E C T */
/* returns true iff the two rects are nearly equal */
/* %%Function:FNearRect %%Owner:NOTUSED */
FNearRect(rc1, rc2)
int *rc1, *rc2;
{
	int i, dw;
	for (i = 0; i < 4; i++)
		{
		dw = *rc1++ - *rc2++;
		if (dw > 7 || dw < -7) return fFalse;
		}
	return fTrue;
}


#endif /* MAC */

/* %%Function:IdrFirstForDoc %%Owner:chrism */
IdrFirstForDoc(ww, doc)
int ww; 
int doc;
{
	int idr;
	struct DR *pdr;
	int idrMac;
	struct WWD *pwwd;

	pwwd = PwwdWw(ww);
	idrMac = pwwd->idrMac;
	if (!pwwd->fPageView)
		{
		if (idrMac == 0)
			return idrNil;
		Assert(PdrGalley(PwwdWw(ww))->doc == doc);
		return(0);
		}
	for (idr = 0, pdr = PdrWw(ww, 0); idr < idrMac; idr++, pdr++)
		{
		if (doc == pdr->doc)
			return idr;
		}
	return idrNil;
}


#ifdef NOTUSED
/* %%Function:IdrLastForDoc %%Owner:NOTUSED */
IdrLastForDoc(ww, doc)
int ww; 
int doc;
{
	int idr;
	struct DR *pdr;

	if (!PwwdWw(ww)->fPageView)
		{
		Assert(PdrGalley(PwwdWw(ww))->doc == doc);
		return(0);
		}
	for (idr = PwwdWw(ww)->idrMac - 1, pdr = PdrWw(ww, idr); idr >= 0; idr--, pdr--)
		{
		if (doc == pdr->doc)
			return idr;
		}
	return idrNil;
}


#endif


/* F R E E  E D L */
/* Frees the extra memory an Edl takes up */
/* %%Function:FreeEdl %%Owner:chrism */
NATIVE FreeEdl(hplcedl, dl)
struct PLCEDL **hplcedl;
int dl;
{
	struct PLDR **hpldr;
	struct EDL edl;

	GetPlc(hplcedl, dl, &edl);
	if ((hpldr = edl.hpldr) != hNil)
		{
		Break3();
		if (vihpldrMac < ihpldrMax)
			{
			vrghpldr[vihpldrMac++] = hpldr;
			}
		else
			{
			FreeDrs(hpldr, 0);
			if ((*hpldr)->fExternal)
				FreeHq((*hpldr)->hqpldre);
			FreeH(hpldr);
			}
		edl.hpldr = hNil;
		PutPlc(hplcedl, dl, &edl);
		}

}


/* F R E E  E D L S */
/* Frees the edls [dlFirst, dlLim) */
/* %%Function:FreeEdls %%Owner:chrism */
NATIVE FreeEdls(hplcedl, dlFirst, dlLim)
struct PLCEDL **hplcedl;
int dlFirst, dlLim;
{

	for ( ; dlFirst < dlLim ; dlFirst++)
		FreeEdl(hplcedl, dlFirst);

}


/*******************/
/* F r e e   D r s */
/* %%Function:FreeDrs %%Owner:chrism %%reviewed: 7/10/89 */
NATIVE FreeDrs(hpldr, idr)
struct PLDR **hpldr;
int idr;
{
/* release all drs after idr */
	struct DR *pdr;
	int idrMac = (*hpldr)->idrMac;
	int idrT = idr;
	BOOL fInnerDr;
	struct EDL edl;
	struct PLCEDL **hplcedl;
	struct PLDR *ppldr;
	struct DRF drfFetch;

	AssertH(hpldr);
	fInnerDr = (*hpldr)->hpldrBack != hNil;

	if (idrMac <= idr)
		return;
	Assert((*hpldr)->idrMax > 0);
	while (idrT < idrMac)
		{
		pdr = PdrFetch(hpldr, idrT++, &drfFetch);
		/* for tables, free pldr and plcedls inside each edl */
		/* hplcedl can be hNil if window is not visible */
		if ((hplcedl = pdr->hplcedl) != hNil)
			{
			if (!fInnerDr)
				FreeEdls(hplcedl, 0, IMacPlc(hplcedl));
			FreeHplc(hplcedl);
			}
		FreePdrf(&drfFetch);
		}
	ppldr = *hpldr;
	ppldr->idrMac = idr;
	if (!ppldr->fExternal)
		{
		/* make it smaller, because there can be a LOT of drs, but we need
		   at least one for galley */
		ppldr->idrMax = max(1, ppldr->idrMac);
		FChngSizeHCw(hpldr, (ppldr->brgdr + ppldr->cbDr * ppldr->idrMax) / sizeof(int), fTrue);
		}
}


/* %%Function:HplcedlWw %%Owner:bryanl */
struct PLC **HplcedlWw(ww, idr)
int ww, idr;
{
#ifdef DEBUG
	struct PLC **hplcedl = PwwdWw(ww)->rgdr[idr].hplcedl;
	AssertH(hplcedl);
	Assert(!(*hplcedl)->fExtBase);
#endif /* DEBUG */
	return PwwdWw(ww)->rgdr[idr].hplcedl;
}


#ifdef DEBUG
/* P D R   G A L L E Y */
/* %%Function:PdrGalley %%Owner:chrism */
struct DR *PdrGalley(pwwd)
struct WWD *pwwd;
{
	Assert(!pwwd->fPageView);
	Assert(vpdrfHead == NULL);
	return(&pwwd->rgdr[0]);
}


#endif


#ifdef DEBUG
/* I P G D  C U R R E N T  W W */
/* %%Function:IpgdCurrentWw %%Owner:chrism %%reviewed: 7/10/89 */
IpgdCurrentWw(ww)
{
	Assert(PwwdWw(ww)->fPageView);
	return PwwdWw(ww)->ipgd;
}


#endif

/* I P G D  N E X T  W W */
/* %%Function:IpgdNextWw %%Owner:chrism %%reviewed: 7/10/89 */
IpgdNextWw(ww, fRepag)
{
	int ipgd = PwwdWw(ww)->ipgd + 1;
	int doc = PmwdWw(ww)->doc;
	struct PLC **hplcpgd;
	struct RPL rpl;
	struct LBS lbsText, lbsFtn;
	int fFormatVisiSave;
	int lmSave = vlm;
#ifdef WIN
	int flmSave = vflm;
#endif
	Assert(PwwdWw(ww)->fPageView);

	if (ipgd >= IMacPlc(PdodDoc(doc)->hplcpgd) && fRepag)
		{
#ifdef MAC
		InitFliForLayout(lmPagevw, &fFormatVisiSave);
#else
		if (!FInitWwLbsForRepag(wwCur, DocMother(doc), lmPagevw, &lbsText, &lbsFtn))
			return ipgdNil;
#endif
		SetWords(&rpl, pgnMax, cwRPL);
		Mac( rpl.crs = crsHourglass);
		rpl.ipgd = ipgd+1;
		lbsText.hpllr = lbsFtn.hpllr = hNil;
		/* OK if this fails - return checks bounds */
		FUpdateHplcpgd(wwCur, doc, &lbsText, &lbsFtn, &rpl, fFalse);
		EndLayout(&lbsText, &lbsFtn);
		EndFliLayout(lmSave, WinMac(flmSave, fFormatVisiSave));
		}

	return ipgd >= IMacPlc(PdodDoc(doc)->hplcpgd) ? ipgdNil : ipgd;
}


/* I P G D  P R E V  W W */
/* %%Function:IpgdPrevWw %%Owner:chrism %%reviewed: 7/10/89 */
IpgdPrevWw(ww)
{
/* IpgdRepagToCpIpgd wants to go forward for empty pages, so we force one
	more backward for those */
	int ipgd = PwwdWw(ww)->ipgd - 1;
	struct PGD pgd;

	Assert(PwwdWw(ww)->fPageView);

	if (ipgd < 0)
		return(ipgdNil);
	if (ipgd == 0)
		return(ipgd);
	GetPlc(PdodMother(DocBaseWw(ww))->hplcpgd, ipgd, &pgd);
	if (pgd.fEmptyPage)
		ipgd--;
	return(ipgd);
}


/*  F  N O R M  H O R I Z  */
/* returns true iff ww is normalized horizontally.
used as param to SetPageDisp */
/* %%Function:FNormHoriz %%Owner:chic %%reviewed: 7/10/89 */
BOOL FNormHoriz(ww)
{
	struct WWD *pwwd = PwwdWw(ww);
	Assert(pwwd->fPageView);
	return (pwwd->ipgd < 0 || XeLeftPage(ww) == pwwd->rcePage.xeLeft);

}


/* X E  L E F T  P A G E */
/* %%Function:XeLeftPage %%Owner:chic %%reviewed: 7/10/89 */
int XeLeftPage(ww)
int ww;
{
	struct DOD *pdod = PdodDoc(PmwdWw(ww)->doc);
	struct WWD *pwwd = PwwdWw(ww);
	int dxw, dyw, xlLeft, xlRight;
	struct PGD pgd;

/* special case if page is smaller than rcDisp: center the page in the display */
	if (FSmallPage(ww, &dxw, &dyw) || dxw > 0)
		return (dxw >> 1);

	if (pwwd->ipgd == ipgdNil)
		{
		/* out of memory -- pageview will go away but meanwhile
			we have to keep breathing */
		return(vfli.dxsInch);
		}
/* take facing pages doc properties into account */
	GetPlc(pdod->hplcpgd, pwwd->ipgd, &pgd);
	GetXlMargins(&pdod->dop, pgd.pgn & 1,
			WinMac(vfli.dxsInch, DxsSetDxsInch(DocBaseWw(ww))),
			&xlLeft, &xlRight);
	return max(dxwSelBarSci - xlLeft, ((dxw >> 1) - dxpGrayOutsideSci));
}


/* Y E  T O P  P A G E */
/* %%Function:YeTopPage %%Owner:chic %%reviewed: 7/10/89 */
int YeTopPage(ww)
int ww;
{
/* return ye such that if pwwd->yeTop equals ye, then top of main text area is at 
the top of the display.
*/
	return (YePage(ww, fTrue/*fTop*/));
}


/* Y E  B O T T O M  P A G E */
/* %%Function:YeBottomPage %%Owner:chic %%reviewed: 7/10/89 */
int YeBottomPage(ww)
int ww;
{
/* return ye such that if pwwd->yeTop equals ye, then we will scroll to the 
bottom of the gray area.
*/
	return (YePage(ww, fFalse/*fTop*/));
}


/* Y E  P A G E */
/* %%Function:YePage %%Owner:chic */
YePage(ww, fTop)
int ww;
BOOL fTop;
{
/* return yeTopPage or yeBottomPage */

	struct DOD *pdod;
	int dxw, dyw;

/* special case if page is smaller than rcDisp: center the page in the display */
	if (FSmallPage(ww, &dxw, &dyw) || dyw > 0)
		return (dyw >> 1);

	pdod = PdodDoc(PmwdWw(ww)->doc);
	return (fTop ? -DysFromDya(abs(pdod->dop.dyaTop)) + dypMinWwInitSci :
			-DysFromDya(pdod->dop.yaPage) + 
			DyOfRc(&(PwwdWw(ww)->rcwDisp)) - dypGrayOutsideSci);
}


/* F  S M A L L  P A G E */
/* %%Function:FSmallPage %%Owner:chic %%reviewed: 7/10/89 */
FSmallPage(ww, pdxw, pdyw)
int *pdxw, *pdyw;
{
	struct WWD *pwwd = PwwdWw(ww);
	/* use locals cuz callers like to pass pdxw == pdyw */
	int dyw = DyOfRc(&pwwd->rcwDisp) - DyOfRc(&pwwd->rcePage);
	int dxw = DxOfRc(&pwwd->rcwDisp) - DxOfRc(&pwwd->rcePage);
	*pdyw = dyw; 
	*pdxw = dxw;
	return (dyw > 0 && dxw > 0);
}


#ifdef MAC
/* S C C  C P  F I R S T */
/* Update cpFirst of sccBelow.
The first line should be a repeat of the last line on the screen if it
is not fully visible, otherwise the line following */

/* %%Function:SccCpFirst %%Owner:NOTUSED */
SccCpFirst(hwwd)
struct WWD **hwwd;
{
	int dlMac;
	int idr = 0;
	struct WWD *pwwd = *hwwd;
	struct PLCEDL **hplcedl = PdrGalley(pwwd)->hplcedl;

	if ((hplcedl != hNil) &&
			(dlMac = IMacPlc(hplcedl)) > 0)
		{
		struct EDL edl;
		GetPlc(hplcedl, dlMac - 1, &edl);
		if (!edl.fEnd)
			{
			vpsccBelow->fCpBad = fFalse;
			vpsccBelow->cpFirst = CpPlc(hplcedl, dlMac - 1);
			if (YwFromYp(hwwd, idr, edl.ypTop + edl.dyp) == pwwd->ywMac)
/* last line is fully visible */
				vpsccBelow->cpFirst += edl.dcp;
			}
		}
}


#endif	/* MAC */

/* F  C A  V I S I B L E */
/* Given a pca, returns true if the pca is visible in the window.
	Returns true as long as part of the ca is on the screen,
	it does not care if the active pt is off the screen - this is by design 
*/
/* %%Function:FCaVisible %%Owner:bryanl */
FCaVisible(ww, pca)
struct CA *pca;
{

	int doc = pca->doc;
	struct WWD **hwwd = HwwdWw(ww);
	int dlFirst, dlLim;
	int idr, idrMac;
	struct DR *pdr;
	CP cpFirst, cpLim, cpFirstT, cpLimT;

/* Determine limits of visible cp's */
	cpFirst = CpMacDoc(doc);
	cpLim = cp0;
/*BOGUS see bug 3428	if ((*hwwd)->fDrDirty) return fFalse;*/
	if ((*hwwd)->fDirty) UpdateWw(ww, fFalse);
/* the generic loop will work for galley and page views. */
	for (idr = 0, idrMac = (*hwwd)->idrMac; idr < idrMac; idr++)
		{
		pdr = PdrWw(ww, idr);
		if (pdr->doc != pca->doc || pdr->cpFirst == cpNil)
			continue;
		FullDlBoundsDr(ww, hwwd, idr, &dlFirst, &cpFirstT, &dlLim, &cpLimT);
		if (dlFirst < dlLim)
			{
			cpFirst = CpMin(cpFirst, cpFirstT);
			cpLim = CpMax(cpLim, cpLimT);
			}
		}

	return (cpLim > pca->cpFirst && cpFirst <= pca->cpLim);
}


/* F U L L  D L  B O U N D S  D R */
/* returns dl's and cp's of the verticall fully visible and horizontally
partially visible edl's of ww.
if only one line is visible, it is taken to be vertically visible.
Note: pdlLim may be NULL and then the limits are not computed.
if dlFirst >= dlLim, none are visible.
*/
/* %%Function:FullDlBoundsDr %%Owner:bryanl */
FullDlBoundsDr(ww, hpldr, idr, pdlFirst, pcpFirst, pdlLim, pcpLim)
struct PLDR **hpldr; 
int ww, idr, *pdlFirst, *pdlLim; 
CP *pcpFirst, *pcpLim;
{
	int dl, dlFirst, dlLim;
	int dlMac;
	struct PLCEDL **hplcedl;
	struct DR *pdr;
	CP cpFirst, cpLim;
	struct EDL edl;
	struct DRF drfFetch;
	struct RC rcpDisp;

	pdr = PdrFetch(hpldr, idr, &drfFetch);
	dlFirst = 0; 
	dlLim = 0; 
	cpFirst = cpLim = pdr->cpFirst;

	if ((hplcedl = pdr->hplcedl))
		{
		RcwToRcp(hpldr, idr, &PwwdWw(ww)->rcwDisp, &rcpDisp);
		for (dl = 0, dlMac = IMacPlc(hplcedl); dl < dlMac ; dl++)
			{
			GetPlc(hplcedl, dl, &edl);
			if (rcpDisp.ypTop <= edl.ypTop &&
					rcpDisp.xpLeft < edl.xpLeft + edl.dxp &&
					rcpDisp.xpRight > edl.xpLeft) break;
			}
		dlFirst = dl; 
		cpFirst = CpPlc(hplcedl, dl);
		if (pdlLim != NULL && dlMac != 0)
			{
			for (dl = dlMac - 1; dl >= dlFirst; dl--)
				{
				GetPlc(hplcedl, dl, &edl);
				if ((rcpDisp.ypBottom >= edl.ypTop + edl.dyp || dl == dlFirst) &&
						rcpDisp.xpLeft < edl.xpLeft + edl.dxp &&
						rcpDisp.xpRight > edl.xpLeft) break;
				}
			dlLim = dl + 1; 
			cpLim = CpPlc(hplcedl, dlLim);
			}
		}
	FreePdrf(&drfFetch);
	*pdlFirst = dlFirst; 
	*pcpFirst = cpFirst;
	if (pdlLim != NULL)
		*pdlLim = dlLim, *pcpLim = cpLim;
}


#ifdef DISABLE_DRM
/* R E A L L O C  B M P  S C C */
/* %%Function:ReallocBmpScc %%Owner:NOTUSED */
ReallocBmpScc(pscc)
struct SCC *pscc;
{
	int bpp;
	uns cb;
	struct RC rcT;

	if (!pscc)
		return;

	cb = pscc->cbSccBmb;
	DisposeHPixMap(&(pscc->qqbitmap));

	if (cb)
		{
		SetRect(&rcT, 0, 0, 16, cb/RowBytes(16, bpp = BitsPerPixelWw(pscc->ww)));
		pscc->qqbitmap = HNewPixMap(wwCur, &rcT);
		pscc->cbSccBmb = LPixMapSize((struct RC far *)&rcT, bpp);
		}

	SynchSccWw(pscc, wwCur);

}


#endif


/* W W  S C R O L L  W K  T E S T */
/* returns wwOther iff other window exists AND it is of the same type
with respect to horizontal scrolling. 
Outline view only scrolls with outline view.
Page view only scrolls with page view.
Mother doc in galley only scrolls with subdoc in galley view.
For WIN, subdoc includes footnote, annotation, header.
For MAC, subdoc only includes footnote.
*/
/* %%Function:WwScrollWkTest %%Owner:chic %%reviewed: 7/10/89 */
WwScrollWkTest(ww)
{
	int wwOther;

	if ((wwOther = WwOther(ww)) != wwNil &&
			!((PwwdWw(wwOther)->wk ^ PwwdWw(ww)->wk) & ~WinMac(wkSDoc, wkFtn)))
		return wwOther;
	return wwNil;
}


/* S E T  E L E V  W W */
/* %%Function:SetElevWw %%Owner:bryanl */
EXPORT SetElevWw(ww)
{
	struct WWD *pwwd;
	CP cpMac, cpCurPg, cpNextPg;
	int dq;
	int dyeScroll, dyeScrollMac;
	int doc, ipgd;
	struct DRF drfFetch;
	extern CP CpFromIpgd();
#ifdef	MAC
	extern char far *far *hctlSbarHelp;
	extern long GetWindowControlRefcon();
#endif /*MAC*/

	Assert(cHpFreeze == 0);

	if ((pwwd = PwwdWw(ww))->idrMac == 0)
		{
		/* NO DRS!!! */
		dq = 0;
		goto LEternalComment;
		}

	doc = DocBaseWw(ww);
	cpMac = doc != docNil ? CpMacDoc(doc) : cp0;
	if (pwwd->fPageView)
		{
		int dqNextPg, dqCurPg;
		cpCurPg = CpFromIpgd(doc, ww, pwwd->ipgd); /* HM */
		if ((ipgd = IpgdNextWw(ww, fFalse)) == ipgdNil)
			cpNextPg = CpMacDoc(doc);
		else
			cpNextPg = CpFromIpgd(doc, ww, ipgd); /* HM */

		FreezeHp();
		pwwd = PwwdWw(ww);
		dqCurPg = DqRatio(cpCurPg, dqMax - dqPgvwElevSpace, cpMac);
		dqNextPg = DqRatio(cpNextPg, dqMax - dqPgvwElevSpace, cpMac);
		dyeScrollMac = max(1,
				DyOfRc(&pwwd->rcePage) - DyOfRc(&pwwd->rcwDisp));
		dyeScroll = min(dyeScrollMac, max(0, -pwwd->rcePage.yeTop));
		if (dyeScrollMac > 0)
			dq = min( dqCurPg + NMultDiv(dqNextPg - dqCurPg, dyeScroll, dyeScrollMac),
					dqMax - dqPgvwElevSpace);
		else
			dq = dqMax - dqPgvwElevSpace;
		MeltHp();
		}
	else
		{
		dq = DqRatio(PdrFetchAndFree(HwwdWw(ww), 0, &drfFetch)->cpFirst,
				dqMax - 1, cpMac);
		}

LEternalComment:
/* Following comment is preserved verbatim for eternity */
/* Contemplating this 'if' statement should elevate one to a higher plane
of existence. */
	if (dq != pwwd->dqElevator)
		{
		FreezeHp();
#ifdef MAC
		PwwdSetWw(ww, cptDoc);
		pwwd->dqElevator = dq;
/* reset the value of the vertical scroll bar */
		PwwdSetWw(ww, cptAll);
		if (ww == wwHelp)
			SetCtlValue(hctlSbarHelp, dq);
		else
			SetCtlValue(GetWindowControlRefcon(
					pwwd->fLower ? ctpSbVer2 : ctpSbVer), dq);
#endif
#ifdef WIN
/* reset the value of the vertical scroll bar */
		if (pwwd->wk == wkClipboard)
			SetScrollPos( pwwd->hwnd, SB_VERT, dq, fTrue );
		else  if (pwwd->hwndVScroll != NULL)
			RSBSetSps( pwwd->hwndVScroll, dq );
		pwwd->dqElevator = dq;
#endif
		MeltHp();
		}
}


/* D Q  R A T I O */
/* returns dqX such that given [cp0, cpLim] mapping [0, dqLim], cp maps into
dqX.
*/
/* %%Function:DqRatio %%Owner:bryanl */
private int DqRatio(cp, dqLim, cpLim)
CP cp, cpLim; 
int dqLim;
{
	int dqScale = cpLim > 0x7777 ? 128 : 1;
	CP cpLimScale = cpLim / dqScale;
	return(min(dqLim,
			(int)((cp / dqScale * dqLim + (cpLimScale >> 1))
			/ cpLimScale)));
}


/* D L  W H E R E  P T */
/* finds the smallest enclosing hpldr, idr, dl for the point in pt (xw, yw).
xw in pt may be changed to xwRight of the dr if yw is pointing to below an
endmark.
In pspt, a flag is set if the cursor is within any of the special areas
of the dr.
Returns dlNil if idr is not found.
*/
/* %%Function:DlWherePt %%Owner:bryanl */
int DlWherePt(ww, ppt, phpldr, pidr, pspt, fInner, fEmptyOK)
int ww; 
struct PT *ppt; 
struct PLDR ***phpldr; 
int *pidr; 
struct SPT *pspt;
int fInner;   /* If false, return without looking inside
table pldrs */
{

/* find a DR containing pt. Backward scan because we draw top-to-bottom in
	hpldr, therefore first entry is on the bottom */
	int i;
	int fInTable = fFalse;
	int idr, idrMac;
	struct PLDR **hpldr;
	struct DR *pdr;
	int dl, ywSum;
	struct PLCEDL *hplcedl;
	int dlMac;
	int doc;
	long dxyMin, dxyT;
	int idrMin;
	CP cpFirst;
	int fPageView = PwwdWw(ww)->fPageView;
	int fTableTop;
	int dylMac;
	int xwLeft;
	struct PT pt;
	struct EDL edl;
	struct RC rcw, rcwT;
	struct DRF drfFetch;
	int iRepeat = 0;

	Assert(!FOutlineEmpty(ww, fFalse));
	pt = *ppt;
	*(int *)pspt = 0;
#ifdef WIN
	/* if in styname area, set flag and fake point in selbar to get a dl */
	if (pt.xw < PwwdWw(ww)->xwSelBar)
		{
		pspt->fInStyArea = fTrue;
		pt.xw = PwwdWw(ww)->xwSelBar;
		}
#endif

	pspt->fInDr = fTrue;
LStart:
	hpldr = HwwdWw(ww);
	if ((*((struct WWD **)hpldr))->fDirty)
		WinMac( UpdateWindowWw(ww), UpdateWw(ww, fFalse));

LFindIdr:
	idrMin =idrNil;
/* loop is for: i=0: in dr, i=1: in sel bar, i=2: in general area */
	for (i = 0; i < 3; i++)
		{
		for (idr = (*hpldr)->idrMac - 1;
				idr >= 0; FreePdrf(&drfFetch), idr--)
			{
			pdr = PdrFetch(hpldr, idr, &drfFetch);
			if (pdr->doc <= 0 || (fPageView && !FSelectableDr(pdr)))
				continue;
			DrclToRcw(hpldr, &pdr->drcl, &rcw);
			if (i == 0)
				{
				if (!(fPageView || fInTable))
					rcw.xwLeft = max(PwwdWw(ww)->xwSelBar+dxwSelBarSci, rcw.xwLeft);
				else  if (fInTable)
					{
					rcw.ywBottom = rcw.ywTop + dylMac;
					rcw.xwRight += pdr->dxpOutRight;
					}
				}
			else  if (i == 1)
				{
				if (fPageView || fInTable)
					{
					int xwRight = rcw.xwRight = rcw.xwLeft;
					if (fPageView)
						{

/* check for possibility of left indent text being displayed in OutLeft */
						rcw.xwLeft = xwRight - pdr->dxpOutLeft;
						if (FPtInRect(pt, &rcw))
							{
/* chances are we will select in this dr so it is ok to invest into scanning the edl's */
							struct PLCEDL **hplcedl = pdr->hplcedl;
							dlMac = IMacPlc(hplcedl);
							for (dl = 0; dl < dlMac; dl++)
								{
								GetPlc(hplcedl, dl, &edl);
								if (edl.xpLeft < 0)
									{
									struct RC rcwEdl;
									DrcpToRcw(hpldr, idr, &edl.drcp, &rcwEdl);
									if (FPtInRect(pt, &rcwEdl))
										{
										i = 0;
										goto LGotIdr;
										}
									rcwEdl.xwRight = rcwEdl.xwLeft;
									rcwEdl.xwLeft = rcwEdl.xwLeft - dxwSelBarSci;
									if (FPtInRect(pt, &rcwEdl))
										goto LGotIdr;
									}
								}
							}

						}
					/* NOTE: maxing with 2 is arbitrary, we just don't want
						to leave them with no selBar */
					rcw.xwLeft = xwRight - 
							((fInTable && idr != 0) ? max(2, pdr->dxpOutLeft) : dxwSelBarSci);
					}
				else
					{
					rcw.xwLeft = PwwdWw(ww)->xwSelBar;
					rcw.xwRight = rcw.xwLeft + dxwSelBarSci;
					}
				if (fInTable)
					{
					rcw.ywTop = YwFromYl(hpldr, 0);
					rcw.ywBottom = YwFromYl(hpldr, (*hpldr)->dyl);
					}
				}
			else  if (i == 2)
				{
				pspt->fInDr = fFalse;
				if (pdr->dxl && pdr->doc > 0)
					{
					dxyT = LDist2RcPt(&rcw, pt);
					if (idrMin == idrNil || dxyT < dxyMin)
						{
						dxyMin = dxyT;
						idrMin = idr;
						}
					}
				continue;
				}
			if (FPtInRect(pt, &rcw) && pdr->doc > 0)
				goto LGotIdr;
			}
		}
	if (idrMin == idrNil)
		{
		*pidr = idrNil;
		return dlNil;
		}
	idr = idrMin;
	pdr = PdrFetch(hpldr, idr, &drfFetch);
LGotIdr:
/* we have idr, pdr */
/* search the plcedl of the hpldr and return the dl which brackets the
given yw. pt.xw is changed to xwRight of the dr if yw is below the endmark
and not in the selection bar.
Repeat process if edl of dl contains a table row.
*/
	hplcedl = pdr->hplcedl;
	/* hplcedl can be nil in super-tight memory */
	if (hplcedl == hNil || (dlMac = IMacPlc(hplcedl)) == 0)
		{
		FreePdrf(&drfFetch);
		return dlNil;
		}
	GetPlc(hplcedl, 0, &edl);
	ywSum = YwFromYp(hpldr, idr, edl.ypTop);
	for (dl = 0; dl < dlMac; ++dl)
		{
		GetPlc(hplcedl, dl, &edl);
		if (edl.fEnd)
			{
			if (dl == 0)
/* nothing but endmark in the window */
				{
				if (fEmptyOK)
					{
					FreePdrf(&drfFetch);
					return dlNil;
					}
				FreePdrf(&drfFetch);
				if (++iRepeat > 1)
/* don't return 0, caller could have fetched a cpMacDoc from that dl, 
all callers should be able to handle dlNil returned */
					return dlNil;
				ScrollDown(ww, dysMinAveLineSci, dysMacAveLineSci);
				goto LStart;
				}
			break;
			}
		ywSum += edl.dyp;
		if (ywSum > pt.yw)
			goto LReturnDl;
		}
	if (i != 1)
		{
		DrclToRcw(hpldr, &pdr->drcl, &rcwT);
		ppt->xw = rcwT.xwRight;
		}
	dl = max(0, dl - 1);
LReturnDl:
	GetPlc(hplcedl, dl, &edl);
	if (edl.hpldr != hNil && fInner)
		{
		hpldr = edl.hpldr;
		dylMac = edl.dyp;
		fInTable = fTrue;
		FreePdrf(&drfFetch);
		goto LFindIdr;
		}
	xwLeft = XwFromXp(hpldr, idr, edl.xpLeft);
/* set selection position if we were in the sel bar */
	if (i == 1 || (pt.xw < xwLeft - (dxwSelBarSci / 2) &&
			ppt->xw != xwMaxSci))
		pspt->fSelBar = fTrue;
/* if we were selecting in a table cell we would like
	to see if we were touching any of the special control areas of the table.*/
	if (fInTable)
		{
		pspt->fInTable = fTrue;
		pspt->itc = idr;
/* check if we are in the topmost table row */
		fTableTop = fFalse;
		if (i != 1 && dl == 0 && (doc = pdr->doc) != docNil)
			{
			cpFirst = CpPlc(hplcedl, dl);
			CpFirstTap(doc, cpFirst);
			if (caTap.cpFirst > cp0)
				CachePara(doc, caTap.cpFirst - 1);
			fTableTop = (caTap.cpFirst == cp0 ||
					!FInTableVPapFetch(doc, caTap.cpFirst - 1));
			}
/* if we're in the sel bar or in the top table row and
	the mouse is touching a narrow strip at the top of the dl which is
	proportional to the size of the dl, the mouse is touching one of the
	table control areas. */
		if ((i == 1 || fTableTop) &&
				pt.yw - (ywSum - edl.dyp) <
				min(12,  ((edl.dyp + 1)/ 4) + 1))
			{
/* if in sel bar of the 0th cell, the mouse is touching the control area for
	selecting the entire row/table. */
			if (i == 1)
				{
				if (idr == 0)
					pspt->fWholeRow = fTrue;
				}
			else
				{
/* else record which cell the mouse pt is within and note if it is close to
	the cell's borders. */
				pspt->fWholeColumn = fTrue;
				if (pt.xw - rcw.xwLeft < dxwBorder)
					pspt->fLeftBorder = fTrue;
				if (rcw.xwRight - pt.xw < dxwBorder)
					pspt->fRightBorder = fTrue;
				}
			}
		}
	*phpldr = hpldr;
	*pidr = idr;
	FreePdrf(&drfFetch);
	return dl;
}


/* F   S e l e c t a b l e   D r */
/* %%Function:FSelectableDr %%Owner:chrism */
int FSelectableDr(pdr)
struct DR *pdr;
{
/* return fTrue if the DR is not fSpec, is not empty, and is not a 
	footnote separator; used in page view only */
	struct DOD *pdod;

	if (pdr->doc <= 0 || pdr->cpFirst == cpNil)
		return(fFalse);
	pdod = PdodDoc(pdr->doc);
	return(pdod->dk != dkDispHdr || pdod->ihdt < ihdtMaxSep);
}


/* T R A S H  W W */
/* %%Function:TrashWw %%Owner:bryanl */
TrashWw(ww)
{ /* Invalidate all dl's in ww */
	struct WWD **hwwd;
	struct WWD *pwwd;

	if (hwwd = mpwwhwwd[ww])
		{
		/* invalidate and extend rcInval to whole ww content */
		/* use negative yp's for page view */
		InvalBand(ww, -ypMaxAll, ypMaxAll);
		pwwd = *hwwd;
		Mac(pwwd->fFullSizeUpd = fTrue;)
		pwwd->fBackgroundDirty = fTrue;
#ifdef WIN
/* MAC's default updaterect is the whole window or it is adjusted by MAC's window event, so MAC's DrawBlankPage does not have to worry about blanking too much */
/* WIN's DrawBlankPage relies on WINDOW providing a rcw to draw into. 
if there is invalid area caused by a dialog box, DrawBlankPage will only
redraw that area so we must force the whole rect to be invalidated. */
		if (pwwd->fPageView)
			InvalidateRect(pwwd->hwnd, (LPRECT)&pwwd->rcwDisp, fTrue);
#endif
		}

	TrashSccFli();
	if (ww == wwCur)
		{
		if (!selCur.fBlock && !selCur.fTable)
			{
			selCur.xpFirst = xpNil;
			selCur.xpLim = xpNil;
			}
		if (!selDotted.fBlock)
			{
			selDotted.xpFirst = xpNil;
			selDotted.xpLim = xpNil;
			}
		TrashRuler();
		}
}


/* T R A S H  A L L  W W S */
/* %%Function:TrashAllWws %%Owner:bryanl */
TrashAllWws()
{ /* trash all doc windows */
	int     ww;

	InvalCaFierce();
	for (ww = wwDocMin; ww < wwMax; ++ww)
		if (mpwwhwwd[ww])
			TrashWw(ww);
}


/* T R A S H  W W S  F O R  D O C */
/* %%Function:TrashWwsForDoc %%Owner:bryanl */
TrashWwsForDoc(doc)
int doc;
{
/* invalidate windows owned directly by doc */
	int ww;

/* FUTURE - should we call InvalCaFierce) here? Some callers do, some don't
and look like they should */
	for (ww = WwDisp(doc, wwNil, fFalse); ww != wwNil; ww = WwDisp(doc, ww, fFalse))
		TrashWw(ww);
}


#ifdef MAC
/* E N S U R E  M A R G I N S  I N  R A N G E 
*  ensures that margins and gutter are acceptable, given xaPage and yaPage.
*  If they are out of range, set them to zero.
*/
/* %%Function:EnsureMarginsInRange %%Owner:NOTUSED */
EnsureMarginsInRange(pdop)
struct DOP *pdop;
{
	if (pdop->dxaRight + pdop->dxaLeft + pdop->dxaGutter > pdop->xaPage)
		pdop->dxaRight = pdop->dxaLeft = pdop->dxaGutter = 0;
	if (pdop->dyaTop + pdop->dyaBottom > pdop->yaPage)
		pdop->dyaTop = pdop->dyaBottom = 0;
}


#endif

#ifdef MAC
/* S Y N C  S B  H O R */
/* set horizontal scroll thumb to value in wwd.
Inverse function is in ThumbWindow.
*/
/* %%Function:SyncSbHor %%Owner:NOTUSED */
SyncSbHor(ww)
{
	struct WWD *pwwd = PwwdWw(ww);
	int dxaNum, dxaDenom;
	uns dq;
	struct RC rc;
	extern CONTROLREC far * far *GetWindowControlRefcon();

	if (pwwd->fPageView)
		{
		dxaNum = DxaFromDxs(pwwd, -pwwd->xeScroll + dxpGrayOutsideSci);
		dxaDenom = DxaFromDxs(pwwd,
				max(XeLeftPage(ww), DxOfRc(&pwwd->rcePage) - DxOfRc(&pwwd->rcwDisp)
				+ dxpGrayOutsideSci) + dxpGrayOutsideSci);
		}
	else
		{
		dxaNum = DxaFromDxs(pwwd, max(0, -pwwd->xhScroll));
		dxaDenom = xaRightMaxSci - DxaFromDxs(pwwd, DxOfRc(&pwwd->rcwDisp));
		dq = NMultDiv(min(dxaNum, dxaDenom), xaRightMaxSci, dxaDenom);
		}
	PwwdSetWw(ww, cptAll);
	SetCtlValue(HctlSbHor(ww), 
			NMultDiv(min(dxaNum, dxaDenom), xaRightMaxSci, dxaDenom));
	rc = *((struct RC *)&(*GetWindowControlRefcon(ctpSbHor))->contrlRect);
	ValidRect(&rc);
}


#else	/* WIN */
/* S Y N C  S B  H O R */
/* set horizontal scroll thumb to value in wwd.
Inverse function is in ThumbWindow.
*/
/* %%Function:SyncSbHor %%Owner:bryanl */
SyncSbHor(ww)
{
	struct WWD *pwwd = PwwdWw(ww);
	int dxsNum, dxsDenom;
	uns dq;
	HWND hwnd;
	struct RC rc;

	if (pwwd->fPageView)
		{
		if (pwwd->ipgd == ipgdNil)
			return;
		dxsNum = max(0, -pwwd->xeScroll + dxpGrayOutsideSci);
		dxsDenom = max(XeLeftPage(ww), 
				DxOfRc(&pwwd->rcePage) - DxOfRc(&pwwd->rcwDisp)
				+ dxpGrayOutsideSci) + dxpGrayOutsideSci;
		}
	else
		{
		dxsNum = max(0, -pwwd->xhScroll);
		dxsDenom = vsci.xpRightMax - DxOfRc(&pwwd->rcwDisp);
		}

	Assert(dxsDenom > 0);
	dq = NMultDiv(min(dxsNum, dxsDenom), vsci.xpRightMax, dxsDenom);
	if (pwwd->wk == wkClipboard)
		SetScrollPos( pwwd->hwnd, SB_HORZ, dq, fTrue /* fRedraw */ );
	else  if ((hwnd = PmwdMw(pwwd->mw)->hwndHScroll) != NULL)
		RSBSetSps( hwnd, umin( dq, SpsLimFromHwndRSB( hwnd )-1));
}


#endif

#ifdef	NOTUSED
#ifdef	DEBUG
/* C H E C K  P L D R */
/* Description: This routine runs through a PLDR, checking
/* its EDLs and recursively checking any table row PLDRs.
/* I used this to track down a timing dependent bug which
/* was trashing EDLs. Feel free to tune it for particular bugs.
/**/
/* %%Function:CheckPldr %%Owner:tomsax */
CheckPldr(hpldr)
struct PLDR **hpldr;
{
	int	idr, idrMac, dl, dlMac;
	BOOL	fInner;
	CP cpCur, cpLim;
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct DRF drf;

	fInner = (*hpldr)->hpldrBack != hNil;

	for (idr = 0, idrMac = (*hpldr)->idrMac; idr < idrMac; ++idr)
		{
		pdr = PdrFetch(hpldr, idr, &drf);
		Assert(!fInner || idr == 0 || pdr->fDirty || cpCur == cpNil || pdr->cpFirst == cpCur);

		hplcedl = pdr->hplcedl;
		cpCur = CpPlc(hplcedl, 0);
		Assert(pdr->fDirty || pdr->cpFirst == cpNil || cpCur == cpMax || cpCur == pdr->cpFirst);
		for (dl = 0, dlMac = IMacPlc(hplcedl); dl < dlMac; ++dl, cpCur = cpLim)
			{
			GetPlc(hplcedl, dl, &edl);
			cpLim = CpPlc(hplcedl, dl+1);
			if (edl.fDirty || edl.fTableDirty)
				continue;
			if (edl.hpldr != hNil)
				{
				Assert(!fInner);
				CheckPldr(edl.hpldr);
				}
			Assert(cpLim > cpCur);
			}
		if (dl < dlMac)
			cpCur = cpNil;
		FreePdrf(&drf);
		}
}


#endif	/* DEBUG */
#endif	/* NOTUSED */


#ifdef WIN
#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Disp3_Last(){}
#endif /* PROFILE */
#endif /* WIN */

/* ADD NEW CODE *ABOVE* Disp3_Last() */
