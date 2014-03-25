/* D L G T A B L E . C */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "disp.h"
#include "doc.h"
#include "ch.h"
#include "props.h"
#include "border.h"
#include "format.h"
#include "sel.h"
#include "cmd.h"
#include "file.h"
#include "prm.h"
#include "debug.h"
#include "screen.h"
#include "table.h"
#include "dlbenum.h"
#include "error.h"
#include "message.h"
#include "rerr.h"
#include "keys.h"
#include "rareflag.h"
#include "inter.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "tablecmd.hs"
#include "tablecmd.sdm"
#include "tableins.hs"
#include "tableins.sdm"
#include "tablefmt.hs"
#include "tablefmt.sdm"
#include "tabletxt.hs"
#include "tabletxt.sdm"



/* E X T E R N A L S */

extern struct SAB       vsab;
extern struct PREF      vpref;
extern struct SEL       selCur;
extern struct CA        caTap;
extern struct CA        caPara;
extern struct CA        caSect;
extern struct CA        caTable;
extern struct TAP       vtapFetch;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern struct WWD       **hwwdCur;
extern int              wwCur;
extern struct SCI       vsci;
extern struct MERR      vmerr;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern CHAR             rgchEop[];
extern CHAR             rgchTable[];

extern int              vitcFirst; /* range of columns being affected  */
extern int              vitcLim;   /*     by Format Table              */

extern int              vitcMac;   /* max itcMac of all selected rows  */
extern HWND             vhwndApp;
extern int		vdxaMinCell;    /* min cell width in selection */
extern int		vdxaGapHalfMax; /* max GapHalf in selection    */
extern BOOL             fElActive;
extern struct FTI       vfti;
extern KMP              **hkmpTable;
extern int		vdxaTLeftMin;	/* min left edge of all rows in sel */
extern int		vdxaTRightMax;  /* max right edge of rows in sel    */
extern struct RF	vrf;



/* G L O B A L S */

struct CORW     vrgcorwCur[4];
int             vrgdxaCol[itcMax];
int             vtMergeCells;
int             vdxaParaWidth;
int             vfselType;



/* C M D  I N S  T A B L E */
/* Command function for Insert Table dialog. */
/* %%Function:CmdInsTable %%Owner:rosiep */
CMD CmdInsTable(pcmb)
CMB *pcmb;
{
	CABINSERTTABLE *pcab;
	struct CA caSel, caUndo;
	CP cpIns, cpUndoFirst;
	int tmc, itc, dxaCenter;
	struct TAP tapRow;
	int tblfmt, cRowsIns;
	int cmd = cmdOK;

	CacheSect(selCur.doc, selCur.cpFirst);
	vdxaParaWidth = vsepFetch.dxaColumnWidth;

	/* fForcePara disables comma- and tab-delimited table conversion */
	vrf.fForcePara = fFalse;

	if (selCur.fIns)
		{
		/* Insert a table at the insertion point */
LInsertion:
		/* can't insert tables within tables; command should be
			disabled if in a table */
		Assert(!FInTableDocCp(selCur.doc, selCur.cpFirst));
		vfselType = fselInsertion;
		}
	else
		{
		/* Turn this selection into a table */
		Assert(!FInTableDocCp(selCur.doc, selCur.cpFirst));

		if (!FDeleteCheck(fFalse, rpkNil, NULL))
			return cmdError;
		else  if (selCur.fIns)  /* FDeleteCheck backed over EOD */
			goto LInsertion;

		vfselType = fselSelection;

		/* find out what table conversion we should propose */
		if ((tblfmt = TblFormatScanSel(&selCur, &vrgcorwCur)) == -1)
			return cmdError;

		/* This case occurs if only final EOP was selected but
			FDeleteCheck didn't fail above; e.g. multiple EOP's
			in a row. */
		if (selCur.cpFirst == selCur.cpLim)
			goto LInsertion;
		}

	/* Check for wrong context if run via Repeat or a Macro */
	/* pcab contains the values we are trying to use; vfselType, tblfmt,
		and vrgcorwCur contain the values appropriate for the current
		selection; they might not match.  If it's Repeat and they don't
		match, we just bring up the dialog again with our proposed values;
		if a Macro, it's an error. */

	if (!pcmb->fDefaults && !pcmb->fDialog)
		{
		pcab = *pcmb->hcab;
		if (vfselType == fselSelection)
			{
			/* wrong type of selection */
			if (pcab->tblfmt == tblfmtInsert || (pcab->tblfmt != tblfmt &&
				(pcmb->fRepeated || vrf.fForcePara)))
				{
				if (!pcmb->fRepeated)
					{
LRtError:
					Assert(fElActive);
					RtError(rerrModeError);
					}
				pcmb->fDefaults = pcmb->fDialog = fTrue;
				}
			/* wrong dimensions */
			else
				{
				int cRows;
				int fValid = fTrue;

				if (pcab->cColumns <= 0 || pcab->cRows <= 0 || pcab->cColumns >= itcMax)
					fValid = fFalse;
				else if (pcab->tblfmt != tblfmtPara)
					{
					cRows = vrgcorwCur[pcab->tblfmt].cColumns / pcab->cColumns;
					if (vrgcorwCur[pcab->tblfmt].cColumns % pcab->cColumns != 0)
						cRows++;
					if (cRows + vrgcorwCur[pcab->tblfmt].cRows > pcab->cRows + 1)
						fValid = fFalse;
					}
				else
					{
					cRows = ((vrgcorwCur[pcab->tblfmt].cRows - 1) / pcab->cColumns) + 1;
					if (cRows > pcab->cRows)
						fValid = fFalse;
					}
				if (!fValid)
					{
					if (!pcmb->fRepeated)
						goto LRtError;
					pcmb->fDefaults = pcmb->fDialog = fTrue;
					}
				}
			}
		}


	/* Set up proposed defaults */
	if (pcmb->fDefaults)
		{
		pcab = *pcmb->hcab;
		pcab->dxaWidth = 0;
		switch (vfselType)
			{
		case fselInsertion:
			pcab->cColumns = 2;
			pcab->cRows = 1;
			pcab->tblfmt = uNinchRadio;
			break;

		case fselSelection:
			if (vrgcorwCur[tblfmt].cColumns >= itcMax)
				pcab->tblfmt = tblfmtPara;
			else
				pcab->tblfmt = tblfmt;
			pcab->cColumns = vrgcorwCur[pcab->tblfmt].cColumns;
			pcab->cRows = vrgcorwCur[pcab->tblfmt].cRows;
			break;
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif
			}
		}
	if (pcmb->fDialog)
		{
		CHAR dlt[sizeof(dltInsertTable)];

		BltDlt(dltInsertTable, dlt);

		switch (tmc = TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;
			}
		}

	if (!pcmb->fAction)
		return cmdOK;

	/* do macro error checking */
	if (pcmb->fCheck)
		{
		pcab = *pcmb->hcab;
		if (pcab->cColumns < 1 || pcab->cColumns > itcMax - 1)
			{
			RtError(rerrOutOfRange);
			/* NOT REACHED - RtError does a DoJump */
			Assert(fFalse);
			Debug(return cmdError);
			}

		if (vfselType != fselTextToTable && pcab->dxaWidth != 0 &&
				(pcab->dxaWidth < dxaMinDefaultColumnWidth || 
				pcab->dxaWidth > (xaRightMaxSci - dxTableBorderMax) / 
				pcab->cColumns))
			{
			RtError(rerrOutOfRange);
			/* NOT REACHED - RtError does a DoJump */
			Assert(fFalse);
			Debug(return cmdError);
			}
		}

	if (!FSetUndoBefore(ucmNewTable, uccPaste))
		return cmdCancelled;

	StartLongOp();

	pcab = *pcmb->hcab;

	/* Set up the initial tap */
	if (pcab->cColumns <= 0)
		pcab->cColumns = 1;

	if (pcab->dxaWidth == 0) /* Auto */
		{
		pcab->dxaWidth = max(vdxaParaWidth / pcab->cColumns,
				dxaMinDefaultColumnWidth);
		}

	SetWords(&tapRow, 0, cwTAP);
	tapRow.dxaGapHalf = dxaDefaultGapHalf;
	tapRow.rgdxaCenter[0] = dxaCenter = -dxaDefaultGapHalf;
	for (itc = 1 ; itc <= pcab->cColumns ; itc++)
		tapRow.rgdxaCenter[itc] = (dxaCenter += pcab->dxaWidth);

	switch (vfselType)
		{
	case fselInsertion:
		/* Make sure there is a paragraph mark before the table */
		caSel = selCur.ca;
		cpUndoFirst = selCur.cpFirst;
		if (!FEopBeforePca(&caSel))
			{
LNoMemory:
			SetUndoNil();
			cmd = cmdNoMemory;
			goto LRet;
			}
		cpIns = caSel.cpFirst;
		/* Insert needed chTables */
		pcab = *pcmb->hcab;
		if (pcab->cRows <= 0)
			pcab->cRows = 1;
		cRowsIns = CInsertEmptyTable(selCur.doc, cpIns, pcab->cRows,
				pcab->cColumns, &tapRow, fTrue, fTrue);  /* HEAP MVMT */
		pcab = *pcmb->hcab;
		PcaSet(&caUndo, selCur.doc, cpUndoFirst,
				cpIns + cRowsIns * (pcab->cColumns + 1) * ccpEop);
		SelectIns(&selCur, cpIns);
		PushInsSelCur();
		break;

	case fselSelection:
		CacheSect(selCur.doc, selCur.cpFirst);
		if (caSect.cpLim <= selCur.cpLim)
			{
			/* We don't allow section breaks in tables. */
			ErrorEid(eidIllegalTextInTable, "CmdInsTable");
			SetUndoNil();
			cmd = cmdError;
			goto LRet;
			}
		caUndo = selCur.ca;
		TurnOffSel(&selCur);  /* HEAP MVMT */
		caSel = selCur.ca;
		/* Make sure there is a para mark before the table */
		if (!FEopBeforePca(&caSel))
			goto LNoMemory;

		caUndo.cpLim = caSel.cpLim;
		if (!FCheckPcaEndDoc(&caUndo))
			goto LNoMemory;

		/* set up the table movement keymap */
		if (!hkmpTable)
			{
			if ((hkmpTable = HkmpCreate(cKmeTable, kmfPlain)) == hNil)
				goto LNoMemory;
			AddKeyToKmp(hkmpTable, kcTab, bcmNextCell);
			AddKeyToKmp(hkmpTable, KcShift(kcTab), bcmPrevCell);
			}

		/* insert a table around the selection */
		pcab = *pcmb->hcab;
		if (!FFormatTablePca(&caSel, &caUndo, pcab->tblfmt, pcab->cRows, pcab->cColumns, &tapRow))
			{
			selCur.doc = caSel.doc;
			SetUndoNil();
#ifdef NOTUSED
			/* cmdNoMemory and cmdError are the same value.  Old code: */
			cmd = (vmerr.fMemFail ? cmdNoMemory : cmdError);
#endif /* NOTUSED */
			cmd = cmdError;
			goto LRet;
			}

		/* select our new table */
		selCur.doc = caSel.doc;
		SelectRow(&selCur, CpTableFirst(caSel.doc, caSel.cpFirst),
				CpTableLim(caSel.doc, caSel.cpFirst));

		PushInsSelCur();
		break;
		}


	SetUndoAfter(&caUndo);

	/* if we didn't get the total # of rows we requested, error */
	pcab = *pcmb->hcab;
	if (vfselType == fselInsertion && cRowsIns != pcab->cRows)
		{
#ifdef NOTUSED
		/* cmdNoMemory and cmdError are the same value.  Old code: */
		cmd = (vmerr.fMemFail ? cmdNoMemory : cmdError);
#endif /* NOTUSED */
		cmd = cmdError;
		goto LRet;
		}

	/* tunnel to the Format Table dialog */
	if (tmc == tmcTunnelFormat)
		{
		CacheTable(selCur.doc, selCur.cpFirst);
		SelectRow(&selCur, caTable.cpFirst, caTable.cpLim);
		ChainCmd(bcmFormatTable);
		}

LRet:
	EndLongOp(fFalse);

	return cmd;
}


