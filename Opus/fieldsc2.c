/* F I E L D S C 2 . C */
/*  Functions for specific field types II */

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


/* E X T E R N A L S */

extern int          vdocScratch;
extern int          vdocTemp;
extern struct MERR  vmerr;
extern struct CA    caPara;
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



/*  I N F O  F I E L D  C A L C U L A T I O N */

/*  Info Field Descriptor */
csconst struct IFD
	{
	int ifcSource : 3;  /* where to get info */
	int ifcFormat : 3;  /* what format to display info */
	int fModify : 1;    /* will second (third) argument modify info? */
	int : 1;
	int w;              /* usage depends on ifcSource: location of info */
	} rgifd [] =

{

	/* NOTE:  order is the same as flt ordering */

	/* fltTitle */
	{ ifcSttbAssoc, ifcPst, fTrue, ibstAssocTitle },

	/* fltSubject */
	{ ifcSttbAssoc, ifcPst, fTrue, ibstAssocSubject },

	/* fltAuthor */
	{ ifcSttbAssoc, ifcPst, fTrue, ibstAssocAuthor },
	
	/* fltKeyWords */
	{ ifcSttbAssoc, ifcPst, fTrue, ibstAssocKeyWords },
	
	/* fltComments */
	{ ifcSttbAssoc, ifcPst, fTrue, ibstAssocComments },
	
	/* fltLastRevBy */
	{ ifcSttbAssoc, ifcPst, fFalse, ibstAssocLastRevBy },
	
	/* fltCreateDate */
	{ ifcDop, ifcDate, fFalse, offset (DOP, dttmCreated) },
	
	/* fltRevDate */
	{ ifcDop, ifcDate, fFalse, offset (DOP, dttmRevised) },
	
	/* fltPrintDate */
	{ ifcDop, ifcDate, fFalse, offset (DOP, dttmLastPrint) },
	
	/* fltRevNum */
	{ ifcDop, ifcInt, fFalse, offset (DOP, nRevision) },
	
	/* fltEditTime */
	{ ifcEditTime, ifcLong, fFalse, offset (DOP, tmEdited) },
	
	/* fltNumPages */
	{ ifcDop, ifcInt, fFalse, offset (DOP, cPg) },
	
	/* fltNumWords */
	{ ifcDop, ifcLong, fFalse, offset (DOP, cWords) },
	
	/* fltNumChars */
	{ ifcDop, ifcLong, fFalse, offset (DOP, cCh) },
	
	/* fltFileName */
	{ ifcDocName, ifcPst, fFalse, 0 },
	
	/* fltDot */
	{ ifcSttbAssoc, ifcFileName, fFalse, ibstAssocDot }
	
};








/* F C R  C A L C  F L T  I N F O */

/* %%Function:FcrCalcFltInfo %%Owner:peterj */
FcrCalcFltInfo (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int iifd;
	int fcr = fcrNormal;
	int cch;
	int docMother = DocMother (doc);
	struct STTB **hsttbAssoc = PdodDoc (docMother)->hsttbAssoc;
	CHAR rgch [cchMaxSz];
	struct CHP chp;

	InitFvbBufs (&pffb->fvb, rgch, cchMaxSz, NULL, 0);
	SkipArgument (pffb);

	/*  optional {info info-id} format */
	if (flt == fltInfo)
		{
		if (!FFetchArgText (pffb, fTrue) ||
				(flt = FltParseSz(rgch, fFalse)) <= fltInfo || flt > fltDot)
			{
			CchInsertFieldError (doc, cpResult, istErrUnknownInfoKwd);
			return fcrError;
			}

		}

	iifd = IifdFromFlt(flt);

	/*  see if using optional {info-id new-value} format */
	if (rgifd [iifd].fModify &&
			FFetchArgText (pffb, fTrue))
		{
		/* other cases not currently supported, but could be */
		Assert (rgifd [iifd].ifcSource == ifcSttbAssoc);
		if ((hsttbAssoc = HsttbAssocEnsure (docMother)) == hNil)
			return fcrError; /* memory problems */

		SzToStInPlace (rgch);
		if (!FChangeStInSttb (hsttbAssoc, rgifd [iifd].w, rgch))
			return fcrError; /* memory problems */
		}

	/* get string value of info item and count in cch */
	if (fltNumPages <= flt && flt <= fltNumChars)
		{
		FillStatInfo_fieldspc(docMother, fTrue);
		}

	cch = CchSzInfoFromIifd (iifd, doc, rgch, hsttbAssoc, ifcFieldCalc,
			&fcr);

	/*  send rgch to document */
	if (cch > 0)
		{
		/*  get chp */
		GetResultChp (pffb, &chp);
		/* could contain CRLF's */
		InsertMultiLineRgch (doc, cpResult, rgch, cch, &chp, NULL);
		}

	return fcr;
}


