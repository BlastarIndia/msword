/* D L G L O O K 1 . C */
/* FormatCharacter and FormatParagraph dialogs */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "debug.h"
#include "format.h"
#include "prm.h"
#include "print.h"
#include "error.h"
#include "rerr.h"
#include "prompt.h"
#include "wininfo.h"
#include "style.h"
#include "opuscmd.h"
#include "dlbenum.h"
#include "message.h"
#include "ch.h"
#include "keys.h"
#include "inter.h"
#include "border.h"
#include "field.h"

#include "idd.h"
#include "rareflag.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "para.hs"
#include "para.sdm"
#include "char.hs"
#include "char.sdm"



/* Externals */

extern BOOL vfRecording;
extern struct RF	vrf;
extern struct SEL selCur;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
extern struct CA caSect;
extern struct CA caPara;
extern struct SEP vsepFetch;
extern struct PAP vpapFetch;
extern BOOL vfDefineStyle;
extern struct ESPRM dnsprm [];
extern struct PREF vpref;
extern HWND vhwndApp;
extern HWND vhwndPgPrvw;
extern BOOL fElActive;
extern int vstcStyle;
extern int vlm;
extern int vfShowAreas;
extern char szEmpty [];
extern struct STTB ** vhsttbFont;
extern struct PRI vpri;
extern struct PLAOR **vhplaor;
extern struct LBS vlbsText, vlbsText2;
extern struct MERR vmerr;
extern KMP **hkmpCur;
extern void ToggleShowStd();
extern struct WWD       **hwwdCur;
extern struct MWD ** hmwdCur;

BOOL FCkMarginsSect(HCAB);
TMC TmcParagraph(CMB *);
TMC TmcPosition(CMB *);


#define hpsSubSupDefault (6)   /* default "by" value in char dialog */
#define qpsCompDefault   (6)
#define qpsExpDefault    (12)

#define czaLIndentMin (-(czaMax /2))  /* -11" limit */


/* The following maps are dependent on the order of the cab entries */

csconst SPNT mpiagspntPap [] =
{
/* sprmPStc MUST be the first entry in this table to ensure it is the first
	prl in a grpprl created by NewGrpprlFromRgProp. */
	sprmPStc,		wNinch,
			sprmPJc,		uNinchRadio,
			sprmPDxaLeft,		wNinch,
			sprmPDxaRight,		wNinch,
			sprmPDxaLeft1,		wNinch,
			sprmPDyaBefore,		uNinch,
			sprmPDyaAfter,		uNinch,
			sprmPDyaLine,		wNinch,
			sprmPFKeep,		uNinchCheck,
			sprmPFKeepFollow,	uNinchCheck,

			sprmNoop,		uNinchRadio, /* placeholders for ibrcl, ibrcp */
	sprmNoop,		uNinchRadio,

			sprmPFPageBreakBefore,	uNinchCheck,
			sprmPFNoLineNumb,	uNinchCheck,
			sprmPBrcTop,            uNinch,
			sprmPBrcLeft,           uNinch,
			sprmPBrcBottom,         uNinch,
			sprmPBrcRight,          uNinch,
			sprmPBrcBetween,        uNinch,
			sprmPBrcBar,            uNinch
};


#define iagPapMac (sizeof (mpiagspntPap) / sizeof (SPNT))

struct PRBPAP
	{
	WORD cwPrb;
	WORD stc;
	WORD Jc;
	WORD dxaLeft;
	WORD dxaRight;
	WORD dxaLeft1;
	WORD dyaBefore;
	WORD dyaAfter;
	WORD dyaLine;
	WORD fKeep;
	WORD fKeepFollow;

	WORD ibrcp;
	WORD ibrcl;

	WORD fPageBreakBefore;
	WORD fNoLineNumb;

	WORD brcTop;
	WORD brcLeft;
	WORD brcBottom;
	WORD brcRight;
	WORD brcBetween;
	WORD brcBar;
};

#define cwPrbPap (sizeof (struct PRBPAP) / sizeof (WORD))
#define cwPrbPaps (offset (PRBPAP, brcTop) / sizeof (WORD))

/* this map must be kept in exact sync with the above CAB description. Note
	that it skips the string fields but uses the values beyond those known
	by SDM
*/

csconst SPNT mpiagspntChp[] =
	{
	sprmCFtc,		wNinch,
	sprmCHps,		wNinch,
	sprmCIco,		uNinchList,
	sprmCFBold,		uNinchCheck,
	sprmCFItalic,		uNinchCheck,
	sprmCFSmallCaps,	uNinchCheck,
	sprmCFVanish,		uNinchCheck,

	/* KLUDGE ALERT: fCharUL field of cab holds the kul! */
	sprmCKul,        	uNinchCheck,
	sprmNoop,		0,
	sprmNoop,		0,

	sprmNoop,		0, /* sub/superscript code */

	sprmCHpsPos,		wNinch,    /* sub/superscript value */

	sprmNoop,		0, /* Intercharacter spacing. */
	sprmCQpsSpace,		wNinch
	};


#define iagChpMac sizeof(mpiagsprmChp)


#define IFromSign(qps) (((qps) > 0) ? \
		(tmcCharISExpanded - tmcCharISNormal) : \
				(((qps) < 0) ? (tmcCharISCompressed - tmcCharISNormal) : 0))

#define SignFromI(_i) (((_i) > 0) ? 1 : (((_i) < 0) ? -1 : 0))


/* N E W  P R O P  T O  C A B  G R A Y */
/* NEW SDM VERSION */
/* writes properties specified by mpiagsprm into cab */
/*  %%Function:  NewPropToCabGray  %%Owner:  bobz       */

NewPropToCabGray(hcab, prgbProp, prgbGray, mpiagspnt)
HCAB hcab;
char * prgbProp;
char * prgbGray;
SPNT mpiagspnt [];
{
	int sprm;
	int iag = 0;
	int iagMac = ((CABH *) *hcab)->cwSimple;

	OurNinchCab(hcab, mpiagspnt);

	for ( ; iag < iagMac; ++iag)
		{
		if ((sprm = mpiagspnt[iag].sprm) != sprmNoop)
			{
			if (ValFromPropSprm(prgbGray, sprm) == 0)
				{
				WORD val;

				val = ValFromPropSprm(prgbProp, sprm);

				/* Translate val if necessary... */
				switch (sprm)
					{
				case sprmPStc:
				case sprmCKul:
					continue;
					}

				*((WORD *) (*hcab) + cwCabMin + iag) = val;
				}
			}
		}
}



/*  %%Function:  OurNinchCab  %%Owner:  bobz       */

OurNinchCab(hcab, mpiagspnt)
HCAB	hcab;
SPNT	mpiagspnt[];
{
	int	iag;
	int	iagMac;

	iagMac = ((CABH *) *hcab)->cwSimple;
	for (iag = 0; iag < iagMac; iag++)
		{
		if (mpiagspnt[iag].sprm != sprmNoop)
			{
			*((WORD *) (*hcab) + cwCabMin + iag) =
					(mpiagspnt[iag].wNinchVal == 0) ? wNinch :
					mpiagspnt[iag].wNinchVal;
			}
		}
}


