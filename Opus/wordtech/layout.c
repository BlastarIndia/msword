/* layout.c */
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

#ifdef WIN
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#endif /* WIN */

#include "ch.h"
#include "props.h"
#include "doc.h"
#include "disp.h"

#ifdef PCWORD
#include "chfile.h"
#include "font.h"
#include "layoutpc.h"
#include "strmsg.h"
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
#include "border.h"
#include "debug.h"
#endif /*!PCWORD*/

#ifdef MAC 
#include "qsetjmp.h"
#endif /*MAC*/

#ifdef WIN
#include "heap.h"
#include "field.h"
#endif /* WIN */

#ifdef PROTOTYPE
#include "layout.cpt"
#endif /* PROTOTYPE */

#ifdef PCWORD
#define	vpapFetch       vpapAbs
#define vsepFetch       vsepAbs
AssertDataCc("layout.c")
#endif /* !PCWORD */

/* E X T E R N A L S */

extern uns              *pfgrMac;
extern int              vdocTemp;
extern int              vdocFetch;
extern int		vfAbsInHeaders;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA        caTap;
extern struct CA        caHdt;
extern struct PLC       **vhplcfrl;
extern struct FLS       vfls;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct TCC       vtcc;
extern struct FLI       vfli;
extern struct FMTSS     vfmtss;
extern struct MERR      vmerr;
extern struct PREF      vpref;
extern int              vfBkgrndRepag;
extern struct PLLR      **vhpllrAbs;
extern int              vfRestartLayout;
extern int              vylTopPage;
extern int              vlm;
extern ENV              venvLayout;
extern CP               vcpAbsMic;

extern struct DBS       vdbs;
extern CP               vcpFirstLayout;
extern struct DOD       **mpdochdod[];
extern struct HDT       vrghdt[];
extern struct PLLBS     **vhpllbs;

extern int              vfInFormatPage;
extern struct PLLR      **vhpllrSpare;
#ifndef WIN
extern int              vcparaAbort;
#endif /* !WIN */
extern struct LBS       *vplbsFrame;
extern struct LBS       *vplbsFrameHdt;
extern struct CA        caParaL;
extern int		vcLayout;
extern int		vcBalance;

#ifndef WIN
extern struct LBS       *vplbsFrameSxs;
#endif

#ifdef PCWORD
extern struct PAP       vpapStd;
extern struct PRSU      vprsu;
extern struct PMD       vpmd;
extern struct RCPAGE    vrcPage;
#endif /* PCWORD */

#ifdef WIN
extern CP               vcpLimLayout;
extern struct FTI       vfti;
#endif /* WIN */

#ifndef PCWORD
extern struct DRF	*vpdrfHead;
#ifdef DEBUG
extern struct DRF	*vpdrfHeadUnused;
#endif /* DEBUG */
#endif /* !PCWORD */

/* check for interrupt every vcparaAbortMax paras */
#ifndef WIN
#define vcparaAbortMax	3
#endif /* !WIN */

#ifndef PCWORD
struct PL *HplInit();
#endif

/* NOTE: DO NOT USE CACHEPARA AND CACHESECT WITHIN THIS MODULE; USE
		CACHEPARAL AND CACHESECTL INSTEAD */

/*
Layout rules:
	Keeps can not force a whole page length column to be empty (i.e.
	when there is no chance that the next column will be any longer.)

	Table paras have implicit keeps.

	Keep paras must appear on the same page.

	The last line of a keep follow para must be kept with the first line
	of the following para. This is in addition of the other rules.
	This means that a block keep can be implemented by keep on all
	paras and keep follow on all paras except the last one.
	New page commands (character, division property, para property)
	override any keep follow that contradicts them.

	Tables/keeps/keep follow also work in footnotes.

	If widow control is on, the first or last lines of multiple-line
	paragraphs will never appear on a page by themselves.

	Footnote references and the referenced footnotes will almost always
	appear on the same page. Exception: with multiple references in a line
	where the first few references fill up the page, the remaining
	references will have to be continued.

	A footnote header or footnote continue header will appear before
	the first footnote on a page. The footnote continue header will
	follow the last footnote if it is continued to the next page.

	It will be possible for the footnote continuations to occupy a full
	page. This will be quite normal when the footnotes are all collected
	to the end of a division or the end of the document.
*/


/*******************************/
/* L b c  F o r m a t  P a g e */
#ifndef WIN
NATIVE /* WINIGNORE - LbcFormatPage */
#endif /* !WIN */
/* %%Function:LbcFormatPage %%Owner:chrism */
int LbcFormatPage(plbsText, plbsFtn)
struct LBS *plbsText;  /* start laying out text at this point, cpNil illegal*/
struct LBS *plbsFtn;   /* footnote stream, fContinue means ftn pending */
{
/*	Results: lbs* will be modified for the next page.
	list of lr's detailing the layout is collected in pllr of lbsText.
*/
	int ccol, ccolMac;
	int xlLeft, xlRight, xlCol, dxlCol, dxaBetween;
	int ylBottom, dylRemain, dylMac;
	int ddyl, ddylRelax, dylFill;
	int lbc;
	int ilr, ilrMac;
	int doc = plbsText->doc;
#ifndef PCWORD
	struct DOP *pdop;
	struct DOD *pdod = PdodDoc(doc);
	int fLRHeaders = FFacingPagesPdod(pdod);
#endif /* !PCWORD */
	int bkc, bkcLast;
	int fOdd = plbsText->pgn & 1; /* odd page number */
	int fTopOfSection, fTitlePage, fCopyHdr;
	CP cp, cpFirst, cpMac, cpT;
	long dylAvail;
	LRP lrp;
#ifdef WIN
	int yl, fRedoPage, ilnbc, ilnbcSect, ilnbcCol;
	int fAlign = fFalse, cHdrFtrLayoutAttempts = 1;
	struct ALG **hgrpalg = hNil;
	int balgCur, balgMac, balgMax, ialgMac, clrAdjust;
	struct ALG *palg;
	struct ALC *palc;
	struct LNBC lnbc, *plnbc;
	struct PLLNBC *ppllnbc;
#endif /* WIN */
	struct LBS lbsTextStart, lbsFtnStart;
	struct LBS lbsTextInit, lbsFtnInit;
	int pgnT;

	StartProfile();
	Assert(vlm != lmNil);
	vfInFormatPage = fTrue;
#ifdef MAC
	DxsSetDxsInch(doc);
#endif /* MAC */

	Scribble(ispLayout1, 'F');
	vhpllrSpare = vhpllrAbs = hNil;
#ifndef WIN
	vcparaAbort = vcparaAbortMax;	/* try on 1st para */
#endif /* !WIN */
	vcLayout = 0;	/* first try */
	cpMac = vcpAbsMic = CpMacDocPlbs(plbsText);
	cpFirst = plbsText->cp;
	if (FEmerg1(vmerr) ||
			(vhpllbs = HplInit(sizeof(struct LBS), 8)) == hNil)
		{
		lbc = lbcAbort;
		goto LInterrupted;
		}
	plbsText->dcpDepend = 0;
#ifndef PCWORD
	Assert(vpdrfHead == NULL);
#endif /* !PCWORD */
	if (SetLayoutAbort() != 0)
		{
		if (vfRestartLayout || IfWinElse(0, plbsText->fSimpleLr))
			{
LMustRestart:
			PopLbs(&lbsFtnInit, plbsFtn);
			PopLbs(&lbsTextInit, plbsText);
			/* for complex pages, better to have page at all than PS or preview */
			if (!vfRestartLayout)
				{
				plbsText->fSimpleLr = plbsFtn->fSimpleLr = fFalse;
#ifndef PCWORD
				vmerr.mat = matNil;	/* reset error */
#endif
				}
			goto LRestartPage;
			}
		lbc = lbcAbort;
		goto LInterrupted;
		}

#ifndef WIN
	vfli.fStopAtPara = fTrue;	/* never exceed para bounds */
#endif /* !WIN */
	vdocFetch = caParaL.doc = caPara.doc = caSect.doc = docNil;
	InvalFli();
	SetIMacPllr(plbsText, 0);
	SetIMacPllr(plbsFtn, 0);
	plbsText->fOnLbsStack = plbsFtn->fOnLbsStack = fFalse;

#ifndef WIN
	/* note vcpFirstLayout is reset below */
	vcpFirstLayout = cp = CpMin(cpFirst, cpMac);
#else /* WIN */
	cp = CpMin(vcpFirstLayout = plbsText->cp, cpMac);
	vcpLimLayout = vcpFirstLayout; /* prevent crashing if field needs vcpLimLayout before it is set up correctly */
#endif /* WIN */

	CacheSectL(doc, cp, plbsText->fOutline);
	if (plbsFtn->fContinue && plbsText->cp == caSect.cpFirst && plbsText->cl == 0)
		{
		/* use prev section props for continued footnotes and endnotes */
		Assert(cp > cp0);
		vcpFirstLayout = cpFirst = --cp;
		}

#ifdef WIN
	if (FNeRgw(&caHdt, &caSect, cwCA))
		caHdt = caSect;

	if (vlm == lmPrint || vlm == lmPreview)
		{
		fAlign = fTrue;
		if ((hgrpalg = HAllocateCw(cwALG)) == hNil)
			{
			lbc = lbcAbort;
			goto LInterrupted;
			}
		}
#endif /* WIN */

LRestartPage:
	vplbsFrame = vplbsFrameHdt = 0;
#ifndef WIN
	vplbsFrameSxs = 0;
#endif
	vfRestartLayout = fFalse;
	plbsText->fAbsPresent = plbsFtn->fAbsPresent = fFalse;
	plbsText->ilrCur = plbsFtn->ilrCur = ilrNil;
	PushLbs(plbsText, &lbsTextInit);
	PushLbs(plbsFtn, &lbsFtnInit);

	CacheSectL(doc, cp, plbsText->fOutline);
	plbsText->bkc = vsepFetch.bkc;
	plbsText->fFirstColumn = fTrue;
#ifdef WIN
	/* forget everything in the vertical line plex and the grpalg */
	Assert(plbsText->hpllnbc != hNil);
	(*plbsText->hpllnbc)->ilnbcMac = 0;
	balgMax = cwALG;
	ialgMac = balgMac = balgCur = 0;
#endif /* WIN */
	if (FEmerg1(vmerr))
		{
		lbc = lbcAbort;
		goto LInterrupted;
		}
	fTopOfSection = caSect.cpFirst == cp && plbsText->cl == 0;
	fTitlePage = fTopOfSection && FTitlePageSep(vsepFetch);

