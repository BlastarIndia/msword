/* S A V E . C */
/*  Save command and save preperation */

#ifdef PCJ
/* #define DSAVE */
#endif /* PCJ */

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
#define NOMINMAX
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

#include "doc.h"
#include "props.h"
#include "sel.h"
#include "disp.h"
#include "fkp.h"
#include "file.h"
#undef HEADER
#include "ch.h"
#include "format.h"
#include "doslib.h"
#include "prm.h"
#include "ruler.h"
#include "debug.h"
#include "error.h"
#include "pic.h"
#include "menu2.h"
#include "opuscmd.h"
#include "savefast.h"
#include "prompt.h"
#include "message.h"
#include "cmdtbl.h"
#include "dlbenum.h"
#include "dmdefs.h"
#include "core.h"
#include "filecvt.h"
#include "rerr.h"
#include "field.h"
#include "rareflag.h"
#include "version.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "inter.h"
#include "strtbl.h"

#include "saveas.hs"
#include "saveas.sdm"


#ifdef PROTOTYPE
#include "save.cpt"
#endif /* PROTOTYPE */

/* CAB Extension for the Save As dialog */
typedef struct _cesa
	{
	int doc;
	BOOL fQuicksaveOK;
	BOOL fQuicksaveLast;
	int wOld; /* SDM bug work around */
} CESA;



/* word offsets for FRebuildSubdocPlcpcd */
#define ddsrHdr (offset(DSR, ccpHdr)/sizeof(int))
#define ddsrFtn (offset(DSR, ccpFtn)/sizeof(int))
#define ddsrMcr (offset(DSR, ccpMcr)/sizeof(int))
#define ddsrAtn (offset(DSR, ccpAtn)/sizeof(int))
#define edcDrpHdr (offset(DOD, docHdr)/sizeof(int))
#define edcDrpMcr (offset(DOD, docMcr)/sizeof(int))


extern BOOL             fElActive;
extern int             vdocFetch;
extern struct FLI      vfli;
extern CHAR	    	szBak[];
extern CHAR	    	szDoc[];
extern HQ        	vhqchTblBackup;
extern struct SAB       vsab;
extern HWND             vhwndApp;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern int              wwCur;
extern int              mwCur;
extern int              docMac;
extern struct STTB    **vhsttbCvt;
extern struct SEL       selCur;
extern struct DOD       **mpdochdod[];
extern struct FCB       **mpfnhfcb[];
extern struct MERR      vmerr;
extern struct RULSS     vrulss;
extern struct WWD       **mpwwhwwd[];
extern struct BPTB      vbptbExt;
extern CHAR             szApp[];
extern KMP              **vhkmpUser;
extern int              fnFetch;
extern struct CHP       vchpFetch;
extern struct PREF      vpref;
extern CHAR             stDOSPath[];
extern struct QSIB      *vpqsib;
extern int              vfFileCacheDirty;
extern struct DMFLAGS   DMFlags;
extern int              fnDMEnum;
extern HPSYT 		vhpsyt;
extern struct PPR     **vhpprPRPrompt;
extern struct ITR	vitr;

#ifdef DEBUG
extern struct DBS vdbs;
#endif  /* DEBUG */

CHAR *PchFillPchId();
void                    AppendRgchToFnRtf();
#ifdef NOASM
FC                      FcAppendRgchToFn();
#endif /* NOASM */
CMD                     CmdDoSaveDoc();
BOOL			FTranslFields();

/* G L O B A L S */
int vfDeletePad;         /* plcpad should be deleted for successful save */
int vfScanPad;           /* set to true for first call to FQuicksaveOk */
struct PPR              **vhpprSave = hNil;


struct BKINFO   /* Communication from FBackupStFile to caller */
	{
	int fn;                     /* fn of backup file, fnNil if none */
	int fErr;                   /* Serious error doing backup, do not proceed */
	CHAR stFile [ichMaxFile];   /* Normalized name of backup file */
};

#define cbBKINFO (sizeof( struct BKINFO ))


/* D O  S A V E */
/* %%Function:DoSaveDoc  %%Owner:peterj */
DoSaveDoc(doc, fPromptSI)
int doc;
BOOL fPromptSI;
{
	CMB cmb;

	if ((cmb.hcab = HcabAlloc(cabiCABSAVE)) == hNil)
		{
		if (fElActive)
			RtError(rerrOutOfMemory);
		if (DiDirtyDoc(doc))
			DoEmergSave(doc);
		return;
		}

	cmb.cmm = cmmNormal;

	CmdDoSaveDoc(doc, fFalse/*fSaveAs*/, fPromptSI, &cmb);

	FreeCab(cmb.hcab);
}


/* C M D   S A V E */
/* %%Function:CmdSave  %%Owner:peterj */
CMD CmdSave(pcmb)
CMB * pcmb;
{
	CMD cmd;
	BOOL fDirty;

	Assert(pcmb->hcab == hNil);

	if (selCur.doc == docNil)
		{
		Beep ();
LError:
		cmd = cmdError;
		goto LRet;
		}

	if ((pcmb->hcab = HcabAlloc(cabiCABSAVE)) == hNil)
		{
		if (fElActive)
			RtError(rerrOutOfMemory);
		if (DiDirtyDoc(selCur.doc))
			DoEmergSave(selCur.doc);
		goto LError;
		}

	pcmb->cmm = cmmDefaults | cmmAction;

	fDirty = DiDirtyDoc(DocMother(selCur.doc));

	if ((cmd = CmdDoSaveDoc(selCur.doc, fFalse /* fSaveAs */, 
			vpref.fPromptSI, pcmb)) == cmdOK)
		{
		DoPostSaveDoc(pcmb, fDirty);
		}

	if (pcmb->hcab != hNil)
		FreeCab(pcmb->hcab);
	pcmb->hcab = hNil;

LRet:
	/* at all costs, avoid telling the user to save his work when giving an
	error message for save! */
	if (vmerr.mat == matMem)
		vmerr.mat = matLow;

	return cmd;
}


/* C M D   S A V E   A S */
/* %%Function:CmdSaveAs  %%Owner:peterj */
CMD CmdSaveAs(pcmb)
{
	CMD cmd;
	BOOL fDirty;

	if (selCur.doc == docNil)
		{
		/* This is all but guaranteed not to happen! */
		Beep ();
		cmd = cmdError;
		goto LRet;
		}

	fDirty = DiDirtyDoc(DocMother(selCur.doc));

	if ((cmd = CmdDoSaveDoc(selCur.doc, fTrue /* fSaveAs */, 
			vpref.fPromptSI, pcmb)) == cmdOK)
		{
		DoPostSaveDoc(pcmb, fDirty);
		}
LRet:
	/* at all costs, avoid telling the user to save his work when giving an
	error message for save! */
	if (vmerr.mat == matMem)
		vmerr.mat = matLow;

	return cmd;
}




/* C M D  S A V E  D O T */
/*  Save the DOT of the current document.
*/
/* %%Function:CmdSaveDot  %%Owner:peterj */
CMD CmdSaveDot(pcmb)
CMB * pcmb;
{
	CMD cmd;
	BOOL fRecordingSav;
	int docDot = DocDotMother (selCur.doc);

	if (docDot == docNil)
		return cmdOK;

	Assert(pcmb->hcab == hNil);

	if ((pcmb->hcab = HcabAlloc(cabiCABSAVE)) == hNil)
		{
		if (fElActive)
			RtError(rerrOutOfMemory);
		if (DiDirtyDoc(docDot))
			DoEmergSave(docDot);
		cmd = cmdError;
		goto LRet;
		}

	/* We always need to fill the just-allocated Save cab */
	pcmb->fDefaults = fTrue;

	/* Don't record the SaveAs */
	InhibitRecorder(&fRecordingSav, fTrue);
	cmd = CmdDoSaveDoc(docDot, fFalse /* fSaveAs */, fFalse, pcmb);
	InhibitRecorder(&fRecordingSav, fFalse);

	FreeCab(pcmb->hcab);
	pcmb->hcab = hNil;
LRet:
	/* at all costs, avoid telling the user to save his work when giving an
	error message for save! */
	if (vmerr.mat == matMem)
		vmerr.mat = matLow;

	return cmd;
}


/* C M D  S A V E  A L L */
/*  Save all dirty documents and document types.
*/
/* %%Function:CmdSaveAll  %%Owner:peterj */
CMD CmdSaveAll(pcmb)
CMB * pcmb;
{
	if (FConfirmSaveAllDocs(acSaveAll))
		return cmdOK;

	/* at all costs, avoid telling the user to save his work when giving an
	error message for save! */
	if (vmerr.mat == matMem)
		vmerr.mat = matLow;

	return cmdCancelled;
}





/* %%Function:DoPostSaveDoc  %%Owner:peterj */
DoPostSaveDoc(pcmb, fDirty)
CMB * pcmb;
BOOL fDirty;
{
	/*  save the dot if appropriate */
	SaveDotToo (pcmb, fDirty);
	if (fnDMEnum != fnNil)
		DMFlags.dma = dma_StartFromScratch;
	if (vmerr.fFnFull)
		FExecCmd (bcmCloseWnd);
}




/* S A V E  D O T  T O O */
/*  Checks if we should save the dot of selCur.doc (if any).  If so does
	the appropriate ChainCmd possibly setting up the cab.
*/

/* %%Function:SaveDotToo   %%Owner:peterj */
SaveDotToo (pcmb, fDirty)
CMB * pcmb;
BOOL fDirty;
{
	int doc;

	if (selCur.doc == docNil || mpdochdod[selCur.doc] == hNil
			|| DiDirtyDoc (selCur.doc)
			|| !fDirty
			|| (doc = DocDotMother (selCur.doc)) == docNil
			|| ! DiDirtyDoc (doc) || WwDisp(doc,wwNil,fFalse) != wwNil)
		/*  the original doc was not saved, or there is no dot, or
				the dot isn't dirty, or the dot is in a user window.
				--> don't save it. */
		{
		return;
		}

	if (IdMessageBoxMstRgwMb (mstSaveDotToo, NULL, MB_YESNO | 
			MB_APPLMODAL | MB_DEFBUTTON1 | MB_ICONQUESTION)  !=  IDYES )
		return;

	ChainCmd (bcmSaveDot);
}




