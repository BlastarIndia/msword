/* T A B L E C M D . C */


#ifdef MAC
#define WINDOWS
#define DIALOGS
#define CONTROLS
#include "toolbox.h"
#endif /* MAC */

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
#include "error.h"

#ifdef WIN
#include "keys.h"
/* WARNING!!! This is stolen from sdm.h */
#define	uNinch		(0xffff)	// Unsigned. 
#define	wNinch		(-32767)	// Ints. 
#else
#include "dlg.h"
#endif


extern struct SAB	vsab;
extern struct SEL	selCur;
extern struct CA	caTap;
extern struct CA	caTable;
extern struct CA	caPara;
extern CP		vmpitccp[];
extern struct TAP	vtapFetch;
extern struct CHP	vchpFetch;
extern struct PAP	vpapFetch;
extern struct SEP	vsepFetch;
extern char		rgchEop[];
extern char		rgchTable[];
extern struct MERR	vmerr;
extern struct SCI       vsci;

#ifdef WIN
extern struct FTI       vfti;
#endif

extern CP 	CpTableLim();


/* G l o b a l s */
int	vitcFirst;
int	vitcLim;
int	vitcMac;
int	vdxaGapHalfMax;
int	vdxaTRightMax, vdxaTLeftMin;
int	vdxaMinCell;


/* C M D  I N S  T A B L E  R O W S */
/* Insert rows into a table.  Used by Edit Table. */
/* Requires the current selection to be in a table */
/* %%Function:CmdInsTableRows %%Owner:davidmck %%Reviewed:6/27/89 */
CmdInsTableRows()
{
	int cRows, cColumns, cRowsIns;
	struct CA caT;
	struct SELS selsIns;

	Assert(selCur.fTable || selCur.fWithinCell);

	blt(&selCur, &selsIns, cwSELS);
	ExtendPselsRows(&selsIns);     /* Extend the selection to cover rows */

	CpFirstTap(selsIns.doc, selsIns.cpFirst);
	cColumns = vtapFetch.itcMac;

	/* Number of rows to insert is based on number in selection */
	cRows = CountRowsPca(&selsIns.ca);

	if (!FSetUndoB1(ucmInsRows, uccPaste, PcaPoint(&caT, selsIns.doc, selsIns.cpFirst)))
		return(cmdCancelled);

	/* Call subroutine to actually insert the rows.  Will return how
		many were successfully insertted */
	CpFirstTap(selsIns.doc, selsIns.cpFirst);
	cRowsIns = CInsertNewRows(selsIns.doc, selsIns.cpFirst, cRows, cColumns, &vtapFetch, fFalse);
	if (cRowsIns)
		SelectRow(&selCur, selsIns.cpFirst, selsIns.cpFirst + cRowsIns * (cColumns + 1) * ccpEop);

	SetUndoAfter(&selCur.ca);
	Mac( SetAgainUcm(ucmInsRows, ascEdit) );
	DirtyOutline(selCur.doc);

	return (cRowsIns != cRows) ? cmdError : cmdOK;

}


/* C M D  I N S  T A B L E  C O L U M N S */
/* Insert columns into a table */
/* If the fExpand flag is set, we will expand the selection to
	cover the entire table, i.e. insert columns as opposed to
	inserting cells
*/
/* %%Function:CmdInsTableColumns %%Owner:davidmck %%Reviewed:6/27/89 */
CmdInsTableColumns(fExpand)
int fExpand;
{
	int itcFirst, itcLim, itcMac;
	CP cpFirst, cpLim;
	BOOL fTableSel;
	struct CA caUndo;
	int ucm;

	Assert(selCur.fTable || selCur.fWithinCell);

	/* If fExpand, we want to insert columns into the whole table.
		Therefore we will extend the selection to cover the
		entire table */
	if (fExpand)
		ExtendSelCurColumns();

	TurnOffSel(&selCur);

	if (!(fTableSel = selCur.fTable))
		{
		CpFirstTap(selCur.doc, selCur.cpFirst);
		caUndo = caTap;
		}
	else
		caUndo = selCur.ca;

	if (!FSetUndoB1(ucm = (fExpand ? ucmInsColumns : ucmInsCellsHoriz), uccPaste, &caUndo))
		return cmdCancelled;

	ItcsFromSelCur(&itcFirst, &itcLim);
	CpFirstTap(selCur.doc, selCur.cpFirst);
	cpLim = caUndo.cpLim;

	if (!FInsertTableColumns(selCur.doc,cpFirst = caTap.cpFirst,&cpLim,itcFirst,itcLim - itcFirst,itcNil))
		return cmdError;

	InvalTableCp(selCur.doc, cpFirst, cpLim - cpFirst );

	/* Re-establish the selection */
	if (!fTableSel)
		{
		CpFirstTap(selCur.doc, cpFirst);
		cpFirst = caTap.cpFirst;
		cpLim = caTap.cpLim;
		}
	SelectColumn(&selCur, cpFirst, cpLim, itcFirst, itcLim);
	SetUndoAfter(&selCur.ca);
	Mac( SetAgainUcm(ucm, ascEdit) );

	return cmdOK;

}


/* F  I N S E R T  T A B L E  C O L U M N S */
/* The business end of CmdInsTableColumns */
/* if itcMac = -1, do checking on the fly rather than at the beginning */
/* %%Function:FInsertTableColumns %%Owner:davidmck %%Reviewed:6/27/89 */
FInsertTableColumns(doc, cpFirst, pcpLim, itcFirst, dtc, itcMac)
int doc;
CP cpFirst, *pcpLim;
int itcFirst, dtc, itcMac;
{
	int ctc, ibrc, cch;
	int itcStart;
	int dxaCol;
	int docTemp;
	int fComplete = fFalse;
	CP dcp, cpCur, cpLim;
	CP cpStart;
	struct CA caRow, caT;
	struct CA caCopy;
	char prl[5], prlBorders[25];
	int *prgbrc;
	int itcT;
	struct CHP chp;
	struct PAP pap;
	struct DOD *pdodTemp;
	int dxaTableWidth;
#ifdef MAC
	strcut DOD *pdod;
#endif

	Assert(FInTableDocCp(doc, cpFirst));

#ifdef MAC
	if (itcMac != itcNil && itcMac + dtc >= itcMax)
		{
		ErrorEid(eidMaxCol,"FInsertTableColumns");
		return fFalse;
		}
#else 
	Assert(itcMac == itcNil);
#endif /* MAC */

	/* Get first and last cps of the first and last rows */
	cpStart = CpFirstTap(doc, cpFirst);
	CpFirstTap(doc, *pcpLim - 1 /* not ccpEop */);
	Assert(caTap.cpLim >= *pcpLim);

	PcaSet(&caCopy, doc, cpStart, caTap.cpLim);
	if ((docTemp = DocCreate(fnNil)) == docNil)
		return fFalse;

/* FUTURE...MacWord will want to do this too when you have rev marking */
#ifdef WIN
	PdodDoc(docTemp)->dop.fRevMarking = PdodMother(doc)->dop.fRevMarking;
#endif

	SetWholeDoc(docTemp, &caCopy);
	if (vmerr.fMemFail)
		goto LAbort;

	(pdodTemp = PdodDoc(docTemp))->fMotherStsh = fTrue;
	pdodTemp->doc = doc;
#ifdef MAC
	/* This is needed on the Mac because the Mac style code currently
		only backs up one document for style sheets */
	pdod = PdodDoc(doc);
	if (pdod->fShort || pdod->fMotherStsh)
		pdodTemp->doc = pdod->doc;
#endif

	prl[0] = sprmTInsert; /* more filled in later */

	cpCur = cp0;
	cpLim = CpMacDocEdit(docTemp);

	/* Deal with each row individually */
	while (cpCur < cpLim)
		{
		Assert(CpFirstTap(docTemp, cpCur) == cpCur);
		CpFirstTap(docTemp, cpCur);
		cpCur = caTap.cpLim;
		caRow = caTap;

		/* We do not want to insert cells in the middle of a merged block */
		itcStart = ItcNonMerged(itcFirst, &vtapFetch);

		ctc = dtc;
		if (vtapFetch.itcMac <= itcStart)
			/* Need to add extra columns or at least move caT.cpFirst */
			{
			struct CA caCell;
			ctc += itcStart - vtapFetch.itcMac;
			caT.cpFirst = caTap.cpLim - ccpEop;

			/* Take the properties (chp and pap) from the last cell in the
			/* row (not the ttp since it has no properties) */
			FetchCpAndParaCa(PcaColumnItc(&caCell, &caRow, vtapFetch.itcMac - 1), fcmProps);
			}
		else
			{
			/* Take the properties (chp and pap) from the first selected cell */
			PcaColumnItc(&caT, &caRow, itcStart);
			FetchCpAndPara(docTemp, caT.cpFirst, fcmProps);
			}
		pap = vpapFetch;
		chp = vchpFetch;
		CleanTableChp(doc, &chp);

		/* Determine width of new columns */
		itcT = min(itcFirst+1, vtapFetch.itcMac);
		Assert(itcT > 0);
		dxaCol = vtapFetch.rgdxaCenter[itcT] -
				vtapFetch.rgdxaCenter[itcT - 1];

		/* Check that new width is at least as big as a gap */
		/* This is possible if we are insertting next to a merged cell */
		if (dxaCol <= vtapFetch.dxaGapHalf*2)
			dxaCol = max(vtapFetch.dxaGapHalf * 2, dxaInch);

		/* Assure that the number of columns is less than itcMax */
		if (vtapFetch.itcMac + ctc >= itcMax)
			{
			Assert(itcMac == -1);
			ErrorEid(eidMaxCol,"FInsertTableColumns");
			goto LAbort;
			}

		if (xaRightMaxSci/ctc < dxaCol)
			{
			ErrorEid(eidMaxWidth,"FInsertTableColumns");
			return fFalse;
			}

		/* dxTableBorderMax is to allow space for the ttp */
		dxaTableWidth = vtapFetch.rgdxaCenter[vtapFetch.itcMac] -
				min(0, vtapFetch.rgdxaCenter[0]) + dxTableBorderMax;

		if (xaRightMaxSci - dxaTableWidth < ((long) ctc * (long) dxaCol))
			{
			ErrorEid(eidMaxWidth,"FInsertTableColumns");
			goto LAbort;
			}

		/* Set up prl to apply borders */
		cch = 0;
		if (itcStart < vtapFetch.itcMac)
			{
			prgbrc = &vtapFetch.rgtc[itcStart].rgbrc;
			for (ibrc = 0 ; ibrc < 4 ; ibrc++)
				if (*(prgbrc + ibrc) != brcNone)
					{
					prlBorders[cch++] = sprmTSetBrc;
					prlBorders[cch++] = (char) itcStart;
					prlBorders[cch++] = (char) itcStart+dtc;
					prlBorders[cch++] = 1<<ibrc;
					MovePwToPch((prgbrc + ibrc), &prlBorders[cch]);
					cch += sizeof(int);
					}
			}

		/* Create prl for inserting cells */
		prl[1] = (char) itcStart;
		prl[2] = (char) dtc;
		MovePwToPch(&dxaCol, &prl[3]);

		ApplyGrpprlCa(&prl, 5, &caRow);
		if (vmerr.fMemFail || vmerr.fFmtFailed)
			goto LAbort;

		if (cch != 0)
			{
			ApplyGrpprlCa(&prlBorders, cch, &caRow);
			if (vmerr.fMemFail || vmerr.fFmtFailed)
				goto LAbort;
			}

		/* Insert cell marks */
		if (!FInsTableCCells(docTemp, caT.cpFirst, ctc, &chp, &pap))
			goto LAbort;
		cpCur += (dcp = ctc * ccpEop);
		cpLim += dcp;
		}

	/* Replace old table with new table */
	if (!FReplaceCps(&caCopy, PcaSet(&caT, docTemp, cp0, cpLim)))
		goto LAbort;
	*pcpLim = cpLim + cpFirst;
	fComplete = fTrue;
LAbort:
	DisposeDoc(docTemp);
	DirtyOutline(doc);
	return fComplete;
}


