/* F I E L D S P C . C */
/*  Functions for specific field types */


#ifdef DEBUG
#ifdef PCJ
#define SHOWSTYLEREF
/* #define DBGFIELDS */
#endif /* PCJ */
#endif /* DEBUG */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "doc.h"
#include "ch.h"
#include "screen.h"
#include "style.h"
#include "file.h"
#include "doslib.h"
#define AVGWORD
#include "inter.h"
#include "layout.h"
#include "strtbl.h"
#include "ourmath.h"
#include "debug.h"
#include "sel.h"
#include "field.h"
#include "fltexp.h"
#include "keys.h"
#include "cmdtbl.h"
#include "format.h"


#ifdef PROTOTYPE
#include "fieldspc.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */

extern int          vdocScratch;
extern int          vdocTemp;
extern struct MERR  vmerr;
extern struct CA    caPara;
extern CP           vcpFetch;
extern struct PAP   vpapFetch;
extern CP           vcpFirstLayout;
extern CP           vcpLimLayout;
extern struct CA    caSect;
extern struct SEP   vsepFetch;
extern CHAR         rgchEop[];
extern int          vwFieldCalc;
extern int          vipgd;
extern CHAR HUGE    *vhpchFetch;
extern struct PMD   *vppmd;
extern int          docGlsy;
extern struct SEL   selCur;
extern HWND         vhwndApp;
extern struct FMTSS vfmtss;

struct DTTM DttmCur ();
CP DcpCopyArgs();


/* S P E C I A L  F I E L D  H I T  C O D E */


/* C M D  D O  F I E L D  H I T */
/*  If there is a field at cpFirst which has a special hit action do
	the associated action.
*/

/* %%Function:CmdDoFieldHit %%Owner:peterj */
CMD CmdDoFieldHit (pcmb)
CMB *pcmb;
{
	int ifld = IfldSelCur ();
	struct FLCD flcd;

	if (ifld != ifldNil)
		{
		GetIfldFlcd (selCur.doc, ifld, &flcd);
		switch (flcd.flt)
			{
		case fltHyperText:
			DoHyperTextHit (selCur.doc, flcd.cpFirst);
			break;

		case fltMacroText:
			DoMacroTextHit (selCur.doc, flcd.cpFirst);
			break;

		default:
			goto LError;
			}
		return cmdOK;
		}
	else
		{
LError:
		Beep ();
		return cmdError;
		}
}



/* D O  H Y P E R  T E X T  H I T */
/* %%Function:DoHyperTextHit %%Owner:peterj */
DoHyperTextHit (doc, cp)
int doc;
CP cp;

{
	int ifld = IfldFromDocCp (doc, cp, fFalse);
	CHAR szGoto [cchBkmkMax];
	struct FFB ffb;

	Assert (ifld != ifldNil);

	if (doc != DocMother(doc))
		{
		Beep();
		return;
		}

	InitFvb (&ffb.fvb);
	InitFvbBufs (&ffb.fvb, szGoto, cchBkmkMax, NULL, 0);
	SetFfbIfld (&ffb, doc, ifld);

	SkipArgument (&ffb); /* keyword */
	FFetchArgText (&ffb, fTrue);

	FGotoSz (szGoto);
}


/* D O  M A C R O  T E X T  H I T */
/* %%Function:DoMacroTextHit %%Owner:peterj */
DoMacroTextHit (doc, cp)
int doc;
CP cp;

{
	int bcm;
	int ifld = IfldFromDocCp (doc, cp, fFalse);
	CHAR stMacro [cchMaxSz];
	struct FFB ffb;

	Assert (ifld != ifldNil);

	InitFvb (&ffb.fvb);
	InitFvbBufs (&ffb.fvb, stMacro+1, cchMaxSz-1, NULL, 0);
	SetFfbIfld (&ffb, doc, ifld);

	SkipArgument (&ffb); /* keyword */
	FFetchArgText (&ffb, fTrue);

	if (!(*stMacro = ffb.cch) || 
			WCompSt(stMacro, StSharedKey("DoFieldClick",DoFieldClick)) == 0 ||
			(bcm = BcmOfSt(stMacro)) == bcmNil || 
			(FetchCm(bcm), FElMct(vpsyFetch->mct)))
		/* macro name not valid or does not exist */
		Beep ();
	else
		{
		extern vfRecording;
		BOOL fRecordingSav ;

		InhibitRecorder(&fRecordingSav, fTrue);
		CmdExecBcmKc(bcm, kcNil);
		InhibitRecorder(&fRecordingSav, fFalse);
		}
}



/* F I E L D  C A L C U L A T I O N  F U N C T I O N S */



/* F C R  C A L C  F L T  B K M K  R E F */
/*  This is the calculation function for fltBkmkRef and fltPosBkmk
*/

/* %%Function:FcrCalcFltBkmkRef %%Owner:peterj */
FcrCalcFltBkmkRef (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int cch;
	int ich;
	int istErr;
	int docMother = DocMother (doc);
	CP cpFirst, cpLim, dcp;
	CHAR rgch [cchBkmkMax + 1];
	struct CA caField;

	/*  initialize the field fetch block for our buffers. Note: rgch+1. */
	InitFvbBufs (&pffb->fvb, rgch+1, cchBkmkMax, NULL, 0);

	/*  if field is of the form {ref bkmkname}, skip the keyword */
	if (flt == fltBkmkRef)
		SkipArgument (pffb);

	/*  fetch the bookmark name */
	FFetchArgText (pffb, fTrue /* fTruncate */);

	/*  make rgch an st */
	rgch [0] = pffb->cch;

	if (pffb->cch && FSearchBookmark(docMother, rgch, &cpFirst, 
			&cpLim, NULL, bmcUser))
		{
		struct CA caSrc, caDest;
		PcaSet( &caSrc, docMother, cpFirst, cpLim );
		AssureLegalSel ( &caSrc );
		if (FInCa (doc, cpInst, &caSrc) || FInCa (doc, cpResult, &caSrc))
			{
			istErr = istErrBkmkSelfRef;
			goto LError;
			}

		PcaField( &caField, doc, ifld );
		if (!FMoveOkForTable(doc, caField.cpLim, &caSrc))
			{
			istErr = istErrTableAction;
			goto LError;
			}
		/*  copy the bookmarks text.  use intermediate scratch doc.  remove
			bookmarks from copied text (else old bookmark will be lost!!). */
		if (DocCreateScratch (docMother) != docNil)
			{
			SetWholeDoc( vdocScratch, &caSrc );
			PcaSet ( &caSrc, vdocScratch, cp0, caSrc.cpLim - caSrc.cpFirst);
			ScratchBkmks(&caSrc);
			FReplaceCps (PcaPoint( &caDest, doc, cpResult), &caSrc );
			ReleaseDocScratch ();
			}
		return fcrNormal;
		}

	else
		{
		/*  no bookmark name specified or bookmark does not exist, place
			standard error message into result. */
		if (pffb->cch)
			istErr = istErrBkmkNotDef;
		else
			istErr = istErrNoBkmkName;
LError:
		CchInsertFieldError (doc, cpResult, istErr);
		return fcrError;
		}
}