/*  %%Function:  TmcGosubParagraph  %%Owner:  bobz       */

TMC TmcGosubParagraph()
{
	TMC tmc;
	CMB cmb;


	if ((cmb.hcab = HcabAlloc(cabiCABPARALOOKS)) == hNil)
		return tmcError;

	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;
#ifdef DEBUG_BCH
	CommSzNum(SzShared("TmcGosubParagraph: cmb.hcab = "), cmb.hcab);
#endif

	tmc = TmcParagraph(&cmb);

	FreeCab(cmb.hcab);

	return tmc;
}


/*  %%Function:  CmdParagraph  %%Owner:  bobz       */

CMD CmdParagraph(pcmb)
CMB * pcmb;
{
	TMC tmc;

#ifdef PROFILE
	static int fNotFirst = fFalse;

	if (fNotFirst)
		vpfi == pfiParaDlg ? StartProf(30) : 0;
#endif /* PROFILE */

	tmc = TmcParagraph(pcmb);

#ifdef PROFILE
	if (fNotFirst)
		vpfi == pfiParaDlg ? StopProf() : 0;

	fNotFirst = fTrue;
#endif	/* PROFILE */

	return (tmc == tmcOK || tmc == tmcParTabs) ? cmdOK :
			  tmc == tmcCancel ? cmdCancelled : cmdError;
}


/*  %%Function:  TmcParagraph  %%Owner:  bobz       */

TMC TmcParagraph(pcmb)
CMB * pcmb;
{
	extern char (** vhmpiststcp) [];
	extern int vdocStsh;

	int tmc = tmcOK;
	CABPARALOOKS *pcabparalooks;

	if (!vfDefineStyle && selCur.fUpdatePap)
		{
		/* get paragraph properties */
		AssertDo(FGetParaState(fFalse /* fAll */ , fFalse /* fAbortOk */));
		}

	Assert (!selCur.fUpdatePap || vfDefineStyle);

	/* If we're in define style mode, or the ruler is up, vhmpiststcp
			may have already been allocated... */
	Assert(vfDefineStyle || vhmpiststcp == hNil ||
			(hmwdCur != hNil && (*hmwdCur)->hwndRuler != NULL));

	if (vhmpiststcp == hNil && 
			(vhmpiststcp = HAllocateCw(cwMpiststcp)) == hNil)
		{
		return tmcError;
		}

	if (pcmb->fDefaults)
		{
		struct PAP pap, * ppap, * ppapGray;

		/*
		*  Get all nongray properties indicated by mpiagsprmPap into
		*  the cab. All non-handle entries beyond those SDM knows
		*  about will be wNinch if gray.  If called from DefineStyle,
		*  get props from stsh, not the current selection.
		*/

		if (vfDefineStyle)
			{
			SetVdocStsh(selCur.doc);
			MapStc(PdodDoc(vdocStsh), vstcStyle, NULL, &pap);
			ppap = &pap;
			ppapGray = NULL;
			}
		else
			{
			ppap = &vpapSelCur;
			ppapGray = &vpapGraySelCur;
			}

		if (!FGetParaDefaults(pcmb, ppap, ppapGray, fFalse /* fPaps */,
				fTrue /* fStyList */))
			{
			tmc = tmcError;
			goto LRet;
			}

		((CABPARALOOKS *) *pcmb->hcab)->sab = 0;
		}
	else  if (fElActive)
		{
		/* Needs to be redone for macros... */
		SetVdocStsh(selCur.doc);
		GenLbStyleMapping();
		}


	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltParaLooks)];

		Assert (!vfDefineStyle ? vhmpiststcp != hNil : fTrue);

		BltDlt(dltParaLooks, dlt);

#ifdef DBGYXY
		CommSzNum(SzShared("TmcParagraph: pcmb->hcab = "), pcmb->hcab);
		CommSzNum(SzShared("pcabparalooks->jc: "), pcabparalooks->iJustify);
#endif
		tmc = TmcOurDoDlg(dlt, pcmb);
		}

	if (pcmb->fCheck)
		{
		if (FFinishParCab(pcmb->hcab, &tmc))
			tmc = tmcOK;
		else
			return tmcError;
		}

LRet:
	if (!vfDefineStyle && vhmpiststcp != hNil)
		FreePh(&vhmpiststcp);

/* Do the work */
	if (pcmb->fAction)
		{

		switch (tmc)
			{
#ifdef DEBUG
		default:
			Assert (fFalse);
			break;
#endif   /* DEBUG */

		case tmcError:
			break;

		case tmcCancel:
			break;

		case tmcParTabs:
			if (!vfDefineStyle)
				ChainCmd(bcmTabs);
			/* FALL THROUGH */

		case tmcOK:
			if (!FApplyHcabToPara(pcmb->hcab))
				return tmcError;
			}
		}

	return tmc;
}


/* taken from border.c in MacWord and later modified slightly by BL*/
/* B R C S  F R O M  I B R C L  I B R C P */
/* Given a brcl and a brcp (old Word 3 borders) return
	an array of "new" style borders */
/*  %%Function:  BrcsFromIbrclIbrcp  %%Owner:  bobz       */

BrcsFromIbrclIbrcp(pbrc, ibrcl, ibrcp)
int *pbrc;
int ibrcl, ibrcp;
{
	int brc;

	SetWords( pbrc, 0, 6 );
	/* if none, we don't care about line type */
	if (ibrcp == ibrcpNone)
		return;

	/* Determine the kind of line */
	switch (ibrcl)
		{
	default:      /* unrecognized */
		SetWords( pbrc, uNinch, 6 );
		/* fall through */
	case ibrclNone:	 /* will cause both to be set to none */
		return;
	case ibrclSingle:
		brc = brcSingle;
		break;
	case ibrclDouble:
		brc = brcTwoSingle;
		break;
	case ibrclThick:
		brc = brcThick;
		break;
	case ibrclShadow:
		brc = brcSingle | brcfShadow;
		break;
		}

	/* hard coded spacing, defined in border.h  */
	/* Note: Hungarian is wrong here - dxpSpace is in points */
	((struct BRC)brc).dxpSpace = dptBrcSpace;

	switch (ibrcp)
		{
	default:      /* unrecognized */
		SetWords( pbrc, uNinch, 6 );
		return;
	case ibrcpBox:
		SetWords( pbrc, brc, 4 );
		break;
	case ibrcpAbove:
		*(pbrc) = brc;
		break;
	case ibrcpBelow:
		*(pbrc+2) = brc;
		break;
	case ibrcpBar:
		*(pbrc+5) = brc;
		break;
		}

	/* between bit always 0 for Opus. Set in initial clear  */
	Assert (brcNone == 0);
}


/* F  F I N I S H  P A R  C A B */
/* Fill out the post-dialog entries in the paragraph dialog cab given
the in-dialog items. */
/*  %%Function:  FFinishParCab  %%Owner:  bobz       */