/* C M D  I N S  T A B L E  C E L L S */
/* Insert single cells into a table */
/* %%Function:CmdInsTableCells %%Owner:davidmck %%Reviewed:6/27/89 */
CmdInsTableCells(fShiftVert)
int fShiftVert;
{
	struct CA caDest, caSrc, caUndo, caT;
	int docTemp;
	int docCopy;
	struct SELS selsDel;
	int cRows;
	CP cpDest;
	struct DOD *pdodTemp;
	int cmd = cmdError;
#ifdef MAC
	struct DOD *pdod;
#endif

	if (!fShiftVert)      /* Horizontal shift */
		return CmdInsTableColumns(fFalse);

	/* Vertical shift */
	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdError;
/* FUTURE...MacWord will want to do this too when you have rev marking */
#ifdef WIN
	PdodDoc(docTemp)->dop.fRevMarking = PdodMother(selCur.doc)->dop.fRevMarking;
#endif
	if ((docCopy = DocCreate(fnNil)) == docNil)
		{
		DisposeDoc(docTemp);
		return cmdError;
		}
	TurnOffSel(&selCur);
	(pdodTemp = PdodDoc(docTemp))->fMotherStsh = fTrue;
	pdodTemp->doc = selCur.doc;
#ifdef MAC
	pdod = PdodDoc(selCur.doc);
	if (pdod->fShort || pdod->fMotherStsh)
		pdodTemp->doc = pdod->doc;
#endif
	/* This must be done here because selCur.ca changes */
	cRows = CountRowsPca(&selCur.ca);
	/* We must remember all of the table below us */
	PcaSet(&caUndo, selCur.doc, CpFirstTap(selCur.doc, selCur.cpFirst), CpTableLim(selCur.doc, selCur.cpLim));

	if (!FSetUndoB1(ucmInsCellsVert, uccPaste, &caUndo))
		{
		cmd = cmdCancelled;
		goto LAbort;
		}

	SetWholeDoc(docCopy, &caUndo);
	if (vmerr.fMemFail)
		goto LAbort;
	PcaSetWholeDoc(&selsDel.ca, docCopy);
	selsDel.sk = skColumn;
	ItcsFromSelCur(&selsDel.itcFirst, &selsDel.itcLim);
	selsDel.itcLim = ItcMinItcItcMacPca(selsDel.itcLim, &selCur.ca);
	/* Copy the columns and save them in docTemp */
	if (CmdCopyTable(&selsDel, docTemp) != cmdOK)
		goto LAbort;
	if (!selCur.fTable)
		{
		CpFirstTap(selCur.doc, selCur.cpLim);
		cpDest = caTap.cpLim - caUndo.cpFirst;
		}
	else
		cpDest = selCur.cpLim - caUndo.cpFirst;

	PcaSet(&caDest, docCopy, cpDest, selsDel.cpLim);
	PcaSetWholeDoc(&caSrc, docTemp);
	/* Paste the columns back into the table */
	if (CmdTableToTablePaste(&caSrc, 0, &caDest, selsDel.itcFirst,
			selsDel.itcLim - selsDel.itcFirst, fFalse, fFalse) != cmdOK)
		goto LAbort;
	selsDel.cpLim = cpDest;
	if (CmdCutTable(&selsDel, docNil, fFalse) != cmdOK)
		goto LAbort;
	Assert(selsDel.cpFirst == cp0);
	if (!FReplaceCps(&caUndo, PcaSetWholeDoc(&caT, docCopy)))
		goto LAbort;
	caUndo.cpLim = CpTableLim(selCur.doc, selsDel.cpFirst + caUndo.cpFirst);
	cpDest = caUndo.cpFirst;

	for (  ; cRows > 0 ; cRows--)
		{
		CpFirstTap(selCur.doc, cpDest);
		cpDest = caTap.cpLim;
		}
	SelectColumn(&selCur, caUndo.cpFirst, cpDest, selsDel.itcFirst, selsDel.itcLim);
	SetUndoAfter(&caUndo);
	Mac( SetAgainUcm(ucmInsCellsVert, ascEdit) );
	DirtyOutline(selCur.doc);
	cmd = cmdOK;

LAbort:
	DisposeDoc(docTemp);
	DisposeDoc(docCopy);
	return cmd;

}


/* C M D  D E L  T A B L E  R O W S */
/* Delete rows in a table */
/* %%Function:CmdDelTableRows %%Owner:davidmck %%Reviewed:6/27/89 */
CmdDelTableRows()
{
	struct SELS selsDel;

	blt(&selCur, &selsDel, cwSELS);

	ExtendPselsRows(&selsDel);     /* Extend the selection to cover rows */

	TurnOffSel(&selCur);

	if (!FSetUndoB1(ucmDeleteRows, uccPaste, &selsDel.ca))
		return(cmdCancelled);

	FDeleteRM(&selsDel.ca);

	/* Do this to reset the flags in selCur */
	SelectIns(&selCur, selCur.cpFirst);

	if (vmerr.fMemFail)
		return cmdNoMemory;

#ifdef MAC
	Assert(DcpCa(&selsDel.ca) == 0);
#endif
	SetUndoAfter(&selsDel.ca);
	Mac( SetAgainUcm(ucmDeleteRows, ascEdit) );
	DirtyOutline(selCur.doc);

	return cmdOK;
}


