/*
*   style.c  --  ported from Mac 8/86, dsb
*           contains front end dialog code for manipulating styles.
*/

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW

#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOGDI
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
#include "keys.h"
#include "props.h"
#include "prm.h"
#include "disp.h"
#include "sel.h"
#include "doc.h"
#include "file.h"
#include "debug.h"
#include "error.h"
#include "ch.h"
#include "fontwin.h"

#include "message.h"
#include "winddefs.h"
#include "doslib.h"
#include "wininfo.h"
#include "core.h"
#include "prompt.h"

#include "style.h"
#include "opuscmd.h"
#include "inter.h"
#define REPEAT
#define NOSTYCAB
#include "ruler.h"
#include "ibdefs.h"
#include "cmdlook.h"
#include "idd.h"
#include "rerr.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "tmc.h"

#include "edstyle.hs"
#include "edstyle.sdm"
#include "renstyle.hs"
#include "renstyle.sdm"
#include "mrgstyle.hs"
#include "mrgstyle.sdm"

#include "para.hs"
#include "char.hs"
#include "tabs.hs"

#ifdef PROTOTYPE
#include "style.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */
extern int vdocFetch;
extern int docGlobalDot;
extern struct SEL selCur;
extern struct PAP vpapFetch;
extern struct CHP vchpFetch;
extern struct CA caPara;
extern struct CA caPage;
extern int wwCur;
extern struct MWD **hmwdCur;
extern KMP ** hkmpCur;
extern struct PREF vpref;
extern struct MERR vmerr;
extern BOOL fCopiedLBSelection;
extern CHAR szEmpty[];
extern HWND vhwndApp;
extern int vstcStyle;
extern int vstcLast;
/* declared in stylesub.c */
extern int vistLbMac;
extern int vdocStsh;
extern int vgrpfKeyBoardState;
extern int vfFileCacheDirty;
extern CHAR szDot[];
extern CHAR stDOSPath[];
extern BOOL fElActive;
extern BOOL vfRecording;
extern RRF  rrf;
extern BOOL             vfRecorderOOM;

/* G L O B A L S */
/* when vstcStyle == stcNil, this means that the Define Styles dialog is
	inactive. When it contains a value from 0 to 255, it is the stc for the
	style that is currently selected by Define Style. */
int             vstcStyle = stcNil;
int             vstcLast = stcNil;
int             vcchPapxLast = 0;

int vfDefineStyle = fFalse;
int vfShowAllStd = fFalse;
int vstcSel = stcNil;
int vstcBackup = stcNil;
int ibstFontDS = ibstNil;
char (**vhmpiststcp)[];

int vstcEl = stcNil;
BOOL vfElTemplateStyle;
char vstElStyle [cchMaxStyle];

void ToggleShowStd();
CMD CmdMrgSty();
CMD CmdGosubMerge();
CMD CmdGosubRename();
CMD CmdDeleteStyle();
CMD CmdGotoDefine();


/* %%Function:CmdDefineStyle %%Owner:davidbo */
CMD CmdDefineStyle(pcmb)
CMB * pcmb;
{
	int tmc;
	int stcp;
	int docTest, fHasDot;
	CMD cmd;
	struct DOD *pdod;
	CESTY cesty;

	if (fElActive)
		return CmdElDefineStyle(pcmb);

	cmd = cmdOK;
	vistLbMac = 0;
	if ((vhmpiststcp = HAllocateCw(cwMpiststcp)) == hNil)
		{
		cmd = cmdNoMemory;
		goto LRet;
		}

	cesty.fDirty = fFalse;
	cesty.istLb = istLbNil;
	cesty.dlmLast = 0;
	cesty.fFake = fFalse;
	cesty.fIgnore = fFalse;
	cesty.fStyleDirty = fFalse;
	SetVdocStsh(selCur.doc);
	SetVstcSel();
	docTest = PdodDoc(vdocStsh)->docDot;
	if (docTest == docNil)
		{
		docTest = docGlobalDot;
		if (docTest == docNil)
			fHasDot = fFalse;
		else
			goto LTest;
		}
	else
		{
LTest:
		fHasDot = fTrue;
		pdod = PdodDoc(docTest);
		if (pdod->fn != fnNil && PfcbFn(pdod->fn)->fReadOnly)
			fHasDot = fFalse;
		}
	cesty.fHasDot = fHasDot;

	if (FCmdFillCab())
		{
		((CABDEFINESTYLE *) *pcmb->hcab)->sab = 0;
		((CABDEFINESTYLE *) *pcmb->hcab)->fTemplate = fFalse;

		InitStyleCabToStc(pcmb, vstcSel);

		/* note: the other 2 strings, and the source flag are macro only;
			they will be ninched at this point by the command processor */

		/* fMemFail will be true if FSetCabSz (in InitStyleCab...) fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltDefineStyle)];

		/* allocate stsh entry for backup */
		stcp = StcpCreateNewStyle(vdocStsh, fTrue /* fUseReserve */);
		if (stcp == stcpNil)
			{
			Assert(vmerr.fMemFail);
			cmd = cmdNoMemory;
			goto LRet;
			}
		vstcBackup = StcFromStcp(stcp, PdodDoc(vdocStsh)->stsh.cstcStd);
		GenLbStyleMapping();
		BltDlt(dltDefineStyle, dlt);
		vfDefineStyle = fTrue;
		pcmb->pv = &cesty;
		switch (tmc = TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			break;
		case tmcCancel:
			cmd = cmdCancelled;
			break;
			}
		}

LRet:
	if (cesty.fDirty)
		SetUndoNil();
	FreePh(&vhmpiststcp);
	vfDefineStyle = fFalse;
	vfShowAllStd = fFalse;
	vdocStsh = docNil;

	return cmd;
}


/* %%Function:FDlgDefineStyle %%Owner:davidbo */
BOOL FDlgDefineStyle(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	extern EnumFontUpDS(), EnumFontDownDS(),
	EnumPtSizeUpDS(), EnumPtSizeDnDS(),
	EnumColorUpDS(), EnumColorDnDS();

	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;
	int fMatchStd, fMerge, fUseSelProps, fChange, fBasedOn, fNew;
	int stc, stcBase, stcNext, stcMatch;
	int istLb, tmcErr;
	int stcp, stcpMac;
	int cbn, fn, id;
	BOOL fRet;
	CMD cmd;
	struct ESTCP *pestcp;
	struct FNS fns;
	CESTY *pcesty;
	char stName[cchMaxStyle+1];
	char szStyle[cchMaxStyle+1];
	HCAB hcab;

	fRet = fTrue;

	pcesty = PcmbDlgCur()->pv;
	if (pcesty->fIgnore)
		return fTrue;

	SetVdocStsh(selCur.doc);
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	fMerge = fFalse;

	switch (dlm)
		{
	case dlmInit:
		/* Add all the looks keys to the DefineStyle Dlg box. */
		AddLooksKeys((*hkmpCur)->hkmpNext, hkmpCur, ilcdMaxStyle);
		AddKeyPfn(hkmpCur, KcCtrl(vkFont), EnumFontUpDS);
		AddKeyPfn(hkmpCur, KcShift(KcCtrl(vkFont)), EnumFontDownDS);
		AddKeyPfn(hkmpCur, KcCtrl(vkPoint), EnumPtSizeUpDS);
		AddKeyPfn(hkmpCur, KcShift(KcCtrl(vkPoint)), EnumPtSizeDnDS);
		AddKeyPfn(hkmpCur, KcCtrl(vkColor), EnumColorUpDS);
		AddKeyPfn(hkmpCur, KcShift(KcCtrl(vkColor)), EnumColorDnDS);
		AddKeyPfn(hkmpCur, KcCtrl(vkShowStd), ToggleShowStd);

		if (fElActive)
			{
			EnableTmc(tmcDSChars, fFalse);
			EnableTmc(tmcDSParas, fFalse);
			EnableTmc(tmcDSTabs, fFalse);
			EnableTmc(tmcDSPosition, fFalse);
			EnableTmc(tmcDSOptions, fFalse);
			}

		MakeStshNonShrink(vdocStsh);
		GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
		SzToStInPlace(stName);
		if (FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, stName, &stcMatch, &fMatchStd))
			{
			istLb = IstLbFromStcp(StcpFromStc(stcMatch, stsh.cstcStd));
			pcesty->fIgnore = fTrue;
			SetTmcVal((tmcDSStyle & ~ftmcGrouped) + 1, istLb);
			pcesty->fIgnore = fFalse;
			}
		else
			istLb = istLbNil;
		EnableTmc(tmcDSDefine, *stName != 0);
		EnableTmc(tmcDSDelete, !fMatchStd);
		EnableTmc(tmcDSRename, !fMatchStd);
		pcesty->fRecorded = fFalse;
		goto LSelectLb;

	case dlmExit:
		if (vstcBackup != stcNil)
			InvalStyleEntry(&stsh, hsttbChpe, hsttbPape, StcpFromStc(vstcBackup, stsh.cstcStd));
		vstcStyle = vstcBackup = stcNil;
		if (pcesty->fDirty)
			{
			struct DOD *pdod;

			InvalDoc(vdocStsh);
			InvalSelCurProps(fTrue /* fStatLineToo */);
			pdod = PdodDoc(vdocStsh);
			pdod->fStshDirty = fTrue;
			pdod->fFormatted = fTrue;
			}
		CloseUpStsh(vdocStsh);
		break;

	case dlmTerm:
		if (tmc == tmcOK)
			{
			if (!FDefStyleFromDlg(pcesty, fTrue /* fFromOK */,
					&stsh, hsttbChpe, hsttbPape) && !vmerr.fMemFail)
				return fFalse;
			}
		else
			RestoreStcFromBackup();
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (vmerr.fMemFail)
			goto LOomRetRest;
		return fFalse; /* keep idling */

	case dlmChange:
		if (tmc == (tmcDSStyle & ~ftmcGrouped))
			{
			int fEnable;

			GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
			SzToStInPlace(stName);
			fEnable = FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd,
					stName, &stcMatch, &fMatchStd) && !fMatchStd;
			EnableTmc(tmcDSRename, fEnable);
			EnableTmc(tmcDSDelete, fEnable);
			EnableTmc(tmcDSDefine, *stName != 0);
			pcesty->dlmLast = (pcesty->dlmLast == dlmClick || 
					pcesty->istLb == istLbNil) ? 0 : dlmChange;
			pcesty->fRecorded = fFalse;
			}
		break;

	case dlmKillItmFocus:
		fBasedOn = (tmc == (tmcDSBasedOn & ~ftmcGrouped));
		if (fBasedOn || tmc == (tmcDSNext & ~ftmcGrouped))
			{
			GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
			SzToStInPlace(stName);
			if (FVerifyBasedOnNext(&stsh, hsttbChpe, hsttbPape, fFalse,
					fBasedOn, fTrue, stName))
				{
				pcesty->fStyleDirty = pcesty->fDirty = fTrue;
				}
			else if (vmerr.fMemFail)
				goto LOomRetRest;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcDSOptions:
			 /* FSetDlgSab not safe if dialog coming down */
			if (FIsDlgDying() || !FSetDlgSab(sabDSOptions))
				{
				Beep();
				return fFalse;
				}
			SetFocusTmc(tmcDSBasedOn);
			EnableTmc(tmcDSTemplate, pcesty->fHasDot);
			EnableTmc(tmcDSOptions, fFalse);
			return fFalse;

		case (tmcDSStyle & ~ftmcGrouped) + 1:
			fChange = pcesty->dlmLast == dlmChange;
			pcesty->dlmLast = fChange ? 0 : dlmClick;
			istLb = ValGetTmc(tmc);
			if (istLb == pcesty->istLb)
				break;
			pcesty->fRecorded = fFalse;

			if (!pcesty->fStyleDirty)
				goto LSelectLb;

			if (fChange && pcesty->istLb != istLbNil)
				{
				/* name in edit control is not the one we want! */
				stcp = (**vhmpiststcp)[pcesty->istLb];
				GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, stcp);
				}
			else
				{
				GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
				SzToStInPlace(stName);
				}

			id = IdSaveDirtyStyle(stName, &stsh, hsttbChpe, hsttbPape, fFalse);
			if (vmerr.fMemFail)
				goto LOomRetRest;

			if (id == IDYES)
				{
				ChangeCancelToClose();
				stc = istLb == istLbNil ? stcNil : StcFromStcp((**vhmpiststcp)[istLb], stsh.cstcStd);
				if (!FDefineStyle(stName,ValGetTmc(tmcDSTemplate) != 0,
						pcesty, fTrue /* fClick */))
					{
					if (!vmerr.fMemFail)
						{
						SetFocusTmc(tmcDSStyle);
						fRet = fFalse;
						break;
						}
					goto LOomRetRest;
					}
				stsh.cstcStd = PdodDoc(vdocStsh)->stsh.cstcStd;
				if (!FSetStcBackup())
					goto LOomRet;
				GenLbStyleMapping();
				istLb = stc == stcNil ? istLbNil : IstLbFromStcp(StcpFromStc(stc, stsh.cstcStd));
				if (pcesty->istLb == istLbNil || pcesty->fFake)
					{
					Assert(istLb < vistLbMac);
					pcesty->fIgnore = fTrue;
					RedisplayComboBox(tmcDSStyle, istLb);
					RefillDSDropDown();
					pcesty->fIgnore = fFalse;
					}
				}
			else
				{
				/* Note: szStyle used as st here */
				GenStyleNameForStcp(szStyle, stsh.hsttbName, stsh.cstcStd, StcpFromStc(vstcBackup, stsh.cstcStd));
				RestoreStcFromBackup();
				/* if style was renamed and we're restoring, regen combo */
				if (*szStyle && FNeNcSt(stName, szStyle))
					{
					pcesty->fIgnore = fTrue;
					RedisplayComboBox(tmcDSStyle, istLb);
					RefillDSDropDown();
					pcesty->fIgnore = fFalse;
					}
				}

