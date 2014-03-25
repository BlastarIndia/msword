#ifdef WIN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW

#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#endif

#include "word.h"
#ifndef MAC
DEBUGASSERTSZ           /* WIN - bogus macro for assert string */
#endif /* !MAC */
#include "ch.h"
#include "props.h"
#include "doc.h"
#include "disp.h"

#ifdef PCWORD
#include "chfile.h"
#include "layoutpc.h"
#include "plc.h"
#include "print.h"
#include "units.h"
#include "ww.h"
#endif /*PCWORD*/

#include "prm.h"
#include "sel.h"
#include "format.h"
#include "layout.h"

#ifdef PCWORD
#include "etc.h"
#endif /*PCWORD*/

#ifndef PCWORD
#include "outline.h"
#include "debug.h"
#endif /*!PCWORD*/

#ifdef MAC 
#include "cmd.h"
#include "qsetjmp.h"
#endif /*MAC*/

#ifdef WIN
#include "screen.h"
#include "field.h"
#endif /* WIN */


#ifdef PROTOTYPE
#include "layout1.cpt"
#endif /* PROTOTYPE */

#ifdef PCWORD
#define vsepFetch       vsepAbs
#define vpapFetch       vpapAbs
extern struct PRSU      vprsu;
#endif /* PCWORD */

/* E X T E R N A L S */

extern CP               vcpAbsMic;
extern struct CA        caHdt;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA        caParaL;
extern struct SEP       vsepFetch;
extern struct FLI       vfli;
extern struct MERR      vmerr;
extern struct HDT       vrghdt[];
extern struct FLS       vfls;
extern struct PAP       vpapFetch;
extern int              vlm;
extern ENV              venvLayout;
extern CP               vcpFirstLayout;
extern BOOL             vfInFormatPage;
extern struct PLLR      **vhpllrAbs;
extern int              vfRestartLayout;
extern struct LBS	*vplbsFrame;

#ifdef WIN
extern CP               vcpLimLayout;
extern struct FTI       vfti;

int *vmpihdtihdd;
#endif /* WIN */

/* means one of the headers contains absolute objects; headers must be 
	rebuilt for every page */
int vfAbsInHeaders;

/* remembers state of LRs in vrghdt */
int vfSimpleHdrLrs;

/* used to save hdr lbs while it's in progress */
struct LBS *vplbsFrameHdt;

#ifndef WIN
/* used to save side by side lbs while it's in progress */
struct LBS *vplbsFrameSxs;
#endif

/* top of current page (below headers) */
int	vylTopPage;

int	vcBalance;	/* current balancing iteration */

#ifndef PCWORD
#ifdef WIN
void SetHdts(); /* DECLARATION ONLY */
void FillInHdt(); /* DECLARATION ONLY */
#else /* !WIN */
EXPORT void SetHdts(); /* DECLARATION ONLY */
EXPORT void FillInHdt(); /* DECLARATION ONLY */
#endif /* !WIN */
#endif /* !PCWORD */


#ifdef MAC
/******************************/
/* A d d  H d r  F t r  P g n */
/* %%Function:AddHdrFtrPgn %%Owner:NOTUSED */
EXPORT void AddHdrFtrPgn(plbsText, cp, fTitlePage, fFacingLeft, pylTop, pylBottom, xlLeft) /* WINIGNORE - MAC only */
struct LBS *plbsText;
CP cp;
int fTitlePage, fFacingLeft, *pylTop, *pylBottom, xlLeft;
{
	int ilr, xlRight;
	int ihdtHdr, ihdtFtr;
	int dxlInch = DxsSetDxsInch(plbsText->doc);
	struct DOP *pdop;
	struct DOD *pdod;
	struct HDT *phdtHdr, *phdtFtr;
	struct LR lrPgn;

/* lay out the headers */
	if (vfAbsInHeaders || plbsText->fFacingPages || plbsText->fSimpleLr != vfSimpleHdrLrs)
		{
		/* these conditions can change header margins from page to page */
		caHdt.doc = docNil;
		}
	SetHdts(plbsText, cp);

/* determine what header/footer to use */
	if (fTitlePage)
		{
		ihdtHdr = ihdtTFirst;   /* first header/footer */
		ihdtFtr = ihdtBFirst;
		}
	else  if (!fFacingLeft)
		{
		/* the default header/footer */
		ihdtHdr = ihdtTRight;   /* right/odd header/footer */
		ihdtFtr = ihdtBRight;
		}
	else
		{
		/* left/even header/footer */
		if (FNilHdt(ihdtHdr = ihdtTLeft))
			ihdtHdr = ihdtTRight;   /* use default */
		if (FNilHdt(ihdtFtr = ihdtBLeft))
			ihdtFtr = ihdtBRight;   /* use default */
		}

/* add the headers/footers */
	if (!FNilHdt(plbsText->ihdtTop = ihdtHdr))
		{
		phdtHdr = &vrghdt[ihdtHdr];
		pdod = PdodDoc(plbsText->doc);  /* two lines for compiler */
		if (pdod->dop.dyaTop >= 0)
			*pylTop = max(*pylTop, phdtHdr->yl + phdtHdr->dyl);
		CopyHdtLrs(ihdtHdr, plbsText, phdtHdr->yl, 0);
		}
	if (!FNilHdt(plbsText->ihdtBottom = ihdtFtr))
		{
		phdtFtr = &vrghdt[ihdtFtr];
		pdod = PdodDoc(plbsText->doc);  /* two lines for compiler */
		if (pdod->dop.dyaBottom >= 0)
			*pylBottom = min(*pylBottom, phdtFtr->yl);
		CopyHdtLrs(ihdtFtr, plbsText, phdtFtr->yl, 0);
		}

/* handle auto page number */
	if (vsepFetch.fAutoPgn && !fTitlePage)
		{
		SetWords(&lrPgn, 0, cwLR);
		lrPgn.yl = YlFromYa(vsepFetch.dyaPgn);
		lrPgn.doc = -chPage; /* not a real doc, special char */
		if (fFacingLeft)
			{
			/* left with facing */
			lrPgn.xl = XlFromXa(vsepFetch.dxaPgn);
			lrPgn.dxl = lrPgn.dyl = 1; /* keep from being excised */
			}
		else  if (FFormatLineFspec(plbsText->ww, plbsText->doc, 0, chPage))
			{
			/* right or normal page */
			CacheSectL(plbsText->doc, vcpFirstLayout, plbsText->fOutline);
			pdop = &PdodDoc(plbsText->doc)->dop;
			xlRight = vfli.xpRight - vfli.rgdxp[vfli.ichMac - 1];   /* ignore eop */
			if (vfli.fPrint)
				xlRight = NMultDiv(xlRight, dxlInch, vfli.dxuInch);
			lrPgn.dxl = xlRight - vfli.xpLeft;
			lrPgn.xl = XlFromXa(pdop->xaPage - vsepFetch.dxaPgn) - lrPgn.dxl;
			lrPgn.dyl = vfli.dypLine;
			}
		ReplaceInPllr(plbsText->hpllr, IMacPllr(plbsText), &lrPgn);
		}
}