/* F C R  C A L C  F L T  S E T */
/*  This is the calculation function for the "SET" field.
	Format: {set bkmkname bkmktext}
*/

/* %%Function:FcrCalcFltSet %%Owner:peterj */
FcrCalcFltSet (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	/* FUTURE: how to report errors?  This field has no "result" to
		place error messages in.
	*/
	CP dcp;
	struct CA ca;
	CHAR ch = chSpace;
	struct CHP chp;
	CHAR rgch [cchBkmkMax], stBkmk [cchBkmkMax];

	/*  you cannot place a bookmark in a sub document */
	if (doc != DocMother (doc))
		return fcrError;

	/*  set up ffb to fetch arguments */
	InitFvbBufs (&pffb->fvb, rgch, cchBkmkMax, NULL, 0);

	/*  fetch past the keyword */
	SkipArgument (pffb);

	/*  fetch bookmark name */
	if (!FFetchArgText (pffb, fTrue /* fTrunc */))
		return fcrError;

	/*  turn it into an st */
	SzToSt (rgch, stBkmk);

	if (!FLegalBkmkName (stBkmk))
		return fcrError;

	/* copy second argument to the result */
	dcp = DcpCopyArgument (pffb, doc, cpResult);
	if (pffb->fNoArg)
		/* there was no second argument */
		return fcrError;

	/*  we now have the cp range, assure that it is legal */
	AssureLegalSel( PcaSetDcp( &ca, doc, cpResult, dcp 	));

	/*  we have valid cp range and a valid bookmark name */
	/* if this fails, oh well, we tried! */
	FInsertStBkmk (&ca, stBkmk, NULL);

	/*  assure that insertion point bookmarks are not deleted */
	if (DcpCa(&ca) == 0)
		/* Can you say Hack? */
		{
		GetResultChp (pffb, &chp);
		if (!FInsertRgch (doc, cpResult, &ch, 1, &chp, NULL))
			return fcrError;
		}

	return fcrNormal;

}



/* F C R  C A L C  F L T  S T Y L E  R E F */
/*  Field calculation for the STYLE REFERENCE field.  (AKA headref).
*/

/* %%Function:FcrCalcFltStyleRef %%Owner:peterj */
FcrCalcFltStyleRef (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int istErr = iNil;
	int stc;
	int docMother;
	CP cpMother;
	BOOL fT;
	BOOL fLastSwitch = fFalse;
	CHAR chSw;
	struct STSH stsh;
	CHAR rgch [cchMaxStyle + 1];
	CHAR rgchResult [cchMaxSz];
	struct FVB fvbResult;
	struct CHP chp;
	struct CR rgcr [20];
	int cch;

	GetPdocPcpMotherFromDocCp (doc, cpInst, &docMother, &cpMother);

	/*  use our buffer */
	InitFvbBufs (&pffb->fvb, rgch+1, cchMaxStyle, NULL, 0);

	/*  we care about the first argument and any switches */
	/*  skip the keyword */
	SkipArgument (pffb);
	/*  get the first argument */
	if (!FFetchArgText (pffb, fTrue) || pffb->cch == 0)
		{
		istErr = istErrNoStyleName;
		goto LError;
		}

	/*  figure out what style we are looking for */
	if (rgch [1] >= '1' && rgch [1] <= '9' && pffb->cch == 1)
		/*  headref shorthand (single digit) */
		stc = stcLevLast - (rgch [1] - '1');

	else
		{
		stsh = PdodDoc (docMother)->stsh;
		rgch [0] = pffb->cch; /* make an st */
		if (!FMatchDefinedAndStdStyles (stsh.hsttbName, stsh.cstcStd,
				rgch, &stc, &fT))
			{
			istErr = istErrStyleNotDefined;
			goto LError;
			}
		}

	/*  now have stc */

	/*  check for switches in the rest of the field */
	ExhaustFieldText (pffb);
	/*  process any switches */
	while ((chSw = ChFetchSwitch (pffb, fFalse/*fSys*/)) != chNil)
		switch (chSw)
			{
		case chFldSwStyleRefLast:
			fLastSwitch = fTrue;
			break;
#ifdef DEBUG
		default:
			Assert (fFalse);
#endif /* DEBUG */
			}

	if (vcpFirstLayout != cpNil && PdodDoc (doc)->fHdr && !fLastSwitch)
		{  /* special search for header doc during printing */
#ifdef SHOWSTYLEREF
		CommSz (SzShared("StyleRef search, first in header\n\r"));
#endif /* SHOWSTYLEREF */
		if (!FFindStc (docMother, stc, fTrue, vcpFirstLayout, vcpLimLayout)
				&& !FFindStc (docMother, stc, fFalse, cp0, vcpFirstLayout)
				&& !FFindStc (docMother, stc, fTrue, vcpLimLayout, cpMax))
			{
			istErr = istErrNoStyleText;
			goto LError;
			}
		}

	else
		{  /* normal search */
#ifdef SHOWSTYLEREF
		CommSz (SzShared("StyleRef search, normal search\n\r"));
#endif /* SHOWSTYLEREF */
		if (!FFindStc (docMother, stc, fFalse, cp0, cpMother)
				&& !FFindStc (docMother, stc, fTrue, cpMother, cpMax))
			{
			istErr = istErrNoStyleText;
			goto LError;
			}
		}

	/* caPara is the paragraph we want */
	/* disallowing self-referal */
	if (DcpCa(&caPara) > 0 && !FInCa( doc, cpInst, &caPara ))
		{
		InitFvb (&fvbResult);
		InitFvbBufs (&fvbResult, rgchResult, cchMaxSz, rgcr, 20);
		fvbResult.doc = caPara.doc;
		fvbResult.cpFirst = caPara.cpFirst;
		fvbResult.cpLim = CpFirstSty(wwNil, caPara.doc, caPara.cpLim-1,
				styCRLF, fFalse);
		FetchVisibleRgch (&fvbResult, fvcResults, fFalse, fFalse);
		GetResultChp (pffb, &chp);
		cch = CchSanitizeRgch(rgchResult, fvbResult.cch, cchMaxSz, fFalse /* fSpecToCRLF */);
		cch = CchStripString(rgchResult, cch);
		if (!FInsertRgch (doc, cpResult, rgchResult, cch, &chp, NULL))
			return fcrError;
		return fcrNormal;
		}

	else
		{
LError:
		CchInsertFieldError (doc, cpResult, istErr);
		return fcrError;
		}
}



/* F  F I N D  S T C */
/*  Scan document for the next paragraph of style stc.  Scan only in the
	range cpFirst to cpLim.  Scan forward if fForward, else start at cpLim
	-1 & scan backwards.  Return fFalse if no para of that style.  Return
	fTrue if para found.  If fTrue, that para will be cached. (caPara)
*/

