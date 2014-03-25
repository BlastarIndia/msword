/* F O R M U L A . C */

#ifdef MAC
#include "syscodes.h"
#include "toolbox.h"
#endif /* MAC */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "format.h"
#include "formula.h"
#include "ch.h"
#include "inter.h"
#include "print.h"
#include "debug.h"

#ifdef MAC
#include "mac.h"
#endif

#ifdef BRYANL
#define DFORMULA
#endif

/* E X T E R N A L S */
extern char             (**vhgrpchr)[];
extern int              vbchrMax;
extern int              vbchrMac;

extern struct CHP       vchpFetch;
extern char HUGE        *vhpchFetch;
extern int              vccpFetch;

extern int              vifmaMac;
extern struct FMA       rgfma[];
extern struct MERR      vmerr;

extern struct FLI       vfli;
extern struct CA        caPara;

extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct PRSU      vprsu;
extern struct ITR       vitr;
extern int              catrLast;
extern struct PRI	    vpri;
#ifdef MAC
extern int              psLast;
extern BOOL             fWidthOK;
extern int		vdxuFrac;
extern struct PREF      vpref;
#endif /* MAC */

struct CHP              *PchpSetChrDyp();


#define DxpFromHps(hps)	NMultDiv( (hps), vfti.dxpInch, 2 * 72 )
#define DxtFromHps(hps)	NMultDiv( (hps), vftiDxt.dxpInch, 2 * 72 )

struct FAPB /* Format Array Parameter Block */
	{
	/* arguments */
	struct FMA * pfma;
	int cfma;
	int ccol;
	int hpsHSpace;
	int hpsVSpace;
	/* return values */
	int dxp;
	int dxt;
	int dypFract;
	int dytFract;
	int dypAscent;
	int dytAscent;
	int dypDescent;
	int dytDescent;
};

/* NOTE:
        The following two arrays (rgwFmaCmds and rgwFmaOpts) must be kept
        in alphabetical order and must match the #define's in formula.n
*/

	csconst int rgwFmaCmds[] = { 
	'A', 'B', 'D', 'F', 'I', 'L', 'O', 'R', 'S', 'X' 	};


#define cCmds (sizeof(rgwFmaCmds) / sizeof(int))

#define WFromChCh(ch1, ch2) (int)((((ch1) & 0xff) << 8) + ((ch2) & 0xff))

csconst int rgwFmaOpts[] = {
	WFromChCh('A', 'C'),
			WFromChCh('A', 'I'),
			WFromChCh('A', 'L'),
			WFromChCh('A', 'R'),
			WFromChCh('B', 'A'),
			WFromChCh('B', 'C'),
			WFromChCh('B', 'O'),
			WFromChCh('C', 'O'),
			WFromChCh('D', 'I'),
			WFromChCh('D', 'O'),
			WFromChCh('F', 'C'),
			WFromChCh('F', 'O'),
			WFromChCh('H', 'S'),
			WFromChCh('I', 'N'),
			WFromChCh('L', 'C'),
			WFromChCh('L', 'E'),
			WFromChCh('L', 'I'),
			WFromChCh('P', 'R'),
			WFromChCh('R', 'C'),
			WFromChCh('R', 'I'),
			WFromChCh('S', 'U'),
			WFromChCh('T', 'O'),
			WFromChCh('U', 'P'),
			WFromChCh('V', 'C'),
			WFromChCh('V', 'S') };


#define cOpts (sizeof(rgwFmaOpts) / sizeof(int))

csconst long mpfmtmsk [] = {
	mskArray,	mskBracket,	mskDisplace,	mskFraction,
			mskIntegral,	mskList,	mskOver,	mskRoot,
			mskSubSuper,	mskBox };


/* P C H R  N E W

    Builds a chr of type chrm in vgrpchr. b is available in vbchrMac
    before the call.  Return 0 if no room.  Assigns chrm and ich in chr.
*/

/* %%Function:PchrNew %%Owner:krishnam */
struct CHR * PchrNew(chrm, ich, pbchr)
int chrm;
int *pbchr;
{
	struct CHR *pchr;
	int bchr = vbchrMac;
	if ((vbchrMac += chrm) > vbchrMax)
		if (!FExpandGrpchr(chrm))
			return 0;
	pchr = &((**vhgrpchr)[*pbchr = bchr]);
	pchr->ich = ich;
	pchr->chrm = chrm;
	return pchr;
}


/* W  P A R S E  I N T

    Parse a signed number from the doc starting at *pcp.  Return the
    number and set *pcp to the point past the number.  Leading blanks
    are ignored.
*/

/* %%Function:WParseInt %%Owner:krishnam */
WParseInt(doc, pcp)
int doc;
CP * pcp;
{
	int w, ch;
	CP cp;
	BOOL fNeg;

	w = 0;
	cp = CpFirstNonBlank(doc, *pcp);
	if (fNeg = ((ch = ChFetchNonVanish(doc, &cp)) == '-'))
		{
		cp++;
		ch = ChFetchNonVanish(doc, &cp);
		}
	while (FDigit(ch))
		{
		cp++;
		w = w * 10 + ch - '0';
		ch = ChFetchNonVanish(doc, &cp);
		}

	*pcp = cp;
	return fNeg ? -w : w;
}



/* F  N E W  C H R F  C H R

    Create a CHRF followed by a CHR, return fTrue if sucessful.  Also,
    put ch in vfli.rgch.
*/

/* %%Function:FNewChrfChr %%Owner:krishnam */
BOOL FNewChrfChr(ich, ch, pchp, fLoad)
int ich;
char ch;
struct CHP *pchp;
BOOL fLoad;
{
	struct CHRF *pchrf;
	struct CHR *pchr;
	int bchr;

	if (!FRareF(324, pchrf = PchrNew(chrmFormula, ich, &bchr)))
		return fFalse;
	pchrf->dxp = 0; /* fixed later */
	pchrf->dyp = 0; /* fixed later */
	pchrf->fLine = fFalse;

	if (!FRareF(324, pchr = PchrNew(chrmChp, ich, &bchr)))
		return fFalse;
	pchr->chp = *pchp;
	vfli.rgch[ich] = ch;
	if (fLoad)
		{
		LoadFont(pchp, fTrue);	/* HM (WIN) */
		Win(((struct CHR *) &((**vhgrpchr)[bchr]))->fcid = vfti.fcid);
		vfli.rgdxp[ich] = DxpFromCh(ch, &vfti);
		}

	return fTrue;
}


/* F O R M A T  F O R M U L A */
/*

Plan:
we just encountered the chFormula escape character or comma or right paren.
If it was chFormula, the next char is: F, R, S, I, B, O, L

pfmal contains:
        xt, xp, ich: next available position.
        cp: points to char following chFormula.


begin actions include some or all of the following steps:
        1. next character is chFmtt.
        2. create CHRV covering command
        3. create CHR and CHRF covering symbol(s)
        4. store symbol(s) in rgdxp, rgch, etc. with current width
        5. create CHRF which will be updated later.
        6. create FMA with ich, xp, xt, dypAscent, dypDescent, fmt, fmtt,
                bchr and return xp, xt, ich, etc.

comma action is: (a subset of the begin actions)
        2. create CHRV covering chFormula and the comma
        5. create CHRF which will be updated later.
        6. create FMA with ich, xp, xt, dypAscent, dypDescent, fmt, fmtt,
                bchr and return xp, xt, ich, etc.

the end actions include some or all of the following steps, depending on
the fmt found in step 1.
        2. create CHRV covering chFormula and parenthesis
        5. create CHRF which will be updated later.
        10. find begin action on the stack, count operands.
                if none/wrong number: behave as in visible mode.
        11. reset stack
        12. perform formatting action for fmt adjusting CHR's CHRF's etc.
        13. return xp, xt, ich, etc.
*/

/* %%Function:FormatFormula %%Owner:krishnam */
EXPORT FormatFormula(pfmal)
struct FMAL *pfmal;
{
	int doc;
	int fmt;
	struct FMA *pfma1, *pfma2, *pfma3, *pfmaEnd;
	int cfma;
	int ifma;
	int bchr;
	struct CHRF *pchrf;
	struct CHRV *pchrv;
	int bchrT;
	struct CHR *pchr;
	int ch1 = 0, ch2 = 0;
	int ch;
	int dxp1, dxp2, dxp3, dyp1, dyp2, dyp3;
	int dypAscent, dypDescent;
	int dytAscent, dytDescent;
	int dypAscentCh1, dypDescentCh1, dypAscentCh2, dypDescentCh2;
	int dytAscentCh1, dytDescentCh1, dytAscentCh2, dytDescentCh2;
	int dxt1, dxt2, dxt3, dyt1, dyt2, dyt3;
	int dxpLeft1, dxpLeft2, dxpFract, dypFract, dytFract;
	int dxpChSym, dypChSym, dxtChSym, dytChSym;
	int ich;
	int dxpLarger, dxtLarger;
	int dxpAdj, dxtAdj, dypAdj, dytAdj;
	int dyuWinHack;
	BOOL f1 = fFalse;
	BOOL f3 = fFalse;
	BOOL f6 = fTrue;
	CP cp;
	struct FAPB fapb;
	long msk;
	struct CHP chp1, chp2;
	struct FMA fma;  /* this is the fma that is stacked */

	if (vmerr.fMemFail)
		goto LError;

