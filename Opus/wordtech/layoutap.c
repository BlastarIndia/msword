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
DEBUGASSERTSZ         /* WIN - bogus macro for assert string */
#endif /* WIN */
#include "ch.h"
#include "props.h"
#include "doc.h"
#include "disp.h"

#ifdef PCWORD
#include "chfile.h"
#include "font.h"
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
#include "border.h"
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
#include "layoutap.cpt"
#endif /* PROTOTYPE */

#ifdef PCWORD
#define vsepFetch	vsepAbs
#define vpapFetch	vpapAbs
#endif /* PCWORD */

/* E X T E R N A L S */

extern struct CA        caSect;
extern struct CA        caTap;
extern struct CA        caPara;
extern struct CA        caParaL;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct FLI       vfli;
extern struct FLS		vfls;
extern struct MERR      vmerr;
extern struct PREF      vpref;
extern char HUGE        *vhpchFetch;
extern ENV              venvLayout;
extern struct PLLR      **vhpllrAbs;
extern int              vfRestartLayout;
extern BOOL             vfInFormatPage;
#ifdef PCWORD
extern struct RCPAGE	vrcPage;
extern struct PMD		vpmd;
extern struct LDF		vldf;
#else
extern struct TAP       vtapFetch;
#endif /* PCWORD */
extern int              vylTopPage;

#ifdef WIN
extern struct FTI       vfti;
#endif /* WIN */

/* number of attempts at laying out page; used to limit retries */
int			vcLayout;

#ifdef PCWORD
/***********************/
/* S e t   A b s   L r */
/* %%Function:SetAbsLr %%Owner:NOTUSED */
NATIVE SetAbsLr(plbs, plr) /* WINIGNORE - PCWORD version */
struct LBS *plbs;
struct LR *plr; /* on frame */
{
	struct PAP pap;
	int doc = plbs->doc;
	int fOutline = plbs->fOutline;
	CP cp = plbs->cp;
	CP cpLim;

	AssertSz(plbs->cl == 0, "SAL0");
	AssertSz(plr->clFirst == 0, "SAL1");

	plbs->fAbsPresent = fTrue;
	plr->cp = cp;
	plr->ihdt = plbs->ihdt;
	plr->dyl = 0;     /* need to know exact height */

	CacheParaL(doc, cp, fOutline);
	pap = vpapFetch;

	plr->dxl = XlFromXa(DxaFromDocCp(doc, cp, fTrue, &pap));

/* assign the x coordinate */
	AssignAbsXl(plbs, plr);

/* scan for end of consecutive absolute objects with same props */

	CacheSectBody(doc, cp);
	CacheParaL(doc, cp, fOutline);
	cpLim = CpMin(caSect.cpLim, CpMacText(doc));
	for (; FMatchAbs(caPara.doc, &pap, &vpapFetch); CacheParaL(doc, cp, fOutline))
		{
		plr->cpLim = caParaL.cpLim;
		if (caParaL.cpLim >= cpLim)
			break;
		cp = caParaL.cpLim;
		}
}


#else /* MAC || WIN */
/***********************/
/* S e t   A b s   L r */
#ifdef WIN
EXPORT /* SetAbsLr */
#else /* !WIN */
NATIVE /* WINIGNORE - SetAbsLr - EXPORT if WIN */
#endif /* !WIN */
/* %%Function:SetAbsLr %%Owner:chrism */
SetAbsLr(plbs, plr)
struct LBS *plbs;
struct LR *plr; /* on frame */
{
	CP cpLim;
	struct PAP pap;
	struct LBS lbsT;
	int dxl;
	CP cpImport;
	struct CA ca;

	/* WARNING: if Outline, FInTable calls below blow away vpapFetch
		constructed by CacheParaL!  Should be no reason to take pre-
		cautions since APO properties ignored in Outline. */
	Assert(!plbs->fOutline);

	CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
	plbs->fAbsPresent = fTrue;
	plr->cp = plbs->cp;
	plr->ihdt = plbs->ihdt;
	Assert(plbs->cl == 0);
	/* plr->clFirst = 0;    */
	plr->dyl = 0;     /* need to know exact height */
	pap = vpapFetch;
/* special cases: explicit width; table; or if 1st para just a picture,
	width is width of picture */
	if (vpapFetch.dxaWidth != 0)
		plr->dxl = XlFromXa(vpapFetch.dxaWidth);
	else  if (FInTableVPapFetch(plbs->doc, plbs->cp))
		{
		plr->dxl = DxlFromTable(plbs->ww, plbs->doc, plbs->cp);
		CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
		}
	/* this will check if the ca is a picture alone by itself
		in a paragraph, and handles pictures that are field results
	*/
	else  if (FCaIsGraphics(PcaSet(&ca, plr->doc, plr->cp,
			caParaL.cpLim - ccpEop), plbs->ww, &cpImport))
		{

#ifdef DEBUG
		/* FCaIs... did a ChFetch */
		if (cpImport == cpNil)
			Assert (*vhpchFetch == chPicture && vchpFetch.fSpec);
#endif /* DEBUG */

		/* if not a field - check visibility */
		if (cpImport != cpNil || (cpImport == cpNil && !vchpFetch.fVanish))
			{
			FormatLine(plbs->ww, plr->doc,
					cpImport == cpNil ? plr->cp : cpImport);
#ifdef WIN
			plr->dxl = vfli.xpRight;
			if (vfli.chBreak == chEop || vfli.fSplatBreak)
				plr->dxl -= vfli.rgdxp[vfli.ichMac - 1];
#else
			dxl = vfli.xpRight;
			if (vfli.fPrint)
				dxl = NMultDiv(dxl, vfli.dxsInch, vfli.dxuInch);
			plr->dxl = dxl;
			if (vfli.chBreak == chEop || vfli.fSplatBreak)
				{
				dxl = vfli.rgdxp[vfli.ichMac - 1];
				if (vfli.fPrint)
					dxl = NMultDiv(dxl, vfli.dxsInch, vfli.dxuInch);
				plr->dxl -= dxl;
				}
#endif
			}
		}

/* assign the x coordinate */
	AssignAbsXl(plbs, plr);

/* scan for end of consecutive absolute objects with same props */
	CacheSectL(plbs->doc, plbs->cp, plbs->fOutline);
	cpLim = CpMin(caSect.cpLim, CpMacDocPlbs(plbs));
	lbsT = *plbs;    /* for CacheParaL only */
	for (CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
			FMatchAbs(caPara.doc, &pap, &vpapFetch); 
			CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline))
		{
		if (FPageBreakBeforePap(vpapFetch) && caParaL.cpFirst > plr->cp)
			break;
		plr->cpLim = caParaL.cpLim;
		if (plr->cpLim >= cpLim)
			break;
		if (FInTableVPapFetch(lbsT.doc, lbsT.cp))
			{
			CpFirstTap(caParaL.doc, caParaL.cpFirst);
			lbsT.cp = plr->cpLim = caTap.cpLim;
			continue;
			}
		lbsT.cp = plr->cpLim;
		}
}