/******************/
/* S e t  H d t s */
/* %%Function:SetHdts %%Owner:NOTUSED */
EXPORT void SetHdts(plbs, cp) /* WINIGNORE - MAC only */
struct LBS *plbs;
CP cp;
{
/* computes hdt's for doc and section starting at cp in doc */
	int ihdt, ihdd;
	struct DOD *pdod;
	int fFtns;
	int grpfIhdt;
	int xlLeft, xlRight, dxl;
	struct CA caT;
	int mpihdtihdd[ihdtMaxSep];

	if (FInCa(plbs->doc, cp, &caHdt))
		return;

	vfAbsInHeaders = fFalse;
	pdod = PdodDoc(plbs->doc);
	fFtns = FFtnsPdod(pdod);
	grpfIhdt = pdod->dop.grpfIhdt;
	GetXlMargins(&pdod->dop, plbs->fRight, vfli.dxsInch, &xlLeft, &xlRight);

/* do document hdts */
	CacheSectL(plbs->doc, cp, plbs->fOutline);
	caHdt = caSect;
	vfSimpleHdrLrs = plbs->fSimpleLr;
	/* take width from starting section */
	dxl = NMultDiv(vsepFetch.dxaColumnWidth, vfli.dxsInch, dxaInch);
	for (ihdd = 0, ihdt = ihdtTFtn; ihdt < ihdtMax; ihdt++, grpfIhdt >>= 1)
		{
		/* bits in grpfIhdt are numbered  2, 1, 0 
			left to right, right flush */
		FillInHdt(plbs, xlLeft, dxl, cp0, ihdt,
				(!fFtns) ? ihddNil : (grpfIhdt & 1) ? ihdd++ :
				(ihdt == ihdtTFtnCont ? ihddTFtnCont :
				(ihdt == ihdtTFtn ? ihddTFtn : ihddNil)));
		}

/* do section hdts */
	SetWords(&mpihdtihdd, ihddNil, ihdtMaxSep);
	IhddFromDocCp(PcaSet(&caT, plbs->doc, cp0,
			cp + ((cp == caHdt.cpFirst) ? 1 : 0)), 0, mpihdtihdd);
	if (plbs->fOutline)
		caSect.doc = docNil;	/* CacheSect called directly */
#ifdef JR
	/* no "first" headers in Junior */
	mpihdtihdd[ihdtTFirst] = mpihdtihdd[ihdtBFirst] = ihddNil;
#endif
	for (ihdt = 0; ihdt < ihdtMaxSep; ihdt++)
		FillInHdt(plbs, xlLeft, xlRight - xlLeft, cp, ihdt, mpihdtihdd[ihdt]);
}


#endif /* MAC */


