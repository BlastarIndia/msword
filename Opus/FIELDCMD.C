/* F I E L D C M D . C */
/*  Field commands module */

#ifdef DEBUG
#ifdef PCJ
/* #define SHOWCALC */
/* #define DFLDS */
#endif /* PCJ */
#endif /* DEBUG */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "doc.h"
#include "sel.h"
#include "ch.h"
#include "message.h"
#define EXTMATH
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
#include "doslib.h"
#include "border.h"
#include "error.h"
#include "debug.h"
#include "field.h"
#include "toc.h"
#include "opuscmd.h"
#include "pic.h"

#ifdef PROTOTYPE
#include "fieldcmd.cpt"
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
extern struct PMD   *vppmd;
extern int          wwCur;
extern struct WWD   **hwwdCur;
extern struct DDES  vddes;
extern BOOL         vfDdeIdle;
extern struct UAB   vuab;
extern ENV	    *penvMathError;
extern CHAR         rgchEop[];
extern struct TCC   vtcc;
extern BOOL	    vfSeeSel;
extern struct PIC 	vpicFetch;	/* picture prefix from FetchPe */


/* C M D  L E V E L  F U N C T I O N S */



/* C M D  I N S E R T  F I E L D */
/*  Insert a open field and close field character at the current selection
	(if !fIns, fieldify selection).  Place an insertion point between
	the inserted characters.
*/
/* %%Function:CmdInsertField %%Owner:peterj */
CMD CmdInsertField (pcmb)
CMB *pcmb;
{
	int cmd;
	struct CA ca, caT;
	CP dcp;
	struct DOD *pdod;
	BOOL fInsertEmpty;  /* insert empty field characters */

	if ((*hwwdCur)->fPageView && (*hwwdCur)->grpfvisi.grpfShowResults
			&& !(*hwwdCur)->grpfvisi.fvisiShowAll)
		/* force ViewFieldCodes on */
		SetViewResultsFltg(wwCur, fltgOther, fFalse);

	ca = selCur.ca;

	/*  selCur.chp is used to determine the chp for the field chars */
	GetSelCurChp (fFalse);

	TurnOffSel (&selCur);

	if (fInsertEmpty = (selCur.sk == skIns))
		{
		if (!FSetUndoB1(imiInsField, uccPaste, PcaPoint(&caT, selCur.doc, selCur.cpFirst)))
			return cmdCancelled;
		ca.cpLim = ca.cpFirst;
		}
	else
		{
		if (!FSetUndoBefore(imiInsField, uccPaste))
			return cmdCancelled;
		ca.cpLim = CpMin (ca.cpLim, CpMacDocEdit (ca.doc));
		}

	if (((pdod = PdodDoc(ca.doc))->fFtn || pdod->fAtn) &&
			!FCheckRefDoc(ca.doc, ca.cpFirst, ca.cpLim, fTrue))
		return cmdError;

	if (fInsertEmpty)
		{
		FInsertFieldDocCp (ca.doc, ca.cpFirst, fFalse/*fSeparator*/);
		ca.cpLim += 2;
		}
	else
		{
		/* don't allow field char w/o intervening para mark before a table */
		if (FInTableDocCp(ca.doc, ca.cpLim))
			{
			CacheTc(wwNil, ca.doc, ca.cpLim, fFalse, fFalse);
			if (ca.cpFirst < vtcc.cpFirst)
				{
				Beep();
				return cmdError;
				}
			}
		if (FInTableDocCp(ca.doc, ca.cpFirst))
			{
			CacheTc(wwNil, ca.doc, ca.cpFirst, fFalse, fFalse);
			if (ca.cpLim >= vtcc.cpLim)
				{
				Beep();
				return cmdError;
				}
			}
		dcp = DcpCa(&ca);
		FPutCpsInField (fltUnknownKwd, ca.doc, ca.cpFirst, &dcp, cp0, NULL);
		ca.cpLim = ca.cpFirst + dcp + 2;
		}

	if (vmerr.fMemFail || vmerr.fDiskFail)
		return cmdNoMemory;

	DirtyDoc (ca.doc);
	SelectIns(&selCur, ca.cpFirst+1);
	PushInsSelCur();
	TurnOnSelCur();
	SetUndoAfter(&ca);

	return cmdOK;
}


/* D C P  U N L I N K  I F L D */
/*  Replace field ifld with its result.
*/
/* %%Function:DcpUnlinkIfld %%Owner:peterj */
CP DcpUnlinkIfld(doc, ifld)
int doc, ifld;
{
	struct EFLT eflt;
	struct FLCD flcd;
	CP dcpInst, dcpResult=0;
	int cch = 0;
	struct CA caT;

#ifdef DFLDS
	CommSzNum(SzShared("DcpUnlinkIfld: ifld = "), ifld);
#endif /* DFLDS */

	GetIfldFlcd (doc, ifld, &flcd);
	eflt = EfltFromFlt (flcd.flt);

	if (flcd.flt == fltImport)
		{
		/* let import metafiles (post winword 1.1, which stored bogus
		   metafiles) be unlinked and left behind.
		   Just restrict TIFF and the old format metafiles.
		*/
		CP cpImport = flcd.cpFirst + flcd.dcpInst; /* cp of chPic char */

#ifdef BZ
   	CommSzLong(SzShared("unlink import cp "), cpImport);
#endif /* BZ */

		if (ChFetch(doc, cpImport, fcmChars + fcmProps) ==
				chPicture && vchpFetch.fSpec)
			   {
			   FetchPe(&vchpFetch, doc, cpImport);

				// false if TIFF or old format metafile, else true
			   eflt.fResult = 
			   	!(vpicFetch.mfp.mm == MM_TIFF ||
				    (vpicFetch.mfp.mm < MM_META_MAX &&
		               !FEmptyRc(&vpicFetch.rcWinMF)));
			   }

#ifdef DEBUG
		else
			Assert (fFalse);
#endif /* DEBUG */

		}


	if (eflt.fResult)
		{  /* preserve the result */
		cch = CchRmFieldAtDocCp (doc, flcd.cpFirst, &dcpInst, &dcpResult);

		if (dcpInst != 0)
			FDelete( PcaSetDcp( &caT, doc, flcd.cpFirst, dcpInst ));
		}
	else
		/* no result, delete whole field */
		FDelete( PcaSetDcp( &caT, doc, flcd.cpFirst, 
				flcd.dcpInst+flcd.dcpResult) );

	return cch + DcpCa(&caT); /* number chars deleted */

}