/* C M D  D E L  T A B L E  C O L U M N S */
/* Delete columns in a table */
/* fExpand indicated whether we are doing columns or cells */
/* %%Function:CmdDelTableColumns %%Owner:davidmck %%Reviewed:6/27/89 */
CmdDelTableColumns(fExpand)
int fExpand;
{
	int itcFirst, itcLim, itcMac, itcStart, itcEnd, fApplySprm;
	CP cpStart, dcpAdjust, dcpT;
	struct CA caT, caRow, caUndo;
	struct CA caCur, caDel;
	char grpprl[3];
	int ucm;
	int docTemp;
	int cmd = cmdError;

	if (fExpand)
		/* Expand to the whole table */
		ExtendSelCurColumns();

	TurnOffSel(&selCur);

	/* If we are deleting every column in the selected rows, it
		is just a row deletion */
	if (FWholeRowsPsel(&selCur))
		return CmdDelTableRows();

	if (selCur.fTable)
		caUndo = selCur.ca;
	else
		{
		CpFirstTap(selCur.doc, selCur.cpFirst);
		caUndo = caTap;
		}

	ItcsFromSelCur(&itcFirst, &itcLim);
	Assert(itcFirst < itcLim);
	itcLim = ItcMinItcItcMacPca(itcLim, &selCur.ca);

	if (itcFirst >= itcLim)
		{
		ErrorEid(eidCantDelEOR,"CmdDelTableColumns");
		return cmdError;
		}

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdError;

/* FUTURE...MacWord will want to do this too when you have rev marking */
#ifdef WIN
	PdodDoc(docTemp)->dop.fRevMarking = PdodMother(caUndo.doc)->dop.fRevMarking;
#endif

	SetWholeDoc(docTemp, &caUndo);
	if (vmerr.fMemFail)
		goto LAbort;

	cpStart = caUndo.cpFirst;
	PcaSetWholeDoc(&caCur, docTemp);

	Assert(caCur.cpFirst < caCur.cpLim);

	/* Set up grpprl for deleting */
	grpprl[0] = sprmTDelete;
	while (caCur.cpFirst < caCur.cpLim)
		{
		CpFirstTap(docTemp, caCur.cpFirst);
		caRow = caTap;
		caCur.cpFirst = caRow.cpLim;
		itcMac = vtapFetch.itcMac;
		itcEnd = min(itcLim, itcMac)-1;
		itcStart = itcFirst;
		/* Now we have the difficulties of what to do when
			cells are merged.  The INTENT of the following code is
			this:
			1) if the first cell you are in is merged, but NOT
				the first merged cell, find the first non-merged cell
			2) if the last cell selected is merged, make the
				last cell the last non-merged cell.
			The idea is that this will correspond to what is selected
			on the screen (and so the user will get no surprises!).
		*/
		itcStart = ItcNonMerged(itcStart, &vtapFetch);
		if (itcStart > itcEnd)
			continue;
		if (vtapFetch.rgtc[itcEnd].fFirstMerged)
			itcEnd++;
		if (vtapFetch.rgtc[itcEnd].fMerged)
			itcEnd = ItcNonMerged(itcEnd, &vtapFetch);
		else
			itcEnd++;

		/* In this case, delete the whole row */
		if (itcStart == 0 && itcEnd >= itcMac)
			{
			fApplySprm = fFalse;
			caDel = caRow;
			}
		else
			{
			fApplySprm = fTrue;
			/* Find first cell */
			PcaColumnItc(&caDel, &caRow, itcStart);
			/* Find last cell */
			PcaColumnItc(&caT, &caRow, itcEnd-1);
			caDel.cpLim = caT.cpLim;
			/* Protect the fTtp chTable */
			if (caDel.cpLim == caRow.cpLim)
				caDel.cpLim -= ccpEop;
			}
		dcpT = DcpCa(&caDel);
		if (!FDeleteRM(&caDel))
			goto LAbort;
		dcpAdjust = dcpT - DcpCa(&caDel);
		caCur.cpFirst -= dcpAdjust;
		caCur.cpLim -= dcpAdjust;
		caRow.cpLim -= dcpAdjust;

		/* In the revision marking case, the application of the sprm is taken
		/* care of by the revision marking code */
		if (fApplySprm && DcpCa(&caDel) == 0 && !(PdodMother(docTemp)->dop.fRevMarking))
			{
			grpprl[1] = (char) itcStart;
			grpprl[2] = (char) itcEnd;
			ApplyGrpprlCa(grpprl, 3, &caRow);
			if (vmerr.fMemFail || vmerr.fFmtFailed)
				goto LAbort;
			}
		}

	if (!FSetUndoB1(ucm = (fExpand ? ucmDeleteColumns : ucmDeleteCellsHoriz), uccPaste, &caUndo))
		{
		cmd = cmdCancelled;
		goto LAbort;
		}

	if (!FReplaceCps(&caUndo, PcaSet(&caT, docTemp, cp0, caCur.cpLim)))
		goto LAbort;

	InvalTableCp(selCur.doc, caUndo.cpFirst, DcpCa(&caUndo));

	if (FInTableDocCp(selCur.doc, cpStart))
		{
		CpFirstTap(selCur.doc, cpStart);
		caRow = caTap;
		if (itcFirst >= vtapFetch.itcMac)
			itcFirst = vtapFetch.itcMac - 1;
		while (itcFirst > 0 && vtapFetch.rgtc[itcFirst].fMerged)
			itcFirst--;
		PcaColumnItc(&caT, &caRow, itcFirst);
		cpStart = caT.cpFirst;
		}

	SelectIns(&selCur, cpStart);

	SetUndoAfter(PcaSetDcp(&caT, selCur.doc, caUndo.cpFirst, caCur.cpLim));
	Mac( SetAgainUcm(ucm, ascEdit) );
	cmd = cmdOK;

LAbort:
	DisposeDoc(docTemp);
	DirtyOutline(selCur.doc);

	return cmd;

}



/* C M D  D E L  T A B L E  C E L L S */
/* Delete cells in a table */
/* %%Function:CmdDelTableCells %%Owner:davidmck %%Reviewed:6/27/89 */
CmdDelTableCells(fShiftVert)
int fShiftVert;
{
	int cRows;
	int docTemp = docNil, docCopy = docNil;
	struct CA caSrc, caDest, caUndo, caT;
	struct SELS selsDel, selsMove;
	CP cpCur;
	int cmd = cmdError;
#ifdef DEBUG
	CP cpT;
#endif

	if (!fShiftVert)      /* Horizontal shift */
		return CmdDelTableColumns(fFalse);

	/* Vertical shift */
	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdError;
	if ((docCopy = DocCreate(fnNil)) == docNil)
		{
		DisposeDoc(docTemp);
		return cmdError;
		}
/* FUTURE...MacWord will want to do this too when you have rev marking */
	Win(PdodDoc(docCopy)->dop.fRevMarking = PdodMother(selCur.doc)->dop.fRevMarking);
	TurnOffSel(&selCur);

	Assert(selCur.fTable == 0 || selCur.fTable == 1);

	/* This is a cheep trick to back up one if necessary */
	CpFirstTap(selCur.doc, selCur.cpLim - selCur.fTable);
	PcaSet(&selsDel.ca, docCopy, cp0, caTap.cpLim);
	selsDel.sk = skColumn;
	ItcsFromSelCur(&selsDel.itcFirst, &selsDel.itcLim);
	CpFirstTap(selCur.doc, selCur.cpFirst);
	caUndo = caTap;
	caUndo.cpLim = CpTableLim(selCur.doc, selCur.cpLim);
	selsDel.itcLim = ItcMinItcItcMacPca(selsDel.itcLim, &caUndo);

	if (selsDel.itcFirst >= selsDel.itcLim)
		{
		ErrorEid(eidCantDelEOR,"CmdDelTableColumns");
		goto LAbort;
		}

	SetWholeDoc(docCopy, &caUndo);
	if (vmerr.fMemFail)
		goto LAbort;
	/* Adjust cps to new document */
	selsDel.cpLim -= caUndo.cpFirst;
	selsMove = selsDel;
	selsMove.cpFirst = selsDel.cpLim;
	selsMove.cpLim = DcpCa(&caUndo);
	if (selsMove.cpLim > selsMove.cpFirst)
		{
		/* Remove the section we want to move up */
		if (CmdCopyTable(&selsMove, docTemp) != cmdOK)
			goto LAbort;
		/* Count the number of rows we are moving */
		cRows = CountRowsPca(&selsMove.ca);
		cpCur = cp0;
		/* Find where the last row will go that we are
			* moving (giving us the start of the deletion
			* area).  CachePara is done instead of CpFirstTap
			* for speed. */
		while (cRows > 0)
			{
			CachePara(selsMove.doc, cpCur);
			Assert(vpapFetch.fInTable);
			if (vpapFetch.fTtp)
				cRows--;
			cpCur = caPara.cpLim;
			}
		selsDel.cpFirst = cpCur;
		selsDel.cpLim = DcpCa(&caUndo);
		if (CmdCutTable(&selsDel, docNil, fFalse) != cmdOK)
			goto LAbort;
		/* This points to the part of the table we will insert into */
		PcaSet(&caDest, docCopy, cp0, cpCur);
		PcaSetWholeDoc(&caSrc, docTemp);
		Debug(cpT = caDest.cpFirst);
		if (CmdTableToTablePaste(&caSrc, 0, &caDest, selsDel.itcFirst,
				selsDel.itcLim - selsDel.itcFirst, fFalse, fFalse) != cmdOK)
			goto LAbort;
		Assert(cpT == caDest.cpFirst);
		}
	else
		{
		if (CmdCutTable(&selsDel, docNil, fFalse) != cmdOK)
			goto LAbort;
		PcaPoint(&caDest, docCopy, selsDel.cpLim);
		}

	/* Need to remember most of the table */
	if (!FSetUndoB1(ucmDeleteCellsVert, uccPaste, &caUndo))
		{
		cmd = cmdCancelled;
		goto LAbort;
		}
	if (!FReplaceCps(&caUndo, PcaSetWholeDoc(&caSrc, docCopy)))
		goto LAbort;
	if (FInTableDocCp(selCur.doc, caDest.cpFirst + caUndo.cpFirst))
		{
		caUndo.cpLim = CpTableLim(selCur.doc, caDest.cpFirst + caUndo.cpFirst);
		CpFirstTap(selCur.doc, caDest.cpFirst + caUndo.cpFirst);
		caDest = caTap;
		PcaColumnItc(&caT, &caDest, selsDel.itcFirst);
		}
	else
		caT.cpFirst = caUndo.cpLim = caDest.cpLim + caUndo.cpFirst;

	SelectIns(&selCur, caT.cpFirst);
	SetUndoAfter(&caUndo);
	Mac( SetAgainUcm(ucmDeleteCellsVert, ascEdit) );
	DirtyOutline(selCur.doc);
	cmd = cmdOK;

LAbort:
	DisposeDoc(docTemp);
	DisposeDoc(docCopy);
	return cmd;

}


/* C M D  S P L I T  C E L L S */
/* %%Function:CmdSplitCells %%Owner:davidmck %%Reviewed:6/27/89 */
CmdSplitCells(psel)
struct SEL *psel;
{
	int itcFirst, itcLim, itc;
	CP cpDel, cpFirst, dcp, ccpDel;
	struct CA caT, caRow, caCell, caNewCell, caDel;
	struct CA caUndo, caNewCellT, caCur;
	int docTemp;
	int fRMark;
	char prl[3];
	char prlBorder[18];
	char prlWidth[5];
	int dxaCol;
	struct CHP chpT;
	struct PAP papT;
	int cmd = cmdError;
	int fTooWide = fFalse;

	Assert(psel->fTable || psel->fWithinCell);

	caUndo = psel->ca;

	if (!psel->fTable)
		{
		caUndo.cpFirst = CpFirstTap(caUndo.doc, caUndo.cpFirst);
		CpFirstTap(caUndo.doc, caUndo.cpLim);
		caUndo.cpLim = caTap.cpLim;
		}

	/* Get the itcs of the selection */
	ItcsFromPsel(&itcFirst, &itcLim, psel);

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdNoMemory;

	cpFirst = psel->cpFirst;
	SetWholeDoc(docTemp, &caUndo);
	if (vmerr.fMemFail)
		{
		cmd = cmdNoMemory;
		goto LAbort;
		}

	PcaSetWholeDoc(&caCur, docTemp);
	Assert(caCur.cpFirst < caCur.cpLim);

	CpFirstTap(docTemp, caCur.cpFirst);
	if (!vtapFetch.rgtc[itcFirst].fFirstMerged)
		{
		/* FSplitOk should guarantee that we never get here */
		Assert(fFalse);
		Beep();
		cmd = cmdError;
		goto LAbort;
		}

	/* Set up the row-independent parts of sprm */
	prl[0] = sprmTSplit;
	prl[1] = (char) itcFirst;

