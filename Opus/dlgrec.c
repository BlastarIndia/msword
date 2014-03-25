#include "word.h"
#include "error.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "status.h"
#include "cmdtbl.h"
#include "props.h"
#include "sel.h"
#include "recorder.h"
#include "menu2.h"
#define USEBCM
#include "opuscmd.h"
#include "format.h"
#include "el.h"
#include "macrocmd.h"
#include "insert.h"
#include "file.h"
#include "wininfo.h"
#include "message.h"
#include "help.h"
#include "resource.h"
#include "keys.h"
#include "prm.h"
/*#include "fontwin.h"*/

#include "idd.h"
/* #include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h" included by el.h*/
#include "sdmtmpl.h"

#include "tmc.h"

#include "asgn2key.hs"
#include "asgn2mnu.hs"
#include "tabs.hs"
#include "char.hs"
#include "para.hs"
#include "abspos.hs"
#include "edstyle.hs"
#include "cust.hs"
#include "style.hs"

#include "recorder.hs"
#include "recorder.sdm"

#include "prm.h"
#include "fkp.h"

#include "screen.h"
#include "rareflag.h"


/* E X T E R N A L S */
extern HWND vhwndCBT;
extern struct SCI vsci;
extern struct STTB ** vhsttbFont;
extern BOOL vfDefineStyle;
extern struct PREF vpref;
extern struct MERR vmerr;
extern CHAR szEmpty [];
extern CHAR stEmpty [];
extern struct FLI vfli;
extern HWND vhwndApp;
extern HWND vhwndDeskTop;
extern BOOL vfRecording;
extern int docRecord;
extern CP cpRecord;
extern char rgchEop [];
extern struct MWD ** hmwdCur;
extern char rgchInsert[];
extern int ichInsert;
extern struct RAC vrac;
extern struct SEL selCur;
extern int docGlobalDot;
extern BOOL	fRecordMove;
extern BOOL	fRecordSize;
extern int	cdlScroll;
extern BOOL	fCdlScrollSet;
extern HWND vhwndStatLine;
extern HCURSOR  vhcRecorder;
extern char ** hrgchRecIns;
extern uns ichMacRecIns;
extern uns ichMaxRecIns;
extern BOOL FRecordCab();

/*extern*/ BOOL vfRecordNext = fFalse;
/*extern*/ BOOL vfRecorderOOM = fFalse;
/*extern*/ int vcchRecord;
/*extern*/ int vdocDotRecord;

#ifdef DEBUG
static HWND hwndRecorder;
#endif /* DEBUG */


CMR ** vhcmr = hNil;

csconst char stMacro[] = St("Macro");


CMD CmdStartRecorder();
CMD CmdStopRecorder();


/* C M D  R E C O R D E R */
/* %%Function:CmdRecorder %%Owner:bradch */
CMD CmdRecorder(pcmb)
CMB * pcmb;
{
	CMD cmd;

	if (vrf.fPauseRecorder && (cmd = CmdPauseRecorder(pcmb)) != cmdOK)
		return cmd;

	cmd = (*((vfRecording || vhcmr != hNil) ? CmdStopRecorder : CmdStartRecorder))(pcmb);

	if (vhwndStatLine != NULL)
		UpdateStatusLine(usoToggles);

	return cmd;
}


/* C M D  S T O P  R E C O R D E R */
/* %%Function:CmdStopRecorder %%Owner:bradch */
CMD CmdStopRecorder(pcmb)
CMB * pcmb;
{
	if (!pcmb->fAction)
		return cmdOK;

	return FStopRecorder(fFalse /* fAskSave */, fFalse) ? cmdOK : cmdError;
}


/* E N D  R E C O R D  N E X T  C M D */
/* For "special cases" that are explicitly recorded while in RecordNext,
 * this function should be called to end the recording. */
/* %%Function:EndRecordNextCmd %%Owner:bradch */
void EndRecordNextCmd()
{
	Assert(vfRecording);

	if (!vfRecordNext)
		return;
	if (vrac.racop != racopNil)
		FlushVrac();
	EndSdmTranscription();
	vfRecording = vfRecordNext = fFalse;
	if (vhwndStatLine != NULL)
		UpdateStatusLine(usoToggles);	/* Make sure "REC" toggle updated */
	/* Blow away Undo and Repeat, since we have inserted text, and they
	 * may no longer be valid */
	SetUndoNil();
	SetAgain(bcmNil);
}