/* C M D  R E P L A C E  F I E L D */
/*  Replace fields with their last result */

/* %%Function:CmdReplaceField %%Owner:peterj */
CMD CmdReplaceField (pcmb)
CMB *pcmb;
{
	int ifldFirst = IfldSelCur ();
	int ifld;
	struct FLD fld;
	struct PLC **hplcfld = PdodDoc(selCur.doc)->hplcfld;
	struct CA caT;
	int cfld = 0;
	CP cpLimSel = selCur.cpLim + (selCur.fIns ? 1 : 0);
	CP cpFirst, cpLim = -1, cpT;
	BOOL fUndo = fTrue;

	if (selCur.fBlock || ifldFirst == ifldNil)
		/*  no live field in the current selection.  FUTURE: would it be
			interesting to do something with dead fields?  i.e. Delete them?
			Note that if the user is in a dead field nested in a live field
			then the live field will be replaced.
		*/
		{
		Beep();
		return cmdError;
		}

	TurnOffSel(&selCur);

#ifdef DFLDS
	CommSzRgNum(SzShared("ca Before: "), &caAdjust, 5);
#endif /* DFLDS */

	cpFirst = CpFirstField(selCur.doc, ifldFirst);

	ifld = IInPlcCheck(hplcfld, cpLimSel-1);
	cpLim = CpMax(CpLimField(selCur.doc, IfldFromDocCp(selCur.doc, CpPlc(hplcfld,
			ifld), fTrue)), CpLimField(selCur.doc, ifldFirst));

	/* BLOCK */
		{
		struct DOD *pdod = PdodDoc(selCur.doc);
		PcaSet(&caT, selCur.doc, cpFirst, cpLim);
		if ((pdod->fFtn || pdod->fAtn) && !FCheckLargeEditFtn(&caT, &fUndo))
			return cmdCancelled;
		if (fUndo && !FSetUndoB1(bcmDerefField, uccPaste, &caT))
			return cmdCancelled;
		}

	AcquireCaAdjust();
	caAdjust = selCur.ca;

	if (caAdjust.cpFirst > cpFirst || caAdjust.cpLim < cpLim)
		caAdjust.doc = docNil;

	while (ifld >= ifldFirst && !vmerr.fMemFail && !vmerr.fDiskFail)
		{
		GetPlc(hplcfld, ifld, &fld);
		if (fld.ch == chFieldBegin)
			cpLim -= DcpUnlinkIfld(selCur.doc, ifld);
		ifld--;
		}

#ifdef DFLDS
	CommSzRgNum(SzShared("ca After: "), &caAdjust, 5);
#endif /* DFLDS */

	if (caAdjust.doc == docNil)
		{
		caAdjust.cpFirst = cpFirst;
		caAdjust.cpLim = cpLim;
		}

	Select (&selCur, caAdjust.cpFirst, caAdjust.cpLim);
	ReleaseCaAdjust();

	if (fUndo)
		SetUndoAfter(PcaSet(&caT, selCur.doc, cpFirst, cpLim));
	else
		SetUndoNil();
	DirtyDoc (selCur.doc);

	return cmdOK;

}


/* C M D  N E X T  F I E L D */
/*  Goto the next live field from selCur.cpFirst
*/

/* %%Function:CmdNextField %%Owner:peterj */
CMD CmdNextField (pcmb)
CMB *pcmb;
{
	if (!FNextField())
		{
		Beep();
		return cmdError;
		}

	return cmdOK;
}


/* %%Function:FNextField %%Owner:peterj */
FNextField()
{

	int ifld;
	CP cp;
	struct WWD *pwwd = *hwwdCur;
	struct CA ca;
	extern FNI	fni;

	Assert (PselActive() == &selCur);

	if (selCur.fBlock)
		return fFalse;

/* If sel is off screen, start from first cp on screen */
	EnsureSelInWw(fTrue, &selCur);

	cp = vfExtendSel ? selCur.cpLim : selCur.cpFirst+1;
	if ((ifld = IfldNextField (selCur.doc, cp)) == ifldNil)
		return fFalse;

	/* else select field ifld */

	PcaField( &ca, selCur.doc, ifld );
	MakeExtendSel(&selCur, ca.cpFirst, ca.cpLim, 0 /*grpf*/);

	if (vfSeeSel && hwwdCur != hNil)
		{
		SeeSel1(fTrue);
		vfSeeSel = fFalse;
		}

	fni.is = isNextFld;
	return fTrue;
}


