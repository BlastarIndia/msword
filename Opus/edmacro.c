/*#define DBG*/

/* edmacro.c */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "resource.h"
#include "wininfo.h"
#include "opuscmd.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "screen.h"
#include "el.h"
#include "compare.h"
#include "rerr.h"
#include "keys.h"
#include "ibdefs.h"
#include "macrocmd.h"
#include "error.h"
#include "message.h"
#include "props.h"
#include "prm.h"
#include "sel.h"
#include "rareflag.h"

#include "iconbar.h"

#ifdef NOPE /* el.h includes these */
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#endif
#include "sdmtmpl.h"
#include "idd.h"
#include "tmc.h"

#include "edmacro.hs"
#include "edmacro.sdm"
#include "runmacro.hs"
#include "runmacro.sdm"
#include "renmacro.hs"
#include "renmacro.sdm"
#include "showvars.hs"
#include "showvars.sdm"

#include "props.h"


#ifdef PROTOTYPE
#include "edmacro.cpt"
#endif /* PROTOTYPE */

extern SY * PsyGetSyFromBcmElg();

extern int vgrfMenuCmdsAreDirty;
extern vrerr;
extern BOOL vfElDisableInput;
extern HANDLE vhInstance;
extern char szEmpty [];
extern HWND vhDlg;
extern HWND vhwndApp;
extern HWND vhwndDeskTop;
extern HWND vhwndRibbon;
extern struct SCI vsci;
extern HANDLE hInstance;
extern struct SEL selCur;
extern struct UAB vuab;
extern struct MWD ** hmwdCur;
extern int docRecord;
extern int docGlobalDot;
extern struct DOD ** mpdochdod [];
extern struct MWD ** mpmwhmwd [];
extern struct WWD ** mpwwhwwd [];
extern int mwCur;
/* extern */ HWND vhwndAppIconBar;
extern struct PREF vpref;
extern struct SEL selMacro;
extern struct MERR      vmerr;
extern HWND	vhwndCBT;
extern BCM vbcmFetch;


/* CAB Extension for ReName macro dialog (and set var) */
typedef struct _cern
	{
	char * szPrompt;
	char * stOrig;
	BOOL fNumber;
	int docDot;
} CERN;





WORD CchReadEdit();


MES ** vhmes = hNil;

BOOL vfRecording = fFalse;
BOOL vfMcrRunning = fFalse;

int vcSubNest;
int docRecord = docNil;
CP cpRecord = cp0;


csconst char stSubMain[] = StKey("Sub MAIN", Sub);
csconst char stSuper[] = StKey("Super ", Super);
csconst char stEndSub[] = StKey("End Sub", EndSub);
csconst char stDimAs[] = StKey("Dim dlg As ", DimAs);
csconst char stGetCurValsDlg[] = StKey("GetCurValues dlg", GetCurValsDlg);
csconst char stDialogDlg[] = StKey("Dialog dlg", DialogDlg);
csconst char stDlg[] = StKey(" dlg", SpDlg);
csconst char stMacroRun[] = StKey("MacroRun \"", MacroRun);


/* The MacroEdit Command */

/* C M D  E D I T  M A C R O */
/* %%Function:CmdEditMacro %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdEditMacro(pcmb)
CMB * pcmb;
{
	int mct;

	if (pcmb->pv == (void *) 0)
		pcmb->pv = &mct;
	
	mct = mctGlobalMacro;

	if (!FInitMacroDoc())
		{
		ErrorEid(eidNoMemory, "CmdEditMacro");
		return cmdError;
		}

	if (FCmdFillCab())
		{
		CABEDMACRO * pcab;

		FSetCabSz(pcmb->hcab, szEmpty, Iag(CABEDMACRO, hszName));
		FSetCabSz(pcmb->hcab, szEmpty, Iag(CABEDMACRO, hszDesc));
		FSetCabSz(pcmb->hcab, szEmpty, Iag(CABEDMACRO, hszNewName));
		pcab = *pcmb->hcab;
		pcab->iContext = iContextGlobal;
		pcab->fShowAll = vpref.fShowAllMacros;
		pcab->sab = 0;
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltEdMacro)];

		BltDlt(dltEdMacro, dlt);

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
			/* break; IE */;
			}
		}

	if (pcmb->fCheck)
		{
		int vmn;
		char stName [cchMaxMacroName + 1];

		GetCabSt(pcmb->hcab, stName, cchMaxMacroName + 1, 
				Iag(CABEDMACRO, hszName));
		if ((vmn = VmnValidMacroName(stName)) == vmnBad)
			RtError(rerrIllegalFunctionCall);

		if (FIsRecorderSt(stName))
			{
			ErrorEid(eidCantEdRecording, "CmdEditMacro");
			return cmdError;
			}

		if (((CABEDMACRO *) *pcmb->hcab)->iContext == iContextGlobal ?
				docGlobalDot == docNil : FDisableTemplate(selCur.doc))
			{
			ModeError();
			}

		switch (pcmb->tmc)
			{
		default:
			ModeError();
			Assert(fFalse); /* NOT REACHED */

		case tmcOK:
		case tmcSetDesc:
		case tmcDelete:
			break;

		case tmcRename:
			GetCabSt(pcmb->hcab, stName, cchMaxMacroName + 1, 
					Iag(CABEDMACRO, hszNewName));
			if ((vmn = VmnValidMacroName(stName)) == vmnBad ||
					vmn == vmnExists)
				{
				RtError(rerrIllegalFunctionCall);
				}
			break;
			}
		}

	if (pcmb->fAction)
		{
		switch (pcmb->tmc)
			{
		case tmcOK:
			{
#ifdef DEBUG
			int cch;
#endif
			uns bsy;
			char stName [cchMaxMacroName + 1];
			char stDesc [cchMaxSt];

			GetCabSt(pcmb->hcab, stName, cchMaxMacroName + 1, 
				Iag(CABEDMACRO, hszName));
			
			/* Get the propper capitalization... */
			if ((bsy = BcmOfSt(stName)) != bsyNil)
				{
#ifdef DEBUG
				cch = stName[0];
#endif
				GetNameFromBsy(stName, bsy);
				Assert(cch == stName[0]);
				}
			
				
			GetCabSt(pcmb->hcab, stDesc, sizeof (stDesc), 
				Iag(CABEDMACRO, hszDesc));

			if (!FEditMacro(stName, ((CABEDMACRO *) *pcmb->hcab)->iContext, 
					stDesc[0] == '\0' ? NULL : stDesc))
				{
				return cmdError;
				}
			}
			break;

		case tmcSetDesc:
				{
				BCM bcm;
				int docDot;
				CHAR st [cchMaxSt];

				GetNameBcmDocDotFromCab(st, &bcm, &docDot, 
						pcmb->hcab);

				if (bcm == bcmNil)
					{
					if ((bcm = BsyEnsureMacroExists(st, docDot, 
							NULL)) == bsyNil)
						{
						ErrorEid(eidNoMemory, "CmdEditMacro");
						break;
						}
					}

				if (bcm < bcmMacStd)
					{
					ErrorEid(eidCantChangeMenuHelp, 
							"CmdEditMacro");
					return cmdError;
					}

				GetCabSt(pcmb->hcab, st, sizeof (st), 
						Iag(CABEDMACRO, hszDesc));
				if (!FSetMenuHelp(bcm, st))
					{
				/* We are OOM! */
					ErrorEid(eidNoMemory, "CmdEditMacro");
					return cmdError;
					}

				DirtyDoc(docDot);
				}
			break;

		case tmcDelete:
		case tmcRename:
				{
				BCM bcm;
				int docDot;
				CHAR st [cchMaxMacroName + 1];

				GetNameBcmDocDotFromCab(st, &bcm, &docDot, 
						pcmb->hcab);

				if (bcm == bcmNil)
					{
					ErrorEid(eidNoSuchMacro, "CmdEditMacro");
					return cmdError;
					}

				if (bcm < bcmMacStd)
					{
					ErrorEid(eidCantRenDelCmd, "CmdEditMacro");
					return cmdError;
					}

				if (pcmb->tmc == tmcDelete)
					{
					if (!FDeleteMacro(st, docDot))
						return cmdError;
					}
				else
					{
					char stNewName [cchMaxMacroName + 1];
					char stDesc [256];

					GetCabSt(pcmb->hcab, stNewName,
							sizeof (stNewName),
							Iag(CABEDMACRO, hszNewName));
					GetCabSt(pcmb->hcab, stDesc, sizeof (stDesc),
							Iag(CABEDMACRO, hszDesc));
					if (FNeSt(st, stNewName))
						{
						RenameMacro(st, stNewName, 
								docDot, stDesc);
						}
					}
				}
			break;
			}
		}
	vbcmFetch = bsyNil;	/* Just in case we changed something */
	return cmdOK;
}


/* G E T  N A M E  B C M  D O C  D O T  F R O M  C A B */
/* %%Function:GetNameBcmDocDotFromCab %%Owner:bradch %%Reviewed:7/31/89 */
GetNameBcmDocDotFromCab(stName, pbcm, pdocDot, hcab)
char * stName;
BCM * pbcm;
int * pdocDot;
HCABEDMACRO hcab;
{
	int docDot;

	GetCabSt(hcab, stName, cchMaxMacroName + 1, Iag(CABEDMACRO, hszName));
	*pdocDot = docDot = ((*hcab)->iContext == iContextGlobal) ? 
			docGlobalDot : DocDotMother(selCur.doc);
	Assert(docDot != docNil);
	*pbcm = BsyOfStDocDot(stName, docDot);
}



csconst int rgmctEdMacroList [] = 
{
	/* Global,	Template */
	mctGlobalMacro,	mctMacro,	/* ShowAll == fFalse */
	mctGlobal,	mctAll		/* ShowAll == fTrue */
};


/* M C T  E D  M A C R O  L I S T */
/* %%Function:MctEdMacroList %%Owner:bradch %%Reviewed:7/31/89 */
MctEdMacroList(pcmb)
CMB * pcmb;
{
	return rgmctEdMacroList[(vpref.fShowAllMacros ? 2 : 0) + 
		(*((int *) pcmb->pv) != mctGlobalMacro)];
}


