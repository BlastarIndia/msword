/*#define ALLOW_ALT*/

/* This file contains the customization commands */



#include "word.h"
#include "error.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "keys.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "props.h" /* sel.h needs this */
#include "sel.h"
#include "doc.h"
#include "opuscmd.h"
#include "rerr.h"
#include "el.h"
#include "macrocmd.h"
#include "rareflag.h"

#include "idd.h"
#include "sdmtmpl.h"
#include "sdmparse.h"


#include "asgn2key.hs"
#include "asgn2key.sdm"
#include "asgn2mnu.hs"
#include "asgn2mnu.sdm"

#include "edmacro.hs"
#include "runmacro.hs"


extern SY * PsyGetSyOfBsy();
extern KME * PkmeOfKcInChain();

extern struct PREF vpref;
extern int vgrfMenuCmdsAreDirty;
extern int vgrfMenuKeysAreDirty;
extern BOOL vfRecording;
extern struct SEL selCur;
extern KMP ** hkmpBase;
extern KMP ** hkmpCur;
extern KMP ** vhkmpUser;
extern HANDLE vhDlg;
extern MUD ** vhmudBase;
extern MUD ** vhmudUser;
extern struct STTB ** hsttbMenu;
extern int docGlobalDot;
extern CHAR szEmpty[];
extern struct MERR vmerr;



/* Command Argument Block for AssignToKey */
#define ikcMaxChangeKeys 8

#define iContextGlobal 0
#define iContextDocType 1


#define imnuMaxUser 8

/* CAB Extension for Assign To Key */
typedef struct _ceatk
	{
	int fDirty : 1;
	int fChanged : 1;
	int mct : 4;
	KMP ** hkmp;
	int kc;
	int ckc;
	int rgkc[ikcMaxChangeKeys];
} CEATK;

#define PceatkDlgCur() ((CEATK *) PcmbDlgCur()->pv)
#define PceatkPcmb() ((CEATK *) pcmb->pv)


/* CAB Extension for Assign To Menu */
typedef struct _ceatm
	{
	int docDot;		/* DocDotMother(selCur.doc) for template
		context, docNil for global */

	int fSelMacro : 1;
	int fDirty : 1;
	int fTemplate : 1;
	int fChanged : 1;
	int mct : 4;

	MUD ** hmudFull;	/* represents full menu */
	MUD ** hmudEdit;	/* just the mud we're editing */

	int ibsyCur;		/* ibsy of item selected in item list */
	int ibsyMac;
	int rgbsy [imtmMaxMenu];/* bsy's of menu items */
} CEATM;

#define PceatmDlgCur() ((CEATM *) PcmbDlgCur()->pv)


/* C M D  C H A N G E  K E Y S */
/* %%Function:CmdChangeKeys %%Owner:krishnam */
CMD CmdChangeKeys(pcmb)
CMB * pcmb;
{
	BOOL fRecord = fFalse;
	CEATK ceatk;

	SetOurKeyState(); /* Make sure we're in sync */

	if (!vrf.fChangeKeys)
		{
		pcmb->pv = &ceatk;
		ceatk.mct = mctGlobal;
		}

	if (FCmdFillCab())
		{
		CABCHANGEKEYS * pcab;

		/* Setup post dialog entries in cab */
		ceatk.fDirty = fFalse;
		ceatk.fChanged = fFalse;
		ceatk.kc = kcNil;
		ceatk.ckc = 0;
		ceatk.hkmp = vhkmpUser;

		pcab = *pcmb->hcab;
		pcab->uKeyList = uNinchList;
		pcab->iContext = iContextGlobal;
		if (!FSetCabSz(pcmb->hcab, szEmpty, 
				Iag(CABCHANGEKEYS, hszMacroList)))
			{
			return cmdNoMemory;
			}
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltChangeKeys)];

		BltDlt(dltChangeKeys, dlt);

		if ((*(HCABCHANGEKEYS)pcmb->hcab)->iContext == iContextDocType)
			ceatk.mct = mctAll;

		vrf.fChangeKeys = fTrue;
		TmcOurDoDlg(dlt, pcmb);
		vrf.fChangeKeys = fFalse;

		switch (pcmb->tmc)
			{
		case tmcError:
			return cmdNoMemory;
			}
		}

	if (pcmb->fAction)
		{
		HCABCHANGEKEYS hcab;
		KMP ** hkmp;
		int kc;
		BCM bcm;
		char stName [cchMaxSyName];

		hcab = pcmb->hcab;

		if ((*hcab)->iContext == iContextGlobal)
			{
			hkmp = vhkmpUser;
			}
		else
			{
			if (selCur.doc == docNil || 
					DocDotMother(selCur.doc) == docNil)
				{
				Assert(fFalse);
				return cmdError;
				}
			hkmp = PdodDoc(DocDotMother(selCur.doc))->hkmpUser;
			}
		AssertH(hkmp);

		if (vrf.fChangeKeys)  // recursive call from fdlg
		PceatkDlgCur()->hkmp = hkmp;

		switch (pcmb->tmc)
			{
		case tmcReset:
			fRecord = fTrue;
			(*hkmp)->ikmeMac = 0;
			goto LDirtyDot;

		case tmcAssignKey:
		case tmcUnassignKey:
		case tmcOK:
			fRecord = fTrue;

			if (fElActive ?
					((uns) (kc = (*hcab)->uKeyList) == uNinchList) :
					((kc = PceatkPcmb()->kc) == kcNil))
				{
				return cmdError;
				}

			if (pcmb->tmc != tmcUnassignKey)
				{
				GetCabSt(hcab, stName, sizeof (stName), 
					Iag(CABCHANGEKEYS, hszMacroList));
				if ((bcm = BcmOfSt(stName)) == bcmNil)
					{
					ErrorEid(eidNoSuchMacro, "CmdChangeKeys");
					return cmdError;
					}
					
				AddKeyToKmp(hkmp, kc, bcm);
				}
			else
				{
				int ikme;
				KMP ** hkmpSav;

				hkmpSav = hkmpCur;
				hkmpCur = hkmp;

				if (FSearchKmp(hkmp, kc, &ikme))
					{
					/* remove from user's keymap */
					KME * pkme;

					pkme = &(*hkmp)->rgkme[ikme];
					if (pkme->kt != ktMacro)
						{
						/* Reset */
						RemoveKeyFromKmp(hkmp, 
								kc, bcm);
						}
					else
						{
						/* Unassign */
						pkme->kt = ktBeep;
						}
					}
				else  if (PkmeOfKcInChain(kc) != NULL)
					{
					/* Unassign */
					AddKeyKtW(hkmp, kc, ktBeep, 0);
					}

				hkmpCur = hkmpSav;
				}
LDirtyDot:
			if (hkmp == vhkmpUser)
				PdodDoc(docGlobalDot)->fDirty = fTrue;
			else
				DirtyDotOfDoc(selCur.doc);

			vgrfMenuKeysAreDirty = 0xffff;
			}

		if (!fElActive && vfRecording && fRecord)
			{
			/* We need to change the key list box index to the
				key code of the selected key.  It sure would be
				nice if SDM had "parsed list boxes." */
			uns uKeyListSav;
			HCABCHANGEKEYS hcab;

			hcab = pcmb->hcab;
			uKeyListSav = (*hcab)->uKeyList;
			(*hcab)->uKeyList = PceatkPcmb()->kc;
			FRecordCab(hcab, IDDChangeKeys, pcmb->tmc, fFalse);
			(*hcab)->uKeyList = uKeyListSav;
			}
		}

	if (pcmb->tmc == tmcCancel && !ceatk.fDirty)
		return cmdCancelled;

	return cmdOK;
}


