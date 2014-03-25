
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "ch.h"
#include "disp.h"
#include "props.h"
#include "border.h"
#include "banter.h"
#include "format.h"
#include "sel.h"
#include "cmd.h"
#include "file.h"
#include "prm.h"
#include "debug.h"
#include "screen.h"
#include "error.h"
#include "table.h"

#ifdef WIN
#include "message.h"
#include "inter.h"
#endif /* WIN */

#ifdef PROTOTYPE
#include "tablesub.cpt"
#endif /* PROTOTYPE */

extern struct PAP       vpapFetch;
extern struct CHP       vchpFetch;
extern struct TAP       vtapFetch;
extern struct TCC	vtcc;
extern struct CA        caPara;
extern struct CA	caTap;
extern struct CA	caSect;
extern CP               vmpitccp[];

extern char HUGE        *vhpchFetch;

extern struct SEL       selCur;

extern struct SAB       vsab;
extern struct DOD       **mpdochdod[];
extern struct MERR      vmerr;
extern struct SCI       vsci;
extern BOOL             vfSeeSel;
extern int		wwCur;
extern int              vfselType;
extern struct CORW      vrgcorwCur[4];

#ifdef WIN
extern int vssc;
extern struct FTI       vfti;
extern struct PREF      vpref;
#endif /* WIN */



/* P C A  C E L L */
/* Returns pca if it has placed a cell bounds in it.
	Otherwise returns 0. */
/* %%Function:PcaCell %%Owner:davidmck %%Reviewed:6/27/89 */
struct CA *PcaCell(pca, doc, cp)
struct CA *pca;
int doc;
CP cp;
{

	if (!FInTableDocCp(doc, cp))        /* Not in a table */
		{
		Assert(fFalse);
		return(0);
		}

	CacheTc(wwNil, doc, cp, fFalse, fFalse);
	*pca = vtcc.ca;
	return(pca);

}


/* P C A  T A B L E */
/* As PcaCell but returns the bounds for the whole table */
/* %%Function:PcaTable %%Owner:davidmck %%Reviewed:6/27/89 */
struct CA *PcaTable(pca, doc, cp)
struct CA *pca;
int doc;
CP cp;
{

	if (!FInTableDocCp(doc, cp))
		{
		Assert(fFalse);
		return(0);
		}

	return (PcaSet(pca, doc, CpTableFirst(doc,cp), CpTableLim(doc,cp)));

}


#ifdef UNUSED
/* P C A  E X P A N D  W H O L E  T A B L E */
/* Given a source ca, returns the ca of the whole table */
/* Endpoints of the ca which are not in a table are left alone */
/* %%Function:PcaExpandWholeTable %%Owner:davidmck %%Reviewed:6/27/89 */
struct CA *PcaExpandWholeTable(pca, pcaSrc)
struct CA *pca, *pcaSrc;
{
	int doc = pca->doc = pcaSrc->doc;

	pca->cpFirst = (FInTableDocCp(doc, pcaSrc->cpFirst) ?
			CpTableFirst(doc, pcaSrc->cpFirst) : pcaSrc->cpFirst);

	pca->cpLim = (FInTableDocCp(doc, pcaSrc->cpLim) ?
			CpTableLim(doc, pcaSrc->cpLim) : pcaSrc->cpLim);

	return(pca);
}


#endif /* UNUSED */

/* C P  T A B L E  F I R S T
/* Returns cpFirst of first row in table containing doc,cp */
/* This takes into account wierd field/table boundaries */
/* Note: there shouldn't be any anymore now that we guard the para
	mark before a table. */
/* %%Function:CpTableFirst %%Owner:davidmck %%Reviewed:6/27/89 */
CP
CpTableFirst(doc,cp)
int doc;
CP cp;
{
	extern CP vcpFirstTablePara;

	Assert(FInTableDocCp(doc, cp));

	for ( ; cp > cp0; cp = vcpFirstTablePara)             /* Search Backwards until !fInTable */
		{
		if (!FInTableDocCp(doc, cp-1))
			break;
		}
	return cp;
}


/* C P  T A B L E  L I M */
/* Returns the cpLim of the table at doc and cp */
/* %%Function:CpTableLim %%Owner:davidmck %Reviewed:6/27/89 */
CP CpTableLim(doc, cp)
int doc;
CP cp;
{
	CP cpMac = CpMacDoc(doc);

	Assert(FInTableDocCp(doc, cp) || FInTableDocCp(doc, cp-1));

	for ( ; cp < cpMac; cp = caPara.cpLim)             /* Search Forwards until !fInTable */
		{
		if (!FInTableDocCp(doc, cp))
			break;
		}

	return cp;

}


/* E X T E N D  P S E L S  R O W S */
/* Extends psels to cover complete rows */
/* %%Function:ExtendPselsRows %%Owner:davidmck %%Reviewed:6/27/89 */
ExtendPselsRows(psels)
struct SELS *psels;
{
	int doc = psels->doc;

	Assert(psels->fTable || psels->fWithinCell);

	psels->cpFirst = CpFirstTap(doc, psels->cpFirst);

	if (psels->cpLim > caTap.cpLim)
		CpFirstTap(doc, psels->cpLim - 1);

	psels->cpLim = caTap.cpLim;
	psels->sk = skRows;

}


/* E X T E N D  S E L  C U R  C O L U M N S */
/* Extend selCur to cover complete columns */
/* %%Function:ExtendSelCurColumns %%Owner:davidmck %%Reviewed:6/27/89 */
ExtendSelCurColumns()
{
	int itcFirst, itcLim;

	ItcsFromSelCur(&itcFirst, &itcLim);

	SelectColumn(&selCur, CpTableFirst(selCur.doc, selCur.cpFirst),
			CpTableLim(selCur.doc, selCur.cpLim), itcFirst, itcLim);

}


/* C O U N T  R O W S  P C A */
/* Counts the number of rows in the given ca */
/* %%Function:CountRowsPca %%Owner:davidmck %%Reviewed:6/27/89 */
CountRowsPca(pca)
struct CA *pca;
{
	int cRows = 0;
	struct CA caCur;

	caCur = *pca;

	if (caCur.cpFirst == caCur.cpLim)
		return 1;

	CpFirstTap(caCur.doc, caCur.cpLim - 1);
	caCur.cpLim = caTap.cpLim;

	while (caCur.cpFirst < caCur.cpLim)
		{
		/* Count the number of rows selected */
		/* Using CachePara() because it is faster than caching the taps */
		CachePara(caCur.doc, caCur.cpFirst);
		Assert(vpapFetch.fInTable);
		if (vpapFetch.fTtp)
			cRows++;
		caCur.cpFirst = caPara.cpLim;
		}

	Assert(cRows > 0);

	return(cRows);

}


/* P C A  C O L U M N  I T C */
/* Returns pca with the itcth Column from the row in pcaRow */
/* %%Function:PcaColumnItc %%Owner:davidmck %%Reviewed:6/27/89 */
struct CA *PcaColumnItc(pca, pcaRow, itc)
struct CA *pca, *pcaRow;
int itc;
{

	Assert(itc >= 0);

	CpFirstTap(pcaRow->doc, pcaRow->cpFirst);

	if (itc > vtapFetch.itcMac)
		{
		Assert(fFalse);
		return(0);
		}

	PcaSet(pca, pcaRow->doc, vmpitccp[itc], vmpitccp[itc+1]);
	return(pca);

}


