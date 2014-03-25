/* F I E L D F M T . C */
/*  Fields formatting functions */

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

#define FIELDFMT  /* get the dnflt */
#include "field.h"


#ifdef PROTOTYPE
#include "fieldfmt.cpt"
#endif /* PROTOTYPE */

#ifdef PCJ
/* #define DFORMULA */
#endif /* PCJ */

#ifdef DEBUG
CP C_DcpSkipFieldChPflcd();
CP C_CpVisibleCpField();
#endif /* DEBUG */

/* E X T E R N A L S */

extern struct FLI   vfli;
extern struct FMTSS vfmtss;
extern int          vdocFetch;
extern CP           vcpFetch;
extern struct CHP   vchpFetch;
extern int          vccpFetch;
extern CHAR HUGE    *vhpchFetch;
extern struct PAP   vpapFetch;
extern struct SEL   selCur;
extern BOOL         vfInsertMode;
extern CP           cpInsert;
extern struct WWD **hwwdCur;
extern int          wwCur;
extern struct FFS  *vpffs;
extern struct DOD  **mpdochdod[];
extern char		(**vhgrpchr)[];
extern struct SCI	vsci;
extern struct PRI	vpri;
extern struct FTI	vfti;
extern struct FTI	vftiDxt;
extern int		vdocTemp;
extern struct CHP	vchpStc;
extern struct MERR	vmerr;
extern struct CA        caPara;

#ifdef DEBUG
BOOL vfSeeAllFieldCps = fFalse;
#endif /* DEBUG */


#ifdef DEBUG
/* C M D  S E E  A L L  C P S */

/* %%Function:CmdSeeAllCps %%Owner:peterj */
CMD CmdSeeAllCps (pcmb)
CMB *pcmb;
{
	vfSeeAllFieldCps = !vfSeeAllFieldCps;
	TrashAllWws();
	return cmdOK;
}


#endif /* DEBUG */








/* E F L T  F R O M  F L T */
/*  Perform a lookup in dnflt which can only be acessed from this module.
*/

/* %%Function:EfltFromFlt %%Owner:peterj */
NATIVE struct EFLT EfltFromFlt (flt) /* NATIVE for codespace table access */
int flt;

{
	Assert (flt >= 0 && flt < fltMax);
	return dnflt [flt];
}


/* F  F F L T  I N  F L T G */
/*  Perform a lookup in dnflt for the fltg of a field type.
*/

/* %%Function:FFltInFltg %%Owner:peterj */
FFltInFltg (flt, fltg)
int flt, fltg;

{
	Assert (flt >= 0 && flt < fltMax);
	return fltg == fltgAll || dnflt [flt].fltg == fltg;
}



/* F O R M A T  F U N C T I O N S */


#ifdef DEBUG
/* F F C  F O R M A T  F I E L D  P D C P */
/*  This function is called from FormatLine() to format a field.
	Returns:
		ffcSkip (skip *pdcp characters, no other processing)
		ffcShow (keep formatting normally, *pdcp == 0)
		ffcDisplay (create CHRDF, then skip *pdcp chars)
		ffcBeginGroup (create CHRFG, skip *pdcp chars)
		ffcEndGroup (terminate CHRFG, skip *pdcp chars)
*/

