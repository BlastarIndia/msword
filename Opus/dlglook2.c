/* D L G L O O K 2 . C */
/* FormatSection and FormatPosition dialogs */


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

#include "idd.h"
#include "rareflag.h"
#include "rerr.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "sect.hs"
#include "sect.sdm"
#include "abspos.hs"
#include "abspos.sdm"



/* Externals */

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
extern BOOL fElActive;

BOOL FCkMarginsSect(HCAB);
TMC TmcParagraph(CMB *);
TMC TmcPosition(CMB *);


/* The following maps are dependent on the order of the cab entries */

csconst SPNT mpiagspntSep [] =
{
	sprmSCcolumns,		uNinch,
			sprmSDxaColumns,	uNinch,
			sprmSLBetween,		uNinchCheck,
			sprmSBkc,		uNinchList,
			sprmSFEndnote,		uNinchCheck,
			sprmNoop,          	uNinchCheck, /* Computed from nLnnMod. */
	sprmSLnnMin,		uNinch,
			sprmSDxaLnn,		uNinch,
			sprmSNLnnMod,		uNinch,
			sprmSLnc,		uNinchRadio,
			sprmSVjc,		uNinchRadio,
};


#define iagSepMac (sizeof (mpiagspntSep) / sizeof (SPNT))

csconst SPNT mpiagspntPos [] =
{
	sprmPDxaAbs,		wNinch,
			sprmNoop,		wNinch,
			sprmPDyaAbs,		wNinch,
			sprmPPc,		uNinch,
			sprmPFromText,		uNinch,
			sprmPDxaWidth,		uNinch,
};


#define iagPosMac (sizeof (mpiagspntPos) / sizeof (SPNT))

struct PRBPOS
	{
	WORD cwPrb;
	WORD dxaHorz;
	WORD pcHorz;
	WORD dyaVert;
	struct PCVH pcVert;   /* must be word sized */
	WORD dxaFromText;
	WORD dxaWidth;
};

#define cwPrbPos (sizeof (struct PRBPOS) / sizeof (WORD))


/*  %%Function:  CmdSection  %%Owner:  bobz       */


CMD CmdSection(pcmb)
CMB * pcmb;
{
	CABSECTION * pcabsection;
	SPNT mpiagspnt [iagSepMac];
	struct SEP sepT;
	int cstcStd;

	bltbx((CHAR FAR *) mpiagspntSep, (CHAR FAR *) mpiagspnt,
			sizeof (mpiagspntSep));

	CacheSectSedMac(selCur.doc, selCur.cpFirst);
	sepT = vsepFetch; /* for compare later */
	caSect.doc = docNil; /* invalidate because SedMac used */

	if (pcmb->fDefaults)
		{
		/* initialize cab to proposed responses. If one section covers
		*  selCur, load props to cab otherwise load ninch.
		*/
		if (selCur.cpLim > caSect.cpLim)
			{
			/* whole cab is ninch */
			OurNinchCab(pcmb->hcab, mpiagspnt);
			}
		else
			{
			/* init cab with properties */
			NewPropToCab(pcmb->hcab, &vsepFetch, mpiagspnt);
			}

		pcabsection = (CABSECTION *) *pcmb->hcab;

		if (pcabsection->nLnnMod == uNinch)
			pcabsection->fLnn = uNinchCheck;
		else
			pcabsection->fLnn = (pcabsection->nLnnMod != 0);

		/* sep.ccolM1 uses zero for one column, etc. */
		if (pcabsection->cColumns != uNinch)
			pcabsection->cColumns++;

		if (pcabsection->lnnMin != uNinch)
			pcabsection->lnnMin++;
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltSection)];

		BltDlt(dltSection, dlt);

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

	if (pcmb->fCheck)
		{
		if (!FCkMarginsSect(pcmb->hcab))
			{
			return cmdError;
			}
		}

	if (pcmb->fAction)
		{
		/* treat special parameters, check parameters */
		pcabsection = (CABSECTION *) *pcmb->hcab;

		if (pcabsection->fLnn == uNinchCheck)
			pcabsection->nLnnMod = uNinch;
		else  if (pcabsection->fLnn)
			{
			if (pcabsection->nLnnMod == 0)
				pcabsection->nLnnMod = 1;
			if (!FStcDefined(selCur.doc, stcLnn)
					&& !FEnsureStcDefined(selCur.doc, stcLnn, &cstcStd))
				return cmdNoMemory;
			}
		else
			{
			/* signals no line numbering */
			pcabsection->nLnnMod = 0;
			}

		pcabsection = (CABSECTION *) *pcmb->hcab;

		if (pcabsection->cColumns != uNinch && 
				--pcabsection->cColumns < 0)
			{
			pcabsection->cColumns = 0;
			}

		if (pcabsection->lnnMin != uNinch && 
				--pcabsection->lnnMin < 0)
			{
			pcabsection->lnnMin = 0;
			}

		/* apply division properties to selection */
		/* invalidate page views handle in InvalSoftlyPgvwPara */
		if (!FNewPropFromRgProp(pcmb->hcab, fTrue /* fCab */, mpiagspnt))
			{
			return cmdCancelled;
			}

		/* check for changes that invalidate footnotes/annotations */
		CacheSect(selCur.doc, selCur.cpFirst);
		if (sepT.dxaColumnWidth != vsepFetch.dxaColumnWidth)
			{
			InvalDoc(selCur.doc);
			}

		/* Unmodify the modified cab for repeat and record... */
		pcabsection = (CABSECTION *) *pcmb->hcab;
		if (pcabsection->cColumns != uNinch)
			pcabsection->cColumns++;
		if (pcabsection->lnnMin != uNinch)
			pcabsection->lnnMin++;
		}

	return cmdOK;
}



