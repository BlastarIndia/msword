/* ***************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ***************************************************************************
*
*  Module: dlg stuff for open/new related things.
*
*  Functions included:
*		File Work-on stuff.
*		File Compare
*		File New
*               File Open
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** ************************************************************************ */

#define NOMINMAX
#define NOSCROLL
#define NOSHOWWINDOW
#define NOREGION
#define NOWH
#define NORASTEROPS
#define NOGDI
#define NOMETAFILE
#define NOBITMAP
#define NOWNDCLASS
#define NOBRUSH
#define NOWINOFFSETS
#define NONCMESSAGES
#define NOKEYSTATE
#define NOCLIPBOARD
#define NOHDC
#define NOCREATESTRUCT
#define NOTEXTMETRIC
#define NOGDICAPMASKS
#define NODRAWTEXT
#define NOSYSMETRICS
#define NOMENUS
#define NOCOLOR
#define NOPEN
#define NOFONT
#define NOOPENFILE
#define NOMEMMGR
#define NORESOURCE
#define NOSYSCOMMANDS
#define NOICON
#define NOKANJI
#define NOSOUND
#define NOCOMM

#ifdef DEBUG
#ifdef PCJ
/* #define SHOWFLDS */
/* #define DBGDOCOPEN */
#endif /* PCJ */
#endif /* DEBUG */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "ch.h"
#include "props.h"
#include "doc.h"
#include "sel.h"
#include "disp.h"
#include "doslib.h"
#include "file.h" 
#include "rerr.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#define NEWOPENTMC
#include "tmc.h"
#include "newopen.hs"
#include "newopen.sdm"

#include "open.hs"
#include "open.sdm"
#include "new.hs"
#include "new.sdm"
#include "insfile.hs"
#include "doc.hs"  /* just for tmc defines */

#include "opuscmd.h"
#include "message.h"
#include "core.h"
#include "debug.h"
#include "error.h"
#include "automcr.h"


#ifdef PROTOTYPE
#include "dlgopen.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */


/* E X T E R N A L S */
extern BOOL		fElActive;
extern CHAR		stDOSPath[];
extern BOOL		vfFileCacheDirty;
extern BOOL		vfRecording;
extern HWND		vhwndApp;
extern struct SEL       selCur;
extern struct MERR      vmerr;
extern int		wwCur;
extern CHAR		szEmpty[];
extern int		docGlobalDot;
extern CHAR	    	szDoc[];
extern CHAR	    	szDot[];
extern int          	docMac;
extern struct DOD       **mpdochdod[];


/* The Work-On Cache Commands */
/*  %%Function:  CmdWkOn1  %%Owner:  bobz       */

CMD CmdWkOn1(pcmb)
CMB *pcmb;
{
	return FWorkOn(0) ? cmdOK : cmdError;
}


/*  %%Function:  CmdWkOn2  %%Owner:  bobz       */

CMD CmdWkOn2(pcmb)
CMB *pcmb;
{
	return FWorkOn(1) ? cmdOK : cmdError;
}


/*  %%Function:  CmdWkOn3  %%Owner:  bobz       */

CMD CmdWkOn3(pcmb)
CMB *pcmb;
{
	return FWorkOn(2) ? cmdOK : cmdError;
}


/*  %%Function:  CmdWkOn4  %%Owner:  bobz       */

CMD CmdWkOn4(pcmb)
CMB *pcmb;
{
	return FWorkOn(3) ? cmdOK : cmdError;
}


/*  %%Function:  CmdWkOn5  %%Owner:  bobz       */

CMD CmdWkOn5(pcmb)
CMB *pcmb;
{
	return FWorkOn(4) ? cmdOK : cmdError;
}


/*  %%Function:  CmdWkOn6  %%Owner:  bobz       */

CMD CmdWkOn6(pcmb)
CMB *pcmb;
{
	return FWorkOn(5) ? cmdOK : cmdError;
}


/*  %%Function:  CmdWkOn7  %%Owner:  bobz       */

CMD CmdWkOn7(pcmb)
CMB *pcmb;
{
	return FWorkOn(6) ? cmdOK : cmdError;
}