/* %%Function:FFindStc %%Owner:peterj */
FFindStc (doc, stc, fForward, cpFirst, cpLim)
int doc, stc;
BOOL fForward;
CP cpFirst, cpLim;

{

	CP cpCur;
	int ch;

	cpLim = CpMin (CpMacDoc (doc), cpLim);
	cpCur = CpMax (cp0, fForward ? cpFirst : cpLim - 1);
	if (!fForward)
		cpLim = cpFirst - 1;

#ifdef SHOWSTYLEREF
	CommSzNum (SzShared("from "),(int)cpCur);
	CommSzNum (SzShared("  to "),(int)cpLim);
#endif /* SHOWSTYLEREF */

	while (fForward ? cpCur < cpLim : cpCur > cpLim)
		{
		CachePara (doc, cpCur);
		if (vpapFetch.stc == stc)
			{
			if (fForward && cpLim == caPara.cpFirst+1 && 
					(ch = ChFetch(doc,caPara.cpFirst,fcmChars)) == chSect
					|| ch == chCRJ || ch == chColumnBreak)
				/* catch para on next page starting with page break */
				{
				return fFalse;
				}
#ifdef SHOWSTYLEREF
			CommSzNum (SzShared ("found "),(int)caPara.cpFirst);
#endif /* SHOWSTYLEREF */
			return fTrue;
			}
		cpCur = fForward ? caPara.cpLim : caPara.cpFirst - 1;
		}

#ifdef SHOWSTYLEREF
	CommSz(SzShared("not found\n\r"));
#endif /* SHOWSTYLEREF */
	return fFalse;

}





/* F C R  C A L C  F L T  S E Q U E N C E */
/*  format: {seq seq-id [bkmkname|\c|\r xxx]}
*/

/* %%Function:FcrCalcFltSequence %%Owner:peterj */
FcrCalcFltSequence (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	CHAR chSw;
	int docMother;
	CP cpMother, cpT;
	int w, cch, iBkmk;
	CHAR * pch;
	int fcr;
	struct CHP chp;
	CHAR rgchBkmk [cchBkmkMax + 1];
	CHAR szId [cchSequenceIdMax];
	CHAR rgchValue [cchMaxInt];

	/*  bookmark refers to mother doc.  if no bookmark use cpMother */
	GetPdocPcpMotherFromDocCp (doc, cpInst, &docMother, &cpMother);

	InitFvbBufs (&pffb->fvb, szId, cchSequenceIdMax, NULL, 0);
	SkipArgument (pffb);

	/*  get sequence id into szId */
	FFetchArgText (pffb, fTrue /* fTruncate */);

	pffb->rgch = rgchBkmk + 1;  /* room to make st */
	pffb->cchMax = cchBkmkMax;

	/*  get bookmark name */
	FFetchArgText (pffb, fTrue /* fTruncate */);

	/*  make rgch an st */
	rgchBkmk [0] = pffb->cch;

	if (!*szId || (pffb->cch && !FSearchBookmark (docMother, rgchBkmk,
			&cpMother, &cpT, NULL, bmcUser)))
		{
		/*  no sequence specified OR bookmark specified but does not exist */
		CchInsertFieldError (doc, cpResult, 
				*szId ? istErrBkmkNotDef : istErrNoSeqSpecified);
		return fcrError;
		}

	/*  get value of szId at cpMother or cp of bookmark (cache only for !bkmk)*/
	w = WSequenceValue (szId, docMother, cpMother, !pffb->cch);
	fcr = FcrRegisterIntResult (w);

	/*  make a string */
	pch = rgchValue;
	cch = CchIntToPpch (w, &pch);

	/* check for hidden */
	ExhaustFieldText (pffb);
	while ((chSw = ChFetchSwitch (pffb, fFalse/*fSys*/)) != chNil)
		if (chSw == chFldSeqHidden)
			/* don't put the result into the document */
			return fcr;

	/* get standard result chp */
	GetResultChp (pffb, &chp);
	if (!FInsertRgch (doc, cpResult, rgchValue, cch, &chp, NULL))
		return fcrError;

	return fcr;

}



struct STTB ** hsttbSequenceCalc = hNil;


struct SFC
	{  /* Sequence Field Cache */
	CHAR cch;
	CP cp;
	int w;
	CHAR sz [cchSequenceIdMax];
};


/* %%Function:WCompSfc %%Owner:peterj */
WCompSfc (sz, psfc)
CHAR * sz;
struct SFC * psfc;

{
	return WCompSz (sz, psfc->sz);
}



/* %%Function:CheckSequenceCache %%Owner:peterj */
CheckSequenceCache (sz, pcp, pw)
CHAR *sz;
CP * pcp;
int * pw;

{
	int isfc;
	struct SFC * psfc;

	if (!FSearchSttb (hsttbSequenceCalc, sz, &isfc, WCompSfc))
		{
		*pcp = (CP) -1;
		*pw = 0;
		}

	else
		{
		psfc = PstFromSttb (hsttbSequenceCalc, isfc);
		*pcp = psfc->cp;
		*pw = psfc->w;
		}

}



/* %%Function:CacheNewSequenceValue %%Owner:peterj */
CacheNewSequenceValue (sz, cp, w)
CHAR * sz;
CP cp;
int w;

{
	int cch;
	int isfc;
	struct SFC *psfc, sfc;

	if (hsttbSequenceCalc == hNil && vwFieldCalc & fclCa)
		hsttbSequenceCalc = HsttbInit(0, fFalse/*fExt*/);

	if (hsttbSequenceCalc == hNil)
		return;

	if (!FSearchSttb (hsttbSequenceCalc, sz, &isfc, WCompSfc))
		{
		sfc.cp = cp;
		sfc.w = w;
		sfc.cch = offset (SFC, sz) + (cch = CchSz (sz));
		bltb (sz, sfc.sz, cch);
		/* not a big deal if this fails */
		FInsStInSttb (hsttbSequenceCalc, isfc, &sfc);
		}

	else
		{
		psfc = PstFromSttb (hsttbSequenceCalc, isfc);
		psfc->cp = cp;
		psfc->w = w;
		}

}




/* %%Function:WSequenceValue %%Owner:peterj */
WSequenceValue (sz, doc, cp, fCacheResult)
CHAR *sz;
int doc;
CP cp;
BOOL fCacheResult;

{

	int w;

	if (*sz > '9' || *sz < '0' || *(sz + 1))
		/* standard sequence name */
		{
		CP cpCache;
		CheckSequenceCache (sz, &cpCache, &w);

		if (cpCache != cp)
			{
			if (cpCache < cp)
				w = WCalcSequenceValue (sz, doc, cpCache + 1, w, cp);

			else
				w = WCalcSequenceValue (sz, doc, cp0, 0, cp);

			if (fCacheResult)
				CacheNewSequenceValue (sz, cp, w);
			}
		}
	else
		/* outline level sequence */
		{
		int rgw [10];
		if (!FGetLevelSequences (doc, cp, rgw))
			w = 0;
		else  if (*sz != '0')
			w = rgw [*sz - '1'];
		else
			w = rgw [9];
		}

	return w;

}




