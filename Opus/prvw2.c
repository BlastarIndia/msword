/* split from preview 5/24/88 davidbo */

#define NOTBITMAP
#define NOCLIPBOARD
#define NOCREATESTRUCT
#define NOMEMMGR
#define NOOPENFILE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOWH
#define NOGDICAPMASKS
#define NOICON
#define NODRAWTEXT
#define NOMB
#define NOWNDCLASS

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "format.h"
#include "layout.h"
#include "border.h"
#include "file.h"
#include "heap.h"
#include "ch.h"
#include "splitter.h"
#include "ibdefs.h"
#include "inter.h"
#include "prm.h"
#include "sel.h"
#include "preview.h"
#include "print.h"
#include "screen.h"
#include "opuscmd.h"
#include "keys.h"
#include "message.h"
#include "debug.h"
#define REVMARKING
#include "compare.h"
#include "resource.h"

#include "help.h"
#include "rerr.h"
#include "error.h"
#include "cmdtbl.h"
#include "recorder.h"


/* Externals */
extern int              wwCur;
extern int              vflm;
extern int              vlm;
extern int              vfPrvwDisp;
extern int              vfRecording;
extern uns              mputczaUt[];
extern PVS              vpvs;
extern char             *mputsz[];
extern CHAR		*rgchEop[];
extern HWND             vhwndCBT;
extern HBRUSH           vhbrGray, vhbrLtGray, vhbrDkGray, vhbrWhite;
extern HCURSOR          vhcArrow, vhcPrvwCross;
extern BOOL		vfElFunc;
extern int		vcElParams;
extern int              fElActive;
extern struct SEL       selCur;
extern struct CA        caSect;
extern struct CA        caHdt;
extern struct CA	caTap;
extern struct PREF      vpref;
extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct PRI       vpri;
extern struct SCI       vsci;
extern struct ITR       vitr;
extern struct MERR      vmerr;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct FMTSS     vfmtss;
extern struct ESPRM     dnsprm[];

/* externals defined in preview */
extern HWND vhwndPgPrvw;
extern HWND vhwndPgPrvwScrl;
extern HWND vhwndPrvwIconBar;
extern uns **HCopyHeapBlock();
extern DWORD GetRgbIco();

/* MUST match declaration in preview.c */
csconst CHAR szCsPrvwOnePg[] = SzKey("One P&age",PrvwOnePg);
csconst CHAR szCsPrvwTwoPg[] = SzKey("Two P&ages",PrvwTwoPg);

LONG lParamPrvwDrag = (LONG) -1;

/* %%Function:GetMargin %%Owner:davidbo */
GetMargin(ircs, prcHandle, prcLine)
short ircs;
struct RC *prcHandle, *prcLine;
{
	/* return the line and handle for the specified margin */
	struct RC rcPage;

	rcPage = (vpvs.tShowAreas == tAreasLeft) ? vpvs.rcPage0 : vpvs.rcPage1;

	if (FHorzIrcs(ircs))
		{
		/* top/bottom - horizontal lines */
		prcLine->xpLeft = rcPage.xpLeft + 1;
		prcLine->xpRight = rcPage.xpRight - 1;
		prcLine->ypTop = ZpFromRcIrcs(vpvs.rcMargin, ircs);
		prcLine->ypBottom = prcLine->ypTop + 1;
		/* right page handles usually go on the right of the page, but
			we make exceptions for one odd facing page */
		if (vpvs.tShowAreas == tAreasRight ||
				(!vpref.fPrvwTwo && vpvs.fFacing && (vfmtss.pgn&1)))
			{
			prcLine->xpRight -= dzpHandle + 1;
			prcHandle->xpRight = rcPage.xpRight;
			prcHandle->xpLeft = prcHandle->xpRight - dzpHandle;
			}
		else
			{
			prcLine->xpLeft += dzpHandle + 1;
			prcHandle->xpLeft = rcPage.xpLeft + 1;
			prcHandle->xpRight = prcHandle->xpLeft + dzpHandle;
			}
		prcHandle->ypTop = prcLine->ypTop - (dzpHandle >> 1);
		prcHandle->ypBottom = prcHandle->ypTop + dzpHandle;
		}
	else
		{
		/* left/right - vertical lines */
		prcLine->ypTop = rcPage.ypTop + 1;
		prcLine->ypBottom = rcPage.ypBottom - dzpHandle - 2;
		prcLine->xpLeft = ZpFromRcIrcs(vpvs.rcMargin, ircs);
		prcLine->xpRight = prcLine->xpLeft + 1;
		prcHandle->ypBottom = rcPage.ypBottom;
		prcHandle->ypTop = prcHandle->ypBottom - dzpHandle;
		prcHandle->xpLeft = prcLine->xpLeft - (dzpHandle >> 1);
		prcHandle->xpRight = prcHandle->xpLeft + dzpHandle;
		}
}


/* %%Function:ChangeMargins %%Owner:davidbo */
ChangeMargins(hdc, pt, ircs)
HDC hdc;
struct PT pt;
int ircs;
{
/* change the document's margins */
	int fEven = FFacingEven(vpvs.tShowAreas == tAreasLeft);
	int fSwapLeftRight = fFalse;
	struct DOP *pdop;
	struct DOP dopT;
	int xa, ya, za;
	struct PT ptT;
	struct RC rc;
	struct CA caT;

	Assert(hdc);
	/* check for dragging off page */
	rc = vpvs.tShowAreas == tAreasLeft ? vpvs.rcPage0 : vpvs.rcPage1;
	ptT = pt;
	if (fEven && ircs == ircsRight)
		{
		rc.xpRight -= vpvs.dxpGutter;
		ptT.xp += vpvs.dxpGutter;
		}
	else  if (!fEven && ircs == ircsLeft)
		{
		rc.xpLeft += vpvs.dxpGutter;
		ptT.xp -= vpvs.dxpGutter;
		}
	if (!FPtInRc(&rc, pt))
		return;
	AssertDo(FXaYaFromPt(ptT, &xa, &ya, (ircs==ircsLeft || ircs==ircsBottom) ?
			bitBottomLeft : bitTopRight));
	FreezeHp();
	pdop = PdopDoc(vpvs.docPrvw);
	dopT = *pdop;
	switch (ircs)
		{
	case ircsLeft:
		/* if fMirrorMargins, the "left" margin on an even page is really the
			"outside" margin, which is stored in the dxaRight slot of the dop. */
		if (fEven && dopT.fMirrorMargins)
			{
			za = dopT.dxaRight = xa;
			fSwapLeftRight = fTrue;
			}
		else
			za = dopT.dxaLeft = xa;
		break;
	case ircsTop:
		za = dopT.dyaTop = (dopT.dyaTop >= 0) ? ya : -ya;
		break;
	case ircsRight:
		/* if fMirrorMargins, the "right" margin on an even page is really the
			"inside" margin, which is stored in the dxaLeft slot of the dop. */
		if (fEven && dopT.fMirrorMargins)
			{
			za = dopT.dxaLeft = xa;
			fSwapLeftRight = fTrue;
			}
		else
			za = dopT.dxaRight = xa;
		break;
	case ircsBottom:
		za = dopT.dyaBottom = (dopT.dyaBottom >= 0) ? ya : -ya;
		break;
	default:
		Assert(fFalse);
		break;
		}
	if (!FNeRgw(&dopT.dyaTop, &pdop->dyaTop, 4))
		{
		MeltHp();
		return; /* no change */
		}
	MeltHp();
	ShowAreas(hdc, fFalse);  /* erase */
	*PdopDoc(vpvs.docPrvw) = dopT;
	ChangePrvwCancel();
	DirtyDoc(vpvs.docPrvw);

	/* invalidate: for left/right margins, invalidate all cps and mark
		margins changed; for top/bottom, simply invalidate the plcpgd */
	if (FHorzIrcs(ircs))
		{
		SetPlcUnk(PdodDoc(vpvs.docPrvw)->hplcpgd, cp0, CpMacDoc(vpvs.docPrvw));
		*((int *) &vpvs.rcMargin + ircs) = pt.yp;
		}
	else
		{
		PdodDoc(vpvs.docPrvw)->fLRDirty = fTrue;
		InvalCp(PcaSet(&caT, vpvs.docPrvw, cp0, CpMacDoc(vpvs.docPrvw)));

		/* left/right -- change hdr/ftr rectangles also */
		*((int *) &vpvs.rcMargin + ircs) = pt.xp;
		pt.xp += (ircs == ircsLeft) ? xpClose : -xpClose;
		if (vpvs.rcHdr.xpLeft > 0)
			*((int *) &vpvs.rcHdr + ircs) = pt.xp;
		if (vpvs.rcFtr.xpLeft > 0)
			*((int *) &vpvs.rcFtr + ircs) = pt.xp;
		}
	vpvs.fModeMargin = fTrue;
	ShowAreas(hdc, fFalse);  /* draw at new position */

	if (vfRecording)
		{
		if (fSwapLeftRight)
			ircs = IrcsOppIrcs(ircs); /* map ircsLeft <-> ircsRight */
		RecordMargin(ircs, za);
		}
}