/* I T C  F R O M  D O C  C P */
/* Given a doc and a cp, return the itc */
/* %%Function:ItcFromDocCp %%Owner:davidmck %%Reviewed:6/27/89 */
ItcFromDocCp(doc, cp)
int doc;
CP cp;
{

	CacheTc(wwNil, doc, cp, fFalse, fFalse);

	return (vtcc.itc);

}


/* C M D  T A B L E  T O  T A B L E  P A S T E */
/* General table to table paste with column and row extensions */
/* If dtc == -1 (dtcNil) then paste every cell from itcFirstSrc in pcaSrc to pcaDest (allows
	for varying numbers of columns */
/* Any merged cells that are found are split */
/* If fEachCell is true, paste into individual cells so cell properties are kept */
/* We will assume that pcaDest points to the beginning and end of rows */
/* %%Function:CmdTableToTablePaste %%Owner:davidmck %%Reviewed:6/27/89 */
CmdTableToTablePaste(pcaSrc, itcFirstSrc, pcaDest, itcFirstDest, dtc, fSetUndo, fEachCell)
struct CA *pcaSrc, *pcaDest;
int itcFirstSrc, itcFirstDest, dtc;
int fSetUndo, fEachCell;
{
	struct CA caRowDest, caRowSrc, caIns, caDel, caT;
	struct CA caDest, caSrc;
	int itcLimDest, itcLimSrc, fRevMarking;
	CP dcp, dcpT;
	int ctc;
	char grpprl[8];
	int itc, itcSrc, itcMergedFirst, itcMergedMac;
	int dxaCol;
	long dxaRowWidth;
	int fTooWide = fFalse;
	int docTemp;
	int cmd = cmdError;
	struct TC rgtcSrc[itcMax];

#ifdef DEBUG
	/* Asserting that the pcaDest does indeed start and end rows, or else
		it starts right at the end of a table */
	if (pcaDest->cpFirst <= cp0 ||
		(CachePara(pcaDest->doc, pcaDest->cpFirst-1), !vpapFetch.fTtp) ||
		(CachePara(pcaDest->doc, pcaDest->cpFirst), vpapFetch.fTtp))
		{
		Assert(pcaDest->cpFirst == CpFirstTap(pcaDest->doc, pcaDest->cpFirst));
		CpFirstTap(pcaDest->doc, pcaDest->cpLim - 1 /* Not Eop */);
		Assert(pcaDest->cpLim == caTap.cpLim);
		}
#endif

#ifdef WIN
	fRevMarking = PdodMother(pcaDest->doc)->dop.fRevMarking || fEachCell;
#else
	fRevMarking = fEachCell;
#endif

#ifdef INEFFICIENT
	if (!FCheckTablePasteWidth(pcaDest, itcFirstDest, dtc == dtcNil ? (ItcMacPca(pcaSrc) - itcFirstSrc) : dtc))
		{
		ErrorEid(eidMaxWidth,"CmdTableToTablePaste");
		return cmdError;
		}
#endif

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdNoMemory;

/* FUTURE...MacWord will want to do this too when you have rev marking */
#ifdef WIN
	PdodDoc(docTemp)->dop.fRevMarking = PdodMother(pcaDest->doc)->dop.fRevMarking;
#endif

	/* Set the temporary document equal to the destination area */
	SetWholeDoc(docTemp, pcaDest);
	PcaSetWholeDoc(&caDest, docTemp);

	/* Local caSrc for speed and code size */
	caSrc = *pcaSrc;

	/* Get Lim cells from first cells and dtc */
	itcLimDest = itcFirstDest + dtc;
	itcLimSrc = itcFirstSrc + dtc;

	/* Continue the paste while there are rows left in the source */
	while (caSrc.cpFirst < caSrc.cpLim)
		{
		/* This is arranged to take advantage of the caches */
		CpFirstTap(caSrc.doc, caSrc.cpFirst);
		caRowSrc = caTap;

		/* We need to remember only the tcs from the source */
		bltb(vtapFetch.rgtc, rgtcSrc, cbTC * itcMax);

		/* dtc == dtcNil is a special case */
		/* Copy all cells from first to end of the row */
		if (dtc == dtcNil)
			{
			itcLimSrc = vtapFetch.itcMac;
			itcLimDest = itcLimSrc - itcFirstSrc + itcFirstDest;
			}

		/* Get cps of cells to insert */
		PcaColumnItc(&caIns, &caRowSrc, itcFirstSrc);
		if (itcFirstSrc != itcLimSrc - 1)
			{
			PcaColumnItc(&caT, &caRowSrc, itcLimSrc - 1);
			caIns.cpLim = caT.cpLim;
			}

		/* If we have reached the limit of the destination we need
			* to add some new rows */
		if (caDest.cpFirst >= caDest.cpLim)
			{
			dcp = DcpInsertNewRow(caDest.doc, caDest.cpFirst, fFalse,
				(caDest.cpFirst == cp0 ? pcaSrc : NULL));
			if (dcp == cpNil)
				{
				cmd = cmdNoMemory;
				goto LAbort;
				}
			caDest.cpLim += dcp;
			}

		CpFirstTap(caDest.doc, caDest.cpFirst);
		caRowDest = caTap;
		ctc = vtapFetch.itcMac;

		/* Add extra columns */
		if (ctc < itcLimDest)
			{
			int dxaMaxT;
			int dtcT;

			/* Try to find a valid width for the new cells */
			dxaRowWidth = vtapFetch.rgdxaCenter[vtapFetch.itcMac] - min(0, vtapFetch.rgdxaCenter[0]);
			grpprl[0] = sprmTInsert;
			grpprl[1] = (char) ctc;
			grpprl[2] = (char) (dtcT = itcLimDest - ctc);
			dxaCol = vtapFetch.rgdxaCenter[vtapFetch.itcMac] - vtapFetch.rgdxaCenter[vtapFetch.itcMac - 1];

			/* When all else fails, try to make a good guess.  We allow
				* dxaCol == dxaGap to improve the chances of success.  While
				* we do not like no space for text, everyone can deal with it */
			if (dxaCol <= 0)
				dxaCol = max(dxaInch, vtapFetch.dxaGapHalf * 2);
			if ((dxaMaxT = (xaRightMaxSci-dxaRowWidth)/dtcT) < dxaCol)
				{
				if (dxaMaxT > vtapFetch.dxaGapHalf * 3)
					/* Try to give them some space */
					dxaCol = vtapFetch.dxaGapHalf * 3;
				else
					dxaCol = vtapFetch.dxaGapHalf * 2;
				}

			/* One last despairate attempt to work */
			if (dxaRowWidth + dxaCol * dtcT > (long) xaRightMaxSci)
				{
				dxaCol = vtapFetch.dxaGapHalf * 2;
				dxaRowWidth += dxaCol * dtcT;
				/* When all else fails, punt */
				if (dxaRowWidth > (long) xaRightMaxSci)
					{
					ErrorEid(eidMaxWidth,"CmdTableToTablePaste");
					goto LAbort;
					}
				}

			Assert(dtcT + vtapFetch.itcMac < itcMax);
			MovePwToPch(&dxaCol, &grpprl[3]);
			/* Insert new cells */
			if (!FInsTableCCells(caDest.doc, caRowDest.cpLim - ccpEop, dtcT, 0, 0))
				{
				cmd = cmdNoMemory;
				goto LAbort;
				}

			/* Adjust caDest accordingly */
			if (caRowDest.cpLim <= caDest.cpLim)
				caDest.cpLim += dtcT * ccpEop;
			caRowDest.cpLim += ccpEop * dtcT;

			/* Apply grpprl with insertion sprm */
			ApplyGrpprlCa(&grpprl, 5, &caRowDest);
			if (vmerr.fMemFail)
				{
				cmd = cmdNoMemory;
				goto LAbort;
				}
			}
		CpFirstTap(caDest.doc,caDest.cpFirst);

		/* We do not want to put text into merged cells.  Therefore,
			we unmerge those cells before pasting into them. */
		for (itc = itcFirstDest, itcSrc = itcFirstSrc, itcMergedFirst = -1 ;
				itc < itcLimDest ; itc++, itcSrc++)
			{
			if (vtapFetch.rgtc[itc].fFirstMerged && !rgtcSrc[itcSrc].fFirstMerged)
				{
				/* Need to unmerge this */
				itcMergedFirst = itc;
				}
			else  if (vtapFetch.rgtc[itc].fMerged && !rgtcSrc[itcSrc].fMerged)
				{
				int dxaCol;

				if (itcMergedFirst == itcNil)       /* Find the first one */
					for (itcMergedFirst = itc ;
							!vtapFetch.rgtc[itcMergedFirst].fFirstMerged ;
							itcMergedFirst--)
						;
				itcMergedMac = ItcNonMerged(itc+1, &vtapFetch);
				caT = caRowDest;

				/* Change the width to something reasonable */
				grpprl[0] = sprmTDxaCol;
				grpprl[1] = (char) itcMergedFirst;
				grpprl[2] = (char) itcMergedMac;
				dxaCol = (vtapFetch.rgdxaCenter[itcMergedMac] -
						vtapFetch.rgdxaCenter[itcMergedFirst]) / (itcMergedMac - itcMergedFirst);
				if (dxaCol < vtapFetch.dxaGapHalf * 2)
					{
					int dtc = itcMergedMac - itcMergedFirst;
					int dxaTotal = vtapFetch.rgdxaCenter[itcMergedMac] -
					vtapFetch.rgdxaCenter[itcMergedFirst];
					int dxaWidth;

					dxaWidth = vtapFetch.rgdxaCenter[vtapFetch.itcMac] -
							min(0, vtapFetch.rgdxaCenter[0]) + dxTableBorderMax;
					dxaCol = vtapFetch.dxaGapHalf * 2;
					if (xaRightMaxSci - dxaWidth < dxaCol * dtc - dxaTotal)
						{
						fTooWide = fTrue;
						cmd = cmdError;
						goto LAbort;
						}
					}

				MovePwToPch(&dxaCol, &grpprl[3]);

				/* Create and apply the split sprm */
				grpprl[5] = sprmTSplit;
				grpprl[6] = (char) itcMergedFirst;
				grpprl[7] = (char) itcMergedMac;
				ApplyGrpprlCa(&grpprl, 8, &caT);
				if (vmerr.fMemFail)
					{
					cmd = cmdNoMemory;
					goto LAbort;
					}

				CpFirstTap(caDest.doc, caDest.cpFirst);
				itcMergedFirst = itcNil;
				}
			}

		/* if RevMarking, do replace a cell at time */
		if (fRevMarking)
			{
			itcSrc = itcFirstSrc;
			dcp = 0;
			for (itc = itcFirstDest; itc < itcLimDest; itc++, itcSrc++)
				{
				PcaColumnItc(&caIns, &caRowSrc, itcSrc);
				PcaColumnItc(&caDel, &caRowDest, itc);
				caIns.cpLim -= ccpEop;
				caDel.cpLim -= ccpEop;
				dcpT = DcpCa(&caDel);
				if (vmerr.fMemFail || !FReplaceCpsRM(&caDel, &caIns))
					{
					cmd = cmdNoMemory;
					goto LAbort;
					}
				/* Adjust caDest as needed */
				dcp += DcpCa(&caIns) - dcpT + DcpCa(&caDel);
				}
			CpFirstTap(caRowDest.doc, caRowDest.cpFirst);
			caDest.cpFirst = caTap.cpLim;
			}
		else
			{
			/* Do the replacing a row at a time rather than cell by cell */
			PcaColumnItc(&caDel, &caRowDest, itcFirstDest);
			if (itcFirstDest != itcLimDest - 1)
				{
				PcaColumnItc(&caT, &caRowDest, itcLimDest - 1);
				caDel.cpLim = caT.cpLim;
				}

			dcpT = DcpCa(&caDel);
			if (vmerr.fMemFail || !FReplaceCpsRM(&caDel, &caIns))
				{
				cmd = cmdNoMemory;
				goto LAbort;
				}
			/* Adjust caDest as needed */
			dcp = DcpCa(&caIns) - dcpT + DcpCa(&caDel);
			caDest.cpFirst = caRowDest.cpLim + dcp;
			}

		if (caRowDest.cpFirst < caDest.cpLim)
			caDest.cpLim += dcp;
		caSrc.cpFirst = caRowSrc.cpLim;
		}

	if (fSetUndo)
		{
		if (!FSetUndoB1(ucmPaste, uccPaste, pcaDest))
			{
			cmd = cmdCancelled;
			goto LAbort;
			}
		}

	/* Put the temporary document back into the original document */
	caDest.cpFirst = cp0;
	if (!FReplaceCps(pcaDest, &caDest))
		goto LAbort;

	if (fSetUndo)
		{
		pcaDest->cpLim = pcaDest->cpFirst + DcpCa(&caDest);
		SetUndoAfter(pcaDest);
		}

	InvalTableCp(pcaDest->doc, pcaDest->cpFirst, DcpCa(&caDest));

	cmd = cmdOK;

LAbort:
	if (fTooWide)
		ErrorEid(eidTooWideForOperation, "CmdTableToTablePaste");

	DisposeDoc(docTemp);

	return cmd;

}