#endif /* !PCWORD */

/*****************************/
/* A s s i g n   A b s   X l */
#ifdef WIN
EXPORT /* AssignAbsXl */
#else /* !WIN */
NATIVE /* WINIGNORE - AssignAbsXl - EXPORT if WIN */
#endif /* !WIN */
/* %%Function:AssignAbsXl %%Owner:chrism */
AssignAbsXl(plbs, plr)
struct LBS *plbs;
struct LR *plr;
{
	int xlLeftMargin, xlRightMargin, dxlBetween;
	int xlRightAbs, xlLeftAbs;
	int xlLeftCol, dxlBleed, dxaAbs;

/* find left and right margins */
#ifdef PCWORD
	struct RC rcPage;
	GetXlMargins(plbs->fRight, fFalse, &xlLeftMargin, &xlRightMargin);

/* make room for right/left borders */
	rcPage = vrcPage;
	if (vpapFetch.btc && (vpapFetch.bsc != bscThick || vpmd.useBar))
		{
		if (vpapFetch.btc == btcBox || vpapFetch.fBorderLeft)
			rcPage.xpLeft += vldf.dxpAdj;
		if (vpapFetch.btc == btcBox || vpapFetch.fBorderRight)
			rcPage.xpRight -= vldf.dxp - vldf.dxpAdj;
		}
#else
	struct DOD *pdod = PdodMother(plbs->doc);
	GetXlMargins(&pdod->dop, plbs->fRight, WinMac(vfti.dxpInch, vfli.dxsInch),
			&xlLeftMargin, &xlRightMargin);
#endif /* !PCWORD */
	if (plr->fFixedXl)
		return;
	dxlBetween = XlFromXa(vpapFetch.dxaFromText);

/* set xl now */
	if (vpapFetch.pcHorz == pcHMargin)
		{
		xlLeftAbs = xlLeftMargin;
		xlRightAbs = xlRightMargin;
		}
	else  if (vpapFetch.pcHorz == pcHColumn)
		{
		xlLeftAbs = plbs->xl;
		xlRightAbs = plbs->xl + plbs->dxlColumn;
		}
	else
		{
		xlLeftAbs = 0;
		xlRightAbs = XlFromXa(IfPCWordElse(vsepFetch.xaPage, pdod->dop.xaPage));
		}

	dxaAbs = vpapFetch.dxaAbs;
	if (dxaAbs == dxaAbsOutside)
		dxaAbs = (plbs->fRight) ? dxaAbsRight : dxaAbsLeft;
	else  if (dxaAbs == dxaAbsInside)
		dxaAbs = (plbs->fRight) ? dxaAbsLeft : dxaAbsRight;

	switch (dxaAbs)
		{
	case dxaAbsCenter:
		plr->xl = (xlLeftAbs + xlRightAbs - plr->dxl) / 2;
		break;
	case dxaAbsLeft:
		plr->xl = xlLeftAbs;
		if (vpapFetch.pcHorz == pcHPage && plr->dxl + dxlBetween <= xlLeftMargin)
			plr->xl = xlLeftMargin - plr->dxl - dxlBetween;
		break;
	case dxaAbsRight:
		plr->xl = IfPCWordElse(min(xlRightAbs, rcPage.xpRight),
				xlRightAbs) - plr->dxl;
		if (vpapFetch.pcHorz == pcHPage && xlRightMargin +
				dxlBetween + plr->dxl <= xlRightAbs)
			plr->xl = xlRightMargin + dxlBetween;
		break;
	default:
		Assert(dxaAbs >= dxaAbsMin);
		plr->xl = xlLeftAbs + XlFromXa(dxaAbs - dxaAbsMin);
		}

/* inline may not interfere with columns to the left -- otherwise we can get 
	infinite loops in which the yl of the inline object fluctuates between a 
	closed set of values */
	if (vpapFetch.dyaAbs == dyaAbsInline)
		{
		dxlBleed = XlFromXa(vpapFetch.dxaFromText);
		xlRightAbs = plr->xl + plr->dxl + dxlBleed;
		if (xlRightAbs <= xlLeftMargin)
			return;	/* right edge of abs is in left margin */

		xlLeftAbs = plr->xl - dxlBleed;
		dxlBetween = XlFromXa(vsepFetch.dxaColumns);
		xlLeftCol = plbs->xl - dxlBetween;
		if (xlLeftCol <= xlLeftMargin || xlLeftAbs >= xlLeftCol)
			goto LEndAssignAbsXl;	/* only affects columns to the right */

		if (xlLeftCol >= xlLeftMargin || xlRightAbs > plbs->xl + plbs->dxlColumn + dxlBetween)
			{
			/* OK to be left of the first column; otherwise move it */
			plr->xl = plbs->xl + max(0, dxlBleed - dxlBetween);
			}
		}

LEndAssignAbsXl:
	;
#ifdef PCWORD
/* unprintable region bounds checking */
	if (plr->xl < rcPage.xpLeft)
		plr->xl = rcPage.xpLeft;
	if ((unsigned)plr->xl + (unsigned)plr->dxl >
			rcPage.xpRight && plr->xl < rcPage.xpRight)
		plr->dxl = rcPage.xpRight - plr->xl;
#endif /* PCWORD */
}