#ifdef WIN
/***************************/
/* F  A d d  H d r  F t r  */
/* %%Function:FAddHdrFtr %%Owner:chrism */
FAddHdrFtr(plbsText, cp, cHdrFtrLayoutAttempts, fTitlePage, fFacingLeft, pylTop, pylBottom, xlRight, xlLeft, pclrAdjust)
struct LBS *plbsText;
CP cp;
int cHdrFtrLayoutAttempts, fTitlePage, fFacingLeft;
int *pylTop, *pylBottom, xlRight, xlLeft, *pclrAdjust;
{
	int ilr, yl, fRedoPage, fSave;
	int ihdtHdr, ihdtFtr;
	struct DOP *pdop;
	struct DOD *pdod;
	struct HDT *phdtHdr, *phdtFtr;
	int iMacOld, iMacNew;
	struct PLLR **hpllr = plbsText->hpllr;

	Assert(hpllr);
	iMacOld = IMacPllr(plbsText);

	Assert(plbsText->ww != wwNil);
	fSave = PwwdWw(plbsText->ww)->fOutline;
	PwwdWw(plbsText->ww)->fOutline = fFalse;
/* find which header and footer go on this page */
	if (fTitlePage)
		{
		ihdtHdr = ihdtTFirst;   /* first header/footer */
		ihdtFtr = ihdtBFirst;
		}
	else  if (!fFacingLeft)
		{                               /* the default header/footer */
		ihdtHdr = ihdtTRight;   /* right/odd header/footer */
		ihdtFtr = ihdtBRight;
		}
	else
		{
		/* left/even header/footer */
		ihdtHdr = ihdtTLeft;
		ihdtFtr = ihdtBLeft;
		}

	/* these conditions can change header margins from page to page */
	if (vfAbsInHeaders || plbsText->fFacingPages)
		caHdt.doc = docNil;

	/* layout hdr/ftr/special ftn gunk for this page */
	SetHdts(plbsText, cp, &ihdtHdr, &ihdtFtr);

	/* add the headers/footers */
	fRedoPage = FALSE;
	if (!FNilHdt(plbsText->ihdtTop = ihdtHdr))
		{
		phdtHdr = &vrghdt[ihdtHdr];
		pdod = PdodDoc(plbsText->doc);  /* two lines for compiler */
		if (pdod->dop.dyaTop >= 0 && (yl = phdtHdr->yl + phdtHdr->dyl) > *pylTop)
			{
			fRedoPage = TRUE;
			*pylTop = yl;
			}
		else  if (vfAbsInHeaders)
			fRedoPage |= FAbsInPllr(phdtHdr->hpllr);
		}

	if (!FNilHdt(plbsText->ihdtBottom = ihdtFtr))
		{
		phdtFtr = &vrghdt[ihdtFtr];
		pdod = PdodDoc(plbsText->doc);  /* two lines for compiler */
		if (pdod->dop.dyaBottom >= 0 && (yl = phdtFtr->yl) < *pylBottom)
			{
			fRedoPage = TRUE;
			*pylBottom = yl;
			}
		else  if (vfAbsInHeaders)
			fRedoPage |= FAbsInPllr(phdtFtr->hpllr);
		}

	PwwdWw(plbsText->ww)->fOutline = fSave;

	Assert(!(fRedoPage && cHdrFtrLayoutAttempts < 2) || iMacOld == IMacPllr(plbsText));

	if (!FNilHdt(ihdtHdr))
		CopyHdtLrs(ihdtHdr, plbsText, phdtHdr->yl, 0);
	if (!FNilHdt(ihdtFtr))
		CopyHdtLrs(ihdtFtr, plbsText, phdtFtr->yl, 0);

	/*
		* if hdr or ftr moved top or bottom margin AND we haven't already
		* tried this, layout the page again but with the new margins.
		* If we've already tried this once and the either the top or
		* bottom margin changed again, let hdr/ftr overlap text.
		* NOTE: this has to be AFTER the CopyHdtLrs so APOs get copied
		*/
	if (fRedoPage && cHdrFtrLayoutAttempts < 2)
		{
		return(FALSE);
		}

	iMacNew = IMacPllr(plbsText);
	if (iMacNew > iMacOld)
		{
/* move hdr/ftr lrs to the beginning of the hpllr, so that MAC and WIN both
have hdr/ftr lrs come first to simplfly pageview code. pass back clrAdjust
so AlignLrs can adjust info in hgrpalg. */
		int cMac = iMacNew - iMacOld;
		int c;
		struct LR lrT;

		*pclrAdjust = cMac;
		ilr = iMacNew - 1;
		for (c = 0; c < cMac; c++)
			{
			bltLrp(LrpInPl(hpllr, ilr), &lrT, sizeof(struct LR));
			StartGuaranteedHeap();
			DeleteFromPl(hpllr, ilr);
			AssertDo(FInsertInPl(hpllr, 0, &lrT));
			EndGuarantee();
			}

		}
	return(fTrue);
}


/*****************************/
/* F   A b s   I n   P l l r */
/* %%Function:FAbsInPllr %%Owner:chrism */
int FAbsInPllr(hpllr)
struct PLLR **hpllr;
{
/* returns TRUE if the pllr contains an lrkAbs lr */
	int ilr, ilrMac;
	struct LR lr;

	for (ilr = 0, ilrMac = (*hpllr)->ilrMac; ilr < ilrMac; ilr++)
		{
		bltLrp(LrpInPl(hpllr, ilr), &lr, sizeof(struct LR));
		if (lr.lrk == lrkAbs)
			return(TRUE);
		}
	return(fFalse);
}


/*******************/
/* F  I h d t  G t */
/* %%Function:FIhddGt %%Owner:chrism */
FIhddGt(ihdt0, ihdt1)
int ihdt0, ihdt1;
{
	int ihdd0 = vmpihdtihdd[ihdt0];
	int ihdd1 = vmpihdtihdd[ihdt1];

	return ihdd0 > ihdd1;
}


