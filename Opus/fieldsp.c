/* F I E L D S P . C */
/* Field-specific stuff for formatsp.c */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "doc.h"
#include "format.h"
#include "ch.h"
#include "disp.h"
#include "sel.h"
#include "outline.h"
#include "screen.h"
#include "print.h"
#include "debug.h"
#include "layout.h"
#include "inter.h"
#include "strtbl.h"
#include "prm.h"
#include "error.h"
#include "idle.h"
#include "formula.h"

#define FORMATSP
#include "field.h"


#ifdef PROTOTYPE
#include "fieldsp.cpt"
#endif /* PROTOTYPE */


struct FMA      rgfma[ifmaMax];
int             vifmaMac;

int 			docSeqCache = docNil;
CP 				cpSeqCache;

int 			rgwSeqCache[10];

struct ITR      vitr;           /* international stuff */


/* E X T E R N A L S */

extern IDF          vidf;
extern struct PAP   vpapFetch;
extern int          vifldInsert;
extern struct PREF  vpref;
extern struct FLI   vfli;
extern struct FMTSS vfmtss;
extern int          vdocFetch;
extern CP           vcpFetch;
extern struct CHP   vchpFetch;
extern BOOL         vfEndFetch;
extern int          vccpFetch;
extern CHAR HUGE *  vhpchFetch;
extern struct SEL   selCur;
extern BOOL         vfInsertMode;
extern CP           cpInsert;
extern struct WWD **hwwdCur;
extern int          wwCur;
extern struct FFS  *vpffs;

extern char		(**vhgrpchr)[];
extern struct SCI	vsci;
extern struct PRI	vpri;
extern struct FTI	vfti;
extern struct FTI	vftiDxt;
extern int		vdocTemp;
extern struct CHP	vchpStc;
extern struct MERR	vmerr;
extern int              vfPrvwDisp;
extern int		vfInsertMode;


/* F I E L D  F O R M A T T I N G  F U N C T I O N S */

/* F F C  F O R M A T  D I S P L A Y */
/*  Format a display type field (fDisplay).

	To generate a CHRDF, return ffcDisplay.
	Individual display field format functions are responsible for
	setting up vfmtss.  Some standard defaults are preset. Individual
	functions MUST set vfmtss.dxp, vfmtss.dyp, vfmtss.dxt and vfmtss.dyt.

	To generate a CHRFG, return ffcBeginGroup.  To terminate a CHRFG return
	ffcEndGroup.
*/
/* %%Function:FfcFormatDisplay %%Owner:peterj */
EXPORT FfcFormatDisplay (ch, doc, pflcd, pdcp)
CHAR ch;
int doc;
struct FLCD *pflcd;
CP *pdcp;

{
	int ffcReturn = ffcSkip;
	int flt = pflcd->flt;
	int ifld = pflcd->ifldChBegin;
	int cSkipArgs;

	Scribble (ispFieldFormat2, 'D');

	vfmtss.dyyDescent.wlAll = 0;
	vfmtss.flt = flt;

	switch (flt)
		{
	case fltPrint:
		if (vfli.fPrint && ch == chFieldBegin)
			{
			vfmtss.dxp = 0;
			vfmtss.dyp = 0;
			vfmtss.dxt = 0;
			vfmtss.dyt = 0;
			ffcReturn = ffcDisplay;
			}
		break;

	case fltSeqLevOut:
	case fltSeqLevLeg:
	case fltSeqLevNum:
		if (ch == chFieldBegin)
			ffcReturn = FfcFormatSeqLev (doc, pflcd->cpFirst, flt);
		break;

	case fltImport:
		cSkipArgs = -1;
		goto LFormatGroup;

	case fltMacroText:
	case fltHyperText:
		cSkipArgs = 1;
		goto LFormatGroup;

	case fltFormula:
		cSkipArgs = 0;

LFormatGroup:
		Assert (vpffs != NULL && ifld != ifldNil);
		if (ch == chFieldBegin)
			{
			if (vpffs->ifldWrap == ifld)
				ffcReturn = ffcWrap;

			else  if (vpffs->ifldError == ifldNil || 
					ifld < vpffs->ifldError)
				{
				if (!vpffs->fFormatting)
					{
					vpffs->fFormatting = fTrue;
					vpffs->ifldFormat = ifld;
					ffcReturn = ffcBeginGroup;
					}
				if (cSkipArgs >= 0)
					*pdcp = CchSkipFieldKeywordArgs (doc, ifld, cSkipArgs);
				else
					/* skip to the result */
					*pdcp = pflcd->dcpInst;
				}
			else
				ffcReturn = FfcFormatFltMessage (istMsgError, doc,
						pflcd->cpFirst);
			}
		else  if (ch == chFieldEnd && vpffs->fFormatting &&
				vpffs->ifldFormat == ifld)
			{
			/* check for legal termination
				+ formulas: parens must be balanced
			*/
			if (!vpffs->fValidEnd)
				{
				vpffs->ifldError = ifld;
				ffcReturn = ffcRestart;
				}
			else
				{
				vpffs->fFormatting = fFalse;
				ffcReturn = ffcEndGroup;
				}
			}
		break;

#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */

		}

	Scribble (ispFieldFormat2, ' ');

	return ffcReturn;

}





/* D I S P L A Y  F I E L D */
/* Show "display type" field */

/* %%Function:DisplayField %%Owner:peterj */
EXPORT DisplayField (doc, cp, bchr, flt, hdc, pt, ypBase, ypUL, puls, prcClip)
int doc;
CP cp;                  /* CP of field start */
int bchr;               /* invoking CHRDF */
int flt;                /* invoking field type */
HDC hdc;                /* HDC on which to draw */
struct PT pt;           /* pt of upper-left of draw box */
int ypBase;             /* base line for text */
int ypUL;               /* base line for underline */
struct ULS *puls;       /* underline state */
struct RC *prcClip;     /* clip rect (in hdc coordinate system) */

{
	int ifld = IfldFromDocCp (doc, cp, fTrue);