/* F  D L G  E D  M A C R O */
/* %%Function:FDlgEdMacro %%Owner:bradch %%Reviewed:7/31/89 */
BOOL FDlgEdMacro(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CMB * pcmb;
	CABEDMACRO ** hcab;
	int mct;
	BOOL fEnable;
	int vmn;
	int docDot;
	char st [ichMaxBufDlg];

	pcmb = PcmbDlgCur();

	mct = *((int *) pcmb->pv);

	switch (dlm)
		{
	case dlmDblClk:
		if (tmc == tmcEMContext)
			return fFalse;
		break;

	case dlmInit:
		if (FDisableTemplate(selCur.doc))
			EnableTmc(tmcEMDocType, fFalse);
		GrayButtonOnBlank(tmcEdMacroList, tmcSetDesc);
		EnableTmc(tmcDesc, fEnable = FEnabledTmc(tmcSetDesc));
		EnableTmc(tmcDescHdr, fEnable);
		goto LRefillInit;

	case dlmTerm:
		if (tmc == tmcOK)
			{
			st[0] = CchGetTmcText(tmcEdMacroList, st + 1, ichMaxBufDlg - 1);
			if (FIsRecorderSt(st))
				{
				ErrorEid(eidCantEdRecording, "FDlgEdMacro");
				SetFocusTmc(tmcEdMacroList);
				return fFalse;
				}
			}
		break;

	case dlmChange:
		switch (tmc)
			{
			BOOL fBuiltIn;

		case tmcDesc:
			if (FEnabledTmc(tmcSetDesc))
				SetDefaultTmc(tmcSetDesc);
			break;

		case tmcEdMacroList & ~ftmcGrouped:
LChangeMacro:
			st[0] = CchGetTmcText(tmcEdMacroList, st + 1, sizeof (st) - 1);
			vmn = VmnValidMacroNameSt(st);
			EnableTmc(tmcOK, fEnable = (vmn != vmnBad));
			SetDefaultTmc(fEnable ? tmcOK : tmcCancel);

			docDot = ValGetTmc(tmcEMContext) ? 
				DocDotMother(selCur.doc) : docGlobalDot;
			fBuiltIn = fFalse;
			if (fEnable = (vmn == vmnExists))
				{
				/* Do not enable delete or rename for 
					built-in commands! */
				FetchSy(BsyOfStDocDot(st, docDot));
				if (vpsyFetch->docDot != docDot /* found in same context? */
					 || vpsyFetch->mct == mctNil)
					{
					fEnable = fFalse;
					}
				else if (vpsyFetch->mct <= mctSdm)
					{
					fBuiltIn = fTrue;
					fEnable = fFalse;
					}
				}

			/* Can't rename/delete open macros! */
			if (ImeiFromSt(st, docDot) != iNil)
				fEnable = fFalse;

			EnableTmc(tmcDelete, fEnable);

			if (!fElActive)
				EnableTmc(tmcRename, fEnable);

			fEnable = !fBuiltIn && vmn != vmnBad;
			EnableTmc(tmcSetDesc, fEnable);
			EnableTmc(tmcDesc, fEnable);
			EnableTmc(tmcDescHdr, fEnable);

			goto LSetMenuHelp;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcEMShowAll:
			if (wNew != wOld)
				{
				vpref.fShowAllMacros = wNew;
				goto LRefillList2;
				}
			break;

		case tmcSetDesc:
			SetDefaultTmc(tmcOK);
			ChangeCancelToClose();
			if (CmdDlgAction(CmdEditMacro, tmc) == cmdError)
				{
				SetFocusTmc(tmcDesc);
				return fFalse;
				}
			goto LRefillList2;

		case tmcEMGlobal:
		case tmcEMDocType:
			mct = tmc == tmcEMGlobal ? mctGlobalMacro : mctMacro;
			*((int *) pcmb->pv) = mct;
			RedisplayTmc(tmcEdMacroList);
			SetFocusTmc(tmcEdMacroList);
			goto LChangeMacro;

		case tmcRename:
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				/* sdm filter will take down dialog */
				return fFalse;

			if (TmcRenameMacro(hcab) != tmcOK)
				return fFalse;
			/* FALL THROUGH */

		case tmcDelete:
			ChangeCancelToClose();
			SetFocusTmc(tmcEdMacroList);
			if (CmdDlgAction(CmdEditMacro, tmc) == cmdError)
				return fFalse;

			if (tmc == tmcRename)
				{
				/* NOTE: st is used as an sz here */
				GetCabSz(pcmb->hcab, st, sizeof (st),
						Iag(CABEDMACRO, hszNewName));
				SetTmcText(tmcEdMacroList, st);
				SetTmcTxs(tmcEdMacroList, TxsAll());
				}
			else
				{
				EnableTmc(tmcDesc, fFalse);
				EnableTmc(tmcDescHdr, fFalse);
				EnableTmc(tmcSetDesc, fFalse);
LRefillInit:
				EnableTmc(tmcOK, fFalse);
				EnableTmc(tmcDelete, fFalse);
				EnableTmc(tmcRename, fFalse);
				if (dlm != dlmInit)
					SetTmcText(tmcEdMacroList, szEmpty);
				}

			if (dlm != dlmInit)
				{
LRefillList2:
				*((int *) pcmb->pv) = mct;
				RedisplayTmc(tmcEdMacroList);
				}

			SetFocusTmc(tmcEdMacroList);

LSetMenuHelp:
			vbcmFetch = bsyNil;	/* Just in case we changed something */
			/* NOTE: st is used as an sz here */
			GetMenuHelpSz(BcmCurSelTmc(tmcEdMacroList), st);
			SetTmcText(tmcDesc, st);

			if (vfRecording && tmc == tmcRename)
				{
				FRecordCab(pcmb->hcab, IDDEdMacro, 
						tmcRename, fFalse);
				return fFalse; /* disable auto-record */
				}

			break;
			}
		}

	return fTrue;
}


/* T M C  R E N A M E  M A C R O */
/* %%Function:TmcRenameMacro %%Owner:bradch %%Reviewed:7/31/89 */
TmcRenameMacro(hcabedmacro)
HCABEDMACRO hcabedmacro;
{
	CMB cmb;
	TMC tmc;
	CERN cern;
	char dlt [sizeof (dltRenMacro)];
	char stName [cchMaxMacroName + 1];
	char stNewName [cchMaxMacroName + 1];

	cern.szPrompt = NULL; /* use default prompt */
	cern.stOrig = stName;
	cern.fNumber = fFalse;
	cern.docDot = ((*hcabedmacro)->iContext == iContextGlobal) ?
			docGlobalDot : DocDotMother(selCur.doc);

	if ((cmb.hcab = HcabAlloc(cabiCABRENMACRO)) == hNil)
		return tmcError;

	cmb.pv = &cern;
	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;

	GetCabSt(hcabedmacro, stName, sizeof (stName), Iag(CABEDMACRO, hszName));
	if (!FSetCabSt(cmb.hcab, stName, Iag(CABRENMACRO, hszName)))
		{
		tmc = tmcError;
		goto LRet;
		}

	BltDlt(dltRenMacro, dlt);

	if ((tmc = TmcOurDoDlg(dlt, &cmb)) == tmcOK)
		{
		GetCabStz(cmb.hcab, stNewName, sizeof (stNewName),
				Iag(CABRENMACRO, hszName));
		SetTmcText(tmcNewName, stNewName + 1);
		}

LRet:
	FreeCab(cmb.hcab);

	return tmc;
}


/* F  D E L E T E  M A C R O */
/* %%Function:FDeleteMacro %%Owner:bradch %%Reviewed:7/31/89 */
FDeleteMacro(stName, docDot)
char * stName;
int docDot;
{
	int elg, elm;
	BCM bcm;

	if (ImeiFromSt(stName, docDot) != iNil)
		{
		ErrorEid(eidCantRenDelOpenMacro, "FDeleteMacro");
		return fFalse;
		}
		
	FetchSy(BsyOfStDocDot(stName, docDot));
	if (vpsyFetch->docDot != docDot /* found in same context? */
		 || vpsyFetch->mct == mctNil) 
		{
		ErrorEid(eidNoSuchMacro, "FDeleteMacro");
		return fFalse;
		}

	elg = ElgFromDoc(docDot);
	elm = ImcrFromDocDotBcm(docDot, BsyFromElgSt(elg, stName));
	bcm = BcmOfSt(stName);

	if (FRunningElgElm(elg, elm))
		return fFalse;

	if (!FRemoveImcr(stName, docDot, elm))
		return fFalse;

	vgrfMenuCmdsAreDirty = 0xffff;

	DirtyDoc(docDot);
	return fTrue;
}