/* %%Function:FDlgRecorder %%Owner:bradch */
FDlgRecorder(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int docDot;
	char stName[ichMaxBufDlg];

	switch (dlm)
		{
	case dlmInit:
		if (FDisableTemplate(selCur.doc))
			EnableTmc(tmcDocType, fFalse);
		break;

	case dlmChange:
		if (tmc == tmcName)
			{
			BOOL fGlobal;

			fGlobal = ValGetTmc(tmcContext) == iContextGlobal;
			GetTmcText(tmcName, stName, sizeof (stName));
			SzToStInPlace(stName);
			EnableTmc(tmcOK, VmnValidMacroNameSt(stName) ?
					fTrue : fFalse);
			}
		break;

	case dlmTerm:
		if (tmc != tmcOK)
			break;

		GetTmcText(tmcName, stName, sizeof (stName));
		SzToStInPlace(stName);

		/* Check to see if macro is being edited, don't allow that! */
		docDot = ValGetTmc(tmcContext) == iContextGlobal ? 
			docGlobalDot : DocDotMother(selCur.doc);
			
		if (ImeiFromSt(stName, docDot) != iNil)
			{
			ErrorEid(eidCantRecordOverEd, "FDlgRecorder");
			SetFocusTmc(tmcName);
			return fFalse;
			}

		/* Check to see if macro exists already, and ask to replace */
		if (vhwndCBT == NULL && VmnValidMacroNameSt(stName) == vmnExists &&
				(FetchSy(BsyOfStDocDot(stName, docDot)), 
				(vpsyFetch->docDot == docDot &&  vpsyFetch->mct != mctNil)))
			{
			switch (IdMessageBoxMstRgwMb(mstReplMacro, NULL, 
					MB_YESNOCANCEL | MB_ICONQUESTION))
				{
			case IDNO:
				SetFocusTmc(tmcName);
				return fFalse;

			case IDCANCEL:
				return fFalse;
				}
			}
		/* break; */
		}

	return fTrue;
}


/* C M D  S T A R T  R E C O R D E R */
/* %%Function:CmdStartRecorder %%Owner:bradch */
CMD CmdStartRecorder(pcmb)
CMB * pcmb;
{
	Assert(!vfRecording);
	Assert(vhcmr == hNil);

	if (pcmb->fDefaults)
		{
		char * pch;
		char stName [cchMaxSyName];

		/* Generate a name to suggest. */
		GenUniqueMacroNameSz(stName);

		Assert(pcmb->hcab != hNil);
		((CABRECORDER *) *pcmb->hcab)->iContext = iContextGlobal;
		FSetCabSt(pcmb->hcab, stName, Iag(CABRECORDER, pszName));
		FSetCabSz(pcmb->hcab, szEmpty, Iag(CABRECORDER, pszDesc));
		/* fMemFail will be true if FSetCabSz fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltRecorder)];

		BltDlt(dltRecorder, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}


	if (pcmb->fAction)
		{
		CMR * pcmr;
		char stName [cchMaxSyName];
		char stDesc [ichMaxBufDlg + 1];

		cpRecord = cp0;
		if (!FInitMacroDoc() || !FInitRecorderDoc())
			return cmdError;
		Assert(docRecord != docNil);

		if (!FAssureIrcds(ircdsRecorder))
			return cmdError;

		GetCabStz(pcmb->hcab, stDesc, sizeof (stDesc),
				Iag(CABRECORDER, pszDesc));

		if ((vhcmr = HAllocateCw(CwFromCch(cbCMR + *stDesc + 1)))
				== hNil)
			{
			/* Not enough memory to record (should never happen
				since memory was just freed from dialog) */
			return cmdNoMemory;
			}


		GetCabStz(pcmb->hcab, stName, sizeof (pcmr->stName), 
				Iag(CABRECORDER, pszName));

		pcmr = *vhcmr;
		pcmr->iContext = ((CABRECORDER *) *pcmb->hcab)->iContext;
		CopySt(stDesc, pcmr->stDesc);
		CopySt(stName, pcmr->stName);

		vdocDotRecord = pcmr->iContext == iContextGlobal ? 
				docGlobalDot : DocDotMother(selCur.doc);
		PdodDoc(vdocDotRecord)->crefLock++;

		vfRecording = fTrue;
		vfRecorderOOM = fFalse;
		vfRecordNext = fFalse;
		vrf.fPauseRecorder = fFalse;
		RecordSt(StSharedKey("Sub MAIN", SubMain));
		RecordEop();
		vrac.racop = racopNil;
		SaveCabs(FRecordCab, fFalse);
		}

	return cmdOK;
}


csconst char stEndSub [] = StKey("End Sub", EndSub);


