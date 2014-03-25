#ifdef MAC
#define CONTROLS
#define WINDOWS
#include "toolbox.h"
#endif

#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "doc.h"
#include "disp.h"
#include "layout.h"
#include "preview.h"
#include "cmd.h"
#include "sel.h"
#include "debug.h"
#include "screen.h"
#include "scc.h"
#include "format.h"
#include "prm.h"
#define RULER
#include "ruler.h"
#include "ch.h"
#include "heap.h"
#include "error.h"

#ifdef MAC
#include "mac.h"
#else
#include "rsb.h"
#include "message.h"
#endif

#ifdef WIN
extern BOOL fElActive;
#endif
extern int              vlm;
extern int 		wwCur;
extern struct WCC 	vwcc;
extern struct DOD       **mpdochdod[];
extern struct WWD 	**hwwdCur;
extern struct WWD 	**mpwwhwwd[];
extern struct CLSE      dncls[];
extern struct SEL       selCur;
extern struct MERR      vmerr;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern CP               vcpFirstLayout;
extern struct CA        caSect;
extern struct SCC       vsccAbove;
extern struct SCC       *vpsccBelow;
extern struct FLI       vfli;
extern struct RULDS     vrulds;
extern struct FMTSS     vfmtss;
extern int              mwCur;
extern struct PREF	vpref;
extern struct UAB	vuab;
extern int              vfSeeSel;
extern struct DRF *vpdrfHead;

#ifdef WIN
extern struct SCI	vsci;
extern struct FTI       vfti;
extern int		vflm;
extern PVS              vpvs;
extern HRGN		vhrgnGrey;
extern HRGN		vhrgnPage;
extern struct MWD       **mpmwhmwd[];

#define FRulerUp(ww)    (PmwdWw(ww)->wwRuler == ww)
#endif /* WIN */


#ifndef WIN
NATIVE int FPldrFromPllr(); /* DECLARATION ONLY */
NATIVE void AdjustPdrDyl(); /* DECLARATION ONLY */
#else /* WIN */
int FPldrFromPllr();
void AdjustPdrDyl();
#endif /* WIN */


/*****************************/
/* C m d   P a g e   V i e w */
/* %%Function:CmdPageView %%Owner:chic */
CmdPageView(pcmb /* fOn if invoked from macro statement */)
CMB *pcmb; /* NOTE: for Win, this might be a flag (0 or 1) */
{
/* toggle page view mode in wwCur (wwOther if footnote)
	plan for going to page view:
	1. find page containing selCur.cpFirst (for real, not from plcpgd)
	2. if page starts with selCur.cpFirst, format the page; otherwise,
		UpdateHplcpgd formatted the page; creates hpllr
	3. create pldr from pllr
*/
	int wwT;
	int ihdt = ihdtNil;
	int cmd = cmdError;
#ifdef WIN
	int ipgdPrvw = ipgdNil;
	BOOL fRecordingSav;
#endif
	struct DRF drfFetch;

	Assert(wwCur >= wwDocMin);
	Assert(PselActive() == &selCur);

	StartLongOp();
	FCleanUpHdr(DocMother(selCur.doc), fFalse /*fForce*/, fTrue/*fKillEmpty*/);
#ifdef WIN
	/* going into page view gets you out of preview or draft view */
	if (vpref.fDraftView && CmdDraftView(NULL) != cmdOK)
		goto LRet2;

	if (vlm == lmPreview)
		{
		ipgdPrvw = vpvs.ipgdPrvw;
		FExecCmd(bcmPrintPreview);
		/* if already in pageview, skip the rest of this junk... */
		if (pcmb != 0 && (*hwwdCur)->fPageView)
			goto LRet;
		}
#endif


/* get window of mother doc */
	if ((*hwwdCur)->fHdr)
		{
		Win(wwT = WwOther(wwCur));
		Mac(ActivateWw(wwT = WwDisp(DocMother(selCur.doc), wwNil, fTrue)));
		ihdt = PdodDoc(selCur.doc)->ihdt;
		/* Don't record pane change */
		Win(InhibitRecorder(&fRecordingSav, fTrue));
		NewCurWw(wwT, fFalse /*fDoNotSelect*/);
		Win(InhibitRecorder(&fRecordingSav, fFalse));
		}
	else  if (PdodDoc(selCur.doc)->fFtn || PdodDoc(selCur.doc)->fAtn)
		{
		Mac(Assert(PdodDoc(selCur.doc)->fFtn));
		if ((*hwwdCur)->wk != wkPage)
			/* changing galley=>pagevw or pagevw=>galley from
				ftn split */
			{
			/* Don't record pane change */
			Win(InhibitRecorder(&fRecordingSav, fTrue));
			NewCurWw(WwOther(wwCur), fFalse /*fDoNotSelect*/);
			Win(InhibitRecorder(&fRecordingSav, fFalse));
			}
		else
			{
			/* leaving pagevw from footnote dr, never a atn dr */
			int docFtn = selCur.doc;
			int docMother = DocMother(docFtn);
			CP cpNew = CpRefFromCpSub(docFtn, selCur.cpFirst, edcDrpFtn) + 1;
			Assert(!PdodDoc(docFtn)->fAtn); /* Yes, even for WIN */
			if (cpNew <= CpMacDocEdit(docMother))
				{
				/* guarded against ins.pt. after last ftn */
				TurnOffSel(&selCur);
				selCur.sk = skNil;
				selCur.doc = DocMother(docFtn);
				SelectIns(&selCur, cpNew);
				}
			}
		}

	if ((*hwwdCur)->fOutline && CmdExecUcm(ucmOutlining, hNil) != cmdOK)
		goto LRet2;

	Mac(PwwdSetWw(wwCur, cptAll));

	
#ifdef WIN
/* Macro is using pcmb as fOn (0 or 1), it is assuming a normal pcmb is > 2 */
	Assert(fElActive || (uns) pcmb > 2);
#endif

	if ((*hwwdCur)->wk == wkPage)
		{
		/* goto galley view */
		if ((BOOL) pcmb != fTrue && CmdPageViewOff(wwCur) != cmdOK)
			goto LRet2;
		}
	else
		{
		/* goto page view */
		if ((BOOL) pcmb != fFalse && 
				CmdPageViewOn(wwCur, selCur.doc) != cmdOK)
			{
			goto LRet2;
			}
		}
	
	if (ihdt != ihdtNil)
		{
		/* go to the header/footer area in page view */
		int fHdr = FHeaderIhdt(ihdt);
		int idr, idrMac;
		int docHdrDisp;

		for (idr = 0, idrMac = (*hwwdCur)->idrMac; idr < idrMac; idr++)
			{
			docHdrDisp = PdrFetchAndFree(hwwdCur,idr,&drfFetch)->doc;
			if (docHdrDisp > 0 && PdodDoc(docHdrDisp)->fDispHdr && 
					FHeaderIhdt(PdodDoc(docHdrDisp)->ihdt) == fHdr)
				{
				selCur.doc = docHdrDisp;
				selCur.sk = skNil;
				SelectIns(&selCur, cp0);
				vfSeeSel = fTrue;
				break;
				}
			}
		}

	Mac(SizeWindowControls());

	/* Force a redraw of the ruler */
	if (FRulerUp(wwCur))
		{
#ifdef MAC
		vrulds.stc = -1;
		vrulds.rk = -1;
		vrulds.itbdMac = -1;
		UpdateRuler();
#else
		selCur.fUpdateRuler = fTrue;
#endif		
		}
	Mac(DrawWindowControls((*hwwdCur)->wwptr, fFalse));

#ifdef WIN
	if (ipgdPrvw != ipgdNil && PwwdWw(wwCur)->ipgd != ipgdPrvw)
		SetPageDisp(wwCur, ipgdPrvw, YeTopPage(wwCur), fFalse, fFalse);
#endif
LRet:
	cmd = cmdOK;

LRet2:
	EndLongOp(fFalse);
	return(cmd);
}