/* C M D  P R E V I O U S  F I E L D */
/*  Goto the previous live field from selCur.cpFirst
*/

/* %%Function:CmdPreviousField %%Owner:peterj */
CMD CmdPreviousField (pcmb)
CMB *pcmb;
{
	if (!FPrevField())
		{
		Beep();
		return cmdError;
		}

	return cmdOK;
}


/* %%Function:FPrevField %%Owner:peterj */
FPrevField()
{
	CP cp;
	int ifld;
	struct WWD *pwwd = *hwwdCur;
	struct PLC **hplcfld = PdodDoc (selCur.doc)->hplcfld;
	extern FNI	fni;
	struct FLD fld;

	Assert (PselActive() == &selCur);

	if (selCur.fBlock)
		return fFalse;

/* If sel is off screen, start from first cp on screen */
	EnsureSelInWw(fTrue, &selCur);

	cp = vfExtendSel ? selCur.cpLim : selCur.cpFirst+1;
	if ((ifld = IfldSelCur ()) == ifldNil)
		if ((ifld = IfldNextField (selCur.doc, cp)) == ifldNil)
			if (hplcfld != hNil)
				ifld = IMacPlc(hplcfld);

	while (--ifld >= 0)
		{
		GetPlc (hplcfld, ifld, &fld);
		if (fld.ch == chFieldBegin)
			break;
		}

	if (ifld < 0)
		return fFalse;

	/* else select field ifld */

	MakeExtendSel(&selCur, CpFirstField (selCur.doc, ifld), CpLimField (selCur.doc, ifld), 0 /*grpf*/);

	if (vfSeeSel && hwwdCur != hNil)
		{
		SeeSel1(fTrue);
		vfSeeSel = fFalse;
		}

	fni.is = isPrevFld;
	return fTrue;
}


/* C M D  F I E L D  C O D E S  */
/*  This toggles the show results mode for fields in fltgOther.
*/

/* %%Function:CmdFieldCodes %%Owner:peterj */
CMD CmdFieldCodes(pcmb)
CMB *pcmb;
{
	if (hwwdCur == hNil || (*hwwdCur)->grpfvisi.fvisiShowAll)
		{
		Beep();
		return cmdError;
		}

	SetViewResultsFltg(wwCur, fltgOther, 
			!FFromIGrpf(fltgOther, (*hwwdCur)->grpfvisi.grpfShowResults));
	MakeSelCurVisi(fTrue /*fForceBlockToIp*/);
	return cmdOK;
}




/* C M D  T O G G L E  F I E L D  D I S P L A Y */
/*  This function toggles the Show Results/Show Instructions mode for all
	fields in the current selection.  Toggles the first field and
	brings all others into alingment with that one.
*/

/* %%Function:CmdToggleFieldDisplay %%Owner:peterj */
CMD CmdToggleFieldDisplay (pcmb)
CMB *pcmb;
{
	int doc = selCur.doc;
	CP cpFirst, cpLim = cpNil;
	CP cpImport;
	struct CA ca;
	int ifld = IfldSelCur ();
	int dcpIns = selCur.fIns ? 1 : 0;
	BOOL fDifferNew;
	struct FLCD flcd;

	if (selCur.fBlock || hwwdCur == hNil || (*hwwdCur)->grpfvisi.fvisiShowAll)
		{
		goto LBeep;
		}

	if ((*hwwdCur)->fPageView)
		return CmdFieldCodes(pcmb);

	GetIfldFlcd (doc, ifld, &flcd);
	fDifferNew = !flcd.fDiffer;
	cpFirst = flcd.cpFirst;

	while (ifld != ifldNil && flcd.cpFirst < selCur.cpLim+dcpIns)
		{
		flcd.fDiffer = fDifferNew;
		SetFlcdCh (doc, &flcd, chFieldEnd); /* write out the new fDiffer */
		cpLim = CpMax (cpLim, flcd.cpFirst + flcd.dcpInst + flcd.dcpResult);
		ifld = IfldAfterFlcd (doc, ifld, &flcd);
		}

	if (cpLim == cpNil)
		{
		/* no fields in selection feedback */
LBeep:
		Beep ();
		return cmdError;
		}

	PcaSet( &ca, doc, cpFirst, cpLim );
	CheckInvalCpFirst (&ca);
	InvalCp (&ca);
	MakeSelCurVisi(fFalse/*fForceBlockToIp, since selCur.fBlock already being rejected */);
	/* when toggling a picture import field, we might not call select,
		and so the graphics bit would not be set when it should be
		and vice versa
	*/
	if (FCaIsGraphics(&selCur.ca, selCur.ww, &cpImport))
		selCur.sk = skGraphics;
	else
		/* can't change the sk in this case, don't know what to
			set other bits to
		*/
		selCur.fGraphics = fFalse;


	return cmdOK;
}





/* C M D  F I E L D  L O C K S  O N */
/*  Turn on the field locks in the current selection.
*/

/* %%Function:CmdFldLocksOn %%Owner:peterj */
CMD CmdFldLocksOn (pcmb)
CMB *pcmb;
{
	int dcpIns = selCur.fIns ? 1 : 0;

	if (selCur.fBlock)
		{
		Beep();
		return cmdError;
		}

	if (!FSetFieldLocks (fTrue, selCur.doc, IfldSelCur(),
			selCur.cpLim+dcpIns))
		Beep ();
	else
		DirtyDoc (selCur.doc);
	return cmdOK;
}