/* F  S A V E  R E C O R D E R */
/* Stop recording and if stName is not 0, save the macro with
the given name */
/* %%Function:FSaveRecorder %%Owner:bradch */
FSaveRecorder(stName, stDesc, fGlobal)
char * stName, * stDesc;
BOOL fGlobal;
{
	struct CA caT;
	int docDot;
	int bcm;
	HQ hqrgbTokens;
	uns cbTokens;
	char stBuf [40];
	BOOL fRet;

	Assert(vfRecording || vhcmr != hNil);

	StartLongOp();

	/* FinishLastStatement */
	FlushVrac();
	CopyCsSt(stEndSub, stBuf);
	RecordSt(stBuf);

	if (!FTokenizePca(PcaSet(&caT, docRecord, cp0, cpRecord), 
			&hqrgbTokens, &cbTokens))
		{
		cpRecord -= stEndSub[0];
		EndLongOp(fFalse);
		return fFalse;
		}

	Assert(vdocDotRecord != docNil);

	if ((bcm = BsyEnsureMacroExists(stName, vdocDotRecord, NULL)) == 
			bsyNil)
		{
		EndLongOp(fFalse);
		return fFalse;
		}

		/* BLOCK */
		{
		int docMcr, imcr;

		docMcr = PdodDoc(vdocDotRecord)->docMcr;
		imcr = ImcrFromDocDotBcm(vdocDotRecord, bcm);
		CaFromIhdd(docMcr, imcr, &caT);
		if (!FReplacePcaWithHqrgb(&caT, hqrgbTokens, cbTokens))
			{
			EndLongOp(fFalse);
			return fFalse;
			}
		DirtyDoc(vdocDotRecord);
		}

	fRet = FSetMenuHelp(bcm, stDesc);
	EndLongOp(fFalse);
	return fRet;
}


/* %%Function:FInitRecorderDoc %%Owner:bradch */
FInitRecorderDoc()
{
	BOOL fRet;

	cpRecord = cp0;

	fRet = (docRecord != docNil || 
			(docRecord = DocCloneDoc(docNew, /*dkSDoc*/dkDoc)) != docNil);

	return fRet;
}


/* %%Function:CmdPauseRecorder %%Owner:bradch */
CMD CmdPauseRecorder(pcmb)
CMB * pcmb;
{
	if (!pcmb->fAction)
		return cmdOK;

	if (vrf.fPauseRecorder)
		{
		SaveCabs(FRecordCab, fFalse);
		}
	else
		{
		if (!vfRecording)
			{
			Beep();
			return cmdError;
			}

		EndSdmTranscription();
		}

	vfRecording = vrf.fPauseRecorder;
	vrf.fPauseRecorder = !vrf.fPauseRecorder;

	return cmdOK;
}


/* %%Function:GenUniqueMacroNameSz %%Owner:bradch */
GenUniqueMacroNameSz(stBuf)
char * stBuf;
{
	char * pch, * pchT;
	int cch, w;

	CopyCsSt(stMacro, stBuf);
	cch = stBuf[0];
	pch = &stBuf[1 + cch];
	w = 0;

	do
		{
		pchT = pch;
		stBuf[0] = cch + CchIntToPpch(++w, &pchT);
		}
	while (BcmOfSt(stBuf) != bcmNil);
}


/* %%Function:CmdRecordNext %%Owner:bradch */
CMD CmdRecordNext(pcmb)
CMB *pcmb;
{
	extern void ** vhmes; /* NOTE: really a MES ** */

	if (vfRecording || vhmes == hNil || vhcmr != hNil ||
			(vhcRecorder == NULL && !FAssureIrcds(ircdsRecorder)))
		{
		Beep();
		return cmdError;
		}

	vfRecording = vfRecordNext = fTrue;

	return cmdOK;
}


/* %%Function:FIsRecorderSt %%Owner:bradch */
FIsRecorderSt(stName)
char * stName;
{
	return (FEqNcSt(stName, (*vhcmr)->stName));
}