/* F  D L G  I N S E R T  T A B L E */
/* Dialog function for Insert Table */
/* %%Function:FDlgInsertTable %%Owner:rosiep */
BOOL FDlgInsertTable(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int cCols, cRows;
	int rgtmc[2];
	int tblfmt;

	switch (dlm)
		{
	case dlmInit:
		if (vrf.fForcePara)
			EnableTmc(tmcConvertFrom, fFalse);

		if (vfselType == fselInsertion)
			{
			/* With an insertion point there is no format */
			EnableTmc(tmcConvertFrom, fFalse);
			}
		else  if ((vfselType == fselSelection || 
				vfselType == fselTextToTable) &&
				((CABINSERTTABLE *) *(HcabDlgCur()))->tblfmt
				!= tblfmtPara)
			{
			/* Unless paragraph format is selected,
				the number of rows and columns is
				dependant on the text selected */
			EnableTmc(tmcRows, fFalse);
			}
		if (fElActive)
			EnableTmc(tmcTunnelFormat, fFalse);

		break;

	case dlmTerm:
		if (tmc != tmcCancel && vfselType != fselTable)
			{
			cCols = ValGetTmc(tmcColumns);
			if (FBadTmcCheck(tmcColumns, 1, itcMax - 1, fFalse, fFalse))
				{
				SetTmcTxs(tmcColumns, TxsAll());
				return fFalse;
				}
			if (vfselType != fselTextToTable)
				{
				if (ValGetTmc(tmcInitColWidth) != 0 &&
						FBadTmcCheck(tmcInitColWidth, dxaMinDefaultColumnWidth,
						(xaRightMaxSci-dxTableBorderMax)/cCols, fTrue, fFalse))
					{
					SetTmcTxs(tmcInitColWidth, TxsAll());
					return fFalse;
					}
				}
			if (ValGetTmc(tmcConvertFrom) == tblfmtPara)
				{
				cRows = (vrgcorwCur[0].cRows-1)/cCols+1;
				if (FBadTmcCheck(tmcRows, cRows, 32767, fFalse, fFalse))
					{
					if (FEnabledTmc(tmcRows))
						SetTmcTxs(tmcRows, TxsAll());
					else
						{
						/* if row is disabled col should be setting the value;
							we should never get here in that case, but if we do, we'll
							pop back to cols
						*/
						Assert(fFalse);
						SetTmcTxs(tmcColumns, TxsAll());
						}

					return fFalse;
					}
				}
			}
		break;

	case dlmChange:
		rgtmc[0] = tmcColumns;
		rgtmc[1] = tmcRows;
		GrayRgtmcOnBlank(rgtmc, 2, tmcOK);
		/* Special case: Tunnel buttons should remain grayed if in macro */
		if (!fElActive)
			GrayRgtmcOnBlank(rgtmc, 2, tmcTunnelFormat);
		if (vfselType == fselSelection || vfselType == fselTextToTable)
			{
			if (tmc == tmcColumns)
				{
				if ((tblfmt = ValGetTmc(tmcConvertFrom)) == tblfmtPara)
					{
					cCols = ValGetTmc(tmcColumns);
					cCols = max(cCols, 1);
					SetTmcVal(tmcRows, (vrgcorwCur[0].cRows-1)/cCols+1);
					}
				else
					{
					cCols = ValGetTmc(tmcColumns);
					cCols = max(cCols, 1);
					cRows = vrgcorwCur[tblfmt].cColumns/cCols;
					if (vrgcorwCur[tblfmt].cColumns % cCols != 0)
						cRows++;
					SetTmcVal(tmcRows, vrgcorwCur[tblfmt].cRows*cRows);
					}
				}
			}
		break;

	case dlmClick:
		if (vfselType == fselSelection || vfselType == fselTextToTable)
			{
			/* Change the Rows and Columns */
			if (tmc == tmcFormatPara || tmc == tmcFormatTabs || tmc == tmcFormatComma)
				{
				int tblfmt = ValGetTmc(tmcConvertFrom);
				SetTmcVal(tmcColumns, vrgcorwCur[tblfmt].cColumns);
				SetTmcVal(tmcRows, vrgcorwCur[tblfmt].cRows);
				EnableTmc(tmcRows, tblfmt == tblfmtPara);
				}
			SetTmcVal(tmcInitColWidth, 0);
			}
		break;
		}

	return fTrue;
}