BOOL FFinishParCab(hcab, ptmc)
HCABPARALOOKS hcab;
TMC *ptmc;
{
	CABPARALOOKS * pcab;
	int iag;
	int tmcSpace;
	int stc;
	int xaWidth;
	char stzStyle [cchMaxStyle + 2];
	struct DOP dop;
	long lxaIndent;
	int xaIndentFirst, xaIndentLeft;

	*ptmc = tmcNull;
	CacheSect(selCur.ca.doc, selCur.ca.cpFirst);
	/* Perform range check on indentation. */
	dop = PdodMother(selCur.doc)->dop;

	xaWidth = dop.dxaLeft + dop.dxaRight + dop.dxaGutter + 
			(vsepFetch.ccolM1 * vsepFetch.dxaColumns);
	xaWidth = (dop.xaPage - xaWidth) / (vsepFetch.ccolM1 + 1);

	pcab = *hcab;

	/* set the values corresponding to gray indents to 0 - this will
	remove their influence in the comparisons, but still allow
	some checking. This could allow indents that would make the
	page size negative, but the ruler allows that, and it will make
	the para be 1 char wide.
	
	we could avoid that by going through all the paras in the selection
	and picking the max indent from all, and comparing to that, but
	it seemed like too much work. bz 4/6/89
	*/

	xaIndentLeft = pcab->wParLeftIn == wNinch ? 0 : pcab->wParLeftIn;
	xaIndentFirst = pcab->wParFirstIn == wNinch ? 0 : pcab->wParFirstIn;
	lxaIndent = xaIndentLeft +
			(pcab->wParRightIn == wNinch ?
			0L : (long) (int)pcab->wParRightIn);

	/* leave at least a tab width */
	if (lxaIndent > (long)(xaWidth - dop.dxaTab) ||
			lxaIndent + (long)xaIndentFirst >= xaWidth ||
			/* first  and left indent, and sum must be >= -11" */
	xaIndentLeft < czaLIndentMin ||
			(long)xaIndentLeft + (long)xaIndentFirst
			< (long)czaLIndentMin)
		{
		ErrorEid(eidIndentTooLarge, "FFinishParCab");
		*ptmc = tmcParLeftIn;
		return fFalse;
		}

	if ((long)xaWidth - lxaIndent >= czaMax ||
			(long)xaWidth - lxaIndent +
			(long) xaIndentFirst >= czaMax)
		{
		ErrorEid(eidParTooWide, "FFinishParCab");
		*ptmc = tmcParLeftIn;
		return fFalse;
		}

	/* Style combo is disabled when called from Style dialog */
	if (!vfDefineStyle)
		{
		/* Ensure valid style name is present */
		GetCabStz(hcab, stzStyle, sizeof (stzStyle), 
				Iag(CABPARALOOKS, hszStyle));

		if (*stzStyle != 0)
			{
			int id, fRedef = fFalse;
			id = IdApplyStyle(&stc, stzStyle, fFalse, fFalse, &fRedef, 
					fFalse, fFalse);
			if (id == IDNO || id == -1)
				{
				*ptmc = tmcParStyle;
				return fFalse;
				}
			}
		else
			{
			stc = wNinch;
			}

		vstcStyle = stc;
		}

	pcab = *hcab;
	if (selCur.ca.cpLim > caSect.cpLim || !vsepFetch.nLnnMod)
		pcab->fParNoLNum = uNinchCheck;
	else  if (pcab->fParNoLNum != uNinchCheck)
		pcab->fParNoLNum = !pcab->fParNoLNum;

	return fTrue;
}


/*  %%Function:  FDlgParaLooks  %%Owner:  bobz       */

BOOL FDlgParaLooks(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	HCAB hcab;
	CHAR sz[cchMaxSz];
	int val;

	switch (dlm)
		{
	case dlmIdle:
		/* we can do idle time processing here.  wNew is cIdle */
		switch (wNew)
			{
		case 0:
			return fTrue;  /* call FSdmDoIdle and keep idling */
			}
		return fFalse;

	case dlmInit:
		CacheSect(selCur.ca.doc, selCur.ca.cpFirst);
		if (selCur.ca.cpLim > caSect.cpLim || !vsepFetch.nLnnMod)
			{
			EnableTmc(tmcParNoLNum, fFalse);
			}

		if (vfDefineStyle)
			EnableTmc(tmcParStyle, fFalse);

		AddKeyPfn(hkmpCur, KcCtrl(vkShowStd), ToggleShowStd);

		Assert(HcabDlgCur());
		val = ((CABPARALOOKS *) *(HcabDlgCur()))->iBrcp;
		if (val == ibrcpNone || val == iszNinchList)
			EnableTmc(tmcParBrcl, fFalse);
		else  if (val == ibrcpBar || val == ibrcpAbove)
			{
			/* this will force early lb creation, but we have to
				do it now. Default fill will include shadow */
			Assert (CEntryListBoxTmc(tmcParBrcl) == ibrclShadow + 1);
			DeleteListBoxEntry(tmcParBrcl, ibrclShadow);
			}
		if (fElActive)
			EnableTmc(tmcParTabs, fFalse);

		break;

	case dlmClick:
		if (tmc == tmcParBrcp)
			{
			if (wNew == ibrcpNone || wNew == iszNinchList)
				{
				SetTmcVal(tmcParBrcl, ibrclNone);
				EnableTmc(tmcParBrcl, fFalse);
				}
			else
				{
				if (!FEnabledTmc(tmcParBrcl))
					EnableTmc(tmcParBrcl, fTrue);

				if (wNew == ibrcpBar || wNew == ibrcpAbove)
					{
					if (ValGetTmc(tmcParBrcl) == ibrclShadow)
						SetTmcVal(tmcParBrcl, ibrclSingle);
					/* if shadow present, remove */
					if (CEntryListBoxTmc(tmcParBrcl) == ibrclShadow + 1)
						DeleteListBoxEntry(tmcParBrcl, ibrclShadow);
					}
				else  /* box or below */					
					{
					/* add shadow if missing */
					if (CEntryListBoxTmc(tmcParBrcl) == ibrclShadow)
						{
						Look1CopyEntblToSz(iEntblBrcl, ibrclShadow, sz);
						AddListBoxEntry(tmcParBrcl, sz);
						}
					}
				if (ValGetTmc(tmcParBrcl) == ibrclNone)
					SetTmcVal(tmcParBrcl, ibrclSingle);
				}
			}
		break;
	case dlmTerm:
		if (tmc != tmcOK && tmc != tmcParTabs)
			break;
		/* Fill the cab first. */
		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			{
			/* sdm will take down dialog */
			return (fTrue);
			}

		if (!FFinishParCab(hcab, &tmc))
			{
			if (tmc != tmcNull)
				{
				SetTmcTxs(tmc, TxsOfFirstLim(0, ichLimLast));
				}
			return fFalse;
			}
		/* break; */
		}

	return fTrue;
}


csconst CHAR stAuto[] = StKey("Auto",AutoDef);

/* Character Dialog Stuff */

/* ****
*
*  Gosub tunnel button for character looks
*
** ***/
/*  %%Function:  TmcGosubCharacter  %%Owner:  bobz       */

