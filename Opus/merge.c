/* M E R G E . C */
/*  Functions to implement Print Merge */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "sel.h"
#include "field.h"
#include "doc.h"
#include "prompt.h"
#include "message.h"
#include "print.h"
#include "disp.h"
#include "ch.h"
#include "file.h"
#include "prm.h"
#include "doslib.h"
#include "debug.h"
#include "error.h"

/* specify which descriptions we want */
#define PRINTMERGE

#include "idd.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "printmrg.hs"
#include "printmrg.sdm"

#include "print.hs"



#ifdef PROTOTYPE
#include "merge.cpt"
#endif /* PROTOTYPE */

extern struct SEL       selCur;
extern CHAR             szEmpty[];
extern struct CA        caAdjust;
extern struct PMD       *vppmd;
extern struct SEP       vsepFetch;
extern struct MERR      vmerr;
extern struct CA        caSect;
extern struct PRI       vpri;
extern int              vlm;
extern int              vflm;
extern BOOL		    fElActive;

struct PMD      *vppmd = NULL;
char viFormLetter = 0;

/* iMergeRec values */
#define iMergeAll   0
#define iMergeRange 1




/* C M D  P R I N T  M E R G E */
/*  Execute Print Merge for the current document.  Dialog gives user choice
	of records to perform operation over and the option of printing directly
	or sending the result to a single large document.
*/

/* %%Function:CmdPrintMerge %%Owner:PETERJ */
CMD CmdPrintMerge(pcmb)
CMB * pcmb;
{
	int doc;
	int docResult;
	BOOL fPrint;
	struct SELS sels;
	CABPRINTMERGE *pcabPM;
	CHAR stResult [ichMaxFile];
	CHAR stPrFrom [20], stPrTo [20];

	doc = DocMother(selCur.doc);
	fPrint = fTrue;

	if (doc == docNil)
		{
		Assert (fFalse);
		return cmdError;
		}

	if (FCmdFillCab())
		{
		Assert(pcmb->hcab != hNil);
		pcabPM = *pcmb->hcab;
		pcabPM->iMergeRec = iMergeAll;
		pcabPM->iRecFrom = uNinch;
		pcabPM->iRecTo = uNinch;
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltPrintMerge)];

		BltDlt(dltPrintMerge, dlt);

		/*  bring up dialog */
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;
			}
		}

	if (FCmdAction())
		{
		int iRecFrom = 0;
		int iRecTo = iNil;

		switch (pcmb->tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcOK:
		case tmcPMPrint:
			fPrint = fTrue;
			break;

		case tmcPMNewDoc:
			fPrint = fFalse;
			break;
			}

		StartLongOp ();

		/* get the merge range */
		pcabPM = *pcmb->hcab;
		if (pcabPM->iMergeRec != iMergeAll)
			{
			int w;
			CHAR szRange [10];

			Assert (pcabPM->iMergeRec == iMergeRange);

			if ((w = pcabPM->iRecFrom) != wNinch && w > 0 && w < 0x7ff0)
				iRecFrom = w - 1;

			if ((w = pcabPM->iRecTo) != wNinch && w > 0 && w < 0x7ff0)
				iRecTo = w - 1;
			}

		/* get the destination */
		if (fPrint)
			/* Bring up the PRINT dialog to get print options set up */
			{
			int cmdRet;
			HCABPRINT hcabprint;
			CMB cmb;

			if (!FPrinterOK())
				{
				ErrorEid(eidNoPrinter, " CmdPrintMerge");
				cmdRet = cmdError;
				goto LPrintRet;
				}

			if ((hcabprint = HcabAlloc(cabiCABPRINT)) == hNil)
				{
				cmdRet = cmdError;
				goto LPrintRet;
				}

			cmb.hcab = hcabprint;
			cmb.cmm = cmmNormal;
			cmb.pv = NULL;
			cmb.bcm = bcmNil;

			if (TmcDoPrintDialog (&cmb,
					fTrue/*fPrintMerge*/, 
					fFalse/*fFileFind*/) != tmcOK)
				{
				cmdRet = cmdCancelled;
				Assert (hcabprint != hNil);
				FreeCab(hcabprint);
LPrintRet:
				EndLongOp (fFalse);
				return cmdRet;
				}

			FillPrsu(hcabprint, stPrFrom, stPrTo, 20);
			FreeCab(hcabprint);
			docResult = docNil;
			}
		else  /* !fPrint */			
			{
			CHAR *pst;
			struct STTB **hsttbSrc, **hsttbDest;
			CHAR stAuthor [cchMaxSz];

			/* create the result doc */
			docResult = DocCloneDocNoText (doc, dkDoc, fTrue);
			if (docResult == docNil)
				{
LFailed:
				DisposeDoc (docResult);
				EndLongOp (fFalse);
				return cmdNoMemory;
				}

				/* give it a name */
				{
				struct DOD *pdod = PdodDoc(docResult);
				pdod->udt = udtFormLetter;
				pdod->iInstance = ++viFormLetter;
				}

			ApplyDocMgmtNew(docResult, fnNil);
			if ((hsttbSrc = PdodDoc(doc)->hsttbAssoc) != hNil)
				{
				/* make doc's author docResult's author 
					(will have been made current user by 
					ApplyDocMgmtNew). */
				if ((hsttbDest = HsttbAssocEnsure(docResult)) == hNil)
					goto LFailed;

				GetStFromSttb(hsttbSrc, ibstAssocAuthor,
						stAuthor);
				if (!FChangeStInSttb(hsttbDest, ibstAssocAuthor, 
						stAuthor))
					goto LFailed;
				}
			sels.ca.doc = docNil;
			if (!FCreateMw (docResult, wkDoc, &sels, NULL))
				{
				ErrorEid(eidPMCantOpenWind, " CmdPrintMerge 3");
				goto LFailed;
				}
			}

		/*  do the actual print merge */
		DoPrintMerge(doc, docResult, iRecFrom, iRecTo, stPrFrom, stPrTo);

		EndLongOp(fFalse);
		}

	return cmdOK;
}