/* C M D  D O  S A V E  D O C */
/* %%Function:CmdDoSaveDoc  %%Owner:peterj */
CMD CmdDoSaveDoc(doc, fSaveAs, fPromptSI, pcmb)
int doc;
BOOL fSaveAs, fPromptSI;
CMB *pcmb;
{
	CMD cmd = cmdOK;
	HCABSAVE hcab = pcmb->hcab;
	CABSAVE * pcab;
	BOOL fFileNameChanged = fFalse;
	BOOL fGuaranteeSave = !fElActive;/* never guarantee for macro language */
	CESA cesa;
	CHAR stFile [ichMaxFile];
	char stDOSPathOld [ichMaxFile + 1];
	extern BOOL vfRecording;

	AssertH(hcab);

	/* avoid stack overflow */
	ReturnOnNoStack(6250,cmdError,fTrue);

#ifdef DEMO
	if (CpMacDocEdit(DocMother(doc)) > cpMaxDemo)
		{
		ErrorEid(eidDemoCantSave, "CmdDoSaveDoc");
		return cmdError;
		}
#endif /* DEMO */

#ifdef DCORELOAD
	CommSz(SzShared("\t\tEntering Save\r\n"));
#endif /* DCORELOAD */

	if (vhsttbCvt == hNil)
		FInitCvt();

	CopySt(stDOSPath, stDOSPathOld);

	doc = DocMother (doc);

	vfScanPad = fTrue;
	vfDeletePad = fFalse;

	/* allocates Quicksave memory block, freed below */
	cesa.fQuicksaveOK = FQuicksaveOK(doc);
	cesa.doc = doc;
	pcmb->pv = &cesa;

	if (pcmb->fDefaults)
		{
		if (!FFillCabSave(pcmb, &cesa))
			{
			cmd = cmdNoMemory;
			goto LCleanup;
			}
		if (!pcmb->fDialog && !pcmb->fAction)
			{
			/* NOTE: don't worry about freeing Quicksave memory
				block in this case because it will get freed next
				time FQuicksaveOK() is called. */
			return cmdOK;
			}
		}

	pcab = *hcab;
	/* Macro language (and repeat) uses 100 for first external fmt */
	if (pcab->iFmt >= 100)
		pcab->iFmt -= 100 + dffSaveMin;
	cesa.fQuicksaveLast = pcab->fQuicksave;
	cesa.wOld = pcab->iFmt;

	if (vrulss.caRulerSprm.doc != docNil)
		FlushRulerSprms();


	if (!fSaveAs)
		{
		/* User did not request the dialog box; check here to
			see if we should throw it at them anyway... */

		if (PdodDoc(doc)->fn == fnNil)
			{
			if (PdodDoc(doc)->udt != udtGlobalDot)
				goto LSaveAs;

			/* Saving NORMAL.DOT... */
			GetDocSt(doc, stFile, gdsoFullPath);
			if (FExistsStFile(stFile, fFalse))
				goto LSaveAs;
			}

		/* FCheck... normalizes filename filling out stFile, 
			and assures save legal */
		if (FCheckSaveCab(hcab, &cesa, stFile))
			{
			/*  check for overwritting foreign file */
			switch (IdOverwriteForeign(doc, stFile, (*hcab)->iFmt))
				{
			case IDYES:
				break; /* overwrite */

			case IDNO:
				/* resort to save as */
				goto LSaveAs;

			default:
				/* cancel */
				cmd = cmdCancelled;
				goto LCleanup;
				}
			}
		else
			{
LSaveAs:
			pcmb->fDialog = fTrue;
			fSaveAs = fTrue;
			}
		}

	if (fSaveAs)
		{
		char stT [ichMaxFile];

		if (pcmb->fDialog)
			{
			extern BOOL FRecordCab();

			/* Pause recorder so that summary dialog can
				get recorded first! */
			if (vfRecording)
				EndSdmTranscription();

			cmd = CmdDoSaveAsDlg(pcmb);

			if (vfRecording) /* Restart recorder */
				SaveCabs(FRecordCab, fFalse);

			if (cmd != cmdOK)
				goto LCleanup;

			GetCabSt(hcab, stT, sizeof (stT), Iag(CABSAVE, hszFile));
			AssertDo(FNormalizeStFile(stT, stFile, 
					(*hcab)->iFmt == ISaveFmtDff(dffSaveDocType) ? 
					nfoDot : nfoNormal));
			}

		if (pcmb->fCheck)
			{
			CheckElSaveAsCab(pcmb);
			if (!FCheckSaveCab(hcab, &cesa, stFile))
				{
				cmd = cmdError;
				goto LCleanup;
				}
			}

		if (FUnderstoodDff(DffISaveFmt((*hcab)->iFmt), fTrue))
			/* see if the name has changed */
			{
			GetDocSt(doc, stT, gdsoFullPath|gdsoNoUntitled);
			if (*stT && FNeSt(stT, stFile))
				/* name has changed, all new info! */
				{
				PdodDoc(doc)->fPromptSI = fTrue;
				ApplyDocMgmtNew(doc, fnNil);
				fFileNameChanged = fTrue;
				}
			}
		}

#ifdef DCORELOAD
	CommSz(SzShared("\t\tDlg Complete, begin saving\r\n"));
#endif /* DCORELOAD */


	if (!pcmb->fAction)
		{
		goto LCleanup;
		}


	if (FUnderstoodDff(DffISaveFmt((*hcab)->iFmt), fTrue))
		{
		/* update doc management information before prompting for
			summary info.  if the save fails, this information will
			be wrong, but not too big of a deal since it isn't on
			disk. */
		ApplyDocMgmtSave(doc); /* before prompting for SI */

		if (fPromptSI && WwDisp(doc, wwNil,fFalse) != wwNil && PdodDoc(doc)->fPromptSI)
			{
			BOOL  fDirty;

			/* HM */
			if (TmcDoDocSummary((CMB *) NULL /* implies local cab */, 
					doc, &fDirty,
					(PdodDoc(doc)->fLockForEdit && !fFileNameChanged) ?
					fFalse : fTrue /*fEnableOK*/, 
					stFile, fTrue)
					!= tmcOK)
				{
				cmd = cmdCancelled;
				PdodDoc(doc)->dop.nRevision--;/* incr in ApplyDocMgmtSave */
				goto LCleanup;
				}
			PdodDoc(doc)->fPromptSI = fFalse;
			}
		}
	else
		/* don't do the "RESCUE.DOC" stuff for non-opus files */
		fGuaranteeSave = fFalse;

	/* Recorded now so summary gets recorded first! */
	if (fSaveAs && vfRecording)
		{
		int iFmtSav;

		/* If SaveTemplate was used, the doc is garunteed to have
			a name, so this should be fine... */
		Assert(pcmb->bcm != bcmSaveDot);
		if (FNeNcSt(stDOSPath, stDOSPathOld))
			RecordChdir(stDOSPath);

		/* Recorder uses 100 for first external fmt... */
		pcab = *hcab;
		if ((iFmtSav = (int) pcab->iFmt) >= -dffSaveMin)
			pcab->iFmt += 100 + dffSaveMin;

		FRecordCab(hcab, IDDSaveAs, tmcOK, fTrue);
		pcmb->bcm = imiSaveAs; /* For recorder... */

		/* Restore cab entry as it's used below... */
		(*hcab)->iFmt = iFmtSav;
		}

	/* make sure header has been saved first */
	FCleanUpHdr(doc, fFalse, fTrue /* fKillEmpty */);

	Profile ( vpfi == pfiSaveFile ? StartProf (30) : 0 );

	/*  perform the actual save */
	PdodDoc(doc)->dop.fLockAtn = (*hcab)->fLockAnnot;
	if (FSaveFile(stFile, doc, DffISaveFmt((*hcab)->iFmt), 
			(*hcab)->fQuicksave, (*hcab)->fBackup, fTrue /* fReport */))
		{
		/*  display number of characters written in prompt */
		CP cp = CpMacDocEdit(doc);
		unsigned rgw [1 + (sizeof (long)/sizeof (unsigned))];

		/*  display prompt:  shortfile.ext: 123456 Ch. */
		rgw [0] = cesa.doc;
		bltb (&cp, &rgw[1], sizeof (CP));
		Assert (sizeof (CP) == sizeof (long));
		SetPromptRgwMst (mstNumChars, rgw, pdcAdvise);
		OldestCmg (cmgPromptCode);

		cmd = cmdOK;
		}
	else
		{
		cmd = cmdError;
		StopProgressReport(hNil, pdcRestoreImmed);
		}

	vhpprSave = hNil;

	Profile ( vpfi == pfiSaveFile ? StopProf () : 0 );

#ifdef DCORELOAD
	CommSz(SzShared("\t\tSave completed\r\n"));
#endif /* DCORELOAD */

LCleanup:
	if (cmd == cmdCancelled)
		fGuaranteeSave = fFalse;

	if (vhqchTblBackup != hqNil)
		{
		FreeHq(vhqchTblBackup);
		vhqchTblBackup = hqNil;
		}

	/*Change all window titles to reflect both path and filename changes*/
	UpdateTitles();

#ifdef DISABLE
	/* this does not work because of a windows bug (cannot pass 0L to SendMessage
	of WM_WININICHANGE). pj */
	/* special hack for win.ini */
	if (cmd == cmdOK)
		{
		struct FNS fns;
		StFileToPfns(stFile, &fns);
		fns.stPath[0] = 0;
		PfnsToStFile(&fns, stFile);
		if (FEqNcSt(stFile, StSharedKey("WIN.INI",winini)))
			/* we have changes win.ini! */
			{
#ifdef PCJ
			CommSz(SzShared("win.ini changed!!\r\n"));
#endif /* PCJ */
			SendMessage(-1, WM_WININICHANGE, 0, 0L);
			}
		}
#endif /* DISABLE */

	if (vmerr.fMemFail && mpdochdod[doc] != hNil &&	DiDirtyDoc(doc)
			&& fGuaranteeSave)
		{
		DoEmergSave(doc);
		}

	OldestCmg (cmgSaveCode);

	/* Repeat uses 100 for first external fmt... */
	pcab = *hcab;
	if ((int) pcab->iFmt >= -dffSaveMin)
		pcab->iFmt += 100 + dffSaveMin;

	return cmd;
}





/* F F I L L  C A B  S A V E */
/* Fill out cab for Save/Save As.  The dialog is not called.
*/

/* %%Function:FFillCabSave  %%Owner:peterj */
FFillCabSave(pcmb, pcesa)
CMB *pcmb;
CESA * pcesa;
{
	int doc = pcesa->doc;
	struct DOD *pdod;
	CABSAVE * pcabsave = *pcmb->hcab;
	CHAR stDefault [ichMaxFile];

	FreezeHp();
	pdod = PdodDoc(doc);
	pcabsave->fBackup = pdod->dop.fBackup;
	pcabsave->iDirList = uNinchList;

	/* determine quicksave */
	pcabsave->fQuicksave = pcesa->fQuicksaveOK;

	/* determine format */
	Assert(pdod->fDoc || pdod->fDot);
	if (pdod->fFormatted || pdod->fn == fnNil)
		pcabsave->iFmt = pdod->fDot ? ISaveFmtDff(dffSaveDocType) : 
				ISaveFmtDff(dffSaveNative);
	else
		{
		Assert (!pdod->fDot);
		pcabsave->iFmt = ISaveFmtDff(dffSaveText);
		}

	pcabsave->fLockAnnot = pdod->dop.fLockAtn;
	MeltHp();

	GetDocSt(doc, stDefault, gdsoNoUntitled|gdsoRelative|gdsoNotRelDot);
	FSetCabSt(pcmb->hcab, stDefault, Iag(CABSAVE, hszFile)); /* HEAP */
	/* fMemFail will be true if FSetCabSt fails */
	return (!vmerr.fMemFail);
}


/* C M D  D O  S A V E  A S  D L G */
/* %%Function:CmdDoSaveAsDlg  %%Owner:peterj */
CMD CmdDoSaveAsDlg(pcmb)
CMB * pcmb;
{
	CABSAVE * pcab;
	CMD cmd;
	BCM bcmSav;
	char dlt [sizeof (dltSaveAs)];

	Assert(pcmb->pv != NULL);

	BltDlt(dltSaveAs, dlt);

	bcmSav = pcmb->bcm;
	pcmb->bcm = imiSaveAs;

	if (pcmb->cmm == cmmDialog)
		CheckElSaveAsCab(pcmb);

	pcab = *pcmb->hcab;
	pcab->sab = 0;
	pcab->fOptions = fFalse;

	switch (TmcOurDoDlg(dlt, pcmb))
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		cmd = cmdError;
		break;
#endif
	case tmcError:
		cmd = cmdError;
		break;

	case tmcCancel:
		cmd = cmdCancelled;
		break;

	case tmcOK:
		cmd = cmdOK;
		}

	pcmb->bcm = bcmSav;

	UpdateStDOSPath();

	return cmd;
}





/* D L G F  S A V E  A S */
/*  Dialog function for "Save As" dialog.
*/