	switch (flt)
		{
	case fltPrint:
		Assert (ifld != ifldNil);
		DisplayFltPrint(doc, cp, ifld, hdc, pt);
		break;

	case fltSeqLevOut:
	case fltSeqLevLeg:
	case fltSeqLevNum:
		Assert (ifld != ifldNil);
		DisplayFltSeqLev (doc, cp, hdc, pt, ypBase, ypUL, puls, 
				prcClip, bchr);
		break;

	case fltMessage:
		/* may not have associated ifld */
		DisplayFltMessage (hdc, pt, ypBase, ypUL, puls, prcClip, bchr);
		break;

#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif  /* DEBUG */
		}
}




CP DcpCopyArgs();

/* D I S P L A Y  F L T  P R I N T */
/* %%Function:DisplayFldPrint %%Owner:peterj */
DisplayFltPrint(doc, cpStart, ifld, hdc, pt)
int doc;
CP cpStart;
int ifld;
HDC hdc;
struct PT pt;
{
	int w, wCh, cch, fError = fFalse;
	int chSwitch;
	int idPrintPs;
	CP cp, dcp, cpFetch, cpMac;
	struct FFB ffb;
	struct CA caT1, caT2;
	char rgch[cchMaxSz];

	/* printer literals are sent only during print */
	if (!vfli.fPrint)
		return;

	if (DocCreateTemp(doc) == docNil)
		return;

	Assert (hdc == vpri.hdc);
	InitFvb(&ffb.fvb);
	SetFfbIfld(&ffb, doc, ifld);
	ExhaustFieldText(&ffb);
	if ((chSwitch = ChFetchSwitch(&ffb, fFalse /* !fSys */)) == chFldPrintPs)
		{
		InitFvbBufs (&ffb.fvb, rgch, cchMaxSz, NULL, 0);
		FFetchArgText (&ffb, fTrue /* fTrunc */);
		if ((idPrintPs = IdFromSzgSz(szgPrintPs, rgch)) != idPrintPsDict)
			{
			int doc, ww, dxa;
			CP cpMin;
			doc = vfli.doc;
			ww = vfli.ww;
			cpMin = vfli.cpMin;
			dxa = vfli.dxa;
			EmitPsSz(SzFrameKey("gsave",Gsave));
			EmitPs(idPrintPs == idNil ? idPrintPsPage : idPrintPs, cpStart);
			/*
			*  EmitPs does FormatLines in the Pic and Cell cases...need to
			*  regenerate vfli since we're in the bowels of DisplayFli!
			*/
			FormatLineDxa(ww, doc, cpMin, dxa);
			}
		}
	InitFvb(&ffb.fvb);
	SetFfbIfld(&ffb, doc, ifld);
	SkipArgument (&ffb); /* skip keyword */
	for (;;)
		{
		/* Put the characters to be passed into vdocTemp */
		if (!FDelete(PcaSetWholeDoc(&caT1, vdocTemp)))
			break;
		dcp = DcpCopyArgs(&ffb, vdocTemp, &wCh);

		/* error in field -- pass an error message through */
		if (dcp < 0)
			{
			wCh = chNil;
			FDelete(PcaSetWholeDoc(&caT1, vdocTemp));
			CchInsertFieldError(vdocTemp, cp0, (int) -dcp);
			fError = fTrue;
			}

		/* Fetch and Pass characters from vdocTemp */
		if (dcp != 0)
			{
			cp = cp0;
			cpMac = CpMacDoc(vdocTemp) - ccpEop;
			for (cpFetch = cpMac; cp < cpMac; cp += (CP)cch)
				{
				FetchRgch(&cch, &rgch[2], vdocTemp, cp, cpFetch, cchMaxSz - 2);
				blt(&cch, (int *)rgch, 1);
				Escape(vpri.hdc, PASSTHROUGH, cch, (LPSTR)rgch, (LPSTR)NULL);
				}
			}

		/* If DcpCopyArgs barfed on a single CR or LF, pass it here */
		if (wCh != chNil)
			{
			rgch[2] = wCh;
			w = 1;
			blt(&w, (int *)rgch, 1);
			Escape(vpri.hdc, PASSTHROUGH, 1, (LPSTR)rgch, (LPSTR)NULL);
			}

		else  if (dcp == cp0 || fError)
			break;
		}

	EmitPsSz(SzFrame(""));
	if (chSwitch == chFldPrintPs && idPrintPs != idPrintPsDict)
		EmitPsSz(SzFrameKey(" grestore",GreStore));
}



/* F F C  F O R M A T  R G C H */
/* %%Function:FfcFormatRgch %%Owner:peterj */
FfcFormatRgch (cch, pch, pchp)
int cch;
CHAR *pch;
struct CHP *pchp;

{
	int dxp, dxt;
	BOOL fDxt = (!vfli.fPrint && vfli.fFormatAsPrint);
	CHAR ch;
	struct FTI *pftiDxt = (fDxt ? &vftiDxt : &vfti);

	vfmtss.w2 = pchp->ico;
	LoadFont (pchp, fTrue/*fWidthsOnly*/);
	bltb (&vfti.fcid, &vfmtss.l, sizeof (union FCID));
	Assert (sizeof (union FCID) == sizeof (long));

	vfmtss.dyp = vfti.dypDescent + vfti.dypAscent;
	vfmtss.dyyDescent.dyp = vfti.dypDescent;
	vfmtss.dyyDescent.dyt = pftiDxt->dypDescent;
	vfmtss.dyt = pftiDxt->dypAscent + pftiDxt->dypDescent;

/* sum up the width of cch characters */
	dxp = dxt = 0;
	for (; cch != 0; cch--)
		{
		dxp += DxpFromCh(ch = *pch++, &vfti);
		dxt += DxpFromCh(ch, pftiDxt);
		}
	vfmtss.dxt = dxt;
	vfmtss.dxp = dxp;

	return ffcDisplay;
}


/* F F C  F O R M A T  S E Q  L E V */
/* %%Function:FfcFormatSeqLev %%Owner:davidmck */
FfcFormatSeqLev (doc, cp, flt)
int doc;
CP cp;
int flt;

