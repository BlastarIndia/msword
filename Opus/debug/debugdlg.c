/* ****************************************************************************
**
**      COPYRIGHT (C) 1986 MICROSOFT
**
** ****************************************************************************
*
*  Module: debugdlg.c
*
**
** REVISIONS
**
** Date       Who Rel Ver      Remarks
**
** ************************************************************************* */

#ifdef DEBUG         /* This whole module is debug only. */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "winddefs.h"
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "prompt.h"
#include "message.h"
#include "cmdtbl.h"
#include "keys.h"


#ifdef PROTOTYPE
#include "debugdlg.cpt"
#endif /* PROTOTYPE */

#ifndef NEWCONTROLS
#define NEWCONTROLS
#endif

extern struct DBS vdbs;
extern struct SEL       selCur;


extern CMD CmdNew();
extern CMD CmdOpen();
extern CMD CmdSaveAs();
extern CMD CmdCustomize();
extern CMD CmdMacro();
extern CMD CmdRecorder();
extern CMD CmdViewPreferences();
extern CMD CmdGlossary();
extern CMD CmdInsBookmark();
extern CMD CmdGoto();
extern CMD CmdSummaryInfo();
extern CMD CmdDocStat();
extern CMD CmdDocument();
extern CMD CmdCharacter();
extern CMD CmdParagraph();
extern CMD CmdSection();
extern CMD CmdInsBreak();
extern CMD CmdInsFile();
extern CMD CmdCompare();
extern CMD CmdPicture();
extern CMD CmdRevMarking();
extern CMD CmdRenumParas();
extern CMD CmdApplyStyDlg();
extern CMD CmdDefineStyle();
extern CMD CmdMergeStyle();
/* extern CMD CmdRenameStyle(); *** not available  */
extern CMD CmdTabs();
extern CMD CmdInsPicture();
extern int TmcNewOpen();
extern CMD CmdSort();
extern CMD CmdSearch();
extern CMD CmdReplace();

/* debugging dialogs */
extern CMD CmdEnableTests();
extern CMD CmdTestFunction();
extern CMD CmdRareEvents();
extern CMD CmdCkDiskFile();
extern CMD CmdDbgMemory();

extern CMD CmdHyphenate();
extern CMD CmdChangeKeys();
extern CMD CmdAssignToMenu();
extern CMD CmdInsHeader();
extern CMD CmdPrint();
extern CMD CmdChangePrinter();
extern CMD CmdPrintMerge();
extern CMD CmdAbout();
extern CMD CmdEditMacro();
extern CMD CmdShowVars();
extern CMD CmdStopRecorder();
extern CMD CmdScribble();
extern CMD CmdControlRun();
extern CMD CmdPasteLink();
extern CMD CmdIndex();
extern CMD CmdIndexEntry();
extern PromptUserName();
extern CMD CmdSpelling();
extern CMD TmcGosubSpellingMM();
extern CMD CmdThesaurus();

extern CMD CmdCatalog();
extern CMD CmdInsField();
extern CMD CmdTOC();
extern CMD CmdPosition();

/**** GLOBALS ****/
int vfScanDlg = FALSE;
/* table of command functions used with CmdScanDialogs */

/* WARNING:  Every dialog should have an idd in dlgdefs.h */

/* Note there should be the same number of entries in this table as
	the number of IDD entries in dlgdefs.h and they should be in the same
	order!! */