/* C C H  S Z   I N F O   F R O M  I I F D  */

/* %%Function:CchSzInfoFromIifd %%Owner:peterj */
CchSzInfoFromIifd (iifd, doc, rgch, hsttbAssoc, ifcDest, pfcr)
int iifd, doc;
CHAR *rgch;
struct STTB **hsttbAssoc;
int ifcDest, *pfcr;

{
	int docMother = DocMother (doc);
	CHAR *pdop;
	CHAR rgchBuff [cchMaxSz];
	CHAR *pValue = rgchBuff; /* large enough area for any current result */
	int cch = 0;

	Assert (ifcDest != ifcFieldCalc || pfcr != NULL);

	FreezeHp ();

	/*  get pValue which represents result */
	switch (rgifd [iifd].ifcSource)
		{
	case ifcSttbAssoc:
		/* pValue is pst into heap */
		if (hsttbAssoc == hNil)
			rgchBuff[0] = 0;
		else
			GetStFromSttb(hsttbAssoc, rgifd[iifd].w, rgchBuff);
		break;

	case ifcDop:
			{
			pdop = &PdodDoc (docMother)->dop;

			bltb (pdop + rgifd [iifd].w, pValue, cchMaxInfoObj);
			break;
			}

	case ifcEditTime:
			/*  *pValue = stored time plus time since doc opened this session */
			{
			long tm;
			struct DTTM dttmCur;
			pdop = &PdodDoc (docMother)->dop;

			bltb (pdop + rgifd [iifd].w, pValue, sizeof (long));
			/* time in minutes since file was opened */
			dttmCur = DttmCur ();
			tm = CMinutesFromDttms(PdodDoc(docMother)->dttmOpened, dttmCur);
			*(long *)pValue += tm;
			break;
			}
	case ifcDocName:
			{
			GetDocSt(docMother, rgchBuff, gdsoShortName);
		/*pValue = rgchBuff;*/
			break;
			}

#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */
		}

	/*  convert pValue to the correct format in rgch */
	switch (rgifd [iifd].ifcFormat)
		{
	case ifcFileName:
		if (pValue != NULL && *pValue)
			{   /* pValue is st */
			/* now have file name--force it to be just "leaf" name, no path */
			struct FNS fns;
			Assert (cchMaxSz >= ichMaxFile);
			FreezeHp ();
			FNormalizeStFile (pValue, rgch, nfoNormal);
			MeltHp ();
			StFileToPfns (rgch, &fns);
			CopySt (fns.stShortName, rgch);
			StStAppend (rgch, fns.stExtension);
			cch = rgch [0];
			StToSzInPlace (rgch);
			}
		else
			cch = 0;
		break;

	case ifcPst:
		/* pValue is a pst */
		if (pValue == NULL)
			cch = 0;
		else
			{
			bltb (1 + (CHAR *)pValue, rgch, *(CHAR *)pValue);
			cch = *pValue;
			}
		break;

	case ifcInt:
			{
			CHAR * pch = rgch;
			cch = CchIntToPpch (*(int *)pValue, &pch);
			if (ifcDest == ifcFieldCalc)
				{
				*pfcr = FcrRegisterIntResult (*(int *)pValue);
				}
			break;
			}

	case ifcLong:
			{
			CHAR * pch = rgch;
			cch = CchLongToPpch (*(long *)pValue, &pch);
			if (ifcDest == ifcFieldCalc)
				{
				*pfcr = FcrRegisterLongResult (*(long *)pValue);
				}
			break;
			}

	case ifcDate:
		/*  rgch gets date/time represented by pValue in
				string form */
		if (ifcDest == ifcRTF)
			cch = CchRTFFormatDttmPch (pValue, rgch);
		else
			{
			CHAR szPic [cchMaxPic];
			GetDefaultSzPic (szPic, fTrue);
			cch = CchFormatDttmPic (pValue, szPic, rgch, cchMaxSz);
			if (ifcDest == ifcFieldCalc)
				{
				*pfcr = FcrRegisterDttmResult (*(struct DTTM *)pValue);
				}
			}
		break;

#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */
		}

	MeltHp ();

	return (cch);
}



/* S E T  I N F O  F R O M  I I F D  P V A L  */

/* %%Function:SetInfoFromIifdPval %%Owner:peterj */
SetInfoFromIifdPval(iifd, pValue, doc, hsttbAssoc)
int iifd;
CHAR *pValue;
int  doc;
struct STTB **hsttbAssoc;

{
	CHAR *pDest;
	int docMother = DocMother (doc);

	if (pValue == NULL)
		return;

