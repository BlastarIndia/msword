/* P R I N T 1 . C */
/* Routines common to both Print and Preview */


#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOSYSMETRICS

#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "dlbenum.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "screen.h"
#include "sel.h"
#include "field.h"
#include "file.h"
#include "format.h"
#include "layout.h"
#include "print.h"
#include "style.h"
#include "border.h"
#include "debug.h"
#include "prm.h"
#include "ch.h"
#include "inter.h"
#include "prompt.h"
#include "message.h"
#define REVMARKING
#include "compare.h"
#undef REVMARKING
#include "sumstat.h"
#include "doslib.h"

#include "error.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"


#define ilrNil           (-1)
#define dxaEighth       (dxaInch/8)

extern int              vdocTemp;
extern int              vlm;
extern int              vflm;
extern int              wwCur;
extern int              vpgnFirst, vpgnLast, vsectFirst, vsectLast;
extern struct MWD       **hmwdCur;
extern struct WWD       **hwwdCur;
extern struct UAB       vuab;
extern struct SEL       selCur;
extern struct FLI       vfli;
extern struct SCI       vsci;
extern struct MERR      vmerr;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern struct PRI       vpri;
extern struct CA        caPara;
extern struct FMTSS     vfmtss;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct CA        caAdjust;
extern struct CA        caTap;
extern struct PMD       *vppmd;
extern struct FTI       vfti;
extern struct PLLR      **vhpllr;
extern FARPROC lpFPrContinue;
extern CHAR             szEmpty[];
extern struct PREF      vpref;
extern struct RC        vrcBand;
extern HWND vhwndPgPrvw;
extern HBRUSH vhbrBlack;
extern CP vcpLimLayout;
extern HWND				vhwndCBT;

struct DTTM DttmCur();
DisplayLrTable();
extern DWORD GetRgbIco();

extern struct PRSU     vprsu;
extern DSI             dsi;
#ifdef DEBUG
extern HDC             hdcPrint;
#endif


/*
*  Ensure document page size matches the printer's paper size and that
*  document margins fall within the printable area of the printer's page.
*  Returns fTrue if page and margins are cool or if they're not and joe
*  user doesn't care, fFalse otherwise.
*  Note: a match is defined as within +/- .75".
*/
/* %%Function:FCheckPageAndMargins %%Owner:davidbo */
FCheckPageAndMargins(doc, fSwapDimensions, fPrinting, pfPrintableArea)
int doc, *pfPrintableArea;
int fPrinting;
int fSwapDimensions;
{
	struct DOD *pdod;

#ifdef BRYANL
	CommSzNum( SzShared( "FCheckPageAndMargins, fSwap: "), fSwapDimensions );
#endif
	if (doc == docNil)
		return fTrue;	/* no doc, so no conflict */
	doc = DocMother(doc);
	pdod = PdodDoc(doc);
	/*
	*  Since user can't change the macro edit document, force the
	*  correct paper size...and don't even ask permission....
	*/
	if (pdod->udt == udtMacroEdit)
		{
		int xaPage, yaPage;
		xaPage = NMultDiv(vpri.dxpRealPage, czaInch, vfli.dxuInch);
		yaPage = NMultDiv(vpri.dypRealPage, czaInch, vfli.dyuInch);
		if (xaPage != pdod->dop.xaPage || yaPage != pdod->dop.yaPage)
			{
			pdod->dop.xaPage = xaPage;
			pdod->dop.yaPage = yaPage;
			pdod->fEnvDirty = fTrue;
/* FUTURE - TrashWwsForDoc (with InvalCaFierce) would seem to be enough here */
			TrashAllWws();
			}
		return fTrue;
		}

	if (FPageOK(doc, pdod->dop.xaPage, pdod->dop.yaPage, pfPrintableArea))
		goto LCheckMargins;

	if ((fSwapDimensions) && 
			FPageOK(doc, pdod->dop.yaPage, pdod->dop.xaPage, pfPrintableArea))
		{
		if (IdMessageBoxMstRgwMb(mstPageSwap, NULL,MB_YESNOQUESTION)==IDYES)
			{
			int zaT, docMotherLayout;

			docMotherLayout = DocMotherLayout(doc);
			pdod = PdodDoc(docMotherLayout);
			zaT = pdod->dop.xaPage;
			pdod->dop.xaPage = pdod->dop.yaPage;
			pdod->dop.yaPage = zaT;
			pdod->fDirty = fTrue;
			pdod->fEnvDirty = fTrue;
			InvalPageView(docMotherLayout);
/* FUTURE - TrashWwsForDoc (with InvalCaFierce) would seem to be enough here */
			TrashAllWws();
			goto LCheckMargins;	/* now it's OK */
			}
		}

	/* Don't put up unpredictable messages with CBT running */
	if (vhwndCBT)
		return fTrue;

	if (fPrinting)
		return (IdMessageBoxMstRgwMb(mstPageMismatch,NULL,MB_YESNOQUESTION)==IDYES);
	else
		{
		IdMessageBoxMstRgwMb(mstPageMismatchNP, NULL, MB_OK);
		return fTrue;
		}

LCheckMargins:
	if (FMarginsOK(doc))
		return fTrue;
	else if (fPrinting)
		return (IdMessageBoxMstRgwMb(mstBadMargins, NULL, MB_YESNOQUESTION)==IDYES);
	else
		IdMessageBoxMstRgwMb(mstBadMarginsNP, NULL, MB_OK);
	return fTrue;
}