/*******************************/
/* C m d P a g e V i e w O f f */
/* Get out of page view, go into galley view */

/* %%Function:CmdPageViewOff %%Owner:chic */
CmdPageViewOff(ww)
int ww;
{
	int doc;
	struct DOD *pdod;
	int idr, idrMac;
	struct DR *pdr;
	int dlFirst;
	struct WWD *pwwd;
	struct WWD **hwwd = HwwdWw(ww);
	struct RC rcT;
	CP cpFirst, cpT, cpSee;
	struct RC rcw;
	struct SELS *psels;
	int wwOther, wk;

#ifdef MAC
/* sync horiz with other normal pane, if any */
	if ((wwOther = WwOther(ww)) != wwNil &&
			((wk = PwwdWw(wwOther)->wk) == wkDoc || wk == wkFtn))
		ScrollHoriz(wwOther, PwwdWw(wwOther)->xhScroll);
#endif	
	if (ww == wwCur && selCur.fIns)
		TurnOffSel(&selCur);
	rcw = (*hwwd)->rcwDisp;

/* goto galley view */
	doc = PmwdWw(ww)->doc;
	/* default is cp for start of page */
	cpFirst = ((*hwwd)->ipgd == ipgdNil) ? cp0 : CpPlc(PdodDoc(doc)->hplcpgd, (*hwwd)->ipgd);
	idrMac = (*hwwd)->idrMac;
	for (idr = 0; idr < idrMac; idr++)
		{
		pdr = PdrWw(ww, idr);
		if (pdr->doc == doc && pdr->cpFirst != cpNil)
			{
/* find the first visible cp */
			FullDlBoundsDr(ww, hwwd, idr, &dlFirst, &cpT, 
					NULL, NULL);
			cpFirst = CpMin(cpFirst, cpT);
			}
		}
	cpFirst = CpMin(CpMacDocEdit(doc), cpFirst);

	psels = (ww == wwCur ? &selCur : &(*hwwd)->sels);
	if (doc != psels->doc)
		{
		psels->doc = doc;
		psels->sk = skIns;	/* ensure no funny business for pics etc. */
		if (ww == wwCur)
			SelectIns(&selCur, cpFirst );
		else
			psels->cpFirst = psels->cpLim = cpFirst;
		}
	cpSee = (ww == wwCur && FCaVisible(ww, &selCur.ca)) ? 
			selCur.cpFirst : cpFirst;

	FreeDrs(hwwd, 1); /* bug fix : do this after finding cpFirst above */
	FreezeHp();
	pwwd = *hwwd;
	pwwd->wk = wkDoc;
	Win(pwwd->grpfvisi.flm = FlmDisplayFromFlags(pwwd->fDisplayAsPrint, fFalse, vpref.fDraftView));
	Mac(Assert(pwwd->xwSelBar == 0));
	pwwd->rcwDisp.ywTop += dypMinWwInitSci;
	pwwd->rcwDisp.xwLeft = pwwd->xwSelBar;
	pwwd->sels.doc = doc;
	Mac(vwcc.wwSet = wwNil);
	MeltHp();
	InitGalleyDr(ww);
	FreezeHp();
	pwwd = *hwwd;
	pdod = PdodDoc(doc);
	Win(Assert(vflm != flmRepaginate && vflm != flmPrint));

	PrcSet(&(pwwd->rcePage), dxwSelBarSci /* Left */,
			0 /* Top */, 
			(pwwd->rcePage.xeLeft + 
			DxsFromDxa(pwwd, pdod->dop.xaPage - pdod->dop.dxaLeft)) /* Right */,
			ypMaxAll /* Bottom */);

#ifdef WIN
/* so that style name border above rcwDisp.ywMin is drawn */
	if (pwwd->xwSelBar > 0)
		{
		rcT = pwwd->rcwDisp;
		rcT.ywBottom = rcT.ywTop;
		rcT.ywTop = 0;
		InvalidateRect(pwwd->hwnd, (LPRECT)&rcT, fFalse);
		}
#endif
	/* reset horiz scrollbar */
	pwwd->xhScroll = 0;
#ifdef MAC
	pwwd->rcwDisp.xwLeft = 0;
#endif
	TrashWw(ww);
	InvalFli();
	PwwdSetWw(wwCur, cptAll);
	EraseRc(wwCur, &rcw);
#ifdef WIN /* restore styname area */
	if (pwwd->dxwStyWnd)
		{
		/* don't restore styname area if other pane is in pageview */
		if ((wwOther = WwOther(ww)) == wwNil || !PwwdWw(wwOther)->fPageView)
			ShowStyWnd(pwwd->mw, pwwd->dxwStyWnd, fTrue/*fSetDxw*/);
		}
#endif
	MeltHp();

#ifdef MAC
	if (vpsccBelow)
		{
		SynchSccWw(&vsccAbove, ww);
		SynchSccWw(vpsccBelow, ww);
		}
#endif
	pwwd = PwwdWw(ww);
	PdrGalley(pwwd)->cpFirst = cpFirst;
	PdrGalley(pwwd)->fCpBad = fTrue;
	pwwd->fDirty = fTrue;
	PdrGalley(pwwd)->dypAbove = 0;

	/* destroy the page up/down icons in vertical scroll bar */
	Win(DestroyPgvCtls(ww));
	Win(vpref.fPageView = fFalse);
	FCleanUpHdr(doc, fFalse/*fForce*/, fTrue/*fKillEmpty*/);
	SyncSbHor(ww);
	SetElevWw(ww);
#ifdef WIN
/* ruler may be holding onto non existing idr, set it to idr 0 */
	if (FRulerUp(ww))
		{
		HWND hwndRuler;
		struct RSD **hrsd;

		if ((hwndRuler = PmwdWw(ww)->hwndRuler) /* ! hNil */)
			{
			hrsd = HrsdFromHwndRuler(hwndRuler);
			(*hrsd)->idr = 0;
			}
#ifdef DEBUG
		else
			Assert(fFalse);
#endif
		}
#endif
	NormCp(ww, doc, cpSee, 0, 0, fFalse);

	return cmdOK;
}


   