LSelectLb:
			pcesty->fFake = fFalse;
			fUseSelProps = fTrue;
			if (istLb == istLbNil)
				{
				/* if click in blank part of list box, clear edit control */
				if (!fChange)
					{
					stName[0] = 0;
					pcesty->fIgnore = fTrue;
					SetTmcText(tmcDSStyle, stName);
					pcesty->fIgnore = fFalse;
					}
LNewEntry:
				fNew = fTrue;
				if ((stcp = StcpCreateNewStyle(vdocStsh, fFalse /* fUseReserve */)) == stcpNil)
					{
					if (vmerr.fMemFail)
						goto LOomRet;
					ErrorEid(eidStshFull, "");
					istLb = IstLbFromStcp(stsh.cstcStd);
					goto LSelectLb;
					}

				if (fUseSelProps)
					{
					vstcStyle = StcFromStcp(stcp, stsh.cstcStd);
					if (!FUseSelProps(stcp, fFalse))
						{
						MakeStcpEntryNull(stcp, &stsh, hsttbChpe, hsttbPape);
						goto LOomRet;
						}
					}
				else
					{
					/* using standard style not yet in stsh so copy its
						properties to new style */
					struct CHP chp;
					struct PAP pap;

					GetStcBaseNextForStcp(&stsh, StcpFromStc(vstcStyle,
							stsh.cstcStd), &stcBase, &stcNext);
					SetStcBaseNextForStcp(&stsh, stcp, stcBase, stcNext);
					MapStc(PdodDoc(vdocStsh), vstcStyle, &chp, &pap);
					vstcStyle = StcFromStcp(stcp, stsh.cstcStd);
					if (!FStorePropeForStcp((char*)&chp,stcp,hsttbChpe,fTrue) ||
							!FStorePropeForStcp((char*)&pap,stcp,hsttbPape,fFalse)||
							!FGenChpxPapxNewBase(vdocStsh, stcp))
						{
						MakeStcpEntryNull(stcp, &stsh, hsttbChpe, hsttbPape);
						goto LOomRet;
						}
					pcesty->fFake = fTrue;
					}

				EnableTmc(tmcDSDelete, fFalse);
				EnableTmc(tmcDSRename, fFalse);
				}
			else
				{
				int fEnable;

				fNew = fFalse;
				Assert(istLb >= 0 && istLb < vistLbMac);
				stcp = (**vhmpiststcp)[istLb];
				vstcStyle = StcFromStcp(stcp, stsh.cstcStd);

				/* when showing all std, std style might 
					not have entry in stsh.  Create scratchpad
					entry and let Define worry about moving 
					changes to appropriate location in stsh. */
				if (vstcStyle > stcStdMin && vstcStyle < cstcMax &&
						(((cstcMax - vstcStyle) > stsh.cstcStd) ||
						FStcpEntryIsNull(stsh.hsttbName, StcpFromStc(vstcStyle, stsh.cstcStd))))
					{
					fUseSelProps = fFalse;
					goto LNewEntry;
					}

				fEnable = vstcStyle > 0 && vstcStyle < stcStdMin;
				EnableTmc(tmcDSDelete, fEnable);
				EnableTmc(tmcDSRename, fEnable);
				}

			if (!FSetStcBackup())
				{
				if (fNew)
					MakeStcpEntryNull(stcp, &stsh, hsttbChpe, hsttbPape);
				goto LOomRet;
				}

			pcesty->istLb = istLb;
			pcesty->fStyleDirty = fFalse;

			/*
			* Stuff BasedOn and Next style names in the 
			* appropriate combo box.
			*/
			GetStcBaseNextForStcp(&stsh, stcp, &stcBase, &stcNext);
			GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, StcpFromStc(stcBase, stsh.cstcStd));
			StToSzInPlace(stName);
			SetTmcText(tmcDSBasedOn, stName);

			GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, StcpFromStc(stcNext, stsh.cstcStd));
			StToSzInPlace(stName);
			SetTmcText(tmcDSNext, stName);

			SetIbstFontDSFromStc(vstcStyle);
			BanterToTmc(tmcDSBanter, vstcStyle, &stsh, hsttbChpe, hsttbPape);
			break;


		case (tmcDSBasedOn & ~ftmcGrouped) + 1:
			if ((istLb = ValGetTmc(tmc)) == istLbNil)
				break;

			fBasedOn = fTrue;
			tmc = tmcDSBasedOn;
			goto LBasedOnNext;

		case (tmcDSNext & ~ftmcGrouped) + 1:
			if ((istLb = ValGetTmc(tmc)) == istLbNil)
				break;

			fBasedOn = fFalse;
			tmc = tmcDSNext;

LBasedOnNext:
			GetListBoxEntry(tmc, istLb, stName, cchMaxStyle+1);
			SzToStInPlace(stName);
			GetTmcText(tmcDSStyle, szStyle, cchMaxStyle + 1);
			SzToStInPlace(szStyle);

			/* szStyle filled with an st here! */
			if (!FChangeBN(szStyle, stName, &stsh, hsttbChpe, hsttbPape,
					pcesty, fBasedOn, fTrue /* fClick */, fTrue /* fDlg */))
				{
				if (vmerr.fMemFail)
					goto LOomRetRest;
				break;
				}

			GrayButtonOnBlank(tmcDSStyle, tmcDSDefine);
			pcesty->fStyleDirty = pcesty->fDirty = fTrue;
			break;

		case tmcDSDelete:
			fRet = fFalse; /* Do not record! */
			GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
			SzToStInPlace(stName);
			FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, stName, &stc, &fMatchStd);
			Assert(!fMatchStd);
			istLb = IstLbFromStcp(StcpFromStc(stc, stsh.cstcStd));
			Assert(istLb >= 0 && istLb < vistLbMac); /*If not, Delete illegal!*/

			cmd = CmdDeleteStyle((**vhmpiststcp)[istLb], fTrue);
			if (cmd == cmdCancelled)
				break;
			else  if (cmd == cmdNoMemory)
				goto LOomRetRest;

			/* Set up for defining new style */
			stcp = StcpCreateNewStyle(vdocStsh, fFalse /* fUseReserve */);
			if (stcp == stcpNil)
				goto LOomRet;
			vstcStyle = StcFromStcp(stcp, stsh.cstcStd);
			if (!FUseSelProps(stcp, fFalse) || !FSetStcBackup())
					
				{
				MakeStcpEntryNull(stcp, &stsh, hsttbChpe, hsttbPape);
				goto LOomRet;
				}

			/* record now, before we trash the cab with RedisplayComboBox */
			if (vfRecording)
				{
				RecordStyTmc(tmcDSDelete);
				pcesty->fRecorded = fFalse;
				}

			pcesty->istLb = istLbNil;
			/* "fix" vhmpiststcp...there's one less entry! */
			if (istLb+1 < vistLbMac)
				{
				bltbyte(&(**vhmpiststcp)[istLb+1],
						&(**vhmpiststcp)[istLb], vistLbMac - istLb - 1);
				}
			vistLbMac -= 1;
			SetVstcSel();
			SetFocusTmc(tmcDSStyle);
			EnableTmc(tmcDSDelete, fFalse);
			EnableTmc(tmcDSRename, fFalse);
			EnableTmc(tmcDSDefine, fFalse);
			RedisplayComboBox(tmcDSStyle, istLbNil);

			/* Reset names in BasedOn and Next */
			GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, 
					StcpFromStc(stcNormal, stsh.cstcStd));
			StToSzInPlace(stName);
			SetTmcText(tmcDSBasedOn, stName);
			pcesty->dlmLast = dlmClick;
			stName[0] = 0;
			SetTmcText(tmcDSNext, stName);

			/* force refill of combo boxes on next drop-down */
			RefillDSDropDown();
			pcesty->fDirty = fTrue;

			/* update banter */
			SetIbstFontDSFromStc(vstcStyle);
			BanterToTmc(tmcDSBanter, vstcStyle, &stsh, hsttbChpe, hsttbPape);

			ChangeCancelToClose();
			break;

		case tmcDSDefine:
			SetFocusTmc(tmcDSStyle);
			if (FDefStyleFromDlg(pcesty, fFalse /* fFromOK */, &stsh, hsttbChpe,hsttbPape))
				{
				istLb = istLbNil;
				pcesty->dlmLast = dlmClick;
				goto LSelectLb;
				}
			else  if (vmerr.fMemFail)
				goto LOomRet;
			break;

		case tmcDSParas:
			if (TmcGosubParagraph() == tmcParTabs)
				TmcGosubTabs();
			goto LButtonFocus;

		case tmcDSChars:
			SetFocusTmc(tmcDSStyle);
			EnsureFtcDefined(vstcStyle);
			TmcGosubCharacter();
			SetIbstFontDSFromStc(vstcStyle);
			goto LButtonFocus;

		case tmcDSPosition:
			TmcGosubPosition();
			goto LButtonFocus;

		case tmcDSTabs:
			TmcGosubTabs();

LButtonFocus:
			fRet = fFalse; /* Do not record! */
			/* ApplyGrpprlToStshPrope may go OOM! */
			if (vmerr.fMemFail)
				goto LOomRetRest;
			SetFocusTmc(tmcDSStyle);
			pcesty->fStyleDirty = fTrue;
			break;