/* %%Function:FPageOK %%Owner:davidbo */
FPageOK( docMother, dxaDocPage, dyaDocPage, pfPrintableArea )
int docMother;
int dxaDocPage, dyaDocPage;
int *pfPrintableArea;
{
	int dxaRealPage, dyaRealPage;

	dxaRealPage = NMultDiv(vpri.dxpRealPage, czaInch, vfli.dxuInch);
	dyaRealPage = NMultDiv(vpri.dypRealPage, czaInch, vfli.dyuInch);

#define dxaSlop	((czaInch * 3)/4) 
#define dyaSlop ((czaInch * 3)/4)

	/* check page size */
	Assert(docMother == DocMother(docMother));
	if (abs(dxaDocPage - dxaRealPage) > dxaSlop ||
			abs(dyaDocPage - dyaRealPage) > dyaSlop)
		{
		if (pfPrintableArea)
			*pfPrintableArea = fFalse;
		return fFalse;
		}
	return fTrue;
}


/* %%Function:FMarginsOK %%Owner:davidbo */
FMarginsOK( docMother)
int docMother;
{
	int iza, *pza, *pzaMin, rgza[4], rgzaMin[4];
	struct DOD *pdod;

	/* check margins */

	/* Determine the minimum margins of the page. */
	if (vpri.hdc != hNil)
		{
		rgzaMin[0] = vpri.ptUL.xp;
		rgzaMin[1] = vpri.ptUL.yp;
		rgzaMin[2] = max(0,vpri.dxpRealPage - vpri.ptUL.xp - vpri.dxpPrintable);
		rgzaMin[3] = max(0,vpri.dypRealPage - vpri.ptUL.yp - vpri.dypPrintable);
		for (iza = 0, pzaMin = rgzaMin; iza < 4; iza++, pzaMin++)
			{
			if (*pzaMin)
				*pzaMin = NMultDiv(*pzaMin, czaInch,
						iza & 1 ? vfli.dyuInch : vfli.dxuInch) - 40;
			}
		}
	else
		{
		SetWords(rgzaMin, 0, 4);
		}

	pdod = PdodDoc(docMother);
	rgza [0] = pdod->dop.dxaLeft + pdod->dop.dxaGutter;
	rgza [1] = abs(pdod->dop.dyaTop);
	rgza [2] = pdod->dop.dxaRight;
	rgza [3] = abs(pdod->dop.dyaBottom);

	/* Are the margins valid? */
	for (iza = 0, pza = rgza, pzaMin = rgzaMin; iza < 4; iza++, pza++, pzaMin++)
		{
		if (*pza < *pzaMin)
			return fFalse;
		}

	/* if FacingPages, need to check margins for even numbered pages */
	if (FFacingPages(docMother))
		{
		if ((rgzaMin [0] > (pdod->dop.fMirrorMargins ?
				pdod->dop.dxaRight : pdod->dop.dxaLeft)) ||
				(rgzaMin [1] > pdod->dop.dxaGutter + (pdod->dop.fMirrorMargins ? 
				pdod->dop.dxaLeft : pdod->dop.dxaRight)))
			{
			return fFalse;
			}
		}

	return fTrue;
}


