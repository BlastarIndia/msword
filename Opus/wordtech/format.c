/* F O R M A T . C */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "doc.h"
#include "props.h"
#include "border.h"
#include "format.h"
#include "formula.h"
#include "sel.h"
#include "ch.h"
#include "inter.h"
#include "pic.h"
#include "print.h"
#include "debug.h"
#include "compare.h"
#include "screen.h"

#ifdef MAC
#define FONTS
#define WINDOWS
#define DIALOGS
#include "hyph.h"
#include "toolbox.h"
#include "font.h"
#include "printMac.h"
#else
#include "field.h"
#define REVMARKING
#include "compare.h"
#include "resource.h"
#include "rareflag.h"

#undef DxpFromCh
#undef DxuExpand
#define DxpFromCh	C_DxpFromCh
#define DxuExpand	C_DxuExpand
#endif /* WIN */

#ifdef MAC

#define FSeeHiddenFli           (vfli.fSeeHidden)
#define FVisiSpacesFli          (vfli.fvisiSpaces)
#define FVisiParaMarksFli       (vfli.fvisiParaMarks)
#define FVisiCondHyphensFli     (vfli.fvisiCondHyphens)
#define XtFromXa(xa)		NMultDiv((xa), vfli.dxuInch, dxaInch)

#endif /* MAC */

#ifdef WIN
#define FSeeHiddenFli		(vfli.grpfvisi.fSeeHidden | vfli.grpfvisi.fvisiShowAll)
#define FVisiParaMarksFli	(vfli.grpfvisi.fvisiParaMarks | vfli.grpfvisi.fvisiShowAll)
#define FVisiCondHyphensFli	(vfli.grpfvisi.fvisiCondHyphens | vfli.grpfvisi.fvisiShowAll)
#define FVisiSpacesFli		(vfli.grpfvisi.fvisiSpaces | vfli.grpfvisi.fvisiShowAll)
#define XtFromXa(xa)		NMultDiv((xa), vftiDxt.dxpInch, dxaInch)

#endif /* WIN */

/* E X T E R N A L S */
extern char             (**vhgrpchr)[];
extern int              vbchrMax;
extern int              vbchrMac;
extern int              vlm;
extern struct WWD       **mpwwhwwd[];
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CHP       vchpFetch;
extern struct SEP       vsepFetch;
extern struct PAP       vpapFetch;
extern CP               vcpFetch;
extern char HUGE        *vhpchFetch;
extern uns              vccpFetch;
extern struct TCC       vtcc;
extern int              fnFetch;
extern int		vdocFetch;
extern int		vdocTemp;
extern struct DOD       **mpdochdod[];
extern struct PREF      vpref;
extern struct ITR       vitr;
extern struct FLI       vfli;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct FMTSS     vfmtss;
extern int              vifmaMac;
extern struct PIC       vpicFetch;
extern struct PRDD      vprdd;
extern struct FMA	rgfma[];

#ifdef MAC
extern int far * far    *vqqmpchdxu;
extern int far * far    *vqqfontCur;
extern int		vdxuFrac;
/* current font cache */
extern int              ftcLast;
extern int              catrLast;
extern int              psLast;
extern BOOL             fWidthOK; /* means vqqfontCur matches ftcLast etc. */
/* saved ftc's for daisy substitution */
extern int              viftcMac;
extern int              cHpFreeze;
extern BOOL		vfLargeSystem;
extern struct PRSU	vprsu;
extern int		vfHyphCapitals;
extern int		vfColorQD;
#endif

#ifdef WIN
extern struct ffs	*vpffs;
extern int		vflm;
extern struct SCI       vsci;
extern struct RF	vrf;
#endif /* WIN */

#ifdef DEBUG
extern struct DBS vdbs;
#endif /* DEBUG */



/* F O R M A T  L I N E */
/* formats line in doc starting at cp.
	Results are in fli.
Parameters are doc, cp to be formatted + mode flags in fli.

	fPrint=fTrue    if set, xp's in fli are in "device units".
			conversion factor is in dxuInch = the units in
			1 inch or dxaInch = 1440 xa's.
			yp's will be in points as usual. (factor: dyaPoint = 20)
	fPrint=fFalse   otherwise xp's in fli will be in screen units.
		if fFormatAsPrint
			the line break is computed as if fPrint were true.
			but fli is returned in screen units with an even
			conversion factor of dxsInch (72 or 80)
		else
			line break computation AND fli are computed using
			dxsInch.

		vpref flags:
					fvisiTabs means tabs are visible
					fvisiSpaces means spaces are visible
					fvisiParaMarks means end of paragraph marks are visible
					fvisiDivMarks means end of division marks are visible
					fvisiCondHyphens means conditional hyphens are visible

The end of the document must be detected outside of this procedure.

New features:
	heights of non-printing chars will not affect auto line height
	special char/picture formatting
	justify is faster. spaces beyond the right margin will have normal
		width. Initial spaces in the line will not be justified.
	invisible character format supported
	new right margin and tab modes supported
	letterspacing supported
	better memory use and toleration of out-of-memory.


Here is a map of the procedure:

FormatLine()

	determine margins from pap (and sep)
	initialize
	goto LFetch

	for (;;)
		{
		if end of run then goto LNewRun;
		ch = next char in current run
LHaveCh:
		rgch[ich] = ch;
		calculate width of ch: dxp to be stored in rgdxp
			and dxt to be used to check for breaks.
		fPrint    !fPrint & !fFormatAsPrint  !fPrint & fFormatAsPrint
dxp           res/heap          res                       res
	units:  device            screen                    screen
dxt           res/heap          res                       resDxt/heap
	units:  device            screen                    device
		where res is the resource set up by LoadFont, heap is a
		width table in the heap also set up by LoadFont.

		The width calculation includes letterspacing.
LHaveDxp:
		xp += rgdxp[ich++] = dxp;
		xt += dxt;

		treat exceptional characters (see below)
			non required hyphens
			hyphen (line break point)
			space
			end of line
			end of para
			section/page mark
			tab

		normal characters:
		if (xt > right margin (True units))
LBreak:
			this last character does not fit
			if there are no break opportunities then
				if this is the first character on the line then
					goto PChar that will break after the char.
				break line in front of the current character.
				justify
				return.
			break at the last break opportunity.
			if it was a NRH then
				append hyphen to the line
			justify
			return.
PChar:
		printing character:
		if height adjustment pending
			adjust height of line from current font info
			reset height adjustment pending flag
		reset "previous space" flag (see spaces special action)
		continue loop.

LNewRun:        at the end of a run do:
		if ich overflow then
			terminate line
LFetch:
		fetch next run
		limit length of run as not to overflow ich space
		if vanished run
			limit run to end of paragraph - 1
			create vanished entry in grpchr
			continue fetching at the limit of vanished run
		if first chp run or change in chp
			set chp pointer
			create entry in grpchr
				if out of mem? break line in front of next ch.
			if special
				limit run to 1 char
			if special & picture then
				set dxp, dxt
				set "font info" equivalent of height
				goto LHaveDxp;
			load font
			get font info, modify it for sub/superscript.
now we have: info to calculate width of a char, height of the line
the current chp is stored in grpchr.
			if special
				get dxp, dxt for special char
				goto LHaveDxp;
			end if change in chp
		end if next run

exceptional character treatment:
common ideas:
	Actions typically start by removing the ich, dxp, dxt from
	where they were put to create a clean slate.
	Trailing spaces in a line might have incremented xt to beyond
	the right margin.

non required hyphen:
	restore state as it was before LHaveDxp
	if hyphen would not fit or if visible mode
		it is just like getting a hyphen
		ch = '-'; goto LHaveCh;
	this may be an invisible hyphen
	record a vanished run
	remember break opportunity.
	continue from top of the main loop.

hyphen:
	special because it is a break opportunity.
	if xt > right margin then break on the left else
	set up break opportunity after the char.
	continue at the top of the main loop

space:
	special because break opportunity after; influences justification
	and because it may extend over right margin.
	leading spaces in the line are ignored.
	if justified, spaces are counted and put on a list.
	if not left flush, the first of a run of spaces is detected and
		the "justification point" is set up to point before the char.
		(fPrevSpace and xpJust)
	set up break opportunity after the char.
	fPrevSpace = fTrue;
	continue at the top of the main loop

end of para:
	justification will be inhibited.
end of line:
	substitute visible char, if needed otherwise substitute space
	if !fPrevSpace
		Point to the left of the char is the justification point
	break line at the right of the char and treat special tabs.
	return

section character (paragraph or line breaking):
	if first character in the line then
		return splat
	break line in front of the current character.

tab:
	if (xt > right margin) goto LBreak.
	restore state to before LHaveDxp.

	adjust previous tab, if any.

	decide on tab stop using xt and tab table/options.
	if left flush tab
		do it immediately
		xp, xt = desired position in correct units.
	else
		save sufficient state for later tab adjustment
	re-init justification
	record a tab run in grpchr.
	continue at the top of the main loop

Justification:

justify at the end of a line:
parameters:

xpJust, the xp of the point that will have to be brought to
	the right margin;
xtRight, is the right margin


cchSpace, the number of spaces to distibute the slop amongst
ichSpace1, the spaces before this ich are not to be touched
ichSpace3, the spaces after this ich are not to be touched.

center: dxp = (Xp(xtRight) - xpJust) >> 1

right:  dxp = XP(xtRight) - xpJust

	if dxp<=0 do nothing
	in either case xpLeft and xpRight will be advanced by dxp.

justified:
	dxp = xpRight - xpJust;
	if dxp<=0 or cchSpace = 0 do nothing
	dxpExtra = dxp / cchSpace;
	cchWide = dxp - dxpExtra * cchSpace; (i.e. = dxp % cchSpace)

	enumerate all spaces
		if ich >= ichSpace3 then
			cchSpace--; and go back to recalc numbers.
		if cchWide-- == 0
			record transition from wide to narrow spaces
			ichSpace2 = ich
		rgdxp[ich] += dxpExtra or dxpExtra+1
		follow chain and correct rgch entry to be a space.

Tab adjustment:

parameters:
xpTabStop:  place where the tab stop was
xpTabStart:  place or the right edge of the minimum width tab
jcTab:  the justification code
ichTab: ich of the tab character.
ich:    current ich.
xpJust: point to be justified to by right flush tab

Adjustment consists of increasing rgdxp[ichTab]
so that xt is increased to xtTab.

right:
	dxt = xtTabStop - xtJust
center:
	dxt = xtTabStop - xtTabStart - ((xtJust - xtTabStart + 1) >> 1)
decimal:
	dxt = xtTabStop - xtTabStart
	for (ichT = ichTab + 1; ichT < ich (the current ich))
		rgch[ichT] != chDecimal; ichT++)
		dxt -= "rgdxt"[ichT] calculated from ch's.
*/

#ifdef DEBUGORNOTWIN
/* F O R M A T  L I N E */
/* formats line in doc in ww starting at cp. */
/* works for text in tables */
#ifdef MAC
/* %%Function:FormatLine %%Owner:NOTUSED */
NATIVE FormatLine(ww, doc, cp) /* WINIGNORE - "C_" in WIN */
#else /* WIN */
/* %%Function:C_FormatLine %%Owner:bryanl */
HANDNATIVE C_FormatLine(ww, doc, cp)
#endif /* WIN */
int ww, doc;
CP cp;
{
	if (!FInCa(doc, cp, &caSect))
		CacheSect(doc, cp);
	FormatLineDxa(ww, doc, cp, vsepFetch.dxaColumnWidth);
}


#endif /* DEBUGORNOTWIN */

#ifdef DEBUGORNOTWIN
/* formats line in doc in ww starting at cp possibly according to the
width of pdr */
#ifdef MAC
/* %%Function:FormatLineDr %%Owner:NOTUSED */
NATIVE FormatLineDr(ww, cp, pdr) /* WINIGNORE - "C_" in WIN */
#else /* WIN */
/* The name has been changed from C_FormatLineDr in the WIN code to
	avoid a conflict with C_FormatLineDxa (the first 13 characters match).
	The other uses of FormatLineDr are changed with a macro in debug.h.
*/
/* %%Function:C_FormatDrLine %%Owner:bryanl */
HANDNATIVE C_FormatDrLine(ww, cp, pdr)
#endif /* WIN */
int ww;
CP cp;
struct DR *pdr;
{
	int dxa;
	int doc = pdr->doc;
	if (!pdr->fForceWidth)
		{
		if (!FInCa(doc, cp, &caSect))
			CacheSect(doc, cp);
		dxa = vsepFetch.dxaColumnWidth;
		}
	else
		{
#ifdef WIN
/* set up at least vfti.dxpInch right for DxaFromDxp */
		int flmWw = PwwdWw(ww)->grpfvisi.flm;
		if (flmWw != vflm)
			SetFlm(flmWw);
		dxa = pdr->dxa;
#else
		dxa = DxaFromDxp(PwwdWw(ww), pdr->dxl);
#endif
		}
	FormatLineDxa(ww, doc, cp, dxa);
}