/* F D L G  P R I N T  M E R G E */
/*  Dialog function for Print Merge dialog.
*/

/* %%Function:FDlgPrintMerge %%Owner:PETERJ */
BOOL FDlgPrintMerge(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	switch (dlm)
		{
	case dlmInit:
		if (!FPrinterOK())
			{
			EnableTmc(tmcPMPrint, fFalse);
			SetFocusTmc(tmcPMNewDoc);
			}
		if (fElActive)
			EnableTmc(tmcPMPrint, fFalse);
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcPMRange:
			SetFocusTmc(tmcPMFrom);
			break;

		case tmcPMAll:
			SetTmcVal(tmcPMFrom, uNinch);
			SetTmcVal(tmcPMTo, uNinch);
			break;
			}
		break;

	case dlmChange:
		switch (tmc)
			{
		case tmcPMFrom:
		case tmcPMTo:
			SetTmcVal(tmcPMMergeRec, iMergeRange);
			break;
			}
		break;
		}

	return fTrue;
}




/* D O  P R I N T  M E R G E */
/*  Perform a print merge over doc.  Start with record iFirst and continue
	until iLast (end of docData if iNil) has been done or error.  If
	!docResult send each result to Print (Print settings already made).
	If docResult != docNil append each result to a new document along with
	a "next page" type section break.
*/

/* %%Function:DoPrintMerge %%Owner:PETERJ */
DoPrintMerge(doc, docResult, iFirst, iLast, stPrFrom, stPrTo)
int doc, docResult;
int iFirst, iLast;
CHAR *stPrFrom, *stPrTo;