/* *****  Vertical Alignment/Line Between Column stuff ***** */

/* S e t  S e c t  L n b c  H e i g h t */
/* %%Function:SetSectLnbcHeight %%Owner:davidbo */
SetSectLnbcHeight(hpllnbc, dypSect, ilnbcSectStart, ilnbcSectMac)
struct PLLNBC **hpllnbc;
int dypSect, ilnbcSectStart, ilnbcSectMac;
{
	int ilnbc, ilnbcMac;
	struct LNBC *plnbc;

	ilnbcMac = (*hpllnbc)->ilnbcMac;
	Assert(vlm == lmPrint || vlm == lmPreview);
	Assert(ilnbcSectStart <= ilnbcMac);
	Assert(ilnbcSectMac <= ilnbcMac);

	if (ilnbcMac == 0 || ilnbcSectStart == ilnbcSectMac)
		return;
	plnbc = PInPl(hpllnbc, ilnbcSectStart);
	for (ilnbc = ilnbcSectStart; ilnbc < ilnbcSectMac; ilnbc++, plnbc++)
		plnbc->dyl = dypSect;
}


/* F  A l l o c  A l g */
/*
*  Allocates space for one ALG and its ALCs.  Note, cwAlg includes the
*  space for one ALC, which is why we only need to allocate extra space
*  for ccolM1 extra ALCs.
*  Returns fTrue if allocation succeeded, fFalse otherwise.
*/
/* %%Function:FAllocAlg %%Owner:davidbo */
FAllocAlg(hgrpalg, ccolM1, pbalgMac, pbalgMax)
struct ALG **hgrpalg;
int ccolM1, *pbalgMac, *pbalgMax;
{
	int cw = cwALG + cwALC * ccolM1;

	Assert(vlm == lmPrint || vlm == lmPreview);
	if (*pbalgMac + cw > *pbalgMax)
		{
		if (!FChngSizeHCw(hgrpalg, *pbalgMac + cw, fFalse))
			return fFalse;
		*pbalgMax = *pbalgMac + cw;
		}
	*pbalgMac += cw;
	return fTrue;
}


/* P a l g  F r o m  H g r p a l g  B a l g */
/*
*  Returns a pointer to an ALG given a grpalg and an offset.
*/
/* %%Function:PalgFromHgrpalgBalg %%Owner:davidbo */
struct ALG *PalgFromHgrpalgBalg(hgrpalg, balg)
struct ALG **hgrpalg;
int balg;
{
	return (struct ALG *) (((int *)*hgrpalg) + balg);
}


/* P a l g  N e x t  F r o m  H g r p a l g  P b a l g */
/*
*  Returns a pointer to the ALG after the ALG derived from
*  hgrpalg and pbalg.  Pbalg is updated to be the offset of
*  the new ALG.
*/
/* %%Function:PalgNextFromHgrpalgPbalg %%Owner:davidbo */
struct ALG *PalgNextFromHgrpalgPbalg(hgrpalg, pbalg)
struct ALG **hgrpalg;
int *pbalg;
{
	struct ALG *palg = (struct ALG *) (((int *)*hgrpalg) + *pbalg);

	*pbalg += (cwALG + (palg->calc - 1) * cwALC);
	return (struct ALG *) (((int *)*hgrpalg) + *pbalg);
}