/* %%Function:FChangePageBreaks %%Owner:davidbo */
FChangePageBreak(hdc, pt, prc)
HDC hdc;
struct PT pt;
struct RC *prc;
{
	/* change a page break */
	int ilrHit, ilrCol, yl, xl, ya, xa, flmSave;
	char ch;
	struct PLLR **hpllr;
	struct LBS *plbs;
	CP cpBreak, cpInsert;
	struct CA caT;
	struct CHP chp;
	struct PAP pap;
	struct LR lr;

	Assert(hdc);
	if (abs(pt.yp - vpvs.ypPageBreak) < 3)
		return fTrue;

	ya = NMultDiv(pt.yp - prc->ypTop, vpvs.dyaPage, prc->ypBottom - prc->ypTop);
	yl = NMultDiv(ya, vfli.dyuInch, czaInch);
	xa = NMultDiv(pt.xp - prc->xpLeft, vpvs.dxaPage, prc->xpRight -prc->xpLeft);
	xl = NMultDiv(xa, vfli.dxuInch, czaInch);
	ShowAreas(hdc, fFalse);  /* erase */

/* find the LR containing the mouse drop point */
	plbs = FLeftLbs() ? &vpvs.lbsLeft : &vpvs.lbsText;
	hpllr = plbs->hpllr;
	ilrHit = IlrFromYlXl(hpllr, yl, xl, &ilrCol);

	/* mouse is below all text in column */
	if (ilrHit < 0)
		{
		/* disappearing page break */
		if (ilrCol < 0)
			{
LNoChange:
			ShowAreas(hdc, fFalse);  /* redraw */
			return fTrue;
			}
		bltLrp(LrpInPl(hpllr, ilrCol), &lr, sizeof(struct LR));
		RawLrCps(plbs, &lr, wwLayout);
		cpBreak = lr.cpLim - 1;
		ch = ChFetch(vpvs.docPrvw, cpBreak, fcmChars);
		if (ch == chColumnBreak)
			goto LDelete;
		if (ch != chSect)
			goto LNoChange; /* natural break */
		CacheSect(lr.doc, cpBreak);
		if (cpBreak + 1 == caSect.cpLim)
			goto LNoChange; /* can't move section break */
LDelete:
		/* FUTURE davidbo: (dsb) revision marking? */
		FDelete(PcaSetDcp(&caT, vpvs.docPrvw, cpBreak, (CP) 1));
		if (pt.yp >= vpvs.rcMargin.ypBottom)
			goto LPageBreakChanged; /* removed entirely */

		if (!FRegenLrs())
			return fFalse;

		/* not enough data to fill to desired line */
		hpllr = FLeftLbs() ? vpvs.lbsLeft.hpllr : vpvs.lbsText.hpllr;
		if ((ilrHit = IlrFromYlXl(hpllr, yl, xl, &ilrCol)) < 0)
			goto LPageBreakChanged;
		}

/* find the line in the LR at the mouse drop point */
	bltLrp(LrpInPl(hpllr, ilrHit), &lr, sizeof(struct LR));
	if (lr.lrk == lrkAbs || lr.yl >= yl)
		goto LNoChange;
	CachePara(lr.doc, lr.cp);
	RawLrCps(plbs, &lr, wwLayout);
	lr.cpLim = CpMin(lr.cpLim, CpMacDocEdit(lr.doc));
	ch = chSect;
	flmSave = vflm;
	FResetWwLayout(vpvs.docPrvw, flmRepaginate, lmPreview);
	for (; lr.cp < lr.cpLim; lr.cp = vfli.cpMac)
		{
		FormatLineDxa(wwLayout, lr.doc, lr.cp,
				NMultDiv(lr.dxl, dxaInch, vfli.dxuInch));
		if ((vfli.fSplatBreak && vfli.ichMac == 1) ||
				(vfli.cpMac >= CpMacDocEdit(lr.doc)))
			goto LNoChange;
		if ((lr.yl += vfli.dypLine) >= yl)
			{
			cpInsert = vfli.cpMac;
			CachePara(lr.doc, cpInsert);
			chp = vchpStc;
			if (FInTableVPapFetch(lr.doc, cpInsert))
				{
				/* insert para mark in front of break */
				CpFirstTap(lr.doc, cpInsert);
				cpInsert = caTap.cpFirst;
				MapStc(PdodDoc(lr.doc), stcNormal, 0, &pap);
				if (!FInsertRgch(lr.doc, cpInsert, rgchEop, cchEop, &chp, &pap))
					goto LNoInsert;
				InvalTableProps(lr.doc, cpInsert, ccpSect+ccpEop, fTrue);
				InvalPageView(lr.doc);
				}
			if (FInCa(lr.doc, cpInsert, &selCur.ca))
				/* ensure correct selection */
				SelectIns(&selCur, selCur.cpFirst);
			if (!FInsertRgch(lr.doc, cpInsert, &ch, 1, &chp, 0))
				{
LNoInsert:
				FResetWwLayout(vpvs.docPrvw, flmSave, lmPreview);
				return fFalse;
				}
			break;
			}
		if (vfli.fSplatBreak)
			goto LNoChange;
		}
	FResetWwLayout(vpvs.docPrvw, flmSave, lmPreview);

LPageBreakChanged:
	selCur.fUpdatePap = fTrue;	/* in case there are tables around */
	vpvs.fModeMargin = fTrue;  /* force redraw */
	if (!FSetModeNorm(hdc, fTrue))
		return fFalse;
	ChangePrvwCancel();
	DirtyDoc(vpvs.docPrvw);
	return fTrue;
}


/* %%Function:IlrFromYlXl %%Owner:davidbo */
int IlrFromYlXl(hpllr, yl, xl, pilrCol)
struct PLLR **hpllr;
int yl, xl, *pilrCol;
{
	/* find the LR containing the mouse drop point -
		only main doc text can contain the new page break,
		not headers, nor footers, nor footnotes.
	*/
	int ilr, ilrMac = (*hpllr)->ilrMac;
	struct LR lr;

	*pilrCol = -1;
	for (ilr = 0; ilr < ilrMac; ilr++)
		{
		bltLrp(LrpInPl(hpllr, ilr), &lr, sizeof(struct LR));
		if (lr.doc == vpvs.docPrvw && lr.lrk != lrkAbs && 
				(lr.cp != lr.cpLim || lr.clFirst != lr.clLim))
			{
			CacheSect(lr.doc, lr.cp);
			if (vsepFetch.ccolM1 == 0 || lr.xl < xl && lr.xl + lr.dxl > xl)
				{
				*pilrCol = ilr;   /* mouse in column */
				if (lr.yl < yl && lr.yl + lr.dyl >= yl)
					return(ilr);
				}
			}
		}
	return(-1);
}


/* %%Function:FFacingEven %%Owner:davidbo */
FFacingEven(fLeft)
int fLeft;
{
	/* returns true when facing pages is on and the "current" page is even */
	return(vpvs.fFacing &&
			(vpref.fPrvwTwo && fLeft || !vpref.fPrvwTwo && !(vfmtss.pgn & 1)));
}


/* %%Function:FChangePrvwDoc %%Owner:davidbo */
FChangePrvwDoc(hdc, fNuke, fNewRcs)
HDC hdc;
int fNuke, fNewRcs;
{
	/*
	because of recent changes to the doc, force recalculation at the same
	CP which the current page displays.
	*/
	int tShowAreas = vpvs.tShowAreas;
	int yp = YpFromCp(CpPlc(PdodDoc(vpvs.docPrvw)->hplcpgd, vpvs.ipgdPrvw));

	Assert(hdc);
	caHdt.doc = caSect.doc = docNil;
	vpvs.tShowAreas = tAreasOff;
	vpvs.fAreasKnown = fFalse;
	vpvs.fLeftEmpty = vpvs.fRightEmpty = fTrue;
	vpvs.fFacing = FFacingPages(vpvs.docPrvw);
	vpvs.ipgdPrvw = ipgdLost;
	vpvs.cpMacPrvw = cp0;
	ChangePrvwCancel();
	if (fNewRcs)
		{
		SetPageRcs();
		SendMessage(vhwndPgPrvw, WM_ERASEBKGND, hdc, 0L);
		BlankPageRect(hdc, &vpvs.rcPage0);
		if (vpref.fPrvwTwo)
			BlankPageRect(hdc, &vpvs.rcPage1);
		}
	if (fNuke)
		{
		struct CA ca;

		PdodDoc(vpvs.docPrvw)->fLRDirty = fTrue;
		InvalCp(PcaSet(&ca, vpvs.docPrvw, cp0, CpMacDoc(vpvs.docPrvw)));
		}
	if (!FGotoPagePrvw(hdc, yp, dipgdRandom))
		return fFalse;
	vpvs.tShowAreas = tShowAreas;
	ShowAreas(hdc, fTrue);
	return fTrue;
}