/*********************************/
/* C m d   P a g e   V i e w  On */
/* %%Function:CmdPageViewOn %%Owner:chic */
CmdPageViewOn(ww, doc)
int ww;
int doc;
{
/* go into page view */
/* this routine can be called directly from open code to create a 
pageview window, so ww may not be wwCur yet */

	struct WWD **hwwd = HwwdWw(ww);
	int ipgd, fDirty;
	struct WWD *pwwd;
	CP cpSee;
#ifdef WIN
	int mw;
#endif

	if ((*hwwd)->fPageView)
		return cmdOK;
	if (ww == wwCur && selCur.fIns)
		TurnOffSel(&selCur);
#ifdef WIN
/* create the region handle for DrawBlankPage */
	if (vhrgnGrey == NULL)
		{
		if ((vhrgnGrey = CreateRectRgn(0, 0, 0, 0)) == NULL)
			return cmdNoMemory;
#ifdef DEBUG
		else
			LogGdiHandle(vhrgnGrey, 1025);
#endif /* DEBUG */
		}
	if (vhrgnPage == NULL)
		{
		if ((vhrgnPage = CreateRectRgn(0, 0, 0, 0)) == NULL)
			return cmdNoMemory;
#ifdef DEBUG
		else
			LogGdiHandle(vhrgnPage, 1024);
#endif /* DEBUG */
		}

/* some people think it is appropriate to close down the header pane */
	Assert(PwwdWw(ww)->wk != wkHdr);
	mw = PwwdWw(ww)->mw;
	if (PmwdMw(mw)->fSplit && PwwdWw(PmwdMw(mw)->wwLower)->wk == wkHdr)
		KillSplit(mpmwhmwd[mw], fTrue/*fLower*/, fFalse/*fAdjustWidth*/);
#endif /* WIN */
	Assert(PdodDoc(doc)->fMother);

	StartLongOp();

	/* update the fields in the document (as if printing) */
	AssertDo(FPrepareDocFieldsForPrint (doc, pfpNormal));

	pwwd = PwwdSetWw(wwCur, cptAll);
	fDirty = pwwd->fDirty;
	cpSee = (ww == wwCur && !fDirty && FCaVisible(ww, &selCur.ca)) ? 
			selCur.cpFirst : PdrGalley(pwwd)->cpFirst;

	PmwdWw(ww)->doc = doc;
	ipgd = IpgdPldrFromDocCp(ww, doc, cpSee);
	if (ipgd == ipgdNil)
		{
LRet:
		EndLongOp(fFalse/*fAll*/);
		InitGalleyDr(ww);
		return(cmdNoMemory);
		}
#ifdef WIN
	if (!FCreatePgvCtls(ww))
		{
		Beep();
		FreeDrs(hwwd, 1); 
		goto LRet;
		}
#endif
	FreezeHp();
	pwwd = *hwwd;
	pwwd->wk = wkPage;
	pwwd->ipgd = ipgd;
	pwwd->fSetElev = fTrue;
	pwwd->fDirty = fTrue; 
	pwwd->fDrDirty = fFalse;
	pwwd->fBackgroundDirty = fTrue;
#ifdef WIN
	pwwd->grpfvisi.flm = FlmDisplayFromFlags(pwwd->fDisplayAsPrint, fTrue, vpref.fDraftView);
	vpref.fPageView = fTrue;
	/* close down styname window after wkPage is set */
	if (pwwd->dxwStyWnd > 0)
		ShowStyWnd(mw, 0, fFalse/*fSetDxw*/);
#endif

	PrcSet(&(pwwd->rcePage), 0/*Left*/, 0/*Top*/,
			DxsFromDxa(pwwd, PdodDoc(doc)->dop.xaPage)/*Right*/,
			DysFromDya(PdodDoc(doc)->dop.yaPage)/*Bottom*/);

	/* no white space at top in page view */
	pwwd->rcwDisp.ywTop -= dypMinWwInitSci;
	pwwd->rcwDisp.xwLeft = 0;
	MoveRc(&pwwd->rcePage, XeLeftPage(ww), YeTopPage(ww));
	if (WwScrollWkTest(ww) != wwNil)
		MoveRc(&pwwd->rcePage, PwwdOther(ww)->xeScroll, pwwd->yeScroll);
	pwwd->xhScroll = pwwd->xeScroll;
	Mac(vwcc.wwSet = wwNil);

	MeltHp();

#ifdef WIN
	ShowPgvCtls(ww, fFalse /*fMove*/);
	/* force all pageview panes to match this pane's grpfvisi */
	SyncPageViewGrpfvisi(DocMother(doc), (*hwwd)->grpfvisi, (*hwwd)->fDisplayAsPrint);
#endif
	TrashRuler();

	if (!fDirty)
		NormCp(ww, doc, cpSee, 0, 0, fFalse);
	/* reset horiz scrollbar since changing mode */
	if (ww == wwCur)
		SyncSbHor(ww);

	if (vmerr.fNoHdrDr)
		ErrorEid(eidComplexSession, "CmdPageViewOn");
	EndLongOp(fFalse);
	return cmdOK;

}


#ifdef NOTUSED
/*****************************/
/* C p   F i r s t   P a g e */
/* %%Function:CpFirstPage %%Owner:NOTUSED */
CP CpFirstPage(doc, cp, tDirection)
int doc;
CP cp;
int tDirection;    /* tri-state flag: <0 for prev, 0 for cur, >0 for next */
{
/* locates CP at start of next or previous page relative to the page which
	contains cp */
	struct PLC **hplcpgd;
	struct LBS lbsText, lbsFtn;

	hplcpgd = PdodDoc(doc)->hplcpgd;
	if (tDirection == 0)
		return(CpPlc(hplcpgd, IInPlc(hplcpgd, cp)));
	if (IpgdRepagToCpIpgd(wwCur, doc, cp, ipgdNil, &lbsText, &lbsFtn) != ipgdNil)
		cp = CpFromCpCl(&lbsText, fTrue);
	EndLayout(&lbsText, &lbsFtn);
	return((tDirection > 0) ? cp : CpPlc(hplcpgd, max(0, IInPlc(hplcpgd, cp) - 1)));
}


#endif


/*********************************************/
/* I p g d   P l d r   F r o m   D o c   C p */
/* %%Function:IpgdPldrFromDocCp %%Owner:chrism */
IpgdPldrFromDocCp(ww, doc, cp)
int ww, doc;
CP cp;
{
/* makes a pldr describing the page containing doc,cp in wwd */

/* special cases for short docs */
	if (PdodDoc(doc)->dk == dkDispHdr)
		/* for header, show same page */
		return(IpgdPldrFromIpgd(ww, PwwdWw(ww)->ipgd));
	Assert(PdodDoc(doc)->fFtn || !PdodDoc(doc)->fShort);

	return IpgdPldrFromWwDocCpIpgd(ww, doc, cp, ipgdNil);
}


/*****************************************/
/* I p g d   P l d r   F r o m   I p g d */
/* %%Function:IpgdPldrFromIpgd %%Owner:chrism */
IpgdPldrFromIpgd(ww, ipgd)
int ww, ipgd;
{
/* makes a pldr describing page ipgd */

	return IpgdPldrFromWwDocCpIpgd(ww, PmwdWw(ww)->doc, cp0, ipgd);
}