/* %%Function:C_FfcFormatFieldPdcp %%Owner:peterj */
HANDNATIVE C_FfcFormatFieldPdcp (pdcp, ww, doc, cp, ch)
CP *pdcp;
int ww, doc;
CP cp;
CHAR ch;
{
	int flt;
	int ifld;
	BOOL ffcReturn = ffcSkip;
	BOOL fResult;
	struct FLCD flcd;

	Assert (doc != docNil && cp < CpMacDoc (doc) && cp >= cp0);

#ifdef DEBUG
	if (vfSeeAllFieldCps)
		{
		*pdcp = 0;
		return ffcShow;
		}
#endif /* DEBUG */

	Scribble (ispFieldFormat1, 'F');

	if ((ifld = IfldFromDocCp (doc, cp, fTrue)) != ifldNil)
		FillIfldFlcd (PdodDoc(doc)->hplcfld, ifld, &flcd);

	flt = flcd.flt;

	if ((ifld == ifldNil || flcd.fDirty) && (ch == chFieldBegin))
		/* field is dead or must be parsed, try to parse it */
		{
		flt = FltParseDocCp (doc, cp, ifld, ww==wwCur /* fChgView */, fFalse /* fEnglish */);
		if ((ifld = IfldFromDocCp (doc, cp, fTrue)) != ifldNil)
		/* Note: FltParseDocCp may initialize hplcfld, so we need
		   to do another PdodDoc(doc) here */
			FillIfldFlcd (PdodDoc(doc)->hplcfld, ifld, &flcd);
		}

	if (ifld == ifldNil)
		{  /* dead field */
		/*  handles the case where the parse call caused the field to be
			vanished. */
		ffcReturn = ffcShow;
		*pdcp = cp0;
		goto LReturn;
		}

	Assert (ifld != ifldNil && flcd.flt >= fltMin && flcd.flt < fltMax);

	fResult = FShowResultPflcdFvc (&flcd, ww/*fvcScreen*/);
	*pdcp = DcpSkipFieldChPflcd (ch, &flcd, fResult, fFalse/*fFetch*/);

	if (fResult && dnflt [flt].fDisplay)
		{
		/* display type fields may have a CHRDF/CHRFG to build */
		ffcReturn = FfcFormatDisplay (ch, doc, &flcd, pdcp);
		}
	else  if (*pdcp == 0)
		ffcReturn = ffcShow;

LReturn:
	Assert (!(ffcReturn == ffcShow ^ *pdcp == 0));

	Scribble (ispFieldFormat1, ' ');

#ifdef DFORMULA
		{
		int rgw[6];
		rgw[0]=ifld;
		rgw[1]=flt;
		rgw[2]=ch;
		rgw[3]=ffcReturn;
		bltb (pdcp, &rgw[4], sizeof(CP));
		CommSzRgNum (SzShared("FormatField (ifld,flt,ch,ffc,dcp): "), rgw, 6);
		}
#endif /* DFORMULA */

	return ffcReturn;

}


#endif /* DEBUG */


#ifdef DEBUG
/* D C P  S K I P  F I E L D  C H  P F L C D */
/*  Return the number of characters to skip to display field fld given
	ch in mode fShowResult.  fFetch is true for fetch visible operations.
*/

/* %%Function:C_DcpSkipFieldChPflcd %%Owner:peterj */
HANDNATIVE CP C_DcpSkipFieldChPflcd (ch, pflcd, fShowResult, fFetch)
CHAR ch;
struct FLCD *pflcd;
BOOL fShowResult, fFetch;

{
	int flt = pflcd->flt;

	switch (ch)
		{
	case chFieldBegin:

		if (fShowResult)
			if (dnflt [flt].fResult && !pflcd->fPrivateResult)
				return pflcd->dcpInst;
			else
				return pflcd->dcpInst + pflcd->dcpResult - fFetch;

		else  /* ! fShowResults */
			return cp0;


	case chFieldSeparate:

		if (fShowResult)
			return (dnflt [flt].fResult && !pflcd->fPrivateResult ?
					(CP) 1 : pflcd->dcpResult);

		else
			return pflcd->dcpResult;



	case chFieldEnd:

		if (fShowResult)
			if (fFetch)
				return dnflt[flt].fDisplay ? (CP) 0 : (CP) 1;
			else
				return (CP) 1;

		else
			return cp0;



#ifdef DEBUG
	default:
		Assert (fFalse);
		return cp0;
#endif /* DEBUG */

		}

}


#endif /* DEBUG */


#ifdef DEBUG
/* F  S H O W  R E S U L T  P F L C D */
/*  Determine the Show Results/Instructions mode for a specific field.
*/

/* %%Function:C_FShowResultPflcdFvc %%Owner:peterj */
HANDNATIVE C_FShowResultPflcdFvc(pflcd, fvc)
struct FLCD * pflcd;
int fvc;
{
	struct WWD *pwwd;
	Assert (pflcd != NULL && fvc);

	if (fvc & fvcmWw)
		{
		Assert (!(fvc & ~fvcmWw));
		pwwd = PwwdWw(fvc/*ww*/);
		if (!pwwd->grpfvisi.fvisiShowAll)
			{
			int f;

			f = FFromIGrpf (dnflt [pflcd->flt].fltg, pwwd->grpfvisi.grpfShowResults);
			if (!pwwd->grpfvisi.fForceField && !pwwd->fPageView)
				f ^= pflcd->fDiffer;
			return f;
			}
		else
			return fFalse;
		}

	else  if (fvc & fvcmFields)
		return fFalse;
	else
		return fTrue;
}