/*  %%Function:  FWorkOn  %%Owner:  bobz       */

FWorkOn(mru)
int mru;
{
	extern struct STTB ** vhsttbOpen;

	Assert( vhsttbOpen != hNil && mru >= 0 );

	if (mru >= (*vhsttbOpen)->ibstMac)
		{
		if (fElActive)
			RtError(rerrOutOfRange);
		return fFalse;
		}

	Assert(*PstFromSttb(vhsttbOpen, mru) < ichMaxFile);

	if (DocOpenStDof(PstFromSttb(vhsttbOpen, mru), dofCmdNewOpen, NULL) == docNil)
		/* this may open or new a file; if opening
			a different file, the autoopen and the mods
			to the mru list are handled in DocOpenStDof
			including deleting entry if not found
		*/
		return fFalse;

	CmdRunAtmOnDoc(atmOpen, selCur.doc);

	return fTrue;
}


/* C M D  O P E N */
/*  %%Function:  CmdOpen  %%Owner:  bobz       */

CMD CmdOpen(pcmb)
CMB * pcmb;
{
	int tmc;

	tmc = TmcOpen(pcmb);
	return tmc == tmcOK ? cmdOK : tmc == cmdCancelled ? tmc : cmdError;
}


CHAR **hstTermFileNorm = hNil;

/*  %%Function:  TmcOpen  %%Owner:  bobz       */

int TmcOpen(pcmb)
CMB * pcmb;
{
	int doc;
	CABOPEN * pcabOpen;
	int tmc = tmcOK;


	if (pcmb->fDefaults)
		{
		CHAR szAll[6];

		BuildSzAll(szDoc, szAll);
		pcmb->pv = 0; /* used at dlmInit time. Not nil when we start
						 with other than wildcard */
		if (!FFillCabOpen(pcmb, szAll))
			return cmdNoMemory;
		}

	/* note: the ChainCmd is done here rather than in the
		action part because a macro would have no way to get the
		tmc back if it just put up the dialog. This way, the
		Catalog can be done as part of the dialog process
		bz
	*/

	if (pcmb->fDialog)
		{
		if ((tmc = TmcDoFileDlg(pcmb)) == tmcCancel)
			return cmdCancelled;
		else  if (tmc == tmcError)
			return cmdError;
		else  if (tmc == tmcOpenCatalog)
			{
			ChainCmd(bcmCatalog);
			return tmcOK;
			}
		}

	if (pcmb->fCheck)
		{
		if (!FTermFile(pcmb, Iag(CABOPEN, hszFile),
				tmcNull, fFalse, fFalse, nfoNormal))
			return cmdError;
		}

	if (pcmb->fAction)
		{
		Assert (tmc == tmcOK);
		pcabOpen = (CABOPEN *) PcabFromHcab(pcmb->hcab);
#ifdef DEBUG
			/* BLOCK - make sure FTermFile filled in hst with normed file */
			{
			CHAR stNorm[ichMaxFile];
			CHAR st[ichMaxFile];
			GetSzFile(pcmb->hcab, st, sizeof(st), Iag(CABOPEN, hszFile));
			SzToStInPlace(st);
			Assert(FNormalizeStFile(st, stNorm, nfoNormal));
			Assert(FEqualSt(stNorm, *hstTermFileNorm));
			}
#endif /* DEBUG */

		if ((doc = DocOpenStDof(*hstTermFileNorm, dofNormalized|
				(pcabOpen->fReadOnly?dofReadOnly:0)|dofNormal, NULL))
				== docNil)
			{
			/*  error already reported */
			return tmcError;
			}

		CmdRunAtmOnDoc(atmOpen, selCur.doc);
		}

	return tmcOK;
}


/*  %%Function:  FFillCabOpen  %%Owner:  bobz       */

FFillCabOpen(pcmb, szInit)
CMB * pcmb;
CHAR *szInit;   /* initial file name string */