/*******************************************************/
/* I p g d  P l d r  F r o m  W w  D o c  C p  I p g d */
/* %%Function:IpgdPldrFromWwDocCpIpgd %%Owner:chrism */
IpgdPldrFromWwDocCpIpgd(ww, doc, cp, ipgd)
int ww, doc;
CP cp;
int ipgd;
{
/* makes a pldr describing page ipgd or the page containing doc,cp in wwd */
	int ipgdT;
	struct LBS lbsText, lbsFtn;
	struct WWD *pwwd;

/* note that a WWD is also a pldr */
	if ((ipgdT = IpgdRepagToCpIpgd(ww, doc, cp, ipgd, &lbsText, &lbsFtn)) != ipgdNil)
		if (!FPldrFromPllr(doc, HwwdWw(ww), ww, &lbsText))
			ipgdT = ipgdNil;
		else
			PwwdWw(ww)->fNewDrs = fTrue;	/* set fFalse after 1st update */
	EndLayout(&lbsText, &lbsFtn);
	FCleanUpHdr(doc, fFalse/*fForce*/, fTrue/*fKillEmpty*/);

	pwwd = PwwdWw(ww);
	pwwd->ipgd = ipgdT;
	if (!(pwwd->fDrDirty = (ipgdT == ipgdNil)))
		pwwd->fDirty = pwwd->fBackgroundDirty = fTrue;
	return(ipgdT);
}


/*********************************************/
/* I p g d   R e p a g   T o   C p   I p g d */
/* %%Function:IpgdRepagToCpIpgd %%Owner:chrism */
int IpgdRepagToCpIpgd(ww, doc, cp, ipgd, plbsText, plbsFtn)
int ww, doc;
CP cp;
int ipgd;       /* if != ipgdNil, ignore cp and repag to ipgd */
struct LBS *plbsText, *plbsFtn;
{
/* repaginate up to the page containing cp, such that plbsText contains
	the LRs for that page */
	int ipgdT, ipgdMac, fFtn = PdodDoc(doc)->fFtn;
	struct PLC **hplcpgd;
	struct RPL rpl;
	struct PGD pgd;
	CP cpPgd;
	int lmSave = vlm;
#ifdef WIN
	int flmSave = vflm;
#else
	int fFormatVisiSave;
#endif

/* ensure that we get LRs by dirtying the page we need */
	if (ipgd == ipgdNil)
		cp = CpMin(cp, CpMacDocEdit(doc));
	Assert(fFtn || !PdodDoc(doc)->fShort);
	if ((hplcpgd = PdodDoc(doc)->hplcpgd) != hNil)
		{
		if ((ipgdT = ipgd) == ipgdNil)
			{
			ipgdT = IInPlcMult(hplcpgd, cp);
			/* We now have dcpDepend */
			if (ipgdT > 0)
				{
				GetPlc(hplcpgd, ipgdT, &pgd);
				if (pgd.cl != 0)
					{
					if (!pgd.fUnk)
						{
						pgd.fUnk = fTrue;
						PutPlcLast(hplcpgd, ipgdT, &pgd);
						}
					ipgdT--;
					}
				}
			}
		ipgdMac = IMacPlc(hplcpgd);
		ipgdT = max(0, min(ipgdT, ipgdMac - 1));
		for (cpPgd = CpPlc(hplcpgd, ipgdT);
				ipgdT < ipgdMac && cpPgd == CpPlc(hplcpgd, ipgdT);
				ipgdT++)
			{
			GetPlc(hplcpgd, ipgdT, &pgd);
			if (!pgd.fUnk)
				{
				pgd.fUnk = fTrue;
				PutPlcLast(hplcpgd, ipgdT, &pgd);
				}
			}
		}

/* this repagination leaves lbsText describing the page starting with or
	containing the desired cp */
#ifdef MAC
	InitFliForLayout(lmPagevw, &fFormatVisiSave);
#else
	if (!FInitWwLbsForRepag(ww, DocMother(doc), lmPagevw, plbsText, plbsFtn))
		return ipgdNil;
#endif
	StartLongOp();
	SetWords(&rpl, pgnMax, cwRPL);
	if (ipgd == ipgdNil)
		rpl.cp = cp;
	else
		rpl.ipgd = ipgd;
	Mac( rpl.crs = crsHourglass );
	plbsText->hpllr = plbsFtn->hpllr = hNil;
	if (FUpdateHplcpgd(ww, doc, plbsText, plbsFtn, &rpl, patmDelayReport))
		{
		/* flmDisplay is the Printerless Pageview */
		Win(Assert(vflm == flmRepaginate || vflm == flmDisplay));
		Win(Assert(plbsText == 0 || plbsText->ww == wwLayout));

/* format desired page if not described by lbsText */
		if (!vmerr.fMemFail && (ipgd != ipgdNil ? rpl.ipgd == ipgd :
				((fFtn ? plbsFtn->cp : CpFromCpCl(plbsText, fTrue)) <= cp)))
			{
			Debug(int fPageFormattedTwice = fFalse);
LFormatAgain:
			if (LbcFormatPage(plbsText, plbsFtn) == lbcAbort)
				{
				ipgd = ipgdNil;
				goto LRet;
				}
			SetPgdValid(rpl.ipgd, plbsText, plbsFtn, fTrue);
			if (plbsText->fEmptyPage)
				{
				/* section break caused empty page */
				Assert(!fPageFormattedTwice);
				Debug(fPageFormattedTwice = fTrue);
				++rpl.ipgd;
				goto LFormatAgain;
				}
			Win(Assert(vflm == flmRepaginate || vflm==flmDisplay));
			}
		}
	else
		{
/* in case rpl.ipgd is still pgnMax when FUpdateHplcpgd failed due to printer
failure that MAC does not have */
		rpl.ipgd = ipgdNil;
		}

	ipgd = rpl.ipgd;
LRet:
	EndFliLayout(lmSave, WinMac(flmSave, fFormatVisiSave));
	EndLongOp(fFalse);
	return(ipgd);
}


#define dxpOutLeftMin   0
#define dxpOutRightMin  1

/***********************************/
/* F   P l d r   F r o m   P l l r */
#ifndef WIN
NATIVE /* WINIGNORE - !WIN */
#endif /* !WIN */
/* %%Function:FPldrFromPllr %%Owner:chrism */
int FPldrFromPllr(doc, hpldr, ww, plbsText)
int doc;
struct PLDR **hpldr;
int ww;
struct LBS *plbsText;
{
/* collect layout rectangles into display rectangles of same section
	and column.  Note: for Opus, everything is done in printer units
	and then converted to screen units at the very end; this avoids
	off-by-1 conversion/round-off problems. */
/* assumes vcpFirstLayout is still correct (EndLayout not called) */

/* note: we know no one's using these dr's, because we're in the process
	of building them; we know they're near because they're wwd dr's */

	int ilr, ilrMac, ilrT;
	int idr, idrMac, idrT;
	int ylMic, ylPageBottom, ylT, xlLeft, xlRight, dxlCol, dxl;
	int dxuColumn;
	int idrPrevText, idrPrevFtn, dk;
	int dxlBetween, xaPage;
	int ccol, idrNotHdr = 0x7fff;
	int fFtr, fHdr, fFloat, fFtnSep;
	struct PLLR **hpllr = plbsText->hpllr;
	CP cpHdr;
	struct DOD *pdod;
	struct DOP *pdop;
	struct DR *pdr, *pdrT;
	LRP lrp;
	struct PLC **hplchdd;
	struct CA caSectCur;
	struct DRC drcl;
	struct RC rcl, rclAbs;
	struct DR dr;
	struct LR lr;
	struct LBS lbs;
#ifdef WIN
	struct RC rcDummy;
#endif /* WIN */

