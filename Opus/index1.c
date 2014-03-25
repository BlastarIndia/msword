/* INDEX1.C - Ported from MacWord 9/10/87 - rosiep */

/*  This contains the code to scan for index entries and then  */
/*  format them and insert the index into the document.  */


/*  This code uses four data types:

			IDB: index data block:  the information stored for each
									index entry
			FPGN: formatted page number: a pgn and an nfc, one word total
			PNB: page number block: an array of page numbers and an
									offset to another pnb
			EXT: explicit reference

	The data structure is a hash-table which is then turned into
	a heap-tree.  The nodes are inserted in the hash table with
	chains in sorted order.  Then a heap-tree is formed (out of the
	top-level nodes of the chains coming off the hash table) with its
	head at the beginning of the hash table (see more descriptive
	comment above ConstructHeap).  The index is built up in alphabetic
	order in a temporary document by removing the smallest node in
	the heap tree one at a time and keeping the tree balanced (this
	is taken care of by HpidbFirstFromHeap and FilterDownHeap).
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "format.h"
#include "disp.h"
#include "ch.h"
#include "file.h"
#include "prm.h"
#include "inter.h"
#include "field.h"
#include "prompt.h"
#include "message.h"
#include "debug.h"
#include "layout.h"
#include "index.h"
#include "opuscmd.h"
#include "error.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "index.hs"
#include "index.sdm"

#include "indexent.hs"
#include "indexent.sdm"



#ifdef PROTOTYPE
#include "index1.cpt"
#endif /* PROTOTYPE */

/*  E x t e r n a l s  */
extern CHAR             stEmpty[];
extern CHAR             rgchEop[];
extern int              wwCur;
extern struct MERR      vmerr;
extern struct PREF      vpref;
extern struct SEL       selCur;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct CA        caSect;
extern struct CA        caPara;
extern struct FLI       vfli;
/* compiler bug forces us to declare w/ double indirection */
extern HPIDB            **prghpIndex;
extern struct UAB       vuab;
extern struct ITR       vitr;
extern int              vwFieldCalc;
extern BOOL             fElActive;
extern BOOL             vfRecording;
extern HWND				vhwndCBT;

/* G l o b a l s */
struct IIB     *piib = NULL;    /* Index Info Block - global junk */
BOOL fRemoveIndex;

#ifdef DEBUG
extern struct DBS vdbs;
int vcEntries;
#endif /* DEBUG */


CP CpOutputEntry(), CpOutputHeading();
HPIDB HpidbFirstFromHeap();