{
	int cch;
	struct CHP chp;
	CHAR rgch [cchMaxSz];

	CachePara (doc,cp+1);
	FetchCp (doc, cp+1, fcmProps);
	chp = vchpFetch;

	if (doc != DocMother(doc))
		return FfcFormatFltMessage (istMsgNotMother, doc, cp);

	else  if ((cch = CchFormatLevelSeqs (doc, cp, flt, rgch, cchMaxSz)) == 0)
		return FfcFormatFltMessage (istMsgCannotFormat, doc, cp);

	else
		return FfcFormatRgch (cch, rgch, &chp);
}



/* D I S P L A Y  F L T  S E Q  L E V */
/* %%Function:DisplayFldSeqLev %%Owner:davidmck */
DisplayFltSeqLev (doc, cp, hdc, pt, ypBase, ypUL, puls, prcClip, bchr)
int doc;
CP cp;
HDC hdc;
struct PT pt;
int ypBase;
int ypUL;
struct ULS *puls;
struct RC *prcClip;
int bchr;

{
	int flt;
	int cch;
	CHAR rgch [cchMaxSz];
	struct CHRDF * pchr = (char *)*vhgrpchr + bchr;

	Assert (pchr->chrm == chrmDisplayField);
	flt = pchr->flt;
	cch = CchFormatLevelSeqs (doc, cp, flt, rgch, cchMaxSz);
	Assert (cch > 0);
	DisplayRgch(hdc, pt, ypBase, ypUL, puls, prcClip, bchr, rgch, cch);
}



#define lvlBody 9
#define lvlArabic 9

csconst struct  /* Outline Number Formatting Info */
{
	CHAR chBefore;
	CHAR nfc;
	CHAR chAfter;
}


mplvlonfi [10] =
{
		/* level 1 */   { 0,    nfcUCRoman,   '.' },
	
	/* level 2 */   { 0,    nfcUCLetter,  '.' },
	
	/* level 3 */   { 0,    nfcArabic,    '.' },
	
	/* level 4 */   { 0,    nfcLCLetter,  ')' },
	
	/* level 5 */   { '(',  nfcArabic,    ')' },
	
	/* level 6 */   { '(',  nfcLCLetter,  ')' },
	
	/* level 7 */   { '(',  nfcLCRoman,   ')' },
	
	/* level 8 */   { '(',  nfcLCLetter,  ')' },
	
	/* level 9 */   { '(',  nfcLCRoman,   ')' },
	

	/* body */      { 0,    nfcArabic,    '.' }
	};	




/* F  G E T  L E V E L  S E Q U E N C E S */
/* %%Function:FGetLevelSequences %%Owner:davidmck */
FGetLevelSequences (doc, cp, rgw)
int doc;
CP cp;
int *rgw;
{
	int ipad;
	int ifld;
	struct PLC **hplcpad;
	struct PLC **hplcfld;
	struct DOD *pdod = PdodDoc(doc);
	int iw;
	int ipadSav;
	struct PAD pad;
	int fFake = fFalse;
#ifdef DEBUG
	int fReturn;
#endif /* DEBUG */

	Assert (pdod->fMother);

	if ((pdod->hplcpad == hNil || pdod->fOutlineDirty) &&
			!FUpdateHplcpad(doc))
		return fFalse;

	if ((hplcpad = (pdod = PdodDoc(doc))->hplcpad) == hNil || 
			(hplcfld = pdod->hplcfld) == hNil || pdod->fOutlineDirty)
		return fFalse;

	if (docSeqCache == doc && !vidf.fInvalSeqLev && cp >= (cpSeqCache >> 1))
		{
LCpCacheSet:
		blt(rgwSeqCache, rgw, 10);
		if (cp == cpSeqCache)
			return fTrue;
		if (cp >= cpSeqCache)
			{
			ipad = max(IInPlcCheck(hplcpad, cpSeqCache),0)+1;
			if ((ifld = IInPlcRef(hplcfld, CpPlc(hplcpad, ipad))) == -1)
				return fFalse;
			}
		else
			{
		/* We are looking just before cpSeqCache, try recomputing
			the rgw for the part of the ipad between cp and cpSeqCache
			and then subtracting it from rgwSeqCache to get a valid
			rgw. */
			SetWords (rgw, 0, 10);
			if ((ipad = IInPlcCheck(hplcpad, cp)) < 0)
				goto LCantUseCache;
			ifld = IInPlcRef(hplcfld, CpPlc(hplcpad, ipad));
			Assert (ifld != -1);
#ifdef DEBUG
			fReturn =
#endif /* DEBUG */
					FFillRgwWithSeqLevs(doc, cpSeqCache,
					ipad, ifld, hplcpad, hplcfld, rgw);
			Assert (fReturn);
			for (iw = 0; iw < 10 && rgw[iw] == 0; iw++)
				;
			if (iw >= 10)
				{
		/* if rgw is empty then cp is in the same pad
			as cpSeqCache and we are done */
				cpSeqCache = cp;
				goto LCpCacheSet;
				}
			*blt(rgwSeqCache, rgw, iw) = rgwSeqCache[iw] - rgw[iw];
			if (iw < 9)
				SetWords (&rgw[iw + 1], 0, 9 - iw);
		/* If (*hplcpad)->rgpad[ipad].lvl == iw (the lowest level we
			encountered when computing rgw between cp and cpSeqCache)
			then rgw is valid.  Otherwise we have to find an earlier
			ipadPrev such that (*hplcpad)->rgpad[ipadPrev].lvl == iw
			and add the lvl information between ipadPrev and cp to rgw.
		*/
			for (ipadSav = ipad; ipad >= 0; ipad-- )
				{
				GetPlc(hplcpad, ipad, &pad);
				if (pad.lvl <= iw)
					break;
				}
			if (ipad < 0 || pad.lvl != iw)
		/* This document is wierd, either it starts with pad.lvl > 0
			or there is a paragraph with one level immediately
			followed by another with a level at least two higher.
			Too wierd for me, bag out.
		*/
				goto LCantUseCache;
			if (ipadSav != ipad)
		/* We had to back up to find a pad where pad.lvl == iw,
			this means pad.lvl is already registered in rgw, so
			we must increment ipad and not count it again. */
				ipad++;
			ifld = IInPlcRef(hplcfld, CpPlc(hplcpad, ipad));
			Assert (ifld != -1);
			}
		}
	else
		{
LCantUseCache:
		SetWords (rgw, 0, 10);
		ipad = 0;
		ifld = 0;
		}

#ifdef PCJ
		{
		int rgwComm[6];
		rgwComm[0] = doc;
		rgwComm[1] = (int)(cp & 0x7fffL);
		rgwComm[2] = docSeqCache;
		rgwComm[3] = (int)(cpSeqCache & 0x7fffL);
		rgwComm[4] = ipad;
		rgwComm[5] = ifld;
		CommSzRgNum(SzShared("FGetLev: (doc,cp,docSC,cpSC,ipad,ifld): "),
				rgwComm, 6);
		}
#endif /* PCJ */

	if (ipad == 0 && cp < CpPlc(hplcpad, 0))
		{
		/* Special case for rgw */
		Assert(rgw[0] == 0);
		rgw[lvlBody] = 1;
		fFake = fTrue;
		}
	else
		{
		if (!FFillRgwWithSeqLevs(doc, cp,
				ipad, ifld, hplcpad, hplcfld, rgw))
			return fFalse;
		}
	blt(rgw, rgwSeqCache, 10);
#ifdef DEBUG
	SetWords (rgw, 0, 10);
	if (ipad == 0 && cp < CpPlc(hplcpad, 0))
		{
		/* Special case for rgw */
		rgw[lvlBody] = 1;
		}
	else
		Assert (FFillRgwWithSeqLevs(doc, cp, 0, 0, hplcpad, hplcfld, rgw));
	Assert (!FNeRgw(rgw, rgwSeqCache, 10));
#endif /* DEBUG */
	if (fFake)
		/* So we don't use the cache */
		docSeqCache = docNil;
	else
		{
		cpSeqCache = cp;
		docSeqCache = doc;
		}
	return fTrue;
}




