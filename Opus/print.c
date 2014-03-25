#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "dlbenum.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "screen.h"
#include "sel.h"
#include "field.h"
#include "file.h"
#include "format.h"
#include "layout.h"
#include "print.h"
#include "rerr.h"
#include "style.h"
#include "border.h"
#include "debug.h"
#include "prm.h"
#include "ch.h"
#include "inter.h"
#include "prompt.h"
#include "message.h"
#define REVMARKING
#include "compare.h"
#undef REVMARKING
#include "sumstat.h"
#include "doslib.h"
#include "version.h"
#include "resource.h"
#include "ibdefs.h"

#include "error.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "print.hs"
#include "print.sdm"


#define ilrNil           (-1)
#define dxaEighth       (dxaInch/8)

extern int              vdocTemp;
extern int              vlm;
extern int              vflm;
extern int              wwCur;
extern int              vpgnFirst, vpgnLast, vsectFirst, vsectLast;
extern struct MWD       **hmwdCur;
extern struct WWD       **hwwdCur;
extern struct UAB       vuab;
extern struct SEL       selCur;
extern struct FLI       vfli;
extern struct SCI       vsci;
extern struct MERR      vmerr;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct TAP       vtapFetch;
extern struct TCC       vtcc;
extern struct PRI       vpri;
extern struct CA        caPara;
extern struct FMTSS     vfmtss;
extern struct CHP       vchpStc;
extern struct CA        caAdjust;
extern struct CA        caTap;
extern struct PMD       *vppmd;
extern struct FTI       vfti;
extern struct PLLR      **vhpllr;
extern FARPROC lpFPrContinue;
extern CHAR             szEmpty[];
extern CHAR             szApp[];
extern struct PREF      vpref;
extern struct RC        vrcBand;
extern int              fElActive;
extern int				vfRecording;
extern HWND vhwndPgPrvw;
extern HBRUSH vhbrBlack;
extern CP vcpLimLayout;
extern int              vwWinVersion;
extern struct DBS			vdbs;
extern struct STTB  **vhsttbFont;
extern HWND vhwndPrvwIconBar;
extern HWND vhwndPgPrvwScrl;
extern HWND vhwndRibbon;
extern HWND vhwndAppIconBar;
extern int mwCur;
extern int mwMac;
extern struct MWD       **mpmwhmwd[];

struct DTTM DttmCur();
DisplayLrTable();

struct PRSU     vprsu;
DSI             dsi;
/* Stores the double underline mode */
int				vulm = ulmNil;
#ifdef DEBUG
HDC             hdcPrint;
#endif

csconst char szOn[] = SzSharedKey(" on ",On);

/* %%Function:PdodMotherPrint %%Owner:davidbo */
struct DOD *PdodMotherPrint(doc)
int doc;
{
	return (PdodMother(DocMotherLayout(doc)));
}


/* %%Function:CmdPrint %%Owner:davidbo */
CMD CmdPrint(pcmb)
CMB * pcmb;
{
	int lmSave = -1, flmSave;
	CMD cmd = cmdOK;

	StartLongOp();
	if (!FPrinterOK())
		{
		ErrorEid(eidNoPrinter, " CmdPrint");
		cmd = cmdError;
		goto LRet;
		}

	switch (TmcDoPrintDialog(pcmb, fFalse/*fPrintMerge*/, fFalse/*fFileFind*/))
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

	if (pcmb->fCheck)
		{
		char st [cchMaxSz];
		CABPRINT *pcabPrint;

		pcabPrint = *pcmb->hcab;
		switch (pcabPrint->istPrSrc)
			{
		case istDocument:
			if (pcabPrint->prng != prngAll)
				pcabPrint->fSummary = fFalse;
			break;

		case istSummary:
		case istStyles:
		case istKeys:
			pcabPrint->fSummary = fFalse;
			pcabPrint->fShowInst = fFalse;
			pcabPrint->fSeeHidden = fFalse;
			pcabPrint->fAnnotations = fFalse;
			break;

		case istGlossary:
		case istAnnotation:
			pcabPrint->fSummary = fFalse;
			pcabPrint->fAnnotations = fFalse;
			break;
			}

		switch (pcabPrint->prng)
			{
		case prngSel:
			pcabPrint->fSummary = fFalse;
			/* FALL THROUGH */

		case prngAll:
			FSetCabSz(pcmb->hcab, szEmpty, Iag(CABPRINT, hszPrFrom));
			FSetCabSz(pcmb->hcab, szEmpty, Iag(CABPRINT, hszPrTo));
			if (vmerr.fMemFail)
				{
				cmd = cmdNoMemory;
				goto LRet;
				}
			pcabPrint = *pcmb->hcab;
			break;

		case prngPages:
			if ((SzToSt(*pcabPrint->hszPrFrom, st), !FCheckPageSect(st)) || 
					(SzToSt(*pcabPrint->hszPrTo, st), !FCheckPageSect(st)))
				{
				ErrorEid(eidInvalPrintRange, "CmdPrint");
				cmd = cmdError;
				goto LRet;
				}
			break;
			}

		if (pcabPrint->fAnnotations)
			pcabPrint->fSeeHidden = fTrue;

		if (vpri.fHaveBinInfo && vpri.cBins > 0)
			{
			int iidBin = pcabPrint->istPrFeed;
			if (iidBin < iidBinManual || iidBin > iidBinMax ||
					(iidBin == iidBinMixed &&
					(vpri.rgidBin[iidBin1] == idBinNil ||
					vpri.rgidBin[iidBin2] == idBinNil)) ||
					(iidBin < iidBinMax &&
					vpri.rgidBin[iidBin] == idBinNil))
				{
				if (fElActive)
					{
					RtError(rerrIllegalFunctionCall);
					Assert(fFalse);
					}
				else
					{
					Beep();
					cmd = cmdError;
					goto LRet;
					}
				}
			}

		if (selCur.sk != skRows && selCur.sk != skGraphics &&
				(selCur.sk != skSel || selCur.fTable || selCur.fWithinCell) &&
				pcabPrint->prng == prngSel)
			{
			ModeError();
			cmd = cmdError;
			goto LRet;
			}

		if ((*hwwdCur)->wk == wkMacro)
			{
			/* These don't make sense for macros... */
			pcabPrint->fAnnotations = fFalse;
			pcabPrint->fSeeHidden = fFalse;
			pcabPrint->istPrSrc = istDocument;
			pcabPrint->fRefresh = fFalse;
			pcabPrint->fSummary = fFalse;
			pcabPrint->fShowInst = fFalse;
			}
		}

	if (pcmb->fAction)
		{
		extern BOOL fElActive;
		int doc;
		BOOL fCloseDoc;
		char stFrom [20 + 1], stTo [20 + 1];

		doc = selCur.doc;
		fCloseDoc = fFalse;

		/* The FilePrint statement has an extra field called FileName.  This
			is used to handle printing from the catalog dialog... */
		if (fElActive)
			{
			char stFileName [ichMaxFile];

			GetCabSt(pcmb->hcab, stFileName, sizeof (stFileName), 
					Iag(CABPRINT, hszFileName));
			if (stFileName[0] != 0)
				{
				if ((fCloseDoc = (doc = DocFromSt(stFileName)) == 
						docNil) && (doc = DocOpenStDof(stFileName, 
						dofNoWindow, NULL)) == docNil)
					{
					cmd = cmdError;
					goto LRet;
					}
				}
			}

		if (!vpri.fHaveBinInfo)
			GetBinInfo(pcmb->hcab);
		FillPrsu(pcmb->hcab, stFrom, stTo, sizeof (stFrom));

		lmSave = vlm;
		flmSave = vflm;
		if (FBeginPrintJob(DocMotherLayout(doc), lmSave, flmSave))
			{
			DoPrint(doc, stFrom, stTo, fFalse/*fWholeDoc*/, fFalse);
			EndPrintJob (lmSave, flmSave);
			}

		if (fCloseDoc)
			DisposeDoc(doc);
		}

	StopProgressReport(hNil, pdcRestore);

	/* must be done after StopProgressReport so FAbortLayout
		returns legitimate values */
	if (vhwndPgPrvw != NULL)
		cmd = CmdRegenPrvw();
LRet:
	EndLongOp(fFalse);
	return cmd;
}



/* %%Function:TmcDoPrintDialog %%Owner:davidbo */
TmcDoPrintDialog (pcmb, fPrintMerge, fFileFind)
CMB *pcmb;
BOOL fPrintMerge;
{
	int tmc = tmcOK;
	CEPR cepr;

	cepr.fPrintMerge = fPrintMerge;
	cepr.fFileFind = fFileFind;
	pcmb->pv = &cepr;
	Assert(pcmb->hcab != hNil);
	if (FCmdFillCab())
		if (!FFillCabPrint(pcmb->hcab, fPrintMerge))
			return tmcError;

	if (FCmdDialog())
		{
		HCABPRINT hcabPrint = pcmb->hcab;
		char dlt [sizeof (dltPrint)];

		BltDlt(dltPrint, dlt);
		/*
		*  Because of macro language, istPrFeed must be iidBin except
		*  while in dialog.
		*/
		(*hcabPrint)->istPrFeed = (vpri.fHaveBinInfo && vpri.cBins > 0) ?
				IstPrFeedFromVpri() : uNinchList;
		tmc =  TmcOurDoDlg(dlt, pcmb);
		if (tmc == tmcOK)
			{
			(*hcabPrint)->istPrFeed =
					IidBinFromIstPrFeed((*hcabPrint)->istPrFeed);
			if (vfRecording)
				FRecordCab(hcabPrint, IDDPrint, tmcOK, fFalse);
			}
		}

	return tmc;
}


/* %%Function:FillPrsu %%Owner:davidbo */
FillPrsu(hcab, stFrom, stTo, cchMax)
HCABPRINT hcab;
CHAR *stFrom, *stTo;
int cchMax;
{
	int ist, iidBin;
	CABPRINT * pcabPrint;

	pcabPrint = *hcab;

	vprsu.cCopies = max(1, pcabPrint->cCopies);
	vprsu.fSeeHidden = pcabPrint->fSeeHidden;
	vprsu.fReverse = pcabPrint->fReverse;
	vprsu.prng = pcabPrint->prng;
	vprsu.fPrintAllPages = vprsu.prng == prngAll;
	vprsu.istPrSrc = pcabPrint->istPrSrc;
	vprsu.fDraft = pcabPrint->fDraft;
	vprsu.fShowResults = !pcabPrint->fShowInst;
	vprsu.fSummary = pcabPrint->fSummary;
	vprsu.fAnnotations = pcabPrint->fAnnotations;
	vprsu.fRefresh = pcabPrint->fRefresh;
	vprsu.iidBinPrFeed = iidBinNil;
	if (vpri.cBins > 0)
		{
		vprsu.iidBinPrFeed = pcabPrint->istPrFeed;
		/*
		*  In Dlg, impossible to select something other than what's in
		*  the list box.  In the Macro case, this case is screened at
		*  fCheck time in CmdPrint.
		*/
		Assert(vprsu.iidBinPrFeed != iidBinNil);
		Assert(vpri.rgidBin[vprsu.iidBinPrFeed] != idBinNil);
		}

	GetCabStz(hcab, stFrom, cchMax, Iag(CABPRINT, hszPrFrom));
	GetCabStz(hcab, stTo, cchMax, Iag(CABPRINT, hszPrTo));
}