	Assert(vpdrfHead == NULL);
	Assert(!(*hpldr)->fExternal);
	Assert(vcpFirstLayout != cpNil);

	doc = DocMother(doc); /* works with mother doc */
	vmerr.fNoHdrDr = fFalse;
	if ((ilrMac = (*hpllr)->ilrMac) == 0)
		return(fFalse);

	FreezeHp();
	pdod = PdodDoc(doc);
	Assert(!pdod->fShort);
	pdop = &pdod->dop;
#ifdef MAC
	ylPageBottom = DysFromDya(pdop->yaPage);
	ylMic = ylPageBottom - DysFromDya(abs(pdop->dyaBottom));
#else
	ylPageBottom = DyuFromDya(pdop->yaPage);
	ylMic = ylPageBottom - DyuFromDya(abs(pdop->dyaBottom));
#endif
	MeltHp();

	FreeDrs(hpldr, 0);
	SetWords(&dr, 0, sizeof(struct DR) / sizeof(short));
	SetWords(&lbs, 0, cwLBS);
	for (idrMac = ilr = 0; ilr < ilrMac; ++ilr)
		{
		bltLrp(LrpInPl(hpllr, ilr), &lr, sizeof(struct LR));
		if (ilr == 0 && ilrMac == 1 && lr.cp == cpNil)
			{
			/* extreme error case; no cps */
			struct WWD *pwwd = PwwdWw(ww);
			pwwd->ipgd = ipgdNil;
			return(fFalse);
			}
		if (lr.doc >= 0)
			{
			RawLrCps(&lbs, &lr, WinMac(wwLayout, ww));
			CacheSect(lr.doc, lr.cp);
			caSectCur = caSect;
			}

/* check if we have a DR that contains this LR */
		if (idrMac != 0 && lr.doc >= 0)
			{
			/* no ghost DRs with simple LRs and APOs */
			if (lr.lrk == lrkAbsHit && plbsText->fSimpleLr)
				{
				for (pdr = PInPl(hpldr, idr = 0); idr < idrMac; )
					{
					if (pdr->xl > lr.xl && pdr->cpFirst == cpNil)
						{
						DeleteFromPl(hpldr, idr);
						idrMac--;
						}
					else
						{
						idr++;
						pdr++;
						}
					}
				}
			if (lr.doc > 0 && !PdodDoc(lr.doc)->fShort &&
					caSectCur.cpFirst == lr.cp)
				/* new dr for beginning of section */
				goto LNewDr;

			for (pdr = PInPl(hpldr, idr = idrMac - 1); idr >= 0; --pdr, --idr)
				{
				if (lr.ihdt == ihdtNil && lr.doc != pdr->doc)
					continue;
				if (pdr->cpFirst == cpNil)
					{
					if (lr.xl >= pdr->xl)
						break;
					continue;
					}

				if (plbsText->fSimpleLr && !lr.fChain &&
						(lr.ihdt == ihdtNil || dr.ihdt == lr.ihdt) &&
						lr.xl == pdr->xl && lr.dxl == pdr->dxl &&
						lr.lrk != lrkAbs && pdr->lrk != lrkAbs && 
						Mac( lr.lrk != lrkSxs && pdr->lrk != lrkSxs && )
						lr.yl > pdr->yl && lr.cp == pdr->cpLim)
					{
					/* pdr->dyl usually goes to bottom */
					if (pdr->lrk == lrkAbsHit && 
							pdr->yl + pdr->dyl != lr.yl)
						{
						continue;
						}
					if (lr.lrk == lrkAbsHit)
						{
						pdr->dyl = lr.yl + lr.dyl - pdr->yl;
						pdr->lrk = lrkAbsHit;
						}
					else  if (pdr->yl + pdr->dyl == lr.yl)
						pdr->dyl += lr.dyl;
					else  if (pdr->yl + pdr->dyl < lr.yl)
						continue;
#ifdef DEBUG
					if (lr.ihdt != ihdtNil)
						{
						Assert(pdr->doc == DocDispFromDocCpIhdt(doc, vcpFirstLayout, lr.ihdt));
						}
#endif
					AdjustPdrDyl(hpldr, &xlRight, idr, &idrMac);
					goto LAppendCp;
					}
				}
			if (idr < 0)
				goto LNewDr;
			Assert(idr > 0);
			if (lr.lrk == lrkAbsHit || FSxsLrk(lr.lrk) || lr.yl != pdr->yl)
				{
				ylT = pdr->yl;
				/* an absolute object is affecting this
					column, or chopped a natural dr so the
					heights don't match; delete the empty dr
					and all its clones */
				FreezeHp();
				DeleteFromPl(hpldr, idr);
				for (idrMac--; idr < idrMac; idrMac--)
					{
					if (ylT != pdr->yl || lr.doc != pdr->doc || pdr->cpFirst != cpNil)
						break;
					DeleteFromPl(hpldr, idr);
					}
				MeltHp();
				goto LNewDr;
				}
			if (lr.xl != pdr->xl)
				goto LNewDr;
			pdr->cpFirst = lr.cp;
			pdr->lrk = lr.lrk;
LAppendCp:
			pdr->cpLim = lr.cpLim;
			pdr->fLimSuspect = fFalse;
			continue;       /* next LR */
			} /* end - check if we have a DR that contains this LR */

/* now we know this LR doesn't match a prefabricated DR - create a DR for
	each column */
LNewDr:         
		SetWords(&dr, 0, cwDR);
		dr.doc = lr.doc;
		dr.cpFirst = lr.cp;
		dr.yl = lr.yl;
		dr.xl = lr.xl;
		dr.fDirty = fTrue;
		dr.fIncomplete = fTrue;
		dr.fNoPrev = fTrue;
		dr.fForceFirstRow = lr.fForceFirstRow;
		/* dr.fLimSuspect = fFalse; *//* done by SetWords */
		/* dr.fNewColOnly = fFalse; *//* done by SetWords */
		dr.lrk = lr.lrk;
		dr.idrFlow = idrNil;
		dr.fConstrainLeft = lr.fConstrainLeft;
		dr.fConstrainRight = lr.fConstrainRight;
		dr.fSpaceBefore = lr.fSpaceBefore;
		dr.ihdt = lr.ihdt;
		if (lr.doc < 0)
			{
			/* auto pgn, ftn sep,  or other fSpec character */
			int dxl = lr.dxl;
			int dxlMin = dxl;
#ifdef WIN
			if (vfti.dxpInch == vfli.dxsInch)
				{
				/* convert from printer to screen unit to be passed to FFormatLineFspec */
				dxl = NMultDiv(lr.dxl, vfli.dxsInch, vfli.dxuInch);
				}
#endif
			if (!FFormatLineFspec(ww, doc, dxl, -lr.doc))
				break;
#ifdef WIN
			if (vfti.dxpInch == vfli.dxsInch)
				{ /* get back printer unit */
				dr.dxl = NMultDiv(vfli.xpRight, vfli.dxuInch, vfli.dxsInch);
				dr.dyl = NMultDiv(vfli.dypLine, vfli.dyuInch, vfli.dysInch);
				}
			else  /* already in printer unit, no need to convert */				
				{
				dr.dxl = vfli.xpRight;
				dr.dyl = vfli.dypLine;
				}
			dr.dxl = min(dxlMin, dr.dxl); /* limit to column width */
#else
			dr.dxl = vfli.xpRight;
			dr.dyl = vfli.dypLine;
#endif
			dr.cpLim = vfli.cpMac;
			if (!FInsertInPl(hpldr, idrMac++, &dr))
				goto LRetFalse;
			if (idrNotHdr <= idrMac && idrMac > 1)
				AdjustPdrDyl(hpldr, &xlRight, idrMac - 1, &idrMac);
			continue;
			}

		CachePara(lr.doc, lr.cp);
		/* save section values for tolerance */
		CacheSect(lr.doc, lr.cp);
		dr.dxaBetween = vsepFetch.dxaColumns;
		dr.ccolM1 = vsepFetch.ccolM1;
		dr.fForceWidth = lr.lrk != lrkNormal && !FSxsLrk(lr.lrk);
		fFtr = fHdr = fFloat = fFtnSep = fFalse;
		if (lr.ihdt != ihdtNil && !vmerr.fNoHdrDr)
			{
			int docFreeLastT;
			/* header/footer - create display doc */
			if (FHeaderIhdt(lr.ihdt))
				fHdr = fTrue;
			else
				fFtr = fTrue;
			/* first see if display doc already exists; otherwise create one */
			if ((dr.doc = DocDispFromDocCpIhdt(doc, vcpFirstLayout, lr.ihdt)) == docNil)
				{
				/* leave 3 for clsplc, one for hdr, one for ftr,
					and another pair for background repag */
				if (CFreeCls(clsDOD, &docFreeLastT) + dncls[clsDOD].fooMax -
						*(dncls[clsDOD].pfooMac) <= 7 ||
						(dr.doc = DocDisplayHdr(doc, vcpFirstLayout, lr.ihdt)) == docNil)
					{
					vmerr.fNoHdrDr = fTrue;
					continue;
					}
				NewTextHdr(doc, vcpFirstLayout, dr.doc, lr.ihdt, wwNil, fFalse);
				}
			else if (PdodDoc(dr.doc)->fGetHdr) /* fetch new text due to field calculation */
				NewTextHdr(doc, vcpFirstLayout, dr.doc, lr.ihdt, wwNil, fFalse);

			pdod = PdodDoc(lr.doc);
			hplchdd = pdod->hplchdd;
			cpHdr = CpPlc(hplchdd, IInPlc(hplchdd, lr.cp));
			dr.cpFirst = lr.cp - cpHdr;
			lr.cpLim -= cpHdr;
			if (lr.ihdt >= ihdtMaxSep)
				{
				/* don't treat ftn separators as headers */
				fHdr = fFtr = fFalse;
				fFtnSep = fTrue;
				dr.fForceWidth = fTrue;
				}
			else
				{
				pdod = PdodMother(lr.doc);
				fFloat = ((fHdr) ? pdod->dop.dyaTop : pdod->dop.dyaBottom) < 0;
				}
			}
		else  if (idrNotHdr == 0x7fff)
			idrNotHdr = idrMac;
		dr.cpLim = lr.cpLim;
		pdod = PdodMother(dr.doc);
		xaPage = pdod->dop.xaPage;
		GetXlMargins(&pdod->dop, vfmtss.pgn & 1, WinMac(vfli.dxuInch,DxsSetDxsInch(dr.doc)), &xlLeft, &xlRight);
		ccol = 1;
		pdod = PdodDoc(dr.doc);
		if (dr.lrk == lrkNormal && lr.ihdt == ihdtNil)
			{
			/* it's necessary to set ccol here for fFtn so 
				dxpOutRight gets set; but it's set to 1 below
				so there are no ghost drs */
#ifdef WIN /* avoid round up or else may cause ccol adding 0 */
			dxuColumn = (int)((long)(vsepFetch.dxaColumnWidth + vsepFetch.dxaColumns) * (long)vfli.dxuInch / (long)dxaInch);
#else
			dxuColumn = NMultDiv(vsepFetch.dxaColumnWidth + vsepFetch.dxaColumns, vfli.dxsInch, dxaInch);
#endif
			if (dxuColumn > 0)
				{
				/* add an extra space between columns to right to avoid
				   rounding down */
				dxl = xlRight - lr.xl - lr.dxl + 1 + 
#ifdef WIN
					(int)((long)(vsepFetch.dxaColumns) * (long)vfli.dxuInch / (long)dxaInch);
#else
					NMultDiv(vsepFetch.dxaColumns, vfli.dxsInch, dxaInch);
#endif
				ccol += min(vsepFetch.ccolM1, max(0, dxl / dxuColumn));
				}

			}
		if (dr.lrk == lrkAbs)
			{
			CachePara(dr.doc, dr.cpFirst);
			dr.dxpOutRight = dr.dxpOutLeft = NMultDiv(vpapFetch.dxaFromText, WinMac(vfli.dxuInch, vfli.dxsInch),
					 dxaInch);
			}
		else
			{
			dxlBetween = NMultDiv(vsepFetch.dxaColumns, WinMac(vfli.dxuInch, vfli.dxsInch), 2 * dxaInch) ;
			if (lr.fConstrainLeft || fFtnSep)
				dr.dxpOutLeft = dxpOutLeftMin;
#ifdef MAC
			else  if (vpapFetch.fSideBySide)
				{
				if ((dr.dxpOutLeft = -max(0, NMultDiv(vpapFetch.dxaLeft +
						min(0, vpapFetch.dxaLeft1),
						vfli.dxsInch, dxaInch))) == 0)
					goto LElse;
				}
#endif
			else
				{
LElse:
				dr.dxpOutLeft = (lr.xl == xlLeft) ? xlLeft : dxlBetween;
				}
			dr.dxpOutRight = (lr.fConstrainRight || fFtnSep) ?
					dxpOutRightMin : 
					(ccol > 1 || dr.lrk != lrkNormal && !FSxsLrk(dr.lrk)) ? dxlBetween :
					NMultDiv(xaPage, WinMac(vfli.dxuInch, vfli.dxsInch), dxaInch) - xlRight;
			if (pdod->fFtn)
				ccol = 1;
			}
		dr.fNoParaStart = (lr.ilrNextChain != ilrNil);
		dr.dxl = lr.dxl;
		dxlCol = NMultDiv(vsepFetch.dxaColumnWidth + vsepFetch.dxaColumns, WinMac(vfli.dxuInch, vfli.dxsInch), dxaInch);
		/* set dr height; don't let it go negative */
		if (dr.lrk != lrkNormal || (!fFtr && ylMic <= dr.yl))
			dr.dyl = lr.dyl;
		else
			dr.dyl = ((fFtr) ? ylPageBottom : ylMic) - dr.yl;

		for (;;)
			{
			/* create a dr for each column */
			/* in ghost dr's, stop after collision with apo's */
			if (dr.fNewColOnly)
				{
				for (lrp = LrpInPl(hpllr, ilrT = 0); ilrT < ilrMac; ++ilrT, ++lrp)
					{
					drcl = lrp->drcl;
					if (lrp->lrk == lrkAbs && FSectDrc(&dr.drcl, &drcl))
						goto LEndCol;
					}
				}
			FInsertInPl(hpldr, idrMac++, &dr);
			if (vmerr.fMemFail)
				{
LRetFalse:
				(*hpldr)->idrMac = 0;
				vmerr.fNoDrs = fTrue;
				return(fFalse);
				}
			/* adjust dr's that we made too tall, overlapping this one */
			if (idrNotHdr <= idrMac && idrMac > 1 && !fFloat)
				AdjustPdrDyl(hpldr, &xlRight, idrMac - 1, &idrMac);
			if (--ccol <= 0)
				break;
			dr.xl += dxlCol;
			dr.cpFirst = cpNil;
			dr.fLimSuspect = fTrue;
			dr.fConstrainLeft = dr.fConstrainRight = fFalse;
			dr.lrk = lrkNormal;
			dr.dxpOutLeft = dxlBetween;
			dr.dxpOutRight = (ccol > 1) ? dxlBetween : NMultDiv(xaPage, WinMac(vfli.dxuInch, vfli.dxsInch), dxaInch) - xlRight;
			dr.fNewColOnly = fTrue;
			}
LEndCol:
		pdod = PdodMother(dr.doc);
		if (fFtr && (dr.lrk == lrkNormal || FSxsLrk(dr.lrk)) && !fFloat)
			{
			/* footer - adjust ylMic */
			Break2();
			ylMic = min(ylMic, dr.yl);
			}
		}

/* set flow for all dr's */
/* NOTE: this relies on all text and ftn LRs being in CP order (except APOs) */
	idrPrevText = idrPrevFtn = idrNil;
	for (pdr = PInPl(hpldr, idr = 0); idr < idrMac; idr++, pdr++)
		{
		if (pdr->lrk == lrkAbs || pdr->doc < 0)
			continue;
		pdod = PdodDoc(pdr->doc);
		if ((dk = pdod->dk) == dkDispHdr)
			continue;
		if (dk == dkFtn)
			{
			if (idrPrevFtn != idrNil)
				{
				pdr->fNoPrev = fFalse;
				pdrT = PInPl(hpldr, idrPrevFtn);
				pdrT->idrFlow = idr;
				}
			idrPrevFtn = idr;
			continue;
			}
		Assert(idrPrevText != idrNil || pdr->cpFirst != cpNil);
		if (idrPrevText != idrNil)
			{
			if (pdr->cpFirst == cpNil)
				/* ghost dr */
				goto LSetFlowForNil;
			CacheSect(pdr->doc, pdr->cpFirst);
			pdr = PInPl(hpldr, idr);
			if (pdr->cpFirst == pdr->cpLim)
				{
				Assert(pdr->lrk == lrkAbsHit);
				/* cpFirst will be set to cpNil below */
				if (pdr->cpFirst == caSect.cpFirst)
					/* this dr belongs to previous section */
					pdr->cpFirst = pdr->cpLim = caSect.cpFirst - 1;
LSetFlowForNil:
				pdr->fNoPrev = fFalse;
				pdrT = PInPl(hpldr, idrPrevText);
				pdrT->idrFlow = idr;
				}
			else
				{
				pdrT = PInPl(hpldr, idrPrevText);
				if (pdrT->cpFirst == cpNil || FInCa(pdrT->doc, pdrT->cpFirst, &caSect))
					{
					/* must be in same section to flow */
					pdrT->idrFlow = idr;
					pdr->fNoPrev = fFalse;
					}
				}
			}
		idrPrevText = idr;
		}
	if (idrPrevFtn != idrNil)
		{
		pdr = PInPl(hpldr, idrPrevFtn);
		pdod = PdodDoc(pdr->doc);
		/* note: one extra entry in plcfnd; one extra chEop at end */
		if (IInPlc(pdod->hplcfnd, pdr->cpFirst) < IMacPlc(pdod->hplcfnd) - 2 ||
				pdr->cpLim < CpMacDocEdit(pdr->doc) - ccpEop)
			{
			pdr->idrFlow = idrNextPage;
			}
		}
	if (idrPrevText != idrNil)
		{
		pdr = PInPl(hpldr, idrPrevText);
		pdr->idrFlow = idrNextPage;
		}

#ifdef WIN
/* convert to screen coordinates */
	for (pdr = PInPl(hpldr, idr = 0); idr++ < idrMac; pdr++)
		{
		int dyT;

		/* store dxa, play silly games so that round-off doesn't
		munge DRs which should exactly touch */
		pdr->dxa = NMultDiv(pdr->dxl, dxaInch, vfli.dxuInch);
		pdr->dxl = NMultDiv(pdr->xl+pdr->dxl, vfli.dxsInch,vfli.dxuInch);
		dyT = pdr->dyl;
		pdr->dyl = NMultDiv(pdr->yl+pdr->dyl, vfli.dysInch,vfli.dyuInch);
		pdr->xl = NMultDiv(pdr->xl, vfli.dxsInch, vfli.dxuInch);
		pdr->yl = NMultDiv(pdr->yl, vfli.dysInch, vfli.dyuInch);
		pdr->dxl = max(1,pdr->dxl - pdr->xl);
		pdr->dyl = max(NMultDiv(dyT,vfli.dysInch, vfli.dyuInch),pdr->dyl - pdr->yl);
		Assert(pdr->dypAbove == 0);
		pdr->dxpOutLeft = NMultDiv(pdr->dxpOutLeft, vfli.dxsInch, vfli.dxuInch);
		pdr->dxpOutRight = NMultDiv(pdr->dxpOutRight, vfli.dxsInch, vfli.dxuInch);
		}
#endif

/* last pass - check for dxpOutLeft and dxpOutRight running into absolute
	positioned objects; also set empty DRs to have cpFirst of cpNil */
	for (pdr = PInPl(hpldr, idr = 0); idr++ < idrMac; pdr++)
		{
		if (pdr->cpFirst == pdr->cpLim)
			{
			pdr->cpFirst = cpNil;
			pdr->cpLim = CpMacDoc(pdr->doc);
			pdr->fLimSuspect = fTrue;
			}
		if (!plbsText->fAbsPresent || pdr->lrk == lrkAbs)
			continue;
		DrcToRc(&pdr->drcl, &rcl);
		rcl.xlLeft -= pdr->dxpOutLeft;
		rcl.xlRight += pdr->dxpOutRight;
		for (pdrT = PInPl(hpldr, idrT = 0); idrT++ < idrMac; pdrT++)
			{
			if (pdrT->lrk != lrkAbs)
				continue;
			DrcToRc(&pdrT->drcl, &rclAbs);
			rclAbs.xlLeft -= pdrT->dxpOutLeft;
			rclAbs.xlRight += pdrT->dxpOutRight;
#ifdef MAC
			if (FSectRc(&rcl, &rclAbs))
#else /* WIN */
				if (FSectRc(&rcl, &rclAbs, &rcDummy))
#endif /* WIN */
					{
					if (pdrT->xl > pdr->xl)
						pdr->dxpOutRight = max(dxpOutRightMin, min(rclAbs.xlLeft - pdr->xl - pdr->dxl - 1,
								 pdr->dxpOutRight));
					else
						pdr->dxpOutLeft = max(dxpOutLeftMin, min(pdr->xl - rclAbs.xlRight - 1, pdr->dxpOutLeft));
					}
			}
		}

/* check that insertion point is not moved past cpMacDocEdit for headers;
   the NewTextHeader call above suppresses selection fixes to avoid updates */
	if (selCur.cpFirst > CpMacDocEdit(selCur.doc) && PdodDoc(selCur.doc)->dk == dkDispHdr)
		SelectIns(&selCur, CpMacDocEdit(selCur.doc));