#ifdef DEBUG
/* %%Function:C_FFillRgwWithSeqLevs %%Owner:davidmck */
HANDNATIVE C_FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw)
int doc;
CP cp;
int ipad;
int ifld;
struct PLC **hplcpad;
struct PLC **hplcfld;
int *rgw;
{
	CP cpCur;
	int ipadMac = IMacPlc(hplcpad);
	int ifldMac = IMacPlc(hplcfld);
	struct PAD pad;
	struct FLD fld;

	cpCur = CpPlc(hplcpad, ipad);

	while (ipad < ipadMac && cpCur <= cp)
		{
		while (ifld < ifldMac && CpPlc(hplcfld, ifld) < cpCur)
			/* advance to this para */
			ifld++;

		cpCur = CpPlc(hplcpad, ipad+1);
		while (ifld < ifldMac && CpPlc(hplcfld, ifld) < cpCur)
			{
			GetPlc(hplcfld, ifld++, &fld);
			if (fld.ch == chFieldBegin && 
					(uns)(fld.flt-fltSeqLevOut)<=(fltSeqLevNum - fltSeqLevOut))
				/* this para does have an auto number, count it */
				{
				GetPlc (hplcpad, ipad, &pad);
				if (pad.lvl == lvlUpdate)
					{
					Assert(fFalse);
					PdodDoc(doc)->fOutlineDirty = fTrue;
					return fFalse;
					}
				if (!pad.fBody && !pad.fInTable)
					{
					Assert (pad.lvl < lvlBody && pad.lvl >= 0);
					rgw [pad.lvl]++;
					SetWords (rgw + pad.lvl + 1, 0, lvlBody - pad.lvl);
					}
				else
					rgw [lvlBody]++;
				break;
				}
			}
		ipad++;
		}

	return fTrue;
}


#endif /* DEBUG */



/* C C H  F O R M A T  W  L V L  P C H */
/* %%Function:CchFormatWLvlPch %%Owner:davidmck */
CchFormatWLvlPch (w, lvl, pch, cchMax)
int w, lvl;
CHAR *pch;
int cchMax;

{
	CHAR ch;
	int cch = 0;

	if ((ch = mplvlonfi [lvl].chBefore) != 0 && cchMax > 0)
		{
		cch++;
		*pch = ch;
		}

	cch += CchLongToRgchNfc ((long) w, pch+cch, mplvlonfi[lvl].nfc, 
			cchMax-cch);

	if (cchMax-cch > 0)
		{
		*(pch+cch) = mplvlonfi [lvl].chAfter;
		cch++;
		}

	Assert (cch <= cchMax);
	return cch;
}



/* C C H  F O R M A T  L E V E L  S E Q S */
/* %%Function:CchFormatLevelSeqs %%Owner:davidmck */
CchFormatLevelSeqs (doc, cp, flt, pch, cchMax)
int doc;
CP cp;
int flt;
CHAR *pch;
int cchMax;

{
	int rgw [10];
	int lvl, lvlLast;
	int cch, lvlT;

	if (!FGetLevelSequences (doc, cp, rgw))
		return 0;

	for (lvlLast = 8; lvlLast >= 0 && rgw [lvlLast] == 0; lvlLast--)
		;

	if (rgw[lvlBody])
		lvl = lvlBody;
	else
		lvl = lvlLast;

	Assert (lvl >= 0);

	switch (flt)
		{
	case fltSeqLevLeg:
		for (lvlT = 0, cch = 0; lvlT <= lvlLast; lvlT++)
			cch += CchFormatWLvlPch (rgw [lvlT], lvlArabic, pch+cch,
					cchMax-cch);
		if (lvl == lvlBody)
			cch += CchFormatWLvlPch (rgw [lvlBody], lvlArabic, pch+cch,
					cchMax-cch);
		return cch;

	case fltSeqLevNum:
		return CchFormatWLvlPch (rgw [lvl], lvlArabic, pch, cchMax);

	case fltSeqLevOut:
		return CchFormatWLvlPch (rgw [lvl], lvl, pch, cchMax);

#ifdef DEBUG
	default:
		Assert (fFalse);
		return 0;
#endif /* DEBUG */
		}
}


/* F F C  F O R M A T  F L T  M E S S A G E */
/* Display the string identified by istMsg.  Store in w istMsg, in l the
	fcid of the font to use.
*/

/* %%Function:FfcFormatFltMessage %%Owner:peterj */
FfcFormatFltMessage (istMsg, doc, cp)
int istMsg, doc;
CP cp;