/* C M D  E D I T  T A B L E */
/* Command function for Edit Table dialog. */
/* %%Function:CmdEditTable %%Owner:rosiep */
CMD CmdEditTable(pcmb)
CMB *pcmb;
{
	CABEDITTABLE *pcab;
	int tmc;
	int itcFirst, itcLim, itc;
	int fSplit = fTrue, fMerge = fTrue;
	CP cpCur, cpLim;
	struct CA caTable;
	int cmd = cmdOK;

	if (pcmb->fDefaults)
		{
		/* initialize Command Argument Block */
		pcab = *pcmb->hcab;

		if (selCur.sk == skRows)
			pcab->cmdtype = cmdtypeRow;
		else  if (FPcaWholeTable(&selCur.ca))
			pcab->cmdtype = cmdtypeColumn;
		else
			pcab->cmdtype = cmdtypeSelection;

		pcab->fShiftVert = 0; /* must init to something */
		}

	ItcsFromSelCur(&itcFirst, &itcLim);

	cpCur = selCur.cpFirst;
	cpLim = selCur.cpLim;
	if (selCur.fIns)
		cpLim++;

	while (cpCur < cpLim)
		{
		CpFirstTap(selCur.doc, cpCur);
		cpCur = caTap.cpLim;

		if (!vtapFetch.rgtc[itcFirst].fFirstMerged)
			fSplit = fFalse;
		else
			fMerge = fFalse;

		if (vtapFetch.rgtc[itcFirst].fMerged)
			fMerge = fFalse;

		for (itc = itcFirst+1 ; itc < itcLim && itc < vtapFetch.itcMac; itc++)
			{
			if (fSplit && !vtapFetch.rgtc[itc].fMerged)
				fSplit = fFalse;

			if (vtapFetch.rgtc[itc].fMerged || vtapFetch.rgtc[itc].fFirstMerged)
				fMerge = fFalse;
			}
		}

	if (itcFirst == itcLim - 1 || itcFirst == ItcMacPca(&selCur.ca) - 1)
		fMerge = fFalse;

	if (fSplit)
		vtMergeCells = tNeg;
	else  if (!fMerge)
		vtMergeCells = tZero;
	else
		vtMergeCells = tPos;

	StartLongOp();
	if (pcmb->fDialog)
		{
		CHAR dlt[sizeof(dltEditTable)];

		BltDlt(dltEditTable, dlt);

		switch (tmc = TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			cmd = cmdNoMemory;
			goto LRet;

		case tmcCancel:
			cmd = cmdCancelled;
			goto LRet;
			}
		}

	if (!pcmb->fAction)
		{
		cmd = cmdOK;
		goto LRet;
		}

	/* not repeatable, yet we do want to blow away the again state */
	SetAgain(bcmNil);

	pcab = *pcmb->hcab;
	switch (pcmb->tmc)
		{
	case tmcTblInsert:
		switch (pcab->cmdtype)
			{
		case cmdtypeRow:        /* Insert Rows */
			cmd = CmdInsTableRows();
			break;
		case cmdtypeColumn:        /* Insert Columns */
			cmd = CmdInsTableColumns(fTrue);
			break;
		case cmdtypeSelection:        /* Insert single cells */
			cmd = CmdInsTableCells(pcab->fShiftVert);
			break;
			}
		break;

	case tmcTblDelete:
		switch (pcab->cmdtype)
			{
		case cmdtypeRow:        /* Delete Rows */
			cmd = CmdDelTableRows();
			break;
		case cmdtypeColumn:        /* Delete Columns */
			cmd = CmdDelTableColumns(fTrue);
			break;
		case cmdtypeSelection:        /* Delete single cells */
			cmd = CmdDelTableCells(pcab->fShiftVert);
			break;
			}
		break;
	case tmcMergeCells:
	case tmcSplitCells:
		if (vtMergeCells == tZero)
			ModeError();

		if (fSplit)     /* Actually splitting, not merging */
			cmd = CmdSplitCells(&selCur);
		else  /* Merge the cells */
			cmd = CmdMergeCells(&selCur);
		break;
		}

LRet:
	EndLongOp(fFalse);
	return cmd;

}



/* F  D L G  E D I T  T A B L E */
/* Dialog function for Edit Table command. */
/* %%Function:FDlgEditTable %%Owner:rosiep */
BOOL FDlgEditTable(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int fEnable;
	int cmdtype;
	char stSplit[256];
	CABEDITTABLE *pcab;

	switch (dlm)
		{
	case dlmInit:
		if (vtMergeCells == tZero)
			EnableTmc(tmcMergeCells, fFalse);
		else  if (vtMergeCells == tNeg)
			SetTmcText(tmcMergeCells, SzSharedKey("S&plit Cells",SplitCells));
		pcab = (CABEDITTABLE *) *(HcabDlgCur());
		if (pcab->cmdtype != cmdtypeSelection)
			goto LEnable;
		break;

	case dlmClick:
		switch (tmc)
			{
		case tmcRow:
		case tmcColumn:
		case tmcSelection:
LEnable:
			/* Enable or disable Shift radio buttons depending on
			whether selection radio button is on */
			fEnable = fFalse;
			if ((cmdtype = ValGetTmc(tmcCmdType)) == cmdtypeSelection)
				{
				fEnable = fTrue;
				SetTmcVal(tmcShiftDirection, fFalse);
				}
			EnableTmc(tmcShiftDirection, fEnable);
			EnableTmc(tmcShiftText, fEnable);
			break;
			}
		break;
		}

	return fTrue;
}