#define ciihdtMax 3
/*************************/
/* S e t  H d t s */
/* %%Function:SetHdts %%Owner:chrism */
void SetHdts(plbs, cp, pihdtTop, pihdtBottom)
struct LBS *plbs;
CP cp;
int *pihdtTop, *pihdtBottom;
{
/* computes hdt's for division starting at cpFirst in doc. */
	int ihdt, ihdd, iihdt, fAbsInHeaders, fOutlineSave;
	struct DOD *pdod = PdodDoc(plbs->doc);
	int fFtns, grpfIhdt = pdod->dop.grpfIhdt;
	struct CA ca;
	int ciihdtMac = (pihdtTop != NULL ? 2 : 3);
	int xlLeft, xlRight, dxl, dxaColumnWidth;
	CP cpT;
	int rgihdt[ciihdtMax];
	int mpihdtihdd[ihdtMax];

	fFtns = FFtnsPdod(pdod);
	fAbsInHeaders = vfAbsInHeaders;
	GetXlMargins(&pdod->dop, plbs->fRight, vfti.dxpInch, &xlLeft, &xlRight);

	vfAbsInHeaders = fFalse;
	SetWords(&mpihdtihdd, ihddNil, ihdtMaxSep);
	CacheSectL(plbs->doc, cp, plbs->fOutline);
	dxaColumnWidth = vsepFetch.dxaColumnWidth;
	if (PdodDoc(plbs->doc)->docHdr != docNil)
		IhddFromDocCp(PcaSet(&ca, plbs->doc,
				cp0, cp + ((cp == caSect.cpFirst) ? 1 : 0)), 0, mpihdtihdd);
	for (ihdd = 0, ihdt = ihdtTFtn; ihdt < ihdtMax; ihdt++, grpfIhdt >>= 1)
		{
		mpihdtihdd[ihdt] = !fFtns ? ihddNil : ((grpfIhdt & 1) ? ihdd++ :
				(ihdt == ihdtTFtnCont ? ihddTFtnCont :
				(ihdt == ihdtTFtn ? ihddTFtn : ihddNil)));
		}

#ifdef BOGUS /* this is causing odd header to show up even when we meant to have a null even/first header (cc) */
	/* if header is not default (top right) and is empty, use default */
	if (pihdtTop != NULL && *pihdtTop != ihdtTRight)
		{
		ihdd = mpihdtihdd[*pihdtTop];
		if (ihdd == ihddNil)
			goto LUseDefHdr;
		CaFromIhddSpec(plbs->doc, ihdd, &ca);
		if (ca.cpFirst == ca.cpLim)
			{
LUseDefHdr:     
			*pihdtTop = ihdtTRight;
			}
		}

	/* if footer is not default (bottom right) and is empty, use default */
	if (pihdtBottom != NULL && *pihdtBottom != ihdtBRight)
		{
		ihdd= mpihdtihdd[*pihdtBottom];
		if (ihdd== ihddNil)
			goto LUseDefFtr;
		CaFromIhddSpec(plbs->doc, ihdd, &ca);
		if (ca.cpFirst == ca.cpLim)
			{
LUseDefFtr:     
			*pihdtBottom = ihdtBRight;
			}
		}
#endif

	if (pihdtTop != NULL)
		{ /* do section hdts */
		Assert(pihdtBottom != NULL);
		rgihdt[0] = *pihdtTop;
		rgihdt[1] = *pihdtBottom;
		dxl = xlRight - xlLeft;
		}
	else
		{ /* do dod hdts */
		rgihdt[0] = ihdtTFtn;
		rgihdt[1] = ihdtTFtnCont;
		rgihdt[2] = ihdtBFtnCont;
		/* take width from section */
		dxl = NMultDiv(dxaColumnWidth, vfti.dxpInch, dxaInch);
		}

	vmpihdtihdd = mpihdtihdd;
	Sort(rgihdt, ciihdtMac, FIhddGt);

	for (iihdt = 0; iihdt < ciihdtMac; iihdt++)
		{
		ihdt = rgihdt[iihdt];
		if (ihdt >= ihdtMaxSep)
			{
			fOutlineSave = PwwdWw(plbs->ww)->fOutline;
			PwwdWw(wwLayout)->fOutline = fFalse;
			cpT = cp0;
			}
		else
			cpT = cp;
		FillInHdt(plbs, xlLeft, dxl, cpT, ihdt, mpihdtihdd[ihdt]);
		if (ihdt >= ihdtMaxSep)
			PwwdWw(plbs->ww)->fOutline = fOutlineSave;
		}
	vfAbsInHeaders |= fAbsInHeaders;
}


/* S O R T */
/* sorts the array rgw with respect to FGt */
/* %%Function:Sort %%Owner:chrism */
Sort(rgw, iwMac, FGt)
int *rgw, iwMac, (*FGt)();
{
	int iw;
	if (iwMac < 2) return;
	for (iw = iwMac>>1; iw >= 2; --iw)
		SortSiftUp(rgw, iw, iwMac, FGt);
	for (iw = iwMac; iw >= 2; --iw)
		{
		int w;
		SortSiftUp(rgw, 1, iw, FGt);
		w = rgw[0];
		rgw[0] = rgw[iw - 1];
		rgw[iw - 1] = w;
		}
}


/* S O R T  S I F T  U P */
/* see Floyd, Robert W. Algorithm 245 TREESORT 3 [M1] CACM 7, December 1964. */
/* %%Function:SortSiftUp %%Owner:chrism */
SortSiftUp(rgw, iwI, iwN, FGt)
int *rgw, iwI, iwN, (*FGt)();
{
	int iwJ;
	int wCopy;

	wCopy = rgw[iwI - 1];
Loop:
	iwJ = 2 * iwI;
	if (iwJ <= iwN)
		{
		if (iwJ < iwN)
			{
			if ((*FGt)(rgw[iwJ], rgw[iwJ - 1]))
				iwJ++;
			}
		if ((*FGt)(rgw[iwJ - 1], wCopy))
			{
			rgw[iwI - 1] = rgw[iwJ - 1];
			iwI = iwJ;
			goto Loop;
			}
		}
	rgw[iwI - 1] = wCopy;
}


#endif /* WIN */


