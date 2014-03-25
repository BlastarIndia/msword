/* search.c    Search/Replace */

#define NOGDICAPMASKS
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
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
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "doc.h"
#include "sel.h"
#include "ch.h"
#include "fkp.h"
#include "file.h"
#include "winddefs.h"
#include "prompt.h"
#include "message.h"
#include "cmdtbl.h"

#include "keys.h"
#include "wininfo.h"
#include "inter.h"
#include "field.h"
#include "debug.h"
#include "error.h"
#include "prm.h"
#include "cmd.h"
#include "cmdlook.h"
#define FINDNEXT
#define SEARCHTMC
#include "search.h"
#include "tmc.h"
#include "core.h"
#include "format.h"
#include "border.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "search.hs"
#include "search.sdm"
#include "replace.hs"
#include "char.hs"
#include "para.hs"


#ifdef PROTOTYPE
#include "search.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */
extern BOOL          fElActive;
extern int	vdocFetch;
extern struct SEL selCur;
extern struct CHP vchpFetch;
extern struct PAP vpapFetch;
extern struct CHP vchpStc;
extern CP            vcpFetch;
extern int	vccpFetch;
extern char	HUGE     *vhpchFetch;
extern int	vfSeeSel;
extern struct MERR vmerr;
extern struct CA caPara;
extern int	wwCur;
extern BOOL          vfExtendSel;
extern KMP           **hkmpCur;
extern struct WWD **hwwdCur;
extern FNI           fni;
extern int	vdocStsh;
extern BOOL          vfRecording;
extern BOOL          vfAwfulNoise;
extern struct TCC vtcc;
extern struct RRI *vprri;


CP CpSearchFb(), CpSearchFbBackward();
SetSpecialMatch();
CHAR ChAnsiFromOem();

#ifdef DEBUG
CP C_CpSearchSz();
CP C_CpSearchSzBackward();
#endif


/* G L O B A L S */
CP      vcpMatchFirst, vcpMatchLim;
CP      vcpSearchStart;
CP      vcpSearchLim = cpNil;
/* vcpLimWrap exists so that we avoid searching through more of
	the document than necessary after starting a search of the entire
	document from the interior of the document and reaching the end
	of the document before finding a match.  If the user decides to wrap
	then we want to search from the beginning of the document to vcpLimWrap
	for the second part of the search.	vcpLimWrap is maintained
	as the maximum cpLim reached whenever CpSearchSz has found
	a match for the tail of the pattern starting at the cpFirst
	passed to CpSearchSz.  Without this optimization we must search
	to the end of the document a second time in order to ensure we
	have not missed a valid match of the pattern.
		A parallel, backwards use of vcpLimWrap exists for
	CpSearchSzBackward. */
CP	vcpLimWrap;
CP      vcpLimWrapSave;
BOOL    vfSearchWord = fFalse;
BOOL    vfSearchCase = fFalse;
BOOL    vfNoWrap = fFalse;
BOOL    vfFound;
BOOL    vfFwd = fTrue;
BOOL    vfReplace;
BOOL    vfSearchRepl;
BOOL    vfSearchWholeDoc;
int	vtmcFocus;
struct FB vfbSearch;  /* search formatting block */
struct FB vfbReplace;  /* replace formatting block */
struct CHP vchpSRGray;        /* combination of SearchGray and ReplaceGray */

CHAR        vszSearch[cchSearchMax] = "";
CHAR        vszReplace[cchReplaceMax] = "";

extern BOOL vfConfirm;


/* C M D  S E A R C H */
/* %%Function:CmdSearch %%Owner:rosiep */
CMD CmdSearch(pcmb)
CMB *pcmb;
{
	CABSEARCH * pcab;
	int	tmc;
	struct BMIB bmib;
	CMD cmdT;

	Assert (wwCur != wwNil);
	Assert (PselActive() == &selCur);

	if (selCur.fBlock)
		TurnOffBlock(&selCur);

	vfSearchRepl = fTrue;
	vfReplace = fFalse;
	InitSearchFormatting();
	vcpLimWrapSave = cpNil;

	tmc  = tmcOK;
	cmdT = cmdOK;

	if (FCmdFillCab())
		{
		/* initialize Command Argument Block */
		pcab = (CABSEARCH * ) * pcmb->hcab;
		pcab->sab = 0;
		pcab->fFormatted = fFalse;
		pcab->fWholeWord = vfSearchWord;
		pcab->fMatchUpLow = vfSearchCase;
		pcab->iDirection = vfFwd;
		if (!FSetCabSz(pcmb->hcab, vszSearch, Iag (CABSEARCH, hszSearch)))
			return cmdNoMemory;
		}

	if (FCmdDialog())
		{
		BOOL fRecordingSav;
		CHAR dlt [sizeof(dltSearch)];
		struct FB fbSearchT;

		BltDlt(dltSearch, dlt);

		fbSearchT = vfbSearch;

		InhibitRecorder(&fRecordingSav, fTrue);
		pcmb->pv = &fRecordingSav;

		switch (tmc = TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			cmdT = cmdNoMemory;
			break;
		case tmcCancel:
			vfbSearch = fbSearchT;
			cmdT = cmdCancelled;
			break;
			}

		InhibitRecorder(&fRecordingSav, fFalse);
		}

	if (FCmdAction() && tmc == tmcOK)
		{
		CABSEARCH * pcab;
		Profile(vpfi == pfiSearch ? StartProf(30) : 0);

		pcab = *pcmb->hcab;

		vfSearchWholeDoc = fFalse;

		GetCabSz(pcmb->hcab, vszSearch, cchSearchMax,
				Iag (CABSEARCH, hszSearch));

		if (fElActive)
			{
			vfNoWrap = fFalse;
			switch (pcab->iDirection)
				{
#ifdef DEBUG
			default:
				Assert(fFalse);
				/* FALL THROUGH */
#endif

			case 1:
				vfNoWrap = fFalse;
				break;

			case 2:
				vfSearchWholeDoc = fTrue;
				pcab->iDirection = 1;
				/* FALL THROUGH */

			case 0:
				vfNoWrap = fTrue;
				break;
				}

			vfbSearch.fChp = vfbSearch.fPap = 0;
			if (!FValidSRText(vszSearch, &vfbSearch))
				{
				if (vmerr.fMemFail)
					ErrorNoMemory(eidNoMemOperation);
				else
					ErrorEid(eidInvalidStyle, "CmdSearch");
				cmdT = cmdError;
				goto LRet;
				}

			if (((CABSEARCH * ) * pcmb->hcab)->fFormatted)
				CheckForFormatting();
			}

		/* else FValidSRText && CheckForFormatting done in dialog proc */
		/***/

		/* if empty search text and no search formatting, we got here
			erroneously through user executing Repeat after a cancelled
			search */

		if (!vfbSearch.fInfo)
			{
			/* Repeat should be disabled after a cancelled search, but
				just in case, we have this message and we bag out. */
			/* FUTURE (rp): If the Report can't be caused to fail, lets remove the message */
			ReportSz("Should never get here!");
			ErrorEid(eidNoPrevSearch, "CmdSearch");
			cmdT = cmdError;
			goto LRet;
			}

		fni.is = isSearch;

		pcab = (CABSEARCH * ) * pcmb->hcab;
		vfSearchWord = pcab->fWholeWord;
		vfSearchCase = pcab->fMatchUpLow;
		vfFwd = pcab->iDirection;
		EnablePreload(PdodDoc(selCur.doc)->fn);

		SetSpecialMatch(&bmib);

		vfFound = fFalse;
		/* if extending sel, we don't want to wrap */
		if (!fElActive)
			vfNoWrap = vfExtendSel;
		FSetUpForNextSearch(&bmib, fFalse /* fIgnoreMatch */);

		SetPromptMst(mstSearching, pdcCkReport);
		InitAbortCheck();

		FFindNext(&bmib);

		TerminateAbortCheck(pdcRestoreImmed);

		DisablePreload();
		Profile(vpfi == pfiSearch ? StopProf() : 0);
		}

LRet:
	vfSearchRepl = fFalse;
	return (cmdT);
}





/* S E A R C H  A G A I N */
/* command will repeat last find or scan looks command */

/* %%Function:SearchAgain %%Owner:rosiep */
SearchAgain()
{
	struct BMIB bmib;
	struct WWD *pwwd;

	Assert (wwCur != wwNil);
	Assert (PselActive() == &selCur);

	if (selCur.fBlock)
		TurnOffBlock(&selCur);

	pwwd = *hwwdCur;

	SetSpecialMatch(&bmib);
	vfReplace = fFalse;
	FSetUpForNextSearch(&bmib, fFalse /* fIgnoreMatch */);
	vfFound = fFalse;
	vfNoWrap = vfExtendSel; /* if extending sel, we don't want to wrap */
	SetPromptMst(mstSearching, pdcCkReport);
	InitAbortCheck();
	FFindNext(&bmib);
	TerminateAbortCheck(pdcRestoreImmed);
}




/* F  D L G  S E A R C H  R E P L */
/* shared by search and replace dialogs */