/***********************************/
/* C o n s t r a i n   T o   A b s */
#ifdef WIN
EXPORT /* ConstrainToAbs */
#else /* !WIN */
NATIVE /* WINIGNORE - ConstrainToAbs - EXPORT if WIN */
#endif /* !WIN */
/* %%Function:ConstrainToAbs %%Owner:chrism */
int ConstrainToAbs(plbs, dylFill, prcl, prclPend, pfPend, plrResult)
struct LBS *plbs;
int dylFill;
struct RC *prcl, *prclPend;
int *pfPend;
struct LR *plrResult;
{
/* modifies prcl to avoid absolute objects. if prcl is split across an abs
	object, returns fTrue and the rectangle on the other side in prclPend */
	int ilr, ilrMac = IMacPllr(plbs);
	LRP lrp;
	int dxlBleed, dylBleed;
	struct RC rcl, rclPend, rclAbs;
	struct LBS lbsT;

	lbsT = *plbs;
	rcl = *prcl;
	if (*pfPend)
		rclPend = *prclPend;
	for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; ilr++, lrp++)
		{
		/* don't avoid zero size object */
		if (lrp->lrk != lrkAbs || lrp->dyl == 0 || lrp->dxl == 0)
			continue;
		/* ignore inlines that haven't been done yet */
		if (lrp->fInline && lrp->cp > plbs->cp)
			continue;
/* an absolute object was found */
		lbsT.doc = lrp->doc;
		lbsT.cp = lrp->cp;
		CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
		dxlBleed = XlFromXa(vpapFetch.dxaFromText);
/* in-line means 0 vertical space between, otherwise this code contradicts 
	LbcPositionAbs and we get weird pages */
		dylBleed = (lrp->fInline) ? 0 : YlFromYa(vpapFetch.dxaFromText);
		rclAbs.xlRight = lrp->xl + lrp->dxl + dxlBleed;
		rclAbs.xlLeft = lrp->xl - dxlBleed;
		rclAbs.ylTop = lrp->yl - dylBleed;
		rclAbs.ylBottom = lrp->yl + lrp->dyl + dylBleed;
		if (rclAbs.xlLeft >= rcl.xlRight ||
				(unsigned)rclAbs.xlRight <= rcl.xlLeft ||
				(unsigned)rclAbs.ylBottom <= rcl.ylTop)
			continue;	/* no collision */

/* check for vertical collision with object */
		if (rclAbs.ylTop - rcl.ylTop <= plbs->dylOverlap)
			{
			/* abs obj forces us narrower */
			rcl.ylBottom = min(rcl.ylBottom, rclAbs.ylBottom);
			plrResult->lrk = lrkAbsHit;
			if (plrResult->tSoftBottom != tZero)
				plrResult->tSoftBottom = (rcl.ylBottom < plbs->ylMaxLr) ? tPos : tZero;
			if (rcl.xlLeft >= rclAbs.xlLeft)
				{
				plrResult->fConstrainLeft = fTrue;
				rcl.xlLeft = rclAbs.xlRight;
				}
			else
				{
				plrResult->fConstrainRight = fTrue;
				if (rcl.xlRight >= DxlTooNarrow + rclAbs.xlRight)
					{
					/* block split by abs */
					if (rclAbs.xlLeft < DxlTooNarrow + rcl.xlLeft)
						{
						rcl.xlLeft = rclAbs.xlRight;
						continue;
						}
					rclPend = rcl;
					rclPend.xlLeft = rclAbs.xlRight;
					*pfPend = fTrue;
					}
				rcl.xlRight = rclAbs.xlLeft;
				}
			if (rcl.xlRight < DxlTooNarrow + rcl.xlLeft)
				{
				rcl.ylTop = rcl.ylBottom;  /* force retry */
				plbs->fPrevTable = fFalse;
				plrResult->fConstrainTop = fTrue;
				break;
				}
			}
		else
			{
			/* abs obj forces us shorter; or at least constrains */
			if (rclAbs.ylTop <= rcl.ylBottom)
				{
				plrResult->lrk = lrkAbsHit;
				rcl.ylBottom = rclAbs.ylTop;
				}
			plbs->ylMaxLr = min(plbs->ylMaxLr, rclAbs.ylTop);
			}
		}
	*prcl = rcl;
	if (*pfPend)
		*prclPend = rclPend;
}