/* %%Function:FFillCabPrint %%Owner:davidbo */
FFillCabPrint(hcab, fPrintMerge)
HCABPRINT hcab;
BOOL fPrintMerge;
{

	/* # of chars in printer name - must be tmcPrPrinter size >> 2 */
#define cchTitlePr (38)

	CABPRINT *pcabPrint;
	int cch, cchPort, cchRemain;
	char * pch, sz [cchMaxSz];
	char  szPort [cchMaxSz];
	char *pchPort;

	pcabPrint = *hcab;

	pcabPrint->sab = 0;
	pcabPrint->fReverse = vprsu.fReverse;
	pcabPrint->fSummary = vprsu.fSummary;
	pcabPrint->fDraft = vprsu.fDraft;
	pcabPrint->fAnnotations = vprsu.fAnnotations;
	pcabPrint->fSeeHidden = vprsu.fAnnotations || vprsu.fSeeHidden;
	pcabPrint->fShowInst = !vprsu.fShowResults;
	pcabPrint->prng = prngAll;
	pcabPrint->istPrSrc = istDocument;
	pcabPrint->cCopies = 1;
	pcabPrint->fRefresh = fFalse;

	/*
	*  If in macro, go ahead and get bin info now.  If in dlg, this
	*  delays visibility of dialog too much...wait for dlg idle.
	*/
	if (fElActive && !vpri.fHaveBinInfo)
		{
		GetBinInfo(hcab); /* HM */
		pcabPrint = *hcab;
		}
	else
		{
		pcabPrint->istPrFeed = (vpri.fHaveBinInfo && vpri.cBins > 0) ?
				vprsu.iidBinPrFeed : iidBinNil;
		}

	sz[0] = 0;
	FSetCabSz(hcab, sz, Iag(CABPRINT, hszPrFrom));
	FSetCabSz(hcab, sz, Iag(CABPRINT, hszPrTo));
	FSetCabSz(hcab, sz, Iag(CABPRINT, hszFileName));

	cch = CchCopySz(*vpri.hszPrinter, sz);
	pch = sz + cch;
	/*
	*  we check for the length to fit in the dialog here and
	*  truncate the path if it will help. Have to hard code
	*  the size, though  (cchTitlePr)
	*/
	cchPort = CchCopySz(*vpri.hszPrPort, szPort);
	pchPort = szPort;
	cchRemain = cchTitlePr - (cch + sizeof(szOn) - 1);
	if (cchRemain > 6 && cchRemain < cchPort && szPort[1] == ':')
		{
		FTruncatePathCbPsz(cchTitlePr - (cch + sizeof(szOn) - 1), &pchPort);
		cchPort -= (pchPort - szPort);
		}

	if ((cch + sizeof(szOn) + cchPort - 1) <= cchTitlePr)
		{
		CopyCsSz(szOn, pch);
		pch += sizeof(szOn) - 1;
		CchCopySz(pchPort, pch);
		}

	FSetCabSz(hcab, sz, Iag(CABPRINT, hszPrPrinter));
	/* fMemFail will be true if FSetCabSz fails */
	return (!vmerr.fMemFail);
}


/* %%Function:FDlgPrint %%Owner:davidbo */
BOOL FDlgPrint(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int istLb, cch, fEnable, fRecord;
	char * pch, st [cchMaxSz];

	switch (dlm)
		{
	case dlmInit:
		if (((CEPR *) PcmbDlgCur()->pv)->fPrintMerge)
			{  /* disable functions during PrintMerge */
			EnableTmc(tmcPrSel, fFalse);
			EnableTmc(tmcPrSrc, fFalse);
			}
		else  if (((CEPR *) PcmbDlgCur()->pv)->fFileFind)
			{
			EnableTmc(tmcPrSel, fFalse);
			}
		else
			{
			if (selCur.sk != skRows && selCur.sk != skGraphics &&
					(selCur.sk != skSel || selCur.fTable || selCur.fWithinCell))
				EnableTmc(tmcPrSel, fFalse);

			if ((*hwwdCur)->wk == wkMacro)
				{
				/* These don't make sense for macros... */
				EnableTmc(tmcPrSrc, fFalse);
				EnableTmc(tmcPrRefresh, fFalse);
				EnableTmc(tmcPrSummary, fFalse);
				EnableTmc(tmcPrAnnotations, fFalse);
				EnableTmc(tmcPrSeeHidden, fFalse);
				EnableTmc(tmcPrFields, fFalse);
				}
			}
		SetFocusTmc(tmcPrCopies);

		/* size of printer name text limited in cab fill rtn */
		break;

	case dlmChange:
		if (tmc == tmcPrTo || tmc == tmcPrFrom)
			{
			SetTmcVal(tmcPrRange, prngPages);
			SetTmcVal(tmcPrSummary, fFalse);
			goto LSetRest;
			}
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */

		if (wNew == 10)
			{
			if (!vpri.fHaveBinInfo)
				GetBinInfo(NULL /* hcab */);
			return fTrue;   /* stop idling */
			}

		return fFalse;  /* keep idling */


	case dlmClick:
		switch (tmc)
			{
		case tmcPrOptions:
			if (!vpri.fHaveBinInfo)
				GetBinInfo(NULL /* hcab */);
			 /* FSetDlgSab not safe if dialog coming down */
			if (FIsDlgDying()  || !FSetDlgSab(sabPRTOptions))
				{
				Beep();
				return fFalse;
				}
			SetFocusTmc(tmcPrReverse);
			EnableTmc(tmcPrOptions, fFalse);
			fEnable = vpri.cBins != 0;
			EnableTmc(tmcPrFeedTxt, fEnable);
			EnableTmc(tmcPrFeed, fEnable);

			/* enable function only if not PrintMerge */
			EnableTmc(tmcPrRefresh,
					(!((CEPR *) PcmbDlgCur()->pv)->fPrintMerge));

			fRecord = fFalse; /* so this is not recorded! */
			goto LSetRest; /* enable appropriate Include options */

		case tmcPrPages:
			SetTmcVal(tmcPrSummary, fFalse);
			EnableTmc(tmcPrSummary, fFalse);
			SetFocusTmc(tmcPrFrom);
			break;

		case tmcPrSrc:
			fRecord = fTrue;
LSetRest:
			istLb = ValGetTmc(tmcPrSrc);
			switch (istLb)
				{
			case istDocument:
				EnableTmc(tmcPrSummary, ValGetTmc(tmcPrRange) == prngAll);
				EnableTmc(tmcPrFields, fTrue);
				EnableTmc(tmcPrSeeHidden, fTrue);
				EnableTmc(tmcPrAnnotations, fTrue);
				break;

			case istSummary:
			case istStyles:
			case istKeys:
				EnableTmc(tmcPrSummary, fFalse);
				EnableTmc(tmcPrFields, fFalse);
				EnableTmc(tmcPrSeeHidden, fFalse);
				EnableTmc(tmcPrAnnotations, fFalse);
				SetTmcVal(tmcPrSummary, fFalse);
				SetTmcVal(tmcPrFields, fFalse);
				SetTmcVal(tmcPrSeeHidden, fFalse);
				SetTmcVal(tmcPrAnnotations, fFalse);
				break;

			case istGlossary:
			case istAnnotation:
				EnableTmc(tmcPrFields, fTrue);
				EnableTmc(tmcPrSeeHidden, fTrue);
				EnableTmc(tmcPrSummary, fFalse);
				EnableTmc(tmcPrAnnotations, fFalse);
				SetTmcVal(tmcPrSummary, fFalse);
				SetTmcVal(tmcPrAnnotations, fFalse);
				}

			if (!((CEPR *) PcmbDlgCur()->pv)->fFileFind &&
					(*hwwdCur)->wk == wkMacro)
				{
				/* These don't make sense for macros... */
				EnableTmc(tmcPrSrc, fFalse);
				EnableTmc(tmcPrRefresh, fFalse);
				EnableTmc(tmcPrSummary, fFalse);
				EnableTmc(tmcPrAnnotations, fFalse);
				EnableTmc(tmcPrSeeHidden, fFalse);
				EnableTmc(tmcPrFields, fFalse);
				}

			if (!fRecord)
				return fFalse;
			break;

		case tmcPrSel:
			SetTmcVal(tmcPrSummary, fFalse);
			/* FALL THROUGH */

		case tmcPrAll:
			SetTmcText(tmcPrTo, szEmpty);
			SetTmcText(tmcPrFrom, szEmpty);
			goto LSetRest;

		case tmcPrSeeHidden:
			/* if Hidden is off, Annotations must be off */
			if (!ValGetTmc(tmcPrSeeHidden))
				SetTmcVal(tmcPrAnnotations, fFalse);
			break;

		case tmcPrAnnotations:
			/* if Annotations are off, Hidden must be off */
			if (ValGetTmc(tmcPrAnnotations));
			SetTmcVal(tmcPrSeeHidden, fTrue);
			break;
			}
		break;

	case dlmTerm:
		if (tmc == tmcOK)
			{
			if (ValGetTmc(tmcPrAll) == prngPages)
				{
				GetTmcText(tmcPrFrom, st, cchMaxSz);
				SzToStInPlace(st);
				if (!FCheckPageSect(st))
					{
					SetTmcTxs(tmcPrFrom, TxsOfFirstLim(0, ichLimLast));
					ErrorEid(eidInvalPrintRange, "DlgfPrint");
					return fFalse;
					}

				GetTmcText(tmcPrTo, st, cchMaxSz);
				SzToStInPlace(st);
				if (!FCheckPageSect(st))
					{
					SetTmcTxs(tmcPrTo, TxsOfFirstLim(0, ichLimLast));
					ErrorEid(eidInvalPrintRange, "DlgfPrint");
					return fFalse;
					}
				}
			}
		break;
		}

	return fTrue;
}


/* enumerate print feed bins, i.e. bin 1, bin 2, etc. */
/* %%Function:WListPrFeed %%Owner:davidbo */
EXPORT WORD WListPrFeed(tmm, sz, isz, filler, tmc, wParam)
TMM	tmm;
char *	sz;
int	isz;
WORD	filler;
TMC	tmc;
WORD	wParam;
{
	int iidBin;
#ifdef DAVIDBO
	CommSzNum(SzShared("Enum isz: "), isz);
#endif
	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		if (!vpri.fHaveBinInfo || vpri.cBins <= 0 || isz > vpri.cBins)
			return fFalse;

		if (isz == vpri.cBins && (vpri.rgidBin[iidBin1] == idBinNil ||
				vpri.rgidBin[iidBin2] == idBinNil))
			return fFalse;

		iidBin = IidBinFromIstPrFeed(isz);
		Assert(iidBin != iidBinNil);
		Assert(vpri.rgidBin[iidBin] != idBinNil);
		CopyEntblToSz(iEntblPrFeed, iidBin, sz);
		return fTrue;
		}

	return fFalse;
}