/* C M D  F I E L D  L O C K S  O F F */
/*  Turn off the field locks in the current selection.
*/

/* %%Function:CmdFldLocksOff %%Owner:peterj */
CMD CmdFldLocksOff (pcmb)
CMB *pcmb;
{
	int dcpIns = selCur.fIns ? 1 : 0;

	if (selCur.fBlock)
		{
		Beep();
		return cmdError;
		}

	if (!FSetFieldLocks (fFalse, selCur.doc, IfldSelCur(), 
			selCur.cpLim+dcpIns))
		Beep ();
	else
		DirtyDoc (selCur.doc);
	return cmdOK;
}




/* F  S E T  F I E L D  L O C K S */
/*  Set all field locks starting with ifld and before cpLim to fOn.
*/

/* %%Function:FSetFieldLocks %%Owner:peterj */
FSetFieldLocks (fOn, doc, ifld, cpLim)
BOOL fOn;
int doc, ifld;
CP cpLim;

{
	BOOL fFound = fFalse;
	struct FLCD flcd;

	GetIfldFlcd (doc, ifld, &flcd);
	while (ifld != ifldNil && flcd.cpFirst < cpLim)
		{
		flcd.fLocked = fOn;
		SetFlcdCh (doc, &flcd, chFieldEnd);
		fFound = fTrue;
		ifld = IfldAfterFlcd (doc, ifld, &flcd);
		}

	return fFound;  /* no fields in selection feedback */
}


/* C M D  U P D A T E  S O U R C E */
/*  For all dirty INCLUDE fields in the current selection, copy their
	result back into the source document.
*/

/* %%Function:CmdUpdateSource %%Owner:peterj */
CmdUpdateSource (pcmb)
CMB *pcmb;
{
	int doc = selCur.doc;
	int ifld = IfldSelCur ();
	int dcpIns = selCur.fIns ? 1 : 0;
	BOOL fFound = fFalse;
	struct FLCD flcd;

	if (selCur.fBlock)
		{
		Beep();
		return cmdError;
		}

	StartLongOp ();

	GetIfldFlcd (doc, ifld, &flcd);
	while (ifld != ifldNil && flcd.cpFirst < selCur.cpLim+dcpIns &&
			!vmerr.fMemFail && !vmerr.fDiskFail)
		{
		if (flcd.flt == fltInclude && flcd.fResultDirty)
			fFound |= FUpdateSourceIfld (doc, ifld);
		ifld = IfldAfterFlcd (doc, ifld, &flcd);
		}

	if (!fFound)
		Beep ();
	else
		SetUndoNil();

	EndLongOp (fFalse);

	return cmdOK;
}


/* F  U P D A T E  S O U R C E  I F L D */
/*  Open the document ifld's result came from and stuff it back.
*/

/* %%Function:FUpdateSourceIfld %%Owner:peterj */
FUpdateSourceIfld (doc, ifld)
int doc, ifld;

{
	int docDest, fn, iBkmk;
	CHAR chSw;
	struct CA  caSrc, caDest;
	struct FFB ffb;
	CHAR stFile [ichMaxFile+1];
	CHAR stNorm [ichMaxFile];
	CHAR stBkmk [cchBkmkMax+1];

	PcaFieldResult (&caSrc, doc, ifld);
	if (DcpCa(&caSrc) <= 0)
		return fFalse;

	InitFvb (&ffb.fvb);
	SetFfbIfld (&ffb, doc, ifld);
	SkipArgument (&ffb);

	/*  filename */
	InitFvbBufs (&ffb.fvb, stFile+1, ichMaxFile, NULL, 0);
	if (!FFetchArgText (&ffb, fTrue/*fTruncate*/) || !ffb.cch)
		return fFalse;
	stFile [0] = ffb.cch;

	/*  bookmark */
	InitFvbBufs (&ffb.fvb, stBkmk+1, cchBkmkMax, NULL, 0);
	FFetchArgText (&ffb, fTrue);
	stBkmk [0] = ffb.cch;

	/*  converter switch */
	ExhaustFieldText (&ffb);
	while ((chSw = ChFetchSwitch (&ffb, fFalse/*fSys*/)) != chNil)
		if (chSw == chFldSwConverter)
			/* user specified converter--don't bother */
			return fFalse;

	/* open file/doc */
	if (!FNormalizeStFile (stFile, stNorm, nfoNormal))
		return fFalse;

	if ((docDest = DocOpenStDof (stNorm, dofBackground|dofNativeOnly, NULL)) == docNil)
		return fFalse;

#ifdef DEBUG
	if ((fn = PdodDoc (docDest)->fn) != fnNil && 
			(PfcbFn(fn)->fForeignFormat || !PfcbFn(fn)->fHasFib))
		{  /* assured by dofNativeOnly in DocOpenStDof() */
		Assert (fFalse);
		goto LFailed;
		}
#endif /* DEBUG */

	if (!*stBkmk || !FSearchBookmark (caDest.doc = docDest, stBkmk, &caDest.cpFirst,
			&caDest.cpLim, &iBkmk, bmcUser))
		{
		stBkmk [0] = 0;
		PcaSet(  &caDest, docDest, cp0, CpMacDocEdit(docDest) );
		}
	else
		/* get real bookmark name */
		{
		GetStFromSttb(PdodDoc(docDest)->hsttbBkmk, iBkmk, stBkmk);
		Assert (*stBkmk < cchBkmkMax);
		caDest.cpLim = CpMin (caDest.cpLim, CpMacDocEdit(docDest));
		}


	AssureLegalSel (&caSrc);
	AssureLegalSel (&caDest);

	FReplaceCps (&caDest, &caSrc );
	caDest.cpLim = caDest.cpFirst + DcpCa(&caSrc);

	DirtyDoc(docDest);

	if (vmerr.fMemFail || vmerr.fDiskFail)
		{
LFailed:
		DisposeDoc (docDest);
		KillExtraFns();
		return fFalse;
		}

	if (*stBkmk)
		/* must replace the bookmark (if this fails, life is tough but just the
			bkmk is lost). */
		FInsertStBkmk (&caDest, stBkmk, NULL );

	if (WwDisp(docDest,wwNil, fFalse) == wwNil)
		DoSaveDoc(docDest, fFalse);

	DisposeDoc (docDest);
	KillExtraFns();

	return fTrue;
}