#ifndef PCWORD
/***********************/
/* F i l l  I n  H d t */
#ifdef MAC
EXPORT /* WINIGNORE - MAC only */
#endif /* MAC */
/* %%Function:FillInHdt %%Owner:chrism */
void FillInHdt(plbs, xl, dxl, cp, ihdt, ihdd)
struct LBS *plbs;
int xl, dxl;
CP cp;
int ihdt, ihdd;
{
/* fills in ihdd, dyl, hpllr for header ihdt. First obtains ca of text
	then formats the text.
*/
	int lbc;
	int yl;
	int fTop;
	struct HDT *phdt = &vrghdt[ihdt];
	int ilr;
	uns dylPage;
	LRP lrp;
	struct DOP *pdop;
	struct LBS lbs, lbsDummy;
	struct CA ca;
#ifdef WIN
	int cT;
	extern struct CA caAdjustL;
#endif

	if ((phdt->ihdd = ihdd) == ihddNil)
		goto LNoHdt;
	fTop = FHeaderIhdt(ihdt);

#ifdef WIN
	/* Refresh fields in header/footer */
	if (vlm == lmPrint || vlm == lmPreview || vlm == lmPagevw)
		{
#ifdef SHOWLAYOUT
		CommSzNum (SzShared("Refreshing ihdd "), ihdd);
#endif /* SHOWLAYOUT */
		Assert(vcpFirstLayout != cpNil && vcpLimLayout >= vcpFirstLayout);
		AcquireCaAdjustL();
		CaFromIhddSpec (plbs->doc, ihdd, &caAdjustL);
		if (caAdjustL.doc == PdodDoc (plbs->doc)->docHdr &&
				FSetPcaForCalc (&caAdjustL, caAdjustL.doc,
				caAdjustL.cpFirst, caAdjustL.cpLim, &cT))
			{
			Assert (PdodDoc(caAdjustL.doc)->fHdr);
			if (FCalcFields (&caAdjustL, frmHdrAll|frmPaginate, fFalse, fTrue))
				{
/* docHdr is refleshed, force snap shot copy (i.e. docHdrDisp) to get new hdr text next time it is updated */
				int docHdrDisp;
				if ((docHdrDisp = DocDispFromDocCpIhdt(plbs->doc, cp, ihdt)) != docNil)
					PdodDoc(docHdrDisp)->fGetHdr = fTrue;
				}
			}
		ReleaseCaAdjustL();
		}
#endif

	CaFromIhddSpec(plbs->doc, ihdd, &ca);
	if (ca.doc == docNil)
		{
		Assert(fFalse);
LNoHdt:		
		phdt->dyl = 0;
		FreePhpl(&phdt->hpllr);
		return;
		}
	SetWords(&lbs, 0, cwLBS);
	if (phdt->hpllr != hNil)
		lbs.hpllr = phdt->hpllr;
	else  if ((lbs.hpllr = HpllrInit(sizeof(struct LR), 1)) == hNil)
		{
		Assert(vfInFormatPage);
		AbortLayout();
		}
	SetIMacPllr(&lbs, 0);
	LinkDocToWw(lbs.doc = ca.doc, WinMac(wwLayout, wwTemp), wwNil);
	lbs.ww = WinMac(wwLayout, wwTemp);
	lbs.cp = ca.cpFirst;
	lbs.ihdt = ihdt;
	lbs.lnn = lnnNil;
	lbs.fFirstColumn = lbs.fFirstPara = fTrue;
	lbs.xl = xl;
	lbs.dxlColumn = dxl;
	lbs.fOutline = plbs->fOutline;
	lbs.fPostScript = plbs->fPostScript;
	lbs.fSimpleLr = plbs->fSimpleLr;
	lbs.fRight = plbs->fRight;
	vfls.ca.doc = docNil;   /* blow fls cache */
	pdop = &PdodDoc(plbs->doc)->dop;
	dylPage = YlFromYa(pdop->yaPage);
/* for hard margins, allow full page for headers, otherwise half page */
	if (ihdt >= ihdtMaxSep || (fTop && pdop->dyaTop >= 0) || (!fTop && pdop->dyaBottom >= 0))
		dylPage >>= 1;

	InvalFli();     /* inval for vdocTemp */
	caParaL.doc = caPara.doc = docNil;
	phdt->hpllr = 0;	/* will be changed by FormatBlock */
	caHdt.doc = docNil;	/* in case we get interrupted */
	vplbsFrameHdt = &lbs;
	LbcFormatBlock(&lbs, ca.cpLim, dylPage, fFalse, fFalse, fFalse);
	vplbsFrameHdt = 0;
	caHdt.doc = plbs->doc;
	phdt->hpllr = lbs.hpllr;  /* save LRs in hdt */
	phdt->dyl = lbs.yl;
	vfAbsInHeaders |= lbs.fAbsPresent;
	if (ihdt >= ihdtMaxSep)
		phdt->yl = 0;
	else
		{
		CacheSectL(plbs->doc, cp, plbs->fOutline);
		phdt->yl = FHeaderIhdt(ihdt) ? YlFromYa(vsepFetch.dyaHdrTop) :
				YlFromYa(PdodDoc(plbs->doc)->dop.yaPage - vsepFetch.dyaHdrBottom)
				- lbs.yl;
		}
	/* signal special footnote chars */
	if (ihdd == ihddTFtn)
		ca.doc = -chTFtn;
	else  if (ihdd == ihddTFtnCont)
		ca.doc = -chTFtnCont;
	if (ca.doc < 0 && (ilr = IMacPllr(&lbs)) > 0)
		for (lrp = LrpInPl(phdt->hpllr, 0); ilr-- > 0; lrp++)
			lrp->doc = ca.doc;	/* for special ftn chars */
}