/* %%Function:FDlgSaveAs  %%Owner:peterj */
BOOL FDlgSaveAs(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CHAR stFile [ichMaxBufDlg];
	CHAR   stUser[ichMaxFile + 2];
	HCABSAVE hcab;
	CESA * pcesa;
	int fn;

	pcesa = PcmbDlgCur()->pv;

	switch (dlm)
		{
	case dlmInit:
		Assert(HcabDlgCur());
		GetCabSz(HcabDlgCur(), stFile, ichMaxBufDlg, Iag(CABSAVE, hszFile));
		GrayBtnOnSzBlank(stFile, tmcOK);
		break;
	case dlmIdle:
		/* we can do idle time processing here.  wNew is cIdle */
		switch (wNew)
			{
		case 0:
			return fTrue;  /* call FSdmDoIdle and keep idling */
		case 2:
#ifdef DCORELOAD
			CommSz(SzShared("\t\tLoading save code\r\n"));
#endif /* DCORELOAD */

			FAbortNewestCmg(cmgPromptCode, fFalse, fTrue);
			FAbortNewestCmg(cmgPromptUtilNAC, fFalse, fTrue);
			FAbortNewestCmg(cmgSaveCode, fFalse, fTrue);
			FAbortNewestCmg(cmgSaveUtil1, fFalse, fTrue);
			FAbortNewestCmg(cmgSaveUtil2, fFalse, fTrue);
			break;
		case 4:
			FAbortNewestCmg(cmgPromptCode, fTrue/*fLoad*/, fTrue);
			break;
		case 6:
			FAbortNewestCmg(cmgPromptUtilNAC, fTrue/*fLoad*/, fTrue);
			break;
		case 8:
			FAbortNewestCmg(cmgSaveCode, fTrue/*fLoad*/, fTrue);
			break;
		case 10:
			FAbortNewestCmg(cmgSaveUtil1, fTrue/*fLoad*/, fTrue);
			break;
		case 12:
			FAbortNewestCmg(cmgSaveUtil2, fTrue/*fLoad*/, fTrue);
#ifdef DCORELOAD
			CommSz(SzShared("\t\tForced loading done\r\n"));
#endif /* DCORELOAD */
			break;
			}
		return (wNew > 12);

	case dlmDblClk:
		if (tmc == tmcSavDirList)
			{
			char * pchT;

			/* Change directories */
			if ( !FGetDirListEntry(tmcSavDirList, stUser + 1, 
					sizeof (stUser) - 1) )
				{
				/* should only happen temporarily when clicking in empty space below dirs */
				Beep();
				return fFalse;
				}
			/* needs an stz */
			SzSzAppend(stUser + 1, SzShared("*.*"));
			*stUser = CchSz(stUser + 1) - 1;
			FOurFillDirListTmc(stUser, tmcSavDirList, tmcSavDir,
					fdirSubDirectories | fdirDrives | fdirXOR);

			/* Remove directory from name */
			GetTmcText(tmcSavFile, stUser, sizeof (stUser));
			pchT = stUser - 1 + 
					CchStripString(stUser, CchSz(stUser) - 1);
			while (pchT > stUser && *pchT != '\\' && *pchT != ':')
				pchT -= 1;
			SetTmcText(tmcSavFile, pchT + 1);
			SetFocusTmc(tmcSavFile);
			GrayButtonOnBlank(tmcSavFile, tmcOK);

			UpdateStDOSPath();
			}
		return fFalse; /* never leave SaveAs because of a dbl click */

	case dlmTerm:
		if (tmc != tmcOK)
			return fTrue;

		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			{
			/* dialog will end; no need to call EndDlg(tmcError) */
			return (fTrue);
			}

		/* Check for and deal with wild cards and directory changes
					Doc type can't change dir. If you type a dir name you will
					get invalid file name.
				*/

		if (FChangeDlgSaveAsPath(pcesa->doc, (*hcab)->iFmt, stFile,
				stUser))
			goto LStayIn;

		/*returns legal, normal filename, possibly modify fQuicksave*/
		if (!FCheckSaveCab(hcab, pcesa, stFile))
			{
LStayIn:
			SetTmcTxs(tmcSavFile, TxsAll());
			return fFalse;
			}

		/* If: saving to another name and a file of that name exists
			on the currently-on-line floppy */

		if (FExistsStFile(stFile, fFalse /* fAnywhere */) &&
				((fn = PdodDoc(pcesa->doc)->fn) == fnNil ||
				FnFromOnLineSt(stFile) != fn))
			{
			int idResult;
			CHAR *pch = stUser;

			SzShortFromStNormFile(stUser, stFile);

			idResult = IdMessageBoxMstRgwMb (mstReplaceFile, &pch,
					MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION );

			switch (idResult)
				{
				/* Response to "Replace Existing FOO.DOC?" */
			default:
			case IDCANCEL:/* CANCEL - exit, cancel Save */
				EndDlg(tmcCancel);  /* ok to call enddlg here */
				return fTrue;  /* must return true bz */

			case IDYES:   /* YES - exit, complete Save */
				break;

			case IDNO:    /* NO - stay in dialog */
				SetTmcTxs(tmcSavFile, TxsAll());
				return fFalse;
				}
			}

		/*  check for overwritting foreign file */
		switch (IdOverwriteForeign (pcesa->doc, stFile, (*hcab)->iFmt))
			{
		case IDYES:
			break; /* overwrite */

		case IDNO:
			SetTmcTxs(tmcSavFile, TxsAll());
			return fFalse;

		default:
			/* cancel */
			EndDlg(tmcCancel);  /* ok to call enddlg here */
			return fTrue;  /* must return true bz */
			}
		break;

	case dlmChange:
		if  (tmc == tmcSavFile)
			GrayButtonOnBlank(tmc, tmcOK);
		break;

	case dlmClick:
		switch (tmc)
			{
			CHAR szAll[6];

		case tmcSavOptions:
				{
				int fOkToChange;

			 /* FSetDlgSab not safe if dialog coming down */
				if (FIsDlgDying() || !FSetDlgSab(sabSAOptions))
					{
					Beep();
					return fFalse;
					}

				if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
					return fFalse;
				if (hcab == hcabNull)
					{
					/* sdm filter msg will take dialog down */
					return (fTrue);
					}

				EnableTmc(tmcSavBackup, fTrue);

				EnableTmc(tmcSavQuick,
						FUnderstoodDff (DffISaveFmt((*hcab)->iFmt), 
						fTrue/*binary*/)
						&& pcesa->fQuicksaveOK);

				fOkToChange = !PdodDoc(pcesa->doc)->dop.fLockAtn ||
						FAuthor(pcesa->doc);

				/* if not saving a Document Type, and not locked by non-owner,
				   or at exit windows time, where we cannot have gosub dialogs
				   that a convertor may bring up,  enable format */

				if ((*hcab)->iFmt != ISaveFmtDff(dffSaveDocType) &&
						fOkToChange && !vrf.fInQueryEndSession)
					EnableTmc(tmcSavIfmt, fTrue);
				else
					{
					/* was true; can't tab to static text so
									only disable if visible */
					EnableTmc(tmcSavDfmtPrompt, fFalse);
					EnableTmc(tmcSavIfmt, fFalse);
					}

				/* Lock for annotation button */
				fOkToChange = fOkToChange && 
						((*hcab)->iFmt == ISaveFmtDff(dffSaveNative)
						|| (*hcab)->iFmt == ISaveFmtDff(dffSaveDocType));

				EnableTmc(tmcSavLockAnnot, fOkToChange);

				/* focus goes to first enabled item in option area */
				if (FEnabledTmc(tmcSavIfmt))
					SetFocusTmc(tmcSavIfmt);
				else  if (FEnabledTmc(tmcSavQuick))
					SetFocusTmc(tmcSavQuick);
				else
					SetFocusTmc(tmcSavBackup);

				EnableTmc(tmcSavOptions, fFalse);

				return fFalse; /* do not record option button! */
				}

		case tmcSavDirList:
			BuildSzAll(szDoc, szAll);
			NewShowNewDirTmc(tmcSavFile, tmcSavDirList, szAll);
			/* this shouldn't be needed if we got a dlmChange when
								text went to the edit control but... */
			GrayButtonOnBlank(tmcSavFile, tmcOK);
			UpdateStDOSPath();
			break;

		case tmcSavBackup:
			if (wNew != 0)
				SetTmcVal(tmcSavQuick, fFalse);
			break;

		case tmcSavQuick:
			if (wNew != 0)
				SetTmcVal(tmcSavBackup, fFalse);
			break;

		case tmcSavIfmt:
#ifdef DSAVE
				{
				int rgw[4];
				rgw[0] = wNew;
				rgw[1] = wOld;
				rgw[2] = pcesa->fQuicksaveLast;
				rgw[3] = pcesa->wOld;
				CommSzRgNum(SzShared("wNew,wOld,fQSL,wOld2: "), rgw, 4);
				}
#endif /* DSAVE */

			/* if saving as doc template. set dir text but
							don't change dir; clear and disable directory
							list box. If leaving template format. reenable
						*/


			if (wNew == ISaveFmtDff(dffSaveDocType) && 
					pcesa->wOld != ISaveFmtDff(dffSaveDocType))
				{
				DisableDirForTemplate(stUser);
				}
			else if (wNew != ISaveFmtDff(dffSaveDocType) &&
					pcesa->wOld == ISaveFmtDff(dffSaveDocType))
				{
				/* reenable list box and string */
				EnableTmc(tmcSavDirList, fTrue);
				EnableTmc(tmcSavDirText, fTrue);

				UpdateStDOSPath();
				StToSz(stDOSPath, stUser + 1);
				SzSzAppend(stUser + 1, SzShared("*.*"));
				*stUser = CchSz(stUser + 1) - 1;
				/* needs an stz */
				FOurFillDirListTmc(stUser, tmcSavDirList,
						tmcSavDir, 
						fdirSubDirectories | fdirDrives | fdirXOR);
				}


			if (wNew == ISaveFmtDff(dffSaveNative) || 
					wNew == ISaveFmtDff(dffSaveDocType))
				{
				if (pcesa->wOld != ISaveFmtDff(dffSaveNative) &&
						pcesa->wOld != ISaveFmtDff(dffSaveDocType))
					{
					EnableTmc(tmcSavQuick, 
							pcesa->fQuicksaveOK);
					SetTmcVal(tmcSavQuick, 
							pcesa->fQuicksaveLast);
					}
				}
			else
				{
				if (pcesa->wOld == ISaveFmtDff(dffSaveNative) || 
						pcesa->wOld == ISaveFmtDff(dffSaveDocType))
					{
					pcesa->fQuicksaveLast =
							ValGetTmc(tmcSavQuick);
					SetTmcVal(tmcSavQuick, fFalse);
					EnableTmc(tmcSavQuick, fFalse);
					}
#ifdef DEBUG
				else
					Assert (!ValGetTmc(tmcSavQuick));
#endif /* DEBUG */
				}

			pcesa->wOld = wNew;
			EnableTmc(tmcSavLockAnnot,
					((wNew == ISaveFmtDff(dffSaveNative) ||
					wNew == ISaveFmtDff(dffSaveDocType))
					&& (!PdodDoc(pcesa->doc)->dop.fLockAtn
					|| FAuthor(pcesa->doc))));
			}
		/* break; */
		}

	return fTrue;
}


/* called from FDlgSaveAs only */
/* %%Function:DisableDirForTemplate  %%Owner:peterj */
DisableDirForTemplate(stUser)
CHAR *stUser;
{
	CHAR *pch;

	/* put template dir in dir text...*/
	if (!FGetStFpi (fpiDotPath, stUser))
		/* if none, get program dir */
		AssertDo(FGetStFpi(fpiProgram, stUser));
	(*stUser)--; /* remove trailing backslash */
	StToSzInPlace (stUser);
	/* shorten path if necessary */
	pch = stUser;
	FTruncateTmcPathPsz(tmcSavDir, &pch);
	SetTmcText(tmcSavDir, pch);

	/* empty and disable directory list box */
	StartListBoxUpdate(tmcSavDirList); /* empties box and starts redraw */
	EndListBoxUpdate(tmcSavDirList);
	EnableTmc(tmcSavDirList, fFalse);
	/* gray the string too */
	EnableTmc(tmcSavDirText, fFalse);
}



/* F  O U R  F I L L  D I R  L I S T */

/* %%Function:FOurFillDirListTmc  %%Owner:peterj */
FOurFillDirListTmc(stz, tmcLB, tmcPath, fdir)
CHAR *stz;
int tmcLB, tmcPath, fdir;
{
	CHAR stzTemplate[14];   /* max wildcard size says sdm */
	int wReturn;

      /* stzTemplate now needs to contain some wildcard; shove in *.* to
		 avoid intl problems; will not be used if stz contains a wildcard
		 (all our callers do), but sdm asserts if no wildcard.
	  */
    *stzTemplate = CchCopyLpszCchMax((LPSTR)SzShared("*.*"),
			(LPSTR)(stzTemplate + 1), sizeof (stzTemplate) - 1);
	wReturn = FlbfFillDirListTmc (stz, stzTemplate, tmcLB, NULL, tmcPath, fdir, NULL);
	Assert ((*stzTemplate + 2)  <= sizeof (stzTemplate));

	if (wReturn & flbfListingMade)
		{
		UpdateStDOSPath();
		return fTrue;
		}

	return fFalse;  /* no listing made, i.e., failure */
}


