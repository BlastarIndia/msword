/* F I E L D C L C . C */
/*  Routines needed to calculate fields. */


#ifdef DEBUG
#ifdef PCJ
/* #define SHOWCALC */
#endif /* PCJ */
#endif /* DEBUG */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "doc.h"
#include "sel.h"
#include "ch.h"
#include "prompt.h"
#include "message.h"
#include "ourmath.h"
#include "format.h"
#include "inter.h"
#include "print.h"
#include "disp.h"
#include "dde.h"
#include "file.h"
#include "strtbl.h"
#define FINDNEXT
#include "search.h"
#include "error.h"

#define FIELDCMD
#include "field.h"   /* includes rgstFldError */

#include "debug.h"

#include "opuscmd.h"


#ifdef PROTOTYPE
#include "fieldclc.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */

extern struct SEL   selCur;
extern struct PREF  vpref;
extern struct CA    caAdjust; /* used while traversing fields during calc */
extern int          docMac;
extern struct CA    caPara;
extern struct CHP   vchpFetch;
extern CHAR         szError[];
extern struct MERR  vmerr;
extern CP           vcpFirstLayout;
extern CP           vcpLimLayout;
extern HCURSOR      vhcArrow;
extern struct DOD **mpdochdod [];
extern struct CA    caSect;
extern int          vfExtendSel;
extern int          vwFieldCalc;
extern union RRU    vrruCalc;
extern CHAR         vrrut;
extern struct PMD   *vppmd;
extern int          wwCur;
extern struct PAP   vpapFetch;
extern char			rgchEop[];
extern struct WWD   **hwwdCur;
extern struct DDES  vddes;
extern BOOL         vfDdeIdle;
extern struct UAB   vuab;
extern ENV	    *penvMathError;
extern int          vdocScratch;


extern CP CpMomFromHdr();


/* C M D  C A L C  S E L */
/*  Calculate all fields within the current selection. */

/* %%Function:CmdCalcSel %%Owner:peterj */
CMD CmdCalcSel (pcmb)
CMB *pcmb;
{
	BOOL fCalc = fFalse;
	BOOL fUndo = fTrue;
	int cFields;

	if (selCur.fBlock)
		{
		Beep();
		return cmdError;
		}

	Profile( vpfi == pfiFieldRef ? StartProf(30) : 0);

	AcquireCaAdjust ();

	if (FSetPcaForCalc (&caAdjust, selCur.doc, selCur.cpFirst,
			selCur.cpLim, &cFields))
		{
		struct DOD *pdod = PdodDoc(selCur.doc);
		if ((pdod->fFtn || pdod->fAtn) && !FCheckLargeEditFtn(&caAdjust, &fUndo))
			goto LCantUndo;
		if (fUndo && !FSetUndoB1(bcmCalcFields, uccPaste, &caAdjust))
			goto LCantUndo;
		StartLongOp ();
		TurnOffSel(&selCur); /* to avoid weirdness on the screen */
		Assert (vwFieldCalc == 0);
		vwFieldCalc = fclSel;
		fCalc = FCalcFields (&caAdjust, frmUser, (cFields>1), fTrue);
		Assert (vwFieldCalc == fclSel);
		vwFieldCalc = 0;
		EndLongOp (fFalse);
		}

	if (!fCalc)
		Beep ();
	else
		{
		DirtyDoc (selCur.doc);
		if (fUndo)
			SetUndoAfter(&caAdjust);
		else
			SetUndoNil();
		}

LCantUndo:
	ReleaseCaAdjust ();
	MakeSelCurVisi(fTrue /*fForceBlockToIp*/);

	Profile( vpfi == pfiFieldRef ? StopProf() : 0);

	return cmdOK;
}




/* F  C A L C  F I E L D S */
/*  Calculate all fields enclosed in *pca.
	frm indicates the types of fields to be calculated.
*/

/* %%Function:FCalcFields %%Owner:peterj */
FCalcFields (pca, frm, fPrompt, fClearDiffer)
struct CA * pca;
int frm;
BOOL fPrompt, fClearDiffer;