{
	int cch;
	struct CHP chp;
	CHAR rgch [cchMaxSz];

	vfmtss.flt = fltMessage;
	vfmtss.w = istMsg;

	CachePara (doc, cp);
	chp = vchpStc;
	chp.fBold = !chp.fBold;
	chp.kul = kulNone;

	cch = *rgstFltMessage [istMsg];

	bltbx ((CHAR FAR *)rgstFltMessage [istMsg] + 1,
			(CHAR FAR *) rgch, cch);

	return FfcFormatRgch (cch, rgch, &chp);
}


/* D I S P L A Y  F L T  M E S S A G E */
/* %%Function:DisplayFltMessage %%Owner:peterj */
DisplayFltMessage (hdc, pt, ypBase, ypUL, puls, prcClip, bchr)
HDC hdc;
struct PT pt;
int ypBase;
int ypUL;
struct ULS *puls;
struct RC *prcClip;
int bchr;

{
	int ist;
	int cch;
	CHAR rgch [cchMaxSz];
	struct CHRDF * pchr = (char *)*vhgrpchr + bchr;

	Assert (pchr->chrm == chrmDisplayField);
	ist = pchr->w;
	cch = *rgstFltMessage [ist];
	bltbx ((CHAR FAR *)rgstFltMessage [ist] + 1,
			(CHAR FAR *) rgch, cch);
	DisplayRgch(hdc, pt, ypBase, ypUL, puls, prcClip, bchr, rgch, cch);
}


/* D I S P L A Y  R G C H */
/* %%Function:DisplayRgch %%Owner:peterj */
DisplayRgch (hdc, pt, ypBase, ypUL, puls, prcClip, bchr, pch, cch)
HDC hdc;
struct PT pt;
int ypBase;
int ypUL;
struct ULS *puls;
struct RC *prcClip;
int bchr;
char *pch;
int cch;
{
	int ilevel;
	int ico;
	union FCID fcid;
	struct CHRDF * pchr = (char *)*vhgrpchr + bchr;

	Assert (pchr->chrm == chrmDisplayField);

	fcid.lFcid = pchr->l;
	ico = pchr->w2;

	/*  treat underline */
	if (puls)
		{
		/* these show if any underline is set in current looks */
		if (puls->grpfUL)
			{
			puls->xwLim = pt.xp;
			EndUL(puls, prcClip);
			}
		puls->kul = fcid.kul;
		}

	LoadFcidFull(fcid);
	ForeColor(hdc, ico);

	if (puls && puls->grpfUL)
		{
		puls->pt.xp = pt.xp;
		puls->pt.yp = ypUL;
		}

	ilevel = SaveDC( hdc );
	IntersectClipRect(hdc, prcClip->xpLeft, prcClip->ypTop, prcClip->xpRight,
			prcClip->ypBottom);

	TextOut (hdc, pt.xp, ypBase-vfti.dypAscent, (LPSTR)pch, cch);

	RestoreDC( hdc, ilevel );
}




/* D E A D  F I E L D  C O D E */



/* U N V A N I S H  C P  C P L I M */
/*  Unvanish a range of text.  Handles properly the possibility of
	nested dead fields.
*/

/* %%Function:UnvanishCpCpLim %%Owner:peterj */
UnvanishCpCpLim (doc, cp, cpLim)
int doc;
CP cp, cpLim;

{
	CP cpFirstRun = cp;
	CP dcp;
	struct CA ca;

	dcp = DcpUnHideFld1 (doc, cp-1, cpLim, fTrue, &cpFirstRun, 0);
	Assert (cp-1 + dcp == cpLim);
	ApplyFFldVanish (PcaSet( &ca,doc, cpFirstRun, cpLim), fFalse);

}



/* U N F I E L D I F Y  D O C  C P */
/*  This is a recovery function.  It will remove the field at cp and
	replace it with a plain stream of chars.  The FieldBegin/End and
	Separate characters are replaced with their Displayed versions.
	Any last result is left and becomes visible.

	Major points to note:
		-field is no longer a field
		-field still takes same amount of cp space
		-if dead field, removes fFldVanish as well
*/

/* %%Function:UnfieldifyDocCp %%Owner:peterj */
UnfieldifyDocCp (doc, cp)
int doc;
CP cp;

{
	CP dcpInst, dcpResult, cpLim;
	CHAR ch;
	int cch;
	struct CHP chp;
	struct CA caT;
	BOOL fUnVanish = fFalse, fInDead;

	/* we'll unvanish if vanished and not nested in a dead field */
	/* but we can't unvanish until after CchRmFieldAtDocCp or it will barf */
	fInDead = FNestedInDeadField(doc, cp);
	if (FFldVanishDocCp(doc, cp, fTrue) && !fInDead)
		{
		cpLim = CpLimDeadField(doc, cp);
		fUnVanish = fTrue;
		}

	/*  remove field & special field chars */
	cch = CchRmFieldAtDocCp (doc, cp, &dcpInst, &dcpResult);

	/*  replace the special field chars with something that looks the same */
	/* these FInsertRgch calls are likely to succeed even in low memory since
	they won't split the piece table (they replace characters just deleted) */

	ch = chFieldEndDisp;
	StandardChp(&chp);
	/* AssureNestedProps needs UnfieldifyDocCp to set fFldVanish if necessary */
	chp.fFldVanish = fInDead;
	FInsertRgch (doc, cp+dcpInst+dcpResult, &ch, 1, &chp, NULL);

	if (cch > 2)
		{
		ch = chFieldSeparateDisp;
		FInsertRgch (doc, cp+dcpInst, &ch, 1, &chp, NULL);
		}

	ch = chFieldBeginDisp;
	FInsertRgch (doc, cp, &ch, 1, &chp, NULL);

	if (fUnVanish)
		ApplyFFldVanish (PcaSet( &caT, doc, cp, cpLim), fFalse);
}




/* R E S U R R E C T  F I E L D */
/*  Place an entry in the plcfld for the field starting at doc, cp with type
	flt.  If and only if the field is not nested within another dead field
	remove the fFldVanish property from all of its text that is not a nested
	dead field. 
*/