/* C M D  F O R M A T  T A B L E */
/* Format a Table */
/* %%Function:CmdFormatTable %%Owner:rosiep */
CMD CmdFormatTable(pcmb)
CMB *pcmb;
{
	CABFORMATTABLE *pcab, cabRepeat, cabSave;
	int tmc;
	CMD cmd = cmdOK;

	StartLongOp();

	/* Check to make sure we are legal */
	if (!selCur.fTable && !selCur.fWithinCell)
		{
		/* this should be disabled if not in a table */
		Assert(fFalse);
		Beep();
		cmd = cmdError;
		goto LReturn;
		}

	/* Repeated needs to go through the fDefaults code to set up globals */
	/* Explanation of all the bltb's:  Need to save and restore CAB around
		call to TableSelToCab in the fRepeated case, because TableSelToCab
		changes the CAB.  We could avoid that by letting TableSelToCab only
		change the CAB if !fRepeated, but then we'd have to call it again if
		the Repeat state isn't going to fly, and it can be time consuming.
		To avoid calling it again, we save the CAB it returns in case we
		need it for Repeat if we need to bring the dialog up after all (if
		the settings are invalid for the current selection). */

	if (pcmb->fDefaults || pcmb->fRepeated)
		{
		int itc, dxaCol;

		/* Invalidate globals */
		SetWords(vrgdxaCol, uNinch, itcMax);
		vitcMac = vitcFirst = vitcLim = -1;
		vdxaMinCell = vdxaGapHalfMax = vdxaTRightMax = -1;
		/* can't invalidate vdxaTLeftMin; assume it will be valid it
			vdxaTRightMax is */

		if (pcmb->fRepeated)
			bltb(*pcmb->hcab, &cabSave, sizeof(CABFORMATTABLE));

		/* Get formatting info about the current selection;
			TableSelToCab fills CAB and sets up some globals
			for later error checking. */
		TableSelToCab(&selCur, pcmb);
		bltb(*pcmb->hcab, &cabRepeat, sizeof(CABFORMATTABLE));
		if (pcmb->fRepeated)
			bltb(&cabSave, *pcmb->hcab, sizeof(CABFORMATTABLE));
LDefaults:
		if (pcmb->fRepeated && pcmb->fDialog)
			bltb(&cabRepeat, *pcmb->hcab, sizeof(CABFORMATTABLE));


		/* special fields for macro language */
		pcab = *pcmb->hcab;
		Assert(vitcFirst != -1);
		Assert(vitcLim != -1);
		pcab->wFromCol = vitcFirst+1;
		pcab->wCol = vitcLim;
		if (pcab->wFromCol == pcab->wCol)
			pcab->wFromCol = 0;
		}

	if (pcmb->fDialog)
		{
		CHAR	dlt[sizeof(dltFormatTable)];

		BltDlt(dltFormatTable, dlt);

		switch (tmc = TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			cmd = cmdNoMemory;
			goto LReturn;

		case tmcCancel:
			cmd = cmdCancelled;
			goto LReturn;
			}
		}

	if (pcmb->fCheck)
		{
		CABFORMATTABLE cab, *pcab;

		pcab = *pcmb->hcab;

		if (pcab->ibrclOutline == ibrclShadow)
			{
			pcab->ibrclTop = ibrclSingle;
			pcab->ibrclBottom = ibrclThick;
			pcab->ibrclLeft = ibrclSingle;
			pcab->ibrclRight = ibrclThick;
			}
		else  if (pcab->ibrclOutline != 0)
			{
			pcab->ibrclTop = pcab->ibrclOutline;
			pcab->ibrclBottom = pcab->ibrclOutline;
			pcab->ibrclLeft = pcab->ibrclOutline;
			pcab->ibrclRight = pcab->ibrclOutline;
			}

		/* ibrclOutline / tmcOutline */
		if (pcab->ibrclOutline != uNinchList && 
				pcab->ibrclOutline > 4)
			goto LCheckError;

		/* ibrclTop / tmcTop */
		if (pcab->ibrclTop != uNinchList && pcab->ibrclTop > 4)
			goto LCheckError;

		/* ibrclBottom / tmcBottom */
		if (pcab->ibrclBottom != uNinchList && pcab->ibrclBottom > 4)
			goto LCheckError;

		/* ibrclInside / tmcInside */
		if (pcab->ibrclInside != uNinchList && pcab->ibrclInside > 4)
			goto LCheckError;

		/* ibrclLeft / tmcLeft */
		if (pcab->ibrclLeft != uNinchList && pcab->ibrclLeft > 4)
			goto LCheckError;

		/* ibrclRight / tmcRight */
		if (pcab->ibrclRight != uNinchList && pcab->ibrclRight > 4)
			goto LCheckError;

		/* jc / tmcTFAlign */
		if (pcab->jc != uNinchRadio && pcab->jc > 2)
			goto LCheckError;

		/* use frame variable to avoid heap movement problems */
		bltb(pcab, &cab, sizeof(CABFORMATTABLE));

		if (!FCheckFormatTable(&cab, fFalse))
			{
			if (pcmb->fRepeated)
				{
				pcmb->fDialog = fTrue;
				goto LDefaults;
				}
LCheckError:
			ErrorEid(eidDxaOutOfRange, "CmdFormatTable");
			return cmdError;
			}
		}

	if (pcmb->fAction)
		{
		Assert(pcmb->fDialog || pcmb->fCheck);

		pcab = *pcmb->hcab;

		/* special fields for macro language */
		vitcLim = pcab->wCol;
		if ((vitcFirst = pcab->wFromCol) == 0)
			vitcFirst = pcab->wCol - 1;
		else
			vitcFirst--;

		/* Apply properties */
		if (!FApplyHcabToTap(pcmb->hcab))
			return cmdCancelled;
		}

LReturn:
	EndLongOp(fFalse);
	return cmd;
}



/* F  D L G  F O R M A T  T A B L E */
/* Dialog function for Format Table command. */
/* %%Function:FDlgFormatTable %%Owner:rosiep */
BOOL FDlgFormatTable(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CABFORMATTABLE *pcab;
	HCAB hcab;
	int dxaGap;
	struct CA caApply;
	int val;
	int dxaColMin, dxaColMax, fIndentBad;
	BOOL fFail;

	switch (dlm)
		{
	case dlmInit:
		Assert(vitcFirst != -1);
		Assert(vitcLim != -1);
		AddColsToDlg(vitcFirst, vitcLim);
		pcab = (CABFORMATTABLE *) *(HcabDlgCur());
		if (pcab->fWholeTable)
			EnableTmc(tmcApplyTo, fFalse);
		break;

	case dlmTerm:
	case dlmClick:
		/* We shouldn't get both a dlmClick and a dlmTerm for OK;
			don't want to do this code twice */
		Assert(dlm == dlmTerm || tmc != tmcOK);

		/* check validity of cab values */
		if (tmc == tmcOK || tmc == tmcNextColumn || tmc == tmcPrevColumn)
			{
			CABFORMATTABLE cab;
			/* failure may be oom, in which case sdm should abort, or
				invalid values, where we should stay in the dialog
			*/
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				{
				/* dialog will come down normally if dlmterm;
				                                otherwise FFilterSdmMsg will take dialog down
				                             */
				return (fTrue);
				}

			/* use frame variable to avoid heap movement problems */
			bltb(*hcab, &cab, sizeof(CABFORMATTABLE));

			if (!FCheckFormatTable(&cab, fTrue /* fDialog */))
				return fFalse;
			}

		if (dlm == dlmTerm)
			break;

		switch (tmc)
			{
		case tmcNextColumn:
		case tmcPrevColumn:
			/* failure may be oom, in which case sdm should abort, or
				invalid values, where we should stay in the dialog
			*/
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				{
				/* FFilterSdmMsg will take dialog down */
				return (fTrue);
				}

			/* Apply properties to current selection */
			FApplyHcabToTap(hcab);

			/* Move selection to next or prev column */
			Assert(vitcFirst != -1);
			Assert(vitcLim != -1);
			if (tmc == tmcNextColumn)
				vitcFirst = vitcLim % vitcMac;
			else
				vitcFirst = (vitcFirst + vitcMac - 1) % vitcMac;
			vitcLim = vitcFirst + 1;
			ChangeFormatTableSel();
			AddColsToDlg(vitcFirst, vitcLim);
			SetTmcVal(tmcFTElCol, vitcLim);
			SetTmcVal(tmcFTElFromCol, 0);
			break;

		case tmcOutline:
			if (wNew == ibrclShadow)
				{
				/* fake shadow border */
				SetTmcVal(tmcTop, ibrclSingle);
				SetTmcVal(tmcBottom, ibrclThick);
				SetTmcVal(tmcLeft, ibrclSingle);
				SetTmcVal(tmcRight, ibrclThick);
				}
			else
				{
				SetTmcVal(tmcTop, wNew);
				SetTmcVal(tmcBottom, wNew);
				SetTmcVal(tmcLeft, wNew);
				SetTmcVal(tmcRight, wNew);
				}
			break;
			}

		break;
		}

	return fTrue;
}