/***********************************/
/* L b c   P o s i t i o n   A b s */
#ifndef WIN
NATIVE /* WINIGNORE - MAC only */
#endif /* !WIN */
/* %%Function:LbcPositionAbs %%Owner:chrism */
int LbcPositionAbs(plbs, cpLim, dylFill, fNeedHeight, fEmptyOK)
struct LBS *plbs;
CP cpLim;
int dylFill, fNeedHeight, fEmptyOK;
{
/* assign correct vertical position to an absolute object. format the
	paragraphs in the object if its height is not known */
	LRP lrp;
	LRP lrpT;
	int ilr, ilrMac, dyaAbs, pcVert;
	int lbc = lbcNil;
	int ylTopAbs, ylBottomAbs, yl;
	CP cp;
	struct DOD *pdod;
	struct DRC drcl1, drcl2;
#ifdef PCWORD
	int ylBottomEdge = vrcPage.ypBottom;
#endif /* PCWORD */

	CacheParaL(plbs->doc, cp = plbs->cp, plbs->fOutline);
	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	if (cpLim < lrp->cpLim && fEmptyOK)
		return(lbcYlLim);	/* cps restricted; say out of room */
	cpLim = lrp->cpLim;
	plbs->fPrevTable = fFalse;
	lrp->lrs = lrsNormal;
	dyaAbs = vpapFetch.dyaAbs;
	pcVert = vpapFetch.pcVert;
	if (fNeedHeight || cpLim < caParaL.cpLim)
		{
		lrp->dyl = 0;	/* force 0 height in case re-using LR */
		lbc = LbcFormatBlock(plbs, cpLim, ylLarge, fFalse, fFalse, fTrue);
		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
		if (lbc == lbcEndOfPage)
			{
			lrp->fPageAfterAbs = fTrue;
			goto LSetDyl;
			}
		else  if (lbc == lbcEndOfColumn)
			{
			lrp->fColAfterAbs = fTrue;
LSetDyl:
			if (lrp->cpLim == lrp->cp + ccpSect)
				lrp->dyl = 0;
			}
		else  if (lbc == lbcEndOfSection && plbs->cp > cpLim)
			{
			lbc = lbcNil;	/* ignore fancy section addition */
			lrp->cpLim = cpLim;
			vfls.ca.doc = docNil;
			}
		}
	CacheSectL(plbs->doc, cp, plbs->fOutline);
	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	plbs->cp = lrp->cpLim;
	if (plbs->cp >= CpMacDocPlbs(plbs))
		lbc = lbcEndOfDoc;
	else  if (plbs->cp == caSect.cpLim)
		lbc = lbcEndOfSection;
	else  if (lrp->fPageAfterAbs)
		lbc = lbcEndOfPage;
	else  if (lrp->fColAfterAbs)
		lbc = lbcEndOfColumn;

#ifdef PCWORD
	/* move up to prevent bottom border from being clipped */
	if (vpapFetch.btc && (vpapFetch.btc == btcBox || vpapFetch.fBorderBelow) &&
			!(vpapFetch.bsc == bscThick || vpmd.useBar))
		ylBottomEdge -= YpFromYa(czaLine / 2);
#endif /* PCWORD */

/* assign true abs yl */
	if (dyaAbs == dyaAbsInline)
		{
		/* look for interfering in-line abs */
		drcl1 = lrp->drcl;
		drcl1.yl = plbs->yl;
		/* before ignored at top of page */
		if (drcl1.yl == vylTopPage && vpapFetch.dyaBefore != 0)
			drcl1.yl -= YlFromYa(vpapFetch.dyaBefore);
		ilrMac = IMacPllr(plbs);
		for (lrpT = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; ilr++, lrpT++)
			{
			if (ilr == plbs->ilrCur || !lrpT->fInline || lrpT->cp > cp)
				continue;
			drcl2 = lrpT->drcl;
			if (FSectDrc(&drcl1, &drcl2))
				{
				/* overlap vertically and horizontally */
				drcl1.yl = drcl2.yl + drcl2.dyl;
				}
			}
		if (fEmptyOK && drcl1.yl + drcl1.dyl > plbs->ylColumn + dylFill &&
				!(plbs->fFirstColumn && drcl1.yl == plbs->ylColumn))
			/* abs won't fit on page */
			return(lbcYlLim);
		lrp->fInline = fTrue;
		yl = drcl1.yl;
		}
	else
		{
#ifdef PCWORD
		if (pcVert == pcVPage)
			{
			ylTopAbs = 0;
			ylBottomAbs = YlFromYa(vsepFetch.yaPage);
			}
		else
			{
			ylTopAbs = YlFromYa(vsepFetch.yaTop);
			ylBottomAbs = YlFromYa(vsepFetch.yaTop + vsepFetch.dyaText);
			}
#else
		pdod = PdodMother(plbs->doc);
		if (pcVert == pcVPage)
			{
			ylTopAbs = 0;
			ylBottomAbs = YlFromYa(pdod->dop.yaPage);
			}
		else
			{
			ylTopAbs = YlFromYa(abs(pdod->dop.dyaTop));
			ylBottomAbs = YlFromYa(pdod->dop.yaPage - abs(pdod->dop.dyaBottom));
			}
#endif /* !PCWORD */

		switch (dyaAbs)
			{
		case dyaAbsTop:
			yl = ylTopAbs;
			break;
		case dyaAbsCenter:
			yl = (ylTopAbs + ylBottomAbs - lrp->dyl) / 2;
			break;
		case dyaAbsBottom:
			yl = IfPCWordElse(min(ylBottomAbs, ylBottomEdge), ylBottomAbs) -					 lrp->dyl;
			break;
		default:
			Assert(dyaAbs >= dyaAbsMin);
			yl = ylTopAbs + YlFromYa(dyaAbs - dyaAbsMin);
			}
		}

#ifdef PCWORD
	/* unprintable region bounds checking */
	if (yl < vrcPage.ypTop)
		yl = vrcPage.ypTop;
	if ((unsigned)yl + (unsigned)lrp->dyl > ylBottomEdge && yl < ylBottomEdge)
		lrp->dyl = ylBottomEdge - yl;
#endif /* PCWORD */

/* we always have to check because of inline */
	lrp->yl = yl;
	CheckAbsCollisions(plbs);    /* may not return */
	return(lbc);
}