/* A l i g n  L r s */
/*
*  Apply the vertical alignment information in hgrpalg to the LRs in
*  hpllr.  Note, vjcCenter is only done if all ALGs are vjcCenter,
*  otherwise it is interpreted as vjcTop.
*/
/* %%Function:AlignLrs %%Owner:davidbo */
AlignLrs(plbs, hgrpalg, ialgMac, clrAdjust)
struct LBS *plbs;
struct ALG **hgrpalg;
int ialgMac, clrAdjust;
{
	struct PLLR **hpllr;
	struct ALG *palg;
	struct ALC *palc;
	int ialg, balg, ialc, clrM1;
	int dyp, dypAdjust, dypSpecAdjust;
	int ilr, ilrMac;
	LRP lrp;
	LRP lrpFirst;
	LRP lrpLast;
	/* Only used if all columns of all ALGs are vjcCenter */
	int fCenter = fTrue;
	int dypUsed = 0;
	int dypAvail = 0;
	int ilnbc, ilnbcMac;
	struct LNBC *plnbc;
	struct PLLNBC **hpllnbc;

#ifdef DAVIDBO
	CommSzNum("\n\rcalg = ", ialgMac);
#endif
	Assert(vlm == lmPrint || vlm == lmPreview);
	hpllr = plbs->hpllr;
	balg = 0;
	palg = PalgFromHgrpalgBalg(hgrpalg, balg);
	for (ialg = 0; ialg < ialgMac; ialg++)
		{
		if (palg->dylMac > palg->dylAvail)
			continue;
		switch (palg->vjc)
			{
		case vjcTop:
#ifdef DAVIDBO
			CommSz("Top\n\r");
#endif
			/* This case is done by default by the normal layout stuff */
			fCenter &= fFalse;
			break;

		case vjcCenter:
#ifdef DAVIDBO
			CommSz("Center\n\r");
			CommSzNum("\tdylMac = ", palg->dylMac);
			CommSzNum("\tdylAvail = ", palg->dylAvail);
#endif
			fCenter &= fTrue;
			dypUsed += ((ialg == ialgMac - 1) ? palg->dylMac : palg->dylAvail);
			dypAvail += palg->dylAvail;
			break;

		case vjcBoth:
			fCenter &= fFalse;
			palc = palg->rgalc;
#ifdef DAVIDBO
			CommSz("Justify\n\r");
			CommSzNum("\tcalc = ", palg->calc);
			CommSzNum("\tdylAvail = ", palg->dylAvail);
#endif
			for (ialc = 0; ialc < palg->calc; ialc++, palc++)
				{
				/*
					*  do not justify columns that are
					*   1.  end of section OR
					*   2.  end of document OR
					*   2.  have only one lr
					*/
#ifdef DAVIDBO
				CommSzNum("\n\r\t\tlbc = ", palc->lbc);
#endif
				if (palc->lbc == lbcEndOfSection || palc->lbc == lbcEndOfDoc || palc->clr == 1)
					continue;

				Assert(palc->clr > 1);

				/*
					*  DypAdjust is the amount of space to add between each
					*  LR.  DypSpecAdjust is the remainder after the dypAdjust
					*  calculation.  Each LR, after the first one of course,
					*  has its yp incremented by dypAdjust.  An additional
					*  1 yp is added as long as dypSpecAdjust is non-zero.
					*  Naturally, dypSpecAdjust is decremented after each LR.
					*/
				clrM1 = palc->clr - 1;
				lrpFirst = LrpInPl(hpllr, palc->ilrFirst + clrAdjust);
				lrpLast = LrpInPl(hpllr, palc->ilrFirst + clrM1 + clrAdjust);
				dyp = palg->dylAvail - (lrpLast->yl + lrpLast->dyl - lrpFirst->yl);
				dypAdjust = dyp / clrM1;
				dypSpecAdjust = dyp % clrM1;
				ilrMac = palc->ilrFirst + palc->clr + clrAdjust;
				Assert(ilrMac <= (*hpllr)->ilrMac);
#ifdef DAVIDBO
				CommSzNum("\t\tdypCol = ", lrpLast->yl + lrpLast->dyl - lrpFirst->yl);
				CommSzNum("\t\tdyp = ", dypAdjust);
				CommSzNum("\t\tdypSpec = ", dypSpecAdjust);
				CommSzNum("\t\tclr = ", palc->clr);
				CommSzNum("\t\tilrFirst = ", palc->ilrFirst + clrAdjust);
				CommSzNum("\t\tilrMac = ", ilrMac);
#endif
				lrp = LrpInPl(hpllr, palc->ilrFirst + 1 + clrAdjust);
				dyp = 0;
				for (ilr = 0; ilr < clrM1; ilr++, lrp++)
					{
					dyp += (dypAdjust + ((dypSpecAdjust != 0) ? 1 : 0));
					lrp->yl += dyp;
					if (dypSpecAdjust != 0)
						dypSpecAdjust--;
					}
				}
			}

		palg = PalgNextFromHgrpalgPbalg(hgrpalg, &balg);
		}

	/* If all ALGs are vjcCenter, center them as a unit. */
	if (fCenter)
		{
		dypAdjust = (dypAvail - dypUsed) >> 1;
#ifdef DAVIDBO
		CommSz("Centering\n\r");
		CommSzNum("\tdyp = ", dypAdjust);
#endif
		ilrMac = (*hpllr)->ilrMac;
		if (ilrMac == 0)
			return;
		lrp = LrpInPl(hpllr, 0);
		for (ilr = 0; ilr < ilrMac; ilr++, lrp++)
			{
			/* skip hdr/ftr, and non-normal lrs */
			if (lrp->lrk != lrkNormal || (lrp->ihdt != ihdtNil && lrp->ihdt < ihdtMaxSep))
				continue;
			lrp->yl += dypAdjust;
			}
		if ((hpllnbc = plbs->hpllnbc) != NULL)
			{
			ilnbcMac = (*hpllnbc)->ilnbcMac;
			if (ilnbcMac == 0)
				return;
			plnbc = PInPl(hpllnbc, 0);
			for (ilnbc = 0; ilnbc < ilnbcMac; ilnbc++, plnbc++)
				plnbc->yl += dypAdjust;
			}
		}
}