{

	int cbGrpprlCleanup = 0;
	CHAR grpprlCleanup [7];
	struct PMD pmd;
	struct PPR **hppr = hNil;
	int lmSave, flmSave;
#ifdef DEBUG
	int iPrev = -1;
#endif /* DEBUG */

	StartLongOp ();

	/* if they entered TO, FROM backwards, correct them */
	if (iLast >= 0 && iFirst >= 0 && iLast < iFirst)
		{
		int iT;
		iT = iFirst;
		iFirst = iLast;
		iLast = iT;
		}

	/*  Cannot undo a print merge */
	SetUndoNil();
	ClearFDifferFltg (doc, fltgAll);
	Assert (doc == DocMother (doc));
	Assert (iFirst >= 0);
	vppmd = &pmd;
	SetBytes (&pmd, 0, sizeof (struct PMD));
	pmd.iRec = iFirst;
	/* pmd.docData = docNil; IE*/
	/* pmd.fAbortPass = fFalse; IE*/
	pmd.fFirstPass = fTrue;
	/* pmd.fEndMerge = fFalse; IE*/
	/* pmd.hsttbHeader = hNil; IE*/
	/* pmd.cpData = cp0; IE*/

	DisableInval();

	if (docResult != docNil)
		/*  Merge to document.  Set up source document to make copying easy! */
		{
		struct SEP sep;
		struct CA ca;
		CP cpDest;
		int cch;
		CHAR grpprl [7];
		CHAR *pch = grpprl, *pchCleanup = grpprlCleanup;

		/* make the first section of the document: 1) "new page" if
			not new, odd or even; 2) restart at page one if not already
			restart.  These changes are undone when we are through.
		*/
		CacheSectSedMac (doc, cp0);
		PcaSet( &ca, doc, CpMin (caSect.cpLim-1, CpMacDoc(doc)),
				caSect.cpLim );
		caSect.doc = docNil; /* inval because SedMac used */
		Assert (sprmSBkc < sprmSFPgnRestart &&
				sprmSFPgnRestart < sprmSPgnStart);
		switch (vsepFetch.bkc)
			{
		case bkcNewPage:
		case bkcEvenPage:
		case bkcOddPage:
			break;
		default:
			/*  make section have "new page" property */
			*pch++ = sprmSBkc;
			*pch++ = bkcNewPage;
			/* setup to undo */
			*pchCleanup++ = sprmSBkc;
			*pchCleanup++ = vsepFetch.bkc;
			break;
			}
		if (!vsepFetch.fPgnRestart)
			{
			int w = 1;
			/* change the doc */
			*pch++ = sprmSFPgnRestart;
			*pch++ = 1;
			*pch++ = sprmSPgnStart;
			blt (&w, pch, sizeof(int));
			pch += sizeof(int);
			/* set up to undo change */
			*pchCleanup++ = sprmSFPgnRestart;
			*pchCleanup++ = 0;
			*pchCleanup++ = sprmSPgnStart;
			blt (&vsepFetch.pgnStart, pchCleanup, sizeof(int));
			pchCleanup += sizeof(int);
			}
		if ((cch = pch - grpprl) > 0)
			{
			ApplyGrpprlCa (grpprl, cch, &ca);
			caSect.doc = docNil; /* inval props */
			cbGrpprlCleanup = pchCleanup - grpprlCleanup;
			}
		Assert (cch <= sizeof(grpprl) &&
				cbGrpprlCleanup <= sizeof(grpprlCleanup));

		/* must append a section break to the source document so that
			we can get a hold of the section properties when we copy later.
		*/
		CacheSect (doc, cpDest = CpMacDocEdit(doc));
		bltb (&vsepFetch, &sep, sizeof (struct SEP));
		CmdInsertSect1(PcaPoint( &ca, doc, cpDest), &sep, NULL /*pchp*/,
				NULL /* ppap */, fTrue /* fSepNext */, fFalse /* fRM */ );
		}

	else
		/*  Merge to printer, start the print job */
		{
		lmSave = vlm;
		flmSave = vflm;
		if (!FBeginPrintJob (doc, lmSave, flmSave))
			goto LPrintJobEnd;
		}

		/* put up progress report */
		{
		int iT = pmd.iRec + 1;
		hppr = HpprStartProgressReport(mstMergingRecord, &iT, 1, fTrue);
		}

	/*  do each printmerge pass */
	while (!pmd.fEndMerge && (iLast == iNil || pmd.iRec <= iLast)
			&& pmd.iRec < 0x7ff0)
		{
		int cT, docT;

		/*  must make progress during each pass */
		Assert (iPrev < pmd.iRec);
		Debug (iPrev = pmd.iRec);

		if (FQueryAbortCheck () || vmerr.fMemFail || vmerr.fDiskFail)
			/* cancel out */
			break;

		/*  put up progress report */
		ChangeProgressReport (hppr, pmd.iRec+1);

		/* Refresh the fields */
		AcquireCaAdjust ();

		/*  main document */
		if (FSetPcaForCalc (&caAdjust, doc, cp0, CpMacDoc (doc), &cT))
			FCalcFields (&caAdjust, frmMerge, fFalse, fTrue);

		/*  header document */
		if ((docT = PdodDoc (doc)->docHdr) != docNil)
			if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
				FCalcFields (&caAdjust, frmMerge, fFalse, fTrue);

		/*  footnote document */
		if ((docT = PdodDoc (doc)->docFtn) != docNil)
			if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
				FCalcFields (&caAdjust, frmMerge, fFalse, fTrue);

		/*  annotation document */
		if ((docT = PdodDoc (doc)->docAtn) != docNil)
			if (FSetPcaForCalc (&caAdjust, docT, cp0, CpMacDoc (docT), &cT))
				FCalcFields (&caAdjust, frmMerge, fFalse, fTrue);

		ReleaseCaAdjust ();

		if (FQueryAbortCheck () || vmerr.fMemFail || vmerr.fDiskFail)
			/* cancel out */
			break;

		/*  do something with the result */
		if (!pmd.fAbortPass)
			{
			if (docResult != docNil)
				{
				struct CA caDest;
				struct CA caSrc;

				/* copy the source (except final EOP) to the result */
				if (FReplaceCps( PcaPoint( &caDest, docResult, CpMacDocEdit(docResult) ),
						PcaSet( &caSrc, doc, cp0, CpMacDocEdit(doc) )))
					CopyStylesFonts(caSrc.doc, caDest.doc, caDest.cpFirst,
							caSrc.cpLim - caSrc.cpFirst );

				/* remove all bookmarks */
				caDest.cpLim = CpMacDoc(docResult);
				ScratchBkmks (&caDest);

				/* dereference all fields in the result except frmPaginate
					and frmPrint and display fields. */
				RemoveFieldsFromDoc (docResult, fTrue);
				RemoveFieldsFromDoc (PdodDoc(docResult)->docFtn, fTrue);
				RemoveFieldsFromDoc (PdodDoc(docResult)->docAtn, fTrue);
				RemoveFieldsFromDoc (PdodDoc(docResult)->docHdr, fFalse);

				/* compress piece table before it gets too big */
				CheckCompressDoc(docResult);
				}
			else
				{
				/* print it */
				Assert (doc == DocMother(selCur.doc));
				DoPrint (doc, stPrFrom, stPrTo, fTrue/*fWholeDoc*/, fFalse);
				if (vpri.fPrErr)
					goto LPrintJobEnd;
				}
			}

		if (pmd.docData == docNil)
			{
			/* no data, so we are done */
			break;
			}
		pmd.fAbortPass = fFalse;
		pmd.fFirstPass = fFalse;

		} /* while() */

	if (docResult != docNil)
		/*  Merge was to a file, clean up source and destination */
		{
		struct CA caT;
		/* remove the section mark we added (in the source) */
		PcaPoint(&caT, doc, CpMacDocEdit(doc));
		caT.cpFirst--;
		FDelete(&caT);

		/* undo any changes to first section of source */
		if (cbGrpprlCleanup > 0)
			{
			CacheSectSedMac(doc, cp0);
			caT.cpFirst = CpMin(caSect.cpLim-1, CpMacDoc(doc));
			caT.cpLim = caSect.cpLim;
			caSect.doc = docNil; /* because SedMac used */
			ApplyGrpprlCa(grpprlCleanup, cbGrpprlCleanup, &caT);
			caT.cpFirst = cp0;
			InvalCp(&caT);
			}

		/* make sure we have dirtied result */
		DirtyDoc (docResult);

		/* make the section of the final EOP not force a page break */
		CacheSectSedMac(docResult, CpMacDocEdit(docResult));
		if (vsepFetch.bkc != bkcNoBreak)
			{
			CHAR grpprl[7];
			CHAR *pch = grpprl;
			*pch++ = sprmSBkc;
			*pch++ = bkcNoBreak;
			Assert (caSect.cpLim == CpMac1Doc(docResult));
			ApplyGrpprlCa(grpprl, pch - grpprl, 
					PcaSetDcp(&caT, docResult, caSect.cpLim-ccpEop, ccpEop));
			}
		}
	else
		/* Merge to Print, end the print job */
		{
LPrintJobEnd:
		EndPrintJob (lmSave, flmSave);
		}

		/* take care of invalidation */
		{
		struct CA caT;
		EnableInval();
		if (docResult != docNil)
			InvalCp(PcaSetWholeDoc(&caT, docResult));
		InvalCp(PcaSetWholeDoc(&caT, doc));
		}

	StopProgressReport(hppr, pdcRestoreImmed);
	if (vmerr.fMemFail)
		/* something failed report "incomplete", not failure */
		SetErrorMat(matLow);
	if (pmd.docData != docNil)
		{
		DisablePreload();
		DisposeDoc (pmd.docData);
		}
	FreeHsttb (pmd.hsttbHeader);
	vppmd = NULL;
	EndLongOp (fFalse);
}