#endif /* DEBUGORNOTWIN */


/* F O R M A T  L I N E  D X A */
/* formats line in ww in doc starting at cp with column width dxa. */

#ifdef DEBUGORNOTWIN
#ifdef MAC
/* %%Function:FormatLineDxa %%Owner:NOTUSED */
NATIVE FormatLineDxa(ww, doc, cp, dxa) /* WINIGNORE - "C_" in WIN */
#else /* WIN */
/* %%Function:C_FormatLineDxa %%Owner:bryanl */
HANDNATIVE C_FormatLineDxa(ww, doc, cp, dxa)
#endif /* WIN */
int ww; /* specifies formatting parameters e.g. fOutline */
int doc;
CP cp;
uns dxa;
{
	int ich;
	int ichSpace;
	int cchSpace;
	int jc, jcTab;
	BOOL fPrevSpace;
	BOOL fInhibitJust;
/* set if separate xp/xt calculations are needed */
	BOOL fDxt;
	int xt;
	int xp;
	uns dxt;
	uns dxp;
	int ch;
	int dypBefore;
	DYY dyyAscent;
	DYY dyyDescent;
	DYY dyyAscentBreak;
	DYY dyyDescentBreak;
	DYY dyyAscentMac;
	DYY dyyDescentMac;
/* set if font changed and dyp*Mac will have to be updated when non-blank
character appears */
	BOOL fHeightPending;
	char HUGE *hpch, HUGE *hpchLim;
	int itbd;
	int xpJust;
	int xtJust;
	int xtRight;
	int xaRight;
	int xaLeft;
	int cchWide;
	int bchrPrev; /* used to back up over ch if no break opportunity */
	int bchrNRH;
	int bchrChp;
	int bchrBreak;

	int xtTabStart;
	int xpTabStart;
	int xtTabStop;
	int ichTab;
	int chReal, chT, ichT, ichT1, dxpT;
	int ichM1;
	int tlc;
/* pointer to either vfti or vftiDxt */
	struct FTI *pftiDxt;
	CP cpNext;
	int docNext;
	struct CHR *pchr;
	int fcm;
	CP cpCurPara;
	char cParen;
	int dxtTabMinT;
	int itbdMac;
	struct DOD *pdodT, *pdod;
	struct FMAL fmal;
	struct CHP chpFLDxa; /* munge local instead of invalidating fetch cache every time */
	struct WWD *pwwd;
	BOOL fPassChSpec;
#ifdef MAC
	BOOL fFormulaVisi;
	BOOL fFormulaError;
	/* see comment at LHaveCh */
	BOOL fFastWidth;
	long dxpFrac;
#else /* WIN */
	CP dcpVanish;
	struct FFS ffs;
	int dytFont;
	BOOL fStopFormat;
	int dichSpace3;
#endif /* WIN/~MAC */
	BOOL fInTable;
	CP cpFirstPara;
	int fAbs;

#ifdef WIN
	vrf.fInFormatLine = !vrf.fInFormatLine;
	Assert (vrf.fInFormatLine);
	if (!vrf.fInFormatLine)
		{
		vrf.fInFormatLine = !vrf.fInFormatLine;
		return;
		}
#endif /* WIN */

	dyyAscent.wlAll = 0;
	dyyDescent.wlAll = 0;
	xpJust = 0;
	Win( dichSpace3 = 0 );

	Assert(ww >= wwTemp);
	pwwd = *mpwwhwwd[ww];
#ifdef WIN
	if (pwwd->grpfvisi.flm != vflm)
		{
		SetFlm( pwwd->grpfvisi.flm );
		pwwd = *mpwwhwwd [ww];
		}
	Assert( vflm == flmDisplayAsPrint || vflm == flmDisplay ||
			vflm == flmPrint || vflm == flmRepaginate );
#endif

/* check cache for fli current */
	if (ww == vfli.ww && vfli.doc == doc && vfli.cpMin == cp &&
#ifdef WIN
			pwwd->grpfvisi.w == vfli.grpfvisi.w &&
#endif
			vfli.fPageView == pwwd->fPageView && vfli.dxa == dxa)
		{
#ifdef WIN
		vrf.fInFormatLine = !vrf.fInFormatLine;
#endif /* WIN */
		return; /* Just did this one */
		}

/* obtain any format modes from ww */
	vfli.ww = ww;
	vfli.fOutline = pwwd->fOutline;
#ifdef MAC
	vfli.grpfvisi = (vpref.fShowP) ? -1 : 0;
	vfli.fSeeHidden = vpref.fSeeHidden;
	if (doc != vfli.doc)
		DxsSetDxsInch(doc);
#else
	vfli.grpfvisi = pwwd->grpfvisi;
#endif
	Assert( doc != docNil );
	vfli.omk = omkNil;      /* no outline mark */
	vfli.doc = doc;
	vfli.fPageView = pwwd->fPageView;
	if (!FInCa(doc, cp, &caPara))
		CachePara(doc, cp);
	fAbs = FAbsPap(doc, &vpapFetch);
	if (FInTableVPapFetch(doc, cp))
		{
		/* in the interest of speed, steal some of GetTableCell's job */
		CacheTc(ww, doc, cp, fFalse, fFalse);
		dxa = DxaFromDxp(*mpwwhwwd[ww], vtcc.xpDrRight - vtcc.xpDrLeft);
		if (dxa == 0)
			dxa = xaRightMaxSci;
		}
	else  if (vpapFetch.dxaWidth != 0 && fAbs)
		dxa = vpapFetch.dxaWidth;

	vfli.dxa = dxa;

	vfli.fError = 0;
	fcm = fcmChars + fcmProps;
	if (!pwwd->fOutline || WinMac(pwwd->fShowF, vpref.fOtlShowFormat))
		fcm += fcmParseCaps;
/* rest of format loads up cache with current data */
	vfli.cpMin = cp;
#ifdef WIN
	tlc = 0;
	ffs.ifldError = ifldNil;
	ffs.ifldWrap = ifldNil;
	fStopFormat = fFalse;
#endif
#ifdef MAC
	fFormulaVisi = vfli.fShowFormulas && !vfli.fPageView;
	fFormulaError = fFalse;
#endif
	if (!FInCa(doc, cp, &caSect))
		CacheSect(doc, cp);

LRestartLine:
/* we retart from here if an error in a formula (or other format group field)
	forces a reformat */
	cpCurPara = cp;
	cpNext = cp;
/* initialize run table */
	vifmaMac = 0;
	vbchrMac = 0;
	bchrChp = -1;
	dxtTabMinT = (vfli.dxuInch < 72) || fWin ? 1 : dxtTabMin;
	fPassChSpec = fFalse;
	vfli.fAdjustForVisi = fFalse;
#ifdef WIN
	vfli.fRMark = fFalse;
	vfli.fGraphics = fFalse;
	vfli.fPicture = fFalse;
	vfli.fMetafile = fFalse;
	vpffs = &ffs;
	ffs.fFormatting = fFalse;
	fmal.fLiteral = fFalse;
#endif /* WIN */
	vfli.fVolatile = fFalse;

/* we restart from here if a vanished Eop appeared on the line. The format
of paragraph at cpCurPara will be effective. */
LRestartPara:
	vfli.fSplats = 0;
	ich = 0;
	ichSpace = ichNil;
	cchSpace = 0;
	fPrevSpace = fFalse;
/* in lines ending with chEop etc. justification will be inhibited */
	fInhibitJust = fFalse;
	dyyAscentBreak.wlAll = 0;
	dyyDescentBreak.wlAll = 0;
	dyyAscentMac.wlAll = 0;
	dyyDescentMac.wlAll = 0;
	itbd = 0;
	cParen = 255;
	docNext = doc;

	Assert(cHpFreeze == 0);
	Scribble(ispFormatLine,'F');

/* cache paragraph properties */

	CachePara(doc, cpCurPara);

	if (fInTable = vpapFetch.fInTable)
		{
		extern CP vcpFirstTablePara;
		fInTable = FInTableDocCp(doc, cpCurPara);
		cpFirstPara = vcpFirstTablePara;
		}
	else
		cpFirstPara = caPara.cpFirst;

#ifdef WIN
	if (!fInTable)
		cpFirstPara = caPara.cpFirst;
#endif /* WIN */

	vfli.grpfBrc = 0;
	itbdMac = vpapFetch.itbdMac;
	if (vpapFetch.itbdMac) vfli.fBarTabs = fTrue;
	/* this can be done quite quickly in hand NATIVE */
	if ((vpapFetch.brcTop != brcNone || vpapFetch.brcLeft != brcNone
			|| vpapFetch.brcBottom != brcNone || vpapFetch.brcRight != brcNone
			|| vpapFetch.brcBetween != brcNone || vpapFetch.brcBar != brcNone)
			&& !vfli.fOutline)
		FormatBorders(doc, cp, cpCurPara, cpFirstPara);

#ifdef WIN
#ifdef DEBUG
	/* Note: MacWord removed the "ifdef DEBUG" just before they shipped.
		Let's wait until we get an assert here that we don't like before
		opus does that too.	Brad Verheiden (4-17-89). */
	pdod = PdodDoc(doc);
	if (pdod->fShort && vfli.fOutline)
		{
		Assert(fFalse);
		vfli.fOutline = fFalse;
		}
#endif /* DEBUG */
#else /* !WIN */
	pdod = PdodDoc(doc);
	if (pdod->fShort && vfli.fOutline)
		{
		Assert(fFalse);
		vfli.fOutline = fFalse;
		}
#endif /* !WIN */
	xaRight = dxa - vpapFetch.dxaRight;
	xaLeft = vpapFetch.dxaLeft;

	dypBefore = 0;
	
	if (!vfli.fOutline || fInTable || PdodDoc(doc)->hplcpad == hNil)
		{
		jc = vpapFetch.jc;
		if (cpCurPara - cpFirstPara <= 1)
			{
/* special case for leading page/col break char in paragraph: first line
properties are still maintained */
			if (cpCurPara != cpFirstPara &&
					(ch = ChFetch(doc, cpFirstPara, fcmChars))
					!= chSect Win(&& ch != chColumnBreak))
				goto LNotFirstLine;

			dypBefore = DypFromDya(vpapFetch.dyaBefore);
/* leave room for the boxes */
			if (vfli.fTopEnable)
				{
				vfli.fTop = fTrue;
				dypBefore += vfli.dypBrcTop;
				}

/* note: we permit xaLeft to become negative */
			xaLeft += vpapFetch.dxaLeft1;
#ifdef MAC
/* set bit to draw mark for invisible properties such as keep */
			if (vfli.fvisiParaMarks && !vpapFetch.fTtp &&
					cpCurPara == cpFirstPara &&
					(vpapFetch.fSideBySide || vpapFetch.fKeep ||
					vpapFetch.fKeepFollow || vpapFetch.fPageBreakBefore ||
					vpapFetch.fNoLnn || fAbs))
				vfli.fPropMark = fTrue;
#endif
			}
		}
#ifndef JR
	else
		{
#ifdef WIN
#ifdef DEBUG
		pdod = PdodDoc(doc);
		Assert(!pdod->fShort);
#endif /* DEBUG */
#endif /* WIN */
		vfli.grpfBrc = 0;
		OutlineProps(&xaLeft, &xaRight, &dypBefore);
		itbdMac = 0;
		jc = jcLeft;
		}
#endif /* JR */

LNotFirstLine:
/* now do necessary checks on xaRight and xaLeft */
	if (xaRight > xaRightMaxSci) xaRight = xaRightMaxSci;
	if (xaLeft > xaRightMaxSci) xaLeft = xaRightMaxSci;
	if (xaRight < xaLeft) xaRight = xaLeft;


/* determine:
	xt, xp, vfli.xpLeft from xaLeft
	xtRight from xaRight
*/
/* make sure that these long scales work on signed xaLeft and xt! */
	xt = XtFromXa(xaLeft);
	xtRight = XtFromXa(xaRight);

#ifdef MAC
	vfli.xpLeft = xp = NMultDiv(xaLeft, vfli.dxsInch, dxaInch);
	if (vfli.fPrint) xp = xt;
#endif /* MAC */
	Win( vfli.xpLeft = xp = NMultDiv(xaLeft, vfti.dxpInch, dxaInch) );

/* xt/xp is different iff !fPrint && fFormatAsPrint */
	fmal.pftiDxt =
			pftiDxt = (fDxt = !vfli.fPrint && vfli.fFormatAsPrint) ?
			&vftiDxt : &vfti;

/* initialize other format variables */
	vfli.chBreak = chNil;
/* special initialization for decimal table entries */
	if (fInTable && vpapFetch.itbdMac == 1)
		{
#ifdef WIN
		jcTab = ((struct TBD *)vpapFetch.rgtbd + itbd)->jc;
#else
		jcTab = ((vpapFetch.rgtbd[itbd] >> 5) & 7);
#endif
		if (jcTab == jcDecimal)
			{
			ichTab = -1;
			xtTabStart = xt;
			xpTabStart = xp;
			xtTabStop = XtFromXa(vpapFetch.rgdxaTab[0]);
			jcTab = jcDecTable;
			}
		else
			jcTab = jcLeft;
		}
	else
		jcTab = jcLeft;
	vfli.ichSpace1 = 0;
	vfli.ichSpace2 = ichNil;
	vfli.ichSpace3 = ichNil;

#ifdef MAC
	vfli.cchSpace = 0;
	pdod = PdodMother(doc);
	vfli.fFrac = (vprsu.prid != pridImage && pdod->dop.fFracWidth);
	vfli.dxuFrac = (vfli.fFrac) ? vdxuFrac : 0;
	vfti.dxuFrac = vftiDxt.dxuFrac = vfli.dxuFrac;
#endif /* MAC */

	goto LFetch;


/* main loop */
/* loop variables:
	xp      current position, incremented at LHaveDxp
	xt      same as xp in xt units.
	ich     next char will be stored at rgch[ich], rgdxp[ich]
	hpch
		points to next character
	fPrevSpace
		true iff prev char was space; means "we have already seen
		the start of the current run of white space and have acted
		on it by remembering its position for justification"
	hpchLim  ends current run. If different from vhpchFetch + vccpFetch
		cpNext, docNext must be set.
	cpNext  if not cpNil, the cp to be read when hpch reaches hpchLim.

	xpJust  is the place which will be justified to xtRight.

	cchSpace        number of spaces on ichSpace list
	ichSpace        start of the list of spaces to be justified (except
		for those trailing the line.) Spaces appear in reverse ich
		order. List is terminated by ichNil.

Current break opportunity is stored in vfli:
	chBreak
	cpMac
	ichMac
	xpRight
	bchrBreak       (stored in local)
*/
/* note that the loop is organized so that all code that needs to be executed
for every character in the line appears in the front for easy NATIVE coding.
*/
	for (;;)
		{
		if (hpch == hpchLim) goto LNewRun;

		ch = *hpch++;
LHaveCh:
		vfli.rgch[ich] = ch;

/* calculate the dxp and dxt width of ch */
#ifdef MAC
		if (fFastWidth)
			{
/* open code if no heap, no scaling, no dxp/dxt difference */
/* see DxpFromCh for explanation of code */
			TurnOffPtrCheck(); /* Let us into the ROM */
			dxpFrac = *(**(long far *far *far *) 0xb2a + ch) + vfti.dxuFrac;
			vfti.dxuFrac = (int) dxpFrac;	/* low word carried over */
			dxp = *(int *) &dxpFrac + vfti.dxuExpanded;
			dxt = dxp;
			TurnOnPtrCheck(); /* Start checking again */
			}
		else
/* contend with heap, scaling, dxt calculation based on mode */
			{
			dxt = dxp = DxpFromCh(ch, &vfti);
			if (fDxt)
				dxt = DxpFromCh(ch, &vftiDxt);
			}
#else
		dxt = dxp = DxpFromCh(ch, &vfti);
		if (fDxt)
			dxt = DxpFromCh(ch, &vftiDxt);
#endif
LHaveDxp:
		xp += dxp;
		xt += dxt;
		vfli.rgdxp[ich++] = dxp;
/* now we are at the right of the current character */

#ifdef MAC
		if (ch == 209 /* em dash */ || ch == 208 /* en dash */)
			goto LHyphen;
#endif

		switch (ch)
			{
#ifdef MAC
/* non-default characters are: 0,    6,    7,	9,  11, 12,  13/10,  14,  31,  32,  40, 41, 45
					Null,Formula,Table,Tab,CRJ,Sect,Eop,  SectJ,NRH,Space,(,),Hyphen
*/
#else
/* non-default characters are: 0,    7,	9,  11, 12,  13 or 10,  14,  31,  32,
					Null,Table,Tab,CRJ,Sect,Eop,	   SectJ,NRH,Space

					40,	41,	   45,	  92 or 6  145-151
					LeftParen,RightParen,Hyphen,Formula, Publishing
*/
#endif /* WIN/MAC */
		default:
#ifdef MAC
			if (ch == vitr.chList && !fFormulaVisi)
				{
				if (cParen != 1 || fmal.fLiteral || (vifmaMac > 0 &&
						rgfma[vifmaMac - 1].fmt == fmtList))
					goto LLit;
				goto LFormula;
				}
#endif /* MAC */
#ifdef WIN
			if (ch == vitr.chList)
				goto LChComma;
#endif /* WIN */
LDef:
			if (xt <= xtRight)
				{
LPChar:
/* printing character that does not break the line */
				fPrevSpace = fFalse;
				if (!fHeightPending)
					continue;
/* this is the end of the high frequency code. All code below is
executed only a few times per line */

/* tabs come here eventually, but shouldn't change line height; when tabs do
	come here, ch has been changed to zero (doesn't hurt for real ch=0, which
	has zero width hence zero height) */
				if (ch)
					{
/* update line height information */
					fHeightPending = fFalse;
					if (dyyDescentMac.dyp < dyyDescent.dyp)
						dyyDescentMac.dyp = dyyDescent.dyp;
					if (dyyAscentMac.dyp < dyyAscent.dyp)
						dyyAscentMac.dyp = dyyAscent.dyp;
#ifdef WIN
					if (dyyDescentMac.dyt < dyyDescent.dyt)
						dyyDescentMac.dyt = dyyDescent.dyt;
					if (dyyAscentMac.dyt < dyyAscent.dyt)
						dyyAscentMac.dyt = dyyAscent.dyt;
#endif
					}
				continue;
				}
			else
				{
LBreak:
/* printable character with right edge beyond right margin */
				vfli.dcpDepend =
						(hpch - vhpchFetch) + vcpFetch - vfli.cpMac;
#ifdef MAC
#ifdef FUTURE
/* in hyphenation mode, a character which could be replaced by a hyphen
should be included in the interval considered for hyphenation */
/* check for auto hyphenation */
				if ((jc == jcBoth && vfli.fAutoHyphenate))
					{
					int ichLim = ich;
					if (xt - dxt + DxpFromCh(chHyphen, pftiDxt) <= xtRight)
						{
						vfli.dcpDepend++;
						ichLim++;
						}
					if (FFormatHyph(&bchrBreak, ichLim, pftiDxt))
						{
						xpJust = vfli.xpRight;
						vfli.chBreak = chNonReqHyphen;
						goto LEndJ;
						}
					}
#endif /* FUTURE */
#endif	/* MAC */
				if (vfli.chBreak == chNil)
					{
/* there is no break opportunity to the left of xt */
/* break to the right of the character if first ch on line, else to its left */
					if (ich != 1)
						{
						struct CHR *pchrT = &(**vhgrpchr)[bchrPrev];
						hpch--;
						ich--;
						if (pchrT->ich == ich && pchrT->chrm != chrmVanish)
							vbchrMac = bchrPrev;
						xp -= dxp;
						}
#ifdef WIN
					else  if (ffs.fFormatting)
						{
						if ((hpch-vhpchFetch)+vcpFetch
								+ 1 == CpLimField(doc,
								ffs.ifldFormat))
							goto LPChar;
						}
#endif /* WIN */

					vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
					vfli.ichMac = ich;
					vfli.dcpDepend = 1;
					vfli.xpRight = xp;
					bchrBreak = vbchrMac;
					goto LEnd;
					}
/* otherwise break at the last opportunity. vfli was set up at the break
opportunity */
				if (vfli.chBreak == chNonReqHyphen &&
						!FVisiCondHyphensFli)
					{ /* Append the now-required hyphen to end of line */
					vfli.ichMac++;
/* WIN: Adjustment to assure vfli.ichSpace3 is a valid guide to 
	the underline termination point. */
					vfli.ichSpace3 = WinMac( vfli.ichMac,
							vfli.ichMac-1);
					vfli.rgdxp[vfli.ichMac-1] = dxp = DxpFromCh(chHyphen, &vfti);
					vfli.rgch[vfli.ichMac-1] = chHyphen;
					vfli.xpRight += dxp;
					xpJust = vfli.xpRight;
					/* xtJust not needed at LEndJ */
/* truncate grpchr before the vanished run */
					bchrBreak = bchrNRH;
					}
				if (vfli.chBreak == chTab) vfli.ichSpace3 = ichNil;
				goto LEndJ;
				}


/* Non-required hyphen. Code here is motivated by the possibility of
the NRH being turned into a line-terminating hyphen. Until that happens:
	nothing stored in rgch, rgdxp.
	room is assured but not reserved.
	vanished crt is placed in grpcrt.
	break opportunity is set up following the assumed hyphen.
*/
		case chNonReqHyphen: /* == 31 */
				{
				if ((int)(xt + DxpFromCh(chHyphen, pftiDxt) - dxt) > xtRight)
					{
					goto LBreak;
					}
				if (FVisiCondHyphensFli)
	/* visible. Treat like a hyphen/non break-hyphen combo.
See conditional at end of LChrt */
					{
					chT = chVisNonReqHyphen;
					chReal = '-';
					goto LNonBreakHyphen;
					}
/* undo main loop actions */
				ich--;
				xtJust = (xt -= dxt);
				xpJust = (xp -= dxp);
				Win(OverhangChr(&xp, bchrPrev,ich));
				bchrPrev = bchrNRH = vbchrMac;
				Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
				if ((vbchrMac += cbCHRV) <= vbchrMax ||
						FExpandGrpchr(cbCHRV))
					{
					(pchr = &(**vhgrpchr)[bchrNRH])->ich = ich;
					pchr->chrm = chrmVanish;
					((struct CHRV *)pchr)->dcp = 1;
					}
				goto LBreakOppR;
				}

#ifdef WIN
		case chNonBreakSpace:	/* == 160 */
LChNonBreakSpace:
	/* display as visi space, otherwise as LDef ordinary character */
			chReal = chSpace;
			chT = chVisNonBreakSpace;
			if (!FVisiSpacesFli)
				chT = chReal;
			goto LVisi;

		case chPubLDblQuote:
		case chPubRDblQuote:
		case chPubBullet:
		case chPubEmDash:
		case chPubEnDash:
			ich--;
			xp -= dxp;
			xt -= dxt;
			FormatChPubs( ch, &dxp, &dxt );
			goto LChrt;
#endif /* WIN */

		case chNonBreakHyphen: /* == 30 */
/* display as hyphen, otherwise as LDef ordinary character */
			chReal = '-';
			WinMac( chT = chNil, chT = '-' );
LNonBreakHyphen:	
			if (!FVisiCondHyphensFli)
				chT = chReal;
LVisi:                  
			ich--;
			xp -= dxp;
			xt -= dxt;
			if (chT == chNil)
				{
				vfli.fAdjustForVisi = fTrue;
				chT = chSpace;
				dxt = dxp = DxpFromCh('-',&vfti) << 1;
				}
			else
				{
				dxp = DxpFromCh(chT, &vfti);
				dxt = ch == chNonReqHyphen ? 0 : dxp;
				}
			if (fDxt)
				dxt = DxpFromCh(chReal, &vftiDxt);
			vfli.rgch[ich] = chT;
			goto LChrt;

		case chHyphen: /* == 45 */
LHyphen:
			if (xt > xtRight)
				goto LBreak;
LHyphen1:               
			xpJust = xp;
			xtJust = xt;
/* WIN: Adjustment to assure vfli.ichSpace3 is a valid guide to 
	the underline termination point. */
			vfli.ichSpace3 = WinMac( ich, ich - 1);
			dyyDescentMac.dyp = max(dyyDescentMac.dyp, dyyDescent.dyp);
			dyyAscentMac.dyp = max(dyyAscentMac.dyp, dyyAscent.dyp);
#ifdef WIN
			dyyDescentMac.dyt = max(dyyDescentMac.dyt, dyyDescent.dyt);
			dyyAscentMac.dyt = max(dyyAscentMac.dyt, dyyAscent.dyt);
#endif
			fPrevSpace = fFalse;
			goto LBreakOppR;
#ifdef MAC
/* following characters are special in Formulas (if cParen is small) */
		case '(':
			if (fFormulaVisi) goto LDef;
			if (!fmal.fLiteral)
				{
				if (cParen != 255)
					cParen++;
				goto LDef;
				}
LLit:                   
			fmal.fLiteral = fFalse;
			goto LDef;

		case ')':
			if (fFormulaVisi) goto LDef;
			if (fmal.fLiteral) goto LLit;
			if (cParen != 255)
				cParen--;
			if (cParen == 0)
				goto LFormula;
			goto LDef;
#endif

/* Null characters are made visible in visible mode */
		case 0: /* null */
			if (FVisiSpacesFli)
				{
				xp -= dxp;
				xt -= dxt;
				ich--;
				ch = 0377; 
				goto LHaveCh;
				}
			goto LDef;
/* space: count and enter in list */
		case chSpace: /* == 32 */
			if (vifmaMac != 0)
				{
				xp -= dxp;
				xt -= dxt;
				ich--;
				ch = chNonBreakSpace; 
				goto LHaveCh;
				}
			ichM1 = ich - 1;
			if (!fPrevSpace)
/* first in a series of spaces */
				{
				xpJust = xp - dxp;
				xtJust = xt - dxt;
				vfli.ichSpace3 = ichM1;
				}
			fPrevSpace = fTrue;
#ifdef WIN
			if (FVisiSpacesFli)
				vfli.rgch [ichM1] = chVisSpace;
#endif /* WIN */

/* spaces are entered in a list iff jcBoth */
			if (jc == jcBoth)
				if (ichM1 == vfli.ichSpace1)
/* except leading spaces in the line are ignored */
					vfli.ichSpace1++;
				else
					{
					cchSpace++;
					vfli.rgch[ichM1] = ichSpace;
					ichSpace = ichM1;
					}
LBreakOppR:
/* set up break opportunity to the right of the char */
/* ch is current, hpch, xp, ich all incremented */
			vfli.chBreak = ch;
			vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
			vfli.xpRight = xp;
			vfli.ichMac = ich;
			bchrBreak = vbchrMac;

			dyyAscentBreak = dyyAscentMac;
			dyyDescentBreak = dyyDescentMac;
			continue;


/* end of paragraph/line/section */
#ifdef CRLF
		case chReturn:  /* == 13 */
			/* get here when CR is not at run start */
			ich--;
			xp -= dxp;
			xt -= dxt;
			cpNext = vcpFetch + (hpch - vhpchFetch);
			dcpVanish = 1;
			goto LVanish;
#endif
		case chEop: /* == 13 or == 10 (CRLF) */
			fInhibitJust = fTrue;
#ifdef WIN
			chT = FVisiParaMarksFli ? chVisEop : chSpace;
			xp -= dxp;
			dxp = dxt = C_DxpFromCh( chVisEop, &vfti );
			goto LEol;
#endif
			/* FALL THRU */

		case chCRJ: /* == 11 */
			chT = chSpace;
LChTSet:
			xp -= dxp;

/* WIN version compensates for VISI-CRJ bitmap that is wider than a
	normal char */

#ifdef WIN
			dxt = dxp = min( C_DxpFromCh( chSpace, &vfti ) << 2,
					dxpChVisEach );
#else
			dxp = 9;
/* dxt doesn't really matter since ch is not visi on printer */
#endif

LEol:
			vfli.rgch[ich - 1] = chT;
			vfli.rgdxp[ich - 1] = dxp;
			xp += dxp;
			vfli.dcpDepend = 0;
LEol1:
/* at this point xp points after the substituted character, dxp is
substituted width, xt points after the original char, dxt is original width. */
			xt -= dxt;
			vfli.chBreak = ch;
/* set up justification point */
			if (!fPrevSpace)
				{
				xpJust = xp - dxp;
				xtJust = xt;
/* WIN: Adjustment to assure vfli.ichSpace3 is a valid guide to 
	the underline termination point. */
				vfli.ichSpace3 = ich - 1 + WinMac( dichSpace3, 0 );
				}
			if (jcTab != jcLeft)
				{ 
				ich--; 
				goto LAdjustTab; 
				}
			goto LEndBreak;

		case chTable: /* == 7 */
				{
				CachePara(doc,vcpFetch);
				if (vpapFetch.fInTable && vcpFetch+(hpch-vhpchFetch)==caPara.cpLim)
					{
					fInhibitJust = fTrue;
					if (!vfli.fLayout)
						vfli.fSplatBreak = fTrue;
#ifdef WIN
					xp -= dxp;
					dxp = DxpFromCh(chVisCellEnd, &vfti);
					chT = FVisiParaMarksFli ? chVisCellEnd : chSpace;
					goto LEol;
#else
					chT = chSpace;
					goto LChTSet;
#endif
					}
			/* It's not an end of cell mark so treat it as a normal character */
				goto LDef;
				}

#ifdef WIN
		case chColumnBreak: /* == 14 */
			if (ich == 1 || vfli.fPageView || vfli.fLayout)
				vfli.fSplatColumn = fTrue;
#endif                  /* FALL THROUGH */

/* section mark, line is broken in front of the char unless first in line.
There are justifying and a non-justifying variants.
*/
		case chSect: /* == 12 */
				{
				if (fInTable)
					goto LEndBreak0;
				if (vfli.fPageView || vfli.fLayout)
					{
/* page view ; return a splat */
					vfli.fSplatBreak = fTrue;
					xp -= dxp;
					dxp = NMultDiv(xaRight, vfli.dxsInch, dxaInch) - xp;
					chT = chSpace;
					goto LEol;
					}
				if (ich == 1)
					{
/* Beginning of line; return a splat */
					vfli.fSplatBreak = fTrue;
					dxp = vfli.rgdxp[0] = xpSplat - (xp - dxp);
					xp = xpSplat;
					goto LBreak;
					}
/* undo main loop actions */
				ich--;
				hpch--;
				xp -= dxp; 
				dxp = 0;
				vfli.dcpDepend = 1;
				dichSpace3 = 1;
				goto LEol1;
				}


/* tab */
		case chTab: /* == 9 */
			if (xt > xtRight) goto LBreak;
/* undo main loop actions */
			ich--;
			xt -= dxt;
			xp -= dxp;
			if (!fPrevSpace)
				{
				xpJust = xp;
				xtJust = xt;
				}
LAdjustTab:
			Mac( vfti.dxuFrac = vftiDxt.dxuFrac = vfli.dxuFrac );
			Break1();
/* adjust the previous right/ctr/dec tab, if any. This code is actually
a subroutine written as in-line code so that locals in FormatLine can
be freely referenced.
*/
/* remove chain of spaces */
			for (ichT = ichSpace; ichT != ichNil; ichT = ichT1)
				{
				ichT1 = vfli.rgch[ichT];
				vfli.rgch[ichT] = (fWin && FVisiSpacesFli)
						? chVisSpace : chSpace;
				}
			ichSpace = ichNil;
			vfli.ichSpace1 = ich;
			if (jcTab != jcLeft)
/* we have from prev tab: jcTab, xtTabStart, xtTabStop, ichTab */
/* xt is the alignment point that will be moved to right flush tab */
				{
				int dxtT;
				int xpTabStop = fDxt ?
				Mac( NMultDiv(xtTabStop, vfli.dxsInch, vfli.dxuInch) )
					Win( NMultDiv(xtTabStop, vfti.dxpInch, vftiDxt.dxpInch) )
							: xtTabStop;
				switch (jcTab)
					{
				case jcRight:
					dxtT = xtTabStop - xtJust;
					dxpT = xpTabStop - xpJust;
					break;
				case jcCenter:
					dxtT = xtTabStop - xtTabStart -
							((xtJust - xtTabStart + 1) >> 1);
					dxpT = xpTabStop - xpTabStart -
							((xpJust - xpTabStart + 1) >> 1);
					break;
				case jcDecimal:
				case jcDecTable:
/* decimal alignment method defined elsewhere for easy modification */
					dxtT = -DxtLeftOfTabStop(ichTab, ich, pftiDxt, &dxpT) + xtTabStop - xtTabStart;
					dxpT = -dxpT + xpTabStop - xpTabStart;
					break;
					}
				xt += max(0, dxtT);
				dxpT = max(0, dxpT);
				if (jcTab != jcDecTable)
					vfli.rgdxp[ichTab] = dxpT;
				else
					vfli.xpLeft += dxpT;
				xp += dxpT;
				}
			if (ch != chTab) 
				{
				ich++; 
				goto LEndBreak; 
				}

/* continue ch == chTab processing */

/* break opportunity in front of the tab, unless first in line */
			if (ich)
				{
				vfli.cpMac = (hpch - vhpchFetch) + vcpFetch - 1;
				vfli.xpRight = xp;
				vfli.ichMac = ich;
				vfli.chBreak = ch;
				bchrBreak = vbchrMac;

				dyyAscentBreak = dyyAscentMac;
				dyyDescentBreak = dyyDescentMac;
				}
/* Now get info about this tab using xt, result to xtTabStop, jcTab.
The tab will go to the next tab stop at position > xt.
*/
			while (itbd < itbdMac &&
					(xtTabStop = XtFromXa(vpapFetch.rgdxaTab[itbd]))
					- dxtTabMinT <= xt ||
					/* bar tab stop */
#ifdef WIN
			((struct TBD *)vpapFetch.rgtbd + itbd)->jc == jcBar)
#else
				((vpapFetch.rgtbd[itbd] >> 5) & 7) == jcBar)
#endif
						itbd++;
/* splice in special default tab at left indent point */
			if (vpapFetch.dxaLeft > xaLeft)
				{
/* there is a hanging indent, checked to gain speed in common case */
				int xtTabDef = XtFromXa(vpapFetch.dxaLeft);
				if (xt < (xtTabDef - dxtTabMinT) &&
						(itbd >= itbdMac ||
						xtTabDef < xtTabStop))
/* select default tab at hanging indent point */
					{
					xtTabStop = xtTabDef;
					goto LDefTab;
					}
				}
			if (itbd >= itbdMac)
				{
/* go to next default tab */
				struct DOP *pdop = &PdodMother(doc)->dop;
				int dxtTab = XtFromXa(pdop->dxaTab);
				if (dxtTab == 0) dxtTab = 1;
				xtTabStop = ((xt + dxtTabMinT) / dxtTab + 1) * dxtTab;
LDefTab:                        
				tlc = tlcNone;
				jcTab = jcLeft;
				}
			else
				{
#ifdef WIN
				struct TBD tbd;
#else
				char tbd;
#endif
/* choose real tab stop itbd */
				if (xtTabStop >= xtRight && abs(vfli.dxa - vsepFetch.dxaColumnWidth) <= 20)
					{
/* "breakthrough" tab: set right margin to 20" -- but only in normal lines,
	meaning not tables and not APO-constrained.  Fuzzy compare on dxa's 
	because dxa passed in might have been generated from printer units. */
					Mac( xtRight = 20 * vfli.dxuInch );
					Win( xtRight = NMultDiv( vsci.xaRightMax,
							vftiDxt.dxpInch,
							czaInch));
					jc = jcLeft;
					}
				tbd = vpapFetch.rgtbd[itbd++];
#ifdef WIN
				tlc = tbd.tlc;
				jcTab = tbd.jc;
#else
				tlc = (tbd >> 2) & 7;
				jcTab = (tbd >> 5) & 7;
#endif
				}
/* Do left-justified tabs immediately */
			if (jcTab == jcLeft)
				{
				dxt = xtTabStop - xt;
				dxp = ((fDxt) ?
#ifdef MAC
						NMultDiv(xtTabStop, vfli.dxsInch,
						vfli.dxuInch) : xtTabStop)
#endif /* MAC */
#ifdef WIN
						NMultDiv(xtTabStop, vfti.dxpInch,
						vftiDxt.dxpInch) : 
				xtTabStop)
#endif /* WIN */
						- xp;
				dxp = max(dxp, 0);
				}
			/* else dxt, dxp = dxuMissing */
			else
				{
/* save state for AdjustTab */
				ichTab = ich;
				dxt = dxtTabMinT; 
				dxp = 0;
				}

/* reset justification parameters */
			/* ichSpace = ichNil as set above */
			cchSpace = 0;
			fPrevSpace = fFalse;
/* create CHR describing the tab */
			ch = 0;
			xtTabStart = xt + dxt;
			xpTabStart = xp;

/* code to create CHRT for tab or visible char in ch, then handle as normal
character;
we have: ch, dxp, dxt.
*/
LChrt:		
			Win(OverhangChr(&xp, bchrPrev,ich));
			bchrPrev = vbchrMac;
			Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
			if ((vbchrMac += cbCHRT) <= vbchrMax ||
					FExpandGrpchr(cbCHRT))
				{
				pchr = &(**vhgrpchr)[bchrPrev];
				pchr->chrm = chrmTab;
				pchr->ich = ich;
				((struct CHRT *)pchr)->tlc = tlc;
				((struct CHRT *)pchr)->ch = ch;
				}
/* now we have a character that either fits or does not fit.
dxp and dxt are set
*/
			xp += dxp;
			xt += dxt;
			vfli.rgdxp[ich++] = dxp;
			if (ch == chNonReqHyphen)
				goto LHyphen1;
#ifdef WIN
			else if (ch == chPubEmDash || ch == chPubEnDash)
				goto LHyphen;
			else
#endif					 
			goto LDef;

#ifdef WIN
/* following characters are special in Formulas (if cParen is small) */
		case '(':
			if (!ffs.fFormatting || ffs.flt != fltFormula)
				goto LDef;
			if (!fmal.fLiteral)
				{
				if (cParen != 255)
					cParen++;
				goto LDef;
				}
LLit:			
			fmal.fLiteral = fFalse;
			goto LDef;

		case ')':
			if (!ffs.fFormatting || ffs.flt != fltFormula)
				goto LDef;
			if (fmal.fLiteral) goto LLit;
			if (cParen != 255)
				cParen--;
			if (cParen == 0)
				goto LFormula;
			goto LDef;

LChComma:	/* case ',': */
			if (!ffs.fFormatting || ffs.flt != fltFormula)
				goto LDef;
			if (cParen != 1 || fmal.fLiteral || (vifmaMac > 0 &&
					rgfma[vifmaMac - 1].fmt == fmtList))
				goto LLit;
			goto LFormula;
#endif /* WIN */

		case chFormula: /* == 6 */
/* undo main loop actions */
#ifdef MAC
			if (xt > xtRight || fFormulaVisi)
/* visible or won't fit. Treat just like a normal char */
				{
				chT = chVisFormula;
				goto LVisi;
				}
#endif /* MAC */
#ifdef WIN
			if (!ffs.fFormatting || ffs.flt != fltFormula)
				goto LDef;
			if (fmal.fLiteral)
				goto LLit;
#endif /* WIN */
LFormula:
			ich--;
			xt -= dxt;
			xp -= dxp;
/* call procedure to do as much of the processing as possible outside of
the optimized environment.
*/
			fmal.doc = doc;
			fmal.cp = hpch - vhpchFetch + vcpFetch;
			fmal.ich = ich;
			fmal.xt = xt;
			fmal.xp = xp;
			fmal.dypAscent = dyyAscentMac.dyp;
			Win( fmal.dytAscent = dyyAscentMac.dyt );
			fmal.dypDescent = dyyDescentMac.dyp;
			Win( fmal.dytDescent = dyyDescentMac.dyt );
			fmal.bchrChp = bchrChp;
			fmal.cParen = cParen;
			FormatFormula(&fmal);
			if (fmal.fError)
				{
#ifdef MAC
				fFormulaVisi = fTrue;
				fFormulaError = fTrue;
#endif /* MAC */
#ifdef WIN
				Assert ((uns) ffs.ifldError > ffs.ifldFormat);
				ffs.ifldError = ffs.ifldFormat;
#endif /* WIN */
				goto LRestartLine;
				}
			docNext = doc;
			cpNext = fmal.cp;
			/*hpch = hpchLim;*/
			ich = fmal.ich;
			xt = fmal.xt;
			xp = fmal.xp;
			dyyAscentBreak.dyp = dyyAscentMac.dyp = fmal.dypAscent;
			dyyDescentBreak.dyp = dyyDescentMac.dyp = fmal.dypDescent;
			Win( dyyAscentBreak.dyt = dyyAscentMac.dyt = fmal.dytAscent );
			Win( dyyDescentBreak.dyt = dyyDescentMac.dyt = fmal.dytDescent );
			fHeightPending = fTrue;
			bchrChp = fmal.bchrChp;
			cParen = fmal.cParen;
			Win( ffs.fValidEnd = (fmal.cParen == 255) );
			goto LFetch;

			} /* switch (ch) */