/*  %%Function:  FDlgSection  %%Owner:  bobz       */

BOOL FDlgSection(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CABSECTION * pcabsection;
	HCAB hcab;

	switch (dlm)
		{
	case dlmInit:
		if (PdodMother(selCur.doc)->dop.fpc != fpcEndnote)
			EnableTmc(tmcSecFtn, fFalse);

		hcab = HcabDlgCur();
		Assert (hcab);

		pcabsection = (CABSECTION *) *hcab;
		if (pcabsection->fLnn == uNinchCheck || !pcabsection->fLnn)
			{
			SetTmcVal(tmcSecLNPerPage, uNinchRadio);
			SetTmcText(tmcSecLNFrom, szEmpty);
			SetTmcText(tmcSecLNStartAt, szEmpty);
			SetTmcText(tmcSecLNCountBy, szEmpty);
			EnableTmcGroup(tmcSecLNStartAt, 
					tmcSecLNCountBy, fFalse);
			EnableTmc(tmcSecLNGroup, fFalse);
			}
		break;

	case dlmClick:
		if (tmc == tmcSecLineNum)
			{
			BOOL fChecked;

			if ((fChecked = ValGetTmc(tmcSecLineNum)) != 
					uNinchCheck)
				{
				if (fChecked)
					{
					SetTmcVal(tmcSecLNFrom, 0);
					SetTmcVal(tmcSecLNStartAt, 1);
					SetTmcVal(tmcSecLNCountBy, 1);
					}
				else
					{
					SetTmcText(tmcSecLNFrom, szEmpty);
					SetTmcText(tmcSecLNStartAt, szEmpty);
					SetTmcText(tmcSecLNCountBy, szEmpty);
					}

				SetTmcVal(tmcSecLNPerPage, 
						fChecked ? 0 : uNinchRadio);
				EnableTmcGroup(tmcSecLNStartAt, 
						tmcSecLNCountBy, fChecked);
				EnableTmc(tmcSecLNGroup, fChecked);
				}
			}
		break;

	case dlmTerm:
		if (tmc != tmcOK)
			break;

		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			{
			/* sdm will take down dialog */
			return (fTrue);
			}

		if (!FCkMarginsSect(hcab))
			{
			/* Stay in dialog. */
			return fFalse;
			}
		/* break; */
		}

	return fTrue;
}


