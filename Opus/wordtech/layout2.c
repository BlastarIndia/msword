#define DEBUGORNOTWINLATER

/* layout2.c */
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
#include "code.h"
#include "font.h"
#include "layoutpc.h"
#include "strmsg.h"
#include "print.h"
#include "units.h"
#include "ww.h"
AssertDataCc("layout2.c")
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
#include "qsetjmp.h"
#endif /*MAC*/

#ifdef WIN
#include "heap.h"
#include "field.h"
#include "prompt.h"
#endif /* WIN */

#ifdef PROTOTYPE
#include "layout.cpt"
#endif /* PROTOTYPE */

#ifdef PCWORD
#define	vpapFetch	vpapAbs
#define vsepFetch	vsepAbs
#endif /* !PCWORD */

/* E X T E R N A L S */

extern uns              *pfgrMac;
extern int              vdocFetch;
extern struct CA        caPara;
extern struct CA        caParaL;
extern struct CA        caSect;
extern struct CA        caTap;
extern struct CA        caHdt;
extern struct PLC       **vhplcfrl;
extern struct FLS       vfls;
extern struct CHP       vchpStc;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct FLI       vfli;
extern struct FMTSS     vfmtss;
extern struct MERR      vmerr;
extern struct PREF      vpref;
extern int              vfBkgrndRepag;
extern BOOL             vfInFormatPage;
extern struct PLLR      **vhpllrAbs;
extern struct PLLR      **vhpllrSpare;
extern int              vfRestartLayout;
extern int              vlm;
extern ENV              venvLayout;

extern struct DBS       vdbs;
extern CP               vcpFirstLayout;
extern CP               vcpFetch;
extern struct DOD       **mpdochdod[];

#ifdef PCWORD
extern struct PAP       vpapStd;
extern struct HDT      	vrghdt[];        /* header descriptor table */
extern struct PLLBS    	**vhpllbs;       /* stacked LBS structures */
extern struct PRSU      vprsu;
extern struct PMD       vpmd;
#else
extern int				vdocTemp;
extern struct HDT       vrghdt[];
extern struct PLLBS     **vhpllbs;
#endif /* PCWORD */

#ifdef WIN
extern CP               vcpLimLayout;
extern struct FTI       vfti;
extern int              vflm;
#endif /* WIN */

#ifdef MAC
extern int		vfVisiSave;
extern int		vfAsPrintSave;
#endif


#ifndef PCWORD
struct PL *HplInit();
#endif