	/* This sets up the transfer border sprm */
	prlBorder[0] = prlBorder[6] = sprmTSetBrc;
	prlBorder[1] = prlBorder[7] = (char) itcFirst+1;
	prlBorder[3] = brckTop;
	prlBorder[9] = brckBottom;

	/* Change the width */
	prlWidth[0] = sprmTDxaCol;
	prlWidth[1] = (char) itcFirst;

	/* Do this for each row in the selection */
	while (caCur.cpFirst < caCur.cpLim)
		{
		/* Get the tap and the area */
		CpFirstTap(docTemp, caCur.cpFirst);
		caRow = caTap;
		itcLim = ItcNonMerged(itcFirst+1, &vtapFetch);

		/* set up the row-dependent parts of prl's */
		prlWidth[2] = prl[2] = (char) itcLim;
		prlBorder[2] = prlBorder[8] = (char) itcLim;

		MovePwToPch(&vtapFetch.rgtc[itcFirst].brcTop, &prlBorder[4]);
		MovePwToPch(&vtapFetch.rgtc[itcFirst].brcBottom, &prlBorder[10]);

		dxaCol = (vtapFetch.rgdxaCenter[itcLim] - vtapFetch.rgdxaCenter[itcFirst]) /
				(itcLim - itcFirst);
		if (dxaCol < vtapFetch.dxaGapHalf * 2)
			{
			int dxaWidth;
			long	xaT;

			/* Include extra dxTableBorderMax for ttp cell */
			dxaWidth = vtapFetch.rgdxaCenter[vtapFetch.itcMac] -
					min(0, vtapFetch.rgdxaCenter[0]) + dxTableBorderMax;
			dxaCol = vtapFetch.dxaGapHalf * 2;
			xaT = (long) (itcLim - itcFirst) * ((long) dxaCol) -
					(vtapFetch.rgdxaCenter[itcLim] - vtapFetch.rgdxaCenter[itcFirst]);
			if (xaT > xaRightMaxSci || xaRightMaxSci - dxaWidth < xaT)
				{
				fTooWide = fTrue;
				goto LTooWide;
				}
			}

		MovePwToPch(&dxaCol, &prlWidth[3]);

		/* Apply the sprm */
		ApplyGrpprlCa(&prl, 3, &caRow);
		ApplyGrpprlCa(&prlBorder, 12, &caRow);
		ApplyGrpprlCa(&prlWidth, 5, &caRow);

		if (vmerr.fMemFail || vmerr.fFmtFailed)
			goto LAbort;

		/* Is there text in the invisible cells */
		for (itc = itcFirst+1 ; itc < itcLim ; itc++)
			{
			PcaColumnItc(&caT, &caRow, itc);
			if (caT.cpFirst != caT.cpLim - ccpEop)
				{
				/* Delete text */
				caT.cpLim -= ccpEop;
				dcp = DcpCa(&caT);
				if (!FDelete(&caT))
					goto LAbort;
				caCur.cpLim -= dcp;
				caRow.cpLim -= dcp;
				}
			}
		PcaColumnItc(&caCell, &caRow, itcFirst);
		for (itc = itcLim-1 ; itc > itcFirst ; itc--)
			{
			PcaColumnItc(&caT, &caRow, itcLim-1);
			cpDel = caT.cpFirst;
			CachePara(caCell.doc, caCell.cpLim-ccpEop);
			caNewCell.doc = caCell.doc;
			caNewCell.cpLim = caPara.cpLim - ccpEop;
LGetNewCell:
			if (caCell.cpFirst == caPara.cpFirst)
				break;          /* Only one para left */
			caNewCell.cpFirst = caPara.cpFirst;
			Assert(ccpEop == 2);
			ccpDel = ccpEop;
			if (FOneCharEop(caCell.doc, caNewCell.cpFirst))
				ccpDel = 1;
			caNewCell.cpFirst -= ccpDel;
			caNewCellT = caNewCell;
			AssureLegalSel(&caNewCell); /* make sure we don't split fields */
			if (FNeRgch(&caNewCell, &caNewCellT, sizeof(struct CA)))
				{
				/* if we had a field, back up to include whole para */
				Assert(caNewCell.cpFirst >= caCell.cpFirst);
				CachePara(caNewCell.doc, caNewCell.cpFirst);
				goto LGetNewCell;
				}
			FetchCpAndPara(caCell.doc, cpDel, fcmProps);
			papT = vpapFetch;
			chpT = vchpFetch;
			fRMark = chpT.fRMark;
			CleanTableChp(caCell.doc, &chpT);
			chpT.fRMark = fRMark;
			/* Delete the end-of-cell mark and the paragraph mark */
			/* Yes, this should be FDelete, not FDeleteRM */
			if (!FDelete(PcaSet(&caDel, caCell.doc, cpDel, cpDel+ccpEop))
					|| !FDelete(PcaSet(&caDel, caCell.doc, caNewCell.cpFirst,
					caNewCell.cpFirst + ccpDel)))
				goto LAbort;
			if (!FInsertRgch(caCell.doc, caNewCell.cpFirst,
					rgchTable, cchEop, &chpT, &papT))
				goto LAbort;
			Assert(ccpDel == 1 || ccpDel == 2);
			caRow.cpLim -= ccpDel;
			/* Should be
			caRow.cpLim += (ccpEop - ccpDel) - ccpEop;
			but it is simplified */
			caCur.cpLim -= ccpDel;
			caCell.cpLim -= DcpCa(&caNewCell);
			}
		caCur.cpFirst = caRow.cpLim;
		}

	if (!FSetUndoB1(ucmSplitCells, uccPaste, &caUndo))
		{
		cmd = cmdCancelled;
		goto LAbort;
		}

	if (!FReplaceCps(&caUndo, PcaSet(&caT, docTemp, cp0, caCur.cpLim)))
		goto LAbort;

	Assert(psel == &selCur);
	TurnOffSel(&selCur);
	SelectIns(&selCur, selCur.cpFirst);
	SelectColumn(&selCur, caUndo.cpFirst, caCur.cpFirst + caUndo.cpFirst, itcFirst, itcLim);
	caUndo.cpLim = caUndo.cpFirst + caCur.cpLim;
	SetUndoAfter(&caUndo);
	Mac( SetAgainUcm(ucmSplitCells, ascEdit) );

	cmd = cmdOK;

LTooWide:
	if (fTooWide)
		ErrorEid(eidTooWideToSplit, "CmdSplitCells");

LAbort:
	DirtyOutline(selCur.doc);
	DisposeDoc(docTemp);
	return cmd;

}


/* C M D  M E R G E  C E L L S */
/* %%Function:CmdMergeCells %%Owner:davidmck %%Reviewed:6/27/89 */
CmdMergeCells(psel)
struct SEL *psel;
{
	int itcFirst, itcLim, itc, itcLast;
	CP cpIns, cpFirst;
	struct CA caT, caRow, caDel, caCur;
	struct CA caUndo;
	int docTemp;
	int cmd = cmdError;
	int dxaCol;
	int fRMark;
	char prl[3];
	char prlWidth[10];
	char prlBorder[6];
	struct PAP papT;
	struct CHP chpT;
#ifdef DEBUG
	CP mpitccp[itcMax+1];
#endif

	Assert(psel->fTable);
	caUndo = psel->ca;

	if (!FSetUndoB1(ucmMergeCells, uccPaste, &caUndo))
		return cmdCancelled;

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdNoMemory;

	cpFirst = psel->cpFirst;
	SetWholeDoc(docTemp, &caUndo);
	if (vmerr.fMemFail)
		goto LAbort;

	PcaSetWholeDoc(&caCur, docTemp);
	ItcsFromPsel(&itcFirst, &itcLim, psel);

	prl[0] = sprmTMerge;
	prlBorder[0] = sprmTSetBrc;
	prlBorder[3] = brckRight;
	prlWidth[5] = prlWidth[0] = sprmTDxaCol;
	prlWidth[3] = prlWidth[4] = (char) 0;

	caDel.doc = docTemp;
	while (caCur.cpFirst < caCur.cpLim)
		{
		Assert(caCur.cpFirst == CpFirstTap(docTemp, caCur.cpFirst));
		CpFirstTap(docTemp, caCur.cpFirst);
		caRow = caTap;
		caCur.cpFirst = caRow.cpLim;

		itcLast = min(itcLim,vtapFetch.itcMac) - 1;
		if (itcFirst == itcLast)
			continue;
		Assert (itcLast >= 0);

		Assert(!vtapFetch.rgtc[itcFirst].fMerged);
		if (vtapFetch.rgtc[itcFirst].fMerged)
			continue;

		prl[1] = (char) itcFirst;
		prl[2] = (char) itcLim;

		/* Move the right border to the last cell */
		prlBorder[1] = (char) itcLast-1;
		prlBorder[2] = (char) itcLast;
		MovePwToPch(&vtapFetch.rgtc[itcFirst].brcRight, &prlBorder[4]);

		/* Make the invisible cells have width = 0 */
		/* Gives first cell all the width */
		prlWidth[1] = (char) itcFirst + 1;
		prlWidth[2] = (char) itcLast + 1;
		prlWidth[6] = (char) itcFirst;
		prlWidth[7] = (char) itcFirst + 1;

		dxaCol = vtapFetch.rgdxaCenter[itcLast + 1] - vtapFetch.rgdxaCenter[itcFirst];
		MovePwToPch(&dxaCol, &prlWidth[8]);

		ApplyGrpprlCa(&prlBorder, 6, &caRow);
		ApplyGrpprlCa(&prl, 3, &caRow);
		ApplyGrpprlCa(&prlWidth, 10, &caRow);
		if (vmerr.fMemFail || vmerr.fFmtFailed)
			goto LAbort;

		AssertDo ( PcaColumnItc(&caT, &caRow, itcLast) != 0 );
		/* Where we place the extra chTables */
		cpIns = caT.cpLim;
		CpFirstTap(docTemp, caRow.cpFirst);
#ifdef DEBUG
		bltb(vmpitccp, mpitccp, sizeof(CP) * (itcMax + 1));
#endif
		for (itc = itcFirst ; itc < itcLast; itc++)
			{
			Assert(!FNeRgw(vmpitccp, mpitccp, sizeof(CP)/sizeof(int) * itcLast));
			/* Just don't cache another tap! */
			caDel.cpLim = vmpitccp[itc+1];
			FetchCpAndPara(docTemp, caDel.cpFirst = (caDel.cpLim-ccpEop), fcmProps);
			papT = vpapFetch;
			chpT = vchpFetch;
			fRMark = chpT.fRMark;
			CleanTableChp(psel->doc, &chpT);
			chpT.fRMark = fRMark;
			/* Add extra chTable */
			if (!FInsertRgch(docTemp, cpIns, rgchTable, cchEop, 
					&chpT, &papT))
				goto LAbort;
			/* Delete chTable from cell */
			/* This should be FDelete, not FDeleteRM */
			if (!FDelete(&caDel))
				goto LAbort;
			/* Insert an Eop in the cell */
			chpT.fStrike = chpT.fRMark = fFalse;
			if (!FInsertRgch(docTemp, caDel.cpFirst, rgchEop, cchEop, 
					&chpT, &papT))
				goto LAbort;
			/* Adjust cps */
			caRow.cpLim += ccpEop;
#ifdef INEFFICIENT
			cpIns -= ccpEop - ccpEop;
#endif
			caCur.cpLim += ccpEop;
			}
		caCur.cpFirst = caRow.cpLim;
		}