{
	CABOPEN *pcabopen;
	HCAB hcab;

	hcab = pcmb->hcab;
	FSetCabSz(hcab, szInit, Iag(CABOPEN, hszFile));
	pcabopen = (CABOPEN *) PcabFromHcab(hcab);
	pcabopen->iDirectory = uNinchList;
	pcabopen->fReadOnly = fFalse;

	/* fMemFail will be true if FSetCabSz fails */
	return (!vmerr.fMemFail);

}


/* T M C   D O  F I L E  D L G */
/*  %%Function:  TmcDoFileDlg  %%Owner:  bobz       */

TmcDoFileDlg(pcmb)
CMB * pcmb;
{
	int tmc;
	char dlt [sizeof (dltOpen)];
	CHAR stDOSPathOld[ichMaxFile + 1];

	/* To check if the path was changed by the open dialog. */
	CopySt(stDOSPath, stDOSPathOld);

	BltDlt(dltOpen, dlt);
	if ((tmc = TmcOurDoDlg(dlt, pcmb)) == tmcError)
		{
		return tmcError;
		}

	/* Path has been changed by the dialog, update window titles. */
	if (FNeNcSt(stDOSPath, stDOSPathOld))
		{
		if (vfRecording)
			RecordChdir(stDOSPath);
		vfFileCacheDirty = fTrue;
		UpdateTitles();
		}

	if (tmc == tmcOK && vfRecording)
		FRecordCab(pcmb->hcab, IDDOpen, tmcOK, imiOpen);

	return (tmc);
}



/* ****
*
	Function: FDlgOpen
*  Author:
*  Copyright: Microsoft 1986
*  Date: 3/10/86
*
*  Description: Dialog function for "open class" dialogs
*
** ***/
/*  %%Function:  FDlgOpen  %%Owner:  bobz       */


BOOL FDlgOpen(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int iT;
	CMB * pcmb;


	switch (dlm)
		{
	default:
		return (fTrue);
	case dlmInit:
		pcmb = PcmbDlgCur();
		if (pcmb->pv != 0)
			/* we have something other than wildcard really.
				From NewOpen stuff where we loaded in wildcard,
				but left this around.   */
			{
			SetTmcText(tmcOpenFileName, pcmb->pv);
			}
		if (fElActive)
			EnableTmc(tmcOpenCatalog, fFalse);
		break;
	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (wNew /* cIdle */ == 3)
			FAbortNewestCmg (cmgLoad1, fTrue, fTrue);
		else  if (wNew /* cIdle */ == 5)
			FAbortNewestCmg (cmgLoad2, fTrue, fTrue);
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */

	case dlmClick:
		/* non-atomic combos get no dlmChange on list box click.
			if we get any non-nil entry, be sure OK is enabled
		*/
		if  (tmc == tmcOpenFileList || tmc == tmcOpenFileDir)
			{
			GrayButtonOnBlank(tmcOpenFileName, tmcOK);
			}
		break;
	case dlmChange:
			{
			/* disable Ok if no filename string */
			if  (tmc == (tmcOpenFileName & ~ftmcGrouped))
				{
				GrayButtonOnBlank(tmcOpenFileName, tmcOK);
				break;
				}
			break;
			}

	case dlmDirFailed:
		ErrorEid(eidBadFileDlg," FDlgOpen");
		SetTmcTxs (tmcOpenFileName, TxsAll());
		return fFalse;    /* could not fill directory; stay in dlg */
	case dlmTerm:
			{

#ifdef BZ
			CommSzNum(SzShared("dlm at term: open"), dlm);
			CommSzNum(SzShared("wNew at term: open"), wNew);
#endif
			UpdateStDOSPath();

			if (tmc != tmcOK)
				break;

			return (FTermFile (PcmbDlgCur(), Iag(CABOPEN, hszFile),
					tmcOpenFileName, fFalse /* fLink */,
					fFalse /* fDocCur  */, nfoNormal));
			}  /* dlmTerm */


		}  /* switch */
	return (fTrue);

}


/* dltTerm routine for Open class dialogs */
/* %%Function:  FTermFile   %%Owner:  bobz       */