/* C M D  P A S T E  T O  T A B L E */
/* Does a paste to the current selection when the current selection
	is inside a table */
/* %%Function:CmdPasteToTable %%Owner:davidmck %%Reviewed:6/27/89 */
CmdPasteToTable()
{
	struct CA caT;

	if (!vsab.fTable && FTableInPca(PcaSetWholeDoc(&caT, docScrap)))
		goto LErrorIllegal;

	if (selCur.fWithinCell)
		{
		CachePara(selCur.doc, selCur.cpFirst);
		if (vpapFetch.fTtp && !vsab.fTable)
			{
LErrorIllegal:
			ErrorEid(eidIllegalPasteToTable, "CmdPasteToTable");
			return cmdError;
			}
		if (!vpapFetch.fTtp)
			{
			CpFirstTap(selCur.doc, selCur.cpFirst);
			CacheTc(wwNil, selCur.doc, selCur.cpFirst, fFalse, fFalse);
			if (vtapFetch.rgtc[vtcc.itc].fMerged)
				goto LErrorIllegal;
			}
		}

	if (!selCur.fTable)     /* Small portion of a cell or ins point */
		{
		if (vsab.fBlock)
			{
			ErrorEid(eidIllegalPasteToTable,"CmdPasteToTable");
			return cmdError;
			}
		Assert(selCur.fWithinCell);
#ifdef UNNECESSARY
		if (!selCur.fWithinCell)
			/* Covers more than a table, do a kill */
			return WinMac(CmdPaste(NULL),CmdPaste1());
#endif
		if (vsab.fTable)
			return CmdPasteTableToCell();
		/* Inserting straight text */
		return CmdPasteTextToCell();
		}
	/* Column or Row Selection */
	return CmdPasteRowsColumns(docScrap, selCur.sk == skColumn, fFalse, !vsab.fTable);
}