/* %%Function:ResurrectField %%Owner:peterj */
BOOL ResurrectField (doc, cp, flt, fChgView)
int doc;
CP cp;
int flt;
BOOL fChgView;

{
	int fUnvanish;
	int ifld;
	CP dcpInst;
	struct CHP chp;

	Assert (IfldFromDocCp (doc, cp, fTrue) == ifldNil);

	/* determine if we are nested inside a dead field */
	fUnvanish = !FNestedInDeadField (doc, cp);

	/* unvanish the field and determine its extent */
	dcpInst = DcpUnHideFld (doc, cp, fUnvanish);

	/* make an entry in the plcfld */
	if (!FInsertFltDcps (flt, doc, cp, dcpInst, cp0, NULL))
		{
		/* not enough memory */
		return;
		}
	ifld = IfldFromDocCp (doc, cp, fTrue);
	Assert (ifld != ifldNil);

	if (vfInsertMode)
		{
		/*  The field was resurrected because the user, in insert mode,
			changed the keyword.  We must keep the insert code up to date.
			1) subsequently inserted text may not have fFldVanish set and
			2) field type invalidation needs to know what ifld to
			invalidate.
		*/
		vifldInsert = ifld;
		/*  we may be resetting the fFldVanishness */
		selCur.chp.fFldVanish = !fUnvanish;
		chp = selCur.chp;
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		selCur.fUpdateChp = fFalse;
		Debug( vfInsertMode = fFalse );
		FNewChpIns (doc, cpInsert, &chp, stcNil);
		Debug( vfInsertMode = fTrue );
		}

	if (fChgView)
		AssureInstVisible (doc, ifld);
}





/* K I L L  F I E L D */
/*  Make field ifld located at doc, cp a dead field.  Delete any result it
	may have, apply the fFldVanish property to it and remove it from the
	plcfld.
*/

/* %%Function:KillField %%Owner:peterj */
KillField (doc, cp, ifld)
int doc;
CP cp;
int ifld;

{
	struct FLCD flcd;
	struct CA ca;
	struct CHP chp;

	GetIfldFlcd (doc, ifld, &flcd);

	if (flcd.dcpResult)
		DeleteFieldResult (doc, ifld, fTrue, (CP) -1);

	ApplyFFldVanish (PcaSetDcp(&ca, doc, cp, flcd.dcpInst), fTrue);

	DeleteFld (doc, ifld);

	if (vfInsertMode)
		{
		/*  this field was killed because the user has modified the keyword
			to be that of a dead field.  Keep the insert code up to date on
			its correct state.  1) subsequently inserted chars are nested in
			a dead field and have fFldVanish & 2) no field needs to be
			invalidated during subsequent insertion.
		*/
		Assert (vifldInsert == ifld);
		vifldInsert = ifldNil;
		selCur.chp.fFldVanish = fTrue;
		chp = selCur.chp;
		chp.fRMark = PdodMother(selCur.doc)->dop.fRevMarking;
		selCur.fUpdateChp = fFalse;
		selCur.fUpdateRibbon = fTrue;
		Debug( vfInsertMode = fFalse );
		FNewChpIns (doc, cpInsert, &chp, stcNil);
		Debug( vfInsertMode = fTrue );
		}
}






/* C P  M A T C H  F I E L D  B A C K */
/*  Return the cp of a chFieldBegin.  CchFieldBegin is zero or a negative
	number.  If it is zero match one chFieldBegin.  If it is negative match
	the abs(cchFieldBegin)'th chFieldBegin.  If fDeadOnly then continue
	scanning backwards only while characters have fFldVanish set.
*/

/* %%Function:CpMatchFieldBack %%Owner:peterj */
CP CpMatchFieldBack (doc, cp, fDeadOnly, cchFieldBegin)
int doc;
CP cp;
BOOL fDeadOnly;
int cchFieldBegin;

{
	int cch;
	CHAR ch;
	CP cpFetch;
	CP cpCur = cp;
	CHAR rgch [cchBackFetchChunk];

	Assert (cchFieldBegin <= 0);

	if (cchFieldBegin == 0)
		cchFieldBegin = -1;

	while (cpCur > cp0)
		{
		cpFetch = cpCur > cchBackFetchChunk ? cpCur - cchBackFetchChunk : cp0;
		FetchRgch (&cch, rgch, doc, cpFetch, cpCur, cchBackFetchChunk);
		Assert (cch == cpCur - cpFetch);

		while (cpCur > cpFetch)
			{
			cpCur--;
			if ((ch = rgch [--cch]) == chFieldBegin || ch == chFieldEnd)
				{
				CachePara (doc, cpCur);
				FetchCp (doc, cpCur, fcmProps+fcmChars);
				Assert (vccpFetch);
				Assert (*vhpchFetch == ch);

				if (fDeadOnly && !vchpFetch.fFldVanish)
					return cp;

				if (vchpFetch.fSpec)
					if (ch == chFieldEnd)
						cchFieldBegin--;
					else  if /* ch == chFieldBegin */
					(!++cchFieldBegin)
						return cpCur;

				}
			}

		if (fDeadOnly && cpCur > cp0)
			{
			CachePara(doc, cpCur);
			FetchCp (doc, cpCur, fcmProps);
			if (!vchpFetch.fFldVanish)
				return cp;
			}

		}

	Assert (fDeadOnly);
	return cp;

}





/* D C P  U N V A N I S H  F I E L D */
/*  Return the length of a field which starts at doc, cp.  If fUnVanish then
	while scanning the field remove the fFldVanish property.
*/

/* %%Function:DcpUnHideFld %%Owner:peterj */
CP DcpUnHideFld (doc, cp, fUnVanish)
int doc;
CP cp;
BOOL fUnVanish;

{
	CP dcp;
	CP cpFirstRun = cp;
	CP cpLim = CpMacDoc (doc);
	struct CA ca;

	dcp = DcpUnHideFld1 (doc, cp, cpLim, fUnVanish, &cpFirstRun, 0);

	if (fUnVanish)
		ApplyFFldVanish (PcaSet( &ca,doc, cpFirstRun, cp+dcp), fFalse);

	return dcp;

}