/* F I E L D  R E S U L T  M O D E  F U N C T I O N S */



/* S E T  V I E W  R E S U L T S  F L T G */
/*  Set the view preference state for showing results for
	field type group fltg.
*/

/* %%Function:SetViewResultsFltg %%Owner:peterj */
SetViewResultsFltg (ww, fltg, f)
int ww, fltg, f;
{
	struct WWD *pwwd = PwwdWw(ww);

	Assert( (uns)f <= 1 );
	pwwd->grpfvisi.grpfShowResults &= ~(1<<fltg);
	pwwd->grpfvisi.grpfShowResults |= f << fltg;
	vpref.grpfvisi.grpfShowResults &= ~(1<<fltg);
	vpref.grpfvisi.grpfShowResults |= f << fltg;
	ClearFDifferFltg(selCur.doc, fltg);
	TrashFltg (fltg);
}




/* T O G G L E  V I E W  R E S U L T S  I F L D */
/*  Toggle the show inst/results mode for field ifld.
	This function is used by a display field to resort to show instructions
	mode.
*/

/* %%Function:ToggleViewResultsIfld %%Owner:peterj */
ToggleViewResultsIfld (doc, ifld)
int doc, ifld;

{
	struct FLCD flcd;
	GetIfldFlcd (doc, ifld, &flcd);
	flcd.fDiffer = !flcd.fDiffer;
	SetFlcdCh (doc, &flcd, chFieldEnd);
}











/* F I E L D  L O O K U P  F U N C T I O N S */



/* I F L D  E N C L O S I N G  I F L D */
/*  Return the ifld of the field in which ifld is nested. */

/* %%Function:IfldEncloseIfld %%Owner:peterj */
IfldEncloseIfld (doc, ifld)
int doc, ifld;

{
	int cChBegin = 0;
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLD fld;

	Assert (hplcfld != hNil);

	if (ifld < 0 || ifld >= IMacPlc( hplcfld ))
		return ifldNil;

	while (--ifld > ifldNil)
		{
		GetPlc( hplcfld, ifld, &fld );
		if (fld.ch == chFieldBegin)
			{
			if (cChBegin++ == 0)
				break;
			}
		else  if (fld.ch == chFieldEnd)
			cChBegin--;
		}

	return ifld;

}




/* I F L D  S E L  C U R */
/*  Determine what field the current selection is in.  Takes into
	consideration insertion points and looks for presence of fields starting
	after selCur.cpFirst.
*/

/* %%Function:IfldSelCur %%Owner:peterj */
IfldSelCur ()

{
	int dcpIns = selCur.fIns ? 1 : 0;
	CP cp = CpMax (cp0, selCur.cpFirst - dcpIns);
	int ifld = IfldFromDocCp (selCur.doc, cp, fFalse);

	if (selCur.fBlock)
		return ifldNil;

	if (ifld == ifldNil || CpLimField (selCur.doc, ifld) <= selCur.cpFirst)
		/*  no field at cp, look for one later */
		ifld = IfldFromDocCp (selCur.doc, selCur.cpFirst, fFalse);

	if (ifld == ifldNil || CpLimField (selCur.doc, ifld) <= selCur.cpFirst)
		/*  no field at cp, look for one later */
		ifld = IfldNextField (selCur.doc, cp);

	/*  assure the field isn't after the current selection */
	if (ifld != ifldNil && CpFirstField (selCur.doc, ifld) < selCur.cpLim
			+ dcpIns)
		return ifld;
	else
		return ifldNil;

}




/* I F L D  D O C  I F L D  C H */
/*  Returns index to the FLD for entry ch of field ifld.
*/

/* %%Function:IfldDocIfldCh %%Owner:peterj */
int IfldDocIfldCh (doc, ifld, ch)
int doc, ifld;
CHAR ch;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLCD flcd;
	struct FLD fld;

	Assert (hplcfld != hNil);
	Assert (IMacPlc( hplcfld ) > ifld);

	GetPlc( hplcfld, ifld, &fld );
	if (fld.ch == ch)
		return ifld;

	FillIfldFlcd (hplcfld, ifld, &flcd);
	Assert (ch != chFieldBegin);
	if (ch == chFieldSeparate)
		return flcd.ifldChSeparate;
	else
		return flcd.ifldChEnd;
}






/* S E T  F I E L D  P C H P */
/*  Set the standard values for a field character to be inserted at doc, cp.
	If cpFirst != cpNil, cpFirst is the field begin character of the field.*/