{
	int ifld, ifldFirst;
	long ifldNext = 0;
	BOOL fReturn = fFalse;
	int doc = pca->doc;
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct PPR **hppr;

	/*  *pca must be an adjusted ca because the
		calculation function of a field may
		(and, in general, will) add characters to the document.
	*/

	if (doc == docNil || pca->cpFirst >= pca->cpLim || vmerr.fMemFail
			|| vmerr.fDiskFail)
		/*  invalid calculation range. */
		{
		Assert (vmerr.fMemFail || vmerr.fDiskFail);
		return fFalse;
		}

	vwFieldCalc |= fclCa;
	Scribble (ispFieldCalc1, 'C');

	if (fPrompt)
		hppr = HpprStartProgressReport (mstCalcFields, NULL, nIncrPercent, fTrue);

	/*  what field is cpFirst in? */
	ifldFirst = ifld = IfldNextField (doc, pca->cpFirst);
	Assert (ifld != ifldNil);

	/*  if next field exists and is in our range, calculate it.
		Terminate at this level if we are having problems or user ESC. */
	while (ifld != ifldNil
			&& CpFirstField (doc, ifld) < pca->cpLim
			&& !vmerr.fMemFail
			&& !vmerr.fDiskFail
			&& ! FQueryAbortCheck ()
			&& (vppmd == NULL || !vppmd->fAbortPass))
		{
		/*  put up a % complete message */
		if (fPrompt && ifld >= (int)ifldNext)
			ProgressReportPercent (hppr, (long)ifldFirst,
					(long)IInPlcRef (hplcfld, pca->cpLim), (long)ifld,
					&ifldNext);
		fReturn |= FCalcFieldIfld (doc, ifld, frm, 0, fClearDiffer);
		ifld = IfldNextField (doc, CpLimField (doc, ifld));
		}

	Scribble (ispFieldCalc1, ' ');

	if (fPrompt)
		{
		ChangeProgressReport (hppr, 100);
		StopProgressReport (hppr, pdcAdvise);
		}

		/* FIELD CALCULATION STRUCTURES/TERMINATION */
		{
		extern struct STTB **hsttbSequenceCalc;
		if (hsttbSequenceCalc != hNil)
			FreePhsttb (&hsttbSequenceCalc);
		vwFieldCalc &= ~fclCa;
		}

	return fReturn;

}



/* F  C A L C  F I E L D  I F L D */
/*  Calculate field ifld.  First calculate any field enclosed in ifld's
	instruction range.  Then calculate this field.

	NB:  field calculation functions are assumed to be WELL BEHAVED.  A well
	behaved calculation function is one that does not modify any portion of
	a document except its result portion.  IF A CALC FUNCTION MODIFIES ANY
	PORTION OF A DOCUMENT OUTSIDE OF ITS RESULT, THE FOLLOWING TRAVERSAL
	MAY NOT FUNCTION PROPERLY.
*/

/* %%Function:FCalcFieldIfld %%Owner:peterj */
FCalcFieldIfld (doc, ifld, frm, cNestLevel, fClearDiffer)
int doc, ifld, frm, cNestLevel;
BOOL fClearDiffer;

{
	int ifldCalc;
	CP cp = CpFirstField (doc, ifld) + 1;
	struct EFLT eflt;
	BOOL fReturn = fFalse;
	struct FLCD flcd;

	Assert (ifld != ifldNil);
	GetIfldFlcd (doc, ifld, &flcd);
	/*  if we are having technical problems or not allowed to refresh,
		don't go on */
	if (vmerr.fMemFail || vmerr.fDiskFail)
		return fFalse;

	/*  check if we are beyond the nesting limit */
	if (++cNestLevel > cNestFieldMax)
		{
		ErrorEid (eidFields2DeepCalc, "FCalcFieldIfld");
		return fFalse;
		}

	Scribble (ispFieldCalc2, ('0'+cNestLevel-1));

	/*  Calculate fields enclosed in instructions.  Note that CpLimInstField
		for ifld may change. */
	while ((ifldCalc = IfldNextField (doc, cp)) != ifldNil &&
			CpFirstField (doc, ifldCalc) < CpLimInstField (doc, ifld))
		{
		fReturn |= FCalcFieldIfld (doc, ifldCalc, frm, cNestLevel,
				fClearDiffer);
		cp = CpLimField (doc, ifldCalc);
		}

	eflt = EfltFromFlt (flcd.flt);

	/*  Calculate the field itself, if appropriate */
	if (eflt.frm & frm && !flcd.fLocked)
		{
#ifdef SHOWCALC
		CommSzNum(SzShared("FCalcFieldIfld: calculating "), ifld);
#endif /* SHOWCALC */
		fReturn = fTrue;
		if (FInsertFieldSeparator (doc, ifld))
			CallCalcFunc (doc, ifld, fClearDiffer);
		}

	/*  Calculate fields enclosed in result.  Note that CpLimField
		of ifld may change. */
	cp = CpLimInstField (doc, ifld);
	while ((ifldCalc = IfldNextField (doc, cp)) != ifldNil &&
			CpFirstField (doc, ifldCalc) < CpLimField (doc, ifld))
		{
		fReturn |= FCalcFieldIfld (doc, ifldCalc, frm, cNestLevel,
				fClearDiffer);
		cp = CpLimField (doc, ifldCalc);
		}

	Scribble (ispFieldCalc2, (cNestLevel>1 ? '0'+cNestLevel-2 : ' '));

	return fReturn;

}




/* C A L L  C A L C  F U N C */
/*  Call the calculation function for field ifld.
*/

/* %%Function:CallCalcFunc %%Owner:peterj */
CallCalcFunc (doc, ifld, fClearDiffer)
int doc, ifld;