/* %%Function:FStopRecorder %%Owner:bradch */
FStopRecorder(fAskSave, fForceKill)
BOOL fAskSave, fForceKill;
{
	BOOL fSave, fRet;
	CMR * pcmr;
	char stName [cchMaxSyName];
	char stDesc [ichMaxBufDlg];

	Assert(vfRecording || vhcmr != hNil);

	if (fForceKill)
		fAskSave = fFalse;

	if (vfRecordNext || vhcmr == hNil)
		{
		/* Recording "StopRecorder" as the next command of a RecordNextCmd
		 * should just end the RecordNextCmd. */
		EndRecordNextCmd();
		return fTrue;
		}

	Assert(!vfRecordNext);
	Assert(vhcmr != hNil);

	fSave = fForceKill ? fFalse : fTrue;

	if (fAskSave)
		{
		switch (IdMessageBoxMstRgwMb(mstSaveMacroRecording, NULL,
				MB_YESNOCANCEL | MB_ICONQUESTION))
			{
		case IDCANCEL:
			return fFalse;

		case IDNO:
			fSave = fFalse;
			break;

#ifdef DEBUG
		case IDYES:
			break;

		default:
			Assert(fFalse);
			break;
#endif
			}
		}


	fRet = fTrue;
	EndSdmTranscription();

	pcmr = *vhcmr;
	CopySt(pcmr->stName, stName);
	CopySt(pcmr->stDesc, stDesc);

	if (fSave && 
			!FSaveRecorder(stName, stDesc, pcmr->iContext == iContextGlobal))
		{
		BOOL FRecordCab();

		if (vmerr.fMemFail || vmerr.fDiskFail)
			{
			/* Not enough memory to tokenize, so we should not
				stop recording until the user frees some memory */

			ErrorEid(eidNoMemRecord, "CmdStopRecorder");

			SaveCabs(FRecordCab, fFalse);
			return fFalse;
			}
		else
			{
			ErrorEid(eidRecordingTooBig, "CmdStopRecorder");
			fRet = fFalse;
			}
		}

	Assert(vdocDotRecord != docNil);
	PdodDoc(vdocDotRecord)->crefLock--;
	vdocDotRecord = docNil;
	vfRecording = fFalse;
	FreeIrcds(ircdsRecorder);
	FreePh(&vhcmr);

	if (hrgchRecIns != hNil)
		{
		FreePh(&hrgchRecIns);
		ichMacRecIns = ichMaxRecIns = 0;
		}

	Assert(hrgchRecIns == hNil && ichMacRecIns == 0 && ichMaxRecIns == 0);

	return fRet;
}


/***************************************************/
/*** Special recording functions - not used much ***/
/***************************************************/