int TmcGosubCharacter()
{
	/* allocate a local Cab on the stack */
	TMC tmc;
	CMB cmb;

	if ((cmb.hcab = HcabAlloc(cabiCABCHARACTER)) == hNil)
		return tmcError;

	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;
	tmc = TmcCharacterLooks(&cmb);

	FreeCab(cmb.hcab);

	return tmc;
}


/* ****
*
*  Menu level command for character looks
*
** ***/
/*  %%Function:  CmdCharacter  %%Owner:  bobz       */

CMD CmdCharacter(pcmb)
CMB * pcmb;
{
	TMC tmc;

	tmc = TmcCharacterLooks(pcmb);

	vrf.fPreloadSelFont = fTrue;	/* Preload font in Idle */

	return tmc == tmcOK ? cmdOK : tmc == tmcCancel ? cmdCancelled : cmdError;
}


/* ****
*
*  Actual processing routine for character looks
*
** ***/

/*  %%Function:  TmcCharacterLooks  %%Owner:  bobz       */

int TmcCharacterLooks(pcmb)
CMB * pcmb;
{
	char * sz;
	char rgb [cchMaxSz];
	extern int vdocStsh;


	if (!vfDefineStyle && selCur.fUpdateChpGray)
		{
		FGetCharState(fFalse /* fAll */, fFalse /* fAbortOk */);
		}

	if (pcmb->fDefaults)
		{
		struct CHP chp, * pchp, * pchpGray;

		if (vfDefineStyle)
			{
			struct PAP pap;

			SetVdocStsh(selCur.doc);
			MapStc(PdodDoc(vdocStsh), vstcStyle, &chp, &pap);
			pchp = &chp;
			pchpGray = NULL;
			}
		else
			{
			pchp = &selCur.chp;
			pchpGray = &vchpGraySelCur;
			}

		GetCharDefaults(pcmb, pchp, pchpGray);
		((CABCHARACTER *) *pcmb->hcab)->sab = 0;
		}

	if (pcmb->fCheck)
		{
		CABCHARACTER * pcab;

		pcab = *pcmb->hcab;
		if ((pcab->hps < 4 || pcab->hps > 127) && pcab->hps != wNinch)
			{
			ErrorEid(eidDxaOutOfRange, "TmcCharacterLooks");
			return tmcError;
			}
		}

	if (pcmb->fDialog)
		{
		CABCHARACTER * pcab;
		char dlt [sizeof (dltCharacter)];

		if (!pcmb->fDefaults)
			{
			/* Convert action cab (from macro) to dialog cab */
			pcab = *pcmb->hcab;
			if (pcab->wCharQpsSpacing != wNinch)
				{
				int qps = pcab->wCharQpsSpacing;

				if (qps < 0)
					{
					pcab->wCharQpsSpacing = -qps;
					pcab->iCharIS = 2;
					}
				else  if (qps > 0)
					{
					pcab->iCharIS = 1;
					}
				else
					{
					pcab->iCharIS = 0;
					}
				}

			if (pcab->wCharHpsPos != wNinch)
				{
				int hps = pcab->wCharHpsPos;

				if (hps < 0)
					{
					pcab->wCharHpsPos = -hps;
					pcab->iCharPos = 2;
					}
				else  if (hps > 0)
					{
					pcab->iCharPos = 1;
					}
				else
					{
					pcab->iCharPos = 1;
					}
				}

			}
#ifdef BZ
		CommSzRgNum(SzShared("cabChar before dialog: "),
				((WORD *) (*(pcmb->hcab))) + cwCabMin,
				((CABH *) *(pcmb->hcab))->cwSimple);
#endif

		BltDlt(dltCharacter, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return tmcError;
#endif

		case tmcError:
			return tmcError;

		case tmcCancel:
			return tmcCancel;

		case tmcOK:
			break;
			}

		/* Convert dialog cab to action cab */
		pcab = *pcmb->hcab;

		if (pcab->iCharIS == 0)   /* normal */
			pcab->wCharQpsSpacing = 0;
		else  if (pcab->iCharIS == 2)
			pcab->wCharQpsSpacing = -pcab->wCharQpsSpacing;

		if (pcab->iCharPos == 0)
			pcab->wCharHpsPos = 0;
		else  if (pcab->iCharPos == 2)
			pcab->wCharHpsPos = -pcab->wCharHpsPos;

		if (vfRecording)
			{
			/* Record the translated values... */
			FRecordCab(pcmb->hcab, IDDCharacter, tmcOK, fFalse);
			}
		}

	if (pcmb->fAction)
		{
		CABCHARACTER * pcab;
		if (!FApplyHcabToChar(pcmb->hcab))
			return tmcCancel;

		pcab = *pcmb->hcab;
		if (pcab->fCharHidden)
			{
			if (!selCur.fIns)
				MakeSelCurVisi(fTrue /*fForceBlockToIp*/);
			else
				InvalVisiCache();
			}
		}


	return tmcOK;
}


/*  %%Function:  FApplyHcabToChar  %%Owner:  bobz       */

FApplyHcabToChar(hcab)
HCABCHARACTER hcab;
{
	CABCHARACTER * pcab;
	int hpsPos, qpsSpace, fCharULine;
	BOOL fRet;
	SPNT mpiagspnt [sizeof(mpiagspntChp) / sizeof (SPNT)];

	pcab = *hcab;

	/* Convert pos/neg scripting values */
	if ((hpsPos = pcab->wCharHpsPos) < 0 && hpsPos != wNinch)
		pcab->wCharHpsPos = 256 + hpsPos;

	/* Convert pos/neg spacing values */
	if ((qpsSpace = pcab->wCharQpsSpacing) < 0 && qpsSpace != wNinch)
		pcab->wCharQpsSpacing = 64 + qpsSpace;

	if ((fCharULine = pcab->fCharULine) != uNinchCheck)
		{
		int kul;

		if (pcab->fCharULine == fTrue)
			kul = kulSingle;
		else  if (pcab->fCharWordUL == fTrue)
			kul = kulWord;
		else  if (pcab->fCharDUL == fTrue)
			kul = kulDouble;
		else
			kul = kulNone;

		pcab->fCharULine = kul;
		}


	/* Apply the properties */
	bltbx((CHAR FAR *)mpiagspntChp, (CHAR FAR *) mpiagspnt, 
			sizeof (mpiagspntChp));
	fRet = FNewPropFromRgProp(hcab, fTrue /* fCab */, mpiagspnt);


	/* Restore original cab values */
	pcab = *hcab;
	pcab->wCharHpsPos = hpsPos;
	pcab->wCharQpsSpacing = qpsSpace;
	pcab->fCharULine = fCharULine;

	return fRet;
}


/*  %%Function:  FApplyHcabToPara  %%Owner:  bobz       */