	/*  store *pValue into info area */
	switch (rgifd [iifd].ifcSource)
		{
	case ifcSttbAssoc:
		/* pValue is pst */
		if (hsttbAssoc != hNil)
			/* not a real big deal if this fails... */
			FChangeStInSttb (hsttbAssoc, rgifd [iifd].w, pValue);
		break;

	case ifcDop:
	case ifcEditTime:
			{
			CHAR *pdop = &PdodDoc (docMother)->dop;

			switch (rgifd [iifd].ifcFormat)
				{
			case ifcInt:
				*(int *)(pdop + rgifd [iifd].w) = *(int *)pValue;
				break;
			case ifcDate:
				*(struct DTTM *)(pdop + rgifd [iifd].w) =
						*(struct DTTM *)pValue;
				break;
			case ifcLong:
				*(long *)(pdop + rgifd [iifd].w) = *(long *)pValue;
				break;
#ifdef DEBUG
			default:
				Assert (fFalse);
				break;
#endif /* DEBUG */
				}
			break;
			}

#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */
		}

}



/* We don't want to bring in dlgdoc here, do we? */
/* %%Function:FillStatInfo_fieldspc %%Owner:peterj */
FillStatInfo_fieldspc(doc, fCWords)
int	doc;
BOOL	fCWords;
{
	struct DOP	*pdop, *pdopFtn;
	struct DOD	*pdod;
	struct PLC	**hplcpgd;
	extern int	cchAvgWord10;

	pdod = PdodDoc(doc);
	pdop = &(pdod->dop);
	hplcpgd = pdod->hplcpgd;
	if (hplcpgd == hNil)
		{
		pdop->cPg = 1;
		}
	else
		{
		pdop->cPg = IMacPlc(hplcpgd);
		}
	pdop->cCh = CpMacDocEdit(doc);
	if (pdod->docFtn != docNil)
		{
		pdop->cCh += CpMacDocEdit(pdod->docFtn);
		}
	if (fCWords && !pdop->fExactCWords)
		{
		long	cw, div;

		if (pdop->cCh * 10L < pdop->cCh)
			{
			cw = pdop->cWords = (pdop->cCh / cchAvgWord10) * 10L;
			}
		else
			{
			cw = pdop->cWords = (pdop->cCh * 10L) / cchAvgWord10;
			}
		if (cw < 1000L)
			{
			div = (100L < cw) ? 100L :
					((10L < cw) ? 10L : 1L);
			}
		else
			{
			div = 10L;
			while (cw / div >= 1000L)
				{
				div *= 10L;
				}
			}
		pdop->cWords = div * (pdop->cWords / div);
		}
}




/* F C R  C A L C  F L T  D A T E  T I M E*/
/*  Get print date.
	Format: {date} 
			{time}
*/

/* %%Function:FcrCalcFltDateTime %%Owner:peterj */
FcrCalcFltDateTime (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	struct DTTM dttm;
	int cch;
	int fcr;
	struct CHP chp;
	CHAR szResult [cchMaxSz];
	CHAR szPic [cchMaxPic];

	/*  determine the date */
	dttm = DttmCur ();

	fcr = FcrRegisterDttmResult (dttm);

	/*  format the date into a string */
	GetDefaultSzPic (szPic, flt==fltDate);
	cch = CchFormatDttmPic (&dttm, szPic, szResult, cchMaxSz);

	/*  place the string into the doc */
	GetResultChp (pffb, &chp);
	if (!FInsertRgch (doc, cpResult, szResult, cch, &chp, NULL))
		return fcrError;

	return fcr;
}


/* F C R  C A L C  F L T  P A G E */
/*  Get current page number.
	Format: {page}
*/