/* %%Function:FDlgSearchRepl %%Owner:rosiep */
BOOL FDlgSearchRepl(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{
	int	cchSearch, cchBanter;
	int	EnumFontUp(), EnumFontDown(), EnumPtSizeUp(), EnumPtSizeDn();
	int	EnumColorUp(), EnumColorDn();
	int	DoLooksStrike(), DoLooksRMark(), Beep();
	static int	fNewestCode = fFalse;
	static int	fNewestUtil = fFalse;
	CMB cmb;
	BOOL fTmcValSet = fFalse;
	BOOL fBeepMacro;

	char	sz[cchSearchMax];  /* assumes cchSearchMax >= cchReplaceMax */

	Assert(tmcSearch == tmcRSearch);
	switch (dlm)
		{
	default:
		break;

	case dlmIdle:
		vfAwfulNoise = fFalse; /* enable beep for illegal looks keys */
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (wNew /* cIdle */ == 2)
			{
			fNewestCode = fTrue;
			/* make sure we don't throw out what we need */
			FAbortNewestCmg(vfReplace ? cmgReplaceCode : cmgSearchCode, fFalse, fTrue);
			FAbortNewestCmg(vfReplace ? cmgReplaceUtil : cmgSearchUtil, fFalse, fTrue);

			FAbortNewestCmg(vfReplace ? cmgReplaceCode : cmgSearchCode, fTrue, fTrue);
			}
		else  if (wNew /* cIdle */ == 4)
			{
			fNewestUtil = fTrue;
			FAbortNewestCmg (vfReplace ? cmgReplaceUtil : cmgSearchUtil, fTrue, fTrue);
			}
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */

	case dlmInit:
		fBeepMacro = ((*hwwdCur)->wk == wkMacro);

		/* Add all the looks keys to the Search (or Replace) keymap. */
		/* Don't allow search & replace formatting in macro desktop */
		AddLooksKeys((*hkmpCur)->hkmpNext, hkmpCur, ilcdMaxSearch);
		AddKeyPfn(hkmpCur, KcCtrl(vkFont), fBeepMacro ? Beep : EnumFontUp);
		AddKeyPfn(hkmpCur, KcShift(KcCtrl(vkFont)), fBeepMacro ? Beep : EnumFontDown);
		AddKeyPfn(hkmpCur, KcCtrl(vkPoint), fBeepMacro ? Beep : EnumPtSizeUp);
		AddKeyPfn(hkmpCur, KcShift(KcCtrl(vkPoint)), fBeepMacro ? Beep : EnumPtSizeDn);
		AddKeyPfn(hkmpCur, KcCtrl(vkColor), fBeepMacro ? Beep : EnumColorUp);
		AddKeyPfn(hkmpCur, KcShift(KcCtrl(vkColor)), fBeepMacro ? Beep : EnumColorDn);

		/* Don't allow user to replace revision properties */
		AddKeyPfn(hkmpCur, KcCtrl(vkStrike), fBeepMacro || vfReplace ? Beep : DoLooksStrike);
		AddKeyPfn(hkmpCur, KcCtrl(vkRMark), fBeepMacro || vfReplace ? Beep : DoLooksRMark);

		/* the above additions to the keymap could fail on low memory */
		if (FRareT(reMemAlert7, vmerr.fMemFail))
			return fFalse;

		DisplaySRBanter(tmcSearchBanter, &vfbSearch.chp, &vfbSearch.chpGray,
				&vfbSearch.pap, &vfbSearch.papGray);
		if (vfReplace)
			DisplaySRBanter(tmcReplaceBanter, &vfbReplace.chp,
					&vfbReplace.chpGray, &vfbReplace.pap, &vfbReplace.papGray);
		vtmcFocus = tmcSearch;
		goto LEnable;

	case dlmKillItmFocus:
		vtmcFocus = tmcSearchNil;
		break;

	case dlmSetItmFocus:
		if (tmc == tmcSearch || tmc == tmcReplace)
			vtmcFocus = tmc;
		else
			vtmcFocus = tmcSearchNil;
		break;

	case dlmChange:
		if (tmc == tmcSearch)
			{
LEnable:
			cchBanter = CchGetTmcText(tmcSearchBanter, sz, cchSearchMax);
			cchSearch = CchGetTmcText(tmcSearch, sz, cchSearchMax);
			EnableTmc(tmcOK, cchSearch > 0 || cchBanter > 0);
			SetWordSearch( sz );
			return (fTrue);
			}
		break;
	case dlmTerm:
		InhibitRecorder((BOOL *) PcmbDlgCur()->pv, fFalse);
		if (tmc == tmcOK)
			{
			int	tmcErr;

			CchGetTmcText(tmcSearch, sz, cchSearchMax);
			if (!FValidSRText(sz, &vfbSearch))
				{
				tmcErr = tmcSearch;
LStyleErr:
				if (vmerr.fMemFail)
					{
					ErrorNoMemory(eidNoMemOperation);
					break;
					}
				else
					{
					ErrorEid(eidInvalidStyle, "FDlgSearchRepl");
					SetTmcTxs(tmcErr, TxsAll());
					return (fFalse); /* stay in dialog */
					}
				}
			if (vfReplace)
				{
				CchGetTmcText(tmcReplace, sz, cchReplaceMax);
				if (!FValidSRText(sz, &vfbReplace))
					{
					tmcErr = tmcReplace;
					goto LStyleErr;
					}
				}

			CheckForFormatting();  /* to set fChp, fPap bits in FB's */

			if (vfRecording)
				{
				if (vfbSearch.fChp)
					{
					/* HACK to keep revision marking property alone
					from causing a EditSearchChar statment... */
					if (!vfbSearch.chpGray.fRMark || 
							!vfbSearch.chpGray.fStrike)
						{
						BOOL fRecFmt;

						vfbSearch.chpGray.fRMark = fTrue;
						vfbSearch.chpGray.fStrike = fTrue;
						fRecFmt = FZeroBit(&vfbSearch.chpGray, 
								cwCHP);
						vfbSearch.chpGray.fRMark = fFalse;
						vfbSearch.chpGray.fStrike = fFalse;
						if (!fRecFmt)
							goto LSkipSChar;
						}

					if ((cmb.hcab = HcabAlloc(cabiCABCHARACTER)) == 
							hNil)
						{
						ErrorNoMemory(eidNoMemForRecord);
						break;
						}
					fTmcValSet = fTrue;
					SetTmcVal(vfReplace ? tmcRFmt : tmcSFmt, fTrue);
					GetCharDefaults(&cmb, &vfbSearch.chp, 
							&vfbSearch.chpGray);
					FRecordCab(cmb.hcab, IDDSearchChar, 
							tmcOK, fFalse);
					FreeCab(cmb.hcab);
					}
LSkipSChar:
				if (vfbSearch.fPap)
					{
					if ((cmb.hcab = HcabAlloc(cabiCABPARALOOKS)) == hNil)
						{
						ErrorNoMemory(eidNoMemForRecord);
						break;
						}
					if (!fTmcValSet)
						{
						SetTmcVal(vfReplace ? tmcRFmt : tmcSFmt, fTrue);
						fTmcValSet = fTrue;
						}
					if (FGetParaDefaults(&cmb, &vfbSearch.pap,
							&vfbSearch.papGray, fTrue  /* fPaps */, fFalse /* fStyList */))
						FRecordCab(cmb.hcab, IDDSearchPara, tmcOK, fFalse);
					FreeCab(cmb.hcab);
					}

				if (vfReplace && vfbReplace.fChp)
					{
					/* HACK to keep revision marking property alone
					from causing a EditReplaceChar statment... */
					if (!vfbReplace.chpGray.fRMark || 
							!vfbReplace.chpGray.fStrike)
						{
						BOOL fRecFmt;

						vfbReplace.chpGray.fRMark = fTrue;
						vfbReplace.chpGray.fStrike = fTrue;
						fRecFmt = FZeroBit(&vfbReplace.chpGray, 
								cwPAPS);
						vfbReplace.chpGray.fRMark = fFalse;
						vfbReplace.chpGray.fStrike = fFalse;
						if (!fRecFmt)
							goto LSkipRChar;
						}

					if ((cmb.hcab = HcabAlloc(cabiCABCHARACTER)) == hNil)
						{
						ErrorNoMemory(eidNoMemForRecord);
						break;
						}
					if (!fTmcValSet)
						{
						SetTmcVal(tmcRFmt, fTrue);
						fTmcValSet = fTrue;
						}
					GetCharDefaults(&cmb, &vfbReplace.chp, &vfbReplace.chpGray);
					FRecordCab(cmb.hcab, IDDReplaceChar, tmcOK, fFalse);
					FreeCab(cmb.hcab);
					}
LSkipRChar:
				if (vfReplace && vfbReplace.fPap)
					{
					if ((cmb.hcab = HcabAlloc(cabiCABPARALOOKS)) == hNil)
						{
						ErrorNoMemory(eidNoMemForRecord);
						break;
						}
					if (!fTmcValSet)
						{
						SetTmcVal(tmcRFmt, fTrue);
						fTmcValSet = fTrue;
						}
					if (FGetParaDefaults(&cmb, &vfbReplace.pap,
							&vfbReplace.papGray, fTrue  /* fPaps */, fFalse /* fStyList */))
						FRecordCab(cmb.hcab, IDDReplacePara, tmcOK, fFalse);
					FreeCab(cmb.hcab);
					}
				}

			/* make sure the code is in */
			/* fNewestUtil implies fNewestCode, so we only
				need to check the latter if not the former */
			if (!fNewestUtil)
				{
				FAbortNewestCmg (vfReplace ? 
						cmgReplaceUtil : cmgSearchUtil, fTrue,
						fFalse);
				if (!fNewestCode)
					FAbortNewestCmg (vfReplace ? 
							cmgReplaceCode : cmgSearchCode,
							fTrue, fFalse);
				}
			}
		break;
		}
	return (fTrue);
}


/*
*  F  V A L I D  S R  T E X T
*
*  Checks to see if edit control text begins with ^y.  If so, it is
*  trying to be a style and better be a defined or standard one or
*  it is illegal and we return fFalse.  If it is a legal style, we
*  set the appropriate stc in the Formatting Block.
*
*/

/* %%Function:FValidSRText %%Owner:rosiep */
BOOL FValidSRText(sz, pfb)
char	*sz;
struct FB *pfb;
{
	struct STSH *pstsh;
	char	st[cchSearchMax];  /* assumes cchSearchMax >= cchReplaceMax - 2 */
	int	stc;
	BOOL fMatchStd;
	int	docStsh = DocMother(selCur.doc);
	int	cstcStd;

	pfb->pap.stc = 0;
	pfb->papGray.stc = -1;

	if (*sz == '\0')   /* no text */
		pfb->fText = fFalse;

		/* check if it's a style */
	else  if (*sz == chPrefixMatch && *(sz + 1) == chMatchStyle)
		{
		pstsh = &(PdodMother(selCur.doc)->stsh);
		for (sz += 2; *sz == chSpace; sz++)
			;

		SzToSt(sz, st);
		if (FMatchDefinedAndStdStyles(pstsh->hsttbName, pstsh->cstcStd,
				st, &stc, &fMatchStd))
			{
			/* valid style */
			if (fMatchStd)
				if (!FEnsureStcDefined(docStsh, stc, &cstcStd))
					return fFalse;
			pfb->pap.stc = stc;
			pfb->papGray.stc = 0;
			if (fElActive) /* as we skip CheckForFormatting for the style case
							when we do a style search from a macro */
				pfb->fPap = fTrue;
			pfb->fText = fFalse;
			}
		else
			{
			/* invalid style */
			return fFalse;
			}
		}
	else  /* there's text and it's not a style */
		pfb->fText = fTrue;

	return fTrue;
}


/* I N I T  S  R  C H P S */
/* Initialize CHPs for Search/Replace dialogs */

/* %%Function:InitSRChps %%Owner:rosiep */
InitSRChps(fSearch)
BOOL fSearch;
{
	struct FB *pfb = (fSearch ? &vfbSearch : &vfbReplace);

	SetBytes(&(pfb->chp), 0, cbCHP);
	SetBytes(&(pfb->chpGray), -1, cbCHP);
}


/* I N I T  S  R  P A P S */
/* Initialize PAPs for Search/Replace dialogs */

/* %%Function:InitSRPaps %%Owner:rosiep */
InitSRPaps(fSearch)
BOOL fSearch;
{
	struct FB *pfb = (fSearch ? &vfbSearch : &vfbReplace);

	SetBytes(&(pfb->pap), 0, cbPAPS);
	SetBytes(&(pfb->papGray), -1, cbPAPS);
}


/* I N I T  S E A R C H  F O R M A T T I N G */
/* Initialize formatting block for Search */


/* %%Function:InitSearchFormatting %%Owner:rosiep */
InitSearchFormatting()
{
	vtmcFocus = tmcSearchNil;

	if (!vfbSearch.fInfo || (*hwwdCur)->wk == wkMacro)
		{
		InitSRChps(fTrue /* fSearch */);
		InitSRPaps(fTrue /* fSearch */);
		}
}


/*
*  D I S P L A Y  S  R  B A N T E R
*
*  Sets up the parameters to CchFillStWithBanter and calls it.  We start
*  with a base chp & pap that have zeros in fields we care about and
*  elsewhere are identical to the chp & pap associated with the edit
*  control in question, so that we are guaranteed to generate banter
*  text only for props that we care about.
*
*/


/* %%Function:DisplaySRBanter %%Owner:rosiep */
DisplaySRBanter(tmc, pchp, pchpGray, ppap, ppapGray)
int	tmc;
struct CHP *pchp, *pchpGray;
struct PAP *ppap, *ppapGray;
{
	char	stBanter[cchMaxReplaceBanter+1];
	int	cw, *pw, *pwBase, *pwGray, ibstFont;
	struct CHP chpBase;
	struct PAP papBase;
	CHAR sz[4];
	int	cch;
	BOOL fFormatted = fFalse;

	cw = cwCHP;
	pwBase = &chpBase;
	pw = pchp;
	pwGray = pchpGray;
	while (cw--)
		*pwBase++ = *pw++ & *pwGray++;

	cw = cwPAPS;
	pwBase = &papBase;
	pw = ppap;
	pwGray = ppapGray;
	while (cw--)
		*pwBase++ = *pw++ & *pwGray++;
	/* for the full-sized papBase, ensure that there are no differences from
		whatever happens to be out there in memory past the end of our
		short PAP */
	cw = cwPAP - cwPAPS;
	while (cw--)
		*pwBase++ = *pw++;

	papBase.stc = stcStdMin;

	/* handle items for which 0 is a valid value */

	if (!ppapGray->jc)
		/* fill in impossible value so we find a difference for jc */
		papBase.jc = 255;

	if (!pchpGray->ftc)
		/* force a difference for ftc only if non-gray */
		chpBase.ftc = (pchp->ftc ? 0 : 1);

	if (!pchpGray->ico)
		/* force a difference for ico only if non-gray */
		chpBase.ico = (pchp->ico ? 0 : 1);

	SetVdocStsh(selCur.doc);
	ibstFont = pchpGray->ftc ? ibstNil : 
			IbstFontFromFtcDoc(pchp->ftc, selCur.doc);
	CchFillStWithBanter(stBanter, (vfReplace ? cchMaxReplaceBanter : 
			cchMaxSearchBanter), pchp, &chpBase, ppap, &papBase, ibstFont);

	StToSzInPlace(stBanter);
	SetTmcText(tmc, stBanter);
	if (tmc == tmcSearchBanter)
		{
		/* only need to get the first few characters to see if it has any */
		/* have to get at least 4 due to bogosity in SDM */
		cch = CchGetTmcText(tmcSearch, sz, 4);
		EnableTmc(tmcOK, (stBanter[0] != 0) || cch > 0);
		}

	/* set dummy checkbox as flag to tell recorder if there is formatting */
	if (stBanter[0] > 0)
		fFormatted = fTrue;
	else  if (vfReplace) /* check other banter */
		fFormatted = (CchGetTmcText(tmc == tmcSearchBanter ? 
				tmcReplaceBanter : tmcSearchBanter, sz, 4) > 0);
	SetTmcVal(vfReplace ? tmcRFmt : tmcSFmt, fFormatted);
}


/* S E T  W O R D  S E A R C H */
/* turns off the word flag if search string contains non-word chars. */

/* %%Function:SetWordSearch %%Owner:rosiep */
SetWordSearch(szSearch)
char	*szSearch;
{
	while (*szSearch != '\0')
		if (WbFromCh(*szSearch++) != wbText)
			{
			SetTmcVal(tmcWholeWord, fFalse);
			break;
			}
}


/* F  F I N D  N E X T */

/* %%Function:FFindNext %%Owner:rosiep */
BOOL FFindNext(pbmib)
struct BMIB *pbmib;
{
	CP cpSearchLim;
	CP cpMatchFirst;
	CP cpMac = CpMacDoc(selCur.doc);
	struct CA caT;

	/** A note on nomenclature in backwards searching:
	
	For ease of translating the code to work backwards as well as forward,
	I have kept vcpSearchStart as the point in the doc where the search
	actually starts, even though it's really a cpLim of a range being
	searched.  Similarly, when searching backwards, cpSearchLim is the
	limit of the search, but as we're going backwards, this is the first
	character in the doc that we'd accept as a starting position for the
	match.  So the range of cp's that gets searched is as follows:
	
		if (vfFwd):     [vcpSearchStart, cpSearchLim)
		else:           [cpSearchLim, vcpSearchStart)
	
	It makes lots of sense if you think of backwards searching as a
	forward search through an exact mirror image of the doc, which is
	how the code treats it.
	
	rosiep
	
	**/
	if (pbmib->fNotPlain && pbmib->fChTableInPattern)
		return fFalse;

	if (vcpSearchLim != cpNil && vfNoWrap)
		cpSearchLim = vcpSearchLim;
	else
		cpSearchLim = vfFwd ? cpMac : cp0;

	if (vfFwd && vcpSearchStart == cp0 ||
		!vfFwd && vcpSearchStart >= CpMacDocEdit(selCur.doc))
		{
		vfNoWrap = fTrue;
		}
	StartLongOp();

LRestartSearch:
	/* do the appropriate search */

	Profile(vpfi == pfiCpSearchSz ? StartProf(30) : 0);
	/* searching for text (and possibly properties) */
	if (vfFwd)
		cpMatchFirst = CpSearchSz(pbmib, vcpSearchStart, cpSearchLim, NULL, NULL);
	else
		cpMatchFirst = CpSearchSzBackward(pbmib, cpSearchLim, vcpSearchStart);

	Profile(vpfi == pfiCpSearchSz ? StopProf() : 0);

	if (vcpLimWrapSave == cpNil) vcpLimWrapSave = vcpLimWrap;

	if (cpMatchFirst == cpNil)
		{
		EndLongOp( fFalse );
		if (FQueryAbortCheck())  /* user aborted search */
			return fFalse;

		if (vfNoWrap)
			{
			if (!vfFound && !fElActive)
				ErrorEid(eidNotFound, "FindNext");

			return fFalse;
			}
		/* check for wrapping around the end (or beginning, if backwards) of doc */
		if (vfSearchWholeDoc || 
				IdMessageBoxMstRgwMb(vfFwd ? mstSearchWrapFwd : mstSearchWrapBkwd,
				NULL, MB_YESNOQUESTION) == IDYES)
			{
			cpSearchLim = vfReplace ? vcpLimWrapSave : vcpLimWrap;
			vcpSearchStart = vfFwd ? cp0 : cpMac;
			vfNoWrap = fTrue;
			goto LRestartSearch;
			}
		else
			return fFalse;
		}

	/* check if it's a whole word */
	if ((vfSearchWord && !FWordCp(cpMatchFirst, vcpMatchLim - cpMatchFirst))
			|| (!vfFwd && vfbSearch.fText && 
			(vfbSearch.fChp && !FMatchChpCpCp(cpMatchFirst, vcpMatchLim)
			|| vfbSearch.fPap && !FMatchPapCp(cpMatchFirst))))
		{
		vcpSearchStart = (vfFwd ? CpLimSty(wwCur, selCur.doc, cpMatchFirst + 1, styChar, fFalse) : cpMatchFirst);
		goto LRestartSearch;
		}

	/* found it */
	vfFound = fTrue;
	EndLongOp(fFalse);
	AssureCpAboveDyp(cpMatchFirst,
			(vfReplace ? (((*hwwdCur)->ywMac - (*hwwdCur)->ywMin) / 3) : 
			(*hwwdCur)->ywMac), fFalse /* fEnd */);
	TurnOnSelCur();
	vcpMatchFirst = cpMatchFirst;
	MakeExtendSel(&selCur, vcpMatchFirst, vcpMatchLim, 0/*grpf*/);
	PushInsSelCur();
	return fTrue;
}


/*
*
*  F  S E T  U P  F O R  N E X T  S E A R C H
*
*  sets vcpSearchStart for next search
*  returns true if search pattern matches the selection exactly
*
*/


/* %%Function:FSetUpForNextSearch %%Owner:rosiep */
BOOL FSetUpForNextSearch(pbmib, fIgnoreMatch)
struct BMIB *pbmib;
BOOL fIgnoreMatch;
{
	BOOL fChp, fPap, fMatchSel = fFalse;
	CP cpT;

	if (selCur.cpLim > CpMacDoc(selCur.doc))
		Select(&selCur, selCur.cpFirst, CpMacDoc(selCur.doc));

	cpT = (selCur.cpLim > CpMacDocEdit(selCur.doc)) ? 
			CpMacDocEdit(selCur.doc) : selCur.cpLim;

	if (vfExtendSel)
		{
		vcpSearchStart = (selCur.fForward ? selCur.cpLim : selCur.cpFirst);
		return fFalse;
		}
	else  if (!vfbSearch.fText && vfbSearch.fChp)
		{   /* no text, but character properties */
		if (!vfFwd)
			vcpSearchStart = selCur.cpFirst;
		else  if (FMatchChpCpCp(selCur.cpFirst, cpT /*selCur.cpLim*/) && 
				!fIgnoreMatch)
			{
			vcpSearchStart = selCur.cpLim;
			fMatchSel = fTrue;
			}
		else
			vcpSearchStart = selCur.cpFirst;
		return fMatchSel;
		}
	else  if (!vfbSearch.fText)
		{  /* searching for paragraph properties only */
		Assert (vfbSearch.fPap);
		CachePara(selCur.doc, selCur.cpFirst);
		if (selCur.fIns)
			{
			if (caPara.cpFirst == selCur.cpFirst || selCur.cpFirst == cp0 || 
					selCur.cpFirst == CpMacDocEdit(selCur.doc))
				{
				vcpSearchStart = selCur.cpFirst;
				}
			else
				vcpSearchStart = (vfFwd ? caPara.cpLim : caPara.cpFirst);
			}
		else
			{
			/* if current selection is exactly one paragraph */
			if (vfFwd && caPara.cpFirst == selCur.cpFirst && 
					caPara.cpLim == selCur.cpLim && !fIgnoreMatch)
				{
				vcpSearchStart = caPara.cpLim;
				fMatchSel = fTrue;
				}
			else
				vcpSearchStart = caPara.cpFirst;
			}

		return fMatchSel;
		}
	fChp = vfbSearch.fChp;
	fPap = vfbSearch.fPap;

	/* so CpSearchSz won't use properties as a search criterion */
	vfbSearch.fChp = vfbSearch.fPap = fFalse;

	vcpSearchStart = (vfFwd ? selCur.cpFirst : selCur.cpLim);
	if (!selCur.fIns)
		{
		/* if current selection matches search string exactly */
		Assert (vfbSearch.fText);
		/* if we call CpSearchSz(Backward) while vfbSearch.fText is true
		and nothing is found, it cpNil will be returned, and vcpMatchLim
		will be unaltered.  This is what we want to simulate when
		pbmib->fNotPlain && pbmib->fChTableInPattern. */
		vcpMatchFirst = cpNil;
		if (!pbmib->fNotPlain || !pbmib->fChTableInPattern)
			vcpMatchFirst = (vfFwd ? 
					CpSearchSz(pbmib, selCur.cpFirst, selCur.cpLim, NULL, NULL) : 
					CpSearchSzBackward(pbmib, selCur.cpFirst, selCur.cpLim));
		if (vcpMatchFirst == selCur.cpFirst && selCur.cpLim == vcpMatchLim)
			{
			fMatchSel = fTrue;
			if (vfReplace)
				vcpSearchStart = (fIgnoreMatch ? selCur.cpFirst : selCur.cpLim);
			else

				/* Suppose we have the search text "ababab" with the first
				four characters selected, and we are searching for "abab".
				Assuming the first four characters are selected due to the
				previous search and we are performing a search again, then
				we want to make sure that we get a different pattern which
				is why we perform the vcpSearchStart++ below.
				
				But now suppose we have the search text "  l" with all three
				characters selected, and we search for "^wl", then search
				will find that the second space and the l match the pattern,
				so we will not really be finding a new pattern.
				
				It seems to me a good FUTURE question if we want to do
				anything about this. (BradV) */

				{
				if (vfFwd)
					vcpSearchStart = CpLimSty(selCur.ww, selCur.doc,
							vcpSearchStart, styChar, fFalse);
				else
					vcpSearchStart = CpFirstSty(selCur.ww, selCur.doc,
							vcpSearchStart, styChar, fTrue);
				}
			}
		}
	vfbSearch.fChp = fChp;
	vfbSearch.fPap = fPap;
	return (fMatchSel);
}



#ifdef DEBUG

/* %%Function:CpSearchSz %%Owner:rosiep */
HANDNATIVE CP C_CpSearchSz (pbmib, cpFirst, cpLim, pcpNextRpt, hppr)
struct BMIB *pbmib;
CP      cpFirst;
CP      cpLim;
CP      *pcpNextRpt;
struct PPR **hppr;
{
	/*  *** CpSearchSz ***

	Finds the next occurrence of *pwPatFirst in the current document
	starting at cpFirst. Ignores case of letters if vfSearchCase is FALSE.
	
	This search procedure uses the Boyer/Moore algorithm: *pwPatFirst is
	compared against the document from end to front - when a mismatch
	occurs, the search jumps ahead mpchdcp[chDoc] CP's, where mpchdcp is
	an array storing the minimum # of characters between the last char
	and any occurrence of chDoc in *pwPatFirst. If there are no such
	occurrences, we can jump the length of *pwPatFirst.
	
	Returns the first cp of the text match in the document; cpNil if
	not found.
	
	Note: vcpMatchLim is undefined whenever cpNil is returned.
	*/

	int	*pwPatFirst = pbmib->rgwSearch;  /* local for speed */
	char	*mpchdcp = pbmib->mpchdcp;       /* local for speed */
	int	chDoc;
	char	HUGE  *hpchDoc;
	BOOL   fMatchedWhite;
	int	cchMatched;
	CP     cpRun, cpMatchStart;
	int	ccpFetch;  /* local instead of vccpFetch */
	int	ichDoc, cchT;
	int	wPat, *pwPat;
	int	ccpRun, ccpRemain;
	int	dwrgwPat;
	int	ichPat;
	int	*pwPatLast;
	BOOL   fMatchFoundNonText = fFalse;
	CP	   cpFirstInit = cpFirst;
	struct CA caT;
	BOOL   fTableFirst, fTableLim;
	CP     cpLimOrig = cpLim;
	BOOL   fTableCur;

	Assert (!pbmib->fNotPlain || !pbmib->fChTableInPattern);
	dwrgwPat = ((int)pbmib->rgwOppCase - (int)pwPatFirst) / sizeof(int);
	fMatchedWhite = fFalse;
	pwPatLast = &pwPatFirst[pbmib->cwSearch - 1];
	cpRun = cpFirst;
	vcpLimWrap = CpMin(cpFirst + pbmib->cwSearch - 1, cpLim);
	goto LFResetSearch;

LFCharLoop:

	if (cchMatched != 0)
		{
		/* do things the slow way */
LFSlowCharLoop:
		/* The character in the document matches if :
				the search pattern char is chMatchAny
			OR the search pattern char is chMatchWhite
				AND chDoc is a white-space char
			OR chDoc matches the pattern char exactly
			OR the search pattern char is a space
				AND chDoc is a non-breaking space
			OR the search pattern char is a hyphen
				AND chDoc is a non-breaking hyphen
			OR chDoc is a non-required hyphen
			OR the search is NOT case sensitive
				AND it matches the pattern char of opposite case
				OR (international version only) its corresponding
					upper-case char matches the pattern char (to contend
					with accented chars: want e(acute) to match E).  */

		chDoc = *hpchDoc;
		if ((wPat = *pwPat) == chDoc)
			goto LFCharMatch;
		if (chDoc == chNonReqHyphen || chDoc == chReturn)
			{
			cchMatched++;
			goto LFBackUp;
			}
		if (wPat == wMatchAny && chDoc != chTable)
			goto LFCharMatch;
		if (wPat == wMatchWhite && FMatchWhiteSpace(chDoc))
			{
			cchMatched++;
			fMatchedWhite = fTrue;
			goto LFBackUp;
			}
		if (wPat == chSpace && chDoc == chNonBreakSpace)
			goto LFCharMatch;
		if (wPat == chHyphen && chDoc == chNonBreakHyphen)
			goto LFCharMatch;
		if (!vfSearchCase)
			{
			if (chDoc == *(pwPat + dwrgwPat))
				goto LFCharMatch;
			/* if it failed all above, and could need special intl
					upper casing, try ChUpper
				*/
			if (chDoc > 127)
				if (ChUpper(chDoc) == wPat)
					goto LFCharMatch;
			}
		if (fMatchedWhite)
			{        /* end of matched white space, may still have match    */
			/* compare next chPat against SAME chDoc   */
			if (--pwPat < pwPatFirst)
				{   /* a match has been found   */
				hpchDoc++;       /* next char was start of white space   */
				goto LFMatchFound;
				}
			fMatchedWhite = fFalse;
			goto LFCheckRemain;
			}

		fMatchedWhite = fFalse;
		cchT = max(cchMatched + 1, mpchdcp[chDoc]);
		cchMatched = 0;
		pwPat = pwPatLast;
		hpchDoc += cchT;
		ccpRemain -= cchT;
		goto LFCheckRemain;
		}
	else
		{
		/* do things the fast way */
		do
			{
			chDoc = *hpchDoc;
			if ((cchT = mpchdcp[chDoc]) == 0)
				goto LFSlowCharLoop;
			hpchDoc += cchT;
			}
		while ((ccpRemain -= cchT) > 0);
		}
LFCheckRemain:
	if (ccpRemain > 0)
		goto LFCharLoop;
	goto LFFetch;

LFCharMatch:
	/* chDoc is a match   */
	cchMatched++;
	if (--pwPat < pwPatFirst)
		goto LFMatchFound;

LFBackUp:
	hpchDoc--;
	ccpRemain++;
	if (ccpRemain <= ccpRun)
		goto LFCheckRemain;
	/* string compare backed over fetch boundary, fetch previous run */
	ichPat = ((int)pwPat - (int)pwPatFirst) / sizeof(int);
	ichDoc = max(ichPat - 1, 0);
	cpRun = vcpFetch - (ichDoc + 1);
	if (cpRun < cpFirst)
		{
		if (pwPat == pwPatFirst && *pwPat == wMatchWhite)
			{
			hpchDoc = vhpchFetch;
			goto LFMatchFound;
			}
		else
			/* hit cpFirst before matching all of word:  */
			{	/* look beyond the matched string */
			ichDoc = cchMatched;
			cpRun = vcpFetch;
			if (cpFirst == cpFirstInit)
				vcpLimWrap = vcpFetch + cchMatched;
			pwPat = pwPatLast;
			cchMatched = 0;
			}
		}

	FetchCpForSearch(selCur.doc, cpRun, &ccpFetch,
			selCur.ww/*fvcScreen*/, pbmib->fNotPlain);
	Assert(ccpFetch > 0);
	Assert(vcpFetch == cpRun); /* there can't have been any
	invisible chars skipped because we bumped cpFirst up
	past them already */
	goto LFAfterFetch;

LFFetch:
	ichDoc = (int)(hpchDoc - vhpchFetch);

LFFetchAgain:
	ichDoc -= ccpRun;
	cpRun += ccpFetch;
LFFetchAfterReset:
	/* Need more cp's */
	Assert (ichDoc >= 0);
	if (cpRun + ichDoc >= cpLim)
		{
		if (fMatchFoundNonText)
			goto LFDone;
		return cpNil;
		}
	/* if any user input (mouse, keyboard) check for abort */
	if (FQueryAbortCheck())
		return cpNil;

	/* report progress */
	if (hppr != hNil && cpRun >= *pcpNextRpt)
		ProgressReportPercent(hppr, vprri->cpLowRpt, vprri->cpHighRpt, cpRun, pcpNextRpt);
	FetchCpForSearch(selCur.doc, cpRun, &ccpFetch, selCur.ww/*fvcScreen*/,
		pbmib->fNotPlain);
	if (ccpFetch == 0)
		{
		if (fMatchFoundNonText)
			goto LFDone;
		return cpNil;
		}
	Assert(vcpFetch >= cpRun);
	if (vfbSearch.fPap && !C_FMatchPap() && vcpFetch + ichDoc < cpLim)
		{
		cpRun = caPara.cpLim;
		goto LFResetSearch;
		}
	if (vfbSearch.fChp && !C_FMatchChp() && vcpFetch + ichDoc < cpLim)
		{
		cpRun = vcpFetch + ccpFetch;
		goto LFResetSearch;
		}
	if (fMatchFoundNonText && (!fTableCur != !vpapFetch.fInTable))
		goto LFDone;
	/* there were invisible chars skipped */
	if (vcpFetch > cpRun && vfbSearch.fText)
		{ /* start over at beginning of pattern */
		cpRun = vcpFetch;
		goto LFResetSearch;
		}
LFAfterFetch:
	/* Prevent overflow of cchMatched and other variables if
	   someone has a crazy document with a zillion contigous
	   whitespace characters. */
	if (cchMatched > 0x7FFF - 1 - cbSector)
		return cpNil;
	Assert(cpLim >= cpRun);
	ccpRun = (int)CpMin(cpLim - cpRun, (CP)ccpFetch);
	if (!vfbSearch.fText)
		{
		if (!fMatchFoundNonText)
			{
			fMatchFoundNonText = fTrue;
			fTableCur = vpapFetch.fInTable;
			cpMatchStart = vcpFetch;
			/* stop at para boundary unless doing non-confirmed replace */
			/* allows user to confirm paragraph at a time in a run of
				like-formatted paragraphs */
			if (!vfReplace || vfConfirm)
				cpLim = caPara.cpLim;
			}
		vcpMatchLim = vcpFetch + ccpRun;
		ichDoc = ccpRun;
		goto LFFetchAgain;
		}
	ccpRemain = ccpRun - ichDoc;
	hpchDoc = vhpchFetch + ichDoc;
	goto LFCheckRemain;

LFResetSearch:
	if (fMatchFoundNonText)
		goto LFDone;
	cpFirst = cpRun;
	cpLim = cpLimOrig;
	ichDoc = pbmib->cwSearch - 1;
	pwPat = pwPatLast;
	cchMatched = 0;
	goto LFFetchAfterReset;

LFMatchFound:
	ichDoc = (int)(hpchDoc - vhpchFetch);
	cpMatchStart = vcpFetch + ichDoc;
	vcpMatchLim = vcpFetch + ichDoc + cchMatched;
	if (*pwPatLast == wMatchWhite)
		{           /* look for trailing white space characters */
		while (vcpMatchLim < cpLim)
			{
			FetchCpForSearch(selCur.doc, vcpMatchLim, &ccpFetch,
					selCur.ww/*fvcScreen*/, pbmib->fNotPlain);
			if (ccpFetch == 0)
				goto LFDone;
			Assert(vcpFetch >= vcpMatchLim);
			if (vcpFetch > vcpMatchLim) /* check for skipped invisible chars */
				goto LFDone;
			if (vfbSearch.fChp && !C_FMatchChp())
				goto LFDone;
			if (vfbSearch.fPap && !C_FMatchPap())
				goto LFDone;
			Assert(cpLim >= vcpFetch);
			/* Add one to compensate for first decrement below. */
			ccpRemain = (int)CpMin(cpLim - vcpFetch, (CP)ccpFetch) + 1;
			hpchDoc = vhpchFetch;
			while (--ccpRemain > 0 && FMatchWhiteSpace(*hpchDoc++));
			vcpMatchLim = vcpFetch + ((uns)(hpchDoc - vhpchFetch));
			if (ccpRemain != 0)
				{
				Assert(ccpRemain > 0);
				vcpMatchLim--;
				break;
				}
			}
		}

LFDone:
	AssureLegalSel(PcaSet(&caT, selCur.doc, cpMatchStart, vcpMatchLim));
	/* check for spanning across table cells, not allowed */
	fTableFirst = FInTableDocCp(caT.doc, caT.cpFirst);
	fTableLim = FInTableDocCp(caT.doc, caT.cpLim - 1);
	if (fTableFirst || fTableLim)
		{
		if (!(fTableFirst && fTableLim))
			{
			cpRun = vfbSearch.fText ? cpMatchStart+1 : caT.cpLim;
			fMatchFoundNonText = fFalse;
			goto LFResetSearch;
			}
		CacheTc(selCur.ww, caT.doc, caT.cpFirst, fFalse, fFalse);
		if (caT.cpLim >= vtcc.cpLim)
			{
			if (vfbSearch.fText || caT.cpFirst == vtcc.cpLim - ccpEop)
				{
				cpRun = vtcc.cpLim;
				fMatchFoundNonText = fFalse;
				goto LFResetSearch;
				}
			else
				caT.cpLim = vtcc.cpLim - ccpEop;
			}
		}
	cpMatchStart = caT.cpFirst;
	vcpMatchLim = caT.cpLim;
	return cpMatchStart;
}


#endif /* DEBUG */

#ifdef DEBUG

/* %%Function:CpSearchSzBackward %%Owner:rosiep */
HANDNATIVE CP C_CpSearchSzBackward (pbmib, cpFirst, cpLim)
struct BMIB *pbmib;
CP    cpFirst;
CP    cpLim;
{
	/*  *** CpSearchSzBackward ***

	Same as CpSearchSz, but searches backward.  Uses the Boyer/Moore
	algorithm in reverse.
	
	Returns the first cp of the text match in the document; cpNil if
	not found.
	
	Note: vcpMatchLim is undefined whenever cpNil is returned.
	*/

	int	*pwPatFirst = pbmib->rgwSearch;  /* local for speed */
	char	*mpchdcp = pbmib->mpchdcp;       /* local for speed */
	int	chDoc;
	char	HUGE     *hpchDoc;
	BOOL    fMatchedWhite;
	int	cchMatched;
	CP    cpRun, cpMatchStart;
	int	ccpFetch;                /* local instead of vccpFetch */
	int	ichDoc, cchT;
	int	wPat, *pwPat;
	int	ccpRun, ccpRemain;
	int	dwrgwPat;
	int	ichPat;
	int	*pwPatLast;
	CP	   cpLimInit = cpLim;
	BOOL   fMatchFoundNonText = fFalse;
	struct CA caT;
	int	icpFetchMac;            /* number of entries in rgcpFetch */
	CP	   rgcpFetch[128];    /* record of beginnings of visible runs */
	BOOL   fTableFirst, fTableLim;
	CP     cpFirstOrig = cpFirst;

	Assert (!pbmib->fNotPlain || !pbmib->fChTableInPattern);
	dwrgwPat = ((int)pbmib->rgwOppCase - (int)pwPatFirst) / sizeof(int);
	fMatchedWhite = fFalse;
	pwPatLast = &pwPatFirst[pbmib->cwSearch - 1];
	cpRun = cpLim;
	vcpLimWrap = CpMax(cpLim - pbmib->cwSearch + 1, cpFirst);
	goto LBResetSearch;

LBCharLoop:
	if (cchMatched != 0)
		{
		/* do things the slow way */
LBSlowCharLoop:
		/* The character in the document matches if :
				the search pattern char is chMatchAny
			OR the search pattern char is chMatchWhite
				AND chDoc is a white-space char
			OR chDoc matches the pattern char exactly
			OR the search pattern char is a space
				AND chDoc is a non-breaking space
			OR the search pattern char is a hyphen
				AND chDoc is a non-breaking hyphen
			OR chDoc is a non-required hyphen
			OR the search is NOT case sensitive
				AND it matches the pattern char of opposite case
				OR (international version only) its corresponding
					upper-case char matches the pattern char (to contend
					with accented chars: want e(acute) to match E).  */

		chDoc = *hpchDoc;
		if ((wPat = *pwPat) == chDoc)
			goto LBCharMatch;
		if (chDoc == chNonReqHyphen || chDoc == chReturn)
			{
			cchMatched++;
			goto LBBackUp;
			}
		if (wPat == wMatchAny && chDoc != chTable)
			goto LBCharMatch;
		if (wPat == wMatchWhite && FMatchWhiteSpace(chDoc))
			{
			cchMatched++;
			fMatchedWhite = fTrue;
			goto LBBackUp;
			}
		if (wPat == chSpace && chDoc == chNonBreakSpace)
			goto LBCharMatch;
		if (wPat == chHyphen && chDoc == chNonBreakHyphen)
			goto LBCharMatch;
		if (!vfSearchCase)
			{
			if (chDoc == *(pwPat + dwrgwPat))
				goto LBCharMatch;
			/* if it failed all above, and could need special intl
					upper casing, try ChUpper
				*/
			if (chDoc > 127)
				if (ChUpper(chDoc) == wPat)
					goto LBCharMatch;
			}
		if (fMatchedWhite)
			{        /* end of matched white space, may still have match */
			/* compare next chPat against SAME chDoc */
			if (++pwPat > pwPatLast)
				{    /* a match has been found */
				goto LBMatchFound;
				}
			fMatchedWhite = fFalse;
			goto LBCheckRemain;
			}

		fMatchedWhite = fFalse;
		cchT = max(cchMatched + 1, mpchdcp[chDoc]);
		cchMatched = 0;
		pwPat = pwPatFirst;
		hpchDoc -= cchT;
		ccpRemain -= cchT;
		goto LBCheckRemain;
		}
	else
		{
		/* do things the fast way */
		do
			{
			chDoc = *hpchDoc;
			if ((cchT = mpchdcp[chDoc]) == 0)
				goto LBSlowCharLoop;
			hpchDoc -= cchT;
			}
		while ((ccpRemain -= cchT) > 0);
		}
LBCheckRemain:
	if (ccpRemain > 0)
		goto LBCharLoop;
	goto LBFetch;

LBCharMatch:
	/* chDoc is a match */
	cchMatched++;
	if (++pwPat > pwPatLast)
		{
		hpchDoc++;    /* advance hpch to get the Lim */
		goto LBMatchFound;
		}

LBBackUp:
	hpchDoc++;
	ccpRemain++;
	if (ccpRemain <= ccpRun)
		goto LBCheckRemain;
	/* string compare advanced over fetch boundary, fetch next run */
	ichPat = ((int)pwPat - (int)pwPatFirst) / sizeof(int);
	cpRun = vcpFetch + ccpFetch;
	ichDoc = 0;
	if (cpRun + (pbmib->cwSearch - ichPat) > cpLim)
		{
		if (pwPat == pwPatLast && *pwPat == wMatchWhite)
			{
			hpchDoc = vhpchFetch;
			goto LBMatchFound;
			}
		else
			/* hit cpLim before matching all of word:  */
			{  /* look before the matched string */
			/* We must invalidate rgcpFetch here because we have
			no guarantee that we will be fetching the piece just after
			the one starting at rgcpFetch[icpFetchMac - 1]. */
			icpFetchMac = 0;
			if (cpLim == cpLimInit)
				vcpLimWrap = cpRun - cchMatched;
			ichDoc = -cchMatched - 1;
			pwPat = pwPatFirst;
			cchMatched = 0;
			goto LBFetchAfterReset;
			}
		}
	FetchCpForSearch(selCur.doc, cpRun, &ccpFetch,
			selCur.ww/*fvcScreen*/, pbmib->fNotPlain);
	Assert(ccpFetch > 0);
	Assert(vcpFetch == cpRun);    /* there can't have been any
	invisible chars skipped because we bumped cpLim down past them
	already. */
	Assert(icpFetchMac >= 0);
	Debug(CheckRgcpFetch(selCur.ww, &ccpFetch, pbmib->fNotPlain,
			&icpFetchMac, rgcpFetch));
	if ((long)ccpFetch > cpLim - vcpFetch)
		ccpFetch = (int)(cpLim - vcpFetch);
	if (icpFetchMac < 128)
		rgcpFetch[icpFetchMac++] = vcpFetch;
	else
		{
		Assert(icpFetchMac == 128);
		blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP) * (128 - 1));
		rgcpFetch[128 - 1] = vcpFetch;
		}
	goto LBAfterFetch;