	if (!FReplaceCps(&caUndo, PcaSet(&caT, docTemp, cp0, caCur.cpFirst)))
		goto LAbort;

	Assert(psel == &selCur);
	TurnOffSel(&selCur);
	SelectColumn(&selCur, cpFirst, caCur.cpFirst + cpFirst, itcFirst, itcLim);

	caUndo.cpLim = caCur.cpFirst + caUndo.cpFirst;
	SetUndoAfter(&caUndo);
	Mac( SetAgainUcm(ucmMergeCells, ascEdit) );
	cmd = cmdOK;

LAbort:
	DisposeDoc(docTemp);
	DirtyOutline(selCur.doc);
	return cmd;

}


#ifdef MAC /* Moved to dlgtable for WIN */
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
#endif


/* A P P L Y  S P R M  T  D X A  C O L */
/* Given a pair of columns and a column width (*pdxaCol), applies it to
	the given ca.  dxaColumn is column width of SEP */
/* %%Function:ApplySprmTDxaCol %%Owner:rosiep */
ApplySprmTDxaCol(itcFirst, itcLim, dxaCol, dxaColumn, pcaApply)
int itcFirst, itcLim, dxaCol, dxaColumn;
struct CA *pcaApply;
{
	int dxaColAuto;
	CP cpCur = pcaApply->cpFirst;
	int dxaSum, *rgdxa, itcLimT, cCols;
	int doc = pcaApply->doc;

	Assert(itcLim > itcFirst);

	while (cpCur < pcaApply->cpLim)
		{
		CpFirstTap(doc, cpCur);
#ifdef DEBUG
			{
		/* Can do CpFirstTap for efficiency because caller did
			CpFirstTap to get pcaApply, but be certain because
			we refer to caTap.cpFirst below. */
			CP cpFirstT = caTap.cpFirst;
			CpFirstTap(doc, cpCur);
			Assert(cpFirstT == caTap.cpFirst);
			}
#endif 
		cpCur = caTap.cpLim;

		if (dxaCol == 0)  /* Auto - Normalize col widths */
			{
			rgdxa = vtapFetch.rgdxaCenter;
			itcLimT = min(itcLim, vtapFetch.itcMac);

			/* Yes, this situation can arise. */
			if (itcLimT <= itcFirst)
				continue;  /* can't normalize this row */

			cCols = itcLimT - itcFirst;
			dxaSum = rgdxa[itcFirst] + rgdxa[vtapFetch.itcMac]
					- rgdxa[itcLimT] + vtapFetch.dxaGapHalf;
			if (dxaSum < dxaColumn)
				{
				dxaColAuto = ((dxaColumn - dxaSum) / cCols);
				if (dxaColAuto < vtapFetch.dxaGapHalf*2)
					continue;
				}
			else
				/* can't normalize this row */
				continue;
			}

		SetWidthOfColumns(doc, caTap.cpFirst, itcFirst, itcLim,
				dxaCol == 0 ? dxaColAuto : dxaCol);
		}

}


/* S E T  W I D T H  O F  C O L U M N S */
/* Sets the width of the given columns in one row, excluding merged cols */
/* %%Function:SetWidthOfColumns %%Owner:rosiep */
SetWidthOfColumns(doc, cp, itcFirst, itcLim, dxaCol)
int doc, itcFirst, itcLim, dxaCol;
CP cp;
{
	char grpprl[5];
	struct CA caRow;
	struct TAP tapSave;
	int itc, itcStart;

	grpprl[0] = sprmTDxaCol;
	MovePwToPch(&dxaCol, &grpprl[3]);
	CpFirstTap(doc, cp);
	tapSave = vtapFetch;
	caRow = caTap;
	for (itc = itcFirst, itcStart = -1 ; itc < itcLim ; itc++)
		{
		if (itcStart == -1 && !tapSave.rgtc[itc].fMerged)
			itcStart = itc;

		if (tapSave.rgtc[itc].fFirstMerged ||
				itc == itcLim -1 && itcStart != -1)
			{
			grpprl[1] = (char) itcStart;
			grpprl[2] = (char) itc + 1;
			ApplyGrpprlCa(grpprl, 5, &caRow);
			itcStart = -1;
			}
		}

}


/*  F I X  U P  F I R S T  C O L U M N  */
/* Adjusts the first column after a sprmTDxaGapHalf has been applied, in
order to leave the row indent untouched. */
/* %%Function:FixUpFirstColumn %%Owner:rosiep */
FixUpFirstColumn(pca)
struct CA *pca;
{
	CP cpCur;
	int dxaCol;
	int doc = pca->doc;

	Assert(pca->cpFirst < pca->cpLim);
	cpCur = pca->cpFirst;
	while (cpCur < pca->cpLim)
		{
		CpFirstTap(doc, cpCur);
		cpCur = caTap.cpLim;
		dxaCol = vtapFetch.rgdxaCenter[1] - vtapFetch.rgdxaCenter[0];
		SetWidthOfColumns(doc, caTap.cpFirst, 0, 1, dxaCol);
		}

}



/* C L E A R  B O T T O M  B R C */
/* Clears the top brc of the cells below this row.  Alters caTap!! */
/* %%Function:ClearBottomBrc %%Owner:rosiep */
ClearBottomBrc(itcFirst, itcLim, brc, pca)
int itcFirst, itcLim, brc;
struct CA *pca;
{
	struct CA caT;
	int itcMac;
	int rgdxaCenter[itcMax+1];

	if (FInTableDocCp(pca->doc, pca->cpLim))
		{
		CpFirstTap(pca->doc, pca->cpFirst);
		itcMac = vtapFetch.itcMac;
		blt(vtapFetch.rgdxaCenter, rgdxaCenter, vtapFetch.itcMac+1);
		Assert(pca->cpLim == caTap.cpLim);
		CpFirstTap(pca->doc, pca->cpLim);
		if (itcMac == vtapFetch.itcMac &&
				!FNeRgch(vtapFetch.rgdxaCenter, rgdxaCenter, (itcMac + 1) * sizeof(int)))
			{
			caT = caTap;
			ApplySetBrcPca(itcFirst, itcLim, brckTop, brc, &caT);
			}
		}

}



/* A P P L Y  S E T  B R C  P C A */
/* Applies the border property to the given pca */
/* %%Function:ApplySetBrcPca %%Owner:rosiep */
ApplySetBrcPca(itcFirst, itcLim, brck, brc, pca)
int itcFirst, itcLim, brck, brc;
struct CA *pca;
{
	char grpprl[6];
	CP cpCur;
#ifdef DEBUG
	struct CA caT;

	/* pca passed in should include the Ttp */
	Assert(pca->cpLim > cp0);
	CachePara(pca->doc, pca->cpLim-1);
	Assert(vpapFetch.fTtp);
#endif

	if (brc == brcNil)
		return;

	grpprl[0] = sprmTSetBrc;
	grpprl[1] = itcFirst;
	grpprl[2] = itcLim;
	grpprl[3] = brck;
	MovePwToPch(&brc, &grpprl[4]);

	/* We can reduce the number of prms necessary by shrinking the
		pca from the beginning up to the first Ttp. */

	cpCur = pca->cpFirst;
	while (CachePara(pca->doc, cpCur), !vpapFetch.fTtp)
		cpCur = caPara.cpLim;
	pca->cpFirst = caPara.cpFirst;

#ifdef DEBUG
	caT = *pca;
#endif
	ApplyGrpprlCa(&grpprl, 6, pca);
	Assert(!FNeRgch(&caT,pca,sizeof(struct CA)));
}