/*******************************************/
/* C h e c k   A b s   C o l l i s i o n s */
#ifndef WIN
NATIVE /* WINIGNORE - MAC only */
#endif /* !WIN */
/* %%Function:CheckAbsCollisions %%Owner:chrism */
CheckAbsCollisions(plbs)
struct LBS *plbs;
{
	LRP lrp;
	int ilr, ilrMac = IMacPllr(plbs);
	int xlLeft, xlRight;
	int fAbsCur, fInline;
	int dxl, dyl;
	struct DOD *pdod;
	struct DRC drcl, drclT;

	Assert(vfInFormatPage);
	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	if (lrp->dyl == 0)
		return;
	Assert(lrp->lrk == lrkAbs);
	fInline = lrp->fInline;
	drcl = lrp->drcl;
	CacheParaL(lrp->doc, lrp->cp, plbs->fOutline);
	dxl = XlFromXa(vpapFetch.dxaFromText);
	drcl.xl -= dxl;
	drcl.dxl += dxl + dxl;
/* in-line means 0 vertical space between, otherwise this code contradicts 
	LbcPositionAbs and we get weird pages */
	if (!fInline)
		{
		dyl = YlFromYa(vpapFetch.dxaFromText);
		drcl.yl -= dyl;
		drcl.dyl += dyl + dyl;
		}

/* scan for a collision */
	for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; ilr++, lrp++)
		{
		/* don't need to check against itself or against other abs;
			abs doesn't affect headers */
		if (ilr == plbs->ilrCur || lrp->lrk == lrkAbs || 
				lrp->ihdt != ihdtNil || lrp->doc < 0 || PdodDoc(lrp->doc)->fShort)
			continue;
		drclT = lrp->drcl;
		if (FSectDrc(&drcl, &drclT))
			{
			if (!fInline || lrp->lrs == lrsFrozen || plbs->yl > drcl.yl)
				break;
			lrp->drcl.dyl = drcl.yl - lrp->drcl.yl;
			lrp->lrk = lrkAbsHit;
			lrp->lrs = lrsFrozen;
			}
		else  if (drclT.yl + drclT.dyl == drcl.yl)
			/* touch at top of apo; prevent overlap */
			lrp->tSoftBottom = tZero;
		}

	if (ilr == ilrMac)
		return;
	if (++vcLayout > 10)
		{
#ifdef DEBUG
#ifdef MAC
		Assert(fFalse);
#else
		ReportSz("Layout exceeded 10 retries.");
#endif
#endif
		return;
		}

/* collision was found - restart, remembering abs positions */
	if (vhpllrAbs != hNil || (vhpllrAbs
			= HpllrInit(sizeof(struct LR), 2)) != hNil)
		{
		(*vhpllrAbs)->ilrMac = 0;
		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
		lrp->fFixedXl = (vpapFetch.pcHorz == pcHColumn);
		CopyAbsLrs(plbs->hpllr, vhpllrAbs, fFalse, fTrue);
		vfRestartLayout = fTrue;
		}
	AbortLayout();
}


#ifndef WIN	/* win doesn't let APOs perturb footnotes */
/********************************/
/* D y l   D e a d   S p a c e  */
#ifndef WIN
NATIVE /* WINIGNORE - DylDeadSpace - pcode if WIN */
#endif /* !WIN */
/* %%Function:DylDeadSpace %%Owner:chrism */
int DylDeadSpace(plbs)
struct LBS *plbs;
{
/* returns amount of dead space due to APOs below plbs->yl */
	int ilr, ilrMac = IMacPllr(plbs);
	int dxlBleed, dylBleed;
	int xlRightCol, xlRightLr, ylBottomLr;
	int dyl = 0;
	struct LR lr;

	xlRightCol = plbs->xl + plbs->dxlColumn;
	for (ilr = 0; ilr < ilrMac; ilr++)
		{
		bltLrp(LrpInPl(plbs->hpllr, ilr), &lr, sizeof(struct LR));
		if (lr.lrk != lrkAbs)
			continue;
		CacheParaL(lr.doc, lr.cp, plbs->fOutline);
		dxlBleed = XlFromXa(vpapFetch.dxaFromText);
		xlRightLr = lr.xl + lr.dxl + dxlBleed;
		lr.xl -= dxlBleed;

		if (lr.xl < xlRightCol && xlRightLr >= plbs->xl &&
				lr.xl - plbs->xl < DxlTooNarrow && 
				xlRightCol - xlRightLr < DxlTooNarrow)
			{
			/* FUTURE: be aware of overlapping APOs */
			dylBleed = (lr.fInline) ? 0 : YlFromYa(vpapFetch.dxaFromText);
			if (lr.yl - dylBleed >= plbs->ylMaxColumn)
				continue;	/* apo is in bottom margin, ignore it */
			ylBottomLr = min(plbs->ylMaxColumn, lr.yl + lr.dyl + dylBleed);
			if (ylBottomLr >= plbs->yl)
				dyl += ylBottomLr - max(lr.yl - dylBleed, plbs->yl);
			}
		}
	return(dyl);
}
#endif