LBFetch:
	ichDoc = (int)(hpchDoc - vhpchFetch);
LBFetchAgain:
LBFetchAfterReset:
	/* Need more cp's */
	if (cpRun + ichDoc < cpFirst)
		{
		if (fMatchFoundNonText)
			goto LBDone;
		return cpNil;
		}
	Assert (ichDoc < 0);
	/* if any user input (mouse, keyboard) check for abort */
	if (FQueryAbortCheck())
		return cpNil;
	FetchCpPccpVisibleBackward(selCur.doc, cpRun,
			&ccpFetch, selCur.ww, pbmib->fNotPlain, &icpFetchMac, rgcpFetch);
	if (ccpFetch == 0)
		{
		if (fMatchFoundNonText)
			goto LBDone;
		return cpNil;
		}
	if ((long)ccpFetch > cpRun - vcpFetch)
		ccpFetch = (int)(cpRun - vcpFetch);
	ichDoc += (int)(cpRun - vcpFetch);
	if (vfbSearch.fPap && !FMatchPap() && vcpFetch + ichDoc >= cpFirst)
		{
		cpRun = caPara.cpFirst;
		goto LBResetSearch;
		}
	if (vfbSearch.fChp && !FMatchChp() && vcpFetch + ichDoc >= cpFirst)
		{
		cpRun = vcpFetch;
		goto LBResetSearch;
		}
	/* there were invisible chars skipped */
	if (vcpFetch + ccpFetch < cpRun && vfbSearch.fText)
		{ /* start from the beginning of pattern */
		cpRun = vcpFetch + ccpFetch;
		goto LBResetSearch;
		}
	cpRun = vcpFetch;