/* E N D  U  L  P R I N T */
/* draw underlines from a point described in uls to the current pen position
(which is saved and restored.) Pen is at the baseline.

		kulSingle: no need to do anything, was handled by LoadFont
		kulDouble: first underline was handled by Loadfont;
					next one is done by putting out underlined spaces
Line positions relative to the baseline can be adjusted herein.
*/

/* %%Function:EndULPrint %%Owner:davidbo */
EndULPrint(puls)
struct ULS *puls;
{
	int kul = puls->kul;
	HDC hdc = puls->hdc;
	int xpFirst = puls->pt.xp;
	int xpLim = puls->xwLim;
	int ypBase = puls->pt.yp;
	int ypSingle;
	int ypDouble;
	int xwMin;

	ypSingle = ypBase;
	ypDouble = min( ypBase + 1 + NMultDiv( czaPoint, vfti.dxpInch, czaInch ),
			ypBase + puls->dypPen + vfli.dypBase - 1 );

	switch (puls->kul)
		{
#ifdef DEBUG
	default:            
		Assert( fFalse );
	case kulNone:       
		break;
#endif  /* DEBUG */
	case kulDotted:	/* does not print */
	case kulSingle:	/* handled by underlined font request */
	case kulWord:
		break;
	case kulDouble:
		PrintXLine( hdc, xpFirst, ypDouble, xpLim );
		break;
		}
}


/* %%Function:ClipLnbcToApo %%Owner:davidbo */
ClipLnbcToApo(plbs)
struct LBS *plbs;
{
	int dxl, dyl, ylLnbcBottom, ylLrBottom;
	int ilr, ilrMac, ilnbc, ilnbcMac, fRegenLrp;
	LRP lrp;
	struct LNBC *plnbc, lnbc;
	struct LR lr;