FTermFile (pcmb, iag, tmcErrorSel, fLink, fDocCur, nfo)
CMB * pcmb;
int iag, tmcErrorSel, fLink, fDocCur, nfo;
{
	BOOL fOnDisk, fOnLine;
	CHAR stName[ichMaxBufDlg];
	CHAR stNorm[ichMaxFile];

	int eid;
	HCAB hcab;

	if (tmcErrorSel != tmcNull)
		{
		/* may be notfilled due to parse errors. If null should
			not get this far, but bag out safely in case of new
			callers bz
		*/

		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled
				|| hcab == hcabNull)
			return fFalse;
		}
	else
		hcab = pcmb->hcab;

	GetSzFile(hcab, stName, sizeof(stName), iag);
	SzToStInPlace(stName);
	if (!FValidFilename (stName, stNorm, NULL, &fOnDisk,
			&fOnLine, nfo))
		{
		eid = eidBadFileDlg;
LErrRet:
		ErrorEid(eid," FTermFile");
		if (tmcErrorSel != tmcNull)
			SetTmcTxs (tmcErrorSel, TxsAll());
		return (fFalse);  /* will stay in dialog */
		}

	if (!fLink && !fOnDisk && !fOnLine)
		{
		eid = eidFileNotExist;
		goto LErrRet;
		}

	if (fDocCur && DocFromSt(stNorm) == selCur.doc)
		{
		eid = eidCantInsertSelf;
		goto LErrRet;
		}

	/* put the normalized name into global */
	FreePh(&hstTermFileNorm);
	if ((hstTermFileNorm = HAllocateCb(*stNorm+1)) == hNil)
		return fFalse;
	else
		bltb(stNorm, *hstTermFileNorm, *stNorm+1);

	return (fTrue);

}


/* G E T  S Z  F I L E */
/*  %%Function:  GetSzFile  %%Owner:  bobz       */

GetSzFile(hcab, szName, cbSzName, iag)
HCAB hcab;
CHAR *szName;
int cbSzName;
int iag;
{
	GetCabSz(hcab, szName, cbSzName, iag);
	CchStripString (szName, CchSz(szName) - 1);

}


/*  %%Function:  CmdNew  %%Owner:  bobz       */

CMD CmdNew(pcmb)
CMB * pcmb;
{
	int tmc;

	tmc = TmcNew(pcmb);
	return tmc == tmcOK ? cmdOK : tmc == cmdCancelled ? tmc : cmdError;
}


/*  %%Function:  TmcNew  %%Owner:  bobz       */

int TmcNew(pcmb)
CMB * pcmb;
{

	int tmc = tmcOK;
	int fContinue;

	if (pcmb->fDefaults)
		{
		if (FFillCabNew(pcmb))
			tmc = tmcOK;
		else
			return tmcError;
		}

	if (pcmb->fDialog)
		{
		tmc = TmcDoCabNew(pcmb);
		}

	if (pcmb->fAction &&
			(fContinue = (tmc == tmcOK || tmc == tmcSummary)) )
		{
		if (CmdCreateNewDocPcmb(pcmb) != cmdOK)
			return tmcError;

		if (tmc == tmcSummary)
			FExecCmd(bcmSummaryInfo);

		CmdRunAtmOnDoc(atmNew, selCur.doc);
		}

	return (fContinue ? tmcOK : tmc);

}


/*  %%Function:  FFillCabNew  %%Owner:  bobz       */

FFillCabNew(pcmb)
CMB * pcmb;
{
	extern char stNormalDot[];
	CABNEWDOC *pcabNewDoc;
	HCAB hcab;

	hcab = pcmb->hcab;
	FSetCabSt(hcab, stNormalDot, Iag(CABNEWDOC, hszNewType));
	pcabNewDoc = (CABNEWDOC *) PcabFromHcab(hcab);

	pcabNewDoc->fNewDot = fFalse;

	pcmb->pv = 0;

	/* fMemFail will be true if FSetCabSz fails */
	return (!vmerr.fMemFail);

}


/*  %%Function:  TmcDoCabNew  %%Owner:  bobz       */