LBAfterFetch:
	/* Prevent overflow of cchMatched and other variables if
	   someone has a crazy document with a zillion contigous
	   whitespace characters. */
	if (cchMatched > 0x7FFF - 1 - cbSector)
		return cpNil;
	Assert(vcpFetch + ccpFetch > cpFirst);
	ccpRun = (int)CpMin(vcpFetch + ccpFetch - cpFirst, (CP)ccpFetch);
	if (!vfbSearch.fText)
		{
		if (!fMatchFoundNonText)
			{
			fMatchFoundNonText = fTrue;
			vcpMatchLim = vcpFetch + ccpFetch;
			/* stop at para boundary unless doing non-confirmed replace */
			/* allows user to confirm paragraph at a time in a run of
				like-formatted paragraphs */
			if (!vfReplace || vfConfirm)
				cpFirst = caPara.cpFirst;
			}
		cpMatchStart = vcpFetch + ccpFetch - ccpRun;
		ichDoc = -1;
		goto LBFetchAgain;
		}
	/* Add one here in the backward case because we want to include
	the character at cpFirst in the characters we look at.  In the
	forward case we do not want to include the character at cpLim. */
	ccpRemain = ichDoc - (ccpFetch - ccpRun) + 1;
	hpchDoc = vhpchFetch + ichDoc;
	goto LBCheckRemain;