FApplyHcabToPara(hcab)
HCABPARALOOKS hcab;
{
	CABPARALOOKS * pcab;
	SPNT mpiagspnt [sizeof(mpiagspntPap) / sizeof (SPNT)];
	struct PRBPAP prbPap;

	bltbx((CHAR FAR *)mpiagspntPap, (CHAR FAR *) mpiagspnt, 
			sizeof (mpiagspntPap));

	prbPap.cwPrb = cwPrbPap - 1;
	CabToPrb(&prbPap, hcab, 0, ((CABH *) *hcab)->cwSimple);

	pcab = *hcab;
	BrcsFromIbrclIbrcp(&prbPap.brcTop, pcab->iBrcl,
			pcab->iBrcp);

	return FNewPropFromRgProp(&prbPap, fFalse /* fCab */, mpiagspnt);
}



/*  %%Function:  FDlgCharacter  %%Owner:  bobz       */

BOOL FDlgCharacter(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	switch (dlm)
		{

#ifdef NOTUSED
	case dlmInit:
		break;
#endif 


	case dlmIdle:
		/* we can do idle time processing here.  wNew is cIdle */
		switch (wNew)
			{
		case 0:
			return fTrue;  /* call FSdmDoIdle and keep idling */
			}
		return fFalse;

	case dlmChange:
			{
			int val;

			switch (tmc)
				{
			case (tmcCharName & ~ftmcGrouped):
			/* Font has changed; dirty the size combo so we'll
				enumerate sizes next time it's dropped. */
				RedisplayTmc(tmcCharSize);
				break;

			case tmcCharHpsPos:
				if ((val = ValGetTmc(tmcCharPos)) == 0
						|| val == uNinchRadio)
					SetTmcVal(tmcCharPos, 1);
				break;

			case tmcCharQpsSpacing:
				if ((val = ValGetTmc(tmcCharIS)) == 0
						|| val == uNinchRadio)
					SetTmcVal(tmcCharIS, 1);
				break;
				}
			break;
			}
	case dlmClick:
		switch (tmc)
			{
			TMC tmcT;
			int val;

		case tmcCharULine:
		case tmcCharWordUL:
		case tmcCharDUL:
			/* Turn off or gray all underline buttons except
				the one hit */
			val = ValGetTmc(tmc);
			/* fTrue is valid since these are checkboxes not radios */
			/* if true, set all others false; if false, all will be
				set false; if gray, all will be set gray.
			*/
			if (val == fTrue)
				val = fFalse;
			for (tmcT = tmcCharULine; tmcT <= tmcCharDUL; tmcT++)
				{
				if (tmcT != tmc)
					SetTmcVal(tmcT, val);
				}
			break;

		case tmcCharPosNormal:
			SetTmcVal(tmcCharHpsPos, wNinch);
			break;

		case tmcCharPosSuper:
		case tmcCharPosSub:
			if ((val = ValGetTmc(tmcCharHpsPos)) == wNinch ||
					val == 0)
				{
				SetTmcVal(tmcCharHpsPos, hpsSubSupDefault);
				}
			break;

		case tmcCharISNormal:
			SetTmcVal(tmcCharQpsSpacing, wNinch);
			break;

		case tmcCharISExpanded:
		case tmcCharISCompressed:
			SetTmcVal(tmcCharQpsSpacing,
					(tmc == tmcCharISCompressed) ? 
					qpsCompDefault : qpsExpDefault);
			/* break; */
			}  /* switch(tmc) */

		/* break; */  /* dlmClick */
		}

	return fTrue;
}


/* Special parse function specific to Character dialog */
/*  %%Function:  WParseHpsQps  %%Owner:  bobz       */

EXPORT WORD WParseHpsQps(tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	int za;
	DPV dpv;
	int wHigh, wLow;
	int wFactor;
	WORD w;

#ifdef DEBUG_BCH
	CommSzNum(SzShared("WParseHpsQps: tmm="), tmm);
#endif


	wLow = 0;

	switch (tmc)
		{
	case tmcCharHpsPos:
		wFactor = 2;
		if (tmm == tmmParse)
			{
			/* Yeah, subscript could go down to 128, but then
				FormatLine pukes...generates a 256 point line */
			wHigh = (czaPoint * 127) / 2;

			if (fElActive)
				wLow = -wHigh;
			}
		break;

	case tmcCharQpsSpacing:
		wFactor = 4;
		if (tmm == tmmParse)
			{
			if (fElActive)
				{
				wHigh = czaPoint * 56 / 4;
				wLow = -(czaPoint * 7 / 4);
				}
			else
				{
				wHigh = czaPoint * 
						(ValGetTmc(tmcCharIS) == 1 ?
						56 /* Expanded */ : 
						7 /* Compressed */) / 4;
				}
			}
		break;

	default:
		Assert (fFalse);
		}

	switch (tmm)
		{
	case tmmParse:
		dpv =  DpvParseFdxa(&za, tmc, sz, wLow, wHigh,
				dpvBlank | dpvSpaces, 
				eidDxaOutOfRange, fTrue, utPt);

		if (dpv == dpvNormal || dpv == dpvDouble)
			{
			/* Fend against round-off errors by doing a multiplication
				first. */
			SetPpvBToW(ppv, bArg, (za * wFactor) / czaPoint);
			return fTrue;
			}
		else  if (dpv == dpvError)
			/* error reporting already handled */
			/* **ppval is set to something different than the gray value
				since ValGetTmc pays no attention to the return value */
			{
			SetPpvBToW(ppv, bArg, wError);
			return fFalse;
			}
		else  /* blank or null string - treat as unchanged value */			
			{
			SetPpvBToW(ppv, bArg, wNinch);
			return fTrue;
			}
		/* NOT REACHED */
		Assert(fFalse);

	case tmmFormat:
		if ((w = WFromPpvB(ppv, bArg)) == wNinch)
			{
			*sz = 0;
			}
		else
			{
			int utT;

			za = w * czaPoint / wFactor;
			utT = vpref.ut;
			vpref.ut = utPt;
			CchExpZa(&sz, za, utPt, ichMaxNum, fTrue);
			vpref.ut = utT;
			}
		break;

#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif
		}

	return 0;
}


/*  %%Function:  WListFontName  %%Owner:  bobz       */

WORD WListFontName(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
/* If we have never gotten a printer DC, this is the time to do it */
		if (vpri.hsttbPaf == hNil)
			{
			SetFlm(flmRepaginate);
			}

		if (vpri.hsttbPaf != hNil && isz < (*vpri.hsttbPaf)->ibstMac)
			{
			int ibst = ((struct PAF *)
					PstFromSttb(vpri.hsttbPaf, isz))->ibst;

			CchCopySz(((struct FFN *) PstFromSttb(vhsttbFont, 
					ibst))->szFfn, sz);
			return fTrue;
			}

		/* break; */
		}

	return 0;
}


/*  %%Function:  WListFontSize  %%Owner:  bobz       */