TmcDoCabNew(pcmb)
CMB * pcmb;
{

	int tmc;
	CABNEWDOC *pcabnewdoc;
	char dlt [sizeof (dltNewDoc)];

	CHAR rgchSgstSubdir[ichMaxFile];
	CHAR stDOSPathOld[ichMaxFile + 1];

	/* To check if the path was changed by the save as dialog. */
	CopySt(stDOSPath, stDOSPathOld);

	BltDlt(dltNewDoc, dlt);

	if ((tmc = TmcOurDoDlg(dlt, pcmb)) == tmcError)
		{
		return cmdError;
		}

	/* Path has been changed by the dialog, update window titles. */
	if (FNeNcSt(stDOSPath, stDOSPathOld))
		{
		vfFileCacheDirty = fTrue;
		UpdateTitles(); /* changes window menu, vhsttbWnd and captions */
		}

	if (tmc == tmcCancel)
		return cmdCancelled;

	return tmc;
}




/*  %%Function:  CmdCreateNewDocPcmb  %%Owner:  bobz       */

CmdCreateNewDocPcmb(pcmb)
CMB * pcmb;
{
	CHAR stType [ichMaxFile];

	GetCabSt(pcmb->hcab, stType, sizeof(stType),
			Iag (CABNEWDOC, hszNewType));

	/* Check for NORMAL */
	if (FStNormal(stType))
		stType[0]=0;

	if (ElNewFile(stType, ((CABNEWDOC *) *pcmb->hcab)->fNewDot) != cmdOK)
		{
		return cmdError;
		}

	return cmdOK;
}



/*  %%Function:  FStNormal  %%Owner:  bobz       */

FStNormal(stType)
CHAR *stType;
{
	/* Check for NORMAL */
	if (*stType)
		{
		CHAR *pstNormal=StFrameKey("NORMAL",Normal);
		CHAR stT[ichMaxFile];
		struct FNS fns;
		if (!FNormalizeStFile(stType, stT, nfoDot))
			return fFalse;
		StFileToPfns(stT, &fns);
		return (FEqNcSt(pstNormal, fns.stShortName));
		}
	else
		return fTrue;

}



/* ****
*
	Function: FDlgNew
*  Author:
*  Copyright: Microsoft 1986
*  Date: 4/10/86
*
*
** ***/
/*  %%Function:  FDlgNew  %%Owner:  bobz       */