	pfmal->fError = fFalse;
	pfmal->fLiteral = fFalse;
	fma = pfmal->fma;

	cp = pfmal->cp;
	doc = pfmal->doc;
/* see how we got here; either escape, right parenthesis, or comma */
	if ((ch = vfli.rgch[fma.ich]) != chFormula)
		{
		if (ch == ')')
			{
			fma.fmt = fmtEnd;
			f6 = fFalse;
			goto LHaveFmt;
			}
		if (ch == vitr.chList)
			{
			fma.fmt = fmtComma;
			goto LHaveFmt;
			}
#ifdef DFORMULA
		CommSzNum("*** Formula error: Illegal entry to FormatFormula(): ", ch);
#endif
		Assert(fFalse);
		goto LError;
		}

/* handle escaped characters, either commands, commas or right parens */
/* cp points to character after escape */
	ch = ChFetchNonVanish(doc, &cp);
	++cp; /* point to ch following command letter */

/* check for escaped commas, ) and ( */
	if (ch == vitr.chList || ch == ')' || ch == '(' || ch == '\\')
		{
		/* vanish chFormula and return fLiteral set */
		if (!FRareF(513, pchrv = PchrNew(chrmVanish, pfmal->ich, &bchrT)))
			goto LError;
		pchrv->dcp = 1;
		pfmal->fLiteral = fTrue;
		goto LRestoreFont;
		}

/* get middle of ascent for fraction, matrix, and integral */
	LoadFont(&vchpFetch, fTrue);
	fma.dypFract = vfti.dypAscent / 2;
	fma.dytFract = vftiDxt.dypAscent / 2;

/* look for command in table */
	if ((fmt = IScanLprgw( (int far *)rgwFmaCmds, ChUpper(ch), cCmds )) == iNil)
		{
#ifdef DFORMULA
		CommSz(SzShared("*** Formula error: unknown fmt\r\n"));
#endif
		goto LError;
		}

	fma.fmt = fmt;
	msk = mpfmtmsk[fmt];

/* we have entered a formula function, so cParen is 1 */
	pfmal->cParen = 1;

/* 1. parse arguments */
		{
		char rgch[10];
		BOOL fLC = fFalse, fRC = fFalse;
		int opt;

		/* set up common defaults */
		fma.jc = jcCenter;
		fma.fLineOrVar = fFalse;
		fma.fInline = fFalse;
		StandardChp(&chp1);
		chp1.ftc = ftcSymbol;
		chp1.ico = vchpFetch.ico;
		chp2 = chp1;
		fma.lOptions = ch1 = ch2 = 0;

		/* set up individual defaults */
		switch (fmt)
			{
		case fmtBracket:
			chp1.ftc = chp2.ftc = vchpFetch.ftc;
			f3 = fTrue;
			break;

		case fmtIntegral:
			fma.chLeft = ch1 = chIntegral;
			fma.fLineOrVar = fTrue;
			f3 = fTrue;
			break;

		case fmtRoot:
			fma.chLeft = ch1 = chRoot;
			ch2 = chRootExt;
			f3 = fTrue;
			break;

		case fmtArray:
			fma.cCol = 1;
			break;

		case fmtSubSuper:
			fma.hpsVMove = hpsSubSuperDef;
			fma.dhpsAscent = 0;
			fma.dhpsDescent = 0;
			fma.jc = jcLeft;
			/*break;*/
			}

		for (;;)
			{
			int ich;

			cp = CpFirstNonBlank(doc, cp);
			ch = ChFetchNonVanish(doc, &cp);
			cp++;
			/* check for done */
			if (ch == '(')
				{
				/* we are done */
				break;
				}

			/* check for escape */
			if (ch != chFormula)
				{
#ifdef DFORMULA
				CommSzNum("*** Formula error: expected chFormula: ", ch);
#endif
				goto LError;
				}

/* check if previous option expects an escaped character and deal with it */
			if (fLC || fRC)
				{
				if (cp >= caPara.cpLim - ccpEop) goto LError;
				FetchCp(doc, cp++, fcmChars + fcmProps);

				if (fLC)
					{
					fma.chLeft = ch1 = *vhpchFetch;
					chp1 = vchpFetch;
					}

				if (fRC)
					{
					ch2 = *vhpchFetch;
					if (fLC)
						{
						switch (ch1)
							{
						case '(':
							ch2 = ')';
							break;
						case '[':
							ch2 = ']';
							break;
						case '{':
							ch2 = '}';
							break;
						case '<':
							ch2 = '>';
							break;
						case 225: /* symbol < */
							if (vchpFetch.ftc == ftcSymbol)
								ch2 = 241;
							break;
							}
						}
					fma.chRight = ch2;
					chp2 = vchpFetch;
					fRC = fFalse;
					}

				fLC = fFalse;

				continue;
				}

			/* collect alpha characters of token */
			for (ich = 0;; ++cp)
				{
				ch = ChFetchNonVanish(doc, &cp);
				if (!(FLower(ch) || FUpper(ch))) break;
				if (ich < sizeof(rgch))
					rgch[ich++] = ch;
				}

/* Error Checking:
   1. Make sure we have at least two characters
   2. And that they make up a valid option
   3. And that the option is valid for this command
*/
			if (ich < 2 ||
					(opt = IScanLprgw( (int far *)rgwFmaOpts,
					WFromChCh(ChUpper(rgch[0]), ChUpper(rgch[1])),
					cOpts)) == iNil ||
					(msk & (1L << opt)) == 0)
				{
#ifdef DFORMULA
				CommSz(SzShared("*** Formula error: invalid options\r\n"));
#endif
				goto LError;
				}

			switch (opt)
				{
			case optAC:
				fma.jc = jcCenter;
				break;

			case optAI:
					{
					int w = WParseInt(doc, &cp) * 2;
					if (w < 0 || w > 254)
						goto LError;
					fma.dhpsAscent = w;
					}
				break;

			case optAL:
				fma.jc = jcLeft;
				break;

			case optAR:
				fma.jc = jcRight;
				break;

			case optBA:
				fma.hpsHMove = -WParseInt(doc, &cp) * 2;
				break;

			case optBC:
				fLC = fRC = fTrue;
				break;

			case optBO:
				fma.fBottom = fTrue;
				break;

			case optCO:
				fma.cCol = min(ifmaMax - 1, 
						max(1, WParseInt(doc, &cp)));
				break;

			case optDI:
					{
					int w = WParseInt(doc, &cp) * 2;
					if (w < 0 || w > 254)
						goto LError;
					fma.dhpsDescent = w;
					}
				break;

			case optDO:
				fma.hpsVMove = WParseInt(doc, &cp) * 2;
				break;

			case optFC:
				fma.fLineOrVar = fFalse;
				fLC = fTrue;
				break;

			case optFO:
				fma.hpsVMove = WParseInt(doc, &cp) * 2;
				break;

			case optHS:
					{
					int w = WParseInt(doc, &cp) * 2;
					if (w < 0 || w > 254)
						goto LError;
					fma.hpsHSpace = w;
					}
				break;

			case optIN:
				fma.fInline = fTrue;
				break;

			case optLC:
				fLC = fTrue;
				break;

			case optLE:
				fma.fLeft = fTrue;
				break;

			case optLI:
				fma.fLineOrVar = fTrue;
				break;

			case optPR:
				fma.chLeft = ch1 = chProduct;
				break;

			case optRC:
				fRC = fTrue;
				break;

			case optRI:
				fma.fRight = fTrue;
				break;

			case optSU:
				fma.chLeft = ch1 = chSumma;
				break;

			case optTO:
				fma.fTop = fTrue;
				break;

			case optUP:
				fma.hpsVMove = -WParseInt(doc, &cp) * 2;
				break;

			case optVC:
				fma.fLineOrVar = fTrue;
				fLC = fTrue;
				break;

			case optVS:
					{
					int w = WParseInt(doc, &cp) * 2;
					if (w < 0 || w > 254)
						goto LError;
					fma.hpsVSpace = w;
					}
				break;

			default:
				Assert(fFalse);
				goto LError;
				} /* switch (opt) */
			} /* for (;;) */

		if (fmt == fmtBracket && (ch1 | ch2) == 0)
			{
			fma.chLeft = ch1 = '(';
			fma.chRight = ch2 = ')';
			}

		} /* end of option parser */


LHaveFmt:

/* 2. create CHRV */
	if (!FRareF(514, pchrv = PchrNew(chrmVanish, pfmal->ich, &bchrT)))
		goto LError;
	pchrv->dcp = cp - pfmal->cp + 1 - (ch1 != 0) - (ch2 != 0);
/* +1 because we need to vanish the char that got us here as well */
/* -.. because ch's generate ich's in vfli. See xp->cp mapping */

/* skip over those characters that are vanished */
	pfmal->cp = cp;

/* 3.4. create CHRFs and CHRs covering ch1/chp1 and ch2/chp2 */
	if (f3)
		{
		if (ch1 != 0 && !FNewChrfChr(pfmal->ich++, ch1, &chp1,
				!fma.fLineOrVar /* fLoadFont */))
			goto LError;
		if (ch2 != 0 && !FNewChrfChr(pfmal->ich++, ch2, &chp2,
				fFalse /* fLoadFont */))
			goto LError;
		pfmal->bchrChp = -1;
		fma.ich = pfmal->ich;
		}

/* 5. create CHRF for following argument or end */
	if (!FRareF(515, pchrf = PchrNew(chrmFormula, pfmal->ich, &fma.bchr)))
		goto LError;
	pchrf->dxp = 0; /* in case there is no fmtEnd to fix it up */
	pchrf->dyp = 0;
	pchrf->fLine = fFalse;
#ifdef MAC
	/* reset fractional width because of Move() at display time */
	if (!vpref.fShowP)
		vfti.dxuFrac = vftiDxt.dxuFrac = vfli.dxuFrac;
#endif

/* 6. find operands and create FMA */