csconst int (*rgCmdDlg[])() =
{
			{ NULL },
	                 /* Used to be FLogin,which is gone */
		{ CmdNew },
	               /* REVIEW iftime:  THIS SHOULD ALL BE DONE
										WITH bcm's AND FExecCmd (rp) */
		{ CmdOpen },
	
		{ CmdSaveAs },
	
		{ CmdTOC}


{
	CmdCustomize },
	
		{ CmdMacro },
	
		{ CmdStopRecorder },
	
		{ CmdViewPreferences },
	
		{ CmdGlossary },
	
		{ CmdInsBookmark },
	
		{ CmdGoto },
	
		{ CmdSummaryInfo },
	
		{ NULL },
	 /* DocStat dialog does not have corresponding Cmd function. */
		{ CmdDocument },
	
		{ CmdCharacter },
	
		{ CmdParagraph },
	
		{ CmdSection },
	
		{ CmdInsBreak },
	
		{ CmdInsFile },
	
		{ CmdCompare },
	
		{ CmdRenumParas },
	
		{ CmdApplyStyDlg },
	
		{ CmdDefineStyle },
	
		{ CmdMergeStyle },
	
		{ NULL },
	   /* since GoSubRename can't be called */
		{ CmdTabs },
	
		{ CmdPicture },
	
		{ TmcNewOpen },
	
		{ CmdSort },
	
		{ CmdSearch },
	
		{ CmdReplace },
	

		{ CmdEnableTests },
	
		{ CmdTestFunction },
	
		{ CmdRareEvents },
	
		{ CmdCkDiskFile },
	

		{ CmdHyphenate },
	
		{ CmdChangeKeys },
	
		{ CmdAssignToMenu },
	
		{ CmdInsHeader },
	
		{ CmdPrint },
	
		{ CmdPrintMerge },
	
		{ CmdAbout },
	
		{ CmdEditMacro },
	
		{ NULL },
	
		{ CmdShowVars },
	
		{ CmdScribble },
	
		{ NULL },
	 /* InsertFtn */
		{ NULL },
	  /* Prompt */
		{ CmdChangePrinter },
	
		{ CmdRevMarking },
	
		{ NULL },
	 /* SPARE was filename */
		{ CmdControlRun },
	
		{ CmdPasteLink },
	
		{ PromptUserName },
	 /* user name */
		{ NULL },
	 /* Autosave */
		{ CmdSpelling },
	
		{ TmcGosubSpellingMM },
	
		{ CmdIndex },
	
		{ CmdIndexEntry },
	
		{ CmdThesaurus },
	
		{ CmdCatalog },
	
		{ NULL },
	
		{ NULL },
	 /* Catalog Print */
		{ NULL },
	 /* Catalog Search Progress */
		{ CmdInsField },
	
		{ NULL },
	 /* File Password */
		{ NULL },
	 /* Confirm Replace */
		{ CmdDbgMemory },
	
		{ NULL },
	
		{ NULL },
	  /* Verify File Converter */
		{ NULL },
	
		{ NULL },
	
		{ CmdInsPicture },
	
		{ NULL },
	
		{ NULL },
	
		{ NULL },
	
		{ NULL },
	
		{ CmdPosition}


};


#define iCmdDlgMax (sizeof(rgCmdDlg) / sizeof (int *))





/* C M D  S C A N  D I A L O G S */
/* Causes the command functions for all dialog boxes to be executed
	in sequence.  When any given dialog box is up, the scan can be continued by
	pressing the Cancel button or the OK button.  An Alert will appear
	between dialogs to allow user to continue with scan or cancel scan.

	Note that the actual command function is executed, and so, it will do the
	action associated with the dialog, e.g. open a file. If you just want to see
	the dialogs, use ESC to cancel out of them without performing the action
	whenever possible. 

*/
/*  %%Function:  CmdScanDialogs  %%Owner:  bobz       */