/*  Apply fields from cpFirst to cpLast to sequence sz which starts at
	value w.
*/
/* %%Function:WCalcSequenceValue %%Owner:peterj */
WCalcSequenceValue (sz, doc, cpFirst, w, cpLast)
CHAR *sz;
int doc, w;
CP cpFirst, cpLast;

{
	int ifld = IfldNextField (doc, cpFirst);
	CHAR rgch [cchSequenceIdMax];
	struct FLCD flcd;

	GetIfldFlcd (doc, ifld, &flcd);
	while (ifld != ifldNil && flcd.cpFirst <= cpLast)
		{
		if (flcd.flt == fltSequence
				&& FGetFieldSequenceSz (doc, ifld, rgch)
				&& FEqNcSz (sz, rgch))
			/* this is a field for this sequence, apply it */
			w = WApplyFieldToSequence (doc, ifld, w);

		ifld = IfldAfterFlcd (doc, ifld, &flcd);
		}

	return w;

}




/* %%Function:FGetFieldSequenceSz %%Owner:peterj */
FGetFieldSequenceSz (doc, ifld, rgch)
int doc, ifld;
CHAR * rgch;

{
	struct FFB ffb;

	InitFvb (&ffb.fvb);
	InitFvbBufs (&ffb.fvb, rgch, cchSequenceIdMax, NULL, 0);
	SetFfbIfld (&ffb, doc, ifld);

	SkipArgument (&ffb);

	return (FFetchArgText (&ffb, fTrue) && ffb.cch > 0);

}




/* %%Function:WApplyFieldToSequence %%Owner:peterj */
WApplyFieldToSequence (doc, ifld, w)
int doc, ifld, w;

{
	int ich;
	int ch;
	CHAR rgch [cchMaxSz];
	struct FFB ffb;

	InitFvb (&ffb.fvb);
	InitFvbBufs (&ffb.fvb, rgch, cchMaxSz, NULL, 0);
	SetFfbIfld (&ffb, doc, ifld);

	SkipArgument (&ffb);
	SkipArgument (&ffb);

	/*  see if there was a bookmark, if so no effect */
	FFetchArgText (&ffb, fTrue);
	if (ffb.cch != 0)
		return w;

	ExhaustFieldText (&ffb);
	while ((ch = ChFetchSwitch (&ffb, fFalse)) == chFldSeqHidden)
		;

	switch (ch)
		{
	case chFldSeqReset:
		FFetchArgText (&ffb, fTrue);
		w = 0;
		ich = 0;
		while (ich < ffb.cch && !FDigit (rgch [ich]))
			ich++;
		while (ich < ffb.cch && FDigit (rgch [ich]))
			w = (w * 10) + (rgch [ich++] - '0');
		break;

	case chNil:
	case chFldSeqNext:
		w++;
		break;

	case chFldSeqCurrent:
		break;

#ifdef DEBUG
	default:
		Assert (fFalse);
#endif /* DEBUG */
		}

	return w;

}





/* F C R  C A L C  F L T  Q U O T E */
/*  Quote arguments to result
	Format:  {quote arguments}
	arguments are:
		ddd     ddd is decimal number 0-255; copy chr$(ddd) to result
		0xhh    hh is hex number 00-ff; copy chr$(hh) to result
		else copy argument to result
*/

/* %%Function:FcrCalcFltQuote %%Owner:peterj */
FcrCalcFltQuote (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	CP dcp;
	struct CA caScratch, caT;

	if (DocCreateScratch (doc) == docNil)
		/* could not create vdocScratch, bail out */
		return fcrKeepOld; /* old result preserved */

	SkipArgument (pffb); /* skip keyword */

	PcaPoint( &caScratch, vdocScratch, cp0 );
	caScratch.cpLim = dcp = DcpCopyArgs( pffb, caScratch.doc, NULL);
	PcaField( &caT, doc, ifld );
	if (!FMoveOkForTable(doc, caT.cpLim, &caScratch))
		dcp = -istErrTableAction;
	if (dcp > cp0  )
		/* copy from vdocScratch to the result */
		FReplaceCps (PcaPoint( &caT, doc, cpResult), &caScratch);

	else  if (dcp < cp0 && !vmerr.fMemFail)
		/*  field argument invalid (number not well formed, result
			was CR or LF) place standard error message in result. 
			(error number is in -dcpSrc). */
		CchInsertFieldError (doc, cpResult, (int) -dcp);

	ReleaseDocScratch ();
	return dcp < cp0 ? fcrError : fcrNormal;
}


/* D C P  C O P Y  A R G S */
/*  Copy the arguments of a field to doc.  Return number of cps copied
	or -istErr if error encountered.
*/

/* %%Function:DcpCopyArgs %%Owner:peterj */
CP DcpCopyArgs (pffb, doc, pwCh)
struct FFB *pffb;
int doc;
int *pwCh;
{
	CP cpDest = cp0;
	CHAR *pch, *pchMac;
	CHAR chInsert;
	BOOL fContinued;
	int wT;
	CHAR rgch [cchArgumentFetch];
	struct CR rgcr [ccrArgumentFetch];

	InitFvbBufs (&pffb->fvb, rgch, cchArgumentFetch, rgcr, ccrArgumentFetch);
	Assert (doc != pffb->doc);

	if (pwCh != NULL)
		*pwCh = chNil;
	for (;;)
		{
		fContinued = pffb->fOverflow;
		FetchFromField (pffb, fTrue/*fArg*/, fFalse /* fRTF */);
		if (pffb->fNoArg)
			/* end of instructions */
			return cpDest;
		if (pffb->cch == 0)
			/*  empty argument ("") */
			continue;

		if (FDigit (rgch [0]) && !pffb->fGrouped && !fContinued)
			/*  non-literal text option */
			{
			chInsert = 0;

			if (rgch [0] == '0' && pffb->cch >= 3 && 
					ChLower (rgch [1]) == 'x')
				/* hex format: 0xhh */
				for (pch=rgch+2, pchMac = pch+min(2, pffb->cch); 
						pch < pchMac; pch++)
					{
					if (FHexCh (*pch, &wT))
						chInsert = (chInsert * 16) + wT;
					else
						/*  error, invalid hex digit */
						return (CP) -istErrInvalidHexDigit;
					}

			else
				/* decimal format: ddd */
				for (pch=rgch, pchMac = pch+min (3, pffb->cch); 
						pch < pchMac; pch++)
					if (FDigit (*pch))
						chInsert = (chInsert * 10) + *pch - '0';
					else
						/* error, not a digit */
						return (CP) -istErrInvalidDigit;

			if (chInsert != chReturn
#ifdef CRLF
					&& chInsert != chEop && chInsert != chTable
#endif /* CRLF */
					)
				/* legal insertion character */
				{
				struct CHP chp;
				/* get insertion chp */
				GetResultChp (pffb, &chp);
				if (FInsertRgch (doc, cpDest, &chInsert, 1, &chp, NULL))
					cpDest++;
				}
			else
				{
				/* cannot insert CR or LF... */
				if (pwCh == NULL)
					return (CP) -istErrNoInsertCRLF;

				/* ...unless the caller will deal with it */
				*pwCh = chInsert;
				return cpDest;
				}
			}

		else
			{
			cpDest += DcpCopyIchCchPcr (doc, cpDest, pffb->doc, rgcr,
					pffb->ccr, 0, -1);

			if (vmerr.fMemFail)
				/* memory problems, abort */
				return cpNil;
			}

		} /* for (;;) */
}