/* D C P  U N V A N I S H  F I E L D  1 */
/*  Return the length of the field which begins at doc, cp.  Calls itself
	recursively on nested fields.  If fUnVanish then remove the fFldVanish
	property from runs which are not nested DEAD fields.  *pcp is the
	cpFirst of the current run, where a run is a contigous sequence in the
	cp stream which will all either have fFldVanush property removed or not.
*/

/* %%Function:DcpUnHideFld1 %%Owner:peterj */
CP DcpUnHideFld1 (doc, cp, cpLim, fUnVanish, pcp, cNestLevel)
int doc;
CP cp, cpLim;
BOOL fUnVanish;
CP *pcp;
int cNestLevel;

{
	CP cpCur = cp + 1;
	CHAR HUGE *hpch = vhpchFetch + cp - vcpFetch;
	CP dcp;
	struct CA ca;

	if (++cNestLevel > cNestFieldMax)
		/*  field is nested too deeply.  Replace the field with something
			that is not a field but takes the same amount of space. */
		{
		ErrorEid (eidFields2Deep, " DcpUnHideFld1");
		UnfieldifyDocCp (doc, cp);
		return cp0;
		}

	while (cpCur < cpLim)
		{

		/* assure we have characters */
		if (cpCur < vcpFetch || cpCur >= vcpFetch + vccpFetch ||
				vdocFetch == docNil)
			{
			CachePara (doc, cpCur);
			FetchCp (doc, cpCur, fcmProps+fcmChars);
			if (!vccpFetch)
				{  /* possibly hit end of document */
				Assert (fFalse);
				return cp0;
				}
			hpch = vhpchFetch;
			}
		else
			hpch++;

		if (vchpFetch.fSpec)
			if (*hpch == chFieldBegin)
				if (fUnVanish && IfldFromDocCp (doc, cpCur, fTrue) != ifldNil)
					{
					/* beginning of undead field, simply recurse */
					dcp = DcpUnHideFld1 (doc, cpCur, cpLim, fTrue,
							pcp, cNestLevel);
					if (dcp-- == cp0)
						/*  field was deeply nested, force a re-fetch */
						vdocFetch = docNil;
					cpCur += dcp;
					hpch = vhpchFetch + cpCur - vcpFetch;
					}

				else
					{
					if (fUnVanish)
						/*  beginning of a nested dead field, unvanish *pcp
							thru cpCur-1, recurse with !fUnVanish, on return
							set *pcp to be our new cpCur (cpLast of nested
							dead field)
						*/
						ApplyFFldVanish (PcaSet(&ca,doc, *pcp, cpCur), fFalse);

					dcp = DcpUnHideFld1 (doc, cpCur, cpLim, fFalse,
							pcp, cNestLevel);
					if (dcp-- == cp0)
						/* field was deeply nested, force re-fetch */
						vdocFetch = docNil;
					*pcp = (cpCur += dcp) + 1;
					hpch = vhpchFetch + cpCur - vcpFetch;
					}

			else  if (*hpch == chFieldEnd)
				return cpCur - cp + 1;

		cpCur++;

		}

	return cpCur - cp;

}





/* A P P L Y  F  F L D  V A N I S H */
/*  Set fFldVanish to fVanish for all cps from cp up to cpLim in doc. */

/* %%Function:ApplyFFldVanish %%Owner:peterj */
ApplyFFldVanish (pca, fVanish)
struct CA *pca;
BOOL fVanish;

{
	CHAR grpprl [2];

	if (DcpCa(pca) < 0)
		return;

	grpprl [0] = sprmCFFldVanish;
	grpprl [1] = fVanish;

	InvalCp (pca);
	ApplyGrpprlCa (grpprl, 2 /*cb*/, pca);

}





/* E X P A N D  T O  S A M E  F I E L D */
/*  Scans forward through document until the later of *pcpLim reached and
	all chFieldBegins encountered matched with a chFieldEnd.  If necessary
	increases *pcpLim. If all chFieldEnds encountered were not matched,
	scans backwards decreasing *pcpFirst to match them.
*/

/* %%Function:ExpandToSameField %%Owner:peterj */
ExpandToSameField (doc, pcpFirst, pcpLim)
int doc;
CP *pcpFirst, *pcpLim;

{
	int docFetch = doc;
	int cchFieldBegin = 0;
	CHAR HUGE *hpch = 0, HUGE *hpchMac = 0;
	CP cpCur = *pcpFirst, cpMac = CpMacDoc (doc);

	while ((cchFieldBegin > 0 || cpCur < *pcpLim) && cpCur < cpMac)
		{
		if (hpch >= hpchMac)
			{
			CachePara (doc, cpCur);
			FetchCp (docFetch, cpCur, fcmProps+fcmChars);
			docFetch = docNil;
			if (vchpFetch.fSpec)
				{
				hpchMac = vhpchFetch + vccpFetch;
				hpch = vhpchFetch;
				}
			else
				{
				if (cchFieldBegin > 0)
					cpCur += vccpFetch;
				else
					{
					cpCur = CpMin (cpCur + vccpFetch, *pcpLim);
					docFetch = doc;  /* potentially random access */
					}
				continue;
				}
			}

		if (*hpch == chFieldBegin)
			cchFieldBegin++;

		else  if (*hpch == chFieldEnd)
			if (--cchFieldBegin < 0)
				{
				*pcpFirst = CpMatchFieldBack (doc, *pcpFirst, fFalse,
						cchFieldBegin);
				cchFieldBegin = 0;
				hpch = hpchMac;
				docFetch = doc;
				}

		cpCur++;
		hpch++;
		}

	if (cchFieldBegin > 0)
		{  /* we went over the end--unmatched field */
		/*  FUTURE:  Can we recover from this in a nice way? Doc must be in
			an invalid state, report an error (i.e. non-debug)?? */
		Assert (fFalse);
		return;
		}

	*pcpLim = cpCur;

	Assert(cchFieldBegin >= 0);
}


/* C C H  S K I P  F I E L D  K E Y W O R D */
/* %%Function:CchSkipFieldKeywordArgs %%Owner:peterj */
CchSkipFieldKeywordArgs (doc, ifld, cSkipArgs)
int doc, ifld, cSkipArgs;