/* C M D  C O P Y  T A B L E  */
/* %%Function:CmdCopyTable %%Owner:davidmck %%Reviewed:6/27/89 */
CmdCopyTable(psel, docDest)
struct SEL *psel;
int docDest;
{
	struct CA caT, caIns, caRow;
	int ctc, itc, dxaColDefault;
	struct TAP tapRow;
	struct TC *ptc, *ptcMac;
	CP cpT;
	struct PAP papT;
	struct CHP chpT;
	struct SELS selsLocal;
	int *pdxaCenter, *pdxaFetch;
	int fRows = fFalse;
	int fFirstMerged;

	Assert(psel->fTable);

	blt(psel, &selsLocal, cwSELS);

	SetWholeDoc(docDest, PcaSetNil(&caT));
	if (vmerr.fMemFail)
		return cmdNoMemory;

	/* If a set of columns, take each cell */
	if (selsLocal.sk == skColumn)
		{
		/* The itc cannot be greater than every itcMac.  This
			handles the situation when the Ttp is selected */
		/* When we are putting information to the clipboard,
			we want to ignore the ttp.  When using CmdCopyTable()
			to insert or delete cells, we want the selection left alone */
		if (docDest == docScrap)
			selsLocal.itcLim = ItcMinItcItcMacPca(selsLocal.itcLim, &selsLocal.ca);
		if (selsLocal.itcLim <= selsLocal.itcFirst)
			{
			ErrorEid(eidNotValidEOR, "CmdCopyTable");
			return cmdError;
			}
		while (selsLocal.cpFirst < selsLocal.cpLim)
			{
			CpFirstTap(selsLocal.doc, selsLocal.cpFirst);
			tapRow = vtapFetch;
			tapRow.itcMac = selsLocal.itcLim - selsLocal.itcFirst;
			caRow = caTap;

			dxaColDefault = dxaInch + tapRow.dxaGapHalf*2;

			SetWords(tapRow.rgtc, 0, (cbTC/sizeof(int))*itcMax);
			if (vtapFetch.itcMac > selsLocal.itcFirst)
				bltb(&vtapFetch.rgtc[selsLocal.itcFirst], &tapRow.rgtc,
						cbTC * min(vtapFetch.itcMac - selsLocal.itcFirst, tapRow.itcMac));

			/* Fix up the merged properties of the new row */
			for ( fFirstMerged = fFalse, ptcMac = &tapRow.rgtc[tapRow.itcMac], ptc = tapRow.rgtc ;
					ptc < ptcMac ; ptc++)
				{
				if (ptc->fFirstMerged)
					{
					if (ptc+1 == ptcMac)
						ptc->fFirstMerged = fFalse;
					else
						fFirstMerged = fTrue;
					}
				else  if (ptc->fMerged)
					{
					if (!fFirstMerged)
						ptc->fMerged = fFalse;
					}
				}

			/* Move the widths to the new row */
			tapRow.rgdxaCenter[0] = vtapFetch.rgdxaCenter[0];
			for (pdxaCenter = tapRow.rgdxaCenter, ptc = tapRow.rgtc,
					pdxaFetch = &vtapFetch.rgdxaCenter[selsLocal.itcFirst], itc = 0 ;
					itc < tapRow.itcMac ; pdxaCenter++, pdxaFetch++, itc++, ptc++ )
				{
				if (itc+selsLocal.itcFirst >= vtapFetch.itcMac)
					{
					if (xaRightMaxSci - *pdxaCenter < dxaColDefault)
						{
						int dxa;
LFixUpTap:
						/* Let's be really vicious about it */
						if (tapRow.rgdxaCenter[0] > xaRightMaxSci/2)
							tapRow.rgdxaCenter[0] = 0;
						/* 10 is a random Fudge factor to keep the row away from
							the right edge */
						dxa = (xaRightMaxSci - max(0, tapRow.rgdxaCenter[0])) / (tapRow.itcMac + 10);
						if (dxa < tapRow.dxaGapHalf * 2)
							tapRow.dxaGapHalf = 0;

						Assert(dxa > 0);
						for (itc = 0, pdxaCenter = tapRow.rgdxaCenter ;
								itc < tapRow.itcMac ; itc++, pdxaCenter++)
							{
							*(pdxaCenter+1) = vtapFetch.rgtc[itc].fMerged ?
									*pdxaCenter : *pdxaCenter + dxa;
							}
						break;
						}
					*(pdxaCenter+1) = *pdxaCenter + dxaColDefault;
					}
				else
					{
					*(pdxaCenter + 1) = *pdxaCenter +
							*(pdxaFetch+1) - *pdxaFetch;

					/* Need to give this one some width that it didn't have before? */
					if (*(pdxaCenter + 1) == *pdxaCenter && !ptc->fMerged)
						{
						if (*(pdxaCenter + 1) > xaRightMaxSci - dxaColDefault)
							goto LFixUpTap;
						*(pdxaCenter+1) += dxaColDefault;
						}

					}
				}

			ctc = vtapFetch.itcMac;
			if (ctc > selsLocal.itcFirst)
				{
				PcaColumnItc(&caIns, &caRow, selsLocal.itcFirst);
				PcaColumnItc(&caT, &caRow, min(ctc, selsLocal.itcLim) - 1);
				caIns.cpLim = caT.cpLim;
				if (!FReplaceCpsRM(PcaPoint(&caT, docDest, 
						CpMacDocEdit(docDest)), &caIns))
					return cmdNoMemory;
				}
			if (ctc < selsLocal.itcLim)       /* Need extra columns */
				{
				if (!FInsTableCCells(docDest, CpMacDocEdit(docDest), selsLocal.itcLim - max(ctc, selsLocal.itcFirst),
						 0, 0))
					return cmdNoMemory;
				}

			/* Insert End of row mark with appropriate properties */
			FetchCpAndPara(docDest, CpMacDocEdit(docDest) - 1, fcmProps);
			papT = vpapFetch;
			chpT = vchpFetch;
			CleanTableChp(docDest, &chpT);
			papT.ptap = &tapRow;
			papT.fTtp = fTrue;
			papT.fInTable = fTrue;
			if (!FInsertRgch(docDest, cpT = CpMacDocEdit(docDest), 
					rgchTable, cchEop, &chpT, &papT))
				return cmdNoMemory;
			selsLocal.cpFirst = caRow.cpLim;
			}
		}
	else  /* Just a set of rows */		
		{
		SetWholeDoc(docDest, &selsLocal.ca);
		if (vmerr.fMemFail)
			return cmdNoMemory;
		fRows = fTrue;
		}

	if (docDest == docScrap)
		{
		vsab.fTable = fTrue;
		vsab.fRows = fRows;
		vsab.fPict = vsab.fBlock = fFalse;
		vsab.docStsh = DocMother(selCur.doc);
		vsab.fFormatted = PdodDoc(vsab.docStsh)->fFormatted;
		vsab.fMayHavePic = PdodMother(vsab.docStsh)->fMayHavePic;
#ifdef WIN
		if (fRows)  /* set up for dde link */
			vsab.caCopy = selsLocal.ca;
		else
			vsab.caCopy.doc = docNil;
		ChangeClipboard(); /* force repaint and set ownership */
#endif /* WIN */
		}

	return cmdOK;
}


/* C M D  C U T  T A B L E  */
/* Cuts the table pointed to by psel and places it in docDest */
/* If fSetUndo it sets the undo state */
/* If docDest is docNil, the structure is just cleared */
/* %%Function:CmdCutTable %%Owner:davidmck %%Reviewd:6/27/89 */
CmdCutTable(psel, docDest, fSetUndo)
struct SEL *psel;
int docDest, fSetUndo;
{
	struct CA caDel, caRow, caT;
	CP cpCur, dcp, dcpT;
	int itc;
	int cmd;
	struct SELS selsLocal;

	if (fSetUndo)
		{
		if (!FSetUndoBefore(docDest != docNil ? ucmCut : ucmClear, uccPaste))
			return cmdCancelled;
		}

	if (psel == &selCur)
		TurnOffSel(&selCur);

	/* Save the table */
	blt(psel, &selsLocal, cwSELS);
	if (docDest != docNil)
		{
#ifdef DEBUG
		struct SELS selsCopy;

		selsCopy = selsLocal;
#endif
		if ((cmd = CmdCopyTable(&selsLocal, docDest)) != cmdOK)
			return cmd;
		Assert(!FNeRgw(&selsCopy, &selsLocal, cwSELS));
		}

	/* Now zero the individual cells out */
	cpCur = selsLocal.cpFirst;

	while (cpCur < selsLocal.cpLim)
		{
		dcp = 0;
		CpFirstTap(selsLocal.doc, cpCur);
		caRow = caTap;
		for (itc = min(vtapFetch.itcMac, selsLocal.itcLim) - 1 ; itc >= selsLocal.itcFirst ; itc--)
			{
			PcaSet(&caDel, caRow.doc, vmpitccp[itc], vmpitccp[itc+1] - ccpEop);
#ifdef WIN
			dcpT = DcpCa(&caDel);
			if (!FDeleteRM(&caDel))
				return cmdNoMemory;
			dcpT -= DcpCa(&caDel);
#else /* MAC */
			if (!FDelete(&caDel))
				return cmdNoMemory;
			dcpT = DcpCa(&caDel);
#endif
			dcp += dcpT;
			/* reestablish table row variables */
			CpFirstTap(selsLocal.doc, cpCur); 
			}
		cpCur = caRow.cpLim - dcp;
		selsLocal.cpLim -= dcp;
		}

	if (psel != &selCur)
		psel->cpLim = selsLocal.cpLim;
	else
		{
		CpFirstTap(selsLocal.doc, selsLocal.cpFirst);
		caRow = caTap;
		PcaColumnItc(&caT, &caRow, min(selsLocal.itcFirst,vtapFetch.itcMac-1));
		SelectIns(psel, caT.cpFirst);
		}

	if (fSetUndo)
		SetUndoAfter(&selsLocal.ca);

	return cmdOK;

}



/*  F  A P P L Y  R G B R C  T A P  */
/* Description: Apply the brcs given in the array to the cells
/* contained in *pcaApply.
/* Returns: fTrue iff properties have been changed and the cp's
/* described by *pcaApply need to be invalidated.
/**/
/* %%Function:FApplyRgbrcTap %%Owner:rosiep */
FApplyRgbrcTap(rgbrc, fEveryCell, pcaApply)
int rgbrc[];
BOOL fEveryCell;
struct CA *pcaApply;
{
	int itcFirst, itcLim, itcMac;
	int rgbrcApply[cbrcCellBorders];
	int doc = pcaApply->doc;
	CP cpCur, cpLim = pcaApply->cpLim;
	BOOL fInval = fFalse;
	BOOL fLastRow;
	struct CA caT;
#ifdef MAC
	int ibrc, ibrcT;
	int brc;
	int brck;
	int *rgbrcCur;
	struct TC tc;
#endif /* MAC */


#ifdef WIN
/* Win uses fEveryCell to mean whole table (i.e. every cell in
	every row).  MacWord uses it to mean apply to each cell in the
	selection individually, as opposed to treating the whole unit as
	a block (this is only used for border formatting).  Opus treats
	the whole unit as a block regardless.
	
	FUTURE:  We should align the two products on this fEveryCell
	business.  Each of us has functionality the other is missing.
*/