/* we come here at the end of each run */
LNewRun:
		if (ich >= ichMaxLine)
	/* end of run because of line length limit has been reached. Equivalent to
breaking through the right margin */
			{
			fInhibitJust = fTrue;
			goto LEndBreak0;
			}
/* read next run sequentially, unless docNext, cpNext has been set */
LFetch:

#ifdef MAC
		if (cpNext >= caPara.cpLim)
			goto LEndBreak0;        /* missing chEop, bad file? */
#else
		if (cpNext >= caPara.cpLim || cpNext >= CpMacDoc (doc)
				|| fStopFormat)
			{
			vfli.cpMac = cpNext;
			vfli.chBreak = chEop;
			goto LEndBreakCp;	/* possible vanished eop */
			}
#endif
		FetchCp(docNext, cpNext, fcm);
		chpFLDxa = vchpFetch;
		docNext = docNil;
		hpch = vhpchFetch;
		Win( vfli.fRMark |= !chpFLDxa.fSysVanish &&
				(chpFLDxa.fRMark || chpFLDxa.fStrike); )
#ifdef CRLF
		/* Do this here AND in switch (ch) so that we don't get 0-length chrmCHPs
	when there's a chReturn at the start of a run */
		if (*hpch == chReturn)
			{       /* Make CR's a phony vanished run */
			cpNext = vcpFetch + 1;
			dcpVanish = 1;
			goto LVanish;
			}