csconst int rgkcPass [] =
{
	/*#ifdef DEBUG*/
	/*        kcPrintScr, /* ctrl-prtscreen for putting windows in clipboard */
	/*#endif   /* DEBUG */

	kcTab,          kcReturn,       kcEscape,       kcSpace,
			kcUp,           kcDown,         kcLeft,         kcRight,
			kcBeginLine,    kcEndLine,      kcPageUp,       kcPageDown,
			kcF1,           kcBackSpace,	kcClear,
			kcNil
};


/* %%Function:FOKToAssignKc %%Owner:krishnam */
FOKToAssignKc(kc)
int kc;
{
	int kcBase;
	int far * lpkc;
	KME * pkme;

	kcBase = KcStrip(kc);

	/* Convert Alt+F1 & Alt+F2 to F11 & F12 */
	if (FAltKc(kc) && (kcBase == VK_F1 || kcBase == VK_F2))
		{
		kcBase += 10;
		kc += 10;
		kc &= ~KcAlt(0);
		}

	if (
#ifndef ALLOW_ALT
			FAltKc(kc) ||
#endif /* ALLOW_ALT */
			(!FCtrlKc(kc) && ((kcBase >= '0' && kcBase <= 'Z') || 
			(kcBase >= VK_NUMPAD0 && kcBase <= VK_NUMPAD9) ||
			kcBase == VK_DECIMAL)) ||
			kcBase > 0x7f)
		{
		return fFalse;
		}

	for (lpkc = rgkcPass; *lpkc != kcNil; ++lpkc)
		{
		if (kcBase == (*lpkc & 0xff))
			return fFalse;
		}

	return fTrue;
}


/* F  C H A N G E  K E Y S  H O O K */
/* %%Function:FChangeKeysHook %%Owner:krishnam */
FChangeKeysHook(kc)
int kc;
{
	if ((kc & 0xff00) == 0xff00)
		return fFalse;

	if (!FOKToAssignKc(kc))
		return fFalse;

		{
		/* Convert Alt+F1 & Alt+F2 to F11 & F12 */
		int kcBase = KcStrip(kc);
		if (FAltKc(kc) && (kcBase == VK_F1 || kcBase == VK_F2))
			{
			kc += 10;
			kc &= ~KcAlt(0);
			}
		}

	PceatkDlgCur()->kc = kc;
	PceatkDlgCur()->fChanged = fTrue;

	UpdKeyBox();
	/* remove any selection in key list box */
	SetTmcVal(tmcKeyList, uNinchList);

	EnableTmc(tmcAssignKey, fTrue);

	SetChangeKeyFocus();

		/* BLOCK: select key in key list if it's there... */
		{
		int ikc, ikcMac;
		int * pkc;

		pkc = &PceatkDlgCur()->rgkc[0];
		ikcMac = PceatkDlgCur()->ckc;
		for (ikc = 0; ikc < ikcMac; ++ikc, ++pkc)
			{
			if (*pkc == kc)
				{
				SetTmcVal(tmcKeyList, ikc);
				break;
				}
			}
		}

	return fTrue;
}