#ifdef DEBUGORNOTWIN
/*******************************/
/* C l	F o r m a t  L i n e s */
#ifndef WIN
/* %%Function:ClFormatLines %%Owner: NOTUSED */
NATIVE int ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, fRefLine, fStopAtPageBreak) /* WINIGNORE - "C_" in WIN */
#else /* WIN */
/* %%Function:C_ClFormatLines %%Owner:chrism */
HANDNATIVE int C_ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, fRefLine, fStopAtPageBreak)
#endif /* WIN */
struct LBS *plbs;
CP cpLim;
int dylFill;    /* space to fill */
int dylMax;     /* lines can go past dylFill but not dylMax */
int clLim;	/* 0 means cache is made to refer to plbs->cp */
int dxl;        /* width in which to format */
int fRefLine;	/* true means that line containing cpLim is required */
int fStopAtPageBreak;   /* care about page breaks? */
{
/* returns the cl of the line after the lines that fully fit into dylFIll
	with [cpFirst,cpLim) but not more than clLim.
	Updates at least the relevant elements in plcyl and
	updates clMac if end of para is reached in the process.
	Does not modify/advance plbs.
*/
	int ilnh, ilnh2, ilnhMac;
	struct PLC **hplclnh;
	int dyl, dylFirst;
	int dxa;
	int fBreakLast;
	int dylFixed;
	struct LNH lnh, lnhPrev;
	BOOL fNotDylFillMax = (dylFill != ylLarge);
	BOOL fNotDylMax = (dylMax != ylLarge);
	YLL dyllFill = fNotDylFillMax ? (YLL)dylFill : yllLarge;
	YLL dyllMax = fNotDylMax ? (YLL)dylMax : yllLarge;
	CP cp;

	StartProfile();

	Debug(CheckFlmState());
	hplclnh = vfls.hplclnh;
	CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
	if (!FInCa(plbs->doc, plbs->cp, &vfls.ca) || vfls.dxl != dxl ||
			plbs->cp < CpPlc(hplclnh, 0))
		{
		/* reset fls; no knowledge of para's height or lines */
LResetFls:
		vfls.ca = caParaL;
		if (cpLim != cpMax)
			vfls.ca.cpLim = CpMax(cpLim, caParaL.cpLim);
		vfls.ww = plbs->ww;
		vfls.clMac = clNil;
		vfls.dxl = dxl;
		vfls.fVolatile = vfls.fPageBreak = vfls.fBreakLast = fFalse;
		vfls.fOutline = plbs->fOutline;
		PutIMacPlc(hplclnh, 0);
		PutCpPlc(hplclnh, 0, plbs->cp);
		SetWords(&vfls.phe, 0, cwPHE);
		}
	if (clLim == 0)
		{
		StopProfile();
		return 0;  /* just wanted to establish cache */
		}

/* find cpFirst */
	vfls.fFirstColumn = plbs->fFirstColumn;
	CacheSectL(vfls.ca.doc, vfls.ca.cpFirst, plbs->fOutline);
	cpLim = CpMin(cpLim, vfls.ca.cpLim);

#ifdef PCWORD
	dxa = XaFromXl(dxl);
#else
	dxa = NMultDiv(dxl, dxaInch, WinMac(vfli.dxuInch, (vfli.fPrint) ? vfli.dxuInch : vfli.dxsInch));
#endif

	ilnh = IInPlcRef(hplclnh, plbs->cp);
	if (ilnh < 0)
		{
		FillLinesUntil(plbs, plbs->cp, yllLarge, clMax, dxa, fFalse, fFalse);
		ilnh = IInPlcRef(hplclnh, plbs->cp);
		Assert(ilnh >= 0);
		}
	if (ilnh >= clMaxLbs)
		goto LResetFls;

/* find cl beyond cpFirst */
	vfli.fLayout = fTrue;	/* splats like page view */
	ilnh += plbs->cl;
	if (ilnh > IMacPlc(hplclnh))
		{
		FillLinesUntil(plbs, cpLim, yllLarge, ilnh, dxa, fFalse, fFalse);
		/* this can happen when CpFromCpCl is called after editing */
		if (ilnh > (ilnhMac = IMacPlc(hplclnh)))
			goto LEndCl;
		}

/* now ilnh indicates start of line to start formatting; adjust dyllFill to be
	the space remaining - but don't count the lines of the para we won't be
	using */
	if (fNotDylFillMax)
		dyllFill -= plbs->yl - plbs->ylColumn;
	if (fNotDylMax)
		dyllMax -= plbs->yl - plbs->ylColumn;
	if (ilnh > 0)
		{
		GetPlc(hplclnh, ilnh - 1, &lnh);
		if (fNotDylFillMax)
			dyllFill += lnh.yll;
		if (fNotDylMax)
			dyllMax += lnh.yll;
		}

/* find dyllFill, clLim beyond */
	InvalFli();
	fBreakLast = vfls.fBreakLast = vfls.fEndBreak = fFalse;
	for (ilnhMac = IMacPlc(hplclnh); ; )
		{
		if (ilnh == ilnhMac)
			{
			if (fBreakLast)
				{
				/* last FillLinesUntil hit page break */
				vfls.fEndBreak = fTrue;
				break;
				}
			FillLinesUntil(plbs, cpLim, dyllMax, clLim, dxa, fStopAtPageBreak, fRefLine);
			fBreakLast = vfls.fBreakLast;
			if (ilnh == (ilnhMac = IMacPlc(hplclnh)))
				{
				if (fBreakLast)
					vfls.fEndBreak = fTrue;
				break;	/* FillLinesUntil hit a limit */
				}
			}
		if (!fRefLine && CpPlc(hplclnh, ilnh) >= cpLim)
			break;
		GetPlc(hplclnh, ilnh, &lnh);
		if (lnh.yll > dyllMax && !lnh.fSplatOnly)
			break;
		fBreakLast = vfls.fBreakLast = lnh.chBreak;
		cp = CpPlc(hplclnh, ++ilnh);
		if (cp > cpLim || !fRefLine && cp == cpLim)
			{
			if (ilnh == ilnhMac && vfls.fBreakLast)
				vfls.fEndBreak = fTrue;
			break;
			}
		if (fBreakLast && fStopAtPageBreak)
			{
			vfls.chBreak = lnh.chBreak;
			vfls.fEndBreak = fTrue;
			break;
			}
		if (lnh.yll >= dyllFill || ilnh - plbs->cl >= clLim)
			break;
		}
LEndCl:
	vfli.fLayout = fFalse;
	vfli.doc = docNil;

/* ilnh is one past the line with largest dyl <= dyllFill or largest
	cpLim <= cpLim */
/* if plc spans whole para, check for lines all same height */
	if (ilnh > 0 && vfls.clMac == clNil && !vfls.fPageBreak &&
			CpPlc(hplclnh, ilnhMac) == vfls.ca.cpLim)
		{
		vfls.clMac = ilnhMac;
		vfls.phe.w0 = 0;
		vfls.phe.clMac = -1;
		/* vfls.phe.fUnk = vfls.phe.fDiffLines = fFalse; */
		vfls.phe.dxaCol = XaFromXl(dxl);
		if (vfls.fVolatile || ilnh > vfls.phe.clMac)
			goto LDiffLines;
		GetPlc(hplclnh, 0, &lnhPrev);
		Assert(lnhPrev.yll < ylLarge);	/* the height of any single line
			should fit in an integer */
		dylFixed = (int)lnhPrev.yll -
			(lnhPrev.fDypBeforeAdded ? YlFromYa(vpapFetch.dyaBefore) : 0);
		for (ilnh2 = 1; ilnh2 < ilnhMac - 1; ilnh2++, lnhPrev = lnh)
			{
			GetPlc(hplclnh, ilnh2, &lnh);
			if (lnh.yll - lnhPrev.yll != dylFixed)
				goto LDiffLines;
			}
		/* ilnh2 is ilnhMac - 1 */
		dyl = YlFromYa(vpapFetch.dyaAfter);
		if (ilnhMac <= 1)
			dylFixed -= dyl;
		else
			{
			GetPlc(hplclnh, ilnh2, &lnh);
			Assert(lnh.yll - lnhPrev.yll < ylLarge);
			if ((int)(lnh.yll - lnhPrev.yll) - dyl != dylFixed)
				goto LDiffLines;
			}

		vfls.phe.clMac = vfls.clMac;
		vfls.phe.dylLine = IfWinElse(YaFromYl(dylFixed), dylFixed);
		}
	StopProfile();
	return(ilnh);

LDiffLines:
	GetPlc(hplclnh, ilnhMac - 1, &lnh);
	if (lnh.yll >= ylLarge)
		{
		SetWords(&vfls.phe, 0, cwPHE);
		}
	else
		{
		vfls.phe.dylHeight = IfWinElse(NMultDiv(lnh.yll, czaInch, vfli.dyuInch), 		(int)lnh.yll);
		vfls.phe.fDiffLines = fTrue;
		}

	StopProfile();
	return(ilnh);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/*********************************/
/* F i l l  L i n e s  U n t i l */
/* %%Function:FillLinesUntil %%Owner:chrism */
HANDNATIVE FillLinesUntil(plbs, cpLim, dyllFill, clLim, dxa, fStopAtPageBreak, fRefLine)
struct LBS *plbs;
CP cpLim;
YLL dyllFill;
int clLim;
int dxa;
int fStopAtPageBreak;
int fRefLine;
{
/*      add lines to plcyl in fls until
		clLim lines have been added in total
		next line would go over dyllFill
		next line would go over cpLim
		page break (conditionally)
*/
	int chSplat, fSplatOnly, fSeeSplat = fTrue, fAPO = fFalse;
	struct DOD *pdod;
	YLL dyll = 0;
	int ilnhMac;
	int ww = vfls.ww;
	CP cp;
	struct LNH lnh;

	StartProfile();
	Assert(cpLim <= vfls.ca.cpLim);
	Debug(CheckFlmState());
	cp = CpPlc(vfls.hplclnh, ilnhMac = IMacPlc(vfls.hplclnh));
	if (cp >= vfls.ca.cpLim)
		goto LEndFill;
#ifdef PCWORD
	if (cp < CpMacText(vfls.ca.doc) && FAbsPapM(vfls.ca.doc, &vpapFetch))
		fAPO = fTrue;
#else
	fAPO = FAbsPapM(vfls.ca.doc, &vpapFetch);
	pdod = PdodDoc(vfls.ca.doc);    /* two lines for compiler */
	if (pdod->fShort)
		{
		if (vflm == flmRepaginate || vflm == flmPrint)
			{
			ww = WinMac(wwLayout, wwTemp);
			LinkDocToWw(vfls.ca.doc, ww, wwNil);
			}
		fSeeSplat = fFalse;
		}
#endif /* !PCWORD */
	chSplat = fSplatOnly = fFalse;

	if (ilnhMac)
		{
		GetPlc(vfls.hplclnh, ilnhMac - 1, &lnh);
		dyll = lnh.yll;
		}

	while (ilnhMac < clLim)
		{
#ifdef WIN
		if (vfInFormatPage && vlm == lmBRepag && FAbortLayout(vfls.fOutline, plbs))
#else /* MAC */
		if (vfInFormatPage && vlm == lmBRepag && FAbortLayout(vfls.fOutline))
#endif /* !WIN */
			AbortLayout();
		FormatLineDxaL(ww, vfls.ca.doc, cp, dxa);
		if (vfli.fParaStopped && IfPCWordElse(ilnhMac , fTrue))
			{
			/* last line of para is hidden text, limit ourselves
				to para */
			PutCpPlc(vfls.hplclnh, ilnhMac, vfls.ca.cpLim);
			vfls.fVolatile = fTrue;
			goto LEndFill;
			}
		cp = vfli.cpMac;
		chSplat = fSplatOnly = fFalse;
		if (vfli.fSplatColumn)
			chSplat = chColumnBreak;
		else  if (vfli.fSplatBreak)
			chSplat = chSect;

		if (chSplat)
			{
			if (fSeeSplat)
				{
				fSplatOnly = IfPCWordElse(vfli.fSplatOnly, vfli.ichMac == 1);
				if (fAPO && !(fSplatOnly && vfli.cpMin == vfls.ca.cpFirst))
					fSplatOnly = chSplat = fFalse;
				else
					vfls.chBreak = chSplat;
				}
			else
				chSplat = fFalse; /* ignore splat in hdr, apo, etc. */
			}
		else  if (vfli.chBreak == chSect || vfli.chBreak == chColumnBreak)
			{
			/* we have to be sensitive to splats that are
				pending in order to detect end of section 
				properly */
			chSplat = vfli.chBreak;
			vfls.chBreak = vfli.chBreak;
			cp += ccpSect;
			}

		dyll += vfli.dypLine;
		if (!fSplatOnly)
			{
			if (dyll > dyllFill || (!chSplat && !fRefLine && cp > cpLim))
				goto LEndFill;
			}

		vfls.fBreakLast = fStopAtPageBreak && chSplat;
		vfls.fPageBreak |= chSplat;
		vfls.fVolatile |= vfli.fVolatile;

		if (fSplatOnly && ilnhMac > 0)
			{
			GetPlc(vfls.hplclnh, ilnhMac - 1, &lnh);
			if (!lnh.chBreak)
				{
				lnh.chBreak = chSplat;
				PutPlcLast(vfls.hplclnh, ilnhMac - 1, &lnh);
				dyll -= vfli.dypLine;
				goto LSetCp;
				}
			}

		lnh.fSplatOnly = fSplatOnly;
		lnh.chBreak = chSplat;
		lnh.fDypBeforeAdded = vfli.dypBefore != 0;
		lnh.yll = dyll;
		if (!FInsertInPlc(vfls.hplclnh, ilnhMac++, vfli.cpMin, &lnh))
			{
			if (vfInFormatPage)
				AbortLayout();
			goto LEndFill;
			}
LSetCp:
		PutCpPlc(vfls.hplclnh, ilnhMac, cp);
		vfls.ca.cpLim = CpMax(vfls.ca.cpLim, cp);
		if (dyll >= dyllFill || cp > cpLim || vfls.fBreakLast ||
				!fRefLine && cp == cpLim)
			{
			goto LEndFill;
			}
		}
LEndFill:
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/*************************/
/* F   A s s i g n   L r */
#ifndef WIN
/* %%Function:FAssignLr %%Owner:NOTUSED */
EXPORT int FAssignLr(plbs, dylFill, fEmptyOK) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_FAssignLr %%Owner:chrism */
HANDNATIVE int C_FAssignLr(plbs, dylFill, fEmptyOK)
#endif /* WIN */
struct LBS *plbs;
int dylFill, fEmptyOK;
{
/* create or assign a layout rectangle to the upcoming paragraphs;
	returns fFalse if height is already known */
	int ilr, ilrMac, ilrT;
	int doc = plbs->doc;
#ifndef PCWORD
	int docMother;
	int fFtn;
#endif /* !PCWORD */
	int fShort;
	struct DOD *pdod;
	int fAbs, fPend, fPrevTable;
	int xlRightAbs, xlLeftAbs;
	int xaLeft, xaRight;
	int dylOverlapNew;
	LRP lrp;
	LRP lrpT;
	struct PAP pap;
	struct RC rcl, rclPend;
	struct LR lr;
	struct LBS lbsT;
	int ihdt;

/* quick check - see if we can reuse previous lr */
	StartProfile();

	CacheParaL(doc, plbs->cp, plbs->fOutline);
#ifdef PCWORD
	/* CacheSectBody will cache the section of the footnote reference
		if we are in footnotes */
	CacheSectBody(doc, plbs->cp);
	fShort = plbs->cp > CpMacText(doc);
	fAbs = plbs->ihdt != ihdtTFtn && !fShort && FAbsPapM(doc, &vpapFetch);
#else
	pdod = PdodDoc(doc);
	fFtn = pdod->fFtn;
	fShort = pdod->fShort;
	fAbs = !fFtn && FAbsPapM(doc, &vpapFetch);
#endif /* !PCWORD */
	fPrevTable = fFalse;
	if (!fAbs && plbs->ilrCur >= 0 &&
			plbs->ilrCur < IMacPllr(plbs))
		{
		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
		if (lrp->doc == doc && lrp->ihdt == plbs->ihdt &&
				lrp->xl == plbs->xl)
			{
			if (lrp->lrs != lrsFrozen)
				{
				if (!plbs->fAbsPresent)
					{
					plbs->dylOverlap = 0;
					StopProfile();
					return(fFalse);
					}
				}
#ifndef PCWORD
			else  if (plbs->fSimpleLr && plbs->fPrevTable && vpapFetch.fInTable
					&& plbs->yl == lrp->yl + lrp->dyl)
				{
				plbs->yl -= plbs->dylBelowTable;
				lrp->dyl -= plbs->dylBelowTable;
				plbs->dylBelowTable = 0;
				fPrevTable = fTrue;
				}
#endif /* PCWORD */
			}
		}

#ifndef PCWORD
	/* get sect properties */
	if (fFtn)
		{
		docMother = DocMother(doc);
		CacheSectL(docMother, CpRefFromCpSub(doc, plbs->cp, edcDrpFtn), plbs->fOutline);
		}
	else
		CacheSectL(doc, plbs->cp, plbs->fOutline);
#endif /* !PCWORD */

/* find an existing LR to which we can assign the new text */
	if ((ilrMac = IMacPllr(plbs)) > 0)
		{
		for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; ilr++, lrp++)
			{
			/* doc and fAbs must match; ihdt match means both from same header or
				both not headers */
			if (lrp->doc != doc || lrp->ihdt != plbs->ihdt || lrp->lrs == lrsFrozen)
				continue;
			if (fAbs ^ lrp->lrk == lrkAbs)
				continue;
			/* if both abs, cp's must match */
			if (fAbs)
				{
				if (plbs->cp != lrp->cp)
					continue;
				/* reuse abs block */
				plbs->ilrCur = ilr;
				plbs->fPrevTable = fFalse;
				bltLrp(lrp, &lr, sizeof(struct LR));
				AssignAbsXl(plbs, &lr);
				bltLrp(&lr, lrp, sizeof(struct LR));
				plbs->dylOverlap = 0;
				StopProfile();
				return(fFalse);
				}
			/* check for different columns */
			if (lrp->lrk != lrkAbsHit)
				{
				if (plbs->xl != lrp->xl)
					continue;
				}
			else  if (plbs->xl > lrp->xl)
				/* apos can only force the xl to be bigger */
				continue;
			/* finally check for same section */
#ifdef PCWORD
			if (FInCa(lrp->doc, lrp->cp, &caSect))
				break;
#else
			if (fFtn)
				{
				if (FInCa(docMother, CpRefFromCpSub(lrp->doc, lrp->cp, edcDrpFtn), &caSect))
					break;
				}
			else  if (FInCa(lrp->doc, lrp->cp, &caSect))
				break;
#endif /* !PCWORD */
			}
		if (ilr < ilrMac)
			{
			plbs->ilrCur = ilr;
			plbs->dylOverlap = 0;
			StopProfile();
			return(fTrue);
			}
		}

/* need new LR */
	dylOverlapNew = 0;
	fPend = fFalse;
	plbs->fPrevTable = fPrevTable;
	plbs->ilrCur = ilrMac;
	plbs->ylMaxLr = plbs->ylMaxBlock;
	SetWords(&lr, 0, cwLR);
	lr.doc = doc;
	lr.yl = plbs->yl;       /* for abs, these are first approximations */
	lr.xl = plbs->xl;
	lr.dxl = plbs->dxlColumn;
	lr.ihdt = plbs->ihdt;
#ifdef PCWORD
	if (lr.ihdt == ihdtTFtn)
		lr.ihdt = ihdtNil;
#endif /* PCWORD */
	if (lr.ihdt == ihdtNil)
		lr.dyl = plbs->ylColumn + dylFill - lr.yl;
#ifdef WIN
	else
		lr.cpMacCounted = cpNil;
#endif /* WIN */
	lr.lnn = (fAbs || fShort || !FLnnSep(vsepFetch)) ? lnnNil : plbs->lnn;
	lr.ilrNextChain = ilrNil;
	lr.lrk = (fAbs) ? lrkAbs : lrkNormal;
	lr.tSoftBottom = tNeg;      /* ninch for exceed bottom */
#ifdef MAC
	DxsSetDxsInch(doc);
#endif /* MAC */

/* absolute object */
	if (fAbs)
		/* for abs. positioning, set cpFirst and cpLim now */
		SetAbsLr(plbs, &lr);

/* normal object, may be affected by absolutes */
	else
		{
		plbs->ylMaxLr = plbs->ylMaxBlock;
		plbs->dylChain = ylLarge;
		lr.cp = lr.cpLim = cpNil;
		if (plbs->fAbsPresent && lr.ihdt == ihdtNil)
			{
			/* restrict lr so it will miss abs objects */
			rcl.ylTop = lr.yl;
LFitLr:
			rcl.xlLeft = lr.xl;
			rcl.xlRight = lr.xl + lr.dxl;
			rcl.ylBottom = plbs->ylColumn + dylFill;
LHaveRcl:
			ConstrainToAbs(plbs, dylFill, &rcl, &rclPend, &fPend, &lr);
			if (rcl.ylBottom - plbs->yl <= plbs->dylOverlap ||
					rcl.ylBottom <= rcl.ylTop)
				{
				/* rectangle is too short */
				if (fPend)
					{
					/* ignore short one, use pending one */
					fPend = fFalse;
					rcl = rclPend;
					lr.fConstrainLeft = fTrue;
					lr.fConstrainRight = fFalse;
					goto LHaveRcl;
					}
				if (plbs->ilrFirstChain != ilrNil)
					goto LGiveUp;
				if (rcl.ylBottom >= plbs->ylColumn + dylFill)
					{
					/* avoid inf loop when no room by not flowing */
					if (!fEmptyOK)
						goto LTakeRcl;
					/* maybe only the last one is too short */
LGiveUp:
					plbs->ilrCur = ilrNil;
					if (rcl.ylBottom - rcl.ylTop > 0)
						plbs->dylChain = min(plbs->dylChain, rcl.ylBottom - rcl.ylTop);
					goto LMendChain;
					}
				/* retry with advanced yl */
				lr.lrk = lrkAbsHit;
				rcl.ylTop = rcl.ylBottom;
				lr.fConstrainLeft = lr.fConstrainRight = fFalse;
				dylOverlapNew = rcl.ylTop - plbs->yl;
				goto LFitLr;
				}

			if (lr.lrk == lrkAbsHit)
				{
LTakeRcl:
				lr.lrs = lrsIgnore;    /* in case too small */
				RcToDrc(&rcl, &lr.drcl);
				}
			}
		}
LReplace:
	ReplaceInPllr(plbs->hpllr, plbs->ilrCur, &lr);

	ShowLbs(plbs, irtnAssignLr/*irtn*/);

/* rectangle was split by an abs object - create or extend chain and redo fit
	for the new rectangle also */
LMendChain:
	if (fPend)
		{
		if (plbs->ilrFirstChain == ilrNil)
			plbs->ilrFirstChain = plbs->ilrCur;
		else
			{
			lrp = LrpInPl(plbs->hpllr, plbs->ilrCurChain);
			lrp->ilrNextChain = plbs->ilrCur;
			}
		plbs->ilrCurChain = plbs->ilrCur;
		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
		plbs->dylChain = min(plbs->dylChain, lrp->dyl);
		plbs->ilrCur = IMacPllr(plbs);
		fPend = fFalse;
		rcl = rclPend;
		lr.ilrNextChain = ilrNil;
		lr.fConstrainLeft = fTrue;
		lr.fConstrainRight = fFalse;
		goto LHaveRcl;
		}

/* end of chain - check for single entry, otherwise set height of all chained
	entries to be the min of all entries' heights */
	if (plbs->ilrFirstChain != ilrNil)
		{
		if (plbs->ilrCurChain == plbs->ilrFirstChain && plbs->ilrCur == ilrNil)
			{
			plbs->ilrCur = plbs->ilrFirstChain;
			plbs->ilrFirstChain = ilrNil;
			}
		else
			{
			if (plbs->ilrCur != ilrNil)
				{
				lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
				lrp->dyl = plbs->dylChain = min(plbs->dylChain, lrp->dyl);
				lrp->ilrNextChain = ilrNil;
				}
			for (ilr = plbs->ilrFirstChain; ; ilr = lrp->ilrNextChain)
				{
				lrp = LrpInPl(plbs->hpllr, ilr);
				lrp->dyl = plbs->dylChain;
				if (ilr == plbs->ilrCurChain)
					{
					/* next-to-last entry, chain in last */
					lrp->ilrNextChain = plbs->ilrCur;
					break;
					}
				}
			plbs->ilrCurChain = plbs->ilrCur = plbs->ilrFirstChain;
			}
		}
	plbs->dylOverlap = dylOverlapNew;
	StopProfile();
	return(fTrue);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/**************************************/
/* I f r d  G a t h e r  F t n  R e f */
#ifndef WIN
/* %%Function:IfrdGatherFtnRef %%Owner:NOTUSED */
NATIVE int IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_IfrdGatherFtnRef %%Owner:chrism */
HANDNATIVE int C_IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept)
#endif /* WIN */
struct LBS *plbs, *plbsNew;
int ifrd, fNormal, ylReject, ylAccept;
{
	struct DOD *pdod;
	int ifrl;
	struct PLC **hplcfrd;
	CP cpLimFnd, cpRef, cpMac = CpMacDocPlbs(plbs);
	LRP lrp;
	struct FRL frlNew;

	StartProfile();
	cpLimFnd = CpFromCpCl(plbsNew, fTrue);
	if ((frlNew.fNormal = fNormal) != fFalse)
		if (plbs->cp >= cpMac)
			{
			ylReject = ylAccept = 1;
			frlNew.fNormal = fFalse;
			}
		else
			{
			lrp = LrpInPl(plbsNew->hpllr, plbsNew->ilrCur);
			ylReject = max(plbs->yl, lrp->yl);
			ylAccept = plbsNew->yl;
			}
	frlNew.ylReject = ylReject;
	frlNew.ylAccept = ylAccept;

/* all refs between cp of lbs and lbsNew */
	pdod = PdodDoc(plbs->doc); /* two lines for compiler */
	hplcfrd = pdod->hplcfrd;
	for (ifrl = IMacPlc(vhplcfrl); (cpRef = CpPlc(hplcfrd, ifrd)) < cpLimFnd; ifrd++)
		{
		frlNew.ifnd = ifrd;
		if (!FInsertInPlc(vhplcfrl, ifrl++, cpRef, &frlNew))
			{
			Assert(vfInFormatPage);
			AbortLayout();
			}
		}
	StopProfile();
	return(ifrd);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/*********************************/
/* F   G e t   F t n   B r e a k */
#ifndef WIN
/* %%Function:FGetFtnBreak %%Owner:NOTUSED */
NATIVE int FGetFtnBreak(plbs, ifrl, pfrl, fpc) /* WINIGNORE "C_" if WIN */
#else /* WIN */
/* %%Function:C_FGetFtnBreak %%Owner:chrism */
HANDNATIVE int C_FGetFtnBreak(plbs, ifrl, pfrl, fpc)
#endif /* WIN */
struct LBS *plbs;
int ifrl;	/* negative when we're recursing; ifrl = abs(ifrl) */
struct FRL *pfrl;
int fpc;
{
/* call ClFormatLines to determine what line contains a footnote reference,
	set frl accordingly; returns fFalse if no better information available */
	int ilr, ilrMac = IMacPllr(plbs), ilrBest = -1;
	int fRecurse;
	LRP lrp;
	int fWidow, ilnhMac;
	struct PLC **hplclnh = vfls.hplclnh;
	int cl, clFirst, clRef;
	YLL dyllPrevCol;
	int ylAccept, ylReject;
	CP cpRef, dcp, dcpBest = cpMax;
	CP cpLim;
	struct DOD *pdod;
	struct PLC **hplcfrd;
	struct LNH lnh;
	struct LR lr;
	struct FRL frl, frlT;
	struct LBS lbsT;

/* find the lr containing the reference */
	StartProfile();
	if (fpc == fpcEndnote || fpc == fpcEndDoc)
		goto LNoInfo;	/* endnotes -- essentially no reference */
	if (fRecurse = (ifrl < 0))
		ifrl = -ifrl;
	cpRef = CpPlc(vhplcfrl, ifrl);

	for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; lrp++, ilr++)
		{
		if (lrp->doc != plbs->doc || cpRef < lrp->cp)
			continue;
		cpLim = lrp->cpLim;
		if (lrp->clLim != 0)
			{
			lbsT = *plbs;
			lbsT.cp = cpLim;
			lbsT.cl = lrp->clLim;
			cpLim = CpFromCpCl(&lbsT, fTrue);
			lrp = LrpInPl(plbs->hpllr, ilr);
			}
		if (cpRef >= cpLim)
			continue;
		if ((dcp = cpRef - lrp->cp) < dcpBest)
			{
			ilrBest = ilr;
			dcpBest = dcp;
			}
		}
	Assert(ilrBest != -1);
	if (ilrBest == -1)
		goto LNoInfo;
	lrp = LrpInPl(plbs->hpllr, ilrBest);
	if (lrp->lrk == lrkAbs)
		goto LNoInfo;
	lr = *lrp;

/* set up LBS */
	lbsT = *plbs;
	lbsT.cp = cpRef;
	CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
	/* find cp/cl at which to start formatting */
	if (lr.cp >= caParaL.cpFirst)
		{
		lbsT.cp = lr.cp;
		lbsT.cl = lr.clFirst;
		}
	else
		{
		lbsT.cp = caParaL.cpFirst;
		lbsT.cl = 0;
		}
	GetPlc(vhplcfrl, ifrl, &frl);    /* NOT the same as *pfrl */
	lbsT.yl = frl.ylReject;

/* calculate above/below based on line with ref; to do widow control we
	need entire para */
	pdod = PdodDoc(lbsT.doc);
	fWidow = pdod->dop.fWidowControl;
	hplcfrd = pdod->hplcfrd;
	ClFormatLines(&lbsT, (fWidow) ? caParaL.cpLim : cpRef,
			ylLarge, ylLarge, clMax, lr.dxl, !fWidow, fFalse);
	Assert(IMacPlc(hplclnh) != 0);
	Assert(cpRef < CpPlc(hplclnh, IMacPlc(hplclnh)));
	/* we have to adjust for lines that have already been used in a
		previous column or page */
	if ((clFirst = lbsT.cl + IInPlc(hplclnh, lbsT.cp)) == 0)
		dyllPrevCol = (YLL)0;
	else
		{
		GetPlc(hplclnh, clFirst - 1, &lnh);
		dyllPrevCol = lnh.yll;
		}
	cl = clRef = IInPlc(hplclnh, cpRef);  /* ref is on line cl + 1 */
	ylReject = lbsT.yl;
	if (cl != clFirst && !(cl == 1 && fWidow))
		{
		GetPlc(hplclnh, cl - 1, &lnh);
		Assert(lnh.yll - dyllPrevCol < ylLarge);
		ylReject = (int)(lnh.yll - dyllPrevCol) + lbsT.yl;
		}
	if (fWidow && CpPlc(hplclnh, ilnhMac = IMacPlc(hplclnh)) == vfls.ca.cpLim)
		{
		if (ilnhMac < 4)
			{
			/* need whole para */
			ylAccept = frl.ylAccept;
			ylReject = frl.ylReject;
			goto LCheckFirst;
			}
		if (cl == ilnhMac - 2 || (cl == 0 && vfls.ca.cpFirst == caParaL.cpFirst))
			cl++; /* reference in first or next-to-last line */
		}
	GetPlc(hplclnh, cl, &lnh);
	Assert(lnh.yll - dyllPrevCol < ylLarge);
	ylAccept = (int)(lnh.yll - dyllPrevCol) + lbsT.yl;

/* if not first footnote on line, we can't reject it */
LCheckFirst:
	if (!fRecurse)
		if (cpRef > CpPlc(hplcfrd, 0) &&
				CpPlc(hplclnh, clRef) <= CpPlc(hplcfrd, IInPlc(hplcfrd, cpRef - 1)))
			{
			ylReject = ylAccept;
			}
		else  if (ifrl > 0)
			{
			/* if reject point same as previous footnote, we can't reject it */
			frlT = frl;
			FGetFtnBreak(plbs, -(ifrl - 1), &frlT, fpc);
			if (ylReject == frlT.ylReject)
				ylReject = ylAccept;
			}

	if (ylAccept == frl.ylAccept && ylReject == frl.ylReject)
		{
LNoInfo:
		StopProfile();
		return(fFalse);
		}
	pfrl->ylAccept = ylAccept;
	pfrl->ylReject = ylReject;
	StopProfile();
	return(fTrue);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/*************************/
/* C o p y  H d t  L r s */
#ifndef WIN
/* %%Function:CopyHdtLrs %%Owner:NOTUSED */
NATIVE CopyHdtLrs(ihdt, plbs, yl, plbsAbs) /* WINIGNORE "C_" if WIN */
#else /* WIN */
/* %%Function:C_CopyHdtLrs %%Owner:chrism */
HANDNATIVE C_CopyHdtLrs(ihdt, plbs, yl, plbsAbs)
#endif /* WIN */
int ihdt;
struct LBS *plbs;
int yl;
struct LBS *plbsAbs;	/* reference for abs objects */
{
/* copy some header LRs, if any, onto a page. Footnote separators must have
	a non-zero height to be used, but headers can be zero height if they are
	entirely composed of absolute objects */
	int xl = -1;
	struct HDT *phdt = &vrghdt[ihdt];
	struct LBS lbsT;
	LRP lrp;

	StartProfile();
	if (phdt->hpllr == hNil)
		{
		StopProfile();
		return;
		}
	if (ihdt >= ihdtMaxSep)
		{
		if (phdt->dyl == 0)
			{
			StopProfile();
			return;
			}
		if (plbsAbs == 0 || !plbsAbs->fAbsPresent)
			xl = plbs->xl;
		else
			{
			/* get correct xl, yl by requesting an lr */
			PushLbs(plbsAbs, &lbsT);
			lbsT.yl = lbsT.ylColumn = yl;
#ifdef PCWORD
			lbsT.ihdt = ihdtTFtn;
#else
			lbsT.doc = docNew;
#endif /* PCWORD */
			lbsT.ilrFirstChain = ilrNil;
			lbsT.cp = cp0;
			lbsT.cl = lbsT.dylOverlap = 0;
			FAssignLr(&lbsT, phdt->dyl, fFalse);
			xl = plbs->xl;
			if (lbsT.ilrCur != iNil)
				{
				lrp = LrpInPl(lbsT.hpllr, lbsT.ilrCur);
				if (lrp->dyl != 0)
					{
					xl = lrp->xl;
					yl = lrp->yl;
					}
				}
			PopLbs(&lbsT, 0);
			}
		plbs->dylUsedCol += phdt->dyl;
		plbs->dyllTotal = phdt->dyl + plbs->dyllTotal;
		plbs->clTotal++; /* assume 1 line; it's indivisible */
		}
	CopyLrs(phdt->hpllr, plbs, (*phdt->hpllr)->ilrMac, yl, fFalse, xl);
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/******************/
/* C o p y  L r s */
#ifndef WIN
/* %%Function:CopyLrs %%Owner:NOTUSED */
NATIVE CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_CopyLrs %%Owner:chrism */
HANDNATIVE C_CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl)
#endif /* WIN */
struct PLLR **hpllrFrom;
struct LBS *plbsTo;
int ilrMac;	/* number to copy */
int yl; /* yl for top of LRs */
int fCopyIgnore;
int xl;         /* use -1 to maintain xl */
{
/* copy LRs from one LR plex to another, forcing them to live at yl height */
	int ilrFrom, ilrTo, fFirst;
	int ylFirst;
	struct LR lrT;

	StartProfile();
	if (ilrMac <= 0)
		{
		StopProfile();
		return;
		}
	fFirst = fTrue;
	for (ilrFrom = 0, ilrTo = IMacPllr(plbsTo); ilrFrom < ilrMac; ilrFrom++)
		{
		/* have to copy: heap moves */
		bltLrp(LrpInPl(hpllrFrom, ilrFrom), &lrT, sizeof(struct LR));
		if (lrT.lrs == lrsIgnore && !fCopyIgnore)
			continue;
		if (lrT.lrk == lrkAbs)
			{
			plbsTo->fAbsPresent = fTrue;
			if (lrT.ihdt != ihdtNil && lrT.fInline && yl != ylLarge)
				{
				/* need to position inline header APO */
				lrT.yl += yl - ((fFirst) ? 0 : ylFirst);
				}
			}
		else
			{
			if (yl != ylLarge)
				{
				/* displace to required yl */
				if (fFirst)
					{
					ylFirst = lrT.yl;
					fFirst = fFalse;
					}
				lrT.yl += yl - ylFirst;
				plbsTo->yl = lrT.yl + lrT.dyl;
				}
			if (xl >= 0)
				lrT.xl = xl;
			}
		ReplaceInPllr(plbsTo->hpllr, ilrTo++, &lrT);
		}
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/*******************************/
/* R e p l a c e  I n  P l l r */
#ifndef WIN
/* %%Function:ReplaceInPllr %%Owner:NOTUSED */
NATIVE ReplaceInPllr(hpllr, ilr, plr) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_ReplaceInPllr %%Owner:chrism */
HANDNATIVE C_ReplaceInPllr(hpllr, ilr, plr)
#endif /* WIN */
struct PLLR **hpllr;
int ilr;
struct LR *plr;
{
	struct PLLR *ppllr = *hpllr;

	StartProfile();
	if (ilr < ppllr->ilrMax)
		{
		ppllr->ilrMac = max(ilr + 1, ppllr->ilrMac);
		bltLrp(plr, LrpInPl(hpllr, ilr), sizeof(struct LR));
		}
	else  if (!FInsertInPl(hpllr, ilr, plr))
		{
		Assert(vfInFormatPage);
		AbortLayout();
		}
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifndef PCWORD
#ifdef DEBUGORNOTWIN
/*************************/
/* C a c h e  P a r a  L */
#ifdef MAC
/* %%Function:CacheParaL %%Owner:NOTUSED */
NATIVE CacheParaL(doc, cp, fOutline) /* WINIGNORE "C_" if WIN */
#else /* WIN */
/* %%Function:C_CacheParaL %%Owner:chrism */
HANDNATIVE C_CacheParaL(doc, cp, fOutline)
#endif /* WIN */
int doc;
CP cp;
int fOutline;
{
/* cache current paragraph and, if fOutline, kill all properties that should
	not be expressed in outline mode */

	StartProfile();
	if (!FInCa(doc, cp, &caParaL) || caParaL.doc != caPara.doc || 
			caPara.cpFirst < caParaL.cpFirst || caPara.cpLim > caParaL.cpLim)
		{
#ifdef WIN
		int ccp;

		caParaL.doc = doc;
		LinkDocToWw(doc, wwLayout, wwNil);
		GetCpFirstCpLimDisplayPara(wwLayout, doc, cp, &caParaL.cpFirst, &caParaL.cpLim);
		/* get correct para props in vpap */
		FetchCpPccpVisible(doc, caParaL.cpFirst, &ccp, wwLayout /* fvc */, 0 /* ffe */);
		/* now vcpFetch is first visible char after caParaL.cpFirst and
		   caPara contains vcpFetch; make sure apo's or something didn't
		   cause caParaL to stop short of a visible character */
		if (vcpFetch >= caParaL.cpLim)
			CachePara(doc, caParaL.cpFirst);
#else
		CachePara(doc, cp);
		caParaL = caPara;
#endif

#ifndef JR
		if (fOutline)
			{
			vpapFetch.fSideBySide = vpapFetch.fKeep =
					vpapFetch.fKeepFollow = vpapFetch.fPageBreakBefore = fFalse;
			if (!vpapFetch.fInTable)
				{
				vpapFetch.brcp = brcpNone;
				vpapFetch.dyaLine = vpapFetch.dyaBefore = vpapFetch.dyaAfter = 0;
				}
			vpapFetch.dxaWidth = vpapFetch.dxaAbs = vpapFetch.dyaAbs = 0;
			vpapFetch.pc = 0;
			}
#endif /* !JR */
		}
}


#endif /* DEBUGORNOTWIN */
#endif /* !PCWORD */


#ifndef PCWORD
#ifdef DEBUGORNOTWIN
/*************************/
/* C a c h e  S e c t  L */
#ifdef MAC
/* %%Function:CacheSectL %%Owner:NOTUSED */
NATIVE CacheSectL(doc, cp, fOutline) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_CacheSectL %%Owner:chrism */
HANDNATIVE C_CacheSectL(doc, cp, fOutline)
#endif /* WIN */
int doc;
CP cp;
int fOutline;
{
/* cache current section and, if fOutline, kill all properties that should
	not be expressed in outline mode */
	struct DOP *pdop;

/* MacWord 1.05 conversions and PCWord docs can have last
	character of doc == chSect, so back off */
	StartProfile();
	cp = CpMin(cp, CpMacDocEditL(doc, cp));
	if (!FInCa(doc, cp, &caSect))
		{
		CacheSect(doc, cp);
#ifndef JR
		if (fOutline)
			{
			vsepFetch.bkc = bkcNewPage;
			vsepFetch.ccolM1 = vsepFetch.nLnnMod = 0;
#ifdef WIN
			vsepFetch.vjc = vjcTop;
#endif
			pdop = &PdodMother(doc)->dop;
			vsepFetch.dxaColumnWidth = pdop->xaPage
					- pdop->dxaLeft - pdop->dxaRight
					- pdop->dxaGutter;
			}
#endif /* !JR */
		}
}


#endif /* DEBUGORNOTWIN */
#endif /* !PCWORD */


#ifdef DEBUGORNOTWIN
/*******************/
/* P u s h   L b s */
#ifndef WIN
/* %%Function:PushLbs %%Owner:NOTUSED */
NATIVE PushLbs(plbsFrom, plbsTo) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_PushLbs %%Owner:chrism */
HANDNATIVE C_PushLbs(plbsFrom, plbsTo)
#endif /* WIN */
struct LBS *plbsFrom, *plbsTo;
{
/* saves layout state by copying lbs and its hpllr; the lbs is copied to
	the target and to the lbs stack for memory cleanup; pop or copy will free
	the stack entry by comparing hpllr */
	int ilbs = (*vhpllbs)->ilbsMac;
	int ilrMac;
	struct LBS lbsT;

	StartProfile();
	Assert(vfInFormatPage);
	Assert(plbsTo != 0 && plbsFrom != 0);
	ilrMac = IMacPllr(plbsFrom);
	*plbsTo = *plbsFrom;

	if ((plbsTo->hpllr = vhpllrSpare) != hNil)
		{
		if ((*vhpllrSpare)->ilrMax < ilrMac)
			{
#ifdef WIN
			struct PL *ppl = *vhpllrSpare;
			HQ hq = *((HQ *)(((char *)ppl) + ppl->brgfoo));

			Assert(ppl->fExternal);
			/* FChngSizePhqLcb may cause heap movement! */
			if (!FChngSizePhqLcb(&hq, (long)((ilrMac + 1) * sizeof(struct LR))))
				{
				vhpllrSpare = 0;
				goto LFreeAbort;
				}
			ppl = *vhpllrSpare;
			*((HQ *)(((char *)ppl) + ppl->brgfoo)) = hq;
#else /* !WIN */
			Mac(Assert(!(*vhpllrSpare)->fExternal));
			if (!FChngSizeHCw(vhpllrSpare,
					(*vhpllrSpare)->brglr / sizeof(int)
					+ (ilrMac + 1) * cwLR, fFalse))
				{
				vhpllrSpare = 0;
				goto LFreeAbort;
				}
#endif /* !WIN */
			(*vhpllrSpare)->ilrMax = ilrMac + 1;
			}
		vhpllrSpare = hNil;
		}
	else  if ((plbsTo->hpllr = HpllrInit(sizeof(struct LR), max(3, ilrMac))) == hNil)
		{
		AbortLayout();
		}

	if (!FInsertInPl(vhpllbs, ilbs, plbsTo))
		{
LFreeAbort:
		FreePhpl(&plbsTo->hpllr);
		AbortLayout();
		}
	plbsTo->fOnLbsStack = fTrue;
	SetIMacPllr(plbsTo, ilrMac);
	if (ilrMac > 0)
		{
		bltLrp(LrpInPl(plbsFrom->hpllr, 0), LrpInPl(plbsTo->hpllr, 0),
				ilrMac * sizeof(struct LR));
		}
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/****************/
/* P o p  L b s */
#ifndef WIN
/* %%Function:PopLbs %%Owner:NOTUSED */
NATIVE int PopLbs(plbsId, plbsTo) /* WINIGNORE - "C_ if WIN */
#else /* WIN */
/* %%Function:C_PopLbs %%Owner:chrism */
HANDNATIVE int C_PopLbs(plbsId, plbsTo)
#endif /* WIN */
struct LBS *plbsId, *plbsTo;
{
/* removes the entries above the one whose hpllr matches plbsId's; copies
	the matching stack entry to plbsTo if not zero; pass zero for plbsId to
	free all of lbs stack */
	int ilbsTop;
	struct LBS *plbsTop;
	struct LBS lbsT;

	StartProfile();
	if (vhpllbs == hNil)
		{
		Assert(plbsId == 0 && plbsTo == 0);
		StopProfile();
		return;
		}

	if (plbsId == 0)
		(plbsId = &lbsT)->hpllr = hNil;

/* free hpllr from target lbs */
	if (plbsTo != 0)
		CopyLbs(0, plbsTo);

/* pop all higher entries in vhpllbs */
	ilbsTop = (*vhpllbs)->ilbsMac - 1;
	if (ilbsTop >= 0)
		for (plbsTop = PInPl(vhpllbs, ilbsTop);
				ilbsTop >= 0 && plbsTop->hpllr != plbsId->hpllr;
				--ilbsTop, --plbsTop)
			{
			if (vhpllrSpare == hNil)
				vhpllrSpare = plbsTop->hpllr;
			else
				FreePhpl(&plbsTop->hpllr);
			}

/* pop the entry */
	if (ilbsTop < 0)
		ilbsTop = 0;
	else  if (plbsTo != 0)
		blt(plbsTop, plbsTo, cwLBS);
	else  if (vhpllrSpare == hNil)
		vhpllrSpare = plbsTop->hpllr;
	else
		FreePhpl(&plbsTop->hpllr);
	(*vhpllbs)->ilbsMac = ilbsTop;
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/******************/
/* C o p y  L b s */
#ifndef WIN
/* %%Function:CopyLbs %%Owner:NOTUSED */
NATIVE CopyLbs(plbsFrom, plbsTo) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
/* %%Function:C_CopyLbs %%Owner:chrism */
HANDNATIVE C_CopyLbs(plbsFrom, plbsTo)
#endif /* WIN */
struct LBS *plbsFrom, *plbsTo;
{
/* copies lbsFrom to lbsTo; lbsTo's original hpllr is freed. If an lbs stack
	entry has the same hpllr, that stack entry is deleted */
	StartProfile();
	Assert(plbsTo != 0);
	UnstackLbs(plbsTo);
	if (vhpllrSpare == hNil)
		vhpllrSpare = plbsTo->hpllr;
	else
		FreePhpl(&plbsTo->hpllr);
	if (plbsFrom == 0)
		plbsTo->hpllr = 0;
	else
		{
		*plbsTo = *plbsFrom;
		plbsFrom->hpllr = 0;
		UnstackLbs(plbsTo);
		}
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN
/*************************/
/* U n s t a c k   L b s */
/* %%Function:UnstackLbs %%Owner:chrism */
NATIVE UnstackLbs(plbs) /* WINIGNORE - near assembler in WIN */
struct LBS *plbs;
{
/* removes from the lbs stack the first entry whose hpllr matches plbs's */
	int ilbs;
	struct LBS *plbsStack;

	StartProfile();
	if (plbs == 0 || !plbs->fOnLbsStack || (ilbs = (*vhpllbs)->ilbsMac - 1) < 0)
		{
		StopProfile();
		return;
		}

	plbs->fOnLbsStack = fFalse;
	for (plbsStack = PInPl(vhpllbs, ilbs); ilbs >= 0; --plbsStack, --ilbs)
		if (plbsStack->hpllr == plbs->hpllr)
			{
			DeleteFromPl(vhpllbs, ilbs);
			StopProfile();
			return;
			}
	StopProfile();
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC
/*******************************/
/* F   A b o r t   L a y o u t */
/* %%Function:FAbortLayout %%Owner:NOTUSED */
NATIVE FAbortLayout(fOutline) /* WINIGNORE - Mac version */
int fOutline;
{
	int lmSave, fAbort;
	struct CA caSave;

	if (vlm == lmBRepag)
		{
		caSave = caPara;
		lmSave = vlm;
		vlm = lmNil;
		vfli.fStopAtPara = fFalse;	/* restore */
		InvalFli();
		vpref.fShowP = vfVisiSave;
		vfli.fFormatAsPrint = vfAsPrintSave;
		IdleTrack(fTrue);	/* OK to call format */
		vpref.fShowP = fFalse;
		vfli.fFormatAsPrint = fTrue;
		InvalFli();
		vfli.fStopAtPara = fTrue;	/* never exceed para bounds */
		vlm = lmSave;
		fAbort = FMsgPresent(mtyBRepag);
		if (!fAbort && FNeRgw(&caSave, &caPara, cwCA))
			{
			caParaL.doc = docNil;
			CacheParaL(caSave.doc, caSave.cpFirst, fOutline);
			}
		return(fAbort);
		}
	return((vlm == lmPagevw || vlm == lmPreview) ? fFalse : FCancel());
}


#endif /* MAC */


#ifdef WIN
#ifdef DEBUG
/* F   A b o r t   L a y o u t */
/* %%Function:C_FAbortLayout %%Owner:chrism */
HANDNATIVE C_FAbortLayout(fOutline, plbs)
int fOutline;
struct LBS *plbs;
{
	BOOL fAbort, fOutlineWw, fLayoutSav;
	struct CA caSave;
	int flmSave = vflm;
	int lmSave = vlm;
#ifdef DEBUG
	CP cpFirstSave = vcpFirstLayout;
	CP cpLimSave = vcpLimLayout;
#endif

#ifdef DEBUG
	if (vdbs.cBkgndSucceed)
		{
		vdbs.cBkgndSucceed--;
		return fFalse;
		}
	else  if (vdbs.cBkgndFail)
		{
		vdbs.cBkgndFail--;
		return fTrue;
		}
#endif /* DEBUG */

	caSave = caPara;

	fOutlineWw = PwwdWw(wwLayout)->fOutline;

	/* So that any display routines called will work */
	fLayoutSav = vfli.fLayout;
	vfli.fLayout = fFalse;
	fAbort = (vlm == lmPagevw) ? fFalse : (vlm == lmBRepag) ?
			FMsgPresent(mtyIdle) : FQueryAbortCheck();
	vfli.fLayout = fLayoutSav;

	fAbort = fAbort || (vflm != flmSave && !FResetWwLayout(DocMotherLayout(plbs->doc), flmSave, lmSave));
	PwwdWw(wwLayout)->fOutline = fOutlineWw;

/* processing some messages should not change vcpFirstLayout */
	Assert(vcpFirstLayout == cpFirstSave);
	Assert(vcpLimLayout == cpLimSave);
	/* Abort checks can change caPara...not a good thing! */
	/* Yes...use caPara, not caParaL */
	if (!fAbort && FNeRgw(&caSave, &caPara, cwCA))
		{
		caParaL.doc = docNil;
		if (caSave.doc != docNil)
			CacheParaL(caSave.doc, caSave.cpFirst, fOutline);
		}
	return fAbort;
}


#endif /* DEBUG */
#endif /* WIN */