/* %%Function:SetHdrFtr %%Owner:davidbo */
SetHdrFtr(hdc, pt, fHdr, fFree)
HDC hdc;
struct PT pt;
int fHdr;
int fFree;      /* true if free-floating header/footer */
{
	/* change the header/footer position */
	int ipgd;
	int xa, ya;
	struct CA ca;
	char rgb[3];

	Assert(hdc);
	ipgd = vpvs.ipgdPrvw + ((vpvs.tShowAreas == tAreasLeft || vpvs.fLeftEmpty || vpvs.fFirstBlank) ? 0 : 1);
	CacheSectSedMac(vpvs.docPrvw, CpMin(CpMacDocEdit(vpvs.docPrvw), CpPlc(PdodDoc(vpvs.docPrvw)->hplcpgd, ipgd)));
	AssertDo(FXaYaFromPt(pt, &xa, &ya, (fHdr) ? bitTopRight : bitBottomLeft));
	Assert(dnsprm[sprmSDyaHdrTop].cch == 3 &&
			dnsprm[sprmSDyaHdrBottom].cch == 3 &&
			sizeof(rgb) >= 3);
	if (fHdr)
		{
		if (vsepFetch.dyaHdrTop == ya)
			return;  /* no change */
		rgb[0] = sprmSDyaHdrTop;
		}
	else
		{
		if (vsepFetch.dyaHdrBottom == ya)
			return;  /* no change */
		rgb[0] = sprmSDyaHdrBottom;
		}

	if (fFree)
		{
		/* free floating signified by negative margin */
		struct DOP *pdop = PdopDoc(vpvs.docPrvw);
		FreezeHp();
		if (fHdr)
			pdop->dyaTop = -abs(pdop->dyaTop);
		else
			pdop->dyaBottom = -abs(pdop->dyaBottom);
		MeltHp();
		}

	ca = caSect;
	/* DcpCa(caSect) can be < ccpEop! */
	ca.cpFirst = CpMax(caSect.cpFirst, ca.cpLim - ccpEop);
	caSect.doc = docNil;

	/* effect the change */
	bltb(&ya, &rgb[1], sizeof(int));
	ApplyGrpprlCa(rgb, 3, &ca);
	DirtyDoc(vpvs.docPrvw);

	/* move rectangle */
	ChangePrvwCancel();
	ShowAreas(hdc, fFalse);    /* erase */
	if (fHdr)
		OffsetRect((LPRECT) &vpvs.rcHdr, 0, pt.yp - vpvs.rcHdr.ypTop);
	else
		OffsetRect((LPRECT) &vpvs.rcFtr, 0, pt.yp - vpvs.rcFtr.ypBottom - 1);
	vpvs.fModeMargin = fTrue;
	ShowAreas(hdc, fFalse);    /* redraw */
}


/* %%Function:SetAbsPos %%Owner:davidbo */
SetAbsPos(hdc, iaor, prcNew, prcPage)
HDC hdc;
int iaor;
struct RC *prcNew, *prcPage;
{
	/* change the position of an absolute object */
	int dxp, dyp, dxa, dya, cch;
	int pcVert, pcHorz;
	struct PLLR **hpllr;
	char rgb[8];
	struct AOR aor;
	struct LR lr;

/* determine new positions */
	Assert(hdc);
	blt(PInPl(vpvs.hplaor, iaor), &aor, cwAOR);
	hpllr = FLeftLbs() ? vpvs.lbsLeft.hpllr : vpvs.lbsText.hpllr;
	bltLrp(LrpInPl(hpllr, aor.ilr), &lr, sizeof(struct LR));
	CachePara(lr.doc, lr.cp);
	pcHorz = vpapFetch.pcHorz;
	pcVert = vpapFetch.pcVert;
	dxp = abs(prcNew->xpLeft - aor.rc.xpLeft);
	dyp = abs(prcNew->ypTop - aor.rc.ypTop);
	if (dxp < 3 && dyp < 3)
		return;

	dxa = dya = tmvalNinch;
	if (dxp > xpClose)
		{
		int dxpPage0 = vpvs.rcPage0.xpRight - vpvs.rcPage0.xpLeft;
		pcHorz = pcHMargin;
		if (abs(prcNew->xpLeft - vpvs.rcMargin.xpLeft) < xpClose)
			dxa = dxaAbsLeft;
		else  if (abs(prcNew->xpRight - vpvs.rcMargin.xpRight) < xpClose)
			dxa = dxaAbsRight;
		else  if (abs(((vpvs.rcMargin.xpRight + vpvs.rcMargin.xpLeft - aor.rc.xpRight + aor.rc.xpLeft)>>1) - prcNew->xpLeft) < xpClose)
			dxa = dxaAbsCenter;
		else  if (prcNew->xpLeft - prcPage->xpLeft >= xpClose &&
				prcPage->xpRight - prcNew->xpRight < xpClose &&
				abs(((dxpPage0 - aor.rc.xpRight + aor.rc.xpLeft)>>1) - prcNew->xpLeft) < xpClose)
			{
			pcHorz = pcHPage;
			dxa = dxaAbsCenter;
			}
		else
			{
			/* note that we don't generate dxaAbsLeft of
				dxaAbsRight with pcHPage because that would
				give sidebars - which is not what we're currently
				showing on screen */
			pcHorz = pcHPage;
			dxa = 1 + NMultDiv(prcNew->xpLeft - prcPage->xpLeft, vpvs.dxaPage, prcPage->xpRight - prcPage->xpLeft);
			}
		}

	if (dyp > ypClose)
		{
		int dypPage0 = vpvs.rcPage0.ypBottom - vpvs.rcPage0.ypTop;
		pcVert = pcVMargin;
		if (abs(prcNew->ypTop - vpvs.rcMargin.ypTop) < ypClose)
			dya = dyaAbsTop;
		else  if (abs(prcNew->ypBottom - vpvs.rcMargin.ypBottom) < ypClose)
			dya = dyaAbsBottom;
		else  if (abs(((vpvs.rcMargin.ypBottom + vpvs.rcMargin.ypTop - aor.rc.ypBottom + aor.rc.ypTop)>>1) - prcNew->ypTop) < ypClose)
			dya = dyaAbsCenter;
		else  if (prcNew->ypTop - prcPage->ypTop < ypClose)
			{
			pcVert = pcVPage;
			dya = dyaAbsTop;
			}
		else  if (prcPage->ypBottom - prcNew->ypBottom < ypClose)
			{
			pcVert = pcVPage;
			dya = dyaAbsBottom;
			}
		else  if (abs(((dypPage0 - aor.rc.ypBottom + aor.rc.ypTop)>>1) - prcNew->ypTop) < ypClose)
			{
			pcVert = pcVPage;
			dya = dyaAbsCenter;
			}
		else
			{
			pcVert = pcVPage;
			dya = 1 + NMultDiv(prcNew->ypTop - prcPage->ypTop, vpvs.dyaPage, prcPage->ypBottom - prcPage->ypTop);
			}
		}

/* effect the change */
	Assert(sprmPDxaAbs < sprmPDyaAbs && sprmPDyaAbs < sprmPPc &&
			dnsprm[sprmPDxaAbs].cch == 3 &&
			dnsprm[sprmPDyaAbs].cch == 3 &&
			dnsprm[sprmPPc].cch == 2 &&
			sizeof(rgb) >= 8);
	cch = 0;
	if (dxa != tmvalNinch)
		{
		rgb[0] = sprmPDxaAbs;
		bltb(&dxa, &rgb[1], sizeof(int));
		cch += 3;
		}
	if (dya != tmvalNinch)
		{
		rgb[cch] = sprmPDyaAbs;
		bltb(&dya, &rgb[cch + 1], sizeof(int));
		cch += 3;
		}
	if (pcVert != vpapFetch.pcVert || pcHorz != vpapFetch.pcHorz || cch != 0)
		{
		rgb[cch++] = sprmPPc;
		/* see PCVH structure (prm.h) to understand this obvious expression. */
		rgb[cch++] = (pcHorz << 6) + (pcVert << 4);
		}
	ApplyGrpprlCa(rgb, cch, &lr.ca);
	InvalCp(&lr.ca);
	DirtyDoc(vpvs.docPrvw);
/* move rectangle */
	ChangePrvwCancel();
	ShowAreas(hdc, fFalse);    /* erase */
	aor.rc = *prcNew;
	blt(&aor, PInPl(vpvs.hplaor, iaor), cwAOR);
	vpvs.fModeMargin = fTrue;
	ShowAreas(hdc, fFalse);    /* redraw */
}