/* S E T  C H A N G E  K E Y  F O C U S */
/* %%Function:SetChangeKeyFocus %%Owner:krishnam */
SetChangeKeyFocus()
{
	int i;

	i = ValGetTmc(tmcMacroList);

	if (PceatkDlgCur()->kc != kcNil && i != uNinchList)
		{
		SetFocusTmc(tmcAssignKey);
		}
}


/* U P D  K E Y  B O X */
/* This is for the little box that displays the name
	of the current key and what it is bound to */
/* %%Function:UpdKeyBox %%Owner:krishnam */
UpdKeyBox()
{
	KME * pkme;
	char * sz;
	KMP ** hkmpSave = hkmpCur;
	BOOL f;
	int kt, ikme, kc;
	char rgch[ichMaxBufDlg];

	hkmpCur = PceatkDlgCur()->hkmp;

	if ((kc = PceatkDlgCur()->kc) != kcNil)
		FKcToSz(kc, rgch);
	else
		rgch[0] = '\0';
	SetTmcText(tmcKey, rgch);
	EnableTmc(tmcAssignKey, kc != kcNil);

	if (kc != kcNil)
		{
		extern BCM vbcmFetch;

		pkme = PkmeOfKcInChain(kc);
		kt = ktNil;
		f = (pkme == 0 || (kt = pkme->kt) != ktMacro);
		if (pkme != 0)
			FetchCm(pkme->bcm);

		if (f |= (vbcmFetch == bcmNil))
			{
			SetTmcText(tmcCurrent, 
					SzFrameKey("[currently unassigned]",CurrentlyUnassigned));
			}
		else
			{
			StToSzInPlace(
					sz = PsyGetSyOfBsy(rgch, pkme->bcm)->stName);
			SetTmcText(tmcCurrent, sz);
			}
		}
	else
		{
		f = fTrue;
		SetTmcText(tmcCurrent, szEmpty);
		}

	hkmpCur = (*hkmpCur)->hkmpNext;
	if (kc != kcNil && kt == ktBeep && PkmeOfKcInChain(PceatkDlgCur()->kc) != 0)
		{
		SetTmcText(tmcUnassignKey, SzSharedKey("Res&et",Reset));
		f = fFalse;
		}
	else
		{
		SetTmcText(tmcUnassignKey, SzSharedKey("&Unassign",Unassign));
		}

	EnableTmc(tmcUnassignKey, !f);

	hkmpCur = hkmpSave;
}


/* F  D L G  C H A N G E  K E Y S */
/* %%Function:FDlgChangeKeys %%Owner:krishnam */
BOOL FDlgChangeKeys(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int kc;
	BOOL fRet;
	char rgch [ichMaxBufDlg];
	char sz [ichMaxBufDlg];

	fRet = fTrue; /* OK until proven not OK */

	switch (dlm)
		{
	case dlmDblClk:
		return fFalse;

	case dlmInit:
		EnableTmc(tmcTemplateKey, !FDisableTemplate(selCur.doc));
		EnableTmc(tmcAssignKey, fFalse);
		EnableTmc(tmcUnassignKey, fFalse);

		EnableTmc(tmcAssignKey, fFalse);
		EnableTmc(tmcUnassignKey, fFalse);

		SetTmcVal(tmcMacroList, 0);
		goto LSelMacro;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (vmerr.fMemFail)
			ErrorNoMemory(eidNoMemOpRepeats);

		return fFalse;

	case dlmTerm:
		/* for OK, do basically what assign does as long as the
					assign button is not gray - may do the action twice,
					but this is simpler than checking on all the changes...
				*/
		if (tmc == tmcOK && PceatkDlgCur()->fChanged && 
				PceatkDlgCur()->kc != kcNil && FEnabledTmc(tmcAssignKey))
			{
			CmdDlgAction(CmdChangeKeys, tmcAssignKey);

			if (vmerr.fMemFail)
				{
				vmerr.fHadMemAlert = fFalse;
				ErrorNoMemory(eidNoMemOpRepeats);
				return fTrue;
				}

			PceatkDlgCur()->fDirty = fTrue;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcReset:
		case tmcAssignKey:
		case tmcUnassignKey:
				{
				CmdDlgAction(CmdChangeKeys, tmc);

				if (vmerr.fMemFail)
					{
					vmerr.fHadMemAlert=fFalse;
					ErrorNoMemory(eidNoMemOpRepeats);
					return fFalse;
					}
				
				SetFocusTmc(tmcMacroList);

				PceatkDlgCur()->fDirty = fTrue;
				PceatkDlgCur()->fChanged = fFalse;
				
				PceatkDlgCur()->kc = kcNil;
				UpdKeyBox();

				ChangeCancelToClose();
				FillKeyList(BcmCurSelTmc(tmcMacroList));

				fRet = fFalse; /* so the recorder doesn't record... */
				break;
				}

		case tmcMacroList:
			PceatkDlgCur()->fChanged = fTrue;
LSelMacro:
				{
				BCM bcm;

				bcm = BcmCurSelTmc(tmcMacroList);
				FillKeyList(bcm);
				GetMenuHelpSz(bcm, rgch);
				SanitizeSz(rgch, sz, sizeof (sz), fFalse);
				SetTmcText(tmcDescKey, sz);
				return fTrue;
				}

		case tmcKeyList:
			if (wNew == LB_ERR)
				return fTrue;

			Assert(wNew < PceatkDlgCur()->ckc);
			if (FOKToAssignKc(kc = PceatkDlgCur()->rgkc[wNew]))
				{
				PceatkDlgCur()->kc = kc;
				PceatkDlgCur()->fChanged = fTrue;
				}
			break;

		case tmcGlobalKey:
		case tmcTemplateKey:
			if (wOld == wNew)
				return fTrue;

			PceatkDlgCur()->fDirty = fFalse;
			PceatkDlgCur()->hkmp=(tmc==tmcTemplateKey &&
					selCur.doc != docNil &&
					DocDotMother(selCur.doc) != docNil) ? PdodDoc(DocDotMother(selCur.doc))->hkmpUser : vhkmpUser;

			/* refill macro list */
			GetTmcText(tmcMacroList, rgch, sizeof (rgch));
			PceatkDlgCur()->mct = wNew == iContextGlobal ? 
					mctGlobal : mctAll;
			RedisplayTmc(tmcMacroList);

			if (rgch[0] != 0)
				SetTmcText(tmcMacroList, rgch);
			if (ValGetTmc(tmcMacroList) == uNinchList)
				SetTmcVal(tmcMacroList, 0);
			FillKeyList(BcmCurSelTmc(tmcMacroList));
			}

		UpdKeyBox();
		/* break; */
		}

	return fRet;
}