/* F  E D I T  M A C R O */
/* %%Function:FEditMacro %%Owner:bradch %%Reviewed:7/31/89 */
FEditMacro(stName, iContext, stDesc)
char * stName;
int iContext;
char * stDesc; /* may be NULL */
{
	MEI * pmei;
	int doc, docMcr, imcr;
	int elg, bcm, docDot;
	CP cpSav;
	struct CA ca, caT;
	struct SELS sels;
	int imeiStart;
	BOOL fNew, fWinOpen;
	BOOL fRunning;
	char ** pstName;

	/* Setup for error recovery... */
	fRunning = fFalse;
	fNew = fFalse;
	fWinOpen = fFalse;
	pstName = hNil;
	doc = docNil;

	docDot = (iContext == iContextGlobal) ?
			docGlobalDot : DocDotMother(selCur.doc);
	Assert(docDot != docNil);

	if (vhmes == hNil)
		{
		/* Create a MES and one MEI... */
		MES * pmes;

		selMacro.doc = docNil;

		if ((vhmes = HAllocateCw(cwMES + cwMEI)) == hNil)
			goto LError2;

		pmes = *vhmes;
#ifdef INEFFICIENT
		pmes->cmwOpen = 0;
		pmes->fCanCont = fFalse;
		pmes->fStep = fFalse;
		pmes->fStepSUBs = fFalse;
		pmes->fAnimate = fFalse;
#else
		SetBytes(pmes, 0, sizeof (MES));
#endif
		pmes->imeiCur = iNil;
		pmes->imeiMax = 1;
		imeiStart = 0;
		}
	else
		{
		uns cmei;
		MEI * pmei;
		MES * pmes;

		AssertH(vhmes);

		/* Check to see if a window is already open for this macro. */
		pmes = *vhmes;
		pmei = &pmes->rgmei[0];
		for (cmei = pmes->imeiMax; cmei > 0; --cmei, ++pmei)
			{
			if (FEqNcSt(*pmei->pstName, stName) && 
					pmei->elg == docDot)
				{
				/* Only one window is allowed per macro!
					Bring the macro window to the top of the
					screen and activate it... */

				SetMw(pmei->mw);
				return fTrue;
				}
			}

		/* Just add another MEI... */

		imeiStart = (*vhmes)->imeiMax;
		if (!FChngSizeHCw(vhmes, cwMES + (imeiStart + 1) * cwMEI, fFalse))
			{
			goto LError;
			}
		(*vhmes)->imeiMax += 1;
		}


		{
		BOOL fDirtySav;

		fDirtySav = PdodDoc(docDot)->fDirty;
		if ((bcm = BsyEnsureMacroExists(stName, docDot, &fNew)) == bsyNil)
			goto LError;

		/* Preserve dirtiness of document (we do not want to dirty the doc
			if the user does not save their changes...) */
		if (fNew && !fDirtySav)
			PdodDoc(docDot)->fDirty = fFalse;
		}


	/* Set the description if one was supplied... */
	if (stDesc != NULL)
		{
		if (!FSetMenuHelp(bcm, stDesc))
			{
			ErrorEid(eidNoMemory, "FEditMacro");
			goto LError;
			}
		}


	docMcr = PdodDoc(docDot)->docMcr;
	Assert(docMcr != docNil); /* must have been set by BsyEnsure..() */
	imcr = ImcrFromDocDotBcm(docDot, bcm);
	
	if (fRunning = FRunningElgElm(docDot, imcr))
		{
		goto LError;
		}


	/* Create an editing doc for the macro... */
	if ((pstName = HstCreate(stName)) == hNil)
		goto LError;

	if ((doc = DocCloneDoc(docNew, dkDoc)) == docNil)
		goto LError;

	PdodDoc(doc)->udt = udtMacroEdit;
	PdodDoc(doc)->docDot = docDot;

	/*  keep the template from being closed on us */
	PdodDoc(docDot)->crefLock++;

	/* Setup instance information... */
	pmei = &(*vhmes)->rgmei[(*vhmes)->imeiMax - 1];

#ifdef INEFFICIENT
	pmei->hqrgbTokens = hqNil;
	pmei->fRunning = fFalse;
	pmei->fHlt = fFalse;
	pmei->fDirty = fFalse;
	pmei->imelMax = 0;
	pmei->imelMac = 0;
	pmei->hrgmel = hNil;
	pmei->heli = hNil;
	pmei->fNotExpanded = fFalse;
#else
	Assert(hqNil == 0 && hNil == 0);
	SetBytes(pmei, 0, sizeof (MEI));
#endif

	pmei->bcm = bcm;
	pmei->elg = docDot;
	pmei->elm = imcr;
	pmei->pstName = pstName;
	pmei->docEdit = doc;
	pmei->fNew = fNew;
	pmei->fRestartable = fTrue;
	pmei->cpHlt = cpNil;

	/* Create a window for the editing doc... */
	sels.ca.doc = docNil;
	if (!(fWinOpen = FCreateMw(doc, wkMacro, &sels, NULL)))
		{
		ErrorEid(eidSessionComplex, " FEditMacro");
		goto LError;
		}

	/* Put up the icon bar if this is the first instance... */
	/* Also initialize the interpreter */
	if ((*vhmes)->cmwOpen == 0)
		{
		extern KMP ** vhkmpUser;
		KMP ** hkmp;

		if (!FAllocMacroSbs())
			goto LError;

		if ((hkmp = HkmpCreate(6, kmfPlain)) == hNil)
			goto LError;
		(*hkmp)->hkmpNext = (*vhkmpUser)->hkmpNext;
		(*vhkmpUser)->hkmpNext = hkmp;
		AddKeyToKmp(hkmp, kcTraceMacro, bcmTraceMacro);
		AddKeyToKmp(hkmp, kcAnimateMacro, bcmAnimateMacro);
		AddKeyToKmp(hkmp, kcContinueMacro, bcmContinueMacro);
		AddKeyToKmp(hkmp, kcContinueMacro2, bcmContinueMacro);
		AddKeyToKmp(hkmp, kcStepMacro, bcmStepMacro);
		AddKeyToKmp(hkmp, kcShowVars, bcmShowVars);
		(*vhmes)->hkmp = hkmp;

		if (!FCreateMcrIconBar())
			goto LError;
		}

		/* BLOCK: update window information */
		{
		struct MWD * pmwd;

		pmwd = *hmwdCur;
		pmwd->docMcr = docMcr;
		(*vhmes)->cmwOpen += 1;
		}

	pmei = &(*vhmes)->rgmei[(*vhmes)->imeiMax - 1];
	pmei->mw = mwCur;

	/* Copy the macro from the doc template to the editing doc... */
	CaFromIhdd(docMcr, imcr, &ca);

	if (DcpCa(&ca) == cp0)
		{
		extern char rgchEop [];
		/* New macro, auto-insert "SUB MAIN / END SUB" */
		int mct;
		CP cp;
		struct CHP chp;
		struct PAP pap;
		char stBuf [40];

		StandardChp(&chp);
		StandardPap(&pap);

		CopyCsSt(stSubMain, stBuf);
		if (!FInsertRgch(doc, cp = cp0, stBuf + 1, stBuf[0], &chp, NULL))
			goto LError;
		cp += stBuf[0];

		if (!FInsertRgch(doc, cp, rgchEop, (int) ccpEop, &chp, &pap))
			goto LError;
		cp += (int) ccpEop;

		mct = mctNil;
		bcm = BcmOfSt(stName);

		if (bcm != bcmNil)
			{
			FetchCm2(bcm, cmmBuiltIn);
			mct = vpsyFetch->mct;
			}

		/* The following commands do the same things for cmmDialog and 
			cmmAction, so build prototype like they are non-dialog commands */
		if (bcm == bcmAbout || bcm == imiThesaurus || bcm == imiHyphenate)
			mct = mctCmd;
		
		switch (mct)
			{
		case mctSdm:
			if (bcm == bcmCatalog || bcm == bcmStyles)
				{
				CopyCsSt(stMacroRun, stBuf);
				if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
					goto LError;
				cp += stBuf[0];
				if (!FInsertRgch(doc, cp, stName + 1, stName[0], &chp, NULL))
					goto LError;
				cp += stName[0];
				stBuf[0] = 1;
				stBuf[1] = '"';
				if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
					goto LError;
				cp += stBuf[0];
				break;
				}

			CopyCsSt(stDimAs, stBuf);
			if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
				goto LError;
			cp += stBuf[0];
			if (!FInsertRgch(doc, cp, stName + 1, stName[0], &chp, NULL))
				goto LError;
			cp += stName[0];
			if (!FInsertRgch(doc, cp, rgchEop, (int) ccpEop, &chp, &pap))
				goto LError;
			cp += (int) ccpEop;

			CopyCsSt(stGetCurValsDlg, stBuf);
			if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
				goto LError;
			cp += stBuf[0];
			if (!FInsertRgch(doc, cp, rgchEop, (int) ccpEop, &chp, &pap))
				goto LError;
			cp += (int) ccpEop;

			CopyCsSt(stDialogDlg, stBuf);
			if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
				goto LError;
			cp += stBuf[0];
			if (!FInsertRgch(doc, cp, rgchEop, (int) ccpEop, &chp, &pap))
				goto LError;
			cp += (int) ccpEop;

			/* FALL THROUGH */

		case mctCmd:
			CopyCsSt(stSuper, stBuf);
			if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
				goto LError;
			cp += stBuf[0];

			if (!FInsertRgch(doc, cp, stName + 1, stName[0], &chp, NULL))
				goto LError;
			cp += stName[0];

			if (mct == mctSdm)
				{
				CopyCsSt(stDlg, stBuf);
				FInsertRgch(doc, cp, stBuf + 1, 
						stBuf[0], &chp, NULL);
				cp += stBuf[0];
				}
			break;
			}

		cpSav = cp;
		if (!FInsertRgch(doc, cp, rgchEop, (int) ccpEop, &chp, &pap))
			goto LError;
		cp += (int) ccpEop;

		CopySt(stEndSub, stBuf);
		if (!FInsertRgch(doc, cp, stBuf + 1, stBuf[0], &chp, NULL))
			goto LError;
		}
	else
		{
		HQ hqrgbTokens;
		uns cbTokens;

		cbTokens = DcpCa(&ca);
		if ((hqrgbTokens = HqAllocLcb((long) cbTokens)) == hqNil)
			goto LError;

		FetchPcaToHqrgb(&ca, hqrgbTokens);
		if (!FExpandHqrgbToPca(hqrgbTokens, cbTokens, 
				PcaPoint(&caT, doc, cp0),(*vhmes)->imeiMax - 1, fFalse))
			{
			FreeHq(hqrgbTokens);
			goto LError;
			}

		pmei = &(*vhmes)->rgmei[(*vhmes)->imeiMax - 1];
		pmei->cbTokens = cbTokens;
		pmei->hqrgbTokens = hqrgbTokens;
		cpSav = cp0;
		}


		{
		int imei;

	/* If a macro is suspended, stop it completely to change Continue
		button to start, and set things up to start new macro. */
		if ((imei = (*vhmes)->imeiCur) != iNil && !PmeiImei(imei)->fRunning)
			FreeEditMacro(imei);
		}

	GrayIibbHwndIb(vhwndAppIconBar, IDMCRVARS, fTrue);
	SelectEdMacro(mwCur);

	if (fNew)
		{
		/* Put insert point between SUB MAIN and END SUB */
		Select(&selCur, cpSav, cpSav);
		}

	return fTrue;


LError:
	if (fWinOpen)
		{
		CloseMwPanes(mwCur);
		--(*vhmes)->cmwOpen;
		}

	if ((*vhmes)->cmwOpen == 0)
		{
		if (vhwndAppIconBar != NULL)
			DestroyIconBar(hNil);
		if ((*vhmes)->hkmp != hNil)
			RemoveKmp((*vhmes)->hkmp);
		}

	if (!fWinOpen && doc != docNil)
		{
		int docDot = PdodDoc(doc)->docDot;

		if (docDot != docNil)
			{
			PdodDoc(docDot)->crefLock--;
			if (PdodDoc(docDot)->crefLock == 0)
				DisposeDoc(docDot);
			PdodDoc(doc)->docDot = docNil;
			}
		DisposeDoc(doc);
		}

	if (vhmes != hNil)
		{
		if (((*vhmes)->imeiMax = imeiStart) == 0)
			FreePh(&vhmes);
		}

	if (pstName != hNil)
		FreeH(pstName);

	if (fNew)
		FDeleteMacro(stName, docDot);

LError2:
	if (fRunning)
		ModeError();
	else
		ErrorEid(eidNoMemory, "FEditMacro");

	return fFalse;
}