/* %%Function:FRegenLrs %%Owner:davidbo */
FRegenLrs()
{
	struct RPL rpl;

	Assert(vpvs.tShowAreas != tAreasOff);
	if (vflm != flmRepaginate && !FResetWwLayout(vpvs.docPrvw, flmRepaginate, lmPreview))
		return fFalse;
	rpl.ipgd = vpvs.ipgdPrvw;
	rpl.ised = rpl.pgn = pgnMax;
	rpl.cp = cpMax;
	if (!FUpdateHplcpgd(wwLayout, vpvs.docPrvw, &vpvs.lbsText,&vpvs.lbsFtn,&rpl,patSilent))
		return fFalse;
	vpvs.cpMacPrvw = CpMax(vpvs.cpMacPrvw, vpvs.lbsText.cp);
	vpvs.ipgdLbs = rpl.ipgd + 1; /* +1 because of LbcFormatPage below */
	if (LbcFormatPage(&vpvs.lbsText, &vpvs.lbsFtn) == lbcAbort)
		return fFalse;
	if (vpref.fPrvwTwo && !vpvs.fFirstBlank)
		{
		if (!FSyncLbs() || LbcFormatPage(&vpvs.lbsText, &vpvs.lbsFtn) == lbcAbort)
			return fFalse;
		}
	return fTrue;
}


/* %%Function:ShowXaYaFromPt %%Owner:davidbo */
ShowXaYaFromPt(hdc, pt, grpfDrag, fTellCBT)
HDC hdc;
struct PT pt;
int grpfDrag;
BOOL fTellCBT;
{
	/* show location in xa and ya in the data box. */
	char szBuf[40], *pch;
	int xa, ya, cch, cchT, fBoth;

	Assert(hdc);
	Assert(2*ichMaxNum + 4 < 40); /* i.e. buffer can hold generated strings */
	pch = szBuf;
	cch = 0;
	fBoth = ((grpfDrag & (bitDragXa + bitDragYa)) == bitDragXa + bitDragYa);
	if (!grpfDrag)
		return; /* nothing to do */

	if (!FXaYaFromPt(pt, &xa, &ya, grpfDrag))
		return; /* mouse not in page */

	if (grpfDrag & bitDragXa)
		{
		/* show xa */
		if (fBoth)
			{
			*pch++ = '(';
			cch++;
			}
		cch += CchExpZa(&pch, xa, vpref.ut, ichMaxNum, fFalse);
		}

	if (grpfDrag & bitDragYa)
		{
		/* show ya */
		if (fBoth)
			{
			*pch++ = vitr.chList;
			*pch++ = ' ';
			cch += 2;
			}
		cch += CchExpZa(&pch, ya, vpref.ut, ichMaxNum, fFalse);
		if (fBoth)
			{
			*pch++ = ')';
			cch++;
			}
		}

	cch += CchStuff(&pch, mputsz[vpref.ut], max(0, sizeof(szBuf) - cch));
	/* if printing out fewer characters than currently displayed, fill buffer
		with enough ' ' to erase bogus characters.
	*/
	for (cchT = cch; cchT <= vpvs.cchIbLast; cchT++)
		*pch++ = ' ';
	*pch = 0;

#ifdef TEXTEVENT
/* This is disabled for Opus 1.0, but in the future CBT might want
   to teach the user to drag to a specific measurement, in which
   case we need to re-enable this code and they need to author in
   a Semantic Event (Text) pass/compare */
	if (fTellCBT)
		{
		CbtTextEvent(szBuf, cch);
		}
	else
#endif /* TEXTEVENT */
		{
		/* Remember how many characters are being put in the display. */
		vpvs.cchIbLast = cch;
		SetIibbTexthwndIb(vhwndPrvwIconBar, iibbPrvwStats, szBuf);
		GrayIibbHwndIb(vhwndPrvwIconBar, iibbPrvwStats, fFalse);
		}
}


/* %%Function:FXaYaFromPt %%Owner:davidbo */
FXaYaFromPt(pt, pxa, pya, grpf)
struct PT pt;
int *pxa, *pya, grpf;
{
	/* Here are the real guts of converting xp's and yp's to xa's and
		ya's.
	*/
	struct RC rcLim;

	/* Determine which page the pt is in. */
	/* full page view */
	if (FPtInRc(&vpvs.rcPage0, pt))
		rcLim = vpvs.rcPage0;
	else  if (FPtInRc(&vpvs.rcPage1, pt))
		rcLim = vpvs.rcPage1;
	else
		return fFalse;

	/* Map the pt to xa's and ya's. */
	*pxa =NMultDiv(pt.xp-rcLim.xpLeft, vpvs.dxaPage,rcLim.xpRight-rcLim.xpLeft);
	*pya =NMultDiv(pt.yp-rcLim.ypTop, vpvs.dyaPage, rcLim.ypBottom-rcLim.ypTop);

	if (vpref.ut == utInch)
		{
		RoundPzaTo16ths(pxa);
		RoundPzaTo16ths(pya);
		}

	if (grpf & bitTopRight)
		*pxa = vpvs.dxaPage - *pxa;
	else  if (grpf & bitBottomLeft)
		*pya = vpvs.dyaPage - *pya;
	return fTrue;
}


/* %%Function:RoundPzaTo16ths %%Owner:davidbo */
RoundPzaTo16ths(pza)
int *pza;
{
	/* round an xa or ya value to the nearest 16th of an inch 
		(expressed in xa or ya units).
	*/
	long lUnits = ((((long)*pza) << 4) + (czaInch>>1)) / czaInch;
	*pza = (int) (lUnits * (czaInch >> 4));
}


/* %%Function:FEqualPt %%Owner:davidbo */
FEqualPt(lpt1,lpt2)
long lpt1,lpt2;
{
	/* Returns true if pt1 == pt2 */
	/*
	*  Note: using longs so we can do one comparison rather than
	*  having to individually compare each element of the PT struct.
	*/
	Assert(sizeof(long) == sizeof(struct PT));
	return(lpt1 == lpt2);
}


/*
*  FCheckPopupRect is an EnumWindows function.  If a window is a popup and is
*  visible, FCheckPopupRect returns fTrue if the popup overlaps the rectangle
*  passed in lppuchk
*/
/* %%Function:FCheckPopupRect %%Owner:davidbo */
EXPORT BOOL far PASCAL FCheckPopupRect(hwnd,lppuchk)
HWND hwnd;
struct PUCHK FAR *lppuchk;
{
	struct RC rc,rcResult,rcT;

	/* window is not popup, therefore doesn't need to be tested */
	if (!(GetWindowLong(hwnd,GWL_STYLE) & WS_POPUP))
		return fTrue;

	/* window is not visible, therefore doesn't need to be tested.  Also
		due to the enumeration order of EnumWindows, enumeration can stop
		at the first hidden popup since the remaining windows to be enu-
		merated are hidden popups.
	*/
	if (!IsWindowVisible(hwnd))
		return fFalse;

	/* get popup's location in screen coordinates */
	GetWindowRect(hwnd, (LPRECT) &rc);

	/* convert from screen coordinates to lppuchk->hwnd coordinates */
	ScreenToClient(lppuchk->hwnd, (LPPOINT) &rc.ptTopLeft);
	ScreenToClient(lppuchk->hwnd, (LPPOINT) &rc.ptBottomRight);

	/* check for overlap */
	rcT = lppuchk->rc;
	if (FSectRc(&rc,&rcT,&rcResult))
		{
		lppuchk->fPopupOverlap = fTrue;
		return fFalse;   /*  overlap found, stop enumeration */
		}
	return fTrue;    /* enumerate next window */
}


/* %%Function:CacheAbsObj %%Owner:davidbo */
CacheAbsObj()
{
	int iaor = vpvs.iobj - iobjMinAbs;
	struct AOR aor;
	struct PLLR **hpllr;
	struct LR lr;

	Assert(iaor >= 0);
	Assert(vlm == lmPreview);
	blt(PInPl(vpvs.hplaor, iaor), &aor, cwAOR);
	hpllr = FLeftLbs() ? vpvs.lbsLeft.hpllr : vpvs.lbsText.hpllr;
	Assert(hpllr);
	bltLrp(LrpInPl(hpllr, aor.ilr), &lr, sizeof(struct LR));
	CachePara(lr.doc, lr.cp);
}


/* %%Function:ChangePrvwCancel %%Owner:davidbo */
ChangePrvwCancel()
{
	SetIibbTexthwndIb(vhwndPrvwIconBar, iibbPrvwClose, SzSharedKey("&Close",PreviewIBClose));
	if (vhwndCBT && lParamPrvwDrag != (LONG) -1)
		{
		SendMessage(vhwndCBT, WM_CBTSEMEV, smvPrvwDrag, lParamPrvwDrag);
		lParamPrvwDrag = (LONG) -1;
		}
}