/* %%Function:GetBinInfo %%Owner:davidbo */
GetBinInfo(hcabPrint)
HCABPRINT hcabPrint;
{
	short *pidBin, idBin, iidBinOther;
	char *sz;
	int lm, flm, iidBin, rgwOut[6];
	BINS bins;

	/* ensure vpri.hdc is set */
	lm = vlm;
	flm = vflm;
	SetWords(vpri.rgidBin, idBinNil, iidBinMax);
	SetWords(rgwOut, 0, 6);
	vpri.cBins = 0;
	vpri.idBinCur = idBinNil;
	vprsu.iidBinPrFeed = iidBinNil;
#ifdef INTL
	vpri.fHaveBinInfo = fTrue; /* before RedisplayTmc so lb will fill */
	return;
#endif /* INTL */
	ShrinkSwapArea();
	if (!FInitWwLbsForRepag(wwCur, DocMotherLayout(selCur.doc), lmPrint, NULL, NULL))
		goto LRet;
	Assert(vpri.hdc != hNil);
	Debug(hdcPrint = vpri.hdc);

	/* See how many paper sources the current device has... */
	if (Escape(vpri.hdc, GETSETPAPERBINS, 6, 0L, (LPSTR) rgwOut) > 0)
		{
		short cBins = cBinsMax;

		if (rgwOut[1] <= 0 ||
				Escape(vpri.hdc, ENUMPAPERBINS, NULL, (LPSTR)&cBins, (LPSTR)&bins) <= 0)
			goto LRet;

		vpri.idBinCur = rgwOut[0];
		vpri.cBins = min(rgwOut[1], cBinsMax);
		pidBin = &vpri.rgidBin[iidBin1];
		iidBinOther = iidBin1;
		for (iidBin = 0; iidBin < vpri.cBins; iidBin++)
			{
			idBin = bins.rgidBin[iidBin];
			sz = bins.szBin[iidBin];

#ifdef DAVIDBO
			CommSzNum(SzShared("gbi; idBin: "), idBin);
			CommSzSz(SzShared("gbi; sz: "), sz);
#endif

/* REVIEW jurgenl (davidbo):
	We have a few options here, none of which look very good.  Most
	drivers don't even implement the GetSetPaperBins and EnumPaperBins
	Escapes, the result being the Paper Feed drop down list box is dis-
	abled.  The HPPCL driver appears to return English strings, whereas
	the PSCRIPT driver appears to return localized strings.

	1)	The easiest thing to do would be to leave these strings as they
		are.  Users of HPPCL are OK, but PSCRIPT users are hosed.

	2)	The next easiest thing to do would be to localize these strings.
		Now, PSCRIPT users are OK, but HPPCL users get pimped.

	3)	Another possibility is to completely disable this feature for
		international versions of Opus.  The following code placed right
		after vpri and vprsu are filled with 0's and idBinNil's does
		this rather nicely:

				#ifdef INTL
					return;
				#endif

		This, of course, hoses all users of the international version.

	4)	The more difficult, but more useable, thing to do would be to
		fill the list box with the actual strings passed to us from the
		driver.  This makes implementation of the Mixed feature a little
		more difficult, makes a mockery of our fine documentation, and
		still isn't quite right since HPPCL passes non-localized strings.
		The documentation can be re-written, the HPPCL driver can be
		localized, and Mixed should be implemented in the driver anyway!
		This option seems more suited to version 1.1 or Coretex.
*/
			if (FPrefixSzSzBin(SzShared("Manual"), sz))
				vpri.rgidBin[iidBinManual] = idBin;
			else  if (FPrefixSzSzBin(SzShared("Envelope"), sz))
				vpri.rgidBin[iidBinEnvelope] = idBin;
			else  if (FPrefixSzSzBin(SzShared("Tractor"), sz))
				vpri.rgidBin[iidBinTractor] = idBin;
			else  if (FPrefixSzSzBin(SzShared("Auto"), sz))
				vpri.rgidBin[iidBinAuto] = idBin;
			else  if (FPrefixSzSzBin(SzShared("Either"), sz))
				vpri.rgidBin[iidBinAuto] = idBin;
			else
				{
				if (iidBinOther++ < iidBinMax)
					*pidBin++ = idBin;
				}
			}
#ifdef DAVIDBO
		CommSzNum(SzShared("gbi; idBinCur: "), vpri.idBinCur);
		CommSzRgNum(SzShared("gbi; vpri.rgidBin: "), vpri.rgidBin, cBinsMax);
#endif
		}

LRet:
	vpri.fHaveBinInfo = fTrue; /* before RedisplayTmc so lb will fill */

	if (vpri.cBins > 0)
		{
		int ist = IstPrFeedFromVpri();
		if (hcabPrint == NULL)
			{
			RedisplayTmc(tmcPrFeed);
			SetTmcVal(tmcPrFeed, ist);
			}
		else
			(*hcabPrint)->istPrFeed = IidBinFromIstPrFeed(ist);
		}
	vpri.wmm = 0;
	EndFliLayout(lm, flm);
	GrowSwapArea();
}


/* %%Function:IstPrFeedFromVpri %%Owner:davidbo */
IstPrFeedFromVpri()
{
	int iidBin, ist = -1;

	Assert(vpri.fHaveBinInfo && vpri.cBins > 0);
	/*
	*  If Joe User last selected Mixed, vpri.idBinCur is set to Bin2,
	*  which isn't what we want to be selected in the list box this
	*  time around!
	*/
	if (vprsu.iidBinPrFeed == iidBinMixed)
		{
		Assert(vpri.rgidBin[iidBin1] != idBinNil &&
				vpri.rgidBin[iidBin2] != idBinNil);
		/* cBins corresponds to the postion of "Mixed" in the list box */
		return vpri.cBins;
		}
	for (iidBin = 0; iidBin < iidBinMax; iidBin++)
		{
		if (vpri.rgidBin[iidBin] != idBinNil)
			{
			ist++;
			if (vpri.rgidBin[iidBin] == vpri.idBinCur)
				break;
			}
		}
	Assert(ist >= 0);
	return ist;
}


/* %%Function:IidBinFromIstPrFeed %%Owner:davidbo */
IidBinFromIstPrFeed(istPrFeed)
int istPrFeed;
{
	int ist, iidBin;

	if (!vpri.fHaveBinInfo || vpri.cBins <= 0)
		return iidBinNil;

	if (istPrFeed < 0 || istPrFeed > vpri.cBins)
		{
		Assert(fElActive);
		return iidBinNil;
		}
	if (istPrFeed == vpri.cBins)
		{
		if (vpri.rgidBin[iidBin1] == idBinNil ||
				vpri.rgidBin[iidBin2] == idBinNil)
			{
			Assert(fElActive);
			return iidBinNil;
			}
		return iidBinMixed;
		}
	ist = -1;
	for (iidBin = 0; iidBin < iidBinMax; iidBin++)
		{
		if (vpri.rgidBin[iidBin] != idBinNil)
			{
			ist++;
			if (ist == istPrFeed)
				return iidBin;
			}
		}
	Assert(fElActive);
	return iidBinNil;
}


/* returns fTrue if pch1 string is prefix of pch2 string */
/* %%Function:FPrefixSzSzBin %%Owner:davidbo */
FPrefixSzSzBin(pch1, pch2)
char *pch1, *pch2;
{
	while (*pch1)
		{
		if (!*pch2)
			return fFalse;
		if (ChUpper(*pch1++) != ChUpper(*pch2++))
			return fFalse;
		}
	return fTrue;
}