LBResetSearch:
	if (fMatchFoundNonText)
		goto LBDone;
	/* We might as well invalidate rgcpFetch since we know anything
	in it will be useless. */
	icpFetchMac = 0;
	cpLim = cpRun;
	cpFirst = cpFirstOrig;
	ichDoc = -pbmib->cwSearch;
	pwPat = pwPatFirst;
	cchMatched = 0;
	goto LBFetchAfterReset;

LBMatchFound:
	ichDoc = (int)(hpchDoc - vhpchFetch);
	cpMatchStart = cpRun + ichDoc - cchMatched;
	vcpMatchLim = cpRun + ichDoc;

	if (*pwPatFirst == wMatchWhite)
		{           /* look for leading white space characters   */
		while (cpMatchStart > cpFirst)
			{
			FetchCpPccpVisibleBackward(selCur.doc, cpMatchStart,
					&ccpFetch, selCur.ww, pbmib->fNotPlain, &icpFetchMac, rgcpFetch);
			if (ccpFetch == 0)
				goto LBDone;
			/* check for skipped invisible chars */
			if (vcpFetch + ccpFetch < cpMatchStart)
				goto LBDone;
			if (vfbSearch.fChp && !FMatchChp())
				goto LBDone;
			if (vfbSearch.fPap && !FMatchPap())
				goto LBDone;
			/* Add one to compensate for first decrement below. */
			ccpRemain = (int)(cpMatchStart - CpMax(cpFirst, vcpFetch)) + 1;
			hpchDoc = vhpchFetch + (int)(cpMatchStart - vcpFetch);
			while (--ccpRemain > 0 && FMatchWhiteSpace(*(--hpchDoc)));
			cpMatchStart = vcpFetch
					+ ((uns)(hpchDoc - vhpchFetch));
			if (ccpRemain != 0)
				{
				Assert(ccpRemain > 0);
				cpMatchStart++;
				break;
				}
			}
		}