/* %%Function:DrawPrvwLine %%Owner:davidbo */
EXPORT DrawPrvwLine(hdc, xpFrom, ypFrom, dxp, dyp, wColor)
HDC hdc;
int xpFrom, ypFrom, dxp, dyp, wColor;
{
	int xpTo, ypTo, dzpPen, fHorz = dxp > dyp, fStock = fFalse;
	HANDLE hPen, hOld;
	long rgb; /* NOTE: this is red/green/blue, not an rg of bytes! */

	Assert(hdc);
	Assert(vpvs.rcPage0.ypBottom - vpvs.rcPage0.ypTop > 0);
	Assert(vpvs.rcPage0.xpRight - vpvs.rcPage0.xpLeft > 0);
	if (fHorz)
		dzpPen = NMultDiv(vfti.dxpBorder, dypPage, vpvs.rcPage0.ypBottom - vpvs.rcPage0.ypTop);
	else
		dzpPen = NMultDiv(vfti.dypBorder, dxpPage, vpvs.rcPage0.xpRight - vpvs.rcPage0.xpLeft);

	Assert(wColor == colText || wColor == colAuto || wColor == colFetch);
	rgb = (wColor == colText) ? GetTextColor(hdc) : 
	      (wColor == colAuto) ? vpri.rgbText : GetRgbIco(vchpFetch.ico);

	if ((hPen = CreatePen(PS_SOLID, dzpPen, rgb)) == hNil)
		{
		hPen = GetStockObject(BLACK_PEN);
		fStock = fTrue;
		}
	LogGdiHandle(hPen, 1012);
	hOld = SelectObject(hdc, hPen);
	if (fHorz)
		{
		ypFrom += (dyp >> 1);
		xpTo = xpFrom + dxp + vfti.dxpBorder;
		ypTo = ypFrom;
		}
	else
		{
		xpFrom += (dxp >> 1);
		xpTo = xpFrom;
		ypTo = ypFrom + dyp + vfti.dypBorder;
		}
	MoveTo(hdc, xpFrom, ypFrom);
	LineTo(hdc, xpTo, ypTo);
	if (hOld != NULL)
		SelectObject(hdc, hOld);
	if (!fStock)
		{
		UnlogGdiHandle(hPen, 1012);
		DeleteObject(hPen);
		}
}


/* %%Function:CmdPrvwBound %%Owner:davidbo */
CMD CmdPrvwBound(pcmb)
CMB *pcmb;
{
	if (vlm != lmPreview)
		{
		ModeError();
		return cmdError;
		}
	return (FPrvwBoundaries(vpvs.tShowAreas == tAreasOff) ? cmdOK :cmdNoMemory);
}


/* %%Function:WElPrvwBoundaries %%Owner:davidbo */
EL WElPrvwBoundaries(fOn)
BOOL fOn;
{
	Assert(fElActive);
	if (vlm != lmPreview)
		{
		RtError(rerrModeError);
		Assert(fFalse);
		}

	if (vfElFunc)
		return (vpvs.tShowAreas == tAreasOff ? 0 : -1);

	if (vcElParams == 0)
		fOn = vpvs.tShowAreas == tAreasOff;
	else  if (!fOn == (vpvs.tShowAreas == tAreasOff))
		return 0;

	return (FPrvwBoundaries(fOn) ? -1 : 0);
}


/* %%Function:FPrvwBoundaries %%Owner:davidbo */
FPrvwBoundaries(fOn)
int fOn;
{
	HDC hdc;
	int fFail = fFalse;

	if ((hdc = GetDC(vhwndPgPrvw)) == NULL)
		{
		if (fElActive)
			RtError(rerrOutOfMemory);
		else
			ErrorNoMemory(eidNoMemDisplay);
		return fFalse;
		}
	vpvs.iobj = iobjNil;

	if (fOn)
		{
		vpvs.tShowAreas = tAreasLeft;
		ShowAreas(hdc, fTrue);
		}
	else
		{
		ShowAreas(hdc, fFalse);
		vpvs.tShowAreas = tAreasOff;
		vpvs.fAreasKnown = fFalse;
		}
	fFail = !FSetModeNorm(hdc, fFalse);
	vpvs.fModeMargin = fFalse;
	ReleaseDC(vhwndPgPrvw, hdc);
	if (fFail)
		{
		EndPreviewMode();
		if (fElActive)
			RtError(rerrOutOfMemory);
		else
			return fFalse;
		Assert(fFalse);
		}
	return fTrue;
}


/* %%Function:CmdPrvwPages %%Owner:davidbo */
CMD CmdPrvwPages(pcmb)
CMB *pcmb;
{
	if (vlm != lmPreview)
		{
		ModeError();
		return cmdError;
		}
	return (FPrvwPages(0) ? cmdOK : cmdNoMemory);
}


/* %%Function:WElPrvwPages %%Owner:davidbo */
EL WElPrvwPages(wPages)
int wPages;
{
	Assert(fElActive);
	if (vlm != lmPreview)
		{
		RtError(rerrModeError);
		Assert(fFalse);
		}

	if (wPages < 0 || wPages > 2)
		RtError(rerrOutOfRange);

	if (vfElFunc)
		return vpref.fPrvwTwo ? 2 : 1;
	else
		return (FPrvwPages(wPages) ? -1 : 0);
}


/* %%Function:FPrvwPages %%Owner:davidbo */
FPrvwPages(wPages)
{
	HDC hdc;
	int fWasTwo, fFail;
	char sz[maxww(sizeof(szCsPrvwOnePg), sizeof(szCsPrvwTwoPg))];

	fWasTwo = vpref.fPrvwTwo;
	if (wPages == 0)
		vpref.fPrvwTwo = !vpref.fPrvwTwo;
	else  if ((wPages == 2) == vpref.fPrvwTwo)
		return fTrue;
	else
		vpref.fPrvwTwo = (wPages == 2);

	if (vpref.fPrvwTwo)
		CopyCsSz(szCsPrvwOnePg, sz);
	else
		CopyCsSz(szCsPrvwTwoPg, sz);
	SetIibbTexthwndIb(vhwndPrvwIconBar, iibbPrvwPages, sz);

	vpvs.f1To2 = vpref.fPrvwTwo && !fWasTwo;
	if (vpvs.fModeMargin)
		{
		if ((hdc = GetDC(vhwndPgPrvw)) == NULL)
			{
			if (fElActive)
				{
				RtError(rerrOutOfMemory);
				Assert(fFalse);
				}
			else
				ErrorNoMemory(eidNoMemDisplay);
			return fFalse;
			}
		ShowAreas(hdc, fFalse); /* erase */
		vpvs.tShowAreas = tAreasOff;
		fFail = !FChangePrvwDoc(hdc, fTrue, fTrue);
		ReleaseDC(vhwndPgPrvw, hdc);
		vpvs.fModeMargin = fFalse;
		if (fFail)
			{
			EndPreviewMode();
			if (fElActive)
				{
				RtError(rerrOutOfMemory);
				Assert(fFalse);
				}
			else
				return fFalse;
			}
		else
			return fTrue;
		}

	/*
	*  For One Page mode, we say we're playing the left page...make sure
	*  on 2->1 page transistion, fLeftEmpty reflects the current lbs state.
	*/
	if (!vpvs.f1To2)
		{
		vpvs.fLeftEmpty = vpvs.lbsText.fEmptyPage;
		vpvs.ipgdPrvw = vpvs.ipgdLbs - 1;
		vpvs.ypScrlBar = YpFromCp(CpPlc(PdodDoc(vpvs.docPrvw)->hplcpgd,
			vpvs.ipgdPrvw));
		RSBSetSps(vhwndPgPrvwScrl, vpvs.ypScrlBar);
		}

	vpvs.tShowAreas = tAreasOff;
	vpvs.fAreasKnown = fFalse;
	SetPageRcs();
	InvalidateRect(vhwndPgPrvw, (LPRECT) NULL, fTrue);
	UpdateWindow(vhwndPgPrvw);
LRet:
	return fTrue;
}