/*  %%Function:  FCkMarginsSect  %%Owner:  bobz       */

BOOL FCkMarginsSect(hcab)
HCAB hcab;
{
	CABSECTION * pcabsect;
	unsigned uWidth;
	struct DOP dop;
	int cNinch;

	dop = PdodMother(selCur.doc)->dop;

	pcabsect = (CABSECTION *) *hcab;

	/* assumes true == 1 */
	cNinch = (pcabsect->cColumns == uNinch) +
			(pcabsect->dxaColumns == uNinch);

	if (cNinch)
		goto LOk;
	if (pcabsect->cColumns  < 1 || pcabsect->dxaColumns < 0)
		goto LError;

	uWidth = dop.dxaLeft + dop.dxaRight + dop.dxaGutter +
			(pcabsect->cColumns - 1) * pcabsect->dxaColumns;

	if (dop.xaPage <= uWidth ||
			dop.dxaTab >= (dop.xaPage - uWidth) / pcabsect->cColumns)
		{
LError:
		ErrorEid(eidMTL, "FCkMarginsSect");
		return fFalse;
		}
LOk:
	return fTrue;
}


/* N E W  P R O P  T O  C A B */
/* NEW SDM VERSION */
/* writes properties specified by mpiagsprm into cab */
/*  %%Function:  NewPropToCab  %%Owner:  bobz       */

