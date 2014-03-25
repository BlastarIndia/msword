/* toc.c - Table of Contents routines

	10/28/87 (rosiep) - wrote from a combination of MacWord table.c and
							Opus index1.c
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "ch.h"
#include "file.h"
#include "prm.h"
#include "format.h"
#include "field.h"
#include "inter.h"
#include "toc.h"
#include "prompt.h"
#include "message.h"
#include "debug.h"
#include "error.h"
#include "opuscmd.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "toc.hs"
#include "toc.sdm"



#ifdef PROTOTYPE
#include "toc.cpt"
#endif /* PROTOTYPE */

/*  E x t e r n a l s */
extern int              wwCur;
extern int              vccpFetch;
extern int              vfSeeSel;
extern CHAR HUGE        *vhpchFetch;
extern struct FLI       vfli;
extern struct SEL       selCur;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct CHP       vchpStc;
extern struct PREF      vpref;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct MERR      vmerr;
extern struct UAB       vuab;
extern CHAR             stEmpty[];
extern CHAR             szEmpty[];
extern int              vwFieldCalc;
extern BOOL             fElActive;
extern BOOL             vfRecording;
extern HWND				vhwndCBT;


/*  Globals:  */
struct TIB     *ptib = NULL;              /* Table of contents Info Block */
BOOL    fRemoveToc;



/* C M D  T O C */
/* Insert TOC field into document */

/* %%Function:CmdTOC %%Owner:rosiep */
CMD CmdTOC(pcmb)
CMB *pcmb;
{
	CABTOC * pcabToc;
	struct CA ca, caT;
	CHAR stArg[4];  /* room for "2-9" + cch */
	CHAR st[7];  /* room for "\o 2-9" + cch */
	CHAR *pch;
	int cmd;

	if (FCmdFillCab())
		{
		pcabToc = *pcmb->hcab;

		pcabToc->iTocType = iTocOutline;
		pcabToc->iOutlineLevels = iLevAll;

		pcabToc->wTocFrom = uNinch;
		pcabToc->wTocTo = uNinch;
		pcabToc->fReplace = fFalse;
		}

	StartLongOp();
	if (FCmdDialog())
		{
		char dlt [sizeof (dltToc)];

		BltDlt(dltToc, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			cmd = cmdError;
			goto LRet;
#endif
		case tmcError:
			cmd = cmdNoMemory;
			goto LRet;

		case tmcCancel:
			cmd = cmdCancelled;
			goto LRet;

		case tmcOK:
			break;
			}
		}

	if (!FCmdAction())
		{
		cmd = cmdOK;
		goto LRet;
		}
		
	if (pcmb->fCheck)
		{
		CABTOC * pcab;
		
		/* If either From or To is specified in a macro, get rid of 
			the All option and make sure both From and To has a value. */
		pcab = *pcmb->hcab;
		if (pcab->wTocFrom != uNinch || pcab->wTocTo != uNinch)
			{
			if (pcab->wTocFrom == uNinch)
				pcab->wTocFrom = iLevFirstDef;

			if (pcab->wTocTo == uNinch)
				pcab->wTocTo = iLevLastDef;

			pcab->iOutlineLevels = iLevFromTo;
			}
		}

	/* not repeatable, yet we do want to blow away the again state */
	SetAgain(bcmNil);

	/* check for existing toc and ask if user wants to replace it */
	ca.doc = selCur.doc;
	if (FScanDocForFlt(&ca, fltToc))
		{
		/* select old toc and make it visible */
		Select(&selCur, ca.cpFirst, ca.cpLim);
		NormCp(wwCur, selCur.doc,ca.cpFirst, 0, 0, fFalse);
		/* If running the macro statement, use the fReplace field to
		   determine whether to replace old index, otherwise ask the user */
		if (fElActive)
			{
			if (!((CABTOC *) *pcmb->hcab)->fReplace)
				{
				cmd = cmdCancelled;
				goto LRet;
				}
			}
		else  if (IdMessageBoxMstRgwMb(mstReplaceToc, NULL, 
				MB_YESNOQUESTION) != IDYES)
			{
			cmd = cmdCancelled;
			goto LRet;
			}
		else
			{
			/* Record fReplace so Macro recording gets users choice */
			((CABTOC *) *pcmb->hcab)->fReplace = fTrue;
			}

		/* user wants to blow away old toc */

		/* get transient properties */
		GetSelCurChp (fFalse);

		/* delete old toc */
		if ((cmd = CmdDeleteSelCur(fFalse, fFalse, &caT, rpkText)) != cmdOK)
			goto LRet;

		/* keep from losing props */
		selCur.fUpdateChp = fFalse;
		}

	/* OK */

	/* Now that we have all the info, record cab if applicable */
	if (vfRecording)
		FRecordCab(pcmb->hcab, IDDInsToc, tmcOK, fFalse);

	/* collect switch info from dialog */

	st[0] = 0;

	pcabToc = *pcmb->hcab;

	if (pcabToc->iTocType == iTocOutline)
		{
		stArg[0] = 0;
		if (pcabToc->iOutlineLevels != iLevAll)
			{
			pch = &stArg[1];
			stArg[0] += CchIntToPpch(pcabToc->wTocFrom, &pch);
			*pch++ = '-';
			stArg[0]++;
			stArg[0] += CchIntToPpch(pcabToc->wTocTo, &pch);
			}

		AppendSwitchSt(chFldTocOutline, st, stArg[0] ? stArg : NULL);
		}
	else
		AppendSwitchSt(chFldTocFields, st, NULL);

	/* remove extra space at end */
	st[0]--;

	StToSzInPlace(st);

	fRemoveToc = fFalse;  /* set indirectly by CmdInsFltSz... */
	caT = selCur.ca;

	vwFieldCalc |= fclSpecialFunc;
	cmd = CmdInsFltSzAtSelCur(fltToc, (st[0] ? st : NULL) /* szArgs */, 
			imiTOC, fTrue /* fCalc */, fFalse /* fShowDefault */, 
			fTrue /* fShowResult */);
	vwFieldCalc &= ~fclSpecialFunc;

	if (fRemoveToc)  /* no toc entries were found; get rid of field */
		{
		caT.cpLim = selCur.cpLim;
		AssureLegalSel(&caT);
		FDelete(&caT);
		SetUndoAfter(&selCur.ca);
		cmd = cmdError;
		}
	PushInsSelCur();

LRet:
	EndLongOp(fFalse);
	return cmd;
}