LBDone:
	AssureLegalSel(PcaSet(&caT, selCur.doc, cpMatchStart, vcpMatchLim));
	/* check for spanning across table cells, not allowed */
	fTableFirst = FInTableDocCp(caT.doc, caT.cpFirst);
	fTableLim = FInTableDocCp(caT.doc, caT.cpLim - 1);
	if (fTableFirst || fTableLim)
		{
		if (!(fTableFirst && fTableLim))
			{
			cpRun = vfbSearch.fText ? vcpMatchLim-1 : caT.cpFirst;
			fMatchFoundNonText = fFalse;
			goto LBResetSearch;
			}
		CacheTc(selCur.ww, caT.doc, caT.cpLim - 1, fFalse, fFalse);
		if (caT.cpFirst < vtcc.cpFirst)
			{
			if (vfbSearch.fText || caT.cpFirst == vtcc.cpLim - ccpEop)
				{
LBResetTable:
				cpRun = vtcc.cpFirst;
				fMatchFoundNonText = fFalse;
				goto LBResetSearch;
				}
			else
				caT.cpFirst = vtcc.cpFirst;
			}
		if (caT.cpLim == vtcc.cpLim)
			{
			if (vfbSearch.fText)
				goto LBResetTable;
			caT.cpLim -= ccpEop;
			}
		}
	cpMatchStart = caT.cpFirst;
	vcpMatchLim = caT.cpLim;
	return cpMatchStart;
}


#endif /* DEBUG */



#ifdef DEBUG
/* FetchCpPccpVisibleBackward
*
*  Mock mirror image of FetchCpPccpVisible.  Used for searching
*  backwards, and I can't think of any other uses for it at the
*  moment.
*
*  We want to fetch the last visible run which begins prior to cpRun.
*
*  For maximum speed we are going to start looking cbSector characters
*  before cpRun.  In a long unedited document the run would typically
*  start here and we will spend little time in this routine.
*
*  The other case which might take some time is if we have a lot
*  of short little runs.  The algorithm is to fetch runs starting
*  cbSector CP's before the beginning of previously fetched run.
*  we then sequentially look at successive runs following this point
*  until we get the last one that begins before the beginning of the
*  previously fetched run.  In a region with many little runs this
*  would mean on average many fetches would be performed for each
*  call to this routine.  To avoid this problem rgcpFetch is kept
*  around to record beginnings of fetched runs, and this record
*  is used whenever possible to find the beginning of the desired
*  run.  This optimization means that in regions of many
*  little runs we should average about two fetches per call to this
*  routine.
*
*  Care is taken to ensure that rgcpFetch contains only true
*  beginnings of runs.  That is, if we fetch before CP for any CP
*  in rgcpFetch, vcpFetch will be either CP or vcpFetch + *pccpFetch
*  will be less than or equal to CP.
*
*  If no visible run is fetched prior to cpLimRun then *pccpFetch is
*  set to zero before returning.
*/