/* C M D  P A S T E  T A B L E  T O  C E L L */
/* Pastes docScrap, a table, into selCur, which is a cell, or part of one */
/* %%Function:CmdPasteTableToCell %%Owner:davidmck %%Reviewed:6/27/89 */
CmdPasteTableToCell()
{
	struct CA caDel, caRow, caIns, caTable;
	struct CA caT;
	int itcFirst;
	int cmd;

	PcaTable(&caTable, selCur.doc, selCur.cpFirst);
	caTable.cpFirst = CpFirstTap(selCur.doc, selCur.cpFirst);
	itcFirst = ItcFromDocCp(selCur.doc, selCur.cpFirst);

/* FUTURE: remove when MacWord has revision marking (dsb) */
#ifdef MAC
	caDel = selCur.ca;
	FDeleteRM(&caDel);
#endif

	PcaSetWholeDoc(&caIns, docScrap);
	if (itcFirst + ItcMacPca(&caIns) >= itcMax)
		{
		/* Cannot allow too many cells */
		ErrorEid(eidMaxCol,"CmdPasteTableToCell");
		return cmdError;
		}

	if ((cmd = CmdTableToTablePaste(&caIns, 0, &caTable, itcFirst, dtcNil, fTrue, fFalse)) != cmdOK)
		return cmd;

/* copy styles from source doc to destination. */
	Assert(vsab.fFormatted);
	CpFirstTap(selCur.doc, selCur.cpFirst);
	caRow = caTap;
	PcaColumnItc(&caT, &caRow, itcFirst);

#ifdef MAC
	CopyStyles(vsab.docStsh != docNil ? vsab.docStsh : docScrap,
			selCur.doc, caT.cpFirst, DcpCa(&caIns));
#else
	CopyStylesFonts(vsab.docStsh != docNil ? vsab.docStsh:docScrap,
			selCur.doc, caT.cpFirst, DcpCa(&caIns));
#endif			   

	return cmdOK;

}


/* T C K  C K  T E X T  O K  F O R  T A B L E */
/* Can we paste this text into the table? */
/* %%Function:TckCkTextOkForTable %%Owner:davidmck %%Reviewed:6/27/89 */
TckCkTextOkForTable(pca)
struct CA *pca;
{
	struct CA caLocal;
	CP cpCur;
	int iFirst;
	struct PLC **hplcsed = PdodDoc(pca->doc)->hplcsed;
	int cch;
	char rgch[2];


	caLocal = *pca;
	if (hplcsed != hNil)
		{
		iFirst = IInPlc(hplcsed, caLocal.cpFirst);
		if (IInPlc2(hplcsed, caLocal.cpLim, iFirst) - iFirst > 0)
			return tckSection;
		}

	for ( cpCur = caLocal.cpFirst ; cpCur < caLocal.cpLim ; cpCur = caPara.cpLim)
		{
		if (FInTableDocCp(caLocal.doc, cpCur) && caPara.cpLim - ccpEop < caLocal.cpLim)
			{
			FetchRgch(&cch, rgch, caLocal.doc, caPara.cpLim - 1, caPara.cpLim, (int) ccpEop);
			if (rgch[0] == chTable)
				{
#ifdef WIN
#ifdef DEBUG
				if (caPara.cpLim >= ccpEop)
					Assert(ChFetch(caLocal.doc, caPara.cpLim - ccpEop, fcmChars) == chReturn);
#endif /* Debug */
#endif /* WIN */
				return tckTable;
				}
			}
		}
	return tckNil;
}


/* C M D  P A S T E  T E X T  T O  C E L L */
/* Pastes the text in docScrap to the cell selected by selCur */
/* %%Function:CmdPasteTextToCell %%Owner:davidmck %%Reviewed:6/27/89 */
CmdPasteTextToCell()
{
	struct DOD **hdodSrc = mpdochdod[docScrap];
	struct CA caDel, caIns, caT, caCell, caUndo;
	int cch = 0;
	char grpprl[cbMaxGrpprl];
	struct PAP papT, papStd;
	CP cpCur, dcpDelIns;
	struct SELS selsT;
	int docTemp;
	int cmd = cmdError;
	int tck;

	if (!FSetUndoBefore(ucmPaste, uccPaste)) return cmdCancelled;
	caUndo = selCur.ca;

	if ((tck = TckCkTextOkForTable(PcaSetWholeDoc(&caT, docScrap))) != tckNil)
		{
		ErrorEid(tck == tckSection ? eidCantPasteTextWChSec : eidCantPasteTableInTable,
				"CmdPasteTextToCell");
		return cmdError;
		}

#ifdef MAC
	if (CpMacDoc(docScrap) > 2000)
		if (!FAlert(SzFrameKey("Large Paste into Cell.  Do you want to do it?", LgPasteinCell)))
			return cmdCancelled;
#endif /* MAC */

	if ((docTemp = DocCreate(fnNil)) == docNil)
		return cmdNoMemory;

	caDel = selCur.ca;
	SetWholeDoc(docTemp, PcaSetWholeDoc(&caT, docScrap));
	if (vmerr.fMemFail)
		goto LAbort;

	PcaSetWholeDoc(&caIns, docTemp);
	PcaCell(&caCell, caDel.doc, caDel.cpFirst);
	if (caDel.cpLim == caCell.cpLim)
		{
		/* Don't copy an ending EOP */
		/* WARNING, CUTE TRICK AHEAD! - Since docTemp is just a duplicate
			* of docScrap, caIns.cpLim is valid here although it does look
			* funny */
		CachePara(docScrap, caIns.cpLim - ccpEop);
		if (caPara.cpLim == caIns.cpLim)
			{
			caIns.cpLim -= ccpEop;
			/* But we do want to apply its properties */
			papT = vpapFetch;
			papT.fInTable = fTrue;          /* Just to be safe */
			papT.fTtp = fFalse;
			/* Remember this now, apply it later */
			cch = CbGrpprlFromPap(fTrue, &grpprl, &papT, &papStd, fFalse);
			}
		caDel.cpLim -= ccpEop;         /* Protect the end of cell */
		}

	if (selCur.fGraphics)
		TurnOffSel(&selCur);
	ApplyTableProps(&caIns, fTrue);
	if (!FReplaceCpsRM(&caDel, &caIns))
		{
		cmd = cmdNoMemory;
		goto LAbort;
		}

	AssureNestedPropsCorrect(PcaSetDcp(&caT, selCur.doc, selCur.cpFirst,
			DcpCa(&caIns)), fFalse /* fTable - table props already handled */);

/* copy styles from source doc to destination. */
	if (vsab.fFormatted)
#ifdef MAC
		CopyStyles(vsab.docStsh != docNil ? vsab.docStsh : docScrap,
				selCur.doc, selCur.cpFirst, DcpCa(&caIns));
#else
	CopyStylesFonts(vsab.docStsh != docNil ? vsab.docStsh:docScrap,
			selCur.doc, selCur.cpFirst, DcpCa(&caIns));
#endif

	Mac(Assert(DcpCa(&caDel) == 0));
	dcpDelIns = DcpCa(&caIns) + DcpCa(&caDel);

	/* Its ok not to check for mem failure, we really don't care whether this works */
	if (cch != 0)
		ApplyGrpprlCa(&grpprl, cch, PcaSetDcp(&caT, caDel.doc, caUndo.cpFirst + dcpDelIns, ccpEop));

	InvalCaFierce();
	SetSelsIns(&selsT, selCur.doc, selCur.cpFirst);
	PushInsSels(&selsT);
	SelectIns(&selCur, selCur.cpFirst + dcpDelIns);
	PushInsSelCur();

	caUndo.cpLim = caUndo.cpFirst + dcpDelIns;
	SetUndoAfter(&caUndo);
	vfSeeSel = fTrue;
	cmd = cmdOK;

LAbort:
	DisposeDoc(docTemp);

	return cmd;

}