	return(fTrue);
}


/*******************************/
/* A d j u s t   P d r   D y l */
#ifndef WIN
NATIVE /* WINIGNORE - !WIN */
#endif /* !WIN */
/* %%Function:AdjustPdrDyl %%Owner:chrism */
void AdjustPdrDyl(hpldr, pxlRight, idr, pidrMac)
struct PLDR **hpldr;
int *pxlRight, idr, *pidrMac;
{
/* note: we know no one's using these dr's, because we're in the process
	of building them; we know they're near because they're wwd dr's */
/* DR at *pidrMac is new */
	int xlRightNew;
	struct DR *pdr, *pdrNew, *pdrLim;
	struct DOD *pdod;
	int fHdrFtr, ihdt;

	Assert(vpdrfHead == NULL);
	Assert(!(*hpldr)->fExternal);
	pdr = PInPl(hpldr, 0);
	pdrLim = pdr + *pidrMac - 1;
	pdrNew = pdr + idr;
	if (pdrNew->doc <= 0)
		{
		fHdrFtr = fFalse;
#ifdef MAC
		if (pdrNew->doc == -chPage)
			return;
#endif
		}
	else
		{
		pdod = PdodDoc(pdrNew->doc);
/* this is a hdr or ftr, not footnote separaters */
		fHdrFtr = ((pdod->dk == dkDispHdr) && (pdod->ihdt < ihdtMaxSep));
		}
	xlRightNew = pdrNew->xl + pdrNew->dxl;

	for (; pdr < pdrLim; pdr++)
		{
		if (pdr->doc <= 0 || pdr == pdrNew)
			continue;
		if (pdr->lrk == lrkNormal && pdr->yl < pdrNew->yl && pdr->yl + pdr->dyl > pdrNew->yl)
			{
			*pxlRight = pdr->xl + pdr->dxl;
			if (pdrNew->doc <= 0)
				{
				/* note: ftn separators only affect same column */
				if (pdr->xl == pdrNew->xl)
					goto LTrimDrcl;
				continue;
				}
			if (pdr->xl >= pdrNew->xl && pdr->xl < xlRightNew ||
					*pxlRight > pdrNew->xl && *pxlRight <= xlRightNew)
				{
LTrimDrcl:
				if (pdr->cpFirst == cpNil)
					{
					/* bad ghost dr - delete it */
					FreezeHp();
					DeleteFromPl(hpldr, pdr - PInPl(hpldr, 0));
					MeltHp();
					if (pdrNew > pdr)
						pdrNew--;
					(*pidrMac)--;
					pdrLim--;
/* Decrement pdr to counter pdr++ in the for loop */
					pdr--;
					continue;
					}
				pdr->dyl = pdrNew->yl - pdr->yl;
				if (pdrNew->lrk == lrkAbs)
					pdr->fCantGrow = fTrue;
				else
					{
					/* can't grow free-floating truncated headers */
					pdod = PdodDoc(pdr->doc);
					if (pdod->dk == dkDispHdr)
						{
						ihdt = pdod->ihdt;
						pdod = PdodMother(pdr->doc);
						if ((FHeaderIhdt(ihdt) ? pdod->dop.dyaTop : pdod->dop.dyaBottom) < 0)
							pdr->fCantGrow = fTrue;
						}
					}
				}
			}
/* make header or footer smaller and frozen -- it overlaps other text */
		else  if (fHdrFtr && pdr->yl > pdrNew->yl && pdr->yl < pdrNew->yl + pdrNew->dyl)
			{
			*pxlRight = pdr->xl + pdr->dxl;
			if (pdr->xl >= pdrNew->xl && pdr->xl < xlRightNew ||
					*pxlRight > pdrNew->xl && *pxlRight <= xlRightNew)
				{
				pdrNew->dyl = pdr->yl - pdrNew->yl;
				pdrNew->fCantGrow = fTrue;
				}
			}
		}
}