/* %%Function:FetchCpPccpVisibleBackward %%Owner:rosiep */
HANDNATIVE FetchCpPccpVisibleBackward(doc, cpLimRun,
pccpFetch, ww, fNotPlain, picpFetchMac, rgcpFetch)
int	doc;
CP    cpLimRun;
int	*pccpFetch;
int	ww;
BOOL	fNotPlain;
int	*picpFetchMac;
CP    rgcpFetch[];
{
	CP    cpCur;
	BOOL  fRunStartFetched;
#ifdef DEBUG
	int	cBeenHere;
	CP    cpFetchSav;
#endif /* DEBUG */

	Debug(cBeenHere = 0);
	if (cpLimRun <= 0)
		{
		*pccpFetch = 0;
		return;
		}
LRestart:
	Assert(*picpFetchMac >= 0);
	if (*picpFetchMac > 0)
		/* table of beginning of runs is not empty.
			Take one of the runs from the table. */
		{
		if ((cpCur = rgcpFetch[--(*picpFetchMac)]) >= cpLimRun)
			goto LRestart;
		(*picpFetchMac)++;
		FetchCpForSearch(doc, cpCur, pccpFetch,
				ww/*fvcScreen*/, fNotPlain);
#ifdef DEBUG
		FetchCpForSearch(doc, vcpFetch + *pccpFetch,
				pccpFetch, ww/*fvcScreen*/, fNotPlain);
		Assert(*pccpFetch == 0 || vcpFetch >= cpLimRun);
		FetchCpForSearch(doc, cpCur, pccpFetch,
				ww/*fvcScreen*/, fNotPlain);
#endif /* DEBUG */
		Assert(*pccpFetch <= cbSector);
		return;
		}
	cpCur = cpLimRun;
LContinue:
	while (cpCur > cp0)
		{
		/* Try looking for a run beginning 1 sector before the
			last attempt. */
		cpCur = CpMax(cpCur - cbSector, cp0);
		FetchCpForSearch(doc, cpCur, pccpFetch, ww/*fvcScreen*/, fNotPlain);
		Assert(*pccpFetch <= cbSector);
		if (vcpFetch < cpLimRun)
			{
			Assert(cBeenHere++ < 2);
			if (vcpFetch + *pccpFetch >= cpLimRun)
				return;
			/* We know we have the true beginning of a run only if
			vcpFetch > cpCur or if cpCur == cp0. */
			while (vcpFetch + *pccpFetch < cpLimRun)
				{
				if (fRunStartFetched = (vcpFetch > cpCur || cpCur == cp0))
					{
					Debug(CheckRgcpFetch(ww, pccpFetch, fNotPlain,
							picpFetchMac, rgcpFetch));
					if (*picpFetchMac < 128)
						rgcpFetch[(*picpFetchMac)++] = vcpFetch;
					else
						{
						Assert(*picpFetchMac == 128);
						blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP) * (128 - 1));
						rgcpFetch[128 - 1] = vcpFetch;
						}
					}
				Debug(cpFetchSav = vcpFetch + *pccpFetch);
				FetchCpForSearch(doc, vcpFetch + *pccpFetch,
						pccpFetch, ww/*fvcScreen*/, fNotPlain);
				Assert(*pccpFetch <= cbSector);
				Assert(vcpFetch >= cpFetchSav);
				if (*pccpFetch == 0 || vcpFetch >= cpLimRun)
					{
					if (fRunStartFetched)
						goto LRestart;
					goto LContinue;
					}
				}
			Debug(CheckRgcpFetch(ww, pccpFetch, fNotPlain,
					picpFetchMac, rgcpFetch));
			if (*picpFetchMac < 128)
				rgcpFetch[(*picpFetchMac)++] = vcpFetch;
			else
				{
				Assert(*picpFetchMac == 128);
				blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP) * (128 - 1));
				rgcpFetch[128 - 1] = vcpFetch;
				}
			return;
			}
		}
	*pccpFetch = 0;
}


#endif /* DEBUG */


#ifdef DEBUG

/* %%Function:CheckRgcpFetch %%Owner:rosiep */
HANDNATIVE CheckRgcpFetch(ww, pccpFetch, fNotPlain, picpFetchMac, rgcpFetch)
int	ww;
int	*pccpFetch;
BOOL	fNotPlain;
int	*picpFetchMac;
CP    rgcpFetch[];
{
	CP cpFetchSav;
	int	doc = selCur.doc;

	if (*picpFetchMac > 0)
		{
		cpFetchSav = vcpFetch;
		FetchCpForSearch(doc, rgcpFetch[*picpFetchMac - 1],
				pccpFetch, ww/*fvcScreen*/, fNotPlain);
		Assert(vcpFetch + *pccpFetch <= cpFetchSav);
		if (vcpFetch + *pccpFetch < cpFetchSav)
			{
			FetchCpForSearch(doc, vcpFetch + *pccpFetch,
					pccpFetch, ww/*fvcScreen*/, fNotPlain);
			Assert(vcpFetch == cpFetchSav);
			}
		FetchCpForSearch(doc, cpFetchSav,
				pccpFetch, ww/*fvcScreen*/, fNotPlain);
		}
}


#endif /* DEBUG */



#ifdef DEBUG /* A near subroutine in searchn.asm if !DEBUG */

/* %%Function:FMatchWhiteSpace %%Owner:rosiep */
HANDNATIVE BOOL FMatchWhiteSpace (ch)
int	ch;
{
	switch (ch)
		{
	case chSpace:
	case chNonBreakSpace:
	case chTab:
		return fTrue;

	default:
		return fFalse;
		}
}


#endif /* DEBUG */


#ifdef DEBUG

/* %%Function:FetchCpForSearch %%Owner:rosiep */
HANDNATIVE FetchCpForSearch(doc, cpRun, pccpFetch, ww, fNotPlain)
int	doc;
CP	cpRun;
int	*pccpFetch;
int	ww;
BOOL	fNotPlain;

{
	if (fNotPlain)
		FetchCpPccpVisible(doc, cpRun, pccpFetch, ww, fFalse /* fNested */);
	else
		{
		FetchCp(doc, cpRun, fcmChars);
		*pccpFetch = vccpFetch;
		}
}


#endif /* DEBUG */



/* S E T  S P E C I A L  M A T C H */
/*
	Parses the global vszSearch for special characters - all prefixed
	chars are translated into a single word in rgwSearch; non-prefixed
	chars also take up a full word; this is necessary for Boyer/Moore
	algorithm since a backwards scan through a search string is
	non-deterministic if there are ^ characters in it. A table is created
	where mpchdcp[chDoc] is the minimum jump required to find a match
	when chDoc occurs as a mismatch (Boyer/Moore strategy).

	rgwOppCase holds a copy of rgwSearch of opposite case: if rgwSearch[i]
	is lower-case, rgwOppCase[i] is the corresponding upper-case char,
	and vice versa. The length of the search string is returned in
	pbmib->cwSearch.
*/



/* %%Function:SetSpecialMatch %%Owner:rosiep */
SetSpecialMatch (pbmib)
struct BMIB *pbmib;
{
	int	iw, diw, cch, cwSearch, ich, iwMatchAny, iwMatchWhite;
	int	dcp, dcpMatchAny, dcpMatchWhite;
	int	wNew, chPattern;
	int	cchMatchAny = 0;  /* number of unescaped ?'s in string */
	char	*mpchdcp = pbmib->mpchdcp;   /* local for speed */

	pbmib->fChTableInPattern = fFalse;

	pbmib->fNotPlain = PdodDoc(selCur.doc)->fFormatted
			|| PwwdWw(wwCur)->fOutline
			|| vfbSearch.fChp || vfbSearch.fPap
			|| (vfReplace && (vfbReplace.fChp || vfbReplace.fPap));

	if (!vfbSearch.fText)
		{
		/* setting cwSearch to 1 makes the initialization of
		CpSearchSz() and CpSearchSzBackward() work out for non-text
		searches. */
		pbmib->cwSearch = 1;
		return;
		}

	/* special value that will make initialization of mpchdcp work
	out perfectly if no chMatchAny or chMatchWhite in pattern; however
	if !vfFwd, these will need to be reset once we know cwSearch */

	iwMatchAny = iwMatchWhite = -1;

	cch = CchSz (vszSearch) - 1;
	for (iw = ich = 0; ich < cch; ich++, iw++)
		{
		switch (chPattern = vszSearch[ich])
			{
		default:
			wNew = chPattern;
			break;

		case chMatchAny:
			/* want iw to be index of rightmost chMatchAny in pattern (leftmost if !vfFwd) */
			if (vfFwd || iwMatchAny == -1)
				iwMatchAny = iw;
			wNew = wMatchAny;      /* special word value for MatchAny   */
			cchMatchAny++;
			break;

		case chPrefixMatch:
			switch (chPattern = vszSearch[ich + 1])
				{
			default:
				if (FIsDigitS (chPattern))
					{
					int	cchT;
					CHAR chT;

					cchT = CchChFromAnsiOrOem(vszSearch, ich + 1, cch, &chT);
					wNew = chT;
					if (wNew == chTable)
						pbmib->fChTableInPattern = fTrue;
					ich += (cchT - 1);  /* ich is now pointing to 2nd-to-last
						digit (or still the ^ if only one digit */
					}
				else  if (chPattern == '\0')
					wNew = chPrefixMatch;
				else  /* treat ^x as x   */
					wNew = chPattern;
				break;

			case chPrefixMatch:
				wNew = chPrefixMatch;
				break;
			case chMatchAny:
				wNew = chMatchAny;
				break;
			case chMatchWhite:
				/* want iw to be index of rightmost chMatchWhite (leftmost if !vfFwd) */
				if (vfFwd || iwMatchWhite == -1)
					iwMatchWhite = iw;
				wNew = wMatchWhite;
				break;
			case chMatchNBS:
				wNew = chNonBreakSpace;
				break;
			case chMatchTab:
				wNew = chTab;
				break;
			case chMatchNewLine:
				wNew = chCRJ;
				break;
			case chMatchNRH:
				wNew = chNonReqHyphen;
				break;
			case chMatchNBH:
				wNew = chNonBreakHyphen;
				break;
			case chMatchSect:
				wNew = chSect;
				break;
			case chMatchEol:
				wNew = chEol;
				break;
				}
			ich++;
			break;
			}

		if (FUpper (wNew))
			pbmib->rgwOppCase[iw] = ChLower (wNew);
		else
			pbmib->rgwOppCase[iw] = ChUpper (wNew);
		pbmib->rgwSearch[iw] = wNew;
		}

	cwSearch = iw;

	if (cchMatchAny == cch)
		{    /* string of all '?'s is to be treated as
			a sequence of question marks and not
					a group of wild cards. */
		for (iw = 0; iw < cch; iw++)
			pbmib->rgwSearch[iw] = '?';
		iwMatchAny = vfFwd ? -1 : cwSearch;
		}


	/* need to reset these so they'll work in the backwards case */
	if (!vfFwd)
		{
		if (iwMatchAny == -1)
			iwMatchAny = cwSearch;
		if (iwMatchWhite == -1)
			iwMatchWhite = cwSearch;
		}


	/* mpchdcp[chDoc] will hold minimum shift to next possible   */
	/* match when mismatched chDoc is encountered;               */
	/* ( = dcpMatchAny if chDoc not in *pwPatFirst)              */


	dcpMatchAny = (vfFwd ? cwSearch - iwMatchAny - 1 : iwMatchAny);
	SetBytes (mpchdcp, dcpMatchAny, 256);

	diw = (vfFwd ? 1 : -1);
	for (iw = iwMatchAny + diw, dcp = dcpMatchAny - 1; 
			dcp >= 0; iw += diw, dcp--)
		{
		if (pbmib->rgwSearch[iw] >= 256)   /* don't trash memory when */
			continue;                      /* we encounter wMatchWhite */
		mpchdcp[pbmib->rgwSearch[iw]] = dcp;
		if (!vfSearchCase)
			{{ /* NATIVE - short pattern loop in SetSpecialMatch */
			int	chIntl, chUpper;

			chUpper = pbmib->rgwSearch[iw];
			for (chIntl = 0; chIntl < 256; chIntl++)
				if (ChUpper(chIntl) == chUpper)
					mpchdcp[chIntl] = dcp;
			mpchdcp[pbmib->rgwOppCase[iw]] = dcp;
			}}
		}