/* C M D  P A S T E  R O W S  C O L U M N S */
/* Paste to a row or column selection */
/* fConverted - Converted from text? (and thus we should paste individual cells) */
/* fText - is source text or a table? */
/* %%Function:CmdPasteRowsColumns %%Owner:davidmck %%Reviewed:6/27/89 */
CmdPasteRowsColumns(docSrc, fColumns, fConverted, fText)
int docSrc;
BOOL fColumns;
{
	int dtc, ctc;
	struct CA caSrc, caDest, caT;
	int cmd;
	int docTemp = docNil;
	int fScrap = docSrc == docScrap;
	CP cpCur, cpMac;

	/* Check the number of columns to see if they match */
	if (fColumns)
		dtc = ItcMinItcItcMacPca(selCur.itcLim, &selCur.ca) -
				selCur.itcFirst;
	else
		{
		CpFirstTap(selCur.doc, selCur.cpFirst);
		dtc = vtapFetch.itcMac;
		}

	/* The check below avoids trying to paste into just the
		* ttp.  Such an action is not allowed */
	if (dtc <= 0)
		{
		ErrorEid(eidIllegalPasteToTable, "CmdPasteRowsColumns");
		return cmdError;
		}

	/* If it is text, we need to convert it into a table to perform
		the paste */
	if (fText)
		{
		if (dtc == 1 && fColumns)
			{
			CpFirstTap(selCur.doc, selCur.cpFirst);
			if (selCur.cpFirst == caTap.cpFirst && selCur.cpLim == caTap.cpLim)
				{
				/* Special case, one cell in one row.
					Don't convert - paste into */
				CachePara(selCur.doc, selCur.cpFirst);
				if (!vpapFetch.fTtp)
					{
					struct CA caT, caRow;

					caRow = caTap;
					PcaColumnItc(&caT, &caRow, selCur.itcFirst);
					Select(&selCur, caT.cpFirst, caT.cpLim - ccpEop);
					Assert(docSrc == docScrap);
					return CmdPasteTextToCell();
					}
				}
			}
		if ((docTemp = DocCreate(fnNil)) == docNil)
			return cmdError;
		SetWholeDoc(docTemp, PcaSetWholeDoc(&caT, docSrc));
		if (vmerr.fMemFail)
			{
			cmd = cmdNoMemory;
			goto LAbort;
			}
		PcaSetWholeDoc(&caT, docTemp);
		CacheSect(docTemp, cp0);
		if (caSect.cpLim <= caT.cpLim)
			{
			ErrorEid(eidCantPasteTextWChSec, "CmdPasteRowsColumns");
			goto LAbort;
			}
		if (!FConvertPcaToTable(&caT, dtc))
			{
			cmd = cmdCancelled;
			goto LAbort;
			}
		docSrc = docTemp;
		}
	CpFirstTap(docSrc, cp0);

	/* The source and destination must have the same number
		of rows and columns for this paste */
	ctc = vtapFetch.itcMac;
	cpMac = CpMacDocEdit(docSrc);
	if (docSrc != docScrap || (!fText && vsab.fRows))
		{
		cpCur = caTap.cpLim;
		while (cpCur < cpMac)
			{
			if (!FInTableDocCp(docSrc, cpCur))
				{
				/* this weird case can happen if we are pasting all hidden
				   text into a table when show hidden is off; this hidden
				   text will follow the table in the temp doc, and we just
				   ignore it */
				cpMac = cpCur;
				break;
				}
			CpFirstTap(docSrc, cpCur);
			if (ctc != vtapFetch.itcMac)
				goto LCopyPasteDiff;
			cpCur = caTap.cpLim;
			}
		}

	if (ctc != dtc || CountRowsPca(&selCur.ca) !=
		CountRowsPca(PcaSet(&caSrc, docSrc, cp0, cpMac)))
		{
LCopyPasteDiff:
		ErrorEid(eidCopyPasteAreaDiff,"CmdPasteRowsColumns");
		goto LAbort;
		}

	caDest = selCur.ca;
	if ((cmd = CmdTableToTablePaste(&caSrc, 0, &caDest,
			fColumns ? selCur.itcFirst : 0, dtc, fTrue, fConverted)) != cmdOK)
		goto LAbort;

/* copy styles from source doc to destination. */
	if (fScrap && vsab.fFormatted)
#ifdef MAC
		CopyStyles(vsab.docStsh != docNil ? vsab.docStsh : docScrap,
				selCur.doc, selCur.cpFirst, DcpCa(&caSrc));
#else
	CopyStylesFonts(vsab.docStsh != docNil ? vsab.docStsh:docScrap,
			selCur.doc, selCur.cpFirst, DcpCa(&caSrc));
#endif			   

LAbort:
	if (docTemp != docNil)
		DisposeDoc(docTemp);

	return cmd;

}


/* I T C S  F R O M  S E L  C U R */
/* %%Function:ItcsFromSelCur %%Owner:rosiep */
ItcsFromSelCur(pitcFirst, pitcLim)
int *pitcFirst, *pitcLim;
{

	ItcsFromPsel(pitcFirst, pitcLim, &selCur);

}


/* I T C S  F R O M  P S E L */
/* Returns the itcFirst and itcLim from psel.  If this is an skColumn
	selection, the operation is trivial.  Otherwise, things are more tricky.
	Psel must be in a table. */
/* %%Function:ItcsFromPsel %%Owner:rosiep */
ItcsFromPsel(pitcFirst, pitcLim, psel)
int *pitcFirst, *pitcLim;
struct SEL *psel;
{
	CP cpT, cpCur;

	if (psel->sk == skColumn) /* Column selection */
		{
		*pitcLim = psel->itcLim;
		*pitcFirst = psel->itcFirst;
		}
	else  if (psel->fTable)    /* Row selection */
		{
		*pitcFirst = 0;
		*pitcLim = ItcMacPca(&psel->ca);
		}
	else		/* Partial cell selection */		
		{
		Assert(psel->fWithinCell);
		*pitcFirst = ItcFromDocCp(psel->doc, psel->cpFirst);
		*pitcLim = *pitcFirst + 1;
		}


}