	if (fEveryCell)
		{
		itcFirst = 0;
		itcMac = itcMax;
		}
	else
		ItcsFromSelCur(&itcFirst, &itcMac);

#else /* MAC */

	/* Apply the border properties that have changed */
	if (fEveryCell)
		{
		cpCur = pcaApply->cpFirst;
		rgbrcCur = &tc.rgbrc;
		while (cpCur < cpLim)
			{
			CpFirstTap(doc, cpCur);
			caT = caTap;
			cpCur = caTap.cpLim;

			itcLim = min(itcMac,vtapFetch.itcMac);
			itcLim = ItcNonMerged(itcLim, &vtapFetch);

			if (itcLim <= itcFirst)
				continue;

			TcFromTap(&tc,&vtapFetch,itcFirst,itcLim);
			if (rgbrcCur[ibrcRight] != (brc=rgbrc[ibrcRight])
					&& brc != tmvalNinch && itcLim < vtapFetch.itcMac)
				{
				fInval = fTrue;
				ApplySetBrcPca(itcLim, itcLim + 1, brckLeft, brcNone, &caT);
				}
			if (rgbrcCur[ibrcLeft] != (brc=rgbrc[ibrcLeft])
					&& brc != tmvalNinch && itcFirst > 0)
				{
				fInval = fTrue;
				ApplySetBrcPca(itcFirst - 1, itcFirst, brckRight, brcNone, &caT);
				}
			for (ibrc = 0; ibrc < 4 ; ibrc++)
				{
				if (rgbrcCur[ibrc] != (brc=rgbrc[ibrc]) && brc != tmvalNinch)
					{
					fInval = fTrue;
					brck = 1<<ibrc;
					/* collect other borders that are the same */
					for (ibrcT = ibrc+1 ; ibrcT < 4 ; ibrcT++)
						if (rgbrc[ibrcT] == brc && rgbrcCur[ibrcT] != brc)
							{
							brck |= 1<<ibrcT;
							rgbrcCur[ibrcT] = brc;
							}
					/* Now we have the collection, send the sprm */
					ApplySetBrcPca(itcFirst, itcLim, brck, brc, &caT);
					}
				}
			}
		if (fInval)
			ClearBottomBrc(itcFirst, itcMac, rgbrc[ibrcBottom], &caT);
		}
else  /* yes the else is supposed to be inside the MAC only part! */
#endif /* MAC */
		{
		/* do the top, maybe only, row */
		CpFirstTap(doc, pcaApply->cpFirst);
		cpCur = caTap.cpLim;
		itcLim = min(vtapFetch.itcMac,itcMac);

		blt(rgbrc, rgbrcApply, cbrcCellBorders);
		if (!(fLastRow = (cpLim <= caTap.cpLim)))
			rgbrcApply[ibrcBottom] = rgbrc[ibrcInside]; /* correction to blt */

		fInval |= FApplyRgbrcRow(rgbrcApply, itcFirst, itcLim, caTap, fLastRow);

		if (fLastRow)
			return fInval;

		/* do any middle rows of the selection */
		rgbrcApply[ibrcTop] = rgbrc[ibrcInside];
		CpFirstTap(doc, cpCur);
		for ( ; (cpCur=caTap.cpLim) < cpLim; CpFirstTap(doc,cpCur))
			{
			itcLim = min(vtapFetch.itcMac,itcMac);
			fInval |= FApplyRgbrcRow(rgbrcApply, itcFirst, itcLim, caTap, fFalse/*fLastRow*/);
			}

		/* do the last row of the selection */
		Assert(caTap.cpFirst < cpLim);
		rgbrcApply[ibrcBottom] = rgbrc[ibrcBottom];
		itcLim = min(vtapFetch.itcMac,itcMac);
		fInval |= FApplyRgbrcRow(rgbrcApply, itcFirst, itcLim, caTap, fTrue/*fLastRow*/);
		}

	return fInval;
}



/*  F  A P P L Y  R G B R C  R O W  */
/* Apply the supplied borders to the single table row in caRow as needed */
/* %%Function:FApplyRgbrcRow %%Owner:rosiep */
FApplyRgbrcRow(rgbrc, itcFirst, itcLim, caRow, fLastRow)
int rgbrc[], itcFirst, itcLim;
struct CA caRow;
int fLastRow;
{
	int brc;
	int brck;
	int itcFirstT, itcLimT;
	int rgbrcCur[cbrcCellBorders];
	int fInval = fFalse;

	Assert(caTap.doc != docNil && !FNeRgch(&caTap, &caRow, sizeof(struct CA)));
	itcLim = ItcNonMerged(itcLim, &vtapFetch);

	/* This case could arise for short rows; important to catch it */
	if (itcLim <= itcFirst)
		return fFalse;

	GetRowBorders(rgbrcCur, &vtapFetch, itcFirst, itcLim, fTrue /*fStrict*/);

	/* fix things so that setting the right-hand border will stick */
	if (rgbrcCur[ibrcRight] != (brc=rgbrc[ibrcRight])
			&& brc != brcNil && itcLim < vtapFetch.itcMac)
		{
		fInval = fTrue;
		ApplySetBrcPca(itcLim, itcLim+1, brckLeft, brcNone, &caRow);
		}
	/* Fix things so that the left hand border will stick */
	if (rgbrcCur[ibrcLeft] != rgbrc[ibrcLeft]
			&& rgbrc[ibrcLeft] != brcNil && itcFirst > 0)
		{
		fInval = fTrue;
		ApplySetBrcPca(itcFirst-1, itcFirst, brckRight, brcNone, &caRow);
		}

	/* if this is the last row being altered, fix things so the
	/* bottom borders will stick
	/**/
	fLastRow &= rgbrc[ibrcBottom] != rgbrcCur[ibrcBottom]
			&& rgbrc[ibrcBottom] != brcNil;

	/* set top, and maybe bottom, maybe vertical, borders */
	if ((brc = rgbrc[ibrcTop]) != rgbrcCur[ibrcTop] && brc != brcNil)
		{
		fInval = fTrue;
		brck = brckTop;
		if (brc == rgbrc[ibrcBottom] && brc != rgbrcCur[ibrcBottom])
			{
			brck |= brckBottom;
			rgbrcCur[ibrcBottom] = brc;
			}
		if (brc == rgbrc[ibrcLeft] && brc != rgbrcCur[ibrcLeft]
				&& brc == rgbrc[ibrcRight] && brc != rgbrcCur[ibrcRight]
				&& (brc == rgbrc[ibrcInside]
				&& (brc!=rgbrcCur[ibrcInside] || itcFirst>=itcLim-2)))
			{
			brck |= brckLeft | brckRight;
			rgbrcCur[ibrcLeft] = brc;
			rgbrcCur[ibrcRight] = brc;
			rgbrcCur[ibrcInside] = brc;
			}
		ApplySetBrcPca(itcFirst,itcLim,brck,brc,&caRow);
		}

	/* set bottom borders */
	if ((brc = rgbrc[ibrcBottom]) != rgbrcCur[ibrcBottom] && brc != brcNil)
		{
		fInval = fTrue;
		brck = brckBottom;
		if (brc == rgbrc[ibrcLeft] && brc != rgbrcCur[ibrcLeft]
				&& brc == rgbrc[ibrcRight] && brc != rgbrcCur[ibrcRight]
				&& (brc == rgbrc[ibrcInside]
				&& (brc!=rgbrcCur[ibrcInside] || itcFirst>=itcLim-2)))
			{
			brck |= brckLeft | brckRight;
			rgbrcCur[ibrcLeft] = brc;
			rgbrcCur[ibrcRight] = brc;
			rgbrcCur[ibrcInside] = brc;
			}
		ApplySetBrcPca(itcFirst,itcLim,brck,brc,&caRow);
		}

	/* set inside, and maybe left and/or right, borders */
	if (itcFirst < itcLim - 1 && (brc=rgbrc[ibrcInside])!=rgbrcCur[ibrcInside]
			&& brc != brcNil)
		{
		fInval = fTrue;
		itcFirstT = itcFirst + 1;
		itcLimT = itcLim - 1;
		brck = brckLeft | brckRight;
		if (brc == rgbrc[ibrcLeft] && brc != rgbrcCur[ibrcLeft])
			{
			itcFirstT = itcFirst;
			rgbrcCur[ibrcLeft] = brc;
			}
		if (brc == rgbrc[ibrcRight] && brc != rgbrcCur[ibrcRight])
			{
			itcLimT = itcLim;
			rgbrcCur[ibrcRight] = brc;
			}
		if (itcFirstT >= itcLimT)
			{
			itcLimT = itcLim;
			brck = brckLeft;
			}
		ApplySetBrcPca(itcFirstT,itcLimT,brck,brc,&caRow);
		/* Handle setting the inside borders on the outer cells of the
			selection */
		if (itcFirstT != itcFirst)
			ApplySetBrcPca(itcFirst, itcFirstT, brckRight, brc, &caRow);
		if (itcLim != itcLimT)
			ApplySetBrcPca(itcLimT, itcLim, brckLeft, brc, &caRow);
		}

	/* set the left border */
	if ((brc = rgbrc[ibrcLeft]) != rgbrcCur[ibrcLeft] && brc != brcNil)
		{
		fInval = fTrue;
		ApplySetBrcPca(itcFirst,itcFirst+1,brckLeft,brc,&caRow);
		}

	/* set the right border */
	if ((brc = rgbrc[ibrcRight]) != rgbrcCur[ibrcRight] && brc != brcNil)
		{
		fInval = fTrue;
		ApplySetBrcPca(itcLim-1,itcLim,brckRight,brc,&caRow);
		}

	if (fLastRow)
		{
		fInval = fTrue;
		/* Note: ClearBottomBrc alters caTap!! */
		ClearBottomBrc(itcFirst, itcLim, rgbrc[ibrcBottom], &caRow);
		}

	return fInval;
}