	pgnT = vfmtss.pgn + 1; /* pg # for blank pg in case section has restart pg # */
/* save the real page number for FormatChSpec and page preview */
	vfmtss.pgn = plbsText->pgn;

/* possible empty page: if break code and pgn don't match, or if facing pages
	and page number don't match */
	plbsText->fCpOnPage = fFalse;
	plbsText->fEmptyPage = fFalse;
	if (!fTopOfSection)
		plbsText->fPgnRestart = fFalse; /* continue numbering */
	else  if (FEmptyPage(vsepFetch.bkc, &plbsText->pgn, plbsText->fFacingPages, plbsText->fRight))
		{
		vfmtss.pgn = pgnT;
		plbsText->fEmptyPage = fTrue;
		lbc = lbcNil;
		goto LPageDone;
		}

/* determine page boundaries */
#ifdef PCWORD
	vylTopPage = max(YlFromYa(vsepFetch.yaTop), vrcPage.ypTop);
	ylBottom = min(YlFromYa(vsepFetch.yaTop + vsepFetch.dyaText),
			vrcPage.ypBottom);
	plbsText->fFacingPages = vsepFetch.fMirrorMargins;
	GetXlMargins(fOdd, fFalse, &xlLeft, &xlRight);
#else
	pdop = &PdodDoc(doc)->dop;
#ifdef WIN
	if (cHdrFtrLayoutAttempts == 1)
#endif
		{
		vylTopPage = YlFromYa(abs(pdop->dyaTop));
		ylBottom = YlFromYa(pdop->yaPage - abs(pdop->dyaBottom));
		}
	GetXlMargins(pdop, fOdd, WinMac(vfli.dxuInch, vfli.dxsInch),
			&xlLeft, &xlRight);
#endif /* !PCWORD */

/* find headers, auto pgn */
#ifndef WIN
	plbsText->xl = xlLeft;
	plbsText->dxlColumn = xlRight - xlLeft;
	AddHdrFtrPgn(plbsText, cp, fTitlePage, IfMacElse(fLRHeaders && !fOdd, !fOdd),
			&vylTopPage, &ylBottom, xlLeft);
#else /* WIN */
/* 
set up the footnote separator first so that its lr gets copied in
LbcFormatColumn, (punt the case if it has field that needs the last cp
of the page).  Header and footer are set up at end of the page because
of fields.
*/
	SetHdts(plbsText, cp, NULL, NULL);
#endif 
	/* SetHdts changes vsepFetch/caSect...reset them */
	CacheSectL(doc, cp, plbsText->fOutline);

/* copy abs objects; MAC wants headers BEFORE apos */
	if (vhpllrAbs != hNil && (ilrMac = (*vhpllrAbs)->ilrMac) != 0)
		{
		CopyLrs(vhpllrAbs, plbsText, ilrMac, ylLarge, fTrue, -1);
		(*vhpllrAbs)->ilrMac = 0;
		}

/* reset lnn if numbering starts over on each page */
	if (FLnnSep(vsepFetch) && vsepFetch.lnc == lncPerPage)
		plbsText->lnn = IfWinElse(vsepFetch.lnnMin + 1, 1);
	plbsFtn->lnn = lnnNil;
	if (FEmerg1(vmerr))
		{
		lbc = lbcAbort;
		goto LInterrupted;
		}

/* fill in the page */
	bkcLast = bkcNoBreak;
	dylRemain = ylBottom - vylTopPage;
	cpMac = CpMin(cpMac, vcpAbsMic);
	for (plbsText->ylColumn = vylTopPage; ; plbsText->ylColumn = lbsTextStart.ylColumn + dylMac)
		{
		/* next section on page */
		plbsText->fNoBalance = fFalse;
		ccolMac = CColSep(vsepFetch);
		dxaBetween = vsepFetch.dxaColumns;
		bkc = vsepFetch.bkc;
		/* width of column and separator */
		/* XlFromXa(a + b) to match pageview calculation! */
#ifdef PCWORD
		/* unprintable regions may effect column width */
		plbsText->dxlColumn = XlFromXa(vrcPage.dxaColPr);
		dxlCol = plbsText->dxlColumn + XlFromXa(vsepFetch.dxaColumns);
#else
		plbsText->dxlColumn = XlFromXa(vsepFetch.dxaColumnWidth);
		dxlCol = XlFromXa(vsepFetch.dxaColumnWidth + vsepFetch.dxaColumns);
#endif /* !PCWORD */
		PushLbs(plbsText, &lbsTextStart);     /* save initial state */
		PushLbs(plbsFtn, &lbsFtnStart);

#ifdef WIN
		Assert(plbsText->hpllnbc != hNil);
		ilnbcSect = (*plbsText->hpllnbc)->ilnbcMac;

		if (fAlign)
			{
			/* initialize vertical alignment structure */
			balgCur = balgMac;
			if (!FAllocAlg(hgrpalg, CColM1Sep(vsepFetch), &balgMac, &balgMax))
				{
				lbc = lbcAbort;
				goto LInterrupted;
				}
			ialgMac++;
			palg = PalgFromHgrpalgBalg(hgrpalg, balgCur);
			palg->ylTop = plbsText->ylColumn;
			palg->dylMac = 0;
			palg->dylAvail = dylRemain;
			palg->calc = 0;
			palg->vjc = vsepFetch.vjc;
			}
#endif /* WIN */
		for (dylFill = dylRemain, vcBalance = 0; ; vcBalance++)
			{
			/* next balancing iteration */
			plbsText->dylUsedCol = plbsFtn->dylUsedCol = 0; /* amount of linear space used */
			plbsText->clTotal = plbsFtn->clTotal = 0;
			plbsText->dyllTotal = plbsFtn->dyllTotal = 0;
			dylMac = 0;
#ifdef WIN
			ilnbcCol = ilnbcSect;
#endif /* WIN */
			for (ccol = 0; ccol < ccolMac; ccol++)
				{
				/* next column */
				ilr = IMacPllr(plbsText);    /* old ilrMac */
				plbsText->xl = plbsFtn->xl = xlCol = xlLeft + ccol * dxlCol;
				lbc = LbcFormatColumn(plbsText, plbsFtn, cpMac, dylFill);
				dylMac = max(dylMac, plbsText->yl - lbsTextStart.ylColumn);

#ifdef WIN
				if (fAlign)
					{
					/* Update vertical alignment structures */
					palg = PalgFromHgrpalgBalg(hgrpalg, balgCur);
					palg->vjc = vsepFetch.vjc;
					palg->calc += 1;
					palg->dylMac = max(palg->dylMac, plbsText->yl - plbsText->ylColumn);
					/* ccol == ialc */
					palc = &(palg->rgalc[ccol]);
					palc->ilrFirst = ilr;
					palc->clr = IMacPllr(plbsText) - ilr;
					palc->lbc = lbc;

					/* check for line between columns */
					if (vsepFetch.fLBetween && ccol > 0)
						{
						Assert(plbsText->hpllnbc != hNil);
						ppllnbc = *plbsText->hpllnbc;
						lnbc.xl = xlCol - (XlFromXa(vsepFetch.dxaColumns) >> 1);
						lnbc.yl = plbsText->ylColumn;
						lnbc.dyl = 0;
						if (ilnbcCol == ppllnbc->ilnbcMac)
							{
							if (!FInsertInPl(plbsText->hpllnbc, ilnbcCol++, (char *)&lnbc))
								AbortLayout();
							}
						else
							{
							Assert(ilnbcCol >= 0 && ilnbcCol < ppllnbc->ilnbcMac);
							plnbc = PInPl(plbsText->hpllnbc, ilnbcCol++);
							bltbyte((char *)&lnbc, (char *)plnbc, ppllnbc->cb);
							}
						}
					}
#endif /* WIN */

				if (lbc == lbcPageBreakBefore ||
						lbc == lbcEndOfDoc || lbc == lbcEndOfPage)
					{
#ifdef WIN
					if (fAlign)
						{
						Assert(plbsText->hpllnbc != hNil);
						SetSectLnbcHeight(plbsText->hpllnbc, dylMac, ilnbcSect, ilnbcCol);
						}
#endif /* WIN */
					goto LPageDone;
					}

				if (lbc == lbcEndOfSection)
					{
					if (plbsFtn->fContinue)
						{
						plbsText->fNoBalance = fTrue;
						break;
						}
					NewSectLayout(plbsText, fTrue);
					/* no balancing when new column section was used */
					if (bkcLast != bkcNoBreak && vsepFetch.bkc == bkcNoBreak)
						plbsText->fNoBalance = fTrue;
					bkc = bkcLast = vsepFetch.bkc;
					if (bkc != bkcNewColumn)
						break;
					/* this page is done if new column and: only one column
						or no more columns left or change in # columns */
					if (ccolMac == 1 || ccol + 1 == ccolMac || 
							CColSep(vsepFetch) != ccolMac || 
							vsepFetch.dxaColumns != dxaBetween)
						{
#ifdef WIN
						if (fAlign)
							{
							Assert(plbsText->hpllnbc != hNil);
							SetSectLnbcHeight(plbsText->hpllnbc, dylMac, ilnbcSect, ilnbcCol);
							}
#endif /* WIN */
						goto LPageDone;
						}
					}
				}

			if (ccolMac == 1 || bkc != bkcNoBreak || plbsText->fNoBalance)
				goto LNoBalance;	/* no balancing needed */
			/* Balancing algorithm:
				first: reduce dylFill by theoretical amount
				then:  if not too tight, we are done, else continue to relax
					by fixed increment but not above the original dyl.
			*/
			if (vcBalance == 0 && lbc == lbcYlLim ||
					vcBalance > 0 && lbc != lbcYlLim)
				{
				/* balanced */
LNoBalance:
#ifdef PCWORD /* PCWORD can change margins within a page */
				GetXlMargins(fOdd, fFalse, &xlLeft, &xlRight);
#endif /* PCWORD */
				PopLbs(&lbsFtnStart, 0);
				PopLbs(&lbsTextStart, 0);
				break;
				}
			Scribble(ispLayout2, '0' + vcBalance);
			ddylRelax = (plbsText->clTotal == 0) ? 0 : plbsText->dyllTotal / plbsText->clTotal;
			if (ddylRelax == 0)
				ddylRelax = ddylRelaxDef;
			if (vcBalance > 0)
				ddyl = min(ddylRelax, dylRemain - dylFill);
			else
				{
				/* balance columns */
				dylAvail = (long) ccolMac * dylFill;  /* linear stuff available */
				if (dylAvail - plbsText->dylUsedCol <= ddylRelax)
					break;	/* balanced */
				ddyl = (plbsText->dylUsedCol - dylAvail) / ccolMac;
				}
			dylFill = max(ddylRelax, dylFill + ddyl);
			PopLbs(&lbsFtnStart, plbsFtn);
			PopLbs(&lbsTextStart, plbsText);
			PushLbs(plbsText, &lbsTextStart); /* save initial state again */
			PushLbs(plbsFtn, &lbsFtnStart);
#ifdef WIN
			/* Reset vertical alignment structures */
			if (fAlign)
				{
				palg = PalgFromHgrpalgBalg(hgrpalg, balgCur);
				palg->dylMac = 0;
				palg->calc = 0;
				palg->dylAvail = dylFill;
				}
#endif /* WIN */
#ifdef MAC
			SetHdts(plbsText, cp);
#endif
#ifdef WIN
			SetHdts(plbsText, cp, NULL, NULL);
#endif
			}
#ifdef WIN
		/* set height of vertical line between columns */
		/* ilnbcSect starts the section, ilnbcCol is the Mac */
		if (fAlign)
			{
			Assert(plbsText->hpllnbc != hNil);
			SetSectLnbcHeight(plbsText->hpllnbc, dylMac, ilnbcSect, ilnbcCol);
			}
#endif /* WIN */

		if (lbc != lbcEndOfSection)
			break;
		/* starting new section */
		NewSectLayout(plbsText, fTrue);
#ifdef PCWORD
		/* PCWORD can change margins within a page */
		GetXlMargins(fOdd, fFalse, &xlLeft, &xlRight);
#endif /* PCWORD */
		if (vsepFetch.bkc != bkcNoBreak)
			break;
		dylRemain -= dylMac;
		}

LPageDone:

/* a rare case: check for absolute objects near end of page that perturbed
	this page but didn't end up on it -- we stop layout before that object */
	if (plbsText->fAbsPresent)
		{
		cpT = -1;	/* cp0 might match plbsText->cp */
		ilrMac = IMacPllr(plbsText);
		for (lrp = LrpInPl(plbsText->hpllr, 0); ilrMac-- > 0; lrp++)
			if (lrp->lrk == lrkAbs && lrp->doc == doc && lrp->cp > cpT)
				cpT = lrp->cp;
		if (cpT >= plbsText->cp)
			{
			cpMac = cpT;
			fCopyHdr = fFalse;
LCopyAndRestart:
			Assert(vhpllrAbs != hNil);
			(*vhpllrAbs)->ilrMac = 0;
			CopyAbsLrs(plbsText->hpllr, vhpllrAbs, fCopyHdr, fFalse);
			vfRestartLayout = fTrue;
			goto LMustRestart;
			}
		}

#ifdef WIN
	/* For the last time: I think we've demonstrated that the cl optimization
	   is worthless. Too many of the clients of LbcFormatPage need a raw CP.
	   Cl's were only really useful as long as people did foreground
	   repaginations; doing single pages for page view can't use cl's,
	   because CachePage has to resolve them; background repag and subdocs 
	   can't use them; and printing and preview are so slow no one will
	   ever notice. For now we can keep them within the page, but we're
	   not going to export them, and in Cortex we should eliminate them
	   altogether. FUTURE. chrism */
	if (plbsText->cl != 0)
		{
		plbsText->cp = CpFromCpCl(plbsText, fTrue);
		plbsText->cl = 0;
		}
	vcpLimLayout = plbsText->cp;
	plbsText->xl = xlLeft;
	plbsText->dxlColumn = xlRight - xlLeft;
	CacheSectL(doc, vcpFirstLayout, plbsText->fOutline);
	clrAdjust = 0;
	if (!FAddHdrFtr(plbsText, cp, cHdrFtrLayoutAttempts++, fTitlePage, fLRHeaders && !fOdd, &vylTopPage, &ylBottom, xlRight,
			 xlLeft, &clrAdjust))
		{
		if (!vfAbsInHeaders)
			goto LMustRestart;
		if (vhpllrAbs == hNil &&
				(vhpllrAbs = HpllrInit(sizeof(struct LR), 2)) == hNil)
			{
			lbc = lbcAbort;
			goto LInterrupted;
			}
		fCopyHdr = fTrue;
		goto LCopyAndRestart;
		}