#ifdef DAVIDBO
			SetVdocStsh(selCur.doc);
			RecordStshForDoc(vdocStsh, &stsh, 
					&hsttbChpe, &hsttbPape);
			DumpStsh(&stsh, hsttbChpe, hsttbPape);
			goto LButtonFocus;
#endif

#ifdef DAVIDBO
			RestoreStcFromBackup();
			ShowMsg("FS", 0, NULL, 0, 0);
			for (stc = stcStdMin+1; stc < cstcMax; stc++)
				FEnsureStcDefined(vdocStsh, stc, &stsh.cstcStd);
			for (stc = 1; stc < 100; stc++)
				{
				int cch;

				stcp = StcpCreateNewStyle(vdocStsh, fFalse /* fUseReserve */);
				ShowMsg("FS", stc, NULL, stcp, 0);
				if (stcp == stcpNil)
					break;
				cch = CchLongToRgchNfc((LONG)stc, stName+1, nfcLCLetter, 10);
				stName[0] = cch;
				AssertDo(FChangeStInSttb(stsh.hsttbName, stcp, stName));
				}
			vstcStyle = stcNormal;
			GenLbStyleMapping();
			FSetStcBackup();
			pcesty->fIgnore = fTrue;
			RedisplayComboBox(tmcDSStyle, IstLbFromStcp(PdodDoc(vdocStsh)->stsh.cstcStd));
			RefillDSDropDown();
			pcesty->fIgnore = fTrue;
			goto LButtonFocus;
#endif

		case tmcDSRename:
				{
				int fNew = fFalse;

				fRet = fFalse; /* Do not record! */
				GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
				SzToStInPlace(stName);
				FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, stName,
						&stc, &fMatchStd);
			/*
			*  HACK...if "new" style (vstcStyle...typed into edit control
			*  rather than selected via the list box) matches a pre-existing
			*  style (stc), Rename is enabled.  Do a little sleight of hand
			*  to make like the new style is really the pre-existing style.  
			*  Unfortunately, this nukes any properties the user might have
			*  applied to the "new" style.  User is hosed if he cancels
			*  out of Rename dlg since we've changed the the state of the
			*  stsh.
			*/
				if (stc != vstcStyle)
					{
					InvalStyleEntry(&stsh, hsttbChpe, hsttbPape,
							StcpFromStc(vstcStyle, stsh.cstcStd));
					vstcStyle = stc;
					if (!FSetStcBackup())
						goto LOomRet;
					GetStcBaseNextForStcp(&stsh, StcpFromStc(stc, stsh.cstcStd),
							&stcBase, &stcNext);
					GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd,
							StcpFromStc(stcBase, stsh.cstcStd));
					StToSzInPlace(stName);
					pcesty->fIgnore = fTrue;
					SetTmcText(tmcDSBasedOn, stName);
					pcesty->fIgnore = fFalse;
					fNew = fTrue;
					}
				Assert(!fMatchStd);
				Assert(stc != stcNil);
			/* Remember stc of current selection */
				istLb = IstLbFromStcp(stcp = StcpFromStc(stc, stsh.cstcStd));
				Assert(istLb >= 0 && istLb < vistLbMac); /*If not, Rename illegal!*/
				switch (CmdGosubRename())
					{
				case cmdOK:
				/* only needed by record, but avoid actions if hcab... fails bz */

					if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
						return fFalse;
					if (hcab == hcabNull)
						goto LOomRetRest;

					ChangeCancelToClose();
					GetStcBaseNextForStcp(&stsh, stcp, &stcBase, &stcNext);
					if (stcNext == stc)
						{
						GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, stcp);
						StToSzInPlace(stName);
						pcesty->fIgnore = fTrue;
						SetTmcText(tmcDSNext, stName);
						pcesty->fIgnore = fFalse;
						}
					if (vfRecording)
						{
						if ((hcab = HcabFromDlg(fFalse)) == hcabNull)
							goto LOomRetRest;
						Assert(hcab != hcabNotFilled);

			/* Make sure stName contains *new* name (in st format) */
						GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, stcp);
						if (!FSetCabSt(PcmbDlgCur()->hcab, stName, 
								Iag(CABDEFINESTYLE, hszDSNewName)))
							{
							vfRecorderOOM = fTrue;
							goto LOomRetRest;
							}
						FRecordCab(PcmbDlgCur()->hcab, IDDDefineStyle, 
								tmcDSRename, fFalse);
						pcesty->fRecorded = fFalse;
						}
					pcesty->fDirty = fTrue;
				/*
				*  See HACK comment above.  In this case, the banter
				*  needs to be changed from the "new" style to the
				*  pre-existing one.
				*/
					if (fNew)
						{
						BanterToTmc(tmcDSBanter, vstcStyle, &stsh,
								hsttbChpe, hsttbPape);
						}
					goto LMerRenCommon;
				case cmdNoMemory:
					goto LOomRetRest;
					}
				break;
				}

		case tmcDSMerge:
			fRet = fFalse; /* Do not record! */
			GetTmcText(tmcDSStyle, stName, cchMaxStyle + 1);
			SzToStInPlace(stName);

			id = IdSaveDirtyStyle(stName, &stsh, hsttbChpe, hsttbPape, fTrue);
			if (vmerr.fMemFail)
				goto LOomRetRest;

			if (id == IDYES)
				{
				if (!FDefineStyle(stName, ValGetTmc(tmcDSTemplate)!=0,
						pcesty, fFalse /* fClick */))
					{
					if (!vmerr.fMemFail)
						{
						SetFocusTmc(tmcDSStyle);
						fRet = fFalse;
						break;
						}
					goto LOomRetRest;
					}
				if (!FSetStcBackup())
					goto LOomRet;
				ChangeCancelToClose();
				}
			else
				{
				/* Note: szStyle used as st here */
				GenStyleNameForStcp(szStyle, stsh.hsttbName, stsh.cstcStd, StcpFromStc(vstcBackup, stsh.cstcStd));
				RestoreStcFromBackup();
				/* if style was renamed and we're restoring, regen combo */
				if (*szStyle && FNeNcSt(stName, szStyle))
					{
					pcesty->fIgnore = fTrue;
					RedisplayComboBox(tmcDSStyle, ValGetTmc((tmcDSStyle & ~ftmcGrouped) + 1));
					RefillDSDropDown();
					pcesty->fIgnore = fFalse;
					}
				}

			/*
			*  Pointless to remember current selection because of
			*  possible style code permutations by Merge code.
			*/
			istLb = istLbNil;
			pcesty->istLb = istLbNil;
			cmd = CmdGosubMerge();
			/* Reset these since Merge whomps all over them... */
			SetVdocStsh(selCur.doc);
			RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
			/* allocate stsh entry for backup if destroyed in merge */
			if (vstcBackup == stcNil)
				{
				stcp = StcpCreateNewStyle(vdocStsh, fTrue /* fUseReserve */);
				if (stcp == stcpNil)
					goto LOomRet;
				vstcBackup = StcFromStcp(stcp, PdodDoc(vdocStsh)->stsh.cstcStd);
				}

			if (cmd != cmdOK)
				{
				SetFocusTmc(tmcDSStyle);
				break;
				}

			ChangeCancelToClose();
			fMerge = fTrue;

LMerRenCommon:
			pcesty->dlmLast = dlmClick;
			SetFocusTmc(tmcDSStyle);
			GenLbStyleMapping();
			RecordStshForDocNoExcp(vdocStsh, &stsh);
			if (istLb != istLbNil)
				{
				stcp = StcpFromStc(stc, stsh.cstcStd);
				istLb = IstLbFromStcp(stcp);
				}
			Assert(istLb < vistLbMac);
			pcesty->fIgnore = fTrue;
			RedisplayComboBox(tmcDSStyle, istLb);
			RefillDSDropDown();
			pcesty->fIgnore = fFalse;

			if (fMerge)
				goto LSelectLb;
			break;
			}
		break;
		}

	return fRet;

LOomRetRest:
	RestoreStcFromBackup();
LOomRet:
	Assert (vmerr.fMemFail);  /* this will assure dialog coming down */
	return fTrue;
}


/*
*  Make sure the BasedOn and Next combos contain valid style names.
*  If so, return tmcNull.  If not return the tmc of the first bogus
*  combo.  If fReport is fTrue, errors generate a message box, else
*  just return the offending tmc.
*/
/* %%Function:TmcVerifyBasedOnNext %%Owner:davidbo */
TmcVerifyBasedOnNext(pstsh, hsttbChpe, hsttbPape, fReport, fUpd, stStyle)
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
int fReport, fUpd;
CHAR stStyle[];
{
	int tmc = tmcNull;

	if (!FVerifyBasedOnNext(pstsh, hsttbChpe, hsttbPape, fReport,
			fTrue, fUpd, stStyle))
		{
		tmc = tmcDSBasedOn;
		}
	else  if (!FVerifyBasedOnNext(pstsh, hsttbChpe, hsttbPape, fReport,
			fFalse, fUpd, stStyle))
		{
		tmc = tmcDSNext;
		}

	if (vmerr.fMemFail)
		tmc = tmcError;

	return tmc;
}


/*
*  Check validity of name in BasedOn or Next edit control.  If legal
*  name, make the change and return true.  Return false on OOM or
*  on illegal names.  If fReport, bring up message box on bogus name.
*  If fUpd, update the banter.
*/
/* %%Function:FVerifyBasedOnNext %%Owner:davidbo */
FVerifyBasedOnNext(pstsh, hsttbChpe, hsttbPape, fReport, fBasedOn, fUpd,stStyle)
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
int fReport, fBasedOn, fUpd;
CHAR stStyle[];
{
	int tmc, stc, stcp, stcMatch, stcBase, stcNext, fMatchStd, fChange;
	struct ESTCP *pestcp;
	CESTY *pcesty;
	char stName[cchMaxStyle+1];

	tmc = fBasedOn ? tmcDSBasedOn : tmcDSNext;

	/* check if valid name in BasedOn/Next combo */
	GetTmcText(tmc, stName, cchMaxStyle+1);
	SzToStInPlace(stName);
	/* Null name legal in BasedOn/Next combos */
	if (stName[0] != 0 && !FValidStyleName(stName))
		goto LInvalidStyle;

	/* if BasedOn, ensure style isn't based on itself */
	if (fBasedOn && FEqNcSt(stName, stStyle))
		{
		if (fReport)
			ErrorEid(eidStupidBasedOn, "");
		return fFalse;
		}

	/* No name in edit control...use defaults */
	if (!stName[0])
		{
		if (fBasedOn)
			stcMatch = stcStdMin;
		else
			stcMatch = vstcStyle;
		}
	else  if (!FMatchDefinedAndStdStyles(pstsh->hsttbName, pstsh->cstcStd,
			stName, &stcMatch, &fMatchStd))
		{
LInvalidStyle:
		if (fReport)
			ErrorEid(fBasedOn ? eidInvalidBasedOn : eidInvalidNext,"");
		return fFalse;
		}

	/* if stcMatch matches what's already there, no need to change */
	stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
	GetStcBaseNextForStcp(pstsh, stcp, &stcBase, &stcNext);
	if (stcMatch == (fBasedOn ? stcBase : stcNext))
		return fTrue;

	/* A previously unused standard style name has been entered */
	if (stcMatch > stcStdMin && (((cstcMax - stcMatch) > pstsh->cstcStd) ||
			FStcpEntryIsNull(pstsh->hsttbName,
				StcpFromStc(stcMatch, pstsh->cstcStd))))
		{
		if (!FEnsureStcDefined(vdocStsh, stcMatch, &pstsh->cstcStd))
			return fFalse;
		/* FEnsure... perturbs cstcStd... */
		stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
		GenLbStyleMapping();

		pcesty = PcmbDlgCur()->pv;
		UpdateCombos(pstsh, pcesty, stStyle);
		}

	/* Now, make the change... */
	pestcp = PInPl(pstsh->hplestcp, stcp);
	if (!fBasedOn)
		pestcp->stcNext = stcMatch;
	else
		{
		if (!FChangeBasedOn(stcMatch, &fChange, pstsh, pestcp, hsttbChpe, hsttbPape, stName, stStyle))
			{
			RestoreBasedOnEditControl(pstsh, pestcp->stcBase);
			return fFalse;
			}
		SetIbstFontDSFromStc(vstcStyle);
		if (fUpd && fChange)
			BanterToTmc(tmcDSBanter,vstcStyle,pstsh,hsttbChpe,hsttbPape);
		}
	return fTrue;
}