CMD CmdScanDialogs()
{
#ifdef REVIEW /* REVIEW iftime: Not really valid anymore with the new SDM (rp) */
	int  idCmd, tmc;
	CHAR *pch;
	CHAR rgch[cchMaxSz];

	/* if untrue, someone has added a dialog to the project but not
		included it int the debug table, or removed one without updating
		the table */

	/* note rgCmdDlg is a csconst structure and pointers to the table should
		not be used. */

	Assert (iCmdDlgMax == IDDLast);

	idCmd = 1;  /* default starting */
	SetCmdMode(cmmDefaults | cmmDialog);
	vcmb.kc = kcNil;

	do
		{
		pch = rgch;
		CchIntToPpch (idCmd, &pch);
		*pch = '\0';
		tmc = TmcInputPmtMst (mstDbgNextDlg, rgch, ichMaxIDSTR, 
				bcmNil, mmoNormal);
		if (tmc == tmcOK)
			{
			int w = 0;
			pch = rgch;
			while (*pch && !FDigit(*pch)) pch++;
			while (*pch && FDigit(*pch))
				w = w*10 + (*pch++) - '0';
			if (w >= IDDMin && w <= IDDLast)
				idCmd = w;
			pch = rgch + CchCopySz (SzShared("Dialog # "), rgch);
			CchIntToPpch (idCmd, &pch);
			*pch = '\0';
			SzToStInPlace(rgch);
			SetPromptSt (rgch, pdcMode);
			if (rgCmdDlg[idCmd-1] != NULL)
				{
				/*  REVIEW iftime:  CALLING CMD FUNCTION DIRECTLY!!! (rp) */
				/* execute the cmd function for this dialog */
				(*(rgCmdDlg[idCmd-1]))();
				}
			else
				NotImplemented ();
			idCmd++;
			}
		} 
	while (tmc == tmcOK && idCmd < IDDLast);
	RestorePrompt ();

	vfScanDlg = FALSE;
	SetCmdMode(cmmNormal); /* REVIEW iftime: needed? */
#endif
	return cmdOK;
}


#ifdef REVIEW  /* REVIEW iftime: Not really valid anymore with the New SDM  (rp) */

/* C H E C K  I A G */
/* Check that dialog structure and CAB have the same number of CAB entries */
/*  %%Function:  CheckIag  %%Owner:  bobz       */

CheckIag( hcab, iag, pDlg )
HCAB hcab;
int iag;
DLG *pDlg;
{
	char rgch[ichMaxIDSTR];

	if ((*hcab)->iagMac < iag)
		Report(mstDbgCab);
	else  if ((*hcab)->iagMac > iag)
		{
		char *pch = rgch;

/*  To clear up confusion:  a diag in this function is a difference
	between iag's, not an abbreviation for dialog or diagonal. */

		int diag = (*hcab)->iagMac - iag;
		int diagCorrect;

		switch (pDlg->idd)
			{
		case IDDNewDoc:
			diagCorrect = 1;
			break;
		case IDDOpen:
			diagCorrect = 1;
			break;
		case IDDFilename:
			diagCorrect = 2;
			break;
		case IDDSaveAs:
			diagCorrect = 2;
			break;
		case IDDDocSummary:
#ifdef TUNNELSTAT
			diagCorrect = 3;
#else
			diagCorrect = 4;
#endif /* TUNNELSTAT */
			break;
		case IDDDocStat:
			diagCorrect = 13;
			break;
		case IDDParaLooks:
			diagCorrect = 3;
			break;
		case IDDCharacter:
			diagCorrect = 3;
			break;
		case IDDDocument:
			diagCorrect = 1;
			break;
		case IDDDefineStyle:
			diagCorrect = 5;
			break;
		case IDDApplyStyle:
			diagCorrect = 1;
			break;
		case IDDGlossary:
			diagCorrect = 1;
			break;
		case IDDChangeKeys:
			diagCorrect = 12;
			break;
		case IDDAssignToMenu:
			diagCorrect = 4;
			break;
		case IDDHeader:
			diagCorrect = 2;
			break;
		case IDDEdMacro:
			diagCorrect = 1;
			break;
		case IDDRenMacro:
			diagCorrect = 1;
			break;
		case IDDPrompt:
			diagCorrect = 1;
			break;
		case IDDChgPr:
			diagCorrect = 3;
			break;
		case IDDTabs:
			diagCorrect = 1;
			break;
		case IDDFormatPic:
			diagCorrect = 2;
			break;
		case IDDPrint:
			diagCorrect = 1;
			break;
		case IDDAutosave:
			diagCorrect = 1;
			break;
		case IDDCatalog:
			diagCorrect = 5;
			break;
		case IDDPasteLink:
			diagCorrect = 1;
			break;
		default:
			diagCorrect = 0;
			}

		if (diag != diagCorrect)
			{
			pch = PchFillPchId( pch, mstDbg2ManyCab, 80 );
			CchIntToPpch( (*hcab)->iagMac - iag, &pch );
			pch = PchFillPchId( pch, mstDbgShouldBe, 80 );
			CchIntToPpch( diagCorrect, &pch );
			pch += CchCopySz( ".", pch );
			*pch = '\0';
			Alert( rgch );
			}
		}
}