/* F  H E X  C H */
/*  Return true if ch is a hex char.  Return ch's value in *pw.
*/

/* %%Function:FHexCh %%Owner:peterj */
FHexCh (ch, pw)
CHAR ch;
int *pw;

{
	if (FDigit (ch))
		*pw = ch - '0';
	else  if ((ch=ChLower(ch)) >= 'a' && ch <= 'f')
		*pw = ch - 'a' + 10;
	else
		return fFalse;

	return fTrue;
}


/* F C R  C A L C  F L T  I N C L U D E */
/*  Insert a file.
	Format {include file-name [bookmark-name]}
*/

/* %%Function:FcrCalcFltInclude %%Owner:peterj */
FcrCalcFltInclude (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int docSrc, fn;
	int istErr;
	CHAR chSw;
	int fNewDocSrc = fTrue;
	struct CA ca, caT;
	CHAR stFile [ichMaxFile+1];
	CHAR stNorm [ichMaxFile];
	CHAR stSubset [cchMaxSz];
	CHAR stConverter [ichMaxFile+1];

	/*  skip keyword */
	SkipArgument (pffb);

	InitFvbBufs (&pffb->fvb, stFile+1, ichMaxFile, NULL, 0);
	/*  get filename */
	if (!FFetchArgText (pffb, fTrue/* fTruncate */) || !pffb->cch)
		{
		istErr = istErrNoFileName;
		goto LError;
		}

	/* make an st */
	stFile [0] = pffb->cch;

	InitFvbBufs (&pffb->fvb, stSubset+1, cchMaxSz-1, NULL, 0);
	/* subset specification */
	if (FFetchArgText (pffb, fTrue/* fTruncate */))
	/* make st */
		stSubset [0] = pffb->cch;

	/*  check for converter specification */
	stConverter[0] = 0;
	ExhaustFieldText (pffb);
	while ((chSw = ChFetchSwitch (pffb, fFalse)) != chNil)
		switch (chSw)
			{
		case chFldSwConverter:
			InitFvbBufs (&pffb->fvb, stConverter+1, ichMaxFile, NULL, 0);
			if (!FFetchArgText (pffb, fTrue))
				{
				istErr = istErrNoSwitchArgument;
				goto LError;
				}
			stConverter [0] = pffb->cch;
			break;
			}

	if (!FNormalizeStFile (stFile, stNorm, nfoNormal))
		{
LOpenError:
		istErr = istErrCannotOpenFile;
LError:
		CchInsertFieldError (doc, cpResult, istErr);
		return fcrError;
		}

	/* see if file is already opened and we can use it */
	if ((docSrc = DocFromSt(stNorm)) != docNil &&
			((fn = PdodDoc(docSrc)->fn) == fnNil || !PfcbFn(fn)->fForeignFormat))
		{
		fNewDocSrc = fFalse;
		if (!*stSubset || !FSearchBookmark (ca.doc = docSrc, stSubset,
				&ca.cpFirst, &ca.cpLim, NULL, bmcUser))
			{
			goto LWholeDoc;
			}
		ca.cpLim = CpMin (ca.cpLim, CpMacDocEdit(docSrc));
		}

	/*  try to open the file using converter & subset */
	else  if ((fn = FnOpenSt(stNorm, 0, ofcDoc, NULL)) == fnNil ||
			(docSrc = DocCreateFn (fn, fFalse, stConverter, stSubset, NULL))
			== docNil)
		/*  open failed */
		goto LOpenError;

	else  if (docSrc == docCancel)
		return fcrKeepOld;

	else
		/*  open succeeded */
		{
LWholeDoc: 
		PcaSet( &ca, docSrc, cp0, CpMacDocEdit(docSrc));
		}

	/* no self reference allowed */
	if (doc == docSrc)
		{
		istErr = istErrNoSelfRefInclude;
		goto LError;
		}

	/* we now have the doc and the limits of interest */
	AssureLegalSel (&ca);
	PcaField( &caT, doc, ifld );
	if (!FMoveOkForTable(doc, caT.cpLim, &ca))
		{
		istErr = istErrTableAction;
		goto LError;
		}
	if (FReplaceCps (PcaPoint( &caT, doc, cpResult), &ca))
		CopyStylesFonts(docSrc, doc, cpResult, DcpCa(&ca));
	if (fNewDocSrc)
		{
		DisposeDoc (docSrc);
		KillExtraFns();
		}

	return fcrNormal;
}



/* F C R  C A L C  F L T  A S K  */
/*  Prompt user for text
	Format {ask [\o][\n] bookmark [prompt] [\d default]}
			{fillin [\o][\n] [prompt] [\d default]}
*/