#endif /* DEBUG */



#ifdef DEBUG
/* F I L L  I F L D  F L C D */
/*  Fill pflcd with information about field ifld.  Must scan plc and find
	all of the entries which refer to ifld.
*/

/* %%Function:C_FillIfldFlcd %%Owner:peterj */
HANDNATIVE C_FillIfldFlcd (hplcfld, ifld, pflcd)
struct PLC **hplcfld;
int ifld;
struct FLCD *pflcd;

{
	int cChBegin = 0;
	BOOL fSeparateFound = fFalse;
	CP cpSeparate;
	struct FLD fld;

	SetBytes (pflcd, 0, cbFLCD);
	Assert (ifld != ifldNil);
	pflcd->ifldChSeparate = ifldNil;
	GetPlc( hplcfld, ifld, &fld );
	Assert (fld.ch == chFieldBegin);

	pflcd->ifldChBegin = ifld;
	pflcd->cpFirst = CpPlc( hplcfld, ifld );
	pflcd->flt = fld.flt;
	pflcd->fDirty = fld.fDirty;

	for (;;)
		{
		GetPlc( hplcfld, ++ifld, &fld );
		Assert (ifld < IMacPlc( hplcfld ));
		switch (fld.ch)
			{
		case chFieldBegin:
			cChBegin++;
			break;
		case chFieldSeparate:
			if (cChBegin == 0)
				{
				Assert (!fSeparateFound);
				fSeparateFound = fTrue;
				pflcd->ifldChSeparate = ifld;
				pflcd->bData = fld.bData;
				cpSeparate = CpPlc( hplcfld, ifld );
				pflcd->dcpInst = cpSeparate - pflcd->cpFirst +1;
				}
			break;
		case chFieldEnd:
			if (cChBegin-- == 0)
				{
				pflcd->ifldChEnd = ifld;
				pflcd->grpf = fld.grpf;
				if (fSeparateFound)
					pflcd->dcpResult = CpPlc( hplcfld, ifld ) - cpSeparate;
				else
					pflcd->dcpInst = CpPlc( hplcfld, ifld ) - pflcd->cpFirst + 1;
				return;
				}
			break;
#ifdef DEBUG
		default:
			Assert (fFalse);
#endif /* DEBUG */
			}
		}
}


#endif /* DEBUG */


#ifdef DEBUG
/* G E T  I F L D  F L C D */
/*  Fill pflcd with the composit field information for field ifld in doc.
*/

/* %%Function:C_GetIfldFlcd %%Owner:peterj */
HANDNATIVE C_GetIfldFlcd (doc, ifld, pflcd)
int doc, ifld;
struct FLCD *pflcd;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;

	if (hplcfld != hNil && ifld >= 0 && ifld < IMacPlc( hplcfld ))
		FillIfldFlcd (hplcfld, ifld, pflcd);
	else
		SetBytes (pflcd, 0, cbFLCD);
}


#endif /* DEBUG */


#ifdef DEBUG
/* S E T  F L C D  C H */
/*  Set data items for the field represented by pflcd in doc.  ch
	indicates which items to update. (ch == NULL) -> all.
*/

/* %%Function:C_SetFlcdCh %%Owner:peterj */
HANDNATIVE C_SetFlcdCh (doc, pflcd, ch)
int doc;
struct FLCD *pflcd;
CHAR ch;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	int ifld;
	struct FLD fld;

	Assert( hplcfld != hNil);

	if (!ch || ch == chFieldBegin)
		{
		ifld = pflcd->ifldChBegin;
		GetPlc( hplcfld, ifld, &fld );
		Assert( fld.ch == chFieldBegin );
		fld.flt = pflcd->flt;
		fld.fDirty = pflcd->fDirty;
		PutPlcLast( hplcfld, ifld, &fld );
		}

	if (!ch || ch == chFieldSeparate)
		if (pflcd->ifldChSeparate != ifldNil)
			{
			ifld = pflcd->ifldChSeparate;
			GetPlc( hplcfld, ifld, &fld );
			Assert( fld.ch == chFieldSeparate );
			fld.bData = pflcd->bData;
			PutPlcLast( hplcfld, ifld, &fld );
			}

	if (!ch || ch == chFieldEnd)
		{
		ifld = pflcd->ifldChEnd;
		GetPlc( hplcfld, ifld, &fld );
		Assert( fld.ch  == chFieldEnd );
		fld.grpf = pflcd->grpf;
		PutPlcLast( hplcfld, ifld, &fld );
		}
}