#endif /* !PCWORD */


/*******************************/
/* N e w  S e c t  L a y o u t */
#ifdef MAC
EXPORT /* WINIGNORE - MAC only */
#endif /* MAC */
/* %%Function:NewSectLayout %%Owner:chrism */
void NewSectLayout(plbs, fDoHdrs)
struct LBS *plbs;
int fDoHdrs;
{
/* perform setup for a new section; this should be called as soon as a new
	section is recognized even if the section won't be processed immediately */
	CP cp = CpMin(plbs->cp, CpMacDocEditPlbs(plbs));
	BOOL fOdd;

#ifndef PCWORD
	if (fDoHdrs)
#ifdef MAC
		SetHdts(plbs, cp);
#else
	SetHdts(plbs, cp, NULL, NULL);
#endif
#endif
	CacheSectL(plbs->doc, cp, plbs->fOutline);
	if ((plbs->fPgnRestart = FPgnRestartSep(vsepFetch)) != 0)
		{
		plbs->pgn = IfMacElse(1, max(vsepFetch.pgnStart, 1));
/* make starting page number agrees with type of section break */
		fOdd = plbs->pgn & 1;
		if ((vsepFetch.bkc == bkcEvenPage && fOdd) ||
				(vsepFetch.bkc == bkcOddPage && !fOdd))
			plbs->pgn++;
		}
	if (!FLnnSep(vsepFetch))
		plbs->lnn = lnnNil;
	else  if (vsepFetch.lnc == lncRestart || plbs->lnn == lnnNil)
		plbs->lnn = IfWinElse(vsepFetch.lnnMin + 1, 1);
}


/****************************************/
/* L b c  A d v a n c e  O n e  L i n e */
#ifdef MAC
EXPORT /* WINIGNORE - MAC only */
#endif /* MAC */
/* %%Function:LbcAdvanceOneLine %%Owner:chrism */
int LbcAdvanceOneLine(plbs)
struct LBS *plbs;
{
/* not even one line will fit on the page; we must fit one anyway or else
	be doomed to the ravages of Infinite Recursions, Asserts, Crashes, and
	other Horrors */
/* returns lbcYlLim if LbcFormatPara doesn't mind just one line */
	int cl, lbc, doc, ilr, dxl, clBefore;
	CP cpMinAbs, cpMinNorm, cpBefore;
	LRP lrp;

/* if there are main doc APOs constraining us, stop before them */
	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	dxl = lrp->dxl;
	if (lrp->lrk == lrkAbsHit)
		{
		doc = DocMother(plbs->doc);
		cpMinNorm = (doc == plbs->doc) ? plbs->cp : cpMax;
		lrp = LrpInPl(plbs->hpllr, ilr = IMacPllr(plbs) - 1);
		for (cpMinAbs = cpMax; ilr >= 0; ilr--, lrp--)
			{
			if (lrp->doc != doc)
				break;
			if (lrp->lrk == lrkAbs)
				cpMinAbs = CpMin(cpMinAbs, lrp->cp);
			else  if (lrp->cp != cpNil)
				cpMinNorm = CpMin(cpMinNorm, lrp->cp);
			}
		if (cpMinAbs != cpMax && cpMinAbs > cpMinNorm)
			{
			vcpAbsMic = cpMinAbs;
			if (vhpllrAbs != hNil)
				(*vhpllrAbs)->ilrMac = 0;
			vfRestartLayout = fTrue;
			AbortLayout();
			}
		}

/* figure out how many lines in we are */
	Debug(cpBefore = plbs->cp);
	Debug(clBefore = plbs->cl);
	CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
	cl = plbs->cl;
	if (caParaL.cpFirst != plbs->cp)
		{
		ClFormatLines(plbs, cpMax, ylLarge, ylLarge, clMax, dxl, fFalse, fFalse);
		cl += IInPlc(vfls.hplclnh, plbs->cp);
		}

/* demand one more */
	lbc = LbcFormatPara(plbs, ylLarge, cl + 1);
	Assert(plbs->cp > cpBefore || plbs->cl > clBefore);
	return((lbc == lbcNil) ? lbcYlLim : lbc);
}