/* F I L L  K E Y  L I S T */
/* %%Function:FillKeyList %%Owner:krishnam */
FillKeyList(bcm)
int bcm;
{
	int ikc, * pkc, kc;
	KMP ** hkmpSav;

	hkmpSav = hkmpCur;
	hkmpCur = PceatkDlgCur()->hkmp;

	StartListBoxUpdate(tmcKeyList); /* empties box */

	/* put in the key names*/
	pkc = &PceatkDlgCur()->rgkc[0];
	for (ikc = 0, kc = KcFirstOfBcm(PceatkDlgCur()->hkmp, bcm);
			ikc < ikcMaxChangeKeys && kc != kcNil;
			kc = KcNextOfBcm(bcm))
		{
		int ikcT;
		int * pkcT;
		char sz[cchMaxSzKey];

		pkcT = &PceatkDlgCur()->rgkc[0];
		for (ikcT = 0; ikcT < ikc; ++ikcT, ++pkcT)
			{
			int kcT = *pkcT;
			int kcBase = kc & 0xff;
			int kcTBase = kcT & 0xff;

			/* Don't put key in list twice! */
			if (kcT == kc || 
					((kc & 0xff00) == (kcT & 0xff00) &&
					((kcBase == VK_CLEAR && kcTBase == VK_NUMPAD5) ||
					(kcBase == VK_NUMPAD5 && kcTBase == VK_CLEAR))))
				goto LContinue1;
			}

		if (kc != 0)
			{
			FKcToSz(kc, sz);
			AddListBoxEntry(tmcKeyList, sz);
			ikc += 1;
			*pkc++ = kc;
			}
LContinue1:
		;
		}
	PceatkDlgCur()->ckc = ikc;

	/* remove any selection in key list box */
	SetTmcVal(tmcKeyList, uNinchList);

	hkmpCur = hkmpSav;

	EndListBoxUpdate(tmcKeyList);
}



/* M e n u  T h i n g s */

/* %%Function:HmudFromCab %%Owner:krishnam */
MUD ** HmudFromCab(hcab)
HCABASSIGNTOMENU hcab;
{
	if ((*hcab)->iContext == iContextGlobal)
		{
		return vhmudUser;
		}
	else
		{
		int docDot;

		if (selCur.doc == docNil || 
				(docDot = DocDotMother(selCur.doc)) == docNil)
			{
			return hNil;
			}
		return HmudUserPdod(PdodDoc(docDot));
		}
}