	if (plbs->hpllnbc == hNil || plbs->hpllr == hNil)
		return;
	ilnbcMac = (*plbs->hpllnbc)->ilnbcMac;
	if (ilnbcMac == 0)
		return;
	ilrMac = IMacPllr(plbs);
	lrp = LrpInPl(plbs->hpllr, 0);
	fRegenLrp = fFalse;
	for (ilr = 0; ilr < ilrMac; ilr++, lrp++)
		{
		if (fRegenLrp)
			{
			lrp = LrpInPl(plbs->hpllr, ilr);
			fRegenLrp = fFalse;
			}
		if (lrp->lrk != lrkAbs)
			continue;
		CachePara(lrp->doc, lrp->cp);
		dxl = NMultDiv(vpapFetch.dxaFromText, vfli.dxuInch, czaInch);
		dyl = NMultDiv(vpapFetch.dxaFromText, vfli.dyuInch, czaInch);
		bltLrp(lrp, &lr, sizeof(struct LR));
		lr.xl -= dxl;
		lr.yl -= dyl;
		lr.dxl += 2*dxl;
		lr.dyl += 2*dyl;
		ylLrBottom = lr.yl + lr.dyl;
		for (ilnbc = 0; ilnbc < ilnbcMac; ilnbc++)
			{
			/* Regen plnbc each time (instead of simple increment) because
				of possible deletion of plex element */
			plnbc = PInPl(plbs->hpllnbc, ilnbc);
			ylLnbcBottom = plnbc->yl + plnbc->dyl;
			/* filter out simple cases of no intersection */
			if (plnbc->yl >= ylLrBottom || ylLnbcBottom <= lr.yl ||
					plnbc->xl <= lr.xl || plnbc->xl > lr.xl+lr.dxl)
				continue;
			if (plnbc->yl >= lr.yl)
				{
				if (ylLnbcBottom <= ylLrBottom)
					{
					/* line completely inside APO, nuke it */
					DeleteFromPl(plbs->hpllnbc, ilnbc);
					ilnbcMac--;
					ilnbc--;	/* will be incremented by loop */
					fRegenLrp = fTrue;
					}
				else
					{
					/* line starts inside APO, clip to bottom of APO */
					plnbc->dyl = ylLnbcBottom - ylLrBottom;
					plnbc->yl = ylLrBottom;
					}
				}
			else  if (ylLnbcBottom <= ylLrBottom)
				{
				/* line ends inside APO, clip to top of APO */
				plnbc->dyl = lr.yl - plnbc->yl;
				}
			else
				{
				/* line starts above and ends after APO, split line and add
					new line at end of list */
				lnbc = *plnbc;
				/* top portion clipped to top of APO */
				plnbc->dyl = lr.yl - plnbc->yl;
				/* bottom portion clipped to bottom of APO */
				lnbc.dyl = ylLnbcBottom - ylLrBottom;
				lnbc.yl = ylLrBottom;
				if (FInsertInPl(plbs->hpllnbc, ilnbcMac, &lnbc))
					ilnbcMac++;
				fRegenLrp = fTrue;

				}
			}
		}
}


/* D R A W  F O R M U L A  L I N E */
/* Draw/Print a line for the formula invoking graphics */
/* %%Function:DrawFormulaLine %%Owner:davidbo */
EXPORT DrawFormulaLine(hdc, x2, y2, x1, y1, grpf)
HDC hdc;
int x1, y1, x2, y2;
int grpf;
{
	static HPEN	hpen;
	static HANDLE  hOld;
	static BOOL    fStock;

	Assert(hdc);

	if (grpf & 8 /* fCreate */)
		{
		DWORD rgb;
		int dzpPen;

		dzpPen = (grpf & 1) /* fPrint */ ? vfti.dxpBorder : vsci.dxpBorder;
		rgb = GetTextColor(hdc); /* assumes ForeColor() done */
		
		if ((hpen = CreatePen(0 /* solid */, dzpPen, rgb)) == NULL)
			{
			hpen = GetStockObject(BLACK_PEN);
			fStock = fTrue;
			}
		else
			fStock = fFalse;
		LogGdiHandle(hpen, 1078);
		hOld = SelectObject(hdc, hpen);
		}

	if (grpf & 4 /* fMove */)
		MoveTo(hdc, x1, y1);

	LineTo(hdc, x2, y2);

	if ((grpf & 2 /* fDestroy */) && !fStock)
		{
		Assert(hOld != NULL);
		SelectObject(hdc, hOld);
		UnlogGdiHandle(hpen, 1078);
		DeleteObject(hpen);
		}
}