#endif  /* CRLF */
#ifdef MAC
		if (chpFLDxa.fVanish)
			{
			vfli.fVolatile = fTrue;
			if (chpFLDxa.fSysVanish || !FSeeHiddenFli)
				{
				CP cpRunLim = vcpFetch + vccpFetch;
				bchrPrev = vbchrMac;
				Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
				if ((vbchrMac += cbCHRV) <= vbchrMax ||
						FExpandGrpchr(cbCHRV))
					{
					pchr = &(**vhgrpchr)[bchrPrev];
					pchr->ich = ich;
					pchr->chrm = chrmVanish;
					((struct CHRV *)pchr)->dcp = vccpFetch;
					}
/* vanish covering paragraphs may come in the beginning of the line
but later ones must not cross para bounds, else line will be terminated */
				if (caPara.cpLim == cpRunLim)
					{
					if (ich == 0 && caPara.cpLim < CpMacDoc(doc)
							&& *(vhpchFetch + vccpFetch - 1) != chTable
							&& !fAbs && !vfli.fOutline)
						if (vfli.fStopAtPara)
							{
							/* can't go past para bounds */
							vfli.fParaStopped = fTrue;
							vfli.dcpDepend = 0;
							vfli.cpMac = caPara.cpLim;
							goto LEndBreakPara;
							}
						else
							{
/* also stop attempts to slide into tables by vanishing text */
							cpNext = cpCurPara = cpRunLim;
							CachePara(doc, cpNext);
							if (vpapFetch.fInTable == fInTable)
								goto LRestartPara;
							CachePara(doc, cpNext - ccpEop);
							}
					if (!vfli.fError)
						((struct CHRV *)pchr)->dcp = vccpFetch - ccpEop;
					FetchCp(docNext = doc, (cpNext = caPara.cpLim) - ccpEop, fcm);
					chpFLDxa = vchpFetch;
					hpch = vhpchFetch;
					vccpFetch = 1;
					goto LVanish1;
					}
/* append vanished runs at the end of a line to the line (i.e. include them
among the spaces) */
				if (fPrevSpace)
					{
					vfli.cpMac = cpRunLim;
					bchrBreak = vbchrMac;
					}
				goto LFetch;
				}
#endif
#ifdef WIN
			if (chpFLDxa.fVanish || chpFLDxa.fFldVanish)
				{
				BOOL fRestartPara;
				vfli.fVolatile = fTrue;
				if (chpFLDxa.fSysVanish || !FSeeHiddenFli)
					{
					fPassChSpec = fFalse;
					if (fInTable && *(vhpchFetch + vccpFetch - 1) == chTable)
						{
/* We don't allow vanishing cell marks.  There are two possibilities if we
	encounter a hidden cell mark.  Either it is alone in the run, in which
	case we just show it, or it is in a run with other hidden text, in which
	case we hide only the text and back up vccpFetch so we pick up the cell
	mark alone the next time through.
*/
						if (vccpFetch == 1)
							goto LVanish2;
						else
							vccpFetch -= 1;
						}
					cpNext = vcpFetch + vccpFetch;
					dcpVanish = vccpFetch;
LVanish:

/*  At this point, cpNext has been set up to
	the cpLim of the "run" to be vanished. it may or may not correspond  to
	a run.  If it does, avoid random access fetch.

	dcpVanish has been set to the number of characters to be vanished.
*/
					if (cpNext != vcpFetch + vccpFetch)
						docNext = doc;

					OverhangChr(&xp, bchrPrev,ich);
					bchrPrev = vbchrMac;
					Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
					if ((vbchrMac += cbCHRV) > vbchrMax)
						if (!FExpandGrpchr(cbCHRV))
							goto LEndBreak0;
					pchr = &(**vhgrpchr)[bchrPrev];
					pchr->ich = ich;
					pchr->chrm = chrmVanish;
					((struct CHRV *)pchr)->dcp = dcpVanish;

					fRestartPara = fFalse;
					/*  if vanished run vanishes the EOP, cache
						the new paragraph */
					if (caPara.cpLim <= cpNext)
						{
						/* end line if in outline mode */
						if (vfli.fOutline)
							goto LBreakMode;

						cpCurPara = cpNext;
						CachePara (doc, cpNext);

					/* for non-tables, make sure that apos
						do not flow into, and are not flowed
						into by, non-apo text */
						if (!fInTable && (vfli.fLayout || vfli.fPageView))
							{
							if (fAbs != FAbsPap(doc, &vpapFetch))
								{
								vfli.fParaStopped = fTrue;
								goto LBreakMode;
								}
							if (fAbs)
								{
								int pap[cwPAPBaseScan];
								CachePara(doc, vfli.cpMin);
								blt(&vpapFetch, &pap, cwPAPBaseScan);
								CachePara(doc, cpNext);
								if (!FMatchAbs(doc, &vpapFetch, pap))
									{
									vfli.fParaStopped = fTrue;
									goto LBreakMode;
									}
								}
							}
						if (ich == 0)
							fRestartPara = fTrue;
						}

					/* we have revealed an unexpected table or end
						of para in outline view*/
					if (vpapFetch.fInTable && 
							(!fInTable != !FInTableVPapFetch(doc,cpNext)))
						{
LBreakMode:
						vfli.cpMac = cpNext;
						vfli.chBreak = chEop;
						vfli.dcpDepend = 1;
						goto LEndBreakCp;
						}
					else  if (fRestartPara)
						goto LRestartPara;

					goto LFetch;
					}
#endif /* WIN */
				else  if (vlm == lmNil)
LVanish1:
					chpFLDxa.kul = kulDotted;
				}
LVanish2:
/* limit run not to exceed ichMaxLine */
			if (ich + vccpFetch > ichMaxLine)
				{
				hpchLim = hpch + ichMaxLine - ich;
				docNext = doc;
				cpNext = vcpFetch + ichMaxLine - ich;
				}
			else
				hpchLim = hpch + vccpFetch;
/* also limit run not to exceed para */
			if ((long) hpchLim >= (long) vhpchFetch + caPara.cpLim - vcpFetch)
				{
				hpchLim = vhpchFetch + (int) (caPara.cpLim - vcpFetch);
				cpNext = caPara.cpLim;
				}
#ifdef WIN
			/*  pre-process fSpec chars */
			if (chpFLDxa.fSpec)
				{
				/* limit all fSpec runs to be one character long */
				hpchLim = hpch + 1;
				docNext = doc;
				cpNext = vcpFetch + 1;

				if (fPassChSpec)
					/*	we want to display this field character */
					{
					chpFLDxa.fBold = fTrue;
					fPassChSpec = fFalse;

					/* we have revealed an unexpected table */
					if (vpapFetch.fInTable && 
							(!fInTable != !FInTableVPapFetch(doc,cpNext)))
						{
						fStopFormat = fTrue;
						vfli.dcpDepend = 1;
						}
					}

				else  if (*hpch < chFieldMax && *hpch >= chFieldMin)
					/* Field Character */
					/*  *pch is a special field deliminator which we
						have not already looked at.
					*/
					{
					CP cpT = vcpFetch;
					int ffc;
					int dxpT = vfti.dxpOverhang;

					ch = *hpch;

					ffc = FfcFormatFieldPdcp (&dcpVanish,
							ww, doc, cpT, ch);

				/*  WARNING: above call will call FetchCp.
					v*Fetch cannot be assumed to be anything
					reasonable.
				*/

					cpNext = cpT + dcpVanish;  /* cp next char */
					docNext = doc; /* vdocFetch may be invalid */
					hpchLim = hpch; /* force a fetch */

				/* order of ffc's and use of if for speed in
					common cases. */
					if (ffc == ffcSkip)
						/* Skip over dcpVanish characters */
						{
						/* restore previous para for LVanish's sake */
						CachePara(doc, cpT);
						goto LVanish;
						}

				/*  FfcFormat... may have cached a different para */
					CachePara (doc, cpNext);

					if (ffc == ffcShow)
					/* Don't skip anything.  Go around and
						do this character again, but next
						time don't call FfcFormatFieldPdcp.
				*/
						{
						fPassChSpec = fTrue;
						goto LFetch;
						}
					else  if (ffc == ffcDisplay)
						{
						int dxpT1;
				/*  we have a display field.  generate a
					chrmDisplayField.
				*/
						dxpT1 = vfti.dxpOverhang;
						vfti.dxpOverhang = dxpT;
						Win(OverhangChr(&xp, bchrPrev,ich));
						vfti.dxpOverhang = dxpT1;
						bchrPrev = vbchrMac;
						if ((vbchrMac += cbCHRDF) > vbchrMax)
							if (!FExpandGrpchr(cbCHRDF))
								{
LNoExpand:
								vfli.dcpDepend = 0;
								vfli.cpMac = cpT;
								goto LEndBreakCp;
								}

				/*  set up CHRDF */
						pchr = &(**vhgrpchr)[bchrPrev];
						pchr->chrm = chrmDisplayField;
						pchr->ich = ich;
						((struct CHRDF *)pchr)->flt = vfmtss.flt;
						((struct CHRDF *)pchr)->w = vfmtss.w;
						((struct CHRDF *)pchr)->w2 = vfmtss.w2;
						((struct CHRDF *)pchr)->l = vfmtss.l;
						((struct CHRDF *)pchr)->dxp = vfmtss.dxp;
						((struct CHRDF *)pchr)->dyp = vfmtss.dyp;
						((struct CHRDF *)pchr)->dcp = dcpVanish;

				/*  set up sizing information */
						dxp = vfmtss.dxp;
						dyyAscent.dyp = vfmtss.dyp - vfmtss.dyyDescent.dyp;
						dyyAscent.dyt = vfmtss.dyt - vfmtss.dyyDescent.dyt;
						dyyDescent = vfmtss.dyyDescent;
						dxt = vfmtss.dxt;

				/*  do main loop actions */
						xp += dxp;
						xt += dxt;

				/* guarantee subsequent chrmChp */
						bchrChp = -1;

				/* a CHRDF has chDisplayField at ich */
						vfli.rgch [ich] = chDisplayField;
						vfli.rgdxp [ich++] = dxp;

						/* The following are duplicates of "default"
							character processing.  Here due to needed
							knowledge of the presence of CHRDF.
						*/

				/*  check for line overflow */
						if (xt <= xtRight || ich == 1)
						/* we are ok or we broke through but it's
						the only thing on line--leave it */
							{
					/* update line height */
							if (dyyDescentMac.dyp < dyyDescent.dyp)
								dyyDescentMac.dyp = dyyDescent.dyp;
							if (dyyAscentMac.dyp < dyyAscent.dyp)
								dyyAscentMac.dyp = dyyAscent.dyp;
							if (dyyDescentMac.dyt < dyyDescent.dyt)
								dyyDescentMac.dyt = dyyDescent.dyt;
							if (dyyAscentMac.dyt < dyyAscent.dyt)
								dyyAscentMac.dyt = dyyAscent.dyt;

					/* set up break opportunity to right*/
							vfli.chBreak = chDisplayField;
							vfli.cpMac = cpNext;
							vfli.xpRight = xp;
							vfli.ichMac = ich;
							bchrBreak = vbchrMac;
							dyyAscentBreak = dyyAscentMac;
							dyyDescentBreak = dyyDescentMac;

							continue;
							}
				/* else went beyond right margin */
						else  /* break to left of field */					
							{
					/* undo "main loop" action */
							xp -= dxp;
							xt -= dxt;
							ich--;
							vfli.dcpDepend = 1;
							vbchrMac = bchrPrev;
							vfli.cpMac = cpT;

							goto LEndBreakCp;
							}

						}  /* ffcDisplay */

					else  if (ffc == ffcBeginGroup)
						{
						Win(OverhangChr(&xp, bchrPrev,ich));
						bchrPrev = vbchrMac;
						if ((vbchrMac += cbCHRFG) > vbchrMax)
							if (!FExpandGrpchr (cbCHRFG))
								{
								goto LNoExpand;
								}
						pchr = &(**vhgrpchr)[bchrPrev];
						((struct CHRFG *)pchr)->chrm = chrmFormatGroup;
						((struct CHRFG *)pchr)->ich = ich;
						((struct CHRFG *)pchr)->dbchr = cbCHRFG;
						((struct CHRFG *)pchr)->dcp = -cpT;
						ffs.flt = ((struct CHRFG *)pchr)->flt
								= vfmtss.flt;
						ffs.bchr = bchrPrev;
						ffs.xp = xp;
						ffs.fValidEnd = fTrue;
						goto LVanish;
						}

					else  if (ffc == ffcEndGroup)
						{
				/* must guarantee we have room */
						if ((vbchrMac + cbCHRV) > vbchrMax)
							{
							vbchrMac += cbCHRV;
							if (!FExpandGrpchr(cbCHRV))
								goto LEndBreak0;
							vbchrMac -= cbCHRV;
							}
						pchr = &(**vhgrpchr)[ffs.bchr];
						((struct CHRFG *)pchr)->dxp = xp - ffs.xp;
						((struct CHRFG *)pchr)->dbchr =
								vbchrMac - ffs.bchr + cbCHRV;
						((struct CHRFG *)pchr)->dcp += cpT + 1;
						bchrChp = -1; /* guarantee chr to follow */

				/* set up break opportunity to right*/
						vfli.chBreak = chDisplayField;
						vfli.cpMac = cpNext;
						vfli.xpRight = xp;
						vfli.ichMac = ich;
						bchrBreak = vbchrMac + cbCHRV;
						dyyAscentBreak = dyyAscentMac;
						dyyDescentBreak = dyyDescentMac;

						goto LVanish;
						}

					else  if (ffc == ffcWrap)
						goto LEndBreak0;

					else  if (ffc == ffcRestart)
						goto LRestartLine;
#ifdef DEBUG
					else
						Assert (fFalse);
#endif /* DEBUG */
					}  /* Field Character */

				/* else non-field fSpec character */
				/* place any chpFLDxa pre-processing here */
				else  if (*hpch == chTFtn || *hpch == chTFtnCont)
					chpFLDxa.fStrike = fTrue;

				}  /* fSpec Character */
#endif /* WIN */
#ifndef JR
			if (vfli.fOutline)
				OutlineChp(doc, &chpFLDxa);
#endif /* JR */
			if (bchrChp == -1 ||
#ifdef WIN
		/*	if previous font had overhang, always generate chrChp so we can adjust
	for overhang from previous font, unless it's just a continuation
			of a char run that will be one TextOut call in DisplayFliCore. */
			(vfti.dxpOverhang != 0 && bchrChp != bchrPrev) ||
#endif
					FNeChp(&chpFLDxa,
					&(((struct CHR *)(&(**vhgrpchr)[bchrChp]))->chp)))
				{
				int hps;

				Win(OverhangChr(&xp, bchrPrev,ich));
				bchrPrev = vbchrMac;
				Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
				if ((vbchrMac += cbCHR) > vbchrMax &&
						!FExpandGrpchr(cbCHR))
					{
					goto LEndBreak0;
					}
				else
					{
					bchrChp = bchrPrev;
#ifdef WIN
/* transform looks if fRMark is on */
/* play games with chpFLDxa if fRMark is on */
					if (chpFLDxa.fRMark)
						{
						struct DOD *pdod = PdodMother(doc);
						int irmProps = pdod->dop.irmProps;

						Assert(irmProps >= irmPropsNone && irmProps <= irmPropsDblUnder);
						switch (irmProps)
							{
						case irmPropsBold:
							chpFLDxa.fBold = fTrue;
							break;
						case irmPropsItalic:
							chpFLDxa.fItalic = fTrue;
							break;
						case irmPropsUnder:
							chpFLDxa.kul = kulSingle;
							break;
						case irmPropsDblUnder:
							chpFLDxa.kul = kulDouble;
							break;
							}
						}
#endif
					pchr = &(**vhgrpchr)[bchrChp];
					blt(&chpFLDxa, &pchr->chp, cwCHP);
					pchr->ich = ich;
					pchr->chrm = chrmChp;
					}
#ifdef MAC
/* large characters in symbol font may be synthesized from parts. Other
fSpec characters are pictures, auto footnote refs, date, time, etc. */
				if (chpFLDxa.fSpec || (chpFLDxa.ftc == ftcSymbol &&
						chpFLDxa.hps > hpsSymbolLast))
					{
					bchrChp = -1;
					if (vccpFetch != 1)
						{
						cpNext = vcpFetch + 1;
						docNext = doc;
						}
					hpchLim = hpch + 1;
/* this procedure will treat:
1. pictures: determine dimensions
2. true special characters: establish LoadFont, determine width.
3. compound characters: establish sepcial LoadFont, determine width.
	Set fSpec bits at pchr and chpFLDxa.
4. non-compound characters: establish LoadFont as if normal,
	but run is limited to 1 char!
*/
					vfli.rgch[ich] = *hpch;
					FormatChSpec(bchrPrev, ich, &dxp, &dxt,
							&dyyAscent, &dyyDescent);
					}
				else
					{
					LoadFont(&chpFLDxa, fTrue);
					dyyDescent.dyp = vfti.dypDescent;
					dyyAscent.dyp = vfti.dypAscent;
					}
/* fast open code if no heap, no scaling, no dxp/dxt difference */
				fFastWidth = vfti.wDenom == 0 && !fDxt;
/* see how this run will affect the line height */
				if ((hps = chpFLDxa.hpsPos) != 0) /* modify font for sub/super */
					if (hps < 128)
						{
/* note that we let the characters have 1 point grace to avoid changing the
line height in the normal case (12 pt font with 9 pt super 3 and sub 2 */
						dyyAscent.dyp += (hps = (hps >> 1) - 1);
					/*dyyDescent.dyp = max(0, dyyDescent.dyp - hps);*/
						}
					else
						{
						dyyDescent.dyp += (hps = ((256 - hps) >> 1) - 1);
					/*dyyAscent.dyp = max(0, dyyAscent.dyp - hps);*/
						}
#endif	/* MAC */
#ifdef WIN
				/* note LoadFont done even for special chars
					since vfti fields used later, and fcid from
					vfti is used in displayflicore.
				*/

				LoadFont( &chpFLDxa, fTrue );

			/* format special chars */
				if (chpFLDxa.fSpec)
					{
					vfli.rgch[ich] = *hpch++;
					FormatChSpec(bchrPrev, ich, &dxp, &dxt,
							&dyyAscent, &dyyDescent);
					if (!fDxt) dxt = dxp;
					}
				else
					{
					dyyDescent.dyp = vfti.dypDescent;
					dyyDescent.dyt = vftiDxt.dypDescent;
					dyyAscent.dyp = vfti.dypXtraAscent;
					dyyAscent.dyt = vftiDxt.dypXtraAscent;
					}
				/* Store away the fcid of the actual font obtained */
				/* so DisplayFli can request it directly */
				((struct CHR *) &(**vhgrpchr)[bchrChp])->fcid = vfti.fcid;

				if (FVisiSpacesFli && !vfli.fPrint && vfti.fVisiBad)
					vfli.fAdjustForVisi = fTrue;

/* see how this run will affect the line height */
				if ((hps = chpFLDxa.hpsPos) != 0) /* modify font for sub/super */
					{
					int dypT;
					if (hps < 128)
						{
/* FUTURE: If you tweak here to avoid changing line height, be sure
	to compensate in GetPictureInfo. Display fields, too? (bl)*/
						dyyAscent.dyp += (dypT = NMultDiv( hps,
								vfti.dypInch, 144 ));
					/* dyyDescent.dyp = max(0,dyyDescent.dyp - dypT); */
						dyyAscent.dyt += (dypT = NMultDiv( hps,
								vftiDxt.dypInch, 144 ));
					/* dyyDescent.dyt = max(0,dyyDescent.dyt - dypT); */
						}
					else
						{
						dyyDescent.dyp += (dypT = NMultDiv((256-hps),
								vfti.dypInch, 144) );
					/* dyyAscent.dyp = max( 0, dyyAscent.dyp - dypT ); */
						dyyDescent.dyt += (dypT = NMultDiv((256-hps),
								vftiDxt.dypInch, 144) );
					/* dyyAscent.dyt = max( 0, dyyAscent.dyt - dypT ); */
						}
					}
#endif	/* WIN */
				fHeightPending = fTrue;

				if (chpFLDxa.kul && !chpFLDxa.fVanish)
					{
					dyyDescent.dyp = max(dyyDescent.dyp,
							chpFLDxa.kul == kulDouble ? 
							WinMac( 3, 5 ) : WinMac( 1, 3));
					Win( dyyDescent.dyt = max(dyyDescent.dyt,
							chpFLDxa.kul == kulDouble ? 
							3 : 1 ) );
					}

				if (chpFLDxa.fSpec)
					{
					Win(bchrChp = -1);
					Mac(hpch++);
					ch = 'a';	/* this is bogus but safe */
					goto LHaveDxp;
					}
				} /* new chrmChp */
			} /* for (;;) */