/* %%Function:SetFieldPchp %%Owner:peterj */
SetFieldPchp (doc, cp, pchp, cpFirst)
int doc;
CP cp;
struct CHP *pchp;
CP cpFirst;

{
	/*  If the field characters are being inserted at the insertion point,
		use selCur.chp (this allows transient insertion point props to
		apply).  Otherwise use the "insertion point" properties at doc,cp.
		Apply fSpec in any case.
		
		Special aperances are handled at formatline level to prevent the
		user from changing them.
	*/

	if (cpFirst == cpNil)
		if (doc == selCur.doc && cp == selCur.cpFirst)
			{
			GetSelCurChp (fFalse);
			*pchp = selCur.chp;
			}
		else
			GetPchpDocCpFIns (pchp, doc, cp, fTrue /* fIns */, wwNil);
	else
		GetPchpDocCpFIns (pchp, doc, cpFirst, fFalse, wwNil);

	pchp->fSpec = fTrue;

}








/* F I E L D  I N S E R T I O N  /  D E L E T I O N  U T I L I T I E S */




/* F  I N S E R T  F I E L D  D O C  C P */
/*  Place an empty field in doc at cp.  This field contains a chFieldBegin,
	a chFieldEnd and, if fSeparator, a chFieldSeparate between them.
	Instruction text can then be inserted at cp + 1, or, if fSeparator,
	result text can be inserted at cp + 2.
*/

/* %%Function:FInsertFieldDocCp %%Owner:peterj */
BOOL FInsertFieldDocCp (doc, cp, fSeparator)
int doc;
CP cp;
BOOL fSeparator;

{
	int ich = 0;
	CHAR rgch [3];
	struct CHP chp;

	Assert (doc != docNil && !vmerr.fMemFail && !vmerr.fDiskFail);

	if (cp > CpMacDocEdit (doc))
		cp = CpMacDocEdit (doc);

	rgch [ich++] = chFieldBegin;
	if (fSeparator)
		rgch [ich++] = chFieldSeparate;
	rgch [ich++] = chFieldEnd;

	SetFieldPchp (doc, cp, &chp, cpNil);
	chp.fRMark = PdodMother(doc)->dop.fRevMarking;
	if (!FInsertRgch (doc, cp, rgch, ich, &chp, NULL))
		return fFalse;
	/*  using fltUnknownKwd forces parse which forces inst visibility */
	if (!FInsertFltDcps (fltUnknownKwd, doc, cp, (CP)2,
			(fSeparator ? (CP)1 : (CP)0), NULL))
		{
		struct CA ca;
		/* delete what we just added */
		FDelete(PcaSetDcp(&ca, doc, cp, (CP)ich));
		return fFalse;
		}
	return fTrue;
}




/* F  P U T  C P S  I N  F I E L D */
/*  Given two contiguous ranges of cps, make them a field.  Insert
	a chFieldBegin at the beginning, a chFieldEnd at the end and, if
	there is a second range, a chFieldSeparate between them.  Insert the
	resulting field into the plcfld with type flt.

	If the caller 1) does not know (for sure) the type of the field they
	are inserting OR 2) wants to guarantee that the field will be displayed
	in SHOW INSTRUCTIONS mode, use fltUnknownKwd.

	Adjusts *pdcpInst if necessary to include EOP inserted before table.
*/

/* %%Function:FPutCpsInField %%Owner:peterj */
FPutCpsInField (flt, doc, cpFirst, pdcpInst, dcpResult, pchp)
int flt, doc;
CP cpFirst, *pdcpInst, dcpResult;
struct CHP *pchp;
{
	CHAR ch;
	struct CHP chp;
	struct PAP pap;
	struct CA caT;

	Assert (*pdcpInst >= cp0 && dcpResult >= cp0);

	if (pchp == NULL)
		SetFieldPchp (doc, cpFirst, &chp, cpNil);
	else
		chp = *pchp;

	chp.fSpec = fTrue;
	chp.fRMark = PdodMother(doc)->dop.fRevMarking;

	ch = chFieldEnd;
	if (!FInsertRgch (doc, cpFirst + *pdcpInst + dcpResult, &ch, 1, &chp, NULL))
		return fFalse;

	if (dcpResult != 0)
		{
		ch = chFieldSeparate;
		if (!FInsertRgch (doc, cpFirst + *pdcpInst, &ch, 1, &chp, NULL))
			goto LFail1;
		dcpResult++;
		}

	ch = chFieldBegin;
	if (!FInsertRgch (doc, cpFirst, &ch, 1, &chp, NULL))
		goto LFail2;

	if (!FInsertFltDcps (flt, doc, cpFirst, *pdcpInst + 2, dcpResult, NULL))
		{
		/* Insert failed, get rid of field characters */
		FDelete(PcaSetDcp(&caT, doc, cpFirst, 1));
LFail2:
		if (dcpResult)
			{
			--dcpResult;
			FDelete(PcaSetDcp(&caT, doc, cpFirst + *pdcpInst, 1));
			}
LFail1:
		FDelete(PcaSetDcp(&caT, doc, cpFirst + *pdcpInst + dcpResult, 1));
		return fFalse;
		}

	InvalTableProps(doc, cpFirst, *pdcpInst + dcpResult + 2, fTrue/*fFierce*/);

	return fTrue;
}