BOOL FDlgNew(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{


	switch (dlm)
		{
	default:
		if (fElActive && dlm == dlmInit)
			EnableTmc(tmcSummary, fFalse);
		return (fTrue);

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (wNew /* cIdle */ == 3)
			FAbortNewestCmg (cmgLoad1, fTrue, fTrue);
		else  if (wNew /* cIdle */ == 5)
			FAbortNewestCmg (cmgLoad2, fTrue, fTrue);
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */

	case dlmTerm:
			{
			BOOL fOnLine, fOnDisk;
			HCAB hcab;
			CHAR stDot[ichMaxFile];
			CHAR stT[ichMaxFile];

			if (tmc != tmcOK && tmc != tmcSummary)
				break;


			/*  Check to see that the Type entered is a valid file.
				NOTE that any filename may be entered, it does not have to
				end in .DOT. */

			/* failure may be oom, in which case sdm should abort, or
				invalid values, where we should stay in the dialog
			*/
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				{
				/* dialog will be taken down without EndDlg */
				return (fTrue);
				}

			GetCabSt(hcab, stDot, sizeof(stDot),
					Iag (CABNEWDOC, hszNewType));

			if (FFindFileSpec(stDot, stT, grpfpiDot, nfoDot))
				CopySt(stT, stDot);

			/* note stDot is now st */
			if (*stDot && (!FValidFilename (stDot, stT, NULL, &fOnDisk, 
					&fOnLine, nfoDot) || !(fOnLine || fOnDisk)))
				{
				ErrorEid(eidInvalidDOT," FTermFileNew");
				SetTmcTxs (tmcNewType, TxsAll());
				return fFalse;  /* will stay in dialog */
				}

			return fTrue;
			}
		}
	return fTrue;

}


/*  %%Function:  WListDot  %%Owner:  bobz       */

EXPORT WORD WListDot(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	/* used by file new and format document dialogs */
	if (tmm == tmmCount)
		{
		if (wParam == tmcDocTemplate && !PdodMother(selCur.doc)->fDoc)
			return 0;
		UpdateDOTList(wParam);
		return 0;
		}

	return 0;
}



/* ****
*
	Function: FDlgInsFile
*  Author:
*  Copyright: Microsoft 1988
*  Date: 3/10/88
*
*  Description: Dialog function for insert file dialogs
*
** ***/
/*  %%Function:  FDlgInsFile  %%Owner:  bobz       */


BOOL FDlgInsFile(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int iT;
	CMB * pcmb;


	switch (dlm)
		{
	default:
		return (fTrue);
	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (wNew /* cIdle */ == 3)
			FAbortNewestCmg (cmgLoad1, fTrue, fTrue);
		else  if (wNew /* cIdle */ == 5)
			FAbortNewestCmg (cmgLoad2, fTrue, fTrue);
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */

	case dlmInit:
			{
			break;
			}

	case dlmClick:
		/* non-atomic combos get no dlmChange on list box click.
			if we get any non-nil entry, be sure OK is enabled
		*/
		if  (tmc == tmcIFList || tmc == tmcIFDir)
			{
			if (wNew != uNinchList)
				EnableTmc(tmcOK, fTrue);
			}
		break;
	case dlmChange:
			{
			/* disable Ok if no filename string */
			if  (tmc == (tmcIFName & ~ftmcGrouped))
				{
				GrayButtonOnBlank(tmcIFName, tmcOK);
				break;
				}
			break;
			}
	case dlmDirFailed:
		ErrorEid(eidBadFileDlg," FDlgInsFile");
		SetTmcTxs (tmcIFName, TxsAll());
		return fFalse;    /* could not fill directory; stay in dlg */
	case dlmTerm:
			{

			CABINSFILE *pcabinsfile;

			UpdateStDOSPath();

			if (tmc != tmcOK)
				break;

			pcmb = PcmbDlgCur();
			pcabinsfile = (CABINSFILE *) PcabFromHcab(pcmb->hcab);
			return (FTermFile (pcmb, Iag(CABINSFILE, hszFile), tmcIFName,
					pcabinsfile->fLink, fTrue /* fDocCur */, nfoNormal));
			}  /* dlmTerm */


		}  /* switch */
	return (fTrue);

}


/*  %%Function:  FillDOTComboTmc  %%Owner:  bobz       */

FillDOTComboTmc(tmc, szTHD)
int     tmc;
CHAR   *szTHD;
{
	CHAR   *pchDot;
	CHAR    pb [ichMaxFile + 1];
	struct FINDFILE dta;
	CHAR *pszNormal=SzFrameKey("NORMAL",Normal2);

	StartListBoxUpdate(tmc); /* empties box and starts redraw */

	/* fill the combo box with new doc names with
	extensions stripped. Use FFirst and FNext to get the names
	from the wildcard, then strip the extension.
	
	If Normal is found, leave it out since we will shove it in later anyway 
	*/

	if (FFirst(&dta, szTHD, DA_NORMAL | DA_READONLY))
		goto Cleanup;
	else
		{
		pchDot = index(dta.szFileName, '.');
		if (pchDot != NULL)
			*pchDot = '\0';
		if (FNeNcSz(pszNormal, dta.szFileName))
			AddListBoxEntry(tmc, dta.szFileName);
		}

	while (!FNext(&dta))
		{
		pchDot = index(dta.szFileName, '.');
		if (pchDot != NULL)
			*pchDot = '\0';
		if (FNeNcSz(pszNormal, dta.szFileName))
			AddListBoxEntry(tmc, dta.szFileName);
		}  /* while */


Cleanup:
	/* be sure "Normal" is always in the list */
	AddListBoxEntry(tmc, pszNormal);
	EndListBoxUpdate(tmc);

}


/*  %%Function:  UpdateDOTList  %%Owner:  bobz       */

int UpdateDOTList(tmc)
int tmc;
{

	CHAR stPath [ichMaxBufDlg + 1];
	CHAR szName[ichMaxFile + 1];

	CchGetTHDSz(szName, ichMaxFile);

	/* Save the current path. */

	Assert(ichMaxBufDlg >= ichMaxFile);
	CopySt(stDOSPath, stPath);
	PrettyUpPath(stPath);

	/* fill the combo */

	FillDOTComboTmc(tmc, szName);

	/* restore original directory */

	/* can fail, like if drive a: which was good but now fails */
	FSetCurStzPath (stPath);

	UpdateStDOSPath();
}


/*  %%Function:  CchGetTHDSz  %%Owner:  bobz       */

int CchGetTHDSz(sz, ich)
CHAR    sz[];
int     ich;
{
	Assert(ich <= ichMaxFile);

	if (!FGetStFpi(fpiDotPath, sz))
		AssertDo(FGetStFpi(fpiProgram, sz));
	StToSzInPlace(sz);
	Assert(CchSz(sz) + 6 <= ichMaxFile);
	BuildSzAll(szDot, sz+CchSz(sz)-1);
	return (CchSz(sz) - 1);

}


/*  %%Function:  BuildSzAll  %%Owner:  bobz       */

BuildSzAll(szExt, szAll)
CHAR *szExt, *szAll;
{
	szAll[0] = chDOSWildAll;
	bltb(szExt, szAll+1, 5);
}




/*  %%Function:  TmcNewOpen  %%Owner:  bobz       */

TmcNewOpen(st)
char *st;
{
	CMB cmb;
	char dlt [sizeof (dltNewOpen)];
	int tmc;

	if ((cmb.hcab = HcabAlloc(cabiCABNEWOPEN)) == hNil)
		return tmcError;

	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	if (!FSetCabSt(cmb.hcab, st, 
			Iag(CABNEWOPEN, hszFileName)))
		{
		tmc = tmcError;
		goto LRet;
		}
	BltDlt(dltNewOpen, dlt);

	tmc = TmcOurDoDlg(dlt, &cmb);
LRet:
	FreeCab(cmb.hcab);
	return tmc;

}


/*  %%Function:  DocDoCmdNewOpen  %%Owner:  bobz       */

DocDoCmdNewOpen(st)
CHAR *st;
{
	int tmc;
	tmc = TmcNewOpen (st);

	if (tmc == tmcNONew || tmc == tmcNOOpen)
		{
		CMB cmb;
		HCAB hcab;

		/* get default values in cab */

		/* allocate a local Cab on the stack */

		hcab = HcabAlloc(tmc == tmcNONew ? cabiCABNEWDOC : cabiCABOPEN);
		if (hcab == hNil)
			return docNil;

		cmb.hcab = hcab;
		cmb.cmm = cmmDialog | cmmAction;
		cmb.bcm = bcmNil;

		if (tmc == tmcNONew)
			{
			if (!FFillCabNew(&cmb))
				{
				tmc = tmcError;
				goto LRet;
				}
			}
		else
			{
			CHAR stFile[ichMaxFile];
			CHAR szExt[2+3+1];
			struct FNS fns;

			/* strip off and load *.EXT so the list
				box will fill with the files of the same
				extension. At dlmInit, if cmb.pv != 0,
				shove the file name (as an sz) into the
				edit control
			*/

			/* incoming file name may not be normalized */
			if (!FNormalizeStFile(st, stFile, nfoNormal))
				/* completely bogus -- discard */
				{
				SzToSt(szDoc, fns.stExtension);
				cmb.pv = NULL;
				}
			else
				{
				StFileAbsToStFileRel (stFile, st);
				StFileToPfns( st, &fns ); /* get extension */

				/* set up name for use by dialog */
				*(st + *st + 1) = 0; /* null terminate */
				cmb.pv = st + 1; /* use as sz at init time */
				}

			/* build search path */
			szExt[0] = '*';
			StToSz(fns.stExtension, szExt + 1);
			if (!FFillCabOpen (&cmb, szExt))
				{
				tmc = tmcError;
				goto LRet;
				}
			}

		/* now do the dialog with the name */

		tmc = (tmc == tmcNONew) ? TmcNew(&cmb) : TmcOpen(&cmb);
LRet:
		FreeCab(hcab);

		if (tmc == tmcOK)
			return selCur.doc;
		}

	return docNil;
}