#ifdef MAC /* moved to COMMAND.C for WIN */
/* F  W H O L E  R O W S  P S E L */
/* Returns true if the selection covers all of the rows */
/* %%Function:FWholeRowsPsel %%Owner:NOTUSED */
FWholeRowsPsel(psel)
struct SEL *psel;
{

	if (!psel->fTable)
		return fFalse;
	else  if (psel->sk == skRows)
		return fTrue;
	else  if (psel->itcFirst == 0 && psel->itcLim >= ItcMacPca(&psel->ca))
		return fTrue;

	return fFalse;

}


#endif /* MAC */

/* S E L E C T  N E X T  P R E V  C E L L */
/* Selects the next (1) or the previous (-1) cell based on
	the current selCur.  Beeps on failure. */
/* %%Function:SelectNextPrevCell %%Owner:rosiep */
SelectNextPrevCell(dtc)
int dtc;
{
	if (!FSelectNextPrevCell(dtc))
		Beep();
}


#ifdef WIN
/* F  N E X T  C E L L */
/* Select the next cell based on the current selCur.  
Returns success flag.  Used by EL. */
/* %%Function:FNextCell %%Owner:rosiep */
FNextCell()
{
	struct SEL *psel = PselActive();

	if (!psel->fWithinCell && !psel->fTable)
		ModeError();
	return FSelectNextPrevCell(1);
}


/* F  P R E V  C E L L */
/* Select the previous cell based on the current PselActive().  
Returns success flag.  Used by EL. */
/* %%Function:FPrevCell %%Owner:rosiep */
FPrevCell()
{
	struct SEL *psel = PselActive();

	if (!psel->fWithinCell && !psel->fTable)
		ModeError();
	return FSelectNextPrevCell(-1);
}


#endif



/* F  S E L E C T  N E X T  P R E V  C E L L */
/* Selects the next (1) or the previous (-1) cell based on
	the current PselActive().  Returns success flag. */
/* %%Function:FSelectNextPrevCell %%Owner:rosiep */
BOOL FSelectNextPrevCell(dtc)
int dtc;
{
	struct CA caCell, caT;
	int itc;

	struct SEL *psel = PselActive();

	if (dtc > 0)
		{
		/* Get current row */
		CpFirstTap(psel->doc, psel->cpFirst);
		caT = caTap;
		if (!psel->fTable)
			PcaCell(&caCell, psel->doc, psel->cpFirst);
		else  if (psel->itcFirst < psel->itcLim - 1)
			{
			PcaColumnItc(&caCell, &caT, psel->itcFirst);
			goto LSelectChars;
			}
		else
			{
			/* Do we have a single cell or several rows?? */
			PcaColumnItc(&caCell, &caT, psel->itcFirst);
			if (psel->cpLim > caTap.cpLim)
				/* More than one row */
				goto LSelectChars;
			}

		do 
			{
			if (caCell.cpLim == caTap.cpLim ||        /* At the end of the row */
					caCell.cpLim+ccpEop == caTap.cpLim)
				{
				caCell.cpLim = caTap.cpLim;
				CachePara(psel->doc, caCell.cpLim);
				if (!FInTableVPapFetch(psel->doc, caCell.cpLim))
					{         /* Need to add a row */
#ifdef WIN
					extern BOOL fElActive, vfElFunc;

					/* The NextCell() function returns
						FALSE at the end of a table rather
						than insert a new row! */
					if (fElActive && vfElFunc)
						return fFalse;
					/* if adding, bag out of add. This is
						easier than killing dyadic and continuing
						since psel comes from pselactive bz
					*/
					if (vssc != sscNil ||
							PdodDoc(psel->doc)->fLockForEdit)
						{
						Beep();
						return fFalse;
						}
#endif /* WIN */

					FSetUndoB1(ucmInsRows, uccPaste, PcaPoint(&caT, psel->doc, caTap.cpLim));
					if (DcpInsertNewRow(psel->doc, caCell.cpLim, fTrue, NULL) == cpNil)
						{
						SetUndoNil();
						return cmdNoMemory;
						}
					DirtyOutline(psel->doc);
					CpFirstTap(psel->doc, caCell.cpLim);
					caT = caTap;
					InvalCp1(&caT);
					SetUndoAfter(&caT);
					SetAgain(bcmNil);
					}
				}

			PcaCell(&caCell, psel->doc, caCell.cpLim);
			itc = ItcFromDocCp(caCell.doc, caCell.cpFirst);
			}
		while (vtapFetch.rgtc[itc].fMerged);
		}
	else
		{
		CpFirstTap(psel->doc, psel->cpFirst);
		caT = caTap;
		if (!psel->fTable)
			PcaCell(&caCell, psel->doc, psel->cpFirst);
		else  if (psel->itcFirst < psel->itcLim - 1)
			{
			PcaColumnItc(&caCell, &caT, psel->itcFirst);
			goto LSelectChars;
			}
		else
			{
			/* Do we have a single cell or several rows?? */
			PcaColumnItc(&caCell, &caT, psel->itcFirst);
			if (psel->cpLim > caTap.cpLim)
				/* More than one row */
				goto LSelectChars;
			}

		do
			{
			if (caCell.cpFirst == CpFirstTap(psel->doc,psel->cpFirst))
				{
				/* At the beginning of the row */
				if (caCell.cpFirst < 1)
					goto LReturnFail;
				CachePara(psel->doc, caCell.cpFirst - 1/*not ccpSect*/);
				if (!vpapFetch.fTtp)        /* Cannot select there */
					{
LReturnFail:
					return fFalse;
					}
				/* back up to beginning of fTtp paragraph */
				caCell.cpFirst -= ccpEop;
				Assert(caCell.cpFirst == caPara.cpFirst);
#ifdef	DEBUG
				CachePara(psel->doc,caCell.cpFirst-1);
				Assert(vpapFetch.fInTable && !vpapFetch.fTtp);
#endif
				}

			PcaCell(&caCell, psel->doc, caCell.cpFirst - ccpEop);
			itc = ItcFromDocCp(caCell.doc, caCell.cpFirst);
			}
		while (vtapFetch.rgtc[itc].fMerged);
		}

LSelectChars:
	Select(psel, caCell.cpFirst, caCell.cpLim - ccpEop);
	NormCp(wwCur, psel->doc, psel->cpFirst, ncpHoriz, 
			dysMacAveLineSci, psel->fInsEnd);
	vfSeeSel = fTrue;

	return fTrue;
}


/* I N V A L  T A B L E  C P */
/* Does an InvalCp3() on the appropriate area */
/* %%Function:InvalTableCp %%Owner:rosiep */
InvalTableCp(doc, cpFirst, dcp)
int doc;
CP cpFirst, dcp;
{
	int ww;

	for (ww = WwDisp(doc, wwNil, fTrue) ; ww != wwNil ; ww = WwDisp(doc, ww, fTrue))
		InvalCp3(HwwdWw(ww), doc, cpFirst, dcp, fTrue);

	InvalTableProps(doc,cpFirst,dcp,fTrue/*fFierce*/);
}


/* I T C  M A C  P C A */
/* Returns the largest itcMac for the given ca range */
/* %%Function:ItcMacPca %%Owner:davidmck %%Reviewed:6/27/89 */
ItcMacPca(pca)
struct CA *pca;
{
	int itcMac;
	struct CA caLocal;
	CP cpCur, cpLim;

	itcMac = 0;
	caLocal = *pca;