/* C C H  R M  F I E L D  A T  D O C  C P */
/*  Remove the field characters in and around a field.  Remove it from
	the plcfld if it is there.  Doc, cp is cpFirst of some field (dead or
	live).  Number of resulting instruction and result
	characters is returned in *pdcpInst and *pdcpResult.
*/

/* %%Function:CchRmFieldAtDocCp %%Owner:peterj */
CchRmFieldAtDocCp (doc, cp, pdcpInst, pdcpResult)
int doc;
CP cp, *pdcpInst, *pdcpResult;

{
	int ifld = IfldFromDocCp (doc, cp, fTrue);
	int cch = 2;
	struct CA ca;
	CP cpFirst, cpLim;
	struct FLCD flcd;

#ifdef DEBUG
	/*  This is hack--this function does something that is normally
		not allowed.  It deletes unmatched field delimiters.  It is
		ok for it to do so (it is its job).  To do it we must turn off
		the debugging check that assures we don't! */
	BOOL fCkFldDel = vdbs.fCkFldDel;
#endif /* DEBUG */

	if (ifld == ifldNil)
		{    /* dead field */
		cpFirst = cp;
		cpLim = CpLimDeadField (doc, cp);

#ifdef DEBUG
			{
			extern struct CHP vchpFetch;
			extern CHAR HUGE  *vhpchFetch;

			CachePara (doc, cpFirst);
			FetchCp (doc, cpFirst, fcmProps+fcmChars);

			if (!vchpFetch.fSpec || *vhpchFetch != chFieldBegin || cpFirst != cp)
				{
				Assert (fFalse);
				return 0;
				}
			}
#endif /* DEBUG */

		*pdcpInst = cpLim - cpFirst;
		*pdcpResult = cp0;
		}

	else  /* live field */		
		{
		GetIfldFlcd (doc, ifld, &flcd);
		cpFirst = flcd.cpFirst;

		if (cpFirst != cp)
			{
			Assert (fFalse);
			return 0;
			}

		cpLim = cpFirst + flcd.dcpInst + flcd.dcpResult;
		*pdcpInst = flcd.dcpInst;
		*pdcpResult = flcd.dcpResult;

		DeleteFld (doc, ifld);
		}

	/*  turn off debug checks during illegal deletes! */
	Debug (vdbs.fCkFldDel = fFalse);
	FDelete( PcaSet( &ca, doc, cpLim - 1, cpLim ) );
	if (*pdcpResult)
		{
		FDelete(PcaSetDcp( &ca, doc, cpFirst + *pdcpInst - 1, (CP)1));
		*pdcpResult -= 1;
		cch++;
		}
	FDelete( PcaSetDcp( &ca, doc, cpFirst, (CP)1));
	*pdcpInst -= 2;
	Debug (vdbs.fCkFldDel = fCkFldDel);

	return cch;
}


/* C M D  I N S  F L T  S Z  A T  S E L  C U R */
/*  Perform auto deletion at selCur.  Insert a field of type flt.
	If szArg is non-null, insert it as the field's arguments.  If
	fCalc cause the field to be calculated (frmUser). If !fShowDefault,
	force the view mode to be fShowResult.  SetUndo for the entire
	operation.
	
	If flt is a dead field the field is killed and fCalc and fShowFoo are
	ignored.  If flt==fltNil, szArg is assumed to contain a keyword (not
	fatal if it doesn't).
*/

/* %%Function:CmdInsFltSzAtSelCur %%Owner:peterj */
CMD CmdInsFltSzAtSelCur (flt, szArg, bcm, fCalc, fShowDefault, fShowResult)
int flt, bcm;
CHAR *szArg;
BOOL fCalc, fShowDefault, fShowResult;

{
	CMD cmdRet;
	int ifld;
	int cch;
	int doc = selCur.doc;
	CP cpIns, cpField, cpLimField, dcpInserted;
	struct EFLT eflt;
	struct CA ca, caRM;
	CHAR szKeyword [cchMaxFieldKeyword + 1];
	extern struct UAB vuab;

	Assert ((flt >= fltMinParse && flt < fltMax) || flt == fltNil);
	Assert (doc != docNil);

	CheckCollapseBlock();

	if (FTtpPsel(&selCur))
		{
		Beep();
		return cmdError;
		}

	if (!FSetUndoBefore(bcm, uccPaste))
		return cmdCancelled;

	if ((cmdRet = CmdAutoDelete(&caRM)) != cmdOK)
		return cmdRet;

	/*  insert the field and get the correct keyword */
	cpField = selCur.cpFirst;
	cpIns = cpField + 1;
	if (!FInsertFieldDocCp (doc, cpField, fFalse /* fSeparator */))
		return cmdNoMemory;

	dcpInserted = 2;

	if (flt != fltNil)
		GetFltKeyword (flt, szKeyword);

	if (szArg != NULL)
		/*  Argument portion given.  Insert it first. */
		{
		/*  add a space to the keyword */
		if (flt != fltNil)
			{
			CHAR *pch = szKeyword + CchSz (szKeyword) - 1;
			*pch++ = chSpace;
			*pch = '\0';
			}

		/*  insert the argument into the field */
		if (!FInsertRgch(doc,cpIns,szArg,(cch=CchSz(szArg)-1),&selCur.chp,NULL))
			goto LNoMem;
		dcpInserted += cch;
		}

	/*  now insert the keyword, in front of argument if present */
	if (flt != fltNil)
		{
		if (!FInsertRgch (doc, cpIns, szKeyword, (cch=CchSz(szKeyword)-1), 
				&selCur.chp, NULL))
			goto LNoMem;
		}

	/* if we run out of memory after this point we aren't too concerned */

	/*  parse the new field */
	ifld = IfldFromDocCp (doc, cpField, fTrue);
	Assert (ifld != ifldNil);
	flt = FltParseDocCp (doc, cpField, ifld, fFalse, fFalse);

	/*  get info about the new field */
	eflt = EfltFromFlt (flt);

	if (!eflt.fDead)
		/*  new field live.  set fDiffer and possibly calculate */
		{
		if (fCalc)
			{
			vwFieldCalc |= fclInsert;
			FCalcFieldIfld (doc, ifld, frmUser, 0, fFalse);
			vwFieldCalc &= ~fclInsert;
			}

		if (!fShowDefault)
			{
			struct FLCD flcd;
			GetIfldFlcd (doc, ifld, &flcd);
			flcd.fDiffer = (!PwwdWw(wwCur)->grpfvisi.fvisiShowAll) &&
					(FShowResultPflcdFvc (&flcd, selCur.ww/*fvcScreen*/)
					^ fShowResult);
			SetFlcdCh (doc, &flcd, chFieldEnd);
			}

		cpLimField = CpLimField (doc, ifld);
		}
	else  /* dead field */
		cpLimField = CpLimDeadField(doc, cpField);

	DirtyDoc (doc);
	PcaSet( &ca, doc, cpField, cpLimField );
	InvalCp (&ca);
	SelectIns( &selCur, cpLimField);
	caRM.cpLim += DcpCa(&ca);
	SetUndoAfter(&caRM);

	return cmdOK;

LNoMem:
	FDelete(PcaSetDcp(&ca, selCur.doc, cpField, dcpInserted));
	return cmdNoMemory;
}