/* %%Function:TrackMouseXaYa %%Owner:davidbo */
TrackMouseXaYa(hdc, grpfShow, ppt)
int grpfShow;
struct PT *ppt;
HDC hdc;
{
	/*
		Allows cursor to be moved around while the mouse button is down.
		The location of the mouse is shown in the iconbar as per the
		grpfShow flag.  Whwn the mouse button is released, *pt will
		contain its final postion in local coordinates.
	*/
	struct PT ptOld;
	int fShowPgn,fPgnVis = fFalse;
	struct RC rcClip;

	Assert(hdc);
	SetRect((LPRECT) &rcClip,xpMinPrvw,ypMinPrvw,vpvs.xpMacPrvw,vpvs.ypMacPrvw);
	ptOld = MAKEPOINT((long) zpUnknown);
	while (FStillDownReplay(ppt,fFalse))
		{
		/* If cursor is in a page, show offset from upper left
			corner, else show the current page number.
		*/
		if (!FPtInRc(&rcClip, *ppt))
			fShowPgn = fTrue;
		else
			fShowPgn = !(FPtInRc(&vpvs.rcPage0, *ppt) || FPtInRc(&vpvs.rcPage1, *ppt));

		/* Avoid flashing...only update when contents of data box
			change
		*/
		if (!fShowPgn && !FEqualPt(ptOld,*ppt))
			{
			ptOld = *ppt;
			ShowXaYaFromPt(hdc,*ppt,grpfShow,fFalse);
			fPgnVis = fFalse;
			}
		else  if (fShowPgn && !fPgnVis)
			{
			ShowPgnP(vpvs.ipgdPrvw, fFalse);
			fPgnVis = fTrue;
			}
		}
	/* When all done, display page number if not already displayed */
	if (grpfShow && !fPgnVis)
		ShowPgnP(vpvs.ipgdPrvw, fFalse);
}


/* %%Function:FTrackAndChangeObject %%Owner:davidbo */
FTrackAndChangeObject(hdc, pt, prcPage, fShift, fChange, pfError)
HDC hdc;
struct PT pt;
struct RC *prcPage;
int fShift, fChange, *pfError;
{
	/*
	* checks if pt is near any line or box object and if so tracks the
	* motion.  returns fTrue if the object was grabbed (and possibly
	* moved), fFalse otherwise.
	*/
	int iobj, iobjMac;

	Assert(hdc != NULL || !fChange);
	*pfError = fFalse;

	for (iobj = 0; iobj <= iobjMarginBottom; iobj++)
		if (FCheckLine(hdc, iobj, pt, fChange, pfError, prcPage))
			return fTrue;

	if (FCheckLine(hdc, iobjPgBrk, pt, fChange, pfError, prcPage))
		return fTrue;

	if (vpvs.hplaor != hNil)
		{
		iobjMac = iobjMinAbs + (*vpvs.hplaor)->iaorMac;
		for (iobj = iobjMinAbs; iobj < iobjMac; iobj++)
			if (FCheckBox(hdc, iobj, pt, fShift, fChange, prcPage))
				return fTrue;
		}

	return (FCheckBox(hdc, iobjHdr, pt, fShift, fChange, prcPage) ||
			FCheckBox(hdc, iobjFtr, pt, fShift, fChange, prcPage));
}


/* %%Function:FCheckLine %%Owner:davidbo */
FCheckLine(hdc, iobj, pt, fChange, pfError, prcPage)
HDC hdc;
int iobj, fChange, *pfError;
struct PT pt;
struct RC *prcPage;
{
	int ircs;
	struct RC rcBox, rcLim;

	rcLim = *prcPage;
	InflateRect((LPRECT) &rcLim, -1, -1);

	if (!FGenDragLine(iobj, &rcBox, &rcLim, fChange))
		return fFalse;

	if (!fChange)
		return (FPtInRc(&rcBox, pt));
	Assert(hdc != NULL);
	Assert(pfError != NULL);

	/* if iobjPgBrk set to ircs that is horizontal */
	ircs = (iobj == iobjPgBrk) ? ircsTop : iobj;

	lParamPrvwDrag = (LONG) -1;
	if (FTrackPrvwLine(hdc, &pt, ircs, &rcBox, &rcLim))
		{
		if (vhwndCBT)
			{
			/* advise CBT that we've dragged something */
			int grpf;
			lParamPrvwDrag = MakeLong(iobj, vfShiftKey ? 1 : 0);
			grpf = (FHorzIrcs(ircs) ? bitDragYa : bitDragXa) +
					((ircs == ircsLeft || ircs == ircsBottom) ?
					bitBottomLeft : bitTopRight);
#ifdef TEXTEVENT
/* This is disabled for Opus 1.0, but in the future CBT might want
   to teach the user to drag to a specific measurement, in which
   case we need to re-enable this code and they need to author in
   a Semantic Event (Text) pass/compare */
			ShowXaYaFromPt(hdc, pt, grpf, fTrue);
#endif /* TEXTEVENT */
			}
		if (iobj == iobjPgBrk)
			{
			if (!FChangePageBreak(hdc, pt, prcPage))
				{
				*pfError = fTrue;
				return fTrue;
				}
			}
		else
			ChangeMargins(hdc, pt, iobj);
		return fTrue;
		}
	return fFalse;
}


/* %%Function:FCheckBox %%Owner:davidbo */
FCheckBox(hdc, iobj, pt, fShift, fChange, prcPage)
HDC hdc;
int iobj, fShift, fChange;
struct PT pt;
struct RC *prcPage;
{
	int fTopLeft, grpfDrag;
	struct RC rcBox, rcLim;

	rcLim = *prcPage;
	InflateRect((LPRECT) &rcLim, -1, -1);

	if (!FGenDragBox(iobj, &rcBox, &rcLim, &fTopLeft, &grpfDrag, fShift))
		return fFalse;

	if (!fChange)
		return (FPtInRc(&rcBox, pt));
	Assert(hdc != NULL);

	lParamPrvwDrag = (LONG) -1;
	if (FTrackBox(hdc, pt, &rcBox, &rcLim, fTopLeft, grpfDrag))
		{
		if (vhwndCBT)
			{
			/* advise CBT that we've dragged something */
			lParamPrvwDrag = MakeLong(iobj, vfShiftKey ? 1 : 0);
#ifdef TEXTEVENT
/* This is disabled for Opus 1.0, but in the future CBT might want
   to teach the user to drag to a specific measurement, in which
   case we need to re-enable this code and they need to author in
   a Semantic Event (Text) pass/compare */
			ShowXaYaFromPt(hdc, pt, grpfDrag, fTrue);
#endif /* TEXTEVENT */
			}
		if (iobj >= iobjMinAbs)
			SetAbsPos(hdc, iobj - iobjMinAbs, &rcBox, prcPage);
		else  if (iobj == iobjHdr)
			SetHdrFtr(hdc, rcBox.ptTopLeft, fTrue, fShift);
		else
			{
			Assert(iobj == iobjFtr);
			SetHdrFtr(hdc, rcBox.ptBottomRight, fFalse, fShift);
			}
		return fTrue;
		}
	return fFalse;
}


/* %%Function:FGenDragLine %%Owner:davidbo */
FGenDragLine(iobj, prcHit, prcLim, fChange)
int iobj, fChange;
struct RC *prcHit, *prcLim;
{
	/*
		* Given a preview line object index (iobj), generate interesting data
		* about that object.  prcHit is the area around the line, prcLim is
		* the box the line can be dragged in.  This routine expects prcLim to
		* already be filled with a reasonable rectangle (vpvs.rcPage0 or vpvs.rcPage1).
		*/
	int z, zp, fHorz;
	int ircs, ircsOpposite;
	struct RC rcT;

	Assert(vpvs.tShowAreas != tAreasOff);
	if (iobj == iobjPgBrk)
		{
		if (vpvs.ypPageBreak == zpUnknown)
			return fFalse;
		prcLim->xpLeft = vpvs.rcMargin.xpLeft + 1;
		prcLim->xpRight = vpvs.rcMargin.xpRight + 1;
		prcLim->ypTop = vpvs.rcMargin.ypTop;
		zp = vpvs.ypPageBreak;
		fHorz = fTrue;
		}
	else
		{
		ircs = iobj;
		fHorz = FHorzIrcs(ircs);

		GetMargin(ircs, prcHit, &rcT);
		if (!fChange)
			goto LRet;

		/* limit motion to some distance from opposing margin */
		ircsOpposite = IrcsOppIrcs(ircs);
		if (fHorz)
			z = NMultDiv(vfli.dysInch>>1, vpvs.rcPage0.ypBottom - vpvs.rcPage0.ypTop, dypPage);
		else
			z = NMultDiv(vfli.dxsInch>>1, vpvs.rcPage0.xpRight - vpvs.rcPage0.xpLeft, dxpPage);
		*((int *)prcLim + ircsOpposite) =
				ZpFromRcIrcs(vpvs.rcMargin, ircsOpposite) + (FTopLeftIrcs(ircs) ? -z:z);

		zp = ZpFromRcIrcs(vpvs.rcMargin,ircs);
		}

	/* rcHit becomes the line and the area "near" it. */
	if (iobj == iobjPgBrk)
		{
		if (fHorz)
			SetRect((LPRECT) prcHit, prcLim->xpLeft, zp-xpClose, prcLim->xpRight, zp+xpClose);
		else
			SetRect((LPRECT) prcHit, zp-ypClose, prcLim->ypTop, zp+ypClose, prcLim->ypBottom);
		}

LRet:
	return fTrue;
}