NewPropToCab(hcab, prgbProp, mpiagspnt)
HCAB hcab;
char * prgbProp;
SPNT mpiagspnt [];
{
	int sprm;
	int iag = 0;
	int iagMac = ((CABH *) *hcab)->cwSimple;

	for ( ; iag < iagMac; ++iag)
		{
		if ((sprm = mpiagspnt[iag].sprm) != sprmNoop)
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



/*  %%Function:  NinchPrb  %%Owner:  bobz       */

NinchPrb(prb, mpiagspnt)
WORD * prb;
SPNT	mpiagspnt[];
{
	int	iag;
	int iagMac = *prb;    /* 1st entry in prb is number of entries */

	for (iag = 0; iag < iagMac; iag++)
		{
		if (mpiagspnt[iag].sprm != sprmNoop)
			{
			*(prb + iag + 1) =
					(mpiagspnt[iag].wNinchVal == 0) ? wNinch :
					mpiagspnt[iag].wNinchVal;
			}
		}
}


/*  %%Function:  CmdPosition  %%Owner:  bobz       */

CMD CmdPosition(pcmb)
CMB * pcmb;
{
	TMC tmc;

	tmc = TmcPosition(pcmb);

	return (tmc == tmcOK || tmc == tmcPosPreview) ? cmdOK :
			  tmc == tmcCancel ? cmdCancelled : cmdError;
}


/*  %%Function:  TmcGosubPosition  %%Owner:  bobz       */

int TmcGosubPosition()
{
	/* allocate a local Cab on the stack */
	TMC tmc;
	CMB cmb;

	if ((cmb.hcab = HcabAlloc(cabiCABABSPOS)) == hNil)
		return tmcError;

	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;
	tmc = TmcPosition(&cmb);

	FreeCab(cmb.hcab);

	return tmc;
}


/*  %%Function:  TmcPosition  %%Owner:  bobz       */

TMC TmcPosition(pcmb)
CMB * pcmb;
{
	extern char (** vhmpiststcp) [];
	extern int vdocStsh;


	/* Note: this must be in sysc with the Position CAB as much as
	possible.  The 1st entry is the count of entries, excluding
	itself.
	*/


	pcmb->tmc = tmcOK; /* Default return value */

	if (!vfDefineStyle && selCur.fUpdatePap)
		{
		/* get paragraph properties */
		FGetParaState(fFalse /* fAll */ , fFalse /* fAbortOk */);
		}


	if (pcmb->fDefaults)
		{
		struct PAP * ppap, * ppapGray;
		struct PAP pap;

		/*
		*  Get all nongray properties indicated by mpiagsprmPos into
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
#ifdef FUTURE
	/* somewhat buggy feature disabled for version 1.0 */
		else  if (vlm == lmPreview)
			{
			CacheAbsObj();
			ppap = &vpapFetch;
			ppapGray = NULL;
			}
#endif
		else
			{
			ppap = &vpapSelCur;
			ppapGray = &vpapGraySelCur;
			}

		GetAbsPosDefaults(pcmb, ppap, ppapGray);
		}



#ifdef INFO_ONLY  /* why we are doing weird stuff for the abs pos vals */

	struct /* from PCVH in prm.h - what we get from sprmPpc */
		{
		int : 3;
		int fVert : 1;
		int pcVert : 2;
		int pcHorz : 2;
		int : 8;
		};
/* absolute positioning from props.h */
#define pcVMargin       0
#define pcVPage         1
#define pcHColumn       0
#define pcHMargin       1
#define pcHPage         2

	/* these are multiples of 4 because of tmvalNinch */
#define dxaAbsLeft       0
#define dxaAbsCenter    -4
#define dxaAbsRight     -8
#define dxaAbsInside    -12
#define dxaAbsOutside   -16

#define dyaAbsInline     0
#define dyaAbsTop       -4
#define dyaAbsCenter    -8
#define dyaAbsBottom    -12

#endif /* INFO_ONLY */


#ifdef BZ
	CommSzRgNum(SzShared("cabPos before dlg: "),
			((WORD *) (*(pcmb->hcab))) + cwCabMin,
			((CABH *) *(pcmb->hcab))->cwSimple);
#endif

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltAbsPos)];

		vrf.fNormalPrev = fFalse;
		BltDlt(dltAbsPos, dlt);
		TmcOurDoDlg(dlt, pcmb);
		}

	if (pcmb->fDefaults && !vfDefineStyle)
		FreePh(&vhmpiststcp);

	if (pcmb->fCheck)
		{
		int dxaHorz, dyaVert;

		dxaHorz = (int) ((CABABSPOS *) *pcmb->hcab)->dxaHorz;
		dyaVert = (int) ((CABABSPOS *) *pcmb->hcab)->dyaVert;
		if (dxaHorz < 0)
			{
			if (dxaHorz != dxaAbsCenter && dxaHorz != dxaAbsRight
					&& dxaHorz != dxaAbsInside && dxaHorz != dxaAbsOutside)
				{
				RtError(rerrIllegalFunctionCall);
				}
			}
		if (dyaVert < 0)
			{
			if (dyaVert != dyaAbsTop && dyaVert != dyaAbsCenter
					&& dyaVert != dyaAbsBottom)
				{
				RtError(rerrIllegalFunctionCall);
				}
			}
		}

/* Do the work */

	if (pcmb->fAction)
		{
		switch (pcmb->tmc)
			{
		case tmcPosPreview:
		case tmcOK:
			if (!FApplyHcabToAbsPos(pcmb->hcab))
				return tmcError;

			if (pcmb->tmc == tmcPosPreview)
				ChainCmd(bcmPrintPreview);
			break;
			}
		}

	return pcmb->tmc;
}


/*  %%Function:  FDlgAbsPos  %%Owner:  bobz       */