/* %%Function:FcrCalcFltAskFill %%Owner:peterj */
FcrCalcFltAskFill (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int cch;
	struct CA ca;
	CHAR chSw;
	BOOL fGetDefault = fTrue;
	CHAR szPrompt [cchMaxSz];
	CHAR szInput [cchMaxSz];
	CHAR stBkmk [cchBkmkMax+1];
	struct CHP chp;
	struct FLCD flcd;

	/* skip keyword */
	SkipArgument (pffb);

	/* get bookmark name */
	if (flt == fltAsk)
		{
		if (PdodDoc(doc)->fShort)
		/* cannot put a bookmark in a subdoc */
		/* Cannot report the error */
			return fcrError;

		InitFvbBufs (&pffb->fvb, stBkmk+1, cchBkmkMax, NULL, 0);
		if (!FFetchArgText (pffb, fTrue/*fTruncate*/))
			{
		/* Cannot report the error */
			return fcrError;
			}
		/*  make an st */
		stBkmk [0] = pffb->cch;
		if (!FLegalBkmkName (stBkmk))
			{
		/* Cannot report the error */
			return fcrError;
			}
		}

	/* get prompt */
	InitFvbBufs (&pffb->fvb, szPrompt, cchMaxSz, NULL, 0);
	if (!FFetchArgText (pffb, fTrue/*fTruncate*/))
		if (flt == fltAsk)
			StToSz (stBkmk, szPrompt);
		else
			szPrompt [0] = 0;

	szInput [0] = 0;

	/* check for switches */
	ExhaustFieldText (pffb);
	while ((chSw = ChFetchSwitch (pffb, fFalse/*fSys*/)) != chNil)
		if (chSw == chFldSwAskOnce && vppmd != NULL && !vppmd->fFirstPass)
			return fcrKeepOld;
		else  if (chSw == chFldSwDefault)
			{
			fGetDefault = fFalse;
			InitFvbBufs (&pffb->fvb, szInput, cchMaxSz, NULL, 0);
			if (!FFetchArgText (pffb, fTrue))
				{
				if (flt != fltAsk)
				/* Cannot report the error for ASK field */
					CchInsertFieldError (doc, cpResult, istErrNoSwitchArgument);
				return fcrError;
				}
			}

	/*  get default input */
	GetIfldFlcd (doc, ifld, &flcd);
	if (fGetDefault && flcd.dcpResult != cp0)
		{
		struct CR rgcr [5];
		struct FVB fvb;
		InitFvb (&fvb);
		InitFvbBufs (&fvb, szInput, cchMaxSz-1, rgcr, 5);
		fvb.doc = doc;
		fvb.cpFirst = flcd.cpFirst+flcd.dcpInst;
		fvb.cpLim = flcd.cpFirst+flcd.dcpInst+flcd.dcpResult-1;
		FetchVisibleRgch (&fvb, 
				(pffb->fFetchVanished?fvcHidResults:fvcResults),
				fFalse, fTrue);
		szInput [fvb.cch] = 0;
		}

	/*  get the user's input */
	if (!FPromptDialog (szPrompt, szInput, cchMaxSz))
		return fcrKeepOld;

	if ((cch = CchSz (szInput) - 1) > 0)
		{
		GetResultChp (pffb, &chp);
		InsertMultiLineRgch (doc, cpResult, szInput, cch, &chp, NULL);
		if (flt == fltAsk)
			{ /* set a bookmark on the data */
			AssureLegalSel(PcaSetDcp( &ca, doc, cpResult, (CP)cch));
			/* if this fails, oh well, we tried */
			FInsertStBkmk (&ca, stBkmk, NULL);
			}
		}

	return fcrNormal;
}




/* F C R  C A L C  F L T  I F */
/*  Evaluate a condition.  If true result is true-text, if false result is
	false-text.
	Format:  {if cond-exp true-text false-text}
*/

/* %%Function:FcrCalcFltIf %%Owner:peterj */
FcrCalcFltIf (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	BOOL fCondition;
	int istErr;

	SkipArgument (pffb);  /* skip the keyword */

	/*  evaluate the condition */
	if (!FEvalFieldCondition (pffb, &istErr, &fCondition))
		/* evaluation failed, condition not well formed */
		{
		CchInsertFieldError (doc, cpResult, istErr);
		return fcrError;
		}

	/* FEval... fetches all of the condition text.  pffb is now set up
		to fetch true-text and false-text.  Note one or both may be missing
		and the field is still considered well formed.
	*/

	if (!fCondition)
		/* if false, skip the true text. */
		SkipArgument (pffb);

	/* copy the next argument to the result */
	DcpCopyArgument (pffb, doc, cpResult);

	return fcrNormal;
}




/* F  E V A L  F I E L D  C O N D I T I O N */
/*  Evaluate a conditional expression.  A conditional expression has
	the form:  exp1 opcode exp2.  pffb is assumed to be set up to fetch from
	the beginning of the expression (not beginning of field).  On return,
	if successful, pffb will be ready to fetch any arguments beyond exp2.
	Returns true if cond-exp could be evaluated and the value is in
	*pfCondition.  Returns false if the expression was not well formed and a
	message number is in *pistErr.
*/

/* %%Function:FEvalFieldCondition %%Owner:peterj */
FEvalFieldCondition (pffb, pistErr, pfCondition)
struct FFB *pffb;
int *pistErr;
BOOL *pfCondition;