/* %%Function:FTrackPrvwLine %%Owner:davidbo */
FTrackPrvwLine(hdc, ppt, ircs, prcHit, prcFrame)
HDC hdc;
struct PT *ppt;
int ircs;
struct RC *prcHit, *prcFrame;
{
	/* if the mouse was clicked near an important line, track the mouse 
		while dragging the line and report the final mouse position.
	*/
	Assert(hdc);

	/* If not near the line, say so to whoever thought you were. */
	if (!FPtInRc(prcHit, *ppt))
		return(fFalse);

	return(FTrackPL(hdc, ppt, prcFrame, ircs));
}


/*
* Does keyboard or mouse tracking of drag lines in preview.  Returns fFalse
* if user hits Escape, fTrue otherwise.
*/
/* %%Function:FTrackPL %%Owner:davidbo */
FTrackPL(hdc, ppt, prcLim, ircs)
HDC hdc;
struct PT *ppt;
struct RC *prcLim;
int ircs;
{
	int grpf, grpfDrag, fHorz, fEven, fNearGutter;
	struct PT ptMouse, ptT;

	Assert(hdc);
	fHorz = FHorzIrcs(ircs);
	grpfDrag = (fHorz ? bitDragYa : bitDragXa) +
			((ircs == ircsLeft || ircs == ircsBottom) ? bitBottomLeft :bitTopRight);
	fEven = FFacingEven(vpvs.tShowAreas == tAreasLeft);
	fNearGutter = vpvs.fFacing && ircs == (fEven ? ircsRight : ircsLeft);

	SelectObject(hdc, vhbrGray);
	DrawDragLine(hdc, ptMouse = *ppt, prcLim, fHorz); /* draw */
	ptT = ptMouse;
	if (fNearGutter)
		ptT.xp += (fEven ? vpvs.dxpGutter : -vpvs.dxpGutter);
	ShowXaYaFromPt(hdc, ptT, grpfDrag, fFalse);

	SetCapture(vhwndPgPrvw);
	while ((grpf = GrpfStillTracking(&ptMouse, fHorz)) & FSTILLTRACKING)
		{
	/* handle keyboard interface */
		if (grpf & FTOP)
			{
			if (fHorz)
				ptMouse.yp = prcLim->ypTop;
			else  /* FLEFT case for vertical line */
				ptMouse.xp = prcLim->xpLeft;
			}
		else  if (grpf & FBOTTOM)
			{
			if (fHorz)
				ptMouse.yp = prcLim->ypBottom;
			else  /* FRIGHT case for vertical line */
				ptMouse.xp = prcLim->xpRight;
			}
		else
			PtIntoRc(&ptMouse, prcLim);

		if (grpf & FKEYBOARD)
			{
			ptT = ptMouse;
			ClientToScreen(vhwndPgPrvw, (LPPOINT)&ptT);
			SetCursorPos(ptT.xp, ptT.yp);
			}

	/* erase old line, draw new line */
		if ((fHorz && ppt->yp != ptMouse.yp) || (!fHorz && ppt->xp != ptMouse.xp))
			{
			DrawDragLine(hdc, *ppt, prcLim, fHorz); /* erase */
			DrawDragLine(hdc, *ppt = ptMouse, prcLim, fHorz); /* draw */
			ptT = ptMouse;
			if (fNearGutter)
				{
				ptT.xp += (fEven ? vpvs.dxpGutter : -vpvs.dxpGutter);
				if (!FPtInRc(prcLim, ptT))
					ptT.xp = 0;
				}
			ShowXaYaFromPt(hdc, ptT, grpfDrag, fFalse);
			}
		}
	ReleaseCapture();

	PtIntoRc(ppt, prcLim);
	DrawDragLine(hdc, *ppt, prcLim, fHorz); /* erase */
	ShowPgnP(vpvs.ipgdPrvw, fFalse);
	return(!(grpf & FESCAPE));
}


/* %%Function:FGenDragBox %%Owner:davidbo */
FGenDragBox(iobj, prcBox, prcLim, pfTopLeft, pgrpfDrag, fShift)
int iobj, *pfTopLeft, *pgrpfDrag;
struct RC *prcBox, *prcLim;
{
	/*
	* Given a preview box object index (iobj), generate interesting data about
	* that object.  prcBox is the object, prcLim is the rectangle it can
	* be dragged in.  This routine expects prcLim to already be filled with
	* a reasonable rectangle (vpvs.rcPage0 or vpvs.rcPage1).
	* Hdr/Ftr objects tweak prcLim, but apos leave it alone.
	*/

	Assert(vpvs.tShowAreas != tAreasOff);
	if (iobj == iobjHdr)
		{
		if (vpvs.rcHdr.xpLeft == zpUnknown)
			return fFalse;
		*prcBox = vpvs.rcHdr;
		prcLim->xpRight = vpvs.rcMargin.xpRight - xpClose;
		prcLim->xpLeft = vpvs.rcMargin.xpLeft + xpClose;
		/* when free-floating headers/footers are not enabled,
			limit header/footer motion to within the margins */
		if (!fShift && PdopDoc(vpvs.docPrvw)->dyaTop >= 0)
			prcLim->ypBottom = max(vpvs.rcHdr.ypTop, vpvs.rcMargin.ypTop);
		*pfTopLeft = fTrue;
		*pgrpfDrag = bitDragYa + bitTopRight;
		}
	else  if (iobj == iobjFtr)
		{
		if (vpvs.rcFtr.xpLeft == zpUnknown)
			return fFalse;
		*prcBox = vpvs.rcFtr;
		prcLim->xpRight = vpvs.rcMargin.xpRight - xpClose;
		prcLim->xpLeft = vpvs.rcMargin.xpLeft + xpClose;
		if (!fShift && PdopDoc(vpvs.docPrvw)->dyaBottom >= 0)
			prcLim->ypTop = min(vpvs.rcFtr.ypBottom, vpvs.rcMargin.ypBottom);
		*pfTopLeft = fFalse;
		*pgrpfDrag = bitDragYa + bitBottomLeft;
		}
	else
		{
		int iaor;
		struct AOR aor;

		iaor = iobj - iobjMinAbs;
		Assert(iaor >= 0 && iaor < (*vpvs.hplaor)->iaorMac);
		blt(PInPl(vpvs.hplaor, iaor), &aor, cwAOR);
		*prcBox = aor.rc;
		*pfTopLeft = fTrue;
		*pgrpfDrag = bitDragYa + bitDragXa;
		}
	return fTrue;
}


/* %%Function:FTrackBox %%Owner:davidbo */
FTrackBox(hdc, pt, prcBox, prcLim, fTopLeft, grpfDrag)
HDC hdc;
struct PT pt;
struct RC *prcBox, *prcLim;
int fTopLeft;
int grpfDrag;
{
	/* If the mouse was clicked near an important rectangle, track the mouse
		while dragging the rectangle.  Final rectangle position is in *prcBox.
	*/
	int grpf, dxp, dyp;
	struct PT ptMouse, ptT;
	struct RC rcLim;

	Assert(hdc);
	/* determine if hit in rc */
	if (!FPtInRc(prcBox, pt))
		return(fFalse);

	/* bound the box - mouse tracks the same relative point within
		the rectangle where it was clicked.
	*/
	rcLim = *prcLim;
	dxp = pt.xp - prcBox->xpLeft;
	dyp = pt.yp - prcBox->ypTop;

	/* don't let any part of the box get outside rcLim */
	/* ...mins and maxes are for weird cases where object is bigger than page */
	rcLim.xpLeft += dxp;
	rcLim.xpLeft = min(rcLim.xpLeft, pt.xp);
	rcLim.xpRight -= prcBox->xpRight - pt.xp;
	rcLim.xpRight = max(rcLim.xpRight, pt.xp);
	rcLim.ypTop += dyp;
	rcLim.ypTop = min(rcLim.ypTop, pt.yp);
	rcLim.ypBottom -= prcBox->ypBottom - pt.yp;
	rcLim.ypBottom = max(rcLim.ypBottom, pt.yp);

	/* Drag the box */
	SelectObject(hdc, vhbrGray);
	DrawDragBox(hdc, prcBox, pt.xp - dxp, pt.yp - dyp);   /* draw */
	ShowXaYaFromPt(hdc, (fTopLeft)? prcBox->ptTopLeft : prcBox->ptBottomRight,
			grpfDrag, fFalse);
	ptMouse = pt;
	SetCapture(vhwndPgPrvw);
	while ((grpf = GrpfStillTracking(&ptMouse, fTrue)) & FSTILLTRACKING)
		{
		/* handle keyboard interface */
		if (grpf & FTOP)
			ptMouse.yp = rcLim.ypTop;
		else  if (grpf & FBOTTOM)
			ptMouse.yp = rcLim.ypBottom;
		else
			PtIntoRc(&ptMouse, (struct RC *)&rcLim);

		if (grpf & FKEYBOARD)
			{
			ptT = ptMouse;
			ClientToScreen(vhwndPgPrvw, (LPPOINT)&ptT);
			SetCursorPos(ptT.xp, ptT.yp);
			}

		if (!FEqualPt(ptMouse, pt))
			{
			FrameRectRop(hdc, prcBox, PATINVERT);
			pt = ptMouse;
			DrawDragBox(hdc, prcBox, pt.xp - dxp, pt.yp - dyp);
			ShowXaYaFromPt(hdc,(fTopLeft) ? prcBox->ptTopLeft : prcBox->ptBottomRight,
					grpfDrag, fFalse);
			}
		}
	ReleaseCapture();

	/* erase */
	FrameRectRop(hdc, prcBox, PATINVERT);
	ShowPgnP(vpvs.ipgdPrvw, fFalse);
	return(!(grpf & FESCAPE));
}