BOOL FDlgAbsPos(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{

	HCAB hcab;
	CABABSPOS * pcababspos;

	BOOL fRecord;
	BOOL fNormal = vrf.fNormalPrev;
	int val;

	fRecord = fTrue;

	switch (dlm)
		{
	case dlmInit:
		if (vfDefineStyle)
			EnableTmc(tmcPosPreview, fFalse);
		hcab = HcabDlgCur();
		Assert (hcab);
		pcababspos = (CABABSPOS *) *hcab;

		/* check for normal. See props.h for why pcVert not checked */
		/* we can do this at init since there will be no invalid
				values then. Otherwise the parsed fields may complain
			*/
		fNormal = (pcababspos->dxaHorz == 0 && pcababspos->dyaVert == 0
				&& pcababspos->pcHorz == 2 
				&& pcababspos->dxaWidth == 0);
		if (fElActive)
			EnableTmc(tmcPosPreview, fFalse);
		break;

	case dlmChange:

		switch (tmc)
			{
		case tmcDyaVert & ~ftmcGrouped:
		case tmcDxaHorz & ~ftmcGrouped:
		case tmcPosWidth:
			/* note these are not char-validated, so we only get change
				messages when we leave the control */
			goto TestNormal;
			}
		break;
	case dlmClick:
		switch (tmc)
			{
		case tmcPosReset:
			Assert (!vrf.fNormalPrev);

			SetTmcVal(tmcDxaHorz, 0);
			SetTmcVal(tmcDyaVert, 0);
			SetTmcVal(tmcPosFromText, 0);
			SetTmcVal(tmcPosWidth, 0);
			SetTmcVal(tmcPcHorz, 2);
			SetTmcVal(tmcPcVert, 0);

			SetTmcTxs(tmcDxaHorz, TxsAll());
			fNormal = fTrue;
			fRecord = fFalse;
			break;

		case tmcPcHorzM:
		case tmcPcHorzP:
		case tmcPcHorzC:
TestNormal:
			fNormal = GetfNormal();
			if ( !fNormal && vrf.fNormalPrev)
				SetTmcVal(tmcPosFromText, dxaPosFromTextDef);
			break;
			}
		break;
		}

	if (fNormal != vrf.fNormalPrev)
		{
		if (fNormal)
			SetTmcVal(tmcPosFromText, uNinch);
		EnableTmc(tmcPosFromText, !fNormal);
		EnableTmc(tmcDistText, !fNormal);
		EnableTmc(tmcPosReset, !fNormal);
		vrf.fNormalPrev = fNormal;
		}

	return fRecord;
}


/*  %%Function:  GetfNormal  %%Owner:  bobz       */

GetfNormal()
	/* normal if 3 edits are 0 and horz radio is 2 */

{
	BOOL fNormal;

	/* error and ninch edit vals make fNormal false */
	/* inhibit error reporting during these tests so we
		can change normal state when typing into edits.
	*/

	vmerr.fInhibit = fTrue;              /* stop reporting errors */
	fNormal = !(ValGetTmc(tmcPosWidth)) &&
			(ValGetTmc(tmcPcHorz) == 2);

	fNormal &= !(ValGetTmc(tmcDxaHorz));

	fNormal &= !(ValGetTmc(tmcDyaVert));

	vmerr.fInhibit = fFalse;             /*  report errors again  */
	return fNormal;
}


/*  %%Function:  FApplyHcabToAbsPos  %%Owner:  bobz       */

FApplyHcabToAbsPos(hcab)
HCABABSPOS hcab;
{
	CABABSPOS * pcab;
	struct PRBPOS prbPos;
	SPNT mpiagspnt [sizeof (mpiagspntPos) / sizeof (SPNT)];

	bltbx((CHAR FAR *) mpiagspntPos, (CHAR FAR *) mpiagspnt,
			sizeof (mpiagspntPos));

	prbPos.cwPrb = cwPrbPos - 1;  /* # entries excluding count */

	/* cab values */
	CabToPrb(&prbPos, hcab, 0, prbPos.cwPrb);
	pcab = *hcab;

	/* load into struct for sprmPpc */
	/* can set these values separately using
		pcvhNinch value for graying each part */

	prbPos.pcVert = 0;  /* clear other bits */

	prbPos.pcVert.pcHorz =  (pcab->pcHorz == uNinchRadio) ?
			pcvhNinch : (pcab->pcHorz + 4) % 3;

	prbPos.pcVert.pcVert =  (pcab->pcVert == uNinchRadio) ?
			pcvhNinch : pcab->pcVert;

#ifdef BZ
	CommSzRgNum(SzShared("prbPos after loading after dlg: "),
			&prbPos, cwPrbPos );
#endif
	/* FNewProp... will handle invalidating page view */
	return (FNewPropFromRgProp(&prbPos, fFalse /* fCab */, mpiagspnt));
}



/*  %%Function:  WParseCol  %%Owner:  bobz       */

WORD WParseCol(tmm, sz, ppv, bArg, tmc, opt)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD opt; /* wParam */
{
	int iMin = 1;
	int iMax = 100;

	Assert(HidOfDlg(hdlgNull) == IDDSection);
	switch (tmc)
		{
	default:
		Assert(fFalse);
	case tmcSecColNum:
		break; /* ranage is 1 and 100 */
	case tmcSecLNStartAt:
		iMax = 0x7FFF;
		break;
	case tmcSecLNCountBy:
		iMax = 100;
		break;
		}
	return (WParseOptRange(tmm, sz, ppv, bArg, tmc, opt, iMin, iMax));
}



/*  %%Function:  WParsePos   %%Owner:  bobz       */

EXPORT WORD WParsePos (tmm, sz, ppv, bArg, tmc, wParam)
TMM tmm;
char * sz;
void ** ppv;
WORD bArg;
TMC tmc;
WORD wParam;
{
	DPV dpv;
	int dzaAbs;
	CHAR * pch;
	int isz;
	int cLim;
	CHAR szTbl[cchMaxSz];
	switch (tmm)
		{
#ifdef tmmCwVal 
	case tmmCwVal:
		return 1;
#endif

	case tmmFormat:
		dzaAbs = WFromPpvB(ppv, bArg);
		pch = sz;
		*pch = '\0';

		if (dzaAbs != wNinch)
			{
			if (dzaAbs > 0)
				{
				/* values above 0 are biased by 1. see props.h */
				CchExpZa(&sz, dzaAbs - 1, vpref.ut, ichMaxNum, fTrue);
				}
			else
				{
				/* see props.h or info_only in this module for reasons */
				isz = (-dzaAbs) >> 2;
				AssertDo (Look2FEnumIEntbl (wParam, isz, pch));
				}
			}

		return fTrue;

	case tmmParse:
			{
			CchStripString(sz, CchSz(sz) - 1);

			/* check for prefefined string */

			cLim = (wParam == iEntblPosH) ?  cPosH : cPosV;

			for (isz = 0; isz < cLim; isz++)
				{
				AssertDo (Look2FEnumIEntbl (wParam, isz, szTbl));

				if (FEqNcSz(sz, szTbl))
					break;
				}
			if (isz < cLim)
				{
				dzaAbs = - (isz << 2);
				goto RetVal;
				}

			dpv = DpvParseFdxa(&dzaAbs, tmc, sz, 0, czaMax,
					dpvBlank | dpvSpaces | dpvDouble, 
					eidDxaOutOfRange, fTrue, vpref.ut);

			if (dpv == dpvError)
				{
				SetPpvBToW(ppv, bArg, wError);
				return fFalse;
				}

			if (dpv != dpvNormal && dpv != dpvDouble)
				dzaAbs = wNinch;
			else
				{
				Assert (dzaAbs >= 0);
				dzaAbs++;  /* bias positive vals by 1. see props.h */
				}

RetVal:
			SetPpvBToW(ppv, bArg, dzaAbs);
			return fTrue;
			}
		}

	return 0;
}


/*  %%Function:  GetAbsPosDefaults  %%Owner:  bobz       */

GetAbsPosDefaults(pcmb, ppap, ppapGray)
CMB * pcmb;
struct PAP * ppap, * ppapGray;
{
	CABABSPOS * pcab;
	struct PCVH  pcvh; /* position code operand for sprmPPc */
	struct PRBPOS prbPos;
	SPNT mpiagspnt [sizeof (mpiagspntPos) / sizeof (SPNT)];

	bltbx((CHAR FAR *) mpiagspntPos, (CHAR FAR *) mpiagspnt,
			sizeof (mpiagspntPos));

	prbPos.cwPrb = cwPrbPos - 1;  /* # entries excluding count */

	PropToPrbGray(&prbPos, ppap, ppapGray, mpiagspnt);


	/* copy entries from prb into cab.
		asserts to be sure they are in sync */

	Assert (offset(PRBPOS, dxaHorz) - sizeof(WORD) ==  offset(_cababspos, dxaHorz) - cbCabOverhead);
	Assert (offset(PRBPOS, pcHorz) - sizeof(WORD) ==  offset(_cababspos, pcHorz) - cbCabOverhead);
	Assert (offset(PRBPOS, dyaVert) - sizeof(WORD) ==  offset(_cababspos, dyaVert) - cbCabOverhead);
	Assert (offset(PRBPOS, pcVert) - sizeof(WORD) ==  offset(_cababspos, pcVert) - cbCabOverhead);
	Assert (offset(PRBPOS, dxaFromText) - sizeof(WORD) ==  offset(_cababspos,dxaFromText ) - cbCabOverhead);
	Assert (offset(PRBPOS, dxaWidth) - sizeof(WORD) ==  offset(_cababspos, dxaWidth) - cbCabOverhead);

	PrbToCab(&prbPos, pcmb->hcab, 0, prbPos.cwPrb);
	pcab = (CABABSPOS *) *pcmb->hcab;

	Assert (sizeof (struct PCVH) == sizeof (WORD));
	pcvh = prbPos.pcVert;
	/* note: must gray them both since in 1 sprm  bz */
	if ((WORD)pcvh == uNinch)
		{
		pcab->pcHorz = pcab->pcVert = uNinchRadio;
		}
	else
		{
		pcab->pcHorz = (pcvh.pcHorz + 2) % 3;/* rotate values */
		pcab->pcVert = pcvh.pcVert;
		}

	pcab->sab = 0;
}




/*  %%Function:  EnableTmcGroup  %%Owner:  bobz       */

EnableTmcGroup(tmcFirst, tmcLast, fEnable)
TMC tmcFirst;
TMC tmcLast;
BOOL fEnable;
{
	TMC tmc;

	for (tmc = tmcFirst; tmc <= tmcLast; tmc++)
		EnableTmc(tmc, fEnable);
}


/* Fill listboxes with string contents stuff for cmd funcs in this module */

/* General purpose list box fill function. */

/*  %%Function:  Look2WListEntbl  %%Owner:  bobz       */

EXPORT WORD Look2WListEntbl(tmm, sz, isz, filler, tmc, iEntbl)
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
		return Look2FEnumIEntbl(iEntbl, isz, sz);
		}

	return 0;
}