/* F  D L G  T O C */
/* Dialog function for Table of Contents */

/* %%Function:FDlgToc %%Owner:rosiep */
BOOL FDlgToc(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	BOOL fEnable = fTrue;
	TMC rgtmc[2];

	switch (dlm)
		{
	case dlmChange:
		if (tmc == tmcTocFrom || tmc == tmcTocTo)
			{
			/* if in all, set up the OTHER edit control to
			its default value and set the radio to tofrom
			*/
			if (ValGetTmc(tmcOutlineLevels) == iLevAll)
				{
				if (tmc == tmcTocFrom)
					SetTmcVal(tmcTocTo, iLevLastDef);
				else
					SetTmcVal(tmcTocFrom, iLevFirstDef);

				SetTmcVal(tmcOutlineLevels, iLevFromTo);
				}
			rgtmc[0] = tmcTocFrom;
			rgtmc[1] = tmcTocTo;
			GrayRgtmcOnBlank(rgtmc, 2, tmcOK);
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcTocFields:
			SetTmcVal(tmcOutlineLevels, iLevAll);
			fEnable = fFalse;
			/* FALL THROUGH */

		case tmcTocOutline:
/* No longer needed as the group box was removed  */
/*			EnableTmc(tmcTocGroup, fEnable);   */
			EnableTmc(tmcTocAll, fEnable);
			EnableTmc(tmcTocFromTo, fEnable);
			EnableTmc(tmcTocFrom, fEnable);
			EnableTmc(tmcTocTo, fEnable);
			if (fEnable)
				break;
			/* FALL THROUGH */

		case tmcTocAll:
			SetTmcText(tmcTocFrom, szEmpty);
			SetTmcText(tmcTocTo, szEmpty);
			EnableTmc(tmcOK, fTrue);
			break;

		case tmcTocFromTo:
			if (wOld == iLevAll)
				{
				SetTmcVal(tmcTocFrom, iLevFirstDef);
				SetTmcVal(tmcTocTo, iLevLastDef);
				/* this must be after sets or dlmChange will be sent */
				SetFocusTmc(tmcTocFrom);
				}
			break;
			}
		break;
		}

	return fTrue;
}