	for (cfma = 1, ifma = vifmaMac; ifma != 0; cfma++)
		if ((fmt = rgfma[--ifma].fmt) != fmtComma) break;

	pfma1 = &rgfma[ifma];   /* in front of first operand */
	pfma2 = pfma1 + 1;      /* in front of second operand */
	pfma3 = pfma2 + 1;      /* in front of third operand */
	pfmaEnd = &fma;         /* after last operand */

/* stack fma */
	if (vifmaMac >= ifmaMax)
		{
#ifdef DFORMULA
		CommSz("*** Formula error: too many fma's\r\n");
#endif
		goto LError;
		}
	rgfma[vifmaMac++] = fma;

/* underlying operator fmt is at ifma. cfma is the number of operands
including the one at ifma plus one for every comma.
*/
	if (f6)
		{
/* we have a begin parent or a comma, set up return values */
		if (fma.fmt == fmtComma)
			{
			if (vifmaMac == 1)
				{
#ifdef DFORMULA
				CommSz(SzShared("*** Formula error: illegal use of comma\r\n"));
#endif
				goto LError;
				}
/* for comma: Reset operand height, reset xt, xp depending on fmt */
			switch (fmt)
				{
			case fmtIntegral:
				if (cfma != 1) /* 1st 2 op's stacked */
					break;
				/* fall through */

			case fmtArray:
			case fmtSubSuper:
			case fmtFract:
			case fmtOver:
LResetWidth:
				pfmal->xt = pfma1->xt;
				pfmal->xp = pfma1->xp;
				/*break;*/
				}
			}
/* else begin parent. Reset operand height. xt, xp, unchanged.
*/
		pfmal->dypAscent = pfmal->dypDescent = 0;
		pfmal->dytAscent = pfmal->dytDescent = 0;
		goto LRestoreFont;
		}

/* 10. reset stack, we have reached the end */
	if (vifmaMac <= 1)
		{
#ifdef DFORMULA
		CommSz(SzShared("*** Formula error: stack problem\r\n"));
#endif
		goto LError;
		}
	vifmaMac = ifma;
	pfmal->cParen = rgfma[ifma].cParen;

/* there are cfma stacked operands, vifmaMac points to the first of them.
They are all effectively off the stack now.
*/