/* C M D  A S S I G N  T O  M E N U */
/* %%Function:CmdAssignToMenu %%Owner:krishnam */
CMD CmdAssignToMenu(pcmb)
CMB * pcmb;
{
	CEATM ceatm;
	extern BCM vbcmFetch;

	vgrfMenuCmdsAreDirty = 0xffff; /* may as well do it now! */

	pcmb->pv = &ceatm;
	ceatm.fSelMacro = fFalse;
	ceatm.fDirty = fFalse;
	ceatm.fChanged = fFalse;
	ceatm.ibsyCur = uNinchList;
	ceatm.mct = mctGlobal;

	if (pcmb->fDefaults)
		{
		((CABASSIGNTOMENU *) *pcmb->hcab)->iContext = iContextGlobal;
		FSetCabSz(pcmb->hcab, szEmpty, 
				Iag(CABASSIGNTOMENU, hszTitle));
		FSetCabSz(pcmb->hcab, szEmpty,
				Iag(CABASSIGNTOMENU, hszMacroList));
		FSetCabSz(pcmb->hcab, szEmpty,
				Iag(CABASSIGNTOMENU, hszMenuList));
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		CABASSIGNTOMENU * pcab;
		char dlt [sizeof (dltAssignToMenu)];

		pcab = *pcmb->hcab;

		if (!FSetCeatmContext(&ceatm, pcab->iContext))
			return cmdNoMemory;

		BltDlt(dltAssignToMenu, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdNoMemory;
			}
		}

	if (pcmb->cmm != cmmNormal && pcmb->fAction)
		{
		BCM bcm;
		MUD ** hmud;
		uns imnu;
		char stMacro [cchMaxSyName];
		char stMenu [ichMaxBufDlg];
		char stItem [ichMaxBufDlg];

		ceatm.fTemplate = (((CABASSIGNTOMENU *) *pcmb->hcab)->iContext 
				!= iContextGlobal);
		/* called from a macro */
		switch (pcmb->tmc)
			{
		case tmcResetMenu:
			if ((hmud = HmudFromCab(pcmb->hcab)) == hNil)
				return cmdError;
			ClearMud(hmud);
			ceatm.fDirty = fTrue;
			vgrfMenuKeysAreDirty = 0xffff;
			break;

		case tmcAssignMenu:
		case tmcUnassignMenu:
		case tmcOK:
			GetCabSt(pcmb->hcab, stMacro, sizeof (stMacro),
					Iag(CABASSIGNTOMENU, hszMacroList));

			if (stMacro[0] > 0 && stMacro[1] == '-')
				bcm = bcmNil;
			else  if ((bcm = BcmOfSt(stMacro)) == bcmNil || 
					(FetchCm(bcm), (vbcmFetch == bcmNil || 
					vpsyFetch->mct == mctEl)))
				{
				ErrorEid(eidNoSuchMacro, "CmdAssignToMenu");
				return cmdError;
				}

			GetCabSt(pcmb->hcab, stMenu, sizeof (stMenu),
					Iag(CABASSIGNTOMENU, hszMenuList));
			if ((imnu = IbstFindSt(hsttbMenu, stMenu)) >= imnuMaxUser)
				{
LBadParam:
				Assert(fElActive);
				RtError(rerrBadParam);
				Assert(fFalse); /* NOT REACHED */
				}

			GetCabSt(pcmb->hcab, stItem, sizeof (stItem),
					Iag(CABASSIGNTOMENU, hszTitle));

			Assert(stItem[0] >= 0);
			if ((stItem[0] == 0 && pcmb->tmc != tmcUnassignMenu)
					|| stItem[0] >= cchMaxMenuText)
				goto LBadParam;

			if ((hmud = HmudFromCab(pcmb->hcab)) == hNil)
				return cmdError;

			if (pcmb->tmc == tmcUnassignMenu)
				{
				int imtm;
				
				if (!FFindImnuBsy(hmud, imnu, bcm, &imtm) &&
						!FFindImnuBsy(vhmudBase, imnu, bcm, &imtm) &&
						ceatm.fTemplate &&
						!FFindImnuBsy(vhmudUser, imnu, bcm, &imtm))
					{
					return cmdOK;
					}
				}

			/* count no of menu items and check to see if we
			   exceed max. */
			if (pcmb->tmc == tmcAssignMenu || pcmb->tmc == tmcOK)
				{
				int wItem=0, imtm, imtmMac;
				MTM *pmtm;

				pmtm = (*vhmudBase)->rgmtm;
				imtmMac = (*vhmudBase)->imtmMac;
				for (imtm=0; imtm < imtmMac && pmtm->imnu < imnu; ++pmtm, ++imtm)
					;
				for (; imtm < imtmMac && pmtm->imnu == imnu; ++pmtm, ++imtm, ++wItem)
					;

				pmtm = (*hmud)->rgmtm;
				imtmMac = (*hmud)->imtmMac;
				for (imtm=0; imtm < imtmMac && pmtm->imnu < imnu; ++pmtm, ++imtm)
					;
				for (; imtm < imtmMac && pmtm->imnu == imnu; ++pmtm, ++imtm, ++wItem)
					;

				if (wItem >= imtmMaxMenu-1)
						return cmdError;
				}

			if (!FAddToMud(hmud, imnu, stItem, bcm, 
					pcmb->tmc == tmcUnassignMenu))
				{
				ErrorEid(eidNoMemory, "CmdAssignToMenu");
				return cmdError;
				}
			ceatm.fDirty = fTrue;
			vgrfMenuKeysAreDirty = 0xffff;
			/* break; */
			}
		}

	if (pcmb->fDialog)
		{
		FreeHmud(ceatm.hmudFull);
		}

	if (ceatm.fDirty)
		{
		if (ceatm.fTemplate)
			{
			/* Doc Type */
			DirtyDotOfDoc(selCur.doc);
			}
		else
			{
			PdodDoc(docGlobalDot)->fDirty = fTrue;
			}
		}

	if (pcmb->tmc == tmcCancel && !ceatm.fDirty)
		return cmdCancelled;

	return cmdOK;
}


/* %%Function:FSetCeatmContext %%Owner:krishnam */
FSetCeatmContext(pceatm, iContext)
CEATM * pceatm;
int iContext;
{
	pceatm->docDot = (selCur.doc == docNil) ? 
			docNil : DocDotMother(selCur.doc);

	if (iContext == iContextGlobal)
		{
		pceatm->fTemplate = fFalse;
		pceatm->hmudEdit = vhmudUser;
		pceatm->mct = mctGlobal;
		}
	else
		{
		if (pceatm->docDot == docNil)
			{
			Assert(fElActive);
			RtError(rerrModeError);
			}
		pceatm->fTemplate = fTrue;
		pceatm->hmudEdit = HmudUserPdod(PdodDoc(pceatm->docDot));
		pceatm->mct = mctAll;
		}

	AssertH(pceatm->hmudEdit);

	if ((pceatm->hmudFull = HmudInit(0)) == hNil)
		goto LNoMemory;

	if (!FCopyMudToMud(vhmudBase, pceatm->hmudFull))
		{
		FreeHmud(pceatm->hmudFull);
		pceatm->hmudFull = hNil;
LNoMemory:
		return fFalse;
		}

	if (!FMergeMuds(pceatm->hmudFull, vhmudUser))
		goto LNoMemory;

	if (iContext == iContextDocType)
		if (!FMergeMuds(pceatm->hmudFull, pceatm->hmudEdit))
			goto LNoMemory;

	return fTrue;
}