/***************************/
/* C o p y   A b s   L r s */
#ifndef WIN
NATIVE /* WINIGNORE - MAC only */
#endif /* !WIN */
/* %%Function:CopyAbsLrs %%Owner:chrism */
CopyAbsLrs(hpllrFrom, hpllrTo, fCopyHdr, fCopyAll)
struct PLLR **hpllrFrom, **hpllrTo;
int fCopyHdr, fCopyAll;
{
	int ilrFrom, ilrTo, ilrMac = (*hpllrFrom)->ilrMac;
	struct LR lr;

	for (ilrFrom = 0, ilrTo = (*hpllrTo)->ilrMac; ilrFrom < ilrMac; )
		{
		bltLrp(LrpInPl(hpllrFrom, ilrFrom++), &lr, sizeof(struct LR));
		if (lr.lrk == lrkAbs && (fCopyHdr || lr.ihdt == ihdtNil) &&
				(fCopyAll || lr.lrs != lrsIgnore))
			{
			lr.lrs = lrsIgnore;
			ReplaceInPllr(hpllrTo, ilrTo++, &lr);
			}
		}
}


/***********************/
/* F   S e c t   D r c */
#ifndef WIN
NATIVE /* WINIGNORE - FSectDrc */
#endif /* !WIN */
/* %%Function:FSectDrc %%Owner:chrism */
FSectDrc(pdrc1, pdrc2)
struct DRC *pdrc1, *pdrc2;
{
	struct RC rc1, rc2;
#ifdef WIN
	struct RC rcDummy;
#endif /* WIN */

	DrcToRc(pdrc1, &rc1);
	DrcToRc(pdrc2, &rc2);
#ifdef MAC
	return(FSectRc(&rc1, &rc2));
#else /* WIN */
	return(FSectRc(&rc1, &rc2, &rcDummy));
#endif /* WIN */
}