	/* deal with vertical alignment, clipping lnbc */
	if (fAlign)
		{
		AlignLrs(plbsText, hgrpalg, ialgMac, clrAdjust);
		ClipLnbcToApo(plbsText);
		}

#endif /* WIN */

	/* don't bump page number if none of section was used */
	if (!plbsText->fEmptyPage)
		{
		plbsText->pgn++;
		CacheSectL(doc, plbsText->cp, plbsText->fOutline);
		if (plbsText->cp == caSect.cpFirst && plbsText->cl == 0 &&
				!plbsFtn->fContinue)
			{
			NewSectLayout(plbsText, fTrue);
			}
		}
	plbsText->fRight ^= fTrue;

LInterrupted:
#ifdef WIN
	if (fAlign)
		FreeH(hgrpalg);
#else
	vfli.fStopAtPara = fFalse;
	vcpFirstLayout = cpFirst;
#endif /* !WIN */

#ifdef PCWORD
	vfli.cpFirstLr = cp0;
	if (lbc == lbcYlLim && plbsText->cl == 0 &&
			FEitherOr(ChFromDocCp(doc, plbsText->cp, vprsu.fSeeHidden),
			chSect, chColumnBreak))
		{
		plbsText->cp++;
		lbc = lbcEndOfPage;
		}
#endif /* PCWORD */

/* remove LRs that are half-formed */
	PopLbs(0, 0);
	FreePhpl(&vhpllbs);
	if (plbsText->fAbsPresent)
		FreePhpl(&vhpllrAbs);
	if (vhpllrSpare != hNil)
		FreePhpl(&vhpllrSpare);
	ilrMac = IMacPllr(plbsText);
	if (ilrMac == 1)
		{
		lrp = LrpInPl(plbsText->hpllr, 0);
		if (lrp->dyl == 0)
			lrp->dyl = ddylRelaxDef;
		}
	else
		for (ilr = 0; ilr < ilrMac; )
			{
			lrp = LrpInPl(plbsText->hpllr, ilr);
			if (lrp->lrs == lrsIgnore || lrp->cp == cpNil || lrp->dyl == 0)
				{
				DeleteFromPl(plbsText->hpllr, ilr);
				ilrMac--;
				}
			else
				ilr++;
			}

	vdocFetch = caParaL.doc = caPara.doc = caSect.doc = vtcc.doc = docNil;
	InvalFli();
	Scribble(ispLayout1, ' ');
	Scribble(ispLayout2, ' ');
	Scribble(ispLayout3, ' ');
	Scribble(ispLayout4, ' ');
	plbsText->lbc = lbc;
	vfInFormatPage = fFalse;

	StopProfile();
	return(lbc);
}