/*  F  C H E C K  F O R M A T  T A B L E  */
/* Error checking for the Format Table command.  If fDialog is true, it's
	for the dialog, otherwise it's for the macro language or Repeat.
	Note:  pcabFrame is not a heap pointer, it is a pointer to a frame
	variable in the caller, so we don't need to worry about heap movement
	in here.
*/
/* %%Function:FCheckFormatTable %%Owner:rosiep */
BOOL FCheckFormatTable(pcabFrame, fDialog)
CABFORMATTABLE *pcabFrame;
BOOL fDialog;
{
	struct CA caApply;
	int dxaWidth, dxaGap, val;
	int dxaColMin, dxaColMax;
	BOOL fFail, fIndentBad;

	if (pcabFrame->fWholeTable)
		{
		CacheTable(selCur.doc, selCur.cpFirst);
		caApply = caTable;
		}
	else
		caApply = selCur.ca;

	/* dxaGap / tmcGapSpace */
	dxaGap = pcabFrame->dxaGap;
	if (vdxaMinCell == -1)
		vdxaMinCell = DxaMinCellFromCa(&caApply);  /* HM */

	if (dxaGap != uNinch && dxaGap > vdxaMinCell)
		{
		if (fDialog)
			{
			SetTmcTxs(tmcGapSpace, TxsAll());
			RangeError(0, vdxaMinCell, fTrue, vpref.ut);
			}
		return fFalse;
		}

	if (dxaGap != uNinch)
		vdxaGapHalfMax = dxaGap / 2;
	else  if (vdxaGapHalfMax == -1)
		vdxaGapHalfMax = DxaMaxGapFromCa(&caApply);  /* HM */

	/* dxaWidth / tmcColumnWidth */
	dxaWidth = pcabFrame->dxaWidth;
	Assert(vitcFirst != -1);
	Assert(vitcLim != -1);
	fFail = !FOkExpandTable(&selCur.ca, pcabFrame->fWholeTable, vitcFirst,
			vitcLim, dxaWidth, pcabFrame->dxaLeft, fDialog ? &dxaColMin : NULL,
			fDialog ? &dxaColMax : NULL, fDialog ? &fIndentBad : NULL);

/* WARNING: dxaColMin, dxaColMax, and fIndentBad are only filled if the
	dialog is up; they should not be referenced below if !fDialog */

	if (fFail && fIndentBad)
		{
		if (fDialog)
			{
			SetTmcTxs(tmcIndent, TxsAll());
			RangeError(xaLeftMinSci, dxaColMax, fTrue, vpref.ut);
			}
		return fFalse;
		}

	if (dxaGap != uNinch)
		dxaColMin = dxaGap;

	/*  0 is Auto, a special case */
	if (dxaWidth != uNinch && dxaWidth != 0 && dxaWidth - czaPoint < dxaColMin)
		fFail = fTrue;

	if (fFail)
		{
		if (fDialog)
			{
			SetTmcTxs(tmcColumnWidth, TxsAll());
			if (dxaColMin >= dxaColMax)
				/* An odd case, but it can happen */
				ErrorEid(eidTooWideForOperation, "");
			else
				RangeError(dxaColMin+czaPoint, dxaColMax,
						fTrue, vpref.ut);
			}
		return fFalse;
		}

	/* dxaLeft / tmcIndent */
	val = pcabFrame->dxaLeft;
	if (val != wNinch && (val < xaLeftMinSci + vdxaGapHalfMax ||
			val > xaRightMaxSci))
		{
		if (fDialog)
			{
			SetTmcTxs (tmcIndent, TxsAll());
			RangeError(xaLeftMinSci + vdxaGapHalfMax,
					xaRightMaxSci, fTrue, vpref.ut);
			}
		return fFalse;
		}

	return fTrue;
}


/* C H A N G E  F O R M A T  T A B L E  S E L */
/* Changes the current selection to a column selection from vitcFirst to
	vitcLim.   Sets the dialog fields appropriately for the new selection.
	Implicit inputs: vitcFirst, vitcLim */
/* %%Function:ChangeFormatTableSel %%Owner:rosiep */
ChangeFormatTableSel()
{
	struct CA caT;
	int rgbrc[5];
	int tmc, ibrc;

	if (selCur.fTable)
		caT = selCur.ca;
	else  /* This is just applied to a single row */		
		{
		CpFirstTap(selCur.doc, selCur.cpFirst);
		caT = caTap;
		}

	/* show the change made to old column(s), then switch to new */
	UpdateWw(wwCur, fFalse);
	Assert(vitcFirst != -1);
	Assert(vitcLim != -1);
	SelectColumn(&selCur, caT.cpFirst, caT.cpLim, vitcFirst, vitcLim);

	if (vrf.fTablePropsNinch)    /* All Ninch */
		{
 		/* Order assumed by for loop */
 		Assert(tmcTop + 1 == tmcBottom && tmcBottom + 1 == tmcInside &&
			tmcInside + 1 == tmcLeft && tmcLeft + 1 == tmcRight);
		for (tmc = tmcTop; tmc <= tmcRight; tmc++)
			{
			SetTmcVal(tmc, ibrclNone); /* because brain-dead SDM doesn't
			                          clear the list box when it is ninch */
			SetTmcVal(tmc, uNinchList);
			}
		}
	else
		{
		GetCellBorders(rgbrc, &selCur);
		SetTmcVal(tmcOutline, ibrclNone);
		SetTmcVal(tmcTop, IbrclFromBrc(rgbrc[ibrcTop]));
		SetTmcVal(tmcBottom, IbrclFromBrc(rgbrc[ibrcBottom]));
		SetTmcVal(tmcLeft, IbrclFromBrc(rgbrc[ibrcLeft]));
		SetTmcVal(tmcRight, IbrclFromBrc(rgbrc[ibrcRight]));
		SetTmcVal(tmcInside, IbrclFromBrc(rgbrc[ibrcInside]));
		}

	if (ValGetTmc(tmcApplyTo) == 1 && !FWholeTable(&selCur))
		{
		EnableTmc(tmcApplyTo, fTrue);
		SetTmcVal(tmcApplyTo, 0);
		}

	SetTmcTxs(tmcColumnWidth, TxsAll());
}



/* T A B L E  S E L  T O  C A B */
/* Given a selection in a table, set up the CAB for Format Table dialog,
	and store information in various globals.  Used only by CmdFormatTable.
	If pcmb->fRepeated, don't fill the CAB, just fill the globals for
	error checking.

	Globals filled:

	(via call to GetTapState):
	vdxaMinCell	  min cell width in selection
	vdxaGapHalfMax	  max GapHalf in selection
	vdxaTLeftMin      min left edge of all rows in sel
	vdxaTRightMax	  max right edge of all rows in sel

	(done in TableSelToCab):
	vitcFirst	  first column of selection
	vitcLim		  lim column of selection
	vrgdxaCol	  array of column widths (via RgWidthFromCenter)
	vitcMac	          max itcMac of all selected rows (used for
						Next/Prev column calculation)
*/
/* %%Function:TableSelToCab %%Owner:rosiep */
TableSelToCab(psel, pcmb)
struct SEL *psel;
CMB *pcmb;
{
	CABFORMATTABLE *pcab;
	struct CA caT;
	int dxaCol, itc, rgbrc[5];

	caT = psel->ca;

	ItcsFromSelCur(&vitcFirst, &vitcLim);

	/* Check if all the rows match */
	if (!FGetTapState(&caT, fFalse))
		{
/* FUTURE (rp): This will cause the whole dialog to be ninch when really
	only part of it is ninch.  It would be too much work at this point to
	deal with each property separately.  Also FGetTapState compares the
	whole TAP, so if one column were ninch, and then the user did Next or
	Prev Column, the dialog would still be ninch, even if the currently
	selected column were not ninch.  Some day it would be nice to do all
	that right...
*/
		/* All Ninch */
		vrf.fTablePropsNinch = fTrue;
		pcab = *pcmb->hcab;
		pcab->dxaWidth = pcab->dxaGap = uNinch;
		pcab->dxaLeft = pcab->dyaRowHeight = wNinch;
		pcab->jc = uNinchRadio;
		vitcMac = ItcMacPca(&caT);
		SetWords(vrgdxaCol, uNinch, itcMax);
		SetWords(rgbrc, tmvalNinch, cbrcCellBorders);
		}
	else
		{
		/* Rows Match */
		vrf.fTablePropsNinch = fFalse;
		RgWidthFromCenter(vrgdxaCol, vtapFetch.rgdxaCenter,
				vtapFetch.rgtc, vtapFetch.itcMac);
#ifdef DEBUG
		/* verify that they're all valid */
		for (itc = 0; itc < vtapFetch.itcMac; itc++)
			Assert(vrgdxaCol[itc] != uNinch);
#endif
		/* Ninch column width if they don't all match */
		dxaCol = vrgdxaCol[vitcFirst];
		for (itc = vitcFirst + 1; itc < vitcLim; itc++)
			{
			if (dxaCol != vrgdxaCol[itc])
				{
				dxaCol = uNinch;
				break;
				}
			}

		pcab = *pcmb->hcab;
		pcab->dxaWidth = dxaCol;
		pcab->dxaGap = vtapFetch.dxaGapHalf * 2;
		pcab->jc = vtapFetch.jc;
		pcab->dyaRowHeight = vtapFetch.dyaRowHeight;
		pcab->dxaLeft = vtapFetch.rgdxaCenter[0] + vtapFetch.dxaGapHalf;
		vitcMac = vtapFetch.itcMac;
		GetCellBorders(rgbrc, psel);
		}

	pcab = *pcmb->hcab;
	pcab->ibrclOutline = ibrclNone;
	pcab->ibrclTop = IbrclFromBrc(rgbrc[ibrcTop]);
	pcab->ibrclLeft = IbrclFromBrc(rgbrc[ibrcLeft]);
	pcab->ibrclBottom = IbrclFromBrc(rgbrc[ibrcBottom]);
	pcab->ibrclRight = IbrclFromBrc(rgbrc[ibrcRight]);
	pcab->ibrclInside = IbrclFromBrc(rgbrc[ibrcInside]);

	pcab->fWholeTable = fFalse;
	if (FWholeTable(psel))  /* HM */
		{
		pcab = *pcmb->hcab;
		pcab->fWholeTable = fTrue;
		}

}