/* take break opportunity at the left of the char at xp, ich, hpch */
LEndBreak0:
		vfli.dcpDepend = 0;
LEndBreak:
		vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
LEndBreakCp:
LEndBreakPara:
		vfli.xpRight = xp;
		vfli.ichMac = ich;
		pdodT = PdodDoc(vfli.doc);
		if (!pdodT->fShort && (vfli.fPageView || vfli.fLayout) && !fInTable && 
				!fAbs && ich < ichMaxLine && vfli.cpMac == caPara.cpLim)
			{
			CP cpPara = caPara.cpFirst;
			CacheSect(vfli.doc, vfli.cpMin);
			if (vfli.cpMac == caSect.cpLim - 1 /* not ccpEop */
					&& vfli.cpMac != CpMacDocEdit(vfli.doc) 
					&& !FInTableDocCp(vfli.doc, vfli.cpMac))
				{
				/* section mark in page view - uses no space in dr */
				vfli.rgch[vfli.ichMac] = chSpace;
				vfli.rgdxp[vfli.ichMac] = dxp = min(9, NMultDiv(xaRight, vfli.dxsInch, dxaInch) - vfli.xpRight);
				vfli.ichMac++;
				vfli.xpRight += dxp;
				vfli.fSplatBreak = fTrue;
				vfli.fParaAndSplat = fTrue;
				vfli.cpMac = caSect.cpLim;
				vfli.chBreak = chSect;
				}
			CachePara(vfli.doc, cpPara); /* need same para for vpapFetch.dya* */
			}
		bchrBreak = vbchrMac;

		dyyAscentBreak = dyyAscentMac;
		dyyDescentBreak = dyyDescentMac;