/* %%Function:WParseToc %%Owner:rosiep */
EXPORT WORD WParseToc(tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	uns uLow, uFrom;

	uLow = iLevFirstDef;
	if (HidOfDlg(hdlgNull) == IDDInsToc && tmc == tmcTocTo)
		{
		uFrom = ValGetTmc(tmcTocFrom);
		if (uFrom != uNinch)
			uLow = uFrom;
		}

	return WParseIntRange(tmm, sz, ppv, bArg, tmc, uLow, iLevLastDef);
}


/* F C R  C A L C  F L T  T O C */
/* Field calculation function for TOC field.  Scan document for outline
	levels or TC fields (depending on whether \o switch or \f switch)
	and build up table of contents.
*/

/* %%Function:FcrCalcFltToc %%Owner:rosiep */
FcrCalcFltToc(doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;
{
	CHAR stRange[cchBkmkMax+1];       /* bookmark range of toc */
	char chSwitch;
	CP cpFirst = cpNil, cpLim = cpNil;
	int istError;
	int fcr = fcrNormal;
	CHAR szChar[2]; /* includes 1 char plus '\0' */
	CHAR szLevRange[cchMaxLevRange];
	int f;
	struct TIB tib;

	ptib = &tib; /* set up global pointer */

	tib.docMain = doc;
	if (tib.docMain != DocMother(doc))
		{
		CchInsertFieldError(doc, cpResult, istErrSubDocToc);
		return fcrError;
		}

/* reset defaults in case field switches have changed since last refresh */
	tib.fTocFields = fFalse;
	tib.chTbl = chTblDef;
	tib.iLevMin = 1;
	tib.iLevLast = 9;
	tib.fSequence = fFalse;
	tib.fTable = FInTableDocCp(doc, cpInst);
	CopySt(StSharedKey("-",TOCSeqSep), tib.stSeqSep);

	/* skip the keyword */
	SkipArgument (pffb);

	/* process switches */

	ExhaustFieldText(pffb);
	while ((chSwitch = ChFetchSwitch(pffb, fFalse /* fSys */)) != chNil)
		{
		switch (chSwitch)
			{
		case chFldTocOutline:
			InitFvbBufs(&pffb->fvb, szLevRange, cchMaxLevRange, NULL, 0);
			if (FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				if (!FParseLevRange(szLevRange, &tib.iLevMin, &tib.iLevLast))
					{
					istError = istErrInvalidOutlineRange;
					goto LError;
					}
				}
			break;

		case chFldTocFields:
			tib.fTocFields = fTrue;
			InitFvbBufs(&pffb->fvb, szChar, 2, NULL, 0);
			if (FFetchArgText (pffb, fTrue /* fTruncate */))
				tib.chTbl = ChUpper(szChar[0]);
			break;

		case chFldTocBookmark:
			InitFvbBufs(&pffb->fvb, stRange+1, cchBkmkMax, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoBkmkName;
				goto LError;
				}
			/* make an st */
			stRange[0] = pffb->cch;
			if (!FSearchBookmark (doc, stRange, &cpFirst, &cpLim, NULL,
					bmcUser))
				{
				istError = istErrBkmkNotDef;
				goto LError;
				}
			break;

		case chFldTocSeqname:
			tib.fSequence = fTrue;
			InitFvbBufs(&pffb->fvb, tib.szSequence, cchSequenceIdMax, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoSeqSpecified;
				goto LError;
				}
			break;

		case chFldTocSeqSep:
			InitFvbBufs(&pffb->fvb, tib.stSeqSep + 1, cchMaxSeparator, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoSwitchArgument;
				goto LError;
				}
			/* make an st */
			tib.stSeqSep[0] = pffb->cch;
			break;
			}

		}


/*  Use a temporary document to build up the toc in. */

	if ((ptib->doc = DocCreate(fnNil)) == docNil)
		{
		ErrorEid(eidCantCreateTemp, " FcrCalcFltToc");
		return fFalse;
		}
	ptib->cpMac = cp0;

	if (FScanDocForEntries(tib.docMain, cpFirst, cpLim,
			tib.fTocFields ? sdeToc : sdeOutline))
		{
		HpprStartProgressReport(mstBuildingToc, NULL, 1, fTrue);
		if (!FOutputToc(doc, cpResult, pffb->cpField))
			fcr = fcrKeepOld;
		}

	DisposeDoc(ptib->doc);
	return fcr;

LError:
	CchInsertFieldError (doc, cpResult, istError);
	return fcrError;
}



/* F  P R O C E S S  T O C  E N T R Y */
/* %%Function:FProcessTocEntry %%Owner:rosiep */
BOOL FProcessTocEntry(pffb, doc, pgn, nfc)
struct FFB *pffb;
int doc;
uns pgn, nfc;
{
	CHAR rgchEntry[cchMaxEntry+1];
	CHAR rgchTable[cchMaxTable]; /* one char + '\0' */
	CHAR stLevel[cchMaxLevel+1];
	CHAR *pch, *pchLim;
	CHAR chSwitch;
	CHAR chTbl = chTblDef;
	int iLevel = 1;
	int cch;
	int wSequence;

	Assert (ptib != NULL);
	Assert (0 <= ptib->iLevMin && ptib->iLevMin <= ptib->iLevLast &&
			ptib->iLevLast <= 9);

	/* skip the keyword */
	SkipArgument(pffb);

	InitFvbBufs(&pffb->fvb, rgchEntry, cchMaxEntry+1, NULL, 0);

	if (!FFetchArgText(pffb, fTrue /* fTruncate */))
		{
		/* null toc entry - continue processing other entries */
		return fTrue;
		}

	cch = pffb->cch;
	pch = rgchEntry;

	/* get rid of leading whitespace */
	cch -= CchSkipSpace(&pch, cch);

	/* get rid of trailing whitespace */
	pchLim = pch + cch;
	while (pchLim > pch && FWhite(*--pchLim))
		cch--;

	/* process switches */

	ExhaustFieldText(pffb);
	while ((chSwitch = ChFetchSwitch(pffb, fFalse /* fSys */)) != chNil)
		{
		if (chSwitch == chFldTceTableId)
			{
			InitFvbBufs(&pffb->fvb, rgchTable, cchMaxTable, NULL, 0);
			if (FFetchArgText (pffb, fTrue /* fTruncate */))
				chTbl = ChUpper(*rgchTable);
			}
		else  if (chSwitch == chFldTceLevel)
			{
			InitFvbBufs(&pffb->fvb, stLevel+1, cchMaxLevel, NULL, 0);
			if (FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				iLevel = stLevel[1] - '0';

				/* check if bogus level (out of range 1-9 or not a number) */
				if (pffb->cch != 1 || (uns)(iLevel - iLevFirstDef) >
						(iLevLastDef - iLevFirstDef))
					return FTocEntryError(doc, pgn, istErrInvalidLevelTC, nfc);

				if (iLevel < ptib->iLevMin || iLevel > ptib->iLevLast)
					return fTrue;  /* not in range we're looking for */
				}
			}
		/* else illegal switch - ignore */
		}

	if (ptib->fSequence)
		wSequence = WSequenceValue (ptib->szSequence, doc, pffb->cpField, fFalse);

	if (chTbl == ptib->chTbl)
		if (!FEnterTocEntry(doc, pch, cch, iLevel, pgn, nfc, wSequence))
			return fFalse;

	/* if entry doesn't match table-id, it doesn't get collected */

	return fTrue;
}


/* F  E N T E R  T O C  E N T R Y */
/* Insert toc entry into temp doc */
/* returns fFalse if failed */
/* handles characters < chSpace */

/* %%Function:FEnterTocEntry %%Owner:rosiep */
BOOL FEnterTocEntry(doc, rgchEntry, cchEntry, iLevel, pgn, nfc, wSequence)
char *rgchEntry;
int doc, cchEntry, iLevel;
uns pgn, nfc;
int wSequence;    /* garbage if !ptib->fSequence; that's OK */
{

	int     cch = 0;
	char	*pch, *pchLim;
	int		ch;
	char    rgch[cchChSpecMax + 3];
	struct CHP chp;
	struct PAP pap;

	Assert (ptib != NULL);

	if (cchEntry == 0)
		return fTrue;

	if (iLevel < ptib->iLevMin || iLevel > ptib->iLevLast)
		return fTrue;

	if (!FGetChpPapForToc(doc, &chp, &pap, iLevel))
		return fFalse;

/* scan for fSpec or other dangerous characters */
	for (pch = rgchEntry, pchLim = pch + cchEntry; pch < pchLim; ++pch)
		if (*pch < chSpace)
			{
			ch = *pch;
			if (ch == chTab || ch == chNonBreakHyphen)
				continue;	/* these are the only OK low ch's */
			if (ch == chReturn || ch == chEop || ch == chTable)
				*pch = chSpace;
			else
				{
				bltbyte(pch + 1, pch, --pchLim - pch);	/* at end this moves 0 */
				pch--;	/* point at same character position */
				cchEntry--;
				}
			}

	if (!FInsertRgch(ptib->doc, ptib->cpMac, rgchEntry, cchEntry, &chp, 0))
		return fFalse;
	ptib->cpMac += cchEntry;

	if (pgn != pgnNil)
		{
		rgch[cch++] = chTab;
		if (ptib->fSequence)
			{
			char *pchT;

			if (nfc == nfcArabic || wSequence != 0)
				{
				pchT = rgch+cch;
				cch += CchIntToPpch(wSequence, &pchT);
				cch += CchCopyStRgch(ptib->stSeqSep, rgch+cch);
				}
			}
		cch += CchLongToRgchNfc((LONG)pgn, &rgch[cch], nfc, cchChSpecMax);
		}

#ifdef CRLF
	rgch[cch++] = chReturn;
#endif /*CRLF*/
	rgch[cch++] = chEol;

	if (!FInsertRgch(ptib->doc, ptib->cpMac, rgch, cch, &chp, &pap))
		return fFalse;

	ptib->cpMac += cch;
	return fTrue;
}


/*  F  G E T  C H P  P A P  F O R  T O C  */
/*  Sets up the CHP and PAP for the next TOC entry */

/* %%Function:FGetChpPapForToc %%Owner:rosiep */
BOOL FGetChpPapForToc(doc, pchp, ppap, iLevel)
int doc, iLevel;
struct CHP *pchp;
struct PAP *ppap;
{
	int stc, cstcStd;

	stc = stcToc1 - iLevel + 1;
/* FUTURE ... this hack gets around the fact that we have one
	more heading level than toc levels.  When we fix the bogus limit
	on standard styles, we should add a toc9 and use that instead of
	mapping level9 to toc8. (davidbo) */
	if (stc < stcTocMin)
		stc = stcTocMin;
	if (!FEnsureStcDefined(doc, stc, &cstcStd))
		return fFalse;
	MapStc(PdodDoc(doc), stc, 0, ppap);
	ppap->fInTable = ptib->fTable;
	CachePara(ptib->doc, ptib->cpMac);
	*pchp = vchpStc;

	return fTrue;
}


/* F  T O C  E N T R Y  E R R O R */

/* %%Function:FTocEntryError %%Owner:rosiep */
BOOL FTocEntryError(doc, pgn, istError, nfc)
int doc, pgn, istError;
uns nfc;
{
	char rgch[cchChSpecMax + 3];
	int cch = 0;
	struct CHP chp;
	struct PAP pap;

	if (!FGetChpPapForToc(doc, &chp, &pap, 1))
		return fFalse;

	ptib->cpMac += CchInsertFieldError (ptib->doc, ptib->cpMac, istError);
	if (pgn != pgnNil)
		{
		cch += CchLongToRgchNfc((LONG)pgn, rgch, nfc, cchChSpecMax);
		}

#ifdef CRLF
	rgch[cch++] = chReturn;
#endif /*CRLF*/
	rgch[cch++] = chEol;

	if (!FInsertRgch(ptib->doc, ptib->cpMac, rgch, cch, &chp, &pap))
		return fFalse;

	ptib->cpMac += cch;
	return fTrue;
}


/* F  F I N D  T O C  O U T L I N E */
/* Scan a page (cpFirst and cpLim delimit the page) for outline
	heading levels.
*/

/* %%Function:FFindTocOutline %%Owner:rosiep */
BOOL FFindTocOutline(pca, pgn, nfc)
struct CA *pca;
uns pgn, nfc;
{
	int doc = pca->doc;
	CP cpLim = pca->cpLim;
	CHAR rgchText[cchMaxEntry], *pchText, *pchLim;
	CP cpNext;
	int iLevel, cchText, dpch;
	struct FVB fvb;
	struct CR rgcr[ccrArgumentFetch];
	int wSequence;

	InitFvbBufs (&fvb, rgchText, cchMaxEntry, rgcr, ccrArgumentFetch);
	fvb.doc = doc;

	cpLim = CpMin(cpLim, CpMacDoc(doc) - ccpEop);
	cpNext = pca->cpFirst;

	/* don't start in mid-para; one page break is allowed at start of para */
	CachePara(doc, pca->cpFirst);
	if (pca->cpFirst != caPara.cpFirst)
		if (pca->cpFirst != caPara.cpFirst + 1)
			cpNext = caPara.cpLim;
		else
			{
			FetchCp(doc, caPara.cpFirst, fcmChars);
			if (*vhpchFetch == chSect)
				cpNext++;
			else
				cpNext = caPara.cpLim;
			}

	while (!vmerr.fMemFail && cpNext < cpLim && !vmerr.fDiskFail)
		{
		CachePara(doc, cpNext);
		cpNext = caPara.cpLim;
		if ((iLevel = stcLevLast - vpapFetch.stc + 1) >= ptib->iLevMin &&
				iLevel <= ptib->iLevLast)
			{
			pchText = &rgchText[0];
			fvb.cpFirst = caPara.cpFirst;
			fvb.cpLim = CpMin(caPara.cpLim, cpLim);
			FetchVisibleRgch(&fvb, fvcResults, fFalse, fFalse);
			cchText = fvb.cch;

			/* get rid of leading whitespace */
			cchText -= CchSkipSpace(&pchText, cchText);
			pchLim = pchText + cchText;

			/* back up over Eop */
#ifdef CRLF
			if (*(pchLim-1) == chEol && *(pchLim-2) == chReturn)
#else
				if (*(pchLim-1) == chEol)
#endif /* CRLF */
					{
					cchText -= ccpEop;
					pchLim -= ccpEop;
					}

			/* get rid of trailing whitespace */
			while (pchLim > pchText && FWhite(*--pchLim))
				cchText--;

			if (ptib->fSequence)
				wSequence = WSequenceValue (ptib->szSequence, doc, caPara.cpFirst,
						fFalse);
			if (!FEnterTocEntry(doc, pchText, cchText, iLevel, pgn, nfc, wSequence))
				return fFalse;
			}
		}

	return fTrue;
}



/* F  O U T P U T  T O C */
/* Copy the contents of the temp doc into the toc field result. */
/* Return true if we actually put something in the result. */

/* %%Function:FOutputToc %%Owner:rosiep */
BOOL FOutputToc(doc, cpResult, cpField)
int doc;
CP cpResult;
CP cpField;   /* first cp of TOC field */
{
	struct SEP sep;
	struct SEL sel;
	struct CA caSrc, caDest, ca;

	if (ptib->cpMac == cp0)
		{
		if (vwFieldCalc & fclSpecialFunc)
			{
			/* CBT will guarantee that there are entries, and they
				can't deal with a "no printers installed" message */
			if (!vhwndCBT)
				{
				ErrorEid(vmerr.fPrintEmerg ? eidCantRepag :
					(ptib->fTocFields ? eidNoTocFields :
					eidNoOutlineLevels), "FOutputToc");
				}
			fRemoveToc = fTrue;
			return fFalse;
			}
		else
			{
			CchInsertFieldError(doc, cpResult,
				vmerr.fPrintEmerg ? istErrCantRepag :
				(ptib->fTocFields ? istErrNoTocEntries :
				istErrNoOutlineLevels));
			return fTrue;
			}
		}

/*  Make sure TOC starts a new paragraph */
	if (!FEnsureNewPara(doc, &cpResult, cpField))
		return fFalse;

/*  Put the table into the document  */
	if (!FReplaceCps(PcaPoint( &caDest, doc, cpResult ),
			PcaSet( &caSrc, ptib->doc, cp0, ptib->cpMac )))
		return fFalse;

	DirtyOutline(doc);
	return fTrue;
}





/* F  P A R S E  L E V  R A N G E  */
/* Parse a range string that is supposed to be of the form: x-y
	where 0 <= x <= y <= 9.  Return false if invalid range.  Non-
	numeric characters are ignored.  I.e. the range separator
	does not necessarily have to be a hyphen; this is to be very
	lenient on the user, and also because it would require more
	code to be rigid.
*/

/* %%Function:FParseLevRange %%Owner:rosiep */
BOOL FParseLevRange(sz, piMin, piLast)
CHAR *sz;
int *piMin, *piLast;
{
	char *pch = sz;
	CHAR ch;
	int cFound = 0;  /* number of single digit numbers found so far */

	while ((ch = *pch++) != 0)
		if (ch >= '0' && ch <= '9')
			{
			if (*pch >= '0' && *pch <= '9') /* two digit number found */
				return fFalse;
			else
				{
				switch (cFound)
					{
				case 0:
					*piMin = ch - '0';
					break;
				case 1:
					*piLast = ch - '0';
					break;
				case 2:
					return fFalse;
					}
				}
			cFound++;
			}

	return (cFound = 2 && *piMin <= *piLast);
}