{
	int fcr;
	CP dcpPrevResult;
	BOOL fResultDirty;
	BOOL fResultEdited;
	BOOL fInTablePrev;
	BOOL fFldDirty;
	struct EFLT eflt;
	struct FLCD flcd;
	struct FFB ffb;
	struct CA ca;

	vwFieldCalc |= fclFunc;

	/* some callers are not going through normal command dispatch */
	InvalAgain();

	GetIfldFlcd (doc, ifld, &flcd);
	dcpPrevResult = CpMax (cp0, flcd.dcpResult - 1);
	fResultDirty = flcd.fResultDirty;
	fResultEdited = flcd.fResultEdited;
	fFldDirty = flcd.fDirty;
	eflt = EfltFromFlt (flcd.flt);
	Assert (eflt.pfnCalcFunc != NULL);
	Assert (flcd.dcpResult > 0);
	CachePara(doc, flcd.cpFirst + flcd.dcpInst - 1);
	fInTablePrev = vpapFetch.fInTable;

	/*  initialize the field fetch block */
	InitFvb (&ffb.fvb);
	SetFfbIfld (&ffb, doc, ifld);

	/* call calc function */
	fcr = (*eflt.pfnCalcFunc) (doc, ifld, flcd.flt,
			flcd.cpFirst, flcd.cpFirst+flcd.dcpInst, &ffb);


	/*  if this field is nested within a dead field, its result must
		be vanished. */
	SetFieldResultProps(doc, ifld, fInTablePrev);

	if (fcr >= fcrNormal)
		{
		/*  assure all switches were fetched */
		ExhaustFieldText (&ffb);
		if (!fInTablePrev)
			{
			/* note: can use old cpFirst and dcpInst values--they don't change */
			CachePara(doc, flcd.cpFirst + flcd.dcpInst - 1);
			if (vpapFetch.fInTable)
				{
				struct PAP pap;
				struct CHP chp;

				GetIfldFlcd (doc, ifld, &flcd);
				CachePara(doc, flcd.cpFirst + flcd.dcpInst + flcd.dcpResult - 1);
				pap = vpapFetch;
				Assert(!pap.fInTable);
				StandardChp(&chp);
				chp.fVanish = fFalse;
				FInsertRgch(doc, flcd.cpFirst + flcd.dcpInst, rgchEop,
						(int)ccpEop, &chp, &pap);
				}
			}

		if (ffb.fsfSys.c > 0)
			{
			/* apply the "system" switches to the result */
			GetIfldFlcd (doc, ifld, &flcd);
			DcpApplySysSwitches (&ffb, doc, flcd.cpFirst+flcd.dcpInst,
					flcd.dcpResult-dcpPrevResult-1, dcpPrevResult, fcr);
			}
		}

	if (fcr != fcrKeepOld)
		{
		/* remove old result */
		DeleteFieldResult (doc, ifld, flcd.flt!=fltDdeHot/*fSeparator*/,
				dcpPrevResult);

		/*  no longer dirty */
		fResultDirty = fFalse;
		fResultEdited = fFalse;
		}

	/*  reset the result dirtyness */
	GetIfldFlcd (doc, ifld, &flcd);
	flcd.fResultDirty = fResultDirty;
	flcd.fResultEdited = fResultEdited;
	flcd.fDirty = fFldDirty;
	if (fClearDiffer)
		flcd.fDiffer = fFalse;
	SetFlcdCh (doc, &flcd, 0);
	/*  invalidate fields display */
	PcaSetDcp( &ca, doc, flcd.cpFirst, flcd.dcpInst + flcd.dcpResult );
	CheckInvalCpFirst (&ca);
	InvalCp (&ca);

	vwFieldCalc &= ~fclFunc;
}




/* F  S E T  P C A  F O R  C A L C */
/*  Determines the cp range over which fields will be calculated for
	a given calculation range.
*/

/* %%Function:FSetPcaForCalc %%Owner:peterj */
FSetPcaForCalc (pca, doc, cpFirst, cpLim, pcFields)
struct CA * pca;
int doc;
CP cpFirst, cpLim;
int *pcFields;

{
	int ifld;
	CP cpT, dcpIns = cpFirst==cpLim;
	struct FLCD flcd;

	pca->doc = docNil;
	if (mpdochdod[doc]==hNil)
		return fFalse;

	ifld = IfldFromDocCp (doc, cpFirst, fFalse);

	/*  an insertion point immediately prior to a field should refresh
		the enclosing field, not just the field it preceeds */
	if (ifld != ifldNil && cpFirst == cpLim &&
			CpFirstField (doc, ifld) == cpFirst)
		ifld = IfldEncloseIfld (doc, ifld);

	if (ifld == ifldNil)
		ifld = IfldNextField (doc, cpFirst);

	if (ifld != ifldNil)
		{
		GetIfldFlcd (doc, ifld, &flcd);
		pca->cpFirst = pca->cpLim = flcd.cpFirst;
		}
	else
		return fFalse;

	*pcFields = 0;
	while (ifld != ifldNil && flcd.cpFirst < cpLim+dcpIns)
		{
		if (flcd.fDirty)
			{
			FltParseDocCp (doc, flcd.cpFirst, ifld, fTrue, fFalse);
			ifld = IfldNextField (doc, flcd.cpFirst);
			GetIfldFlcd (doc, ifld, &flcd);
			continue;
			}
		if ((cpT = flcd.cpFirst+flcd.dcpInst+flcd.dcpResult) > pca->cpLim)
			pca->cpLim = cpT;
		*pcFields += 1;
		ifld = IfldAfterFlcd (doc, ifld, &flcd);
		}

	pca->doc = doc;
	AssureLegalSel (pca);
	return pca->cpLim > pca->cpFirst;
}