	switch (fmt)
		{

	default:
		Assert(fFalse);
		break;

	case fmtList:
		pfmal->dypAscent = max(pfma1->dypAscent, pfmaEnd->dypAscent);
		pfmal->dypDescent = max(pfma1->dypDescent, pfmaEnd->dypDescent);
		pfmal->dytAscent = max(pfma1->dytAscent, pfmaEnd->dytAscent);
		pfmal->dytDescent = max(pfma1->dytDescent, pfmaEnd->dytDescent);
		break;

	case fmtFract:
		if (cfma != 2)
			{
#ifdef DFORMULA
			CommSzNum(SzShared("*** Formula error: wrong number of args: "), cfma);
#endif
			goto LError;
			}
/* Fractions plan:
        dxp1 = width of numerator
        dxp2 = width of denominator
        dyp1 = height of numerator
        dyp2 = height of denominator
        1st CHRF: goto start of numerator
        2nd: goto start of denominator
        3rd: goto start of fraction line
        4th: goto end of fraction line
        5th: goto start of rest.
fraction line is dypFract above the base line (func of hps of *f command)
*/
		dxp1 = pfma2->xp - pfma1->xp;
/* left edge of denominator is set equal to start of numerator */
		dxp2 = pfmaEnd->xp - pfma1->xp;

		dyp1 = pfma2->dypAscent + pfma2->dypDescent;
		dyt1 = pfma2->dytAscent + pfma2->dytDescent;
		dyp2 = pfmaEnd->dypAscent + pfmaEnd->dypDescent;
		dyt2 = pfmaEnd->dytAscent + pfmaEnd->dytDescent;

		dxpFract = max(dxp1, dxp2);
		dypFract = pfma1->dypFract;
		dytFract = pfma1->dytFract;
/* 1st CHRF: */
		SetChrf(pfma1->bchr, dxpLeft1 = (dxpFract - dxp1) / 2,
				-dypFract - pfma2->dypDescent, fFalse);
/* 2nd CHRF: */

		SetChrf(pfma2->bchr, -dxp1 - dxpLeft1 + (dxpLeft2 =
				(dxpFract - dxp2) / 2),
				pfma2->dypDescent + pfmaEnd->dypAscent + 1, fFalse);
/* 3rd CHRF: */
		SetChrf(pfmaEnd->bchr, -dxp2 - dxpLeft2,
				-pfmaEnd->dypAscent - 1, fFalse);
/* 4th CHRF: */
		PchrNew(chrmFormula, pfmal->ich, &bchrT);
		SetChrf(bchrT, dxpFract, 0, fTrue);
/* 5th CHRF: */
		PchrNew(chrmFormula, pfmal->ich, &bchrT);
		SetChrf(bchrT, 0, dypFract, fFalse);
/* set height, xt, xp following the construction */
/* WIN HACK: Allow extra space for fraction bar on printer, so height of
   line doesn't get clipped in page view. */
		Win( dyuWinHack = UMultDiv( 1 * 20, vfli.dyuInch, czaInch ));
		pfmal->dypAscent = max(dyp1 + dypFract, pfma1->dypAscent);
		pfmal->dypDescent = max(dyp2 - dypFract + 
				WinMac(vfli.fPrint ? dyuWinHack : 1, 1), 
				pfma1->dypDescent);
		pfmal->dytAscent = max(dyt1 + dytFract, pfma1->dytAscent);
		pfmal->dytDescent = max(dyt2 - dytFract + 
				WinMac( dyuWinHack, 1 ), pfma1->dytDescent);

		pfmal->xt = pfma1->xt + max(pfma2->xt - pfma1->xt,
				pfmaEnd->xt - pfma1->xt);
		pfmal->xp = pfma1->xp + dxpFract;
		break; /* case fmtFract */

	case fmtRoot:
/*
    dxp1 = width of N for Nth root
    dxp2 = width of argument
    dxp3 = max( width of N , width of symbol )

    dyp2 = amount root symbol is dipped below baseline of argument
    dypAscent, Descent start out to describe argument and
	are updated to inlcude effect of root.
*/

		if (cfma < 1 || cfma > 2)
			{
#ifdef DFORMULA
			CommSzNum(SzShared("*** Formula error: wrong number of args: "), cfma);
#endif
			goto LError;
			}

		dypDescent = pfmaEnd->dypDescent;
		dytDescent = pfmaEnd->dytDescent;

		if (cfma == 2)
			{
			/* this is an Nth root */
			dxp1 = pfma2->xp - pfma1->xp;
			dxt1 = pfma2->xt - pfma1->xt;
			dxp2 = pfmaEnd->xp - pfma2->xp;
			dxt2 = pfmaEnd->xt - pfma2->xt;
			}
		else
			{
			/* this is a square root */
			dxp1 = dxt1 = 0;
			dxp2 = pfmaEnd->xp - pfma1->xp;
			dxt2 = pfmaEnd->xt - pfma1->xt;
			}

/* update CHR for root symbol. (two ch's) */
		dypChSym = (pfmaEnd->dypAscent + dypDescent - 1);
		dytChSym = (pfmaEnd->dytAscent + dytDescent - 1);
#ifdef MAC
		if (vprsu.prid == pridLaser && vfli.fPrint)
			dypChSym = dypChSym * 9 / 8;
#endif /* MAC */
		PchpSetChrDyp(pfma1->bchr - chrmChp,
				dypChSym,
				chRootExt, fTrue)->fSpec = fTrue;

		FFormatChSpecSymbol(
				PchpSetChrDyp((bchr = pfma1->bchr - chrmChp - chrmFormula - chrmChp),
				dypChSym,
				chRoot, fTrue),
				chRoot, &dxpChSym, &dxtChSym,
				&dypAscentCh1, &dypDescentCh1, &dytAscentCh1, &dytDescentCh1);

		vfli.rgdxp[pfma1->ich - 2] = dxpChSym;
		vfli.rgdxp[pfma1->ich - 1] = dxp2;
		vfli.fGraphics = fTrue;

		dxpAdj = dxpChSym / 2;
		dxtAdj = dxtChSym / 2;
		dypAdj = dypAscentCh1 / 4;
		dytAdj = dytAscentCh1 / 4;

		dxp3 = max(dxp1 + dxpAdj, dxpChSym);
		dxt3 = max(dxt1 + dxtAdj, dxtChSym);
		dyp2 = dypDescent - 2;
		dyt2 = dytDescent - 2;
		dypAscent = dypAscentCh1 - dyp2;
		dytAscent = dytAscentCh1 - dyt2;

		/* update CHRF to goto start of root symbol */
		SetChrf(bchr - chrmFormula, dxp3 - dxpChSym,
				dyp2,
				fFalse);
#ifdef MAC  /* win does not have prid */
/* on laser printer, the root extension is assumed to start at the
same place as the root symbol and also ends up there.
On other printers, extension is drawn with width dxp2 after the root symbol */
		if (vprsu.prid == pridLaser && vfli.fPrint)
			{
			SetChrf(bchr + chrmChp, - dxpChSym,
					0, fFalse);
			dxpLarger = dxpChSym; /* move right from ext to operand */
			}
		else
#endif /* MAC */
			dxpLarger = -dxp2;
		dxpLarger += XpFromXs(1);
		if (cfma == 2)
			{
			/* goto start of N for Nth root */
			SetChrf(pfma1->bchr, -dxp1 - dxpAdj + dxpLarger,
					(dyp3 = -dypAscent + dypAdj) - dyp2, fFalse);
			dyt3 = -dytAscent + dytAdj;
			SetChrf(pfma2->bchr, dxpAdj, -dyp3, fFalse);

			/* set dypAscent to total ascent */
			dypAscent += max(0, pfma2->dypDescent + pfma2->dypAscent
					- dypAdj);
			dytAscent += max(0, pfma2->dytDescent + pfma2->dytAscent
					- dytAdj);
			}
		else
			{
			/* compensate for root symbol ascent difference */
			SetChrf(pfma1->bchr, dxpLarger, -dyp2, fFalse);
			}

		/* modification  factor for Win, to prevent clipping while printing
			and displaying in Page View */
		dyuWinHack = UMultDiv(2 * 20, vfli.dyuInch, czaInch); 

		pfmal->dypAscent = max(pfma1->dypAscent, 
				dypAscent + WinMac(vfli.fPrint ? dyuWinHack : 2, 2));
		pfmal->dytAscent = max(pfma1->dytAscent, dytAscent + 
					WinMac(dyuWinHack, 2));
		pfmal->dypDescent = max(pfma1->dypDescent,
				dypDescent);
		pfmal->dytDescent = max(pfma1->dytDescent,
				dytDescent);

		pfmal->xp = pfma1->xp + dxp2 + dxp3 + XpFromXs(1);
		pfmal->xt = pfma1->xt + dxt2 + dxt3 + XtFromXs(1);
		break; /* case fmtRoot */

	case fmtSubSuper:
/*
    dyp1 = amount of super- (+) or sub- (-) scripting
*/
		if (cfma == 1)
			{
			dyp1 = DypFromHps(pfma1->hpsVMove);
			dyt1 = DytFromHps(pfma1->hpsVMove);
			SetChrf(pfma1->bchr, 0, dyp1, fFalse);
			SetChrf(pfmaEnd->bchr, 0, -dyp1, fFalse);
			pfmal->dypAscent = max(pfma1->dypAscent,
					pfmaEnd->dypAscent - dyp1 +
					DypFromHpsCh(pfma1->dhpsAscent));
			pfmal->dytAscent = max(pfma1->dytAscent,
					pfmaEnd->dytAscent - dyt1 +
					DytFromHpsCh(pfma1->dhpsAscent));
			pfmal->dypDescent = max(pfma1->dypDescent,
					pfmaEnd->dypDescent + dyp1 +
					DypFromHpsCh(pfma1->dhpsDescent));
			pfmal->dytDescent = max(pfma1->dytDescent,
					pfmaEnd->dytDescent + dyt1 +
					DytFromHpsCh(pfma1->dhpsDescent));
			break; /* case fmtSubSuper */
			}

		if (cfma < 1)
			{
#ifdef DFORMULA
			CommSzNum(SzShared("*** Formula error: wrong number of args: "), cfma);
#endif
			goto LError;
			}
/* with more than one argument, acts just like ^A() */
		fapb.ccol = 1;
		fapb.hpsHSpace = 0;
		fapb.hpsVSpace = 0;
		goto LFmtArray;

	case fmtBracket:
/*
   dxp1, dxt1 = width of stack
   dxp2, dxt2 = width of left symbol
   dxp3, dxt3 = width of right symbol
   dyp1 = height of text
   dyp2 = distance from base line to bottom of stack
*/
		if (cfma != 1)
			{
#ifdef DFORMULA
			CommSzNum(SzShared("*** Formula error: wrong number of args: "), cfma);
#endif
			goto LError;
			}

		dxp1 = pfma2->xp - pfma1->xp;
		dxt1 = pfma2->xt - pfma1->xt;
		dyp1 = pfma2->dypAscent + pfma2->dypDescent;
		dyt1 = pfma2->dytAscent + pfma2->dytDescent;

		/* for fixing up argument position */
		pchrf = &((**vhgrpchr)[bchr = pfma1->bchr]);
		Assert(pchrf->chrm == chrmFormula);

		/* for fixing up bounding symbols */
		bchr -= chrmChp + chrmFormula;
		ich = pfma1->ich - 1;

		ch1 = pfma1->chLeft;
		ch2 = pfma1->chRight;

		/* left symbol */
		if (ch1 == 0)
			{
			dyp2 = dyt2 = dytAscentCh1 = dypAscentCh1 = 
					dytDescentCh1 = dypDescentCh1 = dxp2 = dxt2 = 0;
			}
		else
			{
			if (ch2 != 0)
				{
				bchr -= chrmChp + chrmFormula;
				ich--;
				}
			FFormatChSpecSymbol(
					PchpSetChrDyp(bchr + chrmFormula, dyp1, ch1, fTrue),
					ch1, &dxp2, &dxt2,
					&dypAscentCh1, &dypDescentCh1, &dytAscentCh1, &dytDescentCh1);
			SetChrf(bchr, 0, dyp2 = 
					dypAscentCh1 - pfma2->dypAscent -
					(dypDescentCh1 + dypAscentCh1 - dyp1) / 2,
					fFalse);

			dyt2 = dytAscentCh1 - pfma2->dytAscent -
					(dytDescentCh1 + dytAscentCh1 - dyt1) / 2;

			vfli.rgdxp[ich++] = dxp2;
			bchr += chrmChp + chrmFormula;
			}

		/* right symbol */
		if (ch2 == 0)
			{
			dyp3 = dyt3 = dypAscentCh2 = dytAscentCh2 =
					dytDescentCh2 = dypDescentCh2 = dxp3 = dxt3 = 0;
			pchrf->dyp -= dyp2; /* adjust text */
			}
		else
			{
			FFormatChSpecSymbol(
					PchpSetChrDyp(bchr + chrmFormula, dyp1, ch2, fTrue),
					ch2, &dxp3, &dxt3,
					&dypAscentCh2, &dypDescentCh2, &dytAscentCh2, &dytDescentCh2);
			SetChrf(bchr, dxp1, -dyp2 + (dyp3 =
					dypAscentCh2 - pfma2->dypAscent -
					(dypAscentCh2 + dypDescentCh2 - dyp1) / 2),
					fFalse);

			dyt3 = dytAscentCh2 - pfma2->dytAscent -
					(dytAscentCh2 + dytDescentCh2 - dyt1) / 2;

			vfli.rgdxp[ich] = dxp3;
			AdjustChrf(pfma1->bchr, -(dxp1 + dxp3), -dyp3);
			}

		/* reposition end */
		AdjustChrf(pfmaEnd->bchr, dxp3 == 0 ? dxp2 : dxp3, 0);

		pfmal->dypAscent = max(
				max(pfma1->dypAscent, pfma2->dypAscent),
				max(dypAscentCh1 - dyp2, dypAscentCh2 - dyp3));
		pfmal->dytAscent = max(
				max(pfma1->dytAscent, pfma2->dytAscent),
				max(dytAscentCh1 - dyt2, dytAscentCh2 - dyt3));
		pfmal->dypDescent = max(
				max(pfma1->dypDescent, pfma2->dypDescent), 
				max(dypDescentCh1 + dyp2, dypDescentCh2 + dyp3));
		pfmal->dytDescent = max(
				max(pfma1->dytDescent, pfma2->dytDescent), 
				max(dytDescentCh1 + dyt2, dytDescentCh2 + dyt3));

		pfmal->xp = pfma1->xp + dxp1 + dxp2 + dxp3;
		pfmal->xt = pfma1->xt + dxt1 + dxt2 + dxt3;
		break; /* case fmtBracket */

	case fmtIntegral:
/*
    dxp1 = width of lower limit
    dxp2 = width of upper limit
    dxp3 = width of integrand
*/
		if (cfma != 3)
			{
#ifdef DFORMULA
			CommSzNum(SzShared("*** Formula error: wrong number of args: "), cfma);
#endif
			goto LError;
			}

		dxp1 = pfma2->xp - pfma1->xp;
		dxt1 = pfma2->xt - pfma1->xt;
		dxp2 = pfma3->xp - pfma1->xp;
		dxt2 = pfma3->xt - pfma1->xt;
		dxp3 = pfmaEnd->xp - pfma3->xp;
		dxt3 = pfmaEnd->xt - pfma3->xt;

		dyp1 = pfma2->dypAscent + pfma2->dypDescent;
		dyt1 = pfma2->dytAscent + pfma2->dytDescent;
		dyp2 = pfma3->dypAscent + pfma3->dypDescent;
		dyt2 = pfma3->dytAscent + pfma3->dytDescent;
		dyp3 = pfmaEnd->dypAscent + pfmaEnd->dypDescent;
		dyt3 = pfmaEnd->dytAscent + pfmaEnd->dytDescent;

		/* calculate size of symbol */
		bchr = pfma1->bchr - chrmChp;
		ch1 = pfma1->chLeft;

		FFormatChSpecSymbol(
				PchpSetChrDyp(bchr, pfma1->fInline ?
				max(((dyp1 + dyp2) * 8 + 5) / 10/*80%*/, dyp3) : dyp3,
				ch1, pfma1->fLineOrVar),
				ch1, &dxpChSym, &dxtChSym,
				&dypAscentCh1, &dypDescentCh1, &dytAscentCh1, &dytDescentCh1);

/* effective size of integral symbol */
		dypChSym = dypAscentCh1;
		dytChSym = dytAscentCh1;
		vfli.rgdxp[pfma1->ich - 1] = dxpChSym;

/* dyp3 = distance from real base line to base line of symbol */
		dyp3 = pfmaEnd->dypDescent - dyp3 / 2 +
				(dypChSym + 1) / 2;
		dyt3 = pfmaEnd->dytDescent - dyt3 / 2 +
				(dytChSym + 1) / 2;
/* rules are different for non-integral symbols */
		switch (ch1)
			{
		case chIntegral:
			dypDescentCh1 = max(dypDescentCh1, 0);
			dytDescentCh1 = max(dytDescentCh1, 0);
			
			dyp3 -= (dypChSym <= 15 ? 2 : 4);
			dyt3 -= (dytChSym <= 15 ? 2 : 4);

			if (dyp1 == 0 && dypChSym <= DypFromHps(hpsSymbolLast))
				dyp1 = dypChSym / 4;
			if (dyt1 == 0 && dytChSym <= DytFromHps(hpsSymbolLast))
				dyt1 = dytChSym / 4;
			break;
		case chSumma:
		case chProduct:
			dypChSym = ((dypChSym * 8) + 5) / 10;
			dytChSym = ((dytChSym * 8) + 5) / 10;
			dypDescentCh1 = max(dypDescentCh1, (dypChSym + 3)/ 7);
			dytDescentCh1 = max(dytDescentCh1, (dytChSym + 3)/ 7);
			dyp3 -= (dypChSym + 3) / 7;
			dyt3 -= (dytChSym + 3) / 7;
			
			if (dypAscentCh1 < 18)
				dyp3 = 1;
			else  if (dypAscentCh1 == 18)
				dyp3--;
			else  
				dyp3 -= 3;
			if (dytAscentCh1 < 18)
				dyt3 = 1;
			else  if (dytAscentCh1 == 18)
				dyt3--;
			else  
				dyt3 -= 3;
			if (pfma1->fInline)
				{
				dypChSym += dypAscentCh1 >= 18 ? 2 : 1;
				dytChSym += dytAscentCh1 >= 18 ? 2 : 1;
				}
			}

		if (pfma1->fInline)
			{
			dxpLarger = max(dxp1, dxp2);
			dxtLarger = max(dxt1, dxt2);

			/* goto start of symbol */
			SetChrf(bchr - chrmFormula, 0,
					dyp3,
					fFalse);

			/* goto start of lower limit */
			SetChrf(pfma1->bchr,
					 0,
					dypDescentCh1 - 1,
					fFalse);

			/* goto start of upper limit */
			SetChrf(pfma2->bchr, -dxp1,
					-dypDescentCh1  + 1 - dypChSym + pfma3->dypAscent,
					fFalse);

			/* goto start of integrand */
			SetChrf(pfma3->bchr, dxpLarger - dxp2,
					-pfma3->dypAscent + dypChSym - dyp3,
					fFalse);

			/* clean up */
			pfmal->dypAscent = max(pfma1->dypAscent,
					max(pfmaEnd->dypAscent, dypChSym - dyp3));
			pfmal->dytAscent = max(pfma1->dytAscent,
					max(pfmaEnd->dytAscent, dytChSym - dyt3));
			pfmal->dypDescent = max(pfma1->dypDescent,
					max(pfmaEnd->dypDescent,
					dypDescentCh1 + dyp3 + pfma2->dypDescent));
			pfmal->dytDescent = max(pfma1->dytDescent,
					max(pfmaEnd->dytDescent,
					dytDescentCh1 + dyt3 + pfma2->dytDescent));
			}
		else
			{
			int dxpT1, dxpT2;
/* distance from start to midpoint of integral - half of integral */
			dxpLarger = max(dxpChSym / 2,
					max(dxp2 / 2,
					dxp1 / 2 )) - dxpChSym / 2;
			dxtLarger = max(dxtChSym / 2,
					max(dxt2 / 2 ,
					dxt1 / 2 )) - dxtChSym / 2;

			/* goto start of symbol */
			SetChrf(bchr - chrmFormula,
					dxpLarger,
					dyp3,
					fFalse);

			/* goto start of lower limit */
			SetChrf(pfma1->bchr,
					dxpT1 = -dxpChSym + (dxpChSym - dxp1) / 2 ,
					pfma2->dypAscent + dypDescentCh1,
					fFalse);

			/* goto start of upper limit */

			SetChrf(pfma2->bchr, dxpT2 = -(dxp1 + (dxp2 - dxp1) / 2),
					-pfma2->dypAscent - dypDescentCh1 - pfma3->dypDescent - dypChSym, 
					fFalse);

			/* goto start of integrand */
			SetChrf(pfma3->bchr, -dxpT1 - dxpT2 - dxp1 - dxp2,
					pfma3->dypDescent + dypChSym - dyp3,
					fFalse);

			/* clean up */
			/* modification factor for Win to prevent clipping while printing,
			  and displaying in Page View */
			dyuWinHack = UMultDiv(3 * 20, vfli.dyuInch, czaInch);

			pfmal->dypAscent = max(pfma1->dypAscent,
					max(dypChSym - dyp3, pfmaEnd->dypAscent) + 
					max(WinMac(vfli.fPrint ? dyuWinHack : 3, 3), dyp2));
			pfmal->dytAscent = max(pfma1->dytAscent,
					max(dytChSym - dyt3, pfmaEnd->dytAscent) + max(WinMac(dyuWinHack, 3), dyt2));
			pfmal->dypDescent = max(pfma1->dypDescent,
					max(dyp3 + dypDescentCh1, pfmaEnd->dypDescent) + dyp1);
			pfmal->dytDescent = max(pfma1->dytDescent,
					max(dyt3 + dytDescentCh1, pfmaEnd->dytDescent) + dyt1);
			}

		pfmal->xp = pfma1->xp + dxpChSym + dxpLarger + dxp3 ;
		pfmal->xt = pfma1->xt + dxtChSym + dxtLarger + dxt3 ;
		break; /* case fmtIntegral */

	case fmtOver:
		FmtOverstrike(pfma1, cfma, &dxp1, &dxt1);
		pfmal->xp = pfma1->xp + dxp1;
		pfmal->xt = pfma1->xt + dxt1;
		pfmal->dypAscent = pfma1->dypAscent;
		pfmal->dytAscent = pfma1->dytAscent;
		pfmal->dypDescent = pfma1->dypDescent;
		pfmal->dytDescent = pfma1->dytDescent;
		break; /* case fmtOver */

	case fmtArray:
		fapb.ccol = pfma1->cCol;
		fapb.hpsHSpace = pfma1->hpsHSpace;
		fapb.hpsVSpace = pfma1->hpsVSpace;
LFmtArray:
		fapb.pfma = pfma1;
		fapb.cfma = cfma;
		fapb.dypFract = pfma1->dypFract;
		fapb.dytFract = pfma1->dytFract;
		FmtArray(&fapb);
		pfmal->xp = pfma1->xp + fapb.dxp;
		pfmal->xt = pfma1->xt + fapb.dxt;
		pfmal->dypAscent = max(pfma1->dypAscent, fapb.dypAscent);
		pfmal->dytAscent = max(pfma1->dytAscent, fapb.dytAscent);
		pfmal->dypDescent = max(pfma1->dypAscent, fapb.dypDescent);
		pfmal->dytDescent = max(pfma1->dytAscent, fapb.dytDescent);
		break; /* case fmtArray */

	case fmtBox:
		if (cfma != 1)
			{
#ifdef DFORMULA
			CommSzNum(SzShared("*** Formula error: wrong number of args: "), cfma);
#endif
			goto LError;
			}

		if (pfma1->grpfSides == 0)
			pfma1->grpfSides = 0xf;

		dxp1 = pfmaEnd->xp - pfma1->xp;

		dxpAdj = XpFromXs(2);
		SetChrf(pfma1->bchr, dxpAdj*2, 0, fFalse);
		SetChrf(pfmaEnd->bchr, dxpAdj, 0, fFalse);

		/* go up from base line on right */
		PchrNew(chrmFormula, pfmal->ich,&bchrT);
		SetChrf(bchrT, 0, -(pfmaEnd->dypAscent + 2), pfma1->fRight);

		/* go left accross top */
		PchrNew(chrmFormula, pfmal->ich, &bchrT);
		SetChrf(bchrT, -(dxp1 + XpFromXs(5)), 0, pfma1->fTop);

		/* go down on left */
		PchrNew(chrmFormula, pfmal->ich, &bchrT);
		SetChrf(bchrT, 0, pfmaEnd->dypAscent + pfmaEnd->dypDescent + 3,
				pfma1->fLeft);

		/* go right accross on bottom */
		PchrNew(chrmFormula, pfmal->ich, &bchrT);
		SetChrf(bchrT, dxp1 + XpFromXs(5), 0, pfma1->fBottom);

		/* go up to base line on right */
		PchrNew(chrmFormula, pfmal->ich,&bchrT);
		SetChrf(bchrT, 0, -(pfmaEnd->dypDescent + 1), pfma1->fRight);

		/* go to end */
		PchrNew(chrmFormula, pfmal->ich,&bchrT);
		SetChrf(bchrT, dxpAdj, 0, fFalse);

		/* modification factor for Win to prevent clipping while printing
			or displaying in Page View. */  
		dyuWinHack = UMultDiv( 4 * 20, vfli.dyuInch, czaInch );

		pfmal->dypAscent = max(pfma1->dypAscent, 
				pfmaEnd->dypAscent + WinMac( 
				vfli.fPrint ? dyuWinHack : 4, 4));
		pfmal->dytAscent = max(pfma1->dytAscent, 
				pfmaEnd->dytAscent + WinMac( dyuWinHack, 4));
		pfmal->dypDescent = max(pfma1->dypDescent, 
				pfmaEnd->dypDescent + WinMac(
				vfli.fPrint ? dyuWinHack : 4, 4));
		pfmal->dytDescent = max(pfma1->dytDescent, 
				pfmaEnd->dytDescent + WinMac( dyuWinHack, 4));
		pfmal->xp = pfmaEnd->xp + dxpAdj*4;
		pfmal->xt = pfmaEnd->xt + XtFromXs(8);
		break; /* case fmtBox */

	case fmtDisplace:
		if (pfma1->ich != pfmaEnd->ich)
			{
#ifdef DFORMULA
			CommSz(SzShared("*** Formula error: ich mismatch"));
#endif
			goto LError;
			}

		dxp1 = DxpFromHps(pfma1->hpsHMove);
		dxt1 = DxtFromHps(pfma1->hpsHMove);
		SetChrf(pfma1->bchr, dxp1, 0, pfma1->fLineOrVar );

		pfmal->dypAscent = pfma1->dypAscent;
		pfmal->dytAscent = pfma1->dytAscent;
		pfmal->dypDescent = pfma1->dypDescent;
		pfmal->dytDescent = pfma1->dytDescent;

		pfmal->xp = pfma1->xp + dxp1;
		pfmal->xt = pfma1->xt + dxt1;
		break; /* case fmtDisplace */
		} /* switch (fmt) */


/* restore font to what FormatLine() thinks it is */
LRestoreFont:
	if (pfmal->bchrChp != -1)
		{
		pchr = &((**vhgrpchr)[pfmal->bchrChp]);
		Assert(pchr->chrm == chrmChp);
		LoadFont(&pchr->chp, fTrue);
		}