	do
		{
		CpFirstTap(caLocal.doc, caLocal.cpFirst);
		itcMac = max(itcMac, vtapFetch.itcMac);
		caLocal.cpFirst = caTap.cpLim;
		}
	while (caLocal.cpFirst < caLocal.cpLim);

	return itcMac;

}


/* I T C  M I N  I T C  I T C  M A C  P C A */
/* Return either itcMin or the largest itcMac for the given ca range,
* whichever is smaller */
/* This routine is faster than a min(itcMin, ItcMacPca(pca)) because
* it can return early */
/* %%Function:ItcMinItcItcMacPca %%Owner:davidmck %%Reviewed:6/27/89 */
ItcMinItcItcMacPca(itcMin, pca)
int itcMin;
struct CA *pca;
{
	int itcMac;
	struct CA caLocal;

	caLocal = *pca;
	itcMac = 0;

	do
		{
		CpFirstTap(caLocal.doc, caLocal.cpFirst);
		itcMac = max(itcMac, vtapFetch.itcMac);
		if (itcMac >= itcMin)
			return itcMin;
		caLocal.cpFirst = caTap.cpLim;
		} 
	while (caLocal.cpFirst < caLocal.cpLim);

	return itcMac;

}


#ifdef MAC
/* F  S E L  W I T H I N  T A B L E */
/* Description: Returns true if deleting the range of characters described
/* by psel will leave an insertion point within a table. In other words,
/* if replacing psel by a character will put that character into a table.
/* This is used to avoid putting illegal characters (like chSect) into a table.
/**/
/* %%Function:FSelWithinTable %%Owner:davidmck %%Reviewed:6/27/89 */
FSelWithinTable(psel)
struct SEL	*psel;
{
	/* does the selection begin within a table? */
#ifdef WIN
	if ((selCur.fIns || !vpref.fAutoDelete) &&
			FInTableDocCp(psel->doc, psel->cpFirst))
#else
		if (FInTableDocCp(psel->doc, psel->cpFirst))
#endif
			return fTrue;

	if (psel->fBlock && !psel->fColumn)
		return fFalse;

	/* does the selection end within a table? */
	if (psel->sk != skIns)
		{
		Assert(psel->cpLim > psel->cpFirst);
		return FInTableDocCp(psel->doc, psel->cpLim-1);
		}

	/* it's OK to have a table strictly contained within the selection */
	return fFalse;

} /* FSelWithinTable */


#endif /* MAC */


/* C L E A N  T A B L E  C H P */
/* Cleans up a chp for inserting marks in a table*/
/* %%Function:CleanTableChp %%Owner:rosiep */
CleanTableChp(doc, pchp)
int doc;
struct CHP *pchp;
{

	pchp->fSpec = fFalse;
	pchp->fcPic = 0;
	Win(pchp->fRMark = PdodDoc(DocMother(doc))->dop.fRevMarking);
	Win(pchp->fStrike = fFalse);

}


/* C L E A N  T A B L E  P A P */
/* %%Function:CleanTablePap %%Owner:rosiep */
CleanTablePap(ppap, doc, fClearBorders, clrIndents)
struct PAP *ppap;
int doc, fClearBorders, clrIndents;
{
	struct PAP papStc;

	switch (clrIndents)
		{
	case clrStyle:  /* Clear execptions */
		MapStc(PdodDoc(doc), ppap->stc, 0, &papStc);
		ppap->dxaLeft = papStc.dxaLeft;
		ppap->dxaLeft1 = papStc.dxaLeft1;
		ppap->dxaRight = papStc.dxaRight;
		break;
	case clrAll:
		ppap->dxaLeft = 0;
		if (ppap->dxaLeft1 < 0)
			ppap->dxaLeft1 = 0;
		ppap->dxaRight = 0;
		break;
	default:
		Assert(fFalse);
	case clrNone:
		break;
		}

	if (fClearBorders)
		{
		ppap->brcTop = ppap->brcLeft = ppap->brcBottom = ppap->brcRight =
				ppap->brcBetween = ppap->brcBar = 0;
		ppap->brcp = ppap->brcl = 0;
		}
	ppap->fSideBySide = fFalse;
	ppap->fTtp = fFalse;

}


#ifdef INEFFICIENT
/* F  C H E C K  T A B L E  P A S T E  W I D T H */
/* Tells us if there is suitable width to paste into this table */
/* %%Function:FCheckTablePasteWidth %%Owner:davidmck %%Reviewed:6/27/89 */
FCheckTablePasteWidth(pca, itcFirst, dtc)
struct CA *pca;
int itcFirst, dtc;
{
	long dxaWidth;
	CP cpCur;
	int itcLim = itcFirst + dtc;

	cpCur = pca->cpFirst;

	do
		{
		CpFirstTap(pca->doc, cpCur);
		cpCur = caTap.cpLim;
		if (itcLim < vtapFetch.itcMac)
			continue;
		dxaWidth = vtapFetch.rgdxaCenter[vtapFetch.itcMac] -
				min(0, vtapFetch.rgdxaCenter[0]);
		/* Ensure only that 0 width columns are availible */
		dxaWidth += vtapFetch.dxaGapHalf * 2 *
				(itcLim - vtapFetch.itcMac + 1);
		if (dxaWidth >= xaRightMaxSci)
			return fFalse;
		} 
	while (cpCur < pca->cpLim);

	return fTrue;

}


#endif /* INEFFICIENT */

/* F  P C A  W H O L E  T A B L E */
/* Returns true if the pca spans all of a table */
/* %%Function:FPcaWholeTable %%Owner:rosiep */
FPcaWholeTable(pca)
struct CA *pca;
{

	if (pca->cpFirst > cp0)
		{
		if (FInTableDocCp(pca->doc, pca->cpFirst - 1 /* not ccpEop! */))
			return fFalse;
		}

	if (pca->cpLim < CpMacDoc(pca->doc))
		{
		if (FInTableDocCp(pca->doc, pca->cpLim))
			return fFalse;
		}

	return fTrue;

}

#ifdef MAC
/* P C A  E X P A N D  I N V A L  T A B L E */
/* Expands pca to extend over the row above and the row below */
/* %%Function:PcaExpandInvalTable %%Owner:rosiep */
struct CA *PcaExpandInvalTable(pca, pcaSrc)
struct CA *pca, *pcaSrc;
{
	int doc = pcaSrc->doc;

	*pca = *pcaSrc;

	Assert(FInTableDocCp(doc, pcaSrc->cpFirst));

	pca->cpFirst = CpFirstTap(doc, pca->cpFirst);
	if (pca->cpFirst != cp0)
		{
		extern CP vcpFirstTablePara;
		if (FInTableDocCp(doc, pca->cpFirst - ccpEop))
			pca->cpFirst = CpFirstTap(doc, vcpFirstTablePara);
		}

	if (pcaSrc->cpFirst == pca->cpLim)
		{
		/* This can be tricky, it is an insertion point */
		CpFirstTap(doc, pca->cpLim);
		if (caTap.cpFirst == pca->cpLim)
			pca->cpLim += ccpEop;
		}

	CpFirstTap(doc, pca->cpLim - 1 /* not ccpEop, for Opus's sake! */);
	pca->cpLim = caTap.cpLim;
	if (pca->cpLim < CpMac2Doc(pca->doc))
		{
		CachePara(doc, pca->cpLim);
		if (FInTableVPapFetch(doc, pca->cpLim))
			{
			CpFirstTap(doc, pca->cpLim);
			pca->cpLim = caTap.cpLim;
			}
		}