/***********************************/
/* L b c  F o r m a t  C o l u m n */
#ifndef WIN
NATIVE /* WINIGNORE - LbcFormatColumn */
#endif /* !WIN */
/* %%Function:LbcFormatColumn %%Owner:chrism */
int LbcFormatColumn(plbsText, plbsFtn, cpLim, dylFill)
struct LBS *plbsText;	/* start of column, also lim returned here */
struct LBS *plbsFtn;	/* footnote overflow text. remaining overflow returned.
doc != docHdr */
CP cpLim;
int dylFill;            /* height to be filled */
{
/* This procedure treats footnotes; it collects a block of main text and the
	corresponding block of footnotes and fits them into the column */
/* WIN no longer allows APOs to perturb footnotes. Cortex will do this also */
	int dylRemain = dylFill;           /* remaining height */
	int ylCleave, dyl, ylT, ylCol = plbsText->ylColumn, dylFtn = 0;
	int dylContin = 0;
	int ifrl, ifrlMac;
	int lbc = lbcNil, lbcFtn, ilr, fpc;
	int fEmptyOK = !plbsText->fFirstColumn;
	int fEmptyOKFtn, fNoText;
	int fEndnotes, fRetryFtn, fFtnOnPage;
#ifndef PCWORD
	struct DOP *pdop;
#endif
	struct PLC **hplcfnd;
	struct DOD *pdod;
	CP cpLimFtn, cpRef, cpMacFtn, cpMacDepend, cpFtn;
	struct FRL *pfrl;
	struct LBS lbsTextStart, lbsFtnStart, lbsSave, lbsAbsSave;
	struct FRL frl;

/* set up lbs's for column */
#ifdef PCWORD
	fpc = vsepFetch.fEndFtns ? fpcEndnote : fpcBottomPage;
#else
	pdod = PdodDoc(plbsText->doc);  /* two lines for compiler */
	fpc = pdod->dop.fpc;
#endif /* !PCWORD */
	StartProfile();
	plbsText->ihdt = plbsFtn->ihdt = ihdtNil;
	plbsText->fFirstPara = plbsFtn->fFirstPara = fTrue; /* ignore page breaks */
	SetIMacPllr(plbsFtn, 0);
	/* if we run out of memory in FAssignLr ilrCur will be invalid,
		so set it to ilrNil here. */
	plbsFtn->ilrCur = ilrNil;
	PutIMacPlc(vhplcfrl, 0);
	plbsFtn->yl = plbsFtn->ylColumn = 0;    /* tracks the amount of space used for footnotes */
	plbsText->yl = ylCol;   /* tracks the amount of space used for text */
	plbsFtn->fFirstColumn = plbsText->fFirstColumn;
	plbsFtn->xl = plbsText->xl;
	plbsFtn->dxlColumn = plbsText->dxlColumn;
	plbsText->ylMaxColumn = plbsFtn->ylMaxColumn = plbsText->yl + dylFill;
	plbsFtn->fAbsPresent = fFalse;
	plbsFtn->dcpDepend = 0;
	plbsText->fPrevTable = plbsFtn->fPrevTable = fFalse;

	PushLbs(plbsText, &lbsTextStart);   /* save current text state */
	PushLbs(plbsFtn, &lbsFtnStart);   /* save current ftn state */

/* process overdue footnotes */
	if (plbsFtn->fContinue)
		{
		PushLbs(plbsFtn, &lbsSave);
		plbsFtn->yl = plbsFtn->ylColumn = ylCol;
#ifndef WIN
		if (plbsText->fAbsPresent)
			{
			/* allow abs objects to perturb continuation */
			plbsFtn->fAbsPresent = fTrue;
			CopyAbsLrs(plbsText->hpllr, plbsFtn->hpllr, fTrue, fTrue);
			}
#endif
		CopyHdtLrs(ihdtTFtnCont, plbsFtn, ylCol, plbsText);
		/* cpLimFtn is end of last footnote text with ref < cpText;
			leave room for continuation footer in case we need it */
		ylT = plbsFtn->yl;
		dylRemain -= vrghdt[ihdtBFtnCont].dyl;
		cpLimFtn = CpFirstRef(plbsText->doc, CpFromCpCl(plbsText, fTrue), &cpRef, edcDrpFtn);
		lbcFtn = LbcFormatBlock(plbsFtn, cpLimFtn, dylRemain, fEmptyOK, fFalse, fFalse);

		fEndnotes = (fpc == fpcEndnote || fpc == fpcEndDoc);
		if (lbcFtn == lbcYlLim)
			{
			/* didn't fit, append footnote continuation footer */
			if (plbsFtn->yl == ylT)
				SetIMacPllr(plbsFtn, 0);
			lbc = lbcYlLim;
			goto LEndFtn;
			}
		if (fEndnotes)
			/* footnotes at end of document or division... */
			goto LEndFtn;
		CacheSectL(lbsTextStart.doc, lbsTextStart.cp, lbsTextStart.fOutline);
		fNoText = fFalse;
		if (lbsTextStart.cp == caSect.cpFirst && lbsTextStart.cl == 0)
			{
			if (fpc == fpcBeneathText)
				goto LEndFtn;
			fNoText = fTrue;
			}
		/* we aren't going to keep the footnotes we just laid out
			because the yl was wrong; but set yl to end of continued
			footnotes so subsequent footnote calculations will work */
		dylContin = DylLrsForDoc(plbsFtn);
		dylRemain -= vrghdt[ihdtTFtnCont].dyl;
		PopLbs(&lbsSave, plbsFtn);
		/* footnotes zero-based */
		plbsFtn->yl = dylContin + vrghdt[ihdtTFtnCont].dyl;
		fEmptyOKFtn = fEmptyOK;
		fEmptyOK = fTrue;
		plbsFtn->fFirstPara = fFalse;
		if (fNoText)
			goto LEndnotes;
		}

/* process text - this will collect footnote references and their cleavage
	points into vhplcfrl; plbsText->fEndnotes is set if endnotes needed */
	lbc = LbcFormatBlock(plbsText, cpLim, dylRemain - dylContin, fEmptyOK, fTrue, fFalse);
	ifrlMac = IMacPlc(vhplcfrl);
	fEndnotes = plbsText->fEndnotes;
	plbsText->fCpOnPage |= (plbsText->cp != lbsTextStart.cp ||
			plbsText->cl != lbsTextStart.cl);
	if (!plbsFtn->fContinue && ifrlMac == 0 && !fEndnotes)
		{
		PopLbs(&lbsFtnStart, 0);
		goto LEndCol;   /* no footnote references */
		}
	/* leave room for continuation separator */
	dylFill -= vrghdt[ihdtBFtnCont].dyl;
	fRetryFtn = fFalse;
	pdod = PdodDoc(plbsFtn->doc);	/* two lines for compiler */
	hplcfnd = pdod->hplcfnd;

/* leave room for footnote header */
	if (!plbsFtn->fContinue)
		{
		int dylFtn = vrghdt[ihdtTFtn].dyl;
		if (dylRemain <= dylFtn)
			{
			if (fEndnotes)
				{
				if (fEmptyOK)
					{
					PopLbs(&lbsFtnStart, 0);
					goto LEndCol;
					}
				}
			else					
				{
				GetPlc(vhplcfrl, ifrl = 0, &frl);
				if (fEmptyOK || frl.ylReject > plbsText->ylColumn)
					goto LCleaveAboveFtn;
				}
			}
		dylRemain -= dylFtn;
		plbsFtn->yl += dylFtn;
		}

#ifndef WIN
/* copy absolute objects to footnote lbs so footnotes will be perturbed */
	if (plbsText->fAbsPresent)
		{
		plbsFtn->fAbsPresent = fTrue;
		CopyAbsLrs(plbsText->hpllr, plbsFtn->hpllr, fTrue, fTrue);
		PushLbs(plbsFtn, &lbsAbsSave);
		plbsFtn->ylColumn = plbsFtn->yl;
		}
#endif

/* process endnotes in remaining space */
	if (fEndnotes || ifrlMac == 0)
		{
LEndnotes:
		ylT = plbsFtn->yl;	/* save height of continuation */
		PopLbs(&lbsFtnStart, plbsFtn);
		plbsFtn->ylColumn = plbsText->yl;
		if (plbsFtn->fContinue)
			{
			if (fpc == fpcBottomPage)
				{
				plbsFtn->ylColumn = max(plbsFtn->ylColumn, ylCol + dylFill - ylT);
#ifndef WIN
				if (plbsText->fAbsPresent)
					plbsFtn->ylColumn = max(plbsText->yl, plbsFtn->ylColumn - DylDeadSpace(plbsText));
#endif
				}
			goto LFtnBlock;
			}
		if (plbsText->yl > ylCol)
			fEmptyOKFtn = fTrue;
		/* start collecting footnotes from beginning of last
			section without fEndnote (fpcEndnote) or beginning
			of document (fpcEndDoc). */
#ifdef PCWORD
		Assert(fpc == fpcEndnote);
		CacheSectL(lbsTextStart.doc, lbsTextStart.cp, lbsTextStart.fOutline);
		cpRef = caSect.cpFirst;
#else
		if (fpc == fpcEndDoc)
			cpRef = cp0;
		else
			{
			Assert(fpc == fpcEndnote);
			CacheSectL(lbsTextStart.doc, lbsTextStart.cp, lbsTextStart.fOutline);
			for (cpRef = caSect.cpFirst; cpRef > cp0; cpRef = caSect.cpFirst)
				{
				CacheSectL(lbsTextStart.doc, cpRef - 1, lbsTextStart.fOutline);
				if (FEndnoteSep(vsepFetch))
					break;
				}
			}
#endif /* !PCWORD */

		if (cpRef >= CpFromCpCl(plbsText, fTrue))
			goto LEndCol;
		plbsFtn->cp = CpFirstRef(lbsTextStart.doc, cpRef, &cpRef, edcDrpFtn);
		plbsFtn->cl = 0;
		goto LFtnBlock;
		}

/* process footnotes in remaining space */
	GetPlc(vhplcfrl, 0, &frl);
	if (frl.ylReject > vylTopPage)
		fEmptyOK = fTrue;
	fEmptyOKFtn = fEmptyOK;
	if (!plbsFtn->fContinue)
		dylRemain -= vrghdt[ihdtBFtnCont].dyl;
#ifdef WIN
	else
		dylRemain -= dylContin;
#else
	else  if (!plbsFtn->fAbsPresent)
		dylRemain -= dylContin;
#endif
	ylCleave = ylLarge;
	fFtnOnPage = fFalse;
	dyl = IfWinElse(0, (plbsFtn->fAbsPresent) ? lbsAbsSave.yl : 0);
	for (ifrl = 0; ifrl < ifrlMac; ifrl++, fEmptyOKFtn = fTrue)
		{
		GetPlc(vhplcfrl, ifrl, &frl);
		/* first check cleave point below ref ifrl */
		if (fEmptyOKFtn && frl.ylReject > ylCol + dylRemain - dylFtn)
			goto LCleaveAboveFtn;
LFormatFtn:
		cpFtn = CpPlc(hplcfnd, frl.ifnd);
#ifndef WIN
		if (plbsFtn->fAbsPresent)
			{
			PopLbs(&lbsAbsSave, plbsFtn);
			PushLbs(plbsFtn, &lbsAbsSave);
			plbsFtn->yl = plbsFtn->ylColumn = frl.ylAccept + lbsAbsSave.yl;
			}
		else
#endif
			{
			plbsFtn->cp = cpFtn;
			plbsFtn->cl = 0;
			}
		cpLimFtn = CpPlc(hplcfnd, frl.ifnd + 1);
		PushLbs(plbsFtn, &lbsSave);     /* remember state */
		lbc = LbcFormatBlock(plbsFtn, cpLimFtn, 
				dylFill + ylCol - frl.ylAccept - dyl,
				fEmptyOKFtn, fFalse, fFalse);
		if (lbc == lbcNil)
			{
#ifndef WIN
			if (plbsFtn->fAbsPresent)
				dylFtn = DylLrsForDoc(plbsFtn);
			else
#endif
				dylRemain -= plbsFtn->yl - lbsSave.yl;   /* amount added */
			fRetryFtn = fFalse;
			fFtnOnPage = fTrue;
			continue;
			}
		if (lbc == lbcEndOfDoc)
			{
			/* everything will fit */
			dylRemain -= plbsFtn->yl - lbsSave.yl;   /* amount added */
			ylCleave = ylLarge;
			fFtnOnPage = fTrue;
			break;
			}
		Assert(lbc == lbcYlLim);
		if (!fRetryFtn && frl.fNormal && FGetFtnBreak(plbsText, ifrl, &frl, fpc))
			{
			/* now we have accurate yl information to accept
				or reject the footnote; try again */
			PopLbs(&lbsSave, plbsFtn);   /* restore state from before footnote */
			fRetryFtn = fTrue;
			goto LFormatFtn;
			}
#ifndef WIN
		if (plbsFtn->fAbsPresent && plbsFtn->cp <= cpFtn)
			{
			/* no progress; reset yl to where it was from last
				footnote that fit */
			plbsFtn->yl = plbsFtn->ylColumn + dylFtn;
			/* guard against footnotes in APOs getting assigned
				but not used */
			cpLim = CpPlc(vhplcfrl, ifrl);
			goto LCleaveAboveFtn;
			}
#endif
		if (plbsFtn->yl == lbsSave.yl)
			{
			/* no progress */
LCleaveAboveFtn:
			/* this will change frl.ylReject for the better or not at all */
			if (!fRetryFtn)
				FGetFtnBreak(plbsText, ifrl, &frl, fpc);
			ylCleave = frl.ylReject;
			break;
			}
		ylCleave = frl.ylAccept;
#ifndef WIN
		if (plbsFtn->fAbsPresent)
			dylFtn = DylLrsForDoc(plbsFtn);
		else
#endif
			dylRemain -= plbsFtn->yl - lbsSave.yl;   /* amount added */
		fFtnOnPage = fTrue;
		break;
		} /* next ifrl */

/* Here we have a list of lr's in plbsFtn->hpllr that fit on the page, and
	their references and the required neighbourhoods of the references also
	fit.  Just re-fit the main text subtracting the total footnotes from the
	available space, but not below the "cleavage point" below which there
	are references that we could not satisfy. */
#ifndef WIN
	if (plbsFtn->fAbsPresent)
		{
		ylT = dylFtn + lbsAbsSave.yl;
		/* take abs objects that span into account; OK to trash this,
			it's popped below */
		plbsText->yl = ylCol + dylFill - ylT;
		plbsText->yl = max(plbsText->yl - DylDeadSpace(plbsText),
				(ylCleave == ylLarge) ? frl.ylAccept : ylCleave);
		dylRemain -= DylDeadSpace(plbsText) + dylFtn;
		}
	else
#endif
		ylT = plbsFtn->yl;
	cpMacDepend = plbsText->dcpDepend + plbsText->cp;
	PopLbs(&lbsFtnStart, plbsFtn);
LRedoText:
	PopLbs(&lbsTextStart, plbsText);
	PushLbs(plbsText, &lbsTextStart);
	lbc = LbcFormatBlock(plbsText, cpLim, min(ylCleave - ylCol, dylRemain), 
			plbsFtn->fContinue, fFalse, fFalse);
	plbsText->dcpDepend = max(plbsText->dcpDepend, (int) CpMin(0x7fffL, cpMacDepend - plbsText->cp));
	plbsText->fCpOnPage |= (plbsText->cp != lbsTextStart.cp ||
			plbsText->cl != lbsTextStart.cl);
	if (!fFtnOnPage && !plbsFtn->fContinue)
		{
		if (vcBalance > 0 && CpPlc(vhplcfrl, 0) < CpFromCpCl(plbsText, fTrue))
			lbc = lbcYlLim;	/* force out-of-room for balancing */
		goto LEndCol;	/* constrained by footnotes, but no ref */
		}

/* now we re-fill the footnotes to be sure that widow control, keep, etc.
	didn't remove any footnote references */
	/* remember that plbsFtn is zero-based at the moment */
	plbsFtn->ylColumn = plbsText->yl;
	if (fpc == fpcBottomPage)
		{
		plbsFtn->ylColumn = max(plbsFtn->ylColumn, ylCol + dylFill - ylT);
#ifndef WIN
		if (plbsText->fAbsPresent)
			plbsFtn->ylColumn = max(plbsText->yl, plbsFtn->ylColumn - DylDeadSpace(plbsText));
#endif
		}
	fEmptyOKFtn = fEmptyOK;
LFtnBlock:
	plbsFtn->yl = plbsFtn->ylColumn; /* plbsFtn is now absolute */
	/* we know from above logic that there is room for separator */
	if (plbsFtn->fContinue)
		fEmptyOKFtn = fFalse;
	else  if (!fEndnotes)
		{
		GetPlc(vhplcfrl, 0, &frl);
		plbsFtn->cp = CpPlc(hplcfnd, frl.ifnd);
		plbsFtn->cl = 0;
		}
	cpLimFtn = CpFirstRef(plbsText->doc, CpFromCpCl(plbsText, fTrue), &cpRef, edcDrpFtn);
	Assert(cpLimFtn >= plbsFtn->cp);
	if (cpLimFtn == plbsFtn->cp)
		/* no references */
		goto LEndCol;

	CopyHdtLrs((plbsFtn->fContinue) ? ihdtTFtnCont : ihdtTFtn, plbsFtn, plbsFtn->yl, plbsText);
	lbsFtnStart.yl = plbsFtn->yl;
#ifndef WIN
	if (plbsText->fAbsPresent)
		{
		plbsFtn->fAbsPresent = fTrue;
		CopyAbsLrs(plbsText->hpllr, plbsFtn->hpllr, fTrue, fTrue);
		}
#endif
	lbcFtn = LbcFormatBlock(plbsFtn, cpLimFtn, ylCol + dylFill - plbsFtn->ylColumn,
			fEmptyOKFtn, fFalse, fFalse);
	if (lbcFtn == lbcYlLim && plbsFtn->yl == lbsFtnStart.yl)
		{
		/* we can't fit anything */
		Assert(!plbsFtn->fContinue);
		fFtnOnPage = fFalse;
		if (fEndnotes)
			{
			/* no room for endnotes, try next page */
			plbsFtn->fContinue = fTrue;
			lbc = lbcYlLim;
			goto LEndCol;
			}
		if (!FGetFtnBreak(plbsText, 0, &frl, fpc))
			GetPlc(vhplcfrl, 0, &frl);
		ylCleave = frl.ylReject;
		goto LRedoText;
		}

#ifndef WIN
/* determine whether some footnotes did not fit due to abs objects - if there
	are such, reformat the text yet AGAIN, limiting to last included footnote */
	if (plbsText->fAbsPresent && lbcFtn == lbcYlLim && !fEndnotes)
		{
		cpMacFtn = CpPlc(hplcfnd, IInPlc(hplcfnd, cpLimFtn - ccpEop));
		if (plbsFtn->cp > cpMacFtn ||
				(plbsFtn->cp == cpMacFtn && plbsFtn->cl > 0))
			goto LEndFtn;
		/* cleave point is before first ftn not included */
		ifrl = IInPlc(vhplcfrl, CpRefFromCpSub(plbsFtn->doc, plbsFtn->cp, edcDrpFtn));
		if (plbsFtn->cl != 0 ||
				plbsFtn->cp != CpPlc(hplcfnd, IInPlc(hplcfnd, plbsFtn->cp)))
			ifrl++;
		CacheParaL(plbsText->doc, cpRef = CpPlc(vhplcfrl, ifrl), plbsText->fOutline);
		if (FAbsPapM(plbsText->doc, &vpapFetch))
			goto LEndFtn;	/* let them continue */
		if (!FGetFtnBreak(plbsText, ifrl, &frl, fpc))
			GetPlc(vhplcfrl, ifrl, &frl);
		cpMacDepend = plbsText->dcpDepend + plbsText->cp;
		PopLbs(&lbsTextStart, plbsText);
		PushLbs(plbsText, &lbsTextStart);
		lbc = LbcFormatBlock(plbsText, cpLim, frl.ylReject - ylCol, fFalse, fFalse, fFalse);
		plbsText->dcpDepend = max(plbsText->dcpDepend, (int) CpMin(0x7fffL, cpMacDepend - plbsText->cp));
		plbsText->fCpOnPage = fTrue;
		}
#endif

/* now footnotes are done - check whether we had to continue them */
LEndFtn:
	plbsFtn->fContinue = fFalse;	/* assume no continuation */
	if (lbcFtn == lbcYlLim)
		{
		if (fEndnotes)
			{
			plbsFtn->dcpDepend = (int) CpMin(0x7fffL, cpLimFtn - plbsFtn->cp);
			goto LContinued;
			}
		pdod = PdodDoc(plbsFtn->doc);   /* two lines for compiler */
		hplcfnd = pdod->hplcfnd;
		cpMacFtn = CpPlc(hplcfnd, ifrl = IInPlc(hplcfnd, plbsFtn->cp));
		if (plbsFtn->cl != 0 || plbsFtn->cp > cpMacFtn)
			{
			plbsFtn->dcpDepend = (int) CpMin(0x7fffL, CpPlc(hplcfnd, ifrl + 1) - plbsFtn->cp);
LContinued:
			plbsFtn->fContinue = fTrue;	/* more text for ftn */
			CopyHdtLrs(ihdtBFtnCont, plbsFtn, plbsFtn->yl, plbsText);
			}
		else  if (plbsFtn->cp < cpLimFtn)
			plbsFtn->fContinue = fTrue;	/* more footnotes */
		}
	else  if (plbsFtn->cp < cpLimFtn)
		plbsFtn->fContinue = fTrue;	/* more footnotes */
	else  if (fpc == fpcEndnote || fpc == fpcEndDoc)
		{
		Assert(lbc == lbcEndOfDoc || lbc == lbcEndOfSection || lbc == lbcNil);
		Assert(plbsText->cl == 0);
		lbc = (plbsText->cp >= CpMacDocPlbs(plbsText)) ? lbcEndOfDoc : lbcEndOfSection;
		}
	else  if (lbc == lbcNil && lbsFtnStart.fContinue)
		{
		if (plbsText->cp >= CpMacDocPlbs(plbsText))
			lbc = lbcEndOfDoc;
		else  if (plbsText->cl == 0)
			{
			CacheSectL(plbsText->doc, plbsText->cp, plbsText->fOutline);
			if (plbsText->cp == caSect.cpFirst)
				lbc = lbcEndOfSection;
			}
		}

/* append footnotes, if any, to text */
	if (IMacPllr(plbsFtn) != 0)
		{
		if (plbsFtn->fContinue)
			/* continued ftns: must not be enough room */
			lbc = lbcYlLim;
		if (fpc == fpcBottomPage)
			{
			plbsText->fNoBalance = fTrue;     /* no balancing allowed */
			plbsFtn->yl = plbsText->ylMaxColumn;
			}
		CopyLrs(plbsFtn->hpllr, plbsText, IMacPllr(plbsFtn), ylLarge, fFalse, -1);
		plbsText->yl = plbsFtn->yl; /* ftn lrs are excessively tall */
		plbsText->dylUsedCol += plbsFtn->dylUsedCol;
		plbsText->dyllTotal += plbsFtn->dyllTotal;
		plbsText->clTotal += plbsFtn->clTotal;
		}

LEndCol:
	PopLbs(&lbsTextStart, 0);
	plbsText->fFirstColumn = fFalse;
	StopProfile();
	return(lbc);
}