/* Called when the user changes the active window through one of the normal
Windows mechanisms. */
/* %%Function:RecordWindowChange %%Owner:bradch */
RecordWindowChange(hwnd, iPane)
HWND hwnd;
uns iPane;	/* 1 or 2 means change to Upper pane, 3 or 4 for Lower pane */
{
	extern struct STTB ** vhsttbWnd;
	int mw;
	CHAR rgch [ichMaxFile+5];

	if (vrac.racop != racopNil)
		FlushVrac();
	mw = MwFromHwndMw(hwnd);
	RecordSt(StSharedKey("Activate ",CmdActivate));
	GetWindowText(hwnd, (LPSTR)rgch, ichMaxFile+4);
	RecordQuotedRgch(rgch, CchSz(rgch)-1);
	if (iPane)
		{
		Assert(iPane >= 1 && iPane <= 4);
		RecordComma();
		RecordInt(iPane);
		}
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* R E C O R D  P A N E  C H A N G E */
/* %%Function:RecordPaneChange %%Owner:bradch */
RecordPaneChange()
{
	if (vrac.racop != racopNil)
		FlushVrac();
	RecordSt(StSharedKey("OtherPane",CmdOtherPane));
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordAppSysCmd %%Owner:bradch */
RecordAppSysCmd(sc)
int sc;
{
	if (vrac.racop != racopNil)
		FlushVrac();

	switch (sc & 0xfff0)
		{
	case SC_MAXIMIZE:
		/* Check to see if we're already maximized.  If so, this is actually
		 * an AppRestore call. */
		if (GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE)
			RecordSt(StSharedKey("AppRestore",CmdAppRestore));
		else
			RecordSt(StSharedKey("AppMaximize",CmdAppMaximize));
		fRecordMove = fFalse;
		fRecordSize = fFalse;
		break;

	case SC_MINIMIZE:
		RecordSt(StSharedKey("AppMinimize",CmdAppMinimize));
		fRecordMove = fFalse;
		fRecordSize = fFalse;
		break;

	case SC_RESTORE:
		RecordSt(StSharedKey("AppRestore",CmdAppRestore));
		fRecordMove = fFalse;
		fRecordSize = fFalse;
		break;

	default:
		return;
		}

	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordDocSysCmd %%Owner:bradch */
RecordDocSysCmd(sc)
int sc;
{
	if (vrac.racop != racopNil)
		FlushVrac();

	switch (sc & 0xfff0)
		{
	case SC_MAXIMIZE:
		RecordSt(StSharedKey("DocMaximize",CmdDocMaximize));
		break;

	case SC_NEXTWINDOW:
		RecordSt(StSharedKey("NextWindow",CmdNextWindow));
		break;

	case SC_PREVWINDOW:
		RecordSt(StSharedKey("PrevWindow",CmdPrevWindow));
		break;

	case SC_ARRANGE:
		RecordSt(StSharedKey("WindowArrangeAll",CmdWindowArrangeAll));
		break;

	case SC_RESTORE:
		RecordSt(StSharedKey("DocRestore",CmdDocRestore));
		break;

	default:
		return;
		}

	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordDocMove %%Owner:bradch */
RecordDocMove(xp, yp)
int xp, yp;
{
	struct RC rc;

	if (vrac.racop != racopNil)
		FlushVrac();

	GetWindowRect((*hmwdCur)->hwnd, (LPRECT) &rc);
	ScreenToClient(vhwndDeskTop, (LPPOINT) &rc.ptTopLeft);

	RecordSt(StSharedKey("DocMove ",CmdDocMove));
	RecordInt(NMultDiv(rc.xpLeft, dxaInch, vfli.dxsInch) / dyaPoint);
	RecordComma();
	RecordInt(NMultDiv(rc.ypTop, dxaInch, vfli.dysInch) / dyaPoint);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordDocSize %%Owner:bradch */
RecordDocSize(dxp, dyp)
int dxp, dyp;
{
	struct RC rc;

	if (vrac.racop != racopNil)
		FlushVrac();

	GetWindowRect((*hmwdCur)->hwnd, (LPRECT) &rc);

	RecordSt(StSharedKey("DocSize ",CmdDocSize));
	RecordInt(NMultDiv(rc.xpRight - rc.xpLeft, dxaInch, vfli.dxsInch) / dyaPoint);
	RecordComma();
	RecordInt(NMultDiv(rc.ypBottom - rc.ypTop, dxaInch, vfli.dysInch) / dyaPoint);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordAppMove %%Owner:bradch */
RecordAppMove(xp, yp)
int xp, yp;
{
	struct RC rc;

	if (vrac.racop != racopNil)
		FlushVrac();

	GetWindowRect(vhwndApp, (LPRECT) &rc);

	RecordSt(StSharedKey("AppMove ",CmdAppMove));
	RecordInt(NMultDiv(rc.xpLeft, dxaInch, vfli.dxsInch) / dyaPoint);
	RecordComma();
	RecordInt(NMultDiv(rc.ypTop, dxaInch, vfli.dysInch) / dyaPoint);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordAppSize %%Owner:bradch */
RecordAppSize(dxp, dyp)
int dxp, dyp;
{
	struct RC rc;

	if (vrac.racop != racopNil)
		FlushVrac();

	GetWindowRect(vhwndApp, (LPRECT) &rc);

	RecordSt(StSharedKey("AppSize ",CmdAppSize));
	RecordInt(NMultDiv(rc.xpRight - rc.xpLeft, dxaInch, vfli.dxsInch) / dyaPoint);
	RecordComma();
	RecordInt(NMultDiv(rc.ypBottom - rc.ypTop, dxaInch, vfli.dysInch) / dyaPoint);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordSplit %%Owner:bradch */
RecordSplit(yp)
int yp;
{
	if (vrac.racop != racopNil)
		FlushVrac();

	RecordSt(StSharedKey("DocSplit ",CmdDocSplit));
	RecordInt(NMultDiv(yp, 100, (*hmwdCur)->dyp));

	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


csconst char csstKillSp [] = StKey("Kill ",CmdKillSp);

/* %%Function:RecordDelFile %%Owner:bradch */
RecordDelFile(stName)
char * stName;
{
	char stBuf [sizeof (csstKillSp)];

	Assert(sizeof (csstKillSp) == csstKillSp[0] + 1);

	CopyCsSt(csstKillSp, stBuf);
	RecordSt(stBuf);
	RecordQuotedSt(stName);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordChdir %%Owner:bradch */
RecordChdir(stPath)
char * stPath;
{
	BOOL fReplaceSlash;

	Assert(vfRecording);

	if (fReplaceSlash = (stPath[0] > 3 && stPath[stPath[0]] == '\\'))
		{
		stPath[0] -= 1;
		}
	RecordSt(StSharedKey("ChDir ", CmdChDir));
	RecordQuotedSt(stPath);
	RecordEop();

	stPath[0] += fReplaceSlash;

	/* Note that RecordNextCommand is not ended here */
}


/* %%Function:RecordRepeatTyping %%Owner:bradch */
RecordRepeatTyping(pca)
struct CA * pca;
{
	int doc;
	CP cp, cpLim;
	char ch;

	doc = pca->doc;
	cpLim = pca->cpLim;

	for (cp = pca->cpFirst; cp < cpLim; cp += 1)
		{
		ch = ChFetch(doc, cp, fcmChars);

		if (ch == rgchEop[0])
			cp += ccpEop - 1;

		if (!FRecInsRgch(&ch, 1))
			break;
		}
}


/* R E C O R D  E X T E N D  S E L  T O  C H */
/* %%Function:RecordExtendSelToCh %%Owner:bradch */
RecordExtendSelToCh(ch)
int ch;
{
	CHAR rgch[1];

	if (vrac.racop != racopNil)
		FlushVrac();

	RecordSt(StSharedKey("ExtendSelection ",CmdExtendSelection));
	rgch[0] = (CHAR) ch;
	RecordQuotedRgch(rgch, 1);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


csconst char csstFmtPara [] = StKey("FormatParagraph ", CmdFmtPara);
csconst char csstFmtTbl [] = StKey("FormatTable ", CmdFmtTbl);
csconst char csstFmtSec [] = StKey("FormatSection ", CmdFmtSec);
csconst char csstFmtDoc [] = StKey("FormatDocument ", CmdFmtDoc);
csconst char csstFirstIndent [] = StKey(".FirstIndent = ", CmdFirstIndent);
csconst char csstLeftIndent [] = StKey(".LeftIndent = ", CmdLeftIndent);
csconst char csstRightIndent [] = StKey(".RightIndent = ", CmdRightIndent);
csconst char csstIndentRows [] = StKey(".IndentRows = ", CmdIndentRows);
csconst char csstSpaceBefore [] = StKey(".SpaceBefore = ", CmdSpaceBefore);
csconst char csstColWidth [] = StKey(".ColumnWidth = ", CmdColumnWidth);
csconst char csstLeftMargin [] = StKey(".LeftMargin = ", CmdLeftMargin);
csconst char csstRightMargin [] = StKey(".RightMargin = ", CmdRightMargin);
csconst char csstTopMargin [] = StKey(".TopMargin = ", CmdTopMargin);
csconst char csstBottomMargin [] = StKey(".BottomMargin = ", CmdBottomMargin);
csconst char csstColSpacing [] = StKey(".ColumnSpacing = ", CmdColumnSpacing);


/* %%Function:RecordRulerMargins %%Owner:bradch */
RecordRulerMargins(sprm, val)
int sprm, val;
{
	char stCmd [sizeof (csstFmtPara)];
	char stKey [128];

	/* If either of these asserts fail, change the use a different
		string in the stBuf [] sizeof */
	Assert(sizeof (csstFmtPara) > sizeof (csstFmtTbl));
	Assert(sizeof (csstFmtPara) > sizeof (csstFmtSec));

#ifdef DRECORDER
	CommSzNum(SzShared("sizeof (csstFmtPara)="), sizeof (csstFmtPara));
#endif

	CopyCsSt(csstFmtPara, stCmd); /* default */

	switch (sprm)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		return;
#endif

	case sprmPDyaBefore:
		/* FormatParagraph .SpaceBefore = */
		CopyCsSt(csstSpaceBefore, stKey);
		break;

	case sprmPDxaLeft1:
		/* FormatParagraph .FirstIndent = */
		CopyCsSt(csstFirstIndent, stKey);
		break;

	case sprmPDxaLeft:
		/* FormatParagraph .LeftIndent = */
		CopyCsSt(csstLeftIndent, stKey);
		break;

	case sprmPDxaRight:
		/* FormatParagraph .RightIndent = */
		CopyCsSt(csstRightIndent, stKey);
		break;

	case sprmTDxaLeft:
		/* FormatTable .IndentRows = */
		CopyCsSt(csstFmtTbl, stCmd);
		CopyCsSt(csstIndentRows, stKey);
		break;

	case sprmSDxaColumns:
		CopyCsSt(csstFmtSec, stCmd);
		CopyCsSt(csstColSpacing, stKey);
		break;

	case sprmTDxaCol:
		CopyCsSt(csstFmtTbl, stCmd);
		CopyCsSt(csstColWidth, stKey);
		break;
		}

	RecordSt(stCmd);
	RecordSt(stKey);
	RecordZa(val);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordZa %%Owner:bradch */
RecordZa(za)
int za;
{
	char * pchT;
	char stKey [20];

	pchT = stKey + 1;
	stKey[0] = CchExpZa(&pchT, za, vpref.ut, sizeof (stKey) - 1, fTrue);
	RecordQuotedSt(stKey);
}


/* %%Function:RecordStyleProps %%Owner:bradch */
RecordStyleProps(idd)
int idd;
{
	extern int vdocStsh, vstcStyle;

	CMB cmb;
	CABI cabi;
	struct CHP chp;
	struct PAP pap;

	switch (idd)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		return;
#endif

	case IDDStylePara:
		cabi = cabiCABPARALOOKS;
		break;

	case IDDStyleChar:
		cabi = cabiCABCHARACTER;
		break;

	case IDDStyleAbsPos:
		cabi = cabiCABABSPOS;
		break;
		}

	if ((cmb.hcab = HcabAlloc(cabi)) == hNil)
		{
		ErrorNoMemory(eidNoMemForRecord);
		return;
		}

	SetVdocStsh(selCur.doc);
	MapStc(PdodDoc(vdocStsh), vstcStyle, &chp, &pap);

	switch (idd)
		{
	case IDDStylePara:
		FGetParaDefaults(&cmb, &pap, (struct PAP *) NULL, 
				fFalse, fFalse);
		break;
	case IDDStyleChar:
		GetCharDefaults(&cmb, &chp, (struct CHP *) NULL);
		break;

	case IDDStyleAbsPos:
		GetAbsPosDefaults(&cmb, &pap, (struct PAP *) NULL);
		break;
		}

	FRecordCab(cmb.hcab, idd, tmcOK, fFalse);
	FreeCab(cmb.hcab);

	if (idd == IDDStylePara)
		{
		int itbd;
		HCABTABS hcab;

		if ((hcab = HcabAlloc(cabiCABTABS)) == hNil)
			{
			ErrorEid(eidNoMemRecord, "RecordStyleProps(t)");
			return;
			}

		for (itbd = 0; itbd < pap.itbdMac; itbd += 1)
			{
			CABTABS * pcab;
			char * pchT, sz [ichMaxNum + 1];

			pcab = *hcab;
			pcab->iAlignment = ((struct TBD * ) &pap.rgtbd[itbd])->jc;
			pcab->iLeader = ((struct TBD *) &pap.rgtbd[itbd])->tlc;
			pchT = sz;
			CchExpZa(&pchT, pap.rgdxaTab[itbd], vpref.ut, 
					ichMaxNum, fTrue);
			if (!FSetCabSz(hcab, sz, Iag(CABTABS, hszTabsPos)))
				{
				vfRecorderOOM = fTrue;
				break;
				}
			if (!FRecordCab(hcab, IDDStyleTabs, tmcTabsSet, fFalse))
				break;
			}

		FreeCab(hcab);
		}

	/* Note that EndRecordNextCmd is not called here because RecordStyleProps
	 * may be called multiple times (as in DefineStyles) */
}


csconst char csstBold [] = StKey("Bold ", CmdBold);
csconst char csstItalic [] = StKey("Italic ", CmdItalic);
csconst char csstSmallCaps [] = StKey("SmallCaps ", CmdSmallCaps);
csconst char csstHidden [] = StKey("Hidden ", CmdHidden);
csconst char csstUnderline [] = StKey("Underline ", CmdUnderline);
csconst char csstWordUnderline [] = StKey("WordUnderline ", CmdWordUnderline);
csconst char csstDblUnderline [] = StKey("DoubleUnderline ", CmdDblUnderline);
csconst char csstFont [] = StKey("Font ", CmdFont);
csconst char csstFontSize [] = StKey("FontSize ", CmdFontSize);
csconst char csstSuperscript [] = StKey("Superscript ", CmdSuperscript);
csconst char csstSubscript [] = StKey("Subscript ", CmdSubscript);
csconst char csstResetChar [] = StKey("ResetChar", CmdResetChar);

csconst char csstLeftPara [] = StKey("LeftPara", CmdLeftPara);
csconst char csstRightPara [] = StKey("RightPara", CmdRightPara);
csconst char csstJustifyPara [] = StKey("JustifyPara", CmdJustifyPara);
csconst char csstCenterPara [] = StKey("CenterPara", CmdCenterPara);

csconst char csstHdrDist [] = StKey("EditHeaderFooter .HeaderDistance=",
CmdHdrDist);
csconst char csstFtrDist [] = StKey("EditHeaderFooter .FooterDistance=",
CmdFtrDist);



/* %%Function:RecordGrpprl %%Owner:bradch */
RecordGrpprl(grpprl, cb)
char * grpprl;
int cb;
{
	extern struct ESPRM dnsprm [];

	char * pprl;
	int val;

#ifdef DRECORDER
	CommSz(SzShared("RecordGrpprl.\r\n"));
#endif
	pprl = grpprl;
	while (cb > 0)
		{
		int cchPrl;
		char * pval;

		pval = pprl + 1;
		Assert((uns) dnsprm[*pprl].cch <= 3);
		val = dnsprm[*pprl].cch == 2 ? *pval : *((int *) pval);
		RecordSprmVal(*pprl, val);
		cchPrl = CchPrl(pprl);
		pprl += cchPrl;
		cb -= cchPrl;
		}

	if (vfRecordNext)
		EndRecordNextCmd();
}


/* %%Function:RecordSprmVal %%Owner:bradch */
RecordSprmVal(sprm, val)
{
	char far * lpst;
	char stBuf [40];
	char szStyle [cchNameMax + 1];
	BOOL fWArg, fZaArg;
	char * szArg;

#ifdef DRECORDER
	CommSz(SzShared("RecordSprmVal.\r\n"));
#endif
	lpst = NULL;
	fWArg = fTrue;
	fZaArg = fFalse;
	szArg = NULL;

	switch (sprm)
		{
	case sprmSDyaHdrTop:
		lpst = csstHdrDist;
		fZaArg = fTrue;
		fWArg = fFalse;
		break;

	case sprmSDyaHdrBottom:
		lpst = csstFtrDist;
		fZaArg = fTrue;
		fWArg = fFalse;
		break;

	case sprmCPlain:
		lpst = csstResetChar;
		fWArg = fFalse;
		break;

	case sprmCFBold:
		lpst = csstBold;
		break;

	case sprmCFItalic:
		lpst = csstItalic;
		break;

	case sprmCFSmallCaps:
		lpst = csstSmallCaps;
		break;

	case sprmCFVanish:
		lpst = csstHidden;
		break;

	case sprmCKul:
		switch (val)
			{
		case kulNone:
		case kulSingle:
			lpst = csstUnderline;
			break;

		case kulWord:
			lpst = csstWordUnderline;
			break;

		case kulDouble:
			lpst = csstDblUnderline;
			break;
			}
		break;

	case sprmPJc:
		switch (val)
			{
		case jcLeft:
			lpst = csstLeftPara;
			break;

		case jcRight:
			lpst = csstRightPara;
			break;

		case jcCenter:
			lpst = csstCenterPara;
			break;

		case jcBoth:
			lpst = csstJustifyPara;
			break;
			}

		fWArg = fFalse;
		break;

	case sprmPDyaBefore:
	case sprmPDxaRight:
	case sprmPDxaLeft:
	case sprmPDxaLeft1:
	case sprmTDxaLeft:
	case sprmSDxaColumns:
		RecordRulerMargins(sprm, val);
		break;

	case sprmPChgTabs:
#ifdef REVIEW /* bradch: don't have dxaIns, dxaDel, or tbd! */
		RecordRulerTab(dxaIns, dxaDel, tbd);
#endif
		break;

	case sprmPStc:
			{
			int cstcStd;
			struct STSH * pstsh;

			pstsh = &PdodDoc(selCur.doc)->stsh;
			cstcStd = pstsh->cstcStd;
			GenStyleNameForStcp(szStyle, pstsh->hsttbName, cstcStd, 
					StcpFromStc(val, cstcStd));
			StToSzInPlace(szStyle);
			fWArg = fFalse;
			szArg = szStyle;
			}
		break;

	case sprmCFtc:
		lpst = csstFont;
		fWArg = fFalse;
#define PffnI(i)	((struct FFN *) PstFromSttb(vhsttbFont, i))
		szArg = PffnI(IbstFontFromFtcDoc(val, selCur.doc))->szFfn;
		break;

	case sprmCHps:
		lpst = csstFontSize;
		fWArg = fFalse;
		fZaArg = fTrue;
		break;

	case sprmCHpsPos:
		if (val > 0)
			{
			lpst = csstSuperscript;
			}
		else
			{
			lpst = csstSubscript;
			}
		val = fTrue;
		break;
		}

	if (lpst != NULL)
		{
		CopyCsSt(lpst, stBuf);
		RecordSt(stBuf);
		if (fWArg)
			RecordInt(val);
		if (fZaArg)
			RecordZa(val);
		if (szArg != NULL)
			RecordQuotedRgch(szArg, CchSz(szArg) - 1);
		RecordEop();
		}
}


/* %%Function:RecordRulerDoc %%Owner:bradch */
RecordRulerDoc(fRight, xa)
{
	RecordMargin(fRight ? 2 : 0, xa);
}


/* %%Function:RecordMargin %%Owner:bradch */
RecordMargin(ircs, za)
{
	char stBuf [80];

	CopyCsSt(csstFmtDoc, stBuf);
	RecordSt(stBuf);

	switch (ircs)
		{
	case 0: /* left */
		CopyCsSt(csstLeftMargin, stBuf);
		break;

	case 1: /* top */
		CopyCsSt(csstTopMargin, stBuf);
		break;

	case 2: /* right */
		CopyCsSt(csstRightMargin, stBuf);
		break;

	case 3: /* bottom */
		CopyCsSt(csstBottomMargin, stBuf);
		break;
		}

	RecordSt(stBuf);
	RecordZa(za);
	RecordEop();

	if (vfRecordNext)
		EndRecordNextCmd();
}