	return; /* normal return */

/* Error condition return:
        1. Set error flag
        2. Clear stack
*/

LError:
	pfmal->fError = fTrue;
	vifmaMac = 0;
	return;
}



/* %%Function:FmtOverstrike %%Owner:krishnam */
FmtOverstrike(pfmaFirst, cfma, pdxpMax, pdxtMax)
struct FMA * pfmaFirst;
int cfma, * pdxpMax, * pdxtMax;
{
	struct FMA * pfma;
	int ifma, jc;
	int dxp, dxpLeft, dxpMax, dxtMax;
	int rgdxp[ifmaMax], xp, xt;
	int dypAscentMax, dypDescentMax;
	int dytAscentMax, dytDescentMax;

	Assert(cfma < ifmaMax - 1);

	jc = pfmaFirst->jc;
	xp = pfmaFirst->xp;
	xt = pfmaFirst->xt;
	dxpMax = dxtMax = 0;

	dypAscentMax = pfmaFirst->dypAscent;
	dytAscentMax = pfmaFirst->dytAscent;
	dypDescentMax = pfmaFirst->dypDescent;
	dytDescentMax = pfmaFirst->dytDescent;

	/* calculate element widths and maximum width and maximum heights */
	for (ifma = 0, pfma = pfmaFirst + 1; ifma < cfma; ++ifma, ++pfma)
		{
		if ((dxp = pfma->xp - xp) > dxpMax)
			{
			dxpMax = dxp;
			dxtMax = pfma->xt - xt;
			}
		rgdxp[ifma] = dxp;

		dypAscentMax = max(dypAscentMax, pfma->dypAscent);
		dytAscentMax = max(dytAscentMax, pfma->dytAscent);
		dypDescentMax = max(dypDescentMax, pfma->dypDescent);
		dytDescentMax = max(dytDescentMax, pfma->dytDescent);
		}

	/* Set up chrf's */
	dxp = 0;
	for (ifma = 0, pfma = pfmaFirst; ifma < cfma; ++ifma, ++pfma)
		{
		if (jc == jcLeft)
			dxpLeft = 0;
		else
			{
			dxpLeft = dxpMax - rgdxp[ifma];
			if (jc == jcCenter)
				dxpLeft >>= 1;
			}

		SetChrf(pfma->bchr, dxp + dxpLeft, 0, fFalse);

		dxp = -rgdxp[ifma] - dxpLeft;
		}

	SetChrf(pfma->bchr, dxp + dxpMax, 0, fFalse);

	*pdxpMax = dxpMax;
	*pdxtMax = dxtMax;

	pfmaFirst->dypAscent = dypAscentMax;
	pfmaFirst->dytAscent = dytAscentMax;
	pfmaFirst->dypDescent = dypDescentMax;
	pfmaFirst->dytDescent = dytDescentMax;
}