/* %%Function:DrawDragBox %%Owner:davidbo */
DrawDragBox(hdc, prc, xp, yp)
HDC hdc;
struct RC *prc;
int xp,yp;
{
	/* Draw prc at (xp,yp).  Uses PATINVERT raster-op so that successive
		calls toggle the visibility of the Box.
	*/
	Assert(hdc);
	OffsetRect((LPRECT) prc, xp - prc->xpLeft, yp - prc->ypTop);
	FrameRectRop(hdc, prc, PATINVERT);
}


/* %%Function:CmdRegenPrvw %%Owner:davidbo */
CMD CmdRegenPrvw()
{
	int ipgd, tShowAreas, fFail = fFalse;
	HDC hdc;

	ipgd = vpvs.ipgdPrvw;
	tShowAreas = vpvs.tShowAreas;
	UnInitPrvw(fFalse);
	/* if these fail, take down preview mode */
	if (!FInitWwLbsForRepag(wwCur, vpvs.docPrvw, lmPreview, &vpvs.lbsText,&vpvs.lbsFtn) ||
			!FInitPrvw(fFalse))
		{
		EndPreviewMode();
		return cmdError;
		}
	vpvs.ipgdPrvw = ipgd;
	vpvs.tShowAreas = tShowAreas;
	Assert(vpvs.ipgdPrvw <= IMacPlc(PdodDoc(vpvs.docPrvw)->hplcpgd));
	if ((hdc = GetDC(vhwndPgPrvw)) == NULL)
		{
		ErrorNoMemory(eidNoMemDisplay);
		return cmdNoMemory;
		}
	fFail = !FChangePrvwDoc(hdc, fTrue, fFalse);
	ReleaseDC(vhwndPgPrvw, hdc);
	if (fFail)
		{
		EndPreviewMode();
		return cmdNoMemory;
		}
	return cmdOK;
}


/* %%Function:FSetModeNorm %%Owner:davidbo */
FSetModeNorm(hdc, fAreasOff)
HDC hdc;
int fAreasOff;  /* ShowAreas() already called */
{
	Assert(hdc);
	if (vpvs.fModeMargin && !vpvs.fPrvwTab)
		{
		if (!fAreasOff)
			ShowAreas(hdc, fFalse); /* erase--turned on in FChangePrvwDoc */
		if (!FChangePrvwDoc(hdc, fTrue, fFalse))
			return fFalse;
		vpvs.fModeMargin = fFalse;
		}
	return fTrue;
}


/* %%Function:PrvwReturn %%Owner:davidbo */
void PrvwReturn()
{
	HDC hdc;
	int fFail;

	Assert(vhwndPgPrvw != NULL);
	if (vpvs.tShowAreas != tAreasOff)
		{
		if ((hdc = GetDC(vhwndPgPrvw)) == NULL)
			{
			ErrorNoMemory(eidNoMemDisplay);
			return;
			}
		fFail = !FSetModeNorm(hdc, fFalse);
		ReleaseDC(vhwndPgPrvw, hdc);
		if (fFail)
			EndPreviewMode();
		}
	else
		{
		Beep();
		}
}


/* %%Function:PrvwF1 %%Owner:davidbo */
void PrvwF1()
{
	GetHelp(cxtPrintPreview);
}


/* %%Function:PrvwShiftF1 %%Owner:davidbo */
void PrvwShiftF1()
{
	CmdHelpContext(NULL);
}


/* %%Function:PrvwTab %%Owner:davidbo */
void PrvwTab()
{
	int ircs, zp, fTopLeft, grpfDrag, iobjMac, fFail = fFalse;
	HDC hdc;
	struct PT pt, ptScreen;
	struct RC *prcPage, rcBox, rcLim;

	if (vpvs.tShowAreas == tAreasOff || PdodDoc(vpvs.docPrvw)->fLockForEdit)
		{
		Beep();
		return;
		}
	if ((hdc = GetDC(vhwndPgPrvw)) == NULL)
		{
		ErrorNoMemory(eidNoMemDisplay);
		return;
		}
	iobjMac = iobjMinAbs;
	if (vpvs.hplaor != hNil)
		iobjMac += (*vpvs.hplaor)->iaorMac;
	OurSetCursor(vhcPrvwCross);

LIncr:
	vpvs.iobj += vfShiftKey ? -1 : 1;
	if (vpvs.iobj >= iobjMac)
		vpvs.iobj = 0;
	else  if (vpvs.iobj < 0)
		vpvs.iobj = iobjMac - 1;

	prcPage = (vpvs.tShowAreas == tAreasLeft) ? &vpvs.rcPage0 : &vpvs.rcPage1;
	rcLim = *prcPage;
	Assert(vpvs.iobj >= 0 && vpvs.iobj < iobjMac);
	vpvs.fPrvwTab = fTrue;
	if (vpvs.iobj <= iobjMarginBottom || vpvs.iobj == iobjPgBrk)
		{
		if (!FGenDragLine(vpvs.iobj, &rcBox, &rcLim, fTrue))
			goto LIncr;
		/* if iobjPgBrk, make ircs a horizontal one */
		ircs = vpvs.iobj == iobjPgBrk ? ircsTop : vpvs.iobj;
		pt.xp = (rcBox.xpLeft + rcBox.xpRight) >> 1;
		pt.yp = (rcBox.ypTop + rcBox.ypBottom) >> 1;
		ptScreen = pt;
		ClientToScreen(vhwndPgPrvw, (LPPOINT) &ptScreen);
		SetCursorPos(ptScreen.xp, ptScreen.yp);
		if (FTrackPrvwLine(hdc, &pt, ircs, &rcBox, &rcLim))
			{
			if (vpvs.iobj == iobjPgBrk)
				{
				vpvs.fPrvwTab = fFalse;
				fFail = !FChangePageBreak(hdc, pt, prcPage);
				}
			else
				ChangeMargins(hdc, pt, vpvs.iobj);
			}
		}
	else
		{
		if (!FGenDragBox(vpvs.iobj, &rcBox, &rcLim, &fTopLeft, &grpfDrag, fFalse))
			goto LIncr;
		pt.xp = (rcBox.xpLeft + rcBox.xpRight) >> 1;
		pt.yp = (rcBox.ypTop + rcBox.ypBottom) >> 1;
		ptScreen = pt;
		ClientToScreen(vhwndPgPrvw, (LPPOINT) &ptScreen);
		SetCursorPos(ptScreen.xp, ptScreen.yp);
		if (FTrackBox(hdc, pt, &rcBox, &rcLim, fTopLeft, grpfDrag))
			{
			if (vpvs.iobj >= iobjMinAbs)
				SetAbsPos(hdc, vpvs.iobj - iobjMinAbs, &rcBox, prcPage);
			else  if (vpvs.iobj == iobjHdr)
				SetHdrFtr(hdc, rcBox.ptTopLeft, fTrue, fFalse);
			else
				{
				Assert(vpvs.iobj == iobjFtr);
				SetHdrFtr(hdc, rcBox.ptBottomRight, fFalse, fFalse);
				}
			}
		}
	vpvs.fPrvwTab = fFalse;
	OurSetCursor(vhcArrow);
	ReleaseDC(vhwndPgPrvw, hdc);
	if (fFail)
		EndPreviewMode();
}


/*
*  Returns true if pt is in rc.  NOTE: this is subtly different from the
*  gdi call PtInRect, viz. points on the right or bottom border are 'in'
*  the rc inFPtInRc but not in PtInRect.
*/
/* %%Function:FPtInRc %%Owner:davidbo */
FPtInRc(prc, pt)
struct RC *prc;
struct PT pt;
{
	return (pt.xp >= prc->xpLeft && pt.xp <= prc->xpRight &&
			pt.yp >= prc->ypTop && pt.yp <= prc->ypBottom);
}


FLeftLbs()
{
	return (vpref.fPrvwTwo && (vpvs.tShowAreas == tAreasLeft));
}