#ifdef DEBUGORNOTWIN
/*******************************/
/* F  W i d o w  C o n t r o l */
#ifndef WIN
/* %%Function:FWidowControl %%Owner:NOTUSED */
NATIVE int FWidowControl(plbs, plbsNew, dylFill, fEmptyOK) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_FWidowControl %%Owner:chrism */
HANDNATIVE int C_FWidowControl(plbs, plbsNew, dylFill, fEmptyOK)
#endif /* WIN */
struct LBS *plbs, *plbsNew;
int dylFill, fEmptyOK;
{
/* enforce widow/orphan control; returns fFalse if new paragraph should be
	rejected entirely */
	LRP lrp = LrpInPl(plbsNew->hpllr, plbsNew->ilrCur);
	int doc = plbs->doc;
	int ilnhMac;
	int fOutline = plbs->fOutline;
	CP cp = plbs->cp;
	int cl = plbs->cl;
	CP cpNew = plbsNew->cp;
	int clNew = plbsNew->cl;
	int dxl, dxa;
	struct PHE phe;
	struct LBS lbsT;

	StartProfile();

	Assert(cpNew >= cp);

	Assert(lrp->lrk != lrkAbs);
	if (lrp->lrk != lrkNormal && cl != 0)
		{
		cp = plbs->cp = CpFromCpCl(plbs, fTrue);
		cl = plbs->cl = 0;
		}

	CacheParaL(doc, cpNew, fOutline);
	if (cp < caParaL.cpFirst)
		goto LEndWidow; /* cpNew is the beginning of a new paragraph */

	Assert(FInCa(doc, cp, &caParaL));
	dxa = XaFromXl(dxl = lrp->dxl);

	/* make sure a single line is not stranded at the bottom (orphan)*/
	if (cp == caParaL.cpFirst && cl == 0)
		{
		if (cpNew == cp && clNew == 1)
			goto LAvoidOrphan;

		if (clNew == 0)
			{
			/* we don't have a cl: format the lines */
			ClFormatLines(plbs, cpMax, ylLarge, ylLarge, 2,
					dxl, fFalse, fFalse);
			if (IInPlcCheck(vfls.hplclnh, cpNew) == 1)
				{
LAvoidOrphan:
				StopProfile();
				return (!fEmptyOK);
				}
			}
		}

/* Now check for last line being cut off in lbsNew */
	if (clNew == 0)
		{
LClFormatLines:
		/* we don't have a cl: format the lines */
		ClFormatLines(plbs, cpNew, ylLarge, ylLarge, clMax,
				dxl, fTrue, fFalse);
		clNew = IInPlc(vfls.hplclnh, cpNew);
		ilnhMac = IMacPlc(vfls.hplclnh);
		if (CpPlc(vfls.hplclnh, ilnhMac) >= vfls.ca.cpLim)
			{
			/* check for 4 lines is still necessary in unusual
				circumstances (very tall lines) */
			if (ilnhMac < 4)
				/* no way to avoid widow in 2 or 3 lines */
				goto LAvoidOrphan;
			if (clNew == ilnhMac - 1)
				{
				/* back up a line unless this would violate fEmptyOK */
				if (fEmptyOK || cl > 0 || CpPlc(vfls.hplclnh, clNew - 1) > cp)
					goto LHaveWidow;
				}
			}
		}
	else
		{  /* clNew > 0 */
		CacheParaL(doc, cpNew, fOutline);
		if (!FGetValidPhe(doc, cpNew, &phe) || phe.fDiffLines || 
				phe.dxaCol != dxa || phe.dylHeight == 0)
			{
			/* This is rare: the phe ended up getting zeroed somehow */
			Assert(!phe.fDiffLines && phe.dylHeight == 0);
			cpNew = CpFromCpCl(plbsNew, fTrue);
			clNew = 0;
			goto LClFormatLines;
			}
		Assert(phe.clMac >= clNew);
		if (phe.clMac < 4)
			/* no way to avoid widow in 2 or 3 lines */
			goto LAvoidOrphan;
		if (phe.clMac == clNew + 1)
			{
			/* note that rolling back the cl's is not enough:
				LbcFormatPara has other side effects such as
				advancing the yl, lnn, etc. */
			/* stranded line */
LHaveWidow:
			PushLbs(plbs, &lbsT);
			CopyLbs(&lbsT, plbsNew);
			FAssignLr(plbsNew, dylFill, fFalse);
			LbcFormatPara(plbsNew, dylFill, clNew - 1);
			}
		}
LEndWidow:
	StopProfile();
	return(fTrue);
}


#endif /* DEBUGORNOTWIN */


/*************************/
/* F  E m p t y  P a g e */
#ifdef MAC
EXPORT /* WINIGNORE - MAC only */
#endif /* MAC */
/* %%Function:FEmptyPage %%Owner:chrism */
int FEmptyPage(bkc, ppgn, fFacingPages, fRight)
int bkc, *ppgn, fFacingPages, fRight;
{
/* this little beauty returns whether the specified configuration would force
	an empty page; if break code and pgn don't match, or if facing pages and
	odd pgn. this should be called only for the pgn at beginning of a section
*/
	int fOdd = *ppgn & 1;

	if ((fOdd && bkc == bkcEvenPage) || (!fOdd && bkc == bkcOddPage))
		{
		(*ppgn)++;	/* advance past empty page */
		return(fTrue);
		}
	return(fFacingPages && (fOdd ^ fRight));
}


#ifndef PCWORD
/*********************/
/* F   N i l   H d t */
/* %%Function:FNilHdt %%Owner:chrism */
FNilHdt(ihdt)
int ihdt;
{
/* return true if the specified header is nil */
	struct HDT *phdt = &vrghdt[ihdt];

	return(phdt->hpllr == hNil || (*phdt->hpllr)->ilrMac == 0);
}


#endif /* !PCWORD */