/* A P P L Y  S Y S  S W I T C H E S */
/*  Apply the "system" switches of a calculated field as a post operation
	on the result.  The size of the result may change.  Will not change
	the previous result in any way.  May be applied to a "result" that is
	not part of any field.
*/

/* %%Function:DcpApplySysSwitches %%Owner:peterj */
CP DcpApplySysSwitches (pffb, doc, cpResult, dcpNew, dcpPrev, fcr)
struct FFB *pffb;
int doc;
CP cpResult, dcpNew, dcpPrev;
int fcr;

{
	CHAR chSw;
	BOOL fPicDefined = fTrue;

	pffb->fGroupInternal = fFalse;

	while ((chSw = ChFetchSwitch (pffb, fTrue /*fSys*/)) != chNil)
		/*  above call sets pffb up to fetch argument */
		switch (chSw)
			{
		case chFldSwSysNumeric:
		case chFldSwSysDateTime:
			if (fPicDefined)
				{
				dcpNew = DcpApplyPictureSw (chSw, pffb, doc, cpResult, 
						dcpNew, fcr);
				/*  picture only meaningful if first formatting switch */
				fPicDefined = fFalse;
				}
			else
				dcpNew = DcpReplaceWithError (doc, cpResult, dcpNew,
						istErrPicSwFirst);
			break;

		case chFldSwSysLock:
			/* lock all fields in the result */
			FSetFieldLocks (fTrue, doc, IfldNextField (doc, cpResult),
					cpResult+dcpNew);
			break;

		case chFldSwSysFormat:
			dcpNew = DcpApplyFormatSw (pffb, doc, cpResult, dcpNew, 
					dcpPrev, fcr);
			/*  picture only meaningful if first formatting switch */
			fPicDefined = fFalse;
			break;

#ifdef DEBUG
		default:
			Assert (fFalse);
			break;
#endif /* DEBUG */
			}
	return dcpNew;
}



/* S E T  F I E L D  R E S U L T  P R O P S */
/* If field is nested within a dead field, make its result vanished.
*  If Revision Marking is on, set result according to field begin char.
*/

/* %%Function:SetFieldResultProps %%Owner:peterj */
SetFieldResultProps(doc, ifld, fInTable)
int doc, ifld;
BOOL fInTable;
{
	struct CA caField, caResult;

	PcaField( &caField, doc, ifld );
	if (DcpCa(PcaFieldResult( &caResult, doc, ifld )) == cp0)
		return;

	FetchCpAndParaCa(&caField, fcmProps);

	if (vchpFetch.fFldVanish)
		ApplyFFldVanish (&caResult, fTrue /* fVanish */);

	if (PdodMother(doc)->dop.fRevMarking)
		ApplyRevMarking(&caResult, vchpFetch.fRMark, vchpFetch.fStrike);

	if (fInTable)
		ApplyTableProps(&caResult, fTrue);
	else  if (FTableMismatch(&caResult))
		ApplyTableProps(&caResult, fFalse);
}



/* G E T  P D O C  P C P  M O T H E R  F R O M  D O C  C P */
/*  For a field being calculated at doc, cp, what is the doc, cp that is
	should consider itself to be at?  (Some fields only are defined in the
	Mother document, if they exist in a sub document they must have a
	reference point in the mother).
*/

/* %%Function:GetPdocPcpMotherFromDocCp %%Owner:peterj */
GetPdocPcpMotherFromDocCp (doc, cp, pdoc, pcp)
int doc, *pdoc;
CP cp, *pcp;

{
	struct DOD * pdod = PdodDoc (doc);
	struct DRP *pdrp;

	*pdoc = DocMother (doc);

	if (*pdoc == doc)
		/*  normal document case */
		*pcp = cp;

	else  if (pdod->fHdr)
		{ /* in header/footer at PRINT time, use cpLast of the page */
		*pcp = CpMax (cp0, vcpLimLayout - 1);
		}

	else  if (pdod->fFtn || pdod->fAtn)
		{ /* in footnote or annotation, use cp of reference */
		int ifnd = IInPlc (pdod->hplcfnd, cp);
		pdrp = ((int *)PdodDoc(*pdoc)) + (pdod->fFtn ? edcDrpFtn : edcDrpAtn);
		*pcp = CpPlc( pdrp->hplcRef, ifnd );
		Assert (*pcp <= CpMacDocEdit(*pdoc));
		}

	else  if (pdod->fDispHdr)
		{ /* in docHdrDisp */
		Assert (vcpFirstLayout == cpNil && vcpLimLayout == cpNil);
		Assert (pdod->doc == *pdoc);
		CacheSect(*pdoc, CpMomFromHdr(doc));
		*pcp = caSect.cpFirst;
		}
	else
		{
		*pcp = cp0;
		Assert(fFalse);
		}
}