/* %%Function:FcrCalcFltPage %%Owner:peterj */
FcrCalcFltPage (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB * pffb;

{
	int pgn;
	CP cpRef;
	int docRef;
	int cch;
	struct CHP chp;

	/*  who's page do we want to know about? */
	GetPdocPcpMotherFromDocCp (doc, cpInst, &docRef, &cpRef);

	/*  place the string into the doc */
	GetResultChp (pffb, &chp);
	pgn = PgnInsertSzPage (docRef, cpRef, doc, cpResult, &chp, &cch);

	return (FcrRegisterIntResult (pgn));
}



/* F C R  C A L C  F L T  P A G E  R E F */
/*  Get page number of bookmark.
	Format: {pageref bookmark}
*/

/* %%Function:FcrCalcFltRefPage %%Owner:peterj */
FcrCalcFltRefPage (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB * pffb;

{
	int pgn, cch;
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
		pgn = PgnInsertSzPage (docMother, cpFirst, doc, cpResult, &chp,
				&cch);
		return (FcrRegisterIntResult (pgn));
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



/* P G N  I N S E R T  S Z  P A G E */
/*  Insert the string representation of the page containing docRef, cpRef at
	doc, cp using chp.  Return the pgn inserted.
*/
/* %%Function:PgnInsertSzPage %%Owner:peterj */
PgnInsertSzPage (docRef, cpRef, doc, cp, pchp, pcch)
int docRef;
CP cpRef;
int doc;
CP cp;
struct CHP *pchp;
int *pcch;

{
	int pgn;
	int nfc;

	CacheSect (docRef, cpRef);
	nfc = vsepFetch.nfcPgn;

	/* multi-page footnotes generate strange plcpgd entries such that the
		normal (doc,cp) way of finding your page basically doesn't work.
		Page fields in the footnote text don't matter because they're mapped
		back to their references in the mother doc.  The only interesting
		case is a page field in a header or footer, so grab it from vfmtss,
		which is filled at the beginning of LbcFormatPage.  This is correct
		for Print and Preview, but will be wrong for multiple page view
		panes on different footnote pages of the same doc.  Hey, two out
		of three ain't bad.... (davidbo) */

	pgn = PdodDoc(doc)->fHdr ? vfmtss.pgn : PgnFromDocCp(docRef, cpRef);

	/* put number into doc */
	*pcch = CchInsertLongNfc (doc, cp, (LONG)pgn, nfc, pchp);

	return pgn;
}



/* P G N  F R O M  D O C  C P */

/* %%Function:PgnFromDocCp %%Owner:peterj */
PgnFromDocCp (doc, cp)
int doc;
CP cp;

{
	struct PLC **hplcpgd = PdodDoc (doc)->hplcpgd;
	struct PGD pgd;

	Assert (doc == DocMother (doc));
	Assert (cp != cpNil);

	if (hplcpgd == hNil)
		{
		CacheSect(doc, cp);
		return vsepFetch.fPgnRestart ? max(vsepFetch.pgnStart,1) : 1;
		}

	CachePage (doc, cp);
	GetPlc( hplcpgd, vipgd, &pgd );
	return pgd.pgn != pgnMax ? pgd.pgn : 1;
}



/* C C H  I N S E R T  L O N G	N F C */
/*  Insert a representation of l based on nfc into doc at cp using *pchp.
*/

/* %%Function:CchInsertLongNfc %%Owner:peterj */
CchInsertLongNfc (doc, cp, l, nfc, pchp)
int doc;
CP cp;
long l;
int nfc;
struct CHP *pchp;

{
	int cch;
	CHAR rgch [cchMaxSz];

	if ((cch = CchLongToRgchNfc (l, rgch, nfc, cchMaxSz)) > 0)
		if (!FInsertRgch (doc, cp, rgch, cch, pchp, NULL))
			cch = 0;
	Assert (cch >= 0);
	return cch;
}


/* C C H  L O N G  T O	R G C H  N F C */
/* Format the l according to nfc.
Result to pch, return cch <= cchMax, 0 if cchMax is reached.
*/
/* %%Function:CchLongToRgchNfc %%Owner:peterj */
int CchLongToRgchNfc(l, pch, nfc, cchMax)
long l;
char *pch;
int nfc, cchMax;
{
	int ch;
	int cch, ich;
	BOOL f = fTrue;

	switch (nfc)
		{
	case nfcArabic:
		if (cchMax < cchMaxLong)
			return 0;
		return CchLongToPpch(l, &pch);

	case nfcLCRoman:
		f = fFalse;  /* fUpperCase */
		/* FALL THROUGH */
	case nfcUCRoman:
		if (l >= nMinRoman && l < nMaxRoman)
			return CchStuffRoman(&pch, (uns)l, f, cchMax);
		else
			return 0;

	case nfcLCLetter:
		f = fFalse;  /* fUpperCase */
		/* FALL THROUGH */
	case nfcUCLetter:
		if (l <= 0 || (cch = (l - 1) / 26 + 1) > cchMax)
			return 0;
		ch = (l - 1) % 26 + (f ? 'A' : 'a');
		for (ich = 0; ich < cch; ich++)
			*pch++ = ch;
		return cch;

	case nfcOrdinal:  /* 1st, 2nd,... */
		if (cchMax < cchMaxLong+cchMaxOrdApp)
			return 0;
		return CchFormatLongOrdinal (l, &pch);

	case nfcCardtext: /* One, Two,... */
		f = fFalse; /* fOrdinal */
		/* FALL THROUGH */
	case nfcOrdtext:  /* First, Second,... */
		if (l < nMinText || l >= nMaxText || cchMax < cchMaxTextNum)
			return 0;
		return CchStuffLongText (l, &pch, f);

	case nfcHex:
		if (cchMax < 8 || l < 0 || l > 0x00007fffL)
			return 0;
		return CchIntToAsciiHex ((int)l, &pch, 0);

#ifdef DEBUG
	default:
		Assert(fFalse);
		return 0;
#endif /* DEBUG */
		}
}