WORD WListFontSize(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;  /* tmc of font combo */
{
	static int ibpaf;
	struct PAF *ppaf;
	int hps;
	CHAR rgchFfn [cbFfnLast];
	struct FFN *pffn = rgchFfn;
	int ibffn;
	int ibpafT;

	if (tmm == tmmCount)
		return -1;
	else  if (tmm != tmmText)
		return 0;

/* If we have never gotten a printer DC, this is the time to do it */
	if (vpri.hsttbPaf == hNil)
		{
		SetFlm( flmRepaginate );
		}

	if (vpri.hsttbPaf == hNil)
		{
		/* no fonts available on this printer! */
		return fFalse;
		}

/* Determine the printer font for which we are enumerating sizes 
	(first call w/ isz == 0 only) */

	if (isz == 0)
		{
/* grab text for the font from the combo box of the char dlg or ribbon */
		GetTmcText(wParam, pffn->szFfn, LF_FACESIZE);

/* fill out an ffn for the font, substituting the actual font name
	if the user has typed "default" */

		pffn->cbFfnM1 = CbFfnFromCchSzFfn(CchSz(pffn->szFfn)) - 1;

/* search for a matching font in the master font table */
		if ((ibffn = IbstFindSzFfn( vhsttbFont, pffn)) == iNil)
			return fFalse;

/* see if this ibst is in the list of fonts that are available on the printer */

		ibpaf = iNil;
		for (ibpafT = 0; ibpafT < (*vpri.hsttbPaf)->ibstMac; ibpafT++)
			{
			if (ibffn == ((struct PAF *) PstFromSttb(vpri.hsttbPaf, ibpafT))->ibst)
				{
				ibpaf = ibpafT;
				break;
				}
			}

		if (ibpaf == iNil)
			{
			/* can't enumerate sizes; the printer doesn't 
				have this font */
			return fFalse;
			}

		}

	Assert((uns)ibpaf < (*vpri.hsttbPaf)->ibstMac );

/* yield the isz'th available size as a string */

	ppaf = (struct PAF *) PstFromSttb( vpri.hsttbPaf, ibpaf );
	if (isz >= IhpsMacPpaf(ppaf))
		return fFalse;

	hps = ppaf->rghps [isz];
	CchIntToPpch( hps >> 1, &sz );
	*sz = 0;
	if (hps & 1)
		{
		extern struct ITR vitr;
		*sz++ = vitr.chDecimal;
		*sz++ = '5';
		*sz++ = '\0';
		}

	return fTrue;
}


/*  %%Function:  IbrclIbrcpFromBrcs  %%Owner:  bobz       */

IbrclIbrcpFromBrcs(pbrc, pibrcl, pibrcp, fMapLR)
WORD *pbrc, *pibrcl, *pibrcp;
BOOL fMapLR;  /* if true, map left, right borders to bar */
{
	int ibrc;
	int cNonNone = 0;
	int cfShadow = 0;
	int iNonNone = -1;
	int cSameBrc = 0;
	int brc, brcFirst;

	/* find if lines are the same, non none and shadow */
	/* only checking 1st 4 entries, then bar by itself */

	/* quickly check for ninch */
	for (ibrc = 0; ibrc <= ibrcRight; ibrc++)
		{
		brc = *(pbrc + ibrc);
		if (brc == uNinch)
			goto LUnknownBrcp;
		}

	for (ibrc = 0; ibrc <= ibrcRight; ibrc++)
		{
		brc = (*(pbrc + ibrc) & (brcLine1 | brcSpace2 | brcLine2));
		if (ibrc == 0)
			brcFirst = brc;
		else
			{
			if (brc == brcFirst)
				cSameBrc++;
			}
		if (brc != brcNone)
			{
			cNonNone++;
			iNonNone = ibrc;
			}

		if (*(pbrc + ibrc) & brcfShadow)
			cfShadow ++;
		}

	if (!cNonNone)  /* no borders */
		{
		/* check for bar border */
		brc = (*(pbrc + 5) & (brcLine1 | brcSpace2 | brcLine2));
		if (brc == brcNone)
			*pibrcp = ibrcpNone;
		else
			{
			*pibrcp = ibrcpBar;
			iNonNone = 5;
			if (*(pbrc + 5) & brcfShadow)
				cfShadow ++;
			}
		}
	else  if (cSameBrc == 3) /* all same: box */
		*pibrcp = ibrcpBox;
	else  if (cNonNone == 1)
		{
		switch (iNonNone)
			{

#ifdef DEBUG
		default:
			Assert (fFalse);
			goto LUnknownBrcp;
#endif /* DEBUG */

		case ibrcTop:
			*pibrcp = ibrcpAbove;
			break;
		case ibrcBottom:
			*pibrcp = ibrcpBelow;
			break;
		case ibrcLeft:   /* not same as bar */
		case ibrcRight:
			if (fMapLR)
				*pibrcp = ibrcpBar;
			else
				goto LUnknownBrcp;
			break;
			}
		}
	else  /* a combination we don't know about */		
		{
LUnknownBrcp:
		*pibrcp = uNinchRadio;
		*pibrcl = uNinchRadio;
		return;
		}

	/* figure out the line width */

	if (*pibrcp == ibrcpNone)
		*pibrcl = ibrclNone;
	else
		{
		brc = (*(pbrc + iNonNone) & (brcLine1 | brcSpace2 | brcLine2));
		switch (brc)
			{
		case brcSingle:
			*pibrcl = ibrclSingle;
			break;
		case brcTwoSingle:
			*pibrcl = ibrclDouble;
			break;
		case brcThick:
			*pibrcl = ibrclThick;
			break;
		default:  /* case we don't know about */
			*pibrcl = uNinchRadio;
			break;
			}
		}

	/* shadow form? */

	if (cfShadow)
		if ( (cfShadow == 4 || cfShadow == 1 ) && *pibrcl == ibrclSingle)
			*pibrcl = ibrclShadow    ;
		else  /* don't know other shadows */
			*pibrcl = uNinchRadio;


}



/* G E T  C H A R  D E F A U L T S */
/* Fills the cab in pcmb with properties from pchp and pchpGray (pchpGray
	may be NULL, in which case it is ignored). */
/*  %%Function:  GetCharDefaults  %%Owner:  bobz       */

GetCharDefaults(pcmb, pchp, pchpGray)
CMB * pcmb;
struct CHP * pchp, * pchpGray;
{
	int kul, val;
	CABCHARACTER * pcabchar;
	SPNT mpiagspnt [sizeof(mpiagspntChp) / sizeof (SPNT)];

	bltbx((CHAR FAR *)mpiagspntChp, (CHAR FAR *) mpiagspnt, 
			sizeof (mpiagspntChp));
	/*
		*  get all nongray properties indicated by mpiagsprmChp into
		*  the cab.  PropToCabGray will put wNinch in as the default
		*  gray value for non-handle items beyond those known by SDM.
		*/
	if (pchpGray == NULL)
		{
		NewPropToCab(pcmb->hcab, pchp, mpiagspnt);
		kul = pchp->kul;
		}
	else
		{
		NewPropToCabGray(pcmb->hcab, pchp, 
				pchpGray, mpiagspnt);
		kul = pchpGray->kul == 0 ? 
				pchp->kul : uNinchCheck;
		}

	/* special treatment for these values */
	/* sub/superscripts. Use value in hpsPos field for 
		the "by" val */

	pcabchar = (CABCHARACTER *) *pcmb->hcab;

	if ((val = pcabchar->wCharHpsPos) == wNinch)  /* gray? */
		{
		/* can't tell which to gray, so do both */
		pcabchar->iCharPos = uNinch;
		pcabchar->wCharHpsPos = wNinch;
		}
	else  /* not gray */		
		{
		if (val == 0)
			{
			pcabchar->iCharPos = 0;
			pcabchar->wCharHpsPos = wNinch;
			}
		else  if (val & 0x80)
			{
			/* subscript - 8 bit signed field */
			/* "absolute value" */
			pcabchar->wCharHpsPos = 256 - (val & 0xFF);
			pcabchar->iCharPos = 2;
			}
		else
			{
			pcabchar->wCharHpsPos = val & 0xFF;
			pcabchar->iCharPos = 1;
			}
		}

	if ((val = pcabchar->wCharQpsSpacing) == wNinch) /* gray? */
		{
		pcabchar->wCharQpsSpacing = wNinch;
		pcabchar->iCharIS = uNinch;
		}
	else  /* not gray */		
		{
		if (val == 0)
			{
			pcabchar->iCharIS = 0; /* IFromSign(0) */
			pcabchar->wCharQpsSpacing = wNinch;
			}
		else  if (val > 56)
			{
			/* IFromSign(val - 64) */
			pcabchar->iCharIS = 2;
			pcabchar->wCharQpsSpacing = 64 - val;
			}
		else
			{
			pcabchar->iCharIS = 1; /* IFromSign(val) */
			pcabchar->wCharQpsSpacing = val;
			}
		}

	/* Underline Codes */
	/* KLUDGE ALERT: fCharUL field of cab holds the kul! */
	if (kul == uNinchCheck)
		{
		pcabchar->fCharULine = uNinchCheck;
		pcabchar->fCharWordUL = uNinchCheck;
		pcabchar->fCharDUL = uNinchCheck;
		}
	else
		{
		pcabchar->fCharULine = fFalse;
		pcabchar->fCharWordUL = fFalse;
		pcabchar->fCharDUL = fFalse;

		switch (kul)
			{
		case kulSingle:
			pcabchar->fCharULine = fTrue;
			break;

		case kulWord:
			pcabchar->fCharWordUL = fTrue;
			break;

		case kulDouble:
			pcabchar->fCharDUL = fTrue;
			break;
			}
		}

	if (fElActive)
		{
		CABCHARACTER * pcab;

		pcab = *pcmb->hcab;
		switch (pcab->iCharIS)
			{
		case 0:
			pcab->wCharQpsSpacing = 0;
			break;

		case 2:
			pcab->wCharQpsSpacing = -pcab->wCharQpsSpacing;
			break;
			}

		switch (pcab->iCharPos)
			{
		case 0:
			pcab->wCharHpsPos = 0;
			break;

		case 2:
			pcab->wCharHpsPos = -pcab->wCharHpsPos;
			break;
			}
		}
}


/*  %%Function:  FGetParaDefaults  %%Owner:  bobz       */

FGetParaDefaults(pcmb, ppap, ppapGray, fPaps, fStyList)
CMB * pcmb;
struct PAP * ppap, * ppapGray;
BOOL fPaps;  /* if true we have a paps, not a pap */
BOOL fStyList;  /* if true we get the list of style names (now only in para dlg) */
{
	struct PRBPAP prbPap;
	SPNT mpiagspnt [sizeof (mpiagspntPap) / sizeof (SPNT)];
	CABPARALOOKS * pcabparalooks;
	extern int vdocStsh;

	bltbx((CHAR FAR *) mpiagspntPap, (CHAR FAR *) mpiagspnt,
			sizeof (mpiagspntPap));

	/* for paps leave off the brc entries */
	prbPap.cwPrb = (!fPaps ? cwPrbPap : cwPrbPaps) - 1;

	PropToPrbGray(&prbPap, ppap, ppapGray, mpiagspnt);

	/* copy entries from prb into cab.
		asserts to be sure they are in sync */

	Assert (offset(PRBPAP, Jc) - sizeof(WORD) == offset(_cabparalooks, iJustify ) - cbCabOverhead);
	Assert (offset(PRBPAP, dxaLeft) - sizeof(WORD) == offset(_cabparalooks, wParLeftIn ) - cbCabOverhead);
	Assert (offset(PRBPAP, dxaRight) - sizeof(WORD) == offset(_cabparalooks, wParRightIn ) - cbCabOverhead);
	Assert (offset(PRBPAP, dxaLeft1) - sizeof(WORD) == offset(_cabparalooks, wParFirstIn ) - cbCabOverhead);
	Assert (offset(PRBPAP, dyaBefore) - sizeof(WORD) == offset(_cabparalooks, wParDyaBefore ) - cbCabOverhead);
	Assert (offset(PRBPAP, dyaAfter) - sizeof(WORD) == offset(_cabparalooks, wParDyaAfter ) - cbCabOverhead);
	Assert (offset(PRBPAP, dyaLine) - sizeof(WORD) == offset(_cabparalooks, wParDyaLine ) - cbCabOverhead);
	Assert (offset(PRBPAP, fKeep) - sizeof(WORD) == offset(_cabparalooks, fParKeepLT ) - cbCabOverhead);
	Assert (offset(PRBPAP, fKeepFollow) - sizeof(WORD) == offset(_cabparalooks, fParKeepNP ) - cbCabOverhead);
	Assert (offset(PRBPAP, ibrcp) - sizeof(WORD) == offset(_cabparalooks, iBrcp ) - cbCabOverhead);
	Assert (offset(PRBPAP, ibrcl) - sizeof(WORD) == offset(_cabparalooks, iBrcl ) - cbCabOverhead);
	Assert (offset(PRBPAP, fPageBreakBefore) - sizeof(WORD) == offset(_cabparalooks, fParNewPg ) - cbCabOverhead);
	Assert (offset(PRBPAP, fNoLineNumb) - sizeof(WORD) == offset(_cabparalooks, fParNoLNum ) - cbCabOverhead);

	/* note: we don't copy in stc entry */
	/* convert brc array to ibrcl and ibrcp */

	if (!fPaps)
		IbrclIbrcpFromBrcs(&prbPap.brcTop, &prbPap.ibrcl,
				&prbPap.ibrcp, fFalse /* fMapLR */);
	else
		prbPap.ibrcl = prbPap.ibrcp = uNinchRadio;

#ifdef BZ
	CommSzNum(SzShared("ibrclfrom brcs = "), prbPap.ibrcl);
	CommSzNum(SzShared("ibrcpfrom brcs = "), prbPap.ibrcp);
	CommSzRgNum(SzShared("prbPap before copy to cab: "),
			&prbPap, cwPrbPap );
#endif
	/* exclude 1st entry and trailing border entries */
	/* prbpaps excludes borders, so this works for the pap ans paps cases */
	PrbToCab(&prbPap, pcmb->hcab, Iag(CABPARALOOKS, iJustify),
			cwPrbPaps - 2 /* for cwPrb  and stc */);


		{
	/* Init style vars and generate style name */
		char stStyle [cchMaxSz];

		stStyle[0] = 0;
		if (!vfDefineStyle)
			{
			SetVdocStsh(selCur.doc);
			if (fStyList)
				GenLbStyleMapping();
			if (vpapGraySelCur.stc == 0)
				{
				int stcp;
				struct STSH stsh;

				RecordStshForDocNoExcp(vdocStsh, &stsh);
				stcp = StcpFromStc(vpapSelCur.stc, stsh.cstcStd);
				GenStyleNameForStcp(stStyle, stsh.hsttbName, 
						stsh.cstcStd, stcp);
				}
			}

#ifdef DEBUG_BCH
		CommSzNum(SzShared("TmcParagraph: pcmb->hcab = "), pcmb->hcab);
#endif

		if (!FSetCabSt(pcmb->hcab, stStyle, Iag(CABPARALOOKS, hszStyle)))
			return (fFalse);
		}


	pcabparalooks = (CABPARALOOKS *) *pcmb->hcab;
	CacheSect(selCur.ca.doc, selCur.ca.cpFirst);
	if (selCur.ca.cpLim > caSect.cpLim || !vsepFetch.nLnnMod)
		pcabparalooks->fParNoLNum = fFalse;  /* will be disabled */
		/* in pap meaning is opposite from dialog */
	else  if (pcabparalooks->fParNoLNum != uNinchCheck)
		pcabparalooks->fParNoLNum = !pcabparalooks->fParNoLNum;




#ifdef BZ
	CommSzRgNum(SzShared("cabPap after copy from prb and style name: "),
			((WORD *) (*(pcmb->hcab))) + cwCabMin,
			((CABH *) *(pcmb->hcab))->cwSimple);
#endif

	return fTrue;
}


#define CbMaxSizeof(v1, v2) (sizeof (v1) > sizeof (v2) ? \
		sizeof (v1) : sizeof (v2))