csconst ENTBL rgEntbl[] = {
	{ 
	cSecStarts,
		/* Must correspond to ordering of bkc in SEP. */
		{
		StKey("No Break",SecNoBreak),
				StKey("New Column",SecNewColumn),
				StKey("New Page",SecNewPage),
				StKey("Even Page",SecEvenPage),
				StKey("Odd Page",SecOddPage)
		}
	 },
	{ /* horizontal position */
	cPosH,
				{
		StKey("Left",PosLeft),
				StKey("Center",PosCenter),
				StKey("Right",PosRight),
				StKey("Inside",PosInside),
				StKey("Outside",PosOutside)
		}
	 },
	{ /* vertical position */
	cPosV,
				{
		StKey("Inline",PosInline),
				StKey("Top",PosTop),
				StKey("Center",PosCenter),
				StKey("Bottom",PosBottom)
		}
}


};


/*  %%Function:  Look2CopyEntblToSz  %%Owner:  bobz       */

Look2CopyEntblToSz(iEntbl, isz, sz)
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


/*  %%Function:  Look2FEnumIEntbl  %%Owner:  bobz       */

BOOL Look2FEnumIEntbl(iEntbl, isz, sz)
int     iEntbl;
int     isz;
CHAR    *sz;
{
	Assert(iEntbl < iEntblMax);

	if (isz < rgEntbl[iEntbl].iMax)
		{
		Look2CopyEntblToSz(iEntbl, isz, sz);
		return (fTrue);
		}
	else
		{
		return (fFalse);
		}

}