/* %%Function:ImeiFromMw %%Owner:bradch %%Reviewed:7/31/89 */
int ImeiFromMw(mw)
int mw;
{
	int imei, imeiMac;
	MEI * pmei;

	imeiMac = (*vhmes)->imeiMax;
	pmei = (*vhmes)->rgmei;
	for (imei = 0; imei < imeiMac; imei += 1, pmei += 1)
		if (pmei->mw == mw)
			return imei;

	return iNil;
}


/* F  C L O S E  E D  M A C R O */
/* Called whan the window is being closed. Returns fFalse iff user cancels
from SaveChanges message box. */
/* %%Function:FCloseEdMacro %%Owner:bradch %%Reviewed:7/31/89 */
FCloseEdMacro(imei, ac)
int imei;
int ac;
{
	MEI * pmei;
	int docEdit;

	Assert(vhmes != hNil);
	Assert(imei >= 0 && imei < (*vhmes)->imeiMax);

	if (PmeiImei(imei)->fRunning)
		{
		ErrorEid(eidCantCloseRunningMacro, "FCloseEdMacro");
		return fFalse;
		}

	FreeEditMacro(imei);

	pmei = PmeiImei(imei);

	docEdit = pmei->docEdit;

	if (pmei->fDirty || PdodDoc(docEdit)->fDirty)
		{
		int docDot = PdodDoc(docEdit)->docDot;
		int id;

	/* if CBT is exiting, don't bother asking - we never save
		anything done in CBT */

		if (!vhwndCBT && vrf.fRanCBT)
			{
			if (!FRemoveImei(imei, fFalse /*fCheckSave*/))
				return fFalse;
			else
				goto LCleanUp;
			}

		switch (ac)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
#endif
		case acSaveAll:
		case acQuerySave:
			id = IdQuerySaveChanges(pmei);
			break;

		case acSaveNoQuery:
			id = IDYES;
			break;

		case acNoSave:
			id = IDNO;
			break;
			}

		switch (id)
			{
		case IDNO:
			if (!FRemoveImei(imei, fTrue /*fCheckSave*/))
				return fFalse;
			break; /* to LCleanUp */

		case IDYES:
			if (!FSaveEdMacro(imei, fTrue /* fClosing */))
				{
				ErrorEid(eidNoMemory, "FCloseEdMacro");
				return fFalse;
				}

			/* Save the template too, if the macro is the 
				only thing referencing it... */
			if (PdodDoc(docDot)->crefLock == 1 
					&& WwDisp(docDot, wwNil, fFalse) == wwNil
					&& !FConfirmSaveDoc(docDot, fFalse, acQuerySave))
				{
				return fFalse;
				}
			break;

		case IDCANCEL:
			return fFalse;
			}
		}
	else  if (pmei->fNew /* new macro that was not edited! */
			&& !FRemoveImei(imei, fTrue /*fCheckSave*/))
		return fFalse;

LCleanUp:
	/* BLOCK: free resources used by mei */
		{
		if (PmeiImei(imei)->hrgmel != hNil)
			FreeH(PmeiImei(imei)->hrgmel);
		FreeH(PmeiImei(imei)->pstName);
		if (PmeiImei(imei)->hqrgbTokens != hqNil)
			FreeHq(PmeiImei(imei)->hqrgbTokens);
		}

	/* BLOCK: Remove from (*vhmes)->rgmei[]... */
		{
		MES * pmes;
		MEI * pmeiCur;
		int mw, docEdit, imeiMax;

		pmes = *vhmes;
		pmeiCur = &pmes->rgmei[imei];
		pmes->imeiMax -= 1;
		imeiMax = pmes->imeiMax;
		mw = pmeiCur->mw;
		docEdit = pmeiCur->docEdit;

		bltb(pmeiCur + 1, pmeiCur, sizeof (MEI) * (imeiMax - imei));

		if (imeiMax > 0)
			{
			/* block is shrinking, so this cannot fail */
			FChngSizeHCw(vhmes, cwMES + imeiMax * cwMEI, fTrue);
			}
		/* else it's going to be freed below... */

		/* Make sure a new macro gets selected if there are any... */
		if (imei == (*vhmes)->imeiCur)
			{
			if (imei == imeiMax)
				imei -= 1;

			if (imei >= 0)
				{
				(*vhmes)->imeiCur = iNil;
				SelectEdMacro((*vhmes)->rgmei[imei].mw);
				}
			else
				{
				Assert(vhwndAppIconBar != NULL);
				SetIibbTextHwndIb(vhwndAppIconBar, 
						IDMCRNAME, szEmpty);
				}
			}
		else  if (imei < (*vhmes)->imeiCur)
			{
			(*vhmes)->imeiCur -= 1;
			}

			{
			int docDot = PdodDoc(docEdit)->docDot;

			if (docDot != docNil)
				{
				PdodDoc(docDot)->crefLock--;
				if (PdodDoc(docDot)->crefLock == 0)
					DisposeDoc(docDot);
				PdodDoc(docEdit)->docDot = docNil;
				}
			}
		if (mpmwhmwd[mw] != hNil)
			CloseMwPanes(mw);

		if (--(*vhmes)->cmwOpen == 0)
			{
			RemoveKmp((*vhmes)->hkmp);
			DestroyIconBar(hNil);
			FreePh(&vhmes);
			FreeMacroSbs();
			}
		}

	return fTrue;
}


/* I D  Q U E R Y  S A V E  C H A N G E S */
/* %%Function:IdQuerySaveChanges %%Owner:bradch %%Reviewed:7/31/89 */
int IdQuerySaveChanges(pmei)
MEI *pmei;
{
	unsigned rgw[2];
	char szName[cchMaxSyName];

	AssertDo(FGetStMacro(pmei->docEdit, szName));

	StToSzInPlace(szName);
	rgw[0] = szName;
	return IdMessageBoxMstRgwMb(mstSaveChangesMcr, rgw, mbQuerySave);
}


/* S E L E C T  E D  M A C R O */
/* %%Function:SelectEdMacro %%Owner:bradch %%Reviewed:7/31/89 */
SelectEdMacro(mw)
int mw;
{
	struct MWD ** hmwd;
	int imei;
	MEI *pmei;

	hmwd = mpmwhmwd[mw];
	AssertH(hmwd);
	/* will happen when first macro editor window is opened */
	if (vhwndAppIconBar == NULL || (*hmwd)->docMcr == docNil)
		return;

	imei = ImeiFromMw(mw);
	pmei = PmeiImei(imei);
	if (pmei->fHlt && (*vhmes)->fCanCont)
		{
		/* should be true when macro was stepped from a diff window */
		pmei->fHlt = fFalse;
		TurnOffSel(&selCur); /* hack to keep hilighting on */
		blt(&selCur, &selMacro, cwSEL);
		}

	SetImeiCur(imei);
}


/* S E T  I M E I  C U R */
/* %%Function:SetImeiCur %%Owner:bradch %%Reviewed:7/31/89 */
SetImeiCur(imei)
int imei;
{
	char szTitle [ichMaxBufDlg];

	Assert(imei != -1);

	if (imei == (*vhmes)->imeiCur)
		return;

	GetWindowText(PmwdMw(PmeiImei(imei)->mw)->hwnd, 
			(LPSTR) szTitle, sizeof (szTitle));
	Assert(vhwndAppIconBar != NULL);
	SetIibbTextHwndIb(vhwndAppIconBar, IDMCRNAME, szTitle);

	if ((*vhmes)->imeiCur != iNil && !PmeiCur()->fRunning)
		FreeEditMacro((*vhmes)->imeiCur);

	(*vhmes)->imeiCur = imei;
}


/* R E N A M E  M A C R O */
/* %%Function:RenameMacro %%Owner:bradch %%Reviewed:7/31/89 */
RenameMacro(stOldName, stNewName, docDot, stDesc)
char * stOldName;
char * stNewName;
int docDot;
char * stDesc;
{
	int elg, imcr;
	uns bsy;

	if (!FNeSt(stOldName, stNewName))
		return;

	if (!FNeNcSt(stOldName, stNewName))
		{
		/* Just change the letter case! */
		extern HPSYT vhpsyt;
		HPSY hpsy;
		BCM bcm;

		/* Can't change built-in names! */
		if ((bcm = BcmOfSt(stOldName)) < bcmMacStd)
			return;

		hpsy = (HPSY) (vhpsyt->grpsy + bcm);
		Assert(hpsy->stName[0] == stNewName[0]);
		bltbh((char huge *) stNewName + 1, hpsy->stName + 1, 
				stNewName[0]);
		DirtyDoc(docDot);
		return;
		}

	elg = ElgFromDoc(docDot);

	/* make a copy with the new name */
	imcr = ImcrFromDocDotBcm(docDot, BsyOfStDocDot(stOldName, docDot));
	if ((bsy = BsyAddCmd1(stNewName, docDot, imcr)) == bsyNil)
		return;
	DirtyDoc(docDot);

	if (stDesc != NULL)
		FSetMenuHelp(bsy, stDesc);

	/* remove the old one */
	RemoveCmd(stOldName, elg);
}


/* F  E N U M  R E M O V E  M C R */
/* %%Function:FEnumRemoveMcr %%Owner:bradch %%Reviewed:7/31/89 */
BOOL FEnumRemoveMcr(lpsy, bsy, stName, wParam)
SY FAR *lpsy;
uns bsy;
char * stName;
{
	if (((lpsy->mct == mctMacro && 
			lpsy->docDot == DocDotMother(selCur.doc)) || 
			lpsy->mct == mctGlobalMacro) && lpsy->imcr > wParam)
		{
		lpsy->imcr -= 1;
		}

	return fTrue;
}