/* %%Function:FBeginPrintJob %%Owner:davidbo */
FBeginPrintJob(doc, lm, flm)
int doc;
{
	int iEscape, docMother, cch;
	struct WWD *pwwd;
	char szTitle[cchMaxSz], *pch;
	BOOL fRet = fTrue;
	BOOL fMsg = fTrue;

#ifdef DEMO
	if (CpMacDocEdit(doc) > cpMaxDemo)
		{
		ErrorEid(eidDemoCantPrint, "FBeginPrintJob");
		return fFalse;
		}
#endif /* DEMO */

	StartLongOp ();
	ShrinkSwapArea();
	vpri.hbrText = hNil;
	if (!FInitWwLbsForRepag(wwCur, doc, lmPrint, NULL, NULL))
		{
		ReportSz("Warning - print stopped because !vpri.hdc");
		goto LRetErr;
		}

	docMother = DocMotherLayout(doc);
	if (!FCheckPageAndMargins(docMother, fFalse, fTrue, NULL))
		{
		fMsg = fFalse;
		goto LRetErr;
		}

	vpri.fPrErr = fFalse;

#ifdef DEBUG
	hdcPrint = vpri.hdc;
#endif

	iEscape = Escape(vpri.hdc, SETABORTPROC, NULL, (LPSTR)lpFPrContinue, (LPSTR)NULL);
#ifdef DAVIDBO
	CommSz(SzShared("SetAbortProc\n\r"));
#endif
	if (iEscape < 0 || (vflm!=flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
		{
		EscapeError(iEscape);
		goto LRetErr;
		}

	Assert(doc == DocMotherLayout(doc));
	Assert(cchMaxSz >= ichMaxFile+3);
	cch = min(CchCopySz(szAppTitle, szTitle), cchMaxSz - ichMaxFile - 3);
	cch += CchCopySz(SzSharedKey(" - ", OneDash), &szTitle[cch]);
	GetDocSt(doc, &szTitle[cch], gdsoShortName);
	pch = &szTitle[cch];
	cch += szTitle[cch];
	StToSzInPlace(pch);
#ifdef DAVIDBO
	CommSzSz(SzShared("StartDoc: "), szTitle);
	CommSzNum(SzShared("\tcch: "), cch);
#endif

	/* internal windows buffer is only 32 big! */
	szTitle[31] = 0;
	cch = min(cch, 32);
	iEscape = Escape(vpri.hdc, STARTDOC, cch, (LPSTR)szTitle, (LPSTR)NULL);
#ifdef DAVIDBO
	CommSz(SzShared("StartDoc\n\r"));
#endif
	if (iEscape < 0 || (vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
		{
		EscapeError(iEscape);
		goto LRetErr;
		}

	if ((vpri.hbrText = CreateSolidBrush(vpri.rgbText)) == hNil)
		vpri.hbrText = vhbrBlack;
	Debug(else  
		LogGdiHandle(vpri.hbrText,1011));

	EnableHotSpots(fFalse /*fEnable*/);
	return fTrue;

LRetErr:
	vpri.fPrErr = fTrue;
	if (fMsg)
		ErrorEid(eidCantPrint, "");
	EndFliLayout(lm, flm);
	EndLongOp (fFalse);
	GrowSwapArea();
	return fFalse;
}


/* %%Function:EndPrintJob %%Owner:davidbo */
EndPrintJob(lm, flm)
int lm, flm;
{
	int iEscape;

	if (!vpri.fPrErr)
		{
		Escape(vpri.hdc, ENDDOC, 0, (LPSTR)NULL, (LPSTR)NULL);
#ifdef DAVIDBO
		CommSz(SzShared("EndDoc\n\r"));
#endif
		}

	if (vpri.hbrText != vhbrBlack && vpri.hbrText != hNil)
		{
		UnlogGdiHandle(vpri.hbrText, 1011);
		DeleteObject(vpri.hbrText);
		}
	GrowSwapArea();
	EndFliLayout(lm, flm);
	EnableHotSpots(fTrue /*fEnable*/);
	EndLongOp (fFalse);
}


/* %%Function:DoPrint %%Owner:davidbo */
DoPrint(doc, stFrom, stTo, fWholeDoc, fChkPgMgn)
int doc;
CHAR *stFrom, *stTo;
BOOL fWholeDoc;
int fChkPgMgn;
{
	CP cpFirst, cpLim;
	int docPr, docMother, istPrSrc;
	int sectFirst, sectLast, pgnFirst, pgnLast;
	int ftcMacMother, ftcMacSave, ftcMaxSave;
	CHAR (**hmpftcibstFontSave)[];
	struct DOD *pdod, *pdodMother;
	struct DOP *pdop;
	struct CA ca;
	struct DOP dop;
	struct PPR **hppr;
	char st[cchMaxFile];

	docMother = DocMotherLayout(doc);
	vpri.cpAdjust = cp0;
	InitDsi();

	/* make sure header/footer is up to date */
	FCleanUpHdr(docMother, fFalse, fFalse);

	if (vpri.hdc == NULL)
		{
		ErrorEid(eidCantPrint, "");
		goto LRet;
		}

	/* re-enable the "could not display font" message */
	vmerr.fHadDispAlert = fFalse;

	istPrSrc = vprsu.istPrSrc;
	SetUpPrintPrompt(docMother, istPrSrc, 1, &hppr, fTrue);

	if (FQueryAbortCheck() || (fChkPgMgn && !FCheckPageAndMargins(docMother,fFalse,fTrue,NULL)) ||
			((istPrSrc == istDocument || istPrSrc == istGlossary ||
			istPrSrc == istAnnotation) &&
			!FPrepareDocFieldsForPrint (docMother, 
			pfpPrinting|(vprsu.fRefresh?pfpRefreshAll:0))))
		{
		istPrSrc = istNil;
		goto LRet;
		}

/* cp are adjusted after FPrepareDocFieldsForPrint */
	if (fWholeDoc || vprsu.prng != prngSel || 
			(vprsu.prng == prngSel && istPrSrc == istAnnotation && docMother != doc))
		{
		cpFirst = cp0;
		cpLim = CpMacDoc(docMother);
		}
	else
		{
		cpFirst = selCur.cpFirst;
		cpLim = selCur.cpLim;
		}

	if (vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint))
		goto LRet;

	vlm = lmPrint;
	vpgnFirst = vsectFirst = 1;
	vpgnLast = vsectLast = 0x7FFF;
	if (vprsu.prng == prngPages && (istPrSrc == istDocument ||
			istPrSrc == istAnnotation || istPrSrc == istGlossary))
		{
		/* use start/end pages */
		Assert(stFrom);
		Assert(stTo);
		PageSectFromSt(stFrom, &vpgnFirst, &vsectFirst/*, fTrue*/);
		PageSectFromSt(stTo, &vpgnLast, &vsectLast/*, fFalse*/);
		vsectLast = max(vsectLast, vsectFirst);
		/*if (vpgnLast == 0)
			vpgnLast = 0x7FFF;*/
		if (vpgnLast==0)
			vpgnLast = vprsu.fReverse ? 1:0x7FFF;
		if (vpgnFirst==0)
			vpgnFirst = vprsu.fReverse ? 0x7FFF:1;
		}

	if (vpgnLast < vpgnFirst)
		{
		int t = vpgnFirst;
		vpgnFirst = vpgnLast;
		vpgnLast = t;
		vprsu.fReverse = fTrue;
		}
	pgnFirst = vpgnFirst;
	pgnLast = vpgnLast;
	sectFirst = vsectFirst;
	sectLast = vsectLast;

	/* set doc to be printed - if selection/glsy/style, use undo document */
	SetUndoNil();
	switch (istPrSrc)
		{
	case istStyles:
		if (!FFillUndoStsh(docMother))
			goto LRet;
		docPr = docUndo;
		break;

	case istGlossary:
		if (!FFillUndoGlsy(docMother))
			goto LRet;
		docPr = docUndo;
		break;

	case istAnnotation:
		if (PdodDoc(docMother)->docAtn == docNil ||
				!FFillUndoAtn(docMother, cpFirst, cpLim))
			goto LRet;
		docPr = docUndo;
		break;

	case istSummary:
		if (!FFillUndoSummaryInfo(docMother))
			goto LRet;
		docPr = docUndo;
		break;

	case istKeys:
		if (!FFillUndoKeys(docMother))
			goto LRet;
		docPr = docUndo;
		break;

	case istDocument:
		if (vprsu.prng == prngSel)
			{
			/* print selection */
			docPr = docUndo;
			SetWholeDoc(docPr, PcaSet(&ca,doc,cpFirst,cpLim));
			/* ensure chFtns and chAtns evaluate properly */
			vpri.cpAdjust = cpFirst;
			}
		else
			{
			docPr = docMother;
			Assert(!PdodDoc(docPr)->fShort);
			}
		break;
		}

	if (docPr == docUndo)
		{
		/* set dop of derivative doc to docMother's dop */
		pdod = PdodDoc(docUndo);
		pdodMother = PdodDoc(docMother);
		dop = pdod->dop;
		pdod->dop = pdodMother->dop;
		pdod->dop.grpfIhdt = 0; /* no special footnote separators */
		pdod->fMotherStsh = fTrue;      /* use docMother's stsh */
		pdod->doc = docMother;
		ftcMacMother = pdodMother->ftcMac;
		ftcMacSave = pdod->ftcMac;
		ftcMaxSave = pdod->ftcMax;
		hmpftcibstFontSave = pdod->hmpftcibstFont;
		pdod->ftcMac = ftcMacMother;
		pdod->ftcMax = pdodMother->ftcMax;
		pdod->hmpftcibstFont = pdodMother->hmpftcibstFont;
		}

	if (istPrSrc != istDocument)
		{
		vpgnFirst = vsectFirst = 1;
		vpgnLast = vsectLast = 0x7fff;
		}
	else  if (vsectFirst <= 1)
		{
		/* if printing doc, adjust starting page to true page number */
		CacheSect(docPr, cp0);
		vpgnFirst = max(vpgnFirst, vsepFetch.pgnStart);
		}

	if (vsectFirst != vsectLast || vpgnFirst <= vpgnLast)
		PrintDocCopies(docPr, cpFirst, cpLim, hppr);

	if (istPrSrc != istDocument)
		{
		vpgnFirst = pgnFirst;
		vpgnLast = pgnLast;
		vsectFirst = sectFirst;
		vsectLast = sectLast;
		}

	if (docPr == docUndo)
		{
		pdod = PdodDoc(docUndo);
		pdod->dop = dop;
		pdod->fMotherStsh = fFalse;
		pdod->doc = docNil;
		/* we assume the print did not add any fonts...if it did we would
			have to copy the entire font table from docUndo to docMother */
		Assert(ftcMacMother == pdod->ftcMac);
		pdod->ftcMac = ftcMacSave;
		pdod->ftcMax = ftcMaxSave;
		hmpftcibstFontSave = pdod->hmpftcibstFont;
		}

LRet:
	vlm = lmNil;
	vpri.fPrintingAtn = fFalse;
}


csconst char szStyleSheet[] = SzKey(" style sheet",StyleSheet);
csconst char szGlossary[] = SzKey(" glossary",Glossary);
csconst char szAnnotations[] = SzKey(" annotations",Annotations);
csconst char szSummaryInfo[] = SzKey(" summary information",SummaryInformation);
csconst char szKeyAssignments[] = SzKey(" key assignments",KeyAssignments);

/* %%Function:SetUpPrintPrompt %%Owner:davidbo */
SetUpPrintPrompt(doc, ist, pgn, phppr, fFirst)
int doc, ist, pgn, fFirst;
struct ***phppr;
{
	char pch [cchMaxSz];
	int rgw[2];

	Assert(doc >= docMinNormal);
	Assert(PdodDoc(doc)->fMother);
	rgw[0] = doc;

	if (ist == istDocument)
		{
		rgw[1] = fFirst ? 0 : pgn;
		}
	else
		{
		rgw[1] = pch;
		switch (ist)
			{
		case istDocument:
			break;
		case istStyles:
			CopyCsSz(szStyleSheet, pch);
			break;
		case istGlossary:
			CopyCsSz(szGlossary, pch);
			break;
		case istAnnotation:
			CopyCsSz(szAnnotations, pch);
			break;
		case istSummary:
			CopyCsSz(szSummaryInfo, pch);
			break;
		case istKeys:
			CopyCsSz(szKeyAssignments, pch);
			break;
		default:
			rgw[1] = NULL;
			break;
			}
		}

	*phppr = HpprStartProgressReport(((ist == istDocument) ? 
			(fFirst ? mstPrinting : mstPrintingPage) : 
	mstPrintingSub),rgw,1,fTrue);
}


/* Spooler callback function -- always continue with print! */
/* %%Function:FPrContinue %%Owner:davidmck */
EXPORT BOOL FAR PASCAL FPrContinue(hdc, iCode)
HDC hdc;
int iCode;
{
	MSG	msg;

	if (FQueryAbortCheck())
		return fFalse;

	/* Eat Key and sys key messages */
	while (PeekMessage((LPMSG) &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
		;

	return fTrue;
}


/* %%Function:PrintDocCopies %%Owner:davidbo */
PrintDocCopies(doc, cpFirst, cpLim, hppr)
int doc;
CP cpFirst, cpLim;
struct PPR **hppr;
{
	int cCopies, iEscape, fDraft;
	int docMother = DocMotherLayout(doc);
	struct CA ca;

	StartLongOp();

	cCopies = vprsu.cCopies;

	if (fDraft = vprsu.fDraft)
		{
		iEscape = Escape(vpri.hdc, DRAFTMODE, sizeof(BOOL), (LPSTR)&fDraft, (LPSTR)NULL);
#ifdef DAVIDBO
		CommSz(SzShared("DraftMode\n\r"));
#endif
		if (iEscape < 0 || (vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
			{
			EscapeError(iEscape);
			goto LErrorNoAbort;
			}
		}

	while (vpri.fPrErr == fFalse && cCopies-- > 0)
		{
		if (FQueryAbortCheck() || (vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
			goto LError;

		if (!FPrintDoc(doc, hppr))
			goto LError;

		/* Update statistics now in case summary info being printed */
		if (doc != docUndo)
			DsiToDopDoc(doc);

#ifdef DEBUG
		if (vprsu.fAnnotations || vprsu.fSummary)
			Assert(vprsu.istPrSrc == istDocument);
#endif

		if (vprsu.fAnnotations)
			{
			int docT = doc;

			if (doc == docUndo)
				docT = PdodDoc(doc)->doc;
			Assert(PdodDoc(docT)->fMother);
			vprsu.istPrSrc = istAnnotation;
			if (PdodDoc(docT)->docAtn != docNil && !FPrintAtn(docT, cpFirst, cpLim))
				{
				vprsu.istPrSrc = istDocument;
				goto LError;
				}
			vprsu.istPrSrc = istDocument;
			}

		if (vprsu.fSummary)
			{
			vprsu.istPrSrc = istSummary;
			if (!FPrintSummaryInfo(doc, cpFirst, cpLim))
				{
				vprsu.istPrSrc = istDocument;
				goto LError;
				}
			vprsu.istPrSrc = istDocument;
			}
		}

LEnd:
	if (vprsu.fDraft)
		Escape(vpri.hdc, DRAFTMODE, sizeof(BOOL), (LPSTR)&iEscape, (LPSTR)NULL);
	TrashAllWws();  /* page breaks might have changed, hence display */

LCancel:
	SetUndoNil();
	EndLongOp(fFalse);
	return;

LErrorMsg:
	ErrorEid(eidPRFAIL, "");
LError:
	Escape(vpri.hdc, ABORTDOC, 0, (LPSTR)NULL, (LPSTR)NULL);
#ifdef DAVIDBO
	CommSz(SzShared("AbortDoc\n\r"));
#endif
LErrorNoAbort:
	vpri.fPrErr = fTrue;
	goto LEnd;
}


/* %%Function:EscapeError %%Owner:davidbo */
EscapeError(iEscape)
int iEscape;
{
#ifdef DAVIDBO
	CommSzNum(SzShared("iEscape ="), iEscape);
#endif
	switch (iEscape)
		{
	default:
		if ((iEscape & SP_NOTREPORTED) == 0)
			break;
	case SP_ERROR:
		ErrorEid(eidCantPrint, "");
		break;
	case SP_APPABORT:
	case SP_USERABORT:
		break;
	case SP_OUTOFDISK:
		ErrorEid(eidPrDiskErr, "");
		vmerr.fDiskAlert = fTrue;
		break;
	case SP_OUTOFMEMORY:
		ErrorEid(eidPRFAIL, "");
		break;
		}
}


/*
*  FPrintDoc prints the requested document from (vSectStart, vPageStart)
*  to (vSectStop, vPageStop) inclusive.  Returns fTrue if the limiting
*  page was reached.
*/
/* %%Function:FPrintDoc %%Owner:davidbo */
FPrintDoc(doc, hppr)
int doc;
struct PPR **hppr;
{
	struct PLC **hplcpgd;
	int lbc = lbcNil;
	int ipgdFirst, docMother = DocMotherLayout(doc);
	int fEnd = fFalse, fFirst = fTrue;
	CP cpMacDoc = CpMacDoc(doc);
	int rgwIn[4];
	struct LBS lbsText, lbsFtn;
	struct RPL rpl;
	struct PGD pgd;

	/* set up format state */
	InvalFli();

	/* repaginate up to printing start position (end for reverse) */
	lbsText.hpllr = lbsFtn.hpllr = hNil;
	lbsText.hpllnbc = lbsFtn.hpllnbc = hNil;

	rpl.cp = cpMax;
	rpl.ipgd = pgnMax;
	rpl.ised = vsectFirst;
	rpl.pgn = vpgnFirst;
	LinkDocToWw(doc, wwLayout, wwNil);
	PwwdWw(wwLayout)->fOutline = (vprsu.istPrSrc == istDocument) && ((*hmwdCur)->doc == doc) && (*hwwdCur)->fOutline;

	PwwdWw(wwLayout)->fShowF = (*hwwdCur)->fShowF;
	if (!FUpdateHplcpgd(wwLayout, doc, &lbsText, &lbsFtn, &rpl, patAbort))
		goto LPrintDone;

	lbc = lbsText.lbc;
	if (lbc == lbcEndOfDoc)
		{
		fEnd = fTrue;
		goto LPrintDone;
		}

	hplcpgd = PdodDoc(doc)->hplcpgd;
	if (vprsu.fReverse)
		{
		ipgdFirst = rpl.ipgd;
		rpl.cp = cpMax;
		rpl.ipgd = pgnMax;
		rpl.ised = vsectLast;
		rpl.pgn = vpgnLast;
		LinkDocToWw(doc, wwLayout, wwNil);
		if (!FUpdateHplcpgd(wwCur, doc, &lbsText, &lbsFtn, &rpl, patAbort))
			goto LPrintDone;
		rpl.ised = rpl.pgn = pgnMax;
		/* one special case: verify that the page we're poised at
			is not in the wrong section */
		if (vsectLast != 0 && IInPlc(PdodDoc(doc)->hplcsed, lbsText.cp) >= vsectLast)
			rpl.ipgd--;
		if (rpl.ipgd < ipgdFirst)
			{
			fEnd = fTrue;
			goto LPrintDone;
			}
		/* check for nothing to print */
		GetPlc(hplcpgd, rpl.ipgd, &pgd);
		if (pgd.pgn > vpgnLast)
			{
			fEnd = fTrue;
			goto LPrintDone;
			}
		}

	/* print the document */
	for (; ; rpl.ipgd += (vprsu.fReverse) ? -1 : 1)
		{
		if (vprsu.fReverse)
			{
			/* keep backing up until we reach the starting page */
			if (rpl.ipgd < ipgdFirst)
				{
				fEnd = fTrue;
				break;
				}
			Assert(rpl.ipgd >= 0);
			LinkDocToWw(doc, wwLayout, wwNil);
			if (!FUpdateHplcpgd(wwLayout, doc, &lbsText, &lbsFtn,&rpl,patAbort))
				break;
			}
		else  if (lbc==lbcEndOfDoc || FPrintDone(&lbsText, vsectLast, vpgnLast))
			{
			fEnd = fTrue;
			break;
			}
		else
			SetPgdInvalid(rpl.ipgd, &lbsText, &lbsFtn);

		if (vpri.cBins > 0)
			{
			if (vprsu.iidBinPrFeed == iidBinMixed)
				rgwIn[0] = rpl.ipgd == 0 ?
						vpri.rgidBin[iidBin1] : vpri.rgidBin[iidBin2];
			else  /* bin1, bin2, bin3... */				
				{
				Assert(vprsu.iidBinPrFeed >= iidBinManual && vprsu.iidBinPrFeed <= iidBin3);
				rgwIn[0] = vpri.rgidBin[vprsu.iidBinPrFeed];
				}

			Assert(rgwIn[0] != idBinNil);
			rgwIn[0] |= 0x8000;
			if ((rgwIn[0] & 0x7fff) != vpri.idBinCur &&
					Escape(vpri.hdc, GETSETPAPERBINS, 4, (LPSTR)rgwIn, 0L) > 0)
				{
				vpri.idBinCur = rgwIn[0] & 0x7fff;
				}
			}

		GetPlc( hplcpgd, rpl.ipgd, &pgd );
#ifdef DAVIDBO
		CommSzNum(SzShared("pgd.pgn: "), pgd.pgn);
#endif /* DAVIDBO */
		if (vprsu.istPrSrc == istDocument)
			{
			if (fFirst)
				SetUpPrintPrompt(docMother, istDocument, pgd.pgn, &hppr,fFalse);
			else
				ChangeProgressReport(hppr, pgd.pgn);
			}
		fFirst = fFalse;

		/* format the page */
		if ((lbc = LbcFormatPage(&lbsText, &lbsFtn)) == lbcAbort)
			goto LPrintDone;

		if (!vprsu.fReverse)
			SetPgdValid(rpl.ipgd, &lbsText, &lbsFtn, fTrue);

		if (FQueryAbortCheck() || (vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
			goto LPrintDone;

		/* empty page */
		if (lbsText.fEmptyPage && lbsText.cp == cp0)
			{
			if (vprsu.fReverse)
				rpl.ipgd = 2;   /* 1, except decremented */
			continue;
			}

		/* print the page */
		if (!FSortAndDisplayLrs(&lbsText) || vpri.fPrErr)
			goto LPrintDone;
		}  /* loop to next page */

LPrintDone:
	/* clean up from printing */
	EndLayout(&lbsText, &lbsFtn);

	/* reset format state */
	ResetFont(fTrue);
	return(fEnd);
}


FLrGt(ilr0, ilr1)
{
	LRP lrp0 = LrpInPl(vhpllr, ilr0);
	LRP lrp1 = LrpInPl(vhpllr, ilr1);

	return(lrp0->yl > lrp1->yl || (lrp0->yl == lrp1->yl && lrp0->xl >lrp1->xl));
}


/* Break up the LRs into line LRs and display */
/* %%Function:FSortAndDisplayLrs %%Owner:davidbo */
FSortAndDisplayLrs(plbs)
struct LBS *plbs;
{
	int iilr, iilrMac;
	int *pilr, **hrgilr;
	uns dxaLnn;
	int xlRevBar, dyl, dylAbove, dylBelow, dxa, ylLnn;
	int iEscape, iEscape2;
	int fDone = fFalse;
	int fBinf = fFalse;
	int fGraphics, fOutlineWw, fOutlineLr, fLnn;
	int fShort, esc, fTop, fBottom, fTable;
	int fTextBand = fFalse;
	int docMother = DocMotherLayout(plbs->doc);
	int fCalcXlRevBar = fTrue;
	int irmBar = PdodMotherPrint(plbs->doc)->dop.irmBar;
	CP cpFirstLr;
	struct DOD *pdod;
	struct PLC **hplcpad = 0;
	struct LR lr;
	struct PAD pad;

	struct LNBC *plnbc;
	int ilnbc, ilnbcMac;
	int dxp = vfti.dxpBorder;
	int dxpO2 = dxp >> 1;
	HBRUSH hbrOld;

	struct BINF binfIn, binfOut;   /* from BANDINFO escape */

	binfIn.fGraphics = fTrue;
	binfIn.fText = fTrue;
	binfIn.rcGraphics.xpLeft   = 0;
	binfIn.rcGraphics.ypTop    = 0;
	binfIn.rcGraphics.xpRight  = vpri.dxpRealPage;
	binfIn.rcGraphics.ypBottom = vpri.dypRealPage;

	/* set up for drawing vertical lines between columns */
	ilnbcMac = (plbs->hpllnbc == hNil) ? 0 : (*plbs->hpllnbc)->ilnbcMac;
	if (ilnbcMac != 0)
		{
		plnbc = PInPl(plbs->hpllnbc, 0);
		for (ilnbc = 0; ilnbc < ilnbcMac; ilnbc++, plnbc++)
			{
			plnbc->xl -= vpri.ptUL.xp;
			plnbc->yl -= vpri.ptUL.yp;
			}
		}

	vhpllr = plbs->hpllr;
	/* create a sort vector and sort the line LRs */
	if ((hrgilr = HAllocateCw((iilrMac = (*vhpllr)->ilrMac) * sizeof(int))) == hNil)
		{
		goto LError;
		}
	for (pilr = *hrgilr, iilr = 0; iilr < iilrMac; )
		*pilr++ = iilr++;
	Sort(pilr = *hrgilr, iilrMac, FLrGt);

	/* print the layout rectangles */
	SetBkMode(vpri.hdc, TRANSPARENT);
	ResetFont(fTrue);
	if (plbs->fOutline)
		hplcpad = PdodDoc(plbs->doc)->hplcpad;

#ifdef DEBUG
	/* we are or'ing into the lr.fGraphics, so they better be all false
		when we start this. bz
	*/
	for (iilr = 0; iilr < iilrMac; iilr++)
		Assert (!LrpInPl(vhpllr,iilr)->fGraphics);
#endif /* DEBUG */

#ifdef DEBUG
	if (vdbs.fAllGraphicsBands)
		{
		for (iilr = 0; iilr < iilrMac; iilr++)
			LrpInPl(vhpllr,iilr)->fGraphics = fTrue;
		}
#endif

	/* for each band.... */
	esc = BANDINFO;
	fBinf = Escape(vpri.hdc, QUERYESCSUPPORT, 2, (LPINT) &esc, (LPSTR) NULL);
	for (;;)
		{
		iEscape = Escape(vpri.hdc, NEXTBAND, 0, (LPSTR)NULL, (LPSTR)&vrcBand);
		SetBkMode(vpri.hdc,TRANSPARENT); /* as NEXTBAND seems to set it to OPAQUE */
		/* if no more bands, done with page */
		if (vrcBand.ypTop >= vrcBand.ypBottom ||
				vrcBand.xpLeft >= vrcBand.xpRight)
			{
			ResetFont(fTrue);
			break;
			}

		if (plbs->fEmptyPage)
			continue;

		iEscape2 = fBinf ? Escape(vpri.hdc, BANDINFO, 0, (LPSTR)&binfIn, (LPSTR)&binfOut) : 0;

#ifdef DAVIDBO
		CommSzRgNum(SzShared("rcBand  "), &vrcBand, 4);
		CommSzNum(SzShared("NextBand return: "), iEscape);
		CommSzNum(SzShared("BandInfo return: "), iEscape2);
		CommSzNum(SzShared("\tfGraphics: "), binfOut.fGraphics);
		CommSzNum(SzShared("\tfText: "), binfOut.fText);
#endif

		if (FQueryAbortCheck() ||
				(vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
			{
			goto LEndPrint;
			}

		if (iEscape < 0)
			{
			EscapeError(iEscape);
			vpri.fPrErr = fTrue;
			goto LEndPrint;
			}

		if (iEscape2 > 0)  /* ok */
			{
			vpri.fGraphics = Usgn(binfOut.fGraphics);
			vpri.fText = Usgn(binfOut.fText);
			}
		else  /* error or not imp */			
			{
			vpri.fGraphics = fTrue;
			vpri.fText = fTrue;
			}

		/* The graphics band code requires we see a text band first */
		if (vpri.fText)
			fTextBand = fTrue;

		fOutlineWw = PwwdWw(wwLayout)->fOutline;
		vfli.fLayout = fTrue;

		/* Scan the sorted list of LRs */
		for (iilr = 0; iilr < iilrMac; iilr++)
			{
			struct LR lrLocal;
			LRP lrp;

			lr = *LrpInPl(vhpllr, (*hrgilr)[iilr]);
			/* store current lr for PostScript code */
			/* Need to use 2nd copy of lr (in lrLocal) because contents
				of lr change in this loop.  PostScript processing code
				needs starting values of lr, unchanged by printing offset or
				adjustments made in loop. */
			lrLocal = lr;
			vpri.plr = &lrLocal;

			/* translate lr from "paper" space to "page" space */
			lr.xl -= vpri.ptUL.xp;
			lr.yl -= vpri.ptUL.yp;

			/* If no more LRs in current band, go to the next band. */
			if (lr.yl > vrcBand.ypBottom)
				break;

			if (!FLrInBand(lr, xlRevBar))
				continue;

			/* skip if in graphics band but lr has no graphics */
			/* WARNING...this is only guaranteed with hppcl.drv */
			/* I think this will work with all printers except the
			   very perverted.  We want to skip only on a Graphics only
			   band and only if we have seen a Text Band (hopefully taking
			   up the entire page) */
			if (fBinf && vpri.fGraphics && !lr.fGraphics && !vpri.fText && fTextBand)
				continue;

			Assert(vfli.fLayout);
			vfli.fLayout = fFalse;
			if (FQueryAbortCheck() || (vflm != flmPrint && !FResetWwLayout(docMother, flmPrint, lmPrint)))
				goto LEndPrint;
			vfli.fLayout = fTrue;

			fOutlineLr = lr.ihdt == ihdtNil ? fOutlineWw : fFalse;
			PwwdWw(wwLayout)->fOutline = fOutlineLr;

			PwwdWw(wwLayout)->hdc = vpri.hdc;
			fGraphics = fFalse;

			if (lr.doc < 0)
				{
				Assert(vfli.fLayout);
				if (FFormatLineFspec(plbs->ww, plbs->doc, lr.dxl, -lr.doc))
					{
					fGraphics |= (vfli.fRMark | vfli.fPicture | vfli.grpfBrc | vfli.fGraphics) != 0;
					Assert((fGraphics & 0xfffe) == 0);
					LrpInPl(vhpllr, (*hrgilr)[iilr])->fGraphics |= fGraphics;
					vpri.fInPrintDisplay = fTrue;
					DisplayFliCore(wwLayout, vrcBand, lr.xl - vfli.xpLeft, lr.yl + vfli.dypLine);
					vpri.fInPrintDisplay = fFalse;
					}
				continue;
				}

			/* normal LR...format and display each line. */
			if (lr.cp == cpNil)
				continue;
			fShort = PdodDoc(lr.doc)->fShort;
			Assert(wwLayout == WwFromLbsDoc(plbs, lr.doc));
			RawLrCps(plbs, &lr, wwLayout);
			Assert(vfli.fLayout);

			dxa = NMultDiv(lr.dxl, dxaInch, vfli.dxuInch);
			for (cpFirstLr = lr.cp; lr.cp < lr.cpLim; )
				{
				/* this LR has no more lines in the current band */
				if (lr.yl > vrcBand.ypBottom)
					break;

				CachePara(lr.doc, lr.cp);
				fTable = FInTableVPapFetch(lr.doc, lr.cp);

/* outline mode and para not being shown */
				if (hplcpad && fOutlineLr)
					{
					GetPlc(hplcpad, IInPlc(hplcpad, lr.cp), &pad);
					if (!pad.fShow)
						{
						if (!fTable)
							lr.cp = caPara.cpLim;
						else
							{
							CpFirstTap1(lr.doc, lr.cp, fOutlineLr);
							lr.cp = caTap.cpLim;
							}
						continue;
						}
					}

/* table */
				if (fTable)
					{
					CachePara(lr.doc, lr.cp);
/* absolute text encountered during a non-abs table lr */
					if (!fOutlineLr && FAbsPap(lr.doc, &vpapFetch) && lr.lrk != lrkAbs)
						{
						CpFirstTap1(lr.doc, lr.cp, fOutlineLr);
						Assert(caTap.cpFirst == lr.cp);
						lr.cp = caTap.cpLim;
						continue;
						}
					CacheTc(wwLayout, lr.doc, lr.cp, fFalse, fFalse);
					CpFirstTap1(lr.doc, lr.cp, fOutlineLr);
					Assert(caTap.cpFirst == lr.cp);
					lr.cp = caTap.cpLim;
					fTop = vtcc.fFirstRow || caTap.cpFirst == cpFirstLr && lr.fForceFirstRow;
					CachePara(lr.doc, caTap.cpLim);
					/* may fail if row is followed by abs object */
					if (vtcc.fLastRow || (lr.lrk == lrkAbs && caTap.cpLim >= lr.cpLim))
						fBottom = fTrue;
					else  if (lr.cp != lr.cpLim)
						fBottom = fFalse;
					else  if ((*hrgilr)[iilr] == iilrMac - 1)
						fBottom = fTrue;
					else 						
						{
						LRP lrp;
						lrp = LrpInPl(vhpllr, (*hrgilr)[iilr]+1);
						fBottom = lr.xl != (lrp->xl - vpri.ptUL.xp) || lr.doc != lrp->doc;
						}
					dyl = DypHeightTable(wwLayout, lr.doc, caTap.cpFirst, fTop, fBottom, &dylAbove, &dylBelow);
					Assert(vfli.fLayout);
					ScanTableRow(wwLayout, lr.doc, caTap.cpFirst, DisplayLrTable, &lr, dylAbove);
					if ((vpri.fGraphics && !vpri.fDPR) || (vpri.fText && vpri.fDPR))
						{
						CacheTc(wwLayout, lr.doc, caTap.cpFirst, fTop, fBottom);
						PrintTableBorders(wwLayout, lr.xl, lr.yl, dyl, dylAbove, dylBelow, fTop,
								fBottom);
						}
					if (!vpri.fGraphics)
						LrpInPl(vhpllr, (*hrgilr)[iilr])->fGraphics |= dylAbove > 0 || lr.fGraphics;
					lr.yl += dyl + dylAbove + dylBelow;
					continue;
					}

/* absolute text encountered during a non-abs lr */
				if (hplcpad == hNil && FAbsPap(lr.doc, &vpapFetch) && lr.lrk != lrkAbs)
					{
					lr.cp = caPara.cpLim;
					continue;
					}

				/* Normal text...format a line */
				Assert(vfli.fLayout);
				FormatLineDxa(wwLayout, lr.doc, lr.cp, dxa);

				/* lr.cpMacCounted == cpNil, we are in the header/footer
					layout rectangles. */
				if (lr.cpMacCounted != cpNil && lr.cp >= lr.cpMacCounted)
					{
					UpdateDsi();
					lr.cpMacCounted = vfli.cpMac;
					}

				fGraphics |= (vfli.fRMark | vfli.fPicture | vfli.grpfBrc | vfli.fGraphics) != 0;
				if (lr.fSpaceBefore)
					{
					lr.yl -= vfli.dypBefore;
					lr.fSpaceBefore = fFalse;
					}

				lr.cp = vfli.cpMac;       /* advance for next line. */
				if (vfli.fSplatBreak && vfli.ichMac == 1)
					{
					if (fShort || lr.lrk == lrkAbs)
						{
						lr.yl += vfli.dypLine;
						continue;
						}
					Assert(lr.cp == lr.cpLim);
					break;
					}

				/*
				*  set here because following optimization would otherwise
				*  keep us from counting this line.
				*/
				fLnn = (lr.lnn != lnnNil && !vpapFetch.fNoLnn);
				if (fLnn)
					vfmtss.lnn = lr.lnn++;

				/*
				*  if the LR starts before the band and we haven't reached the
				*  top of the band yet, don't bother displaying
				*/
				lr.yl += vfli.dypLine;
				if (lr.yl < vrcBand.ypTop)
					continue;

				/* handle main text */
				vpri.fInPrintDisplay = fTrue;
				DisplayFliCore(wwLayout, vrcBand, lr.xl, lr.yl);
				vpri.fInPrintDisplay = fFalse;

				/* draw revision bar if needed */
				if (vfli.fRMark && vpri.fGraphics && irmBar != irmBarNone)
					{
					if (fCalcXlRevBar)
						{
						/* plbs->pgn refers to Next page */
						/* we need fOdd for this page */
						int fOdd = !(plbs->pgn & 1);
						/* if true, bar goes in left margin on this page */
						int fLeft;
						int xlLeft, xlRight;
						struct DOP *pdop;

						if (irmBar == irmBarOutside)
							fLeft = !(FFacingPages(plbs->doc) && fOdd);
						else
							fLeft = (irmBar == irmBarLeft);
						pdop = &PdodMotherPrint(plbs->doc)->dop;
						GetXlMargins(pdop, fOdd, vfli.dxuInch,
								&xlLeft, &xlRight);
						if (fLeft)
							{
							xlRevBar = xlLeft -
									NMultDiv(dxaEighth, vfli.dxuInch, dxaInch);
							}
						else
							{
							xlRevBar = xlRight +
									NMultDiv(dxaEighth, vfli.dxuInch, dxaInch);
							}
						xlRevBar -= vpri.ptUL.xp;
						fCalcXlRevBar = fFalse;
						}
					DrawRevBar(vpri.hdc, xlRevBar, lr.yl - vfli.dypLine,
							vfli.dypLine, NULL /* prcwClip */);
					}

				if (!fLnn)
					continue;

				CacheSect(lr.doc, vfli.cpMin);
				if (vfmtss.lnn % vsepFetch.nLnnMod)
					continue;

#ifdef DAVIDBO
				CommSzNum(SzShared("lnn: "), vfmtss.lnn);
				CommSzNum(SzShared("\tyl: "), ylLnn);
#endif
				/* handle line number */
				dxaLnn = (vsepFetch.dxaLnn) ? vsepFetch.dxaLnn :
						((vsepFetch.ccolM1) ? dxaInch / 8 : dxaInch / 4);
				ylLnn = lr.yl - vfli.dypBase;
				Assert(vfli.fLayout);
				if (FFormatLineFspec(plbs->ww, plbs->doc, 0, chLnn))
					{
					int xlRight, xlLnn;
					struct RC rcLnn;

					rcLnn = vrcBand;
					xlRight = vfli.xpRight - vfli.rgdxp[vfli.ichMac - 1];
					xlLnn = lr.xl-xlRight-NMultDiv(dxaLnn,vfli.dxuInch,czaInch);
					rcLnn.xpLeft = xlLnn;
					rcLnn.xpRight = lr.xl;
					vpri.fInPrintDisplay = fTrue;
					DisplayFliCore(wwLayout, vrcBand, xlLnn,ylLnn+vfli.dypBase);
					vpri.fInPrintDisplay = fFalse;
					fGraphics |= (vfli.fGraphics != 0);
					}
				}

			/* We have counted up to this lr.cpMacCounted, don't count
				that part again. */
			lrp = LrpInPl(vhpllr, (*hrgilr)[iilr]);
			lrp->cpMacCounted = lr.cpMacCounted;
			/* note: or'ed in so multiple bands won't wipe out bit bz */
			Assert((fGraphics & 0xfffe) == 0);
			lrp->fGraphics |= fGraphics;
			}

		PwwdWw(wwLayout)->fOutline = fOutlineWw;
		vfli.fLayout = fFalse;

		/* draw vertical lines between columns */
		if (ilnbcMac != 0 && vpri.fGraphics)
			{
			hbrOld = SelectObject(vpri.hdc, vpri.hbrText);
			plnbc = PInPl(plbs->hpllnbc, 0);
			for (ilnbc = 0; ilnbc < ilnbcMac; ilnbc++, plnbc++)
				{
				if (FLnbcInBand(*plnbc, dxpO2))
					PatBlt(vpri.hdc, plnbc->xl - dxpO2, plnbc->yl,
							dxp, plnbc->dyl, PATCOPY);
				}
			if (hbrOld != NULL)
				SelectObject(vpri.hdc, hbrOld);
			}

		}
	fDone = fTrue;
LEndPrint:
	FreeH(hrgilr);
LError:
	vfli.fLayout = fFalse;
	return fDone;
}


/*
*  Returns fTrue if the intersection of vrcBand and lr is not empty.
*/
/* %%Function:FLrInBand %%Owner:davidmck */
FLrInBand(lr, xpRevBar)
struct LR lr;
int xpRevBar;
{
	CP cpCur;

	/* The more usual checks are first */
	if (/* lr.yl > vrcBand.ypBottom || */ lr.yl + lr.dyl < vrcBand.ypTop)
		return fFalse;

	if (lr.xl <= vrcBand.xpRight && lr.xl + lr.dxl >= vrcBand.xpLeft)
		return fTrue;

	/* Would revision bars go in this band? */
	if (xpRevBar <= vrcBand.xpRight && xpRevBar >= vrcBand.xpLeft)
		return fTrue;

	/*See if we have any paragraph borders (yet another hassle) */
	if (lr.doc > 0)
		{
		cpCur = lr.cp;
		do
			{
			int ibrc;

			CachePara(lr.doc, cpCur);
			for (ibrc = 0 ; ibrc < ibrcPapLim ; ibrc++)
				if (vpapFetch.rgbrc[ibrc] != brcNone)
					return fTrue;

			cpCur = caPara.cpLim;
			} 
		while (cpCur < lr.cpLim);
		}

	/* Removed this part of the if because of italics in
	   landscape mode (not a hit in portrait */
	/* lr.xl + lr.dxl < vrcBand.xpLeft || */
			/* This is a hack to avoid problems in landscape mode with
			   line numbers not being printed in their band */
	if (lr.xl > vrcBand.xpRight && lr.lnn == lnnNil)
		return fFalse;

	return fTrue;
}


/*
* Returns fTrue if lnbc intersects vrcBand
*/
/* %%Function:FLnbcInBand %%Owner:davidbo */
FLnbcInBand(lnbc, dxpO2)
struct LNBC lnbc;
int dxpO2;
{
	if (lnbc.yl > vrcBand.ypBottom || lnbc.yl + lnbc.dyl < vrcBand.ypTop ||
			lnbc.xl - dxpO2 > vrcBand.xpRight || lnbc.xl + dxpO2 - 1 < vrcBand.xpLeft)
		return fFalse;
	return fTrue;
}



/************************/
/* C o m p r e s s  S t */
/* %%Function:CompressSt %%Owner:davidbo */
CompressSt(st)
char st[];
{
	/* removes all white space from string st */
	int cch, cchMac, cchCompress;
	char *pchCompress, *pchScan;

	pchCompress = pchScan = &st[1];
	cchMac = st[0];
	cchCompress = 0;
	for (cch = 0; cch < cchMac; cch++)
		{
		if (WbFromCh(*pchScan) == wbWhite)
			pchScan++;
		else
			{
			*pchCompress++ = *pchScan++;
			cchCompress++;
			}
		}
	st[0] = cchCompress;
}


/**********************************/
/* F  C h e c k  P a g e  S e c t */
/* %%Function:FCheckPageSect %%Owner:davidbo */
FCheckPageSect(st)
char st[];
/* parse a page/section number
	valid forms are: ppp, dsss, pppdsss, pppd, and null
	where: ppp = page number, d = 'S' or 's', sss = section number
	the number may be preceded with 'P' or 'p', which is ignored
	a single space may be used between the page number and 's': "1 s2"
*/
{
	int cch = 0, cchMac;
	char ch, *pch;

	CompressSt(st);
	pch = &st[1];
	cchMac = st[0];
	if (cchMac == 0)
		return fTrue;
	if (ChLower(*pch) == chGGPage)
		{
		cch++;
		pch++;
		}
	for (; cch < cchMac; cch++, pch++)
		{
		ch = ChLower(*pch);
		if (ch == chGGSection)
			{
			cch++;
			pch++;
			if (cch == cchMac)
				goto LBadPageSect;
			break;
			}
		if (ch < '0' || ch > '9')
			goto LBadPageSect;
		}
	for (; cch < cchMac; cch++, pch++)
		if (*pch < '0' || *pch > '9')
			goto LBadPageSect;
	return(fTrue);
LBadPageSect:
	return(fFalse);
}


/**********************************/
/* P a g e  S e c t  F r o m  S t */
/* %%Function:PageSectFromSt %%Owner:davidbo */
PageSectFromSt(st, ppgn, pscn/*, fFirst*/)
char *st;
int *ppgn, *pscn/*, fFirst*/;
	/* convert a page/section string to the corresponding page/section numbers--
		valid forms same as above
		default for an omitted number is zero
	*/
{
	int pgn = 0, scn = 0;
	int cch = 0, cchMac;
	char ch, *pch;

	CompressSt(st);
	cchMac = st[0];
	/* Nothing there? return page 1, section 1 */
	if (cchMac == 0)
		{
		/*pgn = scn = fFirst ? 1 : 0x7fff;*/
		pgn = scn = 0;     /* the invalid value */
		/* fix for #2731 */
		goto LRet;
		}
	pch = &st[1];
	if (cch == cchMac)
		goto LRet;
	if (ChLower(*pch) == chGGPage)
		{
		cch++;
		pch++;
		}
	for (; cch < cchMac; cch++, pch++)
		{
		ch = ChLower(*pch);
		if (ch == chGGSection)
			break;
		pgn = pgn * 10 + ch - '0';
		}
	if (cch == cchMac)
		goto LRet;
	Assert(ch == chGGSection);
	cch++;
	pch++;
	for (; cch < cchMac; cch++, pch++)
		scn = scn * 10 + *pch - '0';
LRet:
	*ppgn = pgn;
	*pscn = scn;
}


/* F  M U S T  P R E P A G I N A T E */
/*  Returns true if doc must be paginated before it can be printed.
	True if fltPage in doc or docFtn; if fltPageRef anywhere.
*/
/* %%Function:FMustPrePaginate %%Owner:davidbo */
FMustPrePaginate (doc)
int doc;
{
	struct DOD *pdod = PdodDoc (doc);
	FreezeHp ();
	if (FContainsFlts (doc, fltPage, fltPageRef) ||
			FContainsFlts (pdod->docFtn, fltPage, fltPageRef) ||
			FContainsFlts (pdod->docHdr, fltPageRef, fltNil))
		{
		MeltHp ();
		return fTrue;
		}
	MeltHp ();
	return fFalse;
}


/* F  C O N T A I N S  F L T S */
/*  Return true if doc contains either flt1 or flt2.  Either may be
	fltNil.  Returns false if doc == docNil.
*/
/* %%Function:FContainsFlts %%Owner:davidbo */
FContainsFlts (doc, flt1, flt2)
int doc, flt1, flt2;

{
	int ifld;
	struct PLC **hplcfld;
	struct FLD fld;

	if (doc == docNil || (hplcfld = PdodDoc (doc)->hplcfld) == hNil)
		return fFalse;

	for ( ifld = 0 ; ifld < IMacPlc( hplcfld ); ifld++ )
		{
		GetPlc( hplcfld, ifld, &fld );

		if (fld.ch == chFieldBegin)
			{
			Assert (fld.flt != fltNil);
			if (fld.flt == flt1 || fld.flt == flt2)
				return fTrue;
			}
		}

	return fFalse;
}


/* *****  Misc. Repag. stuff that is not part of background repag ***** */
/******************************/
/* C m d  R e p a g i n a t e */
/* %%Function:CmdRepaginate %%Owner:davidbo */
CMD CmdRepaginate(pcmb)
CMB *pcmb;
{
	struct RPL rpl;
	BOOL f;

	if (vhwndPgPrvw)
		{
		HDC hdc = GetDC(vhwndPgPrvw);

		if (hdc == NULL)
			f = fFalse;
		else
			{
			f = FSetModeNorm(hdc, fFalse);
			ReleaseDC(vhwndPgPrvw, hdc);
			}
		return f ? cmdOK : cmdError;
		}

	if (PwwdWw(selCur.ww)->fPageView)
		{ /* pageview: mash the drs, UpdateWw does the rest */
		int doc = DocMother(selCur.doc);

		InvalPageView(doc);
		TrashWwsForDoc(doc);
		if (doc != selCur.doc)
			{
			InvalPageView(selCur.doc);
			TrashWwsForDoc(selCur.doc);
			}
		f = fTrue;
		goto LRet;
		}

	SetWords(&rpl, pgnMax, cwRPL);
	rpl.cp = cpMax;
#ifdef PROFILE
		{
		(vpfi == pfiRepag ? StartProf(30) : 0);
		f = FRepaginateDoc(selCur.doc, &rpl, patAbort);
		(vpfi == pfiRepag ? StopProf() : 0);
		}
#else /* NOT PROFILE */
	f = FRepaginateDoc(selCur.doc, &rpl, patAbort);
#endif /* NOT PROFILE */

	if (f)
		{ /* soft page break will change, cause redisplay */
		TrashWwsForDoc(selCur.doc);
		}
LRet:
	SetUndoNil();
	return f ? cmdOK : cmdError;
}


/******************************/
/* F  R e p a g i n a t e  D o c */
/* Do an explicit repaginate (other than via printing).  Used by
	the menu command Repaginate and by the Index and TOC code.
	Returns fFalse if user aborted, fTrue otherwise
*/
/* %%Function:FRepaginateDoc %%Owner:davidbo */
BOOL FRepaginateDoc(doc, prpl, pat)
int doc;
struct RPL *prpl;
int pat;
{
	BOOL fCompleted;
	int docMother = DocMotherLayout(doc);
	int lmSave, flmSave;
	struct LBS lbsText, lbsFtn;
	struct WWD *pwwd;

	lmSave = vlm;
	flmSave = vflm;
	StartLongOp();
	if (!FInitWwLbsForRepag(wwCur, docMother, lmFRepag, &lbsText, &lbsFtn))
		{
		fCompleted = fFalse;
		goto LErrRet;
		}

	vpri.wmm = 0;
	fCompleted = FUpdateHplcpgd(wwLayout, docMother, &lbsText, &lbsFtn, prpl, pat);
	EndLayout(&lbsText, &lbsFtn);

LErrRet:
	EndFliLayout(lmSave, flmSave);
	EndLongOp(fFalse);
	return fCompleted;
}




/* *****  Summary Info routines  ***** */

/* %%Function:InitDsi %%Owner:davidbo */
InitDsi()
{
	dsi.fFullPrint = vprsu.fPrintAllPages && vprsu.istPrSrc == istDocument;
	dsi.cWords     = 0L;
	dsi.cCh        = 0L;
}


/* %%Function:DsiToDopDoc %%Owner:davidbo */
DsiToDopDoc(doc)
{
	extern int cchAvgWord10;

	if (dsi.fFullPrint)
		{
		struct DOD     *pdod;
		struct DOP     *pdop;
		struct PLC **hplcpgd;

		pdod = PdodDoc(doc);
		pdop = &(pdod->dop);
		pdop->cWords        = dsi.cWords;
		pdop->cCh           = dsi.cCh;
		pdop->fExactCWords  = fTrue;
		/* print date set before print */

		hplcpgd = pdod->hplcpgd;
		if (hplcpgd == hNil)
			{
			pdop->cPg = 1;
			}
		else
			{
			pdop->cPg = (*hplcpgd)->iMac;
			}
	/* Calculate the ratio. */
		if (dsi.cWords > 0L)
			{
			if (dsi.cCh * 10L < dsi.cCh)
				{
				cchAvgWord10 = (int) ((dsi.cCh / dsi.cWords) * 10L);
				}
			else
				{
				cchAvgWord10 = (int) ((dsi.cCh * 10L) /dsi.cWords);
				}
			}
#ifdef BOGUS
	/* don't dirty the doc: otherwise opening and printing will prompt
		the user to save, which is very annoying. it's true that you
		can lose the last print date this way, but that's not as bad. */
		pdod->fDirty = fTrue;
#endif
		}
	dsi.fFullPrint = fFalse;
}


/* %%Function:UpdateDsi %%Owner:davidbo */
UpdateDsi()
{
	if (dsi.fFullPrint)
		{
		CHAR *pch = vfli.rgch;
		CHAR *pchLim = &vfli.rgch[vfli.ichMac];
		CHAR *pchHyph = pch;
		BOOL  fSpace = fTrue;
		int   cWords = 0;

		dsi.cCh += (long)(vfli.ichMac);

		while (pch < pchLim)
			{
			switch (*pch++)
				{
			case chSpace:
			case chNonBreakSpace:
			case chEop:
#ifdef CRLF
			case chReturn:
#endif
			case chSect:
			case chTable:
			case chTab:
			case chCRJ:
			case chPicture:
				fSpace = fTrue;
				break;
			case chHyphen:
				pchHyph = pch;
				/* fall through */
			default:
				if (fSpace)
					{
					cWords++;
					fSpace = fFalse;
					}
				break;
				}
			}
		if (pchHyph == pchLim)
			{
			cWords--; /* Hyphen at the end of line. */
			}
		dsi.cWords += cWords;
		}
}


/* P r i n t  X  L i n e */
/* draw a horz bar on the printer by putting out underlined spaces;
	this will draw sucessfully without invoking graphics */

/* %%Function:PrintXLine %%Owner:davidbo */
PrintXLine( hdc, xpFirst, yp, xpLim )
HDC hdc;
int xpFirst, yp, xpLim;
{
	int dxp, dxpSpace;
	int cch;
	int ypTopSpace;
	int bkm;
	char rgch[2];
	int rgdxp [2];
	union FCID	fcidOld;
	int fChangedFont = fFalse;
	int fOldMethod = fTrue;

	if (xpLim <= xpFirst)
		return;

	Assert( vfti.hfont != NULL );
	dxpSpace = vfti.rgdxp[(int) ' '];
	dxp = xpLim - xpFirst;
	if (vfti.dxpExpanded < 0)
		dxp -= vfti.dxpExpanded;

	ypTopSpace = yp - vfti.dypAscent;
	cch = 1;

	Assert (vulm != ulmNil);

	if (vfti.fcid.fStrike)
		{
		union FCID fcidNew;
		/* Can't have the underline struckthrough */
		fChangedFont = fTrue;
		fcidOld = vfti.fcid;
		fcidNew = fcidOld;
		fcidNew.fStrike = fFalse;
		LoadFcidFull(fcidNew);
		}

	if (vulm == ulmAlwaysSpace)
		fOldMethod = fFalse;
	else  if (vulm == ulmNormal)
		{
		int ibst = vfti.fcid.ibstFont;
		struct FFN *pffn;

		Assert( ibst < (*vhsttbFont)->ibstMac );
		pffn = PstFromSttb( vhsttbFont, ibst );
		if (!pffn->fRaster && pffn->fGraphics)
			fOldMethod = fFalse;
		}

	/* Yes, this looks really ugly.  Some printers fail
		on printing expanded spaces.  Those printers can
		use DoubleUnderlineMode=1 in their win.ini to
		get better results */
	if (fOldMethod)
		{
		*((int *)rgch) = 0x2020;
		if ((rgdxp [0] = dxp) > dxpSpace)
			{
			cch++;
			rgdxp [0] = dxp - (rgdxp [1] = dxpSpace);
			}
		ExtTextOut( hdc, xpFirst, ypTopSpace, 0,
				(LPRECT) 0,
				(LPCH)rgch,
				cch,
				(LPINT)rgdxp );
		}
	else
		{
		PrintUlFaked(hdc, xpFirst, ypTopSpace, dxp, 0, (LPRECT) NULL);
		}

	if (fChangedFont)
		LoadFcidFull(fcidOld);

}


/* P R I N T  U L  L I N E */
/* Deals with printing a continuous underline */
export
PrintUlLine(hdc, xp, yp, eto, lprcClip, lpdxp, cch)
HDC hdc;
int xp, yp, eto, cch;
LPRECT lprcClip;
int far *lpdxp;
{
	int dxp;

	Assert(vfli.fPrint);
	Assert(lpdxp);
	Assert(vfti.fcid.kul || vfti.fcid.fStrike);

	Assert(vulm != ulmNil);

	if (vulm == ulmNeverSpace)
		return;

	if (vulm == ulmNormal)
		{
		int ibst = vfti.fcid.ibstFont;
		struct FFN *pffn;

		Assert( ibst < (*vhsttbFont)->ibstMac );
		pffn = PstFromSttb( vhsttbFont, ibst );
		if (pffn->fRaster || !pffn->fGraphics)
			return;
		}

	/* Add up all the space in the array of character widths */
	for ( dxp = 0; cch > 0 ; cch--)
		dxp += *lpdxp++;

	PrintUlFaked(hdc, xp, yp, dxp, eto, lprcClip);

}


/* P R I N T  U L  F A K E D */
/* Code is here so that underlining and double underlining
   can both take advantage of it */
PrintUlFaked(hdc, xpFirst, yp, dxp, eto, lprcClip)
HDC hdc;
int xpFirst, yp, dxp, eto;
LPRECT lprcClip;
{
	int dxpSpace;
	int c;
	int cchLast;
	int xp;
	char rgch[32];

	dxpSpace = vfti.rgdxp[(int) ' '];
	xp = xpFirst;

	SetBytes(&rgch[0], ' ', 32);
	c = dxp / dxpSpace;
	cchLast = c & 31;
	c >>= 5;

	while (c-- > 0)
		{
		ExtTextOut( hdc, xp, yp, eto, lprcClip, (LPCH) rgch, 32 , (LPINT) 0);
		xp += dxpSpace << 5;
		}

	ExtTextOut( hdc, xp, yp, eto, lprcClip, (LPCH) rgch, cchLast, (LPINT) 0 );

	ExtTextOut(hdc, xpFirst + dxp - dxpSpace, yp, eto, lprcClip, (LPCH) rgch, 1, (LPINT) 0);

}


/* E N A B L E  H O T  S P O T S */
/* Disable the hot spots in FBeginPrintJob then enable the hot spots in
   EndPrintJob so that while printing and the app is deactivated by another
   app, if the user click in these hot spots, the wndproc of these hot
   spots will not perform action that mess up layout.
	
   Hot spots includes :
   ribbon, ruler, scroll bars, iconbar, page view icons
   mw's and ww's hot spots.
*/
/* %%Function:EnableHotSpots %%Owner:chic */
EnableHotSpots(fEnable)
BOOL fEnable;
{
	if (selCur.doc == docNil) /* no doc opened, no work to do */
		return;

	if (vhwndPgPrvw) /* in preview mode */
		{
		/* this is easy since the preview window covers the app window */
		EnableWindow(vhwndPrvwIconBar, fEnable);
		EnableWindow(vhwndPgPrvwScrl, fEnable);
		return;
		}

	if (vhwndRibbon)
		EnableIconAndCombo(vhwndRibbon, IDLKSFONT, fEnable);
	else if (vhwndAppIconBar)	/* macro icon bar */
		EnableWindow(vhwndAppIconBar, fEnable);

	if (vpref.fZoomMwd) /* great! don't have to loop through all windows */
		EnableMwHotSpots(mwCur, fEnable);
	else
		{
		int mw;
		for (mw = mwDocMin; mw < mwMac; mw++)
			{
			if (mpmwhmwd[mw] != hNil)
				EnableMwHotSpots(mw, fEnable);
			}
		}
}


/* %%Function:EnableMwHotSpots %%Owner:chic */
EnableMwHotSpots(mw, fEnable)
int mw;
BOOL fEnable;
{
/* hot spots in mw includes:
	Ruler (one in mw)
	Horizontal scroll bar
	Splitbox, splitbar, sizebox
	Hot spots in ww.
*/
	struct MWD *pmwd = PmwdMw(mw);
	int ww;

	FreezeHp();
	if (pmwd->hwndRuler)
		EnableIconAndCombo(pmwd->hwndRuler, idRulStyle, fEnable);
	if (pmwd->hwndHScroll)
		EnableWindow(pmwd->hwndHScroll, fEnable);
	if (pmwd->hwndSplitBox)
		EnableWindow(pmwd->hwndSplitBox, fEnable);
	if (pmwd->hwndSizeBox)
		EnableWindow(pmwd->hwndSizeBox, fEnable);
	if (pmwd->hwndSplitBar)
		EnableWindow(pmwd->hwndSplitBar, fEnable);

	EnableWwHotSpots(pmwd->wwUpper, fEnable);
	if ((ww = pmwd->wwLower) != wwNil)
		EnableWwHotSpots(ww, fEnable);
	MeltHp();
}



/* %%Function:EnableWwHotSpots %%Owner:chic */
EnableWwHotSpots(ww, fEnable)
int ww;
BOOL fEnable;
{
/*	Hot spots in ww includes:
	Vertical scroll bar
	Iconbar - in outline or header/footer/footnote separator window
	Page icons on vertical scroll bar - if in page view window only
*/
	struct WWD *pwwd = PwwdWw(ww);
	if (pwwd->hwndVScroll)
		EnableWindow(pwwd->hwndVScroll, fEnable);
	if (pwwd->hwndIconBar)
		EnableWindow(pwwd->hwndIconBar, fEnable);
	if (pwwd->hwndPgvUp)
		{
		EnableWindow(pwwd->hwndPgvUp, fEnable);
		Assert(pwwd->hwndPgvDown);
		EnableWindow(pwwd->hwndPgvDown, fEnable);
		}
}