/* %%Function:FmtArray %%Owner:krishnam */
FmtArray(pfapb)
struct FAPB * pfapb;
{
	struct FMA * pfma, * pfmaCol;
	int jc, * pdyp, * pdyt, mprowdyp[ifmaMax], mprowdyt[ifmaMax];
	int ifma, ifmaCol;
	int xp, xt, dxp, dyp, dyt, dxpLeft, dxpCol, dxtCol, dxpTotal, dxtTotal;
	int dxpLastRow, dxpSpace, dxtSpace;
	int dypAscentFirstRow, dypCenter, dypSpace;
	int dytAscentFirstRow, dytCenter, dytSpace;
	int cfma, ccol, crow, row;

	cfma = pfapb->cfma;
	ccol = pfapb->ccol;
	crow = (cfma + ccol - 1) / ccol;

	dxpSpace = DxpFromHps(pfapb->hpsHSpace);
	dxtSpace = DxtFromHps(pfapb->hpsHSpace);
	dypSpace = DypFromHps(pfapb->hpsVSpace);
	dytSpace = DytFromHps(pfapb->hpsVSpace);

	Assert(cfma < ifmaMax && ccol < ifmaMax);

	dxpTotal = 0;
	dxtTotal = 0;
	dypAscentFirstRow = 0;
	dytAscentFirstRow = 0;
	dxpLastRow = 0;
	SetBytes(&mprowdyp[0], 0, sizeof(mprowdyp));
	SetBytes(&mprowdyt[0], 0, sizeof(mprowdyt));

	pfmaCol = pfapb->pfma;
	xp = pfmaCol->xp;
	xt = pfmaCol->xt;
	jc = pfmaCol->jc;

	for (ifmaCol = 0; ifmaCol < ccol; ++pfmaCol, ++ifmaCol)
		{
		/* calculate the width of this column */
		dxtCol = dxpCol = 0;
		pfma = pfmaCol + 1;
		pdyp = &mprowdyp[0];
		pdyt = &mprowdyt[0];
		dypAscentFirstRow = max(pfma->dypAscent, dypAscentFirstRow);
		dytAscentFirstRow = max(pfma->dytAscent, dytAscentFirstRow);
		row = 0;
		for (ifma = ifmaCol; ifma < cfma; ifma += ccol, pfma += ccol)
			{
			dyp = pfma->dypDescent;
			dyt = pfma->dytDescent;
			if (ifma + ccol < cfma)
				{
				dyp += (pfma + ccol)->dypAscent + dypSpace;
				dyt += (pfma + ccol)->dytAscent + dytSpace;
				}
			*pdyp++ = max(*pdyp, dyp);
			*pdyt++ = max(*pdyt, dyt);

			if ((dxp = pfma->xp - xp) > dxpCol)
				{
				dxpCol = dxp;
				dxtCol = pfma->xt - xt;
				}
			++row;
			}
/* NOTE: adjusting dxpCol below causes the column width to be different,
rather than putting space between columns. */
		dxpCol += dxpSpace;
		dxtCol += dxtSpace;

		if (row == crow)
			dxpLastRow += dxpCol;
		dxpTotal += dxpCol;
		dxtTotal += dxtCol;

		/* adjust dxp's of CHRFs on this column and the next */
		pfma = pfmaCol;
		for (ifma = ifmaCol; ifma < cfma; pfma += ccol, ifma += ccol)
			{
			dxp = (pfma + 1)->xp - xp;
			if (jc == jcLeft)
				dxpLeft = 0;
			else
				{
				dxpLeft = dxpCol - dxp;
				if (jc == jcCenter)
					dxpLeft >>= 1;
				}
			AdjustChrf(pfma->bchr, dxpLeft, 0);
			AdjustChrf((pfma + 1)->bchr, 
					dxpCol - (dxp + dxpLeft), 0);
			}
		}