/* F  R E M O V E  I M C R */
/* %%Function:FRemoveImcr %%Owner:bradch %%Reviewed:7/31/89 */
FRemoveImcr(stName, docDot, imcr)
char * stName;
int docDot, imcr;
{
	MEI * pmei, * pmeiMax;
	int docMcr;
	struct CA ca;
	char stNameSave [cchMaxMacroName + 1];

	docMcr = PdodDoc(docDot)->docMcr;
	Assert(docMcr != docNil);

	/* the following bltb is necessary as the FDelete operation
		can trash the function parameters of pointer type */

	Assert(stName[0] + 1 <= sizeof (stNameSave));
	bltb(stName, stNameSave, stName[0] + 1);
	CaFromIhdd(docMcr, imcr, &ca);
	ca.cpLim += ccpEop;
	if (!FDelete(&ca))
		return fFalse;

	RemoveCmd(stNameSave, ElgFromDoc(docDot));

	AssertH(PdodDoc(docMcr)->hplcmcr);
	AssertDo(FOpenPlc(PdodDoc(docMcr)->hplcmcr, imcr, -1));

	/* scan the command table for entries with imcr's > imcr and adjust */
	FEnumMacros(FEnumRemoveMcr, imcr, 
		docDot == docGlobalDot ? mctGlobalMacro : mctMacro);

	if (vhmes != hNil)
		{
		/* Adjust imcr's in MEIs too. */
		pmei = (*vhmes)->rgmei;
		pmeiMax = pmei + (*vhmes)->imeiMax;
		for ( ; pmei < pmeiMax; pmei += 1)
			{
			if (pmei->elg == docDot && pmei->elm > imcr)
				pmei->elm -= 1;
			}
		}

	return fTrue;
}


/* F  D L G  R E N  M A C R O */
/* NOTE: This dialog is also used by Set Variable and may be used by others
as well.  PcmbDlgCur()->pv should be set up to point to the prompt text, or
NULL if this is just Rename Macro.  If a prompt is given, OK is always
enabled. */
/* %%Function:FDlgRenMacro %%Owner:bradch %%Reviewed:7/31/89 */
BOOL FDlgRenMacro(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CERN * pcern;
	char stz [ichMaxBufDlg + 1];

	pcern = PcmbDlgCur()->pv;

	switch (dlm)
		{
	case dlmInit:
		if (pcern->szPrompt != NULL)
			SetTmcText(tmcRenPrompt, pcern->szPrompt);
		break;

	case dlmChange:
		stz[0] = CchGetTmcText(tmcName, stz + 1, sizeof (stz) - 1);
		if (tmc == tmcName && pcern->szPrompt == NULL)
			{
			int vmn;

			if ((vmn = VmnValidMacroNameSt(stz)) == vmnExists)
				{
				FetchSy(BsyOfStDocDot(stz, pcern->docDot));
				if (!FNeNcSt(stz, pcern->stOrig) || vpsyFetch->mct == mctNil
					 || vpsyFetch->docDot != pcern->docDot)
					vmn = vmnValid;
				}
		 
			EnableTmc(tmcOK, vmn == vmnValid);
			}
		
		if (pcern->fNumber)
			{
			NUM num;
			EnableTmc(tmcOK, CchPackSzPnum(stz + 1, &num) > 0);
			}
		/* break; IE */
		}

	return fTrue;
}


/* C M D  T R A C E  M A C R O */
/* %%Function:CmdTraceMacro %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdTraceMacro(pcmb)
CMB * pcmb;
{
	CMD cmd;

	if (fElActive)
		ModeError();

	(*vhmes)->fAnimate = fTrue;
	(*vhmes)->fStep = fTrue;
	cmd = CmdCallInterp((*vhmes)->imeiCur);
	(*vhmes)->fStep = fFalse;
	(*vhmes)->fAnimate = fFalse;

	return cmd;
}


/* C M D  S T E P  M A C R O */
/* Execute the next statement of the currently running macro */
/* %%Function:CmdStepMacro %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdStepMacro(pcmb)
CMB * pcmb;
{
	CMD cmd;

	if (fElActive)
		ModeError();

	(*vhmes)->fAnimate = fTrue;
	
	/* Make StepSUBs act like step if we are not already stepping...  This
		makes StepSUBs step into Sub MAIN instead of over it! */
	if ((*vhmes)->fCanCont)
		(*vhmes)->fStepSUBs = fTrue;
	else
		(*vhmes)->fStep = fTrue;
	
	vcSubNest = 0;
	cmd = CmdCallInterp((*vhmes)->imeiCur);
	
	(*vhmes)->fStepSUBs = fFalse;
	(*vhmes)->fStep = fFalse;
	(*vhmes)->fAnimate = fFalse;

	return cmd;
}


/* C M D  C O N T I N U E  M A C R O */
/* Start macro execution */
/* %%Function:CmdContinueMacro %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdContinueMacro(pcmb)
CMB * pcmb;
{
	CMD cmd;

	if (fElActive)
		ModeError();

	cmd = CmdCallInterp((*vhmes)->imeiCur);

	return cmd;
}


/* %%Function:CmdCallInterp %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdCallInterp(imei)
int imei;
{
	CMD cmd;
	MEI * pmei;
	BOOL fRecording;

#ifdef DEBUG
	extern int vfInCmd;
	int fInCmdSave;
#endif /* DEBUG */

	if (vhmes == hNil || imei == iNil)
		{
		Beep();
		return cmdError;
		}

	if (PmeiImei(imei)->fRunning)
		RtError(rerrModeError);

	StartLongOp();

#ifdef DEBUG
	fInCmdSave = vfInCmd;
	if (!vdbs.fFailInMcr)
		vfInCmd = 0;
#endif /* DEBUG */

	if (selCur.doc == PmeiImei(imei)->docEdit)
		TurnOffSel(&selCur);

	if (selMacro.doc == docNil)
		{
		*((struct SELS *) &selMacro) = 
				PwwdWw(PmwdMw(PmeiImei(imei)->mw)->wwActive)->sels;
		selMacro.ww = PmwdMw(PmeiImei(imei)->mw)->wwActive;
		selMacro.fHidden = fTrue;
		selMacro.fOn = fFalse;
		}

	pmei = PmeiImei(imei);
	if (pmei->cpHlt)
		{
		struct CA caT;
		char rgb [2];

		pmei->cpHlt = cpNil;

		rgb[0] = sprmCIco;
		rgb[1] = icoAuto;
		ApplyGrpprlCa(rgb, 2, 
				PcaSetWholeDoc(&caT, pmei->docEdit));
		InvalCp(&caT);
		}

	pmei = PmeiImei(imei);
	/* Lock so macro doesn't trash itself */
	PdodDoc(pmei->docEdit)->fLockForEdit = fTrue;

	cmd = ((pmei->heli == hNil && !FTokenizeImei(imei)) || 
			!FRunImei(imei)) ? cmdError : cmdOK;

	PdodDoc(PmeiImei(imei)->docEdit)->fLockForEdit = fFalse;

	Debug(vfInCmd = fInCmdSave);

	EndLongOp(fFalse);

	return cmd;
}


/* F  T O K E N I Z E  I M E I */
/* %%Function:FTokenizeImei %%Owner:bradch %%Reviewed:7/31/89 */
FTokenizeImei(imei)
int imei;
{
	HQ hqrgbTokens;
	uns cbTokens;
	struct CA ca;
	int docEdit;
	MEI * pmei;
	BOOL fDirtyTokens;

	pmei = PmeiImei(imei);
	docEdit = pmei->docEdit;

	if (fDirtyTokens = PdodDoc(docEdit)->fDirty)
		pmei->fDirty = fTrue;
	PdodDoc(docEdit)->fDirty = fFalse;

	if (fDirtyTokens || pmei->hqrgbTokens == hqNil)
		{
		StartLongOp();
		if (!FTokenizePca(PcaSetWholeDoc(&ca, docEdit),
				&hqrgbTokens, &cbTokens))
			{
			EndLongOp(fFalse);
			return fFalse;
			}

		pmei = PmeiImei(imei);
		pmei->fNotExpanded = fTrue;

		if (pmei->hqrgbTokens != hqNil)
			{
			FreezeHp();
			FreeHq(pmei->hqrgbTokens);
			MeltHp();
			}

		pmei->hqrgbTokens = hqrgbTokens;
		pmei->cbTokens = cbTokens;

		EndLongOp(fFalse);
		}

	FTouchElgElm(pmei->elg, pmei->elm);

	return fTrue;
}


/* F  R U N  I M E I */
/* %%Function:FRunImei %%Owner:bradch %%Reviewed:7/31/89 */
BOOL FRunImei(imei)
int imei;
{
	extern int ElaDebug();
	extern int GetInfoElx();
	extern ELI ** HeliNew();

	RERR rerr;
	MEI * pmei;
	ELG elg, elgCurSav;
	BCM bcm, bcmCurSav;

	Assert(vhmes != hNil);
	Assert(imei >= 0 && imei < (*vhmes)->imeiMax);

	StartLongOp();

	if (PmeiImei(imei)->heli == hNil)
		{
		pmei = &(*vhmes)->rgmei[imei];
		if ((PmeiImei(imei)->heli = HeliNew(pmei->elg, pmei->elm,
				CchReadEdit, ElaDebug, GetInfoElx, -1, (long) imei)) == hNil)
			{
			EndLongOp(fFalse);
LNoMemory:
			ErrorEid(eidNoMemory, "FRunMacro");
			return fFalse;
			}

		vcSubNest = 0;
		}

	vfElDisableInput = fFalse;
	ClearAbortKeyState();

	elgCurSav = PmeiCur()->elg;
	bcmCurSav = PmeiCur()->bcm;
	SetImeiCur(imei);

	pmei = PmeiImei(imei);
	pmei->fRunning = fTrue;
	elg = pmei->elg;
	bcm = pmei->bcm;
	
	vfMcrRunning = fTrue;
	rerr = RerrRunHeli(pmei->heli);
	vfMcrRunning = fFalse;
	
	Assert(ImeiFromElgBcm(elg, bcm) != iNil);
	pmei = PmeiImei(ImeiFromElgBcm(elg, bcm));
	pmei->fRunning = fFalse;

	if (FMacroErrMsg(pmei->heli, rerr))
		{
		(*vhmes)->fCanCont = fTrue;
		EnableContinue(fTrue);
		}
	else
		{
		int imeiT;
		
		if ((imeiT = ImeiFromElgBcm(elgCurSav, bcmCurSav)) != iNil)
			SetImeiCur(imeiT);
		}

	EndLongOp(fFalse /* fAll */ );

	return rerr == rerrNil;
}


/* F R E E  E D I T  M A C R O */
/* %%Function:FreeEditMacro %%Owner:bradch %%Reviewed:7/31/89 */
FreeEditMacro(imei)
int imei;
{
	extern int vcElParams;
	int imeiMac;
	ELI ** heli;
	MEI * pmei;

	Assert(vhmes != hNil);
	if (imei == iNil)
		{
		/* Then free all of the helis */
		imei = 0;
		imeiMac = (*vhmes)->imeiMax;
		}
	else
		{
		imeiMac = imei + 1;
		}

	for ( ; imei < imeiMac; imei += 1)
		{
		pmei = PmeiImei(imei);
		pmei->fSuspended = fFalse;
		if ((heli = pmei->heli) != hNil)
			{
			Assert(!pmei->fRunning);
			TermMacro(heli);
			PmeiImei(imei)->heli = hNil;
			}
		}

	(*vhmes)->fCanCont = fFalse;

	/* IsWindow check is for case where macro did something such that
		the icon bar no longer exists */
	if (IsWindow(vhwndAppIconBar))
		EnableContinue(fFalse);

	if (!FTdsInUse())
		CleanupEl();
}