LEndJ:
/* justify (center, right flush) line and return */
#ifdef MAC

		dxp = max((fDxt ?
				NMultDiv(xtRight, vfli.dxsInch, vfli.dxuInch)
				: xtRight) - xpJust, 0);
#endif
#ifdef WIN
		dxp = max((fDxt ?
				NMultDiv(xtRight, vfti.dxpInch, vftiDxt.dxpInch)
				: xtRight) - xpJust, 0);
#endif
		switch (jc)
			{
		case jcCenter:
			dxp >>= 1;
			break;
		case jcLeft:
			goto LEndNoJ;
/* Following comment is preserved verbatim for eternity */
/* Rounding becomes a non-existant issue due to brilliant re-thinking */
/* "What a piece of work is man
	How noble in reason
	In form and movement,
	how abject and admirable..."

			Bill "Shake" Spear [describing Sand Word] */

		case jcBoth:
/* special justification algorithms to be performed outside of FormatLine */
/* ichSpace2 signals that justification is taking place, plus shows that
spaces with ich>=ichSpace2 are wider */
			vfli.ichSpace2 = vfli.ichSpace3;
/* what are auto hyphenation requirements ? */
			if (vfli.fSpecialJust)
				goto LEndNoJ;
LDivJust:
			Mac( vfli.cchSpace = cchSpace );
			if (cchSpace != 0)
				{
				int ddxp, dxpT = dxp / cchSpace;
				vfli.dxpExtra = dxpT;
#ifdef MAC
				if (vfli.fPrint && vfli.fFrac)
					vfli.dxpExtra = dxp;
#endif
				ddxp = dxpT + 1;
				cchWide = dxp - dxpT * cchSpace;
				if (fInhibitJust)
					{ 
					ddxp = 1; 
					cchWide = 0; 
					}
				for (ichT = ichSpace; ichT != ichNil; ichT = ichSpace)
					{
/* for all spaces (at ichT) do: */
					ichSpace = vfli.rgch[ichT];
					vfli.rgch[ichT] = FVisiSpacesFli ? chVisSpace :
							chSpace;
/* ichT1 is next space, rgch is now correct */
					if (ichT >= vfli.ichSpace3)
						{
/* exclude spaces above ichSpace3 from justification. */
						cchSpace--;
						goto LDivJust;
						}
					else
						{
/* last (first in list) cchWide spaces to be one longer than than rest */
						if (cchWide-- == 0)
							{
							ddxp--;
						/* ichT is last narrow */
							vfli.ichSpace2 = ichT + 1;
							}
						vfli.rgdxp[ichT] += ddxp;
						}
					}
				}
			else
				goto LJNoSpace;
			if (!fInhibitJust)
				vfli.xpRight += dxp;
			else
LJNoSpace:              
				vfli.ichSpace2 = ichNil;
			goto LEndNoJ;
/*      case jcRight: dxp = dxp */
			}