/***********************************/
/* L b c   F o r m a t   C h a i n */
#ifdef WIN
EXPORT /* LbcFormatChain */
#else /* !WIN */
NATIVE /* WINIGNORE - LbcFormatChain - EXPORT if WIN */
#endif /* !WIN */
/* %%Function:LbcFormatChain %%Owner:chrism */
int LbcFormatChain(plbs, cpLim, dylFill, clLim, fLnn)
struct LBS *plbs;
CP cpLim;
int dylFill, clLim, fLnn;
{
	int ilr, ilrMac, ilrNext, ilrFirst, ilrT, clrDel;
	int yl, dxl, lbc = lbcNil, cl, ww;
	int fEndChain, fGenLr = fFalse, fSoftBottom, fTable;
	int dylOverlapNew = 0;
	CP cp, cpFirst;
	struct DOD *pdod;
	int dylLine, dylBase, dylBaseCur, dylBefore;
	int dylAbove, dylBelow;
	LRP lrp;
	struct LR lr;
	struct LBS lbsT;
	int fShort;

	/* WARNING: if Outline, FInTable calls below blow away vpapFetch
		constructed by CacheParaL!  Should be no reason to take pre-
		cautions since APO properties ignored in Outline. */
	Assert(!plbs->fOutline);
	ShowLbs(plbs, irtnFormChain/*irtn*/);

/* determine formatting environment */
	Assert(plbs->ilrCurChain == plbs->ilrFirstChain);
	lbsT = *plbs;
	ww = plbs->ww;

#ifdef PCWORD
	fShort = (plbs->cp > CpMacText(plbs->doc));
#else
	pdod = PdodDoc(plbs->doc);
	if (fShort = pdod->fShort)
		{
		ww = WinMac(wwLayout, wwTemp);
		LinkDocToWw(plbs->doc, ww, wwNil);
		}
#endif /* !PCWORD */

#ifdef MAC
	DxsSetDxsInch(plbs->doc);
#endif /* MAC */

/* get raw cp */
	ClFormatLines(plbs, cp0, 0, 0, 0, 0, fFalse, fFalse);

	/* use if-else 'cuz WIN native compiler can't handle such a complex
		ternary operation.  Yep, this is just sooooo complex. */
	if ((cl = plbs->cl) == 0)
		cp = plbs->cp;
	else
		cp = CpFromCpCl(plbs, fTrue);

	ilrMac = IMacPllr(plbs);
	lrp = LrpInPl(plbs->hpllr, plbs->ilrFirstChain);
	Assert(lrp->ilrNextChain != ilrNil);
	dylFill = min(lrp->dyl, dylFill - lrp->yl + plbs->ylColumn);

/* format through the chain until limit of cp or cl reached */
	vfli.fLayout = fTrue;	/* splats like page view */
	for (yl = lrp->yl; lbc == lbcNil && cp < cpLim && cl < clLim; cl++)
		{
		dylLine = dylBase = dylBefore = 0;
		cpFirst = cp;
		fEndChain = fFalse;
		for (ilrFirst = ilrMac, ilr = plbs->ilrFirstChain; ilr != ilrNil; ilr = ilrNext)
			{
			bltLrp(LrpInPl(plbs->hpllr, ilr), &lr, sizeof(struct LR));
			lr.lnn = (fLnn && ilr == plbs->ilrFirstChain) ? plbs->lnn++ : lnnNil;
			Assert(lr.lrk == lrkAbsHit);
			ilrNext = lr.ilrNextChain;
/* end of cp space */
			if (fEndChain)
				{
LDummyLr:
				lr.dyl = dylLine - dylBefore;
				dylBaseCur = dylBase;
				lr.cp = cp;
				lr.yl = yl;
				goto LHaveDyl;
				}

/* tables */
LFormat:
			fTable = fFalse;
#ifndef PCWORD
			lbsT.cp = cp;
			CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
			if (FInTableVPapFetch(lbsT.doc, lbsT.cp))
				{
				fTable = fTrue;
				lr.cp = cp;
				lr.dyl = DypHeightTable(plbs->ww, plbs->doc, cp, !plbs->fPrevTable, fTrue, &dylAbove, &dylBelow);
				if (DxlFromTable(plbs->ww, plbs->doc, plbs->cp) > lr.dxl)
					{
					dylLine = max(dylLine, lr.dyl);	/* for dylOverlap */
					caPara.doc = docNil;
					goto LTooTall;
					}
				caPara.doc = docNil;	/* munged by DypHeightTable */
				lr.dyl += dylAbove + dylBelow;
				if (lr.tSoftBottom == tPos)
					{
					if (lr.dyl > plbs->ylMaxLr - yl)
						{
						dylLine = max(dylLine, lr.dyl);	/* for dylOverlap */
						goto LTooTall;
						}
					}
				else  if (lr.dyl > dylFill)
					{
					dylLine = max(dylLine, lr.dyl);	/* for dylOverlap */
					goto LTooTall;
					}
				dylBaseCur = 0; /* no descender */
				cp = caTap.cpLim;
				if (cp >= cpLim)
					fEndChain = fTrue;

				Assert(plbs->cp == lr.cp);
				Assert(ilr == plbs->ilrFirstChain);
				if (!plbs->fPrevTable)
					lr.fForceFirstRow = fTrue;

				plbs->fPrevTable = fTrue;
				plbs->dylBelowTable = dylBelow;
				CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
				goto LHaveDyl;
				}
			if (ilr == plbs->ilrFirstChain)
				plbs->fPrevTable = fFalse;
#endif /* !PCWORD */

/* normal text */
			FormatLineDxaL(ww, plbs->doc, lr.cp = cp, XaFromXl(lr.dxl));
			cp = vfli.cpMac;
			dxl = vfli.xpRight;
#ifdef MAC
			if (vfli.fPrint)
				dxl = NMultDiv(dxl, vfli.dxsInch, vfli.dxuInch);
#endif /* MAC */
			if (vfli.ichMac == 1 && dxl > lr.dxl)
				{
				/* a character (e.g. pic) is too wide for this space */
				dylLine = max(dylLine, vfli.dypLine);	/* for dylOverlap */
				goto LTooTall;
				}
			if (vfli.fParaStopped)
				{
				fEndChain = fTrue;
				goto LDummyLr;
				}
			if (lr.tSoftBottom == tPos)
				{
				if (vfli.dypLine > plbs->ylMaxLr - yl)
					{
					dylLine = max(dylLine, vfli.dypLine);	/* for dylOverlap */
					goto LTooTall;
					}
				}
			else  if (vfli.dypLine > dylFill)
				{
				dylLine = max(dylLine, vfli.dypLine);	/* for dylOverlap */
				goto LTooTall;
				}
			lr.yl = yl;
			lr.dyl = vfli.dypLine;
			dylBaseCur = DypBaseFli(vfli);
			dylBefore = max(vfli.dypBefore, dylBefore);

/* now we have the height of this line (or table) - finish the lr */
LHaveDyl:
			dylLine = max(dylLine, lr.dyl);
			dylBase = max(dylBase, lr.clFirst = dylBaseCur); /* temporary storage */
			lr.cpLim = cp;
			lr.lrs = lrsFrozen;
			/* ilrNextChain points to self for all but 1st */
			lr.ilrNextChain = (ilr == plbs->ilrFirstChain) ? ilrNil : ilr;
			lr.fChain = fTrue;
			if (!FInsertInPl(plbs->hpllr, ilrMac++, &lr))
				{
				Assert(vfInFormatPage);
				AbortLayout();
				}
			if (!fTable && (vfli.fSplatBreak || vfli.fSplatColumn))
				{
				if ((fEndChain = !fShort) == fTrue)
					{
					CacheSectL(vfli.doc, vfli.cpMin, plbs->fOutline);
					lbc = (vfli.fSplatColumn) ? lbcEndOfColumn :
							(cp == caSect.cpLim) ? lbcEndOfSection :
							lbcEndOfPage;
					}
				}
			if (cp >= cpLim)
				fEndChain = fTrue;
			}

/* adjust dylLine in case baselines didn't match */
		lrp = LrpInPl(plbs->hpllr, ilrT = ilrMac - 1);
		for (fSoftBottom = fFalse; ilrT-- >= ilrFirst; lrp--)
			{
			fSoftBottom |= lrp->tSoftBottom == tPos;
			dylLine = max(dylLine, lrp->dyl - lrp->clFirst + dylBase);
			}
		if (!fSoftBottom && dylLine > dylFill)
			{
LTooTall:
			Assert(ilrMac >= ilrFirst);
			SetIMacPllr(plbs, ilrFirst);
			dylOverlapNew = plbs->dylOverlap + max(dylFill, max(dylLine, 0));
			cp = cpFirst;
			lbc = lbcYlLim;
			if (fLnn)
				plbs->lnn--;
			break;
			}

/* make the baselines align for this "line" */
		fGenLr = fTrue;	/* templates can be deleted */
		for (lrp++; ilrFirst++ < ilrMac; lrp++)
			{
			lrp->yl += dylLine - dylBase - (lrp->dyl - lrp->clFirst);
			lrp->clFirst = 0;
			}
		yl += dylLine;
		if (fEndChain)
			break;
		if ((dylFill -= dylLine) <= 0)
			{
			lbc = lbcYlLim;
			break;
			}
		}

/* delete template lrs */
LEndChain:
	vfli.fLayout = fFalse;
	if (fGenLr)
		for (clrDel = 0, ilr = plbs->ilrFirstChain; ilr != ilrNil; ilr = ilrNext - ++clrDel)
			{
			/* subtract 1 to adjust for deletion */
			lrp = LrpInPl(plbs->hpllr, ilr);
			ilrNext = lrp->ilrNextChain;
			Assert(ilrNext == ilrNil || ilrNext > ilr);
			DeleteFromPl(plbs->hpllr, ilr);
			if (ilrNext == ilrNil)
				break;
			}
	plbs->ilrFirstChain = ilrNil;
	plbs->yl = yl;
	plbs->cp = cp;
	plbs->cl = 0;
	plbs->dylOverlap = dylOverlapNew;
	plbs->fFirstPara = fFalse;      /* at least one done */
	return(lbc);
}