/*********************************/
/* L b c  F o r m a t  B l o c k */
#ifndef WIN
NATIVE /* WINIGNORE - LbcFormatBlock */
#endif /* !WIN */
/* %%Function:LbcFormatBlock %%Owner:chrism */
int LbcFormatBlock(plbs, cpLim, dylFill, fEmptyOK, fCollectFtn, fAbs)
struct LBS *plbs;
CP cpLim;
int dylFill;
int fEmptyOK;   /* !fEmptyOK means we are in a situation where zero advance
is not acceptable, and restrictions (keep etc.) may
be relaxed. */
int fCollectFtn; /* OK to collect footnotes; there may or may not be any */
int fAbs;       /* collecting paragraphs for absolute object */
{
/* This procedure will collect paragraphs in lbs until limiting cp or yl is
	reached.  Keep, Keep With Next, Side by Side, widows/orphans are treated.
	If fCollectFtn, a list of frl's is created in vhplcfrl for the scanned text */
	int fKeepFollow;
	int fNormal;            /* not keep and not table */
	int fLastLine;
	int fInTable;
	int fFRLNeeded;         /* need to collect ftn references */
	int fEndnotes;
	int fKFSaved;           /* means keep follow is in progress */
	int fKFAtTop;
	int fParaStart;         /* working with start of para */
	int fNeedLr = fTrue;    /* need new LR for next para */
	int fWidow;    /* widow control enabled */
	int fFtn;
	int fShort;
	int ifrd;
	int fNeedHeight;        /* don't know height of abs obj */
	int fAbsHit;
	struct DOD *pdod;
	int lbc = lbcNil, fpc;
	int doc = plbs->doc;
	int ylAccept, ylReject, ylT, ylMaxSave, ylMaxLr;
	CP cpFirstFnd, cpLimFnd, cpRef, cpMac;
	LRP lrp;
	int rgw[cwPAPBaseScan];
	struct LBS lbsNew, lbsT, lbsKeep;

	StartProfile();

	fFRLNeeded = fEndnotes = fKFSaved = fFalse;
	pdod = PdodMother(doc);		/* two lines for compiler */
	fWidow = !fAbs && FWidowPdod(pdod);
	plbs->fEndnotes = fFalse;
	plbs->dylOverlap = 0;
	plbs->ilrFirstChain = ilrNil;   /* no chained LRs */

/* check for end of doc due to long footnotes that ran on after text ended */
	cpMac = CpMacDocPlbs(plbs);
	if (plbs->cp > cp0 && plbs->cp >= cpMac)
		{
		StopProfile();
		return(lbcEndOfDoc);
		}
	if (plbs->cp >= cpLim)
		{
		StopProfile();
		return(lbcNil);
		}

	if (fAbs)
		ylMaxSave = plbs->ylMaxBlock;
	else
		plbs->ilrCur = ilrNil;
	plbs->ylMaxBlock = plbs->ylColumn + dylFill;
	pdod = PdodDoc(doc);
#ifdef PCWORD
	vfli.cpFirstLr = plbs->cp;
	fFtn = fShort = (plbs->cp > CpMacText(doc));
#else
	fShort = pdod->fShort;
	pdod = PdodDoc(doc);
	fFtn = pdod->fFtn;
#endif /* !PCWORD */

/* determine if there are footnote references in the cp range */
	if (fCollectFtn)
		{
#ifdef PCWORD
		fpc = FAssign(fEndnotes = vsepFetch.fEndFtns) ?
				fpcEndnote : fpcBottomPage;
		if (FFtnsPdod(pdod) && !fEndnotes)
#else
			fpc = pdod->dop.fpc;
		if (!plbs->fOutline && !fShort && FFtnsPdod(pdod) &&
				(fEndnotes = (fpc == fpcEndnote || fpc == fpcEndDoc)) == fFalse)
#endif /* !PCWORD */
			{
			cpFirstFnd = CpFromCpCl(plbs, fTrue);/* heap movement */
			cpLimFnd = cpLim;
			pdod = PdodDoc(doc);
			ifrd = IInPlcRef(pdod->hplcfrd, cpFirstFnd); /* used way below */
			cpRef = CpPlc(pdod->hplcfrd, ifrd);
			if (cpRef >= cpFirstFnd && cpRef < cpLimFnd)
				{
				PutCpPlc(vhplcfrl, 0, cpLimFnd);
				fFRLNeeded = fTrue;	/* footnote references found */
				}
			}
		}

/* we are at plbs->cp,cl - handle next para */
/* depending on paragraph type, advance lbs into lbsNew...
	lbc, fNormal, ylAccept, ylReject are used at LAdvanceToLbsNew */
/* get para props (cl's are not supposed to cross over to other paras....) */
	for (;;)
		{
		CacheParaL(doc, plbs->cp, plbs->fOutline);
		fParaStart = (plbs->cl == 0 && plbs->cp == caParaL.cpFirst);

/* page break before */
		if (FPageBreakBeforePap(vpapFetch) && !fShort && fParaStart &&
				!(plbs->fFirstColumn && plbs->fFirstPara))
			{
			lbc = lbcPageBreakBefore;
			fKFAtTop = fTrue;	/* ensure no keep */
			plbs->dcpDepend = (int) CpMin(0x7fffL, caPara.cpLim - caPara.cpFirst);
			break;	/* goto LEndBlock */
			}
		fNormal = fFalse; /* assume keep or table */
		fKeepFollow = !fAbs && vpapFetch.fKeepFollow && (!fShort || fFtn);

		fInTable = FInTableVPapFetch(doc, plbs->cp);
		if (plbs->fOutline)
			caParaL.doc = docNil;	/* props got refreshed */
		CacheParaL(doc, plbs->cp, plbs->fOutline);

/* assign (create if necessary) a layout rectangle */
LAssignLr:      
		PushLbs(plbs, &lbsNew);
		if (fAbs)
			goto LNormalPara;
		if (fNeedLr)
			{
			fNeedHeight = FAssignLr(&lbsNew, dylFill, fEmptyOK);
			if (lbsNew.ilrCur == ilrNil)
				goto LNoRoomForLr;
			lrp = LrpInPl(lbsNew.hpllr, lbsNew.ilrCur);
			if (lrp->dyl < 0 && fEmptyOK)
				{
				/* for footnotes that don't fit */
				goto LNoRoomForLr;
				}
			ylMaxLr = lrp->yl + lrp->dyl;
			fAbsHit = lrp->lrk == lrkAbsHit;
			if (lrp->lrk == lrkAbs)
				{
				/* make sure simple LR interrupted by this APO
					gets marked apo-affected (for page view) */
				if (lbsNew.fSimpleLr && plbs->ilrCur != ilrNil && plbs->ilrCur < IMacPllr(&lbsNew))
					{
					lrp = LrpInPl(lbsNew.hpllr, plbs->ilrCur);
					if (lrp->cpLim == plbs->cp && lrp->lrk == lrkNormal)
						lrp->lrk = lrkAbsHit;
					}

				if (fNeedHeight)
					/* recursing may lose an hpllr if we
						abort */
					vplbsFrame = &lbsNew;
				lbc = LbcPositionAbs(&lbsNew, cpLim, dylFill, fNeedHeight, fEmptyOK);
				if (lbc == lbcYlLim)
					{
					plbs->dcpDepend = (int) CpMin(0x7fffL, lbsNew.cp + lbsNew.dcpDepend - plbs->cp);
					goto LNoRoomForLr;
					}
				ylReject = lbsNew.yl;
				fNeedLr = fTrue;
				fEmptyOK = fTrue;
				goto LAdvanceToLbsNew;
				}
			/* ensure correct caPara */
			CacheParaL(doc, plbs->cp, plbs->fOutline);
			}

/* side-by-side */
#ifndef WIN 
#ifndef JR
		if (vpapFetch.fSideBySide && !fInTable)
			{
			ylReject = lbsNew.yl;
			if ((lbc = LbcSideBySide(&lbsNew, cpLim, dylFill, fEmptyOK, &fKeepFollow)) == lbcYlLim)
				{
				plbs->dcpDepend = lbsNew.dcpDepend;
				if (fAbsHit)
					{
					lrp = LrpInPl(lbsNew.hpllr, lbsNew.ilrCur);
					if (lrp->dyl > 0 && lrp->yl + lrp->dyl < lbsNew.ylMaxBlock)
						{
						/* there's hope that we can fill in
							below the apo that constrained us */
						goto LNewLrNewYl;
						}
					}
				if (fEmptyOK)
					goto LNoRoomForLr;
				}
			if (fKFSaved && (fKFSaved = fKeepFollow) == fFalse)
				{
				/* turn it off if last para not keep follow */
				CopyLbs(0, &lbsKeep);
				}
			fEmptyOK = fTrue;
			goto LAdvanceToLbsNew;
			}
#endif /* JR */
#endif /* !WIN */

/* handle keep */
		if (FKeepPap(vpapFetch) && fEmptyOK && !fInTable && lbsNew.yl != vylTopPage)
			{
			lbc = LbcFormatPara(&lbsNew, dylFill, clMax);
			if (lbc != lbcYlLim || lbsNew.cp == caParaL.cpLim)
				{
				ylReject = plbs->yl;
				if (lbc == lbcEndOfPage)
					{
					/* page break in keep para - ignore keep */
					fNormal = fTrue;
					fLastLine = fFalse;
					}
				fEmptyOK = fTrue;
				goto LAdvanceToLbsNew;
				}
			plbs->dcpDepend = lbsNew.dcpDepend + (int) (lbsNew.cp - plbs->cp);
			if (!fAbsHit)
				{
				/* kept para did not fit */
LNoRoomForLr:
				lbc = lbcYlLim;
				CopyLbs(0, &lbsNew);
				vplbsFrame = 0;
#ifndef WIN
				vplbsFrameSxs = 0;
#endif
				goto LEndBlock;
				}
			/* para that must be kept intact doesn't fit because
				of abs obj - waste the space & freeze previous lr */
			lbsNew.dylOverlap += max(0, ylMaxLr - plbs->yl);
LNewLrNewYl:
			lrp = LrpInPl(lbsNew.hpllr, lbsNew.ilrCur);
			plbs->yl = lrp->yl + lrp->dyl;
LNewLr:
			if (lbsNew.ilrCur < IMacPllr(plbs))
				{
				lrp = LrpInPl(plbs->hpllr, lbsNew.ilrCur);
				lrp->lrs = lrsFrozen;	/* no more text in here */
				lrp->dyl = plbs->yl - lrp->yl;
				}
			else
				{
				Assert(lbsNew.dylOverlap >= 0);
				plbs->dylOverlap = lbsNew.dylOverlap;
				}
			CopyLbs(0, &lbsNew);
			fNeedLr = fTrue;
			goto LAssignLr;
			}

/* we have a normal paragraph (possibly with keep follow) */
LNormalPara:
		fNormal = !fAbs && !fInTable;
		ylReject = plbs->yl;
		fLastLine = fFalse;	/* haven't seen last line of para */

		lbc = LbcFormatPara(&lbsNew, dylFill, clMax);
		if (lbc == lbcNil || lbc == lbcEndOfDoc || lbc == lbcEndOfSection)
			fLastLine = fTrue;
		else  if (lbc == lbcYlLim)
			{
			if (lbsNew.cp == plbs->cp && lbsNew.cl == plbs->cl)
				{
				/* no progress */
				if (fAbsHit)
					{
					lrp = LrpInPl(lbsNew.hpllr, lbsNew.ilrCur);
					if (lrp->dyl > 0 && lrp->yl + lrp->dyl < lbsNew.ylMaxBlock)
						{
						/* there's hope that we can fill in
							below the apo that constrained us */
						plbs->dylOverlap = lbsNew.dylOverlap;
						goto LNewLr;
						}
					}
				if (!fEmptyOK && (vcBalance == 0 || lbsNew.yl == vylTopPage))
					{
					/* we must fit at least one line */
					lbc = LbcAdvanceOneLine(&lbsNew);
					fEmptyOK = fTrue;
					goto LAdvanceToLbsNew;
					}
				}
#ifndef JR
			/* we have a break in the paragraph. check for
				widows/orphans */
			else  if (fWidow && !fInTable && lrp->ihdt == ihdtNil && 
					ylMaxLr >= lbsNew.ylMaxBlock &&
					!FWidowControl(plbs, &lbsNew, dylFill, fEmptyOK))
				{
				plbs->dcpDepend = lbsNew.dcpDepend;
				CopyLbs(0, &lbsNew);
				goto LEndBlock;
				}
#endif /* JR */
			if (fAbsHit)
				{
				lbc = lbcNil;
				fNeedLr = fTrue;
				}
			else 				
				{
				CacheParaL(doc, plbs->cp, plbs->fOutline);
				if (lbsNew.cp == caParaL.cpLim)
					fLastLine = fTrue;
				}
			}

		if (lbsNew.yl > plbs->yl)
			fEmptyOK = fTrue;

LAdvanceToLbsNew:

/* when we accept a block, in toto or just before returning, we scan it for
	footnote references. fNormal means that yl* must be calculated
	separately for each reference. */

/* we must have: lbsNew, fNormal, lbc, fKeepFollow, if !fNormal: ylRject.
	First check for keep follow in the paragraph just ended. Note: we assume
	fEmptyOK is false and use fKFAtTop instead; see note at LEndBlock */
		ylAccept = lbsNew.yl;
		if (!fKeepFollow || fNormal && !fLastLine)
			fKeepFollow = fFalse;
		else  if (!fKFSaved)
			{
			fKFSaved = fTrue;
			PushLbs(plbs, &lbsKeep);
			/* our concern here is that keep follow not create
				an empty column */
			fKFAtTop = plbs->yl == vylTopPage;
			}

/* update the footnote references */
		if (fFRLNeeded)
			ifrd = IfrdGatherFtnRef(plbs, &lbsNew, ifrd, fNormal, ylReject, ylAccept);

		if (fKFSaved && (!fKeepFollow && !(lbc == lbcYlLim && plbs->yl == lbsNew.yl) ||
				lbc == lbcEndOfDoc || lbc == lbcEndOfSection || lbc == lbcEndOfColumn))
			{
			/* something used from this para, keep follow satisfied */
			fKFSaved = fFalse;
			CopyLbs(0, &lbsKeep);
			}

/* accept the paragraph */
		CopyLbs(&lbsNew, plbs);
		if (!fAbs)
			/* if not recursing, we now have all LBS's on stack */
			vplbsFrame = 0;
#ifndef WIN
		vplbsFrameSxs = 0;
#endif
		if (lbc != lbcNil || plbs->cp >= cpLim)
			break;
		} /* end of loop for paragraphs */

/* now we have layout rectangles for the block - check for interrupted keep
	follow, which means we must back up to the beginning of the keep follow
	paragraphs and discard all footnote references past that point.  Note
	that if it was interrupted and the first keep follow para was at the top
	of a column, we ignore keep follow altogether; this prevents indiscriminate
	use of keep follow from creating pages with one paragraph each */
LEndBlock:
	if (fKFSaved && !fKFAtTop && plbs->cp < cpLim)
		{
		cpMac = plbs->dcpDepend + plbs->cp;
		CopyLbs(&lbsKeep, plbs);
		fKFSaved = fFalse;
		plbs->dcpDepend = (int) (cpMac - plbs->cp);
		if (fCollectFtn && IMacPlc(vhplcfrl))
			PutIMacPlc(vhplcfrl, IInPlcRef(vhplcfrl, CpFromCpCl(plbs, fTrue)));
		}
	else  if (fEndnotes)
		{
		if (lbc == lbcEndOfDoc)
			plbs->fEndnotes = fTrue;
		else  if (lbc == lbcEndOfSection)
			{
			CacheSectL(doc, CpMax(cp0, plbs->cp - ccpSect), plbs->fOutline);
			plbs->fEndnotes = fpc == fpcEndnote && FEndnoteSep(vsepFetch);
			}
		}
	if (fKFSaved)
		CopyLbs(0, &lbsKeep);
	if (fAbs)
		plbs->ylMaxBlock = ylMaxSave;
	else  if (plbs->fAbsPresent && plbs->ilrCur != ilrNil)
		{
		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
		if (lrp->lrs != lrsFrozen && lrp->lrk == lrkNormal)
			{
			lrp->lrs = lrsFrozen;
			lrp->dyl = plbs->yl - lrp->yl;
			}
		}
	StopProfile();
	return(lbc);
}