/* Note: xpLeft is always in points (screen units); xpRight, during printing,
	is in device units. The justification dxp is in xt's only for printing */
#ifdef MAC
		vfli.xpLeft += (!vfli.fPrint) ? dxp : NMultDiv(dxp, vfli.dxsInch, vfli.dxuInch);
#else
		vfli.xpLeft += dxp;
#endif
		vfli.xpRight += dxp;
LEndNoJ:


/* set line height and return */
LEnd:
#ifdef MAC
/* special check for balanced formulas */
		if (!fFormulaError)
			{
			if (cParen < 255)
				{
				fFormulaError = fFormulaVisi = fTrue; 
				goto LRestartLine;
				}
			}
		else
/* formulas with errors may be the continuation of previous lines */
			vfli.dcpDepend = dcpDependMax - 1;
#endif /* MAC */
#ifdef WIN
/* assure entire format group field was formatted */
		if (ffs.fFormatting)
			{
			struct CHR *pchrT = &(**vhgrpchr)[ffs.bchr];
			Assert (ffs.ifldWrap == ifldNil && ffs.ifldError == ifldNil);
			if (pchrT->ich != 0)
			/* force wrap */
				ffs.ifldWrap = ffs.ifldFormat;
			else
				ffs.ifldError = ffs.ifldFormat;
			goto LRestartLine;
			}
		vpffs = NULL;
#endif /* WIN */
/* caution due to possibility of a line consisting of non-printing chars
that do not set the line height. */
		if (dyyAscentBreak.dyp == 0)
			{
			if (dyyAscentMac.dyp == 0)
				{
				dyyDescentBreak = dyyDescent;
				dyyAscentBreak = dyyAscent;
				}
			else
				{
				dyyDescentBreak = dyyDescentMac;
				dyyAscentBreak = dyyAscentMac;
				}
			}
#ifdef WIN
		dytFont = 0;
		if (vfli.fPageView || vlm == lmPreview)
			{
			dytFont = dyyAscentBreak.dyt + dyyDescentBreak.dyt;
			vfli.dytLine = NMultDiv( abs(vpapFetch.dyaLine), vftiDxt.dypInch, czaInch );
			}
#endif

		vfli.dypBefore = dypBefore;
		vfli.dypFont = dyyAscentBreak.dyp + dyyDescentBreak.dyp;
/* negative dyaLine means value is maximum line height; else min */
		vfli.dypLine = DypFromDya(abs(vpapFetch.dyaLine));
		vfli.dypBase = dyyDescentBreak.dyp;

		if (vfli.fOutline)
			{
			vfli.dypLine = vfli.dypFont;
			Win( vfli.dytLine = dytFont );
			}
		else  if (vpapFetch.dyaLine >= 0)
			{
			vfli.dypLine = max(vfli.dypLine, vfli.dypFont);
			Win( vfli.dytLine = max( vfli.dytLine, dytFont ) );
			}
		else
			{
			vfli.dypFont = vfli.dypLine;
			vfli.dypBase = vfli.dypFont / 5;
			}

#ifdef WIN /* special case for fields & tables */
		if (vfli.ichMac == 0)
			vfli.dypLine = 0;
#endif /* WIN */

		vfli.dypLine += vfli.dypBefore;
		Win( vfli.dytLine += NMultDiv( dypBefore, vftiDxt.dypInch, vfti.dypInch ));
		vfli.dypAfter = 0;
		if ((vfli.cpMac == caPara.cpLim || vfli.fParaAndSplat) && !vfli.fOutline)
			{
			int dyp;
			int dyt;

			Win( dyt = NMultDiv( vpapFetch.dyaAfter, vftiDxt.dypInch, czaInch ) );
			dyp = DypFromDya(vpapFetch.dyaAfter);

			if (vfli.fBottomEnable)
				{
				vfli.fBottom = fTrue;
				Win( dyt += NMultDiv( vfli.dypBrcBottom, vftiDxt.dypInch, vfti.dypInch ) );
				dyp += vfli.dypBrcBottom;
				}
			vfli.dypLine += dyp;
			vfli.dypBase += dyp;
			vfli.dypAfter = dyp;
			Win( vfli.dytLine += dyt );
			}
#ifdef WIN	
	/* if !fDxt, don't have a printer but can still be in pagevw!  If no
			printer, dytLine == 0 */
		if (vfli.fPageView && fDxt)
			{
/* do not round, to avoid pushing screen text to next DR */
			int dypT = UMultDivNR( vfli.dytLine, vfti.dypInch, vftiDxt.dypInch );

			vfli.dypLine = dypT;
			}
#endif

/* reset pchr to break */
		Win(OverhangChr(&vfli.xpRight, bchrPrev,ich));
		pchr = &(**vhgrpchr)[vbchrMac = bchrBreak];
/* Now, enter chrmEnd in grpchr. Note: no need to check for sufficient space*/
		pchr->chrm = chrmEnd;
		pchr->ich = vfli.ichMac;

		Scribble(ispFormatLine,' ');
		Debug(vdbs.fCkFli ? CkVfli() : 0);
#ifdef WIN
		vrf.fInFormatLine = !vrf.fInFormatLine;
#endif /* WIN */
		return;
		}
#endif /* DEBUGORNOTWIN */

#ifdef MAC
/* D X P  F R O M  C H */
/* performs the general calculation of getting a dxp from a fti, ch pair.
Note: pfti always points to globals (hand native code assumption)
*/

/* %%Function:DxpFromCh %%Owner:NOTUSED */
	NATIVE int DxpFromCh(ch, pfti) /* WINIGNORE - MAC version */
			int ch; 
	struct FTI *pfti;
		{
		long dxpFrac = *(**(long far *far *far *) 0xb2a + ch);
		if (pfti->wDenom)
			{
			dxpFrac >>= 8;	/* sum of fractional parts = 16 */
			dxpFrac *= pfti->wNumer;
			}
		dxpFrac += pfti->dxuFrac;
		pfti->dxuFrac = (int) dxpFrac;	/* low word carried over */
		return(*(int *) &dxpFrac + pfti->dxuExpanded);
		}


/* L O A D  F O N T */
/* Plan:
	1. determine Mac ftc, catr, and ps (Font, Face, and Size)
		considering chp and possible font substitutions
		if fFormatAsPrint
	2. return if no change and if current font is still in core
	3. set graphport
	4. if fWidth: set up vfti with fonts width information
	5. if fWidth && fFormatAsPrint
		set up vftiDxt with printer's width table
		(see also FormatLine comments)

Font cache must be invalidated by:
	ftcLast = -1;
whenever
	fFormatAsPrint, fPrint,
	or printer setup changes that affect the font substitution or
		printer width table
change.
*/

/* %%Function:LoadFont %%Owner:NOTUSED */
	NATIVE LoadFont(pchp, fWidth) /* WINIGNORE - MAC version */
			struct CHP *pchp; 
	BOOL fWidth;
		{
		int catr, ftc, ps, b;
		int chLim;
		struct FONTREC far *qfontReal;
		long ldxpSpaceExtra;
		GRAFPORT far *wwptr;
		int fontinfo[4];

		GetPort(&wwptr);
	/* make sure we don't change either of the window mgr ports  - bad menus! */
		Assert(wwptr != *(GRAFPORT far *far *) 0x9de
				&& (!vfColorQD || wwptr != *(GRAFPORT far *far *) 0xD2C));
		ldxpSpaceExtra = wwptr->spExtra.l;
		wwptr->spExtra.l = 0;
		catr = 0;
/* we know that Bold, Italic, Outline and Shadow are in byte 0; Underline
is in byte 3 & 07 == 1
*/
		if ((b = *((char *)pchp)) != 0)
			{
			if (b & 0200/*fBold*/)
				catr = 1;
			if (b & 0100/*fItalic*/)
				catr += 2;
			if (b & 0020/*fOutline*/)
				catr += 8;
			if (b & 0010/*fShadow*/)
				catr += 16;
			}
		if ((b = *((char *)pchp + 7)) != 0)
			if ((b & 016) == (kulSingle << 1))
				catr += 4;
		ftc = pchp->ftc;
		ps = (uns) pchp->hps >> 1;

/* We have: ftc, catr, ps, fWidth */
/* check to see if font is already current */

		if ((ftcLast == ftc) &&
				(catrLast == catr) &&
				(psLast == ps) &&
				(vqqfontCur == qfmo->fontHandle) &&
				(*(long far *)vqqfontCur != NULL) &&
				(fWidthOK || !fWidth))
			goto LReturn;

		fWidthOK = fWidth;
		if (vfli.fFrac != vfli.fFracOn)
			{
			LastSpExtra = (long) -1;   /* blow font mgr's cache */
			vfli.fFracOn = vfli.fFrac;
			}
		fFracFont = (vfli.fFrac) ? PTRUE : fFalse; /* set fractional pixel widths */

		TextFont(ftcLast = ftc);
		TextFace(catrLast = catr);
		TextSize(psLast = ps);

/* Cause the font to be swapped in */
/* we TRIED to use FMSwapFont for this, but Apple recommends using CharWidth
	instead; that's how the ROM does it! */
		CharWidth(' ');
		vqqfontCur = qfmo->fontHandle;
		Assert(*((long far *)vqqfontCur) != 0l);

		if (fWidth || vfti.fHeap)
/* Set vfti up for DxpFromCh which replaces CharWidth in FormatLine */
			{

/* set up fields in vfti */
			vfti.fHeap = fFalse;
			vfti.chFirst = 0;
			vfti.cch = 255;
			vfti.dxuExpand0 = 0;
			vfti.wDenom = 0; /* assume no scaling */
			if ((vfti.wNumer = (**(struct GWT far *far *far *) 0xb2a)->hOutput) != 256)
				vfti.wDenom = 256;
			if (vfli.fFormatAsPrint)
				blt(&vfti, &vftiDxt, cwFTI);
			GetFontInfo(fontinfo);
			vfti.dypDescent = fontinfo[1];
			vfti.dypAscent = fontinfo[0];

			}
LReturn:
		wwptr->spExtra.l = ldxpSpaceExtra;
		vfti.dxuExpanded = vfti.dxuExpand0;
		if (!vfli.fPrint && vfli.fFormatAsPrint)
			{
			vfti.dxuExpanded += DxuExpand(pchp, vfli.dxsInch);
			vftiDxt.dxuExpanded = vftiDxt.dxuExpand0 +
					DxuExpand(pchp, vfli.dxuInch);
			}
		else
			vfti.dxuExpanded += DxuExpand(pchp, (vfli.fPrint) ? vfli.dxuInch : vfli.dxsInch);
		}