/* %%Function:RestoreBasedOnEditControl %%Owner:davidbo */
RestoreBasedOnEditControl(pstsh, stcBase)
struct STSH *pstsh;
int stcBase;
{
	CESTY *pcesty = PcmbDlgCur()->pv;
	char sz[cchMaxStyle];

	GenStyleNameForStcp(sz, pstsh->hsttbName, pstsh->cstcStd, StcpFromStc(stcBase, pstsh->cstcStd));
	StToSzInPlace(sz);
	pcesty->fIgnore = fTrue;
	SetTmcText(tmcDSBasedOn, sz);
	pcesty->fIgnore = fFalse;
}


/* %%Function:FChangeBasedOn %%Owner:davidbo */
FChangeBasedOn(stcNew, pfChange, pstsh, pestcp, hsttbChpe, hsttbPape, stName, stStyle)
int stcNew, *pfChange;
struct STSH *pstsh;
struct ESTCP *pestcp;
struct STTB **hsttbChpe, **hsttbPape;
char stName[], stStyle[];
{
	int cstc, eid, cstcBasedOn;
	int stcBase, stcNext, stcp;
	int stcBaseSave = pestcp->stcBase;
	CESTY *pcesty;
	struct CHP chp;
	struct PAP pap;

	*pfChange = fFalse;
	/* Style can't be based on itself! */
	if (FEqNcSt(stName, stStyle))
		{
		eid = eidStupidBasedOn;
		goto LFixBasedOn;
		}

	if (stcNew != pestcp->stcBase)
		{
		pestcp->stcBase = stcNew;
		if (!FCheckBasedOn(pstsh, &eid))
			{
LFixBasedOn:
			pestcp->stcBase = stcBaseSave;
			goto LErrRet;
			}

		stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
		RetrievePropeForStcp(&chp, stcp, hsttbChpe, fTrue);
		RetrievePropeForStcp(&pap, stcp, hsttbPape, fFalse);
		if (!FGenChpxPapxChangedBasedOn(stcBaseSave,stcNew,pstsh,hsttbChpe,hsttbPape))
			{
			pestcp = PInPl(pstsh->hplestcp, stcp);
			pestcp->stcBase = stcBaseSave;
			RestoreStcpAndDependentsDefn(vdocStsh, stcp, &chp, &pap);
			return fFalse;
			}
		*pfChange = fTrue;
		}
	return fTrue;

LErrRet:
	ErrorEid(eid, "");
	return fFalse;
}


/* %%Function:FGenChpxPapxChangedBasedOn %%Owner:davidbo */
FGenChpxPapxChangedBasedOn(stcFromBase,stcToBase, pstsh, hsttbChpe, hsttbPape)
int stcFromBase, stcToBase;
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
{
	int stcp, stcpFromBase, stcpToBase, cb;
	struct STSH stsh;
	struct CHP chp, chpBase;
	struct PAP pap, papBase;
	char grpprlDelta[cchMaxSz];

	SetVdocStsh(selCur.doc);
	stsh = *pstsh;
	stcp = StcpFromStc(vstcStyle, stsh.cstcStd);
	stcpFromBase = StcpFromStc(stcFromBase, stsh.cstcStd);
	stcpToBase = StcpFromStc(stcToBase, stsh.cstcStd);

	/* if style is standard, both hsttbChpe and hsttbPape must be filled
		in, in parallel in order to satisfy MapStc. */
	if (stcStdMin < vstcStyle && vstcStyle < cstcMax)
		{
		if (FStcpEntryIsNull(hsttbPape, stcp))
			{
			char stNull[1];
			MapStc(PdodDoc(vdocStsh), 0, &chp, &pap);
			MapStcStandard(vstcStyle, &chp, &pap);
			if (!FStorePropeForStcp(&chp, stcp, hsttbChpe, fTrue))
				return fFalse;
			if (!FStorePropeForStcp(&pap, stcp, hsttbPape, fFalse))
				{
				char st[1];
				st[0] = 255;
				FChangeStInSttb(hsttbChpe, stcp, st);
				return fFalse;
				}
			}
		}

	MapStc(PdodDoc(vdocStsh), vstcStyle, &chp, &pap);
	MapStc(PdodDoc(vdocStsh), stcFromBase, &chpBase, &papBase);

	/* First deal with PAP stuff */
	cb = CbGrpprlFromPap(fFalse, grpprlDelta, &pap, &papBase, fFalse);
	if (!FApplyGrpprlForNewBasedOn(stcToBase, grpprlDelta, cb, fFalse))
		return fFalse;

	/* ...now deal with CHP stuff */
	cb = CbGrpprlFromChp(grpprlDelta, &chp, &chpBase);
	return FApplyGrpprlForNewBasedOn(stcToBase, grpprlDelta, cb, fTrue);
}


/* %%Function:FApplyGrpprlForNewBasedOn %%Owner:davidbo */
FApplyGrpprlForNewBasedOn(stcBaseNew, grpprl, cb, fChp)
int stcBaseNew;
char *grpprl;
int cb, fChp;
{
	int stcp;
	struct STSH stsh;
	struct STTB **hsttbPape, **hsttbChpe;
	struct CHP chp;
	struct PAP pap;
	char *pprope;

	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	stcp = StcpFromStc(vstcStyle, stsh.cstcStd);

	MapStc(PdodDoc(vdocStsh), stcBaseNew, &chp, &pap);
	pprope = fChp ? &chp : &pap;

	if (!FStorePropeForStcp(pprope, stcp, fChp ? hsttbChpe : hsttbPape, fChp))
		return fFalse;
	ApplyGrpprlToStshPrope(grpprl, cb, fChp, fFalse /* fUpdBanter */);
	return (!vmerr.fMemFail);
}


/*
* changes stStyle's BasedOn or Next to stName.  Returns false if it
* fails (illegal name, causes invalid based-on chain, OOM)
*/
/* %%Function:FChangeBN %%Owner:davidbo */
FChangeBN(stStyle, stName, pstsh, hsttbChpe, hsttbPape, pcesty, fBasedOn, fClick, fDlg)
char stStyle[], stName[];
struct STSH *pstsh;
CESTY *pcesty;
int fBasedOn, fClick, fDlg;
{
	int stcMatch, stcp, stcBase, stcNext, fMatchStd, fChange, istLb;
	struct ESTCP *pestcp;

	stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
	/* Null name...use defaults */
	if (!stName[0])
		{
		if (fBasedOn)
			stcMatch = stcStdMin;
		else
			stcMatch = vstcStyle;
		}
	else  if (!FMatchDefinedAndStdStyles(pstsh->hsttbName, pstsh->cstcStd, stName, &stcMatch, &fMatchStd))
		{
		GetStcBaseNextForStcp(pstsh, stcp, &stcBase, &stcNext);
		stcMatch = fBasedOn ? stcBase : stcNext;
		}

	/* A previously unused standard style name has been entered */
	if (stcMatch > stcStdMin && (((cstcMax - stcMatch) > pstsh->cstcStd) ||
			FStcpEntryIsNull(pstsh->hsttbName,
				StcpFromStc(stcMatch, pstsh->cstcStd))))
		{
		if (!FEnsureStcDefined(vdocStsh, stcMatch, &pstsh->cstcStd))
			return fFalse;
		/* FEnsure... perturbs cstcStc */
		stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
		GenLbStyleMapping();

		/* if fClick, style already exists in list box so no need to regen
			the contents of the list boxes */
		if (fDlg && !fClick)
			UpdateCombos(pstsh, pcesty, stStyle);
		}

	/* Now, make the change... */
	pestcp = PInPl(pstsh->hplestcp, stcp);
	if (!fBasedOn)
		pestcp->stcNext = stcMatch;
	else  /* tmc == tmcDSBasedOn */		
		{
		if (!FChangeBasedOn(stcMatch, &fChange, pstsh, pestcp, hsttbChpe, hsttbPape, stName, stStyle))
			{
			if (fDlg)
				{
				RestoreBasedOnEditControl(pstsh, pestcp->stcBase);
				SetFocusTmc(tmcDSBasedOn);
				}
			return fFalse;
			}

		SetIbstFontDSFromStc(vstcStyle);
		if (fDlg && fChange)
			BanterToTmc(tmcDSBanter, vstcStyle, pstsh, hsttbChpe, hsttbPape);
		}
	return fTrue;
}


/* %%Function:UpdateCombos %%Owner:davidbo */
UpdateCombos(pstsh, pcesty, stStyle)
struct STSH *pstsh;
CESTY *pcesty;
char stStyle[];
{
	int stcp, istLb;

	Assert(pcesty);
	pcesty->fIgnore = fTrue;
	stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
	istLb = IstLbFromStcp(stcp);
	Assert(istLb < vistLbMac);
	RedisplayComboBox(tmcDSStyle, istLb);
	if (istLb == istLbNil)
		{
		StToSzInPlace(stStyle);
		SetTmcText(tmcDSStyle, stStyle);
		SzToStInPlace(stStyle);
		}
	RefillDSDropDown();
	pcesty->fIgnore = fFalse;
}