#ifdef DEBUGORNOTWIN
/*******************************/
/* L b c  F o r m a t  P a r a */
#ifndef WIN
/* %%Function:LbcFormatPara %%Owner:NOTUSED */
NATIVE int LbcFormatPara(plbs, dylFill, clLim) /* WINIGNORE "C_" if WIN */
#else /* WIN */
/* %%Function:C_LbcFormatPara %%Owner:chrism */
HANDNATIVE int C_LbcFormatPara(plbs, dylFill, clLim)
#endif /* WIN */
struct LBS *plbs;
int dylFill, clLim;
{
/* adds rectangle starting at lbs and ending before dylFill is exhausted and
	clLim is reached; advances lbs; returns lbc:
	lbcEndOfSect
	lbcEndOfColumn
	lbcEndOfPage
	lbcEndOfDoc
	lbcYlLim
	lbcNil means para fits
	saves the height information obtained in the process. advances lbs
*/
	YLL dyllCur, dyllAdvance;
	YLL dyllRemain;
	int dylLine, dylBefore, dylT, dylMax;
	int dcl = -1, dxlT;
	int fParaStart, fEndBreak = fFalse, fChain = fFalse;
	int cl, clT, lbc;
	int fLnn, fTableRow = fFalse;
	int fNoYlLimit = (dylFill == ylLarge);
	int dylText, dylAbove, dylBelow, dylOverlapNew = plbs->dylOverlap;
	int doc = plbs->doc;
#ifndef PCWORD
	struct PAD pad;
	struct PAD **hplcpad;
	struct DOD **hdod;
#endif
	int fShort;
	struct PHE phe;
	CP cpFirst, cpLim, cpMac, cpMacDoc;
	struct LNH lnh, lnhT;
	LRP lrp;
	struct LR lr;
	int rgw[cwPAPBaseScan];
#ifdef DEBUG
	CP cpStart = plbs->cp;
	int clStart = plbs->cl;
#endif /* DEBUG */ 

	StartProfile();
	Assert(vfInFormatPage);
	CacheParaL(doc, plbs->cp, plbs->fOutline);
	cpLim = caParaL.cpLim;
	cpMacDoc = CpMacDocPlbs(plbs);

	bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
	if (lr.lrk == lrkAbsHit)
		{
		if (dylFill != ylLarge)
			dylFill = lr.yl + lr.dyl - plbs->ylColumn;
		if (plbs->cl != 0)
			{
			/* for absolute-affected rectangles, we need raw
				cps since we can't save rectangle */
			plbs->cp = CpFromCpCl(plbs, fTrue);
			plbs->cl = 0;
			}
		}

#ifndef WIN
	if (++vcparaAbort >= vcparaAbortMax)
		{
#endif /* !WIN */
#ifdef WIN
		if (FAbortLayout(plbs->fOutline, plbs))
#else /* MAC */
		if (FAbortLayout(plbs->fOutline))
#endif /* !WIN */
			{
			AbortLayout();
			}
#ifndef WIN
		vcparaAbort = 0;
		}
#endif /* !WIN */

	vfls.fFirstPara = plbs->fFirstPara;

#ifndef PCWORD
	fTableRow = FInTableVPapFetch(doc, plbs->cp);
	if (plbs->fOutline)
		caParaL.doc = docNil;	/* props got refreshed */
	CacheParaL(doc, plbs->cp, plbs->fOutline);

	hdod = mpdochdod[doc];
	fShort = (*hdod)->fShort;
/* in outline mode, ignore paragraphs not being shown */
	if (plbs->fOutline && !fShort && (hplcpad = (*hdod)->hplcpad) != hNil)
		{
		GetPlc(hplcpad, IInPlc(hplcpad, plbs->cp), &pad);
		if (!pad.fShow)
			{
			/* collapsed text */
			plbs->cl = 0;
			lbc = ((plbs->cp = cpLim) >= cpMacDoc) ? lbcEndOfDoc : lbcNil;
			vfls.ca.doc = docNil;
			vfls.ca.cpLim = cpLim;
			goto LAdjustDyl;
			}
		}
#else

	fShort = (plbs->cp > CpMacText(doc));

/* skip paragraphs which are not the appropriate running head type */
	if (!fShort && (vpapFetch.rhc != 0 || lr.ihdt != ihdtNil))
		{
		if (!FRhcInIhdt(vpapFetch.rhc, lr.ihdt) &&
				!FEitherOr(ChFromDocCp(doc, plbs->cp, vprsu.fSeeHidden),
				chSect, chColumnBreak))
			{
			if (vpapFetch.rhc && !FAddToRHTable(plbs))
				AbortLayout();
			goto LIgnorePara;
			}
		}
	if (plbs->fOutline && vpapFetch.fHidden)
		{
LIgnorePara:
		plbs->cl = 0;
		lbc = ((plbs->cp = cpLim) >= cpMacDoc) ? lbcEndOfDoc : lbcNil;
		vfls.ca.doc = docNil;
		vfls.ca.cpLim = cpLim;
		goto LAdjustDyl;
		}
#endif /* PCWORD */

#ifdef MAC
/* skip PostScript paragraphs */
	if (vpapFetch.stc == stcPostScript)
		{
		if (lr.ihdt != ihdtNil)
			vfAbsInHeaders = fTrue;	/* volatile headers */
		if (!plbs->fOutline && !fTableRow && !vpref.fSeeHidden && 
				vchpStc.fVanish)
			{
			if (lr.cp == plbs->cp)
				{
				/* make display faster by skipping invisible stuff
					at start of lr */
				lr.cp = cpLim;
				lr.clFirst = 0;
				if (lr.cp == lr.cpLim)
					{
					/* apo */
					lr.lrs = lrsFrozen;
					lr.lrk = lrkNormal;
					}
				bltLrp(&lr, LrpInPl(plbs->hpllr, plbs->ilrCur), sizeof(struct LR));
				}
			plbs->cp = cpLim;
			plbs->cl = 0;
			lbc = lbcNil;
			fChain = fTrue;	/* skip special lbc stuff */
			vfls.ca.doc = docNil;
			vfls.ca.cpLim = cpLim;
			goto LSetLbc;
			}
		}
#endif /* MAC */

/* chained across absolute object */
/* some of the test for lnn was done by FAssignLr */
	fLnn = lr.lnn != lnnNil && IfWinElse(fTrue, !vpapFetch.fSideBySide) &&
			FLnnPap(vpapFetch) && !fTableRow;
	if (plbs->ilrFirstChain != ilrNil)
		{
		fChain = fTrue;
		lbc = LbcFormatChain(plbs, cpLim, dylFill, clLim, fLnn);
		dylOverlapNew = plbs->dylOverlap;
		goto LSetLbc;
		}

	cl = plbs->cl;
	lbc = lbcNil;
	CacheSectL(caParaL.doc, caParaL.cpFirst, plbs->fOutline);
	fParaStart = (cl == 0 && plbs->cp == caParaL.cpFirst);
	dylBefore = fParaStart ? YlFromYa(vpapFetch.dyaBefore) : 0;
	dylOverlapNew = 0;

#ifndef PCWORD
/* ignore heights in some cases */
	if (plbs->fOutline || lr.lrk == lrkAbsHit || plbs->doc == vdocTemp ||
			fTableRow)
		SetWords(&phe, 0, cwPHE);
#else /* PCWORD */
	if (lr.lrk == lrkAbsHit || plbs->ihdt == ihdtTFtn)
		SetWords(&phe, 0, cwPHE);
#endif /* PCWORD */

/* if we don't know the para's height, calculate it now and save the height in
	hplcphe so we (hopefully) don't have to do this again */
	else  if (!FGetValidPhe(doc, plbs->cp, &phe) ||
			phe.dxaCol != XaFromXl(lr.dxl))
		{
		if (plbs->cp != caParaL.cpFirst)
			{
			SetWords(&phe, 0, cwPHE);
			goto LHavePhe;
			}
		/* limit the number of lines: 255 is the largest clMac that will fit 
		   in phe and represents 53" of 15-pt lines */
		clT = ClFormatLines(plbs, cpLim, ylLarge, ylLarge, 255, lr.dxl, fFalse, lr.lrk != lrkAbs);
		if (vfls.fPageBreak || (clT != 0 && vfls.clMac == clNil))
			SetWords(&phe, 0, cwPHE);
		else  if (clT == 0)
			goto LEmptyPara;
		else  if (vfls.fVolatile)
			/* para has vanished chars but we know its height */
			/* FUTURE chrism: remember vanished state (dsb) */
			phe = vfls.phe;
		else
			{
			/* para is not interrupted, has no vanished chars */
			if (!FSavePhe(&vfls.phe))
				{
				AbortLayout();
				}
			phe = vfls.phe;
			}
		}
#ifdef DEBUGMAC
	else  if ((vdbs.fCkPaph) && phe.dxaCol != 0 && phe.dylHeight != 0)
		{
		clT = ClFormatLines(plbs, cpLim, ylLarge, ylLarge, clMax, lr.dxl, fFalse, lr.lrk != lrkAbs);
		Assert(!FNeRgch(&phe, &vfls.phe, cbPHE));
		}
#endif /* DEBUGMAC */

/* here we have a phe; it may or may not be of use */
LHavePhe:
/* first para for LR */
	if (lr.cp == cpNil)
		{
		Assert(lr.lrk != lrkAbs);
		lr.cp = plbs->cp;
		lr.clFirst = plbs->cl;
		lr.lrs = lrsNormal;
/* ignore space before at top of page, and here's the rule:
   not table or side by side, never at top of section, only in 1-column
   sections, not page break before, and top not affected by apo */
		if (lr.yl == vylTopPage && dylBefore != 0 && fParaStart && 
			!fTableRow && !lr.fConstrainTop && 
			!vpapFetch.fPageBreakBefore && Mac(!vpapFetch.fSideBySide &&)
			vsepFetch.ccolM1 == 0 && lr.cp != caSect.cpFirst)
			{
			lr.yl -= dylBefore;
			lr.fSpaceBefore = fTrue;
			}
		plbs->yl = lr.yl;       /* update to current position */
		}

	InvalFli();	      /* clear fli cache */
	dyllRemain = fNoYlLimit ? yllLarge :
			(YLL)dylFill - plbs->yl + plbs->ylColumn;  /* space left */

/* table */
#ifndef PCWORD
	if (fTableRow)
		{
		Assert(plbs->cl == 0);
		if (lr.lrk == lrkAbsHit)
			{
			dxlT = DxlFromTable(plbs->ww, doc, plbs->cp);
			if (dxlT > lr.dxl && clLim == clMax &&
					(lr.fConstrainLeft || lr.fConstrainRight))
				{
				lbc = lbcYlLim;
				goto LNoChange;
				}
			}
		if (!FTableHeight(plbs->ww, doc, plbs->cp, vlm == lmBRepag/*fAbortOK*/,
				!plbs->fPrevTable, fTrue, &dylText, &dylAbove, &dylBelow))
			AbortLayout();
		dyllAdvance = dyllCur = dylText + dylAbove + dylBelow;
		dcl = 1;
		if (plbs->fPrevTable)
			{
			if (plbs->cp == lr.cp)
				/* 1st thing in lr is table following table */
				lr.yl -= plbs->dylBelowTable;
			dyllAdvance -= plbs->dylBelowTable;
			}
		else  if (plbs->cp == lr.cp)
			lr.fForceFirstRow = fTrue;
		if (lr.tSoftBottom == tPos)
			dyllRemain = plbs->ylMaxLr - plbs->yl;
		if (dyllAdvance > dyllRemain)
			{
			lbc = lbcYlLim;
			if (vylTopPage + dyllAdvance < plbs->ylMaxColumn)
				{
				Assert(dylFill != ylLarge);	 /* are we lying? */
				if (lr.lrk != lrkNormal || plbs->yl != vylTopPage)
					goto LNoChange;
				}
			}
		CpFirstTap(doc, plbs->cp);
		plbs->cp = caTap.cpLim;
		fLnn = fFalse;
		plbs->dylBelowTable = dylBelow;
		}
	else
#endif /* !PCWORD */

/* all equal lines. this is the common, important, fast case */
		if (!phe.fDiffLines)
			{
			dylLine = IfWinElse(YlFromYa(phe.dyaLine), phe.dylLine);
			if (dylLine == 0 || plbs->cp != caParaL.cpFirst)
				goto LFormatPara;       /* invalid phe */
			ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
			vfls.phe = phe;
			clLim = min(clLim, vfls.clMac = phe.clMac);
			dyllRemain -= dylBefore;
			if (lr.tSoftBottom == tPos && clLim == clMax)
				{
			/* dyllRemain is long for OPUS and PCWORD - can't use min */
				dyllRemain += dylLine - 1;
				if (dyllRemain > plbs->ylMaxLr - plbs->yl)
					dyllRemain = plbs->ylMaxLr - plbs->yl;
				}

			if (dylLine > dyllRemain)
				{
				lbc = lbcYlLim;
				Assert(dylFill != ylLarge);	 /* are we lying? */
				goto LNoChange;
				}
#ifndef WIN	/* Assert causes CS compiler native bug (bl) */
			Assert(dyllRemain / dylLine == (int)(dyllRemain / dylLine));
#endif
			dcl = clLim - cl;
			if (!fNoYlLimit && dcl > dyllRemain / dylLine)
				{
				Assert(dyllRemain / dylLine < ylLarge);
				dcl = dyllRemain / dylLine;
				}
			dyllCur = dcl * dylLine;
			Assert(dyllCur == (int)dyllCur);

			if (phe.clMac == cl + dcl)
				{
			/* last line: must carry space after as well;
				if text fits and dyaAfter doesn't, keep text */
				YLL ddyll = YlFromYa(vpapFetch.dyaAfter);
				if (dyllRemain - dyllCur < ddyll)
					ddyll = dyllRemain - dyllCur;
				dyllCur += ddyll;
				plbs->cp = cpLim;
				plbs->cl = 0;
				}
			else
				{
				Assert(dcl >= 0);
				plbs->cl += dcl;
				lbc = lbcYlLim;
				}
			dyllCur += dylBefore;
			}

/* still a common case: height is known and it fits */
		else  if (fParaStart && phe.dxaCol != 0 && !fLnn && clLim == clMax && 
				(dyllCur = IfWinElse(YlFromYa(phe.dyaHeight), phe.dylHeight)) <= dyllRemain)
			{
			ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
			vfls.phe = phe;
			plbs->cp = cpLim;
			plbs->cl = 0;
			dcl = 1;	/* treat it as indivisible */
			}

/* we don't know para's height, or lines are different heights and cl != 0; 
	we have to format the lines */
		else
			{
		/* we only measure the amount of TEXT -- if dyaAfter doesn't
			fit but the text does, that's OK */
LFormatPara:
			if (dylFill == ylLarge)
				dylT = dylMax = ylLarge;
			else
				{
				dylT = (lr.lrk == lrkAbsHit) ? 0 : YlFromYa(vpapFetch.dyaAfter);
				dylMax = dylT + ((lr.tSoftBottom != tPos) ? dylFill : plbs->ylMaxLr - plbs->ylColumn);
				dylT += dylFill;
				}
			clLim = min(clLim, ClFormatLines(plbs, cpLim, dylT, dylMax, clLim, lr.dxl, fFalse, fTrue));
			if (clLim == 0 || (!fParaStart &&
					(cl = plbs->cl + IInPlc(vfls.hplclnh, plbs->cp)) >= clLim))
				{
			/* no lines used */
				if (!vfli.fParaStopped)
					{
				/* not even one line fit */
					lbc = lbcYlLim;
					goto LNoChange;
					}
			/* entire para is invisible, simply skip it; we ignore
				section boundaries for short docs */
LEmptyPara:
				plbs->cl = 0;
				plbs->cp = vfls.ca.cpLim;
				lbc = (vfls.ca.cpLim >= cpMacDoc) ? lbcEndOfDoc :
						(fShort || vfls.ca.cpLim != caSect.cpLim) ?
						lbcNil : lbcEndOfSection;
				if (lbc == lbcNil)
					{
					Debug(Assert(plbs->cp > cpStart ||
							(plbs->cp == cpStart && plbs->cl > clStart)));
					goto LExitLbcFormatPara;
					}
LNoChange:
				bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
				if (lbc == lbcYlLim && lr.lrk == lrkAbsHit)
					{
					dylOverlapNew = plbs->dylOverlap + max(0, lr.yl + lr.dyl - plbs->yl);
					}
				goto LAdjustDyl;
				}
			dcl = clLim - cl;
			GetPlc(vfls.hplclnh, clLim - 1, &lnh);
			dyllCur = lnh.yll;
			if (cl > 0)
				{
				GetPlc(vfls.hplclnh, cl - 1, &lnhT);
				dyllCur -= lnhT.yll;
				}
		/* give height to splat-only lines only at start of column */
			if (lnh.fSplatOnly && dcl-- == 1)
				{
				fLnn = fFalse; /* don't count splats as lines */
				/* make lr not have zero height */
				if (plbs->yl > plbs->ylColumn)
					dyllCur = 1;
				}
		/* this works even if plclnh is empty: rgcp[iylMac = 0] != cpLim */
			plbs->cp = CpPlc(vfls.hplclnh, clLim);
			plbs->cl = 0;
			if (plbs->cp != vfls.ca.cpLim && !vfls.fEndBreak)
				{
			/* ran out of space */
				lbc = lbcYlLim;
				}
			fEndBreak = vfls.fEndBreak;
			}

/* now we know amount of text from para that will be used in this lr */
LHaveDylCur:
	if (dyllCur >= ylLarge - plbs->yl)
		dyllCur = ylLarge - plbs->yl -1;
	Assert(dyllCur < ylLarge); /* after subtracting height of lines in
	previous columns, we should be back in the integer range */
	if (!fTableRow)
		dyllAdvance = dyllCur;

	Scribble(ispLayout3, (((int)IMacPllr(plbs) / 10) % 10) + '0');
	Scribble(ispLayout4, ((int)IMacPllr(plbs) % 10) + '0');

	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	if (lr.fSpaceBefore && lrp->cp == cpNil)
		{
		/* don't let space before at top of page change the LR */
		lr.yl += dylBefore;
		dyllCur -= dylBefore;
		}

	if (lr.lrk == lrkAbs || lr.ihdt != ihdtNil)
		lr.dyl += dyllCur;
	else  if (lr.dyl <= plbs->yl + (int)dyllAdvance - lr.yl)
		{
		/* if tSoftBottom, we can extend past end of lr */
		lr.dyl = plbs->yl + (int)dyllAdvance - lr.yl;
		if (dyllAdvance > dyllRemain)
			lbc = lbcYlLim;	/* non-apo footnotes don't set ylLim */
		}
	if (lr.lrk != lrkAbs)
		{
		lr.cpLim = plbs->cp;
		lr.clLim = plbs->cl;
		plbs->yl += dyllAdvance;
#ifdef WIN
		if (plbs->fSimpleLr)
#else
		if (plbs->fSimpleLr && lr.lrk != lrkSxs)
#endif
			{
			lr.lrs = lrsFrozen;
			/* will be off slightly for unusual borders because 
			   we don't come back and adjust lr.dyl */
			lr.dyl = plbs->yl - lr.yl;
			}
		plbs->dylUsedCol += dyllAdvance;	/* amount of linear space used */
		if (dcl > 0)
			{
			plbs->dyllTotal += dyllAdvance;
			plbs->clTotal += dcl;
			}
		}
	Assert(lr.dxl >= 0 && lr.dyl >= 0);
	bltLrp(&lr, lrp, sizeof(struct LR));
	if (fLnn)
		plbs->lnn += dcl;
	plbs->fFirstPara = fFalse;      /* at least one done */
	plbs->fPrevTable = fTableRow;   /* remember if we're in a table */

/* determine why we stopped */
LSetLbc:
	if (plbs->cp >= cpMacDoc)
		lbc = lbcEndOfDoc;
	else  if (fEndBreak)
		{
		/* terminated by chSect/chColumnBreak;
		note: short docs can't get in here */
LPageBreak:     
		Assert(!fShort);
		if (vfls.ca.cpLim == caSect.cpLim && plbs->cp == vfls.ca.cpLim)
			lbc = lbcEndOfSection;
		else  if (CColM1Sep(vsepFetch) == 0 || IfMacElse(fFalse, vfls.chBreak == chSect))
			lbc = lbcEndOfPage;
		else
			{
			Assert(vfls.chBreak == chColumnBreak);
			plbs->fNoBalance = fTrue;
			lbc = lbcEndOfColumn;
			}
		}
	else  if (!fChain && !fShort && lr.lrk != lrkAbs && plbs->cp == caSect.cpLim - ccpSect &&
		plbs->cp < CpMacDocEditPlbs(plbs))
		{
		/* special check: last line of section exactly filled space;
			especially useful when balancing */
		if (FInTableDocCp(plbs->doc, plbs->cp))
			{
			caParaL.doc = docNil;	/* make sure caParaL gets redone */
			goto LAdjustDyl;
			}
		lbc = ((plbs->cp = lr.cpLim = caSect.cpLim) >= cpMacDoc) ?
				lbcEndOfDoc : lbcEndOfSection;

		/* update cpLim for dcpDepend */
		vfls.ca.cpLim = CpMax(vfls.ca.cpLim, caSect.cpLim);
		plbs->cl = lr.clLim = 0;
		}

LAdjustDyl:
	if (!fChain && lbc != lbcNil)
		{
		/* keep advances past this lr; others need to shorten lr and 
			try to use remaining space */
		if (lr.cp != cpNil && lr.ihdt == ihdtNil)
			if (lbc == lbcYlLim)
				{
				if (fTableRow || !FKeepPap(vpapFetch))
					lr.dyl = plbs->yl - lr.yl;
				}
			else  if (lr.lrk != lrkAbs)
				lr.dyl = plbs->yl - lr.yl;
		/* can't maintain width for rectangles altered by APO */
		/* need raw cp's for accurate page breaks on screen */
		/* keeping cl for footnotes raises too many problems */
		/* Mac: because yl is int, allowing large cl can overflow 
			ClFormatLines' clFirst scan; in any case, having a big
			cl is NOT an optimization */
		if (lr.clLim != 0 && (lr.lrk != lrkNormal || 
				vlm == lmBRepag || fShort || plbs->cl >= clMaxLbs))
			{
			/* reduce cl; this is a smart version of CpFromCpCl */
			cl = lr.clLim + max(0, IInPlcCheck(vfls.hplclnh, plbs->cp));
			if (IMacPlc(vfls.hplclnh) < cl)
				ClFormatLines(plbs, vfls.ca.cpLim, ylLarge, ylLarge, cl, lr.dxl, fFalse, fTrue);
			plbs->cp = lr.cpLim = CpPlc(vfls.hplclnh, cl);
			plbs->cl = lr.clLim = 0;
			}
		if (lr.lrk == lrkAbs && plbs->cp < vfls.ca.cpLim)
			lr.cpLim = plbs->cp;
		lr.lrs = lrsFrozen;
		/* check height/width, but balancing causes restricted room
			cases all the time, and we don't care about unused lrs */
		Assert(vcBalance > 0 || lr.cp == cpNil || (lr.dxl >= 0 && lr.dyl >= 0));
		bltLrp(&lr, LrpInPl(plbs->hpllr, plbs->ilrCur), sizeof(struct LR));
		}

	/* notice that dcpDepend ignores cl */
	plbs->dcpDepend = max (0, (int)CpMin(0x7fffL, vfls.ca.cpLim-plbs->cp));
	plbs->dylOverlap = dylOverlapNew;
LExitLbcFormatPara:
	/* I certainly hope we accomplished something! */
	/* We may not have made any progress if the first line we tried
		would not fit. (lbc == lbcYlLim) */
	/* we don't advance past CpMacDocEdit */
	Debug(Assert(lbc == lbcYlLim || lbc == lbcEndOfDoc || 
			(plbs->cp > cpStart ||
			(plbs->cp == cpStart && plbs->cl > clStart))));

	ShowLbs(plbs, irtnFormPara/*irtn*/);

	StopProfile();
	return(lbc);
}


#endif /* DEBUGORNOTWIN */


#ifdef WIN
EXPORT /* AbortLayout */
#else /* !WIN */
NATIVE /* WINIGNORE - AbortLayout - EXPORT if WIN */
#endif /* !WIN */
/* %%Function:AbortLayout %%Owner:chrism */
AbortLayout()
{
#ifndef PCWORD
	struct DRF *pdrfList;

	for (pdrfList = vpdrfHead; pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
		bltbh(&pdrfList->dr, HpInPl(pdrfList->hpldr, pdrfList->idr), sizeof(struct DR));
	vpdrfHead = NULL;
	Debug(vpdrfHeadUnused = NULL);
#endif /* !PCWORD */

	vfli.fLayout = fFalse;
	if (vplbsFrame)
		CopyLbs(0, vplbsFrame);
	if (vplbsFrameHdt)
		CopyLbs(0, vplbsFrameHdt);
#ifdef WIN
	DoJmp(&venvLayout, 1);
#else
	if (vplbsFrameSxs)
		CopyLbs(0, vplbsFrameSxs);
	DoJump(&venvLayout);
#endif /* !WIN */
}