/* F  I N S E R T  F I E L D  S E P E R A T O R */
/*  Insert a chFieldSeparate into field ifld.  Field separator goes into
	the result section of the field.  If there is already a result section
	(and, therefore, a separator) do nothing.
*/

/* %%Function:FInsertFieldSeparator %%Owner:peterj */
FInsertFieldSeparator (doc, ifld)
int doc, ifld;

{
	CP cp;
	int ifldSeparate;
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	CHAR ch = chFieldSeparate;
	struct FLD fld;
	struct CHP chp;
	struct FLCD flcd;

	Assert (doc != docNil && hplcfld != hNil && ifld >= 0 && ifld <
			(*hplcfld)->iMac);

	GetIfldFlcd (doc, ifld, &flcd);

	if (flcd.dcpResult)
		/* already has a result */
		return fTrue;

	cp = flcd.cpFirst + flcd.dcpInst -1;
	SetFieldPchp (doc, cp, &chp, flcd.cpFirst);
	if (!FAssureHplcfld(doc, 1) ||
			!FInsertRgch (doc, cp, &ch, 1, &chp, NULL))
		return fFalse;

	fld.ch = chFieldSeparate;
	flcd.bData = fld.bData = bDataInval;
	ifldSeparate = IInPlcRef (hplcfld, cp);
	Assert (ifldSeparate > ifld && ifldSeparate <= flcd.ifldChEnd);
	/* won't fail--assured above */
	AssertDo(FInsertInPlc (hplcfld, ifldSeparate, cp, &fld));

	/*  update the fields structures to reflect separator */
	/* using the cached flt since field's will have been inval by InsertRgch */
	flcd.fResultDirty = fFalse;
	flcd.fResultEdited = fFalse;
	flcd.ifldChSeparate = ifldSeparate;
	flcd.ifldChEnd++;
	SetFlcdCh (doc, &flcd, 0);

	return fTrue;
}





/* D E L E T E  F I E L D  R E S U L T */
/*  Deletes all or a portion of the result of a field.  If fSeparator is
	specified will also delete separator.  If dcp >= 0 then dcp is the
	number of result characters to delete (iff dcp < the number normally
	deleted if dcp was not given).  THE LAST DCP RESULT CHARACTERS ARE
	DELETED, NOT THE FIRST DCP CHARACTERS!!
*/

/* %%Function:DeleteFieldResult %%Owner:peterj */
DeleteFieldResult (doc, ifld, fSeparator, dcp)
int doc, ifld;
BOOL fSeparator;
CP dcp;

{
	struct PLC **hplcfld = PdodDoc( doc )->hplcfld;
	CP dcpDelete;
	struct CA caDel;
	struct FLCD flcd;
	struct FLD fld;
	BOOL fSetCellBits;

	GetIfldFlcd (doc, ifld, &flcd);

	dcpDelete = flcd.dcpResult - (fSeparator ? 0 : 1);

	if (dcp >= 0 && dcp < (dcpDelete - (fSeparator ? 1 : 0)))
		dcpDelete = dcp;

	if (dcpDelete <= 0)
		return;

	if (dcpDelete == flcd.dcpResult)
		/* must remove the separator from the plc */
		{
		GetPlc(hplcfld, flcd.ifldChSeparate, &fld);
		FOpenPlc (hplcfld, flcd.ifldChSeparate, -1);
		FStretchPlc(hplcfld, 1);
		}

	PcaPoint( &caDel, doc, flcd.cpFirst+flcd.dcpInst+flcd.dcpResult - 1);
	caDel.cpFirst -= dcpDelete;
	/* adjust if selCur is forced out of a table it was in */
	fSetCellBits = (selCur.fWithinCell && selCur.doc == caDel.doc &&
			selCur.cpLim >= caDel.cpFirst && selCur.cpFirst <= caDel.cpLim);
	if (!FDelete(&caDel) && dcpDelete == flcd.dcpResult)
		/* delete failed, must restore the separator fld */
		FInsertInPlc (hplcfld, flcd.ifldChSeparate, flcd.cpFirst+flcd.dcpInst-1, 
				&fld);
	if (fSetCellBits)
		SetSelCellBits(&selCur);

	/*  must restore the field type, in case of invalidation by Replace */
	GetPlc( hplcfld, ifld, &fld );
	fld.flt = flcd.flt;
	PutPlcLast( hplcfld, ifld, &fld );
}


/* G E T  R E S U L T  C H P */
/*  Returns the chp that should be used for a field's result (where not
	otherwise defined.
*/

/* %%Function:GetResultChp %%Owner:peterj */
GetResultChp (pffb, pchp)
struct FFB *pffb;
struct CHP *pchp;

{
	GetPchpDocCpFIns (pchp, pffb->doc, pffb->cpField+1, fFalse, wwNil);
	pchp->fFldVanish = fFalse;
	Assert (pchp->fSpec == fFalse);
}