#ifndef WIN 
#ifndef JR
/*********************************/
/* L b c  S i d e  B y   S i d e */
/* %%Function:LbcSideBySide %%Owner:NOTUSED */
EXPORT int LbcSideBySide(plbs, cpLim, dylFill, fEmptyOK, pfKeepFollow) /* WINIGNORE - MAC only */
struct LBS *plbs;
CP cpLim;
int dylFill, fEmptyOK, *pfKeepFollow;
{
/* handle side-by side paragraphs; caPara describes current para */
	struct DOD *pdod = PdodDoc(plbs->doc);  /* 2 lines for compiler */
	int fShort = IfPCWordElse(plbs->cp > CpMacText(plbs->doc), pdod->fShort);
	int ylTopTable, ilrFirst, ilrFirstSxs;
	int xaLeftCur, xaLeftCol;
	int lbc = lbcNil, ylRowMac;
	int fTouchedLim = fFalse;
	int fAnyTouchedLim = fFalse;
	CP cpFirst = plbs->cp;
	struct PLC **hplcfnd;
	LRP lrp;
	struct LBS lbsT;
	struct LR lr;

	lrp = LrpInPl(plbs->hpllr, ilrFirst = plbs->ilrCur);
	ylTopTable = (lrp->cp == cpNil) ? lrp->yl : plbs->yl;  /* start of row in lbs */

/* save left of first para in row */
#ifdef PCWORD
	if (plbs->cp > CpMacText(plbs->doc))
#else
		if (pdod->fFtn)
#endif /* !PCWORD */
			{
		/* limit to one footnote */
			hplcfnd = pdod->hplcfnd;
			cpLim = CpMin(cpLim, CpPlc(hplcfnd, 1 + IInPlc(hplcfnd, plbs->cp)));
			}
	CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
	xaLeftCol = vpapFetch.dxaLeft + min(0, vpapFetch.dxaLeft1);
	NewLrForSxs(plbs, ylRowMac = ylTopTable);
	ilrFirstSxs = plbs->ilrCur;

	while (plbs->cp < cpLim)
		{
		/* next para */
		CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
		plbs->dcpDepend = (int) CpMin(0x7fffL, caParaL.cpLim - caParaL.cpFirst);
		xaLeftCur = vpapFetch.dxaLeft + min(0, vpapFetch.dxaLeft1);
		if (!vpapFetch.fSideBySide || xaLeftCur < xaLeftCol ||
				FAbsPapM(plbs->doc, &vpapFetch))
			{
			/* first para of new row */
			break;
			}
		if (FPageBreakBeforePap(vpapFetch) && !plbs->fFirstPara && !fShort)
			{
			/* first para of new row, next page */
			lbc = lbcPageBreakBefore;
			*pfKeepFollow = fFalse;
			break;
			}
		*pfKeepFollow = vpapFetch.fKeepFollow && !fShort;
		if (xaLeftCur > xaLeftCol)
			{
			/* new column; duplicate lr and continue */
			fTouchedLim = fFalse;
			ylRowMac = max(plbs->yl, ylRowMac);
			xaLeftCol = xaLeftCur;
			NewLrForSxs(plbs, ylTopTable);
			/* continue to next para in table */
			}
		/* continue in this column */
		if (fTouchedLim)
			{
			/* last para in column exactly fit */
			if (fEmptyOK)
				goto LRejectTable;
			break;
			}
		PushLbs(plbs, &lbsT);
		lbc = LbcFormatPara(&lbsT, dylFill, clMax);
		if (lbc != lbcNil)
			{
			if (lbc == lbcYlLim)
				{
				CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
				if (lbsT.cp == caParaL.cpFirst && lbsT.cp > plbs->cp)
					{
					/* para exactly fit */
					fTouchedLim = fAnyTouchedLim = fTrue;
					goto LAcceptPara;
					}
				if (fEmptyOK)
					{
LRejectTable:
					/* don't use table because
						entire para must fit */
					CopyLbs(0, &lbsT);
					plbs->dcpDepend = (int) CpMin(0x7fffL, caParaL.cpLim - cpFirst);
					plbs->dylOverlap += max(0, lbsT.yl - plbs->yl);
					plbs->ilrCur = ilrFirst; /* reset for new lr test */
					return(lbcYlLim);
					}
				/* a column cannot fit
					(taller than page).
					break in mid-para
					if possible */
				if (plbs->yl == lbsT.yl)
					/* no progress, must fit one line */
					LbcAdvanceOneLine(&lbsT);
				}
			CopyLbs(&lbsT, plbs);
			break;
			}
LAcceptPara:
		CopyLbs(&lbsT, plbs);
#ifndef WIN
		if (plbs != vplbsFrame && plbs != vplbsFrameHdt)
			vplbsFrameSxs = plbs;
#endif
		/* continue to next para in table */
		}

/* adjust dyl's */
	plbs->yl = max(plbs->yl, ylRowMac);
	lrp = LrpInPl(plbs->hpllr, ilrFirstSxs);
	for (; ilrFirstSxs <= plbs->ilrCur; ilrFirstSxs++, lrp++)
		lrp->dyl = plbs->yl - lrp->yl;
	plbs->dylOverlap = 0;
	return((fAnyTouchedLim) ? lbcYlLim : lbc);
}


/*******************************/
/* N e w   L r   F o r   S x s */
/* %%Function:NewLrForSxs %%Owner:NOTUSED */
NewLrForSxs(plbs, ylTop)
struct LBS *plbs;
int ylTop;
{
/* clone and freeze the current LR to allow new column or row for side-by-side */
	LRP lrp;
	struct LR lr;

	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	if (lrp->lrk == lrkNormal)
		lrp->lrk = lrkSxs;	/* mark for page view */
	lrp->lrs = lrsFrozen;
	bltLrp(lrp, &lr, sizeof(struct LR));
	lr.dyl -= ylTop - lr.yl;
	plbs->yl = lr.yl = ylTop;
	lr.cp = lrp->cpLim = plbs->cp;
	lr.clFirst = lrp->clLim = plbs->cl;
	lr.lnn = lnnNil;
	if (lrp->cp != cpNil)
		{
		/* can't take over the existing lr */
		plbs->ilrCur = IMacPllr(plbs);
		lrp->dyl = ylTop - lrp->yl;
		}
	ReplaceInPllr(plbs->hpllr, plbs->ilrCur, &lr);
}


#endif /* JR */
#endif /* !WIN */