/* F D L G  A S S I G N  T O  M E N U */
/* %%Function:FDlgAssignToMenu %%Owner:krishnam */
BOOL FDlgAssignToMenu(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	MUD ** hmud;
	int bcm;
	CEATM * pceatm;
	HCAB hcab;
	char st[ichMaxBufDlg];
	char szSugg[ichMaxBufDlg];
	extern int vbsySepCur;

	pceatm = PceatmDlgCur();

	switch (dlm)
		{
	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (vmerr.fMemFail)
			ErrorNoMemory(eidNoMemOpRepeats);

		return fFalse;

	case dlmDblClk:
		return fFalse;

	case dlmInit:
		EnableTmc(tmcTemplateMenu, !FDisableTemplate(selCur.doc));
		EnableTmc(tmcUnassignMenu, fFalse);

		AddListBoxEntry(tmcMnuMacroList, 
				SzSharedKey("----------------",	ListBoxDashes));

		SetTmcVal(tmcMnuMacroList, 0);
		SetTmcVal(tmcMenuList, 0);

		UpdMenuTitle();
		UpdItemList();
		goto LSetDesc;

	case dlmTerm:
		/* for OK, do basically what assign does as long as the
					assign button is not gray - may do the action twice,
					but this is simpler than checking on all the changes...
				*/
		if (tmc == tmcOK && pceatm->fChanged &&
				FEnabledTmc(tmcAssignMenu))
			{
			goto LChangeMenu;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcTitleList:
			if (wNew == uNinchList)
				pceatm->fChanged = fTrue;

			EnableTmc(tmcUnassignMenu, 
					(pceatm->ibsyCur = wNew) != uNinchList);
			break;

		case tmcGlobalMenu:
		case tmcTemplateMenu:
			if (wOld == wNew)
				break; /* no change! */

LContext:
			/* NOTE: st is used here as an sz */
			GetTmcText(tmcMnuMacroList, st, sizeof (st));

			/* set up for new context */
			if (pceatm->fDirty)
				{
				if (pceatm->fTemplate)
					DirtyDotOfDoc(selCur.doc);
				else
					PdodDoc(docGlobalDot)->fDirty = fTrue;

				pceatm->fDirty = fFalse;
				}

			FreeHmud(pceatm->hmudFull);

			if (!FSetCeatmContext(pceatm, wNew))
				ErrorEid(eidNoMemory, "DlgfAssignToMenu");

			/* update all the list boxes */
			UpdMenuTitle();
			UpdItemList();

			FillMacroListTmc(tmcMnuMacroList, PceatmDlgCur()->mct);
	    	AddListBoxEntry(tmcMnuMacroList, 
				SzSharedKey("----------------",	ListBoxDashes));

			/* Try to select the same macro that was selected
				before changing contexts. */
			if (*st != 0) /* NOTE: st is really an sz! */
				SetTmcText(tmcMnuMacroList, st);
			if (ValGetTmc(tmcMnuMacroList) == uNinchList)
				SetTmcVal(tmcMnuMacroList, 0);
LSetDesc:
			if ((int) (bcm = BcmCurSelTmc(tmcMnuMacroList)) >= 0)
				{
				GetMenuHelpSz(bcm, szSugg);
				/* NOTE: st is used an an sz here */
				SanitizeSz(szSugg, st, sizeof (st), fFalse);
				SetTmcText(tmcDescMenu, st);
				}
			else
				{
				SetTmcText(tmcDescMenu,
						SzSharedKey("-- Separator --",
						Separator));
				}
			break;

		case tmcAssignMenu:
		case tmcUnassignMenu:
			/* tmcOK can get here from dlmTerm */
LChangeMenu:
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			/* sdm filter will take down dialog */
			if (hcab == hcabNull)
				return (fTrue);

			if (pceatm->ibsyMac >= imtmMaxMenu - 1)
				{
				if (tmc == tmcOK)
					/* just ignore the extra assingment */
					break;
				else  if (tmc == tmcAssignMenu)
					{
					extern BOOL vfAwfulNoise;
					Beep();
					vfAwfulNoise = fFalse;
					return fFalse;
					}
				}

			st[0] = CchGetTmcText(tmcTitle, st + 1, 
					sizeof (st) - 1);

			if (st[0] >= cchMaxMenuText)
				{
				SetFocusTmc(tmcTitle);
				ErrorEid(eidMenuTextTooLong,"FDlgAssignToMenu");
				EnableTmc(tmcOK,fFalse);
				EnableTmc(tmcCancel,fTrue);
				return fFalse;
				}

			if (tmc != tmcOK)
				EnableTmc(tmcOK, fTrue);

			/* For unassign, we grab the bsy out of our saved
				set because there might not be a macro selected
				in the list (for separators and deleted macros).
				For assign, we get it from the macro selected
				because it's not in our saved set yet! */
			if (tmc == tmcUnassignMenu)
				{
				Assert(pceatm->ibsyCur != uNinchList);
				bcm = pceatm->rgbsy[pceatm->ibsyCur];
				}
			else
				bcm = BcmCurSelTmc(tmcMnuMacroList);

			if (!FAddToMud(pceatm->hmudFull,
					ValGetTmc(tmcMenuList),
					st, bcm, (tmc == tmcUnassignMenu)) ||
					!FAddToMud(pceatm->hmudEdit,
					ValGetTmc(tmcMenuList),
					st, 
					(tmc==tmcAssignMenu && (uns)bcm==bsyNil)?
					vbsySepCur - 1 : bcm, 
					(tmc == tmcUnassignMenu)))
				{
				ErrorEid(eidNoMemory, "FDlgAssignToMenu");
				}

			pceatm->fDirty = fTrue;
			vgrfMenuKeysAreDirty = 0xffff;

			if (tmc != tmcOK)
				{
				SetFocusTmc(tmcMnuMacroList);
				UpdItemList();
				ChangeCancelToClose();
				}
			else  if (vfRecording)
				{
				FRecordCab(hcab, IDDAssignToMenu, 
						tmcAssignMenu, fFalse);
				}

			pceatm->fChanged = fFalse;
			break;

		case tmcResetMenu:
			wNew = ValGetTmc(tmcMnuContext);
			ClearMud(pceatm->hmudEdit);
			pceatm->fDirty = fTrue;
			pceatm->fChanged = fFalse;
			ChangeCancelToClose();
			SetFocusTmc(tmcMnuMacroList);
			goto LContext;

		case tmcMnuMacroList:
			if (!pceatm->fSelMacro)
				{
				pceatm->fChanged = fTrue;
				pceatm->fSelMacro = fTrue;
				UpdMenuTitle();
				pceatm->fSelMacro = fFalse;

				GrayButtonOnBlank(tmcTitle, tmcOK);
				GrayButtonOnBlank(tmcTitle, tmcAssignMenu);

				goto LSetDesc;
				}
			break;

		case tmcMenuList: /* contents of menu bar */
			pceatm->fChanged = fTrue;
			UpdItemList();
			/* break; */
			}
		break;

	case dlmChange:
		switch (tmc)
			{
		case tmcTitle & ~ftmcGrouped: /* menu item list */
			GrayButtonOnBlank(tmcTitle, tmcOK);
			GrayButtonOnBlank(tmcTitle, tmcAssignMenu);
			if (FEnabledTmc(tmcOK))
				{
				pceatm->fChanged = fTrue;
				if (!pceatm->fSelMacro)
					SelMacroFromItem();
				goto LSetDesc;
				}
			/* break; */
			}
		/* break; */
		}

	return fTrue;
}