/* I N S E R T  F I E L D  E R R O R */
/*  Insert the field calculation error message into doc
	at cp.
*/

/* %%Function:CchInsertFieldError %%Owner:peterj */
CchInsertFieldError (doc, cp, istError)
int doc;
CP cp;
int istError;

{
	int cch;
	CHAR * stT = stErrorDef;
	CHAR stBuffer [cchMaxSz];
	struct CHP chp;

	GetPchpDocCpFIns (&chp, doc, cp, fTrue, wwNil);

	chp.fBold = !chp.fBold;

	/*  general error message */
	if (!FInsertRgch (doc, cp, stT+1, *stT, &chp, NULL))
		return 0;

	if (istError != iNil)
		{
		bltbx ((CHAR FAR *)rgstFldError [istError], (CHAR FAR *)stBuffer, 
				*rgstFldError [istError]+1);

		/*  specific error message */
		if (!FInsertRgch (doc, cp+*stT, stBuffer+1, *stBuffer, &chp, NULL))
			return *stT;
		}
	else
		*stBuffer = 0;

	return *stT + *stBuffer;
}


/* D C P  R E P L A C E  W I T H  E R R O R */
/*  Replace cp, cp+dcp with the field error string and return the new dcp.
*/

/* %%Function:DcpReplaceWithError %%Owner:peterj */
CP DcpReplaceWithError (doc, cp, dcp, istErr)
int doc;
CP cp, dcp;
int istErr;

{
	struct CA ca;

	PcaSetDcp(&ca, doc, cp, dcp);
	/*  delete old text */
	FDelete(&ca);

	/*  insert new error string */
	return (CP) CchInsertFieldError (doc, cp, istErr);
}




/* F C R  R E G I S T E R  I N T  R E S U L T */
/*  Called by calculation functions which have a result which is an
	integer.  Sets up the global result info for the numeric picture
	switch.
*/
/* %%Function:FcrRegisterIntResult %%Owner:peterj */
int FcrRegisterIntResult (n)
int n;
{
	vrruCalc.l = (long)n;
	vrrut = rrutLong;
	return fcrNumeric;
}


/* F C R  R E G I S T E R  L O N G  R E S U L T */
/*  Called by calculation functions which have a result which is a
	long.  Sets up the global result info for the numeric picture
	switch.
*/
/* %%Function:FcrRegisterLongResult %%Owner:peterj */
int FcrRegisterLongResult (l)
long l;
{
	vrruCalc.l = l;
	vrrut = rrutLong;
	return fcrNumeric;
}


/* F C R  R E G I S T E R  R E A L  R E S U L T */
/*  Called by calculation functions which have a result which is a
	real number.  Sets up the global result info for the numeric picture
	switch.
*/

/* %%Function:FcrRegisterRealResult %%Owner:peterj */
int FcrRegisterRealResult (pnum)
NUM *pnum;

{
	bltbyte(pnum, &vrruCalc.num, sizeof(NUM));
	vrrut = rrutNum;
	return fcrNumeric;
}


/* F C R  R E G I S T E R  D T T M  R E S U L T */
/*  Called by calculation functions which have a result which is a
	date or time.  Sets up the global result info for the numeric picture
	switch.
*/

/* %%Function:FcrRegisterDttmResult %%Owner:peterj */
int FcrRegisterDttmResult (dttm)
struct DTTM dttm;

{
	vrruCalc.dttm = dttm;
	vrrut = rrutDttm;
	return fcrDateTime;
}


/* F  F E T C H  A R G U M E N T  T E X T */
/*  Fetch the text of the next argument into rgch.  Does not preserve any
	other information.  If fTruncate then if the next argument is too large
	to fit into rgch the portion that does not fit will be skipped over;
	subsequent calls will fetch subsequent arguments.  If !fTruncate then if
	an argument is too long fOverflow will be set and subsequent calls will
	fetch subsequent portions of the argument.
	Fetches at most cchMax-1 characters into rgch and NULL terminates.
	Returns true if an argument was fetched.  Argument may have had zero
	length ("") in which case cch = 0.
*/

/* %%Function:FFetchArgText %%Owner:peterj */
FFetchArgText (pffb, fTruncate)
struct FFB * pffb;
BOOL fTruncate;