/* %%Function:FDefineStyle %%Owner:davidbo */
FDefineStyle(stName, fDot, pcesty, fClick)
char *stName;
int fDot;
CESTY *pcesty;
{
	int istLb;
	int stc, stcFrom, stcBase, stcNext;
	int fMatchStd, fInval, fErrRet;
	int id, idError, eid;
	MST mst;
	struct STTB **hsttbChpe, **hsttbPape;
	struct STSH stsh;
	struct CHP chp;
	struct PAP papBefore, papAfter;

	stcFrom = stcNil;
	fErrRet = fFalse;
	MapStc(PdodDoc(vdocStsh), vstcStyle, &chp, &papBefore);
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	if (FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, stName, &stc, &fMatchStd))
		{
		if (stc != vstcStyle)
			{
			mst = fMatchStd ? mstNukeStdStcProps : mstNukeMatchingStyle;

			if (FStylesEqual(stc, &stsh, hsttbChpe, hsttbPape))
				id = IDNO;
			else
				id = IdMessageBoxMstRgwMb(mst, NULL, MB_DEFYESQUESTION);

			/* if using previously undefined std, make sure it's in stsh */
			if (fMatchStd && id != IDCANCEL && !FEnsureStcDefined(vdocStsh, stc, &stsh.cstcStd))
				goto LErrRet;

			switch (id)
				{
			case IDYES:
				stcFrom = vstcStyle;
				vstcStyle = stc;
				/* save matching style before attempting to copy over it */
/* FUTURE: This is really rude...on a failure, we nuke Joe User's changes.
Close to shipping and don't want to make the changes necessary at higher
levels */
				if (!FSetStcBackup())
					{
					fInval = fTrue;
					fErrRet = fFalse;
					vstcStyle = stcFrom;
					goto LErrRet;
					}
				if (!FCopyStyleWithinStsh(vdocStsh, stcFrom, stc))
					{
					fInval = fTrue;
					fErrRet = fFalse;
					goto LErrRet;
					}
				if (!FCheckBasedOn(&stsh, &eid))
					{
					ErrorEid(eid, "");
					fInval = fTrue;
					fErrRet = fClick;
					goto LErrRet;
					}

				GetStcBaseNextForStcp(&stsh, StcpFromStc(stcFrom,stsh.cstcStd),
						&stcBase, &stcNext);
				if (stcNext == stcFrom)
					SetStcBaseNextForStcp(&stsh, StcpFromStc(stc, stsh.cstcStd), stcBase, stc);
				/* set back so following InvalStyleEntry kills correct entry! */
				vstcStyle = stcFrom;
				/* fall through */
			case IDNO:
LInval:
				InvalStyleEntry(&stsh, hsttbChpe, hsttbPape, StcpFromStc(vstcStyle, stsh.cstcStd));
				vstcStyle = stc;

				if (!FChangeStInSttb(stsh.hsttbName, StcpFromStc(vstcStyle,
						stsh.cstcStd), fMatchStd ? szEmpty : stName))
					goto LErrRet;
				break;
			case IDCANCEL:
				return fFalse;
				}
			}
		}
	else
		{
		/* defining new style */
		Assert(*stName);
		if (!FChangeStInSttb(stsh.hsttbName, 
				StcpFromStc(vstcStyle, stsh.cstcStd), stName))
			goto LErrRet;
		}

	EnsureFtcDefined(vstcStyle);
	SetIbstFontDSFromStc(vstcStyle);
	pcesty->fDirty = fTrue;

	MapStc(PdodDoc(vdocStsh), vstcStyle, &chp, &papAfter);
	if (FDestroyParaHeight(&papBefore, &papAfter))
		InvalPageView(vdocStsh);

	if (fDot && !PdodDoc(vdocStsh)->fDot)
		{
		if (!FDefineInTemplate(&stsh, hsttbChpe, hsttbPape, &idError) && vmerr.fMemFail)
			{
			goto LErrRet;
			}
		}
	/* blow caches so UseSelProps picks up the new properties! */
	vdocFetch = caPara.doc = docNil;
	vstcLast = stcNil;
LRet:
	return fTrue;

LErrRet:
	if (fInval)
		{
		Assert(stcFrom != stcNil);
		InvalStyleEntry(&stsh, hsttbChpe, hsttbPape,
			StcpFromStc(stcFrom, stsh.cstcStd));
		}
	RestoreStcFromBackup();
	return fErrRet;
}


/* %%Function:FStylesEqual %%Owner:davidbo */
FStylesEqual(stc2, pstsh, hsttbChpe, hsttbPape)
int stc2;
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
{
	int stc1, stcp1, stcp2, stcBase1, stcBase2, stcNext1, stcNext2;
	struct CHP chp1, chp2;
	struct PAP pap1, pap2;

	stc1 = vstcStyle;
	Assert(stc1 != stcNil);
	Assert(stc2 != stcNil);
	stcp1 = StcpFromStc(stc1, pstsh->cstcStd);
	stcp2 = StcpFromStc(stc2, pstsh->cstcStd);

#ifdef BOGUS
	/* If both Pape's are 255, both are stds and haven't changed.  Can't
		just return true since it's possible to change the Next style without
		affecting the Pape! */
	if (FStcpEntryIsNull(hsttbPape,stcp1) && FStcpEntryIsNull(hsttbPape,stcp2))
		goto LSkipChpPapTest;
#endif

	MapStc(PdodDoc(vdocStsh), stc1, &chp1, &pap1);
	MapStc(PdodDoc(vdocStsh), stc2, &chp2, &pap2);
	if (FNeRgch(&chp1, &chp2, cbCHP) || ibstFontDS != IbstFontFromFtcDoc(chp2.ftc, vdocStsh))
		goto LNotEq;
	pap2.stc = pap1.stc;  /* remove bogus difference! */
	if (FNeRgw(&pap1, &pap2, cwPAPBase) ||
			FNeRgw(pap1.rgdxaTab, pap2.rgdxaTab, pap1.itbdMac) ||
			FNeRgch(pap1.rgtbd, pap2.rgtbd, pap1.itbdMac))
		goto LNotEq;

LSkipChpPapTest:
	GetStcBaseNextForStcp(pstsh, stcp1, &stcBase1, &stcNext1);
	GetStcBaseNextForStcp(pstsh, stcp2, &stcBase2, &stcNext2);
	if (stcBase1 != stcBase2)
		goto LNotEq;
	if (stcNext1 != stcNext2)
		goto LNotEq;
	return fTrue;

LNotEq:
	return fFalse;
}


/* %%Function:IdSaveDirtyStyle %%Owner:davidbo */
IdSaveDirtyStyle(stStyle, pstsh, hsttbChpe, hsttbPape, fUpd)
char stStyle[];
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
int fUpd;
{
	char *pch = stStyle;

	if (FValidStyleName(stStyle) &&
			(TmcVerifyBasedOnNext(pstsh, hsttbChpe, hsttbPape, fFalse,
				fUpd, stStyle) == tmcNull) &&
			!FStylesEqual(vstcBackup, pstsh, hsttbChpe, hsttbPape))
		{
		return(IdMessageBoxMstRgwMb(mstSaveStyleChange,&pch,MB_YESNOQUESTION));
		}
	return IDNO;
}


/* copies vstcStyle to current doc's template (or at least tries to) */
/* %%Function:FDefineInTemplate %%Owner:davidbo */
FDefineInTemplate(pstsh, hsttbChpe, hsttbPape, pidError)
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
int *pidError;
{
	int docDot, istcCopyMac;
	struct STTB **hsttbChpeDot, **hsttbPapeDot;
	char rgstcCopy[1];
	struct STSH stshDot;
	char rgb[cstcMax + 2];

	docDot = PdodDoc(vdocStsh)->docDot;
	if (docDot == docNil)
		docDot = docGlobalDot;
	Assert(docDot != docNil);
	RecordStshForDoc(docDot,&stshDot,&hsttbChpeDot,&hsttbPapeDot);
	rgstcCopy[0] = vstcStyle;
	istcCopyMac = 1;
	return (FCopyStyleToDestStsh(vdocStsh, pstsh, hsttbChpe, hsttbPape,
			docDot, &stshDot, hsttbChpeDot, hsttbPapeDot, rgstcCopy,
			istcCopyMac, rgb, fTrue, fFalse, fFalse, pidError));
}


/* %%Function:CmdDeleteStyle %%Owner:davidbo */
CMD CmdDeleteStyle(stcp, fMsg)
int stcp, fMsg;
{
	int stcCut, stcMapTo, fMatchStd;
	char *pch;
	struct STTB **hsttbChpe, **hsttbPape;
	struct PLESTCP **hplestcpBackup;
	struct STSH stsh;
	struct CHP chp;
	struct PAP pap;
	char st[cchMaxStyle];

	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	stcCut = StcFromStcp(stcp, stsh.cstcStd);
	/* Not allowed to delete std styles...can only get here from Macro. */
	if (stcCut == 0 || stcCut > stcStdMin)
		{
		Assert(fElActive);
		ModeError();
		return cmdError;
		}
	Assert(stcCut > 0 && stcCut < stcStdMin);

	GenStyleNameForStcp(st, stsh.hsttbName, stsh.cstcStd, stcp);
	pch = st;
	if (fMsg && IdMessageBoxMstRgwMb(mstDeleteStyle, &pch, MB_DEFYESQUESTION) != IDYES)
		return cmdCancelled;

	stcMapTo = (*stsh.hplestcp)->dnstcp[stcp].stcBase;
	MapStc(PdodDoc(vdocStsh), stcCut, &chp, &pap);
	if (!FRetargetStcBaseNext(vdocStsh, stcCut, stcMapTo, &hplestcpBackup))
		{
		if (hplestcpBackup != hNil)
			{
			FreeH(PdodDoc(vdocStsh)->stsh.hplestcp);
			PdodDoc(vdocStsh)->stsh.hplestcp = hplestcpBackup;
			RestoreStcpAndDependentsDefn(vdocStsh, stcp, &chp, &pap);
			}
		return cmdNoMemory;
		}
	/* discard backup of this style */
	if (vstcBackup != stcNil)
		{
		InvalStyleEntry(&stsh, hsttbChpe, hsttbPape,
				StcpFromStc(vstcBackup, stsh.cstcStd));
		}
#ifdef DEBUG
	else
		Assert(fElActive);
#endif
	/* make name null */
	InvalStyleEntry(&stsh, hsttbChpe, hsttbPape, stcp);

	/* map cut style to stcNormal */
	GenApplyStcPermuteToDoc(selCur.doc, stcCut, stcNormal);
	PdodDoc(vdocStsh)->fStshDirty = fTrue;

	vstcStyle = stcNil;
	vstcEl = stcNil;
	return cmdOK;
}


/*
* Rename current style, returning fTrue if succeeds, fFalse otherwise.
*/
/* %%Function:CmdGosubRename %%Owner:davidbo */
CMD CmdGosubRename()
{
	CMD cmd = cmdNoMemory;
	struct STSH stsh;
	HCABRENAMESTYLE hcab;
	CMB cmb;
	char st[cchMaxStyle + 1];
	CHAR dlt[sizeof(dltRenameStyle)];

	if ((hcab = HcabAlloc(cabiCABRENAMESTYLE)) == hNil)
		return cmdNoMemory;

	if (!FSetCabSz(hcab, szEmpty, Iag(CABRENAMESTYLE, hszRenameStyle)))
		{
		Assert (cmd == cmdNoMemory);
		goto LRet;
		}

	BltDlt(dltRenameStyle, dlt);
	cmb.hcab = hcab;
	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	switch (TmcOurDoDlg(dlt, &cmb))
		{
	case tmcCancel:
		cmd = cmdCancelled;
		break;

	case tmcOK:
		Assert(vstcStyle >= 0 && vstcStyle <= stcStdMin);
		RecordStshForDocNoExcp(vdocStsh, &stsh);
		GetCabStz(hcab, st, sizeof (st), Iag(CABRENAMESTYLE, hszRenameStyle));

		if (!FChangeStInSttb(stsh.hsttbName, StcpFromStc(vstcStyle,stsh.cstcStd), st))
			goto LRet;

		cmd = cmdOK;
		break;
		}

LRet:
	FreeCab(hcab);
	return cmd;
}


/* %%Function:FDlgRenameStyle %%Owner:davidbo */
BOOL FDlgRenameStyle(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int stc, fMatchStd, eid;
	struct STSH stsh;
	char st [cchMaxStyle + 1];

	switch (dlm)
		{
	case dlmInit:
	case dlmChange:
		GrayButtonOnBlank(tmcRSRenameStyle, tmcOK);
		break;

	case dlmTerm:
		if (tmc != tmcOK)
			break;

		GetTmcText(tmcRSRenameStyle, st, cchMaxStyle + 1);
		SzToStInPlace(st);

		RecordStshForDocNoExcp(vdocStsh, &stsh);
		if (!FValidStyleName(st))
			{
			eid = eidBadStyle;
			goto LRenError;
			}
		else  if (FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, st, &stc, &fMatchStd))
			{
			eid = eidStyleExists;
LRenError:
			ErrorEid(eid, "");
			SetFocusTmc(tmcRSRenameStyle);

			return fFalse;
			}
		/* break; */
		}

	return fTrue;
}