/* E N A B L E  C O N T I N U E */
/* %%Function:EnableContinue %%Owner:bradch %%Reviewed:7/31/89 */
EnableContinue(fEnable)
BOOL fEnable;
{
	Assert(vhmes != hNil);
	Assert(vhwndAppIconBar != NULL);
	Assert(IsWindow(vhwndAppIconBar));

	if ((*vhmes)->fCanCont = fEnable)
		{
		SetIibbTextHwndIb(vhwndAppIconBar, IDMCRGO, 
				SzSharedKey("C&ontinue",Continue));
		GrayIibbHwndIb(vhwndAppIconBar, IDMCRVARS, fFalse);
		}
	else
		{
		SetIibbTextHwndIb(vhwndAppIconBar, 
				IDMCRGO, SzSharedKey("&Start",Start));
		GrayIibbHwndIb(vhwndAppIconBar, IDMCRVARS, fTrue);
		}
}


/* C M D  A N I M A T E  M A C R O */
/* %%Function:CmdAnimateMacro %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdAnimateMacro(pcmb)
CMB * pcmb;
{
	CMD cmd;

	if (fElActive)
		ModeError();

	(*vhmes)->fAnimate = fTrue;
	cmd = CmdCallInterp((*vhmes)->imeiCur);
	(*vhmes)->fAnimate = fFalse;

	return cmd;
}


/* C M D  S H O W  V A R S */
/* %%Function:CmdShowVars %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdShowVars(pcmb)
CMB * pcmb;
{
	TMC tmc;
	char dlt [sizeof (dltShowVars)];
	
	if (vhmes == hNil || (!(*vhmes)->fCanCont && !PmeiCur()->fRunning))
		{
		Beep();
		return cmdError;
		}
		
	Assert(pcmb->hcab == hNil);
	if ((pcmb->hcab = HcabAlloc(cabiCABSHOWVARS)) == hNil)
		return cmdNoMemory;

	((CABSHOWVARS *) *pcmb->hcab)->iVar = uNinchList;

	BltDlt(dltShowVars, dlt);

	tmc = TmcOurDoDlg(dlt, pcmb);
	FreeCab(pcmb->hcab);
	pcmb->hcab = hNil;
		
	switch (tmc)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		return cmdError;
#endif

	case tmcError:
		return cmdNoMemory;

	case tmcCancel:
		break;
		}

	/* All actions performed in dialog function! */

	return cmdOK;
}


/* F  D L G  S H O W  V A R S */
/* %%Function:FDlgShowVars %%Owner:bradch %%Reviewed:7/31/89 */
BOOL FDlgShowVars(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	switch (dlm)
		{
	case dlmInit:
		/* fill the list box with names/values */
		EnableTmc(tmcSet, fFalse);
		FillVarsList(tmcVarList);
		if (CEntryListBoxTmc(tmcVarList) > 0)
			{
			SetTmcVal(tmcVarList, 0);
			EnableTmc(tmcSet, fTrue);
			}
		break;

	case dlmClick:
		if (tmc == tmcSet)
			{
			int uList;
			char * pch;
			void huge * hpval;
			char szBuf [ichMaxBufDlg];

			/* put current value in szBuf */
			uList = ValGetTmc(tmcVarList);
			GetTmcText(tmcVarList, szBuf, sizeof (szBuf));
			for (pch = szBuf; *pch != 0 && *pch != '='; ++pch)
				;
			if (*pch == '=')
				CchCopySz(pch + 1, szBuf);


			if (TmcSetVarDlg(szBuf, 
					ElvFromListBoxI(uList, &hpval) == elvNum) == tmcOK)
				{
				int i, iFrame, iLocal;
				ELV elv;

				/* set variable to stBuf */
				elv = ElvFromListBoxI(uList, &hpval);
				switch (elv)
					{
					NUM num;
					SD sd;

#ifdef DEBUG
				default:
					Assert(fFalse);
					break;
#endif

				case elvSd:
					if ((sd = SdCopySz(szBuf)) != 0)
						{
						FreeSd(*((SD huge *) hpval));
						*((SD huge *) hpval) = sd;
						}
					break;

				case elvNum:
					AssertDo(CchPackSzPnum(szBuf, &num) > 0);
					*((NUM huge *) hpval) = num;
					break;
					}

				FillVarsList(tmcVarList);
				}
			SetFocusTmc(tmcCancel);
			}
		/* break; IE */
		}

	return fTrue;
}


/* T M C  S E T  V A R  D L G */
/* %%Function:TmcSetVarDlg %%Owner:bradch %%Reviewed:7/31/89 */
TmcSetVarDlg(szBuf, fNumber)
char * szBuf;
{
	CMB cmb;
	TMC tmc;
	CERN cern;
	char dlt [sizeof (dltRenMacro)];
	char stName [cchMaxMacroName + 1];
	char stNewName [cchMaxMacroName + 1];

	cern.szPrompt = SzShared("&Set Variable to:");
	cern.stOrig = NULL;
	cern.fNumber = fNumber;
	cmb.hcab = HcabAlloc(cabiCABRENMACRO);
	cmb.pv = &cern;
	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;

	if (cmb.hcab == hNil)
		return tmcError;

	if (!FSetCabSz(cmb.hcab, szBuf, Iag(CABRENMACRO, hszName)))
		{
		tmc = tmcError;
		goto LRet;
		}

	BltDlt(dltRenMacro, dlt);

	if ((tmc = TmcOurDoDlg(dlt, &cmb)) == tmcOK)
		{
		GetCabSz(cmb.hcab, szBuf, ichMaxBufDlg,
				Iag(CABRENMACRO, hszName));
		}
LRet:
	FreeCab(cmb.hcab);

	return tmc;
}


/* F I L L  V A R S  L I S T */
/* %%Function:FillVarsList %%Owner:bradch %%Reviewed:7/31/89 */
FillVarsList(tmc)
TMC tmc;
{
	extern SB sbStrings;
	ELV elv;
	int iFrame, iLocal;
	ELG elg;
	ELM elm;
	SD sd;
	void huge * hpval;
	BOOL fAddedOne;
	int cch;
	char * pch;
	NUM num;
	char sz [256];

	iFrame = iLocal = 0;
	fAddedOne = fFalse;

	StartListBoxUpdate(tmc);

	while ((elv = ElvEnumerateVar(iFrame, iLocal, sz, &elg, &elm, 
			&hpval)) != elvEndStack)
		{
		pch = &sz[sz[0]];
		StToSzInPlace(sz);
		*pch++ = '=';
		*pch = '\0';

		iLocal += 1;

		switch (elv)
			{
		case elvNum:
			num = *((NUM huge *) hpval);
			Assert(&sz[sizeof(sz)-1]-pch >= cchMaxNum*2);
			pch += CchPnumToPch(&num, pch);
			*pch = '\0';
			break;

#ifdef FUTURE
		case elvInt:
			Assert(fFalse);
			pch += CchCopySz(SzShared("INTEGERS ARE NYI!"), pch);
			break;
#endif

		case elvSd:
			sd = *((SD huge *) hpval);
			cch = min(sizeof (sz) - (pch - sz) - 2, CchFromSd(sd));
			BLTBH(HpchFromSd(sd), (char huge *) pch, cch);
			pch += cch;
			*pch = '\0';
			break;

#ifdef FUTURE
		case elvArray:
			*pch++ = '(';
			*pch++ = ')';
			*pch = '\0';
			break;
#endif

		case elvEndModuleFrame:
		case elvEndProcFrame:
			iLocal = 0;
			iFrame += 1;
			/* FALL THROUGH */

		default:
			continue;
			}

		AddListBoxEntry(tmc, sz);
		fAddedOne = fTrue;
		}

	EndListBoxUpdate(tmc);
	if (fAddedOne)
		SetTmcVal(tmc, 0);
}


/* D O C  C R E A T E  M C R */
/* %%Function:DocCreateMcr %%Owner:bradch %%Reviewed:7/31/89 */
int DocCreateMcr(docDot)
int docDot;
{
	int doc;
	struct DOD ** hdod;
	MCR mcr;
	struct PLC ** hplcmcr;

#ifdef DEBUG
		{
		struct DOD * pdod;
		pdod = PdodDoc(docDot);
		Assert(pdod->fDot);
		Assert(pdod->docMcr == docNil);
		}
#endif

	if ((doc = DocCreateSub(docDot, dkMcr)) == docNil)
		return docNil;

	hdod = mpdochdod[doc];

	Assert((*hdod)->hplcmcr == hNil);
	if ((hplcmcr = HplcCreateEdc(hdod, edcMcr)) == hNil)
		{
		DisposeDoc(doc);
		PdodDoc(docDot)->docMcr = docNil;
		return docNil;
		}
	/* HplcCreateEdc() always inserts one entry */
	mcr.bcm = bsyNil;
	PutPlc(hplcmcr, 0, &mcr);

	Assert((*hdod)->hplcmcr != hNil);
	Assert(PdodDoc(docDot)->docMcr == doc);

	return doc;
}


/* I M C R  C R E A T E */
/* Add the text of a new macro to the given macro document. */
/* %%Function:ImcrCreate %%Owner:bradch %%Reviewed:7/31/89 */
int ImcrCreate(docMcr, stName, pcaSrc)
int docMcr;
char * stName;
struct CA * pcaSrc;
{
	struct PLC ** hplcmcr;
	int imcr, imcrMac;
	MCR mcr;
	struct CA caOld;

/* Look for an empty slot in the dod's hplcmcr */
	hplcmcr = PdodDoc(docMcr)->hplcmcr;
	imcrMac = IMacPlc(hplcmcr) - 1;
	for (imcr = 0; imcr < imcrMac; ++imcr )
		{
		GetPlc( hplcmcr, imcr, &mcr );
		if (mcr.bcm == -1)
			break;
		}

/* Insert a new one if there aren't any empty slots */
	if (imcr == imcrMac && !FInsertIhdd(docMcr, imcrMac))
		return iNil;