{
	struct FBB fbb;
	int cch;

	fbb = pffb->fbb;

	pffb->ccrMax = 0;
	pffb->rgcr = NULL;

	pffb->cchMax--;

	FetchFromField (pffb, fTrue /* fArgument */, fFalse /* fRTF */);

	cch = pffb->cch;
	pffb->rgch [cch] = '\0';

	Assert (cch == pffb->cchMax || !pffb->fOverflow);

	if (fTruncate && pffb->fOverflow)
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




/* F  F E T C H  A R G U M E N T  E X T E N T S */
/*  Fetches all of one argument.  Sets *pcpFirst and *pcpLim for
	the argument.  Does not maintain any other information.  Subsequent
	calls will fetch subsequent arguments.  Does not use buffers provided by
	caller.  Returns false if there was no argument fetched.  Fetched
	argument may have had zero length in which case *pcpFirst = *pcpLim =
	pffb->cpFirst on return.
*/

/* %%Function:FFetchArgExtents %%Owner:peterj */
FFetchArgExtents (pffb, pcpFirst, pcpLim, fRTF)
struct FFB * pffb;
CP * pcpFirst, * pcpLim;
BOOL fRTF;
{
	struct FBB fbb;
	struct CR rgcr [ccrArgumentFetch];

	fbb = pffb->fbb;

	pffb->ccrMax = ccrArgumentFetch;
	pffb->rgcr = rgcr;
	pffb->cchMax = 0;
	pffb->rgch = NULL;

	FetchFromField (pffb, fTrue /* fArgument */, fRTF);

	if (pffb->ccr > 0)
		{
		*pcpFirst = rgcr [0].cp;

		while (pffb->fOverflow)
			FetchFromField (pffb, fTrue, fFalse);

		Assert (pffb->ccr > 0);

		*pcpLim = rgcr [pffb->ccr-1].cp + rgcr [pffb->ccr-1].ccp;
		}
	else
		*pcpFirst = *pcpLim = pffb->cpFirst;

	pffb->fbb = fbb;
	pffb->cch = 0;
	pffb->ccr = 0;

	return !pffb->fNoArg;

}




/* S K I P  A R G U M E N T */
/*  Skips over one argument.
*/

/* %%Function:SkipArgument %%Owner:peterj */
SkipArgument (pffb)
struct FFB * pffb;

{
	int foc = pffb->foc;
	struct FBB fbb;

	fbb = pffb->fbb;
	SetBytes (&pffb->fbb, 0, sizeof (struct FBB));

	if (pffb->cpFirst == pffb->cpField+1)
		/* fetching keyword */
		pffb->foc = focNone;

	FetchFromField (pffb, fTrue /* fArg */, fFalse /* fRTF */);

	pffb->foc = foc;
	pffb->fbb = fbb;

}



/* D C P  C O P Y  A R G U M E N T */
/*  Copies the text of the next argument to docDest, cpDest.  Note that
	the text copied is that that would be fetched into an rgch (only
	results of fields, no vanished text).  Returns 0 if there is not
	another argument, else the number of characters copied.  (Warning:
	check vmerr.fMemFail).  ccr and cch are undefined on return.
*/

/* %%Function:DcpCopyArgument %%Owner:peterj */
CP DcpCopyArgument (pffb, docDest, cpDest)
struct FFB *pffb;
int docDest;
CP cpDest;

{
	CP dcp = 0;
	int docSrc = pffb->doc;
	int docCopy = docDest;
	CP cpCopy = cpDest, cpCopyStart = cp0;
	struct CR rgcr [ccrArgumentFetch];
	struct CA caSrc, caDest;
	struct FBB fbb;
	struct EFLT eflt;
	struct CA caT;

	if (docDest == docSrc)
		if ((docCopy = DocCreateScratch (docSrc)) == docNil)
			return cp0;
		else
			cpCopy = cp0;

	fbb = pffb->fbb;

	pffb->cchMax = 0;
	pffb->rgch = 0;
	pffb->ccrMax = ccrArgumentFetch;
	pffb->rgcr = rgcr;
	pffb->fOverflow = fFalse;

	do
		{
		CP dcpT;
		FetchFromField (pffb, fTrue/*fArg*/, fFalse /* fRTF */);
		if (pffb->ccr == 0)
			break;
		dcpT = DcpCopyIchCchPcr (docCopy, cpCopy, docSrc, rgcr,
				pffb->ccr, 0, -1);
		dcp += dcpT;
		cpCopy += dcpT;
		} 
	while (pffb->fOverflow && !vmerr.fMemFail);


	/* get rid of fFldVanish property for args of dead fields */
	if (pffb->flt != fltNil)
		{
		eflt = EfltFromFlt(pffb->flt);
		if (eflt.fDead)
			ApplyFFldVanish (PcaSet(&caT, docCopy, cpCopyStart, cpCopy), fFalse);
		}


	pffb->fbb = fbb;
	if (docDest != docCopy)
		{
		if (dcp != 0)
			{
			PcaSet( &caSrc, docCopy, cp0, dcp );
			ScratchBkmks (&caSrc);
			PcaPoint( &caDest, docDest, cpDest );
			FReplaceCps (&caDest, &caSrc);
			}
		ReleaseDocScratch();
		if (vmerr.fMemFail)
			return cp0;
		}
	return dcp;
}


/* D C P  C O P Y  I C H  C C H  P C R */
/*  Uses information in pcr (ccr) to copy cch characters starting at ich.
	if cch == -1, copies all.
*/

/* %%Function:DcpCopyIchCchPcr %%Owner:peterj */
CP DcpCopyIchCchPcr (docDest, cp, docSrc, pcr, ccr, ich, cch)
int docDest;
CP cp;
int docSrc;
struct CR *pcr;
int ccr, ich, cch;

{
	struct CR *pcrMac = pcr+ccr;
	CP dcp = 0, ccp;
	struct CA caSrc, caDest;

	PcaPoint( &caSrc, docSrc, CpFromIchPcr (ich, pcr, ccr) );
	PcaPoint( &caDest, docDest, cp );
	while (pcr->cp + pcr->ccp <= caSrc.cpFirst)
		pcr++;
	Assert (pcr < pcrMac);

	do
		{
		if ((ccp = (pcr->ccp - (caSrc.cpFirst-pcr->cp))) == 0)
			goto LContinue;
		if (cch != -1)
			ccp = CpMin (ccp, (CP)(cch - dcp));
		Assert (ccp > 0);

		caSrc.cpLim = caSrc.cpFirst + ccp;

		if (!FReplaceCps( &caDest, &caSrc ))
			/* failure, return number copied */
			break;
		caDest.cpLim = caDest.cpFirst + DcpCa(&caSrc);
		if (caDest.doc != vdocScratch)
			CopyStylesFonts (docSrc, caDest.doc, caDest.cpFirst, DcpCa(&caDest));

		if (vmerr.fMemFail)
			/* failure, return number copied */
			break;

		dcp += DcpCa(&caSrc);
		caDest.cpFirst = caDest.cpLim;

LContinue:
		pcr++;
		caSrc.cpFirst = pcr->cp;
		} 
	while (pcr < pcrMac && (cch == -1 || dcp < cch));


	return dcp;
}




/* E X H A U S T  F I E L D  T E X T */
/*  Scan the remaining text of pffb without picking up any characters or
	CRs.  Used to assure all switches in the field instructions have been
	read.
*/

/* %%Function:ExhaustFieldText %%Owner:peterj */
ExhaustFieldText (pffb)
struct FFB *pffb;

{
	struct FBB fbb;

	if (pffb->cpFirst < pffb->cpLim && pffb->foc <= focNormal)
		{
		Assert (pffb->foc == focNormal || pffb->foc == focNone);
		fbb = pffb->fbb;
		SetBytes (&pffb->fbb, 0, sizeof (struct FBB));
		FetchFromField (pffb, fFalse/*fArg*/, fFalse /* fRTF */);
		pffb->fbb = fbb;
		Assert (pffb->cpFirst >= pffb->cpLim);
		}
}


/* C H  F E T C H  S W I T C H */
/*  Iterates through the switches cached in pffb.  Returns each ch in turn,
	and returns chNil when complete.  If the switch has an argument, sets up
	pffb to fetch.  Calls with fSys TRUE and FALSE may be interspersed.
*/

/* %%Function:ChFetchSwitch %%Owner:peterj */
CHAR ChFetchSwitch (pffb, fSys)
struct FFB *pffb;
BOOL fSys;

{
	int ich;
	int foc = pffb->foc;
	struct FSF *pfsf;

	Assert (pffb->cpFirst == pffb->cpLim || foc > focNormal);

	if (foc < focNormal)
		/*  nothing (should be) cached */
		return chNil;

	if (fSys)
		{
		ich = foc / (cSwitchMax+1);
		foc += cSwitchMax+1;
		pfsf = &pffb->fsfSys;
		}
	else
		{
		ich = foc % (cSwitchMax+1);
		foc++;
		pfsf = &pffb->fsfFlt;
		}

	pffb->foc = foc;

	if (ich >= pfsf->c)
		/*  nothing left */
		return chNil;

	if (pfsf->rgcp [ich] != cpNil)
		{
		/*  set up to fetch the argument */
		pffb->cpFirst = pfsf->rgcp [ich];
		pffb->fOverflow = fFalse;
		}
	else
		/*  no argument */
		pffb->cpFirst = pffb->cpLim;


	return ChLower(pfsf->rgch [ich]);
}



/* C P  L I M  I N S T  F I E L D */
/*  Return the cpLim of the instruction portion of field ifld.  Includes
	either chFieldSeparate or chFieldEnd */

/* %%Function:CpLimInstField %%Owner:peterj */
CP CpLimInstField (doc, ifld)
int doc, ifld;

{
	struct FLCD flcd;
	GetIfldFlcd (doc, ifld, &flcd);
	return flcd.cpFirst + flcd.dcpInst;
}


/* %%Function:PcaFieldResult %%Owner:peterj */
struct CA *PcaFieldResult(pca, doc, ifld)
struct CA *pca;
int doc;
int ifld;
{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLCD flcd;

	GetIfldFlcd (doc, ifld, &flcd);
	return PcaSetDcp( pca, doc, flcd.cpFirst + flcd.dcpInst, flcd.dcpResult-1);
}


/* C P  L I M  F I E L D */
/*  Returns the cpLim of the field */
/* %%Function:CpLimField %%Owner:peterj */
EXPORT CP CpLimField (doc, ifld)
int doc, ifld;

{
	struct FLCD flcd;
	GetIfldFlcd (doc, ifld, &flcd);
	return flcd.cpFirst + flcd.dcpInst + flcd.dcpResult;
}