	/* fixup the CHRFs in the last column and calculate total height */
	pfma = pfapb->pfma + ccol;
	ifma = ccol;
	pdyp = &mprowdyp[0];
	pdyt = &mprowdyt[0];
	dyp = dypAscentFirstRow;
	dyt = dytAscentFirstRow;
	while (dyp += *pdyp, dyt += *pdyt, ifma < cfma)
		{
		AdjustChrf(pfma->bchr, -dxpTotal, *pdyp++);
		pdyt++;
		pfma += ccol;
		ifma += ccol;
		}

	dypCenter = pfapb->dypFract;
	dytCenter = pfapb->dytFract;

	/* fixup first CHRF */
	AdjustChrf(pfapb->pfma->bchr, 0,
			-dypCenter - dyp / 2 + dypAscentFirstRow);

	/* fixup last CHRF */
	AdjustChrf((pfapb->pfma + cfma)->bchr, dxpTotal - dxpLastRow,
			dypCenter - (dyp + 1) / 2 + *pdyp);

	pfapb->dxp = dxpTotal;
	pfapb->dxt = dxtTotal;
	pfapb->dypAscent = dypCenter + dyp / 2;
	pfapb->dytAscent = dytCenter + dyt / 2;
	pfapb->dypDescent = max(0, dyp - pfapb->dypAscent);
	pfapb->dytDescent = max(0, dyt - pfapb->dytAscent);
}


/* X P  F R O M  X S */
/* due to lack of good Hungarian name, this xp is to mean
the quantity that is in xs or xu units depending on fPrint */
/* %%Function:XpFromXs %%Owner:krishnam */
XpFromXs(xs)
{
	if (vfli.fPrint) xs = NMultDiv(xs, vfli.dxuInch, vfli.dxsInch);
	return xs;
}


/* %%Function:XtFromXs %%Owner:krishnam */
XtFromXs(xs)
{
	if (vfli.fPrint || vfli.fFormatAsPrint)
		xs = NMultDiv(xs, vfli.dxuInch, vfli.dxsInch);
	return xs;
}


#ifdef WIN
/* D Y P  F R O M  H P S  C H  */
/* %%Function:DypFromHps %%Owner:krishnam */
DypFromHpsCh(hps)
{
	return UMultDiv( hps, vfti.dypInch, 2 * 72 );
}


/* D Y T  F R O M  H P S  C H  */
/* %%Function:DytFromHpsCh %%Owner:krishnam */
DytFromHpsCh(hps)
{
	return UMultDiv( hps, vftiDxt.dypInch, 2 * 72 );
}


#endif /* WIN */

/* D Y P  F R O M  H P S  */
/* %%Function:DypFromHps %%Owner:krishnam */
DypFromHps(hps)
{
	return WinMac(NMultDiv(hps, vfti.dypInch, 2 * 72),
			hps/2);
}


/* D Y T  F R O M  H P S */
/* %%Function:DytFromHps %%Owner:krishnam */
DytFromHps(hps)
{
	return WinMac(NMultDiv(hps, vftiDxt.dypInch, 2 * 72),
			hps/2);
}


/* H P S  F R O M  D Y P */
/* %%Function:HpsFromDyp %%Owner:krishnam */
HpsFromDyp(dyp)
{
	return WinMac(NMultDiv(dyp, 2 * 72, vfti.dypInch), dyp * 2);
}


/* S E T  C H R F

    sets fields of CHRF at bchr
*/

/* %%Function:SetChrf %%Owner:krishnam */
SetChrf(bchr, dxp, dyp, fLine)
{
	struct CHRF *pchrf = &((**vhgrpchr)[bchr]);

	Assert(pchrf->chrm == chrmFormula);
	pchrf->dxp = dxp;
	pchrf->dyp = dyp;
	if (pchrf->fLine = fLine)
		vfli.fGraphics = fTrue;
}


/* A D J U S T  C H R F
    offsets fields of CHRF at bchr
*/

/* %%Function:AdjustChrf %%Owner:krishnam */
AdjustChrf(bchr, dxp, dyp)
{
	struct CHRF * pchrf = &((**vhgrpchr)[bchr]);

	Assert(pchrf->chrm == chrmFormula);
	pchrf->dxp += dxp;
	pchrf->dyp += dyp;
}


/* P C H P  S E T  C H R  D Y P
Assign hps to chp of ch at bchr to accomodate operand of size dyp.
Ch is given so that brackets can be changed to symbol font if > hpsSymbolLast.
*/

/* %%Function:PchpSetChrDyp %%Owner:krishnam */
struct CHP *PchpSetChrDyp(bchr, dyp, ch, fVar)
int bchr, dyp, ch, fVar;
{
	struct CHR *pchr =((struct CHR *)&((**vhgrpchr)[bchr]));
	struct CHP *pchp = &pchr->chp;
#ifdef WIN
	struct CHP chpT;
#endif /* WIN */

	int dypQ = (ch == chRoot || ch == chRootExt) && pchp->ftc == ftcSymbol ? 3 : 6;

/* map dyp operand into hps of variable size symbol */
	if (fVar)
		{
		int hps = HpsFromDyp((dyp + dypQ - 1) / dypQ * dypQ);
		if (hps > hpsSeeLarge && ch > ' ')
			{
/* for specially drawn characters, encode large hps's */
			pchp->hpsLarge = hps;
			hps = hpsSeeLarge;
			}
		else
			hps = min(254, hps);
		pchp->hps = hps;
		}
#ifdef WIN
	chpT = *pchp;
#endif /* WIN */
	switch (ch)
		{
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
	case '|':
		if (pchp->hps > hpsSymbolLast) pchp->ftc = ftcSymbol;
#ifdef WIN
		chpT.ftc = pchp->ftc;

	case chIntegral:
			{
			int hps = pchp->hps;
			if (ch == '{' || ch == '}')
				chpT.hps = hps > 72 * 2 ? 24 * 2 :
						hps > 50 * 2 ? 18 * 2 :
						hps > 32 * 2 ? 12 * 2 : 9 * 2;
			else
				chpT.hps = hps > 72 * 2 ? 24 * 2 :
						hps > 48 * 2 ? 18 * 2 : 12 * 2;

			}
#endif /* WIN */
		}

#ifdef WIN
	LoadFont(&chpT, fTrue);	/* HM (WIN) */
	pchr =((struct CHR *)&((**vhgrpchr)[bchr]));
	pchp = &pchr->chp;
	pchr->fcid = vfti.fcid;
#endif
	return pchp;
}


/* F  F O R M A T  C H  S P E C  S Y M B O L */
/* returns the width of a large symbol. When the symbol is a composite,
returns width of composite.
Returns true iff composite. */
/* %%Function:FFormatChSpecSymbol %%Owner:krishnam */
FFormatChSpecSymbol(pchp, ch, pdxp, pdxt, pdypAscent, pdypDescent, pdytAscent, pdytDescent)
struct CHP *pchp;
int ch, *pdxp, *pdxt, *pdypAscent, *pdypDescent, *pdytAscent, *pdytDescent;
{
	BOOL fReturn = fFalse;
	int chT = ch;
	int hps = pchp->hps;

	if (ch == chRoot) goto LBreak;
	if (ch == chIntegral)
				{
				chT = 243;	
				goto LBreak;
				}

	if (hps <= hpsSymbolLast) goto LDefault;

	switch (ch)
		{
	case '[':
	case ']':
		chT = 233;
		break;
	case '(':
	case ')':
	case '}':
	case '{':
		chT = 236;
		break;
	case '|':
		chT = 239;
		break;
	default:
/* normal character, establish LoadFont and return */
LDefault:
		LoadFont(pchp, fTrue);	/* HM (WIN); pchp no longer valid */
		*pdypAscent = vfti.dypAscent;
		*pdypDescent = vfti.dypDescent;
		*pdytAscent = vftiDxt.dypAscent;
		*pdytDescent = vftiDxt.dypDescent;
		goto LReturn;
		}
LBreak:
/* composite symbol. Prototypical ch has been already substituted */
	pchp->fSpec = fTrue;
	fReturn = fTrue;
/* composite character has small descenders */
	SymbolProto(ch, pchp, pdypAscent, pdypDescent, pdytAscent, pdytDescent, fTrue);
	if (hps == hpsSeeLarge) hps = pchp->hpsLarge;
	{
	int wPix = (vfli.fPrint && FEqNcSz(*vpri.hszPrDriver, SzSharedKey("PSCRIPT", pscript))) ?	5 : 3;
	int dyuWinHack = UMultDiv(wPix * 20, vfli.dyuInch, czaInch);
	*pdypAscent = (ch==chIntegral ? max(*pdypAscent+(vfli.fPrint ? dyuWinHack : wPix), DypFromHps(hps) - *pdypDescent)  
											:	DypFromHps(hps) - *pdypDescent);
	*pdytAscent = (ch==chIntegral ? max(*pdytAscent+dyuWinHack, DytFromHps(hps) - *pdytDescent)
											: DytFromHps(hps) - *pdytDescent);
	}
LReturn:
	*pdxp = DxpFromCh(chT, &vfti);
	*pdxt = (!vfli.fPrint && vfli.fFormatAsPrint)
			? DxpFromCh(chT, &vftiDxt) : *pdxp;
	return fReturn;
}


/* S H O W  S P E C  S Y M B O L */
/* show the possibly compound char ch.
Also show the root extension symbol according to the width in rgdxp */
/* NOTE: Parameters are different between Win and Mac! */
/* %%Function:ShowSpecSymbol %%Owner:krishnam */
ShowSpecSymbol(ich, pchp, puls, hdc, pptPen)
int ich;
struct CHP * pchp;
struct ULS * puls;
HDC hdc;
struct PT * pptPen;
{
#define MapWin(w) (vfli.fPrint ? UMultDiv((w) * 20, vfli.dyuInch, czaInch) : (w))