#endif /* DEBUG */



/* I F L D  N E X T  F I E L D */
/*  Return the index of the next field in the plcfld for doc after cp.
	if fForward then search forward, else search backwards.
*/

/* %%Function:IfldNextField %%Owner:peterj */
IfldNextField (doc, cp)
int doc;
CP cp;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	int ifld;
	int ifldLast;
	struct FLD fld;

	if (hplcfld == hNil)
		return ifldNil;

	ifldLast = IMacPlc( hplcfld ) - 1;
	if ((ifld = IInPlcRef (hplcfld, cp)) >= 0 
			&& ifld <= ifldLast)
		{
		GetPlc( hplcfld, ifld, &fld );
		while (fld.ch != chFieldBegin)
			{
			if (ifld == ifldLast)
				goto LReturnNil;
			GetPlc( hplcfld, ++ifld, &fld );
			}
		return ifld;
		}
LReturnNil:
	return ifldNil;
}



/* I F L D  A F T E R  F L C D */
/*  Return the ifld of the next field after ifld and fill out pflcd with its
	composite field information.  Returns first field in doc if ifld ==
	ifldNil.  Returns ifldNil if ifld is the last field.
*/

/* %%Function:IfldAfterFlcd %%Owner:peterj */
IfldAfterFlcd (doc, ifld, pflcd)
int doc, ifld;
struct FLCD *pflcd;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLD fld;

	if (hplcfld == hNil)
		return ifldNil;

	while (++ifld < IMacPlc (hplcfld))
		{
		GetPlc (hplcfld, ifld, &fld);
		if (fld.ch == chFieldBegin)
			{
			FillIfldFlcd (hplcfld, ifld, pflcd);
			return ifld;
			}
		}

	return ifldNil;
}




/* C P  F I R S T  F I E L D */
/*  Return the starting cp of field ifld */

/* %%Function:CpFirstField %%Owner:peterj */
CP CpFirstField (doc, ifld)
int doc, ifld;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;

	Assert (doc != docNil && hplcfld != hNil);

	if (ifld < 0 || ifld >= IMacPlc( hplcfld ))
		return cp0;

	else
		{
#ifdef DEBUG
		struct FLD fld;
		GetPlc( hplcfld, ifld, &fld );
		Assert( fld.ch == chFieldBegin );
#endif
		return CpPlc( hplcfld, ifld );
		}

}



/* %%Function:PcaField %%Owner:peterj */
struct CA *PcaField(pca, doc, ifld)
struct CA *pca;
int doc;
int ifld;
{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	struct FLCD flcd;

	GetIfldFlcd (doc, ifld, &flcd);
	return PcaSetDcp( pca, doc, flcd.cpFirst, flcd.dcpInst + flcd.dcpResult );
}


/* C L E A R  F D I F F E R  F L T G */
/*  For all fields in doc whose field type is in field type group fltg,
	clear their fDiffer flags.  FDiffer flags
	indicate that the given field's show result/instructions mode is
	the opposite of the default for that field type group.
*/

/* %%Function:ClearFDifferFltg %%Owner:peterj */
ClearFDifferFltg (doc, fltg)
int doc, fltg;

{
	int ifld = ifldNil;
	struct FLCD flcd;
	struct DOD *pdod;

	FreezeHp ();
	while ((ifld = IfldAfterFlcd (doc, ifld, &flcd)) != ifldNil)
		{
		if (FFltInFltg (flcd.flt, fltg))
			{
			flcd.fDiffer = fFalse;
			SetFlcdCh (doc, &flcd, chFieldEnd);
			}
		}

	/* clear the flags for any sub-docs */
	if (!(pdod = PdodDoc (doc))->fShort)
		{
		if (pdod->docFtn != docNil)
			ClearFDifferFltg (pdod->docFtn, fltg);
		if (pdod->docAtn != docNil)
			ClearFDifferFltg (pdod->docAtn, fltg);
		if (pdod->docHdr != docNil)
			ClearFDifferFltg (pdod->docHdr, fltg);
		if (pdod->fDot)
			{
			if (pdod->docGlsy != docNil)
				ClearFDifferFltg (pdod->docGlsy, fltg);
			if (pdod->docMcr != docNil)
				ClearFDifferFltg (pdod->docMcr, fltg);
			}
		}
	MeltHp ();
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Fieldfmt_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Fieldfmt_Last() */