{
	struct FFB ffb;

	InitFvb (&ffb.fvb);
	SetFfbIfld (&ffb, doc, ifld);
	/*  skip keyword */
	FFetchArg (&ffb, NULL, 0);
	while (cSkipArgs--)
		/*  skip additional arguments */
		FFetchArg (&ffb, NULL, 0);
	Assert (ffb.cpFirst - ffb.cpField <= 0x7fff);
	return ffb.cpFirst - ffb.cpField;
}



/* S E T  F F B  D E A D  F L T */
/*  Set up pffb to fetch the instructions of a dead field of type flt at
	doc, cp.
*/

/* %%Function:SetFfbDeadFlt %%Owner:peterj */
SetFfbDeadFlt (pffb, doc, cp, flt)
struct FFB *pffb;
int doc;
CP cp;
int flt;

{
	pffb->doc = doc;
	pffb->cpFirst = cp + 1;
	pffb->cpLim = CpLimDeadField (doc, cp) - 1;
	pffb->fOverflow = fFalse;

	SetBytes (&pffb->fsfb, 0, sizeof (struct FSFB));
	pffb->flt = flt;
	pffb->cpField = cp;

	CachePara (doc, cp);
	FetchCp (doc, cp, fcmProps);
	Assert (vchpFetch.fSpec && vchpFetch.fFldVanish);
	pffb->fFetchVanished = vchpFetch.fVanish || vchpFetch.fStrike
			|| vchpFetch.fFldVanish;
	pffb->fFetchTable = FInTableDocCp(doc, cp);
}




/* F  S C A N  F O R  D E A D  F L T */
/*  Scan a document for fields of type flt.  Note: flt must be a field
	with fDead set!  Sets up *pffb to fetch arguments from found field.
*/

/* %%Function:FltScanForDeadField %%Owner:peterj */
int FltScanForDeadField (pffb, pca)
struct FFB *pffb;
struct CA *pca;

{
	int doc;
	int docFetch = doc = pca->doc;
	CP cp = pca->cpFirst;
	CHAR HUGE *hpch, HUGE *hpchLim;
	int flt;

	for (;;)
		{
		/*  scan through the doc until we find vanished fSpec chars or
			we run out of doc. */
		do
			{
			Assert (cp < pca->cpLim && cp < CpMac2Doc (doc));
			CachePara (doc, cp);
			FetchCp (docFetch, cp, fcmProps);
			docFetch = docNil;
			} 
		while ( (!vchpFetch.fSpec || !vchpFetch.fFldVanish) &&
				(cp += vccpFetch) < pca->cpLim && !vfEndFetch);

		if (vfEndFetch || cp >= pca->cpLim)
			{
			pffb->cpFirst = pca->cpFirst = pca->cpLim;
			return fltNil;
			}

		Assert (vchpFetch.fSpec && vchpFetch.fFldVanish);
		FetchCp ((docFetch = doc), cp, fcmChars+fcmProps);
		hpch = vhpchFetch;
		hpchLim = hpch + vccpFetch;

		/*  we have a run of dead fSpec chars, search for the beginning
			of a dead field in this run. */
		while (hpch < hpchLim && (*hpch != chFieldBegin || 
				IfldFromDocCp (doc, cp, fTrue) != ifldNil))
			{
			hpch++;
			cp++;
			}

		/*  we have either exausted the run OR we are at the beginning
			of a dead field. */
		if (hpch < hpchLim)
			{
			pca->cpFirst = cp + 1;
			flt = FltParseDocCp (doc, cp, ifldNil, fFalse /* fChgView */, fFalse /* fEnglish */);
			SetFfbDeadFlt (pffb, doc, cp, flt);
			return flt;
			}
		}

}



/* F  N E S T E D  I N  D E A D  F I E L D */
/*  Determine if the character at doc, cp is nested within a dead field. 
	Note that if doc, cp is chFieldBegin of a dead field it still may
	not be nested within a dead field.
*/

/* %%Function:FNestedInDeadField %%Owner:peterj */
FNestedInDeadField (doc, cp)
int doc;
CP cp;

{
	if (!FFldVanishDocCp (doc, cp-1, fTrue /* fFetch */) || 
			!FFldVanishDocCp (doc, cp, fFalse))
		return fFalse;

	else
		return CpMatchFieldBack (doc, cp, fTrue /* fDead */, 0) != cp;

}


/* C P  L I M  D E A D  F I E L D */
/*  Figure out where the end of a dead field is.
*/

/* %%Function:CpLimDeadField %%Owner:peterj */
CP CpLimDeadField (doc, cpFirst)
int doc;
CP cpFirst;

{
	CP cpLim = cpFirst + 2;

#ifdef DEBUG
	CP cpFirstOrig = cpFirst;
#endif /* DEBUG */

	Assert (IfldFromDocCp (doc, cpFirst, fTrue) == ifldNil &&
			FFldVanishDocCp (doc, cpFirst, fTrue /* fFetch */));

	ExpandToSameField (doc, &cpFirst, &cpLim);

	Assert (cpFirstOrig == cpFirst);

	return cpLim;
}


/* %%Function:FFetchArg %%Owner:peterj */
BOOL FFetchArg (pffb, rgch, cchMax)
struct FFB *pffb;
CHAR *rgch;
int cchMax;

{
	struct FBB fbb;
	int cch;

	fbb = pffb->fbb;

	pffb->ccrMax = 0;
	pffb->rgcr = NULL;

	pffb->cchMax = rgch == NULL ? 0 : cchMax-1;
	pffb->rgch = rgch;

	FetchFromField (pffb, fTrue /* fArgument */, fFalse /* fRTF */);

	cch = pffb->cch;
	if (rgch != NULL)
		rgch [cch] = '\0';

	Assert (cch == pffb->cchMax || !pffb->fOverflow);

	if (pffb->fOverflow)
		{
		pffb->cchMax = 0;
		pffb->rgch = NULL;

		FetchFromField (pffb, fTrue, fFalse);

		/*  nothing to overflow!! */
		Assert (!pffb->fOverflow);
		}

	pffb->fbb = fbb;  /* restore orginal buffers */

	pffb->cch = cch;
	pffb->ccr = 0;

	return !pffb->fNoArg;

}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Fieldsp_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Fieldsp_Last() */