	int ch = vfli.rgch[ich];
	int chTop, chBottom, chExt, chMiddle = 0;
	int dypTotal, dypAscent = vfti.dypAscent, dypFill, dypOverlap, dypDescent;
	int dytAscent, dytDescent;
	int cchExt, cch;
	int dxp, dxpCh, c, cchLast;
	int dyp;
	int hps;
	int ypMid;
	struct PT pt;
	char rgch[32];
	
	pt = *pptPen;
	Assert(pchp->ftc == ftcSymbol);
	Assert( vfti.hfont != NULL );

	if ((hps = pchp->hps) == hpsSeeLarge) hps = pchp->hpsLarge;
	dyp = DypFromHps(hps);
	switch (ch)
		{
	case chRootExt:
/* root extension. Fill in dxp space with the characters. See DisplayFliCore
for leader dot code */
		dxp = vfli.rgdxp[ich];

/* use "DrawFormulaLine" for drawing the root extension even while printing
as we assume that printing will be done on the highest resolution for formulas.
Takes care of color mapping.
(kcm) */
		Assert(!(vfli.fPrint & 0xFE));
		DrawFormulaLine(hdc, pt.xp+dxp, pt.yp-dyp, pt.xp, pt.yp-dyp,
				vfli.fPrint /* fPrint */
				+ 8 /* fCreate */
				+ 4 /* fMove */
		+ 2 /* fDestroy */);
		return;

	case chRoot:

/* use "DrawFormulaLine" for drawing the root sign even while printing
as we assume that printing will be done on the highest resolution for formulas.
Takes care of color mapping.
Using the TextOut to write the special character - chRoot leaves gaps when
printing on the laserjet.
(kcm) */
		dxp = vfli.rgdxp[ich];
/* strokes: up 1/2 dyp
	up to 5/9 dyp, right 2/10 dxp
	down to 0 dyp, right to 7/10 dxp
	up dyp, right rest of dxp
	down dyp to baseline
*/
		Assert(!(vfli.fPrint & 0xFE));
		DrawFormulaLine(hdc, pt.xp + dxp * 2 / 10,
				pt.yp - (dyp * 10 + 9) / 18,
				pt.xp, pt.yp - (dyp /2),
				vfli.fPrint /* fPrint */
		+ 8 /* fCreate */ + 4 /* fMove */);
		DrawFormulaLine(hdc, pt.xp + dxp * 7 / 10,
				pt.yp, NULL, NULL, 0);
		DrawFormulaLine(hdc, pt.xp + dxp,
				pt.yp - dyp,
				NULL, NULL,
				2 /* fDestroy */);
		return;

	default:
		/* not a composite symbol */
		rgch[0] = ch;
		TextOut(hdc, pt.xp, pt.yp - dypAscent, rgch, 1);
		return;

	case '[':
		chTop = 233;
		chExt = 234;
		chBottom = 235;
		break;

	case ']':
		chTop = 249;
		chExt = 250;
		chBottom = 251;
		break;

	case ')':
	case '}':
		chTop = 252;
		chExt = 239;
		if (ch == '}')
			chMiddle = 253;
		chBottom = 254;
		break;

	case '(':
	case '{':
		chTop = 236;
		chExt = 239;
		if (ch == '{')
			chMiddle = 237;
		chBottom = 238;
		break;

	case chIntegral:
		chTop = 243;
		chExt = 244;
		chBottom = 245;
		break;

	case '|':
		chBottom = chTop = chExt = 239;
		break;
		}

/* composite symbol. Prototypical ch has been already substituted */
	SymbolProto(ch, pchp, &dypAscent, &dypDescent, &dytAscent, &dytDescent,fFalse);

/* Allocate space to various components */
	dypTotal = dyp - dypDescent;
/* amount to be filled in the middle with extensions and chMiddle */
	dypFill = dypTotal - dypAscent * 2;
	if (chMiddle != 0)
		{
		dypFill = (dypFill - dypAscent) / 2 + MapWin(1);
		ypMid = pt.yp - (dypTotal - dypAscent)/2 - dypAscent;
		}

	cchExt = dypFill > 0 ? (dypFill+dypAscent - MapWin(2))/(dypAscent - MapWin(1)) : 0;
		
/* amount of fill overlapping chTop and Bottom */
	dypOverlap = ((cchExt * (dypAscent - MapWin(1))) - dypFill);
	if (chMiddle)
		dypOverlap = NMultDiv(dypOverlap, 4, 5) + MapWin(3);
	else
		dypOverlap = dypOverlap / 2 + MapWin(1);
/*
Composite symbol is drawn here:
*/
	rgch[0] = chBottom;
	TextOut(hdc, pt.xp, pt.yp - dypAscent, rgch, 1);

	rgch[0] = chExt;
	dyp = pt.yp - (dypAscent * 2) + dypOverlap;
	for (cch = 0; cch < cchExt; cch++)
		{
		TextOut(hdc, pt.xp, dyp, rgch, 1);
		dyp -= dypAscent - MapWin(1);
		}

	if (chMiddle != 0)
		{
		int yp = ypMid;

		rgch[0] = chMiddle;
		TextOut(hdc, pt.xp, yp, rgch, 1);
		rgch[0] = chExt;
		dypOverlap = ((cchExt * (dypAscent - MapWin(1))) - dypFill) / 3;
		yp -= dypAscent - dypOverlap;
		for (cch = 0; cch < cchExt; cch++)
			{
			TextOut(hdc, pt.xp, yp, rgch, 1);
			yp -= dypAscent - MapWin(1);
			}
		}

	rgch[0] = chTop;
	{
	int wPix = (vfli.fPrint && FEqNcSz(*vpri.hszPrDriver, SzSharedKey("PSCRIPT", pscript))) ? 5 : 3;
	TextOut(hdc, pt.xp, (ch==chIntegral && dypTotal < dypAscent + MapWin(wPix+1)) ? 
								pt.yp - dypAscent - MapWin(wPix) :
				            pt.yp - dypTotal + MapWin(1), rgch, 1);
	}
}



/* S Y M B O L  P R O T O */
/* %%Function:SymbolProto %%Owner:krishnam */
SymbolProto(ch, pchp, pdypAscent, pdypDescent, pdytAscent, pdytDescent,fFormatting)
int ch;
struct CHP *pchp;
int *pdypAscent, *pdypDescent;
int *pdytAscent, *pdytDescent;
int fFormatting;
{
	int hps;
	struct CHP chpT;

	chpT = *pchp;
	hps = chpT.hps;
	if (ch == chRoot)
		{
		if (fFormatting)
			LoadFont(pchp, fTrue);	/* HM (WIN); pchp invalid */
		*pdypAscent = DypFromHps(hps);
		*pdypDescent = 0;
		*pdytAscent = DytFromHps(hps);
		*pdytDescent = 0;
		Mac( fWidthOK = fFalse );
		return;
		}
	if (ch == '{' || ch == '}')
		chpT.hps = hps > 72 * 2 ? 24 * 2 :
				hps > 50 * 2 ? 18 * 2 :
				hps > 32 * 2 ? 12 * 2 : 9 * 2;
	else
		chpT.hps = hps > 72 * 2 ? 24 * 2 :
				hps > 48 * 2 ? 18 * 2 : 12 * 2;
	if (fFormatting)
		LoadFont(&chpT, fTrue);
	*pdypAscent = vfti.dypAscent;
	*pdypDescent = vfti.dypDescent;
	*pdytAscent = vftiDxt.dypAscent;
	*pdytDescent = vftiDxt.dypDescent;
}



/* C H  F E T C H  N O N  V A N I S H
*  Return one character from document skipping vanished runs.
*   WARNING: NOT field sensitive.
*/
#ifdef MAC  /* native in win */
/* %%Function:ChFetchNonVanish %%Owner:NOTUSED */
NATIVE char ChFetchNonVanish(doc, pcp) /* WINIGNORE - in assembler if WIN */
int doc;
CP *pcp;
{
	CP cp;
	char ch;

	cp = *pcp;
	for (;;)
		{
		FetchCp(doc, cp, fcmChars | fcmProps);
#ifdef WIN
		if (vchpFetch.fVanish /*|| vchpFetch.fFldVanish*/)
#else
			if (vchpFetch.fVanish)
#endif
				{
				cp += vccpFetch;
				if (cp >= caPara.cpLim - ccpEop)
					{
					*pcp = caPara.cpLim - ccpEop;
					return (0); /* cause error */
					}
				}
			else
				break;
		}
	*pcp = cp;
	return (*vhpchFetch);
}


#endif /* MAC */


/* C P	F I R S T  N O N  B L A N K

    Return the cp of the first non blank (space and tab are considered blank)
    character after the given cp.
*/

#ifdef MAC
/* %%Function:CpFirstNonBlank %%Owner:krishnam */
NATIVE CP CpFirstNonBlank(doc, cp) /* WINIGNORE - in assembler if WIN */
int doc;
CP cp;
{
	int ch;

	while ((ch = ChFetchNonVanish(doc, &cp)) == ' ' || ch == chTab)
		cp++;

	return cp;
}
#endif


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Formula_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Formula_Last() */