CMD CmdGosubMerge()
{
	HCABMERGESTYLE hcab;
	CMD cmd;

	if ((hcab = HcabAlloc(cabiCABMERGESTYLE)) == hNil)
		return fFalse;

	cmd = CmdMrgSty(hcab, fTrue /* fDoDlg */);
	FreeCab(hcab);

	return cmd;
}


/* %%Function:CmdMergeStyle %%Owner:davidbo */
CMD CmdMergeStyle(pcmb)
CMB * pcmb;
{
	return CmdMrgSty(pcmb->hcab, FCmdDialog());
}


/* %%Function:FDlgMergeStyle %%Owner:davidbo */
BOOL FDlgMergeStyle(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int docTest, fEnable;
	BOOL fOnDisk, fOnLine;
	struct DOD *pdod;
	int eid;
	char stFile[ichMaxFile];
	char stName[ichMaxBufDlg];

	switch (dlm)
		{
	case dlmInit:
LDefault:
		docTest = PdodDoc(vdocStsh)->docDot;
		if (docTest == docNil)
			{
			docTest = docGlobalDot;
			if (docTest == docNil)
				fEnable = fFalse;
			else
				goto LTest;
			}
		else
			{
LTest:
			fEnable = fTrue;
			pdod = PdodDoc(docTest);
			if (pdod->fn != fnNil && PfcbFn(pdod->fn)->fReadOnly)
				fEnable = fFalse;
			}
		EnableTmc(tmcMSFrom, docTest != docNil);
		EnableTmc(tmcMSTo, fEnable);

		break;

	case dlmDirFailed:
		eid = eidBadFileDlg;
		goto LErrRet;

	case dlmChange:
		if (tmc != (tmcMSFile & ~ftmcGrouped))
			break;
		GetTmcText(tmcMSFile, stName, sizeof(stName));
		SzToStInPlace(stName);

		UpdateStDOSPath();
		if (FValidFilename(stName, stFile, NULL, &fOnDisk, &fOnLine, nfoDotExt))
			{
			EnableTmc(tmcMSFrom, fTrue);
			fEnable = !fOnDisk;
			/* can only Merge To open docs that aren't ReadOnly */
			if (fOnDisk && fOnLine)
				{
				docTest = DocFromSt(stFile);
				Assert(docTest != docNil);
				pdod = PdodDoc(docTest);
				fEnable = pdod->fn == fnNil ||
					!PfcbFn(pdod->fn)->fReadOnly;
				}
			EnableTmc(tmcMSTo, fEnable);
			}
		else
			goto LDefault;
		break;

	case dlmTerm:
			{
			int docStshNew;
			HCAB hcab;

			UpdateStDOSPath();

			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				{
				/* dialog will end without needing to call EndDlg(tmcError) */
				return (fTrue);
				}

			if (tmc != tmcOK)
				break;

			GetCabSt(hcab, stName, sizeof(stName), Iag(CABMERGESTYLE,hszMergeStyle));
			if (!FValidFilename (stName, stFile, NULL, &fOnDisk, &fOnLine, nfoDotExt))
				{
				eid = eidBadFileDlg;
LErrRet:
				ErrorEid(eid,"FDlgMergeStyle");
				SetTmcTxs(tmcMSFile, TxsAll());
				return fFalse;
				}

			if (!fOnDisk && !fOnLine)
				{
				eid = eidFileNotExist;
				goto LErrRet;
				}

			if ((docStshNew=DocFromSt(stFile)) != docNil && docStshNew == vdocStsh)
				{
				eid = eidCantMerge;
				goto LErrRet;
				}
			break;
			}
		}
	return fTrue;
}


/*
*  Merge style sheets.  Return fTrue if Merge succeeds, fFalse otherwise.
*/
/* %%Function:CmdMrgSty %%Owner:davidbo */
CMD CmdMrgSty(hcab, fDoDlg)
HCABMERGESTYLE hcab;
int fDoDlg;
{
	CMB cmb;
	int docStshNew, fNuke, fFrom, fnStsh, cmd, fRestoreDOSPath;
	char st[ichMaxFile];
	char stFile[ichMaxFile];
	CHAR stDOSPathOld[ichMaxFile+1];
	CHAR dlt[sizeof(dltMergeStyle)];

	SetVdocStsh(selCur.doc);
	fRestoreDOSPath = fFalse;
	if (fDoDlg)
		{
		CHAR szAll[6];

		BltDlt(dltMergeStyle, dlt);

		BuildSzAll(szDot, szAll);
		if (!FSetCabSz(hcab, szAll, Iag(CABMERGESTYLE, hszMergeStyle)))
			{
			cmd = cmdNoMemory;
			goto LRet;
			}

		(*hcab)->iDirList = uNinchList;

		/* Save off current DOS path */
		CopySt(stDOSPath, stDOSPathOld);

		/* change the current path to the DOT directory */
		CchGetTHDSz(st, ichMaxFile);
		SzToStInPlace(st);
		/* remove "*.dot" */
		*st -= 5;
		PrettyUpPath(st);
		AssertDo(FSetCurStzPath(st));
		UpdateStDOSPath();
		fRestoreDOSPath = fTrue;

		cmb.hcab = hcab;
		cmb.cmm = cmmNormal;
		cmb.pv = NULL;
		cmb.bcm = bcmNil;

		TmcOurDoDlg(dlt, &cmb);
		}

	switch (cmb.tmc)
		{
	case tmcError:
		cmd = cmdError;
		goto LRet;

	case tmcCancel:
		cmd = cmdCancelled;
		goto LRet;

	case tmcMSTo:
		fFrom = fFalse;
		goto LGetName;
	case tmcMSFrom:
	case tmcOK:
		fFrom = fTrue;
LGetName:
		GetCabSt(hcab, st, sizeof(st), Iag(CABMERGESTYLE, hszMergeStyle));
		/*  On no name, assume merge with current template */
		if (*st == 0)
			{
LCurrentTemplate:
			docStshNew = PdodDoc(vdocStsh)->docDot;
			if (docStshNew == docNil)
				docStshNew = docGlobalDot;
			/* if docGlobalDot nil, To and From buttons would have been
					disabled! */
			Assert(docStshNew != docNil);
			fNuke = fFalse;
			}
		else
			{
			if (!FValidFilename(st, stFile, NULL, NULL, NULL, nfoDotExt))
				{
				if (cmb.tmc == tmcMSFrom || cmb.tmc == tmcMSTo)
					{
					*st = 0;
					goto LCurrentTemplate;
					}
				else
					{
					/* should be caught at dlmTerm time */
					Assert(fFalse);
					cmd = cmdError;
					goto LRet;
					}
				}

			/*  Verify merge before openning document */
			if (IdMessageBoxMstRgwMb(mstMergeStyleMunge, NULL, 
					MB_YESNOQUESTION) != IDYES)
				{
				cmd = cmdCancelled;
				goto LRet;
				}

			fNuke = (docStshNew = DocFromSt(stFile)) == docNil;
			if (fNuke && (docStshNew = DocOpenStDof(stFile,
					dofNoErrors | dofNoWindow | dofNormalized, NULL)) == docNil)
				{
				cmd = cmdError;
				goto LRet;
				}
#ifdef DEBUG
			if (fNuke)
				Assert(fFrom);
			if (docStshNew == docGlobalDot)
				Assert(!fNuke);
#endif
			}
		break;
		}

	/* tmcOK caught at dlmTerm...check tmcTo and tmcTo here */
	if (docStshNew == vdocStsh)
		{
		ErrorEid(eidCantMerge, "CmdMrgStyle");
		cmd = cmdError;
		goto LRet;
		}

	/* on no name, we haven't asked to verify the merge */
	if ((*st == 0) && IdMessageBoxMstRgwMb(mstMergeStyleMunge, NULL, 
			MB_YESNOQUESTION) != IDYES)
		{
		cmd = cmdCancelled;
		goto LRet;
		}

	if (vfRecording)
		{
		HCABDEFINESTYLE hcab;

		if (((hcab = HcabFromDlg(fFalse)) == hcabNotFilled) ||
				hcab == hcabNull)
			{
			cmd = cmdError;
			goto LRet;
			}

		Assert (hcab == PcmbDlgCur()->hcab);
		if (!FSetCabSt(hcab, st, Iag(CABDEFINESTYLE, hszMergeStyle)))
			{
			vfRecorderOOM = fTrue;
			goto LEndRecord;
			}
		Assert(cmb.tmc == tmcOK || cmb.tmc == tmcMSFrom || cmb.tmc == tmcMSTo);

		/* From or OK -> 1, To -> 0 */
		(*hcab)->uSource = (cmb.tmc == tmcMSTo) ? 0 : 1;
		FRecordCab(hcab, IDDDefineStyle, tmcDSMerge, fFalse);
LEndRecord:
		;
		}

	OpenStsh(docStshNew, fNuke, fFrom);
	cmd = vmerr.fMemFail ? cmdNoMemory : cmdOK;
LRet:
	if (fRestoreDOSPath && FNeNcSt(stDOSPath, stDOSPathOld))
		{
		PrettyUpPath(stDOSPathOld);
		AssertDo(FSetCurStzPath(stDOSPathOld));
		UpdateStDOSPath();
		}
	return cmd;
}


/*
*  If ibstFontDS does not match the ibst of the ftc in stcp's chp,
*  generate an ftc for ibstFontDS and apply it to stcp's stsh entry.
*/
/* %%Function:EnsureFtcDefined %%Owner:davidbo */
EnsureFtcDefined(stc)
int stc;
{
	int ftc;
	char grpprl [3];
	struct DOD * pdod;
	struct CHP chp;
	struct PAP pap;

	SetVdocStsh(selCur.doc);
	pdod = PdodDoc(vdocStsh);
	MapStc(pdod, stc, &chp, &pap);
	if (ibstFontDS == IbstFontFromFtcDoc(chp.ftc, vdocStsh))
		return;
	ftc = FtcFromDocIbst(vdocStsh, ibstFontDS);
	grpprl[0] = sprmCFtc;
	bltbyte(&ftc, &grpprl[1], sizeof(int));
	ApplyGrpprlToStshPrope(grpprl, sizeof(grpprl), fTrue , fFalse);
}


/* %%Function:CmdGotoDefine %%Owner:davidbo */
CMD CmdGotoDefine(stName)
char stName[];
{
	CMD cmd;
	CMB cmb;

	if (!FInitCmb(&cmb, bcmStyles, hNil, cmmNormal | cmmBuiltIn))
		return cmdNoMemory;
	cmb.cmm = cmmDefaults | cmmBuiltIn;
	if ((cmd = CmdExecCmb(&cmb)) != cmdOK)
		return cmd;
	if (!FSetCabSt(cmb.hcab, stName, Iag(CABDEFINESTYLE, hszDSStyle)))
		return cmdNoMemory;
	cmb.cmm = cmmDialog | cmmAction | cmmBuiltIn;
	cmd = CmdExecCmb(&cmb);
	FreeCab(cmb.hcab);
	return cmd;
}