/* T R A N S L A T E  F I E L D S */
/*  Translate keywords of fields to the current local's keywords.
*/

/* %%Function:TranslateFields %%Owner:peterj */
TranslateFields (doc)
int doc;

{
	int ifld = ifldNil;
	int fltT, cchNew;
	struct CA ca;
	FC fcIns;
	struct CHP chp;
	struct FFB ffb;
	struct FLCD flcd;
	CHAR szKwd [cchMaxFieldKeyword+1];

#ifdef DEBUG
#ifdef LOCALETEST
	CommSz (SzShared("Translating fields\n\r"));
#endif /* LOCALETEST */
#endif /* DEBUG */

	ca.doc = doc;
	while ((ifld = IfldAfterFlcd (doc, ifld, &flcd)) != ifldNil &&
			!vmerr.fMemFail && !vmerr.fDiskFail)
		{
		Assert (!flcd.fDirty);
		if ((cchNew = CchSzFromSzgId (szKwd+1, szgFltNormal, flcd.flt,
				cchMaxFieldKeyword)) <= 1)
			/*  we cannot translate this field */
			continue;

		szKwd [0] = chFieldEscape;
		InitFvb (&ffb.fvb);
		SetFfbIfld (&ffb, doc, ifld);
		ffb.foc = focNone;
		if (!FFetchArgExtents (&ffb, &ca.cpFirst, &ca.cpLim, fFalse))
			{
			Assert (fFalse);
			continue;
			}

		AssureLegalSel(&ca);
		GetPchpDocCpFIns (&chp, ca.doc, ca.cpFirst, fFalse, wwNil);
		if (!FNewChpIns (ca.doc, ca.cpFirst, &chp, stcNil))
			return;
		fcIns = FcAppendRgchToFn (fnScratch, szKwd, cchNew);
		if (vmerr.fDiskFail || !FReplace( &ca, fnScratch, fcIns, (FC)cchNew))
			return;

		fltT = flcd.flt;
		GetIfldFlcd (doc, ifld, &flcd);
		flcd.flt = fltT;
		SetFlcdCh (doc, &flcd, chFieldBegin);

		DirtyDoc (doc);
		}

	/* translate sub-docs */
	if (!PdodDoc (doc)->fShort)
		{
		if (PdodDoc (doc)->docFtn != docNil)
			TranslateFields (PdodDoc (doc)->docFtn);
		if (PdodDoc (doc)->docAtn != docNil)
			TranslateFields (PdodDoc (doc)->docAtn);
		if (PdodDoc (doc)->docHdr != docNil)
			TranslateFields (PdodDoc (doc)->docHdr);
		if (PdodDoc (doc)->fDot)
			{
			if (PdodDoc (doc)->docGlsy != docNil)
				TranslateFields (PdodDoc (doc)->docGlsy);
			if (PdodDoc (doc)->docMcr != docNil)
				TranslateFields (PdodDoc (doc)->docMcr);
			}
		}
}



/* G E T  F L T  K E Y W O R D */
/*  Fill a buffer with the keyword of a given flt.  sz must have at
	least room for cchMaxFieldKeyword characters.
*/

/* %%Function:GetFltKeyword %%Owner:peterj */
GetFltKeyword (flt, sz)
int flt;
CHAR *sz;

{
	int cch;
	Assert (flt >= 0 && flt < fltMax);

	/* try for single character keyword */
	if ((cch = CchSzFromSzgId (sz, szgFltSingle, flt, cchMaxFieldKeyword))
			== 0)
		cch = CchSzFromSzgId (sz, szgFltNormal, flt, cchMaxFieldKeyword);

	Assert (cch);
}