/* I B R C L  F R O M  B R C */
/* Convert a brc to an ibrcl */
/* %%Function:IbrclFromBrc %%Owner:rosiep */
int IbrclFromBrc(brc)
int brc;
{
	switch (brc)
		{
	default:
		Assert(fFalse);
		/* fall through for robustness */
	case brcNone:
		return(ibrclNone);

	case brcSingle:
		return ibrclSingle;

	case brcTwoSingle:
		return ibrclDouble;

	case brcThick:
		return ibrclThick;

	case (brcSingle | brcfShadow):
		return ibrclShadow;

	case tmvalNinch:
		return uNinchList;
		}
}


/* B R C  F R O M  I B R C L */
/* Convert an ibrcl to a brc */
/* %%Function:BrcFromIbrcl %%Owner:rosiep */
int BrcFromIbrcl(ibrcl)
int ibrcl;
{
	switch (ibrcl)
		{
	default:
		Assert(fFalse);
		/* fall through for robustness */
	case ibrclNone:
		return brcNone;

	case ibrclSingle:
		return brcSingle;

	case ibrclDouble:
		return brcTwoSingle;

	case ibrclThick:
		return brcThick;

	case ibrclShadow:
		return (brcSingle | brcfShadow);

	case uNinchList:
		return tmvalNinch;
		}
}


/* F  A P P L Y  H C A B  T O  T A P */
/* Applies the properties in the cab to the current selection */
/* Note:  When applying table properties to a row, you only need to
	apply them to the Ttp, however caUndo must cover whole rows of
	the table.
*/
/* %%Function:FApplyHcabToTap %%Owner:rosiep */
BOOL FApplyHcabToTap(hcab)
void **hcab;
{
	CABFORMATTABLE *pcab;
	int rgdxaWidth[itcMax];
	struct CA caApply, caInval, caUndo;
	struct TAP tapSave;
	int itc, itcFirst, itcLim;
	char grpprl[12];
	int cch = 0;
	int dxaGapHalf;
	int dyaRowHeight;
	BOOL fInval = fFalse;
	BOOL fSetColWidth = fFalse;
	int rgbrc[5];

	StartLongOp();
	pcab = *hcab;

	Assert(vitcFirst != -1);
	Assert(vitcLim != -1);

	if (pcab->fWholeTable)
		{
		/* Apply to the whole table */
		itcFirst = 0;
		itcLim = itcMax;
		PcaTable(&caApply, selCur.doc, selCur.cpFirst);
		}
	else
		{
		itcFirst = vitcFirst;
		itcLim = vitcLim;
		if (selCur.fTable)
			caApply = selCur.ca;
		else
			{
			CpFirstTap(selCur.doc, selCur.cpFirst);
			caApply = caTap;
			}
		}

	caInval = caApply;
	/* Get the current tap to compare with the arguments */
	if (!FGetTapState(&caApply, fTrue))
		{
		/* All Ninch */
		SetWords(&tapSave, tmvalNinch, cwTAP);
		SetWords(rgdxaWidth, uNinch, itcMax);
		tapSave.itcMac = itcMax;
		}
	else
		{
		tapSave = vtapFetch;
		RgWidthFromCenter(rgdxaWidth, vtapFetch.rgdxaCenter,
				vtapFetch.rgtc, vtapFetch.itcMac);
		}

	caUndo = caApply;
	/* expand caUndo to encompass next row because of possible change
		to its above border */
	if (caUndo.cpLim < CpMacDoc(caUndo.doc) && FInTableDocCp(caUndo.doc, caUndo.cpLim))
		{
		CpFirstTap(caUndo.doc, caUndo.cpLim);
		caUndo.cpLim = caTap.cpLim;
		}

	if (!FSetUndoB1(ucmFormatTable, uccFormat, &caUndo))
		{
		EndLongOp(fFalse);
		return fFalse;
		}

	/* With the values that were changed, build up a grpprl */
	pcab = *hcab;
	if (pcab->jc != uNinchRadio && tapSave.jc != pcab->jc)
		{
		grpprl[cch++] = sprmTJc;
		bltb(&pcab->jc, &grpprl[cch], sizeof(int));
		cch += sizeof(int);
		}

	if (pcab->dxaLeft != wNinch && (tapSave.rgdxaCenter[0] == tmvalNinch ||
			tapSave.dxaGapHalf == tmvalNinch ||
			!FEqDza(tapSave.rgdxaCenter[0] + tapSave.dxaGapHalf, pcab->dxaLeft)))
		{
		grpprl[cch++] = sprmTDxaLeft;
		bltb(&pcab->dxaLeft, &grpprl[cch], sizeof(int));
		cch += sizeof(int);
		}

	if (pcab->dxaWidth != uNinch && (itcFirst != itcLim - 1 ||
			rgdxaWidth[itcFirst] == uNinch ||
			!FEqDza(rgdxaWidth[itcFirst], pcab->dxaWidth)))
		{
		CacheSect(caApply.doc, caApply.cpFirst);     /* HM */
		pcab = *hcab;
		ApplySprmTDxaCol(itcFirst, itcLim, pcab->dxaWidth,
				vsepFetch.dxaColumnWidth, &caApply); /* HM */
		pcab = *hcab;
		fInval = fTrue;
		fSetColWidth = fTrue;
		for (itc = vitcFirst ; itc < vitcLim ; itc++)
			vrgdxaCol[itc] = pcab->dxaWidth;
		}

	rgbrc[ibrcTop] = BrcFromIbrcl(pcab->ibrclTop);
	rgbrc[ibrcLeft] = BrcFromIbrcl(pcab->ibrclLeft);
	rgbrc[ibrcBottom] = BrcFromIbrcl(pcab->ibrclBottom);
	rgbrc[ibrcRight] = BrcFromIbrcl(pcab->ibrclRight);
	rgbrc[ibrcInside] = BrcFromIbrcl(pcab->ibrclInside);

	/* Apply the border formatting */
	fInval |= FApplyRgbrcTap(rgbrc, pcab->fWholeTable, &caApply);

	pcab = *hcab;

	if (pcab->dxaGap != uNinch && (tapSave.dxaGapHalf == tmvalNinch ||
			!FEqDza(2*tapSave.dxaGapHalf, pcab->dxaGap)))
		{
		grpprl[cch++] = sprmTDxaGapHalf;
		dxaGapHalf = pcab->dxaGap / 2;
		bltb(&dxaGapHalf, &grpprl[cch], sizeof(int));
		cch += sizeof(int);
		if (!fSetColWidth || vitcFirst != 0)
			{
			FixUpFirstColumn(&caApply); /* HM */
			pcab = *hcab;
			}
		}

	dyaRowHeight = pcab->dyaRowHeight;
	if (dyaRowHeight != wNinch && abs(dyaRowHeight) < czaPoint)
		dyaRowHeight = 0;

	if (dyaRowHeight != wNinch && (tapSave.dyaRowHeight == tmvalNinch ||
			!FEqDza(tapSave.dyaRowHeight, dyaRowHeight)))
		{
		grpprl[cch++] = sprmTDyaRowHeight;
		bltb(&dyaRowHeight, &grpprl[cch], sizeof(int));
		cch += sizeof(int);
		}


	/* Go ahead and apply the changes, if any... */
	if (cch != 0)
		{
		ApplyGrpprlCa(&grpprl, cch, &caApply);
		fInval = fTrue;
		}

	if (fInval)
		{
		/* Invalidate selection */
		InvalCp(&caInval);
		InvalTableCp(caInval.doc, caInval.cpFirst, DcpCa(&caInval));
		PdodDoc(caInval.doc)->fDirty = fTrue;
		}

	SetUndoAfter(&caUndo);
	EndLongOp(fFalse);
	return fTrue;
}