/*  %%Function:  CabToPropPropGray  %%Owner:  bobz       */
/* NOTE: this stuff is currently only used by search and replace which
use a PAPS not PAP! */
CabToPropPropGray(hcab, prgbProp, prgbPropGray, sgc)
HCAB hcab;
CHAR * prgbProp, * prgbPropGray;
int sgc;
{
	int sprm, iag, iagMac, cbProp;
	SPNT mpiagspnt [CbMaxSizeof(mpiagspntChp, mpiagspntPap) / 
		sizeof (SPNT)];

	switch (sgc)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif

	case sgcPap:
		bltbx((CHAR FAR *)mpiagspntPap, (CHAR FAR *) mpiagspnt, 
				sizeof (mpiagspntPap));
		cbProp = cbPAPS;
		break;

	case sgcChp:
		bltbx((CHAR FAR *)mpiagspntChp, (CHAR FAR *) mpiagspnt, 
				sizeof (mpiagspntChp));
		cbProp = cbCHP;
		break;
		}

	SetBytes(prgbPropGray, -1, cbProp);

	iagMac = ((CABH *) *hcab)->cwSimple;
	for (iag = 0; iag < iagMac; ++iag)
		{
		if ((sprm = mpiagspnt[iag].sprm) != sprmNoop)
			{
			WORD val;

			val = *((WORD *) (*hcab) + cwCabMin + iag);

			/* Translate val if necessary... */
			switch (sprm)
				{
			case sprmPStc:
				continue;

			case sprmCKul:
					{
					CABCHARACTER * pcab;

					/* Translate underline "checkboxes" to a valid kul code.
						From a macro we may get a combination that is invalid,
						so we arbitrarily translate to something valid.
						Procedure: If any UL flags are set, we use the kul of
						the first one found.  If none are set, then if any are
						reset (false), we use kulNone.  Otherwise, use ninch.
					*/
					pcab = *hcab;
					if (pcab->fCharULine == fTrue)
						val = kulSingle;
					else  if (pcab->fCharWordUL == fTrue)
						val = kulWord;
					else  if (pcab->fCharDUL == fTrue)
						val = kulDouble;
					else  if (pcab->fCharULine == fFalse ||
								 pcab->fCharWordUL == fFalse ||
								 pcab->fCharDUL == fFalse)
						{
						val = kulNone;
						}
					else
						val = mpiagspnt[iag].wNinchVal;
					}
				break;
				}

			if (val != mpiagspnt[iag].wNinchVal)
				{
				SetPropFromSprmVal(prgbPropGray, sprm, 0);
				SetPropFromSprmVal(prgbProp, sprm, val);
				}
			}
		}
}