/* D X U  E X P A N D */
/* returns the character expansion number in dxuInch units */
/* %%Function:DxuExpand %%Owner:NOTUSED */
	NATIVE int DxuExpand(pchp, dxuInch) /* WINIGNORE - MAC version */
			struct CHP *pchp;
	int dxuInch;
		{
		int qps;

		if ((qps = pchp->qpsSpace) == 0 || pchp->fSpec) return 0;
/* qps is encoded to be in the range [-7, 56] */
		if (qps > 56) qps -= 64;
/* 4 (quarter) * 72 (points per inch) */
		return NMultDiv(qps, dxuInch, 72 * 4);
		}



#ifdef FUTURE
/* FUTURE: This will need to take a pchp instead of using vchpFetch (rp) */
/* F  F O R M A T  H Y P H */
/*
state at entry:
abcd__defghijX where X did not fit on the line
last break opportunity (in front of "d") is set up in vfli if chBreak != chNil
cpMac + dcpDepend is cp beyond X
ichMac points at d, ichLim points beyond X.
Plan:
1. determine extent of word around j
2. hyphenate word
	if hyphenation is not feasible, return.
3. we got the ich of a ch which should be "replaced" by a - in vfli.
This char should not be selectable, hence vfli.cpMac will not include it.
cpMac, xpRight, ichMac, bchrBreak in format's state will be
advanced to this point. Returns true iff change is indicated.
*/
/* %%Function:FFormatHyph %%Owner:NOTUSED */
	FFormatHyph(pbchrBreak, ichLim, pftiDxt)
			int *pbchrBreak, ichLim; 
	struct FTI *pftiDxt;
		{
		int ich, dich, chT, ichChr, iich;
		int dichMac;
		int chrm;
		int docT;
		int cchT;
		char *pch, *pchTo, *pchT;
		char far *qchT;
		struct CHR *pchr;
		CP cpWordLim;
		CP cpWordStart;
		CP cpChr, cpT;
		CP CpBestHypd();

		struct HYPD hypd;
/* 1. */
		pch = &vfli.rgch[vfli.ichMac];
		pchTo = &hypd.rgch[0];
		dichMac = ichLim - vfli.ichMac - 1;
		for (dich = 0; dich < dichMac; dich++)
			{
			chT = *pch++;
			if (!FUpper(chT) && ! FLower(chT))
				goto LEndWord;
			*pchTo++ = chT;
			}
/* continue fetching remainder of word starting at cpMac+dcpDepend */
		docT = vfli.doc;
		cpT = vfli.cpMac + vfli.dcpDepend - 1;
		for (;;)
			{
			FetchCp(docT, cpT, fcmChars + fcmProps);
			docT = docNil;
			cpT += vccpFetch;
			if (vchpFetch.fSysVanish ||
					(vchpFetch.fVanish && !vpref.fSeeHidden)) continue;
			qchT = LpFromHp(vhpchFetch);
			cchT = vccpFetch;
			while (cchT-- > 0)
				{
				chT = *qchT++;
				if (!FUpper(chT) && !FLower(chT))
					goto LEndWord;
				*pchTo++ = chT;
				dich++;
				}
			}
LEndWord:
		hypd.ichMac = dich;
/* 2. */
		HyphenateWord(&hypd, vfHyphCapitals);
/* find the best (last) acceptable hyphenation point */

		for (iich = 0; iich < hypd.iichMac; iich++)
			{
/* break if hyphen iich does not fit */
			ich = hypd.rgich[iich]; /* - after char ich */
			if (ich >= dichMac - 1 ||
					(ich == dichMac - 2 &&
		/* assume that - is wider than any 2 characters */
			DxpFromCh(chHyphen, pftiDxt) >
					DxpFromCh(vfli.rgch[vfli.ichMac + ich + 1], pftiDxt)))
				break;
			}
		if (iich == 0) goto NoHyph;
		hypd.iichBest = iich - 1;
		if (FHypdWidow(&hypd)) goto NoHyph;
		dich = hypd.rgich[hypd.iichBest] + 1;

/* 3. */
/* compute cpMac, bchr corresponding to ich = vfli.ichMac + dich
*/
		cpChr = vfli.cpMac;
		ichChr = vfli.ichMac;
		ichLim = ichChr + dich;
		while (*pbchrBreak < vbchrMac)
			{
			pchr = &(**vhgrpchr)[*pbchrBreak];
			chrm = pchr->chrm;
			if (chrm == chrmVanish)
				cpChr += ((struct CHRV *)pchr)->dcp;
			else
				{
				cpT = cpChr + pchr->ich - ichChr;
				if (pchr->ich >= ichLim)
					break;
				cpChr = cpT;
				}
			ichChr = pchr->ich;
			*pbchrBreak += chrm;
			}
/* now ichLim is in the "break" run */
		vfli.cpMac = cpChr + ichLim - ichChr;
		vfli.rgch[ichLim] = chHyphen;
	/* where do properties come from? */
		vfli.rgdxp[ichLim] = DxpFromCh(chHyphen, &vfti);
		ich = vfli.ichMac;
		vfli.ichMac = ichLim + 1;
		for (; ich < vfli.ichMac; ich++)
			vfli.xpRight += vfli.rgdxp[ich];
		vfli.ichSpace3 = vfli.ichMac - 1;
		return fTrue;
NoHyph:
		return fFalse;
		}
#endif

#endif /* MAC */

#ifdef WIN

#ifdef DEBUG
/* Given char, ptr to fti, return width of char */
/* D X P  F R O M  C H */
/* %%Function:C_DxpFromCh %%Owner:bryanl */
	HANDNATIVE int C_DxpFromCh( ch, pfti )
			int ch;
	struct FTI *pfti;
		{
		return pfti->rgdxp [ch] + pfti->dxpExpanded;
		}
#endif /* DEBUG */

#ifdef DEBUG
/* D X U  E X P A N D */
/* returns the character expansion number in dxuInch units */
/* %%Function:C_DxuExpand %%Owner:bryanl */
	int C_DxuExpand(pchp, dxuInch)
		struct CHP *pchp;
	int dxuInch;
		{
		int qps;

		if ((qps = pchp->qpsSpace) == 0 || pchp->fSpec) return 0;
/* qps is encoded to be in the range [-7, 56] */
		if (qps > 56) qps -= 64;
/* 4 (quarter) * 72 (points per inch) */
		return NMultDiv(qps, dxuInch, 72 * 4);
		}
#endif /* DEBUG */

#endif /* WIN */


#ifdef DEBUGORNOTWIN
/* X P  F R O M  D C P */
/* returns the xpFirst integrated from the beginning of the line until the
front of the char with cp>=cpMin+dcp1 with vanished runs taken into
account. Continues integration to dcp2 with the position returned as the
result.
Assumes dcp's <= dcpMac of the line (no check for chrmEnd.)
*/

#ifdef MAC
/* %%Function:XpFromDcp %%Owner:NOTUSED */
	NATIVE int XpFromDcp(dcp1, dcp2, pxpFirst, pich) /* WINIGNORE - "C_" in WIN */
#else /* WIN */
		/* %%Function:C_XpFromDcp %%Owner:bryanl */
	HANDNATIVE int C_XpFromDcp(dcp1, dcp2, pxpFirst, pich)
#endif /* WIN */
			CP dcp1, dcp2;
	uns *pxpFirst;
	int *pich;
		{
		uns *pdxp = &vfli.rgdxp[0];
		int ich, ichNext, xp, chrm;
		BOOL fFirst = fTrue;
		CP dcp = dcp1; /* modified by vanish */
		struct CHR *pchr = &(**vhgrpchr)[0];
		int xpCeil; /* mac xp seen during scan before formula moves */
		int xpBeforeFG;

		Assert(vfli.ww != wwNil);
		ich = 0;
		xpCeil = xp = vfli.xpLeft;

		for (;;)
			{
			xpBeforeFG = -1;

			while (pchr->ich == ich)
				{
				if ((chrm = pchr->chrm) == chrmVanish)
					dcp -= ((struct CHRV *)pchr)->dcp;
#ifdef WIN
				else  if (chrm == chrmDisplayField)
					{
/* chrmDisplayField has a vfli.rgdxp entry, which takes care of width */
/* -1 adjusts for the vfli.rgch entry that corresponds to the dxp     */
					dcp -= ((struct CHRDF *)pchr)->dcp - 1;
					}
				else  if (chrm == chrmFormatGroup)
					{
					struct CHR *pchrNew = (char *)pchr + 
					((struct CHRFG *)pchr)->dbchr;
					int dich = pchrNew->ich - ich;
					ich += dich;
					dcp -= ((struct CHRFG *)pchr)->dcp - dich;
					pdxp = &vfli.rgdxp[ich];
					xpBeforeFG = xp;
					xp += ((struct CHRFG *)pchr)->dxp;
					pchr = pchrNew;
					break;
					}

				Assert (chrm != chrmFormula); /* hidden by formatgroup */
#else  /* MAC */
else  if (chrm == chrmFormula)
{
	xpCeil = max(xpCeil, xp);
	xp += ((struct CHRF *)pchr)->dxp;
}


#endif
if (chrm == chrmEnd) break;
(char *)pchr += chrm;
				}
			if ((CP) ich >= dcp)
				{
				if (fFirst)
					{
					fFirst = fFalse;
#ifdef MAC
/* the if is necessary to take care of chrmFormulas at the beginning of lines */
					*pxpFirst = max(dcp1 == 0 ? vfli.xpLeft : xp, xpCeil);
#else
					*pxpFirst = dcp1 == 0 ? vfli.xpLeft : 
							xpBeforeFG == -1 || ich == dcp ? 
							xp : xpBeforeFG;
#endif /* MAC */
/* correct dcp2 by the vanish already seen: dcp = dcp2 - (dcp1 - dcp) */
					dcp += dcp2 - dcp1;
					continue;
					}
				else
					{
					*pich = ich;
#ifdef MAC
					return max(xp, xpCeil);
#else
					return xpBeforeFG == -1 || ich == dcp ?
							xp : xpBeforeFG;
#endif /* MAC */
					}
				}
/* note how chrm is cb of the variant chr structure! */
			if (chrm == chrmEnd)
				{
#ifdef	DEBUG
				struct WWD *pwwdT = PwwdWw(vfli.ww);
				Assert(vfli.fOutline || pwwdT->fPageView);
#endif
				if (fFirst)
					*pxpFirst = xp;
				*pich = ich;
				return xp;
				}
/* scan until dcp expires or beginning of chr is reached */
			ichNext = CpMin((CP) pchr->ich, dcp);
/* next while loop is speed critical section */
			while (ich < ichNext)
				{
				xp += *pdxp++;
				ich++;
				}
			}
		}
#endif /* DEBUGORNOTWIN */

#ifdef WIN
/* O v e r h a n g  C h r */
/* Called before entering new chr. If previous chr was a chrmChp with
	an overhang, adjust width of final char to include the overhang. */
/* This is to avoid clipping off the overhang when doing blanking on 
	screens under WINDOWS */

/* %%Function:OverhangChr %%Owner:bryanl */
	OverhangChr(pxp, bchrPrev, ich)
			int *pxp;
		{
		struct CHR *pchr;

		if (vbchrMac == 0)
			return;

		pchr = &(**vhgrpchr)[bchrPrev];

		if (!vfli.fPrint && pchr->chrm == chrmChp && ich > 0 && ich > pchr->ich)
			{
			vfli.rgdxp [ich - 1] += vfti.dxpOverhang;
			*pxp += vfti.dxpOverhang;
			}
		}
#endif

#ifdef	MAC
/***************************/
/* %%Function:LpInitFormat %%Owner:NOTUSED */
	native long LpInitFormat(lpmh4) /* WINIGNORE - unused in WIN */
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