/* F C R  C A L C  F L T  P M  N E X T  D A T A */
/*  Bring in the next data record.  If fltPMNextIf first evaluate cond-expr
	and read in data record only if true. If fltPMData and docData has not
	been set up, get the filenames and setup docData and hsttbHeader.

	Formats:
		fltPMData       {data datafile [headerfile]}
		fltPMNext)      {next}
		fltPMNextIf     {nextif cond-expr}
*/

/* %%Function:FcrCalcFltPMNextData %%Owner:PETERJ */
FcrCalcFltPMNextData(doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{

	int eid = eidNil;

	Assert (vppmd != NULL);

	if (IfldEncloseIfld (doc, ifld) != ifldNil)
		/* field is enclosed in another! */
		{
		eid = eidPMFieldNested;
		goto LErrRet;
		}

	if (doc != DocMother (doc))
		/* field is in a sub doc */
		{
		eid = eidPMFieldInSubDoc;
		goto LErrRet;
		}

	SkipArgument (pffb); /* skip keyword */

	if (vppmd->docData == docNil)
		/* data file has not been initialized */
		{
		CHAR stData [ichMaxFile+1];
		CHAR stHeader [ichMaxFile+1];

		if (flt != fltPMData)
			/* data field must come before NEXT, NEXTIF */
			{
			eid = eidPMDataNotFirstPM;
			goto LErrRet;
			}

		/*  get datafile */
		InitFvbBufs (&pffb->fvb, stData+1, ichMaxFile, NULL, 0);
		FFetchArgText (pffb, fTrue/*fTruncate*/);
		if (! (stData [0] = pffb->cch))
			/* no datafile given */
			{
			eid = eidPMNoDataFileSpec;
			goto LErrRet;
			}

		/*  get headerfile, if any */
		InitFvbBufs (&pffb->fvb, stHeader+1, ichMaxFile, NULL, 0);
		FFetchArgText (pffb, fTrue/*fTruncate*/);
		stHeader [0] = pffb->cch;

		if (!FInitMergeData (stData, stHeader) ||
				!FSkipDataRecs (vppmd->iRec))
			/* cannot read data or header file, error already reported */
			goto LErrRet;
		}

	/* see if this is a conditional next */
	if (flt == fltPMNextIf)
		/* evaluate the condition */
		{
		int istErr;
		BOOL fCondition;
		if (!FEvalFieldCondition (pffb, &istErr, &fCondition))
			/* condition not well formed */
			{
			eid = eidPMBadFieldCond;
			goto LErrRet;
			}
		if (!fCondition)
			/*  this field does nothing */
			return fcrNormal;
		}

	if (!FGetNextMergeData (doc, cpResult))
		goto LErrRet;

	return fcrNormal;

LErrRet:
	if (eid != eidNil)
		ErrorEid (eid, " FcrCalcFltPMNextData");
	vppmd->fEndMerge = fTrue;
	ForceAbortCheck (); /* simulates user abort */
	return fcrError;
}






/* F C R  C A L C  F L T  P M  S K I P I F */
/*  Evaluate cond-exp.  If true abort the current print-merge record.
	PrintMerge will continue from the top with the next record.
	Format:  {skipif cond-expr}
*/

/* %%Function:FcrCalcFltPMSkipIf %%Owner:PETERJ */
FcrCalcFltPMSkipIf(doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int eid = eid;
	int istErr;
	BOOL fCondition;

	Assert (vppmd != NULL);

	if (IfldEncloseIfld (doc, ifld) != ifldNil)
		/* field is enclosed in another! */
		{
		eid = eidPMFieldNested;
		goto LErrRet;
		}

	if (doc != DocMother (doc))
		/* field is in a sub doc */
		{
		eid = eidPMFieldInSubDoc;
		goto LErrRet;
		}

	if (vppmd->docData == docNil)
		/* data file has not been initialized--must come before SKIPIF */
		{
		eid = eidPMDataNotFirstPM;
		goto LErrRet;
		}

	SkipArgument (pffb); /* skip keyword */

	if (!FEvalFieldCondition (pffb, &istErr, &fCondition))
		/* condition not well formed */
		{
		eid = eidPMBadFieldCond;
		goto LErrRet;
		}

	if (fCondition)
		/*  abort iff condition was true */
		vppmd->fAbortPass = fTrue;

	return fcrNormal;

LErrRet:
	if (eid != eidNil)
		ErrorEid(eid, " FcrCalcFltPMSkipIf");
	vppmd->fEndMerge = fTrue;
	ForceAbortCheck (); /* simulates user abort */
	return fcrError;
}




/* F C R  C A L C  F L T  P M  R E C */
/*  Return the number of the current Print-Merge record.
	Format:  {mergerec}
*/

/* %%Function:FcrCalcFltPMRec %%Owner:PETERJ */
FcrCalcFltPMRec(doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int cch;
	CHAR *pch;
	int fcr;
	CHAR rgch [cchMaxInt];
	struct CHP chp;

	Assert (vppmd != NULL);

	pch = rgch;
	cch = CchIntToPpch (vppmd->iRec, &pch);
	fcr = FcrRegisterIntResult (vppmd->iRec);
	GetResultChp (pffb, &chp);
	return FInsertRgch (doc, cpResult, rgch, cch, &chp, NULL) ?
			fcr : fcrError;
}




/* F  I N I T  M E R G E  D A T A */
/*  Open stData as a doc for use as the data document for printmerge.  Set
	docData and cpData in *vppmd.  If stHeader is not empty it is a file
	which contains the bookmark names to use.  Otherwise use the first
	paragraph of stData (and advance cpData).  Build hsttbHeader (in *vppmd)
	with the names of the bookmarks to be inserted.  In case of problems,
	report the error and return false.
*/

/* %%Function:FInitMergeData %%Owner:PETERJ */
FInitMergeData(stData, stHeader)
CHAR *stData, *stHeader;

{
	int eid = eidNil;
	int docData = docNil, docHeader = docNil;
	struct STTB **hsttb = hNil;
	CHAR rgch [cchBkmkMax+1];
	struct FFB ffb;

	/*  open data document */
	Assert (stData != NULL && *stData);
	if ((docData = DocOpenStDof (stData, dofBackground, NULL)) == docNil)
		{
		eid = eidPMNoOpenDataFile;
		goto LErrRet;
		}

	/*  open header document, if any */
	if (stHeader != NULL && *stHeader)
		{
		if ((docHeader = DocOpenStDof (stHeader, dofBackground, NULL)) == docNil)
			{
			eid = eidPMNoOpenHeadFile;
			goto LErrRet;
			}
		}
	else
		docHeader = docNil;

	/*  build hsttb up with the bookmark names to use */
	InitFvb (&ffb.fvb);
	SetFfbPrintMergeText (&ffb, docHeader == docNil ? docData : docHeader,
			cp0);
	InitFvbBufs (&ffb.fvb, rgch+1, cchBkmkMax, NULL, 0);

	if ((hsttb = HsttbInit (0, fTrue/*fExt*/)) == hNil)
		goto LErrRet;

	while (FFetchArgText (&ffb, fTrue/*fTruncate*/))
		/*  get each bookmark name and put it into the sttb */
		{
		rgch [0] = ffb.cch;
		if (!FLegalBkmkName (rgch))
			{
			int i = vppmd->iRec + 1;
			Assert (i > 0);
			IdMessageBoxMstRgwMb (mstPMIllegalBkmk, &i,
					MB_OK | MB_ICONASTERISK);
			goto LErrRet;
			}
		IbstAddStToSttb (hsttb, rgch);
		if (vmerr.fMemFail)
			goto LErrRet;
		}

	if (docHeader != docNil)
		{
		DisposeDoc (docHeader);
		vppmd->cpData = cp0;
		}
	else
		vppmd->cpData = ffb.cpFirst;

	Assert(vppmd->hsttbHeader == hNil && vppmd->docData == docNil);
	vppmd->hsttbHeader = hsttb;
	vppmd->docData = docData;

	if (vppmd->cpData >= CpMacDoc (docData))
		/*  there is no data */
		vppmd->fEndMerge = fTrue;

	EnablePreload(PdodDoc(docData)->fn);

	return fTrue;

LErrRet:
	if (eid != eidNil)
		ErrorEid (eid, " FInitMergeData");
	FreeHsttb (hsttb);
	DisposeDoc (docData);
	DisposeDoc (docHeader);

	return fFalse;
}



/* F  S K I P  D A T A  R E C S */
/*  Skip i records in vppmd->docData.  Advance vppmd->cpData but do not
	advance vppmd->iRec.
*/

/* %%Function:FSkipDataRecs %%Owner:PETERJ */
FSkipDataRecs(i)
int i;

{
	CP cp = vppmd->cpData;
	struct FFB ffb;

	Assert (vppmd != NULL);
	InitFvb (&ffb.fvb);

	while (i--)
		{
		SetFfbPrintMergeText (&ffb, vppmd->docData, cp);
		if (ffb.cpFirst >= CpMacDoc (vppmd->docData)-ccpEop)
			{
			ErrorEid (eidPMFirstBeyondEnd, " FSkipDataArgs");
			return fFalse;
			}
		ExhaustFieldText (&ffb);
		cp = ffb.cpFirst;
		}

	vppmd->cpData = cp;
	return fTrue;
}




/* F  G E T  N E X T  M E R G E  D A T A */
/*  Get the next record of print merge data from docData (starting at
	cpData).  A record is one paragraph and contains fields separated by
	tabs or commas.  Fields may be quoted and quoted fields may contain
	commas, tabs and paragraph marks.
	Increment iRec in *vppmd unless the end of the
	file has been reached in which case set it to iNil.  Copy each field to
	doc, cp and place a bookmark on it as specified by hsttbHeader (in
	*vppmd).  If there is a mismatch in number of arguments report it to the
	user with the record number.
*/

/* %%Function:FGetNextMergeData %%Owner:PETERJ */
FGetNextMergeData(doc, cp)
int doc;
CP cp;
{
	int ibst, ibstMac;
	BOOL fMismatch = fFalse;
	CP cpOrig = cp;
	struct CA ca;
	CHAR st [cchBkmkMax];
	struct FFB ffb;

	Assert (vppmd != NULL && vppmd->docData != docNil && vppmd->hsttbHeader
			!= hNil);
	Assert (!((vppmd->fEndMerge) ^
			(vppmd->cpData >= CpMacDoc (vppmd->docData))));

	if (!vppmd->fEndMerge)
		{
		InitFvb (&ffb.fvb);
		SetFfbPrintMergeText (&ffb, vppmd->docData, vppmd->cpData);
		}
	else
		fMismatch = fTrue;

	ibst = 0;
	ibstMac = (*vppmd->hsttbHeader)->ibstMac;

	ca.doc = doc;
	while (ibst < ibstMac)
		{
		ca.cpFirst = cp;
		if (!vppmd->fEndMerge)
			{
			cp += DcpCopyArgument (&ffb, doc, cp);
			if (ffb.fNoArg)
				/*  more names then data, insert empty bookmarks */
				fMismatch = fTrue;
			}
		GetStFromSttb(vppmd->hsttbHeader, ibst, st);
		Assert (*st < cchBkmkMax);
		ca.cpLim = cp;
		AssureLegalSel (&ca);
		if (vmerr.fMemFail || !FInsertStBkmk (&ca, st, NULL))
			return fFalse;

		ibst++;
		}

	if (ibst != 0)
		/* force empty bkmks not to be deleted when prev result is deleted */
		{
		/* Can you say Hack? */
		CHAR ch = chSpace;
		struct CHP chp;
		GetPchpDocCpFIns (&chp, doc, cp, fFalse, wwNil);
		if (!FInsertRgch (doc, cp, &ch, 1, &chp, NULL))
			return fFalse;
		}

	if (vppmd->fEndMerge)
		return fTrue;

	if (ffb.cpFirst < ffb.cpLim)
		/*  more data then names, pass over the extra data */
		{
		fMismatch = fTrue;
		ExhaustFieldText (&ffb);
		}

	Assert (ffb.cpFirst == ffb.cpLim);

	if (fMismatch)
		/* Warn the user that there was a mismatch, but may continue */
		{
		int i = vppmd->iRec + 1;
		Assert (i > 0);
		if (IdMessageBoxMstRgwMb (mstPMMismatch, &i,
				MB_YESNO | MB_ICONQUESTION) != IDYES)
			/*  user wants to abort */
			return fFalse;
		}

	/*  check for end of docData (also for 1 more para which is empty) */
	if (ffb.cpFirst >= CpMacDoc (vppmd->docData)-ccpEop)
		{
		vppmd->cpData = CpMacDoc (vppmd->docData);
		vppmd->fEndMerge = fTrue;
		}
	else
		vppmd->cpData = ffb.cpFirst;

	return vppmd->iRec++ < 0x7ff0;
}




/* R E M O V E  F I E L D S  F R O M  D O C */
/*  Dereference fields in doc.  All fields except frmPaginate and frmPrint
	and display types are dereferenced if fAll is true.  If fAll is false, then 
	frmHdrAll is also preserved.
*/

/* %%Function:RemoveFieldsFromDoc %%Owner:PETERJ */
RemoveFieldsFromDoc (doc, fAll)
int doc;
BOOL fAll;

{
	struct PLC **hplcfld;
	int ifld = 0;
	CP dcpInst, dcpResult, cp;
	struct EFLT eflt;
	int frm = frmPaginate | frmPrint | (fAll ? 0 : frmHdrAll);
	struct CA ca;
	struct FLD fld;

	if (doc == docNil || (hplcfld = PdodDoc (doc)->hplcfld) == hNil)
		/*  nothing to do */
		return;

	while (ifld < IMacPlc( hplcfld ))
		{
		GetPlc( hplcfld, ifld, &fld );
		if (fld.ch == chFieldBegin)
			{
			eflt = EfltFromFlt (fld.flt);

			if (!(eflt.frm & frm) && !eflt.fDisplay)
				/*  replace this field with its result */
				{
				cp = CpPlc( hplcfld, ifld );
				CchRmFieldAtDocCp(doc, cp, &dcpInst, &dcpResult);

				if (dcpInst != 0)
					/* kill inst text */
					FDelete( PcaSet( &ca, doc, cp, cp + dcpInst ) );

				if (!eflt.fResult && dcpResult != 0)
					/* not result field--kill result text */
					FDelete( PcaSet( &ca, doc, cp, cp + dcpResult ) );

				/*  load pointer to newly next field */
				continue;
				}
			}
		ifld++;
		}
}




/* S E T  F F B  P R I N T  M E R G E  T E X T */
/*  Set pffb for a fetch of the Print Merge record which starts at doc, cp.
*/

/* %%Function:SetFfbPrintMergeText %%Owner:PETERJ */
SetFfbPrintMergeText (pffb, doc, cp)
struct FFB *pffb;
int doc;
CP cp;

{
	pffb->doc = doc;
	pffb->cpFirst = cp;
	pffb->cpLim = CpMacDoc (doc);
	pffb->fOverflow = fFalse;

	SetBytes (&pffb->fsfb, 0, sizeof (struct FSFB));
	pffb->flt = fltNil;
	pffb->cpField = cpNil;
	pffb->fPMergeRules = fTrue;
}