#ifndef PCWORD
/*******************************/
/* D x l   F r o m   T a b l e */
#ifdef WIN
EXPORT /* DxlFromTable */
#else /* !WIN */
NATIVE /* WINIGNORE - DxlFromTable - EXPORT if WIN */
#endif /* !WIN */
/* %%Function:DxlFromTable %%Owner:chrism */
DxlFromTable(ww, doc, cp)
int ww, doc;
CP cp;
{
/* returns width of a row; don't care about top/bottom borders */
/* munges caPara */
	struct WWD *pwwd = PwwdWw(ww);
	struct TCX tcx;

	CpFirstTap1(doc, cp, pwwd->fOutline);
	ItcGetTcx(ww, &vtapFetch, vtapFetch.itcMac, &tcx);
	return(tcx.xpCellLeft);
}


#endif /* !PCWORD */

/*********************************/
/* D y l   L r s   F o r   D o c */
#ifndef WIN
EXPORT /* WINIGNORE - !WIN */
#endif /* !WIN */
/* %%Function:DylLrsForDoc %%Owner:chrism */
int DylLrsForDoc(plbs)
struct LBS *plbs;
{
/* return amount of space actually used by a doc's lrs */
	int ilr, dyl = 0, ilrMac = IMacPllr(plbs);
	LRP lrp;

	for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; ilr++, lrp++)
		{
		if (lrp->doc == plbs->doc && lrp->ilrNextChain == ilrNil)
			dyl += min(plbs->yl - lrp->yl, lrp->dyl);
		}
	return(dyl);
}


#ifdef	DEBUG
#define	WHITENESS (DWORD)0x00FF0062

extern int wwCur;
extern struct SCI vsci;
extern struct SEL selCur;

/* %%Function:ShowLbsProc %%Owner:tomsax */
EXPORT ShowLbsProc(plbs, irtn)
struct LBS *plbs;
int irtn;
{
	int ilr, ilrMac, lrk;
	int dxw, dyw, dxe, dye;
	int docMother;
	struct WWD *pwwd;
	LRP lrp;
	HDC hdc;
	HWND hwnd;
	struct PLLR **hpllr;
	char *psz;
	struct RC rcw, rcl;
	struct DRC drcl;
	char szTemp[256];

	Assert(selCur.ww != wwNil);
	pwwd = PwwdWw(selCur.ww);
	/*GetDocAreaPrc(PmwdWw(selCur.ww), &rcw);*/
	rcw = pwwd->rcwDisp;
	++rcw.xwLeft;
	--rcw.ywBottom;
	hwnd = pwwd->hwnd;
	hdc = pwwd->hdc;

	docMother = DocMother(plbs->doc);
	dxe = NMultDiv(PdopDoc(docMother)->xaPage, vfli.dxuInch, dxaInch);
	dye = NMultDiv(PdopDoc(docMother)->yaPage, vfli.dyuInch, czaInch);
	dyw = DyOfRc(&rcw);
	dxw = NMultDiv(dyw,dxe,dye);
	rcw.xwRight = rcw.xwLeft + dxw;

	FrameRect(hdc, (LPRECT) &rcw, vsci.hbrBorder);
	rcl = rcw;
	++rcl.ywTop;
	++rcl.xwLeft;
	--rcl.ywBottom;
	--rcl.xwRight;
	PatBltRc(hdc,&rcl,WHITENESS);

	switch (irtn)
		{
	default:
		szTemp[0] = '0' + irtn/10;
		szTemp[1] = '0' + irtn % 10;
		szTemp[2] = 0;
		psz = szTemp;
		break;
	case irtnAssignLr:
		psz = SzFrame("FAssignLr");
		break;
	case irtnFormPara:
		psz = SzFrame("LbcFormatPara");
		break;
	case irtnFormChain:
		psz = SzFrame("LbcFormatChain");
		break;
		}

	TextOut(hdc, rcw.xwLeft+1, rcw.ywTop+1, (LPSTR)psz, CchSz(psz) - 1);

	hpllr = plbs->hpllr;
	ilrMac = hpllr == hNil ? 0 : IMacPllr(plbs);
	for (ilr = -1; ++ilr < ilrMac; )
		{
		lrp = LrpInPl(hpllr, ilr);
		drcl = lrp->drcl;
		lrk = lrp->lrk;
		DrcToRc(&drcl, &rcl);
		MutateRcl(&rcl, rcw.ywTop, rcw.xwLeft, dxw, dyw, dxe, dye);
		FrameRect(hdc, (LPRECT) &rcl, vsci.hbrBorder);
		if (lrk == lrkAbs)
			{
			MoveTo(hdc, rcl.xlLeft, rcl.ylTop);
			LineTo(hdc, rcl.xlRight, rcl.ylBottom);
			MoveTo(hdc, rcl.xlLeft, rcl.ylBottom);
			LineTo(hdc, rcl.xlRight, rcl.ylTop);
			}
		}
}


MutateRcl(prcl, ywTop, xwLeft, dxw, dyw, dxe, dye)
struct RC *prcl;
int ywTop, xwLeft, dxw, dyw, dxe, dye;
{
	PrcSet(prcl, 
			xwLeft + NMultDiv(prcl->xlLeft,dxw,dxe),
			ywTop + NMultDiv(prcl->ylTop,dyw,dye),
			xwLeft + NMultDiv(prcl->xlRight,dxw,dxe),
			ywTop + NMultDiv(prcl->ylBottom,dyw,dye));
}


#endif /*DEBUG*/