	Assert(CpPlc( hplcmcr, imcr) + ccpEop == CpPlc( hplcmcr, imcr + 1));

/* Set the text */
	Assert(imcr < IMacPlc(hplcmcr) - 1);
	CaFromIhdd(docMcr, imcr, &caOld);
	if (!FReplaceCps(&caOld, pcaSrc))
		return iNil;

	return imcr;
}


/* I M C R  F R O M  D O C  D O T  B C M */
/* %%Function:ImcrFromDocDotBcm %%Owner:bradch %%Reviewed:7/31/89 */
int ImcrFromDocDotBcm(docDot, bcm)
int docDot;
int bcm;
{
	char rgchSy [cbMaxSy];

	return (PsyGetSyFromBcmElg(rgchSy, bcm, docDot)->imcr);
}


/* C M D  M A C R O */
/* BRADCH: should be called CmdRunMacro() */
/* %%Function:CmdMacro %%Owner:bradch %%Reviewed:7/31/89 */
CMD CmdMacro(pcmb)
CMB * pcmb;
{
	int cmd;

	if (!FInitMacroDoc())
		{
		ErrorEid(eidNoMemory, "Cmd(Run)Macro");
		return cmdError;
		}

	cmd = cmdOK;

	if (FCmdFillCab())
		{
		HCABRUNMACRO hcab;

		hcab = pcmb->hcab;
		AssertH(hcab);
		/* Up with the dialog */
		(*hcab)->fShowAll = vpref.fShowAllMacros;
		if (!FSetCabSz(hcab, szEmpty, Iag(CABRUNMACRO, hszName)))
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltRunMacro)];

		BltDlt(dltRunMacro, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif /* DEBUG */

		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}

		if (pcmb->fAction)
			{
			UpdateWindow(vhwndApp);
			UpdateAllWws(fFalse);
			}
		}

	if (FCmdAction())
		{
		BCM bcm;
		char stName[cchMaxSyName + 1];

		/* Get the macro's name */
		GetCabSt(pcmb->hcab, stName, sizeof (stName), 
				Iag(CABRUNMACRO, hszName));
		if ((bcm = BcmOfSt(stName)) == bcmNil)
			{
			ErrorEid(eidNoSuchMacro, "CmdMacro");
			return cmdError;
			}
		
		/* Avoid recursion with repeat and don't allow macro icon buttons
			when a macro is already running! */
		if ((pcmb->fRepeated && bcm == bcmRepeat) ||
				bcm == bcmTraceMacro || bcm == bcmStepMacro || 
				bcm == bcmContinueMacro || bcm == bcmAnimateMacro)
			{
			ModeError();
			return cmdError;
			}

		cmd = CmdRunBcmEl(bcm);
		}

LReturn:
	return cmd;
}


/* F  D L G  R U N  M A C R O */
/* %%Function:FDlgRunMacro %%Owner:bradch %%Reviewed:7/31/89 */
BOOL FDlgRunMacro(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	char st [ichMaxBufDlg];
	char sz [ichMaxBufDlg];

	switch (dlm)
		{
	case dlmInit:
		EnableTmc(tmcOK, fFalse);
		break;

	case dlmChange:
		if (tmc == (tmcRMCombo & ~ftmcGrouped))
			{
			BOOL fEnable;
			int docDot;
			
			st[0] = CchGetTmcText(tmcRMCombo, st + 1, sizeof (st) - 1);
			if (fEnable = VmnValidMacroNameSt(st) == vmnExists)
				{
				docDot = DocDotMother(selCur.doc);
				FetchSy(BsyOfStDocDot(st, docDot));
				if (vpsyFetch == mctMacro && vpsyFetch->docDot != docDot)
					fEnable = fFalse;
				}
			
			EnableTmc(tmcOK, fEnable);
			/* NOTE: st is used as an sz here */
			GetMenuHelpSz(BcmCurSelTmc(tmcRMCombo), st);
			SanitizeSz(st, sz, sizeof (sz), fFalse);
			SetTmcText(tmcDesc, sz);
			}
		break;

	case dlmClick:
		if (tmc == tmcRMShowAll)
			{
			SetFocusTmc(tmcRMCombo);

			if (wNew != wOld)
				{
				vpref.fShowAllMacros = wNew;
				RedisplayTmc(tmcRMCombo);
				}
			}
		/* break; IE */
		}

	return fTrue;
}


/* V M N  V A L I D  M A C R O  N A M E  S T */
/* %%Function:VmnValidMacroNameSt %%Owner:bradch %%Reviewed:7/31/89 */
int VmnValidMacroNameSt(stName)
char * stName;
{
	int cch;
	char * pch;
	BCM bcm;

	pch = stName + 1;
	cch = stName[0];
	if (cch > cchMaxMacroName || cch-- == 0 || !FAlpha(*pch++))
		return vmnBad;

	while (cch-- > 0)
		{
		if (!FAlpha(*pch) && !FDigit(*pch))
			return vmnBad;

		pch += 1;
		}

	bcm = BsyOfStDocDot(stName, DocDotMother(selCur.doc));
	if (bcm == bcmNil)
		return vmnValid;
	FetchSy(bcm);
	return FElMct(vpsyFetch->mct) ? vmnValid : vmnExists;
}


/* F  C O N F I R M  S A V E  M A C R O S */
/* %%Function:FConfirmSaveMacros %%Owner:bradch %%Reviewed:7/31/89 */
FConfirmSaveMacros(ac)
uns ac;
{
	int imei, imeiMax;
	MEI * pmei;
	BOOL fSave;

	/* if CBT is exiting, don't bother asking - we never save
		anything done in CBT */

	if (!vhwndCBT && vrf.fRanCBT)
		return fTrue;

	imeiMax = (*vhmes)->imeiMax;
	for (imei = 0; imei < imeiMax; ++imei)
		{
		pmei = PmeiImei(imei);
		if (pmei->fDirty || PdodDoc(pmei->docEdit)->fDirty)
			{
			fSave = fFalse;

			switch (ac)
				{
			case acSaveNoQuery:
				fSave = fTrue;
				/* FALL THROUGH */

			case acNoSave:
				break;

			case acSaveAll:
			case acQuerySave:
				switch (IdQuerySaveChanges(pmei))
					{
				case IDYES:
					fSave = fTrue;
					/* FALL THROUGH */

				case IDNO:
					break;

				case IDCANCEL:
					return fFalse;

#ifdef DEBUG
				default:
					Assert(fFalse);
					break;
#endif
					}
				}

			if (fSave)
				{
				/* Save it... */
				if (!FSaveEdMacro(imei, 
						fFalse /* fClosing */))
					{
					ErrorEid(eidNoMemory, 
							"FConfirmSaveMacros");
					return fFalse;
					}
				}
			}
		}

	return fTrue;
}


struct STTB ** hsttbMenuHelp = hNil;


/* F  S E T  M E N U  H E L P */
/* %%Function:FSetMenuHelp %%Owner:bradch %%Reviewed:7/31/89 */
FSetMenuHelp(bsy, st)
uns bsy;
char * st;
{
	extern HPSYT vhpsyt;

	HPSY hpsy;
	int ibst;

	Assert(bsy >= bcmMacStd);
	Assert(bsy != bsyNil);
	
	/* Make sure we have a string table */
	if (hsttbMenuHelp == hNil && 
			(hsttbMenuHelp = HsttbInit(CwFromCch(*st+1), fTrue/*fExt*/)) == hNil)
		{
		return fFalse;
		}

	/* Get the symbol we're changing */
	hpsy = (HPSY) (vhpsyt->grpsy + bsy);
	if (hpsy->stName[0] == 0)
		{
		BCM bcm;
		
		bcm = *((uns HUGE *) (hpsy->stName + 1));
		hpsy = (HPSY) (vhpsyt->grpsy + bcm);
		if (bcm < bcmMacStd)
			return fTrue;
		}

	/* Change the old string or add a new one if there was no old one */
	if (hpsy->ibstMenuHelpP1 != 0)
		{
		if (!FChangeStInSttb(hsttbMenuHelp, hpsy->ibstMenuHelpP1 - 1, st))
			return fFalse;
		}
	else  if (st[0] != 0)
		{
		if ((ibst = IbstAddStToSttb(hsttbMenuHelp, st)) == ibstNil)
			return fFalse;

		hpsy->ibstMenuHelpP1 = ibst + 1;
		}

	return fTrue;
}


/* B S Y  E N S U R E  M A C R O  E X I S T S */
/* %%Function:BsyEnsureMacroExists %%Owner:bradch %%Reviewed:7/31/89 */
BsyEnsureMacroExists(stName, docDot, pfNew)
char * stName;
int docDot;
BOOL * pfNew;
{
	int docMcr, imcr;
	BCM bcm;
	BOOL fNew;
	struct CA ca;

	/* Make sure the doc template has a docMcr. */
	if ((docMcr = PdodDoc(docDot)->docMcr) == docNil)
		{
		if ((docMcr = DocCreateMcr(docDot)) == docNil)
			return bsyNil;
		}

	/* If the macro doesn't exist yet, create an empty one... */
	if (fNew = ((bcm = BsyOfStDocDot(stName, docDot)) == bcmNil || 
			(FetchSy(bcm), (vpsyFetch->mct != mctMacro && 
			vpsyFetch->mct != mctGlobalMacro) || 
			vpsyFetch->docDot != docDot)))
		{
		if ((imcr = ImcrCreate(docMcr, stName, 
				PcaPoint(&ca, docDot, cp0))) == iNil)
			{
			return bsyNil;
			}

		if ((bcm = BsyAddCmd1(stName, docDot, imcr)) == bsyNil)
			return bsyNil;
		DirtyDoc(docDot);
		}

	if (pfNew != NULL)
		*pfNew = fNew;

	return bcm;
}


csconst CHAR stGlobal[] = StKey("Global",Global);


/* F  G E T  S T  M A C R O */
/* %%Function:FGetStMacro %%Owner:bradch %%Reviewed:7/31/89 */
FGetStMacro(doc, st)
int doc;
CHAR * st;
{
	MEI *pmei, *pmeiMac;

	if (vhmes != hNil)
		{
		pmei = (*vhmes)->rgmei;
		pmeiMac = pmei + (*vhmes)->imeiMax;

		while (pmei < pmeiMac)
			{
			if (pmei->docEdit == doc)
				{
				int docDot = DocDotMother(pmei->elg);
				Assert(PdodDoc(doc)->udt == udtMacroEdit);
				if (docDot == docGlobalDot)
					CopyCsSt(stGlobal, st);
				else
					GetDocSt(docDot, st, gdsoShortName);
				st[++(*st)] = ':';
				st[++(*st)] = ' ';
				StStAppend(st, *pmei->pstName);
				return fTrue;
				}
			pmei++;
			}
		}

	return fFalse;
}