/* F  O K  E X P A N D  T A B L E */
/* Check to see if we can expand the table.
/* Returns TRUE if things are OK.  False if we are too big.
* ONLY if fFalse is returned are the following values useful.
*	*pdxaColMin, *pdxaColMax return a range for the column
*  values.  If pdxaColMin is NULL, no information is
*  returned.  *pfIndentBad is set to True if the indent
*  caused the failure.  In that case, pdxaColMax holds the
*  maximum indent value.
* This routine now handles dxaColumnWidth == uninch && == Auto (0)
*  DRM 6/9/89
*/
/* %%Function:FOkExpandTable %%Owner:rosiep */
FOkExpandTable(pca, fWholeTable, itcFirst, itcLim, dxaColumnWidth, dxaIndent, pdxaColMin, pdxaColMax, pfIndentBad)
struct CA *pca;
int fWholeTable, itcFirst, itcLim, dxaColumnWidth, dxaIndent;
int *pdxaColMin, *pdxaColMax, *pfIndentBad;
{
	struct CA caCheck;
	long dxaAdd, dxaWidth;
	CP cpCur;
	int dtc = itcLim - itcFirst;
	long dxaColumnAdd = 0;
	long lxaRightMax = (long) xaRightMaxSci;
	long dxaIndentCur;
	int fOk = fTrue;
	int fIndentBad = fFalse;
	int dxaColMax, dxaColMin;
	int dxaIndentNew;
	int ddxaIndent;
	int dxaSepColumn;
	struct TAP tapLast;

	CacheSect(pca->doc, pca->cpFirst);
	dxaSepColumn = vsepFetch.dxaColumnWidth;

	Assert((pdxaColMin != NULL) == (pdxaColMax != NULL));
	Assert(pfIndentBad == NULL || pdxaColMax != NULL);

	if (pdxaColMin != NULL)
		{
		*pdxaColMin = 0;
		*pdxaColMax = 0x7fff;
		}

	if (fWholeTable)
		{
		CacheTable(pca->doc, pca->cpFirst);
		caCheck = caTable;
#ifdef WIN
		itcFirst = 0;
		itcLim = ItcMacPca(&caCheck);
		dtc = itcLim;
#endif
		if (pca->cpFirst != caTable.cpFirst || pca->cpLim != caTable.cpLim)
			vdxaTRightMax = -1;
		}
	else
		caCheck = *pca;

	Assert((int) uNinch == -1);
	Assert((int) uNinch < 0);
	Assert(wNinch < 0);

	/* dxaColumnAdd is the amount of width added to the table */
	if (dxaColumnWidth > 0)
		dxaColumnAdd = (long) dtc * (long) dxaColumnWidth;

	/* dxaIndentNew is the new indent maxed with 0 */
	if ((dxaIndentNew = dxaIndent) < 0)
		dxaIndentNew = 0;

	/* Rough test to see if the amount added is at least less than
		the maximum width */
	if ((dxaAdd = dxaColumnAdd + dxaIndentNew) > lxaRightMax)
		{
		fOk = fFalse;
		if (pdxaColMin == NULL)
			goto LReturn;
		}

	/* This is a speed-up, we can only do this if vdxaTRightMax
		is valid */
	if (vdxaTRightMax != -1 && dxaColumnWidth > 0)
		{
		int dxaT;

		/* We have a valid width to deal with */
		/* This is a rough test to see if things are ok */
		if ((vdxaTRightMax < (dxaT = (int)(xaRightMaxSci - dxaAdd)))
				&& (vdxaTRightMax - vdxaTLeftMin) < dxaT)
			{
			fOk = fTrue;
			if (pdxaColMin == NULL)
				goto LReturn;
			}
		}

	cpCur = caCheck.cpFirst;

	/* Make sure that vtapFetch will not equal tapLast */
	tapLast.itcMac = -10;

	do
		{
		/* Check each row */
		CpFirstTap(caCheck.doc, cpCur);
		/* Check the current tap with the last tap, if they are the same,
			it all works */
		if (vtapFetch.dxaGapHalf == tapLast.dxaGapHalf
				&& vtapFetch.itcMac == tapLast.itcMac
				&& !FNeRgch(tapLast.rgdxaCenter, vtapFetch.rgdxaCenter, (vtapFetch.itcMac + 1) * sizeof(int)))
			{
			goto LNextRow;
			}
		/* Get the current width.  Extra gap is for the ttp */
		dxaWidth = vtapFetch.rgdxaCenter[vtapFetch.itcMac] - min(0, vtapFetch.rgdxaCenter[0]) +
				dxTableBorderMax;
		if (dxaColumnWidth > 0 && itcFirst < vtapFetch.itcMac)
			dxaWidth -= vtapFetch.rgdxaCenter[min(vtapFetch.itcMac, itcLim)] - vtapFetch.rgdxaCenter[itcFirst];
		else  if (dxaColumnWidth == 0)
			/* Deal with the auto case */
			dxaWidth = max(dxaSepColumn, dxaWidth);

		/* Dealing with indent */
		dxaIndentCur = max(0, vtapFetch.rgdxaCenter[0] + vtapFetch.dxaGapHalf);
		if (dxaIndent > 0)
			ddxaIndent = dxaIndent - dxaIndentCur;
		else  if (dxaIndent != wNinch)
			ddxaIndent = -dxaIndentCur;
		else
			ddxaIndent = 0;

		if (fOk)
			{
			if ((dxaWidth + dxaColumnAdd + ddxaIndent) > lxaRightMax)
				{
				fOk = fFalse;
				Assert(pfIndentBad == NULL || pdxaColMin != NULL);
				if (pdxaColMin == NULL)
					return fFalse;
				}
			}

		if (pdxaColMin != NULL)
			{
			int dtcT;

			dtcT = min(vtapFetch.itcMac, itcLim) - itcFirst;
			Assert(pdxaColMax != NULL);
			if (dtcT > 0)
				dxaColMax = (lxaRightMax - dxaWidth - ddxaIndent)/dtcT;
			else
				dxaColMax = *pdxaColMax;

			/* Deal with case where the indent is at fault, not the column width */
			if (dxaColMax < 0 || fIndentBad || dxaColumnWidth <= 0)
				{
				if (!fIndentBad)
					{
					*pdxaColMax = 0x7fff;
					fIndentBad = fTrue;
					}
				dxaColMax = lxaRightMax - dxaWidth + dxaIndentCur - dxaColumnAdd;
				if (*pdxaColMax > dxaColMax)
					*pdxaColMax = dxaColMax;
				}
			else
				{
				if (*pdxaColMax > dxaColMax)
					*pdxaColMax = dxaColMax;
				if (*pdxaColMin < (dxaColMin = vtapFetch.dxaGapHalf * 2))
					*pdxaColMin = dxaColMin;
				}
			}
		/* Remember the last tap */
		tapLast = vtapFetch;
LNextRow:
		cpCur = caTap.cpLim;
		} 
	while (cpCur < caCheck.cpLim);


LReturn:
	if (pfIndentBad != NULL)
		*pfIndentBad = fIndentBad;
	return fOk;

}


#ifdef MAC
/* T C  F R O M  T A P */
/* Fills in a TC structure given a TAP and a range of itc's */
/* %%Function:TcFromTap %%Owner:NOTUSED */
TcFromTap(ptc,ptap,itcFirst,itcLim)
struct TC *ptc;
struct TAP *ptap;
int itcFirst, itcLim;
{
	struct TC *ptcT, *ptcLim;

	itcLim = min(itcLim,ptap->itcMac);
	if (itcFirst >= itcLim)
		{
		SetWords(ptc,tmvalNinch,cwTC);
		return;
		}

	*ptc = *(ptcT = &ptap->rgtc[itcFirst]);
	ptcLim = &ptap->rgtc[itcLim];

	for ( ; ++ptcT < ptcLim; )
		{
		if (ptcT->grpf != ptc->grpf)
			ptc->grpf = tmvalNinch;
		if (ptcT->brcTop != ptc->brcTop)
			ptc->brcTop = tmvalNinch;
		if (ptcT->brcLeft != ptc->brcLeft)
			ptc->brcLeft = tmvalNinch;
		if (ptcT->brcBottom != ptc->brcBottom)
			ptc->brcBottom = tmvalNinch;
		if (ptcT->brcRight != ptc->brcRight)
			ptc->brcRight = tmvalNinch;
		}
}


#endif /* MAC */

#ifdef MAC
/* F  S P L I T  O K */
/* Can we do a split on this selection? */
/* %%Function:FSplitOk %%Owner:NOTUSED */
FSplitOk(psel)
struct SEL *psel;
{
	CP cpCur, cpLim;
	int itcFirst, itcLim, itc;

	cpCur = psel->cpFirst;
	cpLim = psel->cpLim;

	ItcsFromSelCur(&itcFirst, &itcLim);

	if (cpCur == cpLim)
		cpLim++;

	while (cpCur < cpLim)
		{
		CpFirstTap(psel->doc, cpCur);
		cpCur = caTap.cpLim;

		if (itcFirst >= vtapFetch.itcMac)
			return fFalse;

		if (!vtapFetch.rgtc[itcFirst].fFirstMerged)
			return fFalse;

		for (itc = itcFirst+1 ; itc < itcLim && itc < vtapFetch.itcMac; itc++)
			{
			if (!vtapFetch.rgtc[itc].fMerged)
				return fFalse;

			}
		}

	return fTrue;

}


/* F  M E R G E  O K */
/* Is it ok to merge this selection? */
/* %%Function:FMergeOk %%Owner:rosiep */
FMergeOk(psel)
struct SEL *psel;
{
	CP cpCur, cpLim;
	int itcFirst, itcLim, itc;

	cpCur = psel->cpFirst;
	cpLim = psel->cpLim;
	ItcsFromSelCur(&itcFirst, &itcLim);

	if (cpCur == cpLim)
		return fFalse;

	if (itcFirst == itcLim - 1)
		return fFalse;

	while (cpCur < cpLim)
		{
		if (!FInTableDocCp(psel->doc, cpCur))
			return fFalse;
		CpFirstTap(psel->doc, cpCur);
		cpCur = caTap.cpLim;

		for (itc = itcFirst ; itc < itcLim && itc < vtapFetch.itcMac; itc++)
			{
			if (vtapFetch.rgtc[itc].fFirstMerged)
				return fFalse;
			}
		}


	return fTrue;

}


#endif /* MAC */