/* %%Function:FDefStyleFromDlg %%Owner:davidbo */
FDefStyleFromDlg(pcesty, fFromOK, pstsh, hsttbChpe, hsttbPape)
CESTY *pcesty;
BOOL fFromOK; /* fTrue iff from OK button, fFalse if from Define button */
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
{
	int tmcErr = tmcNull, stcp;
	char stStyle[cchMaxStyle + 1];

	GetTmcText(tmcDSStyle, stStyle, cchMaxStyle + 1);
	SzToStInPlace(stStyle);

	if (pcesty->istLb == istLbNil)
		{
		if (fFromOK && stStyle[0] == 0 &&
				FStylesEqual(vstcBackup, pstsh, hsttbChpe, hsttbPape))
			return fTrue;
		if (!FValidStyleName(stStyle))
			{
			ErrorEid(eidBadStyle, "");
			tmcErr = tmcDSStyle;
			goto LErrRet;
			}
		}

	if (!fFromOK)
		{
		EnsureFtcDefined(vstcStyle);
		SetIbstFontDSFromStc(vstcStyle);
		}

	if ((tmcErr = TmcVerifyBasedOnNext(pstsh, hsttbChpe, hsttbPape,
			fTrue, !fFromOK, stStyle)) != tmcNull)
		{
		if (tmcErr == tmcError)
			{
			tmcErr = tmcNull;
			goto LErrRetRest;
			}
		tmcErr = (SabGetDlg() & sabDSOptions) ? tmcErr : tmcDSStyle;
		goto LErrRet;
		}

	if (!FDefineStyle(stStyle, ValGetTmc(tmcDSTemplate) != 0 /* fDot */,
			pcesty, fFalse /* fClick */))
		{
		if (vmerr.fMemFail)
			goto LErrRetRest;
		else
			{
			tmcErr = tmcDSStyle;
			goto LErrRet;
			}
		}
	pstsh->cstcStd = PdodDoc(vdocStsh)->stsh.cstcStd;

	if (vfRecording)
		{
		Assert(stStyle[0] != '\0');
		RecordStyTmc(tmcDSDefine);
		RecordStyleProps(IDDStyleChar);
		RecordStyleProps(IDDStylePara);
		RecordStyleProps(IDDStyleAbsPos);
		}

	if (!fFromOK) /* from Define button */
		{
		if (!FSetStcBackup())
			goto LErrRet;

		/* if new style or one of those undefined standards already
			in the list box, need to regenerate mapping */
		if (pcesty->istLb == istLbNil || pcesty->fFake)
			{
			GenLbStyleMapping();
			pcesty->fIgnore = fTrue;
			RefillDSDropDown();
			pcesty->fIgnore = fFalse;
			}

		/* set up to redefine a new style */
		pcesty->fIgnore = fTrue;
		SetFocusTmc(tmcOK);
		RedisplayComboBox(tmcDSStyle, istLbNil);
		SetFocusTmc(tmcDSStyle);
		pcesty->fIgnore = fFalse;
		ChangeCancelToClose();
		EnableTmc(tmcDSDefine, fFalse);

		/* Blow stc cache so UseSelProps picks up the right properties.  */
		vdocFetch = caPara.doc = docNil;
		vstcLast = stcNil;
		}

	CheckForInvalPadPgd(pstsh, hsttbChpe, hsttbPape);

	return fTrue;

LErrRetRest:
	RestoreStcFromBackup();
LErrRet:
	if (tmcErr != tmcNull)
		SetFocusTmc(tmcErr);
	return fFalse;
}

/* %%Function:CmdStyChar %%Owner:davidbo */
CmdStyChar(pcmb)
CMB * pcmb;
{
	struct CHP chp;
	struct PAP pap;

	if (!fElActive)
		{
		Beep();
		return cmdError;
		}

	if (vstcEl == stcNil)
		{
		RtError(rerrModeError);
		return cmdError;
		}

	vstcStyle = vstcEl;
	vfDefineStyle = fTrue;
	SetVdocStsh(selCur.doc);

	if (pcmb->fDefaults)
		{
		MapStc(PdodDoc(vdocStsh), vstcEl, &chp, &pap);

		GetCharDefaults(pcmb, &chp, NULL);
		}

	if (pcmb->fAction)
		{
		int idError;
		struct STTB **hsttbChpe, **hsttbPape;
		struct STSH stsh;

		FApplyHcabToChar(pcmb->hcab);
		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
		CheckForInvalPadPgd(&stsh, hsttbChpe, hsttbPape);
		if (vfElTemplateStyle)
			{
			if (!FDefineInTemplate(&stsh, hsttbChpe, hsttbPape, &idError))
				{
#ifdef NOTUSED
				/* cmdNoMemory and cmdError are the same value.  Old code: */
				return (idError == -1) ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
				return cmdError;
				}
			}
		}
	vfDefineStyle = fFalse;
	return cmdOK;
}


/* %%Function:CmdStyPara %%Owner:davidbo */
CmdStyPara(pcmb)
CMB * pcmb;
{
	struct CHP chp;
	struct PAP pap;

	if (!fElActive)
		{
		Beep();
		return cmdError;
		}

	if (vstcEl == stcNil)
		{
		RtError(rerrModeError);
		return cmdError;
		}

	vstcStyle = vstcEl;
	vfDefineStyle = fTrue;
	SetVdocStsh(selCur.doc);

	if (pcmb->fDefaults)
		{
		MapStc(PdodDoc(vdocStsh), vstcEl, &chp, &pap);

		if (!FGetParaDefaults(pcmb, &pap, NULL,
				fFalse /* fPaps */, fFalse /* fStyList */))
			return cmdError;
		}

	if (pcmb->fAction)
		{
		int idError;
		struct STTB **hsttbChpe, **hsttbPape;
		struct STSH stsh;

		FApplyHcabToPara(pcmb->hcab);
		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
		CheckForInvalPadPgd(&stsh, hsttbChpe, hsttbPape);
		if (vfElTemplateStyle)
			{
			if (!FDefineInTemplate(&stsh, hsttbChpe, hsttbPape, &idError))
				{
#ifdef NOTUSED
				/* cmdNoMemory and cmdError are the same value.  Old code: */
				return (idError == -1) ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
				return cmdError;
				}
			}
		}
	vfDefineStyle = fFalse;
	return cmdOK;
}


#define JcFromI(i)        (i)
#define TlcFromI(i)       (i)

/* %%Function:CmdStyTabs %%Owner:davidbo */
CmdStyTabs(pcmb)
CMB * pcmb;
{
	struct CHP chp;
	struct PAP pap;

	if (!fElActive)
		{
		Beep();
		return cmdError;
		}

	if (vstcEl == stcNil)
		{
		RtError(rerrModeError);
		return cmdError;
		}

	vstcStyle = vstcEl;
	vfDefineStyle = fTrue;
	SetVdocStsh(selCur.doc);

	if (pcmb->fDefaults)
		{
		CABTABS * pcab;

		pcab = *pcmb->hcab;
		pcab->iAlignment = 0;
		pcab->iLeader = 0;
		if (!FSetCabSz(pcmb->hcab, szEmpty, Iag(CABTABS, hszTabsPos)))
			return cmdNoMemory;
		}

	if (pcmb->fAction)
		{
		CABTABS * pcab;
		int dxaTabPos, cbGrpprl;
		int idError;
		BOOL fOverflow;
		CHAR * pb;
		struct TBD tbd;
		struct STTB **hsttbChpe, **hsttbPape;
		struct STSH stsh;
		CHAR grpprl [5 + sizeof (struct TBD)];
		CHAR stzBuf [ichMaxBufDlg + 1];

		if (pcmb->tmc != tmcTabsReset)
			{
			GetCabStz(pcmb->hcab, stzBuf, sizeof (stzBuf),
					Iag(CABTABS, hszTabsPos));
			if (!FZaFromSs(&dxaTabPos, stzBuf + 1, stzBuf[0], vpref.ut,
					&fOverflow))
				{
				RtError(rerrBadParam);
				Assert(fFalse); /* NOT REACHED */
				}
			}

		pcab = *pcmb->hcab;

		pb = grpprl;
		*pb++ = sprmPChgTabs;
		pb += 1; /* space for cbPrl */

		if (pcmb->tmc == tmcTabsReset)
			{
			*pb++ = 1;
			*((int *) pb)++ = 0;
			*((int *) pb)++ = czaMax;
			}
		else  if (*pb++ = (pcmb->tmc == tmcTabsClear))
			{
			*((int *) pb)++ = dxaTabPos;
			*((int *) pb)++ = dxaCloseMin;
			}

		if (*pb++ = (pcmb->tmc == tmcTabsSet))
			{
			*((int *) pb)++ = dxaTabPos;
			tbd.jc = JcFromI(pcab->iAlignment);
			tbd.tlc = TlcFromI(pcab->iLeader);
			*((struct TBD *) pb)++ = tbd;
			}

		cbGrpprl = pb - grpprl;
		grpprl[1] = cbGrpprl - 2;

		ApplyGrpprlToStshPrope(grpprl, cbGrpprl, fFalse, fFalse);

		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
		CheckForInvalPadPgd(&stsh, hsttbChpe, hsttbPape);

		if (vfElTemplateStyle)
			{
			if (!FDefineInTemplate(&stsh, hsttbChpe, hsttbPape, &idError))
				{
#ifdef NOTUSED
				/* cmdNoMemory and cmdError are the same value.  Old code: */
				return (idError == -1) ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
				return cmdError;
				}
			}
		}
	vfDefineStyle = fFalse;
	return cmdOK;
}


/* %%Function:CmdStyPos %%Owner:davidbo */
CmdStyPos(pcmb)
CMB * pcmb;
{
	struct CHP chp;
	struct PAP pap;

	if (!fElActive)
		{
		Beep();
		return cmdError;
		}

	if (vstcEl == stcNil)
		{
		RtError(rerrModeError);
		return cmdError;
		}

	vfDefineStyle = fTrue;
	SetVdocStsh(selCur.doc);

	if (pcmb->fDefaults)
		{
		MapStc(PdodDoc(vdocStsh), vstcEl, &chp, &pap);
		GetAbsPosDefaults(pcmb, &pap, NULL);
		}

	if (pcmb->fAction)
		{
		int idError;
		struct STTB **hsttbChpe, **hsttbPape;
		struct STSH stsh;

		FApplyHcabToAbsPos(pcmb->hcab);
		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
		CheckForInvalPadPgd(&stsh, hsttbChpe, hsttbPape);
		if (vfElTemplateStyle)
			{
			if (!FDefineInTemplate(&stsh, hsttbChpe, hsttbPape, &idError))
				{
#ifdef NOTUSED
				/* cmdNoMemory and cmdError are the same value.  Old code: */
				return (idError == -1) ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
				return cmdError;
				}
			}
		}
	vfDefineStyle = fFalse;
	return cmdOK;
}