	if ((vfFwd && iwMatchWhite > iwMatchAny) || 
			(!vfFwd && iwMatchWhite < iwMatchAny))
		{       /* may need to update white-space values   */
		dcpMatchWhite = (vfFwd ? cwSearch - iwMatchWhite - 1 : iwMatchWhite);
		if (mpchdcp[chSpace] > dcpMatchWhite)
			mpchdcp[chSpace] = dcpMatchWhite;
		if (mpchdcp[chTab] > dcpMatchWhite)
			mpchdcp[chTab] = dcpMatchWhite;
		if (mpchdcp[chNonBreakSpace] > dcpMatchWhite)
			mpchdcp[chNonBreakSpace] = dcpMatchWhite;
		}

	if (mpchdcp[chNonBreakSpace] > mpchdcp[chSpace])
		mpchdcp[chNonBreakSpace] = mpchdcp[chSpace];
	if (mpchdcp[chNonBreakHyphen] > mpchdcp[chHyphen])
		mpchdcp[chNonBreakHyphen] = mpchdcp[chHyphen];

	/* set mpchdcp[chNonReqHyphen] to zero even though
	it might not be the last character in the search string.
	This allows us to avoid specifically checking for
	chNonReqHyphen in the optimized inner loop of search. */
	mpchdcp[chNonReqHyphen] = 0;
	/* ditto for mpchdcp[chReturn] */
	mpchdcp[chReturn] = 0;
	pbmib->cwSearch = cwSearch;      /* return length of search string */
}



/*
*  C C H  C H  F R O M  A N S I  O R  O E M
*
*  Input:  rgch is the string we are searching through
*          ich is index in rgch of the first digit after the ^
*          digits will be taken as Ansi if first digit is 0, otherwise Oem.
*          ichMax is the length of rgch
*
*  Returns:  cch   # of digits processed
*            ch    ansi or oem character equivalent of the ^[0]nnn
*
*/


/* %%Function:CchChFromAnsiOrOem %%Owner:rosiep */
CchChFromAnsiOrOem(rgch, ich, ichMax, pch)
CHAR rgch[];
int	ich, ichMax;
CHAR *pch;
{
	CHAR chPattern, ch;
	BOOL fAnsi;   /* whether we want to interpret as ansi */
	int	iDigit;
	int	cDigitMax;    /* max number of digits in ansi or oem code; any
		digits beyond this limit will be treated as
			normal text */

	chPattern = rgch[ich++];
	ch = chPattern - '0';
	fAnsi = (chPattern == '0');
	cDigitMax = (fAnsi ? 4 : 3);      /* ansi will have a leading 0 */

	for (iDigit = 1; iDigit < cDigitMax && ich < ichMax && 
			FIsDigitS(chPattern = rgch[ich]); iDigit++, ich++)
		{
		ch = ch * 10 + (chPattern - '0');
		}

	if (fAnsi || ch < chSpace)
		*pch = ch;
	else  /* interpret as Oem */
		*pch = ChAnsiFromOem(ch);

	return (iDigit);
}


/*
*  C H  A N S I  F R O M  O E M
*
*  Converts an Oem character to an Ansi character (shame Windows doesn't
*  provide this directly).
*
*/

/* %%Function:ChAnsiFromOem %%Owner:rosiep */
CHAR ChAnsiFromOem(ch)
CHAR *ch;
{
	CHAR szOem[2];
	CHAR szAnsi[2];

	szOem[0] = ch;
	szOem[1] = '\0';
	OemToAnsi((LPSTR) szOem, (LPSTR) szAnsi);
	return (szAnsi[0]);
}



/* F  I S  D I G I T  S */

/* %%Function:FIsDigitS %%Owner:rosiep */
BOOL FIsDigitS(ch)
char	ch;
{
	return ('0' <= ch && ch <= '9');
}


/* C H E C K  F O R  F O R M A T T I N G */
/* Set up the fChp and fPap bits in the formatting blocks for Search
	and Replace, based on whether there was any formatting specified
	in the dialog
*/


/* %%Function:CheckForFormatting %%Owner:rosiep */
CheckForFormatting()
{
	vfbSearch.fChp = FZeroBit(&vfbSearch.chpGray, cwCHP);
	vfbSearch.fPap = FZeroBit(&vfbSearch.papGray, cwPAPS);
	if (vfReplace)
		{
		vfbReplace.fChp = FZeroBit(&vfbReplace.chpGray, cwCHP);
		vfbReplace.fPap = FZeroBit(&vfbReplace.papGray, cwPAPS);

		/* check for special case of turning off formatting */
		if (*vszReplace == chPrefixMatch && *(vszReplace + 1) == chUseMatched && 
				*(vszReplace + 2) == '\0')
			{
			vfbReplace.fChp |= vfbSearch.fChp;
			vfbReplace.fPap |= vfbSearch.fPap;
			}

		if (vfbReplace.fChp)
			{{ /* NATIVE - small loop to and words in CheckForFormatting */
			int	*pwResult, *pw1, *pw2;
			int	cw = cwCHP;

			pwResult = &vchpSRGray;
			pw1 = &vfbSearch.chpGray;
			pw2 = &vfbReplace.chpGray;
			while (cw--)
				*pwResult++ = *pw1++ & *pw2++;
			}}
		}
}


/* F  Z E R O  B I T */
/* Returns fTrue if there is any zero bit in the range of cw words
starting at pw */


/* %%Function:FZeroBit %%Owner:rosiep */
BOOL FZeroBit(pw, cw)
int	*pw;
int	cw;
{
	int	wT = -1;

		{{ /* NATIVE - small loop anding together words in FZeroBit */
		while (cw--)
			wT &= *pw++;
		}}

	return (wT != -1);
}


/* M E R G E  P R O P S  P R O P  G R A Y */
/* Merges properties from pprop that aren't gray (as specified in ppropGray)
into ppropResult */

/* %%Function:MergePropsPropGray %%Owner:rosiep */
MergePropsPropGray(ppropResult, pprop, ppropGray, cw)
int	*ppropResult, *pprop, *ppropGray;
int	cw;
{{ /* NATIVE - small loop to merge properties in MergePropsPropGray */
	while (cw--)
		*ppropResult++ = (*ppropGray++ & *ppropResult) | *pprop++;
}}


#ifdef DEBUG

/* %%Function:FMatchChp %%Owner:rosiep */
BOOL C_FMatchChp()
{
	int	*pchpFetch = &vchpFetch;   /* !! Assumes vchpFetch has been set up */
	int	*pchpSearch = &vfbSearch.chp;
	int	*pchpGray = &vfbSearch.chpGray;
	int	cw = cwCHP;
	int	wT = 0;

	while (cw--)
		wT |= ((*pchpFetch++ ^ *pchpSearch++) & (~ * pchpGray++));

	return (!wT);
}


#endif /* DEBUG */

#ifdef DEBUG

/* %%Function:FMatchPap %%Owner:rosiep */
BOOL C_FMatchPap()
{
	int	*ppapFetch = &vpapFetch;   /* !! Assumes vpapFetch has been set up */
	int	*ppapSearch = &vfbSearch.pap;
	int	*ppapGray = &vfbSearch.papGray;
	int	cw = cwPAPS;
	int	wT = 0;

	while (cw--)
		wT |= ((*ppapFetch++ ^ *ppapSearch++) & (~ * ppapGray++));

	return (!wT);
}


#endif /* DEBUG */




/* %%Function:CmdSearchChar %%Owner:rosiep */
CMD CmdSearchChar(pcmb)
CMB *pcmb;
{
	InitSearchFormatting();

	if (pcmb->fDefaults)
		{
		GetCharDefaults(pcmb, &vfbSearch.chp, &vfbSearch.chpGray);
		}

	if (pcmb->fAction)
		{
		CabToPropPropGray(pcmb->hcab, &vfbSearch.chp, 
				&vfbSearch.chpGray, sgcChp);
		CheckForFormatting();
		}

	return cmdOK;
}



/* %%Function:CmdSearchPara %%Owner:rosiep */
CMD CmdSearchPara(pcmb)
CMB *pcmb;
{
	InitSearchFormatting();

	if (pcmb->fDefaults)
		if (!FGetParaDefaults(pcmb, &vfbSearch.pap,
				&vfbSearch.papGray, fTrue /* fPaps */, fFalse /* fStyList */))
			return cmdError;

	if (pcmb->fAction)
		{
		CabToPropPropGray(pcmb->hcab, &vfbSearch.pap, 
				&vfbSearch.papGray, sgcPap);
		CheckForFormatting();
		}

	return cmdOK;
}


/* %%Function:FWordCp %%Owner:rosiep */
BOOL FWordCp(cp, dcp)
CP cp;
CP dcp;
{
	/* sees if the word starting at cp (with dcp chars) is a separate
		word. */
	int	ich;

	Assert (cp >= cp0);

	if (dcp == cp0)
		return fFalse;

	/* check the start of the word */
	if (cp != cp0)
		{
		int	wbStart;

		CachePara(selCur.doc, cp - 1);
		FetchCp(selCur.doc, cp - 1, fcmChars + fcmParseCaps);
		ich = 0;
		wbStart = WbFromCh(vhpchFetch[ich]);
		if (vcpFetch + vccpFetch <= cp)
			{
			FetchCpAndPara(selCur.doc, cp, fcmChars + fcmParseCaps);
			ich = 0;
			}
		else
			ich++;
		if (wbStart == WbFromCh(vhpchFetch[ich]))
			return(fFalse);
		}

	/* check the end of the word */
	if (cp + dcp < CpMacDocEdit(selCur.doc))
		{
		int	wbEnd;

		if (vcpFetch + vccpFetch <= cp + dcp - 1 || vcpFetch > cp + dcp - 1)
			{
			FetchCpAndPara(selCur.doc, cp + dcp - 1, fcmChars + fcmParseCaps);
			ich = 0;
			}
		else
			ich =  (dcp - 1) - (vcpFetch - cp);
		wbEnd = WbFromCh(vhpchFetch[ich]);
		if (vcpFetch + vccpFetch <= cp + dcp)
			{
			FetchCpAndPara(selCur.doc, cp + dcp, fcmChars + fcmParseCaps);
			ich = 0;
			}
		else
			ich++;
		if (vccpFetch != 0 && wbEnd == WbFromCh(vhpchFetch[ich]))
			return(fFalse);
		}

	return(fTrue);
}


/* %%Function:FEditSearchFound %%Owner:bradch */
FEditSearchFound()
{
	return vfFound ? -1 : 0;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Search_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Search_Last() */