/* C M D  I N D E X */
/* Insert index field into document */
/* %%Function:CmdIndex %%Owner:rosiep */
CMD CmdIndex(pcmb)
CMB * pcmb;
{
	CABINDEX * pcabIndex;
	struct CA caT;
	CMD cmd = cmdOK;

	if (FCmdFillCab())
		{
		pcabIndex = (CABINDEX *) *pcmb->hcab;
		pcabIndex->iIndexType = iTypeNested;
		pcabIndex->iHeading = iHeadingNone;
		pcabIndex->fReplace = fFalse;
		}

	StartLongOp();
	if (FCmdDialog())
		{
		char dlt [sizeof (dltIndex)];

		BltDlt(dltIndex, dlt);
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

	if (FCmdAction())
		{
		struct CA ca;
		char st [cchMaxIndexArgs];

		/* not repeatable, yet we do want to blow away the again state */
		SetAgain(bcmNil);

		/* check for existing index and ask if user wants to 
			replace it */
		ca.doc = selCur.doc;
		if (FScanDocForFlt(&ca, fltIndex))
			{
			/* select old index and make it visible */
			Select(&selCur, ca.cpFirst, ca.cpLim);
			NormCp(wwCur, selCur.doc, ca.cpFirst, 0, 0, fFalse);
			/* If running the macro statement, use the fReplace field to
			   determine whether to replace old index, otherwise ask the user */
			if (fElActive)
				{
				if (!((CABINDEX *) *pcmb->hcab)->fReplace)
					{
					cmd = cmdCancelled;
					goto LRet;
					}
				}
			else  if (IdMessageBoxMstRgwMb(mstReplaceIndex, NULL, 
					MB_YESNOQUESTION) != IDYES)
				{
				cmd = cmdCancelled;
				goto LRet;
				}
			else
				/* Record fReplace so Macro recording gets users choice */
				((CABINDEX *) *pcmb->hcab)->fReplace = fTrue;

			/* user wants to blow away old index */
			if ((cmd=CmdDeleteSelCur(fFalse, fFalse, &caT, rpkText)) != cmdOK)
				goto LRet;
			}

		/* Now that we have all the info, record cab if applicable */
		if (vfRecording)
			FRecordCab(pcmb->hcab, IDDIndex, tmcOK, fFalse);

		/* collect switch info from dialog */
		st[0] = 0;

		pcabIndex = (CABINDEX *) *pcmb->hcab;

		if (pcabIndex->iIndexType == iTypeRunin)
			AppendSwitchSt(chFldIndexRunin, st, NULL);

		/* don't bother with inserting the default case */
		if (pcabIndex->iHeading != iHeadingNone)
			{
			char stChar [2];

			stChar[0] = 1;
			if (pcabIndex->iHeading == iHeadingBlank)
				stChar[1] = chSpace;
			else
				stChar[1] = chIndexLetter;

			AppendSwitchSt(chFldIndexHeading, st, stChar);
			}

		/* if there are any args at all, remove extra space at end */
		if (st[0] != 0)
			st[0]--;

		StToSzInPlace(st);

		fRemoveIndex = fFalse; /* set indirectly by CmdInsFltSz... */
		caT = selCur.ca;
		vwFieldCalc |= fclSpecialFunc;
		cmd = CmdInsFltSzAtSelCur(fltIndex, st[0] ? st : NULL, 
				bcmIndex, fTrue /* fCalc */, fFalse /* fShowDefault */, 
				fTrue /* fShowResult */);
		vwFieldCalc &= ~fclSpecialFunc;

		if (fRemoveIndex)  /* no index entries were found; get rid of field */
			{
			caT.cpLim = selCur.cpLim;
			AssureLegalSel(&caT);
			FDelete(&caT);
			SetUndoAfter(&selCur.ca);
			cmd = cmdError;
			}
		PushInsSelCur();
		}

LRet:
	EndLongOp(fFalse);
	return cmd;
}


/* C M D  I N D E X  E N T R Y */
/* Insert index entry field into document */

/* %%Function:CmdIndexEntry %%Owner:rosiep */
CMD CmdIndexEntry(pcmb)
CMB * pcmb;
{
	CABINDEXENTRY * pcabIndexEntry;
	CHAR stEntry[cchMaxEntry];
	struct FVB fvb;
	struct CR rgcr[ccrArgumentFetch];
	int cmd = cmdOK;

	if (FCmdFillCab())
		{
		stEntry[0] = 0;
		if (!selCur.fIns)
			{
			char stT[cchMaxSuggest+3];
			InitFvb(&fvb);
			InitFvbBufs(&fvb, stT+1, cchMaxSuggest, rgcr,
					ccrArgumentFetch);
			fvb.doc = selCur.doc;
			fvb.cpFirst = selCur.cpFirst;
			fvb.cpLim = selCur.cpLim;
			FetchVisibleRgch(&fvb, wwCur /*fvc*/, fFalse, fFalse);
			stT[0] = fvb.cch;
			ValidatePotentialXe(stT, stEntry);
			}


		FSetCabSt(pcmb->hcab,stEntry, 
				Iag(CABINDEXENTRY, hstIndexEntry));
		FSetCabSt(pcmb->hcab, stEmpty, 
				Iag(CABINDEXENTRY, hstXeRange));

		pcabIndexEntry = (CABINDEXENTRY *) *pcmb->hcab;
		pcabIndexEntry->fBold = fFalse;
		pcabIndexEntry->fItalic = fFalse;
		/* fMemFail will be true if FSetCabSt fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;

		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltIndexEntry)];

		BltDlt(dltIndexEntry, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		CP cpFirst;
		struct CHP chp;
		/* room for at most: "entry \b \i \r range" */
		CHAR st [cchMaxEntry + cchBkmkMax + 10];
		CHAR stRange [cchBkmkMax + 1];

		/* not repeatable, yet we do want to blow away the again state */
		SetAgain(bcmNil);

		GetCabStz(pcmb->hcab, stEntry, sizeof (stEntry), 
				Iag(CABINDEXENTRY, hstIndexEntry));
		GetCabStz(pcmb->hcab, stRange, sizeof (stRange), 
				Iag(CABINDEXENTRY, hstXeRange));

		st[0] = 0;
		AppendSwitchSt(chNil, st, stEntry);

		if (stRange[0] != 0)
			AppendSwitchSt(chFldXeRange, st, stRange);

		pcabIndexEntry = (CABINDEXENTRY *) *pcmb->hcab;
		if (pcabIndexEntry->fBold)
			AppendSwitchSt(chFldXeBold, st, NULL);

		if (pcabIndexEntry->fItalic)
			AppendSwitchSt(chFldXeItalic, st, NULL);

		st[0]--; /* remove extra space */
		StToSzInPlace(st);

		/* insert after selection so XE ends up on same page as 
			what it's referencing */
		if (!selCur.fIns)
			SelectIns(&selCur, CpMin(selCur.cpLim,
					CpMacDocEdit(selCur.doc)));
		cpFirst = selCur.cpFirst;

		/* last three args are ignored, as this is a dead field */
		cmd = CmdInsFltSzAtSelCur(fltXe, st /* szArgs */, 
				bcmIndexEntry, fFalse, fFalse, fFalse);
		PushInsSelCur();
		}

	return cmd;
}


/* D L G F  I N D E X  E N T R Y */
/* Dialog function for Insert Index Entry dialog */

/* %%Function:FDlgIndexEntry %%Owner:rosiep */
BOOL FDlgIndexEntry(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	switch (dlm)
		{
	case dlmInit:
	case dlmChange:
		if (tmc == tmcIndexEntry || dlm == dlmInit)
			GrayButtonOnBlank(tmcIndexEntry, tmcOK);
		break;
		}

	return fTrue;
}


/* P A R S E  B O O K M A R K */
/* Combo box parse function for Index Entry dialog - combo will display
	all existing bookmarks when dropped down.
*/
/* %%Function:WLBPBookmark %%Owner:rosiep */
WORD WLBPBookmark(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	struct STTB ** hsttb;
	CHAR *pst;
	int ibst = isz;   /* ibst's of bookmarks are same as isz's of combo
			box; use ibst's to make Hungarian happy */

	hsttb = PdodDoc(selCur.doc)->hsttbBkmk;

	if (hsttb == hNil)
		return fFalse;

	switch (tmm)
		{
	case tmmCount:
		return (*hsttb)->ibstMac;

	case tmmText:
		if (hsttb != hNil && ibst < (*hsttb)->ibstMac)
			{
			GetSzFromSttb(hsttb, ibst, sz);
			return fTrue;
			}
		break;
		}

	return fFalse;
}



/* F C R  C A L C  F L T  I N D E X */
/* Field calculation function for Index field.  Scan document for index
	entries (this process builds up a heap tree containing all the info
	about each index entry), then output the index (this process pulls
	the index entries out of the data structure in sorted order and writes
	them to a temporary scratch doc, then copies that doc into the result
	of the index field).
*/

/* %%Function:FcrCalcFltIndex %%Owner:rosiep */
FcrCalcFltIndex(doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;
{
	CHAR stzRange[cchBkmkMax+1];       /* bookmark range of index */
	CHAR stzPartial[cchMaxPartial+1];  /* partial index range */
	char chSwitch;
	CP cpFirst = cpNil, cpLim = cpNil;
	CHAR *st;
	int istError;
	int fcr = fcrNormal;
	struct IIB iib;

	piib = &iib;   /* set up global pointer */
	iib.docMain = doc;
	if (iib.docMain != DocMother(doc))
		{
		CchInsertFieldError(doc, cpResult, istErrSubDocIndex);
		return fcrError;
		}

	/* make sure our special nfc's are unique */
	Assert (nfcIndexMin > nfcLast);

/* reset defaults in case field switches have changed since last refresh */
	iib.fRunin = fFalse;
	iib.istErrIndex = istNil;
	iib.fSequence = fFalse;
	iib.fPartial = fFalse;
	iib.iHeading = iHeadingNone;
	CopySt(StSharedKey(", ",IndexEntrySep), iib.stEntrySep);
	iib.stListSep[0] = 1;
	iib.stListSep[1] = vitr.chList;
	CopySt(StSharedKey("-",IndexSeqSep), iib.stSeqSep);
	CopySt(StSharedKey("-",IndexRangeSep), iib.stRangeSep);

	iib.docIndexScratch = docNil;
	iib.cpIndexScratch = cp0;
	iib.fTable = FInTableDocCp(doc, cpInst);

	/* skip the keyword */
	SkipArgument (pffb);

	/* process switches */

	ExhaustFieldText(pffb);
	while ((chSwitch = ChFetchSwitch(pffb, fFalse /* fSys */)) != chNil)
		{
		switch (chSwitch)
			{
		case chFldIndexRunin:
			piib->fRunin = fTrue;
			break;

		case chFldIndexBookmark:
			InitFvbBufs(&pffb->fvb, stzRange+1, cchBkmkMax, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoBkmkName;
				goto LError;
				}
			/* make an st */
			stzRange[0] = pffb->cch;
			if (!FSearchBookmark (doc, stzRange, &cpFirst, &cpLim, NULL,
					bmcUser))
				{
				istError = istErrBkmkNotDef;
				goto LError;
				}
			break;

		case chFldIndexSeqname:
			piib->fSequence = fTrue;
			InitFvbBufs(&pffb->fvb, piib->szSequence, cchSequenceIdMax, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoSeqSpecified;
				goto LError;
				}
			break;

		case chFldIndexEntrySep:
			st = piib->stEntrySep;
			goto LGetSep;

		case chFldIndexListSep:
			st = piib->stListSep;
			goto LGetSep;

		case chFldIndexSeqSep:
			st = piib->stSeqSep;
			goto LGetSep;

		case chFldIndexRangeSep:
			st = piib->stRangeSep;

LGetSep:
			InitFvbBufs(&pffb->fvb, st /* sz */, cchMaxSeparator, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoSwitchArgument;
				goto LError;
				}
			/* make an st */
			SzToStInPlace(st);
			break;


		case chFldIndexHeading:
			InitFvbBufs(&pffb->fvb, piib->stHeading + 1, cchMaxHeading, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoSwitchArgument;
				goto LError;
				}
			/* make an st */
			piib->stHeading[0] = pffb->cch;
			piib->iHeading = WParseHeading(piib->stHeading);
			break;

		case chFldIndexPartial:
			piib->fPartial = fTrue;
			InitFvbBufs(&pffb->fvb, stzPartial + 1, cchMaxPartial, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				istError = istErrNoSwitchArgument;
				goto LError;
				}
			/* make an st */
			stzPartial[0] = pffb->cch;
			if (!FParsePartialRange(stzPartial, &piib->chMinPartial,
					&piib->chMaxPartial))
				{
				istError = istErrInvalidCharRange;
				goto LError;
				}

			break;

#ifdef ENABLE
		case chFldIndexConcord:
			break;
#endif /* ENABLE */
			}

		}

	if (!FIndexInit())
		return fcrKeepOld;

	if (FScanDocForEntries(iib.docMain, cpFirst, cpLim, sdeIndex))
		if (!FOutputIndex(doc, cpResult, pffb->cpField))
			fcr = fcrKeepOld;

	FreeIndexMem();
	Debug(vdbs.fCkHeap ? CkHeap() : 0);
	if (piib->docIndexScratch != docNil)
		ReleaseDocScratch();

	return fcr;

LError:
	CchInsertFieldError (doc, cpResult, istError);
	return fcrError;
}


/* F  S C A N  D O C  F O R  F L T */
/* Look for first occurrence of flt in document */

/* %%Function:FScanDocForFlt %%Owner:rosiep */
BOOL FScanDocForFlt(pca, flt)
struct CA *pca;
int flt;
{
	struct PLC **hplcfld;
	int ifld, ifldMac;
	int doc = pca->doc;
	struct FLD fld;
#ifdef DEBUG    
	CP cpT;
#endif /* DEBUG */

	if (doc == docNil || (hplcfld = PdodDoc (doc)->hplcfld) == hNil)
		return fFalse;

	ifldMac = IMacPlc(hplcfld);

	for (ifld = 0; ifld < ifldMac; ifld++)
		{
		GetPlc( hplcfld, ifld, &fld );
		if (fld.ch == chFieldBegin)
			{
			Assert (fld.flt != fltNil);
			if (fld.flt == flt)
				{
				PcaSet( pca, doc, CpPlc( hplcfld, ifld ),
						CpPlc( hplcfld, ifld + 1));
				Debug(cpT = pca->cpFirst);
				ExpandToSameLiveField (pca);
				Assert(cpT == pca->cpFirst);
				return fTrue;
				}
			}
		}

	return fFalse;
}


/* F  S C A N  D O C  F O R  E N T R I E S */
/* Repaginate the given doc, then scan it, one page at a time,
	for index or toc entries.  Returns false if user aborted, true
	otherwise.  This function can be called recursively by
	FindEntries if a RefDoc (RD) field is encountered in the scan
	for index entry (XE) or toc entry (TC) fields.  But we do not
	allow more than one level of recursion.

	Index or TOC may have a range imposed on it (through the \b switch
	and a given bookmark).  If this is the case, [cpFirst, cpLim)
	will describe that range.  If not, these two will be cpNil.
	If they are cpNil, we scan the whole doc, otherwise we limit
	our scan to that range.
*/

/* %%Function:FScanDocForEntries %%Owner:rosiep */
BOOL FScanDocForEntries(doc, cpFirst, cpLim, sde)
int doc;
CP cpFirst, cpLim;    /* range of index */
int sde;      /* Scan Doc for Entries code:
				sdeIndex:    look for XE fields
				sdeToc:      look for TC fields
			sdeOutline:  look for outline levels for TOC. */

{
	static int cRecurse = 0;
	struct PLC **hplcpgd;
	struct CA caPage;
	int ipgd;
	BOOL fRange = (cpFirst != cpNil);  /* index/toc only covers a range */
	struct RPL rpl;
	BOOL fNoUserAbort = fTrue;
	struct PPR **hppr;

	if (cRecurse > 1)
		return fTrue; /* fTrue: continue the scan... */

	SetWords(&rpl, pgnMax, cwRPL);
/* if index/toc covers only a range of text, only repaginate what we must */
	rpl.cp = fRange ? cpLim : cpMax;
	if (!FRepaginateDoc(doc, &rpl, patAbort|patmChain))
		{
		if (vmerr.fPrintEmerg)
			return fTrue;   /* no printer installed, report error later */
		else
			return fFalse;  /* user aborted */
		}

	hppr = HpprStartProgressReport (sde == sdeIndex ? mstScanningIndex :
			mstBuildingToc, NULL, 1, fTrue);

	hplcpgd = PdodDoc(doc)->hplcpgd;
	caPage.doc = doc;

	cRecurse++;
	for (ipgd = 0; ipgd < IMacPlc(hplcpgd); ipgd++)
		{
		caPage.cpFirst = CpPlc( hplcpgd, ipgd );
		caPage.cpLim = CpPlc( hplcpgd, ipgd + 1);

		if (caPage.cpFirst >= CpMacDoc(doc))
			break;

		/* if index/toc has a range, limit scan for entries to that range */
		if (fRange)
			if (caPage.cpFirst > cpLim)
				break;
			else  if (caPage.cpLim < cpFirst)
				continue;
			else
				{
				caPage.cpFirst = CpMax(caPage.cpFirst, cpFirst);
				caPage.cpLim = CpMin(caPage.cpLim, cpLim);
				}

		ChangeProgressReport (hppr, ipgd+1);

		if (FQueryAbortCheck() || !FFindEntries(&caPage, ipgd, sde))
			{
			fNoUserAbort = fFalse;
			break;
			}
		}
	cRecurse--;

	return fNoUserAbort;
}



/* F  F I N D  E N T R I E S */
/* Scan a page of text for index entry (XE) or table of contents entry (TC)
	fields.  If we run into a RefDoc (RD) field along the way, take care of
	it (this will involve one level of recursion into FScanDocForEntries.
	That function handles assuring that it is indeed nested only once).
	Return false if aborted, true otherwise.
*/

/* %%Function:FFindEntries %%Owner:rosiep */
BOOL FFindEntries(pca, ipgd, sde)
struct CA *pca;
int ipgd;
int sde;
{
	uns pgn;
	int doc = pca->doc;
	BOOL fAbort = fFalse;
	struct FFB ffb;
	uns nfc;
	int flt, rd, istError;
	struct PGD pgd;

	GetPlc( PdodDoc(doc)->hplcpgd, ipgd, &pgd );
	pgn = pgd.pgn;

	/* this can happen with facing pages */
	if (pca->cpFirst == pca->cpLim) /* this page intentionally left blank */
		return fTrue;

	CacheSect(doc, pca->cpFirst);
	nfc = vsepFetch.nfcPgn;

	InitFvb(&ffb.fvb);

	if (sde == sdeOutline)
		if (!FFindTocOutline(pca, pgn, nfc))
			return fFalse;

	while ((flt = FltScanForDeadField(&ffb, pca)) != fltNil)
		{
		if (FQueryAbortCheck() || vmerr.fMemFail || vmerr.fDiskFail)
			return fFalse;

		switch (flt)
			{
		default:    /* probably an unknown keyword; ignore */
			break;
		case fltXe:
			if (sde == sdeIndex && !FProcessIndexEntry(&ffb, doc, pgn, nfc))
				return fFalse;
			break;
		case fltTce:
			if (sde == sdeToc && !FProcessTocEntry(&ffb, doc, pgn, nfc))
				return fFalse;
			break;
		case fltRefDoc:
			if ((rd = RDProcessRefDoc(&ffb, sde, &istError)) == rdError)
				{
				if (sde == sdeIndex)
					/* flag for later and continue scanning */
					piib->pgnError = pgn;
				else  if (!FTocEntryError(doc, pgn, istError, nfc))
					return fFalse;
				}
			else  if (rd == rdAbort)
				return fFalse;
			break;
			}
		}

	return fTrue;
}



/* R D  P R O C E S S  R E F  D O C */
/* Process RD (RefDoc) field.  Open the referenced doc and scan for
	index/toc entries.  Returns rd (RefDoc) return code as follows:
		rdError:  an error occurred which needs to be reported to user
		rdAbort:  user aborted or we need to abort due to some internal error
		rdNormal: anything else; caller continues processing normally
*/

/* %%Function:RDProcessRefDoc %%Owner:rosiep */
int RDProcessRefDoc(pffb, sde, pist)
struct FFB *pffb;
int sde;   /* sdeIndex, sdeToc, or sdeOutline */
int *pist;
{
	CHAR stzDoc[cchMaxFile+1];
	int doc;
	int rd = rdNormal;

	/* skip keyword */
	SkipArgument(pffb);

	InitFvbBufs(&pffb->fvb, stzDoc + 1, cchMaxFile, NULL, 0);
	if (!FFetchArgText(pffb, fTrue))
		{
		if (sde != sdeIndex)
			{
			*pist = istErrNoFileRD;
			return rdError;
			}
		/* will only report the first such error found */
		if (piib->istErrIndex == istNil)
			{
			piib->istErrIndex = istErrNoFileRD;
			return rdError;
			}
		else
			return rdNormal;
		}

	/* make an st */
	stzDoc[0] = pffb->cch;

	if ((doc = DocOpenStDof(stzDoc, dofNoWindow | dofNoErrors, NULL)) == docNil)
		{
		if (sde != sdeIndex)
			{
			*pist = istErrBadFileRD;
			return rdError;
			}
		/* will only report the first such error found */
		if (piib->istErrIndex == istNil)
			{
			piib->istErrIndex = istErrBadFileRD;
			return rdError;
			}
		else
			return rdNormal;
		}
	else  if (doc == pffb->doc)
		{
		if (sde != sdeIndex)
			{
			*pist = istErrSelfRefRD;
			return rdError;
			}
		/* will only report the first such error found */
		if (piib->istErrIndex == istNil)
			{
			piib->istErrIndex = istErrSelfRefRD;
			return rdError;
			}
		else
			return rdNormal;
		}

	/* pass [cpNil, cpNil) for the range so we scan the whole doc */
	if (!FScanDocForEntries(doc, cpNil, cpNil, sde))
		rd = rdAbort;

	DisposeDoc(doc);

	return rd;
}





/* F  O U T P U T  I N D E X */
/* Compact the entries in the hash table and insert a sorted index in the
	appropriate place.  Sorting is achieved through creating a heap tree out
	of the hash table entries and removing elements from it.  Return true
	if we actually put something in the result.
*/

/* %%Function:FOutputIndex %%Owner:rosiep */
BOOL FOutputIndex(doc, cpIndex, cpField)
int doc;        /* doc where index will be inserted */
CP cpIndex;     /* cp where index will go */
CP cpField;     /* first cp of INDEX field */
{
	int ww;
	struct WWD *pwwd;
	int docInd, clev;
	CP  cp = cp0;
	HPIDB *rghpidb;
	int ihte, ihteLast;
	HPIDB hpidb;
	int cch;
	CHAR chT;
	struct CHP chp;
	struct PAP pap;
	struct SEL sel;
	char rgch[2];
	HPIDB rghpidbLevel[cLevels];
/* declared int on purpose so I can have an invalid byte value;
	this has to deal with unsigned char so chNil won't do it */
	int chBeginLast = 256;
	int fRet = fFalse;
	char *pchT;
	struct CA caDest, caSrc;

/*  Display Building Index... */
	HpprStartProgressReport(mstBuildingIndex, NULL, 1, fTrue);

/*  Compact the hash table  */

	FreezeHp();
	rghpidb = &(HpidbFromIhte(0));  /* Assumes no heap mvmt for compaction */
	ihteLast = ihteBuckets-1;

	while (rghpidb[ihteLast] == hpNil)
		ihteLast--;

	for (ihte = 0; ihte < ihteLast; ihte++)
		{
		if (rghpidb[ihte] == hpNil)
			{
			rghpidb[ihte] = rghpidb[ihteLast];
			while (rghpidb[--ihteLast] == hpNil)
				;
			}
		}

	MeltHp();

/*  Check if there are any entries.  If so, go ahead and output
	them.  If not then report an error and return  */

	if (HpidbFromIhte(0) == hpNil)
		{
		if (vwFieldCalc & fclSpecialFunc)
			{
			/* CBT will guarantee that there are entries, and they
				can't deal with a "no printers installed" message */
			if (!vhwndCBT)
				{
				ErrorEid(vmerr.fPrintEmerg ? eidCantRepag : eidNoEntries,
					"FOutputIndex");
				}
			fRemoveIndex = fTrue;
			return fFalse;
			}
		else
			{
			CchInsertFieldError(doc, cpIndex, vmerr.fPrintEmerg ?
				istErrCantRepag : istErrNoIndexEntries);
			return fTrue;
			}
		}

/*  Now take the compacted hash table and make a heap-tree  */

	ConstructHeap(++ihteLast);

/*  Use a temporary document to build up the index in. */

	if ((docInd = DocCreate(fnNil)) == docNil)
		{
		ErrorEid(eidCantCreateTemp, "FOutputIndex");
		return fFalse;
		}

	rghpidbLevel[0] = hpNil;

/*  Now repeatedly remove an HPIDB from the heap-tree and  */
/*  put it into the index.  Once ConstructHeap was done,  */
/*  HpidbFirstFromHeap takes care of the sorting  */

	while ((hpidb = HpidbFirstFromHeap(&ihteLast)) != hpNil)
		{
		/* check if new letter of alphabet */
		if (piib->iHeading != iHeadingNone &&
				(chT = ChLower(hpidb->rgch[0])) != chBeginLast)
			{
			cp = CpOutputHeading(docInd, cp, chT);
			}
		chBeginLast = chT;
		cp = CpOutputEntry(hpidb, 0, cp, docInd);
		if (vmerr.fMemFail || vmerr.fDiskFail)
			goto LRet;

		/* The problem now is to output all sublevel entries non-recursively */
		clev = 1;
		rghpidbLevel[1] = hpidb->hpidbSub;
		for (;;)
			{
			if (clev <= 0 || rghpidbLevel[clev] == hpNil)
				break;
			/*  Output an entry  */
			cp = CpOutputEntry(rghpidbLevel[clev], clev, cp, docInd);
			if (vmerr.fMemFail || vmerr.fDiskFail)
				goto LRet;

			/*  Update info about the current level  */
			if (clev+1 < cLevels)
				rghpidbLevel[clev+1] = rghpidbLevel[clev]->hpidbSub;
			rghpidbLevel[clev] = rghpidbLevel[clev]->hpidbNext;

			/*  If there are subentries, work on them  */
			if (clev + 1 < cLevels && rghpidbLevel[clev+1] != hpNil)
				++clev;

			/*  If there are no more at the current clev  */
			/*  then pop up a level  */
			else  if (rghpidbLevel[clev] == hpNil)
				while (--clev > 0 && rghpidbLevel[clev] == hpNil)
					;       /*  do nothing (already DEC'ed)  */
			Assert((clev >= 1 && clev < cLevels) || rghpidbLevel[clev] == hpNil);

			if (FQueryAbortCheck())
				goto LRet;
			}
		}

	if (piib->fRunin || piib->istErrIndex != istNil)
		{
		cch = 0;
		if (piib->istErrIndex != istNil)
			{
			pchT = rgch;
			cp += (CP)CchInsertFieldError(docInd, cp, piib->istErrIndex);
			cch += CchIntToPpch(piib->pgnError, &pchT);
			}

#ifdef CRLF
		rgch[cch++] = chReturn;
#endif /* CRLF */
		rgch[cch++] = chEol;

		MapStc(PdodDoc(doc), stcIndex1, &chp, &pap);
		pap.fInTable = piib->fTable;
		if (!FInsertRgch(docInd, cp, rgch, cch, &chp, &pap))
			goto LRet;

		cp += cch;
		}


	if (selCur.doc == doc)
		TurnOffSel(&selCur);

	/*  Make sure Index starts a new paragraph */
	if (!FEnsureNewPara(doc, &cpIndex, cpField))
		goto LRet;

	if (!FReplaceCps(PcaPoint(&caDest, doc, cpIndex), 
			PcaSet(&caSrc, docInd, cp0, cp)))
		goto LRet;

	/*  remove any bookmarks that might have been copied over */
	ScratchBkmks(&caSrc);

	DirtyOutline(doc);
	fRet = fTrue;

LRet:
	DisposeDoc(docInd);           /*  clean things up  */

	return fRet;
}


/* C P  O U T P U T  E N T R Y */
/* output an index entry to the scratch file  */

/* %%Function:CpOutputEntry %%Owner:rosiep */
CP CpOutputEntry(hpidb, ilev, cp, docInd)
HPIDB hpidb;
int     ilev;
CP      cp;
int     docInd;
{
	int     cch, fRange;
	HPPNB   hppnb;
	int     ifpgn, pgnPrev = -1;
	int     fBold, fItalic;
	uns     pgn, grpf;
	int     nfc, nfcPrev, fFirstPgn;
	int     dch;
	char    *pch;
	int ich;
	char rgch[cchMaxEntry + 2];
	char rgchRange[cchMaxNum * 2 + 1];
	char rgb[5];
	struct CA ca;
	struct CHP chp;
	struct PAP pap;
	int wSequence;
	static BOOL fPgnForLast;

	Debug(--vcEntries);

	cch = hpidb->cch;

	if (cch == 0 && ilev != 0)
		return cp;

/*  Take care of the formatting  */
	if (!FGetIndexChpPap(docInd, cp, (piib->fRunin) ? stcIndex1 :
			max(stcIndexMin, stcIndex1 - ilev), &chp, &pap))
		return cpNil;

	if (piib->fRunin)
		if (ilev > 0)
			{
			rgch[0] = (ilev == 1 && !fPgnForLast) ? ':' : ';';
			rgch[1] = ' ';
			if (!FInsertRgch(docInd, cp, rgch, 2, &chp, 0))
				return cpNil;
			cp += 2;
			}
		else  if (cp > cp0 && !piib->fHeadingLast)
			{
			if (!FInsertRgch(docInd, cp, rgchEop, cchEop, &chp, &pap))
				return cpNil;
			cp += cchEop;
			}

	piib->fHeadingLast = fFalse; /* last thing output was an entry, not a heading */
	fPgnForLast = fFalse;  /* so far, no references for this level */
	bltbh(hpidb->rgch, rgch, min(hpidb->cch, cchMaxEntry-1));
	if (vmerr.fMemFail || vmerr.fDiskFail ||
			!FInsertRgch(docInd, cp, rgch, hpidb->cch, &chp, 0))
		return cpNil;
	ca.doc = docInd;
	cp += hpidb->cch;

	cch = CchCopyStRgch(piib->stEntrySep, rgch);

	fFirstPgn = fTrue;
	for (hppnb = &hpidb->pnb; ; hppnb = hppnb->hppnbNext)
		{
		grpf = hppnb->grpf;
		if (hppnb->rgfpgn[0].nfc == nfcExplicit)
			{
			struct EXT HUGE *hpext = hppnb;
			struct CA caSrc;
			if (!fFirstPgn)
				cch += CchCopyStRgch(piib->stListSep, rgch+cch);
			if (!FInsertRgch(docInd, cp, rgch, cch, &chp, 0))
				return cpNil;

			cp += (CP)cch;
			cch = 0;

			if (!FReplaceCps(PcaPoint( &ca, docInd, cp),
					PcaSet( &caSrc, hpext->doc, hpext->cp, hpext->cp + hpext->cch)))
				return cpNil;

			cp += (CP) hpext->cch;
			ca.cpLim = cp;
			pch = rgb;
			if (grpf & 1)
				{
				*pch++ = sprmCFBold;
				*pch++ = fTrue;
				}
			if (grpf & 2)
				{
				*pch++ = sprmCFItalic;
				*pch++ = fTrue;
				}
			grpf >>= 2;
			if (pch != rgb)
				ApplyGrpprlCa(rgb, pch - rgb, &ca);
			fPgnForLast = fTrue;
			fFirstPgn = fFalse;
			}
		else
			{
			for (ifpgn = 0; ifpgn < ifpgnMax; ifpgn++, pgnPrev = pgn, fFirstPgn = fFalse)
				{
				if ((pgn = hppnb->rgfpgn[ifpgn].pgn) == fpgnNil)
					break;
				fPgnForLast = fTrue;
				nfc = hppnb->rgfpgn[ifpgn].nfc;

				if (!fFirstPgn)
					{
					if (nfc != nfcRange)
						cch += CchCopyStRgch(piib->stListSep, rgch+cch);
					else  if (pgn == pgnPrev)
						{
						grpf >>= 2;
						continue;
						}
					else
						{
						cch += CchCopyStRgch(piib->stRangeSep, rgch+cch);
						if (nfc != nfcArabic && nfc != nfcError)
							nfc = nfcPrev;
						else
							{
							dch = CchInclusiveNums(pgnPrev, pgn, rgchRange, &ich);
							/* skip hyphen, it's there already */
							if ((dch -= ++ich) + cch >= cchMaxEntry)
								{
								if (!FInsertRgch(docInd, cp, rgch, cch, &chp, 0))
									return cpNil;
								cp += (CP)cch;
								cch = 0;
								}
							}
						}
					}

				if (nfc == nfcError)
					{
					if (!FInsertRgch(docInd, cp, rgch, cch, &chp, 0))
						return cpNil;
					cp += (CP)cch;
					cp += (CP)CchInsertFieldError(docInd, cp,
							istErrInvalidBkmkXE);
					cch = 0;
					nfc = (fFirstPgn ? nfcArabic : nfcPrev);
					}

				/* the two shifts of grpf are not combined like above
					because fTrue must equal 1 for fBold and fItalic */
				fBold = grpf & 1;
				grpf >>= 1;
				fItalic = grpf & 1;
				grpf >>= 1;
				if (fBold || fItalic)
					{
					/* insert plain text before bold or italic */
					if (!FInsertRgch(docInd, cp, rgch, cch, &chp, 0))
						return cpNil;
					cp += (CP)cch;
					cch = 0;
					chp.fBold = fBold;
					chp.fItalic = fItalic;
					}

				if (piib->fSequence)
					{
					char *pchT;

					wSequence = hppnb->rgwSequence[ifpgn];
					if (nfc == nfcArabic || wSequence != 0)
						{
						pchT = rgch+cch;
						cch += CchIntToPpch(wSequence, &pchT);
						cch += CchCopyStRgch(piib->stSeqSep, rgch+cch);
						}
					}

				if (nfc == nfcRange)
					{
					bltb(&rgchRange[ich], &rgch[cch], dch);
					nfc = nfcPrev;
					}
				else
					while ((dch = CchLongToRgchNfc((LONG)pgn, &rgch[cch],
							nfc, cchMaxEntry-cch-1)) == 0)
						{
						if (!FInsertRgch(docInd, cp, rgch, cch, &chp, 0))
							return cpNil;
						cp += (CP)cch;
						cch = 0;
						}
				cch += dch;
				if (fBold || fItalic)
					{
					/* insert formatted text */
					if (!FInsertRgch(docInd, cp, rgch, cch, &chp, 0))
						return cpNil;
					cp += (CP)cch;
					cch = 0;
					chp.fBold = chp.fItalic = fFalse;
					}
				nfcPrev = nfc;
				}
			}

		if (hppnb->hppnbNext == hpNil)
			break;
		}

/* if no page numbers existed, overwrite the separator  */
	if (!fPgnForLast)
		cch = 0;
	if (!piib->fRunin)
		{
#ifdef CRLF
		rgch[cch++] = chReturn;
#endif /* CRLF */
		rgch[cch++] = chEol;
		}
	if (cch)
		if (!FInsertRgch(docInd, cp, rgch, cch, &chp, (piib->fRunin) ? 0 : &pap))
			return cpNil;

	return cp + (CP)cch;
}


/* C P  O U T P U T  H E A D I N G */
/* Output the heading letter string or blank line between letters
	of the alphabet.  Use stHeading as a template.  Replace all
	occurrences of 'A' (special value we stuck in there for all
	alpha chars) with ch passed in.
*/

/* %%Function:CpOutputHeading %%Owner:rosiep */
CP CpOutputHeading(doc, cp, ch)
int doc;
CP cp;
CHAR ch;
{
	struct PAP pap;
	struct CHP chp;
	CHAR rgch[cchMaxHeading+4];
	int cch = 0;

	piib->fHeadingLast = fTrue;

	if (piib->fRunin && cp > cp0)
		{
		if (!FGetIndexChpPap(doc, cp, stcIndex1, &chp, &pap))
			return cpNil;
		if (!FInsertRgch(doc, cp, rgchEop, cchEop, &chp, &pap))
			return cpNil;
		cp += cchEop;
		}

	if (!FGetIndexChpPap(doc, cp, stcIndexHeading, &chp, &pap))
		return cpNil;

	if (piib->iHeading != iHeadingBlank)
		{
		for (cch = 0; cch < piib->stHeading[0]; cch++)
			{
			rgch[cch] = piib->stHeading[cch+1];
			if (rgch[cch] == chIndexLetter)
				rgch[cch] = ChUpper(ch);
			}
		}
#ifdef CRLF
	rgch[cch++] = chReturn;
#endif /* CRLF */
	rgch[cch++] = chEol;

	if (!FInsertRgch(doc, cp, rgch, cch, &chp, &pap))
		return cpNil;

	return cp + (CP)cch;
}



/*  ------------------------------------------------------------------   */
/*  A heap-tree is defined as a binary tree where every node is smaller  */
/*  than its two children.  Such a tree is maintained here in an array   */
/*  on the heap by knowing that node K's children are at 2*k+1 and 2*k+2 */
/*  (these are different from the textbook versions because this array   */
/*  uses zero-based subscripting)   The heap-tree is kept balanced by    */
/*  the removal routine HpidbFirstFromHeap()                             */
/*                                                                       */
/*  The heap construction routine is non-recursive, and the max number   */
/*  of comparisons needed for a heap of height K (of size N=2^K-1) is    */
/*                                                                       */
/*  by recursion    A(k) = 2 * A(k-1) + 2 * (k-1)                        */
/*  or iteratively  A(k) = summation from 1 to (k-1) of ((2^i) * (k-i))  */
/*  or by observation A(k) = A(k-1) + 2^k - 2  =  A(k-1) + n - 1         */
/*  ------------------------------------------------------------------   */

/* 
/* C O N S T R U C T  H E A P */
/* Heap construction routine: given ihteMax make a heap-tree with its head
	at the beginning of the hash table.
*/

/* %%Function:ConstructHeap %%Owner:rosiep */
ConstructHeap(ihteMax)
int ihteMax;
{
	int ihteScan;

	for (ihteScan = ihteMax / 2 + 1; ihteScan >= 0; --ihteScan)
		FilterDownHeap(ihteScan, ihteMax);
}



/* H P I D B  F I R S T  F R O M  H E A P */
/* Remove the first element from the heap tree and fix up the tree
	accordingly.  NOTE:  This routine assumes that there is no heap movement
	during the execution of the routine.  If heap movement is added, use
	HpidbFromIhte() instead of rghpidb[]
*/

/* %%Function:HpidbFirstFromHeap %%Owner:rosiep */
HPIDB HpidbFirstFromHeap(pihteMax)
int *pihteMax;
{
	HPIDB hpidbFirst, hpidbNext, *rghpidb;

/*  Treat the heap area as an array (assumes no heap movement)  */
	rghpidb = &(HpidbFromIhte(0));

	if (*pihteMax == 0)
		return hpNil;

	hpidbFirst = rghpidb[0];
	if ((hpidbNext = hpidbFirst->hpidbNext) != hpNil)
		rghpidb[0] = hpidbNext;
	else
		rghpidb[0] = rghpidb[--*pihteMax];

	FilterDownHeap(0, *pihteMax);

	return hpidbFirst;
}


/* F I L T E R  D O W N  H E A P */
/* This routine will "filter down" the HPIDB at ihteHead until it is in its
	proper place in the heap tree, by swapping it with the minimum of its kids
	as long as the minimum kid is less than the parent.  NOTE:  This routine
	assumes no heap movement by using rghpidb */

FilterDownHeap(ihteScan, ihteMax)      /*  (ihteScan is the head)  */
int ihteScan, ihteMax;
{
	HPIDB hpidbScan, hpidbMinKid, hpidbKid1, hpidbKid2;
	int ihteMinKid, ihteKid1, ihteKid2;
	HPIDB *rghpidb;
	BOOL    fHeapOk = fFalse;

/*  assumes no heap movement  */
	rghpidb = &(HpidbFromIhte(0));

	while (!fHeapOk)
		{
		fHeapOk = fTrue;
		ihteKid2 = (ihteKid1 = 2 * ihteScan+1) + 1;
		hpidbScan = rghpidb[ihteScan];

		/*  Check the special case of a node with no kids  */
		if (ihteKid1 >= ihteMax)
			return;

		hpidbKid1 = rghpidb[ihteKid1];
		hpidbKid2 = rghpidb[ihteKid2];

		/*  Check for a node with one kid  */
		if (ihteKid2 >= ihteMax)
			{
			ihteMinKid = ihteKid1;
			hpidbMinKid = hpidbKid1;
			}

		/*  Find the minimum kid  */
		else  if (WCompRgchIndex(hpidbKid1->rgch, hpidbKid1->cch,
				hpidbKid2->rgch, hpidbKid2->cch) > 0)
			{
			ihteMinKid = ihteKid2;
			hpidbMinKid = hpidbKid2;
			}
		else
			{
			ihteMinKid = ihteKid1;
			hpidbMinKid = hpidbKid1;
			}

		/*  Compare the minimum kid with the parent  */
		if (WCompRgchIndex(hpidbScan->rgch, hpidbScan->cch,
				hpidbMinKid->rgch, hpidbMinKid->cch) > 0)
			{
			rghpidb[ihteScan] = hpidbMinKid;
			rghpidb[ihteScan = ihteMinKid] = hpidbScan;
			fHeapOk = fFalse;
			}
		}
}


/* C C H  I N C L U S I V E  N U M S */
/* Plan:
		we assume that 0 < w1 < w2 < 10000

		if (w1 < 100 or w1 % 100 == 0)
			use all of w2's digits: e.g. 40-72, 200-214
		else if (w1 % 100 < 10 and w2 / 100 == w1 / 100)
			use only the changed digits of w2: e.g. 102-5
		else if (w1 > 1000 and w1 / 100 % 10 != w2 / 100 % 10)
			use all of w2's digits: e.g. 5492-5705
		else
			use 2 or more of the changed digits of w2: e.g. 321-25, 1917-64

		This conforms to Chicago Manual of Style as long as
			0 < w1 < w2 < 10,000  (5 digits adds another rule)
*/

/* %%Function:CchInclusiveNums %%Owner:rosiep */
CchInclusiveNums(w1, w2, pchTo, pich)
int w1, w2;
char *pchTo;    /* string returned in here */
int *pich;      /* returned ich of hyphen */
{
	int cch1, cch2;
	char *pch1 = pchTo, *pch2;
	int wTens, wHundreds1, wHundreds2;
	int cchMin = 0;

	Assert(w1 > 0);
	Assert(w1 < w2);
	Assert(w2 < 10000);
	*pich = cch1 = CchIntToPpch(w1, &pch1);
	*pch1++ = '-';
	cch2 = CchIntToPpch(w2, &pch1);
	if (w1 >= 100 && (wTens = w1 % 100) != 0)
		if (wTens < 10 && (wHundreds1 = w1 / 100) == (wHundreds2 = w2 / 100))
			cchMin = 1;
		else  if (w1 <= 1000 || wHundreds1 % 10 == wHundreds2 % 10)
			cchMin = 2;
	if (cchMin)
		{
		pch1 = pchTo + cch1 - cchMin;      /* end of 1st number */
		pch2 = pch1 + cch2 + 1;         /* end of 2nd number */
		for (; cchMin < cch2; cchMin++, pch1--, pch2--)
			if (*pch1 == *pch2)
				{
				bltb(pch2 + 1, pchTo + cch1 + 1, cch2 = cchMin);
				break;
				}
		}
	return(cch1 + cch2 + 1);
}




/* A P P E N D  S W I T C H  S T */
/* Used for building up st to insert field into document, this function
	appends the switch and its argument (if it has one) to the st being
	constructed.  Caller is responsible for making sure st is big enough
	to have stArg appended to it (realizing that stArg could possibly
	grow if it contains quotes, whitespace, and/or field escape chars).
	Asserts in the caller to this effect would be a good idea.  Note that
	this function assumes stArg does not already have quotes around it.
	Any quotes that are already in the string will be escaped with the
	field escape character so they will remain as part of the argument.
*/

/* %%Function:AppendSwitchSt %%Owner:rosiep */
AppendSwitchSt(chSwitch, st, stArg)
int chSwitch;  /* must be declared int because of a compiler bogosity
		in which the comparison below will compare FFh to FFFFh
		if chSwitch is a char or CHAR */
CHAR *st, *stArg;
{
	CHAR *pch = &st[st[0]+1];   /* next available slot in st */
	CHAR *pchArg;
	int cchArg = stArg[0];
	int cch = 0;

	/* append the switch character */
	if (chSwitch != chNil)
		{
		*pch++ = chFieldEscape;
		*pch++ = chSwitch;
		*pch++ = chSpace;
		cch += 3;
		}

	/* append the argument, if any */
	if (stArg != NULL)
		{
		/* quote it */
		*pch++ = chGroupExternal;
		cch++;

		/* copy the arg, escaping quotes and escape chars */
		for (pchArg = &stArg[1]; cchArg; cch++, cchArg--)
			{
			if (*pchArg == chFieldEscape || *pchArg == chGroupExternal)
				{
				*pch++ = chFieldEscape;
				cch++;
				}
			*pch++ = *pchArg++;
			}

		*pch++ = chGroupExternal;
		*pch++ = chSpace;   /* to separate from other switches */
		cch += 2;
		}

	st[0] += cch;
}


/* V A L I D A T E  P O T E N T I A L  X E */
/* The user has a selection when they bring up the Index Entry dialog.
	ValidatePotentialXe's job is to determine whether that selection
	should be stuffed into the dialog's edit control, and if so, escape
	any colons with backslashes to avoid giving them special meaning,
	and truncate the suggested entry if any non-displayable character
	is encountered (field characters, CRLF's, etc)
*/
/* %%Function:ValidatePotentialXe %%Owner:rosiep */
ValidatePotentialXe(stFetch, stEntry)
CHAR *stFetch;
CHAR *stEntry;
{
	char *pch1, *pch2;
	int ich;

	stEntry[0] = stFetch[0];
	pch1 = stFetch + 1;
	pch2 = stEntry + 1;
	for (ich = 1; ich <= stFetch[0]; ich++)
		{
		if (*pch1 == chColon)
			{
			*pch2++ = chFieldEscape;
			stEntry[0]++;
			}
		else  if (!FDisplayableCh(*pch1))
			{
			stEntry[0] = pch2 - stEntry - 1;
			break;
			}
		*pch2++ = *pch1++;
		}
}


/* F  D I S P L A Y A B L E  C H */
/* Returns TRUE if Windows can display this character, FALSE if it
	will show up as a solid rectangle.  FranzR from International
	assures me this is guaranteed to work, even for int'l versions.
	Windows does not provide an equivalent function. -- rp  */

/* %%Function:FDisplayableCh %%Owner:rosiep */
FDisplayableCh(ch)
CHAR ch;
{
	/* unprintable ANSI characters */
	if (ch < 0x20 || (uns)(ch - 0x7F) <= (0x9f - 0x7f)
			|| ch == 0xd7 || ch == 0xf7)
		return fFalse;

	return fTrue;
}



/* C C H  C O P Y  S T  R G C H */
/* %%Function:CchCopyStRgch %%Owner:rosiep */
int CchCopyStRgch(st, rgch)
CHAR *st, *rgch;
{
	int cch = st[0];

	bltb(&st[1], rgch, cch);
	return cch;
}



/* F  P A R S E  P A R T I A L  R A N G E */
/* Parse the argument to the \p switch of the index field to get the
	range of a partial index.  Return true iff the range is valid.
	A valid range is one which contains at least two non-whitespace
	characters such that the first and last such characters are in
	increasing order.  E.g. "a-m", "am", "arm", "a - m", and "17" are
	valid but "a" and "y-p" are not valid.
*/

/* %%Function:FParsePartialRange %%Owner:rosiep */
BOOL FParsePartialRange(st, pchMin, pchMax)
CHAR *st, *pchMin, *pchMax;
{
	int ich, ichMin;

	for (ich = 1; ich <= st[0]; ich++)
		if (!FWhite(st[ich]))
			{
			*pchMin = ChLower(st[ich]);
			ichMin = ich;
			break;
			}

	if (ich > st[0])
		return fFalse;

	for (ich = st[0]; ich > ichMin; ich--)
		if (!FWhite(st[ich]))
			{
			*pchMax = ChLower(st[ich]);
			break;
			}

	return (ich > ichMin && *pchMax >= *pchMin);
}


/* W  P A R S E  H E A D I N G */
/* Parse the argument to the \h switch of the index field (the
	heading letter string).  There are three possible return
	values, based on the contents of the string:

		null string         iHeadingNone
		all whitespace      iHeadingBlank
		anything else       iHeadingLetter

	If there are any alpha characters in the string, they will
	be replaced with 'A' to make the task of building the actual
	string to be inserted in the document easier later.
*/

/* %%Function:WParseHeadings %%Owner:rosiep */
int WParseHeading(st)
CHAR *st;
{
	int ich;
	int wT = iHeadingBlank;

	if (st[0] == 0)
		return iHeadingNone;

	for (ich = 1; ich <= st[0]; ich++)
		{
		if (!FWhite(st[ich]))
			{
			wT = iHeadingLetter;
			if (FAlpha(st[ich]))
				st[ich] = chIndexLetter;
			}
		}

	return wT;
}



/* C C H  S K I P  S P A C E */
/*  Given a ppch and a cchMax, skip white space at **pch,
	return # skipped.
*/
/* %%Function:CchSkipSpace %%Owner:rosiep */
int CchSkipSpace(ppch, cchMax)
char  **ppch;
int  cchMax;
{
	int     ch, cch;

	cch = 0;
	while (((ch = **ppch) == chSpace || ch == chTab || ch == chSect
			|| ch == chTable) && cch < cchMax)
		{
		++(*ppch);
		++cch;
		}
	return cch;
}


/* F  G E T  I N D E X  C H P  P A P */
/* Get chp and pap for text that's going to be output to index temp doc */
/* returns fFalse if failed. */

/* %%Function:FGetIndexChpPap %%Owner:rosiep */
BOOL FGetIndexChpPap(doc, cp, stc, pchp, ppap)
int doc, stc;
CP cp;
struct CHP *pchp;
struct PAP *ppap;
{
	int cstcStd;

	if (!FEnsureStcDefined(piib->docMain, stc, &cstcStd))
		return fFalse;
	MapStc(PdodDoc(piib->docMain), stc, pchp, ppap);
	ppap->fInTable = piib->fTable;
	CachePara(doc, cp);
	*pchp = vchpStc;
	return fTrue;
}



/*  F  E N S U R E  N E W  P A R A  */
/*  Make sure INDEX or TOC field starts a new paragraph */
/*  returns fFalse if failed. */

/* %%Function:FEnsureNewPara %%Owner:rosiep */
BOOL FEnsureNewPara(doc, pcpResult, cpField)
int doc;
CP *pcpResult;    /* cp where index/toc will go */
CP cpField;     /* first cp of field */
{
	CachePara(doc, *pcpResult);
	if (caPara.cpFirst != cpField)
		{
		if (!FInsertRgch(doc, *pcpResult, rgchEop, cchEop, &vchpStc, &vpapFetch))
			return fFalse;
		*pcpResult += cchEop;
		}
	return fTrue;
}