	return(pca);
}
#endif /* MAC */


/* C B  G E N  T A B L E  P A P X  F R O M  P A P */
/* Returns the papx for this pap inside a table */
/* %%Function:CbGenTablePapxFromPap %%Owner:rosiep */
CbGenTablePapxFromPap(doc, ppap, ppapx)
int doc;
struct PAP *ppap;
char *ppapx;
{
	struct PAP papStd;
	struct PAP papCell;
	int cchPapx;

	papCell = *ppap;
	papCell.fInTable = fTrue;

	/* Inserting EOL--must be last character of rgch */
	MapStc(*mpdochdod[doc], papCell.stc, 0, &papStd);
	cchPapx = CbGenPapxFromPap(ppapx, &papCell, &papStd, fFalse);

	return cchPapx;
}


/* D X A  M I N  C E L L  F R O M  C A */
/* Returns the width of the smallest cell in the given CA */
/* %%Function:DxaMinCellFromCa %%Owner:rosiep */
DxaMinCellFromCa(pca)
struct CA *pca;
{
	int dxaMin;
	CP cpCur = pca->cpFirst;
	CP cpLim = pca->cpLim;
	int doc = pca->doc;
	struct TAP tapSave;

	CpFirstTap(doc, cpCur);
	dxaMin = DxaMinWidthFromTap(&vtapFetch);
	tapSave = vtapFetch;
	cpCur = caTap.cpLim;

	while (cpCur < cpLim)
		{
		CpFirstTap(doc, cpCur);
		if (vtapFetch.itcMac != tapSave.itcMac
				|| FNeRgw(&vtapFetch.rgdxaCenter, &tapSave.rgdxaCenter, vtapFetch.itcMac+1))
			{
			dxaMin = min(DxaMinWidthFromTap(&vtapFetch),dxaMin);
			tapSave = vtapFetch;
			}
		cpCur = caTap.cpLim;
		}

	return dxaMin;
}


/* D X A  M A X  G A P  F R O M  C A */
/* Returns the largest space between columns (gap) in the given CA */
/* %%Function:DxaMaxGapFromCa %%Owner:rosiep */
DxaMaxGapFromCa(pca)
struct CA *pca;
{
	int dxaMax, doc = pca->doc;
	CP cpCur = pca->cpFirst, cpLim = pca->cpLim;

	CpFirstTap(doc, cpCur);
	dxaMax = vtapFetch.dxaGapHalf;
	cpCur = caTap.cpLim;

	while (cpCur < cpLim)
		{
		CpFirstTap(doc, cpCur);
		if (vtapFetch.dxaGapHalf > dxaMax)
			dxaMax = vtapFetch.dxaGapHalf;
		cpCur = caTap.cpLim;
		}

	return dxaMax * 2;
}


/* I T C  N O N  M E R G E D */
/* Starting at itc, scans the ptap until it reaches itcMac or
* a non-merged cell */
/* %%Function:ItcNonMerged %%Owner:davidmck */
ItcNonMerged(itc, ptap)
int itc;
struct TAP *ptap;
{
	struct TC *ptc;

	for (ptc = &(ptap->rgtc[itc]); itc < ptap->itcMac ; itc++, ptc++)
		if (!ptc->fMerged)
			break;

	return itc;

}



#ifdef WIN

/* F  C O N V E R T  P C A  T O  T A B L E */
/* Given a pca, convert it into a table from text.
* Used mainly by paste routines.
*/
/* %%Function:FConvertPcaToTable %%Owner:rosiep */
FConvertPcaToTable(pca, cCols)
struct CA *pca;
int cCols;
{
	int dxaCenter, itc;
	CP cpIns;
	int tmc;
	struct SEL selT;
	struct TAP tapRow;
	struct CA caUndoT;
	int cColumns, cRows, tblfmt;

	/* Convert pca into a table */
	selT = selCur;
	selT.ca = *pca;
	vfselType = fselTextToTable;

	if ((tblfmt = TblFormatScanSel(&selT, &vrgcorwCur)) == -1)
		return fFalse;
	if (tblfmt != tblfmtPara)
		{
		cColumns = vrgcorwCur[tblfmt].cColumns;
		cRows = vrgcorwCur[tblfmt].cRows;
		}
	else
		{
		cColumns = cCols;
		cRows = (vrgcorwCur[0].cRows-1)/cCols+1;
		}

	if (!FEopBeforePca(&selT.ca))
		return fFalse;

	caUndoT = selT.ca;
	if (!FCheckPcaEndDoc(&caUndoT))
		return fFalse;

	SetWords(&tapRow, 0, cwTAP);
	tapRow.dxaGapHalf = dxaDefaultGapHalf;
	tapRow.rgdxaCenter[0] = dxaCenter = -dxaDefaultGapHalf;
	for (itc = 1 ; itc <= cColumns ; itc++)
		tapRow.rgdxaCenter[itc] = (dxaCenter += dxaInch + 2 * dxaDefaultGapHalf);

	return FFormatTablePca(&selT.ca, &caUndoT, tblfmt, cRows, cColumns, &tapRow);
}



#endif


#ifdef MAC /* moved to COMMAND.C for WIN */
/* F  T A B L E  I N  P C A */
/* Returns true if there is a table anywhere in the ca */
/* %%Function:FTableInPca %%Owner:NOTUSED */
FTableInPca(pca)
struct CA *pca;
{
	CP cpCur;
	int doc = pca->doc;
	CP cpLim = pca->cpLim;

	cpCur = pca->cpFirst;
	if (cpLim == cpCur)
		cpLim++;

	while (cpCur < cpLim)
		{
		CachePara(doc,cpCur);
		if (FInTableDocCp(doc, cpCur))
			return fTrue;
		cpCur = caPara.cpLim;
		}

	return fFalse;

}


#endif /* MAC */

#ifdef WIN  /* So we can have these in our keymap */
/* C M D  N E X T  C E L L */
/* %%Function:CmdNextCell %%Owner:rosiep */
CMD CmdNextCell(pcmb)
CMB *pcmb;
{
	SelectNextPrevCell( 1 );
	return cmdOK;
}


/* C M D  P R E V  C E L L */
/* %%Function:CmdPrevCell %%Owner:rosiep */
CMD CmdPrevCell(pcmb)
CMB *pcmb;
{
	SelectNextPrevCell( -1 );
	return cmdOK;
}


#endif /* WIN */

/* D X A  M I N  W I D T H  F R O M  T A P */
/* Given a tap, returns the width of the smallest cell */
/* %%Function:DxaMinWidthFromTap %%Owner:rosiep */
DxaMinWidthFromTap(ptap)
struct TAP *ptap;
{
	int dxaWidthMin = 0x7fff;
	int *pdxa, dxaT, *pdxaMac;
	struct TC *ptc;

	pdxa = &ptap->rgdxaCenter[0];
	ptc = &ptap->rgtc[0];
	for (pdxaMac = pdxa + ptap->itcMac; pdxa < pdxaMac; pdxa++, ptc++)
		{
		if (dxaWidthMin > (dxaT = *(pdxa+1) - *pdxa) && !ptc->fMerged)
			dxaWidthMin = dxaT;
		}

	return dxaWidthMin;
}