/* S E L  M A C R O  F R O M  I T E M */
/* Select the macro currently assigned to the selected menu item */
/* %%Function:SelMacroFromItem %%Owner:krishnam */
SelMacroFromItem()
{
	int ibsy;
	uns bsy;

	if ((ibsy = ValGetTmc(tmcTitleList)) == uNinchList)
		return;

	bsy = PceatmDlgCur()->rgbsy[ibsy];

	PceatmDlgCur()->fSelMacro = fTrue; /* hack to keep from looping */
	if ((int) bsy < 0)
		{
		/* separator */
		SetTmcVal(tmcMnuMacroList, 0);
		}
	else
		{
		SY * psy;
		char sz [cchMaxSyName], rgchSy [cbMaxSy];
		uns cch;

		psy = PsyGetSyOfBsy(rgchSy, bsy);
		cch = umin(psy->stName[0], sizeof(sz)-1);
		bltb(&(psy->stName[1]), sz, cch);
		*(sz+cch)='\0';
		SetTmcText(tmcMnuMacroList, sz);
		}

	PceatmDlgCur()->fSelMacro = fFalse; /* un-hack */
}


/* U P D  M E N U  T I T L E */
/* Update name of menu item based on currently selected macro */
/* %%Function:UpdMenuTitle %%Owner:krishnam */
UpdMenuTitle()
{
	BCM bcm;
	MTM * pmtm;
	int imtm, imtmMac, imnu;
	char szBuf [ichMaxBufDlg];

	bcm = BcmCurSelTmc(tmcMnuMacroList);
	imnu = ValGetTmc(tmcMenuList);
	pmtm = (*vhmudBase)->rgmtm;
	imtmMac = (*vhmudBase)->imtmMac;
	imtm = 0;

	for ( ; imtm < imtmMac && pmtm->imnu < imnu; ++pmtm, ++imtm)
		;
	for ( ; imtm < imtmMac && pmtm->imnu == imnu; ++pmtm, ++imtm)
		{
		if (pmtm->bcm == bcm)
			{
			if (pmtm->ibst != 0)
				{
				GetSzFromSttb(hsttbMenu, pmtm->ibst, szBuf);
				goto LHaveIt;
				}
			break;
			}
		}

	CchBuildMenuSz(bcm, szBuf, grfBldMenuDef);
LHaveIt:
	;

	SetTmcText(tmcTitle, szBuf);
	CompleteComboTmc(tmcTitle); /* force list box completion if a combo */
}