/* %%Function:CmdElDefineStyle %%Owner:davidbo */
CmdElDefineStyle(pcmb)
CMB * pcmb;
{
	if (!fElActive)
		{
		Beep();
		return cmdError;
		}

	SetVdocStsh(selCur.doc);
	if (pcmb->fDefaults)
		{
		SetVstcSel();
		InitStyleCabToStc(pcmb, vstcEl);

		FSetCabSz(pcmb->hcab, szEmpty, Iag(CABDEFINESTYLE, hszDSNewName));
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		return cmdError;

	if (pcmb->fAction)
		{
		int stc, stcp, stcCut, docStshNew;
		int eid, idError, uSource, fMatchStd;
		BOOL fNuke;
		BOOL fFrom;
		struct STTB ** hsttbChpe, ** hsttbPape;
		struct STSH stsh;
		char stName [cchMaxStyle+1];
		char stFile [ichMaxFile];
		char stTemplate [ichMaxFile];

		if (pcmb->tmc != tmcDSMerge)
			{
			GetCabSt(pcmb->hcab, vstElStyle, cchMaxStyle, 
					Iag(CABDEFINESTYLE, hszDSStyle));
			if ((stc = StcUseStyle(vstElStyle, 
					((CABDEFINESTYLE *) *pcmb->hcab)->fTemplate)) == stcNil)
				{
				return cmdError;
				}
			}

		switch (pcmb->tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif

		case tmcDSDefine:
		case tmcOK:
			GetCabSt(pcmb->hcab, stName, sizeof(stName), 
					Iag(CABDEFINESTYLE, hszDSBasedOn));
			RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
			if (stName[0] != '\0')
				{
				if (!FChangeBN(vstElStyle, stName, &stsh, hsttbChpe, hsttbPape,
						0 /* pcesty */, fTrue /* fBasedOn */, fFalse /* fClick */,
						fFalse /* fDlg */))
					{
#ifdef NOTUSED
					/* cmdNoMemory and cmdError are the same value. Old code: */
					return vmerr.fMemFail ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
					return cmdError;
					}
				}

			GetCabSt(pcmb->hcab, stName, sizeof(stName), 
					Iag(CABDEFINESTYLE, hszDSNext));
			if (stName[0] != '\0')
				{
				if (!FChangeBN(vstElStyle, stName, &stsh, hsttbChpe, hsttbPape,
						0 /* pcesty */, fFalse /* fBasedOn */, fFalse /* fClick */,
						fFalse /* fDlg */))
					{
#ifdef NOTUSED
					/* cmdNoMemory and cmdError are the same value. Old code: */
					return vmerr.fMemFail ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
					return cmdError;
					}
				}
			CheckForInvalPadPgd(&stsh, hsttbChpe, hsttbPape);
			goto LDoTemplate;

		case tmcDSMerge:
			uSource = ((CABDEFINESTYLE *) *pcmb->hcab)->uSource;
			if (uSource != 0 && uSource != 1)
				{
LBadParam:
				RtError(rerrBadParam);
				Assert(fFalse); /* NOT REACHED */
				}
			fFrom = uSource == 1;
			GetCabSt(pcmb->hcab, stTemplate, cchMaxFile,
					Iag(CABDEFINESTYLE, hszMergeStyle));
			if (*stTemplate == 0)
				{
				if (docGlobalDot == docNil)
					RtError(rerrModeError);
				docStshNew = PdodDoc(vdocStsh)->docDot;
				if (docStshNew == docNil)
					docStshNew = docGlobalDot;
				Assert(docStshNew != docNil);
				fNuke = fFalse;
				}
			else
				{
				if (!FNormalizeStFile(stTemplate, stFile, nfoDot))
					goto LBadParam;

				fNuke = (docStshNew = DocFromSt(stFile)) == docNil;
				/* can only Merge To open docs */
				if (fNuke && !fFrom)
					goto LBadParam;
				/* cannot Merge To a ReadOnly doc either */
				if (!fNuke && !fFrom)
					{
					struct DOD *pdod = PdodDoc(docStshNew);
					if (pdod->fn != fnNil && PfcbFn(pdod->fn)->fReadOnly)
						goto LBadParam;
					}
				if (fNuke && (docStshNew = DocOpenStDof(stFile,
						dofNoErrors | dofNoWindow | dofNormalized, NULL))==docNil)
					{
					RtError(rerrModeError);
					Assert(fFalse); /* NOT REACHED */
					}
#ifdef DEBUG
				if (docStshNew == docGlobalDot)
					Assert(!fNuke);
#endif
				}

			if (docStshNew == vdocStsh)
				{
				eid = eidCantMerge;
				goto LRenError;
				}

			OpenStsh(docStshNew, fNuke, fFrom);
			break;

		case tmcDSRename:
			GetCabSt(pcmb->hcab, stName, cchMaxStyle+1,
					Iag(CABDEFINESTYLE, hszDSNewName));
			RecordStshForDocNoExcp(vdocStsh, &stsh);

			if (!FValidStyleName(stName))
				{
				eid = eidBadStyle;
				goto LRenError;
				}
			if (FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd,
					stName, &stc, &fMatchStd))
				{
				eid = eidStyleExists;
LRenError:
				ErrorEid(eid, "");
				return cmdError;
				}

			if (!FChangeStInSttb(stsh.hsttbName, 
					StcpFromStc(vstcStyle, stsh.cstcStd), stName))
				return cmdNoMemory;

LDoTemplate:
			if (vfElTemplateStyle)
				{
				struct STTB **hsttbChpe, **hsttbPape;
				struct STSH stsh;
				RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
				if (!FDefineInTemplate(&stsh, hsttbChpe, hsttbPape, &idError))
					{
#ifdef NOTUSED
					/* cmdNoMemory and cmdError are the same value. Old code: */
					return (idError == -1) ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
					return cmdError;
					}
				}
			break;

		case tmcDSDelete:
			RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
			stcp = StcpFromStc(vstcEl, stsh.cstcStd);
			Assert(stcp == StcpFromStc(vstcStyle, stsh.cstcStd));
			return CmdDeleteStyle(stcp, fFalse);
			break;
			}
		}

	return cmdOK;
}


InitStyleCabToStc(pcmb, stc)
CMB *pcmb;
int stc;
{
	int stcp, stcBase, stcNext;
	struct STSH stsh;
	char stName [cchMaxStyle];

	if (stc == stcNil)
		stName[0] = 0;
	else
		{
		RecordStshForDocNoExcp(vdocStsh, &stsh);
		stcp = StcpFromStc(stc, stsh.cstcStd);
		GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, stcp);
		StToSzInPlace(stName);
		}
	FSetCabSz(pcmb->hcab, stName, Iag(CABDEFINESTYLE, hszDSStyle));

	if (stc != stcNil)
		{
		GetStcBaseNextForStcp(&stsh, stcp, &stcBase, &stcNext);
		stcp = StcpFromStc(stcBase, stsh.cstcStd);
		GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, stcp);
		StToSzInPlace(stName);
		}
	else
		{
		FSetCabSz(pcmb->hcab, SzSharedKey("Normal",StyleNormal), 
				Iag(CABDEFINESTYLE, hszDSBasedOn));
		}

	FSetCabSz(pcmb->hcab, stName, Iag(CABDEFINESTYLE, hszDSBasedOn));

	if (stc != stcNil)
		{
		stcp = StcpFromStc(stcNext, stsh.cstcStd);
		GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, stcp);
		StToSzInPlace(stName);
		}
	FSetCabSz(pcmb->hcab, stName, Iag(CABDEFINESTYLE, hszDSNext));
}


/* S T C  U S E  S T Y L E

	Sets "things" up for using the named style and returns it's stc (or stcNil
	if anything goes wrong).  The style will be created if necessary, in which
	case it will contain the properties of the selection.
*/
/* %%Function:StcUseStyle %%Owner:davidbo */
StcUseStyle(stName, fTemplate)
char * stName;
BOOL fTemplate;
{
	int stc, stcp;
	BOOL fStd;
	struct STSH stsh;
	struct STTB ** hsttbChpe, ** hsttbPape;

	vstcEl = vstcStyle = stcNil;
	SetVdocStsh(selCur.doc);
	vfElTemplateStyle = fTemplate && !PdodDoc(vdocStsh)->fDot;

	if (!FValidStyleName(stName))
		{
		ErrorEid(eidBadStyle, " StcUseStyle");
		return stcNil;
		}

	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	if (!FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, stName, &stc, &fStd))
		{
		if ((stcp = StcpCreateNewStyle(vdocStsh, fFalse /* fUseReserve */)) == stcpNil)
			{
			if (!vmerr.fMemFail)
				ErrorEid(eidStshFull, " StcUseStyle");
			return stcNil;
			}

		stc = vstcStyle = StcFromStcp(stcp, stsh.cstcStd);
		if (!FUseSelProps(stcp, fFalse) ||
				!FChangeStInSttb(stsh.hsttbName, stcp, stName))
			{
			InvalStyleEntry(&stsh, hsttbChpe, hsttbPape, stcp);
			return stcNil;
			}
		PdodDoc(vdocStsh)->fStshDirty = fTrue;
		}
	else  if (fStd)
		{
		if (!FEnsureStcDefined(vdocStsh, stc, &stsh.cstcStd))
			return stcNil;
		}

	vstcEl = vstcStyle = stc;

	return stc;
}


/* %%Function:Record %%Owner:davidbo */
RecordStyTmc(tmc)
{
	HCAB hcab;

	if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled ||
			hcab == hcabNull)
		return;

	Assert (hcab == PcmbDlgCur()->hcab);

	FRecordCab(PcmbDlgCur()->hcab, IDDDefineStyle, tmc, fFalse);
	((CESTY *) PcmbDlgCur()->pv)->fRecorded = fTrue;
}


#ifdef DEBUG
/* %%Function:CommRgb %%Owner:davidbo */
CommRgb(pb, cb)
char * pb;
int cb;
{
	static char rgchHex [] = "0123456789ABCDEF";
	int ich;
	char * pch;
	char szLine [80];

	while (cb > 0)
		{
		pch = szLine;
		for (ich = 0; ich < 3*16; ich += 3)
			{
			*pch++ = rgchHex[(*pb) >> 4];
			*pch++ = rgchHex[(*pb) & 0xf];
			*pch++ = ' ';
			cb -= 1;
			pb += 1;
			}
		*pch++ = '\r';
		*pch++ = '\n';
		*pch = '\0';
		CommSz(szLine);
		}
}


#endif

#ifdef DAVIDBO
/* %%Function:DumpStsh %%Owner:davidbo */
DumpStsh(pstsh, hsttbChpe, hsttbPape)
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
{
	int stc, stcp, stcpMac;
	char st[cchMaxSz];
	int rgNum[cchMaxSz + 1];

	stcpMac = (*pstsh->hsttbName)->ibstMac;
	for (stcp = 0; stcp < stcpMac; stcp++)
		{
		stc = StcFromStcp(stcp, pstsh->cstcStd);
		GetStFromSttb(pstsh->hsttbName, stcp, st);
		if (st[0] == 0)
			GenStyleNameForStcp(st, pstsh->hsttbName, pstsh->cstcStd, stcp);
		else  if (st[0] == 0xff)
			continue;
		StToSzInPlace(st);
		CommSz(st);
		CommSzNum(SzShared("  "), stc);

		GetStFromSttb(pstsh->hsttbChpx, stcp, st);
		StToRgNum(st, rgNum);
		CommSzRgNum(SzShared("\tChpx: "), rgNum, st[0]);

		GetStFromSttb(pstsh->hsttbPapx, stcp, st);
		StToRgNum(st, rgNum);
		CommSzRgNum(SzShared("\tPapx: "), rgNum, st[0]);

		GetStFromSttb(hsttbChpe, stcp, st);
		StToRgNum(st, rgNum);
		CommSzRgNum(SzShared("\tChpe: "), rgNum, st[0]);

		GetStFromSttb(hsttbPape, stcp, st);
		StToRgNum(st, rgNum);
		CommSzRgNum(SzShared("\tPape: "), rgNum, st[0]);

		GetStcBaseNextForStcp(pstsh, stcp, &rgNum[0], &rgNum[1]);
		CommSzRgNum(SzShared("\tbase/next: "), rgNum, 2);

		CommSz(SzShared("\n\r"));
		}
}


/* %%Function:StToRgNum %%Owner:davidbo */
StToRgNum(st, rg)
char st[];
int rg[];
{
	int i, iMac;

	/* make st's count byte include itself */
	st[0] += 1;
	iMac = st[0];
	for (i = 0; i < iMac; i++)
		rg[i] = (int) st[i];
}


#endif