#ifdef WIN

HWND vhwndPgvCtl = NULL; /* handle of pgv icon when clicked */

/* %%Function:TrackPgVwMouse %%Owner:chic */
TrackPgVwMouse( hwnd, pt, bcm )
HWND hwnd;
struct PT pt;
int bcm;
{
	HDC hdc = GetDC(hwnd);
	struct RC rc;
	int fHilite;

	SetCapture(hwnd);
	GetClientRect(hwnd, (LPRECT)&rc);
	if (hdc)
		InvertRect(hdc, (LPRECT)&rc);
	fHilite = fTrue;
	while (FStillDownReplay(&pt,fFalse))
		if (PtInRect((LPRECT)&rc, pt) != fHilite)
			{
			if (hdc)
				InvertRect(hdc, (LPRECT)&rc);
			fHilite = !fHilite;
			}
	ReleaseCapture();
	if (fHilite)
		{
		if (hdc)
			{
			InvertRect(hdc, (LPRECT)&rc);
			ReleaseDC(hwnd, hdc);
			}
		vhwndPgvCtl = hwnd;
		FExecCmd(bcm);
		vhwndPgvCtl = NULL;
		}
	else  if (hdc)
		ReleaseDC(hwnd, hdc);
}


/* %%Function:CmdPgvPrevPage %%Owner:chic */
CmdPgvPrevPage(pcmb)
CMB * pcmb;
{
	return CmdPgvNewPage(fFalse /*fNext*/);
}


/* %%Function:CmdPgvNextPage %%Owner:chic */
CmdPgvNextPage(pcmb)
CMB * pcmb;
{
	return CmdPgvNewPage(fTrue /*fNext*/);
}


/* %%Function:CmdPgvNewPage %%Owner:chic */
CmdPgvNewPage(fNext)
BOOL fNext;
{
	extern int  YeTopPage();

	int ipgdNew;
	int ww;

#ifdef RSH
	SetUatMode(uamNavigation);
	StopUatTimer();
#endif /* RSH */

	ww = (vhwndPgvCtl == NULL ? wwCur : 
			GetWindowWord(vhwndPgvCtl, offset(RSBS,ww)));
	if (ww == wwNil)
		return cmdError;

	if (!PwwdWw(ww)->fPageView || (ipgdNew = (fNext ? IpgdNextWw(ww, fTrue) :
			IpgdPrevWw(ww))) == ipgdNil)
		{
		beep();
		return cmdError;
		}
	StartLongOp();
	SetPageDisp(ww, ipgdNew, YeTopPage(ww), fFalse, fFalse);
	EndLongOp(fFalse /*fAll*/);
	return cmdOK;
}


#endif /* WIN */