/* U P D  I T E M  L I S T */
/* Update the listbox containing items of the selected menu. */
/* %%Function:UpdItemList %%Owner:krishnam */
UpdItemList()
{
	int imnu, imtm, imtmMac, ibsyMac, bsySel, iSel;
	int * pbsy;
	MTM * pmtm;
	MUD * pmud, ** hmud;

	PceatmDlgCur()->ibsyCur = uNinchList;

	/* We ran out of memory earlier, so just return... */
	if (PceatmDlgCur()->hmudFull == hNil)
		return;

	/* Inhibit listbox display updating */
	StartListBoxUpdate(tmcTitle);

	imnu = ValGetTmc(tmcMenuList);

	hmud = PceatmDlgCur()->hmudFull;
	pmud = *hmud;

	pmtm = &pmud->rgmtm[0];
	imtm = 0;
	imtmMac = pmud->imtmMac;

	bsySel = BcmCurSelTmc(tmcMnuMacroList);
	iSel = -1;

	for ( ; imtm < imtmMac && pmtm->imnu < imnu; ++pmtm, ++imtm)
		;

	pbsy = PceatmDlgCur()->rgbsy;
	for (ibsyMac = 0; imtm < imtmMac && pmtm->imnu == imnu; 
			++pmtm, ++imtm)
		{
		uns bsy;
		int ich;
		char rgch[ichMaxBufDlg];

		if (pmtm->fRemove)
			continue;

		bsy = pmtm->bsy;

		if (bsy == bcmFileCache || bsy == bcmWndCache)
			continue;

		if (bsySel == pmtm->bsy)
			iSel = ibsyMac;

		*pbsy++ = pmtm->bsy;
		ibsyMac += 1;

		FGetMtmSz(pmtm, rgch);
		AddListBoxEntry(tmcTitle, rgch);

		/* as above call causes a heap movement */
		pmud = *hmud;
		pmtm = &pmud->rgmtm[imtm];
		}

	Assert(ibsyMac < imtmMaxMenu);
	PceatmDlgCur()->ibsyMac = ibsyMac;

	/* Redisplay the listbox */
	EndListBoxUpdate(tmcTitle);
	if (iSel != -1)
		{
		SetTmcVal(tmcTitleList, iSel);
		PceatmDlgCur()->ibsyCur = iSel;
		}
	EnableTmc(tmcUnassignMenu, (ValGetTmc(tmcTitleList) != uNinchList));
}


/* %%Function:WListMenus %%Owner:krishnam */
WORD WListMenus(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		if (isz < imnuMaxUser)
			{
			GetSzFromSttb(hsttbMenu, isz, sz);
			return fTrue;
			}
		}

	return 0;
}


#ifdef DEBUG
/* Dump hmudFull to terminal */
/* %%Function:CmdDumpMud %%Owner:krishnam */
CMD CmdDumpMud()
{
	DumpMud(PceatmDlgCur()->hmudFull);
}


#endif



/* %%Function:DirtyDotOfDoc %%Owner:krishnam */
DirtyDotOfDoc(doc)
int doc;
{
	struct DOD * pdod;
	int docDot;

	Assert(doc != docNil);
	docDot = DocDotMother (doc);

	if (docDot != docNil)
		PdodDoc(docDot)->fDirty = fTrue;
}


/* %%Function:WListMacros %%Owner:krishnam */
WORD WListMacros(tmm, sz, isz, filler, tmc, idd)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD idd;
{
	if (tmm == tmmCount)
		{
		MCT mct;

		if (vmerr.fMemFail)
			return (0);

		switch (idd)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			mct = mctGlobal;
			break;
#endif

		case IDDChangeKeys:
			mct = PceatkDlgCur()->mct;
			break;

		case IDDAssignToMenu:
			mct = PceatmDlgCur()->mct;
			break;

		case IDDEdMacro:
			mct = MctEdMacroList(PcmbDlgCur());
			tmc = tmcEdMacroList;
			break;

		case IDDRunMacro:
			mct = vpref.fShowAllMacros ? mctAll : mctMacros;
			tmc = tmcRMCombo;
			break;
			}

		FillMacroListTmc(tmc, mct);
		}

	return 0;
}


/* M E R G E  M U D S */
/* Apply hmudSrc to hmudDest */
/* NOTE: this may leave hmudDest partly changed in low mem situations! */
/* %%Function:FMergeMuds %%Owner:krishnam */
BOOL FMergeMuds(hmudDest, hmudSrc)
MUD ** hmudDest;
MUD ** hmudSrc;
{
	MUD * pmudSrc, * pmudDest;
	MTM * pmtm, * pmtmDest;
	int imtm, imtmDest, imtmMac;

	pmudSrc = *hmudSrc;
	pmudDest = *hmudDest;

	imtmMac = pmudSrc->imtmMac;
	for (pmtm = &pmudSrc->rgmtm[imtm = 0]; imtm < imtmMac; ++imtm, ++pmtm)
		{
		if (!FFindImnuBsy(hmudDest, pmtm->imnu, pmtm->bsy, &imtmDest))
			{
			if (!FSplitRgmtm(hmudDest, imtmDest))/*HEAP MOVEMENT*/
				{
				return fFalse;
				}

			pmudDest = *hmudDest;
			pmudSrc = *hmudSrc;
			pmtm = &pmudSrc->rgmtm[imtm];
			}
		pmudDest->rgmtm[imtmDest] = *pmtm;
		}

	return fTrue;
}