/* F  E Q  D Z A */

/* Compares two dxa's or two dya's.  This bogus-looking compare is to avoid
	the round-off error that occurs when a dza is converted through the parse
	proc into a string and then back again.  To compare two dza's we first
	convert them both to strings, and if the strings are equal, the dza's
	must have been.  dza1 is the dza passed in to SDM in the cab.  dza2 is
	what we get back from SDM after the dialog is dismissed.  The parameters
	must not be switched or bogus results will occur.

FUTURE:  It would be REAL NICE if SDM provided some functionality for
	letting us know if an edit control is dirty, so we didn't have to do
	this bogosity.  Dirtiness should be based on whether the user typed
	anything in the edit control, not whether it has actually changed.
	This affects ALL our dialogs, not just Format Table.  FEqDza is quite
	inefficient, but there's no other way to do it currently.
*/
/* %%Function:FEqDza %%Owner:rosiep */
BOOL FEqDza(dza1, dza2)
int dza1, dza2;
{
	CHAR sz[cchMaxSz];
	CHAR *pch = sz;
	int za;
	BOOL fOverflow;

	/* they might really be equal!! */
	if (dza1 == dza2)
		return fTrue;

	/* changes pch */
	CchExpZa(&pch, dza1, vpref.ut, cchMaxSz, fTrue);

	/* These asserts should be OK because we've passed through the parse proc */
	AssertDo(FZaFromSs(&za, sz, CchSz(sz) - 1, vpref.ut, &fOverflow));
	Assert(!fOverflow);

	return (dza2 == za);
}


/* A D D  C O L S  T O  D L G */
/* Adds the column numbers to the dialog for formatting a table,
	and enters their column width. */
/* %%Function:AddColsToDlg %%Owner:rosiep */
AddColsToDlg(itcFirst, itcLim)
int itcFirst, itcLim;
{
	char stColumnNums[80];
	int dxaCol, itc;
	int rgw[2];

	rgw[0] = itcFirst + 1;

	/** use the plural or singular form of the message? **/
	if (itcLim - itcFirst > 1)
		{
		rgw[1] = itcLim;
		BuildStMstRgw(mstWidthOfColumns, rgw, stColumnNums, 80, NULL);
		}
	else
		BuildStMstRgw(mstWidthOfColumn, rgw, stColumnNums, 80, NULL);

	stColumnNums[stColumnNums[0]+1] = '\0';   /* zero terminate the st */

	/* Now that we have the string, place it in the dialog */
	SetTmcText(tmcWidthText, &stColumnNums[1]);

	/* Now figure out the column width */
	dxaCol = vrgdxaCol[itcFirst];
	for (itc = itcFirst + 1 ; itc < itcLim ; itc++)
		if (dxaCol != vrgdxaCol[itc])
			{
			dxaCol = uNinch;
			break;
			}

	SetTmcVal(tmcColumnWidth, dxaCol);
}


/* C M D  T A B L E  T O  T E X T */
/* Command function to for Insert TableToText command. */
/* %%Function:CmdTableToText %%Owner:rosiep */
CMD CmdTableToText(pcmb)
CMB * pcmb;
{
	CABTABLETOTEXT * pcab;
	struct CA caUndo;

	CacheSect(selCur.doc, selCur.cpFirst);
	vdxaParaWidth = vsepFetch.dxaColumnWidth;
	vrf.fForcePara = fFalse;

	if (pcmb->fDefaults)
		{
		if (!FWholeRowsPsel(&selCur))
			{
			Assert(fFalse); /* We should not have gotten here */
			Beep();
			return cmdError;
			}

		vfselType = fselTable;
		pcab = *pcmb->hcab;
		pcab->tblfmt = 0;
		}

	if (pcmb->fDialog)
		{
		CHAR dlt [sizeof (dltTableToText)];

		BltDlt(dltTableToText, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;
			}
		}

	if (pcmb->fAction)
		{
		if (!FSetUndoBefore(bcmTableToText, uccPaste))
			return cmdCancelled;

		StartLongOp();

		pcab = *pcmb->hcab;
		if (!FTableToTextSelCur(pcab->tblfmt))
			{
			SetUndoNil();
#ifdef NOTUSED
			/* cmdNoMemory and cmdError are the same value. Old code: */
			if (vmerr.fMemFail)
				return cmdNoMemory;
			else
				return cmdError;
#endif /* NOTUSED */
			return cmdError;
			}
		caUndo = selCur.ca;
		Select(&selCur, selCur.cpFirst, selCur.cpLim);
		PushInsSelCur();
		SetUndoAfter(&caUndo);

		EndLongOp(fFalse);
		}

	return cmdOK;
}



/* F  B A D  T M C  C H E C K  */
/* Check value of a tmc for out of range error and report it. */
/* Returns fTrue if error. */
/* %%Function:FBadTmcCheck %%Owner:rosiep */
BOOL FBadTmcCheck(tmc, wLow, wHigh, fZa, fBlankOK)
int tmc, wLow, wHigh;
BOOL fZa, fBlankOK;
{
	char sz [ichMaxBufDlg];
	int w;

	/* should not be called unless we are in a dialog */
	AssertH(HcabDlgCur());

	if (fBlankOK)
		{
		GetTmcText(tmc, sz, ichMaxBufDlg);
		if (CchStripString(sz, CchSz(sz) - 1) == 0)
			return fFalse;
		}

	w = ValGetTmc(tmc);
	if (w == wError)
		return fTrue;

	/* Note: wLow and wHigh are INCLUSIVE on purpose */
	if (w > wHigh || w < wLow)
		{
		RangeError(wLow, wHigh, fZa, vpref.ut);
		return fTrue;
		}

	return fFalse;
}

/* F  W H O L E  T A B L E */
/* Returns true if the selection is exactly a whole table. */
/* %%Function:FWholeTable %%Owner:rosiep */
FWholeTable(psel)
struct SEL *psel;
{

	if (psel->sk != skRows)
		return fFalse;

	return (FPcaWholeTable(&psel->ca));
}

/* R G  W I D T H  F R O M  C E N T E R */
/* Takes an array of centers (midpoints between cells) and returns an
	array of column widths (for a whole row).  Even though merged cells have
	zero width, this makes them seem to have the same width as their
	corresponding fFirstMerged cell, so the dialog doesn't get ninched with
	a merged cell selected. */
/* %%Function:RgWidthFromCenter %%Owner:rosiep */
RgWidthFromCenter(pdxaWidth, pdxaCenter, ptc, itcMac)
int *pdxaWidth, *pdxaCenter, itcMac;
struct TC *ptc;
{
	int dxaOld, *pdxaCenterMac;
#ifdef DEBUG
	int *pdxaWidthStart = pdxaWidth;
#endif