/* Fill listboxes with string contents stuff for cmd funcs in this module */
/* General purpose list box fill function. */
/*  %%Function:  Look1WListEntbl  %%Owner:  bobz       */

EXPORT WORD Look1WListEntbl(tmm, sz, isz, filler, tmc, iEntbl)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD iEntbl; /* wParam */
{
	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		return Look1FEnumIEntbl(iEntbl, isz, sz);
		}

	return 0;
}


csconst ENTBL rgEntbl[] = {
		{ /* character color */
		/* must correspond to ordering in props.h */
	cCharColor,
		{
		StKey("Auto",ColorAuto),
				StKey("Black",ColorBlack),
				StKey("Blue",ColorBlue),
				StKey("Cyan",ColorCyan),
				StKey("Green",ColorGreen),
				StKey("Magenta",ColorMagenta),
				StKey("Red",ColorRed),
				StKey("Yellow",ColorYellow),
				StKey("White",ColorWhite)
		}
	 },
	{ 
			cBrcp,
			/* Must correspond to ordering
		defined in dlglooks.c */
		{
		StKey("None",BrcpNone),
				StKey("Box",BrcpBox),
				StKey("Bar",BrcpBar),
				StKey("Above",BrcpAbove),
				StKey("Below",BrcpBelow)
		}
	 },
	{ 
			cBrcl,
			/* Must correspond to ordering
		defined in dlglooks.c */
		{
		StKey("None",BrclNone),
				StKey("Single",BrclSingle),
				StKey("Thick",BrclThick),
				StKey("Double",BrclDouble),
				StKey("Shadow",BrclShadow)
		}
	 },


};


/*  %%Function:  Look1CopyEntblToSz  %%Owner:  bobz       */

Look1CopyEntblToSz(iEntbl, isz, sz)
int     iEntbl;
int     isz;
CHAR    *sz;
{
	CHAR FAR *lpch;

	Assert(isz < rgEntbl[iEntbl].iMax && iEntbl < iEntblMax);

	bltbx((CHAR FAR *) (rgEntbl[iEntbl].rgst[isz]), (CHAR FAR *) sz, 
			rgEntbl[iEntbl].rgst[isz][0]+1);
	StToSzInPlace(sz);
}


/*  %%Function:  Look1FEnumIEntbl  %%Owner:  bobz       */

BOOL Look1FEnumIEntbl(iEntbl, isz, sz)
int     iEntbl;
int     isz;
CHAR    *sz;
{
	Assert(iEntbl < iEntblMax);

	if (isz < rgEntbl[iEntbl].iMax)
		{
		Look1CopyEntblToSz(iEntbl, isz, sz);
		return (fTrue);
		}
	else
		{
		return (fFalse);
		}

}