/* D O C  E D I T  O F  M E I  C U R */
/* %%Function:DocEditOfMeiCur %%Owner:bradch %%Reviewed:7/31/89 */
int DocEditOfMeiCur()
{
	if (vhmes == hNil)
		return docNil;

	return PmeiCur()->docEdit;
}


/* I M E I  F R O M  S T */
/* %%Function:ImeiFromSt %%Owner:bradch %%Reviewed:7/31/89 */
int ImeiFromSt(stName, docDot)
char * stName;
{
	int imei, imeiMax;
	MEI * pmei;

	if (vhmes != hNil)
		{
		imeiMax = (*vhmes)->imeiMax;
		pmei = (*vhmes)->rgmei;
		for (imei = 0; imei < imeiMax; ++imei, ++pmei)
			{
			if (FEqNcSt(*pmei->pstName, stName) && docDot == pmei->elg)
				return imei;
			}
		}

	return iNil;
}


/* I M E I  F R O M  E L G  E L M */
/* %%Function:ImeiFromElgElm %%Owner:bradch %%Reviewed:7/31/89 */
int ImeiFromElgElm(elg, elm)
int elg, elm;
{
	int imei, imeiMax;
	MEI * pmei;

	if (vhmes != hNil)
		{
		imeiMax = (*vhmes)->imeiMax;
		pmei = (*vhmes)->rgmei;
		for (imei = 0; imei < imeiMax; ++imei, ++pmei)
			{
			if (pmei->elg == elg && pmei->elm == elm)
				return imei;
			}
		}

	return iNil;
}


/* I B  P R O C  M A C R O  E D I T */
/* %%Function:IbProcMacroEdit %%Owner:bradch %%Reviewed:7/31/89 */
IbProcMacroEdit(hwnd, ibm, iibb, wParam, lParam)
HWND hwnd;
WORD ibm;
WORD iibb;
WORD wParam;
LONG lParam;
{
	if (ibm == ibmDblClk)
		FExecCmd(imiEditMacro);
}


/* F  S A V E  E D  M A C R O */
/* %%Function:FSaveEdMacro %%Owner:bradch %%Reviewed:7/31/89 */
FSaveEdMacro(imei, fClosing)
int imei;
BOOL fClosing;
{
	struct MWD * pmwd;
	MEI * pmei;
	int docEdit, docMcr;
	uns cbTokens;
	HQ hqrgbTokens;
	struct CA ca;

	StartLongOp();

	/* Make sure macro is tokenized... */
	pmei = PmeiImei(imei);
	docEdit = pmei->docEdit;
	cbTokens = pmei->cbTokens;
	hqrgbTokens = pmei->hqrgbTokens;

	if ((PdodDoc(docEdit)->fDirty || pmei->hqrgbTokens == hqNil))
		{
		if (!FTokenizePca(PcaSetWholeDoc(&ca, docEdit), 
				&hqrgbTokens, &cbTokens))
			{
			EndLongOp(fFalse);
			return fFalse;
			}

		if (!fClosing)
			FExpandHqrgbToPca(hqrgbTokens,cbTokens,&ca,imei,fFalse);
		}

	pmei = PmeiImei(imei);
	pmei->fDirty = PdodDoc(docEdit)->fDirty = fFalse;
	pmei->fNew = fFalse;
	FreezeHp();
	if (pmei->hqrgbTokens != hqrgbTokens && pmei->hqrgbTokens != hqNil)
		FreeHq(pmei->hqrgbTokens);
	MeltHp();
	pmei->hqrgbTokens = hqrgbTokens;
	pmei->cbTokens = cbTokens;
	pmwd = PmwdMw(pmei->mw);
	docMcr = pmwd->docMcr;
	CaFromIhdd(docMcr, pmei->elm, &ca);
	if (!FReplacePcaWithHqrgb(&ca, hqrgbTokens, cbTokens))
		{
		EndLongOp(fFalse);
		return fFalse;
		}

	DirtyDoc(DocDotMother(docMcr));

	EndLongOp(fFalse);

	return fTrue;
}


/* C M D  R U N  B C M  E L */
/* %%Function:CmdRunBcmEl %%Owner:bradch %%Reviewed:7/31/89 */
CmdRunBcmEl(bcm)
{
	extern int cmmCache;
	extern BOOL fElActive;
	BOOL fElActiveSav;
	CMD cmd;
	MSG msg;
	int cmm;
	
	Assert(bcm != bcmNil);
	
	fElActiveSav = fElActive;
	fElActive = fFalse;
	
	/* PeekMessage takes care of any chained commands... */
	do
		{
		/* Perform "Auto-Supering."  If we are in a template macro foo 
			calling foo, make sure we use a global or built-in foo.  
			Similarly, if we are in a global macro foo calling foo, use 
			the built-in foo.
		*/
		for (cmm = cmmNormal; ; 
			cmm = (cmm == cmmSuper) ? cmmBuiltIn : cmmSuper)
			{
			FetchCm2(bcm, cmm);

			/* We've gotten down to the global one, so use it... */
			if (vpsyFetch->mct != mctMacro && 
					vpsyFetch->mct != mctGlobalMacro)
				{
				break;
				}

			/* If the one we've fetched is "on the stack" (running), we need 
				to keep trying unless we were trying to get the built-in one, 
				in which case, an attempt is being made to recurse so 
				ModeError()!
			*/
			if (!FRunningElgElm(vpsyFetch->docDot, vpsyFetch->imcr))
				break;
			
			if (cmm == cmmBuiltIn)
				{
				ModeError();
				cmd = cmdError;
				goto LReturn;
				}
			}

		cmmCache = cmmNormal; /* fool FetchCm() into using sy we've cached */

		if ((cmd = CmdExecBcm(bcm, hNil)) != cmdOK)
			vrerr = (cmd == cmdCancelled ? rerrCommandFailed : rerrHalt);
		}
	while (PeekMessage(&msg, vhwndApp, WM_COMMAND, WM_COMMAND, 
		PM_NOYIELD | PM_REMOVE) && (int) (bcm = msg.wParam) >= 0);
	
LReturn:
	fElActive = fElActiveSav;

	return cmd;
}


/* I M E I  F R O M  D O C */
/* %%Function:ImeiFromDoc %%Owner:bradch %%Reviewed:7/31/89 */
int ImeiFromDoc(doc)
int doc;
{
	int imei, imeiMac;
	MEI * pmei;

	imeiMac = (*vhmes)->imeiMax;
	pmei = (*vhmes)->rgmei;
	for (imei = 0; imei < imeiMac; imei += 1, pmei += 1)
		{
		if (pmei->docEdit == doc)
			return imei;
		}

	return iNil;
}


/* E L V  F R O M  L I S T  B O X  I */
/* %%Function:ElvFromListBoxI %%Owner:bradch */
ElvFromListBoxI(uList, phpval)
HP * phpval;
{
	int iFrame, iLocal, i;
	ELG elg;
	ELM elm;
	ELV elv;
	char sz [260];
	
	iFrame = iLocal = i = 0;
	while ((elv = ElvEnumerateVar(iFrame, iLocal, sz, &elg, &elm, 
			phpval)) != elvEndStack)
		{
		iLocal += 1;

		switch (elv)
			{
		case elvNum:
		case elvSd:
			if (i == uList)
				return elv;
			i += 1;
			break;

		case elvEndModuleFrame:
		case elvEndProcFrame:
			iLocal = 0;
			iFrame += 1;
			/* FALL THROUGH */

		default:
			continue;
			}
		}
	
	Assert(fFalse);
}


/* I M E I  F R O M  E L G  B C M */
/* %%Function:ImeiFromElgBcm %%Owner:bradch */
int ImeiFromElgBcm(elg, bcm)
int elg, bcm;
{
	int imei, imeiMax;
	MEI * pmei;

	if (vhmes != hNil)
		{
		imeiMax = (*vhmes)->imeiMax;
		pmei = (*vhmes)->rgmei;
		for (imei = 0; imei < imeiMax; ++imei, ++pmei)
			{
			if (pmei->elg == elg && pmei->bcm == bcm)
				return imei;
			}
		}

	return iNil;
}

/* F  R E M O V E  I M E I */
/* %%Function:FRemoveImei %%Owner:chic %%Reviewed: */
/* Remove a Macro Editor Instance given the index imei, 
fCheckSave should be true if you want to save the template file 
when this macro is the last one refering to the template and 
that the template is dirty because other macros changes are 
saved (to docMcr) but has not yet made it into the template (docDot).
Return fFalse if fail to remove the macro or user cancel from the
save message box.
*/ 
BOOL FRemoveImei(imei, fCheckSave)
int imei; /* index of macro editor instance */
BOOL fCheckSave; /* true if want to check to save the template */
	{
	MEI *pmei = PmeiImei(imei);
	int docEdit = pmei->docEdit;
	int docDot = pmei->elg;
	BOOL fDirtyMei;
	BOOL fDirtyDocEdit;

	/* save the dirty bits */
	if (fCheckSave)
		{
		fDirtyMei = pmei->fDirty;
		fDirtyDocEdit = PdodDoc(docEdit)->fDirty;
		}

	if (pmei->fNew)
		{
		if (!FRemoveImcr(*pmei->pstName, docDot, pmei->elm /*imcr*/))
			{
			ErrorEid(eidNoMemory, "FCloseEdMacro");
			return fFalse;
			}
		pmei = PmeiImei(imei); /* reload pointer */
		}
	pmei->fDirty = PdodDoc(docEdit)->fDirty = fFalse;

	/* Save the template too, if the macro is the 
		only thing referencing it... */
	if (fCheckSave && PdodDoc(docDot)->fDirty &&
			PdodDoc(docDot)->crefLock == 1 
			&& WwDisp(docDot, wwNil, fFalse) == wwNil
			&& !FConfirmSaveDoc(docDot, fFalse, acQuerySave))
		{
		/* save is cancelled, restore the dirty bit for next try */
		pmei = PmeiImei(imei);
		pmei->fDirty = fDirtyMei;
		PdodDoc(docEdit)->fDirty = fDirtyDocEdit;
		return fFalse;
		}

	return fTrue;
	}