/* C K   D L G */
/* Check that dialog structure and CAB are consistent (have handles
	in the same places). */
/*  %%Function:  CkDlg  %%Owner:  bobz       */

CkDlg(hcab, ptm, itmMac)
HCAB hcab;
SDMTM *ptm;
int itmMac;
{
	int itm, fHandle;
	int iag = 0;

	if (!vdbs.fCkDlg)
		return;

	for (itm = 0; itm < itmMac; itm++, ptm++)
		{
		fHandle = ((char *)RgfHandleFromHcab(hcab))[iag];
		switch (ptm->tmtBase)
			{
#ifdef NEWCONTROLS
		case tmtStaticEdit:
			/* fall through */
#endif /* NEWCONTROLS */
		default:  /* no CAB entry */
			continue;

#ifdef NEWCONTROLS
		case tmtToggleCheck:
			/* fall through */
#endif  /* NEWCONTROLS */
		case tmtCheckBox:
			/* fall through */
		case tmtRadioGroup:
			if (fHandle)
				CkDlgErr(itm, FALSE);
			break;
#ifdef NEWCONTROLS
		case tmtComboBox:
			/* fall through */
#endif  /* NEWCONTROLS */
		case tmtCabText:
			if (!fHandle)
				CkDlgErr(itm, TRUE);
			break;
		case tmtEditText:
			if (ptm->pfnTm == NULL && !fHandle)
				CkDlgErr(itm, TRUE);
			else  if (ptm->pfnTm && fHandle)
				CkDlgErr(itm, FALSE);
			break;
#ifdef NEWCONTROLS
		case tmtDropList:
			/* fall through */
#endif
		case tmtListBox:
			if (ptm->fDir)
				{
				if (ptm->fCombo)
					continue; /* no CAB entry */
				else  if (!fHandle)
					CkDlgErr(itm, TRUE);
				}
			else  if (fHandle)
				CkDlgErr(itm, FALSE);
			break;
			}
		iag++;
		}
}


/*  %%Function:  CkDlgErr  %%Owner:  bobz       */

CkDlgErr(itm, fNeedsHandle)
int itm;
int fNeedsHandle;
{
	char rgch[ichMaxIDSTR];
	char *pch = rgch;

	pch = PchFillPchId( pch, mstDbgItmMismatch, 80 );
	CchIntToPpch( itm, &pch );
	pch = PchFillPchId( pch, mstDbgShould, 80 );
	if (!fNeedsHandle)
		pch = PchFillPchId( pch, mstDbgNot, 80 );
	pch = PchFillPchId( pch, mstDbgBeAHandle, 80 );
	if (fNeedsHandle)
		pch = PchFillPchId( pch, mstDbgNot, 80 );
	pch = PchFillPchId( pch, mstDbgCabNotInit, 80 );
	pch = PchFillPchId( pch, mstDbgCabOut, 80 );
	*pch = '\0';
	IdMessageBoxMstRgwMb( rgch, NULL, MB_MESSAGE );
}



#endif /* REVIEW iftime */
#endif /* DEBUG */