{
	int wCompare;
	int coc;
	CP cpFirstExp1, cpLimExp1, cpFirstExp2, cpLimExp2;
	int edtExp1, edtExp2;
	int docExp1, docExp2;


		{
		CP cpExp;
		CHAR szOpCode [5];

	/* get exp1 */
		cpExp = pffb->cpFirst;
		if (!FFetchArgExtents (pffb, &cpFirstExp1, &cpLimExp1, fFalse))
			{
			*pistErr = istErrMissingCondExp1;
			return fFalse;
			}
		edtExp1 = EdtExpFromPffbCps (pffb, cpExp, &docExp1, &cpFirstExp1,
				&cpLimExp1);

	/* get op code */
		InitFvbBufs (&pffb->fvb, szOpCode, 5, NULL, 0);
		if (!FFetchArgText (pffb, fTrue/*fTRuncate*/))
			{
			*pistErr = istErrMissingCondOpCode;
			return fFalse;
			}
		if ((coc = IdFromSzgSz (szgCondOpCodes, szOpCode)) == idNil)
			{
			*pistErr = istErrUnknownCondOpCode;
			return fFalse;
			}

	/* get exp2 */
		cpExp = pffb->cpFirst;
		if (!FFetchArgExtents (pffb, &cpFirstExp2, &cpLimExp2, fFalse))
			{
			*pistErr = istErrMissingCondExp2;
			return fFalse;
			}
		edtExp2 = EdtExpFromPffbCps (pffb, cpExp, &docExp2, &cpFirstExp2,
				&cpLimExp2);
		}

#ifdef DBGFIELDS
	CommSzNum (SzShared("\n\redtExp1 = "), edtExp1);
	CommSzNum (SzShared("edtExp2 = "), edtExp2);
#endif /* DBGFIELDS */

	/* decide what kind of comparison to do */
	switch (edtExp1)
		{
	case edtNumeric:
		goto LNumericCompare;
	case edtString:
	case edtStringField:
		goto LStringCompare;
	default:
		switch (edtExp2)
			{
		case edtNumeric:
			goto LNumericCompare;
		case edtString:
		case edtStringField:
			goto LStringCompare;
		default:
			if (edtExp1 == edtNumericBkmk && edtExp2 ==
					edtNumericBkmk)
				goto LNumericCompare;
			else
				goto LStringCompare;
			}
		}

LNumericCompare:
		{
		NUM n1, n2;
		PES	pes;

		InitPpes(&pes);
		if (!FNumFromCps (docExp1, cpFirstExp1, cpLimExp1, &n1, &pes,
				pffb->fFetchVanished, NULL, NULL))
			goto LStringCompare;
		InitPpes(&pes);
		if (!FNumFromCps (docExp2, cpFirstExp2, cpLimExp2, &n2, &pes,
				pffb->fFetchVanished, NULL, NULL))
			goto LStringCompare;


		wCompare = WApproxCmpPnum(&n1, &n2);

		goto LCompareDone;
		}


LStringCompare:
		{
		BOOL fObserveWild = ((coc == cocEQ || coc == cocNE) && 
				(edtExp2 == edtString || edtExp2 == edtText));
		CHAR *pch1=NULL, *pchMac1=NULL, *pch2=NULL, *pchMac2=NULL;
		struct FVB fvb1;
		struct FVB fvb2;
		CHAR rgch1 [cchArgumentFetch+1];
		CHAR rgch2 [cchArgumentFetch+1];
		struct CR rgcr1 [ccrArgumentFetch];
		struct CR rgcr2 [ccrArgumentFetch];

		InitFvbBufs (&fvb1, rgch1, cchArgumentFetch, rgcr1, ccrArgumentFetch);
		fvb1.doc = docExp1;
		fvb1.cpFirst = cpFirstExp1;
		fvb1.cpLim = cpLimExp1;

		InitFvbBufs (&fvb2, rgch2, cchArgumentFetch, rgcr2, ccrArgumentFetch);
		fvb2.doc = docExp2;
		fvb2.cpFirst = cpFirstExp2;
		fvb2.cpLim = cpLimExp2;

		for (;;)
			{
			if (pch1 >= pchMac1)
				{
				FetchVisibleRgch (&fvb1, 
						(pffb->fFetchVanished?fvcHidResults:fvcResults),
						fFalse, fTrue);
				pch1 = rgch1;
				pchMac1 = pch1 + fvb1.cch;
#ifdef DBGFIELDS
				*pchMac1 = 0;
				CommSz (SzShared("rgch1: "));
				CommSz (pch1);
				CommSz (SzShared("\n\r"));
#endif /* DBGFIELDS */
				}

			if (pch2 >= pchMac2)
				{
				FetchVisibleRgch (&fvb2, 
						(pffb->fFetchVanished?fvcHidResults:fvcResults),
						fFalse, fTrue);
				pch2 = rgch2;
				pchMac2 = pch2 + fvb2.cch;
#ifdef DBGFIELDS
				*pchMac2 = 0;
				CommSz (SzShared("rgch2: "));
				CommSz (pch2);
				CommSz (SzShared("\n\r"));
#endif /* DBGFIELDS */
				}

			if (fObserveWild && pch2 < pchMac2 && *pch2 == chCondMatchMany)
				{  /* match any number of chars wildcard */

			/* I am punting and only allowing the length of exp1
				remaining to be matched to be cchArgumentFetch (128) or less.
			*/

				FillFvbBuffer (&fvb2, pch2-rgch2, pffb->fFetchVanished);
				if (fvb2.cch == 1)
					{
					wCompare = 0;
					goto LCompareDone;
					}
				if (pch1 == pchMac1)
					{
					wCompare = -1;
					goto LCompareDone;
					}
				FillFvbBuffer (&fvb1, pch1-rgch1, pffb->fFetchVanished);
				if (fvb1.fOverflow)
					{
					*pistErr = istErrWildMatchTooLong;
					return fFalse;
					}
				if (fvb2.fOverflow)
					{
					wCompare = -1;
					goto LCompareDone;
					}
				pch1 = rgch1 + fvb1.cch-1;
				pch2 = rgch2 + fvb2.cch-1;
				while (pch1 >= rgch1 && pch2 > rgch2)
					{
					if (*pch2 != chCondMatchOne)
						if (wCompare = WCompChCh (*pch1, *pch2))
							goto LCompareDone;
					pch1--;
					pch2--;
					}
				wCompare = 0;
				goto LCompareDone;
				}

			if (pch1 == pchMac1)
				/* no more Exp1 */
				{
				wCompare = (pch2 == pchMac2) ? 0 : -1;
				goto LCompareDone;
				}

			if (pch2 == pchMac2)
				/* no more Exp2 */
				{
				wCompare = 1;
				goto LCompareDone;
				}

			if (fObserveWild && *pch2 == chCondMatchOne)
				{  /* single char wildcard -- say it matches */
				}
			else  if (wCompare = WCompChCh (*pch1, *pch2))
				goto LCompareDone;

			pch1++;
			pch2++;
			}
		}


LCompareDone:

#ifdef DBGFIELDS
	CommSzNum (SzShared("LCompareDone: coc = "), coc);
	CommSzNum (SzShared("wCompare = "), wCompare);
#endif /* DBGFIELDS */

	switch (coc)
		{
	case cocEQ:
		*pfCondition = wCompare == 0;
		break;
	case cocGE:
		*pfCondition = wCompare >= 0;
		break;
	case cocGT:
		*pfCondition = wCompare > 0;
		break;
	case cocLE:
		*pfCondition = wCompare <= 0;
		break;
	case cocLT:
		*pfCondition = wCompare < 0;
		break;
	case cocNE:
		*pfCondition = wCompare != 0;
		break;
		}

	return fTrue;

}  /* FEvalFieldCondition */




/* E D T  E X P  F R O M  P F F B  C P S */
/*  Determine the data type of an argument.  If the argument is a
	bookmark name, resolve the bookmark and determine its type too.
*/

/* %%Function:EdtExpFromPffbCps %%Owner:peterj */
EdtExpFromPffbCps (pffb, cpExp, pdoc, pcpFirst, pcpLim)
struct FFB *pffb;
CP cpExp;
int *pdoc;
CP *pcpFirst, *pcpLim;

{
	CHAR *pch, *pchMac;
	CP cpFirstBkmk, cpLimBkmk;
	int ccp, docMother;
	struct FFB ffb;
	CHAR rgch [cchArgumentFetch];

	*pdoc = pffb->doc;
	ffb.fsfb = pffb->fsfb;
	ffb.doc = pffb->doc;
	ffb.cpFirst = cpExp;
	ffb.cpLim = *pcpLim;
	ffb.fOverflow = fFalse;

	InitFvbBufs (&ffb.fvb, rgch, cchArgumentFetch, NULL, 0);
	FetchFromField (&ffb, fFalse/*fArg*/, fFalse /* fRTF */);

	pch = rgch;
	pchMac = pch + ffb.cch;
	while (pch < pchMac && FWhite(*pch))
		pch++;
	if (pch == pchMac)
		return edtText;
	else  if (*pch == chGroupExternal)
		return ffb.fNested ? edtStringField : edtString;
	else  if (FDigit (*pch))
		return edtNumeric;

	/* check for bookmark */
	bltb (pch, rgch+1, pchMac-pch);
	pchMac -= pch - (rgch + 1);
	pch = rgch + 1;
	while (pch < pchMac && !FWhite (*pch))
		pch++;
	rgch [0] = pch - (rgch + 1);

	/* now have st, search for it */
	if (FSearchBookmark ((docMother = DocMother(pffb->doc)), rgch,
			&cpFirstBkmk, &cpLimBkmk, NULL, bmcUser))
		{
		*pcpFirst = cpFirstBkmk;
		*pcpLim = cpLimBkmk;
		*pdoc = docMother;

		/* check type at bookmark */
		FetchCpPccpVisible (docMother, cpFirstBkmk, &ccp,
				(pffb->fFetchVanished?fvcHidResults:fvcResults),
				fTrue);
		return (ccp && vcpFetch < cpLimBkmk && FDigit (*vhpchFetch)) ? 
				edtNumericBkmk : edtTextBkmk;
		}

	return edtText;

}





/* F I L L  F V B  B U F F E R */
/*  Fill the rgch buffer in pfvb or fetch to cpLim.  Start filling with the
	cp at ich.
*/