/* %%Function:WListSave  %%Owner:peterj */
EXPORT WORD WListSave(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{

	/* this function is only used the first time the box is filled */
	/* we can get the value out of the cab because we know we are
			not calling it any other time, when the cab value could differ
			from the value on screen
		*/

	CHAR   stUser[ichMaxFile + 1];

	if (tmm == tmmCount)
		{
          /* don't bother if going down. Once inside, if we fail,
             all of the routines should die gracefully */
        if (!FIsDlgDying())
            {
    		Assert(HcabDlgCur());

#ifdef DEBUG
            {
            int val = ValGetTmc(tmcSavIfmt);
            if (!FIsDlgDying())  /* if valgettmc causes death */
    		    Assert( ((CABSAVE *) *(HcabDlgCur()))->iFmt == val);
            }
#endif /* DEBUG */

    		if (((CABSAVE *) *(HcabDlgCur()))->iFmt == ISaveFmtDff(dffSaveDocType))
    			DisableDirForTemplate(stUser);
    		else
    			{
    			/* Restore to the original directory. */
    			StToSz(stDOSPath, stUser + 1);
    			SzSzAppend(stUser + 1, SzShared("*.*"));
    			*stUser = CchSz(stUser + 1) - 1;
    			/* needs an stz */
    			FOurFillDirListTmc(stUser, tmcSavDirList, tmcSavDir, 
    					fdirSubDirectories | fdirDrives | fdirXOR);
    			}
			}
		return 0;
		}

	return fTrue;
}


/* %%Function:NewShowNewDirTmc  %%Owner:peterj */
NewShowNewDirTmc(tmcEdit, tmcList, szExt)
int     tmcEdit;
int     tmcList;
CHAR   *szExt;
{
	CHAR   *pchMac, *pchLeaf, *pchT, *pchUser, *pchStart;
	int     ichT, fnt;
	HWND    hwnd;
	long    lParam;
	CHAR    szT[ichMaxFile];
	int     tmcFocus;

	Assert(szExt != NULL);

	if (ValGetTmc(tmcList) == uNinchList)
		{
		*szT = '\0';
		pchStart = szT;
		}
	else
		{
		int cch;

		cch = CchGetTmcText(tmcList, szT, sizeof (szT) - 1);
		/* we assume that this is a list box of only directories and drives */
		/* strip off brackets and - */
		Assert (*szT == '[');
		/* Selection is drive */
		if (*(szT + 1) == '-')
			{
			pchStart = szT + 2;  /* skip over leading [- */
			*(szT + 3) = ':';
			*(szT + 4) = 0;
			}
		else  /* directory */
			
			{
			pchStart = szT + 1;  /* skip over leading [ */
			*(szT + cch - 1) = '\\';
			*(szT + cch ) = 0;
			}
		}
	pchMac = pchStart + CchSz(pchStart) - 1;

	GetTmcText(tmcEdit, pchUser = pchMac + 1, ichMaxFile - (pchMac - pchStart) - 1);
	CchStripString(pchUser, CchSz(pchUser) - 1);
	for (pchLeaf = pchT = pchUser; *pchT != '\0'; pchT++)
		{
		if (*pchT == ':' || *pchT == '\\' || *pchT == '/')
			{
			pchLeaf = pchT;
			}
		}
	if (pchLeaf != pchUser)
		{
		pchLeaf++;
		}

	if (*pchUser == '\0')
		{
		goto lblApExt;
		}
	else 
		
		{
		fnt = FntSz(pchUser, CchSz(pchUser) - 1, &ichT, nfoPathWildOK);
		switch (fnt)
			{
		case fntInvalid:
lblApExt:
			/* just leave dir/drive name */
			*pchMac = 0;
			break;
		case fntValid:
		case fntValidWild:
			bltbyte(pchLeaf, pchMac, CchSz(pchLeaf));
			break;
			}
		}

	SetTmcText(tmcEdit, pchStart);

}


/* F  C H A N G E  D L G  S A V E  A S  P A T H */
/* The user has pressed OK in the
SaveAs dialog.  Determine whether they were changing directories or
trying to save their file. Returns fTrue for directory.
*/

/* %%Function:FChangeDlgSaveAsPath  %%Owner:peterj */
FChangeDlgSaveAsPath(doc, iFmt, stFile, stUser)
int doc;
int iFmt;
CHAR stFile[];  /* passed in to save stack space */
CHAR stUser[];
{
	int fDocType;
	CHAR   *pchDot;
	int cch;

	fDocType = iFmt == ISaveFmtDff(dffSaveDocType);

	GetTmcText(tmcSavFile, stFile, ichMaxBufDlg);
	cch = CchStripString(stFile, CchSz(stFile) - 1);

	/* if path ending in . or .., add trailing backslash for fNormalize */
	if ((pchDot = PchFnTermPrevCur(stFile)) != NULL)
		{
		*pchDot = chBackSlash;
		*(pchDot + 1) = '\0';
		}

	pchDot = PchFnHasExt(stFile); /* see if original filename had an ext */

	SzToStInPlace(stFile);

	if (!FNormalizeStFile(stFile, stUser,
			nfoPathCk | (fDocType ? nfoDot : nfoNormal|nfoPathOK)))
		goto LErrRet;

	StToSzInPlace(stUser);

	/* we do this because FNormalize will put on an extension
				and we want to just get the full path to check for
				subdir before adding an extension
			*/
	if (pchDot == NULL)  /* no extension in original file */
		{
		pchDot = PchFnHasExt(stUser);
		if (pchDot != NULL)
			*pchDot = '\0';
		}

	if (!FValidSubdir(stUser))
		return fFalse;   /* not a directory name */

	if (fDocType)
		/* no dir names allowed for template */
		{
LErrRet:
		ErrorEid(eidBadFilename, "FChangeDlgSaveAsPath");
		return fTrue;  /* so we will stay in dlg */
		}

	/* stFile is an st at this point. Make an stz */
	stFile[*stFile + 1] = 0;
	FDlgDirAddSrch(stFile + 1, SzShared("*.*"), ichMaxBufDlg);

	/* update the stz */
	*stFile = CchSz(stFile + 1) - 1;
	/* if name is directory, change dir and stay in dialog. */
	if (FOurFillDirListTmc(stFile, tmcSavDirList, 
			tmcSavDir, fdirSubDirectories | fdirDrives | fdirXOR))
		{
		SetTmcText(tmcSavFile, szEmpty);
		SetFocusTmc(tmcSavFile);
		EnableTmc(tmcOK, fFalse);
		UpdateStDOSPath();
		return fTrue;
		}

	/* should never get here - should be dir or already gone */
	Assert (fFalse);
	return fFalse;
}


/* returns pointer past ending . or .. in path name if preceded by backslash
	colon or nothing; NULL otherwise.
*/
/* %%Function:PchFnTermPrevCur  %%Owner:peterj */
PchFnTermPrevCur(sz)
CHAR *sz;
{
	CHAR *pch;
	int cch = CchSz(sz) - 1;  /* excludes null term */

	if (cch < 1)
		return (NULL);

	pch = sz + cch - 1;  /* before null term */
	if (*pch == '.')
		{
		if (pch == sz || (pch == sz + 1 && *sz == '.'))
			return (sz + cch);
		pch--;
		if (*pch == '.')
			pch--;
		if (*pch == chColon || *pch == chBackSlash)
			return (sz + cch);
		}

	return (NULL);
}


/* return location of extension period, or NULL if none */
/* %%Function:PchFnHasExt  %%Owner:peterj */
PchFnHasExt(sz)
char * sz;
{
	char *pch;

	if ((pch = index(sz, '.')) == NULL)
		return (NULL);
	else
		{
		/* must be at least one period */
		/* period if for extension if: last period in name and
				char before is not . or \ or :. so these fail:
				c:\..\foo
				..
				\foo\.
		
				but these work:
				c:\..\foo.ext
				c:\..\foo.
				foo.ext
				..\foo.
			*/
		pch = sz + CchSz(sz) - 2;  /* before null term */
		while (*pch != '.' && pch > sz)
			pch--;  /* get last period */
		if (pch > sz)
			{
			pch--;
			if (*pch != '.' && *pch != chBackSlash && *pch != chColon)
				return (++pch);
			}
		}

	return (NULL);
}



csconst CHAR mpifmtstFormat[][] =
{
	StKey("Normal Word format",NormalWordFormat),
			StKey("Word template format",WordTemplateFormat),
			StKey("plain text",PlainText),
};



/* I D  O V E R W R I T E  F O R E I G N */
/*  if we are writting over a file which has fForeignFormat set, ask if
	we really want to.  Only asks if stFile is file of this doc.
*/

/* %%Function:IdOverwriteForeign   %%Owner:peterj */
IdOverwriteForeign (doc, stFile, iFmt)
int doc;
CHAR *stFile;
int iFmt;

{
	CHAR szShort [ichMaxFile];
	CHAR szFormat [cchMaxSz];
	CHAR *rgpch[2];
	int fn = FnFromOnLineSt (stFile);

	if (fn == fnNil || 
			PdodDoc (doc)->fn != fn ||
			!PfcbFn(fn)->fForeignFormat || 
			!FUnderstoodDff(DffISaveFmt(iFmt), fFalse))
		return IDYES; /* go ahead and overwrite */

	SzShortFromStNormFile (szShort, stFile);
	CopyCsSt(mpifmtstFormat[iFmt], szFormat);
	StToSzInPlace(szFormat);
	rgpch[0] = szShort;
	rgpch[1] = szFormat;
	return IdMessageBoxMstRgwMb (mstOverwriteForeign, rgpch,
			MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION);
}





/* F  C H E C K  S A V E  C A B */
/*  Checks legality of responses in pcabsave.  Fills out stNorm with
	the normalized file name.
	If fReportError will report an error before returning false.
*/

/* %%Function:FCheckSaveCab  %%Owner:peterj */
FCheckSaveCab(hcab, pcesa, stNorm)
HCABSAVE hcab;
CESA *pcesa;
char * stNorm;
{
	int doc = pcesa->doc;
	int dff = DffISaveFmt((*hcab)->iFmt);
	int fn;
	int docName;
	int eid;
	BOOL fDocType;
	struct DOD *pdod = PdodDoc(doc);
	struct FCB * pfcb;
	char stUser [ichMaxFile];

	FreezeHp();/* for pdod */

	/* eidNil fRepeat? */
	eid = eidNil;
	fDocType = (dff == dffSaveDocType);

	if ((vhsttbCvt == hNil && dff >= 0) || (dff >= (*vhsttbCvt)->ibstMac))
		{
		Assert(fElActive);
		RtError(rerrOutOfRange);
		}

	Assert(doc != docNil && doc == DocMother(doc));

	GetCabSt(hcab, stUser, ichMaxFile, Iag(CABSAVE, hszFile));

	/*  validate filename */
	if (!*stUser || !FNormalizeStFile(stUser, stNorm,
			nfoPathCk | (fDocType ? nfoDot : nfoNormal)))
		{
		eid = eidBadFilename;
		goto LErrorRet;
		}

	/*  do we have the requested file open? */
	fn = FnFromOnLineSt(stNorm);

	if ((docName = DocFromSt(stNorm)) != docNil && docName != doc)
		/*  cannot save to a name in use by another document */
		{
		eid = eidFileExists;
		goto LErrorRet;
		}

	if (!pcesa->fQuicksaveOK || fn == fnNil 
			|| !FUnderstoodDff(dff, fTrue /*fBinary*/)
			|| fn != pdod->fn)
		/*  can only quicksave in binary and to the mother fn of the doc */
		{
		(*hcab)->fQuicksave = fFalse;
		}

	if (pdod->fDot && !fDocType)
		/* cannot save DOT to non-dot format */
		{
		eid = eidCantWriteFrgn;
		goto LErrorRet;
		}

	if (fn != fnNil)
		{
		pfcb = PfcbFn(fn);

		if (fn == pdod->fn && !FUnderstoodDff(dff, pdod->fFormatted)
				&& !pfcb->fForeignFormat)
			/* cannot save to understood mother fn in foreign format */
			{
			eid = eidCantWriteFrgn;
			goto LErrorRet;
			}

		/*  cannot save over a file that is read only. */
		if (pfcb->fReadOnly)
			{
			eid = eidFileIsReadOnly;
			goto LErrorRet;
			}
		}

	MeltHp();
	return fTrue;

LErrorRet:
	MeltHp();

	ErrorEid(eid, "FCheckSaveCab");

	return fFalse;
}



/* F  U N D E R S T O O D  D F F */
/*  Return true if dff is a file format that we can display.  If
	fBinaryOnly then only those that are our binary format.
*/

/* %%Function:FUnderstoodDff   %%Owner:peterj */
FUnderstoodDff (dff, fBinaryOnly)
int dff;
BOOL fBinaryOnly;

{
	switch (dff)
		{
	case dffSaveNative:
	case dffSaveDocType:
		return fTrue;
	case dffSaveText:
		return !fBinaryOnly;
		}
	return fFalse;
}


int vcRescue = 0;

/* D O  E M E R G  S A V E */
/* %%Function:DoEmergSave  %%Owner:peterj */
DoEmergSave(doc)
{
	int matSave;
	CHAR stFile[30];
	WORD rgw[2];

	doc = DocMother(doc);
	vmerr.mat = matNil;
	vmerr.fHadMemAlert = fFalse;

	Assert(!fElActive);

	/* figure out file name to suggest (RESCUEn.DOC) */
	BuildStMstRgw(mstRescueDoc, &vcRescue, stFile, 30, NULL);
	if (++vcRescue >= 100)
		vcRescue = 0;

	rgw[0] = doc;
	rgw[1] = stFile;

	if (IdMessageBoxMstRgwMb(mstSaveRescue, rgw, 
			MB_ICONHAND|MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
		/* Memory kludge: To help assure that we can actually save this file under
			low memory conditions, free the bogus heap block.*/
		FreePh(&vmerr.hrgwEmerg3);

		FFlushDoc(doc, stFile, PdodDoc(doc)->fDot ?
				dffSaveDocType : dffSaveNative, fTrue /* fReport */);

		UpdateTitles();

		matSave = vmerr.mat;
		/* reclaim emergency heap allocation for Save */
		if ((vmerr.hrgwEmerg3 = HAllocateCw(cwEmerg3)) == hNil &&
				mpdochdod[doc] != hNil)
			{
			ErrorEid(eidClosingSavedDoc, "DoEmergSave");
			SetErrorMat(matLow);
			if (vsab.docStsh == doc)
				{
				struct CA caT;
				SetWholeDoc(docScrap, PcaSetNil(&caT));
				vsab.doc = docNil;
				}
			CloseMwsOfDoc(doc);
			}
		else
			vmerr.mat = matSave;
		}
}


/* F F L U S H  D O C */
/*  Save doc.
*/
/* %%Function:FFlushDoc  %%Owner:peterj */
FFlushDoc(doc, stName, dff, fReport)
int  doc;
CHAR *stName;
int dff;
BOOL fReport;
{
	BOOL f = fFalse;
	CHAR stFile [ichMaxFile];
	CHAR *pst = stName == NULL ? stFile : stName;
	CHAR stNorm [ichMaxFile];
	BOOL fQuicksave;
	BOOL fBackup;

	/* avoid stack overflow */
	ReturnOnNoStack(6250,fFalse,fTrue);

	if (stName == NULL)
		GetDocSt (doc, stFile, gdsoFullPath|gdsoNoUntitled);
	Assert (*pst);

	AssertDo(FNormalizeStFile(pst, stNorm, nfoNormal));

	if (PdodDoc(doc)->fDot && dff == dffSaveNative)
		dff = dffSaveDocType;

	f = FSaveFile(stNorm, doc, dff, fFalse, fFalse, fReport);
	vhpprSave = hNil;
	if (fReport)
		StopProgressReport(hNil, pdcRestoreImmed);

	return f;
}





/* F S A V E  F I L E */
/*  Perform actual save according to pcabsave.
*/

/* %%Function:FSaveFile  %%Owner:peterj */
BOOL FSaveFile(stFile, doc, dff, fQuicksave, fBackup, fReport)
char * stFile;
int doc, dff;
BOOL fQuicksave, fBackup, fReport;
{
	extern CHAR rgchEop[];

	int  eid = eidNil;
	struct DOD **hdod = mpdochdod [doc];
	BOOL fRegenerateDoc = FUnderstoodDff (dff, (*hdod)->fFormatted);
	BOOL fQuickSaveFailed = fFalse;
	BOOL fSaveSuccessful = fFalse;
	int  fn;
	BOOL fConvertToDoc = fFalse;
	int  docDot = docNil;
	struct PLC **hplcpad;
	struct DSR dsr;
	struct DSR dsrGlsy;
	struct BKINFO bkinfo;
	extern BOOL vfInsertMode;

	Assert(!vfInsertMode); /* bad news if we save while in insert mode */

	Assert (!(*hdod)->fShort);
	Assert (!(*hdod)->fGlsy);
	Assert (doc == DocMother (doc));

	StartLongOp();

	Debug(vdbs.fCkDoc ? CkDoc(doc) : 0);

	/* Clean up undo document */
	SetUndoNil();

	
	/* set so vhprc chain is checked when we run out of memory */
	vmerr.fReclaimHprcs = fTrue;

	/*  if converting a doc to a dot, do so here */
	if (dff == dffSaveDocType && !(*hdod)->fDot)
		{
		/*  if the file had a doctype, we must remember it */
		docDot = (*hdod)->docDot;
		if (docDot != docNil)
			{
			PdodDoc (docDot)->crefLock++; /* don't want it to be deleted */
			}
		Assert ((*hdod)->fDoc);
		if (!FConvertDoc (doc, dkDot))
			{
			if (docDot != docNil)
				PdodDoc (docDot)->crefLock--;
			goto LQuit;
			}
		fConvertToDoc = fTrue;
		}

	/*  once a doc is converted to a DOT, there is no going back! */
	Assert (!(*hdod)->fDot || dff == dffSaveDocType);

	SetBytes (&dsr, 0, cbDSR);
	dsr.doc = doc;
	EnablePreload(PdodDoc(doc)->fn);
	SetBytes (&dsrGlsy, 0, cbDSR);
	dsrGlsy.doc = (dff == dffSaveDocType) ? (*hdod)->docGlsy : docNil;

	if (fRegenerateDoc)
		{
		/*  guarantees that all fields in doc have been parsed */
		AssureAllFieldsParsed (doc);
		if (dsrGlsy.doc != docNil)
			AssureAllFieldsParsed (dsrGlsy.doc);
		}

	/*  perform Quicksave, if requested */
	if (fQuicksave && dsrGlsy.doc == docNil)
		{
		int matSave = vmerr.mat;
		if (fReport)
			StartSaveReport (mstFastSave, stFile, doc);
		fn = (*hdod)->fn;
		fSaveSuccessful = FQuickSave(fn, &dsr, fFalse, NULL) && 
				FFlushFile(fn);
		RemoveSubdocsDsrs(&dsr, &dsrGlsy);
		if (fSaveSuccessful)
			{
			UndirtyDoc(doc);
			goto LQuitOK;
			}

		if (vmerr.fDiskAlert || vmerr.fDiskWriteErr)
			goto LQuit;

		vmerr.mat = matSave; /* failure of quicksave shouldn't affect full save */
		Debug(vhpprSave = hNil); /* to avoid assert in StartSaveReport */
		if (fReport)
			StartSaveReport (mstFullSave, stFile, doc);
		}
	else if (fReport)
		StartSaveReport (mstSaving, stFile, doc);

	/* Memory kludge: To help assure that we can actually save this file under
		low memory conditions, free the bogus heap block.*/
#ifdef DEBUG
	if (!vdbs.fFixedMem)
#endif /* DEBUG */
		FreePh(&vmerr.hrgwEmerg2);

	StartGuaranteedHeap();

	(*hdod)->dop.fBackup = fBackup;

	if (!FBackupStFile(stFile, fBackup, &bkinfo))
		/* We were unable to back up the file.
				An appropriate error message was reported. */
		{
		EndGuarantee();
		ReportSz("Could not backup target file; abandoning save");
		goto LQuit;
		}

	/* Assert that this file does NOT yet exist on the dest. drive */
	/* FBackupStFile should have assured this condition */
	Assert( !FExistsStFile( stFile, FALSE ) );

	/* Create the file that we're going to save to */

	/* because of emerg block */
	
	fn = FnOpenSt(stFile, (FUnderstoodDff (dff, fTrue/*fBinary*/) ?
			fOstFormatted : 0) | fOstCreate | fOstNamed, ofcDoc, NULL );
	EndGuarantee();

	/*  FnOpenSt fail? */
	if (FRareT(reNoSaveTemp, fn == fnNil))
		{
		eid = eidNoSaveTemp;
		goto LQuit;
		}

	/* write out file.  If writing fails, restore the pre-save
		environment by renaming the backup file to the original file
		name and return. (error was already reported & file deleted) */

	if (!FWriteFnDsrs (fn, &dsr, &dsrGlsy, dff))
		{
		ReportSz("FWriteFn Failed");
		if (bkinfo.fn != fnNil)
			{ /* Must rename by fn on Windows, name
									may not be unambiguous */
			if (FRenameFn( bkinfo.fn, stFile ))
				(PfcbFn(bkinfo.fn))->fTemp = fFalse;
#ifdef DEBUG
			else  /* SOL, buddy! */
				ReportSz("Could not rename backup file to original file name");
#endif  /* DEBUG */
			}
		goto LQuit;         /* Disk full or write error */
		}       /* end if save unsuccessful */

	if (fRegenerateDoc)
		{
		PN pnFib = pn0;

		/* touch the MRU list in the file menu, if doc is displayed */
		if (WwDisp(doc,wwNil,fFalse) != wwNil && fReport)
			AddStFileToMru(stFile);

		/* link doc to the new file with its new format */
		if (!FRegenerateDoc (fn, &dsr, &pnFib) ||
				!FRegenerateDoc (fn, &dsrGlsy, &pnFib))
			goto LCleanupQuit;
		}
	else 
		
		{
		if (bkinfo.fn != fnNil && PdodDoc(doc)->fn == bkinfo.fn)
			/* don't change filename to the backup name */
			PdodDoc(doc)->fn = fn;
		PfcbFn(fn)->fForeignFormat = fTrue;
		}

LQuitOK:
	/* At this point, file save was successful and docs should be ok */
	Debug(vdbs.fCkFn ? CkFn(fn) : 0);
	Debug(vdbs.fCkDoc ? CkDoc(doc) : 0);
	Debug(vdbs.fCkDoc && dsrGlsy.doc != docNil ? CkDoc(dsrGlsy.doc) : 0);

	/* if nothing of value in pad, remove it */
	if (vfDeletePad)
		FreePhplc(&(*hdod)->hplcpad);

	/*  we held onto the dot in case of failure, dispose of it now, unless
		it is dirty. (If it is dirty it will hang around until the user
		closes opus). */
	if (docDot != docNil && !DiDirtyDoc (docDot))
		{
		PdodDoc(docDot)->crefLock--;
		DisposeDoc (docDot);
		}

	fSaveSuccessful = fTrue;

LQuit:
	/* set so vhprc chain is checked when we run out of memory */
	vmerr.fReclaimHprcs = fTrue;

	if (!fSaveSuccessful)
		{
		int fnT;
		if (fConvertToDoc)
			{  /* we failed and we had already turned doc into a dot */
			/*  change it back into a doc */
			AssertDo(FConvertDoc (doc, dkDoc)); /* dot --> doc cannot fail! */
			/*  it had a dot, reestablish the link */
			if (docDot != docNil)
				{
				CHAR stDot [ichMaxFile];
				(*hdod)->docDot = docDot; /* crefLock was already incremented */
				GetDocSt(docDot, stDot, gdsoFullPath);
				/* if this fails then the DOT will be lost */
				FChangeStInSttb ((*hdod)->hsttbAssoc, ibstAssocDot, stDot);
				}
			}
		if ((fnT = PdodDoc(doc)->fn) != fnNil)
			PfcbFn(fnT)->fTemp = fFalse;

		if (fRegenerateDoc && fReport)
			(*hdod)->dop.nRevision--; /* incremented in ApplyDocMgmtSave */
		}

	/* this will collect any files on disk volumes currently
		mounted that were made non-referenced through save or
		writing docScrap. */
	KillExtraFns();

		/* reclaim emergency heap allocation for Save */
		{
		int matSave;
		matSave = vmerr.mat;
#ifdef DEBUG
		if (!vdbs.fFixedMem)
#endif /* DEBUG */
			vmerr.hrgwEmerg2 = HAllocateCw(cwEmerg2);
		if (vmerr.hrgwEmerg1 == hNil)
			vmerr.hrgwEmerg1 = HAllocateCw(cwEmerg1);
		vmerr.mat = matSave;
		}

	vdocFetch = docNil;     /* kill fetch optimizations */
	/* reset the picture info in selCur for graphics selections */
	/* i'm not sure why this is being done, but I think it will be ok
		even if the sel is an import field graphic selection. bz
	*/
	if (selCur.fGraphics)
		{
		Assert(selCur.doc != docNil);
		GetSelCurChp(fTrue);
		InvalFli();
		}
	if ((*hdod)->hplcpad == hNil)
		{
		int ww;
		for (ww = WwDisp(doc, wwNil,fFalse); ww != wwNil; 
				ww = WwDisp(doc, ww,fFalse))
			if (PwwdWw(ww)->fOutline)
				{
				FUpdateHplcpad(doc);
				break;
				}
		}

LCleanupQuit:
	InvalCaFierce();
	InvalFli();
	EndLongOp (fFalse /* fAll */);
	if (eid != eidNil)
		{
		ErrorEid(eid, "");
		}
	/* update lock flag for doc and all subdocs */
	if (fSaveSuccessful)
		SetDocLockFlag(doc);
	DisablePreload();

	return fSaveSuccessful;
}



/* R E M O V E  S U B D O C S  D S R S */
/*  Used when exiting save through other than usual path.  Removes the
	subdocs added at end of the docs.  Does both dsr and dsrGlsy.
*/

/* %%Function:RemoveSubdocsDsrs   %%Owner:peterj */
RemoveSubdocsDsrs (pdsr, pdsrGlsy)
struct DSR *pdsr, *pdsrGlsy;

{
	CP dcp;
	struct CA caT;

	if ((dcp = CpMacDoc(pdsr->doc) - pdsr->ccpText) != 0)
		FDelete(PcaSetDcp(&caT, pdsr->doc, pdsr->ccpText - ccpEop, dcp));

	if (pdsrGlsy->doc != docNil &&
			(dcp = CpMacDoc(pdsrGlsy->doc) - pdsrGlsy->ccpText) != 0)
		FDelete(PcaSetDcp(&caT, pdsrGlsy->doc, pdsrGlsy->ccpText - ccpEop, dcp));
}






/* F  R E G E N E R A T E  D O C */
/*  Makes a doc refer to the newly saved fn.
	Pull headers/footers out of mother doc if they had been merged.
*/

/* %%Function:FRegenerateDoc   %%Owner:peterj */
FRegenerateDoc (fn, pdsr, ppnFib)
int fn;
struct DSR *pdsr;
PN *ppnFib;

{
	int        doc = pdsr->doc;
	struct DOD **hdod = mpdochdod [doc];
	struct WWD *pwwd;
	int        ww, wwSave;
	PN	       pnFib = *ppnFib;
	int 	   fTrashWws = fFalse;
	CP         cpMacDoc;
	struct DRF drfFetch;
	struct PLC **hplcedl;
	struct PLC **hplcpad;
	struct DOD dod;
	struct CA  caT;
	struct FIB fib;

	if (doc == docNil)
		return fTrue;

	Assert (hdod != hNil);

	/* cleanup the piece table. */

	UndirtyDoc(doc);
	blt(*hdod, &dod, cwDOD);

	StartGuaranteedHeap();
	FreeHplc(dod.hplcpcd); /* Free old piece table */
	if (!FInitPlcpcd(&dod, fn, ppnFib))  /* modifies *ppnFib */
		{
		EndGuarantee();
		goto LThrowAwayDoc;
		}
	EndGuarantee();

	dod.fn = fn;
	dod.udt = 0;
	dod.iInstance = 0;
	blt(&dod, *hdod, cwDOD);

	if (dod.fFormatted)
		{
		FetchFib(fn, &fib, pn0);
		if (IMacPlc(PfcbFn(fn)->hplcbteChp) < fib.cpnBteChp)
			{
			if (!FFillMissingBtePns(fn, PfcbFn(fn)->hplcbteChp, &fib, 0, fTrue/* CHP */))
				goto LThrowAwayDoc;
			}
		if (IMacPlc(PfcbFn(fn)->hplcbtePap) < fib.cpnBtePap)
			{
			if (!FFillMissingBtePns(fn, PfcbFn(fn)->hplcbtePap, &fib, 0, fFalse/* PAP */))
				{
LThrowAwayDoc:
				ErrorEid(eidClosingSavedDoc, "FRegenerateDoc");
				SetErrorMat(matLow);
				if (vsab.docStsh == doc)
					{
					struct CA caT;
					SetWholeDoc(docScrap, PcaSetNil(&caT));
					vsab.doc = docNil;
					}
				CloseMwsOfDoc(doc);
				if (mpdochdod[doc] != hNil)
					/* this is a severe problem!! */
					{
					/* Disable Auto macros so we don't get in more trouble */
					extern BOOL vfDisableAutoMacros;
					vfDisableAutoMacros = fTrue;
					CommitSuicide(PdodDoc(doc)->fn, NULL);
					Assert(fFalse); /* never returns */
					}
				return fFalse;
				}
			}

		/* make sure nothing fails */
		vmerr.mat = matNil;

		DisableInval();

		hplcpad = (*hdod)->hplcpad;
		(*hdod)->hplcpad = 0;
		Assert (dod.hplcsed != hNil);
		RetrieveHplcsed (fn, doc, pnFib);
		CorrectDodPlcs(doc, CpMac2Doc(doc));
		/*  pull headers/footers and footnotes out of mother document */
		if (!FRebuildSubdocPlcpcd(pdsr, ddsrFtn, edcDrpFtn) ||
				!FRebuildSubdocPlcpcd(pdsr, ddsrHdr, edcDrpHdr))
			{
LThrowAwayDoc2:
			EnableInval();
			goto LThrowAwayDoc;
			}
		if (dod.fDot && !FRebuildSubdocPlcpcd(pdsr, ddsrMcr, edcDrpMcr))
			goto LThrowAwayDoc2;
		if (!FRebuildSubdocPlcpcd(pdsr, ddsrAtn, edcDrpAtn))
			goto LThrowAwayDoc2;
		if (dod.docFtn || dod.docHdr || dod.docAtn || dod.docMcr)
			{
			CP cpExtraEop = pdsr->ccpText - ccpEop;
			/* before deleteing extra EOP, make sure no one is pointing to it */
			AdjustHplc(dod.hplcbkf, cpExtraEop, ccpEop, -1);
			AdjustHplc(dod.hplcbkl, cpExtraEop, ccpEop, -1);
			/* remove the extra chEop at end of main doc */
			FDelete(PcaSetDcp(&caT, doc, cpExtraEop, ccpEop));
			CorrectDodPlcs(doc, CpMac2Doc(doc));
			cpMacDoc = CpMac2Doc(doc) - ccpEop;
			/* wait to trash windows until hidden section mark has been
				reestablished */
			fTrashWws = fTrue;
			}
		EnableInval();
		if (!FInsertHiddenSectionMark(doc) || vmerr.fMemFail)
			goto LThrowAwayDoc;
		if (fTrashWws)
			{
			for ( ww = wwNil; (ww = WwDisp(doc,ww,fTrue)) != wwNil; )
				{
				struct PLDR **hpldr;
				int idr;
				CP cpFirst;
				if (!PwwdWw(ww)->fPageView)
					{
					hplcedl = PdrGalley(PwwdWw(ww))->hplcedl;
					if (CpPlc(hplcedl, 0) < cpMacDoc &&
							cpMacDoc <= CpPlc(hplcedl, IMacPlc(hplcedl)))
						TrashWw(ww);
					}
				else  if ((idr = IdrFromHpldrDocCp(HwwdWw(ww), ww, doc,
						cpMacDoc, fFalse, fFalse)) != idrNil)
					TrashWw(ww);
				}
			}

		(*hdod)->hplcpad = hplcpad;
		}
	else
		/* new doc is plain, should not have been previously formated */
		{
		CP cpMac;
		CP cpT;

		/* !fFormatted ==> doc should not have any of this */
		Assert (dod.fDoc);
		Assert ((*hdod)->docFtn == docNil);
		Assert ((*hdod)->docHdr == docNil);
		Assert ((*hdod)->docAtn == docNil);
		Assert ((*hdod)->hplchdd == hNil);
		Assert ((*hdod)->hplcfrd == hNil);
		Assert ((*hdod)->hplcatrd == hNil);
		Assert ((*hdod)->hplcbkf == hNil);
		Assert ((*hdod)->hplcbkl == hNil);
		Assert ((*hdod)->hsttbBkmk == hNil);
		Assert ((*hdod)->hplcfld == hNil);

		/* all docs have a sed, keep it that way! */
		Assert ((*hdod)->hplcsed != hNil);

		/* Free followed by smaller or equal allocate: guaranteed */
		StartGuaranteedHeap();
		FreePh(&(*hdod)->hplcsed);
		HplcCreateEdc(hdod, edcSed);
		EndGuarantee();

		/* ensure that all ww's for the new text document are NOT in
			outline mode or in page view (since we deleted the structures
			above) */
		wwSave = wwCur;
		for ( ww = wwNil; (ww = WwDisp(doc,ww,fFalse)) != wwNil; )
			{
			pwwd = PwwdWw(ww);
			if (pwwd->fOutline)
				{
				/* For various reasons, we do not restore
				   rulers in this case. */
				pwwd->fHadRuler = fFalse;
				FToggleOutline(ww);
				}
			else  if (pwwd->fPageView)
				{
				if (ww != wwCur)
					NewCurWw(ww, fFalse);
				CmdPageView(NULL);
				}
			/* Protect against a possible heap movement. */
			pwwd = PwwdWw(ww);
			}
		if (wwSave != wwCur)
			NewCurWw(wwSave, fFalse);

		/*  doc may have these because of pagination */
		FreePhplc(&(*hdod)->hplcpgd);
		FreePhplc(&(*hdod)->hplcpad);

		}

	return fTrue;

}




/* F  R E B U I L D  S U B D O C  P L C P C D */

/* %%Function:FRebuildSubdocPlcpcd  %%Owner:peterj */
FRebuildSubdocPlcpcd(pdsr, ddsr, edcDrp)
struct DSR *pdsr;
int ddsr;
int edcDrp;
{
/* we pull some tricks here because the normal editing situation doesn't cover
	what we're trying to do:
		1. We don't disturb the old chEop at the end of the subdoc until after
			we have replaced the old pieces with the new one; then we delete it
			surgically (we know it has either a different fn or noncontiguous
			fc from the new stuff -- so just subtract ccpEop from cpMac and
			remove the last pcd entry)
		2. The plcfnd/plchdd needn't take part in the editing, since the CP
			bounds will be the same after as they were before, and in fact
			AdjustCp doesn't know how to handle editing the subdoc data -- so
			we hide the plc from the editing
*/
	int doc = pdsr->doc;
	int docSubdoc;
	int ipcd;
	CP ccpSubdoc;

	struct DOD *pdod;
	struct PLC **hplcfnd;
	struct PLC **hplcfld;
	struct DRP *pdrp;
	struct CA ca1, ca2;
	struct PLC **hplcpcd, **hplcpcdSub;
	CP *pccpSubdoc;
	struct PCD pcd;

	/*  momentarily break link to subdoc doc in dod so that ReplaceCps
		doesn't get confused. */
	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	docSubdoc = pdrp->doc;
	pccpSubdoc = ((int *)pdsr) + ddsr;
	ccpSubdoc = *pccpSubdoc;
	pdrp->doc = docNil;

	if (docSubdoc != docNil && ccpSubdoc != cp0)
		{
		/* hide the subdoc plcfnd */
		pdod = PdodDoc(docSubdoc);
		hplcfnd = pdod->hplcfnd;
		hplcfld = pdod->hplcfld;
		pdod->hplcfnd = hNil;
		pdod->hplcfld = hNil;

		/* replace original piece table for subdoc with new --
			note that we leave the chEop at end temporarily */
		ccpSubdoc -= ccpEop;
		if (!FReplaceCps(PcaSet(&ca1, docSubdoc, cp0, ccpSubdoc),
				PcaSet(&ca2, doc, pdsr->ccpText, pdsr->ccpText + ccpSubdoc)))
			return fFalse; 

		/* now replace the PCD for the chEop left over from olden times;
		   unfortunately, the editing routines can't do this for us */
		hplcpcd = PdodDoc(doc)->hplcpcd;
		if ((ipcd = IpcdSplit(hplcpcd, pdsr->ccpText + ccpSubdoc)) == iNil)
			return fFalse;
		GetPlc(hplcpcd, ipcd, &pcd);
		hplcpcdSub = PdodDoc(docSubdoc)->hplcpcd;
		PutPlc(hplcpcdSub, IMacPlc(hplcpcdSub) - 1, &pcd);
		if ((ipcd = IpcdSplit(hplcpcdSub, ccpSubdoc+ccpEop)) == iNil)
			return fFalse;
		PutPlc(hplcpcdSub, ipcd-1, &pcd);

		/* eliminate the footnote/header text from the main doc */
		ca2.cpLim += ccpEop;
		if (!FDelete(&ca2))
			return fFalse;
		pdod = PdodDoc(docSubdoc);

		/* restore plcfnd */
		pdod->hplcfnd = hplcfnd;
		pdod->hplcfld = hplcfld;
		pdod->fDirty = fFalse;

		TruncateAllSels(docSubdoc, pdod->cpMac);
		}

	/* restore docFtn/docHdr */
	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	pdrp->doc = docSubdoc;

	return fTrue;
}




/* F   B A C K U P    S T    F I L E */
/* %%Function:FBackupStFile  %%Owner:peterj */
FBackupStFile(stFile, fBackupOption, pbkinfo )
CHAR stFile[];
int  fBackupOption;
struct BKINFO *pbkinfo;
	{ /*
	DESCRIPTION:
	If a file named stFile exists on the current disk, rename it to a backup name.
	The mapping from fn --> physical file sectors is preserved.
	If fBackupOption == fTrue (user selected the backup option in the save dialog),
	give the backup a ".BAK" name. Otherwise, give the backup a temporary name.
	stFile is passed in normalized, stBak is returned normalized.
	MODIFIES:
	*pbkinfo contains the following:
		fn       fn of the backup copy of the file, or fnNil if no backup was made
		stFile   the name of the backup file created (if fn != fnNil)
	RETURNS:
		TRUE if a backup was made OK or if none was necessary
		FALSE if a serious error occurred and the Save should not proceed
				(i.e. we were unable to clear away the existing stFile)
*/

	int fnOld;
	int fTemp = fFalse;
	CHAR *stBak = pbkinfo->stFile;
	CHAR chDrive;
	CHAR stBakExt[ichMaxExtension];
	int fFirstTry = fTrue;
	struct FNS fns;
	char *pch;

	pbkinfo->fn = fnNil;

	/* if stFile doesn't exist anywhere, there ain't nothin' to backup. */
	/* We assume that the user had not switched disks since the
			"save destination" file was created */

	if (!FExistsStFile(stFile, fTrue))
		return fTrue;

	/* if file exists on disk but word has no FCB for it
				if user wants a backup of the file
					create an fn for the file so we can request it from
					OpenFile even if the user swaps disks
				else
					delete the file and return, having done no backup
		*/

	if ((fnOld = FnFromOnlineSt( stFile )) == fnNil)
		{
		if (fBackupOption)
			{
			if (FRareT( reNoAccessBak,
					(fnOld = FnOpenSt(stFile,
					fOstFormatted, ofcDoc, NULL))
					== fnNil))
				{
				goto LLock; /* ERROR: Abort the save */
				}
			}
		else
			{
			if (EcOsDelete(stFile, 0) < 0)
				{
				goto LLock; /* ERROR: Abort the save */
				}
			else
				return fTrue;
			}

		}

	/* At this point we know: the file stFile exists and is represented
			by fn == fnOld. */

	Assert( fnOld != fnNil );
	pbkinfo->fn = fnOld;

	if (PfcbFn(fnOld)->fReadOnly)
		{
		ErrorEidW(eidReadonly, PfcbFn(fnOld)->stFile, "FBackupStFile");
		return fFalse;
		}

	StFileToPfns( stFile, &fns );

	/* if the user did not select the backup option OR the file we're
			backing up happens to be a .BAK already
					use a temporary (~DOCXXXX.TMP) filename
			else
					use <NAME>.BAK
	
			If the temp file creation or rename fails using the default
			drive/directory for temp file creation, try it once again
			using the same drive as the original file.
	
			Motivation for this two-pass approach is the desire to leave temp
			files in the standard location whenever possible
		*/

	SzToSt(szBak, stBakExt);

	if (!fBackupOption ||
			FStSame( fns.stExtension, stBakExt ))
		{
		CHAR stT [ichMaxFile];

		fTemp = fTrue;
		chDrive = 0;
LTryChDrive:
		if (!GetTempFileName( chDrive, (LPSTR) szDoc+1, 0,
				(LPSTR) stT ))
			{
			if (fFirstTry)
				{
LRetry:
				chDrive = stFile [1] | TF_FORCEDRIVE;
				fFirstTry = fFalse;
				goto LTryChDrive;
				}
			else
				goto LTempFail; /* ERROR: Abort Save */
			}

		SzToStInPlace( stT );
		if (!FNormalizeStFile(stT, stBak, nfoNormal) ||
				EcOsDelete(stBak, 0) < 0)
			/* this can happen, especially with ill-formed TEMP var */
			goto LTempFail;   /* ERROR: Abort Save */

		/* this darn well better be true */
		Assert( !FHasFcbFromSt( stBak ) );
		}
	else
		{   /* User selected backup option -- make a ".BAK" file */
		CopySt( stBakExt, fns.stExtension );
		PfnsToStFile( &fns, stBak );

		/* if a file with the backup name exists on the current disk,
						if we have an fn for it
								Make a backup copy of it to a temp filename,
								since someone may be holding pieces of it
						else
								delete it, since it is just taking up space,
								given that we do not support 2 levels of backup
				*/

		if (FExistsStFile( stBak, fFalse ))
			{
			if (FHasFcbFromSt( stBak ))
				{
				struct BKINFO bkinfoT;
				/* use a temporary name for backup */
				if (!FBackupStFile(stBak, fFalse, &bkinfoT))
					return fFalse;  /* ERROR: Abort Save */
				}
			else
				{
				if (EcOsDelete(stBak, 0) < 0)
					{
					stFile = stBak;
					goto LLock; /* ERROR: Abort Save */
					}
				}
			}
		}       /* end else name backup file "Backup of..." */

	/* Now we have stBak, the name of a file known not to exist on-line */

	if (!FRareF(reNoRenameToBak, FRenameFn( fnOld, stBak )))
		{
		if (fFirstTry)
			goto LRetry;    /* See comment about two-pass above */
		else
			{
			stFile = stBak;
			goto LLock;
			}
		}

	if (fTemp)
		{
		struct FCB *pfcbT = PfcbFn(fnOld);
		pfcbT->fTemp = fTrue;
		pfcbT->fDoc = fFalse;
		}

	/* Successfully backed up this file */
	return fTrue;

LLock:
	/* Backup failed due to locked or readonly file, stFile has name */
	ErrorEidW(FReadOnlySt(stFile) ? eidReadonly : eidLock, 
			stFile, "FBackupStFile");
	return fFalse;

LTempFail:
	/* Backup failed due to inability to create/rename the temp file */
	ErrorEid(eidNoSaveTemp, "FBackupStFile" );
	return fFalse;
}       /* end FBackupStFile */


/* %%Function:FReadOnlySt  %%Owner:peterj */
FReadOnlySt(st)
CHAR *st;
{
	CHAR sz[ichMaxFile];
	WORD da;
	StToSz( st, sz );
	AnsiToOem( sz, sz );

	return ((da = DaGetFileModeSz(sz)) != DA_NIL && (da & DA_READONLY));
}


/* F  R E N A M E  F N */
/* %%Function:FRenameFn  %%Owner:peterj */
FRenameFn( fn, stFileNew )
int fn;
CHAR stFileNew[];
	{   /* Rename the file represented by fn to have the name stFileNew.
	return TRUE if we succeeded, False if we failed */

	int ec;
	int cbNew; 
	CHAR szFileOld [ichMaxFile];

	Assert( fn != fnNil );

	/* Make sure that the disk with this file on it is in the drive */

	if (!FEnsureOnLineFn( fn ))
		return fFalse;

	/* cbFCB includes stFile[2], so subtract 2 but add 1 for st[0] */
	if ((cbNew = sizeof(struct FCB) + stFileNew[0] - 1) > CbOfH(mpfnhfcb[fn]) &&
		!FChngSizeHCb(mpfnhfcb[fn], cbNew, fTrue))
		{
		return (fFalse); 
		} 
	
	FCloseFn( fn );

	/* Call DOS to do the actual rename */

	StToSzInPlace( stFileNew );
	StToSz( (PfcbFn(fn))->stFile, szFileOld );
	ec = EcRenameSzFfname( szFileOld, stFileNew );
	SzToStInPlace( stFileNew );

	if (ec < 0)
		return fFalse;      /* rename failed */

	/* Rename succeeded -- change the name kept in the FCB structure */
	/* space for stFileNew was reserved above with call to FChngSizeHCb. 	
	   Size of FCB will never be shrunk by this routine */
	bltb(stFileNew, (PfcbFn(fn))->stFile, stFileNew[0] + 1);

	return fTrue;
}




/* F  W R I T E  F N  D S R S */
/* %%Function:FWriteFnDsrs   %%Owner:peterj */
int FWriteFnDsrs (fn, pdsr, pdsrGlsy, dff)
int fn;
struct DSR *pdsr, *pdsrGlsy;
int dff;
	{ /*
	DESCRIPTION:
	Write characters from a doc to fn.
	If writing file fails, cleans up after itself by deleting the writing file
	both from the disk and the file caches.
	RETURNS:
	fTrue if able to write file.  Otherwise, returns fFalse (ie.- disk full
	encountered during the save).  If fFalse returned then fn has been deleted.
*/

	switch (dff)
		{

	case dffSaveDocType:
	case dffSaveNative:
		if (pdsrGlsy->doc == docNil)
			{
			if (!FQuickSave(fn, pdsr, fTrue, NULL))
				goto LDeleteFile;
			}
		else
			{  /* must save poth pdsr and pdsrGlsy */
			struct MSR msr;

			if (!FQuickSave (fn, pdsr, fTrue, &msr))
				goto LDeleteFile;
			pdsrGlsy->fFkpdChpIncomplete = pdsr->fFkpdChpIncomplete;
			pdsrGlsy->fFkpdPapIncomplete = pdsr->fFkpdPapIncomplete;
			if (!FQuickSave (fn, pdsrGlsy, fTrue, &msr))
				goto LDeleteFile;
			}
		break;

	case dffSaveText8:
	case dffSaveText8CR:
	case dffSaveText:
	case dffSaveTextCR:
		WriteTextCommon( dff, fn, pdsr->doc );
		break;
	default:
			/* RTF or Foreign format - invoke RTF writer or converter */
#ifdef INTL
			{
			int doc = DocTranslate(pdsr->doc);

			if (doc == docNil)
				{
				DisposeDoc(doc);
				goto LDeleteFile;
				}

			if (dff == dffSaveRTF)
				WriteFnDsrsRTF(fn, doc);
			else  if (!FWriteFnDsrsForeign(dff, fn, doc))
				{
				DisposeDoc(doc);
				goto LDeleteFile;
				}
			DisposeDoc(doc);
			break;
			}
#endif /* INTL */

		if (dff == dffSaveRTF)
			WriteFnDsrsRTF(fn, pdsr->doc);
		else  if (!FWriteFnDsrsForeign(dff, fn, pdsr->doc))
			goto LDeleteFile;
		break;

		}

	/* if writing the file has failed due to disk full, clean up! */
	if (!FFlushFile(fn) || vmerr.fDiskAlert || vmerr.fDiskWriteErr)
		{
LDeleteFile:
		DeleteFn(fn,fTrue);
		return fFalse;
		}

	return fTrue;

}       /*  end FWriteFnDsrs */




/* A L I G N  P N */
/* advance cbMac to the next unallocated sector bound and return the pn */
/* %%Function:AlignPn  %%Owner:peterj */
AlignPn(fn)
int fn;
{
	struct FCB *pfcb = PfcbFn(fn);
	int pn = PnFromFc(pfcb->cbMac + (cbSector - 1));
	pfcb->cbMac = FcFromPn(pn);
}



/* F  F L U S H  F I L E  */
/* %%Function:FFlushFile  %%Owner:peterj */
FFlushFile(fn)
int fn;
{
	int fFileFlushed;

	fFileFlushed = 
			!(vmerr.fDiskAlert || vmerr.fDiskWriteErr) &&
			FFlushFn(fn) &&
			!(vmerr.fDiskAlert || vmerr.fDiskWriteErr);

	FCloseFn( fn );
	return(fFileFlushed && FAccessFn(fn,0));
}


/* A P P L Y  D O C  M G M T  S A V E */
/* set up terminal document management properties */

/* %%Function:ApplyDocMgmtSave   %%Owner:peterj */
ApplyDocMgmtSave (doc)   /* set up document management properties */
int doc;
{
	struct DOD *pdod;
	struct DOP *pdop;
	struct STTB **hsttb;
	struct DTTM dttm;
	long tm;

	struct DTTM DttmCur();

	FreezeHp();
	pdod = PdodDoc (doc);
	Assert (!pdod->fShort);
	pdop = &pdod->dop;

	Assert(pdop->nRevision > 0);

	/* update the editing minutes to include the time since we last
		opened or saved. Note that even if the save fails we can leave
		these results, since if we can't save the result is lost, if
		we can, the tmOpened has been adjusted so the result will
		be correct.
	*/
	dttm = DttmCur();
	tm = CMinutesFromDttms (pdod->dttmOpened, dttm);
	pdop->tmEdited += (pdop->fLockAtn && !FAuthor(doc) ? 0L : tm);
	pdod->dttmOpened = dttm;
	pdop->dttmRevised = dttm;

	/* we will decrement in case of failure */
	if (pdop->nRevision < 0x7fff)/* prevent overflow */
		pdop->nRevision++;  /* reflects version working on. */

	MeltHp();

	if ((hsttb = HsttbAssocEnsure(doc)) != hNil)     /* HM */
		{
		/* if either of these fail then info will not be correct, not
		a big deal. */
		FChangeStInSttb( hsttb, ibstAssocLastRevBy, vpref.stUsrName );
		/* if no author make current user author */
		if (!*HpstFromSttb (hsttb, ibstAssocAuthor))
			FChangeStInSttb(hsttb, ibstAssocAuthor, vpref.stUsrName);
		}
}


/* S T A R T  S A V E  R E P O R T */
/* %%Function:StartSaveReport  %%Owner:peterj */
StartSaveReport(mst, stFile, doc)
MST mst;
CHAR *stFile;
int doc;

{
	int nIncr = NIncrFromL(CpMacDoc(doc));
	Assert(vhpprSave == hNil);
	if ((vhpprSave = HpprStartProgressReport (mst, &stFile, nIncr, fFalse))
			== hNil)
		/* no biggie */
		vmerr.mat = matNil;
}


/* R E P O R T  S A V E  P E R C E N T */

/* %%Function:ReportSavePercent   %%Owner:peterj */
ReportSavePercent (cpMin, cpMac, cpNow, fQuicksave)
CP cpMin, cpMac, cpNow;
BOOL fQuicksave;

{

	if (vhpprSave == hNil)
		return;

	StartUMeas (umSavePrompt);

	if (fQuicksave)
		{{ /* NATIVE - ReportSavePercent */
		/* special considerations for templates with glossaries */
		if (vpqsib->fFirstOfTwo)
			cpMac = cpMac << 1;
		else  if (vpqsib->fWritingSecond)
			{
			cpNow += cpMac;
			cpMac = cpMac << 1;
			}
		}}

	AssertH(vhpprSave);
	ProgressReportPercent (vhpprSave, cpMin, cpMac, cpNow, NULL);

	StopUMeas (umSavePrompt);
}


/* I  R E F  F R O M  B C M */
/* Scan the command table counting fRef bits; return the count at bcm.
The piRefMinUser wierdness is an optimization that works with the following
assumtions:
		- Most bindings will be to user written macros which there will only
				be a few of.
		- In the worst case, there will probably not be more user written
				macros than built in commands.

*/
/* %%Function:IRefFromBcm  %%Owner:peterj */
IRefFromBcm(bcm, piRefMinUser)
BCM bcm;
int * piRefMinUser;
{
	int iRef;
	uns bsyI;
	BOOL fSetIRefMinUser;

	iRef = bsyI = 0;

	if (!(fSetIRefMinUser = *piRefMinUser < 0))
		{
		/* We've already calculated iRef for bcmMacStd... */
		if (bcm >= bcmMacStd)
			{
			iRef = *piRefMinUser;
			bsyI = bcmMacStd;
			}
		}

		{{ /* NATIVE - IRefFromBcm */
		while (bsyI < bcm)
			{
			HPSY hpsy;

			if (fSetIRefMinUser && bsyI == bcmMacStd)
				*piRefMinUser = iRef;

			hpsy = (HPSY) (vhpsyt->grpsy + bsyI);
			if (hpsy->fRef)
				{
				++iRef;
				}

			bsyI += CbHpsy(hpsy);
			}
		}}

	return iRef;
}


/* S e t  F R e f  O f  B s y */

/* %%Function:SetFRefOfBsy  %%Owner:peterj */
SetFRefOfBsy(bsy)
uns bsy;
{
	((HPSY)(vhpsyt->grpsy + bsy))->fRef = fTrue;
}


/* %%Function:SetDocLockFlag  %%Owner:peterj */
SetDocLockFlag(doc)
int doc;
{
	/* set fLockForEdit flag for doc and all its subdocs including docHdrDisp */
	int fLock = PdodDoc(doc)->dop.fLockAtn && !FAuthor(doc);
	int docSub;

	PdodDoc(doc)->fLockForEdit = fLock;
	if ((docSub = PdodDoc(doc)->docHdr))
		{
		PdodDoc(docSub)->fLockForEdit = fLock;
		while ((docSub = PdodDoc(docSub)->docHdr))
			PdodDoc(docSub)->fLockForEdit = fLock;
		}
	if ((docSub = PdodDoc(doc)->docFtn))
		PdodDoc(docSub)->fLockForEdit = fLock;
#ifdef DEBUG
	if ((docSub = PdodDoc(doc)->docAtn))
		Assert(!PdodDoc(docSub)->fLockForEdit);
#endif
}

/***********************************/
/* T r u n c a t e  A l l  S e l s */
/* %%Function:TruncateAllSels  %%Owner:peterj */
TruncateAllSels(doc, cpMac)
int doc;
CP cpMac;
{
	int ww, idr;
	struct WWD *pwwd;

	for (ww = WwDisp(doc, wwNil, fTrue); ww != wwNil; ww = WwDisp(doc, ww, fTrue))
		{
		pwwd = PwwdWw(ww);
		if ((idr = IdrFirstForDoc(ww, doc)) != idrNil)
			pwwd->rgdr[idr].cpFirst = CpMin(pwwd->rgdr[idr].cpFirst, cpMac);
		if (pwwd->sels.doc == doc)
			TruncateSels(&pwwd->sels, cpMac);
		}
	if (selCur.doc == doc)
		TruncateSels(&selCur, cpMac);
}


/**************************/
/* T r u n c a t e  S e l s */
/* %%Function:TruncateSels  %%Owner:peterj */
TruncateSels(psels, cpMac)
struct SELS *psels;
CP cpMac;
{
	if (psels->cpFirst >= cpMac)
		{
		psels->cpFirst = cpMac - ccpEop;
		psels->cpLim = cpMac - ccpEop;
		psels->fIns = fTrue;
		}
	else
		psels->cpLim = CpMin(psels->cpLim, cpMac);
}


extern BOOL             vfUrgentAlloc;
extern uns              vdfcTblBackup;


/* %%Function:FcBegOfExt  %%Owner:peterj */
FC FcBegOfExt(fnDest, doc, pfib)
int fnDest;
int doc;
struct FIB *pfib;
{
	FC fc;
	/* calculate beginning of extension (ie. first FC in destination
		file that may be overwritten.) Choices are at beginning of
		old style sheet (fib.fcStsh) or the begin of old footnote PLC
		(fib.fcPlcfnd). There are eight possibilities to handle :
	
		(style sheet dirty (D) | style sheet not dirty (ND)) X
		(document complex (C)  | document non-complex (NC))  X
		(Stsh and Plcfnd adjacent (A) | Stsh and Plcfnd non-adjacent (NA))
	
	The if statements below code the following decision table:
	
		ND   ND  ND  ND  D  D  D  D
		NC   NC  C   C   NC NC C  C
		A    NA  A   NA  A  NA A  NA
		--   --  --  --  -- -- -- --
		p    i   p   p   s  i  s  p
		l    m   l   l   t  m  t  l
		c    p   c   c   s  p  s  c
		f    o   f   f   h  o  h  f
		n    s   n   n      s     n
		d    .   d   d      .     d
	*/
	FetchFib(fnDest, pfib, pn0);
	if (!PdodDoc(doc)->fStshDirty || (pfib->fcStshf + pfib->cbStshf) != pfib->fcPlcffndRef)
		/* if the stsh is not dirty, no need to rewrite it,
			so start with the old plcfndRef position. Also if the
			end of the style sheet and the beginning of the old
			plcfndRef position are non-adjacent, there are CHP and/or
			PAP extensions that we don't wish to smash, so start
			fcDestLim at plcfndRef */
		fc = pfib->fcPlcffndRef;
	else
		/* else we have style sheet adjacent to plcfnd and the
			style sheet is dirty. start the extension at the old
			STSH fc. */
		fc = pfib->fcStshf;
	return (fc);
}


/* F  Q U I C K S A V E  O K */
/*  Rules for allowing fast save:
	- must have enough heap
	- must be a Word formatted file
	- file must not be read only
	- user must not have requested backup
	- para heights must not have been globally invalidated (!fLRDirty)
	- there cannot be too many unrecorded paragraph heights
	- piece table must not be "too large"
	- must be able to allocate huge memory to make a backup of the
		tables that would be overwritten by a fast save
(FOR OPUS)
	- either:
		o doc is fDoc, OR
		o	- doc is fDot, and
		- fn is not compound, and
		- doc does not have docGlsy
*/
/* returns fTrue when fast saving is permissible. Since one of the conditions
	for fast saving is that there be memory available to allocate a large
	enough hp block to hold a copy of the tables that would be written over
	during a fast save, this routine is responsible for allocating the
	vhqchTblBackup used by fQuickSave(). */
/* vcpheHeight should be set to 0, vfScanPad should be set to fTrue,
	and vfPadDelete should be set to fFalse
	when save command processing is initiated before FQuicksaveOk is called
	for the first time. */
/* %%Function:FQuicksaveOk  %%Owner:peterj */
FQuicksaveOk(doc)
int doc;
{
	struct DOD *pdod = PdodDoc(doc);
	int fn = pdod->fn;
	struct FCB *pfcb;
	int ipad, ipadMac;
	struct PAD pad;
	struct PLC **hplcpad;
	int ww;
	struct FIB fib;
	int matSave = vmerr.mat;

#ifdef MAC
	if (vpref.fNoFastSave)
		return fFalse;
#endif

	if (vhqchTblBackup != hqNil)
		{
		FreeHq(vhqchTblBackup);
		vhqchTblBackup = hqNil;
		}
	/* scan plcpad for possibility of deleting it if no outline info. also, force
	full save if there are a lot of para heights to be saved */
	if (vfScanPad && (hplcpad = pdod->hplcpad) != hNil)
		{
		ipadMac = IMacPlc(hplcpad);
		vfDeletePad = fTrue;
		for (ipad = 0; ipad < ipadMac; ipad++)
			{
			GetPlc(hplcpad, ipad, &pad);
			vfDeletePad &= pad.fShow;
			}
		vfScanPad = fFalse;
		}
	/* prevent hplcpad deletion if any ww is in outline mode */
	for (ww = WwDisp(doc, wwNil, fFalse); ww != wwNil; ww = WwDisp(doc, ww, fFalse))
		if (PwwdWw(ww)->fOutline)
			vfDeletePad = fFalse;

	if (fn != fnNil)
		{
		pfcb = PfcbFn(fn);
		if (!vmerr.fMemFail &&
				pfcb->fHasFib && !pfcb->fReadOnly &&
#ifdef MAC
				!pfcb->fWord3File && cwHeapFree > 250 &&
#else
				CbAvailHeap(sbDds) > 1000 &&
#endif
				!pdod->dop.fBackup && !pdod->fLRDirty &&
				!pdod->fStyDirtyBroken &&
				!FExternalFile(fn) &&
				CpcdForDoc(doc) < WinMac(1000,3000) &&
				CpheForDoc(doc) < WinMac(1500,6400)
#ifdef WIN
				&& (!pdod->fDot ||
				(pdod->docGlsy == docNil && !pfcb->fCompound))
#endif /* WIN */
				)
			{
			FC dfcT = PfcbFn(fn)->cbMac - 
					FcBegOfExt(fn, doc, &fib);
			/* calculate size of block we must allocate on Mac
				heap to backup the tables. */
			/* must have 16K bytes extra in Mac heap */
			/* choice of 16K bytes as critical amount
				is totally ad hoc and empirical. */
			if (dfcT <= 65534
#ifdef MAC
					&& LcbFreeMacMem() - dfcT > 16384L
#endif /* MAC */
					)
				{
				vdfcTblBackup = (uns)dfcT;
				/*  do not sacrifice swap space for backup table */
				vfUrgentAlloc = fFalse;
				if ((vhqchTblBackup = 
						HqAllocLcb((long)vdfcTblBackup)) != hqNil)
					{
					vfUrgentAlloc = fTrue;
					/* if we're actually able to allocate
						needed backup block, then say
						quicksave is permitted. */
					return (fTrue);
					}
				vfUrgentAlloc = fTrue;
				vmerr.mat = matSave; /* above failure shouldn't cause more */
				}
			}
		}
	return(fFalse);
} /* end FQuicksaveOk */


/* %%Function:CpcdForDoc  %%Owner:peterj */
CpcdForDoc(doc)
int doc;
{
	/* counts the number of pieces belonging to the document. */
	int cpcd;
	struct DOD *pdod;

	pdod = PdodDoc(doc);
	cpcd = IMacPlc(pdod->hplcpcd);
	if (pdod->docFtn)
		cpcd += IMacPlc((PdodDoc(pdod->docFtn))->hplcpcd);
	if (pdod->docHdr)
		cpcd += IMacPlc((PdodDoc(pdod->docHdr))->hplcpcd);
#ifdef WIN
	if (pdod->fDot && pdod->docMcr)
		cpcd += IMacPlc((PdodDoc(pdod->docMcr))->hplcpcd);
	if (pdod->docAtn)
		cpcd += IMacPlc((PdodDoc(pdod->docAtn))->hplcpcd);
#endif /* WIN */
	return (cpcd);
}


/* %%Function:CpheForDoc  %%Owner:peterj */
CpheForDoc(doc)
int doc;
{
	struct DOD *pdod = PdodDoc(doc);
	int cphe = 0;
	int docFtn, docHdr;
	struct PLC **hplcphe;

	if ((hplcphe = pdod->hplcphe) != hNil)
		cphe = IMacPlc(pdod->hplcphe);
	if ((docFtn = pdod->docFtn) != docNil &&
			(hplcphe = (PdodDoc(docFtn))->hplcphe) != hNil)
		cphe += IMacPlc(hplcphe);
	if ((docHdr = pdod->docHdr) != docNil &&
			(hplcphe = (PdodDoc(docHdr))->hplcphe) != hNil)
		cphe += IMacPlc(hplcphe);
	return cphe;
}


/* %%Function:CheckElSaveAsCab  %%Owner:peterj */
CheckElSaveAsCab(pcmb)
CMB * pcmb;
{
	BOOL fOKToChange;
	CABSAVE * pcab;
	CESA * pcesa;

	FreezeHp();

	pcab = *pcmb->hcab;
	pcesa = pcmb->pv;

	if (!FUnderstoodDff(DffISaveFmt(pcab->iFmt), fTrue) ||
			!pcesa->fQuicksaveOK ||
			(pcab->iFmt != ISaveFmtDff(dffSaveNative) &&
			pcab->iFmt != ISaveFmtDff(dffSaveDocType)))
		{
		pcesa->fQuicksaveOK = fFalse;
		pcab->fQuicksave = fFalse;
		}

	fOKToChange = !PdodDoc(pcesa->doc)->dop.fLockAtn || 
			FAuthor(pcesa->doc);

	if (!fOKToChange)
		pcab->iFmt = ISaveFmtDff(dffSaveDocType);

	fOKToChange = fOKToChange && 
			(pcab->iFmt == ISaveFmtDff(dffSaveNative) ||
			pcab->iFmt == ISaveFmtDff(dffSaveDocType));

	if (!fOKToChange)
		pcab->fLockAnnot = fFalse;

	MeltHp();
}


/* D O C  T R A N S L A T E */
/* Translates the fields of a doc from the local language to English for
	RTF writing and conversion.
*/

#ifdef INTL

/* %%Function:DocTranslate  %%Owner:peterj */
int DocTranslate(docSrc)
int docSrc;
{
	int docTmp;
	struct CA ca;

	if ((docTmp = DocCloneDocNoText(docSrc, dkDoc, fTrue)) == docNil)
		goto LOom;

	/* Now copy the selection to docTmp */
	SetWholeDoc(docTmp, PcaSet(&ca, docSrc, cp0, CpMacDoc(docSrc)));
	if (vmerr.fMemFail)
		goto LOom;

	if (!FTranslFields(docTmp, fTrue /* fLocal */))
		goto LOom;

	Assert(docTmp != docNil);
	return docTmp;

LOom:
	DisposeDoc(docTmp);
	ErrorEid(eidNoMemory, "DocTranslate");
	return docNil;
}


#endif /* INTL */


/* U N D I R T Y  D O C */
/* %%Function:UndirtyDoc %%Owner:davidlu */
UndirtyDoc(doc)
int doc;
{
	struct DOD *pdod;

	pdod = PdodDoc(doc);
	pdod->fDirty = pdod->fStshDirty = pdod->fLRDirty = pdod->fRepag = fFalse;
	if (pdod->docFtn != docNil)
		PdodDoc(pdod->docFtn)->fDirty = fFalse;
	if (pdod->docHdr != docNil)
		PdodDoc(pdod->docHdr)->fDirty = fFalse;
#ifdef WIN
	if (pdod->fDot && (pdod->docGlsy != docNil))
		PdodDoc(pdod->docGlsy)->fDirty = fFalse;
	Assert(!pdod->fShort);
	if (pdod->docAtn != docNil)
		PdodDoc(pdod->docAtn)->fDirty = fFalse;
#endif

}