	dxaOld = *pdxaCenter;
	pdxaCenterMac = ++pdxaCenter + itcMac;
	for ( ; pdxaCenter < pdxaCenterMac;
			pdxaWidth++, pdxaCenter++, ptc++)
		{
		if (ptc->fMerged)
			{
			Assert (pdxaWidth > pdxaWidthStart);
			*pdxaWidth = *(pdxaWidth - 1);
			}
		else
			*pdxaWidth = *pdxaCenter - dxaOld;
		dxaOld = *pdxaCenter;
		}

}

/* F  G E T  T A P  S T A T E */
/* Returns the state of the tap in various globals, for use in error
checking of the dialog values later.  Returns fFalse if rows are
not identical (Ninch), fTrue otherwise.  If !fAll look only at the
first 20 rows.

	Globals filled:
	vdxaMinCell	  min cell width in selection
	vdxaGapHalfMax	  max GapHalf in selection
	vdxaTLeftMin      min left edge of all rows in sel
	vdxaTRightMax	  max right edge of all rows in sel
*/
/* %%Function:FGetTapState %%Owner:rosiep */
BOOL FGetTapState(pca, fAll)
struct CA *pca;
BOOL fAll;
{
	struct TAP tapT;
	CP cpCur;
	int fSame = fTrue;
	int cRows = 1;
	int dxaT;

	CpFirstTap(pca->doc, pca->cpFirst);
	tapT = vtapFetch;
	cpCur = caTap.cpLim;
	vdxaMinCell = DxaMinWidthFromTap(&vtapFetch);
	vdxaGapHalfMax = vtapFetch.dxaGapHalf;
	vdxaTLeftMin = vtapFetch.rgdxaCenter[0] + vtapFetch.dxaAdjust;
	vdxaTRightMax = vtapFetch.rgdxaCenter[vtapFetch.itcMac] + vtapFetch.dxaAdjust;

	while (cpCur < pca->cpLim)
		{
		/* See if we have encountered too many rows */
		if (!fAll && ++cRows > 20)
			{
			fSame = fFalse;
			break;
			}
		CpFirstTap(pca->doc, cpCur);

		/* Check standard characteristics */
		if ((vtapFetch.jc != tapT.jc) ||
				(vtapFetch.dxaGapHalf != tapT.dxaGapHalf) ||
				(vtapFetch.rgdxaCenter[0] != tapT.rgdxaCenter[0]) ||
				(vtapFetch.itcMac != tapT.itcMac) ||
				(vtapFetch.dyaRowHeight != tapT.dyaRowHeight) ||
				FNeRgw(&vtapFetch.rgdxaCenter, &tapT.rgdxaCenter, vtapFetch.itcMac+1))
			{
			fSame = fFalse;
			}
#ifdef DEBUG
		else
			{
			int itc;

			/* rgdxaCenter matching above implies rgtc matches
			(except possibly for borders which we don't care
			about here; GetRowBorders does that. */

			Assert (vtapFetch.itcMac == tapT.itcMac);
			for (itc = 0; itc < vtapFetch.itcMac; itc++)
				Assert(vtapFetch.rgtc[itc].grpf == tapT.rgtc[itc].grpf);
			}
#endif /* DEBUG */

		/* Do this all subsequent times through the loop, even if
			standard characteristics match for this row */
		if (!fSame)
			{
			if ((dxaT = vtapFetch.rgdxaCenter[0] + vtapFetch.dxaAdjust) < vdxaTLeftMin)
				vdxaTLeftMin = dxaT;
			if ((dxaT = vtapFetch.rgdxaCenter[vtapFetch.itcMac] + vtapFetch.dxaAdjust) > vdxaTRightMax)
				vdxaTRightMax = dxaT;
			if (vtapFetch.dxaGapHalf > vdxaGapHalfMax)
				vdxaGapHalfMax = vtapFetch.dxaGapHalf;
			if ((dxaT = DxaMinWidthFromTap(&vtapFetch)) < vdxaMinCell)
				vdxaMinCell = dxaT;
			}


		cpCur = caTap.cpLim;
		}

	/* Properties are not identical */
	if (!fSame)
		{
		if (cRows > 20)
			{
			vdxaTRightMax = vdxaTLeftMin = -1;
			vdxaGapHalfMax = vdxaMinCell = -1;
			}
		return fFalse;
		}

	return fTrue;

}


/* G E T  C E L L  B O R D E R S */
/* Returns an array with the cell borders for the selection.  Ninch
	wherever they don't match throughout the selection. */
/* %%Function:GetCellBorders %%Owner:rosiep */
GetCellBorders(rgbrc, psel)
int rgbrc[];
struct SEL *psel;
{
	int brcTop, brcLeft, brcRight, brcInside;
	int rgbrcT[cbrcCellBorders];
	int itcFirst, itcLim, itcMac;
	CP cpSelLim;
	int doc = psel->doc;

	cpSelLim = psel->cpLim + (psel->fIns ? 1 : 0);

	/* Get itc's from the current selection */
	ItcsFromPsel(&itcFirst, &itcMac, psel);

	/* scan the first row */
	CpFirstTap(doc, psel->cpFirst);
	itcLim = min(vtapFetch.itcMac,itcMac);

	GetRowBorders(rgbrcT, &vtapFetch, itcFirst, itcLim, fTrue /*fStrict*/);

	/* if only one row (or less) selected, we're done */
	if (caTap.cpLim >= cpSelLim)
		{
		/* we don't want to show ninch Inside borders just
			because the user has only one cell selected. */
		if (itcMac == itcFirst + 1)
			rgbrcT[ibrcInside] = brcNone;
		blt(rgbrcT, rgbrc, cbrcCellBorders);
		return;
		}

	brcTop = rgbrcT[ibrcTop];
	brcLeft = rgbrcT[ibrcLeft];
	brcRight = rgbrcT[ibrcRight];
	if (itcFirst == itcLim - 1)
		brcInside = rgbrcT[ibrcBottom];
	else
		{
		if (rgbrcT[ibrcInside] != rgbrcT[ibrcBottom] && rgbrcT[ibrcBottom] != brcNone)
			brcInside = tmvalNinch;
		else
			brcInside = rgbrcT[ibrcInside];
		}

	/* scan any middle rows of the selection */
	CpFirstTap(doc, caTap.cpLim);
	for ( ; caTap.cpLim < cpSelLim; CpFirstTap(doc, caTap.cpLim))
		{
		itcLim = min(vtapFetch.itcMac,itcMac);
		GetRowBorders(rgbrcT, &vtapFetch, itcFirst, itcLim, fTrue /*fStrict*/);
		if ((itcFirst != itcLim - 1 && rgbrcT[ibrcInside] != brcInside)
				|| rgbrcT[ibrcTop] != brcInside
				|| (rgbrcT[ibrcBottom] != brcInside && rgbrcT[ibrcBottom] != brcNone))
			brcInside = tmvalNinch;
		if (rgbrcT[ibrcLeft] != brcLeft)
			brcLeft = tmvalNinch;
		if (rgbrcT[ibrcRight] != brcRight)
			brcRight = tmvalNinch;
		}

	/* scan the last row of the selection */
	Assert(caTap.cpFirst < cpSelLim);
	itcLim = min(vtapFetch.itcMac,itcMac);
	GetRowBorders(rgbrcT, &vtapFetch, itcFirst, itcLim, fTrue /*fStrict*/);
	if ((itcFirst != itcLim - 1 && rgbrcT[ibrcInside] != brcInside)
			|| rgbrcT[ibrcTop] != brcInside)
		brcInside = tmvalNinch;
	if (rgbrcT[ibrcLeft] != brcLeft)
		brcLeft = tmvalNinch;
	if (rgbrcT[ibrcRight] != brcRight)
		brcRight = tmvalNinch;

	Break3();
	/* put the values into the array */
	rgbrc[ibrcBottom] = rgbrcT[ibrcBottom];
	rgbrc[ibrcTop] = brcTop;
	rgbrc[ibrcLeft] = brcLeft;
	rgbrc[ibrcRight] = brcRight;
	rgbrc[ibrcInside] = brcInside;
}