/* %%Function:FillFvbBuffer %%Owner:peterj */
FillFvbBuffer (pfvb, ich, fVanished)
struct FVB *pfvb;
int ich;
BOOL fVanished;

{
	CHAR *rgchOld = pfvb->rgch;
	int cchMaxOld = pfvb->cchMax;

	pfvb->cpFirst = CpFromIchPcr (ich, pfvb->rgcr, pfvb->ccr);

	do
		{
		FetchVisibleRgch (pfvb, 
				(fVanished?fvcHidResults:fvcResults),
				fFalse, fTrue);
		pfvb->cchMax -= pfvb->cch;
		pfvb->rgch += pfvb->cch;
		}
	while (pfvb->cchMax && pfvb->fOverflow);

	pfvb->cch = pfvb->rgch - rgchOld;
	pfvb->cchMax = cchMaxOld;
	pfvb->rgch = rgchOld;

	Assert (pfvb->cch == pfvb->cchMax || !pfvb->fOverflow);
}



/* F C R  C A L C  F L T  G L S Y */
/*  Expand the indicated glossary
	Format:  {glossary glossary-name}
*/

/* %%Function:FcrCalcFltGlsy %%Owner:peterj */
FcrCalcFltGlsy (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int docDot = DocDotMother (doc);
	int docGlsyLocal;
	int docSrc;
	int iglsy;
	int istErr;
	struct CA ca, caT;
	CHAR st [cchGlsyMax + 1];

	if (!FEnsureGlsyInited())
		return fcrError;

	docGlsyLocal = docDot == docNil ? docNil : PdodDoc (docDot)->docGlsy;

	SkipArgument (pffb);
	InitFvbBufs (&pffb->fvb, st+1, cchGlsyMax, NULL, 0);
	FFetchArgText (pffb, fTrue/*fTruncate*/);

	*st = pffb->cch;

	if (*st && docGlsyLocal != docNil &&
			FSearchGlsy (docGlsyLocal, st, &iglsy))
		/*  found in template's glossary */
		docSrc = docGlsyLocal;
	else  if (*st && docGlsy != docNil &&
			FSearchGlsy (docGlsy, st, &iglsy))
		/*  found in global glossary */
		docSrc = docGlsy;

	else
		/*  no glossary specified or not defined */
		{
		istErr = *st ? istErrGlsyNotDefined : istErrNoGlsyName;
LError:
		CchInsertFieldError (doc, cpResult, istErr);
		return fcrError;
		}

	CaFromIhdd (docSrc, iglsy, &ca);
	Assert (ca.doc == docSrc);
	PcaField( &caT, doc, ifld );
	if (!FMoveOkForTable(doc, caT.cpLim, &ca))
		{
		istErr = istErrTableAction;
		goto LError;
		}
	if (FReplaceCps (PcaPoint( &caT, doc, cpResult), &ca))
		CopyStylesFonts(docSrc, caT.doc, caT.cpFirst, DcpCa(&ca));

	return fcrNormal;
}




/*  C C H  S A N I T I Z E  R G C H  */
/*  Convert all non-printable characters to chSpace, except for ones
	that we know how to do something intelligent with.  Returns the
	cch of the resulting rgch (since it can grow or shrink).  Won't
	go beyond cchMax.  This function handles the chVis characters
	because it can be called after FormatLine has already hacked the
	characters.
*/

/* %%Function:CchSanitizeRgch %%Owner:peterj */
int CchSanitizeRgch(rgch, cch, cchMax , fSpecToCRLF)
CHAR *rgch;
int cch, cchMax;
BOOL fSpecToCRLF; /* turn pg break, col break, new line to CRLF pair */
{
	CHAR *pch, *pchLim;

	Assert (cchMax >= cch);

	pchLim = rgch + cch;
	for (pch = rgch; pch < pchLim; pch++)
		{
		switch (*pch)
			{
		default:
			if ((*pch & 0x7f) < 0x20) /* non-printing chars */
				*pch = chSpace;
			break;

		case chPubBullet:
			*pch = 'o';
			break;

		case chPubLDblQuote:
		case chPubRDblQuote:
			*pch = '\"';
			break;

		case chTab:
			break;

		case chNonBreakSpace:
		case chVisNonBreakSpace:
		case chVisSpace:
			*pch = chSpace;
			break;

		case chPubEmDash:
		case chPubEnDash:
		case chNonBreakHyphen:
			*pch = chHyphen;
			break;

		case chNonReqHyphen:
		case chVisNonReqHyphen:  /* == case chVisCRJ: */
		case chVisEop:
LDeleteCh:
			bltb(pch + 1, pch, pchLim - pch - 1);
			pch--;
			pchLim--;
			break;

		case chTable:
			if (pch == rgch || *(pch - 1) != chReturn)
				goto LDeleteCh;
			*pch = chTab;
			break;

		case chEol:
			if (pch == rgch || *(pch - 1) != chReturn)
				goto LDeleteCh;
			break;

		case chReturn:
			if (pch == pchLim - 1 ||
					*(pch + 1) != chTable && *(pch + 1) != chEol)
				goto LDeleteCh;
			break;

		case chSect:
		case chColumnBreak:
		case chCRJ:
			if (fSpecToCRLF && pch != pchLim - 1 && pchLim < rgch+cchMax)
				{
				bltb(pch, pch + 1, pchLim - pch);
				pchLim++;
				*pch++ = chReturn;
				*pch = chEol;
				}
			else
				*pch = chSpace;
			break;
			}
		}

	return (pchLim - rgch);
}



/* F C R  C A L C  F L T  F T N  R E F */
/*  Return the auto numbered footnote value at bookmark.
	Format:  {ftnref bookmark}
*/

/* %%Function:FcrCalcFltFtnRef %%Owner:peterj */
FcrCalcFltFtnRef (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int n;
	int docMother = DocMother (doc);
	CP cpFirst, cpLim;
	struct CHP chp;
	CHAR rgch [cchBkmkMax+1];

	InitFvbBufs (&pffb->fvb, rgch+1, cchBkmkMax, NULL, 0);

	/* skip keyword */
	SkipArgument (pffb);

	/* fetch bookmark name */
	FFetchArgText (pffb, fTrue/*fTruncate*/);
	rgch [0] = pffb->cch;

	if (pffb->cch && FSearchBookmark (docMother, rgch, &cpFirst, 
			&cpLim, NULL, bmcUser))
		{
		/* we have a valid bookmark */
		GetResultChp (pffb, &chp);
		n = NAutoFtn(docMother, cpFirst);
		CchInsertLongNfc(doc, cpResult, (LONG)n, nfcArabic, &chp);
		return (FcrRegisterIntResult (n));
		}

	else
		{
		/*  no bookmark name specified or bookmark does not exist, place
			standard error message into result. */
		CchInsertFieldError (doc, cpResult, 
				pffb->cch ? istErrBkmkNotDef : istErrNoBkmkName);
		return fcrError;
		}
}


